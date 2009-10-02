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
conboy_get_auth_link(const gchar *base_url);

gboolean
conboy_get_access_token(void);

gchar*
conboy_http_get(const gchar *url);

#endif /*CONBOY_OAUTH_H_*/
