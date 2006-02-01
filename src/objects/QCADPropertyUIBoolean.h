#ifndef _OBJECTS_QCADPropertyUIBoolean_H_
#define _OBJECTS_QCADPropertyUIBoolean_H_

#include "QCADPropertyUISingle.h"
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIBoolean      QCADPropertyUIBoolean ;
typedef struct _QCADPropertyUIBooleanClass QCADPropertyUIBooleanClass ;

struct _QCADPropertyUIBoolean
  {
  QCADPropertyUISingle parent_instance ;
#ifdef GTK_GUI
  QCADPropertyUIWidget check_button ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUIBooleanClass
  {
  QCADPropertyUISingleClass parent_class ;
  } ;

GType qcad_property_ui_boolean_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_BOOLEAN "QCADPropertyUIBOOLEAN"
#define QCAD_TYPE_PROPERTY_UI_BOOLEAN (qcad_property_ui_boolean_get_type ())
#define QCAD_PROPERTY_UI_BOOLEAN(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_BOOLEAN, QCADPropertyUIBoolean))
#define QCAD_IS_PROPERTY_UI_BOOLEAN(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_BOOLEAN))
#define QCAD_PROPERTY_UI_BOOLEAN_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_BOOLEAN, QCADPropertyUIBooleanClass))
#define QCAD_PROPERTY_UI_BOOLEAN_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_BOOLEAN, QCADPropertyUIBooleanClass))
#define QCAD_IS_PROPERTY_UI_BOOLEAN_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_BOOLEAN))

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* def _OBJECTS_QCADPropertyUIBoolean_H_ */
