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

#ifndef _CAD_H_
#define _CAD_H_

#include <gdk/gdk.h>
#include "gqcell.h"

typedef enum
  {
  LAYER_TYPE_CELLS,
  LAYER_TYPE_CLOCKING
  } LayerType ;

typedef enum
  {
  LAYER_STATUS_ACTIVE,  /* Editable (=> visible) */
  LAYER_STATUS_VISIBLE, /* Non-editable */
  LAYER_STATUS_HIDDEN   /* not shown */
  } LayerStatus ;

typedef struct LAYER
  {
  LayerType type ;
  LayerStatus status ;
  gchar *pszDescription ;
  struct LAYER *prev ;
  struct LAYER *next ;
  void *first_obj ;
  void *last_obj ;
  } LAYER ;

typedef struct
  {
  LAYER *first_layer ;
  LAYER *last_layer ;
  } DESIGN ;

#define PAN_STEP 10

void draw_grid();
void redraw_selected_cells(GdkDrawable *d, GdkGC *gc, GQCell **selected_cells, int number_of_selected_cells);
void zoom_window(int top_x, int top_y, int bot_x, int bot_y);
void get_extents(GQCell *first_cell, double *pmin_x, double *pmin_y, double *pmax_x, double *pmax_y) ;
void zoom_extents(GQCell *first_cell);
void zoom_in();
void zoom_out();
void redraw_world(GdkDrawable *d, GdkGC *gc, GQCell *first_cell, gboolean bShowGrid);
void zoom_die();
void pan (int cx, int cy) ;
void set_colors(int color);
void clean_up_colors(GQCell *first_cell);
void draw_stdqcell(GdkDrawable *d, GdkGC *gc, GQCell *cell);
inline float grid_world_x(float x);
inline float grid_world_y(float y);
GQCell *select_cell_at_coords(GQCell *first_cell, float world_x, float world_y);
void select_cells_in_window(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, int top_x, int top_y, int bot_x, int bot_y);
void add_cells_to_selection(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, int top_x, int top_y, int bot_x, int bot_y);
void remove_cells_from_selection(GQCell *first_cell, GQCell ***pselected_cells, int *pnumber_of_selected_cells, GdkDrawable *window, GdkGC *gc, int top_x, int top_y, int bot_x, int bot_y);
inline float calc_world_x(int local_x);
inline float calc_world_y(int local_y);
inline float calc_world_dist(int dist);
void draw_temp_array(GdkDrawable *d, GdkGC *gc, int cell_type, double x0, double y0, double x1, double y1);
void draw_ruler(GdkDrawable *d, GdkGC *gc, int dist_x0, int dist_y0, int dist_x1, int dist_y1);
int select_cells_in_radius (GQCell *first_cell, GQCell *cell, float world_radius, GQCell ***p_selected_cells) ;
double determine_distance (GQCell * cell1, GQCell * cell2, int dot_cell_1, int dot_cell_2);
void scale_design(GQCell *first_cell, double scale);

#endif /* _CAD_H_ */
