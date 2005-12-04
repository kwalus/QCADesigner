#include <stdarg.h>
#include "QCADParamSpecTypeList.h"

static void instance_init (GParamSpec *pspec) ;
static void set_default (GParamSpec *pspec, GValue *value) ;
static gboolean value_validate (GParamSpec *pspec, GValue *value) ;
static gint values_cmp (GParamSpec *pspec, const GValue *value1, const GValue *value2) ;
static void finalize (GParamSpec *pspec) ;

GType qcad_param_spec_type_list_get_type ()
  {
  static GType qcad_param_spec_type_list_type = 0 ;

  if (0 == qcad_param_spec_type_list_type)
    {
    static GParamSpecTypeInfo qcad_param_spec_type_list_info =
      {
      sizeof (QCADParamSpecTypeList),
      1,
      instance_init,
      G_TYPE_UINT,
      finalize,
      set_default,
      value_validate,
      values_cmp
      } ;
    qcad_param_spec_type_list_type = g_param_type_register_static (QCAD_TYPE_STRING_PARAM_SPEC_TYPE_LIST, &qcad_param_spec_type_list_info) ;
    }
  return qcad_param_spec_type_list_type ;
  }

GParamSpec *qcad_param_spec_type_list (const char *name, const char *nick, const char *blurb, GType default_type, GParamFlags flags, GType first_type_in_list, ...)
  {
  GType next_type ;
  va_list va ;
  QCADParamSpecTypeList *pspec = QCAD_PARAM_SPEC_TYPE_LIST (g_param_spec_internal (QCAD_TYPE_PARAM_SPEC_TYPE_LIST, name, nick, blurb, flags)) ;

  pspec->default_type = default_type ;
  pspec->type_list = g_list_prepend (pspec->type_list, (gpointer)first_type_in_list) ;

  va_start (va, first_type_in_list) ;
  while (0 != (next_type = va_arg (va, GType)))
    g_list_prepend (pspec->type_list, (gpointer)next_type) ;
  va_end (va) ;

  return G_PARAM_SPEC (pspec) ;
  }

static void instance_init (GParamSpec *pspec)
  {
  QCADParamSpecTypeList *param_spec_type_list = QCAD_PARAM_SPEC_TYPE_LIST (pspec) ;

  param_spec_type_list->default_type     = 0 ;
  param_spec_type_list->ic_types_in_list = 0 ;
  param_spec_type_list->type_list        = NULL ;
  }

static void set_default (GParamSpec *pspec, GValue *value)
  {g_value_set_uint (value, (guint)(QCAD_PARAM_SPEC_TYPE_LIST (pspec)->default_type)) ;}

static gboolean value_validate (GParamSpec *pspec, GValue *value)
  {
  GList *llItr = NULL ;
  QCADParamSpecTypeList *param_spec_type_list = QCAD_PARAM_SPEC_TYPE_LIST (pspec) ;
  GType val ;

  if (NULL == param_spec_type_list->type_list) return FALSE ;

  val = g_value_get_uint (value) ;
  for (llItr = param_spec_type_list->type_list ; llItr != NULL ; llItr = llItr->next)
    if (val == ((GType)(llItr->data)))
      return TRUE ;

  return FALSE ;
  }

static gint values_cmp (GParamSpec *pspec, const GValue *value1, const GValue *value2)
  {return (gint)(!(g_value_get_uint (value1) == g_value_get_uint (value1))) ;}

static void finalize (GParamSpec *pspec)
  {
  g_list_free (QCAD_PARAM_SPEC_TYPE_LIST (pspec)->type_list) ;
  G_PARAM_SPEC_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PARAM_SPEC_TYPE_LIST)))->finalize (pspec) ;
  }
