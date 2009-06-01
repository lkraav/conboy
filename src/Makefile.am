# Includes
INCLUDES = $(HILDON_CFLAGS) $(OSSO_CFLAGS) $(HILDONDESKTOP_CFLAGS) $(GCONF_CFLAGS) $(XML2_CFLAGS) $(JSON_CFLAGS)

# Binary
bin_PROGRAMS = \
        conboy
		
#pluginlib_LTLIBRARIES = \
#		libconboy_applet.la
# /Binary

# Sources
conboy_SOURCES = \
        main.c \
        interface.h \
        interface.c \
        callbacks.h \
        callbacks.c \
        localisation.h \
        serializer.h \
        serializer.c \
        deserializer.h \
        deserializer.c \
        metadata.h \
        metadata.c \
        note.h \
        note.c \
        search_window.h \
        search_window.c \
        note_list_store.c \
        note_list_store.h \
        note_linker.h \
        note_linker.c \
        app_data.h \
        app_data.c \
        json.h \
        json.c

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
        $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(HILDONDESKTOP_LIBS) $(XML2_LIBS) $(JSON_LIBS)

#libconboy_applet_la_LIBADD = \
#		$(HILDONDESKTOP_LIBS)
# /LDADD