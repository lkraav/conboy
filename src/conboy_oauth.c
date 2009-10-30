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


#define c_key    "anyone"  /*< consumer key */
#define c_secret "anyone" /*/< consumer secret */


struct MemoryStruct {
  char *data;
  size_t size;
};

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->data = (char *)xrealloc(mem->data, mem->size + realsize + 1);
  if (mem->data) {
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
  }
  return realsize;
}


static gchar*
add_quotes(gchar *param)
{
	gchar **tokens = g_strsplit(param, "=", 2);
	gchar *result = g_strconcat(tokens[0], "=\"", tokens[1], "\"", NULL);
	return result;
}

static gchar*
http_put(const gchar *url, const gchar *oauth_args, const gchar *body)
{
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.data=NULL;
	chunk.size = 0;

	curl = curl_easy_init();
	if(!curl) return NULL;

	/* Enable debugging */
	/*
	curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
	curl_easy_setopt(curl, CURLOPT_HEADER, TRUE);
	*/

	/* We make the request like POST, but put PUT into the header.
	 * It's easier to do with libcurl and it works. */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

	/* Create OAuth Header */
	gchar *oauth_header = "Authorization: OAuth realm=\"Snowy\"";
	gchar **params = g_strsplit(oauth_args, "&", -1);
	int num = 0;
	while (params[num] != NULL) {
		oauth_header = g_strconcat(oauth_header, ", ", add_quotes(params[num]), NULL);
		num++;
	}

	/* Enable and disable headers to look as similar as possibele like Tomboy */
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, oauth_header);
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept:");
	headers = curl_slist_append(headers, "User-Agent:");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

	/* Set the json string as the body of the request */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	/* Setup function to read the reply */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* Perfome the request */
	res = curl_easy_perform(curl);
	if (res) {
	  return NULL;
	}

	curl_easy_cleanup(curl);
	return (chunk.data);
}



static gchar*
http_get(const gchar *url)
{
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.data=NULL;
	chunk.size = 0;

	curl = curl_easy_init();
	if(!curl) return NULL;

	/* Enable debugging */
	/*
	curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
	curl_easy_setopt(curl, CURLOPT_HEADER, TRUE);
	*/

	curl_easy_setopt(curl, CURLOPT_URL, url);

	/* Enable and disable headers to look as similar as possibele like Tomboy */
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept:");
	headers = curl_slist_append(headers, "User-Agent:");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

	/* Setup function to read the reply */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* Perfome the request */
	res = curl_easy_perform(curl);
	if (res) {
	  return NULL;
	}

	curl_easy_cleanup(curl);
	return (chunk.data);
}



/*
 * Returns 0 on success, 1 otherwise
 */
static int parse_reply(const char *reply, char **token, char **secret) {
  int rc;
  int ok=1;
  char **rv = NULL;
  rc = oauth_split_url_parameters(reply, &rv);
  qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
  if( rc==2
      && !strncmp(rv[0],"oauth_token=",11)
      && !strncmp(rv[1],"oauth_token_secret=",18) ) {
    ok=0;
    if (token)  *token = g_strdup(&(rv[0][12]));
    if (secret) *secret = g_strdup(&(rv[1][19]));
    printf("key:    '%s'\nsecret: '%s'\n",*token, *secret);
  }
  if(rv) free(rv);
  return ok;
}

gchar*
conboy_get_auth_link(const gchar *base_url)
{
	gchar *tok = "";
	gchar *sec = "";

	gchar *call_url = g_strconcat(base_url, "/oauth/request_token/", NULL);
	gchar *link_url = g_strconcat(base_url, "/oauth/authenticate/", NULL);

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

	gchar *req_url = oauth_sign_url2(request_url, &postarg, OA_HMAC, "POST", c_key, c_secret, NULL, NULL);

	if (req_url == NULL) {
		g_printerr("ERROR: REQ URL = NULL\n");
		return NULL;
	}

	reply = oauth_http_post(req_url, postarg);

	g_printerr("Reply: %s\n", reply);

	if (reply == NULL) {
		g_printerr("ERROR: Reply = NULL\n");
		g_free(req_url);
		return NULL;
	}

	if (strlen(reply) > 150) {
		g_printerr("ERROR: Reply is longer then 150 characters, cannot be right\n");
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
	link = g_strconcat(link_url, "?oauth_token=", *t_key, "&oauth_callback=conboy://", NULL);
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

	reply = oauth_http_post(req_url, postarg);
	if (reply == NULL) {
		g_printerr("ERROR: reply = NULL");
		g_free(req_url);
		return FALSE;
	}

	g_printerr("Access Reply: >%s< \n", reply);

	if (strlen(reply) > 150) {
		/* Answer is too long, cannot be correct */
		g_printerr("WARN: Cannot get access token. Answer of server was longer than 150 characters.");
		g_free(reply);
		return FALSE;
	}

	if (parse_reply(reply, t_key, t_secret)) {
		g_free(reply);
		return FALSE;
	} else {
		g_free(reply);
		return TRUE;
	}
}

gboolean
conboy_get_access_token() {

	gchar *tok = settings_load_oauth_access_token();
	gchar *sec = settings_load_oauth_access_secret();
	gchar *url = settings_load_sync_base_url();

	url = g_strconcat(url, "/oauth/access_token/", NULL);

	if (get_access_token(url, &tok, &sec)) {
		g_printerr("acc_tok: %s\n", tok);
		g_printerr("acc_sec: %s\n", sec);
		settings_save_oauth_access_token(tok);
		settings_save_oauth_access_secret(sec);
		return TRUE;
	}

	return FALSE;
}

/*
 * Works
 */
gchar*
conboy_http_get(const gchar *url) {

	gchar *tok = settings_load_oauth_access_token();
	gchar *sec = settings_load_oauth_access_secret();

	gchar *req_url = oauth_sign_url2(url, NULL, OA_HMAC, "GET", c_key, c_secret, tok, sec);
	g_printerr("Request: %s\n", req_url);
	gchar *reply = http_get(req_url);

	g_free(tok);
	g_free(sec);
	g_free(req_url);

	return reply;
}


gint
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


	/*gchar *json_string = "{ \"note-changes\" : [ { \"note-content\" : \"One line of super Inhalt\\nAnd another\\n\", \"tags\" : [], \"pinned\" : false, \"last-meta-data-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"guid\" : \"4621178a-5c4a-2222-a473-1bff040ea575\", \"create-date\" : \"2009-08-07T10:00:32.0000000+02:00\", \"open-on-startup\" : false, \"note-content-version\" : 0.1, \"last-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"title\" : \"New Note mit mehr Inhalt\" } ], \"latest-sync-revision\" : 12 }";*/

	gchar *oauth_args = "";

	/*gchar *uri = g_strconcat(base_url, "/api/1.0/root/notes/", NULL); */


	gchar *req_url = oauth_sign_url2(url, &oauth_args, OA_HMAC, "PUT", c_key, c_secret, t_key, t_secret);

	gchar *reply = http_put(req_url, oauth_args, json_string);


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

JsonNoteList*
web_sync_get_notes(JsonUser *user, int since_rev)
{
	JsonNoteList *result;
	gchar *json_string;
	gchar get_all_notes_url[1024];

	g_sprintf(get_all_notes_url, "%s?include_notes=true&since=%i", user->api_ref, since_rev);

	json_string = conboy_http_get(get_all_notes_url);
	result = json_get_note_list(json_string);

	g_free(json_string);
	return result;
}

