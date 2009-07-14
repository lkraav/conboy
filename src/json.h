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

#ifndef JSON_H
#define JSON_H

#include <json-glib/json-glib.h>
#include "note.h"

JsonNode* json_get_node_from_note(Note *note);
Note* json_get_note_from_node(JsonNode *node);
Note* json_get_note_from_string(const gchar *json_string);
void json_print_note(Note *note);

GSList *json_get_notes_from_string(const gchar *json_string);

void json_test(void);


#endif /* JSON_H */
