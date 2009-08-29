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
#include "storage.h"
#include "note_buffer.h"

#define _(String)gettext(String)

void note_free(Note *note)
{
	g_free(note->content);
	g_free(note->title);
	g_free(note->guid);
	g_slist_free(note->active_tags);
	g_free(note);
	note = NULL;
}

void note_show_by_title(const char* title)
{
	ConboyNote *note;
	AppData *app_data = app_data_get();

	note = conboy_note_store_find_by_title(app_data->note_store, title);

	if (note == NULL) {
		note = conboy_note_new();
		g_object_set(note, "title", title, NULL);
		note_show(note);
		return;
	}

	note_show(note);
}

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
	const gchar* title;
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

	/* Set meta data */
	/* We don't change height, width, x and y because we don't use them */
	g_object_set(note,
			"title", title,
			"change-date", time_in_s,
			"metadata-change-date", time_in_s,
			"cursor-position", cursor_position,
			"open-on-startup", FALSE,
			NULL);

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
	g_object_set(note, "content", note_buffer_get_xml(buffer), NULL);

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


void note_close_window(UserInterface *ui)
{
	HildonProgram *program = hildon_program_get_instance();
	HildonWindow *window = ui->window;
	AppData *app_data = app_data_get();
	guint count = g_list_length(app_data->open_notes);
	GList  *listeners = ui->listeners;
	
	if (count > 1) {
		hildon_program_remove_window(program, window);
		
		/* Remove the listeners */
		while (listeners != NULL) {
			gconf_client_notify_remove(app_data->client, GPOINTER_TO_INT(listeners->data));
			listeners = listeners->next;
		}
		
		gtk_widget_destroy(GTK_WIDGET(window));
		g_list_free(ui->listeners);
		g_free(ui);
		ui = g_new0(UserInterface, 1);
		app_data->open_windows = g_list_remove(app_data->open_windows, ui);
		/* Don't free note, because we reuse this in the menu with the available notes and when reopening */
	} else {
		g_printerr("####################### QUIT ###################\n");
		gtk_main_quit();
	}
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

}


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

void note_show(ConboyNote *note)
{
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer;
	GtkWindow *window;

	/* If the note it already open, we bring this note to the foreground and return */
	/*
	if (note_is_open(note)) {
		note_set_focus(note);
		return;
	}
	*/
	
	

	if (app_data->note_window == NULL) {
		g_printerr("##### Creating Mainwin\n");
		app_data->note_window = create_mainwin(note);
		
		/* Set window width/height, otherwise scroll to cursor doesn't work correctly */
		gtk_window_set_default_size(GTK_WINDOW(app_data->note_window->window), 720, 420);

		/* Set to fullscreen or not */
		if (app_data->fullscreen) {
			gtk_window_fullscreen(GTK_WINDOW(app_data->note_window->window));
		} else {
			gtk_window_unfullscreen(GTK_WINDOW(app_data->note_window->window));
		}
	}
	
	UserInterface *ui = app_data->note_window;
	buffer = ui->buffer;
	window = GTK_WINDOW(ui->window);
	
	if (!GTK_IS_TEXT_BUFFER(buffer)) {
		g_printerr("NO BUFFER\n");
	}
	
	if (!GTK_IS_WINDOW(window)) {
		g_printerr("NO WINDOW\n");
	}
	

	hildon_program_add_window(app_data->program, HILDON_WINDOW(window));

	/* Block signals on TextBuffer until we are done with initializing the content. This is to prevent saves etc. */
	g_signal_handlers_block_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);

	/*
	if (note_exists(note)) {
		note_show_existing(note);
	} else {
		note_show_new(note);
	}
	*/

	app_data->open_notes = g_list_append(app_data->open_notes, note);
	app_data->open_windows = g_list_append(app_data->open_notes, note);

	/* Format note title and update window title */
	note_format_title(buffer);
	note_set_window_title_from_buffer(window, buffer); /* Replace this. And use note->title instead */

	gtk_widget_show(GTK_WIDGET(window));

	gtk_text_buffer_set_modified(buffer, FALSE);

	/* unblock signals */
	g_signal_handlers_unblock_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
}

/*
void note_show_new(Note *note)
{
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter start, end;
	const gchar *content;
	gint notes_count;

	note->guid = get_uuid();

	if (note->title == NULL) {
		notes_count = conboy_note_store_get_length(app_data->note_store);
		note->title = g_strdup_printf(_("New Note %i"), notes_count);
	}

	content = g_strconcat(note->title, "\n\n", _("Describe your new note here."), NULL);
	gtk_text_buffer_set_text(buffer, content, -1);

	gtk_text_buffer_get_iter_at_line(buffer, &start, 2);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_select_range(buffer, &start, &end);
}
*/

/*
void note_show_existing(ConboyNote *note)
{
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buffer, &iter);

	note_buffer_set_xml(buffer, note->content);

	gtk_text_buffer_get_iter_at_offset(buffer, &iter, note->cursor_position);
	gtk_text_buffer_place_cursor(buffer, &iter);

	gtk_text_view_scroll_to_mark(note->ui->view, gtk_text_buffer_get_insert(buffer), 0.0, TRUE, 0.0, 0.5);
}
*/

void note_add_active_tag_by_name(UserInterface *ui, const gchar *tag_name)
{
	note_add_active_tag(ui, gtk_text_tag_table_lookup(ui->buffer->tag_table, tag_name));
}

void note_remove_active_tag_by_name(UserInterface *ui, const gchar *tag_name)
{
	note_remove_active_tag(ui, gtk_text_tag_table_lookup(ui->buffer->tag_table, tag_name));
}

/**
 * Add a tag to the list of active_tags, if it is not yet in the list.
 */
void note_add_active_tag(UserInterface *ui, GtkTextTag *tag)
{
	GSList *tags;

	if (ui == NULL) {
		g_printerr("ERROR: note_add_active_tag: note is NULL\n");
		return;
	}
	if (tag == NULL) {
		g_printerr("ERROR: note_add_active_tag: tag is NULL\n");
		return;
	}

	tags = ui->active_tags;
	while (tags != NULL) {
		if (strcmp(((GtkTextTag*)tags->data)->name, tag->name) == 0) {
			return;
		}
		tags = tags->next;
	}
	ui->active_tags = g_slist_prepend(ui->active_tags, tag);
}

/**
 * Removes a tag from the list of active_tags if it is in this list.
 */
void note_remove_active_tag(UserInterface *ui, GtkTextTag *tag)
{
	GSList *tags = ui->active_tags;
	while (tags != NULL) {
		if (strcmp(GTK_TEXT_TAG(tags->data)->name, tag->name) == 0) {
			ui->active_tags = g_slist_remove(ui->active_tags, tags->data);
			return;
		}
		tags = tags->next;
	}
}

/**
 * Adds a tag (e.g. notebook) to a note
 */
void note_add_tag(Note *note, gchar *tag)
{
	note->tags = g_list_prepend(note->tags, tag);
}


