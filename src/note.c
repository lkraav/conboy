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
#include "note_list_store.h"
#include "storage.h"
#include "note_buffer.h"

#define _(String)gettext(String)

Note* note_create_new()
{
	Note *note = g_new0(Note, 1);
	UserInterface *ui = g_new0(UserInterface, 1);
	note->ui = ui;
	note->active_tags = NULL;
	note->tags = NULL;
	note->guid = get_uuid();
	note->create_date = 0;
	note->last_change_date = 0;
	note->last_metadata_change_date = 0;
	note->cursor_position = 0;
	return note;
}

void note_free(Note *note)
{
	g_free(note->ui);
	g_free(note->content);
	g_free(note->title);
	g_free(note->guid);
	g_slist_free(note->active_tags);
	g_free(note);
	note = NULL;
}

void note_show_by_title(const char* title)
{
	Note *note;
	AppData *app_data = app_data_get();

	note = note_list_store_find_by_title(app_data->note_store, title);

	if (note == NULL) {
		note = note_create_new();
		note->title = g_strdup(title);
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


void note_save(Note *note)
{
	time_t time_in_s;
	const gchar* title;
	gchar* content;
	GtkTextIter iter, start, end;
	GtkTextMark *mark;
	gint cursor_position;
	AppData *app_data;
	GtkTextBuffer *buffer = note->ui->buffer;

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
	note->title = g_strdup(title);
	note->last_change_date = time_in_s;
	note->last_metadata_change_date = time_in_s;
	note->cursor_position = cursor_position;
	note->open_on_startup = FALSE;

	if (note->create_date == 0) {
		note->create_date = time_in_s;
	}
	if (note->height == 0) {
		note->height = 300;
	}
	if (note->width == 0) {
		note->width = 600;
	}
	if (note->x == 0) {
		note->x = 1;
	}
	if (note->y == 0) {
		note->y = 1;
	}
	
	/* Clear note content, then set it with fresh data from the text buffer */
	g_free(note->content);
	note->content = note_buffer_get_xml(buffer);

	/* Save the complete note */
	storage_save_note(note);

	app_data = app_data_get();	
	/* If first save, add to list of all notes */
	if (!note_list_store_find(app_data->note_store, note)) {
		note_list_store_add(app_data->note_store, note, NULL);
	} else {
		note_list_store_note_changed(app_data->note_store, note);
	}

	gtk_text_buffer_set_modified(buffer, FALSE);
}


void note_close_window(Note *note)
{
	HildonProgram *program = hildon_program_get_instance();
	HildonWindow *window = note->ui->window;
	AppData *app_data = app_data_get();
	guint count = g_list_length(app_data->open_notes);

	if (count > 1) {
		hildon_program_remove_window(program, window);
		gtk_widget_destroy(GTK_WIDGET(window));
		note->ui = NULL;
		app_data->open_notes = g_list_remove(app_data->open_notes, note);
		/* Don't free note, because we reuse this in the menu with the available notes and when reopening */
	} else {
		g_printerr("####################### QUIT ###################\n");
		gtk_main_quit();
	}
}


void note_delete(Note *note)
{
	AppData *app_data = app_data_get();

	/* Delete file */
	if (!storage_delete_note(note->guid)) {
		g_printerr("ERROR: The note with the guid %s could not be deleted \n", note->guid);
	}
		
	/*
	if (g_unlink(note->filename) == -1) {
		g_printerr("ERROR: The file %s could not be deleted \n", note->filename);
	}
	*/

	/* Remove from list store */
	note_list_store_remove(app_data->note_store, note);

	/* Free memory */
	/*note_free(note);*/
}


gboolean note_is_open(Note *note)
{
	AppData *app_data = app_data_get();
	GList *element = g_list_find(app_data->open_notes, note);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/** TODO: This should be a function of NoteListStore.
 *  note_list_store_containts(note)
 */
gboolean note_exists(Note *note)
{
	AppData *app_data = app_data_get();
	Note *element = note_list_store_find(app_data->note_store, note);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}

void note_set_focus(Note *note)
{
	if (note->ui->window != NULL) {
		gtk_window_present(GTK_WINDOW(note->ui->window));
	} else {
		g_printerr("ERROR: Trying to focus a window that does not exist.\n");
	}
}

void note_show(Note *note)
{
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer;
	GtkWindow *window;

	/* If the note it already open, we bring this note to the foreground and return */
	if (note_is_open(note)) {
		note_set_focus(note);
		return;
	}

	create_mainwin(note);

	buffer = note->ui->buffer;
	window = GTK_WINDOW(note->ui->window);

	/* Set window width/height, otherwise scroll to cursor doesn't work correctly */
	gtk_window_set_default_size(window, 720, 420);

	/* Set to fullscreen or not */
	if (app_data->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(note->ui->window));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(note->ui->window));
	}

	hildon_program_add_window(app_data->program, HILDON_WINDOW(window));

	/* Block signals on TextBuffer until we are done with initializing the content. This is to prevent saves etc. */
	g_signal_handlers_block_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);

	if (note_exists(note)) {
		note_show_existing(note);
	} else {
		note_show_new(note);
	}

	app_data->open_notes = g_list_append(app_data->open_notes, note);

	/* Format note title and update window title */
	note_format_title(buffer);
	note_set_window_title_from_buffer(window, buffer); /* Replace this. And use note->title instead */

	gtk_widget_show(GTK_WIDGET(window));

	gtk_text_buffer_set_modified(buffer, FALSE);

	/* unblock signals */
	g_signal_handlers_unblock_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
}

void note_show_new(Note *note)
{
	AppData *app_data = app_data_get();
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter start, end;
	const gchar *content;
	gint notes_count;

	/* TODO: Creating a new note should happen in note_create_new(). Not here. */
	/* TODO: Only save guid in note and generate filename on-the-fly when needed. */
	note->guid = get_uuid();
	/*note->filename = (gchar*)note_get_new_filename(note->guid);*/

	if (note->title == NULL) {
		notes_count = note_list_store_get_length(app_data->note_store);
		note->title = g_strdup_printf(_("New Note %i"), notes_count);
	}

	content = g_strconcat(note->title, "\n\n", _("Describe your new note here."), NULL);
	gtk_text_buffer_set_text(buffer, content, -1);

	/* Select the text */
	gtk_text_buffer_get_iter_at_line(buffer, &start, 2);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_select_range(buffer, &start, &end);
}

void note_show_existing(Note *note)
{
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter iter;

	/* iter defines where to start with inserting */
	gtk_text_buffer_get_start_iter(buffer, &iter);

	/* Insert the content of a note into the text buffer */
	note_buffer_set_xml(buffer, note->content);

	/* Set cursor possition */
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, note->cursor_position);
	gtk_text_buffer_place_cursor(buffer, &iter);

	/* Scroll to cursor position */
	gtk_text_view_scroll_to_mark(note->ui->view, gtk_text_buffer_get_insert(buffer), 0.0, TRUE, 0.0, 0.5);
}

void note_add_active_tag_by_name(Note *note, const gchar *tag_name)
{
	note_add_active_tag(note, gtk_text_tag_table_lookup(note->ui->buffer->tag_table, tag_name));
}

void note_remove_active_tag_by_name(Note *note, const gchar *tag_name)
{
	note_remove_active_tag(note, gtk_text_tag_table_lookup(note->ui->buffer->tag_table, tag_name));
}

/**
 * Add a tag to the list of active_tags, if it is not yet in the list.
 */
void note_add_active_tag(Note *note, GtkTextTag *tag)
{
	GSList *tags;

	if (note == NULL) {
		g_printerr("ERROR: note_add_active_tag: note is NULL\n");
		return;
	}
	if (tag == NULL) {
		g_printerr("ERROR: note_add_active_tag: tag is NULL\n");
		return;
	}

	tags = note->active_tags;
	while (tags != NULL) {
		if (strcmp(((GtkTextTag*)tags->data)->name, tag->name) == 0) {
			return;
		}
		tags = tags->next;
	}
	note->active_tags = g_slist_prepend(note->active_tags, tag);
}

/**
 * Removes a tag from the list of active_tags if it is in this list.
 */
void note_remove_active_tag(Note *note, GtkTextTag *tag)
{
	GSList *tags = note->active_tags;
	while (tags != NULL) {
		if (strcmp(GTK_TEXT_TAG(tags->data)->name, tag->name) == 0) {
			note->active_tags = g_slist_remove(note->active_tags, tags->data);
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


