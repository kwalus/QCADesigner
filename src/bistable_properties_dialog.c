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
#include "blocking_dialog.h"
#include "bistable_properties_dialog.h"

typedef struct{
	GtkWidget *bistable_properties_dialog;
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
  	GtkWidget *convergence_tolerance_entry;
  	GtkWidget *radius_of_effect_entry;
  	GtkWidget *K_entry;
  	GtkWidget *decay_exponent_entry;
  	GtkWidget *chkAnimate;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox2;
  	GtkWidget *bistable_properties_ok_button;
  	GtkWidget *bistable_properties_cancel_button;
}bistable_properties_D;

static bistable_properties_D bistable_properties = {NULL};

void create_bistable_properties_dialog (bistable_properties_D *dialog) ;
void on_bistable_properties_ok_button_clicked (GtkButton *button, gpointer user_data);

void get_bistable_properties_from_user (GtkWindow *parent, bistable_OP *pbo)
  {
  char sz[16] = "" ;
  if (NULL == bistable_properties.bistable_properties_dialog)
    create_bistable_properties_dialog (&bistable_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (bistable_properties.bistable_properties_dialog), parent) ;
  
  g_snprintf (sz, 16, "%d", pbo->number_of_samples) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.number_of_samples_entry), sz) ;

  g_snprintf (sz, 16, "%d", pbo->max_iterations_per_sample) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.max_iterations_per_sample_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bistable_properties.chkAnimate),
    pbo->animate_simulation) ;

  g_snprintf (sz, 16, "%f", pbo->convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->convergence_tolerance) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.convergence_tolerance_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->radius_of_effect) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.radius_of_effect_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->K) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.K_entry), sz) ;

  g_snprintf (sz, 16, "%f", pbo->decay_exponent) ;
  gtk_entry_set_text (GTK_ENTRY (bistable_properties.decay_exponent_entry), sz) ;
  
  gtk_object_set_data (GTK_OBJECT (bistable_properties.bistable_properties_dialog), "pbo", pbo) ;
  gtk_object_set_data (GTK_OBJECT (bistable_properties.bistable_properties_dialog), "dialog", &bistable_properties) ;
  
  show_dialog_blocking (bistable_properties.bistable_properties_dialog) ;
  }

void create_bistable_properties_dialog (bistable_properties_D *dialog){

  if (NULL != dialog->bistable_properties_dialog) return ;
    
  dialog->bistable_properties_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->bistable_properties_dialog), "bistable_properties_dialog", dialog->bistable_properties_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->bistable_properties_dialog), _("Bistable Options"));
  gtk_window_set_policy (GTK_WINDOW (dialog->bistable_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->bistable_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->bistable_properties_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->bistable_properties_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (7, 2, FALSE);
  gtk_widget_ref (dialog->table);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "table", dialog->table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Number of Samples:"));
  gtk_widget_ref (dialog->label1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "label1", dialog->label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->number_of_samples_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->number_of_samples_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "number_of_samples_entry", dialog->number_of_samples_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->number_of_samples_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->number_of_samples_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->label2 = gtk_label_new (_("Convergence Tolerance:"));
  gtk_widget_ref (dialog->label2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "label2", dialog->label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label2);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label2), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label2), 1, 0.5);

  dialog->convergence_tolerance_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->convergence_tolerance_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "convergence_tolerance_entry", dialog->convergence_tolerance_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->convergence_tolerance_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->convergence_tolerance_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->label3 = gtk_label_new (_("Radius of Effect [nm]:"));
  gtk_widget_ref (dialog->label3);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "label3", dialog->label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label3);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label3), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label3), 1, 0.5);

  dialog->radius_of_effect_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->radius_of_effect_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "radius_of_effect_entry", dialog->radius_of_effect_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->radius_of_effect_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->radius_of_effect_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->label4 = gtk_label_new (_("K:"));
  gtk_widget_ref (dialog->label4);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "label4", dialog->label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label4);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label4, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label4), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label4), 1, 0.5);

  dialog->K_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->K_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "K_entry", dialog->K_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->K_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->K_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->label5 = gtk_label_new (_("Decay Exponent:"));
  gtk_widget_ref (dialog->label5);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "label5", dialog->label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label5);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label5, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label5), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label5), 1, 0.5);

  dialog->decay_exponent_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->decay_exponent_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "decay_exponent_entry", dialog->decay_exponent_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->decay_exponent_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->decay_exponent_entry, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->lblMaxIter = gtk_label_new (_("Maximum Iterations Per Sample:"));
  gtk_widget_ref (dialog->lblMaxIter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "lblMaxIter", dialog->lblMaxIter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblMaxIter);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lblMaxIter, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblMaxIter), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblMaxIter), 1, 0.5);

  dialog->max_iterations_per_sample_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->max_iterations_per_sample_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "max_iterations_per_sample_entry", dialog->max_iterations_per_sample_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->max_iterations_per_sample_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->max_iterations_per_sample_entry, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->chkAnimate = gtk_check_button_new_with_label (_("Animate")) ;
  gtk_widget_ref (dialog->chkAnimate) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "chkAnimate", dialog->chkAnimate,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkAnimate, 0, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->bistable_properties_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->bistable_properties_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox2 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "hbox2", dialog->hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox2, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox2), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox2), 0, 0);

  dialog->bistable_properties_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->bistable_properties_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "bistable_properties_ok_button", dialog->bistable_properties_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->bistable_properties_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->bistable_properties_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->bistable_properties_ok_button, GTK_CAN_DEFAULT);

  dialog->bistable_properties_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->bistable_properties_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->bistable_properties_dialog), "bistable_properties_cancel_button", dialog->bistable_properties_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->bistable_properties_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->bistable_properties_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->bistable_properties_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->bistable_properties_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_bistable_properties_ok_button_clicked),
                      dialog->bistable_properties_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->bistable_properties_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->bistable_properties_dialog));

  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->bistable_properties_dialog), "delete_event",
      	      	      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->bistable_properties_dialog));
}

void on_bistable_properties_ok_button_clicked(GtkButton *button, gpointer user_data){
  bistable_properties_D *dialog =
    (bistable_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data),"dialog") ;
  bistable_OP *pbo = (bistable_OP *)gtk_object_get_data (GTK_OBJECT (dialog->bistable_properties_dialog), "pbo") ;
  
  pbo->max_iterations_per_sample = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->max_iterations_per_sample_entry))) ;
  pbo->number_of_samples = atoi (gtk_entry_get_text (GTK_ENTRY (dialog->number_of_samples_entry))) ;
  pbo->convergence_tolerance = atof (gtk_entry_get_text (GTK_ENTRY (dialog->convergence_tolerance_entry))) ;
  pbo->radius_of_effect = atof (gtk_entry_get_text (GTK_ENTRY (dialog->radius_of_effect_entry))) ;
  pbo->K = atof (gtk_entry_get_text (GTK_ENTRY (dialog->K_entry))) ;
  pbo->decay_exponent = atof (gtk_entry_get_text (GTK_ENTRY (dialog->decay_exponent_entry))) ;
  pbo->animate_simulation = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->chkAnimate)) ;
  
  gtk_widget_hide(GTK_WIDGET(dialog->bistable_properties_dialog));
}
