#ifndef _OBJECT_QCADComboBox_H_
#define _OBJECT_QCADComboBox_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADComboBox      QCADComboBox ;
typedef struct _QCADComboBoxClass QCADComboBoxClass ;

struct _QCADComboBox
  {GtkComboBox parent_instance ;} ;

struct _QCADComboBoxClass
  {GtkComboBoxClass parent_class ;} ;

GType qcad_combo_box_get_type () ;

#define QCAD_TYPE_STRING_COMBO_BOX "QCADComboBox"
#if (GTK_MINOR_VERSION < 8)
  #define QCAD_TYPE_COMBO_BOX (qcad_combo_box_get_type ())
#else
  #define QCAD_TYPE_COMBO_BOX GTK_TYPE_COMBO_BOX
#endif /* GTK_MINOR_VERSION < 8) */
#define QCAD_COMBO_BOX(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_COMBO_BOX, QCADComboBox))
#define QCAD_IS_COMBO_BOX(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_COMBO_BOX))
#define QCAD_COMBO_BOX_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_COMBO_BOX, QCADComboBoxClass))
#define QCAD_COMBO_BOX_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_COMBO_BOX, QCADComboBoxClass))
#define QCAD_IS_COMBO_BOX_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_COMBO_BOX))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* ndef _OBJECT_QCADComboBox_H_ */
