#!/bin/sh

set -x
glib-gettextize --copy --force
libtoolize --copy --force --automake
intltoolize --copy --force --automake

aclocal-1.9
autoconf
autoheader
automake-1.9 --add-missing --foreign
./configure --enable-maintainer-mode
