#ifndef _GRAPH_OPTIONS_DIALOG_H_
#define _GRAPH_OPTIONS_DIALOG_H_

#include <gtk/gtk.h>
#include "simulation.h"

void get_graph_options_from_user (GtkWindow *parent, struct TRACEDATA *traces, int icTraces, struct TRACEDATA *clocks, int icClocks) ;

#endif /* _GRAPH_OPTIONS_DIALOG_H_ */
