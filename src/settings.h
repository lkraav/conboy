#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <glib/gtypes.h>
#include <gdk/gdkcolor.h>

#define SETTINGS_ROOT              "/apps/maemo/conboy"
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

void settings_save_scrollbar_size(SettingsScrollbarSize size);
SettingsScrollbarSize settings_load_scrollbar_size(void);

gboolean setting_load_use_costum_colors(void);
void settings_load_color(GdkColor *color, SettingsColorType type);


void settings_save_startup_window(SettingsStartupWindow win);
SettingsStartupWindow settings_load_startup_window(void);

void settings_save_use_custom_colors(gboolean use);
gboolean settings_load_use_costum_colors(void);

void settings_save_color(GdkColor *color, SettingsColorType type);






#endif /*SETTINGS_H_*/
