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

#include "../../conboy_note.h"
#include "../../conboy_storage_plugin.h"
#include "conboy_storage_plugin_xml.h"

G_DEFINE_TYPE(ConboyStoragePluginXml, conboy_storage_plugin_xml, CONBOY_TYPE_STORAGE_PLUGIN);


ConboyStoragePluginXml*
conboy_storage_plugin_xml_new ()
{
	g_printerr("Hello from xml plugin \n");
	return g_object_new(CONBOY_TYPE_STORAGE_PLUGIN_XML, NULL);
}


static ConboyNote*
load (ConboyStoragePlugin *self, const gchar *guid)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(guid != NULL, FALSE);
	
	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_XML(self), FALSE);
	
	/* TODO: Read note from xml */
	g_printerr("Called 'load' on ConboyStoragePluginXml\n");
	
	return NULL;
}

static gboolean
save (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_XML(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO */
	/* Write note to xml file */
	g_printerr("Called 'save' on ConboyStoragePluginXml\n");

	return FALSE;
}

static gboolean 
delete (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_XML(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO */
	/* Delete xml file */
	g_printerr("Called 'delete' on ConboyStoragePluginXml\n");

	return FALSE;
}

static ConboyNote**
list (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_XML(self), FALSE);	

	/* TODO */
	/* Return notes */	
	g_printerr("Called 'list' on ConboyStoragePluginXml\n");

	return NULL;
}

static gchar**
list_ids (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_STORAGE_PLUGIN_XML(self), FALSE);	

	/* TODO */
	/* Return notes' IDS */
	g_printerr("Called 'list_ids' on ConboyStoragePluginXml\n");

	return NULL;
}

/*
 * GOBJECT stuff
 */

static void
dispose(GObject *object)
{
	ConboyStoragePluginXml *self = CONBOY_STORAGE_PLUGIN_XML(object);
	
	/*g_free((gchar *)self->path);
	self->path = NULL;*/

	G_OBJECT_CLASS(conboy_storage_plugin_xml_parent_class)->dispose(object);
}

static void
conboy_storage_plugin_xml_class_init (ConboyStoragePluginXmlClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ConboyStoragePluginClass *storage_class = CONBOY_STORAGE_PLUGIN_CLASS(klass);
	
	/*parent_class = g_type_class_peek_parent (klass);*/
	
	g_printerr("XML: class init called\n");
	
	object_class->dispose =	dispose;

	storage_class->load = load;
	storage_class->save = save;
	storage_class->delete = delete;
	storage_class->list = list;
	storage_class->list_ids = list_ids;
	
}

static void
conboy_storage_plugin_xml_init (ConboyStoragePluginXml *klass)
{
	g_printerr("XML: init called\n");
}


