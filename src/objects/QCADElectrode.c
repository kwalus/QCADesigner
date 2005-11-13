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
#include "QCADElectrode.h"
#include "mouse_handlers.h"

static void qcad_electrode_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_electrode_instance_init (GObject *object, gpointer data) ;
static void qcad_electrode_instance_finalize (GObject *object) ;

static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t) ;
static double get_voltage (QCADElectrode *electrode, double t) ;
static double get_area (QCADElectrode *electrode) ;

GType qcad_electrode_get_type ()
  {
  static GType qcad_electrode_type = 0 ;

  if (!qcad_electrode_type)
    {
    static const GTypeInfo qcad_electrode_info =
      {
      sizeof (QCADElectrodeClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_electrode_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADElectrode),
      0,
      (GInstanceInitFunc)qcad_electrode_instance_init
      } ;

    if ((qcad_electrode_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_ELECTRODE, &qcad_electrode_info, 0)))
      g_type_class_ref (qcad_electrode_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADElectrode as %d\n", qcad_ellectron_type)) ;
    }
  return qcad_electrode_type ;
  }

static void qcad_electrode_class_init (GObjectClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->finalize = qcad_electrode_instance_finalize ;

  QCAD_ELECTRODE_CLASS (klass)->get_voltage   = get_voltage ;
  QCAD_ELECTRODE_CLASS (klass)->get_potential = get_potential ;
  QCAD_ELECTRODE_CLASS (klass)->get_area      = get_area ;

  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.clock_function = sin ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.amplitude      =  1 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.frequency      =  1e6 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.phase          =  0 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.dc_offset      =  0 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.min_clock      = -1 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.max_clock      =  1 ;
  DBG_OO (fprintf (stderr, "QCADElectrode::class_init:Leaving\n")) ;
  }

static void qcad_electrode_instance_init (GObject *object, gpointer data)
  {
  QCADElectrodeClass *klass = QCAD_ELECTRODE_GET_CLASS (object) ;
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;
  QCADDesignObject *qcad_obj = QCAD_DESIGN_OBJECT (object) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_init:Entering\n")) ;

  memcpy (&(electrode->electrode_options), &(klass->default_electrode_options), sizeof (QCADElectrodeOptions)) ;

  electrode->permittivity =
  electrode->capacitance  = 0.0 ;

  qcad_obj->clr.red = (int)((electrode->electrode_options.phase * 65535) / TWO_PI) ;
  qcad_obj->clr.green = 0xFFFF ;
  qcad_obj->clr.blue = 0xC0C0 ;
  HSLToRGB (&(qcad_obj->clr)) ;
#ifdef GTK_GUI
  gdk_colormap_alloc_color (gdk_colormap_get_system (), &(qcad_obj->clr), FALSE, FALSE) ;
#endif /* def GTK_GUI */

  DBG_OO (fprintf (stderr, "QCADElectrode::instance_init:Leaving\n")) ;
  }

static void qcad_electrode_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_finalize:Entering\n")) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_ELECTRODE)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

double qcad_electrode_get_potential (QCADElectrode *electrode, double x, double y, double z, double t)
  {return QCAD_ELECTRODE_GET_CLASS (electrode)->get_potential (electrode, x, y, z, t) ;}

double qcad_electrode_get_voltage (QCADElectrode *electrode, double t)
  {return QCAD_ELECTRODE_GET_CLASS (electrode)->get_voltage (electrode, t) ;}

double qcad_electrode_get_area (QCADElectrode *electrode)
  {return QCAD_ELECTRODE_GET_CLASS (electrode)->get_area (electrode) ;}

void qcad_electrode_set_capacitance (QCADElectrode *electrode, double relative_permittivity, double z_to_ground)
  {
  if (!QCAD_IS_ELECTRODE (electrode)) return ;
  if (0 == z_to_ground) return ;

  electrode->permittivity = relative_permittivity * EPSILON ;
  electrode->capacitance = (qcad_electrode_get_area (electrode) * electrode->permittivity) / (z_to_ground * 1e-9) ;
  electrode->two_z_to_ground = 2.0 * z_to_ground ;
  }

///////////////////////////////////////////////////////////////////////////////

static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t)
  {return 0 ;}

static double get_voltage (QCADElectrode *electrode, double t)
  {
  double voltage =
    electrode->electrode_options.amplitude * 
    (*(electrode->electrode_options.clock_function)) 
      (electrode->electrode_options.frequency * TWO_PI * t - electrode->electrode_options.phase) +
    electrode->electrode_options.dc_offset ;

  return CLAMP (voltage, electrode->electrode_options.min_clock, electrode->electrode_options.max_clock) ;
  }

static double get_area (QCADElectrode *electrode)
  {return 0 ;}
