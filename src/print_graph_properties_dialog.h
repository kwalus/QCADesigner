#ifndef _PRINT_GRAPH_PROPERTIES_DIALOG_H_
#define _PRINT_GRAPH_PROPERTIES_DIALOG_H_

#include "print.h"
#include "simulation.h"
#include <gtk/gtk.h>

gboolean get_print_graph_properties_from_user (GtkWindow *parent, print_graph_OP *pPO, simulation_data *sim_data) ;
void init_print_graph_options (print_graph_OP *pPO, simulation_data *sim_data) ;


#endif /* _PRINT_GRAPH_PROPERTIES_DIALOG_H_ */
