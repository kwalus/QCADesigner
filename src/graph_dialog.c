//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// The graph dialog. This displays the waveforms and    //
// the bus values as present in the raw waveform data.  //
// The bus values are interpreted from the waveform     //
// data and an (upper,lower) threshhold pair chosen by  //
// the user. The graph dialog also allows the user to   //
// load and save simulation data.                       //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "support.h"
#include "exp_array.h"
#include "generic_utils.h"
#include "global_consts.h"
#include "graph_dialog_data.h"
#include "simulation.h"
#include "custom_widgets.h"
#include "print_graph_properties_dialog.h"
#include "print_util.h"
#include "print_preview.h"
#include "print.h"
#include "fileio.h"
#include "global_consts.h"
#include "file_selection_window.h"
#include "bus_layout_dialog.h"
#include "honeycomb_thresholds_dialog.h"
#include "objects/object_helpers.h"
#include "qcadstock.h"

// Table index offsets
#define TRACE_TABLE_MIN_X 0
#define TRACE_TABLE_MIN_Y 0
// Initial trace size request
#define TRACE_MIN_CX 300
#define TRACE_MIN_CY 30

enum
  {
  GRAPH_MODEL_COLUMN_VISIBLE = BUS_LAYOUT_MODEL_COLUMN_LAST,
  GRAPH_MODEL_COLUMN_RULER,
  GRAPH_MODEL_COLUMN_TRACE,
  GRAPH_MODEL_COLUMN_UI,
  GRAPH_MODEL_COLUMN_LAST
  } ;

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *sw ;
  GtkWidget *vp ;
  GtkWidget *hscroll ;
  GtkWidget *table_of_traces ;
  GtkWidget *tview ;
  GtkWidget *hpaned ;
  GtkWidget *lbl_status ;
  } graph_D ;

typedef struct
  {
  simulation_data *sim_data ;
  BUS_LAYOUT *bus_layout ;
  gboolean bFreeSourceData ;
  gboolean bFakeCells ;
  GtkTreeModel *model ;
  double dHCThreshLower ;
  double dHCThreshUpper ;
  int icDrawingArea ;
  int icUIWidget ;
  int cxDrawingArea ;
  int cyDrawingArea ;
  int cxUIWidget ;
  int cyUIWidget ;
  int cxMaxGiven ;
  int cyMaxGiven ;
  int bOneTime ;
  int icGraphLines ;
  int base ;
  double dScale ;
  } GRAPH_DIALOG_DATA ;

static graph_D graph = {NULL} ;

static print_graph_OP print_graph_options = {{612, 792, 72, 72, 72, 72, TRUE, TRUE, NULL}, TRUE, TRUE, 1, 1} ;

// x values from user hilighting 
static int x_beg = -1, x_old = -1, x_cur = -1 ;
static int beg_sample ;

static void create_graph_dialog (graph_D *dialog) ;
static GRAPH_DIALOG_DATA *graph_dialog_data_new (simulation_data *sim_data, BUS_LAYOUT *bus_layout, gboolean bOKToFree, double dThreshLower, double dThreshUpper, int base) ;
static void graph_dialog_data_free (GRAPH_DIALOG_DATA *gdd) ;
static void apply_graph_dialog_data (graph_D *dialog, GRAPH_DIALOG_DATA *dialog_data) ;
static void attach_graph_widgets (graph_D *dialog, GtkWidget *table, GtkWidget *trace, GtkWidget *ruler, GtkWidget *ui, int idx) ;
static void build_io_tables (simulation_data *sim_data, BUS_LAYOUT *bus_layout) ;
static void set_ruler_values (GtkWidget *ruler, int cxGiven, int cx, int old_offset, int xOffset, int icSamples) ;
static void draw_trace_reference_lines (GdkDrawable *dst, int cx, int cy) ;
static void set_trace_widget_visible (GtkWidget *trace, gboolean bVisible) ;
static void recalculate_honeycombs (GRAPH_DIALOG_DATA *gdd, gboolean bCalcHoneycombArrays, graph_D *dialog) ;
static gboolean create_waveform_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr) ;
static gboolean create_bus_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr, int base) ;
static gboolean create_graph_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr) ;
static void print_graph_data_init (PRINT_GRAPH_DATA *print_graph_data, GRAPH_DIALOG_DATA *gdd) ;

#ifdef STDIO_FILEIO
static void btnOpen_clicked (GtkWidget *widget, gpointer user_data) ;
static void btnSave_clicked (GtkWidget *widget, gpointer user_data) ;
static void btnPreview_clicked (GtkWidget *widget, gpointer user_data) ;
#endif /* def STDIO_FILEIO */
static void btnClose_clicked (GtkWidget *widget, gpointer user_data) ;
static void btnPrint_clicked (GtkWidget *widget, gpointer user_data) ;
static void btnThresh_clicked (GtkWidget *widget, gpointer user_data) ;
static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void set_bus_expanded (GtkTreeView *tview, GtkTreeIter *itrBus, GtkTreePath *tpath, gpointer data) ;
static void model_visible_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data) ;
static void btnShowBase_clicked (GtkWidget *widget, gpointer data) ;
static gboolean graph_widget_size_allocate (GtkWidget *widget, GtkAllocation *alloc, gpointer data) ;
static gboolean waveform_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
static gboolean honeycomb_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
static gboolean viewport_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data) ;
static gboolean trace_ruler_motion_event (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean trace_motion_update_rulers (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean graph_widget_one_time_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
static gboolean graph_widget_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean graph_widget_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean graph_widget_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean graph_widget_enter_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;
static gboolean graph_widget_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;

void show_graph_dialog (GtkWindow *parent, simulation_data *sim_data, BUS_LAYOUT *bus_layout, gboolean bOKToFree, gboolean bModal)
  {
  int base = 10 ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double dThreshLower = -0.5, dThreshUpper = 0.5 ;

  if (NULL == graph.dialog)
    create_graph_dialog (&graph) ;

  gtk_window_set_transient_for (GTK_WINDOW (graph.dialog), parent) ;

  if (NULL != (gdd = g_object_get_data (G_OBJECT (graph.dialog), "graph_dialog_data")))
    {
    dThreshLower = gdd->dHCThreshLower ;
    dThreshUpper = gdd->dHCThreshUpper ;
    base = gdd->base ;
    }

  g_object_set_data_full (G_OBJECT (graph.dialog), "graph_dialog_data",
    gdd = graph_dialog_data_new (sim_data, bus_layout, bOKToFree, dThreshLower, dThreshUpper, base),
    (GDestroyNotify)graph_dialog_data_free) ;

  apply_graph_dialog_data (&graph, gdd) ;

  gtk_widget_show (graph.dialog) ;

  if (bModal)
    while (GTK_WIDGET_VISIBLE (graph.dialog))
      gtk_main_iteration () ;
  }

static void apply_graph_dialog_data (graph_D *dialog, GRAPH_DIALOG_DATA *dialog_data)
  {
  GtkWidget *trace_ui_widget = NULL, *trace_drawing_widget = NULL, *trace_ruler_widget = NULL ;
  int row_type ;
  GtkTreeIter itr ;
  int idxTbl = 0 ;
  GtkRequisition rq ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tview), dialog_data->model) ;

  gtk_widget_size_request (dialog->tview, &rq) ;

  gtk_paned_set_position (GTK_PANED (dialog->hpaned), rq.width) ;

  gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tview)) ;

  if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return ;
  while (TRUE)
    {
    gtk_tree_model_get (dialog_data->model, &itr,
      BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
      GRAPH_MODEL_COLUMN_TRACE,     &trace_drawing_widget,
      GRAPH_MODEL_COLUMN_RULER,     &trace_ruler_widget,
      GRAPH_MODEL_COLUMN_UI,        &trace_ui_widget, -1) ;
    attach_graph_widgets (dialog, dialog->table_of_traces, trace_drawing_widget, trace_ruler_widget, trace_ui_widget, idxTbl++) ;
    g_object_set_data (G_OBJECT (trace_drawing_widget), "label", dialog->lbl_status) ;
    g_object_set_data (G_OBJECT (trace_drawing_widget), "ruler", trace_ruler_widget) ;
    g_object_set_data (G_OBJECT (trace_ruler_widget), "label", dialog->lbl_status) ;
    if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
    }
  dialog_data->icGraphLines = idxTbl ;
  }

static void attach_graph_widgets (graph_D *dialog, GtkWidget *table, GtkWidget *trace, GtkWidget *ruler, GtkWidget *ui, int idxTbl)
  {
  gtk_widget_show (trace) ;
  gtk_widget_set_redraw_on_allocate (trace, FALSE) ;
  gtk_table_attach (GTK_TABLE (table), trace,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_widget_show (ruler) ;
  gtk_table_attach (GTK_TABLE (table), ruler,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + (idxTbl << 1),
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_widget_show (ui) ;
  gtk_table_attach (GTK_TABLE (table), ui,
    TRACE_TABLE_MIN_X + 1,
    TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  g_signal_connect (G_OBJECT (trace), "motion-notify-event", (GCallback)trace_motion_update_rulers,   dialog) ;
  g_signal_connect (G_OBJECT (ruler), "motion-notify-event", (GCallback)trace_ruler_motion_event,     dialog) ;
  g_signal_connect (G_OBJECT (trace), "expose-event",        (GCallback)graph_widget_one_time_expose, dialog) ;
  g_signal_connect (G_OBJECT (trace), "size-allocate",       (GCallback)graph_widget_size_allocate,   dialog) ;
  g_signal_connect (G_OBJECT (ui),    "size-allocate",       (GCallback)graph_widget_size_allocate,   dialog) ;
  }

// Extremely hacky solution to the problem of making all traces the same size
static gboolean graph_widget_one_time_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  GtkWidget *trace = NULL ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkTreeIter itr ;

  if (NULL == dialog) return FALSE ;

  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return FALSE ;

  if (gdd->bOneTime)
    {
    gdd->bOneTime = FALSE ;
    if (gtk_tree_model_get_iter_first (gdd->model, &itr))
      {
      while (TRUE)
        {
        gtk_tree_model_get (gdd->model, &itr,
          GRAPH_MODEL_COLUMN_TRACE, &trace, -1) ;
        if (GTK_WIDGET_VISIBLE (trace))
          {
          gtk_widget_hide (trace) ;
          gtk_widget_show (trace) ;
          }
        if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
        }
      }
    }

  g_signal_handlers_disconnect_matched (G_OBJECT (widget), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)graph_widget_one_time_expose, NULL) ;

  return FALSE ;
  }

static void btnShowBase_clicked (GtkWidget *widget, gpointer data)
  {
  graph_D *dialog = (graph_D *)data ;
  int base = (int)g_object_get_data (G_OBJECT (widget), "base") ;
  GRAPH_DIALOG_DATA *gdd = NULL ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;
  if (0 == base) return ;
  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;

  gdd->base = base ;

  recalculate_honeycombs (gdd, FALSE, dialog) ;
  }

static void model_visible_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data)
  {
  GtkTreePath *tp = NULL ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkTreeIter itr ;
  gboolean bModelVisible ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;
  if (NULL == (tp = gtk_tree_path_new_from_string (pszTreePath))) return ;

  gtk_tree_model_get_iter (gdd->model, &itr, tp) ;

  gtk_tree_model_get (gdd->model, &itr,
    GRAPH_MODEL_COLUMN_VISIBLE, &bModelVisible,
    GRAPH_MODEL_COLUMN_TRACE, &trace,
    GRAPH_MODEL_COLUMN_RULER, &ruler,
    GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

  bModelVisible = !bModelVisible ;

  set_trace_widget_visible (trace, bModelVisible) ;
  GTK_WIDGET_SET_VISIBLE (ruler, bModelVisible) ;
  GTK_WIDGET_SET_VISIBLE (ui, bModelVisible) ;

  gtk_tree_store_set (GTK_TREE_STORE (gdd->model), &itr, GRAPH_MODEL_COLUMN_VISIBLE, bModelVisible, -1) ;

  gtk_tree_path_free (tp) ;
  }

static gboolean trace_motion_update_rulers (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GtkTreeIter itr ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkWidget *ruler = NULL ;
  GtkWidget *trace = NULL ;
  double lower, upper, position, max_size ;
  int cxTrace = -1, cyTrace = -1 ;
  int x, y ;
  GdkModifierType mask ;

  if (NULL == dialog) return FALSE ;

  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return FALSE ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return FALSE ;

  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_RULER, &ruler,
      GRAPH_MODEL_COLUMN_TRACE, &trace, -1) ;
    gdk_window_get_size (trace->window, &cxTrace, &cyTrace) ;
    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
    gtk_ruler_set_range (GTK_RULER (ruler), lower, upper, lower + (upper - lower) * (((double)(event->x)) / (double)cxTrace), max_size) ;
    if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
    }

  gdk_window_get_pointer (widget->window, &x, &y, &mask) ;

  return FALSE ;
  }

static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  GtkTreeIter itr ;
  GtkWidget *drawing_area = NULL, *ruler = NULL ;
  GRAPH_DATA *graph_data = NULL ;
  GRAPH_DIALOG_DATA *gdd = g_object_get_data (G_OBJECT (data), "graph_dialog_data") ;
  int cx, cy, old_offset = -1 ;

  if (NULL == gdd) return ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return ;

  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_TRACE, &drawing_area,
      GRAPH_MODEL_COLUMN_RULER, &ruler, -1) ;
    if (NULL != drawing_area)
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (drawing_area), "graph_data")))
        {
        old_offset = graph_data->xOffset ;
        graph_data->xOffset = -(adj->value) ;
        gtk_widget_queue_draw (drawing_area) ;
        if (NULL != drawing_area->window)
          {
          gdk_window_get_size (drawing_area->window, &cx, &cy) ;
          set_ruler_values (ruler, graph_data->cxGiven, cx, old_offset, graph_data->xOffset, gdd->sim_data->number_samples) ;
          }
        }
    if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
    }
  }

static gboolean trace_ruler_motion_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  double lower, upper, position, max_size ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  GtkTreeIter itr ;
  GtkWidget *ruler = NULL ;

  if (NULL == dialog) return FALSE ;

  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return FALSE ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return FALSE ;

  gtk_ruler_get_range (GTK_RULER (widget), &lower, &upper, &position, &max_size) ;

  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr, GRAPH_MODEL_COLUMN_RULER, &ruler, -1) ;
    if (ruler != widget)
      gtk_ruler_set_range (GTK_RULER (ruler), lower, upper, position, max_size) ;
    if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) return FALSE ;
    }

  return FALSE ;
  }

static gboolean waveform_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  WAVEFORM_DATA *wf = g_object_get_data (G_OBJECT (widget), "graph_data") ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)data ;
  int Nix ;
  int idxStart = -1 ;
  int ic = 0 ;
  int cx, cy ;

  if (NULL == wf) return FALSE ;
  if (NULL == gdd) return FALSE ;

  gdk_window_get_size (widget->window, &cx, &cy) ;

  if (wf->graph_data.bNeedCalc)
    calculate_waveform_coords (wf, gdd->sim_data->number_samples) ;
//    calculate_waveform_coords (wf, gdd->sim_data->number_samples, gdd->dScale) ;

  gc = gdk_gc_new (widget->window) ;

  gdk_gc_set_foreground (gc, &(wf->graph_data.clr)) ;
  gdk_gc_set_background (gc, &(wf->graph_data.clr)) ;

  for (Nix = 0 ; Nix < wf->arPoints->icUsed ; Nix++)
    exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x += wf->graph_data.xOffset ;

  for (idxStart = 0 ; idxStart < wf->arPoints->icUsed ; idxStart++)
    if (exp_array_index_1d (wf->arPoints, GdkPoint, idxStart).x >= 0)
      break ;

  idxStart = CLAMP (idxStart - 1, 0, wf->arPoints->icUsed - 1) ;

  for (Nix = idxStart ; Nix < wf->arPoints->icUsed ; Nix++, ic++)
    if (exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x >= cx)
      break ;

  ic = CLAMP (ic + 1, 0, wf->arPoints->icUsed - idxStart) ;

  draw_trace_reference_lines (widget->window, cx, cy) ;

  gdk_draw_lines (widget->window, gc, (GdkPoint *)&(exp_array_index_1d (wf->arPoints, GdkPoint, idxStart)), ic);

  for (Nix = wf->arPoints->icUsed - 1 ; Nix > -1 ; Nix--)
    exp_array_index_1d (wf->arPoints, GdkPoint, Nix).x -= wf->graph_data.xOffset ;

  g_object_unref (gc) ;
  return FALSE ;
  }

static gboolean honeycomb_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  int Nix ;
  GdkGC *gc = NULL ;
  HONEYCOMB_DATA *hc = g_object_get_data (G_OBJECT (widget), "graph_data") ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)data ;
  int cx, cy, cxText, cyText ;
  HONEYCOMB *hcCur = NULL ;
  int xCur = 0 ;
  char *psz = NULL ;

  if (NULL == hc) return FALSE ;

  gdk_window_get_size (widget->window, &cx, &cy) ;

  if (hc->graph_data.bNeedCalc)
    calculate_honeycomb_coords (hc, gdd->sim_data->number_samples) ;

  draw_trace_reference_lines (widget->window, cx, cy) ;

  gc = gdk_gc_new (widget->window) ;
  gdk_gc_set_foreground (gc, &(hc->graph_data.clr)) ;
  gdk_gc_set_background (gc, &(hc->graph_data.clr)) ;

  if (0 == hc->arHCs->icUsed)
    gdk_draw_line (widget->window, gc, 0, cy >> 1, cx, cy >> 1) ;
  else
    {
    for (Nix = 0 ; Nix < hc->arHCs->icUsed ; Nix++)
      {
      hcCur = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;
      xCur += hc->graph_data.xOffset ;
      hcCur->pts[0].x += hc->graph_data.xOffset ;
      if ((xCur >= 0 && xCur <= cx) || (hcCur->pts[0].x >= 0 && xCur <= cx))
        // Need gdk_draw_line_in_rect
        gdk_draw_line (widget->window, gc, xCur, hcCur->pts[0].y, hcCur->pts[0].x, hcCur->pts[0].y) ;
      xCur -= hc->graph_data.xOffset ;
      hcCur->pts[0].x -= hc->graph_data.xOffset ;

      hcCur->pts[0].x += hc->graph_data.xOffset ;
      hcCur->pts[1].x += hc->graph_data.xOffset ;
      hcCur->pts[2].x += hc->graph_data.xOffset ;
      hcCur->pts[3].x += hc->graph_data.xOffset ;
      hcCur->pts[4].x += hc->graph_data.xOffset ;
      hcCur->pts[5].x += hc->graph_data.xOffset ;

      if (RECT_INTERSECT_RECT (0, 0, cx, cy, hcCur->pts[0].x, 0, hcCur->pts[3].x - hcCur->pts[0].x + 1, cy))
        {
        gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (HONEYCOMB_BACKGROUND)) ;
        gdk_gc_set_background (gc, clr_idx_to_clr_struct (HONEYCOMB_BACKGROUND)) ;
        gdk_draw_polygon (widget->window, gc, TRUE, hcCur->pts, 6) ;
        gdk_gc_set_foreground (gc, &(hc->graph_data.clr)) ;
        gdk_gc_set_background (gc, &(hc->graph_data.clr)) ;
        get_string_dimensions (psz = strdup_convert_to_base (hcCur->value, gdd->base), FONT_STRING, &cxText, &cyText) ;
        draw_string (widget->window, gc, FONT_STRING,
          (hcCur->pts[3].x + hcCur->pts[0].x - cxText) >> 1,
          (hcCur->pts[1].y + hcCur->pts[4].y - cyText) >> 1, psz) ;
        g_free (psz) ;
        gdk_draw_polygon (widget->window, gc, FALSE, hcCur->pts, 6) ;
        }

      hcCur->pts[0].x -= hc->graph_data.xOffset ;
      hcCur->pts[1].x -= hc->graph_data.xOffset ;
      hcCur->pts[2].x -= hc->graph_data.xOffset ;
      hcCur->pts[3].x -= hc->graph_data.xOffset ;
      hcCur->pts[4].x -= hc->graph_data.xOffset ;
      hcCur->pts[5].x -= hc->graph_data.xOffset ;

      xCur = hcCur->pts[3].x ;
      }
    gdk_draw_line (widget->window, gc,
      xCur + hc->graph_data.xOffset, hcCur->pts[3].y,
      hc->graph_data.cxGiven + hc->graph_data.xOffset, hcCur->pts[3].y) ;
    }

  g_object_unref (gc) ;
  return FALSE ;
  }

static gboolean graph_widget_size_allocate (GtkWidget *widget, GtkAllocation *alloc, gpointer data)
  {
  GtkTreeIter itr ;
  GtkWidget *widget_to_size = NULL ;
  GtkWidget *ruler = NULL ;
  int *pcx = NULL, *pcy = NULL, *pic = NULL ;
  graph_D *dialog = (graph_D *)data ;
  GRAPH_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data") ;
  int model_column = -1 ;
  GRAPH_DATA *graph_data = NULL ;
  GtkAdjustment *adj = NULL ;

  if (NULL == dialog_data) return FALSE ;

  if (GTK_IS_DRAWING_AREA (widget))
    {
    pcx = &(dialog_data->cxDrawingArea) ;
    pcy = &(dialog_data->cyDrawingArea) ;
    pic = &(dialog_data->icDrawingArea) ;
    model_column = GRAPH_MODEL_COLUMN_TRACE ;
    }
  else
    {
    pcx = &(dialog_data->cxUIWidget) ;
    pcy = &(dialog_data->cyUIWidget) ;
    pic = &(dialog_data->icUIWidget) ;
    model_column = GRAPH_MODEL_COLUMN_UI ;
    }

  if ((*pic) == dialog_data->icGraphLines) return FALSE ;

  (*pcx) = MAX ((*pcx), alloc->width) ;
  (*pcy) = MAX ((*pcy), alloc->height) ;
  (*pic)++ ;
  if ((*pic) == dialog_data->icGraphLines)
    {
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, model_column, &widget_to_size, -1) ;
      gtk_widget_set_size_request (widget_to_size, (GRAPH_MODEL_COLUMN_TRACE == model_column) ? -1 : (*pcx), (NULL == pcy) ? -1 : (*pcy)) ;
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    }

  if (GTK_IS_DRAWING_AREA (widget))
    {
    dialog_data->cxMaxGiven = dialog_data->cxDrawingArea ;
    dialog_data->cyMaxGiven = dialog_data->cyDrawingArea ;
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, GRAPH_MODEL_COLUMN_TRACE, &widget_to_size, -1) ;
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (widget_to_size), "graph_data")))
        {
        dialog_data->cxMaxGiven = MAX (dialog_data->cxMaxGiven, graph_data->cxWanted) ;
        dialog_data->cyMaxGiven = MAX (dialog_data->cyMaxGiven, graph_data->cyWanted) ;
        }
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    if (!gtk_tree_model_get_iter_first (dialog_data->model, &itr)) return FALSE ;
    while (TRUE)
      {
      gtk_tree_model_get (dialog_data->model, &itr, GRAPH_MODEL_COLUMN_TRACE, &widget_to_size,
        GRAPH_MODEL_COLUMN_RULER, &ruler, -1) ;
      if (NULL != (graph_data = g_object_get_data (G_OBJECT (widget_to_size), "graph_data")))
        {
        graph_data->cxGiven = dialog_data->cxMaxGiven ;
        graph_data->cyGiven = dialog_data->cyMaxGiven ;
        graph_data->bNeedCalc = TRUE ;
        set_ruler_values (ruler, dialog_data->cxMaxGiven, dialog_data->cxDrawingArea, graph_data->xOffset, graph_data->xOffset, dialog_data->sim_data->number_samples) ;
        }
      if (!gtk_tree_model_iter_next_dfs (dialog_data->model, &itr)) break ;
      }
    if (NULL != (adj = gtk_range_get_adjustment (GTK_RANGE (dialog->hscroll))))
      {
      adj->lower = 0 ;
      adj->upper = dialog_data->cxMaxGiven ;
      adj->page_increment =
      adj->page_size = dialog_data->cxDrawingArea ;
      adj->step_increment = adj->page_size / 10.0 ;
      adj->value = CLAMP (adj->value, adj->lower, adj->upper - adj->page_size) ;
      gtk_adjustment_changed (adj) ;
      gtk_adjustment_value_changed (adj) ;
      }
    dialog_data->icDrawingArea =
    dialog_data->cxDrawingArea =
    dialog_data->cyDrawingArea = 0 ;
    }
  return FALSE ;
  }

static void btnPrint_clicked (GtkWidget *widget, gpointer user_data)
  {
  PRINT_GRAPH_DATA pgd = {NULL, NULL, NULL, 10} ;
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data") ;

  print_graph_data_init (&pgd, gdd) ;

  if (get_print_graph_properties_from_user (GTK_WINDOW (dialog->dialog), &print_graph_options, &pgd))
    print_graphs (&print_graph_options, &pgd) ;

  exp_array_free (pgd.bus_traces) ;
  }

static void print_graph_data_init (PRINT_GRAPH_DATA *print_graph_data, GRAPH_DIALOG_DATA *gdd)
  {
  GtkTreeIter itr ;
  GtkWidget *trace = NULL ;
  int row_type = 0 ;
  HONEYCOMB_DATA *hc = NULL ;

  if (NULL == print_graph_data || NULL == gdd) return ;

  print_graph_data->sim_data = gdd->sim_data ;
  print_graph_data->bus_layout = gdd->bus_layout ;
  print_graph_data->honeycomb_base = gdd->base ;
  print_graph_data->bus_traces = exp_array_new (sizeof (HONEYCOMB_DATA *), 1) ;
  if (gtk_tree_model_get_iter_first (gdd->model, &itr))
    {
    while (TRUE)
      {
      gtk_tree_model_get (gdd->model, &itr,
        GRAPH_MODEL_COLUMN_TRACE, &trace,
        BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

      if (ROW_TYPE_BUS & row_type)
        {
        hc = g_object_get_data (G_OBJECT (trace), "graph_data") ;
        exp_array_insert_vals (print_graph_data->bus_traces, &hc, 1, 1, -1) ;
        }

      if (!gtk_tree_model_iter_next (gdd->model, &itr)) break ;
      }
    }
  }

#ifdef STDIO_FILEIO
static void btnPreview_clicked (GtkWidget *widget, gpointer user_data)
  {
  PRINT_GRAPH_DATA pgd = {NULL, NULL, NULL, 10} ;
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = (GRAPH_DIALOG_DATA *)g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data") ;

  if (NULL == gdd) return ;

  print_graph_data_init (&pgd, gdd) ;

  init_print_graph_options (&print_graph_options, &pgd) ;

  do_print_preview ((print_OP *)(&print_graph_options), GTK_WINDOW (dialog->dialog), &pgd, (PrintFunction)print_graphs) ;

  exp_array_free (pgd.bus_traces) ;
  }

static void btnOpen_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)user_data ;
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT *sim_output = NULL ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double dThreshLower = -0.5, dThreshUpper = 0.5 ;
  int base = 10 ;

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Open Simulation Results"), pszFName, FALSE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  if (NULL == (sim_output = open_simulation_output_file (pszFName)))
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
      _("Failed to open simulation data file %s !\n"), pszFName))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return ;
    }

  if (NULL != (gdd = g_object_get_data (G_OBJECT (graph.dialog), "graph_dialog_data")))
    {
    dThreshLower = gdd->dHCThreshLower ;
    dThreshUpper = gdd->dHCThreshUpper ;
    base = gdd->base ;
    }

  g_object_set_data_full (G_OBJECT (dialog->dialog), "graph_dialog_data",
    gdd = graph_dialog_data_new (sim_output->sim_data, sim_output->bus_layout, TRUE, dThreshLower, dThreshUpper, base),
    (GDestroyNotify)graph_dialog_data_free) ;

  apply_graph_dialog_data (dialog, gdd) ;
  }

static void btnSave_clicked (GtkWidget *widget, gpointer user_data)
  {
  GRAPH_DIALOG_DATA *gdd = g_object_get_data (G_OBJECT (user_data), "graph_dialog_data") ;
  static char *pszFName = NULL ;
  char *pszTempFName = NULL ;
  SIMULATION_OUTPUT sim_output = {NULL, NULL} ;

  if (NULL == gdd) return ;

  if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (user_data), _("Save Simulation Results"), pszFName, TRUE)))
    return ;

  if (NULL != pszFName) g_free (pszFName) ;

  pszFName = pszTempFName ;

  sim_output.sim_data = gdd->sim_data ;
  sim_output.bus_layout = gdd->bus_layout ;

  create_simulation_output_file (pszFName, &sim_output) ;
  }
#endif /* def STDIO_FILEIO */

static void btnClose_clicked (GtkWidget *widget, gpointer user_data)
  {
  GtkWidget *dlgGraphs = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "dlgGraphs")) ;

  // Since widget can be one of btnClose (if called via "clicked") or the main window
  // (if called via delete_event), we must make sure we have a pointer to the main
  // window.  Since btnClose has a data member dlgGraphs and the main window does not,
  // and the main window /is/ widget when called via delete_event, the following will
  // make sure that dlgGraphs always points to the main window
  if (NULL == dlgGraphs) dlgGraphs = widget ;

  gtk_widget_hide (GTK_WIDGET (dlgGraphs)) ;
  }

static void btnThresh_clicked (GtkWidget *widget, gpointer user_data)
  {
  graph_D *dialog = (graph_D *)user_data ;
  GRAPH_DIALOG_DATA *gdd = NULL ;
  double lower, upper ;

  if (NULL == dialog) return ;
  if (NULL == (gdd = g_object_get_data (G_OBJECT (dialog->dialog), "graph_dialog_data"))) return ;

  lower = gdd->dHCThreshLower ;
  upper = gdd->dHCThreshUpper ;

  if (get_honeycomb_thresholds_from_user (dialog->dialog, &lower, &upper))
    {
    gdd->bOneTime = TRUE ;
    gdd->dHCThreshLower = lower ;
    gdd->dHCThreshUpper = upper ;
    recalculate_honeycombs (gdd, TRUE, dialog) ;
    }
  }

static void set_bus_expanded (GtkTreeView *tview, GtkTreeIter *itrBus, GtkTreePath *tpath, gpointer data)
  {
  GtkTreeIter itr ;
  GtkTreeModel *tm = gtk_tree_view_get_model (tview) ;
  gboolean bVisible = (gboolean)data ;
  gboolean bModelVisible = TRUE ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  if (NULL == tm) return ;

  if (!gtk_tree_model_iter_children (tm, &itr, itrBus)) return ;

  while (TRUE)
    {
    gtk_tree_model_get (tm, &itr,
      GRAPH_MODEL_COLUMN_VISIBLE, &bModelVisible,
      GRAPH_MODEL_COLUMN_TRACE, &trace,
      GRAPH_MODEL_COLUMN_RULER, &ruler,
      GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

    set_trace_widget_visible (trace, bVisible && bModelVisible) ;
    GTK_WIDGET_SET_VISIBLE (ruler, bVisible && bModelVisible) ;
    GTK_WIDGET_SET_VISIBLE (ui, bVisible && bModelVisible) ;
    if (!gtk_tree_model_iter_next (tm, &itr)) return ;
    }
  }

static gboolean viewport_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data)
  {
  graph_D *dialog = (graph_D *)data ;
  double dNewVal ;
  GtkAdjustment *adj = NULL ;
  GdkScrollDirection scroll_direction =
    (GDK_SCROLL_UP   == event->direction && (event->state & GDK_CONTROL_MASK)) ? GDK_SCROLL_LEFT  :
    (GDK_SCROLL_DOWN == event->direction && (event->state & GDK_CONTROL_MASK)) ? GDK_SCROLL_RIGHT : event->direction ;

  if (GDK_SCROLL_UP == scroll_direction || GDK_SCROLL_DOWN == scroll_direction)
    {
    adj = gtk_viewport_get_vadjustment (GTK_VIEWPORT (widget)) ;
    dNewVal = adj->value +
      (adj->step_increment * ((GDK_SCROLL_UP == scroll_direction) ? (-1.0) : (1.0))) ;
    gtk_adjustment_set_value (adj, CLAMP (dNewVal, adj->lower, adj->upper - adj->page_size)) ;
    }
  else
  if (GDK_SCROLL_LEFT == scroll_direction || GDK_SCROLL_RIGHT == scroll_direction)
    {
    adj = gtk_range_get_adjustment (GTK_RANGE (dialog->hscroll)) ;
    dNewVal = adj->value +
      (adj->step_increment * ((GDK_SCROLL_LEFT == scroll_direction) ? (-1.0) : (1.0))) ;
    gtk_adjustment_set_value (adj, CLAMP (dNewVal, adj->lower, adj->upper - adj->page_size)) ;
    }
  return FALSE ;
  }

static gboolean graph_widget_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  double lower, upper, position, max_size ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  if (1 != event->button) return FALSE ;

  gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  beg_sample = (int)position ;
  
  x_beg = 
  x_old =
  x_cur = event->x ;
  return TRUE ;
  }

static gboolean graph_widget_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  int cx = 0, cy = 0 ;
  int x, y ;
  GdkModifierType mask ;
  double lower, upper, position, max_size ;
  char *psz = NULL ;
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;

  if (event->state & GDK_BUTTON1_MASK)
    {
    gc = gdk_gc_new (widget->window) ;
    gdk_gc_set_function (gc, GDK_XOR) ;
    gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (WHITE)) ;
    gdk_gc_set_background (gc, clr_idx_to_clr_struct (WHITE)) ;
    gdk_window_get_size (widget->window, &cx, &cy) ;

    if (x_beg != x_old)
      gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;
    x_old = event->x ;
    if (x_beg != x_old)
      gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;

    g_object_unref (gc) ;

    gtk_label_set_text (GTK_LABEL (label),
      psz = g_strdup_printf ("%s %d - %d", _("Sample"), MIN (beg_sample, (int)position), MAX (beg_sample, (int)position))) ;
    g_free (psz) ;
    }
  else
  if (!(NULL == label || NULL == ruler))
    {
    gtk_label_set_text (GTK_LABEL (label),
      psz = g_strdup_printf ("%s %d", _("Sample"), (int)position)) ;
    g_free (psz) ;
    }

  gdk_window_get_pointer (widget->window, &x, &y, &mask) ;

  return FALSE ;
  }

static gboolean graph_widget_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GdkGC *gc = NULL ;
  int cx = 0, cy = 0 ;
  double lower, upper, position, max_size ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  if (1 != event->button) return FALSE ;

  gc = gdk_gc_new (widget->window) ;
  gdk_gc_set_function (gc, GDK_XOR) ;
  gdk_gc_set_foreground (gc, clr_idx_to_clr_struct (WHITE)) ;
  gdk_gc_set_background (gc, clr_idx_to_clr_struct (WHITE)) ;
  gdk_window_get_size (widget->window, &cx, &cy) ;

  if (x_beg != x_old)
    gdk_draw_rectangle (widget->window, gc, TRUE, MIN (x_beg, x_old), 0, ABS (x_beg - x_old), cy) ;
  x_old = event->x ;

  g_object_unref (gc) ;

  if (NULL != ruler)
    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  // desired range goes from beg_sample to (int)position
  return TRUE ;
  }

static gboolean graph_widget_enter_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  char *psz = NULL ;
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;
  GtkWidget *ruler = g_object_get_data (G_OBJECT (widget), "ruler") ;

  if (!(NULL == label || NULL == ruler))
    {
    double lower, upper, position, max_size ;

    gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
    gtk_label_set_text (GTK_LABEL (label),
      psz = g_strdup_printf ("%s %d", _("Sample"), (int)position)) ;
    g_free (psz) ;
    }

  return FALSE ;
  }

static gboolean graph_widget_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  GtkWidget *label = g_object_get_data (G_OBJECT (widget), "label") ;

  if (NULL != widget)
    gtk_label_set_text (GTK_LABEL (label), "") ;

  return FALSE ;
  }

////////////////////////////////////////////////////////////////////////////////////////////////////

static GRAPH_DIALOG_DATA *graph_dialog_data_new (simulation_data *sim_data, BUS_LAYOUT *bus_layout, gboolean bOKToFree, double dThreshLower, double dThreshUpper, int base)
  {
  GtkTreeStore *ts = NULL ;
  GtkTreeIter itr ;
  GRAPH_DIALOG_DATA *graph_dialog_data = NULL ;

  if (NULL == sim_data || NULL == bus_layout) return NULL ;

  graph_dialog_data = g_malloc0 (sizeof (GRAPH_DIALOG_DATA)) ;

  graph_dialog_data->sim_data        = sim_data ;
  graph_dialog_data->bus_layout      = bus_layout ;
  if ((graph_dialog_data->bFakeCells = (0 == bus_layout->inputs->icUsed || 0 == bus_layout->outputs->icUsed)))
    build_io_tables (sim_data, bus_layout) ;
  graph_dialog_data->bFreeSourceData = bOKToFree ;
  graph_dialog_data->model           =
    GTK_TREE_MODEL (ts = create_bus_layout_tree_store (bus_layout,
      5, G_TYPE_BOOLEAN, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER)) ;
  graph_dialog_data->dHCThreshLower  = dThreshLower ;
  graph_dialog_data->dHCThreshUpper  = dThreshUpper ;
  graph_dialog_data->icDrawingArea   =
  graph_dialog_data->icUIWidget      =
  graph_dialog_data->cyDrawingArea   =
  graph_dialog_data->cxUIWidget      =
  graph_dialog_data->cyUIWidget      =
  graph_dialog_data->icGraphLines    =  0 ;
  graph_dialog_data->bOneTime        = TRUE ;
  graph_dialog_data->base            = base ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 0"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 0, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 1"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 1, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 2"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 2, -1) ;

  gtk_tree_store_append (GTK_TREE_STORE (graph_dialog_data->model), &itr, NULL) ;
  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), &itr,
    BUS_LAYOUT_MODEL_COLUMN_ICON, QCAD_STOCK_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_NAME, _("Clock 3"),
    BUS_LAYOUT_MODEL_COLUMN_TYPE, ROW_TYPE_CLOCK,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, 3, -1) ;

  if (!gtk_tree_model_get_iter_first (graph_dialog_data->model, &itr))
    {
    g_object_unref (graph_dialog_data->model) ;
    g_free (graph_dialog_data) ;
    return NULL ;
    }

  // Add the widgets corresponding to the model lines
  while (create_graph_widgets (graph_dialog_data, &itr)) ;

  return graph_dialog_data ;
  }

static void graph_dialog_data_free (GRAPH_DIALOG_DATA *gdd)
  {
  int Nix ;
  GtkTreeIter itr ;
  GtkWidget *trace = NULL, *ruler = NULL, *ui = NULL ;

  // Destroy the widgets
  if (gtk_tree_model_get_iter_first (gdd->model, &itr))
    {
    while (TRUE)
      {
      gtk_tree_model_get (gdd->model, &itr,
        GRAPH_MODEL_COLUMN_TRACE, &trace,
        GRAPH_MODEL_COLUMN_RULER, &ruler,
        GRAPH_MODEL_COLUMN_UI, &ui, -1) ;

      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (trace)), trace) ;
      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (ruler)), ruler) ;
      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (ui)), ui) ;

      if (!gtk_tree_model_iter_next_dfs (gdd->model, &itr)) break ;
      }
    }

  // if the cells are fake, destroy them
  if (gdd->bFakeCells)
    {
    for (Nix = 0 ; Nix < gdd->bus_layout->inputs->icUsed ; Nix++)
      g_object_unref (exp_array_index_1d (gdd->bus_layout->inputs, BUS_LAYOUT_CELL, Nix).cell) ;
    for (Nix = 0 ; Nix < gdd->bus_layout->outputs->icUsed ; Nix++)
      g_object_unref (exp_array_index_1d (gdd->bus_layout->outputs, BUS_LAYOUT_CELL, Nix).cell) ;
    }

  // If it's OK to free the data, free it.
  if (gdd->bFreeSourceData)
    {
    design_bus_layout_free (gdd->bus_layout) ;
    simulation_data_destroy (gdd->sim_data) ;
    }

  // Destroy the model
  g_object_unref (gdd->model) ;

  // Free the structure itself
  g_free (gdd) ;
  }

static gboolean create_graph_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr)
  {
  int row_type = -1 ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  if (ROW_TYPE_CELL & row_type)
    return create_waveform_widgets (graph_dialog_data, itr) ;
  else
  if (ROW_TYPE_BUS & row_type)
    return create_bus_widgets (graph_dialog_data, itr, graph_dialog_data->base) ;
  else
  if (ROW_TYPE_CLOCK == row_type)
    return create_waveform_widgets (graph_dialog_data, itr) ;
  else
    return FALSE ;
  }

static gboolean create_waveform_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr)
  {
  int idx = -1, row_type ;
  GtkWidget *tbl = NULL, *lbl = NULL ;
  GtkWidget *trace_ui_widget = NULL, *trace_ruler_widget = NULL, *trace_drawing_widget = NULL ;
  WAVEFORM_DATA *wf = NULL ;
  double dMin = -1.0, dMax = 1.0 ;
  char *psz = NULL ;

  gtk_tree_model_get (graph_dialog_data->model, itr,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx,
    BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  wf = waveform_data_new (
    ROW_TYPE_CLOCK == row_type
      ? &(graph_dialog_data->sim_data->clock_data[idx])
      : &(graph_dialog_data->sim_data->trace[idx + (row_type & ROW_TYPE_INPUT ? 0 : graph_dialog_data->bus_layout->inputs->icUsed)]),
      clr_idx_to_clr_struct (ROW_TYPE_CLOCK == row_type ? RED : (row_type & ROW_TYPE_INPUT) ? BLUE : YELLOW),
      ROW_TYPE_CLOCK == row_type) ;

  tracedata_get_min_max (wf->trace, 0, graph_dialog_data->sim_data->number_samples, &dMin, &dMax) ;

  trace_ui_widget = gtk_frame_new (NULL) ;
  tbl = gtk_table_new (3, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (trace_ui_widget), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (psz = g_strdup_printf ("max: %6.2e", dMax)) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.0) ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &psz, -1) ;
  lbl = gtk_label_new (psz) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  lbl = gtk_label_new (psz = g_strdup_printf ("min: %6.2e", dMin)) ;
  g_free (psz) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 1.0) ;

  trace_drawing_widget = gtk_drawing_area_new () ;
  gtk_widget_add_events (trace_drawing_widget, GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK) ;
  set_widget_background_colour (trace_drawing_widget, 0, 0, 0) ;
  g_object_set_data_full (G_OBJECT (trace_drawing_widget), "graph_data", wf, (GDestroyNotify)waveform_data_free) ;
  gtk_widget_set_size_request (trace_drawing_widget, -1, TRACE_MIN_CY) ;
  gtk_widget_add_events (trace_drawing_widget,
    GDK_EXPOSURE_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "expose-event", (GCallback)waveform_expose, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-press-event", (GCallback)graph_widget_button_press, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "motion-notify-event", (GCallback)graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-release-event", (GCallback)graph_widget_button_release, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "enter-notify-event", (GCallback)graph_widget_enter_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "leave-notify-event", (GCallback)graph_widget_leave_notify, graph_dialog_data) ;

  trace_ruler_widget = gtk_hruler_new () ;

  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), itr,
    GRAPH_MODEL_COLUMN_VISIBLE, TRUE,
    GRAPH_MODEL_COLUMN_RULER,   trace_ruler_widget,
    GRAPH_MODEL_COLUMN_TRACE,   trace_drawing_widget,
    GRAPH_MODEL_COLUMN_UI,      trace_ui_widget, -1) ;

  return gtk_tree_model_iter_next (graph_dialog_data->model, itr) ;
  }

static gboolean create_bus_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr, int base)
  {
  int idx, bus_type, offset = 0 ;
  GtkTreeIter itrChildren ;
  GtkWidget *trace_drawing_widget = NULL, *trace_ui_widget = NULL, *trace_ruler_widget = NULL, *tbl = NULL, *lbl = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  char *pszBusName = NULL ;
  struct TRACEDATA *trace = NULL ;

  if (!gtk_tree_model_iter_children (graph_dialog_data->model, &itrChildren, itr)) return FALSE ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &bus_type, -1) ;

  hc = honeycomb_data_new (clr_idx_to_clr_struct ((bus_type & ROW_TYPE_OUTPUT) ? YELLOW : BLUE)) ;

  if (bus_type & ROW_TYPE_OUTPUT) offset = graph_dialog_data->bus_layout->inputs->icUsed ;

  while (TRUE)
    {
    gtk_tree_model_get (graph_dialog_data->model, &itrChildren, BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx, -1) ;
    trace = &((graph_dialog_data->sim_data->trace)[idx + offset]) ;
    exp_array_insert_vals (hc->arTraces, &trace, 1, 1, -1) ;
    if (!gtk_tree_model_iter_next (graph_dialog_data->model, &itrChildren)) break ;
    }

  calculate_honeycomb_array (hc, graph_dialog_data->sim_data->number_samples, graph_dialog_data->dHCThreshLower, graph_dialog_data->dHCThreshUpper, base) ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &pszBusName, -1) ;

  trace_ui_widget = gtk_frame_new (NULL) ;
  tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (trace_ui_widget), tbl) ;

  lbl = gtk_label_new (pszBusName) ;
  g_free (pszBusName) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  trace_drawing_widget = gtk_drawing_area_new () ;
  gtk_widget_add_events (trace_drawing_widget, GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK) ;
  set_widget_background_colour (trace_drawing_widget, 0, 0, 0) ;
  g_object_set_data_full (G_OBJECT (trace_drawing_widget), "graph_data", hc, (GDestroyNotify)honeycomb_data_free) ;
  gtk_widget_set_size_request (trace_drawing_widget, -1, TRACE_MIN_CY) ;
  gtk_widget_add_events (trace_drawing_widget,
    GDK_EXPOSURE_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "expose-event", (GCallback)honeycomb_expose, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-press-event", (GCallback)graph_widget_button_press, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "motion-notify-event", (GCallback)graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-release-event", (GCallback)graph_widget_button_release, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "enter-notify-event", (GCallback)graph_widget_enter_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "leave-notify-event", (GCallback)graph_widget_leave_notify, graph_dialog_data) ;

  trace_ruler_widget = gtk_hruler_new () ;

  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), itr,
    GRAPH_MODEL_COLUMN_VISIBLE, TRUE,
    GRAPH_MODEL_COLUMN_RULER,   trace_ruler_widget,
    GRAPH_MODEL_COLUMN_TRACE,   trace_drawing_widget,
    GRAPH_MODEL_COLUMN_UI,      trace_ui_widget, -1) ;

  if (gtk_tree_model_iter_children (graph_dialog_data->model, &itrChildren, itr))
    while (create_waveform_widgets (graph_dialog_data, &itrChildren)) ;

  return gtk_tree_model_iter_next (graph_dialog_data->model, itr) ;
  }

static void create_graph_dialog (graph_D *dialog)
  {
  GtkWidget *table = NULL, *toolbar = NULL, *btn = NULL, *tbl_sw = NULL, *tbl_status = NULL,
    *tbl_vp = NULL, *vscroll = NULL, *sw_tview = NULL, *btnBaseRadioSource = NULL, *statusbar = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
	GtkAccelGroup *accel_group = NULL ;

	accel_group = gtk_accel_group_new () ;

  // The Window
  dialog->dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 800, 600);
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Simulation Results"));
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE);

  table = gtk_table_new (3, 1, FALSE) ;
  gtk_widget_show (table) ;
  gtk_container_add (GTK_CONTAINER (dialog->dialog), table) ;

  toolbar = gtk_toolbar_new () ;
  gtk_widget_show (toolbar) ;
  gtk_table_attach (GTK_TABLE (table), toolbar, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL) ;
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar), TRUE) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH) ;

  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Close"),
    _("Close Window"),
    _("Close simulation results window."),
    gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnClose_clicked,
    NULL) ;
  g_object_set_data (G_OBJECT (btn), "dlgGraphs", dialog->dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;
#ifdef STDIO_FILEIO
  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Open"),
    _("Open Simulation Results"),
    _("Open and display another set of simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnOpen_clicked,
    dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Save"),
    _("Save Simulation Results"),
    _("Save the displayed simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnSave_clicked,
    dialog->dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Print Preview"),
    _("Preview the print layout"),
    _("Converts graphs to PostScript and runs the previewer application."),
    gtk_image_new_from_stock (GTK_STOCK_PRINT_PREVIEW, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnPreview_clicked,
    dialog) ;

#endif /* def STDIO_FILEIO */
  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Print"),
    _("Print Graphs"),
    _("Converts graphs to PostScript and prints them to a file or a printer."),
    gtk_image_new_from_stock (GTK_STOCK_PRINT, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnPrint_clicked,
    dialog) ;

  // This will separate the layers combo from the clocks combo
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Thresholds..."),
    _("Set Thresholds"),
    _("Set thresholds for interpreting logical bits from polarizations."),
    gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnThresh_clicked,
    dialog) ;

  // This will separate the layers combo from the clocks combo
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  g_object_set_data (G_OBJECT (
    btnBaseRadioSource =gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      NULL,
      _("Decimal"),
      _("Show Values As Decimal"),
      _("Display honeycomb values in decimal."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_DEC, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)10) ;

  g_object_set_data (G_OBJECT (
    gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      btnBaseRadioSource,
      _("Binary"),
      _("Show Values As Binary"),
      _("Display honeycomb values in binary."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_BIN, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)2) ;

  g_object_set_data (G_OBJECT (
    gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      btnBaseRadioSource,
      _("Hex"),
      _("Show Values As Hexadecimal"),
      _("Display honeycomb values in hexadecimal."),
      gtk_image_new_from_stock (QCAD_STOCK_GRAPH_HEX, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnShowBase_clicked,
      dialog)),
    "base", (gpointer)16) ;

  dialog->hpaned = gtk_hpaned_new () ;
  gtk_widget_show (dialog->hpaned) ;
  gtk_table_attach (GTK_TABLE (table), dialog->hpaned, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  sw_tview = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (sw_tview) ;
  gtk_paned_add1 (GTK_PANED (dialog->hpaned), sw_tview) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw_tview), GTK_SHADOW_IN) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw_tview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC) ;

  dialog->tview = create_bus_layout_tree_view (TRUE, _("Trace"), GTK_SELECTION_SINGLE) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tview), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (col, _("Visible")) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  gtk_tree_view_column_add_attribute (col, cr, "active", GRAPH_MODEL_COLUMN_VISIBLE) ;
  g_object_set (G_OBJECT (cr), "activatable", TRUE, NULL) ;
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cr), TRUE) ;
  gtk_widget_show (dialog->tview) ;
  gtk_container_add (GTK_CONTAINER (sw_tview), dialog->tview) ;

  tbl_sw = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tbl_sw) ;
  gtk_paned_add2 (GTK_PANED (dialog->hpaned), tbl_sw) ;
  gtk_table_set_row_spacings (GTK_TABLE (tbl_sw), 2) ;
  gtk_table_set_col_spacings (GTK_TABLE (tbl_sw), 2) ;

  dialog->vp = gtk_viewport_new (NULL, NULL) ;
  gtk_widget_show (dialog->vp) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), dialog->vp, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0) ;
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (dialog->vp), GTK_SHADOW_IN) ;
  gtk_widget_set_size_request (dialog->vp, 0, 0) ;

  tbl_vp = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl_vp) ;
  gtk_container_add (GTK_CONTAINER (dialog->vp), tbl_vp) ;

  dialog->table_of_traces = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (dialog->table_of_traces) ;
  gtk_table_attach (GTK_TABLE (tbl_vp), dialog->table_of_traces, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(0), 0, 0) ;

  dialog->hscroll = gtk_hscrollbar_new (NULL) ;
  gtk_widget_show (dialog->hscroll) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), dialog->hscroll, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  vscroll = gtk_vscrollbar_new (NULL) ;
  gtk_widget_show (vscroll) ;
  gtk_table_attach (GTK_TABLE (tbl_sw), vscroll, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0) ;
  gtk_viewport_set_vadjustment (GTK_VIEWPORT (dialog->vp), gtk_range_get_adjustment (GTK_RANGE (vscroll))) ;

  tbl_status = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl_status) ;
  gtk_table_attach (GTK_TABLE (table), tbl_status, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl_status), 2) ;

  dialog->lbl_status = gtk_label_new ("") ;
  gtk_widget_show (dialog->lbl_status) ;
  gtk_table_attach (GTK_TABLE (tbl_status), dialog->lbl_status, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  statusbar = gtk_statusbar_new () ;
  gtk_widget_show (statusbar) ;
  gtk_table_attach (GTK_TABLE (tbl_status), statusbar, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  gtk_window_add_accel_group (GTK_WINDOW (dialog->dialog), accel_group) ;

  g_signal_connect (G_OBJECT (cr), "toggled", (GCallback)model_visible_toggled, dialog) ;
  g_signal_connect (G_OBJECT (dialog->tview), "row-expanded", (GCallback)set_bus_expanded, (gpointer)TRUE) ;
  g_signal_connect (G_OBJECT (dialog->tview), "row-collapsed", (GCallback)set_bus_expanded, (gpointer)FALSE) ;
  g_signal_connect (G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (dialog->hscroll))), "value-changed", (GCallback)hscroll_adj_value_changed, dialog->dialog) ;
  g_signal_connect (G_OBJECT (dialog->vp), "scroll-event", (GCallback)viewport_scroll, dialog) ;
  g_signal_connect (G_OBJECT (dialog->dialog), "delete_event", (GCallback)btnClose_clicked, NULL) ;
  }

static void build_io_tables (simulation_data *sim_data, BUS_LAYOUT *bus_layout)
  {
  int Nix, Nix1 ;
  EXP_ARRAY *cell_list = NULL ;
  BUS_LAYOUT_CELL blcell = {NULL, FALSE} ;
  BUS *bus = NULL ;

  if (NULL == bus_layout) return ;

  if (NULL == sim_data) return ;

  for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
    {
    blcell.cell = QCAD_CELL (qcad_cell_new_with_function ((sim_data->trace)[Nix].trace_function, (sim_data->trace)[Nix].data_labels)) ;
    cell_list = (QCAD_CELL_INPUT == blcell.cell->cell_function) ? bus_layout->inputs : bus_layout->outputs ;
    exp_array_insert_vals (cell_list, &blcell, 1, 1, -1) ;
    }

  for (Nix = 0 ; Nix < bus_layout->buses->icUsed ; Nix++)
    {
    bus = &(exp_array_index_1d (bus_layout->buses, BUS, Nix)) ;
    for (Nix1 = 0 ; Nix1 < bus->cell_indices->icUsed ; Nix1++)
      exp_array_index_1d (
        (QCAD_CELL_INPUT == bus->bus_function) ?
          bus_layout->inputs : bus_layout->outputs,
        BUS_LAYOUT_CELL,
        exp_array_index_1d (bus->cell_indices, int, Nix1)).bIsInBus = TRUE ;
    }
  }

static void set_ruler_values (GtkWidget *ruler, int cxGiven, int cx, int old_offset, int xOffset, int icSamples)
  {
  char *psz = NULL ;
  double lower, upper, position, max_size ;
  double dxInc = ((double)(icSamples)) / ((double)(cxGiven - 1)) ;
  double old_lower = -old_offset * dxInc ;
  GtkWidget *label = g_object_get_data (G_OBJECT (ruler), "label") ;

  gtk_ruler_get_range (GTK_RULER (ruler), &lower, &upper, &position, &max_size) ;
  lower = -xOffset * dxInc ;
  upper = (-xOffset + cx) * dxInc ;
  position += lower - old_lower ;
  max_size = icSamples ;
  gtk_ruler_set_range (GTK_RULER (ruler), lower, upper, position, max_size) ;

  gtk_label_set_text (GTK_LABEL (label),
    psz = g_strdup_printf ("%s %d", _("Sample"), (int)position)) ;
  g_free (psz) ;

  set_ruler_scale (GTK_RULER (ruler), lower, upper) ;
  }

static void draw_trace_reference_lines (GdkDrawable *dst, int cx, int cy)
  {
  GdkGC *gc = NULL ;
  GdkColor *clrGreen = NULL ;
  int cyOffset = cy * MIN_MAX_OFFSET ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, clrGreen = clr_idx_to_clr_struct (GREEN)) ;
  gdk_gc_set_background (gc, clrGreen) ;
  gdk_gc_set_line_attributes (gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);

  gdk_draw_line (dst, gc, 0, cyOffset, cx, cyOffset) ;
  gdk_draw_line (dst, gc, 0, cy >> 1, cx, cy >> 1) ;
  gdk_draw_line (dst, gc, 0, cy - cyOffset, cx, cy - cyOffset) ;

  g_object_unref (gc) ;
  }

static void set_trace_widget_visible (GtkWidget *trace, gboolean bVisible)
  {
  GRAPH_DATA *graph_data = NULL ;

  if (NULL != (graph_data = g_object_get_data (G_OBJECT (trace), "graph_data")))
    {
    graph_data->bVisible = bVisible ;
    if (GRAPH_DATA_TYPE_CELL == graph_data->data_type)
      ((WAVEFORM_DATA *)graph_data)->trace->drawtrace = bVisible ;
    }

  GTK_WIDGET_SET_VISIBLE (trace, bVisible) ;
  }

static void recalculate_honeycombs (GRAPH_DIALOG_DATA *gdd, gboolean bCalcHoneycombArrays, graph_D *dialog)
  {
  GtkTreeIter itr ;
  int row_type ;
  GtkWidget *trace = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  int cxOldWanted = 0 ;

  if (!gtk_tree_model_get_iter_first (gdd->model, &itr)) return ;
  while (TRUE)
    {
    gtk_tree_model_get (gdd->model, &itr,
      GRAPH_MODEL_COLUMN_TRACE, &trace,
      BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
    if (row_type & ROW_TYPE_BUS)
      if (NULL != (hc = g_object_get_data (G_OBJECT (trace), "graph_data")))
        {
        hc->graph_data.bNeedCalc = TRUE ;
        cxOldWanted = hc->graph_data.cxWanted ;

        if (bCalcHoneycombArrays)
          calculate_honeycomb_array (hc, gdd->sim_data->number_samples, gdd->dHCThreshLower, gdd->dHCThreshLower, gdd->base) ;
        else
          hc->graph_data.cxWanted = calculate_honeycomb_cxWanted (hc, gdd->sim_data->number_samples, gdd->base) ;

        if (hc->graph_data.cxWanted > hc->graph_data.cxGiven)
          {
          gdd->bOneTime = TRUE ;
          g_signal_connect (G_OBJECT (trace), "expose-event", (GCallback)graph_widget_one_time_expose, dialog) ;
          }
        gtk_widget_queue_draw (trace) ;
        }

    if (!gtk_tree_model_iter_next (gdd->model, &itr)) return ;
    }
  }
