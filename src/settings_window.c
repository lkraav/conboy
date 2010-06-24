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
#include <glib/gstdio.h>
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
#include "ui_helper.h"
#include "json.h"
#include "conboy_web_sync.h"

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

static void
clear_sync_settings()
{
	gchar *filename;

	settings_save_last_sync_revision(-1);
	settings_save_last_sync_time(0);
	settings_save_sync_base_url("");
	settings_save_oauth_access_token("");
	settings_save_oauth_access_secret("");

	filename = g_strconcat(g_get_home_dir(), "/.conboy/synced_notes.txt", NULL);
	g_unlink(filename);
	g_free(filename);

	filename = g_strconcat(g_get_home_dir(), "/.conboy/deleted_notes.txt", NULL);
	g_unlink(filename);
	g_free(filename);
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

	/* Compare old and new sync URL */
	gchar *old_url = settings_load_sync_base_url();

	if (old_url != NULL && strcmp(old_url, "") != 0) {

		gchar *msg = g_strconcat("You are currently authenticated with\n'", old_url, "'. Are you sure you want to reset the synchonization settings?", NULL);

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

	/* Authenticate with web service */
	if (web_sync_authenticate(url, parent)) {
		gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
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

	/* Frame for general settings */
	GtkWidget *general_frame = gtk_frame_new("General");
	gtk_frame_set_label_align(GTK_FRAME(general_frame), 0, -1);
	gtk_widget_show(general_frame);
	gtk_box_pack_start(GTK_BOX(config_vbox), general_frame, TRUE, TRUE, 0);

	/* VBox for general settings */
	GtkWidget *general_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(general_vbox);
	gtk_container_add(GTK_CONTAINER(general_frame), general_vbox);

	/* Container for "scrollbar size" and for "on startup" */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(general_vbox), hbox, FALSE, FALSE, 0);

#ifndef HILDON_HAS_APP_MENU
	/* Scrollbar vbox */
	scroll_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(scroll_vbox);
	gtk_box_pack_start(GTK_BOX(hbox), scroll_vbox, TRUE, TRUE, 0);

	scroll_label = gtk_label_new("");
	/* Translators: Width of the scrollbar. */
	gtk_label_set_markup(GTK_LABEL(scroll_label), _("Scrollbar size"));
	gtk_misc_set_alignment(GTK_MISC(scroll_label), 0, 0.5);
	gtk_widget_show(scroll_label);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_label);

	/* Translators: Thin scrollbar width. */
	scroll_but1 = gtk_radio_button_new_with_label(NULL, _("Thin"));
	gtk_widget_show(scroll_but1);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but1);

	/* Translators: Thick scrollbar width. */
	scroll_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(scroll_but1), _("Thick"));
	gtk_widget_show(scroll_but2);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but2);
#endif

	/* Use auto portrait mode vbox */
	portrait_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(portrait_vbox);
	gtk_box_pack_start(GTK_BOX(general_vbox), portrait_vbox, FALSE, FALSE, 0);

#ifdef HILDON_HAS_APP_MENU
	portrait_but = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	/* Translators: Whether to enable automatic portrait mode. */
	gtk_button_set_label(GTK_BUTTON(portrait_but), _("Use automatic portrait mode"));
#else
	portrait_but = gtk_check_button_new_with_label(_("Use automatic portrait mode"));
#endif
	gtk_widget_show(portrait_but);
	gtk_box_pack_start(GTK_BOX(portrait_vbox), portrait_but, TRUE, TRUE, 0);

	/* Select Colors vbox */
	color_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(color_vbox);
	gtk_box_pack_start(GTK_BOX(general_vbox), color_vbox, FALSE, FALSE, 0);

#ifdef HILDON_HAS_APP_MENU
	color_but = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	/* Translators: Whether to use custom colors. */
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

	/* Translators: Text color. The color is shown in the dialog, next to
	   the text. */
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

	/* Translators: Background color. The color is shown in the dialog,
	   next to the text. */
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

	/* Translators: Color of links. The color is shown in the dialog, next
	   to the text. */
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

	/* Frame for sync */
	GtkWidget *sync_frame = gtk_frame_new("Synchronization URL (e.g. https://one.ubuntu.com/notes)");
	gtk_frame_set_label_align(GTK_FRAME(sync_frame), 0, -1);
	gtk_widget_show(sync_frame);
	gtk_box_pack_start(GTK_BOX(config_vbox), sync_frame, TRUE, TRUE, 0);

	/* Adding sync section */
	GtkWidget *sync_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(sync_hbox);
	gtk_container_add(GTK_CONTAINER(sync_frame), sync_hbox);

	#ifdef HILDON_HAS_APP_MENU
	GtkWidget *sync_entry = hildon_entry_new(HILDON_SIZE_FINGER_HEIGHT);
	hildon_entry_set_placeholder(HILDON_ENTRY(sync_entry), "https://one.ubuntu.com/notes");
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

	/* Add the plugin manager widget if it contains more than one plugin */
	GtkWidget *plugin_manager = conboy_plugin_manager_new();
	if (conboy_plugin_manager_get_count(CONBOY_PLUGIN_MANAGER(plugin_manager)) > 1) {
		/* Frame for plugins */
		GtkWidget *plugin_frame = gtk_frame_new("Plug-ins");
		gtk_frame_set_label_align(GTK_FRAME(plugin_frame), 0, -1);
		gtk_widget_show(plugin_frame);
		gtk_box_pack_start(GTK_BOX(config_vbox), plugin_frame, TRUE, TRUE, 0);
		/* Plugin manager */
		gtk_widget_show(plugin_manager);
		gtk_container_add(GTK_CONTAINER(plugin_frame), plugin_manager);
	}


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
