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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "globals.h"
#include "support.h"
#include "blocking_dialog.h"
#include "vector_table_options_dialog.h"
#include "new_vector_table_options_dialog.h"
#include "sim_type_setup_dialog.h"
#include "vector_table.h"

#define DBG_STS(s)

typedef struct{
  GtkWidget *simulation_type_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GSList *vbox1_group;
  GtkWidget *digital_verif_radio;
  GtkWidget *vector_table_radio;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox1;
  GtkWidget *simulation_type_options_button;
  GtkWidget *simulation_type_cancel_button;
  GtkWidget *simulation_type_ok_button;
}sim_type_setup_D;  

static sim_type_setup_D sim_type_setup_dialog = {NULL} ;

void on_sim_type_options_button_clicked(GtkButton *button, gpointer user_data);
void on_sim_type_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_digital_verif_radio_toggled(GtkButton *button, gpointer user_data);
void on_vector_table_radio_toggled(GtkButton *button, gpointer user_data);
void create_sim_type_dialog(sim_type_setup_D *dialog);

void get_sim_type_from_user (GtkWindow *parent, int *piSimType, VectorTable *pvt)
  {
  if (NULL == sim_type_setup_dialog.simulation_type_dialog)
    create_sim_type_dialog (&sim_type_setup_dialog) ;
  
  gtk_object_set_data (GTK_OBJECT (sim_type_setup_dialog.simulation_type_dialog), "dialog", &sim_type_setup_dialog) ;
  gtk_object_set_data (GTK_OBJECT (sim_type_setup_dialog.simulation_type_dialog), "piSimType", piSimType) ;
  gtk_object_set_data (GTK_OBJECT (sim_type_setup_dialog.simulation_type_dialog), "pvt", pvt) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (sim_type_setup_dialog.simulation_type_dialog), parent) ;
  
  if (VECTOR_TABLE == *piSimType && pvt->num_of_inputs > 0)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_type_setup_dialog.vector_table_radio), TRUE) ;
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_type_setup_dialog.digital_verif_radio), TRUE) ;

  gtk_widget_set_sensitive (sim_type_setup_dialog.vector_table_radio, (pvt->num_of_inputs > 0)) ;

  gtk_widget_set_sensitive (sim_type_setup_dialog.simulation_type_options_button, (VECTOR_TABLE == *piSimType)) ;
  
  show_dialog_blocking (sim_type_setup_dialog.simulation_type_dialog) ;
  }

void create_sim_type_dialog (sim_type_setup_D *dialog){
  if (NULL != dialog->simulation_type_dialog) return ;
    
  dialog->simulation_type_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->simulation_type_dialog), "simulation_type_dialog", dialog->simulation_type_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->simulation_type_dialog), "Set Simulation Type");
  gtk_window_set_policy (GTK_WINDOW (dialog->simulation_type_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->simulation_type_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->simulation_type_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->simulation_type_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->vbox1 = gtk_table_new (2, 1, FALSE);
  gtk_widget_ref (dialog->vbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "vbox1", dialog->vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox1), 2) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->vbox1, TRUE, TRUE, 0);

  dialog->digital_verif_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, "Exhaustive Verification");
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->digital_verif_radio));
  gtk_widget_ref (dialog->digital_verif_radio);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "digital_verif_radio", dialog->digital_verif_radio,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->digital_verif_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->digital_verif_radio, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->vector_table_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, "Vector Table");
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->vector_table_radio));
  gtk_widget_ref (dialog->vector_table_radio);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "vector_table_radio", dialog->vector_table_radio,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vector_table_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->vector_table_radio, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->simulation_type_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->simulation_type_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox1), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox1), 0, 0);

  dialog->simulation_type_options_button = gtk_button_new_with_label (_("Options"));
  gtk_widget_ref (dialog->simulation_type_options_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "simulation_type_options_button", dialog->simulation_type_options_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->simulation_type_options_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->simulation_type_options_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->simulation_type_options_button, GTK_CAN_DEFAULT);

  dialog->simulation_type_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->simulation_type_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "simulation_type_ok_button", dialog->simulation_type_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->simulation_type_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->simulation_type_ok_button);
  GTK_WIDGET_SET_FLAGS (dialog->simulation_type_ok_button, GTK_CAN_DEFAULT);

  dialog->simulation_type_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->simulation_type_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->simulation_type_dialog), "simulation_type_cancel_button", dialog->simulation_type_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->simulation_type_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->simulation_type_cancel_button);
  GTK_WIDGET_SET_FLAGS (dialog->simulation_type_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->digital_verif_radio), "toggled",
                      GTK_SIGNAL_FUNC (on_digital_verif_radio_toggled),
                      dialog->simulation_type_options_button);
  gtk_signal_connect (GTK_OBJECT (dialog->vector_table_radio), "toggled",
                      GTK_SIGNAL_FUNC (on_vector_table_radio_toggled),
                      dialog->simulation_type_options_button);	

  gtk_signal_connect (GTK_OBJECT (dialog->simulation_type_options_button), "clicked",
                      GTK_SIGNAL_FUNC (on_sim_type_options_button_clicked),
                      dialog->simulation_type_dialog);
  gtk_signal_connect (GTK_OBJECT (dialog->simulation_type_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_sim_type_ok_button_clicked),
                      dialog->simulation_type_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->simulation_type_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->simulation_type_dialog));
					  
	// connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->simulation_type_dialog), "delete_event",
	      	      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->simulation_type_dialog));
}

void on_digital_verif_radio_toggled(GtkButton *button, gpointer user_data){
	GtkWidget *btnOpt = GTK_WIDGET (user_data) ;
	gtk_widget_set_sensitive (btnOpt, FALSE) ;
}

void on_vector_table_radio_toggled(GtkButton *button, gpointer user_data){
	GtkWidget *btnOpt = GTK_WIDGET (user_data) ;
	gtk_widget_set_sensitive (btnOpt, TRUE) ;
}

void on_sim_type_options_button_clicked(GtkButton *button, gpointer user_data){
  sim_type_setup_D *dialog = (sim_type_setup_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  get_vector_table_options_from_user (GTK_WINDOW (dialog->simulation_type_dialog),
    (VectorTable *)gtk_object_get_data (GTK_OBJECT (user_data), "pvt")) ;
  DBG_STS (VectorTable_dump ((VectorTable *)gtk_object_get_data (GTK_OBJECT (user_data), "pvt"), stderr)) ;
}

void on_sim_type_ok_button_clicked (GtkButton *widget, gpointer user_data)
  {
  sim_type_setup_D *dialog = (sim_type_setup_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  *(int *)gtk_object_get_data (GTK_OBJECT (user_data), "piSimType") =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->digital_verif_radio)) ? 
      EXHAUSTIVE_VERIFICATION : VECTOR_TABLE ;
  gtk_widget_hide(GTK_WIDGET(dialog->simulation_type_dialog));
  }
