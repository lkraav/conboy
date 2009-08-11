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


void
conboy_plugin_info_ref (ConboyPluginInfo *info)
{
	g_atomic_int_inc (&info->refcount);
}

static ConboyPluginInfo *
conboy_plugin_info_copy (ConboyPluginInfo *info)
{
	conboy_plugin_info_ref (info);
	return info;
}

void
conboy_plugin_info_unref (ConboyPluginInfo *info)
{
	if (!g_atomic_int_dec_and_test (&info->refcount))
		return;

	/*
	if (info->plugin != NULL)
	{
		conboy_debug_message (DEBUG_PLUGINS, "Unref plugin %s", info->name);

		g_object_unref (info->plugin);
	}
	*/

	g_free (info->file);
	g_free (info->module_name);
	g_free (info->name);
	g_free (info->desc);
	g_free (info->copyright);
	g_free (info->version);
	g_strfreev (info->authors);

	g_free (info);
}

/**
 * conboy_plugin_info_get_type:
 *
 * Retrieves the #GType object which is associated with the #ConboyPluginInfo
 * class.
 *
 * Return value: the GType associated with #ConboyPluginInfo.
 **/
GType
conboy_plugin_info_get_type (void)
{
	static GType the_type = 0;

	if (G_UNLIKELY (!the_type))
		the_type = g_boxed_type_register_static (
					"ConboyPluginInfo",
					(GBoxedCopyFunc) conboy_plugin_info_copy,
					(GBoxedFreeFunc) conboy_plugin_info_unref);

	return the_type;
} 

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

	info = g_new0 (ConboyPluginInfo, 1);
	info->refcount = 1;
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
