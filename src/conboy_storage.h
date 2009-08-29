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

#ifndef CONBOY_STORAGE_H
#define CONBOY_STORAGE_H

#include <glib-object.h>
#include "conboy_storage_plugin.h"

/* convention macros */
#define CONBOY_TYPE_STORAGE				(conboy_storage_get_type())
#define CONBOY_STORAGE(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_STORAGE, ConboyStorage))
#define CONBOY_STORAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_STORAGE, ConboyStorageClass))
#define CONBOY_IS_STORAGE(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_STORAGE))
#define CONBOY_IS_STORAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_STORAGE))
#define CONBOY_STORAGE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_STORAGE, ConboyStorageClass))

typedef struct _ConboyStorage 		ConboyStorage;
typedef struct _ConboyStorageClass	ConboyStorageClass;

struct _ConboyStorage {
	GObject parent;
	ConboyStoragePlugin *plugin;
};

struct _ConboyStorageClass {
	GObjectClass parent;
};

GType			conboy_storage_get_type			(void);
ConboyStorage*	conboy_storage_new				(void);

ConboyNote*		conboy_storage_note_load		(ConboyStorage *self, const gchar *guid);
gboolean		conboy_storage_note_save		(ConboyStorage *self, ConboyNote *note);
gboolean		conboy_storage_note_delete		(ConboyStorage *self, ConboyNote *note);
GSList*			conboy_storage_note_list		(ConboyStorage *self);
GSList*			conboy_storage_note_list_ids	(ConboyStorage *self);

void					conboy_storage_set_plugin	(ConboyStorage *self, ConboyStoragePlugin *plugin);
void					conboy_storage_unset_plugin	(ConboyStorage *self);
ConboyStoragePlugin*	conboy_storage_get_plugin	(ConboyStorage *self);


#endif /* CONBOY_STORAGE_H */
