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

/**
 * Simple wrapper that it can be used from g_list_foreach()
 */
static void
free_data(gpointer data, gpointer ignore)
{
	g_free(data);
}

void
settings_save_use_auto_portrait_mode(gboolean use)
{
	AppData *app_data = app_data_get();
	gconf_client_set_bool(app_data->client, SETTINGS_USE_AUTO_PORTRAIT, use, NULL);
}

gboolean
settings_load_use_auto_portrait_mode()
{
	AppData *app_data = app_data_get();
	return gconf_client_get_bool(app_data->client, SETTINGS_USE_AUTO_PORTRAIT, NULL);
}

void
settings_save_last_open_note(const gchar *guid)
{
	AppData *app_data = app_data_get();
	gconf_client_set_string(app_data->client, SETTINGS_LAST_OPEN_NOTE, guid, NULL);
}

gchar*
settings_load_last_open_note()
{
	AppData *app_data = app_data_get();
	return gconf_client_get_string(app_data->client, SETTINGS_LAST_OPEN_NOTE, NULL);
}

void
settings_save_last_scroll_position(gdouble pos)
{
	AppData *app_data = app_data_get();
	gconf_client_set_float(app_data->client, SETTINGS_LAST_SCROLL_POSITION, pos, NULL);
}

gdouble
settings_load_last_scroll_position()
{
	AppData *app_data = app_data_get();
	return gconf_client_get_float(app_data->client, SETTINGS_LAST_SCROLL_POSITION, NULL);
}

void
settings_save_last_sync_time(time_t time)
{
	AppData *app_data = app_data_get();
	gconf_client_set_int(app_data->client, SETTINGS_STORAGE_LAST_SYNC_TIME, time, NULL);
}

time_t
settings_load_last_sync_time()
{
	AppData *app_data = app_data_get();
	return (time_t) gconf_client_get_int(app_data->client, SETTINGS_STORAGE_LAST_SYNC_TIME, NULL);
}

void
settings_save_last_sync_revision(gint revision)
{
	AppData *app_data = app_data_get();
	gconf_client_set_int(app_data->client, SETTINGS_STORAGE_LAST_SYNC_REV, revision, NULL);
}

gint
settings_load_last_sync_revision(void)
{
	AppData *app_data = app_data_get();
	return gconf_client_get_int(app_data->client, SETTINGS_STORAGE_LAST_SYNC_REV, NULL);
}

void
settings_save_sync_base_url(const gchar *url)
{
	AppData *app_data = app_data_get();
	gconf_client_set_string(app_data->client, SETTINGS_SYNC_BASE_URL, url, NULL);
}

gchar*
settings_load_sync_base_url()
{
	AppData *app_data = app_data_get();
	return gconf_client_get_string(app_data->client, SETTINGS_SYNC_BASE_URL, NULL);
}

void
settings_save_active_plugins(GSList *active_plugins)
{
	AppData *app_data = app_data_get();
	gconf_client_set_list(app_data->client, SETTINGS_ACTIVE_PLUGINS, GCONF_VALUE_STRING, active_plugins, NULL);
}

GSList*
settings_load_active_plugins()
{
	AppData *app_data = app_data_get();
	GSList *list = gconf_client_get_list(app_data->client, SETTINGS_ACTIVE_PLUGINS, GCONF_VALUE_STRING, NULL);
	return list;
}

void
settings_add_active_plugin(const gchar *name)
{
	GSList *plugin_names = settings_load_active_plugins();
	/* We need to duplicate name here, because later all elements get freed */
	plugin_names = g_slist_prepend(plugin_names, g_strdup(name));
	settings_save_active_plugins(plugin_names);

	g_slist_foreach(plugin_names, free_data, NULL);
	g_slist_free(plugin_names);
}

void
settings_remove_active_plugin(const gchar *name)
{
	GSList *plugin_names = settings_load_active_plugins();
	gchar *found_element = NULL;

	/* Find the right element */
	GSList *iter = plugin_names;
	while (iter) {
		gchar *plugin_name = (gchar*) iter->data;
		if (g_str_equal(plugin_name, name)) {
			found_element = plugin_name;
			break;
		}
		iter = iter->next;
	}

	/* Remove the element */
	if (found_element != NULL) {
		plugin_names = g_slist_remove(plugin_names, found_element);
		settings_save_active_plugins(plugin_names);
	} else {
		g_printerr("WARN: settings_remove_active_plugin: Element not found \n");
	}

	/* Free everything */
	g_free(found_element);
	g_slist_foreach(plugin_names, free_data, NULL);
	g_slist_free(plugin_names);
}

void
settings_save_oauth_access_token(const gchar *token)
{
	AppData *app_data = app_data_get();
	gconf_client_set_string(app_data->client, SETTINGS_OAUTH_ACCESS_TOKEN, token, NULL);
}

gchar*
settings_load_oauth_access_token(void)
{
	AppData *app_data = app_data_get();
	return gconf_client_get_string(app_data->client, SETTINGS_OAUTH_ACCESS_TOKEN, NULL);
}

void
settings_save_oauth_access_secret(const gchar *secret)
{
	AppData *app_data = app_data_get();
	gconf_client_set_string(app_data->client, SETTINGS_OAUTH_ACCESS_SECRET, secret, NULL);
}

gchar*
settings_load_oauth_access_secret(void)
{
	AppData *app_data = app_data_get();
	return gconf_client_get_string(app_data->client, SETTINGS_OAUTH_ACCESS_SECRET, NULL);
}

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
	if (hex_color == NULL) {
		/* GConf key or value does not exist. Create key with default value. Then read again. */
		save_default_color(type);
		hex_color = gconf_client_get_string(app_data->client, settings_color_enum_to_key(type), NULL);
	}
	gdk_color_parse(hex_color, color);
	g_free(hex_color);
}


