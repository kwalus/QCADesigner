#ifndef _FILEIO_HELPERS_H_
#define _FILEIO_HELPERS_H_

#include <stdio.h>

char *ReadLine (FILE *pfile, char cComment) ;
char *base_name (char *pszFile) ;
char *CreateUserFName (char *pszBaseName) ;
void RunCmdLineAsync (char *pszCmdLine, char *pszTmpFName) ;
//char **CmdLineToArgv (char *pszCmdLine, int *pargc) ;

#endif /* _FILEIO_HELPERS_H_ */
