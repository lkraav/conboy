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

#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include <glib/gprintf.h>

#include "storage.h"
#include "app_data.h"
#include "note.h"
#include "metadata.h"


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

/* Privates */
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
}XmlTag;

XmlTag string_to_enum(const gchar *string);
const gchar* enum_to_string(XmlTag tag);

void handle_start_element(xmlTextReader *reader, Note *note);

/**
 * Initializes the storage backend. E.g. conntect to a database,
 * create file paths etc.
 * 
 * user_data: Stuff like hostname, username, password, etc...
 */
void storage_initialize(gpointer user_data)
{
	
	/* Init XmlParser here. If possible, then maybe we save time
	 * if we don't always recreate it
	 */
	/* We do nothing here. We share the xml parser with the content parser */
	
}

/**
 * Close all open connections, files, etc. Then free all memory that
 * was allocated by the storage_initialize() function.
 */
void storage_destroy(void)
{
}




void handle_start_element(xmlTextReader *reader, Note *note)
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
			note->version = atof(attr_value);
			g_free(attr_value);
			attr_value = NULL;
		} else {
			g_printerr("ERROR: Couldn't parse note version.\n");
		}
	
	case TITLE:
		note->title = g_strdup(value);
		break;
	
	case NOTE_CONTENT:
		attr_value = xmlTextReaderGetAttribute(reader, BAD_CAST "version");
		if (attr_value != NULL) {
			note->content_version = atof(attr_value);
			g_free(attr_value);
			attr_value = NULL;
		} else {
			g_printerr("ERROR: Couldn't parse content version.\n");
		}
		
		note->content = (gchar*)xmlTextReaderReadOuterXml(reader);
		break;
		
	case LAST_CHANGE_DATE:
		note->last_change_date = get_iso8601_time_in_seconds(value);
		break;
		
	case LAST_METADATA_CHANGE_DATE:
		note->last_metadata_change_date = get_iso8601_time_in_seconds(value);
		break;
		
	case CREATE_DATE:
		note->create_date = get_iso8601_time_in_seconds(value);
		break;
		
	case CURSOR_POSITION:
		note->cursor_position = atoi(value);
		break;
		
	case WIDTH:
		note->width = atoi(value);
		break;
		
	case HEIGHT:
		note->height = atoi(value);
		break;
		
	case X:
		note->x = atoi(value);
		break;
		
	case Y:
		note->y = atoi(value);
		break;
		
	case OPEN_ON_STARTUP:
		note->open_on_startup = atoi(value);
		break;
		
	case TAG:
		note_add_tag(note, g_strdup(value));
		break;
		
	default:
		break;
	}
	
	g_free(value);
	g_free(attr_value);
}



const gchar* enum_to_string(XmlTag tag)
{
	switch (tag) {
	case NOTE:                      return NOTE_TAG;
	case TITLE:                     return TITLE_TAG;
	case TEXT:                      return TEXT_TAG;
	case NOTE_CONTENT:              return NOTE_CONTENT_TAG;
	case LAST_CHANGE_DATE:          return LAST_CHANGE_DATE_TAG;
	case LAST_METADATA_CHANGE_DATE:	return LAST_METADATA_CHANGE_DATE_TAG;
	case CREATE_DATE:               return CREATE_DATE_TAG;
	case CURSOR_POSITION:           return CURSOR_POSITION_TAG;
	case WIDTH:                     return WIDTH_TAG;
	case HEIGHT:                    return HEIGHT_TAG;
	case X:                         return X_TAG;
	case Y:                         return Y_TAG;
	case OPEN_ON_STARTUP:           return OPEN_ON_STARTUP_TAG;
	case TAGS:                      return TAGS_TAG;
	case TAG:                       return TAG_TAG;
	default:                        g_assert_not_reached();
	}
	
}

XmlTag string_to_enum(const gchar *string)
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
	/*
	g_printerr("ERROR: Unknown tag <%s>\n", string);
	g_assert_not_reached();
	*/
	return UNKNOWN;
}
	
static
void process_note(xmlTextReader *reader, Note *note)
{
	if (xmlTextReaderNodeType(reader) == XML_ELEMENT_NODE) {
		handle_start_element(reader, note);
	}
}


/**
 * Loads a complete note from the storage backend. The note must be freed
 * when you're done with it.
 * If an error occurs NULL is returned.
 */
Note* storage_load_note(const gchar *guid)
{
	AppData *app_data = app_data_get();
	int ret;
	Note *note;
	gchar *filename;
	xmlTextReader *reader;
	
	g_assert(guid != NULL);
	
	filename = g_strconcat(app_data->user_path, guid, ".note", NULL);
	
	/* We try to reuse the existing xml parser. If none exists yet, we create a new one. */
	/*
	if (app_data->reader == NULL) {
		app_data->reader = xmlReaderForFile(filename, NULL, 0);
	}
	
	if (xmlReaderNewFile(app_data->reader, filename, NULL, 0) != 0) {
		g_printerr("ERROR: Cannot reuse xml parser. \n");
		g_assert_not_reached();
	}
	
	if (app_data->reader == NULL) {
		g_printerr("ERROR: Cannot open file: %s\n", filename);
		g_assert_not_reached();
	}
	*/
	/* Reusing the xml parser does not work here. TODO: File a bug for libxml2 */
	reader = xmlReaderForFile(filename, NULL, 0);
	/*xmlReaderNewFile(reader, filename, NULL, 0);*/ /* To reproduce uncomment this line */
	
	note = note_create_new();
	note->guid = g_strdup(guid);
	
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


/* =============================================================== */




static
void write_header(xmlTextWriter *writer, Note *note)
{
	int rc;
	gchar version[20];
	
	/* Enable indentation */
	rc = xmlTextWriterSetIndent(writer, TRUE);
	
	/* Start document */
	rc = xmlTextWriterStartDocument(writer, "1.0", "utf-8", NULL);
	  
	/* Start note element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "note");
	
	g_sprintf(version, "%.1f", note->version);
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST &version);
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
	rc = xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");
	  
	/* Title element */
	rc = xmlTextWriterWriteElement(writer, BAD_CAST "title", BAD_CAST note->title);
	
	/* Start text element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "text");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml", BAD_CAST "space", NULL, BAD_CAST "preserve");
}

static
void write_footer(xmlTextWriter *writer, Note *note) 
{
	int rc;
	GList *tags;
	
	/* Enable indentation */
	rc = xmlTextWriterSetIndent(writer, TRUE);
		  
	/* Meta data tags */
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "last-change-date", "%s", get_time_in_seconds_as_iso8601(note->last_change_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "last-metadata-change-date", "%s", get_time_in_seconds_as_iso8601(note->last_metadata_change_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "create-date", "%s", get_time_in_seconds_as_iso8601(note->create_date));
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "cursor-position", "%i", note->cursor_position);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "width", "%i", note->width);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "height", "%i", note->height);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "x", "%i", note->x);
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "y", "%i", note->y);
	
	/* Write tags */
	tags = note->tags;
	if (tags != NULL) {
		rc = xmlTextWriterStartElement(writer, BAD_CAST "tags");
		while (tags != NULL) {
			gchar *tag = tags->data;
			rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tag", "%s", tag);
			tags = tags->next;
		}
		rc = xmlTextWriterEndElement(writer);
	}
	
	if (note->open_on_startup == TRUE) {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "True");
	} else {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "False");
	}

	/* End the document */
	rc = xmlTextWriterEndDocument(writer);
}



/**
 * Saves a note into the storage backend. The containing GUID is used
 * as identifier.
 * Returns TRUE if successfull.
 */
gboolean storage_save_note(Note *note)
{
	AppData *app_data = app_data_get();
	xmlTextWriter *writer;
	gchar *filename;
	
	g_assert(note->guid != NULL);
	
	filename = g_strconcat(app_data->user_path, note->guid, ".note", NULL);
	
	writer = xmlNewTextWriterFilename(filename, 0);
	if (writer == NULL) {
		g_printerr("ERROR: XmlWriter is NULL \n");
		return FALSE;
	}
	
	write_header(writer, note);
	xmlTextWriterWriteRaw(writer, note->content);
	write_footer(writer, note);
	
	xmlFreeTextWriter(writer);
	g_free(filename);
	
	return TRUE;
}

/**
 * Returns a list of gchar* GUIDs which can be used
 * to load a note.
 * Free the list and the containing strings if you're done.
 */
GList* storage_get_all_note_ids() {
	
	const gchar *filename;
	AppData *app_data = app_data_get();
	GDir *dir = g_dir_open(app_data->user_path, 0, NULL);
	GList *result = NULL;
	
	while ((filename = g_dir_read_name(dir)) != NULL) {
		if (g_str_has_suffix(filename, ".note")) {
			gchar *guid = g_strndup(filename, g_utf8_strlen(filename, -1) - 5);
			result = g_list_prepend(result, guid);
		}
	}
	
	g_dir_close(dir);
	
	return result;
}

/**
 * Deletes a note with the identifier "guid" from the storage backend.
 * Returns: TRUE when successfull, FALSE otherwise.
 */
gboolean storage_delete_note(const gchar *guid) {
	return FALSE;
}

/**
 * Returns the time in seconds that pass between a note modification and the
 * actual saving.
 * Note: Tomboy uses 4 seconds with its xml backend.
 */ 
gint storage_get_save_interval()
{
	return 4;
}
