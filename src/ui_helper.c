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

#include <gtk/gtk.h>

#include "ui_helper.h"

/**
 * Opens yes/no dialog which supports markup in message.
 *
 * return true on yes, false on no. Defaults to no
 */
GtkWidget*
ui_helper_create_yes_no_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			" ",
			parent,
			GTK_DIALOG_MODAL,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			NULL);

	GtkWidget *label = gtk_label_new("");
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_markup(GTK_LABEL(label), message);
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	return dialog;
}

GtkWidget*
ui_helper_create_confirmation_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			" ",
			parent,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	GtkWidget *label = gtk_label_new("");
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_markup(GTK_LABEL(label), message);
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	return dialog;
}

void
ui_helper_show_confirmation_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = ui_helper_create_confirmation_dialog(parent, message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
