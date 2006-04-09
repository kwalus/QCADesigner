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
// A (fairly) complete print settings dialog with mar-  //
// gins, Center Page, paper size, user-selectable units //
// (cm/in/pt), etc.                                     //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include "design.h"
#include "support.h"
#include "print_preview.h"
#include "file_selection_window.h"
#include "objects/QCADPrintDialog.h"
#include "print_properties_dialog.h"
#include "print_properties_dialog_interface.h"
#include "print_properties_dialog_callbacks.h"

#define MIN_MARGIN_GAP 72 /* points */
#define CEIL_EXCEPTION_EPSILON 1e-10

#define STATUS_OK 0
#define STATUS_NEED_FILE_NAME 1
#define STATUS_NEED_PIPE 2

static print_properties_D print_properties = {NULL} ;

static void fill_printed_objects_list (print_properties_D *dialog, DESIGN *design) ;
static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, DESIGN *design) ;
static void calc_world_size (int *piCX, int *piCY, print_properties_D *dialog) ;
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj) ;

extern double subs_width ;
extern double subs_height ;

// The main function
gboolean get_print_design_properties_from_user (GtkWindow *parent, print_design_OP *ppo, DESIGN *design)
  {
  gboolean bOK = FALSE ;

  if (NULL == print_properties.dlgPrintProps)
    create_print_design_properties_dialog (&print_properties, ppo) ;

  init_print_design_properties_dialog (&print_properties, parent, ppo, design) ;

  if ((bOK = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (print_properties.dlgPrintProps)))))
      init_print_design_options (ppo, design) ;

  gtk_widget_hide (print_properties.dlgPrintProps) ;

  if (NULL != parent)
    gtk_window_present (parent) ;

  return bOK ;
  }

// initialize the dialog - whether to display it, or to simply ensure correct print_op values
static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, DESIGN *design)
  {
  if (NULL == dialog->dlgPrintProps)
    create_print_design_properties_dialog (dialog, print_op) ;

  // The static data needs to be set right away, because signals will come up empty
  g_object_set_data (G_OBJECT (dialog->dlgPrintProps), "design", design) ;
  g_object_set_data (G_OBJECT (dialog->dlgPrintProps), "dialog", dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (dialog->dlgPrintProps), parent) ;

  fill_printed_objects_list (dialog, design) ;

  // Fill in the dialog from the print_op values (must have the ppPrintedObjs filled in first !)
  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits),
    qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), print_op->dPointsPerNano)) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnPrintOrder), !print_op->bPrintOrderOver) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnCenter), print_op->bCenter) ;

  if (print_op->bFit)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages), TRUE) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCXPages), print_op->iCXPages) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCYPages), print_op->iCYPages) ;
    }

  toggle_scale_mode (NULL, dialog->dlgPrintProps) ;
  }

void chkPrintedObj_toggled (GtkCellRenderer *cr, char *pszPath, gpointer user_data)
  {
  gboolean *pbPrintLayer = NULL ;
  gboolean bPrintLayer = TRUE ;
  GtkTreeIter itr ;
  QCADLayer *layer = NULL ;
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tvPrintedObjs)) ;
  int cx = -1, cy = -1 ;

  if (NULL == tm) return ;
  if (!gtk_tree_model_get_iter_from_string (tm, &itr, pszPath)) return ;

  gtk_tree_model_get (tm, &itr, 
    PRINTED_LAYERS_MODEL_COLUMN_PRINTED, &bPrintLayer, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;

  pbPrintLayer = g_object_get_data (G_OBJECT (layer), PRINT_LAYER_KEY) ;

  bPrintLayer = (!bPrintLayer) ;

  if (NULL != pbPrintLayer) (*pbPrintLayer) = bPrintLayer ;
  gtk_list_store_set (GTK_LIST_STORE (tm), &itr, PRINTED_LAYERS_MODEL_COLUMN_PRINTED, bPrintLayer, -1) ;

  if (!bPrintLayer)
    {
    calc_world_size (&cx, &cy, dialog) ;
    if (0 == cx || 0 == cy)
      {
      if (NULL != pbPrintLayer) (*pbPrintLayer) = !bPrintLayer ;
      gtk_list_store_set (GTK_LIST_STORE (tm), &itr, PRINTED_LAYERS_MODEL_COLUMN_PRINTED, !bPrintLayer, -1) ;
      gdk_beep () ;
      return ;
      }
    }

  check_scale (dialog, NULL) ;
  }

void units_changed (GtkWidget *widget, double conversion_factor, gpointer data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (widget), "dialog") ;
  char *pszShortString = qcad_print_dialog_get_units_short_string (QCAD_PRINT_DIALOG (widget)) ;

  gtk_label_set_text (GTK_LABEL (dialog->lblScale), pszShortString) ;

  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) * conversion_factor) ;

  g_free (pszShortString) ;
  }

// Used when the print properties dialog is to be initialized but not displayed - like
// when the user clicks "Preview" before ever having used the dialog.  Also used for
// filling out the print_OP structure
void init_print_design_options (print_design_OP *pPrintOp, DESIGN *design)
  {
  if (NULL == print_properties.dlgPrintProps)
    {
    create_print_design_properties_dialog (&print_properties, pPrintOp) ;
    init_print_design_properties_dialog (&print_properties, NULL, pPrintOp, design) ;
    }

  qcad_print_dialog_get_options (QCAD_PRINT_DIALOG (print_properties.dlgPrintProps), &(pPrintOp->po)) ;

  // points per nanometer
  pPrintOp->dPointsPerNano = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (print_properties.dlgPrintProps),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (print_properties.adjNanoToUnits))) ;

  // Print over than down ?
  pPrintOp->bPrintOrderOver = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnPrintOrder)) ;

  // Center on pages ?
  pPrintOp->bCenter = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnCenter)) ;

  // Number of horizontal pages - takes precedence over the scaling factor
  pPrintOp->iCXPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCXPages)) ;

  // Number of vertical pages - takes precedence over the scaling factor
  pPrintOp->iCYPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCYPages)) ;

  pPrintOp->bFit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.rbFitPages)) ;
  }

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblPrintOrder),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    _("Down, then over") : _("Over, then down")) ;
  }

void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblCenter), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? _("Center") : _("Do Not Center")) ;
  }

// Calculate the world size (in nanos)
static void calc_world_size (int *piCX, int *piCY, print_properties_D *dialog)
  {
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tvPrintedObjs)) ;
  GtkTreeIter itr ;
  WorldRectangle layer_extents = {0.0} ;
  QCADLayer *layer = NULL ;
  gboolean bPrintLayer = TRUE ;

  (*piCX) = (*piCY) = 0 ;

  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      gtk_tree_model_get (tm, &itr, LAYER_MODEL_COLUMN_LAYER, &layer, PRINTED_LAYERS_MODEL_COLUMN_PRINTED, &bPrintLayer, -1) ;
      if (bPrintLayer)
        {
        qcad_layer_get_extents (layer, &layer_extents, FALSE) ;
        (*piCX) = MAX ((*piCX), layer_extents.cxWorld) ;
        (*piCY) = MAX ((*piCY), layer_extents.cyWorld) ;
        }
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;
  }

static void fill_printed_objects_list (print_properties_D *dialog, DESIGN *design)
  {
  gboolean *pbPrintLayer = NULL ;
  QCADLayer *layer = NULL ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;

  if (NULL == dialog || NULL == design) return ;

  tm = GTK_TREE_MODEL (design_layer_list_store_new (design, 1, G_TYPE_BOOLEAN)) ;
  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      gtk_tree_model_get (tm, &itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;
      if (NULL == (pbPrintLayer = g_object_get_data (G_OBJECT (layer), PRINT_LAYER_KEY)))
        {
        g_object_set_data_full (G_OBJECT (layer), PRINT_LAYER_KEY, pbPrintLayer = g_malloc0 (sizeof (gboolean)), (GDestroyNotify)g_free) ;
        (*pbPrintLayer) = TRUE ;
        }
      gtk_list_store_set (GTK_LIST_STORE (tm), &itr, PRINTED_LAYERS_MODEL_COLUMN_PRINTED, (*pbPrintLayer), -1) ;
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tvPrintedObjs), tm) ;
  }

void toggle_scale_mode (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  gboolean bAuto = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)) ;

  if (NULL != widget)
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  gtk_widget_set_sensitive (dialog->fmScale, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblNanoIs, !bAuto) ;
  gtk_widget_set_sensitive (dialog->spnNanoToUnits, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblScale, !bAuto) ;

  gtk_widget_set_sensitive (dialog->fmFit, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCXPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCYPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsWide, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsTall, bAuto) ;

  check_scale (dialog, NULL) ;
  }

// Make sure the scale and the number of pages tall/wide agree
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj)
  {
  double dcxPg, dcyPg, tmp,
    dNanoToUnits = gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) ;
  int
    icxWorld = 0, icyWorld = 0, /* in nanos */
    icxPages = 0, icyPages = 0 ;
  print_OP po = {0} ;

  qcad_print_dialog_get_options (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), &po) ;
  if (NULL != po.pszPrintString) g_free (po.pszPrintString) ;

  calc_world_size (&icxWorld, &icyWorld, dialog) ;

  dcxPg = po.dPaperCX - po.dLMargin - po.dRMargin ;
  dcyPg = po.dPaperCY - po.dTMargin - po.dBMargin ;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)))
    {
    if (adj == dialog->adjCXPages)
      dNanoToUnits =
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld ;
    else
    if (adj == dialog->adjCYPages)
      dNanoToUnits =
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld ;
    else
      dNanoToUnits = MIN (
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld,
        qcad_print_dialog_to_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld) ;

    dNanoToUnits = floor (dNanoToUnits * 1000.0) / 1000.0 ;

    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits), dNanoToUnits) ;
    }

  tmp = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), icxWorld * dNanoToUnits) / dcxPg ;
  icxPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;
  tmp = qcad_print_dialog_from_current_units (QCAD_PRINT_DIALOG (dialog->dlgPrintProps), icyWorld * dNanoToUnits) / dcyPg ;
  icyPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;

  g_signal_handlers_block_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  g_signal_handlers_block_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)) != icxPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCXPages), icxPages) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)) != icyPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCYPages), icyPages) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)validate_value_change, NULL) ;
  }

// Make sure all spin buttons everywhere always have correct values
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  check_scale (dialog, adj_changed) ;
  }

void user_wants_print_preview (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  DESIGN *design = (DESIGN *)g_object_get_data (G_OBJECT (user_data), "design") ;
  print_design_OP po ;

  init_print_design_options (&po, design) ;

  do_print_preview ((print_OP *)&po, GTK_WINDOW (dialog->dlgPrintProps), (void *)design, (PrintFunction)print_world) ;
  }
