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
#include <string.h>

#include "conboy_plugin_info.h"
#include "settings.h"
#include "conboy_config.h"
#include "app_data.h"

/* Global AppData only access with get_app_data() */
AppData *_app_data = NULL;


/*
 * Returns the path set by the environment variable
 * CONBOY_PLUGIN_DIR or if not set, the default path
 * $prefix/lib/conboy
 * 
 * Return value needs to be freed
 */
static gchar*
get_plugin_base_dir()
{
	gchar *path = NULL;
	const gchar *env_path = g_getenv("CONBOY_PLUGIN_DIR");
	if (env_path != NULL) {
		if (g_file_test(env_path, G_FILE_TEST_IS_DIR)) {
			path = g_strdup(env_path);
		} else {
			g_printerr("WARN: '%s' is not a directory or does not exist. Please set the environment variable CONBOY_PLUGIN_DIR correctly. Trying default.\n", path);
		}
	} else {
		path = g_build_filename(PREFIX, "/lib/conboy");
	}
	return path;
}

/**
 * Returns a GList of all ConboyPluginInfo objects found in the given
 * plugin_base_dir or one level deeper in the directory hierarchy.
 * 
 * Looks at all files in plugin_base_dir if those are .plugin files,
 * ConboyPluginInfo objects are created. Also all directories of
 * plugin_base_dir are searched.
 */
static GList*
get_all_plugin_infos (const gchar *plugin_base_dir)
{
	/*
	 * for each file with .plugin ending create ConboyPluginInfo
	 */
	GList *result = NULL;
	
	const gchar *filename;
	GDir *dir = g_dir_open(plugin_base_dir, 0, NULL);
	while ((filename = g_dir_read_name(dir)) != NULL) {
		gchar *full_path = g_build_filename(plugin_base_dir, filename, NULL);
		
		/* If it's a dir, check if it contains .plugin files */
		if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
			const gchar *inner_filename;
			GDir *inner_dir = g_dir_open(full_path, 0, NULL);
			while ((inner_filename = g_dir_read_name(inner_dir)) != NULL) {
				g_printerr("Check: %s\n", inner_filename);
				if (g_str_has_suffix(inner_filename, ".plugin")) {
					gchar *plugin_file = g_build_filename(full_path, inner_filename, NULL);
					ConboyPluginInfo *info = conboy_plugin_info_new(plugin_file);
					result = g_list_prepend(result, info);
					g_free(plugin_file);
				}
			}
			g_dir_close(inner_dir);
			
		/* If it's a file, check if it's a .plugin file */
		} else if (g_file_test(full_path, G_FILE_TEST_EXISTS)) {
			g_printerr("Check: %s\n", full_path);
			if (g_str_has_suffix(full_path, ".plugin")) {
				ConboyPluginInfo *info = conboy_plugin_info_new(full_path);
				result = g_list_prepend(result, info);
			}
		}
		
		g_free(full_path);
	}
	
	g_dir_close(dir);

	return result;
}

static
ConboyPluginInfo*
get_plugin_info_by_module_name(const gchar *name, GList *infos)
{
	while (infos) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(infos->data);
		if (strcmp(conboy_plugin_info_get_module_name(info), name) == 0) {
			return info; 
		}
		infos = infos->next;
	}
	g_printerr("ERROR: Could not find a plugin info with module name: '%s'\n", name);
	return NULL;
}


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
		
		/* Dicover plugins */
		gchar *plugin_path = get_plugin_base_dir();
		GList *plugin_infos = get_all_plugin_infos(plugin_path);
		g_free(plugin_path);
		
		
		/* Create storage */
		ConboyStorage *storage = conboy_storage_new();
		gchar *plugin_name = settings_load_storage_plugin_name(client);
		ConboyPluginInfo *info = get_plugin_info_by_module_name(plugin_name, plugin_infos);
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
		_app_data->plugin_infos = plugin_infos;

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
