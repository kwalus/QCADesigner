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
// The honeycomb threshhold dialog. This is where the   //
// user picks the (lower,upper) threshholds for inter-  //
// preting the waveform data points as (logic 0,        //
// logic 1, indeterminate). It is used by the graph     //
// dialog.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "support.h"
#include "generic_utils.h"
#include "objects/QCADRadioButton.h"

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *lower_threshold_spin ;
  GtkWidget *upper_threshold_spin ;
  GtkWidget *rbSimple ;
  GtkWidget *rbAverage ;
  GtkWidget *average_samples_spin ;
  } honeycomb_thresholds_D ;

static honeycomb_thresholds_D honeycomb_thresholds_dialog = {NULL} ;

static void create_honeycomb_thresholds_dialog (honeycomb_thresholds_D *dialog) ;

static void spn_value_changed (GtkWidget *widget, gpointer data) ;
//static void rbAverage_toggled (GtkToggleButton *widget, gpointer data) ;

gboolean get_honeycomb_thresholds_from_user (GtkWindow *parent, double *pdThreshLower, double *pdThreshUpper, int *icAverageSamples)
  {
  gboolean bApply = FALSE ;

  if (NULL == honeycomb_thresholds_dialog.dialog)
    create_honeycomb_thresholds_dialog (&honeycomb_thresholds_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (honeycomb_thresholds_dialog.dialog), parent) ;

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.lower_threshold_spin), (*pdThreshLower)) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.upper_threshold_spin), (*pdThreshUpper)) ;
  if ((*icAverageSamples) < 2)
    {
    gtk_widget_set_sensitive (honeycomb_thresholds_dialog.average_samples_spin, FALSE) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (honeycomb_thresholds_dialog.rbSimple), TRUE) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (honeycomb_thresholds_dialog.rbAverage), FALSE) ;
    }
  else
    {
    gtk_widget_set_sensitive (honeycomb_thresholds_dialog.average_samples_spin, TRUE) ;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.average_samples_spin), (*icAverageSamples)) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (honeycomb_thresholds_dialog.rbSimple), FALSE) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (honeycomb_thresholds_dialog.rbAverage), TRUE) ;
    }

  bApply = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (honeycomb_thresholds_dialog.dialog))) ;

  gtk_widget_hide (honeycomb_thresholds_dialog.dialog) ;

  if (bApply)
    {
    (*pdThreshLower)    = gtk_spin_button_get_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.lower_threshold_spin)) ;
    (*pdThreshUpper)    = gtk_spin_button_get_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.upper_threshold_spin)) ;
    (*icAverageSamples) = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (honeycomb_thresholds_dialog.rbAverage)) 
      ? gtk_spin_button_get_value (GTK_SPIN_BUTTON (honeycomb_thresholds_dialog.average_samples_spin))
      : 0 ;
    }

  return bApply ;
  }

static void spn_value_changed (GtkWidget *widget, gpointer data)
  {
  static gboolean bIgnoreChange = FALSE ;

  honeycomb_thresholds_D *dialog = (honeycomb_thresholds_D *)data ;
  double dLower = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->lower_threshold_spin)) ;
  double dUpper = gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->upper_threshold_spin)) ;

  if (dLower > dUpper)
    {
    bIgnoreChange = TRUE ;
    if (widget == dialog->lower_threshold_spin)
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->upper_threshold_spin), dLower) ;
    else // widget == dialog->upper_threshold_spin
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->lower_threshold_spin), dUpper) ;
    bIgnoreChange = FALSE ;
    }
  }

//static void rbAverage_toggled (GtkToggleButton *widget, gpointer data)
//  {gtk_widget_set_sensitive (((honeycomb_thresholds_D *)data)->average_samples_spin, gtk_toggle_button_get_active (widget)) ;}

static void create_honeycomb_thresholds_dialog (honeycomb_thresholds_D *dialog)
  {
  GtkWidget *tblThresh = NULL, *lbl = NULL, *frm = NULL, *tblSmooth ;

  dialog->dialog = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Digital Interpretation")) ;
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), TRUE) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE) ;

  tblThresh = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_show (tblThresh) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dialog)->vbox), tblThresh, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblThresh), 2) ;

  frm = gtk_frame_new (_("Waveform Smoothing")) ;
  gtk_widget_show (frm) ;
  gtk_table_attach (GTK_TABLE (tblThresh), frm, 0, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (frm), 2) ;

  tblSmooth = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_show (tblSmooth) ;
  gtk_container_add (GTK_CONTAINER (frm), tblSmooth) ;

  dialog->rbSimple = g_object_new (QCAD_TYPE_RADIO_BUTTON,
    "label",         _("_No Smoothing"),
    "use-underline", TRUE,
    "visible",       TRUE,
    NULL) ;
//  dialog->rbSimple = gtk_radio_button_new_with_mnemonic (NULL, _("_No Smoothing")) ;
//  gtk_widget_show (dialog->rbSimple) ;
  gtk_table_attach (GTK_TABLE (tblSmooth), dialog->rbSimple, 0, 3, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->rbAverage = g_object_new (QCAD_TYPE_RADIO_BUTTON,
    "label",         _("_Running Average over"),
    "use-underline", TRUE,
    "visible",       TRUE,
    "group",         dialog->rbSimple,
    NULL) ;
//  dialog->rbAverage = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (dialog->rbSimple), _("_Running Average over")) ;
//  gtk_widget_show (dialog->rbAverage) ;
  gtk_table_attach (GTK_TABLE (tblSmooth), dialog->rbAverage, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->average_samples_spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (2, 2, 15, 1, 1, 1)), 1, 0) ;
  gtk_widget_show (dialog->average_samples_spin) ;
  gtk_table_attach (GTK_TABLE (tblSmooth), dialog->average_samples_spin, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  connect_object_properties (G_OBJECT (dialog->rbAverage), "active", G_OBJECT (dialog->average_samples_spin), "sensitive",
    CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL,
    NULL, NULL, NULL) ;
  g_object_notify (G_OBJECT (dialog->rbAverage), "active") ;

  lbl = gtk_label_new (_("samples")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblSmooth), lbl, 2, 3, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  lbl = gtk_label_new (_("Upper Threshold [1]:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblThresh), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->upper_threshold_spin = gtk_spin_button_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (0.5, -1.0, 1.0, 0.005, 0.05, 0.05)), 0.005, 3) ;
  gtk_widget_show (dialog->upper_threshold_spin) ;
  gtk_table_attach (GTK_TABLE (tblThresh), dialog->upper_threshold_spin, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->upper_threshold_spin), TRUE) ;

  lbl = gtk_label_new (_("Lower Threshold [0]:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblThresh), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->lower_threshold_spin = gtk_spin_button_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (-0.5, -1.0, 1.0, 0.005, 0.05, 0.05)), 0.005, 3) ;
  gtk_widget_show (dialog->lower_threshold_spin) ;
  gtk_table_attach (GTK_TABLE (tblThresh), dialog->lower_threshold_spin, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->lower_threshold_spin), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->lower_threshold_spin), "value-changed", (GCallback)spn_value_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->upper_threshold_spin), "value-changed", (GCallback)spn_value_changed, dialog) ;
//  g_signal_connect (G_OBJECT (dialog->rbAverage),            "toggled",       (GCallback)rbAverage_toggled, dialog) ;
  }
