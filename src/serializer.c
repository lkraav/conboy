#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <string.h>

#include "note.h"
#include "metadata.h"
#include "serializer.h"

gint depth = 0;
gint new_depth = 0;
gboolean list_active = FALSE;
gboolean is_bullet = FALSE;

static
void write_header(xmlTextWriter *writer, Note *note)
{
	int rc;
	
	/* Enable indentation */
	rc = xmlTextWriterSetIndent(writer, TRUE);
	
	/* Start document */
	rc = xmlTextWriterStartDocument(writer, "1.0", "utf-8", NULL);
	  
	/* Start note element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "note");
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST "0.3");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
	rc = xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");
	  
	/* Title element */
	rc = xmlTextWriterWriteElement(writer, BAD_CAST "title", BAD_CAST note->title);
}


/**
 * Writes a start element.
 */
static void write_start_element(GtkTextTag *tag, xmlTextWriter *writer)
{
	gchar** strings;
	gchar *tag_name;
	
	tag_name = g_strdup(tag->name);
	
	/* Ignore tags that start with "_". They are considered internal. */
	if (g_ascii_strncasecmp(tag_name, "_", 1) == 0) {
		return;
	}
	
	/* Ignore <list> tags. */
	if (g_ascii_strncasecmp(tag_name, "list", -1) == 0) {
		list_active = TRUE;
		return;
	}
	
	/* Use tag_get_depth() here. Currently in callbacks.c */
	/* If a <depth> tag, ignore */
	if (strncmp(tag_name, "depth", 5) == 0) {
		is_bullet = TRUE;
		/* NEW */
		strings = g_strsplit(tag_name, ":", 2);
		new_depth = atoi(strings[1]);
		g_strfreev(strings);
		/* /NEW */
		return;
	}
	
	/* If not a <list-item> tag, write it and return */
	if (g_ascii_strncasecmp(tag_name, "list-item", 9) != 0) {
		xmlTextWriterStartElement(writer, BAD_CAST tag_name);
		return;
	}
	
	/* It is a <list-item:*> tag */
	/*
	g_printerr("depth    : %i \n", depth);
	g_printerr("new_depth: %i \n", new_depth);
	*/
	
	if (new_depth < depth) {
		gint diff = depth - new_depth;
		/*g_printerr("new_depth < depth. DIFF: %i \n", depth - new_depth);*/
		
		/* </list-item> */
		xmlTextWriterEndElement(writer);
		
		while (diff > 0) { /* For each depth we need to close a <list-item> and a <list> tag. */
			/* </list> */
			xmlTextWriterEndElement(writer);
			/* </list-item> */
			xmlTextWriterEndElement(writer);
			diff--;
		}
		
		/* <list-item dir=ltr> */
		xmlTextWriterStartElement(writer, BAD_CAST "list-item");
		xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
	}
	
	
	/* If there was an increase in depth, we need to add a <list> tag */
	if (new_depth > depth) {
		gint diff = new_depth - depth;
		/*g_printerr("new_depth > depth. DIFF: %i \n", new_depth - depth);*/
		
		while (diff > 0) {
			xmlTextWriterStartElement(writer, BAD_CAST "list");
			xmlTextWriterStartElement(writer, BAD_CAST "list-item");
			xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
			diff--;
		}
		
	} else if (new_depth == depth) {
		xmlTextWriterEndElement(writer); /* </list-item> */
		xmlTextWriterStartElement(writer, BAD_CAST "list-item");
		xmlTextWriterWriteAttribute(writer, BAD_CAST "dir", BAD_CAST "ltr");
	}
	
	depth = new_depth;
	/*g_strfreev(strings);*/
}

/**
 * Writes an end element.
 */
static void write_end_element(GtkTextTag *tag, xmlTextWriter *writer)
{
	gchar *tag_name;
	
	tag_name = g_strdup(tag->name);
	
	/* Ignore tags that start with "_". They are considered internal. */
	if (g_ascii_strncasecmp(tag_name, "_", 1) == 0) {
		return;
	}
	
	/* Ignore depth tags */
	if (strncmp(tag_name, "depth", 5) == 0) {
		is_bullet = FALSE;
		return;
	}
	
	/* If the list completely ends, reset the depth */
	if (g_ascii_strncasecmp(tag_name, "list", -1) == 0) {
		
		/* Close all open <list-item> and <list> tags */
		while (depth > 0) {
			/* </list-item> */
			xmlTextWriterEndElement(writer);
			/* </list> */
			xmlTextWriterEndElement(writer);
			depth--;
		}
		
		list_active = FALSE;
		return;
	}
	
	/* If it is not a <list-item> tag, just close it and return */
	if (g_ascii_strncasecmp(tag_name, "list-item", 9) != 0) {
		xmlTextWriterEndElement(writer);
		return;
	}
	
	
	/*g_free(tag_name);*/
}

/**
 * Writes plain text.
 */
static void write_text(const gchar *text, xmlTextWriter *writer)
{
	/* Don't write bullets to the output */
	if (is_bullet) {
		return;
	}
	
	xmlTextWriterWriteString(writer, BAD_CAST text);
}

/**
 * Sort two GtkTextTags by their priority. Higher priority (bigger number)
 * will be sorted to the left of the list
 */
static gint sort_by_prio(GtkTextTag *tag1, GtkTextTag *tag2)
{	
	if (tag1->priority == tag2->priority) {
		g_printerr("ERROR: Two tags cannot have the same priority.\n");
		return 0;
	}
	
	/* If the priority of tag1 is higher is should be sorted to the left */
	if (tag1->priority > tag2->priority) {
		return -1;
	} else {
		return 1;
	}
}

static
void write_content(xmlTextWriter *writer, Note *note)
{
	int rc = 0;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	GtkTextIter *old_iter, *iter;
	gchar *text;
	GSList *start_tags = NULL, *end_tags = NULL;
	
	/* Start text element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "text");
	rc = xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml", BAD_CAST "space", NULL, BAD_CAST "preserve");
	
	/* Disable indentation */
	rc = xmlTextWriterSetIndent(writer, FALSE);
	  
	/* Start note-content element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "note-content");
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST "0.1");
	
	/*****************************************************/
	
	
	buffer = note->ui->buffer;
	
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	iter = gtk_text_iter_copy(&start);
	old_iter = gtk_text_iter_copy(&start);
	
	while (gtk_text_iter_compare(iter, &end) <= 0) {
		
		start_tags = gtk_text_iter_get_toggled_tags(iter, TRUE);
		end_tags   = gtk_text_iter_get_toggled_tags(iter, FALSE);
		
		/* Write end tags */
		g_slist_foreach(end_tags, (GFunc)write_end_element, writer);
		
		/* Sort start tags by priority */
		start_tags = g_slist_sort(start_tags, (GCompareFunc)sort_by_prio);
		
		/* Write start tags */
		g_slist_foreach(start_tags, (GFunc)write_start_element, writer);
		
		/* Move iter */
		/* Remember position and set iter to next toggle */
		gtk_text_iter_free(old_iter);
		old_iter = gtk_text_iter_copy(iter);
		
		if (gtk_text_iter_compare(iter, &end) >= 0) {
			break;
		}
		gtk_text_iter_forward_to_tag_toggle(iter, NULL);
		
		/* Write text */
		text = gtk_text_iter_get_text(old_iter, iter);
		write_text(text, writer);
	}
	
	/* TODO: Free this stuff */
	/*
	g_slist_free(start_tags);
	g_slist_free(end_tags);
	g_free(text);
	*/
	
	
	/**************************************************/
	  
	/* Close note-content */
	rc = xmlTextWriterEndElement(writer);
	  
	/* Close text */
	rc = xmlTextWriterEndElement(writer);
	
	/* Insert linebreak like in Tomboy format */
	rc = xmlTextWriterWriteRaw(writer, BAD_CAST "\n");	
}

static
void write_footer(xmlTextWriter *writer, Note *note) 
{
	int rc;
	
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
	if (note->open_on_startup == TRUE) {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "True");
	} else {
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "open-on-startup", "False");
	}

	/* End the document */
	rc = xmlTextWriterEndDocument(writer);
}

void serialize_note(Note *note)
{
	xmlTextWriter *writer;
	
	if (note->filename == NULL) {
		g_printerr("ERROR: Cannot save, filename of note is not set.\n");
		return;
	}
	
	writer = xmlNewTextWriterFilename(note->filename, FALSE);
	xmlTextWriterSetIndentString(writer, BAD_CAST "  ");
	
	write_header(writer, note);
	write_content(writer, note);
	write_footer(writer, note);
	
	xmlFreeTextWriter(writer);
}
