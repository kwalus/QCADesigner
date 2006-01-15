#include "QCADParamSpecObjectList.h"

static void instance_init (GParamSpec *pspec) ;
static void set_default (GParamSpec *pspec, GValue *value) ;
static gboolean value_validate (GParamSpec *pspec, GValue *value) ;
static gint values_cmp (GParamSpec *pspec, const GValue *value1, const GValue *value2) ;
static void finalize (GParamSpec *pspec) ;

GType qcad_object_list_get_type ()
  {
  static GType qcad_object_list_type = 0 ;

  if (0 == qcad_object_list_type)
    qcad_object_list_type = g_pointer_type_register_static (QCAD_TYPE_STRING_OBJECT_LIST) ;

  return qcad_object_list_type ;
  }

GType qcad_param_spec_object_list_get_type ()
  {
  static GType qcad_param_spec_object_list_type = 0 ;

  if (0 == qcad_param_spec_object_list_type)
    {
    static GParamSpecTypeInfo qcad_param_spec_object_list_info =
      {
      sizeof (QCADParamSpecObjectList),
      1,
      instance_init,
      0,
      finalize,
      set_default,
      value_validate,
      values_cmp
      } ;
    qcad_param_spec_object_list_info.value_type = QCAD_TYPE_OBJECT_LIST ;
    qcad_param_spec_object_list_type = g_param_type_register_static (QCAD_TYPE_STRING_PARAM_SPEC_OBJECT_LIST, &qcad_param_spec_object_list_info) ;
    }
  return qcad_param_spec_object_list_type ;
  }

GParamSpec *qcad_param_spec_object_list (const char *name, const char *nick, const char *blurb, GParamFlags flags)
  {return g_param_spec_internal (QCAD_TYPE_PARAM_SPEC_OBJECT_LIST, name, nick, blurb, flags) ;}

static void instance_init (GParamSpec *pspec) {}
static void set_default (GParamSpec *pspec, GValue *value)
  {g_value_set_pointer (value, NULL) ;}

static gboolean value_validate (GParamSpec *pspec, GValue *value)
  {return G_VALUE_HOLDS (value, QCAD_TYPE_OBJECT_LIST) ;}

static gint values_cmp (GParamSpec *pspec, const GValue *value1, const GValue *value2)
  {
  GType type1 = 0, type2 = 0 ;
  GList *llItr1 = NULL, *llItr2 = NULL ;
  
  for (llItr1 = g_value_get_pointer (value1), llItr2 = g_value_get_pointer (value2) ; 
       !(NULL == llItr1 || NULL == llItr2) ; 
       llItr1 = llItr1->next, llItr2 = llItr2->next)
    {
    if (NULL == llItr1->data && NULL == llItr2->data) continue ;
    if (NULL == llItr1->data && NULL != llItr2->data) return -1 ;
    if (NULL != llItr1->data && NULL == llItr2->data) return  1 ;

    type1 = G_TYPE_FROM_INSTANCE (llItr1->data) ;
    type2 = G_TYPE_FROM_INSTANCE (llItr2->data) ;

    if (type1 == type2) continue ;
    if (type1  < type2) return -1 ;
    if (type1  > type2) return  1 ;
    }

  if (NULL == llItr1 && NULL != llItr2) return -1 ;
  if (NULL != llItr1 && NULL == llItr2) return  1 ;

  return  0 ;
  }

static void finalize (GParamSpec *pspec) {}
