#ifndef SETTINGS_H_
#define SETTINGS_H_


        
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



#endif /*SETTINGS_H_*/
