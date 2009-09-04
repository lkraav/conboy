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

#ifndef CONBOY_XML_STORAGE_PLUGIN_H
#define CONBOY_XML_STORAGE_PLUGIN_H

#include <glib-object.h>

/* convention macros */
#define CONBOY_TYPE_XML_STORAGE_PLUGIN				(conboy_xml_storage_plugin_get_type())
#define CONBOY_XML_STORAGE_PLUGIN(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object),CONBOY_TYPE_XML_STORAGE_PLUGIN, ConboyXmlStoragePlugin))
#define CONBOY_XML_STORAGE_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CONBOY_TYPE_XML_STORAGE_PLUGIN, ConboyXmlStoragePluginClass))
#define CONBOY_IS_XML_STORAGE_PLUGIN(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), CONBOY_TYPE_XML_STORAGE_PLUGIN))
#define CONBOY_IS_XML_STORAGE_PLUGIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CONBOY_TYPE_XML_STORAGE_PLUGIN))
#define CONBOY_XML_STORAGE_PLUGIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CONBOY_TYPE_XML_STORAGE_PLUGIN, ConboyXmlStoragePluginClass))

typedef struct _ConboyXmlStoragePlugin 		ConboyXmlStoragePlugin;
typedef struct _ConboyXmlStoragePluginClass ConboyXmlStoragePluginClass;

struct _ConboyXmlStoragePlugin {
	ConboyStoragePlugin parent;
	/*<private>*/
	gchar *path;
};

struct _ConboyXmlStoragePluginClass {
	ConboyStoragePluginClass parent;
};

GType					conboy_xml_storage_plugin_get_type	(void);
ConboyXmlStoragePlugin* conboy_plugin_new					(void);

#endif /* CONBOY_XML_STORAGE_PLUGIN_H */
