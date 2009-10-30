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
#include <gdk/gdkx.h>
#include <X11/Xatom.h>

#include "app_data.h"

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
ui_helper_show_confirmation_dialog(GtkWindow *parent, const gchar *message, gboolean supports_portrait)
{
	GtkWidget *dialog = ui_helper_create_confirmation_dialog(parent, message);
	if (!supports_portrait) {
		ui_helper_remove_portrait_support(GTK_WINDOW(dialog));
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void
ui_helper_toggle_fullscreen(GtkWindow *active_window)
{
	AppData *app_data = app_data_get();
	gboolean fullscreen = FALSE;

	GdkWindowState state = gdk_window_get_state(GTK_WIDGET(active_window)->window);
	if (state & GDK_WINDOW_STATE_FULLSCREEN) {
		fullscreen = TRUE;
	}

	fullscreen = !fullscreen;

	/* Set main window to fullscreen or unfullscreen */
	if (fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(app_data->note_window->window));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(app_data->note_window->window));
	}

	/* Set search window to fullscreen or unfullscreen */
	if (app_data->search_window != NULL) {
		if (fullscreen) {
			gtk_window_fullscreen(GTK_WINDOW(app_data->search_window));
		} else {
			gtk_window_unfullscreen(GTK_WINDOW(app_data->search_window));
		}
	}

	/* Focus again on the active note */
	gtk_window_present(active_window);
}

void
ui_helper_remove_portrait_support(GtkWindow *window)
{
	gtk_widget_realize(GTK_WIDGET(window));

	guint32 no = {0};

	gdk_property_change(GTK_WIDGET(window)->window,
			gdk_atom_intern_static_string("_HILDON_PORTRAIT_MODE_SUPPORT"),
			gdk_x11_xatom_to_atom(XA_CARDINAL), 32,
			GDK_PROP_MODE_REPLACE, (gpointer)&no, 1);
}
