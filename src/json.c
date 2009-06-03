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

#include "json.h"
#include "metadata.h"

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


JsonNode* get_json_object_from_note(Note *note)
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
	
	/* TODO:
	 * - Implement note_get_content() which returns the content XML
	 * - It should read note->content, and if it is NULL, read it from disk
	 * - Normal saving should update note->content
	 */
	/*
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, note->content);
	json_object_add_member(obj, JSON_NOTE_CONTENT, node);
	*/
	
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
	
	/*
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(node, note->pinned);
	json_object_add_member(obj, JSON_PINNED, node);
	*/
	
	if (note->tags != NULL) {
		
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
	}
	
	json_node_take_object(root, obj);
	
	return root;
}

void print_note_as_json(Note *note)
{
	JsonNode *obj;
	JsonGenerator *gen;
	gchar *string;
	
	obj = get_json_object_from_note(note);
	
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
Note* get_note_from_json_object(JsonNode *node)
{
	JsonNode *member;
	GList *tags;
	Note *note = note_create_new();
	JsonObject *obj = json_node_get_object(node);
	
	/*
	member = json_object_get_member(obj, "api-ref");
	save_some_where;
	
	member = json_object_get_member(obj, "href");
	save_some_where;
	*/
	
	
	member = json_object_get_member(obj, JSON_GUID);
	note->guid = (gchar*)json_node_get_string(member);
	
	member = json_object_get_member(obj, JSON_TITLE);
	note->title = (gchar*)json_node_get_string(member);
	
	/*
	member = json_object_get_member(obj, JSON_NOTE_CONTENT);
	note->note_content = json_node_get_string(member);
	*/
	
	member = json_object_get_member(obj, JSON_NOTE_CONTENT_VERSION);
	note->content_version = json_node_get_double(member);
	
	member = json_object_get_member(obj, JSON_LAST_CHANGE_DATE);
	note->last_change_date = get_iso8601_time_in_seconds(json_node_get_string(member));
	
	member = json_object_get_member(obj, JSON_LAST_META_DATA_CHANGE_DATE);
	note->last_metadata_change_date = get_iso8601_time_in_seconds(json_node_get_string(member));
	
	member = json_object_get_member(obj, JSON_CREATE_DATE);
	note->create_date = get_iso8601_time_in_seconds(json_node_get_string(member));
	
	member = json_object_get_member(obj, JSON_OPEN_ON_STARTUP);
	note->open_on_startup = json_node_get_boolean(member);
	
	/*
	member = json_object_get_member(obj, JSON_PINNED);
	note->pinned = json_node_get_boolean(member);
	*/
	
	member = json_object_get_member(obj, JSON_TAGS);
	if (member != NULL) {
		tags = json_array_get_elements(json_node_get_array(member));
		while (tags != NULL) {
			JsonNode *node = (JsonNode*)tags->data;
			note_add_tag(note, (gchar*)json_node_get_string(node));
			tags = tags->next;
		}
		g_list_free(tags);
	}
	
	/* Currently not specified for the JSON format. So we set it here. */
	note->version = 0.3;
	
	return note;

}
