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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "support.h"
#include "stdqcell.h"
#include "cad.h"
#include "message_box.h"
#include "clock_select_dialog.h"

typedef struct{
 	GtkWidget *clock_select_dialog;
  	GtkWidget *dialog_vbox1;
  	GtkWidget *vbox1;
  	GSList *vbox1_group;
  	GtkWidget *clock_radio0;
  	GtkWidget *clock_radio1;
  	GtkWidget *clock_radio2;
  	GtkWidget *clock_radio3;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox1;
  	GtkWidget *clock_select_ok_button;
  	GtkWidget *clock_select_cancel_button;
}clock_select_D;

static clock_select_D clock_select = {NULL} ;

void create_clock_select_dialog_priv (clock_select_D *dialog) ;
void on_clock_select_ok_button_clicked(GtkButton *button, gpointer user_data);

GtkWidget *create_clock_select_dialog (GtkWindow *parent)
  {
  if (NULL == clock_select.clock_select_dialog)
    create_clock_select_dialog_priv (&clock_select) ;
  if (NULL != parent)
    gtk_window_set_transient_for (GTK_WINDOW (clock_select.clock_select_dialog), parent) ;
  gtk_object_set_data (GTK_OBJECT (clock_select.clock_select_dialog), "dialog", &clock_select) ;
  return clock_select.clock_select_dialog ;
  }

void update_clock_select_dialog (int iClock)
  {
  if (NULL == clock_select.clock_select_dialog)
    create_clock_select_dialog (NULL) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (clock_select.clock_radio0), (iClock == 0)) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (clock_select.clock_radio1), (iClock == 1)) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (clock_select.clock_radio2), (iClock == 2)) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (clock_select.clock_radio3), (iClock == 3)) ;
  }

void create_clock_select_dialog_priv (clock_select_D *dialog){

  if (NULL != dialog->clock_select_dialog) return ;
 
  dialog->clock_select_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->clock_select_dialog), "clock_select_dialog", dialog->clock_select_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->clock_select_dialog), _("Select the clock"));
  gtk_window_set_policy (GTK_WINDOW (dialog->clock_select_dialog), FALSE, FALSE, FALSE);

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->clock_select_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->clock_select_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->vbox1 = gtk_table_new (4, 1, FALSE);
  gtk_widget_ref (dialog->vbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "vbox1", dialog->vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox1), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->vbox1, FALSE, FALSE, 0);

  dialog->clock_radio0 = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Clock 0"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->clock_radio0));
  gtk_widget_ref (dialog->clock_radio0);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_radio0", dialog->clock_radio0,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_radio0);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->clock_radio0, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->clock_radio1 = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Clock 1"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->clock_radio1));
  gtk_widget_ref (dialog->clock_radio1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_radio1", dialog->clock_radio1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_radio1);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->clock_radio1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->clock_radio2 = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Clock 2"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->clock_radio2));
  gtk_widget_ref (dialog->clock_radio2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_radio2", dialog->clock_radio2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_radio2);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->clock_radio2, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->clock_radio3 = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Clock 3"));
  dialog->vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->clock_radio3));
  gtk_widget_ref (dialog->clock_radio3);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_radio3", dialog->clock_radio3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_radio3);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->clock_radio3, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->clock_select_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->clock_select_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "hbox1", dialog->hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox1), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox1), 0, 0);

  dialog->clock_select_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->clock_select_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_select_ok_button", dialog->clock_select_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_select_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->clock_select_ok_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->clock_select_ok_button, GTK_CAN_DEFAULT);

  dialog->clock_select_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->clock_select_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->clock_select_dialog), "clock_select_cancel_button", dialog->clock_select_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->clock_select_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbox1), dialog->clock_select_cancel_button) ;
  GTK_WIDGET_SET_FLAGS (dialog->clock_select_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->clock_select_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_clock_select_ok_button_clicked),
                      dialog->clock_select_dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->clock_select_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->clock_select_dialog));
					  
  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->clock_select_dialog), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
		      GTK_OBJECT (dialog->clock_select_dialog));

}

void on_clock_select_ok_button_clicked(GtkButton *button, gpointer user_data)
  {
  clock_select_D *clock_select =
    (clock_select_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog");
  int selected_clock = 0;

  selected_clock = 
    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_select->clock_radio0))) ? 0 :
    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_select->clock_radio1))) ? 1 :
    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_select->clock_radio2))) ? 2 :
    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_select->clock_radio3))) ? 3 : -1 ;
    
  if (-1 == selected_clock)
    message_box (GTK_WINDOW (clock_select->clock_select_dialog), MB_OK, "Clock Error", "Please select an appropriate clock !") ;
  else
    set_clock_for_selected_cells(selected_clock);
  }
