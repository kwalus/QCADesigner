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
// This file written by Timothy Dysart (tdysart@nd.edu) //
// This is the equivalent of TwoStateCell.cpp/.h in     //
// Aquinas.  For the most part a 1 -> 1 converstion is  //
// how I've implemented it.                             //
// Completion Date: August 1, 2002                      //
//////////////////////////////////////////////////////////

#ifndef _BISTABLE_SIMULATION_H_
#define _BISTABLE_SIMULATION_H_

#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "globals.h"
#include "vector_table.h"
#include "simulation.h"

typedef struct{
	int number_of_neighbours;
	struct qcell **neighbours;
	double *Ek;
}bistable_model;

typedef struct{

	int number_of_samples;
	gboolean animate_simulation;
	double convergence_tolerance;
	double radius_of_effect;
	double K;
	double decay_exponent;
	int max_iterations_per_sample;

}bistable_OP;

simulation_data *run_bistable_simulation(int SIMULATION_TYPE, qcell *first_cell, bistable_OP *options, VectorTable *pvt);
void run_bistable_iteration(int sample_number, int number_of_sorted_cells, bistable_OP *options, simulation_data *sim_data);
int compare_energy_sort_structs (const void *p1, const void *p2);
inline void sort_energies (int *index, float *energy, int NumberElements);
double bistable_determine_Ek(qcell * cell1, qcell * cell2, bistable_OP *options);
void bistable_refresh_all_Ek(qcell *cell, bistable_OP *options);

#endif /* _BISTABLE_SIMULATION_H_ */
