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


/* Includes */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>
#include <gtk/gtkmain.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <libosso.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/xmlreader.h>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <stdio.h>

#include <libintl.h>
#include <locale.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#include "metadata.h"
#include "interface.h"
#include "callbacks.h"
#include "note.h"
#include "app_data.h"

#include "search_window.h"
#include "note_list_store.h"


#define APP_NAME "conboy"
#define APP_SERVICE "de.zwong.conboy"
#define APP_METHOD "/de/zwong/conboy"

#define MY_ENCODING "utf-8"

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


int
main (int argc, char *argv[])
{
  HildonProgram *program;
  Note *note;
  osso_context_t *osso_context;
  AppData *app_data;

  /* Init i18n */
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
  
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
  osso_context = osso_initialize(APP_SERVICE, VERSION, TRUE, NULL);

  /* Check that initialization was ok */
  if (osso_context == NULL) {
      return OSSO_ERROR;
  }

  /* Create the Hildon program and setup the title */
  program = HILDON_PROGRAM(hildon_program_get_instance());
  g_set_application_name("Conboy");

  /* Get the most recent note. If there is none, create new. */
  note = note_list_store_get_latest(app_data->note_store);
  if (note == NULL) {
	  note = note_create_new();
  }

  note_show(note);

  gtk_main();

  cleanup();
  
  /* Deinitialize OSSO */
  osso_deinitialize(osso_context);

  return 0;
}

