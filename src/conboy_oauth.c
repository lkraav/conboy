
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gmessages.h>
#include <curl/curl.h>
#include <oauth.h>

#include "conboy_oauth.h"
#include "settings.h"
#include "note.h"
#include "json.h"

#define request_token_uri "http://127.0.0.1:8000/oauth/request_token/"
#define access_token_uri  "http://127.0.0.1:8000/oauth/access_token/"
#define get_all_notes_uri "http://127.0.0.1:8000/api/1.0/root/notes/?include_notes=true"
#define send_note_uri     "http://127.0.0.1:8000/api/1.0/root/notes/"

#define c_key    "root"  /*< consumer key */
#define c_secret "klaus" /*/< consumer secret */

#define OAUTH_USER_AGENT "liboauth-agent"

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


static size_t read_cb(char *bufptr, size_t size, size_t nitems, void *user_data) {
	
	/*
	 * bufptr: buffer to upload
	 * size*nitems: size of this buffer
	 * user_data: just some data for communication between app and callback. for us null
	 * Return: The number of bytes written to the buffer. 0 signals end of upload.
	 * 
	 * 
	 * Ok, also daten nach bufptr schreiben, aber maximal size*nitems. user_data
	 * dann wohl der input...
	 */
	
	g_printerr("Size : %i \n", size);
	g_printerr("Items: %i \n", nitems);
	g_printerr("Data:  %s \n", (char*)user_data);
	
	
	
	return 0;
	
}

/*
 * Copy of oauth_split_post_parameters
 * Only that oauth_signature is not deleted
 */
static int
split_paramters(const char *url, char ***argv, short qesc) {
  int argc=0;
  char *token, *tmp, *t1;
  if (!argv) return 0;
  if (!url) return 0;
  t1=xstrdup(url);

  /* '+' represents a space, in a URL query string */
  while ((qesc&1) && (tmp=strchr(t1,'+'))) *tmp=' ';

  tmp=t1;
  while((token=strtok(tmp,"&?"))) {
    /*if(!strncasecmp("oauth_signature=",token,16)) continue;*/    /* <<<<<<< The is the complete change */
    (*argv)=(char**) xrealloc(*argv,sizeof(char*)*(argc+1));
    while (!(qesc&2) && (tmp=strchr(token,'\001'))) *tmp='&';
    (*argv)[argc]=oauth_url_unescape(token, NULL);
    if (argc==0 && strstr(token, ":/")) {
    	/*
       HTTP does not allow empty absolute paths, so the URL 
       'http://example.com' is equivalent to 'http://example.com/' and should
       be treated as such for the purposes of OAuth signing (rfc2616, section 3.2.1)
       see http://groups.google.com/group/oauth/browse_thread/thread/c44b6f061bfd98c?hl=en
      */
      char *slash=strstr(token, ":/");
      while (slash && *(++slash) == '/'); 
#if 0
      /* skip possibly unescaped slashes in the userinfo - they're not allowed by RFC2396 but have been seen.
        the hostname/IP may only contain alphanumeric characters - so we're safe there. */
      if (slash && strchr(slash,'@')) slash=strchr(slash,'@'); 
#endif
      if (slash && !strchr(slash,'/')) {
#ifdef DEBUG_OAUTH
        fprintf(stderr, "\nliboauth: added trailing slash to URL: '%s'\n\n", token);
#endif
        free((*argv)[argc]);
        (*argv)[argc]= (char*) xmalloc(sizeof(char)*(2+strlen(token)));
        strcpy((*argv)[argc],token);
        strcat((*argv)[argc],"/");
      }
    }
    if (argc==0 && (tmp=strstr((*argv)[argc],":80/"))) {
        memmove(tmp, tmp+3, strlen(tmp+2));
    }
    tmp=NULL;
    argc++;
  }

  free(t1);
  return argc;
}

static gchar*
add_quotes(gchar *param)
{
	gchar **tokens = g_strsplit(param, "=", 2);
	gchar *result = g_strconcat(tokens[0], "=\"", tokens[1], "\"", NULL);
	return result;
}

static gchar*
http_put(const gchar *url, const gchar *post_args, const gchar *json_string)
{
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.data=NULL;
	chunk.size = 0;

	curl = curl_easy_init();
	if(!curl) return NULL;
	
	curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
	curl_easy_setopt(curl, CURLOPT_HEADER, TRUE);

	/*url = g_strconcat(url, "?", post_args, NULL);*/
	
	curl_easy_setopt(curl, CURLOPT_URL, url);

	
	g_printerr("POST_ARGS: %s \n", post_args);

	/**/
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	/*
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_PUT, 1);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb); 
	curl_easy_setopt(curl, CURLOPT_READDATA, "klaus");
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) 4);
	*/
	
	
	/* Create OAuth Header */
	gchar *oauth_header = "Authorization: OAuth realm=\"Snowy\"";
	/*gchar *oauth_header = "Authorization: ";*/
	
	/*char **params = NULL;*/
	/*int num = oauth_split_url_parameters(post_args, &params);*/
	gchar **params = g_strsplit(post_args, "&", -1); /*split_paramters(post_args, &params, FALSE);*/
	int num = 0;
	while (params[num] != NULL) {
		oauth_header = g_strconcat(oauth_header, ", ", add_quotes(params[num]), NULL);
		num++;
	}
	
	
	
	g_printerr("################### NUM: %i \n", num);
	
	/*
	int i;
	for (i = 0; i < num - 1; i++) {
		oauth_header = g_strconcat(oauth_header, add_quotes(params[i]), ",\n", NULL);
	}
	oauth_header = g_strconcat(oauth_header, add_quotes(params[num - 1]), NULL);
	*/
	
	g_printerr("XXXXX\n");
	g_printerr(">%s<\n", oauth_header);
	g_printerr("XXXXX\n");
	
	
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept:");
	headers = curl_slist_append(headers, "User-Agent:");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, oauth_header);
	
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
	
	
	
	/*
	curl_easy_setopt(curl, CURLOPT_HEADER, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HEADER, "Authorization: OAuth realm \"Snowy\"");
	*/
	/**/

	/****/
	/*
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_READDATA, (void *)p);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadMemoryCallback);
	*/
	/****/

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
	res = curl_easy_perform(curl);
	if (res) {
	  return NULL;
	}

	curl_easy_cleanup(curl);
	return (chunk.data);
}

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
    if (token)  *token =strdup(&(rv[0][12]));
    if (secret) *secret=strdup(&(rv[1][19]));
    printf("key:    '%s'\nsecret: '%s'\n",*token, *secret); 
  }
  if(rv) free(rv);
  return ok;
}


/* TODO: Improve error checking */
gchar*
get_auth_link(gchar **t_key, gchar **t_secret)
{
	gchar *postarg = NULL;
	gchar *reply   = NULL;
	gchar *link = NULL;
	
	gchar *req_url = oauth_sign_url(request_token_uri, &postarg, OA_HMAC, c_key, c_secret, NULL, NULL);
	
	if (req_url == NULL) {
		return NULL;
	}
	
	reply = oauth_http_post(req_url, postarg);
	
	if (reply == NULL) {
		g_free(req_url);
		return NULL;
	}
	 
	g_printerr("Token Reply: >%s< \n", reply);
	  
	parse_reply(reply, t_key, t_secret);
	
	g_free(req_url);
	g_free(reply);
	
	link = g_strconcat("http://localhost:8000/oauth/authenticate/?oauth_token=", *t_key, "&oauth_callback=http://www.google.de", NULL);
	
	return link;
}


gboolean
get_access_token(gchar **t_key, gchar **t_secret)
{
	gchar *reply = NULL;
	gchar *postarg = NULL;
	gchar *req_url = oauth_sign_url(access_token_uri, &postarg, OA_HMAC, c_key, c_secret, *t_key, *t_secret);
	
	if (req_url == NULL) {
		return FALSE;
	}
	
	reply = oauth_http_post(req_url, postarg);
	if (reply == NULL) {
		g_free(req_url);
		return FALSE;
	}
	g_printerr("Access Reply: >%s< \n", reply);
		  
	parse_reply(reply, t_key, t_secret);
	
	g_free(reply);
	return TRUE;
}

gchar*
get_all_notes(const gchar *t_key, const gchar *t_secret)
{
	gchar *req_url = oauth_sign_url(get_all_notes_uri, NULL, OA_HMAC, c_key, c_secret, t_key, t_secret);
	g_printerr("%s\n", req_url);
	/**/
	return "";
	/**/
	gchar *reply = oauth_http_get(req_url, NULL);
	g_printerr("Get All Notes Reply: >%s< \n", reply);
	
	return reply;
}

void
web_send_note(Note *note, const gchar *t_key, const gchar *t_secret)
{
	/*
	 * Create correct json structure to send the note
	 */
	
	JsonNode *result = json_node_new(JSON_NODE_OBJECT);
	
	JsonObject *obj = json_object_new();
	
	JsonNode *note_node = json_get_node_from_note(note);
	
	JsonArray *array = json_array_new();
	json_array_add_element(array, note_node);
	
	JsonNode *node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(node, array);
	json_object_add_member(obj, "note-changes", node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(node, 13);
	json_object_add_member(obj, "latest-sync-revision", node);
	
	json_node_take_object(result, obj);
	
	
	gchar *json_string = json_node_to_string(result, FALSE);
	
	
	
	/*gchar *json_string = "{ \"note-changes\" : [ { \"note-content\" : \"One line of super Inhalt\\nAnd another\\n\", \"tags\" : [], \"pinned\" : false, \"last-meta-data-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"guid\" : \"4621178a-5c4a-2222-a473-1bff040ea575\", \"create-date\" : \"2009-08-07T10:00:32.0000000+02:00\", \"open-on-startup\" : false, \"note-content-version\" : 0.1, \"last-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"title\" : \"New Note mit mehr Inhalt\" } ], \"latest-sync-revision\" : 12 }";*/
	/*gchar *json_string = "{ \"note-changes\" : [ { \"note-content\" : \"Bla bla bla\", \"pinned\" : false, \"last-meta-data-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"guid\" : \"4621178a-5c4a-4d2f-aaaa-1bff040ea575\", \"create-date\" : \"2009-08-07T10:00:32.0000000+02:00\", \"open-on-startup\" : false, \"note-content-version\" : 0.1, \"last-change-date\" : \"2009-08-07T10:00:45.0000000+02:00\", \"title\" : \"Tata\" } ], \"latest-sync-revision\" : 5 }";*/

	/*gchar *json_string = "{\"note-changes\":[{\"guid\":\"ffdaad13-eeee-47ec-89d2-c6c62a73227f\",\"title\":\"Very much new note\",\"note-content\":\"Describe your new note here.\",\"note-content-version\":0.1,\"last-change-date\":\"2009-08-10T16:42:43.4918810+02:00\",\"last-metadata-change-date\":\"2009-08-10T16:42:43.4918810+02:00\",\"create-date\":\"2009-08-10T16:42:37.6393970+02:00\",\"open-on-startup\":false,\"pinned\":false,\"tags\":[]}],\"latest-sync-revision\":3}";*/
	
	/*gchar *post_args = g_strconcat("bla=", json_node_to_string(result, FALSE), NULL);*/
	gchar *post_args = ""; 
	/*gchar *post_in = "bla=blub&peter=paul";*/
	
	/*gchar *uri = g_strconcat(send_note_uri, "?", post_in, NULL);*/
	gchar *uri = send_note_uri;
	
	/*
	 * Somehow the post args are gone after signing and only the 
	 * signature is left. Should it be like that? Probably not.
	 * Investigate.....
	 */
	
	
	/*
	g_printerr("XXXX\n");
	g_printerr("%s\n", post_args);
	g_printerr("XXXX\n");
	*/
	
	
	gchar *req_url = oauth_sign_url2(uri, &post_args, OA_HMAC, "PUT", c_key, c_secret, t_key, t_secret);
	/*gchar *req_url = oauth_sign_url2(uri, &post_args, OA_HMAC, "PUT", "ckey", "csec", "xxx", "yyy");*/
	
	/*
	g_printerr("AAAA\n");
	g_printerr("%s\n", post_args);
	g_printerr("AAAA\n");
	
	
	g_printerr("ZZZZ\n");
	g_printerr("%s\n", req_url);
	g_printerr("ZZZZ\n");
	*/
	
	/*gchar *reply = oauth_http_post(req_url, post_args);*/
	gchar *reply = http_put(req_url, post_args, json_string);
	/*gchar *reply = oauth_http_get(req_url, NULL);*/
	
	g_printerr("YYYY\n");
	g_printerr("%s\n", reply);
	g_printerr("YYYY\n");
	
}





