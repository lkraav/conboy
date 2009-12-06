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
#include <gconf/gconf-client.h>

/* TODO: It would be nice to have those defines not in the public interface,
 * but therefor settings.c must be able to send "changed signals. Those are
 * ATM handled using direct gconf and not the wrappers in settings.c.
 */
#define SETTINGS_ROOT                "/apps/maemo/conboy"
#define SETTINGS_FONT_SIZE           SETTINGS_ROOT"/font_size"
#define SETTINGS_SCROLLBAR_SIZE      SETTINGS_ROOT"/scrollbar_size"
#define SETTINGS_STARTUP_WINDOW      SETTINGS_ROOT"/startup_window"
#define SETTINGS_USE_CUSTOM_COLORS   SETTINGS_ROOT"/use_custom_colors"
#define SETTINGS_BACKGROUND_COLOR    SETTINGS_ROOT"/background_color"
#define SETTINGS_TEXT_COLOR          SETTINGS_ROOT"/text_color"
#define SETTINGS_LINK_COLOR          SETTINGS_ROOT"/link_color"
#define SETTINGS_OAUTH_ACCESS_TOKEN  SETTINGS_ROOT"/oauth_access_token"
#define SETTINGS_OAUTH_ACCESS_SECRET SETTINGS_ROOT"/oauth_access_secret"
#define SETTINGS_ACTIVE_PLUGINS      SETTINGS_ROOT"/active_plugins"
#define SETTINGS_SYNC_BASE_URL       SETTINGS_ROOT"/sync_base_url"
#define SETTINGS_STORAGE_PLUGIN_NAME SETTINGS_ROOT"/storage_plugin_name"
#define SETTINGS_STORAGE_LAST_SYNC_REV SETTINGS_ROOT"/last_sync_rev"
#define SETTINGS_STORAGE_LAST_SYNC_TIME SETTINGS_ROOT"/last_sync_time"
#define SETTINGS_LAST_SCROLL_POSITION SETTINGS_ROOT"/last_scroll_position"
#define SETTINGS_LAST_OPEN_NOTE      SETTINGS_ROOT"/last_open_note"
#define SETTINGS_USE_AUTO_PORTRAIT   SETTINGS_ROOT"/use_auto_portrait_mode"

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


void settings_save_use_auto_portrait_mode(gboolean use);
gboolean settings_load_use_auto_portrait_mode(void);

void settings_save_last_open_note(const gchar *guid);
gchar* settings_load_last_open_note(void);

void settings_save_last_scroll_position(gdouble pos);
gdouble settings_load_last_scroll_position(void);

void settings_save_last_sync_time(time_t time);
time_t settings_load_last_sync_time(void);

void settings_save_last_sync_revision(gint revision);
gint settings_load_last_sync_revision(void);

void settings_save_sync_base_url(const gchar *url);
gchar* settings_load_sync_base_url(void);

void settings_save_active_plugins(GSList *active_plugins);
GSList* settings_load_active_plugins(void);

void settings_add_active_plugin(const gchar *name);
void settings_remove_active_plugin(const gchar *name);

void settings_save_oauth_access_token(const gchar *token);
gchar* settings_load_oauth_access_token(void);

void settings_save_oauth_access_secret(const gchar *secret);
gchar* settings_load_oauth_access_secret(void);

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
