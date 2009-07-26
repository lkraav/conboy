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

#include "conboy_module.h"

static GObjectClass *parent_class = NULL;


#define _CONBOY_MODULES_DIR "/usr/lib/conboy/modules"

/* Opens all modules from given array.
   Modules array should be NULL terminated */
void
conboy_module_initialize_modules(ConboyModule **modules, GError **error)
{
	g_return_if_fail(modules != NULL);
	g_return_if_fail(g_module_supported());

	guint i = 0;

	while (modules[i] != NULL) {

		ConboyModule *module = modules[i];
		gchar *path = g_build_path(G_DIR_SEPARATOR_S, _CONBOY_MODULES_DIR, module->name, NULL);
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

/* Initialize new ConboyModuleby name */
ConboyModule *
conboy_module_new(const gchar *name)
{
	g_return_val_if_fail(name != NULL, NULL);

	GType module_type = g_type_from_name(name);
	g_return_val_if_fail(module_type != 0, NULL);

	return g_object_new(module_type, NULL);
}

gboolean 
conboy_module_initialize (ConboyModule *module)
{
	g_return_val_if_fail(module != NULL, FALSE);

	ConboyModuleClass *cmc = CONBOY_MODULE_GET_CLASS(module);

	return cmc->initialize(module);
}

/* GOBJECT ROUTINES */

static GObject *
_conboy_module_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	CONBOY_MODULE(object)->name = NULL;
	CONBOY_MODULE(object)->path = NULL;
	CONBOY_MODULE(object)->gmodule = NULL;
	CONBOY_MODULE(object)->open_on_startup = FALSE;
	
	return G_OBJECT(object);
}

static void
_conboy_module_class_dispose(GObject *object)
{
	ConboyModule *self = CONBOY_MODULE(object);
	
	g_free(self->name);
	self->name = NULL;

	g_free(self->path);
	self->path = NULL;

	if (self->gmodule)
		g_module_close(self->gmodule);
	self->gmodule = NULL;

	parent_class->dispose(object);
}

static void
_conboy_module_class_init (ConboyModuleClass *klass, gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	ConboyModuleClass *cmc = CONBOY_MODULE_CLASS(object_class);

	object_class->constructor = _conboy_module_constructor;
	object_class->dispose = _conboy_module_class_dispose;
	
	cmc->initialize = NULL;
}

GType
conboy_module_get_type (void)
{
	static GType type = 0;
	if (type == 0) {

		static const GTypeInfo info = {
			sizeof (ConboyModuleClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _conboy_module_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
 			sizeof (ConboyModule),
			0,              /* n_preallocs */
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
				"ConboyModule", &info, 0);
	}

	return type;
}
