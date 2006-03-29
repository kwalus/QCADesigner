#!/bin/bash

set -x

#Ask for the appropriate automake/autoconf versions
# WANT_AUTOMAKE=1.6
# WANT_AUTOCONF_2_5=1

# export WANT_AUTOMAKE
# export WANT_AUTOCONF_2_5

set +x

# Hacky test for Windoze follows :o)
if test "" != "$WINDIR" ; then
  echo '########################################################################################'
  echo 'IMPORTANT: Under cygwin, you must set the following environment variables:'
  echo ''
  echo 'LDFLAGS="-L/lib/mingw -Xlinker -subsys -Xlinker windows"'
  echo 'CFLAGS="-mno-cygwin -mms-bitfields -g"'
  echo 'CXXFLAGS=${CFLAGS}'
  echo ''
  echo 'If you have not done so, you had better rm -rf the source tree and check it out again'
  echo 'because, for some reason, you cannot run autogen.sh twice on the same source tree'
  echo '########################################################################################'

  if [ "" = "$GTK_SOURCES" ]; then
    echo "The environment variable GTK_SOURCES is not defined. Please enter the location of your GTK+ development environment (see http://www.gimp.org/~tml/gimp/win32/downloads.html):"
    read
  fi
  ACLOCAL_FLAGS="${ACLOCAL_FLAGS} ${GTK_SOURCES}/share/aclocal"
  export ACLOCAL_FLAGS
# Test for Fink if Darwin
elif [ "Darwin" = "$(uname)" ]; then
  if ![ -d /sw/share/aclocal ]; then
    echo "Couldn't find directory /sw/share/aclocal. This probably means that you don't have Fink installed, or that it is installed improperly. Please download Fink from http://fink.sourceforge.net/ and re-run this script afterwards."
  else
    ACLOCAL_FLAGS="${ACLOCAL_FLAGS} -I /sw/share/aclocal"
    export ACLOCAL_FLAGS
  fi
fi

set -x

aclocal ${ACLOCAL_FLAGS}
automake --gnu --add-missing
glib-gettextize -c
autoconf
