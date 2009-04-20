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
#include "metadata.h"

Note* note_create_new();

void note_format_title(GtkTextBuffer *buffer);

void note_set_window_title_from_buffer(GtkWindow *win, GtkTextBuffer *buffer);

void note_save(Note *note);

void note_close_window(Note *note);

const gchar* note_extract_title_from_buffer(GtkTextBuffer *buffer);

void note_show(Note *note);

void note_show_by_title(const char* title);

void note_show_existing(Note *note);

void note_show_new(Note *note);

gboolean note_is_open(Note *note);

void note_set_focus(Note *note);

gboolean note_exists(Note *note);

#endif /* NOTE_H */
