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
// (schulhof@vlsi.enel.ucalgary.ca).  It is a place to  //
// store widgets that are too complex to be maintained  //
// within the various user interface elements and have  //
// a high chance of being reused.                       //
//////////////////////////////////////////////////////////

// GTK includes //
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "support.h"
#include "custom_widgets.h"

static void switch_pix (GtkWidget *tbtn, gpointer user_data) ;

/* Creates a button whose label is a pixmap */
GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle)
  {
  GtkWidget *btn ;
  GtkWidget *vbox1, *label1 ;
  
  btn = bToggle ? gtk_toggle_button_new () : gtk_button_new () ;
  gtk_widget_ref (btn) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "btn", btn, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (btn) ;
  
  vbox1 = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_ref (vbox1) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1) ;
  gtk_container_add (GTK_CONTAINER (btn), vbox1) ;

  gtk_object_set_data_full (GTK_OBJECT (btn), "pixmap", pixmap, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmap) ;
  gtk_table_attach (GTK_TABLE (vbox1), pixmap, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;
  
  if (NULL != text)
    {
    label1 = gtk_label_new (text) ;
    gtk_widget_ref (label1) ;
    gtk_object_set_data_full (GTK_OBJECT (btn), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1) ;
    gtk_table_attach (GTK_TABLE (vbox1), label1, 1, 2, 0, 1,
      (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
      (GtkAttachOptions)(0), 2, 2) ;
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT) ;
    gtk_misc_set_alignment (GTK_MISC (label1), 0.0, 0.5) ;
    }
  return btn ;
  }

void pixmap_button_toggle_pixmap (GtkWidget *btn, gboolean bShow)
  {
  GtkWidget *pixmap = NULL ;
  if (NULL != (pixmap = gtk_object_get_data (GTK_OBJECT (btn), "pixmap")))
    {
    if (bShow)
      gtk_widget_show (pixmap) ;
    else
      gtk_widget_hide (pixmap) ;
    }
  }

/* Creates a toggle button with a "down" pixmap and an "up" pixmap
   both pixmaps are added to a vbox, but one of them is always hidden.  A signal
   handler is connected to the "toggled" signal to hide the visible pixmap and
   unhide the hidden one */
GtkWidget *create_two_pixmap_toggle_button (GtkImage *pix1, GtkImage *pix2, gchar *pszLbl)
  {
  GtkWidget *btn ;
  GtkWidget *vbox1, *label1 ;
  
  btn = gtk_toggle_button_new () ;
  gtk_widget_ref (btn) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "btn", btn, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (btn) ;
  
  vbox1 = gtk_vbox_new (FALSE, 0) ;
  gtk_widget_ref (vbox1) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1) ;
  gtk_container_add (GTK_CONTAINER (btn), vbox1) ;
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 2) ;
  
  gtk_widget_ref (GTK_WIDGET (pix1)) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "pix1", pix1, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (GTK_WIDGET (pix1)) ;
  gtk_box_pack_start (GTK_BOX (vbox1), GTK_WIDGET (pix1), FALSE, FALSE, 0) ;
  gtk_widget_ref (GTK_WIDGET (pix2)) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "pix2", pix2, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (GTK_WIDGET (pix2)) ;
  gtk_box_pack_start (GTK_BOX (vbox1), GTK_WIDGET (pix2), FALSE, FALSE, 0) ;
  gtk_widget_hide (GTK_WIDGET (pix2)) ;
  
  if (NULL != pszLbl)
    {
    label1 = gtk_label_new (pszLbl) ;
    gtk_widget_ref (label1) ;
    gtk_object_set_data_full (GTK_OBJECT (btn), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1) ;
    gtk_box_pack_end (GTK_BOX (vbox1), label1, TRUE, TRUE, 0) ;
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT) ;
    gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.5) ;
    }
    
  gtk_signal_connect (GTK_OBJECT (btn), "toggled", GTK_SIGNAL_FUNC (switch_pix), vbox1) ;
  
  return btn ;
  }

static void switch_pix (GtkWidget *tbtn, gpointer user_data)
  {
  GtkImage *pix_old = NULL, *pix_new = NULL ;
  gboolean bActive = FALSE ;
  
  bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tbtn)) ;
  pix_old = GTK_IMAGE (gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix1" : "pix2")) ;
  pix_new = GTK_IMAGE (gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix2" : "pix1")) ;
  gtk_widget_hide (GTK_WIDGET (pix_old)) ;
  gtk_widget_show (GTK_WIDGET (pix_new)) ;
  }

GtkWidget *gtk_button_new_with_stock_image (gchar *pszStock, gchar *pszLabel)
  {
  GtkWidget *btn = NULL, *hbox = NULL, *align = NULL, *label = NULL, *img = NULL ;

  btn = gtk_button_new () ;
  gtk_widget_show (btn) ;
  GTK_WIDGET_SET_FLAGS (btn, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  align = gtk_alignment_new (0.5, 0.5, 0, 0) ;
  gtk_widget_show (align) ;
  gtk_container_add (GTK_CONTAINER (btn), align) ;

  hbox = gtk_hbox_new (FALSE, 0) ;
  gtk_widget_show (hbox) ;
  gtk_container_add (GTK_CONTAINER (align), hbox) ;
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0) ;

  img = gtk_image_new_from_stock (pszStock, GTK_ICON_SIZE_BUTTON) ;
  gtk_widget_show (img) ;
  gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, FALSE, 0) ;

  label = gtk_label_new (pszLabel) ;
  gtk_widget_show (label) ;
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0) ;

  return btn ;
  }
