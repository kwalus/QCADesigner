#ifndef _OBJECTS_QCADPropertyUIText_H_
#define _OBJECTS_QCADPropertyUIText_H_

#include "../exp_array.h"
#include "QCADPropertyUISingle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUIText      QCADPropertyUIText ;
typedef struct _QCADPropertyUITextClass QCADPropertyUITextClass ;

struct _QCADPropertyUIText
  {
  QCADPropertyUISingle parent_instance ;
#ifdef GTK_GUI
  QCADPropertyUIWidget entry ;
#endif /* def GTK_GUI */
  } ;

struct _QCADPropertyUITextClass
  {
  QCADPropertyUISingleClass parent_class ;
  } ;

GType qcad_property_ui_text_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI_TEXT "QCADPropertyUIText"
#define QCAD_TYPE_PROPERTY_UI_TEXT (qcad_property_ui_text_get_type ())
#define QCAD_PROPERTY_UI_TEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI_TEXT, QCADPropertyUIText))
#define QCAD_IS_PROPERTY_UI_TEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI_TEXT))
#define QCAD_PROPERTY_UI_TEXT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI_TEXT, QCADPropertyUITextClass))
#define QCAD_PROPERTY_UI_TEXT_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI_TEXT, QCADPropertyUITextClass))
#define QCAD_IS_PROPERTY_UI_TEXT_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI_TEXT))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUIText_H_ */
