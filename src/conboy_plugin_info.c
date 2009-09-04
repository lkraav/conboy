/*
 * Heavily borrowed from gedit here
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib.h>

#include "conboy_plugin_info.h"


#define PLUGIN_GROUP        "Conboy Plugin"
#define PLUGIN_MODULE       "Module"
#define PLUGIN_NAME         "Name"
#define PLUGIN_DESCRIPTION  "Description"
#define PLUGIN_VERSION      "Version"
#define PLUGIN_AUTHORS      "Authors"
#define PLUGIN_COPYRIGHT    "Copyright"


G_DEFINE_TYPE(ConboyPluginInfo, conboy_plugin_info, G_TYPE_OBJECT);

static void
conboy_plugin_info_class_dispose (GObject *object)
{
	ConboyPluginInfo *self = CONBOY_PLUGIN_INFO(object);
	
	if (self->plugin != NULL) {
		g_object_unref(self->plugin);
	}

	g_free (self->file);
	g_free (self->module_name);
	g_free (self->name);
	g_free (self->desc);
	g_free (self->copyright);
	g_free (self->version);
	g_strfreev (self->authors);

	/* TODO: Not sure that I should free self here */
	/*g_free (self);*/	
}



enum {
	PLUGIN_ACTIVATE,
	PLUGIN_DEACTIVATE,
	PLUGIN_ACTIVATED,
	PLUGIN_DEACTIVATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

/*
static void conboy_plugin_info_status_changed (ConboyPluginInfo *info, gboolean active)
{
	g_printerr("Plugin '%s'   Active '%i'\n", conboy_plugin_info_get_name(info), active);
}
*/

static void
conboy_plugin_info_class_init (ConboyPluginInfoClass *klass)
{
	G_OBJECT_CLASS(klass)->dispose = conboy_plugin_info_class_dispose;
	
	klass->plugin_activate    = NULL;
	klass->plugin_deactivate  = NULL;
	klass->plugin_activated   = NULL;
	klass->plugin_deactivated = NULL;
	
	signals[PLUGIN_ACTIVATE] =
		g_signal_new(
				"plugin-activate",
				CONBOY_TYPE_PLUGIN_INFO,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginInfoClass, plugin_activate),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
	
	signals[PLUGIN_DEACTIVATE] =
			g_signal_new(
				"plugin-deactivate",
				CONBOY_TYPE_PLUGIN_INFO,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginInfoClass, plugin_deactivate),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
		
	signals[PLUGIN_ACTIVATED] =
			g_signal_new(
				"plugin-activated",
				CONBOY_TYPE_PLUGIN_INFO,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginInfoClass, plugin_activated),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
		
	signals[PLUGIN_DEACTIVATED] =
			g_signal_new(
				"plugin-deactivated",
				CONBOY_TYPE_PLUGIN_INFO,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyPluginInfoClass, plugin_deactivated),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
		
}

static void
conboy_plugin_info_init (ConboyPluginInfo *self)
{
	g_return_if_fail(CONBOY_PLUGIN_INFO(self));
	
	self->authors = NULL;
	self->available = FALSE;
	self->copyright = NULL;
	self->desc = NULL;
	self->file = NULL;
	self->module_name = NULL;
	self->name = NULL;
	self->plugin = NULL;
	self->version = NULL;
}

/*
 * Implementation
 */ 

/**
 * conboy_plugin_info_new:
 * @filename: the filename where to read the plugin information
 *
 * Creates a new #ConboyPluginInfo from a file on the disk.
 *
 * Return value: a newly created #ConboyPluginInfo.
 */
ConboyPluginInfo *
conboy_plugin_info_new (const gchar *file)
{
	ConboyPluginInfo *info;
	GKeyFile *plugin_file = NULL;
	gchar *str;

	g_return_val_if_fail (file != NULL, NULL);

	g_print("Loading plugin: %s\n", file);

	info = g_object_new(CONBOY_TYPE_PLUGIN_INFO, NULL);
	
	info->file = g_strdup (file);

	plugin_file = g_key_file_new ();
	if (!g_key_file_load_from_file (plugin_file, file, G_KEY_FILE_NONE, NULL))
	{
		g_warning ("Bad plugin file: %s", file);
		goto error;
	}

	
	/* Get module name */
	str = g_key_file_get_string (plugin_file, PLUGIN_GROUP, PLUGIN_MODULE, NULL);

	if ((str != NULL) && (*str != '\0')) {
		info->module_name = str;
	} else {
		g_warning ("Could not find '%s' in %s", PLUGIN_MODULE, file);
		goto error;
	}


	/* Get Name */
	str = g_key_file_get_locale_string (plugin_file, PLUGIN_GROUP, PLUGIN_NAME, NULL, NULL);
	if (str) {
		info->name = str;
	} else {
		g_warning ("Could not find 'Name' in %s", file);
		goto error;
	}

	
	/* Get Description */
	str = g_key_file_get_locale_string (plugin_file, PLUGIN_GROUP, PLUGIN_DESCRIPTION, NULL, NULL);
	if (str) {
		info->desc = str;
	} else {
		g_printerr("Could not find '%s' in %s\n", PLUGIN_DESCRIPTION, file);
	}


	/* Get Authors */
	info->authors = g_key_file_get_string_list (plugin_file, PLUGIN_GROUP, PLUGIN_AUTHORS, NULL, NULL);
	if (info->authors == NULL) {
		g_printerr("Could not find '%s' in %s\n", PLUGIN_AUTHORS, file);
	}


	/* Get Copyright */
	str = g_key_file_get_string (plugin_file, PLUGIN_GROUP, PLUGIN_COPYRIGHT, NULL);
	if (str) {
		info->copyright = str;
	} else {
		g_printerr("Could not find '%s' in %s\n", PLUGIN_COPYRIGHT, file);
	}

	
	/* Get Version */
	str = g_key_file_get_string (plugin_file, PLUGIN_GROUP, PLUGIN_VERSION, NULL);
	if (str) {
		info->version = str;
	} else {
		g_printerr("Could not find '%s' in %s\n", PLUGIN_VERSION, file);
	}
	
	g_key_file_free (plugin_file);
	
	info->available = TRUE;
	
	return info;

error:
	g_free (info->file);
	g_free (info->module_name);
	g_free (info->name);
	g_free (info);
	g_key_file_free (plugin_file);

	return NULL;
}

const gchar *
conboy_plugin_info_get_module_name (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->module_name;
}

const gchar *
conboy_plugin_info_get_name (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->name;
}

const gchar *
conboy_plugin_info_get_description (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->desc;
}

const gchar **
conboy_plugin_info_get_authors (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, (const gchar **)NULL);

	return (const gchar **) info->authors;
}

const gchar *
conboy_plugin_info_get_copyright (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->copyright;
}

const gchar*
conboy_plugin_info_get_version (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->version;
}

gboolean
conboy_plugin_info_is_available (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->available != FALSE;
}

gboolean
conboy_plugin_info_is_active (ConboyPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);
	g_printerr("Is active: %i\n", (info->available && info->plugin != NULL));
	return info->available && info->plugin != NULL;
}

gboolean
conboy_plugin_info_is_configurable (ConboyPluginInfo *info)
{
	if (conboy_plugin_info_is_active(info)) {
		return conboy_plugin_has_settings(info->plugin);
	}
	return FALSE;
}

gboolean
conboy_plugin_info_activate_plugin (ConboyPluginInfo *info)
{
	g_return_val_if_fail(info != NULL, NULL);
	g_return_val_if_fail(info->file != NULL, NULL);
	g_return_val_if_fail(info->module_name != NULL, NULL);
	
	if (info->plugin != NULL) {
		g_printerr("ERROR: Plugin is already active\n");
		return FALSE;
	}
	
	ConboyPlugin *result = NULL;
	
	gchar *dir = g_path_get_dirname(info->file);
	gchar *filename = g_strconcat("lib", info->module_name, ".la", NULL);
	gchar *path = g_build_filename(dir, filename, NULL);
	g_printerr("INFO: Trying to create plugin from: %s\n", path);
	
	g_signal_emit_by_name(info, "plugin-activate");
	
	result = conboy_plugin_new_from_path(path);
	
	if (result != NULL) {
		info->plugin = result;
		
		/* Emmit signal */
		/* Signals are synchronous, so if we call something after
		 * g_signal_emit, we know that all signal handlers
		 * already finished at this point */
		g_signal_emit_by_name(info, "plugin-activated");
	}
	
	g_free(dir);
	g_free(filename);
	g_free(path);
	
	return (CONBOY_PLUGIN(result));
}

gboolean
conboy_plugin_info_deactivate_plugin (ConboyPluginInfo *info)
{
	g_return_val_if_fail(info != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_PLUGIN_INFO(info), FALSE);
	
	if (info->plugin == NULL || !CONBOY_IS_PLUGIN(info->plugin)) {
		g_printerr("ERROR: Plugin not active. Cannot be deactivated\n");
		return FALSE;
	}

	/* We emit the signal before destroying the plugin, so that
	 * listeners have a change to react, e.g. save notes. */
	g_signal_emit_by_name(info, "plugin-deactivate");
	
	
	g_printerr("## NOW KILLING THE PLUGIN\n");
	g_object_unref(info->plugin);
	g_printerr("## NOW AFTER KILLING THE PLUGIN\n");
	info->plugin = NULL;
	
	g_signal_emit_by_name(info, "plugin-deactivated");
	
	return TRUE;
}
