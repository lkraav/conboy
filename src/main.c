/*******************************************************************************
 */

/*
 ============================================================================
 Name        : main.c
 Author      : Cornelius Hald
 Version     : 0.1
 Description : Hildon GUI Application in C
 ============================================================================
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

