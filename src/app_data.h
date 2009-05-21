#ifndef APP_DATA_H_
#define APP_DATA_H_

#include <gconf/gconf-client.h>
#include <hildon/hildon-program.h>

#include "note_list_store.h"

typedef struct
{
	const gchar   *user_path;
	GList         *open_notes;
	gint           font_size;
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	HildonWindow  *search_window;
	NoteListStore *note_store;
} AppData;

AppData* app_data_get(void);
void app_data_free(void);

#endif /*APP_DATA_H_*/
