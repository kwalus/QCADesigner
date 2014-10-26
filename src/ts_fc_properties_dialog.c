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
// Contents:                                            //
//                                                      //
// This UI allows the user to specify parameters for    //
// the ts_fc vector simulation engine.              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "global_consts.h"
#include "ts_fc_properties_dialog.h"

#define DBG_RO(s)

typedef struct
  {
  GtkWidget *ts_fc_properties_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table;
  GtkWidget *temp_entry;
  GtkWidget *time_step_entry;
  GtkWidget *duration_entry;
	 GtkWidget *tolerance_entry;  
	GtkWidget *gamma_entry;
  GtkWidget *clock_high_entry;
  GtkWidget *clock_low_entry;
  GtkWidget *clock_shift_entry;
  GtkWidget *Emax_entry;
  GtkWidget *radius_of_effect_entry;
  GtkWidget *epsilonR_entry;
	GtkWidget *layer_separation_entry;
	GtkWidget *cell_elevation_entry;
	GtkWidget *cell_height_entry;
	  GtkWidget *counter_ion_entry;
	  GtkWidget *dx_entry;
	  GtkWidget *dy_entry;
	  GtkWidget *dz_entry;
	  GtkWidget *lambda_x_entry;
	  GtkWidget *lambda_y_entry;
	GtkWidget *cont_clocking_radio;
	GtkWidget *electrode_clocking_radio;
	GtkWidget *euler_method_radio;
	GtkWidget *runge_kutta_radio;
	  GtkWidget *chkTemp;
	GtkWidget *chkRandomizeCells;
	GtkWidget *chkAnimate;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox2;
  GtkWidget *ts_fc_properties_ok_button;
  GtkWidget *ts_fc_properties_cancel_button;
  GSList    *radio_group;
  } ts_fc_properties_D;

static ts_fc_properties_D ts_fc_properties = {NULL};

static void create_ts_fc_properties_dialog (ts_fc_properties_D *dialog) ;
static void ts_fc_OP_to_dialog (ts_fc_OP *psco, ts_fc_properties_D *dialog) ;
static void dialog_to_ts_fc_OP (ts_fc_OP *psco, ts_fc_properties_D *dialog) ;
static void create_ts_fc_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry) ;
static void properties_changed (GtkWidget *widget, gpointer user_data) ;

void get_ts_fc_properties_from_user (GtkWindow *parent, ts_fc_OP *pbo)
  {
  ts_fc_OP scoLocal = {0} ;

  if (NULL == ts_fc_properties.ts_fc_properties_dialog)
    create_ts_fc_properties_dialog (&ts_fc_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (ts_fc_properties.ts_fc_properties_dialog), parent) ;

  g_object_set_data (G_OBJECT (ts_fc_properties.ts_fc_properties_dialog), "bIgnoreChangeSignal", (gpointer)TRUE) ;
  ts_fc_OP_to_dialog (pbo, &ts_fc_properties) ;
  dialog_to_ts_fc_OP (&scoLocal, &ts_fc_properties) ;
  ts_fc_OP_to_dialog (&scoLocal, &ts_fc_properties) ;
  g_object_set_data (G_OBJECT (ts_fc_properties.ts_fc_properties_dialog), "bIgnoreChangeSignal", (gpointer)FALSE) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (ts_fc_properties.ts_fc_properties_dialog)))
    dialog_to_ts_fc_OP (pbo, &ts_fc_properties) ;

  gtk_widget_hide (ts_fc_properties.ts_fc_properties_dialog) ;
  }

static void create_ts_fc_properties_dialog (ts_fc_properties_D *dialog)
  {
  GtkWidget *label = NULL ;
  GtkWidget *lblunits = NULL ;
  if (NULL != dialog->ts_fc_properties_dialog) return ;

  dialog->ts_fc_properties_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->ts_fc_properties_dialog), _("Three State Coherence Options"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->ts_fc_properties_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->ts_fc_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->ts_fc_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (26, 3, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  create_ts_fc_properties_line (dialog->table,  1, &(label), &(dialog->temp_entry),              &lblunits, _("Temperature:"),              "K",  TRUE) ;	  
  create_ts_fc_properties_line (dialog->table,  2, &(label), &(dialog->time_step_entry),              &lblunits, _("Time Step:"),              "s",  TRUE) ;
  create_ts_fc_properties_line (dialog->table,  3, &(label), &(dialog->duration_entry),               &lblunits, _("Total Simulation Time:"),  "s",  TRUE) ;
	  create_ts_fc_properties_line (dialog->table,  4, &(label), &(dialog->tolerance_entry),              &lblunits, _("Convergence Tolerance:"),              NULL,  TRUE) ;
	create_ts_fc_properties_line (dialog->table,  5, &(label), &(dialog->gamma_entry),									 &lblunits, _("Tunneling Energy:"),  "J",  TRUE) ;
  create_ts_fc_properties_line (dialog->table,  6, &(label), &(dialog->clock_high_entry),             &lblunits, _("Clock High:"),             "J",  TRUE) ;
  create_ts_fc_properties_line (dialog->table,  7, &(label), &(dialog->clock_low_entry),              &lblunits, _("Clock Low:"),              "J",  TRUE) ;
  create_ts_fc_properties_line (dialog->table,  8, &(label), &(dialog->clock_shift_entry),            &lblunits, _("Clock Shift:"),            NULL, TRUE) ;
  create_ts_fc_properties_line (dialog->table,  9, &(label), &(dialog->Emax_entry),				&lblunits, _("Emax:"), NULL, TRUE) ;
  create_ts_fc_properties_line (dialog->table,  10, &(label), &(dialog->radius_of_effect_entry),       &lblunits, _("Radius of Effect:"),       "nm", TRUE) ;
  create_ts_fc_properties_line (dialog->table, 11, &(label), &(dialog->epsilonR_entry),               NULL,      _("Relative Permittivity:"),  NULL, TRUE) ;
  create_ts_fc_properties_line (dialog->table, 12, &(label), &(dialog->layer_separation_entry),       &lblunits, _("Layer Separation:"),       "nm", TRUE) ;
	create_ts_fc_properties_line (dialog->table, 13, &(label), &(dialog->cell_elevation_entry),				 &lblunits, _("Cell Elevation:"),         "nm", TRUE) ;
  create_ts_fc_properties_line (dialog->table, 14, &(label), &(dialog->cell_height_entry),            &lblunits, _("Cell Height:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 15, &(label), &(dialog->counter_ion_entry),            &lblunits, _("Location of counter-ion:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 16, &(label), &(dialog->dx_entry),            &lblunits, _("dx:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 17, &(label), &(dialog->dy_entry),            &lblunits, _("dy:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 18, &(label), &(dialog->dz_entry),            &lblunits, _("dz:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 19, &(label), &(dialog->lambda_x_entry),            &lblunits, _("lambda x:"),						"nm", TRUE) ;
	  create_ts_fc_properties_line (dialog->table, 20, &(label), &(dialog->lambda_y_entry),            &lblunits, _("lambda y:"),						"nm", TRUE) ;

	  
	dialog->cont_clocking_radio = gtk_radio_button_new_with_mnemonic (dialog->radio_group, _("_Continuous Clocking"));
  g_object_set_data (G_OBJECT (dialog->cont_clocking_radio), "which_options", (gpointer)CONT_CLOCKING) ;
  dialog->radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->cont_clocking_radio));
  gtk_widget_show (dialog->cont_clocking_radio);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->cont_clocking_radio, 0, 2, 21, 22,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->electrode_clocking_radio = gtk_radio_button_new_with_mnemonic (dialog->radio_group, _("_Electrode Clocking"));
  g_object_set_data (G_OBJECT (dialog->electrode_clocking_radio), "which_options", (gpointer)ELECTRODE_CLOCKING) ;
  dialog->radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->electrode_clocking_radio));
  gtk_widget_show (dialog->electrode_clocking_radio);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->electrode_clocking_radio, 0, 2, 22, 23,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

	  
      dialog->chkTemp = gtk_check_button_new_with_label (_("Thermodynamic Model")) ;
	  gtk_widget_show (dialog->chkTemp) ;
	  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkTemp, 0, 2, 23, 24,
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);  
	  
	  
  // Randomize Cells ?
  dialog->chkRandomizeCells = gtk_check_button_new_with_label (_("Randomize Simulation Order")) ;
  gtk_widget_show (dialog->chkRandomizeCells) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkRandomizeCells, 0, 2, 24, 25,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  // Animate ?
  dialog->chkAnimate = gtk_check_button_new_with_label (_("Animate")) ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkAnimate, 0, 2, 25, 26,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
	  
  g_signal_connect (G_OBJECT (dialog->temp_entry),              "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->time_step_entry),              "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->duration_entry),               "changed", (GCallback)properties_changed, dialog) ;
	   g_signal_connect (G_OBJECT (dialog->tolerance_entry),               "changed", (GCallback)properties_changed, dialog) ;
	g_signal_connect (G_OBJECT (dialog->gamma_entry),                  "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_high_entry),             "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_low_entry),              "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_shift_entry),            "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->Emax_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->radius_of_effect_entry),       "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->epsilonR_entry),               "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->layer_separation_entry),       "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->cell_elevation_entry),				 "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->cell_height_entry),						 "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->counter_ion_entry),						 "changed", (GCallback)properties_changed, dialog) ;
	  g_signal_connect (G_OBJECT (dialog->dx_entry),						 "changed", (GCallback)properties_changed, dialog) ;
	g_signal_connect (G_OBJECT (dialog->dy_entry),						 "changed", (GCallback)properties_changed, dialog) ;
	  g_signal_connect (G_OBJECT (dialog->dz_entry),						 "changed", (GCallback)properties_changed, dialog) ;
	   g_signal_connect (G_OBJECT (dialog->lambda_x_entry),						 "changed", (GCallback)properties_changed, dialog) ;
	  g_signal_connect (G_OBJECT (dialog->lambda_y_entry),						 "changed", (GCallback)properties_changed, dialog) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->ts_fc_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->ts_fc_properties_dialog), GTK_STOCK_OK,     GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->ts_fc_properties_dialog), GTK_RESPONSE_OK) ;
  }

static void properties_changed (GtkWidget *widget, gpointer user_data)
  {
  ts_fc_properties_D *dialog = (ts_fc_properties_D *)user_data ;
  ts_fc_OP sco ;

  if ((gboolean)g_object_get_data (G_OBJECT (dialog->ts_fc_properties_dialog), "bIgnoreChangeSignal"))
    return ;

  dialog_to_ts_fc_OP (&sco, dialog) ;
  }

static void create_ts_fc_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry)
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

static void ts_fc_OP_to_dialog (ts_fc_OP *psco, ts_fc_properties_D *dialog)
  {
  char sz[16] = "" ;

	  g_snprintf (sz, 16, "%f", psco->temperature) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->temp_entry), sz) ;	  
	  
  g_snprintf (sz, 16, "%e", psco->time_step) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->time_step_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->duration) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->duration_entry), sz) ;
	
	  g_snprintf (sz, 16, "%e", psco->convergence_tolerance) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->tolerance_entry), sz) ;	  
	  
	g_snprintf (sz, 16, "%e", psco->gamma) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->gamma_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkTemp), psco->temp_model) ;	  
	  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkRandomizeCells), psco->randomize_cells) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate), psco->animate_simulation) ;

  g_snprintf (sz, 16, "%e", psco->clock_high) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_high_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->clock_low) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_low_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->clock_shift) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_shift_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->Emax) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->Emax_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->radius_of_effect) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->radius_of_effect_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->epsilonR) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->epsilonR_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->layer_separation) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->layer_separation_entry), sz) ;
  
	g_snprintf (sz, 16, "%lf", psco->cell_elevation) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->cell_elevation_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->cell_height) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->cell_height_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->counter_ion) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->counter_ion_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->dx) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->dx_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->dy) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->dy_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->dz) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->dz_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->lambda_x) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->lambda_x_entry), sz) ;
	  
	  g_snprintf (sz, 16, "%lf", psco->lambda_y) ;
	  gtk_entry_set_text (GTK_ENTRY (dialog->lambda_y_entry), sz) ;
	
	if (CONT_CLOCKING == psco->clocking_scheme)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->cont_clocking_radio), TRUE);
  else
  if (ELECTRODE_CLOCKING == psco->clocking_scheme)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->electrode_clocking_radio), TRUE);
	
	/*
  if (EULER_METHOD == psco->algorithm)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->euler_method_radio), TRUE);
  else
  if (RUNGE_KUTTA == psco->algorithm)
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->runge_kutta_radio), TRUE);
  */
	}

static void dialog_to_ts_fc_OP (ts_fc_OP *psco, ts_fc_properties_D *dialog)
  {
  psco->temperature            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->temp_entry))) ;	  
  psco->time_step              = atof (gtk_entry_get_text (GTK_ENTRY (dialog->time_step_entry))) ;
  psco->duration               = atof (gtk_entry_get_text (GTK_ENTRY (dialog->duration_entry))) ;
  psco->convergence_tolerance  = atof (gtk_entry_get_text (GTK_ENTRY (dialog->tolerance_entry))) ;
	psco->gamma				   = atof (gtk_entry_get_text (GTK_ENTRY (dialog->gamma_entry))) ;
  psco->clock_high             = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_high_entry))) ;
  psco->clock_low              = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_low_entry))) ;
  psco->clock_shift            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_shift_entry))) ;
  psco->Emax				   = atof (gtk_entry_get_text (GTK_ENTRY (dialog->Emax_entry))) ;
  psco->radius_of_effect       = atof (gtk_entry_get_text (GTK_ENTRY (dialog->radius_of_effect_entry))) ;
  psco->epsilonR               = atof (gtk_entry_get_text (GTK_ENTRY (dialog->epsilonR_entry))) ;
  psco->layer_separation       = atof (gtk_entry_get_text (GTK_ENTRY (dialog->layer_separation_entry))) ;
	psco->cell_elevation				 = atof (gtk_entry_get_text (GTK_ENTRY (dialog->cell_elevation_entry))) ;
  psco->cell_height            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->cell_height_entry))) ;
	  psco->counter_ion            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->counter_ion_entry))) ;
	  psco->dx            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->dx_entry))) ;
	  psco->dy            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->dy_entry))) ;
	  psco->dz            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->dz_entry))) ;
	  psco->lambda_x            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->lambda_x_entry))) ;
	  psco->lambda_y            = atof (gtk_entry_get_text (GTK_ENTRY (dialog->lambda_y_entry))) ;
	  
	psco->clocking_scheme        = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->cont_clocking_radio)) ? CONT_CLOCKING : ELECTRODE_CLOCKING;
  //psco->algorithm            = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->euler_method_radio)) ? EULER_METHOD : RUNGE_KUTTA;
  psco->temp_model           = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->chkTemp)) ;
  psco->randomize_cells        = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->chkRandomizeCells)) ;
  psco->animate_simulation     = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate)) ;
  }
