#ifndef _OBJECTS_QCADPropertyUINumeric_H_
#define _OBJECTS_QCADPropertyUINumeric_H_

#include "QCADPropertyUISingle.h"
#include "../exp_array.h"
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUINumeric      QCADPropertyUINumeric ;
typedef struct _QCADPropertyUINumericClass QCADPropertyUINumericClass ;

struct _QCADPropertyUINumeric
  {
  QCADPropertyUISingle parent_instance ;
#ifdef GTK_GUI
  QCADPropertyUIWidget lblUnits ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUINumericClass
  {
  QCADPropertyUISingleClass parent_class ;
  } ;

GType qcad_property_ui_numeric_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_NUMERIC "QCADPropertyUINumeric"
#define QCAD_TYPE_PROPERTY_UI_NUMERIC (qcad_property_ui_numeric_get_type ())
#define QCAD_PROPERTY_UI_NUMERIC(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_NUMERIC, QCADPropertyUINumeric))
#define QCAD_IS_PROPERTY_UI_NUMERIC(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_NUMERIC))
#define QCAD_PROPERTY_UI_NUMERIC_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_NUMERIC, QCADPropertyUINumericClass))
#define QCAD_PROPERTY_UI_NUMERIC_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_NUMERIC, QCADPropertyUINumericClass))
#define QCAD_IS_PROPERTY_UI_NUMERIC_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_NUMERIC))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUINumeric_H_ */
