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
#include "name_dialog.h"

typedef struct{
	GtkWidget *name_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *hbox1;
	GtkWidget *label1;
  	GtkWidget *name_dialog_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox2;
  	GtkWidget *name_dialog_ok_button;
  	GtkWidget *name_dialog_cancel_button;
}name_D;

static name_D name = {NULL} ;

void create_name_dialog (name_D *dialog);
void on_name_dialog_ok_button_clicked (GtkButton *button, gpointer user_data);

gboolean get_name_from_user (GtkWindow *parent, char *pszName, int cb)
  {
  gboolean bOK = FALSE ;
  if (NULL == name.name_dialog)
    create_name_dialog (&name) ;
  gtk_window_set_transient_for (GTK_WINDOW (name.name_dialog), parent) ;
  gtk_entry_set_text (GTK_ENTRY (name.name_dialog_entry), pszName) ;
  gtk_object_set_data (GTK_OBJECT (name.name_dialog), "pszName", pszName) ;
  gtk_object_set_data (GTK_OBJECT (name.name_dialog), "pcb", &cb) ;
  gtk_object_set_data (GTK_OBJECT (name.name_dialog), "pbOK", &bOK) ;
  
  show_dialog_blocking (name.name_dialog) ;
  
  return bOK ;
  }

void create_name_dialog (name_D *dialog){

  if (NULL != dialog->name_dialog) return ;
    
  dialog->name_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->name_dialog), "name_dialog", dialog->name_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->name_dialog), _("Enter Name"));
  gtk_window_set_policy (GTK_WINDOW (dialog->name_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->name_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->name_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->name_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->hbox1 = gtk_table_new (1, 2, FALSE);
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->hbox1), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->hbox1, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Name:"));
  gtk_widget_ref (dialog->label1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "label1", dialog->label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->name_dialog_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->name_dialog_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "name_dialog_entry", dialog->name_dialog_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->name_dialog_entry);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->name_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->name_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->name_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox2 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "hbox2", dialog->hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox2, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox2), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox2), 0, 0);

  dialog->name_dialog_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->name_dialog_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "name_dialog_ok_button", dialog->name_dialog_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->name_dialog_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->name_dialog_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->name_dialog_ok_button, GTK_CAN_DEFAULT);

  dialog->name_dialog_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->name_dialog_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->name_dialog), "name_dialog_cancel_button", dialog->name_dialog_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->name_dialog_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->name_dialog_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->name_dialog_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->name_dialog_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_name_dialog_ok_button_clicked),
                      dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->name_dialog_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->name_dialog));

  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->name_dialog), "delete_event",
      	      	      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->name_dialog));

  
}

void on_name_dialog_ok_button_clicked(GtkButton *button, gpointer user_data){
  name_D *dialog = (name_D *)user_data ;
  char *pszName = (char *)gtk_object_get_data (GTK_OBJECT (dialog->name_dialog), "pszName") ;
  int *pcb = (int *)gtk_object_get_data (GTK_OBJECT (dialog->name_dialog), "pcb") ;
  gboolean *pbOK = (gboolean *)gtk_object_get_data (GTK_OBJECT (dialog->name_dialog), "pbOK") ;

  g_snprintf (pszName, *pcb, "%s", gtk_entry_get_text (GTK_ENTRY (dialog->name_dialog_entry))) ;
  *pbOK = TRUE ;
  gtk_widget_hide (GTK_WIDGET (name.name_dialog)) ;
}
