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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "support.h"
#include "file_selection_window.h"
#include "fileio_helpers.h"
#include "custom_widgets.h"

#define DBG_FSW(s)

static gboolean filesel_ok_button_activate (GtkWidget *widget, GdkEventButton *ev, gpointer data) ;

static GtkWidget *file_selection = NULL ;

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bOverwritePrompt)
  {
  gchar *pszRet = NULL ;
  gulong handlerID = -1 ;

  if (NULL == file_selection)
    file_selection = gtk_file_selection_new (pszWinTitle) ;

  gtk_window_set_transient_for (GTK_WINDOW (file_selection), parent) ;
  gtk_window_set_title (GTK_WINDOW (file_selection), pszWinTitle) ;
  if (NULL != pszFName)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selection), pszFName) ;
  
  if (bOverwritePrompt)
    handlerID = g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (file_selection)->ok_button),
      "button_release_event", G_CALLBACK (filesel_ok_button_activate), file_selection) ;

  if ((GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (file_selection))))
    pszRet = g_strdup_printf ("%s", gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selection))) ;
  gtk_widget_hide (file_selection) ;
  
  if (bOverwritePrompt)
    g_signal_handler_disconnect (G_OBJECT (GTK_FILE_SELECTION (file_selection)->ok_button), handlerID) ;

  return pszRet ;
  }

static gboolean filesel_ok_button_activate (GtkWidget *widget, GdkEventButton *ev, gpointer data)
  {
  char *pszFName = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (data))) ;
  
  if (g_file_test (pszFName, G_FILE_TEST_EXISTS))
    {
    int resp = GTK_RESPONSE_CANCEL ;
    GtkWidget *msg = gtk_message_dialog_new (GTK_WINDOW (data), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "A file named \"%s\" already exists.\nDo you want to replace it with the one you are saving ?", pszFName) ;
    
    gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
    gtk_dialog_add_action_widget (GTK_DIALOG (msg), gtk_button_new_with_stock_image (GTK_STOCK_REFRESH, "Replace"), GTK_RESPONSE_OK) ;
    resp = gtk_dialog_run (GTK_DIALOG (msg)) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    
    if (GTK_RESPONSE_OK != resp)
      gtk_button_released (GTK_BUTTON (widget)) ;
    else
      gtk_button_clicked (GTK_BUTTON (widget)) ;
    g_free (pszFName) ;
    return (GTK_RESPONSE_OK != resp) ;
    }
  
  g_free (pszFName) ;
  return FALSE ;
  }
