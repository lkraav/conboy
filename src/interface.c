#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <hildon/hildon-program.h>
#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "callbacks.h"

#include "support.h"

#include "serializer.h"
#include "deserializer.h"
#include "metadata.h"
#include "interface.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)


static void initialize_tags(GtkTextBuffer *buffer) {
	
	gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(buffer, "strikethrough", "strikethrough", TRUE, NULL);
	gtk_text_buffer_create_tag(buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "monospace", "family", "monospace", NULL);
	
	gtk_text_buffer_create_tag(buffer, "size:small", "scale", PANGO_SCALE_SMALL, NULL);
	gtk_text_buffer_create_tag(buffer, "size:large", "scale", PANGO_SCALE_LARGE, NULL);
	gtk_text_buffer_create_tag(buffer, "size:huge", "scale", PANGO_SCALE_X_LARGE, NULL);
	
	gtk_text_buffer_create_tag(buffer, "highlight", "background", "yellow", NULL);
	
	gtk_text_buffer_create_tag(buffer, "link:internal", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:url", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_buffer_create_tag(buffer, "link:broken", "foreground", "gray", "underline", PANGO_UNDERLINE_SINGLE, NULL);	
	
	gtk_text_buffer_create_tag(buffer, "_title", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, "scale", PANGO_SCALE_X_LARGE, NULL);
	
	gtk_text_buffer_create_tag(buffer, "list", "left-margin", 40, NULL);
	gtk_text_buffer_create_tag(buffer, "list-item", "indent", -20, NULL);
	
}

static void register_serializer_and_deserializer(GtkTextBuffer *buffer, Note *note) {
	
	GtkTextBufferDeserializeFunc deserializer;
	GtkTextBufferSerializeFunc   serializer;
	AppData *app_data = get_app_data();
	
	/* Serializer */
	serializer = serialize_to_tomboy;
	app_data->serializer = gtk_text_buffer_register_serialize_format(buffer, "text/xml", serializer, note, NULL);
	
	/* Deserializer */
	deserializer = deserialize_from_tomboy;
	app_data->deserializer = gtk_text_buffer_register_deserialize_format(buffer, "text/xml", deserializer, note, NULL);
}

GtkWidget* create_mainwin(Note *note) {
	
	GtkWidget *mainwin;
	GtkWidget *vbox1;
	GtkWidget *menu;
	GtkWidget *menu_quit;
	GtkWidget *menu_test;

	GtkWidget *toolbar;
	GtkWidget *scrolledwindow1;
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkToolItem *quit_button;
	/*GtkToolItem *save_button;*/
	/*GtkToolItem *load_button;*/
	GtkToolItem *bold_button;
	GtkToolItem *italic_button;
	GtkToolItem *strike_button;
	GtkToolItem *link_button;
	GtkToolItem *delete_button;
	GtkToolItem *notes_button;
	GtkToolItem *smaller_button;
	GtkToolItem *bigger_button;
	GtkTextTag *link_internal_tag;
	
	PangoFontDescription *font;
	AppData *app_data = get_app_data();

	mainwin = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW (mainwin), _("Conboy"));

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox1);

	/* MENU */
	menu = gtk_menu_new();

	menu_test = gtk_menu_item_new_with_label("Test");
	gtk_menu_append(menu, menu_test);

	menu_quit = gtk_menu_item_new_with_label("Close all notes");
	gtk_menu_append(menu, menu_quit);

	/* Must be at the end of the menu definition */
	hildon_window_set_menu(HILDON_WINDOW(mainwin), GTK_MENU(menu));

	/* TOOLBAR */
	toolbar = gtk_toolbar_new();
	
	/*
	quit_button = gtk_tool_button_new_from_stock("gtk-quit");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), quit_button, -1);
	*/

	/*
	save_button = gtk_tool_button_new_from_stock("gtk-save");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar1), save_button, -1);
	
	load_button = gtk_tool_button_new_from_stock("gtk-open");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar1), load_button, -1);
	*/
	
	bold_button = gtk_toggle_tool_button_new_from_stock("gtk-bold");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bold_button, -1);
	
	italic_button = gtk_toggle_tool_button_new_from_stock("gtk-italic");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), italic_button, -1);
	
	strike_button = gtk_toggle_tool_button_new_from_stock("gtk-strikethrough");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), strike_button, -1);
	
	link_button = gtk_tool_button_new_from_stock("gtk-redo");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), link_button, -1);
	
	delete_button = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), delete_button, -1);
	
	notes_button = gtk_tool_button_new_from_stock("gtk-file");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), notes_button, -1);
	
	smaller_button = gtk_tool_button_new_from_stock("gtk-zoom-out");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), smaller_button, -1);
	
	bigger_button = gtk_tool_button_new_from_stock("gtk-zoom-in");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bigger_button, -1);

	gtk_widget_show_all(toolbar);

	/*hildon_window_add_toolbar(HILDON_WINDOW(mainwin), GTK_TOOLBAR(toolbar));*/
	/* TODO: Maybe we can use one intance of the toolbar for all windows. */
	HildonProgram *prg = hildon_program_get_instance();
	if (hildon_program_get_common_toolbar(prg) == NULL) {
		g_printerr("ADD THE TOOLBAR \n");
		hildon_program_set_common_toolbar(prg, toolbar);
	}

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
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	
	initialize_tags(buffer);
	
	note->buffer = buffer;
	note->window = HILDON_WINDOW(mainwin);
	note->view = GTK_TEXT_VIEW(textview);
	if (note->filename == NULL) {
		note->filename = note_get_new_filename();
	}
	
	register_serializer_and_deserializer(buffer, note);
	
	/** FONT **/
	/*
	PangoContext *ctx = gtk_widget_get_pango_context(GTK_WIDGET(textview));
	PangoFontDescription *font = pango_context_get_font_description(ctx);
	*/
	
	font = pango_font_description_new();
	pango_font_description_set_size(font, app_data->font_size);
	gtk_widget_modify_font(GTK_WIDGET(textview), font);
	/*****/

	/* SIGNALS */
	g_signal_connect (G_OBJECT(mainwin), "delete-event",
	        G_CALLBACK(on_window_close_button_clicked),
	        note);
	
	g_signal_connect ((gpointer) menu_quit, "activate",
			G_CALLBACK (on_quit_button_clicked),
			NULL);

	/*
	g_signal_connect ((gpointer) quit_button, "clicked",
			G_CALLBACK (on_quit_button_clicked),
			NULL);
	*/

	/*
	g_signal_connect ((gpointer) save_button, "clicked",
			G_CALLBACK (on_save_button_clicked),
			metadata);
	
	g_signal_connect ((gpointer) load_button, "clicked",
			G_CALLBACK (on_load_button_clicked),
			buffer);
	*/
	
	g_signal_connect ((gpointer) bold_button, "clicked",
			G_CALLBACK (on_bold_button_clicked),
			NULL);
	
	g_signal_connect ((gpointer) italic_button, "clicked",
			G_CALLBACK (on_italic_button_clicked),
			NULL);
	
	g_signal_connect ((gpointer) strike_button, "clicked",
			G_CALLBACK (on_strike_button_clicked),
			NULL);
	
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
			textview);
	
	g_signal_connect ((gpointer) buffer, "changed",
			G_CALLBACK (on_textbuffer_changed),
			mainwin);
	
	g_signal_connect ((gpointer)buffer, "modified-changed",
			G_CALLBACK(on_text_buffer_modified_changed),
			note);
	
	g_signal_connect ((gpointer)smaller_button, "clicked",
			G_CALLBACK(on_smaller_button_clicked),
			textview);
	
	g_signal_connect ((gpointer)bigger_button, "clicked",
			G_CALLBACK(on_bigger_button_clicked),
			textview);
	
	g_signal_connect ((gpointer)mainwin, "key_press_event",
			G_CALLBACK(on_hardware_key_pressed),
			note);
	
	link_internal_tag = gtk_text_tag_table_lookup(buffer->tag_table, "link:internal");
	g_signal_connect ((gpointer) link_internal_tag, "event",
			G_CALLBACK (on_link_internal_tag_event),
			note);
	

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (mainwin, mainwin, "mainwin");
	GLADE_HOOKUP_OBJECT (mainwin, vbox1, "vbox1");
	GLADE_HOOKUP_OBJECT (mainwin, toolbar, "toolbar1");
	/*
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(quit_button), "quit_button");
	*/
	/*
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(save_button), "save_button");
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(load_button), "load_button");
	*/
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(bold_button), "bold_button");
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(italic_button), "italic_button");
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(strike_button), "strike_button");
	GLADE_HOOKUP_OBJECT (mainwin, GTK_WIDGET(strike_button), "link_button");
	GLADE_HOOKUP_OBJECT (mainwin, scrolledwindow1, "scrolledwindow1");
	GLADE_HOOKUP_OBJECT (mainwin, textview, "textview");

	return mainwin;
}

