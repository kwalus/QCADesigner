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


// -- includes -- //
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "globals.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "stdqcell.h"
#include "simulation.h"
#include "cad.h"
#include "fileio.h"
#include "print_preview.h"
#include "print.h"
#include "recent_files.h"
#include "vector_table.h"

// dialogs and windows used //
#include "about.h"
#include "file_selection_window.h"
#include "project_properties_dialog.h"
#include "cell_properties_dialog.h"
#include "grid_spacing_dialog.h"
#include "print_properties_dialog.h"
#include "sim_engine_setup_dialog.h"
#include "fixed_polarization_dialog.h"
#include "clock_select_dialog.h"
#include "name_dialog.h"
#include "sim_type_setup_dialog.h"
#include "graph_dialog.h"
#include "random_fault_setup_dialog.h"
#include "message_box.h"

#define DBG_CB(s) s

#define NUMBER_OF_RULER_SUBDIVISIONS 3

static int show_grid_backup ;

//!Currently selection action to perform by the CAD interface (ex Move Cell)
static int selected_action = SELECT_CELL;

//!Wether or not to listen to the motion of the mouse.
static gint listen_motion = FALSE;

//!If cells are moved onto other cells this flag will be set.
static int INVALID_MOVE = FALSE;

//!Current simulation engine.
static int SIMULATION_ENGINE = BISTABLE;

//!The top X coordinate of the selection window.
static int window_top_x;

//!The top Y coordinate of the selction window.
static int window_top_y;

//!Used to keep track of the old top x position of the seletion window
static int prev_window_top_x;

//!Used to keep track of the old top y position of the seletion window
static int prev_window_top_y;

//!The width of the selction window.
static int window_width;

//!The height of the selction window.
static int window_height;

//!Maximum random response function shift.
static float max_response_shift = 0.0;

//!Probability that a design cell will be affected by the random response function shift.
static float affected_cell_probability = 0.0;

//!Has the design been altered ?
static gboolean bDesignAltered = FALSE ;

//!Current project file name.
char current_file_name[PATH_LENGTH] = "" ;

VectorTable *pvt = NULL ;

extern print_OP print_options ;
extern cell_OP cell_options ;

void draw_pan_arrow (int x1, int y1, int x2, int y2) ;
void get_arrow_head_coords (int x1, int y1, int x2, int y2, int length, int *px, int *py, int deg) ;
void setup_rulers () ;
void gcs_set_rop (GdkFunction func) ;
void set_selected_action (int action, int cell_type) ;
void set_ruler_scale (GtkRuler *ruler, double dXLower, double dYLower) ;
gboolean do_save () ;

// This function gets called during a resize
gboolean main_window_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
  {
  setup_rulers () ;
  // This function needs to return a value.
  // this is the source of one of the compiler warnings.
  return TRUE;
  }

// motion notify is called whenever the mouse is moved above the drawing area //
// used to track all mouse motion in the drawing area //
gboolean motion_notify_event(GtkWidget * widget, GdkEventMotion * event, gpointer user_data){

    int x;
    int y;
    int top_x;
    int top_y;

    float offset_x;
    float offset_y;
    
    int i;

    GdkModifierType state;
    
    GdkEventMotion *pevVRule = NULL ;
    GdkEventMotion *pevHRule = NULL ;
    
	// listen_motion is triggered whenever the mouse needs to be tracked //
	// example: when the user wants to zoom window the user clicks and holds the middle mouse button //
	// when this button is clicked the listen_motion flag is set and the mouse is tracked //
	// when the user releases the middle mouse button the flag is released and the mouse is no longer tracked //

    if (listen_motion) {
		
		// get the current mouse position variables and its state ie which buttons are pressed //
		gdk_window_get_pointer(event->window, &x, &y, &state);
	
		// switch acording to the current selected action //
		switch (selected_action) {
	
		case MOVE_CELL:
	
			// -- if there are cells selected for move -- //
			if (number_of_selected_cells > 0) {
			
			gcs_set_rop(GDK_XOR);
			redraw_selected_cells();
			gcs_set_rop(GDK_COPY);	
				
			assert(selected_cells != NULL && window_move_selected_cell != NULL);
	
			if (SNAP_TO_GRID) {
				offset_x = grid_world_x(calc_world_x(x)) - window_move_selected_cell->x;
				offset_y = grid_world_y(calc_world_y(y)) -
				window_move_selected_cell->y;
			} else {
				offset_x = calc_world_x(x) - window_move_selected_cell->x;
				offset_y = calc_world_y(y) - window_move_selected_cell->y;
			}
	
			for (i = 0; i < number_of_selected_cells; i++)
				move_cell_by_offset(selected_cells[i], offset_x, offset_y);
			}
	
			gcs_set_rop(GDK_XOR);
			redraw_selected_cells();
			gcs_set_rop(GDK_COPY);
			
			break;
	
	
		case SELECT_CELL:
	
			// -- draw the selection window as the mouse is being moved -- //
	
			// In order to speed up redrawing I do not redraw the entire background //
			// Instead i draw a black rectangle over where the previous window was then redraw the contents of the design //
			// with a new white window in the current mouse position //
			gdk_draw_rectangle(widget->window, widget->style->black_gc,
					   FALSE, prev_window_top_x, prev_window_top_y,
					   window_width, window_height);
	
			// Determine the true top coords, because the user could have stretched the window upward not downward.
			if (x < window_top_x) {
			top_x = x;
			x = window_top_x;
			} else {
			top_x = window_top_x;
			}
	
			if (y < window_top_y) {
			top_y = y;
			y = window_top_y;
	
			} else {
			top_y = window_top_y;
			}
	
			// record the window parameters that the black window can be drawn next iteration.
			prev_window_top_x = top_x;
			prev_window_top_y = top_y;
			window_width = x - top_x;
			window_height = y - top_y;
	
			// Draw the white selection window
			gdk_draw_rectangle(widget->window, widget->style->white_gc,
					   FALSE, top_x, top_y, window_width,
					   window_height);
	
			redraw_contents(0, 0);
	
			break;
	
		case DRAW_CELL_ARRAY:
	
			// set up the color //
			global_colormap = gdk_colormap_get_system();
			global_gc = gdk_gc_new(widget->window);
	
			global_color.red = 0;
			global_color.green = 0xFFFF;
			global_color.blue = 0xFFFF;
	
			gdk_color_alloc(global_colormap, &global_color);
			gdk_gc_set_foreground(global_gc, &global_color);
			gdk_gc_set_background(global_gc, &global_color);
	
			// draw over the old coords with GDK_XOR to erase the old array cells //
			if(!(array_x0 == array_x1 && array_y0 == array_y1)){
				if (abs(array_x0 - array_x1) >= abs(array_y0 - array_y1)) {
					
					array_y1 = array_y0;
					draw_temp_array(array_x0, array_y0, array_x1, array_y0);
		
				} else {
					
					array_x1 = array_x0;
					draw_temp_array(array_x0, array_y0, array_x0, array_y1);
		
				}
			}
			
			// set the second point of the line to the current mouse position //
			array_x1 = event->x;
			array_y1 = event->y;
	
			// -- draw a temporary horizontal array -- //
			if (abs(array_x0 - array_x1) >= abs(array_y0 - array_y1)) {
				
				array_y1 = array_y0;
				draw_temp_array(array_x0, array_y0, array_x1, array_y0);
	
			// -- Draw a temporary vertical array -- //
			} else {
				
				array_x1 = array_x0;
				draw_temp_array(array_x0, array_y0, array_x0, array_y1);
	
			}
	
			break;
	
		case MIRROR_CELLS:
	
			// set up the color //
			global_colormap = gdk_colormap_get_system();
			global_gc = gdk_gc_new(widget->window);
	
			global_color.red = 0;
			global_color.green = 0xFFFF;
			global_color.blue = 0xFFFF;
	
			gdk_color_alloc(global_colormap, &global_color);
			gdk_gc_set_foreground(global_gc, &global_color);
			gdk_gc_set_background(global_gc, &global_color);
	
			// set the second point of the line to the current mouse position //
			mirror_x1 = event->x;
			mirror_y1 = event->y;
	
			// -- Draw a Horizontal Line -- //
			if (abs(mirror_x0 - mirror_x1) >= abs(mirror_y0 - mirror_y1)) {
			gdk_draw_line(widget->window, global_gc, mirror_x0,
					  mirror_y0, mirror_x1, mirror_y0);
	
	
			// -- Draw a Vertical Line -- //
			} else {
			gdk_draw_line(widget->window, global_gc, mirror_x0,
					  mirror_y0, mirror_x0, mirror_y1);
	
			}
	
			break;
	
		case MEASURE_DISTANCE:
	
			//redraw_world();
			
			//draw over the old ruler with XOR
			if(!(dist_x0 == dist_x1 && dist_y0 == dist_y1))draw_ruler(dist_x0, dist_y0, dist_x1, dist_y1);
			
			// set the second point of the line to the current mouse position //
			dist_x1 = event->x;
			dist_y1 = event->y;
			
			//draw the new ruler
			draw_ruler(dist_x0, dist_y0, dist_x1, dist_y1);
			
			
			break;
		
		case PAN:
		  draw_pan_arrow (dist_x0, dist_y0, dist_x1, dist_y1) ;
		  
//		  fprintf (stderr, "dist_x1 - dist_x0 = %d\ndist_y1 - dist_y0 = %d\n",
//		    dist_x1 - dist_x0, dist_y1 - dist_y0) ;
		  
//		  redraw_contents (dist_x1 - dist_x0, dist_y1 - dist_y0) ;
		  dist_x1 = event->x ;
		  dist_y1 = event->y ;
//		  redraw_contents (dist_x1 - dist_x0, dist_y1 - dist_y0) ;
		  draw_pan_arrow (dist_x0, dist_y0, dist_x1, dist_y1) ;
		  break ;
	
		default:
	
			// In order to speed up redrawing I do not redraw the entire background //
			// Instead i draw a black rectangle over where the previous window was then redraw the contents of the design //
			// with a new white window in the current mouse position //
			gdk_draw_rectangle(widget->window, widget->style->black_gc,
					   FALSE, prev_window_top_x, prev_window_top_y,
					   window_width, window_height);
	
			// Determine the true top coords, because the user could have stretched the window upward not downward.
			if (x < window_top_x) {
			top_x = x;
			x = window_top_x;
			} else {
			top_x = window_top_x;
			}
	
			if (y < window_top_y) {
			top_y = y;
			y = window_top_y;
	
			} else {
			top_y = window_top_y;
			}
	
			// record the window parameters that the black window can be drawn next iteration.
			prev_window_top_x = top_x;
			prev_window_top_y = top_y;
			window_width = x - top_x;
			window_height = y - top_y;
	
			// Draw the white selection window
			gdk_draw_rectangle(widget->window, widget->style->white_gc,
					   FALSE, top_x, top_y, window_width,
					   window_height);
	
			redraw_contents(0, 0);
		}

    }				//if listen_motion

    // Pass this motion event onto the two rulers
    
    pevVRule = (GdkEventMotion *)gdk_event_copy ((GdkEvent *)event) ;
    pevVRule->window = (main_window.vertical_ruler)->window ;
    
    pevHRule = (GdkEventMotion *)gdk_event_copy ((GdkEvent *)event) ;
    pevHRule->window = (main_window.horizontal_ruler)->window ;
    
    gtk_main_do_event ((GdkEvent *)pevVRule) ;
    gtk_main_do_event ((GdkEvent *)pevHRule) ;
    return FALSE;
}

gboolean button_release_event(GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{

    float offset_x;
    float offset_y;
    int i, j;

    if (event->button == 1) {
      switch (selected_action)
        {
	case MOVE_CELL:

	  listen_motion = FALSE;

	  if (number_of_selected_cells > 0) {

	      if (SNAP_TO_GRID) {
		  	offset_x = grid_world_x(calc_world_x(event->x)) - window_move_selected_cell->x;
		  	offset_y = grid_world_y(calc_world_y(event->y)) - window_move_selected_cell->y;
	      } else {
		  	offset_x = calc_world_x(event->x) - window_move_selected_cell->x;
		  	offset_y = calc_world_y(event->y) - window_move_selected_cell->y;
	      }


	      for (j = 0; j < number_of_selected_cells; j++) {

		  assert(selected_cells[j] != NULL);

		  // check to make sure that we are not moving a cell onto another cell //                
		  if (select_cell_at_coords_but_not_this_one (offset_x + selected_cells[j]->x, offset_y + selected_cells[j]->y, selected_cells[j]) != NULL) {
      	      	      char *pszCmdHistMsg = "Cannot move this group of cells to this location, due to exact overlap of at least one of the cells\n" ;
		      // write message to the command history window //
	      	      gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL, pszCmdHistMsg, strlen(pszCmdHistMsg));

		      listen_motion = TRUE;

		      INVALID_MOVE = TRUE;

		      return 0;
		  }
	      }

	      // move all the selected cells by the offset value //           
	      for (i = 0; i < number_of_selected_cells; i++)
		  move_cell_by_offset(selected_cells[i], offset_x, offset_y);

	      clean_up_colors();

	      number_of_selected_cells = 0;
	      window_move_selected_cell = NULL;

	      free(selected_cells);
	      selected_cells = NULL;
	      bDesignAltered = TRUE ;
	      redraw_world();
	}			//if number_selected_cells > 0
      	  break ;
    
    case PAN:
      {
      set_selected_action (SELECT_CELL, selected_cell_type) ;
      draw_pan_arrow (dist_x0, dist_y0, dist_x1, dist_y1) ;
//      redraw_contents (dist_x1 - dist_x0, dist_y1 - dist_y0) ;
//      redraw_contents (event->x - dist_x0, event->y - dist_y0) ;
      gcs_set_rop (GDK_COPY) ;
      listen_motion = FALSE ;
      subs_top_x += (event->x - dist_x0) ;
      subs_top_y += (event->y - dist_y0) ;
      setup_rulers () ;
      SHOW_GRID = show_grid_backup ;
      redraw_world () ;
      break ;
      }

    case DRAW_CELL_ARRAY:
		
	set_colors(GREEN);
	gdk_gc_set_function(global_gc, GDK_COPY);	
	
	create_array_of_cells(array_x0, array_y0, array_x1, array_y1);

	listen_motion = FALSE;
//	set_selected_action (SELECT_CELL, selected_cell_type) ;
	bDesignAltered = TRUE ;
	redraw_world();
	break ;


    case MIRROR_CELLS:
    
	listen_motion = FALSE;
	set_selected_action (SELECT_CELL, selected_cell_type) ;

	if (abs(mirror_x0 - mirror_x1) >= abs(mirror_y0 - mirror_y1)) {
	    mirror_cells_about_line(mirror_x0, mirror_y0, mirror_x1,
				    mirror_y0);
	} else {
	    mirror_cells_about_line(mirror_x0, mirror_y0, mirror_x0,
				    mirror_y1);
	}
	bDesignAltered = TRUE ;
	redraw_world();
	break ;

    case MEASURE_DISTANCE:

	listen_motion = FALSE;
	set_selected_action (SELECT_CELL, selected_cell_type) ;
	redraw_world();
	break ;

    case SELECT_CELL:

	listen_motion = FALSE;
	select_cells_in_window(zoom_top_x, zoom_top_y, event->x, event->y);
	if (number_of_selected_cells > 0)
          update_clock_select_dialog (get_clock_from_selected_cells ()) ;
	redraw_world();
	break ;
    }
  } else if ((event->button == 1 || event->button == 3)
	       && selected_action == CLEAR) {

	set_selected_action (SELECT_CELL, selected_cell_type) ;

    } else if (event->button == 2) {
	listen_motion = FALSE;
	zoom_bottom_x = event->x;
	zoom_bottom_y = event->y;
	zoom_window(zoom_top_x, zoom_top_y, zoom_bottom_x, zoom_bottom_y);
	setup_rulers () ;

    }
    
    else if (event->button == 4) /* Mouse wheel clicked away from the user */
      {
      if (event->state & GDK_CONTROL_MASK)
        pan_left (10) ;
      else
        pan_up (10) ;
      setup_rulers () ;
      }
    else if (event->button == 5) /* Mouse wheel clicked towards the user */
      {
      if (event->state & GDK_CONTROL_MASK)
        pan_right (10) ;
      else
        pan_down (10) ;
      setup_rulers () ;
      }

    return FALSE;
}

gboolean key_press_event(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{

    int i;
    switch (event->keyval) {
    case GDK_Delete:

	if (number_of_selected_cells > 0) {
	    for (i = 0; i < number_of_selected_cells; i++)
	      	{
		if (selected_cells[i]->is_input)
		  VectorTable_del_input (pvt, selected_cells[i]) ;
		delete_stdqcell(selected_cells[i]);
		}


	    free(selected_cells);
	    selected_cells = NULL;
	    number_of_selected_cells = 0;
	    bDesignAltered = TRUE ;
	    redraw_world();
	}

	break;

    case GDK_Left:
	pan_left(1);
	setup_rulers () ;
	break;

    case GDK_Up:
	pan_up(1);
	setup_rulers () ;
	break;

    case GDK_Down:
	pan_down(1);
	setup_rulers () ;
	break;

    case GDK_Right:
	pan_right(1);
	setup_rulers () ;
	break;
	
	case GDK_w:
	printf("w");
	zoom_in();
	setup_rulers () ;
	break;
	
	case GDK_q:

	zoom_out();
	setup_rulers () ;
	break;

    }

    return TRUE;
}

gboolean button_press_event(GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{
    int i = 0, j;
    char *text;
    int TAG;
    float offset_x;
    float offset_y;
    qcell cell;
    qcell *cellp;

    if (event->button == 1) {
	switch (selected_action) {

	case NONE:
	    set_selected_action (CLEAR, selected_cell_type) ;
	    break;

	case DIAG_CELL:

	    if (SNAP_TO_GRID) {
			cell.x = grid_world_x(calc_world_x(event->x));
			cell.y = grid_world_y(calc_world_y(event->y));
	    } else {
			cell.x = calc_world_x(event->x);
			cell.y = calc_world_y(event->y);
	    }

	    add_stdqcell(&cell, TRUE, selected_cell_type);
	    bDesignAltered = TRUE ;
      	    clean_up_colors () ;
	    redraw_world();

	    break;

	case CUSTOM_CELL:

	    if (SNAP_TO_GRID) {
		cell.x = grid_world_x(calc_world_x(event->x));
		cell.y = grid_world_y(calc_world_y(event->y));
	    } else {
		cell.x = calc_world_x(event->x);
		cell.y = calc_world_y(event->y);
	    }

	    // -- create custom dimensions -- //
	    // *** Default cell is used until I create the custom cell dialog *** //
	    cell.cell_width = 0;
	    cell.cell_height = 0;
	    cell.number_of_dots = 0;

	    add_stdqcell(&cell, TRUE, selected_cell_type);
      	    clean_up_colors () ;
	    redraw_world();
	    bDesignAltered = TRUE ;
	    break;

	case CELL_PROPERTIES:
	    set_selected_action (CLEAR, selected_cell_type) ;
	    break;

	case MOVE_CELL:

	    if (INVALID_MOVE) {

		// turn off the motion listen //
		listen_motion = FALSE;

		if (number_of_selected_cells > 0) {

		    // debug //
		    assert(window_move_selected_cell != NULL);
		    assert(selected_cells != NULL);

		    if (SNAP_TO_GRID) {
			offset_x =
			    grid_world_x(calc_world_x(event->x)) -
			    window_move_selected_cell->x;
			offset_y =
			    grid_world_y(calc_world_y(event->y)) -
			    window_move_selected_cell->y;
		    } else {
			offset_x =
			    calc_world_x(event->x) -
			    window_move_selected_cell->x;
			offset_y =
			    calc_world_y(event->y) -
			    window_move_selected_cell->y;
		    }

		    // check if any of the cells will fall on other cells in the design //
		    for (j = 0; j < number_of_selected_cells; j++) {

			if (select_cell_at_coords_but_not_this_one
			    (offset_x + selected_cells[j]->x,
			     offset_y + selected_cells[j]->y,
			     selected_cells[j]) != NULL) {
			     
			     text = "Cannot move this group of cells to this location, due to exact overlap of at least one of the cells\n" ;

			    // write message to the command history window //               
			    gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL,
					    NULL, text, strlen(text));

			    listen_motion = TRUE;

			    INVALID_MOVE = TRUE;

			    return FALSE;
			}
		    }


		    // move all the selected cells //
		    for (i = 0; i < number_of_selected_cells; i++)
			move_cell_by_offset(selected_cells[i], offset_x, offset_y);
		    bDesignAltered = TRUE ;
					


		}
		clean_up_colors();

		free(selected_cells);
		selected_cells = NULL;
		number_of_selected_cells = 0;
		window_move_selected_cell = NULL;

		listen_motion = FALSE;
		INVALID_MOVE = FALSE;
		set_selected_action (CLEAR, selected_cell_type) ;
		redraw_world();
		return FALSE;

	    }			//if INVALID_MOVE

	    // if no cells have been selected then select the cell at the currnt x,y for moving //
	    if (number_of_selected_cells == 0) {

		// check to see if there actually is a cell at these coords //
		if (select_cell_at_coords
		    (calc_world_x(event->x),
		     calc_world_y(event->y)) == NULL) return FALSE;

		selected_cells = malloc(sizeof(qcell *));
		number_of_selected_cells = 1;
		selected_cells[0] =
		    select_cell_at_coords(calc_world_x(event->x),
					  calc_world_y(event->y));
		selected_cells[0]->color = WHITE;
		window_move_selected_cell = selected_cells[0];

	    } else {

		window_move_selected_cell =
		    select_cell_at_coords(calc_world_x(event->x),
					  calc_world_y(event->y));

		// check to see if the user has actually selected a cell from the selection to move //                                   
		if (window_move_selected_cell == NULL) {
		    set_selected_action (SELECT_CELL, selected_cell_type) ;
		    number_of_selected_cells = 0;
		    free(selected_cells);
		    selected_cells = NULL;
		    set_selected_action (CLEAR, selected_cell_type) ;
		    clean_up_colors();
		    redraw_world();
		    return 0;
		}

		TAG = TRUE;

		// check to make sure that the selected cell is within all the window selected cells //
		for (i = 0; i < number_of_selected_cells; i++) {
		    selected_cells[i]->color = WHITE;
		    if (selected_cells[i] == window_move_selected_cell) {
			TAG = FALSE;
		    }
		}

		if (TAG == TRUE) {
		    set_selected_action (SELECT_CELL, selected_cell_type) ;
		    free(selected_cells);
		    selected_cells = NULL;
		    number_of_selected_cells = 0;
		    window_move_selected_cell = NULL;
		    set_selected_action (CLEAR, selected_cell_type) ;
		    clean_up_colors();
		    redraw_world();
		    return 0;
		}
	    }

	    listen_motion = TRUE;

	    break;

	case MIRROR_CELLS:

	    mirror_x0 = event->x;
	    mirror_y0 = event->y;

	    listen_motion = TRUE;

	    break;
	    
	case MEASURE_DISTANCE:

	    dist_x0 = event->x;
	    dist_y0 = event->y;
		dist_x1 = event->x;
	    dist_y1 = event->y;

	    listen_motion = TRUE;

	    break;

	case DRAW_CELL_ARRAY:

	    array_x0 = event->x;
	    array_y0 = event->y;
	    array_x1 = array_x0;
	    array_y1 = array_y0;
	
	    listen_motion = TRUE;
	    break;

	case SELECT_CELL:
      	  if (!(event->state & GDK_CONTROL_MASK))
	    {
	    free(selected_cells);
	    selected_cells = NULL;
	    number_of_selected_cells = 0;
	    window_move_selected_cell = NULL;

	    window_top_x = event->x;
	    window_top_y = event->y;


	    // reusing the zoom variables should be changed later //
	    zoom_top_x = event->x;
	    zoom_top_y = event->y;

	    listen_motion = TRUE;
	    break;
	    }
	  else
	    set_selected_action (PAN, selected_cell_type) ;
	    // ... and keep going on to "case PAN:" below ...

        case PAN:
	    {
	    if (GDK_2BUTTON_PRESS == event->type || GDK_3BUTTON_PRESS == event->type) break ;
	    show_grid_backup = SHOW_GRID ;
	    SHOW_GRID = FALSE ;
	    dist_x0 = dist_x1 = event->x;
	    dist_y0 = dist_y1 = event->y;
	    
	    gcs_set_rop (GDK_XOR) ;
//	    draw_grid () ;
	    draw_pan_arrow (dist_x0, dist_y0, dist_x1, dist_y1) ;

	    listen_motion = TRUE;
	    break ;
	    }

	case ROTATE_CELL:

	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
		rotate_cell(cellp, 3.14159 / 4.0);
		bDesignAltered = TRUE ;
		redraw_world();

	    }

	    break;



	case SELECT_CELL_AS_FIXED:

	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
	      	double dPolarization = calculate_polarization (cellp) ;
		get_fixed_polarization_from_user (GTK_WINDOW (main_window.main_window), &dPolarization) ;
		
		set_cell_polarization(cellp, dPolarization);
		set_cell_as_fixed(cellp);
	
		set_selected_action (CLEAR, selected_cell_type) ;
		bDesignAltered = TRUE ;
		clean_up_colors();
		redraw_world();


	    }


	    break;

	case SELECT_CELL_AS_INPUT:

	    cellp = NULL;
	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
	        char szName[256] = "" ;
      	      	text = "Selected cell for input\n" ;

		// write message to the command history window //               
		gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL, text, strlen(text));

		// set the cell as an input and make its index its input number //

      	      	if (get_name_from_user (GTK_WINDOW (main_window.main_window), szName, 256))
		  {
		  set_cell_label (cellp, szName) ;
		  set_cell_as_input(cellp);
		  VectorTable_add_input (pvt, cellp) ;
		  bDesignAltered = TRUE ;
		  }
		
		set_selected_action (CLEAR, selected_cell_type) ;
		clean_up_colors();
		redraw_world();


	    }
	    break;



	case SELECT_CELL_AS_OUTPUT:

	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
	      	char szName[256] = "" ;
	        text = "Selected cell for output\n" ;

		// write message to the command history window //               
		gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL, text, strlen(text));

      	      	if (get_name_from_user (GTK_WINDOW (main_window.main_window), szName, 256))
		  {
  		  set_cell_as_output(cellp);
  		  set_cell_label (cellp, szName) ;
		  bDesignAltered = TRUE ;
		  }

		set_selected_action (CLEAR, selected_cell_type) ;
		clean_up_colors();
		redraw_world();


	    }
	    break;

	case CHANGE_INPUT_PROPERTIES:

	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
		if (cellp->is_input == TRUE) {
		    char szName[256] = "" ;
		    g_snprintf (szName, 256, "%s", cellp->label) ;
		    if (get_name_from_user (GTK_WINDOW (main_window.main_window), szName, 256))
		      {
		      set_cell_label (cellp, szName) ;
		      bDesignAltered = TRUE ;
		      }

		    set_selected_action (CLEAR, selected_cell_type) ;
		    redraw_world();
		}


	    }
	    break;

	case DELETE_CELL:

	    cellp =
		select_cell_at_coords(calc_world_x(event->x),
				      calc_world_y(event->y));

	    if (cellp != NULL) {
	      	if (cellp->is_input)
		  VectorTable_del_input (pvt, cellp) ;
		delete_stdqcell(cellp);
		bDesignAltered = TRUE ;
		redraw_world();
	    }
	    break;

	}			//switch

    }				//if


    if (event->button == 2) {
	listen_motion = TRUE;

	window_top_x = event->x;
	window_top_y = event->y;

	zoom_top_x = event->x;
	zoom_top_y = event->y;

    }				//if

    if (event->button == 3) {
	set_selected_action (CLEAR, selected_cell_type) ;
    }



    return FALSE;
}

gboolean expose_event(GtkWidget * widget, GdkEvent * event,
		      gpointer user_data)
{
//    fprintf (stderr, "expose_event\n") ;

    AREA_WIDTH = (int) widget->allocation.width;
    AREA_HEIGHT = (int) widget->allocation.height;
    redraw_world();
    return FALSE;
}

gboolean configure_event(GtkWidget * widget, GdkEvent * event,
			 gpointer user_data)
{
    setup_rulers () ;
    return FALSE;
}

void on_preview_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  init_print_options (&print_options, first_cell) ;
  do_print_preview (&print_options, GTK_WINDOW (main_window.main_window), first_cell) ;
  }

void on_grid_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  get_grid_spacing_from_user (GTK_WINDOW (main_window.main_window), &grid_spacing) ;
  redraw_world () ;
}

void on_snap_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  }

void on_cell_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  get_cell_properties_from_user (GTK_WINDOW (main_window.main_window), &cell_options) ;
  }

void on_window_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  }

void on_layer_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  }

// toggle the snap to grid option //
void on_snap_to_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  SNAP_TO_GRID = ((GtkCheckMenuItem *) menuitem)->active;
  }

// toggle the show grid option //
void on_show_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  SHOW_GRID = ((GtkCheckMenuItem *) menuitem)->active;
  redraw_world();
  }

void file_operations (GtkWidget *widget, gpointer user_data)
  {
  char szFName[PATH_LENGTH] = "project.qca" ;
  int fFileOp = (int)user_data ;
  gboolean bRedraw = FALSE ;
  
  if ((OPEN == fFileOp || OPEN_RECENT == fFileOp) && bDesignAltered)
    {
    MBButton mbb = message_box (GTK_WINDOW (main_window.main_window), (MBButton)(MB_YES | MB_NO | MB_CANCEL), "Project Modified",
      "You have altered your design.  If you open another one, you will lose your changes.  Save first ?") ;
    if (MB_YES == mbb)
      {
      if (!do_save ()) return ;
      }
    else if (MB_CANCEL == mbb)
      return ;
    }

  if (OPEN_RECENT != fFileOp)
    {
    if (!get_file_name_from_user (GTK_WINDOW (main_window.main_window), 
      OPEN == fFileOp ? "Open Project" :
      SAVE == fFileOp ? "Save Project As" :
      IMPORT == fFileOp ? "Import Block" :
      EXPORT == fFileOp ? "Export Block" :
      "Select File",
      szFName, PATH_LENGTH)) return ;
    }
  else
    g_snprintf (szFName, PATH_LENGTH, "%s", (char *)gtk_object_get_data (GTK_OBJECT (widget), "file")) ;
    
  if (szFName[0] != 0 && *(base_name (szFName)) != 0)
    {
    switch (fFileOp)
      {
      case OPEN_RECENT:
      case OPEN:
	// -- Clear all the cells in the current design -- //
	clear_all_cells();
	
	if (NULL != open_project_file(szFName, &first_cell, &last_cell))
	  {
	  VectorTable_fill (pvt, first_cell) ;
	  add_to_recent_files (main_window.recent_files_menu, szFName, file_operations, (gpointer)OPEN_RECENT) ;
	  g_snprintf (current_file_name, PATH_LENGTH, "%s", szFName) ;
	  bRedraw = TRUE ;
	  }
	else
	  {
	  message_box (GTK_WINDOW (main_window.main_window), MB_OK, "Error", "File %s failed to open !", base_name (szFName)) ;
	  remove_recent_file (main_window.recent_files_menu, szFName, file_operations, (gpointer)OPEN_RECENT) ;
	  }
	break ;

      case SAVE:
	g_snprintf (current_file_name, PATH_LENGTH, "%s", szFName) ;
	if (!do_save ())
	  message_box (GTK_WINDOW (main_window.main_window), MB_OK, "Error", "Failed to create file %s !", base_name (szFName)) ;
	break ;

      case IMPORT:
	{
	qcell *pqc = NULL ;

        if (NULL != (pqc = import_block (szFName, &selected_cells, &number_of_selected_cells, &last_cell)))
	  {
	  VectorTable_add_inputs (pvt, pqc) ;
      	  window_move_selected_cell = selected_cells[0];
	  set_selected_action (MOVE_CELL, -1);
	  INVALID_MOVE = TRUE;
	  listen_motion = TRUE;
	  }
	break ;
	}

      case EXPORT:
	export_block (szFName, selected_cells, number_of_selected_cells) ;
	break ;
      }
    }
  if (bRedraw) redraw_world () ;
  }

// allow the user to select an input cell //
void on_create_input_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    char *text = "Select the cell to be marked as input\n" ;
    gtk_text_insert(GTK_TEXT (main_window.command_history), NULL, NULL, NULL, text, strlen(text));
    set_selected_action (SELECT_CELL_AS_INPUT, selected_cell_type) ;
}

void on_input_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    char *text = "Select the cell to edit\n";
    gtk_text_insert(GTK_TEXT (main_window.command_history), NULL, NULL, NULL, text, strlen(text));
    set_selected_action (CHANGE_INPUT_PROPERTIES, selected_cell_type) ;
}

void on_connect_output_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    char *text = "Select the cell to be marked as output\n" ;
    gtk_text_insert(GTK_TEXT (main_window.command_history), NULL, NULL, NULL, text, strlen(text));
    set_selected_action (SELECT_CELL_AS_OUTPUT, selected_cell_type) ;
}

void on_clock_select_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  gtk_widget_show (create_clock_select_dialog(GTK_WINDOW (main_window.main_window))) ;
}

void on_clock_increment_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data){

    int i;

    if (number_of_selected_cells > 0) {
		
		// -- increment the clock for each of the selected cells -- //
		for (i = 0; i < number_of_selected_cells; i++) {
			assert(selected_cells[i] != NULL);
			
			selected_cells[i]->clock++;
			selected_cells[i]->clock %= 4 ;
		}
		bDesignAltered = TRUE ;
		// -- deselect the cells and clean up selected colors -- //
		clean_up_colors();
		free(selected_cells);
		selected_cells = NULL;
		number_of_selected_cells = 0;
		redraw_world();
    }

}//on_clock_increment_menu_item_activate

void on_fixed_polarization_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    char *text = "Select the cell whose polarization is to be fixed\n";
    gtk_text_insert(GTK_TEXT (main_window.command_history), NULL, NULL, NULL, text, strlen(text));
    set_selected_action (SELECT_CELL_AS_FIXED, selected_cell_type) ;
}

void on_measure_distance_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    set_selected_action (MEASURE_DISTANCE, selected_cell_type) ;
}

void on_measurement_preferences1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_draw_dimensions_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_dimension_properties1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_draw_text_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_text_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_draw_arrow_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_arrow_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_draw_line_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_line_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_draw_rectangle_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_rectangle_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_save_output_to_file_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_logging_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}

void on_simulation_type_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  get_sim_type_from_user (GTK_WINDOW (main_window.main_window), &SIMULATION_TYPE, pvt) ;
}  //on_simulation_properties_menu_item_activate

void on_simulation_engine_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
get_sim_engine_from_user (GTK_WINDOW (main_window.main_window), &SIMULATION_ENGINE) ;
}  //on_simulation_engine_setup_menu_item_activate

void on_zoom_in_menu_item_activate(GtkMenuItem * menuitem,
				   gpointer user_data)
{
    zoom_in(main_window.drawing_area);
    setup_rulers () ;
}

void on_zoom_out_menu_item_activate(GtkMenuItem * menuitem,
				    gpointer user_data)
{
    zoom_out(main_window.drawing_area);
    setup_rulers () ;
}

// when activated will zoom to fit the entire die in the window
void on_zoom_die_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  zoom_die(main_window.drawing_area);
  setup_rulers () ;
}

void on_zoom_extents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  zoom_extents () ;
  setup_rulers () ;
}

// when clicked start placing cells each button click //
void on_insert_type_1_cell_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (DIAG_CELL, selected_cell_type = TYPE_1) ;
}

void on_insert_type_2_cell_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (DIAG_CELL, selected_cell_type = TYPE_2) ;
}

void on_insert_cell_array_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (DRAW_CELL_ARRAY, selected_cell_type) ;
}

void on_copy_cell_button_clicked(GtkButton * button, gpointer user_data)
{

    qcell new_cell;
    qcell *cell;
    int i;

    if (number_of_selected_cells > 0) {

	assert(selected_cells != NULL);

	for (i = 0; i < number_of_selected_cells; i++) {

	    new_cell.x = 0;
	    new_cell.y = 0;

	    // force default cell //
	    new_cell.number_of_dots = 0;
	    new_cell.cell_width = 0;
	    new_cell.cell_height = 0;

	    cell = add_stdqcell(&new_cell, FALSE, selected_cell_type);

	    if (cell == NULL) {
		printf ("memory allocation error in on_copy_cell_button_clicked\n");
		exit(1);
	    }

	    assert(selected_cells[i] != NULL);

	    cell_copy(cell, selected_cells[i]);
	    selected_cells[i] = cell;
	}

	redraw_world();
	window_move_selected_cell = selected_cells[0];
	set_selected_action (MOVE_CELL, selected_cell_type) ;
	INVALID_MOVE = TRUE;
	listen_motion = TRUE;

    }
}

void on_move_cell_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (MOVE_CELL, selected_cell_type) ;
}

void on_rotate_cell_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (ROTATE_CELL, selected_cell_type) ;
}

void on_mirror_button_clicked(GtkButton * button, gpointer user_data)
{
    set_selected_action (MIRROR_CELLS, selected_cell_type) ;
}

void on_delete_cells_button_clicked(GtkButton * button, gpointer user_data)
{

    int i;

    // -- if there are cells already selected delete them first -- //
    if (number_of_selected_cells > 0) {
	for (i = 0; i < number_of_selected_cells; i++)
	    {
	    if (selected_cells[i]->is_input)
	      VectorTable_del_input (pvt, selected_cells[i]) ;
	    delete_stdqcell(selected_cells[i]);
	    bDesignAltered = TRUE ;
	    }

	// free up the memory from the selected cell array of pointers //
	free(selected_cells);
	selected_cells = NULL;
	number_of_selected_cells = 0 ;
	window_move_selected_cell = NULL;

	// -- redraw so that the deleted cells disappear from the screen -- //
	redraw_world();
    }

    set_selected_action (DELETE_CELL, selected_cell_type) ;
}

void on_command_entry_changed(GtkEditable * editable, gpointer user_data)
{
}

void on_new_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    if (bDesignAltered)
      {
      MBButton mbb = message_box (GTK_WINDOW (main_window.main_window), (MBButton)(MB_YES | MB_NO | MB_CANCEL), "Project Modified",
	"You have altered your design.  If you start a new one, you will lose your changes.  Save first ?") ;
      if (MB_YES == mbb)
      	{
	if (!do_save ()) return ;
	}
      else if (MB_CANCEL == mbb)
      	return ;
      }
    current_file_name[0] = 0 ;
    clear_all_cells();
    redraw_world();
    VectorTable_fill (pvt, first_cell) ;
    bDesignAltered = FALSE ;
}

void on_save_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  do_save () ;
  }

void on_print_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  if (get_print_properties_from_user (GTK_WINDOW (main_window.main_window), &print_options, first_cell))
    print_world (&print_options, first_cell) ;
  }

void on_project_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  get_project_properties_from_user (GTK_WINDOW (main_window.main_window), &subs_width, &subs_height) ;
  redraw_world () ;
  }

void on_close_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{    
  if (bDesignAltered)
    {
      MBButton mbb = message_box (GTK_WINDOW (main_window.main_window), (MBButton)(MB_YES | MB_NO | MB_CANCEL), "Project Modified",
				  "You have altered your design.  If you close this design, you will lose your changes.  Save first ?") ;
      if (MB_YES == mbb)
      	{
	if (!do_save ()) return ;
	}
      else if (MB_CANCEL == mbb)
      	return ;
      }

    clear_all_cells () ;
    redraw_world () ;
    current_file_name[0] = 0 ;
    bDesignAltered = FALSE;
}

// quit QCADesigner selected from menu //
void on_quit_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    if (bDesignAltered)
      {
      MBButton mbb = message_box (GTK_WINDOW (main_window.main_window), (MBButton)(MB_YES | MB_NO | MB_CANCEL), "Project Modified",
	"You have altered your design.  If you exit QCADesigner, you will lose your changes.  Save first ?") ;
      if (MB_YES == mbb)
      	{
	if (!do_save ()) return ;
	}
      else if (MB_CANCEL == mbb)
      	return ;
      }
    gtk_main_quit () ;
}


void on_undo_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_redo_meu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_copy_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_cut_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_paste_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_preferences_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_start_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  simulation_data *sim_data = run_simulation(SIMULATION_ENGINE, SIMULATION_TYPE);
  if (NULL != sim_data)
    show_graph_dialog (GTK_WINDOW (main_window.main_window), sim_data) ;
}

void on_random_fault_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  get_random_fault_params_from_user (GTK_WINDOW (main_window.main_window), &max_response_shift, &affected_cell_probability) ;
}

void on_pause_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_stop_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_reset_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}

void on_calculate_ground_state_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data){
	calculate_ground_state(SIMULATION_ENGINE);
}

void on_animate_test_simulation_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data){
/*
	nonlinear_approx_options.animate_simulation = 1;
	run_simulation(SIMULATION_ENGINE);
	nonlinear_approx_options.animate_simulation = 0;
*/
}

void on_contents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_search_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}
void on_about_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
show_about_dialog (GTK_WINDOW (main_window.main_window), FALSE) ;
}

///////////////////////////////////////////////////////////////////
///////////////////////// HELPERS /////////////////////////////////
///////////////////////////////////////////////////////////////////

void draw_pan_arrow (int x1, int y1, int x2, int y2)
  {
  int x1_1 = 0, y1_1 = 0, x1_2 = 0, y1_2 = 0 ;
  GdkWindow *pwnd = main_window.drawing_area->window ;
  GdkGC *pgc = main_window.drawing_area->style->white_gc ;
  
  gdk_draw_arc (pwnd, pgc, FALSE, x1 - 5, y1 - 5, 11, 11, 0, 360 * 64) ;
  gdk_draw_line (pwnd, pgc, x1, y1, x2, y2) ;
  if (!(x1 == x2 && y1 == y2))
    {
    get_arrow_head_coords (x1, y1, x2, y2, 10, &x1_1, &y1_1, -30) ;
    get_arrow_head_coords (x1, y1, x2, y2, 10, &x1_2, &y1_2, 30) ;
    gdk_draw_line (pwnd, pgc, x1_1, y1_1, x2, y2) ;
//  gdk_draw_line (pwnd, pgc, x1_2, y1_2, x2, y2) ;
    }
  }

void get_arrow_head_coords (int xRef, int yRef, int xTip, int yTip, int length, int *px, int *py, int deg)
/* returns coordinates such that xTip and yTip are the origin,
   the line (xRef,yRef) <-> (xTip,yTip) is the positive x axis,
   the angle [(*px,*py),(xTip,yTip),(xRef,yRef)] is deg degrees, 
   and the line (*px,*py) <-> (xTip,yTip) is length pixels long */
  {
  double angle, angOffset = deg * M_PI / 180.0 ;

  if (yRef < yTip)
    deg *= -1 ;
  
  angle = acos ((xTip - xRef) / (sqrt (((xTip - xRef) * (xTip - xRef)) + ((yTip - yRef) * (yTip - yRef))))) ;
  *px = xTip - length * cos (angle - angOffset) ;
  *py = yTip + (length * sin (angle - angOffset)) * ((yTip > yRef) ? -1.0 : 1.0) ;
  }

void setup_rulers ()
  {
  GdkModifierType mask ;
  double world_x1, world_y1, world_x2, world_y2, world_x, world_y ;
  int xOffset = 0, yOffset = 0, x = 0, y = 0,
    xFrame = main_window.drawing_area_frame->allocation.x,
    yFrame = main_window.drawing_area_frame->allocation.y,
    cxFrame = main_window.drawing_area_frame->allocation.width,
    cyFrame = main_window.drawing_area_frame->allocation.height,
    xDA = main_window.drawing_area->allocation.x,
    yDA = main_window.drawing_area->allocation.y ;
    
  gdk_window_get_pointer (main_window.drawing_area->window, &x, &y, &mask) ;
  
  world_x1 = calc_world_x (xOffset = xFrame - xDA) ;
  world_y1 = calc_world_y (yOffset = yFrame - yDA) ;
  world_x = calc_world_x (x - xOffset) ;
  world_y = calc_world_y (y - yOffset) ;
  world_x2 = calc_world_x (xOffset + cxFrame + 1) ;
  world_y2 = calc_world_y (yOffset + cyFrame + 1) ;
  
  world_x = CLAMP (world_x, world_x1, world_x2) ;
  world_y = CLAMP (world_y, world_y1, world_y2) ;

  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2) ;
  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_y1, world_y2) ;

  gtk_ruler_set_range (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2, world_x, world_x2) ;
  gtk_ruler_set_range (GTK_RULER (main_window.vertical_ruler), world_y1, world_y2, world_y, world_y2) ;
  }

void set_ruler_scale (GtkRuler *ruler, double dLower, double dUpper)
  {
  double dRange = dUpper - dLower ;
  int iPowerOfTen = ceil (log10 (dRange)), Nix = 0, iPowerOfDivisor = 0 ;
  double dScale = pow (10, iPowerOfTen) ;
  double dTmp = 0 ;
  
  if (dRange < dScale / 2)
    {
    dScale /= 2 ;
    iPowerOfDivisor = 1 ;
    }
  
  for (Nix = 9 ; Nix > -1 ; Nix--)
    {
    ruler->metric->ruler_scale[Nix] = floor (dScale / ((double)(1 << iPowerOfDivisor))) ;
    iPowerOfDivisor++ ;
    iPowerOfDivisor %= NUMBER_OF_RULER_SUBDIVISIONS ;
    if (0 == iPowerOfDivisor)
      dScale = pow (10, dTmp = floor (log10 (dScale / NUMBER_OF_RULER_SUBDIVISIONS))) ;
    }
  }

void gcs_set_rop (GdkFunction func)
  {
  if (NULL != global_gc)
    gdk_gc_set_function (global_gc, func) ;
  if (NULL != main_window.drawing_area->style->white_gc)
    gdk_gc_set_function (main_window.drawing_area->style->white_gc, func) ;
  if (NULL != main_window.drawing_area->style->black_gc)
    gdk_gc_set_function (main_window.drawing_area->style->black_gc, func) ;
  }

void set_selected_action (int action, int cell_type)
  {
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.insert_type_1_cell_button), on_insert_type_1_cell_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.insert_type_1_cell_button),
    DIAG_CELL == action &&  TYPE_1 == cell_type) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.insert_type_1_cell_button), on_insert_type_1_cell_button_clicked, NULL) ;
  
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.insert_type_2_cell_button), on_insert_type_2_cell_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.insert_type_2_cell_button),
    DIAG_CELL == action &&  TYPE_2 == cell_type) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.insert_type_2_cell_button), on_insert_type_2_cell_button_clicked, NULL) ;
  
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.insert_cell_array_button), on_insert_cell_array_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.insert_cell_array_button),
    DRAW_CELL_ARRAY == action) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.insert_cell_array_button), on_insert_cell_array_button_clicked, NULL) ;
  
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.move_cell_button), on_move_cell_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.move_cell_button),
    MOVE_CELL == action) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.move_cell_button), on_move_cell_button_clicked, NULL) ;
  
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.rotate_cell_button), on_rotate_cell_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.rotate_cell_button),
    ROTATE_CELL == action) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.rotate_cell_button), on_rotate_cell_button_clicked, NULL) ;
  
  gtk_signal_handler_block_by_func (GTK_OBJECT (main_window.delete_cells_button), on_delete_cells_button_clicked, NULL) ;
  gtk_toggle_button_set_active (
    GTK_TOGGLE_BUTTON (main_window.delete_cells_button),
    DELETE_CELL == action) ;
  gtk_signal_handler_unblock_by_func (GTK_OBJECT (main_window.delete_cells_button), on_delete_cells_button_clicked, NULL) ;
  selected_action = action ;
  }

gboolean do_save ()
  {
  char szFName[PATH_LENGTH] = "" ;
  
  g_snprintf (szFName, PATH_LENGTH, "%s", current_file_name) ;
  
  if (0 == szFName[0])
    {
    if (!get_file_name_from_user (GTK_WINDOW (main_window.main_window), "Save Project As", szFName, PATH_LENGTH)) return FALSE ;
    if (0 == szFName[0] || 0 == base_name (szFName)) return FALSE ;
    }

  if (create_file(szFName, first_cell))
    {
    add_to_recent_files (main_window.recent_files_menu, szFName, file_operations, (gpointer)OPEN_RECENT) ;
    bDesignAltered = FALSE ;
    g_snprintf (current_file_name, PATH_LENGTH, "%s", szFName) ;
    return TRUE ;
    }
  else
    message_box (GTK_WINDOW (main_window.main_window), MB_OK, "Error", "Failed to create file %s !", base_name (szFName)) ;
  
  return FALSE ;
  }
