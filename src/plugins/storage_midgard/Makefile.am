
plugindir = $(pluginslibdir)

INCLUDES = \
    -I$(top_srcdir)  \
    $(HILDON_CFLAGS) \
    $(CONBOY_CFLAGS)
           
plugin_LTLIBRARIES = libstoragemidgard.la

libstoragemidgard_la_SOURCES = \
    conboy_storage_midgard.h  \
    conboy_storage_midgard.c

#TODO: Reduce number of libs most of them are not needed
libstoragemidgard_la_LIBADD = \
        $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(HILDONDESKTOP_LIBS) $(XML2_LIBS) $(JSON_LIBS)

plugin_in_files = conboy_storage_midgard.plugin.in

%.plugin: %.plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.in=.plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

