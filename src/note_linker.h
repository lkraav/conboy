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

#ifndef NOTE_LINKER_H_
#define NOTE_LINKER_H_

#include <gtk/gtk.h>
#include "note.h"

void auto_highlight_links(UserInterface *ui, GtkTextIter *start_iter, GtkTextIter *end_iter);

void auto_highlight_urls(UserInterface *ui, GtkTextIter *start_iter, GtkTextIter *end_iter);

#endif /*NOTE_LINKER_H_*/
