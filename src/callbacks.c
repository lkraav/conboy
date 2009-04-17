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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-banner.h>
#include <libhildondesktop/hildon-thumb-menu-item.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>


#include "callbacks.h"
#include "support.h"

#include "metadata.h"
#include "interface.h"

#include "note.h"
#include "serializer.h"
#include "deserializer.h"

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

void on_quit_button_clicked(GtkButton *button, gpointer user_data)
{	
	Note *note;
	GList *open_notes = get_app_data()->open_notes;
	
	while (open_notes != NULL) {
		note = (Note*)open_notes->data;
		note_save(note);
		note_close_window(note);
		open_notes = g_list_next(open_notes);
	}
	
	gtk_main_quit();
}

void on_load_button_clicked(GtkButton *button, gpointer user_data)
{
	Note *note = (Note*)user_data;
		
	/* TODO: Probably a GError object would be good. E.g. if File does not exist etc... */
	/*deserialize_note(note);*/ 
	/*note_show(note);*/
}

void on_save_button_clicked(GtkButton *button, gpointer user_data) {
	Note *note = (Note*)user_data;
	
	/*serialize_note(note);*/
	
}

void
on_bold_button_clicked				   (GtkButton		*button,
										gpointer		 user_data)
{
	change_format("bold", button);
}

void
on_italic_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	change_format("italic", button);
}

void
on_strike_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	change_format("strikethrough", button);
}

void
on_highlight_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	change_format("highlight", button);
}

static void
add_bullets(GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	gint i = 0;
	gint start_line = gtk_text_iter_get_line(start_iter);
	gint end_line   = gtk_text_iter_get_line(end_iter);
	gint total_lines = gtk_text_buffer_get_line_count(buffer);
	gchar *list_item[2] = {"list-item-A:1", "list-item-B:1"}; /* TODO: Replace with get_depth_tag() from deserialzer2.c
	
	/* For each selected line */
	for (i = start_line; i <= end_line; i++) {
		
		/* li decideds if we take list-item-A or list-item-B */
		int li = i % 2;
		
		/* Insert bullet character */
		gtk_text_buffer_get_iter_at_line(buffer, start_iter, i);
		gtk_text_buffer_insert(buffer, start_iter, BULLET, -1);
		
		/* Surround line with "list-item" tags */
		gtk_text_buffer_get_iter_at_line(buffer, start_iter, i);
		if (i == end_line) {
			gtk_text_buffer_get_iter_at_line(buffer, end_iter, i);
			gtk_text_iter_forward_to_line_end(end_iter);
		} else {
			gtk_text_buffer_get_iter_at_line(buffer, end_iter, i + 1);
		}
		gtk_text_buffer_apply_tag_by_name(buffer, list_item[li], start_iter, end_iter);
	}
	
	/* Surround it with "list" tags */
	gtk_text_buffer_get_iter_at_line(buffer, start_iter, start_line);
	gtk_text_buffer_get_iter_at_line(buffer, end_iter, end_line);
	gtk_text_iter_forward_to_line_end(end_iter);
	gtk_text_buffer_apply_tag_by_name(buffer, "list", start_iter, end_iter);
}

static void
remove_bullets(GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	gint i = 0;
	gint start_line = gtk_text_iter_get_line(start_iter);
	gint end_line = gtk_text_iter_get_line(end_iter);
	
	/* Remove tags */
	gtk_text_buffer_get_iter_at_line(buffer, start_iter, start_line);
	gtk_text_buffer_get_iter_at_line(buffer, end_iter, end_line);
	gtk_text_iter_forward_to_line_end(end_iter);
	
	gtk_text_buffer_remove_tag_by_name(buffer, "list-item-A", start_iter, end_iter);
	gtk_text_buffer_remove_tag_by_name(buffer, "list-item-B", start_iter, end_iter);
	gtk_text_buffer_remove_tag_by_name(buffer, "list", start_iter, end_iter);
		
	/* Remove bullets */
	for (i = start_line; i <= end_line; i++) {
		gtk_text_buffer_get_iter_at_line(buffer, start_iter, i);
		gtk_text_buffer_get_iter_at_line(buffer, end_iter, i);
		gtk_text_iter_forward_chars(end_iter, 2);
		gtk_text_buffer_delete(buffer, start_iter, end_iter);
	}
}

void
on_bullets_button_clicked				(GtkButton		*button,
										 gpointer		 user_data)
{
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
			add_bullets(buffer, &start_iter, &end_iter);
			
		} else {
			/* Nothing is selected */
			gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_get_iter_at_mark(buffer, &end_iter, gtk_text_buffer_get_insert(buffer));
			add_bullets(buffer, &start_iter, &end_iter);
		}
	} else {
		/* The button just became deactive, so we should remove the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			remove_bullets(buffer, &start_iter, &end_iter);
			
		} else {
			/* Nothing is selected */
			gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_get_iter_at_mark(buffer, &end_iter, gtk_text_buffer_get_insert(buffer));		
			remove_bullets(buffer, &start_iter, &end_iter);
		}
	}
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
	
	note_show_by_title(text);
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
	
	AppData *app_data = get_app_data();
	
	GtkWidget *menu = gtk_menu_new ();
	GList *notes = app_data->all_notes;
	
	while(notes != NULL) {
		GtkWidget *menu_item;
		GtkWidget *image_small, *image_large;
		Note *note = notes->data;
		/* TODO: Don't hardcode the path to the icons*/
		/* TODO: When starting from Eclipse, use local paths, not from /usr/share/ */
		menu_item = hildon_thumb_menu_item_new_with_labels(note->title, note->title, "Open Note...");
		image_small = gtk_image_new_from_file("/usr/share/icons/hicolor/26x26/hildon/conboy.png");
		image_large = gtk_image_new_from_file("/usr/share/icons/hicolor/40x40/hildon/conboy.png");
		hildon_thumb_menu_item_set_images(HILDON_THUMB_MENU_ITEM(menu_item), image_small, image_large);
		
		g_signal_connect(G_OBJECT(menu_item), "activate",
		            	 G_CALLBACK(on_notes_menu_item_activated),
		            	 note);
		gtk_menu_append(menu, menu_item);
		notes = notes->next;
	}
	
	hildon_menu_set_thumb_mode(GTK_MENU(menu), TRUE);
	
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
	note_show(note);
}


/* TODO: The signal "mark-set" is emitted 4 times when clicking into the text. While selecting
 * it's emitted continuesly. */
void
on_textview_cursor_moved			   (GtkTextBuffer	*buffer,
										GtkTextIter		*location,
										GtkTextMark		*mark,
										gpointer		 user_data)
{
	GtkToggleToolButton *bold_button, *italic_button, *strike_button, *highlight_button, *bullets_button;
	GSList *tags;
	GtkTextTag *tag;
	
	bold_button      = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "bold_button"));
	italic_button    = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "italic_button"));
	strike_button    = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "strike_button"));
	highlight_button = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "highlight_button"));
	bullets_button   = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(user_data), "bullets_button"));
	
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
	g_signal_handlers_block_by_func(highlight_button, on_highlight_button_clicked, NULL);
	g_signal_handlers_block_by_func(bullets_button, on_bullets_button_clicked, NULL);
	
	gtk_toggle_tool_button_set_active(bold_button, FALSE);
	gtk_toggle_tool_button_set_active(italic_button, FALSE);
	gtk_toggle_tool_button_set_active(strike_button, FALSE);
	gtk_toggle_tool_button_set_active(highlight_button, FALSE);
	gtk_toggle_tool_button_set_active(bullets_button, FALSE);
	
	while (tags != NULL) {
		tag = GTK_TEXT_TAG(tags->data);
		if (g_strcasecmp(tag->name, "bold") == 0) {
			gtk_toggle_tool_button_set_active(bold_button, TRUE);
		} else if (g_strcasecmp(tag->name, "italic") == 0) {
			gtk_toggle_tool_button_set_active(italic_button, TRUE);
		} else if (g_strcasecmp(tag->name, "strikethrough") == 0) {
			gtk_toggle_tool_button_set_active(strike_button, TRUE);
		} else if (g_strcasecmp(tag->name, "highlight") == 0) {
			gtk_toggle_tool_button_set_active(highlight_button, TRUE);
		} else if (g_strcasecmp(tag->name, "list-item-A") == 0) {
			gtk_toggle_tool_button_set_active(bullets_button, TRUE);
		} else if (g_strcasecmp(tag->name, "list-item-B") == 0) {
			gtk_toggle_tool_button_set_active(bullets_button, TRUE);
		}
		tags = tags->next;
	}
	
	g_slist_free(tags);
	
	/* unblock signals */
	g_signal_handlers_unblock_by_func(bold_button, on_bold_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(italic_button, on_italic_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(strike_button, on_strike_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(highlight_button, on_highlight_button_clicked, NULL);
	g_signal_handlers_unblock_by_func(bullets_button, on_bullets_button_clicked, NULL);
	
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
			note_show_by_title(link_text);
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
	
	if (app_data->font_size > 5000) {
		app_data->font_size -= 5000;
	} else {
		return;
	}
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
	
	if (app_data->font_size < 65000) {
		app_data->font_size += 5000;
	} else {
		return;
	}
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
									 gpointer           user_data)
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
	
	case HILDON_HARDKEY_FULLSCREEN:
		/* TODO: Implement full screen. */
		/* Should this be on application level: If fullscreen, then all windows fullscreen
		 * Or on window leve: Toggle fullscreen on/off for every window
		 * Or can only be the window which is active be fullscreen? */
		/*
		gtk_window_fullscreen(note->window);
		gtk_window_unfullscreen(note->window);
		*/
		return TRUE;
	}
	
	return FALSE;
}

void
on_textview_tap_and_hold(GtkWidget *widget, gpointer user_data)
{
	/* We emit the popup-menu signal, this will make the
	 * default context menu of the treeview pop up. */
	gboolean bool;
	g_signal_emit_by_name(widget, "popup-menu", &bool);
}



