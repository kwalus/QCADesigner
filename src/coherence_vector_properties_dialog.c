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
#include "coherence_vector_properties_dialog.h"

#define DBG_RO(s)

typedef struct{
	GtkWidget *coherence_properties_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *table;

  	GtkWidget *T_entry;
  	GtkWidget *relaxation_entry;
  	GtkWidget *time_step_entry;
  	GtkWidget *duration_entry;
  	GtkWidget *clock_high_entry;
  	GtkWidget *clock_low_entry;
	GtkWidget *radius_of_effect_entry;
  	GtkWidget *epsilonR_entry;
	GtkWidget *chkAnimate;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox2;
  	GtkWidget *coherence_properties_ok_button;
  	GtkWidget *coherence_properties_cancel_button;
}coherence_properties_D;

static coherence_properties_D coherence_properties = {NULL};

static void create_coherence_properties_dialog (coherence_properties_D *dialog) ;
static void coherence_OP_to_dialog (coherence_OP *psco, coherence_properties_D *dialog) ;
//static void coherence_OP_to_dialog_ro (coherence_OP *psco, coherence_properties_D *dialog) ;
static void dialog_to_coherence_OP (coherence_OP *psco, coherence_properties_D *dialog) ;
static void create_coherence_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry) ;
static void properties_changed (GtkWidget *widget, gpointer user_data) ;

void get_coherence_properties_from_user (GtkWindow *parent, coherence_OP *pbo)
  {
  coherence_OP scoLocal = {0} ;
  
  if (NULL == coherence_properties.coherence_properties_dialog)
    create_coherence_properties_dialog (&coherence_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (coherence_properties.coherence_properties_dialog), parent) ;
  
  g_object_set_data (G_OBJECT (coherence_properties.coherence_properties_dialog), "bIgnoreChangeSignal", (gpointer)TRUE) ;
  coherence_OP_to_dialog (pbo, &coherence_properties) ;
  dialog_to_coherence_OP (&scoLocal, &coherence_properties) ;
  coherence_OP_to_dialog (&scoLocal, &coherence_properties) ;
  g_object_set_data (G_OBJECT (coherence_properties.coherence_properties_dialog), "bIgnoreChangeSignal", (gpointer)FALSE) ;
  
  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (coherence_properties.coherence_properties_dialog)))
    dialog_to_coherence_OP (pbo, &coherence_properties) ;

  gtk_widget_hide (coherence_properties.coherence_properties_dialog) ;
  }

static void create_coherence_properties_dialog (coherence_properties_D *dialog)
  {
  GtkWidget *label = NULL ;
  GtkWidget *lblunits = NULL ;  
  if (NULL != dialog->coherence_properties_dialog) return ;
    
  dialog->coherence_properties_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->coherence_properties_dialog), _("Coherence Vector Options"));
  gtk_window_set_policy (GTK_WINDOW (dialog->coherence_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->coherence_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->coherence_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (16, 3, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  create_coherence_properties_line (dialog->table, 0, &(label), &(dialog->T_entry), &lblunits, 
    "Temperature:", "K", TRUE) ;
  create_coherence_properties_line (dialog->table, 1, &(label), &(dialog->relaxation_entry), &lblunits,
    "Relaxation Time:", "s", TRUE) ;
  create_coherence_properties_line (dialog->table, 2, &(label), &(dialog->time_step_entry), &lblunits,
    "Time Step:", "s", TRUE) ;
  create_coherence_properties_line (dialog->table, 3, &(label), &(dialog->duration_entry), &lblunits,
    "Total Simulation Time:", "s", TRUE) ;
  create_coherence_properties_line (dialog->table, 4, &(label), &(dialog->clock_high_entry), &lblunits,
    "Clock High:", "J", TRUE) ;
  create_coherence_properties_line (dialog->table, 5, &(label), &(dialog->clock_low_entry), &lblunits,
    "Clock Low:", "J", TRUE) ;
  create_coherence_properties_line (dialog->table, 6, &(label), &(dialog->radius_of_effect_entry), &lblunits,
    "Radius of Effect:", "nm", TRUE) ;
  create_coherence_properties_line (dialog->table, 7, &(label), &(dialog->epsilonR_entry), NULL,
    "Relative Permittivity:", NULL, TRUE) ;
    
  // Animate ?
  dialog->chkAnimate = gtk_check_button_new_with_label (_("Animate")) ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkAnimate, 0, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_widget_set_sensitive (dialog->chkAnimate, FALSE) ;

  g_signal_connect (G_OBJECT (dialog->T_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->relaxation_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->time_step_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->duration_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_high_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->clock_low_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->radius_of_effect_entry), "changed", (GCallback)properties_changed, dialog) ;
  g_signal_connect (G_OBJECT (dialog->epsilonR_entry), "changed", (GCallback)properties_changed, dialog) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->coherence_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->coherence_properties_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->coherence_properties_dialog), GTK_RESPONSE_OK) ;
}

static void properties_changed (GtkWidget *widget, gpointer user_data)
  {
  coherence_properties_D *dialog = (coherence_properties_D *)user_data ;
  coherence_OP sco ;
  
  if ((gboolean)g_object_get_data (G_OBJECT (dialog->coherence_properties_dialog), "bIgnoreChangeSignal"))
    return ;
  
  dialog_to_coherence_OP (&sco, dialog) ;
  
//  coherence_OP_to_dialog_ro (&sco, dialog) ;
  }

static void create_coherence_properties_line (GtkWidget *table, int idx, GtkWidget **plabel, GtkWidget **pentry, GtkWidget **plblUnits, char *pszLabel, char *pszUnits, gboolean bEnableEntry)
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

static void coherence_OP_to_dialog (coherence_OP *psco, coherence_properties_D *dialog)
  {
  char sz[16] = "" ;

  g_snprintf (sz, 16, "%f", psco->T) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->T_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->relaxation) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->relaxation_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->time_step) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->time_step_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->duration) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->duration_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate), psco->animate_simulation) ;

  g_snprintf (sz, 16, "%e", psco->clock_high) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_high_entry), sz) ;

  g_snprintf (sz, 16, "%e", psco->clock_low) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->clock_low_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->radius_of_effect) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->radius_of_effect_entry), sz) ;

  g_snprintf (sz, 16, "%lf", psco->epsilonR) ;
  gtk_entry_set_text (GTK_ENTRY (dialog->epsilonR_entry), sz) ;
  
//  coherence_OP_to_dialog_ro (psco, dialog) ;
  }
/*
static void coherence_OP_to_dialog_ro (coherence_OP *psco, coherence_properties_D *dialog)
  {
  char sz[16] = "" ;

  }
*/
static void dialog_to_coherence_OP (coherence_OP *psco, coherence_properties_D *dialog)
  {
  psco->T = atof (gtk_entry_get_text (GTK_ENTRY (dialog->T_entry))) ;
  psco->relaxation = atof (gtk_entry_get_text (GTK_ENTRY (dialog->relaxation_entry))) ;
  psco->time_step = atof (gtk_entry_get_text (GTK_ENTRY (dialog->time_step_entry))) ;
  psco->duration = atof (gtk_entry_get_text (GTK_ENTRY (dialog->duration_entry))) ;
  psco->clock_high = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_high_entry))) ;
  psco->clock_low = atof (gtk_entry_get_text (GTK_ENTRY (dialog->clock_low_entry))) ;
  psco->radius_of_effect = atof (gtk_entry_get_text (GTK_ENTRY (dialog->radius_of_effect_entry))) ;
  psco->epsilonR = atof (gtk_entry_get_text (GTK_ENTRY (dialog->epsilonR_entry))) ;
  }
