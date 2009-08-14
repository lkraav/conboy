/* 
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef CONBOY_PLUGIN_H
#define CONBOY_PLUGIN_H

#include <glib-object.h>
#include <gmodule.h>

/* convention macros */
#define CONBOY_TYPE_MODULE (conboy_plugin_get_type())
#define CONBOY_PLUGIN(object)  (G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_MODULE, ConboyPlugin))
#define CONBOY_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_MODULE, ConboyPluginClass))
#define CONBOY_IS_MODULE(object)   (G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_MODULE))
#define CONBOY_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_MODULE))
#define CONBOY_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_MODULE, ConboyPluginClass))

typedef struct _ConboyPlugin ConboyPlugin;
typedef struct _ConboyPluginClass ConboyPluginClass;

struct _ConboyPlugin{
	GObject parent;
	
	/* <private> */
	gchar *path;
	gchar *name;
	GModule *gmodule;
	gboolean open_on_startup;	
};

struct _ConboyPluginClass{
	GObjectClass parent;

	/* virtual methods */
	gboolean	(*initialize)	(ConboyPlugin *module);
	
	/* signals */
	
};

GType		conboy_plugin_get_type		(void);

ConboyPlugin	*conboy_plugin_new(const gchar *name);
void			 conboy_plugin_initialize_modules(ConboyPlugin **modules, GError **error);
gboolean		 conboy_plugin_initialize(ConboyPlugin *module);

/* Conny: I'm not sure whether this is the right place */
/*
gboolean	 	 conboy_plugin_is_configurable (ConboyPlugin *plugin);
GtkWidget		*conboy_plugin_create_configure_dialog (ConboyPlugin *plugin);
*/

#endif /* CONBOY_PLUGIN_H */
