## Process this file with automake to produce Makefile.in

INCLUDES = @GTK_CFLAGS@

bin_PROGRAMS = QCADesigner

QCADesigner_SOURCES = \
	main.c \
	recent_files.c recent_files.h \
	support.c support.h \
	globals.c globals.h qcell.h \
	interface.c interface.h \
	about.c about.h \
	fileio.c fileio.h \
	callbacks.c callbacks.h \
	message_box.c message_box.h \
	cad.c cad.h \
	file_selection_window.c file_selection_window.h \
	blocking_dialog.c blocking_dialog.h \
	fixed_polarization_dialog.c fixed_polarization_dialog.h \
	graph_dialog.c graph_dialog.h \
	graph_options_dialog.c graph_options_dialog.h \
	grid_spacing_dialog.c grid_spacing_dialog.h \
	name_dialog.c name_dialog.h \
	project_properties_dialog.c project_properties_dialog.h \
	cell_properties_dialog.c cell_properties_dialog.h \
	nonlinear_approx_properties_dialog.c nonlinear_approx_properties_dialog.h \
	clock_select_dialog.c clock_select_dialog.h \
	sim_type_setup_dialog.c sim_type_setup_dialog.h \
	sim_engine_setup_dialog.c sim_engine_setup_dialog.h \
	random_fault_setup_dialog.c random_fault_setup_dialog.h \
	new_vector_table_options_dialog.c new_vector_table_options_dialog.h \
	vector_table_options_dialog.c dialog_boxes.h \
	print_properties_dialog.c print_properties_dialog.h\
	print.c print.h \
	print_preview.c print_preview.h \
	bistable_properties_dialog.c bistable_properties_dialog.h\
	vector_table.c vector_table.h \
	stdqcell.c stdqcell.h \
	nonlinear_approx.c nonlinear_approx.h \
	eigenvalues.c eigenvalues.h \
	nrutil.c nrutil.h \
	custom_widgets.c custom_widgets.h \
	bistable_simulation.c bistable_simulation.h \
	simulation.c simulation.h run_dig_sim.h \
	enkidu_parcel.cpp enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h\
	enkidu_api.cpp enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h\
	qca_sim.cpp qca_sim.h cell_info.h enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h cell.h cell_neighbor.h \
	cell.cpp cell.h enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h cell_neighbor.h ptr_comp.h globals.h \
	cell_info.cpp enkidu.h cell_info.h enkidu_parcel.h enkidu_api.h enkidu_output.h \
	cell_neighbor.cpp cell_neighbor.h ptr_comp.h

##QCADesigner_LDFLAGS = -static

QCADesigner_LDADD = @GTK_LIBS@ $(PTHREAD_LIBS)
