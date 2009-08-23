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

#include "conboy_plugin_manager.h"
#include "conboy_plugin_info.h"
#include "conboy_config.h"

enum
{
	ACTIVE_COLUMN,
	AVAILABLE_COLUMN,
	INFO_COLUMN,
	N_COLUMNS
};

#define PLUGIN_MANAGER_NAME_TITLE _("Plugin")
#define PLUGIN_MANAGER_ACTIVE_TITLE _("Enabled")

#define CONBOY_PLUGIN_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerPrivate))

struct _ConboyPluginManagerPrivate
{
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
	ConboyPluginInfo *info;

	/*conboy_debug (DEBUG_PLUGINS);*/

	info = plugin_manager_get_selected_plugin (pm);

	g_return_if_fail (info != NULL);

	/* if there is another about dialog already open destroy it */
	if (pm->priv->about)
		gtk_widget_destroy (pm->priv->about);

	pm->priv->about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
		"name", conboy_plugin_info_get_name (info),
		"copyright", conboy_plugin_info_get_copyright (info),
		"authors", conboy_plugin_info_get_authors (info),
		"comments", conboy_plugin_info_get_description (info),
		/*"website", conboy_plugin_info_get_website (info),*/
		/*"logo-icon-name", conboy_plugin_info_get_icon_name (info),*/
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
configure_button_cb (GtkWidget          *button,
		     ConboyPluginManager *pm)
{
	ConboyPluginInfo *info;
	/*GtkWindow *toplevel;*/

	/*conboy_debug (DEBUG_PLUGINS);*/

	info = plugin_manager_get_selected_plugin (pm);

	g_return_if_fail (info != NULL);

	/*
	conboy_debug_message (DEBUG_PLUGINS, "Configuring: %s\n", 
			     conboy_plugin_info_get_name (info));
	*/
	
	g_printerr("Not yet implemented: Configuring: %s\n", conboy_plugin_info_get_name (info));
	
	/*
	toplevel = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(pm)));
	 */
	/*
	conboy_plugins_engine_configure_plugin (pm->priv->engine,
					       info, toplevel);
					       */
	/*
	conboy_debug_message (DEBUG_PLUGINS, "Done");
	*/	
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
/*
static void
plugin_manager_view_icon_cell_cb (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer   *cell,
				  GtkTreeModel      *tree_model,
				  GtkTreeIter       *iter,
				  gpointer           data)
{
	ConboyPluginInfo *info;
	
	g_return_if_fail (tree_model != NULL);
	g_return_if_fail (tree_column != NULL);

	gtk_tree_model_get (tree_model, iter, INFO_COLUMN, &info, -1);

	if (info == NULL)
		return;

//	g_object_set (G_OBJECT (cell),
//		      "icon-name", conboy_plugin_info_get_icon_name (info),
//		      "sensitive", conboy_plugin_info_is_available (info),
//		      NULL);

}*/


static void
active_toggled_cb (GtkCellRendererToggle *cell,
		   gchar                 *path_str,
		   ConboyPluginManager    *pm)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	g_printerr("Toggled\n");
	/*conboy_debug (DEBUG_PLUGINS);*/

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
	ConboyPluginInfo *info;

	/*conboy_debug (DEBUG_PLUGINS);*/

	info = plugin_manager_get_selected_plugin (pm);

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
	GtkTreeModel *model;

	/*conboy_debug (DEBUG_PLUGINS);*/

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);

	g_return_if_fail (&iter != NULL);

	plugin_manager_toggle_active (pm, &iter, model);
}

/**
 * Returns a GList of all ConboyPluginInfo objects found in the given
 * plugin_base_dir or one level deeper in the directory hierarchy.
 * 
 * Looks at all files in plugin_base_dir if those are .plugin files,
 * ConboyPluginInfo objects are created. Also all directories of
 * plugin_base_dir are searched.
 */
static GList*
plugin_manager_get_all_plugins (const gchar *plugin_base_dir)
{
	/*
	 * for each file with .plugin ending create ConboyPluginInfo
	 */
	GList *result = NULL;
	
	const gchar *filename;
	GDir *dir = g_dir_open(plugin_base_dir, 0, NULL);
	while ((filename = g_dir_read_name(dir)) != NULL) {
		gchar *full_path = g_build_filename(plugin_base_dir, filename, NULL);
		
		/* If it's a dir, check if it contains .plugin files */
		if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
			const gchar *inner_filename;
			GDir *inner_dir = g_dir_open(full_path, 0, NULL);
			while ((inner_filename = g_dir_read_name(inner_dir)) != NULL) {
				g_printerr("Check: %s\n", inner_filename);
				if (g_str_has_suffix(inner_filename, ".plugin")) {
					gchar *plugin_file = g_build_filename(full_path, inner_filename, NULL);
					ConboyPluginInfo *info = conboy_plugin_info_new(plugin_file);
					result = g_list_prepend(result, info);
					g_free(plugin_file);
				}
			}
			g_dir_close(inner_dir);
			
		/* If it's a file, check if it's a .plugin file */
		} else if (g_file_test(full_path, G_FILE_TEST_EXISTS)) {
			g_printerr("Check: %s\n", full_path);
			if (g_str_has_suffix(full_path, ".plugin")) {
				ConboyPluginInfo *info = conboy_plugin_info_new(full_path);
				result = g_list_prepend(result, info);
			}
		}
		
		g_free(full_path);
	}
	
	g_dir_close(dir);

	return result;
}

/*
 * Returns the path set by the environment variable
 * CONBOY_PLUGIN_DIR or if not set, the default path
 * $prefix/lib/conboy
 * 
 * Return value needs to be freed
 */
static gchar*
plugin_manager_get_plugin_base_dir()
{
	gchar *path = NULL;
	const gchar *env_path = g_getenv("CONBOY_PLUGIN_DIR");
	if (env_path != NULL) {
		if (g_file_test(env_path, G_FILE_TEST_IS_DIR)) {
			path = g_strdup(env_path);
		} else {
			g_printerr("WARN: '%s' is not a directory or does not exist. Please set the environment variable CONBOY_PLUGIN_DIR correctly. Trying default.\n", path);
		}
	} else {
		path = g_build_filename(PREFIX, "/lib/conboy");
	}
	
	return path;
}

static void
plugin_manager_populate_lists (ConboyPluginManager *pm)
{
	const GList *plugins;
	GtkListStore *model;
	GtkTreeIter iter;

	/*conboy_debug (DEBUG_PLUGINS);*/

	/*plugins = conboy_plugins_engine_get_plugin_list (pm->priv->engine);*/
	
	gchar *plugin_base_dir = plugin_manager_get_plugin_base_dir();
	plugins = plugin_manager_get_all_plugins(plugin_base_dir);

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
static gboolean
plugin_manager_set_active (ConboyPluginManager *pm,
			   GtkTreeIter        *iter,
			   GtkTreeModel       *model,
			   gboolean            active)
{
	ConboyPluginInfo *info;
	gboolean res = TRUE;
	
	/*conboy_debug (DEBUG_PLUGINS);*/

	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);

	g_return_val_if_fail (info != NULL, FALSE);

	if (active) {
		/* activate the plugin */
		return conboy_plugin_info_activate_plugin(info);
		
	} else {
		/* deactivate the plugin */
		return conboy_plugin_info_deactivate_plugin(info);
	}

	return res;
}

static void
plugin_manager_toggle_active (ConboyPluginManager *pm,
			      GtkTreeIter        *iter,
			      GtkTreeModel       *model)
{
	gboolean active;
	
	/*conboy_debug (DEBUG_PLUGINS);*/
	g_printerr("plugin_manager_toggle_active\n");

	gtk_tree_model_get (model, iter, ACTIVE_COLUMN, &active, -1);
	active ^= 1;
	plugin_manager_set_active (pm, iter, model, active);
}

static ConboyPluginInfo *
plugin_manager_get_selected_plugin (ConboyPluginManager *pm)
{
	ConboyPluginInfo *info = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	/*conboy_debug (DEBUG_PLUGINS);*/

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_val_if_fail (model != NULL, NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
	g_return_val_if_fail (selection != NULL, NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_tree_model_get (model, &iter, INFO_COLUMN, &info, -1);
	}
	
	return info;
}

/*
static void
plugin_manager_set_active_all (ConboyPluginManager *pm,
			       gboolean            active)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	conboy_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));

	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter_first (model, &iter);

	do {
		plugin_manager_set_active (pm, &iter, model, active);
	}
	while (gtk_tree_model_iter_next (model, &iter));
}
*/

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

/*
static void
enable_plugin_menu_cb (GtkMenu            *menu,
		       ConboyPluginManager *pm)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (model != NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
	g_return_if_fail (selection != NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
		plugin_manager_toggle_active (pm, &iter, model);
}
*/

/*
static void
enable_all_menu_cb (GtkMenu            *menu,
		    ConboyPluginManager *pm)
{
	plugin_manager_set_active_all (pm, TRUE);
}
*/

/*
static void
disable_all_menu_cb (GtkMenu            *menu,
		     ConboyPluginManager *pm)
{
	plugin_manager_set_active_all (pm, FALSE);
}
*/

/*
static GtkWidget *
create_tree_popup_menu (ConboyPluginManager *pm)
{
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *image;
	ConboyPluginInfo *info;

	info = plugin_manager_get_selected_plugin (pm);

	menu = gtk_menu_new ();

	item = gtk_image_menu_item_new_with_mnemonic (_("_About"));
	image = gtk_image_new_from_stock (GTK_STOCK_ABOUT,
					  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect (item, "activate",
			  G_CALLBACK (about_button_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_image_menu_item_new_with_mnemonic (_("C_onfigure"));
	image = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES,
					  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect (item, "activate",
			  G_CALLBACK (configure_button_cb), pm);
	gtk_widget_set_sensitive (item, conboy_plugin_info_is_configurable (info));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_check_menu_item_new_with_mnemonic (_("A_ctivate"));
	gtk_widget_set_sensitive (item, conboy_plugin_info_is_available (info));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
					conboy_plugin_info_is_active (info));
	g_signal_connect (item, "toggled",
			  G_CALLBACK (enable_plugin_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("Ac_tivate All"));
	g_signal_connect (item, "activate",
			  G_CALLBACK (enable_all_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("_Deactivate All"));
	g_signal_connect (item, "activate",
			  G_CALLBACK (disable_all_menu_cb), pm);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	
	gtk_widget_show_all (menu);
	
	return menu;
}
*/

static void
tree_popup_menu_detach (ConboyPluginManager *pm,
			GtkMenu            *menu)
{
	pm->priv->popup_menu = NULL;
}

static void
show_tree_popup_menu (GtkTreeView        *tree,
		      ConboyPluginManager *pm,
		      GdkEventButton     *event)
{
	if (pm->priv->popup_menu)
		gtk_widget_destroy (pm->priv->popup_menu);

	/*pm->priv->popup_menu = create_tree_popup_menu (pm);*/
	
	gtk_menu_attach_to_widget (GTK_MENU (pm->priv->popup_menu),
				   GTK_WIDGET (pm),
				   (GtkMenuDetachFunc) tree_popup_menu_detach);

	if (event != NULL)
	{
		gtk_menu_popup (GTK_MENU (pm->priv->popup_menu), NULL, NULL,
				NULL, NULL,
				event->button, event->time);
	}
	else
	{
		/*
		gtk_menu_popup (GTK_MENU (pm->priv->popup_menu), NULL, NULL,
				conboy_utils_menu_position_under_tree_view, tree,
				0, gtk_get_current_event_time ());
		*/
		gtk_menu_popup (GTK_MENU (pm->priv->popup_menu), NULL, NULL,
				NULL, tree, 0, gtk_get_current_event_time ());

		gtk_menu_shell_select_first (GTK_MENU_SHELL (pm->priv->popup_menu),
				FALSE);
	}
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
		
	/* The selection is fully updated by now */
	show_tree_popup_menu (GTK_TREE_VIEW (tree), pm, event);
	return TRUE;
}

static gboolean
popup_menu_cb (GtkTreeView        *tree,
	       ConboyPluginManager *pm)
{
	show_tree_popup_menu (tree, pm, NULL);
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

	return g_utf8_collate (conboy_plugin_info_get_name (info1),
			       conboy_plugin_info_get_name (info2));
}

static void
plugin_manager_construct_tree (ConboyPluginManager *pm)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkListStore *model;

	/*conboy_debug (DEBUG_PLUGINS);*/

	model = gtk_list_store_new (N_COLUMNS,
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

	/*cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
						 plugin_manager_view_icon_cell_cb,
						 pm, NULL);*/
	
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

/*
static void
plugin_toggled_cb (ConboyPluginsEngine *engine,
		   ConboyPluginInfo    *info,
		   ConboyPluginManager *pm)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean info_found = FALSE;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		ConboyPluginInfo *tinfo;
		gtk_tree_model_get (model, &iter, INFO_COLUMN, &tinfo, -1);
		info_found = info == tinfo;
	}

	if (!info_found)
	{
		gtk_tree_model_get_iter_first (model, &iter);

		do
		{
			ConboyPluginInfo *tinfo;
			gtk_tree_model_get (model, &iter, INFO_COLUMN, &tinfo, -1);
			info_found = info == tinfo;
		}
		while (!info_found && gtk_tree_model_iter_next (model, &iter));
	}

	if (!info_found)
	{
		g_warning ("ConboyPluginManager: plugin '%s' not found in the tree model",
			   conboy_plugin_info_get_name (info));
		return;
	}

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ACTIVE_COLUMN, conboy_plugin_info_is_active (info), -1);
}
*/

static void 
conboy_plugin_manager_init (ConboyPluginManager *pm)
{
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
	gtk_button_set_label (GTK_BUTTON(pm->priv->about_button), "About Plugin");
	gtk_container_add (GTK_CONTAINER (hbuttonbox), pm->priv->about_button);

	pm->priv->configure_button = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
	gtk_button_set_label (GTK_BUTTON(pm->priv->configure_button), "Configure Plugin");
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
	

	/*
	g_signal_connect_after (pm->priv->engine,
				"activate-plugin",
				G_CALLBACK (plugin_toggled_cb),
				pm);
	g_signal_connect_after (pm->priv->engine,
				"deactivate-plugin",
				G_CALLBACK (plugin_toggled_cb),
				pm);
	*/
	
	plugin_manager_populate_lists(pm);
	
	gtk_widget_show_all(GTK_WIDGET(pm));
}

static void
conboy_plugin_manager_finalize (GObject *object)
{
	ConboyPluginManager *pm = CONBOY_PLUGIN_MANAGER (object);
/*
	g_signal_handlers_disconnect_by_func (pm->priv->engine,
					      plugin_toggled_cb,
					      pm);
*/

	if (pm->priv->popup_menu)
		gtk_widget_destroy (pm->priv->popup_menu);

	G_OBJECT_CLASS (conboy_plugin_manager_parent_class)->finalize (object);

}

GtkWidget *conboy_plugin_manager_new (void)
{
	return g_object_new (CONBOY_TYPE_PLUGIN_MANAGER,0);
}

