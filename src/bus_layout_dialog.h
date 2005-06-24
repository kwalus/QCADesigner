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
// Header file for the bus layout dialog. This allows   //
// the user to group individual inputs into input       //
// buses, and individual outputs into output buses,     //
// respectively. The user's choices are encoded in a    //
// BUS_LAYOUT structure, which is part of the DESIGN    //
// structure.                                           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _BUS_LAYOUT_DIALOG_H_
#define _BUS_LAYOUT_DIALOG_H_

#include <gtk/gtk.h>
#include "design.h"

enum
  {
  BUS_LAYOUT_MODEL_COLUMN_ICON = 0,
  BUS_LAYOUT_MODEL_COLUMN_NAME,
  BUS_LAYOUT_MODEL_COLUMN_TYPE,
  BUS_LAYOUT_MODEL_COLUMN_INDEX,
  BUS_LAYOUT_MODEL_COLUMN_LAST
  } ;

#define ROW_TYPE_CELL_INPUT  1 << 0
#define ROW_TYPE_CELL_OUTPUT 1 << 1
#define ROW_TYPE_BUS_INPUT   1 << 2
#define ROW_TYPE_BUS_OUTPUT  1 << 3
#define ROW_TYPE_CLOCK       1 << 4
#define ROW_TYPE_BUS     (ROW_TYPE_BUS_INPUT | ROW_TYPE_BUS_OUTPUT)
#define ROW_TYPE_CELL   (ROW_TYPE_CELL_INPUT | ROW_TYPE_CELL_OUTPUT)
#define ROW_TYPE_INPUT  (ROW_TYPE_BUS_INPUT  | ROW_TYPE_CELL_INPUT)
#define ROW_TYPE_OUTPUT (ROW_TYPE_BUS_OUTPUT | ROW_TYPE_CELL_OUTPUT)

void get_bus_layout_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout) ;
GtkWidget *create_bus_layout_tree_view (gboolean bColsVisible, char *pszColumnTitle, GtkSelectionMode sel_mode) ;
GtkTreeStore *create_bus_layout_tree_store (BUS_LAYOUT *bus_layout, int icExtraColumns, ...) ;
gboolean gtk_tree_model_iter_next_dfs (GtkTreeModel *model, GtkTreeIter *itr) ;
void bus_layout_tree_model_dump (GtkTreeModel *model, FILE *pfile) ;

#endif /* _BUS_LAYOUT_DIALOG_H_ */
