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


#ifndef _BISTABLE_SIMULATION_H_
#define _BISTABLE_SIMULATION_H_

#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "vector_table.h"
#include "gqcell.h"
#include "simulation.h"

typedef struct{

	int number_of_samples;
	gboolean animate_simulation;
	double convergence_tolerance;
	double radius_of_effect;
	double epsilonR;
	double clock_high;
	double clock_low;
	int max_iterations_per_sample;

}bistable_OP;

simulation_data *run_bistable_simulation(int SIMULATION_TYPE, GQCell *first_cell, bistable_OP *options, VectorTable *pvt);

#endif /* _BISTABLE_SIMULATION_H_ */
