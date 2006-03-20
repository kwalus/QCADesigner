#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_

#include <gtk/gtk.h>
#include "design.h"
#include "vector_table.h"

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *sw ;
  GtkWidget *tv ;
  GtkActionGroup *vector_table_action_group ;
  GtkAction *actVectorTable ;
  GtkAction *actExhaustive ;
  GtkWidget *lblVectorCount ;
  GtkWidget *lblFileName ;
  GtkWidget *vector_ops ;
  } vector_table_options_D ;

typedef struct
  {
  BUS_LAYOUT *bus_layout ;
  int *sim_type_p ;
  VectorTable *pvt ;
  } VECTOR_TABLE_OPTIONS_DIALOG_DATA ;

void create_vector_table_options_dialog (vector_table_options_D *dialog) ;

#endif /* def _VECTOR_TABLE_OPTIONS_DIALOG_INTERFACE_H_ */
