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


#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "blocking_dialog.h"
#include "nonlinear_approx_properties_dialog.h"

typedef struct{
	GtkWidget *nonlinear_approx_properties_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *hbox_k;
	GtkWidget *hbox_exp;
	GtkWidget *label_k;
	GtkWidget *label_exp;
  	GtkWidget *k_dialog_entry;
	GtkWidget *exp_dialog_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox_buttons;
  	GtkWidget *nonlinear_approx_properties_dialog_ok_button;
  	GtkWidget *nonlinear_approx_properties_dialog_cancel_button;
	GtkWidget *chkAnimate ;
}nonlinear_approx_properties_D;

nonlinear_approx_properties_D nonlinear_approx_properties = {NULL} ;

void on_nonlinear_approx_properties_dialog_ok_button_clicked (GtkButton *button, gpointer user_data);
void create_nonlinear_approx_properties_dialog (nonlinear_approx_properties_D *dialog) ;

void get_nonlinear_approx_properties_from_user (GtkWindow *parent, nonlinear_approx_OP *pnao)
  {
  char text[16] = "" ;
  if(nonlinear_approx_properties.nonlinear_approx_properties_dialog == NULL)
    create_nonlinear_approx_properties_dialog(&nonlinear_approx_properties);
  gtk_window_set_transient_for (GTK_WINDOW (nonlinear_approx_properties.nonlinear_approx_properties_dialog), parent) ;
  
  g_snprintf(text, 16, "%4.4f", pnao->K);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.k_dialog_entry), text);
  g_snprintf(text, 16, "%4.4f", pnao->decay_exponent);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.exp_dialog_entry), text);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nonlinear_approx_properties.chkAnimate), pnao->animate_simulation) ;
  
  gtk_object_set_data (GTK_OBJECT (nonlinear_approx_properties.nonlinear_approx_properties_dialog), "pnao", pnao) ;
  gtk_object_set_data (GTK_OBJECT (nonlinear_approx_properties.nonlinear_approx_properties_dialog), "dialog", &nonlinear_approx_properties) ;
  
  show_dialog_blocking (nonlinear_approx_properties.nonlinear_approx_properties_dialog) ;
  }

void create_nonlinear_approx_properties_dialog (nonlinear_approx_properties_D *dialog)
{

GtkWidget *table = NULL ;

if (NULL != dialog->nonlinear_approx_properties_dialog) return ;
    
// -- create the dialog window -- //
    
  dialog->nonlinear_approx_properties_dialog= gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "nonlinear_approx_properties_dialog", dialog->nonlinear_approx_properties_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), _("Nonlinear Approximation Properties"));
  gtk_window_set_policy (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), TRUE, TRUE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), TRUE) ;

// -- create and add the vertical box -- //

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->nonlinear_approx_properties_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);
  
  table = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_ref (table) ;
  gtk_object_set_data (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "table", table);
  gtk_widget_show (table) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), table, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (table), 2) ;

// -- create and add the K value label -- //

  dialog->label_k = gtk_label_new (_("K:"));
  gtk_widget_ref (dialog->label_k);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "label_k", dialog->label_k,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label_k);
  gtk_table_attach (GTK_TABLE (table), dialog->label_k, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_k), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_k), 1, 0.5);

// -- create and add the K value entry box -- //

  dialog->k_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->k_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "k_dialog_entry", dialog->k_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->k_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->k_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

// -- create and add the exp value label -- //

  dialog->label_exp = gtk_label_new (_("Decay Exponent:"));
  gtk_widget_ref (dialog->label_exp);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "label_exp", dialog->label_exp,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label_exp);
  gtk_table_attach (GTK_TABLE (table), dialog->label_exp, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_exp), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_exp), 1, 0.5);

// -- create and add the exp value entry box -- //

  dialog->exp_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->exp_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "exp_dialog_entry", dialog->exp_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->exp_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->exp_dialog_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->chkAnimate = gtk_check_button_new_with_label ("Animate") ;
  gtk_widget_ref (dialog->chkAnimate) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "chkAnimate", dialog->chkAnimate,
                            (GtkDestroyNotify)gtk_widget_unref) ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (table), dialog->chkAnimate, 0, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

// -- create and add the action area -- //

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->nonlinear_approx_properties_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

// -- Create and add the hbox for the buttons -- //

  dialog->hbox_buttons = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox_buttons);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "hbox_buttons", dialog->hbox_buttons,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox_buttons);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox_buttons, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox_buttons), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox_buttons), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox_buttons), 0, 0);

  dialog->nonlinear_approx_properties_dialog_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->nonlinear_approx_properties_dialog_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "nonlinear_approx_properties_dialog_ok_button", dialog->nonlinear_approx_properties_dialog_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->nonlinear_approx_properties_dialog_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox_buttons), dialog->nonlinear_approx_properties_dialog_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->nonlinear_approx_properties_dialog_ok_button, GTK_CAN_DEFAULT);

  dialog->nonlinear_approx_properties_dialog_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->nonlinear_approx_properties_dialog_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "nonlinear_approx_properties_dialog_cancel_button", dialog->nonlinear_approx_properties_dialog_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->nonlinear_approx_properties_dialog_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox_buttons), dialog->nonlinear_approx_properties_dialog_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->nonlinear_approx_properties_dialog_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_nonlinear_approx_properties_dialog_ok_button_clicked),
                      dialog->nonlinear_approx_properties_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->nonlinear_approx_properties_dialog));
					  
  // connect the delete_event function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->nonlinear_approx_properties_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->nonlinear_approx_properties_dialog));
}

void on_nonlinear_approx_properties_dialog_ok_button_clicked(GtkButton *button, gpointer user_data)
  {
  nonlinear_approx_properties_D *properties =
    (nonlinear_approx_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog");
  nonlinear_approx_OP *pnao = (nonlinear_approx_OP *)gtk_object_get_data (GTK_OBJECT (user_data), "pnao") ;
	
  pnao->K = atof(gtk_entry_get_text(GTK_ENTRY (properties->k_dialog_entry)));
  pnao->decay_exponent = atof(gtk_entry_get_text(GTK_ENTRY (properties->exp_dialog_entry)));
  pnao->animate_simulation = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (properties->chkAnimate)) ;
  gtk_widget_hide(GTK_WIDGET(properties->nonlinear_approx_properties_dialog));
  }
