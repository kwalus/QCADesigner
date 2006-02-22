#ifndef _OBJECTS_QCADRadioButton_H_
#define _OBJECTS_QCADRadioButton_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADRadioButton      QCADRadioButton ;
typedef struct _QCADRadioButtonClass QCADRadioButtonClass ;

struct _QCADRadioButton
  {
  GtkRadioButton parent_instance ;
  } ;

struct _QCADRadioButtonClass
  {
  GtkRadioButtonClass parent_class ;
  } ;

GType qcad_radio_button_get_type () ;

#define QCAD_TYPE_STRING_RADIO_BUTTON "QCADRadioButton"
#if (GTK_MINOR_VERSION < 9)
  #define QCAD_TYPE_RADIO_BUTTON (qcad_radio_button_get_type ())
#else
  #define QCAD_TYPE_RADIO_BUTTON GTK_TYPE_RADIO_BUTTON
#endif
#define QCAD_RADIO_BUTTON(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_RADIO_BUTTON, QCADRadioButton))
#define QCAD_IS_RADIO_BUTTON(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_RADIO_BUTTON)
#define QCAD_RADIO_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_RADIO_BUTTON, QCADRadioButtonClass))
#define QCAD_RADIO_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_RADIO_BUTTON, QCADRadioButtonClass))
#define QCAD_IS_RADIO_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_RADIO_BUTTON))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECTS_QCADRadioButton_H_ */
