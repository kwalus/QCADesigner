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
#include "nonlinear_approx_properties_dialog.h"

typedef struct{
	GtkWidget *nonlinear_approx_properties_dialog;
  	GtkWidget *dialog_vbox1;
	GtkWidget *label_epsilonR;
	GtkWidget *label_clock_high;
	GtkWidget *label_clock_low;
	GtkWidget *label_samples;
  	GtkWidget *epsilonR_dialog_entry;
	GtkWidget *clock_high_dialog_entry;
	GtkWidget *clock_low_dialog_entry;
	GtkWidget *samples_dialog_entry;
  	GtkWidget *dialog_action_area1;
  	GtkWidget *hbox_buttons;
  	GtkWidget *nonlinear_approx_properties_dialog_ok_button;
  	GtkWidget *nonlinear_approx_properties_dialog_cancel_button;
	GtkWidget *chkAnimate ;
}nonlinear_approx_properties_D;

nonlinear_approx_properties_D nonlinear_approx_properties = {NULL} ;

static void create_nonlinear_approx_properties_dialog (nonlinear_approx_properties_D *dialog) ;

void get_nonlinear_approx_properties_from_user (GtkWindow *parent, nonlinear_approx_OP *pnao)
  {
  char text[16] = "" ;
  if(nonlinear_approx_properties.nonlinear_approx_properties_dialog == NULL)
    create_nonlinear_approx_properties_dialog(&nonlinear_approx_properties);
  gtk_window_set_transient_for (GTK_WINDOW (nonlinear_approx_properties.nonlinear_approx_properties_dialog), parent) ;
  
  g_snprintf(text, 16, "%4.4f", pnao->epsilonR);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.epsilonR_dialog_entry), text);
  g_snprintf(text, 16, "%e", pnao->clock_high);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.clock_high_dialog_entry), text);
  g_snprintf(text, 16, "%e", pnao->clock_low);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.clock_low_dialog_entry), text);
  g_snprintf(text, 16, "%d", pnao->number_of_samples);
  gtk_entry_set_text(GTK_ENTRY (nonlinear_approx_properties.samples_dialog_entry), text);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nonlinear_approx_properties.chkAnimate), pnao->animate_simulation) ;
  
  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (nonlinear_approx_properties.nonlinear_approx_properties_dialog)))
    {
    pnao->epsilonR = atof(gtk_entry_get_text(GTK_ENTRY (nonlinear_approx_properties.epsilonR_dialog_entry)));
    pnao->clock_high = atof(gtk_entry_get_text(GTK_ENTRY (nonlinear_approx_properties.clock_high_dialog_entry)));
	pnao->clock_low = atof(gtk_entry_get_text(GTK_ENTRY (nonlinear_approx_properties.clock_low_dialog_entry)));
    pnao->animate_simulation = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (nonlinear_approx_properties.chkAnimate)) ;
    pnao->number_of_samples = atoi (gtk_entry_get_text (GTK_ENTRY (nonlinear_approx_properties.samples_dialog_entry))) ;
    }
  gtk_widget_hide (nonlinear_approx_properties.nonlinear_approx_properties_dialog) ;
  }

static void create_nonlinear_approx_properties_dialog (nonlinear_approx_properties_D *dialog)
{

GtkWidget *table = NULL ;

if (NULL != dialog->nonlinear_approx_properties_dialog) return ;
    
// -- create the dialog window -- //
    
  dialog->nonlinear_approx_properties_dialog= gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), _("Nonlinear Approximation Properties"));
  gtk_window_set_policy (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), TRUE, TRUE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->nonlinear_approx_properties_dialog), TRUE) ;

// -- create and add the vertical box -- //

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->nonlinear_approx_properties_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);
  
  table = gtk_table_new (4, 2, FALSE) ;
  gtk_widget_show (table) ;
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), table, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (table), 2) ;

// -- create and add the samples value label -- //

  dialog->label_samples = gtk_label_new (_("Number Of Samples:"));
  gtk_widget_show (dialog->label_samples);
  gtk_table_attach (GTK_TABLE (table), dialog->label_samples, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_samples), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_samples), 1, 0.5);


// -- create and add the Relative Permittivity value label -- //

  dialog->label_epsilonR = gtk_label_new (_("Relative Premittivity:"));
  gtk_widget_show (dialog->label_epsilonR);
  gtk_table_attach (GTK_TABLE (table), dialog->label_epsilonR, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_epsilonR), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_epsilonR), 1, 0.5);
  
// -- create and add the clock high value label -- //

  dialog->label_clock_high = gtk_label_new (_("Clock High:"));
  gtk_widget_show (dialog->label_clock_high);
  gtk_table_attach (GTK_TABLE (table), dialog->label_clock_high, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_clock_high), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_clock_high), 1, 0.5);
  
// -- create and add the clock low value label -- //

  dialog->label_clock_low = gtk_label_new (_("Clock Low:"));
  gtk_widget_show (dialog->label_clock_low);
  gtk_table_attach (GTK_TABLE (table), dialog->label_clock_low, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label_clock_low), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label_clock_low), 1, 0.5);

// -- create and add the samples value entry box -- //

  dialog->samples_dialog_entry = gtk_entry_new ();
  gtk_widget_show (dialog->samples_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->samples_dialog_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->samples_dialog_entry), TRUE) ;

// -- create and add the Relative Permittivity value entry box -- //

  dialog->epsilonR_dialog_entry = gtk_entry_new ();
  gtk_widget_show (dialog->epsilonR_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->epsilonR_dialog_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->epsilonR_dialog_entry), TRUE) ;

// -- create and add the clock high value entry box -- //

  dialog->clock_high_dialog_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_high_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->clock_high_dialog_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_high_dialog_entry), TRUE) ;
  
// -- create and add the clock low value entry box -- //

  dialog->clock_low_dialog_entry = gtk_entry_new ();
  gtk_widget_show (dialog->clock_low_dialog_entry);
  gtk_table_attach (GTK_TABLE (table), dialog->clock_low_dialog_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->clock_low_dialog_entry), TRUE) ;

  dialog->chkAnimate = gtk_check_button_new_with_label ("Animate") ;
  gtk_widget_show (dialog->chkAnimate) ;
  gtk_table_attach (GTK_TABLE (table), dialog->chkAnimate, 0, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  gtk_dialog_add_button (GTK_DIALOG (dialog->nonlinear_approx_properties_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->nonlinear_approx_properties_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->nonlinear_approx_properties_dialog), GTK_RESPONSE_OK) ;
}
