# List extras to package
EXTRA_DIST = \
        autogen.sh \
        debian/postinst \
        debian/copyright \
        debian/control \
        debian/rules \
        debian/changelog \
        debian/postinst

# Sub directories
SUBDIRS = src data po

#deb: dist
#	-mkdir $(top_builddir)/debian-build
#	cd $(top_builddir)/debian-build && tar zxf ../$(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
#	cd $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION) && dpkg-buildpackage -rfakeroot
#	rm -rf $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION)
