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


static gchar*
remove_xml_tag_and_title(const gchar* content)
{
	
	AppData *app_data = app_data_get();
	
	if (app_data->reader == NULL) {
		app_data->reader = xmlReaderForMemory(content, strlen(content), "", "UTF-8", 0);
	}
	
	if (xmlReaderNewMemory(app_data->reader, content, strlen(content), "", "UTF-8", 0) != 0) {
		g_printerr("ERROR: Cannot reuse xml parser. \n");
		g_assert_not_reached();
	}
	
	/* Remove xml tags */
	xmlTextReaderRead(app_data->reader);
	gchar *result = xmlTextReaderReadInnerXml(app_data->reader);
	
	/* Remove first 2 lines */
	gchar **parts = g_strsplit(result, "\n", 3);
	
	/*
	g_printerr("###: >%s<\n", parts[0]);
	g_printerr("###: >%s<\n", parts[1]);
	g_printerr("###: >%s<\n", parts[2]);
	*/
	
	g_free(result);
	
	if (strcmp(parts[1], "") == 0) {
		result = g_strdup(parts[2]);
	} else {
		result = g_strconcat(parts[1], parts[2], NULL);
	}
	
	g_strfreev(parts);
	
	g_printerr("ZZZZZZZ:\n>%s<\n", result);
	
	return result;
}

static gchar*
convert_content(const gchar *content)
{
	gchar *tmp = remove_xml_tag_and_title(content);
	gchar *result = g_strescape(tmp, NULL);
	g_free(tmp);
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
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, note->title);
	json_object_add_member(obj, JSON_TITLE, node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, convert_content(note->content));
	json_object_add_member(obj, JSON_NOTE_CONTENT, node);
	
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
	/*json_node_set_boolean(node, note->pinned);*/
	json_node_set_boolean(node, FALSE); /* TODO: Implement note->pinned */
	json_object_add_member(obj, JSON_PINNED, node);
	
	/*if (note->tags != NULL) {*/
		
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
	/*}*/
	
	json_node_take_object(root, obj);
	
	return root;
}


gchar*
json_node_to_string(JsonNode *node, gboolean pretty)
{
	JsonGenerator *gen;
	gchar *string;
		
	gen = json_generator_new();
	if (pretty) {
		g_object_set(gen, "pretty", TRUE, NULL);
	}
	json_generator_set_root(gen, node);
	string = json_generator_to_data(gen, NULL);
	
	g_object_unref(gen);
	
	return string;
}

void
json_print_note(ConboyNote *note)
{
	/*
	 * TODO: Use json_node_to_string()
	 */
	
	JsonNode *obj;
	JsonGenerator *gen;
	gchar *string;
	
	obj = json_get_node_from_note(note);
	
	gen = json_generator_new();
	g_object_set(gen, "pretty", TRUE, NULL);
	json_generator_set_root(gen, obj);
	string = json_generator_to_data(gen, NULL);
	
	g_printerr("%s", string);
	
	g_free(string);
	json_node_free(obj);
	g_object_unref(gen);
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
	
	/*
	member = json_object_get_member(obj, "api-ref");
	save_some_where;
	
	member = json_object_get_member(obj, "href");
	save_some_where;
	*/
	
	member = json_object_get_member(obj, JSON_GUID);
	if (member)	note->guid = (gchar*)json_node_dup_string(member);
	
	member = json_object_get_member(obj, JSON_TITLE);
	if (member) note->title = (gchar*)json_node_dup_string(member);
	
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
	
	/*
	member = json_object_get_member(obj, JSON_PINNED);
	if (member) note->pinned = json_node_get_boolean(member);
	*/
	
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
	
	/* Currently not specified for the JSON format. So we set it here. */
	note->note_version = 0.3;
	
	return note;
}

ConboyNote*
json_get_note_from_string(const gchar *json_string)
{
	JsonParser *parser = json_parser_new();
	Note *note = NULL;
	
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
		Note *note = (Note*) note_list->notes->data;
		
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



JsonUser*
json_get_user(const gchar* json_string)
{
	JsonParser *parser = json_parser_new();
	JsonUser *result = g_new0(JsonUser, 1);
	GError *error = NULL;
	
	if (json_parser_load_from_data(parser, json_string, -1, &error)) {
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
		if (error != NULL) g_printerr("ERROR: %s\n", error->message);
	}
	
	g_object_unref(G_OBJECT(parser));
	return result;
}

JsonNoteList*
json_get_note_list(const gchar* json_string)
{
	JsonParser *parser = json_parser_new();
	JsonNoteList *result = g_new0(JsonNoteList, 1);
	GError *error = NULL;
	
	if (json_parser_load_from_data(parser, json_string, -1, &error)) {
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
	}
	
	g_object_unref(G_OBJECT(parser));
	return result;
}
