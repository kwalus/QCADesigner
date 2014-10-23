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
// The substrate. This object serves no purpose (so     //
// far) other than to provide a guide for placing ob-   //
// jects.                                               //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glib-object.h>
#include "../gdk_structs.h"
#include "../intl.h"
#include "../custom_widgets.h"
#include "QCADSubstrate.h"
#include "object_helpers.h"
#include "../fileio_helpers.h"
#include "QCADCell.h"

/**
 * SECTION:QCADSubstrate
 * @short_description: QCA substrate.
 *
 * The substrate serves as the object to snap other design objects to.
 */

static void qcad_substrate_class_init (QCADStretchyObjectClass *klass, gpointer data) ;
static void qcad_substrate_instance_init (QCADStretchyObject *object, gpointer data) ;
static void qcad_substrate_instance_finalize (GObject *object) ;
static void qcad_substrate_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void qcad_substrate_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) ;

static void copy (QCADObject *src, QCADObject *dst) ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static QCADObject *class_get_default_object () ;

enum
  {
  QCAD_SUBSTRATE_PROPERTY_FIRST=1,

  QCAD_SUBSTRATE_PROPERTY_SPACING,

  QCAD_SUBSTRATE_PROPERTY_LAST
  } ;

GType qcad_substrate_get_type ()
  {
  static GType qcad_substrate_type = 0 ;

  if (!qcad_substrate_type)
    {
    static const GTypeInfo qcad_substrate_info =
      {
      sizeof (QCADSubstrateClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_substrate_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADSubstrate),
      0,
      (GInstanceInitFunc)qcad_substrate_instance_init
      } ;

    if ((qcad_substrate_type = g_type_register_static (QCAD_TYPE_STRETCHY_OBJECT, QCAD_TYPE_STRING_SUBSTRATE, &qcad_substrate_info, 0)))
      g_type_class_ref (qcad_substrate_type) ;
    }
  return qcad_substrate_type ;
  }

static void qcad_substrate_class_init (QCADStretchyObjectClass *klass, gpointer data)
  {
#ifdef PROPERTY_UIS
  // Gotta be static so the strings don't die
  static QCADPropertyUIProperty properties[] =
    {
    {NULL,      "title", {0, }},
    {"spacing", "units", {0, }}
    } ;

  // substrate.title = "QCA Substrate"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), "Substrate") ;
  // substrate.spacing.units = "nm"
  g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;

  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
#endif /* def PROPERTY_UIS */

  G_OBJECT_CLASS (klass)->finalize     = qcad_substrate_instance_finalize ;
  G_OBJECT_CLASS (klass)->get_property = qcad_substrate_get_property ;
  G_OBJECT_CLASS (klass)->set_property = qcad_substrate_set_property ;
  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = class_get_default_object ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance = PostScript_instance ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                = draw ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize           = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize         = unserialize ;
#endif /* def STDIO_FILEIO */

  /**
   * QCADSubstrate:spacing:
   *
   * The substrate's grid spacing in world coordinates.
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_SUBSTRATE_PROPERTY_SPACING,
    g_param_spec_double ("spacing", _("Grid Spacing"), _("Grid Spacing [nm]"),
      1.0, G_MAXDOUBLE, 10.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_substrate_instance_init (QCADStretchyObject *object, gpointer data)
  {QCAD_SUBSTRATE (object)->grid_spacing = 1.0000 /* nm */ ;}

static void qcad_substrate_instance_finalize (GObject *object)
  {G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_SUBSTRATE)))->finalize (object) ;}

static QCADObject *class_get_default_object ()
  {return g_object_new (QCAD_TYPE_SUBSTRATE, NULL) ;}

static void qcad_substrate_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADSubstrate *subs = QCAD_SUBSTRATE (object) ;

  switch (property_id)
    {
    case QCAD_SUBSTRATE_PROPERTY_SPACING:
      {
      double new_spacing = g_value_get_double (value) ;

      if (new_spacing == subs->grid_spacing) break ;
      subs->grid_spacing = new_spacing ;
      g_object_notify (object, "spacing") ;
      break ;
      }
    }
  }

static void qcad_substrate_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADSubstrate *subs = QCAD_SUBSTRATE (object) ;

  switch (property_id)
    {
    case QCAD_SUBSTRATE_PROPERTY_SPACING:
      g_value_set_double (value, subs->grid_spacing) ;
      break ;
    }
  }

///////////////////////////////////////////////////////////////////////////////

/**
 * qcad_substrate_new:
 * @x: World x-coordinate of substrate's top left corner.
 * @y: World y-coordinate of substrate's top left corner.
 * @cx: Substrate width in world coordinates.
 * @cy: Substrate height in world coordinates.
 * @grid_spacing: Grid spacing to use when objects are snapped with respect to this substrate.
 *
 * Creates a new substrate of the given world size and grid spacing originating at the given world coordinates.
 *
 * Returns: A new substrate object.
 */
QCADDesignObject *qcad_substrate_new (double x, double y, double cx, double cy, double grid_spacing)
  {
  QCADDesignObject *obj = QCAD_DESIGN_OBJECT (g_object_new (QCAD_TYPE_SUBSTRATE, NULL)) ;
  obj->x = (x + cx) / 2 ;
  obj->y = (y + cy) / 2 ;
  obj->bounding_box.xWorld = x ;
  obj->bounding_box.yWorld = y ;
  obj->bounding_box.cxWorld = cx ;
  obj->bounding_box.cyWorld = cy ;
  QCAD_SUBSTRATE (obj)->grid_spacing = grid_spacing ;

  return obj ;
  }

/**
 * qcad_substrate_snap_point:
 * @subs: Reference substrate.
 * @px: Pointer containing x-coordinate to manipulate.
 * @py: Pointer containing y-coordinate to manipulate.
 *
 * Retrieves the world x and y coordinates of a point from @px and @py, respectively, and overwrites them with 
 * the coordinates of @subs' nearest grid point.
 */
void qcad_substrate_snap_point (QCADSubstrate *subs, double *px, double *py)
  {
  if (!QCAD_IS_SUBSTRATE (subs)) return ;

  (*px) = QCAD_DESIGN_OBJECT (subs)->bounding_box.xWorld +
    NINT (((*px) - QCAD_DESIGN_OBJECT (subs)->bounding_box.xWorld) / subs->grid_spacing) * subs->grid_spacing ;
  (*py) = QCAD_DESIGN_OBJECT (subs)->bounding_box.yWorld +
    NINT (((*py) - QCAD_DESIGN_OBJECT (subs)->bounding_box.yWorld) / subs->grid_spacing) * subs->grid_spacing ;
  }

///////////////////////////////////////////////////////////////////////////////

static void copy (QCADObject *src, QCADObject *dst)
  {
  QCAD_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_SUBSTRATE)))->copy (src, dst) ;
  QCAD_SUBSTRATE (dst)->grid_spacing = QCAD_SUBSTRATE (src)->grid_spacing ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  char doubles[4][G_ASCII_DTOSTR_BUF_SIZE] = {""} ;
  return
    g_strdup_printf ("%s nmx %s nmy %s nm %s nm QCADSubstrate",
      g_ascii_dtostr (doubles[0], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld), 
      g_ascii_dtostr (doubles[1], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld), 
      g_ascii_dtostr (doubles[2], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld), 
      g_ascii_dtostr (doubles[3], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld)) ;
  }

static const char *PostScript_preamble ()
  {
  return
    "% x y cx cy QCADSubstrate\n"
    "/QCADSubstrate {\n"
    "  gsave\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  linewidth epsilon gt\n"
    "    {\n"
    "    newpath\n"
    "    x y moveto\n"
    "    x cx add y lineto\n"
    "    x cx add y cy sub lineto\n"
    "    x y cy sub lineto\n"
    "    closepath stroke\n"
    "    }\n"
    "  if\n"
    "\n"
    "  grestore\n"
    "} def\n" ;
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  GdkGC *gc = gdk_gc_new (dst) ;
  GdkPoint *pts = NULL ;
  GdkRectangle rc, rcOffset, rcRealBoundingBox ;
  QCADSubstrate *subs = QCAD_SUBSTRATE (obj) ;
  double Nix1, Nix2, dot_spacing = subs->grid_spacing,
    xTopView, yTopView, xBotView, yBotView, magnitude, actual_spacing, divisor = 1.0 ;
  int idx = 0,
    xIdxBeg = -1, yIdxBeg = -1,
    xIdxEnd = -1, yIdxEnd = -1 ;
  int icPts = 0, real_dot_spacing = -1 ;

  // Copy the real coords both into rc and into rcRealBoundingBox
  memcpy (&rcRealBoundingBox, world_to_real_rect (&(obj->bounding_box), &rc), sizeof (GdkRectangle)) ; 
//  if (!gdk_rectangle_intersect (rcClip, &rc, &rc)) return ;

  magnitude = pow (10, ceil (log10 (actual_spacing = real_to_world_cx (10)))) ;
  divisor =
    (ABS (magnitude / 2 - actual_spacing) < ABS (magnitude - actual_spacing)) ?
    (ABS (magnitude / 5 - actual_spacing) < ABS (magnitude / 2 - actual_spacing)) ? 5.0 : 2.0 : 1.0 ;

  real_dot_spacing = world_to_real_cx (dot_spacing = magnitude / divisor) ; ;

  if (!(is_real_rect_visible (&rc))) return ;

  rcOffset.x = rc.x + real_dot_spacing ;
  rcOffset.y = rc.y + real_dot_spacing ;
  rcOffset.width = rc.width - real_dot_spacing ;
  rcOffset.height = rc.height - real_dot_spacing ;

  if (is_real_rect_visible (&rcOffset))
    {
    get_world_viewport (&xTopView, &yTopView, &xBotView, &yBotView) ;
    xIdxBeg = MAX (0, ceil ((xTopView - obj->bounding_box.xWorld - dot_spacing) / dot_spacing) - 1) ;
    yIdxBeg = MAX (0, ceil ((yTopView - obj->bounding_box.yWorld - dot_spacing) / dot_spacing) - 1) ;
    xIdxEnd = floor ((obj->bounding_box.cxWorld - dot_spacing) / dot_spacing) -
      MAX (0, ceil ((obj->bounding_box.cxWorld + obj->bounding_box.xWorld - xBotView - dot_spacing) / dot_spacing)) ;
    yIdxEnd = floor ((obj->bounding_box.cyWorld - dot_spacing) / dot_spacing) -
      MAX (0, ceil ((obj->bounding_box.cyWorld + obj->bounding_box.yWorld - yBotView - dot_spacing) / dot_spacing)) ;

    pts = g_malloc ((icPts = (xIdxEnd - xIdxBeg + 1) * (yIdxEnd - yIdxBeg + 1)) * sizeof (GdkPoint)) ;

    for (Nix1 = yIdxBeg ; Nix1 <= yIdxEnd ; Nix1++)
      for (Nix2 = xIdxBeg ; Nix2 <= xIdxEnd ; Nix2++)
        {
        if (idx < icPts)
          {
          pts[idx].x = world_to_real_x (obj->bounding_box.xWorld + (Nix2 + 1) * dot_spacing) ;
          pts[idx].y = world_to_real_y (obj->bounding_box.yWorld + (Nix1 + 1) * dot_spacing) ;
          }
        idx++ ;
        }
    }

  gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_draw_rectangle (dst, gc, FALSE, rcRealBoundingBox.x, rcRealBoundingBox.y, rcRealBoundingBox.width, rcRealBoundingBox.height) ;

  if (!(0 == icPts || NULL == pts))
    {
    gdk_draw_points (dst, gc, pts, icPts) ;
    g_free (pts) ;
    }

  g_object_unref (gc) ;
  }
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp)
{
  char szDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;
  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_SUBSTRATE);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_SUBSTRATE)))->serialize (obj, fp) ;

  // output variables
  fprintf(fp, "grid_spacing=%s\n", g_ascii_dtostr (szDouble, G_ASCII_DTOSTR_BUF_SIZE, QCAD_SUBSTRATE(obj)->grid_spacing));

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_SUBSTRATE);
}

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADSubstrate *subs = QCAD_SUBSTRATE (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_SUBSTRATE "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_SUBSTRATE "]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_SUBSTRATE)))->unserialize (obj, fp)))
          bStopReading = TRUE ;
        }
      else
      if (!strcmp (pszLine, "grid_spacing"))
        subs->grid_spacing = g_ascii_strtod (pszValue, NULL) ;
      }

    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  // parent instantiated OK
  if (!bStopReading)
    // grid spacing has to be between 1 and 100 nanometers
    subs->grid_spacing = CLAMP (subs->grid_spacing, 1, 100) ;

  return bParentInit ;
  }
#endif /* STDIO_FILEIO */
