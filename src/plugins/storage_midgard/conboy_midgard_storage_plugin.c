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
#include "conboy_midgard_storage_plugin.h"

G_DEFINE_TYPE(ConboyMidgardStoragePlugin, conboy_midgard_storage_plugin, CONBOY_TYPE_STORAGE_PLUGIN);

static ConboyNote*
_conboy_midgard_storage_plugin_note_load (ConboyStoragePlugin *self, const gchar *uuid)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(uuid != NULL, FALSE);
	
	g_return_val_if_fail(CONBOY_IS_MIDGARD_STORAGE_PLUGIN(self), FALSE);
	
	/* TODO: Read note from midgard */
	
	return NULL;
}

static gboolean
_conboy_midgard_storage_plugin_note_save (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_MIDGARD_STORAGE_PLUGIN(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Write note to midgard */

	return FALSE;
}

static gboolean 
_conboy_midgard_storage_plugin_note_delete (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_MIDGARD_STORAGE_PLUGIN(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO: Delete note from midgard */
	return FALSE;
}

static GSList*
_conboy_midgard_storage_plugin_note_list (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_MIDGARD_STORAGE_PLUGIN(self), FALSE);	

	/* TODO: Return notes */	

	return NULL;
}

static GSList*
_conboy_midgard_storage_plugin_note_list_ids (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_MIDGARD_STORAGE_PLUGIN(self), FALSE);	

	/* TODO: Return notes' IDS */

	return NULL;
}

static GtkWidget*
_conboy_midgard_storage_plugin_get_widget (ConboyPlugin *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_PLUGIN(self), NULL);
	
	/* First row */
	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	GtkWidget *label1 = gtk_label_new("Host:");
	GtkWidget *entry1 = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox1), label1);
	gtk_container_add(GTK_CONTAINER(hbox1), entry1);
	gtk_widget_show(label1);
	gtk_widget_show(entry1);
	gtk_widget_show(hbox1);
	
	/* First row */
	GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
	GtkWidget *label2 = gtk_label_new("Username:");
	GtkWidget *entry2 = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox2), label2);
	gtk_container_add(GTK_CONTAINER(hbox2), entry2);
	gtk_widget_show(label2);
	gtk_widget_show(entry2);
	gtk_widget_show(hbox2);
		
	/* First row */
	GtkWidget *hbox3 = gtk_hbox_new(FALSE, 0);
	GtkWidget *label3 = gtk_label_new("Password:");
	GtkWidget *entry3 = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox3), label3);
	gtk_container_add(GTK_CONTAINER(hbox3), entry3);
	gtk_widget_show(label3);
	gtk_widget_show(entry3);
	gtk_widget_show(hbox3);
	
	/* Result widget */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(vbox), hbox1);
	gtk_container_add(GTK_CONTAINER(vbox), hbox2);
	gtk_container_add(GTK_CONTAINER(vbox), hbox3);
	
	return vbox;
}

/* GOBJECT ROUTINES */

static GObject *
_conboy_midgard_storage_plugin_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (conboy_midgard_storage_plugin_parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	CONBOY_MIDGARD_STORAGE_PLUGIN(object)->user = NULL;
	CONBOY_MIDGARD_STORAGE_PLUGIN(object)->pass = NULL;

	return G_OBJECT(object);
}

static void
_conboy_midgard_storage_plugin_dispose(GObject *object)
{
	g_printerr("INFO: Dispose() called on Midgard plugin\n");
	ConboyMidgardStoragePlugin *self = CONBOY_MIDGARD_STORAGE_PLUGIN(object);
	
	g_free((gchar *)self->user);
	self->user = NULL;
	
	g_free((gchar *)self->pass);
	self->pass = NULL;

	G_OBJECT_CLASS(conboy_midgard_storage_plugin_parent_class)->dispose(object);
}

static void
conboy_midgard_storage_plugin_class_init (ConboyMidgardStoragePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ConboyPluginClass *plugin_class = CONBOY_PLUGIN_CLASS(klass);
	ConboyStoragePluginClass *storage_plugin_class = CONBOY_STORAGE_PLUGIN_CLASS(klass);

	object_class->constructor = _conboy_midgard_storage_plugin_constructor;
	object_class->dispose     = _conboy_midgard_storage_plugin_dispose;

	storage_plugin_class->load     = _conboy_midgard_storage_plugin_note_load;
	storage_plugin_class->save     = _conboy_midgard_storage_plugin_note_save;
	storage_plugin_class->delete   = _conboy_midgard_storage_plugin_note_delete;
	storage_plugin_class->list     = _conboy_midgard_storage_plugin_note_list;
	storage_plugin_class->list_ids = _conboy_midgard_storage_plugin_note_list_ids;
	
	plugin_class->get_widget 		= _conboy_midgard_storage_plugin_get_widget;
}

static void
conboy_midgard_storage_plugin_init (ConboyMidgardStoragePlugin *self)
{
	g_printerr("Hello from Midgard plugin\n");
	CONBOY_PLUGIN(self)->has_settings = TRUE;
}

ConboyMidgardStoragePlugin*
conboy_plugin_new() {
	return g_object_new(CONBOY_TYPE_MIDGARD_STORAGE_PLUGIN, NULL);
}
