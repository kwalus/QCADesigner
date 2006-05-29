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
// QCADLabel: A simple text label to accompany cells    //
// or to stand on its own in a drawing layer.           //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdarg.h>
#include <string.h>
#include <glib-object.h>
#include "../gdk_structs.h"
#include "QCADLabel.h"
#include "object_helpers.h"
#include "../intl.h"
#include "../fileio_helpers.h"
#include "../custom_widgets.h"
#include "../global_consts.h"
#ifdef GTK_GUI
  #include "../exp_pixmap.h"
#endif /* def GTK_GUI */

#define XTOP_LABEL_OFFSET 2
#define YTOP_LABEL_OFFSET 2

#define CYFONT 10 /* nanometers */

static void qcad_label_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_label_instance_init (GObject *object, gpointer data) ;

static void finalize (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void copy (QCADObject *src, QCADObject *dst) ;
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
#ifdef DESIGNER
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */
#endif /* def DESIGNER */
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static QCADObject *class_get_default_object () ;

//extern GdkFont *font ;
extern GdkColor clrBlue ;

enum
  {
  QCAD_LABEL_PROPERTY_FIRST=1,

  QCAD_LABEL_PROPERTY_TEXT,

  QCAD_LABEL_PROPERTY_LAST
  } ;

GType qcad_label_get_type ()
  {
  static GType qcad_label_type = 0 ;

  if (!qcad_label_type)
    {
    static const GTypeInfo qcad_label_info =
      {
      sizeof (QCADLabelClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_label_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADLabel),
      0,
      (GInstanceInitFunc)qcad_label_instance_init
      } ;

    if ((qcad_label_type = g_type_register_static (QCAD_TYPE_STRETCHY_OBJECT, QCAD_TYPE_STRING_LABEL, &qcad_label_info, 0)))
      g_type_class_ref (qcad_label_type) ;
    }
  return qcad_label_type ;
  }

static void qcad_label_class_init (GObjectClass *klass, gpointer data)
  {
#ifdef PROPERTY_UIS
  static QCADPropertyUIProperty properties[] =
    {
    {NULL, "title", {0, }}
    } ;

  // cell.title = "QCA Cell"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), _("QCA Label")) ;

  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
#endif /* def PROPERTY_UIS */

  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = class_get_default_object ;
#ifdef GTK_GUI
  if (0 == clrBlue.pixel)
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clrBlue, FALSE, TRUE) ;
#ifdef DESIGNER
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                = draw ;
#endif /* def DESIGNER */
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize           = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize         = unserialize ;
#endif /* def STDIO_FILEIO */
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance = PostScript_instance ;

  g_object_class_install_property (klass, QCAD_LABEL_PROPERTY_TEXT,
    g_param_spec_string ("text", _("Text"), _("Label text"), 
      "", G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_label_instance_init (GObject *object, gpointer data)
  {
  QCAD_LABEL (object)->psz = g_strdup (_("Label")) ;
  QCAD_LABEL (object)->bShrinkWrap = TRUE ;
#ifdef DESIGNER
  QCAD_LABEL (object)->bNeedsEPMDraw = TRUE ;
#ifdef GTK_GUI
  QCAD_LABEL (object)->epm = NULL ;
#endif /* def DESIGNER */
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT (object)->x =
  QCAD_DESIGN_OBJECT (object)->y =
  QCAD_DESIGN_OBJECT (object)->bounding_box.xWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.yWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.cxWorld =
  QCAD_DESIGN_OBJECT (object)->bounding_box.cyWorld = 0.0 ;
  memcpy (&(QCAD_DESIGN_OBJECT (object)->clr), &clrBlue, sizeof (GdkColor)) ;
  qcad_label_shrinkwrap (QCAD_LABEL (object)) ;
  }

static void finalize (GObject *object)
  {
  g_free (QCAD_LABEL (object)->psz) ;
#ifdef DESIGNER
#ifdef GTK_GUI
  exp_pixmap_free (QCAD_LABEL (object)->epm) ;
#endif /* def GTK_GUI */
#endif /* def DESIGNER */
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->finalize (object) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_LABEL_PROPERTY_TEXT:
      qcad_label_set_text (QCAD_LABEL (object), (char *)g_value_get_string (value)) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_LABEL_PROPERTY_TEXT:
      g_value_set_string (value, QCAD_LABEL (object)->psz) ;
      break ;
    }
  }

///////////////////////////////////////////////////////////////////////////////

static QCADObject *class_get_default_object ()
  {return g_object_new (QCAD_TYPE_LABEL, NULL) ;}

static void copy (QCADObject *src, QCADObject *dst)
  {
  QCAD_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->copy (src, dst) ;
  QCAD_LABEL (dst)->psz = g_strdup (QCAD_LABEL (src)->psz) ;
#ifdef DESIGNER
  QCAD_LABEL (dst)->bNeedsEPMDraw = TRUE ;
#endif /* def DESUGNER */
  }

#ifdef DESIGNER
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  GdkGC *gc = NULL ;
  char *pszFont = NULL ;
  GdkRectangle rc = {0} ;
  GdkRectangle rcDst = {0} ;
  GdkRectangle rcDraw = {0} ;
  QCADLabel *label = NULL ;

  if ((label = QCAD_LABEL (obj))->bShrinkWrap)
    qcad_label_shrinkwrap (label) ;

  world_to_real_rect (&(obj->bounding_box), &rc) ;
  gdk_drawable_get_size (dst, &(rcDst.width), &(rcDst.height)) ;
  if (!gdk_rectangle_intersect (&rc, &rcDst, &rcDraw)) return ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_clip_rectangle (gc, &rc) ;
  if (GDK_COPY == rop)
    {
    gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
    draw_string (dst, gc, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)),
      rc.x + XTOP_LABEL_OFFSET, rc.y + YTOP_LABEL_OFFSET, label->psz) ;
    g_free (pszFont) ;
    }
  else
    {
    if (label->bNeedsEPMDraw)
      {
      GdkGC *gcEPM = NULL ;

      label->bNeedsEPMDraw = FALSE ;
      label->epm = exp_pixmap_cond_new (label->epm, dst, rcDraw.width, rcDraw.height, -1) ;
      exp_pixmap_clean (label->epm) ;
      gcEPM = gdk_gc_new (label->epm->pixmap) ;
      gdk_gc_set_foreground (gcEPM, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
      draw_string (label->epm->pixmap, gcEPM, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)),
        XTOP_LABEL_OFFSET, YTOP_LABEL_OFFSET, label->psz) ;
      g_free (pszFont) ;
      g_object_unref (gcEPM) ;
      }
    gdk_draw_drawable (dst, gc, label->epm->pixmap, 0, 0, rc.x, rc.y, rcDraw.width, rcDraw.height) ;
    }

  if (obj->bSelected)
    gdk_draw_rectangle (dst, gc, FALSE, rc.x, rc.y, rc.width - 1, rc.height - 1) ;
  gdk_gc_unref (gc) ;
  }
#endif /* def GTK_GUI */
#endif /* def DESIGNER */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_LABEL);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LABEL)))->serialize (obj, fp) ;

  // output variables
  fprintf (fp, "psz=%s\n", QCAD_LABEL (obj)->psz) ;

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_LABEL);
  }

static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_LABEL "]", NULL)) return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE)))
      break ;

    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_LABEL "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_STRETCHY_OBJECT)))->unserialize (obj, fp)))
          bStopReading = TRUE ;
        }

      else
      if (!strcmp (pszLine, "psz"))
        {
        QCAD_LABEL (obj)->psz = g_strdup (pszValue) ;
#ifdef DESIGNER
        QCAD_LABEL (obj)->bNeedsEPMDraw = TRUE ;
#endif /* def DESIGNER */
        }
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  if (bParentInit)
    qcad_label_shrinkwrap (QCAD_LABEL (obj)) ;

  return bParentInit ;
  }
#endif /* STDIO_FILEIO */

static const char *PostScript_preamble ()
  {
  return
    "% x y cx cy r g b (label) QCADLabel\n"
    "/QCADLabel\n"
    "  {\n"
    "  gsave\n"
    "  /label exch def\n"
    "  /b exch def\n"
    "  /g exch def\n"
    "  /r exch def\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  newpath\n"
    "  x y moveto\n"
    "  x cx add y lineto\n"
    "  x cx add y cy sub lineto\n"
    "  x y cy sub lineto\n"
    "  closepath clip\n"
    "\n"
    "  r g b setrgbcolor\n"
    "\n"
//    "  linewidth epsilon gt\n"
//    "    {\n"
//    "    newpath\n"
//    "    x y moveto\n"
//    "    x cx add y lineto\n"
//    "    x cx add y cy sub lineto\n"
//    "    x y cy sub lineto\n"
//    "    closepath stroke\n"
//    "    }\n"
//    "  if\n"
    "\n"
    "  x y moveto\n"
    "  (" PS_FONT ") labelfontsize label 0 txt\n"
    "  grestore\n"
    "  } def\n" ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  QCADLabel *lbl = QCAD_LABEL (obj) ;
  GdkColor clr = {0} ;
  char doubles[7][G_ASCII_DTOSTR_BUF_SIZE] = {""} ;
  double
    r = ((double)(obj->clr.red)) / 65535.0,
    g = ((double)(obj->clr.green)) / 65535.0,
    b = ((double)(obj->clr.blue)) / 65535.0 ;

  if (!bColour)
    {
    memcpy (&clr, &(obj->clr), sizeof (GdkColor)) ;
    RGBToHSL (&clr) ;
    r =
    g =
    b = ((double)(clr.blue)) / 65536.0 ; // .blue has become the luminance
    }

  return g_strdup_printf ("%s nmx %s nmy %s nm %s nm %s %s %s (%s) QCADLabel",
    g_ascii_dtostr (doubles[0], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld), 
    g_ascii_dtostr (doubles[1], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld), 
    g_ascii_dtostr (doubles[2], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld), 
    g_ascii_dtostr (doubles[3], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld), 
    g_ascii_dtostr (doubles[4], G_ASCII_DTOSTR_BUF_SIZE, r), 
    g_ascii_dtostr (doubles[5], G_ASCII_DTOSTR_BUF_SIZE, g), 
    g_ascii_dtostr (doubles[6], G_ASCII_DTOSTR_BUF_SIZE, b), lbl->psz) ;
  }

QCADLabel *qcad_label_new (char *psz, ...)
  {
  QCADLabel *lbl = g_object_new (QCAD_TYPE_LABEL, NULL) ;
  va_list va ;

  va_start (va, psz) ;
  qcad_label_vset_text (lbl, psz, va) ;
  va_end (va) ;
  return lbl ;
  }

void qcad_label_set_text (QCADLabel *label, char *psz, ...)
  {
  va_list va ;

  va_start (va, psz) ;
  qcad_label_vset_text (label, psz, va) ;
  va_end (va) ;

  g_object_notify (G_OBJECT (label), "text") ;
  }

void qcad_label_vset_text (QCADLabel *label, char *psz, va_list va)
  {
  if (NULL != label->psz)
    g_free (label->psz) ;
  label->psz = g_strdup_vprintf (psz, va) ;
#ifdef DESIGNER
  label->bNeedsEPMDraw = TRUE ;
#endif /* def DESIGNER */
  if (label->bShrinkWrap)
    qcad_label_shrinkwrap (label) ;
  }

void qcad_label_shrinkwrap (QCADLabel *label)
  {
  // This function has no effect if there's no Gtk, because there's no way to measure the (cx,cy) of a
  // string without Gdk
  #ifdef GTK_GUI
  int cx, cy ;
  char *pszFont ;

  get_string_dimensions (label->psz, pszFont = g_strdup_printf ("Courier %d", world_to_real_cy (CYFONT)), &cx, &cy) ;
  g_free (pszFont) ;

  QCAD_DESIGN_OBJECT (label)->bounding_box.cxWorld = real_to_world_cx (cx + 2.0 * XTOP_LABEL_OFFSET) ;
  QCAD_DESIGN_OBJECT (label)->bounding_box.cyWorld = real_to_world_cy (cy + 2.0 * YTOP_LABEL_OFFSET) ;
  QCAD_DESIGN_OBJECT (label)->x = QCAD_DESIGN_OBJECT (label)->bounding_box.cxWorld / 2.0 + QCAD_DESIGN_OBJECT (label)->bounding_box.xWorld ;
  QCAD_DESIGN_OBJECT (label)->y = QCAD_DESIGN_OBJECT (label)->bounding_box.cyWorld / 2.0 + QCAD_DESIGN_OBJECT (label)->bounding_box.yWorld ;
  #endif /* def GTK_GUI */
  }
