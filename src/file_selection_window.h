#ifndef _FILE_SELECTION_WINDOW_H_
#define _FILE_SELECTION_WINDOW_H_

#include <gtk/gtk.h>

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bOverwritePrompt) ;

gchar *get_external_app (GtkWindow *parent, char *pszWinTitle, char *pszCfgFName, char *pszDefaultContents, gboolean bForceNew) ;

#endif /* _FILE_SELECTION_WINDOW_H_ */
