

#include "settings.h"
#include "app_data.h"


        




void
settings_save_scrollbar_size(SettingsScrollbarSize size)
{
	AppData *app_data = app_data_get();
	g_printerr("Save scrollbar: %i \n", size);
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
		g_printerr("Load scrollbar: %i \n", size);
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
	g_printerr("Save custom colors: %i \n", use);
	gconf_client_set_bool(app_data->client, SETTINGS_USE_CUSTOM_COLORS, use, NULL);
}

gboolean
settings_load_use_costum_colors()
{
	AppData *app_data = app_data_get();
	gboolean use = gconf_client_get_bool(app_data->client, SETTINGS_USE_CUSTOM_COLORS, NULL);
	g_printerr("Load custom colors: %i \n", use);
	return use;
}



/**
 * You have to free the gchar after using.
 */
static gchar*
gdk_color_to_string(GdkColor *color)
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

void
settings_save_color(GdkColor *color, SettingsColorType type)
{
	AppData *app_data = app_data_get();
	gchar *hex_color = gdk_color_to_string(color);

	gconf_client_set_string(app_data->client, settings_color_enum_to_key(type), hex_color, NULL);
	
	g_free(hex_color);
}

void settings_load_color(GdkColor *color, SettingsColorType type)
{
	AppData *app_data = app_data_get();	
	gchar *hex_color = gconf_client_get_string(app_data->client, settings_color_enum_to_key(type), NULL);
	gdk_color_parse(hex_color, color);
	g_free(hex_color);
}


