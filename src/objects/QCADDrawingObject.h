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
// Header for QCADDrawingObject: Common subclass for    //
// allstretchable drawing objects. So far, this class   //
// looks useless. I might remove it.                    //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADDrawingObject_H_
#define _OBJECTS_QCADDrawingObject_H_

#include <glib-object.h>
#include <gdk/gdk.h>
#include "QCADStretchyObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  QCADStretchyObject parent_instance ;
  } QCADDrawingObject ;

typedef struct
  {
  /* public */
  QCADStretchyObjectClass parent_class ;
  } QCADDrawingObjectClass ;

GType qcad_drawing_object_get_type () ;

#define QCAD_TYPE_STRING_DRAWING_OBJECT "QCADDrawingObject"
#define QCAD_TYPE_DRAWING_OBJECT (qcad_drawing_object_get_type ())
#define QCAD_DRAWING_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_DRAWING_OBJECT, QCADDrawingObject))
#define QCAD_DRAWING_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_DRAWING_OBJECT, QCADDrawingObjectClass))
#define IS_QCAD_DRAWING_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_DRAWING_OBJECT))
#define IS_QCAD_DRAWING_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_DRAWING_OBJECT))
#define QCAD_DRAWING_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_DRAWING_OBJECT, QCADDrawingObjectClass))

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADDrawingObject_H_ */
