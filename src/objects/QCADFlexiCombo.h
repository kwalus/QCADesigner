#ifndef _OBJECTS_QCADFlexiCombo_H_
#define _OBJECTS_QCADComboBox_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADFlexiCombo      QCADFlexiCombo ;
typedef struct _QCADFlexiComboClass QCADFlexiComboClass ;

struct _QCADFlexiCombo
  {
  GtkComboBoxEntry parent_instance ;
//  GtkBin parent_instance ;
  } ;

struct _QCADFlexiComboClass
  {
  GtkComboBoxEntryClass parent_class ;
//  GtkBinClass parent_class ;
  } ;

GType qcad_flexi_combo_get_type () ;
void qcad_flexi_combo_show_popup (QCADFlexiCombo *flexi_combo, gboolean bShow) ;

#define QCAD_TYPE_STRING_FLEXI_COMBO "QCADFlexiCombo"
#define QCAD_TYPE_FLEXI_COMBO (qcad_flexi_combo_get_type ())
#define QCAD_FLEXI_COMBO(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_FLEXI_COMBO, QCADFlexiCombo))
#define QCAD_IS_FLEXI_COMBO(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_FLEXI_COMBO))
#define QCAD_FLEXI_COMBO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_FLEXI_COMBO, QCADFlexiComboClass))
#define QCAD_FLEXI_COMBO_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_FLEXI_COMBO, QCADFlexiComboClass))
#define QCAD_IS_FLEXI_COMBO_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_FLEXI_COMBO))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADFlexiCombo_H_ */
