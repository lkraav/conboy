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

/*char _bullets[3][4] = {"\u2022 ", "\u2218 ", "\u2023 "};   \0xE2 \0x88 \0x98*/
char *_bullets[] = {"\u2022 ", "\u2218 ", "\u2023 ", "\u2043 ", "\u204d ", "\u2219 ", "\u25e6 "};
const gchar* get_bullet_by_depth(gint depth) {
	if (depth <= 0) {
		g_printerr("ERROR: get_bullets_by_depth(): depth must be at least 1.\n");
		return "\u2022 ";
	}
	return _bullets[(depth - 1) % 7];
}

const gchar* get_bullet_by_depth_tag(GtkTextTag *tag) {
	return get_bullet_by_depth(tag_get_depth(tag));
}


static
gint compare_notes_by_change_date(gconstpointer a, gconstpointer b) {
	
	Note *n1 = (Note*)a;
	Note *n2 = (Note*)b;
	
	if (n1->last_change_date > n2->last_change_date) {
		return -1;
	} else if (n1->last_change_date < n2->last_change_date) {
		return 1;
	} else {
		return 0;
	}
}

GList* sort_note_list_by_change_date(GList* note_list)
{
	/* Sort list using last-change-date */
	return g_list_sort(note_list, compare_notes_by_change_date);
}

GList* create_note_list(AppData *app_data) {
		
	const gchar *filename; 
	GList *notes = NULL;
	GDir *dir = g_dir_open(app_data->user_path, 0, NULL);
	Note *note;
	GMappedFile *file;
	gchar *content;
	gchar *start, *end;
	gchar *tmp;
	
	while ((filename = g_dir_read_name(dir)) != NULL) {
		if (g_str_has_suffix(filename, ".note")) {
			
			/* Create new note and append to list */
			note = note_create_new(); /*g_slice_new0(Note);*/ /* note = malloc(sizeof(Note)); */
			notes = g_list_prepend(notes, note);
			
			/* Save filename in note */
			note->filename = g_strconcat(app_data->user_path, filename, NULL);
			
			/* Open file and read out title. Save title in note */
			file = g_mapped_file_new(g_strconcat(app_data->user_path, filename, NULL), FALSE, NULL);
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
	notes = sort_note_list_by_change_date(notes);
	return notes;
}

const gchar* get_uuid()
{	
	gchar *content;
	g_file_get_contents("/proc/sys/kernel/random/uuid", &content, NULL, NULL);
	g_strchomp(content);
	return g_strdup(content);
}

const gchar* note_get_new_filename()
{
	AppData *app_data = get_app_data();
	return g_strconcat(app_data->user_path, get_uuid(), ".note", NULL);
}


/**
 * Returns the given time as iso8601 formatted string.
 * Example: 2009-03-24T13:16:42.0000000+01:00
 * 
 * This method is needed, because the glib function
 * g_time_val_to_iso8601() does only produce the short format without
 * milliseconds. E.g. 2009-04-17T13:14:52Z
 */
const gchar* get_time_in_seconds_as_iso8601(time_t time_in_seconds) {
	
	gchar      time_string[40];
	gchar     *minutes;
	gchar     *text_pointer;
	struct tm *local_time;
	gchar     *first_part;
	gchar     *result;
	
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


/**
 * Returns the current time as string formattet as in iso8601.
 * Example: 2009-03-24T13:16:42.0000000+01:00
 * 
 * This method is needed, because the glib function
 * g_time_val_to_iso8601() does only produce the short format without
 * milliseconds. E.g. 2009-04-17T13:14:52Z
 */
const gchar* get_current_time_in_iso8601() {
	return get_time_in_seconds_as_iso8601(time(NULL));
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


