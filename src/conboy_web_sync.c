/* This file is part of Conboy.
 *
 * Copyright (C) 2010 Cornelius Hald
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

#include "localisation.h"

#include <gtk/gtk.h>
#include <tablet-browser-interface.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "settings.h"
#include "json.h"
#include "app_data.h"
#include "conboy_http.h"
#include "ui_helper.h"
#include "conboy_web_sync.h"
#include "conboy_oauth.h"


static gint
web_sync_send_notes(GList *notes, gchar *url, gint expected_rev, time_t last_sync_time, gint *uploaded_notes, GError **error)
{
	/*
	 * Create correct json structure to send the note
	 */
	JsonArray *array = json_array_new();

	GList *iter = notes;
	while (iter) {
		ConboyNote *note = CONBOY_NOTE(iter->data);
		if (note->last_metadata_change_date > last_sync_time || note->last_change_date > last_sync_time) {
			g_printerr("Will send: %s\n", note->title);
			JsonNode *note_node = json_get_node_from_note(note);
			json_array_add_element(array, note_node);
		}
		iter = iter->next;
	}

	*uploaded_notes = json_array_get_length(array);
	if (*uploaded_notes == 0) {
		g_printerr("INFO: No new notes on client. Sending nothing.\n");
		json_array_unref(array);
		return expected_rev - 1;
	}

	JsonObject *obj = json_object_new();
	JsonNode *node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(node, array);
	json_object_add_member(obj, "note-changes", node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(node, expected_rev);
	json_object_add_member(obj, "latest-sync-revision", node);

	JsonNode *result = json_node_new(JSON_NODE_OBJECT);
	json_node_take_object(result, obj);

	/* Convert to string */
	gchar *json_string = json_node_to_string(result, FALSE);

	/* Freeing the root node should also free the children */
	json_node_free(result);

	g_printerr("&&&&&&&&&&&&&&&&&&\n");
	g_printerr("%s\n", json_string);
	g_printerr("&&&&&&&&&&&&&&&&&&\n");


	gchar *reply = conboy_http_put(url, json_string, TRUE);
	g_free(json_string);
	/*
	g_printerr("Reply from Snowy:\n");
	g_printerr("%s\n", reply);
	*/

	/* Parse answer and see if expected_rev fits or not */
	JsonNoteList *note_list = json_get_note_list(reply);
	/* TODO: JsonNoteList plus containing Notes needs to be freed or unrefed */
	g_free(reply);

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
web_sync_get_notes(JsonUser *user, int since_rev, gboolean with_content)
{
	JsonNoteList *result;
	gchar *json_string;
	gchar get_all_notes_url[1024];

	g_sprintf(get_all_notes_url, "%s?include_notes=%s&since=%i", user->api_ref, with_content ? "true" : "false", since_rev);

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


/**
 * Removes a note from a list of notes
 */
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
		return g_list_remove(list, found_note);
	} else {
		return list;
	}
}

static gchar*
create_title_for_conflict_note(const gchar* orig_title)
{
	AppData *app_data = app_data_get();
	gchar *base_title = g_strconcat(orig_title, " (old)", NULL);
	gchar *result = g_strdup(base_title);

	int index = 1;
	while (conboy_note_store_find_by_title(app_data->note_store, result)) {
		g_free(result);
		result = g_strdup_printf("%s %i", base_title, index);
		index++;
	}

	g_free(base_title);
	return result;
}

/**
 * Receives all notes that have been changed on the server and merges them
 * with the local notes.
 *
 * Returns a list of local notes that are newer than the server notes
 *
 * TODO: Its not nice to have so many ref parameters. Introduce a proper
 * struct that contains all paramters and return values.
 */
static GList*
web_sync_incoming_changes(JsonUser *user, gint *last_sync_rev, time_t last_sync_time, gint *added_note_count, gint *changed_note_count)
{
	AppData *app_data = app_data_get();
	gint added_notes = 0;
	gint changed_notes = 0;

	/* Create list of all local notes */
	GList *local_notes = conboy_note_store_get_all(CONBOY_NOTE_STORE(app_data->note_store));

	/* Get all notes since last syncRev*/
	JsonNoteList *server_note_list = web_sync_get_notes(user, *last_sync_rev, TRUE);
	*last_sync_rev = server_note_list->latest_sync_revision;

	/* Save notes */
	GSList *server_notes = server_note_list->notes;
	while (server_notes != NULL) {
		ConboyNote *server_note = CONBOY_NOTE(server_notes->data);
		g_printerr("Saving: %s\n", server_note->title);
		/* TODO: Check for title conflicts */

		/* Update metadata change date and save */
		g_object_set(server_note, "metadata-change-date", time(NULL), NULL);
		conboy_storage_note_save(app_data->storage, server_note);

		/* If not yet in the note store, add this note */
		if (!conboy_note_store_find_by_guid(app_data->note_store, server_note->guid)) {
			g_printerr("INFO: Adding note '%s' to note store\n", server_note->title);
			gdk_threads_enter();
			conboy_note_store_add(app_data->note_store, server_note, NULL);
			gdk_threads_leave();
			added_notes++;
		} else {
			g_printerr("INFO: Updating note '%s' in note store\n", server_note->title);
			ConboyNote *local_note = conboy_note_store_find_by_guid(app_data->note_store, server_note->guid);

			/* Replace local note with the updated version from server */
			gdk_threads_enter();
			conboy_note_store_remove(app_data->note_store, local_note);
			conboy_note_store_add(app_data->note_store, server_note, NULL);
			gdk_threads_leave();
			changed_notes++;

			/* Compare note timestamp with last_sync_time. If it's bigger, the note has been also modified locally. */
			if (local_note->last_metadata_change_date > last_sync_time) {
				g_printerr("INFO: We have a conflict\n");

				/* TODO: Prompt user, ask for overwrite or not. If not overwrite, ask for new name */
				gboolean overwrite = TRUE;

				if (!overwrite) { /* If we do not overwrite, we need to create a copy that note */
					ConboyNote *rescue_note = conboy_note_copy(local_note);
					conboy_note_renew_guid(rescue_note);

					/* Set new title */
					gchar *rescue_note_title = create_title_for_conflict_note(local_note->title);
					conboy_note_rename(rescue_note, rescue_note_title);
					g_free(rescue_note_title);

					/* Save rescue_note */
					conboy_storage_note_save(app_data->storage, rescue_note);
					/* Add to note store */
					gdk_threads_enter();
					conboy_note_store_add(app_data->note_store, rescue_note, NULL);
					gdk_threads_leave();
					/* Add to temp list of local notes */
					local_notes = g_list_append(local_notes, rescue_note);

					added_notes++;
				}
			}
		}
		/* TODO: JsonNoteList *server_note_list needs to be freed or unrefed */

		/* Remove from list of local notes */
		local_notes = web_sync_remove_by_guid(local_notes, server_note);

		server_notes = server_notes->next;
	}

	*added_note_count = added_notes;
	*changed_note_count = changed_notes;

	return local_notes;
}

/**
 * TODO: This implementation is very naiv. It opens and reads through the file each time it is
 * called. This should probably be cached somewhere...
 */
static gboolean
note_has_been_synced_at_least_once(const gchar *guid)
{
	g_return_val_if_fail(guid != NULL, FALSE);
	g_return_val_if_fail(strcmp("", guid) != 0, FALSE);

	gchar *filename = g_strconcat(g_get_home_dir(), "/.conboy/synced_notes.txt", NULL);
	if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
		g_free(filename);
		return FALSE;
	}

	gboolean result = FALSE;
	GIOChannel *channel = g_io_channel_new_file(filename, "r", NULL);
	gchar *line = NULL;
	while (g_io_channel_read_line(channel, &line, NULL, NULL, NULL) != G_IO_STATUS_EOF) {
		if (line != NULL) {
			if (strncmp(guid, line, 36) == 0) {
				result = TRUE;
			}
			g_free(line);
		}
		if (result) break;
	}

	g_io_channel_shutdown(channel, TRUE, NULL);
	g_io_channel_unref(channel);
	g_free(filename);

	return result;
}

static void
update_synced_notes_file()
{
	AppData *app_data = app_data_get();
	gchar *filename = g_strconcat(g_get_home_dir(), "/.conboy/synced_notes.txt", NULL);

	/* Create the content of the file */
	GString *content = g_string_new("");
	GList *all_local_notes = conboy_note_store_get_all(app_data->note_store);
	GList *iter = all_local_notes;
	while (iter) {
		ConboyNote *note = CONBOY_NOTE(iter->data);
		g_string_append(content, note->guid);
		g_string_append(content, "\n");
		iter = iter->next;
	}

	/* Write the file */
	if (!g_file_set_contents(filename, content->str, -1, NULL)) {
		g_printerr("ERROR: Could not write to file: %s\n", filename);
	}

	/* Free stuff */
	g_list_free(all_local_notes);
	g_string_free(content, TRUE);
	g_free(filename);
}

static GList*
web_sync_delete_local_notes(JsonUser *user, GList *notes_to_upload, gint *deleted_notes_count)
{
	/*
	 * 1. Get since rev=0 without content
	 * 2. Compare to local notes
	 * 3. Delete local difference
	 */
	AppData *app_data = app_data_get();

	JsonNoteList *json_server_notes = web_sync_get_notes(user, 0, FALSE);
	GSList *server_notes = json_server_notes->notes;
	GList *local_notes = conboy_note_store_get_all(CONBOY_NOTE_STORE(app_data->note_store));

	/* All that is not on the server has to be deleted locally */
	/* Go though list of local notes and everything that cannot be found in the
	 * list of server notes, must be deleted locally.
	 */

	GList *local_notes_iter = local_notes;
	while (local_notes_iter != NULL) {

		ConboyNote *local_note = CONBOY_NOTE(local_notes_iter->data);

		gboolean found_note = FALSE;
		GSList *server_notes_iter = server_notes;
		while (server_notes_iter != NULL) {

			ConboyNote *server_note = CONBOY_NOTE(server_notes_iter->data);

			if (strcmp(local_note->guid, server_note->guid) == 0) {
				found_note = TRUE;
				break;
			}

			server_notes_iter = server_notes_iter->next;
		}

		/* If we couldn't find the local note on the server - delete it but only if it has been synced before */
		if (!found_note) {
			if (note_has_been_synced_at_least_once(local_note->guid)) {
				g_printerr("Deleting note: %s\n", local_note->title);
				/* Delete from filesystem etc. */
				gdk_threads_enter();
				note_delete(local_note);
				gdk_threads_leave();
				/* Remove from notes_to_update if on it */
				notes_to_upload = web_sync_remove_by_guid(notes_to_upload, local_note);
				(*deleted_notes_count)++;
			} else {
				g_printerr("INFO: Not deleting '%s', because it was never synced before\n", local_note->title);
			}
		}

		local_notes_iter = local_notes_iter->next;
	}

	g_list_free(local_notes);
	/* TODO: JsonNoteList *json_server_notes needs to be freed or unrefed */
	/* We need to make sure that memory is given back, but ConboyNotes that are now local notes are not deleted */
	//json_server_notes_free(json_server_notes);

	return notes_to_upload;
}


static gint
web_sync_send_local_deletions(gchar *url, gint expected_rev, gint *local_deletions, GError **error)
{
	gchar *content = NULL;
	gchar *filename = g_strconcat(g_get_home_dir(), "/.conboy/deleted_notes.txt", NULL);
	g_file_get_contents(filename, &content, NULL, NULL);
	if (content == NULL) {
		g_printerr("INFO: File '%s' empty or does not exist. Nothing to delete\n", filename);
		g_free(filename);
		return expected_rev - 1;
	}
	g_free(filename);

	JsonArray *array = json_array_new();
	gchar **parts = g_strsplit(content, "\n", -1);
	int i = 0;
	while (parts[i] != NULL) {
		gchar *guid = parts[i];
		if (strcmp(guid, "") != 0) {
			if (note_has_been_synced_at_least_once(guid)) {
				JsonNode *note_node = json_get_delete_node(guid);
				json_array_add_element(array, note_node);
			}
		}
		i++;
	}
	g_strfreev(parts);

	*local_deletions = json_array_get_length(array);
	if (*local_deletions == 0) {
		g_printerr("INFO: No notes deleted on client. Sending nothing.\n");
		json_array_unref(array);
		return expected_rev - 1;
	}

	JsonObject *obj = json_object_new();
	JsonNode *node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(node, array);
	json_object_add_member(obj, "note-changes", node);

	node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(node, expected_rev);
	json_object_add_member(obj, "latest-sync-revision", node);

	JsonNode *result = json_node_new(JSON_NODE_OBJECT);
	json_node_take_object(result, obj);

	/* Convert to string */
	gchar *json_string = json_node_to_string(result, FALSE);

	/* Freeing this node should also destroy the child nodes */
	json_node_free(result);

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
		g_printerr("WARN: Expected sync rev (%i) and actual sync rev (%i) are not the same\n", expected_rev, note_list->latest_sync_revision);
		//g_set_error(error, 0, 0, "Expected sync rev (%i) and actual sync rev (%i) are not the same\n", expected_rev, note_list->latest_sync_revision);
		return note_list->latest_sync_revision;
	}

	return expected_rev;
}

static void
remove_deleted_notes_file()
{
	/* TODO: This filename and "synced_notes_txt" is plastered all over this file. Again and again... */
	gchar *filename = g_strconcat(g_get_home_dir(), "/.conboy/deleted_notes.txt", NULL);
	g_unlink(filename);
	g_free(filename);
}

/* TODO: This function is way too long */
/**
 * 1.) Receive new/updated notes from server
 * 2.) Process incoming deletion
 * 3.) Send new/updated notes to server
 * 4.) Send local deletion to server (Diff zwischen gesamter Server-Version und lokaler Version. Alles was lokal fehlt, kann auf dem Server geloescht werden)
 */
gpointer
web_sync_do_sync (gpointer *user_data)
{
	WebSyncDialogData *data = (WebSyncDialogData*)user_data;
	AppData *app_data = app_data_get();
	UserInterface *ui = app_data->note_window;
	GtkProgressBar *bar = data->bar;

	/* Save and make uneditable */
	/* TODO: Function is returned often, witout making it editable again */
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
		return NULL;
	}

	web_sync_pulse_bar(bar);

	int last_sync_rev = settings_load_last_sync_revision();
	time_t last_sync_time = settings_load_last_sync_time();

	gchar *request = g_strconcat(url, "/api/1.0/", NULL);

	gchar *reply = conboy_http_get(request, TRUE); /* <<<<<<<< PROBLEM TODO: For some reason this calls fails using Snowy, but only on the device */

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: %s\n", request, NULL);
		web_sync_show_message(data, msg);
		g_free(msg);
		return NULL;
	}
	g_free(request);
	web_sync_pulse_bar(bar);

	g_printerr("Reply from /api/1.0/:: %s\n", reply);

	gchar *api_ref = json_get_api_ref(reply);
	g_free(reply);

	if (api_ref == NULL) {
		web_sync_show_message(data, "Authentication failed. Got no api_ref. Make sure your lokal clock is correct.");
		return NULL;
	}

	g_printerr("Now asking: %s\n", api_ref);

	reply = conboy_http_get(api_ref, TRUE);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: \n", api_ref, NULL);
		web_sync_show_message(data, msg);
		g_free(msg);
		g_free(api_ref);
		return NULL;
	}
	web_sync_pulse_bar(bar);
	g_free(api_ref);

	g_printerr("Reply from /user/:: %s\n", reply);

	/* Revision checks */
	JsonUser *user = json_get_user(reply);
	if (user == NULL) {
		if (g_strstr_len(reply, 30, "Subscription required")) {
			web_sync_show_message(data, "Server said: 'Subscription required'. Please check that your local time is set correctly.\n");
		} else {
			web_sync_show_message(data, "Could not parse server answer. Probably server error.");
		}
		g_free(reply);
		return NULL;
	}
	g_free(reply);

	if (user->latest_sync_revision < last_sync_rev) {
		g_printerr("Server revision older than our revision\nU1 rev: %i   Local rev: %i\n", user->latest_sync_revision, last_sync_rev);
		/* Looks like checking this does not help anything */
		//web_sync_show_message(data, "Server revision older than our revision.");
		//return;
		last_sync_rev = user->latest_sync_revision;
	}
	web_sync_pulse_bar(bar);

	g_printerr("###### Got User from server ########\n");
	g_printerr("# First name: %s\n", user->first_name);
	g_printerr("# Last name: %s\n", user->last_name);
	g_printerr("#   Api ref: %s\n", user->api_ref);
	g_printerr("####################################\n");


	/* Get notes from server */
	gint added_note_count = 0;
	gint changed_note_count = 0;
	GList *local_changes = web_sync_incoming_changes(user, &last_sync_rev, last_sync_time, &added_note_count, &changed_note_count);
	g_printerr("DEBUG: After web_sync_incoming_changes\n");
	web_sync_pulse_bar(bar);

	/*
	 * Process deletions made on server
	 * Notes that get deleted get also deleted from local_changes, to
	 * make sure we don't upload them again.
	 */
	gint deleted_note_count = 0;
	local_changes = web_sync_delete_local_notes(user, local_changes, &deleted_note_count);
	web_sync_pulse_bar(bar);

	/*
	 * Remaining local notes are newer on the client.
	 * Send them to the server
	 */
	gint uploaded_notes = 0;

	GError *error = NULL;
	int sync_rev = web_sync_send_notes(local_changes, user->api_ref, last_sync_rev + 1, last_sync_time, &uploaded_notes, &error);
	web_sync_pulse_bar(bar);
	g_list_free(local_changes);

	gint deleted_on_server_count = 0;
	if (!error) {
		/*
		 * Now that we updated notes on both sides, we can send local deletions
		 * to the server.
		 */
		sync_rev = web_sync_send_local_deletions(user->api_ref, sync_rev + 1, &deleted_on_server_count, &error);
		web_sync_pulse_bar(bar);
	}

	json_user_free(user);

	gchar msg[1000];

	if (!error) {
		/* Store new sync_rev and sync_time */
		settings_save_last_sync_revision(sync_rev);
		settings_save_last_sync_time(time(NULL));

		/* Update synced_notes and deleted_notes file */
		update_synced_notes_file();
		remove_deleted_notes_file();

		g_sprintf(msg, "<b>%s</b>\n\n%s: %i\n%s: %i\n%s: %i\n%s: %i\n%s: %i",
				"Synchonization completed",
				"Added notes", added_note_count,
				"Changed notes", changed_note_count,
				"Deleted notes", deleted_note_count,
				"Uploaded notes", uploaded_notes,
				"Deleted on server", deleted_on_server_count);

	} else {
		g_printerr("ERROR: %s\n", error->message);
		g_sprintf(msg, "<b>Synchonization failed</b>\n\nError Message was:\n%s\n", error->message);
		g_error_free(error);
	}

	/*
	 * Sync finished
	 */

	/* Show message to user */
	web_sync_show_message(data, msg);

	/* Try to get previous note */
	ConboyNote *note = NULL;
	if (guid != NULL) {
		note = conboy_note_store_find_by_guid(app_data->note_store, guid);
	}

	/* If not possible, because it was deleted or there was no previous note, get latest */
	if (note == NULL) {
		note = conboy_note_store_get_latest_or_new(app_data->note_store);
	}

	/* Load again and make editable */
	gdk_threads_enter();
	gtk_text_view_set_editable(ui->view, TRUE);
	note_show(note, FALSE, TRUE, FALSE);
	gdk_threads_leave();

	return NULL;
}

struct AuthDialogData {
	GtkDialog *dialog;
	gchar *verifier;
};

/**
 * Extracts the oauth verifier from a http GET string. E.g.
 * GET /bla/blub?key=val&key2=val2&oauth_verifier=abcdefg&key3=val3 HTTP/1.0
 */
/*
static gchar*
extract_verifier (gchar* http_get_string)
{
	gchar **parts = g_strsplit(http_get_string, " ", 3); // Cut away the GET and HTTP/1.0 parts
	gchar **parameters = g_strsplit_set(parts[1], "?&", 5);
	gchar *verifier = NULL;
	int i = 0;

	while (parameters[i] != NULL) {
		if (strncmp(parameters[i], "oauth_verifier=", 15) == 0) {
			verifier = g_strdup(&(parameters[i][15])); // Copy starting from character 15
			break;
		}
		i++;
	}

	g_strfreev(parameters);
	g_strfreev(parts);
	return verifier;
}
*/

/**
 * Extracts the oauth verifier and the redirect url from a http GET string. E.g.
 * GET /bla/blub?return=http://one.ubuntu.com&key2=val2&oauth_verifier=abcdefg&key3=val3 HTTP/1.0
 */
static void
extract_verifier_and_redirect_url (gchar* http_get_string, gchar** verifier, gchar** url)
{
	gchar **parts = g_strsplit(http_get_string, " ", 3); /* Cut away the GET and HTTP/1.0 parts */
	gchar **parameters = g_strsplit_set(parts[1], "?&", 5);
	if (verifier) *verifier = NULL;
	if (url) *url = NULL;
	int i = 0;

	while (parameters[i] != NULL) {
		if (verifier && strncmp(parameters[i], "oauth_verifier=", 15) == 0) {
			*verifier = g_strdup(&(parameters[i][15])); /* Copy starting from character 15 */
		}
		if (url && strncmp(parameters[i], "return=", 7) == 0) {
			*url = g_strdup(&(parameters[i][7])); /* Copy starting from character 7 */
		}
		i++;
	}

	g_strfreev(parameters);
	g_strfreev(parts);
}

/**
 * Opens a socket on localhost:14680 and blocks until a client connects.
 * Returns FALSE if an error occured, TRUE otherwise.
 */
static gboolean
open_server_socket(int *l_sock, int *sock)
{
	*l_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*l_sock < 0) {
		g_printerr("ERROR: Cannot open listening socket\n");
		return FALSE;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(14680);

	if (bind(*l_sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		g_printerr("ERROR: Cannot bind socket\n");
		return FALSE;
	}

	listen(*l_sock, 1);

	int sin_size = sizeof(struct sockaddr_in);

	/* The following accept() call blocks until a client connects */
	*sock = accept(*l_sock, (struct sockaddr *) &address, &sin_size);
	if (sock < 0) {
		g_printerr("ERROR: Cannot open SOCKET\n");
		return FALSE;
	}

	return TRUE;
}

static gpointer
oauth_callback_handler(gpointer user_data)
{
	struct AuthDialogData *data = (struct AuthDialogData*) user_data;
	GtkWidget *dialog = GTK_WIDGET(data->dialog);
	gchar *oauth_verifier = NULL;
	gchar *redirect_url = NULL;

	/* Open socket */
	int l_sock, sock;
	if (!open_server_socket(&l_sock, &sock)) {
		g_printerr("ERROR Cannot open socket\n");
		return NULL;
	}

	/* Read from socket */
	GIOChannel *channel = g_io_channel_unix_new(sock);
	gchar *buf;

	/* TODO: If/Else chaos. Clean up */
	g_io_channel_read_line(channel, &buf, NULL, NULL, NULL);
	g_printerr("Read: %s\n", buf);
	if (g_str_has_prefix(buf, "GET")) {
		extract_verifier_and_redirect_url(buf, &oauth_verifier, &redirect_url);
		/* Send reply to the browser to redirect it */
		if (redirect_url) {
			g_printerr("Redirecting to: %s\n", redirect_url);
			gchar *msg = g_strconcat("HTTP/1.1 302 Found\nStatus: 301 Found\nLocation: ", redirect_url, "\n\n", NULL);
			g_io_channel_write_chars(channel, msg, -1, NULL, NULL);
			g_free(msg);
		} else {
			g_printerr("Did not get redirect_url\n");
		}
	} else if (g_str_has_prefix(buf, "KILL")) {
		/* This happens if the user manually canceled, then we don't need to manipulate
		 * the dialog, because the dialog does not exist anymore.*/
		g_printerr("INFO: Received KILL, shuting down this thread\n");
		/* Close socket */
		g_io_channel_shutdown(channel, TRUE, NULL);
		close(sock);
		close(l_sock);
		g_free(buf);
		return NULL;
	} else {
		g_printerr("ERROR: First line did not start with 'GET', maybe we need to read more lines?\n");
		g_free(buf);
		return NULL;
	}

	g_free(buf);

	/* Close socket */
	g_io_channel_shutdown(channel, TRUE, NULL);
	close(sock);
	close(l_sock);

	/* Present the UI */
	gdk_threads_enter();
	gtk_window_present(GTK_WINDOW(dialog));
	gdk_threads_leave();

	if (oauth_verifier != NULL) {
		/* Close the modal dialog and signal success */
		g_printerr("OAuth Verifier: %s\n", oauth_verifier);
		data->verifier = oauth_verifier;
		gdk_threads_enter();
		g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_OK);
		gdk_threads_leave();

	} else {
		/* Close the modal dialog and signal failure */
		gdk_threads_enter();
		g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_REJECT);
		gdk_threads_leave();
	}

	return NULL;
}

typedef struct {
	int sock;
	int socket_fd;
	struct sockaddr_in socket_addr;
	int sin_size;
} SocketData;

static gboolean
open_client_socket(int *sock)
{
	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock < 0) {
		g_printerr("ERROR: Cannot open listening socket\n");
		return FALSE;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(14680);

	if (connect(*sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		g_printerr("ERROR: Cannot connect to socket\n");
		return FALSE;
	}

	return TRUE;
}

static void
kill_callback_thread()
{
	/* Open socket */
	int sock;
	if (!open_client_socket(&sock)) {
		g_printerr("ERROR: Cannot open client socket\n");
		return;
	}

	/* Write to socket */
	GIOChannel *channel = g_io_channel_unix_new(sock);
	g_io_channel_write_chars(channel, "KILL", -1, NULL, NULL);

	/* Close socket */
	g_io_channel_shutdown(channel, TRUE, NULL);
	close(sock);
}

gboolean
web_sync_authenticate(const gchar *url, GtkWindow *parent)
{
	/* Get URLs */
	gchar *request = g_strconcat(url, "/api/1.0", NULL);

	gchar *reply = conboy_http_get(request, TRUE);
	g_printerr("Reply:\n%s\n", reply);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: ", request, "\n", NULL);
		ui_helper_show_confirmation_dialog(parent, msg, FALSE);
		g_free(msg);
		g_free(request);
		return FALSE;
	}
	g_free(request);

	JsonApi *api = json_get_api(reply);
	g_free(reply);

	/*g_printerr("###################################\n");
	g_printerr("Request token url: %s\n", api->request_token_url);
	g_printerr("Authenticate url : %s\n", api->authorize_url);
	g_printerr("Access token url : %s\n", api->access_token_url);
	g_printerr("###################################\n");*/

	/* Get auth link url */
	GError *error = NULL;
	gchar *link = conboy_get_request_token_and_auth_link(api->request_token_url, api->authorize_url, &error);

	if (link == NULL) {
		if (error == NULL) {
			ui_helper_show_confirmation_dialog(parent, "Could not connect to host.", FALSE);
		} else {
			gchar *msg = g_strconcat("Could not connect to host.\nReason: ", error->message, NULL);
			ui_helper_show_confirmation_dialog(parent, msg, FALSE);
			g_free(msg);
		}
		json_api_free(api);
		return FALSE;
	}

	/* Open link in browser */
	AppData *app_data = app_data_get();
	osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_browser",
			OSSO_BROWSER_OPEN_NEW_WINDOW_REQ, NULL,
			DBUS_TYPE_STRING, link, DBUS_TYPE_INVALID);
	g_printerr("Opening browser with URL: >%s<\n", link);
	g_free(link);

	GtkWidget *dialog = ui_helper_create_cancel_dialog(parent, "Please grant access on the website of your service provider that just opened.\nAfter that you will be automatically redirected back to Conboy.");

	struct AuthDialogData auth_data;
	auth_data.dialog = GTK_DIALOG(dialog);

	/* Create thread that listens to oauth callbacks */
	GThread *thread = g_thread_create((GThreadFunc)oauth_callback_handler, &auth_data, FALSE, NULL);
	if (!thread) {
		g_printerr("ERROR: Cannot create socket thread\n");
		json_api_free(api);
		return FALSE;
	}

	/* Open dialog and wait for result */
	int result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	gtk_widget_destroy(dialog);

	/* Handle return values of the dialog */
	if (result == GTK_RESPONSE_OK) {

		GtkWidget *wait_dialog = gtk_dialog_new();
		gtk_window_set_title(GTK_WINDOW(wait_dialog), "Connecting to server");
		#ifdef HILDON_HAS_APP_MENU
		hildon_gtk_window_set_progress_indicator(GTK_WINDOW(wait_dialog), TRUE);
		#endif
		gtk_window_set_modal(GTK_WINDOW(wait_dialog), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(wait_dialog), parent);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(wait_dialog), TRUE);
		gtk_widget_show_all(wait_dialog);
		while (gtk_events_pending()) {
			gtk_main_iteration_do(FALSE);
		}

		if (conboy_get_access_token(api->access_token_url, auth_data.verifier)) {
			gtk_widget_destroy(wait_dialog);
			ui_helper_show_confirmation_dialog(parent, "<b>You are successfully authenticated</b>\nYou can now use the synchronization from the main menu.", FALSE);
			settings_save_sync_base_url(url);
			/* Everything is good */
			json_api_free(api);
			return TRUE;
		}

		json_api_free(api);

		/* We did not get the access token */
		gtk_widget_destroy(wait_dialog);
		ui_helper_show_confirmation_dialog(parent, "Conboy could not get an access token from your service provider. Please try again.", FALSE);
		settings_save_sync_base_url("");
		return FALSE;
	}

	if (result == GTK_RESPONSE_REJECT) {
		ui_helper_show_confirmation_dialog(parent, "There were problems with the data received from your service provider. Please try again.", FALSE);
		settings_save_sync_base_url("");
		return FALSE;
	}

	if (result == 666 || result == -4) {
		kill_callback_thread();
		ui_helper_show_confirmation_dialog(parent, "You have manually canceled the authentication process.", FALSE);
		return FALSE;
	}

	/* Some other error */
	kill_callback_thread();
	ui_helper_show_confirmation_dialog(parent, "An unknown error occured, please try again.", FALSE);
	return FALSE;
}

