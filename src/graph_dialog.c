//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// **** Please use complete names in variables and      //
// **** functions. This will reduce ramp up time for new//
// **** people trying to contribute to the project.     //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@vlsi.enel.ucalgary.ca).  It is a leaner,   //
// cleaner implementation of the graph dialog.          //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "support.h"
#include "graph_options_dialog.h"
#include "graph_dialog.h"
#include "print.h"
#include "print_preview.h"
#include "print_graph_properties_dialog.h"

#define GRAPH_CX 1000
#define GRAPH_X_PADDING 10
#define GRAPH_Y_PADDING 10
#define GRAPH_TRACE_CY 75
#define GRAPH_LABEL_CX 65
#define GRAPH_TRACE_PAD_Y 5
#define GRAPH_LABEL_TRACE_PAD 20
#define GRAPH_TRACE_BOX_PAD 3
#define GRAPH_TRACE_MINMAX_GAP 3
#define GRAPH_LABEL_TEXT_LEFT_INDENT 3
#define GRAPH_TEXT_CLEARANCE 2

#define DBG_GD(s)

typedef struct
  {
  GtkWidget *dlgGraphs;
  GtkWidget *dialog_vbox1;
  GtkWidget *swndGraphs;
  GtkWidget *vpGraphs;
  GtkWidget *daGraphs;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbuttonbox1;
  GtkWidget *btnOptions;
  GtkWidget *btnZoomOut;
  GtkWidget *btnZoomIn;
  GtkWidget *btnPreview;
  GtkWidget *btnPrint;
  GtkWidget *btnClose;
  GtkWidget *btnReset ;
  } graph_D ;

typedef struct
  {
  double dZoom ;
  double dIncrem ;
  int cx ;
  int cy ;
  int iTextLeft ;
  int iLabelCX ;
  int iTraceBoxLeft ;
  int iTraceBoxCX ;
  int iTraceRight ;
  int iTraceLeft ;
  int iBoxCY ;
  int iTraceCY ;
  int idxBeg ;
  int idxEnd ;
  } DRAW_PARAMS ;

static graph_D graph = {NULL} ;
static print_graph_OP poGraphs = {{792, 612, 72, 72, 72, 72, TRUE, NULL}, TRUE, FALSE, FALSE, 1, 1} ;

static void create_graph_dialog (graph_D *dialog) ;
static gboolean daGraphs_expose (GtkWidget *widget, GdkEventExpose *ev, gpointer user_data) ;
static gboolean daGraphs_configure (GtkWidget *widget, GdkEventConfigure *ev, gpointer user_data) ;
static gboolean daGraphs_mousedown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data) ;
static gboolean daGraphs_mousemove (GtkWidget *widget, GdkEventMotion *ev, gpointer user_data) ;
static gboolean daGraphs_mouseup (GtkWidget *widget, GdkEventButton *ev, gpointer user_data) ;
static void btnPrint_clicked (GtkWidget *widget, gpointer user_data) ;
static void show_graph_options (GtkWidget *widget, gpointer user_data) ;
static void graph_zoom_inc (GtkWidget *widget, gpointer user_data) ;
static void graph_zoom_dec (GtkWidget *widget, gpointer user_data) ;
static void reset_graph_params (GtkWidget *widget, gpointer user_data) ;
static void btnPreview_clicked (GtkWidget *widget, gpointer user_data) ;
static void CalculateDrawingArea (graph_D *dialog, simulation_data *sim_data, int iZoomNum, int iZoomDen, int *pcx, int *pcy) ;
static void paint_graph_window (GtkWidget *da, DRAW_PARAMS *pdp, simulation_data *sim_data, GdkRectangle *prcClip) ;
static void draw_single_trace (GtkWidget *da, GdkGC *pgc, DRAW_PARAMS *pdp, struct TRACEDATA *trace, int idx, int icSamples) ;
static gboolean set_current_colour (GdkColor *pclr, int iClr, int iClrMask) ;
static void CacheDrawingParams (DRAW_PARAMS *pdp, int cx, int cy, int iZoomNum, int iZoomDen) ;
static void RedoGraph (graph_D *dialog, simulation_data *sim_data, int iZoomNum, int iZoomDen, DRAW_PARAMS *pdp) ;

void show_graph_dialog (GtkWindow *parent, simulation_data *sim_data)
  {
  int cx = 0, cy = 0 ;
  DRAW_PARAMS dp = {0} ;
  
  if (NULL == graph.dlgGraphs)
    create_graph_dialog (&graph) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (graph.dlgGraphs), parent) ;
  
  /* Static data for the dialog */
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "dialog", &graph) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "sim_data", sim_data) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "iZoomNum", (gpointer)1) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "iZoomDen", (gpointer)1) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "php", NULL) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "bDrag", (gpointer)FALSE) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "xBeg", (gpointer)FALSE) ;
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "xOld", (gpointer)FALSE) ;
  
  CalculateDrawingArea (&graph, sim_data, 1, 1, &cx, &cy) ;

  /* Initialize the drawing parameters - this way coordinates and dimensions dependent solely on the size of the window
     do not have to be recalculated at each _expose */
  dp.idxBeg = 0 ;
  dp.idxEnd = sim_data->number_samples - 1 ;
  CacheDrawingParams (&dp, cx, cy, 1, 1) ;
  
  gtk_object_set_data (GTK_OBJECT (graph.dlgGraphs), "pdp", &dp) ;
  
  gtk_widget_set_usize (graph.daGraphs, cx, cy) ;
  
  gtk_dialog_run (GTK_DIALOG (graph.dlgGraphs)) ;
  gtk_widget_hide (graph.dlgGraphs) ;
  }

static void create_graph_dialog (graph_D *dialog)
  {
  /* The window */
  dialog->dlgGraphs = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphs), "dlgGraphs", dialog->dlgGraphs);
  gtk_widget_set_usize (dialog->dlgGraphs, 800, 600);
  gtk_window_set_title (GTK_WINDOW (dialog->dlgGraphs), _("Simulation Results"));
  gtk_window_set_modal (GTK_WINDOW (dialog->dlgGraphs), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog->dlgGraphs), TRUE, TRUE, TRUE);

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->dlgGraphs)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphs), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  /* The graphing area widgets */
  dialog->swndGraphs = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (dialog->swndGraphs);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->swndGraphs, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swndGraphs), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->vpGraphs = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (dialog->vpGraphs);
  gtk_container_add (GTK_CONTAINER (dialog->swndGraphs), dialog->vpGraphs);

  dialog->daGraphs = gtk_drawing_area_new ();
  gtk_widget_show (dialog->daGraphs);
  gtk_container_add (GTK_CONTAINER (dialog->vpGraphs), dialog->daGraphs);
  gtk_widget_set_usize (dialog->daGraphs, 640, 480);

  /* The button containers */
  dialog->dialog_action_area1 = GTK_DIALOG (dialog->dlgGraphs)->action_area;
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  /* The buttons */
  dialog->btnReset = gtk_button_new_with_label (_("Reset"));
  gtk_widget_show (dialog->btnReset);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnReset);
  GTK_WIDGET_SET_FLAGS (dialog->btnReset, GTK_CAN_DEFAULT);

  dialog->btnOptions = gtk_button_new_with_label (_("Options"));
  gtk_widget_show (dialog->btnOptions);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnOptions);
  GTK_WIDGET_SET_FLAGS (dialog->btnOptions, GTK_CAN_DEFAULT);

  dialog->btnZoomOut = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  gtk_widget_show (dialog->btnZoomOut);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnZoomOut);
  GTK_WIDGET_SET_FLAGS (dialog->btnZoomOut, GTK_CAN_DEFAULT);

  dialog->btnZoomIn = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  gtk_widget_show (dialog->btnZoomIn);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnZoomIn);
  GTK_WIDGET_SET_FLAGS (dialog->btnZoomIn, GTK_CAN_DEFAULT);

//  dialog->btnPreview = gtk_toggle_button_new_with_label (_("Preview Mode"));
  dialog->btnPreview = gtk_button_new_from_stock (GTK_STOCK_PRINT_PREVIEW) ;
  gtk_widget_show (dialog->btnPreview);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnPreview);
  GTK_WIDGET_SET_FLAGS (dialog->btnPreview, GTK_CAN_DEFAULT);

  dialog->btnPrint = gtk_button_new_from_stock (GTK_STOCK_PRINT) ;
  gtk_widget_show (dialog->btnPrint);
  gtk_container_add (GTK_CONTAINER (dialog->dialog_action_area1), dialog->btnPrint);
  GTK_WIDGET_SET_FLAGS (dialog->btnPrint, GTK_CAN_DEFAULT);

  dialog->btnClose = gtk_dialog_add_button (GTK_DIALOG (dialog->dlgGraphs), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgGraphs), GTK_RESPONSE_CLOSE) ;

  /* Manipulating and drawing the graphs */
  gtk_signal_connect (GTK_OBJECT (dialog->daGraphs), "expose_event", GTK_SIGNAL_FUNC (daGraphs_expose), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->dlgGraphs), "configure_event", GTK_SIGNAL_FUNC (daGraphs_configure), dialog->dlgGraphs) ;
  gtk_signal_connect_after (GTK_OBJECT (dialog->daGraphs), "button_press_event", GTK_SIGNAL_FUNC (daGraphs_mousedown), dialog->dlgGraphs) ;
  gtk_signal_connect_after (GTK_OBJECT (dialog->daGraphs), "motion_notify_event", GTK_SIGNAL_FUNC (daGraphs_mousemove), dialog->dlgGraphs) ;
  gtk_signal_connect_after (GTK_OBJECT (dialog->daGraphs), "button_release_event", GTK_SIGNAL_FUNC (daGraphs_mouseup), dialog->dlgGraphs) ;
  
  /* The various buttons */
  gtk_signal_connect (GTK_OBJECT (dialog->btnZoomIn), "clicked", GTK_SIGNAL_FUNC (graph_zoom_inc), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnZoomOut), "clicked", GTK_SIGNAL_FUNC (graph_zoom_dec), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnPreview), "clicked", GTK_SIGNAL_FUNC (btnPreview_clicked), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnOptions), "clicked", GTK_SIGNAL_FUNC (show_graph_options), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnReset), "clicked", GTK_SIGNAL_FUNC (reset_graph_params), dialog->dlgGraphs) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnPrint), "clicked", GTK_SIGNAL_FUNC (btnPrint_clicked), dialog->dlgGraphs) ;

  gtk_widget_set_events (GTK_WIDGET (dialog->daGraphs), GDK_EXPOSURE_MASK
		       | GDK_LEAVE_NOTIFY_MASK
		       | GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK
		       | GDK_KEY_PRESS_MASK
		       | GDK_POINTER_MOTION_MASK);
  }

static void show_graph_options (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  get_graph_options_from_user (GTK_WINDOW (dialog->dlgGraphs), sim_data->trace, sim_data->number_of_traces, sim_data->clock_data, 4) ;
  
  RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
  }

static gboolean daGraphs_expose (GtkWidget *widget, GdkEventExpose *ev, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  paint_graph_window (dialog->daGraphs, pdp, sim_data, (NULL != ev) ? &(ev->area) : NULL) ;
  return TRUE ;
  }

static gboolean daGraphs_configure (GtkWidget *widget, GdkEventConfigure *ev, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
  
  /* This is done only once in the lifetime of the graph, so disconnect */
  gtk_signal_disconnect_by_func (GTK_OBJECT (dialog->dlgGraphs), GTK_SIGNAL_FUNC (daGraphs_configure), user_data) ;
  
  return FALSE ;
  }

static gboolean daGraphs_mousedown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data)
  {
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  int x = (int)ev->x ;
  
  if (ev->type != GDK_BUTTON_PRESS) return FALSE ; /* We don't like double-clicks */
  if (1 == ev->button && x >= pdp->iTraceLeft && x <= pdp->iTraceRight)
    {
    /* Draw a line that will widen into a XOR window as the user drags */
    gtk_object_set_data (GTK_OBJECT (user_data), "xBeg", (gpointer)x) ;
    gtk_object_set_data (GTK_OBJECT (user_data), "xOld", (gpointer)x) ;
    gtk_object_set_data (GTK_OBJECT (user_data), "bDrag", (gpointer)TRUE) ;
    gdk_gc_set_function (widget->style->white_gc, GDK_XOR) ;
    gdk_draw_line (widget->window, widget->style->white_gc, x, 0, x, pdp->cy - 1) ;
    }
  
  return TRUE ;
  }

static gboolean daGraphs_mousemove (GtkWidget *widget, GdkEventMotion *ev, gpointer user_data)
  {
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  int x = (int)ev->x ;
  
  if ((gboolean)gtk_object_get_data (GTK_OBJECT (user_data), "bDrag") && x >= pdp->iTraceLeft && x <= pdp->iTraceRight)
    {
    int xBeg = (int)gtk_object_get_data (GTK_OBJECT (user_data), "xBeg"),
        xOld = (int)gtk_object_get_data (GTK_OBJECT (user_data), "xOld") ;
    
    /* Draw the XOR window differentially */
    
    if (x != xOld)
      {
      if (x > xOld)
        {
	if (xOld < xBeg)
	  {
	  if (x > xBeg)
  	    gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, xOld, 0, x - xOld + 1, pdp->cy) ;
	  else
  	    gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, xOld, 0, x - xOld, pdp->cy) ;
	  }
	else
	  gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, xOld + 1, 0, x - xOld, pdp->cy) ;
	}
      else
      	{
	if (xOld < xBeg)
	  gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, x, 0, xOld - x, pdp->cy) ;
	else
	  {
	  if (x < xBeg)
	    gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, x, 0, xOld - x + 1, pdp->cy) ;
	  else
	    gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, x + 1, 0, xOld - x, pdp->cy) ;
	  }
	}
      }
    
    gtk_object_set_data (GTK_OBJECT (user_data), "xOld", (gpointer)x) ;
    }

  return TRUE ;
  }

static gboolean daGraphs_mouseup (GtkWidget *widget, GdkEventButton *ev, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  if (1 == ev->button && (gboolean)gtk_object_get_data (GTK_OBJECT (user_data), "bDrag"))
    {
    int xBeg = (int)gtk_object_get_data (GTK_OBJECT (user_data), "xBeg"),
      	xOld = (int)gtk_object_get_data (GTK_OBJECT (user_data), "xOld"),
	iTmp, idxBeg, idxEnd ;
    gboolean b = TRUE ;
      
    if (xBeg > xOld)
      {
      b = FALSE ;
      iTmp = xBeg ;
      xBeg = xOld ;
      xOld = iTmp ;
      }
    
    /* Clean up the XOR rectangle */
    gdk_draw_rectangle (widget->window, widget->style->white_gc, TRUE, 
      xBeg, 0, xOld - xBeg + 1 - (b ? 0 : 1), pdp->cy) ;
    
    gtk_object_set_data (GTK_OBJECT (user_data), "bDrag", (gpointer)FALSE) ;
    gdk_gc_set_function (widget->style->white_gc, GDK_COPY) ;
    
    /* Hit test for the two indices */
    idxBeg = CLAMP ((int)(((double)(xBeg - pdp->iTraceLeft)) / pdp->dIncrem), 0, sim_data->number_samples - 1) ;
    idxEnd = CLAMP ((int)(((double)(xOld - pdp->iTraceLeft)) / pdp->dIncrem), 0, sim_data->number_samples - 1) ;


    /* The beginning and ending sample idices must be relative to the already visible chunk */
    if (idxEnd != idxBeg)
      {
      pdp->idxBeg += idxBeg ;
      pdp->idxEnd  = pdp->idxBeg + idxEnd - idxBeg ;
      pdp->dIncrem = (double)(pdp->iTraceRight - pdp->iTraceLeft + 1) / (double)(pdp->idxEnd - pdp->idxBeg) ;
      RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
      }
    }
  return TRUE ;
  }

static void btnPreview_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  do_print_preview ((print_OP *)&poGraphs, GTK_WINDOW (dialog->dlgGraphs), sim_data, (PrintFunction)print_graphs) ;
  }

static void reset_graph_params (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  pdp->idxBeg = 0 ;
  pdp->idxEnd = sim_data->number_samples - 1 ;
  iZoomNum = 1 ;
  iZoomDen = 1 ;
  
  RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
  
  gtk_object_set_data (GTK_OBJECT (user_data), "iZoomNum", (gpointer)iZoomNum) ;
  gtk_object_set_data (GTK_OBJECT (user_data), "iZoomDen", (gpointer)iZoomDen) ;
  }

static void graph_zoom_inc (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  /* Rules for setting the zoom numerator and denominator */
  if (1 == iZoomDen && iZoomNum < 10)
    gtk_object_set_data (GTK_OBJECT (user_data), "iZoomNum", (gpointer)(++iZoomNum)) ;
  else if (iZoomDen > 1) 
    gtk_object_set_data (GTK_OBJECT (user_data), "iZoomDen", (gpointer)(--iZoomDen)) ;
  
  RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
  }

static void graph_zoom_dec (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  int iZoomNum = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomNum") ;
  int iZoomDen = (int)gtk_object_get_data (GTK_OBJECT (user_data), "iZoomDen") ;
  DRAW_PARAMS *pdp = (DRAW_PARAMS *)gtk_object_get_data (GTK_OBJECT (user_data), "pdp") ;
  
  /* Rules for setting the zoom numerator and denominator */
  if (1 == iZoomNum && iZoomDen < 2)
    gtk_object_set_data (GTK_OBJECT (user_data), "iZoomDen", (gpointer)(++iZoomDen)) ;
  else if (iZoomNum > 1)
    gtk_object_set_data (GTK_OBJECT (user_data), "iZoomNum", (gpointer)(--iZoomNum)) ;
  
  RedoGraph (dialog, sim_data, iZoomNum, iZoomDen, pdp) ;
  }

static void btnPrint_clicked (GtkWidget *widget, gpointer user_data)
  {
  simulation_data *sim_data = (simulation_data *)gtk_object_get_data (GTK_OBJECT (user_data), "sim_data") ;
  if (get_print_graph_properties_from_user (GTK_WINDOW (user_data), &poGraphs, sim_data))
    print_graphs (&poGraphs, sim_data) ;
  }

static void CacheDrawingParams (DRAW_PARAMS *pdp, int cx, int cy, int iZoomNum, int iZoomDen)
  {
  /* When the window is resized, these parameters can be precalculated */
  pdp->cx = cx ;
  pdp->cy = cy ;
  pdp->dZoom = (double)iZoomNum / (double)iZoomDen ;
  pdp->iTextLeft = GRAPH_X_PADDING + GRAPH_LABEL_TEXT_LEFT_INDENT ;
  pdp->iLabelCX = GRAPH_LABEL_CX * pdp->dZoom ;
  pdp->iTraceBoxLeft = GRAPH_X_PADDING + pdp->iLabelCX + GRAPH_LABEL_TRACE_PAD ;
  pdp->iTraceBoxCX = cx - 2 * GRAPH_X_PADDING - pdp->iLabelCX - GRAPH_LABEL_TRACE_PAD ;
  pdp->iTraceRight = cx - GRAPH_X_PADDING - GRAPH_TRACE_BOX_PAD ;
  pdp->iTraceLeft = pdp->iTraceBoxLeft + GRAPH_TRACE_BOX_PAD ;
  pdp->iBoxCY = GRAPH_TRACE_CY * pdp->dZoom - 2 * GRAPH_TRACE_PAD_Y ;
  pdp->iTraceCY = pdp->iBoxCY - 2 * GRAPH_TRACE_MINMAX_GAP ;
  pdp->dIncrem = (double)(pdp->iTraceRight - pdp->iTraceLeft + 1) / (double)(pdp->idxEnd - pdp->idxBeg) ;
  }

static void CalculateDrawingArea (graph_D *dialog, simulation_data *sim_data, int iZoomNum, int iZoomDen, int *pcx, int *pcy)
  {
  int Nix, cx, cy ;
  double dZoom = (double)iZoomNum / (double)iZoomDen ;
  
  /* Calculate the size of the drawing window based on the zoom, the size of the traces and the #defined width @ zoom 1:1 */
  
  *pcx = GRAPH_CX * dZoom ;
  *pcy = GRAPH_Y_PADDING + gdk_string_height (gdk_font_load (QCAD_GDKFONT), "Xg") + 2 * GRAPH_TEXT_CLEARANCE ;
  
  if (NULL != sim_data->trace && 0 != sim_data->number_of_traces)
    for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
      if (sim_data->trace[Nix].drawtrace)
	*pcy += GRAPH_TRACE_CY * dZoom ;
  
  if (NULL != sim_data->clock_data)
    for (Nix = 0 ; Nix < 4 ; Nix++)
      if (sim_data->clock_data[Nix].drawtrace)
	*pcy += GRAPH_TRACE_CY * dZoom ;
  
  if (NULL != dialog->vpGraphs->window)
    {
    gdk_window_get_size (dialog->vpGraphs->window, &cx, &cy) ;
    *pcy = MAX (*pcy, cy) ;
    }
  }

static void paint_graph_window (GtkWidget *da, DRAW_PARAMS *pdp, simulation_data *sim_data, GdkRectangle *prcClip)
  {
  int Nix = 0 ;
  int idxSample = 0 ;
  char *szText = "Simulation Results" ;
  GdkRectangle rc = {0, 0, pdp->cx, pdp->cy} ;
  GdkGC *pgc = da->style->black_gc ;
  GdkFont *pfont = gdk_font_load (QCAD_GDKFONT) ;
  GdkColor clr ;
  
  /* Set the clipping rectangle for the background gc to speed up drawing */
  if (NULL != prcClip)
    gdk_gc_set_clip_rectangle  (pgc, prcClip) ;
  gdk_draw_rectangle (da->window, pgc, TRUE, 0, 0, pdp->cx - 1, pdp->cy - 1) ;
  if (NULL != prcClip)
    gdk_gc_set_clip_rectangle (pgc, &rc) ;
  
  pgc = gdk_gc_new (da->window) ;
  /* Set the clipping rectangle for the drawing gc to speed up drawing */
  if (NULL != prcClip)
    gdk_gc_set_clip_rectangle (pgc, prcClip) ;
  
  set_current_colour (&clr, GREEN, 0xFFFF) ;
  
  gdk_gc_set_foreground (pgc, &clr) ;
  
  /* The title - "Simulation Results" */
  gdk_draw_string (da->window, pfont, pgc,
    (pdp->cx - gdk_string_width (pfont, szText)) / 2, 
    GRAPH_TEXT_CLEARANCE + gdk_string_height (pfont, szText),
    szText) ;
  
  /* Draw each trace */
  if (!(NULL == sim_data->trace || 0 == sim_data->number_of_traces))
    for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
      if (sim_data->trace[Nix].drawtrace)
        draw_single_trace (da, pgc, pdp, &sim_data->trace[Nix], idxSample++, sim_data->number_samples) ;
  
  /* Draw each clock */
  if (NULL != sim_data->clock_data)
    for (Nix = 0 ; Nix < 4 ; Nix++)
      if (sim_data->clock_data[Nix].drawtrace)
        draw_single_trace (da, pgc, pdp, &sim_data->clock_data[Nix], idxSample++, sim_data->number_samples) ;
  
  gdk_gc_unref (pgc) ;
  }

static void draw_single_trace (GtkWidget *da, GdkGC *pgc, DRAW_PARAMS *pdp, struct TRACEDATA *trace, int idx, int icSamples)
  {
  int iClrMask = 0xFFFF ;
  GdkColor clr = {0, 0, 0, 0} ;
  int iInc, Nix, y = 0, yOld = 0 ;
  double dMin = 0, dMax = 0, dSampleMin = 0, dSampleMax = 0 ;
  char szText[32] = "" ;
  GdkFont *pfont = gdk_font_load(QCAD_GDKFONT);
  int cyFont = gdk_string_height (pfont, "X")/*, iCXString*/ ;
  int iBoxTop, iTraceTop, iTraceOffset1, iTraceOffset2 ;
  
        iBoxTop = cyFont + 2 * GRAPH_TEXT_CLEARANCE + (idx * GRAPH_TRACE_CY) * pdp->dZoom + GRAPH_TRACE_PAD_Y ;
      iTraceTop = iBoxTop + GRAPH_TRACE_MINMAX_GAP ;
  iTraceOffset1 = iTraceTop + 0.5 * pdp->iTraceCY ;
  iTraceOffset2 = iTraceTop + pdp->iTraceCY ;

  DBG_GD (fprintf (stderr, "Graphing trace |%s| between indices %d and %d\n", trace->data_labels, pdp->idxBeg, pdp->idxEnd)) ;

  set_current_colour (&clr, GREEN, iClrMask) ;
  gdk_gc_set_foreground (pgc, &clr) ;
  gdk_gc_set_background (pgc, &clr) ;

  /* The 2 boxen that make up the trace */  
  gdk_draw_rectangle (da->window, pgc, FALSE, GRAPH_X_PADDING,    iBoxTop, pdp->iLabelCX,    pdp->iBoxCY) ;
  gdk_draw_rectangle (da->window, pgc, FALSE, pdp->iTraceBoxLeft, iBoxTop, pdp->iTraceBoxCX, pdp->iBoxCY) ;

  /* The min, mid and max lines */
  gdk_gc_set_line_attributes(pgc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
  gdk_draw_line (da->window, pgc, pdp->iTraceLeft, iTraceTop,     pdp->iTraceRight, iTraceTop) ;
  gdk_draw_line (da->window, pgc, pdp->iTraceLeft, iTraceOffset1, pdp->iTraceRight, iTraceOffset1) ;
  gdk_draw_line (da->window, pgc, pdp->iTraceLeft, iTraceOffset2, pdp->iTraceRight, iTraceOffset2) ;
  gdk_gc_set_line_attributes(pgc, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
  
  /* Get the vertical scale */
  tracedata_get_min_max (trace, 0, icSamples - 1, &dMin, &dMax) ;
  if (0 == pdp->idxBeg && icSamples - 1 == pdp->idxEnd)
    {
    dSampleMin = dMin ;
    dSampleMax = dMax ;
    }
  else
    tracedata_get_min_max (trace, pdp->idxBeg, pdp->idxEnd, &dSampleMin, &dSampleMax) ;
  
  
  set_current_colour (&clr, trace->trace_color, iClrMask) ;
  gdk_gc_set_foreground (pgc, &clr) ;
  gdk_gc_set_background (pgc, &clr) ;

  /* Fill in the trace label */  
  g_snprintf (szText, 32, "max:%s%1.2f", dSampleMax > 0 ? " " : "", dSampleMax) ;
  gdk_draw_string (da->window, pfont, pgc, pdp->iTextLeft, iBoxTop + cyFont + GRAPH_TEXT_CLEARANCE + 1, szText) ;
  gdk_draw_string (da->window, pfont, pgc, pdp->iTextLeft, iTraceOffset1 + 0.5 * cyFont, trace->data_labels) ;
  g_snprintf (szText, 32, "min:%s%1.2f", dSampleMin > 0 ? " " : "", dSampleMin) ;
  gdk_draw_string (da->window, pfont, pgc, pdp->iTextLeft, iBoxTop + pdp->iBoxCY - GRAPH_TEXT_CLEARANCE, szText) ;

  /* If there are no samples, or the sample range is too narrow, return */
  if (icSamples <= 1 || pdp->idxEnd == pdp->idxBeg) return ;
  
  /* Plot the trace */
  yOld = iTraceOffset2 - (((trace->data[pdp->idxBeg] - dMin) / (dMax - dMin)) * pdp->iTraceCY) ;
  for (Nix = pdp->idxBeg + 1, iInc = 1 ; Nix <= pdp->idxEnd ; Nix++, iInc++)
    {
    y = iTraceOffset2 - (((trace->data[Nix] - dMin) /(dMax - dMin)) * pdp->iTraceCY) ;
    gdk_draw_line (da->window, pgc, 
      pdp->iTraceLeft + (double)(iInc - 1) * pdp->dIncrem, yOld, 
      pdp->iTraceLeft + (double)iInc * pdp->dIncrem, y) ;
    yOld = y ;
    }
  }

static void RedoGraph (graph_D *dialog, simulation_data *sim_data, int iZoomNum, int iZoomDen, DRAW_PARAMS *pdp)
  {
  int cx = 0, cy = 0 ;
  
  CalculateDrawingArea (dialog, sim_data, iZoomNum, iZoomDen, &cx, &cy) ;
  CacheDrawingParams (pdp, cx, cy, iZoomNum, iZoomDen) ;
  gtk_widget_set_usize (dialog->daGraphs, cx, cy) ;
  
  gtk_widget_queue_clear (dialog->vpGraphs) ;
  
  paint_graph_window (dialog->daGraphs, pdp, sim_data, NULL) ;
  }

static gboolean set_current_colour (GdkColor *pclr, int iClr, int iClrMask)
  {
  GdkColormap *pgcm = gdk_colormap_get_system () ;
  
  switch (iClr)
    {
    case WHITE:
      pclr->red = 0xFFFF & iClrMask ;
      pclr->green = 0xFFFF & iClrMask ;
      pclr->blue = 0xFFFF & iClrMask ;
      break ;
      
    case BLACK:
      pclr->red = 0x0000 ;
      pclr->green = 0x0000 ;
      pclr->blue = 0x0000 ;
      break ;
    
    case GREEN:
      pclr->red = 0x0000 ;
      pclr->green = 0xFFFF & iClrMask ;
      pclr->blue = 0x0000 ;
      break ;

    case GREEN1:
      pclr->red = 0xF000 & iClrMask;
      pclr->green = 0x0FFF & iClrMask;
      pclr->blue = 0xF000 & iClrMask;
      break;

    case GREEN2:
      pclr->red = 0xFF00 & iClrMask;
      pclr->green = 0xAAFF & iClrMask;
      pclr->blue = 0xFF00 & iClrMask;
      break;

    case GREEN3:
      pclr->red = 0xFFF0 & iClrMask;
      pclr->green = 0xFFFF & iClrMask;
      pclr->blue = 0xFFF0 & iClrMask;
      break;

    case ORANGE:
      pclr->red = 0xFFFF & iClrMask;
      pclr->green = 0x6000 & iClrMask;
      pclr->blue = 0x5000 & iClrMask;
      break;

    case RED:
      pclr->red = 0xFFFF & iClrMask;
      pclr->green = 0x0000;
      pclr->blue = 0x0000;
      break;

    case BLUE:
      pclr->red = 0x0000;
      pclr->green = 0x0000;
      pclr->blue = 0xFFFF & iClrMask;
      break;

    case YELLOW:
      pclr->red = 0xFFFF & iClrMask;
      pclr->green = 0xFFFF & iClrMask;
      pclr->blue = 0x0000;
      break;
    }
  return gdk_color_alloc (pgcm, pclr) ;
  }
