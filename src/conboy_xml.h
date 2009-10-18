
#ifndef __CONBOY_XML_H__
#define __CONBOY_XML_H__

#include <glib.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>



xmlTextReader* conboy_xml_get_reader_for_file(const gchar *file_name);
xmlTextReader* conboy_xml_get_reader_for_memory(const gchar *xml_string);

void conboy_xml_reader_free(void);



#endif /* __CONBOY_XML_H__ */
