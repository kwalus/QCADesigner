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
#include "project_properties_dialog.h"

typedef struct{
	GtkWidget *project_properties_dialog;
  	GtkWidget *dialog_vbox1;
	GtkWidget *tblDim ;
	GtkWidget *label_width;
	GtkWidget *label_height;
  	GtkWidget *width_dialog_entry;
	GtkWidget *height_dialog_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox_buttons;
  	GtkWidget *project_properties_dialog_ok_button;
  	GtkWidget *project_properties_dialog_cancel_button;
}project_properties_D;

//!Dialog used to setup project properties.
static project_properties_D project_properties = {NULL} ;

void on_project_properties_dialog_ok_button_clicked (GtkButton *button, gpointer user_data);
void create_project_properties_dialog (project_properties_D *dialog);

void get_project_properties_from_user (GtkWindow *parent, double *pdSubsWidth, double *pdSubsHeight)
  {
  char szText[16] = "" ;
  
  if (NULL == project_properties.project_properties_dialog)
    create_project_properties_dialog (&project_properties) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (project_properties.project_properties_dialog), parent) ;
  
  g_snprintf (szText, 16, "%f", *pdSubsWidth) ;
  gtk_entry_set_text (GTK_ENTRY (project_properties.width_dialog_entry), szText) ;
  
  g_snprintf (szText, 16, "%f", *pdSubsHeight) ;
  gtk_entry_set_text (GTK_ENTRY (project_properties.height_dialog_entry), szText) ;
  
  gtk_object_set_data (GTK_OBJECT (project_properties.project_properties_dialog), "pdSubsWidth", pdSubsWidth) ;
  gtk_object_set_data (GTK_OBJECT (project_properties.project_properties_dialog), "pdSubsHeight", pdSubsHeight) ;
  gtk_object_set_data (GTK_OBJECT (project_properties.project_properties_dialog), "dialog", &project_properties) ;
  
  show_dialog_blocking (project_properties.project_properties_dialog) ;
  }

void create_project_properties_dialog (project_properties_D *dialog){
  if (NULL != dialog->project_properties_dialog) return ;
    
// -- create the dialog window -- //
    
  dialog->project_properties_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->project_properties_dialog), "project_properties_dialog", dialog->project_properties_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->project_properties_dialog), "Project Properties");
  gtk_window_set_policy (GTK_WINDOW (dialog->project_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->project_properties_dialog), TRUE) ;

// -- create and add the vertical box -- //

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->project_properties_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->project_properties_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

// -- create and add the table for the dimension widgets -- //

  dialog->tblDim = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (dialog->tblDim);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "tblDim", dialog->tblDim,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblDim);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblDim), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblDim, TRUE, TRUE, 0);

// -- create and add the die width value label -- //

  dialog->label_width = gtk_label_new (_("Die Width [nm]:"));
  gtk_widget_ref (dialog->label_width);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "label_width", dialog->label_width,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label_width);
  gtk_table_attach (GTK_TABLE (dialog->tblDim), dialog->label_width, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_width), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_width), 1, 0.5);

// -- create and add the die width value entry box -- //

  dialog->width_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->width_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "width_dialog_entry", dialog->width_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->width_dialog_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblDim), dialog->width_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

// -- create and add the height value label -- //

  dialog->label_height = gtk_label_new (_("Die Height [nm]:"));
  gtk_widget_ref (dialog->label_height);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "label_height", dialog->label_height,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label_height);
  gtk_table_attach (GTK_TABLE (dialog->tblDim), dialog->label_height, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_height), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_height), 1, 0.5);


// -- create and add the height value entry box -- //

  dialog->height_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->height_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "height_dialog_entry", dialog->height_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->height_dialog_entry);
  gtk_table_attach (GTK_TABLE (dialog->tblDim), dialog->height_dialog_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

// -- create and add the action area -- //

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->project_properties_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->project_properties_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

// -- Create and add the hbox for the buttons -- //

  dialog->hbox_buttons = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox_buttons);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "hbox_buttons", dialog->hbox_buttons,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox_buttons);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox_buttons, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox_buttons), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox_buttons), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox_buttons), 0, 0);

  dialog->project_properties_dialog_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->project_properties_dialog_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "project_properties_dialog_ok_button", dialog->project_properties_dialog_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->project_properties_dialog_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox_buttons), dialog->project_properties_dialog_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->project_properties_dialog_ok_button, GTK_CAN_DEFAULT);

  dialog->project_properties_dialog_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->project_properties_dialog_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->project_properties_dialog), "project_properties_dialog_cancel_button", dialog->project_properties_dialog_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->project_properties_dialog_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox_buttons), dialog->project_properties_dialog_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->project_properties_dialog_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->project_properties_dialog_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_project_properties_dialog_ok_button_clicked),
		      GTK_OBJECT (dialog->project_properties_dialog));
  gtk_signal_connect_object (GTK_OBJECT (dialog->project_properties_dialog_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->project_properties_dialog));
	
  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->project_properties_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->project_properties_dialog));
}

void on_project_properties_dialog_ok_button_clicked(GtkButton *button, gpointer user_data)
  {
  project_properties_D *dialog =
    (project_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  *((double *)(gtk_object_get_data (GTK_OBJECT (user_data), "pdSubsWidth"))) =
    atof (gtk_entry_get_text (GTK_ENTRY (dialog->width_dialog_entry))) ;
  *((double *)(gtk_object_get_data (GTK_OBJECT (user_data), "pdSubsHeight"))) =
    atof (gtk_entry_get_text (GTK_ENTRY (dialog->height_dialog_entry))) ;
  gtk_widget_hide(GTK_WIDGET(user_data));
  }
