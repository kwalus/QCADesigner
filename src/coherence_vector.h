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


#ifndef _COHERENCE_SIMULATION_H_
#define _COHERENCE_SIMULATION_H_

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
	double T;
	double relaxation;
	double time_step;
	double duration;
}coherence_OP;

simulation_data *run_coherence_simulation(int SIMULATION_TYPE, GQCell *first_cell, coherence_OP *options, VectorTable *pvt);

#endif /* _COHERENCE_SIMULATION_H_ */
