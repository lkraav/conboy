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

#include "settings_window.h"



typedef struct
{
	GtkWidget *window;
	GtkWidget *url_entry;
	GtkWidget *auth_button;
} SettingsWidget;




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

static void
on_sync_auth_but_clicked(GtkButton *button, SettingsWidget *widget)
{

#ifdef HILDON_HAS_APP_MENU
	const gchar *url = hildon_entry_get_text(HILDON_ENTRY(widget->url_entry));
#else
	const gchar *url = gtk_entry_get_text(GTK_ENTRY(widget->url_entry));
#endif

	GtkWindow *parent = GTK_WINDOW(widget->window);

	if (url == NULL) {
		return;
	}

	gchar *old_url = settings_load_sync_base_url();

	if (old_url != NULL && strcmp(old_url, "") != 0 && strcmp(url, old_url) != 0) {
		GtkWidget *dialog = ui_helper_create_yes_no_dialog(parent, "Really reset the sync settings?");
		int ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if (ret == GTK_RESPONSE_OK) {
			/* Reset settings */
			settings_save_last_sync_revision(0);
			settings_save_last_sync_time(0);
			settings_save_sync_base_url("");
		} else {
			return;
		}
	}

	/* DO auth stuff */

	gchar *link = conboy_get_auth_link(url);

	if (link == NULL) {
		ui_helper_show_confirmation_dialog(parent, "Could not connect to host.");
		return;
	}

	/* If successfull, save url */
	settings_save_sync_base_url(url);

	/* Open link in browser */
	AppData *app_data = app_data_get();
	osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_browser",
			OSSO_BROWSER_OPEN_NEW_WINDOW_REQ, NULL,
			DBUS_TYPE_STRING, link, DBUS_TYPE_INVALID);

	g_printerr("Opening browser with URL: >%s<\n", link);

	ui_helper_show_confirmation_dialog(parent, "Click OK after authenticating on the website.");

	if (conboy_get_access_token()) {
		/* Disable Authenticate button */
		gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
		/* Popup dialog */
		ui_helper_show_confirmation_dialog(parent, "You're authenticated. Everything is good :)");

	} else {
		ui_helper_show_confirmation_dialog(parent, "Something went wrong. Not good :(");
		settings_save_sync_base_url("");
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
	hildon_gtk_window_set_portrait_flags(GTK_WINDOW(dialog), 0);
	orientation_disable_accelerators();

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

#ifdef HILDON_HAS_APP_MENU
	orientation_enable_accelerators();
#endif
}
