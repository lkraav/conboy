
plugindir = $(pluginslibdir)

INCLUDES = \
    -I$(top_srcdir)  \
    $(HILDON_CFLAGS) \
    $(CONBOY_CFLAGS) \
    $(MIDGARD_CFLAGS) 
           
plugin_LTLIBRARIES = libstorageevernote.la

libstorageevernote_la_SOURCES = \
    conboy_evernote_storage_plugin.h  \
    conboy_evernote_storage_plugin.c

#TODO: Reduce number of libs most of them are not needed
libstorageevernote_la_LIBADD = \
        $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(evernote_LIBS)

libstorageevernote_la_LDFLAGS = -module -avoid-version

plugin_in_files = conboy_storage_evernote.plugin.in

%.plugin: %.plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.in=.plugin)

##nobase_dist_data_DATA = evernote-2.0/schema/org_gnome_tomboy.xml

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

