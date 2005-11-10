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
// Header file for the UI allowing the user to specify  //
// parameters for the coherence vector simulation       //
// engine.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _TS_COHERENCE_VECTOR_PROPERTIES_DIALOG_H_
#define _TS_COHERENCE_VECTOR_PROPERTIES_DIALOG_H_

#include <gtk/gtk.h>
#include "three_state_coherence.h"

void get_ts_coherence_properties_from_user (GtkWindow *parent, ts_coherence_OP *pso) ;

#endif /* _TS_COHERENCE_VECTOR_PROPERTIES_DIALOG_H_ */
