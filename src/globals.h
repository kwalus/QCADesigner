//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// **** Please use complete names in variables and      //
// **** functions. This will reduce ramp up time for new//
// **** people trying to contribute to the project.     //
//////////////////////////////////////////////////////////


/*!
This file defines all the global variables used in the program.
Each of the variables here should be defined as "extern" then
declared in the globals.c file. All typedefs should be included
here and not in the globals.c file.
*/

#ifndef GLOBALS
#define GLOBALS

#include "support.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>


///////////////////////////////////////////////////////////////////
///////////////////////// DEFINES /////////////////////////////////
///////////////////////////////////////////////////////////////////

//!File OPEN/SAVE actions used by file save dialog to determing whether to open or save
//!the argument file.
#define NO_ACTION 0
#define OPEN 1
#define SAVE 2
#define EXPORT 3
#define IMPORT 4
#define OPEN_RECENT 5

#define PRINTED_OBJECTS_DIE     0
#define PRINTED_OBJECTS_CELLS   1
#define PRINTED_OBJECTS_COLOURS 2

//!Color defines
#define GREEN 0
#define GREEN1 5
#define GREEN2 6
#define GREEN3 7
#define RED 1
#define BLUE 2
#define YELLOW 3
#define WHITE 4
#define ORANGE 8
#define BLACK 9

//!Qcell Types
#define TYPE_1 1
#define TYPE_2 2

//!Simulation Engines
#define NONLINEAR_APPROXIMATION 0
#define BISTABLE 2
#define DIGITAL_SIM 3

//!Simulation Types
#define EXHAUSTIVE_VERIFICATION 0
#define VECTOR_TABLE 1

//!Simulation Algorithms
enum {CRANK_NICHOLSON,SPECTRAL_DECOMPOSITION,ADIABATIC};

//!Some usefull physical constants
#define QCHARGE 1.6021892e-19
#define HALF_QCHARGE 0.8e-19
#define EPSILON 8.8541878e-12
#define PI 3.14159265358979
#define HBAR 1.0545887e-34
#define PRECISION 1e-5

// Used by graphing window
#define BOUNDARY_RECT  15
#define BOUNDARY_GRAPH 20
#define DLG_HEIGHT 400
#define DLG_WIDTH 500

// -- menu choices -- //
#define NONE 0
#define HORIZ_CELL 1
#define DIAG_CELL 2
#define ZOOM_WINDOW 3
#define SELECT_CELL 4
#define MOVE_CELL 5
#define SELECT_CELL_AS_INPUT 6
#define SELECT_CELL_AS_OUTPUT 7
#define CHANGE_INPUT_PROPERTIES 8
#define DRAW_CELL_ARRAY 9
#define DELETE_CELL 10
#define MEASURE_DISTANCE 11
#define ROTATE_CELL 12
#define MIRROR_CELLS 13
#define CUSTOM_CELL 14
#define CELL_PROPERTIES 15
#define SELECT_CELL_AS_FIXED 16
#define CLEAR 17
#define PAN 18

// Maximum length of a file system path
#define PATH_LENGTH 1024

//!QCADesigner Version Number
extern double qcadesigner_version;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEVICE TYPEDEFS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// -- Quantum Dot Structure used in the qcell structure -- //
typedef struct {
  
  // absolute world qdot coords //
  double x;
  double y;
  
  // qdot diameter //
  double diameter;
  
  // qdot charge //
  double charge;
  
  // quantum spin of charge within dot //
  float spin;

  /* electrostatic potential induced by all other cells on
     this dot. */
  double potential;

} qdot;


// standard qcell type //
typedef struct qcell {

// Cell Model

	void *cell_model;

// center coords //
    double x;
    double y;

// corner coords //
    double top_x;
    double top_y;
    double bot_x;
    double bot_y;

// -- cell physical parameters -- //
    double cell_width;
    double cell_height;

// cell orientation
     int orientation;
  
// all the dots within this cell  //
    qdot *cell_dots;
    int number_of_dots;

  //ENGINE SPECIFIC (NEXT 3) -- Leave until I grab these for the 
  // full physical model. --TD
  /* distance between the centers of two closest quantum dots */
  //double intradot_distance;
  /* Number of dots with spin up/down */
  //int num_dots_spin_up;
  //int num_dots_spin_down;
     
// current cell color //
    int color;

// the clock that this cell is linked to //
    int clock;

  /* ENGINE SPECIFIC??*/
// response shift is used in the random fault simulation //
	//double response_shift;

// cell type flags //
    gint is_input;
    gint is_output;
    gint is_fixed;

// cell label used to store input name or output name //
    char *label;

// pointers to the previous and next cell //
// needed since all the cells form a doubly linked list //      
    struct qcell *previous;
    struct qcell *next;
	
} qcell;

/* Unfortunately, I think this has to stay */
typedef struct doublecomplex{double r; double i;} doublecomplex;
 
/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEVICE GLOBALS /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!Currently selected cell type
extern int selected_cell_type;

//!Structure holding main window widgets
typedef struct
  {
    GtkWidget *main_window;
    GtkWidget *vbox1;
    GtkWidget *vpaned1;
    GtkWidget *main_menubar;
    GtkWidget *file_menu;
    GtkWidget *file_menu_menu;
    GtkAccelGroup *file_menu_menu_accels;
    GtkWidget *new_menu_item;
    GtkWidget *open_menu_item;
    GtkWidget *save_menu_item;
    GtkWidget *save_as_menu_item;
    GtkWidget *separator2;
    GtkWidget *print_menu_item;
    GtkWidget *preview_menu_item;
    GtkWidget *separator3;
    GtkWidget *project_properties_menu_item;
    GtkWidget *close_menu_item;
    GtkWidget *quit_menu_item;
    GtkWidget *edit_menu;
    GtkWidget *edit_menu_menu;
    GtkAccelGroup *edit_menu_menu_accels;
    GtkWidget *undo_menu_item;
    GtkWidget *redo_menu_item;
    GtkWidget *separator7;
    GtkWidget *copy_menu_item;
    GtkWidget *cut_menu_item;
    GtkWidget *paste_menu_item;
    GtkWidget *separator6;
    GtkWidget *grid_properties_menu_item;
    GtkWidget *snap_properties_menu_item;
    GtkWidget *cell_properties_menu_item;
    GtkWidget *window_properties_menu_item;
    GtkWidget *layer_properties_menu_item;
    GtkWidget *preferences_menu_item;
    GtkWidget *separator4;
    GtkWidget *snap_to_grid_menu_item;
    GtkWidget *show_grid_menu_item;
    GtkWidget *tools_menu;
    GtkWidget *tools_menu_menu;
    GtkAccelGroup *tools_menu_menu_accels;
    GtkWidget *create_block_menu_item;
    GtkWidget *import_block_menu_item;
    GtkWidget *separator8;
    GtkWidget *create_input_menu_item;
    GtkWidget *input_properties_menu_item;
    GtkWidget *separator9;
    GtkWidget *connect_output_menu_item;
    GtkWidget *clock_select_menu_item;
    GtkWidget *clock_increment_menu_item;
    GtkWidget *fixed_polarization;
    GtkWidget *separator11;
    GtkWidget *measure_distance_menu_item;
    GtkWidget *measurement_preferences1;
    GtkWidget *draw_menu;
    GtkWidget *draw_menu_menu;
    GtkAccelGroup *draw_menu_menu_accels;
    GtkWidget *draw_dimensions_menu_item;
    GtkWidget *dimension_properties1;
    GtkWidget *separator16;
    GtkWidget *draw_text_menu_item;
    GtkWidget *text_properties_menu_item;
    GtkWidget *separator10;
    GtkWidget *draw_arrow_menu_item;
    GtkWidget *arrow_properties_menu_item;
    GtkWidget *separator14;
    GtkWidget *draw_line_menu_item;
    GtkWidget *line_properties_menu_item;
    GtkWidget *separator15;
    GtkWidget *draw_rectangle_menu_item;
    GtkWidget *rectangle_properties_menu_item;
    GtkWidget *simulation_menu;
    GtkWidget *simulation_menu_menu;
    GtkAccelGroup *simulation_menu_menu_accels;
    GtkWidget *start_simulation_menu_item;
    GtkWidget *stop_simulation_menu_item;
    GtkWidget *pause_simulation_menu_item;
    GtkWidget *reset_simulation_menu_item;
    GtkWidget *calculate_ground_state_menu_item;
    GtkWidget *animate_test_simulation_menu_item;
    GtkWidget *separator12;
    GtkWidget *save_output_to_file_menu_item;
    GtkWidget *separator13;
    GtkWidget *logging_properties_menu_item;
    GtkWidget *simulation_type_setup_menu_item;
    GtkWidget *simulation_engine_setup_menu_item;
    GtkWidget *random_fault_setup_menu_item;
    GtkWidget *view_menu;
    GtkWidget *view_menu_menu;
    GtkAccelGroup *view_menu_menu_accels;
    GtkWidget *zoom_in_menu_item;
    GtkWidget *zoom_out_menu_item;
    GtkWidget *zoom_window_menu_item;
    GtkWidget *zoom_die_menu_item;
    GtkWidget *zoom_extents_menu_item;
    GtkWidget *pan_menu_item;
    GtkWidget *help_menu;
    GtkWidget *help_menu_menu;
    GtkAccelGroup *help_menu_menu_accels;
    GtkWidget *contents_menu_item;
    GtkWidget *search_menu_item;
    GtkWidget *about_menu_item;
    GtkWidget *hbox1;
    GtkWidget *toolbar2;
    GtkWidget *tmp_toolbar_icon;
    GtkWidget *insert_type_1_cell_button;
    GtkWidget *insert_type_2_cell_button;
    GtkWidget *insert_cell_array_button;
    GtkWidget *copy_cell_button;
    GtkWidget *cell_properties_button;
    GtkWidget *move_cell_button;
    GtkWidget *rotate_cell_button;
    GtkWidget *mirror_button;
    GtkWidget *delete_cells_button;
    GtkWidget *zoom_plus_button;
    GtkWidget *zoom_minus_button;
    GtkWidget *zoom_extents_button;
    GtkWidget *table1;
    GtkWidget *drawing_area_frame;
    GtkWidget *drawing_area;
    GtkWidget *horizontal_ruler;
    GtkWidget *vertical_ruler;
    GtkWidget *scrolledwindow1;
    GtkWidget *command_history;
    GtkWidget *command_entry;
    GtkWidget *status_bar;
    GtkAccelGroup *accel_group;
    GtkTooltips *tooltips;
    GtkWidget *mnuSepPreRecentFiles ;
    GtkWidget *mnuSepPostRecentFiles ;
    GtkWidget *recent_files_menu ;
    GtkWidget *recent_files_menu_item ;
  } main_W ;

/////////////////////////////////////////////////////////////////////////////////
///////////////////////// SIMULATION ENGINE TYPDEF //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!Is used to make a list of activated inputs to be used in vector table simulations.
typedef struct{
	
	int num_activated;
	int num_available;
	qcell **activated_cells;
	qcell **available_cells;
	int activated_selected_row;
	int available_selected_row;
	
}activated_input_list;

//!contains the vector table data
typedef struct{
	
	int num_of_vectors;
	int num_of_bits;
	int **data;

}vector_data;

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DIALOG TYPDEFS /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

typedef struct{

	GtkWidget *vector_table_options_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *top_vbox;
  	GtkWidget *instructions_label;
  	GtkWidget *list_frame;
  	GtkWidget *vector_table_frame;
  	GtkWidget *scrolledwindow1;
  	GtkWidget *vector_table_textbox;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox1;
  	GtkWidget *vector_table_options_load_table_button;
  	GtkWidget *vector_table_options_save_table_button;
  	GtkWidget *vector_table_options_ok_button;
	GtkAdjustment *available_adjust;
	GtkAdjustment *activated_adjust;
	GtkWidget *list_table;
	GtkWidget *list_vbox;
	GtkWidget *list_hbox;
	GtkWidget *list_add_button;
	GtkWidget *list_remove_button;
	GtkWidget *activated_scrbar;
	GtkWidget *activated_box;
	GtkWidget *activated_list;
	GtkWidget *available_scrbar;
	GtkWidget *available_box;
	GtkWidget *available_list;


}vector_table_options_D;

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CAD TYPDEFS ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CAD GLOBALS ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!General CAD options
extern gint SNAP_TO_GRID;
extern gint SHOW_GRID;
extern gint DRAW_POLARIZATION;
extern gint PRINT_PREVIEW;

//!scale multiplier in nanometers.
extern double scale; // [pixel/ nm]

//!grid spacing measured in nanometers.
extern double grid_spacing;  

//!Substrate height.
extern double subs_height;

//!Substrate width.
extern double subs_width;

// -- current top coords of the substrate relative to top corner of drawing area -- //
extern double subs_top_x;
extern double subs_top_y;

// -- Drawing Area width and height -- //
extern int AREA_WIDTH;
extern int AREA_HEIGHT;

// widgets and such needed by the draw_stdqcell finction //
// could be declared within but this would slow down the drawing algoritm //
extern GdkColormap *global_colormap;
extern GdkGC *global_gc;
extern GdkColor global_color;

// pointers to the first and last cell in the linked list //
extern qcell *first_cell;
extern qcell *last_cell;

// total number of cells currently in the design space //
extern int total_number_of_cells;

// an area of selected cells used in window selection and also in simulation for all cells within a radius //
extern qcell **selected_cells;
extern int number_of_selected_cells;

//!Pointer to the cell which was clicked on when moving many cells.
//!This is the cell that is kept under the pointer when many are moved.
extern qcell *window_move_selected_cell;

//!Currently zooming is performed by selecting a window smaller or larger then the screen.
//!Then performing a window zoom function based on that window.
extern int zoom_top_x;
extern int zoom_top_y;
extern int zoom_bottom_x;
extern int zoom_bottom_y;  

//!Coordinates used to draw arrays of cells.
extern int array_x0, array_x1, array_y0, array_y1;

//!Coordinates used to draw a ruler to the screen.
extern int dist_x0, dist_y0, dist_x1, dist_y1;

//!Coordinates used to create a mirror line for mirroring cells.
extern int mirror_x0, mirror_y0, mirror_x1, mirror_y1;

//!
extern int click_count;

//!cell pointer used in callbacks
extern qcell cell;

//!Dialog for setting up the vector tables.
extern vector_table_options_D vector_table_options;

//!Main window
extern main_W main_window ;

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SIMULATION TYPEDEFS ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//!Structure used to pass the cell and number of neighbours to quicksort.
//!Used to sort the cells with respect to the number of neighbours they have.
typedef struct
{
  qcell *cell;
  int number_of_neighbours;
}
SORTSTRUCT;

typedef struct
{
  int index;
  float energy;
}
ENERGYSORTSTRUCT;
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SIMULATION GLOBALS ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!Current simulation type.
extern int SIMULATION_TYPE;

//!Arrays of cells sorted by number of neighbours.
extern qcell **sorted_cells;

//!Simulation log file
extern FILE *sim_log_file;

//!Maximum random response function shift.
extern float max_response_shift;

//!Probability that a design cell will be affected by the random response function shift.
extern float affected_cell_probability;

//!Vector table activated list
extern activated_input_list active_inputs;

//!Vector table data
extern vector_data vector_table;

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////// GRAPHING TYPEDEFS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


enum
  {
     TRACE_COLUMN,
     N_COLUMNS
  };
  

typedef struct{
	char *row[1];
	
}row_define;

typedef struct{
	
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
	GtkWidget *add_button;
	GtkWidget *remove_button;
	GtkWidget *available_list;
	GtkWidget *activated_list;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *available_box;
	GtkWidget *activated_box;
	GtkWidget *available_scrbar;
	GtkWidget *activated_scrbar;
}options_D;	

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// GRAPHING GLOBALS /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

extern GtkAdjustment *available_adjust;
extern GtkAdjustment *activated_adjust;

#endif




















