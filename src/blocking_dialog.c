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
