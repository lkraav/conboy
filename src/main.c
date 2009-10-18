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
#include "conboy_note.h"
#include "conboy_plugin_info.h"
#include "conboy_plugin.h"
#include "conboy_note_store.h"
#include "orientation.h"
#include "fullscreenmanager.h"


#define APP_NAME "conboy"
#define APP_SERVICE "de.zwong.conboy"
#define APP_METHOD "/de/zwong/conboy"


static void cleanup()
{
	AppData *app_data = app_data_get();
	gtk_list_store_clear(GTK_LIST_STORE(app_data->note_store));

	/* Free AppData */
	app_data_free();
	
	conboy_xml_reader_free();
}

static gint
dbus_handler(const gchar *interface, const gchar *method, GArray *arguments, gpointer user_data, osso_rpc_t *retval)
{
	g_printerr("Method: %s\n", method);
	GtkWindow *win = GTK_WINDOW(user_data);
	gtk_window_present(win);

	if (g_strcasecmp(method, "authenticated") == 0) {

		/*
		 * TODO: Implement something that checks
		 * whether the config/auth window is open
		 * and if yes, that it is authorized.
		 * Change the UI and so on...
		 */

		g_printerr("___ CORRECTLY AUTHENTICATED ____");


	}
	return OSSO_OK;
}

#ifdef HILDON_HAS_APP_MENU
static void
on_window_is_topmost_changed (HildonProgram *prog, GParamSpec *prop, gpointer data)
{
	if (hildon_program_get_is_topmost(prog)) {
		orientation_enable_accelerators();
	} else {
		orientation_disable_accelerators();
	}
}
#endif

int
main (int argc, char *argv[])
{
  HildonProgram *program;
  AppData *app_data;

  /* Startup message */
  g_printerr("Starting %s, Version %s \n", APP_NAME, VERSION);

  /* Init i18n */
  locale_init();

  /* Init threads */
  g_thread_init(NULL);
  gdk_threads_init();

  /* Init GTK */
#ifdef HILDON_HAS_APP_MENU
  g_printerr("HILDON INIT \n");
  hildon_gtk_init(&argc, &argv);
#else
  gtk_init(&argc, &argv);
#endif

  /* Call this to initialize it */
  app_data_init();
  app_data = app_data_get();

  /* Initialize maemo application */
  app_data->osso_ctx = osso_initialize(APP_SERVICE, VERSION, TRUE, NULL);

  /* Check that initialization was ok */
  if (app_data->osso_ctx == NULL) {
      return OSSO_ERROR;
  }

  /* Create the Hildon program and setup the title */
  program = HILDON_PROGRAM(hildon_program_get_instance());
  g_set_application_name("Conboy");


#ifdef HILDON_HAS_APP_MENU
  g_signal_connect(program, "notify::is-topmost", G_CALLBACK(on_window_is_topmost_changed), NULL);
  orientation_init(app_data);
#endif


  app_data->note_window = create_mainwin();
  hildon_program_add_window(app_data->program, HILDON_WINDOW(app_data->note_window->window));

  /* Register URL listener */
  if (osso_rpc_set_cb_f(app_data->osso_ctx, APP_SERVICE, APP_METHOD, APP_SERVICE, dbus_handler, app_data->note_window->window) != OSSO_OK) {
      g_printerr("Failed to set callback\n");
  }

  /* EXPERIEMNT */
  /*
  gtk_widget_hide(GTK_WIDGET(app_data->note_window->toolbar));

  FullscreenManager *mgr = fullscreen_create_manager(GTK_WINDOW(app_data->note_window->window));
  fullscreen_enable(mgr);
  */

  /****/

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();

  cleanup();

  /* Deinitialize OSSO */
  osso_deinitialize(app_data->osso_ctx);

  return 0;
}

