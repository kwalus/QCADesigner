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

static void create_cell_properties_dialog (cell_properties_D *dialog);
static void mnuoptDefaultClock_activate (GtkWidget *widget, gpointer data) ;

void get_cell_properties_from_user (GtkWindow *parent, cell_OP *pco)
  {
  GtkWidget *menu_item = NULL ;
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
  
  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (cell_properties.cell_properties_dialog)))
    {
    pco->type_1_cell_height = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t1_cell_height_entry)));
    pco->type_1_cell_width = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t1_cell_width_entry)));
    pco->type_1_dot_diameter = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t1_dot_diameter_entry)));

    pco->type_2_cell_height = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t2_cell_height_entry)));
    pco->type_2_cell_width = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t2_cell_width_entry)));
    pco->type_2_dot_diameter = atof(gtk_entry_get_text(GTK_ENTRY (cell_properties.cell_properties_t2_dot_diameter_entry)));

    menu_item = gtk_menu_get_active (GTK_MENU (cell_properties.mnuoptDefaultClock)) ;

    pco->default_clock = 
      menu_item == cell_properties.mnuoptDefaultClock0 ? 0 :
      menu_item == cell_properties.mnuoptDefaultClock1 ? 1 :
      menu_item == cell_properties.mnuoptDefaultClock2 ? 2 : 3 ;
    }
  gtk_widget_hide (cell_properties.cell_properties_dialog) ;
  }

static void create_cell_properties_dialog(cell_properties_D *dialog){
  if (NULL != dialog->cell_properties_dialog) return ;
  
  dialog->cell_properties_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->cell_properties_dialog), "Cell Properties");
  gtk_window_set_policy (GTK_WINDOW (dialog->cell_properties_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->cell_properties_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->cell_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);
  
  dialog->notebook1 = gtk_notebook_new ();
  gtk_widget_show (dialog->notebook1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->notebook1), 2);
  
  dialog->table1 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (dialog->table1);
  gtk_container_add (GTK_CONTAINER (dialog->notebook1), dialog->table1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table1), 2);

  dialog->t1_dot_diameter_label = gtk_label_new (_("Dot Diameter [nm]:)"));
  gtk_widget_show (dialog->t1_dot_diameter_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_dot_diameter_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_dot_diameter_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_dot_diameter_label), 1, 0.5);

  dialog->t1_cell_height_label = gtk_label_new (_("Cell Height [nm]:"));
  gtk_widget_show (dialog->t1_cell_height_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_cell_height_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_cell_height_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_cell_height_label), 1, 0.5);

  dialog->t1_cell_width_label = gtk_label_new (_("Cell Width [nm]:"));
  gtk_widget_show (dialog->t1_cell_width_label);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->t1_cell_width_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t1_cell_width_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t1_cell_width_label), 1, 0.5);

  dialog->cell_properties_t1_dot_diameter_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t1_dot_diameter_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_dot_diameter_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t1_dot_diameter_entry), TRUE) ;

  dialog->cell_properties_t1_cell_height_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t1_cell_height_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_cell_height_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t1_cell_height_entry), TRUE) ;

  dialog->cell_properties_t1_cell_width_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t1_cell_width_entry);
  gtk_table_attach (GTK_TABLE (dialog->table1), dialog->cell_properties_t1_cell_width_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t1_cell_width_entry), TRUE) ;

  dialog->lblTab0 = gtk_label_new (_("Type 1"));
  gtk_widget_show (dialog->lblTab0);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook1), 0), dialog->lblTab0);

  dialog->table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (dialog->table2);
  gtk_container_add (GTK_CONTAINER (dialog->notebook1), dialog->table2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table2), 2);

  dialog->t2_dot_diameter_label = gtk_label_new (_("Dot Diameter [nm]:"));
  gtk_widget_show (dialog->t2_dot_diameter_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_dot_diameter_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_dot_diameter_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_dot_diameter_label), 1, 0.5);

  dialog->t2_cell_height_label = gtk_label_new (_("Cell Height [nm]:"));
  gtk_widget_show (dialog->t2_cell_height_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_cell_height_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_cell_height_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_cell_height_label), 1, 0.5);

  dialog->t2_cell_width_label = gtk_label_new (_("Cell Width [nm]:"));
  gtk_widget_show (dialog->t2_cell_width_label);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->t2_cell_width_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->t2_cell_width_label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->t2_cell_width_label), 1, 0.5);

  dialog->cell_properties_t2_dot_diameter_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t2_dot_diameter_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_dot_diameter_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t2_dot_diameter_entry), TRUE) ;

  dialog->cell_properties_t2_cell_height_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t2_cell_height_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_cell_height_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t2_cell_height_entry), TRUE) ;

  dialog->cell_properties_t2_cell_width_entry = gtk_entry_new ();
  gtk_widget_show (dialog->cell_properties_t2_cell_width_entry);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->cell_properties_t2_cell_width_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->cell_properties_t2_cell_width_entry), TRUE) ;

  dialog->lblTab1 = gtk_label_new (_("Type 2"));
  gtk_widget_show (dialog->lblTab1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook1), 1), dialog->lblTab1);

  dialog->tblDefaultClock = gtk_table_new (1, 2, FALSE);
  gtk_widget_show (dialog->tblDefaultClock);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblDefaultClock, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblDefaultClock), 2);
  
  dialog->lblDefaultClock = gtk_label_new (_("Default Clock:")) ;
  gtk_widget_show (dialog->lblDefaultClock) ;
  gtk_table_attach (GTK_TABLE (dialog->tblDefaultClock), dialog->lblDefaultClock, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblDefaultClock), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblDefaultClock), 1, 0.5);
  
  dialog->optDefaultClock = gtk_option_menu_new () ;
  g_object_set_data (G_OBJECT (dialog->cell_properties_dialog), "optDefaultClock", dialog->optDefaultClock) ;
  gtk_widget_show (dialog->optDefaultClock) ;
  gtk_table_attach (GTK_TABLE (dialog->tblDefaultClock), dialog->optDefaultClock, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0) ;
  GTK_WIDGET_SET_FLAGS (dialog->optDefaultClock, GTK_CAN_FOCUS) ;

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
  
  gtk_dialog_add_button (GTK_DIALOG (dialog->cell_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  dialog->cell_properties_ok_button = 
    gtk_dialog_add_button (GTK_DIALOG (dialog->cell_properties_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->cell_properties_dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->mnuoptDefaultClock0), "activate",
    G_CALLBACK (mnuoptDefaultClock_activate), dialog->cell_properties_dialog) ;
  g_signal_connect (G_OBJECT (dialog->mnuoptDefaultClock1), "activate",
    G_CALLBACK (mnuoptDefaultClock_activate), dialog->cell_properties_dialog) ;
  g_signal_connect (G_OBJECT (dialog->mnuoptDefaultClock2), "activate",
    G_CALLBACK (mnuoptDefaultClock_activate), dialog->cell_properties_dialog) ;
  g_signal_connect (G_OBJECT (dialog->mnuoptDefaultClock3), "activate",
    G_CALLBACK (mnuoptDefaultClock_activate), dialog->cell_properties_dialog) ;
}

static void mnuoptDefaultClock_activate (GtkWidget *widget, gpointer data)
  {
  GtkWidget *default_widget = GTK_WINDOW (data)->default_widget ;
  GtkWidget *optDefaultClock = GTK_WIDGET (g_object_get_data (G_OBJECT (data), "optDefaultClock")) ;
  
  gtk_widget_grab_focus (optDefaultClock) ;
  gtk_widget_grab_default (default_widget) ;
  gtk_widget_queue_draw (GTK_WIDGET (data)) ;
  }
