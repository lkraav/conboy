/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * Conboy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Conboy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Conboy. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONBOY_NOTE_BUFFER_H_
#define CONBOY_NOTE_BUFFER_H_


#include <glib-object.h>
#include <gtk/gtk.h>


/* convention macros */
#define CONBOY_TYPE_NOTE_BUFFER				(conboy_note_buffer_get_type())
#define CONBOY_NOTE_BUFFER(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBuffer))
#define CONBOY_NOTE_BUFFER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBufferClass))
#define CONBOY_IS_NOTE_BUFFER(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_NOTE_BUFFER))
#define CONBOY_IS_NOTE_BUFFER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_NOTE_BUFFER))
#define CONBOY_NOTE_BUFFER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_NOTE_BUFFER, ConboyNoteBufferClass))

typedef struct _ConboyNoteBuffer			ConboyNoteBuffer;
typedef struct _ConboyNoteBufferClass		ConboyNoteBufferClass;

struct _ConboyNoteBuffer {
	GtkTextBuffer parent;
	/* <private> */
	GSList *active_tags;
};

struct _ConboyNoteBufferClass {
	GtkTextBufferClass parent_class;
};


GType				conboy_note_buffer_get_type (void);

ConboyNoteBuffer*	conboy_note_buffer_new(void);

void	conboy_note_buffer_add_active_tag				(ConboyNoteBuffer *self, GtkTextTag *tag);
void	conboy_note_buffer_remove_active_tag			(ConboyNoteBuffer *self, GtkTextTag *tag);
void	conboy_note_buffer_add_active_tag_by_name		(ConboyNoteBuffer *self, const gchar *tag_name);
void	conboy_note_buffer_remove_active_tag_by_name	(ConboyNoteBuffer *self, const gchar *tag_name);

/**
 * The list belongs to the ConboyNoteBuffer, don't free or change it.
 */
GSList*	conboy_note_buffer_get_active_tags				(ConboyNoteBuffer *self);
void	conboy_note_buffer_set_active_tags				(ConboyNoteBuffer *self, GSList *tags);
void	conboy_note_buffer_clear_active_tags			(ConboyNoteBuffer *self);
void    conboy_note_buffer_update_active_tags           (ConboyNoteBuffer *self);

GtkTextTag* conboy_note_buffer_find_depth_tag           (GtkTextIter* iter);
GtkTextTag* conboy_note_buffer_get_depth_tag            (ConboyNoteBuffer *buffer, gint depth);

void conboy_note_buffer_increase_indent(ConboyNoteBuffer *buffer, gint start_line, gint end_line);
void conboy_note_buffer_decrease_indent(ConboyNoteBuffer *buffer, gint start_line, gint end_line);

void conboy_note_buffer_enable_bullets                  (ConboyNoteBuffer *buffer);
void conboy_note_buffer_disable_bullets                 (ConboyNoteBuffer *buffer);

void     conboy_note_buffer_check_selection             (ConboyNoteBuffer *buffer);
gboolean conboy_note_buffer_backspace_handler           (ConboyNoteBuffer *buffer);
gboolean conboy_note_buffer_delete_handler              (ConboyNoteBuffer *buffer);
gboolean conboy_note_buffer_add_new_line                (UserInterface *ui);

void     conboy_note_buffer_fix_list_tags               (ConboyNoteBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter);

gchar*	conboy_note_buffer_get_xml				(ConboyNoteBuffer *self);
void	conboy_note_buffer_set_xml				(ConboyNoteBuffer *self, const gchar *xmlString);

#endif /*CONBOY_NOTE_BUFFER_H_*/
