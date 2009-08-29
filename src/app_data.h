#ifndef APP_DATA_H_
#define APP_DATA_H_

#include <gconf/gconf-client.h>
#include <hildon/hildon-program.h>
#include <libxml/xmlreader.h>
#include <libosso.h>

#include "conboy_note_store.h"

typedef struct
{
	const gchar   *user_path;
	GList         *open_notes;
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	gboolean       portrait;
	HildonWindow  *search_window;
	ConboyNoteStore *note_store;
	xmlTextReader *reader;
	osso_context_t *osso_ctx;
	ConboyStorage  *storage;
} AppData;

AppData* app_data_get(void);
void app_data_free(void);

#endif /*APP_DATA_H_*/
