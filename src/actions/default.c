#include <gtk/gtk.h>
#include <stdlib.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "action_handlers.h"
#include "../clock_select_dialog.h"
#include "../undo_create.h"
#include "command_history.h"

#define DBG_DF(s)

extern cell_OP cell_options ;

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

// Are we gonna draw a window ? This is decided at button_press time
static gboolean bHaveWindow = FALSE ;

// Cells overlapped upon drop
static gboolean bHadOverlap = FALSE ;

static int xRef, yRef, xPrev, yPrev ;

void run_action_DEFAULT (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
  {
  global_gc = gc ;
  pDesign = design ;
  pProjectOpts = options ;
  wndMain = main_window ;

  pmh->lIDButtonPressed = g_signal_connect (G_OBJECT (widget), "button_press_event", (GCallback)button_press_event, NULL) ;
  pmh->lIDMotionNotify = g_signal_connect (G_OBJECT (widget), "motion_notify_event", (GCallback)motion_notify_event, NULL) ;
  pmh->lIDButtonReleased = g_signal_connect (G_OBJECT (widget), "button_release_event", (GCallback)button_release_event, NULL) ;
  }

void run_action_DEFAULT_sel_changed ()
  {
  bHadOverlap = TRUE ;
  }

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GQCell *pqc = NULL ;
  int Nix = pProjectOpts->number_of_selected_cells ;

  if (!((1 == event->button) || (2 == event->button))) return FALSE ;
  
  pqc = select_cell_at_coords ((GQCell *)(pDesign->first_layer->first_obj), calc_world_x (event->x), calc_world_y (event->y)) ;
  
  if (NULL != pqc)
    for (Nix = 0 ; Nix < pProjectOpts->number_of_selected_cells ; Nix++)
      if (pqc == pProjectOpts->selected_cells[Nix])
        break ;

  if ((bHaveWindow = (!bHadOverlap && (((1 == event->button) && (NULL == pqc || (event->state & GDK_CONTROL_MASK) || 
    (event->state & GDK_SHIFT_MASK) || Nix >= pProjectOpts->number_of_selected_cells)) || 2 == event->button))))
    {
    bHaveWindow = TRUE ;
    gdk_gc_set_foreground (global_gc, &(pProjectOpts->clrWhite)) ;
    xRef = xPrev = event->x ;
    yRef = yPrev = event->y ;
    gdk_gc_set_function (global_gc, GDK_XOR) ;
    gdk_draw_rectangle (widget->window, global_gc, FALSE, xRef, yRef, 0, 0) ;
    gdk_gc_set_function (global_gc, GDK_COPY) ;
    }
  else
  if (!(bHadOverlap && Nix >= pProjectOpts->number_of_selected_cells))
    pProjectOpts->window_move_selected_cell = pqc ;
  return TRUE ;
  }

static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkModifierType state ;
  int Nix, x, y ;
  double dxOffset, dyOffset ;
  
  if (!((event->state & GDK_BUTTON1_MASK) || (event->state & GDK_BUTTON2_MASK) || bHadOverlap)) return FALSE ;
  
  gdk_window_get_pointer (widget->window, &x, &y, &state) ;

  if (bHaveWindow)
    {
    gdk_gc_set_function (global_gc, GDK_XOR) ;
    gdk_draw_rectangle (widget->window, global_gc, FALSE,
      MIN (xRef, xPrev), MIN (yRef, yPrev), abs (xRef - xPrev), abs (yRef - yPrev)) ;
    
    xPrev = event->x ;
    yPrev = event->y ;

    gdk_draw_rectangle (widget->window, global_gc, FALSE,
      MIN (xRef, xPrev), MIN (yRef, yPrev), abs (xRef - xPrev), abs (yRef - yPrev)) ;
    gdk_gc_set_function (global_gc, GDK_COPY) ;
    }
  else
    {
    // We don't have a window, so we're certainly moving things
    dxOffset = grid_world_x (calc_world_x (event->x)) - pProjectOpts->window_move_selected_cell->x ;
    dyOffset = grid_world_y (calc_world_y (event->y)) - pProjectOpts->window_move_selected_cell->y ;
    if (0 == dxOffset && 0 == dyOffset) return FALSE ;
    gdk_gc_set_function (global_gc, GDK_XOR) ;
    redraw_selected_cells (widget->window, global_gc, pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
    for (Nix = 0 ; Nix < pProjectOpts->number_of_selected_cells ; Nix++)
      gqcell_move_by_offset (pProjectOpts->selected_cells[Nix], dxOffset, dyOffset) ;
    redraw_selected_cells (widget->window, global_gc, pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
    gdk_gc_set_function (global_gc, GDK_COPY) ;
    }
  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  int 
    xTop = MIN (xRef, xPrev), yTop = MIN (yRef, yPrev), 
    xBot = MAX (xRef, xPrev), yBot = MAX (yRef, yPrev),
    Nix ;

  if (bHaveWindow)
    {
    gdk_gc_set_function (global_gc, GDK_XOR) ;
    gdk_draw_rectangle (widget->window, global_gc, FALSE, xTop, yTop, xBot - xTop, yBot - yTop) ;
    gdk_gc_set_function (global_gc, GDK_COPY) ;
    bHaveWindow = FALSE ;
    }
  else 
  if ((1 == event->button) && NULL != (pProjectOpts->window_move_selected_cell))
    {
    // if we don't have a window and button 1 was just released, we must have been moving things
    double
      dxOffset = grid_world_x (calc_world_x (event->x)) - pProjectOpts->window_move_selected_cell->x,
      dyOffset = grid_world_y (calc_world_y (event->y)) - pProjectOpts->window_move_selected_cell->y ;

    if (!(0 == dxOffset && 0 == dyOffset))
      {
      gdk_gc_set_function (global_gc, GDK_XOR) ;
      redraw_selected_cells (widget->window, global_gc, pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
      for (Nix = 0 ; Nix < pProjectOpts->number_of_selected_cells ; Nix++)
        gqcell_move_by_offset (pProjectOpts->selected_cells[Nix], dxOffset, dyOffset) ;
      redraw_selected_cells (widget->window, global_gc, pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
      gdk_gc_set_function (global_gc, GDK_COPY) ;
      }

    for (Nix = 0 ; Nix < pProjectOpts->number_of_selected_cells ; Nix++)
      if (find_overlapping ((GQCell *)(pDesign->first_layer->first_obj), pProjectOpts->selected_cells[Nix]) != NULL) 
        {
        bHadOverlap = TRUE ;
        return FALSE;
        }

    bHadOverlap = FALSE ;
    gui_add_to_undo (pProjectOpts->drop_new_cells ?
      Undo_CreateAction_CreateCells (pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) :
      Undo_CreateAction_CellParamChange (pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells)) ;
    pProjectOpts->drop_new_cells = FALSE ;
    pProjectOpts->bDesignAltered = TRUE ;
    return TRUE ;
    }

  gdk_gc_set_function (global_gc, GDK_COPY) ;
  
  if (2 == event->button)
    {
    zoom_window (MIN (xRef, event->x), MIN (yRef, event->y), MAX (xRef, event->x), MAX (yRef, event->y)) ;
    setup_rulers () ;
    redraw_world (widget->window, global_gc, (GQCell *)(pDesign->first_layer->first_obj), pProjectOpts->SHOW_GRID) ;
    }
  else
  if (1 == event->button)
    {
    if (!((event->state & GDK_SHIFT_MASK) || (event->state & GDK_CONTROL_MASK)))
      release_selection () ;

    if (event->state & GDK_SHIFT_MASK)
      add_cells_to_selection ((GQCell *)(pDesign->first_layer->first_obj), &(pProjectOpts->selected_cells), 
        &(pProjectOpts->number_of_selected_cells), xTop, yTop, xBot, yBot) ;
    else
    if (event->state & GDK_CONTROL_MASK)
      remove_cells_from_selection ((GQCell *)(pDesign->first_layer->first_obj), &(pProjectOpts->selected_cells), 
        &(pProjectOpts->number_of_selected_cells), widget->window, global_gc, xTop, yTop, xBot, yBot) ;
    else
      select_cells_in_window ((GQCell *)(pDesign->first_layer->first_obj), &(pProjectOpts->selected_cells), 
        &(pProjectOpts->number_of_selected_cells), xTop, yTop, xBot, yBot);
    if (pProjectOpts->number_of_selected_cells > 0)
      {
      update_clock_select_dialog (get_clock_from_selected_cells (
        pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells)) ;

      if (NULL != pProjectOpts->selected_cells)
        redraw_selected_cells (widget->window, global_gc, pProjectOpts->selected_cells, pProjectOpts->number_of_selected_cells) ;
      }
    }
  return TRUE ;
  }

