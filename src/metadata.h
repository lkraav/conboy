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
/* without this, time.h will not include strptime() */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#include "conboy_note_store.h"

gboolean is_note_list_changed(void);

GList* sort_note_list_by_change_date(GList* note_list);

const gchar* get_current_time_in_iso8601(void);

const gchar* get_time_in_seconds_as_iso8601(time_t time_in_seconds);

time_t get_iso8601_time_in_seconds(const gchar *time_string);

const gchar* note_get_new_filename(const gchar *uuid);

gboolean is_portrait_mode(void);

#endif /* METADATA_H */
