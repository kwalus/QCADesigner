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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "QCADObject.h"
#include "QCADPropertyUISingle.h"
#include "QCADPropertyUIGroup.h"

/**
 * SECTION:QCADObject
 * @short_description: QCADesigner's extensions to #GObject
 *
 * #QCADObject provides several additions to #GObject:
 * <itemizedlist>
 * <listitem><para>
 * A consistent way of performing a deep copy of an object via qcad_object_new_from_object().
 * </para></listitem>
 * <listitem><para>
 * The concept of a default object. Each subclass of #QCADObject can have a default instance. This instance 
 * can then serve as a template for new objects. For example:
 * <informalexample><programlisting>
 * new_object = qcad_object_new_from_default (qcad_object_get_default (qcad_object_subtype)) ;
 * </programlisting></informalexample>
 * When an object becomes the default object, it emits the "set-default" signal, and when another object
 * becomes the default, it emits the "unset-default" signal, and the new default object emits the "set-default"
 * signal.
 *
 * As a convenience, qcad_object_connect_signal_to_default_object() will connect a signal handler to the
 * current default object, and will keep it connected to whatever object becomes default at a later time.
 *
 * As an additional convenience, qcad_object_create_property_ui_for_default_object() will create a property
 * UI for one of the default object's properties, and will keep it pointing at the object currently set as
 * default.
 * </para></listitem>
 * <listitem><para>
 * Property UI property hints. When constructing a UI for a #QCADObject property, the UI itself might have some
 * properties whose default values should be overridden. Assembling an array of #QCADPropertyUIProperty 
 * structures and passing the array to qcad_object_class_install_ui_properties() at class initialization time 
 * will cause UIs exposing the given property to have properties set as described in the array. For example:
 * <informalexample><programlisting>
 * static QCADPropertyUIProperty properties[] =
 *   {
 *   {NULL,     "title",     {0, }},
 *   {"width",  "units",     {0, }},
 *   {"height", "units",     {0, }},
 *   } ;
 *  
 * // RectangleElectrode.title = "QCA Rectangular Electrode"
 * g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), _("QCA Rectangular Electrode")) ;
 * // RectangleElectrode.width.units = "nm"
 * g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;
 * // RectangleElectrode.height.units = "nm"
 * g_value_set_string (g_value_init (&(properties[2].ui_property_value), G_TYPE_STRING), "nm") ;
 *  
 * qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
 * </programlisting></informalexample>
 * This will cause property UIs exposing an object's "width" property and "height" property to display
 * a "nm" label as part of the property UI. Note that, when the first member of the structure is %NULL, the 
 * second member refers to a property UI hint for the property UI group that will be created for objects of 
 * this type.
 * </para></listitem>
 * <listitem><para>
 * Property UI behaviour. In addition to assigning values to certain property UI properties at class 
 * initialization time, it is also possible to ``connect'' property UI properties to other property UI 
 * properties, as well as property UI properties to properties of the instance treated by the property UIs. 
 * This is accomplished using the connect_object_properties() function.
 * </para></listitem>
 * </itemizedlist>
 */

typedef struct
  {
  GType type ;
  guint replace_default_handler_id ;
  } DefaultGoneStruct ;

typedef struct
  {
  DefaultGoneStruct default_is_gone ;
  QCADPropertyUI *pui ;
  } QCADPropertyUIDefaultGoneStruct ;

typedef struct
  {
  DefaultGoneStruct default_is_gone ;
  gulong notify_id ;
  char *pszSignal ;
  GCallback pfn ;
  gpointer data ;
  } SignalDefaultGoneStruct ;

static void qcad_object_class_init (GObjectClass *klass, gpointer data) ;
#ifdef PROPERTY_UIS
static void qcad_object_base_init (QCADObjectClass *klass) ;
static void qcad_object_base_finalize (QCADObjectClass *klass) ;
#endif /* def PROPERTY_UIS */
static void qcad_object_instance_finalize (GObject *object) ;

static void copy (QCADObject *objSrc, QCADObject *objDst) ;

#ifdef PROPERTY_UIS
static void qcad_object_reset_ui_for_default_object (GObject *obj, gpointer data) ;
#endif /* def PROPERTY_UIS */
static void qcad_object_reset_signal_for_default_object (GObject *obj, gpointer data) ;

enum
  {
  QCAD_OBJECT_SET_DEFAULT_SIGNAL,
  QCAD_OBJECT_UNSET_DEFAULT_SIGNAL,
  QCAD_OBJECT_LAST_SIGNAL
  } ;

static guint qcad_object_signals[QCAD_OBJECT_LAST_SIGNAL] = {0} ;

GType qcad_object_get_type ()
  {
  static GType qcad_object_type = 0 ;

  if (!qcad_object_type)
    {
    static const GTypeInfo qcad_object_info =
      {
      sizeof (QCADObjectClass),
#ifdef PROPERTY_UIS
      (GBaseInitFunc)qcad_object_base_init,
      (GBaseFinalizeFunc)qcad_object_base_finalize,
#else
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
#endif /* def PROPRETY_UIS */
      (GClassInitFunc)qcad_object_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADObject),
      0,
      (GInstanceInitFunc)NULL
      } ;

    if ((qcad_object_type = g_type_register_static (G_TYPE_OBJECT, "QCADObject", &qcad_object_info, 0)))
      g_type_class_ref (qcad_object_type) ;
    }
  return qcad_object_type ;
  }

#ifdef PROPERTY_UIS
static void qcad_object_base_init (QCADObjectClass *klass)
  {
  QCADObjectClass *parent_klass = g_type_class_peek (g_type_parent (G_TYPE_FROM_CLASS (klass))) ;

  if (QCAD_TYPE_OBJECT == G_TYPE_FROM_CLASS (klass))
    {
    klass->property_ui_behaviour = NULL ;
    klass->property_ui_properties = NULL ;
    }
  else
    {
    klass->property_ui_behaviour = exp_array_copy (parent_klass->property_ui_behaviour) ;
    klass->property_ui_properties = exp_array_copy (parent_klass->property_ui_properties) ;
    }
  }

static void qcad_object_base_finalize (QCADObjectClass *klass)
  {
  if (NULL != klass->property_ui_behaviour)
    exp_array_free (klass->property_ui_behaviour) ;
  if (NULL != klass->property_ui_properties)
    exp_array_free (klass->property_ui_properties) ;

  klass->property_ui_behaviour = NULL ;
  klass->property_ui_properties = NULL ;
  }
#endif /* def PROPERTY_UIS */

static void qcad_object_class_init (GObjectClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->finalize = qcad_object_instance_finalize ;
  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = NULL ;

  qcad_object_signals[QCAD_OBJECT_SET_DEFAULT_SIGNAL] =
    g_signal_new ("set-default", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADObjectClass, set_default), NULL, NULL, g_cclosure_marshal_VOID__VOID, 
      G_TYPE_NONE, 0) ;

  qcad_object_signals[QCAD_OBJECT_UNSET_DEFAULT_SIGNAL] =
    g_signal_new ("unset-default", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADObjectClass, unset_default), NULL, NULL, g_cclosure_marshal_VOID__VOID, 
      G_TYPE_NONE, 0) ;
  }

static void qcad_object_instance_finalize (GObject *object)
  {
  void (*parent_finalize) (GObject *obj) = G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_OBJECT)))->finalize ;
  if (NULL != parent_finalize)
    (*parent_finalize) (object) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADObject *qcad_object_new_from_object (QCADObject *src)
  {
  GType type = 0 ;
  QCADObject *dst = NULL ;

  if (NULL == src) return NULL ;

  type = G_TYPE_FROM_INSTANCE (src) ;

  if (type)
    {
    dst = g_object_new (type, NULL) ;
    QCAD_OBJECT_GET_CLASS (src)->copy (src, dst) ;
    }

  return dst ;
  }

QCADObject *qcad_object_get_default (GType type)
  {
  QCADObject *ret = NULL ;
  gpointer klass = NULL ;

  if (0 != type)
    {
    if (NULL == QCAD_OBJECT_CLASS (klass = g_type_class_peek (type))->default_object)
      {
      if (NULL != QCAD_OBJECT_CLASS (klass)->class_get_default_object)
        ret = (QCAD_OBJECT_CLASS (klass)->default_object =
          (QCAD_OBJECT_CLASS (klass)->class_get_default_object ())) ;
      else
        ret = NULL ;
      }
    else
      ret = QCAD_OBJECT_CLASS (klass)->default_object ;
    }
  if (NULL == ret)
    g_warning ("qcad_object_get_default: Returning NULL for type %s\n", (0 == type ? "0" : g_type_name (type))) ;
  return ret ;
  }

void qcad_object_set_default (GType type, QCADObject *obj)
  {
  QCADObject *old_default = NULL ;
  QCADObjectClass *klass = NULL ;

  if (0 == type)
    {
    g_warning ("qcad_object_set_default: Cannot set default object for type 0\n") ;
    return ;
    }
  if (NULL == obj)
    {
    g_warning ("qcad_object_set_default: Refusing to set NULL default object for type %s\n", g_type_name (type)) ;
    return ;
    }
  if (G_TYPE_FROM_INSTANCE (obj) != type)
    {
    g_warning ("qcad_object_set_default: Refusing to set default object of type %s for class of type %s\n",
      g_type_name (G_TYPE_FROM_INSTANCE (obj)), g_type_name (type)) ;
    return ;
    }

  // Do not set the default object to itself
  if ((old_default = (klass = QCAD_OBJECT_CLASS (g_type_class_peek (type)))->default_object) == obj) return ;
  klass->default_object = g_object_ref (obj) ;
  if (NULL != old_default)
    g_signal_emit (G_OBJECT (old_default), qcad_object_signals[QCAD_OBJECT_UNSET_DEFAULT_SIGNAL], 0) ;
  g_signal_emit (G_OBJECT (obj), qcad_object_signals[QCAD_OBJECT_SET_DEFAULT_SIGNAL], 0) ;
  if (NULL != old_default)
    g_object_unref (G_OBJECT (old_default)) ;
  }

#ifdef PROPERTY_UIS
QCADPropertyUI *qcad_object_create_property_ui_for_default_object (GType type, char *property, ...)
  {
  QCADPropertyUI *pui = NULL ;
  QCADObject *obj = NULL ;
  QCADPropertyUIDefaultGoneStruct *pui_default_is_gone = NULL ;
  va_list va ;
  char *pszFirstProp = NULL ;

  if (0 == type || NULL == property) return NULL ;
  if (NULL == (obj = qcad_object_get_default (type))) return NULL ;
  if (NULL == (pui = qcad_property_ui_single_new (G_OBJECT (obj), property, NULL))) return NULL ;

  va_start (va, property) ;
  if (NULL != (pszFirstProp = va_arg (va, char *)))
    g_object_set_valist (G_OBJECT (pui), pszFirstProp, va) ;
  va_end (va) ;

  pui_default_is_gone = g_malloc0 (sizeof (QCADPropertyUIDefaultGoneStruct)) ;
  pui_default_is_gone->pui  = pui ;
  pui_default_is_gone->default_is_gone.type = type ;

  // When the default object is replaced, attach this UI to the new default object
  pui_default_is_gone->default_is_gone.replace_default_handler_id =
    g_signal_connect (G_OBJECT (obj), "unset-default", (GCallback)qcad_object_reset_ui_for_default_object, pui_default_is_gone) ;
  // When the UI is destroyed, destroy the above weak_ref and the structure associated with it
  g_object_weak_ref (G_OBJECT (pui), (GWeakNotify)g_free, pui_default_is_gone) ;

  return pui ;
  }
#endif /* def PROPERTY_UIS */

void qcad_object_connect_signal_to_default_object (GType type, char *pszSignal, GCallback callback, gpointer data)
  {
  QCADObject *obj = NULL ;
  gulong notify_id = 0 ;
  SignalDefaultGoneStruct *sig_default_is_gone = NULL ;

  if (0 == type || NULL == pszSignal || NULL == callback) return ;
  if (NULL == (obj = qcad_object_get_default (type))) return ;
  if (0 == (notify_id = g_signal_connect (G_OBJECT (obj), pszSignal, callback, data))) return ;

  sig_default_is_gone = g_malloc0 (sizeof (SignalDefaultGoneStruct)) ;
  sig_default_is_gone->pszSignal = g_strdup (pszSignal) ;
  sig_default_is_gone->pfn = callback ;
  sig_default_is_gone->data = data ;
  sig_default_is_gone->notify_id =
    g_signal_connect (G_OBJECT (obj), pszSignal, callback, data) ;
  sig_default_is_gone->default_is_gone.type = type ;
  sig_default_is_gone->default_is_gone.replace_default_handler_id =
    g_signal_connect (G_OBJECT (obj), "unset-default", (GCallback)qcad_object_reset_signal_for_default_object, sig_default_is_gone) ;
  }

#ifdef PROPERTY_UIS
void qcad_object_class_install_ui_behaviour (QCADObjectClass *klass, QCADPropertyUIBehaviour *behaviour, int icBehaviour)
  {
  if (NULL == klass->property_ui_behaviour)
    klass->property_ui_behaviour = exp_array_new (sizeof (QCADPropertyUIBehaviour), 1) ;
  exp_array_1d_insert_vals (klass->property_ui_behaviour, behaviour, icBehaviour, -1) ;
  }

void qcad_object_class_install_ui_properties (QCADObjectClass *klass, QCADPropertyUIProperty *properties, int icProperties)
  {
  int Nix, Nix1 ;
  QCADPropertyUIProperty *klass_puip = NULL ;

  if (NULL == klass->property_ui_properties)
    klass->property_ui_properties = exp_array_new (sizeof (QCADPropertyUIProperty), 1) ;

  for (Nix = 0 ; Nix < icProperties ; Nix++)
    {
    for (Nix1 = 0 ; Nix1 < klass->property_ui_properties->icUsed ; Nix1++)
      {
      klass_puip = &exp_array_index_1d (klass->property_ui_properties, QCADPropertyUIProperty, Nix1) ;

      if (NULL == properties[Nix].instance_property_name && NULL != klass_puip->instance_property_name) continue ;
      if (NULL != properties[Nix].instance_property_name && NULL == klass_puip->instance_property_name) continue ;
      if (NULL != properties[Nix].instance_property_name)
        if (strcmp (properties[Nix].instance_property_name, klass_puip->instance_property_name)) continue ;

      if (NULL == properties[Nix].ui_property_name && NULL != klass_puip->ui_property_name) continue ;
      if (NULL != properties[Nix].ui_property_name && NULL == klass_puip->ui_property_name) continue ;
      if (NULL != properties[Nix].ui_property_name)
        if (strcmp (properties[Nix].ui_property_name, klass_puip->ui_property_name)) continue ;

      memcpy (&(klass_puip->ui_property_value), &(properties[Nix].ui_property_value), sizeof (GValue)) ;
      break ;
      }
    if (Nix1 == klass->property_ui_properties->icUsed)
      exp_array_1d_insert_vals (klass->property_ui_properties, &(properties[Nix]), 1, -1) ;
    }
  }
#endif /* def PROPERTY_UIS */

///////////////////////////////////////////////////////////////////////////////

static void copy (QCADObject *objSrc, QCADObject *objDst) {}

///////////////////////////////////////////////////////////////////////////////

#ifdef PROPERTY_UIS
static void qcad_object_reset_ui_for_default_object (GObject *obj, gpointer data)
  {
  QCADPropertyUIDefaultGoneStruct *pui_default_is_gone = (QCADPropertyUIDefaultGoneStruct *)data ;
  QCADObject *new_default = qcad_object_get_default (pui_default_is_gone->default_is_gone.type) ;

  g_signal_handler_disconnect (obj, pui_default_is_gone->default_is_gone.replace_default_handler_id) ;
  pui_default_is_gone->default_is_gone.replace_default_handler_id =
    g_signal_connect (G_OBJECT (new_default), "unset-default", (GCallback)qcad_object_reset_ui_for_default_object, data) ;

  qcad_property_ui_set_instance (pui_default_is_gone->pui, G_OBJECT (new_default)) ;
  }
#endif /* def PROPERTY_UIS */

static void qcad_object_reset_signal_for_default_object (GObject *obj, gpointer data)
  {
  SignalDefaultGoneStruct *sig_default_is_gone = (SignalDefaultGoneStruct *)data ;
  QCADObject *new_default = qcad_object_get_default (sig_default_is_gone->default_is_gone.type) ;

  g_signal_handler_disconnect (obj, sig_default_is_gone->default_is_gone.replace_default_handler_id) ;
  sig_default_is_gone->default_is_gone.replace_default_handler_id =
    g_signal_connect (G_OBJECT (new_default), "unset-default", (GCallback)qcad_object_reset_signal_for_default_object, data) ;

  g_signal_handler_disconnect (obj, sig_default_is_gone->notify_id) ;
  sig_default_is_gone->notify_id =
    g_signal_connect (G_OBJECT (new_default), sig_default_is_gone->pszSignal, sig_default_is_gone->pfn, sig_default_is_gone->data) ;
  }
