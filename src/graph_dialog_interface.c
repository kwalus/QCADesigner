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
// Functions responsible for creating the graph dialog  //
// display elements, including the dialog itself, as    //
// well as the trace widgets.                           //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "support.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "qcadstock.h"
#include "bus_layout_dialog.h"
#include "objects/QCADRadioToolButton.h"
#include "graph_dialog_data.h"
#include "graph_dialog_interface.h"
#include "graph_dialog_widget_data.h"
#include "graph_dialog_callbacks.h"

static char *graph_dialog_ui_xml =
  "<ui>"
    "<menubar>"
      "<menu name=\"FileMenu\" action=\"FileMenuAction\">"
#ifdef STDIO_FILEIO
        "<menuitem name=\"FileOpen\" action=\"FileOpenAction\"/>"
        "<separator/>"
        "<menuitem name=\"FileSave\" action=\"FileSaveAction\"/>"
        "<separator/>"
        "<menuitem name=\"FilePreview\" action=\"FilePreviewAction\"/>"
#else
        "<separator/>"
#endif /* def STDIO_FILEIO */
        "<menuitem name=\"FilePrint\" action=\"FilePrintAction\"/>"
        "<separator/>"
        "<menuitem name=\"FileClose\" action=\"FileCloseAction\"/>"
      "</menu>"
      "<menu name=\"ViewMenu\" action=\"ViewMenuAction\">"
        "<menuitem name=\"ResetZoom\" action=\"ResetZoomAction\"/>"
        "<separator/>"
        "<menuitem name=\"ViewAsDecimal\" action=\"ViewAsDecimalAction\"/>"
        "<menuitem name=\"ViewAsBinary\" action=\"ViewAsBinaryAction\"/>"
        "<menuitem name=\"ViewAsHex\" action=\"ViewAsHexAction\"/>"
      "</menu>"
      "<menu name=\"ToolsMenu\" action=\"ToolsMenuAction\">"
        "<menuitem name=\"WaveInterp\" action=\"WaveInterpAction\"/>"
      "</menu>"
    "</menubar>"
    "<toolbar name=\"MainToolbar\" action=\"MainToolbarAction\">"
      "<placeholder name=\"ToolItems\">"
        "<separator/>"
        "<toolitem name=\"FileClose\" action=\"FileCloseAction\"/>"
#ifdef STDIO_FILEIO
        "<separator/>"
        "<toolitem name=\"FileOpen\" action=\"FileOpenAction\"/>"
        "<toolitem name=\"FileSave\" action=\"FileSaveAction\"/>"
        "<separator/>"
        "<toolitem name=\"FilePreview\" action=\"FilePreviewAction\"/>"
#else
        "<separator/>"
#endif /* def STDIO_FILEIO */
        "<toolitem name=\"FilePrint\" action=\"FilePrintAction\"/>"
        "<separator/>"
        "<toolitem name=\"ResetZoom\" action=\"ResetZoomAction\"/>"
        "<separator/>"
        "<toolitem name=\"WaveInterp\" action=\"WaveInterpAction\"/>"
        "<separator/>"
        "<toolitem name=\"ViewAsDecimal\" action=\"ViewAsDecimalAction\"/>"
        "<toolitem name=\"ViewAsBinary\" action=\"ViewAsBinaryAction\"/>"
        "<toolitem name=\"ViewAsHex\" action=\"ViewAsHexAction\"/>"
        "<separator/>"
      "</placeholder>"
    "</toolbar>"
  "</ui>" ;

static GtkActionEntry action_entries[] =
  {
  {"FileMenuAction",    NULL,                    N_("_File")},
#ifdef STDIO_FILEIO
  {"FileOpenAction",    GTK_STOCK_OPEN,          NULL,                    NULL, 
    NULL,                                 (GCallback)gd_actOpen_activate},
  {"FileSaveAction",    GTK_STOCK_SAVE,          NULL,                    NULL, 
    NULL,                                 (GCallback)gd_actSave_activate},
  {"FilePreviewAction", GTK_STOCK_PRINT_PREVIEW, NULL,                    NULL, 
    NULL,                                 (GCallback)gd_actPreview_activate},
#endif /* def STDIO_FILEIO */
  {"FilePrintAction",   GTK_STOCK_PRINT,         NULL,                    NULL, 
    NULL,                                 (GCallback)gd_actPrint_activate},
  {"FileCloseAction",   GTK_STOCK_CLOSE,         NULL,                    NULL, 
    NULL,                                 (GCallback)gd_actClose_activate},
  {"ViewMenuAction",    NULL,                    N_("_View")},
  {"ResetZoomAction",   GTK_STOCK_ZOOM_100,      N_("Reset Zoom"),        NULL, 
    N_("Un-stretch Traces"),              (GCallback)gd_actZoom100_activate},
  {"ToolsMenuAction",   NULL,                    N_("_Tools")},
  {"WaveInterpAction",  GTK_STOCK_PREFERENCES,   N_("Interpretation..."), NULL, 
    N_("Digital Interpretation Options"), (GCallback)gd_actThresh_activate}
  } ;
static int n_action_entries = G_N_ELEMENTS (action_entries) ;

static GtkRadioActionEntry view_as_action_entries[] =
  {
  {"ViewAsDecimalAction", QCAD_STOCK_GRAPH_DEC, N_("Decimal"), NULL, N_("Show Values As Decimal"),     10},
  {"ViewAsBinaryAction",  QCAD_STOCK_GRAPH_BIN, N_("Binary"),  NULL, N_("Show Values As Binary"),       2},
  {"ViewAsHexAction",     QCAD_STOCK_GRAPH_HEX, N_("Hex"),     NULL, N_("Show Values As Hexadecimal"), 16},
  } ;
static int n_view_as_action_entries = G_N_ELEMENTS (view_as_action_entries) ;

static gboolean create_waveform_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr) ;
static gboolean create_bus_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr, int base) ;
static GtkWidget *create_trace_drawing_area (GRAPH_DATA *graph_data, GDestroyNotify graph_data_free, GCallback graph_widget_expose, gpointer data) ;
static void force_adj_to_lower (GtkAdjustment *adj, gpointer data) ;

void create_graph_dialog (graph_D *dialog)
  {
  GtkWidget *table = NULL, *tbl_vp = NULL, *sw_tview = NULL, *status_table = NULL, *frm = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkAdjustment *fake_hadj = NULL ;

  GError *error = NULL ;
  GtkUIManager *ui_mgr = NULL ;
  GtkActionGroup *actions = NULL ;

  // The Window
  dialog->dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 800, 600);
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Simulation Results"));
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE);

  table = gtk_table_new (5, 1, FALSE) ;
  gtk_widget_show (table) ;
  gtk_container_add (GTK_CONTAINER (dialog->dialog), table) ;

  ui_mgr = gtk_ui_manager_new () ;
  actions = gtk_action_group_new ("QCADGraphDialogActions") ;
  gtk_action_group_set_translation_domain (actions, PACKAGE) ;
  gtk_action_group_add_actions (actions, action_entries, n_action_entries, dialog) ;
  gtk_action_group_add_radio_actions (actions, view_as_action_entries, n_view_as_action_entries, 10, (GCallback)gd_actShowBase_activate, dialog) ;
  gtk_ui_manager_insert_action_group (ui_mgr, actions, -1) ;
  gtk_ui_manager_add_ui_from_string (ui_mgr, graph_dialog_ui_xml, -1, &error) ;
  gtk_window_add_accel_group (GTK_WINDOW (dialog->dialog), gtk_ui_manager_get_accel_group (ui_mgr)) ;
  if (error != NULL)
    {
    g_message ("Failed to create UI: %s\n", error->message) ;
    g_error_free (error) ;
    }
  else
    {
    GtkWidget *widget = NULL ;

    widget = gtk_ui_manager_get_widget (ui_mgr, "/ui/menubar") ;
    gtk_widget_show (widget) ;
    gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 0, 1, 
      (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions)(GTK_FILL), 2, 0) ;

    widget = gtk_ui_manager_get_widget (ui_mgr, "/ui/MainToolbar") ;
    gtk_widget_show (widget) ;
    gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 1, 2, 
      (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions)(GTK_FILL), 2, 0) ;

    g_object_set_data (G_OBJECT (gtk_ui_manager_get_action (ui_mgr, "/ui/menubar/FileMenu/FileClose")), "dlgGraphs", dialog->dialog) ;
    }

  dialog->hpaned = g_object_new (GTK_TYPE_HPANED, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (table), dialog->hpaned, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  sw_tview = g_object_new (GTK_TYPE_SCROLLED_WINDOW, "visible", TRUE, "shadow-type", GTK_SHADOW_IN,
    "hscrollbar-policy", GTK_POLICY_AUTOMATIC, "vscrollbar-policy", GTK_POLICY_AUTOMATIC, NULL) ;
  gtk_paned_add1 (GTK_PANED (dialog->hpaned), sw_tview) ;

  dialog->tview = create_bus_layout_tree_view (TRUE, _("Trace"), GTK_SELECTION_SINGLE) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tview), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (col, _("Visible")) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  gtk_tree_view_column_add_attribute (col, cr, "active", GRAPH_MODEL_COLUMN_VISIBLE) ;
  g_object_set (G_OBJECT (cr), "activatable", TRUE, NULL) ;
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cr), TRUE) ;
  gtk_widget_show (dialog->tview) ;
  gtk_container_add (GTK_CONTAINER (sw_tview), dialog->tview) ;

  dialog->sw = g_object_new (GTK_TYPE_SCROLLED_WINDOW, "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
    "vscrollbar-policy", GTK_POLICY_AUTOMATIC, "shadow-type", GTK_SHADOW_NONE, "visible", TRUE, NULL) ;
  gtk_paned_add2 (GTK_PANED (dialog->hpaned), dialog->sw) ;

  tbl_vp = g_object_new (GTK_TYPE_TABLE, "visible", TRUE, "n-rows", 1, "n-columns", 1, "homogeneous", FALSE, NULL) ;
  dialog->vp = g_object_new (GTK_TYPE_VIEWPORT, "visible", TRUE, NULL) ;
  gtk_container_add (GTK_CONTAINER (dialog->vp), tbl_vp) ;
  gtk_container_add (GTK_CONTAINER (dialog->sw), dialog->vp) ;
  fake_hadj = g_object_new (GTK_TYPE_ADJUSTMENT, NULL) ;
  g_object_set (G_OBJECT (dialog->vp), "hadjustment", fake_hadj, NULL) ;

  dialog->table_of_traces = g_object_new (GTK_TYPE_TABLE, "visible", TRUE, "n-rows", 1, "n-columns", 2, "homogeneous", FALSE, NULL) ;
  gtk_table_attach (GTK_TABLE (tbl_vp), dialog->table_of_traces, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(0), 0, 0) ;

  status_table = g_object_new (GTK_TYPE_TABLE, "n-columns", 2, "n-rows", 1, "homogeneous", FALSE, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (table), status_table, 0, 1, 3, 4,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL), 0, 2) ;

  frm = g_object_new (GTK_TYPE_FRAME, "shadow-type", GTK_SHADOW_IN, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (status_table), frm, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL), 1, 0) ;

  dialog->lbl_status = g_object_new (GTK_TYPE_LABEL, "label", "", "visible", TRUE, "justify", GTK_JUSTIFY_LEFT, 
    "xalign", 0.0, "yalign", 0.5, "xpad", 2, "ypad", 2, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), dialog->lbl_status) ;

//  dialog->size_group_vert = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL) ;

  g_signal_connect (G_OBJECT (cr),             "toggled",       (GCallback)gd_model_visible_toggled,     dialog) ;
  g_signal_connect (G_OBJECT (dialog->tview),  "row-expanded",  (GCallback)gd_set_bus_expanded,          (gpointer)TRUE) ;
  g_signal_connect (G_OBJECT (dialog->tview),  "row-collapsed", (GCallback)gd_set_bus_expanded,          (gpointer)FALSE) ;
  g_signal_connect (G_OBJECT (gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (dialog->sw))),
                                               "value-changed", (GCallback)gd_hscroll_adj_value_changed, dialog->dialog) ;
  g_signal_connect (G_OBJECT (dialog->vp),     "scroll-event",  (GCallback)gd_viewport_scroll,           dialog) ;
  g_signal_connect (G_OBJECT (dialog->dialog), "delete-event",  (GCallback)gd_actClose_activate,         NULL) ;
  g_signal_connect (G_OBJECT (fake_hadj),      "value-changed", (GCallback)force_adj_to_lower,           NULL) ;
  }

void attach_graph_widgets (graph_D *dialog, GtkWidget *table, GtkWidget *trace, GtkWidget *ruler, GtkWidget *ui, int idxTbl)
  {
  gtk_widget_show (trace) ;
  gtk_widget_set_redraw_on_allocate (trace, FALSE) ;

  // Attach the trace
  gtk_table_attach (GTK_TABLE (table), trace, 
    TRACE_TABLE_MIN_X + 2,                   TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1), TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  // Attach the ruler
  gtk_widget_show (ruler) ;
  gtk_table_attach (GTK_TABLE (table), ruler,
    TRACE_TABLE_MIN_X + 2,             TRACE_TABLE_MIN_X + 3,
    TRACE_TABLE_MIN_Y + (idxTbl << 1), TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1),
    (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  // Attach the UI
  gtk_widget_show (ui) ;
  gtk_table_attach (GTK_TABLE (table), ui,
    TRACE_TABLE_MIN_X + 1,                   TRACE_TABLE_MIN_X + 2,
    TRACE_TABLE_MIN_Y + ((idxTbl << 1) + 1), TRACE_TABLE_MIN_Y + ((idxTbl + 1) << 1),
    (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  set_window_icon (GTK_WINDOW (dialog->dialog), "graph_dialog") ;
  }

gboolean create_graph_widgets (GRAPH_DIALOG_DATA *graph_dialog_data, GtkTreeIter *itr)
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
//  g_object_weak_ref (G_OBJECT (trace_ui_widget), (GWeakNotify)g_print, "Waveform trace UI widget g0ne!\n") ;

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

  trace_drawing_widget = create_trace_drawing_area ((GRAPH_DATA *)wf, (GDestroyNotify)waveform_data_free, (GCallback)gd_waveform_expose, graph_dialog_data) ;
//  g_object_weak_ref (G_OBJECT (trace_ui_widget), (GWeakNotify)g_print, "Waveform trace drawing widget g0ne!\n") ;

  gtk_size_group_add_widget (graph_dialog_data->size_group_vert, trace_drawing_widget) ;
  gtk_size_group_add_widget (graph_dialog_data->size_group_vert, trace_ui_widget) ;

  trace_ruler_widget = gtk_hruler_new () ;

  g_object_set_data (G_OBJECT (trace_drawing_widget), "ruler", trace_ruler_widget) ;

  g_signal_connect (G_OBJECT (trace_ruler_widget),   "motion-notify-event", (GCallback)gd_graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "size-allocate",       (GCallback)gd_graph_widget_size_allocate, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_ui_widget),      "size-allocate",       (GCallback)gd_graph_widget_size_allocate, graph_dialog_data) ;

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

  gtk_tree_model_get (graph_dialog_data->model, itr, 
    BUS_LAYOUT_MODEL_COLUMN_TYPE,  &bus_type, 
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx, -1) ;

  hc = honeycomb_data_new (clr_idx_to_clr_struct ((bus_type & ROW_TYPE_OUTPUT) ? YELLOW : BLUE),
    &exp_array_index_1d (graph_dialog_data->bus_layout->buses, BUS, idx)) ;

  if (bus_type & ROW_TYPE_OUTPUT) offset = graph_dialog_data->bus_layout->inputs->icUsed ;

  while (TRUE)
    {
    gtk_tree_model_get (graph_dialog_data->model, &itrChildren, BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx, -1) ;
    trace = &((graph_dialog_data->sim_data->trace)[idx + offset]) ;
    exp_array_1d_insert_vals (hc->arTraces, &trace, 1, -1) ;
    if (!gtk_tree_model_iter_next (graph_dialog_data->model, &itrChildren)) break ;
    }

  calculate_honeycomb_array (hc, graph_dialog_data->sim_data->number_samples, graph_dialog_data->dHCThreshLower, graph_dialog_data->dHCThreshUpper, graph_dialog_data->icAverageSamples, base) ;

  gtk_tree_model_get (graph_dialog_data->model, itr, BUS_LAYOUT_MODEL_COLUMN_NAME, &pszBusName, -1) ;

  trace_ui_widget = gtk_frame_new (NULL) ;
  tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (trace_ui_widget), tbl) ;
//  g_object_weak_ref (G_OBJECT (trace_ui_widget), (GWeakNotify)g_print, "Bus trace UI widget g0ne!\n") ;

  lbl = gtk_label_new (pszBusName) ;
  g_free (pszBusName) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  trace_drawing_widget = create_trace_drawing_area ((GRAPH_DATA *)hc, (GDestroyNotify)honeycomb_data_free, (GCallback)gd_honeycomb_expose, graph_dialog_data) ;
//  g_object_weak_ref (G_OBJECT (trace_ui_widget), (GWeakNotify)g_print, "Bus trace drawing widget g0ne!\n") ;

  trace_ruler_widget = gtk_hruler_new () ;

  g_object_set_data (G_OBJECT (trace_drawing_widget), "ruler", trace_ruler_widget) ;

  gtk_size_group_add_widget (graph_dialog_data->size_group_vert, trace_drawing_widget) ;
  gtk_size_group_add_widget (graph_dialog_data->size_group_vert, trace_ui_widget) ;

  g_signal_connect (G_OBJECT (trace_ruler_widget),   "motion-notify-event", (GCallback)gd_graph_widget_motion_notify, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "size-allocate",       (GCallback)gd_graph_widget_size_allocate, graph_dialog_data) ;
  g_signal_connect (G_OBJECT (trace_ui_widget),      "size-allocate",       (GCallback)gd_graph_widget_size_allocate, graph_dialog_data) ;

  gtk_tree_store_set (GTK_TREE_STORE (graph_dialog_data->model), itr,
    GRAPH_MODEL_COLUMN_VISIBLE, TRUE,
    GRAPH_MODEL_COLUMN_RULER,   trace_ruler_widget,
    GRAPH_MODEL_COLUMN_TRACE,   trace_drawing_widget,
    GRAPH_MODEL_COLUMN_UI,      trace_ui_widget, -1) ;

  if (gtk_tree_model_iter_children (graph_dialog_data->model, &itrChildren, itr))
    while (create_waveform_widgets (graph_dialog_data, &itrChildren)) ;

  return gtk_tree_model_iter_next (graph_dialog_data->model, itr) ;
  }

static GtkWidget *create_trace_drawing_area (GRAPH_DATA *graph_data, GDestroyNotify graph_data_free, GCallback graph_widget_expose, gpointer data)
  {
  GtkWidget *trace_drawing_widget = gtk_drawing_area_new () ;
  gtk_widget_add_events (trace_drawing_widget, GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK) ;
  set_widget_background_colour (trace_drawing_widget, 0, 0, 0) ;
  g_object_set_data_full (G_OBJECT (trace_drawing_widget), "graph_data", graph_data, (GDestroyNotify)graph_data_free) ;
  gtk_widget_set_size_request (trace_drawing_widget, -1, TRACE_MIN_CY) ;
  gtk_widget_add_events (trace_drawing_widget,
    GDK_EXPOSURE_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "expose-event",         (GCallback)graph_widget_expose,            data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-press-event",   (GCallback)gd_graph_widget_button_press,   data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "motion-notify-event",  (GCallback)gd_graph_widget_motion_notify,  data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "button-release-event", (GCallback)gd_graph_widget_button_release, data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "enter-notify-event",   (GCallback)gd_graph_widget_enter_notify,   data) ;
  g_signal_connect (G_OBJECT (trace_drawing_widget), "leave-notify-event",   (GCallback)gd_graph_widget_leave_notify,   data) ;

  return trace_drawing_widget ;
  }

static void force_adj_to_lower (GtkAdjustment *adj, gpointer data)
  {if (adj->value != adj->lower) gtk_adjustment_set_value (adj, adj->lower) ;}
