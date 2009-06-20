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

#include "conboy_note.h"
#include "conboy_storage.h"

static GObjectClass *parent_class = NULL;

gboolean
conboy_storage_note_save (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	ConboyStorageClass *klass = CONBOY_STORAGE_GET_CLASS(self);

	return klass->save(self, note);
}

gboolean 
conboy_storage_note_delete (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	ConboyStorageClass *klass = CONBOY_STORAGE_GET_CLASS(self);

	return klass->save(self, note);
}

ConboyNote**
conboy_storage_note_list (ConboyStorage *self, guint *n_notes)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(n_notes != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);	
	
	ConboyStorageClass *klass = CONBOY_STORAGE_GET_CLASS(self);

	return klass->list(self, n_notes);
}

gchar**
conboy_storage_note_list_ids (ConboyStorage *self, guint *n_notes)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(n_notes != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE(self), FALSE);	
	
	ConboyStorageClass *klass = CONBOY_STORAGE_GET_CLASS(self);

	return klass->list(self, n_notes);
}

/* GOBJECT ROUTINES */

static GObject *
_conboy_storage_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);
	
	return G_OBJECT(object);
}


static void
_conboy_storage_class_dispose(GObject *object)
{
	ConboyStorage *self = CONBOY_STORAGE(object);
	parent_class->dispose(object);
}

static void
_conboy_storage_class_init (ConboyStorageClass *klass, gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = _conboy_storage_constructor;
	object_class->dispose = _conboy_storage_class_dispose;

	klass->save = NULL;
	klass->remove = NULL;
	klass->list = NULL;
	klass->list_ids = NULL;
}

GType
conboy_storage_get_type (void)
{
	static GType type = 0;
	if (type == 0) {

		static const GTypeInfo info = {
			sizeof (ConboyStorageClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _conboy_storage_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
 			sizeof (ConboyStorage),
			0,              /* n_preallocs */
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
				"ConboyStorage", &info, G_TYPE_FLAG_ABSTRACT);
	}

	return type;
}
