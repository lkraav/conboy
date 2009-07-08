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

#ifndef CONBOY_STORAGE_MIDGARD_H
#define CONBOY_STORAGE_MIDGARD_H

#include <glib-object.h>

/* convention macros */
#define CONBOY_TYPE_STORAGE_MIDGARD (conboy_storage_midgard_get_type())
#define CONBOY_STORAGE_MIDGARD(object) (G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_STORAGE_MIDGARD, ConboyStorageMidgard))
#define CONBOY_STORAGE_MIDGARD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_STORAGE_MIDGARD, ConboyStorageMidgardClass))
#define CONBOY_IS_STORAGE_MIDGARD(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_STORAGE_MIDGARD))
#define CONBOY_IS_STORAGE_MIDGARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_STORAGE_MIDGARD))
#define CONBOY_STORAGE_MIDGARD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_STORAGE_MIDGARD, ConboyStorageMidgardClass))

typedef struct _ConboyStorageMidgard ConboyStorageMidgard;
typedef struct _ConboyStorageMidgardClass ConboyStorageMidgardClass;

struct _ConboyStorageMidgard {
	GObject parent;

	/* <private> */
	const gchar *user;
	const gchar *pass;
};

struct _ConboyStorageMidgardClass {
	GObjectClass parent;
	
	ConboyNote*  (*load)     (ConboyStorage *self, gchar *uuid);
	gboolean	 (*save)     (ConboyStorage *self, ConboyNote *note);
	gboolean	 (*remove)   (ConboyStorage *self, ConboyNote *note);
	ConboyNote** (*list)     (ConboyStorage *self, guint *n_notes);
	gchar**      (*list_ids) (ConboyStorage *self, guint *n_notes);	
};

GType		conboy_storage_midgard_get_type			(void);

#endif /* CONBOY_STORAGE_MIDGARD_H */
