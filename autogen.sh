#!/bin/sh

set -x
glib-gettextize --copy --force
libtoolize --automake
intltoolize --copy --force --automake

aclocal-1.8
autoconf
autoheader
automake-1.8 --add-missing --foreign
./configure --enable-maintainer-mode
