#ifdef WIN32
  #include <windows.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "file_selection_window.h"
#include "print.h"
#include "print_preview.h"
#include "fileio_helpers.h"

#define DBG_PP(s)

void do_print_preview (print_OP *ppo, GtkWindow *parent, void *data, PrintFunction fcnPrint)
  {
  char *pszPrintString = ppo->pszPrintString ;
  gboolean bPrintFile = ppo->bPrintFile ;
  int fd = -1 ;
  char *pszCfgFile = NULL, *pszPreviewer = NULL, *pszFName = NULL, *pszCmdLine = NULL ;
#ifdef WIN32
  char szBuf[MAX_PATH] = "" ;
  FILE *pfile = NULL ;
#endif
  
#ifdef WIN32
  pszPreviewer = get_external_app (parent, "Please Select PostScript Viewer", "previewer", "C:\\Program Files\\Ghostgum\\gsview\\gsview32.exe", FALSE) ;
#else
  pszPreviewer = get_external_app (parent, "Please Select PostScript Viewer", "previewer", "/usr/bin/ggv", FALSE) ;
#endif
  
  if (NULL == pszPreviewer)
    return ;

  ppo->pszPrintString = CreateUserFName ("previewXXXXXX") ;
#ifdef WIN32
  mktemp (ppo->pszPrintString) ;
  pfile = fopen (ppo->pszPrintString, "w") ;
  fd = (NULL == pfile ? -1 : -2) ;
  if (NULL != pfile) fclose (pfile) ;

  /* In Windoze, we need to perform the extra step of grabbing
     the DOS-style path corresponding to the preview file name */
  GetShortPathName (ppo->pszPrintString, szBuf, MAX_PATH) ;
  g_free (ppo->pszPrintString) ;
  ppo->pszPrintString = g_strdup_printf ("%s", szBuf) ;
#else
  fd = mkstemp (ppo->pszPrintString) ;
#endif /* WIN32 */

  if (-1 == fd)
    {
    /* After much effort, a temporary file could not be conjured up.
       Give the user the bad news */
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, 
      GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      "Unable to create temporary file for preview !"))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszPreviewer) ;
    g_free (ppo->pszPrintString) ;
    ppo->pszPrintString = pszPrintString ;
    ppo->bPrintFile = bPrintFile ;
    return ;
    }

  if (-2 != fd) close (fd) ;
  (*fcnPrint) (ppo, data) ;

  pszFName = ppo->pszPrintString ;

  /* Restore the 2 ppo fields that were mutilated */
  ppo->pszPrintString = pszPrintString ;
  ppo->bPrintFile = bPrintFile ;

  pszCmdLine = g_strdup_printf ("%s %s", pszPreviewer, pszFName) ;

  RunCmdLineAsync (pszCmdLine, pszFName) ;

  g_free (pszPreviewer) ;
  g_free (pszFName) ;
  g_free (pszCmdLine) ;
  g_free (pszCfgFile) ;

  fprintf (stderr, "Returning from do_print_preview\n") ;
  }
