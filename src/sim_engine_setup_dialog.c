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

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <string.h>
#include <stdlib.h>

//#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "support.h"
#include "blocking_dialog.h"
#include "sim_engine_setup_dialog.h"
#include "bistable_properties_dialog.h"
#include "nonlinear_approx_properties_dialog.h"

typedef struct{
  	GtkWidget *sim_engine_setup_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *vbox1;
  	GSList *vbox1_group;
  	GtkWidget *mean_field_radio;
  	GtkWidget *bistable_radio;
	GtkWidget *digital_radio;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox1;
  	GtkWidget *sim_engine_options_button;
  	GtkWidget *sim_engine_ok_button;
  	GtkWidget *sim_engine_cancel_button;
}sim_engine_setup_D;

extern bistable_OP bistable_options ;
extern nonlinear_approx_OP nonlinear_approx_options ;
static sim_engine_setup_D sim_engine_setup_dialog = {NULL} ;

int get_sim_engine (sim_engine_setup_D *dialog) ;
void create_sim_engine_dialog(sim_engine_setup_D *dialog);
void on_sim_engine_options_button_clicked(GtkButton *button, gpointer user_data);
void on_sim_engine_ok_button_clicked(GtkWidget *button,gpointer user_data);

void get_sim_engine_from_user (GtkWindow *parent, int *piSimEng)
  {
  if (NULL == sim_engine_setup_dialog.sim_engine_setup_dialog)
    create_sim_engine_dialog (&sim_engine_setup_dialog) ;
  gtk_window_set_transient_for (GTK_WINDOW (sim_engine_setup_dialog.sim_engine_setup_dialog), parent) ;
  
  if (NONLINEAR_APPROXIMATION == *piSimEng)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.mean_field_radio), TRUE) ;
  else if (BISTABLE == *piSimEng)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.bistable_radio), TRUE) ;
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.digital_radio), TRUE) ;

  gtk_object_set_data (GTK_OBJECT (sim_engine_setup_dialog.sim_engine_setup_dialog), "piSimEng", piSimEng) ;
  gtk_object_set_data (GTK_OBJECT (sim_engine_setup_dialog.sim_engine_setup_dialog), "dialog", &sim_engine_setup_dialog) ;
  
  show_dialog_blocking (sim_engine_setup_dialog.sim_engine_setup_dialog) ;
  }

void create_sim_engine_dialog (sim_engine_setup_D *dialog){

  if (NULL != dialog->sim_engine_setup_dialog) return ;
  
  dialog->sim_engine_setup_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->sim_engine_setup_dialog), "sim_engine_setup_dialog", dialog->sim_engine_setup_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->sim_engine_setup_dialog), "Set Simulation Engine");
  gtk_window_set_policy (GTK_WINDOW (dialog->sim_engine_setup_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->sim_engine_setup_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->sim_engine_setup_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->sim_engine_setup_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->vbox1 = gtk_table_new (3, 1, FALSE);
  gtk_widget_ref (dialog->vbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "vbox1", dialog->vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->vbox1, TRUE, TRUE, 0);

  dialog->bistable_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, "Bistable Approximation");
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->bistable_radio));
  gtk_widget_ref (dialog->bistable_radio);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "bistable_radio", dialog->bistable_radio,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->bistable_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->bistable_radio, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  
  dialog->mean_field_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, "Nonlinear Approximation");
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->mean_field_radio));
  gtk_widget_ref (dialog->mean_field_radio);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "mean_field_radio", dialog->mean_field_radio,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->mean_field_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->mean_field_radio, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  // ** Attempting to add button for digital simulation **/
  dialog->digital_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, "Digital Simulation");
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->digital_radio));
  gtk_widget_ref (dialog->digital_radio);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "digital_radio", dialog->digital_radio,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->digital_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->digital_radio, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->sim_engine_setup_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->sim_engine_setup_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox1), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox1), 0, 0);

  dialog->sim_engine_options_button = gtk_button_new_with_label ("Options");
  gtk_widget_ref (dialog->sim_engine_options_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "sim_engine_options_button", dialog->sim_engine_options_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->sim_engine_options_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->sim_engine_options_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->sim_engine_options_button, GTK_CAN_DEFAULT);

  dialog->sim_engine_ok_button = gtk_button_new_with_label ("OK");
  gtk_widget_ref (dialog->sim_engine_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "sim_engine_ok_button", dialog->sim_engine_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->sim_engine_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->sim_engine_ok_button);
  GTK_WIDGET_SET_FLAGS (dialog->sim_engine_ok_button, GTK_CAN_DEFAULT);

  dialog->sim_engine_cancel_button = gtk_button_new_with_label ("Cancel");
  gtk_widget_ref (dialog->sim_engine_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->sim_engine_setup_dialog), "sim_engine_cancel_button", dialog->sim_engine_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->sim_engine_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->sim_engine_cancel_button);
  GTK_WIDGET_SET_FLAGS (dialog->sim_engine_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->sim_engine_options_button), "clicked",
                      GTK_SIGNAL_FUNC (on_sim_engine_options_button_clicked),
                      dialog->sim_engine_setup_dialog);
  gtk_signal_connect (GTK_OBJECT (dialog->sim_engine_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_sim_engine_ok_button_clicked),
                      dialog->sim_engine_setup_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->sim_engine_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->sim_engine_setup_dialog));

  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->sim_engine_setup_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->sim_engine_setup_dialog));
}

void on_sim_engine_options_button_clicked(GtkButton *button, gpointer user_data){
  sim_engine_setup_D *dialog = (sim_engine_setup_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;

  switch(get_sim_engine (dialog)){

    case NONLINEAR_APPROXIMATION:
      get_nonlinear_approx_properties_from_user (GTK_WINDOW (dialog->sim_engine_setup_dialog), &nonlinear_approx_options) ;
      break;

    case BISTABLE:
      get_bistable_properties_from_user (GTK_WINDOW (dialog->sim_engine_setup_dialog), &bistable_options) ;
      break ;

  case DIGITAL_SIM:
    break;

  }//switch
}

int get_sim_engine (sim_engine_setup_D *dialog)
  {
  return
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->mean_field_radio)) ? NONLINEAR_APPROXIMATION :
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->bistable_radio))   ? BISTABLE : DIGITAL_SIM ;
  }

void on_sim_engine_ok_button_clicked(GtkWidget *widget, gpointer user_data)
  {
  sim_engine_setup_D *dialog = (sim_engine_setup_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;

  *(int *)gtk_object_get_data (GTK_OBJECT (user_data), "piSimEng") = get_sim_engine (dialog) ;
  gtk_widget_hide(GTK_WIDGET(sim_engine_setup_dialog.sim_engine_setup_dialog));
  }
