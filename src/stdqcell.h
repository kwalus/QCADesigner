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

#ifndef _STDQCELL_H_
#define _STDQCELL_H_
/*
#ifdef SUN_SOLARIS
	#include <sunperf.h>
#else
	#include "f2c.h"
	#include "clapack.h"
	#include "blaswrap.h"
#endif
*/
#include "globals.h"

typedef struct
  {
  int default_clock ;
  double type_1_cell_width ;
  double type_1_cell_height ;
  double type_1_dot_diameter ;
  double type_2_cell_width ;
  double type_2_cell_height ;
  double type_2_dot_diameter ;
  } cell_OP ;

//---[ PROTOS ]------------------------------------------------------//

qcell *add_stdqcell(qcell * cell, int check_overlap, int type);
void clear_all_cells();
void delete_stdqcell(qcell * cell);
void cell_copy(qcell * to_cell, qcell * from_cell);
void copy_dot(qdot * to_dot, qdot * from_dot);
void free_qcell(qcell * cell);
void set_cell_as_output(qcell * cell);
void set_cell_as_input(qcell * cell);
void set_cell_label(qcell * cell, char *label);
void set_cell_as_fixed(qcell * cell);
double calculate_polarization(qcell * cell);
void set_cell_polarization (qcell * cell, double new_polarization);
void move_cell_to_location(qcell * cell, float x, float y);
void move_cell_by_offset(qcell * cell, float x_offset, float y_offset);
void create_array_of_cells(double x0, double y0, double x1, double y1);
void rotate_cell(qcell * cell, float angle);
void mirror_cells_about_line(int mirror_x0, int mirror_y0, int mirror_x1, int mirror_y1);
int get_clock_from_selected_cells () ;
void set_clock_for_selected_cells(int selected_clock);

#endif /* _STDQCELL_H_ */
