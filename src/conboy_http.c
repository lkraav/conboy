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
#include <curl/curl.h>
#include <oauth.h>
#include <glib.h>

#include <settings.h>
#include <conboy_http.h>

#define c_token    "anyone"  /* consumer token */
#define c_secret   "anyone"  /* consumer secret */

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
add_quotes(const gchar *param)
{
	gchar **tokens = g_strsplit(param, "=", 2);
	gchar *result = g_strconcat(tokens[0], "=\"", tokens[1], "\"", NULL);
	g_strfreev(tokens);
	return result;
}


static gchar*
make_oauth_header(const gchar *oauth_args)
{
	gchar *oauth_header = "Authorization: OAuth realm=\"Snowy\"";
	gchar **params = g_strsplit(oauth_args, "&", -1);
	int num = 0;
	while (params[num] != NULL) {
		oauth_header = g_strconcat(oauth_header, ", ", add_quotes(params[num]), NULL);
		num++;
	}
	g_strfreev(params);

	return oauth_header;
}


static gchar*
http_put(const gchar *url, const gchar *putdata, const gchar *oauth_args)
{
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.data=NULL;
	chunk.size = 0;

	curl = curl_easy_init();
	if(!curl) return NULL;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

	/* Add OAuth headers */
	gchar *oauth_header = make_oauth_header(oauth_args);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, oauth_header);

	/* Add other headers */
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept:");
	headers = curl_slist_append(headers, "User-Agent:");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

	/* Skip https security */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	/* Follow redirects */
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, putdata);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	res = curl_easy_perform(curl);
	if (res) {
		return NULL;
	}

	curl_easy_cleanup(curl);
	return (chunk.data);
}

gchar*
conboy_http_put(const gchar *url, const gchar *putdata, gboolean auth)
{
	gchar *response = NULL;

	if (auth) {
		gchar *tok = settings_load_oauth_access_token();
		gchar *sec = settings_load_oauth_access_secret();
		gchar *oauth_args = NULL;
		gchar *signed_url = oauth_sign_url2(url, &oauth_args, OA_HMAC, "PUT", c_token, c_secret, tok, sec);
		g_printerr("Request: %s\n", signed_url);
		response = http_put(signed_url, putdata, oauth_args);
		g_free(tok);
		g_free(sec);
		g_free(signed_url);
		g_free(oauth_args);
	} else {
		response = http_put(url, putdata, NULL);
	}

	return response;
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

	curl_easy_setopt(curl, CURLOPT_URL, url);

	/* Enable and disable headers to look as similar as possibele like Tomboy */
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept:");
	headers = curl_slist_append(headers, "User-Agent:");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

	/* Skip https security */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	/* Follow redirects */
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

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
http_post (const gchar *url, const gchar *postdata)
{
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.data=NULL;
	chunk.size = 0;

	curl = curl_easy_init();
	if(!curl) return NULL;
	curl_easy_setopt(curl, CURLOPT_URL, url);

	/* Skip https security */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	/* Follow redirects */
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	res = curl_easy_perform(curl);
	if (res) {
		return NULL;
	}

	curl_easy_cleanup(curl);
	return (chunk.data);
}

gchar*
conboy_http_post (const gchar *url, gchar *postdata, gboolean auth)
{
	gchar *response = NULL;

	if (auth) {
		gchar *tok = settings_load_oauth_access_token();
		gchar *sec = settings_load_oauth_access_secret();
		gchar *signed_url = oauth_sign_url2(url, &postdata, OA_HMAC, "POST", c_token, c_secret, tok, sec);
		g_printerr("Request: %s\n", signed_url);
		response = http_post(signed_url, postdata);
		g_free(tok);
		g_free(sec);
		g_free(signed_url);
	} else {
		response = http_post(url, postdata);
	}

	return response;
}



gchar*
conboy_http_get(const gchar *url, gboolean auth)
{
	gchar *response = NULL;

	if (auth) {
		gchar *tok = settings_load_oauth_access_token();
		gchar *sec = settings_load_oauth_access_secret();
		gchar *signed_url = oauth_sign_url2(url, NULL, OA_HMAC, "GET", c_token, c_secret, tok, sec);
		g_printerr("Request: %s\n", signed_url);
		response = http_get(signed_url);
		g_free(tok);
		g_free(sec);
		g_free(signed_url);
	} else {
		response = http_get(url);
	}

	return response;
}
