#ifndef _PRINT_H_
#define _PRINT_H_

#include "globals.h"

typedef struct
  {
  /* Units are in points == 1/72 inches */
  double dPaperWidth ;
  double dPaperHeight ;
  double dLeftMargin ;
  double dTopMargin ;
  double dRightMargin ;
  double dBottomMargin ;
  double dPointsPerNano ;
  gboolean bPrintOrderOver ;
  gboolean bCenter ;
  gboolean *pbPrintedObjs ;
  int icPrintedObjs ;
  gboolean bPrintFile ;
  char szPrintString[PATH_LENGTH] ;
  int iCXPages ;
  int iCYPages ;
  } print_OP ;

void print_world (print_OP *pPrintOpts, qcell *first_cell) ;

#endif /*_PRINT_H_*/
