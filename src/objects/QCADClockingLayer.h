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
// Header for the layer. This is a structure containing //
// design objects. The kinds of objects a layer may     //
// contain depend on the kind of layer it is.           //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADClockingLayer_H_
#define _OBJECTS_QCADClockingLayer_H_

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "QCADLayer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADClockingLayer      QCADClockingLayer ;
typedef struct _QCADClockingLayerClass QCADClockingLayerClass ;

struct _QCADClockingLayer
  {
  QCADLayer parent_instance ;
  gboolean bDrawPotential ;
  double z_to_draw ; // nm
  int tile_size ; // pixels
  double time_coord ; // s

  double dExtremePotential ;
  } ;

struct _QCADClockingLayerClass
  {
  QCADLayerClass parent_class ;
  } ;

GType qcad_clocking_layer_get_type () ;

#define QCAD_TYPE_STRING_CLOCKING_LAYER "QCADClockingLayer"
#define QCAD_TYPE_CLOCKING_LAYER (qcad_clocking_layer_get_type ())
#define QCAD_CLOCKING_LAYER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CLOCKING_LAYER, QCADClockingLayer))
#define QCAD_IS_CLOCKING_LAYER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CLOCKING_LAYER))
#define QCAD_CLOCKING_LAYER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_CLOCKING_LAYER, QCADClockingLayerClass))
#define QCAD_CLOCKING_LAYER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_CLOCKING_LAYER, QCADClockingLayerClass))
#define QCAD_IS_CLOCKING_LAYER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_CLOCKING_LAYER))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _QCADClockingLayer_H_ */
