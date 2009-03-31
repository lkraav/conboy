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

#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <hildon/hildon-window.h>

/* without this, time.h will not include strptime() */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#include "metadata.h"


/* Global AppData only access with get_app_data() */
AppData *_app_data = NULL;

AppData* get_app_data() {
	
	gint font_size;
	GConfClient *client;
	const gchar *path;
	
	if (_app_data == NULL) {
		
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
		
		_app_data = g_slice_alloc0(sizeof(AppData));
		_app_data->user_path = path;
		_app_data->all_notes = create_note_list(_app_data);
		_app_data->open_notes = NULL;
		_app_data->client = client;
		_app_data->font_size = font_size;
		_app_data->program = hildon_program_get_instance();
	}
	
	return _app_data;
}

int last_hash = 0;
/* TUT NICHT */
gboolean is_note_list_changed() {
	
	/*
	struct stat buf;
	time_t mtime;
	const gchar *filename;
	int hash = 0;
	gboolean result;
	GDir *dir = g_dir_open(NOTE_DIR, 0, NULL);
	
	while ((filename = g_dir_read_name(dir)) != NULL) {	
		stat(filename, &buf);
		mtime = buf.st_mtime;
		hash = hash & mtime;
		g_printerr("%s  Time: %i  Hash: %i \n", filename, mtime, hash);
	}
	
	g_dir_close(dir);
	
	result = (hash != last_hash);
	last_hash = hash;
	return result;
	*/
	return TRUE;
}

GList* create_note_list(AppData *app_data) {
	
	g_printerr("FFFILE: >%s< \n", app_data->user_path);
	
	const gchar *filename; 
	GList *notes = NULL;
	GDir *dir = g_dir_open(app_data->user_path, 0, NULL);
	Note *note;
	
	GMappedFile *file;
	gchar *content;
	gchar *start, *end;
	
	/*
	 * TODO: We create a lot of Note objects here. They should be
	 * freed or reused somehow.
	 */
	
	while ((filename = g_dir_read_name(dir)) != NULL) {
		if (g_str_has_suffix(filename, ".note")) {
			
			/* Create new note and append to list */
			note = g_slice_new(Note); /* note = malloc(sizeof(Note)); */
			notes = g_list_append(notes, note); /* faster is prepend and then reverse */
			
			/* Save app_data in note */
			/*note->app_data = app_data;*/
			
			/* Save filename in note */
			note->filename = g_strconcat(app_data->user_path, filename, NULL); /*g_strdup(filename);*/ /* copy string, otherwise it could be invalid outside of this function */
			
			/* Open file and read out title. Save title in note */
			file = g_mapped_file_new(g_strconcat(app_data->user_path, filename, NULL), FALSE, NULL);
			content = g_mapped_file_get_contents(file);			
			start = g_strrstr(content, "<title>"); /* move pointer to begining of <title> */
			start = start + sizeof(gchar) * 7; /* move another 7 characters to be after <title> */
			end = g_strrstr(content, "</title>"); /* move another pointer to begining of </title> */
			note->title = g_strndup(start, end - start); /* copy the area between start and end, which is the title */
			g_mapped_file_free(file);
			
		}
	}
  
	g_dir_close(dir);
	return notes;
}



/* TODO: Replace with real UUID function */
const gchar* get_uuid()
{	
	gchar *content;
	g_file_get_contents("/proc/sys/kernel/random/uuid", &content, NULL, NULL);
	g_strchomp(content);
	return g_strdup(content);
}

const gchar* note_get_new_filename() {
	AppData *app_data = get_app_data();
	return g_strconcat(app_data->user_path, get_uuid(), ".note", NULL);
}

/**
 * Returns the current time as string formattet as in iso8601.
 * Example: 2009-03-24T13:16:42.0000000+01:00
 */
const gchar* get_current_time_in_iso8601() {
	
	gchar      time_string[40];
	gchar     *minutes;
	gchar     *text_pointer;
	time_t     time_in_seconds;
	struct tm *local_time;
	gchar     *first_part;
	gchar     *result;
	
	time(&time_in_seconds);
	local_time = localtime(&time_in_seconds);
	
	/* Milliseconds are always 0 and timezone is +0100 -> should be +01:00 */
	strftime(time_string, 40, "%Y-%m-%dT%H:%M:%S.0000000%z", local_time);
	
	/* Save minutes, add colon, add minutes */
	text_pointer = (gchar*) &time_string;
	text_pointer = text_pointer + 30;
	
	minutes = g_strndup(text_pointer, 2);
	first_part = g_strndup(time_string, 30);
	result = g_strconcat(first_part, ":", minutes, NULL);
	
	g_free(minutes);
	g_free(first_part);
	
	return result;
}

time_t get_iso8601_time_in_seconds(const gchar *time_string) {
	
	time_t result;
	struct tm local_time;
	
	/* Looks like it's ok that the colon is used with the timezone,
	 * if there will be trouble, remove the colon first. See
	 * get_current_time_in_iso8601() */
	
	strptime(time_string, "%Y-%m-%dT%H:%M:%S.0000000%z", &local_time);
	result = mktime(&local_time);
	return result;
}


