## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

# bin_PROGRAMS = QCADesigner batch_sim graph_dialog test
bin_PROGRAMS = QCADesigner batch_sim graph_dialog

include Makefile.inc

#################### QCADesigner ####################
QCADesigner_DEFINES = -DPROPERTY_UIS -DSTDIO_FILEIO -DUNDO_REDO -DGTK_GUI -DDESIGNER

# This variable is only used in Windoze
QCADesigner_LIBS =	-lgtk-win32-2.0 \
			-latk-1.0 \
			-lpango-1.0 \
			-lglib-2.0 \
			-lgthread-2.0 \
			-lgdk-win32-2.0 \
			-lgobject-2.0 \
			-lintl \
			-lgdk_pixbuf-2.0

QCADesigner_CFLAGS = \
  $(QCADesigner_DEFINES) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@ \
	@GTHREAD_CFLAGS@

QCADesigner_SOURCES = \
	gtk_preamble.c                          gtk_preamble.h \
	support.c                               support.h \
\
  $(CORE_FILES) \
  $(OBJECT_FILES) \
  $(SIMULATION_FILES) \
  $(STDIO_FILEIO_FILES) \
\
  $(GRAPH_DIALOG_FILES) \
  $(PROPERTY_UI_FILES) \
  $(UNDO_REDO_FILES) \
  $(GTK_WORKAROUND_FILES) \
  $(SIMULATION_UI_FILES) \
  $(VECTOR_TABLE_DIALOG_FILES) \
\
	interface.c                             interface.h \
	callbacks.c                             callbacks.h \
	                                        actions.h \
                                         	actions/action_handlers.h \
	actions/array.c                         actions/array.h \
	actions/rotate.c                        actions/rotate.h \
	actions/select.c                        actions/select.h \
	                                        callback_helpers.h \
	exp_pixmap.c                            exp_pixmap.h \
	                                        global_consts.h \
\
	simulation_data.c                       simulation_data.h \
	graph_dialog_widget_data.c              graph_dialog_widget_data.h \
\
	objects/mouse_handlers.c                objects/mouse_handlers.h \
	                                    	  objects/mouse_handler_struct.h \
	                                    	  objects/objects_debug.h \
	objects/QCADCellRendererVT.c            objects/QCADCellRendererVT.h \
	objects/QCADCellRendererLayerList.c     objects/QCADCellRendererLayerList.h \
\
	objects/QCADPrintDialog.c               objects/QCADPrintDialog.h \
	print_properties_dialog.c               print_properties_dialog.h \
	print_properties_dialog_interface.c     print_properties_dialog_interface.h \
	print_graph_properties_dialog.c         print_graph_properties_dialog.h \
	                                        print_properties_dialog_callbacks.h \
	print.c                                 print.h \
	print_graph.c \
	print_util.c                            print_util.h \
	print_preview.c                         print_preview.h \
\
	                                        qcadstock.h \
	selection_renderer.c                    selection_renderer.h \
	vector_table.c                          vector_table.h \
	recent_files.c                          recent_files.h \
\
	about.c                                 about.h \
	file_selection_window.c                 file_selection_window.h \
	bus_layout_dialog.c                     bus_layout_dialog.h \
	layer_mapping_dialog.c                  layer_mapping_dialog.h \
	layer_order_dialog.c                    layer_order_dialog.h \
	scale_dialog.c                          scale_dialog.h \
	sim_engine_setup_dialog.c               sim_engine_setup_dialog.h \
	translate_selection_dialog.c            translate_selection_dialog.h \
\
	main.c

QCADesigner_LDADD = \
	-lm \
	@GTK_LIBS@ \
	$(INTLLIBS) \
	@GTHREAD_LIBS@ 

#################### batch_sim ######################

# This variable is only used in Windoze
batch_sim_LIBS =	-lglib-2.0 \
			-lgobject-2.0 \
			-lintl

batch_sim_DEFINES = -DSTDIO_FILEIO

batch_sim_CFLAGS = \
	$(batch_sim_DEFINES) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GLIB_CFLAGS@ \
	@GOBJECT_CFLAGS@

batch_sim_SOURCES = \
	$(CORE_FILES) \
	$(OBJECT_FILES) \
	$(SIMULATION_FILES) \
	$(STDIO_FILEIO_FILES) \
\
	simulation_data.c                       simulation_data.h \
	graph_dialog_widget_data.c              graph_dialog_widget_data.h \
\
	main_batch_sim.c

batch_sim_LDADD = \
	-lm \
	@GLIB_LIBS@ \
	@GOBJECT_LIBS@

#################### graph_dialog ###################
graph_dialog_DEFINES = -DSTDIO_FILEIO -DGTK_GUI

# This variable is only used in Windoze
graph_dialog_LIBS =	-lgtk-win32-2.0 \
			-latk-1.0 \
			-lpango-1.0 \
			-lglib-2.0 \
			-lgthread-2.0 \
			-lgdk-win32-2.0 \
			-lgobject-2.0 \
			-lintl \
			-lgdk_pixbuf-2.0

graph_dialog_CFLAGS = \
  $(graph_dialog_DEFINES) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@ \
	@GTHREAD_CFLAGS@

graph_dialog_SOURCES = \
	gtk_preamble.c                          gtk_preamble.h \
	support.c                               support.h \
\
  $(CORE_FILES) \
  $(OBJECT_FILES) \
  $(GRAPH_DIALOG_FILES) \
  $(STDIO_FILEIO_FILES) \
\
	simulation_data.c                       simulation_data.h \
	graph_dialog_widget_data.c              graph_dialog_widget_data.h \
\
	objects/QCADPrintDialog.c               objects/QCADPrintDialog.h \
	print_graph_properties_dialog.c         print_graph_properties_dialog.h \
	                                        print_properties_dialog_callbacks.h \
	print_graph.c \
	print_util.c                            print_util.h \
	print_preview.c                         print_preview.h \
\
	file_selection_window.c                 file_selection_window.h \
	bus_layout_dialog.c                     bus_layout_dialog.h \
\
	objects/QCADRadioButton.c               objects/QCADRadioButton.h \
	objects/QCADRadioToolButton.c           objects/QCADRadioToolButton.h \
	main_graph_dialog.c

graph_dialog_LDADD = \
	-lm \
	@GTK_LIBS@ \
	$(INTLLIBS) \
	@GTHREAD_LIBS@ 

#################### test ###########################
# test_CFLAGS = \
# 	-DGTK_GUI \
# 	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
# 	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
# 	@GTK_CFLAGS@ \
# 	@GTHREAD_CFLAGS@
# 
# test_SOURCES = \
# 	hsl2rgb.c                                  hsl2rgb.h \
# 	gtk_preamble.c                             gtk_preamble.h \
# 	exp_array.c                                exp_array.h \
# 	custom_widgets.c                           custom_widgets.h \
# 	support.c                                  support.h \
# 	drag_rect.c                                drag_rect.h \
# \
#         $(OBJECT_FILES) \
# \
# 	objects/QCADClockCombo.c                   objects/QCADClockCombo.h \
# 	exp_pixmap.c                               exp_pixmap.h \
# \
# 	objects/object_helpers.c                   objects/object_helpers.h \
# 	generic_utils.c                            generic_utils.h \
# 	main_test.c
# 
# test_LDADD = \
# 	-lm \
# 	@GTK_LIBS@ \
# 	$(INTLLIBS) \
# 	@GTHREAD_LIBS@ 
