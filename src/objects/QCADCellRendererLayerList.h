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
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADCellRendererLayerList_H_
#define _OBJECTS_QCADCellRendererLayerList_H_

G_BEGIN_DECLS

#include "QCADCellRendererText.h"
#include "../design.h"

typedef struct _QCADCellRendererLayerList      QCADCellRendererLayerList ;
typedef struct _QCADCellRendererLayerListClass QCADCellRendererLayerListClass ;

struct _QCADCellRendererLayerList
  {
  QCADCellRendererText parent ;

  DESIGN *design ;
  QCADLayer *layer ;
  QCADLayer *template ;
  } ;

struct _QCADCellRendererLayerListClass
  {
  QCADCellRendererTextClass parent_class ;
  void (*layer_changed) (GtkCellRenderer *cell, char *pszPath, QCADLayer *layer) ;
  } ;

GType qcad_cell_renderer_layer_list_get_type () ;
GtkCellRenderer *qcad_cell_renderer_layer_list_new () ;

#define QCAD_TYPE_STRING_CELL_RENDERER_LAYER_LIST "QCADCellRendererLayerList"
#define QCAD_TYPE_CELL_RENDERER_LAYER_LIST (qcad_cell_renderer_layer_list_get_type ())
#define QCAD_CELL_RENDERER_LAYER_LIST(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerList))
#define QCAD_IS_CELL_RENDERER_LAYER_LIST(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST))
#define QCAD_CELL_RENDERER_LAYER_LIST_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerListClass))
#define QCAD_CELL_RENDERER_LAYER_LIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_CELL_RENDERER_LAYER_LIST, QCADCellRendererLayerListClass))
#define QCAD_IS_CELL_RENDERER_LAYER_LIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_CELL_RENDERER_LAYER_LIST))

G_END_DECLS

#endif /* def _OBJECTS_QCADCellRendererLayerList_H_ */
