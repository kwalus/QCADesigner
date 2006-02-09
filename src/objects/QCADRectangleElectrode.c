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
#include "QCADRectangleElectrode.h"
#include "mouse_handlers.h"

#ifdef DESIGNER
extern DropFunction drop_function ;
#endif /* def DESIGNER */

static struct { ClockFunction clock_function ; char *pszDescription ; } clock_functions[1]  =
  {
  {sin, "sin"}
  } ;
int n_clock_functions = G_N_ELEMENTS (clock_functions) ;

enum
  {
  QCAD_RECTANGLE_ELECTRODE_FIRST=1,

  QCAD_RECTANGLE_ELECTRODE_PROPERTY_CX,
  QCAD_RECTANGLE_ELECTRODE_PROPERTY_CY,
  QCAD_RECTANGLE_ELECTRODE_PROPERTY_X_DOTS,
  QCAD_RECTANGLE_ELECTRODE_PROPERTY_Y_DOTS,

  QCAD_RECTANGLE_ELECTRODE_LAST
  } ;

static void qcad_rectangle_electrode_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_rectangle_electrode_instance_init (GObject *object, gpointer data) ;

static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
#endif /* def GTK_GUI */
static void copy (QCADObject *src, QCADObject *dst) ;
static void move (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t) ;
static double get_area (QCADElectrode *electrode) ;
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
static EXTREME_POTENTIALS extreme_potential (QCADElectrode *electrode, double z) ;
static void precompute (QCADElectrode *electrode) ;
static QCADObject *class_get_default_object () ;

GType qcad_rectangle_electrode_get_type ()
  {
  static GType qcad_rectangle_electrode_type = 0 ;

  if (!qcad_rectangle_electrode_type)
    {
    static const GTypeInfo qcad_rectangle_electrode_info =
      {
      sizeof (QCADRectangleElectrodeClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_rectangle_electrode_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADRectangleElectrode),
      0,
      (GInstanceInitFunc)qcad_rectangle_electrode_instance_init
      } ;

    if ((qcad_rectangle_electrode_type = g_type_register_static (QCAD_TYPE_ELECTRODE, QCAD_TYPE_STRING_RECTANGLE_ELECTRODE, &qcad_rectangle_electrode_info, 0)))
      g_type_class_ref (qcad_rectangle_electrode_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADRectangleElectrode as %d\n", (int)qcad_rectangle_electrode_type)) ;
    }
  return qcad_rectangle_electrode_type ;
  }

static void qcad_rectangle_electrode_class_init (GObjectClass *klass, gpointer data)
  {
  // Gotta be static so the strings don't die
  static QCADPropertyUIProperty properties[] =
    {
    {NULL,     "title", {0, }},
    {"width",  "units", {0, }},
    {"height", "units", {0, }},
    } ;

  // RectangleElectrode.title = "QCA Rectangular Electrode"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), _("QCA Rectangular Electrode")) ;
  // RectangleElectrode.width.units = "nm"
  g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;
  // RectangleElectrode.height.units = "nm"
  g_value_set_string (g_value_init (&(properties[2].ui_property_value), G_TYPE_STRING), "nm") ;

  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;

  DBG_OO (fprintf (stderr, "QCADRectangleElectrode::class_init:Leaving\n")) ;
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = class_get_default_object ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                       = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed          = (GCallback)button_pressed ;
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT_CLASS (klass)->move                       = move ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize                  = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize                = unserialize ;

  QCAD_ELECTRODE_CLASS (klass)->get_potential     = get_potential ;
  QCAD_ELECTRODE_CLASS (klass)->get_area          = get_area ;
  QCAD_ELECTRODE_CLASS (klass)->precompute        = precompute ;
  QCAD_ELECTRODE_CLASS (klass)->extreme_potential = extreme_potential ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_RECTANGLE_ELECTRODE_PROPERTY_CX,
    g_param_spec_double ("width", _("Electrode Width"), _("Electrode Width"),
      0.1, 1e9,  6.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_RECTANGLE_ELECTRODE_PROPERTY_CY,
    g_param_spec_double ("height", _("Electrode Height"), _("Electrode Height"),
      0.1, 1e9, 40.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_RECTANGLE_ELECTRODE_PROPERTY_X_DOTS,
    g_param_spec_uint ("x-dots", _("")
  }

static void qcad_rectangle_electrode_instance_init (GObject *object, gpointer data)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (object) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_init:Entering\n")) ;

  rc_electrode->n_x_divisions =  2 ;
  rc_electrode->n_y_divisions = 10 ;
  rc_electrode->cxWorld       =  6.0 ;
  rc_electrode->cyWorld       = 40.0 ;
  rc_electrode->angle         =  0.0 ;
  precompute (QCAD_ELECTRODE (object)) ;

  DBG_OO (fprintf (stderr, "QCADElectrode::instance_init:Leaving\n")) ;
  }

static void finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_finalize:Entering\n")) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_ELECTRODE)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADElectrode::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

static QCADObject *class_get_default_object ()
  {return g_object_new (QCAD_TYPE_RECTANGLE_ELECTRODE, NULL) ;}

static void copy (QCADObject *src, QCADObject *dst)
  {
  QCADRectangleElectrode *rc_electrode_src, *rc_electrode_dst ;

  QCAD_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_ELECTRODE)))->copy (src, dst) ;

  rc_electrode_src = QCAD_RECTANGLE_ELECTRODE (src) ; rc_electrode_dst = QCAD_RECTANGLE_ELECTRODE (dst) ;

  rc_electrode_dst->angle         = rc_electrode_src->angle ;
  rc_electrode_dst->n_x_divisions = rc_electrode_src->n_x_divisions ;
  rc_electrode_dst->n_y_divisions = rc_electrode_src->n_y_divisions ;
  rc_electrode_dst->cxWorld       = rc_electrode_src->cxWorld ;
  rc_electrode_dst->cyWorld       = rc_electrode_src->cyWorld ;
  memcpy (&(rc_electrode_dst->precompute_params), &(rc_electrode_src->precompute_params), sizeof (QCADRectangleElectrodePrecompute)) ;
  if (NULL != rc_electrode_src->precompute_params.pts)
    rc_electrode_dst->precompute_params.pts = exp_array_copy (rc_electrode_src->precompute_params.pts) ;
  }

static EXTREME_POTENTIALS extreme_potential (QCADElectrode *electrode, double z)
  {
  EXTREME_POTENTIALS ret = {0, 0} ;
  double p_over_two_pi_f = (electrode->electrode_options.phase / (TWO_PI * electrode->electrode_options.frequency)) ;
  double one_over_four_f = (1.0 / (4.0 * electrode->electrode_options.frequency)) ;
  // This assumes a sin function
  ret.max = get_potential (electrode, QCAD_DESIGN_OBJECT (electrode)->x, QCAD_DESIGN_OBJECT (electrode)->y, z, 
    p_over_two_pi_f + one_over_four_f) ;
  ret.min = get_potential (electrode, QCAD_DESIGN_OBJECT (electrode)->x, QCAD_DESIGN_OBJECT (electrode)->y, z, 
    p_over_two_pi_f - one_over_four_f) ;

  return ret ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_RECTANGLE_ELECTRODE_PROPERTY_CX:
      {
      double new_cx = g_value_get_double (value) ;

      if (new_cx == rc_electrode->cxWorld) break ;
      rc_electrode->cxWorld = new_cx ;
      precompute (QCAD_ELECTRODE (object)) ;
      g_object_notify (object, "width") ;
      break ;
      }

    case QCAD_RECTANGLE_ELECTRODE_PROPERTY_CY:
      {
      double new_cy = g_value_get_double (value) ;

      if (new_cy == rc_electrode->cyWorld) break ;
      rc_electrode->cyWorld = new_cy ;
      precompute (QCAD_ELECTRODE (object)) ;
      g_object_notify (object, "height") ;
      break ;
      }
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (object) ;

  switch (property_id)
    {
    case QCAD_RECTANGLE_ELECTRODE_PROPERTY_CX:
      g_value_set_double (value, rc_electrode->cxWorld) ;
      break ;

    case QCAD_RECTANGLE_ELECTRODE_PROPERTY_CY:
      g_value_set_double (value, rc_electrode->cyWorld) ;
      break ;
    }
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  char pszDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (obj) ;

  fprintf (fp, "[TYPE:" QCAD_TYPE_STRING_RECTANGLE_ELECTRODE "]\n") ;
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_ELECTRODE)))->serialize (obj, fp) ;
  fprintf (fp, "angle=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, rc_electrode->angle)) ;
  fprintf (fp, "n_x_divisions=%d\n", rc_electrode->n_x_divisions) ;
  fprintf (fp, "n_y_divisions=%d\n", rc_electrode->n_y_divisions) ;
  fprintf (fp, "cxWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, rc_electrode->cxWorld)) ;
  fprintf (fp, "cyWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, rc_electrode->cyWorld)) ;
  fprintf (fp, "[#TYPE:" QCAD_TYPE_STRING_RECTANGLE_ELECTRODE "]\n") ;
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_RECTANGLE_ELECTRODE "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;

    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_RECTANGLE_ELECTRODE "]", pszLine))
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

        if (!strcmp (pszValue, QCAD_TYPE_STRING_ELECTRODE))
          {
          if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_ELECTRODE)))->unserialize (obj, fp)))
            bStopReading = TRUE ;
          }
        }
      else
      if (!strcmp (pszLine, "angle"))
        rc_electrode->angle = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "n_x_divisions"))
        rc_electrode->n_x_divisions = atoi (pszValue) ;
      else
      if (!strcmp (pszLine, "n_y_divisions"))
        rc_electrode->n_y_divisions = atoi (pszValue) ;
      else
      if (!strcmp (pszLine, "cxWorld"))
        rc_electrode->cxWorld = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cyWorld"))
        rc_electrode->cyWorld = g_ascii_strtod (pszValue, NULL) ;
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  if (bParentInit && !bStopReading)
    {
    if (QCAD_DESIGN_OBJECT_CLASS (QCAD_RECTANGLE_ELECTRODE_GET_CLASS (rc_electrode))->unserialize == unserialize)
      QCAD_ELECTRODE_GET_CLASS (rc_electrode)->precompute (QCAD_ELECTRODE (rc_electrode)) ;
    return TRUE ;
    }
  else
    return FALSE ;
  }

static void move (QCADDesignObject *obj, double dxDelta, double dyDelta)
  {
  int Nix, Nix1 ;
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (obj) ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek_parent (QCAD_DESIGN_OBJECT_GET_CLASS (obj)))->move (obj, dxDelta, dyDelta) ;

  rc_electrode->precompute_params.pt[0].xWorld += dxDelta ;
  rc_electrode->precompute_params.pt[0].yWorld += dyDelta ;
  rc_electrode->precompute_params.pt[1].xWorld += dxDelta ;
  rc_electrode->precompute_params.pt[1].yWorld += dyDelta ;
  rc_electrode->precompute_params.pt[2].xWorld += dxDelta ;
  rc_electrode->precompute_params.pt[2].yWorld += dyDelta ;
  rc_electrode->precompute_params.pt[3].xWorld += dxDelta ;
  rc_electrode->precompute_params.pt[3].yWorld += dyDelta ;

  if (NULL != rc_electrode->precompute_params.pts)
    for (Nix = 0 ; Nix < rc_electrode->n_x_divisions ; Nix++)
      for (Nix1 = 0 ; Nix1 < rc_electrode->n_y_divisions ; Nix1++)
        {
        exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).xWorld += dxDelta ;
        exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).yWorld += dyDelta ;
        }
  }
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  int Nix, Nix1 ;
  GdkGC *gc = NULL ;
  GdkRectangle rcReal ;
  GdkPoint ptSrc, ptDst ;
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (obj) ;
  GdkColor *clr = NULL ;

  world_to_real_rect (&(obj->bounding_box), &rcReal) ;

  if (!RECT_INTERSECT_RECT (rcReal.x, rcReal.y, rcReal.width, rcReal.height, rcClip->x, rcClip->y, rcClip->width, rcClip->height))
    return ;

  clr = obj->bSelected ? clr_idx_to_clr_struct (RED) : &(obj->clr) ;
  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, clr) ;
  gdk_gc_set_background (gc, clr) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_clip_rectangle (gc, rcClip) ;

  ptSrc.x = world_to_real_x (rc_electrode->precompute_params.pt[0].xWorld) ;
  ptSrc.y = world_to_real_y (rc_electrode->precompute_params.pt[0].yWorld) ;
  ptDst.x = world_to_real_x (rc_electrode->precompute_params.pt[1].xWorld) ;
  ptDst.y = world_to_real_y (rc_electrode->precompute_params.pt[1].yWorld) ;
  gdk_draw_line (dst, gc, ptSrc.x, ptSrc.y, ptDst.x, ptDst.y) ;
  ptSrc = ptDst ;
  ptDst.x = world_to_real_x (rc_electrode->precompute_params.pt[2].xWorld) ;
  ptDst.y = world_to_real_y (rc_electrode->precompute_params.pt[2].yWorld) ;
  gdk_draw_line (dst, gc, ptSrc.x, ptSrc.y, ptDst.x, ptDst.y) ;
  ptSrc = ptDst ;
  ptDst.x = world_to_real_x (rc_electrode->precompute_params.pt[3].xWorld) ;
  ptDst.y = world_to_real_y (rc_electrode->precompute_params.pt[3].yWorld) ;
  gdk_draw_line (dst, gc, ptSrc.x, ptSrc.y, ptDst.x, ptDst.y) ;
  ptSrc = ptDst ;
  ptDst.x = world_to_real_x (rc_electrode->precompute_params.pt[0].xWorld) ;
  ptDst.y = world_to_real_y (rc_electrode->precompute_params.pt[0].yWorld) ;
  gdk_draw_line (dst, gc, ptSrc.x, ptSrc.y, ptDst.x, ptDst.y) ;

  for (Nix = 0 ; Nix < rc_electrode->n_x_divisions ; Nix++)
    for (Nix1 = 0 ; Nix1 < rc_electrode->n_y_divisions ; Nix1++)
      {
      ptSrc.x = world_to_real_x (exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).xWorld) ;
      ptSrc.y = world_to_real_y (exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).yWorld) ;
      if (PT_IN_RECT (ptSrc.x, ptSrc.y, rcClip->x, rcClip->y, rcClip->width, rcClip->height))
        gdk_draw_point (dst, gc, ptSrc.x, ptSrc.y) ;
      }

  g_object_unref (gc) ;
  }

static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADDesignObject *obj = NULL ;
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;

  if (1 != event->button) return FALSE ;

#ifdef DESIGNER
  world_to_grid_pt (&xWorld, &yWorld) ;
#endif /* def DESIGNER */

//  fprintf (stderr, "QCADRectangleElectrode::button_pressed:Calling qcad_rectangle_electrode_new\n") ;
  obj = QCAD_DESIGN_OBJECT (qcad_object_new_from_object (qcad_object_get_default (QCAD_TYPE_RECTANGLE_ELECTRODE))) ;
//  fprintf (stderr, "QCADRectangleElectrode::button_pressed:Calling qcad_design_object_move\n") ;
  qcad_design_object_move_to (obj, xWorld, yWorld) ;

#ifdef DESIGNER
  if (NULL != drop_function)
    if ((*drop_function) (obj))
      return FALSE ;
#endif /* def DESIGNER */

  g_object_unref (obj) ;

  return FALSE ;
  }
#endif /* def GTK_GUI */

/*
static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (electrode) ;
  int Nix, Nix1 ;
  double potential = 0, rho = 0 ;
  double cx, cy, cz_mir, cx_sq_plus_cy_sq ;

  if (z < 0 || NULL == electrode) return 0 ;

  rho = rc_electrode->precompute_params.rho_factor * qcad_electrode_get_voltage (electrode, t) ;
  
  for (Nix = 0 ; Nix < rc_electrode->n_x_divisions ; Nix++)
    for (Nix1 = 0 ; Nix1 < rc_electrode->n_y_divisions ; Nix1++)
      {
      cx = x - exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).xWorld ;
      cy = y - exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).yWorld ;
      cx_sq_plus_cy_sq = cx * cx + cy * cy ;

      cz_mir = electrode->precompute_params.two_z_to_ground - z ;

      potential +=
        (rho * ((1.0 / sqrt (cx_sq_plus_cy_sq +   z    *   z   )) - 
                (1.0 / sqrt (cx_sq_plus_cy_sq + cz_mir * cz_mir))) * 1e9)
        / (FOUR_PI * electrode->precompute_params.permittivity) ;
      }

  return potential ;
  }
*/

static double get_potential (QCADElectrode *electrode, double x, double y, double z, double t)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (electrode) ;
  int Nix, Nix1 ;
  double z_sq, rho, cz_mirror_sq, cx, cy, cx_sq_plus_cy_sq, potential = 0 ;

  if (z < 0 || NULL == electrode) return 0 ;

  z_sq = z * z ;
  rho = rc_electrode->precompute_params.rho_factor * qcad_electrode_get_voltage (electrode, t) ;
  cz_mirror_sq = electrode->precompute_params.two_z_to_ground - z ;
  cz_mirror_sq *= cz_mirror_sq ;
  
  for (Nix = 0 ; Nix < rc_electrode->n_x_divisions ; Nix++)
    for (Nix1 = 0 ; Nix1 < rc_electrode->n_y_divisions ; Nix1++)
      {
      cx = x - exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).xWorld ;
      cy = y - exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).yWorld ;
      cx_sq_plus_cy_sq = cx * cx + cy * cy ;

      // The multiplication by 1e9 has moved into the calculation of the rho_factor in precompute
      // and is now part of rho, which is multiplied into the potential at the end
      potential +=
        ((1.0 / sqrt (cx_sq_plus_cy_sq +     z_sq    )) - 
         (1.0 / sqrt (cx_sq_plus_cy_sq + cz_mirror_sq))) ;
      }

  return ((rho * potential) / (FOUR_PI * electrode->precompute_params.permittivity)) ;
  }

static double get_area (QCADElectrode *electrode)
  {
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (electrode) ;

  return rc_electrode->cxWorld * rc_electrode->cyWorld * 1e-18 ;
  }
///////////////////////////////////////////////////////////////////////////////

static void precompute (QCADElectrode *electrode)
  {
  int Nix, Nix1 ;
  WorldPoint ptSrcLine, ptDstLine ;
  double factor1, factor2, dstx_minus_srcx, dsty_minus_srcy ;
  double pt1x_minus_pt0x, pt1y_minus_pt0y, pt3x_minus_pt2x, pt3y_minus_pt2y ;
  double reciprocal_of_x_divisions, reciprocal_of_y_divisions ;
  QCADRectangleElectrode *rc_electrode = QCAD_RECTANGLE_ELECTRODE (electrode) ;
  QCADDesignObject *obj = QCAD_DESIGN_OBJECT (electrode) ;
  double 
    kose =  cos (rc_electrode->angle),
    msin = -sin (rc_electrode->angle),
    sine =  sin (rc_electrode->angle),
    half_cx = rc_electrode->cxWorld / 2.0,
    half_cy = rc_electrode->cyWorld / 2.0,
    xMin, yMin, xMax, yMax ;
  WorldPoint pt[4] = {{0,0},{0,0},{0,0},{0,0}}, ptCenter = {0,0} ;

  // Call parent precompute function
  QCAD_ELECTRODE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_ELECTRODE)))->precompute (electrode) ;

  ptCenter.xWorld = obj->bounding_box.xWorld + obj->bounding_box.cxWorld / 2.0 ;
  ptCenter.yWorld = obj->bounding_box.yWorld + obj->bounding_box.cyWorld / 2.0 ;

  // Create corner points
  pt[0].xWorld = ptCenter.xWorld - half_cx ;
  pt[0].yWorld = ptCenter.yWorld - half_cy ;
  pt[1].xWorld = ptCenter.xWorld + half_cx ;
  pt[1].yWorld = ptCenter.yWorld - half_cy ;
  pt[2].xWorld = ptCenter.xWorld + half_cx ;
  pt[2].yWorld = ptCenter.yWorld + half_cy ;
  pt[3].xWorld = ptCenter.xWorld - half_cx ;
  pt[3].yWorld = ptCenter.yWorld + half_cy ;

  if (0 != rc_electrode->angle)
    {
    // rotate corner points
    rc_electrode->precompute_params.pt[0].xWorld = kose * pt[0].xWorld + sine * pt[0].yWorld ;
    rc_electrode->precompute_params.pt[0].yWorld = msin * pt[0].xWorld + kose * pt[0].yWorld ;
    rc_electrode->precompute_params.pt[1].xWorld = kose * pt[1].xWorld + sine * pt[1].yWorld ;
    rc_electrode->precompute_params.pt[1].yWorld = msin * pt[1].xWorld + kose * pt[1].yWorld ;
    rc_electrode->precompute_params.pt[2].xWorld = kose * pt[2].xWorld + sine * pt[2].yWorld ;
    rc_electrode->precompute_params.pt[2].yWorld = msin * pt[2].xWorld + kose * pt[2].yWorld ;
    rc_electrode->precompute_params.pt[3].xWorld = kose * pt[3].xWorld + sine * pt[3].yWorld ;
    rc_electrode->precompute_params.pt[3].yWorld = msin * pt[3].xWorld + kose * pt[3].yWorld ;

    // Find binding box
    xMin = MIN (rc_electrode->precompute_params.pt[0].xWorld, MIN (rc_electrode->precompute_params.pt[1].xWorld, MIN (rc_electrode->precompute_params.pt[2].xWorld, rc_electrode->precompute_params.pt[3].xWorld))) ;
    xMax = MAX (rc_electrode->precompute_params.pt[0].xWorld, MAX (rc_electrode->precompute_params.pt[1].xWorld, MAX (rc_electrode->precompute_params.pt[2].xWorld, rc_electrode->precompute_params.pt[3].xWorld))) ;
    yMin = MIN (rc_electrode->precompute_params.pt[0].yWorld, MIN (rc_electrode->precompute_params.pt[1].yWorld, MIN (rc_electrode->precompute_params.pt[2].yWorld, rc_electrode->precompute_params.pt[3].yWorld))) ;
    yMax = MAX (rc_electrode->precompute_params.pt[0].yWorld, MAX (rc_electrode->precompute_params.pt[1].yWorld, MAX (rc_electrode->precompute_params.pt[2].yWorld, rc_electrode->precompute_params.pt[3].yWorld))) ;

    obj->bounding_box.xWorld = xMin ;
    obj->bounding_box.yWorld = yMin ;
    obj->bounding_box.cxWorld = xMax - xMin ;
    obj->bounding_box.cyWorld = yMax - yMin ;
    obj->x = obj->bounding_box.xWorld + obj->bounding_box.cxWorld / 2.0 ;
    obj->y = obj->bounding_box.yWorld + obj->bounding_box.cyWorld / 2.0 ;

    // move points back
    rc_electrode->precompute_params.pt[0].xWorld += ptCenter.xWorld - obj->x ;
    rc_electrode->precompute_params.pt[0].yWorld += ptCenter.yWorld - obj->y ;
    rc_electrode->precompute_params.pt[1].xWorld += ptCenter.xWorld - obj->x ;
    rc_electrode->precompute_params.pt[1].yWorld += ptCenter.yWorld - obj->y ;
    rc_electrode->precompute_params.pt[2].xWorld += ptCenter.xWorld - obj->x ;
    rc_electrode->precompute_params.pt[2].yWorld += ptCenter.yWorld - obj->y ;
    rc_electrode->precompute_params.pt[3].xWorld += ptCenter.xWorld - obj->x ;
    rc_electrode->precompute_params.pt[3].yWorld += ptCenter.yWorld - obj->y ;
    obj->bounding_box.xWorld += ptCenter.xWorld - obj->x ;
    obj->bounding_box.yWorld += ptCenter.yWorld - obj->y ;
    obj->x = ptCenter.xWorld ;
    obj->y = ptCenter.yWorld ;
    }
  else
    {
    rc_electrode->precompute_params.pt[0] = pt[0] ;
    rc_electrode->precompute_params.pt[1] = pt[1] ;
    rc_electrode->precompute_params.pt[2] = pt[2] ;
    rc_electrode->precompute_params.pt[3] = pt[3] ;
    obj->bounding_box.xWorld = pt[0].xWorld ;
    obj->bounding_box.yWorld = pt[0].yWorld ;
    obj->bounding_box.cxWorld = rc_electrode->cxWorld ;
    obj->bounding_box.cyWorld = rc_electrode->cyWorld ;
    obj->x = obj->bounding_box.xWorld + obj->bounding_box.cxWorld / 2.0 ;
    obj->y = obj->bounding_box.yWorld + obj->bounding_box.cyWorld / 2.0 ;
    }

  if (NULL != rc_electrode->precompute_params.pts)
    exp_array_free (rc_electrode->precompute_params.pts) ;

  rc_electrode->precompute_params.pts = exp_array_new (sizeof (WorldPoint), 2) ;

  for (Nix = 0 ; Nix < rc_electrode->n_y_divisions ; Nix++)
    exp_array_insert_vals (rc_electrode->precompute_params.pts, NULL, rc_electrode->n_x_divisions, 2, -1, 0) ;

  // This is a faaar better place to multiply by 1e9 than get_potential
  rc_electrode->precompute_params.rho_factor = 1e9 * QCAD_ELECTRODE (rc_electrode)->precompute_params.capacitance / (rc_electrode->n_x_divisions * rc_electrode->n_y_divisions) ;
  pt1x_minus_pt0x = rc_electrode->precompute_params.pt[1].xWorld - rc_electrode->precompute_params.pt[0].xWorld ;
  pt1y_minus_pt0y = rc_electrode->precompute_params.pt[1].yWorld - rc_electrode->precompute_params.pt[0].yWorld ;
  pt3x_minus_pt2x = rc_electrode->precompute_params.pt[3].xWorld - rc_electrode->precompute_params.pt[2].xWorld ;
  pt3y_minus_pt2y = rc_electrode->precompute_params.pt[3].yWorld - rc_electrode->precompute_params.pt[2].yWorld ;
  reciprocal_of_x_divisions = 1.0 / rc_electrode->n_x_divisions ;
  reciprocal_of_y_divisions = 1.0 / rc_electrode->n_y_divisions ;
  
  for (Nix = 0 ; Nix < rc_electrode->n_x_divisions ; Nix++)
    {
    factor1 = reciprocal_of_x_divisions * (Nix + 0.5) ;
    factor2 = reciprocal_of_x_divisions * ((rc_electrode->n_x_divisions - Nix) - 0.5) ;

    ptSrcLine.xWorld = rc_electrode->precompute_params.pt[0].xWorld + pt1x_minus_pt0x * factor1 ;
    ptSrcLine.yWorld = rc_electrode->precompute_params.pt[0].yWorld + pt1y_minus_pt0y * factor1 ;
    ptDstLine.xWorld = rc_electrode->precompute_params.pt[2].xWorld + pt3x_minus_pt2x * factor2 ;
    ptDstLine.yWorld = rc_electrode->precompute_params.pt[2].yWorld + pt3y_minus_pt2y * factor2 ;

    dstx_minus_srcx = ptDstLine.xWorld - ptSrcLine.xWorld ;
    dsty_minus_srcy = ptDstLine.yWorld - ptSrcLine.yWorld ;
    for (Nix1 = 0 ; Nix1 < rc_electrode->n_y_divisions ; Nix1++)
      {
      exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).xWorld = 
        ptSrcLine.xWorld + dstx_minus_srcx * reciprocal_of_y_divisions * (Nix1 + 0.5) ;
      exp_array_index_2d (rc_electrode->precompute_params.pts, WorldPoint, Nix1, Nix).yWorld = 
        ptSrcLine.yWorld + dsty_minus_srcy * reciprocal_of_y_divisions * (Nix1 + 0.5) ;
      }
    }
  }
