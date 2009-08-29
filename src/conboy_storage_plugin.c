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

#include "conboy_storage_plugin.h"

G_DEFINE_ABSTRACT_TYPE(ConboyStoragePlugin, conboy_storage_plugin, CONBOY_TYPE_PLUGIN)


/* GOBJECT ROUTINES */

static void
conboy_storage_plugin_dispose(GObject *object)
{
	ConboyStoragePlugin *self = CONBOY_STORAGE_PLUGIN(object);
	G_OBJECT_CLASS(conboy_storage_plugin_parent_class)->dispose(object);
}


static void
conboy_storage_plugin_init (ConboyStoragePlugin *self)
{
	g_printerr("INFO: conboy_storage_plugin_init() called\n");
}


static void
conboy_storage_plugin_class_init (ConboyStoragePluginClass *klass)
{
	/* This class is abstract, we don't provide default implementation */
	klass->load     = NULL;
	klass->save     = NULL;
	klass->delete   = NULL;
	klass->list     = NULL;
	klass->list_ids = NULL;
}



/*
 * Virtual methods
 */

ConboyNote*
conboy_storage_plugin_note_load (ConboyStoragePlugin *self, const gchar *guid)
{
	return CONBOY_STORAGE_PLUGIN_GET_CLASS(self)->load(self, guid);
}

gboolean
conboy_storage_plugin_note_save (ConboyStoragePlugin *self, ConboyNote *note)
{
	return CONBOY_STORAGE_PLUGIN_GET_CLASS(self)->save(self, note);
}

gboolean
conboy_storage_plugin_note_delete (ConboyStoragePlugin *self, ConboyNote *note)
{
	return CONBOY_STORAGE_PLUGIN_GET_CLASS(self)->delete(self, note);
}

GSList*
conboy_storage_plugin_note_list (ConboyStoragePlugin *self)
{
	return CONBOY_STORAGE_PLUGIN_GET_CLASS(self)->list(self);
}

GSList*
conboy_storage_plugin_note_list_ids (ConboyStoragePlugin *self)
{
	return CONBOY_STORAGE_PLUGIN_GET_CLASS(self)->list_ids(self);
}

