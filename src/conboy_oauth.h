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


#ifndef CONBOY_OAUTH_H_
#define CONBOY_OAUTH_H_

#include <glib/gtypes.h>

#include "note.h"
#include "json.h"

gchar*
get_auth_link(gchar *call_url, gchar *link_url, gchar **t_key, gchar **t_secret);

gboolean
get_access_token(gchar *url, gchar **t_key, gchar **t_secret);

gchar*
get_all_notes(gboolean inc_notes);

gint
web_sync_send_notes(GList *notes, gchar *url, gint expected_rev, time_t last_sync_time, gint *uploaded_notes, GError **error);

JsonNoteList*
web_sync_get_notes(JsonUser *user, int since_rev);

gchar*
conboy_get_auth_link(const gchar *call_url, const gchar *link_url);

gboolean
conboy_get_access_token(const gchar *url, const gchar *verifier);

gchar*
conboy_http_get(const gchar *url);

#endif /*CONBOY_OAUTH_H_*/
