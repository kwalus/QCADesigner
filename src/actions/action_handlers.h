#ifndef _ACTION_HANDLERS_H_
#define _ACTION_HANDLERS_H_

#include <gtk/gtk.h>
#include "../interface.h"
#include "../callback_helpers.h"
#include "../cad.h"

typedef void (*ActionHandler) (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;

void run_action_SINGLE_CELL (MOUSE_HANDLERS *pmh, GtkWidget *drawing_area, GdkGC *global_gc, DESIGN *pDesign, project_OP *pProjectOpt, main_W *wndMain) ;
void run_action_ROTATE (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;
void run_action_MIRROR (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;
void run_action_ARRAY (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;
void run_action_PAN (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;
void run_action_DELETE (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;
void run_action_CELL_FUNCTION (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window) ;

void run_action_DEFAULT (MOUSE_HANDLERS *pmh, GtkWidget *drawing_area, GdkGC *global_gc, DESIGN *pDesign, project_OP *pProjectOpt, main_W *wndMain) ;
void run_action_DEFAULT_sel_changed () ;

#endif /* _ACTION_HANDLERS_H_ */
