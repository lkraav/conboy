/* 
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "conboy_storage_plugin_xml.h"

#include "conboy_plugin_info.h"
#include "conboy_plugin.h"


G_DEFINE_ABSTRACT_TYPE(ConboyPlugin, conboy_plugin, G_TYPE_OBJECT);
/*G_DEFINE_TYPE(ConboyPlugin, conboy_plugin, G_TYPE_OBJECT);*/




static void
conboy_plugin_class_dispose(GObject *object)
{
	ConboyPlugin *self = CONBOY_PLUGIN(object);
	
	g_free(self->name);
	self->name = NULL;

	g_free(self->path);
	self->path = NULL;

	if (self->gmodule)
		g_module_close(self->gmodule);
	self->gmodule = NULL;

	G_OBJECT_CLASS(conboy_plugin_parent_class)->dispose(object);
}

static void
conboy_plugin_class_init (ConboyPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = conboy_plugin_class_dispose;
}

static void
conboy_plugin_init (ConboyPlugin *self)
{
	g_printerr("INFO: conboy_plugin_init() called\n");
}

/*
 * Public methods
 */

/* Opens all modules from given array.
   Modules array should be NULL terminated */
/*
void
conboy_plugin_initialize_modules(ConboyPlugin **modules, GError **error)
{
	g_return_if_fail(modules != NULL);
	g_return_if_fail(g_module_supported());

	guint i = 0;

	while (modules[i] != NULL) {

		ConboyPlugin *module = modules[i];
		gchar *path = g_build_path(G_DIR_SEPARATOR_S, _CONBOY_PLUGINS_DIR, module->name, NULL);
		module->gmodule = g_module_open(path, G_MODULE_BIND_LAZY);
		g_free(path);
		
		if (module->gmodule) {

			gchar *errmsg = g_strdup_printf("Failed to load %s module. %s", 
					module->name, g_module_error() ? g_module_error() : "Unknown reason");
			g_set_error(error, 0, 0, errmsg);
			g_free(errmsg);	
		}
	}
}
*/

/* Initialize new ConboyPlugin by name */
/*
ConboyPlugin *
conboy_plugin_new(const gchar *name)
{
	g_return_val_if_fail(name != NULL, NULL);

	GType module_type = g_type_from_name(name);
	g_return_val_if_fail(module_type != 0, NULL);

	return g_object_new(module_type, NULL);
}
*/
/*
gboolean 
conboy_plugin_initialize (ConboyPlugin *module)
{
	g_return_val_if_fail(module != NULL, FALSE);

	ConboyPluginClass *cmc = CONBOY_PLUGIN_GET_CLASS(module);

	return cmc->initialize(module);
}
*/

void
conboy_plugin_activate (ConboyPlugin *self)
{
	g_return_if_fail (CONBOY_IS_PLUGIN(self));
	CONBOY_PLUGIN_GET_CLASS(self)->activate(self);
}

/*
ConboyPlugin*
conboy_plugin_new_from_info (ConboyPluginInfo *info)
{

	const gchar *name = conboy_plugin_info_get_module_name(info);
	
	g_printerr("Module name: %s\n", name);
	g_printerr("Filename: %s\n", info->file);
}
*/

/**
 * TODO: Add error handling. A lot can go wrong here.
 */
ConboyPlugin*
conboy_plugin_new_from_path (gchar *filename)
{
	ConboyPlugin *result = NULL;
	
	typedef ConboyPlugin* (* TypeFunc) (void);
	TypeFunc func = NULL;
	
	GModule *module = g_module_open(filename, G_MODULE_BIND_LAZY);
	g_module_symbol(module, "conboy_storage_plugin_xml_new", (gpointer*)&func);
	
	if (func == NULL) {
		g_printerr("ERROR: Could not load plugin: %s\n", g_module_error());
	}
	
	result = func();
	
	return result;
}
