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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>

#include "note.h"
#include "metadata.h"
#include "conboy_storage.h"
#include "conboy_note.h"

#include "conboy_note_store.h"

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

/*
 * Implementation of the interface
 */
static void		conboy_note_store_tree_model_iface_init(GtkTreeModelIface *iface);
static int		conboy_note_store_get_n_columns(GtkTreeModel *self);
static GType	conboy_note_store_get_column_type(GtkTreeModel *self, int column);
static void		conboy_note_store_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value);


/* ConboyNoteStore inherits from GtkListStore, and implements the GtkTreeStore
 * interface */
G_DEFINE_TYPE_EXTENDED(ConboyNoteStore, conboy_note_store, GTK_TYPE_LIST_STORE, 0,
		G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL,
			conboy_note_store_tree_model_iface_init))

/* our parent's model iface */
static GtkTreeModelIface parent_iface = { 0, };

/* this method is called once to set up the class */
static void
conboy_note_store_class_init (ConboyNoteStoreClass *klass)
{
}

/* this method is called once to set up the interface */
static void
conboy_note_store_tree_model_iface_init(GtkTreeModelIface *iface)
{
	/* this is where we override the interface methods */
	/* first make a copy of our parent's interface to call later */
	parent_iface = *iface;

	/* now put in our own overriding methods */
	iface->get_n_columns 	= conboy_note_store_get_n_columns;
	iface->get_column_type 	= conboy_note_store_get_column_type;
	iface->get_value 		= conboy_note_store_get_value;
}

/* this method is called every time an instance of the class is created */
static void
conboy_note_store_init (ConboyNoteStore *self)
{
	/* initialise the underlying storage for the GtkListStore */
	GType types[] = { CONBOY_TYPE_NOTE };
	gtk_list_store_set_column_types(GTK_LIST_STORE(self), 1, types);

	self->storage = NULL;
	self->max_title_length = 0;
}

/**
 * my_list_store_add:
 * @self: the #MyListStore
 * @obj: a #MyTestObject to add to the list
 * @iter: a pointer to a #GtkTreeIter for the row (or NULL)
 *
 * Appends @obj to the list.
 */
void
conboy_note_store_add(ConboyNoteStore *self, ConboyNote *note, GtkTreeIter *iter)
{
	GtkTreeIter iter1;

	/* validate our parameters */
	g_return_if_fail(CONBOY_IS_NOTE_STORE(self));
	g_return_if_fail(CONBOY_IS_NOTE(note));

	/* put this object in our data storage */
	gtk_list_store_append(GTK_LIST_STORE(self), &iter1);
	gtk_list_store_set(GTK_LIST_STORE(self), &iter1, 0, note, -1);

	/* TODO: If Note would be a gobject, then here we could
	 * connect signals to recognize whenever the Note itself was
	 * changed. If such a change would occure we could update the
	 * coresponding row. */
	
	/* Find out if title of the newly added note is longer then the currently longest */
	self->max_title_length = max (g_utf8_strlen(note->title, -1), self->max_title_length);

	/* return the iter if the user cares */
	if (iter) *iter = iter1;
}

/* retreive an object from our parent's data storage,
 * unref the returned object when done */
static ConboyNote*
conboy_note_store_get_object(ConboyNoteStore *self, GtkTreeIter *iter)
{
	GValue value = {0, };
	ConboyNote *note;

	/* validate our parameters */
	g_return_val_if_fail(CONBOY_IS_NOTE_STORE(self), NULL);
	g_return_val_if_fail(iter != NULL, NULL);

	/* retreive the object using our parent's interface, take our own
	 * reference to it */
	parent_iface.get_value(GTK_TREE_MODEL(self), iter, 0, &value);

	note = CONBOY_NOTE(g_value_dup_object(&value));

	g_value_unset(&value);

	return note;
}

/* this method returns the number of columns in our tree model */
static int
conboy_note_store_get_n_columns(GtkTreeModel *self)
{
	return N_COLUMNS;
}

/* this method returns the type of each column in our tree model */
static GType
conboy_note_store_get_column_type(GtkTreeModel *self, int column)
{
	/* This is, how it should look to the outside. So we have an icon, title, change_date and the note itself */
	GType types[] = {
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_STRING,
		CONBOY_TYPE_NOTE,
	};

	/* validate our parameters */
	g_return_val_if_fail(CONBOY_IS_NOTE_STORE(self), G_TYPE_INVALID);
	g_return_val_if_fail(column >= 0 && column < N_COLUMNS, G_TYPE_INVALID);

	return types[column];
}

/* Returns newly allocated string. Needs to be freed later */
static gchar*
get_date_string(time_t t)
{
	gchar date_str[20];
	GDate *date = g_date_new();

	g_date_set_time_t(date, t);
	g_date_strftime(date_str, 20, "%x", date);
	g_free(date);

	return g_strdup(date_str);
}


GdkPixbuf *_icon = NULL; /* TODO: Should not be global. Make it member of this Class. */
/* this method retrieves the value for a particular column */
static void
conboy_note_store_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value)
{
	ConboyNote *note;
	if (_icon == NULL) {
#ifdef HILDON_HAS_APP_MENU
		_icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/48x48/hildon/conboy.png", NULL);
#else
		_icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/26x26/hildon/conboy.png", NULL);
#endif
	}

	/* validate our parameters */
	g_return_if_fail(CONBOY_IS_NOTE_STORE(self));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(column >= 0 && column < N_COLUMNS);
	g_return_if_fail(value != NULL);

	/* get the object from our parent's storage */
	note = conboy_note_store_get_object(CONBOY_NOTE_STORE(self), iter);

	/* initialise our GValue to the required type */
	g_value_init(value, conboy_note_store_get_column_type(GTK_TREE_MODEL (self), column));

	switch (column)
	{
		case ICON_COLUMN:
			/* the object itself was requested */
			g_value_set_object(value, _icon);
			break;

		case TITLE_COLUMN:
			/* the notes title was requested */
			if (note != NULL) {
				gchar *title;
				g_object_get(note, "title", &title, NULL);
				g_value_set_string(value, title);
				g_free(title);
			} else {
				g_value_set_string(value, "");
			}
			break;

		case CHANGE_DATE_COLUMN:
			if (note != NULL) {
				gint date;
				g_object_get(note, "change-date", &date, NULL);
				g_value_set_string(value, get_date_string(date));
			} else {
				g_value_set_string(value, "");
			}
			break;

		case NOTE_COLUMN:
			g_value_set_object(value, note);
			break;

		default:
			g_assert_not_reached ();
	}

	/* release the reference gained from my_list_store_get_object() */
	if (note) {
		g_object_unref(note);
	}
}

/*
 * Own stuff
 */

ConboyNoteStore*
conboy_note_store_new(void)
{
	return g_object_new(CONBOY_TYPE_NOTE_STORE, NULL);
}

static gint
find_longest_title(ConboyNoteStore *self)
{
	gint max_len = 0;
	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL(self);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);

	while (valid) {
		ConboyNote *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
		max_len = max(g_utf8_strlen(note->title, -1), max_len);
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	return max_len;
}

gboolean
conboy_note_store_remove(ConboyNoteStore *self, ConboyNote *note)
{
	GtkTreeIter iter;

	if (conboy_note_store_get_iter(self, note, &iter)) {
		gtk_list_store_remove(GTK_LIST_STORE(self), &iter);
		
		/* If the note with the longest title was removed, we need to find out what the longest title is now */
		if (g_utf8_strlen(note->title, -1) == self->max_title_length) {
			self->max_title_length = find_longest_title(self);
		}
		
		return TRUE;
	}
	
	return FALSE;
}

gboolean
conboy_note_store_get_iter(ConboyNoteStore *self, ConboyNote *note_a, GtkTreeIter *iter)
{
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), iter)) do
	{
		ConboyNote *note_b;
		gtk_tree_model_get(GTK_TREE_MODEL(self), iter, NOTE_COLUMN, &note_b, -1);
		if (note_a == note_b) {
			return TRUE;
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), iter));

	return FALSE;
}

ConboyNote*
conboy_note_store_find(ConboyNoteStore *self, ConboyNote *note_a)
{
	ConboyNote *note_b;
	GtkTreeIter iter;
	if (conboy_note_store_get_iter(self, note_a, &iter)) {
		gtk_tree_model_get(GTK_TREE_MODEL(self), &iter, NOTE_COLUMN, &note_b, -1);
		return note_b;
	}
	return NULL;
}

ConboyNote*
conboy_note_store_find_by_title(ConboyNoteStore *self, const gchar *title)
{
	GtkTreeIter iter;
	gchar *title_a = g_utf8_casefold(title, -1);

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		ConboyNote *note;
		gchar *title_b;
		gtk_tree_model_get(GTK_TREE_MODEL(self), &iter, NOTE_COLUMN, &note, -1);
		title_b = g_utf8_casefold(note->title, -1);
		if (strcmp(title_a, title_b) == 0) {
			g_free(title_a);
			g_free(title_b);
			return note;
		}
		g_free(title_b);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), &iter));

	g_free(title_a);
	return NULL;
}

gint
conboy_note_store_get_length(ConboyNoteStore *self)
{
	gint count = 0;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		count++;
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), &iter));

	return count;
}

ConboyNote*
conboy_note_store_get_latest(ConboyNoteStore *self)
{
	GtkTreeIter iter;
	ConboyNote *latest_note = NULL;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		ConboyNote *note;
		gtk_tree_model_get(GTK_TREE_MODEL(self), &iter, NOTE_COLUMN, &note, -1);
		if (latest_note != NULL) {
			if (note->last_change_date > latest_note->last_change_date) {
				latest_note = note;
			}
		} else {
			latest_note = note;
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), &iter));

	return latest_note;
}

void
conboy_note_store_note_changed(ConboyNoteStore *self, ConboyNote *note)
{
	GtkTreeIter iter;
	if (conboy_note_store_get_iter(self, note, &iter)) {
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(self), &iter);
		gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, &iter);
		gtk_tree_path_free(path);
	}
}


static void
on_storage_activated(ConboyStorage *storage, ConboyNoteStore *self)
{
	if (conboy_note_store_get_length(self) > 0) {
		g_printerr("ERROR: Storage activated, but already notes in notes store\n");
		gtk_list_store_clear(GTK_LIST_STORE(self));
	}

	/* Add all notes from Storage to NoteStore */
	GSList *notes = conboy_storage_note_list(storage);
	while (notes) {
		conboy_note_store_add(self, notes->data, NULL);
		notes = notes->next;
	}
}

static void
on_storage_deactivated(ConboyStorage *storage, ConboyNoteStore *self)
{
	gtk_list_store_clear(GTK_LIST_STORE(self));
}

void
conboy_note_store_set_storage(ConboyNoteStore *self, ConboyStorage *storage) {

	g_return_if_fail(self != NULL);
	g_return_if_fail(storage != NULL);

	g_return_if_fail(CONBOY_IS_NOTE_STORE(self));
	g_return_if_fail(CONBOY_IS_STORAGE(storage));

	self->storage = storage;
	g_object_ref(storage);

	GSList *notes = conboy_storage_note_list(storage);
	while (notes) {
		conboy_note_store_add(self, notes->data, NULL);
		notes = notes->next;
	}

	g_signal_connect(storage, "activated",   G_CALLBACK(on_storage_activated),   self);
	g_signal_connect(storage, "deactivated", G_CALLBACK(on_storage_deactivated), self);
}

ConboyNote*
conboy_note_store_find_by_guid(ConboyNoteStore *self, const gchar *guid)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(guid != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_NOTE_STORE(self), NULL);
	
	GtkTreeIter iter;
	ConboyNote *result = NULL;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		ConboyNote *note;
		gtk_tree_model_get(GTK_TREE_MODEL(self), &iter, NOTE_COLUMN, &note, -1);
		gchar *tmp_guid;
		g_object_get(note, "guid", &tmp_guid, NULL);
		if (strcmp(tmp_guid, guid) == 0) {
			result = note;
			g_free(tmp_guid);
			break;
		}
		g_free(tmp_guid);

	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), &iter));

	return result;
}


