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
#include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

#include "conboy_plugin_info.h"
#include "settings.h"
#include "conboy_config.h"
#include "conboy_plugin_store.h"
#include "app_data.h"

/* Global AppData only access with get_app_data() */
AppData *_app_data = NULL;

AppData*
app_data_get()
{
	return _app_data;
}

void
app_data_init()
{
	g_return_if_fail(_app_data == NULL);

	_app_data = g_new(AppData, 1);
	_app_data->fullscreen = FALSE;
	_app_data->search_window = NULL;
	_app_data->note_window = NULL;
	_app_data->portrait = is_portrait_mode();
	_app_data->program = hildon_program_get_instance();
	_app_data->client = gconf_client_get_default();
	_app_data->note_history = NULL;
	_app_data->current_element = NULL;
	_app_data->started = FALSE;
	_app_data->accelerators = FALSE;

	gconf_client_add_dir(_app_data->client, SETTINGS_ROOT, GCONF_CLIENT_PRELOAD_NONE, NULL);

	/* Create dir if needed */
	gchar *path = g_strconcat(g_get_home_dir(), "/.conboy/", NULL);
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		g_mkdir(path, 0700);
	}
	g_free(path);

	/* Dicover plugins */
	ConboyPluginStore *plugin_store = conboy_plugin_store_new();
	_app_data->plugin_store = plugin_store;

	/* Create storage */
	ConboyStorage *storage = conboy_storage_new();
	conboy_storage_set_plugin_store(storage, plugin_store);
	g_object_unref(plugin_store);
	_app_data->storage = storage;

	/* Create store */
	ConboyNoteStore *note_store = conboy_note_store_new();
	conboy_note_store_set_storage(note_store, storage);
	g_object_unref(storage);
	_app_data->note_store = note_store;

}

void app_data_free()
{
	AppData *app_data = app_data_get();

	g_object_unref(app_data->client);
	if (app_data->search_window != NULL) {
		gtk_widget_destroy(GTK_WIDGET(app_data->search_window));
	}
	
	g_free(app_data);
}
