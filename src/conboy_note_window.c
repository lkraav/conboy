
#include <gtk/gtk.h>
#include <hildon/hildon-window.h>

#include "conboy_note.h"
#include "app_data.h"

#include "conboy_note_window.h"

static
void initialize_tags(GtkTextBuffer *buffer) {
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
}

static void
on_but1_clicked(GtkButton *but, gpointer user_data)
{
	g_printerr("Clicked\n");
	
	AppData *app_data = app_data_get();
	
	ConboyNote *the_note = conboy_note_store_get_latest(app_data->note_store);
	
	conboy_note_window_show_note(user_data, the_note);
}

static void
on_but2_clicked(GtkButton *but, gpointer user_data)
{
	ConboyNoteWindow *win = (ConboyNoteWindow*) user_data;
	g_printerr("Title: %s\n", win->note->title);
}

static GtkWidget*
create_toolbar(ConboyNoteWindow *win)
{
	GtkWidget* bar = gtk_toolbar_new();
	gtk_widget_show(bar);
	
	GtkToolItem* but1 = gtk_tool_button_new_from_stock(GTK_STOCK_ABOUT);
	gtk_toolbar_insert(GTK_TOOLBAR(bar), but1, -1);
	gtk_widget_show(GTK_WIDGET(but1));
	
	GtkToolItem* but2 = gtk_tool_button_new_from_stock(GTK_STOCK_CDROM);
	gtk_toolbar_insert(GTK_TOOLBAR(bar), but2, -1);
	gtk_widget_show(GTK_WIDGET(but2));
	
	g_signal_connect(but1, "clicked", G_CALLBACK(on_but1_clicked), win);
	g_signal_connect(but2, "clicked", G_CALLBACK(on_but2_clicked), win);
	
	return bar;
}

static GtkWidget*
create_text_view(ConboyNoteWindow *win)
{
	GtkWidget *scrollable = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrollable), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrollable);
	
	GtkWidget *view = gtk_text_view_new();
	gtk_widget_show(view);
	win->view = GTK_TEXT_VIEW(view);
	win->buffer = gtk_text_view_get_buffer(win->view);
	
	gtk_container_add(GTK_CONTAINER(scrollable), view);
	
	return scrollable;
}

ConboyNoteWindow*
conboy_note_window_new()
{
	ConboyNoteWindow *info = g_new0(ConboyNoteWindow, 1);
	
	GtkWidget *win = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW(win), "Conboy");
	gtk_window_set_default_size(GTK_WINDOW(win), 720, 420);
	gtk_widget_show(win);

	GtkWidget *bar = create_toolbar(info);
	hildon_window_add_toolbar(HILDON_WINDOW(win), GTK_TOOLBAR(bar));

	GtkWidget *view = create_text_view(info);
	gtk_container_add(GTK_CONTAINER(win), view);

	initialize_tags(info->buffer);
	
	return info;
}

void
conboy_note_window_show_note(ConboyNoteWindow* window, ConboyNote *note)
{
	window->note = note;
	g_object_ref(note);
	
	gchar *content;
	g_object_get(note, "content", &content, NULL);
	note_buffer_set_xml(window->buffer, content);
}



