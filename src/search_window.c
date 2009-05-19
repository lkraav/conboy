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

#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-defines.h>
#include <string.h>
#include <libintl.h>


#include "search_window.h"
#include "metadata.h"
#include "note.h"
#include "note_list_store.h"

#define _(String)gettext(String)

static gboolean
is_row_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	GtkEntry *text_box = GTK_ENTRY(data);

	const gchar *text;
	gboolean visible;
	gchar *title;
	gchar *u_title;
	gchar *u_text;

	text = gtk_entry_get_text(text_box);

	if (strcmp(text, "") == 0) {
		return TRUE;
	}

	gtk_tree_model_get(model, iter, TITLE_COLUMN, &title, -1);

	u_title = g_utf8_casefold(title, -1);
	u_text  = g_utf8_casefold(text, -1);

	if (strstr(u_title, u_text) != NULL) {
		visible = TRUE;
	} else {
		visible = FALSE;
	}

	g_free(title);
	g_free(u_title);
	g_free(u_text);
	return visible;
}

static
gboolean update_search_result(gpointer user_data)
{
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(user_data));
	/* Don't call this function over and over again */
	return FALSE;
}

guint _source_id = 0;

static
void on_search_string_changed(GtkEditable *entry, gpointer user_data)
{
	/* With every change we reset the timer to 500ms. */
	if (_source_id != 0) { /* Trying to remove source with id == 0 creates runtime warning */
		g_source_remove(_source_id);
	}
	_source_id = g_timeout_add(500, update_search_result, user_data);
}

static
void on_clear_button_clicked(GtkWidget *widget, gpointer user_data)
{
	GtkEntry *entry = GTK_ENTRY(user_data);
	gtk_entry_set_text(entry, "");
	gtk_widget_grab_focus(GTK_WIDGET(entry));
}

static
void on_row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	Note *note;
	GtkTreeIter iter;
	AppData *app_data = get_app_data();
	GtkTreeModel *model = gtk_tree_view_get_model(view);

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);

	/* Close this window and show the selected note */
	gtk_widget_hide(GTK_WIDGET(app_data->search_window));

	note_show(note);
}

static
gboolean on_window_visible(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	/* Set focus on the search_field widget */
	gtk_widget_grab_focus(GTK_WIDGET(user_data));
	return FALSE;
}

static
gboolean on_hardware_key_pressed(GtkWidget *widget, GdkEventKey	*event, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET(user_data);
	AppData *app_data = get_app_data();
	GList *open_notes;

	switch (event->keyval) {

	case HILDON_HARDKEY_ESC:
		gtk_widget_hide(window);
		return TRUE;
	
	/* TODO: The same code (only last line differs) is used in callbacks.c */
	case HILDON_HARDKEY_FULLSCREEN:
		/* Toggle fullscreen */
		app_data->fullscreen = !app_data->fullscreen;

		/* Set all open windows to fullscreen or unfullscreen */
		open_notes = app_data->open_notes;
		while (open_notes != NULL) {
			Note *open_note = (Note*)open_notes->data;
			if (app_data->fullscreen) {
				gtk_window_fullscreen(GTK_WINDOW(open_note->ui->window));
			} else {
				gtk_window_unfullscreen(GTK_WINDOW(open_note->ui->window));
			}
			open_notes = open_notes->next;
		}
		/* Set search window to fullscreen or unfullscreen */
		if (app_data->search_window != NULL) {
			if (app_data->fullscreen) {
				gtk_window_fullscreen(GTK_WINDOW(app_data->search_window));
			} else {
				gtk_window_unfullscreen(GTK_WINDOW(app_data->search_window));
			}
		}

		/* Focus again this window */
		gtk_window_present(GTK_WINDOW(window));
		return TRUE;
	}
	
	return FALSE;
}

static
gboolean on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	/* TODO: We could implement here, that whenever a key is pressed, that is not
	 * up, down, enter, space, etc. the cursor jumps to the search field and the
	 * letter is entered there. This would save the user from setting the focus
	 * to the search field manually before he's able to search....
	 */

	/* Set focus on the search_field widget */
	/*
	gtk_widget_grab_focus(GTK_WIDGET(user_data));
	return FALSE;
	*/
	
	return FALSE;
}

static
void on_row_inserted (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	g_printerr("on_row_inserted() called \n");
}

static
gint compare_titles(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	gint result = 0;
	gchar *title_a, *title_b;
	gchar *u_title_a, *u_title_b;

	gtk_tree_model_get(model, a, TITLE_COLUMN, &title_a, -1);
	gtk_tree_model_get(model, b, TITLE_COLUMN, &title_b, -1);

	u_title_a = g_utf8_casefold(title_a, -1);
	u_title_b = g_utf8_casefold(title_b, -1);

	result = strcmp(u_title_a, u_title_b);

	g_free(title_a);
	g_free(title_b);
	g_free(u_title_a);
	g_free(u_title_b);

	return result;
}

static
gint compare_dates(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	Note *note_a;
	Note *note_b;

	if (a == NULL || b == NULL) {
		return 0;
	}

	gtk_tree_model_get(model, a, NOTE_COLUMN, &note_a, -1);
	gtk_tree_model_get(model, b, NOTE_COLUMN, &note_b, -1);
	
	if (note_a == NULL || note_b == NULL) {
		return 0;
	}

	return note_a->last_change_date - note_b->last_change_date;
}

static
HildonWindow* search_window_create()
{
	AppData *app_data = get_app_data();
	GtkWidget *win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *search_label;
	GtkWidget *search_field;
	GtkWidget *clear_button;
	GtkWidget *scrolledwindow;
	GtkWidget *tree;
	GtkTreeSelection *selection;
	NoteListStore *store;
	GtkTreeModel *filtered_store;
	GtkTreeModel *sorted_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *title_column;
	GtkTreeViewColumn *change_date_column;

	win = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW(win), _("Search All Notes"));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(win), vbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);

	search_label = gtk_label_new(_("Search:"));
	gtk_widget_show(search_label);
	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);
	#ifdef HILDON_HAS_APP_MENU
	search_field = hildon_entry_new();
	#else
	search_field = gtk_entry_new();
	#endif
	gtk_widget_show(search_field);
	gtk_box_pack_start(GTK_BOX(hbox), search_field, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS(search_field, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(search_field);

	clear_button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	gtk_widget_show(clear_button);
	gtk_box_pack_start(GTK_BOX(hbox), clear_button, FALSE, FALSE, 0);

	/* SCROLLED WINDOW */
	#ifdef HILDON_HAS_APP_MENU
	scrolledwindow = hildon_pannable_area_new();
	#else
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	#endif
	gtk_widget_show(scrolledwindow);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);

	/* LIST STORE */
	store = app_data->note_store;
	/* Add filter wrapper */
	filtered_store = gtk_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtered_store), is_row_visible, search_field, NULL);
	/* Add sort wrapper */
	sorted_store = gtk_tree_model_sort_new_with_model(filtered_store);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), TITLE_COLUMN, compare_titles, NULL, NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), CHANGE_DATE_COLUMN, compare_dates, NULL, NULL);

	/* TREE VIEW */
#ifdef HILDON_HAS_APP_MENU
	g_printerr("NEW code executed \n");
	tree = hildon_gtk_tree_view_new_with_model(HILDON_UI_MODE_NORMAL, GTK_TREE_MODEL(sorted_store)); /*HILDON_UI_MODE_NORMAL, HILDON_UI_MODE_EDIT */
#else
	g_printerr("OLD code executed \n");
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sorted_store));
#endif
	g_object_unref(sorted_store);
	g_object_unref(filtered_store);
	g_object_unref(store);

	gtk_widget_show(tree);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), tree);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);

	/* TREE SELECTION OBJECT */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	/* TITLE COLUMN WITH ICON */
	title_column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(title_column, _("Note"));
	gtk_tree_view_column_set_sort_column_id(title_column, TITLE_COLUMN);
	/*gtk_tree_view_column_set_sort_indicator(title_column, FALSE);*/
	gtk_tree_view_column_set_reorderable(title_column, FALSE);
	/*gtk_tree_view_column_set_sort_order(title_column, GTK_SORT_ASCENDING);*/
	gtk_tree_view_column_set_sizing(title_column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_expand(title_column, TRUE);
	/* Add icon to column */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(title_column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(title_column, renderer, "pixbuf", ICON_COLUMN);
	/* Add text to column */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(title_column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(title_column, renderer, "text", TITLE_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), title_column);

	/* CHANGE DATE COLUMN */
	renderer = gtk_cell_renderer_text_new();
	change_date_column = gtk_tree_view_column_new_with_attributes(_("Last Changed"), renderer, "text", CHANGE_DATE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(change_date_column, CHANGE_DATE_COLUMN);
	gtk_tree_view_column_set_sort_indicator(change_date_column, TRUE);
	gtk_tree_view_column_set_reorderable(change_date_column, FALSE);
	gtk_tree_view_column_set_sort_order(change_date_column, GTK_SORT_DESCENDING);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), change_date_column);
	/* Sort the using the CHANGE_DATE column */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sorted_store), CHANGE_DATE_COLUMN, GTK_SORT_DESCENDING);
	gtk_tree_sortable_sort_column_changed(GTK_TREE_SORTABLE(sorted_store));

	/* CONNECT SIGNALS */
	g_signal_connect(search_field, "changed", G_CALLBACK(on_search_string_changed), filtered_store);
	g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_button_clicked), search_field);
	/* g_signal_connect(selection, "changed", G_CALLBACK(on_selection_changed), NULL); */
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_row_activated), NULL);
	/*g_signal_connect(tree, "hildon-row-tapped", G_CALLBACK(on_row_tapped), NULL);*/

	g_signal_connect(win, "map-event", G_CALLBACK(on_window_visible), search_field);
	g_signal_connect(win, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	g_signal_connect(win, "key_press_event", G_CALLBACK(on_hardware_key_pressed), win);
	g_signal_connect(tree, "key_press_event", G_CALLBACK(on_key_pressed), search_field);

	/* TEST */
	g_signal_connect(store, "row-inserted", G_CALLBACK(on_row_inserted), NULL);

	app_data = get_app_data();
	app_data->note_store = store;

	return HILDON_WINDOW(win);
}

void search_window_open()
{
	AppData *app_data = get_app_data();

	if (app_data->search_window == NULL) {
		app_data->search_window = search_window_create();
	}
	
	if (app_data->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(app_data->search_window));
	}

	gtk_widget_show(GTK_WIDGET(app_data->search_window));
	gtk_window_present(GTK_WINDOW(app_data->search_window));
}

