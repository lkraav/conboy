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

#ifndef CONBOY_NOTE_H
#define CONBOY_NOTE_H

#include <glib-object.h>

/* convention macros */
#define CONBOY_TYPE_NOTE (conboy_note_get_type())
#define CONBOY_NOTE(object)  (G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_NOTE, ConboyNote))
#define CONBOY_NOTE_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_NOTE, ConboyNoteClass))
#define CONBOY_IS_NOTE(object)   (G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_NOTE))
#define CONBOY_IS_NOTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_NOTE))
#define CONBOY_NOTE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_NOTE, ConboyNoteClass))

typedef struct _ConboyNote ConboyNote;
typedef struct _ConboyNoteClass ConboyNoteClass;

struct _ConboyNote{
	GObject parent;
	
	/* <private> */
	/* identifiers */
	gchar *guid;
	gchar *path;
	
	/* note */
	gchar *content;
	gchar *title;

	/* metadata */
	gboolean open_on_startup;	
	time_t changed;
	time_t created;

	/* ui ? */	
	gint cursor_position;
	gint width;
	gint height;
	gint x;
	gint y;
	
	const gchar *version;
};

struct _ConboyNoteClass{
	GObjectClass parent;
	
	/* signals */
	
};

GType		conboy_note_get_type		(void);

#endif /* CONBOY_NOTE_H */