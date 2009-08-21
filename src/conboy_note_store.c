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

#include "app_data.h"
#include "note.h"
#include "metadata.h"
#include "conboy_note_store.h"

/*
 * Implementation of the interface
 */
static void conboy_note_store_tree_model_iface_init(GtkTreeModelIface *iface);
static int conboy_note_store_get_n_columns(GtkTreeModel *self);
static GType conboy_note_store_get_column_type(GtkTreeModel *self, int column);
static void conboy_note_store_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value);

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
	iface->get_n_columns = conboy_note_store_get_n_columns;
	iface->get_column_type = conboy_note_store_get_column_type;
	iface->get_value = conboy_note_store_get_value;
}

/* this method is called every time an instance of the class is created */
static void
conboy_note_store_init (ConboyNoteStore *self)
{
	/* initialise the underlying storage for the GtkListStore */
	GType types[] = { G_TYPE_POINTER };

	gtk_list_store_set_column_types(GTK_LIST_STORE(self), 1, types);
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
conboy_note_store_add(ConboyNoteStore *self, Note *note, GtkTreeIter *iter)
{
	GtkTreeIter iter1;

	/* validate our parameters */
	g_return_if_fail(NOTE_IS_LIST_STORE(self));
	/*g_return_if_fail(NOTE_IS_NOTE(note));*/

	/* put this object in our data storage */
	gtk_list_store_append(GTK_LIST_STORE(self), &iter1);
	gtk_list_store_set(GTK_LIST_STORE(self), &iter1, 0, note, -1);

	/* TODO: If Note would be a gobject, then here we could
	 * connect signals to recognize whenever the Note itself was
	 * changed. If such a change would occure we could update the
	 * coresponding row. */

	/* return the iter if the user cares */
	if (iter) *iter = iter1;
}

/* retreive an object from our parent's data storage,
 * unref the returned object when done */
static Note*
conboy_note_store_get_object(ConboyNoteStore *self, GtkTreeIter *iter)
{
	GValue value = {0};
	Note *note;

	/* validate our parameters */
	g_return_val_if_fail(NOTE_IS_LIST_STORE(self), NULL);
	g_return_val_if_fail(iter != NULL, NULL);

	/* retreive the object using our parent's interface, take our own
	 * reference to it */
	parent_iface.get_value(GTK_TREE_MODEL(self), iter, 0, &value);

	note = (Note*)g_value_get_pointer(&value); /*(g_value_dup_object(&value));*/

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
		G_TYPE_POINTER,
	};

	/* validate our parameters */
	g_return_val_if_fail(NOTE_IS_LIST_STORE(self), G_TYPE_INVALID);
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


GdkPixbuf *_icon = NULL; /* TODO: Should not be global */
/* this method retrieves the value for a particular column */
static void
conboy_note_store_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value)
{
	Note *note;
	if (_icon == NULL) {
#ifdef HILDON_HAS_APP_MENU
		_icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/40x40/hildon/conboy.png", NULL);
#else
		_icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/26x26/hildon/conboy.png", NULL);
#endif
	}

	/* validate our parameters */
	g_return_if_fail(NOTE_IS_LIST_STORE(self));
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
				g_value_set_string(value, note->title);
			} else {
				g_value_set_string(value, "");
			}
			break;

		case CHANGE_DATE_COLUMN:
			if (note != NULL) {
				g_value_set_string(value, get_date_string(note->last_change_date));
			} else {
				g_value_set_string(value, "");
			}
			break;

		case NOTE_COLUMN:
			g_value_set_pointer(value, note);
			break;

		default:
			g_assert_not_reached ();
	}

	/* release the reference gained from my_list_store_get_object() */
	/*g_object_unref (obj);*/
}

/*
 * Own stuff
 */

ConboyNoteStore *conboy_note_store_new(void)
{
	return g_object_new(NOTE_TYPE_LIST_STORE, NULL);
}

gboolean conboy_note_store_remove(ConboyNoteStore *self, Note *note)
{
	GtkTreeIter iter;

	if (conboy_note_store_get_iter(self, note, &iter)) {
		gtk_list_store_remove(GTK_LIST_STORE(self), &iter);
		return TRUE;
	}
	return FALSE;
}

gboolean conboy_note_store_get_iter(ConboyNoteStore *self, Note *note_a, GtkTreeIter *iter)
{
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), iter)) do
	{
		Note *note_b;
		gtk_tree_model_get(GTK_TREE_MODEL(self), iter, NOTE_COLUMN, &note_b, -1);
		if (note_a == note_b) {
			return TRUE;
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), iter));

	return FALSE;
}

Note *conboy_note_store_find(ConboyNoteStore *self, Note *note_a)
{
	Note *note_b;
	GtkTreeIter iter;
	if (conboy_note_store_get_iter(self, note_a, &iter)) {
		gtk_tree_model_get(GTK_TREE_MODEL(self), &iter, NOTE_COLUMN, &note_b, -1);
		return note_b;
	}
	return NULL;
}

Note *conboy_note_store_find_by_title(ConboyNoteStore *self, const gchar *title)
{
	GtkTreeIter iter;
	gchar *title_a = g_utf8_casefold(title, -1);

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		Note *note;
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

gint conboy_note_store_get_length(ConboyNoteStore *self)
{
	gint count = 0;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		count++;
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(self), &iter));

	return count;
}

Note *conboy_note_store_get_latest(ConboyNoteStore *self)
{
	GtkTreeIter iter;
	Note *latest_note = NULL;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self), &iter)) do {
		Note *note;
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

void conboy_note_store_note_changed(ConboyNoteStore *self, Note *note)
{
	GtkTreeIter iter;
	if (conboy_note_store_get_iter(self, note, &iter)) {
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(self), &iter);
		gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, &iter);
		gtk_tree_path_free(path);
	}
}

