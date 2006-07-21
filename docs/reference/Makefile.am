## Process this file with automake to produce Makefile.in

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# This is a blank Makefile.am for using gtk-doc.
# Copy this to your project's API docs directory and modify the variables to
# suit your project. See the GTK+ Makefiles in gtk+/docs/reference for examples
# of using the various options.

# The name of the module, e.g. 'glib'.
DOC_MODULE=QCADesigner

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
# e.g. DOC_SOURCE_DIR=../../../gtk
DOC_SOURCE_DIR=../../src

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=--type-init-func=MY_INIT_FUNC

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED" 
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
# e.g. MKDB_OPTIONS=--sgml-mode --output-format=xml
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS=

# Used for dependencies. The docs will be rebuilt if any of these change.
# e.g. HFILE_GLOB=$(top_srcdir)/gtk/*.h
# e.g. CFILE_GLOB=$(top_srcdir)/gtk/*.c
HFILE_GLOB=$(DOC_SOURCE_DIR)/objects/*.h
CFILE_GLOB=$(DOC_SOURCE_DIR)/objects/*.c

# Header files to ignore when scanning.
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h
IGNORE_HFILES=intl.h

# Images to copy into HTML directory.
# e.g. HTML_IMAGES=$(top_srcdir)/gtk/stock-icons/stock_about_24.png
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
# e.g. INCLUDES=-I$(top_srcdir) -I$(top_builddir) $(GTK_DEBUG_FLAGS)
# e.g. GTKDOC_LIBS=$(top_builddir)/gtk/$(gtktargetlib)
INCLUDES=$(GTK_CFLAGS) $(GTHREAD_CFLAGS) -DGTK_GUI -DUNDO_REDO -DDESIGNER -DFILEIO_STDIO -DPROPERTY_UIS -DGTK_DOC
GTKDOC_LIBS=$(GTK_LIBS) $(GTHREAD_LIBS) $(DOC_SOURCE_DIR)/libgtkdocfiles.a $(shell bash -c 'find ../../src | grep QCADesigner-.\*\\.o | \
        grep -v QCADesigner-main\\.o | \
        grep -v QCADesigner-QCADComboBox\\.o | \
        grep -v QCADesigner-QCADRadioButton\\.o | \
        grep -v QCADesigner-QCADRadioToolButton\\.o | \
        grep -v QCADesigner-QCADToggleToolButton\\.o | \
        grep -v QCADesigner-QCADCellRendererText\\.o')

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST += 

GNOME_DEV_HOME = http://developer.gnome.org/doc/API/2.0

# Things to do to make the gtk-doc-generated HTML Web-friendly:
# 1. Replace all hrefs of the form href="/usr/share/doc/gtk-doc with
#    hrefs of the form href="http://developer.gnome.org/doc/API/2.0
#    This is accomplished by the first sed's first "-e"
# 2. Insert the shortcut icon before </head>
#    This is accomplished by the first sed's second "-e"
# 3. Find the last occurrence of </div> and replace it with
#    <the comments necessary to produce the "Last Modified:" message></div>
#    This is accomplished by reversing the order of lines in the file with
#    "tac" and replacing the /first/ occurrence of </div> as described, and
#    then reversing the order of lines again with another "tac"
#    Tasks 1. and 2. are unaffected by these reversals
webdocs: all ../../pixmaps/QCADesigner.ico
	echo "$(prefix)"
	rm -rf html-web
	mkdir -p html-web
	for file in `find html`; do \
	  ROOT_PATH=`echo $${file} | sed 's/^html//'`; \
	  if test "" != "$${ROOT_PATH}"; then \
	    if test -d $$file; then \
	      echo "Creating directory html-web$${ROOT_PATH}..."; \
	      mkdir -p html-web$${ROOT_PATH}; \
	    else \
	      if echo $$file | grep -q '\.html$$'; then \
	        echo "Creating file html-web$${ROOT_PATH}..."; \
	        tac $$file \
	          | sed -r \
	            -e 's!href="(/usr/share/gtk-doc/html|\.\.)!href="$(GNOME_DEV_HOME)!g' \
	            -e 's@</head>@<link rel="shortcut icon" href="QCADesigner.ico" type="image/x-icon"></head>@' \
	          | awk -v 'maxmatch=0' \
	            '/<\/div>/{if (0 == maxmatch) sub ("</div>", "<font size=\"-3\"><!--#config timefmt=\"%F %T\" --><!--#set var=\"LastModifiedMsg\" value=\"Last modified: $${LAST_MODIFIED}\" --><!--#echo var=\"LastModifiedMsg\" --></font>&") ; maxmatch = 1 ;} {print $$0 ;}' \
	          | tac \
	          > html-web$${ROOT_PATH}; \
	      else \
	        echo "Copying file html-web$${ROOT_PATH}..."; \
	        cp $$file html-web$${ROOT_PATH}; \
	      fi \
	    fi; \
	  fi; \
	done
	cp ../../pixmaps/QCADesigner.ico html-web

deepclean: clean
	rm -rf \
	  QCADesigner-decl-list.txt QCADesigner-decl.txt QCADesigner-overrides.txt QCADesigner-undocumented.txt \
	  QCADesigner-unused.txt QCADesigner.args QCADesigner.hierarchy QCADesigner.interfaces \
	  QCADesigner.prerequisites QCADesigner.signals html html-build.stamp html-web html.stamp scan-build.stamp \
	  sgml-build.stamp sgml.stamp tmpl tmpl-build.stamp tmpl.stamp xml
