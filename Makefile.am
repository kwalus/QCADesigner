## Process this file with automake to produce Makefile.in

docdir = $(datadir)/doc/@PACKAGE@-@VERSION@
appdir = $(datadir)/applications

SUBDIRS = src po pixmaps docs

doc_DATA = \
  AUTHORS \
  COPYING \
  ChangeLog \
  README \
  README.win32.txt

app_DATA = QCADesigner.desktop

EXTRA_DIST = $(doc_DATA) $(app_DATA)
