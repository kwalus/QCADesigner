#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "../undo_create.h"
#include "../callback_helpers.h"
#include "action_handlers.h"

extern cell_OP cell_options ;

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

static int x0, y0, x1, y1 ;

void run_action_ARRAY (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
  {
  global_gc = gc ;
  pDesign = design ;
  pProjectOpts = options ;
  wndMain = main_window ;

  pmh->lIDButtonPressed = g_signal_connect (G_OBJECT (widget), "button_press_event", (GCallback)button_press_event, NULL) ;
  pmh->lIDMotionNotify = g_signal_connect (G_OBJECT (widget), "motion_notify_event", (GCallback)motion_notify_event, NULL) ;
  pmh->lIDButtonReleased = g_signal_connect (G_OBJECT (widget), "button_release_event", (GCallback)button_release_event, NULL) ;
  }

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (1 != event->button) return FALSE ;

  x0 = x1 = event->x ;
  y0 = y1 = event->y ;
  
  return TRUE ;
  }

static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkModifierType state ;
  int x, y ;
  
  if (!(event->state & GDK_BUTTON1_MASK)) return FALSE ;

  gdk_window_get_pointer (widget->window, &x, &y, &state) ;
  
  // draw over the old coords with GDK_XOR to erase the old array cells //
  gdk_gc_set_function (global_gc, GDK_XOR) ;
  if(!(x0 == x1 && y0 == y1))
    {
    if (abs(x0 - x1) >= abs(y0 - y1))
      {
      y1 = y0;
      draw_temp_array (widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x1, y0);
      } 
    else 
      {
      x1 = x0;
      draw_temp_array (widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x0, y1);
      }
    }

  // set the second point of the line to the current mouse position //
  x1 = event->x;
  y1 = event->y;

  // -- draw a temporary horizontal array -- //
  if (abs (x0 - x1) >= abs (y0 - y1))
    {
    y1 = y0;
    draw_temp_array (widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x1, y0);
    } 
  // -- Draw a temporary vertical array -- //
  else 
    {
    x1 = x0;
    draw_temp_array (widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x0, y1);
    }
  gdk_gc_set_function (global_gc, GDK_COPY) ;
  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  int icNewCells = 0 ;
  GQCell **pqcNewCells = NULL ;
  
  if (3 == event->button)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wndMain->default_action_button), TRUE) ;
    return FALSE ;
    }

  if (1 != event->button) return FALSE ;

  gdk_gc_set_function (global_gc, GDK_XOR) ;
  if(!(x0 == x1 && y0 == y1))
    {
    if (abs(x0 - x1) >= abs(y0 - y1))
      {
      y1 = y0;
      draw_temp_array(widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x1, y0);
      } 
    else 
      {
      x1 = x0;
      draw_temp_array(widget->window, global_gc, pProjectOpts->selected_cell_type, x0, y0, x0, y1);
      }
    }
    
  gdk_gc_set_function (global_gc, GDK_COPY) ;
  
  if ((icNewCells = 
    create_array_of_cells (
      (GQCell **)(&((pDesign->first_layer->first_obj))), 
      (GQCell **)(&((pDesign->first_layer->last_obj))), 
      x0, y0, x1, y1, &cell_options, pProjectOpts->selected_cell_type, &pqcNewCells, pProjectOpts->SNAP_TO_GRID)) > 0)
    {

    gui_add_to_undo (Undo_CreateAction_CreateCells (pqcNewCells, icNewCells)) ;

    redraw_selected_cells (widget->window, global_gc, pqcNewCells, icNewCells) ;
    free (pqcNewCells) ;
    pqcNewCells = NULL ;
    icNewCells = 0 ;
    }

  pProjectOpts->bDesignAltered = TRUE ;
  return TRUE ;
  }
