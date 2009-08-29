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

#include "interface.h"


typedef struct
{
  
  gchar   *title;
  gchar   *content;

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


void note_free(Note *note);

void note_format_title(GtkTextBuffer *buffer);

void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer);

void note_save(UserInterface *ui);

void note_close_window(UserInterface *ui);

void note_delete(ConboyNote *note);

gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer);

void note_show(ConboyNote *note);

void note_show_by_title(const char* title);

void note_show_existing(Note *note);

void note_show_new(Note *note);

gboolean note_is_open(UserInterface *ui);

void note_set_focus(UserInterface *ui);

gboolean note_exists(ConboyNote *note);

void note_add_active_tag(UserInterface *ui, GtkTextTag *tag);

void note_remove_active_tag(UserInterface *ui, GtkTextTag *tag);

void note_add_active_tag_by_name(UserInterface *ui, const gchar *tag_name);

void note_remove_active_tag_by_name(UserInterface *ui, const gchar *tag_name);

void note_add_tag(Note *note, gchar *tag);

#endif /* NOTE_H */
