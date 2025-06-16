#ifndef __CONNMAN_UI_GTK_H__
#define __CONNMAN_UI_GTK_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <locale.h>
#include <libintl.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <connman-interface.h>

struct cui_selected_service {
	char *path;
	char *name;
};

extern GtkBuilder *cui_builder;

void cui_load_theme(void);
void cui_theme_get_type_icone_and_info(const char *type,
					GdkPixbuf **image, const char **info);
void cui_theme_get_signal_icone_and_info(uint8_t signal_strength,
					GdkPixbuf **image, const char **info);
void cui_theme_get_state_icone_and_info(enum connman_state state,
					GdkPixbuf **image, const char **info);
void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image,
							const char **info);

gint cui_load_trayicon(GtkBuilder *builder);
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
gint cui_load_left_menu(GtkBuilder *builder, GtkStatusIcon *trayicon);
gint cui_load_right_menu(GtkBuilder *builder, GtkStatusIcon *trayicon);
void cui_right_menu_enable_only_quit(void);
void cui_right_menu_enable_all(void);

gint cui_settings_popup(const char *path);

GtkLabel *set_label(GtkBuilder *builder, const char *name,
				const char *value, const char *default_value);
GtkEntry *set_entry(GtkBuilder *builder, const char *name,
				const char *value, const char *default_value);
GtkWidget *set_widget_sensitive(GtkBuilder *builder,
					const char *name, gboolean value);
GtkWidget *set_widget_hidden(GtkBuilder *builder,
					const char *name, gboolean value);
GtkWidget *set_button_toggle(GtkBuilder *builder,
					const char *name, gboolean active);
GtkImage *set_image(GtkBuilder *builder,
			const char *name, GdkPixbuf *image, const char *info);
void set_signal_callback(GtkBuilder *builder,
				const char *name, const char *signal_name,
				GCallback handler, gpointer user_data);
const char *get_entry_text(GtkBuilder *builder, const char *name);

#endif /* __CONNMAN_UI_GTK_H__ */
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __GTK_SERVICE_H__
#define __GTK_SERVICE_H__

#include <gtk/gtk.h>

#include <connman-interface.h>

G_BEGIN_DECLS

#define GTK_TYPE_SERVICE \
	(gtk_service_get_type())
#define GTK_SERVICE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
			GTK_TYPE_SERVICE, GtkService))
#define GTK_SERVICE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), \
			GTK_TYPE_SERVICE, GtkServiceClass))
#define GTK_IS_SERVICE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_SERVICE))
#define GTK_IS_SERVICE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_SERVICE))
#define GTK_SERVICE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), \
			GTK_TYPE_SERVICE, GtkServiceClass))

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

#endif /* __GTK_SERVICE_H__ */
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __GTK_TECHNOLOGY_H__
#define __GTK_TECHNOLOGY_H__

#include <gtk/gtk.h>

#include <connman-interface.h>

G_BEGIN_DECLS

#define GTK_TYPE_TECHNOLOGY \
	(gtk_technology_get_type())
#define GTK_TECHNOLOGY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
			GTK_TYPE_TECHNOLOGY, GtkTechnology))
#define GTK_TECHNOLOGY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), \
			GTK_TYPE_TECHNOLOGY, GtkTechnologyClass))
#define GTK_IS_TECHNOLOGY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_TECHNOLOGY))
#define GTK_IS_TECHNOLOGY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_TECHNOLOGY))
#define GTK_TECHNOLOGY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), \
			GTK_TYPE_TECHNOLOGY, GtkTechnologyClass))

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

#endif /* __GTK_TECHNOLOGY_H__ */

/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>

#define CUI_AGENT_DIALOG_UI_PATH CUI_UI_PATH "/agent.ui"

static GtkDialog *input_dbox = NULL;
static GtkDialog *login_dbox = NULL;
static GtkDialog *error_dbox = NULL;
static GtkDialog *tethering_dbox = NULL;

static struct cui_selected_service service;
static char *technology = NULL;
static gboolean tethering = FALSE;

static void agent_popup_error_dbox(const char *error)
{
	GtkLabel *label;

	label = (GtkLabel *)gtk_builder_get_object(cui_builder,
							"error_message");
	gtk_label_set_text(label, error);

	gtk_widget_show(GTK_WIDGET(error_dbox));

	cui_tray_disable();
}

static void agent_error_cb(const char *path, const char *error)
{
	agent_popup_error_dbox(error);
}

static void agent_browser_cb(const char *path, const char *url)
{
	return;
}

static void agent_popup_input_dbox(gboolean hidden, gboolean passphrase,
				const char *previous_passphrase,
				gboolean wpspin, const char *previous_wpspin)
{
	GtkToggleButton *button;
	gchar *previous_value;
	GtkLabel *label;
	GtkEntry *entry;

	set_label(cui_builder, "service_label", service.name, "");

	/* Hidden settings */
	button = (GtkToggleButton *) gtk_builder_get_object(cui_builder,
							"name_button");
	gtk_widget_set_visible(GTK_WIDGET(button), hidden);
	button = (GtkToggleButton *) gtk_builder_get_object(cui_builder,
							"ssid_button");
	gtk_widget_set_visible(GTK_WIDGET(button), hidden);
	entry = (GtkEntry *) gtk_builder_get_object(cui_builder,
							"hidden_entry");
	gtk_widget_set_visible((GtkWidget *)entry, hidden);
	gtk_entry_set_text(entry, "");

	/* Passphrase/WPS settings */
	button = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"passphrase_button");
	gtk_toggle_button_set_active(button, TRUE);
	entry = (GtkEntry *)gtk_builder_get_object(cui_builder,
							"secret_entry");
	gtk_entry_set_text(entry, "");
	gtk_entry_set_visibility(entry, FALSE);
	set_widget_sensitive(cui_builder, "input_ok", FALSE);

	/* Setting WPS part */
	button = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"wpspbc_button");
	gtk_widget_set_visible(GTK_WIDGET(button), wpspin);
	gtk_toggle_button_set_active(button, FALSE);

	button = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"wpspin_button");
	gtk_widget_set_visible(GTK_WIDGET(button), FALSE); /* Disabling PIN */
	gtk_toggle_button_set_active(button, FALSE);

	label = (GtkLabel *)gtk_builder_get_object(cui_builder,
							"previous_label");
	gtk_widget_set_visible(GTK_WIDGET(label), TRUE);

	if (previous_passphrase != NULL) {
		previous_value = g_strdup_printf(_("Previous Passphrase:\n%s"),
							previous_passphrase);
		gtk_label_set_text(label, previous_value);
	}/* else if (previous_wpspin != NULL) {
		previous_value = g_strdup_printf(_("Previous WPS PIN:\n%s"),
							previous_wpspin);
		gtk_label_set_text(label, previous_value);
	}*/ else {
		gtk_label_set_text(label, "");
		gtk_widget_set_visible(GTK_WIDGET(label), FALSE);
	}

	gtk_widget_show(GTK_WIDGET(input_dbox));

	cui_tray_disable();
}

static void agent_popup_login_dbox(void)
{
	GtkEntry *entry;

	set_entry(cui_builder, "login_username", "", "");

	entry = set_entry(cui_builder, "login_password", "", "");
	gtk_entry_set_visibility(entry, FALSE);

	gtk_widget_show(GTK_WIDGET(login_dbox));

	cui_tray_disable();
}

static void agent_input_cb(const char *path, gboolean hidden,
			gboolean identity, gboolean passphrase,
			const char *previous_passphrase, gboolean wpspin,
			const char *previous_wpspin, gboolean login)
{
	if (g_strcmp0(path, service.path) != 0) {
		connman_agent_reply_canceled();
		return;
	}

	if (passphrase == TRUE || hidden == TRUE)
		agent_popup_input_dbox(hidden, passphrase,
				previous_passphrase, wpspin, previous_wpspin);
	else if (login == TRUE)
		agent_popup_login_dbox();
	else
		connman_agent_reply_canceled();
}

static void agent_cancel_cb(void)
{
	gtk_widget_hide(GTK_WIDGET(input_dbox));
	cui_tray_enable();
}

static void agent_ok_input(GtkButton *button, gpointer user_data)
{
	const char *name, *passphrase, *wpspin;
	GtkToggleButton *toggle;
	gboolean wps = FALSE;
	GtkEntry *entry;

	name = passphrase = wpspin = NULL;

	entry = (GtkEntry *)gtk_builder_get_object(cui_builder,
							"hidden_entry");
	if (gtk_entry_get_text_length(entry) > 0)
		name = gtk_entry_get_text(entry);

	entry = (GtkEntry *)gtk_builder_get_object(cui_builder,
							"secret_entry");

	toggle = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"passphrase_button");
	if (gtk_toggle_button_get_active(toggle) == TRUE)
		passphrase = gtk_entry_get_text(entry);

	toggle = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"wpspbc_button");
	if (gtk_toggle_button_get_active(toggle) == TRUE) {
		wps = TRUE;
		wpspin = "";
	}

	toggle = (GtkToggleButton *)gtk_builder_get_object(cui_builder,
							"wpspin_button");
	if (gtk_toggle_button_get_active(toggle) == TRUE) {
		wps = TRUE;
		wpspin = gtk_entry_get_text(entry);
	}

	connman_agent_reply_passphrase(name, passphrase, wps, wpspin);
	gtk_widget_hide(GTK_WIDGET(input_dbox));

	cui_tray_enable();
}

static void agent_ok_login(GtkButton *button, gpointer user_data)
{
	GtkEntry *entry;
	const char *username, *password;

	username = password = NULL;

	entry = (GtkEntry *)gtk_builder_get_object(cui_builder,
							"login_username");
	if (gtk_entry_get_text_length(entry) > 0)
		username = gtk_entry_get_text(entry);

	entry = (GtkEntry *)gtk_builder_get_object(cui_builder,
							"login_password");
	if (gtk_entry_get_text_length(entry) > 0)
		password = gtk_entry_get_text(entry);

	connman_agent_reply_login(username, password);
	gtk_widget_hide(GTK_WIDGET(login_dbox));

	cui_tray_enable();
}

static void cancel(void)
{
	cui_tray_enable();

	connman_agent_reply_canceled();
}

static void agent_cancel(GtkButton *button, gpointer user_data)
{
	GtkDialog *dialog_box = user_data;

	gtk_widget_hide(GTK_WIDGET(dialog_box));

	cancel();
}

static void agent_close(GtkDialog *dialog_box,
			gint response_id, gpointer user_data)
{
	if (response_id == GTK_RESPONSE_DELETE_EVENT ||
				response_id == GTK_RESPONSE_CLOSE)
		cancel();
}

static void agent_retry(GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(error_dbox));
	cui_tray_enable();

	connman_agent_reply_retry();
}

static void agent_passphrase_changed(GtkToggleButton *togglebutton,
						gpointer user_data)
{
	const gchar *label;
	GtkWidget *entry;

	entry = (GtkWidget *)gtk_builder_get_object(cui_builder,
							"secret_entry");

	label = gtk_button_get_label(GTK_BUTTON(togglebutton));
	if (g_strcmp0(label, "WPS Push-Button") == 0) {
		gtk_widget_set_sensitive(entry, FALSE);
		set_widget_sensitive(cui_builder, "input_ok", TRUE);
	} else
		gtk_widget_set_sensitive(entry, TRUE);
}

void cui_agent_init_callbacks(void)
{
	connman_agent_set_error_cb(agent_error_cb);
	connman_agent_set_browser_cb(agent_browser_cb);
	connman_agent_set_input_cb(agent_input_cb);
	connman_agent_set_cancel_cb(agent_cancel_cb);
}

static gboolean change_invisible_char_on_entry(GtkWidget *widget,
					GdkEvent *event, gpointer user_data)
{
	GdkEventCrossing *ev_cross = (GdkEventCrossing *) event;
	GtkEntry *entry = user_data;

	if (ev_cross->type == GDK_ENTER_NOTIFY)
		gtk_entry_set_visibility(entry, TRUE);
	else
		gtk_entry_set_visibility(entry, FALSE);

	return TRUE;
}

void cui_agent_set_selected_service(const char *path, const char *name)
{
	g_free(service.path);
	service.path = g_strdup(path);

	g_free(service.name);
	if (name == NULL)
		service.name = g_strdup("Hidden");
	else
		service.name = g_strdup(name);
}

static void agent_ok_tethering(GtkButton *button, gpointer user_data)
{
	GtkEntry *entry;

	entry = (GtkEntry *) gtk_builder_get_object(cui_builder,
							"tethering_ssid");
	connman_technology_set_tethering_identifier(technology,
						gtk_entry_get_text(entry));
	entry = (GtkEntry *) gtk_builder_get_object(cui_builder,
						"tethering_passphrase");
	connman_technology_set_tethering_passphrase(technology,
						gtk_entry_get_text(entry));

	if (tethering == TRUE)
		connman_technology_tether(technology, TRUE);

	g_free(technology);
	technology = NULL;
	tethering = FALSE;

	gtk_widget_hide(GTK_WIDGET(tethering_dbox));

	cui_tray_enable();
}

static void agent_popup_tethering_dbox(void)
{
	const char *value;
	GtkEntry *entry;

	value = connman_technology_get_tethering_identifier(technology);

	set_entry(cui_builder, "tethering_ssid", value, "");

	value = connman_technology_get_tethering_passphrase(technology);
	entry = set_entry(cui_builder, "tethering_passphrase", value, "");
	gtk_entry_set_visibility(entry, FALSE);
	if (gtk_entry_get_text_length(entry) >= 8)
		set_widget_sensitive(cui_builder, "tethering_ok", TRUE);
	else
		set_widget_sensitive(cui_builder, "tethering_ok", FALSE);

	gtk_widget_show(GTK_WIDGET(tethering_dbox));

	cui_tray_disable();
}

static gboolean secret_entry_key_release_cb(GtkWidget *widget,
					GdkEvent  *event, gpointer user_data)
{
	GtkEntry *entry = GTK_ENTRY(widget);
	GtkWidget *button = user_data;
	gboolean enable = FALSE;

	if (gtk_entry_get_text_length(entry) >= 8)
		enable = TRUE;

	gtk_widget_set_sensitive(button, enable);

	return TRUE;
}

void cui_agent_set_wifi_tethering_settings(const char *path, gboolean tether)
{
	const char *ssid, *passphrase;
	g_free(technology);

	technology = g_strdup(path);
	if (technology == NULL)
		return;

	tethering = tether;

	ssid = connman_technology_get_tethering_identifier(path);
	passphrase = connman_technology_get_tethering_passphrase(path);

	if (tethering == TRUE && (ssid == NULL || passphrase == NULL))
		agent_popup_tethering_dbox();
	else if (tethering == TRUE)
		connman_technology_tether(path, TRUE);
	else
		agent_popup_tethering_dbox();
}

gint cui_load_agent_dialogs(void)
{
	GError *error = NULL;
	GtkWidget *button, *entry;

	memset(&service, 0, sizeof(struct cui_selected_service));

	gtk_builder_add_from_file(cui_builder,
				CUI_AGENT_DIALOG_UI_PATH, &error);
	if (error != NULL) {
		printf("Error: %s\n", error->message);
		g_error_free(error);

		return -EINVAL;
	}

	input_dbox = (GtkDialog *) gtk_builder_get_object(cui_builder,
								"input_dbox");
	login_dbox = (GtkDialog *) gtk_builder_get_object(cui_builder,
								"login_dbox");
	error_dbox = (GtkDialog *) gtk_builder_get_object(cui_builder,
								"error_dbox");
	tethering_dbox = (GtkDialog *) gtk_builder_get_object(cui_builder,
							"tethering_dbox");

	/* Passphrase dbox buttons */
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"input_ok");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_ok_input), NULL);

	entry = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"secret_entry");
	g_signal_connect(entry, "enter-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);
	g_signal_connect(entry, "leave-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);
	g_signal_connect(entry, "key-release-event",
			G_CALLBACK(secret_entry_key_release_cb), button);

	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"passphrase_button");
	g_signal_connect(button, "toggled",
				G_CALLBACK(agent_passphrase_changed), NULL);
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"wpspbc_button");
	g_signal_connect(button, "toggled",
				G_CALLBACK(agent_passphrase_changed), NULL);
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"wpspin_button");
	g_signal_connect(button, "toggled",
				G_CALLBACK(agent_passphrase_changed), NULL);

	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"input_cancel");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_cancel), input_dbox);
	g_signal_connect(input_dbox, "response",
				G_CALLBACK(agent_close), NULL);
	g_signal_connect(input_dbox, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	/* Login dbox widgets */
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"login_ok");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_ok_login), NULL);
	entry = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"login_password");
	g_signal_connect(entry, "enter-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);
	g_signal_connect(entry, "leave-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);

	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"login_cancel");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_cancel), login_dbox);
	g_signal_connect(login_dbox, "response",
				G_CALLBACK(agent_close), NULL);
	g_signal_connect(login_dbox, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	/* Tethering dbox widgets */
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"tethering_ok");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_ok_tethering), NULL);
	entry = (GtkWidget *) gtk_builder_get_object(cui_builder,
						"tethering_passphrase");
	g_signal_connect(entry, "enter-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);
	g_signal_connect(entry, "leave-notify-event",
			G_CALLBACK(change_invisible_char_on_entry), entry);
	g_signal_connect(entry, "key-release-event",
			G_CALLBACK(secret_entry_key_release_cb), button);
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"tethering_cancel");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_cancel), tethering_dbox);
	g_signal_connect(tethering_dbox, "response",
				G_CALLBACK(agent_close), NULL);
	g_signal_connect(tethering_dbox, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	/* Error dbox buttons */
	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"error_retry");
	g_signal_connect(button, "clicked", G_CALLBACK(agent_retry), NULL);

	button = (GtkWidget *) gtk_builder_get_object(cui_builder,
							"error_cancel");
	g_signal_connect(button, "clicked",
				G_CALLBACK(agent_cancel), error_dbox);
	g_signal_connect(error_dbox, "response",
				G_CALLBACK(agent_close), NULL);
	g_signal_connect(error_dbox, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	return 0;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>
#include <gtkservice.h>

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
static gboolean gtk_service_button_release_event(GtkWidget *widget,
							GdkEventButton *event);

G_DEFINE_TYPE(GtkService, gtk_service, GTK_TYPE_MENU_ITEM);

static void gtk_service_class_init(GtkServiceClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkMenuItemClass *menu_item_class;

	object_class = G_OBJECT_CLASS(klass);
	widget_class = GTK_WIDGET_CLASS(klass);
	menu_item_class = GTK_MENU_ITEM_CLASS(klass);

	object_class->dispose = gtk_service_dispose;

	widget_class->destroy = gtk_service_destroy;
	widget_class->button_release_event =
			gtk_service_button_release_event;

	menu_item_class->hide_on_activate = FALSE;

	g_type_class_add_private(object_class,
					sizeof(GtkServicePrivate));
}

static void gtk_service_init(GtkService *service)
{
	GtkServicePrivate *priv;

	priv =  G_TYPE_INSTANCE_GET_PRIVATE(service,
						GTK_TYPE_SERVICE,
						GtkServicePrivate);
	service->priv = priv;

	priv->box = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

	priv->name = (GtkLabel *) gtk_label_new(NULL);
	priv->state = (GtkImage *) gtk_image_new();
	priv->security = (GtkImage *) gtk_image_new();
	priv->signal = (GtkImage *) gtk_image_new();

	priv->selected = FALSE;

	//gtk_widget_set_halign((GtkWidget *)priv->box, GTK_ALIGN_START);
	gtk_widget_set_halign((GtkWidget *)priv->name, GTK_ALIGN_START);

	gtk_widget_set_halign((GtkWidget *)priv->state, GTK_ALIGN_END);
	gtk_widget_set_halign((GtkWidget *)priv->security, GTK_ALIGN_END);
	gtk_widget_set_halign((GtkWidget *)priv->signal, GTK_ALIGN_END);

	gtk_box_set_homogeneous(priv->box, FALSE);

	gtk_box_pack_start(priv->box,
			(GtkWidget *)priv->name, TRUE, TRUE, 0);

	gtk_box_pack_start(priv->box,
			(GtkWidget *) priv->state, TRUE, TRUE, 0);
	gtk_box_pack_start(priv->box,
			(GtkWidget *) priv->security, TRUE, TRUE, 0);
	gtk_box_pack_start(priv->box,
			(GtkWidget *) priv->signal, TRUE, TRUE, 0);

	gtk_widget_set_visible((GtkWidget *)priv->box, TRUE);
	gtk_widget_set_visible((GtkWidget *)priv->name, TRUE);

	gtk_widget_set_visible((GtkWidget *)priv->state, FALSE);
	gtk_widget_set_visible((GtkWidget *)priv->security, FALSE);
	gtk_widget_set_visible((GtkWidget *)priv->signal, FALSE);

	gtk_container_add(GTK_CONTAINER(service), (GtkWidget *)priv->box);

	gtk_widget_set_can_focus((GtkWidget *)priv->box, TRUE);
}

static void gtk_service_dispose(GObject *object)
{
	(*G_OBJECT_CLASS(gtk_service_parent_class)->dispose)(object);
}

static void gtk_service_destroy(GtkWidget *widget)
{
	GtkService *service = GTK_SERVICE(widget);
	GtkServicePrivate *priv = service->priv;

	if (priv != NULL && priv->selected == FALSE) {

		connman_service_set_property_changed_callback(service->path,
								NULL, service);
		connman_technology_set_property_error_callback(service->path,
								NULL, service);
	}

	GTK_WIDGET_CLASS(gtk_service_parent_class)->destroy(widget);
}

static gboolean gtk_service_button_release_event(GtkWidget *widget,
							GdkEventButton *event)
{
	GtkService *service = GTK_SERVICE(widget);
	GtkWidget *parent;

	if (event->button != 1 && event->button != 3)
		goto activate;

	if (event->button == 1) {
		if (connman_service_is_connected(service->path) == TRUE)
			connman_service_disconnect(service->path);
		else {
			cui_agent_set_selected_service(service->path,
				connman_service_get_name(service->path));
			connman_service_connect(service->path);
		}
	} else if (event->button == 3) {
		service->priv->selected = TRUE;

		cui_settings_popup(service->path);
	}

activate:
	parent = gtk_widget_get_parent (widget);
	if (parent != NULL && GTK_IS_MENU_SHELL(parent) == TRUE) {
		GtkMenuShell *menu_shell = GTK_MENU_SHELL(parent);
		gtk_menu_shell_activate_item(menu_shell, widget, TRUE);
	}

	return TRUE;
}

static void service_property_changed_cb(const char *path,
					const char *property, void *user_data)
{
	return;
}

static void service_set_state(GtkService *service)
{
	GtkServicePrivate *priv = service->priv;
	const struct connman_ipv4 *ipv4;
	enum connman_state state;
	GdkPixbuf *image = NULL;
	const char *ip = NULL;
	const char *info;

	if (connman_service_is_connected(service->path) == FALSE) {
		gtk_widget_set_tooltip_text((GtkWidget *)priv->name, "");
		return;
	}

	ipv4 = connman_service_get_ipv4(service->path);
	if (ipv4 == NULL) {
		const struct connman_ipv6 *ipv6;

		ipv6 = connman_service_get_ipv6(service->path);
		if (ipv6 != NULL)
			ip = ipv6->address;
	} else
		ip = ipv4->address;

	if (ip == NULL)
		ip = "";

	gtk_widget_set_tooltip_text((GtkWidget *)priv->name, ip);

	state = connman_service_get_state(service->path);
	cui_theme_get_state_icone_and_info(state, &image, &info);

	if (image == NULL)
		return;

	gtk_widget_set_visible((GtkWidget *)priv->state, TRUE);
	gtk_widget_set_tooltip_text((GtkWidget *)priv->state, info);
	gtk_image_set_from_pixbuf(priv->state, image);
}

static void service_set_signal(GtkService *service)
{
	GtkServicePrivate *priv = service->priv;
	GdkPixbuf *image = NULL;
	const char *type, *info;

	type = connman_service_get_type(service->path);

	if (g_strcmp0(type, "wifi") == 0) {
		uint8_t strength;

		strength = connman_service_get_strength(service->path);

		cui_theme_get_signal_icone_and_info(strength, &image, &info);
	} else
		cui_theme_get_type_icone_and_info(type, &image, &info);

	if (image == NULL)
		return;

	gtk_widget_set_visible((GtkWidget *)priv->signal, TRUE);
	gtk_widget_set_tooltip_text((GtkWidget *)priv->signal, info);
	gtk_image_set_from_pixbuf(priv->signal, image);
}

static void service_set_name(GtkService *service)
{
	const char *name;
	char *markup;

	name = connman_service_get_name(service->path);
	if (name == NULL)
		name = "- Hidden -";

	if (connman_service_is_favorite(service->path) == TRUE) {
		if (g_strcmp0(connman_service_get_type(service->path),
							"wifi") == 0) {
			markup = g_markup_printf_escaped(
				"<b>%s</b> <i> (%s) </i>",
				name,
				connman_service_get_security(service->path));
		} else
			markup = g_markup_printf_escaped("<b>%s</b>", name);
	} else {
		if (g_strcmp0(connman_service_get_type(service->path),
							"wifi") == 0) {
			markup = g_markup_printf_escaped(
				"%s  <i> (%s) </i>", name,
				connman_service_get_security(service->path));
		} else
			markup = g_markup_printf_escaped("%s", name);
	}

	gtk_label_set_markup(service->priv->name, markup);

	g_free(markup);
}

GtkService *gtk_service_new(const char *path)
{
	GtkService *service;
	char *path_copy;

	if (path == NULL)
		return NULL;

	path_copy = g_strdup(path);
	if (path_copy == NULL)
		return NULL;

	service = g_object_new(GTK_TYPE_SERVICE, NULL);
	if (service == NULL) {
		g_free(path_copy);
		return NULL;
	}

	service->path = path_copy;

	connman_service_set_property_changed_callback(path_copy,
					service_property_changed_cb,
					service);


	service_set_name(service);
	service_set_state(service);
	service_set_signal(service);

	return service;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <gtktechnology.h>

#include <connman-ui-gtk.h>

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
static gboolean gtk_technology_button_release_event(GtkWidget *widget,
							GdkEventButton *event);

G_DEFINE_TYPE(GtkTechnology, gtk_technology, GTK_TYPE_MENU_ITEM);

static void gtk_technology_class_init(GtkTechnologyClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkMenuItemClass *menu_item_class;

	object_class = G_OBJECT_CLASS(klass);
	widget_class = GTK_WIDGET_CLASS(klass);
	menu_item_class = GTK_MENU_ITEM_CLASS(klass);

	object_class->dispose = gtk_technology_dispose;

	widget_class->destroy = gtk_technology_destroy;
	widget_class->button_release_event =
			gtk_technology_button_release_event;

	menu_item_class->hide_on_activate = FALSE;

	g_type_class_add_private(object_class,
					sizeof(GtkTechnologyPrivate));
}

static void gtk_technology_init(GtkTechnology *technology)
{
	GtkTechnologyPrivate *priv;

	priv =  G_TYPE_INSTANCE_GET_PRIVATE(technology,
						GTK_TYPE_TECHNOLOGY,
						GtkTechnologyPrivate);
	technology->priv = priv;

	priv->box = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	priv->enabler = (GtkSwitch *) gtk_switch_new();
	priv->name = (GtkLabel *) gtk_label_new(NULL);


	gtk_widget_set_margin_start((GtkWidget *)priv->enabler, 0);
	gtk_widget_set_margin_end((GtkWidget *)priv->enabler, 0);
	gtk_widget_set_margin_top((GtkWidget *)priv->enabler, 0);
	gtk_widget_set_margin_bottom((GtkWidget *)priv->enabler, 0);

	gtk_widget_set_margin_start((GtkWidget *)priv->name, 0);
	gtk_widget_set_margin_end((GtkWidget *)priv->name, 0);
	gtk_widget_set_margin_top((GtkWidget *)priv->name, 0);
	gtk_widget_set_margin_bottom((GtkWidget *)priv->name, 0);

	gtk_box_set_spacing(priv->box, 0);
	gtk_box_set_homogeneous(priv->box, TRUE);

	//gtk_widget_set_halign((GtkWidget *)priv->box, GTK_ALIGN_START);
	gtk_widget_set_halign((GtkWidget *)priv->name, GTK_ALIGN_START);
	//gtk_widget_set_halign((GtkWidget *)technology, GTK_ALIGN_START);

	gtk_box_pack_start(priv->box,
			(GtkWidget *)priv->enabler, FALSE, FALSE, 0);
	gtk_box_pack_start(priv->box,
			(GtkWidget *)priv->name, FALSE, FALSE, 0);

	gtk_widget_set_visible((GtkWidget *)priv->box, TRUE);
	gtk_widget_set_visible((GtkWidget *)priv->enabler, TRUE);
	gtk_widget_set_visible((GtkWidget *)priv->name, TRUE);

	gtk_container_add(GTK_CONTAINER(technology), (GtkWidget *)priv->box);

	gtk_widget_set_can_focus((GtkWidget *)priv->box, TRUE);
	gtk_widget_set_can_focus((GtkWidget *)priv->enabler, TRUE);
}

static void gtk_technology_dispose(GObject *object)
{
	(*G_OBJECT_CLASS(gtk_technology_parent_class)->dispose)(object);
}

static void gtk_technology_destroy(GtkWidget *widget)
{
	GtkTechnology *technology = GTK_TECHNOLOGY(widget);
	GtkTechnologyPrivate *priv = technology->priv;

	connman_technology_set_property_changed_callback(technology->path,
							NULL, technology);
	connman_technology_set_property_error_callback(technology->path,
							NULL, technology);

	if (priv->tethering != NULL) {
		gtk_widget_destroy((GtkWidget *)priv->tethering);
		priv->tethering = NULL;
	}

	GTK_WIDGET_CLASS(gtk_technology_parent_class)->destroy(widget);
}

static gboolean gtk_technology_button_release_event(GtkWidget *widget,
							GdkEventButton *event)
{
	GtkTechnology *technology = GTK_TECHNOLOGY(widget);
	GtkTechnologyPrivate *priv = technology->priv;
	gboolean enable;

	if (event->button != 1 && event->button != 3)
		return TRUE;

	if (event->button == 3) {
		GtkWidget *parent;
		const char *type;

		type = connman_technology_get_type(technology->path);
		if (g_strcmp0(type, "wifi") == 0)
			cui_agent_set_wifi_tethering_settings(technology->path,
									FALSE);
		parent = gtk_widget_get_parent (widget);
		if (parent != NULL && GTK_IS_MENU_SHELL(parent) == TRUE) {
			GtkMenuShell *menu_shell = GTK_MENU_SHELL(parent);
			gtk_menu_shell_activate_item(menu_shell, widget, TRUE);
		}

		return TRUE;
	}

	enable = !gtk_switch_get_active(priv->enabler);

	if (connman_technology_enable(technology->path, enable) == 0)
		gtk_widget_set_sensitive((GtkWidget *)priv->enabler, FALSE);

	return TRUE;
}

static void technology_property_error_cb(const char *path,
			const char *property, int error, void *user_data)
{
	if (error != 0)
		printf("Could not set property %s\n", property);
}

static gboolean gtk_technology_tethering_button(GtkWidget *widget,
				GdkEventButton *event, gpointer user_data)
{
	GtkTechnology *technology = user_data;
	GtkTechnologyPrivate *priv = technology->priv;
	gboolean tethering;

	if (event->button != 1)
		return TRUE;

	gtk_widget_set_sensitive((GtkWidget *)priv->tethering, FALSE);

	tethering = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), !tethering);

	connman_technology_tether(technology->path, !tethering);

	return TRUE;
}

static void set_technology_name(GtkTechnology *technology, gboolean enabled)
{
	const char *name = connman_technology_get_name(technology->path);

	if (enabled == TRUE) {
		char *markup;

		markup = g_markup_printf_escaped("<b>%s</b>", name);
		gtk_label_set_markup(technology->priv->name, markup);
		g_free(markup);
	} else
		gtk_label_set_text(technology->priv->name, name);
}

static void technology_property_changed_cb(const char *path,
					const char *property, void *user_data)
{
	GtkTechnology *technology = user_data;
	GtkTechnologyPrivate *priv = technology->priv;

	if (g_strcmp0(property, "Powered") == 0) {
		gboolean enabled;

		enabled = connman_technology_is_enabled(path);

		gtk_switch_set_active(priv->enabler, enabled);
		gtk_widget_set_sensitive((GtkWidget *)priv->enabler, TRUE);

		gtk_widget_set_sensitive((GtkWidget *)priv->tethering,
								enabled);

		set_technology_name(technology, enabled);
	} else if (g_strcmp0(property, "Tethering") == 0) {
		gtk_check_menu_item_set_active(priv->tethering,
					connman_technology_is_tethering(path));
		gtk_widget_set_sensitive((GtkWidget *)priv->tethering,
					connman_technology_is_enabled(path));
	}
}

GtkTechnology *gtk_technology_new(const gchar *path)
{
	GtkTechnology *technology;
	GtkTechnologyPrivate *priv;
	gchar *path_copy, *label;
	gboolean enabled;

	if (path == NULL)
		return NULL;

	path_copy = g_strdup(path);
	if (path_copy == NULL)
		return NULL;

	technology = g_object_new(GTK_TYPE_TECHNOLOGY, NULL);
	if (technology == NULL) {
		g_free(path_copy);
		return NULL;
	}

	priv = technology->priv;
	technology->path = path_copy;

	connman_technology_set_property_changed_callback(path_copy,
					technology_property_changed_cb,
					technology);
	connman_technology_set_property_error_callback(technology->path,
				technology_property_error_cb, technology);

	if (g_strcmp0(connman_technology_get_type(path_copy), "wifi") == 0) {
		gtk_widget_set_tooltip_text(GTK_WIDGET(technology),
				_("Left click to enable/disable\n"
				"Right click to set tethering information"));
	} else
		gtk_widget_set_tooltip_text(GTK_WIDGET(technology),
						_("Left to enable/disable"));

	enabled = connman_technology_is_enabled(path_copy);
	gtk_switch_set_active(priv->enabler, enabled);

	label = g_strdup_printf("via: %s",
				connman_technology_get_name(path_copy));

	priv->tethering = (GtkCheckMenuItem *)
				gtk_check_menu_item_new_with_label(label);
	if (enabled == FALSE)
		gtk_widget_set_sensitive((GtkWidget *)priv->tethering, FALSE);

	set_technology_name(technology, enabled);

	gtk_check_menu_item_set_active(priv->tethering,
				connman_technology_is_tethering(path_copy));

	gtk_widget_set_visible((GtkWidget *)priv->tethering, TRUE);

	g_signal_connect(priv->tethering, "button-release-event",
				G_CALLBACK(gtk_technology_tethering_button),
				technology);

	return technology;
}

GtkMenuItem *gtk_technology_get_tethering_item(GtkTechnology *technology)
{
	GtkTechnologyPrivate *priv = technology->priv;

	return GTK_MENU_ITEM(priv->tethering);
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>
#include <gtkservice.h>

#define CUI_LEFT_MENU_UI_PATH CUI_UI_PATH "/left_menu.ui"

static GtkMenu *cui_left_menu = NULL;
static GtkMenu *cui_more_menu = NULL;
static GtkMenuItem *cui_list_more_item = NULL;
static GHashTable *service_items = NULL;
static GtkMenuItem *cui_scan_spinner = NULL;

static void add_or_update_service(const char *path, int position)
{
	GtkService *s;

	s = gtk_service_new(path);

	if (position > 9)
		gtk_menu_shell_append(GTK_MENU_SHELL(cui_more_menu),
							(GtkWidget *)s);
	else
		gtk_menu_shell_insert(GTK_MENU_SHELL(cui_left_menu),
						(GtkWidget *)s, position);

	gtk_widget_set_visible((GtkWidget *)s, TRUE);
	gtk_widget_show((GtkWidget *)s);

	g_hash_table_insert(service_items, s->path, s);
}

static void remove_service_cb(const char *path)
{
	g_hash_table_remove(service_items, path);

	/* Reposition left menu after updating the list */
	gtk_menu_reposition(cui_left_menu);
}

static void accumulate_menu_size(GtkWidget* widget, gpointer data)
{
	GtkRequisition *menu_size = (GtkRequisition *)data;
	GtkRequisition item_size;
	gtk_widget_get_preferred_size(widget, NULL, &item_size);
	menu_size->width = MAX(item_size.width, menu_size->width);
	menu_size->height += item_size.height;
}

#if 0
static void menu_position_func(GtkMenu *menu, gint *out_x, gint *out_y,
        gboolean *push_in, gpointer user_data)
{
    /* Placement logic adapted for modern GTK */
    GtkWidget *widget = GTK_WIDGET(menu);
    GtkStatusIcon *trayicon = GTK_STATUS_ICON(user_data);
    GtkRequisition requisition;
    gint x, y;
    gint space_left, space_right, space_above, space_below;
    gint needed_width, needed_height;
    GdkDisplay *display;
    GdkMonitor *monitor;
    GdkRectangle area, monitor_area;
    GtkBorder padding, margin;
    GtkStyleContext *context;
    GtkStateFlags state;
    gboolean rtl = (gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL);

    /* Initialize position */
    x = 0;
    y = 0;

    /* Try to get tray icon geometry (fallback for deprecated function) */
    /* Note: GtkStatusIcon is deprecated; consider using a different approach in modern GTK */
    if (gtk_status_icon_get_geometry(trayicon, NULL, &area, NULL))
    {
        x = area.x + area.width / 2;
        y = area.y + area.height / 2;
    }
    else
    {
        /* Fallback: Use pointer position or default screen position */
        GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
        GdkDevice *pointer = gdk_seat_get_pointer(seat);
        gdk_device_get_position(pointer, NULL, &x, &y);
    }

    /* Resize menu */
    requisition.width = 0;
    requisition.height = 0;
    gtk_container_foreach(GTK_CONTAINER(cui_left_menu),
            accumulate_menu_size, &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu),
            requisition.width, requisition.height);

    /* Start actual layout */
    context = gtk_widget_get_style_context(widget);
    state = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_padding(context, state, &padding);
    gtk_style_context_get_margin(context, state, &margin);

    display = gtk_widget_get_display(widget);
    monitor = gdk_display_get_monitor_at_point(display, x, y);
    gdk_monitor_get_workarea(monitor, &monitor_area);

    space_left = x - monitor_area.x;
    space_right = monitor_area.x + monitor_area.width - x - 1;
    space_above = y - monitor_area.y;
    space_below = monitor_area.y + monitor_area.height - y - 1;

    /* Position horizontally */
    needed_width = requisition.width - padding.left;
    if (needed_width <= space_left || needed_width <= space_right)
    {
        if ((rtl && needed_width <= space_left) || (!rtl && needed_width > space_right))
            x = x - margin.left + padding.left - requisition.width + 1;
        else
            x = x + margin.right - padding.right;
    }
    else if (requisition.width <= monitor_area.width)
    {
        if (space_left > space_right)
            x = monitor_area.x;
        else
            x = monitor_area.x + monitor_area.width - requisition.width;
    }
    else
    {
        if (rtl)
            x = monitor_area.x + monitor_area.width - requisition.width;
        else
            x = monitor_area.x;
    }

    /* Position vertically */
    needed_height = requisition.height - padding.top;
    if (needed_height <= space_above || needed_height <= space_below)
    {
        if (needed_height <= space_below)
            y = y + margin.top - padding.top;
        else
            y = y - margin.bottom + padding.bottom - requisition.height + 1;
        y = CLAMP(y, monitor_area.y, monitor_area.y + monitor_area.height - requisition.height);
    }
    else if (needed_height > space_below && needed_height > space_above)
    {
        if (space_below >= space_above)
            y = monitor_area.y + monitor_area.height - requisition.height;
        else
            y = monitor_area.y;
    }
    else
    {
        y = monitor_area.y;
    }

    *out_x = x;
    *out_y = y;
}
#endif

static void get_services_cb(void *user_data)
{
	GSList *services, *list;
	int item_position = 2;

	services = connman_service_get_services();
	if (services == NULL)
		return;

	gtk_widget_hide(GTK_WIDGET(cui_list_more_item));

	for (list = services; list; list = list->next, item_position++)
		add_or_update_service(list->data, item_position);

	if (item_position > 10)
		gtk_widget_show(GTK_WIDGET(cui_list_more_item));

	g_slist_free(services);

	/* メニューサイズを再計算 */
	GtkRequisition requisition = {0, 0};
	gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
	gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);

	/* Reposition left menu after updating the list */
	gtk_menu_reposition(cui_left_menu);
}

static void scanning_cb(void *user_data)
{
	GtkSpinner *spin;

	spin = (GtkSpinner *)gtk_bin_get_child(GTK_BIN(cui_scan_spinner));

	gtk_spinner_stop(spin);
	gtk_widget_hide((GtkWidget *)cui_scan_spinner);
	gtk_widget_hide((GtkWidget *)spin);

	/* メニューサイズを再計算 */
	GtkRequisition requisition = {0, 0};
	gtk_container_foreach(GTK_CONTAINER(cui_left_menu), accumulate_menu_size, &requisition);
	gtk_widget_set_size_request(GTK_WIDGET(cui_left_menu), requisition.width, requisition.height);

	/* Reposition left menu after hidding the spinner */
	gtk_menu_reposition(cui_left_menu);
}

static void delete_service_item(gpointer data)
{
	GtkWidget *item = data;

	gtk_widget_destroy(item);
}

static void cui_popup_left_menu(GtkStatusIcon *trayicon,
						gpointer user_data)
{
	GtkSpinner *spin;

	spin = (GtkSpinner *)gtk_bin_get_child(GTK_BIN(cui_scan_spinner));

	gtk_widget_hide(GTK_WIDGET(cui_list_more_item));
	gtk_widget_show((GtkWidget *)cui_scan_spinner);
	gtk_widget_show((GtkWidget *)spin);

	gtk_spinner_start(spin);

	connman_service_set_removed_callback(remove_service_cb);

	connman_service_refresh_services_list(get_services_cb,
							scanning_cb, user_data);

        //gtk_menu_popup_at_widget(cui_left_menu, user_data, GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL);
	gtk_menu_popup_at_pointer(cui_left_menu, NULL);
	/*gtk_menu_popup(cui_left_menu, NULL, NULL,
			menu_position_func, trayicon, 1, 0);*/
}

static void cui_popdown_left_menu(GtkMenu *menu, gpointer user_data)
{
	connman_service_set_removed_callback(NULL);
	g_hash_table_remove_all(service_items);
	connman_service_free_services_list();
}

gint cui_load_left_menu(GtkBuilder *builder, GtkStatusIcon *trayicon)
{
	GError *error = NULL;

	gtk_builder_add_from_file(builder, CUI_LEFT_MENU_UI_PATH, &error);
	if (error != NULL) {
		printf("Error: %s\n", error->message);
		g_error_free(error);

		return -EINVAL;
	}

	cui_left_menu = (GtkMenu *) gtk_builder_get_object(builder,
							"cui_left_menu");
	cui_more_menu = (GtkMenu *) gtk_builder_get_object(builder,
							"cui_more_menu");
	cui_list_more_item = (GtkMenuItem *) gtk_builder_get_object(builder,
							"cui_list_more_item");
	cui_scan_spinner = (GtkMenuItem *) gtk_builder_get_object(builder,
							"cui_scan_spinner");

	gtk_container_add(GTK_CONTAINER(cui_scan_spinner), gtk_spinner_new());

	g_signal_connect(cui_left_menu, "deactivate",
				G_CALLBACK(cui_popdown_left_menu), NULL);

	service_items = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, delete_service_item);

	cui_tray_hook_left_menu(cui_popup_left_menu);

	return 0;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>
#include <connman-interface.h>

#include <config.h>

GtkBuilder *cui_builder;

static void connman_manager_changed(const char *unused,
				const char *property, void *user_data)
{
	if (g_strcmp0(property, "State") == 0)
		cui_trayicon_update_icon();
}

static void connman_up(void *user_data)
{
	connman_manager_init(connman_manager_changed, NULL);
	connman_service_init();
	connman_technology_init();
	connman_agent_init();
	cui_agent_init_callbacks();

	cui_trayicon_update_icon();

	cui_tray_enable();
	cui_right_menu_enable_all();
}

static void connman_down(void *user_data)
{
	connman_agent_finalize();
	connman_service_finalize();
	connman_technology_finalize();
	connman_manager_finalize();

	cui_trayicon_update_icon();

	cui_tray_left_menu_disable();
	cui_right_menu_enable_only_quit();
}

int main(int argc, char *argv[])
{
	int ret;

	setlocale(LC_ALL, "");

	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	textdomain(GETTEXT_PACKAGE);

	printf("%s\n", GETTEXT_PACKAGE);

	gtk_init(&argc, &argv);

	cui_builder = gtk_builder_new();
	if (cui_builder == NULL)
		return -ENOMEM;

	cui_load_theme();

	if (cui_load_trayicon(cui_builder) != 0)
		return -EINVAL;

	if (cui_load_agent_dialogs() != 0)
		return -EINVAL;

	cui_tray_enable();

	ret = connman_interface_init(connman_up, connman_down, NULL);
	if (ret < 0)
		return ret;

	gtk_main();

	connman_agent_finalize();
	connman_service_finalize();
	connman_technology_finalize();
	connman_manager_finalize();
	connman_interface_finalize();

	return 0;
}

/*
 *
 * Connection Manager UI
 *
 * Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>
#include <gtktechnology.h>

#define CUI_RIGHT_MENU_UI_PATH CUI_UI_PATH "/right_menu.ui"

static GtkMenu *cui_right_menu = NULL;
static GtkWidget *cui_item_mode_off = NULL;
static GtkWidget *cui_item_mode_on = NULL;
static GtkMenu *cui_tethering_menu = NULL;
static GHashTable *tech_items = NULL;
static int item_position = 3;
static gboolean disabled = TRUE;

// cui_builder is declared as extern in connman-ui-gtk.h, so we don't declare it here.
// It will be assigned in cui_load_right_menu.

static void add_technology(const char *path)
{
	GtkTechnology *t;

	t = gtk_technology_new(path);
	gtk_menu_shell_insert(GTK_MENU_SHELL(cui_right_menu),
					(GtkWidget *)t, item_position);
	item_position++;

	gtk_widget_set_visible((GtkWidget *)t, TRUE);
	gtk_widget_show((GtkWidget *)t);

	gtk_menu_shell_append(GTK_MENU_SHELL(cui_tethering_menu),
			(GtkWidget *)gtk_technology_get_tethering_item(t));

	g_hash_table_insert(tech_items, t->path, t);
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

static void cui_item_offlinemode_activate(GtkMenuItem *menuitem,
						gpointer user_data)
{
	gboolean offlinemode;

	offlinemode = !connman_manager_get_offlinemode();

	connman_manager_set_offlinemode(offlinemode);
}

static void cui_item_quit_activate(GtkMenuItem *menuitem,
						gpointer user_data)
{
	gtk_main_quit();
}

static void cui_popup_right_menu(GtkStatusIcon *trayicon,
						guint button,
						guint activate_time,
						gpointer user_data)
{
	GList *tech_list, *list;

	if (disabled == TRUE)
		goto popup;

	if (connman_manager_get_offlinemode()) {
		gtk_widget_show(cui_item_mode_on);
		gtk_widget_hide(cui_item_mode_off);
	} else {
		gtk_widget_show(cui_item_mode_off);
		gtk_widget_hide(cui_item_mode_on);
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

    // Replaced deprecated gtk_menu_popup with gtk_menu_popup_at_pointer
    gtk_menu_popup_at_pointer(cui_right_menu, NULL);
}

static void cui_popdown_right_menu(GtkMenu *menu, gpointer user_data)
{
	connman_technology_set_removed_callback(NULL);
	connman_technology_set_added_callback(NULL);

	g_hash_table_remove_all(tech_items);
}

// Definition of cui_enable_item moved here to resolve implicit declaration error.
static void cui_enable_item(gboolean enable)
{
	// set_widget_hidden is assumed to be defined in connman-ui-gtk.h or a linked utility file.
	set_widget_hidden(cui_builder, "cui_item_offlinemode_on", enable);
	set_widget_hidden(cui_builder, "cui_item_offlinemode_off", enable);
	set_widget_hidden(cui_builder, "cui_sep_technology_up", enable);
	set_widget_hidden(cui_builder, "cui_sep_technology_down", enable);
	set_widget_hidden(cui_builder, "cui_item_tethering", enable);
	set_widget_hidden(cui_builder, "cui_sep_tethering", enable);
	set_widget_hidden(cui_builder, "cui_item_quit", FALSE);

	disabled = enable;
}

void cui_right_menu_enable_only_quit(void)
{
	cui_enable_item(TRUE);
}

void cui_right_menu_enable_all(void)
{
	cui_enable_item(FALSE);
}

gint cui_load_right_menu(GtkBuilder *builder, GtkStatusIcon *trayicon)
{
	GtkImageMenuItem *cui_item_tethering;
	GtkMenuItem *cui_item_quit;
	GdkPixbuf *image = NULL;
	GError *error = NULL;

	gtk_builder_add_from_file(builder, CUI_RIGHT_MENU_UI_PATH, &error);
	if (error != NULL) {
		printf("Error: %s\n", error->message);
		g_error_free(error);

		return -EINVAL;
	}

	// Assign the builder to the global variable declared as extern in connman-ui-gtk.h
	cui_builder = builder;

	cui_right_menu = (GtkMenu *) gtk_builder_get_object(builder,
							"cui_right_menu");
	cui_item_quit = (GtkMenuItem *) gtk_builder_get_object(builder,
							"cui_item_quit");
	cui_item_mode_off = (GtkWidget *) gtk_builder_get_object(builder,
						"cui_item_offlinemode_off");
	cui_item_mode_on = (GtkWidget *) gtk_builder_get_object(builder,
						"cui_item_offlinemode_on");
	cui_tethering_menu = (GtkMenu *) gtk_builder_get_object(builder,
						"cui_tethering_menu");

	g_signal_connect(cui_right_menu, "deactivate",
				G_CALLBACK(cui_popdown_right_menu), NULL);
	g_signal_connect(cui_item_quit, "activate",
				G_CALLBACK(cui_item_quit_activate), NULL);
	g_signal_connect(cui_item_mode_off, "activate",
			G_CALLBACK(cui_item_offlinemode_activate), NULL);
	g_signal_connect(cui_item_mode_on, "activate",
			G_CALLBACK(cui_item_offlinemode_activate), NULL);

	tech_items = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, delete_technology_item);

	cui_item_tethering = (GtkImageMenuItem *) gtk_builder_get_object(
						builder, "cui_item_tethering");

	cui_theme_get_tethering_icone_and_info(&image, NULL);
	if (image != NULL) {
        // Retrieve the GtkImage child from the GtkImageMenuItem (which is a GtkBin)
        GtkWidget *image_widget = gtk_bin_get_child(GTK_BIN(cui_item_tethering));
        if (GTK_IS_IMAGE(image_widget)) {
            // If it's a GtkImage, update its pixbuf directly (non-deprecated)
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), image);
        }
        // Removed the fallback using gtk_image_menu_item_set_image to completely
        // eliminate the deprecation warning. This assumes the UI file is correctly
        // structured to have a GtkImage child within the GtkImageMenuItem.
    }

	cui_tray_hook_right_menu(cui_popup_right_menu);

	return 0;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>

#include <stdlib.h>

#define CUI_SETTINGS_UI_PATH CUI_UI_PATH "/settings.ui"

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

static inline
void toggle_button(const char *name)
{
	GtkToggleButton *button;

	button = (GtkToggleButton *) gtk_builder_get_object(builder, name);
	if (button == NULL)
		return;

	gtk_toggle_button_set_active(button, TRUE);
}

static void favorite_button_toggled(GtkToggleButton *togglebutton,
							gpointer user_data)
{
	if (gtk_toggle_button_get_active(togglebutton) == FALSE)
		connman_service_remove(path);
}

static void autoconnect_button_toggled(GtkToggleButton *togglebutton,
							gpointer user_data)
{
	connman_service_set_autoconnectable(path,
				gtk_toggle_button_get_active(togglebutton));
}

static void update_header(void)
{
	GdkPixbuf *image = NULL;
	const char *type, *info;
	GtkWidget *widget;
	gboolean favorite;

	type = connman_service_get_type(path);
	if (g_strcmp0(type, "wifi") == 0) {
		uint8_t strength;

		strength = connman_service_get_strength(path);

		cui_theme_get_signal_icone_and_info(strength, &image, &info);
	} else
		cui_theme_get_type_icone_and_info(type, &image, &info);

	set_image(builder, "service_type", image, info);
	set_label(builder, "service_name",
			connman_service_get_name(path), "- Hidden -");

	if (connman_service_is_connected(path) == TRUE)
		cui_theme_get_state_icone_and_info(
				connman_service_get_state(path), &image, &info);
	else
		image = NULL;

	set_image(builder, "service_state", image, info);
	set_label(builder, "service_error",
				connman_service_get_error(path), "");

	favorite = connman_service_is_favorite(path);

	set_button_toggle(builder, "service_autoconnect",
				connman_service_is_autoconnect(path));
	widget = set_widget_sensitive(builder,
				"service_autoconnect", favorite);
	if (favorite == TRUE) {
		g_signal_connect(widget, "toggled",
				G_CALLBACK(autoconnect_button_toggled), NULL);
	}

	set_button_toggle(builder, "service_favorite", favorite);
	widget = set_widget_sensitive(builder, "service_favorite", favorite);

	if (favorite == TRUE) {
		g_signal_connect(widget, "toggled",
				G_CALLBACK(favorite_button_toggled), NULL);
	}
}

static void enable_ipv4_config(gboolean enable)
{
	set_widget_sensitive(builder, "ipv4_conf_address", enable);
	set_widget_sensitive(builder, "ipv4_conf_netmask", enable);
	set_widget_sensitive(builder, "ipv4_conf_gateway", enable);
}

static void set_ipv4_method(const char *method)
{
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

static void update_ipv4(void)
{
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

static void enable_ipv6_config(gboolean enable)
{
	set_widget_sensitive(builder, "ipv6_conf_address", enable);
	set_widget_sensitive(builder, "ipv6_conf_prefix_length", enable);
	set_widget_sensitive(builder, "ipv6_conf_gateway", enable);
}

static void set_ipv6_method(const char *method)
{
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

static void set_ipv6_privacy(const char *privacy)
{
	if (g_strcmp0(privacy, "enabled") == 0)
		toggle_button("ipv6_priv_enabled");
	else if (g_strcmp0(privacy, "prefered") == 0)
		toggle_button("ipv6_priv_prefered");
	else
		toggle_button("ipv6_priv_disabled");
}

static void update_ipv6(void)
{
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

static void update_dns(void)
{
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

static void update_timeservers(void)
{
	set_entry(builder, "timerservers", connman_service_get_timeservers(path), "");
	set_entry(builder, "timerservers_conf",
			connman_service_get_timeservers_config(path), "");
}

static void enable_proxy_config(gboolean enable)
{
	set_widget_sensitive(builder, "proxy_conf_url", enable);
	set_widget_sensitive(builder, "proxy_conf_servers", enable);
	set_widget_sensitive(builder, "proxy_conf_excludes", enable);
}

static void set_proxy_method(const char *method)
{
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

static void update_proxy(void)
{
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

static void update_provider(void)
{
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

static void update_ethernet(void)
{
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

static void service_property_changed_cb(const char *path,
					const char *property, void *user_data)
{
	update_header();
}

static void service_property_set_cb(const char *path,
					const char *property, void *user_data)
{
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

	connman_service_set_property_changed_callback(path,
					service_property_changed_cb, NULL);
}

static void service_property_error_cb(const char *path,
			const char *property, int error, void *user_data)
{
	printf("Error setting %s (%d)\n", property, error);

	service_property_set_cb(NULL, NULL, NULL);
}

static void toggled_ipv4_method_cb(GtkToggleButton *togglebutton,
							gpointer user_data)
{
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

static void toggled_ipv6_method_cb(GtkToggleButton *togglebutton,
							gpointer user_data)
{
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

static void toggled_ipv6_privacy_cb(GtkToggleButton *togglebutton,
							gpointer user_data)
{
	const char *name = user_data;

	if (g_strcmp0(name, "ipv6_priv_enabled") == 0)
		ipv6_privacy = "enabled";
	else if (g_strcmp0(name, "ipv6_priv_disabled") == 0)
		ipv6_privacy = "disabled";
	else
		ipv6_privacy = "prefered";

	ipv6_changed = TRUE;
}

static void toggled_proxy_method_cb(GtkToggleButton *togglebutton,
							gpointer user_data)
{
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

static gboolean key_released_entry_cb(GtkWidget *widget,
					GdkEvent *event, gpointer user_data)
{
	gboolean *value = user_data;

	*value = TRUE;

	return TRUE;
}

static void get_ipv4_configuration(struct connman_ipv4 *ipv4)
{
	ipv4->method = (char *) ipv4_method;
	ipv4->address = (char *) get_entry_text(builder, "ipv4_conf_address");
	ipv4->netmask = (char *) get_entry_text(builder, "ipv4_conf_netmask");
	ipv4->gateway = (char *) get_entry_text(builder, "ipv4_conf_gateway");
}

static void get_ipv6_configuration(struct connman_ipv6 *ipv6)
{
	const char *prefix;

	prefix = get_entry_text(builder, "ipv6_conf_prefix_length");

	ipv6->method = (char *) ipv6_method;
	ipv6->address = (char *) get_entry_text(builder, "ipv6_conf_address");
	ipv6->prefix = atoi(prefix);
	ipv6->gateway = (char *) get_entry_text(builder, "ipv6_conf_gateway");
	ipv6->privacy = (char *) ipv6_privacy;
}

static void get_proxy_configuration(struct connman_proxy *proxy)
{
	proxy->method = (char *) proxy_method;
	proxy->url = (char *) get_entry_text(builder, "proxy_conf_url");
	proxy->servers = (char *) get_entry_text(builder, "proxy_conf_servers");
	proxy->excludes = (char *) get_entry_text(builder, "proxy_conf_excludes");
}

static void settings_cancel_callback(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(GTK_WIDGET(service_settings_dbox));
	service_settings_dbox = NULL;

	connman_service_set_property_changed_callback(path, NULL, NULL);
	connman_service_set_property_error_callback(path, NULL, NULL);

	connman_service_deselect();

	g_free(path);
	path = NULL;

	cui_tray_enable();
}

static void settings_ok_callback(GtkButton *button, gpointer user_data)
{
	const char *value;
	int ret = 0;

	connman_service_set_property_changed_callback(path,
					service_property_set_cb, NULL);

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

static void settings_close_callback(GtkDialog *dialog_box,
					gint response_id, gpointer user_data)
{
	if (response_id == GTK_RESPONSE_DELETE_EVENT ||
				response_id == GTK_RESPONSE_CLOSE)
		settings_cancel_callback(NULL, NULL);
}

static void settings_connect_signals(void)
{
	set_signal_callback(builder, "ipv4_dhcp", "toggled",
			G_CALLBACK(toggled_ipv4_method_cb), "ipv4_dhcp");
	set_signal_callback(builder, "ipv4_manual", "toggled",
			G_CALLBACK(toggled_ipv4_method_cb), "ipv4_manual");
	set_signal_callback(builder, "ipv4_off", "toggled",
			G_CALLBACK(toggled_ipv4_method_cb), "ipv4_off");

	set_signal_callback(builder, "ipv6_auto", "toggled",
			G_CALLBACK(toggled_ipv6_method_cb), "ipv6_auto");
	set_signal_callback(builder, "ipv6_manual", "toggled",
			G_CALLBACK(toggled_ipv6_method_cb), "ipv6_manual");
	set_signal_callback(builder, "ipv6_off", "toggled",
			G_CALLBACK(toggled_ipv6_method_cb), "ipv6_off");

	set_signal_callback(builder, "ipv4_conf_address", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv4_changed);
	set_signal_callback(builder, "ipv4_conf_netmask", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv4_changed);
	set_signal_callback(builder, "ipv4_conf_gateway", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv4_changed);

	set_signal_callback(builder, "ipv6_conf_address", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv6_changed);
	set_signal_callback(builder, "ipv6_conf_prefix_length",
			"key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv6_changed);
	set_signal_callback(builder, "ipv6_conf_gateway", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &ipv6_changed);

	set_signal_callback(builder, "ipv6_priv_prefered",
			"toggled", G_CALLBACK(toggled_ipv6_privacy_cb),
			"ipv6_priv_prefered");
	set_signal_callback(builder, "ipv6_priv_enabled",
			"toggled", G_CALLBACK(toggled_ipv6_privacy_cb),
			"ipv6_priv_enabled");
	set_signal_callback(builder, "ipv6_priv_disabled",
			"toggled", G_CALLBACK(toggled_ipv6_privacy_cb),
			"ipv6_priv_disabled");

	set_signal_callback(builder, "nameservers_conf", "key-release-event",
		G_CALLBACK(key_released_entry_cb), &nameservers_changed);
	set_signal_callback(builder, "domains_conf", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &domains_changed);
	set_signal_callback(builder, "timerservers_conf", "key-release-event",
		G_CALLBACK(key_released_entry_cb), &timeservers_changed);

	set_signal_callback(builder, "proxy_auto", "toggled",
			G_CALLBACK(toggled_proxy_method_cb), "proxy_auto");
	set_signal_callback(builder, "proxy_direct", "toggled",
			G_CALLBACK(toggled_proxy_method_cb), "proxy_direct");
	set_signal_callback(builder, "proxy_manual", "toggled",
			G_CALLBACK(toggled_proxy_method_cb), "proxy_manual");

	set_signal_callback(builder, "proxy_conf_url", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &proxy_changed);
	set_signal_callback(builder, "proxy_conf_servers", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &proxy_changed);
	set_signal_callback(builder, "proxy_conf_excludes", "key-release-event",
			G_CALLBACK(key_released_entry_cb), &proxy_changed);

	set_signal_callback(builder, "settings_ok", "clicked",
				G_CALLBACK(settings_ok_callback), NULL);
	set_signal_callback(builder, "settings_close", "clicked",
				G_CALLBACK(settings_cancel_callback), NULL);

	set_signal_callback(builder, "service_settings_dbox", "response",
				G_CALLBACK(settings_close_callback), NULL);
}

gint cui_settings_popup(const char *selected_path)
{
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

	service_settings_dbox = (GtkDialog *) gtk_builder_get_object(builder,
						"service_settings_dbox");

	gtk_widget_show(GTK_WIDGET(service_settings_dbox));

	connman_service_set_property_changed_callback(path,
					service_property_changed_cb, NULL);

	connman_service_set_property_error_callback(path,
					service_property_error_cb, NULL);

	settings_connect_signals();

	/* Force UI update */
	service_property_set_cb(NULL, NULL, NULL);

	cui_tray_disable();

	return 0;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>

static GtkIconTheme *icon_theme = NULL;

void cui_theme_get_tethering_icone_and_info(GdkPixbuf **image,
							const char **info)
{
	if (image != NULL)
		*image = gtk_icon_theme_load_icon(icon_theme,
					"nm-adhoc", 24, 0, NULL);
	if (info != NULL)
		*info = _("Tethering");
}

void cui_theme_get_type_icone_and_info(const char *type,
					GdkPixbuf **image, const char **info)
{
	const char *nfo = NULL;
	GdkPixbuf *img = NULL;

	if (g_strcmp0(type, "ethernet") == 0) {
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-wired-symbolic", 22, 0, NULL);
		nfo = _("Ethernet");
	} else if (g_strcmp0(type, "cellular") == 0) {
		img = gtk_icon_theme_load_icon(icon_theme,
					"gsm-3g-full", 22, 0, NULL);
		nfo = _("Cellular");
	}

	if (image != NULL)
		*image = img;

	if (info != NULL)
		*info = nfo;
}

void cui_theme_get_signal_icone_and_info(uint8_t signal_strength,
					GdkPixbuf **image, const char **info)
{
	const char *nfo;
	GdkPixbuf *img;

	if (signal_strength >= 80) {
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-wireless-signal-excellent-symbolic", 22, 0, NULL);
		nfo = _("Very good signal");
	} else if (signal_strength >= 60 && signal_strength < 80) {
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-wireless-signal-good-symbolic", 22, 0, NULL);
		nfo = _("Good signal");
	} else if (signal_strength >= 40 && signal_strength < 60) {
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-wireless-signal-ok-symbolic", 22, 0, NULL);
		nfo = _("Low signal");
	} else {
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-wireless-signal-weak-symbolic", 22, 0, NULL);
		nfo = _("Very low signal");
	}

	if (image != NULL)
		*image = img;

	if (info != NULL)
		*info = nfo;

}

void cui_theme_get_state_icone_and_info(enum connman_state state,
					GdkPixbuf **image, const char **info)
{
	const char *nfo = NULL;
	GdkPixbuf *img = NULL;


	switch (state) {
	case CONNMAN_STATE_UNKNOWN:
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-offline-symbolic", 24, 0, NULL);
		nfo = _("Connman is not running");
		break;
	case CONNMAN_STATE_READY:
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-idle-symbolic", 24, 0, NULL);
		nfo = _("Connected");
		break;
	case CONNMAN_STATE_ONLINE:
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-transmit-receive-symbolic", 24, 0, NULL);
		nfo = _("Online");
		break;
	default:
		img = gtk_icon_theme_load_icon(icon_theme,
					"network-offline-symbolic", 24, 0, NULL);
		nfo = _("Disconnected");

		break;
	}

	if (image != NULL)
		*image = img;

	if (info != NULL)
		*info = nfo;
}

void cui_load_theme(void)
{
	icon_theme = gtk_icon_theme_get_default ();
	gtk_icon_theme_append_search_path(icon_theme, CUI_ICON_PATH);
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>

#define CUI_TRAYICON_UI_PATH CUI_UI_PATH "/tray.ui"

static GtkStatusIcon *cui_trayicon = NULL;
static void (*popup_left_menu_f)(GtkStatusIcon *, gpointer);
static void (*popup_rigt_menu_f)(GtkStatusIcon *, guint, guint, gpointer);
static int left_menu_handler_id = 0;
static int right_menu_handler_id = 0;

void cui_trayicon_update_icon(void)
{
	enum connman_state state;
	GdkPixbuf *image = NULL;
	const char *info = NULL;

	state = connman_manager_get_state();

	cui_theme_get_state_icone_and_info(state, &image, &info);

	gtk_status_icon_set_from_pixbuf(cui_trayicon, image);

	gtk_status_icon_set_tooltip_text(cui_trayicon, info);
	gtk_status_icon_set_visible(cui_trayicon, TRUE);
}

void cui_tray_hook_left_menu(gpointer callback)
{
	popup_left_menu_f = callback;
}

void cui_tray_hook_right_menu(gpointer callback)
{
	popup_rigt_menu_f = callback;
}

void cui_tray_left_menu_disable(void)
{
	if (left_menu_handler_id != 0) {
		g_signal_handler_disconnect(cui_trayicon, left_menu_handler_id);
		left_menu_handler_id = 0;
	}
}

void cui_tray_enable(void)
{
	if (left_menu_handler_id == 0)
		left_menu_handler_id = g_signal_connect(cui_trayicon,
			"activate", G_CALLBACK(popup_left_menu_f), NULL);
	if (right_menu_handler_id == 0)
		right_menu_handler_id = g_signal_connect(cui_trayicon,
			"popup-menu", G_CALLBACK(popup_rigt_menu_f), NULL);
}

void cui_tray_disable(void)
{
	if (left_menu_handler_id != 0)
		g_signal_handler_disconnect(cui_trayicon,
					left_menu_handler_id);
	if (right_menu_handler_id != 0)
		g_signal_handler_disconnect(cui_trayicon,
					right_menu_handler_id);

	left_menu_handler_id = 0;
	right_menu_handler_id = 0;
}

gint cui_load_trayicon(GtkBuilder *builder)
{
	GError *error = NULL;

	gtk_builder_add_from_file(builder, CUI_TRAYICON_UI_PATH, &error);
	if (error != NULL) {
		printf("Error: %s\n", error->message);
		g_error_free(error);

		return -EINVAL;
	}

	cui_trayicon = (GtkStatusIcon *) gtk_builder_get_object(builder,
							"cui_trayicon");

	cui_trayicon_update_icon();

	cui_load_left_menu(builder, cui_trayicon);
	cui_load_right_menu(builder, cui_trayicon);

	return 0;
}
/*
 *
 *  Connection Manager UI
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <connman-ui-gtk.h>

GtkLabel *set_label(GtkBuilder *builder, const char *name,
				const char *value, const char *default_value)
{
	GtkLabel *label;

	label = (GtkLabel *) gtk_builder_get_object(builder, name);
	if (label == NULL)
		return NULL;

	if (value == NULL)
		value = default_value;

	gtk_label_set_text(label, value);

	return label;
}

GtkEntry *set_entry(GtkBuilder *builder, const char *name,
				const char *value, const char *default_value)
{
	GtkEntry *entry;

	entry = (GtkEntry *) gtk_builder_get_object(builder, name);
	if (entry == NULL)
		return NULL;

	if (value == NULL)
		value = default_value;

	gtk_entry_set_text(entry, value);

	return entry;
}

GtkWidget *set_widget_sensitive(GtkBuilder *builder,
					const char *name, gboolean value)
{
	GtkWidget *widget;

	widget = (GtkWidget *) gtk_builder_get_object(builder, name);
	if (widget == NULL)
		return NULL;

	gtk_widget_set_sensitive(widget, value);

	return widget;
}

GtkWidget *set_widget_hidden(GtkBuilder *builder,
					const char *name, gboolean value)
{
	GtkWidget *widget;

	widget = (GtkWidget *) gtk_builder_get_object(builder, name);
	if (widget == NULL)
		return NULL;

	gtk_widget_set_visible(widget, value);

	if (value == TRUE)
		gtk_widget_hide(widget);
	else
		gtk_widget_show(widget);

	return widget;
}

GtkWidget *set_button_toggle(GtkBuilder *builder,
					const char *name, gboolean active)
{
	GtkWidget *widget;

	widget = (GtkWidget *) gtk_builder_get_object(builder, name);
	if (widget == NULL)
		return NULL;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), active);

	return widget;
}

GtkImage *set_image(GtkBuilder *builder,
			const char *name, GdkPixbuf *image, const char *info)
{
	GtkImage *img_widget;

	img_widget = (GtkImage *) gtk_builder_get_object(builder, name);
	if (img_widget == NULL)
		return NULL;

	if (image != NULL) {
		gtk_widget_set_visible(GTK_WIDGET(img_widget), TRUE);
		gtk_widget_set_tooltip_text(GTK_WIDGET(img_widget), info);

		gtk_image_set_from_pixbuf(img_widget, image);
	} else
		gtk_widget_set_visible(GTK_WIDGET(img_widget), FALSE);

	return img_widget;
}

void set_signal_callback(GtkBuilder *builder,
				const char *name, const char *signal_name,
				GCallback handler, gpointer user_data)
{
	GtkWidget *widget;

	widget = (GtkWidget *) gtk_builder_get_object(builder, name);
	if (widget == NULL)
		return;

	g_signal_connect(widget, signal_name, handler, user_data);
}

const char *get_entry_text(GtkBuilder *builder, const char *name)
{
	GtkEntry *entry;

	entry = (GtkEntry *) gtk_builder_get_object(builder, name);
	if (entry == NULL)
		return NULL;

	return gtk_entry_get_text(entry);
}
