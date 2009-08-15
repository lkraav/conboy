#ifndef CONBOY_OAUTH_H_
#define CONBOY_OAUTH_H_

#include <glib/gtypes.h>

#include "note.h"

gchar*
get_auth_link(gchar *call_url, gchar *link_url, gchar **t_key, gchar **t_secret);

gboolean
get_access_token(gchar *url, gchar **t_key, gchar **t_secret);

gchar*
get_all_notes(const gchar *base_url, gboolean inc_notes, const gchar *t_key, const gchar *t_secret);

void
web_send_note(Note *note, const gchar *base_url, const gchar *t_key, const gchar *t_secret);

gchar*
conboy_http_get(const gchar *url);

#endif /*CONBOY_OAUTH_H_*/
