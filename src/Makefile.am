## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ @GTHREAD_CFLAGS@

bin_PROGRAMS = QCADesigner

QCADesigner_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	command_history.c command_history.h \
	callbacks.c callbacks.h callback_helpers.h \
	actions/array.c \
	actions/default.c \
	actions/single_cell.c \
	actions/rotate.c \
	actions/mirror.c \
	actions/delete.c \
	actions/cell_function.c \
	actions/pan.c actions/action_handlers.h \
	fileio.c fileio.h \
	fileio_helpers.c fileio_helpers.h \
	cad_util.c cad_util.h \
	cad.c cad.h \
	print_util.c print_util.h \
	print.c print_graph.c print.h \
	print_preview.c print_preview.h \
	vector_table.c vector_table.h \
	stdqcell.c stdqcell.h \
	recent_files.c recent_files.h \
	gqcell.c gqcell.h \
	undo_redo.c undo_redo.h undo_structs.h \
	undo_actions.c undo_actions.h \
	undo_create.c undo_create.h \
	undo_destroy.c undo_destroy.h \
	custom_widgets.c custom_widgets.h \
	print_dialog.c pritn_dialog.h \
	about.c about.h \
	print_properties_dialog.c print_properties_dialog.h\
	print_properties_dialog_interface.c print_properties_dialog_interface.h \
	print_properties_dialog_callbacks.h \
	print_graph_properties_dialog.c print_graph_properties_dialog.h \
	clock_select_dialog.c clock_select_dialog.h \
	file_selection_window.c file_selection_window.h \
	cell_properties_dialog.c cell_properties_dialog.h \
	project_properties_dialog.c project_properties_dialog.h \
	grid_spacing_dialog.c grid_spacing_dialog.h \
        cell_function_dialog.c cell_function_dialog.h \
	sim_type_setup_dialog.c sim_type_setup_dialog.h \
	vector_table_options_dialog.c vector_table_options_dialog.h \
	sim_engine_setup_dialog.c sim_engine_setup_dialog.h \
	nonlinear_approx_properties_dialog.c nonlinear_approx_properties_dialog.h \
	bistable_properties_dialog.c bistable_properties_dialog.h \
	scqca_properties_dialog.c scqca_properties_dialog.h \
	random_fault_setup_dialog.c random_fault_setup_dialog.h \
	graph_dialog.c graph_dialog.h \
	graph_options_dialog.c graph_options_dialog.h \
	scale_dialog.c scale_dialog.h\
	simulation.c simulation.h \
	eigenvalues.c eigenvalues.h \
	nrutil.c nrutil.h \
	enkidu_parcel.cpp enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h \
	enkidu_api.cpp run_dig_sim.h \
	qca_sim.cpp qca_sim.h cell_neighbor.h \
	cell.cpp cell.h ptr_comp.h \
	cell_info.cpp cell_neighbor.cpp \
	bistable_simulation.c bistable_simulation.h \
	coherence_vector.c coherence_vector.h \
	nonlinear_approx.c nonlinear_approx.h \
	scqca_simulation.c scqca_simulation.h scqca_constants.h scqca_math.c scqca_math.h

QCADesigner_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) @GTHREAD_LIBS@
