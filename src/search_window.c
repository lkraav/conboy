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

#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-defines.h>
#include <string.h>

#include "search_window.h"
#include "metadata.h"
#include "note.h"

enum
{
	ICON_COLUMN,
	TITLE_COLUMN,
	CHANGE_DATE_COLUMN,
	NOTE_COLUMN,
	N_COLUMNS
};

/* TODO: Should not be global */
GdkPixbuf *note_icon = NULL;

static
void popuplate_list_model(GtkListStore* store) {
	
	GtkTreeIter iter;
	AppData *app_data = get_app_data();
	GList *all_notes = app_data->all_notes;
	GDate *date = g_date_new();
	gchar date_str[20];
	
	if (note_icon == NULL) {
		/* TODO: Don't use absolut path here */
		note_icon = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/26x26/hildon/conboy.png", NULL);
		if (note_icon == NULL) {
			/* TODO: Return some default missing image icon */ 
		}
	}
	
	while (all_notes != NULL) {
		Note *note = (Note*)all_notes->data;
		
		g_date_set_time_t(date, note->last_change_date);
		g_date_strftime(date_str, 20, "%x", date); /* time not possible with this function */
		
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, ICON_COLUMN, note_icon,
				                         TITLE_COLUMN, note->title,
				                         CHANGE_DATE_COLUMN, date_str,
				                         NOTE_COLUMN, note,
				                         -1);
		all_notes = all_notes->next;
	}
	g_free(date);
}

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

/*
static
void on_selection_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		AppData *app_data = get_app_data();
		Note *note = NULL;
		
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
	
		gtk_widget_hide(GTK_WIDGET(app_data->search_window));
		note_show(note);
	}
}
*/

static
void on_row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(view);
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		AppData *app_data = get_app_data();
		Note *note = NULL;
		
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
	
		/* Close this window and show the selected note */
		gtk_widget_hide(GTK_WIDGET(app_data->search_window));
		note_show(note);
	}
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
	
	switch (event->keyval) {
		
	case HILDON_HARDKEY_ESC:
		gtk_widget_hide(window);
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
	gint result = 0;
	Note *note_a;
	Note *note_b;
	
	gtk_tree_model_get(model, a, NOTE_COLUMN, &note_a, -1);
	gtk_tree_model_get(model, b, NOTE_COLUMN, &note_b, -1);
	
	return note_a->last_change_date - note_b->last_change_date;
}

static
HildonWindow* search_window_create()
{
	GtkWidget *win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *search_label;
	GtkWidget *search_field;
	GtkWidget *clear_button;
	GtkWidget *scrolledwindow;
	GtkWidget *tree;
	GtkTreeSelection *selection;
	GtkListStore *store;
	GtkTreeModel *filtered_store;
	GtkTreeModel *sorted_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *title_column;
	GtkTreeViewColumn *change_date_column;
	
	win = hildon_window_new();
	gtk_window_set_title(GTK_WINDOW(win), "Search All Notes");
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(win), vbox);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);
	
	search_label = gtk_label_new("Search:");
	gtk_widget_show(search_label);
	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);
	
	search_field = gtk_entry_new();
	gtk_widget_show(search_field);
	gtk_box_pack_start(GTK_BOX(hbox), search_field, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS(search_field, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(search_field);
	
	clear_button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	gtk_widget_show(clear_button);
	gtk_box_pack_start(GTK_BOX(hbox), clear_button, FALSE, FALSE, 0);
	
	/* SCROLLED WINDOW */
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwindow);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	/* LIST STORE */
	store = gtk_list_store_new(N_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	popuplate_list_model(store);
	/* Add filter wrapper */
	filtered_store = gtk_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtered_store), is_row_visible, search_field, NULL);
	/* Add sort wrapper */
	sorted_store = gtk_tree_model_sort_new_with_model(filtered_store);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), TITLE_COLUMN, compare_titles, NULL, NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), CHANGE_DATE_COLUMN, compare_dates, NULL, NULL);
	
	
	/* TREE VIEW */
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sorted_store));
	g_object_unref(G_OBJECT(store)); /* <<<< TODO: ???? Should I unref all stores??? */
	gtk_widget_show(tree);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), tree);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);
	
	/* TREE SELECTION OBJECT */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	
	/* TITLE COLUMN WITH ICON */
	title_column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(title_column, "Note");
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
	change_date_column = gtk_tree_view_column_new_with_attributes("Last Changed", renderer, "text", CHANGE_DATE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(change_date_column, CHANGE_DATE_COLUMN);
	gtk_tree_view_column_set_sort_indicator(change_date_column, TRUE);
	gtk_tree_view_column_set_reorderable(change_date_column, FALSE);
	gtk_tree_view_column_set_sort_order(change_date_column, GTK_SORT_DESCENDING);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), change_date_column);
	
	
	/* CONNECT SIGNALS */
	g_signal_connect(search_field, "changed", G_CALLBACK(on_search_string_changed), filtered_store);
	g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_button_clicked), search_field);
	/*g_signal_connect(selection, "changed", G_CALLBACK(on_selection_changed), NULL);*/
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_row_activated), NULL);
	g_signal_connect(win, "map-event", G_CALLBACK(on_window_visible), search_field);
	g_signal_connect(win, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	g_signal_connect(win, "key_press_event", G_CALLBACK(on_hardware_key_pressed), win);
	g_signal_connect(tree, "key_press_event", G_CALLBACK(on_key_pressed), search_field);
	
	return HILDON_WINDOW(win);
}

void search_window_open()
{
	AppData *app_data = get_app_data();
	
	if (app_data->search_window == NULL) {
		app_data->search_window = search_window_create();
	}
	
	gtk_widget_show(GTK_WIDGET(app_data->search_window));
	gtk_window_present(GTK_WINDOW(app_data->search_window));
}
