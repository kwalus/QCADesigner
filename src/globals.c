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


/*
This file defines all the global variables used in the program.
Do not include #defines or typdefs here instead include them in
the globals.h file.
*/

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"

//!QCADesigner Version Number
double qcadesigner_version = 1.1;


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEVICE GLOBALS /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!Currently selected cell type
int selected_cell_type = 1;
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CAD GLOBALS ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!General CAD options
gint SNAP_TO_GRID = TRUE;
gint SHOW_GRID = TRUE;
gint DRAW_POLARIZATION = FALSE;
gint PRINT_PREVIEW = FALSE;

//!scale multiplier in nanometers.
double scale = 1; // [pixel/ nm]

//!grid spacing measured in nanometers.
double grid_spacing = 10;  

//!Substrate height.
double subs_height = 3000;
//!Substrate width.
double subs_width = 6000;

// -- current top coords of the substrate relative to top corner of drawing area -- //
double subs_top_x = 100;
double subs_top_y = 100;

// -- Drawing Area width and height -- //
int AREA_WIDTH = 0;
int AREA_HEIGHT = 0;

// widgets and such needed by the draw_stdqcell finction //
// could be declared within but this would slow down the drawing algoritm //
GdkColormap *global_colormap = NULL;
GdkGC *global_gc = NULL;
GdkColor global_color;

// pointers to the first and last cell in the linked list //
qcell *first_cell = NULL;
qcell *last_cell = NULL;

// total number of cells currently in the design space //
int total_number_of_cells = 0;

// an area of selected cells used in window selection and also in simulation for all cells within a radius //
qcell **selected_cells = NULL;
int number_of_selected_cells = 0;

//!Pointer to the cell which was clicked on when moving many cells.
//!This is the cell that is kept under the pointer when many are moved.
qcell *window_move_selected_cell = NULL;

//!Currently zooming is performed by selecting a window smaller or larger then the screen.
//!Then performing a window zoom function based on that window.
int zoom_top_x;
int zoom_top_y;
int zoom_bottom_x;
int zoom_bottom_y;  

//!Coordinates used to draw arrays of cells.
int array_x0, array_x1, array_y0, array_y1;

//!Coordinates used to draw a ruler to the screen.
int dist_x0, dist_y0, dist_x1, dist_y1;

//!Coordinates used to create a mirror line for mirroring cells.
int mirror_x0, mirror_y0, mirror_x1, mirror_y1;

//!cell pointer used in callbacks
qcell cell;

//!Dialog for setting up the vector tables.
vector_table_options_D vector_table_options = {NULL} ;

//!Main window.
main_W main_window = {NULL} ;

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SIMULATION GLOBALS ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!Current simulation type.
int SIMULATION_TYPE;

//!Arrays of cells sorted by number of neighbours.
qcell **sorted_cells = NULL;

//!Simulation log file
FILE *sim_log_file = NULL;

//!Vector table activated list
activated_input_list active_inputs;

//!Vector table data
vector_data vector_table;
