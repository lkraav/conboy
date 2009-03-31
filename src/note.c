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

#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

#include "support.h"

#include "metadata.h"
#include "interface.h"
#include "note.h"

/* TODO:
 * 
 * - note_format_title() and note_set_window_title_from_buffer() have a lot
 * of shared code. Also they are often called together. So maybe write a
 * function which has both functionalities.
 * 
 * 
 */

/* Not needed right now. Maybe later again */
static gint compare_title(gconstpointer a, gconstpointer b)
{	
	const gchar *title1 = ((Note*)a)->title;
	const gchar *title2 = b;
	
	if (title1 == NULL && title2 == NULL) {
		return 0;
	}
	if (title1 == NULL) {
		return -1;
	}
	if (title2 == NULL) {
		return 1;
	}
	return strcmp(title1, title2);
}


void note_show_by_title(const char* title)
{	
	Note *note;
	GList *element;
	AppData *app_data = get_app_data();
	GList *all_notes = app_data->all_notes;
	
	element = g_list_find_custom(all_notes, title, compare_title);
	
	if (element == NULL) {
		note = g_malloc0(sizeof(Note));
		note->title = title;
		note_show(note);
		return;
	}
	
	note = (Note*)element->data;
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


const gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer)
{	
	GtkTextIter start, end;
	const gchar* title;
	
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
	gtk_window_set_title(win, note_extract_title_from_buffer(buffer));
}


static
gboolean is_empty_str(const gchar* str)
{
	/* TODO: A string only containing whitespaces should be handled as empty too */
	if (strcmp(str, "") == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}


void note_save(Note *note)
{		
	const gchar* time;
	const gchar* title;
	const gchar* content;
	GtkTextIter iter, start, end;
	GtkTextMark *mark;
	gint cursor_position;
	guint8 *data;
	gsize length;
	AppData *app_data;
	FILE *file;
	
	/* If note is empty, don't save */
	gtk_text_buffer_get_bounds(note->buffer, &start, &end);
	content = gtk_text_iter_get_text(&start, &end);
	if (is_empty_str(content)) {
		return;
	}
	
	/* If buffer is not dirty, don't save */
	if (!gtk_text_buffer_get_modified(note->buffer)) {
		return;
	}
	
	/* Get time */
	time = get_current_time_in_iso8601();
	
	/* Get cursor position */
	mark = gtk_text_buffer_get_insert(note->buffer);
	gtk_text_buffer_get_iter_at_mark(note->buffer, &iter, mark);
	cursor_position = gtk_text_iter_get_offset(&iter);

	/* Get title */
	title = note_extract_title_from_buffer(note->buffer); 
	
	/* Set meta data */
	/* We don't change height, width, x and y and we don't need them */
	note->title = g_strdup(title);
	note->last_change_date = g_strdup(time);
	note->last_metadata_change_date = g_strdup(time);
	note->cursor_position = cursor_position;
	note->open_on_startup = FALSE;
	
	if (note->create_date == NULL) {
		note->create_date = g_strdup(time);
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
	
	app_data = get_app_data();
	
	g_printerr("Saving >%s< \n", note->title);
	
	/* Set start and end iterators for serialization */
	gtk_text_buffer_get_bounds(note->buffer, &start, &end);
	
	/* Start serialization */
	data = gtk_text_buffer_serialize(note->buffer, note->buffer,
			app_data->serializer, &start, &end, &length);
	
	/* Write to disk */
	file = fopen(note->filename, "wb");
	fwrite(data, sizeof(guint8), length, file);
	fclose(file);
	
	/* If first save, add to list of all notes */
	if (!g_list_find(app_data->all_notes, note)) {
		g_printerr("Note >%s< is not yet in the list. Adding.\n", note->title);
		app_data->all_notes = g_list_append(app_data->all_notes, note);
	}
	
	gtk_text_buffer_set_modified(note->buffer, FALSE);
}

void note_free(Note *note) {
	note->buffer = NULL;
	g_free(note->create_date);
	g_free(note->filename);
	/* TODO: Free the rest */
}

void note_close_window(Note *note)
{
	HildonProgram *program = hildon_program_get_instance();	
	AppData *app_data = get_app_data();
	guint count = g_list_length(app_data->open_notes);
	
	g_printerr("Open windows: %i \n", count);
	
	if (count > 1) {
		hildon_program_remove_window(program, HILDON_WINDOW(note->window));
		gtk_widget_destroy(GTK_WIDGET(note->window));
		app_data->open_notes = g_list_remove(app_data->open_notes, note);
		note_free(note);
		return;
	} 
	
	g_printerr("####################### QUIT ###################");
	gtk_main_quit();
}



gboolean note_is_open(Note *note)
{	
	AppData *app_data = get_app_data();
	GList *element = g_list_find(app_data->open_notes, note);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}

gboolean note_exists(Note *note)
{
	AppData *app_data = get_app_data();
	GList *element = g_list_find(app_data->all_notes, note);
	if (element == NULL) {
		return FALSE;
	} else {
		return TRUE;
	}
}

void note_set_focus(Note *note)
{
	if (note->window != NULL) {
		gtk_window_present(GTK_WINDOW(note->window));
	} else {
		g_printerr("ERROR: Trying to focus a window that does not exist.\n");
	}
}

void note_show(Note *note)
{
	AppData *app_data = get_app_data();
	
	/* If the note it already open, we bring this note to the foreground and return */
	if (note_is_open(note)) {
		note_set_focus(note);
		return;
	}
	
	create_mainwin(note);
	gtk_window_set_default_size(GTK_WINDOW(note->window), 500, 300);
	hildon_program_add_window(app_data->program, HILDON_WINDOW(note->window));
	
	/* Block signals on TextBuffer until we are done with initializing the content. This is to prevent saves etc. */
	g_signal_handlers_block_matched(note->buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
	
	if (note_exists(note)) {
		note_show_existing(note);
	} else {
		note_show_new(note);
	}
	
	/*app_data->open_notes = g_list_first(app_data->open_notes); */ /* TODO: Not needed - but try first */
	app_data->open_notes = g_list_append(app_data->open_notes, note);
	
	/* Format note title and update window title */
	note_format_title(note->buffer);
	note_set_window_title_from_buffer(GTK_WINDOW(note->window), note->buffer); /* Replace this. And use note->title instead */
	
	gtk_widget_show(GTK_WIDGET(note->window));	
	
	gtk_text_buffer_set_modified(note->buffer, FALSE);
	
	/* unblock signals */
	g_signal_handlers_unblock_matched(note->buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
}

void note_show_new(Note *note)
{
	AppData *app_data = get_app_data();
	GtkTextIter start, end;
	const gchar *content;
	gint notes_count;
	
	note->filename = note_get_new_filename();
	
	if (note->title == NULL) {
		notes_count = g_list_length(app_data->all_notes);
		note->title = g_strdup_printf("New Note %i", notes_count);
	}
	
	content = g_strconcat(note->title, "\n\n", "Describe your new note here.", NULL);
	gtk_text_buffer_set_text(note->buffer, content, -1);
	
	/* Select the text */
	gtk_text_buffer_get_iter_at_line(note->buffer, &start, 2);
	gtk_text_buffer_get_end_iter(note->buffer, &end);
	gtk_text_buffer_select_range(note->buffer, &start, &end);
}

void note_show_existing(Note *note)
{	
	GtkTextIter iter;
	gsize length;
	FILE *file = NULL;
	GError *error = NULL;
	guint8 *text = NULL;
	AppData *app_data = get_app_data();
	
	/* iter defines where to start with inserting */
	gtk_text_buffer_get_start_iter(note->buffer, &iter);
	  
	file = fopen(note->filename, "r");
	  
	/* Get the number of bytes */
	fseek(file, 0L, SEEK_END);
	length = ftell(file);
	fseek(file, 0L, SEEK_SET);
	text = (guint8*)g_malloc0(length * sizeof(guint8));
	fread(text, sizeof(guint8), length, file);
	fclose(file);

	/* Start serialization */
	app_data = get_app_data();
	gtk_text_buffer_deserialize(note->buffer, note->buffer, app_data->deserializer, &iter, text, length, &error);
	  
	if (error != NULL) {
		g_printerr("ERROR while deserializing: %s\n", error->message);
	}
	
	/* Set cursor possition */
	gtk_text_buffer_get_iter_at_offset(note->buffer, &iter, note->cursor_position);
	gtk_text_buffer_place_cursor(note->buffer, &iter); 
}


