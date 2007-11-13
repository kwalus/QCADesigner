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
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Various constants, including physical constants, and //
// other enumerations.                                  //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GLOBAL_CONSTS_H_
#define _GLOBAL_CONSTS_H_

#ifdef WIN32
  #include <windows.h>
#endif

///////////////////////////////////////////////////////////////////
///////////////////////// DEFINES /////////////////////////////////
///////////////////////////////////////////////////////////////////

//!File OPEN/SAVE actions used by file save dialog to determing whether to open or save
//!the argument file.
#ifdef STDIO_FILEIO
#define FILEOP_NO_ACTION 0
#define FILEOP_OPEN 1
#define FILEOP_SAVE_AS 2
#define FILEOP_EXPORT 3
#define FILEOP_IMPORT 4
#define FILEOP_OPEN_RECENT 5
#define FILEOP_SAVE 7
#define FILEOP_CMDLINE 9
#define FILEOP_AUTOSAVE 10
#define FILEOP_AUTOLOAD 11
#endif /* def STDIO_FILEIO */
#define FILEOP_NEW 6
#define FILEOP_CLOSE 8

#define PRINTED_OBJECTS_DIE     0
#define PRINTED_OBJECTS_CELLS   1
#define PRINTED_OBJECTS_COLOURS 2

//!Color defines
#define GREEN 0
#define GREEN1 5
#define GREEN2 6
#define GREEN3 7
#define RED 1
#define BLUE 2
#define YELLOW 3
#define WHITE 4
#define ORANGE 8
#define BLACK 9
#define HONEYCOMB_BACKGROUND 10

//!Simulation Engines
#define BISTABLE 1
#define DIGITAL_SIM 2
#define SCQCA 3
#define COHERENCE_VECTOR 4
#define TS_COHERENCE_VECTOR 5

//!Simulation Types
#define EXHAUSTIVE_VERIFICATION 0
#define VECTOR_TABLE 1

//!Simulation Algorithms
#define RUNGE_KUTTA 1
#define EULER_METHOD 2

//Added by Faizal for cont. clocking
//!Coherence Clocking Schemes
#define FOUR_PHASE_CLOCKING 0
#define CONT_CLOCKING 1
//End added by Faizal

//!Clocking Schemes
#define ZONE_CLOCKING 1
#define ELECTRODE_CLOCKING 2

// To get high-quality constants out of Mathematica, type these:
//
// << Miscellaneous`PhysicalConstants`
// SetPrecision[VacuumPermittivity,30]
// SetPrecision[SomeOtherConstant,30]
// SetPrecision[(SomeOtherConstant^2)/5,30]
// You can build expressions with these constants, and wrap them up in SetPrecision[<>,30] 
// to get these numbers

//!Some useful physical constants
//#define QCHARGE_SQUAR_OVER_FOUR 6.417423538e-39
#define QCHARGE_SQUAR_OVER_FOUR    ((double)(6.41742353846709430467559076549e-39))
//#define QCHARGE 1.602176462e-19
#define QCHARGE                    ((double)(1.60217646200000007055474082341e-19))
//#define HALF_QCHARGE 0.801088231e-19
#define HALF_QCHARGE               ((double)(8.01088231000000035277370411706e-20))
//#define OVER_QCHARGE 6.241509745e18
#define OVER_QCHARGE               ((double)(6.24150974451152486400000000000e18))
//#define THIRD_QCHARGE 5.340588207e-20
#define THIRD_QCHARGE              ((double)(5.34058820666666650061373099737e-20))
//#define TWO_THIRDS_QCHARGE 1.068117641e-19
#define TWO_THIRDS_QCHARGE         ((double)(1.06811764133333330012274619947e-19))
//#define TWO_FIFTHS_QCHARGE 6.408705848e-20
#define TWO_FIFTHS_QCHARGE         ((double)(6.40870584800000052296020634206e-20))
//#define ONE_FIFTHS_QCHARGE 3.204352924e-20
#define ONE_FIFTHS_QCHARGE         ((double)(3.20435292400000026148010317103e-20))
//#define THREE_FIFTHS_QCHARGE 9.613058772e-20
#define THREE_FIFTHS_QCHARGE       ((double)(9.61305877200000138629341713410e-20))
//#define FOUR_FIFTHS_QCHARGE 1.28174117e-19
#define FOUR_FIFTHS_QCHARGE        ((double)(1.28174116960000010459204126841e-19))
//#define ONE_OVER_FOUR_HALF_QCHARGE 3.12109e18
#define ONE_OVER_FOUR_HALF_QCHARGE ((double)(3.12075487225576243200000000000e18))
//#define EPSILON 8.854187817e-12
#define EPSILON                    ((double)(8.85418781700000047157054854521e-12))
#define PI                          ((double)(3.141592653589793115997963468544))
#define TWO_PI                      ((double)(6.283185307179586231995926937088))
#define FOUR_PI                    ((double)(12.56637061435917246399185387418))
//#define FOUR_PI_EPSILON 1.112650056e-10
#define FOUR_PI_EPSILON            ((double)(1.11265005597565794635320037482e-10))
//#define HBAR 1.0545887e-34
#define HBAR                       ((double)(1.05457159642078551348871563291e-34))
//#define OVER_HBAR 9.482523555e33
#define OVER_HBAR                  ((double)(9.48252355168675613949607578855e33))
#define PRECISION 1e-5

// Normalized random number
#define RAND_UNIFORME (double)rand()/(double)RAND_MAX

// Maximum length of a file system path
#ifdef MAX_PATH
  // The Win32 headers have a similar variable already defined, so let's use it
  #define PATH_LENGTH MAX_PATH
#else
  #define PATH_LENGTH 1024
#endif

// Font to use
#ifndef WIN32
  #define QCAD_GDKFONT "-adobe-courier-medium-r-normal--12-*-*-*-*-*-*"
#else
  #define QCAD_GDKFONT "Courier"
#endif
#define PS_FONT "FuturaBT-Medium"

#define PAN_STEP 10

#endif /* _GLOBAL_CONSTS_H_ */
