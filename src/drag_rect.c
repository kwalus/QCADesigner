#include <gdk/gdk.h>
#include "drag_rect.h"

// Copy a "save-under" of the window
DRAG_RECT *drag_rect_new (GdkWindow *dst, int xBeg, int yBeg)
  {
  int cx, cy ;
  DRAG_RECT *drag_rect = NULL ;
  GdkGC *gc = NULL ;

  gdk_window_get_size (dst, &cx, &cy) ;

  drag_rect = g_malloc0 (sizeof (DRAG_RECT)) ;
//  drag_rect->bFirstMove = TRUE ;
  drag_rect->src = gdk_pixmap_new (dst, cx, cy, -1) ;
  gc = gdk_gc_new (drag_rect->src) ;
  gdk_draw_drawable (drag_rect->src, gc, dst, 0, 0, 0, 0, cx, cy) ;
  drag_rect->rcDraw.x =
  drag_rect->ptOld.x =
  drag_rect->ptBeg.x = xBeg ;
  drag_rect->rcDraw.y =
  drag_rect->ptOld.y =
  drag_rect->ptBeg.y = yBeg ;
  drag_rect->rcDraw.width =
  drag_rect->rcDraw.height = 0 ;
  g_object_unref (gc) ;

  return drag_rect ;
  }

// Draw bits of the "save-under", thereby adjusting the available rectangle,
// the coordinates of which get deposited in rcDraw. In addition, this function
// returns an "invalid region", IOW the bits of rcDraw that are not part of rcOld.
void drag_rect_move (DRAG_RECT *drag_rect, GdkWindow *dst, int xNew, int yNew, int cxOverlap, int cyOverlap)
  {
  GdkRectangle rcNew = {0} ;
  GdkRegion *rgnSub = NULL, *rgnBlt = NULL ;

  if (NULL == drag_rect) return ;

  if (NULL != drag_rect->rgnDraw)
    {
    gdk_region_destroy (drag_rect->rgnDraw) ;
    drag_rect->rgnDraw = NULL ;
    }

  if (!(xNew == drag_rect->ptOld.x && yNew == drag_rect->ptOld.y))
    {
    int Nix, icRcs = -1 ;
    GdkRectangle *rcs = NULL ;
    GdkGC *gc = NULL ;

    rcNew.width  = ABS (drag_rect->ptBeg.x - xNew) + 1 ;
    rcNew.height = ABS (drag_rect->ptBeg.y - yNew) + 1 ;
    rcNew.x      = MIN (drag_rect->ptBeg.x, xNew) ;
    rcNew.y      = MIN (drag_rect->ptBeg.y, yNew) ;

    // rcNew - rcOld = needs_drawing
    // rcOld - rcNew = Blt
    drag_rect->rgnDraw = gdk_region_rectangle (&rcNew) ;
    rgnSub = gdk_region_copy (drag_rect->rgnDraw) ;
    rgnBlt = gdk_region_rectangle (&(drag_rect->rcDraw)) ;
    gdk_region_subtract (drag_rect->rgnDraw, rgnBlt) ;
    gdk_region_subtract (rgnBlt, rgnSub) ;
    gdk_region_destroy (rgnSub) ;

    if (gdk_region_empty (drag_rect->rgnDraw))
      {
      gdk_region_destroy (drag_rect->rgnDraw) ;
      drag_rect->rgnDraw = NULL ;
      }
    else
    if (!(0 == cxOverlap && 0 == cyOverlap))
      {
      int quadrant = 
        (xNew > drag_rect->ptBeg.x && yNew > drag_rect->ptBeg.y) ? 3 :
        (xNew < drag_rect->ptBeg.x && yNew > drag_rect->ptBeg.y) ? 2 :
        (xNew < drag_rect->ptBeg.x && yNew < drag_rect->ptBeg.y) ? 1 :
        (xNew > drag_rect->ptBeg.x && yNew < drag_rect->ptBeg.y) ? 0 : -1 ;
      GdkRectangle rcOldMod = {drag_rect->rcDraw.x, drag_rect->rcDraw.y, drag_rect->rcDraw.width, drag_rect->rcDraw.height} ;

/*
      +---+---+---+---+---+
      | Q | x | y | w | h | 
      +---+---+---+---+---+
      |-1 |   |   |   |   |
      +---+---+---+---+---+
      | 0 |   | + | - | - |
      +---+---+---+---+---+
      | 1 | + | + | - | - |
      +---+---+---+---+---+
      | 2 | + |   | - | - |                                                   
      +---+---+---+---+---+
      | 3 |   |   | - | - |                                                   
      +---+---+---+---+---+
*/
      if (-1 != quadrant)
        {rcOldMod.width -= cxOverlap ; rcOldMod.height -= cyOverlap ;}
      if (1 == quadrant || 2 == quadrant)
        {rcOldMod.x += cxOverlap ;}
      if (0 == quadrant || 1 == quadrant)
        {rcOldMod.y += cyOverlap ;}

      gdk_region_destroy (drag_rect->rgnDraw) ;
      drag_rect->rgnDraw = gdk_region_rectangle (&rcNew) ;
      rgnSub = gdk_region_rectangle (&rcOldMod) ;
      gdk_region_subtract (drag_rect->rgnDraw, rgnSub) ;
      gdk_region_destroy (rgnSub) ;
      }

    gdk_region_get_rectangles (rgnBlt, &rcs, &icRcs) ;
    if (icRcs > 0)
      {
      gc = gdk_gc_new (dst) ;
      for (Nix = 0 ; Nix < icRcs ; Nix++)
        gdk_draw_drawable (dst, gc, drag_rect->src, rcs[Nix].x, rcs[Nix].y, rcs[Nix].x, rcs[Nix].y, rcs[Nix].width, rcs[Nix].height) ;
      g_object_unref (gc) ;
      g_free (rcs) ;
      }
    gdk_region_destroy (rgnBlt) ;
    }

  drag_rect->ptOld.x = xNew ;
  drag_rect->ptOld.y = yNew ;
  drag_rect->rcDraw.x      = rcNew.x ;
  drag_rect->rcDraw.y      = rcNew.y ;
  drag_rect->rcDraw.width  = rcNew.width ;
  drag_rect->rcDraw.height = rcNew.height ;
  }

// This function frees a DRAG_RECT and always returns NULL
DRAG_RECT *drag_rect_free (DRAG_RECT *drag_rect)
  {
  if (NULL == drag_rect) return NULL ;

  if (NULL != drag_rect->rgnDraw)
    gdk_region_destroy (drag_rect->rgnDraw) ;
  g_object_unref (drag_rect->src) ;
  g_free (drag_rect) ;

  return NULL ;
  }
