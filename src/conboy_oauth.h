#ifndef CONBOY_OAUTH_H_
#define CONBOY_OAUTH_H_

#include <glib/gtypes.h>

#include "note.h"

gchar*
get_auth_link(gchar **t_key, gchar **t_secret);

gboolean
get_access_token(gchar **t_key, gchar **t_secret);

gchar*
get_all_notes(const gchar *t_key, const gchar *t_secret);

void
web_send_note(Note *note, const gchar *t_key, const gchar *t_secret);


#endif /*CONBOY_OAUTH_H_*/
