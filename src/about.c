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
// File Description:                                    //
// This source handles the small about window that pops //
// up when the program is started                       //
//////////////////////////////////////////////////////////


#include <gtk/gtk.h>
#include <support.h>
#include "about.h"
#include "blocking_dialog.h"

typedef struct
  {
  GtkWidget *about_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *pixmap1;
  GtkWidget *about_label;
  GtkWidget *dialog_action_area1;
  GtkWidget *about_ok_button;
  } about_D ;

static about_D about_dialog = {NULL} ;

void create_about_dialog (about_D *about_dialog) ;
int hide_about_dialog (gpointer data) ;

void show_about_dialog (GtkWindow *parent, gboolean bSplash)
  {
  if (NULL == about_dialog.about_dialog)
    create_about_dialog (&about_dialog) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (about_dialog.about_dialog), parent) ;
  
  if (bSplash)
    {
    gtk_window_set_modal (GTK_WINDOW (about_dialog.about_dialog), FALSE) ;
    gtk_widget_hide (about_dialog.dialog_action_area1) ;
    gtk_widget_show (about_dialog.about_dialog);
    gtk_timeout_add (1000, (GtkFunction)hide_about_dialog, about_dialog.about_dialog) ;
    }
  else
    {
    gtk_window_set_modal (GTK_WINDOW (about_dialog.about_dialog), TRUE) ;
    show_dialog_blocking (about_dialog.about_dialog) ;
    }
  }

// -- code for the little about window that pops up each time qcadesigner is started -- //

void create_about_dialog (about_D *about_dialog){
	
	about_dialog->about_dialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (about_dialog->about_dialog), "about_dialog", about_dialog->about_dialog);
	gtk_window_set_title (GTK_WINDOW (about_dialog->about_dialog), _("About QCADesigner"));
	gtk_window_set_policy (GTK_WINDOW (about_dialog->about_dialog), TRUE, TRUE, FALSE);
	
	about_dialog->dialog_vbox1 = GTK_DIALOG (about_dialog->about_dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (about_dialog->about_dialog), "dialog_vbox1", about_dialog->dialog_vbox1);
	gtk_widget_show (about_dialog->dialog_vbox1);
	
	about_dialog->vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (about_dialog->vbox1);
	gtk_object_set_data_full (GTK_OBJECT (about_dialog->about_dialog), "vbox1", about_dialog->vbox1,
							(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (about_dialog->vbox1);
	gtk_box_pack_start (GTK_BOX (about_dialog->dialog_vbox1), about_dialog->vbox1, TRUE, TRUE, 0);
	
	about_dialog->pixmap1 = create_pixmap (about_dialog->about_dialog, "about.xpm");
	gtk_widget_ref (about_dialog->pixmap1);
	gtk_object_set_data_full (GTK_OBJECT (about_dialog->about_dialog), "pixmap1", about_dialog->pixmap1,
							(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (about_dialog->pixmap1);
	gtk_box_pack_start (GTK_BOX (about_dialog->vbox1), about_dialog->pixmap1, TRUE, TRUE, 0);
	
	about_dialog->about_label = gtk_label_new (_(
"QCADesigner (c) 2002 Version 1.20\nProtected by Copright 2002 K. Walus\n"
"Contributers:\n"
"G. Schulhof, T. Dysart, A. Vetteth\n"
"J. Eskritt, G.A. Jullien, V.S. Dimitrov\n"
"Download Free Version of QCADesigner from\n"
"http://www.atips.ca/projects/qcadesigner/"));
	gtk_widget_ref (about_dialog->about_label);
	gtk_object_set_data_full (GTK_OBJECT (about_dialog->about_dialog), "about_label", about_dialog->about_label,
							(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (about_dialog->about_label);
	gtk_box_pack_start (GTK_BOX (about_dialog->vbox1), about_dialog->about_label, FALSE, FALSE, 0);
	
	about_dialog->dialog_action_area1 = GTK_DIALOG (about_dialog->about_dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (about_dialog->about_dialog), "dialog_action_area1", about_dialog->dialog_action_area1);
	gtk_widget_show (about_dialog->dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (about_dialog->dialog_action_area1), 2);
	
	about_dialog->about_ok_button = gtk_button_new_with_label (_("OK"));
	gtk_widget_ref (about_dialog->about_ok_button);
	gtk_object_set_data_full (GTK_OBJECT (about_dialog->about_dialog), "about_ok_button", about_dialog->about_ok_button,
							(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (about_dialog->about_ok_button);
	gtk_box_pack_start (GTK_BOX (about_dialog->dialog_action_area1), about_dialog->about_ok_button, FALSE, TRUE, 0);
	
	gtk_signal_connect_object (GTK_OBJECT (about_dialog->about_dialog), "delete_event", 
	                           GTK_SIGNAL_FUNC (gtk_widget_hide),
				   GTK_OBJECT (about_dialog->about_dialog));
	
	gtk_signal_connect_object (GTK_OBJECT (about_dialog->about_ok_button), "clicked", 
				   GTK_SIGNAL_FUNC (gtk_widget_hide),
				   GTK_OBJECT (about_dialog->about_dialog));
}

int hide_about_dialog (gpointer data)
  {
  gtk_widget_hide (GTK_WIDGET (data)) ;
  gtk_widget_show (lookup_widget (GTK_WIDGET (data), "dialog_action_area1")) ;
  return FALSE ;
  }
