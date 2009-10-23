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

#ifndef APP_DATA_H_
#define APP_DATA_H_

#include <gconf/gconf-client.h>
#include <hildon/hildon-program.h>
#include <libxml/xmlreader.h>
#include <libosso.h>

#include "conboy_note_store.h"
#include "conboy_plugin_store.h"
#include "fullscreenmanager.h"

typedef struct
{
	GConfClient   *client;
	HildonProgram *program;
	gboolean	   fullscreen;
	gboolean       portrait;
	HildonWindow  *search_window;
	UserInterface  *note_window;
	ConboyNoteStore *note_store;
	osso_context_t *osso_ctx;
	ConboyStorage  *storage;
	ConboyPluginStore *plugin_store;
	GList *note_history;
	GList *current_element;
	gboolean started;
	gboolean accelerators;
} AppData;

void app_data_init(void);
AppData* app_data_get(void);
void app_data_free(void);

#endif /*APP_DATA_H_*/
