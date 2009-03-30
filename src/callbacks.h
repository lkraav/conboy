#ifndef CALLBACKS_H
#define CALLBACKS_H

gboolean
on_window_close_button_clicked		   (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data);

void
on_quit_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_load_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_bold_button_clicked				   (GtkButton		*button,
										gpointer		 user_data);

void
on_italic_button_clicked			   (GtkButton		*button,
										gpointer		 user_data);

void
on_strike_button_clicked			   (GtkButton		*button,
										gpointer		 user_data);

void
on_link_button_clicked				   (GtkButton		*button,
										gpointer		 user_data);

void
on_delete_button_clicked			   (GtkButton		*button,
										gpointer		 user_data);

void
on_notes_button_clicked				   (GtkButton		*button,
										gpointer		 user_data);

void
on_smaller_button_clicked			   (GtkButton		*button,
										gpointer		 user_data);

void
on_bigger_button_clicked				(GtkButton		*button,
										gpointer		 user_data);

void
on_notes_menu_item_activated		(GtkMenuItem *menuitem,
									 gpointer     user_data);

void
on_textview_cursor_moved			   (GtkTextBuffer	*buffer,
										GtkTextIter		*location,
										GtkTextMark		*mark,
										gpointer		 user_data);

void
on_textview_insert_at_cursor		   (GtkTextView *text_view,
        								gchar       *string,
        								gpointer     user_data);

void
on_textbuffer_changed				   (GtkTextBuffer *buffer,
									 	gpointer 		user_data);
        								

void
update_title						   (GtkTextBuffer *buffer,
									 	GtkWindow *window);

gboolean
on_link_internal_tag_event				(GtkTextTag  *tag,
										 GObject     *object,
										 GdkEvent    *event,
										 GtkTextIter *iter,
										 gpointer     user_data);

void
on_text_buffer_modified_changed			(GtkTextBuffer *buffer,
										 gpointer		user_data);

gboolean
on_hardware_key_pressed					(GtkWidget			*widget,
									 	 GdkEventKey		*event,
									 	 gpointer           *user_data);

#endif /* CALLBACKS_H */
