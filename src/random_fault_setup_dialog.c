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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>


#include "support.h"
#include "stdqcell.h"
#include "random_fault_setup_dialog.h"

typedef struct{
  GtkWidget *random_fault_setup_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *tblMain;
  GtkWidget *hbox2;
  GtkWidget *vbox1;
  GtkWidget *shift_entry;
  GtkWidget *percent_entry;
  GtkWidget *vbox2;
  GtkWidget *shift_label;
  GtkWidget *label3;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox1;
  GtkWidget *random_fault_setup_dialog_cancel_button;
  GtkWidget *random_fault_setup_dialog_ok_button;
}random_fault_setup_D;

random_fault_setup_D random_fault_setup = {NULL} ;

static void create_random_fault_setup_dialog (random_fault_setup_D *dialog) ;

void get_random_fault_params_from_user (GtkWindow *parent, float *pfMaxResponseShift, float *pfAffectedCellProb)
  {
  char szText[16] = "" ;
  
  if (NULL == random_fault_setup.random_fault_setup_dialog)
    create_random_fault_setup_dialog (&random_fault_setup) ;
    
  gtk_object_set_data (GTK_OBJECT (random_fault_setup.random_fault_setup_dialog), "dialog", &random_fault_setup) ;
  gtk_object_set_data (GTK_OBJECT (random_fault_setup.random_fault_setup_dialog), "pfMaxResponseShift", pfMaxResponseShift) ;
  gtk_object_set_data (GTK_OBJECT (random_fault_setup.random_fault_setup_dialog), "pfAffectedCellProb", pfAffectedCellProb) ;
    
  gtk_window_set_transient_for (GTK_WINDOW (random_fault_setup.random_fault_setup_dialog), parent) ;
  
  g_snprintf(szText, 16, "%1.2f", *pfMaxResponseShift);
  gtk_entry_set_text(GTK_ENTRY (random_fault_setup.shift_entry), szText);
  g_snprintf(szText, 16, "%1.2f", *pfAffectedCellProb);
  gtk_entry_set_text(GTK_ENTRY (random_fault_setup.percent_entry), szText);

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (random_fault_setup.random_fault_setup_dialog)))
    {
    *(float *)pfMaxResponseShift = CLAMP (atof(gtk_entry_get_text((GtkEntry *)random_fault_setup.shift_entry)), 0, 1) ;
    *(float *)pfAffectedCellProb = CLAMP (atof(gtk_entry_get_text((GtkEntry *)random_fault_setup.percent_entry)), 0, 1) ;
    }
  
  gtk_widget_hide (random_fault_setup.random_fault_setup_dialog) ;
  }

static void create_random_fault_setup_dialog (random_fault_setup_D *dialog){

  dialog->random_fault_setup_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "random_fault_setup_dialog", dialog->random_fault_setup_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->random_fault_setup_dialog), "Random Fault Setup");
  gtk_window_set_policy (GTK_WINDOW (dialog->random_fault_setup_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->random_fault_setup_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->random_fault_setup_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);
  
  dialog->tblMain = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (dialog->tblMain) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblMain, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblMain), 2) ;

  dialog->shift_label = gtk_label_new ("Max Response Function Shift (0.0 - 1.0):");
  gtk_widget_show (dialog->shift_label);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->shift_label, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->shift_label), 1.0, 0.5) ;
  gtk_label_set_justify (GTK_LABEL (dialog->shift_label), GTK_JUSTIFY_RIGHT) ;

  dialog->label3 = gtk_label_new ("Probability To Affected Cell:");
  gtk_widget_show (dialog->label3);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->label3, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->label3), 1.0, 0.5) ;
  gtk_label_set_justify (GTK_LABEL (dialog->label3), GTK_JUSTIFY_RIGHT) ;

  dialog->shift_entry = gtk_entry_new ();
  gtk_widget_show (dialog->shift_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->shift_entry, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->percent_entry = gtk_entry_new ();
  gtk_widget_show (dialog->percent_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->percent_entry, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->random_fault_setup_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->random_fault_setup_dialog_cancel_button = 
    gtk_dialog_add_button (GTK_DIALOG (dialog->random_fault_setup_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  dialog->random_fault_setup_dialog_ok_button = 
    gtk_dialog_add_button (GTK_DIALOG (dialog->random_fault_setup_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->random_fault_setup_dialog), GTK_RESPONSE_OK) ;
}
