#include <stdio.h>
#include "print.h"

FILE *OpenPrintStream (print_OP *pPO)
  {
  if (pPO->bPrintFile)
    return fopen (pPO->pszPrintString, "w") ;
  else
    return popen (pPO->pszPrintString, "w") ;
  }
