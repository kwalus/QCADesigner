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
#include "grid_spacing_dialog.h"
#include "blocking_dialog.h"

typedef struct{
	gint INPUT;
	GtkWidget *grid_spacing_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *hbox1;
	GtkWidget *label1;
  	GtkWidget *grid_spacing_dialog_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox2;
  	GtkWidget *grid_spacing_dialog_ok_button;
  	GtkWidget *grid_spacing_dialog_cancel_button;
}grid_spacing_D;

//!Dialog for changing the grid spacing.
static grid_spacing_D spacing = {0, NULL};

void create_grid_spacing_dialog (grid_spacing_D *dialog);
void on_grid_spacing_dialog_ok_button_clicked (GtkButton *button, gpointer user_data);

void get_grid_spacing_from_user (GtkWindow *parent, double *pdGridSpacing)
  {
  char szText[16] = "" ;
  if (NULL == spacing.grid_spacing_dialog)
    create_grid_spacing_dialog (&spacing) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (spacing.grid_spacing_dialog), parent) ;
  
  g_snprintf (szText, 16, "%4.4f", *pdGridSpacing) ;
  gtk_entry_set_text (GTK_ENTRY (spacing.grid_spacing_dialog_entry), szText) ;
  gtk_object_set_data (GTK_OBJECT (spacing.grid_spacing_dialog), "pdGridSpacing", pdGridSpacing) ;
  gtk_object_set_data (GTK_OBJECT (spacing.grid_spacing_dialog), "dialog", &spacing) ;
  show_dialog_blocking (spacing.grid_spacing_dialog) ;
  }

void create_grid_spacing_dialog (grid_spacing_D *dialog)
{
  if (NULL != dialog->grid_spacing_dialog) return ;
  
  dialog->grid_spacing_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->grid_spacing_dialog), "grid_spacing_dialog", dialog->grid_spacing_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->grid_spacing_dialog), _("Enter Grid Spaing [nm]"));
  gtk_window_set_policy (GTK_WINDOW (dialog->grid_spacing_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->grid_spacing_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->grid_spacing_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->grid_spacing_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->hbox1 = gtk_table_new (1, 2, FALSE);
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->hbox1), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->hbox1, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Grid Spacing [nm]:"));
  gtk_widget_ref (dialog->label1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "label1", dialog->label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->grid_spacing_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->grid_spacing_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "grid_spacing_dialog_entry", dialog->grid_spacing_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->grid_spacing_dialog_entry);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->grid_spacing_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->grid_spacing_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->grid_spacing_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox2 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "hbox2", dialog->hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox2, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox2), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox2), 0, -1);

  dialog->grid_spacing_dialog_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->grid_spacing_dialog_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "grid_spacing_dialog_ok_button", dialog->grid_spacing_dialog_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->grid_spacing_dialog_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->grid_spacing_dialog_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->grid_spacing_dialog_ok_button, GTK_CAN_DEFAULT);

  dialog->grid_spacing_dialog_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->grid_spacing_dialog_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->grid_spacing_dialog), "grid_spacing_dialog_cancel_button", dialog->grid_spacing_dialog_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->grid_spacing_dialog_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->grid_spacing_dialog_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->grid_spacing_dialog_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->grid_spacing_dialog_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_grid_spacing_dialog_ok_button_clicked),
                      dialog->grid_spacing_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->grid_spacing_dialog_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->grid_spacing_dialog));

  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->grid_spacing_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->grid_spacing_dialog));

}

void on_grid_spacing_dialog_ok_button_clicked(GtkButton *button, gpointer user_data){
  grid_spacing_D *dialog =
    (grid_spacing_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  *(double *)(gtk_object_get_data (GTK_OBJECT (dialog->grid_spacing_dialog), "pdGridSpacing")) = 
    atof(gtk_entry_get_text(GTK_ENTRY (dialog->grid_spacing_dialog_entry)));
  gtk_widget_hide(GTK_WIDGET(user_data));
}
