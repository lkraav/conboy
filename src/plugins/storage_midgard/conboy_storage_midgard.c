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
#include "../../conboy_storage.h"
#include "conboy_storage_midgard.h"

static GObjectClass *parent_class = NULL;

static ConboyNote*
_conboy_storage_midgard_note_load (ConboyStorage *self, gchar *uuid)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(uuid != NULL, FALSE);
	
	g_return_val_if_fail(CONBOY_IS_STORAGE_MIDGARD(self), FALSE);
	
	/* TODO: Read note from midgard */
	
	return NULL;
}

static gboolean
_conboy_storage_midgard_note_save (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_MIDGARD(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Write note to midgard */

	return FALSE;
}

static gboolean 
_conboy_storage_midgard_note_delete (ConboyStorage *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_MIDGARD(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Delete note from midgard */

	return FALSE;
}

static ConboyNote**
_conboy_storage_midgard_note_list (ConboyStorage *self, guint *n_notes)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(n_notes != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_MIDGARD(self), FALSE);	

	/* TODO: Return notes */	

	return NULL;
}

static gchar**
_conboy_storage_midgard_note_list_ids (ConboyStorage *self, guint *n_notes)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(n_notes != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_MIDGARD(self), FALSE);	

	/* TODO: Return notes' IDS */

	return NULL;
}

/* GOBJECT ROUTINES */

static GObject *
_conboy_storage_midgard_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	CONBOY_STORAGE_MIDGARD(object)->user = NULL;
	CONBOY_STORAGE_MIDGARD(object)->pass = NULL;

	return G_OBJECT(object);
}

static void
_conboy_storage_midgard_dispose(GObject *object)
{
	ConboyStorageMidgard *self = CONBOY_STORAGE_MIDGARD(object);
	
	g_free((gchar *)self->user);
	self->user = NULL;
	
	g_free((gchar *)self->pass);
	self->pass = NULL;

	parent_class->dispose(object);
}

static void
_conboy_storage_midgard_class_init (ConboyStorageMidgardClass *klass, gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = _conboy_storage_midgard_constructor;
	object_class->dispose     = _conboy_storage_midgard_dispose;

	klass->load     = _conboy_storage_midgard_note_load;
	klass->save     = _conboy_storage_midgard_note_save;
	klass->remove   = _conboy_storage_midgard_note_delete;
	klass->list     = _conboy_storage_midgard_note_list;
	klass->list_ids = _conboy_storage_midgard_note_list_ids;
}

GType
conboy_storage_midgard_get_type (void)
{
	static GType type = 0;
	if (type == 0) {

		static const GTypeInfo info = {
			sizeof (ConboyStorageMidgardClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _conboy_storage_midgard_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
 			sizeof (ConboyStorageMidgard),
			0,              /* n_preallocs */
			NULL
		};

		type = g_type_register_static (CONBOY_TYPE_STORAGE,
				"ConboyStorageMidgard", &info, 0);
	}

	return type;
}
