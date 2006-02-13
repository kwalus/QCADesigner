#ifndef _OBJECT_QCADRadioToolButton_H_
#define _OBJECT_QCADRaidoToolButton_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADRadioToolButton      QCADRadioToolButton ;
typedef struct _QCADRadioToolButtonClass QCADRadioToolButtonClass ;

struct _QCADRadioToolButton
  {
  GtkRadioToolButton parent_instance ;
  } ;

struct _QCADRadioToolButtonClass
  {
  GtkRadioToolButtonClass parent_class ;
  } ;

GType qcad_radio_tool_button_get_type () ;

#define QCAD_TYPE_STRING_RADIO_TOOL_BUTTON "QCADRadioToolButton"
#define QCAD_TYPE_RADIO_TOOL_BUTTON (qcad_radio_tool_button_get_type ())
#define QCAD_RADIO_TOOL_BUTTON(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_RADIO_TOOL_BUTTON, QCADRadioToolButton))
#define QCAD_IS_RADIO_TOOL_BUTTON(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_RADIO_TOOL_BUTTON))
#define QCAD_RADIO_TOOL_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_RADIO_TOOL_BUTTON, QCADRadioToolButtonClass))
#define QCAD_RADIO_TOOL_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_RADIO_TOOL_BUTTON, QCADRadioToolButtonClass))
#define QCAD_IS_RADIO_TOOL_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_RADIO_TOOL_BUTTON))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECT_QCADToggleToolButton_H_ */
