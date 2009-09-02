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
#include <gtk/gtk.h>


/* convention macros */
#define CONBOY_TYPE_PLUGIN				(conboy_plugin_get_type())
#define CONBOY_PLUGIN(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_PLUGIN, ConboyPlugin))
#define CONBOY_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_PLUGIN, ConboyPluginClass))
#define CONBOY_IS_PLUGIN(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_PLUGIN))
#define CONBOY_IS_PLUGIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_PLUGIN))
#define CONBOY_PLUGIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_PLUGIN, ConboyPluginClass))

typedef struct _ConboyPlugin		ConboyPlugin;
typedef struct _ConboyPluginClass	ConboyPluginClass;

struct _ConboyPlugin {
	GObject parent;
	
	/* <private> */
	GModule *gmodule;
	gboolean has_settings;
};

struct _ConboyPluginClass {
	GObjectClass parent;

	/* virtual methods */
	GtkWidget* 	(*get_widget)	(ConboyPlugin *self);
	
	/* signals */
	
};

GType			conboy_plugin_get_type		(void);
ConboyPlugin*	conboy_plugin_new_from_path (gchar *filename);

gboolean	 	conboy_plugin_has_settings (ConboyPlugin *self);
GtkWidget*		conboy_plugin_get_settings_widget (ConboyPlugin *self);


#endif /* CONBOY_PLUGIN_H */
