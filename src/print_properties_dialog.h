#ifndef _PRINT_PROPERTIES_DIALOG_H_
#define _PRINT_PROPERTIES_DIALOG_H_

#include <gtk/gtk.h>
#include "print.h"

gboolean get_print_design_properties_from_user (GtkWindow *parent, print_design_OP *pPO, GQCell *first_cell) ;
void init_print_design_options (print_design_OP *pPO, GQCell *first_cell) ;


#endif /* _PRINT_PROPERTIES_DIALOG_H_ */
