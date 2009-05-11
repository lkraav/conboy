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

#ifndef _NOTE_LIST_STORE
#define _NOTE_LIST_STORE

#include <gtk/gtk.h>
#include "note.h"
#include "metadata.h"

G_BEGIN_DECLS

#define NOTE_TYPE_LIST_STORE note_list_store_get_type()

#define NOTE_LIST_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NOTE_TYPE_LIST_STORE, NoteListStore))

#define NOTE_LIST_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NOTE_TYPE_LIST_STORE, NoteListStoreClass))

#define NOTE_IS_LIST_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NOTE_TYPE_LIST_STORE))

#define NOTE_IS_LIST_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NOTE_TYPE_LIST_STORE))

#define NOTE_LIST_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NOTE_TYPE_LISTSTORE, NoteListStoreClass))

typedef struct {
  GtkListStore parent;
} NoteListStore;

typedef struct {
  GtkListStoreClass parent_class;
} NoteListStoreClass;

typedef enum {
	ICON_COLUMN,
	TITLE_COLUMN,
	CHANGE_DATE_COLUMN,
	NOTE_COLUMN,
	N_COLUMNS
} NoteListStoreColumn;

GType note_list_store_get_type(void);

NoteListStore *note_list_store_new(void);
void note_list_store_add(NoteListStore *self, Note *note, GtkTreeIter *iter);
gboolean note_list_store_remove(NoteListStore *self, Note *note);
gboolean note_list_store_get_iter(NoteListStore *self, Note *note_a, GtkTreeIter *iter);
Note *note_list_store_find(NoteListStore *self, Note *note);
Note *note_list_store_find_by_title(NoteListStore *self, const gchar *title);
gint note_list_store_get_length(NoteListStore *self);
Note *note_list_store_get_latest(NoteListStore *self);
void note_list_store_note_changed(NoteListStore *self, Note *note);

G_END_DECLS

#endif /* _NOTE_LIST_STORE */
