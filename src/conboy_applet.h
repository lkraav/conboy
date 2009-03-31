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

#ifndef CONBOY_APPLET_H
#define CONBOY_APPLET_H

#include <glib-object.h>

/* For Task Navigator plugins */
#include <libhildondesktop/tasknavigator-item.h>

G_BEGIN_DECLS

/* Common struct types declarations */
typedef struct _HelloNavigatorPlugin HelloNavigatorPlugin;
typedef struct _HelloNavigatorPluginClass HelloNavigatorPluginClass;
typedef struct _HelloNavigatorPluginPrivate HelloNavigatorPluginPrivate;

/* Common macros */
#define HELLO_TYPE_NAVIGATOR_PLUGIN (hello_navigator_plugin_get_type())
     
#define HELLO_NAVIGATOR_PLUGIN (obj) (G_TYPE_CHECK_INSTANCE_CAST (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPlugin ))

#define HELLO_NAVIGATOR_PLUGIN_CLASS ( klass )    ( G_TYPE_CHECK_CLASS_CAST (( klass), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPluginClass ))

#define HELLO_IS_NAVIGATOR_PLUGIN (obj) (G_TYPE_CHECK_INSTANCE_TYPE (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN ))

#define HELLO_IS_NAVIGATOR_PLUGIN_CLASS ( klass ) ( G_TYPE_CHECK_CLASS_TYPE(( klass), HELLO_TYPE_NAVIGATOR_PLUGIN ))

#define HELLO_NAVIGATOR_PLUGIN_GET_CLASS (obj) (G_TYPE_INSTANCE_GET_CLASS (( obj), HELLO_TYPE_NAVIGATOR_PLUGIN , HelloNavigatorPluginClass ))

/* Instance struct */
struct _HelloNavigatorPlugin
{
     TaskNavigatorItem 				 tnitem;
     HelloNavigatorPluginPrivate 	*priv;
     GtkWidget              		*button;
     GtkWidget              		*menu;
};

/* Class struct */
struct _HelloNavigatorPluginClass
{
     TaskNavigatorItemClass parent_class ;
};

GType hello_navigator_plugin_get_type (void);
GtkWidget * hello_world_button_new (int padding );

G_END_DECLS

#endif /* CONBOY_APPLET_H */
