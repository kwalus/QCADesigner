#include <gtk/gtk.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "../undo_create.h"
#include "action_handlers.h"

extern cell_OP cell_options ;

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

void run_action_DELETE (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
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
  GQCell *pqc = select_cell_at_coords ((GQCell *)(pDesign->first_layer->first_obj), calc_world_x (event->x), calc_world_y (event->y)) ;
  if (NULL != pqc)
    {
    gui_add_to_undo (Undo_CreateAction_DeleteCells (&pqc, 1)) ;
    gui_delete_cells (&pqc, 1) ;
    }
  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (3 == event->button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wndMain->default_action_button), TRUE) ;
  return FALSE ;
  }
