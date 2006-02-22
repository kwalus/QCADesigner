#ifndef _OBJECT_QCADToggleToolButton_H_
#define _OBJECT_QCADToggleToolButton_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADToggleToolButton      QCADToggleToolButton ;
typedef struct _QCADToggleToolButtonClass QCADToggleToolButtonClass ;

struct _QCADToggleToolButton
  {
  GtkToggleToolButton parent_instance ;
  } ;

struct _QCADToggleToolButtonClass
  {
  GtkToggleToolButtonClass parent_class ;
  } ;

GType qcad_toggle_tool_button_get_type () ;

#define QCAD_TYPE_STRING_TOGGLE_TOOL_BUTTON "QCADToggleToolButton"
#if (GTK_MINOR_VERSION < 9)
  #define QCAD_TYPE_TOGGLE_TOOL_BUTTON (qcad_toggle_tool_button_get_type ())
#else
  #define QCAD_TYPE_TOGGLE_TOOL_BUTTON GTK_TYPE_TOGGLE_TOOL_BUTTON
#endif
#define QCAD_TOGGLE_TOOL_BUTTON(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButton))
#define QCAD_IS_TOGGLE_TOOL_BUTTON(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON))
#define QCAD_TOGGLE_TOOL_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButtonClass))
#define QCAD_TOGGLE_TOOL_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButtonClass))
#define QCAD_IS_TOGGLE_TOOL_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_TOGGLE_TOOL_BUTTON))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECT_QCADToggleToolButton_H_ */
