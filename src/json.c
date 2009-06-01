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
	json_object_add_member(obj, "guid", node);
	
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, note->title);
	json_object_add_member(obj, "title", node);
	
	/* TODO:
	 * - Implement note_get_content() which returns the content XML
	 * - It should read note->content, and if it is NULL, read it from disk
	 * - Normal saving should update note->content
	 */
	/*
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, note->content);
	json_object_add_member(obj, "note-content", node);
	*/
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_double(node, note->content_version);
	json_object_add_member(obj, "note-content-version", node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->last_change_date));
	json_object_add_member(obj, "last-change-date", node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->last_metadata_change_date));
	json_object_add_member(obj, "last-meta-data-change-date", node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(node, get_time_in_seconds_as_iso8601(note->create_date));
	json_object_add_member(obj, "create-date", node);
	
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(node, note->open_on_startup);
	json_object_add_member(obj, "open-on-startup", node);
	
	/*
	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(node, note->pinned);
	json_object_add_member(obj, "pinned", node);
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
		json_object_add_member(obj, "tags", node);
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


/* TODO:
 * Deserialize from JSON.
 */
Note* get_note_from_json_object(JsonObject *json)
{
	return NULL;
}
