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

#ifndef _NONLINEAR_APPROX_H_
#define _NONLINEAR_APPROX_H_

#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "qcell.h"
#include "vector_table.h"
#include "simulation.h"

typedef struct{

	int number_of_samples;
	float K ;
	float decay_exponent ;
	gboolean animate_simulation;

}nonlinear_approx_OP;

typedef struct{
	int number_of_neighbours;
	struct qcell **neighbours;
	int is_faulted;
	float response_shift;
	double *Ek;
	
}nonlinear_approx_model;

simulation_data *run_nonlinear_approx(int SIMULATION_TYPE, qcell *first_cell, nonlinear_approx_OP *options, VectorTable *pvt);
int CountActiveInputs (VectorTable *pvt) ;
inline void uglysort(qcell ** sorted_cells, int *number_of_neighbours, int NumberElements);
int compareSortStructs(const void *p1, const void *p2);
double determine_Ek(qcell * cell1, qcell * cell2, nonlinear_approx_OP *options);
void refresh_all_Ek(qcell *cell, nonlinear_approx_OP *options);
//void calculate_ground_state(nonlinear_approx_OP *options);
void animate_test_simulation();

#endif /* NONLINEAR_APPROX_H_ */
