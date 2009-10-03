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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <gtk/gtk.h>
#include "conboy_note.h"

typedef struct
{
	ConboyNote			*note;
	HildonWindow        *window;
	GtkTextView         *view;
	GtkTextBuffer       *buffer;
	HildonFindToolbar	*find_bar;
	gboolean             find_bar_is_visible;
	GtkWidget			*style_menu;
	GtkToolbar			*toolbar;
	GtkWidget			*app_menu;

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
	GtkAction			*action_back;
	GtkAction			*action_forward;
	GtkAction			*action_delete;
	GtkAction			*action_text_style;
	GtkAction			*action_find;

	GtkWidget			*menu_open;
	GList               *listeners;



} UserInterface;

UserInterface* create_mainwin(ConboyNote *note);

void conboy_note_window_show_note(UserInterface *ui, ConboyNote *note);


#endif /* INTERFACE_H */
