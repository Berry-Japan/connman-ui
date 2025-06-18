// gcc `pkg-config --cflags --libs gtk+-3.0 glib-2.0 dbus-1` -o connman-ui ui.c gdbus/client.c gdbus/mainloop.c gdbus/object.c gdbus/polkit.c gdbus/watch.c -I. lib/agent.c lib/dbus.c lib/interface.c lib/manager.c lib/service.c lib/technology.c -Ilib/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <connman-interface.h>

// 選択されたサービスを管理する構造体
struct cui_selected_service {
    char *path;
    char *name;
};

extern GtkBuilder *cui_builder;

// テーマ関連の関数プロトタイプ
void cui_load_theme(void);
void cui_theme_get_type_icone_and_info(const char *type, GdkPixbuf **image, const char **info);
void cui_theme_get_signal_icone_and_info(uint8_t signal_strength, GdkPixbuf **image, const char **info);
void cui_theme_get_state_icone_and_info(enum connman_state state, GdkPixbuf **image, const char **info);
void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image, const char **info);

// トレイアイコン関連の関数プロトタイプ
gint cui_load_trayicon(GtkBuilder *builder);
void cui_trayicon_update_icon(void);
void cui_tray_hook_left_menu(gpointer callback);
void cui_tray_hook_right_menu(gpointer callback);
void cui_tray_left_menu_disable(void);
void cui_tray_enable(void);
void cui_tray_disable(void);

// エージェント関連の関数プロトタイプ
gint cui_load_agent_dialogs(void);
void cui_agent_init_callbacks(void);
void cui_agent_set_selected_service(const char *path, const char *name);
void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether);

// メニュー関連の関数プロトタイプ
gint cui_load_left_menu(GtkBuilder *builder, GtkStatusIcon *trayicon);
gint cui_load_right_menu(GtkBuilder *builder, GtkStatusIcon *trayicon);
void cui_right_menu_enable_only_quit(void);
void cui_right_menu_enable_all(void);
gint cui_settings_popup(const char *path);

// UIユーティリティ関数プロトタイプ
GtkLabel *set_label(GtkBuilder *builder, const char *name, const char *value, const char *default_value);
GtkEntry *set_entry(GtkBuilder *builder, const char *name, const char *value, const char *default_value);
GtkWidget *set_widget_sensitive(GtkBuilder *builder, const char *name, gboolean value);
GtkWidget *set_widget_hidden(GtkBuilder *builder, const char *name, gboolean value);
GtkWidget *set_button_toggle(GtkBuilder *builder, const char *name, gboolean active);
GtkImage *set_image(GtkBuilder *builder, const char *name, GdkPixbuf *image, const char *info);
void set_signal_callback(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler, gpointer user_data);
const char *get_entry_text(GtkBuilder *builder, const char *name);

// GtkServiceクラスの定義
G_BEGIN_DECLS
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
    GtkMenuItem parent_class;
    GtkServicePrivate *priv;
    gchar *path;
};

struct _GtkServiceClass {
    GtkMenuItemClass parent_class;
};

GType gtk_service_get_type(void) G_GNUC_CONST;
GtkService *gtk_service_new(const gchar *path);
G_END_DECLS

// GtkTechnologyクラスの定義
G_BEGIN_DECLS
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
    GtkMenuItem parent_class;
    GtkTechnologyPrivate *priv;
    gchar *path;
};

struct _GtkTechnologyClass {
    GtkMenuItemClass parent_class;
};

GType gtk_technology_get_type(void) G_GNUC_CONST;
GtkTechnology *gtk_technology_new(const gchar *path);
GtkMenuItem *gtk_technology_get_tethering_item(GtkTechnology *technology);
G_END_DECLS

// 定数定義
#define CUI_UI_PATH "/usr/share/connman_ui_gtk"
#define CUI_ICON_PATH CUI_UI_PATH "/icons"
#define CUI_AGENT_DIALOG_UI_PATH CUI_UI_PATH "/ui/agent.ui"
#define CUI_LEFT_MENU_UI_PATH CUI_UI_PATH "/ui/left_menu.ui"
#define CUI_RIGHT_MENU_UI_PATH CUI_UI_PATH "/ui/right_menu.ui"
#define CUI_SETTINGS_UI_PATH CUI_UI_PATH "/ui/settings.ui"
#define CUI_TRAYICON_UI_PATH CUI_UI_PATH "/ui/tray.ui"
#define GETTEXT_PACKAGE "connman-ui"
#define LOCALEDIR "/usr/share/locale"

// グローバル変数
static GtkDialog *input_dbox = NULL;
static GtkDialog *login_dbox = NULL;
static GtkDialog *error_dbox = NULL;
static GtkDialog *tethering_dbox = NULL;
static struct cui_selected_service service = {NULL, NULL};
static char *technology = NULL;
static gboolean tethering = FALSE;

// エージェント関連のコールバック関数
static void agent_popup_error_dbox(const char *error) {
    if (!error) return;
    set_label(cui_builder, "error_message", error, "");
    gtk_widget_show(GTK_WIDGET(error_dbox));
    cui_tray_disable();
}

static void agent_error_cb(const char *path, const char *error) {
    agent_popup_error_dbox(error);
}

static void agent_browser_cb(const char *path, const char *url) {
    // 将来の実装のためのプレースホルダ
}

static void setup_hidden_ui(GtkBuilder *builder, gboolean hidden) {
    GtkWidget *button_name = GTK_WIDGET(gtk_builder_get_object(builder, "name_button"));
    GtkWidget *button_ssid = GTK_WIDGET(gtk_builder_get_object(builder, "ssid_button"));
    GtkWidget *entry_hidden = GTK_WIDGET(gtk_builder_get_object(builder, "hidden_entry"));
    if (!button_name || !button_ssid || !entry_hidden) return;

    gtk_widget_set_visible(button_name, hidden);
    gtk_widget_set_visible(button_ssid, hidden);
    gtk_widget_set_visible(entry_hidden, hidden);
    if (hidden) {
        gtk_entry_set_text(GTK_ENTRY(entry_hidden), "");
    }
}

static void setup_passphrase_ui(GtkBuilder *builder, gboolean passphrase, const char *previous_passphrase) {
    GtkToggleButton *button_passphrase = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "passphrase_button"));
    GtkEntry *entry_secret = GTK_ENTRY(gtk_builder_get_object(builder, "secret_entry"));
    GtkLabel *label_previous = GTK_LABEL(gtk_builder_get_object(builder, "previous_label"));
    if (!button_passphrase || !entry_secret || !label_previous) return;

    gtk_toggle_button_set_active(button_passphrase, TRUE);
    gtk_entry_set_text(entry_secret, "");
    gtk_entry_set_visibility(entry_secret, FALSE);
    if (previous_passphrase) {
        char *text = g_strdup_printf(_("Previous Passphrase:\n%s"), previous_passphrase);
        gtk_label_set_text(label_previous, text);
        g_free(text);
    } else {
        gtk_label_set_text(label_previous, "");
        gtk_widget_set_visible(GTK_WIDGET(label_previous), FALSE);
    }
}

static void setup_wps_ui(GtkBuilder *builder, gboolean wpspin, const char *previous_wpspin) {
    GtkToggleButton *button_wpspbc = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wpspbc_button"));
    GtkToggleButton *button_wpspin = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wpspin_button"));
    if (!button_wpspbc || !button_wpspin) return;

    gtk_widget_set_visible(GTK_WIDGET(button_wpspbc), wpspin);
    gtk_toggle_button_set_active(button_wpspbc, FALSE);
    gtk_widget_set_visible(GTK_WIDGET(button_wpspin), FALSE);
    gtk_toggle_button_set_active(button_wpspin, FALSE);
}

static void agent_popup_input_dbox(gboolean hidden, gboolean passphrase, const char *previous_passphrase, gboolean wpspin, const char *previous_wpspin) {
    if (!service.name) return;
    set_label(cui_builder, "service_label", service.name, "");
    setup_hidden_ui(cui_builder, hidden);
    setup_passphrase_ui(cui_builder, passphrase, previous_passphrase);
    setup_wps_ui(cui_builder, wpspin, previous_wpspin);
    set_widget_sensitive(cui_builder, "input_ok", FALSE);
    gtk_widget_show(GTK_WIDGET(input_dbox));
    cui_tray_disable();
}

static void agent_popup_login_dbox(void) {
    GtkEntry *entry = set_entry(cui_builder, "login_password", "", "");
    if (!entry) return;
    set_entry(cui_builder, "login_username", "", "");
    gtk_entry_set_visibility(entry, FALSE);
    gtk_widget_show(GTK_WIDGET(login_dbox));
    cui_tray_disable();
}

static void agent_input_cb(const char *path, gboolean hidden, gboolean identity, gboolean passphrase, const char *previous_passphrase, gboolean wpspin, const char *previous_wpspin, gboolean login) {
    if (!path || !service.path || g_strcmp0(path, service.path) != 0) {
        connman_agent_reply_canceled();
        return;
    }
    if (passphrase || hidden) {
        agent_popup_input_dbox(hidden, passphrase, previous_passphrase, wpspin, previous_wpspin);
    } else if (login) {
        agent_popup_login_dbox();
    } else {
        connman_agent_reply_canceled();
    }
}

static void agent_cancel_cb(void) {
    if (input_dbox) {
        gtk_widget_hide(GTK_WIDGET(input_dbox));
    }
    cui_tray_enable();
}

typedef struct {
    const char *name;
    const char *passphrase;
    const char *wpspin;
    gboolean wps;
} InputData;

static void agent_ok_input(GtkButton *button, gpointer user_data) {
    InputData data = {NULL, NULL, NULL, FALSE};
    GtkEntry *entry_hidden = GTK_ENTRY(gtk_builder_get_object(cui_builder, "hidden_entry"));
    GtkEntry *entry_secret = GTK_ENTRY(gtk_builder_get_object(cui_builder, "secret_entry"));
    GtkToggleButton *toggle_passphrase = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "passphrase_button"));
    GtkToggleButton *toggle_wpspbc = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "wpspbc_button"));
    GtkToggleButton *toggle_wpspin = GTK_TOGGLE_BUTTON(gtk_builder_get_object(cui_builder, "wpspin_button"));
    if (!entry_hidden || !entry_secret || !toggle_passphrase || !toggle_wpspbc || !toggle_wpspin) return;

    if (gtk_entry_get_text_length(entry_hidden) > 0) {
        data.name = gtk_entry_get_text(entry_hidden);
    }
    if (gtk_toggle_button_get_active(toggle_passphrase)) {
        data.passphrase = gtk_entry_get_text(entry_secret);
    }
    if (gtk_toggle_button_get_active(toggle_wpspbc)) {
        data.wps = TRUE;
        data.wpspin = "";
    } else if (gtk_toggle_button_get_active(toggle_wpspin)) {
        data.wps = TRUE;
        data.wpspin = gtk_entry_get_text(entry_secret);
    }
    connman_agent_reply_passphrase(data.name, data.passphrase, data.wps, data.wpspin);
    gtk_widget_hide(GTK_WIDGET(input_dbox));
    cui_tray_enable();
}

static void agent_ok_login(GtkButton *button, gpointer user_data) {
    const char *username = NULL, *password = NULL;
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "login_username"));
    if (!entry) return;
    if (gtk_entry_get_text_length(entry) > 0) {
        username = gtk_entry_get_text(entry);
    }
    entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "login_password"));
    if (!entry) return;
    if (gtk_entry_get_text_length(entry) > 0) {
        password = gtk_entry_get_text(entry);
    }
    connman_agent_reply_login(username, password);
    gtk_widget_hide(GTK_WIDGET(login_dbox));
    cui_tray_enable();
}

static void cancel(void) {
    cui_tray_enable();
    connman_agent_reply_canceled();
}

static void agent_cancel(GtkButton *button, gpointer user_data) {
    GtkDialog *dialog_box = user_data;
    if (dialog_box) {
        gtk_widget_hide(GTK_WIDGET(dialog_box));
    }
    cancel();
}

static void agent_close(GtkDialog *dialog_box, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_DELETE_EVENT || response_id == GTK_RESPONSE_CLOSE) {
        cancel();
    }
}

static void agent_retry(GtkButton *button, gpointer user_data) {
    gtk_widget_hide(GTK_WIDGET(error_dbox));
    cui_tray_enable();
    connman_agent_reply_retry();
}

static void agent_passphrase_changed(GtkToggleButton *togglebutton, gpointer user_data) {
    const gchar *label = gtk_button_get_label(GTK_BUTTON(togglebutton));
    GtkWidget *entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "secret_entry"));
    if (!entry) return;

    if (g_strcmp0(label, "WPS Push-Button") == 0) {
        gtk_widget_set_sensitive(entry, FALSE);
        set_widget_sensitive(cui_builder, "input_ok", TRUE);
    } else {
        gtk_widget_set_sensitive(entry, TRUE);
    }
}

void cui_agent_init_callbacks(void) {
    connman_agent_set_error_cb(agent_error_cb);
    connman_agent_set_browser_cb(agent_browser_cb);
    connman_agent_set_input_cb(agent_input_cb);
    connman_agent_set_cancel_cb(agent_cancel_cb);
}

static gboolean change_invisible_char_on_entry(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    GdkEventCrossing *ev_cross = (GdkEventCrossing *) event;
    GtkEntry *entry = user_data;
    if (!entry) return TRUE;
    gtk_entry_set_visibility(entry, ev_cross->type == GDK_ENTER_NOTIFY);
    return TRUE;
}

void cui_agent_set_selected_service(const char *path, const char *name) {
    if (!path) return;
    g_free(service.path);
    service.path = g_strdup(path);
    g_free(service.name);
    service.name = g_strdup(name ? name : "Hidden");
}

static void agent_ok_tethering(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "tethering_ssid"));
    if (!entry || !technology) return;
    connman_technology_set_tethering_identifier(technology, gtk_entry_get_text(entry));
    entry = GTK_ENTRY(gtk_builder_get_object(cui_builder, "tethering_passphrase"));
    if (!entry) return;
    connman_technology_set_tethering_passphrase(technology, gtk_entry_get_text(entry));
    if (tethering) {
        connman_technology_tether(technology, TRUE);
    }
    g_free(technology);
    technology = NULL;
    tethering = FALSE;
    gtk_widget_hide(GTK_WIDGET(tethering_dbox));
    cui_tray_enable();
}

static void agent_popup_tethering_dbox(void) {
    if (!technology) return;
    const char *value;
    GtkEntry *entry;
    value = connman_technology_get_tethering_identifier(technology);
    set_entry(cui_builder, "tethering_ssid", value, "");
    value = connman_technology_get_tethering_passphrase(technology);
    entry = set_entry(cui_builder, "tethering_passphrase", value, "");
    if (!entry) return;
    gtk_entry_set_visibility(entry, FALSE);
    set_widget_sensitive(cui_builder, "tethering_ok", gtk_entry_get_text_length(entry) >= 8);
    gtk_widget_show(GTK_WIDGET(tethering_dbox));
    cui_tray_disable();
}

static gboolean secret_entry_key_release_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(widget);
    GtkWidget *button = user_data;
    if (!entry || !button) return TRUE;
    gtk_widget_set_sensitive(button, gtk_entry_get_text_length(entry) >= 8);
    return TRUE;
}

void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether) {
    if (!path) return;
    g_free(technology);
    technology = g_strdup(path);
    if (!technology) return;
    tethering = tether;
    const char *ssid = connman_technology_get_tethering_identifier(path);
    const char *passphrase = connman_technology_get_tethering_passphrase(path);
    if (tether && (!ssid || !passphrase)) {
        agent_popup_tethering_dbox();
    } else if (tether) {
        connman_technology_tether(path, TRUE);
    } else {
        agent_popup_tethering_dbox();
    }
}

gint cui_load_agent_dialogs(void) {
    GError *error = NULL;
    GtkWidget *button, *entry;
    gtk_builder_add_from_file(cui_builder, CUI_AGENT_DIALOG_UI_PATH, &error);
    if (error) {
        g_warning("Error loading agent dialogs: %s", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    input_dbox = GTK_DIALOG(gtk_builder_get_object(cui_builder, "input_dbox"));
    login_dbox = GTK_DIALOG(gtk_builder_get_object(cui_builder, "login_dbox"));
    error_dbox = GTK_DIALOG(gtk_builder_get_object(cui_builder, "error_dbox"));
    tethering_dbox = GTK_DIALOG(gtk_builder_get_object(cui_builder, "tethering_dbox"));
    if (!input_dbox || !login_dbox || !error_dbox || !tethering_dbox) return -EINVAL;

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "input_ok"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_ok_input), NULL);
    entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "secret_entry"));
    g_signal_connect(entry, "enter-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(entry, "leave-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(entry, "key-release-event", G_CALLBACK(secret_entry_key_release_cb), button);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "passphrase_button"));
    g_signal_connect(button, "toggled", G_CALLBACK(agent_passphrase_changed), NULL);
    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "wpspbc_button"));
    g_signal_connect(button, "toggled", G_CALLBACK(agent_passphrase_changed), NULL);
    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "wpspin_button"));
    g_signal_connect(button, "toggled", G_CALLBACK(agent_passphrase_changed), NULL);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "input_cancel"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_cancel), input_dbox);
    g_signal_connect(input_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(input_dbox, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "login_ok"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_ok_login), NULL);
    entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "login_password"));
    g_signal_connect(entry, "enter-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(entry, "leave-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "login_cancel"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_cancel), login_dbox);
    g_signal_connect(login_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(login_dbox, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "tethering_ok"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_ok_tethering), NULL);
    entry = GTK_WIDGET(gtk_builder_get_object(cui_builder, "tethering_passphrase"));
    g_signal_connect(entry, "enter-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(entry, "leave-notify-event", G_CALLBACK(change_invisible_char_on_entry), entry);
    g_signal_connect(entry, "key-release-event", G_CALLBACK(secret_entry_key_release_cb), button);
    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "tethering_cancel"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_cancel), tethering_dbox);
    g_signal_connect(tethering_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(tethering_dbox, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "error_retry"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_retry), NULL);
    button = GTK_WIDGET(gtk_builder_get_object(cui_builder, "error_cancel"));
    g_signal_connect(button, "clicked", G_CALLBACK(agent_cancel), error_dbox);
    g_signal_connect(error_dbox, "response", G_CALLBACK(agent_close), NULL);
    g_signal_connect(error_dbox, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    return 0;
}

// GtkServiceの実装
struct _GtkServicePrivate {
    GtkBox *box;
    GtkLabel *name;
    GtkImage *state;
    GtkImage *security;
    GtkImage *signal;
    gboolean selected;
};

static void gtk_service_destroy(GtkWidget *widget);
static void gtk_service_class_init(GtkServiceClass *klass);
static void gtk_service_init(GtkService *service);
static void gtk_service_dispose(GObject *object);
static gboolean gtk_service_button_release_event(GtkWidget *widget, GdkEventButton *event);

G_DEFINE_TYPE(GtkService, gtk_service, GTK_TYPE_MENU_ITEM);

static void gtk_service_class_init(GtkServiceClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    GtkMenuItemClass *menu_item_class = GTK_MENU_ITEM_CLASS(klass);

    object_class->dispose = gtk_service_dispose;
    widget_class->destroy = gtk_service_destroy;
    widget_class->button_release_event = gtk_service_button_release_event;
    menu_item_class->hide_on_activate = FALSE;
    g_type_class_add_private(object_class, sizeof(GtkServicePrivate));
}

static void gtk_service_init(GtkService *service) {
    GtkServicePrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(service, GTK_TYPE_SERVICE, GtkServicePrivate);
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
    gtk_widget_set_can_focus(GTK_WIDGET(priv->box), TRUE);
}

static void gtk_service_dispose(GObject *object) {
    G_OBJECT_CLASS(gtk_service_parent_class)->dispose(object);
}

static void gtk_service_destroy(GtkWidget *widget) {
    GtkService *service = GTK_SERVICE(widget);
    GtkServicePrivate *priv = service->priv;
    if (priv && !priv->selected && service->path) {
        connman_service_set_property_changed_callback(service->path, NULL, service);
        connman_technology_set_property_error_callback(service->path, NULL, service);
    }
    g_free(service->path);
    service->path = NULL;
    GTK_WIDGET_CLASS(gtk_service_parent_class)->destroy(widget);
}

static void handle_left_click(GtkService *service) {
    if (!service->path) return;
    if (connman_service_is_connected(service->path)) {
        connman_service_disconnect(service->path);
    } else {
        cui_agent_set_selected_service(service->path, connman_service_get_name(service->path));
        connman_service_connect(service->path);
    }
}

static void handle_right_click(GtkService *service) {
    if (!service->path) return;
    service->priv->selected = TRUE;
    cui_settings_popup(service->path);
}

static gboolean gtk_service_button_release_event(GtkWidget *widget, GdkEventButton *event) {
    GtkService *service = GTK_SERVICE(widget);
    if (!service) return FALSE;
    if (event->button == 1) {
        handle_left_click(service);
    } else if (event->button == 3) {
        handle_right_click(service);
    }
    GtkWidget *parent = gtk_widget_get_parent(widget);
    if (parent && GTK_IS_MENU_SHELL(parent)) {
        GtkMenuShell *menu_shell = GTK_MENU_SHELL(parent);
        gtk_menu_shell_activate_item(menu_shell, widget, TRUE);
    }
    return TRUE;
}

static void service_set_state(GtkService *service) {
    GtkServicePrivate *priv = service->priv;
    if (!service->path || !connman_service_is_connected(service->path)) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->name), "");
        return;
    }
    const char *ip = NULL;
    const struct connman_ipv4 *ipv4 = connman_service_get_ipv4(service->path);
    if (ipv4) {
        ip = ipv4->address;
    } else {
        const struct connman_ipv6 *ipv6 = connman_service_get_ipv6(service->path);
        if (ipv6) ip = ipv6->address;
    }
    gtk_widget_set_tooltip_text(GTK_WIDGET(priv->name), ip ? ip : "");
    GdkPixbuf *image = NULL;
    const char *info = NULL;
    cui_theme_get_state_icone_and_info(connman_service_get_state(service->path), &image, &info);
    if (image) {
        gtk_widget_set_visible(GTK_WIDGET(priv->state), TRUE);
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->state), info);
        gtk_image_set_from_pixbuf(priv->state, image);
    }
}

static void service_set_signal(GtkService *service) {
    GtkServicePrivate *priv = service->priv;
    if (!service->path) return;
    const char *type = connman_service_get_type(service->path);
    GdkPixbuf *image = NULL;
    const char *info = NULL;
    if (g_strcmp0(type, "wifi") == 0) {
        cui_theme_get_signal_icone_and_info(connman_service_get_strength(service->path), &image, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &image, &info);
    }
    if (image) {
        gtk_widget_set_visible(GTK_WIDGET(priv->signal), TRUE);
        gtk_widget_set_tooltip_text(GTK_WIDGET(priv->signal), info);
        gtk_image_set_from_pixbuf(priv->signal, image);
    }
}

static void service_set_name(GtkService *service) {
    if (!service->path) return;
    const char *name = connman_service_get_name(service->path) ? connman_service_get_name(service->path) : "- Hidden -";
    const char *type = connman_service_get_type(service->path);
    const char *security = (g_strcmp0(type, "wifi") == 0) ? connman_service_get_security(service->path) : NULL;
    gboolean favorite = connman_service_is_favorite(service->path);
    char *markup = NULL;
    if (favorite) {
        markup = security ? g_markup_printf_escaped("<b>%s</b> <i>(%s)</i>", name, security) : 
                            g_markup_printf_escaped("<b>%s</b>", name);
    } else {
        markup = security ? g_markup_printf_escaped("%s <i>(%s)</i>", name, security) : 
                            g_markup_printf_escaped("%s", name);
    }
    gtk_label_set_markup(service->priv->name, markup);
    g_free(markup);
}

static void service_property_changed_cb(const char *path, const char *property, void *user_data);
GtkService *gtk_service_new(const char *path) {
    if (!path) return NULL;
    char *path_copy = g_strdup(path);
    if (!path_copy) return NULL;
    GtkService *service = g_object_new(GTK_TYPE_SERVICE, NULL);
    if (!service) {
        g_free(path_copy);
        return NULL;
    }
    service->path = path_copy;
    connman_service_set_property_changed_callback(path_copy, service_property_changed_cb, service);
    service_set_name(service);
    service_set_state(service);
    service_set_signal(service);
    return service;
}

// GtkTechnologyの実装
struct _GtkTechnologyPrivate {
    GtkBox *box;
    GtkSwitch *enabler;
    GtkLabel *name;
    GtkCheckMenuItem *tethering;
};

static void gtk_technology_destroy(GtkWidget *widget);
static void gtk_technology_class_init(GtkTechnologyClass *klass);
static void gtk_technology_init(GtkTechnology *technology);
static void gtk_technology_dispose(GObject *object);
static gboolean gtk_technology_button_release_event(GtkWidget *widget, GdkEventButton *event);

G_DEFINE_TYPE(GtkTechnology, gtk_technology, GTK_TYPE_MENU_ITEM);

static void gtk_technology_class_init(GtkTechnologyClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    GtkMenuItemClass *menu_item_class = GTK_MENU_ITEM_CLASS(klass);

    object_class->dispose = gtk_technology_dispose;
    widget_class->destroy = gtk_technology_destroy;
    widget_class->button_release_event = gtk_technology_button_release_event;
    menu_item_class->hide_on_activate = FALSE;
    g_type_class_add_private(object_class, sizeof(GtkTechnologyPrivate));
}

static void gtk_technology_init(GtkTechnology *technology) {
    GtkTechnologyPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(technology, GTK_TYPE_TECHNOLOGY, GtkTechnologyPrivate);
    technology->priv = priv;
    priv->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    priv->enabler = GTK_SWITCH(gtk_switch_new());
    priv->name = GTK_LABEL(gtk_label_new(NULL));

    gtk_widget_set_margin_start(GTK_WIDGET(priv->enabler), 0);
    gtk_widget_set_margin_end(GTK_WIDGET(priv->enabler), 0);
    gtk_widget_set_margin_top(GTK_WIDGET(priv->enabler), 0);
    gtk_widget_set_margin_bottom(GTK_WIDGET(priv->enabler), 0);
    gtk_widget_set_margin_start(GTK_WIDGET(priv->name), 0);
    gtk_widget_set_margin_end(GTK_WIDGET(priv->name), 0);
    gtk_widget_set_margin_top(GTK_WIDGET(priv->name), 0);
    gtk_widget_set_margin_bottom(GTK_WIDGET(priv->name), 0);

    gtk_box_set_spacing(priv->box, 0);
    gtk_box_set_homogeneous(priv->box, TRUE);
    gtk_widget_set_halign(GTK_WIDGET(priv->name), GTK_ALIGN_START);

    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->enabler), FALSE, FALSE, 0);
    gtk_box_pack_start(priv->box, GTK_WIDGET(priv->name), FALSE, FALSE, 0);

    gtk_widget_set_visible(GTK_WIDGET(priv->box), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->enabler), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(priv->name), TRUE);

    gtk_container_add(GTK_CONTAINER(technology), GTK_WIDGET(priv->box));
    gtk_widget_set_can_focus(GTK_WIDGET(priv->box), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(priv->enabler), TRUE);
}

static void gtk_technology_dispose(GObject *object) {
    G_OBJECT_CLASS(gtk_technology_parent_class)->dispose(object);
}

static void gtk_technology_destroy(GtkWidget *widget) {
    GtkTechnology *technology = GTK_TECHNOLOGY(widget);
    GtkTechnologyPrivate *priv = technology->priv;
    if (technology->path) {
        connman_technology_set_property_changed_callback(technology->path, NULL, technology);
        connman_technology_set_property_error_callback(technology->path, NULL, technology);
    }
    if (priv->tethering) {
        gtk_widget_destroy(GTK_WIDGET(priv->tethering));
        priv->tethering = NULL;
    }
    g_free(technology->path);
    technology->path = NULL;
    GTK_WIDGET_CLASS(gtk_technology_parent_class)->destroy(widget);
}

static gboolean gtk_technology_button_release_event(GtkWidget *widget, GdkEventButton *event) {
    GtkTechnology *technology = GTK_TECHNOLOGY(widget);
    if (!technology || !technology->path) return FALSE;
    if (event->button == 1) {
        gboolean enable = !gtk_switch_get_active(technology->priv->enabler);
        if (connman_technology_enable(technology->path, enable) == 0) {
            gtk_widget_set_sensitive(GTK_WIDGET(technology->priv->enabler), FALSE);
        }
    } else if (event->button == 3) {
        const char *type = connman_technology_get_type(technology->path);
        if (g_strcmp0(type, "wifi") == 0) {
            cui_agent_set_wifi_tethering_settings(technology->path, FALSE);
        }
    }
    GtkWidget *parent = gtk_widget_get_parent(widget);
    if (parent && GTK_IS_MENU_SHELL(parent)) {
        GtkMenuShell *menu_shell = GTK_MENU_SHELL(parent);
        gtk_menu_shell_activate_item(menu_shell, widget, TRUE);
    }
    return TRUE;
}

static void technology_property_error_cb(const char *path, const char *property, int error, void *user_data) {
    if (error) g_warning("Could not set property %s: error %d", property, error);
}

static gboolean gtk_technology_tethering_button(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (event->button != 1) return TRUE;
    GtkTechnology *technology = user_data;
    GtkTechnologyPrivate *priv = technology->priv;
    if (!technology->path) return TRUE;
    gtk_widget_set_sensitive(GTK_WIDGET(priv->tethering), FALSE);
    gboolean tethering = !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), tethering);
    connman_technology_tether(technology->path, tethering);
    return TRUE;
}

static void set_technology_name(GtkTechnology *technology, gboolean enabled) {
    if (!technology->path) return;
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
    if (!path || !technology->path || g_strcmp0(path, technology->path) != 0) return;
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

GtkTechnology *gtk_technology_new(const gchar *path) {
    if (!path) return NULL;
    char *path_copy = g_strdup(path);
    if (!path_copy) return NULL;
    GtkTechnology *technology = g_object_new(GTK_TYPE_TECHNOLOGY, NULL);
    if (!technology) {
        g_free(path_copy);
        return NULL;
    }
    GtkTechnologyPrivate *priv = technology->priv;
    technology->path = path_copy;
    connman_technology_set_property_changed_callback(path_copy, technology_property_changed_cb, technology);
    connman_technology_set_property_error_callback(technology->path, technology_property_error_cb, technology);

    const char *type = connman_technology_get_type(path_copy);
    gtk_widget_set_tooltip_text(GTK_WIDGET(technology), g_strcmp0(type, "wifi") == 0 ?
        _("Left click to enable/disable\nRight click to set tethering information") : _("Left to enable/disable"));

    gboolean enabled = connman_technology_is_enabled(path_copy);
    gtk_switch_set_active(priv->enabler, enabled);
    char *label = g_strdup_printf("via: %s", connman_technology_get_name(path_copy));
    priv->tethering = GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(label));
    g_free(label);
    gtk_widget_set_sensitive(GTK_WIDGET(priv->tethering), enabled);
    set_technology_name(technology, enabled);
    gtk_check_menu_item_set_active(priv->tethering, connman_technology_is_tethering(path_copy));
    gtk_widget_set_visible(GTK_WIDGET(priv->tethering), TRUE);
    g_signal_connect(priv->tethering, "button-release-event", G_CALLBACK(gtk_technology_tethering_button), technology);
    return technology;
}

GtkMenuItem *gtk_technology_get_tethering_item(GtkTechnology *technology) {
    if (!technology) return NULL;
    return GTK_MENU_ITEM(technology->priv->tethering);
}

// 左メニューの実装
static GtkMenu *cui_left_menu = NULL;
static GtkMenu *cui_more_menu = NULL;
static GtkMenuItem *cui_list_more_item = NULL;
static GHashTable *service_items = NULL;
static GtkMenuItem *cui_scan_spinner = NULL;

static void add_or_update_service(const char *path, int position) {
    if (!path) return;
    GtkService *s = g_hash_table_lookup(service_items, path);
    if (s) {
        service_set_name(s);
        service_set_state(s);
        service_set_signal(s);
    } else {
        s = gtk_service_new(path);
        if (!s) return;
        if (position > 9) {
            gtk_menu_shell_append(GTK_MENU_SHELL(cui_more_menu), GTK_WIDGET(s));
        } else {
            gtk_menu_shell_insert(GTK_MENU_SHELL(cui_left_menu), GTK_WIDGET(s), position);
        }
        g_hash_table_insert(service_items, s->path, s);
    }
    gtk_widget_set_visible(GTK_WIDGET(s), TRUE);
    gtk_widget_show(GTK_WIDGET(s));
}

static void remove_service_cb(const char *path) {
    if (!path) return;
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
    int item_position = 2;
    for (GSList *list = services; list; list = list->next, item_position++) {
        add_or_update_service((const char *)list->data, item_position);
    }
    gtk_widget_set_visible(GTK_WIDGET(cui_list_more_item), item_position > 10);
    g_slist_free(services);

    GtkRequisition requisition = {0, 0};
    gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);
    gtk_menu_reposition(cui_left_menu);
}

static void scanning_cb(void *user_data) {
    GtkSpinner *spin = GTK_SPINNER(gtk_bin_get_child(GTK_BIN(cui_scan_spinner)));
    if (!spin) return;
    gtk_spinner_stop(spin);
    gtk_widget_hide(GTK_WIDGET(cui_scan_spinner));
    gtk_widget_hide(GTK_WIDGET(spin));

    GtkRequisition requisition = {0, 0};
    gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);
    gtk_menu_reposition(cui_left_menu);
}

static void delete_service_item(gpointer data) {
    gtk_widget_destroy(GTK_WIDGET(data));
}

static void cui_popup_left_menu(GtkStatusIcon *trayicon, gpointer user_data) {
    GtkSpinner *spin = GTK_SPINNER(gtk_bin_get_child(GTK_BIN(cui_scan_spinner)));
    if (!spin) return;
    gtk_widget_hide(GTK_WIDGET(cui_list_more_item));
    gtk_widget_show(GTK_WIDGET(cui_scan_spinner));
    gtk_widget_show(GTK_WIDGET(spin));
    gtk_spinner_start(spin);
    connman_service_set_removed_callback(remove_service_cb);
    connman_service_refresh_services_list(get_services_cb, scanning_cb, user_data);
    gtk_menu_popup_at_pointer(cui_left_menu, NULL);
}

static void cui_popdown_left_menu(GtkMenu *menu, gpointer user_data) {
    connman_service_set_removed_callback(NULL);
    g_hash_table_remove_all(service_items);
    connman_service_free_services_list();
}

gint cui_load_left_menu(GtkBuilder *builder, GtkStatusIcon *trayicon) {
    if (!builder || !trayicon) return -EINVAL;
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_LEFT_MENU_UI_PATH, &error);
    if (error) {
        g_warning("Error loading left menu: %s", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_left_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_left_menu"));
    cui_more_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_more_menu"));
    cui_list_more_item = GTK_MENU_ITEM(gtk_builder_get_object(builder, "cui_list_more_item"));
    cui_scan_spinner = GTK_MENU_ITEM(gtk_builder_get_object(builder, "cui_scan_spinner"));
    if (!cui_left_menu || !cui_more_menu || !cui_list_more_item || !cui_scan_spinner) return -EINVAL;

    gtk_container_add(GTK_CONTAINER(cui_scan_spinner), gtk_spinner_new());
    g_signal_connect(cui_left_menu, "deactivate", G_CALLBACK(cui_popdown_left_menu), NULL);
    service_items = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, delete_service_item);
    cui_tray_hook_left_menu(cui_popup_left_menu);
    return 0;
}

GtkBuilder *cui_builder;

static void connman_manager_changed(const char *unused, const char *property, void *user_data) {
    if (g_strcmp0(property, "State") == 0) {
        cui_trayicon_update_icon();
    }
}

static void connman_up(void *user_data) {
    connman_manager_init(connman_manager_changed, NULL);
    connman_service_init();
    connman_technology_init();
    connman_agent_init();
    cui_agent_init_callbacks();
    cui_trayicon_update_icon();
    cui_tray_enable();
    cui_right_menu_enable_all();
}

static void connman_down(void *user_data) {
    connman_agent_finalize();
    connman_service_finalize();
    connman_technology_finalize();
    connman_manager_finalize();
    cui_trayicon_update_icon();
    cui_tray_left_menu_disable();
    cui_right_menu_enable_only_quit();
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    gtk_init(&argc, &argv);
    cui_builder = gtk_builder_new();
    if (!cui_builder) {
        g_warning("Failed to create GtkBuilder");
        return -ENOMEM;
    }
    cui_load_theme();
    if (cui_load_trayicon(cui_builder) != 0 || cui_load_agent_dialogs() != 0) {
        g_object_unref(cui_builder);
        return -EINVAL;
    }
    cui_tray_enable();
    int ret = connman_interface_init(connman_up, connman_down, NULL);
    if (ret < 0) {
        g_warning("Connman interface initialization failed: %d", ret);
        g_object_unref(cui_builder);
        return ret;
    }
    gtk_main();
    connman_interface_finalize();
    g_object_unref(cui_builder);
    return 0;
}

// 右メニューの実装
static GtkMenu *cui_right_menu = NULL;
static GtkWidget *cui_item_mode_off = NULL;
static GtkWidget *cui_item_mode_on = NULL;
static GtkMenu *cui_tethering_menu = NULL;
static GHashTable *tech_items = NULL;
static int item_position = 3;
static gboolean disabled = TRUE;

static void add_technology(const char *path) {
    if (!path) return;
    GtkTechnology *t = g_hash_table_lookup(tech_items, path);
    if (!t) {
        t = gtk_technology_new(path);
        if (!t) return;
        gtk_menu_shell_insert(GTK_MENU_SHELL(cui_right_menu), GTK_WIDGET(t), item_position++);
        gtk_menu_shell_append(GTK_MENU_SHELL(cui_tethering_menu), GTK_WIDGET(gtk_technology_get_tethering_item(t)));
        g_hash_table_insert(tech_items, t->path, t);
    }
    gtk_widget_set_visible(GTK_WIDGET(t), TRUE);
    gtk_widget_show(GTK_WIDGET(t));
}

static void technology_added_cb(const char *path) {
    add_technology(path);
}

static void technology_removed_cb(const char *path) {
    if (!path) return;
    g_hash_table_remove(tech_items, path);
}

static void delete_technology_item(gpointer data) {
    gtk_widget_destroy(GTK_WIDGET(data));
    item_position--;
}

static void cui_item_offlinemode_activate(GtkMenuItem *menuitem, gpointer user_data) {
    connman_manager_set_offlinemode(!connman_manager_get_offlinemode());
}

static void cui_item_quit_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gtk_main_quit();
}

static void cui_popup_right_menu(GtkStatusIcon *trayicon, guint button, guint activate_time, gpointer user_data) {
    if (disabled) goto popup;
    gtk_widget_set_visible(cui_item_mode_on, connman_manager_get_offlinemode());
    gtk_widget_set_visible(cui_item_mode_off, !connman_manager_get_offlinemode());
    GList *tech_list = connman_technology_get_technologies();
    for (GList *list = tech_list; list; list = list->next) {
        add_technology((const char *)list->data);
    }
    g_list_free(tech_list);
popup:
    connman_technology_set_removed_callback(technology_removed_cb);
    connman_technology_set_added_callback(technology_added_cb);
    gtk_menu_popup_at_pointer(cui_right_menu, NULL);
}

static void cui_popdown_right_menu(GtkMenu *menu, gpointer user_data) {
    connman_technology_set_removed_callback(NULL);
    connman_technology_set_added_callback(NULL);
    g_hash_table_remove_all(tech_items);
}

static void cui_enable_item(gboolean enable) {
    set_widget_hidden(cui_builder, "cui_item_offlinemode_on", enable);
    set_widget_hidden(cui_builder, "cui_item_offlinemode_off", enable);
    set_widget_hidden(cui_builder, "cui_sep_technology_up", enable);
    set_widget_hidden(cui_builder, "cui_sep_technology_down", enable);
    set_widget_hidden(cui_builder, "cui_item_tethering", enable);
    set_widget_hidden(cui_builder, "cui_sep_tethering", enable);
    set_widget_hidden(cui_builder, "cui_item_quit", FALSE);
    disabled = enable;
}

void cui_right_menu_enable_only_quit(void) {
    cui_enable_item(TRUE);
}

void cui_right_menu_enable_all(void) {
    cui_enable_item(FALSE);
}

gint cui_load_right_menu(GtkBuilder *builder, GtkStatusIcon *trayicon) {
    if (!builder || !trayicon) return -EINVAL;
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_RIGHT_MENU_UI_PATH, &error);
    if (error) {
        g_warning("Error loading right menu: %s", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_right_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_right_menu"));
    cui_item_mode_off = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_off"));
    cui_item_mode_on = GTK_WIDGET(gtk_builder_get_object(builder, "cui_item_offlinemode_on"));
    cui_tethering_menu = GTK_MENU(gtk_builder_get_object(builder, "cui_tethering_menu"));
    if (!cui_right_menu || !cui_item_mode_off || !cui_item_mode_on || !cui_tethering_menu) return -EINVAL;

    g_signal_connect(cui_right_menu, "deactivate", G_CALLBACK(cui_popdown_right_menu), NULL);
    g_signal_connect(gtk_builder_get_object(builder, "cui_item_quit"), "activate", G_CALLBACK(cui_item_quit_activate), NULL);
    g_signal_connect(cui_item_mode_off, "activate", G_CALLBACK(cui_item_offlinemode_activate), NULL);
    g_signal_connect(cui_item_mode_on, "activate", G_CALLBACK(cui_item_offlinemode_activate), NULL);

    tech_items = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, delete_technology_item);
    GtkImageMenuItem *cui_item_tethering = GTK_IMAGE_MENU_ITEM(gtk_builder_get_object(builder, "cui_item_tethering"));
    GdkPixbuf *image = NULL;
    cui_theme_get_tethering_icone_and_info(&image, NULL);
    if (image) {
        GtkWidget *image_widget = gtk_bin_get_child(GTK_BIN(cui_item_tethering));
        if (GTK_IS_IMAGE(image_widget)) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), image);
        }
    }
    cui_tray_hook_right_menu(cui_popup_right_menu);
    return 0;
}

// 設定ポップアップの実装
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

static void toggle_button(const char *name) {
    GtkToggleButton *button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, name));
    if (button) gtk_toggle_button_set_active(button, TRUE);
}

static void favorite_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!path || !gtk_toggle_button_get_active(togglebutton)) {
        connman_service_remove(path);
    }
}

static void autoconnect_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!path) return;
    connman_service_set_autoconnectable(path, gtk_toggle_button_get_active(togglebutton));
}

static void update_header(void) {
    if (!path) return;
    const char *type = connman_service_get_type(path);
    GdkPixbuf *image = NULL;
    const char *info = NULL;
    if (g_strcmp0(type, "wifi") == 0) {
        cui_theme_get_signal_icone_and_info(connman_service_get_strength(path), &image, &info);
    } else {
        cui_theme_get_type_icone_and_info(type, &image, &info);
    }
    set_image(builder, "service_type", image, info);
    set_label(builder, "service_name", connman_service_get_name(path), "- Hidden -");

    if (connman_service_is_connected(path)) {
        cui_theme_get_state_icone_and_info(connman_service_get_state(path), &image, &info);
        set_image(builder, "service_state", image, info);
    } else {
        set_image(builder, "service_state", NULL, "");
    }
    set_label(builder, "service_error", connman_service_get_error(path), "");

    gboolean favorite = connman_service_is_favorite(path);
    GtkWidget *widget = set_button_toggle(builder, "service_autoconnect", connman_service_is_autoconnect(path));
    gtk_widget_set_sensitive(widget, favorite);
    if (favorite) {
        g_signal_connect(widget, "toggled", G_CALLBACK(autoconnect_button_toggled), NULL);
    }
    widget = set_button_toggle(builder, "service_favorite", favorite);
    gtk_widget_set_sensitive(widget, favorite);
    if (favorite) {
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
    if (!path) return;
    const struct connman_ipv4 *ipv4 = connman_service_get_ipv4(path);
    const struct connman_ipv4 *ipv4_conf = connman_service_get_ipv4_config(path);
    if (!ipv4) {
        set_entry(builder, "ipv4_address", "", "");
        set_entry(builder, "ipv4_netmask", "", "");
        set_entry(builder, "ipv4_gateway", "", "");
    } else {
        set_ipv4_method(ipv4->method);
        set_entry(builder, "ipv4_address", ipv4->address, "");
        set_entry(builder, "ipv4_netmask", ipv4->netmask, "");
        set_entry(builder, "ipv4_gateway", ipv4->gateway, "");
    }
    if (!ipv4_conf) {
        set_entry(builder, "ipv4_conf_address", "", "");
        set_entry(builder, "ipv4_conf_netmask", "", "");
        set_entry(builder, "ipv4_conf_gateway", "", "");
        if (!ipv4) set_widget_sensitive(builder, "ipv4_settings", FALSE);
        return;
    }
    if (!ipv4) set_ipv4_method(ipv4_conf->method);
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
    if (g_strcmp0(privacy, "enabled") == 0) toggle_button("ipv6_priv_enabled");
    else if (g_strcmp0(privacy, "prefered") == 0) toggle_button("ipv6_priv_prefered");
    else toggle_button("ipv6_priv_disabled");
}

static void update_ipv6(void) {
    if (!path) return;
    const struct connman_ipv6 *ipv6 = connman_service_get_ipv6(path);
    const struct connman_ipv6 *ipv6_conf = connman_service_get_ipv6_config(path);
    char value[6] = {0};
    if (!ipv6) {
        set_entry(builder, "ipv6_address", "", "");
        set_entry(builder, "ipv6_prefix_length", "", "");
        set_entry(builder, "ipv6_gateway", "", "");
    } else {
        snprintf(value, 6, "%u", ipv6->prefix);
        set_ipv6_method(ipv6->method);
        set_label(builder, "ipv6_address", ipv6->address, "");
        set_label(builder, "ipv6_prefix_length", value, "");
        set_label(builder, "ipv6_gateway", ipv6->gateway, "");
        set_ipv6_privacy(ipv6->privacy);
    }
    if (!ipv6_conf) {
        set_entry(builder, "ipv6_conf_address", "", "");
        set_entry(builder, "ipv6_conf_prefix_length", "", "");
        set_entry(builder, "ipv6_conf_gateway", "", "");
        if (!ipv6) set_widget_sensitive(builder, "ipv6_settings", FALSE);
        return;
    }
    snprintf(value, 6, "%u", ipv6_conf->prefix);
    if (!ipv6) set_ipv6_method(ipv6_conf->method);
    set_entry(builder, "ipv6_conf_address", ipv6_conf->address, "");
    set_entry(builder, "ipv6_conf_prefix_length", value, "");
    set_entry(builder, "ipv6_conf_gateway", ipv6_conf->gateway, "");
    if (!ipv6) set_ipv6_privacy(ipv6_conf->privacy);
}

static void update_dns(void) {
    if (!path) return;
    set_entry(builder, "nameservers", connman_service_get_nameservers(path), "");
    set_entry(builder, "nameservers_conf", connman_service_get_nameservers_config(path), "");
    set_entry(builder, "domains", connman_service_get_domains(path), "");
    set_entry(builder, "domains_conf", connman_service_get_domains_config(path), "");
}

static void update_timeservers(void) {
    if (!path) return;
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
    if (!path) return;
    const struct connman_proxy *proxy = connman_service_get_proxy(path);
    const struct connman_proxy *proxy_conf = connman_service_get_proxy_config(path);
    if (!proxy) {
        set_label(builder, "proxy_method", "", "");
        set_entry(builder, "proxy_url", "", "");
        set_entry(builder, "proxy_servers", "", "");
        set_entry(builder, "proxy_excludes", "", "");
    } else {
        set_proxy_method(proxy->method);
        set_entry(builder, "proxy_url", proxy->url, "");
        set_entry(builder, "proxy_servers", proxy->servers, "");
        set_entry(builder, "proxy_excludes", proxy->excludes, "");
    }
    if (!proxy_conf) {
        set_entry(builder, "proxy_conf_url", "", "");
        set_entry(builder, "proxy_conf_servers", "", "");
        set_entry(builder, "proxy_conf_excludes", "", "");
        if (!proxy) set_widget_sensitive(builder, "proxy_settings", FALSE);
        return;
    }
    if (!proxy) set_proxy_method(proxy_conf->method);
    set_entry(builder, "proxy_conf_url", proxy_conf->url, "");
    set_entry(builder, "proxy_conf_servers", proxy_conf->servers, "");
    set_entry(builder, "proxy_conf_excludes", proxy_conf->excludes, "");
    set_widget_sensitive(builder, "proxy_settings", TRUE);
}

static void update_provider(void) {
    if (!path) return;
    const struct connman_provider *provider = connman_service_get_provider(path);
    if (!provider) {
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
    if (!path) return;
    const struct connman_ethernet *ethernet = connman_service_get_ethernet(path);
    char value[6] = {0};
    if (!ethernet) {
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
    snprintf(value, 6, "%u", ethernet->mtu);
    set_label(builder, "ethernet_mtu", value, "");
    snprintf(value, 6, "%u", ethernet->speed);
    set_label(builder, "ethernet_speed", value, "");
    set_label(builder, "ethernet_duplex", ethernet->duplex, "");
    set_widget_sensitive(builder, "ethernet_settings", TRUE);
}

static void service_property_changed_cb(const char *path, const char *property, void *user_data) {
    update_header();
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
    ipv4_changed = ipv6_changed = proxy_changed = nameservers_changed = domains_changed = timeservers_changed = FALSE;
    if (path) connman_service_set_property_changed_callback(path, service_property_changed_cb, NULL);
}

static void service_property_error_cb(const char *path, const char *property, int error, void *user_data) {
    g_warning("Error setting %s: %d", property, error);
    service_property_set_cb(NULL, NULL, NULL);
}

static void toggled_ipv4_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    const char *name = user_data;
    ipv4_method = g_strcmp0(name, "ipv4_manual") == 0 ? "manual" : 
                  g_strcmp0(name, "ipv4_dhcp") == 0 ? "dhcp" : "off";
    enable_ipv4_config(g_strcmp0(ipv4_method, "manual") == 0);
    ipv4_changed = TRUE;
}

static void toggled_ipv6_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    const char *name = user_data;
    ipv6_method = g_strcmp0(name, "ipv6_manual") == 0 ? "manual" : 
                  g_strcmp0(name, "ipv6_auto") == 0 ? "auto" : "off";
    enable_ipv6_config(g_strcmp0(ipv6_method, "manual") == 0);
    set_widget_sensitive(builder, "ipv6_priv_box", g_strcmp0(ipv6_method, "auto") == 0);
    ipv6_changed = TRUE;
}

static void toggled_ipv6_privacy_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    const char *name = user_data;
    ipv6_privacy = g_strcmp0(name, "ipv6_priv_enabled") == 0 ? "enabled" : 
                   g_strcmp0(name, "ipv6_priv_disabled") == 0 ? "disabled" : "prefered";
    ipv6_changed = TRUE;
}

static void toggled_proxy_method_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    const char *name = user_data;
    proxy_method = g_strcmp0(name, "proxy_manual") == 0 ? "manual" : 
                   g_strcmp0(name, "proxy_direct") == 0 ? "direct" : "auto";
    enable_proxy_config(g_strcmp0(proxy_method, "manual") == 0);
    proxy_changed = TRUE;
}

static gboolean key_released_entry_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    *(gboolean *)user_data = TRUE;
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
    if (service_settings_dbox) {
        gtk_widget_destroy(GTK_WIDGET(service_settings_dbox));
        service_settings_dbox = NULL;
    }
    if (path) {
        connman_service_set_property_changed_callback(path, NULL, NULL);
        connman_service_set_property_error_callback(path, NULL, NULL);
        connman_service_deselect();
        g_free(path);
        path = NULL;
    }
    cui_tray_enable();
}

static void settings_ok_callback(GtkButton *button, gpointer user_data) {
    if (!path) return;
    int ret = 0;
    connman_service_set_property_changed_callback(path, service_property_set_cb, NULL);
    if (ipv4_changed) {
        struct connman_ipv4 ipv4;
        get_ipv4_configuration(&ipv4);
        ret = connman_service_set_ipv4_config(path, &ipv4);
    }
    if (!ret && ipv6_changed) {
        struct connman_ipv6 ipv6;
        get_ipv6_configuration(&ipv6);
        ret = connman_service_set_ipv6_config(path, &ipv6);
    }
    if (!ret && proxy_changed) {
        struct connman_proxy proxy;
        get_proxy_configuration(&proxy);
        ret = connman_service_set_proxy_config(path, &proxy);
    }
    if (!ret && nameservers_changed) {
        ret = connman_service_set_nameservers_config(path, get_entry_text(builder, "nameservers_conf"));
    }
    if (!ret && domains_changed) {
        ret = connman_service_set_domains_config(path, get_entry_text(builder, "domains_conf"));
    }
    if (!ret && timeservers_changed) {
        ret = connman_service_set_timeservers_config(path, get_entry_text(builder, "timerservers_conf"));
    }
    if (ret) g_warning("Unable to set property, code %d", ret);
}

static void settings_close_callback(GtkDialog *dialog_box, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_DELETE_EVENT || response_id == GTK_RESPONSE_CLOSE) {
        settings_cancel_callback(NULL, NULL);
    }
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
    if (!selected_path) return -EINVAL;
    GError *error = NULL;
    if (!builder) builder = gtk_builder_new();
    if (!builder) return -ENOMEM;
    gtk_builder_add_from_file(builder, CUI_SETTINGS_UI_PATH, &error);
    if (error) {
        g_warning("Error loading settings: %s", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    connman_service_select(selected_path);
    g_free(path);
    path = g_strdup(selected_path);
    service_settings_dbox = GTK_DIALOG(gtk_builder_get_object(builder, "service_settings_dbox"));
    if (!service_settings_dbox) return -EINVAL;
    gtk_widget_show(GTK_WIDGET(service_settings_dbox));
    connman_service_set_property_changed_callback(path, service_property_changed_cb, NULL);
    connman_service_set_property_error_callback(path, service_property_error_cb, NULL);
    settings_connect_signals();
    service_property_set_cb(NULL, NULL, NULL);
    cui_tray_disable();
    return 0;
}

// テーマ関連の実装
static GtkIconTheme *icon_theme = NULL;
static GHashTable *state_icon_cache = NULL;

void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image, const char **info) {
    if (!icon_theme) return;
    if (image) *image = gtk_icon_theme_load_icon(icon_theme, "nm-adhoc", 24, 0, NULL);
    if (info) *info = _("Tethering");
}

void cui_theme_get_type_icone_and_info(const char *type, GdkPixbuf **image, const char **info) {
    if (!icon_theme || !type) return;
    const char *nfo = NULL;
    GdkPixbuf *img = NULL;
    if (g_strcmp0(type, "ethernet") == 0) {
        img = gtk_icon_theme_load_icon(icon_theme, "network-wired-symbolic", 22, 0, NULL);
        nfo = _("Ethernet");
    } else if (g_strcmp0(type, "cellular") == 0) {
        img = gtk_icon_theme_load_icon(icon_theme, "gsm-3g-full", 22, 0, NULL);
        nfo = _("Cellular");
    }
    if (image) *image = img;
    if (info) *info = nfo;
}

void cui_theme_get_signal_icone_and_info(uint8_t signal_strength, GdkPixbuf **image, const char **info) {
    if (!icon_theme) return;
    const char *nfo;
    GdkPixbuf *img;
    if (signal_strength >= 80) {
        img = gtk_icon_theme_load_icon(icon_theme, "network-wireless-signal-excellent-symbolic", 22, 0, NULL);
        nfo = _("Very good signal");
    } else if (signal_strength >= 60) {
        img = gtk_icon_theme_load_icon(icon_theme, "network-wireless-signal-good-symbolic", 22, 0, NULL);
        nfo = _("Good signal");
    } else if (signal_strength >= 40) {
        img = gtk_icon_theme_load_icon(icon_theme, "network-wireless-signal-ok-symbolic", 22, 0, NULL);
        nfo = _("Low signal");
    } else {
        img = gtk_icon_theme_load_icon(icon_theme, "network-wireless-signal-weak-symbolic", 22, 0, NULL);
        nfo = _("Very low signal");
    }
    if (image) *image = img;
    if (info) *info = nfo;
}

void cui_theme_get_state_icone_and_info(enum connman_state state, GdkPixbuf **image, const char **info) {
    if (!icon_theme) return;
    if (!state_icon_cache) {
        state_icon_cache = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    gpointer key = GINT_TO_POINTER(state);
    gpointer value = g_hash_table_lookup(state_icon_cache, key);
    if (value) {
        if (image) *image = GDK_PIXBUF(value);
        if (info) *info = g_hash_table_lookup(state_icon_cache, GINT_TO_POINTER(state + 1000));
        return;
    }
    const char *nfo = NULL;
    GdkPixbuf *img = NULL;
    switch (state) {
        case CONNMAN_STATE_UNKNOWN:
            img = gtk_icon_theme_load_icon(icon_theme, "network-offline-symbolic", 24, 0, NULL);
            nfo = _("Connman is not running");
            break;
        case CONNMAN_STATE_READY:
            img = gtk_icon_theme_load_icon(icon_theme, "network-idle-symbolic", 24, 0, NULL);
            nfo = _("Connected");
            break;
        case CONNMAN_STATE_ONLINE:
            img = gtk_icon_theme_load_icon(icon_theme, "network-transmit-receive-symbolic", 24, 0, NULL);
            nfo = _("Online");
            break;
        default:
            img = gtk_icon_theme_load_icon(icon_theme, "network-offline-symbolic", 24, 0, NULL);
            nfo = _("Disconnected");
            break;
    }
    g_hash_table_insert(state_icon_cache, key, g_object_ref(img));
    g_hash_table_insert(state_icon_cache, GINT_TO_POINTER(state + 1000), (gpointer)nfo);
    if (image) *image = img;
    if (info) *info = nfo;
}

void cui_load_theme(void) {
    icon_theme = gtk_icon_theme_get_default();
    if (icon_theme) {
        gtk_icon_theme_append_search_path(icon_theme, CUI_ICON_PATH);
    }
}

// トレイアイコンの実装
static GtkStatusIcon *cui_trayicon = NULL;
static void (*popup_left_menu_f)(GtkStatusIcon *, gpointer);
static void (*popup_right_menu_f)(GtkStatusIcon *, guint, guint, gpointer);
static int left_menu_handler_id = 0;
static int right_menu_handler_id = 0;

static enum connman_state get_current_state(void) {
    return connman_manager_get_state();
}

static void set_tray_icon(GtkStatusIcon *trayicon, enum connman_state state) {
    if (!trayicon) return;
    GdkPixbuf *image = NULL;
    const char *info = NULL;
    cui_theme_get_state_icone_and_info(state, &image, &info);
    gtk_status_icon_set_from_pixbuf(trayicon, image);
    gtk_status_icon_set_tooltip_text(trayicon, info);
}

void cui_trayicon_update_icon(void) {
    set_tray_icon(cui_trayicon, get_current_state());
    if (cui_trayicon) {
        gtk_status_icon_set_visible(cui_trayicon, TRUE);
    }
}

void cui_tray_hook_left_menu(gpointer callback) {
    popup_left_menu_f = callback;
}

void cui_tray_hook_right_menu(gpointer callback) {
    popup_right_menu_f = callback;
}

void cui_tray_left_menu_disable(void) {
    if (left_menu_handler_id) {
        g_signal_handler_disconnect(cui_trayicon, left_menu_handler_id);
        left_menu_handler_id = 0;
    }
}

void cui_tray_enable(void) {
    if (!cui_trayicon) return;
    if (!left_menu_handler_id) {
        left_menu_handler_id = g_signal_connect(cui_trayicon, "activate", G_CALLBACK(popup_left_menu_f), NULL);
    }
    if (!right_menu_handler_id) {
        right_menu_handler_id = g_signal_connect(cui_trayicon, "popup-menu", G_CALLBACK(popup_right_menu_f), NULL);
    }
}

void cui_tray_disable(void) {
    if (!cui_trayicon) return;
    if (left_menu_handler_id) g_signal_handler_disconnect(cui_trayicon, left_menu_handler_id);
    if (right_menu_handler_id) g_signal_handler_disconnect(cui_trayicon, right_menu_handler_id);
    left_menu_handler_id = right_menu_handler_id = 0;
}

gint cui_load_trayicon(GtkBuilder *builder) {
    if (!builder) return -EINVAL;
    GError *error = NULL;
    gtk_builder_add_from_file(builder, CUI_TRAYICON_UI_PATH, &error);
    if (error) {
        g_warning("Error loading trayicon: %s", error->message);
        g_error_free(error);
        return -EINVAL;
    }
    cui_trayicon = GTK_STATUS_ICON(gtk_builder_get_object(builder, "cui_trayicon"));
    if (!cui_trayicon) return -EINVAL;
    cui_trayicon_update_icon();
    cui_load_left_menu(builder, cui_trayicon);
    cui_load_right_menu(builder, cui_trayicon);
    return 0;
}

// UIユーティリティ関数の実装
GtkLabel *set_label(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    if (!builder || !name) return NULL;
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, name));
    if (!label) {
        g_warning("Label %s not found", name);
        return NULL;
    }
    gtk_label_set_text(label, value ? value : default_value);
    return label;
}

GtkEntry *set_entry(GtkBuilder *builder, const char *name, const char *value, const char *default_value) {
    if (!builder || !name) return NULL;
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, name));
    if (!entry) {
        g_warning("Entry %s not found", name);
        return NULL;
    }
    gtk_entry_set_text(entry, value ? value : default_value);
    return entry;
}

GtkWidget *set_widget_sensitive(GtkBuilder *builder, const char *name, gboolean value) {
    if (!builder || !name) return NULL;
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) {
        g_warning("Widget %s not found", name);
        return NULL;
    }
    gtk_widget_set_sensitive(widget, value);
    return widget;
}

GtkWidget *set_widget_hidden(GtkBuilder *builder, const char *name, gboolean value) {
    if (!builder || !name) return NULL;
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) {
        g_warning("Widget %s not found", name);
        return NULL;
    }
    gtk_widget_set_visible(widget, value);
    value ? gtk_widget_hide(widget) : gtk_widget_show(widget);
    return widget;
}

GtkWidget *set_button_toggle(GtkBuilder *builder, const char *name, gboolean active) {
    if (!builder || !name) return NULL;
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (!widget) {
        g_warning("Button %s not found", name);
        return NULL;
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), active);
    return widget;
}

GtkImage *set_image(GtkBuilder *builder, const char *name, GdkPixbuf *image, const char *info) {
    if (!builder || !name) return NULL;
    GtkImage *img_widget = GTK_IMAGE(gtk_builder_get_object(builder, name));
    if (!img_widget) {
        g_warning("Image %s not found", name);
        return NULL;
    }
    if (image) {
        gtk_widget_set_visible(GTK_WIDGET(img_widget), TRUE);
        gtk_widget_set_tooltip_text(GTK_WIDGET(img_widget), info);
        gtk_image_set_from_pixbuf(img_widget, image);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(img_widget), FALSE);
    }
    return img_widget;
}

void set_signal_callback(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler, gpointer user_data) {
    if (!builder || !name || !signal_name || !handler) return;
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, name));
    if (widget) g_signal_connect(widget, signal_name, handler, user_data);
}

const char *get_entry_text(GtkBuilder *builder, const char *name) {
    if (!builder || !name) return NULL;
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, name));
    return entry ? gtk_entry_get_text(entry) : NULL;
}
