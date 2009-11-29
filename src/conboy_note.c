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

#include "localisation.h"
#include <glib/gprintf.h>
#include "conboy_note.h"

static GObjectClass *parent_class = NULL;

/* GOBJECT ROUTINES */

static GObject *
_conboy_note_constructor (GType type,
		guint n_construct_properties,
		GObjectConstructParam *construct_properties) 
{
	GObject *object = (GObject *)
		G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_properties,
				construct_properties);

	ConboyNote *note = CONBOY_NOTE(object);
	note->guid = NULL;
	note->title = NULL;
	note->content = NULL;
	
	note->note_version = 0.3;
	note->content_version = 0.1;
	
	return G_OBJECT(object);
}

static void
_conboy_note_class_dispose(GObject *object)
{
	ConboyNote *self = CONBOY_NOTE(object);
	
	g_printerr("Dispose Note: %s\n", self->title);
	
	g_free((gchar*)self->guid);
	self->guid = NULL;

	g_free((gchar*)self->title);
	self->title = NULL;

	g_free((gchar*)self->content);
	self->content = NULL;
	
	conboy_note_clear_tags(self);

	parent_class->dispose(object);
}

enum {
	Prop_0,
	PROP_GUID,
	PROP_TITLE,
	PROP_CONTENT,
	PROP_CREATE_DATE,
	PROP_CHANGE_DATE,
	PROP_METADATA_CHANGE_DATE,
	PROP_CURSOR_POSITION,
	PROP_PINNED,
	PROP_OPEN_ON_STARTUP,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_X,
	PROP_Y,
	PROP_NOTE_VERSION,
	PROP_CONTENT_VERSION
};

static void
conboy_note_get_property (GObject *object, guint id, GValue *value, GParamSpec *spec)
{
	ConboyNote *note = CONBOY_NOTE(object);
	
	switch (id)
	{
		case PROP_GUID:
			g_value_set_string(value, note->guid);
			break;
		case PROP_TITLE:
			g_value_set_string(value, note->title);
			break;
		case PROP_CONTENT:
			g_value_set_string(value, note->content);
			break;
		case PROP_CREATE_DATE:
			g_value_set_uint(value, note->create_date);
			break;
		case PROP_CHANGE_DATE:
			g_value_set_uint(value, note->last_change_date);
			break;
		case PROP_METADATA_CHANGE_DATE:
			g_value_set_uint(value, note->last_metadata_change_date);
			break;
		case PROP_CURSOR_POSITION:
			g_value_set_int(value, note->cursor_position);
			break;
		case PROP_PINNED:
			g_value_set_boolean(value, note->pinned);
			break;
		case PROP_OPEN_ON_STARTUP:
			g_value_set_boolean(value, note->open_on_startup);
			break;
		case PROP_WIDTH:
			g_value_set_int(value, note->width);
			break;
		case PROP_HEIGHT:
			g_value_set_int(value, note->height);
			break;
		case PROP_NOTE_VERSION:
			g_value_set_double(value, note->note_version);
			break;
		case PROP_CONTENT_VERSION:
			g_value_set_double(value, note->content_version);
			break;
		case PROP_X:
			g_value_set_int(value, note->x);
			break;
		case PROP_Y:
			g_value_set_int(value, note->y);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
			break;
	}
}

static void
conboy_note_set_property (GObject *object, guint id, const GValue *value, GParamSpec *spec)
{
	ConboyNote *note = CONBOY_NOTE(object);
	
	switch (id)
	{
		case PROP_GUID:
			g_free (note->guid);
			note->guid = g_value_dup_string(value);
			break;
		case PROP_TITLE:
			g_free (note->title);
			note->title = g_value_dup_string(value);
			break;
		case PROP_CONTENT:
			g_free (note->content);
			note->content = g_value_dup_string(value);
			break;
		case PROP_CREATE_DATE:
			note->create_date = g_value_get_uint(value);
			break;
		case PROP_CHANGE_DATE:
			note->last_change_date = g_value_get_uint(value);
			break;
		case PROP_METADATA_CHANGE_DATE:
			note->last_metadata_change_date = g_value_get_uint(value);
			break;
		case PROP_CURSOR_POSITION:
			note->cursor_position = g_value_get_int(value);
			break;
		case PROP_PINNED:
			note->pinned = g_value_get_boolean(value);
			break;
		case PROP_OPEN_ON_STARTUP:
			note->open_on_startup = g_value_get_boolean(value);
			break;
		case PROP_WIDTH:
			note->width = g_value_get_int(value);
			break;
		case PROP_HEIGHT:
			note->height = g_value_get_int(value);
			break;
		case PROP_X:
			note->x = g_value_get_int(value);
			break;
		case PROP_Y:
			note->y = g_value_get_int(value);
			break;
		case PROP_NOTE_VERSION:
			note->note_version = g_value_get_double(value);
			break;
		case PROP_CONTENT_VERSION:
			note->content_version = g_value_get_double(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
			break;
	}
}

static void
_conboy_note_class_init (ConboyNoteClass *klass, gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->constructor = _conboy_note_constructor;
	object_class->dispose = _conboy_note_class_dispose;
	object_class->get_property = conboy_note_get_property;
	object_class->set_property = conboy_note_set_property;
	
	/*
	 * Create and install properties
	 */

	GParamSpec *spec;
	
	spec = g_param_spec_string("guid", "GUID",
			"The global unique identifier", NULL, G_PARAM_READWRITE);
	g_object_class_install_property(object_class, PROP_GUID, spec);
	
	spec = g_param_spec_string("title", "Title",
			"The title of the note", NULL, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_TITLE, spec);
	
	spec = g_param_spec_string("content", "Content",
			"XML representation of the notes content", NULL, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_CONTENT, spec);
	
	spec = g_param_spec_uint("create-date", "Create date",
			"The date of note creation", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_CREATE_DATE, spec);
	
	spec = g_param_spec_uint("change-date", "Change date",
			"The date of the last change", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_CHANGE_DATE, spec);
	
	spec = g_param_spec_uint("metadata-change-date", "Metadata change date",
			"The date of the last metadata change", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_METADATA_CHANGE_DATE, spec);
	
	spec = g_param_spec_int("cursor-position", "Cursor position",
			"The current cursor position", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_CURSOR_POSITION, spec);
	
	spec = g_param_spec_boolean("pinned", "Pinned",
			"Whether or not the note is pinned (sticky)", FALSE, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_PINNED, spec);
	
	spec = g_param_spec_boolean("open-on-startup", "Open on startup",
			"Whether or not the note should be opened on startup", FALSE, G_PARAM_READWRITE);
	g_object_class_install_property(object_class, PROP_OPEN_ON_STARTUP, spec);
	
	spec = g_param_spec_int("width", "Window width",
			"The width of the note window", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_WIDTH, spec);
	
	spec = g_param_spec_int("height", "Window height",
			"The height of the note window", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_HEIGHT, spec);
	
	spec = g_param_spec_int("x", "Window x position",
			"The x position of the note window", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_X, spec);
	
	spec = g_param_spec_int("y", "Window y position",
			"The y position of the note window", 0, G_MAXINT, 0, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_Y, spec);
	
	spec = g_param_spec_double("note-version", "Note version",
			"The version of the note", 0, G_MAXDOUBLE, 0.3, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_NOTE_VERSION, spec);
	
	spec = g_param_spec_double("content-version", "Content version",
			"The version notes content", 0, G_MAXDOUBLE, 0.1, G_PARAM_READWRITE);	
	g_object_class_install_property(object_class, PROP_CONTENT_VERSION, spec);
	
}

GType
conboy_note_get_type (void)
{
	static GType type = 0;
	if (type == 0) {

		static const GTypeInfo info = {
			sizeof (ConboyNoteClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _conboy_note_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
 			sizeof (ConboyNote),
			0,              /* n_preallocs */
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
				"ConboyNote", &info, 0);
	}

	return type;
}

static gchar*
note_get_uuid()
{
	gchar *content;
	gchar *result;
	g_file_get_contents("/proc/sys/kernel/random/uuid", &content, NULL, NULL);
	g_strchomp(content);
	result = g_strconcat(content, "\0", NULL);
	g_free(content);
	return result;
}

ConboyNote*
conboy_note_new()
{
	return g_object_new(CONBOY_TYPE_NOTE, "guid", note_get_uuid(), NULL);
}

ConboyNote*
conboy_note_new_with_guid(const gchar* guid)
{
	return g_object_new(CONBOY_TYPE_NOTE, "guid", guid, NULL);
}

ConboyNote*
conboy_note_new_with_title (const gchar* title)
{
	ConboyNote *note = conboy_note_new();

	gchar xml[200];
	g_sprintf(xml, "<note-content version=\"0.1\">%s\n\n%s</note-content>",
			title,
			_("Describe your new note here."));

	g_object_set(note, "title", title, "content", xml, NULL);
	
	return note;
}

void
conboy_note_add_tag(ConboyNote* note, const gchar* tag)
{
	g_return_if_fail(note != NULL);
	g_return_if_fail(tag != NULL);
	
	g_return_if_fail(CONBOY_IS_NOTE(note));
	
	note->tags = g_list_append(note->tags, g_strdup(tag));
}

void
conboy_note_clear_tags(ConboyNote* note)
{
	g_return_if_fail(note != NULL);
	g_return_if_fail(CONBOY_IS_NOTE(note));
	g_return_if_fail(note->tags != NULL);
	
	GList *tags = note->tags;
	while (tags != NULL) {
		g_free(tags->data);
		tags = tags->next;
	}
	g_list_free(note->tags);
	note->tags = NULL;
}


GList*
conboy_note_get_tags(ConboyNote* note)
{
	g_return_val_if_fail(note != NULL, NULL);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), NULL);
	
	return note->tags;
}


