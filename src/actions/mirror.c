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

void run_action_MIRROR (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
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
  
  gdk_gc_set_foreground (global_gc, &(pProjectOpts->clrCyan)) ;

  gdk_gc_set_function (global_gc, GDK_XOR) ;
  // -- Draw a Horizontal Line -- //
  if (abs (x0 - x1) >= abs (y0 - y1))
    gdk_draw_line (widget->window, global_gc, x0, y0, x1, y0);
  // -- Draw a Vertical Line -- //
  else
    gdk_draw_line (widget->window, global_gc, x0, y0, x0, y1);
  gdk_gc_set_function (global_gc, GDK_COPY) ;
  return TRUE ;
  }

static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkModifierType state ;
  int x, y ;
  
  if (!(event->state & GDK_BUTTON1_MASK)) return FALSE ;

  gdk_window_get_pointer (widget->window, &x, &y, &state) ;
  
  gdk_gc_set_function (global_gc, GDK_XOR) ;
  // -- Draw a Horizontal Line -- //
  if (abs (x0 - x1) >= abs (y0 - y1))
    gdk_draw_line (widget->window, global_gc, x0, y0, x1, y0);
  // -- Draw a Vertical Line -- //
  else
    gdk_draw_line (widget->window, global_gc, x0, y0, x0, y1);

  // set the second point of the line to the current mouse position //
  x1 = event->x;
  y1 = event->y;

  // -- Draw a Horizontal Line -- //
  if (abs (x0 - x1) >= abs (y0 - y1))
    gdk_draw_line (widget->window, global_gc, x0, y0, x1, y0);
  // -- Draw a Vertical Line -- //
  else
    gdk_draw_line (widget->window, global_gc, x0, y0, x0, y1);
  gdk_gc_set_function (global_gc, GDK_COPY) ;

  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GQCell **ppqcNewCells = NULL ;
  int icNewCells = 0 ;
  
  if (1 != event->button) return FALSE ;

  gdk_gc_set_function (global_gc, GDK_XOR) ;
  // XOR the mirror line one last time, to make it disappear
  // -- Draw a Horizontal Line -- //
  if (abs (x0 - x1) >= abs (y0 - y1))
    gdk_draw_line(widget->window, global_gc, x0, y0, x1, y0);
  // -- Draw a Vertical Line -- //
  else
    gdk_draw_line(widget->window, global_gc, x0, y0, x0, y1);

  gdk_gc_set_function (global_gc, GDK_COPY) ;

  if (abs(x0 - x1) >= abs(y0 - y1))
      icNewCells = mirror_cells_about_line((GQCell **)(&((pDesign->first_layer->first_obj))), 
        (GQCell **)(&((pDesign->first_layer->last_obj))), x0, y0, x1, y0,  pProjectOpts->selected_cells, 
        pProjectOpts->number_of_selected_cells, &cell_options, pProjectOpts->selected_cell_type, &ppqcNewCells);
  else
      icNewCells = mirror_cells_about_line((GQCell **)(&((pDesign->first_layer->first_obj))), 
        (GQCell **)(&((pDesign->first_layer->last_obj))), x0, y0, x0, y1, pProjectOpts->selected_cells, 
        pProjectOpts->number_of_selected_cells, &cell_options, pProjectOpts->selected_cell_type, &ppqcNewCells);

  if (!(event->state & GDK_CONTROL_MASK))
    {
    gui_add_to_undo (Undo_CreateAction_DeleteCells (pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells)) ;
    gui_delete_cells (pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
    free (pProjectOpts->selected_cells) ;
    pProjectOpts->selected_cells = NULL ;
    pProjectOpts->number_of_selected_cells = 0 ;
    pProjectOpts->window_move_selected_cell = NULL ;
    }
  else
    release_selection () ;

  if (icNewCells > 0)
    {
    pProjectOpts->selected_cells = ppqcNewCells ;
    pProjectOpts->number_of_selected_cells = icNewCells ;
    pProjectOpts->window_move_selected_cell = ppqcNewCells[0] ;

    gui_add_to_undo (Undo_CreateAction_CreateCells (ppqcNewCells, icNewCells)) ;

    redraw_selected_cells (widget->window, global_gc, ppqcNewCells, icNewCells) ;

    pProjectOpts->bDesignAltered = TRUE ;
    }

  // Go back to "Default"
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wndMain->default_action_button), TRUE) ;
  return TRUE ;
  }
