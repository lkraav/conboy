/* This file is part of Conboy.
 *
 * Copyright (C) 2009 Cornelius Hald
 *
 * This file is based on gedit-plugin-manager.c from gedit (GPLv2)
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi
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
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include "conboy_plugin_info.h"
#include "conboy_config.h"
#include "app_data.h"
#include "conboy_plugin_manager_row.h"

#include "conboy_plugin_manager.h"

enum
{
	ACTIVE_COLUMN,
	AVAILABLE_COLUMN,
	INFO_COLUMN,
	NUM_COLUMNS
};

#define PLUGIN_MANAGER_NAME_TITLE _("Plugin")
#define PLUGIN_MANAGER_ACTIVE_TITLE _("Enabled")

#define CONBOY_PLUGIN_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerPrivate))

struct _ConboyPluginManagerPrivate
{
	GList		*rows;

	/* Diablo stuff */
	GtkWidget	*tree;
	GtkWidget	*about_button;
	GtkWidget	*configure_button;
	GtkWidget 	*about;
	GtkWidget	*popup_menu;
};

G_DEFINE_TYPE(ConboyPluginManager, conboy_plugin_manager, GTK_TYPE_VBOX)

static ConboyPluginInfo *plugin_manager_get_selected_plugin (ConboyPluginManager *pm);
static void plugin_manager_toggle_active (ConboyPluginManager *pm, GtkTreeIter *iter, GtkTreeModel *model);
static void conboy_plugin_manager_finalize (GObject *object);

static void
conboy_plugin_manager_class_init (ConboyPluginManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = conboy_plugin_manager_finalize;

	g_type_class_add_private (object_class, sizeof (ConboyPluginManagerPrivate));
}

static void
about_button_cb (GtkWidget *button, ConboyPluginManager *pm)
{
	ConboyPluginInfo *info = plugin_manager_get_selected_plugin (pm);

	g_return_if_fail (info != NULL);

	/* if there is another about dialog already open destroy it */
	if (pm->priv->about)
		gtk_widget_destroy (pm->priv->about);

	pm->priv->about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
		"name", conboy_plugin_info_get_name (info),
		"copyright", conboy_plugin_info_get_copyright (info),
		"authors", conboy_plugin_info_get_authors (info),
		"comments", conboy_plugin_info_get_description (info),
		"version", conboy_plugin_info_get_version (info),
		NULL);

	gtk_window_set_destroy_with_parent (GTK_WINDOW (pm->priv->about), TRUE);

	g_signal_connect (pm->priv->about,
			  "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);
	g_signal_connect (pm->priv->about,
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &pm->priv->about);

	gtk_window_set_modal(GTK_WINDOW(pm->priv->about), TRUE);
	gtk_widget_show (pm->priv->about);
}

static void
configure_button_cb (GtkWidget *button, ConboyPluginManager *pm)
{

	g_printerr("INFO: configure button pressed\n");
	g_return_if_fail(pm != NULL);
	g_return_if_fail(CONBOY_IS_PLUGIN_MANAGER(pm));

	ConboyPluginInfo *info = plugin_manager_get_selected_plugin (pm);


	if (!conboy_plugin_info_is_configurable(info)) {
		g_printerr("ERROR: Plugin is not configurable");
		return;
	}

	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Plugin settings"),
			NULL,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_OK,
			NULL);

	GtkWidget *content_area = GTK_DIALOG(dialog)->vbox;

	if (info->plugin == NULL) {
		g_printerr("ERROR: PLUGIN IS NULL\n");
		return;
	}

	GtkWidget *settings = conboy_plugin_get_settings_widget(info->plugin);
	if (settings == NULL) {
		g_printerr("ERROR: Settings widget it NULL\n");
		return;
	}
	gtk_widget_show(settings);

	/* Add the widget to the dialog */
	gtk_box_pack_start(GTK_BOX(content_area), settings, TRUE, TRUE, 10);

	/* When a button (ok/cancel/etc.) is clicked or the dialog is closed - destroy it */
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));
}

static void
plugin_manager_view_info_cell_cb (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer   *cell,
				  GtkTreeModel      *tree_model,
				  GtkTreeIter       *iter,
				  gpointer           data)
{
	ConboyPluginInfo *info;
	gchar *text;

	g_return_if_fail (tree_model != NULL);
	g_return_if_fail (tree_column != NULL);

	gtk_tree_model_get (tree_model, iter, INFO_COLUMN, &info, -1);

	if (info == NULL)
		return;

	text = g_markup_printf_escaped ("<b>%s</b>\n%s",
					conboy_plugin_info_get_name (info),
					conboy_plugin_info_get_description (info));
	g_object_set (G_OBJECT (cell),
		      "markup", text,
		      "sensitive", conboy_plugin_info_is_available (info),
		      NULL);

	g_free (text);
}


static void
active_toggled_cb (GtkCellRendererToggle *cell,
		   gchar                 *path_str,
		   ConboyPluginManager    *pm)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	path = gtk_tree_path_new_from_string (path_str);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);

	if (&iter != NULL) {
		plugin_manager_toggle_active (pm, &iter, model);
	}

	gtk_tree_path_free (path);
}

static void
cursor_changed_cb (GtkTreeView *view,
		   gpointer     data)
{
	ConboyPluginManager *pm = data;
	ConboyPluginInfo *info = plugin_manager_get_selected_plugin (pm);

	gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->about_button),
				  info != NULL);
	gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->configure_button),
				   conboy_plugin_info_is_configurable (info));
}

static void
row_activated_cb (GtkTreeView       *tree_view,
		  GtkTreePath       *path,
		  GtkTreeViewColumn *column,
		  gpointer           data)
{
	ConboyPluginManager *pm = data;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);

	g_return_if_fail (&iter != NULL);

	plugin_manager_toggle_active (pm, &iter, model);
}


static void
conboy_plugin_activated_deactivated_cb (ConboyPluginStore *store, ConboyPluginInfo *info, ConboyPluginManager *pm)
{
#ifndef HILDON_HAS_APP_MENU

	GtkTreeModel *model;
	GtkTreeIter iter;

	g_return_if_fail(info != NULL);
	g_return_if_fail(pm != NULL);

	g_return_if_fail(CONBOY_IS_PLUGIN_INFO(info));
	g_return_if_fail(CONBOY_IS_PLUGIN_MANAGER(pm));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pm->priv->tree));

	gtk_tree_model_get_iter_first(model, &iter);

	/* Iterate over all rows. Once we found the row containing the given ConboyPluginInfo
	 * we set the active flag of this row */
	do {
		ConboyPluginInfo *tinfo;
		gtk_tree_model_get(model, &iter, INFO_COLUMN, &tinfo, -1);
		if (tinfo == info) {
			break;
		}
	}
	while (gtk_tree_model_iter_next(model, &iter));

	/* Set the active flag. This will reflect in the UI */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, ACTIVE_COLUMN, conboy_plugin_info_is_active (info), -1);
#endif
}

static void
plugin_manager_populate_lists (ConboyPluginManager *pm)
{
	GList *plugins;
	GtkListStore *model;
	GtkTreeIter iter;

	AppData *app_data = app_data_get();

	plugins = conboy_plugin_store_get_plugin_infos(app_data->plugin_store);

	model = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree)));

	while (plugins)
	{
		ConboyPluginInfo *info = (ConboyPluginInfo*) plugins->data;
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				    ACTIVE_COLUMN, conboy_plugin_info_is_active (info),
				    AVAILABLE_COLUMN, conboy_plugin_info_is_available (info),
				    INFO_COLUMN, info,
				    -1);

		plugins = plugins->next;
	}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		GtkTreeSelection *selection;
		ConboyPluginInfo* info;

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
		g_return_if_fail (selection != NULL);

		gtk_tree_selection_select_iter (selection, &iter);

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
				    INFO_COLUMN, &info, -1);

		gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->configure_button),
					  conboy_plugin_info_is_configurable (info));
	}
}

/**
 * TODO: Add here to code to load a plugin if we decided to
 * load them during runtime - which we should if we want that
 * a plugin can offer a Settings-Dialog.
 *
 * Or add code here to save to GConf which plugins are active
 */
static void
plugin_manager_set_active (ConboyPluginManager *pm,
			   GtkTreeIter        *iter,
			   GtkTreeModel       *model,
			   gboolean            active)
{
	ConboyPluginInfo *info;

	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);

	g_return_if_fail (info != NULL);

	if (active) {
		/* activate the plugin */
		conboy_plugin_info_activate_plugin(info);

	} else {
		/* deactivate the plugin */
		conboy_plugin_info_deactivate_plugin(info);
	}

}

static void
plugin_manager_toggle_active (ConboyPluginManager *pm,
			      GtkTreeIter        *iter,
			      GtkTreeModel       *model)
{
	gboolean active;

	gtk_tree_model_get (model, iter, ACTIVE_COLUMN, &active, -1);
	active ^= 1;
	plugin_manager_set_active (pm, iter, model, active);
}

static ConboyPluginInfo *
plugin_manager_get_selected_plugin (ConboyPluginManager *pm)
{
	ConboyPluginInfo *info = NULL;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_val_if_fail (model != NULL, NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
	g_return_val_if_fail (selection != NULL, NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_tree_model_get (model, &iter, INFO_COLUMN, &info, -1);
	}

	return info;
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel *model,
		gint          column,
		const gchar  *key,
		GtkTreeIter  *iter,
		gpointer      data)
{
	ConboyPluginInfo *info;
	gchar *normalized_string;
	gchar *normalized_key;
	gchar *case_normalized_string;
	gchar *case_normalized_key;
	gint key_len;
	gboolean retval;

	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);
	if (!info)
		return FALSE;

	normalized_string = g_utf8_normalize (conboy_plugin_info_get_name (info), -1, G_NORMALIZE_ALL);
	normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);
	case_normalized_string = g_utf8_casefold (normalized_string, -1);
	case_normalized_key = g_utf8_casefold (normalized_key, -1);

	key_len = strlen (case_normalized_key);

	/* Oddly enough, this callback must return whether to stop the search
	 * because we found a match, not whether we actually matched.
	 */
	retval = (strncmp (case_normalized_key, case_normalized_string, key_len) != 0);

	g_free (normalized_key);
	g_free (normalized_string);
	g_free (case_normalized_key);
	g_free (case_normalized_string);

	return retval;
}

static gboolean
button_press_event_cb (GtkWidget          *tree,
		       GdkEventButton     *event,
		       ConboyPluginManager *pm)
{
	/* We want the treeview selection to be updated before showing the menu.
	 * This code is evil, thanks to Federico Mena Quintero's black magic.
	 * See: http://mail.gnome.org/archives/gtk-devel-list/2006-February/msg00168.html
	 * FIXME: Let's remove it asap.
	 */

	static gboolean in_press = FALSE;
	gboolean handled;

	if (in_press)
		return FALSE; /* we re-entered */

	if (GDK_BUTTON_PRESS != event->type || 3 != event->button)
		return FALSE; /* let the normal handler run */

	in_press = TRUE;
	handled = gtk_widget_event (tree, (GdkEvent *) event);
	in_press = FALSE;

	if (!handled)
		return FALSE;

	return TRUE;
}


static gint
model_name_sort_func (GtkTreeModel *model,
		      GtkTreeIter  *iter1,
		      GtkTreeIter  *iter2,
		      gpointer      user_data)
{
	ConboyPluginInfo *info1, *info2;

	gtk_tree_model_get (model, iter1, INFO_COLUMN, &info1, -1);
	gtk_tree_model_get (model, iter2, INFO_COLUMN, &info2, -1);

	return g_utf8_collate (
			conboy_plugin_info_get_name (info1),
			conboy_plugin_info_get_name (info2));
}

static void
plugin_manager_construct_tree (ConboyPluginManager *pm)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkListStore *model;

	model = gtk_list_store_new (NUM_COLUMNS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

	gtk_tree_view_set_model (GTK_TREE_VIEW (pm->priv->tree),
				 GTK_TREE_MODEL (model));
	g_object_unref (model);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pm->priv->tree), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pm->priv->tree), FALSE);

	/* first column */
	cell = gtk_cell_renderer_toggle_new ();
	g_object_set (cell, "xpad", 6, NULL);
	g_signal_connect (cell,
			  "toggled",
			  G_CALLBACK (active_toggled_cb),
			  pm);
	column = gtk_tree_view_column_new_with_attributes (PLUGIN_MANAGER_ACTIVE_TITLE,
							   cell,
							   "active",
							   ACTIVE_COLUMN,
							   "activatable",
							   AVAILABLE_COLUMN,
							   "sensitive",
							   AVAILABLE_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pm->priv->tree), column);

	/* second column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, PLUGIN_MANAGER_NAME_TITLE);
	gtk_tree_view_column_set_resizable (column, TRUE);

	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
						 plugin_manager_view_info_cell_cb,
						 pm, NULL);


	gtk_tree_view_column_set_spacing (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pm->priv->tree), column);

	/* Sort on the plugin names */
	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model),
	                                         model_name_sort_func,
        	                                 NULL,
                	                         NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
					      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					      GTK_SORT_ASCENDING);

	/* Enable search for our non-string column */
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (pm->priv->tree),
					 INFO_COLUMN);

	gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (pm->priv->tree),
					     name_search_cb,
					     NULL,
					     NULL);

	g_signal_connect (pm->priv->tree,
			  "cursor_changed",
			  G_CALLBACK (cursor_changed_cb),
			  pm);

	g_signal_connect (pm->priv->tree,
			  "row_activated",
			  G_CALLBACK (row_activated_cb),
			  pm);

	g_signal_connect (pm->priv->tree,
			  "button-press-event",
			  G_CALLBACK (button_press_event_cb),
			  pm);

	gtk_widget_show (pm->priv->tree);
}


static void
conboy_plugin_manager_init (ConboyPluginManager *pm)
{
#ifdef HILDON_HAS_APP_MENU

	pm->priv = CONBOY_PLUGIN_MANAGER_GET_PRIVATE (pm);
	pm->priv->rows = NULL;

	AppData *app_data = app_data_get();
	ConboyPluginStore *plugin_store = app_data->plugin_store;

	GList *infos = conboy_plugin_store_get_plugin_infos(plugin_store);

	while (infos) {
		/* Create row */
		ConboyPluginInfo *info = CONBOY_PLUGIN_INFO(infos->data);
		GtkWidget *row = conboy_plugin_manager_row_new(info);
		gtk_widget_show(row);
		gtk_box_pack_start(GTK_BOX(pm), row, TRUE, TRUE, 0);
		pm->priv->rows = g_list_prepend(pm->priv->rows, row);

		infos = infos->next;
	}



#else
	GtkWidget *label;
	GtkWidget *alignment;
	GtkWidget *viewport;
	GtkWidget *hbuttonbox;
	gchar *markup;

	pm->priv = CONBOY_PLUGIN_MANAGER_GET_PRIVATE (pm);

	gtk_box_set_spacing (GTK_BOX (pm), 6);

	label = gtk_label_new (NULL);
	markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>",
					  _("Active plugins"));
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

	gtk_box_pack_start (GTK_BOX (pm), label, FALSE, TRUE, 0);

	alignment = gtk_alignment_new (0., 0., 1., 1.);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);
	gtk_box_pack_start (GTK_BOX (pm), alignment, TRUE, TRUE, 0);

	viewport = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (viewport),
					     GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (alignment), viewport);

	pm->priv->tree = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (viewport), pm->priv->tree);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (pm), hbuttonbox, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox), 8);

	pm->priv->about_button = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
	gtk_button_set_label (GTK_BUTTON(pm->priv->about_button), _("About plugin"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), pm->priv->about_button);

	pm->priv->configure_button = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
	gtk_button_set_label (GTK_BUTTON(pm->priv->configure_button), _("Configure plugin"));
	gtk_container_add (GTK_CONTAINER (hbuttonbox), pm->priv->configure_button);

	/* setup a window of a sane size. */
	gtk_widget_set_size_request (GTK_WIDGET (viewport), 350, 200);

	g_signal_connect (pm->priv->about_button,
			  "clicked",
			  G_CALLBACK (about_button_cb),
			  pm);
	g_signal_connect (pm->priv->configure_button,
			  "clicked",
			  G_CALLBACK (configure_button_cb),
			  pm);

	plugin_manager_construct_tree (pm);


	AppData *app_data = app_data_get();
	ConboyPluginStore *plugin_store = app_data->plugin_store;

	g_signal_connect(plugin_store, "plugin-activated",   G_CALLBACK(conboy_plugin_activated_deactivated_cb), pm);
	g_signal_connect(plugin_store, "plugin-deactivated", G_CALLBACK(conboy_plugin_activated_deactivated_cb), pm);

	plugin_manager_populate_lists(pm);

	gtk_widget_show_all(GTK_WIDGET(pm));
#endif
}

static void
conboy_plugin_manager_finalize (GObject *object)
{
	ConboyPluginManager *pm = CONBOY_PLUGIN_MANAGER (object);

	AppData *app_data = app_data_get();
	ConboyPluginStore *plugin_store = app_data->plugin_store;
	g_signal_handlers_disconnect_by_func(plugin_store, conboy_plugin_activated_deactivated_cb, pm);

	G_OBJECT_CLASS (conboy_plugin_manager_parent_class)->finalize (object);

}

GtkWidget *conboy_plugin_manager_new (void)
{
	return g_object_new (CONBOY_TYPE_PLUGIN_MANAGER, NULL);
}

