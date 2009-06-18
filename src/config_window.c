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
#include <hildon/hildon-window.h>
#include <hildon/hildon-color-button.h>
#include "config_window.h"


static
HildonWindow *config_window_create()
{
	
	GtkWidget *window;
	GtkWidget *vbox;
	
	GtkWidget *scroll_vbox, *scroll_label, *scroll_but1, *scroll_but2;
	GtkWidget *view_vbox, *view_label, *view_but1, *view_but2;
	GtkWidget *color_vbox, *color_label;
	GtkWidget *text_color_hbox, *text_color_but, *text_color_label;
	GtkWidget *back_color_hbox, *back_color_but, *back_color_label;
	GtkWidget *link_color_hbox, *link_color_but, *link_color_label;
	
	
	window = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW(window), _("Configuration"));
	
	/* Main vbox */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	/* Scrollbar vbox */
	scroll_vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(scroll_vbox);
	gtk_container_add(GTK_CONTAINER(vbox), scroll_vbox);
	
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
	gtk_container_add(GTK_CONTAINER(vbox), view_vbox);
	
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
	gtk_container_add(GTK_CONTAINER(vbox), color_vbox);
	
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
		
	
	
	
	
	/* On startup open... hbox */
	/*
	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox2);
	gtk_container_add(GTK_CONTAINER(vbox), hbox2);
	
	startup_label = gtk_label_new("On startup show:");
	gtk_widget_show(startup_label);
	gtk_container_add(GTK_CONTAINER(hbox2), startup_label);
	
	startup_but1 = gtk_radio_button_new_with_label(NULL, "Note");
	startup_but2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(startup_but1), "Search");
	gtk_widget_show(startup_but1);
	gtk_widget_show(startup_but2);
	gtk_container_add(GTK_CONTAINER(hbox2), startup_but1);
	gtk_container_add(GTK_CONTAINER(hbox2), startup_but2);
	*/
	
	/* Text color stuff */
	/*
	foreground_label = gtk_label_new("Text color:");
	foreground_label = gtk_label_new("Background color:");
	foreground_label = gtk_label_new("Link color:");
	
	foreground_color = hildon_color_button_new();
	background_color = hildon_color_button_new();
	link_color       = hildon_color_button_new();
	
	gtk_widget_show(forground_color);
	gtk_widget_show(background_color);
	gtk_widget_show(link_color);
	*/
	
	
	return HILDON_WINDOW(window);
}

void config_window_open()
{
	/*
	AppData *app_data = app_data_get();

	if (app_data->search_window == NULL) {
		app_data->search_window = search_window_create();
	}

	if (app_data->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(app_data->search_window));
	}
	*/
	GtkWidget *window = config_window_create();
	
	gtk_widget_show(GTK_WIDGET(window));
	gtk_window_present(GTK_WINDOW(window));
}
