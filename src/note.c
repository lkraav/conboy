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
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <libintl.h>

#include "app_data.h"
#include "metadata.h"
#include "interface.h"
#include "note.h"
#include "conboy_note_store.h"
#include "conboy_note_buffer.h"

#define _(String)gettext(String)


void note_show_by_title(const char* title)
{
	ConboyNote *note;
	AppData *app_data = app_data_get();

	note = conboy_note_store_find_by_title(app_data->note_store, title);

	if (note == NULL) {
		note = conboy_note_new_with_title(title);
	}

	note_show(note, TRUE, TRUE);
}

/* TODO: Move to ConboyNoteBuffer */
void note_format_title(GtkTextBuffer *buffer)
{
	GtkTextIter start, end;

	/* Set end iter depending on if we have one or more lines */
	if (gtk_text_buffer_get_line_count(buffer) == 1) {
		gtk_text_buffer_get_end_iter(buffer, &end);
	} else {
		gtk_text_buffer_get_iter_at_line(buffer, &end, 1);
		gtk_text_iter_backward_char(&end);
	}
	/* Set start iter, remove all tags and add _title tags */
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_remove_all_tags(buffer, &start, &end);
	gtk_text_buffer_apply_tag_by_name(buffer, "_title", &start, &end);
}

/* TODO: Move to ConboyNoteBuffer
 * The return value needs to be freed by the caller. 
 */
gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer)
{
	GtkTextIter start, end;
	gchar* title;

	/* Set end iter depending on if we have one or more lines */
	if (gtk_text_buffer_get_line_count(buffer) == 1) {
		gtk_text_buffer_get_end_iter(buffer, &end);
	} else {
		gtk_text_buffer_get_iter_at_line(buffer, &end, 1);
		gtk_text_iter_backward_char(&end);
	}

	gtk_text_buffer_get_start_iter(buffer, &start);
	title = gtk_text_iter_get_text(&start, &end);
	return title;
}


void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer)
{
	gchar *title = note_extract_title_from_buffer(buffer);
	gtk_window_set_title(win, title);
	g_free(title);
}


static
gboolean is_empty_str(const gchar* str)
{
	gchar *tmp;

	if (str == NULL) {
		return TRUE;
	}

	tmp = g_strdup(str);
	g_strstrip(tmp);

	if (strcmp(tmp, "") == 0) {
		return TRUE;
	} else {
		return FALSE;
	}

	g_free(tmp);
}


void note_save(UserInterface *ui)
{
	time_t time_in_s;
	gchar* title;
	gchar* content;
	GtkTextIter iter, start, end;
	GtkTextMark *mark;
	gint cursor_position;
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer = ui->buffer;
	ConboyNote *note = ui->note;

	/* If note is empty, don't save */
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	content = gtk_text_iter_get_text(&start, &end);
	if (is_empty_str(content)) {
		gtk_text_buffer_set_modified(buffer, FALSE);
		g_free(content);
		return;
	}
	g_free(content);

	/* If buffer is not dirty, don't save */
	if (!gtk_text_buffer_get_modified(buffer)) {
		return;
	}

	/* Get time */
	time_in_s = time(NULL);

	/* Get cursor position */
	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
	cursor_position = gtk_text_iter_get_offset(&iter);

	/* Get title */
	title = note_extract_title_from_buffer(buffer);
	
	/* Check for duplicated title */
	ConboyNote *existing_note = conboy_note_store_find_by_title(app_data->note_store, title);
	if (existing_note && (existing_note != note)) {
		/* Save alternative title if title is already taken */
		int num = conboy_note_store_get_length(app_data->note_store) + 1;
		gchar new_title[100];
		g_sprintf(new_title, _("New Note %i"), num);
		g_free(title);
		title = g_strdup(new_title);
	}

	/* Set meta data */
	/* We don't change height, width, x and y because we don't use them */
	g_object_set(note,
			"title", title,
			"change-date", time_in_s,
			"metadata-change-date", time_in_s,
			"cursor-position", cursor_position,
			"open-on-startup", FALSE,
			NULL);

	g_free(title);
	
	if (note->create_date == 0) {
		g_object_set(note, "create-date", time_in_s, NULL);
	}
	if (note->height == 0) {
		g_object_set(note, "height", 300, NULL);
	}
	if (note->width == 0) {
		g_object_set(note, "width", 600, NULL);
	}
	if (note->x == 0) {
		g_object_set(note, "x", 1, NULL);
	}
	if (note->y == 0) {
		g_object_set(note, "y", 1, NULL);
	}

	/* Clear note content, then set it with fresh data from the text buffer */
	g_object_set(note, "content", conboy_note_buffer_get_xml(CONBOY_NOTE_BUFFER(buffer)), NULL);

	/* Save the complete note */
	conboy_storage_note_save(app_data->storage, note);

	/* If first save, add to list of all notes */
	if (!conboy_note_store_find(app_data->note_store, note)) {
		conboy_note_store_add(app_data->note_store, note, NULL);
	} else {
		conboy_note_store_note_changed(app_data->note_store, note);
	}

	gtk_text_buffer_set_modified(buffer, FALSE);
}


void note_delete(ConboyNote *note)
{
	AppData *app_data = app_data_get();

	/* Delete file */
	if (!conboy_storage_note_delete(app_data->storage, note)) {
		g_printerr("ERROR: The note with the guid %s could not be deleted \n", note->guid);
	}

	/* Remove from list store */
	conboy_note_store_remove(app_data->note_store, note);
	
	/* Remove from history */
	if (app_data->current_element->prev) {
		app_data->current_element = app_data->current_element->prev;
	} else {
		app_data->current_element = app_data->current_element->next;
	}
	app_data->note_history = g_list_remove_all(app_data->note_history, note);
}

/*
gboolean note_is_open(UserInterface *ui)
{
	AppData *app_data = app_data_get();
	GList *element = g_list_find(app_data->open_windows, ui);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}
*/

/**
 * TODO: This should be a function of NoteListStore.
 *  note_list_store_containts(note)
 */
gboolean note_exists(ConboyNote *note)
{
	AppData *app_data = app_data_get();
	ConboyNote *element = conboy_note_store_find(app_data->note_store, note);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}

void note_set_focus(UserInterface *ui)
{
	if (ui->window != NULL) {
		gtk_window_present(GTK_WINDOW(ui->window));
	} else {
		g_printerr("ERROR: Trying to focus a window that does not exist.\n");
	}
}

static void
add_to_history(ConboyNote *note)
{
	AppData *app_data = app_data_get();
	
	/* Consistency check */
	if (app_data->current_element == NULL) {
		g_assert(app_data->note_history == NULL);
	}
	
	/* Consistency check */
	if (app_data->note_history == NULL) {
		g_assert(app_data->current_element == NULL);
	}

	/* If we are currently not at the end of the history, we need to remove the 'future' before appending */
	/* Remove everything from the current position to the end */
	if (app_data->current_element != g_list_last(app_data->note_history)) {
		
		gint pos = g_list_position(app_data->note_history, app_data->current_element);
		gint len = g_list_length(app_data->note_history);
		
		GList *to_remove = NULL;
		for (; pos < len; pos++) {
			GList *element = g_list_nth(app_data->note_history, pos);
			to_remove = g_list_prepend(to_remove, element);
		}
		
		GList *iter = to_remove;
		while (iter) {
			GList *element = (GList*) iter->data;
			app_data->note_history = g_list_remove_link(app_data->note_history, element);
			g_list_free(element);
			iter = iter->next;
		}
		g_list_free(to_remove);
	}
	
	if (g_list_length(app_data->note_history) >= 20) {
		g_printerr("INFO: History is too long. Removing oldes element. \n");
		app_data->note_history = g_list_delete_link(app_data->note_history, app_data->note_history);
	}
	
	app_data->note_history = g_list_append(app_data->note_history, note);
	app_data->current_element = g_list_last(app_data->note_history);
}

void note_show(ConboyNote *note, gboolean modify_history, gboolean scroll)
{
	AppData *app_data = app_data_get();
	
	UserInterface *ui = app_data->note_window;
	GtkTextBuffer *buffer = ui->buffer;
	GtkWindow *window = GTK_WINDOW(ui->window);

	/* Before we switch to a new note, we save the last one if it was modified. */
	if (gtk_text_buffer_get_modified(buffer)) {
		note_save(ui);
	}

	/* Add to history */
	if (modify_history) {
		add_to_history(note);
	}
	
	/* Toggle forward/backward buttons */
	gtk_action_set_sensitive(ui->action_back, (gboolean) app_data->current_element->prev);
	gtk_action_set_sensitive(ui->action_forward, (gboolean) app_data->current_element->next);
	
	/* Block signals on TextBuffer until we are done with initializing the content. This is to prevent saves etc. */
	g_signal_handlers_block_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);

	conboy_note_window_show_note(ui, note);

	/* Format note title and update window title */
	note_format_title(buffer);
	note_set_window_title_from_buffer(window, buffer); /* Replace this. And use note->title instead */

	/* Show widget and set focus to the text view */
	gtk_widget_show(GTK_WIDGET(window));
	gtk_widget_grab_focus(GTK_WIDGET(ui->view));
	
	/* Scroll to cursor position */
	if (scroll) {
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, note->cursor_position);
	gtk_text_buffer_place_cursor(buffer, &iter);
	GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_view_scroll_to_mark(ui->view, mark, 0.1, TRUE, 0, 0.5);
	}

	/* Set the buffer to unmodified */
	gtk_text_buffer_set_modified(buffer, FALSE);

	/* unblock signals */
	g_signal_handlers_unblock_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);
}

