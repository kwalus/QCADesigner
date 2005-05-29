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
// QCADDrawingObject: Common subclass for all           //
// stretchable drawing objects. So far, this class      //
// looks useless. I might remove it.                    //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include "QCADDrawingObject.h"
#include "object_helpers.h"
#include "../fileio_helpers.h"
#include "objects_debug.h"

static void qcad_drawing_object_class_init (GObjectClass *klass, gpointer data) ;

static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;

GType qcad_drawing_object_get_type ()
  {
  static GType qcad_drawing_object_type = 0 ;

  if (!qcad_drawing_object_type)
    {
    static const GTypeInfo qcad_drawing_object_info =
      {
      sizeof (QCADDrawingObjectClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_drawing_object_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADDrawingObject),
      0,
      NULL
      } ;

    if ((qcad_drawing_object_type = g_type_register_static (QCAD_TYPE_STRETCHY_OBJECT, QCAD_TYPE_STRING_DRAWING_OBJECT, &qcad_drawing_object_info, 0)))
      g_type_class_ref (qcad_drawing_object_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADDrawingObject as %d\n", qcad_drawing_object_type)) ;
    }
  return qcad_drawing_object_type ;
  }

static void qcad_drawing_object_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADDrawingObject::class_init:Entering\n")) ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;
  DBG_OO (fprintf (stderr, "QCADDrawingObject::class_init:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  /* output object type */
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_DRAWING_OBJECT);

  /* call parent serialize function */
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_DRAWING_OBJECT)))->serialize (obj, fp) ;

  /* output variables */

  /* output end of object */
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_DRAWING_OBJECT);
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_DRAWING_OBJECT "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_DRAWING_OBJECT "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_DRAWING_OBJECT)))->unserialize (obj, fp)))
          bStopReading = TRUE ;
        }
      // No other options to make "else if" statements about
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  return bParentInit ;
  }
