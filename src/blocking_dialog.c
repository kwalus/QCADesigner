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
// This file was contributed by Gabriel Schulhof        //
// (schulhof@vlsi.enel.ucalgary.ca).  It is a small     //
// utility to allow the blocking display of a modal di- //
// alog box.  This way, the retrieval of data from the  //
// user can be abstracted away to the point where cer-  //
// tain functions "magically" fill out data structures, //
// whether via a dialog box or some other mechanism,    //
// such as a database.  The important aspect is that    //
// the function does not return until the data has been //
// filled out.  This blocking behaviour is accomplished //
// by taking over the main GTK event loop, and relea-   //
// sing it only when the dialog box is hidden.          //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <gtk/gtk.h>
#include "blocking_dialog.h"

void dialog_is_being_hidden (GtkWidget *dialog, gpointer data) ;

void show_dialog_blocking (GtkWidget *dialog)
  {
  gboolean bKeepBlocking = TRUE ;
  gtk_signal_connect (GTK_OBJECT (dialog), "hide", GTK_SIGNAL_FUNC (dialog_is_being_hidden), &bKeepBlocking) ;
  gtk_widget_show (dialog) ;
  while (bKeepBlocking)
    gtk_main_iteration_do (TRUE) ;
  }

void dialog_is_being_hidden (GtkWidget *dialog, gpointer data)
  {
  gtk_signal_disconnect_by_func (GTK_OBJECT (dialog), GTK_SIGNAL_FUNC (dialog_is_being_hidden), data) ;
  *(gboolean *)data = FALSE ;
  }
