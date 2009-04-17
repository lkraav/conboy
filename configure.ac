dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/main.c)
AM_CONFIG_HEADER(config.h)

PACKAGE=conboy
VERSION=0.2.0

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

PKG_CHECK_MODULES(OSSO, libosso >= 0.8.4 gnome-vfs-2.0 >= 2.2 hildon-help)
AC_SUBST(OSSO_LIBS)
AC_SUBST(OSSO_CFLAGS)

PKG_CHECK_MODULES(HILDONFM, hildon-fm-2 >= 0.8.4)
AC_SUBST(HILDONFM_LIBS)
AC_SUBST(HILDONFM_CFLAGS)

PKG_CHECK_MODULES(HILDONDESKTOP, libhildondesktop >= 2.0.0)
AC_SUBST(HILDONDESKTOP_LIBS)
AC_SUBST(HILDONDESKTOP_CFLAGS)

PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 0.22)
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

PKG_CHECK_MODULES(XML2, libxml-2.0 >= 2.6.0)
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBS)

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




# Localization-related
dnl AC_PROG_INTLTOOL([0.23])
dnl GETTEXT_PACKAGE=conboy
dnl AC_SUBST( GETTEXT_PACKAGE )
dnl AC_DEFINE_UNQUOTED( GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of gettext package ])
dnl ALL_LINGUAS="de en_GB en_US es fi fr it pt_BR ru"
dnl AC_SUBST(ALL_LINGUAS)
dnl AM_GLIB_GNU_GETTEXT
USE_NLS=no
AC_SUBST(USE_NLS)

AC_SUBST(localedir)
AC_DEFINE_UNQUOTED(localedir,"$localedir",[localedir])
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

#    po/Makefile.in
AC_OUTPUT([
    Makefile 
    src/Makefile
    data/Makefile 
    po/Makefile
    data/de.zwong.conboy.service
    data/conboy.desktop
])
