dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/main.c)
AM_CONFIG_HEADER(config.h)

PACKAGE=conboy
VERSION=0.6.3.2

AM_INIT_AUTOMAKE($PACKAGE,$VERSION)

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PREFIX_DEFAULT(/usr)

# AC_PREFIX_DEFAULT does not get expanded until too late so we need to do this to use prefix in this script
if test "x$prefix" = "xNONE" ; then
    prefix='/usr'
fi

CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wmissing-declarations"

PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
AC_SUBST(GMODULE_LIBS)
AC_SUBST(GMODULE_CFLAGS)

PKG_CHECK_MODULES(HILDON, hildon-1 >= 0.8.4)  
AC_SUBST(HILDON_LIBS)
AC_SUBST(HILDON_CFLAGS)

PKG_CHECK_MODULES(HILDON_MIME, libhildonmime >= 1.0.0)
AC_SUBST(HILDON_MIME_LIBS)
AC_SUBST(HILDON_MIME_CFLAGS)

PKG_CHECK_EXISTS([hildon-1 >= 2.1.62], [AC_DEFINE([HILDON_HAS_APP_MENU], [1], [Does hildon app menu exist])])

# If glib >= 2.14 it has pcre support build in. If not we need the libpcre3 package
PKG_CHECK_EXISTS([glib-2.0 >= 2.14.0], [AC_DEFINE([GLIB_HAS_PCRE], [1], [Does glib have support for pearl regular expressions])])
PKG_CHECK_EXISTS([glib-2.0 < 2.14.0], [ 
	PKG_CHECK_MODULES([PCRE], [libpcre >= 6.7])  
	AC_SUBST(PCRE_LIBS)
	AC_SUBST(PCRE_CFLAGS)
])

PKG_CHECK_EXISTS([libmodest-dbus-client-1.0 >= 1.0], [
	AC_DEFINE([WITH_MODEST], [1], [Does have Modest support])
	PKG_CHECK_MODULES(MODEST, libmodest-dbus-client-1.0 >= 1.0)
	AC_SUBST(MODEST_LIBS)
	AC_SUBST(MODEST_CFLAGS)
])

PKG_CHECK_MODULES(OSSO, libosso >= 0.8.4)
AC_SUBST(OSSO_LIBS)
AC_SUBST(OSSO_CFLAGS)

PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 0.22)
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

PKG_CHECK_MODULES(XML2, libxml-2.0 >= 2.6.0)
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBS)

PKG_CHECK_MODULES(JSON, json-glib-1.0 >= 0.6.2)
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

PKG_CHECK_MODULES(CURL, libcurl >= 7.15.0)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

PKG_CHECK_MODULES(OAUTH, oauth >= 0.5.2)
AC_SUBST(OAUTH_CFLAGS)
AC_SUBST(OAUTH_LIBS)

PKG_CHECK_MODULES(OSSOSETTINGS, osso-af-settings >= 0.8.4)

# Find out maemo version
PKG_CHECK_EXISTS(maemo-version, [
  VERSION=`pkg-config --modversion maemo-version`
  AC_MSG_NOTICE([Configuring for Maemo $VERSION])
  CFLAGS="$CFLAGS -DMAEMO_VERSION=\\\"$VERSION\\\""
  MAJOR=`echo $VERSION | cut -b1 -`
  CFLAGS="$CFLAGS -DMAEMO_VERSION_MAJOR=$MAJOR"
], [
  AC_MSG_WARN([maemo-version not found])
])

# Fremantle uses different categories for backup then Diablo
if [[ "$MAJOR" == "5" ]]; then
  BACKUP_CATEGORY=other
else
  BACKUP_CATEGORY=documents
fi

AC_MSG_NOTICE([Using backup category "$BACKUP_CATEGORY"])
AC_SUBST(BACKUP_CATEGORY)

if [[ "$MAJOR" == "5" ]]; then

  AC_MSG_CHECKING([whether to build Midgard storage plugin])
  AC_ARG_WITH([midgard],[AS_HELP_STRING([--with-midgard], [enable Midgard storage])],
              [], [with_midgard=yes])
  AS_IF(
          [test "$with_midgard" = yes],
          [
          AC_MSG_RESULT([yes])
          PKG_CHECK_MODULES(MIDGARD, midgard2 >= 9.09)
          AC_DEFINE([WITH_MIDGARD], [1], [Does have Midgard support])
          ],
          AC_MSG_RESULT([no])
  )
fi

AM_CONDITIONAL([MIDGARD], [test x$with_midgard = xyes])



# Application icon install directories
icon_26x26dir=$datadir/icons/hicolor/26x26/hildon
icon_40x40dir=$datadir/icons/hicolor/40x40/hildon
icon_48x48dir=$datadir/icons/hicolor/48x48/hildon
icon_64x64dir=$datadir/icons/hicolor/64x64/hildon
icon_scalabledir=$datadir/icons/hicolor/scalable/hildon

localedir=`$PKG_CONFIG osso-af-settings --variable=localedir`

# Plugin dirs (TODO: should not be hardcoded to /usr/)
pluginslibdir=$prefix/lib/conboy
AC_MSG_NOTICE([Using "$pluginslibdir" for plugins])
AC_SUBST(pluginslibdir)

# Localization-related
AC_PROG_INTLTOOL([0.23])
GETTEXT_PACKAGE=conboy
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED( GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of gettext package ])
ALL_LINGUAS="de fr it pt_BR pl fi da nl sk sv ro tr hu ru"
AM_GLIB_GNU_GETTEXT
AC_SUBST(ALL_LINGUAS)
AC_SUBST(localedir)
AC_DEFINE_UNQUOTED(LOCALEDIR,"$localedir",[localedir])

AC_SUBST(desktopentrydir)
AC_SUBST(icon_26x26dir)
AC_SUBST(icon_40x40dir)
AC_SUBST(icon_48x48dir)
AC_SUBST(icon_64x64dir)
AC_SUBST(icon_scalabledir)

AC_CONFIG_FILES([src/conboy_config.h])

AC_OUTPUT([
    Makefile 
    src/Makefile
    data/Makefile
    po/Makefile.in 
    po/Makefile
    data/de.zwong.conboy.service
    data/conboy2.desktop
    data/conboy.conf
    src/plugins/Makefile
    src/plugins/storage_xml/Makefile
    src/plugins/storage_midgard/Makefile
    src/plugins/storage_evernote/Makefile
])
