#ifndef _OBJECTS_QCADParamSpecTypeList_H_
#define _OBJECTS_QCADParamSpecTypeList_H_

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _QCADParamSpecTypeList QCADParamSpecTypeList ;

struct _QCADParamSpecTypeList
  {
  GParamSpec parent_instance ;
  GType default_type ;
  int ic_types_in_list ;
  GList *type_list ;
  } ;

GType qcad_param_spec_type_list_get_type () ;
GParamSpec *qcad_param_spec_type_list (const char *name, const char *nick, const char *blurb, GType default_type, GParamFlags flags, GType first_type_in_list, ...) ;

#define QCAD_TYPE_STRING_PARAM_SPEC_TYPE_LIST "QCADParamSpecTypeList"
#define QCAD_TYPE_PARAM_SPEC_TYPE_LIST (qcad_param_spec_type_list_get_type ())
#define QCAD_PARAM_SPEC_TYPE_LIST(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), QCAD_TYPE_PARAM_SPEC_TYPE_LIST, QCADParamSpecTypeList))
#define QCAD_IS_PARAM_SPEC_TYPE_LIST(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), QCAD_TYPE_PARAM_SPEC_TYPE_LIST))

G_END_DECLS

#endif /* def _OBJECTS_QCADParamSpecTypeList_H_ */
