#ifndef _CUSTOM_WIDGETS_H_
#define _CUSTOM_WIDGETS_H_

#include <gtk/gtk.h>

GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle) ;

GtkWidget *create_two_pixmap_toggle_button (GtkImage *pix1, GtkImage *pix2, gchar *pszLbl) ;

void pixmap_button_toggle_pixmap (GtkWidget *btn, gboolean bShow) ;

GtkWidget *gtk_button_new_with_stock_image (gchar *pszStock, gchar *pszLabel) ;

#endif /* _CUSTOM_WIDGETS_H_ */
