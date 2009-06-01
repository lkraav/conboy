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

#ifndef NOTE_H
#define NOTE_H

#include <gtk/gtk.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-find-toolbar.h>

typedef struct
{
	HildonWindow        *window;
	GtkTextView         *view;
	GtkTextBuffer       *buffer;
	HildonFindToolbar	*find_bar;
	gboolean             find_bar_is_visible;
	GtkWidget			*style_menu;

	GtkToggleToolButton *button_bold;
	GtkToggleToolButton *button_italic;
	GtkToggleToolButton *button_strike;
	GtkToggleToolButton *button_highlight;
	GtkToggleToolButton *button_bullets;

	GtkToggleAction     *action_bold;
	GtkToggleAction     *action_italic;
	GtkToggleAction	    *action_fixed;
	GtkToggleAction     *action_strike;
	GtkToggleAction     *action_highlight;
	GtkToggleAction     *action_bullets;
	GtkAction           *action_link;
	GtkRadioAction		*action_font_small;
	GtkAction			*action_inc_indent;
	GtkAction			*action_dec_indent;

} UserInterface;

typedef struct
{
  UserInterface *ui;
  gchar   *title;
  gchar   *filename;

  time_t last_change_date;
  time_t last_metadata_change_date;
  time_t create_date;

  gint cursor_position;
  gint width;
  gint height;
  gint x;
  gint y;
  gboolean open_on_startup;
  double version;
  double content_version;

  gchar *guid;
  GList *tags; /* Tags aka notebooks */
  
  GSList *active_tags; /* TODO: This shouldn't really be part of this structure, more UI, because it is never written to disk */
  

} Note;


Note* note_create_new(void);

void note_free(Note *note);

void note_format_title(GtkTextBuffer *buffer);

void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer);

void note_save(Note *note);

void note_close_window(Note *note);

void note_delete(Note *note);

gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer);

void note_show(Note *note);

void note_show_by_title(const char* title);

void note_show_existing(Note *note);

void note_show_new(Note *note);

gboolean note_is_open(Note *note);

void note_set_focus(Note *note);

gboolean note_exists(Note *note);

void note_add_active_tag(Note *note, GtkTextTag *tag);

void note_remove_active_tag(Note *note, GtkTextTag *tag);

void note_add_active_tag_by_name(Note *note, const gchar *tag_name);

void note_remove_active_tag_by_name(Note *note, const gchar *tag_name);

void note_add_tag(Note *note, gchar *tag);

#endif /* NOTE_H */
