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
#include <hildon/hildon-find-toolbar.h>
#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-text-view.h>
#endif


#include "callbacks.h"

#include "serializer.h"
#include "deserializer.h"
#include "metadata.h"
#include "interface.h"

#define ACCEL_PATH_ROOT            "<NOTE_WINDOW>"
#define ACCEL_PATH_QUIT            ACCEL_PATH_ROOT"/Quit"
#define ACCEL_PATH_NEW             ACCEL_PATH_ROOT"/New"

#define ACCEL_PATH_STYLE           ACCEL_PATH_ROOT"/Style"
#define ACCEL_PATH_STYLE_BOLD      ACCEL_PATH_STYLE"/Bold"
#define ACCEL_PATH_STYLE_ITALIC    ACCEL_PATH_STYLE"/Italic"
#define ACCEL_PATH_STYLE_FIXED     ACCEL_PATH_STYLE"/Fixed"
#define ACCEL_PATH_STYLE_STRIKE    ACCEL_PATH_STYLE"/Strike"
#define ACCEL_PATH_STYLE_HIGHLIGHT ACCEL_PATH_STYLE"/Highlight"

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

	gtk_text_buffer_create_tag(buffer, "list-item", NULL);
	gtk_text_buffer_create_tag(buffer, "list", NULL);

	/*
	gtk_text_buffer_create_tag(buffer, "list-item", "foreground", "orange", NULL);
	gtk_text_buffer_create_tag(buffer, "list", "background", "gray", NULL);
	*/
}

/* TODO: Put in util file, it's used in callbacks.c too */
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

static
void set_button_label(GtkMenuItem *item, const gchar *text)
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
	GtkMenuShell *main_menu;
	GtkMenuShell *text_style_menu;

	GtkWidget *menu_new;
	GtkWidget *menu_bold;
	GtkWidget *menu_italic;
	GtkWidget *menu_strike;
	GtkWidget *menu_highlight;
	GtkWidget *menu_fixed;
	GtkWidget *menu_bullets;
	GtkWidget *menu_quit;
	GtkWidget *menu_text_style;
	GtkWidget *menu_inc_indent;
	GtkWidget *menu_dec_indent;
	GtkWidget *menu_font_small;
	GtkWidget *menu_font_normal;
	GtkWidget *menu_font_large;
	GtkWidget *menu_font_huge;
	GtkWidget *menu_find;

	GtkWidget *toolbar;
	GtkWidget *find_bar;
	GtkWidget *scrolledwindow1;
	GtkWidget *textview;
	GtkTextBuffer *buffer;

	GtkToolItem *button_link;
	GtkToolItem *button_delete;
	GtkToolItem *button_notes;
	GtkToolItem *button_inc_indent;
	GtkToolItem *button_dec_indent;
	GtkToolItem *button_style;
	GtkToolItem *button_find;

	GtkAction *action_new;
	GtkAction *action_delete;
	GtkAction *action_quit;
	GtkAction *action_notes;
	GtkAction *action_bold;
	GtkAction *action_fixed;
	GtkAction *action_highlight;
	GtkAction *action_italic;
	GtkAction *action_strike;
	GtkAction *action_bullets;
	GtkAction *action_link;
	GtkAction *action_inc_indent;
	GtkAction *action_dec_indent;
	GtkAction *action_text_style;
	GtkAction *action_zoom_in;
	GtkAction *action_zoom_out;
	GtkAction *action_font_small;
	GtkAction *action_font_normal;
	GtkAction *action_font_large;
	GtkAction *action_font_huge;
	GtkAction *action_find;

	GtkTextTag *link_internal_tag;

	GSList *action_group = NULL;

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

	/* ACTIONS */
	action_bold = GTK_ACTION(gtk_toggle_action_new("bold", "Bold", NULL, GTK_STOCK_BOLD));
	action_bullets = GTK_ACTION(gtk_toggle_action_new("bullets", "Bullets", NULL, NULL));
	action_dec_indent = GTK_ACTION(gtk_action_new("dec_indent", "Decrease Indent", NULL, GTK_STOCK_UNINDENT));
	action_delete = GTK_ACTION(gtk_action_new("delete", "Delete Note", NULL, GTK_STOCK_DELETE));
	action_fixed = GTK_ACTION(gtk_toggle_action_new("monospace", "Fixed Width", NULL, NULL));
	action_highlight = GTK_ACTION(gtk_toggle_action_new("highlight", "Highlight", NULL, NULL));
	action_inc_indent = GTK_ACTION(gtk_action_new("inc_indent", "Increase Indent", NULL, GTK_STOCK_INDENT));
	action_link = GTK_ACTION(gtk_action_new("link", "Link", NULL, GTK_STOCK_REDO));
	action_new = GTK_ACTION(gtk_action_new("new", "New Note", NULL, NULL));
	action_notes = GTK_ACTION(gtk_action_new("open", "Open", NULL, GTK_STOCK_OPEN));
	action_quit = GTK_ACTION(gtk_action_new("quit", "Close All Notes", NULL, NULL));
	action_italic = GTK_ACTION(gtk_toggle_action_new("italic", "Italic", NULL, GTK_STOCK_ITALIC));
	action_strike = GTK_ACTION(gtk_toggle_action_new("strikethrough", "Strikeout", NULL, GTK_STOCK_STRIKETHROUGH));
	action_text_style = GTK_ACTION(gtk_action_new("style", "Style", NULL, GTK_STOCK_SELECT_FONT));
	action_zoom_in = GTK_ACTION(gtk_action_new("zoom_in", "Zoom In", NULL, GTK_STOCK_ZOOM_IN));
	action_zoom_out = GTK_ACTION(gtk_action_new("zoom_out", "Zoom Out", NULL, GTK_STOCK_ZOOM_OUT));
	action_find = GTK_ACTION(gtk_action_new("find", "Find In Note", NULL, GTK_STOCK_FIND));
	/* TODO: Use an enum instead of 0 to 3 */
	action_font_small = GTK_ACTION(gtk_radio_action_new("size:small", "Small", NULL, NULL, 0));
	action_font_normal = GTK_ACTION(gtk_radio_action_new("size:normal", "Normal", NULL, NULL, 1));
	action_font_large = GTK_ACTION(gtk_radio_action_new("size:large", "Large", NULL, NULL, 2));
	action_font_huge = GTK_ACTION(gtk_radio_action_new("size:huge", "Huge", NULL, NULL, 3));

	gtk_action_set_sensitive(action_link, FALSE);
	gtk_action_set_sensitive(action_inc_indent, FALSE);
	gtk_action_set_sensitive(action_dec_indent, FALSE);

	/* Build the radio action group */
	gtk_radio_action_set_group(action_font_small, action_group);
	action_group = gtk_radio_action_get_group(action_font_small);
	gtk_radio_action_set_group(action_font_normal, action_group);
	action_group = gtk_radio_action_get_group(action_font_normal);
	gtk_radio_action_set_group(action_font_large, action_group);
	action_group = gtk_radio_action_get_group(action_font_large);
	gtk_radio_action_set_group(action_font_huge, action_group);
	action_group = gtk_radio_action_get_group(action_font_huge);
	gtk_radio_action_set_current_value(action_font_normal, 1);

	/* Add actions to accelerator group */
	gtk_action_set_accel_group(action_bold, accel_group);
	gtk_action_set_accel_group(action_italic, accel_group);
	gtk_action_set_accel_group(action_fixed, accel_group);
	gtk_action_set_accel_group(action_highlight, accel_group);
	gtk_action_set_accel_group(action_strike, accel_group);
	gtk_action_set_accel_group(action_quit, accel_group);
	gtk_action_set_accel_group(action_new, accel_group);

	/* Add accelerator paths to actions */
	gtk_action_set_accel_path(action_bold,      ACCEL_PATH_STYLE_BOLD);
	gtk_action_set_accel_path(action_italic,    ACCEL_PATH_STYLE_ITALIC);
	gtk_action_set_accel_path(action_strike,    ACCEL_PATH_STYLE_STRIKE);
	gtk_action_set_accel_path(action_highlight, ACCEL_PATH_STYLE_HIGHLIGHT);
	gtk_action_set_accel_path(action_fixed,     ACCEL_PATH_STYLE_FIXED);
	gtk_action_set_accel_path(action_quit,      ACCEL_PATH_QUIT);
	gtk_action_set_accel_path(action_new,       ACCEL_PATH_NEW);

	/* Set keybindings for accelerator paths */
	gtk_accel_map_add_entry(ACCEL_PATH_STYLE_BOLD,      GDK_b, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_STYLE_ITALIC,    GDK_i, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_STYLE_STRIKE,    GDK_s, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_STYLE_HIGHLIGHT, GDK_h, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_STYLE_FIXED,     GDK_f, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_QUIT,            GDK_q, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry(ACCEL_PATH_NEW,             GDK_n, GDK_CONTROL_MASK);



	/* FORMAT MENU */
#ifdef HILDON_HAS_APP_MENU
	text_style_menu = hildon_app_menu_new();

	menu_bold = gtk_button_new();
	menu_italic = gtk_button_new();
	menu_strike = gtk_button_new();
	menu_highlight = gtk_button_new();
	menu_fixed = gtk_button_new();
	menu_bullets = gtk_button_new();
	menu_inc_indent = gtk_button_new();
	menu_dec_indent = gtk_button_new();

	menu_font_small = gtk_radio_button_new(NULL);
	menu_font_normal = gtk_radio_button_new(NULL);
	menu_font_large = gtk_radio_button_new(NULL);
	menu_font_huge = gtk_radio_button_new(NULL);

	gtk_action_connect_proxy(action_bold, menu_bold);
	gtk_action_connect_proxy(action_italic, menu_italic);
	gtk_action_connect_proxy(action_strike, menu_strike);
	gtk_action_connect_proxy(action_highlight, menu_highlight);
	gtk_action_connect_proxy(action_fixed, menu_fixed);
	gtk_action_connect_proxy(action_bullets, menu_bullets);
	gtk_action_connect_proxy(action_inc_indent, menu_inc_indent);
	gtk_action_connect_proxy(action_dec_indent, menu_dec_indent);
	gtk_action_connect_proxy(action_font_small, menu_font_small);
	gtk_action_connect_proxy(action_font_normal, menu_font_normal);
	gtk_action_connect_proxy(action_font_large, menu_font_large);
	gtk_action_connect_proxy(action_font_huge, menu_font_huge);

	set_button_label(menu_bold, "<b>Bold</b>");
	set_button_label(menu_italic, "<i>Italic</i>");
	set_button_label(menu_strike, "<s>Strikeout</s>");
	set_button_label(menu_highlight, "<span background=\"yellow\">Highlight</span>");
	set_button_label(menu_fixed, "<tt>Fixed Width</tt>");
	set_button_label(menu_font_small, "<span size=\"small\">Small</span>");
	set_button_label(menu_font_large, "<span size=\"large\">Large</span>");
	set_button_label(menu_font_huge, "<span size=\"x-large\">Huge</span>");

	hildon_app_menu_append(text_style_menu, menu_bold);
	hildon_app_menu_append(text_style_menu, menu_italic);
	hildon_app_menu_append(text_style_menu, menu_strike);
	hildon_app_menu_append(text_style_menu, menu_highlight);
	hildon_app_menu_append(text_style_menu, menu_fixed);
	hildon_app_menu_append(text_style_menu, menu_bullets);
	hildon_app_menu_append(text_style_menu, menu_inc_indent);
	hildon_app_menu_append(text_style_menu, menu_dec_indent);
	hildon_app_menu_append(text_style_menu, menu_font_normal);
	hildon_app_menu_append(text_style_menu, menu_font_small);
	hildon_app_menu_append(text_style_menu, menu_font_large);
	hildon_app_menu_append(text_style_menu, menu_font_huge);

#else
	text_style_menu = GTK_MENU_SHELL(gtk_menu_new());

	menu_bold = gtk_action_create_menu_item(action_bold);
	menu_italic = gtk_action_create_menu_item(action_italic);
	menu_strike = gtk_action_create_menu_item(action_strike);
	menu_highlight = gtk_action_create_menu_item(action_highlight);
	menu_fixed = gtk_action_create_menu_item(action_fixed);
	menu_bullets = gtk_action_create_menu_item(action_bullets);
	menu_inc_indent = gtk_action_create_menu_item(action_inc_indent);
	menu_dec_indent = gtk_action_create_menu_item(action_dec_indent);
	menu_font_small = gtk_action_create_menu_item(action_font_small);
	menu_font_normal = gtk_action_create_menu_item(action_font_normal);
	menu_font_large = gtk_action_create_menu_item(action_font_large);
	menu_font_huge = gtk_action_create_menu_item(action_font_huge);

	set_menu_item_label(GTK_MENU_ITEM(menu_bold),       "<b>Bold</b>");
	set_menu_item_label(GTK_MENU_ITEM(menu_italic),     "<i>Italic</i>");
	set_menu_item_label(GTK_MENU_ITEM(menu_strike),     "<s>Strikeout</s>");
	set_menu_item_label(GTK_MENU_ITEM(menu_highlight),  "<span background=\"yellow\">Highlight</span>");
	set_menu_item_label(GTK_MENU_ITEM(menu_fixed),      "<tt>Fixed Width</tt>");
	set_menu_item_label(GTK_MENU_ITEM(menu_font_small), "<span size=\"small\">Small</span>");
	set_menu_item_label(GTK_MENU_ITEM(menu_font_large), "<span size=\"large\">Large</span>");
	set_menu_item_label(GTK_MENU_ITEM(menu_font_huge),  "<span size=\"x-large\">Huge</span>");

	gtk_menu_shell_append(text_style_menu, menu_bold);
	gtk_menu_shell_append(text_style_menu, menu_italic);
	gtk_menu_shell_append(text_style_menu, menu_strike);
	gtk_menu_shell_append(text_style_menu, menu_highlight);
	gtk_menu_shell_append(text_style_menu, menu_fixed);
	gtk_menu_shell_append(text_style_menu, gtk_separator_menu_item_new());
	gtk_menu_shell_append(text_style_menu, menu_font_small);
	gtk_menu_shell_append(text_style_menu, menu_font_normal);
	gtk_menu_shell_append(text_style_menu, menu_font_large);
	gtk_menu_shell_append(text_style_menu, menu_font_huge);
	gtk_menu_shell_append(text_style_menu, gtk_separator_menu_item_new());
	gtk_menu_shell_append(text_style_menu, menu_bullets);
	gtk_menu_shell_append(text_style_menu, menu_inc_indent);
	gtk_menu_shell_append(text_style_menu, menu_dec_indent);
#endif



	/* MAIN MENU */
#ifdef HILDON_HAS_APP_MENU
	main_menu = hildon_app_menu_new();

	menu_quit = gtk_button_new();
	menu_new = gtk_button_new();

	gtk_action_connect_proxy(action_new, menu_new);
	gtk_action_connect_proxy(action_quit, menu_quit);

	/*
	GtkWidget *one = gtk_button_new_with_label("Something longer");
	GtkWidget *two = gtk_button_new_with_label("Something very very long");
	GtkWidget *copy = gtk_button_new_with_label("Highlight");
	GtkWidget *paste = gtk_button_new_with_label("Strikethrough");

	set_button_label(paste, "<b>Bold</b>");
	set_button_label(copy, "<i>Italic</i>");
	*/

	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_new));
	hildon_app_menu_append(HILDON_APP_MENU(main_menu), GTK_BUTTON(menu_quit));

	hildon_window_set_app_menu(HILDON_WINDOW(mainwin), HILDON_APP_MENU(main_menu));

#else
	main_menu = gtk_menu_new();

	menu_new = gtk_action_create_menu_item(action_new);
	menu_text_style = gtk_menu_item_new_with_label("Text Style");
	gtk_menu_item_set_submenu(menu_text_style, text_style_menu);
	menu_find = gtk_action_create_menu_item(action_find);
	menu_quit = gtk_action_create_menu_item(action_quit);

	gtk_menu_shell_append(main_menu, menu_new);
	gtk_menu_shell_append(main_menu, gtk_separator_menu_item_new());
	gtk_menu_shell_append(main_menu, menu_text_style);
	gtk_menu_shell_append(main_menu, gtk_separator_menu_item_new());
	gtk_menu_shell_append(main_menu, menu_find);
	gtk_menu_shell_append(main_menu, gtk_separator_menu_item_new());
	gtk_menu_shell_append(main_menu, menu_quit);

	/* Must be at the end of the menu definition */
	hildon_window_set_menu(HILDON_WINDOW(mainwin), GTK_MENU(main_menu));
#endif





	/* TOOLBAR */
	toolbar = gtk_toolbar_new();

	/****/
	button_dec_indent = gtk_action_create_tool_item(action_dec_indent);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_dec_indent, -1);
	gtk_widget_set_size_request(GTK_WIDGET(button_dec_indent), 100, -1);

	button_inc_indent = gtk_action_create_tool_item(action_inc_indent);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_inc_indent, -1);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

	button_link = gtk_action_create_tool_item(action_link);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_link, -1);

	button_style = gtk_action_create_tool_item(action_text_style);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_style, -1);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

	button_find = gtk_action_create_tool_item(action_find);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_find, -1);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

	button_delete = gtk_action_create_tool_item(action_delete);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_delete, -1);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

	button_notes = gtk_action_create_tool_item(action_notes);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_notes, -1);


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

	/* FIND TOOL BAR */
	find_bar = hildon_find_toolbar_new("Find");
	hildon_window_add_toolbar(mainwin, find_bar);

	/* SCROLLED WINDOW */
#ifdef HILDON_HAS_APP_MENU
	g_printerr("Using pannable area");
	scrolledwindow1 = hildon_pannable_area_new();
#else
	scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#endif
	gtk_widget_show(scrolledwindow1);
	gtk_box_pack_start(GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);

	/* TEXT VIEW */
#ifdef HILDON_HAS_APP_MENU
	textview = hildon_text_view_new();
	buffer = hildon_text_view_get_buffer(HILDON_TEXT_VIEW(textview));
#else
	textview = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
#endif
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
	gtk_widget_show(textview);
	gtk_container_add(GTK_CONTAINER (scrolledwindow1), textview);

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
	ui->find_bar = HILDON_FIND_TOOLBAR(find_bar);
	ui->find_bar_is_visible = FALSE;
	ui->style_menu = text_style_menu;

	ui->action_bold = GTK_TOGGLE_ACTION(action_bold);
	ui->action_bullets = GTK_TOGGLE_ACTION(action_bullets);
	ui->action_highlight = GTK_TOGGLE_ACTION(action_highlight);
	ui->action_italic = GTK_TOGGLE_ACTION(action_italic);
	ui->action_strike = GTK_TOGGLE_ACTION(action_strike);
	ui->action_fixed = GTK_TOGGLE_ACTION(action_fixed);
	ui->action_link = GTK_ACTION(action_link);
	ui->action_font_small = GTK_RADIO_ACTION(action_font_small);
	ui->action_inc_indent = GTK_ACTION(action_inc_indent);
	ui->action_dec_indent = GTK_ACTION(action_dec_indent);

	/* Window signals */
	g_signal_connect(mainwin, "delete-event",
	        G_CALLBACK(on_window_close_button_clicked),
	        note);

	/* Action signals */
	g_signal_connect(action_new, "activate",
			G_CALLBACK(on_new_button_clicked),
			ui);

	g_signal_connect(action_bold, "activate",
			G_CALLBACK(on_format_button_clicked),
			note);

	g_signal_connect(action_italic, "activate",
			G_CALLBACK(on_format_button_clicked),
			note);

	g_signal_connect(action_strike, "activate",
			G_CALLBACK(on_format_button_clicked),
			note);

	g_signal_connect(action_highlight, "activate",
			G_CALLBACK(on_format_button_clicked),
			note);

	g_signal_connect(action_fixed, "activate",
			G_CALLBACK(on_format_button_clicked),
			note);

	/* It is enough to listen to one action of the radio group */
	g_signal_connect(action_font_small, "changed",
			G_CALLBACK(on_font_size_radio_group_changed),
			note);

	g_signal_connect(action_bullets, "activate",
			G_CALLBACK(on_bullets_button_clicked),
			note);

	g_signal_connect(action_quit, "activate",
			G_CALLBACK(on_quit_button_clicked),
			NULL);

	g_signal_connect(action_inc_indent, "activate",
			G_CALLBACK(on_inc_indent_button_clicked),
			buffer);

	g_signal_connect(action_dec_indent, "activate",
			G_CALLBACK(on_dec_indent_button_clicked),
			buffer);

	g_signal_connect(action_link, "activate",
			G_CALLBACK(on_link_button_clicked),
			note);

	g_signal_connect(action_notes, "activate",
			G_CALLBACK(on_notes_button_clicked),
			mainwin);

	g_signal_connect(action_delete, "activate",
			G_CALLBACK(on_delete_button_clicked),
			note);

	g_signal_connect(action_zoom_in, "activate",
			G_CALLBACK(on_smaller_button_clicked),
			NULL);

	g_signal_connect(action_zoom_in, "activate",
			G_CALLBACK(on_bigger_button_clicked),
			NULL);

	g_signal_connect(action_text_style, "activate",
			G_CALLBACK(on_style_button_clicked),
			ui);

	g_signal_connect(action_find, "activate",
			G_CALLBACK(on_find_button_clicked),
			note);


	/* OTHER SIGNALS */
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

	g_signal_connect_after ((gpointer)buffer, "delete-range",
			G_CALLBACK(on_text_buffer_delete_range),
			note);

	g_signal_connect((gpointer)find_bar, "search",
			G_CALLBACK(on_find_bar_search),
			note);

	g_signal_connect((gpointer)find_bar, "close",
			G_CALLBACK(on_find_bar_close),
			ui);

	link_internal_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	g_signal_connect ((gpointer) link_internal_tag, "event",
			G_CALLBACK (on_link_internal_tag_event),
			note);


	return mainwin;
}

