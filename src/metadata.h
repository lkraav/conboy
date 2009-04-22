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
#include <hildon/hildon-program.h>

#define BULLET "\342\200\242 "

typedef struct
{
	const gchar   *user_path;
	GList		  *all_notes;
	GList         *open_notes;
	GdkAtom        serializer;
	GdkAtom        deserializer;
	gint           font_size;
	GConfClient   *client;
	HildonProgram *program;
} AppData;

typedef struct
{
	HildonWindow        *window;
	GtkTextView         *view;
	GtkTextBuffer       *buffer;
	
	GtkToggleToolButton *button_bold;
	GtkToggleToolButton *button_italic;
	GtkToggleToolButton *button_strike;
	GtkToggleToolButton *button_highlight;
	GtkToggleToolButton *button_bullets;
	
	GtkCheckMenuItem    *menu_bold;
	GtkCheckMenuItem    *menu_italic;
	GtkCheckMenuItem	*menu_fixed;
	GtkCheckMenuItem    *menu_strike;
	GtkCheckMenuItem    *menu_highlight;
	GtkCheckMenuItem    *menu_bullets;

} UserInterface;

typedef struct
{
  UserInterface *ui;
  const gchar   *title;
  const gchar   *filename;
  
  time_t last_change_date;
  time_t last_metadata_change_date;
  time_t create_date;
  
  gint cursor_position;
  gint width;
  gint height;
  gint x;
  gint y;
  gboolean open_on_startup;
  const gchar *version;
  
  GSList *active_tags;
  
} Note;

AppData* get_app_data();

/*AppData* init_app_data(const gchar *user_path);*/

GList* create_note_list(AppData *app_data);

gboolean is_note_list_changed();

GList* sort_note_list_by_change_date(GList* note_list);

const gchar* get_current_time_in_iso8601();

const gchar* get_time_in_seconds_as_iso8601(time_t time_in_seconds);

time_t get_iso8601_time_in_seconds(const gchar *time_string);

const gchar* note_get_new_filename();

const gchar* get_uuid();

#endif /* METADATA_H */
