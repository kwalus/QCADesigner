#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include "file_selection_window.h"
#include "print.h"
#include "print_preview.h"
#include "globals.h"

typedef struct
  {
  char *pszCmdLine ;
  char *pszFName ;
  } PREVIEW_THREAD_PARAMS ;

void CreateUserFName (char *pszBaseName, char *pszFName, int cb) ;
void RunPreviewer (char *pszCmdLine, char *pszFName) ;
void *PreviewerThread (void *p) ;

void CreateUserFName (char *pszBaseName, char *pszFName, int cb)
  {
  char *pszHome = getenv ("HOME") ;
  int ic = 0 ;
  g_snprintf (pszFName, cb, "%s%s.QCADesigner", pszHome, 
    '/' == pszHome[strlen (pszHome) - 1] ? "" : "/") ;
  mkdir (pszFName, 07777) ;
  ic = strlen (pszFName) ;
  g_snprintf (&pszFName[ic], cb - ic, "/%s", pszBaseName) ;
  }

void do_print_preview (print_OP *ppo, GtkWindow *parent, qcell *first_cell)
  {
  char *pszWinTitle = "Please Select Postscript Viewer" ;
  char *pszCmdLine = malloc (2 * PATH_LENGTH) ;
  char szPreviewer[PATH_LENGTH] = "" ;
  char szCfgFile[PATH_LENGTH] = "" ;
  FILE *pfile = NULL ;
  int ic = 0, Nix, fd = -1 ;
  /************************************/
  char szBackupString[PATH_LENGTH] = "" ;
  gboolean bBackupPrintFile = TRUE ;
  /************************************/
  
  /* Back up structure values that must be altered in order to create the preview file */
  g_snprintf (szBackupString, PATH_LENGTH, "%s", ppo->szPrintString) ;
  bBackupPrintFile = ppo->bPrintFile ;
  /*****************************************************************/
  
  CreateUserFName ("previewer", szCfgFile, PATH_LENGTH) ;
  if (NULL == (pfile = fopen (szCfgFile, "r")))
    get_file_name_from_user (parent, pszWinTitle, szPreviewer, PATH_LENGTH) ;
  else if (NULL == fgets (szPreviewer, PATH_LENGTH, pfile))
    {
    fclose (pfile) ;
    get_file_name_from_user (parent, pszWinTitle, szPreviewer, PATH_LENGTH) ;
    }
  else if (0 == szPreviewer[0]) /* grabbed empty string from file */
    {
    fclose (pfile) ;
    get_file_name_from_user (parent, pszWinTitle, szPreviewer, PATH_LENGTH) ;
    }
  else
    fclose (pfile) ;
  
  if (szPreviewer[0] != 0)
    {  
    /* Iron out CRs and LFs */
    ic = strlen (szPreviewer) ;
    for (Nix = 0 ; Nix < ic ; Nix++)
      if ('\r' == szPreviewer[Nix] || '\n' == szPreviewer[Nix])
	szPreviewer[Nix] = 0 ;
      
    if (NULL != (pfile = fopen (szCfgFile, "w")))
      {
      fprintf (pfile, "%s\n", szPreviewer) ;
      fclose (pfile) ;
      }

    CreateUserFName ("previewXXXXXX", ppo->szPrintString, PATH_LENGTH) ;
    if (-1 != (fd = mkstemp (ppo->szPrintString)))
      {
      close (fd) ;
      print_world (ppo, first_cell) ;
      
      g_snprintf (pszCmdLine, 2 * PATH_LENGTH, "%s ", szPreviewer) ;
      ic = strlen (pszCmdLine) ;
      g_snprintf (&pszCmdLine[ic], 2 * PATH_LENGTH - ic, "%s", ppo->szPrintString) ;
      RunPreviewer (pszCmdLine, &pszCmdLine[ic]) ;
      pszCmdLine = NULL ;
      }
    }
  
  /* Restore structure values from the ones backed up at the beginning */
  if (NULL != pszCmdLine) free (pszCmdLine) ;
  g_snprintf (ppo->szPrintString, PATH_LENGTH, "%s", szBackupString) ;
  ppo->bPrintFile = bBackupPrintFile ;
  /*****************************************************************/
  }

void RunPreviewer (char *pszCmdLine, char *pszFName)
  {
  PREVIEW_THREAD_PARAMS *pptp = (PREVIEW_THREAD_PARAMS *)malloc (sizeof (PREVIEW_THREAD_PARAMS)) ;
  pthread_attr_t ptAttr ;
  pthread_t tid ;
  
  pptp->pszCmdLine = pszCmdLine ;
  pptp->pszFName = pszFName ;
  
  if (pthread_attr_init (&ptAttr))
    return ;
  if (pthread_attr_setdetachstate (&ptAttr, PTHREAD_CREATE_DETACHED))
    {
    pthread_attr_destroy (&ptAttr) ;
    return ;
    }
  if (pthread_create (&tid, &ptAttr, PreviewerThread, pptp))
    {
    free (pszCmdLine) ;
    free (pptp) ;
    }
  
  pthread_attr_destroy (&ptAttr) ;
  }

void *PreviewerThread (void *p)
  {
  PREVIEW_THREAD_PARAMS *pptp = (PREVIEW_THREAD_PARAMS *)p ;
  system (pptp->pszCmdLine) ;
  unlink (pptp->pszFName) ;
  free (pptp->pszCmdLine) ;
  free (pptp) ;
  }
