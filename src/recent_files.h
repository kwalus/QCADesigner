#ifndef _RECENT_FILES_H_
#define _RECENT_FILES_H_

#include <gtk/gtk.h>

void fill_recent_files_menu (GtkWidget *menu, GtkSignalFunc pfn, gpointer data) ;
void add_to_recent_files (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data) ;
void remove_recent_file (GtkWidget *menu, char *pszFName, GtkSignalFunc pfn, gpointer data) ;

#endif /* _RECENT_FILES_H_ */
