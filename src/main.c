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
#include <stdio.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#include "support.h"
#include "serializer.h"
#include "deserializer.h"
#include "metadata.h"
#include "interface.h"
#include "callbacks.h"
#include "note.h"

#include "localisation.h"

#define APP_NAME "conboy"
#define APP_VER "0.1"
#define APP_SERVICE "de.zwong.conboy"
#define APP_METHOD "/de/zwong/conboy"




int
main (int argc, char *argv[])
{
  HildonProgram *program;
  Note *metadata = g_slice_alloc0(sizeof(Note));
  osso_context_t *osso_context;
  AppData *app_data;
  
  
  /* Init GTK */
  gtk_init (&argc, &argv);
  
  /* Call this to initialize it */
  app_data = get_app_data();
  
  /* Initialize maemo application */
  osso_context = osso_initialize(APP_SERVICE, APP_VER, TRUE, NULL);
  
  /* Check that initialization was ok */
  if (osso_context == NULL) {
      return OSSO_ERROR;
  }
  
  /* Create the Hildon program and setup the title */
  program = HILDON_PROGRAM(hildon_program_get_instance());
  g_set_application_name("Conboy");
 
  note_open(metadata);

  gtk_main();
  
  
  /* Deinitialize OSSO */
  osso_deinitialize(osso_context);
  
  return 0;
}

