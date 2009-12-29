
plugindir = $(pluginslibdir)

INCLUDES = \
    -I$(top_srcdir)  \
    $(HILDON_CFLAGS) \
    $(CONBOY_CFLAGS) \
    $(XML2_CFLAGS) \
    $(GCONF_CFLAGS)
           
plugin_LTLIBRARIES = libstoragexml.la

libstoragexml_la_SOURCES = \
    conboy_xml_storage_plugin.h  \
    conboy_xml_storage_plugin.c

#TODO: Reduce number of libs most of them are not needed
libstoragexml_la_LIBADD = \
        $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(HILDONDESKTOP_LIBS) $(XML2_LIBS) $(JSON_LIBS)

libstoragexml_la_LDFLAGS = -module -avoid-version

plugin_in_files = conboy_storage_xml.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

