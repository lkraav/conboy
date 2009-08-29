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

#ifndef CONBOY_STORAGE_PLUGIN_H
#define CONBOY_STORAGE_PLUGIN_H

#include <glib-object.h>
#include <gmodule.h>

#include "conboy_note.h"
#include "conboy_plugin.h"

/* convention macros */
#define CONBOY_TYPE_STORAGE_PLUGIN				(conboy_storage_plugin_get_type())
#define CONBOY_STORAGE_PLUGIN(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_STORAGE_PLUGIN, ConboyStoragePlugin))
#define CONBOY_STORAGE_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_STORAGE_PLUGIN, ConboyStoragePluginClass))
#define CONBOY_IS_STORAGE_PLUGIN(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_STORAGE_PLUGIN))
#define CONBOY_IS_STORAGE_PLUGIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_STORAGE_PLUGIN))
#define CONBOY_STORAGE_PLUGIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_STORAGE_PLUGIN, ConboyStoragePluginClass))

typedef struct _ConboyStoragePlugin			ConboyStoragePlugin;
typedef struct _ConboyStoragePluginClass	ConboyStoragePluginClass;



struct _ConboyStoragePlugin {
	ConboyPlugin parent;
};

struct _ConboyStoragePluginClass {
	ConboyPluginClass parent;

	/* virtual methods */
	ConboyNote*		(*load)		(ConboyStoragePlugin *self, const gchar* guid);
	gboolean		(*save)		(ConboyStoragePlugin *self, ConboyNote *note);
	gboolean		(*delete)	(ConboyStoragePlugin *self, ConboyNote *note);
	GSList*			(*list)		(ConboyStoragePlugin *self);
	GSList*			(*list_ids)	(ConboyStoragePlugin *self);

	/* signals */
	
};

GType			conboy_storage_plugin_get_type (void);

/**
 * Loads a note from the storage backend. It does not alter the note in any way.
 */
ConboyNote*		conboy_storage_plugin_note_load (ConboyStoragePlugin *self, const gchar *guid);

/**
 * Saves the note to the storage backend. It does not alter or update any of the notes properties.
 * E.g. Updating the last modification date is not the job of this method.
 */
gboolean		conboy_storage_plugin_note_save (ConboyStoragePlugin *self, ConboyNote *note);

/**
 * Completely deletes the note from the storage backend. It does not invalidate or free this note.
 */
gboolean 		conboy_storage_plugin_note_delete (ConboyStoragePlugin *self, ConboyNote *note);

/**
 * Returns a GSList of all available notes as ConboyNote objects. The list needs to
 * be freed by the caller.
 */
GSList*			conboy_storage_plugin_note_list (ConboyStoragePlugin *self);

/**
 * Returns a GSList of strings (gchar arrays) containing the GUIDs of all available notes.
 * The list and the strings have to be freed by the caller.
 */
GSList*			conboy_storage_plugin_note_list_ids (ConboyStoragePlugin *self);


#endif /* CONBOY_STORAGE_PLUGIN_H */
