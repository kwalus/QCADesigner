#ifndef _OBJECTS_QCADPropertyUI_H_
#define _OBJECTS_QCADPropertyUI_H_

#include <stdarg.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "../exp_array.h"
#include "../generic_utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUI          QCADPropertyUI ;
typedef struct _QCADPropertyUIClass     QCADPropertyUIClass ;
typedef struct _QCADPropertyUIBehaviour QCADPropertyUIBehaviour ;
typedef struct _QCADPropertyUIProperty  QCADPropertyUIProperty ;

struct _QCADPropertyUIBehaviour
  {
  char *instance_property_name1 ;
  char *ui_property_name1 ;
  char *instance_property_name2 ;
  char *ui_property_name2 ;
  PropertyConnectFunction fn_forward ;
  gpointer data_forward ;
  GDestroyNotify destroy_forward ;
  PropertyConnectFunction fn_reverse ;
  gpointer data_reverse ;
  GDestroyNotify destroy_reverse ;
  } ;

struct _QCADPropertyUIProperty
  {
  char *instance_property_name ;
  char *ui_property_name ;
  GValue ui_property_value ;
  } ;

struct _QCADPropertyUI
  {
  GObject parent_instance ;

  int cxWidgets ;
  int cyWidgets ;
  gboolean bSensitive ;
  gboolean bVisible ;
  GObject *instance ;
  } ;

struct _QCADPropertyUIClass
  {
  GObjectClass parent_class ;

  gboolean (*set_instance) (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
#ifdef GTK_GUI
  GtkWidget *(*get_widget) (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
#endif /* def GTK_GUI */
  void (*set_visible) (QCADPropertyUI *property_ui, gboolean bVisible) ;
  void (*set_sensitive) (QCADPropertyUI *property_ui, gboolean bSensitive) ;
  } ;

GType qcad_property_ui_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI "QCADPropertyUI"
#define QCAD_TYPE_PROPERTY_UI (qcad_property_ui_get_type ())
#define QCAD_PROPERTY_UI(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI, QCADPropertyUI))
#define QCAD_IS_PROPERTY_UI(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI))
#define QCAD_PROPERTY_UI_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI, QCADPropertyUIClass))
#define QCAD_PROPERTY_UI_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI, QCADPropertyUIClass))
#define QCAD_IS_PROPERTY_UI_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI))

QCADPropertyUI *qcad_property_ui_new (GObject *instance, char *property_name, ...) ;
void qcad_property_ui_set_instance (QCADPropertyUI *property_ui, GObject *instance) ;
int qcad_property_ui_get_cx_widgets (QCADPropertyUI *property_ui) ;
int qcad_property_ui_get_cy_widgets (QCADPropertyUI *property_ui) ;
#ifdef GTK_GUI
GtkWidget *qcad_property_ui_get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
#endif /* def GTK_GUI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUI_H_ */
