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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-banner.h>
#include <libhildondesktop/hildon-thumb-menu-item.h>
#include <glib/gstdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>


#include "callbacks.h"

#include "metadata.h"
#include "interface.h"

#include "note.h"
#include "serializer.h"
#include "deserializer.h"

static void change_format(GtkTextBuffer *buffer, const gchar* tag_name, GtkWidget *widget, GtkToggleToolButton *button, GtkCheckMenuItem *item, Note *note)
{	
	UserInterface *ui = note->ui;
	GtkTextIter start_iter, end_iter;
	gboolean activate;
	
	if (GTK_IS_TOGGLE_TOOL_BUTTON(widget)) {
		activate = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget));
	} else if (GTK_IS_CHECK_MENU_ITEM(widget)) {
		activate = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	} else {
		g_printerr("ERROR: Widget must be a GtkToogleToolButton or GtkCheckMenuItem.\n");
		activate = FALSE;
	}
	
	/* Set state of buttons and menu items */
	if (widget == GTK_WIDGET(button) || button == NULL) {
		g_signal_handlers_block_matched(item, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);	
		gtk_check_menu_item_set_active(item, activate);
		g_signal_handlers_unblock_matched(item, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);
	} else if (widget == GTK_WIDGET(item) || item == NULL) {
		g_signal_handlers_block_matched(button, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);	
		gtk_toggle_tool_button_set_active(button, activate);
		g_signal_handlers_unblock_matched(button, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, ui);
	} else {
		g_printerr("ERROR: change_format(): GtkToggleToolButton or GtkCheckMenuItem must be equal to GtkWidget.\n");
	}
	
	if (activate) {
		/* The button just became active, so we should enable the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			gtk_text_buffer_apply_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
			/* Manually set the buffer to modified, because applying tags, doesn't do this automatically */
			gtk_text_buffer_set_modified(buffer, TRUE);	
		} else {
			/* Nothing is selected, so this style should start from here on */
			GtkTextTag *tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
			note_add_active_tag(note, tag);
		}
	} else {
		/* The button just became deactive, so we should remove the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			gtk_text_buffer_remove_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
			/* Manually set the buffer to modified, because removing tags, doesn't do this automatically */
			gtk_text_buffer_set_modified(buffer, TRUE);	
		} else {
			/* Nothing is selected, so this style should stop from here on */
			GtkTextTag *tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
			note_remove_active_tag(note, tag);
		}
	}
	
	
}

gboolean
on_window_close_button_clicked		   (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data)
{
	Note *note = (Note*)user_data;	
	note_save(note);
	note_close_window(note);
	return TRUE; /* True to stop other handler from being invoked */
}

void on_quit_button_clicked(GtkButton *button, gpointer user_data)
{	
	Note *note;
	GList *open_notes = get_app_data()->open_notes;
	
	while (open_notes != NULL) {
		note = (Note*)open_notes->data;
		note_save(note);
		note_close_window(note);
		open_notes = g_list_next(open_notes);
	}
	
	gtk_main_quit();
}

void
on_new_button_clicked					(GtkWidget		*widget,
										 gpointer		 user_data)
{
	Note *note = note_create_new();
	note_show(note);
}

void
on_bold_button_clicked				   (GtkWidget		*widget,
										gpointer		 user_data)
{	
	Note *note = ((Note*)user_data);
	UserInterface *ui = note->ui;
	change_format(ui->buffer, "bold", widget, ui->button_bold, ui->menu_bold, note);
}

void
on_italic_button_clicked			   (GtkWidget		*widget,
										gpointer		 user_data)
{
	Note *note = ((Note*)user_data);
	UserInterface *ui = note->ui;
	change_format(ui->buffer, "italic", widget, ui->button_italic, ui->menu_italic, note);
}

void
on_strike_button_clicked			   (GtkWidget		*widget,
										gpointer		 user_data)
{
	Note *note = ((Note*)user_data);
	UserInterface *ui = note->ui;
	change_format(ui->buffer, "strikethrough", widget, ui->button_strike, ui->menu_strike, note);
}

void
on_fixed_button_clicked					(GtkWidget		*widget,
										 gpointer		 user_data)
{
	Note *note = ((Note*)user_data);
	UserInterface *ui = note->ui;
	change_format(ui->buffer, "monospace", widget, NULL, ui->menu_fixed, note);
}

void
on_highlight_button_clicked			   (GtkWidget		*widget,
										gpointer		 user_data)
{
	Note *note = ((Note*)user_data);
	UserInterface *ui = note->ui;
	change_format(ui->buffer, "highlight", widget, ui->button_highlight, ui->menu_highlight, note);
}

static void
add_bullets(GtkTextBuffer *buffer, gint start_line, gint end_line)
{
	gint i = 0;
	GtkTextIter start_iter, end_iter;
	
	/* For each selected line */
	for (i = start_line; i <= end_line; i++) {
		
		/* Insert bullet character */
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);
		gtk_text_buffer_insert(buffer, &start_iter, BULLET, -1);
		
		/* Remove existing tags from the bullet and add the <depth> tag */
		gtk_text_buffer_get_iter_at_line_offset(buffer, &start_iter, i, 0);
		gtk_text_buffer_get_iter_at_line_offset(buffer, &end_iter, i, 2);
		gtk_text_buffer_remove_all_tags(buffer, &start_iter, &end_iter);
		gtk_text_buffer_apply_tag_by_name(buffer, "depth:1", &start_iter, &end_iter);
		
		/* Surround line (starting after BULLET) with "list-item" tags */
		gtk_text_buffer_get_iter_at_line_offset(buffer, &start_iter, i, 2); /* Jump bullet */
		if (i == end_line) {
			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i);
			gtk_text_iter_forward_to_line_end(&end_iter);
		} else {
			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i + 1);
		}
		gtk_text_buffer_apply_tag_by_name(buffer, "list-item", &start_iter, &end_iter);
	}
	
	/* Surround everything it with "list" tags */
	gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);
	gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
	gtk_text_iter_forward_to_line_end(&end_iter);
	gtk_text_buffer_apply_tag_by_name(buffer, "list", &start_iter, &end_iter);
}

static void
remove_bullets(GtkTextBuffer *buffer, GtkTextIter *start_iter, GtkTextIter *end_iter)
{
	gint i = 0;
	gint start_line = gtk_text_iter_get_line(start_iter);
	gint end_line = gtk_text_iter_get_line(end_iter);
	
	/* Remove tags */
	gtk_text_buffer_get_iter_at_line(buffer, start_iter, start_line);
	gtk_text_buffer_get_iter_at_line(buffer, end_iter, end_line);
	gtk_text_iter_forward_to_line_end(end_iter);
	
	gtk_text_buffer_remove_tag_by_name(buffer, "depth:1", start_iter, end_iter);
	gtk_text_buffer_remove_tag_by_name(buffer, "list-item", start_iter, end_iter);
	gtk_text_buffer_remove_tag_by_name(buffer, "list", start_iter, end_iter);
		
	/* Remove bullets */
	for (i = start_line; i <= end_line; i++) {
		gtk_text_buffer_get_iter_at_line(buffer, start_iter, i);
		gtk_text_buffer_get_iter_at_line(buffer, end_iter, i);
		gtk_text_iter_forward_chars(end_iter, 2);
		gtk_text_buffer_delete(buffer, start_iter, end_iter);
	}
}

void
on_bullets_button_clicked				(GtkWidget		*widget,
										 gpointer		 user_data)
{
	Note *note = (Note*)user_data;
	UserInterface *ui = note->ui;
	GtkTextBuffer *buffer = ui->buffer;
	GtkToggleToolButton *button = ui->button_bullets;
	GtkCheckMenuItem *item = ui->menu_bullets;
	GtkTextIter start_iter, end_iter;
	gboolean activate;
	
	g_printerr("__________ on_bullets_button_clicked() \n");
	
	/* TODO: The first two ifs are the same as in change_format() */
	if (GTK_IS_TOGGLE_TOOL_BUTTON(widget)) {
		activate = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget));
	} else if (GTK_IS_CHECK_MENU_ITEM(widget)) {
		activate = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	} else {
		g_printerr("ERROR: Wiget must be a GtkToggleToolButton or a GtkCheckMenuItem.\n");
		activate = FALSE;
	}
	
	g_signal_handlers_block_matched(button, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
	g_signal_handlers_block_matched(item, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
	
	/* Set state of buttons and menu items */
	if (widget == GTK_WIDGET(button)) {
		gtk_check_menu_item_set_active(item, activate);
	} else if (widget == GTK_WIDGET(item)) {
		gtk_toggle_tool_button_set_active(button, activate);
	} else {
		g_printerr("ERROR: change_format(): GtkToggleToolButton or GtkCheckMenuItem must be equal to GtkWidget.\n");
	}
	
	
	
	if (activate) {
		/* The button just became active, so we should enable the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			add_bullets(buffer, gtk_text_iter_get_line(&start_iter), gtk_text_iter_get_line(&end_iter));
			
		} else {
			/* Nothing is selected */
			GtkTextTag *tag;
			int line;
			gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
			line = gtk_text_iter_get_line(&start_iter);
			add_bullets(buffer, line, line);
			
			tag = gtk_text_tag_table_lookup(buffer->tag_table, "list-item");
			note_add_active_tag(note, tag);
			tag = gtk_text_tag_table_lookup(buffer->tag_table, "list");
			note_add_active_tag(note, tag);
		}
	} else {
		/* The button just became deactive, so we should remove the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
			remove_bullets(buffer, &start_iter, &end_iter);
			
		} else {
			/* Nothing is selected */
			GtkTextTag *tag;
			gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_get_iter_at_mark(buffer, &end_iter, gtk_text_buffer_get_insert(buffer));		
			remove_bullets(buffer, &start_iter, &end_iter);
			
			note_remove_active_tag_by_name(note, "list-item");
			note_remove_active_tag_by_name(note, "list");
		}
	}
	
	g_signal_handlers_unblock_matched(button, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
	g_signal_handlers_unblock_matched(item, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, note);
}

void
on_link_button_clicked				   (GtkButton		*button,
										gpointer		 user_data) 
{ 	
	Note *note = (Note*)user_data;
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter start, end;
	const gchar *text;
	
	if (!gtk_text_buffer_get_has_selection(buffer)) {
		return;
	}
	
	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	text = gtk_text_iter_get_text(&start, &end);
	gtk_text_buffer_apply_tag_by_name(buffer, "link:internal", &start, &end);
	
	note_show_by_title(text);
}


/*
static void
menu_position ( GtkMenu *menu ,
                gint *x,
                gint *y,
                gboolean *push_in ,
                gpointer *thw)
{
    g_return_if_fail (thw -> button );
    * push_in = TRUE;
    *x = thw ->button -> allocation .x + thw ->button -> allocation . width ;
    *y = thw ->button -> allocation .y;
}
*/


void
on_notes_button_clicked				   (GtkButton		*button,
										gpointer		 user_data) {
	
	GtkWidget *menu = gtk_menu_new ();
	AppData *app_data = get_app_data();
	GList *notes;
	
	if (app_data->all_notes == NULL) {
		return;
	}
	
	/* TODO: This might become a perf. problem with many notes */
	/* Then we should take care of sorting when inserting and updating elements */
	app_data->all_notes = sort_note_list_by_change_date(app_data->all_notes);
	
	notes = app_data->all_notes;
	while(notes != NULL) {
		GtkWidget *menu_item;
		GtkWidget *image_small, *image_large;
		Note *note = notes->data;
		/* TODO: When starting from Eclipse, use local paths, not from /usr/share/ */
		menu_item = hildon_thumb_menu_item_new_with_labels(note->title, note->title, "Open Note...");
		image_small = gtk_image_new_from_icon_name("conboy", GTK_ICON_SIZE_SMALL_TOOLBAR);
		image_large = gtk_image_new_from_icon_name("conboy", GTK_ICON_SIZE_DIALOG);
		hildon_thumb_menu_item_set_images(HILDON_THUMB_MENU_ITEM(menu_item), image_small, image_large);
		
		g_signal_connect(G_OBJECT(menu_item), "activate",
		            	 G_CALLBACK(on_notes_menu_item_activated),
		            	 note);
		gtk_menu_append(menu, menu_item);
		notes = notes->next;
	}
	
	hildon_menu_set_thumb_mode(GTK_MENU(menu), TRUE);
	
	gtk_widget_show_all(menu);

	gtk_menu_popup ( GTK_MENU (menu),
	                 NULL,
	                 NULL,
	                 NULL,
	                 NULL,
	                 0,
	                 gtk_get_current_event_time());
	
}

void
on_notes_menu_item_activated		(GtkMenuItem *menuitem,
									 gpointer     user_data)
{
	Note *note = (Note*)user_data;
	note_show(note);
}

/* TODO: The signal "mark-set" is emitted 4 times when clicking into the text. While selecting
 * it's emitted continuesly. */
void
on_textview_cursor_moved			   (GtkTextBuffer	*buffer,
										GtkTextIter		*location,
										GtkTextMark		*mark,
										gpointer		 user_data)
{
	Note *note = (Note*)user_data;
	UserInterface *ui = note->ui;
	GSList *tags;
	GSList *tmp_tags;
	GtkTextTag *tag;
	const gchar *mark_name;
	
	/* We only do something if the "insert" mark changed. */
	mark_name = gtk_text_mark_get_name(mark);
	if ((mark_name == NULL) || (strcmp(mark_name, "insert") != 0)) {
		return;
	}
	

	/* Clean the list of active tags */
	g_slist_free(note->active_tags);
	
	/* Add tags at this location */
	note->active_tags = gtk_text_iter_get_tags(location);
	
	/* Go the beginning of line and check if there is a bullet.
	 * If yes, add list-item and list tags */
	gtk_text_iter_set_line_offset(location, 0);
	tmp_tags = gtk_text_iter_get_tags(location);
	if (tmp_tags != NULL) {
		if (strncmp(GTK_TEXT_TAG(tmp_tags->data)->name, "depth", 5) == 0) {
			note_add_active_tag_by_name(note, "list-item");
			note_add_active_tag_by_name(note, "list");
		}
	}
	g_slist_free(tmp_tags);
	
	
	/* Copy pointer for iteration */
	tags = note->active_tags;
	
	
	/* Blocking signals here because the ..set_active() method makes the buttons
	 * emit the clicked signal. And because of this the formatting changes.
	 */
	g_signal_handlers_block_by_func(ui->button_bold, on_bold_button_clicked, note);
	g_signal_handlers_block_by_func(ui->button_italic, on_italic_button_clicked, note);
	g_signal_handlers_block_by_func(ui->button_strike, on_strike_button_clicked, note);
	g_signal_handlers_block_by_func(ui->button_highlight, on_highlight_button_clicked, note);
	g_signal_handlers_block_by_func(ui->button_bullets, on_bullets_button_clicked, note);
	
	
	g_signal_handlers_block_by_func(ui->menu_bold, on_bold_button_clicked, note);
	g_signal_handlers_block_by_func(ui->menu_italic, on_italic_button_clicked, note);
	g_signal_handlers_block_by_func(ui->menu_strike, on_strike_button_clicked, note);
	g_signal_handlers_block_by_func(ui->menu_highlight, on_highlight_button_clicked, note);
	g_signal_handlers_block_by_func(ui->menu_fixed, on_fixed_button_clicked, note);
	g_signal_handlers_block_by_func(ui->menu_bullets, on_bullets_button_clicked, note);
	
	/* TODO: This can be optimized: Note disable all and then enable selected, but determine state and then
	 * set the state. */
	gtk_toggle_tool_button_set_active(ui->button_bold, FALSE);
	gtk_toggle_tool_button_set_active(ui->button_italic, FALSE);
	gtk_toggle_tool_button_set_active(ui->button_strike, FALSE);
	gtk_toggle_tool_button_set_active(ui->button_highlight, FALSE);
	gtk_toggle_tool_button_set_active(ui->button_bullets, FALSE);
	
	gtk_check_menu_item_set_active(ui->menu_bold, FALSE);
	gtk_check_menu_item_set_active(ui->menu_italic, FALSE);
	gtk_check_menu_item_set_active(ui->menu_strike, FALSE);
	gtk_check_menu_item_set_active(ui->menu_highlight, FALSE);
	gtk_check_menu_item_set_active(ui->menu_fixed, FALSE);
	gtk_check_menu_item_set_active(ui->menu_bullets, FALSE);
	
	while (tags != NULL) {
		tag = GTK_TEXT_TAG(tags->data);
		if (strcmp(tag->name, "bold") == 0) {
			gtk_toggle_tool_button_set_active(ui->button_bold, TRUE);
			gtk_check_menu_item_set_active(ui->menu_bold, TRUE);
		} else if (strcmp(tag->name, "italic") == 0) {
			gtk_toggle_tool_button_set_active(ui->button_italic, TRUE);
			gtk_check_menu_item_set_active(ui->menu_italic, TRUE);
		} else if (strcmp(tag->name, "strikethrough") == 0) {
			gtk_toggle_tool_button_set_active(ui->button_strike, TRUE);
			gtk_check_menu_item_set_active(ui->menu_strike, TRUE);
		} else if (strcmp(tag->name, "highlight") == 0) {
			gtk_toggle_tool_button_set_active(ui->button_highlight, TRUE);
			gtk_check_menu_item_set_active(ui->menu_highlight, TRUE);
		} else if (strcmp(tag->name, "monospace") == 0) {
			gtk_check_menu_item_set_active(ui->menu_fixed, TRUE);
		} else if (strncmp(tag->name, "list-item", 9) == 0) {
			gtk_toggle_tool_button_set_active(ui->button_bullets, TRUE);
			gtk_check_menu_item_set_active(ui->menu_bullets, TRUE);
		}
		tags = tags->next;
	}
	
	/*g_slist_free(tags);*/
	
	/* unblock signals */
	g_signal_handlers_unblock_by_func(ui->button_bold, on_bold_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->button_italic, on_italic_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->button_strike, on_strike_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->button_highlight, on_highlight_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->button_bullets, on_bullets_button_clicked, note);
	
	g_signal_handlers_unblock_by_func(ui->menu_bold, on_bold_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->menu_italic, on_italic_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->menu_strike, on_strike_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->menu_highlight, on_highlight_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->menu_fixed, on_fixed_button_clicked, note);
	g_signal_handlers_unblock_by_func(ui->menu_bullets, on_bullets_button_clicked, note);
}

void
on_textview_insert_at_cursor		   (GtkTextView *text_view,
        								gchar       *string,
        								gpointer     user_data)
{
	g_printerr("Insert: %s", string);
}

void
on_textbuffer_changed				(GtkTextBuffer *buffer,
									 gpointer 		user_data)
{
	GtkTextMark *mark;
	GtkTextIter current;
	
	/* If the cursor is in the first logical line, then format
	 * the first line and change the window title.
	 */
	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &current, mark);
	
	if (gtk_text_iter_get_line(&current) != 0) {
		return;
	}
	
	note_format_title(buffer);
	note_set_window_title_from_buffer(GTK_WINDOW(user_data), buffer);
}

gboolean
on_link_internal_tag_event				(GtkTextTag  *tag,
										 GObject     *object,
										 GdkEvent    *event,
										 GtkTextIter *iter,
										 gpointer     user_data)
{
	GdkEventType type = event->type;
	GtkTextIter *start;
	gchar *link_text;
	
	if (type == GDK_BUTTON_RELEASE) {
		if (((GdkEventButton*)event)->button == 1) {
			
			g_printerr("Open Link \n");
			
			if (!gtk_text_iter_begins_tag(iter, tag)) {
				gtk_text_iter_backward_to_tag_toggle(iter, tag);
			}
			
			start = gtk_text_iter_copy(iter);
			
			if (!gtk_text_iter_ends_tag(iter, tag)) {
				gtk_text_iter_forward_to_tag_toggle(iter, tag);
			}
			
			link_text = gtk_text_iter_get_text(start, iter);
			
			g_printerr("Link: >%s< \n", link_text);
			note_show_by_title(link_text);
		}	
	}
		
	return FALSE;
}

void
on_delete_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	/* Popup Dialog ask for sure
	 * Close window
	 * Lower window count
	 * Change open_windows in app_data
	 * Delete file
	 * Free resources
	 */
	Note *note = (Note*)user_data;
	const gchar *message = "<b>Really delete this note?</b>\n\n If you delete a note it is permanently lost.";
	GtkWidget *dialog;
	gint response;
	AppData *app_data;
	
	g_printerr("on_delete_button_clicked() called\n");
	
	dialog = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(note->ui->window),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			message);
	
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	
	if (response != GTK_RESPONSE_YES) {
		return;
	}
	
	/* Block this signal, otherwise the destroy event of the window will trigger saving of the note */
	g_signal_handlers_block_by_func(note->ui->window, on_window_close_button_clicked, note);
	note_close_window(note);
	/* Don't unblock the signal, because the window does not exist anymore */
	
	/* Delete file */
	if (g_unlink(note->filename) == -1) {
		g_printerr("ERROR: The file %s could not be deleted \n", note->filename);
	}
	
	app_data = get_app_data();
	/* TODO: Free note */
	app_data->all_notes = g_list_remove(app_data->all_notes, note);
}

static
gboolean note_save_callback(Note *note)
{
	/* Test, to see if the window is still open */
	if (note != NULL && GTK_IS_TEXT_BUFFER(note->ui->buffer)) {
		note_save(note);
	}
	
	/* Return FALSE to make the Timer stop */
	return FALSE;
}

void
on_text_buffer_modified_changed			(GtkTextBuffer *buffer,
										 gpointer		user_data)
{
	Note *note = (Note*)user_data;
	
	if (!gtk_text_buffer_get_modified(buffer)) {
		g_printerr("Buffer changed from dirty to saved \n");
		return;
	}
	
	g_printerr("Buffer is dirty. Saving in 10 seconds\n");
	
	/* Save 10 seconds after the buffer got dirty */
	g_timeout_add(4000, (GSourceFunc)note_save_callback, note);
}

static
void change_font_size_by(gint size)
{
	AppData *app_data = get_app_data();
	PangoFontDescription *font;
	Note *note;
	GList *note_list;
	
	if (size == 0) {
		return;
	}
	
	if (size > 0) {
		if (app_data->font_size + size <= 65000) {
			app_data->font_size += size;
		} else {
			return;
		}
	} else {
		if (app_data->font_size + size >= 5000) {
			app_data->font_size += size;
		} else {
			return;
		}
	}
	
	gconf_client_set_int(app_data->client, "/apps/maemo/conboy/font_size", app_data->font_size, NULL);
	
	font = pango_font_description_new();
	pango_font_description_set_size(font, app_data->font_size);
	
	note_list = app_data->open_notes;
	while (note_list != NULL) {
		note = (Note*)note_list->data;
		gtk_widget_modify_font(GTK_WIDGET(note->ui->view), font);
		note_list = g_list_next(note_list);
	}
}

void
on_smaller_button_clicked			   (GtkButton		*button,
										gpointer		 user_data)
{
	change_font_size_by(-5000);
}

void
on_bigger_button_clicked				(GtkButton		*button,
										gpointer		 user_data)
{
	change_font_size_by(+5000);
}

gboolean on_hardware_key_pressed	(GtkWidget			*widget,
									 GdkEventKey		*event,
									 gpointer           user_data)
{
	Note *note = (Note*)user_data;
	GtkWidget *window = GTK_WIDGET(note->ui->window);
	
	switch (event->keyval) {
	case HILDON_HARDKEY_INCREASE:
		hildon_banner_show_information(window, NULL, "Zooming in");
		on_bigger_button_clicked(NULL, NULL);
		return TRUE;
	
	case HILDON_HARDKEY_DECREASE:
		hildon_banner_show_information(window, NULL, "Zooming out");
		on_smaller_button_clicked(NULL, NULL);
		return TRUE;
		
	case HILDON_HARDKEY_ESC:
		on_window_close_button_clicked(GTK_OBJECT(window), NULL, note);
		return TRUE;
	
	case HILDON_HARDKEY_FULLSCREEN:
		/* TODO: Implement full screen. */
		/* Should this be on application level: If fullscreen, then all windows fullscreen
		 * Or on window leve: Toggle fullscreen on/off for every window
		 * Or can only be the window which is active be fullscreen? */
		/*
		gtk_window_fullscreen(note->window);
		gtk_window_unfullscreen(note->window);
		*/
		return TRUE;
	}
	
	return FALSE;
}

void
on_textview_tap_and_hold(GtkWidget *widget, gpointer user_data)
{
	/* We emit the popup-menu signal, this will make the
	 * default context menu of the treeview pop up. */
	gboolean bool;
	g_signal_emit_by_name(widget, "popup-menu", &bool);
}

/*
void print_tags(Note *note) {
	GSList *tags = note->active_tags;
	while (tags != NULL) {
		g_printerr("TAG: %s \n", GTK_TEXT_TAG(tags->data)->name);
		tags = tags->next;
	}
}
*/

static gboolean add_new_line(Note *note)
{
	GtkTextBuffer *buffer = note->ui->buffer;
	GtkTextIter iter;
	GSList *tags;
	gint line;
	
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
	gtk_text_iter_set_line_offset(&iter, 0);
	tags = gtk_text_iter_get_tags(&iter);
	if (tags == NULL) {
		g_printerr("Not a bullet line \n");
		return FALSE;
	}
	
	/* If we are on a bullet line and this line is not empty, start a new bullet line */
	if (strncmp(((GtkTextTag*)tags->data)->name, "depth", 5) == 0) {

		gtk_text_iter_forward_to_line_end(&iter);		
		if (gtk_text_iter_get_line_offset(&iter) > 2) {
			GSList *tmp;
			
			/* Remove all tags but <list> from active tags */
			tmp = g_slist_copy(note->active_tags);
			g_slist_free(note->active_tags);
			note->active_tags = NULL;
			note_add_active_tag_by_name(note, "list");
			
			/* Insert newline and bullet */
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_insert(buffer, &iter, "\n", -1);
			line = gtk_text_iter_get_line(&iter);
			add_bullets(buffer, line, line);
			
			/* Add all tags back to active tags */
			note->active_tags = tmp;
			
			return TRUE;
			
		} else {
			/* Remove bullet and insert newline */
			GtkTextTag *tag;
			GtkTextIter *start = gtk_text_iter_copy(&iter);
			
			/* Disable list and list-item tags */
			tag = gtk_text_tag_table_lookup(buffer->tag_table, "list-item");
			note_remove_active_tag(note, tag);
			tag = gtk_text_tag_table_lookup(buffer->tag_table, "list");
			note_remove_active_tag(note, tag);
			
			/* Delete the bullet and the last newline */
			gtk_text_iter_set_line_offset(start, 0);
			gtk_text_iter_backward_char(start);
			gtk_text_buffer_remove_all_tags(buffer, start, &iter);
			gtk_text_buffer_delete(buffer, start, &iter);
			
			gtk_text_buffer_insert(buffer, &iter, "\n", -1);
			
			/* Disable the bullet button */
			on_textview_cursor_moved(buffer, &iter, gtk_text_buffer_get_insert(buffer) ,note);
			
			return TRUE;
		}
	}
	return FALSE;
}

gboolean
on_text_view_key_pressed                      (GtkWidget   *widget,
                                               GdkEventKey *event,
                                               gpointer     user_data)
{
	Note *note = (Note*)user_data;
	
	switch (event->keyval) {
		case GDK_Return:
		case GDK_KP_Enter:
			return add_new_line(note);
		default:
			return FALSE;
	}
	
}

void
on_text_buffer_insert_text					(GtkTextBuffer *buffer,
											 GtkTextIter   *end_iter,
											 gchar		   *text,
											 gint			len,
											 gpointer		user_data)
{
	if (len != 1) {
		return;
	} else {
		GSList *active_tags = ((Note*)user_data)->active_tags;
		GtkTextIter *start_iter = gtk_text_iter_copy(end_iter);
		gtk_text_iter_backward_char(start_iter);
		
		gtk_text_buffer_remove_all_tags(buffer, start_iter, end_iter);
		
		while (active_tags != NULL && active_tags->data != NULL) {
			g_printerr("Apply tag: %s \n", ((GtkTextTag*)active_tags->data)->name);
			gtk_text_buffer_apply_tag(buffer, active_tags->data, start_iter, end_iter);
			active_tags = active_tags->next;
		}
	}
}

/*
 * TODO: Put in common file. And also use in deserializer. 
 */
GtkTextTag* buffer_get_depth_tag(GtkTextBuffer *buffer, gint depth)
{
	GtkTextTag *tag;
	gchar depth_str[5] = {0};
	gchar *tag_name;
	
	if (depth < 1) {
		g_printerr("ERROR: buffer_get_depth_tag(): depth must be at least 1. Not: %i \n", depth);
	}
	
	g_sprintf(depth_str, "%i", depth);
	tag_name = g_strconcat("depth", ":", depth_str, NULL);
	
	tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
	
	if (tag == NULL) {
		tag = gtk_text_buffer_create_tag(buffer, tag_name, "indent", -20, "left-margin", depth * 25, NULL);
		/* Set the priority of the "list" tag to the maximum. It needs a higher priority then the newly created "list-item" tag */
		/* TODO: I think the next line is not needed anymore. Test without. */
		gtk_text_tag_set_priority(gtk_text_tag_table_lookup(buffer->tag_table, "list"), gtk_text_tag_table_get_size(buffer->tag_table) - 1);
	}
	
	g_free(tag_name);
	
	return tag;
}

gboolean tag_is_depth_tag(GtkTextTag *tag)
{
	if (tag == NULL) {
		return FALSE;
	}
	return (strncmp(tag->name, "depth", 5) == 0);
}

gint tag_get_depth(GtkTextTag *tag)
{
	if (tag_is_depth_tag(tag)) {
		char **strings = g_strsplit(tag->name, ":", 2);
		int depth = atoi(strings[1]);
		g_strfreev(strings);
		return depth;
	}
	return 0;
}

GtkTextTag* iter_get_depth_tag(GtkTextIter* iter)
{
	GSList *tags = gtk_text_iter_get_tags(iter);
	while (tags != NULL) {
		if (tag_is_depth_tag(tags->data)) {
			return tags->data;
		}
	}
	return NULL;
}

void increase_indent(GtkTextBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;
	for (i = start_line; i <= end_line; i++) {
		
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);
		
		old_tag = iter_get_depth_tag(&start_iter);
		
		if (old_tag != NULL) {
			gint depth = tag_get_depth(old_tag);
			depth++;
			new_tag = buffer_get_depth_tag(buffer, depth);
			
			g_printerr("Increasing indent \n");
			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i);
			gtk_text_iter_set_line_offset(&end_iter, 2);
			/* Remove old tag and apply new tag */
			gtk_text_buffer_remove_tag(buffer, old_tag, &start_iter, &end_iter);
			gtk_text_buffer_apply_tag(buffer, new_tag, &start_iter, &end_iter);
		}
	}
}

/* TODO: This is almost 100% copy&paste from increase_indent */
void decrease_indent(GtkTextBuffer *buffer, gint start_line, gint end_line)
{
	GtkTextIter start_iter, end_iter;
	GtkTextTag *old_tag;
	GtkTextTag *new_tag;
	gint i;
	for (i = start_line; i <= end_line; i++) {
		
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);
		
		old_tag = iter_get_depth_tag(&start_iter);
		
		if (old_tag != NULL) {
			gint depth = tag_get_depth(old_tag);
			if (depth <= 1) {
				return;
			}
			depth--;
			new_tag = buffer_get_depth_tag(buffer, depth);
			
			g_printerr("Increasing indent \n");
			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i);
			gtk_text_iter_set_line_offset(&end_iter, 2);
			/* Remove old tag and apply new tag */
			gtk_text_buffer_remove_tag(buffer, old_tag, &start_iter, &end_iter);
			gtk_text_buffer_apply_tag(buffer, new_tag, &start_iter, &end_iter);
		}
	}
}

void
on_inc_indent_button_clicked			   (GtkButton		*button,
											gpointer		 user_data)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
	GtkTextIter start_iter, end_iter;
	gint start_line, end_line;
	
	if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter)) {
		start_line = gtk_text_iter_get_line(&start_iter);
		end_line = gtk_text_iter_get_line(&end_iter);
		increase_indent(buffer, start_line, end_line);
		
	} else {
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
		start_line = gtk_text_iter_get_line(&start_iter);
		increase_indent(buffer, start_line, start_line);
	}
	
	gtk_text_buffer_set_modified(buffer, TRUE);
}

/* TODO: Increase and decrease too similar */
void
on_dec_indent_button_clicked			   (GtkButton		*button,
											gpointer		 user_data)
{
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
	GtkTextIter start_iter, end_iter;
	gint start_line, end_line;
	
	if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter)) {
		start_line = gtk_text_iter_get_line(&start_iter);
		end_line = gtk_text_iter_get_line(&end_iter);
		decrease_indent(buffer, start_line, end_line);
		
	} else {
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
		start_line = gtk_text_iter_get_line(&start_iter);
		decrease_indent(buffer, start_line, start_line);
	}
	
	gtk_text_buffer_set_modified(buffer, TRUE);
}




