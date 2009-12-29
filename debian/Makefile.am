
debiandir = debian

debian_in_files = control.rfc822.in

%: %.rfc822.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -r -u -c $(top_builddir)/po/.intltool-merge-cache

debian_DATA = $(debian_in_files:.rfc822.in=)

EXTRA_DIST = $(debian_in_files)

CLEANFILES = $(debian_DATA)
DISTCLEANFILES = $(debian_DATA)

