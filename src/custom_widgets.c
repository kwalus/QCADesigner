// GTK includes //

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "support.h"
#include "custom_widgets.h"

void switch_pix (GtkWidget *tbtn, gpointer user_data) ;

GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle)
  {
  GtkWidget *btn ;
  GtkWidget *vbox1, *label1 ;
  
  btn = bToggle ? gtk_toggle_button_new () : gtk_button_new () ;
  gtk_widget_ref (btn) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "btn", btn, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (btn) ;
  
  vbox1 = gtk_vbox_new (FALSE, 0) ;
  gtk_widget_ref (vbox1) ;
  gtk_object_set_data_full (GTK_OBJECT (btn), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1) ;
  gtk_container_add (GTK_CONTAINER (btn), vbox1) ;
  
  gtk_widget_show (pixmap) ;
  gtk_box_pack_start (GTK_BOX (vbox1), pixmap, FALSE, FALSE, 0) ;
  
  if (NULL != text)
    {
    label1 = gtk_label_new (text) ;
    gtk_widget_ref (label1) ;
    gtk_object_set_data_full (GTK_OBJECT (btn), "label1", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1) ;
    gtk_box_pack_start (GTK_BOX (vbox1), label1, TRUE, TRUE, 0) ;
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT) ;
    gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.5) ;
    }
  
  return btn ;
  }

GtkWidget *create_two_pixmap_toggle_button (GtkPixmap *pix1, GtkPixmap *pix2, gchar *pszLbl)
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

void switch_pix (GtkWidget *tbtn, gpointer user_data)
  {
  GtkPixmap *pix_old = NULL, *pix_new = NULL ;
  gboolean bActive = FALSE ;
  
  bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tbtn)) ;
  pix_old = GTK_PIXMAP (gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix1" : "pix2")) ;
  pix_new = GTK_PIXMAP (gtk_object_get_data (GTK_OBJECT (tbtn), bActive ? "pix2" : "pix1")) ;
  gtk_widget_hide (GTK_WIDGET (pix_old)) ;
  gtk_widget_show (GTK_WIDGET (pix_new)) ;
  }
