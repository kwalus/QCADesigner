#ifndef _OBJECTS_QCADPropertyUIGroup_H_
#define _OBJECTS_QCADPropertyUIGroup_H_

#include <stdarg.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIGroup      QCADPropertyUIGroup ;
typedef struct _QCADPropertyUIGroupClass QCADPropertyUIGroupClass ;

struct _QCADPropertyUIGroup
  {
  QCADPropertyUI parent_instance ;
  EXP_ARRAY *property_uis ;
  } ;

struct _QCADPropertyUIGroupClass
  {
  QCADPropertyUIClass parent_class ;
  } ;

GType qcad_property_ui_group_get_type () ;
QCADPropertyUIGroup *

#define QCAD_TYPE_STRING_PROPERTY_UI_GROUP "QCADPropertyUIGroup"
#define QCAD_TYPE_PROPERTY_UI_GROUP (qcad_property_ui_group_get_type ())
#define QCAD_PROPERTY_UI_GROUP(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_GROUP, QCADPropertyUIGroup))
#define QCAD_PROPERTY_UI_GROUP_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), QCAD_TYPE_PROPERTY_UI_GROUP, QCADPropertyUIGroupClass))
#define QCAD_IS_PROPERTY_UI_GROUP(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_GROUP))
#define QCAD_IS_PROPERTY_UI_GROUP_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), QCAD_TYPE_PROPERTY_UI_GROUP))
#define QCAD_PROPERTY_UI_GROUP_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), QCAD_TYPE_PROPERTY_UI_GROUP, QCADPropertyUIGroupClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUIGroup_H_ */
