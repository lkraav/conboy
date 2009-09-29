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

#ifndef _CONBOY_NOTE_STORE
#define _CONBOY_NOTE_STORE

#include <gtk/gtk.h>
#include "note.h"
#include "metadata.h"
#include "conboy_note.h"
#include "conboy_storage.h"

G_BEGIN_DECLS

#define CONBOY_TYPE_NOTE_STORE conboy_note_store_get_type()

#define CONBOY_NOTE_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CONBOY_TYPE_NOTE_STORE, ConboyNoteStore))

#define CONBOY_NOTE_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_NOTE_STORE, ConboyNoteStoreClass))

#define CONBOY_IS_NOTE_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONBOY_TYPE_NOTE_STORE))

#define CONBOY_IS_NOTE_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_NOTE_STORE))

#define CONBOY_NOTE_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NOTE_TYPE_LISTSTORE, ConboyNoteStoreClass))

typedef struct {
  GtkListStore parent;
  /* <privat> */
  ConboyStorage *storage;
} ConboyNoteStore;

typedef struct {
  GtkListStoreClass parent_class;
} ConboyNoteStoreClass;

typedef enum {
	ICON_COLUMN,
	TITLE_COLUMN,
	CHANGE_DATE_COLUMN,
	NOTE_COLUMN,
	N_COLUMNS
} ConboyNoteStoreColumn;

GType conboy_note_store_get_type(void);

ConboyNoteStore*	conboy_note_store_new(void);

void				conboy_note_store_add(ConboyNoteStore *self, ConboyNote *note, GtkTreeIter *iter);
gboolean			conboy_note_store_remove(ConboyNoteStore *self, ConboyNote *note);
gboolean			conboy_note_store_get_iter(ConboyNoteStore *self, ConboyNote *note_a, GtkTreeIter *iter);
ConboyNote*			conboy_note_store_find(ConboyNoteStore *self, ConboyNote *note);
ConboyNote*			conboy_note_store_find_by_title(ConboyNoteStore *self, const gchar *title);
gint				conboy_note_store_get_length(ConboyNoteStore *self);
ConboyNote*			conboy_note_store_get_latest(ConboyNoteStore *self);
ConboyNote*			conboy_note_store_find_by_guid(ConboyNoteStore *self, const gchar *guid);
void 				conboy_note_store_note_changed(ConboyNoteStore *self, ConboyNote *note);
/*void 				conboy_note_store_fill_from_storage(ConboyNoteStore *self, ConboyStorage *storage);*/
void				conboy_note_store_set_storage(ConboyNoteStore *self, ConboyStorage *storage);

G_END_DECLS

#endif /* _CONBOY_NOTE_STORE */
