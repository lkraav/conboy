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

#include "config_window.h"
#include "settings.h"
#include "app_data.h"

static void
on_scroll_but_toggled(GtkToggleButton *button, gpointer user_data)
{
	AppData *app_data = app_data_get();
	if (gtk_toggle_button_get_active(button)) {
		gconf_client_set_int(app_data->client, SETTINGS_SCROLLBAR_SIZE, SETTINGS_SCROLLBAR_SIZE_SMALL, NULL);
		g_printerr("SMALL \n");
	} else {
		gconf_client_set_int(app_data->client, SETTINGS_SCROLLBAR_SIZE, SETTINGS_SCROLLBAR_SIZE_BIG, NULL);
		g_printerr("LARGE \n");
	}
}

static void
on_view_but_toggled(GtkToggleButton *button, gpointer user_data)
{
	AppData *app_data = app_data_get();
	if (gtk_toggle_button_get_active(button)) {
		gconf_client_set_int(app_data->client, SETTINGS_STARTUP_WINDOW, SETTINGS_STARTUP_WINDOW_NOTE, NULL);
		g_printerr("NOTE \n");
	} else {
		gconf_client_set_int(app_data->client, SETTINGS_STARTUP_WINDOW, SETTINGS_STARTUP_WINDOW_SEARCH, NULL);
		g_printerr("SEARCH \n");
	}
}

/**
 * You have to free the gchar after using.
 */
static gchar
*gdk_color_to_string(GdkColor *color)
{
	PangoColor pColor;
	
	pColor.red = color->red;
	pColor.green = color->green;
	pColor.blue = color->blue;
	
	return pango_color_to_string(&pColor);
}


static void
on_color_but_changed(HildonColorButton *button, gchar *gconf_path)
{
	AppData *app_data = app_data_get();
	GdkColor color;
	gchar *hex_color;
	hildon_color_button_get_color(button, &color);	
	hex_color = gdk_color_to_string(&color);
	g_printerr("Change to: %s \n", hex_color);
	gconf_client_set_string(app_data->client, gconf_path, hex_color, NULL);
	g_free(hex_color);
}


static
GtkWidget *config_widget_create()
{
	GtkWidget *config_vbox;
	GtkWidget *hbox;
	GtkWidget *scroll_vbox, *scroll_label, *scroll_but1, *scroll_but2;
	GtkWidget *view_vbox, *view_label, *view_but1, *view_but2;
	GtkWidget *color_vbox, *color_label;
	GtkWidget *text_color_hbox, *text_color_but, *text_color_label;
	GtkWidget *back_color_hbox, *back_color_but, *back_color_label;
	GtkWidget *link_color_hbox, *link_color_but, *link_color_label;
	
	/* Config vbox */
	config_vbox = gtk_vbox_new(FALSE, 20);
	gtk_widget_show(config_vbox);
	
	/* Container for "scrollbar size" and for "on startup" */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), hbox, FALSE, FALSE, 0);
	
	/* Scrollbar vbox */
	scroll_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(scroll_vbox);
	gtk_box_pack_start(GTK_BOX(hbox), scroll_vbox, TRUE, TRUE, 0);
	
	scroll_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(scroll_label), "<b>Scrollbar Size</b>");
	gtk_misc_set_alignment(GTK_MISC(scroll_label), 0, 0.5);
	gtk_widget_show(scroll_label);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_label);
	
	scroll_but1 = gtk_radio_button_new_with_label(NULL, "Small");
	gtk_widget_show(scroll_but1);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but1);
	
	scroll_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(scroll_but1), "Big");
	gtk_widget_show(scroll_but2);
	gtk_container_add(GTK_CONTAINER(scroll_vbox), scroll_but2);
	
	/* On Startup vbox */
	view_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(view_vbox);
	gtk_container_add(GTK_CONTAINER(hbox), view_vbox);
	
	view_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(view_label), "<b>On Startup</b>");
	gtk_misc_set_alignment(GTK_MISC(view_label), 0, 0.5);
	gtk_widget_show(view_label);
	gtk_container_add(GTK_CONTAINER(view_vbox), view_label);
	
	view_but1 = gtk_radio_button_new_with_label(NULL, "Show Note");
	gtk_widget_show(view_but1);
	gtk_container_add(GTK_CONTAINER(view_vbox), view_but1);
	
	view_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(view_but1), "Show Search");
	gtk_widget_show(view_but2);
	gtk_container_add(GTK_CONTAINER(view_vbox), view_but2);
	
	/* Select Colors vbox */
	color_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(color_vbox);
	gtk_box_pack_start(GTK_BOX(config_vbox), color_vbox, FALSE, FALSE, 0);
	
	color_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(color_label), "<b>Colors</b>");
	gtk_misc_set_alignment(GTK_MISC(color_label), 0, 0.5);
	gtk_widget_show(color_label);
	gtk_box_pack_start(GTK_BOX(color_vbox), color_label, TRUE, TRUE, 0);
	
	/* Text Color */
	text_color_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(text_color_hbox);
	gtk_box_pack_start(GTK_BOX(color_vbox), text_color_hbox, TRUE, TRUE, 0);
	
	text_color_but = hildon_color_button_new();
	gtk_widget_show(text_color_but);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), text_color_but, FALSE, FALSE, 0);
	
	text_color_label = gtk_label_new("Text");
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
	
	back_color_label = gtk_label_new("Background");
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
	
	link_color_label = gtk_label_new("Links");
	gtk_misc_set_alignment(GTK_MISC(link_color_label), 0, 0.5);
	gtk_widget_show(link_color_label);
	gtk_box_pack_start(GTK_BOX(text_color_hbox), link_color_label, TRUE, TRUE, 10);
		
	
	/* Connect signals */
	g_signal_connect(scroll_but1, "toggled", G_CALLBACK(on_scroll_but_toggled), NULL);
	g_signal_connect(view_but1, "toggled", G_CALLBACK(on_view_but_toggled), NULL);
	g_signal_connect(text_color_but, "released", G_CALLBACK(on_color_but_changed), SETTINGS_TEXT_COLOR);
	g_signal_connect(link_color_but, "released", G_CALLBACK(on_color_but_changed), SETTINGS_LINK_COLOR);
	g_signal_connect(back_color_but, "released", G_CALLBACK(on_color_but_changed), SETTINGS_BACKGROUND_COLOR);
	
	return config_vbox;
}


void config_window_open(GtkWindow *parent)
{
	GtkDialog *dialog = gtk_dialog_new_with_buttons("Configuration",
			parent,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_OK,
			NULL);
	
	GtkWidget *content_area = dialog->vbox;
	GtkWidget *content_widget = config_widget_create();
	
	/* Add the widget to the dialog */
	gtk_box_pack_start(GTK_BOX(content_area), content_widget, TRUE, TRUE, 10);
	
	/* When a button (ok/cancel/etc.) is clicked or the dialog is closed - destroy it */
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	
	gtk_dialog_run(dialog);	
}
