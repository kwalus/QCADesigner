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

static void create_grid_spacing_dialog (grid_spacing_D *dialog);

void get_grid_spacing_from_user (GtkWindow *parent, double *pdGridSpacing)
  {
  char szText[16] = "" ;
  if (NULL == spacing.grid_spacing_dialog)
    create_grid_spacing_dialog (&spacing) ;
  
  gtk_widget_grab_focus (spacing.grid_spacing_dialog_entry) ;
  gtk_window_set_transient_for (GTK_WINDOW (spacing.grid_spacing_dialog), parent) ;
  
  g_snprintf (szText, 16, "%4.8f", *pdGridSpacing) ;
  gtk_entry_set_text (GTK_ENTRY (spacing.grid_spacing_dialog_entry), szText) ;
  
  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (spacing.grid_spacing_dialog)))
    {
    if (NULL != pdGridSpacing)
     *pdGridSpacing = atof(gtk_entry_get_text(GTK_ENTRY (spacing.grid_spacing_dialog_entry))) ;
    }
  gtk_widget_hide (spacing.grid_spacing_dialog) ;
  }

static void create_grid_spacing_dialog (grid_spacing_D *dialog)
{
  if (NULL != dialog->grid_spacing_dialog) return ;
  
  dialog->grid_spacing_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->grid_spacing_dialog), _("Enter Grid Spaing [nm]"));
  gtk_window_set_policy (GTK_WINDOW (dialog->grid_spacing_dialog), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->grid_spacing_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->grid_spacing_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->hbox1 = gtk_table_new (1, 2, FALSE);
  gtk_widget_show (dialog->hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->hbox1), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->hbox1, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Grid Spacing [nm]:"));
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->grid_spacing_dialog_entry = gtk_entry_new ();
  gtk_widget_show (dialog->grid_spacing_dialog_entry);
  gtk_table_attach (GTK_TABLE (dialog->hbox1), dialog->grid_spacing_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->grid_spacing_dialog_entry), TRUE) ;
  
  gtk_dialog_add_button (GTK_DIALOG (dialog->grid_spacing_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->grid_spacing_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->grid_spacing_dialog), GTK_RESPONSE_OK) ;
}
