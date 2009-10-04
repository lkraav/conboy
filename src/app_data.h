#ifndef APP_DATA_H_
#define APP_DATA_H_

#include <gconf/gconf-client.h>
#include <hildon/hildon-program.h>
#include <libxml/xmlreader.h>
#include <libosso.h>

#include "conboy_note_store.h"
#include "conboy_plugin_store.h"

typedef struct
{
	/*GList         *open_notes;*/
	/*GList		  *open_windows;*/
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	gboolean       portrait;
	HildonWindow  *search_window;
	UserInterface  *note_window;
	ConboyNoteStore *note_store;
	xmlTextReader *reader;
	osso_context_t *osso_ctx;
	ConboyStorage  *storage;
	ConboyPluginStore *plugin_store;
	GList *note_history;
	GList *current_element;
	gboolean started;
} AppData;

void app_data_init(void);
AppData* app_data_get(void);
void app_data_free(void);

#endif /*APP_DATA_H_*/
