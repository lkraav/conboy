#include <gtk/gtk.h>
#include <hildon/hildon-program.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
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
static gint compare_title(gconstpointer a, gconstpointer b) {
	
	const gchar *title1 = ((Note*)a)->title;
	/*const gchar *title2 = ((Note*)b)->title;*/
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

static
void note_open_in_new_window(Note* note) {
	
	GtkTextIter iter;
	HildonProgram *program = hildon_program_get_instance();
	AppData *app_data = get_app_data();
	
	app_data->open_notes = g_list_first(app_data->open_notes);
	app_data->open_notes = g_list_append(app_data->open_notes, note);	
		
	/* Create a new window and show it */
	create_mainwin(note);
	
	gtk_window_set_default_size(GTK_WINDOW(note->window), 500, 300);
	hildon_program_add_window(program, HILDON_WINDOW(note->window));
	
	/* TODO: File checking inside note_load_to_buffer should be fixed */
	if (note->filename != NULL) {
		note_load_to_buffer(note->filename, note->buffer, note);
	}
	
	note_format_title(note->buffer);
	note_set_window_title_from_buffer(GTK_WINDOW(note->window), note->buffer);
	
	gtk_text_buffer_get_iter_at_offset(note->buffer, &iter, note->cursor_position);
	gtk_text_buffer_place_cursor(note->buffer, &iter); 
	
	gtk_widget_show(GTK_WIDGET(note->window));	
}

/*
 * Shows a note. If the note is already open it will be put to
 * the foreground. If not, a new window will be openede.
 */
void note_open(Note* metadata) {
	
	GList *element;
	AppData *app_data = get_app_data();
	GList *open_notes;
	
	open_notes = app_data->open_notes;
	
	element = g_list_find(open_notes, metadata);
	
	if (element != NULL) {
		gtk_window_present(GTK_WINDOW(metadata->window));
		return;
	}
	
	note_open_in_new_window(metadata);
}

void note_open_new_with_content(GString xml) {
	/* Should open a new window with the content from xml */
	/* Only supply <note-content> as parameter. The rest will be set by this function. */
	
	
	
}

void note_open_new_with_title(const gchar* title) {
	
	/* TODO: In the future it would be nice, if we could pass XML
	 * including formattings
	 */
	/*
	GString *content = g_string_new("");
	g_string_append(content, "<note-content version=\"0.1\">");
	g_string_append(content, title);
	g_string_append(content, "\n\n");
	g_string_append(content, "Describe your new note here.");
	
	note_open_new_wit_content(content);
	*/
	
	GtkTextIter start, end;
	const gchar *text = g_strconcat(title, "\n\n", "Describe your new note here.", NULL);
	
	Note *note = g_slice_alloc0(sizeof(Note));
	note_open_in_new_window(note);
	
	gtk_text_buffer_set_text(note->buffer, text, -1);
	note_format_title(note->buffer);
	note_set_window_title_from_buffer(GTK_WINDOW(note->window), note->buffer);
	
	gtk_text_buffer_get_iter_at_line(note->buffer, &start, 2);
	gtk_text_buffer_get_end_iter(note->buffer, &end);
	
	gtk_text_buffer_select_range(note->buffer, &start, &end);
}

void note_open_by_title(const char* title) {
	
	Note *note;
	GList *note_list;
	AppData *app_data = get_app_data();
	GList *element;
	note_list = app_data->all_notes;
	
	element = g_list_find_custom(note_list, title, compare_title);
	
	if (element == NULL) {
		g_printerr("Note with title: %s could not be found \n", title);
		note_open_new_with_title(title);
		return;
	}
	
	note = (Note*)element->data;
	note_open(note);
}




void note_load_to_buffer(const gchar* filename, GtkTextBuffer *buffer, Note *note) {
	
	GtkTextIter iter;
	gsize length;
	
	FILE *file = NULL;
	GError *error = NULL;
	guint8 *text = NULL;
	AppData *app_data;
	
	g_printerr("FILENAME: >%s< \n", filename);
	
	/* Block signals, so that on_text_buffer_modified_changed doesn't get called,
	 * because this would trigger a save right after loading */ 
	g_signal_handlers_block_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
	
	gtk_text_buffer_set_text(buffer, "", -1);
		  
	/* iter defines where to start with inserting */
	gtk_text_buffer_get_start_iter(buffer, &iter);
	  
	file = fopen(filename, "r");
	if (file == NULL) {
		g_printerr("Info: File does not yet exist. Will be created. \n");
		g_signal_handlers_unblock_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
		return;
	}
	  
	/* Get the number of bytes */
	fseek(file, 0L, SEEK_END);
	length = ftell(file);
	fseek(file, 0L, SEEK_SET);
	/*text = (guint8*)calloc(length, sizeof(guint8));*/
	text = (guint8*)g_malloc0(length * sizeof(guint8));
	fread(text, sizeof(guint8), length, file);
	fclose(file);

	/* Start serialization */
	app_data = get_app_data();
	gtk_text_buffer_deserialize(buffer, buffer, app_data->deserializer, &iter, text, length, &error);
	  
	if (error != NULL) {
		g_printerr("ERROR while deserializing: %s\n", error->message);
	}
	  
	/* After loading format the title line and change window title */
	note_format_title(buffer);
	
	gtk_text_buffer_set_modified(buffer, FALSE);
	
	g_signal_handlers_unblock_matched(buffer, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
}

/*
void note_save_from_buffer(Note *note) {
	
	gsize length;
	guint8 *data;
	GtkTextIter start, end;
	FILE *file;
	GtkTextBuffer *buffer = note->buffer;
	AppData *app_data = get_app_data();
	
	
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	
	
	data = gtk_text_buffer_serialize(buffer, buffer, app_data->serializer,
			&start, &end, &length);
	
		
	file = fopen(note->filename, "wb");
	fwrite(data, sizeof(guint8), length, file);
	fclose(file);
}
*/

void note_format_title(GtkTextBuffer *buffer) {
	
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

const gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer) {
	
	/* TODO: If title is empty set title to "Untitled XY" XY=Number of notes */
	
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

void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer) {
	gtk_window_set_title(win, note_extract_title_from_buffer(buffer));
}

static
gboolean is_empty_str(const gchar* str) {
	/* TODO: A string only containing whitespaces should be handled as empty too */
	if (strcmp(str, "") == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void note_save(Note *note) {
		
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
		/* TODO: Free all data */
		return;
	} 
	
	g_printerr("####################### QUIT ###################");
	gtk_main_quit();
}


