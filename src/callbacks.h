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


#ifndef CALLBACKS_H
#define CALLBACKS_H

gboolean
on_window_close_button_clicked		   (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data);

void
on_new_button_clicked					(GtkWidget		*widget,
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
on_bold_button_clicked				   (GtkWidget		*widget,
										gpointer		 user_data);

void
on_italic_button_clicked			   (GtkWidget		*widget,
										gpointer		 user_data);

void
on_strike_button_clicked			   (GtkWidget		*widget,
										gpointer		 user_data);

void
on_fixed_button_clicked					(GtkWidget		*widget,
										 gpointer		 user_data);

void
on_bullets_button_clicked				(GtkWidget		*widget,
										 gpointer		 user_data);

void
on_highlight_button_clicked			   (GtkWidget		*widget,
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
									 	 gpointer           user_data);

void
on_textview_tap_and_hold				(GtkWidget 			*widget,
										 gpointer 			user_data);

gboolean
on_text_view_key_pressed                (GtkWidget   *widget,
                                         GdkEventKey *event,
                                         gpointer     user_data);

void
on_text_buffer_insert_text					(GtkTextBuffer *buffer,
											 GtkTextIter   *end_iter,
											 gchar		   *text,
											 gint			len,
											 gpointer		user_data);


#endif /* CALLBACKS_H */
