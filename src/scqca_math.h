/* scqca_math.h
 * Author: Mike Mazur
 * Date: 2004.01.30
 *
 * This file contains the declarations for the mathematical functions
 * used in the SCQCA simulation
 */

#ifndef _SCQCA_MATH_H_
#define _SCQCA_MATH_H_

#include "scqca_simulation.h"

typedef struct{
	int number_of_neighbours;
	struct GQCell **neighbours;
	double external_delta[SCQCA_NUM_DOTS];
	double internal_delta[SCQCA_NUM_DOTS];
	double current[SCQCA_NUM_DOTS];
}scqca_model;

#define NINTEGRATE_MAX_ITERS 15
/* determins the current */
double scqca_calculate_current(double delta, double voltage, double dot_area, scqca_OP* options);

/* functions will return the change in energy on dot in the driven cell
   either by all external cells (ext_deltas) or the dots within the cell
   (int_deltas) */
void scqca_calculate_ext_deltas(GQCell* target, GQCell* neighbor, scqca_OP*
options);
void scqca_calculate_int_deltas(GQCell* target, int which_dot, scqca_OP* options);

/* function calculates distance between two points given their x and y
   coordinates */
double scqca_dist(double x1, double y1, double x2, double y2);

/* function returns the polarization based on currents */
double scqca_calculate_polarization(GQCell* cell);

/* function will return the overall quality of the cell */
double scqca_calculate_quality(GQCell* cell);

/* sets currents according to the polarity in 'pole' */
void scqca_set_polarization(GQCell* cell, int pole, scqca_OP* options);

#endif /* !_SCQCA_MATH_H_ */
