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

#include <string.h>

#include "conboy_xml.h"

xmlTextReader *__xml_text_reader = NULL;

xmlTextReader*
conboy_xml_get_reader_for_memory(const gchar *xml_string)
{
	/* We try to reuse the existing xml parser. If none exists yet, we create a new one. */
	if (__xml_text_reader == NULL) {
		__xml_text_reader = xmlReaderForMemory(xml_string, strlen(xml_string), "", "UTF-8", 0);
		return __xml_text_reader;
	}

	if (xmlReaderNewMemory(__xml_text_reader, xml_string, strlen(xml_string), "", "UTF-8", 0) != 0) {
		g_printerr("ERROR: Couldn't reuse xml parser. \n");
		g_assert_not_reached();
	}

	if (__xml_text_reader == NULL) {
		g_printerr("ERROR: Couldn't init xml parser.\n");
		g_assert_not_reached();
	}

	return __xml_text_reader;
}

xmlTextReader*
conboy_xml_get_reader_for_file(const gchar *file_name)
{
	/* Reusing the parser is not possible due to a bug in libxml2 */
	if (__xml_text_reader != NULL) {
		xmlFreeTextReader(__xml_text_reader);
		__xml_text_reader = NULL;
	}

	__xml_text_reader = xmlReaderForFile(file_name, "UTF-8", 0);

	/* We try to reuse the existing xml parser. If none exists yet, we create a new one. */
	/*
	if (__xml_text_reader == NULL) {
		__xml_text_reader = xmlReaderForFile(file_name, "UTF-8", 0);
		return __xml_text_reader;
	}

	if (xmlReaderNewFile(__xml_text_reader, file_name, "UTF-8", 0) != 0) {
		g_printerr("ERROR: Cannot reuse xml parser. \n");
		g_assert_not_reached();
	}

	if (__xml_text_reader == NULL) {
		g_printerr("ERROR: Couldn't init xml parser.\n");
		g_assert_not_reached();
	}
	*/

	return __xml_text_reader;
}

void conboy_xml_reader_free()
{
	if (__xml_text_reader) {
		xmlFreeTextReader(__xml_text_reader);
		xmlCleanupParser();
	}
}

