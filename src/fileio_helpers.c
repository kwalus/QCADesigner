#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "fileio_helpers.h"

#define DBG_FIOH(s)

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
