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

#include "conboy_config.h"
#include "conboy_plugin_info.h"
#include "settings.h"
#include "conboy_plugin_store.h"


G_DEFINE_TYPE(ConboyPluginStore, conboy_plugin_store, G_TYPE_OBJECT);



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
		path = g_build_filename(PREFIX, "/lib/conboy", NULL);
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
create_all_plugin_infos (const gchar *plugin_base_dir)
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
				if (g_str_has_suffix(inner_filename, ".plugin")) {
					gchar *plugin_file = g_build_filename(full_path, inner_filename, NULL);
					ConboyPluginInfo *info = conboy_plugin_info_new(plugin_file);
					if (info) {
						result = g_list_prepend(result, info);
					}
					g_free(plugin_file);
				}
			}
			g_dir_close(inner_dir);
			
		/* If it's a file, check if it's a .plugin file */
		} else if (g_file_test(full_path, G_FILE_TEST_EXISTS)) {
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

static void
on_plugin_activate(ConboyPluginInfo *info, ConboyPluginStore *self)
{
	g_signal_emit_by_name(self, "plugin-activate", info);

	/* If a storage plugin was activated and another storage plugin is already active,
	 * then first deactivate the already active plugin. */
	if (strcmp(conboy_plugin_info_get_kind(info), "storage") == 0) {
		GList *infos = self->plugins;
		while (infos) {
			ConboyPluginInfo *other_info = CONBOY_PLUGIN_INFO(infos->data);
			if (other_info != info) {
				if (strcmp(conboy_plugin_info_get_kind(other_info), "storage") == 0) {
					if (conboy_plugin_info_is_active(other_info)) {
						conboy_plugin_info_deactivate_plugin(other_info);
					}
				}
			}
			infos = infos->next;
		}
	}
}

static void
on_plugin_activated(ConboyPluginInfo *info, ConboyPluginStore *self)
{
	/* Add to list of active plugins */
	settings_add_active_plugin(conboy_plugin_info_get_name(info));
	
	g_signal_emit_by_name(self, "plugin-activated", info);
}

static void
on_plugin_deactivate(ConboyPluginInfo *info, ConboyPluginStore *self)
{
	g_signal_emit_by_name(self, "plugin-deactivate", info);
}

static void
on_plugin_deactivated(ConboyPluginInfo *info, ConboyPluginStore *self)
{
	/* Remove from list of active plugins */
	settings_remove_active_plugin(conboy_plugin_info_get_name(info));
	
	g_signal_emit_by_name(self, "plugin-deactivated", info);
}

/*
 * Public methods
 */

ConboyPluginStore*
conboy_plugin_store_new ()
{
	return g_object_new(CONBOY_TYPE_PLUGIN_STORE, NULL);
}

GList*
conboy_plugin_store_get_plugin_infos (ConboyPluginStore *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_PLUGIN_STORE(self), NULL);
	
	return self->plugins;
}


gboolean
conboy_plugin_store_activate_by_name (ConboyPluginStore *self, const gchar *name)
{
	GList *infos = self->plugins;
	while (infos) {
		ConboyPluginInfo *info = (ConboyPluginInfo*) infos->data;
		const gchar *plugin_name = conboy_plugin_info_get_module_name (info);
		if (name && plugin_name && g_str_equal (name, plugin_name)) {
			conboy_plugin_info_activate_plugin(info);
			return TRUE;
		}
		infos = infos->next;
	}
	return FALSE;
}


/*
 * GOBJECT STUFF
 */

static void
conboy_plugin_store_dispose(GObject *object)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(CONBOY_IS_PLUGIN_STORE(object));
	
	ConboyPluginStore *self = CONBOY_PLUGIN_STORE(object);
	
	/* Deactivate all plugins */
	GList *plugins = self->plugins;
	while (plugins) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(plugins->data);
		conboy_plugin_info_deactivate_plugin(info);
		plugins = plugins->next;
	}
	
	/* Remove signal handlers */
	plugins = self->plugins;
	while (plugins) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(plugins->data);
		/*
		 * TODO: Remove signal handlers
		 */
		plugins = plugins->next;
	}
	
	/* Remove info objects */
	plugins = self->plugins;
	while (plugins) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(plugins->data);
		g_object_unref(info);
		plugins = plugins->next;
	}
	
	g_list_free(self->plugins);

	G_OBJECT_CLASS(conboy_plugin_store_parent_class)->dispose(object);
}

enum {
	PLUGIN_ACTIVATE,
	PLUGIN_ACTIVATED,
	PLUGIN_DEACTIVATE,
	PLUGIN_DEATIVATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
conboy_plugin_store_class_init (ConboyPluginStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->dispose = conboy_plugin_store_dispose;
	
	klass->plugin_activate		= NULL;
	klass->plugin_activated 	= NULL;
	klass->plugin_deactivate	= NULL;
	klass->plugin_deactivated 	= NULL;
	
	signals[PLUGIN_ACTIVATE] =
		g_signal_new(
				"plugin-activate",
				CONBOY_TYPE_PLUGIN_STORE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginStoreClass, plugin_activate),
				NULL, NULL,
				g_cclosure_marshal_VOID__OBJECT,
				G_TYPE_NONE,
				1,
				CONBOY_TYPE_PLUGIN_INFO);
	
	signals[PLUGIN_ACTIVATED] =
		g_signal_new(
				"plugin-activated",
				CONBOY_TYPE_PLUGIN_STORE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginStoreClass, plugin_activated),
				NULL, NULL,
				g_cclosure_marshal_VOID__OBJECT,
				G_TYPE_NONE,
				1,
				CONBOY_TYPE_PLUGIN_INFO);
	
	signals[PLUGIN_DEACTIVATE] =
		g_signal_new(
				"plugin-deactivate",
				CONBOY_TYPE_PLUGIN_STORE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginStoreClass, plugin_deactivate),
				NULL, NULL,
				g_cclosure_marshal_VOID__OBJECT,
				G_TYPE_NONE,
				1,
				CONBOY_TYPE_PLUGIN_INFO);
	
	signals[PLUGIN_DEATIVATED] =
		g_signal_new(
				"plugin-deactivated",
				CONBOY_TYPE_PLUGIN_STORE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginStoreClass, plugin_deactivated),
				NULL, NULL,
				g_cclosure_marshal_VOID__OBJECT,
				G_TYPE_NONE,
				1,
				CONBOY_TYPE_PLUGIN_INFO);
	
}

static void
conboy_plugin_store_init (ConboyPluginStore *self)
{
	g_return_if_fail(CONBOY_PLUGIN_STORE(self));
	
	self->plugin_path = get_plugin_base_dir();
	g_printerr("INFO: Looking for plugins in: %s\n", self->plugin_path);

	self->plugins = create_all_plugin_infos(self->plugin_path);
	
	/* Activate all plugins that should get activated */
	GSList *active_plugins = settings_load_active_plugins();
	while (active_plugins) {
		gchar *plugin_name = (gchar*) active_plugins->data;
		conboy_plugin_store_activate_by_name(self, plugin_name);
		active_plugins = active_plugins->next;
	}
	
	/* Connect signal handler */
	GList *plugin_infos = self->plugins;
	while (plugin_infos) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(plugin_infos->data);
		g_signal_connect(info, "plugin-activate",    G_CALLBACK(on_plugin_activate),    self);
		g_signal_connect(info, "plugin-activated",   G_CALLBACK(on_plugin_activated),   self);
		g_signal_connect(info, "plugin-deactivate",  G_CALLBACK(on_plugin_deactivate),  self);
		g_signal_connect(info, "plugin-deactivated", G_CALLBACK(on_plugin_deactivated), self);
		plugin_infos = plugin_infos->next;
	}
}

