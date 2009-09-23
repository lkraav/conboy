
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
