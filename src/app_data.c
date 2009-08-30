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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include "conboy_plugin_info.h"
#include "settings.h"
#include "app_data.h"

/* Global AppData only access with get_app_data() */
AppData *_app_data = NULL;




AppData* app_data_get() {

	if (_app_data == NULL) {
		gint font_size;
		GConfClient *client;
		const gchar *path;
		ConboyNoteStore *store;

		client = gconf_client_get_default();
		gconf_client_add_dir(client, SETTINGS_ROOT, GCONF_CLIENT_PRELOAD_NONE, NULL);

		font_size = gconf_client_get_int(client, SETTINGS_FONT_SIZE, NULL);
		if (font_size == 0) {
			font_size = 20000;
		}

		path = g_strconcat(g_get_home_dir(), "/.conboy/", NULL);

		/* Create dir if needed */
		if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
			g_mkdir(path, 0700);
		}
		
		/* Create storage */
		ConboyStorage *storage = conboy_storage_new();
		ConboyPluginInfo *info = conboy_plugin_info_new("/home/conny/workspace/conboy/src/plugins/storage_xml/conboy_storage_xml.plugin");
		conboy_plugin_info_activate_plugin(info);
		conboy_storage_set_plugin(storage, CONBOY_STORAGE_PLUGIN(info->plugin)); 
		 

		store = conboy_note_store_new();

		_app_data = g_new(AppData, 1);
		_app_data->user_path = path;
		_app_data->note_store = store;
		_app_data->open_notes = NULL;
		_app_data->client = client;
		_app_data->program = hildon_program_get_instance();
		_app_data->fullscreen = FALSE;
		_app_data->portrait = is_portrait_mode();
		_app_data->search_window = NULL;
		_app_data->reader = NULL;
		_app_data->storage = storage; 
		_app_data->note_window = NULL;

		/*populate_note_store(store, path);*/
		conboy_note_store_fill_from_storage(store, storage);
	}

	return _app_data;
}

void app_data_free()
{
	AppData *app_data = app_data_get();

	g_object_unref(app_data->client);
	g_list_free(app_data->open_notes);
	g_free((gchar*)app_data->user_path);
	if (app_data->search_window != NULL) {
		gtk_widget_destroy(GTK_WIDGET(app_data->search_window));
	}
	if (app_data->reader != NULL) {
		xmlFreeTextReader(app_data->reader);
		xmlCleanupParser();
	}
	g_free(app_data);
}
