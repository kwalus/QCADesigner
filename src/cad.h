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

#define PAN_STEP 10

void draw_grid();
void redraw_selected_cells();
void zoom_window(int top_x, int top_y, int bot_x, int bot_y);
void get_extents(double *pmin_x, double *pmin_y, double *pmax_x, double *pmax_y) ;
void zoom_extents();
void zoom_in();
void zoom_out();
void redraw_world();
void redraw_contents(int tmp_shift_x, int tmp_shift_y);
void zoom_die();
void pan_left(int steps);
void pan_right(int steps);
void pan_up(int steps);
void pan_down(int steps);
void set_colors(int color);
void clean_up_colors();
inline int grid_world_x(float x);
inline int grid_world_y(float y);
qcell *select_cell_at_coords(float world_x, float world_y);
qcell *select_cell_at_coords_but_not_this_one(float world_x, float world_y, qcell *cell);
void select_cells_in_window(int top_x, int top_y, int bot_x, int bot_y);
inline float calc_world_x(int local_x);
inline float calc_world_y(int local_y);
inline float calc_world_dist(int dist);
void draw_temp_array(double x0, double y0, double x1, double y1);
void draw_ruler(int dist_x0, int dist_y0, int dist_x1, int dist_y1);
int select_cells_in_radius (qcell * cell, float world_radius);
double determine_distance (qcell * cell1, qcell * cell2, int dot_cell_1, int dot_cell_2);
