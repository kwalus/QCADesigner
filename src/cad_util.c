#include "cad_util.h"

/* Causes a rectangle of width (*pdRectWidth) and height (*pdRectHeight) to fit inside a rectangle of
   width dWidth and height dHeight.  Th resulting pair ((*px),(*py)) holds the coordinates of the
   upper left corner of the scaled rectangle wrt. the upper left corner of the given rectangle. */

void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight)
  {
  double dAspectRatio, dRectAspectRatio ;
  
  if (0 == dWidth || 0 == dHeight || 0 == *pdRectWidth || 0 == *pdRectHeight) return ;
  
  dAspectRatio = dWidth / dHeight ;
  dRectAspectRatio = *pdRectWidth / *pdRectHeight ;
  
  if (dRectAspectRatio > dAspectRatio)
    {
    *px = 0 ;
    *pdRectWidth = dWidth ;
    *pdRectHeight = *pdRectWidth / dRectAspectRatio ;
    *py = (dHeight - *pdRectHeight) / 2 ;
    }
  else
    {
    *py = 0 ;
    *pdRectHeight = dHeight ;
    *pdRectWidth = *pdRectHeight * dRectAspectRatio ;
    *px = (dWidth - *pdRectWidth) / 2 ;
    }
  }
