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
// Header file for stuff that wouldn't fit anywhere     //
// else.                                                //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GENERIC_UTILS_H_
#define _GENERIC_UTILS_H_

#include <glib-object.h>
#include <glib.h>
#include "objects/object_helpers.h"

#define MAX_SLOPE_DIFF 0.1

typedef struct _INT_IN_LIST_PARAMS INT_IN_LIST_PARAMS ;

typedef void (*PropertyConnectFunction) (GValue *val_src, GValue *val_dst, gpointer data) ;

// General-purpose function to scale one rectangle until it is inscribed in another rectangle
void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight) ;
char *strdup_convert_to_base (long long value, int base) ;
gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dMaxSlopeDiff) ;
#ifdef GTK_GUI
void RunCmdLineAsync (char *pszCmdLine, char *pszTmpFName) ;
void wait_for_async_cmdlines () ;
#endif /* def GTK_GUI */
char *get_enum_string_from_value (GType enum_type, int value) ;
int get_enum_value_from_string (GType enum_type, char *psz) ;
double spread_seq (int idx) ;

gboolean connect_object_properties (GObject *src, char *pszSrc, GObject *dst, char *pszDst, PropertyConnectFunction fn_forward, gpointer data_forward, GDestroyNotify destroy_forward, PropertyConnectFunction fn_reverse, gpointer data_reverse, GDestroyNotify destroy_reverse) ;
void disconnect_object_properties (GObject *src, char *pszSrc, GObject *dst, char *pszDst, PropertyConnectFunction fn_forward, gpointer data_forward, GDestroyNotify destroy_forward, PropertyConnectFunction fn_reverse, gpointer data_reverse, GDestroyNotify destroy_reverse) ;

// val_dst <- val_src
void CONNECT_OBJECT_PROPERTIES_ASSIGN (GValue *val_src, GValue *val_dst, gpointer data) ;

struct _INT_IN_LIST_PARAMS
  {
  int icInts ;
  int *ints ;
  } ;

// val_dst <- (int_in_list_p (val_src, (INT_IN_LIST_PARAMS)data) ? TRUE : FALSE)
void CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_IN_LIST_P (GValue *val_src, GValue *val_dst, gpointer data) ;
// val_dst <- !val_src
void CONNECT_OBJECT_PROPERTIES_ASSIGN_INVERT_BOOLEAN (GValue *val_src, GValue *val_dst, gpointer data) ;
// val_dst <- (TRUE == val_src) ? ((int *)data)[0] : ((int *)data)[1]
void CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_FROM_BOOLEAN_DATA (GValue *val_src, GValue *val_dst, gpointer data) ;

#endif /* _GENERIC_UTILS_H_ */
