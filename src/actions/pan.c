#include <gtk/gtk.h>
#include "../interface.h"
#include "../cad.h"
#include "../stdqcell.h"
#include "action_handlers.h"

extern cell_OP cell_options ;

static gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;

static GdkGC *global_gc ;
static DESIGN *pDesign ;
static main_W *wndMain ;
static project_OP *pProjectOpts ;

int x0, y0, x1, y1 ;

void run_action_PAN (MOUSE_HANDLERS *pmh, GtkWidget *widget, GdkGC *gc, DESIGN *design, project_OP *options, main_W *main_window)
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
  int x, y, cx, cy ;
  
  gdk_window_get_pointer (widget->window, &x, &y, &state) ;
  
  if (!(event->state & GDK_BUTTON1_MASK))
    {
    propagate_motion_to_rulers (widget, event) ;
    return FALSE ;
    }
  
  x1 = event->x ;
  y1 = event->y ;
  cx = x1 - x0 ;
  cy = y1 - y0 ;
  
  if (0 == cx && 0 == cy) return FALSE ;
  
  pan (cx, cy) ;
  setup_rulers () ;
  redraw_async (widget) ;
  
  x0 = x1 ;
  y0 = y1 ;

  return TRUE ;
  }

static gboolean button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (3 == event->button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wndMain->default_action_button), TRUE) ;
  return FALSE ;
  }
