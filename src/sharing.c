/* This file is part of Conboy.
 *
 * Copyright (C) 2010 Cornelius Hald
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

#include "config.h"
#include "conboy_config.h"

#include <libxml/HTMLparser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlsave.h>
#include <libxml/xpathInternals.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/extensions.h>

//#include <dbus/dbus.h>
#include <glib.h>
#include <dbus/dbus-glib.h>


#ifdef WITH_SHARING
#include <sharing-dialog.h>
#endif
#ifdef WITH_MODEST
#include <libmodest-dbus-client/libmodest-dbus-client.h>
#endif
#ifdef WITH_BT
#include <conbtdialogs-dbus.h>
#endif

#include "app_data.h"
#include "conboy_note.h"
#include "gregex.h"
#include "sharing.h"

#define HTML_FILE_NAME "/tmp/notes.html"

/* TODO: This is mostly copy & paste taken from the XML Storage Plug-In.
 * Maybe the code can somehow be shared?
 */
static void
create_xml_doc (ConboyNote *note, xmlDocPtr *doc)
{
	gchar version[20];
	gdouble note_version;
	gchar *title;
	gchar *content;

	xmlTextWriter *writer;

	g_object_get(note, "title", &title, "content", &content, "note-version", &note_version, NULL);

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

	/* Remove duplicated namespace declarations */
	GRegex *regex = g_regex_new(" xmlns(?:.*?)?=\".*?\"", 0, 0, NULL);
	gchar *content_clean = g_regex_replace(regex, content, -1, 0, "", 0, NULL);
	g_regex_unref(regex);

	/* Write note content */
	xmlTextWriterWriteRaw(writer, (const xmlChar *)content_clean);
	g_free(content);
	xmlFree(content_clean);

	xmlTextWriterEndElement(writer); /*</text> */

	xmlTextWriterEndElement(writer); /*</note> */

	xmlFreeTextWriter(writer);
}

void
xslt_to_lower (xmlXPathParserContextPtr ctx, int nargs)
{
	xmlXPathObjectPtr obj = valuePop(ctx);

	/* Convert to string if needed */
	if (obj->type != XPATH_STRING) {
	    valuePush(ctx, obj);
	    xmlXPathStringFunction(ctx, 1);
	    obj = valuePop(ctx);
	}

	/* Convert to lower case */
	gchar *lower = g_utf8_strdown(obj->stringval, -1);

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
	if (xsltRegisterExtFunction(ctx, "ToLower", "http://beatniksoftware.com/tomboy", xslt_to_lower) == -1) {
		g_printerr("ERROR: Could not register ToLower() function\n");
	}
	return NULL;
}

static void
export_html (ConboyNote *note)
{
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;

	/* Register extension */
	xsltRegisterExtModule("http://beatniksoftware.com/tomboy", initFunct, NULL);

	/* Parse style sheet */
	xmlChar *style_sheet = NULL;
	xsltStylesheetPtr ptr = xsltParseStylesheetFile(PREFIX"/share/conboy/conboy_to_html.xsl");

	/* Parse note */
	xmlDocPtr doc;
	create_xml_doc(note, &doc);

	/* Set params */
	const char *params[8+1];
	params[0] = "export-linked";
	params[1] = "false";
	params[2] = "export-linked-all";
	params[3] = "false";
	params[4] = "root-note";
	params[5] = g_strconcat("'", note->title, "'", NULL);
	params[6] = "font";
	params[7] = "'font-family: Arial;'";
	params[8] = NULL;

	/* Apply style sheet */
	xmlDocPtr res = xsltApplyStylesheet(ptr, doc, params);

	/* Save to temp file */
	xsltSaveResultToFilename(HTML_FILE_NAME, res, ptr, 0);

	/* Cleanup */
	xsltFreeStylesheet(ptr);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);
	xsltCleanupGlobals();
}

#ifdef WITH_SHARING
void
conboy_share_note_via_sharing (ConboyNote *note)
{
	export_html(note);

	/* Open sharing dialog */
	AppData *app_data = app_data_get();
	sharing_dialog_with_file(app_data->osso_ctx, GTK_WINDOW(app_data->note_window->window), HTML_FILE_NAME);

	/* Work around. The sharing dialog calls xmlCleanupParser(), so we have to reinit the xml parser.
	 * https://bugs.maemo.org/show_bug.cgi?id=10849 */
	conboy_reinit_xml_reader();

	/* Remove temp file again */
	/* g_unlink(HTML_FILE_NAME); Dont remove file, otherwise the service cant access it */
}

#endif

#ifdef WITH_BT
void
conboy_share_note_via_bluetooth (ConboyNote *note)
{
	DBusGProxy *proxy = NULL;
	DBusGConnection * sys_conn = NULL;
	GError *error = NULL;
	gboolean result = TRUE;
	gchar **files = NULL;

	export_html(note);

	sys_conn = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);

	if (sys_conn == NULL) {
		g_printerr("ERROR: Could not get system D-Bus.\n");
		return;
	}

	files = g_new0(gchar*, 2);
	*files = HTML_FILE_NAME;
	files [1] = NULL;

	/* Open connection for btdialogs service */
	proxy = dbus_g_proxy_new_for_name (sys_conn,
			CONBTDIALOGS_DBUS_SERVICE,
			CONBTDIALOGS_DBUS_PATH,
			CONBTDIALOGS_DBUS_INTERFACE);

	/* Send send file request to btdialogs service */
	if (!dbus_g_proxy_call (proxy,
			CONBTDIALOGS_SEND_FILE_REQ,
			&error, G_TYPE_STRV, files,
			G_TYPE_INVALID, G_TYPE_INVALID))
	{
	    g_printerr("ERROR: Cannot send file via BT. Message was: %s\n", error -> message);
	    g_clear_error(&error);
	    return;
	}

	g_strfreev(files);
	g_object_unref(G_OBJECT(proxy));
}
#endif /* WITH_BT */

#ifdef WITH_MODEST
void
conboy_share_note_via_email (ConboyNote *note)
{
	AppData *app_data = app_data_get();
	GSList *list = NULL;

	export_html(note);

	list = g_slist_append(list, HTML_FILE_NAME);

	gboolean result = libmodest_dbus_client_compose_mail(app_data->osso_ctx,
			NULL, /* to */
			NULL, /* cc */
			NULL, /* bcc */
			NULL, /* body */
			NULL, /* subject */
			list  /* attachments */);

	g_slist_free(list);

	if (result == FALSE) {
		g_printerr("ERROR: Cannot send file via email. Modest did not repond.\n");
	}
}
#endif /* WITH_MODEST */

