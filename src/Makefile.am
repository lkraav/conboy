# Sub directories
#SUBDIRS = tracker plugins
SUBDIRS = plugins

# Includes
INCLUDES = $(GMODULE_CFLAGS) $(HILDON_CFLAGS) $(OSSO_CFLAGS) $(HILDONDESKTOP_CFLAGS) $(GCONF_CFLAGS) $(XML2_CFLAGS) $(JSON_CFLAGS) $(OAUTH_CFLAGS) $(CURL_CFLAGS) $(PCRE_CFLAGS) $(HILDON_MIME_CFLAGS) $(MODEST_CFLAGS)

# Binary
bin_PROGRAMS = \
        conboy
		
#pluginlib_LTLIBRARIES = \
#		libconboy_applet.la
# /Binary

# Sources
conboy_SOURCES = \
        localisation.h \
        main.c \
        interface.h \
        interface.c \
        callbacks.h \
        callbacks.c \
        localisation.h \
        metadata.h \
        metadata.c \
        note.h \
        note.c \
        search_window.h \
        search_window.c \
        conboy_note_store.c \
        conboy_note_store.h \
        note_linker.h \
        note_linker.c \
        app_data.h \
        app_data.c \
        json.h \
        json.c \
        settings.h \
        settings.c \
        settings_window.h \
        settings_window.c \
        search.h \
        search.c \
        conboy_note.h \
        conboy_note.c \
        conboy_oauth.h \
        conboy_oauth.c \
        conboy_plugin.h \
        conboy_plugin.c \
        conboy_storage_plugin.h \
        conboy_storage_plugin.c \
        conboy_plugin_info.h \
        conboy_plugin_info.c \
        conboy_plugin_manager.h \
        conboy_plugin_manager.c \
        conboy_storage.h \
        conboy_storage.c \
        conboy_note_buffer.h \
        conboy_note_buffer.c \
        conboy_plugin_store.h \
        conboy_plugin_store.c \
        conboy_check_button.h \
        conboy_check_button.c \
        conboy_check_mark.h \
        conboy_check_mark.c \
        orientation.h \
        orientation.c \
        fullscreenmanager.h \
        fullscreenmanager.c \
        ui_helper.h \
        ui_helper.c \
        conboy_plugin_manager_row.h \
        conboy_plugin_manager_row.c \
        gregex.h \
        gregex.c
        

#libconboy_applet_la_SOURCES = \
#		conboy_applet.c \
#		conboy_applet.h \
#		metadata.h \
#       metadata.cs
		
# /Sources

# XXX
#libconboy_applet_la_LDFLAGS = -module -avoid-version 
# /XXX

# LDADD
conboy_LDADD = \
        $(GMODULE_LIBS) $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(HILDONDESKTOP_LIBS) $(XML2_LIBS) $(JSON_LIBS) $(OAUTH_LIBS) $(CURL_LIBS) $(PCRE_LIBS) $(HILDON_MIME_LIBS) $(MODEST_LIBS)

#libconboy_applet_la_LIBADD = \
#		$(HILDONDESKTOP_LIBS)
# /LDADD



