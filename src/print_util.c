#include <stdio.h>
#include <math.h>
#include "print.h"

#define MAX_SLOPE_DIFF 0.1

#define DBG_PU(s)

FILE *OpenPrintStream (print_OP *pPO)
  {
  if (pPO->bPrintFile)
    return fopen (pPO->pszPrintString, "w") ;
  else
    return popen (pPO->pszPrintString, "w") ;
  }

inline gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2)
  {
  if (dx0 == dx1 && dx1 == dx2) return TRUE ;
  if (dy0 == dy1 && dy1 == dy2) return TRUE ;
  
  DBG_PU (fprintf (stderr, "slope diff: %lf\n", fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)))) ;
  
  return (fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)) < MAX_SLOPE_DIFF) ;
  }
