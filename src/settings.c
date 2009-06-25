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

#include "settings.h"
#include "app_data.h"

void
settings_save_font_size(gint size)
{
	AppData *app_data = app_data_get();
	gconf_client_set_int(app_data->client, SETTINGS_FONT_SIZE, size, NULL);
}

gint
settings_load_font_size()
{
	AppData *app_data = app_data_get();
	gint size = gconf_client_get_int(app_data->client, SETTINGS_FONT_SIZE, NULL);
	if (size == 0) {
		size = 20000; /* Default size */
	}
	return size;
}

void
settings_save_scrollbar_size(SettingsScrollbarSize size)
{
	AppData *app_data = app_data_get();
	gconf_client_set_int(app_data->client, SETTINGS_SCROLLBAR_SIZE, size, NULL);
}

SettingsScrollbarSize
settings_load_scrollbar_size()
{
	AppData *app_data = app_data_get();
	gint size = gconf_client_get_int(app_data->client, SETTINGS_SCROLLBAR_SIZE, NULL);
	if (size < 0 || size > 1) {
		g_printerr("ERROR: Unknown scrollbar size. Size: %i \n", size);
		return SETTINGS_SCROLLBAR_SIZE_SMALL;
	} else {
		return size;
	}
}

void
settings_save_startup_window(SettingsStartupWindow win)
{
	AppData *app_data = app_data_get();
	gconf_client_set_int(app_data->client, SETTINGS_STARTUP_WINDOW, win, NULL);
}

SettingsStartupWindow
settings_load_startup_window()
{
	AppData *app_data = app_data_get();
	gint win = gconf_client_get_int(app_data->client, SETTINGS_STARTUP_WINDOW, NULL);
	if (win < 0 || win > 1) {
		g_printerr("ERROR: Unknown startup window. Window type: %i \n", win);
		return SETTINGS_STARTUP_WINDOW_SEARCH;
	} else {
		return win;
	}
}

void
settings_save_use_custom_colors(gboolean use)
{
	AppData *app_data = app_data_get();
	gconf_client_set_bool(app_data->client, SETTINGS_USE_CUSTOM_COLORS, use, NULL);
}

gboolean
settings_load_use_costum_colors()
{
	AppData *app_data = app_data_get();
	gboolean use = gconf_client_get_bool(app_data->client, SETTINGS_USE_CUSTOM_COLORS, NULL);
	return use;
}

/**
 * You have to free the gchar after using.
 */
static gchar*
conboy_gdk_color_to_string(GdkColor *color)
{
	PangoColor pColor;
	
	pColor.red = color->red;
	pColor.green = color->green;
	pColor.blue = color->blue;
	
	return pango_color_to_string(&pColor);
}

const static gchar*
settings_color_enum_to_key(SettingsColorType type)
{
	switch(type) {
	case SETTINGS_COLOR_TYPE_BACKGROUND:
		return SETTINGS_BACKGROUND_COLOR;
	case SETTINGS_COLOR_TYPE_TEXT:
		return SETTINGS_TEXT_COLOR;
	case SETTINGS_COLOR_TYPE_LINKS:
		return SETTINGS_LINK_COLOR;
	default:
		g_assert_not_reached();
	}
}

static void
save_default_color(SettingsColorType type)
{
	GdkColor color;
	switch(type) {
	case SETTINGS_COLOR_TYPE_BACKGROUND: gdk_color_parse("gray", &color); break;
	case SETTINGS_COLOR_TYPE_TEXT: gdk_color_parse("black", &color); break;
	case SETTINGS_COLOR_TYPE_LINKS: gdk_color_parse("blue", &color); break;
	default: g_assert_not_reached();
	}
	
	settings_save_color(&color, type);
}

void
settings_save_color(GdkColor *color, SettingsColorType type)
{
	AppData *app_data = app_data_get();
	gchar *hex_color = conboy_gdk_color_to_string(color);
	
	gconf_client_set_string(app_data->client, settings_color_enum_to_key(type), hex_color, NULL);
	
	g_free(hex_color);
}

void settings_load_color(GdkColor *color, SettingsColorType type)
{
	AppData *app_data = app_data_get();
	GError *error;
	gchar *hex_color = gconf_client_get_string(app_data->client, settings_color_enum_to_key(type), &error);
	if (error != NULL || hex_color == NULL) {
		/* GConf key or value does not exist. Create key with default value. Then read again. */
		save_default_color(type);
		hex_color = gconf_client_get_string(app_data->client, settings_color_enum_to_key(type), NULL);
	}
	gdk_color_parse(hex_color, color);
	g_free(hex_color);
}


