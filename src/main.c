//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////


#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif



#include <gtk/gtk.h>
#include <assert.h>
#include <stdlib.h>
#include "globals.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "about.h"
#include "print.h"
#include "stdqcell.h"
#include "nonlinear_approx.h"
#include "bistable_simulation.h"
#include "globals.h"
#include "recent_files.h"
#include "vector_table.h"

//!Print options
print_OP print_options ;

//!Options for the bistable simulation engine
bistable_OP bistable_options;

//!Options for the nonlinear approximation engine
nonlinear_approx_OP nonlinear_approx_options = {0, 50.0, 5.0} ;

//!Options for the cells
cell_OP cell_options = {0, 18, 18, 5, 9, 9, 2} ;

extern VectorTable *pvt ;

// -- This is pretty clear -- //
int main (int argc, char *argv[]){
  	
	// -- GTKWidgets -- //
	
	#ifdef ENABLE_NLS
		bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
		textdomain (PACKAGE);
	#endif
	
	gtk_set_locale ();
	gtk_init (&argc, &argv);

	// -- Pixmaps used by the buttons in the main window -- //
	add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");

	// -- Create the main window and the about dialog -- //
	create_main_window (&main_window);
	
	fill_recent_files_menu (main_window.recent_files_menu, file_operations, (gpointer)OPEN_RECENT) ;
  
	// -- Show the main window and the about dialog -- //
	gtk_widget_show (main_window.main_window);
	
	show_about_dialog (GTK_WINDOW (main_window.main_window), TRUE) ;

	//clear any global variables//
	selected_cells = NULL;
	
	// -- Set the default values for the bistable simulation -- //
	bistable_options.convergence_tolerance = 1e-3;
	bistable_options.K = 1000;
	bistable_options.decay_exponent = 5;
	bistable_options.radius_of_effect = 65;
	bistable_options.number_of_samples = 3000 ;
	bistable_options.max_iterations_per_sample = 100;
	
	// -- Set the default values for the page size and margins -- //
	print_options.dPaperWidth    = 612 /* points */ ;
	print_options.dPaperHeight   = 792 /* points */ ;
	print_options.dLeftMargin    =  72 /* points */ ;
	print_options.dTopMargin     =  72 /* points */ ;
	print_options.dRightMargin   =  72 /* points */ ;
	print_options.dBottomMargin  =  72 /* points */ ;
	print_options.dPointsPerNano =   3 /* points per nanometer */ ;
	print_options.pbPrintedObjs = malloc (3 * sizeof (gboolean)) ;
	print_options.icPrintedObjs = 3 ;
	print_options.pbPrintedObjs[PRINTED_OBJECTS_CELLS] = TRUE ;
	print_options.pbPrintedObjs[PRINTED_OBJECTS_DIE] = FALSE ;
	print_options.pbPrintedObjs[PRINTED_OBJECTS_COLOURS] = TRUE ;
	print_options.bPrintFile = TRUE ;
	print_options.bPrintOrderOver = TRUE ;
	print_options.bCenter = FALSE ;
	print_options.szPrintString[0] = 0 ; /* IOW an empty string */
	print_options.iCXPages =
	print_options.iCYPages = 1 ;
	
	pvt = VectorTable_new () ;

	// -- LET'S GO -- //
	gtk_main ();
	
	VectorTable_clear (pvt) ;
	
	// -- Exit -- //
  	return 0;

}//main
