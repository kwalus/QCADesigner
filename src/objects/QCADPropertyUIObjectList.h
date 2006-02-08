#ifndef _OBJECTS_QCADPropertyUIObjectList_H_
#define _OBJECTS_QCADPropertyUIObjectList_H_

#include "QCADPropertyUISingle.h"

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADPropertyUIObjectList      QCADPropertyUIObjectList ;
typedef struct _QCADPropertyUIObjectListClass QCADPropertyUIObjectListClass ;

struct _QCADPropertyUIObjectList
  {
  QCADPropertyUISingle parent_instance ;
#ifdef GTK_GUI
  QCADPropertyUIWidget frame ;
  GtkWidget *tblObjects ;
  GtkWidget *tvObjects ;
  GtkWidget *sw_tv ;

  GList *llPUIs ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUIObjectListClass
  {
  QCADPropertyUISingleClass parent_class ;
  } ;

GType qcad_property_ui_object_list_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_OBJECT_LIST "QCADPropertyUIObjectList"
#define QCAD_TYPE_PROPERTY_UI_OBJECT_LIST (qcad_property_ui_object_list_get_type ())
#define QCAD_PROPERTY_UI_OBJECT_LIST(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_OBJECT_LIST, QCADPropertyUIObjectList))
#define QCAD_IS_PROPERTY_UI_OBJECT_LIST(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_OBJECT_LIST))
#define QCAD_PROPERTY_UI_OBJECT_LIST_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_OBJECT_LIST))
#define QCAD_PROPERTY_UI_OBJECT_LIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_PROPERTY_UI_OBJECT_LIST, QCADPropertyUIObjectListClass))
#define QCAD_IS_PROPERTY_UI_OBJECT_LIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_PROPERTY_UI_OBJECT_LIST))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECTS_QCADPropertyUIObjectList_H_ */
