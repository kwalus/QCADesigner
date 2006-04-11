#ifndef _OBJECTS_QCADLayersCombo_H_
#define _OBJECTS_QCADLayersCombo_H_

#include "QCADFlexiCombo.h"
#include "QCADLayer.h"

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADLayersCombo      QCADLayersCombo ;
typedef struct _QCADLayersComboClass QCADLayersComboClass ;

struct _QCADLayersCombo
  {
  QCADFlexiCombo parent_instance ;
  } ;

struct _QCADLayersComboClass
  {
  QCADFlexiComboClass parent_class ;
  gboolean (*deactivate_layer) (QCADLayersCombo *layers_combo, QCADLayer *layer, gpointer data) ;
  } ;

GType qcad_layers_combo_get_type () ;

#define QCAD_TYPE_STRING_LAYERS_COMBO "QCADLayersCombo"
#define QCAD_TYPE_LAYERS_COMBO (qcad_layers_combo_get_type ())
#define QCAD_LAYERS_COMBO(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_LAYERS_COMBO, QCADLayersCombo))
#define QCAD_IS_LAYERS_COMBO(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_LAYERS_COMBO))
#define QCAD_LAYERS_COMBO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_LAYERS_COMBO, QCADLayersComboClass))
#define QCAD_LAYERS_COMBO_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_LAYERS_COMBO, QCADLayersComboClass))
#define QCAD_IS_LAYERS_COMBO_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_LAYERS_COMBO))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def _OBJECTS_QCADLayersCombo_H_ */
