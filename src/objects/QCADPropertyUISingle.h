#ifndef _OBJECTS_QCADPropertyUISingle_H_
#define _OBJECTS_QCADPropertyUISingle_H_

#include <stdarg.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "../exp_array.h"
#include "QCADPropertyUI.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUISingle      QCADPropertyUISingle ;
typedef struct _QCADPropertyUISingleClass QCADPropertyUISingleClass ;
#ifdef GTK_GUI
typedef struct _QCADPropertyUIWidget      QCADPropertyUIWidget ;

struct _QCADPropertyUIWidget
  {
  GtkWidget *widget ;
  int idxX ;
  int idxY ;
  } ;
#endif /* def GTK_GUI */

struct _QCADPropertyUISingle
  {
  QCADPropertyUI parent_instance ;

  GParamSpec *pspec ;
  gboolean bShowLbl ;
#ifdef GTK_GUI
  QCADPropertyUIWidget lbl ;
  GtkTooltips *tooltip ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUISingleClass
  {
  QCADPropertyUIClass parent_class ;

  void (*set_pspec) (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec) ;
#ifdef GTK_GUI
  void (*set_tooltip) (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;
#endif /* def GTK_GUI */
  } ;

GType qcad_property_ui_single_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_SINGLE "QCADPropertyUISingle"
#define QCAD_TYPE_PROPERTY_UI_SINGLE (qcad_property_ui_single_get_type ())
#define QCAD_PROPERTY_UI_SINGLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingle))
#define QCAD_IS_PROPERTY_UI_SINGLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_SINGLE))
#define QCAD_PROPERTY_UI_SINGLE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingleClass))
#define QCAD_PROPERTY_UI_SINGLE_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_SINGLE, QCADPropertyUISingleClass))
#define QCAD_IS_PROPERTY_UI_SINGLE_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_SINGLE))

// Creates a new property UI for an instance:
// instance - self-explanatory
// property - the property to create the UI for
// ...      - name, value, ..., NULL pairs of properties to set for the new UI
QCADPropertyUI *qcad_property_ui_single_new (GObject *instance, char *property, ...) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUI_H_ */
