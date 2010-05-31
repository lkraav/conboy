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

#include <string.h>
#include <libxml/xmlreader.h>

#include "json.h"
#include "metadata.h"
#include "app_data.h"

#define JSON_NOTES "notes"
#define JSON_GUID "guid"
#define JSON_TITLE "title"
#define JSON_NOTE_CONTENT "note-content"
#define JSON_NOTE_CONTENT_VERSION "note-content-version"
#define JSON_LAST_CHANGE_DATE "last-change-date"
#define JSON_LAST_META_DATA_CHANGE_DATE "last-meta-data-change-date"
#define JSON_CREATE_DATE "create-date"
#define JSON_OPEN_ON_STARTUP "open-on-startup"
#define JSON_PINNED "pinned"
#define JSON_TAGS "tags"

/**
 * Return value must be freed
 */
static gchar*
remove_xml_tag_and_title(const gchar* content)
{
	/* Using xmlTextReaderReadInnerXml() does not work, because the <note-content> node
	 * is missing the xmlns:link and xmlns:size attributes. We could add these first,
	 * but that would mean to use a XmlTextWriter, add those attributes and then use a
	 * XmlTextReader to remove them again.
	 *
	 * So we just go with normal string processing here.
	 */
	gchar *result;

	/* Remove <note-content> tags */
	gchar **tokens1 = g_strsplit(content, ">", 2);
	gchar **tokens2 = g_strsplit(tokens1[1], "</note-content>", 2);
	gchar *without_tags = g_strdup(tokens2[0]);
	g_strfreev(tokens1);
	g_strfreev(tokens2);

	/* Remove first 2 lines */
	gchar **parts = g_strsplit(without_tags, "\n", 3);
	g_free(without_tags);

	if (parts[0] == NULL) {
		g_printerr("ERROR: PARTS0 IS NULL\n");
		return "";
	}

	/* If second line does not exist, there is no content */
	if (parts[1] == NULL) {
		result = "";
	/* If second line is empty, only use third line and rest */
	} else if (strcmp(parts[1], "") == 0) {
		result = g_strdup(parts[2]);
	/* If second line is not empty, use second line and rest */
	} else {
		result = g_strconcat(parts[1], parts[2], NULL);
	}

	g_strfreev(parts);

	if (result == NULL || strcmp(result, "") == 0) {
		return "";
	}
	return result;
}

/**
 * Escapes only \ \n \r \t \b \f \ and " and leaves unicode characters
 * as they are. g_strescape is doing "H��llo" -> "H\303\266llo" which
 * is not JSON conform.
 */
//static gchar*
//escape_string (gchar *orig)
//{
//	const guchar *p;
//	gchar *result;
//	gchar *q;
//
// 	if (orig == NULL) {
// 		return g_strdup ("\"\"");
// 	}
//
//	p = (guchar *) orig;
//	/* Each source byte needs maximally two destination chars (\n) */
//	q = result = g_malloc (strlen (orig) * 2 + 1);
//
//	while (*p)
//	{
//		switch (*p)
//		{
//		case '\n':
//			*q++ = '\\';
//			*q++ = 'n';
//			break;
//		case '\r':
//			*q++ = '\\';
//			*q++ = 'r';
//			break;
//		case '\t':
//			*q++ = '\\';
//			*q++ = 't';
//			break;
//		case '\b':
//			*q++ = '\\';
//			*q++ = 'b';
//			break;
//		case '\f':
//			*q++ = '\\';
//			*q++ = 'f';
//			break;
//		case '\\':
//			*q++ = '\\';
//			*q++ = '\\';
//			break;
//		case '\"':
//			*q++ = '\\';
//			*q++ = '"';
//			break;
//		default:
//			*q++ = *p;
//		}
//		p++;
//	}
//
//	*q = 0;
//
//	return result;
//}


static gboolean
eval_cb(const GMatchInfo *info, GString *result, gpointer data)
{
	gchar *match;
	gchar *replacement;

	match = g_match_info_fetch(info, 0);
	replacement = g_hash_table_lookup((GHashTable *)data, match);
	g_string_append(result, replacement);
	g_free (match);

	return FALSE;
}

/**
 * Replaces all occurences of &, <, >, " and ' with their
 * xml entities (e.g. &amp;).
 *
 * Returned string needs to be freed.
 */
static gchar*
escape_entities(const gchar *input)
{
	GRegex *regex;
	GHashTable *table;
	gchar *result;

	table = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(table, "&",  "&amp;");
	g_hash_table_insert(table, "<",  "&lt;");
	g_hash_table_insert(table, ">",  "&gt;");
	g_hash_table_insert(table, "\"", "&quot;");
	g_hash_table_insert(table, "'",  "&apos;");

	regex = g_regex_new("&|<|>|\"|';", G_REGEX_CASELESS, 0, NULL);
	result = g_regex_replace_eval(regex, input, -1, 0, 0, eval_cb, table, NULL);

	g_regex_unref(regex);
	g_hash_table_destroy(table);
	return result;
}

/**
 * Replaces all occurences of xml entities like &amp; &gt; etc. with their normal
 * character value (e.g. &, <, >, etc.).
 *
 * Returned string needs to be freed.
 */
static gchar*
unescape_entities(const gchar *input)
{
	GRegex *regex;
	GHashTable *table;
	gchar *result;

	table = g_hash_table_new(g_str_hash, g_str_equal);

	g_hash_table_insert(table, "&amp;", "&");
	g_hash_table_insert(table, "&lt;", "<");
	g_hash_table_insert(table, "&gt;", ">");
	g_hash_table_insert(table, "&quot;", "\"");
	g_hash_table_insert(table, "&apos;", "'");

	regex = g_regex_new("&amp;|&lt;|&gt;|&quot;|&apos;", G_REGEX_CASELESS, 0, NULL);
	result = g_regex_replace_eval(regex, input, -1, 0, 0, eval_cb, table, NULL);

	g_regex_unref(regex);
	g_hash_table_destroy(table);
	return result;
}

JsonNode*
json_get_node_from_note(ConboyNote *note)
{
	JsonNode *root;
	JsonObject *obj;
	JsonNode *node;

	root = json_node_new(JSON_NODE_OBJECT);
	obj = json_object_new();

	/*
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(node, sync_rev);
	json_object_add_member(obj, "latest-sync-revision", node);
	*/

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, note->guid);
	json_object_add_member(obj, JSON_GUID, node);

	gchar *esc_title = escape_entities(note->title);
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, esc_title);
	json_object_add_member(obj, JSON_TITLE, node);
	g_free(esc_title);

	gchar *content =  remove_xml_tag_and_title(note->content);
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, content);
	json_object_add_member(obj, JSON_NOTE_CONTENT, node);
	g_free(content);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_double(node, note->content_version);
	json_object_add_member(obj, JSON_NOTE_CONTENT_VERSION, node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->last_change_date));
	json_object_add_member(obj, JSON_LAST_CHANGE_DATE, node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->last_metadata_change_date));
	json_object_add_member(obj, JSON_LAST_META_DATA_CHANGE_DATE, node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->create_date));
	json_object_add_member(obj, JSON_CREATE_DATE, node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(node, note->open_on_startup);
	json_object_add_member(obj, JSON_OPEN_ON_STARTUP, node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(node, note->pinned);
	json_object_add_member(obj, JSON_PINNED, node);


	GList *tags = note->tags;
	JsonArray *array = json_array_new();

	while (tags != NULL) {
		gchar *tag = (gchar*)tags->data;
		node = json_node_new(JSON_NODE_VALUE);
		json_node_set_string(node, tag);
		json_array_add_element(array, node);
		tags = tags->next;
	}

	node = json_node_new(JSON_NODE_ARRAY);
	json_node_take_array(node, array);
	json_object_add_member(obj, JSON_TAGS, node);


	json_node_take_object(root, obj);

	return root;
}

gchar*
json_node_to_string(JsonNode *node, gboolean pretty)
{
	JsonGenerator *gen;
	gchar *string;
	gchar *result;

	gen = json_generator_new();
	g_object_set(gen, "pretty", pretty, NULL);
	json_generator_set_root(gen, node);
	string = json_generator_to_data(gen, NULL);

	g_object_unref(gen);
	return string;
}




/*
 * Deserialize from JSON.
 * TODO: Save api-ref and href somewhere
 * TODO: Add error checking. The passed in JsonNode could be something
 * completly unexpected.
 * TODO: Check for memory leaks
 */
ConboyNote*
json_get_note_from_node(JsonNode *node)
{
	JsonNode *member;
	GList *tags;
	ConboyNote *note = conboy_note_new();
	JsonObject *obj = json_node_get_object(node);

	/* Not sure if we need api-ref and href. If yes, we should
	 * put them somewhere else. Not into ConboyNote */

	/*
	member = json_object_get_member(obj, "api-ref");
	save_some_where;

	member = json_object_get_member(obj, "href");
	save_some_where;
	*/

	member = json_object_get_member(obj, JSON_GUID);
	if (member)	note->guid = (gchar*)json_node_dup_string(member);

	member = json_object_get_member(obj, JSON_TITLE);
	if (member) {
		gchar *title = (gchar*)json_node_dup_string(member);
		g_printerr("Received title: %s\n", title);
		note->title =  unescape_entities(title);
		g_printerr("Converted title: %s\n", note->title);
	}

	member = json_object_get_member(obj, JSON_NOTE_CONTENT);
	if (member) note->content = (gchar*)json_node_dup_string(member);

	member = json_object_get_member(obj, JSON_NOTE_CONTENT_VERSION);
	if (member) note->content_version = json_node_get_double(member);

	member = json_object_get_member(obj, JSON_LAST_CHANGE_DATE);
	if (member) note->last_change_date = get_iso8601_time_in_seconds(json_node_get_string(member));

	member = json_object_get_member(obj, JSON_LAST_META_DATA_CHANGE_DATE);
	if (member) note->last_metadata_change_date = get_iso8601_time_in_seconds(json_node_get_string(member));

	member = json_object_get_member(obj, JSON_CREATE_DATE);
	if (member) note->create_date = get_iso8601_time_in_seconds(json_node_get_string(member));

	member = json_object_get_member(obj, JSON_OPEN_ON_STARTUP);
	if (member) note->open_on_startup = json_node_get_boolean(member);

	member = json_object_get_member(obj, JSON_PINNED);
	if (member) note->pinned = json_node_get_boolean(member);

	member = json_object_get_member(obj, JSON_TAGS);
	if (member) {
		tags = json_array_get_elements(json_node_get_array(member));
		while (tags != NULL) {
			JsonNode *node = (JsonNode*)tags->data;
			conboy_note_add_tag(note, (gchar*)json_node_dup_string(node));
			tags = tags->next;
		}
		g_list_free(tags);
	}

	/*
	 * Add additional things to create a full ConboyNote
	 */

	/* Currently not specified for the JSON format. So we set it here. */
	g_object_set(note, "note-version", 0.3, NULL);

	/* The JSON format transfers the content without title, but we need
	 * the title in the first row of the content. Also we need to add the
	 * <note-content> tags */
	gdouble version;
	gchar tmp_version[10];
	gchar *tmp_content;
	gchar *tmp_title;

	g_object_get(note, "title", &tmp_title, "content", &tmp_content, "content-version", &version, NULL);
	g_ascii_formatd(tmp_version, 10, "%.1f", version);

	gchar *full_content = g_strconcat(
			"<note-content xmlns:link=\"http://beatniksoftware.com/tomboy/link\" xmlns:size=\"http://beatniksoftware.com/tomboy/size\" xmlns=\"http://beatniksoftware.com/tomboy\" ",
			"version=\"",
			tmp_version,
			"\">",
			tmp_title,
			"\n\n",
			tmp_content,
			"</note-content>",
			NULL);

	g_object_set(note, "content", full_content, NULL);

	g_free(full_content);
	g_free(tmp_content);
	g_free(tmp_title);

	return note;
}

ConboyNote*
json_get_note_from_string(const gchar *json_string)
{
	JsonParser *parser = json_parser_new();
	ConboyNote *note = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, NULL)) {
		JsonNode *root_node = json_parser_get_root(parser);
		note = json_get_note_from_node(root_node);

		g_printerr("Note Title: %s\n", note->title);
		g_printerr("Note UUID : %s\n", note->guid);

	} else {
		g_printerr("ERROR: Could not parse the following JSON string:\n%s\n", json_string);
	}

	g_object_unref(G_OBJECT(parser));

	return note;
}
/*
GSList*
json_get_notes_from_string(const gchar *json_string)
{
	JsonParser *parser = json_parser_new();
	GSList *result = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, NULL)) {
		JsonNode *root_node = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(root_node);
		JsonNode *member = json_object_get_member(obj, JSON_NOTES);
		JsonArray *array = json_node_get_array(member);

		GList *elements = json_array_get_elements(array);
		while (elements != NULL) {
			JsonNode *element = (JsonNode*)elements->data;
			Note *note = json_get_note_from_node(element);
			result = g_slist_append(result, note);
			elements = elements->next;
		}

	} else {
		g_printerr("ERROR: Could not parse the following JSON string:\n%s\n", json_string);
	}

	g_object_unref(G_OBJECT(parser));

	return result;
}
*/


void
json_test()
{
	gchar *test = "{\"notes\": [{\"note-content\": \"Bla bla bla bla\", \"open-on-startup\": false, \"last-metadata-change-date\": \"2009-07-11T11:04:38.204883-05:00\", \"tags\": [], \"title\": \"Test Note\", \"create-date\": \"2009-07-11T11:04:38.204839-05:00\", \"pinned\": false, \"last-sync-revision\": -1, \"last-change-date\": \"2009-07-11T11:04:38.204911-05:00\", \"guid\": \"0058318f-47de-4240-81f7-f846d013250b\"}], \"latest-sync-revision\": -1}";

	JsonNoteList *note_list = json_get_note_list(test);

	while (note_list->notes != NULL) {
		ConboyNote *note = (ConboyNote*) note_list->notes->data;

		g_printerr("Title: %s\n", note->title);
		g_printerr("GUID : %s\n", note->guid);
		g_printerr("------\n");

		note_list->notes = note_list->notes->next;
	}

	g_slist_free(note_list->notes);
	g_free(note_list);
}


gchar*
json_get_api_ref(const gchar* json_string)
{
	JsonParser *parser = json_parser_new();
	gchar *result = NULL;
	GError *error = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, &error)) {
		JsonNode *node = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(node);
		node = json_object_get_member(obj, "user-ref");
		obj = json_node_get_object(node);
		node = json_object_get_member(obj, "api-ref");
		result = json_node_dup_string(node);
	} else {
		if (error != NULL) g_printerr("ERROR: %s\n", error->message);
	}

	g_object_unref(G_OBJECT(parser));
	return result;
}


JsonApi*
json_get_api(const gchar* json_string)
{
	JsonParser *parser = json_parser_new();
	JsonApi *result = g_new0(JsonApi, 1);
	GError *error = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, &error)) {
		JsonNode *node = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(node);

		node = json_object_get_member(obj, "oauth_request_token_url");
		result->request_token_url = json_node_dup_string(node);

		node = json_object_get_member(obj, "oauth_access_token_url");
		result->access_token_url = json_node_dup_string(node);

		node = json_object_get_member(obj, "oauth_authorize_url");
		result->authorize_url = json_node_dup_string(node);

	} else {
		g_printerr("ERROR: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(parser);
	return result;
}


JsonUser*
json_get_user(const gchar* json_string)
{
	JsonParser *parser = json_parser_new();
	JsonUser *result = NULL;
	GError *error = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, &error)) {

		result = g_new0(JsonUser, 1);

		JsonNode *node = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(node);

		node = json_object_get_member(obj, "user-name");
		result->user_name = json_node_dup_string(node);

		node = json_object_get_member(obj, "first-name");
		result->first_name = json_node_dup_string(node);

		node = json_object_get_member(obj, "last-name");
		result->last_name = json_node_dup_string(node);

		node = json_object_get_member(obj, "latest-sync-revision");
		result->latest_sync_revision = json_node_get_int(node);

		node = json_object_get_member(obj, "current-sync-guid");
		result->current_sync_guid = json_node_dup_string(node);

		node = json_object_get_member(obj, "notes-ref");
		obj = json_node_get_object(node);
		node = json_object_get_member(obj, "api-ref");
		result->api_ref = json_node_dup_string(node);

	} else {
		g_printerr("ERROR: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(parser);
	return result;
}

JsonNoteList*
json_get_note_list(const gchar* json_string)
{
	/* First sanity checks on the input */
	if (json_string == NULL || strcmp(json_string, "") == 0) {
		g_printerr("ERROR: Cannot parse empty string\n");
		return NULL;
	}

	if (strncmp(json_string, "{\"notes\":", 9) != 0) {
		g_printerr("ERROR: Cannot parse string. Does not start with '{\"notes\":'\n");
		return NULL;
	}

	JsonParser *parser = json_parser_new();
	JsonNoteList *result = NULL;
	GError *error = NULL;

	if (json_parser_load_from_data(parser, json_string, -1, &error)) {

		result = g_new0(JsonNoteList, 1);

		JsonNode *node = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(node);

		node = json_object_get_member(obj, "latest-sync-revision");
		result->latest_sync_revision = json_node_get_int(node);

		node = json_object_get_member(obj, JSON_NOTES);
		JsonArray *array = json_node_get_array(node);

		GList *elements = json_array_get_elements(array);
		while (elements != NULL) {
			JsonNode *element = (JsonNode*)elements->data;
			ConboyNote *note = json_get_note_from_node(element);
			result->notes = g_slist_append(result->notes, note);
			elements = elements->next;
		}
	} else {
		g_printerr("ERROR: Could not parse the following JSON string:\n%s\n", json_string);
		g_printerr("ERRRO: Message is: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(G_OBJECT(parser));
	return result;
}
