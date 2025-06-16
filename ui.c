// gcc `pkg-config --cflags --libs gtk+-3.0 glib-2.0 dbus-1 ayatana-appindicator3-0.1` -o connman-ui ui.c gdbus/client.c gdbus/mainloop.c gdbus/object.c gdbus/polkit.c gdbus/watch.c -I. lib/agent.c lib/dbus.c lib/interface.c lib/manager.c lib/service.c lib/technology.c -Ilib/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>
#include <connman-interface.h>
#include <stdlib.h>

#define CUI_UI_PATH "/usr/share/connman_ui_gtk"
#define CUI_ICON_PATH CUI_UI_PATH "/icons"
#define CUI_AGENT_DIALOG_UI_PATH CUI_UI_PATH "/ui/agent.ui"
#define CUI_LEFT_MENU_UI_PATH CUI_UI_PATH "/ui/left_menu.ui"
#define CUI_RIGHT_MENU_UI_PATH CUI_UI_PATH "/ui/right_menu.ui"
#define CUI_SETTINGS_UI_PATH CUI_UI_PATH "/ui/settings.ui"
#define CUI_TRAYICON_UI_PATH CUI_UI_PATH "/ui/tray.ui"
#define GETTEXT_PACKAGE "connman-ui"
#define LOCALEDIR "/usr/share/locale"

// Static variables
static GtkMenu *cui_right_menu = NULL;
static GtkMenu *cui_left_menu = NULL;
static GtkWidget *cui_item_mode_off = NULL;
static GtkWidget *cui_item_mode_on = NULL;
static GtkMenu *cui_tethering_menu = NULL;
static GHashTable *tech_items = NULL;
static int item_position = 3;
static gboolean disabled = TRUE;

static GtkBuilder *builder = NULL;
static GtkDialog *service_settings_dbox = NULL;
static char *path = NULL;
static gboolean ipv4_changed = FALSE;
static const char *ipv4_method = NULL;
static gboolean ipv6_changed = FALSE;
static const char *ipv6_method = NULL;
static const char *ipv6_privacy = NULL;
static gboolean proxy_changed = FALSE;
static const char *proxy_method = NULL;
static gboolean nameservers_changed = FALSE;
static gboolean domains_changed = FALSE;
static gboolean timeservers_changed = FALSE;

GtkBuilder *cui_builder = NULL;

// Forward declarations
struct cui_selected_service;
static void gtk_service_dispose(GObject *object);
static void gtk_service_destroy(GtkWidget *widget);
static gboolean gtk_service_button_release_event(GtkWidget *widget, GdkEventButton *event);
static void gtk_technology_dispose(GObject *object);
static void gtk_technology_destroy(GtkWidget *widget);
static gboolean gtk_technology_button_release_event(GtkWidget *widget, GdkEventButton *event);
void cui_load_theme(void);
void cui_theme_get_type_icone_and_info(const char *type, GdkPixbuf **image, const char **info);
void cui_theme_get_signal_icone_and_info(uint8_t signal_strength, GdkPixbuf **image, const char **info);
void cui_theme_get_state_icone_and_info(enum connman_state state, GdkPixbuf **image, const char **info);
void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image, const char **info);
gint cui_load_trayicon(GtkBuilder *builder);
gint cui_load_left_menu(GtkBuilder *builder, AppIndicator *indicator);
gint cui_load_right_menu(GtkBuilder *builder, AppIndicator *indicator);
void cui_trayicon_update_icon(void);
void cui_tray_hook_left_menu(gpointer callback);
void cui_tray_hook_right_menu(gpointer callback);
void cui_tray_left_menu_disable(void);
void cui_tray_enable(void);
void cui_tray_disable(void);
gint cui_load_agent_dialogs(void);
void cui_agent_init_callbacks(void);
void cui_agent_set_selected_service(const char *path, const char *name);
void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether);
void cui_right_menu_enable_only_quit(void);
void cui_right_menu_enable_all(void);
gint cui_settings_popup(const char *path);
static void add_technology(const char *path);
GtkLabel *set_label(GtkBuilder *builder, const char *name, const char *value, const char *default_value);
GtkEntry *set_entry(GtkBuilder *builder, const char *name, const char *value, const char *default_value);
GtkWidget *set_widget_sensitive(GtkBuilder *builder, const char *name, gboolean value);
GtkWidget *set_widget_hidden(GtkBuilder *builder, const char *name, gboolean value);
GtkWidget *set_button_toggle(GtkBuilder *builder, const char *name, gboolean active);
GtkImage *set_image(GtkBuilder *builder, const char *name, GdkPixbuf *image, const char *info);
void set_signal_callback(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler, gpointer user_data);
const char *get_entry_text(GtkBuilder *builder, const char *name);

// Structure definitions
struct cui_selected_service {
    char *path;
    char *name;
};

// GtkService class
#define GTK_TYPE_SERVICE (gtk_service_get_type())
#define GTK_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_SERVICE, GtkService))
#define GTK_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_SERVICE, GtkServiceClass))
#define GTK_IS_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_SERVICE))
#define GTK_IS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_SERVICE))
#define GTK_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_SERVICE, GtkServiceClass))

typedef struct _GtkService GtkService;
typedef struct _GtkServiceClass GtkServiceClass;
typedef struct _GtkServicePrivate GtkServicePrivate;

struct _GtkService {
    GtkMenuItem parent;
    GtkServicePrivate *priv;
    gchar *path;
};

struct _GtkServiceClass {
    GtkMenuItemClass parent_class;
};

struct _GtkServicePrivate {
    GtkBox *box;
    GtkLabel *name;
    GtkImage *state;
    GtkImage *security;
    GtkImage *signal;
    gboolean selected;
};

GType gtk_service_get_type(void);
GtkService *gtk_service_new(const char *path);

G_DEFINE_TYPE_WITH_PRIVATE(GtkService, gtk_service, GTK_TYPE_MENU_ITEM);

static void gtk_service_class_init(GtkServiceClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->dispose = gtk_service_dispose;
    widget_class->destroy = gtk_service_destroy;
    widget_class->button_release_event = gtk_service_button_release_event;
}

static void gtk_service_init(GtkService *service) {
    GtkServicePrivate *priv = g_type_instance_get_private((GTypeInstance *)service, GTK_TYPE_SERVICE);
    service->priv = priv;
    priv->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    priv->name = GTK_LABEL(gtk_label_new(NULL));
    priv->state = GTK_IMAGE(gtk_image_new());
    priv->security = GTK_IMAGE(gtk_image_new());
    priv->signal = GTK_IMAGE(gtk_image_new());
    priv->selected = FALSE;

    gtk_widget_set_halign(GTK_WIDGET(priv->name), GTK_ALIGN_START);
    gtk_widget_set_halign(GTK_WIDGET(priv->state), GTK_ALIGN_END);
    gtk_widget_set_halign(GTK_WIDGET(priv->security), GTK_ALIGN_END);
    gtk_widget_set_halign(GTK_WIDGET(priv->signal), GTK_ALIGN_END);

    gtk_box_set_homogeneous(priv->box, FALSE);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->name), TRUE, TRUE, 0);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->state), TRUE, TRUE, 0);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->security), TRUE, TRUE, 0);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->signal), TRUE, TRUE, 0);

    gtk_widget_set_visible(GTK_WIDGET(priv->box), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->name), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->state), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(priv->security), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(priv->signal), FALSE);

    gtk_container_add(GTK_CONTAINER(service), GTK_WIDGET(priv->box));
}

static void gtk_service_dispose(GObject *object) {
    GtkService *service = GTK_SERVICE(object);
    g_free(service->path);
    service->path = NULL;
    G_OBJECT_CLASS(gtk_service_parent_class)->dispose(object);
}

static void gtk_service_destroy(GtkWidget *widget) {
    GtkService *service = GTK_SERVICE(widget);
    GtkServicePrivate *priv = service->priv;
    if (priv && !priv->selected) {
        connman_service_set_property_changed_callback(service->path, NULL, service);
        connman_service_set_property_error_callback(service->path, NULL, service);
    }
    GTK_WIDGET_CLASS(gtk_service_parent_class)->destroy(widget);
}

static gboolean gtk_service_button_release_event(GtkWidget *widget, GdkEventButton *event) {
    GtkService *service = GTK_SERVICE(widget);
    if (event->button == 1) {
        if (connman_service_is_connected(service->path))
            connman_service_disconnect(service->path);
        else {
            cui_agent_set_selected_service(service->path, connman_service_get_name(service->path));
            connman_service_connect(service->path);
        }
    } else if (event->button == 3) {
        service->priv->selected = TRUE;
        cui_settings_popup(service->path);
    }
    GtkWidget *parent = gtk_widget_get_parent(widget);
    if (parent && GTK_IS_MENU_SHELL(parent)) {
        gtk_menu_shell_activate_item(GTK_MENU_SHELL(parent), widget, TRUE);
    }
    return TRUE;
}

static void service_set_name(GtkService *service) {
    const char *name = connman_service_get_name(service->path);
    if (!name) name = "- Hidden -";
    char *markup;
    if (connman_service_is_favorite(service->path)) {
        if (g_strcmp0(connman_service_get_type(service->path), "wifi") == 0) {
            markup = g_markup_printf_escaped("<b>%s</b> <i> (%s) </i>", name, connman_service_get_security(service->path));
        } else {
            markup = g_markup_printf_escaped("<b>%s</b>", name);
        }
    } else {
        if (g_strcmp0(connman_service_get_type(service->path), "wifi") == 0) {
            markup = g_markup_printf_escaped("%s <i> (%s) </i>", name, connman_service_get_security(service->path));
        } else {
            markup = g_markup_printf_escaped("%s", name);
        }
    }
    gtk_label_set_markup(service->priv->name, markup);
    g_free(markup);
}

static void service_set_state(GtkService *service) {
    GtkServicePrivate *priv = service->priv;
    const struct connman_ipv4 *ipv4;
    enum connman_state state;
    GdkPixbuf *image = NULL;
    const char *ip = NULL;
    const char *info;

    if (!connman_service_is_connected(service->path)) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->name), "");
        return;
    }

    ipv4 = connman_service_get_ipv4(service->path);
    if (ipv4) {
        ip = ipv4->address;
    } else {
        const struct connman_ipv6 *ipv6 = connman_service_get_ipv6(service->path);
        if (ipv6) ip = ipv6->address;
    }
    gtk_widget_set_tooltip_text(GTK_WIDGET(priv->name), ip ? ip : "");

    state = connman_service_get_state(service->path);
    cui_theme_get_state_icone_and_info(state, &image, &info);
    if (image) {
        gtk_widget_set_visible(GTK_WIDGET(priv->state), TRUE);
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->state), info);
        gtk_image_set_from_pixbuf(priv->state, image);
        g_object_unref(image);
    }
}

static void service_set_signal(GtkService *service) {
    GtkServicePrivate *priv = service->priv;
    GdkPixbuf *image = NULL;
    const char *type = connman_service_get_type(service->path);
    const char *info;

    if (g_strcmp0(type, "wifi") == 0) {
        uint8_t strength = connman_service_get_strength(service->path);
        cui_theme_get_signal_icone_and_info(strength, &image, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &image, &info);
    }
    if (image) {
        gtk_widget_set_visible(GTK_WIDGET(priv->signal), TRUE);
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->signal), info);
        gtk_image_set_from_pixbuf(priv->signal, image);
        g_object_unref(image);
    }
}

static void service_property_changed_cb(const char *path, const char *property, void *user_data) {
    GtkService *service = user_data;
    service_set_name(service);
    service_set_state(service);
    service_set_signal(service);
}

GtkService *gtk_service_new(const char *path) {
    if (!path) return NULL;
    GtkService *service = g_object_new(GTK_TYPE_SERVICE, NULL);
    if (!service) return NULL;
    service->path = g_strdup(path);
    if (!service->path) {
        g_object_unref(service);
        return NULL;
    }
    connman_service_set_property_changed_callback(service->path, service_property_changed_cb, service);
    service_set_name(service);
    service_set_state(service);
    service_set_signal(service);
    return service;
}

// GtkTechnology class
#define GTK_TYPE_TECHNOLOGY (gtk_technology_get_type())
#define GTK_TECHNOLOGY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_TECHNOLOGY, GtkTechnology))
#define GTK_TECHNOLOGY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_TECHNOLOGY, GtkTechnologyClass))
#define GTK_IS_TECHNOLOGY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_TECHNOLOGY))
#define GTK_IS_TECHNOLOGY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_TECHNOLOGY))
#define GTK_TECHNOLOGY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_TECHNOLOGY, GtkTechnologyClass))

typedef struct _GtkTechnology GtkTechnology;
typedef struct _GtkTechnologyClass GtkTechnologyClass;
typedef struct _GtkTechnologyPrivate GtkTechnologyPrivate;

struct _GtkTechnology {
    GtkMenuItem parent;
    GtkTechnologyPrivate *priv;
    gchar *path;
};

struct _GtkTechnologyClass {
    GtkMenuItemClass parent_class;
};

struct _GtkTechnologyPrivate {
    GtkBox *box;
    GtkSwitch *enabler;
    GtkLabel *name;
    GtkCheckMenuItem *tethering;
};

GType gtk_technology_get_type(void);
GtkTechnology *gtk_technology_new(const char *path);
GtkMenuItem *gtk_technology_get_tethering_item(GtkTechnology *technology);

G_DEFINE_TYPE_WITH_PRIVATE(GtkTechnology, gtk_technology, GTK_TYPE_MENU_ITEM);

static void gtk_technology_class_init(GtkTechnologyClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->dispose = gtk_technology_dispose;
    widget_class->destroy = gtk_technology_destroy;
    widget_class->button_release_event = gtk_technology_button_release_event;
}

static void gtk_technology_init(GtkTechnology *technology) {
    GtkTechnologyPrivate *priv = g_type_instance_get_private((GTypeInstance *)technology, GTK_TYPE_TECHNOLOGY);
    technology->priv = priv;
    priv->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    priv->enabler = GTK_SWITCH(gtk_switch_new());
    priv->name = GTK_LABEL(gtk_label_new(NULL));

    gtk_box_set_homogeneous(priv->box, TRUE);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->enabler), FALSE, FALSE, 0);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->name), FALSE, FALSE, 0);

    gtk_widget_set_visible(GTK_WIDGET(priv->box), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->enabler), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->name), TRUE);

    gtk_container_add(GTK_CONTAINER(technology), GTK_WIDGET(priv->box));
}

static void gtk_technology_dispose(GObject *object) {
    GtkTechnology *technology = GTK_TECHNOLOGY(object);
    g_free(technology->path);
    technology->path = NULL;
    G_OBJECT_CLASS(gtk_technology_parent_class)->dispose(object);
}

static void gtk_technology_destroy(GtkWidget *widget) {
    GtkTechnology *technology = GTK_TECHNOLOGY(widget);
    GtkTechnologyPrivate *priv = technology->priv;
    connman_technology_set_property_changed_callback(technology->path, NULL, technology);
    connman_technology_set_property_error_callback(technology->path, NULL, technology);
    if (priv->tethering) {
        gtk_widget_destroy(GTK_WIDGET(priv->tethering));
        priv->tethering = NULL;
    }
    GTK_WIDGET_CLASS(gtk_technology_parent_class)->destroy(widget);
}

static gboolean gtk_technology_button_release_event(GtkWidget *widget, GdkEventButton *event) {
    GtkTechnology *technology = GTK_TECHNOLOGY(widget);
    GtkTechnologyPrivate *priv = technology->priv;
    if (event->button == 1) {
        gboolean enable = !gtk_switch_get_active(GTK_SWITCH(priv->enabler));
        if (connman_technology_enable(technology->path, enable) != 0)
            gtk_widget_set_sensitive(GTK_WIDGET(priv->enabler), FALSE);
    } else if (event->button == 3) {
        if (g_strcmp0(connman_technology_get_type(technology->path), "wifi") == 0)
            cui_agent_set_wifi_tethering_settings(technology->path, FALSE);
        GtkWidget *parent = gtk_widget_get_parent(widget);
        if (parent && GTK_IS_MENU_SHELL(parent))
            gtk_menu_shell_activate_item(GTK_MENU_SHELL(parent), widget, TRUE);
    }
    return TRUE;
}

static void technology_property_error_cb(const char *path, const char *property, int error, void *user_data) {
    if (error != 0)
        printf("Could not set property %s: error %d\n", property, error);
}

static void set_technology_name(GtkTechnology *technology, gboolean enabled) {
    const char *name = connman_technology_get_name(technology->path);
    if (enabled) {
        char *markup = g_markup_printf_escaped("<b>%s</b>", name);
        gtk_label_set_markup(technology->priv->name, markup);
        g_free(markup);
    } else {
        gtk_label_set_text(technology->priv->name, name);
    }
}

static void technology_property_changed_cb(const char *path, const char *property, void *user_data) {
    GtkTechnology *technology = user_data;
    GtkTechnologyPrivate *priv = technology->priv;
    if (g_strcmp0(property, "Powered") == 0) {
        gboolean enabled = connman_technology_is_enabled(path);
        gtk_switch_set_active(priv->enabler, enabled);
        gtk_widget_set_sensitive(GTK_WIDGET(priv->enabler), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(priv->tethering), enabled);
        set_technology_name(technology, enabled);
    } else if (g_strcmp0(property, "Tethering") == 0) {
        gtk_check_menu_item_set_active(priv->tethering, connman_technology_is_tethering(path));
        gtk_widget_set_sensitive(GTK_WIDGET(priv->tethering), connman_technology_is_enabled(path));
    }
}

static gboolean gtk_technology_tethering_button(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GtkTechnology *technology = user_data;
    GtkTechnologyPrivate *priv = technology->priv;
    if (event->button != 1) return TRUE;
    gtk_widget_set_sensitive(GTK_WIDGET(priv->tethering), FALSE);
    gboolean tethering = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
    connman_technology_tether(technology->path, tethering);
    return TRUE;
}

GtkTechnology *gtk_technology_new(const char *path) {
    if (!path) return NULL;
    GtkTechnology *technology = g_object_new(GTK_TYPE_TECHNOLOGY, NULL);
    if (!technology) return NULL;
    technology->path = g_strdup(path);
    if (!technology->path) {
        g_object_unref(technology);
        return NULL;
    }
    GtkTechnologyPrivate *priv = technology->priv;
    connman_technology_set_property_changed_callback(technology->path, technology_property_changed_cb, technology);
    connman_technology_set_property_error_callback(technology->path, technology_property_error_cb, NULL);
    gboolean enabled = connman_technology_is_enabled(technology->path);
    gtk_switch_set_active(priv->enabler, enabled);
    char *label = g_strdup_printf("via: %s", connman_technology_get_name(technology->path));
    priv->tethering = GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(label));
    g_free(label);
    gtk_check_menu_item_set_active(priv->tethering, connman_technology_is_tethering(technology->path));
    gtk_widget_set_visible(GTK_WIDGET(priv->tethering), TRUE);
    g_signal_connect(priv->tethering, "button-release-event", G_CALLBACK(gtk_technology_tethering_button), technology);
    set_technology_name(technology, enabled);
    return technology;
}

GtkMenuItem *gtk_technology_get_tethering_item(GtkTechnology *technology) {
    return GTK_MENU_ITEM(technology->priv->tethering);
}

static void add_technology(const char *path)
{
    GtkTechnology *t;

    t = gtk_technology_new(path);
    if (!t) return;
    gtk_menu_shell_insert(GTK_MENU_SHELL(cui_right_menu),
                    GTK_WIDGET(t), item_position);
    item_position++;

    gtk_widget_set_visible(GTK_WIDGET(t), TRUE);
    gtk_widget_show(GTK_WIDGET(t));

    gtk_menu_shell_append(GTK_MENU_SHELL(cui_tethering_menu),
            GTK_WIDGET(gtk_technology_get_tethering_item(t)));

    g_hash_table_insert(tech_items, g_strdup(t->path), t);
}

static void technology_added_cb(const char *path)
{
    add_technology(path);
}

static void technology_removed_cb(const char *path)
{
    g_hash_table_remove(tech_items, path);
}

static void delete_technology_item(gpointer data)
{
    GtkWidget *item = data;
    gtk_widget_destroy(item);
    item_position--;
}

// Theme handling
static GtkIconTheme *icon_theme = NULL;

void cui_load_theme(void) {
    icon_theme = gtk_icon_theme_get_default();
    gtk_icon_theme_append_search_path(icon_theme, CUI_ICON_PATH);
}

void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image, const char **info) {
    if (image) *image = gtk_icon_theme_load_icon(icon_theme, "nm-adhoc", 16, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (info) *info = "Tethering";
}

void cui_theme_get_type_icone_and_info(const char *type, GdkPixbuf **image, const char **info) {
    const char *nfo = NULL;
    const char *icon_name = NULL;
    if (g_strcmp0(type, "ethernet") == 0) {
        icon_name = "network-wired-symbolic";
        nfo = "Ethernet";
    } else if (g_strcmp0(type, "wifi") == 0) {
        icon_name = "network-wireless-symbolic";
        nfo = "Wi-Fi";
    }
    if (image && icon_name) *image = gtk_icon_theme_load_icon(icon_theme, icon_name, 16, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (info) *info = nfo;
}

void cui_theme_get_signal_icone_and_info(uint8_t signal_strength, GdkPixbuf **image, const char **info) {
    const char *nfo = NULL;
    const char *icon_name = NULL;
    if (signal_strength >= 80) {
        icon_name = "network-wireless-signal-excellent-symbolic";
        nfo = "Excellent";
    } else if (signal_strength >= 60) {
        icon_name = "network-wireless-signal-good-symbolic";
        nfo = "Good";
    } else if (signal_strength >= 40) {
        icon_name = "network-wireless-signal-ok-symbolic";
        nfo = "OK";
    } else if (signal_strength >= 20) {
        icon_name = "network-wireless-signal-weak-symbolic";
        nfo = "Weak";
    } else {
        icon_name = "network-wireless-signal-none-symbolic";
        nfo = "None";
    }
    if (image && icon_name) *image = gtk_icon_theme_load_icon(icon_theme, icon_name, 16, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (info) *info = nfo;
}

void cui_theme_get_state_icone_and_info(enum connman_state state, GdkPixbuf **image, const char **info) {
    const char *icon_name = NULL;
    const char *nfo = NULL;
    switch (state) {
        case CONNMAN_STATE_OFFLINE:
            icon_name = "network-offline-symbolic";
            nfo = "Disconnected";
            break;
        case CONNMAN_STATE_READY:
            icon_name = "network-idle-symbolic";
            nfo = "Connected";
            break;
        case CONNMAN_STATE_ONLINE:
            icon_name = "network-transmit-receive-symbolic";
            nfo = "Online";
            break;
        case CONNMAN_STATE_FAILURE:
            icon_name = "network-error-symbolic";
            nfo = "Error";
            break;
        default:
            icon_name = "network-offline-symbolic";
            nfo = "Unknown";
            break;
    }
    if (image && icon_name) *image = gtk_icon_theme_load_icon(icon_theme, icon_name, 16, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
    if (info) *info = nfo;
}

// System tray with AppIndicator
static AppIndicator *cui_indicator = NULL;
static void (*popup_left_menu_f)(AppIndicator *, gpointer);
static void (*popup_right_menu_f)(AppIndicator *, guint, guint, gpointer);
static gulong left_menu_handler_id = 0;
static gulong right_menu_handler_id = 0;

void cui_trayicon_update_icon(void) {
    enum connman_state state = connman_manager_get_state();
    const char *icon_name = NULL;
    const char *info = NULL;
    cui_theme_get_state_icone_and_info(state, NULL, &info);
    switch (state) {
        case CONNMAN_STATE_OFFLINE:
        case CONNMAN_STATE_FAILURE:
            icon_name = "network-offline-symbolic";
            break;
        case CONNMAN_STATE_READY:
            icon_name = "network-idle-symbolic";
            break;
        case CONNMAN_STATE_ONLINE:
            icon_name = "network-transmit-receive-symbolic";
            break;
        default:
            icon_name = "network-offline-symbolic";
            break;
    }
    if (cui_indicator) {
        app_indicator_set_icon_full(cui_indicator, icon_name, info);
        app_indicator_set_label(cui_indicator, info, info);
    }
}

void cui_tray_hook_left_menu(gpointer callback) {
    popup_left_menu_f = callback;
}

void cui_tray_hook_right_menu(gpointer callback) {
    popup_right_menu_f = callback;
}

void cui_tray_left_menu_disable(void) {
    if (left_menu_handler_id != 0 && cui_indicator) {
        g_signal_handler_disconnect(cui_indicator, left_menu_handler_id);
        left_menu_handler_id = 0;
    }
}

void cui_tray_enable(void) {
    if (!cui_indicator || !cui_right_menu || !cui_left_menu) return;
    if (left_menu_handler_id == 0) {
        left_menu_handler_id = g_signal_connect(cui_indicator, "scroll-event", G_CALLBACK(popup_left_menu_f), NULL);
    }
    if (right_menu_handler_id == 0) {
        right_menu_handler_id = g_signal_connect(cui_indicator, "popup-menu", G_CALLBACK(popup_right_menu_f), NULL);
    }
}

void cui_tray_disable(void) {
    if (left_menu_handler_id != 0 && cui_indicator) {
        g_signal_handler_disconnect(cui_indicator, left_menu_handler_id);
        left_menu_handler_id = 0;
    }
    if (right_menu_handler_id != 0 && cui_indicator) {
        g_signal_handler_disconnect(cui_indicator, right_menu_handler_id);
        right_menu_handler_id = 0;
    }
}

gint cui_load_trayicon(GtkBuilder *builder) {
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_TRAYICON_UI_PATH, &error);
    if (error) {
        printf("Error loading tray UI: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    cui_indicator = app_indicator_new_with_path("connman-ui", "network-offline-symbolic",
                                               APP_INDICATOR_CATEGORY_SYSTEM_SERVICES, CUI_ICON_PATH);
#pragma GCC diagnostic pop
    if (!cui_indicator) {
        printf("Error: Failed to create AppIndicator\n");
        return -ENOMEM;
    }
    app_indicator_set_status(cui_indicator, APP_INDICATOR_STATUS_ACTIVE);
    cui_trayicon_update_icon();
    if (cui_load_left_menu(builder, cui_indicator) != 0 || cui_load_right_menu(builder, cui_indicator) != 0) {
        return -EINVAL;
    }
    return 0;
}

// Left menu handling
static GtkMenu *cui_more_menu = NULL;
static GtkMenuItem *cui_list_more_item = NULL;
static GHashTable *service_items = NULL;
static GtkMenuItem *cui_scan_spinner = NULL;

static void add_or_update_service(const char *path, int position) {
    GtkService *s = gtk_service_new(path);
    if (!s) return;
    if (position > 9)
        gtk_menu_shell_append(GTK_MENU_SHELL(cui_more_menu), GTK_WIDGET(s));
    else
        gtk_menu_shell_insert(GTK_MENU_SHELL(cui_left_menu), GTK_WIDGET(s), position);
    gtk_widget_set_visible(GTK_WIDGET(s), TRUE);
    gtk_widget_show(GTK_WIDGET(s));
    g_hash_table_insert(service_items, g_strdup(path), s);
}

static void remove_service_cb(const char *path) {
    g_hash_table_remove(service_items, path);
    gtk_menu_reposition(cui_left_menu);
}

static void accumulate_menu_size(GtkWidget *widget, gpointer data) {
    GtkRequisition *menu_size = (GtkRequisition *)data;
    GtkRequisition item_size;
    gtk_widget_get_preferred_size(widget, NULL, &item_size);
    menu_size->width = MAX(item_size.width, menu_size->width);
    menu_size->height += item_size.height;
}

static void get_services_cb(void *user_data) {
    GSList *services = connman_service_get_services();
    if (!services) return;
    gtk_widget_hide(GTK_WIDGET(cui_list_more_item));
    int item_position = 2;
    for (GSList *list = services; list; list = list->next, item_position++)
        add_or_update_service(list->data, item_position);
    if (item_position > 10)
        gtk_widget_show(GTK_WIDGET(cui_list_more_item));
    g_slist_free(services);
    GtkRequisition requisition = {0, 0};
    gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);
    gtk_menu_reposition(cui_left_menu);
}

static void scanning_cb(void *user_data) {
    GtkSpinner *spin = GTK_SPINNER(gtk_bin_get_child(GTK_BIN(cui_scan_spinner)));
    if (spin) {
        gtk_spinner_stop(spin);
        gtk_widget_hide(GTK_WIDGET(cui_scan_spinner));
        gtk_widget_hide(GTK_WIDGET(spin));
    }
    GtkRequisition requisition = {0, 0};
    gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);
    gtk_menu_reposition(cui_left_menu);
}

static void delete_service_item(gpointer data) {
    gtk_widget_destroy(data);
}

static void cui_popup_left_menu(AppIndicator *indicator, gpointer user_data) {
    if (!cui_left_menu) return;
    GtkSpinner *spin = GTK_SPINNER(gtk_bin_get_child(GTK_BIN(cui_scan_spinner)));
    if (spin) {
        gtk_widget_hide(GTK_WIDGET(cui_list_more_item));
        gtk_widget_show(GTK_WIDGET(cui_scan_spinner));
        gtk_widget_show(GTK_WIDGET(spin));
        gtk_spinner_start(spin);
    }
    connman_service_set_removed_callback(remove_service_cb);
    connman_service_refresh_services_list(get_services_cb, scanning_cb, user_data);
    gtk_menu_popup_at_pointer(cui_left_menu, NULL);
}

static void cui_popdown_left_menu(GtkMenu *menu, gpointer user_data) {
    connman_service_set_removed_callback(NULL);
    g_hash_table_remove_all(service_items);
    connman_service_free_services_list();
}

gint cui_load_left_menu(GtkBuilder *builder, AppIndicator *indicator) {
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_LEFT_MENU_UI_PATH, &error);
    if (error) {
        printf("Error loading left menu UI: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_left_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_left_menu"));
    cui_more_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_more_menu"));
    cui_list_more_item = GTK_MENU_ITEM(gtk_builder_get_object(builder, "cui_list_more_item"));
    cui_scan_spinner = GTK_MENU_ITEM(gtk_builder_get_object(builder, "cui_scan_spinner"));
    if (!cui_left_menu || !cui_more_menu || !cui_list_more_item || !cui_scan_spinner) {
        printf("Error: Failed to load one or more left menu widgets\n");
        return -EINVAL;
    }
    // Create a box to hold the spinner instead of adding it directly to the menu item
    GtkWidget *spinner_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(cui_scan_spinner), spinner_box);
    gtk_box_pack_start(GTK_BOX(spinner_box), gtk_spinner_new(), FALSE, FALSE, 0);
    gtk_widget_show(spinner_box);
    g_signal_connect(cui_left_menu, "deactivate", G_CALLBACK(cui_popdown_left_menu), NULL);
    service_items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, delete_service_item);
    cui_tray_hook_left_menu(cui_popup_left_menu);
    return 0;
}

// Right menu handling
static void cui_item_offlinemode_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    gboolean offlinemode = !connman_manager_get_offlinemode();
    connman_manager_set_offlinemode(offlinemode);
}

static void cui_item_quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    gtk_main_quit();
}

static void cui_popup_right_menu(AppIndicator *trayicon, guint button, guint activate_time, gpointer user_data)
{
    if (!cui_right_menu) return;
    GList *tech_list, *list;

    if (disabled == TRUE)
        goto popup;

    if (connman_manager_get_offlinemode()) {
        if (cui_item_mode_on) gtk_widget_show(cui_item_mode_on);
        if (cui_item_mode_off) gtk_widget_hide(cui_item_mode_off);
    } else {
        if (cui_item_mode_off) gtk_widget_show(cui_item_mode_off);
        if (cui_item_mode_on) gtk_widget_hide(cui_item_mode_on);
    }

    tech_list = connman_technology_get_technologies();
    if (tech_list == NULL)
        goto popup;

    for (list = tech_list; list != NULL; list = list->next)
        add_technology((const char *)list->data);

    g_list_free(tech_list);

popup:
    connman_technology_set_removed_callback(technology_removed_cb);
    connman_technology_set_added_callback(technology_added_cb);
    gtk_menu_popup_at_pointer(cui_right_menu, NULL);
}

gint cui_load_right_menu(GtkBuilder *builder, AppIndicator *indicator) {
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_RIGHT_MENU_UI_PATH, &error);
    if (error) {
        printf("Error loading right menu UI: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_right_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_right_menu"));
    cui_item_mode_off = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_off"));
    cui_item_mode_on = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_on"));
    cui_tethering_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_tethering_menu"));
    GtkMenuItem *quit_item = GTK_MENU_ITEM(gtk_builder_get_object(builder, "cui_item_quit"));
    if (!cui_right_menu || !cui_tethering_menu) {
        printf("Error: Failed to load right_menu or tethering_menu\n");
        return -EINVAL;
    }
    if (quit_item) {
        g_signal_connect(quit_item, "activate", G_CALLBACK(cui_item_quit_activate), NULL);
    } else {
        printf("Warning: quit_item not found in right_menu.ui\n");
    }
    if (cui_item_mode_off) {
        g_signal_connect(cui_item_mode_off, "activate", G_CALLBACK(cui_item_offlinemode_activate), NULL);
    } else {
        printf("Warning: offlinemode_off_item not found in right_menu.ui\n");
    }
    if (cui_item_mode_on) {
        g_signal_connect(cui_item_mode_on, "activate", G_CALLBACK(cui_item_offlinemode_activate), NULL);
    } else {
        printf("Warning: offlinemode_on_item not found in right_menu.ui\n");
    }
    tech_items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, delete_technology_item);
    app_indicator_set_menu(indicator, cui_right_menu);
    cui_tray_hook_right_menu(cui_popup_right_menu);
    return 0;
}

void cui_right_menu_enable_only_quit(void) {
    if (!cui_right_menu) return;
    GList *children = gtk_container_get_children(GTK_CONTAINER(cui_right_menu));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *item = GTK_WIDGET(l->data);
        if (g_strcmp0(gtk_widget_get_name(item), "quit_item") != 0) {
            gtk_widget_set_sensitive(item, FALSE);
        }
    }
    g_list_free(children);
    disabled = TRUE;
}

void cui_right_menu_enable_all(void) {
    if (!cui_right_menu) return;
    GList *children = gtk_container_get_children(GTK_CONTAINER(cui_right_menu));
    for (GList *l = children; l; l = l->next) {
        gtk_widget_set_sensitive(GTK_WIDGET(l->data), TRUE);
    }
    g_list_free(children);
    disabled = FALSE;
}

// Agent dialogs
gint cui_load_agent_dialogs(void) {
    GError *error = NULL;
    gtk_builder_add_from_file(cui_builder, CUI_AGENT_DIALOG_UI_PATH, &error);
    if (error) {
        printf("Error loading agent dialogs UI: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    return 0;
}

void cui_agent_init_callbacks(void) {
    // Placeholder: Initialize agent dialog callbacks
}

static struct cui_selected_service selected_service = { NULL, NULL };

void cui_agent_set_selected_service(const char *path, const char *name) {
    g_free(selected_service.path);
    g_free(selected_service.name);
    selected_service.path = g_strdup(path);
    selected_service.name = g_strdup(name);
}

void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether) {
    connman_technology_tether(path, tether);
}

// Settings popup implementation
static inline void toggle_button(const char *name) {
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, name));
    if (button == NULL) return;
    gtk_toggle_button_set_active(button, TRUE);
}

static void favorite_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (gtk_toggle_button_get_active(togglebutton) == FALSE)
        connman_service_remove(path);
}

static void autoconnect_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    connman_service_set_autoconnectable(path, gtk_toggle_button_get_active(togglebutton));
}

static void update_header(void) {
    GdkPixbuf *image = NULL;
    const char *type, *info;
    GtkWidget *widget;
    gboolean favorite;

    type = connman_service_get_type(path);
    if (g_strcmp0(type, "wifi") == 0) {
        uint8_t strength = connman_service_get_strength(path);
        cui_theme_get_signal_icone_and_info(strength, &image, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &image, &info);
    }

    set_image(builder, "service_type", image, info);
    set_label(builder, "service_name", connman_service_get_name(path), "- Hidden -");

    if (connman_service_is_connected(path) == TRUE)
        cui_theme_get_state_icone_and_info(connman_service_get_state(path), &image, &info);
    else
        image = NULL;

    set_image(builder, "service_state", image, info);
    set_label(builder, "service_error", connman_service_get_error(path), "");

    favorite = connman_service_is_favorite(path);

    set_button_toggle(builder, "service_autoconnect", connman_service_is_autoconnect(path));
    widget = set_widget_sensitive(builder, "service_autoconnect", favorite);
    if (favorite == TRUE) {
        g_signal_connect(widget, "toggled", G_CALLBACK(autoconnect_button_toggled), NULL);
    }

    set_button_toggle(builder, "service_favorite", favorite);
    widget = set_widget_sensitive(builder, "service_favorite", favorite);

    if (favorite == TRUE) {
        g_signal_connect(widget, "toggled", G_CALLBACK(favorite_button_toggled), NULL);
    }
}

static void enable_ipv4_config(gboolean enable) {
    set_widget_sensitive(builder, "ipv4_conf_address", enable);
    set_widget_sensitive(builder, "ipv4_conf_netmask", enable);
    set_widget_sensitive(builder, "ipv4_conf_gateway", enable);
}

static void set_ipv4_method(const char *method) {
    if (g_strcmp0(method, "dhcp") == 0) {
        toggle_button("ipv4_dhcp");
        enable_ipv4_config(FALSE);
    } else if (g_strcmp0(method, "manual") == 0) {
        toggle_button("ipv4_manual");
        enable_ipv4_config(TRUE);
    } else {
        toggle_button("ipv4_off");
        enable_ipv4_config(FALSE);
    }
}

static void update_ipv4(void) {
    const struct connman_ipv4 *ipv4, *ipv4_conf;
    gboolean method_set = FALSE;

    ipv4 = connman_service_get_ipv4(path);
    ipv4_conf = connman_service_get_ipv4_config(path);

    if (ipv4 == NULL) {
        set_entry(builder, "ipv4_address", "", "");
        set_entry(builder, "ipv4_netmask", "", "");
        set_entry(builder, "ipv4_gateway", "", "");
        goto config;
    }

    set_ipv4_method(ipv4->method);
    method_set = TRUE;

    set_entry(builder, "ipv4_address", ipv4->address, "");
    set_entry(builder, "ipv4_netmask", ipv4->netmask, "");
    set_entry(builder, "ipv4_gateway", ipv4->gateway, "");

config:
    if (ipv4_conf == NULL) {
        set_entry(builder, "ipv4_conf_address", "", "");
        set_entry(builder, "ipv4_conf_netmask", "", "");
        set_entry(builder, "ipv4_conf_gateway", "", "");
        if (ipv4 == NULL)
            set_widget_sensitive(builder, "ipv4_settings", FALSE);
        return;
    }

    if (method_set == FALSE)
        set_ipv4_method(ipv4_conf->method);

    set_entry(builder, "ipv4_conf_address", ipv4_conf->address, "");
    set_entry(builder, "ipv4_conf_netmask", ipv4_conf->netmask, "");
    set_entry(builder, "ipv4_conf_gateway", ipv4_conf->gateway, "");

    set_widget_sensitive(builder, "ipv4_settings", TRUE);
}

static void enable_ipv6_config(gboolean enable) {
    set_widget_sensitive(builder, "ipv6_conf_address", enable);
    set_widget_sensitive(builder, "ipv6_conf_prefix_length", enable);
    set_widget_sensitive(builder, "ipv6_conf_gateway", enable);
}

static void set_ipv6_method(const char *method) {
    gboolean privacy_enabled = FALSE;

    if (g_strcmp0(method, "auto") == 0) {
        toggle_button("ipv6_auto");
        enable_ipv6_config(FALSE);
        privacy_enabled = TRUE;
    } else if (g_strcmp0(method, "manual") == 0) {
        toggle_button("ipv6_manual");
        enable_ipv6_config(TRUE);
    } else {
        toggle_button("ipv6_off");
        enable_ipv6_config(FALSE);
    }

    set_widget_sensitive(builder, "ipv6_priv_box", privacy_enabled);
}

static void set_ipv6_privacy(const char *privacy) {
    if (g_strcmp0(privacy, "enabled") == 0)
        toggle_button("ipv6_priv_enabled");
    else if (g_strcmp0(privacy, "prefered") == 0)
        toggle_button("ipv6_priv_prefered");
    else
        toggle_button("ipv6_priv_disabled");
}

static void update_ipv6(void) {
    const struct connman_ipv6 *ipv6, *ipv6_conf;
    gboolean method_set = FALSE;
    gboolean privacy_set = FALSE;
    char value[6];

    ipv6 = connman_service_get_ipv6(path);
    ipv6_conf = connman_service_get_ipv6_config(path);

    if (ipv6 == NULL) {
        set_entry(builder, "ipv6_address", "", "");
        set_entry(builder, "ipv6_prefix_length", "", "");
        set_entry(builder, "ipv6_gateway", "", "");
        goto config;
    }

    memset(value, 0, 6);
    snprintf(value, 6, "%u", ipv6->prefix);

    set_ipv6_method(ipv6->method);
    method_set = TRUE;

    set_label(builder, "ipv6_address", ipv6->address, "");
    set_label(builder, "ipv6_prefix_length", value, "");
    set_label(builder, "ipv6_gateway", ipv6->gateway, "");

    set_ipv6_privacy(ipv6->privacy);
    privacy_set = TRUE;

config:
    if (ipv6_conf == NULL) {
        set_entry(builder, "ipv6_conf_address", "", "");
        set_entry(builder, "ipv6_conf_prefix_length", "", "");
        set_entry(builder, "ipv6_conf_gateway", "", "");
        if (ipv6 == NULL)
            set_widget_sensitive(builder, "ipv6_settings", FALSE);
        return;
    }

    memset(value, 0, 6);
    snprintf(value, 6, "%u", ipv6_conf->prefix);

    if (method_set == FALSE)
        set_ipv6_method(ipv6_conf->method);

    set_entry(builder, "ipv6_conf_address", ipv6_conf->address, "");
    set_entry(builder, "ipv6_conf_prefix_length", value, "");
    set_entry(builder, "ipv6_conf_gateway", ipv6_conf->gateway, "");

    if (privacy_set == FALSE)
        set_ipv6_privacy(ipv6_conf->privacy);
}

static void update_dns(void) {
    const char *dns, *dns_config, *domains, *domains_config;

    dns = connman_service_get_nameservers(path);
    dns_config = connman_service_get_nameservers_config(path);
    domains = connman_service_get_domains(path);
    domains_config = connman_service_get_domains_config(path);

    set_entry(builder, "nameservers", dns, "");
    set_entry(builder, "nameservers_conf", dns_config, "");
    set_entry(builder, "domains", domains, "");
    set_entry(builder, "domains_conf", domains_config, "");
}

static void update_timeservers(void) {
    set_entry(builder, "timerservers", connman_service_get_timeservers(path), "");
    set_entry(builder, "timerservers_conf", connman_service_get_timeservers_config(path), "");
}

static void enable_proxy_config(gboolean enable) {
    set_widget_sensitive(builder, "proxy_conf_url", enable);
    set_widget_sensitive(builder, "proxy_conf_servers", enable);
    set_widget_sensitive(builder, "proxy_conf_excludes", enable);
}

static void set_proxy_method(const char *method) {
    if (g_strcmp0(method, "auto") == 0) {
        toggle_button("proxy_auto");
        enable_proxy_config(FALSE);
    } else if (g_strcmp0(method, "direct") == 0) {
        toggle_button("proxy_direct");
        enable_proxy_config(FALSE);
    } else {
        toggle_button("proxy_manual");
        enable_proxy_config(TRUE);
    }
}

static void update_proxy(void) {
    const struct connman_proxy *proxy, *proxy_conf;
    gboolean method_set = FALSE;

    proxy = connman_service_get_proxy(path);
    proxy_conf = connman_service_get_proxy_config(path);

    if (proxy == NULL) {
        set_label(builder, "proxy_method", "", "");
        set_entry(builder, "proxy_url", "", "");
        set_entry(builder, "proxy_servers", "", "");
        set_entry(builder, "proxy_excludes", "", "");
        goto config;
    }

    set_proxy_method(proxy->method);
    method_set = TRUE;

    set_entry(builder, "proxy_url", proxy->url, "");
    set_entry(builder, "proxy_servers", proxy->servers, "");
    set_entry(builder, "proxy_excludes", proxy->excludes, "");

config:
    if (proxy_conf == NULL) {
        set_entry(builder, "proxy_conf_url", "", "");
        set_entry(builder, "proxy_conf_servers", "", "");
        set_entry(builder, "proxy_conf_excludes", "", "");
        if (proxy == NULL)
            set_widget_sensitive(builder, "proxy_settings", FALSE);
        return;
    }

    if (method_set == FALSE)
        set_proxy_method(proxy_conf->method);

    set_entry(builder, "proxy_conf_url", proxy_conf->url, "");
    set_entry(builder, "proxy_conf_servers", proxy_conf->servers, "");
    set_entry(builder, "proxy_conf_excludes", proxy_conf->excludes, "");

    set_widget_sensitive(builder, "proxy_settings", TRUE);
}

static void update_provider(void) {
    const struct connman_provider *provider;

    provider = connman_service_get_provider(path);

    if (provider == NULL) {
        set_label(builder, "provider_host", "", "");
        set_label(builder, "provider_domain", "", "");
        set_label(builder, "provider_name", "", "");
        set_label(builder, "provider_type", "", "");
        set_widget_sensitive(builder, "provider_settings", FALSE);
        return;
    }

    set_label(builder, "provider_host", provider->host, "");
    set_label(builder, "provider_domain", provider->domain, "");
    set_label(builder, "provider_name", provider->name, "");
    set_label(builder, "provider_type", provider->type, "");

    set_widget_sensitive(builder, "provider_settings", TRUE);
}

static void update_ethernet(void) {
    const struct connman_ethernet *ethernet;
    char value[6];

    ethernet = connman_service_get_ethernet(path);

    if (ethernet == NULL) {
        set_label(builder, "ethernet_method", "", "");
        set_label(builder, "ethernet_interface", "", "");
        set_label(builder, "ethernet_address", "", "");
        set_label(builder, "ethernet_mtu", "0", "");
        set_label(builder, "ethernet_speed", "0", "");
        set_label(builder, "ethernet_duplex", "", "");
        set_widget_sensitive(builder, "ethernet_settings", FALSE);
        return;
    }

    set_label(builder, "ethernet_method", ethernet->method, "");
    set_label(builder, "ethernet_interface", ethernet->interface, "");
    set_label(builder, "ethernet_address", ethernet->address, "");

    memset(value, 0, 6);
    snprintf(value, 6, "%u", ethernet->mtu);
    set_label(builder, "ethernet_mtu", value, "");

    memset(value, 0, 6);
    snprintf(value, 6, "%u", ethernet->speed);
    set_label(builder, "ethernet_speed", value, "");
    set_label(builder, "ethernet_duplex", ethernet->duplex, "");

    set_widget_sensitive(builder, "ethernet_settings", TRUE);
}

static void service_property_set_cb(const char *path, const char *property, void *user_data) {
    update_header();
    update_ipv4();
    update_ipv6();
    update_dns();
    update_timeservers();
    update_proxy();
    update_provider();
    update_ethernet();

    ipv4_changed = FALSE;
    ipv6_changed = FALSE;
    proxy_changed = FALSE;
    nameservers_changed = FALSE;
    domains_changed = FALSE;
    timeservers_changed = FALSE;

    connman_service_set_property_changed_callback(path, service_property_changed_cb, NULL);
}

static void service_property_error_cb(const char *path, const char *property, int error, void *user_data) {
    printf("Error setting %s (%d)\n", property, error);
    service_property_set_cb(NULL, NULL, NULL);
}

static void toggled_ipv4_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *name = user_data;
    gboolean enable_config = FALSE;

    if (gtk_toggle_button_get_active(togglebutton) == FALSE)
        return;

    if (g_strcmp0(name, "ipv4_manual") == 0) {
        ipv4_method = "manual";
        enable_config = TRUE;
    } else if (g_strcmp0(name, "ipv4_dhcp") == 0)
        ipv4_method = "dhcp";
    else
        ipv4_method = "off";

    enable_ipv4_config(enable_config);
    ipv4_changed = TRUE;
}

static void toggled_ipv6_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *name = user_data;
    gboolean enable_config = FALSE;
    gboolean enable_privacy = FALSE;

    if (gtk_toggle_button_get_active(togglebutton) == FALSE)
        return;

    if (g_strcmp0(name, "ipv6_manual") == 0) {
        ipv6_method = "manual";
        enable_config = TRUE;
    } else if (g_strcmp0(name, "ipv6_auto") == 0) {
        ipv6_method = "auto";
        enable_privacy = TRUE;
    } else
        ipv6_method = "off";

    enable_ipv6_config(enable_config);
    set_widget_sensitive(builder, "ipv6_priv_box", enable_privacy);
    ipv6_changed = TRUE;
}

static void toggled_ipv6_privacy_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *name = user_data;

    if (g_strcmp0(name, "ipv6_priv_enabled") == 0)
        ipv6_privacy = "enabled";
    else if (g_strcmp0(name, "ipv6_priv_disabled") == 0)
        ipv6_privacy = "disabled";
    else
        ipv6_privacy = "prefered";

    ipv6_changed = TRUE;
}

static void toggled_proxy_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *name = user_data;
    gboolean enable_config = FALSE;

    if (gtk_toggle_button_get_active(togglebutton) == FALSE)
        return;

    if (g_strcmp0(name, "proxy_manual") == 0) {
        proxy_method = "manual";
        enable_config = TRUE;
    } else if (g_strcmp0(name, "proxy_direct") == 0)
        proxy_method = "direct";
    else
        proxy_method = "auto";

    enable_proxy_config(enable_config);
    proxy_changed = TRUE;
}

static gboolean key_released_entry_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    gboolean *value = user_data;
    *value = TRUE;
    return TRUE;
}

static void get_ipv4_configuration(struct connman_ipv4 *ipv4) {
    ipv4->method = (char *)ipv4_method;
    ipv4->address = (char *)get_entry_text(builder, "ipv4_conf_address");
    ipv4->netmask = (char *)get_entry_text(builder, "ipv4_conf_netmask");
    ipv4->gateway = (char *)get_entry_text(builder, "ipv4_conf_gateway");
}

static void get_ipv6_configuration(struct connman_ipv6 *ipv6) {
    const char *prefix = get_entry_text(builder, "ipv6_conf_prefix_length");
    ipv6->method = (char *)ipv6_method;
    ipv6->address = (char *)get_entry_text(builder, "ipv6_conf_address");
    ipv6->prefix = atoi(prefix);
    ipv6->gateway = (char *)get_entry_text(builder, "ipv6_conf_gateway");
    ipv6->privacy = (char *)ipv6_privacy;
}

static void get_proxy_configuration(struct connman_proxy *proxy) {
    proxy->method = (char *)proxy_method;
    proxy->url = (char *)get_entry_text(builder, "proxy_conf_url");
    proxy->servers = (char *)get_entry_text(builder, "proxy_conf_servers");
    proxy->excludes = (char *)get_entry_text(builder, "proxy_conf_excludes");
}

static void settings_cancel_callback(GtkButton *button, gpointer user_data) {
    gtk_widget_destroy(GTK_WIDGET(service_settings_dbox));
    service_settings_dbox = NULL;

    connman_service_set_property_changed_callback(path, NULL, NULL);
    connman_service_set_property_error_callback(path, NULL, NULL);

    connman_service_deselect();

    g_free(path);
    path = NULL;

    cui_tray_enable();
}

static void settings_ok_callback(GtkButton *button, gpointer user_data) {
    const char *value;
    int ret = 0;

    connman_service_set_property_changed_callback(path, service_property_set_cb, NULL);

    if (ipv4_changed == TRUE) {
        struct connman_ipv4 ipv4;
        get_ipv4_configuration(&ipv4);
        ret = connman_service_set_ipv4_config(path, &ipv4);
    } else if (ipv6_changed == TRUE) {
        struct connman_ipv6 ipv6;
        get_ipv6_configuration(&ipv6);
        ret = connman_service_set_ipv6_config(path, &ipv6);
    } else if (proxy_changed == TRUE) {
        struct connman_proxy proxy;
        get_proxy_configuration(&proxy);
        ret = connman_service_set_proxy_config(path, &proxy);
    } else if (nameservers_changed == TRUE) {
        value = get_entry_text(builder, "nameservers_conf");
        ret = connman_service_set_nameservers_config(path, value);
    } else if (domains_changed == TRUE) {
        value = get_entry_text(builder, "domains_conf");
        ret = connman_service_set_domains_config(path, value);
    } else if (timeservers_changed == TRUE) {
        value = get_entry_text(builder, "timerservers_conf");
        ret = connman_service_set_timeservers_config(path, value);
    }

    if (ret != 0)
        printf("Unable to set property, code %d\n", ret);
}

static void settings_close_callback(GtkDialog *dialog_box, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_DELETE_EVENT || response_id == GTK_RESPONSE_CLOSE)
        settings_cancel_callback(NULL, NULL);
}

static void settings_connect_signals(void) {
    set_signal_callback(builder, "ipv4_dhcp", "toggled", G_CALLBACK(toggled_ipv4_method_cb), "ipv4_dhcp");
    set_signal_callback(builder, "ipv4_manual", "toggled", G_CALLBACK(toggled_ipv4_method_cb), "ipv4_manual");
    set_signal_callback(builder, "ipv4_off", "toggled", G_CALLBACK(toggled_ipv4_method_cb), "ipv4_off");

    set_signal_callback(builder, "ipv6_auto", "toggled", G_CALLBACK(toggled_ipv6_method_cb), "ipv6_auto");
    set_signal_callback(builder, "ipv6_manual", "toggled", G_CALLBACK(toggled_ipv6_method_cb), "ipv6_manual");
    set_signal_callback(builder, "ipv6_off", "toggled", G_CALLBACK(toggled_ipv6_method_cb), "ipv6_off");

    set_signal_callback(builder, "ipv4_conf_address", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv4_changed);
    set_signal_callback(builder, "ipv4_conf_netmask", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv4_changed);
    set_signal_callback(builder, "ipv4_conf_gateway", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv4_changed);

    set_signal_callback(builder, "ipv6_conf_address", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv6_changed);
    set_signal_callback(builder, "ipv6_conf_prefix_length", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv6_changed);
    set_signal_callback(builder, "ipv6_conf_gateway", "key-release-event", G_CALLBACK(key_released_entry_cb), &ipv6_changed);

    set_signal_callback(builder, "ipv6_priv_prefered", "toggled", G_CALLBACK(toggled_ipv6_privacy_cb), "ipv6_priv_prefered");
    set_signal_callback(builder, "ipv6_priv_enabled", "toggled", G_CALLBACK(toggled_ipv6_privacy_cb), "ipv6_priv_enabled");
    set_signal_callback(builder, "ipv6_priv_disabled", "toggled", G_CALLBACK(toggled_ipv6_privacy_cb), "ipv6_priv_disabled");

    set_signal_callback(builder, "nameservers_conf", "key-release-event", G_CALLBACK(key_released_entry_cb), &nameservers_changed);
    set_signal_callback(builder, "domains_conf", "key-release-event", G_CALLBACK(key_released_entry_cb), &domains_changed);
    set_signal_callback(builder, "timerservers_conf", "key-release-event", G_CALLBACK(key_released_entry_cb), &timeservers_changed);

    set_signal_callback(builder, "proxy_auto", "toggled", G_CALLBACK(toggled_proxy_method_cb), "proxy_auto");
    set_signal_callback(builder, "proxy_direct", "toggled", G_CALLBACK(toggled_proxy_method_cb), "proxy_direct");
    set_signal_callback(builder, "proxy_manual", "toggled", G_CALLBACK(toggled_proxy_method_cb), "proxy_manual");

    set_signal_callback(builder, "proxy_conf_url", "key-release-event", G_CALLBACK(key_released_entry_cb), &proxy_changed);
    set_signal_callback(builder, "proxy_conf_servers", "key-release-event", G_CALLBACK(key_released_entry_cb), &proxy_changed);
    set_signal_callback(builder, "proxy_conf_excludes", "key-release-event", G_CALLBACK(key_released_entry_cb), &proxy_changed);

    set_signal_callback(builder, "settings_ok", "clicked", G_CALLBACK(settings_ok_callback), NULL);
    set_signal_callback(builder, "settings_close", "clicked", G_CALLBACK(settings_cancel_callback), NULL);

    set_signal_callback(builder, "service_settings_dbox", "response", G_CALLBACK(settings_close_callback), NULL);
}

gint cui_settings_popup(const char *selected_path) {
    GError *error = NULL;

    if (builder == NULL)
        builder = gtk_builder_new();

    if (builder == NULL)
        return -ENOMEM;

    gtk_builder_add_from_file(builder, CUI_SETTINGS_UI_PATH, &error);
    if (error != NULL) {
        printf("Error: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }

    connman_service_select(selected_path);

    g_free(path);
    path = g_strdup(selected_path);

    service_settings_dbox = GTK_DIALOG(gtk_builder_get_object(builder, "service_settings_dbox"));

    gtk_widget_show(GTK_WIDGET(service_settings_dbox));

    connman_service_set_property_changed_callback(path, service_property_changed_cb, NULL);
    connman_service_set_property_error_callback(path, service_property_error_cb, NULL);

    settings_connect_signals();

    service_property_set_cb(NULL, NULL, NULL);

    cui_tray_disable();

    return 0;
}

// Utility functions
GtkLabel *set_label(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, name));
    if (!label) return NULL;
    gtk_label_set_text(label, value ? value : default_value);
    return label;
}

GtkEntry *set_entry(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, name));
    if (!entry) return NULL;
    gtk_entry_set_text(entry, value ? value : default_value);
    return entry;
}

GtkWidget *set_widget_sensitive(GtkBuilder *builder, const char *name, gboolean value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) return NULL;
    gtk_widget_set_sensitive(widget, value);
    return widget;
}

GtkWidget *set_widget_hidden(GtkBuilder *builder, const char *name, gboolean value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) return NULL;
    gtk_widget_set_visible(widget, !value);
    return widget;
}

GtkWidget *set_button_toggle(GtkBuilder *builder, const char *name, gboolean active) {
    GtkWidget *button = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!button) return NULL;
    if (GTK_IS_TOGGLE_BUTTON(button)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), active);
    }
    return button;
}

GtkImage *set_image(GtkBuilder *builder, const char *name, GdkPixbuf *image, const char *info) {
    GtkImage *img_widget = GTK_IMAGE(gtk_builder_get_object(builder, name));
    if (!img_widget) return NULL;
    if (image) {
        gtk_image_set_from_pixbuf(img_widget, image);
        gtk_widget_set_visible(GTK_WIDGET(img_widget), TRUE);
        if (info) gtk_widget_set_tooltip_text(GTK_WIDGET(img_widget), info);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(img_widget), FALSE);
    }
    return img_widget;
}

void set_signal_callback(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler, gpointer user_data) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (widget) {
        g_signal_connect(widget, signal_name, handler, user_data);
    }
}

const char *get_entry_text(GtkBuilder *builder, const char *name) {
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, name));
    return entry ? gtk_entry_get_text(entry) : NULL;
}

// ConnMan callbacks
static void connman_manager_changed(const char *unused, const char *property, void *user_data) {
    if (g_strcmp0(property, "State") == 0) {
        cui_trayicon_update_icon();
    }
}

static void connman_up(void *user_data) {
    cui_tray_enable();
    connman_technology_set_added_callback(technology_added_cb);
    connman_technology_set_removed_callback(technology_removed_cb);
    GList *technologies = connman_technology_get_technologies();
    for (GList *l = technologies; l; l = l->next) {
        add_technology(l->data);
    }
    g_list_free(technologies);
}

static void connman_down(void *user_data) {
    cui_tray_disable();
    g_hash_table_remove_all(service_items);
    g_hash_table_remove_all(tech_items);
}

// Main function
int main(int argc, char *argv[]) {
    int ret;
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    gtk_init(&argc, &argv);
    cui_builder = gtk_builder_new();
    if (!cui_builder) return -ENOMEM;
    cui_load_theme();
    if (cui_load_trayicon(cui_builder) != 0 || cui_load_agent_dialogs() != 0)
        return -EINVAL;
    cui_tray_enable();
    ret = connman_interface_init(connman_up, connman_down, NULL);
    if (ret < 0) return ret;
    gtk_main();
    connman_interface_finalize();
    if (cui_builder) g_object_unref(cui_builder);
    if (builder) g_object_unref(builder);
    if (tech_items) g_hash_table_destroy(tech_items);
    if (service_items) g_hash_table_destroy(service_items);
    return 0;
}
