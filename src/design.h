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
// Header file for the design. This file defines        //
// functions related to manipulating designs,           //
// selections and, for each design, its corresponding   //
// bus layout.                                          //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _DESIGN_H_
#define _DESIGN_H_

#include <stdio.h>
#include "exp_array.h"
#include "objects/object_helpers.h"
#include "objects/QCADCell.h"
#include "objects/QCADDesignObject.h"
#include "objects/QCADSubstrate.h"
#include "objects/QCADLayer.h"

typedef struct _BUS             BUS ;
typedef struct _BUS_LAYOUT_CELL BUS_LAYOUT_CELL ;
typedef struct _BUS_LAYOUT      BUS_LAYOUT ;
typedef struct _DESIGN          DESIGN ;
typedef struct _BUS_LAYOUT_ITER BUS_LAYOUT_ITER ;

struct _BUS
  {
  char *pszName ;
  QCADCellFunction bus_function ;
  EXP_ARRAY *cell_indices ;
  } ;

struct _BUS_LAYOUT_CELL
  {
  QCADCell *cell ;
  gboolean bIsInBus ;
  } ;

struct _BUS_LAYOUT
  {
  EXP_ARRAY *inputs ;
  EXP_ARRAY *outputs ;
  EXP_ARRAY *buses ;
  } ;

struct _DESIGN
  {
  GList *lstLayers ;
  GList *lstLastLayer ;
  GList *lstCurrentLayer ;
  GList *lstClockingLayer ;
  BUS_LAYOUT *bus_layout ;
  } ;

// This structure helps iterate over the inputs/outputs of a design in a well-defined manner:
// Currently, we want to go through all the buses, followed by all free cells
// NEVER keep these structures around outside iteration loops
struct _BUS_LAYOUT_ITER
  {
  BUS_LAYOUT *bus_layout ;
  QCADCellFunction cell_function ;
  EXP_ARRAY *cell_list ;
  EXP_ARRAY *buses ;
  int idxBus ;
  int idxBusCell ;
  int idxCell ;
  } ;

typedef void (*DesignObjectCallback) (DESIGN *design, QCADDesignObject *obj, gpointer data) ;

enum
  {
  LAYER_MODEL_COLUMN_ICON = 0,
  LAYER_MODEL_COLUMN_NAME,
  LAYER_MODEL_COLUMN_LAYER,
  LAYER_MODEL_COLUMN_LAST
  } ;

enum
  {
  BUS_LAYOUT_MODEL_COLUMN_ICON = 0,
  BUS_LAYOUT_MODEL_COLUMN_NAME,
  BUS_LAYOUT_MODEL_COLUMN_TYPE,
  BUS_LAYOUT_MODEL_COLUMN_INDEX,
  BUS_LAYOUT_MODEL_COLUMN_CELL,
  BUS_LAYOUT_MODEL_COLUMN_LAST
  } ;

#define ROW_TYPE_CELL_INPUT  1 << 0
#define ROW_TYPE_CELL_OUTPUT 1 << 1
#define ROW_TYPE_BUS_INPUT   1 << 2
#define ROW_TYPE_BUS_OUTPUT  1 << 3
#define ROW_TYPE_CLOCK       1 << 4
#define ROW_TYPE_BUS     (ROW_TYPE_BUS_INPUT | ROW_TYPE_BUS_OUTPUT)
#define ROW_TYPE_CELL   (ROW_TYPE_CELL_INPUT | ROW_TYPE_CELL_OUTPUT)
#define ROW_TYPE_INPUT   (ROW_TYPE_BUS_INPUT | ROW_TYPE_CELL_INPUT)
#define ROW_TYPE_OUTPUT (ROW_TYPE_BUS_OUTPUT | ROW_TYPE_CELL_OUTPUT)
#define ROW_TYPE_ANY         (ROW_TYPE_INPUT | ROW_TYPE_OUTPUT | ROW_TYPE_CLOCK)

void              design_serialize (DESIGN *design, FILE *pfile) ;
DESIGN *          design_new (QCADSubstrate **psubs) ;
DESIGN *          design_copy (DESIGN *design) ;
#ifdef GTK_GUI
void              design_draw (DESIGN *design, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip, int flags) ;
GtkTreeStore *    design_bus_layout_tree_store_new (BUS_LAYOUT *bus_layout, int row_types, int icExtraColumns, ...) ;
GtkListStore *    design_layer_list_store_new (DESIGN *design, int icExtraColumns, ...) ;
#endif /* def GTK_GUI */
DESIGN *          design_destroy (DESIGN *design) ;
QCADDesignObject *design_hit_test (DESIGN *design, int x, int y) ;
gboolean          design_get_extents (DESIGN *design, WorldRectangle *extents, gboolean bSelection) ;
gboolean          design_unserialize (DESIGN **pdesign, FILE *pfile) ;
void              design_dump (DESIGN *design, FILE *pfile) ;
void              design_set_layer_order (DESIGN *design, GList *llLayerOrder) ;
void              design_fix_legacy (DESIGN *design) ;
void              design_set_current_layer (DESIGN *design, QCADLayer *layer) ;
gboolean          design_scale_cells (DESIGN *design, double scale) ;
gboolean          design_multiple_visible_layers_p (DESIGN *design) ;

QCADLayer * design_layer_remove (DESIGN *design, QCADLayer *layer) ;
void        design_layer_add (DESIGN *design, QCADLayer *layer) ;
GHashTable *design_layer_object_containment_rules () ;
void        design_layer_dump (QCADLayer *layer, FILE *pfile) ;

#ifdef GTK_GUI
QCADDesignObject *design_selection_create_from_selection (DESIGN *design, GdkWindow *window, GdkFunction rop) ;
EXP_ARRAY *       design_selection_release (DESIGN *design, GdkDrawable *dst, GdkFunction rop) ;
EXP_ARRAY *       design_selection_subtract_window (DESIGN *design, GdkWindow *dst, GdkFunction rop, WorldRectangle *rcWorld) ;
#endif /* def GTK_GUI */
EXP_ARRAY *       design_selection_create_from_window (DESIGN *design, WorldRectangle *rcWorld) ;
EXP_ARRAY *       design_selection_add_window (DESIGN *design, WorldRectangle *rcWorld) ;
EXP_ARRAY *       design_selection_get_object_array (DESIGN *design) ;
void              design_selection_serialize (DESIGN *design, FILE *pfile) ;
void              design_selection_move (DESIGN *design, double dxWorld, double dyWorld) ;
gboolean          design_selection_drop (DESIGN *design) ;
EXP_ARRAY *       design_selection_destroy (DESIGN *design) ;
//void              design_selection_set_cell_host_name (DESIGN *design, char *pszHostName) ;
QCADDesignObject *design_selection_hit_test (DESIGN *design, int x, int y) ;
void              design_selection_objects_foreach (DESIGN *design, DesignObjectCallback cb, gpointer data) ;
GList *           design_selection_get_type_list (DESIGN *design) ;
QCADDesignObject *design_selection_get_anchor (DESIGN *design) ;
QCADDesignObject *design_selection_transform (DESIGN *design, double m11, double m12, double m21, double m22) ;

EXP_ARRAY *design_selection_object_array_add_weak_pointers (EXP_ARRAY *obj_array) ;
void       design_selection_object_array_free (EXP_ARRAY *ar) ;

BUS_LAYOUT *design_bus_layout_new () ;
BUS_LAYOUT *design_bus_layout_unserialize (FILE *pfile) ;
void        design_bus_layout_serialize (BUS_LAYOUT *bus_layout, FILE *pfile) ;
void        design_bus_layout_dump (BUS_LAYOUT *bus_layout, FILE *pfile) ;
void        design_bus_layout_cell_list_dump (EXP_ARRAY *cell_list, char *pszVarName, FILE *pfile) ;
BUS_LAYOUT *design_bus_layout_free (BUS_LAYOUT *bus_layout) ;

QCADCell *design_bus_layout_iter_first (BUS_LAYOUT *bus_layout, BUS_LAYOUT_ITER *bus_layout_iter, QCADCellFunction cell_function, int *pidxMaster) ;
QCADCell *design_bus_layout_iter_next (BUS_LAYOUT_ITER *bus_layout_iter, int *pidxMaster) ;
QCADCell *design_bus_layout_iter_this (BUS_LAYOUT_ITER *bus_layout_iter, int *pidxMaster) ;

#endif /* ndef _DESIGN_H_ */
