AC_INIT([conboy], [0.6.3.32], [https://bugs.maemo.org/enter_bug.cgi?product=conboy], [conboy])
#AC_CONFIG_AUX_DIR([build])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_SRCDIR([src/conboy_note.c])

AC_ARG_VAR([ACLOCAL_FLAGS], [aclocal flags, for example -I <macro dir>])
AM_INIT_AUTOMAKE([-Wall foreign no-define nostdinc subdir-objects])

AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_INSTALL
#LT_INIT
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PREFIX_DEFAULT([/usr])

PKG_CHECK_EXISTS([osso-af-settings >= 0.8.4])
PKG_CHECK_EXISTS([hildon-1 >= 2.1.62], [AC_DEFINE([HILDON_HAS_APP_MENU], [1], [Does hildon app menu exist])])

# If glib >= 2.14, pcre support is builtin. If not we need the libpcre3 package
PKG_CHECK_EXISTS([glib-2.0 >= 2.14.0], [AC_DEFINE([GLIB_HAS_PCRE], [1], [Does glib have support for perl-compatible regular expressions])])
PKG_CHECK_EXISTS([glib-2.0 < 2.14.0], [COMMON_DEPS="$COMMON_DEPS libpcre >= 6.7"])

PKG_CHECK_EXISTS([libmodest-dbus-client-1.0 >= 1.0], [
	AC_DEFINE([WITH_MODEST], [1], [Does have Modest support])
	COMMON_DEPS="$COMMON_DEPS libmodest-dbus-client-1.0 >= 1.0"])

PKG_CHECK_EXISTS([conbtdialogs >= 1.4], [
	AC_DEFINE([WITH_BT], [1], [Does have bluetooth dialogs])
	COMMON_DEPS="$COMMON_DEPS conbtdialogs >= 1.4"])
	
PKG_CHECK_EXISTS([sharingdialog >= 1.1], [
	AC_DEFINE([WITH_SHARING], [1], [Does have sharing dialog])
	COMMON_DEPS="$COMMON_DEPS sharingdialog >= 1.1"])
	
PKG_CHECK_EXISTS([hildon-extras-1 >= 0.9], [
	AC_DEFINE([WITH_HE], [1], [Does have hildon-extras])
	COMMON_DEPS="$COMMON_DEPS hildon-extras-1 >= 0.9"])

# Find out maemo version
PKG_CHECK_EXISTS([maemo-version], [
  MAEMO_VERSION=`pkg-config --modversion maemo-version`
  AC_MSG_NOTICE([Configuring for Maemo $MAEMO_VERSION])
  EXTRA_CPPFLAGS="$CFLAGS -DMAEMO_VERSION=\\\"$MAEMO_VERSION\\\""
  MAJOR=`echo $MAEMO_VERSION | cut -b1 -`
  EXTRAS_CPPFLAGS="$CFLAGS -DMAEMO_VERSION_MAJOR=$MAJOR"
], [
  AC_MSG_WARN([maemo-version not found])
])

# Warning flags and defines
EXTRA_CPPFLAGS="-Wall -Wmissing-prototypes -Wmissing-declarations"

# Fremantle uses different categories for backup then Diablo
AS_IF([test "$MAJOR" == "5"], [BACKUP_CATEGORY=other], [BACKUP_CATEGORY=documents])
AC_MSG_NOTICE([Using backup category "$BACKUP_CATEGORY"])
AC_SUBST([BACKUP_CATEGORY])

# Midgard
AS_IF([test "$MAJOR" = "5"],
	[
	AC_MSG_CHECKING([whether to build Midgard storage plugin])
	AC_ARG_WITH([midgard],[AS_HELP_STRING([--with-midgard], [enable Midgard storage])],
	[], [with_midgard=yes])
	AS_IF([test "$with_midgard" = yes],
		[
		AC_MSG_RESULT([yes])
		MIDGARD_DEPS="midgard2 >= 9.09"
		AC_DEFINE([WITH_MIDGARD], [1], [Does have Midgard support])
		],
		[AC_MSG_RESULT([no])]
	)
	]
)

AM_CONDITIONAL([MIDGARD], [test "x$with_midgard" = xyes])

# Support for maemo-launcher
AC_MSG_CHECKING([whether to build with maemo-launcher support])
AC_ARG_ENABLE([maemo-launcher],
	[AS_HELP_STRING([--enable-maemo-launcher],
		[build with maemo-launcher support])],
	[AS_CASE([${enableval}],
		[yes], [COMMON_DEPS="$COMMON_DEPS maemo-launcher-app"
			AC_MSG_RESULT([yes])],
		[no], [AC_MSG_RESULT([no])],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-maemo-launcher])])])

PLUGIN_DEPS="hildon-1 >= 0.8.4 libosso >= 0.8.4 gconf-2.0 >= 0.22"
STORAGE_XML_DEPS="libxml-2.0 >= 2.6.0 json-glib-1.0 >= 0.6.2"
COMMON_DEPS="$COMMON_DEPS $PLUGIN_DEPS $STORAGE_XML_DEPS gmodule-2.0 libhildonmime >= 1.0.0 libcurl >= 7.15.0 oauth >= 0.5.2 libxslt >= 1.1"
PKG_CHECK_MODULES([DEPS], [$COMMON_DEPS])
PKG_CHECK_MODULES([STORAGE_EVERNOTE], [$PLUGIN_DEPS])
PKG_CHECK_MODULES([STORAGE_MIDGARD], [$PLUGIN_DEPS $MIDGARD_DEPS])
PKG_CHECK_MODULES([STORAGE_XML], [$PLUGIN_DEPS $STORAGE_XML_DEPS])

# Localization-related
AC_PROG_INTLTOOL([0.23])
AC_SUBST([GETTEXT_PACKAGE], [$PACKAGE_NAME])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of gettext package])
AC_SUBST([ALL_LINGUAS], ["de en_GB fr it pt_BR pl fi da nl sk sv ro tr hu ru nb cs ca"])
AM_GLIB_GNU_GETTEXT
AC_SUBST([localedir], [`$PKG_CONFIG osso-af-settings --variable=localedir`])
AC_DEFINE_UNQUOTED([LOCALEDIR], ["$localedir"], [localedir])

AC_CONFIG_FILES([
    Makefile 
    po/Makefile.in 
    po/Makefile
    data/de.zwong.conboy.service
    data/conboy2.desktop
    data/conboy.conf
    src/conboy_config.h
])
AC_OUTPUT
