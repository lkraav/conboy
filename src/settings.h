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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <glib/gtypes.h>
#include <gdk/gdkcolor.h>

/* TODO: It would be nice to have those defines not in the public interface,
 * but therefor settings.c must be able to send "changed signals. Those are
 * ATM handled using direct gconf and not the wrappers in settings.c.
 */
#define SETTINGS_ROOT              "/apps/maemo/conboy"
#define SETTINGS_FONT_SIZE         SETTINGS_ROOT"/font_size"
#define SETTINGS_SCROLLBAR_SIZE    SETTINGS_ROOT"/scrollbar_size"
#define SETTINGS_STARTUP_WINDOW    SETTINGS_ROOT"/startup_window"
#define SETTINGS_USE_CUSTOM_COLORS SETTINGS_ROOT"/use_custom_colors"
#define SETTINGS_BACKGROUND_COLOR  SETTINGS_ROOT"/background_color"
#define SETTINGS_TEXT_COLOR        SETTINGS_ROOT"/text_color"
#define SETTINGS_LINK_COLOR        SETTINGS_ROOT"/link_color"


typedef enum {
	SETTINGS_SCROLLBAR_SIZE_SMALL,
	SETTINGS_SCROLLBAR_SIZE_BIG
} SettingsScrollbarSize;

typedef enum {
	SETTINGS_STARTUP_WINDOW_NOTE,
	SETTINGS_STARTUP_WINDOW_SEARCH
} SettingsStartupWindow;

typedef enum {	
	SETTINGS_COLOR_TYPE_BACKGROUND,
	SETTINGS_COLOR_TYPE_TEXT,
	SETTINGS_COLOR_TYPE_LINKS
} SettingsColorType;


void settings_save_font_size(gint size);
gint settings_load_font_size(void);

void settings_save_scrollbar_size(SettingsScrollbarSize size);
SettingsScrollbarSize settings_load_scrollbar_size(void);

void settings_save_use_custom_colors(gboolean use);
gboolean setting_load_use_costum_colors(void);

void settings_save_startup_window(SettingsStartupWindow win);
SettingsStartupWindow settings_load_startup_window(void);

void settings_save_use_custom_colors(gboolean use);
gboolean settings_load_use_costum_colors(void);

void settings_save_color(GdkColor *color, SettingsColorType type);
void settings_load_color(GdkColor *color, SettingsColorType type);


#endif /*SETTINGS_H_*/
