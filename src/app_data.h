#ifndef APP_DATA_H_
#define APP_DATA_H_

#include <gconf/gconf-client.h>
#include <hildon/hildon-program.h>
#include <libxml/xmlreader.h>

#include "note_list_store.h"

typedef struct
{
	const gchar   *user_path;
	GList         *open_notes;
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	gboolean       portrait;
	HildonWindow  *search_window;
	NoteListStore *note_store;
	xmlTextReader *reader;
} AppData;

AppData* app_data_get(void);
void app_data_free(void);

#endif /*APP_DATA_H_*/
