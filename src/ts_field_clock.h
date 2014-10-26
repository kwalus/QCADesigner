//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Header file for the coherence vector time-dependent  //
// simulation engine.                                   //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _TS_FC_SIMULATION_H_
#define _TS_FC_SIMULATION_H_

#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "vector_table.h"
#include "simulation.h"

// Physical Constants //
#define hbar 1.05457266e-34
#define over_hbar 9.48252e33
#define hbar_sqr 1.11212e-68
#define over_hbar_sqr 8.99183e67
#define kB 1.381e-23
#define over_kB 7.24296e22
#define E 1.602e-19

typedef struct
  {
	double temperature;  
	double time_step;
	double duration;
	double convergence_tolerance;
	double gamma;
	double clock_high;
	double clock_low;
	double clock_shift;
	double Emax;
	double radius_of_effect;
	double epsilonR;
	double layer_separation;
	double cell_elevation;
	double cell_height;
	double counter_ion;
	double dx;
	double dy;
	double dz;
	float lambda_x;
	float lambda_y;
	int algorithm;
	int clocking_scheme;
	gboolean temp_model;
	gboolean randomize_cells;
	gboolean animate_simulation;
  } ts_fc_OP;

void ts_fc_options_dump (ts_fc_OP *ts_fc_options, FILE *pfile) ;
simulation_data *run_ts_fc_simulation(int SIMULATION_TYPE, DESIGN *design, ts_fc_OP *options, VectorTable *pvt);

#endif /* _TS_FC_SIMULATION_H_ */
