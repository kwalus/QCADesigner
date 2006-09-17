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

#ifndef _OBJECTS_QCADDistributionLayer_
#define _OBJECTS_QCADDistributionLayer_

#ifdef __cplusplus
extern "C" {
#endif

#include "../design.h"
#include "QCADLayer.h"

typedef struct _QCADDistributionLayer      QCADDistributionLayer ;
typedef struct _QCADDistributionLayerClass QCADDistributionLayerClass ;

struct _QCADDistributionLayer
  {
  QCADLayer parent_instance ;
  DESIGN *design ;
  } ;

struct _QCADDistributionLayerClass
  {
  QCADLayerClass parent_class ;
  } ;

GType qcad_distribution_layer_get_type () ;
void qcad_distribution_layer_generate_distribution (QCADDistributionLayer *layer, DESIGN *design) ;

#define QCAD_TYPE_STRING_DISTRIBUTION_LAYER "QCADDistributionLayer"
#define QCAD_TYPE_DISTRIBUTION_LAYER (qcad_distribution_layer_get_type ())
#define QCAD_DISTRIBUTION_LAYER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_DISTRIBUTION_LAYER, QCADDistributionLayer))
#define QCAD_IS_DISTRIBUTION_LAYER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_DISTRIBUTION_LAYER))
#define QCAD_DISTRIBUTION_LAYER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_DISTRIBUTION_LAYER, QCADDistributionLayerClass))
#define QCAD_DISTRIBUTION_LAYER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_DISTRIBUTION_LAYER, QCADDistributionLayerClass))
#define QCAD_IS_DISTRIBUTION_LAYER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_DISTRIBUTION_LAYER))

#ifdef __cplusplus
}
#endif

#endif /* def _OBJECTS_QCADDistributionLayer_ */
