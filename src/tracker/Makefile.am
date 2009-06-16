# Includes
INCLUDES = $(TRACKER_CFLAGS) -DG_LOG_DOMAIN=\"Tracker\"

# Binary
pluginlib_LTLIBRARIES = \
    libtracker-module-tomboy-notes.la
# /Binary

# Sources
libtracker_module_tomboy_notes_la_SOURCES = \
    tracker-module.h \
    tracker-module-file.h \
    tracker-module-iterable.h \
    tracker-module-metadata.h \
    tracker-module-metadata-utils.h \
    tomboy-notes.c
# /Sources

# LDFLAGS
libtracker_module_tomboy_notes_la_LDFLAGS = -module -avoid-version -no-undefined 
# /LDFLAGS

# LDADD
libtracker_module_tomboy_notes_la_LIBADD = \
    $(TRACKER_LIBS)
#/LDADD

