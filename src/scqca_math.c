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

/* scqca_math.c
 * Author: Mike Mazur
 * Date: 2004.01.30
 *
 * This file contains the mathematical formulas used in the SCQCA
 * simulation
 */

#include <stdio.h>
#include <math.h>

#include "scqca_simulation.h"
#include "scqca_math.h"
#include "scqca_constants.h"

#define DBG_NI(s) /*s*/

#define CONVERGENCE_EPSILON 1.0e-29

/* function performs the integration by adding rectangles and
   triangles */
static double NIntegrate(double (*f)(double, double, double, scqca_OP *), 
		  double delta,
		  double voltage,
		  double lo, 
		  double hi, 
		  double eps,
		  scqca_OP *options,
		  int *picStepPow);

/* necessary for NIntegrate() to work */
static double DoSingleStep(double (*f)(double, double, double, scqca_OP *), 
		     double delta,
	         double voltage,
		     double lo, 
		     double hi, 
		     int icStepPow, scqca_OP *options);

/* function being integrated inside Current() */
static double Integrand(double enr, double delta, double voltage, scqca_OP* options);

double scqca_calculate_current(double delta, double voltage, double dot_area, scqca_OP* options) {
  /* Equivalent to
   *                   e * A * m * kB * T
   * Current(Delta_) = ------------------ * Integrate(T1(Enr) 
   *                   2 * PI^2 * hbar^3              * T2(Enr, Delta) 
   *                                                  * Log dEnr)
   *
   *                  /  1 + E^[(U - Enr) / (kB * T)]    \
   * where Log = Log |---------------------------------  |
   *                 \ 1 + E^[(U - Enr - eV) / (kB * T)]/
   *
   * and E is euler's number
   */
   
     /* Equivalent to
   *                       Tpk
   *           ---------------------------
   * T1(Enr) =     / Enr + eV/3 - Epk1\ ^2
   *           1 + | ----------------- |
   *               \     1/2 * G      /
   */
   
     /* Equivalent to
   *                                    Tpk
   *                   ---------------------------------------
   * T2(Enr, Delta_) =     / Enr + 2eV/3 - (Epk2 + Delta_)\ ^2
   *                   1 + | ----------------------------- |
   *                       \           1/2 * G            /
   */
   
  
  	static int icStepPow = 1 ;
	DBG_NI (fprintf (stderr, "scqca_calculate_current: gonna use %d integration chunks.\n", 1 << icStepPow)) ;
	
	//*** NOTE For extrememly high temperatures i.e. > 1000K you must change 1.4 * options->U to at least 10.0 * options->U. ***//					 
  	return D_CONST * dot_area * options->temperature * NIntegrate(Integrand, delta, voltage, 0.0, 1.4 * options->U, CONVERGENCE_EPSILON, options, &icStepPow);
}

static double Integrand(double enr, double delta, double voltage, scqca_OP* options) {
  return 	(D_TPK / ((1.0 + pow(2.0 * (enr + (THIRD_E * voltage) - options->Epk1) / options->gamma, 2.0)) * 
  			(1.0 + pow(2.0 * (enr + (TWO_THIRD_E * voltage) - (options->Epk2 + delta)) / options->gamma, 2.0)))) * 
			log((1.0 + exp((options->U - enr) / (P_KB * options->temperature))) / (1.0 + exp((options->U - enr - P_E * voltage) / (P_KB * options->temperature))));
}

/* The following code is an adaptation of the nintegr8 function by
   Gabriel Schulhof */
static double NIntegrate(double (*f)(double, double, double, scqca_OP *), 
                         double delta,
                         double voltage,
                         double lo, 
                         double hi, 
                         double eps, 
                         scqca_OP *options,
                         int *picStepPow){
  double new_val = 0, old_val = 0 ;
  int Nix ;
  
  old_val = DoSingleStep(f, delta, voltage, lo, hi, (*picStepPow)++, options);
  for (Nix = 0; Nix < NINTEGRATE_MAX_ITERS; Nix++) {
    new_val = DoSingleStep(f, delta, voltage, lo, hi, (*picStepPow)++, options);

    if(fabs(new_val - old_val) < eps)
	  {
	  (*picStepPow) -= 2 ;
      return new_val;
	  }
    old_val = new_val;
  }
  (*picStepPow) -= 2 ;
  return new_val ;
}

static double DoSingleStep (double (*f)(double, double, double, scqca_OP *), 
		     double delta,
			 double voltage,
		     double lo, 
		     double hi, 
		     int icStepPow, scqca_OP *options) {
  int icSteps = (1 << icStepPow);
  int Nix;

  double inc = (hi - lo) / (double)(icSteps);
  double x = lo + inc;
  double yLo = (*f)(lo, delta, voltage, options);
  double yHi = (*f)(x, delta, voltage, options);
  double yMin, yMax, dRet = 0;
  double val;
  
  DBG_NI(fprintf(stderr, "Entering DoSingleStep\n"));
  
  for(Nix = 0; Nix < icSteps; Nix++) {
    yMin = MIN(yLo, yHi);
    yMax = MAX(yLo, yHi);

    if(yLo >= 0 && yHi >= 0)
      dRet += (val = yMin * inc + ((yMax - yMin) * inc) / 2);
    else if(yLo <= 0 && yHi <= 0)
      dRet += (val = yMax * inc - ((yMax - yMin) * inc) / 2);
      
    DBG_NI(fprintf(stderr,
		   "Chunk [(%lf,0), (%lf,%lf), (%lf,%lf), (%lf,0)] adds %lf\n",
		   x - inc, x - inc, yLo, x, yHi, x, val));
    
    yLo = yHi;
    yHi = (*f)(x += inc, delta, voltage, options);
  }
  DBG_NI(fprintf(stderr, "Leaving DoSingleStep with %lf\n", dRet)) ;

  return dRet ;
}

void scqca_calculate_ext_deltas(GQCell* target, GQCell* neighbor, scqca_OP* options) {
  int i, j;

  for(i = 0; i < SCQCA_NUM_DOTS; i++) {
  //((scqca_model*)target->cell_model)->external_delta[i] = 0.0;
  
	  for(j = 0; j < SCQCA_NUM_DOTS; j++) {
	    ((scqca_model*)target->cell_model)->external_delta[i] += 
			((scqca_model*)neighbor->cell_model)->current[j] * options->electron_lifetime * P_K  /
			scqca_dist(target->cell_dots[i].x*1e-9, target->cell_dots[i].y*1e-9,
				neighbor->cell_dots[j].x*1e-9, neighbor->cell_dots[j].y*1e-9);
      }
  }
}

void scqca_calculate_int_deltas(GQCell* target, int which_dot, scqca_OP* options) {
	int j;
	
	((scqca_model*)target->cell_model)->internal_delta[which_dot] = 0.0;

	for(j = 0; j < SCQCA_NUM_DOTS; j++) {
		if(j != which_dot) {
		    ((scqca_model*)target->cell_model)->internal_delta[which_dot] += 
				((scqca_model*)target->cell_model)->current[j] * options->electron_lifetime * P_K  /
				scqca_dist(target->cell_dots[which_dot].x*1e-9, target->cell_dots[which_dot].y*1e-9,
					target->cell_dots[j].x*1e-9, target->cell_dots[j].y*1e-9);
		}
	}
	
}

double scqca_dist(double x1, double y1, double x2, double y2) {
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

double scqca_calculate_polarization(GQCell* cell) {
	
	double sum=(((scqca_model *)cell->cell_model)->current[0] + 
		((scqca_model *)cell->cell_model)->current[1] + 
		((scqca_model *)cell->cell_model)->current[2] +
		((scqca_model *)cell->cell_model)->current[3]);
	
	if(sum==0)return 0;
	
	return (((scqca_model *)cell->cell_model)->current[0] + 
  		((scqca_model*)cell->cell_model)->current[2] - 
		((scqca_model *)cell->cell_model)->current[1] - 
		((scqca_model *)cell->cell_model)->current[3]) / sum;
}

double scqca_calculate_quality(GQCell* cell) {
  
  double qp1;
  double qm1;
  double pole;
  double current_sum;
  double quality;
  
  current_sum = ((scqca_model*)cell->cell_model)->current[0] +
  				((scqca_model*)cell->cell_model)->current[1] +
				((scqca_model*)cell->cell_model)->current[2] +
				((scqca_model*)cell->cell_model)->current[3];
				
	if(current_sum==0)return 0;
  
  qp1 = MIN(((scqca_model*)cell->cell_model)->current[0],
  			((scqca_model*)cell->cell_model)->current[2]) / 
		MAX(((scqca_model*)cell->cell_model)->current[0],
			((scqca_model*)cell->cell_model)->current[2]);
  qm1 = MIN(((scqca_model*)cell->cell_model)->current[1],
  			((scqca_model*)cell->cell_model)->current[3]) / 
		MAX(((scqca_model*)cell->cell_model)->current[1],
			((scqca_model*)cell->cell_model)->current[3]);
  pole = scqca_calculate_polarization(cell);
  
	
  quality = ((((scqca_model*)cell->cell_model)->current[0] +
			((scqca_model*)cell->cell_model)->current[2]) / current_sum * qp1 +
		    (((scqca_model*)cell->cell_model)->current[1] +
			((scqca_model*)cell->cell_model)->current[3]) / current_sum * qm1);
  return fabs(pole) * quality;
}

void scqca_set_polarization(GQCell* cell, int pole, scqca_OP* options) {
  ((scqca_model*)cell->cell_model)->current[0] = 
  	options->input_cell_current / 2.0 * (double)pole + options->input_cell_current / 2.0;

  ((scqca_model*)cell->cell_model)->current[1] = 
  	-1.0 * options->input_cell_current / 2.0 * (double)pole + options->input_cell_current / 2.0;

  ((scqca_model*)cell->cell_model)->current[2] =
  	options->input_cell_current / 2.0 * (double)pole + options->input_cell_current / 2.0;

  ((scqca_model*)cell->cell_model)->current[3] =
  	-1.0 * options->input_cell_current / 2.0 * (double)pole + options->input_cell_current / 2.0;
}
