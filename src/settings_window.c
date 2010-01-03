/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * Conboy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Conboy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Conboy. If not, see <http://www.gnu.org/licenses/>.
 */
#include "localisation.h"

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-color-button.h>
#include <tablet-browser-interface.h>
#include <dbus/dbus-protocol.h>
#include <hildon/hildon-note.h>
#include <string.h>

#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-gtk.h>
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-pannable-area.h>
#include <hildon/hildon-entry.h>
#include <hildon/hildon-button.h>
#endif


#include "settings.h"
#include "app_data.h"
#include "conboy_plugin_manager.h"
#include "conboy_check_button.h"
#include "ui_helper.h"
#include "json.h"

#include "settings_window.h"



typedef struct
{
	GtkWidget *window;
	GtkWidget *url_entry;
	GtkWidget *auth_button;
} SettingsWidget;


static void
on_portrait_but_toggled(GtkWidget *button, gpointer user_data)
{
	gboolean active;
	#ifdef HILDON_HAS_APP_MENU
	active = hildon_check_button_get_active(HILDON_CHECK_BUTTON(button));
	#else
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
	#endif
	settings_save_use_auto_portrait_mode(active);
}

static void
on_scroll_but_toggled(GtkToggleButton *button, gpointer user_data)
{
	if (gtk_toggle_button_get_active(button)) {
		settings_save_scrollbar_size(SETTINGS_SCROLLBAR_SIZE_SMALL);
	} else {
		settings_save_scrollbar_size(SETTINGS_SCROLLBAR_SIZE_BIG);
	}
}

static void
on_view_but_toggled(GtkToggleButton *button, gpointer user_data)
{
	if (gtk_toggle_button_get_active(button)) {
		settings_save_startup_window(SETTINGS_STARTUP_WINDOW_NOTE);
	} else {
		settings_save_startup_window(SETTINGS_STARTUP_WINDOW_SEARCH);
	}
}

static void
on_use_colors_but_toggled(GtkButton *button, GtkWidget *color_box)
{
#ifdef HILDON_HAS_APP_MENU
	gboolean active = hildon_check_button_get_active(HILDON_CHECK_BUTTON(button));
#else
	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
#endif

	/* Change activate/deactivate the UI */
	gtk_widget_set_sensitive(color_box, active);
	settings_save_use_custom_colors(active);
}

static void
on_color_but_changed(HildonColorButton *button, SettingsColorType *type)
{
	GdkColor color;
	hildon_color_button_get_color(button, &color);
	settings_save_color(&color, GPOINTER_TO_INT(type));
}

struct DialogData {
	GtkDialog *dialog;
	gchar *verifier;
};

static gint
url_callback_handler(const gchar *interface, const gchar *method, GArray *arguments, gpointer user_data, osso_rpc_t *retval)
{
	g_printerr("Method: %s\n", method);
	struct DialogData *data = (struct DialogData*) user_data;

	GtkWidget *dialog = GTK_WIDGET(data->dialog);
	gtk_window_present(GTK_WINDOW(dialog));

	if (g_strcasecmp(method, "authenticated") == 0) {

		/*
		 * The parameter looks like this:
		 * conboy://authenticate?oauth_token=kBFjXzLsKqzmxx9PGBX0&oauth_verifier=1ccaf32e-ec6e-4598-a77f-020af60f24b5&return=https://one.ubuntu.com
		 */

		g_printerr("___ CORRECTLY AUTHENTICATED ____\n");

		if (arguments != NULL && arguments->len >= 1) {

			osso_rpc_t value = g_array_index(arguments, osso_rpc_t, 0);

			if (value.type == DBUS_TYPE_STRING) {
				gchar *url = value.value.s;
				g_printerr("URL: %s\n", url);

				/* Find out verifier */
				gchar **parts = g_strsplit_set(url, "?&", 4);

				gchar *verifier = NULL;
				int i = 0;
				while (parts[i] != NULL) {

					if (strncmp(parts[i], "oauth_verifier=", 15) == 0) {
						verifier = g_strdup(&(parts[i][15])); /* Copy starting from character 15 */
						break;
					}
					i++;
				}

				g_strfreev(parts);

				if (verifier != NULL) {
					g_printerr("OAuth Verifier: %s\n", verifier);
					data->verifier = verifier;
					/* Close the modal dialog and signal success */
					g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_OK);
					return OSSO_OK;
				}
			}
		}
	}

	/* Close the modal dialog and signal failure */
	g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_REJECT);
	return OSSO_OK;
}

static void
clear_sync_settings()
{
	settings_save_last_sync_revision(0);
	settings_save_last_sync_time(0);
	settings_save_sync_base_url("");
	settings_save_oauth_access_token("");
	settings_save_oauth_access_secret("");
}

static void
on_sync_auth_but_clicked(GtkButton *button, SettingsWidget *widget)
{

#ifdef HILDON_HAS_APP_MENU
	const gchar *url = hildon_entry_get_text(HILDON_ENTRY(widget->url_entry));
#else
	const gchar *url = gtk_entry_get_text(GTK_ENTRY(widget->url_entry));
#endif

	GtkWindow *parent = GTK_WINDOW(widget->window);

	if (url == NULL || strcmp(url, "") == 0) {
		return;
	}

	gchar *old_url = settings_load_sync_base_url();

	if (old_url != NULL && strcmp(old_url, "") != 0) {

		gchar *msg;

		if (strcmp(url, old_url) == 0) {
			msg = g_strconcat("You are already authenticated with\n'", old_url, "'. Are you sure you want to reset the synchonization settings?", NULL);
		} else {
			msg = g_strconcat("You are currently authenticated with\n'", old_url, "'. Are you sure you want to reset the synchonization settings?", NULL);
		}

		GtkWidget *dialog = ui_helper_create_yes_no_dialog(parent, msg);
		int ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_free(msg);
		if (ret == GTK_RESPONSE_YES) {
			clear_sync_settings();
		} else {
			gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
			#ifdef HILDON_HAS_APP_MENU
			hildon_entry_set_text(HILDON_ENTRY(widget->url_entry), old_url);
			#else
			gtk_entry_set_text(GTK_ENTRY(widget->url_entry), old_url);
			#endif
			return;
		}
	}

	/* DO auth stuff */

	/* Get URLs */
	gchar *request = g_strconcat(url, "/api/1.0", NULL);

	gchar *reply = http_get(request);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: %s\n", request, NULL);
		g_printerr(msg);
		g_free(msg);
		g_free(request);
		return;
	}
	g_free(request);

	JsonApi *api = json_get_api(reply);

	g_printerr("###################################\n");
	g_printerr("Request token url: %s\n", api->request_token_url);
	g_printerr("Authenticate url : %s\n", api->authorize_url);
	g_printerr("Access token url : %s\n", api->access_token_url);
	g_printerr("###################################\n");


	/* Get auth link url */
	gchar *link = conboy_get_auth_link(api->request_token_url, api->authorize_url);

	if (link == NULL) {
		ui_helper_show_confirmation_dialog(parent, "Could not connect to host.", FALSE);
		return;
	}

	/* If successfull, save url */
	/*settings_save_sync_base_url(url);*/

	/* Open link in browser */
	AppData *app_data = app_data_get();
	osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_browser",
			OSSO_BROWSER_OPEN_NEW_WINDOW_REQ, NULL,
			DBUS_TYPE_STRING, link, DBUS_TYPE_INVALID);

	g_printerr("Opening browser with URL: >%s<\n", link);


	GtkWidget *dialog = ui_helper_create_cancel_dialog(parent, "Please grant access on the website of your service provider which was just opened.\nAfter that you will be automatically redirected to back Conboy.");


	struct DialogData data;
	data.dialog = GTK_DIALOG(dialog);

	/* Register DBus listener */
	if (osso_rpc_set_cb_f(app_data->osso_ctx, "de.zwong.conboy", "de/zwong/conboy", "de.zwong.conboy", url_callback_handler, &data) != OSSO_OK) {
		g_printerr("ERROR: Failed connect DBus url callback\n");
	}

	/* Open dialog and wait for result */
	int result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	/* Unregister DBus listener */
	if (osso_rpc_unset_cb_f(app_data->osso_ctx, "de.zwong.conboy", "de/zwong/conboy", "de.zwong.conboy", url_callback_handler, &data) != OSSO_OK) {
			g_printerr("ERROR: Failed disconnect DBus url callback\n");
	}

	if (result == GTK_RESPONSE_OK) {

		if (conboy_get_access_token(api->access_token_url, data.verifier)) {

			gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
			ui_helper_show_confirmation_dialog(parent, "<b>You are successfully authenticated</b>\nYou can now use the synchronization from the main menu.", FALSE);
			settings_save_sync_base_url(url);
		} else {
			ui_helper_show_confirmation_dialog(parent, "Conboy could not get an access token from your service provider. Please try again.", FALSE);
			settings_save_sync_base_url("");
		}
	} else if (result == 666) {
		ui_helper_show_confirmation_dialog(parent, "You have manually canceled the authentication process.", FALSE);
	} else if (result == GTK_RESPONSE_REJECT) {
		ui_helper_show_confirmation_dialog(parent, "There were problems with the data received from your service provider. Please try again.", FALSE);
		settings_save_sync_base_url("");
	} else {
		ui_helper_show_confirmation_dialog(parent, "An unknown error occured, please try again.", FALSE);
	}

}

static void
on_sync_entry_changed(GtkEntry *entry, GtkWidget *button)
{
	gtk_widget_set_sensitive(button, TRUE);
}

static
GtkWidget *settings_widget_create(GtkWindow *parent)
{
	GtkWidget *pannable;
	GtkWidget *config_vbox;
	GtkWidget *hbox;
#ifndef HILDON_HAS_APP_MENU
	GtkWidget *scroll_vbox, *scroll_label, *scroll_but1, *scroll_but2;
#endif
	GtkWidget *portrait_vbox, *portrait_but;
	GtkWidget *color_vbox, *color_but;
	GtkWidget *text_color_hbox, *text_color_but, *text_color_label;
	GtkWidget *back_color_hbox, *back_color_but, *back_color_label;
	GtkWidget *link_color_hbox, *link_color_but, *link_color_label;
	GdkColor color;

	SettingsWidget *widget = g_new0(SettingsWidget, 1);
	widget->window = GTK_WIDGET(parent);

	/* Scrolling / Panning */
#ifdef HILDON_HAS_APP_MENU
	pannable = hildon_pannable_area_new();
#else
	pannable = gtk_scrolled_window_new(NULL, NULL);
#endif
	g_object_set(pannable, "hscrollbar-policy", GTK_POLICY_NEVER, NULL);
	gtk_widget_show(pannable);

	/* Config vbox */
	config_vbox = gtk_vbox_new(FALSE, 20);
	gtk_widget_show(config_vbox);
#ifdef HILDON_HAS_APP_MENU
	hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(pannable), config_vbox);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(pannable), config_vbox);
#endif

	/* Container for "scrollbar size" and for "on startup" */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), hbox, FALSE, FALSE, 0);

#ifndef HILDON_HAS_APP_MENU
	/* Scrollbar vbox */
	scroll_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(scroll_vbox);
	gtk_box_pack_start(GTK_BOX(hbox), scroll_vbox, TRUE, TRUE, 0);

	scroll_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(scroll_label), _("Scrollbar size"));
	gtk_misc_set_alignment(GTK_MISC(scroll_label), 0, 0.5);
	gtk_widget_show(scroll_label);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_label);

	scroll_but1 = gtk_radio_button_new_with_label(NULL, _("Thin"));
	gtk_widget_show(scroll_but1);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but1);

	scroll_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(scroll_but1), _("Thick"));
	gtk_widget_show(scroll_but2);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but2);
#endif

	/* Use auto portrait mode vbox */
	portrait_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(portrait_vbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), portrait_vbox, FALSE, FALSE, 0);

#ifdef HILDON_HAS_APP_MENU
	portrait_but = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(portrait_but), _("Use automatic portrait mode"));
#else
	portrait_but = gtk_check_button_new_with_label(_("Use automatic portrait mode"));
#endif
	gtk_widget_show(portrait_but);
	gtk_box_pack_start(GTK_BOX(portrait_vbox), portrait_but, TRUE, TRUE, 0);

	/* Select Colors vbox */
	color_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(color_vbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), color_vbox, FALSE, FALSE, 0);

#ifdef HILDON_HAS_APP_MENU
	color_but = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(color_but), _("Use custom colors"));
#else
	color_but = gtk_check_button_new_with_label(_("Use custom colors"));
#endif
	gtk_widget_show(color_but);
	gtk_box_pack_start(GTK_BOX(color_vbox), color_but, TRUE, TRUE, 0);

	/* Text Color */
	text_color_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(text_color_hbox);
	gtk_box_pack_start(GTK_BOX(color_vbox), text_color_hbox, TRUE, TRUE, 0);

	text_color_but = hildon_color_button_new();
	gtk_widget_show(text_color_but);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), text_color_but, FALSE, FALSE, 0);

	text_color_label = gtk_label_new(_("Text"));
	gtk_misc_set_alignment(GTK_MISC(text_color_label), 0, 0.5);
	gtk_widget_show(text_color_label);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), text_color_label, TRUE, TRUE, 10);

	/* Background Color */
	back_color_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(back_color_hbox);
	gtk_box_pack_start(GTK_BOX(color_vbox), back_color_hbox, TRUE, TRUE, 0);

	back_color_but = hildon_color_button_new();
	gtk_widget_show(back_color_but);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), back_color_but, FALSE, FALSE, 0);

	back_color_label = gtk_label_new(_("Background"));
	gtk_misc_set_alignment(GTK_MISC(back_color_label), 0, 0.5);
	gtk_widget_show(back_color_label);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), back_color_label, TRUE, TRUE, 10);

	/* Link Color */
	link_color_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(link_color_hbox);
	gtk_box_pack_start(GTK_BOX(color_vbox), link_color_hbox, TRUE, TRUE, 0);

	link_color_but = hildon_color_button_new();
	gtk_widget_show(link_color_but);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), link_color_but, FALSE, FALSE, 0);

	link_color_label = gtk_label_new(_("Links"));
	gtk_misc_set_alignment(GTK_MISC(link_color_label), 0, 0.5);
	gtk_widget_show(link_color_label);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), link_color_label, TRUE, TRUE, 10);

	#ifdef HILDON_HAS_APP_MENU
	hildon_gtk_widget_set_theme_size(text_color_but, HILDON_SIZE_FINGER_HEIGHT);
	hildon_gtk_widget_set_theme_size(back_color_but, HILDON_SIZE_FINGER_HEIGHT);
	hildon_gtk_widget_set_theme_size(link_color_but, HILDON_SIZE_FINGER_HEIGHT);

	/* Adding the label directly to the button like HildonCheckButton is not possible,
	 * so just changing size.
	 */
	gtk_widget_set_size_request(GTK_WIDGET(text_color_but), 70, 70);
	gtk_widget_set_size_request(GTK_WIDGET(back_color_but), 70, 70);
	gtk_widget_set_size_request(GTK_WIDGET(link_color_but), 70, 70);
	#endif

	/* Adding sync section */
	GtkWidget *sync_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(sync_hbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), sync_hbox, TRUE, TRUE, 0);

#ifdef HILDON_HAS_APP_MENU
	GtkWidget *sync_entry = hildon_entry_new(HILDON_SIZE_FINGER_HEIGHT);
	hildon_entry_set_placeholder(HILDON_ENTRY(sync_entry), "http://example.com:8080");
#else
	GtkWidget *sync_entry = gtk_entry_new();
#endif
	widget->url_entry = sync_entry;
	gtk_widget_show(sync_entry);
	gtk_box_pack_start(GTK_BOX(sync_hbox), sync_entry, TRUE, TRUE, 0);

#ifdef HILDON_HAS_APP_MENU
	GtkWidget *sync_auth_but = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
#else
	GtkWidget *sync_auth_but = gtk_button_new();
#endif
	widget->auth_button = sync_auth_but;
	gtk_button_set_label(GTK_BUTTON(sync_auth_but), "Authenticate");
	gtk_widget_show(sync_auth_but);
	gtk_box_pack_start(GTK_BOX(sync_hbox), sync_auth_but, FALSE, FALSE, 0);

	/* Add the plugin manager widget */
	GtkWidget *plugin_manager = conboy_plugin_manager_new();
	gtk_widget_show(plugin_manager);
	gtk_box_pack_start(GTK_BOX(config_vbox), plugin_manager, FALSE, FALSE, 0);


	/*
	 * Set initial values from gconf
	 */

#ifndef HILDON_HAS_APP_MENU
	/* Scrollbars */
	if (settings_load_scrollbar_size() == SETTINGS_SCROLLBAR_SIZE_SMALL) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scroll_but1), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scroll_but2), TRUE);
	}
#endif

	/* On Startup */
#ifdef XXX
	if (settings_load_startup_window() == SETTINGS_STARTUP_WINDOW_NOTE) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(view_but1), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(view_but2), TRUE);
	}
#endif

	/* Use automatic portrait mode */
#ifdef HILDON_HAS_APP_MENU
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(portrait_but), settings_load_use_auto_portrait_mode());
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(portrait_but), settings_load_use_auto_portrait_mode());
#endif

	/* Use custom colors */
	gboolean custom_colors = settings_load_use_costum_colors();
	gtk_widget_set_sensitive(text_color_hbox, custom_colors);
	#ifdef HILDON_HAS_APP_MENU
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(color_but), custom_colors);
	#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(color_but), custom_colors);
	#endif

	/* Background color */
	settings_load_color(&color, SETTINGS_COLOR_TYPE_BACKGROUND);
	hildon_color_button_set_color(HILDON_COLOR_BUTTON(back_color_but), &color);

	/* Text color */
	settings_load_color(&color, SETTINGS_COLOR_TYPE_TEXT);
	hildon_color_button_set_color(HILDON_COLOR_BUTTON(text_color_but), &color);

	/* Link color */
	settings_load_color(&color, SETTINGS_COLOR_TYPE_LINKS);
	hildon_color_button_set_color(HILDON_COLOR_BUTTON(link_color_but), &color);

	/* Sync URL */
	if (settings_load_sync_base_url() != NULL) {
#ifdef HILDON_HAS_APP_MENU
		hildon_entry_set_text(HILDON_ENTRY(sync_entry), settings_load_sync_base_url());
#else
		gtk_entry_set_text(GTK_ENTRY(sync_entry), settings_load_sync_base_url());
#endif
		gtk_widget_set_sensitive(GTK_WIDGET(sync_auth_but), FALSE);
	}


	/* Connect signals */
#ifndef HILDON_HAS_APP_MENU
	g_signal_connect(scroll_but1, "toggled", G_CALLBACK(on_scroll_but_toggled), NULL);
#endif
	g_signal_connect(portrait_but, "toggled", G_CALLBACK(on_portrait_but_toggled), NULL);
	g_signal_connect(color_but, "toggled", G_CALLBACK(on_use_colors_but_toggled), text_color_hbox);
	g_signal_connect(text_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_TEXT));
	g_signal_connect(link_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_LINKS));
	g_signal_connect(back_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_BACKGROUND));
	g_signal_connect(sync_auth_but, "clicked", G_CALLBACK(on_sync_auth_but_clicked), widget);
	g_signal_connect(sync_entry, "changed", G_CALLBACK(on_sync_entry_changed), sync_auth_but);

	/* Set to something big to make sure the maximum dialog area is used */
	gtk_widget_set_size_request(pannable, -1, 800);
	return pannable;
}


void settings_window_open(GtkWindow *parent)
{
#ifdef HILDON_HAS_APP_MENU
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Settings"),
			parent,
			GTK_DIALOG_MODAL,
			NULL);

	ui_helper_remove_portrait_support(GTK_WINDOW(dialog));

#else
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Settings"),
			parent,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_OK,
			NULL);
	gtk_widget_set_size_request(dialog, 800, 480);
#endif

	GtkWidget *content_area = GTK_DIALOG(dialog)->vbox;
	GtkWidget *content_widget = settings_widget_create(GTK_WINDOW(dialog));

	/* Add the widget to the dialog */
	gtk_box_pack_start(GTK_BOX(content_area), content_widget, TRUE, TRUE, 10);

	/* When a button (ok/cancel/etc.) is clicked or the dialog is closed - destroy it */
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));
}
