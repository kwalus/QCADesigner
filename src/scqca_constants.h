/* scqca_constants.h
 * Author: Mike Mazur
 * Date: 2004.01.24
 *
 * This file contains global variables which are the physical
 * constants and device parameters for the Split-Curent Quantum Cell
 * Automata (SCQCA) model
 */

#ifndef _SCQCA_CONSTANTS_H_
#define _SCQCA_CONSTANTS_H_

#define E (double)(2.7182818284590452353602874713527)

/* physical constants (P_ prefix) */
/* electrostatic potential constant */
#define P_K (double)(8987551788.0)
#define P_H (double)(6.625e-34)    /* Planck's constant */
/* Planck's constant divided by 2 PI */
#define P_HBAR (double)(1.05457266e-34)
/* effective mass factor of electron in GaAs */
#define P_M0 (double)(0.067)
#define P_ME (double)(9.109e-31)   /* rest mass of electron */
/* effective mas of electron in GaAs */
#define P_M (double)(6.10328586e-32)  
#define P_E (double)(1.602176462e-19)    /* elementary charge */
#define P_KB (double)(1.381e-23)   /* Boltzmann's constant */

/////////// THE FOLLOWING ARE OPTIMIZATION CONSTANTS /////////
//The following is the integratation constant (e*m*kB)/(2*PI^2*hbar^3)
#define D_CONST (double)(5.832334582669863e27)
#define THIRD_E (double)(5.340588207e-20)
#define TWO_THIRD_E (double)(1.068117641e-19)
///////////////////////////////////////////////////////////////

/* device parameters (D_ prefix) */
/* peak transmission through quantum well (almost always 1) */
#define D_TPK (double)(1.0)

#endif /* !_SCQCA_CONSTANTS_H_ */
