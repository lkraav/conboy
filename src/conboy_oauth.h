#ifndef CONBOY_OAUTH_H_
#define CONBOY_OAUTH_H_

#include <glib/gtypes.h>

#include "note.h"

gchar*
get_auth_link(gchar *call_url, gchar *link_url, gchar **t_key, gchar **t_secret);

gboolean
get_access_token(gchar *url, gchar **t_key, gchar **t_secret);

gchar*
get_all_notes(gboolean inc_notes);

gint
web_send_notes(GList *notes, gint expected_rev, time_t last_sync_time);

gchar*
conboy_get_auth_link(const gchar *base_url);

gboolean
conboy_get_access_token(void);

gchar*
conboy_http_get(const gchar *url);

#endif /*CONBOY_OAUTH_H_*/
