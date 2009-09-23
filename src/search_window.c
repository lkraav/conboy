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
#include "localisation.h"

#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-helper.h>
#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-pannable-area.h>
#include <hildon/hildon-entry.h>
#include <hildon/hildon-gtk.h>
#include <gtk/gtkenums.h>
#endif
#include <string.h>

#include "app_data.h"
#include "search_window.h"
#include "metadata.h"
#include "note.h"
#include "conboy_note_store.h"
#include "settings.h"
#include "search.h"

typedef struct {
	GtkWidget          *search_field;
	GtkWidget          *hbox;
	GtkTreeViewColumn  *change_date_column;
	GHashTable         *search_result;
	GtkTreeModelFilter *filtered_model;
} SearchWindowData;

/**
 * This is the filter function for the tree view.
 */
static gboolean
is_row_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	SearchWindowData *data = (SearchWindowData*)user_data;
	GHashTable *search_result = data->search_result;
	ConboyNote *note;

	/* In case the search field does not exist yet, or containts no text, show all */
	if (data->search_field == NULL) {
		return TRUE;
	}

	if (strcmp(gtk_entry_get_text(GTK_ENTRY(data->search_field)), "") == 0) {
		return TRUE;
	}

	gtk_tree_model_get(model, iter, NOTE_COLUMN, &note, -1);

	if (note == NULL) {
		return FALSE;
	}

	if (search_result == NULL) {
		return TRUE;
	}

	if (g_hash_table_lookup(search_result, note)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static
gboolean update_search_result(gpointer user_data)
{
	SearchWindowData *data = (SearchWindowData*) user_data;
	const gchar *query = gtk_entry_get_text(GTK_ENTRY(data->search_field));

	search(query, data->search_result);

	gtk_tree_model_filter_refilter(data->filtered_model);

	return FALSE; /* Don't call this function over and over again */
}

guint _source_id = 0;

static
void on_search_string_changed(GtkEditable *entry, SearchWindowData *data)
{
	/* With every change we reset the timer to 500ms. */
	if (_source_id != 0) { /* Trying to remove source with id == 0 creates runtime warning */
		g_source_remove(_source_id);
	}
	_source_id = g_timeout_add(500, update_search_result, data);
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
	ConboyNote *note;
	GtkTreeIter iter;
	AppData *app_data = app_data_get();
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

static gboolean
on_delete_event(GtkWidget *window, GdkEvent *event, gpointer user_data)
{
	/* If no notes are open and the search window is closed, exit Conboy. */
	gtk_widget_hide(window);
	/*
	AppData *app_data = app_data_get();
	if (g_list_length(app_data->open_notes) == 0) {
		gtk_main_quit();
	} else {
		gtk_widget_hide(window);
	}
	*/
	return TRUE;
}

static
gboolean on_hardware_key_pressed(GtkWidget *widget, GdkEventKey	*event, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET(user_data);
	AppData *app_data = app_data_get();
	GList *open_windows;

	switch (event->keyval) {

	case HILDON_HARDKEY_ESC:
		gtk_widget_hide(window);
		return TRUE;

	/* TODO: Duplicated code The same code (only last line differs) is used in callbacks.c */
	case HILDON_HARDKEY_FULLSCREEN:
		/* Toggle fullscreen */
		app_data->fullscreen = !app_data->fullscreen;

		/* Set all open windows to fullscreen or unfullscreen */
		if (app_data->fullscreen) {
			gtk_window_fullscreen(GTK_WINDOW(app_data->note_window->window));
		} else {
			gtk_window_unfullscreen(GTK_WINDOW(app_data->note_window->window));
		}
		/*
		open_windows = app_data->open_windows;
		while (open_windows != NULL) {
			UserInterface *open_window = (UserInterface*)open_windows->data;
			if (app_data->fullscreen) {
				gtk_window_fullscreen(GTK_WINDOW(open_window->window));
			} else {
				gtk_window_unfullscreen(GTK_WINDOW(open_window->window));
			}
			open_windows = open_windows->next;
		}
		*/
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

#ifdef HILDON_HAS_APP_MENU
static
void on_sort_by_date_changed(GtkToggleButton *button, GtkTreeSortable *sortable)
{
	if (gtk_toggle_button_get_active(button)) {
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), CHANGE_DATE_COLUMN, GTK_SORT_DESCENDING);
	}
}

static
void on_sort_by_title_changed(GtkToggleButton *button, GtkTreeSortable *sortable)
{
	if (gtk_toggle_button_get_active(button)) {
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), TITLE_COLUMN, GTK_SORT_ASCENDING);
	}
}
#endif

static
void on_orientation_changed(GdkScreen *screen, SearchWindowData *data)
{
	GtkTreeViewColumn *column;
	GtkWidget *hbox;
	AppData *app_data = app_data_get();

	column = data->change_date_column;
	hbox = data->hbox;

	app_data->portrait = is_portrait_mode();

	if (app_data->portrait) {
		gtk_tree_view_column_set_visible(column, FALSE);
		gtk_widget_hide(hbox);
	} else {
		gtk_tree_view_column_set_visible(column, TRUE);
		gtk_widget_show(hbox);
	}
}

/* TODO: Duplicated code. It's also in interface.c */
static void
on_scrollbar_settings_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GtkScrolledWindow *win = GTK_SCROLLED_WINDOW(user_data);

	SettingsScrollbarSize size = gconf_value_get_int(entry->value);
	if (size == SETTINGS_SCROLLBAR_SIZE_BIG) {
		hildon_helper_set_thumb_scrollbar(win, TRUE);
	} else {
		hildon_helper_set_thumb_scrollbar(win, FALSE);
	}
}

static void
on_new_note_action_activated(GtkAction *action, gpointer user_data)
{
	GtkWidget *window = GTK_WIDGET(user_data);
	gtk_widget_hide(window);
	ConboyNote *note = conboy_note_new();
	note_show(note);
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
	ConboyNote *note_a, *note_b;
	gint date_a, date_b;

	if (a == NULL || b == NULL) {
		return 0;
	}

	gtk_tree_model_get(model, a, NOTE_COLUMN, &note_a, -1);
	gtk_tree_model_get(model, b, NOTE_COLUMN, &note_b, -1);

	if (note_a == NULL || note_b == NULL) {
		return 0;
	}

	g_object_get(note_a, "change-date", &date_a, NULL);
	g_object_get(note_b, "change-date", &date_b, NULL);

	return date_a - date_b;
}

static
HildonWindow* search_window_create(SearchWindowData *window_data)
{
	AppData *app_data = app_data_get();
	GtkWidget *win;
	GtkWidget *menu;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *search_label;
	GtkWidget *search_field;
	GtkWidget *clear_button;
	GtkWidget *scrolledwindow;
	GtkWidget *tree;
#ifdef HILDON_HAS_APP_MENU
	GtkWidget *button_sort_by_title;
	GtkWidget *button_sort_by_date;
#endif
	GtkTreeSelection *selection;
	ConboyNoteStore *store;
	GtkTreeModel *filtered_store;
	GtkTreeModel *sorted_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *title_column;
	GtkTreeViewColumn *change_date_column;
	GdkScreen *screen;
	GHashTable *search_result;
	GtkAction *new_note_action;
	GtkWidget *menu_new_note;
	win = hildon_stackable_window_new();
	gtk_window_set_title(GTK_WINDOW(win), _("Search All Notes"));
	screen = gdk_screen_get_default();
	search_result = g_hash_table_new(NULL, NULL);

	new_note_action = GTK_ACTION(gtk_action_new("new", _("New Note"), NULL, NULL));

	/* Window menu */
#ifdef HILDON_HAS_APP_MENU
	menu = hildon_app_menu_new();

	/* Add New Note item */
	menu_new_note = gtk_button_new();
	gtk_action_connect_proxy(new_note_action, menu_new_note);
	hildon_app_menu_append(HILDON_APP_MENU(menu), GTK_BUTTON(menu_new_note));

	/* Add sort filters */
	button_sort_by_title = gtk_radio_button_new_with_label(NULL, _("Sort By Title"));
	button_sort_by_date = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button_sort_by_title), _("Sort By Date"));

	/* Draw them as toggle buttons, not as radio buttons */
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_sort_by_title), FALSE);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button_sort_by_date), FALSE);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_sort_by_date), TRUE);

	hildon_app_menu_add_filter(HILDON_APP_MENU(menu), GTK_BUTTON(button_sort_by_title));
	hildon_app_menu_add_filter(HILDON_APP_MENU(menu), GTK_BUTTON(button_sort_by_date));

	hildon_window_set_app_menu(HILDON_WINDOW(win), HILDON_APP_MENU(menu));
#else
	menu = gtk_menu_new();
	menu_new_note = gtk_action_create_menu_item(new_note_action);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_new_note);
	hildon_window_set_menu(HILDON_WINDOW(win), GTK_MENU(menu));
#endif

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(win), vbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);

	#ifdef HILDON_HAS_APP_MENU
	search_field = hildon_entry_new(HILDON_SIZE_FINGER_HEIGHT);
	#else
	search_label = gtk_label_new(_("Search:"));
	gtk_widget_show(search_label);
	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);
	search_field = gtk_entry_new();
	#endif
	gtk_widget_show(search_field);
	gtk_box_pack_start(GTK_BOX(hbox), search_field, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS(search_field, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(search_field);

	clear_button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	#ifdef HILDON_HAS_APP_MENU
	hildon_gtk_widget_set_theme_size(clear_button, HILDON_SIZE_FINGER_HEIGHT);
	#endif
	gtk_widget_show(clear_button);
	gtk_box_pack_start(GTK_BOX(hbox), clear_button, FALSE, FALSE, 0);

	if (app_data->portrait) {
		gtk_widget_hide(hbox);
	} else {
		gtk_widget_show(hbox);
	}

	/* SCROLLED WINDOW */
	#ifdef HILDON_HAS_APP_MENU
	scrolledwindow = hildon_pannable_area_new();
	#else
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if (settings_load_scrollbar_size() == SETTINGS_SCROLLBAR_SIZE_BIG) {
		hildon_helper_set_thumb_scrollbar(GTK_SCROLLED_WINDOW(scrolledwindow), TRUE);
	}
	#endif
	gtk_widget_show(scrolledwindow);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);



	window_data->search_result = search_result;

	/* LIST STORE */
	store = app_data->note_store;
	/* Add filter wrapper */
	filtered_store = gtk_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtered_store), is_row_visible, window_data, NULL);
	/* Add sort wrapper */
	sorted_store = gtk_tree_model_sort_new_with_model(filtered_store);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), TITLE_COLUMN, compare_titles, NULL, NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(sorted_store), CHANGE_DATE_COLUMN, compare_dates, NULL, NULL);

	/* TREE VIEW */
#ifdef HILDON_HAS_APP_MENU
	tree = hildon_gtk_tree_view_new_with_model(HILDON_UI_MODE_NORMAL, GTK_TREE_MODEL(sorted_store)); /*HILDON_UI_MODE_NORMAL, HILDON_UI_MODE_EDIT */
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
#else
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sorted_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);
#endif
	g_object_unref(sorted_store);
	g_object_unref(filtered_store);
	g_object_unref(store);

	gtk_widget_show(tree);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), tree);

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);

	/* TREE SELECTION OBJECT */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	/* TITLE COLUMN WITH ICON */
	title_column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(title_column, _("Note"));
	gtk_tree_view_column_set_sort_column_id(title_column, TITLE_COLUMN);
	gtk_tree_view_column_set_reorderable(title_column, FALSE);
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

	/* CHANGE_DATE COLUMN */
	renderer = gtk_cell_renderer_text_new();
	change_date_column = gtk_tree_view_column_new_with_attributes(_("Last Changed"), renderer, "text", CHANGE_DATE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(change_date_column, CHANGE_DATE_COLUMN);
	gtk_tree_view_column_set_sort_indicator(change_date_column, TRUE);
	gtk_tree_view_column_set_reorderable(change_date_column, FALSE);
	gtk_tree_view_column_set_sort_order(change_date_column, GTK_SORT_DESCENDING);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), change_date_column);
	/* Sort the using the CHANGE_DATE column */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sorted_store), CHANGE_DATE_COLUMN, GTK_SORT_DESCENDING);

	/* Don't show column if we are in portrait mode */
	gtk_tree_view_column_set_visible(change_date_column, !app_data->portrait);

	/* Fill window_data */
	window_data->change_date_column = change_date_column;
	window_data->hbox = hbox;
	window_data->search_field = search_field;
	window_data->filtered_model = GTK_TREE_MODEL_FILTER(filtered_store);


	/* CONNECT SIGNALS */
	g_signal_connect(search_field, "changed", G_CALLBACK(on_search_string_changed), window_data);
	g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_button_clicked), search_field);
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_row_activated), NULL);
	g_signal_connect(win, "map-event", G_CALLBACK(on_window_visible), search_field);
	g_signal_connect(win, "delete-event", G_CALLBACK(on_delete_event), NULL);
	g_signal_connect(win, "key_press_event", G_CALLBACK(on_hardware_key_pressed), win);
	g_signal_connect(tree, "key_press_event", G_CALLBACK(on_key_pressed), search_field);
	g_signal_connect(screen, "size-changed", G_CALLBACK(on_orientation_changed), window_data);
	g_signal_connect(new_note_action, "activate", G_CALLBACK(on_new_note_action_activated), win);

	gconf_client_notify_add(app_data->client, SETTINGS_SCROLLBAR_SIZE, on_scrollbar_settings_changed, scrolledwindow, NULL, NULL);

#ifdef HILDON_HAS_APP_MENU
	g_signal_connect(button_sort_by_title, "toggled", G_CALLBACK(on_sort_by_title_changed), sorted_store);
	g_signal_connect(button_sort_by_date, "toggled", G_CALLBACK(on_sort_by_date_changed), sorted_store);
#endif

	app_data = app_data_get();
	app_data->note_store = store;

	return HILDON_WINDOW(win);
}

void search_window_open()
{
	AppData *app_data = app_data_get();
	SearchWindowData *window_data;

	if (app_data->search_window == NULL) {
		window_data = g_new0(SearchWindowData, 1);
		app_data->search_window = search_window_create(window_data);
	}

	if (app_data->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(app_data->search_window));
	}

	gtk_widget_show(GTK_WIDGET(app_data->search_window));
	gtk_window_present(GTK_WINDOW(app_data->search_window));
}

