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
// Stuff that wouldn't fit anywhere else.               //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
  #include <unistd.h>
#endif /* ndef WIN32 */
#include <glib.h>
#include "exp_array.h"
#include "generic_utils.h"
#include "global_consts.h"
#include "objects/object_helpers.h"

#ifdef GTK_GUI
typedef struct
  {
  char *pszCmdLine ;
  char *pszTmpFName ;
  }
RUN_CMD_LINE_ASYNC_THREAD_PARAMS ;

static gpointer RunCmdLineAsyncThread (gpointer p) ;
#endif /* def GTK_GUI */

typedef struct
  {
  GObject *notify_dst ;
  char *pszDstProperty ;

  PropertyConnectFunction connect_function ;
  gpointer connect_data ;
  }
ConnectObjectPropertiesStruct ;

typedef struct
  {
  GObject *obj ;
  gulong notify_id ;
  ConnectObjectPropertiesStruct *connect_struct ;
  GDestroyNotify destroy_notify ;
  }
ConnectObjectPropertiesDestroyEntry ;

typedef struct
  {
  ConnectObjectPropertiesDestroyEntry src_entry ;
  ConnectObjectPropertiesDestroyEntry dst_entry ;
  }
ConnectObjectPropertiesDestroyStruct ;

#define CONNECT_OBJECT_PROPERTIES_DATA_KEY "connect_object_properties:0x%08X:%s:0x%08X:%s:<0x%08X>:0x%08X:<0x%08X>:<0x%08X>:0x%08X:<0x%08X>"

static void connect_object_properties_notify (GObject *obj, GParamSpec *param_spec, gpointer data) ;
static void connect_object_properties_break_connections (gpointer data, GObject *dead_object) ;
static void connect_object_properties_destroy_struct_free (ConnectObjectPropertiesDestroyStruct *destroy_struct) ;

// Causes a rectangle of width (*pdRectWidth) and height (*pdRectHeight) to fit inside a rectangle of
// width dWidth and height dHeight.  Th resulting pair ((*px),(*py)) holds the coordinates of the
// upper left corner of the scaled rectangle wrt. the upper left corner of the given rectangle.
void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight)
  {
  double dAspectRatio, dRectAspectRatio ;

  if (0 == dWidth || 0 == dHeight || 0 == *pdRectWidth || 0 == *pdRectHeight) return ;

  dAspectRatio = dWidth / dHeight ;
  dRectAspectRatio = *pdRectWidth / *pdRectHeight ;

  if (dRectAspectRatio > dAspectRatio)
    {
    *px = 0 ;
    *pdRectWidth = dWidth ;
    *pdRectHeight = *pdRectWidth / dRectAspectRatio ;
    *py = (dHeight - *pdRectHeight) / 2 ;
    }
  else
    {
    *py = 0 ;
    *pdRectHeight = dHeight ;
    *pdRectWidth = *pdRectHeight * dRectAspectRatio ;
    *px = (dWidth - *pdRectWidth) / 2 ;
    }
  }

// Convert a long long value to decimal, hexadecimal, or binary
char *strdup_convert_to_base (long long value, int base)
  {
  if (10 == base)
    return g_strdup_printf ("%llu", value) ;
  else
  if (16 == base)
    return g_strdup_printf ("%llX", value) ;
  else
  if (2 == base)
    {
    char *psz = NULL ;
    EXP_ARRAY *str = NULL ;

    if (0 == value)
      return g_strdup ("0") ;

    str = exp_array_new (sizeof (char), 1) ;

    while (value)
      {
      exp_array_insert_vals (str, NULL, 1, 1, -1) ;
      exp_array_index_1d (str, char, str->icUsed - 1) = (value & 0x1) ? '1' : '0' ;
      value = value >> 1 ;
      }

    exp_array_insert_vals (str, NULL, 1, 1, -1) ;
    exp_array_index_1d (str, char, str->icUsed - 1) = 0 ;
    psz = g_strreverse (g_strndup (str->data, str->icUsed)) ;
    exp_array_free (str) ;
    return psz ;
    }
  return NULL ;
  }

// Decide whether the slope difference between two line segments is significant enough for the three points
// demarcating the two line segments not to be considered collinear
//                     (dx2,dy2)
//                        /
//                       /
//(dx0,dy0)             /
//    _________________/
//                   (dx1,dy1)
gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dMaxSlopeDiff)
  {
  if (dx0 == dx1 && dx1 == dx2) return TRUE ;
  if (dy0 == dy1 && dy1 == dy2) return TRUE ;

  return (fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)) < dMaxSlopeDiff) ;
  }

#ifdef GTK_GUI
void RunCmdLineAsync (char *pszCmdLine, char *pszTmpFName)
  {
  RUN_CMD_LINE_ASYNC_THREAD_PARAMS *prclap = g_malloc0 (sizeof (RUN_CMD_LINE_ASYNC_THREAD_PARAMS)) ;

  prclap->pszCmdLine = g_strdup (pszCmdLine) ;
  prclap->pszTmpFName = (NULL == pszTmpFName ? NULL : g_strdup (pszTmpFName)) ;

  if (!g_thread_supported ()) g_thread_init (NULL) ;

  g_thread_create ((GThreadFunc)RunCmdLineAsyncThread, (gpointer)prclap, FALSE, NULL) ;
  }

static gpointer RunCmdLineAsyncThread (gpointer p)
  {
  RUN_CMD_LINE_ASYNC_THREAD_PARAMS *prclap = (RUN_CMD_LINE_ASYNC_THREAD_PARAMS *)p ;
#ifdef WIN32
  STARTUPINFO si ;
  PROCESS_INFORMATION pi ;

  memset (&si, 0, sizeof (si)) ;
  memset (&pi, 0, sizeof (pi)) ;
  si.cb = sizeof (STARTUPINFO) ;

  if (CreateProcess (NULL, prclap->pszCmdLine, NULL, NULL, FALSE, DETACHED_PROCESS,
    NULL, NULL, &si, &pi))
    {
    WaitForSingleObject (pi.hProcess, INFINITE) ;
    CloseHandle (pi.hProcess) ;
    CloseHandle (pi.hThread) ;
    }
#else
  system (prclap->pszCmdLine) ;
#endif
  g_free (prclap->pszCmdLine) ;
  if (NULL != prclap->pszTmpFName)
    {
#ifdef WIN32
    DeleteFile (prclap->pszTmpFName) ;
#else
    unlink (prclap->pszTmpFName) ;
#endif /* def WIN32 */
    g_free (prclap->pszTmpFName) ;
    }
  g_free (prclap) ;

  return NULL ;
  }
#endif /* def GTK_GUI */

char *get_enum_string_from_value (GType enum_type, int value)
  {
  GEnumClass *klass = g_type_class_ref (enum_type) ;
  GEnumValue *val = NULL ;

  if (NULL == klass) return g_strdup_printf ("%d", value) ;

  if (NULL == (val = g_enum_get_value (klass, value)))
    return g_strdup_printf ("%d", value) ;
  else
    return g_strdup (val->value_name) ;

  g_type_class_unref (klass) ;
  }

int get_enum_value_from_string (GType enum_type, char *psz)
  {
  GEnumClass *klass = g_type_class_peek (enum_type) ;
  GEnumValue *val = NULL ;

  if (NULL == klass) return g_ascii_strtod (psz, NULL) ;

  if (NULL == (val = g_enum_get_value_by_name (klass, psz)))
    return g_ascii_strtod (psz, NULL) ;
  else
    return val->value ;
  }

double spread_seq (int idx)
  {
  int numer = 0, denom = 1, ic = 0, idx_copy ;

  if (idx < 2) return idx ;
  if (idx == 2) return 0.5 ;
  idx-- ;
  idx_copy = idx ;

  // Fancy shmancy log base 2
  while (idx)
    {
    ic++ ;
    idx = idx >> 1 ;
    }
  denom = 1 << ic ;
  numer = ((idx_copy << 1) % denom) + 1 ;

  return ((double)numer) / ((double)denom) ;
  }

gboolean connect_object_properties (GObject *src, char *pszSrc, GObject *dst, char *pszDst, PropertyConnectFunction fn_forward, gpointer data_forward, GDestroyNotify destroy_forward, PropertyConnectFunction fn_reverse, gpointer data_reverse, GDestroyNotify destroy_reverse)
  {
  char *pszDataName = NULL ;
  char *psz = NULL ;
  ConnectObjectPropertiesDestroyStruct *destroy_struct = NULL ;

  destroy_struct = g_malloc0 (sizeof (ConnectObjectPropertiesDestroyStruct)) ;

  pszDataName = g_strdup_printf (CONNECT_OBJECT_PROPERTIES_DATA_KEY,
    (int)src, pszSrc, (int)dst, pszDst, (int)fn_forward, (int)data_forward, (int)destroy_forward, (int)fn_reverse, (int)data_reverse, (int)destroy_reverse) ;

  g_object_set_data (src, pszDataName, destroy_struct) ;
  g_object_set_data (dst, pszDataName, destroy_struct) ;

  g_free (pszDataName) ;

  // Forward connection
  if (NULL != fn_forward)
    {
    destroy_struct->src_entry.obj            = src ;
    destroy_struct->src_entry.destroy_notify = destroy_forward ;
    destroy_struct->src_entry.connect_struct = g_malloc0 (sizeof (ConnectObjectPropertiesStruct)) ;
    destroy_struct->src_entry.connect_struct->notify_dst       = dst ;
    destroy_struct->src_entry.connect_struct->pszDstProperty   = g_strdup (pszDst) ;
    destroy_struct->src_entry.connect_struct->connect_function = fn_forward ;
    destroy_struct->src_entry.connect_struct->connect_data     = data_forward ;
    psz = g_strdup_printf ("notify::%s", pszSrc) ;
    destroy_struct->src_entry.notify_id      = g_signal_connect (src, psz, (GCallback)connect_object_properties_notify, destroy_struct->src_entry.connect_struct) ;
    g_free (psz) ;
    }

  // Reverse connection
  if (NULL != fn_reverse)
    {
    destroy_struct->dst_entry.obj            = dst ;
    destroy_struct->dst_entry.destroy_notify = destroy_reverse ;
    destroy_struct->dst_entry.connect_struct = g_malloc0 (sizeof (ConnectObjectPropertiesStruct)) ;
    destroy_struct->dst_entry.connect_struct->notify_dst       = src ;
    destroy_struct->dst_entry.connect_struct->pszDstProperty   = g_strdup (pszSrc) ;
    destroy_struct->dst_entry.connect_struct->connect_function = fn_reverse ;
    destroy_struct->dst_entry.connect_struct->connect_data     = data_reverse ;
    psz = g_strdup_printf ("notify::%s", pszDst) ;
    destroy_struct->dst_entry.notify_id      = g_signal_connect (dst, psz, (GCallback)connect_object_properties_notify, destroy_struct->dst_entry.connect_struct) ;
    g_free (psz) ;
    }

  if (NULL != destroy_struct->src_entry.obj)
    {
//    fprintf (stderr, "connect_properties::weak_ref:<0x%x>(%s) connect_object_properties_break_connections(0x%x)\n",
//      (int)src, g_type_name (G_TYPE_FROM_INSTANCE (src)), (int)connect_object_properties_break_connections) ;
    g_object_weak_ref (src, connect_object_properties_break_connections, destroy_struct) ;
    }

  if (NULL != destroy_struct->dst_entry.obj)
    {
//    fprintf (stderr, "connect_properties::weak_ref:<0x%x>(%s) connect_object_properties_break_connections(0x%x)\n",
//      (int)dst, g_type_name (G_TYPE_FROM_INSTANCE (dst)), (int)connect_object_properties_break_connections) ;
    g_object_weak_ref (dst, connect_object_properties_break_connections, destroy_struct) ;
    }

  return TRUE ;
  }

void disconnect_object_properties (GObject *src, char *pszSrc, GObject *dst, char *pszDst, PropertyConnectFunction fn_forward, gpointer data_forward, GDestroyNotify destroy_forward, PropertyConnectFunction fn_reverse, gpointer data_reverse, GDestroyNotify destroy_reverse)
  {
  char *pszDataName = NULL ;
  ConnectObjectPropertiesDestroyStruct *destroy_struct = NULL ;

  pszDataName = g_strdup_printf (CONNECT_OBJECT_PROPERTIES_DATA_KEY,
    (int)src, pszSrc, (int)dst, pszDst, (int)fn_forward, (int)data_forward, (int)destroy_forward, (int)fn_reverse, (int)data_reverse, (int)destroy_reverse) ;
  destroy_struct = g_object_get_data (src, pszDataName) ;
  g_free (pszDataName) ;

  if (NULL == destroy_struct) 
    return ;

  // Unhook the connection from the source object
  if (NULL != destroy_struct->src_entry.obj)
    {
//    fprintf (stderr, "disconnect_properties::weak_unref:<0x%x>(%s) connect_object_properties_break_connections(0x%x)\n",
//      (int)(destroy_struct->src_entry.obj), g_type_name (G_TYPE_FROM_INSTANCE (destroy_struct->src_entry.obj)), (int)connect_object_properties_break_connections) ;
    g_object_weak_unref (destroy_struct->src_entry.obj, connect_object_properties_break_connections, destroy_struct) ;
    g_signal_handler_disconnect (destroy_struct->src_entry.obj, destroy_struct->src_entry.notify_id) ;
    }

  // Unhook the connection from the destination object
  if (NULL != destroy_struct->dst_entry.obj)
    {
//    fprintf (stderr, "disconnect_properties::weak_unref:<0x%x>(%s) connect_object_properties_break_connections(0x%x)\n",
//      (int)(destroy_struct->dst_entry.obj), g_type_name (G_TYPE_FROM_INSTANCE (destroy_struct->dst_entry.obj)), (int)connect_object_properties_break_connections) ;
    g_object_weak_unref (destroy_struct->dst_entry.obj, connect_object_properties_break_connections, destroy_struct) ;
    g_signal_handler_disconnect (destroy_struct->dst_entry.obj, destroy_struct->dst_entry.notify_id) ;
    }

  connect_object_properties_destroy_struct_free (destroy_struct) ;
  }

static void connect_object_properties_notify (GObject *obj_src, GParamSpec *param_spec_src, gpointer data)
  {
  GParamSpec *param_spec_dst = NULL ;
  ConnectObjectPropertiesStruct *connect_struct = (ConnectObjectPropertiesStruct *)data ;
  GValue val_src = {0, }, val_dst_should_be = {0, }, val_dst_is = {0, } ;

//  fprintf (stderr, "connect_object_properties_notify: <0x%08X>%s.%s -> <0x%08X>%s.%s\n",
//    (int)obj_src, g_type_name (G_TYPE_FROM_INSTANCE (obj_src)), param_spec_src->name,
//    (int)(connect_struct->notify_dst), g_type_name (G_TYPE_FROM_INSTANCE (connect_struct->notify_dst)), connect_struct->pszDstProperty) ;

  param_spec_dst = g_object_class_find_property (G_OBJECT_GET_CLASS (connect_struct->notify_dst), connect_struct->pszDstProperty) ;

  g_value_init (&val_src,           G_PARAM_SPEC_VALUE_TYPE (param_spec_src)) ;
  g_value_init (&val_dst_should_be, G_PARAM_SPEC_VALUE_TYPE (param_spec_dst)) ;
  g_value_init (&val_dst_is,        G_PARAM_SPEC_VALUE_TYPE (param_spec_dst)) ;

  // Grab the source property
  g_object_get_property (obj_src, g_param_spec_get_name (param_spec_src), &val_src) ;
  // Grab the val_dst from the rule
  (*(connect_struct->connect_function)) (&val_src, &val_dst_should_be, connect_struct->connect_data) ;
  // Grab the val_dst from the dst object
  g_object_get_property (connect_struct->notify_dst, g_param_spec_get_name (param_spec_dst), &val_dst_is) ;
  if (0 != g_param_values_cmp (param_spec_dst, &val_dst_should_be, &val_dst_is))
    g_object_set_property (connect_struct->notify_dst, connect_struct->pszDstProperty, &val_dst_should_be) ;
  }

static void connect_object_properties_break_connections (gpointer data, GObject *dead_object)
  {
  ConnectObjectPropertiesDestroyStruct *destroy_struct = (ConnectObjectPropertiesDestroyStruct *)data ;
  ConnectObjectPropertiesDestroyEntry *the_other_entry = NULL ;

  the_other_entry = ((dead_object == destroy_struct->src_entry.obj)
    ? &(destroy_struct->dst_entry)
    : &(destroy_struct->src_entry)) ;

  // Unhook the connection from the other object
  if (NULL != the_other_entry->obj)
    {
    g_object_weak_unref (the_other_entry->obj, connect_object_properties_break_connections, data) ;
    g_signal_handler_disconnect (the_other_entry->obj, the_other_entry->notify_id) ;
    }

  connect_object_properties_destroy_struct_free (destroy_struct) ;
  }

static void connect_object_properties_destroy_struct_free (ConnectObjectPropertiesDestroyStruct *destroy_struct)
  {
  // Get rid of user-supplied data
  if (NULL != destroy_struct->src_entry.destroy_notify)
    (*(destroy_struct->src_entry.destroy_notify)) (destroy_struct->src_entry.connect_struct->connect_data) ;
  if (NULL != destroy_struct->dst_entry.destroy_notify)
    (*(destroy_struct->dst_entry.destroy_notify)) (destroy_struct->dst_entry.connect_struct->connect_data) ;

  // Get rid of the destroy_structure itself

  if (NULL != destroy_struct->src_entry.obj)
    {
    g_free (destroy_struct->src_entry.connect_struct->pszDstProperty) ;
    g_free (destroy_struct->src_entry.connect_struct) ;
    }
  if (NULL != destroy_struct->dst_entry.obj)
    {
    g_free (destroy_struct->dst_entry.connect_struct->pszDstProperty) ;
    g_free (destroy_struct->dst_entry.connect_struct) ;
    }
  g_free (destroy_struct) ;
  }

void CONNECT_OBJECT_PROPERTIES_ASSIGN (GValue *val_src, GValue *val_dst, gpointer data)
  {
  GValue val_src_copy = {0, } ;

  g_value_copy (val_src, g_value_init (&val_src_copy, G_VALUE_TYPE (val_src))) ;

  g_value_transform (&val_src_copy, val_dst) ;

  g_value_unset (&val_src_copy) ;
  }

void CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_IN_LIST_P (GValue *val_src, GValue *val_dst, gpointer data)
  {
  int Nix ;
  INT_IN_LIST_PARAMS *iilp = (INT_IN_LIST_PARAMS *)data ;
  GValue val_int = {0, } ;
  int int_val ;

  g_value_transform (val_src, g_value_init (&val_int, G_TYPE_INT)) ;
  int_val = g_value_get_int (&val_int) ;

  for (Nix = 0 ; Nix < iilp->icInts ; Nix++)
    if ((iilp->ints)[Nix] == int_val)
      break ;

  g_value_set_boolean (val_dst, (Nix < iilp->icInts)) ;
  }

void CONNECT_OBJECT_PROPERTIES_ASSIGN_INVERT_BOOLEAN (GValue *val_src, GValue *val_dst, gpointer data)
  {g_value_set_boolean (val_dst, !g_value_get_boolean (val_src)) ;}

void CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_FROM_BOOLEAN_DATA (GValue *val_src, GValue *val_dst, gpointer data)
  {
  if (G_TYPE_INT == G_VALUE_TYPE (val_dst))
    g_value_set_int (val_dst, ((int *)data)[g_value_get_boolean (val_src) ? 0 : 1]) ;
  else
  if (G_TYPE_UINT == G_VALUE_TYPE (val_dst))
    g_value_set_uint (val_dst, ((guint *)data)[g_value_get_boolean (val_src) ? 0 : 1]) ;
  }
