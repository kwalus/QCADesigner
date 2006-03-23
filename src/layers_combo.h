#ifndef _LAYERS_COMBO_H_
#define _LAYERS_COMBO_H_

#include "design.h"

typedef struct
  {
  GtkWidget *cb ;
  GtkWidget *tv ;
  GtkCellRenderer *crVisible ;
  GtkCellRenderer *crActive ;
  } LAYERS_COMBO ;

GtkWidget *layers_combo_new (LAYERS_COMBO *layers_combo) ;
GtkTreeModel *layers_combo_set_design (LAYERS_COMBO *layers_combo, DESIGN *design) ;
void layers_combo_select_layer (LAYERS_COMBO *layers_combo, QCADLayer *layer) ;
void layers_combo_remove_layer (LAYERS_COMBO *layers_combo, QCADLayer *layer) ;

#endif /* def _LAYERS_COMBO_H_ */
