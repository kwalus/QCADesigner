//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// **** Please use complete names in variables and      //
// **** functions. This will reduce ramp up time for new//
// **** people trying to contribute to the project.     //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@vlsi.enel.ucalgary.edu).  It is a rudimen- //
// tary message box implementation.                     //
//////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "message_box.h"
#include "blocking_dialog.h"

typedef struct
  {
  GtkWidget *message_box ;
  GtkWidget *lblMsg ;
  GtkWidget *btnOK ;
  GtkWidget *btnYes ;
  GtkWidget *btnNo ;
  GtkWidget *btnCancel ;
  } MESSAGE_BOX ;

static MESSAGE_BOX the_message_box = {NULL} ;

void message_box_button_clicked (GtkWidget *widget, gpointer user_data) ;
void message_box_close (GtkWidget *widget, gpointer user_data) ;
void create_message_box (MESSAGE_BOX *pmb) ;

MBButton message_box (GtkWindow *parent, MBButton btns, char *pszTitle, char *pszFormat, ...)
  {
  va_list va ;
  char *pszMsg = NULL ;
  MBButton ret = MB_OK ;

  create_message_box (&the_message_box) ;
  gtk_object_set_data (GTK_OBJECT (the_message_box.message_box), "dialog", &(the_message_box)) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (the_message_box.message_box), parent) ;
  gtk_window_set_title (GTK_WINDOW (the_message_box.message_box), pszTitle) ;
  
  va_start (va, pszFormat) ;
  gtk_label_set_text (GTK_LABEL (the_message_box.lblMsg), pszMsg = g_strdup_vprintf (pszFormat, va)) ;
  va_end (va) ;
  g_free (pszMsg) ;
  
  gtk_object_set_data (GTK_OBJECT (the_message_box.message_box), "ret", &ret) ;
  
  if (!btns) btns = MB_OK ;
  
  if (btns & MB_OK)
    gtk_widget_show (the_message_box.btnOK) ;
  else
    gtk_widget_hide (the_message_box.btnOK) ;
  
  if (btns & MB_YES)
    gtk_widget_show (the_message_box.btnYes) ;
  else
    gtk_widget_hide (the_message_box.btnYes) ;
  
  if (btns & MB_NO)
    gtk_widget_show (the_message_box.btnNo) ;
  else
    gtk_widget_hide (the_message_box.btnNo) ;
  
  if (btns & MB_CANCEL)
    gtk_widget_show (the_message_box.btnCancel) ;
  else
    gtk_widget_hide (the_message_box.btnCancel) ;
  
  show_dialog_blocking (the_message_box.message_box) ;
  
  return ret ;
  }

void create_message_box (MESSAGE_BOX *pmb)
  {
  GtkWidget *vb, *tbl, *daa, *hbb ;
  
  if (NULL != pmb->message_box) return ;

  pmb->message_box = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (pmb->message_box), "message_box", pmb->message_box);
  gtk_window_set_title (GTK_WINDOW (pmb->message_box), "Message");
  gtk_window_set_policy (GTK_WINDOW (pmb->message_box), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (pmb->message_box), TRUE) ;

  vb = GTK_DIALOG (pmb->message_box)->vbox;
  gtk_object_set_data (GTK_OBJECT (pmb->message_box), "vb", vb);
  gtk_widget_show (vb);

  tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_ref (tbl) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (vb), tbl, FALSE, FALSE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;
  
  pmb->lblMsg = gtk_label_new ("The Message.") ;
  gtk_widget_ref (pmb->lblMsg) ;
  gtk_widget_show (pmb->lblMsg) ;
  gtk_table_attach (GTK_TABLE (tbl), pmb->lblMsg, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (pmb->lblMsg), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (pmb->lblMsg), TRUE);
  gtk_misc_set_alignment (GTK_MISC (pmb->lblMsg), 0, 0);

  daa = GTK_DIALOG (pmb->message_box)->action_area;
  gtk_object_set_data (GTK_OBJECT (pmb->message_box), "daa", pmb->message_box);
  gtk_widget_show (daa);
  gtk_container_set_border_width (GTK_CONTAINER (daa), 0);

  hbb = gtk_hbutton_box_new () ;
  gtk_widget_ref (hbb) ;
  gtk_widget_show (hbb) ;
  gtk_box_pack_start (GTK_BOX (daa), hbb, TRUE, TRUE, 0) ;
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbb), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbb), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbb), 0, 0);
  
  pmb->btnOK = gtk_button_new_with_label ("OK") ;
  gtk_widget_ref (pmb->btnOK) ;
  gtk_widget_show (pmb->btnOK) ;
  gtk_container_add (GTK_CONTAINER (hbb), pmb->btnOK) ;
  GTK_WIDGET_SET_FLAGS (pmb->btnOK, GTK_CAN_DEFAULT);
  gtk_object_set_data (GTK_OBJECT (pmb->btnOK), "result", (gpointer)MB_OK) ;
  
  pmb->btnYes = gtk_button_new_with_label ("Yes") ;
  gtk_widget_ref (pmb->btnYes) ;
  gtk_widget_show (pmb->btnYes) ;
  gtk_container_add (GTK_CONTAINER (hbb), pmb->btnYes) ;
  GTK_WIDGET_SET_FLAGS (pmb->btnYes, GTK_CAN_DEFAULT);
  gtk_object_set_data (GTK_OBJECT (pmb->btnYes), "result", (gpointer)MB_YES) ;
  
  pmb->btnNo = gtk_button_new_with_label ("No") ;
  gtk_widget_ref (pmb->btnNo) ;
  gtk_widget_show (pmb->btnNo) ;
  gtk_container_add (GTK_CONTAINER (hbb), pmb->btnNo) ;
  GTK_WIDGET_SET_FLAGS (pmb->btnNo, GTK_CAN_DEFAULT);
  gtk_object_set_data (GTK_OBJECT (pmb->btnNo), "result", (gpointer)MB_NO) ;
  
  pmb->btnCancel = gtk_button_new_with_label ("Cancel") ;
  gtk_widget_ref (pmb->btnCancel) ;
  gtk_widget_show (pmb->btnCancel) ;
  gtk_container_add (GTK_CONTAINER (hbb), pmb->btnCancel) ;
  GTK_WIDGET_SET_FLAGS (pmb->btnCancel, GTK_CAN_DEFAULT);
  gtk_object_set_data (GTK_OBJECT (pmb->btnCancel), "result", (gpointer)MB_CANCEL) ;
  
  gtk_signal_connect (GTK_OBJECT (pmb->btnOK), "clicked", message_box_button_clicked, pmb->message_box) ;
  gtk_signal_connect (GTK_OBJECT (pmb->btnYes), "clicked", message_box_button_clicked, pmb->message_box) ;
  gtk_signal_connect (GTK_OBJECT (pmb->btnNo), "clicked", message_box_button_clicked, pmb->message_box) ;
  gtk_signal_connect (GTK_OBJECT (pmb->btnCancel), "clicked", message_box_button_clicked, pmb->message_box) ;
  gtk_signal_connect (GTK_OBJECT (pmb->message_box), "delete_event", message_box_close, pmb->message_box) ;
  }

void message_box_close (GtkWidget *widget, gpointer user_data)
  {
  MESSAGE_BOX *dialog = (MESSAGE_BOX *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  (*(MBButton *)gtk_object_get_data (GTK_OBJECT (user_data), "ret")) =
    GTK_WIDGET_VISIBLE (dialog->btnCancel) ? MB_CANCEL :
    GTK_WIDGET_VISIBLE (dialog->btnNo) ? MB_NO :
    GTK_WIDGET_VISIBLE (dialog->btnYes) ? MB_YES : MB_OK ;
  gtk_widget_hide (GTK_WIDGET (user_data)) ;
  }
  
void message_box_button_clicked (GtkWidget *widget, gpointer user_data)
  {
  (*(MBButton *)gtk_object_get_data (GTK_OBJECT (user_data), "ret")) =
    (MBButton)gtk_object_get_data (GTK_OBJECT (widget), "result") ;
  gtk_widget_hide (GTK_WIDGET (user_data)) ;
  }
