#ifndef _DRAG_RECT_H_
#define _DRAG_RECT_H_

#include <gdk/gdk.h>

typedef struct
  {
  GdkPixmap *src ;
  GdkPoint ptBeg ;
  GdkPoint ptOld ;
  GdkRectangle rcDraw ;
  GdkRegion *rgnDraw ;
  } DRAG_RECT ;

DRAG_RECT *drag_rect_new (GdkWindow *dst, int xBeg, int yBeg) ;
void       drag_rect_move (DRAG_RECT *drag_rect, GdkWindow *dst, int xNew, int yNew, int cxOverlap, int cyOverlap) ;
DRAG_RECT *drag_rect_free (DRAG_RECT *drag_rect) ;

#endif /* def _DRAG_RECT_H_ */
