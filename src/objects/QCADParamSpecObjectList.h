#ifndef _OBJECTS_QCADParamSpecObjectList_H_
#define _OBJECTS_QCADParamSpecObjectList_H_

#include <glib-object.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADParamSpecObjectList QCADParamSpecObjectList ;

GType qcad_object_list_get_type () ;

#define QCAD_TYPE_STRING_OBJECT_LIST "QCADObjectList"
#define QCAD_TYPE_OBJECT_LIST (qcad_object_list_get_type ())

GParamSpec *qcad_param_spec_object_list (const char *name, const char *nick, const char *blurb, GParamFlags flags) ;

GType qcad_param_spec_object_list_get_type () ;

struct _QCADParamSpecObjectList
  {
  GParamSpecPointer parent_instance ;
  } ;

#define QCAD_TYPE_STRING_PARAM_SPEC_OBJECT_LIST "QCADParamSpecObjectList"
#define QCAD_TYPE_PARAM_SPEC_OBJECT_LIST (qcad_param_spec_object_list_get_type ())
#define QCAD_PARAM_SPEC_OBJECT_LIST(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), QCAD_TYPE_PARAM_SPEC_OBJECT_LIST, QCADParamSpecObjectList))
#define QCAD_IS_PARAM_SPEC_OBJECT_LIST(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), QCAD_TYPE_PARAM_SPEC_OBJECT_LIST))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECTS_QCADParamSpecObjectList_H_ */
