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
#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-gtk.h>
#include <hildon/hildon-check-button.h>
#endif

#include "settings_window.h"
#include "settings.h"
#include "app_data.h"

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

static
GtkWidget *settings_widget_create()
{
	GtkWidget *config_vbox;
	GtkWidget *hbox;
#ifndef HILDON_HAS_APP_MENU
	GtkWidget *scroll_vbox, *scroll_label, *scroll_but1, *scroll_but2;
#endif
	GtkWidget *view_box, *view_button_box, *view_label, *view_but1, *view_but2;
	GtkWidget *color_vbox, *color_but;
	GtkWidget *text_color_hbox, *text_color_but, *text_color_label;
	GtkWidget *back_color_hbox, *back_color_but, *back_color_label;
	GtkWidget *link_color_hbox, *link_color_but, *link_color_label;
	GdkColor color;

	/* Config vbox */
	config_vbox = gtk_vbox_new(FALSE, 20);
	gtk_widget_show(config_vbox);

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
	gtk_label_set_markup(GTK_LABEL(scroll_label), _("Scrollbar Size"));
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

	/* On Startup vbox */
	view_box = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(view_box);
	gtk_container_add(GTK_CONTAINER(hbox), view_box);

	view_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(view_label), _("On Startup"));
	gtk_misc_set_alignment(GTK_MISC(view_label), 0, 0.5);
	gtk_widget_show(view_label);
	gtk_container_add(GTK_CONTAINER(view_box), view_label);

	/* Box for the view toggle buttons */
#ifdef HILDON_HAS_APP_MENU
	view_button_box = gtk_hbox_new(FALSE, 0);
#else
	view_button_box = gtk_vbox_new(FALSE, 0);
#endif
	gtk_widget_show(GTK_WIDGET(view_button_box));
	gtk_container_add(GTK_CONTAINER(view_box), view_button_box);

#ifdef HILDON_HAS_APP_MENU
	/* TODO: Write a bug report. This should be only 2 lines instaed of 6 */
	view_but1 = hildon_gtk_radio_button_new(HILDON_SIZE_FINGER_HEIGHT, NULL);
	view_but2 = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_FINGER_HEIGHT, GTK_RADIO_BUTTON(view_but1));
	gtk_button_set_label(GTK_BUTTON(view_but1), _("Show Note"));
	gtk_button_set_label(GTK_BUTTON(view_but2), _("Show Search"));
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(view_but1), FALSE);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(view_but2), FALSE);
#else
	view_but1 = gtk_radio_button_new_with_label(NULL, _("Show Note"));
	view_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(view_but1), _("Show Search"));
#endif

	gtk_widget_show(view_but1);
	gtk_widget_show(view_but2);

	gtk_box_pack_start(GTK_BOX(view_button_box), view_but1, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(view_button_box), view_but2, TRUE, TRUE, 0);



	/* Select Colors vbox */
	color_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(color_vbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), color_vbox, FALSE, FALSE, 0);

#ifdef HILDON_HAS_APP_MENU
	color_but = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(color_but), _("Use Custom Colors"));
#else
	color_but = gtk_check_button_new_with_label(_("Use Custom Colors"));
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


	/* Set initial values from gconf */

#ifndef HILDON_HAS_APP_MENU
	/* Scrollbars */
	if (settings_load_scrollbar_size() == SETTINGS_SCROLLBAR_SIZE_SMALL) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scroll_but1), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scroll_but2), TRUE);
	}
#endif

	/* On Startup */
	if (settings_load_startup_window() == SETTINGS_STARTUP_WINDOW_NOTE) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(view_but1), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(view_but2), TRUE);
	}

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


	/* Connect signals */
#ifndef HILDON_HAS_APP_MENU
	g_signal_connect(scroll_but1, "toggled", G_CALLBACK(on_scroll_but_toggled), NULL);
#endif
	g_signal_connect(view_but1, "toggled", G_CALLBACK(on_view_but_toggled), NULL);
	g_signal_connect(color_but, "toggled", G_CALLBACK(on_use_colors_but_toggled), text_color_hbox);
	g_signal_connect(text_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_TEXT));
	g_signal_connect(link_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_LINKS));
	g_signal_connect(back_color_but, "released", G_CALLBACK(on_color_but_changed), GINT_TO_POINTER(SETTINGS_COLOR_TYPE_BACKGROUND));

	return config_vbox;
}


void settings_window_open(GtkWindow *parent)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Settings"),
			parent,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_OK,
			NULL);

	GtkWidget *content_area = GTK_DIALOG(dialog)->vbox;
	GtkWidget *content_widget = settings_widget_create();

	/* Add the widget to the dialog */
	gtk_box_pack_start(GTK_BOX(content_area), content_widget, TRUE, TRUE, 10);

	/* When a button (ok/cancel/etc.) is clicked or the dialog is closed - destroy it */
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));
}
