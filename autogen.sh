#!/bin/bash

set -x

#Ask for the appropriate automake/autoconf versions
WANT_AUTOMAKE=1.6
WANT_AUTOCONF_2_5=1
                                                                                                                                                              
export WANT_AUTOMAKE
export WANT_AUTOCONF_2_5

HAVE_WINDOWS=0

set +x

# Hacky test for Windoze follows :o)
if [ "" != "$WINDIR" ]; then
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

  HAVE_WINDOWS=1
  if [ "" = "$GTK_SOURCES" ]; then
    echo "The environment variable GTK_SOURCES is not defined. Please enter the location of your GTK+ development environment (see http://www.gimp.org/~tml/gimp/win32/downloads.html):"
    read
    GTK_SOURCES="$REPLY"
    export GTK_SOURCES
  fi
fi

set -x

if [ "1" = "$HAVE_WINDOWS" ]; then
  aclocal -I $GTK_SOURCES/share/aclocal
else
  aclocal
fi
automake --gnu --add-missing
glib-gettextize -c
autoconf
