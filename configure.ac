dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/main.c)
AM_CONFIG_HEADER(config.h)

PACKAGE=conboy
VERSION=0.4.5

AM_INIT_AUTOMAKE($PACKAGE,$VERSION)

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_INSTALL
dnl AC_PREFIX_DEFAULT(/var/lib/install/usr)
AC_PREFIX_DEFAULT(/usr)

CFLAGS="$CFLAGS -Wall -ansi -pedantic -Wmissing-prototypes -Wmissing-declarations"

PKG_CHECK_MODULES(HILDON, hildon-1 >= 0.8.4)  
AC_SUBST(HILDON_LIBS)
AC_SUBST(HILDON_CFLAGS)

PKG_CHECK_EXISTS([hildon-1 >= 2.1.62], [AC_DEFINE([HILDON_HAS_APP_MENU], [1], [Does hildon app menu exist])])

PKG_CHECK_MODULES(OSSO, libosso >= 0.8.4)
AC_SUBST(OSSO_LIBS)
AC_SUBST(OSSO_CFLAGS)

dnl PKG_CHECK_MODULES(HILDONDESKTOP, libhildondesktop >= 2.0.0)
dnl AC_SUBST(HILDONDESKTOP_LIBS)
dnl AC_SUBST(HILDONDESKTOP_CFLAGS)

PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 0.22)
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

PKG_CHECK_MODULES(XML2, libxml-2.0 >= 2.6.0)
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBS)

PKG_CHECK_MODULES(JSON, json-glib-1.0 >= 0.6)
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

PKG_CHECK_EXISTS([tracker >= 0.6.90], [PKG_CHECK_MODULES([TRACKER], [tracker >= 0.6.90])])
AC_SUBST(TRACKER_CFLAGS)
AC_SUBST(TRACKER_LIBS)

PKG_CHECK_MODULES(OSSOSETTINGS, osso-af-settings >= 0.8.4)

# Application icon install directories
icon_26x26dir=$datadir/icons/hicolor/26x26/hildon
icon_34x34dir=$datadir/icons/hicolor/34x34/hildon
icon_40x40dir=$datadir/icons/hicolor/40x40/hildon
icon_50x50dir=$datadir/icons/hicolor/50x50/hildon
icon_scalabledir=$datadir/icons/hicolor/scalable/hildon

localedir=`$PKG_CONFIG osso-af-settings --variable=localedir`
hildondesktopentrydir=/usr/share/dbus-1/services
hildoncpdesktopentrydir=/usr/
hildonstatusbarplugindir=`$PKG_CONFIG hildon-status-bar-lib --variable=pluginlibdir`
hildoncplibdir=`$PKG_CONFIG hildon-control-panel --variable=pluginlibdir`
hildoniconthemedir=$HOME/.icons/hicolor

dnl For the navigation bar plug-in

AC_PROG_LIBTOOL

pluginlibdir=`pkg-config osso-af-settings --variable=hildondesktoplibdir`
AC_SUBST(pluginlibdir)

dnl
dnl Tracker dirs
dnl

trackerlibdir=/usr/lib/tracker/indexer-modules
AC_SUBST(trackerlibdir)

trackermoduledir=/usr/share/tracker/modules
AC_SUBST(trackermoduledir)

trackerservicedir=/usr/share/tracker/services
AC_SUBST(trackerservicedir)


# Localization-related
AC_PROG_INTLTOOL([0.23])
GETTEXT_PACKAGE=conboy
AC_SUBST( GETTEXT_PACKAGE )
AC_DEFINE_UNQUOTED( GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of gettext package ])
ALL_LINGUAS="de fr"
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
AC_SUBST(localedir)
AC_DEFINE_UNQUOTED(LOCALEDIR,"$localedir",[localedir])

AC_SUBST(desktopentrydir)
AC_SUBST(hildondesktopentrydir)
AC_SUBST(hildoncpdesktopentrydir)
AC_SUBST(hildonstatusbarplugindir)
AC_SUBST(hildoncplibdir)
AC_SUBST(hildoniconthemedir)
AC_SUBST(icon_26x26dir)
AC_SUBST(icon_34x34dir)
AC_SUBST(icon_40x40dir)
AC_SUBST(icon_50x50dir)
AC_SUBST(icon_scalabledir)


AC_OUTPUT([
    Makefile 
    src/Makefile
    src/tracker/Makefile
    data/Makefile
    po/Makefile.in 
    po/Makefile
    data/de.zwong.conboy.service
    data/conboy.desktop
])
