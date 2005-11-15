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
#include "../fileio_helpers.h"
#include "QCADElectrode.h"
#include "mouse_handlers.h"

static void qcad_electrode_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_electrode_instance_init (GObject *object, gpointer data) ;
static void qcad_electrode_instance_finalize (GObject *object) ;

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) ;
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t) ;
static double get_voltage (QCADElectrode *electrode, double t) ;
static double get_area (QCADElectrode *electrode) ;
static void precompute (QCADElectrode *electrode) ;
static EXTREME_POTENTIALS extreme_potential (QCADElectrode *electrode, double z) ;

enum
  {
  QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND = 1,
  QCAD_ELECTRODE_PROPERTY_EPSILON_R,
  QCAD_ELECTRODE_PROPERTY_LAST
  } ;

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
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  QCAD_DESIGN_OBJECT_CLASS(klass)->unserialize = unserialize ;
  QCAD_DESIGN_OBJECT_CLASS(klass)->serialize   = serialize ;

  QCAD_ELECTRODE_CLASS (klass)->get_voltage       = get_voltage ;
  QCAD_ELECTRODE_CLASS (klass)->get_potential     = get_potential ;
  QCAD_ELECTRODE_CLASS (klass)->get_area          = get_area ;
  QCAD_ELECTRODE_CLASS (klass)->precompute        = precompute ;
  QCAD_ELECTRODE_CLASS (klass)->extreme_potential = extreme_potential ;

  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.clock_function = sin ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.amplitude      =  1 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.frequency      =  1e6 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.phase          =  0 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.dc_offset      =  0 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.min_clock      = -1 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.max_clock      =  1 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.relative_permittivity = 12.9 ;
  QCAD_ELECTRODE_CLASS (klass)->default_electrode_options.z_to_ground = 20.0 ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND,
    g_param_spec_double ("z-to-ground", _("Distance to ground"), _("Distance from electrode to ground electrode"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_EPSILON_R,
    g_param_spec_double ("relative-permittivity", _("Relative permittivity"), _("Relative permittivity of the environment between the electrode and the ground"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::class_init:Leaving\n")) ;
  }

static void qcad_electrode_instance_init (GObject *object, gpointer data)
  {
  QCADElectrodeClass *klass = QCAD_ELECTRODE_GET_CLASS (object) ;
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;
  QCADDesignObject *qcad_obj = QCAD_DESIGN_OBJECT (object) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_init:Entering\n")) ;

  memcpy (&(electrode->electrode_options), &(klass->default_electrode_options), sizeof (QCADElectrodeOptions)) ;

  precompute (electrode) ;

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

EXTREME_POTENTIALS qcad_electrode_get_extreme_potential (QCADElectrode *electrode, double z)
  {return QCAD_ELECTRODE_GET_CLASS (electrode)->extreme_potential (electrode, z) ;}

///////////////////////////////////////////////////////////////////////////////

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND:
      electrode->electrode_options.z_to_ground = g_value_get_double (value) ;
      QCAD_ELECTRODE_GET_CLASS (electrode)->precompute (electrode) ;
      g_object_notify (object, "z-to-ground") ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_EPSILON_R:
      electrode->electrode_options.relative_permittivity = g_value_get_double (value) ;
      QCAD_ELECTRODE_GET_CLASS (electrode)->precompute (electrode) ;
      g_object_notify (object, "relative-permittivity") ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND:
      g_value_set_double (value, electrode->electrode_options.z_to_ground) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_EPSILON_R:
      g_value_set_double (value, electrode->electrode_options.relative_permittivity) ;
      break ;
    }
  }

static void precompute (QCADElectrode *electrode)
  {
  electrode->precompute_params.permittivity = electrode->electrode_options.relative_permittivity * EPSILON ;
  electrode->precompute_params.two_z_to_ground = electrode->electrode_options.z_to_ground * 2.0 ;
  electrode->precompute_params.capacitance = QCAD_ELECTRODE_GET_CLASS (electrode)->get_area (electrode) * electrode->precompute_params.permittivity / (electrode->electrode_options.z_to_ground * 1e-9) ;
  }

static EXTREME_POTENTIALS extreme_potential (QCADElectrode *electrode, double z) 
  {
  EXTREME_POTENTIALS xp = {0, 0} ;
  return xp ;
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  char pszDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;
  QCADElectrode *electrode = NULL ;

  if (NULL == obj || NULL == fp) return ;

  electrode = QCAD_ELECTRODE (obj) ;

  fprintf (fp, "[TYPE:" QCAD_TYPE_STRING_ELECTRODE "]\n") ;
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_ELECTRODE)))->serialize (obj, fp) ;
  fprintf (fp, "electrode_options.clock_function=%s\n", sin == electrode->electrode_options.clock_function ? "sin" : "unknown") ;
  fprintf (fp, "electrode_options.amplitude=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.amplitude)) ;
  fprintf (fp, "electrode_options.frequency=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.frequency)) ;
  fprintf (fp, "electrode_options.phase=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.phase)) ;
  fprintf (fp, "electrode_options.dc_offset=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.dc_offset)) ;
  fprintf (fp, "electrode_options.min_clock=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.min_clock)) ;
  fprintf (fp, "electrode_options.max_clock=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.max_clock)) ;
  fprintf (fp, "relative_permittivity=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.relative_permittivity)) ;
  fprintf (fp, "z_to_ground=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.z_to_ground)) ;
  fprintf (fp, "[#TYPE:" QCAD_TYPE_STRING_ELECTRODE "]\n") ;
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_ELECTRODE "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;

    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_ELECTRODE "]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        tokenize_line_type (pszLine, strlen (pszLine), &pszValue, ':') ;

        if (!strcmp (pszValue, QCAD_TYPE_STRING_DESIGN_OBJECT))
          {
          if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_ELECTRODE)))->unserialize (obj, fp)))
            bStopReading = TRUE ;
          }
        }
      else
      if (!strcmp (pszLine, "electrode_options.relative_permittivity"))
        electrode->electrode_options.relative_permittivity = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "electrode_options.z_to_ground"))
        electrode->electrode_options.z_to_ground = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "electrode_options.clock_function"))
        electrode->electrode_options.clock_function = (strcmp (pszValue, "sin") ? NULL : sin) ;
      else
      if (!strcmp (pszLine, "electrode_options.amplitude"))
        electrode->electrode_options.amplitude = strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "electrode_options.frequency"))
        electrode->electrode_options.frequency = strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "electrode_options.phase"))
        electrode->electrode_options.phase = strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "electrode_options.dc_offset"))
        electrode->electrode_options.dc_offset = strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "electrode_options.min_clock"))
        electrode->electrode_options.min_clock = strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "electrode_options.max_clock"))
        electrode->electrode_options.max_clock = strtod (pszValue, NULL) ;
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }


  if (bParentInit && !bStopReading)
    {
    if (QCAD_DESIGN_OBJECT_CLASS (QCAD_ELECTRODE_GET_CLASS (electrode))->unserialize == unserialize)
      QCAD_ELECTRODE_GET_CLASS (electrode)->precompute (electrode) ;
    return TRUE ;
    }
  else
    return FALSE ;
  }

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
