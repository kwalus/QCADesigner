#include <stdarg.h>
#include <string.h>
#include "command_history.h"

static GtkWidget *tvHistory ;
static GtkWidget *swHistory ;

GtkWidget *command_history_create ()
  {
  swHistory = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (swHistory) ;
  tvHistory = gtk_text_new (NULL, NULL) ;
  gtk_widget_show (tvHistory) ;
  gtk_container_add (GTK_CONTAINER (swHistory), tvHistory) ;
  gtk_text_set_editable (GTK_TEXT (tvHistory), FALSE) ;
  
  return swHistory ;
  }

void command_history_message (char *pszFmt, ...)
  {
  va_list va ;
  gchar *pszMsg = NULL ;
  
  va_start (va, pszFmt) ;
  pszMsg = g_strdup_vprintf (pszFmt, va) ;
  va_end (va) ;
  
  gtk_text_insert (GTK_TEXT (tvHistory), NULL, NULL, NULL, pszMsg, strlen (pszMsg)) ;
  }
