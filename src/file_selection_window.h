#ifndef _FILE_SELECTION_WINDOW_H_
#define _FILE_SELECTION_WINDOW_H_

#include <gtk/gtk.h>

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bOverwritePrompt) ;

#endif /* _FILE_SELECTION_WINDOW_H_ */
