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

static GObjectClass *parent_class = NULL;

/* GOBJECT ROUTINES */

static GObject *
_conboy_note_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	ConboyNote *note = CONBOY_NOTE(object);
	note->guid = NULL;
	note->content = NULL;
	note->title = NULL;
	
	return G_OBJECT(object);
}

static void
_conboy_note_class_dispose(GObject *object)
{
	ConboyNote *self = CONBOY_NOTE(object);
	
	g_free(self->guid);
	self->guid = NULL;

	g_free(self->title);
	self->title = NULL;

	g_free(self->content);
	self->content = NULL;

	parent_class->dispose(object);
}

static void
_conboy_note_class_init (ConboyNoteClass *klass, gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = _conboy_note_constructor;
	object_class->dispose = _conboy_note_class_dispose;
}

GType
conboy_note_get_type (void)
{
	static GType type = 0;
	if (type == 0) {

		static const GTypeInfo info = {
			sizeof (ConboyNoteClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _conboy_note_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
 			sizeof (ConboyNote),
			0,              /* n_preallocs */
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
				"ConboyNote", &info, 0);
	}

	return type;
}
