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
// Header file for the bistable properties dialog,      //
// which allows the user to set the parameters for the  //
// bistable simulation engine.                          //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _SEMI_COHERENT_PROPERTIES_DIALOG_H_
#define _SEMI_COHERENT_PROPERTIES_DIALOG_H_

#ifdef HAVE_FORTRAN

#include <gtk/gtk.h>
#include "semi_coherent.h"

void get_semi_coherent_properties_from_user (GtkWindow *parent, semi_coherent_OP *pbo) ;

#endif /* HAVE_FORTRAN */

#endif /* _SEMI_COHERENT_PROPERTIES_DIALOG_H_ */
