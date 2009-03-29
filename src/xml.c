#include <stdio.h>

#include <libxml/xmlreader.h>
#include <gtk/gtk.h>

#include "xml.h"

int position = 0;
GtkTextTag *bold;

/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
void processNode(xmlTextReaderPtr reader, GtkTextBuffer *buffer) {
	const xmlChar *name, *value;
	int type;
	GtkTextMark *first, *second;
	GtkTextIter start, end;
	
	
	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

	value = xmlTextReaderConstValue(reader);
	type = xmlTextReaderNodeType(reader);
	
	if (g_ascii_strcasecmp(name, "bold") == 0) {
		if (type == XML_READER_TYPE_ELEMENT) {
			gtk_text_iter_set_offset(&start, position);
			first = gtk_text_buffer_create_mark(buffer, "start", &start, TRUE);
		}
		if (type == XML_READER_TYPE_END_ELEMENT) {
			gtk_text_iter_set_offset(&end, position);
			
			second = gtk_text_buffer_create_mark(buffer, "end", &end, TRUE);
			first = gtk_text_buffer_get_mark(buffer, "start");
			
			gtk_text_buffer_get_iter_at_mark(buffer, &start, first);
			gtk_text_buffer_get_iter_at_mark(buffer, &end, second);
			
			gtk_text_buffer_apply_tag(buffer, bold, &start, &end);
		}
		XML_READER_TYPE_ELEMENT; /* 1 */
		XML_READER_TYPE_END_ELEMENT; /* 15 */
		/*
		char x[10];
		sprintf(x, "*%i*", type);
		gtk_text_buffer_insert_at_cursor(buffer, x, -1);
		*/
	}
	
	if (g_ascii_strcasecmp(name, "#text") == 0) {
		gtk_text_buffer_insert_at_cursor(buffer, value, -1);
	}
	
	position = gtk_text_buffer_get_char_count(buffer);
	
	/*
	printf("%d %d %s %d %d", xmlTextReaderDepth(reader),
			xmlTextReaderNodeType(reader), name,
			xmlTextReaderIsEmptyElement(reader), xmlTextReaderHasValue(reader));
	*/
	/*
	if (value == NULL)
		printf("\n");
	else {
		if (xmlStrlen(value) > 40)
			printf(" %.40s...\n", value);
		else
			printf(" %s\n", value);
	}
	*/
}

void load_file_to_buffer(const gchar *filename, GtkTextBuffer *buffer) {

	xmlTextReaderPtr reader;
	int ret;
	gchar *message;
	gint formats;
	
	/* Empty the text widget */
	gtk_text_buffer_set_text(buffer, "", -1);
	
	/* Create tags */
	bold = gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);

	gtk_text_buffer_get_serialize_formats(buffer, &formats);
	g_print("Supported formats: %i", &formats);
	
	
	reader = xmlReaderForFile(filename, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			processNode(reader, buffer);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			message = g_strdup_printf("Failed to parse: %s", filename);
			gtk_text_buffer_set_text(buffer, message, -1);
			g_free(message);
		}
	} else {
		message = g_strdup_printf("Unable to open: %s", filename);
		gtk_text_buffer_set_text(buffer, message, -1);
		g_free(message);
	}
	
	
	
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
void streamFile(const char *filename) {
/*
	xmlTextReaderPtr reader;
	int ret;

	reader = xmlReaderForFile(filename, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			processNode(reader);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			fprintf(stderr, "%s : failed to parse\n", filename);
		}
	} else {
		fprintf(stderr, "Unable to open %s\n", filename);
	}
	*/
}


/**
 * example1Func:
 * @filename: a filename or an URL
 *
 * Parse the resource and free the resulting tree
 */
void example1Func(const char *filename) {
	xmlDocPtr doc; /* the resulting document tree */

	doc = xmlReadFile(filename, NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse %s\n", filename);
		return;
	}
	xmlFreeDoc(doc);
}

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
void print_element_names(xmlNode * a_node) {
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s\n", cur_node->name);
		}

		print_element_names(cur_node->children);
	}
}

