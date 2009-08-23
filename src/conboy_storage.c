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

#include "conboy_storage.h"


G_DEFINE_TYPE(ConboyStorage, conboy_storage, G_TYPE_OBJECT);

static void
conboy_storage_dispose (GObject *gobject)
{
	ConboyStorage *self = CONBOY_STORAGE(gobject);
	
	/* 
	 * In dispose, you are supposed to free all types referenced from this
	 * object which might themselves hold a reference to self. Generally,
	 * the most simple solution is to unref all members on which you own a 
	 * reference.
	 */
	
	/* dispose might be called multiple times, so we must guard against
	 * calling g_object_unref() on an invalid GObject.
	 */
	
	if (self->plugin) {
		g_object_unref(self->plugin);
		self->plugin = NULL;
	}
	
	/* Chain up to the parent class */
	G_OBJECT_CLASS(conboy_storage_parent_class)->dispose(gobject);
}

static void
conboy_storage_finalize (GObject *gobject)
{
	/*ConboyStorage *self = CONBOY_STORAGE(gobject);*/
	
	/*g_free(self->xyz);*/ /* Stuff which is not gobject based*/
	
	/* Chain up to the parent class */
	G_OBJECT_CLASS(conboy_storage_parent_class)->finalize(gobject);
}

static void
conboy_storage_class_init (ConboyStorageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	
	gobject_class->dispose = conboy_storage_dispose;
	gobject_class->finalize = conboy_storage_finalize;
}

static void
conboy_storage_init (ConboyStorage *self)
{
	/* Init all members */
	self->plugin = NULL;
}


/*
 * Public methods
 */

ConboyNote*
conboy_storage_note_load (ConboyStorage *self, const gchar *guid)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(guid != NULL, NULL);
	
	g_return_val_if_fail(CONBOY_IS_STORAGE(self), NULL);
	
	if (self->plugin == NULL) {
		g_printerr("ERROR: StoragePlugin is NULL\n");
		return NULL;
	}
	
	if (!CONBOY_IS_STORAGE_PLUGIN(self->plugin)) {
		g_printerr("ERROR: The plugin is not a StoragePlugin\n");
		return NULL;
	}
	
	return conboy_storage_plugin_note_load(self->plugin, guid);
}

gboolean
conboy_storage_note_save (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	
	g_return_val_if_fail(self->plugin != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), FALSE);

	return conboy_storage_plugin_note_save(self->plugin, note);
}

gboolean 
conboy_storage_note_delete (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	
	g_return_val_if_fail(self->plugin != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), FALSE);

	return conboy_storage_plugin_note_delete(self->plugin, note);
}

ConboyNote**
conboy_storage_note_list (ConboyStorage *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);	
	
	
	g_return_val_if_fail(self->plugin != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), FALSE);

	return conboy_storage_plugin_note_list(self->plugin);
}

gchar**
conboy_storage_note_list_ids (ConboyStorage *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);	
	
	
	g_return_val_if_fail(self->plugin != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), FALSE);

	return conboy_storage_plugin_note_list_ids(self->plugin);
}

