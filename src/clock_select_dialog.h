#ifndef _CLOCK_SELECT_DIALOG_H_
#define _CLOCK_SELECT_DIALOG_H_

#include <gtk/gtk.h>
#include "globals.h"

GtkWidget *create_clock_select_dialog (GtkWindow *parent) ;
void update_clock_select_dialog (int iClock) ;

#endif /* _CLOCK_SELECT_DIALOG_H_ */
