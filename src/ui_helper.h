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

#ifndef _UI_HELPER_H_
#define _UI_HELPER_H_


GtkWidget*
ui_helper_create_yes_no_dialog(GtkWindow *parent, const gchar *message);

GtkWidget*
ui_helper_create_confirmation_dialog(GtkWindow *parent, const gchar *message);

GtkWidget*
ui_helper_create_cancel_dialog(GtkWindow *parent, const gchar *message);

void
ui_helper_show_confirmation_dialog(GtkWindow *parent, const gchar *message, gboolean supports_portrait);

void
ui_helper_toggle_fullscreen(GtkWindow *active_window);

void
ui_helper_remove_portrait_support(GtkWindow *window);



#endif /* _UI_HELPER_H_ */
