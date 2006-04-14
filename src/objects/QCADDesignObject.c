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
// QCADDesignObject: The base class for all the design  //
// objects. This class provides printing, extents cal-  //
// culation, drawing, moving, pretty much everything    //
// for a selectable, printable, design object.          //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif
#include "QCADDesignObject.h"
#ifdef UNDO_REDO
#include "QCADUndoEntry.h"
#endif /* def UNDO_REDO */
#include "object_helpers.h"
#include "../fileio_helpers.h"
#include "../exp_array.h"
#include "../intl.h"
#include "QCADRectangleElectrode.h"

/**
 * SECTION:QCADDesignObject
 * @short_description: Common base class for all objects appearing in a design
 *
 * #QCADDesignObject is the common base class for all design objects. It defines the necessary virtual 
 * functions for serialization, bounds calculation, selection, drawing, and transformation.
 *
 */

#define DBG_QCADDO_FIN(s)

static void qcad_design_object_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_design_object_instance_init (GObject *object, gpointer data) ;
static void qcad_design_object_instance_finalize (GObject *object) ;

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean motion_notify (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean button_released (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
#endif /* def GTK_GUI */

static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal) ;
static gboolean select_test (QCADDesignObject *obj, WorldRectangle *rc, QCADSelectionMethod method) ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rcWorld) ;
static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected) ;
static void move (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
static void copy (QCADObject *objSrc, QCADObject *objDst) ;
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static GList *add_unique_types (QCADDesignObject *obj, GList *lst) ;
static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22) ;

#ifdef GTK_GUI
static GdkColormap *clrmap = NULL ;
#endif /* def GTK_GUI */

enum
  {
  QCAD_DESIGN_OBJECT_SELECTED_SIGNAL,
  QCAD_DESIGN_OBJECT_LAST_SIGNAL
  } ;

static guint qcad_design_object_signals[QCAD_DESIGN_OBJECT_LAST_SIGNAL] = {0} ;

GType qcad_design_object_get_type ()
  {
  static GType qcad_design_object_type = 0 ;

  if (!qcad_design_object_type)
    {
    static const GTypeInfo qcad_design_object_info =
      {
      sizeof (QCADDesignObjectClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_design_object_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADDesignObject),
      0,
      (GInstanceInitFunc)qcad_design_object_instance_init
      } ;

    if ((qcad_design_object_type = g_type_register_static (QCAD_TYPE_OBJECT, QCAD_TYPE_STRING_DESIGN_OBJECT, &qcad_design_object_info, 0)))
      g_type_class_ref (qcad_design_object_type) ;
    }
  return qcad_design_object_type ;
  }

static void qcad_design_object_class_init (GObjectClass *klass, gpointer data)
  {
  QCAD_OBJECT_CLASS (klass)->copy = copy ;
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize = unserialize ;
#endif /* def STDIO_FILEIO */
  QCAD_DESIGN_OBJECT_CLASS (klass)->get_bounds_box = get_bounds_box ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->set_selected = set_selected ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->move = move ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->transform = transform ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->hit_test = hit_test ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->select_test = select_test ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance = PostScript_instance ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->add_unique_types = add_unique_types ;

  QCAD_DESIGN_OBJECT_CLASS (klass)->clrSelected.pixel = 0 ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrSelected.red   = 0xffff ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrSelected.green = 0x0000 ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrSelected.blue  = 0x0000 ;

  QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault.pixel = 0 ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault.red   = 0xffff ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault.green = 0xffff ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault.blue  = 0xffff ;

  G_OBJECT_CLASS (klass)->finalize     = qcad_design_object_instance_finalize ;

#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                  = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed     = (GCallback)button_pressed ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.motion_notify      = (GCallback)motion_notify ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_released    = (GCallback)button_released ;

  clrmap = gdk_colormap_get_system () ;

  gdk_colormap_alloc_color (clrmap, &(QCAD_DESIGN_OBJECT_CLASS (klass)->clrSelected), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault), FALSE, TRUE) ;
#endif /* def GTK_GUI */

  qcad_design_object_signals[QCAD_DESIGN_OBJECT_SELECTED_SIGNAL] =
    g_signal_new ("selected", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADDesignObjectClass, selected), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0) ;
  }

static void qcad_design_object_instance_init (GObject *object, gpointer data)
  {
  QCAD_DESIGN_OBJECT (object)->bSelected = FALSE ;
  QCAD_DESIGN_OBJECT (object)->x =
  QCAD_DESIGN_OBJECT (object)->y = -999.0 ;

  QCAD_DESIGN_OBJECT (object)->clr.pixel = QCAD_DESIGN_OBJECT_GET_CLASS (object)->clrDefault.pixel ;
  QCAD_DESIGN_OBJECT (object)->clr.red   = QCAD_DESIGN_OBJECT_GET_CLASS (object)->clrDefault.red ;
  QCAD_DESIGN_OBJECT (object)->clr.green = QCAD_DESIGN_OBJECT_GET_CLASS (object)->clrDefault.green ;
  QCAD_DESIGN_OBJECT (object)->clr.blue  = QCAD_DESIGN_OBJECT_GET_CLASS (object)->clrDefault.blue ;
  }

static void qcad_design_object_instance_finalize (GObject *object)
  {
  void (*parent_finalize) (GObject *obj) = G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_DESIGN_OBJECT)))->finalize ;
  DBG_QCADDO_FIN (fprintf (stderr, "QCADDesignObject::instance_finalize:finalizing object 0x%08X of type \"%s\"\n", (int)object, g_type_name (G_TYPE_FROM_INSTANCE (object)))) ;
  if (NULL != parent_finalize)
    (*parent_finalize) (object) ;
  DBG_QCADDO_FIN (fprintf (stderr, "QCADDesignObject::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

/**
 * qcad_design_object_add_types:
 * @obj: #QCADDesignObject whose type to add
 * @lst: #GList of unique types (->data is a #GType)
 *
 * Add this object's #GType to the linked list of unique types stored in @lst, if not already present.
 *
 * Returns: the new start of @lst.
 */
GList *qcad_design_object_add_types (QCADDesignObject *obj, GList *lst)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->add_unique_types (obj, lst) ;}

/**
 * qcad_design_object_get_PostScript_preamble:
 * @obj: #QCADDesignObject whose PostScript preamble to retrieve
 *
 * This function returns a string containing the definition of a PostScript function which renders a
 * #QCADDesignObject of this type into a PostScript stream. This function may make use of the PostScript
 * functions defined in %PS_TEXT_PLACEMENT_PREAMBLE, as well as PrintMagic() and the per-page preamble.
 *
 * See also: print_world().
 *
 * Returns: the PostScript preamble for this object's #GType as a string.
 */
const char *qcad_design_object_get_PostScript_preamble (QCADDesignObject *obj)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->PostScript_preamble () ;}

/**
 * qcad_design_object_get_PostScript_instance:
 * @obj: #QCADDesignObject to render as PostScript.
 * @bColour: Whether to render it in colour.
 *
 * This function returns a string containing a PostScript function call to the function returned by 
 * qcad_design_object_get_PostScript_preamble() for this object type. The parameters to the function may be
 * defined in terms of calls to the PostScript functions defined in %PS_TEXT_PLACEMENT_PREAMBLE, as well as 
 * PrintMagic() and the per-page preamble.
 *
 * See also: print_world().
 *
 * Returns: the PostScript instance function call for this object as a string.
 */
char *qcad_design_object_get_PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->PostScript_instance (obj, bColour) ;}

#ifdef GTK_GUI
/**
 * qcad_design_object_draw:
 * @obj: #QCADDesignObject to draw
 * @dst: Surface to draw it on
 * @rop: #GdkFunction to use
 * @rcClip: #GdkRectangle to use for clipping
 *
 * Draws object @obj onto surface @dst using function @rop and clipping rectangle @rcClip.
 */
void qcad_design_object_draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  GdkRectangle *rcPass = rcClip, rc = {0} ;
  if (NULL == rcClip)
    {
    gdk_window_get_size (dst, &(rc.width), &(rc.height)) ;
    rcPass = &rc ;
    }
  QCAD_DESIGN_OBJECT_GET_CLASS (obj)->draw (obj, dst, rop, rcPass) ;
  }

#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
/**
 * qcad_design_object_serialize:
 * @obj: #QCADDesignObject to be serialized
 * @fp: Stream to serialize it to
 *
 * Serializes @obj as plain UTF-8 text to stream @fp. The object can later be unserialized with
 * qcad_design_object_new_from_stream().
 */
void qcad_design_object_serialize (QCADDesignObject *obj, FILE *fp)
  {QCAD_DESIGN_OBJECT_GET_CLASS (obj)->serialize (obj, fp) ;}
#endif /* def STDIO_FILEIO */

/**
 * qcad_design_object_set_selected:
 * @obj: #QCADDesignObject to (de)select
 * @bSelected: Whether the object is to be (de)selected
 *
 * Set @obj as selected if @bSelected is %TRUE. Set @obj as deselected otherwise.
 */
gboolean qcad_design_object_set_selected (QCADDesignObject *obj, gboolean bSelected)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->set_selected (obj, bSelected) ;}

/**
 * qcad_design_object_move:
 * @obj: #QCADDesignObject to be translated
 * @dxDelta: amount in world coordinates of x-translation
 * @dyDelta: amount in world coordinates of y-translation
 *
 * move the specified #QCADDesignObject by a given x- and y-offset.
 */
void qcad_design_object_move (QCADDesignObject *obj, double dxDelta, double dyDelta)
  {QCAD_DESIGN_OBJECT_GET_CLASS (obj)->move (obj, dxDelta, dyDelta) ;}

/**
 * qcad_design_object_move_to:
 * @obj: #QCADDesignObject to be moved
 * @xWorld: world x-coordinate to move it to
 * @yWorld: world y-coordinate to move it to
 *
 * move the specified #QCADDesignObject to the given coordinates.
 */
void qcad_design_object_move_to (QCADDesignObject *obj, double xWorld, double yWorld)
  {
  if (NULL != obj)
    qcad_design_object_move (obj, xWorld - obj->x, yWorld - obj->y) ;
  }

/**
 * qcad_design_object_get_bounds_box:
 * @obj: #QCADDesignObject whose bounding box to calculate
 * @rcWorld: #WorldRectangle to fill out
 *
 * Fill out @rcWorld with the bounding box of @obj.
 */
void qcad_design_object_get_bounds_box (QCADDesignObject *obj, WorldRectangle *rcWorld)
  {QCAD_DESIGN_OBJECT_GET_CLASS (obj)->get_bounds_box (obj, rcWorld) ;}

/**
 * qcad_design_object_select_test:
 * @obj: #QCADDesignObject to examine
 * @rc: #WorldRectangle to use for comparison
 * @method: #QCADSelectionMethod to use for comparison
 *
 * Determine whether @obj lies inside @rcWorld according to @method. @method is one of
 * %SELECTION_CONTAINMENT or %SELECTION_INTERSECTION.
 *
 * Returns: %TRUE if @obj lies inside @rcWorld according to @method
 */
gboolean qcad_design_object_select_test (QCADDesignObject *obj, WorldRectangle *rc, QCADSelectionMethod method)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->select_test (obj, rc, method) ;}

/**
 * qcad_design_object_hit_test:
 * @obj: #QCADDesignObject to examine
 * @x: x-coordinate in pixels to examine
 * @y: y-coordinate in pixels to examine
 *
 * Determine whether (@x,@y) lies inside @obj or any of its sub-objects.
 *
 * Returns: The object hit (which may or may not be @obj), or %NULL if nothing was hit.
 */
QCADDesignObject *qcad_design_object_hit_test (QCADDesignObject *obj, int x, int y)
  {return QCAD_DESIGN_OBJECT_GET_CLASS (obj)->hit_test (obj, x, y) ;}

/**
 * qcad_design_object_transform:
 * @obj: #QCADDesignObject to transform
 * @m11: 2x2 matrix entry
 * @m12: 2x2 matrix entry
 * @m21: 2x2 matrix entry
 * @m22: 2x2 matrix entry
 *
 * Transform object @obj according to world coordinate matrix ((m11,m12),(m21,m22)).
 */
void qcad_design_object_transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22)
  {QCAD_DESIGN_OBJECT_GET_CLASS (obj)->transform (obj, m11, m12, m21, m22) ;}

/**
 * qcad_design_object_overlaps:
 * @obj1: A #QCADDesignObject
 * @obj2: Another #QCADDesignObject
 *
 * Determines whether two objects overlap by intersecting their bounding boxes.
 *
 * Returns: %TRUE, if the two objects overlap.
 */
gboolean qcad_design_object_overlaps (QCADDesignObject *obj1, QCADDesignObject *obj2)
  {
  double
    obj1_xTop, obj1_yTop, obj1_xBot, obj1_yBot,
    obj2_xTop, obj2_yTop, obj2_xBot, obj2_yBot ;

  if (NULL == obj1 || NULL == obj2) return FALSE ;

  obj1_xTop = obj1->bounding_box.xWorld,
  obj1_yTop = obj1->bounding_box.yWorld,
  obj1_xBot = obj1->bounding_box.xWorld + obj1->bounding_box.cxWorld,
  obj1_yBot = obj1->bounding_box.yWorld + obj1->bounding_box.cyWorld,
  obj2_xTop = obj2->bounding_box.xWorld,
  obj2_yTop = obj2->bounding_box.yWorld,
  obj2_xBot = obj2->bounding_box.xWorld + obj2->bounding_box.cxWorld,
  obj2_yBot = obj2->bounding_box.yWorld + obj2->bounding_box.cyWorld ;

  return
    ((obj1_xTop >= obj2_xTop && obj1_xTop <= obj2_xBot && obj1_yTop >= obj2_yTop && obj1_yTop <= obj2_yBot) ||
     (obj1_xTop >= obj2_xTop && obj1_xTop <= obj2_xBot && obj1_yBot >= obj2_yTop && obj1_yBot <= obj2_yBot) ||
     (obj1_xBot >= obj2_xTop && obj1_xBot <= obj2_xBot && obj1_yTop >= obj2_yTop && obj1_yTop <= obj2_yBot) ||
     (obj1_xBot >= obj2_xTop && obj1_xBot <= obj2_xBot && obj1_yBot >= obj2_yTop && obj1_yBot <= obj2_yBot)) ;
  }

#ifdef STDIO_FILEIO
/**
 * qcad_design_object_new_from_stream:
 * @fp: Stream to read from
 *
 * Unserializes a #QCADDesignObject from a stream. The type of object returned depends on the type of object
 * found in the stream.
 *
 * See also: qcad_design_object_serialize().
 *
 * Returns: a #QCADDesignObject, or %NULL if unsuccessful.
 */
QCADDesignObject *qcad_design_object_new_from_stream (FILE *fp)
  {
  int idx = -1, length = -1 ;
  char *pszLine = NULL, *pszType = NULL ;
  QCADDesignObject *obj = NULL ;
  GType type = 0 ;
  char c = 0 ;

  pszLine = ReadLine (fp, '\0', TRUE) ;

  if (NULL == pszLine)
    return NULL ;

  length = strlen (pszLine) ;
  for (idx = 0 ; idx < length ; idx++)
    if (':' == pszLine[idx])
      {
      pszType = &(pszLine[++idx]) ;
      break ;
      }

  for (; ']' != pszLine[idx] && idx < length ; idx++) ;
  c = pszLine[idx] ;
  pszLine[idx] = 0 ;

  if (!(type = g_type_from_name (pszType)))
    {
    fprintf (stderr, "Type %s does not exist !\n", pszType) ;
    g_free (pszLine) ;
    return NULL ;
    }

  pszLine[idx] = c ;

  obj = g_object_new (type, NULL) ;

  g_free (pszLine) ;

  if (!QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (type))->unserialize (obj, fp))
    {
    g_object_unref (obj) ;
    return NULL ;
    }

  return obj ;
  }
#endif /* def STDIO_FILEIO */

/**
 * qcad_design_object_get_state_array:
 * @obj: Object whose state is to be recorded.
 * @...: %NULL-terminated list of object properties.
 *
 * Creates an #EXP_ARRAY of #GValue items, each containing the value of a property listed in the @... list in order.
 *
 * Returns: an #EXP_ARRAY, or %NULL if unsuccessful.
 */
EXP_ARRAY *qcad_design_object_get_state_array (QCADDesignObject *obj, ...)
  {
  va_list va ;
  char *pszPropertyName = NULL ;
  EXP_ARRAY *ar = NULL ;
  GParameter *opv = NULL ;
  GParamSpec *param_spec = NULL ;
  GValue *val = NULL ;

  if (NULL == obj) return NULL ;

  va_start (va, obj) ;

  while (NULL != (pszPropertyName = va_arg (va, char *)))
    if (NULL != (param_spec = g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (obj)), pszPropertyName)))
      if (0 != param_spec->value_type)
        {
        if (NULL == ar)
          ar = exp_array_new (sizeof (GParameter), 1) ;
        exp_array_1d_insert_vals (ar, NULL, 1, -1) ;
        opv = &exp_array_index_1d (ar, GParameter, ar->icUsed - 1) ;
        opv->name = g_strdup (pszPropertyName) ;
        val = &(opv->value) ;
        memset (val, 0, sizeof (GValue)) ;
        g_value_init (val, param_spec->value_type) ;
        g_object_get_property (G_OBJECT (obj), pszPropertyName, val) ;
        }

  va_end (va) ;

  return ar ;
  }

void qcad_design_object_state_array_free (EXP_ARRAY *ar)
  {
  int Nix ;
  GParameter *opv = NULL ;

  for (Nix = 0 ; Nix < ar->icUsed ; Nix++)
    {
    opv = &(exp_array_index_1d (ar, GParameter, Nix)) ;
    g_free ((char *)(opv->name)) ;
    g_value_unset (&(opv->value)) ;
    }

  exp_array_free (ar) ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) {}

#endif /* def GTK_GUI */

static void copy (QCADObject *objSrc, QCADObject *objDst)
  {
  QCADDesignObject *src = QCAD_DESIGN_OBJECT (objSrc), *dst = QCAD_DESIGN_OBJECT (objDst) ;
  dst->x         = src->x ;
  dst->y         = src->y ;
  memcpy (&(dst->bounding_box), &(src->bounding_box), sizeof (WorldRectangle)) ;
  dst->bSelected = src->bSelected ;
  dst->clr.red   = src->clr.red ;
  dst->clr.green = src->clr.green ;
  dst->clr.blue  = src->clr.blue ;
  dst->clr.pixel = src->clr.pixel ;
  }

static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22)
  {
  double
    x = obj->x * m11 + obj->y * m21,
    y = obj->x * m12 + obj->y * m22,
    xTop = obj->bounding_box.xWorld,
    yTop = obj->bounding_box.yWorld,
    xBot = xTop + obj->bounding_box.cxWorld,
    yBot = yTop + obj->bounding_box.cyWorld ;

  obj->x = x ;
  obj->y = y ;

  x = xTop * m11 + yTop * m21 ;
  y = xTop * m12 + yTop * m22 ;

  xTop = x ;
  yTop = y ;

  x = xBot * m11 + yBot * m21 ;
  y = xBot * m12 + yBot * m22 ;

  xBot = x ;
  yBot = y ;

  obj->bounding_box.xWorld  = MIN (xTop, xBot) ;
  obj->bounding_box.yWorld  = MIN (yTop, yBot) ;
  obj->bounding_box.cxWorld = MAX (xTop, xBot) - obj->bounding_box.xWorld ;
  obj->bounding_box.cyWorld = MAX (yTop, yBot) - obj->bounding_box.yWorld ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  char pszDouble[7][G_ASCII_DTOSTR_BUF_SIZE] = {""} ;
  GdkColor clr ;

  if (QCAD_IS_RECTANGLE_ELECTRODE (obj))
    printf ("(%lf,%lf)[%lfx%lf]\n", obj->bounding_box.xWorld, obj->bounding_box.yWorld, obj->bounding_box.cxWorld, obj->bounding_box.cyWorld) ;

  if (bColour)
    return g_strdup_printf ("%s nmx %s nmy %s nm %s nm %s %s %s QCADDesignObject",
      g_ascii_dtostr (pszDouble[0], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld),
      g_ascii_dtostr (pszDouble[1], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld),
      g_ascii_dtostr (pszDouble[2], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld),
      g_ascii_dtostr (pszDouble[3], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld),
      g_ascii_dtostr (pszDouble[4], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.red)) / 65536.0),
      g_ascii_dtostr (pszDouble[5], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.green)) / 65536.0),
      g_ascii_dtostr (pszDouble[6], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.blue)) / 65536.0)) ;
  else
    {
    memcpy (&clr, &(obj->clr), sizeof (GdkColor)) ;
    RGBToHSL (&clr) ;
    return g_strdup_printf ("%s nmx %s nmy %s nm %s nm %s %s %s QCADDesignObject",
      g_ascii_dtostr (pszDouble[0], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld),
      g_ascii_dtostr (pszDouble[1], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld),
      g_ascii_dtostr (pszDouble[2], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld),
      g_ascii_dtostr (pszDouble[3], G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld),
      g_ascii_dtostr (pszDouble[4], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.blue)) / 65536.0),
      g_ascii_dtostr (pszDouble[5], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.blue)) / 65536.0),
      g_ascii_dtostr (pszDouble[6], G_ASCII_DTOSTR_BUF_SIZE, ((double)(obj->clr.blue)) / 65536.0)) ;
    }
  }

static const char *PostScript_preamble ()
  {
  return
    "% x y cx cy r g b " QCAD_TYPE_STRING_DESIGN_OBJECT "\n"
    "/" QCAD_TYPE_STRING_DESIGN_OBJECT " {\n"
    "  gsave\n"
    "  /b exch def\n"
    "  /g exch def\n"
    "  /r exch def\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "  \n"
    "  [2 2 2 2] 0 setdash\n"
    "\n"
    "  newpath\n"
    "  x y moveto\n"
    "  x cx add y lineto\n"
    "  x cx add y cy sub lineto\n"
    "  x y cy sub lineto\n"
    "  r g b setrgbcolor\n"
    "  closepath stroke\n"
    "\n"
    "  grestore\n"
    "} def\n" ;
  }

static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rcWorld)
  {memcpy (rcWorld, &(obj->bounding_box), sizeof (WorldRectangle)) ;}

static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected)
  {
  gboolean bRet = (bSelected == obj->bSelected) ;
  obj->bSelected = bSelected ;
  if (!bRet)
    g_signal_emit (obj, qcad_design_object_signals[QCAD_DESIGN_OBJECT_SELECTED_SIGNAL], 0) ;
  return bRet ;
  }

static void move (QCADDesignObject *obj, double dxDelta, double dyDelta)
  {
  if (!QCAD_IS_DESIGN_OBJECT (obj)) return ;

  QCAD_DESIGN_OBJECT (obj)->bounding_box.xWorld += dxDelta ;
  QCAD_DESIGN_OBJECT (obj)->bounding_box.yWorld += dyDelta ;
  obj->x += dxDelta ;
  obj->y += dyDelta ;
  }

static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal)
  {
  double xWorld = real_to_world_x (xReal), yWorld = real_to_world_y (yReal) ;

  if (!QCAD_IS_DESIGN_OBJECT (obj)) return NULL ;

  return (((xWorld >= obj->bounding_box.xWorld) && (xWorld <= obj->bounding_box.xWorld + obj->bounding_box.cxWorld)) &&
          ((yWorld >= obj->bounding_box.yWorld) && (yWorld <= obj->bounding_box.yWorld + obj->bounding_box.cyWorld))) ? obj : NULL ;
  }

static gboolean select_test (QCADDesignObject *obj, WorldRectangle *rc, QCADSelectionMethod method)
  {
//  if (!QCAD_IS_SUBSTRATE (obj))
//    fprintf (stderr, "QCADDesignObject::select_test for 0x%08X:(%.2lf,%.2lf)[%.2lfx%.2lf] %s (%.2lf,%.2lf)[%.2lfx%.2lf]\n", (int)obj,
//      obj->bounding_box.xWorld, obj->bounding_box.yWorld, obj->bounding_box.cxWorld, obj->bounding_box.cyWorld, 
//      SELECTION_CONTAINMENT == method ? "contained in" : "intersects",
//      rc->xWorld, rc->yWorld, rc->cxWorld, rc->cyWorld) ;
  return
    (((SELECTION_CONTAINMENT == method) &&
       (RECT_IN_RECT (
          obj->bounding_box.xWorld,
          obj->bounding_box.yWorld,
          obj->bounding_box.cxWorld,
          obj->bounding_box.cyWorld,
          rc->xWorld,
          rc->yWorld,
          rc->cxWorld,
          rc->cyWorld) ||
        RECT_IN_RECT (
          rc->xWorld,
          rc->yWorld,
          rc->cxWorld,
          rc->cyWorld,
          obj->bounding_box.xWorld,
          obj->bounding_box.yWorld,
          obj->bounding_box.cxWorld,
          obj->bounding_box.cyWorld))) ||
     ((SELECTION_INTERSECTION == method) &&
       (RECT_INTERSECT_RECT (
        obj->bounding_box.xWorld,
        obj->bounding_box.yWorld,
        obj->bounding_box.cxWorld,
        obj->bounding_box.cyWorld,
        rc->xWorld,
        rc->yWorld,
        rc->cxWorld,
        rc->cyWorld)))) ;
  }

#ifdef STDIO_FILEIO
static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  char *pszLine = NULL, *pszValue = NULL ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_DESIGN_OBJECT "]", NULL))
    return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;
    if (!strcmp (pszLine, "[#TYPE:" QCAD_TYPE_STRING_DESIGN_OBJECT "]"))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if (!strcmp (pszLine, "x"))
      obj->x = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strcmp (pszLine, "y"))
      obj->y = g_ascii_strtod (pszValue, NULL) ;
    else
// compatibility values
    if (!strcmp (pszLine, "xTop"))
      obj->bounding_box.xWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strcmp (pszLine, "yTop"))
      obj->bounding_box.yWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    // The following 2 calculations impose the following restriction:
    // For any given object, xTop and yTop must appear before xBot and yBot
    if (!strcmp (pszLine, "xBot"))
      obj->bounding_box.cxWorld = g_ascii_strtod (pszValue, NULL) - obj->bounding_box.xWorld ;
    else
    if (!strcmp (pszLine, "yBot"))
      obj->bounding_box.cyWorld = g_ascii_strtod (pszValue, NULL) - obj->bounding_box.yWorld ;
// new, rectangle notation
    else
    if (!strcmp (pszLine, "bounding_box.xWorld"))
      obj->bounding_box.xWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strcmp (pszLine, "bounding_box.yWorld"))
      obj->bounding_box.yWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strcmp (pszLine, "bounding_box.cxWorld"))
      obj->bounding_box.cxWorld = g_ascii_strtod (pszValue, NULL) ;
    else
    if (!strcmp (pszLine, "bounding_box.cyWorld"))
      obj->bounding_box.cyWorld = g_ascii_strtod (pszValue, NULL) ;
// end differences
    else
    if (!strcmp (pszLine, "bSelected"))
      obj->bSelected = (strcmp (pszValue, "FALSE") ? TRUE : FALSE) ;
    else
    if (!strcmp (pszLine, "clr.red"))
      obj->clr.red = atoi (pszValue) ;
    else
    if (!strcmp (pszLine, "clr.green"))
      obj->clr.green = atoi (pszValue) ;
    else
    if (!strcmp (pszLine, "clr.blue"))
      obj->clr.blue = atoi (pszValue) ;

    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }
  obj->clr.pixel = 0 ;
#ifdef GTK_GUI
  gdk_colormap_alloc_color (gdk_colormap_get_system (), &(obj->clr), FALSE, TRUE) ;
#endif /* def GTK_GUI */
  return TRUE ;
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  char pszDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;
  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_DESIGN_OBJECT);

  // output variables
  fprintf(fp, "x=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->x));
  fprintf(fp, "y=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->y));
  fprintf(fp, "bSelected=%s\n", obj->bSelected ? "TRUE" : "FALSE");
  fprintf(fp, "clr.red=%d\n", obj->clr.red);
  fprintf(fp, "clr.green=%d\n", obj->clr.green);
  fprintf(fp, "clr.blue=%d\n", obj->clr.blue);
  fprintf(fp, "bounding_box.xWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld));
  fprintf(fp, "bounding_box.yWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld));
  fprintf(fp, "bounding_box.cxWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld));
  fprintf(fp, "bounding_box.cyWorld=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld));

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_DESIGN_OBJECT);
  }
#endif /* def STDIO_FILEIO */

static GList *add_unique_types (QCADDesignObject *obj, GList *lst)
  {
  GList *lstItr = NULL ;

  for (lstItr = lst ; lstItr != NULL ; lstItr = lstItr->next)
    if (G_TYPE_FROM_INSTANCE (obj) == G_TYPE_FROM_INSTANCE (lstItr->data))
      return lst ;

  return g_list_prepend (lst, obj) ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {return FALSE ;}

static gboolean motion_notify (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {return FALSE ;}

static gboolean button_released (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {return FALSE ;}
#endif /* def GTK_GUI */

///////////////////////////////////////////////////////////////////////////////
