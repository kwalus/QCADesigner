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
// File Description:                                    //
// This source handles cad functions such as move cells,//
// cell arrays, etc                                     //
//////////////////////////////////////////////////////////

// Standard includes //
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// QCADesigner includes //
#include "stdqcell.h"
#include "simulation.h"
#include "cad.h"
#include "cad_util.h"
#include "callbacks.h"
#include "init.h"

#define DBG_CAD(s)

static int world_to_real_x (float x) ;
static int world_to_real_y (float y) ;
static float real_to_world_x (int x) ;
static float real_to_world_y (int y) ;
static int world_to_real_cx (float cx) ;
static int world_to_real_cy (float cy) ;
static float real_to_world_cx (int cx) ;
//static float real_to_world_cy (int cy) ;
static void redraw_all_cells(GdkDrawable *d, GdkGC *gc, GQCell *first_cell);
static void draw_subs(GdkDrawable *d, GdkGC *gc) ;
static void draw_cell_dots_showing_polarization(GdkDrawable *d, GdkGC *gc, GQCell *cell);
static void draw_temp_stdqcell (GdkDrawable *d, GdkGC *gc, int cell_type, double x, double y);

//!previous scale multiplier used when going to previous view.
static double previous_scale = 0;
static double previous_subs_top_x=0; 
static double previous_subs_top_y=0;
static GdkFont *global_font = NULL ;
static GdkColor clrBlack  = {0, 0x0000, 0x0000, 0x0000} ;
static GdkColor clrWhite  = {0, 0xFFFF, 0xFFFF, 0xFFFF} ;
static GdkColor clrBlue   = {0, 0x0000, 0x0000, 0xFFFF} ;
static GdkColor clrCyan   = {0, 0x0000, 0xFFFF, 0xFFFF} ;

// -- current top coords of the substrate relative to top corner of drawing area -- //
static double subs_top_x = 100;
static double subs_top_y = 100;

//!scale multiplier in nanometers.
static double scale = 1; // [pixel/ nm]

extern cell_OP cell_options ;

//!Substrate height.
double subs_height = 3000;
//!Substrate width.
double subs_width = 6000;

//!grid spacing measured in nanometers.
double grid_spacing = 10;  

// -- Drawing Area width and height -- //
int AREA_WIDTH = 0;
int AREA_HEIGHT = 0;

// Initialize all module-level static variables that cannot be initialized at load-time
// Read:  GdkColor and GdkFont stuff ...
void cad_init ()
  {
  GdkColormap *clrmSys = gdk_colormap_get_system () ;
  
  global_font = gdk_font_load (QCAD_GDKFONT) ;
  gdk_color_alloc (clrmSys, &clrBlack) ;
  gdk_color_alloc (clrmSys, &clrWhite) ;
  gdk_color_alloc (clrmSys, &clrBlue) ;
  gdk_color_alloc (clrmSys, &clrCyan) ;
  }

//!Redraws all the design cells to the screen.
static void redraw_all_cells(GdkDrawable *d, GdkGC *gc, GQCell *first_cell){

	GQCell *cell = first_cell;
	
	while(cell != NULL){
		draw_stdqcell(d, gc, cell);
		cell = cell->next;
	}
	
}//redraw_all_cells

//!Redraws all selected cells
void redraw_selected_cells(GdkDrawable *d, GdkGC *gc, GQCell **selected_cells, int number_of_selected_cells){

  int i;
  for(i=0;i<number_of_selected_cells;i++)
    if (NULL != selected_cells[i])
      draw_stdqcell (d, gc, selected_cells[i]);
	
}//redraw_selected_cells

//-------------------------------------------------------------------//
void scale_design(GQCell *first_cell, double scale){
	
	GQCell *cell = first_cell;
	
	while(cell != NULL){
		gqcell_scale(cell, scale);
		cell = cell->next;
	}
}



//-------------------------------------------------------------------//

//!Draws a rectangle representing the substrate onto the drawing area.
static void draw_subs(GdkDrawable *d, GdkGC *gc)
  {
  int x_real = world_to_real_x (0),
      y_real = world_to_real_y (0),
      cx_real = world_to_real_x (subs_width) - x_real,
      cy_real = world_to_real_y (subs_height) - y_real ;
  
  gdk_gc_set_foreground (gc, &clrWhite) ;
  
  gdk_draw_rectangle (d, gc, FALSE, x_real, y_real, cx_real, cy_real) ;
  }//draw_subs


//-------------------------------------------------------------------//

//!Draws the grid onto the screen.
void draw_grid(GdkDrawable *d, GdkGC *gc){
	
	double i;
	double j;
	int k;
	int number_of_points;
	double width = world_to_real_cx (subs_width);
	double height = world_to_real_cy (subs_height);
	double spacing = grid_spacing * scale;
	GdkPoint *grid;
	
	// No need to draw a million points if you can't seem them
	// If the scale is below a threshold I double the spacing
	if(scale < 1)spacing*=2;
	if(scale < 0.3)spacing*=2;
	if(scale < 0.1)spacing*=2;
	if(scale < 0.05)spacing*=2;
	if(scale < 0.01)spacing*=2;
	
	// Estimate the total number of points to be drawn
	number_of_points = (width/spacing)*(height/spacing) + 1;
        
        if (number_of_points <= 0) return ;
        
	grid = malloc(number_of_points*sizeof(GdkPoint));
	
	//Fill the array of grid points
	k = 0;
	for(i = spacing; i < width; i+= spacing)
		for(j = spacing; j < height; j += spacing){
			
			if(subs_top_x + i < 0 || subs_top_y + j < 0 || subs_top_x + i > AREA_WIDTH || subs_top_y + j > AREA_HEIGHT){
				grid[k].x = 0;
				grid[k].y = 0;
				k++;
				continue;
				}
			
			grid[k].x = subs_top_x + i;
			grid[k].y = subs_top_y + j;
			k++;		
			
			}
			
	gdk_gc_set_foreground (gc, &clrWhite) ;
	gdk_draw_points(d, gc, grid, k);
	
	//Free the memory from the pixelarray
	free(grid);
	grid = NULL;

}//draw_grid


//-------------------------------------------------------------------//

//!Calls all the redraw functions to regenerate the image.
void redraw_world(GdkDrawable *dWnd, GdkGC *gcWnd, GQCell *first_cell, gboolean bShowGrid)
  {
  GdkGC *gc = gcWnd ;
  GdkPixmap *pm = gdk_pixmap_new (dWnd, AREA_WIDTH, AREA_HEIGHT, -1) ;
  GdkDrawable *d = dWnd ;
  if (NULL != pm)
    gc = gdk_gc_new (d = GDK_DRAWABLE (pm)) ;
  
  gdk_window_clear (dWnd) ;
  
  DBG_CAD (fprintf (stderr, "redraw_world\n")) ;
  draw_subs(d, gc);
  if(bShowGrid)draw_grid(d, gc);
  redraw_all_cells(d, gc, first_cell);
  
  if (NULL != pm)
    {
    gdk_draw_drawable (dWnd, gcWnd, GDK_DRAWABLE (pm), 0, 0, 0, 0, -1, -1) ;
    g_object_unref (G_OBJECT (gc)) ;
    g_object_unref (G_OBJECT (pm)) ;
    }
  }//redraw_world

//-------------------------------------------------------------------//

void pan (int cx, int cy)
  {
  subs_top_x += cx ;
  subs_top_y += cy ;
  }

//-------------------------------------------------------------------//

//Calculates bounding box around cells //
void get_extents(GQCell *first_cell, double *pmin_x, double *pmin_y, double *pmax_x, double *pmax_y){
	
	GQCell *cell = first_cell;
		
	// if there are no cells then extents makes no sense so we zoom to fit the die //	
	if(first_cell == NULL){
	
	*pmin_x = 0 ;
	*pmin_y = 0 ;
	*pmax_x = subs_width ;
	*pmax_y = subs_height ;
	
	} else {
	
	*pmax_x = cell->x + cell->cell_width / 2;
	*pmax_y = cell->y + cell->cell_height / 2 ;
	
	*pmin_x = cell->x - cell->cell_width / 2;
	*pmin_y = cell->y - cell->cell_height / 2;
	
	cell = cell->next;
	
	// -- find the true mins and maxes //
	while(cell != NULL){
		if(cell->x - cell->cell_width / 2 <= *pmin_x) *pmin_x = cell->x - cell->cell_width / 2;
		if(cell->x + cell->cell_width / 2 >= *pmax_x) *pmax_x = cell->x + cell->cell_width / 2;
		if(cell->y - cell->cell_height / 2 <= *pmin_y) *pmin_y = cell->y - cell->cell_height / 2;
		if(cell->y + cell->cell_height / 2 >= *pmax_y) *pmax_y = cell->y + cell->cell_height / 2;
		cell = cell->next;
	}
	
	// -- make some extra room -- //
	
//	*pmin_x -= PAN_STEP;
//	*pmin_y -= PAN_STEP;
//	*pmax_x += PAN_STEP;
//	*pmax_y += PAN_STEP;
	
	}
	
//      printf (
//	  "*pmin_x = %d\n"
//	  "*pmin_y = %d\n"
//	  "*pmax_x = %d\n"
//	  "*pmax_y = %d\n", *pmin_x, *pmin_y, *pmax_x, *pmax_y) ;
	
}//get_extents

//-------------------------------------------------------------------//

//!Zooms such that all cells fit on the screen //
void zoom_extents(GQCell *first_cell){
  double 
    min_x, min_y, max_x, max_y,
    dcxArea = AREA_WIDTH, dcyArea = AREA_HEIGHT,
    dxFit = 0, dyFit = 0, dcxFit = 0, dcyFit = 0, dcxNano, dcyNano ;
  
  
  if (NULL == first_cell)
    {
    subs_top_x = 0 ;
    subs_top_y = 0 ;
    }

  get_extents (first_cell, &min_x, &min_y, &max_x, &max_y) ;
  
  dcxFit = dcxNano = max_x - min_x ;
  dcyFit = dcyNano = max_y - min_y ;
  
  fit_rect_inside_rect (dcxArea, dcyArea, &dxFit, &dyFit, &dcxFit, &dcyFit) ;
  
  scale = (0 == dxFit ? AREA_WIDTH / dcxNano : AREA_HEIGHT / dcyNano) ;

  subs_top_x = dxFit - min_x * scale ;
  subs_top_y = dyFit - min_y * scale ;
}//zoom_extents

//-------------------------------------------------------------------//

//!Sets up a zoom such that the entire substrate fits in the window //
void zoom_die(){

	subs_top_x = PAN_STEP;
	subs_top_y = PAN_STEP;
	
	if ((float)AREA_WIDTH / (float)(subs_width+2*PAN_STEP) < (float)AREA_HEIGHT / (float)(subs_height+2 * PAN_STEP)){
		scale = (float)AREA_WIDTH / (float)(subs_width+2 * PAN_STEP);
	}else{
		scale = (float)AREA_HEIGHT / (float)(subs_height+2 * PAN_STEP);
	}

}//zoom_die

//-------------------------------------------------------------------//

//!Zooms out a little
void zoom_out(){
	
	zoom_window(-30, -30, AREA_WIDTH + 30, AREA_HEIGHT + 30);
}//zoom_out

//-------------------------------------------------------------------//

//!Zooms in a little
void zoom_in(){
	zoom_window(30, 30, AREA_WIDTH - 30, AREA_HEIGHT - 30);
}//zoom_in


//-------------------------------------------------------------------//

//!Zooms to the provided window dimensions.
void zoom_window(int top_x, int top_y, int bot_x, int bot_y){

  int xMin, yMin, xMax, yMax, cxWindow, cyWindow ;
  double 
    dcxWindow = (double)(cxWindow = (xMax = MAX (top_x, bot_x)) - (xMin = MIN (top_x, bot_x))),
    dcyWindow = (double)(cyWindow = (yMax = MAX (top_y, bot_y)) - (yMin = MIN (top_y, bot_y))),
    dx = 0, dy = 0,
    dcxArea = (double)AREA_WIDTH, 
    dcyArea = (double)AREA_HEIGHT,
    scale_factor = 0;
  
  previous_scale = scale ;
  previous_subs_top_x = subs_top_x ;
  previous_subs_top_y = subs_top_y ;
  
  fit_rect_inside_rect (dcxArea, dcyArea, &dx, &dy, &dcxWindow, &dcyWindow) ;
  
  scale_factor = dcxWindow / (double)cxWindow ;
  
  pan (
    dx - ((xMin - subs_top_x) * scale_factor + subs_top_x), 
    dy - ((yMin - subs_top_y) * scale_factor + subs_top_y)) ;
  
  scale *= scale_factor ;
}//zoom_window

//-------------------------------------------------------------------//
//!Draws a ruler to the screen showing the distance between the two coordinates
void draw_ruler(GdkDrawable *d, GdkGC *gc, int dist_x0, int dist_y0, int dist_x1, int dist_y1)
  {
  int i, j;
  char distance[20];

  // set up the color //
  gdk_gc_set_function (gc, GDK_XOR);

  gdk_gc_set_foreground(gc, &clrCyan);
  gdk_gc_set_background(gc, &clrCyan);

  // -- Draw a Horizantle Ruler -- //
  if (abs(dist_x0 - dist_x1) >= abs(dist_y0 - dist_y1))
    {
    gdk_draw_line (d, gc, dist_x0, dist_y0, dist_x1, dist_y0);

    j = 0;

    g_snprintf(distance, 20, "%f nm", real_to_world_cx (abs(dist_x0 - dist_x1)));

    gdk_draw_string(d, global_font, gc, dist_x1, dist_y0 - 10, distance);

    if (dist_x0 - dist_x1 < 0)
      {
      for (i = 0; i < abs(dist_x0 - dist_x1); i += grid_spacing)
        {
        if (j % 2 == 0)
	  gdk_draw_line(d, gc, dist_x0 + i, dist_y0 - 5, dist_x0 + i, dist_y0 + 5);
        else
	  gdk_draw_line(d, gc, dist_x0 + i, dist_y0 - 2, dist_x0 + i, dist_y0 + 2);
        j++;
        }
      } 
    else 
      {
      for (i = 0; i > -abs(dist_x0 - dist_x1); i -= grid_spacing)
        {
	if (j % 2 == 0)
	  gdk_draw_line(d, gc, dist_x0 + i, dist_y0 - 5, dist_x0 + i, dist_y0 + 5);
	else
	  gdk_draw_line(d, gc, dist_x0 + i, dist_y0 - 2, dist_x0 + i, dist_y0 + 2);
	j++;
	}
      }

    // -- Draw a Vertical Ruler -- //
    }
  else
    {
    gdk_draw_line(d, gc, dist_x0, dist_y0, dist_x0, dist_y1);

    j = 0;

    g_snprintf(distance, 20, "%f nm", real_to_world_cx (abs(dist_y0 - dist_y1)));

    gdk_draw_string(d, global_font, gc, dist_x0 + 10, dist_y1, distance);

    if (dist_y0 - dist_y1 < 0)
      {
      for (i = 0; i < abs (dist_y0 - dist_y1) ; i += grid_spacing)
        {
        if (j % 2 == 0)
          gdk_draw_line(d, gc, dist_x0 - 5, dist_y0 + i, dist_x0 + 5, dist_y0 + i);
        else
	  gdk_draw_line(d, gc, dist_x0 - 2, dist_y0 + i, dist_x0 + 2, dist_y0 + i);
        j++;
        }
      }
    else
      {
      for (i = 0; i > -abs(dist_y0 - dist_y1); i -= grid_spacing)
        {
        if (j % 2 == 0)
          gdk_draw_line(d, gc, dist_x0 - 5, dist_y0 + i, dist_x0 + 5, dist_y0 + i);
        else
	  gdk_draw_line(d, gc, dist_x0 - 2, dist_y0 + i, dist_x0 + 2, dist_y0 + i);
        j++;
        }
      }
    }
  }//draw_ruler

//-------------------------------------------------------------------//

//!Draws a temporary array of cells to show the user how many cells will be created in the arrray  //
void draw_temp_array(GdkDrawable *d, GdkGC *gc, int cell_type, double x0, double y0, double x1, double y1){

    double x=0;
    double y=0;
	double oldx=0;
	double oldy=0;
	int i;

	// if the user drew a horizontal line //
    if ((y0 - y1) == 0) {

		if(x1>x0)for (x = calc_world_x(x0), i=0; x <= calc_world_x(x1); x += grid_spacing, i++) {

			x = grid_world_x(x);
			y = grid_world_y(calc_world_y(y0));

			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && cell_type == TYPE_1){
				if (x < oldx + cell_options.type_1_cell_width)continue;
			}else if(i != 0 && cell_type == TYPE_2){
				if (x < oldx + cell_options.type_2_cell_width)continue;
			}

			oldx=x;
			oldy=y;

			draw_temp_stdqcell (d, gc, cell_type, x, y);
		}

		if(x0>x1)for (x = calc_world_x(x0), i=0; x >= calc_world_x(x1); x -= grid_spacing, i++) {
	
			//printf("x0=%f x1=%f x=%f\n",calc_world_x(x0),calc_world_x(x1),x);
			
			x = grid_world_x(x);
			y = grid_world_y(calc_world_y(y0));
			
			// make sure that the cell does not overlap with previous array cell //
			if(i !=0 && cell_type == TYPE_1){
				if (x > oldx - cell_options.type_1_cell_width)continue;		
			}else if(i != 0 && cell_type == TYPE_2){
				if (x > oldx - cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;

			draw_temp_stdqcell (d, gc, cell_type, x, y);
		}

	// if the user drew a vertical line //
    } else if ((x0 - x1) == 0) {
		
		if(y1>y0)for (y = calc_world_y(y0), i=0; y <= calc_world_y(y1); y += grid_spacing, i++) {

			x = grid_world_x(calc_world_x(x0));
			y = grid_world_y(y);
	
			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && cell_type == TYPE_1){
				if (y < oldy + cell_options.type_1_cell_width)continue;
			}else if(i != 0 && cell_type == TYPE_2){
				if (y < oldy + cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;
			
			draw_temp_stdqcell (d, gc, cell_type, x, y);
		}
		
		if(y0>y1)for (y = calc_world_y(y0), i=0; y >= calc_world_y(y1); y -= grid_spacing, i++) {

				x = grid_world_x(calc_world_x(x0));
				y = grid_world_y(y);
	
			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && cell_type == TYPE_1){
				if (y > oldy - cell_options.type_1_cell_width)continue;
			}else if(i != 0 && cell_type == TYPE_2){
				if (y > oldy - cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;
			
			draw_temp_stdqcell (d, gc, cell_type, x, y);
		}
    }


}//draw_temp_array

//-------------------------------------------------------------------//

//!Draws a temporary QCA cell to the drawing area at the given coordinates.
static void draw_temp_stdqcell (GdkDrawable *d, GdkGC *gc, int cell_type, double x, double y){
        double cxWorld = -1, cyWorld = -1 ;
        int x_top = -1, y_top = -1, cx = -1, cy = -1 ;
        
        if (TYPE_1 == cell_type)
          {
          cxWorld = cell_options.type_1_cell_width ;
          cyWorld = cell_options.type_1_cell_height ;
          }
        else
          {
          cxWorld = cell_options.type_2_cell_width ;
          cyWorld = cell_options.type_2_cell_height ;
          }
        
        x_top = world_to_real_x (x - cxWorld / 2) ;
        y_top = world_to_real_y (y - cyWorld / 2) ;
        cx = world_to_real_cx (cxWorld) ;
        cy = world_to_real_cy (cyWorld) ;
        
        if (x_top > AREA_WIDTH || y_top > AREA_HEIGHT || x_top + cx < 0 || y_top + cy < 0) return ;
        
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
        
        gdk_gc_set_foreground (gc, &clrBlue) ;
        
	gdk_gc_set_function (gc, GDK_XOR);
	
        gdk_draw_rectangle (d, gc, FALSE, x_top, y_top, cx, cy) ;
}//draw_temp_stdqcell

//-------------------------------------------------------------------//

//!Draws a QCA cell to the drawing area at the coords provided by the argument pointer.
void draw_stdqcell(GdkDrawable *d, GdkGC *gc, GQCell *cell){
      
	int top_corner_x = world_to_real_x (cell->x - cell->cell_width/2),
	    top_corner_y = world_to_real_y (cell->y - cell->cell_height/2),
            real_cx = world_to_real_cx (cell->cell_width), 
            real_cy = world_to_real_cy (cell->cell_height) ;
	
	// -- Don't need to draw the cell if it does not appear on the screen anyway -- //
	if(top_corner_x > AREA_WIDTH || top_corner_y > AREA_HEIGHT || 
           top_corner_x + real_cx < 0 || top_corner_y + real_cy < 0) return;
	
	
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
	gdk_gc_set_foreground (gc, cell->color) ;

	// -- draw the rectangle that is the outline of the cell -- //
	gdk_draw_rectangle(d, gc, FALSE, top_corner_x, top_corner_y, real_cx, real_cy);
	
	draw_cell_dots_showing_polarization (d, gc, cell);
		
	if(cell->is_fixed){
	  char text[10] = "" ;
	  g_snprintf(text, 10, "%1.2f", gqcell_calculate_polarization(cell));
	  gdk_draw_string(d, global_font, gc, top_corner_x, top_corner_y - 10, text);
	}

        if(cell->is_input || cell->is_output)
          gdk_draw_string(d, global_font, gc, top_corner_x, top_corner_y - 10, cell->label);
}//draw_stdqcell

//! Function to redraw the cell dots according to the cells polarization//
// used with the real-time animations or when showing the ground state //

static void draw_cell_dots_showing_polarization (GdkDrawable *d, GdkGC *gc, GQCell *cell){
	int i;
	int top_corner_x = world_to_real_x (cell->x - cell->cell_width/2),
	    top_corner_y = world_to_real_y (cell->y - cell->cell_height/2),
            real_cx = world_to_real_cx (cell->cell_width), 
            real_cy = world_to_real_cy (cell->cell_height) ;
	
	// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
	if(top_corner_x > AREA_WIDTH || top_corner_y > AREA_HEIGHT)return;
	if(top_corner_x + real_cx < 0 || real_cy < 0)return;
	
	
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
			
	for(i = 0; i < cell->number_of_dots; i++)
          {
          gdk_gc_set_foreground (gc, &clrBlack) ;
	  gdk_draw_arc (d, gc, TRUE, 
            world_to_real_x (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2), 
            world_to_real_y (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2), 
            world_to_real_cx (cell->cell_dots[i].diameter), 
            world_to_real_cy (cell->cell_dots[i].diameter), 0, 360 * 64);
          gdk_gc_set_foreground (gc, cell->color) ;
	  gdk_draw_arc (d, gc, FALSE, 
            world_to_real_x (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2), 
            world_to_real_y (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2), 
            world_to_real_cx (cell->cell_dots[i].diameter), 
            world_to_real_cy (cell->cell_dots[i].diameter), 0, 360 * 64);
	  gdk_draw_arc (d, gc, TRUE,  
            world_to_real_x (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE), 
            world_to_real_y (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE), 
            world_to_real_cx (cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE), 
            world_to_real_cy (cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE), 0, 360 * 64);
          }
}//draw_cell_dots

//-------------------------------------------------------------------//

//!Resets all cell colors to the natural values determined by the cell type. 
//!Will clear any user changes such as those caused by cell selection.
void clean_up_colors(GQCell *first_cell){
  GQCell *cell = first_cell;

  // -- reset the colors of the previously selected cells -- //
  while(cell != NULL){
    gqcell_reset_colour (cell) ;
    cell = cell->next;
  }
}//clean_up_colors

//-------------------------------------------------------------------//

// Add cells to an existing selection
void add_cells_to_selection(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, int top_x, int top_y, int bot_x, int bot_y)
  {
  GQCell *cell = first_cell ;
  double 
    topxWorld = real_to_world_x (MIN (top_x, bot_x)), 
    topyWorld = real_to_world_y (MIN (top_y, bot_y)), 
    botxWorld = real_to_world_x (MAX (top_x, bot_x)), 
    botyWorld = real_to_world_y (MAX (top_y, bot_y)) ;
  int Nix ;
  
  if (top_x == bot_x && top_y == bot_y)
    {
    GQCell *cell_here = select_cell_at_coords (first_cell, topxWorld, topyWorld) ;
    
    if (NULL != cell_here)
      {
      for (Nix = 0 ; Nix < (*pnumber_of_selected_cells) ; Nix++)  
        if (cell_here == (*pselected_cells)[Nix])
          break ;
      if ((*pnumber_of_selected_cells) == Nix)
        {
        (*pselected_cells) = realloc ((*pselected_cells), ++(*pnumber_of_selected_cells) * sizeof (GQCell *)) ;
        gqcell_select (cell_here) ;
        (*pselected_cells)[(*pnumber_of_selected_cells) - 1] = cell_here ;
        }
      }
    }
  
  while (NULL != cell)
    {
    for (Nix = 0 ; Nix < (*pnumber_of_selected_cells) ; Nix++)  
      if (cell == (*pselected_cells)[Nix])
        break ;
    if ((*pnumber_of_selected_cells) == Nix)
      {
      if (cell->x >= topxWorld && cell->x <= botxWorld &&
          cell->y >= topyWorld && cell->y <= botyWorld)
        {
        (*pselected_cells) = realloc ((*pselected_cells), ++(*pnumber_of_selected_cells) * sizeof (GQCell *)) ;
        gqcell_select (cell) ;
        (*pselected_cells)[(*pnumber_of_selected_cells) - 1] = cell ;
        }
      }
    cell = cell->next ;
    }
  }

// Remove cells from an existing selection
void remove_cells_from_selection(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, GdkDrawable *window, GdkGC *gc, int top_x, int top_y, int bot_x, int bot_y)
  {
  GQCell *cell = first_cell ;
  double 
    topxWorld = real_to_world_x (MIN (top_x, bot_x)), 
    topyWorld = real_to_world_y (MIN (top_y, bot_y)), 
    botxWorld = real_to_world_x (MAX (top_x, bot_x)), 
    botyWorld = real_to_world_y (MAX (top_y, bot_y)) ;
  int Nix ;
  
  if (top_x == bot_x && top_y == bot_y)
    {
    GQCell *cell_here = select_cell_at_coords (first_cell, topxWorld, topyWorld) ;
    
    fprintf (stderr, "Deselecting single cell\n") ;
    
    if (NULL != cell_here)
      {
      fprintf (stderr, "Found single cell for deselection\n") ;
      for (Nix = 0 ; Nix < (*pnumber_of_selected_cells) ; Nix++)  
        if (cell_here == (*pselected_cells)[Nix])
          break ;
      
      fprintf (stderr, "The single cell is idx %d in selected_cells\n", Nix) ;
      
      if (Nix < (*pnumber_of_selected_cells))
        {
        gqcell_reset_colour ((*pselected_cells)[Nix]) ;
        draw_stdqcell (window, gc, (*pselected_cells)[Nix]) ;
        if (Nix < (*pnumber_of_selected_cells) - 1)
          memmove (&((*pselected_cells)[Nix]), &((*pselected_cells)[Nix + 1]), ((*pnumber_of_selected_cells) - Nix - 1) * sizeof (GQCell *)) ;
        (*pselected_cells) = realloc ((*pselected_cells), --(*pnumber_of_selected_cells) * sizeof (GQCell *)) ;
        }
      }
    }
  
  while (NULL != cell)
    {
    for (Nix = 0 ; Nix < (*pnumber_of_selected_cells) ; Nix++)
      if (cell == (*pselected_cells)[Nix])
        break ;
    if (Nix < (*pnumber_of_selected_cells))
      {
      if (cell->x >= topxWorld && cell->x <= botxWorld &&
          cell->y >= topyWorld && cell->y <= botyWorld)
        {
        gqcell_reset_colour ((*pselected_cells)[Nix]) ;
        draw_stdqcell (window, gc, (*pselected_cells)[Nix]) ;
        if (Nix < (*pnumber_of_selected_cells) - 1)
          memmove (&((*pselected_cells)[Nix]), &((*pselected_cells)[Nix + 1]), ((*pnumber_of_selected_cells) - Nix - 1) * sizeof (GQCell *)) ;
        (*pselected_cells) = realloc ((*pselected_cells), --(*pnumber_of_selected_cells) * sizeof (GQCell *)) ;
        }
      }
    cell = cell->next ;
    }
  }

//!Selects all the cells which have centers within the given coordinates.
void select_cells_in_window(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, int top_x, int top_y, int bot_x, int bot_y){
	
	float world_top_x;
	float world_top_y;
	float world_bot_x;
	float world_bot_y;
	float temp;
	int i;
	GQCell *cell;
	
	// -- determine the world coords of the selected window -- //
	world_top_x = real_to_world_x (top_x);
	world_top_y = real_to_world_y (top_y);
	world_bot_x = real_to_world_x (bot_x);
	world_bot_y = real_to_world_y (bot_y);
	
	// make sure the top is at the top //
	if(world_top_x > world_bot_x){
	  temp = world_top_x;
	  world_top_x = world_bot_x;
	  world_bot_x = temp;
	}
	
	if(world_top_y > world_bot_y){
	  temp = world_top_y;
	  world_top_y = world_bot_y;
	  world_bot_y = temp;
	}
	
	// -- free up any previously selected cells -- //
//	free ((*pselected_cells));
//	(*pselected_cells) = NULL;
//	(*pnumber_of_selected_cells) = 0;	
	
	// clear the color of any previously selected_cells //
//	clean_up_colors(first_cell);
	
	// -- if the user simply clicked a point -- //
	if(top_x == bot_x || top_y == bot_y){
		
		// if there is no cell at the point then leave //
		if(select_cell_at_coords(first_cell, world_bot_x, world_bot_y) == NULL) return;
		
		// otherwise allocate the memory //
		(*pselected_cells) = malloc(sizeof(GQCell *));
		
		if((*pselected_cells) == NULL){
			printf("memory allocation error in select_cells_in_window()\n");
			exit(1);
		}
		
		(*pselected_cells)[0] = select_cell_at_coords(first_cell, world_bot_x, world_bot_y);
		(*pnumber_of_selected_cells) = 1;
		gqcell_select ((*pselected_cells)[0]);
		
		return;
	}
	
	// -- else a window -- //
		
	cell = first_cell;
	
	
	while(cell != NULL){
		// -- find all the cells that have centers within the window -- //
		if(world_top_x < cell->x && world_top_y < cell->y){
			if(world_bot_x > cell->x && world_bot_y > cell->y){
				(*pnumber_of_selected_cells)++;
			}
		}
		
		cell = cell->next;
	}
	
	(*pselected_cells) = malloc(sizeof(GQCell *) * (*pnumber_of_selected_cells));
	i = 0;
	
	cell = first_cell;
	while(cell != NULL){
		
		// -- find all the cells that have centers within the window -- //
		if(world_top_x < cell->x && world_top_y < cell->y){
			if(world_bot_x > cell->x && world_bot_y > cell->y){
				(*pselected_cells)[i] = cell;
				i++;
				gqcell_select (cell) ;
			}
		}
		
		cell = cell->next;
	}
	
}// select_cells_in_window

//!Selects cell at the given coords if there is one and is not the one passed in the argument.
GQCell *select_cell_at_coords (GQCell *first_cell, float world_x, float world_y){

  GQCell *cell = first_cell ;

  while (NULL != cell)
    {
    if (gqcell_point_in_cell (cell, world_x, world_y))
      return cell ;
    cell = cell->next ;
    }

  return NULL ;
}// select_cell_at_coords

//-------------------------------------------------------------------//

//!Returns the closest X grid point to the passed coordinate.
inline float grid_world_x(float x){
	// integer casting will do what is required //
	if(x>=0)return (int)( ((float)x+grid_spacing/2) /grid_spacing) * grid_spacing;
	if(x<0)return (int)( ((float)x-grid_spacing/2) /grid_spacing) * grid_spacing;
	return 0;
}//grid_world_x

//-------------------------------------------------------------------//

//!Returns the closest Y grid point to the passed coordinate.
inline float grid_world_y(float y){
	// integer casting will do what is required //
	if(y>=0)return (int)(((float)y+grid_spacing/2)/grid_spacing) * grid_spacing;
	if(y<0)return (int)(((float)y-grid_spacing/2)/grid_spacing) * grid_spacing;	
	return 0;
}//grid_world_y

//-------------------------------------------------------------------//

//!Calculates the X world coordinate from the provided X drawing area coordinate.
inline float calc_world_x(int local_x){
	return (local_x - subs_top_x)/scale;
}//calc_world_y

//-------------------------------------------------------------------//

//!Calculates the X world coordinate from the provided X drawing area coordinate.
inline float calc_world_y(int local_y){
	return (local_y - subs_top_y)/scale;
}//calc_world_y

//-------------------------------------------------------------------//

//!Calculates a world distance from the provided drawing area distance.
inline float calc_world_dist(int dist){
	return dist/scale;
}//calc_world_dist

//-------------------------------------------------------------------//
//!Finds all cells within a specified radius and sets the selected cells array//
int select_cells_in_radius (GQCell *first_cell, GQCell *cell, float world_radius, GQCell ***p_selected_cells){

	GQCell *loop_cell = first_cell;
	int j;
	int number_of_selected_cells = 0 ;
	
	g_assert (cell != NULL);
	
	// free up selected cells //
//	number_of_selected_cells = 0;
//	free (selected_cells);
//	selected_cells = NULL;
	
	while (loop_cell != NULL){
		if (loop_cell != cell){
			if (sqrt((loop_cell->x - cell->x) * (loop_cell->x - cell->x) + (loop_cell->y - cell->y) * (loop_cell->y - cell->y)) <world_radius){
				number_of_selected_cells++;
			}
		}
		
		loop_cell = loop_cell->next;
	}
	
	loop_cell = first_cell;
	
	if (number_of_selected_cells > 0){
		(*p_selected_cells) = malloc (sizeof (GQCell *) * number_of_selected_cells);
		
		// catch any memory allocation errors //
		if ((*p_selected_cells) == NULL){
			printf ("memory allocation error in select_cells_in_radius();\n");
			exit (1);
		}
	
		j = 0;
		
		while (loop_cell != NULL){
			if (loop_cell != cell){
				if (sqrt((loop_cell->x - cell->x) * (loop_cell->x - cell->x) +(loop_cell->y - cell->y) * (loop_cell->y - cell->y)) <world_radius){
					(*p_selected_cells)[j] = loop_cell;
					j++;
				}
			}
			
			loop_cell = loop_cell->next;
		}
	
	}
	
	return number_of_selected_cells;

}//select_cells_in_radius

//-------------------------------------------------------------------//

// -- determine the distance between the centers of two qdots in different cells **** [IN nm]****** -- //
double determine_distance(GQCell * cell1, GQCell * cell2, int dot_cell_1, int dot_cell_2){

  double x, y;

  x = fabs (cell1->cell_dots[dot_cell_1].x - cell2->cell_dots[dot_cell_2].x);
  y = fabs (cell1->cell_dots[dot_cell_1].y - cell2->cell_dots[dot_cell_2].y);

  return sqrt (x * x + y * y);
}//determine_distance

static int world_to_real_x (float x) {return (int)(scale * x + subs_top_x) ;}
static int world_to_real_y (float y) {return (int)(scale * y + subs_top_y) ;}
static float real_to_world_x (int x) {return (float)((x - subs_top_x) / scale) ;}
static float real_to_world_y (int y) {return (float)((y - subs_top_y) / scale) ;}
static int world_to_real_cx (float cx) {return (int)(cx * scale) ;} 
static int world_to_real_cy (float cy) {return (int)(cy * scale) ;} 
static float real_to_world_cx (int cx) {return (int)(cx / scale) ;} 
// static float real_to_world_cy (int cy) {return (int)(cy / scale) ;}
