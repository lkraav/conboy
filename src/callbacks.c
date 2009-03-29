#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-banner.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "callbacks.h"
#include "support.h"

#include "metadata.h"
#include "interface.h"

#include "note.h"

static void change_format(const gchar* tag_name, GtkButton *button) {
	
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkTextIter start_iter, end_iter;
	
	view = lookup_widget(GTK_WIDGET(button), "textview");
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(button))) {
		/* The button just became active, so we should enable the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			gtk_text_buffer_apply_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
		} else {
			/* Nothing is selected */
			/* TODO: Implement that new style should start from here */
		}
	} else {
		/* The button just became deactive, so we should remove the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			gtk_text_buffer_remove_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
		} else {
			/* Nothing is selected */
			/* TODO: Implement that style should be removed from here */
		}
	}
}

gboolean
on_window_close_button_clicked		   (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data)
{
	Note *note = (Note*)user_data;	
	note_save(note);
	note_close_window(note);
	return TRUE; /* True to stop other handler from being invoked */
}

void on_quit_button_clicked(GtkButton *button, gpointer user_data) {
	g_print("Quit button clicked.");
	gtk_main_quit();
}

void on_load_button_clicked(GtkButton *button, gpointer user_data) {
	
	/*
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
	GtkWindow *win = GTK_WINDOW(lookup_widget(GTK_WIDGET(button), "mainwin"));
	
	note_load_to_buffer("/home/conny/test.ser", buffer);
	  
	
	note_format_title(buffer);
	note_set_window_title_from_buffer(win, buffer);
	*/
}

void on_save_button_clicked(GtkButton *button, gpointer user_data) {
	Note *note = (Note*)user_data;
	note_save(note);
}

void
on_bold_button_clicked				   (GtkButton		*button,
										gpointer		 user_data)
{
	g_printerr("BOLD Button clicked\n");
	change_format("bold", button);
}

void
on_italic_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	g_printerr("ITALIC Button clicked\n");
	change_format("italic", button);
}

void
on_strike_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	g_printerr("STRIKE Button clicked\n");
	change_format("strikethrough", button);
}

void
on_link_button_clicked				   (GtkButton		*button,
										gpointer		 user_data) 
{ 	
	Note *note = (Note*)user_data;
	GtkTextIter start, end;
	const gchar *text;
	
	if (!gtk_text_buffer_get_has_selection(note->buffer)) {
		return;
	}
	
	gtk_text_buffer_get_selection_bounds(note->buffer, &start, &end);
	text = gtk_text_iter_get_text(&start, &end);
	gtk_text_buffer_apply_tag_by_name(note->buffer, "link:internal", &start, &end);
	
	note_open_by_title(text);
}


/*
static void
menu_position ( GtkMenu *menu ,
                gint *x,
                gint *y,
                gboolean *push_in ,
                gpointer *thw)
{
    g_return_if_fail (thw -> button );
    * push_in = TRUE;
    *x = thw ->button -> allocation .x + thw ->button -> allocation . width ;
    *y = thw ->button -> allocation .y;
}
*/


void
on_notes_button_clicked				   (GtkButton		*button,
										gpointer		 user_data) {
	
	AppData *app_data = (AppData*)user_data;
	GtkWidget *menu_item;
	Note *note;
	GtkWidget *menu = gtk_menu_new ();
	GList *notes = app_data->all_notes;
	
	while(notes != NULL) {
	
		note = notes->data;
		menu_item = gtk_menu_item_new_with_label(note->title);
		g_signal_connect(G_OBJECT(menu_item), "activate",
		            	 G_CALLBACK(on_notes_menu_item_activated),
		            	 note);
		gtk_menu_append(menu, menu_item);
		notes = notes->next;
	}
	
	gtk_widget_show_all(menu);

	gtk_menu_popup ( GTK_MENU (menu),
	                 NULL,
	                 NULL,
	                 NULL,
	                 NULL,
	                 0,
	                 gtk_get_current_event_time());
	
}

void
on_notes_menu_item_activated		(GtkMenuItem *menuitem,
									 gpointer     user_data)
{
	Note *note = (Note*)user_data;
	note_open(note);
}


/* TODO: The signal "mark-set" is emitted 4 times when clicking into the text. While selecting
 * it's emitted continuesly. */
void
on_textview_cursor_moved			   (GtkTextBuffer	*buffer,
										GtkTextIter		*location,
										GtkTextMark		*mark,
										gpointer		 user_data)
{
	GtkToggleToolButton *bold_button, *italic_button, *strike_button;
	GSList *tags;
	GtkTextTag *tag;
	
	bold_button   = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "bold_button"));
	italic_button = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "italic_button"));
	strike_button = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "strike_button"));
	
	
	/* TODO: This is only workaround for problem with repeated calls. Probably gives problems with
	 * selection. We only call this if the "insert" mark changed. */
	const gchar *mark_name = gtk_text_mark_get_name(mark);
	if ((mark_name == NULL) || (g_strcasecmp(mark_name, "selection_bound") == 0)) {
		return;
	}
	
	tags = gtk_text_iter_get_tags(location);
	
	/*g_printerr("Cursor changed. Iter: %i  Mark: %s\n", gtk_text_iter_get_offset(location), gtk_text_mark_get_name(mark));*/
	
	/* Blocking signals here because the ..set_active() method makes the buttons
	 * emit the clicked signal. And because of this the formatting changes.
	 */
	/* TODO: Replace following with "g_signal_handlers_unblock_matched" */
	g_signal_handlers_block_by_func(bold_button, on_bold_button_clicked, NULL);
	g_signal_handlers_block_by_func(italic_button, on_italic_button_clicked, NULL);
	g_signal_handlers_block_by_func(strike_button, on_strike_button_clicked, NULL);
	
	gtk_toggle_tool_button_set_active(bold_button, FALSE);
	gtk_toggle_tool_button_set_active(italic_button, FALSE);
	gtk_toggle_tool_button_set_active(strike_button, FALSE);
	
	while (tags != NULL) {
		tag = GTK_TEXT_TAG(tags->data);
		if (g_strcasecmp(tag->name, "bold") == 0) {
			gtk_toggle_tool_button_set_active(bold_button, TRUE);
		} else if (g_strcasecmp(tag->name, "italic") == 0) {
			gtk_toggle_tool_button_set_active(italic_button, TRUE);
		} else if (g_strcasecmp(tag->name, "strikethrough") == 0) {
			gtk_toggle_tool_button_set_active(strike_button, TRUE);
		}
		tags = tags->next;
	}
	
	/* unblock signals */
	g_signal_handlers_unblock_by_func(bold_button, on_bold_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(italic_button, on_italic_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(strike_button, on_strike_button_clicked, NULL);
	
	/* TODO: Free tags list */

}

void
on_textview_insert_at_cursor		   (GtkTextView *text_view,
        								gchar       *string,
        								gpointer     user_data)
{
	g_printerr("Insert: %s", string);
}

void
on_textbuffer_changed				(GtkTextBuffer *buffer,
									 gpointer 		user_data)
{
	GtkTextMark *mark;
	GtkTextIter current;
	
	/* If the cursor is in the first logical line, then format
	 * the first line and change the window title.
	 */
	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &current, mark);
	
	if (gtk_text_iter_get_line(&current) != 0) {
		return;
	}
	
	note_format_title(buffer);
	note_set_window_title_from_buffer(GTK_WINDOW(user_data), buffer);
}

gboolean
on_link_internal_tag_event				(GtkTextTag  *tag,
										 GObject     *object,
										 GdkEvent    *event,
										 GtkTextIter *iter,
										 gpointer     user_data)
{
	GdkEventType type = event->type;
	GtkTextIter *start;
	gchar *link_text;
	
	if (type == GDK_BUTTON_RELEASE) {
		if (((GdkEventButton*)event)->button == 1) {
			
			g_printerr("Open Link \n");
			
			if (!gtk_text_iter_begins_tag(iter, tag)) {
				gtk_text_iter_backward_to_tag_toggle(iter, tag);
			}
			
			start = gtk_text_iter_copy(iter);
			
			if (!gtk_text_iter_ends_tag(iter, tag)) {
				gtk_text_iter_forward_to_tag_toggle(iter, tag);
			}
			
			link_text = gtk_text_iter_get_text(start, iter);
			
			g_printerr("Link: >%s< \n", link_text);
			note_open_by_title(link_text);
		}	
	}
		
	return FALSE;
}

void
on_delete_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	/* Popup Dialog ask for sure
	 * Close window
	 * Lower window count
	 * Change open_windows in app_data
	 * Delete file
	 * Free resources
	 */
	Note *note = (Note*)user_data;
	const gchar *message = "<b>Really delete this note?</b>\n\n If you delete a note it is permanently lost.";
	GtkWidget *dialog;
	gint response;
	AppData *app_data;
	
	g_printerr("on_delete_button_clicked() called\n");
	
	dialog = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(note->window),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			message);
	
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	
	if (response != GTK_RESPONSE_YES) {
		return;
	}
	
	/* Block this signal, otherwise the destroy event of the window will trigger saving of the note */
	g_signal_handlers_block_by_func(note->window, on_window_close_button_clicked, note);
	note_close_window(note);
	/* Don't unblock the signal, because the window does not exist anymore */
	
	/* Delete file */
	if (g_unlink(note->filename) == -1) {
		g_printerr("ERROR: The file %s could not be deleted \n", note->filename);
	}
	
	app_data = get_app_data();
	app_data->all_notes = g_list_remove(app_data->all_notes, note);
}

static
gboolean note_save_callback(Note *note)
{
	/* Test, to see if the window is still open */
	if (note != NULL && GTK_IS_TEXT_BUFFER(note->buffer)) {
		note_save(note);
	}
	
	/* Return FALSE to make the Timer stop */
	return FALSE;
}

void
on_text_buffer_modified_changed			(GtkTextBuffer *buffer,
										 gpointer		user_data)
{
	Note *note = (Note*)user_data;
	
	if (!gtk_text_buffer_get_modified(buffer)) {
		g_printerr("Buffer changed from dirty to saved \n");
		return;
	}
	
	g_printerr("Buffer is dirty. Saving in 10 seconds\n");
	
	/* Save 10 seconds after the buffer got dirty */
	g_timeout_add(10000, (GSourceFunc)note_save_callback, note);
}

void
on_smaller_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	AppData *app_data = get_app_data();
	PangoFontDescription *font;
	Note *note;
	GList *note_list;
		
	app_data->font_size -= 5000;
	gconf_client_set_int(app_data->client, "/apps/maemo/conboy/font_size", app_data->font_size, NULL);
	
	font = pango_font_description_new();
	pango_font_description_set_size(font, app_data->font_size);
	
	note_list = g_list_first(app_data->open_notes);
	
	while (note_list != NULL) {
		note = (Note*)note_list->data;
		gtk_widget_modify_font(GTK_WIDGET(note->view), font);
		note_list = g_list_next(note_list);
	}
}

void
on_bigger_button_clicked				(GtkButton		*button,
										gpointer		 user_data)
{
	/* TODO: This is copy&paste from on_smaller_button_clicked. */
	AppData *app_data = get_app_data();
	PangoFontDescription *font;
	Note *note;
	GList *note_list;
	
	app_data->font_size += 5000;
	gconf_client_set_int(app_data->client, "/apps/maemo/conboy/font_size", app_data->font_size, NULL);
	
	font = pango_font_description_new();
	pango_font_description_set_size(font, app_data->font_size);
	
	note_list = g_list_first(app_data->open_notes);
	
	while (note_list != NULL) {
		note = (Note*)note_list->data;
		gtk_widget_modify_font(GTK_WIDGET(note->view), font);
		note_list = g_list_next(note_list);
	}
}

gboolean on_hardware_key_pressed	(GtkWidget			*widget,
									 GdkEventKey		*event,
									 gpointer           *user_data)
{
	Note *note = (Note*)user_data;
	
	switch (event->keyval) {
	case HILDON_HARDKEY_INCREASE:
		hildon_banner_show_information(GTK_WIDGET(note->window), NULL, "Zooming in");
		on_bigger_button_clicked(NULL, NULL);
		return TRUE;
	
	case HILDON_HARDKEY_DECREASE:
		hildon_banner_show_information(GTK_WIDGET(note->window), NULL, "Zooming out");
		on_smaller_button_clicked(NULL, NULL);
		return TRUE;
		
	case HILDON_HARDKEY_ESC:
		on_window_close_button_clicked(GTK_OBJECT(note->window), NULL, note);
		return TRUE;
	}
	
	return FALSE;
}


