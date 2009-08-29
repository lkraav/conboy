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

#include "note.h"
#include "interface.h"

gboolean
on_window_delete		   (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data);

void
on_new_button_clicked					(GtkAction		*action,
										 gpointer		 user_data);

void
on_quit_button_clicked                 (GtkAction       *action,
                                        gpointer         user_data);

void
on_settings_button_clicked             (GtkAction       *action, 
		                                gpointer         user_data);

void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_load_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_link_button_clicked				   (GtkAction		*action,
										gpointer		 user_data);

void
on_format_button_clicked               (GtkAction       *action,
										gpointer		 user_data);

void
on_bullets_button_clicked              (GtkAction       *action,
		                                gpointer         user_data);

void
on_delete_button_clicked			   (GtkAction		*action,
										gpointer		 user_data);

void
on_notes_button_clicked				   (GtkAction		*action,
										gpointer		 user_data);

void
on_smaller_button_clicked			   (GtkAction		*action,
										gpointer		 user_data);

void
on_style_button_clicked                (GtkAction       *action,
		                                gpointer         user_data);

void
on_font_size_radio_group_changed       (GtkRadioAction  *action,
										GtkRadioAction  *current,
		                                gpointer         user_data);

void
on_bigger_button_clicked				(GtkAction		*action,
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

void
on_text_buffer_delete_range					(GtkTextBuffer *buffer,
											 GtkTextIter   *start_iter,
											 GtkTextIter   *end_iter,
											 gpointer		user_data);

void
on_inc_indent_button_clicked			   (GtkAction		*action,
											gpointer		 user_data);

void
on_dec_indent_button_clicked			   (GtkAction		*action,
											gpointer		 user_data);

void
on_bold_action_activated		(GtkAction	*action,
								 gpointer	 user_data);


void on_find_button_clicked(GtkAction *action, gpointer user_data);
void on_find_bar_search(GtkWidget *widget, UserInterface *ui);
void on_find_bar_close(GtkWidget *widget, UserInterface *ui);




#endif /* CALLBACKS_H */
