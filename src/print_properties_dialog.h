#ifndef _PRINT_PROPERTIES_DIALOG_H_
#define _PRINT_PROPERTIES_DIALOG_H_

#include <gtk/gtk.h>
#include "globals.h"

gboolean get_print_properties_from_user (GtkWindow *parent, print_OP *pPO, qcell *first_cell) ;
void init_print_options (print_OP *pPO, qcell *first_cell) ;


#endif /* _PRINT_PROPERTIES_DIALOG_H_ */
