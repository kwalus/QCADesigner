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

#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "gqcell.h"

struct TRACEDATA {
    
	// array containing the labels for each trace //
	char  *data_labels;
	
	// default color to use with trace //
	int  trace_color;
	
	int drawtrace;
	
	// array containing all plot data //
	double *data;
} ;

typedef struct{

	// total number of simulation samples //
	int number_samples;
	
	// Total number of traces //
	int number_of_traces;
	
	// property of trace //
	struct TRACEDATA *trace;
	
	struct TRACEDATA *clock_data ;
}simulation_data;

simulation_data *run_simulation (int sim_engine, int sim_type, GQCell *first_cell);
void calculate_ground_state (int sim_engine) ;
void tracedata_get_min_max (struct TRACEDATA *trace, int idxStart, int idxEnd, double *pdMin, double *pdMax) ;

#endif /* _SIMULATION_H_ */
