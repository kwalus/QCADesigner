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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// QCADesigner includes //
#include "globals.h"
#include "stdqcell.h"
#include "simulation.h"
#include "cad.h"
#include "callbacks.h"

void fill_background();
void draw_stdqcell(qcell *cell);
void redraw_all_cells();
void draw_subs();
void zoom_previous();
void draw_cell_dots_showing_polarization(qcell *cell);
void draw_temp_stdqcell(double x, double y);

//!previous scale multiplier used when going to previous view.
static double previous_scale = 0;
static double previous_subs_top_x=0; 
static double previous_subs_top_y=0;
static GdkFont *global_font = NULL ;
extern cell_OP cell_options ;

//!Redraws all the design cells to the screen.
void redraw_all_cells(){

	qcell *cell = first_cell;
	
	while(cell != NULL){
		draw_stdqcell(cell);
		cell = cell->next;
	}
	
}//redraw_all_cells

//!Redraws all selected cells
void redraw_selected_cells(){

	int i;
	for(i=0;i<number_of_selected_cells;i++)draw_stdqcell(selected_cells[i]);
	
	
}//redraw_all_cells

//-------------------------------------------------------------------//

//!Fills the drawing area with a white background if PRINT_PREVIEW is set else make it black 
void fill_background(){

	if(!PRINT_PREVIEW){
		gdk_draw_rectangle(main_window.drawing_area->window, main_window.drawing_area->style->black_gc, TRUE, 0, 0, AREA_WIDTH, AREA_HEIGHT);
	}else{
		gdk_draw_rectangle(main_window.drawing_area->window, main_window.drawing_area->style->white_gc, TRUE, 0, 0, AREA_WIDTH, AREA_HEIGHT);
	}

}//fill_background


//-------------------------------------------------------------------//

//!Draws a rectangle representing the substrate onto the drawing area.
void draw_subs(){

	gdk_draw_rectangle(main_window.drawing_area->window, main_window.drawing_area->style->white_gc, FALSE, subs_top_x, subs_top_y, subs_width*scale, subs_height*scale);

}//draw_subs


//-------------------------------------------------------------------//

//!Draws the grid onto the screen.
void draw_grid(){
	
	double i;
	double j;
	int k;
	int number_of_points;
	double width = subs_width * scale;
	double height = subs_height * scale;
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
			
				
	gdk_draw_points(main_window.drawing_area->window, main_window.drawing_area->style->white_gc, grid, k);
	
	//Free the memory from the pixelarray
	free(grid);
	grid = NULL;

}//draw_grid


//-------------------------------------------------------------------//

//!Calls all the redraw functions to regenerate the image.
void redraw_world(){

	fill_background();
	draw_subs();
	if(SHOW_GRID)draw_grid();
	redraw_all_cells();
	
}//redraw_world

//-------------------------------------------------------------------//

//!Redraws all the contents but not the background
//Useful when nothing in the design has changed ex. when a zoom window is being drawn.
void redraw_contents(int tmp_shift_x, int tmp_shift_y){

	subs_top_x += tmp_shift_x ;
	subs_top_y += tmp_shift_y ;
	
	draw_subs();
	if(SHOW_GRID)draw_grid();
	redraw_all_cells();
	
	subs_top_x -= tmp_shift_x ;
	subs_top_y -= tmp_shift_y ;
}//redraw_contents

//-------------------------------------------------------------------//

//!Pans the design window to the left.
void pan_left(int steps){
	subs_top_x += PAN_STEP*steps*scale;
	redraw_world();
}//pan_left

//-------------------------------------------------------------------//

//!Pans the design window to the right.
void pan_right(int steps){
	subs_top_x -= PAN_STEP*steps*scale;
	redraw_world();
}//pan_right

//-------------------------------------------------------------------//

//!Pans the design window up.
void pan_up(int steps){
	subs_top_y += PAN_STEP*steps*scale;
	redraw_world();
}//pan_up

//-------------------------------------------------------------------//

//!Pans the design window down.
void pan_down(int steps){
	subs_top_y -= PAN_STEP*steps*scale;
	redraw_world();
}//pan_down

//-------------------------------------------------------------------//

//!Zooms the design window to the previous zoom scale //
void zoom_previous(){

	double ftemp;
	double itemp;
	
	ftemp = scale;
	scale = previous_scale;
	previous_scale = ftemp;
	
	itemp = subs_top_x;
	subs_top_x = previous_subs_top_x;
	previous_subs_top_x = itemp;
	
	itemp = subs_top_y;
	subs_top_y = previous_subs_top_y;
	previous_subs_top_y = itemp;
	
	redraw_world();

}//zoom_previous

//-------------------------------------------------------------------//

//Calculates bounding box around cells //
void get_extents(double *pmin_x, double *pmin_y, double *pmax_x, double *pmax_y){
	
	qcell *cell = first_cell;
		
	// if there are no cells then extents makes no sense so we zoom to fit the die //	
	if(first_cell == NULL){
	
	*pmin_x = 0 ;
	*pmin_y = 0 ;
	*pmax_x = subs_width ;
	*pmax_y = subs_height ;
	
	} else {
	
	*pmax_x = cell->x;
	*pmax_y = cell->y;
	
	*pmin_x = cell->x;
	*pmin_y = cell->y;
	
	cell = cell->next;
	
	// -- find the true mins and maxes //
	while(cell != NULL){
		if(cell->x <= *pmin_x) *pmin_x = cell->x;
		if(cell->x >= *pmax_x) *pmax_x = cell->x;
		if(cell->y <= *pmin_y) *pmin_y = cell->y;
		if(cell->y >= *pmax_y) *pmax_y = cell->y;
		cell = cell->next;
	}
	
	// -- make some extra room -- //
	
	*pmin_x -= PAN_STEP;
	*pmin_y -= PAN_STEP;
	*pmax_x += PAN_STEP;
	*pmax_y += PAN_STEP;
	
	}
	
//      printf (
//	  "*pmin_x = %d\n"
//	  "*pmin_y = %d\n"
//	  "*pmax_x = %d\n"
//	  "*pmax_y = %d\n", *pmin_x, *pmin_y, *pmax_x, *pmax_y) ;
	
}//get_extents

//-------------------------------------------------------------------//

//!Zooms such that all cells fit on the screen //
void zoom_extents(){
  double min_x, min_y, max_x, max_y ;
  
  if (NULL == first_cell)
    {
    subs_top_x = 0 ;
    subs_top_y = 0 ;
    }

  get_extents (&min_x, &min_y, &max_x, &max_y) ;
  
  //fprintf (stderr, "get_extents returns %d, %d, %d, %d\n",min_x, min_y, max_x, max_y) ;
  
  min_x = (min_x*scale + subs_top_x);
  min_y = (min_y*scale + subs_top_y);
  max_x = (max_x*scale + subs_top_x);
  max_y = (max_y*scale + subs_top_y);
  
  zoom_window(min_x, min_y, max_x, max_y);
	
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
	redraw_world();

}//zoom_die

//-------------------------------------------------------------------//

//!Zooms out a little
void zoom_out(){
	
	zoom_window(-30, -30, AREA_WIDTH + 30, AREA_HEIGHT + 30);
	redraw_world();

}//zoom_out

//-------------------------------------------------------------------//

//!Zooms in a little
void zoom_in(){
	
	zoom_window(30, 30, AREA_WIDTH - 30, AREA_HEIGHT - 30);
		
	redraw_world();

}//zoom_in


//-------------------------------------------------------------------//

//!Zooms to the provide window dimensions.
void zoom_window(int top_x, int top_y, int bot_x, int bot_y){

	int act_top_x;
	int act_top_y;
	double width;
	double height;
	double scale_factor;
	
	if(top_x == bot_x || top_y == bot_y){
		return;
	}
	
	//fprintf (stderr, "Zooming window to top_x = %d, top_y = %d, bot_x = %d, bot_y = %d\n",top_x, top_y, bot_x, bot_y) ;
	
	previous_scale = scale;
	previous_subs_top_x = subs_top_x;
	previous_subs_top_y = subs_top_y;
	
	// -- determine which x value is furthest to the left //
	if(top_x > bot_x){
		act_top_x = bot_x;
	}else{
		act_top_x = top_x;
	}
	
	// -- determine which y value is furthest to the top //
	if(top_y > bot_y){
		act_top_y = bot_y;
	}else{
		act_top_y = top_y;
	}
	
	width = fabs(top_x - bot_x);
	height = fabs(top_y - bot_y);
	
	if(width > height){
		
		scale_factor = (double)AREA_WIDTH/(double)width;
		scale *= scale_factor;
		
		subs_top_x = (subs_top_x - act_top_x)*scale_factor;
		subs_top_y = (subs_top_y - act_top_y)*scale_factor;	
	
	}else{
		
		scale_factor = (double)AREA_HEIGHT/(double)height;
		scale *= scale_factor;
		
		subs_top_x = (subs_top_x - act_top_x)*scale_factor;
		subs_top_y = (subs_top_y - act_top_y)*scale_factor;
		
		
	}
	
	redraw_world();

}//zoom_window

//-------------------------------------------------------------------//

//!Sets the global GTK color object to the desired color.
void set_colors(int color){

	if(global_colormap == NULL){
		global_colormap = gdk_colormap_get_system();
		global_gc = gdk_gc_new(main_window.drawing_area->window);
	}
	
	switch(color){
		
	case WHITE:
		global_color.red = 0xFFFF;
		global_color.green = 0xFFFF;
		global_color.blue = 0xFFFF;
		break;
	
	case BLACK:
		global_color.red = 0x0000;
		global_color.green = 0x0000;
		global_color.blue = 0x0000;
		break;
	
	case GREEN:
		global_color.red = 0;
		global_color.green = 0xFFFF;
		global_color.blue = 0;
		break;
	
	case GREEN1:
		global_color.red = 0xF000;
		global_color.green = 0x0FFF;
		global_color.blue = 0xF000;
		break;
	
	case GREEN2:
		global_color.red = 0x00;
		global_color.green = 0xFFFF;
		global_color.blue = 0xFFFF;
		break;
	
	case GREEN3:
		global_color.red = 0xFFF0;
		global_color.green = 0xFFFF;
		global_color.blue = 0xFFF0;
		break;
	
	case ORANGE:
		global_color.red = 0xFFFF;
		global_color.green = 0x6000;
		global_color.blue = 0x5000;
		break;
			
	case RED:
		global_color.red = 0xFFFF;
		global_color.green = 0;
		global_color.blue = 0;
		break;
		
	case BLUE:
		global_color.red = 0;
		global_color.green = 0;
		global_color.blue = 0xFFFF;
		break;
		
	case YELLOW:
		global_color.red = 0xFFFF;
		global_color.green = 0xFFFF;
		global_color.blue = 0;
		break;
	}
	
	gdk_color_alloc(global_colormap, &global_color);
	gdk_gc_set_foreground(global_gc, &global_color);
	gdk_gc_set_background(global_gc, &global_color);

}//set_colors

//-------------------------------------------------------------------//
//!Draws a ruler to the screen showing the distance between the two coordinates
void draw_ruler(int dist_x0, int dist_y0, int dist_x1, int dist_y1){

	int i, j;
	char distance[20];

	// set up the color //
	global_colormap = gdk_colormap_get_system();
	global_gc = gdk_gc_new(main_window.drawing_area->window);

	gdk_gc_set_function(global_gc, GDK_XOR);

	global_color.red = 0;
	global_color.green = 0xFFFF;
	global_color.blue = 0xFFFF;

	gdk_color_alloc(global_colormap, &global_color);
	gdk_gc_set_foreground(global_gc, &global_color);
	gdk_gc_set_background(global_gc, &global_color);



	// -- Draw a Horizantle Ruler -- //
	if (abs(dist_x0 - dist_x1) >= abs(dist_y0 - dist_y1)) {
	gdk_draw_line(main_window.drawing_area->window, global_gc, dist_x0, dist_y0,
			  dist_x1, dist_y0);

	j = 0;

	global_font =
		gdk_font_load
		("-adobe-courier-medium-r-normal--12-*-*-*-*-*-*");

	g_snprintf(distance, 20, "%f nm",
		calc_world_dist(abs(dist_x0 - dist_x1)));

	gdk_draw_string(main_window.drawing_area->window, global_font, global_gc,
			dist_x1, dist_y0 - 10, distance);

	if (dist_x0 - dist_x1 < 0) {
		for (i = 0; i < abs(dist_x0 - dist_x1);
		 i += grid_spacing) {
		if (j % 2 == 0) {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 + i, dist_y0 - 5,
				  dist_x0 + i, dist_y0 + 5);
		} else {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 + i, dist_y0 - 2,
				  dist_x0 + i, dist_y0 + 2);
		}
		j++;
		}
	} else {
		for (i = 0; i > -abs(dist_x0 - dist_x1);
		 i -= grid_spacing) {
		if (j % 2 == 0) {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 + i, dist_y0 - 5,
				  dist_x0 + i, dist_y0 + 5);
		} else {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 + i, dist_y0 - 2,
				  dist_x0 + i, dist_y0 + 2);
		}
		j++;
		}
	}

	// -- Draw a Vertical Ruler -- //
	} else {
	gdk_draw_line(main_window.drawing_area->window, global_gc, dist_x0, dist_y0,
			  dist_x0, dist_y1);

	j = 0;

	global_font =
		gdk_font_load
		("-adobe-courier-medium-r-normal--12-*-*-*-*-*-*");

	g_snprintf(distance, 20, "%f nm",
		calc_world_dist(abs(dist_y0 - dist_y1)));

	gdk_draw_string(main_window.drawing_area->window, global_font, global_gc,
			dist_x0 + 10, dist_y1, distance);

	if (dist_y0 - dist_y1 < 0) {
		for (i = 0; i < abs(dist_y0 - dist_y1);
		 i += grid_spacing) {
		if (j % 2 == 0) {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 - 5, dist_y0 + i,
				  dist_x0 + 5, dist_y0 + i);
		} else {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 - 2, dist_y0 + i,
				  dist_x0 + 2, dist_y0 + i);
		}
		j++;
		}
	} else {
		for (i = 0; i > -abs(dist_y0 - dist_y1);
		 i -= grid_spacing) {
		if (j % 2 == 0) {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 - 5, dist_y0 + i,
				  dist_x0 + 5, dist_y0 + i);
		} else {
			gdk_draw_line(main_window.drawing_area->window, global_gc,
				  dist_x0 - 2, dist_y0 + i,
				  dist_x0 + 2, dist_y0 + i);
		}
		j++;
		}
	}
	}

}//draw_ruler

//-------------------------------------------------------------------//

//!Draws a temporary array of cells to show the user how many cells will be created in the arrray  //
void draw_temp_array(double x0, double y0, double x1, double y1){

    double x=0;
    double y=0;
	double oldx=0;
	double oldy=0;
	int i;

	// if the user drew a horizontal line //
    if ((y0 - y1) == 0) {

		if(x1>x0)for (x = calc_world_x(x0), i=0; x <= calc_world_x(x1); x += grid_spacing, i++) {

			if (SNAP_TO_GRID) {
				x = grid_world_x(x);
				y = grid_world_y(calc_world_y(y0));
			} else {
				y = calc_world_y(y0);
			}

			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && selected_cell_type == TYPE_1){
				if (x < oldx + cell_options.type_1_cell_width)continue;
			}else if(i != 0 && selected_cell_type == TYPE_2){
				if (x < oldx + cell_options.type_2_cell_width)continue;
			}

			oldx=x;
			oldy=y;

			draw_temp_stdqcell(x,y);
		}

		if(x0>x1)for (x = calc_world_x(x0), i=0; x >= calc_world_x(x1); x -= grid_spacing, i++) {
	
			//printf("x0=%f x1=%f x=%f\n",calc_world_x(x0),calc_world_x(x1),x);
			
			if (SNAP_TO_GRID) {
				x = grid_world_x(x);
				y = grid_world_y(calc_world_y(y0));
			} else {
				y = calc_world_y(y0);
			}
			
			// make sure that the cell does not overlap with previous array cell //
			if(i !=0 && selected_cell_type == TYPE_1){
				if (x > oldx - cell_options.type_1_cell_width)continue;		
			}else if(i != 0 && selected_cell_type == TYPE_2){
				if (x > oldx - cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;

			draw_temp_stdqcell(x,y);
		}

	// if the user drew a vertical line //
    } else if ((x0 - x1) == 0) {
		
		if(y1>y0)for (y = calc_world_y(y0), i=0; y <= calc_world_y(y1); y += grid_spacing, i++) {
			if (SNAP_TO_GRID) {
				x = grid_world_x(calc_world_x(x0));
				y = grid_world_y(y);
			} else {
				x = calc_world_x(x0);
			}
	
			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && selected_cell_type == TYPE_1){
				if (y < oldy + cell_options.type_1_cell_width)continue;
			}else if(i != 0 && selected_cell_type == TYPE_2){
				if (y < oldy + cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;
			
			draw_temp_stdqcell(x,y);
		}
		
		if(y0>y1)for (y = calc_world_y(y0), i=0; y >= calc_world_y(y1); y -= grid_spacing, i++) {
			if (SNAP_TO_GRID) {
				x = grid_world_x(calc_world_x(x0));
				y = grid_world_y(y);
			} else {
				x = calc_world_x(x0);
			}
	
			// make sure that the cell does not overlab with previous array cell //
			if(i !=0 && selected_cell_type == TYPE_1){
				if (y > oldy - cell_options.type_1_cell_width)continue;
			}else if(i != 0 && selected_cell_type == TYPE_2){
				if (y > oldy - cell_options.type_2_cell_width)continue;
			}
			
			oldx=x;
			oldy=y;
			
			draw_temp_stdqcell(x,y);
		}
    }


}//draw_temp_array

//-------------------------------------------------------------------//

//!Draws a temporary QCA cell to the drawing area at the given coordinates.
void draw_temp_stdqcell(double x, double y){

	float top_corner_x = 0;
	float top_corner_y = 0;
	
	if(selected_cell_type == TYPE_1){
		top_corner_x = subs_top_x + (x - cell_options.type_1_cell_width/2)*scale;
		top_corner_y = subs_top_y + (y - cell_options.type_1_cell_height/2)*scale;
			
		// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
		//if(top_corner_x + cell_options.type_1_cell_width * scale < 0 || top_corner_y + cell_options.type_1_cell_height * scale < 0)return;
	
	}
	
	if(selected_cell_type == TYPE_2){
		top_corner_x = subs_top_x + (x - cell_options.type_2_cell_width/2)*scale;
		top_corner_y = subs_top_y + (y - cell_options.type_2_cell_height/2)*scale;
			
		// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
		if(top_corner_x + cell_options.type_2_cell_width * scale < 0 || top_corner_y + cell_options.type_2_cell_height * scale < 0)return;
	}
	
	// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
	//if(top_corner_x > AREA_WIDTH || top_corner_y > AREA_HEIGHT)return;
	
	
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
	set_colors(BLUE);
	gdk_gc_set_function(global_gc, GDK_XOR);
		

		if(selected_cell_type == TYPE_1){
			//gdk_draw_rectangle(main_window.drawing_area->window, global_gc, TRUE, top_corner_x, top_corner_y, cell_options.type_1_cell_width * scale, cell_options.type_1_cell_height * scale);
			gdk_draw_rectangle(main_window.drawing_area->window, global_gc, FALSE, top_corner_x, top_corner_y, cell_options.type_1_cell_width * scale, cell_options.type_1_cell_height * scale);
		}
		
		if(selected_cell_type == TYPE_2){
			//gdk_draw_rectangle(main_window.drawing_area->window, global_gc, TRUE, top_corner_x, top_corner_y, cell_options.type_2_cell_width * scale, cell_options.type_2_cell_height * scale);
			gdk_draw_rectangle(main_window.drawing_area->window, global_gc, FALSE, top_corner_x, top_corner_y, cell_options.type_2_cell_width * scale, cell_options.type_2_cell_height * scale);
		}
	
	
	
	
}//draw_temp_stdqcell

//-------------------------------------------------------------------//

//!Draws a QCA cell to the drawing area at the coords provided by the argument pointer.
void draw_stdqcell(qcell *cell){
      
	float top_corner_x = subs_top_x + (cell->x - cell->cell_width/2)*scale;
	float top_corner_y = subs_top_y + (cell->y - cell->cell_height/2)*scale;
	
	assert(cell != NULL);
	
	// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
	if(top_corner_x > AREA_WIDTH || top_corner_y > AREA_HEIGHT)return;
	if(top_corner_x + cell->cell_width * scale < 0 || top_corner_y + cell->cell_height * scale < 0)return;
	
	
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
	set_colors(cell->color);
		
	// -- draw the rectangle that is the outline of the cell -- //
	if(!PRINT_PREVIEW){
		gdk_draw_rectangle(main_window.drawing_area->window, global_gc, FALSE, top_corner_x, top_corner_y, cell->cell_width * scale, cell->cell_height * scale);
	}else{
		switch(cell->clock){
			
			case 0:
				global_color.red = 0x20FF;
				global_color.green = 0x20FF;
				global_color.blue = 0x20FF;
				break;
			case 1:
				global_color.red = 0x64FF;
				global_color.green = 0x64FF;
				global_color.blue = 0x64FF;
				break;
			case 2:
				global_color.red = 0xC8FF;
				global_color.green = 0xC8FF;
				global_color.blue = 0xC8FF;
				break;
			case 3:
				global_color.red = 0xFFFF;
				global_color.green = 0xFFFF;
				global_color.blue = 0xFFFF;
				break;
			}
			
		gdk_color_alloc(global_colormap, &global_color);
		gdk_gc_set_foreground(global_gc, &global_color);
		gdk_gc_set_background(global_gc, &global_color);
			
		
		gdk_draw_rectangle(main_window.drawing_area->window, global_gc, TRUE, top_corner_x, top_corner_y, cell->cell_width * scale, cell->cell_height * scale);
		
		global_color.red = 0x0000;
		global_color.green = 0x0000;
		global_color.blue = 0x0000;
		
		gdk_color_alloc(global_colormap, &global_color);
		gdk_gc_set_foreground(global_gc, &global_color);
		gdk_gc_set_background(global_gc, &global_color);
		
		
		gdk_draw_rectangle(main_window.drawing_area->window, global_gc, FALSE, top_corner_x, top_corner_y, cell->cell_width * scale, cell->cell_height * scale);
		
		
	}
	
	
	/*for(i = 0; i < cell->number_of_dots; i++){
		// draw every even dot solid //
		if(i % 2 == 0){
			if(!PRINT_PREVIEW){
				gdk_draw_arc(main_window.drawing_area->window, global_gc, FALSE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
			}else{
				global_color.red = 0xFFFF;
				global_color.green = 0xFFFF;
				global_color.blue = 0xFFFF;
		
				gdk_color_alloc(global_colormap, &global_color);
				gdk_gc_set_foreground(global_gc, &global_color);
				gdk_gc_set_background(global_gc, &global_color);
				
				gdk_draw_arc(main_window.drawing_area->window, global_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				
				global_color.red = 0x0000;
				global_color.green = 0x0000;
				global_color.blue = 0x0000;
		
				gdk_color_alloc(global_colormap, &global_color);
				gdk_gc_set_foreground(global_gc, &global_color);
				gdk_gc_set_background(global_gc, &global_color);
				
				gdk_draw_arc(main_window.drawing_area->window, global_gc, FALSE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
			}
		
		}else{
			
			gdk_draw_arc(main_window.drawing_area->window, global_gc, TRUE, subs_top_x + (cell->cell_dots[i].x -
			cell->cell_dots[i].diameter/2)*scale, subs_top_y +(cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				
		}
	
	
	}
	*/
	
	draw_cell_dots_showing_polarization(cell);
	
		
	if(!PRINT_PREVIEW){
	if(cell->is_fixed == TRUE){
		char text[10] = "" ;
		global_font = gdk_font_load("-adobe-courier-medium-r-normal--12-*-*-*-*-*-*");
		g_snprintf(text, 10, "%1.2f", calculate_polarization(cell));
		gdk_draw_string(main_window.drawing_area->window, global_font, global_gc, top_corner_x, top_corner_y - 10, text);
	}
	
	if(cell->is_input == TRUE || cell->is_output == TRUE){
		global_font = gdk_font_load("-adobe-courier-medium-r-normal--12-*-*-*-*-*-*");
		gdk_draw_string(main_window.drawing_area->window, global_font, global_gc, top_corner_x, top_corner_y - 10, cell->label);
		
	}
	
	}

}//draw_stdqcell

//! Function to redraw the cell dots according to the cells polarization//
// used with the real-time animations or when showing the ground state //

void draw_cell_dots_showing_polarization(qcell *cell){
	
	int i;
	float top_corner_x = subs_top_x + (cell->x - cell->cell_width/2)*scale;
	float top_corner_y = subs_top_y + (cell->y - cell->cell_height/2)*scale;
	
	assert(cell != NULL);
	
	// -- Dont need to draw the cell if it does not appear on the screen anyway -- //
	if(top_corner_x > AREA_WIDTH || top_corner_y > AREA_HEIGHT)return;
	if(top_corner_x + cell->cell_width * scale < 0 || top_corner_y + cell->cell_height * scale < 0)return;
	
	
	// -- Set the color object to the cells color -- //
	// -- I use a global color object to speed up drawing, no other benefit. -- //
	set_colors(cell->color);
			
	for(i = 0; i < cell->number_of_dots; i++){
			
			if(!PRINT_PREVIEW){
				gdk_draw_arc(main_window.drawing_area->window, main_window.drawing_area->style->black_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				gdk_draw_arc(main_window.drawing_area->window, global_gc, FALSE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				gdk_draw_arc(main_window.drawing_area->window, global_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE*scale, cell->cell_dots[i].diameter*scale*cell->cell_dots[i].charge/QCHARGE, 0, 360 * 64);
			}else{
				global_color.red = 0xFFFF;
				global_color.green = 0xFFFF;
				global_color.blue = 0xFFFF;
		
				gdk_color_alloc(global_colormap, &global_color);
				gdk_gc_set_foreground(global_gc, &global_color);
				gdk_gc_set_background(global_gc, &global_color);
				
				gdk_draw_arc(main_window.drawing_area->window, main_window.drawing_area->style->white_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				gdk_draw_arc(main_window.drawing_area->window, global_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE*scale, cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE*scale, 0, 360 * 64);
				
				global_color.red = 0x0000;
				global_color.green = 0x0000;
				global_color.blue = 0x0000;
		
				gdk_color_alloc(global_colormap, &global_color);
				gdk_gc_set_foreground(global_gc, &global_color);
				gdk_gc_set_background(global_gc, &global_color);
				
				gdk_draw_arc(main_window.drawing_area->window, main_window.drawing_area->style->white_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2)*scale, cell->cell_dots[i].diameter*scale, cell->cell_dots[i].diameter*scale, 0, 360 * 64);
				gdk_draw_arc(main_window.drawing_area->window, global_gc, TRUE, subs_top_x + (cell->cell_dots[i].x - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, subs_top_y + (cell->cell_dots[i].y - cell->cell_dots[i].diameter/2*cell->cell_dots[i].charge/QCHARGE)*scale, cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE*scale, cell->cell_dots[i].diameter*cell->cell_dots[i].charge/QCHARGE*scale, 0, 360 * 64);
			}

		

				
		}


	
}//draw_cell_dots

//-------------------------------------------------------------------//

//!Resets all cell colors to the natural values determined by the cell type. 
//!Will clear any user changes such as those caused by cell selection.
void clean_up_colors(){

	qcell *cell = first_cell;

	// -- reset the colors of the previously selected cells -- //
	while(cell != NULL){
				
		if(cell->is_input == TRUE){
			cell->color = BLUE;
		}else if(cell->is_output == TRUE){
			cell->color = YELLOW;
		}else if(cell->is_fixed){
			cell->color = ORANGE;
		}else{
			if(cell->clock == 0){cell->color = GREEN;}
			else if(cell->clock == 1){cell->color = GREEN1;}
			else if(cell->clock == 2){cell->color = GREEN2;}
			else if(cell->clock == 3){cell->color = GREEN3;}
		}
		
		cell = cell->next;
		
	}
	
}//clean_up_colors

//-------------------------------------------------------------------//

//!Selects all the cells which have centers within the given coordinates.
void select_cells_in_window(int top_x, int top_y, int bot_x, int bot_y){
	
	float world_top_x;
	float world_top_y;
	float world_bot_x;
	float world_bot_y;
	float temp;
	int i;
	qcell *cell;
	
	// -- determine the world coords of the selected window -- //
	world_top_x = calc_world_x(top_x);
	world_top_y = calc_world_y(top_y);
	world_bot_x = calc_world_x(bot_x);
	world_bot_y = calc_world_y(bot_y);
	
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
	free(selected_cells);
	selected_cells = NULL;
	number_of_selected_cells = 0;	
	
	// clear the color of any previously selected_cells //
	clean_up_colors();
	
	// -- if the user simply clicked a point -- //
	if(top_x == bot_x || top_y == bot_y){
		
		// if there is no cell at the point then leave //
		if(select_cell_at_coords(world_bot_x, world_bot_y) == NULL) return;
		
		// otherwise allocate the memory //
		selected_cells = malloc(sizeof(qcell *));
		
		if(selected_cells == NULL){
			printf("memory allocation error in select_cells_in_window()\n");
			exit(1);
		}
		
		selected_cells[0] = select_cell_at_coords(world_bot_x, world_bot_y);
		number_of_selected_cells = 1;
		selected_cells[0]->color = RED;
		
		return;
	}
	
	// -- else a window -- //
		
	cell = first_cell;
	
	
	while(cell != NULL){
		// -- find all the cells that have centers within the window -- //
		if(world_top_x < cell->x && world_top_y < cell->y){
			if(world_bot_x > cell->x && world_bot_y > cell->y){
				number_of_selected_cells++;
			}
		}
		
		cell = cell->next;
	}
	
	selected_cells = malloc(sizeof(qcell *) * number_of_selected_cells);
	i = 0;
	
	cell = first_cell;
	while(cell != NULL){
		
		// -- find all the cells that have centers within the window -- //
		if(world_top_x < cell->x && world_top_y < cell->y){
			if(world_bot_x > cell->x && world_bot_y > cell->y){
				selected_cells[i] = cell;
				i++;
				cell->color = RED;
			}
		}
		
		cell = cell->next;
	}
	
}// select_cells_in_window

//-------------------------------------------------------------------//

//!Selects cell at the given coords if there is one there otherwise returns NULL.
qcell *select_cell_at_coords(float world_x, float world_y){
	
	qcell *cell = first_cell;
	
	while(cell != NULL){
	
		if(world_x >= cell->top_x && world_x <= cell->bot_x)
			if(world_y >= cell->top_y && world_y <= cell->bot_y){
				
				return cell;
				
			}
		
		cell = cell->next;
	
	}
	
	
	// if nothing was found return null pointer //
	return NULL;

}// select_cell_at_coords 

//-------------------------------------------------------------------//

//!Selects cell at the given coords if there is one and is not the one passed in the argument.
qcell *select_cell_at_coords_but_not_this_one(float world_x, float world_y, qcell *this_cell){
	
	qcell *cell = first_cell;
	
	while(cell != NULL){
		
		if(cell == this_cell){
			cell = cell->next;
			continue;
		}
				
		if(world_x >= cell->top_x && world_x <= cell->bot_x)
			if(world_y >= cell->top_y && world_y <= cell->bot_y){
				
				return cell;
				
			}
		
		cell = cell->next;
	
	}
	
	
	// -- if none are found return null pointer -- //
	return NULL;

}// select_cell_at_coords

//-------------------------------------------------------------------//

//!Returns the closest X grid point to the passed coordinate.
inline int grid_world_x(float x){
	// integer casting will do what is required //
	if(x>=0)return (int)( ((float)x+grid_spacing/2) /grid_spacing) * grid_spacing;
	if(x<0)return (int)( ((float)x-grid_spacing/2) /grid_spacing) * grid_spacing;
	return 0;
}//grid_world_x

//-------------------------------------------------------------------//

//!Returns the closest Y grid point to the passed coordinate.
inline int grid_world_y(float y){
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
int select_cells_in_radius (qcell * cell, float world_radius){

	qcell *loop_cell = first_cell;
	int j;
	
	assert (cell != NULL);
	
	// free up selected cells //
	number_of_selected_cells = 0;
	free (selected_cells);
	selected_cells = NULL;
	
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
		selected_cells = malloc (sizeof (qcell *) * number_of_selected_cells);
		
		// catch any memory allocation errors //
		if (selected_cells == NULL){
			printf ("memory allocation error in select_cells_in_radius();\n");
			exit (1);
		}
	
		j = 0;
		
		while (loop_cell != NULL){
			if (loop_cell != cell){
				if (sqrt((loop_cell->x - cell->x) * (loop_cell->x - cell->x) +(loop_cell->y - cell->y) * (loop_cell->y - cell->y)) <world_radius){
					selected_cells[j] = loop_cell;
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
double determine_distance(qcell * cell1, qcell * cell2, int dot_cell_1, int dot_cell_2){

  double x, y;

  x = fabs (cell1->cell_dots[dot_cell_1].x - cell2->cell_dots[dot_cell_2].x);
  y = fabs (cell1->cell_dots[dot_cell_1].y - cell2->cell_dots[dot_cell_2].y);

  return sqrt (x * x + y * y);
}				//determine_distance
