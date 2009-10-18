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

#ifndef __CONBOY_XML_H__
#define __CONBOY_XML_H__

#include <glib.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>



xmlTextReader* conboy_xml_get_reader_for_file(const gchar *file_name);
xmlTextReader* conboy_xml_get_reader_for_memory(const gchar *xml_string);

void conboy_xml_reader_free(void);



#endif /* __CONBOY_XML_H__ */
