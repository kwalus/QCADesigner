//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@vlsi.enel.ucalgary.ca). It implements a    //
// (fairly) complete print settings dialog with mar-    //
// gins, Center Page, paper size, user-selectable units //
// (cm/in/pt), etc.                                     //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <math.h>

#include "support.h"
#include "stdqcell.h"
#include "cad.h"
#include "print_preview.h"

#include "file_selection_window.h"
#include "print_properties_dialog.h"
#include "print_properties_dialog_interface.h"
#include "print_properties_dialog_callbacks.h"

#define MIN_MARGIN_GAP 72 /* points */
#define CEIL_EXCEPTION_EPSILON 1e-10

#define STATUS_OK 0
#define STATUS_NEED_FILE_NAME 1
#define STATUS_NEED_PIPE 2

static print_properties_D print_properties = {NULL} ;
static double
  world_extents_x1 = 0,
  world_extents_y1 = 0,
  world_extents_x2 = 0,
  world_extents_y2 = 0 ;

static double conversion_matrix[3][3] = 
  {
  {    1.00     , 1.00 /  2.54 , 72.00 / 2.54},
  {    2.54     ,     1.00     ,     72.00   },
  {2.54 / 72.00 , 1.00 / 72.00 ,      1.00   }
  } ;

static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, GQCell *first_cell) ;
static void calc_world_size (GQCell *first_cell, int *piWidth, int *piHeight, print_properties_D *dialog) ;
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj, GQCell *first_cell) ;

extern double subs_width ;
extern double subs_height ;

/* The main function */
gboolean get_print_design_properties_from_user (GtkWindow *parent, print_design_OP *ppo, GQCell *first_cell)
  {
  gboolean bOK = FALSE ;
  
  if (NULL == print_properties.dlgPrintProps)
    create_print_design_properties_dialog (&print_properties, ppo) ;

  init_print_design_properties_dialog (&print_properties, parent, ppo, first_cell) ;
  
  if ((bOK = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (print_properties.dlgPrintProps)))))
      init_print_design_options (ppo, first_cell) ;  
  
  gtk_widget_hide (print_properties.dlgPrintProps) ;
  
  return bOK ;
  }

/* initialize the dialog - whether to display it, or to simply ensure correct print_op values */
static void init_print_design_properties_dialog (print_properties_D *dialog, GtkWindow *parent, print_design_OP *print_op, GQCell *first_cell)
  {
  int Nix ;
  
  if (NULL == dialog->dlgPrintProps)
    create_print_design_properties_dialog (dialog, print_op) ;
    
  /* The static data needs to be set right away, because signals will come up empty */
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "first_cell", first_cell) ;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "dialog", dialog) ;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "old_units",
    (gpointer)print_dialog_get_units (PRINT_DIALOG (dialog->dlgPrintProps))) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (dialog->dlgPrintProps), parent) ;

  get_extents (first_cell, &world_extents_x1, &world_extents_y1, &world_extents_x2, &world_extents_y2) ;
  
  if (NULL != dialog->ppPrintedObjs)
    for (Nix = 0 ; Nix < print_op->icPrintedObjs ; Nix++)
      gtk_container_remove (GTK_CONTAINER (dialog->vbPrintedObjs), dialog->ppPrintedObjs[Nix]) ;

  fill_printed_objects_list (dialog->lstPrintedObjs, dialog) ;
  
  for (Nix = 0 ; Nix < print_op->icPrintedObjs ; Nix++)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix]),
      print_op->pbPrintedObjs[Nix]) ;
    gtk_signal_connect (GTK_OBJECT (dialog->ppPrintedObjs[Nix]), "toggled",
      (GtkSignalFunc)chkPrintedObj_toggled, dialog->dlgPrintProps) ;
    }
    
  /* If there are no cells in the design, disallow checking the "The Cells" checkbox by
     programmatically unchecking it and then disabling it */
  if (NULL == first_cell)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS]), FALSE) ;
    gtk_widget_set_sensitive (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS], FALSE) ;
    }
  
  /* Fill in the dialog from the print_op values (must have the ppPrintedObjs filled in first !) */
  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits), 
    print_dialog_to_current_units (PRINT_DIALOG (dialog->dlgPrintProps), print_op->dPointsPerNano)) ;
  
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

void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  int Nix ;
  
  /* Ensure that there's always /something/ to print */
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE])))
    {
    for (Nix = 0 ; Nix < dialog->icPrintedObjs ; Nix++)
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix])) &&
          Nix != PRINTED_OBJECTS_COLOURS)
	break ;

    if (Nix >= dialog->icPrintedObjs)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE]), TRUE) ;
    }
  
  check_scale (dialog, NULL, GQCELL (gtk_object_get_data (GTK_OBJECT (user_data), "first_cell"))) ;
  }

void units_changed (GtkWidget *widget, gpointer data)
  {
  print_properties_D *dialog = (print_properties_D *)g_object_get_data (G_OBJECT (widget), "dialog") ;
  PrintDialogUnits 
    old_units = (PrintDialogUnits)g_object_get_data (G_OBJECT (widget), "old_units"),
    new_units = print_dialog_get_units (PRINT_DIALOG (widget)) ;
  char *pszShortString = print_dialog_get_units_short_string (PRINT_DIALOG (widget)) ;
  
  gtk_label_set_text (GTK_LABEL (dialog->lblScale), pszShortString) ;
  
  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) * 
      conversion_matrix[old_units][new_units]) ;
  
  g_object_set_data (G_OBJECT (widget), "old_units", (gpointer)new_units) ;
  }

/* Used when the print properties dialog is to be initialized but not displayed - like
   when the user clicks "Preview" before ever having used the dialog.  Also used for
   filling out the print_OP structure */
void init_print_design_options (print_design_OP *pPrintOp, GQCell *first_cell)
  {
  int Nix ;

  if (NULL == print_properties.dlgPrintProps)
    {
    create_print_design_properties_dialog (&print_properties, pPrintOp) ;
    init_print_design_properties_dialog (&print_properties, NULL, pPrintOp, first_cell) ;
    }
  
  print_dialog_get_options (PRINT_DIALOG (print_properties.dlgPrintProps), &(pPrintOp->po)) ;
  
  /* points per nanometer */
  pPrintOp->dPointsPerNano = print_dialog_from_current_units (PRINT_DIALOG (print_properties.dlgPrintProps),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (print_properties.adjNanoToUnits))) ;
  
  /* The various layers */
  for (Nix = 0 ; Nix < print_properties.icPrintedObjs ; Nix++)
    pPrintOp->pbPrintedObjs[Nix] =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.ppPrintedObjs[Nix])) ;
  
  /* Print over than down ? */
  pPrintOp->bPrintOrderOver = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnPrintOrder)) ;
  
  /* Center on pages ? */
  pPrintOp->bCenter = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnCenter)) ;
  
  /* Number of horizontal pages - takes precedence over the scaling factor */
  pPrintOp->iCXPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCXPages)) ;
  
  /* Number of vertical pages - takes precedence over the scaling factor */
  pPrintOp->iCYPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCYPages)) ;
  
  pPrintOp->bFit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.rbFitPages)) ;
  }

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblPrintOrder),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    "Down, then over" : "Over, then down") ;
  }

void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblCenter),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    "Center" : "Do Not Center") ;
  }

static void calc_world_size (GQCell *first_cell, int *piWidth, int *piHeight, print_properties_D *dialog)
  {
  int xMin = 0, yMin = 0, xMax = 0, yMax = 0 ;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS])))
    {
    if (NULL != first_cell)
      {
      xMin = world_extents_x1 ;
      yMin = world_extents_y1 ;
      xMax = world_extents_x2 ;
      yMax = world_extents_y2 ;
      }
    }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE])))
    {
    if (xMin > 0) xMin = 0 ;
    if (yMin > 0) yMin = 0 ;
    if (xMax < subs_width) xMax = subs_width ;
    if (yMax < subs_height) yMax = subs_height ;
    }
  *piWidth = xMax - xMin ;
  *piHeight = yMax - yMin ;
  }

/* So far, there are only 3 hardcoded layers */
void fill_printed_objects_list (GtkWidget *ls, print_properties_D *dialog)
  {
  if (NULL == dialog->ppPrintedObjs)
    {
    dialog->icPrintedObjs = 3 ;
    dialog->ppPrintedObjs = malloc (3 * sizeof (GtkWidget *)) ;
    }

  dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE] = gtk_check_button_new_with_label ("The Die") ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE], FALSE, FALSE, 0) ;
  
  dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS] = gtk_check_button_new_with_label ("The Cells") ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS], FALSE, FALSE, 0) ;
  
  dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS] = gtk_check_button_new_with_label ("Colours") ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS], FALSE, FALSE, 0) ;
  }

void toggle_scale_mode (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
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
  
  check_scale (dialog, NULL, GQCELL (gtk_object_get_data (GTK_OBJECT (user_data), "first_cell"))) ;
  }

/* Make sure the scale and the number of pages tall/wide agree */
static void check_scale (print_properties_D *dialog, GtkAdjustment *adj, GQCell *first_cell)
  {
  double dcxPg, dcyPg, tmp,
    dNanoToUnits = gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits)) ;
  int
    icxWorld = 0, icyWorld = 0, /* in nanos */
    icxPages = 0, icyPages = 0 ;
  print_OP po = {0} ;

  print_dialog_get_options (PRINT_DIALOG (dialog->dlgPrintProps), &po) ;
  if (NULL != po.pszPrintString) g_free (po.pszPrintString) ;

  calc_world_size (first_cell, &icxWorld, &icyWorld, dialog) ;
  
  dcxPg = po.dPaperCX - po.dLMargin - po.dRMargin ;
  dcyPg = po.dPaperCY - po.dTMargin - po.dBMargin ;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)))
    {
    if (adj == dialog->adjCXPages)
      dNanoToUnits = 
        print_dialog_to_current_units (PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld ;
    else
    if (adj == dialog->adjCYPages)
      dNanoToUnits = 
        print_dialog_to_current_units (PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld ;
    else
      dNanoToUnits = MIN (
        print_dialog_to_current_units (PRINT_DIALOG (dialog->dlgPrintProps),
          (dcxPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)))) / icxWorld,
        print_dialog_to_current_units (PRINT_DIALOG (dialog->dlgPrintProps),
          (dcyPg * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)))) / icyWorld) ;

    dNanoToUnits = floor (dNanoToUnits * 1000.0) / 1000.0 ;

    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjNanoToUnits), dNanoToUnits) ;
    }

  tmp = print_dialog_from_current_units (PRINT_DIALOG (dialog->dlgPrintProps), icxWorld * dNanoToUnits) / dcxPg ;
  icxPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;
  tmp = print_dialog_from_current_units (PRINT_DIALOG (dialog->dlgPrintProps), icyWorld * dNanoToUnits) / dcyPg ;
  icyPages = (int)((fabs (tmp - (double)((int)tmp)) > CEIL_EXCEPTION_EPSILON) ? ceil (tmp) : tmp) ;

  g_signal_handlers_block_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, validate_value_change, NULL) ;
  g_signal_handlers_block_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, validate_value_change, NULL) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)) != icxPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCXPages), icxPages) ;
  if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)) != icyPages)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog->adjCYPages), icyPages) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCXPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, validate_value_change, NULL) ;
  g_signal_handlers_unblock_matched ((gpointer)dialog->adjCYPages, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, validate_value_change, NULL) ;
  }

/* Make sure all spin buttons everywhere always have correct values */
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  check_scale (dialog, adj_changed, GQCELL (gtk_object_get_data (GTK_OBJECT (user_data), "first_cell"))) ;
  }

void user_wants_print_preview (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  GQCell *first_cell = GQCELL (gtk_object_get_data (GTK_OBJECT (user_data), "first_cell")) ;
  print_design_OP po ;
  
  po.pbPrintedObjs = malloc (dialog->icPrintedObjs * sizeof (gboolean)) ;
  po.icPrintedObjs = dialog->icPrintedObjs ;
  
  init_print_design_options (&po, first_cell) ;
  
  do_print_preview ((print_OP *)&po, GTK_WINDOW (dialog->dlgPrintProps), (void *)first_cell, (PrintFunction)print_world) ;
  
  free (po.pbPrintedObjs) ;
  }
