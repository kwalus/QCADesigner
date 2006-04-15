#ifndef _OBJECT_QCADToggleToolButton_H_
#define _OBJECT_QCADToggleToolButton_H_

#include <gtk/gtk.h>

#if (GTK_MINOR_VERSION <= 8 || defined (GTK_DOC))
G_BEGIN_DECLS

typedef struct _QCADToggleToolButton      QCADToggleToolButton ;
typedef struct _QCADToggleToolButtonClass QCADToggleToolButtonClass ;
#else
  #define QCADToggleToolButton      GtkToggleToolButton
  #define QCADToggleToolButtonClass GtkToggleToolButtonClass
#endif

#if (GTK_MINOR_VERSION <= 8 || defined (GTK_DOC))
struct _QCADToggleToolButton
  {GtkToggleToolButton parent_instance ;} ;

struct _QCADToggleToolButtonClass
  {GtkToggleToolButtonClass parent_class ;} ;

GType qcad_toggle_tool_button_get_type () ;

  #define QCAD_TYPE_STRING_TOGGLE_TOOL_BUTTON "QCADToggleToolButton"
  #define QCAD_TYPE_TOGGLE_TOOL_BUTTON (qcad_toggle_tool_button_get_type ())
#else
  #define QCAD_TYPE_TOGGLE_TOOL_BUTTON GTK_TYPE_TOGGLE_TOOL_BUTTON
#endif
#if (GTK_MINOR_VERSION <= 8 || defined (GTK_DOC))
  #define QCAD_TOGGLE_TOOL_BUTTON(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButton))
  #define QCAD_IS_TOGGLE_TOOL_BUTTON(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON))
  #define QCAD_TOGGLE_TOOL_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButtonClass))
  #define QCAD_TOGGLE_TOOL_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_TOGGLE_TOOL_BUTTON, QCADToggleToolButtonClass))
  #define QCAD_IS_TOGGLE_TOOL_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_TOGGLE_TOOL_BUTTON))

G_END_DECLS
#endif

#endif /* def _OBJECT_QCADToggleToolButton_H_ */
