#ifdef WIN32
  #include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib.h>
#include "fileio_helpers.h"

#define DBG_FIOH(s)

static gpointer RunCmdLineAsyncThread (gpointer p) ;

/* Keep a buffer to hold a line of string, stripping comments. Return the buffer */
char *ReadLine (FILE *pfile, char cComment)
  {
  static char *pszRet = NULL ;
  static int icAvail = 0 ;
  int idx = 0 ;
  int idxComment = -1 ;
  int idxCR = -1 ;
  char c = 0 ;

  if (feof (pfile)) return NULL ;

  while (!feof (pfile))
    {
    c = 0 ;
    fread (&c, 1, 1, pfile) ;
    if ('\n' == c) break ;
    if (cComment && cComment == c) idxComment = idx ;
    if ('\r' == c) idxCR = idx ;
    if (idx == icAvail) pszRet = realloc (pszRet, icAvail = icAvail * 2 + 1) ;
    pszRet[idx++] = c ;
    }
  if (idx == icAvail) pszRet = realloc (pszRet, icAvail = icAvail * 2 + 1) ;
  pszRet[idx] = 0 ;
  if (-1 != idxComment) pszRet[idxComment] = 0 ;
  if (-1 != idxCR) pszRet[idxCR] = 0 ;
  DBG_FIOH (fprintf (stderr, "ReadLine returning |%s|\n", pszRet)) ;
  
  return pszRet ;
  }

char *base_name (char *pszFile)
  {
  char *pszRet = &(pszFile[strlen (pszFile)]) ;
  while (--pszRet > pszFile)
    if (*pszRet == '/' || *pszRet == '\\')
      return pszRet + 1 ;
  return pszFile ;
  }

void RunCmdLineAsync (char *pszCmdLine)
  {
  char *pszCmdLineCopy = g_strdup (pszCmdLine) ;
  if (!g_thread_supported ()) g_thread_init (NULL) ;

  g_thread_create ((GThreadFunc)RunCmdLineAsyncThread, (gpointer)pszCmdLineCopy, FALSE, NULL) ;
  }

static gpointer RunCmdLineAsyncThread (gpointer p)
  {
  char *pszCmdLine = (char *)p ;
#ifdef WIN32
  STARTUPINFO si ;
  PROCESS_INFORMATION pi ;

  memset (&si, 0, sizeof (si)) ;
  memset (&pi, 0, sizeof (pi)) ;
  si.cb = sizeof (STARTUPINFO) ;

  if (CreateProcess (NULL, pszCmdLine, NULL, NULL, FALSE, DETACHED_PROCESS,
    NULL, NULL, &si, &pi))
    {
    WaitForSingleObject (pi.hProcess, INFINITE) ;
    CloseHandle (pi.hProcess) ;
    CloseHandle (pi.hThread) ;
    }
#else
  system (pszCmdLine) ;
#endif
  g_free (pszCmdLine) ;

  return NULL ;
  }

char *CreateUserFName (char *pszBaseName)
  {
  char *pszHome = getenv ("HOME"), *psz = NULL, *pszRet = NULL ;
  psz = g_strdup_printf ("%s%s.QCADesigner", pszHome,
    G_DIR_SEPARATOR == pszHome[strlen (pszHome) - 1] ? "" : G_DIR_SEPARATOR_S) ;
#ifndef WIN32
  mkdir (psz, 07777) ;
#else
  mkdir (psz) ;
#endif
  pszRet = g_strdup_printf ("%s%c%s", psz, G_DIR_SEPARATOR, pszBaseName) ;
  g_free (psz) ;
  return pszRet ;
  }
/*
// Turn a string into an argv-style array
char **CmdLineToArgv (char *pszTmp, int *pargc)
  {
  char **argv = NULL, *psz = g_strdup_printf ("%s", pszTmp), *pszAt = psz, *pszStart = psz ;
  gboolean bString = FALSE ;
  
  (*pargc) = 0 ;
  
  for (pszAt = psz ; ; pszAt++)
    {
    if (0 == (*pszAt)) break ;
    if (' ' == (*pszAt))
      {
      if (!bString)
        {
        (*pszAt) = 0 ;
        argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
        argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
        pszAt++ ;
        while (' ' == (*pszAt))
          pszAt++ ;
        pszStart = pszAt ;
        }
      }
    
    if ('\"' == (*pszAt))
      {
      if (!bString)
        pszStart = pszAt = pszAt + 1 ;
      else
        {
        (*pszAt) = 0 ;
        argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
        argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
        pszAt++ ;
        while (' ' == (*pszAt))
          pszAt++ ;
        pszStart = pszAt ;
        }
      bString = !bString ;
      }
    }

  argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
  argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
  argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
  argv[(*pargc) - 1] = NULL ;
  
  (*pargc)-- ;
  
  g_free (psz) ;
  return argv ;
  }
*/
