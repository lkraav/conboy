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

#include "note_list_store.h"

/*#define BULLET "\342\200\242 "*/
/*#define BULLET "\u2022 "*/

typedef struct
{
	const gchar   *user_path;
	/*GList		  *all_notes;*/
	GList         *open_notes;
	gint           font_size;
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	HildonWindow  *search_window;
	NoteListStore *note_store; /* TODO: Maybe new struct for search related stuff */
} AppData;





AppData* get_app_data(void);

NoteListStore* create_note_list_store(const gchar *user_path);

const gchar* get_bullet_by_depth(gint depth);

const gchar* get_bullet_by_depth_tag(GtkTextTag *tag);

gboolean is_note_list_changed(void);

GList* sort_note_list_by_change_date(GList* note_list);

const gchar* get_current_time_in_iso8601(void);

const gchar* get_time_in_seconds_as_iso8601(time_t time_in_seconds);

time_t get_iso8601_time_in_seconds(const gchar *time_string);

const gchar* note_get_new_filename(void);

const gchar* get_uuid(void);

#endif /* METADATA_H */
