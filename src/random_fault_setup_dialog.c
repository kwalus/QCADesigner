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
#include "blocking_dialog.h"
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

void create_random_fault_setup_dialog (random_fault_setup_D *dialog) ;
void on_random_fault_setup_dialog_ok_button_clicked(GtkButton *button, gpointer user_data);

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

  show_dialog_blocking (random_fault_setup.random_fault_setup_dialog) ;
  }

void create_random_fault_setup_dialog (random_fault_setup_D *dialog){

  dialog->random_fault_setup_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "random_fault_setup_dialog", dialog->random_fault_setup_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->random_fault_setup_dialog), "Random Fault Setup");
  gtk_window_set_policy (GTK_WINDOW (dialog->random_fault_setup_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->random_fault_setup_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->random_fault_setup_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);
  
  dialog->tblMain = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_ref (dialog->tblMain) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "tblMain", dialog->tblMain,
      	      	      	    (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->tblMain) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblMain, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblMain), 2) ;

  dialog->shift_label = gtk_label_new ("Max Response Function Shift (0.0 - 1.0):");
  gtk_widget_ref (dialog->shift_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "shift_label", dialog->shift_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->shift_label);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->shift_label, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->shift_label), 1.0, 0.5) ;
  gtk_label_set_justify (GTK_LABEL (dialog->shift_label), GTK_JUSTIFY_RIGHT) ;

  dialog->label3 = gtk_label_new ("Probability To Affected Cell:");
  gtk_widget_ref (dialog->label3);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "label3", dialog->label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label3);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->label3, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->label3), 1.0, 0.5) ;
  gtk_label_set_justify (GTK_LABEL (dialog->label3), GTK_JUSTIFY_RIGHT) ;

  dialog->shift_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->shift_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "shift_entry", dialog->shift_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->shift_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->shift_entry, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->percent_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->percent_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "percent_entry", dialog->percent_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->percent_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->percent_entry, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->random_fault_setup_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->random_fault_setup_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox1), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox1), 0, 0);

  dialog->random_fault_setup_dialog_ok_button = gtk_button_new_with_label ("OK");
  gtk_widget_ref (dialog->random_fault_setup_dialog_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "random_fault_setup_dialog_ok_button", dialog->random_fault_setup_dialog_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->random_fault_setup_dialog_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->random_fault_setup_dialog_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->random_fault_setup_dialog_ok_button, GTK_CAN_DEFAULT) ;

  dialog->random_fault_setup_dialog_cancel_button = gtk_button_new_with_label ("Cancel");
  gtk_widget_ref (dialog->random_fault_setup_dialog_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->random_fault_setup_dialog), "random_fault_setup_dialog_cancel_button", dialog->random_fault_setup_dialog_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->random_fault_setup_dialog_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->random_fault_setup_dialog_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->random_fault_setup_dialog_cancel_button, GTK_CAN_DEFAULT) ;

  gtk_signal_connect (GTK_OBJECT (dialog->random_fault_setup_dialog_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_random_fault_setup_dialog_ok_button_clicked),
                      dialog->random_fault_setup_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->random_fault_setup_dialog_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->random_fault_setup_dialog));

  // connect the delete_event function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->random_fault_setup_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->random_fault_setup_dialog));

}

void on_random_fault_setup_dialog_ok_button_clicked(GtkButton *button, gpointer user_data)
  {
  random_fault_setup_D *fault = (random_fault_setup_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog");
  *(float *)gtk_object_get_data (GTK_OBJECT (user_data), "pfMaxResponseShift") =
    CLAMP (atof(gtk_entry_get_text((GtkEntry *)fault->shift_entry)), 0, 1) ;
  *(float *)gtk_object_get_data (GTK_OBJECT (user_data), "pfAffectedCellProb") =
    CLAMP (atof(gtk_entry_get_text((GtkEntry *)fault->percent_entry)), 0, 1) ;
    
  gtk_widget_hide(GTK_WIDGET(fault->random_fault_setup_dialog));
  }
