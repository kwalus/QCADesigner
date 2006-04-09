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
// Header file for the QCADesigner design PostScript    //
// printer.                                             //
// Completion Date: June 2003                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_H_
#define _PRINT_H_

#include "simulation.h"
#include "design.h"

#define PRINT_LAYER_KEY "print_layer"

typedef struct _print_OP         print_OP ;
typedef struct _print_design_OP  print_design_OP ;
typedef struct _print_graph_OP   print_graph_OP ;
typedef struct _PRINT_GRAPH_DATA PRINT_GRAPH_DATA ;

struct _print_OP
  {
  double dPaperCX ;
  double dPaperCY ;
  double dLMargin ;
  double dTMargin ;
  double dRMargin ;
  double dBMargin ;
  gboolean bPrintFile ;
  gboolean bPortrait ;
  gboolean bPrintColours ;
  char *pszPrintString ;
  } ;

struct _print_design_OP
  {
  print_OP po ;
  // Units are in points == 1/72 inches
  double dPointsPerNano ;
  gboolean bPrintOrderOver ;
  gboolean bCenter ;
  gboolean bFit ;
//  gboolean *pbPrintedObjs ;
//  int icPrintedObjs ;
  int iCXPages ;
  int iCYPages ;
  } ;

struct _print_graph_OP
  {
  print_OP po ;
  // Units are in points == 1/72 inches
  gboolean bPrintOrderOver ;
  int iCXPages ;
  int iCYPages ;
  } ;

struct _PRINT_GRAPH_DATA
  {
  simulation_data *sim_data ;
  BUS_LAYOUT *bus_layout ;
  EXP_ARRAY *bus_traces ; // HONEYCOMB_DATA *
  int honeycomb_base ;
  } ;

typedef void (*PrintFunction) (print_OP *pPO, void *data) ;

void print_world (print_design_OP *pPrintOpts, DESIGN *design) ;
void print_graphs (print_graph_OP *pPrintOpts, PRINT_GRAPH_DATA *print_graph_data) ;

#endif /*_PRINT_H_*/
