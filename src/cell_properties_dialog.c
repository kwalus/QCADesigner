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

#include "stdqcell.h"
#include "support.h"
#include "blocking_dialog.h"
#include "cell_properties_dialog.h"

typedef struct{
  GtkWidget *cell_properties_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *notebook1 ;
  GtkWidget *table1;
  GtkWidget *table2;
  GtkWidget *t1_dot_diameter_label;
  GtkWidget *t1_cell_height_label;
  GtkWidget *t1_cell_width_label;
  GtkWidget *t2_dot_diameter_label;
  GtkWidget *t2_cell_height_label;
  GtkWidget *t2_cell_width_label;
  GtkWidget *cell_properties_t1_dot_diameter_entry;
  GtkWidget *cell_properties_t1_cell_height_entry;
  GtkWidget *cell_properties_t1_cell_width_entry;
  GtkWidget *cell_properties_t2_dot_diameter_entry;
  GtkWidget *cell_properties_t2_cell_height_entry;
  GtkWidget *cell_properties_t2_cell_width_entry;
  GtkWidget *lblTab0;
  GtkWidget *lblTab1;
  GtkWidget *hbuttonbox1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cell_properties_ok_button;
  GtkWidget *cell_properties_cancel_button;
  GtkWidget *lblDefaultClock ;
  GtkWidget *tblDefaultClock ;
  GtkWidget *optDefaultClock ;
  GtkWidget *mnuoptDefaultClock ;
  GtkWidget *mnuoptDefaultClock0 ;
  GtkWidget *mnuoptDefaultClock1 ;
  GtkWidget *mnuoptDefaultClock2 ;
  GtkWidget *mnuoptDefaultClock3 ;
}cell_properties_D;

//!Dialog used to setup cell properties.
static cell_properties_D cell_properties = {NULL} ;

void on_cell_properties_ok_button_clicked(GtkButton *button, gpointer user_data);
void create_cell_properties_dialog (cell_properties_D *dialog);

void get_cell_properties_from_user (GtkWindow *parent, cell_OP *pco)
  {
  char szText[16] = "" ;
  if (NULL == cell_properties.cell_properties_dialog)
    create_cell_properties_dialog (&cell_properties) ;

  gtk_window_set_transient_for (GTK_WINDOW (cell_properties.cell_properties_dialog), parent) ;
  
  g_snprintf(szText, 16, "%.2f", pco->type_1_cell_height);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t1_cell_height_entry), szText);

  g_snprintf(szText, 16, "%.2f", pco->type_1_cell_width);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t1_cell_width_entry), szText);

  g_snprintf(szText, 16, "%.2f", pco->type_1_dot_diameter);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t1_dot_diameter_entry), szText);

  g_snprintf(szText, 16, "%.2f", pco->type_2_cell_height);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t2_cell_height_entry), szText);

  g_snprintf(szText, 16, "%.2f", pco->type_2_cell_width);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t2_cell_width_entry), szText);

  g_snprintf(szText, 16, "%.2f", pco->type_2_dot_diameter);
  gtk_entry_set_text(GTK_ENTRY (cell_properties.cell_properties_t2_dot_diameter_entry), szText);
  
  gtk_menu_set_active (GTK_MENU (cell_properties.mnuoptDefaultClock), pco->default_clock) ;
  
  gtk_object_set_data (GTK_OBJECT (cell_properties.cell_properties_dialog), "pco", pco) ;
  gtk_object_set_data (GTK_OBJECT (cell_properties.cell_properties_dialog), "dialog", &cell_properties) ;
  
  show_dialog_blocking (cell_properties.cell_properties_dialog) ;
  }

void create_cell_properties_dialog(cell_properties_D *dialog){
  if (NULL != dialog->cell_properties_dialog) return ;
  
  dialog->cell_properties_dialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_dialog", dialog->cell_properties_dialog);
  gtk_window_set_title (GTK_WINDOW (dialog->cell_properties_dialog), "Cell Properties");
  gtk_window_set_policy (GTK_WINDOW (dialog->cell_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->cell_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->cell_properties_dialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->cell_properties_dialog), "dialog_vbox1", dialog->dialog_vbox1);
  gtk_widget_show (dialog->dialog_vbox1);
  
  dialog->notebook1 = gtk_notebook_new ();
  gtk_widget_ref (dialog->notebook1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "notebook1", dialog->notebook1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->notebook1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->notebook1), 2);
  
  dialog->table1 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (dialog->table1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "table1", dialog->table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table1);
  gtk_container_add (GTK_CONTAINER (dialog->notebook1), dialog->table1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table1), 2);

  dialog->t1_dot_diameter_label = gtk_label_new (_("Dot Diameter [nm]:)"));
  gtk_widget_ref (dialog->t1_dot_diameter_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t1_dot_diameter_label", dialog->t1_dot_diameter_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t1_dot_diameter_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_dot_diameter_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_dot_diameter_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_dot_diameter_label), 1, 0.5);

  dialog->t1_cell_height_label = gtk_label_new (_("Cell Height [nm]:"));
  gtk_widget_ref (dialog->t1_cell_height_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t1_cell_height_label", dialog->t1_cell_height_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t1_cell_height_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_cell_height_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_cell_height_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_cell_height_label), 1, 0.5);

  dialog->t1_cell_width_label = gtk_label_new (_("Cell Width [nm]:"));
  gtk_widget_ref (dialog->t1_cell_width_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t1_cell_width_label", dialog->t1_cell_width_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t1_cell_width_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_cell_width_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_cell_width_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_cell_width_label), 1, 0.5);

  dialog->cell_properties_t1_dot_diameter_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t1_dot_diameter_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t1_dot_diameter_entry", dialog->cell_properties_t1_dot_diameter_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t1_dot_diameter_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_dot_diameter_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->cell_properties_t1_cell_height_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t1_cell_height_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t1_cell_height_entry", dialog->cell_properties_t1_cell_height_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t1_cell_height_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_cell_height_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->cell_properties_t1_cell_width_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t1_cell_width_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t1_cell_width_entry", dialog->cell_properties_t1_cell_width_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t1_cell_width_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_cell_width_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblTab0 = gtk_label_new (_("Type 1"));
  gtk_widget_ref (dialog->lblTab0);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "lblTab0", dialog->lblTab0,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblTab0);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook1), 0), dialog->lblTab0);

  dialog->table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (dialog->table2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "table2", dialog->table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table2);
  gtk_container_add (GTK_CONTAINER (dialog->notebook1), dialog->table2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table2), 2);

  dialog->t2_dot_diameter_label = gtk_label_new (_("Dot Diameter [nm]:"));
  gtk_widget_ref (dialog->t2_dot_diameter_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t2_dot_diameter_label", dialog->t2_dot_diameter_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t2_dot_diameter_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_dot_diameter_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_dot_diameter_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_dot_diameter_label), 1, 0.5);

  dialog->t2_cell_height_label = gtk_label_new (_("Cell Height [nm]:"));
  gtk_widget_ref (dialog->t2_cell_height_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t2_cell_height_label", dialog->t2_cell_height_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t2_cell_height_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_cell_height_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_cell_height_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_cell_height_label), 1, 0.5);

  dialog->t2_cell_width_label = gtk_label_new (_("Cell Width [nm]:"));
  gtk_widget_ref (dialog->t2_cell_width_label);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "t2_cell_width_label", dialog->t2_cell_width_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->t2_cell_width_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_cell_width_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_cell_width_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_cell_width_label), 1, 0.5);

  dialog->cell_properties_t2_dot_diameter_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t2_dot_diameter_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t2_dot_diameter_entry", dialog->cell_properties_t2_dot_diameter_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t2_dot_diameter_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_dot_diameter_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->cell_properties_t2_cell_height_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t2_cell_height_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t2_cell_height_entry", dialog->cell_properties_t2_cell_height_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t2_cell_height_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_cell_height_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->cell_properties_t2_cell_width_entry = gtk_entry_new ();
  gtk_widget_ref (dialog->cell_properties_t2_cell_width_entry);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_t2_cell_width_entry", dialog->cell_properties_t2_cell_width_entry,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_t2_cell_width_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_cell_width_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblTab1 = gtk_label_new (_("Type 2"));
  gtk_widget_ref (dialog->lblTab1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "lblTab1", dialog->lblTab1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblTab1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook1), 1), dialog->lblTab1);

  dialog->tblDefaultClock = gtk_table_new (1, 2, FALSE);
  gtk_widget_ref (dialog->tblDefaultClock);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "tblDefaultClock", dialog->tblDefaultClock,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblDefaultClock);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblDefaultClock, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblDefaultClock), 2);
  
  dialog->lblDefaultClock = gtk_label_new (_("Default Clock:")) ;
  gtk_widget_ref (dialog->lblDefaultClock) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "lblDefaultClock", dialog->lblDefaultClock,
      	      	      	    (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->lblDefaultClock) ;
  gtk_table_attach (GTK_TABLE (dialog->tblDefaultClock), dialog->lblDefaultClock, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblDefaultClock), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblDefaultClock), 1, 0.5);
  
  dialog->optDefaultClock = gtk_option_menu_new () ;
  gtk_widget_ref (dialog->optDefaultClock) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "optDefaultClock", dialog->optDefaultClock,
      	      	      	    (GtkDestroyNotify)gtk_widget_unref) ;
  gtk_widget_show (dialog->optDefaultClock) ;
  gtk_table_attach (GTK_TABLE (dialog->tblDefaultClock), dialog->optDefaultClock, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0) ;
  dialog->mnuoptDefaultClock = gtk_menu_new () ;
  gtk_widget_show (dialog->mnuoptDefaultClock0 = gtk_menu_item_new_with_label (_("Clock 0"))) ;
  gtk_menu_append (GTK_MENU (dialog->mnuoptDefaultClock), dialog->mnuoptDefaultClock0) ;
  gtk_widget_show (dialog->mnuoptDefaultClock1 = gtk_menu_item_new_with_label (_("Clock 1"))) ;
  gtk_menu_append (GTK_MENU (dialog->mnuoptDefaultClock), dialog->mnuoptDefaultClock1) ;
  gtk_widget_show (dialog->mnuoptDefaultClock2 = gtk_menu_item_new_with_label (_("Clock 2"))) ;
  gtk_menu_append (GTK_MENU (dialog->mnuoptDefaultClock), dialog->mnuoptDefaultClock2) ;
  gtk_widget_show (dialog->mnuoptDefaultClock3 = gtk_menu_item_new_with_label (_("Clock 3"))) ;
  gtk_menu_append (GTK_MENU (dialog->mnuoptDefaultClock), dialog->mnuoptDefaultClock3) ;
  gtk_option_menu_set_menu (GTK_OPTION_MENU (dialog->optDefaultClock), dialog->mnuoptDefaultClock) ;
  
  dialog->dialog_action_area1 = GTK_DIALOG (dialog->cell_properties_dialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->cell_properties_dialog), "dialog_action_area1", dialog->dialog_action_area1);
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);

  dialog->hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "hbuttonbox1", dialog->hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbuttonbox1, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbuttonbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbuttonbox1), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbuttonbox1), 0, -1);

  dialog->cell_properties_ok_button = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->cell_properties_ok_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_ok_button", dialog->cell_properties_ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_ok_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbuttonbox1), dialog->cell_properties_ok_button);
  GTK_WIDGET_SET_FLAGS (dialog->cell_properties_ok_button, GTK_CAN_DEFAULT);

  dialog->cell_properties_cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->cell_properties_cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (dialog->cell_properties_dialog), "cell_properties_cancel_button", dialog->cell_properties_cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cell_properties_cancel_button);
  gtk_container_add (GTK_CONTAINER (dialog->hbuttonbox1), dialog->cell_properties_cancel_button);
  GTK_WIDGET_SET_FLAGS (dialog->cell_properties_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (dialog->cell_properties_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_cell_properties_ok_button_clicked),
                      dialog->cell_properties_dialog);
  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->cell_properties_dialog), "delete_event",
	              GTK_SIGNAL_FUNC (gtk_widget_hide), 
		      GTK_OBJECT (dialog->cell_properties_dialog));
  gtk_signal_connect_object (GTK_OBJECT (dialog->cell_properties_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_hide),
                      GTK_OBJECT (dialog->cell_properties_dialog));
}

void on_cell_properties_ok_button_clicked(GtkButton *button, gpointer user_data)
  {
  GtkWidget *menu_item = NULL ;
  cell_properties_D *dialog = 
    (cell_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  cell_OP *pco = (cell_OP *)gtk_object_get_data (GTK_OBJECT (dialog->cell_properties_dialog), "pco") ;

  pco->type_1_cell_height = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t1_cell_height_entry)));
  pco->type_1_cell_width = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t1_cell_width_entry)));
  pco->type_1_dot_diameter = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t1_dot_diameter_entry)));

  pco->type_2_cell_height = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t2_cell_height_entry)));
  pco->type_2_cell_width = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t2_cell_width_entry)));
  pco->type_2_dot_diameter = atof(gtk_entry_get_text(GTK_ENTRY (dialog->cell_properties_t2_dot_diameter_entry)));
  
  menu_item = gtk_menu_get_active (GTK_MENU (dialog->mnuoptDefaultClock)) ;
  
  pco->default_clock = 
    menu_item == dialog->mnuoptDefaultClock0 ? 0 :
    menu_item == dialog->mnuoptDefaultClock1 ? 1 :
    menu_item == dialog->mnuoptDefaultClock2 ? 2 : 3 ;

  gtk_widget_hide(GTK_WIDGET(user_data));
  }
