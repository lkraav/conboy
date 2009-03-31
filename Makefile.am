# List extras to package
EXTRA_DIST = \
        autogen.sh \
        debian/postinst \
        debian/copyright \
        debian/control \
        debian/rules \
        debian/changelog \
        debian/postinst \
        src/*

# Sub directories
SUBDIRS = data po

# Includes
INCLUDES = $(DEPS_CFLAGS) $(HILDON_CFLAGS) $(OSSO_CFLAGS) $(HILDONFM_CFLAGS) $(XML2_CFLAGS) $(HILDONDESKTOP_CFLAGS) $(GCONF_CFLAGS)

# Binary
bin_PROGRAMS = \
        conboy
		
pluginlib_LTLIBRARIES = \
		libconboy_applet.la
# /Binary

# Sources
conboy_SOURCES = \
        src/main.c \
        src/interface.h \
        src/interface.c \
        src/callbacks.h \
        src/callbacks.c \
        src/support.h \
        src/support.c \
        src/localisation.h \
        src/serializer.h \
        src/serializer.c \
        src/deserializer.h \
        src/deserializer.c \
        src/metadata.h \
        src/metadata.c \
        src/note.h \
        src/note.c

libconboy_applet_la_SOURCES = \
		src/conboy_applet.c \
		src/conboy_applet.h \
		src/metadata.h \
        src/metadata.cs
		
# /Sources

# XXX
libconboy_applet_la_LDFLAGS = -module -avoid-version 
# /XXX

# LDADD
conboy_LDADD = \
        $(DEPS_LIBS) $(HILDON_LIBS) $(OSSO_LIBS) $(GCONF_LIBS) $(HILDONDESKTOP_LIBS)

libconboy_applet_la_LIBADD = \
		$(DEPS_LIBS) $(HILDONDESKTOP_LIBS)
# /LDADD

deb: dist
	-mkdir $(top_builddir)/debian-build
	cd $(top_builddir)/debian-build && tar zxf ../$(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	cd $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION) && dpkg-buildpackage -rfakeroot
	rm -rf $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION)
