#ifndef _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_
#define _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_

#include <gtk/gtk.h>

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data) ;
void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data) ;
void toggle_scale_mode (GtkWidget *widget, gpointer user_data) ;
void fill_printed_objects_list (GtkWidget *list, print_properties_D *dialog) ;
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data) ;
void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data) ;
void user_wants_print_preview (GtkWidget *widget, gpointer user_data) ;
void units_changed (GtkWidget *widget, gpointer user_data) ;

#endif /* _PRINT_PROPERTIES_DIALOG_CALLBACKS_H_*/
