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

#ifndef CONBOY_PLUGIN_MANAGER_H_
#define CONBOY_PLUGIN_MANAGER_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define CONBOY_TYPE_PLUGIN_MANAGER              (conboy_plugin_manager_get_type())
#define CONBOY_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManager))
#define CONBOY_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerClass))
#define CONBOY_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CONBOY_TYPE_PLUGIN_MANAGER))
#define CONBOY_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN_MANAGER))
#define CONBOY_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CONBOY_TYPE_PLUGIN_MANAGER, ConboyPluginManagerClass))

/* Private structure type */
typedef struct _ConboyPluginManagerPrivate ConboyPluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _ConboyPluginManager ConboyPluginManager;

struct _ConboyPluginManager
{
	GtkVBox vbox;

	/*< private > */
	ConboyPluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _ConboyPluginManagerClass ConboyPluginManagerClass;

struct _ConboyPluginManagerClass
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 conboy_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget*   conboy_plugin_manager_new          (void);

gint         conboy_plugin_manager_get_count    (ConboyPluginManager *self);

G_END_DECLS


#endif /*CONBOY_PLUGIN_MANAGER_H_*/
