#ifndef _OBJECTS_QCADPropertyUIInt_H_
#define _OBJECTS_QCADPropertyUIInt_H_

#include "QCADPropertyUINumeric.h"
#include "../exp_array.h"
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIInt      QCADPropertyUIInt ;
typedef struct _QCADPropertyUIIntClass QCADPropertyUIIntClass ;

struct _QCADPropertyUIInt
  {
  QCADPropertyUINumeric parent_instance ;
  QCADPropertyUIWidget spn ;
  GtkAdjustment *adj ;
  QCADPropertyUIWidget option_menu ;
  GType render_as ;
  guint notify_id ;
  } ;

struct _QCADPropertyUIIntClass
  {
  QCADPropertyUINumericClass parent_class ;
  } ;

GType qcad_property_ui_int_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_INT "QCADPropertyUIInt"
#define QCAD_TYPE_PROPERTY_UI_INT (qcad_property_ui_int_get_type ())
#define QCAD_PROPERTY_UI_INT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_INT, QCADPropertyUIInt))
#define QCAD_IS_PROPERTY_UI_INT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_INT))
#define QCAD_PROPERTY_UI_INT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_INT, QCADPropertyUIIntClass))
#define QCAD_PROPERTY_UI_INT_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_INT, QCADPropertyUIIntClass))
#define QCAD_IS_PROPERTY_UI_INT_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_INT))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUIInt_H_ */
