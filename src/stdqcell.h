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

#include "gqcell.h"
#include "vector_table.h"

//---[ PROTOS ]------------------------------------------------------//

GQCell *add_stdqcell(GQCell **pfirst_cell, GQCell **plast_cell, GQCell *template, gboolean check_overlap, double x, double y, cell_OP *pco, int type) ;
void clear_all_cells(GQCell **pfirst_cell, GQCell **plast_cell) ;
void move_cell_to_location(GQCell * cell, float x, float y);
int create_array_of_cells(GQCell **pfirst_cell, GQCell **plast_cell, double x0, double y0, double x1, double y1, cell_OP *pco, int type, GQCell ***pppqcNewCells, gboolean bSnapToGrid) ;
int mirror_cells_about_line(GQCell **pfirst_cell, GQCell **plast_cell, int mirror_x0, int mirror_y0, int mirror_x1, int mirror_y1, GQCell **selected_cells, int number_of_selected_cells, cell_OP *pco, int type, GQCell ***pppqcNewCells) ;
int get_clock_from_selected_cells (GQCell **selected_cells, int number_of_selected_cells) ;
GQCell *find_overlapping (GQCell *first_cell, GQCell *gqc) ;
void delete_stdqcell (GQCell *gqc, VectorTable *pvt, GQCell **p_first_cell, GQCell **p_last_cell) ;

#endif /* _STDQCELL_H_ */
