#include <stdarg.h>
#include <string.h>
#include "command_history.h"

static GtkWidget *tvHistory ;
static GtkWidget *swHistory ;

GtkWidget *command_history_create ()
  {
  swHistory = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (swHistory) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swHistory), GTK_SHADOW_IN) ;
  tvHistory = gtk_text_view_new () ;
  gtk_widget_show (tvHistory) ;
  gtk_container_add (GTK_CONTAINER (swHistory), tvHistory) ;
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tvHistory), FALSE) ;
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (tvHistory), FALSE);  
  return swHistory ;
  }

void command_history_message (char *pszFmt, ...)
  {
  va_list va ;
  gchar *pszMsg = NULL ;
  GtkTextBuffer *ptbCH = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tvHistory)) ;
  GtkTextIter gtiEnd ;
  GtkTextMark *markEnd ;
  
  va_start (va, pszFmt) ;
  pszMsg = g_strdup_vprintf (pszFmt, va) ;
  va_end (va) ;
  
  gtk_text_buffer_get_end_iter (ptbCH, &gtiEnd) ;
  gtk_text_buffer_insert (ptbCH, &gtiEnd, pszMsg, strlen (pszMsg)) ;
  markEnd = gtk_text_buffer_create_mark (ptbCH, NULL, &gtiEnd, FALSE) ;
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (tvHistory), markEnd) ;
  gtk_text_buffer_delete_mark (ptbCH, markEnd) ;
  
  while (gtk_events_pending ())
    gtk_main_iteration () ;
  }
