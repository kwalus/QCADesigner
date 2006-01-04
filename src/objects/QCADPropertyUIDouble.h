#ifndef _OBJECTS_QCADPropertyUIDouble_H_
#define _OBJECTS_QCADPropertyUIDouble_H_

#include "QCADPropertyUINumeric.h"
#include "../exp_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIDouble      QCADPropertyUIDouble ;
typedef struct _QCADPropertyUIDoubleClass QCADPropertyUIDoubleClass ;

struct _QCADPropertyUIDouble
  {
  QCADPropertyUINumeric parent_instance ;
#ifdef GTK_GUI
  QCADPropertyUIWidget spn ;
  GtkAdjustment *adj ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUIDoubleClass
  {
  QCADPropertyUINumericClass parent_class ;
  } ;

GType qcad_property_ui_double_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_DOUBLE "QCADPropertyUIDouble"
#define QCAD_TYPE_PROPERTY_UI_DOUBLE (qcad_property_ui_double_get_type ())
#define QCAD_PROPERTY_UI_DOUBLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_DOUBLE, QCADPropertyUIDouble))
#define QCAD_IS_PROPERTY_UI_DOUBLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_DOUBLE))
#define QCAD_PROPERTY_UI_DOUBLE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_DOUBLE, QCADPropertyUIDoubleClass))
#define QCAD_PROPERTY_UI_DOUBLE_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_DOUBLE, QCADPropertyUIDoubleClass))
#define QCAD_IS_PROPERTY_UI_DOUBLE_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_DOUBLE))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUIDouble_H_ */
