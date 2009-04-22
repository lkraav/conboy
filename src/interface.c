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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <hildon/hildon-window.h>


#include "callbacks.h"

#include "serializer.h"
#include "deserializer.h"
#include "metadata.h"
#include "interface.h"

static void set_tool_button_icon_by_name(GtkToolButton *button, const gchar *icon_name)
{
	GtkWidget *image;
	image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_tool_button_set_icon_widget(button, image);
}

static void initialize_tags(GtkTextBuffer *buffer) {
	/*
	 * The order in which we add the tags here defines also the priority of the tags.
	 * So if we apply, for example, bold and italic to one word, it will result
	 * in <italic><bold>word</bold></italic>. Because bold has a higher priority
	 * than italic.
	 */
	gtk_text_buffer_create_tag(buffer, "centered", "justification", GTK_JUSTIFY_CENTER, NULL);
	
	gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(buffer, "strikethrough", "strikethrough", TRUE, NULL);
	gtk_text_buffer_create_tag(buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "monospace", "family", "monospace", NULL);
	gtk_text_buffer_create_tag(buffer, "highlight", "background", "yellow", NULL);
	
	gtk_text_buffer_create_tag(buffer, "size:small", "scale", PANGO_SCALE_SMALL, NULL);
	gtk_text_buffer_create_tag(buffer, "size:large", "scale", PANGO_SCALE_LARGE, NULL);
	gtk_text_buffer_create_tag(buffer, "size:huge", "scale", PANGO_SCALE_X_LARGE, NULL);
	
	gtk_text_buffer_create_tag(buffer, "link:internal", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:url", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:broken", "foreground", "gray", "underline", PANGO_UNDERLINE_SINGLE, NULL);	
	
	gtk_text_buffer_create_tag(buffer, "_title", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, "scale", PANGO_SCALE_X_LARGE, NULL);
	
	/* For debugging */
	/*
	gtk_text_buffer_create_tag(buffer, "list-item-A:1", "indent", -20, "left-margin", 25, "foreground", "red", NULL);
	gtk_text_buffer_create_tag(buffer, "list-item-B:1", "indent", -20, "left-margin", 25, "foreground", "orange", NULL);
	gtk_text_buffer_create_tag(buffer, "list", "background", "gray", NULL);
	*/
	
	gtk_text_buffer_create_tag(buffer, "list-item", "foreground", "orange", NULL);
	
	gtk_text_buffer_create_tag(buffer, "depth:1", "indent", -20, "left-margin", 25, NULL);
	
	gtk_text_buffer_create_tag(buffer, "list", "background", "gray", NULL);
}

static
void set_menu_item_label(GtkMenuItem *item, const gchar *text)
{
	GList *children = gtk_container_get_children(GTK_CONTAINER(item));
	
	if (children == NULL) {
		g_printerr("ERROR: set_menu_item_label() expects a GtkMenuItem, which already contains a label.\n");
		return;
	}
	
	gtk_label_set_markup(GTK_LABEL(children->data), text);		
}

GtkWidget* create_mainwin(Note *note) {
	
	UserInterface *ui = note->ui;
	
	GtkWidget *mainwin;
	GtkWidget *vbox1;
	GtkWidget *menu;
	
	GtkWidget *menu_new;
	GtkWidget *menu_bold;
	GtkWidget *menu_italic;
	GtkWidget *menu_strike;
	GtkWidget *menu_highlight;
	GtkWidget *menu_fixed;
	GtkWidget *menu_bullets;
	GtkWidget *menu_quit;

	GtkWidget *toolbar;
	GtkWidget *scrolledwindow1;
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	
	GtkToolItem *new_button;
	GtkToolItem *bold_button;
	GtkToolItem *italic_button;
	GtkToolItem *strike_button;
	GtkToolItem *bullets_button;
	GtkToolItem *link_button;
	GtkToolItem *delete_button;
	GtkToolItem *notes_button;
	GtkToolItem *highlight_button;
	
	GtkTextTag *link_internal_tag;
	
	GtkAccelGroup *accel_group;
	
	PangoFontDescription *font;
	AppData *app_data = get_app_data();

	mainwin = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW(mainwin), ("Conboy"));

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(mainwin), accel_group);
	
	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox1);

	
	/* MENU */
	menu = gtk_menu_new();
	
	menu_new = gtk_menu_item_new_with_label("Create New Note");
	menu_bold = gtk_check_menu_item_new_with_label("");
	menu_italic = gtk_check_menu_item_new_with_label("");
	menu_strike = gtk_check_menu_item_new_with_label("");
	menu_highlight = gtk_check_menu_item_new_with_label("");
	menu_fixed = gtk_check_menu_item_new_with_label("");
	menu_bullets = gtk_check_menu_item_new_with_label("Bullets");
	menu_quit = gtk_menu_item_new_with_label("Close all notes");
	
	set_menu_item_label(GTK_MENU_ITEM(menu_bold), "<b>Bold</b>");
	set_menu_item_label(GTK_MENU_ITEM(menu_italic), "<i>Italic</i>");
	set_menu_item_label(GTK_MENU_ITEM(menu_strike), "<s>Strikeout</s>");
	set_menu_item_label(GTK_MENU_ITEM(menu_highlight), "<span background=\"yellow\">Highlight</span>");
	set_menu_item_label(GTK_MENU_ITEM(menu_fixed), "<tt>Fixed Width</tt>");
	
	gtk_widget_add_accelerator(menu_bold, "activate", accel_group, GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(menu_italic, "activate", accel_group, GDK_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(menu_strike, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(menu_highlight, "activate", accel_group, GDK_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(menu_fixed, "activate", accel_group, GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(menu_quit, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	gtk_menu_append(menu, menu_new);
	gtk_menu_append(menu, gtk_separator_menu_item_new());
	gtk_menu_append(menu, menu_bold);
	gtk_menu_append(menu, menu_italic);
	gtk_menu_append(menu, menu_strike);
	gtk_menu_append(menu, menu_highlight);
	gtk_menu_append(menu, menu_fixed);
	gtk_menu_append(menu, gtk_separator_menu_item_new());
	gtk_menu_append(menu, menu_bullets);
	gtk_menu_append(menu, gtk_separator_menu_item_new());
	gtk_menu_append(menu, menu_quit);
	

	/* Must be at the end of the menu definition */
	hildon_window_set_menu(HILDON_WINDOW(mainwin), GTK_MENU(menu));

	/* TOOLBAR */
	toolbar = gtk_toolbar_new();
	
	new_button = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_button, -1);
	
	bold_button = gtk_toggle_tool_button_new();
	set_tool_button_icon_by_name(GTK_TOOL_BUTTON(bold_button), "qgn_list_gene_bold");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bold_button, -1);
	gtk_widget_set_size_request(GTK_WIDGET(bold_button), 70, -1);
	
	italic_button = gtk_toggle_tool_button_new();
	set_tool_button_icon_by_name(GTK_TOOL_BUTTON(italic_button), "qgn_list_gene_italic");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), italic_button, -1);
	
	strike_button = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_STRIKETHROUGH);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), strike_button, -1);
	
	bullets_button = gtk_toggle_tool_button_new();
	set_tool_button_icon_by_name(GTK_TOOL_BUTTON(bullets_button), "qgn_list_gene_bullets");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bullets_button, -1);
		
	highlight_button = gtk_toggle_tool_button_new();                 
	set_tool_button_icon_by_name(GTK_TOOL_BUTTON(highlight_button), "highlighter");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), highlight_button, -1);
	
	link_button = gtk_tool_button_new_from_stock(GTK_STOCK_REDO);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), link_button, -1);
	
	delete_button = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), delete_button, -1);
	
	notes_button = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), notes_button, -1);
	

	gtk_widget_show_all(toolbar);

	hildon_window_add_toolbar(HILDON_WINDOW(mainwin), GTK_TOOLBAR(toolbar));
	/* TODO: Maybe we can use one intance of the toolbar for all windows. */
	/*
	HildonProgram *prg = hildon_program_get_instance();
	if (hildon_program_get_common_toolbar(prg) == NULL) {
		g_printerr("ADD THE TOOLBAR \n");
		hildon_program_set_common_toolbar(prg, toolbar);
	}
	*/

	/* SCROLLED WINDOW */
	scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwindow1);
	gtk_box_pack_start(GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	/* TEXT VIEW */
	textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
	gtk_widget_show(textview);
	gtk_container_add(GTK_CONTAINER (scrolledwindow1), textview);
	
	/* TEXT BUFFER */
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	
	/* Font */
	font = pango_font_description_new();
	pango_font_description_set_size(font, app_data->font_size);
	gtk_widget_modify_font(GTK_WIDGET(textview), font);

	
	/* Enable support for tap and hold on the textview */
	gtk_widget_tap_and_hold_setup(textview, NULL, NULL, 0);
	
	/* Create basic set of tags */
	initialize_tags(buffer);
	
	/* Save for later usage */
	ui->window = HILDON_WINDOW(mainwin);
	ui->view = GTK_TEXT_VIEW(textview);
	ui->buffer = buffer;
	
	ui->button_bold = GTK_TOGGLE_TOOL_BUTTON(bold_button);
	ui->button_bullets = GTK_TOGGLE_TOOL_BUTTON(bullets_button);
	ui->button_highlight = GTK_TOGGLE_TOOL_BUTTON(highlight_button);
	ui->button_italic = GTK_TOGGLE_TOOL_BUTTON(italic_button);
	ui->button_strike = GTK_TOGGLE_TOOL_BUTTON(strike_button);
	
	ui->menu_bold = GTK_CHECK_MENU_ITEM(menu_bold);
	ui->menu_bullets = GTK_CHECK_MENU_ITEM(menu_bullets);
	ui->menu_highlight = GTK_CHECK_MENU_ITEM(menu_highlight);
	ui->menu_italic = GTK_CHECK_MENU_ITEM(menu_italic);
	ui->menu_strike = GTK_CHECK_MENU_ITEM(menu_strike);
	ui->menu_fixed = GTK_CHECK_MENU_ITEM(menu_fixed);
	

	/* Window signals */
	g_signal_connect (G_OBJECT(mainwin), "delete-event",
	        G_CALLBACK(on_window_close_button_clicked),
	        note);
	
	/* MenuItem signals */
	g_signal_connect ((gpointer) menu_new, "activate",
			G_CALLBACK(on_new_button_clicked),
			ui);
	
	g_signal_connect ((gpointer) menu_bold, "activate",
			G_CALLBACK(on_bold_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_italic, "activate",
			G_CALLBACK (on_italic_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_strike, "activate",
			G_CALLBACK (on_strike_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_highlight, "activate",
			G_CALLBACK (on_highlight_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_fixed, "activate",
			G_CALLBACK (on_fixed_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_bullets, "activate",
			G_CALLBACK (on_bullets_button_clicked),
			note);
	
	g_signal_connect ((gpointer) menu_quit, "activate",
			G_CALLBACK (on_quit_button_clicked),
			NULL);

	/* ToolButton signals */
	g_signal_connect ((gpointer) new_button, "clicked",
			G_CALLBACK (on_new_button_clicked),
			NULL);
	
	g_signal_connect ((gpointer) bold_button, "clicked",
			G_CALLBACK (on_bold_button_clicked),
			note);
	
	g_signal_connect ((gpointer) italic_button, "clicked",
			G_CALLBACK (on_italic_button_clicked),
			note);
	
	g_signal_connect ((gpointer) strike_button, "clicked",
			G_CALLBACK (on_strike_button_clicked),
			note);
	
	g_signal_connect ((gpointer) bullets_button, "clicked",
			G_CALLBACK(on_bullets_button_clicked),
			note);
	
	g_signal_connect ((gpointer) highlight_button, "clicked",
			G_CALLBACK (on_highlight_button_clicked),
			note);
	
	g_signal_connect ((gpointer) link_button, "clicked",
			G_CALLBACK (on_link_button_clicked),
			note);
	
	g_signal_connect ((gpointer) notes_button, "clicked",
			G_CALLBACK (on_notes_button_clicked),
			NULL);
	
	g_signal_connect((gpointer)delete_button, "clicked",
			G_CALLBACK(on_delete_button_clicked),
			note);
	
	g_signal_connect ((gpointer) buffer, "mark-set",
			G_CALLBACK (on_textview_cursor_moved),
			note);
	
	g_signal_connect ((gpointer) buffer, "changed",
			G_CALLBACK (on_textbuffer_changed),
			mainwin);
	
	g_signal_connect ((gpointer)buffer, "modified-changed",
			G_CALLBACK(on_text_buffer_modified_changed),
			note);
	
	g_signal_connect ((gpointer)textview, "tap-and-hold",
			G_CALLBACK(on_textview_tap_and_hold),
			NULL);
	
	g_signal_connect ((gpointer)mainwin, "key_press_event",
			G_CALLBACK(on_hardware_key_pressed),
			note);
	
	g_signal_connect ((gpointer)textview, "key-press-event",
			G_CALLBACK(on_text_view_key_pressed),
			note);
	
	g_signal_connect_after ((gpointer)buffer, "insert-text",
			G_CALLBACK(on_text_buffer_insert_text),
			note);

	link_internal_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	g_signal_connect ((gpointer) link_internal_tag, "event",
			G_CALLBACK (on_link_internal_tag_event),
			note);
	
	
	return mainwin;
}

