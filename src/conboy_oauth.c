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

static gchar*
get_request_token_and_auth_link(const gchar *request_url, const gchar *link_url, gchar **t_key, gchar **t_secret)
{
	gchar *postarg = NULL;
	gchar *reply   = NULL;
	gchar *link = NULL;

	/*
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

	if (strstr(reply, "Expired timestamp")) {
		g_printerr("ERROR: Timestamp is expired. Probably clock running wrong.\n");
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

	/* We now don't need to add the callback here, because we provided it with the request already */
	link = g_strconcat(link_url, "?oauth_token=", *t_key, NULL);

	return link;
}




static gboolean
get_access_token(gchar *url, gchar **t_key, gchar **t_secret)
{
	gchar *reply = NULL;
	gchar *postarg = NULL;

	gchar *req_url = oauth_sign_url2(url, &postarg, OA_HMAC, "POST", c_key, c_secret, *t_key, *t_secret);

	if (req_url == NULL) {
		g_printerr("ERROR: req_url = NULL");
		return FALSE;
	}

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


/**
 * Exchanges the saved request token for a access token.
 */
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


/**
 * Gets a request token, saves it into gconf and returns an authentication
 * link. This link can be used to authenticate a person with the web service.
 *
 * TODO: Check reply. If it contains the string "expired timestamp" like in
 * "Expired timestamp: given 1268315701 and now 1268317299 has a greater difference than threshold 900"
 * return different error object/code.
 */
gchar*
conboy_get_request_token_and_auth_link(const gchar *call_url, const gchar *link_url)
{
	gchar *tok = "";
	gchar *sec = "";

	gchar *link = get_request_token_and_auth_link(call_url, link_url ,&tok, &sec);

	settings_save_oauth_access_token(tok);
	settings_save_oauth_access_secret(sec);

	return link;
}


