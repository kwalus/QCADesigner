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
// Header for QCADDesignObject: The base class for all  //
// the design objects. This class provides printing,    //
// extents calculation, drawing, moving, pretty much    //
// everything for a selectable, printable, design       //
// object.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADObject_H_
#define _OBJECTS_QCADObject_H_

#include <stdio.h>
#include <stdarg.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif
#include <glib-object.h>
#include "../exp_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  GObject parent_instance ;
  } QCADObject ;

typedef struct QCADObjectClass
  {
  GObjectClass parent_class ;
  QCADObject *default_object ;
  EXP_ARRAY *property_uis ;

  void (*copy) (QCADObject *objSrc, QCADObject *objDst) ;
  QCADObject *(*class_get_default_object) () ;
  } QCADObjectClass ;

GType qcad_object_get_type () ;

#define QCAD_TYPE_STRING_OBJECT "QCADObject"
#define QCAD_TYPE_OBJECT (qcad_object_get_type ())
#define QCAD_OBJECT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_OBJECT, QCADObject))
#define QCAD_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_OBJECT, QCADObjectClass))
#define QCAD_IS_OBJECT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_OBJECT))
#define QCAD_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_OBJECT))
#define QCAD_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_OBJECT, QCADObjectClass))

///////////////////////////////////////////////////////////////////////////////

// Returns a copy of src
QCADObject *qcad_object_new_from_object (QCADObject *src) ;
// Returns the default object for the given type
QCADObject *qcad_object_get_default (GType type) ;
// Overwrites the default object for the given type
void qcad_object_set_default (GType type, QCADObject *obj) ;

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADDesignObject_H_ */
