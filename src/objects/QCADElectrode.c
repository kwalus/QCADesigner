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

#include "../generic_utils.h"
#include "../intl.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "../fileio_helpers.h"
#include "QCADElectrode.h"
#include "QCADPropertyUI.h"
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
static double get_long_side (QCADElectrode *electrode) ;
static double get_short_side (QCADElectrode *electrode) ;
static void precompute (QCADElectrode *electrode) ;
static EXTREME_POTENTIALS extreme_potential (QCADElectrode *electrode, double z) ;
static void copy (QCADObject *src, QCADObject *dst) ;

enum
  {
  QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND = 1,
	QCAD_ELECTRODE_PROPERTY_THICKNESS,
  QCAD_ELECTRODE_PROPERTY_EPSILON_R,
  QCAD_ELECTRODE_PROPERTY_AMPLITUDE,
  QCAD_ELECTRODE_PROPERTY_FREQUENCY,
  QCAD_ELECTRODE_PROPERTY_PHASE,
  QCAD_ELECTRODE_PROPERTY_DC_OFFSET,
  QCAD_ELECTRODE_PROPERTY_MIN_CLOCK,
  QCAD_ELECTRODE_PROPERTY_MAX_CLOCK,
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
    }
  return qcad_electrode_type ;
  }

static void qcad_electrode_class_init (GObjectClass *klass, gpointer data)
  {
#ifdef PROPERTY_UIS
  static QCADPropertyUIProperty properties[] =
    {
    {"z-to-ground", "units", {0, }},
		{"thickness",		"units", {0, }},
    {"frequency",   "units", {0, }},
    {"phase",       "units", {0, }},
    {"dc-offset",   "units", {0, }},
    {"min-clock",   "units", {0, }},
    {"max-clock",   "units", {0, }},
    } ;

  // electrode.z-to-ground.units = "nm"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), "nm") ;
	// electrode.thickness.units = "nm"
  g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;
  // electrode.frequency.units = "Hz"
  g_value_set_string (g_value_init (&(properties[2].ui_property_value), G_TYPE_STRING), "MHz") ;
  // electrode.phase.units = "deg"
  g_value_set_string (g_value_init (&(properties[3].ui_property_value), G_TYPE_STRING), "deg") ;
  // electrode.dc-offset.units = "V"
  g_value_set_string (g_value_init (&(properties[4].ui_property_value), G_TYPE_STRING), "V") ;
  // electrode.min-clock.units = "V"
  g_value_set_string (g_value_init (&(properties[5].ui_property_value), G_TYPE_STRING), "V") ;
  // electrode.max-clock.units = "V"
  g_value_set_string (g_value_init (&(properties[6].ui_property_value), G_TYPE_STRING), "V") ;

  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
#endif /* def PROPERTY_UIS */

  G_OBJECT_CLASS (klass)->finalize = qcad_electrode_instance_finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  QCAD_OBJECT_CLASS(klass)->copy = copy ;

  QCAD_DESIGN_OBJECT_CLASS(klass)->unserialize = unserialize ;
  QCAD_DESIGN_OBJECT_CLASS(klass)->serialize   = serialize ;

  QCAD_ELECTRODE_CLASS (klass)->get_voltage       = get_voltage ;
  QCAD_ELECTRODE_CLASS (klass)->get_potential     = get_potential ;
  QCAD_ELECTRODE_CLASS (klass)->get_area          = get_area ;
	QCAD_ELECTRODE_CLASS (klass)->get_long_side     = get_long_side ;
	QCAD_ELECTRODE_CLASS (klass)->get_short_side    = get_short_side ;
  QCAD_ELECTRODE_CLASS (klass)->precompute        = precompute ;
  QCAD_ELECTRODE_CLASS (klass)->extreme_potential = extreme_potential ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND,
    g_param_spec_double ("z-to-ground", _("Distance to ground"), _("Distance from electrode to ground electrode"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_THICKNESS,
    g_param_spec_double ("thickness", _("Electrode Thickness"), _("Electrode thickness in z direction"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
			
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_EPSILON_R,
    g_param_spec_double ("relative-permittivity", _("Relative permittivity"), _("Relative permittivity of the environment between the electrode and the ground"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_AMPLITUDE,
    g_param_spec_double ("amplitude", _("Amplitude"), _("Clock amplitude"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_FREQUENCY,
    g_param_spec_double ("frequency", _("Frequency"), _("Clock frequency"),
      G_MINDOUBLE, G_MAXDOUBLE, 1e6, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_PHASE,
    g_param_spec_double ("phase", _("Phase"), _("Clock phase"),
      0, 360, 270, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_DC_OFFSET,
    g_param_spec_double ("dc-offset", _("DC Offset"), _("Clock DC offset"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_MIN_CLOCK,
    g_param_spec_double ("min-clock", _("Clock Low"), _("Minimum clock voltage"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_ELECTRODE_PROPERTY_MAX_CLOCK,
    g_param_spec_double ("max-clock", _("Clock High"), _("Maximum clock voltage"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_electrode_instance_init (GObject *object, gpointer data)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;

  electrode->electrode_options.clock_function        = sin ;
  electrode->electrode_options.amplitude             =  5 ;
  electrode->electrode_options.frequency             =  57142e6 ;
  electrode->electrode_options.phase                 =  0 ;
  electrode->electrode_options.dc_offset             =  0 ;
  electrode->electrode_options.min_clock             = -5 ;
  electrode->electrode_options.max_clock             =  5 ;
  electrode->electrode_options.relative_permittivity = 12.9 ;
  electrode->electrode_options.z_to_ground           = 10.0 ;
	electrode->electrode_options.thickness						 = 1.0 ;

  precompute (electrode) ;
  }

static void qcad_electrode_instance_finalize (GObject *object)
  {G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_ELECTRODE)))->finalize (object) ;}

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

static void copy (QCADObject *src, QCADObject *dst)
  {
  QCADElectrode *electrode_src = NULL, *electrode_dst = NULL ;

  QCAD_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_ELECTRODE)))->copy (src, dst) ;

  electrode_src = QCAD_ELECTRODE (src) ; electrode_dst = QCAD_ELECTRODE (dst) ;

  memcpy (&(electrode_dst->electrode_options), &(electrode_src->electrode_options), sizeof (QCADElectrodeOptions)) ;
  memcpy (&(electrode_dst->precompute_params), &(electrode_src->precompute_params), sizeof (QCADElectrodePrecompute)) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND:
      electrode->electrode_options.z_to_ground = g_value_get_double (value) ;
      break ;
			
		case QCAD_ELECTRODE_PROPERTY_THICKNESS:
      electrode->electrode_options.thickness = g_value_get_double (value) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_EPSILON_R:
      electrode->electrode_options.relative_permittivity = g_value_get_double (value) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_AMPLITUDE:
      electrode->electrode_options.amplitude = g_value_get_double (value) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_FREQUENCY:
      electrode->electrode_options.frequency = g_value_get_double (value) * 1e6 ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_PHASE:
      electrode->electrode_options.phase = g_value_get_double (value) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_DC_OFFSET:
      electrode->electrode_options.dc_offset = g_value_get_double (value) ;
      break ;
    }

  g_object_notify (object, pspec->name) ;

  if (QCAD_ELECTRODE_PROPERTY_MIN_CLOCK == property_id)
    {
    electrode->electrode_options.min_clock = g_value_get_double (value) ;
    if (electrode->electrode_options.max_clock < electrode->electrode_options.min_clock)
      {
      electrode->electrode_options.max_clock = electrode->electrode_options.min_clock ;
      g_object_notify (object, "max-clock") ;
      }
    }
  else
  if (QCAD_ELECTRODE_PROPERTY_MAX_CLOCK == property_id)
    {
    electrode->electrode_options.max_clock = g_value_get_double (value) ;
    if (electrode->electrode_options.min_clock > electrode->electrode_options.max_clock)
      {
      electrode->electrode_options.min_clock = electrode->electrode_options.max_clock ;
      g_object_notify (object, "min-clock") ;
      }
    }

  QCAD_ELECTRODE_GET_CLASS (electrode)->precompute (electrode) ;
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADElectrode *electrode = QCAD_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_ELECTRODE_PROPERTY_Z_TO_GROUND:
      g_value_set_double (value, electrode->electrode_options.z_to_ground) ;
      break ;
			
		case QCAD_ELECTRODE_PROPERTY_THICKNESS:
      g_value_set_double (value, electrode->electrode_options.thickness) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_EPSILON_R:
      g_value_set_double (value, electrode->electrode_options.relative_permittivity) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_AMPLITUDE:
      g_value_set_double (value, electrode->electrode_options.amplitude) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_FREQUENCY:
      g_value_set_double (value, electrode->electrode_options.frequency / 1e6) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_PHASE:
      g_value_set_double (value, electrode->electrode_options.phase) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_DC_OFFSET:
      g_value_set_double (value, electrode->electrode_options.dc_offset) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_MIN_CLOCK:
      g_value_set_double (value, electrode->electrode_options.min_clock) ;
      break ;

    case QCAD_ELECTRODE_PROPERTY_MAX_CLOCK:
      g_value_set_double (value, electrode->electrode_options.max_clock) ;
      break ;
    }
  }

static void precompute (QCADElectrode *electrode)
  {
  QCADDesignObject *design_object = QCAD_DESIGN_OBJECT (electrode) ;

  electrode->precompute_params.permittivity = electrode->electrode_options.relative_permittivity * EPSILON ;
  electrode->precompute_params.two_z_to_ground = electrode->electrode_options.z_to_ground * 2.0 ;
	electrode->precompute_params.capacitance = 2.64e-11 * QCAD_ELECTRODE_GET_CLASS (electrode)->get_long_side (electrode) * (electrode->electrode_options.relative_permittivity + 1.41)/log((5.98 * electrode->electrode_options.z_to_ground * 1e-9)/(0.8 * QCAD_ELECTRODE_GET_CLASS (electrode)->get_short_side (electrode) + electrode->electrode_options.thickness * 1e-9));
		
  design_object->clr.red = (int)((electrode->electrode_options.phase * 65535) / TWO_PI) ;
  design_object->clr.green = 0xFFFF ;
  design_object->clr.blue = 0xC0C0 ;
  HSLToRGB (&(design_object->clr)) ;
#ifdef GTK_GUI
  gdk_colormap_alloc_color (gdk_colormap_get_system (), &(design_object->clr), FALSE, FALSE) ;
#endif /* def GTK_GUI */
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
	fprintf (fp, "thickness=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, electrode->electrode_options.thickness)) ;
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
			if (!strcmp (pszLine, "electrode_options.thickness"))
        electrode->electrode_options.thickness = g_ascii_strtod (pszValue, NULL) ;
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
      (TWO_PI * (electrode->electrode_options.frequency * t - electrode->electrode_options.phase/360.0)) +
    electrode->electrode_options.dc_offset ;

  return CLAMP (voltage, electrode->electrode_options.min_clock, electrode->electrode_options.max_clock) ;
  }

static double get_area (QCADElectrode *electrode)
  {return 0 ;}

static double get_long_side (QCADElectrode *electrode)
  {return 0 ;}

static double get_short_side (QCADElectrode *electrode)
  {return 0 ;}
