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
// The QCA cell.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif

#include "objects_debug.h"
#include "../generic_utils.h"
#include "../support.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "QCADClockingZone.h"
#include "mouse_handlers.h"

static void qcad_clocking_zone_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_clocking_zone_instance_init (GObject *object, gpointer data) ;
static void qcad_clocking_zone_instance_finalize (GObject *object) ;

GType qcad_clocking_zone_get_type ()
  {
  static GType qcad_clocking_zone_type = 0 ;

  if (!qcad_clocking_zone_type)
    {
    static const GTypeInfo qcad_clocking_zone_info =
      {
      sizeof (QCADClockingZoneClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_clocking_zone_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADClockingZone),
      0,
      (GInstanceInitFunc)qcad_clocking_zone_instance_init
      } ;

    if ((qcad_clocking_zone_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_CLOCKING_ZONE, &qcad_clocking_zone_info, 0)))
      g_type_class_ref (qcad_clocking_zone_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADClockingZone as %d\n", qcad_cell_type)) ;
    }
  return qcad_clocking_zone_type ;
  }

static void qcad_clocking_zone_class_init (GObjectClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->finalize = qcad_clocking_zone_instance_finalize ;

  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.clock_function = sin ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.amplitude      =  1 ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.frequency      =  1e6 ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.phase          =  0 ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.dc_offset      =  0 ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.min_clock      = -1 ;
  QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options.max_clock      =  1 ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::class_init:Leaving\n")) ;
  }

static void qcad_clocking_zone_instance_init (GObject *object, gpointer data)
  {
  QCADClockingZoneClass *klass = QCAD_CLOCKING_ZONE_GET_CLASS (object) ;
  QCADClockingZone *clocking_zone = QCAD_CLOCKING_ZONE (object) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Entering\n")) ;

  memcpy (&(clocking_zone->clocking_zone_options), &(klass->default_clocking_zone_options), sizeof (QCADClockingZoneOptions)) ;

  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Leaving\n")) ;
  }

static void qcad_clocking_zone_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Entering\n")) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_ZONE)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
