#ifndef _CUSTOM_WIDGETS_H_
#define _CUSTOM_WIDGETS_H_

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle) ;

GtkWidget *create_two_pixmap_toggle_button (GtkPixmap *pix1, GtkPixmap *pix2, gchar *pszLbl) ;

#endif /* _CUSTOM_WIDGETS_H_ */
