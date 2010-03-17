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

#include "localisation.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <hildon/hildon-program.h>
#ifdef HILDON_HAS_APP_MENU
#include <hildon/hildon-pannable-area.h>
#include <he-about-dialog.h>
#endif
#include <hildon/hildon-banner.h>
#include <hildon/hildon-defines.h>
#include <hildon-mime.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <math.h>
#include <tablet-browser-interface.h>
#ifdef WITH_MODEST
#include <libmodest-dbus-client/libmodest-dbus-client.h>
#endif

#include "metadata.h"
#include "interface.h"
#include "note.h"
#include "search_window.h"
#include "note_linker.h"
#include "app_data.h"
#include "settings.h"
#include "settings_window.h"
#include "conboy_note_buffer.h"
#include "ui_helper.h"
#include "gregex.h"

#include "callbacks.h"

/* Private. TODO: Move to some public file */
GtkTextTag* iter_get_depth_tag(GtkTextIter* iter);
GtkTextTag* buffer_get_depth_tag(GtkTextBuffer *buffer, gint depth);

gboolean __editing_title = FALSE;

static void
check_title(UserInterface *ui)
{
	if (__editing_title) {
		__editing_title = FALSE;
	} else {
		return;
	}

	AppData *app_data = app_data_get();
	ConboyNote *note = ui->note;
	gchar *title = note_extract_title_from_buffer(ui->buffer);

	ConboyNote *existing_note = conboy_note_store_find_by_title(app_data->note_store, title);
	if (existing_note && (existing_note != note)) {
		/* Display message */
		gchar msg[1024];
		g_sprintf(msg, _("<b>Note title taken</b>\n\nA note with the title <b>%s</b> already exists. Please choose another name for this note before continuing."), title);
		ui_helper_show_confirmation_dialog(GTK_WINDOW(ui->window), msg, FALSE);

		/* Select title */
		GtkTextIter start, end;
		gtk_text_buffer_get_start_iter(ui->buffer, &start);
		end = start;
		gtk_text_iter_forward_to_line_end(&end);
		gtk_text_buffer_select_range(ui->buffer, &start, &end);
	}

	g_free(title);
}

static
GtkTextTag* get_depth_tag_at_line(GtkTextBuffer *buffer, gint line_number)
{
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line(buffer, &iter, line_number);
	return iter_get_depth_tag(&iter);
}

static void change_format(UserInterface *ui, GtkToggleAction *action)
{
	gint start_line, end_line, i;
	GtkTextIter selection_start_iter, selection_end_iter;
	GtkTextIter start_iter, end_iter;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(ui->buffer);

	const gchar *tag_name = gtk_action_get_name(GTK_ACTION(action));

	if (gtk_toggle_action_get_active(action)) {
		/* The button just became active, so we should enable the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &selection_start_iter, &selection_end_iter);

			start_line = gtk_text_iter_get_line(&selection_start_iter);
			end_line = gtk_text_iter_get_line(&selection_end_iter);

			/* We go through the selection line by line, so we have a chance to skip over the bullets */
			for (i = start_line; i <= end_line; i++) {

				if (i == start_line) {
					start_iter = selection_start_iter;
				} else {
					gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);
				}

				if (i == end_line) {
					end_iter = selection_end_iter;
				} else {
					gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i + 1);
					/* If we are inside a list, don't tag the newline character at the end of the line */
					if (get_depth_tag_at_line(buffer, i) != NULL) {
						gtk_text_iter_backward_char(&end_iter);
					}
				}

				/* If we are at a bullet, jump over this bullet */
				if (iter_get_depth_tag(&start_iter) != NULL) {
					gtk_text_iter_set_line_offset(&start_iter, 2);
				}

				/* Apply tag */
				gtk_text_buffer_apply_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
			}

			/* Manually set the buffer to modified, because applying tags, doesn't do this automatically */
			gtk_text_buffer_set_modified(buffer, TRUE);
		} else {
			/* Nothing is selected, so this style should start from here on */
			GtkTextTag *tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
			conboy_note_buffer_add_active_tag(CONBOY_NOTE_BUFFER(ui->buffer), tag);
		}
	} else {
		/* The button just became deactive, so we should remove the formatting */
		if (gtk_text_buffer_get_has_selection(buffer)) {
			/* Something is selected */
			gtk_text_buffer_get_selection_bounds(buffer, &selection_start_iter, &selection_end_iter);
			gtk_text_buffer_remove_tag_by_name(buffer, tag_name, &selection_start_iter, &selection_end_iter);
			/* Manually set the buffer to modified, because removing tags, doesn't do this automatically */
			gtk_text_buffer_set_modified(buffer, TRUE);
		} else {
			/* Nothing is selected, so this style should stop from here on */
			GtkTextTag *tag = gtk_text_tag_table_lookup(buffer->tag_table, tag_name);
			conboy_note_buffer_remove_active_tag(CONBOY_NOTE_BUFFER(ui->buffer), tag);
		}
	}


}

void
on_format_button_clicked               (GtkAction       *action,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	change_format(ui, GTK_TOGGLE_ACTION(action));
}

void
on_font_size_radio_group_changed       (GtkRadioAction  *action,
										GtkRadioAction  *current,
		                                gpointer         user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	const gchar *tag_name = gtk_action_get_name(GTK_ACTION(current));

	if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter)) {
		/* Remove all possible size tags */
		gtk_text_buffer_remove_tag_by_name(buffer, "size:small", &start_iter, &end_iter);
		gtk_text_buffer_remove_tag_by_name(buffer, "size:large", &start_iter, &end_iter);
		gtk_text_buffer_remove_tag_by_name(buffer, "size:huge",  &start_iter, &end_iter);

		/* If not normal size, apply one size tag */
		if (strcmp(tag_name, "size:normal") != 0) {
			gtk_text_buffer_apply_tag_by_name(buffer, tag_name, &start_iter, &end_iter);
		}

		/* Set the buffer to modified */
		gtk_text_buffer_set_modified(buffer, TRUE);

	} else {
		/* Remove all possible size tags */
		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "size:small");
		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "size:large");
		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "size:huge");

		/* If not normal size, add one size tag */
		if (strcmp(tag_name, "size:normal") != 0) {
			conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), tag_name);
		}
	}
}

gboolean
on_window_delete		               (GtkObject		*window,
										GdkEvent		*event,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkAdjustment *adj;

	/* Save the current note */
	note_save(ui);

#ifdef HILDON_HAS_APP_MENU
	adj = hildon_pannable_area_get_vadjustment(HILDON_PANNABLE_AREA(ui->scrolled_window));
	/* Take screenshot for faster startup trick */
	/* TODO: Maybe enable later again, now it causes confusion with scrolling / flickering */
	/*hildon_gtk_window_take_screenshot(ui->window, TRUE);*/
#else
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(ui->scrolled_window));
#endif

	/* Save last viewd note and the scroll position */
	settings_save_last_scroll_position(gtk_adjustment_get_value(adj));
	settings_save_last_open_note(ui->note->guid);

	/* Quit */
	gtk_main_quit();

	return TRUE; /* True to stop other handler from being invoked */
}

void on_quit_button_clicked(GtkAction *action, gpointer user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	/*
	GList *open_windows = app_data_get()->open_windows;

	while (open_windows != NULL) {
		ui = (UserInterface*)open_windows->data;
		note_save(ui);
		note_close_window(ui);
		open_windows = g_list_next(open_windows);
	}
	*/
	note_save(ui);

	gtk_main_quit();
}

void on_settings_button_clicked(GtkAction *action, gpointer user_data)
{
	GtkWindow *parent = GTK_WINDOW(user_data);
	settings_window_open(parent);
}

void
on_new_button_clicked					(GtkAction		*action,
										 gpointer		 user_data)
{
	AppData *app_data = app_data_get();
	UserInterface *ui = (UserInterface*)user_data;

	/* Save current note */
	note_save(ui);

	/* Create new note */
	int num = conboy_note_store_get_length(app_data->note_store) + 1;
	gchar title[100];

	g_sprintf(title, _("New Note %i"), num);

	ConboyNote *note = conboy_note_new_with_title(title);
	note_show(note, TRUE, TRUE, TRUE);
}

static void
add_bullets(GtkTextBuffer *buffer, gint start_line, gint end_line, GtkTextTag *depth_tag)
{
	gint i = 0;
	GtkTextIter start_iter, end_iter;
	gint total_lines = gtk_text_buffer_get_line_count(buffer);

	/* For each selected line */
	for (i = start_line; i <= end_line; i++) {

		/* Insert bullet character */
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, i);
		gtk_text_buffer_insert(buffer, &start_iter, get_bullet_by_depth_tag(depth_tag), -1);

		/* Remove existing tags from the bullet and add the <depth> tag */
		gtk_text_buffer_get_iter_at_line_offset(buffer, &start_iter, i, 0);
		gtk_text_buffer_get_iter_at_line_offset(buffer, &end_iter, i, 2);
		gtk_text_buffer_remove_all_tags(buffer, &start_iter, &end_iter);
		gtk_text_buffer_apply_tag(buffer, depth_tag, &start_iter, &end_iter);

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
	/* Check line above and below. If one or both are bullet lines, include the newline chars at the beginning and the end */
	/* This is done, so that there are no gaps in the <list> tag and that it really surrounds the whole list. */

	/* Set start iter TODO: This if statement is not elegant at all.*/
	if (start_line > 0) {
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line - 1);
		if (iter_get_depth_tag(&start_iter) != NULL) {
			gtk_text_iter_forward_to_line_end(&start_iter);
		} else {
			gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);
		}
	} else {
		gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);
	}

	/* Set end iter TODO: This if statement is not elegant at all. */
	if (end_line < total_lines - 1) {
		gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line + 1);
		if (iter_get_depth_tag(&end_iter) == NULL) {
			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
			gtk_text_iter_forward_to_line_end(&end_iter);
		}
	} else {
		gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
		gtk_text_iter_forward_to_line_end(&end_iter);
	}

	/****/
	/*
	gtk_text_buffer_get_iter_at_line(buffer, &start_iter, start_line);
	gtk_text_buffer_get_iter_at_line(buffer, &end_iter, end_line);
	gtk_text_iter_forward_to_line_end(&end_iter);
	*/
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

	/* Include the newline char before and after this line */
	gtk_text_iter_backward_char(start_iter);
	gtk_text_iter_forward_char(end_iter);

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

static void
enable_bullets(GtkTextBuffer *buffer)
{
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buffer)) {
		/* Something is selected */
		gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
		add_bullets(buffer, gtk_text_iter_get_line(&start_iter), gtk_text_iter_get_line(&end_iter), buffer_get_depth_tag(buffer, 1));

	} else {
		/* Nothing is selected */
		int line;
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
		line = gtk_text_iter_get_line(&start_iter);
		add_bullets(buffer, line, line, buffer_get_depth_tag(buffer, 1));

		conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list-item");
		conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list");
	}
}

static void
disable_bullets(GtkTextBuffer *buffer)
{
	GtkTextIter start_iter, end_iter;

	if (gtk_text_buffer_get_has_selection(buffer)) {
		/* Something is selected */
		gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter);
		remove_bullets(buffer, &start_iter, &end_iter);

		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list-item");
		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list");

	} else {
		/* Nothing is selected */
		gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, gtk_text_buffer_get_insert(buffer));
		gtk_text_buffer_get_iter_at_mark(buffer, &end_iter, gtk_text_buffer_get_insert(buffer));
		remove_bullets(buffer, &start_iter, &end_iter);

		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list-item");
		conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list");
	}
}

void
on_bullets_button_clicked				(GtkAction		*action,
										 gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkTextBuffer *buffer = ui->buffer;

	if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action))) {
		/* The button just became active, so we should enable the formatting */
		enable_bullets(buffer);
		gtk_action_set_sensitive(ui->action_dec_indent, TRUE);
	} else {
		/* The button just became deactive, so we should remove the formatting */
		disable_bullets(buffer);
		gtk_action_set_sensitive(ui->action_dec_indent, FALSE);
	}
}

void
on_link_button_clicked				   (GtkAction		*action,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextIter start, end;
	const gchar *text;

	if (!gtk_text_buffer_get_has_selection(buffer)) {
		return;
	}

	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	text = gtk_text_iter_get_text(&start, &end);
	gtk_text_buffer_apply_tag_by_name(buffer, "link:internal", &start, &end);

	/* Make it save */
	gtk_text_buffer_set_modified(buffer, TRUE);

	note_show_by_title(text);
}

void
on_notes_button_clicked				   (GtkAction		*action,
										gpointer		 user_data) {

	AppData *app_data = app_data_get();
	g_return_if_fail(app_data->note_store != NULL);

	search_window_open();
}





/* TODO: The signal "mark-set" is emitted 4 times when clicking into the text. While selecting
 * it's emitted continuesly. */
void
on_textview_cursor_moved			   (GtkTextBuffer	*buffer,
										GtkTextIter		*location,
										GtkTextMark		*mark,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	const gchar *mark_name;

	/* Only enable the link action, if something is selected */
	gtk_action_set_sensitive(ui->action_link, gtk_text_buffer_get_has_selection(buffer));

	/* We only do something if the "insert" mark changed. */
	mark_name = gtk_text_mark_get_name(mark);
	if ((mark_name == NULL) || (strcmp(mark_name, "insert") != 0)) {
		return;
	}

	check_title(ui);

	conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(ui->buffer));
	conboy_note_window_update_button_states(ui);
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
	GtkTextIter start;
	gchar *link_text;

	if (type == GDK_BUTTON_RELEASE) {
		if (((GdkEventButton*)event)->button == 1) {

			g_printerr("Open Link \n");

			if (!gtk_text_iter_begins_tag(iter, tag)) {
				gtk_text_iter_backward_to_tag_toggle(iter, tag);
			}

			start = *iter;

			if (!gtk_text_iter_ends_tag(iter, tag)) {
				gtk_text_iter_forward_to_tag_toggle(iter, tag);
			}

			link_text = gtk_text_iter_get_text(&start, iter);

			g_printerr("Link: >%s< \n", link_text);
			note_show_by_title(link_text);
		}
	}

	return FALSE;
}

static void
open_url(gchar *url)
{
	AppData *app_data = app_data_get();

	g_printerr("INFO: Open URL: >%s<\n", url);

	DBusConnection *con = osso_get_dbus_connection(app_data->osso_ctx);

	if (hildon_mime_open_file(con, url)) {
		/* Everything ok. File was opened */
		g_printerr("INFO: URL successfull opened\n");
		return;
	}

	g_printerr("INFO: Cannot open URL with hildon_mime. Falling back to hardcoded values\n");

	if (strncmp(url, "http", 4) == 0 || strncmp(url, "ftp", 3) == 0)
	{
		g_printerr("Trying to open in browser\n");
		/* Open in browser */
		osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_browser",
				OSSO_BROWSER_OPEN_NEW_WINDOW_REQ, NULL,
				DBUS_TYPE_STRING, url, DBUS_TYPE_INVALID);
		return;
	}

	if (strncmp(url, "mailto", 6) == 0)
	{
		/* Open in Modest */
		#ifdef WITH_MODEST
		g_printerr("Trying to open with modest\n");
		libmodest_dbus_client_mail_to(app_data->osso_ctx, url);
		#endif
		return;
	}

	if (strncmp(url, "file", 4) == 0)
	{
		/* Open in Filemanager */
		g_printerr("Trying to open in file manager \n");

		/* Test if is a path and the path exits */
		gchar *path = url + 7;
		g_printerr("Path: %s\n", path);

		if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
			osso_rpc_run_with_defaults(app_data->osso_ctx, "osso_filemanager",
					"open_folder", NULL,
					DBUS_TYPE_STRING, path, DBUS_TYPE_INVALID);
			return;
		}
	}

	hildon_banner_show_informationf(GTK_WIDGET(app_data->note_window->window),
			NULL, "Cannot open '%s'", url);
}

/**
 * Create a proper URL out of the given link.
 *
 * Returns: Newly allocated string
 */
static gchar*
create_url(gchar *link)
{
	gchar *result;

	if (strncmp(link, "www.", 4) == 0) {
		result = g_strconcat("http://", link, NULL);
	}
	else if (strncmp(link, "/", 1) == 0 && strncmp(link, "//", 2) != 0) {
		result = g_strconcat("file://", link, NULL);
	}
	else if (strncmp(link, "~/", 2) == 0) {
		gchar *path = link + 1; /* Cut of the tilde char */
		gchar *file = g_build_filename(g_get_home_dir(), path, NULL);
		result = g_strconcat("file://", file, NULL);
		g_free(file);
	}
	else if (g_regex_match_simple("^(?!(news|mailto|http|https|ftp|file|irc):).+@.{2,}$", link, G_REGEX_CASELESS, 0)) {
		result = g_strconcat("mailto:", link, NULL);
	}
	else {
		result = g_strdup(link);
	}

	return result;
}

gboolean
on_link_url_tag_event				(GtkTextTag  *tag,
										 GObject     *object,
										 GdkEvent    *event,
										 GtkTextIter *iter,
										 gpointer     user_data)
{
	GdkEventType type = event->type;
	GtkTextIter start;
	gchar *link_text;

	if (type == GDK_BUTTON_RELEASE) {
		if (((GdkEventButton*)event)->button == 1) {

			if (!gtk_text_iter_begins_tag(iter, tag)) {
				gtk_text_iter_backward_to_tag_toggle(iter, tag);
			}

			start = *iter;

			if (!gtk_text_iter_ends_tag(iter, tag)) {
				gtk_text_iter_forward_to_tag_toggle(iter, tag);
			}

			link_text = gtk_text_iter_get_text(&start, iter);

			g_printerr("Link: >%s< \n", link_text);

			gchar *url = create_url(link_text);
			g_free(link_text);

			open_url(url);
		}
	}

	return FALSE;
}

void
on_delete_button_clicked			   (GtkAction		*action,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	AppData *app_data = app_data_get();

	gchar *message = g_strconcat("<b>",
			_("Really delete this note?"),
			"</b>\n\n",
			_("If you delete a note it is permanently lost."),
			NULL);

	GtkWidget *dialog = ui_helper_create_yes_no_dialog(GTK_WINDOW(ui->window), message);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_free(message);

	if (response == GTK_RESPONSE_YES) {

		/* Delete note */
		note_delete(ui->note);
		g_object_unref(ui->note);
		ui->note = NULL;

		/* Show previous note */
		ConboyNote *note;
		if (app_data->current_element == NULL) {
			note = conboy_note_store_get_latest(app_data->note_store);
			app_data->note_history = g_list_append(app_data->note_history, note);
			app_data->current_element = app_data->note_history;
			note_show(note, FALSE, TRUE, FALSE);
		} else {
			note = CONBOY_NOTE(app_data->current_element->data);
			note_show(note, FALSE, TRUE, FALSE);
		}

	}
}

static
gboolean note_save_callback(UserInterface *ui)
{
	/* Test, to see if the window is still open */
	if (ui->note != NULL && GTK_IS_TEXT_BUFFER(ui->buffer)) {
		if (gtk_text_buffer_get_modified(ui->buffer)) {
			gtk_text_view_set_editable(ui->view, FALSE);
			note_save(ui);
			gtk_text_view_set_editable(ui->view, TRUE);
		}
	}

	/* Return FALSE to make the Timer stop */
	return FALSE;
}

guint event_src_id = 0;

void
on_text_buffer_modified_changed			(GtkTextBuffer *buffer,
										 gpointer		user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	if (!gtk_text_buffer_get_modified(ui->buffer)) {
		return;
	}

	/*g_printerr("Buffer is dirty\n");*/

	/* Remove last timer, if exists. Not strictly needed, most of the time there won't be another timer */
	if (event_src_id > 0) {
		g_source_remove(event_src_id);
	}

	/* Save 4 seconds after the buffer got dirty */
	#ifdef HILDON_HAS_APP_MENU
	event_src_id = g_timeout_add_seconds(4, (GSourceFunc)note_save_callback, ui);
	#else
	event_src_id = g_timeout_add(4000, (GSourceFunc)note_save_callback, ui);
	#endif
}

static
void change_font_size_by(gint change)
{
	gint size;

	if (change == 0) {
		return;
	}

	size = settings_load_font_size();

	if (change > 0) {
		if (size + change <= 65000) {
			size += change;
		} else {
			return;
		}
	} else {
		if (size + change >= 5000) {
			size += change;
		} else {
			return;
		}
	}

	settings_save_font_size(size);
}

void
on_smaller_button_clicked			   (GtkAction		*action,
										gpointer		 user_data)
{
	change_font_size_by(-5000);
}

void
on_bigger_button_clicked				(GtkAction		*action,
										gpointer		 user_data)
{
	change_font_size_by(+5000);
}

gboolean on_hardware_key_pressed	(GtkWidget			*widget,
									 GdkEventKey		*event,
									 gpointer           user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkWidget *window = GTK_WIDGET(ui->window);
	AppData *app_data = app_data_get();

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
		on_window_delete(GTK_OBJECT(window), NULL, ui->note);
		return TRUE;

	case HILDON_HARDKEY_FULLSCREEN:
		/* Toggle fullscreen */
		ui_helper_toggle_fullscreen(GTK_WINDOW(ui->window));
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
static
gboolean line_needs_bullet(GtkTextBuffer *buffer, gint line_number)
{
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_line(buffer, &iter, line_number);

	while (!gtk_text_iter_ends_line(&iter)) {
		switch (gtk_text_iter_get_char(&iter))
		{
		case ' ':
			gtk_text_iter_forward_char(&iter);
			break;

		case '*':
		case '-':
			gtk_text_iter_forward_char(&iter);
			return (gtk_text_iter_get_char(&iter) == ' ');
			break;

		default:
			return FALSE;
		}
	}
	return FALSE;
}

/* TODO: This function must be cut into smaller parts */
static gboolean add_new_line(UserInterface *ui)
{
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextView *view = ui->view;
	GtkTextIter iter;
	GtkTextTag *depth_tag;
	gint line;

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
	gtk_text_iter_set_line_offset(&iter, 0);
	depth_tag = iter_get_depth_tag(&iter);

	/* If line starts with a bullet */
	if (depth_tag != NULL) {

		/* If line is not empty, add new bullet. Else remove bullet. */
		gtk_text_iter_forward_to_line_end(&iter);
		if (gtk_text_iter_get_line_offset(&iter) > 2) {
			GSList *tmp;

			/* Remove all tags but <list> from active tags */
			tmp = g_slist_copy(conboy_note_buffer_get_active_tags(CONBOY_NOTE_BUFFER(buffer)));
			conboy_note_buffer_clear_active_tags(CONBOY_NOTE_BUFFER(buffer));
			conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "list");

			/* Insert newline and bullet */
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_insert(buffer, &iter, "\n", -1);
			gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buffer));
			line = gtk_text_iter_get_line(&iter);
			add_bullets(buffer, line, line, depth_tag);

			/* Add all tags back to active tags */
			conboy_note_buffer_set_active_tags(CONBOY_NOTE_BUFFER(buffer), tmp);

			return TRUE;

		} else {
			/* Remove bullet and insert newline */
			GtkTextIter start = iter;

			/* Disable list and list-item tags */
			conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "list-item");
			conboy_note_buffer_remove_active_tag_by_name(CONBOY_NOTE_BUFFER(ui->buffer), "list");

			/* Delete the bullet and the last newline */
			gtk_text_iter_set_line_offset(&start, 0);
			gtk_text_iter_backward_char(&start);
			gtk_text_iter_forward_char(&iter); /* Also remove tag on comming linebreak */
			gtk_text_buffer_remove_all_tags(buffer, &start, &iter);
			gtk_text_iter_backward_char(&iter);
			gtk_text_buffer_delete(buffer, &start, &iter);

			gtk_text_buffer_insert(buffer, &iter, "\n", -1);

			/* Disable the bullet button */
			conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
			conboy_note_window_update_button_states(ui);

			return TRUE;
		}

	} else {

		/* If line start with a "- " or "- *" */
		if (line_needs_bullet(buffer, gtk_text_iter_get_line(&iter))) {
			GSList *tmp;
			GtkTextIter end_iter;

			gtk_text_iter_set_line_offset(&iter, 0);
			end_iter = iter;
			line = gtk_text_iter_get_line(&iter);

			/* Skip trailing spaces */
			while (gtk_text_iter_get_char(&end_iter) == ' ') {
				gtk_text_iter_forward_char(&end_iter);
			}
			/* Skip "* " or "- " */
			gtk_text_iter_forward_chars(&end_iter, 2);

			/* Delete this part */
			gtk_text_buffer_delete(buffer, &iter, &end_iter);

			/* Add bullet for this line */
			add_bullets(buffer, line, line, buffer_get_depth_tag(buffer, 1));


			/* TODO: Copied from above */

			/* Remove all tags but <list> from active tags */
			tmp = g_slist_copy(conboy_note_buffer_get_active_tags(CONBOY_NOTE_BUFFER(buffer)));
			conboy_note_buffer_clear_active_tags(CONBOY_NOTE_BUFFER(buffer));
			conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list");

			/* Insert newline and bullet */
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_insert(buffer, &iter, "\n", -1);
			line = gtk_text_iter_get_line(&iter);
			add_bullets(buffer, line, line, buffer_get_depth_tag(buffer, 1));
			gtk_text_view_scroll_mark_onscreen(view, gtk_text_buffer_get_insert(buffer));

			/* Add all tags back to active tags */
			if (tmp != NULL) {
				conboy_note_buffer_set_active_tags(CONBOY_NOTE_BUFFER(buffer), tmp);
			}

			/* Turn on the list-item tag from here on */
			conboy_note_buffer_add_active_tag_by_name(CONBOY_NOTE_BUFFER(buffer), "list-item");

			/* Revaluate (turn on) the bullet button */
			conboy_note_window_update_button_states(ui);

			return TRUE;
		}

	}

	return FALSE;

}


static void
apply_active_tags(GtkTextBuffer *buffer, GtkTextIter *iter, const gchar *input, UserInterface *ui)
{
	GtkTextIter start_iter;
	GSList *active_tags = conboy_note_buffer_get_active_tags(CONBOY_NOTE_BUFFER(buffer));

	/* The first line is always the title. Don't apply tags there */
	if (gtk_text_iter_get_line(iter) == 0) {
		return;
	}

	/* First remove all tags, then apply all active tags */
	start_iter = *iter;
	gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(input, -1));
	gtk_text_buffer_remove_all_tags(buffer, &start_iter, iter);

	while (active_tags != NULL && active_tags->data != NULL) {
		gtk_text_buffer_apply_tag(buffer, active_tags->data, &start_iter, iter);
		active_tags = active_tags->next;
	}

}

void
on_text_buffer_insert_text					(GtkTextBuffer *buffer,
											 GtkTextIter   *iter,
											 gchar		   *text,
											 gint			len,
											 gpointer		user_data)
{
	UserInterface *ui = (UserInterface*)user_data;
	GtkTextIter start_iter, end_iter;
	/*GTimer *timer;*/
	gulong micro;

	/* Don't do anything when in the title line */
	if (gtk_text_iter_get_line(iter) == 0) {
		__editing_title = TRUE;
		return;
	}

	check_title(ui);

	apply_active_tags(buffer, iter, text, ui);

	/*timer = g_timer_new();*/

	start_iter = *iter;
	end_iter = *iter;

	/* Move start iter back to the position before the insert */
	gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(text, -1));

	auto_highlight_links(ui, &start_iter, &end_iter);

	auto_highlight_urls(ui, &start_iter, &end_iter);

	/*
	g_timer_stop(timer);
	g_timer_elapsed(timer, &micro);
	g_timer_destroy(timer);
	g_printerr("Insert text: %lu micro seconds. \n", micro);
	*/
}

void
on_text_buffer_delete_range					(GtkTextBuffer *buffer,
											 GtkTextIter   *start_iter,
											 GtkTextIter   *end_iter,
											 gpointer		user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	/* Don't do anything when in the title line */
	if (gtk_text_iter_get_line(start_iter) == 0 || gtk_text_iter_get_line(end_iter) == 0) {
		__editing_title = TRUE;
		return;
	}

	check_title(ui);

	auto_highlight_links(ui, start_iter, end_iter);

	auto_highlight_urls(ui, start_iter, end_iter);
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
	}

	g_free(tag_name);

	return tag;
}

static
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
		tags = tags->next;
	}
	return NULL;
}

static
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

			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			/* Remove old tag */
			gtk_text_buffer_remove_tag(buffer, old_tag, &start_iter, &end_iter);

			/* Delete old bullet */
			gtk_text_buffer_delete(buffer, &start_iter, &end_iter);

			/* Add new bullet with new tag */
			add_bullets(buffer, i, i, new_tag);
		} else {
			/* Not yet a bullet list - create list */
			enable_bullets(buffer);
		}
	}
}

/* TODO: This is almost 100% copy&paste from increase_indent */
static
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

			if (depth < 1) {
				/* I think that should never happen */
				g_assert_not_reached();
				return;
			}

			gtk_text_buffer_get_iter_at_line(buffer, &end_iter, i);
			gtk_text_iter_set_line_offset(&end_iter, 2);

			if (depth == 1) {
				remove_bullets(buffer, &start_iter, &end_iter);
				return;
			}

			/* Remove old tag */
			gtk_text_buffer_remove_tag(buffer, old_tag, &start_iter, &end_iter);

			/* Delete old bullet */
			gtk_text_buffer_delete(buffer, &start_iter, &end_iter);

			depth--;
			new_tag = buffer_get_depth_tag(buffer, depth);

			/* Add new bullet with new tag */
			add_bullets(buffer, i, i, new_tag);
		}
	}
}

static gboolean
line_is_bullet_line(GtkTextIter *line_iter)
{
	GtkTextIter iter = *line_iter;
	gtk_text_iter_set_line(&iter, gtk_text_iter_get_line(line_iter));
	if (iter_get_depth_tag(&iter)) {
		return TRUE;
	} else {
		return FALSE;
	}
}


static gboolean
backspace(UserInterface* ui)
{
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextIter iter, sel_start, sel_end;

	if (gtk_text_buffer_get_selection_bounds(buffer, &sel_start, &sel_end)) {


		if (!line_is_bullet_line(&sel_start) && !line_is_bullet_line(&sel_end)) {
			return FALSE; /* Do normal backspace */
		}

		if (line_is_bullet_line(&sel_start) && line_is_bullet_line(&sel_end)) {

			/* Don't delete the bullet on the left if it is selected */
			int line_offset = gtk_text_iter_get_line_offset(&sel_start);
			if (line_offset <= 2) {
				gtk_text_iter_forward_chars(&sel_start, 2-line_offset);
			}

			gtk_text_buffer_delete(buffer, &sel_start, &sel_end);
			gtk_text_buffer_select_range(buffer, &sel_start, &sel_start);

			return TRUE;
		}

		if (line_is_bullet_line(&sel_start) && !line_is_bullet_line(&sel_end)) {

			/* Don't delete the bullet on the left if it is selected */
			int line_offset = gtk_text_iter_get_line_offset(&sel_start);
			if (line_offset <= 2) {
				gtk_text_iter_forward_chars(&sel_start, 2-line_offset);
			}

			gtk_text_buffer_delete(buffer, &sel_start, &sel_end);

			/* Add list and list-item tags to complete line */
			gtk_text_iter_set_line(&sel_start, gtk_text_iter_get_line(&sel_start));
			gtk_text_iter_forward_chars(&sel_start, 2);
			gtk_text_iter_forward_to_line_end(&sel_end);
			gtk_text_buffer_apply_tag_by_name(buffer, "list", &sel_start, &sel_end);
			gtk_text_buffer_apply_tag_by_name(buffer, "list-item", &sel_start, &sel_end);

			/* Remove selection */
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_select_range(buffer, &iter, &iter);

			return TRUE;
		}

		if (!line_is_bullet_line(&sel_start) && line_is_bullet_line(&sel_end)) {

			gtk_text_buffer_delete(buffer, &sel_start, &sel_end);

			/* Remove list and list-item tags from complete line */
			gtk_text_iter_set_line(&sel_start, gtk_text_iter_get_line(&sel_start));
			gtk_text_iter_forward_to_line_end(&sel_end);
			gtk_text_iter_forward_char(&sel_end);
			gtk_text_buffer_remove_tag_by_name(buffer, "list", &sel_start, &sel_end);
			gtk_text_buffer_remove_tag_by_name(buffer, "list-item", &sel_start, &sel_end);

			/* Remove selection */
			gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
			gtk_text_buffer_select_range(buffer, &iter, &iter);

			return TRUE;
		}
	}

	/*
	 * Nothing is selected
	 */

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	if (gtk_text_iter_get_line_offset(&iter) <= 2) {

		/* If cursor in list, decrease indent */
		if (line_is_bullet_line(&iter)) {

			/* Decrease indent */
			int line = gtk_text_iter_get_line(&iter);
			decrease_indent(buffer, line, line);

			/* Update UI */
			conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
			conboy_note_window_update_button_states(ui);

			/* Stop normal backspace effect */
			return TRUE;
		}

		/* If cursor not in list and on the very left and above is a list, add line to list */
		if (gtk_text_iter_get_line_offset(&iter) == 0) {

			GtkTextIter line_above = iter;
			gtk_text_iter_backward_line(&line_above);

			if (line_is_bullet_line(&line_above)) {
				/* Remove the line break */
				gtk_text_iter_forward_to_line_end(&line_above);
				gtk_text_buffer_delete(buffer, &line_above, &iter);
				/* Add list and list-item tags to complete line */
				gtk_text_iter_set_line(&iter, gtk_text_iter_get_line(&iter));
				gtk_text_iter_forward_to_line_end(&iter);
				gtk_text_buffer_apply_tag_by_name(buffer, "list", &line_above, &iter);
				gtk_text_buffer_apply_tag_by_name(buffer, "list-item", &line_above, &iter);

				/* Update UI */
				conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
				conboy_note_window_update_button_states(ui);

				/* Stop normal backspace effect */
				return TRUE;
			}
		}
	}

	/* Do normal backspace */
	return FALSE;
}


static gboolean
delete(UserInterface* ui)
{
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	/* If the cursor is directly before or after a bullet */
	if (gtk_text_iter_get_line_offset(&iter) <= 1) {
		//if (gtk_text_iter_has_tag(&iter, gtk_text_tag_table_lookup(buffer->tag_table, "list"))) {
		if (line_is_bullet_line(&iter)) {

			/* Decrease indent */
			int line = gtk_text_iter_get_line(&iter);
			decrease_indent(buffer, line, line);

			/* Update UI */
			conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
			conboy_note_window_update_button_states(ui);

			/* Stop normal delete effect */
			return TRUE;
		}
	}

	/* If the cursor is at the end of a bulleted line */
	GtkTextIter line_end = iter;
	/* First set to line start, then move to line end. This avoids that we forward over the linebreak */
	gtk_text_iter_set_line(&line_end, gtk_text_iter_get_line(&iter));
	gtk_text_iter_forward_to_line_end(&line_end);

	/* If cursor is at the end of the line */
	if (gtk_text_iter_equal(&iter, &line_end)) {

		/* If the current line is a list-item */
		gtk_text_iter_backward_char(&iter);
		//if (gtk_text_iter_has_tag(&iter, gtk_text_tag_table_lookup(buffer->tag_table, "list"))) {
		if (line_is_bullet_line(&iter)) {

			/* Move over line break */
			gtk_text_iter_forward_line(&iter);

			/* If next line has bullet, remove two chars additional, else remove only linebreak */
			//if (gtk_text_iter_has_tag(&iter, gtk_text_tag_table_lookup(buffer->tag_table, "list"))) {
			if (line_is_bullet_line(&iter)) {
				gtk_text_iter_forward_chars(&iter, 2);
				gtk_text_buffer_delete(buffer, &line_end, &iter);
			} else {
				gtk_text_buffer_delete(buffer, &line_end, &iter);
				/* First set to line start, then move to line end. This avoids that we forward over the linebreak */
				gtk_text_iter_set_line(&iter, gtk_text_iter_get_line(&iter));
				gtk_text_iter_forward_to_line_end(&iter);
				gtk_text_buffer_apply_tag_by_name(buffer, "list", &line_end, &iter);
				gtk_text_buffer_apply_tag_by_name(buffer, "list-item", &line_end, &iter);
			}

			/* Stop normal delete effect */
			return TRUE;
		}

		/* If the current line is not a list-item */
		/* Move over line break */
		gtk_text_iter_forward_line(&iter);

		/* If next line has bullet, remove two chars additional */
		//if (gtk_text_iter_has_tag(&iter, gtk_text_tag_table_lookup(buffer->tag_table, "list"))) {
		if (line_is_bullet_line(&iter)) {
			gtk_text_iter_forward_chars(&iter, 2);
			gtk_text_buffer_delete(buffer, &line_end, &iter);
			/* First set to line start, then move to line end. This avoids that we forward over the linebreak */
			gtk_text_iter_set_line(&iter, gtk_text_iter_get_line(&iter));
			gtk_text_iter_forward_to_line_end(&iter);
			gtk_text_iter_forward_char(&iter);
			gtk_text_buffer_remove_tag_by_name(buffer, "list", &line_end, &iter);
			gtk_text_buffer_remove_tag_by_name(buffer, "list-item", &line_end, &iter);

			return TRUE;
		}

	}

	/* Do normal delete */
	return FALSE;
}


static void
jump_bullet(UserInterface *ui)
{
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

	if (gtk_text_iter_get_line_offset(&iter) <= 1) {
		if (gtk_text_iter_has_tag(&iter, gtk_text_tag_table_lookup(buffer->tag_table, "list"))) {

			gtk_text_iter_set_line_offset(&iter, 2);
			gtk_text_buffer_place_cursor(buffer, &iter);
		}
	}
}


gboolean
on_text_view_key_pressed                      (GtkWidget   *widget,
                                               GdkEventKey *event,
                                               gpointer     user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	switch (event->keyval) {
		case GDK_Return:
		case GDK_KP_Enter:
			return add_new_line(ui);

		case GDK_BackSpace:
			return backspace(ui);

		case GDK_Delete:
		case GDK_KP_Delete:
			return delete(ui);

		case GDK_KP_Left:
		case GDK_KP_Right:
		case GDK_KP_Up:
		case GDK_KP_Down:
		case GDK_Left:
		case GDK_Right:
		case GDK_Up:
		case GDK_Down:
			return FALSE; // Do nothing

		default:
			jump_bullet(ui);
			return FALSE;
	}

}


void
on_inc_indent_button_clicked			   (GtkAction		*action,
											gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*) user_data;
	GtkTextBuffer *buffer = ui->buffer;
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

	conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
	conboy_note_window_update_button_states(ui);

	gtk_text_buffer_set_modified(buffer, TRUE);
}

/* TODO: Increase and decrease too similar */
void
on_dec_indent_button_clicked			   (GtkAction		*action,
											gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*) user_data;
	GtkTextBuffer *buffer = ui->buffer;
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

	conboy_note_buffer_update_active_tags(CONBOY_NOTE_BUFFER(buffer));
	conboy_note_window_update_button_states(ui);

	gtk_text_buffer_set_modified(buffer, TRUE);
}

void
on_style_button_clicked                (GtkAction       *action,
		                                gpointer         user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	#ifdef HILDON_HAS_APP_MENU
	hildon_app_menu_popup(HILDON_APP_MENU(ui->style_menu), GTK_WINDOW(ui->window));
	#else
	gtk_widget_show_all(GTK_WIDGET(ui->style_menu));
	gtk_menu_popup(GTK_MENU(ui->style_menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	#endif
}

void on_find_button_clicked(GtkAction *action, gpointer user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	if (ui->find_bar_is_visible) {
		gtk_widget_hide_all(GTK_WIDGET(ui->find_bar));
		ui->find_bar_is_visible = FALSE;
	} else {
		gtk_widget_show_all(GTK_WIDGET(ui->find_bar));
		hildon_find_toolbar_highlight_entry(ui->find_bar, TRUE);
		ui->find_bar_is_visible = TRUE;
	}
}

void on_find_bar_search(GtkWidget *widget, UserInterface *ui)
{
	GtkTextBuffer *buffer = ui->buffer;
	GtkTextView *view = ui->view;
	gchar *search_str;
	GtkTextMark *mark;
	GtkTextIter iter, match_start, match_end;

	/* Get the search string from the widget */
	g_object_get(G_OBJECT(widget), "prefix", &search_str, NULL);

	/* The mark "search_pos" remembers the position for subsequent calls
	 * to this method */
	mark = gtk_text_buffer_get_mark(buffer, "search_pos");

	if (mark == NULL) {
		gtk_text_buffer_get_start_iter(buffer, &iter);
		mark = gtk_text_buffer_create_mark(buffer, "search_pos", &iter, FALSE);
	}

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
	if (gtk_text_iter_forward_search(&iter, search_str, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL)) {
		mark = gtk_text_buffer_create_mark(buffer, "search_pos", &match_end, FALSE);
		gtk_text_view_scroll_mark_onscreen(view, mark);
		gtk_text_buffer_select_range(buffer, &match_start, &match_end);
	} else {
		gtk_text_buffer_delete_mark(buffer, mark);
		gtk_text_buffer_select_range(buffer, &iter, &iter);
	}

	g_free(search_str);
}

void on_find_bar_close(GtkWidget *widget, UserInterface *ui)
{
	gtk_widget_hide_all(widget);
	ui->find_bar_is_visible = FALSE;
}

void
on_back_button_clicked (GtkAction *action, gpointer user_data)
{
	AppData *app_data = app_data_get();

	g_return_if_fail(app_data->current_element != NULL);
	g_return_if_fail(app_data->current_element->prev != NULL);

	app_data->current_element = app_data->current_element->prev;

	ConboyNote *note = CONBOY_NOTE(app_data->current_element->data);
	note_show(note, FALSE, TRUE, FALSE);
}

void
on_forward_button_clicked (GtkAction *action, gpointer user_data)
{
	AppData *app_data = app_data_get();

	g_return_if_fail(app_data->current_element != NULL);
	g_return_if_fail(app_data->current_element->next != NULL);

	app_data->current_element = app_data->current_element->next;

	ConboyNote *note = CONBOY_NOTE(app_data->current_element->data);
	note_show(note, FALSE, TRUE, FALSE);
}

void
on_fullscreen_button_clicked		   (GtkAction		*action,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*) user_data;
	ui_helper_toggle_fullscreen(GTK_WINDOW(ui->window));
}

void
on_about_button_clicked				   (GtkAction		*action,
										gpointer		 user_data)
{
	UserInterface *ui = (UserInterface*)user_data;

	HeAboutDialog *dia = HE_ABOUT_DIALOG(he_about_dialog_new());
	gtk_window_set_transient_for(GTK_WINDOW(dia), GTK_WINDOW(ui->window));

	he_about_dialog_set_app_name(dia, "Conboy");
	he_about_dialog_set_bugtracker(dia, "https://bugs.maemo.org/enter_bug.cgi?product=Conboy");
	he_about_dialog_set_copyright(dia, "(c) Cornelius Hald 2010");
	he_about_dialog_set_description(dia, "Conboy is a note taking application.");
	he_about_dialog_set_icon_name(dia, "conboy");
	he_about_dialog_set_version(dia, VERSION);
	he_about_dialog_set_website(dia, "http://conboy.garage.maemo.org");

	gtk_dialog_run(GTK_DIALOG(dia));
	gtk_widget_destroy(GTK_WIDGET(dia));
}

