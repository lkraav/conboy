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

#include "metadata.h"
#include "interface.h"
#include "note.h"
#include "serializer.h"
#include "deserializer.h"


Note* note_create_new()
{
	Note *note = g_new0(Note, 1);
	UserInterface *ui = g_new0(UserInterface, 1);
	note->ui = ui;
	note->active_tags = NULL;
	return note;
}

void note_free(Note *note)
{
	g_free(note->ui);
	g_free(note->filename);
	g_free(note->title);
	g_free(note->version);
	g_slist_free(note->active_tags);
}

/**
 * Compares the the title of a given note the the given title string in an
 * case insensitiv manner.
 * 
 * a must be a note with title
 * b must be a title string
 */
static gint compare_title(gconstpointer a, gconstpointer b)
{	
	int result;
	gchar *title1 = g_utf8_casefold(((Note*)a)->title, -1);
	gchar *title2 = g_utf8_casefold(b, -1);
	
	if (title1 == NULL && title2 == NULL) {
		return 0;
	}
	if (title1 == NULL) {
		return -1;
	}
	if (title2 == NULL) {
		return 1;
	}
	result = strcmp(title1, title2);
	
	g_free(title1);
	g_free(title2);
	
	return result;
}

void note_show_by_title(const char* title)
{	
	Note *note;
	GList *element;
	AppData *app_data = get_app_data();
	GList *all_notes = app_data->all_notes;
	
	element = g_list_find_custom(all_notes, title, compare_title);
	
	if (element == NULL) {
		note = note_create_new();
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
	const gchar* content;
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
		return;
	}
	
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
	/* We don't change height, width, x and y and we don't need them */
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
	
	app_data = get_app_data();
	
	/*g_printerr("Saving >%s< \n", note->title);*/
	
	/* Set start and end iterators for serialization */
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	
	/* Start serialization */
	serialize_note(note);
	
	/* If first save, add to list of all notes */
	if (!g_list_find(app_data->all_notes, note)) {
		/*g_printerr("Note >%s< is not yet in the list. Adding.\n", note->title);*/
		app_data->all_notes = g_list_append(app_data->all_notes, note);
	}
	
	gtk_text_buffer_set_modified(buffer, FALSE);
}



void note_close_window(Note *note)
{
	HildonProgram *program = hildon_program_get_instance();
	HildonWindow *window = note->ui->window;
	AppData *app_data = get_app_data();
	guint count = g_list_length(app_data->open_notes);
	
	if (count > 1) {
		hildon_program_remove_window(program, window);
		gtk_widget_destroy(GTK_WIDGET(window));
		app_data->open_notes = g_list_remove(app_data->open_notes, note);
		/* Don't free note, because we reuse this in the menu with the available notes and when reopening */
		/*note_free(note);*/
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
	if (note->ui->window != NULL) {
		gtk_window_present(GTK_WINDOW(note->ui->window));
	} else {
		g_printerr("ERROR: Trying to focus a window that does not exist.\n");
	}
}

void note_show(Note *note)
{
	AppData *app_data = get_app_data();
	GtkTextBuffer *buffer; /*= note->ui->buffer;*/
	GtkWindow *window; /* = GTK_WINDOW(note->ui->window);*/
	
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
	AppData *app_data = get_app_data();
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter start, end;
	const gchar *content;
	gint notes_count;
	
	note->filename = note_get_new_filename();
	
	if (note->title == NULL) {
		notes_count = g_list_length(app_data->all_notes);
		note->title = g_strdup_printf("New Note %i", notes_count);
	}
	
	content = g_strconcat(note->title, "\n\n", "Describe your new note here.", NULL);
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
	
	/* start deserialization */
	deserialize_note(note);

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
	if (note == NULL) {
		g_printerr("ERROR: note_add_active_tag: note is NULL\n");
		return;
	}
	if (tag == NULL) {
		g_printerr("ERROR: note_add_active_tag: tag is NULL\n");
		return;
	}
	
	GSList *tags = note->active_tags;
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



