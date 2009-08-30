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

#ifndef CONBOY_MIDGARD_STORAGE_PLUGIN_H
#define CONBOY_MIDGARD_STORAGE_PLUGIN_H

#include <glib-object.h>

/* convention macros */
#define CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN				(conboy_midgard_storage_plugin_get_type())
#define CONBOY_MIDGARD_STORAGE_PLUGIN(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN, ConboyMidgardStoragePlugin))
#define CONBOY_MIDGARD_STORAGE_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN, ConboyMidgardStoragePluginClass))
#define CONBOY_IS_MIDGARD_STORAGE_PLUGIN(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN))
#define CONBOY_IS_MIDGARD_STORAGE_PLUGIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN))
#define CONBOY_MIDGARD_STORAGE_PLUGIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN, ConboyMidgardStoragePluginClass))



typedef struct _ConboyMidgardStoragePlugin 		ConboyMidgardStoragePlugin;
typedef struct _ConboyMidgardStoragePluginClass ConboyMidgardStoragePluginClass;

struct _ConboyMidgardStoragePlugin {
	ConboyStoragePlugin parent;
	
	gchar *user;
	gchar *pass;
};

struct _ConboyMidgardStoragePluginClass {
	ConboyStoragePluginClass parent;
};


GType		conboy_midgard_storage_plugin_get_type			(void);

ConboyMidgardStoragePlugin* conboy_plugin_new(void);

#endif /* CONBOY_MIDGARD_STORAGE_PLUGIN_H */
