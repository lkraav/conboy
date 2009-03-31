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

#ifndef METADATA_H
#define METADATA_H

#include <gconf/gconf-client.h>

typedef struct
{
	const gchar *user_path;
	GList		*all_notes;
	GList       *open_notes;
	GdkAtom      serializer;
	GdkAtom      deserializer;
	gint         font_size;
	GConfClient *client;
} AppData;

typedef struct
{
  GtkTextBuffer *buffer;
  HildonWindow  *window;
  GtkTextView   *view;
  const gchar   *title;
  const gchar   *filename;
  
  const gchar *last_change_date;
  const gchar *last_metadata_change_date;
  const gchar *create_date;
  
  gint cursor_position;
  gint width;
  gint height;
  gint x;
  gint y;
  gboolean open_on_startup;
  const gchar *version;
  
} Note;

AppData* get_app_data();

/*AppData* init_app_data(const gchar *user_path);*/

GList* create_note_list(AppData *app_data);

gboolean is_note_list_changed();

const gchar* get_current_time_in_iso8601();

time_t get_iso8601_time_in_seconds(const gchar *time_string);

const gchar* note_get_new_filename();

const gchar* get_uuid();

#endif /* METADATA_H */
