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
// The layer. This is a structure containing design     //
// objects. The kinds of objects a layer may contain    //
// depend on the kind of layer it is.                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "QCADClockingLayer.h"
#include "QCADLayer_priv.h"
#include "support.h"
#include "../fileio_helpers.h"
#include "QCADElectrode.h"
#include "../global_consts.h"
#include "../custom_widgets.h"

enum
  {
  QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL = 1,
  QCAD_CLOCKING_LAYER_PROPERTY_Z_SHOWING,
  QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE,
  QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD,
  QCAD_CLOCKING_LAYER_PROPERTY_Z_TO_GROUND,
  QCAD_CLOCKING_LAYER_PROPERTY_EPSILON_R,
  QCAD_CLOCKING_LAYER_PROPERTY_LAST
  } ;

static void qcad_clocking_layer_class_init (QCADDesignObjectClass *klass, gpointer data) ;
static void qcad_clocking_layer_instance_init (QCADDesignObject *object, gpointer data) ;
static void qcad_clocking_layer_instance_finalize (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) ;
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean do_container_add (QCADDOContainer *container, QCADDesignObject *obj) ;
static gboolean do_container_remove (QCADDOContainer *container, QCADDesignObject *obj) ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */

static void qcad_clocking_layer_calculate_extreme_potentials (QCADClockingLayer *clocking_layer) ;

GType qcad_clocking_layer_get_type ()
  {
  static GType qcad_clocking_layer_type = 0 ;

  if (!qcad_clocking_layer_type)
    {
    static const GTypeInfo qcad_clocking_layer_info =
      {
      sizeof (QCADClockingLayerClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_clocking_layer_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADClockingLayer),
      0,
      (GInstanceInitFunc)qcad_clocking_layer_instance_init
      } ;

    if ((qcad_clocking_layer_type = g_type_register_static (QCAD_TYPE_LAYER, QCAD_TYPE_STRING_CLOCKING_LAYER, &qcad_clocking_layer_info, 0)))
      g_type_class_ref (qcad_clocking_layer_type) ;
    }
  return qcad_clocking_layer_type ;
  }

static void qcad_clocking_layer_class_init (QCADDesignObjectClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->finalize     = qcad_clocking_layer_instance_finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw        = draw ;
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize   = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;

  QCAD_LAYER_CLASS (klass)->do_container_add    = do_container_add ;
  QCAD_LAYER_CLASS (klass)->do_container_remove = do_container_remove ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL,
    g_param_spec_boolean ("show-potential", _("Show Potential"), _("Show potential created by the electrodes on this layer"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_Z_SHOWING,
    g_param_spec_double ("z-showing", _("Distance from layer"), _("Distance from clocking layer to show the cross-section for"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE,
    g_param_spec_uint ("tile-size", _("Tile Size"), _("Resolution (n x n pixels) used to draw the potential"),
      1, G_MAXUINT, 16, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD,
    g_param_spec_double ("time-coord", _("Time Coordinate"), _("Time coordinate to draw the potential for"),
      0, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_Z_TO_GROUND,
    g_param_spec_double ("z-to-ground", _("Distance to ground"), _("Distance from clocking layer to ground electrode"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_EPSILON_R,
    g_param_spec_double ("relative-permittivity", _("Relative permittivity"), _("Relative permittivity of the environment between the clocking layer and the ground"),
      1, G_MAXDOUBLE, 1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_clocking_layer_instance_init (QCADDesignObject *object, gpointer data)
  {
  QCADLayer *layer = QCAD_LAYER (object) ;

  layer->type = LAYER_TYPE_CLOCKING ;
  layer->default_properties = qcad_layer_create_default_properties (LAYER_TYPE_CLOCKING) ;
  QCAD_CLOCKING_LAYER (layer)->bDrawPotential = FALSE ;
  QCAD_CLOCKING_LAYER (layer)->z_to_draw             =  1 ;
  QCAD_CLOCKING_LAYER (layer)->tile_size             = 16.0 ;
  QCAD_CLOCKING_LAYER (layer)->time_coord            =  0.0 ;
  QCAD_CLOCKING_LAYER (layer)->z_to_ground           = 20.0 ;
  QCAD_CLOCKING_LAYER (layer)->relative_permittivity = 12.9 ;
  QCAD_CLOCKING_LAYER (layer)->dExtremePotential     =  0 ;
  }

static void qcad_clocking_layer_instance_finalize (GObject *object)
  {G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LAYER)))->finalize (object) ;}

static gboolean do_container_add (QCADDOContainer *container, QCADDesignObject *obj)
  {
  gboolean bRet = FALSE ;
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (container) ;

  if ((bRet = QCAD_LAYER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->do_container_add (container, obj)))
    if (QCAD_IS_ELECTRODE (obj))
      {
      EXTREME_POTENTIALS dElectrodeExtremePotential = qcad_electrode_get_extreme_potential (QCAD_ELECTRODE (obj), clocking_layer->z_to_draw) ;

      fprintf (stderr, "QCADClockingLayer::do_container_add:Adding electrode 0x%08X with potentials (%e,%e)\n", (int)obj, dElectrodeExtremePotential.min, dElectrodeExtremePotential.max) ;

      dElectrodeExtremePotential.min = fabs (dElectrodeExtremePotential.min) ;
      dElectrodeExtremePotential.max = fabs (dElectrodeExtremePotential.max) ;
      clocking_layer->dExtremePotential = MAX (dElectrodeExtremePotential.min, MAX (dElectrodeExtremePotential.max, clocking_layer->dExtremePotential)) ;

      fprintf (stderr, "QCADClockingLayer::do_container_add:Resulting extreme potential is %e\n", clocking_layer->dExtremePotential) ;
      }

  return bRet ;
  }

static gboolean do_container_remove (QCADDOContainer *container, QCADDesignObject *obj)
  {
  gboolean bRet = FALSE ;
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (container) ;

  if ((bRet = QCAD_LAYER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->do_container_remove (container, obj)))
    if (QCAD_IS_ELECTRODE (obj))
      {
      fprintf (stderr, "QCADClockingLayer::do_container_remove:Removing electrode 0x%08X\n", (int)obj) ;
      qcad_clocking_layer_calculate_extreme_potentials (QCAD_CLOCKING_LAYER (container)) ;
      fprintf (stderr, "QCADClockingLayer::do_container_remove:Resulting extreme potential is %e\n", clocking_layer->dExtremePotential) ;
      }

  return bRet ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  GList *llItr = NULL ;
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (object) ;
  QCADLayer *layer = QCAD_LAYER (object) ;

  switch (property_id)
    {
    case QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL:
      clocking_layer->bDrawPotential = g_value_get_boolean (value) ;
      g_object_notify (object, "show-potential") ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_Z_SHOWING:
      clocking_layer->z_to_draw = g_value_get_double (value) ;
      g_object_notify (object, "z-showing") ;
      qcad_clocking_layer_calculate_extreme_potentials (clocking_layer) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE:
      clocking_layer->tile_size = g_value_get_uint (value) ;
      g_object_notify (object, "tile-size") ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD:
      clocking_layer->time_coord = g_value_get_double (value) ;
      g_object_notify (object, "time-coord") ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_Z_TO_GROUND:
      clocking_layer->z_to_ground = g_value_get_double (value) ;
      if (clocking_layer->z_to_ground < clocking_layer->z_to_draw)
        {
        clocking_layer->z_to_draw = clocking_layer->z_to_ground ;
        g_object_notify (object, "z-showing") ;
        }
      for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
        if (NULL != llItr->data)
          if (QCAD_IS_ELECTRODE (llItr->data))
            g_object_set (G_OBJECT (llItr->data), "z-to-ground", clocking_layer->z_to_ground, NULL) ;
      QCAD_ELECTRODE_CLASS (g_type_class_peek (QCAD_TYPE_ELECTRODE))->default_electrode_options.z_to_ground = clocking_layer->z_to_ground ;
      g_object_notify (object, "z-to-ground") ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_EPSILON_R:
      clocking_layer->relative_permittivity = g_value_get_double (value) ;
      for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
        if (NULL != llItr->data)
          if (QCAD_IS_ELECTRODE (llItr->data))
            g_object_set (G_OBJECT (llItr->data), "relative-permittivity", clocking_layer->relative_permittivity, NULL) ;
      QCAD_ELECTRODE_CLASS (g_type_class_peek (QCAD_TYPE_ELECTRODE))->default_electrode_options.relative_permittivity = clocking_layer->relative_permittivity ;
      g_object_notify (object, "relative-permittivity") ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADClockingLayer *layer = QCAD_CLOCKING_LAYER (object) ;

  switch (property_id)
    {
    case QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL:
      g_value_set_boolean (value, layer->bDrawPotential) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_Z_SHOWING:
      g_value_set_double (value, layer->z_to_draw) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE:
       g_value_set_uint (value, layer->tile_size) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD:
       g_value_set_double (value, layer->time_coord) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_Z_TO_GROUND:
       g_value_set_double (value, layer->z_to_ground) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_EPSILON_R:
       g_value_set_double (value, layer->relative_permittivity) ;
      break ;
    }
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADClockingLayer *clocking_layer = NULL ;
  char pszDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;

  if (NULL == obj) return ;

  clocking_layer = QCAD_CLOCKING_LAYER (obj) ;

  fprintf (fp, "[TYPE:" QCAD_TYPE_STRING_CLOCKING_LAYER "]\n") ;
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->serialize (obj, fp) ;
  fprintf (fp, "bDrawPotential=%s\n", clocking_layer->bDrawPotential ? "TRUE" : "FALSE") ;
  fprintf (fp, "z_to_draw=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, clocking_layer->z_to_draw)) ;
  fprintf (fp, "tile_size=%d\n", clocking_layer->tile_size) ;
  fprintf (fp, "time_coord=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, clocking_layer->time_coord)) ;
  fprintf (fp, "relative_permittivity=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, clocking_layer->relative_permittivity)) ;
  fprintf (fp, "z_to_ground=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, clocking_layer->z_to_ground)) ;
  fprintf (fp, "[#TYPE:" QCAD_TYPE_STRING_CLOCKING_LAYER "]\n") ;
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_CLOCKING_LAYER "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;
    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_CLOCKING_LAYER "]", pszLine))
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

        if (!strcmp (pszValue, QCAD_TYPE_STRING_LAYER))
          {
          if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->unserialize (obj, fp)))
            bStopReading = TRUE ;
          }
        }
      else
      if (!strcmp (pszLine, "bDrawPotential"))
        clocking_layer->bDrawPotential = strcmp (pszValue, "TRUE") ? FALSE : TRUE ;
      else
      if (!strcmp (pszLine, "z_to_draw"))
        clocking_layer->z_to_draw = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "tile_size"))
        clocking_layer->tile_size = atoi (pszValue) ;
      else
      if (!strcmp (pszLine, "time_coord"))
        clocking_layer->time_coord = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "z_to_ground"))
        clocking_layer->z_to_ground = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "relative_permittivity"))
        clocking_layer->relative_permittivity = g_ascii_strtod (pszValue, NULL) ;
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }
  return (bParentInit && !bStopReading) ;
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
	int Nix, Nix1 ;
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (obj) ;
  QCADLayer *layer = QCAD_LAYER (obj) ;
  GdkGC *gc = NULL ;
  double potential ;
  GList *llItr = NULL ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->draw (obj, dst, rop, rcClip) ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (RED)) ;
  gdk_gc_set_background (gc, clr_idx_to_clr_struct (RED)) ;
  gdk_gc_set_clip_rectangle (gc, rcClip) ;

  if (NULL != layer->lstObjs && clocking_layer->bDrawPotential)
    {
    GdkPixbuf *pb = NULL ;

    fprintf (stderr, "QCADClockingLayer::draw:Drawing halo for %e s\n", clocking_layer->time_coord) ;

    pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, clocking_layer->tile_size, clocking_layer->tile_size) ;
    for (Nix = rcClip->x / clocking_layer->tile_size ; Nix * clocking_layer->tile_size < rcClip->x + rcClip->width ; Nix++)
      for (Nix1 = rcClip->y / clocking_layer->tile_size ; Nix1 * clocking_layer->tile_size < rcClip->y + rcClip->height ; Nix1++)
        {
        potential = 0 ;
        for (llItr = layer->lstObjs ; llItr != NULL ; llItr = llItr->next)
          if (NULL != llItr->data)
            if (QCAD_IS_ELECTRODE (llItr->data))
              potential += 
                qcad_electrode_get_potential (QCAD_ELECTRODE (llItr->data), 
                  real_to_world_x (Nix * clocking_layer->tile_size + (clocking_layer->tile_size >> 1)), 
                  real_to_world_y (Nix1 * clocking_layer->tile_size + (clocking_layer->tile_size >> 1)), 
                  clocking_layer->z_to_draw, clocking_layer->time_coord) ;

        gdk_pixbuf_fill (pb,
          ((potential > 0) ? 0xFF000000 : 0x0000FF00) | (((int)((fabs (potential) / clocking_layer->dExtremePotential) * 128.0)) & 0xFF)) ;
//        fprintf (stderr, "opacity = %lf/%lf * 255\n", potential, clocking_layer->dExtremePotential) ;

        gdk_draw_pixbuf (dst, gc, pb, 0, 0, Nix * clocking_layer->tile_size, Nix1 * clocking_layer->tile_size, clocking_layer->tile_size, clocking_layer->tile_size, GDK_RGB_DITHER_NONE, 0, 0) ;

//        gdk_draw_rectangle (dst, gc, TRUE, 
//          Nix * clocking_layer->tile_size + (clocking_layer->tile_size >> 1) - 2,
//          Nix1 * clocking_layer->tile_size + (clocking_layer->tile_size >> 1) - 2,
//          5, 5) ;
        }
    g_object_unref (pb) ;
    }

  g_object_unref (gc) ;
	}
#endif /* def GTK_GUI */

static void qcad_clocking_layer_calculate_extreme_potentials (QCADClockingLayer *clocking_layer)
  {
  GList *llItr = NULL ;
  EXTREME_POTENTIALS dElectrodeExtremePotential = {0, 0} ;

  fprintf (stderr, "QCADClockingLayer::calculate_extreme_potentials:Entering\n") ;

  clocking_layer->dExtremePotential = 0 ;

  for (llItr = QCAD_LAYER (clocking_layer)->lstObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != llItr->data)
      if (QCAD_IS_ELECTRODE (llItr->data))
        {
        dElectrodeExtremePotential = qcad_electrode_get_extreme_potential (QCAD_ELECTRODE (llItr->data), clocking_layer->z_to_draw) ;
        dElectrodeExtremePotential.min = fabs (dElectrodeExtremePotential.min) ;
        dElectrodeExtremePotential.max = fabs (dElectrodeExtremePotential.max) ;
        clocking_layer->dExtremePotential = MAX (dElectrodeExtremePotential.min, MAX (dElectrodeExtremePotential.max, clocking_layer->dExtremePotential)) ;
        }
  }
