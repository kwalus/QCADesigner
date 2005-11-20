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
// QCADDesignObject: The base class for all the design  //
// objects. This class provides printing, extents cal-  //
// culation, drawing, moving, pretty much everything    //
// for a selectable, printable, design object.          //
//                                                      //
//////////////////////////////////////////////////////////

#include "QCADObject.h"
#include "objects_debug.h"

static void qcad_object_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_object_instance_init (GObject *object, gpointer data) ;
static void qcad_object_instance_finalize (GObject *object) ;

static void copy (QCADObject *objSrc, QCADObject *objDst) ;

GType qcad_object_get_type ()
  {
  static GType qcad_object_type = 0 ;

  if (!qcad_object_type)
    {
    static const GTypeInfo qcad_object_info =
      {
      sizeof (QCADObjectClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_object_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADObject),
      0,
      (GInstanceInitFunc)qcad_object_instance_init
      } ;

    if ((qcad_object_type = g_type_register_static (G_TYPE_OBJECT, QCAD_TYPE_STRING_OBJECT, &qcad_object_info, 0)))
      g_type_class_ref (qcad_object_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADObject as %d\n", (int)qcad_object_type)) ;
    }
  return qcad_object_type ;
  }

static void qcad_object_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADObject::class_init:Entering.\n")) ;

  G_OBJECT_CLASS (klass)->finalize = qcad_object_instance_finalize ;
  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = NULL ;

  DBG_OO (fprintf (stderr, "QCADDesignObject::class_init:Leaving.\n")) ;
  }

static void qcad_object_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADDesignObject::instance_init:Entering.\n")) ;
  DBG_OO (fprintf (stderr, "QCADDesignObject::instance_init:Leaving.\n")) ;
  }

static void qcad_object_instance_finalize (GObject *object)
  {
  void (*parent_finalize) (GObject *obj) = G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_OBJECT)))->finalize ;
  if (NULL != parent_finalize)
    (*parent_finalize) (object) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADObject *qcad_object_new_from_object (QCADObject *src)
  {
  GType type = 0 ;
  QCADObject *dst = NULL ;
  DBG_OO_CP (fprintf (stderr, "Copying the following object:\n")) ;
  DBG_OO_CP (qcad_design_object_serialize (src, stderr)) ;
  DBG_OO_CP (fprintf (stderr, "qcad_object_new_from_object:Found type %s\n",
    NULL == g_type_name (type) ? "NULL" : g_type_name (type))) ;

  if (NULL == src) return NULL ;

  type = G_TYPE_FROM_INSTANCE (src) ;

  if (type)
    {
    dst = g_object_new (type, NULL) ;
    QCAD_OBJECT_GET_CLASS (src)->copy (src, dst) ;
    }

  DBG_OO_CP (qcad_object_serialize (dst, stderr)) ;

  DBG_OO_CP (fprintf (stderr, "qcad_object_new_from_object:Copied object.\n")) ;

  return dst ;
  }

QCADObject *qcad_object_get_default (GType type)
  {
  QCADObject *ret = NULL ;
  gpointer klass = NULL ;

  if (0 != type)
    {
    if (NULL == QCAD_OBJECT_CLASS (klass = g_type_class_peek (type))->default_object)
      {
      if (NULL != QCAD_OBJECT_CLASS (klass)->class_get_default_object)
        ret = (QCAD_OBJECT_CLASS (klass)->default_object =
          (QCAD_OBJECT_CLASS (klass)->class_get_default_object ())) ;
      else
        ret = NULL ;
      }
    else
      ret = QCAD_OBJECT_CLASS (klass)->default_object ;
    }
  if (NULL == ret)
    g_warning ("qcad_object_get_default: Returning NULL for type %s\n", (0 == type ? "0" : g_type_name (type))) ;
  return ret ;
  }

void qcad_object_set_default (GType type, QCADObject *obj)
  {
  QCADObjectClass *klass = NULL ;

  if (0 == type)
    {
    g_warning ("qcad_object_set_default: Cannot set default object for type 0\n") ;
    return ;
    }
  if (NULL == obj)
    {
    g_warning ("qcad_object_set_default: Refusing to set NULL default object for type %s\n", g_type_name (type)) ;
    return ;
    }
  if (G_TYPE_FROM_INSTANCE (obj) != type)
    {
    g_warning ("qcad_object_set_default: Refusing to set default object of type %s for class of type %s\n",
      g_type_name (G_TYPE_FROM_INSTANCE (obj)), g_type_name (type)) ;
    return ;
    }

  klass = QCAD_OBJECT_CLASS (g_type_class_peek (type)) ;
  if (NULL != klass->default_object)
    g_object_unref (G_OBJECT (klass->default_object)) ;
  klass->default_object = obj ;
  }

///////////////////////////////////////////////////////////////////////////////

static void copy (QCADObject *objSrc, QCADObject *objDst)
  {
  DBG_OO_CP (fprintf (stderr, "QCADObject::copy:Entering\n")) ;
  DBG_OO_CP (fprintf (stderr, "QCADObject::copy:Leaving\n")) ;
  }
