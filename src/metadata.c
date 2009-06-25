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
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <hildon/hildon-window.h>

/* without this, time.h will not include strptime() */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#include "metadata.h"
#include "note.h"
#include "note_list_store.h"
#include "app_data.h"






/* char *_bullets[] = {"\u2022 ", "\u2218 ", "\u2023 ", "\u2043 ", "\u204d ", "\u2219 ", "\u25e6 "}; */
/* These 3 bullets work with diablo and standard font */
char *_bullets[] = {"\u2022 ", "\u25e6 ", "\u2219 "};
const gchar* get_bullet_by_depth(gint depth) {
	if (depth <= 0) {
		g_printerr("ERROR: get_bullets_by_depth(): depth must be at least 1.\n");
		return "\u2022 ";
	}
	return _bullets[(depth - 1) % 3];
}

const gchar* get_bullet_by_depth_tag(GtkTextTag *tag) {
	return get_bullet_by_depth(tag_get_depth(tag));
}


gchar* get_uuid()
{
	gchar *content;
	gchar *result;
	g_file_get_contents("/proc/sys/kernel/random/uuid", &content, NULL, NULL);
	g_strchomp(content);
	result = g_strconcat(content, "\0", NULL);
	g_free(content);
	return result;
}

const gchar* note_get_new_filename(const gchar *uuid)
{
	AppData *app_data = app_data_get();
	return g_strconcat(app_data->user_path, uuid, ".note", NULL);
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

gboolean is_portrait_mode()
{
	GdkScreen *screen = gdk_screen_get_default();
	int width = gdk_screen_get_width(screen);
	int height = gdk_screen_get_height(screen);

	g_printerr("Screen size: %i x %i \n", width, height);

	/* Hopefully we don't have a square screen ;) */
	if (width > height) {
		return FALSE;
	} else {
		return TRUE;
	}
}

