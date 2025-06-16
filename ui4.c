// gcc `pkg-config --cflags --libs gtk4 glib-2.0 dbus-1 ayatana-appindicator3-0.1` -o connman-ui ui.c gdbus/client.c gdbus/mainloop.c gdbus/object.c gdbus/polkit.c gdbus/watch.c -I. lib/agent.c lib/dbus.c lib/interface.c lib/manager.c lib/service.c lib/technology.c -Ilib/
// dnf install libayatana-appindicator-gtk3-devel

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <libintl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libayatana-appindicator/app-indicator.h>

#include "connman-interface.h"

#define CUI_UI_PATH "/usr/share/connman_ui_gtk"
#define CUI_ICON_PATH CUI_UI_PATH "/icons"
#define CUI_AGENT_DIALOG_UI_PATH CUI_UI_PATH "/ui/agent.ui"
#define CUI_LEFT_MENU_UI_PATH CUI_UI_PATH "/ui/left_menu.ui"
#define CUI_RIGHT_MENU_UI_PATH CUI_UI_PATH "/ui/right_menu.ui"
#define CUI_SETTINGS_UI_PATH CUI_UI_PATH "/ui/settings.ui"
#define CUI_TRAYICON_UI_PATH CUI_UI_PATH "/ui/tray.ui"

// GtkService definition
typedef struct _GtkService GtkService;
typedef struct _GtkServiceClass GtkServiceClass;
typedef struct _GtkServicePrivate GtkServicePrivate;

struct _GtkServicePrivate {
    GtkBox *box;
    GtkLabel *name;
    GtkImage *state;
    GtkImage *security;
    GtkImage *signal;
    gboolean selected;
};

struct _GtkService {
    GtkButton parent;
    GtkServicePrivate *priv;
    gchar *path;
};

struct _GtkServiceClass {
    GtkButtonClass parent_class;
};

GType gtk_service_get_type(void);
G_DEFINE_TYPE_WITH_PRIVATE(GtkService, gtk_service, GTK_TYPE_BUTTON)

static void gtk_service_init(GtkService *service) {
    service->priv = gtk_service_get_instance_private(service);
    service->priv->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    service->priv->name = GTK_LABEL(gtk_label_new(NULL));
    service->priv->state = GTK_IMAGE(gtk_image_new());
    service->priv->security = GTK_IMAGE(gtk_image_new());
    service->priv->signal = GTK_IMAGE(gtk_image_new());
    service->priv->selected = FALSE;
    gtk_widget_set_halign(GTK_WIDGET(service->priv->name), GTK_ALIGN_START);
    gtk_box_append(service->priv->box, GTK_WIDGET(service->priv->name));
    gtk_box_append(service->priv->box, GTK_WIDGET(service->priv->state));
    gtk_box_append(service->priv->box, GTK_WIDGET(service->priv->security));
    gtk_box_append(service->priv->box, GTK_WIDGET(service->priv->signal));
    gtk_button_set_child(GTK_BUTTON(service), GTK_WIDGET(service->priv->box));
}

static void gtk_service_dispose(GObject *object) {
    GtkService *service = G_TYPE_CHECK_INSTANCE_CAST(object, gtk_service_get_type(), GtkService);
    if (!service->priv->selected) {
        connman_service_set_property_changed_callback(service->path, NULL, NULL);
    }
    g_free(service->path);
    G_OBJECT_CLASS(gtk_service_parent_class)->dispose(object);
}

static void gtk_service_class_init(GtkServiceClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = gtk_service_dispose;
}

static void service_set_name(GtkService *service);
static void service_set_state(GtkService *service);
static void service_set_signal(GtkService *service);
static void cui_settings_popup(const char *service_path);

static GtkBuilder *cui_builder = NULL;
static AppIndicator *cui_trayicon = NULL;
static GtkWidget *cui_left_menu = NULL;
static GtkWidget *cui_more_menu = NULL;
static GtkWidget *cui_list_more_item = NULL;
static GHashTable *service_items = NULL;
static GtkWidget *cui_scan_spinner = NULL;
static GtkWidget *cui_right_menu = NULL;
static GtkWidget *cui_item_mode_off = NULL;
static GtkWidget *cui_item_mode_on = NULL;
static GtkWidget *cui_tethering_menu = NULL;
static GHashTable *tech_items = NULL;
static int item_position = 3;
static gboolean disabled = TRUE;
static GtkWidget *input_dbox = NULL;
static GtkWidget *login_dbox = NULL;
static GtkWidget *error_dbox = NULL;
static GtkWidget *tethering_dbox = NULL;
static struct cui_selected_service {
    char *path;
    char *name;
} service = {0};
static char *technology = NULL;
static gboolean tethering = FALSE;
static GtkWidget *service_settings_dbox = NULL;
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
static GtkIconTheme *icon_theme = NULL;

static void (*popup_left_menu_f)(GtkWidget *, gpointer);
static void (*popup_right_menu_f)(GtkWidget *, guint, guint, gpointer);
static int left_menu_handler_id = 0;
static int right_menu_handler_id = 0;

// Utility functions
static GtkLabel *set_label(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!GTK_IS_LABEL(widget)) return NULL;
    gtk_label_set_text(GTK_LABEL(widget), value ? value : default_value);
    return GTK_LABEL(widget);
}

static GtkEntry *set_entry(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!GTK_IS_ENTRY(widget)) return NULL;
    g_object_set(widget, "text", value ? value : default_value, NULL);
    return GTK_ENTRY(widget);
}

static GtkWidget *set_widget_sensitive(GtkBuilder *builder, const char *name, gboolean value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) return NULL;
    gtk_widget_set_sensitive(widget, value);
    return widget;
}

static GtkWidget *set_widget_hidden(GtkBuilder *builder, const char *name, gboolean value) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) return NULL;
    gtk_widget_set_visible(widget, !value);
    return widget;
}

static GtkWidget *set_button_toggle(GtkBuilder *builder, const char *name, gboolean active) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!GTK_IS_TOGGLE_BUTTON(widget)) return NULL;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), active);
    return widget;
}

static GtkImage *set_image(GtkBuilder *builder, const char *name, GdkPaintable *paintable, const char *info) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!GTK_IS_IMAGE(widget)) return NULL;
    if (paintable) {
        gtk_image_set_from_paintable(GTK_IMAGE(widget), paintable);
        gtk_widget_set_visible(widget, TRUE);
        gtk_widget_set_tooltip_text(widget, info);
    } else {
        gtk_widget_set_visible(widget, FALSE);
    }
    return GTK_IMAGE(widget);
}

static void set_signal_callback(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler, gpointer user_data) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (widget) g_signal_connect(widget, signal_name, handler, user_data);
}

static const char *get_entry_text(GtkBuilder *builder, const char *name) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!GTK_IS_ENTRY(widget)) return NULL;
    return gtk_editable_get_text(GTK_EDITABLE(widget));
}

// Theme functions
static void cui_theme_get_tethering_icone_and_info(GdkPaintable **paintable, const char **info) {
    if (paintable) {
        *paintable = GDK_PAINTABLE(gtk_icon_theme_lookup_icon(icon_theme, "nm-adhoc", NULL, 24, 1, GTK_TEXT_DIR_NONE, 0));
    }
    if (info) *info = _("Tethering");
}

static void cui_theme_get_type_icone_and_info(const char *type, GdkPaintable **paintable, const char **info) {
    const char *nfo = NULL;
    const char *icon_name = NULL;
    if (g_strcmp0(type, "ethernet") == 0) {
        icon_name = "network-wired-symbolic";
        nfo = _("Ethernet");
    } else if (g_strcmp0(type, "cellular") == 0) {
        icon_name = "gsm-3g-full";
        nfo = _("Cellular");
    } else if (g_strcmp0(type, "wifi") == 0) {
        icon_name = "network-wireless-symbolic";
        nfo = _("Wi-Fi");
    }
    if (paintable && icon_name) {
        *paintable = GDK_PAINTABLE(gtk_icon_theme_lookup_icon(icon_theme, icon_name, NULL, 24, 1, GTK_TEXT_DIR_NONE, 0));
    }
    if (info) *info = nfo;
}

static void cui_theme_get_signal_icone_and_info(uint8_t signal_strength, GdkPaintable **paintable, const char **info) {
    const char *nfo = NULL;
    const char *icon_name = NULL;
    if (signal_strength >= 80) {
        icon_name = "network-wireless-signal-excellent-symbolic";
        nfo = _("Very good signal");
    } else if (signal_strength >= 60) {
        icon_name = "network-wireless-signal-good-symbolic";
        nfo = _("Good signal");
    } else if (signal_strength >= 40) {
        icon_name = "network-wireless-signal-ok-symbolic";
        nfo = _("Low signal");
    } else {
        icon_name = "network-wireless-signal-weak-symbolic";
        nfo = _("Very low signal");
    }
    if (paintable && icon_name) {
        *paintable = GDK_PAINTABLE(gtk_icon_theme_lookup_icon(icon_theme, icon_name, NULL, 24, 1, GTK_TEXT_DIR_NONE, 0));
    }
    if (info) *info = nfo;
}

static void cui_theme_get_state_icone_and_info(enum connman_state state, GdkPaintable **paintable, const char **info) {
    const char *nfo = NULL;
    const char *icon_name = NULL;
    switch (state) {
        case CONNMAN_STATE_UNKNOWN:
        case CONNMAN_STATE_OFFLINE:
        case CONNMAN_STATE_FAILURE:
            icon_name = "network-offline-symbolic";
            nfo = _("Disconnected");
            break;
        case CONNMAN_STATE_IDLE:
            icon_name = "network-offline-symbolic";
            nfo = _("Connman is not running");
            break;
        case CONNMAN_STATE_READY:
            icon_name = "network-idle-symbolic";
            nfo = _("Connected");
            break;
        case CONNMAN_STATE_ONLINE:
            icon_name = "network-transmit-receive-symbolic";
            nfo = _("Online");
            break;
    }
    if (paintable && icon_name) {
        *paintable = GDK_PAINTABLE(gtk_icon_theme_lookup_icon(icon_theme, icon_name, NULL, 24, 1, GTK_TEXT_DIR_NONE, 0));
    }
    if (info) *info = nfo;
}

static void cui_load_theme(void) {
    icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
    gtk_icon_theme_add_resource_path(icon_theme, CUI_ICON_PATH);
}

// Tray icon handling with libappindicator
static void cui_trayicon_update_icon(void) {
    enum connman_state state = connman_manager_get_state();
    const char *icon_name = NULL;
    switch (state) {
        case CONNMAN_STATE_UNKNOWN:
        case CONNMAN_STATE_OFFLINE:
        case CONNMAN_STATE_FAILURE:
            icon_name = "network-offline-symbolic";
            break;
        case CONNMAN_STATE_IDLE:
            icon_name = "network-offline-symbolic";
            break;
        case CONNMAN_STATE_READY:
            icon_name = "network-idle-symbolic";
            break;
        case CONNMAN_STATE_ONLINE:
            icon_name = "network-transmit-receive-symbolic";
            break;
    }
    if (cui_trayicon && icon_name) {
        app_indicator_set_icon_full(cui_trayicon, icon_name, "Connman UI");
        app_indicator_set_status(cui_trayicon, APP_INDICATOR_STATUS_ACTIVE);
    }
}

static void cui_tray_hook_left_menu(gpointer callback) {
    popup_left_menu_f = callback;
}

static void cui_tray_hook_right_menu(gpointer callback) {
    popup_right_menu_f = callback;
}

static void cui_tray_left_menu_disable(void) {
    // No signal disconnect needed for AppIndicator; menus handled by popup functions
}

static void cui_tray_enable(void) {
    if (cui_trayicon) {
        app_indicator_set_status(cui_trayicon, APP_INDICATOR_STATUS_ACTIVE);
    }
}

static void cui_tray_disable(void) {
    if (cui_trayicon) {
        app_indicator_set_status(cui_trayicon, APP_INDICATOR_STATUS_PASSIVE);
    }
}

static gint cui_load_trayicon(GtkBuilder *builder) {
    cui_trayicon = app_indicator_new("connman-gtk", "network-offline-symbolic", APP_INDICATOR_CATEGORY_SYSTEM_SERVICES);
    if (!cui_trayicon) {
        g_printerr("Failed to create AppIndicator\n");
        return -EINVAL;
    }
    cui_trayicon_update_icon();
    // Menus will be attached after loading left_menu.ui and right_menu.ui
    return 0;
}

// Agent dialogs
static void agent_popup_error_dbox(const char *error) {
    set_label(cui_builder, "error_message", error, "");
    gtk_widget_set_visible(error_dbox, TRUE);
    cui_tray_disable();
}

static void agent_error_cb(const char *path, const char *error) {
    agent_popup_error_dbox(error);
}

static void agent_browser_cb(const char *path, const char *url) {
    g_printerr("Launching browser for URL: %s\n", url);
}

static void agent_popup_input_dbox(gboolean hidden, gboolean passphrase, const char *previous_passphrase, gboolean wpspin, const char *previous_wpspin) {
    set_label(cui_builder, "title_label", service.name, "");
    set_widget_hidden(cui_builder, "name_button", !hidden);
    set_widget_hidden(cui_builder, "ssid_button", !hidden);
    set_entry(cui_builder, "hidden_entry", "", "");
    set_button_toggle(cui_builder, "passphrase_button", TRUE);
    GtkEntry *entry = set_entry(cui_builder, "secret_entry", "", "");
    g_object_set(entry, "visibility", FALSE, NULL);
    set_widget_sensitive(cui_builder, "input_ok", FALSE);
    set_widget_hidden(cui_builder, "wpsbc_button", !wpspin);
    set_button_toggle(cui_builder, "wpsbc_button", FALSE);
    set_widget_hidden(cui_builder, "wpspin_button", !wpspin);
    set_button_toggle(cui_builder, "wpspin_button", FALSE);
    set_label(cui_builder, "previous_label", previous_passphrase, "");
    gtk_widget_set_visible(input_dbox, TRUE);
    cui_tray_disable();
}

static void agent_popup_login_dbox(void) {
    set_entry(cui_builder, "login_username", "", "");
    GtkEntry *entry = set_entry(cui_builder, "login_password", "", "");
    g_object_set(entry, "visibility", FALSE, NULL);
    gtk_widget_set_visible(login_dbox, TRUE);
    cui_tray_disable();
}

static void agent_input_cb(const char *path, gboolean hidden, gboolean identity, gboolean passphrase, const char *previous_passphrase, gboolean wpspin, const char *previous_wpspin, gboolean login) {
    if (g_strcmp0(path, service.path) != 0) {
        connman_agent_reply_canceled();
        return;
    }
    if (passphrase || hidden)
        agent_popup_input_dbox(hidden, passphrase, previous_passphrase, wpspin, previous_wpspin);
    else if (login)
        agent_popup_login_dbox();
    else
        connman_agent_reply_canceled();
}

static void agent_cancel_cb(void) {
    gtk_widget_set_visible(input_dbox, FALSE);
    gtk_widget_set_visible(login_dbox, FALSE);
    gtk_widget_set_visible(error_dbox, FALSE);
    gtk_widget_set_visible(tethering_dbox, FALSE);
    cui_tray_enable();
}

static void agent_ok_input(GtkButton *button, gpointer user_data) {
    const char *name = NULL, *passphrase = NULL, *wpspin = NULL;
    gboolean wps = FALSE;
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "hidden_entry"));
    if (gtk_entry_get_text_length(entry) > 0)
        name = gtk_editable_get_text(GTK_EDITABLE(entry));
    entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "secret_entry"));
    GtkToggleButton *toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "passphrase_button"));
    if (gtk_toggle_button_get_active(toggle))
        passphrase = gtk_editable_get_text(GTK_EDITABLE(entry));
    toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "wpsbc_button"));
    if (gtk_toggle_button_get_active(toggle)) {
        wps = TRUE;
        wpspin = "";
    }
    toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "wpspin_button"));
    if (gtk_toggle_button_get_active(toggle)) {
        wps = TRUE;
        wpspin = gtk_editable_get_text(GTK_EDITABLE(entry));
    }
    connman_agent_reply_passphrase(name, passphrase, wps, wpspin);
    gtk_widget_set_visible(input_dbox, FALSE);
    cui_tray_enable();
}

static void agent_ok_login(GtkButton *button, gpointer user_data) {
    const char *username = get_entry_text(cui_builder, "login_username");
    const char *password = get_entry_text(cui_builder, "login_password");
    connman_agent_reply_login(username, password);
    gtk_widget_set_visible(login_dbox, FALSE);
    cui_tray_enable();
}

static void agent_cancel_button(GtkButton *button, gpointer user_data) {
    gtk_widget_set_visible(GTK_WIDGET(user_data), FALSE);
    agent_cancel_cb();
}

static void agent_close(GtkDialog *dialog, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_DELETE_EVENT || response_id == GTK_RESPONSE_CLOSE)
        agent_cancel_cb();
}

static void agent_retry(GtkButton *button, gpointer user_data) {
    gtk_widget_set_visible(error_dbox, FALSE);
    cui_tray_enable();
    connman_agent_reply_retry();
}

static void agent_passphrase_changed(GtkToggleButton *togglebutton, gpointer user_data) {
    GtkWidget *entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "secret_entry"));
    const char *label = gtk_button_get_label(GTK_BUTTON(togglebutton));
    if (g_strcmp0(label, "WPS Push-Button") == 0) {
        gtk_widget_set_sensitive(entry, FALSE);
        set_widget_sensitive(cui_builder, "input_ok", TRUE);
    } else {
        gtk_widget_set_sensitive(entry, TRUE);
    }
}

static gboolean change_invisible_char_on_entry(GtkEventController *controller, GdkEvent *event, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    if (gdk_event_get_event_type(event) == GDK_ENTER_NOTIFY)
        g_object_set(entry, "visibility", TRUE, NULL);
    else
        g_object_set(entry, "visibility", FALSE, NULL);
    return FALSE;
}

static void cui_agent_init_callbacks(void) {
    connman_agent_set_error_cb(agent_error_cb);
    connman_agent_set_browser_cb(agent_browser_cb);
    connman_agent_set_input_cb(agent_input_cb);
    connman_agent_set_cancel_cb(agent_cancel_cb);
}

static void cui_agent_set_selected_service(const char *path, const char *name) {
    g_free(service.path);
    g_free(service.name);
    service.path = g_strdup(path);
    service.name = g_strdup(name ? name : "Hidden");
}

static void agent_ok_tethering(GtkButton *button, gpointer user_data) {
    const char *identifier = get_entry_text(cui_builder, "tethering_wifi");
    const char *passphrase = get_entry_text(cui_builder, "tethering_passphrase");
    connman_technology_set_tethering_identifier(technology, identifier);
    connman_technology_set_tethering_passphrase(technology, passphrase);
    if (tethering)
        connman_technology_tether(technology, TRUE);
    g_free(technology);
    technology = NULL;
    tethering = FALSE;
    gtk_widget_set_visible(tethering_dbox, FALSE);
    cui_tray_enable();
}

static void agent_popup_tethering_dbox(void) {
    const char *value = connman_technology_get_tethering_identifier(technology);
    set_entry(cui_builder, "tethering_wifi", value, "");
    value = connman_technology_get_tethering_passphrase(technology);
    GtkEntry *entry = set_entry(cui_builder, "tethering_passphrase", value, "");
    g_object_set(entry, "visibility", FALSE, NULL);
    set_widget_sensitive(cui_builder, "tethering_ok", gtk_entry_get_text_length(entry) >= 8);
    gtk_widget_set_visible(tethering_dbox, TRUE);
    cui_tray_disable();
}

static gboolean secret_entry_key_release_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(widget);
    GtkWidget *button = GTK_WIDGET(user_data);
    gtk_widget_set_sensitive(button, gtk_entry_get_text_length(entry) >= 8);
    return FALSE;
}

static void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether) {
    g_free(technology);
    technology = g_strdup(path);
    if (!technology) return;
    tethering = tether;
    const char *ssid = connman_technology_get_tethering_identifier(path);
    const char *passphrase = connman_technology_get_tethering_passphrase(path);
    if (tethering && (!ssid || !passphrase))
        agent_popup_tethering_dbox();
    else if (tethering)
        connman_technology_tether(path, TRUE);
    else
        agent_popup_tethering_dbox();
}

static gint cui_load_agent_dialogs(void) {
    GError *error = NULL;
    gtk_builder_add_from_file(cui_builder, CUI_AGENT_DIALOG_UI_PATH, &error);
    if (error) {
        g_printerr("Error loading agent dialogs: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    input_dbox = GTK_WIDGET(gtk_builder_get_object(cui_builder, "input_dbox"));
    login_dbox = GTK_WIDGET(gtk_builder_get_object(cui_builder, "login_dbox"));
    error_dbox = GTK_WIDGET(gtk_builder_get_object(cui_builder, "error_dbox"));
    tethering_dbox = GTK_WIDGET(gtk_builder_get_object(cui_builder, "tethering_dbox"));

    set_signal_callback(cui_builder, "input_ok", "clicked", G_CALLBACK(agent_ok_input), NULL);
    set_signal_callback(cui_builder, "input_cancel", "clicked", G_CALLBACK(agent_cancel_button), input_dbox);
    set_signal_callback(cui_builder, "login_ok", "clicked", G_CALLBACK(agent_ok_login), NULL);
    set_signal_callback(cui_builder, "login_cancel", "clicked", G_CALLBACK(agent_cancel_button), login_dbox);
    set_signal_callback(cui_builder, "tethering_ok", "clicked", G_CALLBACK(agent_ok_tethering), NULL);
    set_signal_callback(cui_builder, "tethering_cancel", "clicked", G_CALLBACK(agent_cancel_button), tethering_dbox);
    set_signal_callback(cui_builder, "error_retry", "clicked", G_CALLBACK(agent_retry), NULL);
    set_signal_callback(cui_builder, "error_cancel", "clicked", G_CALLBACK(agent_cancel_button), error_dbox);

    GtkWidget *entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "secret_entry"));
    GtkEventController *controller = gtk_event_controller_motion_new();
    g_signal_connect(controller, "enter", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(controller, "leave", G_CALLBACK(change_invisible_char_on_entry), entry);
    gtk_widget_add_controller(entry, controller);
    set_signal_callback(cui_builder, "secret_entry", "key-release-event", G_CALLBACK(secret_entry_key_release_cb), gtk_builder_get_object(cui_builder, "input_ok"));

    entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "login_password"));
    controller = gtk_event_controller_motion_new();
    g_signal_connect(controller, "enter", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(controller, "leave", G_CALLBACK(change_invisible_char_on_entry), entry);
    gtk_widget_add_controller(entry, controller);

    entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "tethering_passphrase"));
    controller = gtk_event_controller_motion_new();
    g_signal_connect(controller, "enter", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(controller, "leave", G_CALLBACK(change_invisible_char_on_entry), entry);
    gtk_widget_add_controller(entry, controller);
    set_signal_callback(cui_builder, "tethering_passphrase", "key-release-event", G_CALLBACK(secret_entry_key_release_cb), gtk_builder_get_object(cui_builder, "tethering_ok"));

    set_signal_callback(cui_builder, "passphrase_button", "toggled", G_CALLBACK(agent_passphrase_changed), NULL);
    set_signal_callback(cui_builder, "wpsbc_button", "toggled", G_CALLBACK(agent_passphrase_changed), NULL);
    set_signal_callback(cui_builder, "wpspin_button", "toggled", G_CALLBACK(agent_passphrase_changed), NULL);

    g_signal_connect(input_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(login_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(tethering_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(error_dbox, "response", G_CALLBACK(agent_close), NULL);

    return 0;
}

// Service functions
static void service_set_name(GtkService *service) {
    const char *name = connman_service_get_name(service->path);
    if (!name) name = "- Hidden -";
    char *markup;
    if (connman_service_is_favorite(service->path)) {
        if (g_strcmp0(connman_service_get_type(service->path), "wifi") == 0) {
            markup = g_markup_printf_escaped("<b>%s</b> <i>(%s)</i>", name, connman_service_get_security(service->path));
        } else {
            markup = g_markup_printf_escaped("<b>%s</b>", name);
        }
    } else {
        if (g_strcmp0(connman_service_get_type(service->path), "wifi") == 0) {
            markup = g_markup_printf_escaped("%s <i>(%s)</i>", name, connman_service_get_security(service->path));
        } else {
            markup = g_markup_printf_escaped("%s", name);
        }
    }
    gtk_label_set_markup(service->priv->name, markup);
    g_free(markup);
}

static void service_set_state(GtkService *service) {
    const struct connman_ipv4 *ipv4 = connman_service_get_ipv4(service->path);
    const char *ip = ipv4 ? ipv4->address : NULL;
    gtk_widget_set_tooltip_text(GTK_WIDGET(service->priv->name), ip ? ip : "");
    enum connman_state state = connman_service_get_state(service->path);
    GdkPaintable *paintable = NULL;
    const char *info = NULL;
    cui_theme_get_state_icone_and_info(state, &paintable, &info);
    gtk_image_set_from_paintable(service->priv->state, paintable);
    gtk_widget_set_tooltip_text(GTK_WIDGET(service->priv->state), info);
}

static void service_set_signal(GtkService *service) {
    const char *type = connman_service_get_type(service->path);
    GdkPaintable *paintable = NULL;
    const char *info = NULL;
    if (g_strcmp0(type, "wifi") == 0) {
        uint8_t strength = connman_service_get_strength(service->path);
        cui_theme_get_signal_icone_and_info(strength, &paintable, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &paintable, &info);
    }
    gtk_image_set_from_paintable(service->priv->signal, paintable);
    gtk_widget_set_tooltip_text(GTK_WIDGET(service->priv->signal), info);
}

static void gtk_service_clicked_cb(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data) {
    GtkService *service = G_TYPE_CHECK_INSTANCE_CAST(user_data, gtk_service_get_type(), GtkService);
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    if (button == GDK_BUTTON_PRIMARY) {
        if (connman_service_is_connected(service->path))
            connman_service_disconnect(service->path);
        else {
            cui_agent_set_selected_service(service->path, connman_service_get_name(service->path));
            connman_service_connect(service->path);
        }
    } else if (button == GDK_BUTTON_SECONDARY) {
        service->priv->selected = TRUE;
        cui_settings_popup(service->path);
    }
}

static GtkService *gtk_service_new(const char *path) {
    if (!path) return NULL;
    GtkService *service = g_object_new(gtk_service_get_type(), NULL);
    service->path = g_strdup(path);
    GtkGesture *gesture = gtk_gesture_click_new();
    g_signal_connect(gesture, "pressed", G_CALLBACK(gtk_service_clicked_cb), service);
    gtk_widget_add_controller(GTK_WIDGET(service), GTK_EVENT_CONTROLLER(gesture));
    service_set_name(service);
    service_set_state(service);
    service_set_signal(service);
    return service;
}

static void service_property_changed_cb(const char *path, const char *property, void *user_data) {
    GtkService *service = G_TYPE_CHECK_INSTANCE_CAST(user_data, gtk_service_get_type(), GtkService);
    if (g_strcmp0(property, "Name") == 0)
        service_set_name(service);
    else if (g_strcmp0(property, "State") == 0 || g_strcmp0(property, "IPv4") == 0 || g_strcmp0(property, "IPv6") == 0)
        service_set_state(service);
    else if (g_strcmp0(property, "Strength") == 0 || g_strcmp0(property, "Type") == 0 || g_strcmp0(property, "Security") == 0)
        service_set_signal(service);
}

// GtkTechnology implementation
typedef struct _GtkTechnology GtkTechnology;
typedef struct _GtkTechnologyClass GtkTechnologyClass;
typedef struct _GtkTechnologyPrivate GtkTechnologyPrivate;

struct _GtkTechnologyPrivate {
    GtkBox *box;
    GtkSwitch *enabler;
    GtkLabel *name;
    GtkCheckButton *tethering;
};

struct _GtkTechnology {
    GtkButton parent;
    GtkTechnologyPrivate *priv;
    gchar *path;
};

struct _GtkTechnologyClass {
    GtkButtonClass parent_class;
};

GType gtk_technology_get_type(void);
G_DEFINE_TYPE_WITH_PRIVATE(GtkTechnology, gtk_technology, GTK_TYPE_BUTTON)

static void gtk_technology_init(GtkTechnology *technology) {
    technology->priv = gtk_technology_get_instance_private(technology);
    technology->priv->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    technology->priv->enabler = GTK_SWITCH(gtk_switch_new());
    technology->priv->name = GTK_LABEL(gtk_label_new(NULL));
    technology->priv->tethering = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Tethering"));
    gtk_box_append(technology->priv->box, GTK_WIDGET(technology->priv->enabler));
    gtk_box_append(technology->priv->box, GTK_WIDGET(technology->priv->name));
    gtk_box_append(technology->priv->box, GTK_WIDGET(technology->priv->tethering));
    gtk_button_set_child(GTK_BUTTON(technology), GTK_WIDGET(technology->priv->box));
}

static void gtk_technology_dispose(GObject *object) {
    GtkTechnology *technology = G_TYPE_CHECK_INSTANCE_CAST(object, gtk_technology_get_type(), GtkTechnology);
    connman_technology_set_property_changed_callback(technology->path, NULL, NULL);
    g_free(technology->path);
    G_OBJECT_CLASS(gtk_technology_parent_class)->dispose(object);
}

static void gtk_technology_class_init(GtkTechnologyClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = gtk_technology_dispose;
}

static void gtk_technology_tethering_toggled(GtkCheckButton *checkbutton, gpointer user_data) {
    GtkTechnology *technology = G_TYPE_CHECK_INSTANCE_CAST(user_data, gtk_technology_get_type(), GtkTechnology);
    gboolean tethering = gtk_check_button_get_active(checkbutton);
    cui_agent_set_wifi_tethering_settings(technology->path, tethering);
}

static void gtk_technology_clicked_cb(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data) {
    GtkTechnology *technology = G_TYPE_CHECK_INSTANCE_CAST(user_data, gtk_technology_get_type(), GtkTechnology);
    gboolean enable = !gtk_switch_get_active(technology->priv->enabler);
    connman_technology_enable(technology->path, enable);
    gtk_switch_set_active(technology->priv->enabler, enable);
}

static GtkTechnology *gtk_technology_new(const char *path) {
    if (!path) return NULL;
    GtkTechnology *technology = g_object_new(gtk_technology_get_type(), NULL);
    technology->path = g_strdup(path);
    gtk_label_set_text(technology->priv->name, connman_technology_get_name(path));
    gtk_switch_set_active(technology->priv->enabler, connman_technology_is_enabled(path));
    gtk_check_button_set_active(technology->priv->tethering, connman_technology_is_tethering(path));
    g_signal_connect(technology->priv->tethering, "toggled", G_CALLBACK(gtk_technology_tethering_toggled), technology);
    GtkGesture *gesture = gtk_gesture_click_new();
    g_signal_connect(gesture, "pressed", G_CALLBACK(gtk_technology_clicked_cb), technology);
    gtk_widget_add_controller(GTK_WIDGET(technology), GTK_EVENT_CONTROLLER(gesture));
    return technology;
}

// Left menu implementation
static void add_or_update_service(const char *path, int position) {
    GtkService *service = gtk_service_new(path);
    if (position > 10)
        gtk_box_append(GTK_BOX(cui_more_menu), GTK_WIDGET(service));
    else
        gtk_box_insert_child_after(GTK_BOX(cui_left_menu), GTK_WIDGET(service), position > 0 ? gtk_widget_get_first_child(cui_left_menu) : NULL);
    g_hash_table_insert(service_items, g_strdup(path), service);
}

static void remove_service(const char *path) {
    g_hash_table_remove(service_items, path);
}

static void cui_popup_left_menu(GtkWidget *trayicon, gpointer user_data) {
    gtk_widget_set_visible(cui_list_more_item, FALSE);
    gtk_widget_set_visible(cui_scan_spinner, TRUE);
    gtk_spinner_start(GTK_SPINNER(cui_scan_spinner));
    g_hash_table_remove_all(service_items);
    GSList *services = connman_service_get_services();
    int item = 2;
    for (GSList *list = services; list; list = list->next) {
        add_or_update_service(list->data, item++);
    }
    if (item > 10)
        gtk_widget_set_visible(cui_list_more_item, TRUE);
    g_slist_free(services);
    gtk_spinner_stop(GTK_SPINNER(cui_scan_spinner));
    gtk_widget_set_visible(cui_scan_spinner, FALSE);
    if (cui_trayicon) {
        app_indicator_set_menu(cui_trayicon, GTK_MENU(gtk_builder_get_object(cui_builder, "cui_left_menu")));
    }
    gtk_popover_popup(GTK_POPOVER(cui_left_menu));
}

static void cui_popdown_left_menu(GtkWidget *menu, gpointer user_data) {
    g_hash_table_remove_all(service_items);
}

static gint cui_load_left_menu(GtkBuilder *builder, GtkWidget *tray) {
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_LEFT_MENU_UI_PATH, &error);
    if (error) {
        g_printerr("Error loading left menu: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_left_menu = GTK_WIDGET(gtk_builder_get_object(builder, "cui_left_menu"));
    cui_more_menu = GTK_WIDGET(gtk_builder_get_object(builder, "cui_more_menu"));
    cui_list_more_item = GTK_WIDGET(gtk_builder_get_object(builder, "cui_list_more_item"));
    cui_scan_spinner = GTK_WIDGET(gtk_builder_get_object(builder, "cui_scan_spinner"));
    g_signal_connect(cui_left_menu, "closed", G_CALLBACK(cui_popdown_left_menu), NULL);
    service_items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
    cui_tray_hook_left_menu(cui_popup_left_menu);
    return 0;
}

// Right menu implementation
static void add_tech(const char *path) {
    GtkTechnology *tech = gtk_technology_new(path);
    gtk_box_insert_child_after(GTK_BOX(cui_right_menu), GTK_WIDGET(tech), item_position > 0 ? gtk_widget_get_first_child(cui_right_menu) : NULL);
    item_position++;
    gtk_box_append(GTK_BOX(cui_tethering_menu), GTK_WIDGET(tech->priv->tethering));
    g_hash_table_insert(tech_items, g_strdup(path), tech);
}

static void cui_popup_right_menu(GtkWidget *trayicon, guint button, guint time, gpointer user_data) {
    if (disabled) goto popup;
    gtk_widget_set_visible(cui_item_mode_on, connman_manager_get_offlinemode());
    gtk_widget_set_visible(cui_item_mode_off, !connman_manager_get_offlinemode());
    GList *techs = connman_technology_get_technologies();
    for (GList *list = techs; list; list = list->next)
        add_tech(list->data);
    g_list_free(techs);
popup:
    if (cui_trayicon) {
        app_indicator_set_menu(cui_trayicon, GTK_MENU(gtk_builder_get_object(cui_builder, "cui_right_menu")));
    }
    gtk_popover_popup(GTK_POPOVER(cui_right_menu));
}

static void cui_popdown_right_menu(GtkWidget *menu, gpointer user_data) {
    g_hash_table_remove_all(tech_items);
}

static void cui_item_offline_mode(GtkWidget *item, gpointer user_data) {
    connman_manager_set_offlinemode(!connman_manager_get_offlinemode());
}

static void cui_item_quit(GtkWidget *item, gpointer user_data) {
    g_application_quit(g_application_get_default());
}

static gint cui_load_right_menu(GtkBuilder *builder, GtkWidget *tray) {
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_RIGHT_MENU_UI_PATH, &error);
    if (error) {
        g_printerr("Error loading right menu: %s\n", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_right_menu = GTK_WIDGET(gtk_builder_get_object(builder, "cui_right_menu"));
    cui_item_mode_off = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_off"));
    cui_item_mode_on = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_on"));
    cui_tethering_menu = GTK_WIDGET(gtk_builder_get_object(builder, "cui_tethering_menu"));
    g_signal_connect(cui_right_menu, "closed", G_CALLBACK(cui_popdown_right_menu), NULL);
    g_signal_connect(gtk_builder_get_object(builder, "cui_item_quit"), "activate", G_CALLBACK(cui_item_quit), NULL);
    g_signal_connect(cui_item_mode_off, "activate", G_CALLBACK(cui_item_offline_mode), NULL);
    g_signal_connect(cui_item_mode_on, "activate", G_CALLBACK(cui_item_offline_mode), NULL);
    tech_items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
    cui_tray_hook_right_menu(cui_popup_right_menu);
    return 0;
}

// Settings implementation
static void favorite_toggled(GtkToggleButton *toggle, gpointer user_data) {
    if (!gtk_toggle_button_get_active(toggle))
        connman_service_remove(path);
}

static void autoconnect_toggled(GtkToggleButton *toggle, gpointer user_data) {
    connman_service_set_autoconnectable(path, gtk_toggle_button_get_active(toggle));
}

static void update_service_header(void) {
    const char *type = connman_service_get_type(path);
    GdkPaintable *paintable = NULL;
    const char *info = NULL;
    if (g_strcmp0(type, "wifi") == 0) {
        uint8_t strength = connman_service_get_strength(path);
        cui_theme_get_signal_icone_and_info(strength, &paintable, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &paintable, &info);
    }
    set_image(cui_builder, "service_type", paintable, info);
    set_label(cui_builder, "service_name", connman_service_get_name(path), "- Hidden -");
    if (connman_service_is_connected(path)) {
        cui_theme_get_state_icone_and_info(connman_service_get_state(path), &paintable, &info);
        set_image(cui_builder, "service_state", paintable, info);
    }
    set_label(cui_builder, "service_error", connman_service_get_error(path), "");
    set_button_toggle(cui_builder, "service_favorite", connman_service_is_favorite(path));
    set_button_toggle(cui_builder, "service_autoconnect", connman_service_is_autoconnect(path));
    set_signal_callback(cui_builder, "service_favorite", "toggled", G_CALLBACK(favorite_toggled), NULL);
    set_signal_callback(cui_builder, "service_autoconnect", "toggled", G_CALLBACK(autoconnect_toggled), NULL);
}

static void enable_ipv4(gboolean enable) {
    set_widget_sensitive(cui_builder, "ipv4_address", enable);
    set_widget_sensitive(cui_builder, "ipv4_netmask", enable);
    set_widget_sensitive(cui_builder, "ipv4_gateway", enable);
}

static void set_ipv4(const char *method) {
    if (g_strcmp0(method, "dhcp") == 0) {
        set_button_toggle(cui_builder, "ipv4_dhcp", TRUE);
        enable_ipv4(FALSE);
    } else if (g_strcmp0(method, "manual") == 0) {
        set_button_toggle(cui_builder, "ipv4_manual", TRUE);
        enable_ipv4(TRUE);
    } else {
        set_button_toggle(cui_builder, "ipv4_off", TRUE);
        enable_ipv4(FALSE);
    }
}

static void update_ipv4(void) {
    const struct connman_ipv4 *ipv4 = connman_service_get_ipv4_config(path);
    set_entry(cui_builder, "ipv4_address", ipv4 ? ipv4->address : "", "");
    set_entry(cui_builder, "ipv4_netmask", ipv4 ? ipv4->netmask : "", "");
    set_entry(cui_builder, "ipv4_gateway", ipv4 ? ipv4->gateway : "", "");
    set_ipv4(ipv4 ? ipv4->method : "off");
}

static void enable_ipv6(gboolean enable) {
    set_widget_sensitive(cui_builder, "ipv6_address", enable);
    set_widget_sensitive(cui_builder, "ipv6_prefix", enable);
    set_widget_sensitive(cui_builder, "ipv6_gateway", enable);
}

static void set_ipv6(const char *method) {
    if (g_strcmp0(method, "auto") == 0) {
        set_button_toggle(cui_builder, "ipv6_auto", TRUE);
        enable_ipv6(FALSE);
        set_widget_sensitive(cui_builder, "ipv6_privacy", TRUE);
    } else if (g_strcmp0(method, "manual") == 0) {
        set_button_toggle(cui_builder, "ipv6_manual", TRUE);
        enable_ipv6(TRUE);
    } else {
        set_button_toggle(cui_builder, "ipv6_off", TRUE);
        enable_ipv6(FALSE);
        set_widget_sensitive(cui_builder, "ipv6_privacy", FALSE);
    }
}

static void update_ipv6(void) {
    const struct connman_ipv6 *ipv6 = connman_service_get_ipv6_config(path);
    char prefix[6] = {0};
    if (ipv6) snprintf(prefix, sizeof(prefix), "%u", ipv6->prefix);
    set_entry(cui_builder, "ipv6_address", ipv6 ? ipv6->address : "", "");
    set_entry(cui_builder, "ipv6_prefix", prefix, "");
    set_entry(cui_builder, "ipv6_gateway", ipv6 ? ipv6->gateway : "", "");
    set_ipv6(ipv6 ? ipv6->method : "off");
    set_button_toggle(cui_builder, ipv6 && g_strcmp0(ipv6->privacy, "enabled") == 0 ? "ipv6_privacy_enabled" : "ipv6_privacy_disabled", TRUE);
}

static void update_dns(void) {
    set_entry(cui_builder, "nameservers", connman_service_get_nameservers_config(path), "");
    set_entry(cui_builder, "domains", connman_service_get_domains_config(path), "");
}

static void update_timeservers(void) {
    set_entry(cui_builder, "timeservers", connman_service_get_timeservers_config(path), "");
}

static void enable_proxy(gboolean enable) {
    set_widget_sensitive(cui_builder, "proxy_url", enable);
    set_widget_sensitive(cui_builder, "proxy_servers", enable);
    set_widget_sensitive(cui_builder, "proxy_excludes", enable);
}

static void set_proxy(const char *method) {
    if (g_strcmp0(method, "auto") == 0) {
        set_button_toggle(cui_builder, "proxy_auto", TRUE);
        enable_proxy(FALSE);
    } else if (g_strcmp0(method, "manual") == 0) {
        set_button_toggle(cui_builder, "proxy_manual", TRUE);
        enable_proxy(TRUE);
    } else {
        set_button_toggle(cui_builder, "proxy_direct", TRUE);
        enable_proxy(FALSE);
    }
}

static void update_proxy(void) {
    const struct connman_proxy *proxy = connman_service_get_proxy_config(path);
    set_entry(cui_builder, "proxy_url", proxy ? proxy->url : "", "");
    set_entry(cui_builder, "proxy_servers", proxy ? proxy->servers : "", "");
    set_entry(cui_builder, "proxy_excludes", proxy ? proxy->excludes : "", "");
    set_proxy(proxy ? proxy->method : "direct");
}

static void toggled_ipv4(GtkToggleButton *toggle, gpointer user_data) {
    if (!gtk_toggle_button_get_active(toggle)) return;
    const char *name = user_data;
    ipv4_method = g_strcmp0(name, "ipv4_manual") == 0 ? "manual" : g_strcmp0(name, "ipv4_dhcp") == 0 ? "dhcp" : "off";
    enable_ipv4(g_strcmp0(ipv4_method, "manual") == 0);
    ipv4_changed = TRUE;
}

static void toggled_ipv6(GtkToggleButton *toggle, gpointer user_data) {
    if (!gtk_toggle_button_get_active(toggle)) return;
    const char *name = user_data;
    ipv6_method = g_strcmp0(name, "ipv6_manual") == 0 ? "manual" : g_strcmp0(name, "ipv6_auto") == 0 ? "auto" : "off";
    enable_ipv6(g_strcmp0(ipv6_method, "manual") == 0);
    set_widget_sensitive(cui_builder, "ipv6_privacy", g_strcmp0(ipv6_method, "auto") == 0);
    ipv6_changed = TRUE;
}

static void toggled_ipv6_privacy(GtkToggleButton *toggle, gpointer user_data) {
    if (!gtk_toggle_button_get_active(toggle)) return;
    const char *name = user_data;
    ipv6_privacy = g_strcmp0(name, "ipv6_privacy_enabled") == 0 ? "enabled" : "disabled";
    ipv6_changed = TRUE;
}

static void toggled_proxy(GtkToggleButton *toggle, gpointer user_data) {
    if (!gtk_toggle_button_get_active(toggle)) return;
    const char *name = user_data;
    proxy_method = g_strcmp0(name, "proxy_manual") == 0 ? "manual" : g_strcmp0(name, "proxy_auto") == 0 ? "auto" : "direct";
    enable_proxy(g_strcmp0(proxy_method, "manual") == 0);
    proxy_changed = TRUE;
}

static void key_released(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    gboolean *flag = user_data;
    *flag = TRUE;
}

static void settings_apply(GtkButton *button, gpointer user_data) {
    if (ipv4_changed) {
        struct connman_ipv4 ipv4 = {0};
        ipv4.method = g_strdup(ipv4_method);
        ipv4.address = g_strdup(get_entry_text(cui_builder, "ipv4_address"));
        ipv4.netmask = g_strdup(get_entry_text(cui_builder, "ipv4_netmask"));
        ipv4.gateway = g_strdup(get_entry_text(cui_builder, "ipv4_gateway"));
        connman_service_set_ipv4_config(path, &ipv4);
        g_free(ipv4.method);
        g_free(ipv4.address);
        g_free(ipv4.netmask);
        g_free(ipv4.gateway);
    }
    if (ipv6_changed) {
        struct connman_ipv6 ipv6 = {0};
        ipv6.method = g_strdup(ipv6_method);
        ipv6.address = g_strdup(get_entry_text(cui_builder, "ipv6_address"));
        ipv6.prefix = atoi(get_entry_text(cui_builder, "ipv6_prefix"));
        ipv6.gateway = g_strdup(get_entry_text(cui_builder, "ipv6_gateway"));
        ipv6.privacy = g_strdup(ipv6_privacy);
        connman_service_set_ipv6_config(path, &ipv6);
        g_free(ipv6.method);
        g_free(ipv6.address);
        g_free(ipv6.gateway);
        g_free(ipv6.privacy);
    }
    if (proxy_changed) {
        struct connman_proxy proxy = {0};
        proxy.method = g_strdup(proxy_method);
        proxy.url = g_strdup(get_entry_text(cui_builder, "proxy_url"));
        proxy.servers = g_strdup(get_entry_text(cui_builder, "proxy_servers"));
        proxy.excludes = g_strdup(get_entry_text(cui_builder, "proxy_excludes"));
        connman_service_set_proxy_config(path, &proxy);
        g_free(proxy.method);
        g_free(proxy.url);
        g_free(proxy.servers);
        g_free(proxy.excludes);
    }
    if (nameservers_changed)
        connman_service_set_nameservers_config(path, get_entry_text(cui_builder, "nameservers"));
    if (domains_changed)
        connman_service_set_domains_config(path, get_entry_text(cui_builder, "domains"));
    if (timeservers_changed)
        connman_service_set_timeservers_config(path, get_entry_text(cui_builder, "timeservers"));
    gtk_widget_set_visible(service_settings_dbox, FALSE);
    g_free(path);
    path = NULL;
    cui_tray_enable();
}

static void settings_cancel(GtkButton *button, gpointer user_data) {
    gtk_widget_set_visible(service_settings_dbox, FALSE);
    g_free(path);
    path = NULL;
    cui_tray_enable();
}

static void cui_settings_popup(const char *service_path) {
    if (service_settings_dbox) {
        gtk_widget_set_visible(service_settings_dbox, FALSE);
        g_free(path);
    }
    GError *error = NULL;
    gtk_builder_add_from_file(cui_builder, CUI_SETTINGS_UI_PATH, &error);
    if (error) {
        g_printerr("Error loading settings UI: %s\n", error->message);
        g_error_free(error);
        return;
    }
    service_settings_dbox = GTK_WIDGET(gtk_builder_get_object(cui_builder, "service_settings_dbox"));
    path = g_strdup(service_path);
    cui_tray_disable();

    update_service_header();
    update_ipv4();
    update_ipv6();
    update_dns();
    update_timeservers();
    update_proxy();

    set_signal_callback(cui_builder, "ipv4_dhcp", "toggled", G_CALLBACK(toggled_ipv4), "ipv4_dhcp");
    set_signal_callback(cui_builder, "ipv4_manual", "toggled", G_CALLBACK(toggled_ipv4), "ipv4_manual");
    set_signal_callback(cui_builder, "ipv4_off", "toggled", G_CALLBACK(toggled_ipv4), "ipv4_off");
    set_signal_callback(cui_builder, "ipv6_auto", "toggled", G_CALLBACK(toggled_ipv6), "ipv6_auto");
    set_signal_callback(cui_builder, "ipv6_manual", "toggled", G_CALLBACK(toggled_ipv6), "ipv6_manual");
    set_signal_callback(cui_builder, "ipv6_off", "toggled", G_CALLBACK(toggled_ipv6), "ipv6_off");
    set_signal_callback(cui_builder, "ipv6_privacy_enabled", "toggled", G_CALLBACK(toggled_ipv6_privacy), "ipv6_privacy_enabled");
    set_signal_callback(cui_builder, "ipv6_privacy_disabled", "toggled", G_CALLBACK(toggled_ipv6_privacy), "ipv6_privacy_disabled");
    set_signal_callback(cui_builder, "proxy_auto", "toggled", G_CALLBACK(toggled_proxy), "proxy_auto");
    set_signal_callback(cui_builder, "proxy_manual", "toggled", G_CALLBACK(toggled_proxy), "proxy_manual");
    set_signal_callback(cui_builder, "proxy_direct", "toggled", G_CALLBACK(toggled_proxy), "proxy_direct");
    set_signal_callback(cui_builder, "settings_apply", "clicked", G_CALLBACK(settings_apply), NULL);
    set_signal_callback(cui_builder, "settings_cancel", "clicked", G_CALLBACK(settings_cancel), NULL);

    set_signal_callback(cui_builder, "ipv4_address", "key-release-event", G_CALLBACK(key_released), &ipv4_changed);
    set_signal_callback(cui_builder, "ipv4_netmask", "key-release-event", G_CALLBACK(key_released), &ipv4_changed);
    set_signal_callback(cui_builder, "ipv4_gateway", "key-release-event", G_CALLBACK(key_released), &ipv4_changed);
    set_signal_callback(cui_builder, "ipv6_address", "key-release-event", G_CALLBACK(key_released), &ipv6_changed);
    set_signal_callback(cui_builder, "ipv6_prefix", "key-release-event", G_CALLBACK(key_released), &ipv6_changed);
    set_signal_callback(cui_builder, "ipv6_gateway", "key-release-event", G_CALLBACK(key_released), &ipv6_changed);
    set_signal_callback(cui_builder, "proxy_url", "key-release-event", G_CALLBACK(key_released), &proxy_changed);
    set_signal_callback(cui_builder, "proxy_servers", "key-release-event", G_CALLBACK(key_released), &proxy_changed);
    set_signal_callback(cui_builder, "proxy_excludes", "key-release-event", G_CALLBACK(key_released), &proxy_changed);
    set_signal_callback(cui_builder, "nameservers", "key-release-event", G_CALLBACK(key_released), &nameservers_changed);
    set_signal_callback(cui_builder, "domains", "key-release-event", G_CALLBACK(key_released), &domains_changed);
    set_signal_callback(cui_builder, "timeservers", "key-release-event", G_CALLBACK(key_released), &timeservers_changed);

    gtk_widget_set_visible(service_settings_dbox, TRUE);
}

// Initialization
static void interface_connected_cb(void *user_data) {
    // Handle interface connection
}

static void interface_disconnected_cb(void *user_data) {
    // Handle interface disconnection
}

static void manager_property_changed_cb(const char *path, const char *property, void *user_data) {
    // Handle manager property changes
}

static gint cui_init(void) {
    cui_builder = gtk_builder_new();
    cui_load_theme();
    if (connman_interface_init(interface_connected_cb, interface_disconnected_cb, NULL) < 0)
        return -EINVAL;
    if (connman_manager_init(manager_property_changed_cb, NULL) < 0)
        return -EINVAL;
    if (connman_technology_init() < 0)
        return -EINVAL;
    if (connman_service_init() < 0)
        return -EINVAL;
    if (connman_agent_init() < 0)
        return -EINVAL;
    if (cui_load_trayicon(cui_builder) < 0)
        return -EINVAL;
    if (cui_load_left_menu(cui_builder, cui_trayicon) < 0)
        return -EINVAL;
    if (cui_load_right_menu(cui_builder, cui_trayicon) < 0)
        return -EINVAL;
    if (cui_load_agent_dialogs() < 0)
        return -EINVAL;
    cui_agent_init_callbacks();
    connman_manager_register_agent("/org/connman/gtk/agent");
    return 0;
}

static void cui_finalize(void) {
    connman_manager_unregister_agent("/org/connman/gtk/agent");
    connman_agent_finalize();
    connman_service_finalize();
    connman_technology_finalize();
    connman_manager_finalize();
    connman_interface_finalize();
    g_object_unref(cui_builder);
}

static void activate(GtkApplication *app, gpointer user_data) {
    if (cui_init() < 0) {
        g_printerr("Initialization failed\n");
        g_application_quit(G_APPLICATION(app));
        return;
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain("connman-gtk", "/usr/share/locale");
    textdomain("connman-gtk");
    GtkApplication *app = gtk_application_new("org.connman.gtk", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    cui_finalize();
    g_object_unref(app);
    return status;
}
