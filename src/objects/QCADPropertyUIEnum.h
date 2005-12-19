#ifndef _OBJECTS_QCADPropertyUIEnum_H_
#define _OBJECTS_QCADPropertyUIEnum_H_

#include <gtk/gtk.h>
#include "QCADPropertyUISingle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIEnum      QCADPropertyUIEnum ;
typedef struct _QCADPropertyUIEnumClass QCADPropertyUIEnumClass ;

struct _QCADPropertyUIEnum
  {
  QCADPropertyUISingle parent_instance ;

  QCADPropertyUIWidget option_menu ;
  QCADPropertyUIWidget frame ;

  GtkWidget *tbl ;
  GtkWidget *rb ;

  GType render_as ;
  guint notify_id ;
  } ;

struct _QCADPropertyUIEnumClass
  {
  QCADPropertyUISingleClass parent_class ;
  } ;

GType qcad_property_ui_enum_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_ENUM "QCADPropertyUIEnum"
#define QCAD_TYPE_PROPERTY_UI_ENUM (qcad_property_ui_enum_get_type ())
#define QCAD_PROPERTY_UI_ENUM(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_ENUM, QCADPropertyUIEnum))
#define QCAD_PROPERTY_UI_ENUM_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), QCAD_TYPE_PROPERTY_UI_ENUM_CLASS, QCADPropertyUIEnumClass))
#define QCAD_IS_PROPERTY_UI_ENUM(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_ENUM))
#define QCAD_IS_PROPERTY_UI_ENUM_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), QCAD_TYPE_PROPERTY_UI_ENUM))
#define QCAD_PROPERTY_UI_UI_ENUM_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_PROPERTY_UI_ENUM, QCADPropertyUIEnumClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADPropertyUIEnum_H_ */
