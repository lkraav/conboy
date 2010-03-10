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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gmessages.h>
#include <glib/gprintf.h>
#include <curl/curl.h>
#include <oauth.h>

#include "conboy_oauth.h"
#include "settings.h"
#include "note.h"
#include "json.h"
#include "app_data.h"
#include "conboy_http.h"


#define c_key    "anyone"  /*< consumer key */
#define c_secret "anyone" /*/< consumer secret */





/*
 * Returns 0 on success, 1 otherwise
 */
static int
parse_reply (const char *reply, char **token, char **secret) {
	int rc;
	int ok=1;
	char **rv = NULL;
	rc = oauth_split_url_parameters(reply, &rv);
	qsort(rv, rc, sizeof(char *), oauth_cmpstringp);

	/* Reply to request_token request returns three parameters */
	if (rc == 3 && !strncmp(rv[1], "oauth_token=", 11) && !strncmp(rv[2], "oauth_token_secret=", 18) ) {
		ok = 0;
		if (token)   *token = g_strdup(&(rv[1][12]));
		if (secret) *secret = g_strdup(&(rv[2][19]));
		g_printerr("key:    '%s'\nsecret: '%s'\n", *token, *secret);
	}

	/* Reply to access_token request returns two parameters */
	else if (rc == 2 && !strncmp(rv[0], "oauth_token=", 11) && !strncmp(rv[1], "oauth_token_secret=", 18) ) {
		ok = 0;
		if (token)   *token = g_strdup(&(rv[0][12]));
		if (secret) *secret = g_strdup(&(rv[1][19]));
		g_printerr("key:    '%s'\nsecret: '%s'\n", *token, *secret);
	}

	if(rv) free(rv);
	return ok;
}




/*
 *
 *
 *
 *
 *
 */





/**
 *
 *
 *
 *
 */


gchar*
conboy_get_auth_link(const gchar *call_url, const gchar *link_url)
{
	gchar *tok = "";
	gchar *sec = "";

	gchar *link = get_auth_link(call_url, link_url ,&tok, &sec);

	settings_save_oauth_access_token(tok);
	settings_save_oauth_access_secret(sec);

	g_free(call_url);
	g_free(link_url);

	return link;
}


/* TODO: Improve error checking */
gchar*
get_auth_link(gchar *request_url, gchar *link_url, gchar **t_key, gchar **t_secret)
{
	gchar *postarg = NULL;
	gchar *reply   = NULL;
	gchar *link = NULL;

	/*
	 * TODO: oob does not work. Somehow we must transport the oauth_verifier from
	 * the service to us. Usually this is done via the callback url, but then
	 * we need to have a webserver....
	 * No need for a webserver. It works with the the url handling of Maemo
	 */
	gchar *request = g_strconcat(request_url, "?oauth_callback=conboy://authenticate", NULL);

	/* Request the token, therefore use NULL, NULL */
	gchar *req_url = oauth_sign_url2(request, &postarg, OA_HMAC, "POST", c_key, c_secret, NULL, NULL);
	g_free(request);

	if (req_url == NULL) {
		g_printerr("ERROR: REQ URL = NULL\n");
		return NULL;
	}

	reply = conboy_http_post(req_url, postarg, FALSE);

	g_printerr("Reply: %s\n", reply);

	if (reply == NULL) {
		g_printerr("ERROR: Reply = NULL\n");
		g_free(req_url);
		return NULL;
	}

	if (strlen(reply) > 200) {
		g_printerr("ERROR: Reply is longer then 200 characters, cannot be right\n");
		g_free(req_url);
		return NULL;
	}

	if (parse_reply(reply, t_key, t_secret)) {
		g_printerr("ERROR: Reply could not be parsed\n");
		g_free(req_url);
		return NULL;
	}

	g_free(req_url);
	g_free(reply);

	/* TODO: Use conboy:// instead of http://google.de. Problem is, it doesnt work for now.
	 * There is some bug in Snowy/Piston/Django...
	 * http://mail.gnome.org/archives/snowy-list/2009-July/msg00002.html */
	/* We now don't need to add the callback here, because we provided it with the request already */
	link = g_strconcat(link_url, "?oauth_token=", *t_key, NULL);
	/*link = g_strconcat(link_url, "?oauth_token=", *t_key, "&oauth_callback=conboy://", NULL);*/
	/*link = g_strconcat(link_url, "?oauth_token=", *t_key, "&oauth_callback=http://www.google.de", NULL);*/

	return link;
}


gboolean
get_access_token(gchar *url, gchar **t_key, gchar **t_secret)
{
	gchar *reply = NULL;
	gchar *postarg = NULL;

	gchar *req_url = oauth_sign_url2(url, &postarg, OA_HMAC, "POST", c_key, c_secret, *t_key, *t_secret);

	if (req_url == NULL) {
		g_printerr("ERROR: req_url = NULL");
		return FALSE;
	}

	/*reply = oauth_http_post(req_url, postarg);*/
	reply = conboy_http_post(req_url, postarg, FALSE);
	if (reply == NULL) {
		g_printerr("ERROR: reply = NULL");
		g_free(req_url);
		return FALSE;
	}

	g_printerr("Access Reply: >%s< \n", reply);

	if (strlen(reply) > 200) {
		/* Answer is too long, cannot be correct */
		g_printerr("ERROR: Cannot get access token. Answer of server was longer than 200 characters.");
		g_free(reply);
		return FALSE;
	}

	if (parse_reply(reply, t_key, t_secret)) {
		g_printerr("ERROR: Cannot parse access token reply\n");
		g_free(reply);
		return FALSE;

	} else {
		g_free(reply);
		return TRUE;
	}
}

gboolean
conboy_get_access_token(const gchar *url, const gchar *verifier) {

	gchar *tok = settings_load_oauth_access_token();
	gchar *sec = settings_load_oauth_access_secret();

	gchar *full_url = g_strconcat(url, "?", "oauth_verifier=", verifier, NULL);
	g_printerr("### AccessToken url: %s\n", full_url);

	if (get_access_token(full_url, &tok, &sec)) {
		g_printerr("acc_tok: %s\n", tok);
		g_printerr("acc_sec: %s\n", sec);
		settings_save_oauth_access_token(tok);
		settings_save_oauth_access_secret(sec);
		g_free(full_url);
		return TRUE;
	}

	g_free(full_url);
	return FALSE;
}





/*
 *
 *
 *
 *
 *
 *
 *
 *
 * New FILE
 *
 *
 *
 *
 *
 *
 *
 *
 */

static gint
web_sync_send_notes(GList *notes, gchar *url, gint expected_rev, time_t last_sync_time, gint *uploaded_notes, GError **error)
{
	gchar *t_key = settings_load_oauth_access_token();
	gchar *t_secret = settings_load_oauth_access_secret();

	/*
	 * Create correct json structure to send the note
	 */
	JsonNode *result = json_node_new(JSON_NODE_OBJECT);
	JsonObject *obj = json_object_new();
	JsonArray *array = json_array_new();

	while (notes) {
		ConboyNote *note = CONBOY_NOTE(notes->data);
		if (note->last_metadata_change_date > last_sync_time) {
			g_printerr("Will send: %s\n", note->title);
			JsonNode *note_node = json_get_node_from_note(note);
			json_array_add_element(array, note_node);
		}
		notes = notes->next;
	}

	*uploaded_notes = json_array_get_length(array);
	if (*uploaded_notes == 0) {
		g_printerr("INFO: No new notes on client. Sending nothing.\n");
		return expected_rev - 1;
	}

	JsonNode *node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(node, array);
	json_object_add_member(obj, "note-changes", node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(node, expected_rev);
	json_object_add_member(obj, "latest-sync-revision", node);

	json_node_take_object(result, obj);

	/* Convert to string */
	gchar *json_string = json_node_to_string(result, FALSE);

	g_printerr("&&&&&&&&&&&&&&&&&&\n");
	g_printerr("%s\n", json_string);
	g_printerr("&&&&&&&&&&&&&&&&&&\n");


	gchar *reply = conboy_http_put(url, json_string, TRUE);


	g_printerr("Reply from Snowy:\n");
	g_printerr("%s\n", reply);

	/* Parse answer and see if expected_rev fits or not */
	JsonNoteList *note_list = json_get_note_list(reply);

	if (note_list == NULL) {
		g_set_error(error, 0, 0, "json_get_note_list() returned NULL");
		return expected_rev - 1;
	}

	if (note_list->latest_sync_revision != expected_rev) {
		g_printerr("ERROR: Expected sync rev (%i) and actual sync rev (%i) are not the same\n", expected_rev, note_list->latest_sync_revision);
		g_set_error(error, 0, 0, "Expected sync rev (%i) and actual sync rev (%i) are not the same\n", expected_rev, note_list->latest_sync_revision);
		return note_list->latest_sync_revision;
	}

	return expected_rev;
}

static JsonNoteList*
web_sync_get_notes(JsonUser *user, int since_rev)
{
	JsonNoteList *result;
	gchar *json_string;
	gchar get_all_notes_url[1024];

	g_sprintf(get_all_notes_url, "%s?include_notes=true&since=%i", user->api_ref, since_rev);

	json_string = conboy_http_get(get_all_notes_url, TRUE);
	result = json_get_note_list(json_string);

	g_free(json_string);
	return result;
}

static void
web_sync_show_message (WebSyncDialogData *data, gchar *msg)
{
	gdk_threads_enter();

	gtk_widget_hide(GTK_WIDGET(data->bar));
	gtk_label_set_markup(data->label, msg);
	gtk_widget_set_sensitive(GTK_WIDGET(data->button), TRUE);

	gdk_threads_leave();
}

static void
web_sync_pulse_bar (GtkProgressBar *bar)
{
	gdk_threads_enter();
	gtk_progress_bar_pulse(bar);
	gdk_threads_leave();
}


static GList*
web_sync_remove_by_guid(GList *list, ConboyNote *note_to_remove)
{
	gchar *guid;
	g_object_get(note_to_remove, "guid", &guid, NULL);
	ConboyNote *found_note = NULL;

	GList *iter = list;
	while (iter) {
		ConboyNote *note = CONBOY_NOTE(iter->data);
		gchar *other_guid;
		g_object_get(note, "guid", &other_guid, NULL);
		if (strcmp(guid, other_guid) == 0) {
			found_note = note;
		}
		g_free(other_guid);
		if (found_note) break;
		iter = iter->next;
	}

	g_free(guid);

	if (found_note) {
		g_printerr("Remove note\n");
		return g_list_remove(list, found_note);
	} else {
		return list;
	}
}



void
web_sync_do_sync (gpointer *user_data)
{
	WebSyncDialogData *data = (WebSyncDialogData*)user_data;
	AppData *app_data = app_data_get();
	UserInterface *ui = app_data->note_window;
	GtkProgressBar *bar = data->bar;



	/* Save and make uneditable */
	gdk_threads_enter();
	gtk_text_view_set_editable(ui->view, FALSE);
	note_save(ui);
	gdk_threads_leave();

	/* Save guid of current note */
	gchar *guid = NULL;
	if (ui->note != NULL) {
		g_object_get(ui->note, "guid", &guid, NULL);
	}




	web_sync_pulse_bar(bar);

	gchar *url = settings_load_sync_base_url();
	if (url == NULL || strcmp(url, "") == 0) {
		web_sync_show_message(data, "Please first set a URL in the settings");
		return;
	}
	web_sync_pulse_bar(bar);

	int last_sync_rev = settings_load_last_sync_revision();
	time_t last_sync_time = settings_load_last_sync_time();

	gchar *request = g_strconcat(url, "/api/1.0/", NULL);

	gchar *reply = conboy_http_get(request, FALSE);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: %s\n", request, NULL);
		web_sync_show_message(data, msg);
		g_free(msg);
		return;
	}
	g_free(request);
	web_sync_pulse_bar(bar);

	g_printerr("Reply from /api/1.0/:: %s\n", reply);

	gchar *api_ref = json_get_api_ref(reply);

	g_printerr("Now asking::: %s\n", api_ref);

	reply = conboy_http_get(api_ref, TRUE);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: \n", api_ref, NULL);
		web_sync_show_message(data, msg);
		g_free(msg);
		return;
	}
	web_sync_pulse_bar(bar);

	g_printerr("Reply from /user/:: %s\n", reply);

	/* Revision checks */
	JsonUser *user = json_get_user(reply);
	if (user->latest_sync_revision < last_sync_rev) {
		g_printerr("U1 rev: %i   Local rev: %i\n", user->latest_sync_revision, last_sync_rev);
		web_sync_show_message(data, "Server revision older than our revision.");
		return;
	}
	web_sync_pulse_bar(bar);

	g_printerr("###### Got User from server ########\n");
	g_printerr("# First name: %s\n", user->first_name);
	g_printerr("# Last name: %s\n", user->last_name);
	g_printerr("#   Api ref: %s\n", user->api_ref);
	g_printerr("####################################\n");

	/* Create list of all local notes */
	/* Just copy NoteStore to normal list */
	ConboyNoteStore *note_store = app_data->note_store;
	GList *local_notes = NULL;
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(note_store), &iter)) do
	{
		ConboyNote *note;
		gtk_tree_model_get(GTK_TREE_MODEL(note_store), &iter, NOTE_COLUMN, &note, -1);
		local_notes = g_list_append(local_notes, note);
		web_sync_pulse_bar(bar);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(note_store), &iter));


	/* Get all notes since last syncRef*/
	JsonNoteList *note_list = web_sync_get_notes(user, last_sync_rev);
	last_sync_rev = note_list->latest_sync_revision;
	web_sync_pulse_bar(bar);

	int added_notes = 0;
	int changed_notes = 0;

	/* Save notes */
	GSList *notes = note_list->notes;
	while (notes != NULL) {
		ConboyNote *note = CONBOY_NOTE(notes->data);
		g_printerr("Saving: %s\n", note->title);
		/* TODO: Check for title conflicts */

		conboy_storage_note_save(app_data->storage, note);

		/* If not yet in the note store, add this note */
		if (!conboy_note_store_find_by_guid(app_data->note_store, note->guid)) {
			g_printerr("INFO: Adding note '%s' to note store\n", note->title);
			conboy_note_store_add(app_data->note_store, note, NULL);
			added_notes++;
		} else {
			g_printerr("INFO: Updating note '%s' in note store\n", note->title);
			ConboyNote *old_note = conboy_note_store_find_by_guid(app_data->note_store, note->guid);
			conboy_note_store_remove(app_data->note_store, old_note);
			conboy_note_store_add(app_data->note_store, note, NULL); /* maybe copy only content from new to old note */
			/*conboy_note_store_note_changed(app_data->note_store, note);*/
			changed_notes++;
		}

		/* Remove from list of local notes */
		/* Find local note and remove from list */
		local_notes = web_sync_remove_by_guid(local_notes, note);

		web_sync_pulse_bar(bar);
		notes = notes->next;
	}

	/*
	 * Remaining local notes are newer on the client.
	 * Send them to the server
	 */
	gint uploaded_notes = 0;

	web_sync_pulse_bar(bar);
	GError *error = NULL;
	int sync_rev = web_sync_send_notes(local_notes, user->api_ref, last_sync_rev + 1, last_sync_time, &uploaded_notes, &error);
	web_sync_pulse_bar(bar);

	gchar msg[1000];

	if (!error) {
		settings_save_last_sync_revision(sync_rev);
		settings_save_last_sync_time(time(NULL));

		g_sprintf(msg, "<b>%s</b>\n\n%s: %i\n%s: %i\n%s: %i\n%s: %i",
				"Synchonization completed",
				"Added notes", added_notes,
				"Changed notes", changed_notes,
				"Deleted notes", 0,
				"Uploaded notes", uploaded_notes);

	} else {
		g_printerr("ERROR: %s\n", error->message);
		g_sprintf(msg, "<b>Synchonization failed</b>\n\nError Message was:\n%s\n", error->message);
		g_error_free(error);
	}

	/*
	 * Sync finished
	 */
	g_free(api_ref);
	g_list_free(local_notes);
	/* TODO: Free json stuff */

	/* Show message to user */
	web_sync_show_message(data, msg);

	/* Try to get previous note */
	ConboyNote *note = NULL;
	if (guid != NULL) {
		note = conboy_note_store_find_by_guid(app_data->note_store, guid);
	}

	/* If not possible, because it was deleted or there was no previous note, get latest */
	if (note == NULL) {
		note = conboy_note_store_get_latest(app_data->note_store);
	}

	if (note == NULL) {
		/* TODO: Show demo note */
		note = conboy_note_new();
		g_printerr("ERROR: No notes to display\n");
	}

	/* Load again and make editable */
	gdk_threads_enter();
	gtk_text_view_set_editable(ui->view, TRUE);
	note_show(note, FALSE, TRUE, FALSE);
	gdk_threads_leave();
}
