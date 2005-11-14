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

#include "QCADClockingLayer.h"
#include "QCADLayer_priv.h"
#include "support.h"
#include "../fileio_helpers.h"

enum
  {
  QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL = 1,
  QCAD_CLOCKING_LAYER_PROPERTY_DISTANCE,
  QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE,
  QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD,
  QCAD_CLOCKING_LAYER_PROPERTY_LAST
  } ;

static void qcad_clocking_layer_class_init (QCADDesignObjectClass *klass, gpointer data) ;
static void qcad_clocking_layer_instance_init (QCADDesignObject *object, gpointer data) ;
static void qcad_clocking_layer_instance_finalize (GObject *object) ;
static void qcad_clocking_layer_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void qcad_clocking_layer_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) ;
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */

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
  G_OBJECT_CLASS (klass)->set_property = qcad_clocking_layer_set_property ;
  G_OBJECT_CLASS (klass)->get_property = qcad_clocking_layer_get_property ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw        = draw ;
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize   = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL,
    g_param_spec_boolean ("show-potential", _("Show Potential"), _("Show potential created by the electrodes on this layer"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_DISTANCE,
    g_param_spec_double ("distance", _("Distance from layer"), _("Distance from clocking layer to show the cross-section for"),
      -G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE,
    g_param_spec_uint ("tile-size", _("Tile Size"), _("Resolution (n x n pixels) used to draw the potential"),
      1, G_MAXUINT, 16, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD,
    g_param_spec_double ("time-coord", _("Time Coordinate"), _("Time coordinate to draw the potential for"),
      0, G_MAXDOUBLE, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  }

static void qcad_clocking_layer_instance_init (QCADDesignObject *object, gpointer data)
  {
  QCADLayer *layer = QCAD_LAYER (object) ;

  layer->type = LAYER_TYPE_CLOCKING ;
  layer->default_properties = qcad_layer_create_default_properties (LAYER_TYPE_CLOCKING) ;
  QCAD_CLOCKING_LAYER (layer)->bDrawPotential = FALSE ;
  QCAD_CLOCKING_LAYER (layer)->z_to_draw  =  0 ;
  QCAD_CLOCKING_LAYER (layer)->tile_size  = 16 ;
  QCAD_CLOCKING_LAYER (layer)->time_coord =  0 ;
  }

static void qcad_clocking_layer_instance_finalize (GObject *object)
  {G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LAYER)))->finalize (object) ;}

static void qcad_clocking_layer_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADClockingLayer *layer = QCAD_CLOCKING_LAYER (object) ;

  switch (property_id)
    {
    case QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL:
      layer->bDrawPotential = g_value_get_boolean (value) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_DISTANCE:
      layer->z_to_draw = g_value_get_double (value) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE:
      layer->tile_size = g_value_get_uint (value) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD:
      layer->time_coord = g_value_get_double (value) ;
      break ;
    }
  }

static void qcad_clocking_layer_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADClockingLayer *layer = QCAD_CLOCKING_LAYER (object) ;

  switch (property_id)
    {
    case QCAD_CLOCKING_LAYER_PROPERTY_SHOW_POTENTIAL:
      g_value_set_boolean (value, layer->bDrawPotential) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_DISTANCE:
      g_value_set_double (value, layer->z_to_draw) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TILE_SIZE:
       g_value_set_uint (value, layer->tile_size) ;
      break ;

    case QCAD_CLOCKING_LAYER_PROPERTY_TIME_COORD:
       g_value_set_double (value, layer->time_coord) ;
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
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }
  return (bParentInit && !bStopReading) ;
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  /*
	int xStart, yStart, Nix, Nix1 ;
  QCADClockingLayer *clocking_layer = QCAD_CLOCKING_LAYER (obj) ;
  QCADLayer *layer = QCAD_LAYER (obj) ;
  GdkPixbuf *pb = NULL ;
  GdkGC *gc = NULL ;
  double xWorld, yWorld, potential ;

  gdk_window_get_size (dst, &cx, &cy) ;
*/
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CLOCKING_LAYER)))->draw (obj, dst, rop, rcClip) ;
/*
  gc = gdk_gc_new (dst) ;
  pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, clocking_layer->tile_size, clocking_layer->tile_size) ;

  xStart = ((int)(rcClip.x / clocking_layer->tile_size)) * clocking_layer->tile_size ;
  yStart = ((int)(rcClip.y / clocking_layer->tile_size)) * clocking_layer->tile_size ;

  for (Nix = 0 ; xStart < rcClip.x + rcClip.width ; xStart += clocking_layer->tile_size,Nix++)
    for (Nix1 = 0 ; yStart < rcClip.y + rcClip.height ; yStart += clocking_layer->tile_size,Nix1++)
      {
      xWorld = real_to_world_x (xStart + (clocking_layer->tile_size >> 1) ;
      yWorld = real_to_world_y (yStart + (clocking_layer->tile_size >> 1) ;

      potential = 0 ;
      for (ll
      }
      
  g_object_unref (pb) ;
  g_object_unref (gc) ;
  */
	}
#endif /* def GTK_GUI */
