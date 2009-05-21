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

#include <glib.h>
#include <glib/gstdio.h>

#include "app_data.h"

/* Global AppData only access with get_app_data() */
AppData *_app_data = NULL;


static void populate_note_store(NoteListStore *store, const gchar *user_path) {
	
	const gchar *filename;
	GDir *dir = g_dir_open(user_path, 0, NULL);
	Note *note;
	GMappedFile *file;
	gchar *content;
	gchar *start, *end;
	gchar *tmp;
	GtkTreeIter iter;
	
	while ((filename = g_dir_read_name(dir)) != NULL) {
		if (g_str_has_suffix(filename, ".note")) {
			gchar *full_filename;
			
			/* Create new note and append to list store */
			note = note_create_new();
			note_list_store_add(store, note, &iter);
			
			/* Save filename in note */
			note->filename = g_strconcat(user_path, filename, NULL);
			
			/* Open file and read out title. Save title in note */
			full_filename = g_strconcat(user_path, filename, NULL);
			file = g_mapped_file_new(full_filename, FALSE, NULL);
			g_free(full_filename);
			content = g_mapped_file_get_contents(file);			
			
			/* TODO: Maybe use real xml parser for this. But I think this way is faster */
			start = g_strrstr(content, "<title>"); /* move pointer to begining of <title> */
			start = start + sizeof(gchar) * 7; /* move another 7 characters to be after <title> */
			end = g_strrstr(content, "</title>"); /* move another pointer to begining of </title> */
			note->title = g_strndup(start, end - start); /* copy the area between start and end, which is the title */
			
			/* Read out last-change-date and save to note */
			start = g_strrstr(content, "<last-change-date>");
			start = start + sizeof(gchar) * 18;
			end = g_strrstr(content, "</last-change-date>");
			tmp = g_strndup(start, end - start);
			note->last_change_date = get_iso8601_time_in_seconds(tmp);
			g_free(tmp);
			
			g_mapped_file_free(file);
		}
	}
	g_dir_close(dir);
}


AppData* app_data_get() {
	
	if (_app_data == NULL) {
		gint font_size;
		GConfClient *client;
		const gchar *path;
		NoteListStore *store;

		client = gconf_client_get_default();
		gconf_client_add_dir(client, "/apps/maemo/conboy", GCONF_CLIENT_PRELOAD_NONE, NULL);
		
		font_size = gconf_client_get_int(client, "/apps/maemo/conboy/font_size", NULL);
		if (font_size == 0) {
			font_size = 20000;
		}
		
		path = g_strconcat(g_get_home_dir(), "/.conboy/", NULL);
			
		/* Create dir if needed */
		if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
			g_mkdir(path, 0700);
		}
		
		store = note_list_store_new();
		
		_app_data = g_new(AppData, 1);
		_app_data->user_path = path;
		_app_data->note_store = store;
		_app_data->open_notes = NULL;
		_app_data->client = client;
		_app_data->font_size = font_size;
		_app_data->program = hildon_program_get_instance();
		_app_data->fullscreen = FALSE;
		_app_data->search_window = NULL;
		
		populate_note_store(store, path);
	}
	
	return _app_data;
}

void app_data_free()
{
	AppData *app_data = app_data_get();
	
	g_object_unref(app_data->client);
	g_list_free(app_data->open_notes);
	g_free((gchar*)app_data->user_path);
	if (app_data->search_window != NULL) {
		gtk_widget_destroy(GTK_WIDGET(app_data->search_window));
	}
	g_free(app_data);
}
