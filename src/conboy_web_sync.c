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

#include <gtk/gtk.h>
#include <tablet-browser-interface.h>

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


/* TODO: This function is way too long */
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
			conboy_note_store_add(app_data->note_store, note, NULL);
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


struct AuthDialogData {
	GtkDialog *dialog;
	gchar *verifier;
};


/**
 * This function is called, whenever Maemo tries to open a conboy:// URL.
 * If the URL is of the form conboy://authenticated, we try to extract the oauth
 * verifier and save it.
 */
static gint
url_callback_handler(const gchar *interface, const gchar *method, GArray *arguments, gpointer user_data, osso_rpc_t *retval)
{
	g_printerr("Method: %s\n", method);
	struct AuthDialogData *data = (struct AuthDialogData*) user_data;

	GtkWidget *dialog = GTK_WIDGET(data->dialog);
	gtk_window_present(GTK_WINDOW(dialog));

	if (g_strcasecmp(method, "authenticated") == 0) {

		/*
		 * The parameter looks like this:
		 * conboy://authenticate?oauth_token=kBFjXzLsKqzmxx9PGBX0&oauth_verifier=1ccaf32e-ec6e-4598-a77f-020af60f24b5&return=https://one.ubuntu.com
		 */
		g_printerr("___ CORRECTLY AUTHENTICATED ____\n");

		if (arguments != NULL && arguments->len >= 1) {

			osso_rpc_t value = g_array_index(arguments, osso_rpc_t, 0);

			if (value.type == DBUS_TYPE_STRING) {
				gchar *url = value.value.s;
				g_printerr("URL: %s\n", url);

				/* Find out verifier */
				gchar **parts = g_strsplit_set(url, "?&", 4);

				gchar *verifier = NULL;
				int i = 0;
				while (parts[i] != NULL) {

					if (strncmp(parts[i], "oauth_verifier=", 15) == 0) {
						verifier = g_strdup(&(parts[i][15])); /* Copy starting from character 15 */
						break;
					}
					i++;
				}

				g_strfreev(parts);

				if (verifier != NULL) {
					g_printerr("OAuth Verifier: %s\n", verifier);
					data->verifier = verifier;
					/* Close the modal dialog and signal success */
					g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_OK);
					return OSSO_OK;
				}
			}
		}
	}

	/* Close the modal dialog and signal failure */
	g_signal_emit_by_name(data->dialog, "response", GTK_RESPONSE_REJECT);
	return OSSO_OK;
}

gboolean
web_sync_authenticate(gchar *url, GtkWindow *parent)
{
	/* Get URLs */
	gchar *request = g_strconcat(url, "/api/1.0", NULL);

	gchar *reply = conboy_http_get(request, FALSE);

	if (reply == NULL) {
		gchar *msg = g_strconcat("Got no reply from: ", request, "\n", NULL);
		ui_helper_show_confirmation_dialog(parent, msg, FALSE);
		g_free(msg);
		g_free(request);
		return FALSE;
	}
	g_free(request);

	JsonApi *api = json_get_api(reply);

	/*g_printerr("###################################\n");
	g_printerr("Request token url: %s\n", api->request_token_url);
	g_printerr("Authenticate url : %s\n", api->authorize_url);
	g_printerr("Access token url : %s\n", api->access_token_url);
	g_printerr("###################################\n");*/

	/* Get auth link url */
	gchar *link = conboy_get_request_token_and_auth_link(api->request_token_url, api->authorize_url);

	if (link == NULL) {
		ui_helper_show_confirmation_dialog(parent, "Could not connect to host.", FALSE);
		return FALSE;
	}

	/* Open link in browser */
	AppData *app_data = app_data_get();
	osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_browser",
			OSSO_BROWSER_OPEN_NEW_WINDOW_REQ, NULL,
			DBUS_TYPE_STRING, link, DBUS_TYPE_INVALID);
	g_printerr("Opening browser with URL: >%s<\n", link);

	GtkWidget *dialog = ui_helper_create_cancel_dialog(parent, "Please grant access on the website of your service provider that just opened.\nAfter that you will be automatically redirected back to Conboy.");

	struct AuthDialogData data;
	data.dialog = GTK_DIALOG(dialog);

	/* Register DBus listener */
	if (osso_rpc_set_cb_f(app_data->osso_ctx, "de.zwong.conboy", "de/zwong/conboy", "de.zwong.conboy", url_callback_handler, &data) != OSSO_OK) {
		g_printerr("ERROR: Failed connect DBus url callback\n");
	}

	/* Open dialog and wait for result */
	g_printerr("Before dialog run\n");
	int result = gtk_dialog_run(GTK_DIALOG(dialog));
	g_printerr("Before widget destroy\n");
	gtk_widget_hide(dialog);
	gtk_widget_destroy(dialog);
	g_printerr("After widget destroy\n");

	/* Unregister DBus listener */
	if (osso_rpc_unset_cb_f(app_data->osso_ctx, "de.zwong.conboy", "de/zwong/conboy", "de.zwong.conboy", url_callback_handler, &data) != OSSO_OK) {
			g_printerr("ERROR: Failed disconnect DBus url callback\n");
	}

	/* Handle return values of the dialog */
	if (result == GTK_RESPONSE_OK) {

		GtkWidget *wait_dialog = gtk_dialog_new();
		gtk_window_set_title(GTK_WINDOW(wait_dialog), "Connecting to server");
		hildon_gtk_window_set_progress_indicator(GTK_WINDOW(wait_dialog), TRUE);
		gtk_window_set_modal(GTK_WINDOW(wait_dialog), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(wait_dialog), parent);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(wait_dialog), TRUE);
		gtk_widget_show_all(wait_dialog);
		while (gtk_events_pending()) {
			gtk_main_iteration_do(FALSE);
		}

		if (conboy_get_access_token(api->access_token_url, data.verifier)) {
			gtk_widget_destroy(wait_dialog);
			ui_helper_show_confirmation_dialog(parent, "<b>You are successfully authenticated</b>\nYou can now use the synchronization from the main menu.", FALSE);
			settings_save_sync_base_url(url);
			/* Everything is good */
			return TRUE;
		}

		/* We did not get the access token */
		gtk_widget_destroy(wait_dialog);
		ui_helper_show_confirmation_dialog(parent, "Conboy could not get an access token from your service provider. Please try again.", FALSE);
		settings_save_sync_base_url("");
		return FALSE;
	}


	if (result == 666) {
		ui_helper_show_confirmation_dialog(parent, "You have manually canceled the authentication process.", FALSE);
		return FALSE;
	}

	if (result == GTK_RESPONSE_REJECT) {
		ui_helper_show_confirmation_dialog(parent, "There were problems with the data received from your service provider. Please try again.", FALSE);
		settings_save_sync_base_url("");
		return FALSE;
	}

	/* Some other error */
	ui_helper_show_confirmation_dialog(parent, "An unknown error occured, please try again.", FALSE);
	return FALSE;
}

