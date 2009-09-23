

#include "conboy_check_mark.h"

G_DEFINE_TYPE(ConboyCheckMark, conboy_check_mark, GTK_TYPE_MISC);




static void
conboy_check_mark_class_init(ConboyCheckMarkClass *klass)
{



}


static void
conboy_check_mark_init(ConboyCheckMark *self)
{
	GtkWidget *cell_view = gtk_cell_view_new ();

	/* Make sure that the check box is always shown, no matter the value of gtk-button-images */
	g_signal_connect (cell_view, "notify::visible", G_CALLBACK (gtk_widget_show), NULL);

	/* Create toggle renderer and pack it into the cell view */
	self->renderer_toggle = GTK_CELL_RENDERER_TOGGLE (gtk_cell_renderer_toggle_new ());
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (cell_view),
								GTK_CELL_RENDERER (self->renderer_toggle), FALSE);



}
