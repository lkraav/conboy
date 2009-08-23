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

#include "conboy_plugin_info.h"
#include "conboy_plugin.h"

/*
 * GOBJECT STUFF
 */

G_DEFINE_ABSTRACT_TYPE(ConboyPlugin, conboy_plugin, G_TYPE_OBJECT);

static void
conboy_plugin_class_dispose(GObject *object)
{
	ConboyPlugin *self = CONBOY_PLUGIN(object);
	
	if (self->gmodule) {
		g_printerr("INFO: Unloading library\n");
		g_module_close(self->gmodule);
	}
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
	g_return_if_fail(CONBOY_PLUGIN(self));
	g_printerr("INFO: conboy_plugin_init() called\n");
	self->has_settings = FALSE;
	self->gmodule = NULL;
}

/*
 * END GOBJECT STUFF
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
	
	g_printerr("FILENAME: %s\n", filename);
	
	GModule *module = g_module_open(filename, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	
	if (module == NULL) {
		g_printerr("ERROR: Could not open library: %s\n", g_module_error());
	}
	
	g_module_symbol(module, "conboy_plugin_new", (gpointer*)&func);
	
	if (func == NULL) {
		g_printerr("ERROR: Could not load plugin: %s\n", g_module_error());
	}
	
	result = func();
	
	g_return_val_if_fail(result != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_PLUGIN(result), NULL);
	
	result->gmodule = module;
	return result;
}

gboolean
conboy_plugin_has_settings (ConboyPlugin *self)
{
	g_return_val_if_fail (CONBOY_IS_PLUGIN(self), FALSE);
	return self->has_settings;
}


