#include <gtk/gtk.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "../undo_create.h"
#include "../callback_helpers.h"
#include "action_handlers.h"
#include "../cell_function_dialog.h"
#include "../vector_table.h"

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

void run_action_CELL_FUNCTION (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
  {
  global_gc = gc ;
  pDesign = design ;
  pProjectOpts = options ;
  wndMain = main_window ;

  pmh->lIDButtonPressed = g_signal_connect (G_OBJECT (widget), "button_press_event", (GCallback)button_press_event, NULL) ;
  pmh->lIDMotionNotify = -1 ;
  pmh->lIDButtonReleased = g_signal_connect (G_OBJECT (widget), "button_release_event", (GCallback)button_release_event, NULL) ;
  
  if (1 == pProjectOpts->number_of_selected_cells)
    {
    if (get_cell_function_from_user (wndMain->main_window, pProjectOpts->selected_cells[0]))
      {
      gui_add_to_undo (Undo_CreateAction_CellParamChange (pProjectOpts->selected_cells, 1)) ;
      VectorTable_update_inputs (pProjectOpts->pvt, pProjectOpts->selected_cells[0]) ;
      }
    release_selection () ;
    }
  }

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GQCell *cellp = NULL ;
  if (1 != event->button) return FALSE ;
  
  cellp = select_cell_at_coords ((GQCell *)(pDesign->first_layer->first_obj), calc_world_x (event->x), calc_world_y (event->y)) ;
  
  if (NULL != cellp)
    {
    if (pProjectOpts->number_of_selected_cells > 0)
      release_selection () ;
    gqcell_select (cellp) ;
    draw_stdqcell (widget->window, global_gc, cellp) ;
    if (get_cell_function_from_user (wndMain->main_window, cellp))
      {
      gui_add_to_undo (Undo_CreateAction_CellParamChange (&cellp, 1)) ;
      VectorTable_update_inputs (pProjectOpts->pvt, cellp) ;
      }
    gqcell_reset_colour (cellp) ;
    draw_stdqcell (widget->window, global_gc, cellp) ;
    }
  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (3 == event->button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wndMain->default_action_button), TRUE) ;
  return FALSE ;
  }
