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

#include "conboy_note_store.h"
#include "conboy_plugin.h"
#include "conboy_plugin_info.h"

#include "conboy_storage.h"


G_DEFINE_TYPE(ConboyStorage, conboy_storage, G_TYPE_OBJECT);

static void
conboy_storage_dispose (GObject *gobject)
{
	ConboyStorage *self = CONBOY_STORAGE(gobject);
	
	if (self->plugin) {
		/* TODO: g_signal_emit(self, "deactivate"; */
		
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


enum {
	ACTIVATED,
	DEACTIVATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
conboy_storage_class_init (ConboyStorageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	
	gobject_class->dispose = conboy_storage_dispose;
	gobject_class->finalize = conboy_storage_finalize;
	
	klass->activated 	= NULL;
	klass->deactivated 	= NULL;
			
	signals[ACTIVATED] =
		g_signal_new(
				"activated",
				CONBOY_TYPE_STORAGE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyStorageClass, activated),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
	
	signals[DEACTIVATED] =
		g_signal_new(
				"deactivated",
				CONBOY_TYPE_STORAGE,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(ConboyStorageClass, deactivated),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);
	
}


static void
conboy_storage_init (ConboyStorage *self)
{
	/* Init all members */
	self->plugin = NULL;
	self->plugin_store = NULL;
}
	

/*
 * Public methods
 */

ConboyStorage*
conboy_storage_new()
{
	return g_object_new(CONBOY_TYPE_STORAGE, NULL);
}

static void
conboy_storage_set_active_plugin(ConboyStorage *self, ConboyPluginStore *plugin_store)
{
	GList *infos = conboy_plugin_store_get_plugin_infos(plugin_store);
	while (infos) {
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(infos->data);
		if (conboy_plugin_info_is_active(info)) {
			if (CONBOY_IS_STORAGE_PLUGIN(info->plugin)) {
				self->plugin = CONBOY_STORAGE_PLUGIN(info->plugin);
				g_object_ref(self->plugin);
				break;
			}
		}
		infos = infos->next;
	}
}

static void
on_plugin_activated(ConboyPluginStore *store, ConboyPluginInfo *info, ConboyStorage *self)
{
	g_return_if_fail(info != NULL);
	g_return_if_fail(self != NULL);
	
	g_return_if_fail(CONBOY_IS_PLUGIN_INFO(info));
	g_return_if_fail(CONBOY_IS_STORAGE(self));
	
	if (self->plugin != NULL) {
		g_printerr("ERROR: Plugin has been activated, but there is already one active\n");
		return;
	}
	
	self->plugin = CONBOY_STORAGE_PLUGIN(info->plugin);
	g_object_ref(self->plugin);
	g_signal_emit_by_name(self, "activated");
}

static void
on_plugin_deactivate(ConboyPluginStore *store, ConboyPluginInfo *info, ConboyStorage *self)
{
	g_return_if_fail(info != NULL);
	g_return_if_fail(self != NULL);
	
	g_return_if_fail(CONBOY_IS_PLUGIN_INFO(info));
	g_return_if_fail(CONBOY_IS_STORAGE(self));
	
	if (!CONBOY_IS_STORAGE_PLUGIN(info->plugin)) return;
	
	if (self->plugin == NULL) {
		g_printerr("ERROR: There is no plugin in use, so it cannot be deactivated \n");
		return;
	}
	
	if (self->plugin != CONBOY_STORAGE_PLUGIN(info->plugin)) {
		g_printerr("ERROR: Another plugin is getting deactivated, but we don't use it\n");
		return;
	}
	
	g_signal_emit_by_name(self, "deactivated");
	g_object_unref(self->plugin);
	self->plugin = NULL;
}

void
conboy_storage_set_plugin_store(ConboyStorage *self, ConboyPluginStore *plugin_store)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(plugin_store != NULL);
	
	g_return_if_fail(CONBOY_IS_STORAGE(self));
	g_return_if_fail(CONBOY_IS_PLUGIN_STORE(plugin_store));
	
	if (self->plugin_store == NULL) {
		self->plugin_store = plugin_store;
		g_object_ref(plugin_store);
		
		conboy_storage_set_active_plugin(self, plugin_store);
		
		g_signal_connect(plugin_store, "plugin-activated", G_CALLBACK(on_plugin_activated), self);
		g_signal_connect(plugin_store, "plugin-deactivate", G_CALLBACK(on_plugin_deactivate), self);
		
	} else {
		g_printerr("ERROR: PluginStore already set\n");
	}
}


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

	if (conboy_storage_plugin_note_delete(self->plugin, note)) {
		/* TODO: Add filename to a files which tracks deleted files */
		return TRUE;
	} else {
		return FALSE;
	}
}

GSList*
conboy_storage_note_list (ConboyStorage *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_STORAGE(self), NULL);	
	
	
	g_return_val_if_fail(self->plugin != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), NULL);

	return conboy_storage_plugin_note_list(self->plugin);
}

GSList*
conboy_storage_note_list_ids (ConboyStorage *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);	
	
	
	g_return_val_if_fail(self->plugin != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN(self->plugin), FALSE);

	return conboy_storage_plugin_note_list_ids(self->plugin);
}

