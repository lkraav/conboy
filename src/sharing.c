
//#include <libxml/xmlmemory.h>
//#include <libxml/debugXML.h>
//#include <libxml/HTMLtree.h>
//#include <libxml/xmlIO.h>
//#include <libxml/xinclude.h>
//#include <libxml/catalog.h>

#include <libxml/HTMLparser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlsave.h>
#include <libxml/xpathInternals.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/extensions.h>



#include "conboy_note.h"

#include "sharing.h"


static void
create_xml_doc(ConboyNote *note, xmlDocPtr *doc)
{

	gchar version[20];
	gdouble note_version;
	gchar *title;
	gchar *content;

	xmlTextWriter *writer;

	g_object_get(note, "title", &title, "content", &content, "note-version", &note_version, NULL);

	//writer = xmlNewTextWriterFilename("/tmp/note.xml", 0);
	writer = xmlNewTextWriterDoc(doc, FALSE);




	/* Enable indentation */
	xmlTextWriterSetIndent(writer, TRUE);

	/* Start document */
	xmlTextWriterStartDocument(writer, "1.0", "utf-8", NULL);

	/* Start note element */
	xmlTextWriterStartElement(writer, BAD_CAST "note");

	g_ascii_formatd(version, 20, "%.1f", note_version);
	xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST &version);
	xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "link", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/link");
	xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xmlns", BAD_CAST "size", NULL, BAD_CAST "http://beatniksoftware.com/tomboy/size");
	xmlTextWriterWriteAttributeNS(writer, NULL, BAD_CAST "xmlns", NULL, BAD_CAST "http://beatniksoftware.com/tomboy");

	/* Title element */
	xmlTextWriterWriteElement(writer, BAD_CAST "title", BAD_CAST title);
	g_free(title);

	/* Start text element */
	xmlTextWriterStartElement(writer, BAD_CAST "text");
	xmlTextWriterWriteAttributeNS(writer, BAD_CAST "xml", BAD_CAST "space", NULL, BAD_CAST "preserve");


	GRegex *regex = g_regex_new(" xmlns(?:.*?)?=\".*?\"", 0, 0, NULL);
	gchar *content_clean = g_regex_replace(regex, content, -1, 0, "", 0, NULL);
	g_regex_unref(regex);

	xmlTextWriterWriteRaw(writer, (const xmlChar *)content_clean);
	g_free(content);
	xmlFree(content_clean);

	xmlTextWriterEndElement(writer); /*</text> */

	xmlTextWriterEndElement(writer); /*</note> */

	xmlFreeTextWriter(writer);
}

void
xslt_to_lower(xmlXPathParserContextPtr ctx, int nargs)
{
	g_printerr("to-lower called with %i params\n", nargs);

	xmlXPathObjectPtr obj = valuePop(ctx);

	/* Convert to string if needed */
	if (obj->type != XPATH_STRING) {
	    valuePush(ctx, obj);
	    xmlXPathStringFunction(ctx, 1);
	    obj = valuePop(ctx);
	}

	g_printerr("Original value: %s\n", obj->stringval);

	gchar *lower = g_strdup("Hallo&nbsp;Welt"); //g_strconcat("\"", g_utf8_strdown(obj->stringval, -1), "\"", NULL);

	g_printerr("Lower value: %s\n", lower);

	/* Push result */
	//xmlXPathObjectPtr result = xmlXPathNewCString(lower);
	xmlXPathObjectPtr result = xmlXPathNewString(lower);

	valuePush(ctx, result);

	xmlXPathFreeObject(obj);
	g_free(lower);
}

static void*
initFunct (xsltTransformContextPtr ctx, const xmlChar *uri)
{
	g_printerr("initFunct() called\n");

	if (xsltRegisterExtFunction(ctx, "ToLower", "http://beatniksoftware.com/tomboy", xslt_to_lower) == -1) {
		g_printerr("ERROR: Could not register ToLower() function\n");
	}

	return NULL;
}

void
conboy_share_note(ConboyNote *note)
{
	xmlSubstituteEntitiesDefault(1);

	xmlLoadExtDtdDefaultValue = 1;

	/* Register extension */
	xsltRegisterExtModule("http://beatniksoftware.com/tomboy", initFunct, NULL);

	g_printerr("After registering module\n");

	/* Parse style sheet */
	xmlChar *style_sheet = NULL;
	xsltStylesheetPtr ptr = xsltParseStylesheetFile("/home/conny/workspace/conboy/data/export_to_html.xsl");

	g_printerr("After parsing style sheet\n");

	/* Parse note */
	xmlDocPtr doc;
	create_xml_doc(note, &doc);

	g_printerr("After creating note document\n");

/*
	xmlSaveCtxtPtr ctxt = xmlSaveToFilename( "/tmp/doc.xml", NULL, 0 );
	long ret = xmlSaveDoc( ctxt, doc );
	ret = xmlSaveClose( ctxt );
*/


	/* Set params */
	const char *params[8+1];
	params[0] = "export-linked";
	params[1] = "false"; /* or 0? */
	params[2] = "export-linked-all";
	params[3] = "false"; /* or 0? */
	params[4] = "root-note";
	params[5] = g_strconcat("'", note->title, "'", NULL);
	params[6] = "font";
	params[7] = "'font-family: Arial;'";
	params[8] = NULL;

	/* Apply style sheet */
	xmlDocPtr res = xsltApplyStylesheet(ptr, doc, params);

	g_printerr("After applying stylesheet\n");

	xsltSaveResultToFilename("/tmp/notes.html", res, ptr, 0);

	g_printerr("After saving\n");

	/* Cleanup */
	xsltFreeStylesheet(ptr);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);
	xsltCleanupGlobals();
}
