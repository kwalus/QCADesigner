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

#ifndef _NONLINEAR_APPROX_H_
#define _NONLINEAR_APPROX_H_

#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "gqcell.h"
#include "vector_table.h"
#include "simulation.h"

typedef struct{

	int number_of_samples;
	double epsilonR ;
	double clock_high ;
	double clock_low ;
	gboolean animate_simulation;

}nonlinear_approx_OP;

simulation_data *run_nonlinear_approx(int SIMULATION_TYPE, GQCell *first_cell, nonlinear_approx_OP *options, VectorTable *pvt);

#endif /* NONLINEAR_APPROX_H_ */
