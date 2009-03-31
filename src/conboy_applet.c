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
#include <glib-object.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <libhildondesktop/libhildondesktop.h>
#include <hildon/hildon-window.h>

#include "metadata.h"
#include "conboy_applet.h"


HD_DEFINE_PLUGIN(HelloNavigatorPlugin, hello_navigator_plugin, TASKNAVIGATOR_TYPE_ITEM)

static GtkWidget*
create_button(int padding) {
	
	GtkIconTheme *icon_theme;
	GdkPixbuf *icon;
	GtkWidget *icon_image, *button;
	
	icon_theme = gtk_icon_theme_get_default ();
	icon = gtk_icon_theme_load_icon (icon_theme,
					   "conboy",
					   40,
					   0,
					   NULL);
	if (icon == NULL)
	  icon = gtk_icon_theme_load_icon (icon_theme,
					     "qgn_list_gene_default_app",
					     40,
					     0,
					     NULL);
	    
	icon_image = gtk_image_new_from_pixbuf (icon);
	gtk_misc_set_padding (GTK_MISC (icon_image), padding, padding);
	g_object_unref (G_OBJECT (icon));
	button = gtk_button_new ();
	gtk_container_add (GTK_CONTAINER (button), icon_image);
	
	gtk_widget_show_all (button);
	
	return button;
}

static void
hello_navigator_plugin_class_init(HelloNavigatorPluginClass *class)
{
	g_printerr("OOOOO: Start plugin_class_init.\n");
}

static void
show_dialog (GtkWidget *item , HelloNavigatorPlugin *thw)
{
    GtkWidget *dialog;
    g_printerr("OOOOO: Start show_dialog.\n");
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE,
                                    "Hello world !");
    gtk_window_set_title(GTK_WINDOW(dialog), "TN Plugin Example");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void
menu_position (GtkMenu *menu,
               gint *x,
               gint *y,
               gboolean *push_in,
               HelloNavigatorPlugin *thw)
{
    g_return_if_fail(thw->button);
    *push_in = TRUE;
    *x = thw->button->allocation.x + thw->button->allocation.width ;
    *y = thw->button->allocation.y;
}

static void
popup_menu ( GtkWidget *button , HelloNavigatorPlugin *plugin )
{	
	GSList *notes;
	Note *note;
	GtkWidget *menu_item;
	
	   
    if (!plugin->menu) {
        return ;
    }
    
    /*
    if (!is_note_list_changed()) {
    	g_printerr("NOTE LIST NOT CHANGED\n");
    	return;
    }
    */
    
    /* TODO: We need to give back the memory of the gtk menu */
    /* TODO: Always recreating this menu is not very efficient... */
    
    plugin->menu = gtk_menu_new();
    /* Name the menu to get the appropriate theming */
    gtk_widget_set_name(plugin->menu, "menu_from_navigator");
    
    /* TODO: NULL for now. Check if we need AppData here too */
    notes = create_note_list(NULL);
    
    while (notes != NULL) {

    	note = notes->data;
    
	    menu_item = gtk_menu_item_new_with_label(note->title);
	    
	    g_signal_connect(G_OBJECT(menu_item), "activate",
	                     G_CALLBACK (show_dialog), NULL);
	    gtk_menu_append(plugin->menu, menu_item);
	    notes = g_slist_next(notes);
	    
    }
    
    gtk_widget_show_all(plugin->menu);
    
    g_slist_free(notes);
    
    
    gtk_menu_popup(GTK_MENU(plugin->menu),
                   NULL,
                   NULL,
                   (GtkMenuPositionFunc)menu_position,
                   plugin,
                   0,
                   gtk_get_current_event_time());
}

static GtkWidget *
create_menu (void)
{
    GtkWidget *menu;
   
    menu = gtk_menu_new();
     
    /* Name the menu to get the appropriate theming */
    gtk_widget_set_name(menu, "menu_from_navigator");
    gtk_widget_show_all(menu);
    return menu;
}

static void
hello_navigator_plugin_init (HelloNavigatorPlugin *navigator_plugin)
{
  GtkWidget *button;
  
  button = create_button(10);
  
  navigator_plugin->button = button ;
  navigator_plugin->menu = create_menu();
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(popup_menu), navigator_plugin);
  gtk_widget_set_size_request(button, 80, 80);
  gtk_widget_set_name (button, "hildon-navigator-button-one");
  gtk_widget_show_all(button);
  gtk_container_add(GTK_CONTAINER(navigator_plugin), button);
  gtk_widget_show_all(navigator_plugin);
}


