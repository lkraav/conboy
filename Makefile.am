ACLOCAL_AMFLAGS = -I macros ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS = --enable-maemo-launcher --with-midgard

bin_PROGRAMS = conboy

conboy_SOURCES = \
	src/localisation.h \
	src/main.c \
	src/interface.h \
	src/interface.c \
	src/callbacks.h \
	src/callbacks.c \
	src/localisation.h \
	src/metadata.h \
	src/metadata.c \
	src/note.h \
	src/note.c \
	src/search_window.h \
	src/search_window.c \
	src/conboy_note_store.c \
	src/conboy_note_store.h \
	src/note_linker.h \
	src/note_linker.c \
	src/app_data.h \
	src/app_data.c \
	src/json.h \
	src/json.c \
	src/settings.h \
	src/settings.c \
	src/settings_window.h \
	src/settings_window.c \
	src/search.h \
	src/search.c \
	src/conboy_note.h \
	src/conboy_note.c \
	src/conboy_oauth.h \
	src/conboy_oauth.c \
	src/conboy_plugin.h \
	src/conboy_plugin.c \
	src/conboy_storage_plugin.h \
	src/conboy_storage_plugin.c \
	src/conboy_plugin_info.h \
	src/conboy_plugin_info.c \
	src/conboy_plugin_manager.h \
	src/conboy_plugin_manager.c \
	src/conboy_storage.h \
	src/conboy_storage.c \
	src/conboy_note_buffer.h \
	src/conboy_note_buffer.c \
	src/conboy_plugin_store.h \
	src/conboy_plugin_store.c \
	src/he-check-button.h \
	src/he-check-button.c \
	src/orientation.h \
	src/orientation.c \
	src/he-fullscreen-button.h \
	src/he-fullscreen-button.c \
	src/ui_helper.h \
	src/ui_helper.c \
	src/conboy_plugin_manager_row.h \
	src/conboy_plugin_manager_row.c \
	src/gregex.h \
	src/gregex.c \
	src/conboy_xml.h \
	src/conboy_xml.c \
	src/he-about-dialog.h \
	src/he-about-dialog.c \
	src/conboy_http.h \
	src/conboy_http.c \
	src/conboy_web_sync.h \
	src/conboy_web_sync.c \
	src/extra_strings.h \
	src/sharing.h \
	src/sharing.c
conboy_CPPFLAGS = $(DEPS_CFLAGS) $(EXTRAS_CPPFLAGS) \
	-I$(top_srcdir)/src -I$(top_builddir) -I$(top_builddir)/src
conboy_LDADD = $(DEPS_LIBS)

plugindir = $(pkglibdir)
plugin_LTLIBRARIES = \
	src/plugins/storage_evernote/libstorageevernote.la \
	src/plugins/storage_midgard/libstoragemidgard.la \
	src/plugins/storage_xml/libstoragexml.la
nodist_plugin_DATA = \
	src/plugins/storage_evernote/conboy_storage_evernote.plugin \
	src/plugins/storage_xml/conboy_storage_xml.plugin
dist_plugin_DATA = \
	src/plugins/storage_evernote/conboy_storage_evernote.plugin.desktop.in \
	src/plugins/storage_midgard/conboy_storage_midgard.plugin.desktop.in \
	src/plugins/storage_xml/conboy_storage_xml.plugin.desktop.in

if MIDGARD
nodist_plugin_DATA += \
	src/plugins/storage_midgard/conboy_storage_midgard.plugin

midgardschemadir = $(datadir)/midgard2/schema
dist_midgardschema_DATA = data/org_gnome_tomboy.xml

src_plugins_storage_midgard_libstoragemidgard_la_SOURCES = \
	src/plugins/storage_midgard/conboy_midgard_storage_plugin.h \
	src/plugins/storage_midgard/conboy_midgard_storage_plugin.c
src_plugins_storage_midgard_libstoragemidgard_la_CPPFLAGS = \
	$(STORAGE_MIDGARD_CFLAGS) $(EXTRA_CPPFLAGS)
src_plugins_storage_midgard_libstoragemidgard_la_LIBADD = \
	$(STORAGE_MIDGARD_LIBS)
src_plugins_storage_midgard_libstoragemidgard_la_LDFLAGS = \
	-module -avoid-version
endif

CLEANFILES = $(nodist_plugin_DATA)

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

src_plugins_storage_evernote_libstorageevernote_la_SOURCES = \
	src/plugins/storage_evernote/conboy_evernote_storage_plugin.h \
	src/plugins/storage_evernote/conboy_evernote_storage_plugin.c
src_plugins_storage_evernote_libstorageevernote_la_CPPFLAGS = \
	$(STORAGE_EVERNOTE_CFLAGS) $(EXTRA_CPPFLAGS)
src_plugins_storage_evernote_libstorageevernote_la_LIBADD = \
	$(STORAGE_EVERNOTE_LIBS)
src_plugins_storage_evernote_libstorageevernote_la_LDFLAGS = \
	-module -avoid-version

src_plugins_storage_xml_libstoragexml_la_SOURCES = \
	src/plugins/storage_xml/conboy_xml_storage_plugin.h \
	src/plugins/storage_xml/conboy_xml_storage_plugin.c
src_plugins_storage_xml_libstoragexml_la_CPPFLAGS = \
	$(STORAGE_XML_CFLAGS) $(EXTRA_CPPFLAGS) -I$(top_builddir)
src_plugins_storage_xml_libstoragexml_la_LIBADD = $(STORAGE_XML_LIBS)
src_plugins_storage_xml_libstoragexml_la_LDFLAGS = -module -avoid-version

dbusdir = $(datadir)/dbus-1/services
nodist_dbus_DATA = data/de.zwong.conboy.service

desktopdir = $(datadir)/applications/hildon
nodist_desktop_DATA = data/conboy2.desktop

#miscdir = $(datadir)/conboy
#dist_misc_DATA = data/conboy_to_html.xsl
stuffdir = $(pkgdatadir)
dist_stuff_DATA = data/conboy_to_html.xsl

# Icons
icon_26x26dir=$(datadir)/icons/hicolor/26x26/hildon
icon_40x40dir=$(datadir)/icons/hicolor/40x40/hildon
icon_48x48dir=$(datadir)/icons/hicolor/48x48/hildon
icon_64x64dir=$(datadir)/icons/hicolor/64x64/hildon
icon_scalabledir=$(datadir)/icons/hicolor/scalable/hildon

dist_icon_26x26_DATA = data/icons/26x26/conboy.png
dist_icon_40x40_DATA = data/icons/40x40/conboy.png
dist_icon_48x48_DATA = data/icons/48x48/conboy.png \
	data/icons/48x48/conboy-decrease_indent.png \
	data/icons/48x48/conboy-hyperlink.png \
	data/icons/48x48/conboy-increase_indent.png \
	data/icons/48x48/conboy-text_transform.png
dist_icon_64x64_DATA = data/icons/64x64/conboy.png
dist_icon_scalable_DATA = data/icons/scalable/conboy.png

backupdir = /etc/osso-backup/applications
nodist_backup_DATA = data/conboy.conf

# List extras to package
dist_noinst_SCRIPTS = \
	autogen.sh \
	debian/modify_control.sh \
	debian/rules

dist_noinst_DATA = \
	debian/changelog \
	debian/conboy.install \
	debian/conboy-midgard.install \
	debian/control \
	debian/control_midgard \
	debian/copyright \
	debian/launcher \
	debian/optify \
	debian/postinst \
	debian/postrm

# Sub directories
SUBDIRS = po

deb: distcheck
	-mkdir $(top_builddir)/debian-build
	cd $(top_builddir)/debian-build && tar -zxf ../$(top_builddir)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
	cd $(top_builddir)/debian-build/$(PACKAGE_NAME)-$(PACKAGE_VERSION) && dpkg-buildpackage -rfakeroot
	rm -rf $(top_builddir)/debian-build/$(PACKAGE_NAME)-$(PACKAGE_VERSION)


