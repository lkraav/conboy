/* This file is part of Conboy.
 *
 * Copyright (C) 2010 Cornelius Hald
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


#ifndef CONBOY_WEB_SYNC_H_
#define CONBOY_WEB_SYNC_H_

#include <gtk/gtk.h>

typedef struct
{
	GtkDialog *dialog;
	GtkProgressBar *bar;
	GtkVBox   *box;
	GtkLabel *label;
	GtkButton *button;
} WebSyncDialogData;

gboolean
web_sync_authenticate (gchar *url, GtkWindow *parent);

gpointer
web_sync_do_sync (gpointer *user_data);


#endif /* CONBOY_WEB_SYNC_H_ */
