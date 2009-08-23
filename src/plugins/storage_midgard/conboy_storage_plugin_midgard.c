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

#include "../../conboy_note.h"
#include "../../conboy_storage_plugin.h"
#include "conboy_storage_plugin_midgard.h"

G_DEFINE_TYPE(ConboyStoragePluginMidgard, conboy_storage_plugin_midgard, CONBOY_TYPE_STORAGE_PLUGIN);

static ConboyNote*
_conboy_storage_plugin_midgard_note_load (ConboyStoragePlugin *self, const gchar *uuid)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(uuid != NULL, FALSE);
	
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_MIDGARD(self), FALSE);
	
	/* TODO: Read note from midgard */
	
	return NULL;
}

static gboolean
_conboy_storage_plugin_midgard_note_save (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_MIDGARD(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Write note to midgard */

	return FALSE;
}

static gboolean 
_conboy_storage_plugin_midgard_note_delete (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_MIDGARD(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Delete note from midgard */

	return FALSE;
}

static ConboyNote**
_conboy_storage_plugin_midgard_note_list (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_MIDGARD(self), FALSE);	

	/* TODO: Return notes */	

	return NULL;
}

static gchar**
_conboy_storage_plugin_midgard_note_list_ids (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_MIDGARD(self), FALSE);	

	/* TODO: Return notes' IDS */

	return NULL;
}

/* GOBJECT ROUTINES */

static GObject *
_conboy_storage_plugin_midgard_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (conboy_storage_plugin_midgard_parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	CONBOY_STORAGE_PLUGIN_MIDGARD(object)->user = NULL;
	CONBOY_STORAGE_PLUGIN_MIDGARD(object)->pass = NULL;

	return G_OBJECT(object);
}

static void
_conboy_storage_plugin_midgard_dispose(GObject *object)
{
	ConboyStoragePluginMidgard *self = CONBOY_STORAGE_PLUGIN_MIDGARD(object);
	
	g_free((gchar *)self->user);
	self->user = NULL;
	
	g_free((gchar *)self->pass);
	self->pass = NULL;

	G_OBJECT_CLASS(conboy_storage_plugin_midgard_parent_class)->dispose(object);
}

static void
conboy_storage_plugin_midgard_class_init (ConboyStoragePluginMidgardClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ConboyStoragePluginClass *parent_class = conboy_storage_plugin_midgard_parent_class;

	object_class->constructor = _conboy_storage_plugin_midgard_constructor;
	object_class->dispose     = _conboy_storage_plugin_midgard_dispose;

	parent_class->load     = _conboy_storage_plugin_midgard_note_load;
	parent_class->save     = _conboy_storage_plugin_midgard_note_save;
	parent_class->delete   = _conboy_storage_plugin_midgard_note_delete;
	parent_class->list     = _conboy_storage_plugin_midgard_note_list;
	parent_class->list_ids = _conboy_storage_plugin_midgard_note_list_ids;
}

static void
conboy_storage_plugin_midgard_init (ConboyStoragePluginMidgard *self)
{
	g_printerr("Hello from Midgard plugin\n");
	CONBOY_PLUGIN(self)->has_settings = TRUE;
}

ConboyStoragePluginMidgard*
conboy_plugin_new() {
	return g_object_new(CONBOY_TYPE_STORAGE_PLUGIN_MIDGARD, NULL);
}
