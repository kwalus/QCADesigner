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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "scqca_constants.h"
#include "scqca_properties_dialog.h"

#define DBG_RO(s)

typedef struct{
	GtkWidget *scqca_properties_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *table;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *label4;
	GtkWidget *label5;
	GtkWidget *lblMaxIter;
  	GtkWidget *max_iterations_per_sample_entry;
  	GtkWidget *number_of_samples_entry;
  	GtkWidget *circuit_convergence_tolerance_entry;
  	GtkWidget *dot_convergence_tolerance_entry;
  	GtkWidget *cell_convergence_tolerance_entry;
  	GtkWidget *radius_of_effect_entry;
  	GtkWidget *chkAnimate;
        GtkWidget *lblMaxCellLoops;
        GtkWidget *max_cell_loops_entry;
        GtkWidget *lblMaxDotLoops;
        GtkWidget *max_dot_loops_entry;
        GtkWidget *lblInputCellCurrent;
        GtkWidget *input_cell_current_entry;
        GtkWidget *temperature_entry;
        GtkWidget *device_voltage_entry;
        GtkWidget *gamma_entry;
        GtkWidget *clock_high_entry;
        GtkWidget *clock_low_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox2;
  	GtkWidget *scqca_properties_ok_button;
  	GtkWidget *scqca_properties_cancel_button;
        GtkWidget *electron_lifetime_entry ;
        GtkWidget *Epk1_entry ;
        GtkWidget *Epk2_entry ;
        GtkWidget *U_entry ;
}scqca_properties_D;

static scqca_properties_D scqca_properties = {NULL};

static void create_scqca_properties_dialog (scqca_properties_D *dialog) ;
static void scqca_OP_to_dialog (scqca_OP *psco, scqca_properties_D *dialog) ;
static void scqca_OP_to_dialog_ro (scqca_OP *psco, scqca_properties_D *dialog) ;
static void dialog_to_scqca_OP (scqca_OP *psco, scqca_properties_D *dialog) ;
static void create_scqca_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry) ;
static void properties_changed (GtkWidget *widget, gpointer user_data) ;

void get_scqca_properties_from_user (GtkWindow *parent, scqca_OP *pbo)
  {
  scqca_OP scoLocal = {0} ;
  
  if (NULL == scqca_properties.scqca_properties_dialog)
    create_scqca_properties_dialog (&scqca_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (scqca_properties.scqca_properties_dialog), parent) ;
  
  g_object_set_data (G_OBJECT (scqca_properties.scqca_properties_dialog), "bIgnoreChangeSignal", (gpointer)TRUE) ;
  scqca_OP_to_dialog (pbo, &scqca_properties) ;
  dialog_to_scqca_OP (&scoLocal, &scqca_properties) ;
  scqca_OP_to_dialog (&scoLocal, &scqca_properties) ;
  g_object_set_data (G_OBJECT (scqca_properties.scqca_properties_dialog), "bIgnoreChangeSignal", (gpointer)FALSE) ;
  
  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (scqca_properties.scqca_properties_dialog)))
    dialog_to_scqca_OP (pbo, &scqca_properties) ;

  gtk_widget_hide (scqca_properties.scqca_properties_dialog) ;
  }

static void create_scqca_properties_dialog (scqca_properties_D *dialog)
  {
  GtkWidget *hsep = NULL ;

  if (NULL != dialog->scqca_properties_dialog) return ;
    
  dialog->scqca_properties_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->scqca_properties_dialog), _("Split Current QCA Options"));
  gtk_window_set_policy (GTK_WINDOW (dialog->scqca_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->scqca_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->scqca_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (7, 2, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  create_scqca_properties_line (dialog->table, 0, &(dialog->label1), &(dialog->number_of_samples_entry), NULL, 
    "Number of Samples:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 1, &(dialog->label2), &(dialog->circuit_convergence_tolerance_entry), NULL,
    "Circuit Convergence Tolerance:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 2, &(dialog->label2), &(dialog->dot_convergence_tolerance_entry), NULL,
    "Dot Convergence Tolerance:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 3, &(dialog->label2), &(dialog->cell_convergence_tolerance_entry), NULL,
    "Cell Convergence Tolerance:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 4, &(dialog->label3), &(dialog->radius_of_effect_entry), &(dialog->label2),
    "Radius of Effect:", "nm", TRUE) ;
  create_scqca_properties_line (dialog->table, 5, &(dialog->label3), &(dialog->max_iterations_per_sample_entry), NULL,
    "Maximum Iterations Per Sample:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 6, &(dialog->lblMaxCellLoops), &(dialog->max_cell_loops_entry), NULL,
    "Maximum Loops Per Cell:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 7, &(dialog->lblMaxDotLoops), &(dialog->max_dot_loops_entry), NULL,
    "Maximum Loops Per Dot:", NULL, TRUE) ;
  create_scqca_properties_line (dialog->table, 8, &(dialog->label2), &(dialog->gamma_entry), &(dialog->label2),
    "Gamma:", "eV", TRUE) ;
  create_scqca_properties_line (dialog->table, 9, &(dialog->lblInputCellCurrent), &(dialog->input_cell_current_entry), &(dialog->label2),
    "Input Cell Current:", "A", TRUE) ;
  create_scqca_properties_line (dialog->table, 10, &(dialog->label2), &(dialog->temperature_entry), &(dialog->label2),
    "Temperature:", "K", TRUE) ;
  create_scqca_properties_line (dialog->table, 11, &(dialog->label2), &(dialog->device_voltage_entry), &(dialog->label2),
    "Device Voltage:", "V", TRUE) ;
  create_scqca_properties_line (dialog->table, 12, &(dialog->label2), &(dialog->clock_high_entry), &(dialog->label2),
    "Clock High:", "V", TRUE) ;
  create_scqca_properties_line (dialog->table, 13, &(dialog->label2), &(dialog->clock_low_entry), &(dialog->label2),
    "Clock Low:", "V", TRUE) ;
  
  // Animate ?
  dialog->chkAnimate = gtk_check_button_new_with_label (_("Animate")) ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkAnimate, 0, 2, 14, 15,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_widget_set_sensitive (dialog->chkAnimate, FALSE) ;

  gtk_table_attach (GTK_TABLE (dialog->table), hsep = gtk_hseparator_new (), 0, 3, 15, 16,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_widget_show (hsep) ;

  create_scqca_properties_line (dialog->table, 16, &(dialog->label2), &(dialog->Epk1_entry), &(dialog->label2),
    "Epk1:", "eV", FALSE) ;
  create_scqca_properties_line (dialog->table, 17, &(dialog->label2), &(dialog->Epk2_entry), &(dialog->label2),
    "Epk2:", "eV", FALSE) ;
  create_scqca_properties_line (dialog->table, 18, &(dialog->label2), &(dialog->U_entry), &(dialog->label2),
    "U:", "eV", FALSE) ;
  create_scqca_properties_line (dialog->table, 19, &(dialog->label2), &(dialog->electron_lifetime_entry), &(dialog->label2),
    "Electron Lifetime:", "s", FALSE) ;
    
  g_signal_connect (G_OBJECT (dialog->max_iterations_per_sample_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->number_of_samples_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->circuit_convergence_tolerance_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->dot_convergence_tolerance_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->cell_convergence_tolerance_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->radius_of_effect_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->max_cell_loops_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->max_dot_loops_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->input_cell_current_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->temperature_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->device_voltage_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->gamma_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_high_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_low_entry), "changed", (GCallback)properties_changed, dialog) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->scqca_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->scqca_properties_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->scqca_properties_dialog), GTK_RESPONSE_OK) ;
}

static void properties_changed (GtkWidget *widget, gpointer user_data)
  {
  scqca_properties_D *dialog = (scqca_properties_D *)user_data ;
  scqca_OP sco ;
  
  if ((gboolean)g_object_get_data (G_OBJECT (dialog->scqca_properties_dialog), "bIgnoreChangeSignal"))
    return ;
  
  dialog_to_scqca_OP (&sco, dialog) ;
  
  scqca_OP_to_dialog_ro (&sco, dialog) ;
  }

static void create_scqca_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry)
  {
  // Electron lifetime
  (*plabel) = gtk_label_new (_(pszLabel));
  gtk_widget_show ((*plabel));
  gtk_table_attach (GTK_TABLE (table), (*plabel), 0, 1, idx, idx + 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL ((*plabel)), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC ((*plabel)), 1, 0.5);

  (*pentry) = gtk_entry_new ();
  gtk_widget_show ((*pentry));
  gtk_table_attach (GTK_TABLE (table), (*pentry), 1, 2, idx, idx + 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY ((*pentry)), bEnableEntry) ;
  gtk_widget_set_sensitive ((*pentry), bEnableEntry) ;

  if (NULL != pszUnits)
    {
    (*plblUnits) = gtk_label_new (_(pszUnits));
    gtk_widget_show ((*plblUnits));
    gtk_table_attach (GTK_TABLE (table), (*plblUnits), 2, 3, idx, idx + 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
    gtk_label_set_justify (GTK_LABEL ((*plblUnits)), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC ((*plblUnits)), 0, 0.5);
    }
  }

static void scqca_OP_to_dialog (scqca_OP *psco, scqca_properties_D *dialog)
  {
  char sz[16] = "" ;

  g_snprintf (sz, 16, "%d", psco->number_of_samples) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->number_of_samples_entry), sz) ;

  g_snprintf (sz, 16, "%d", psco->max_iterations_per_sample) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->max_iterations_per_sample_entry), sz) ;

  g_snprintf (sz, 16, "%d", psco->max_cell_loops) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->max_cell_loops_entry), sz) ;

  g_snprintf (sz, 16, "%d", psco->max_dot_loops) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->max_dot_loops_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate), psco->animate_simulation) ;

  g_snprintf (sz, 16, "%e", psco->circuit_convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->circuit_convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->cell_convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->cell_convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->dot_convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->dot_convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->device_voltage) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->device_voltage_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->input_cell_current) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->input_cell_current_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->gamma / P_E) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->gamma_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->clock_high) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_high_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->clock_low) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_low_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->temperature) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->temperature_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->radius_of_effect) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->radius_of_effect_entry), sz) ;
  
  scqca_OP_to_dialog_ro (psco, dialog) ;
  }

static void scqca_OP_to_dialog_ro (scqca_OP *psco, scqca_properties_D *dialog)
  {
  char sz[16] = "" ;

  // Uneditable quantities
  g_snprintf (sz, 16, "%lf", psco->Epk1 / P_E) ;
  DBG_RO (fprintf (stderr, "Setting Epk1 = %s\n", sz)) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->Epk1_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->Epk2 / P_E) ;
  DBG_RO (fprintf (stderr, "Setting Epk2 = %s\n", sz)) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->Epk2_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->U / P_E) ;
  DBG_RO (fprintf (stderr, "Setting U = %s\n", sz)) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->U_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->electron_lifetime) ;
  DBG_RO (fprintf (stderr, "Setting electron_lifetime = %s\n", sz)) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->electron_lifetime_entry), sz) ;
  }

static void dialog_to_scqca_OP (scqca_OP *psco, scqca_properties_D *dialog)
  {
  psco->max_iterations_per_sample = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->max_iterations_per_sample_entry))) ;
  psco->number_of_samples = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->number_of_samples_entry))) ;
  psco->max_cell_loops = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->max_cell_loops_entry))) ;
  psco->max_dot_loops = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->max_dot_loops_entry))) ;
  psco->circuit_convergence_tolerance = atof (gtk_entry_get_text (GTK_ENTRY (dialog->circuit_convergence_tolerance_entry))) ;
  psco->dot_convergence_tolerance = atof (gtk_entry_get_text (GTK_ENTRY (dialog->dot_convergence_tolerance_entry))) ;
  psco->cell_convergence_tolerance = atof (gtk_entry_get_text (GTK_ENTRY (dialog->cell_convergence_tolerance_entry))) ;
  psco->radius_of_effect = atof (gtk_entry_get_text (GTK_ENTRY (dialog->radius_of_effect_entry))) ;
  psco->input_cell_current = atof (gtk_entry_get_text (GTK_ENTRY (dialog->input_cell_current_entry))) ;
  psco->temperature = atof (gtk_entry_get_text (GTK_ENTRY (dialog->temperature_entry))) ;
  psco->gamma = atof (gtk_entry_get_text (GTK_ENTRY (dialog->gamma_entry))) * P_E ;
  psco->clock_high = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_high_entry))) ;
  psco->clock_low = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_low_entry))) ;
  psco->device_voltage = atof (gtk_entry_get_text (GTK_ENTRY (dialog->device_voltage_entry))) ;
  psco->animate_simulation = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate)) ;
  
  psco->Epk2 = 0.2 * P_E;
  psco->Epk1 = psco->Epk2 - P_E * psco->device_voltage / 3;
  psco->U = 2 * (psco->Epk1 - P_E * psco->device_voltage / 3);
  
  // Make sure we don't divide by 0
  if (0 != psco->gamma)
    psco->electron_lifetime = P_HBAR / psco->gamma;
  }
