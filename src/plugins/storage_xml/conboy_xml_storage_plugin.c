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

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include <glib/gprintf.h>

#include "../../conboy_note.h"
#include "../../conboy_storage_plugin.h"
#include "conboy_xml_storage_plugin.h"

#define NOTE_TAG "note"
#define TITLE_TAG "title"
#define TEXT_TAG "text"
#define NOTE_CONTENT_TAG "note-content"
#define LAST_CHANGE_DATE_TAG "last-change-date"
#define LAST_METADATA_CHANGE_DATE_TAG "last-metadata-change-date"
#define CREATE_DATE_TAG "create-date"
#define CURSOR_POSITION_TAG "cursor-position"
#define WIDTH_TAG "width"
#define HEIGHT_TAG "height"
#define X_TAG "x"
#define Y_TAG "y"
#define OPEN_ON_STARTUP_TAG "open-on-startup"
#define TAGS_TAG "tags"
#define TAG_TAG "tag"

G_DEFINE_TYPE(ConboyXmlStoragePlugin, conboy_xml_storage_plugin, CONBOY_TYPE_STORAGE_PLUGIN);

typedef enum {
	NOTE,
	TITLE,
	TEXT,
	NOTE_CONTENT,
	LAST_CHANGE_DATE,
	LAST_METADATA_CHANGE_DATE,
	CREATE_DATE,
	CURSOR_POSITION,
	WIDTH,
	HEIGHT,
	X,
	Y,
	OPEN_ON_STARTUP,
	TAGS,
	TAG,
	UNKNOWN
} XmlTag;

static XmlTag
string_to_enum(const gchar *string)
{
	if (strcmp(string, NOTE_TAG) == 0) return NOTE;
	if (strcmp(string, TITLE_TAG) == 0) return TITLE;
	if (strcmp(string, TEXT_TAG) == 0) return TEXT;
	if (strcmp(string, NOTE_CONTENT_TAG) == 0) return NOTE_CONTENT;
	if (strcmp(string, LAST_CHANGE_DATE_TAG) == 0) return LAST_CHANGE_DATE;
	if (strcmp(string, LAST_METADATA_CHANGE_DATE_TAG) == 0) return LAST_METADATA_CHANGE_DATE;
	if (strcmp(string, CREATE_DATE_TAG) == 0) return CREATE_DATE;
	if (strcmp(string, CURSOR_POSITION_TAG) == 0) return CURSOR_POSITION;
	if (strcmp(string, WIDTH_TAG) == 0) return WIDTH;
	if (strcmp(string, HEIGHT_TAG) == 0) return HEIGHT;
	if (strcmp(string, X_TAG) == 0) return X;
	if (strcmp(string, Y_TAG) == 0) return Y;
	if (strcmp(string, OPEN_ON_STARTUP_TAG) == 0) return OPEN_ON_STARTUP;
	if (strcmp(string, TAGS_TAG) == 0) return TAGS;
	if (strcmp(string, TAG_TAG) == 0) return TAG;
	
	return UNKNOWN;
}

static void
handle_start_element(xmlTextReader *reader, ConboyNote *note)
{
	/* TODO: Whats the official way to convert xmlChar to gchar? */
	const gchar *name = xmlTextReaderConstName(reader);
	gchar *value = xmlTextReaderReadString(reader);
	gchar *attr_value = NULL;
	XmlTag tag = string_to_enum(name);
	
	switch (tag) {
	
	case NOTE:
		attr_value = xmlTextReaderGetAttribute(reader, BAD_CAST "version");
		if (attr_value != NULL) {
			g_object_set(note, "note-version", atof(attr_value), NULL);
			g_free(attr_value);
			attr_value = NULL;
		} else {
			g_printerr("ERROR: Couldn't parse note version.\n");
		}
	
	case TITLE:
		g_object_set(note, "title", value, NULL);
		break;
	
	case NOTE_CONTENT:
		attr_value = xmlTextReaderGetAttribute(reader, BAD_CAST "version");
		if (attr_value != NULL) {
			g_object_set(note, "content-version", atof(attr_value), NULL);
			g_free(attr_value);
			attr_value = NULL;
		} else {
			g_printerr("ERROR: Couldn't parse content version.\n");
		}
		
		g_object_set(note, "content", (gchar*)xmlTextReaderReadOuterXml(reader), NULL);
		break;
		
	case LAST_CHANGE_DATE:
		g_object_set(note, "change-date", get_iso8601_time_in_seconds(value), NULL);
		break;
		
	case LAST_METADATA_CHANGE_DATE:
		g_object_set(note, "metadata-change-date", get_iso8601_time_in_seconds(value), NULL);
		break;
		
	case CREATE_DATE:
		g_object_set(note, "create-date", get_iso8601_time_in_seconds(value), NULL);
		break;
		
	case CURSOR_POSITION:
		g_object_set(note, "cursor-position", atoi(value), NULL);
		break;
		
	case WIDTH:
		g_object_set(note, "width", atoi(value), NULL);
		break;
		
	case HEIGHT:
		g_object_set(note, "height", atoi(value), NULL);
		break;
		
	case X:
		g_object_set(note, "x", atoi(value), NULL);
		break;
		
	case Y:
		g_object_set(note, "y", atoi(value), NULL);
		break;
		
	case OPEN_ON_STARTUP:
		g_object_set(note, "open-on-startup", atoi(value), NULL);
		break;
		
	case TAG:
		conboy_note_add_tag(note, value);
		break;
		
	default:
		break;
	}
	
	g_free(value);
	g_free(attr_value);
}

static
void process_note(xmlTextReader *reader, ConboyNote *note)
{
	if (xmlTextReaderNodeType(reader) == XML_ELEMENT_NODE) {
		handle_start_element(reader, note);
	}
}

static void
write_header(xmlTextWriter *writer, ConboyNote *note)
{
	int rc;
	gchar version[20];
	gdouble note_version;
	gchar *title;
	
	g_object_get(note, "title", &title, "note-version", &note_version, NULL);
	
	/* Enable indentation */
	rc = xmlTextWriterSetIndent(writer, TRUE);
	
	/* Start document */
	rc = xmlTextWriterStartDocument(writer, "1.0", "utf-8", NULL);
	  
	/* Start note element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "note");
	
	g_ascii_formatd(version, 20, "%.1f", note_version);
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST &version);
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
	rc = xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");
	  
	/* Title element */
	rc = xmlTextWriterWriteElement(writer, BAD_CAST "title", BAD_CAST title);
	g_free(title);
	
	/* Start text element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "text");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml", BAD_CAST "space", NULL, BAD_CAST "preserve");
}

static void
write_footer(xmlTextWriter *writer, ConboyNote *note) 
{
	int rc;
	GList *tags;
	guint change_date, metadata_change_date, create_date;
	gint  cursor_position, width, height, x, y;
	gboolean open_on_startup;
	
	g_object_get(note,
			"change-date", &change_date,
			"metadata-change-date", &metadata_change_date,
			"create-date", &create_date,
			"cursor-position", &cursor_position,
			"open-on-startup", &open_on_startup,
			"width", &width,
			"height", &height,
			"x", &x,
			"y", &y,
			NULL);
	
	/* Enable indentation */
	rc = xmlTextWriterSetIndent(writer, TRUE);
		  
	/* Meta data tags */
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "last-change-date", "%s", get_time_in_seconds_as_iso8601(change_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "last-metadata-change-date", "%s", get_time_in_seconds_as_iso8601(metadata_change_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "create-date", "%s", get_time_in_seconds_as_iso8601(create_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "cursor-position", "%i", cursor_position);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "width", "%i", width);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "height", "%i", height);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "x", "%i", x);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "y", "%i", y);
	
	/* Write tags */
	tags = conboy_note_get_tags(note);
	if (tags != NULL) {
		rc = xmlTextWriterStartElement(writer, BAD_CAST "tags");
		while (tags != NULL) {
			gchar *tag = tags->data;
			rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tag", "%s", tag);
			tags = tags->next;
		}
		rc = xmlTextWriterEndElement(writer);
	}
	
	if (open_on_startup == TRUE) {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "True");
	} else {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "False");
	}

	/* End the document */
	rc = xmlTextWriterEndDocument(writer);
}



/*
 * Public methods
 */

ConboyXmlStoragePlugin*
conboy_plugin_new ()
{
	g_printerr("Hello from xml plugin \n");
	return g_object_new(CONBOY_TYPE_XML_STORAGE_PLUGIN, NULL);
}


static ConboyNote*
load (ConboyStoragePlugin *self, const gchar *guid)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(guid != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_XML_STORAGE_PLUGIN(self), FALSE);
	
	int ret;
	ConboyNote *note;
	gchar *filename;
	xmlTextReader *reader;
	
	
	filename = g_strconcat(CONBOY_XML_STORAGE_PLUGIN(self)->path, guid, ".note", NULL);
	
	reader = xmlReaderForFile(filename, NULL, 0);
	
	note = conboy_note_new_with_guid(guid);
	
	ret = xmlTextReaderRead(reader);
	while (ret == 1) {
		process_note(reader, note);
		ret = xmlTextReaderRead(reader);
	}
		
	if (ret != 0) {
		g_printerr("ERROR: Failed to parse file: %s\n", filename);
	}
	
	xmlFreeTextReader(reader);
	g_free(filename);
	
	if (ret != 0) {
		return NULL;
	} else {
		return note;
	}
	
}

static gboolean
save (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_XML_STORAGE_PLUGIN(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO */
	/* Write note to xml file */
	g_printerr("Called 'save' on ConboyXmlStoragePlugin\n");
	
	xmlTextWriter *writer;
	gchar *filename;
	
	g_assert(note->guid != NULL);
	
	filename = g_strconcat(CONBOY_XML_STORAGE_PLUGIN(self)->path, note->guid, ".note", NULL);
	
	writer = xmlNewTextWriterFilename(filename, 0);
	if (writer == NULL) {
		g_printerr("ERROR: XmlWriter is NULL \n");
		return FALSE;
	}
	
	write_header(writer, note);
	gchar *content;
	g_object_get(note, "content", &content, NULL);
	xmlTextWriterWriteRaw(writer, note->content);
	xmlTextWriterEndElement(writer); /* close <text> */
	g_free(content);
	write_footer(writer, note);
	
	xmlFreeTextWriter(writer);
	g_free(filename);
	
	return TRUE;
	

	
}

static gboolean 
delete (ConboyStoragePlugin *self, ConboyNote *note)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(note != NULL, FALSE);

	g_return_val_if_fail(CONBOY_IS_XML_STORAGE_PLUGIN(self), FALSE);
	g_return_val_if_fail(CONBOY_IS_NOTE(note), FALSE);

	/* TODO */
	/* Delete xml file */
	g_printerr("Called 'delete' on ConboyXmlStoragePlugin\n");

	return FALSE;
}

static GSList*
list_ids (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_XML_STORAGE_PLUGIN(self), FALSE);
	
	const gchar *filename;
	GDir *dir = g_dir_open(CONBOY_XML_STORAGE_PLUGIN(self)->path, 0, NULL);
	GSList *result = NULL;
	
	while ((filename = g_dir_read_name(dir)) != NULL) {
		if (g_str_has_suffix(filename, ".note")) {
			gchar *guid = g_strndup(filename, g_utf8_strlen(filename, -1) - 5);
			result = g_slist_prepend(result, guid);
		}
	}
	
	g_dir_close(dir);
	
	return result;
}

static GSList*
list (ConboyStoragePlugin *self)
{
	g_return_val_if_fail(self != NULL, FALSE);
	g_return_val_if_fail(CONBOY_IS_XML_STORAGE_PLUGIN(self), FALSE);

	g_printerr("Called 'list' on ConboyXmlStoragePlugin\n");
	GSList *result = NULL;
	GSList *ids = list_ids(self);
	GSList *iter = ids;
	while (iter != NULL) {
		ConboyNote *note = load(self, iter->data);
		if (note != NULL) {
			result = g_slist_prepend(result, note);
		}
		g_free(iter->data);
		iter = iter->next;
	}
	g_slist_free(ids);
	return result;
}

/*
 * GOBJECT stuff
 */

static void
dispose(GObject *object)
{
	g_printerr("INFO: Dispose() called on xml storage plugin\n");
	ConboyXmlStoragePlugin *self = CONBOY_XML_STORAGE_PLUGIN(object);
	
	g_free(self->path);
	self->path = NULL;

	G_OBJECT_CLASS(conboy_xml_storage_plugin_parent_class)->dispose(object);
}

static void
conboy_xml_storage_plugin_class_init (ConboyXmlStoragePluginClass *klass)
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
conboy_xml_storage_plugin_init (ConboyXmlStoragePlugin *self)
{
	g_printerr("XML: init called\n");
	CONBOY_PLUGIN(self)->has_settings = FALSE;
	self->path = g_strconcat(g_get_home_dir(), "/.conboy/", NULL);
}


