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


#ifndef _SCQCA_SIMULATION_H_
#define _SCQCA_SIMULATION_H_

#include "vector_table.h"
#include "gqcell.h"
#include "simulation.h"

/* number of dots in a cell; this should never be changed */
#define SCQCA_NUM_DOTS 4

typedef struct{

	int number_of_samples;
	gboolean animate_simulation;
	double cell_convergence_tolerance;
	double dot_convergence_tolerance;
	double circuit_convergence_tolerance ;
	int max_cell_loops;
	int max_dot_loops;
	int max_iterations_per_sample;
	double radius_of_effect;
	double input_cell_current;
	double temperature;
	double device_voltage;
	double gamma;
	double clock_high;
	double clock_low;
	
        /* These members are read-only to the user and are
           calculated from the other members */
	double Epk1;
	double Epk2;
	double U;
	double electron_lifetime;

}scqca_OP;

simulation_data *run_scqca_simulation(int SIMULATION_TYPE, GQCell *first_cell, scqca_OP *options, VectorTable *pvt);
#endif /* _SCQCA_SIMULATION_H_ */
