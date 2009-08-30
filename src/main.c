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

#include "localisation.h"

#include <hildon/hildon-program.h>
#include <gtk/gtkmain.h>
#include <libosso.h>

#include "app_data.h"
#include "settings.h"
#include "search_window.h"
/*#include "json.h"*/
#include "conboy_note.h"
#include "conboy_plugin_info.h"
#include "conboy_plugin.h"
#include "conboy_note_store.h"


#define APP_NAME "conboy"
#define APP_SERVICE "de.zwong.conboy"
#define APP_METHOD "/de/zwong/conboy"


static void cleanup()
{
	AppData *app_data = app_data_get();
	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL(app_data->note_store);
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		Note *note;
		gtk_tree_model_get(model, &iter, NOTE_COLUMN, &note, -1);
		note_free(note);
		valid = gtk_tree_model_iter_next(model, &iter);
	}
	
	/* Free AppData */
	app_data_free();
}

static gint dbus_handler(const gchar *interface,
                         const gchar *method,
                         GArray *arguments,
                         gpointer user_data,
                         osso_rpc_t *retval)
{
	if (g_strcasecmp(method, "present_window") == 0) {
		
		/*
		 * TODO: Implement something that checks
		 * whether the config/auth window is open
		 * and if yes, that it is authorized.
		 * Change the UI and so on...
		 */
		
		
	}
	return OSSO_OK;
}


int
main (int argc, char *argv[])
{
  HildonProgram *program;
  ConboyNote *note;
  AppData *app_data;

  /* Init i18n */
  locale_init();
  
  /* Init GTK */
#ifdef HILDON_HAS_APP_MENU
  g_printerr("HILDON INIT \n");
  hildon_gtk_init(&argc, &argv);
#else
  gtk_init(&argc, &argv);
#endif
  

  /* Call this to initialize it */
  app_data = app_data_get();

  /* Initialize maemo application */
  g_printerr("Starting %s, Version %s \n", APP_NAME, VERSION);
  app_data->osso_ctx = osso_initialize(APP_SERVICE, VERSION, TRUE, NULL);

  /* Check that initialization was ok */
  if (app_data->osso_ctx == NULL) {
      return OSSO_ERROR;
  }

  /* Create the Hildon program and setup the title */
  program = HILDON_PROGRAM(hildon_program_get_instance());
  g_set_application_name("Conboy");
  
  
  /****************/
  
  
  
  /*****************/
  


  if (settings_load_startup_window() == SETTINGS_STARTUP_WINDOW_NOTE) {
	  
	  note = conboy_note_store_get_latest(app_data->note_store);
	  if (note == NULL) {
		  note = conboy_note_new();
	  }
	  note_show(note);
  } else {
	  search_window_open();
  }

  
  gtk_main();

  cleanup();
  
  /* Deinitialize OSSO */
  osso_deinitialize(app_data->osso_ctx);

  return 0;
}

