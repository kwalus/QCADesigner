#include <gtk/gtk.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "../undo_create.h"
#include "../callback_helpers.h"
#include "action_handlers.h"

extern cell_OP cell_options ;

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

void run_action_SINGLE_CELL (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
  {
  
  global_gc = gc ;
  pDesign = design ;
  pProjectOpts = options ;
  wndMain = main_window ;

  pmh->lIDButtonPressed = g_signal_connect (G_OBJECT (widget), "button_press_event", (GCallback)button_press_event, NULL) ;
  pmh->lIDMotionNotify = -1 ;
  pmh->lIDButtonReleased = g_signal_connect (G_OBJECT (widget), "button_release_event", (GCallback)button_release_event, NULL) ;
  }

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  double x, y ;
  GQCell *cellp = NULL ;
  if (1 != event->button) return FALSE ;
  
  x = grid_world_x (calc_world_x (event->x));
  y = grid_world_y (calc_world_y (event->y));

  if (NULL != (cellp = add_stdqcell(
    (GQCell **)(&((pDesign->first_layer->first_obj))), 
    (GQCell **)(&((pDesign->first_layer->last_obj))), 
    NULL, TRUE, x, y, &cell_options, pProjectOpts->selected_cell_type)))
    {
    gui_add_to_undo (Undo_CreateAction_CreateCells (&cellp, 1)) ;

    pProjectOpts->bDesignAltered = TRUE ;
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
