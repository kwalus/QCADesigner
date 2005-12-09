#ifndef _OBJECTS_QCADPropertyUISingle_H_
#define _OBJECTS_QCADPropertyUISingle_H_

#include <stdarg.h>
#include <gtk/gtk.h>
#include "../exp_array.h"
#include "QCADPropertyUI.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUISingle      QCADPropertyUISingle ;
typedef struct _QCADPropertyUISingleClass QCADPropertyUISingleClass ;
typedef struct _QCADPropertyUIWidget      QCADPropertyUIWidget ;

struct _QCADPropertyUIWidget
  {
  GtkWidget *widget ;
  int idxX ;
  int idxY ;
  } ;

struct _QCADPropertyUISingle
  {
  QCADPropertyUI parent_instance ;

  gboolean bShowLbl ;
  QCADPropertyUIWidget lbl ;
  } ;

struct _QCADPropertyUISingleClass
  {
  QCADPropertyUIClass parent_class ;

  void (*set_pspec) (QCADPropertyUISingle *property_ui_single) ;
  } ;

GType qcad_property_ui_single_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_SINGLE "QCADPropertyUISingle"
#define QCAD_TYPE_PROPERTY_UI_SINGLE (qcad_property_ui_single_get_type ())
#define QCAD_PROPERTY_UI_SINGLE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingle))
#define QCAD_PROPERTY_UI_SINGLE_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingleClass))
#define QCAD_IS_PROPERTY_UI_SINGLE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_SINGLE))
#define QCAD_IS_PROPERTY_UI_SINGLE_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), QCAD_TYPE_PROPERTY_UI_SINGLE))
#define QCAD_PROPERTY_UI_SINGLE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingleClass))

// Creates a new property UI for an instance:
// instance - self-explanatory
// property - the property to create the UI for
// ...      - name, value, ..., NULL pairs of properties to set for the new UI
QCADPropertyUI *qcad_property_ui_single_new (GObject *instance, char *property, ...) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUI_H_ */
