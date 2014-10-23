//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// The pixel <--> nanometer transformation functions,   //
// as well as a transformation stack.                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include "object_helpers.h"
#include "../generic_utils.h"
#include "../exp_array.h"
#include "QCADSubstrate.h"

static EXP_ARRAY *transformation_stack = NULL ;

static int subs_top_x = 100, subs_top_y = 100 ;
static double scale = 10 ; // pixels / nm
static int cxClientArea = 0, cyClientArea = 0 ;
static double aspect_ratio ; // cxClient / cyClient

#ifdef DESIGNER
static QCADSubstrate *snap_source = NULL ;

static void snap_source_is_gone (gpointer data, GObject *obj) ;
#endif /* def DESIGNER */

void push_transformation ()
  {
  TRANSFORMATION xform = {-1} ;

  xform.subs_top_x = subs_top_x ;
  xform.subs_top_y = subs_top_y ;
  xform.cxClientArea = cxClientArea ;
  xform.cyClientArea = cyClientArea ;
  xform.scale = scale ;

  if (NULL == transformation_stack)
    transformation_stack = exp_array_new (sizeof (TRANSFORMATION), 1) ;

  exp_array_1d_insert_vals (transformation_stack, &xform, 1, -1) ;
  }

void set_transformation (TRANSFORMATION *xform)
  {
  subs_top_x   = xform->subs_top_x ;
  subs_top_y   = xform->subs_top_y ;
  scale        = xform->scale ;
  cxClientArea = xform->cxClientArea ;
  cyClientArea = xform->cyClientArea ;

  if (0 != cyClientArea)
    aspect_ratio = ((double)cxClientArea) / ((double)cyClientArea) ;
  }

void pop_transformation ()
  {
  subs_top_x =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).subs_top_x ;
  subs_top_y =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).subs_top_y ;
  cxClientArea =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).cxClientArea ;
  cyClientArea =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).cyClientArea ;
  scale =
    exp_array_index_1d (transformation_stack, TRANSFORMATION, transformation_stack->icUsed - 1).scale ;

  exp_array_remove_vals (transformation_stack, 1, transformation_stack->icUsed - 1, 1) ;

  if (0 != cyClientArea)
    aspect_ratio = ((double)cxClientArea) / ((double)cyClientArea) ;
  }

GdkRectangle *world_to_real_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal)
  {
  rcReal->x = world_to_real_x (rcWorld->xWorld) ;
  rcReal->y = world_to_real_y (rcWorld->yWorld) ;
  rcReal->width = world_to_real_cx (rcWorld->cxWorld) ;
  rcReal->height = world_to_real_cy (rcWorld->cyWorld) ;

  return rcReal ;
  }

WorldRectangle *real_to_world_rect (WorldRectangle *rcWorld, GdkRectangle *rcReal)
  {
  rcWorld->xWorld = real_to_world_x (rcReal->x) ;
  rcWorld->yWorld = real_to_world_y (rcReal->y) ;
  rcWorld->cxWorld = real_to_world_cx (rcReal->width) ;
  rcWorld->cyWorld = real_to_world_cy (rcReal->height) ;

  return rcWorld ;
  }

int world_to_real_x (double xWorld) {return (int)NINT (scale * xWorld + subs_top_x) ;}

int world_to_real_y (double yWorld) {return (int)NINT (scale * yWorld + subs_top_y) ;}

int world_to_real_cx (double cxWorld) {return (int)NINT (cxWorld * scale) ;}

int world_to_real_cy (double cyWorld) {return (int)NINT (cyWorld * scale) ;}

double real_to_world_x (int xReal) {return (double)((xReal - subs_top_x) / scale) ;}

double real_to_world_y (int yReal) {return (double)((yReal - subs_top_y) / scale) ;}

double real_to_world_cx (int cxReal) {return (double)(cxReal / scale) ;}

double real_to_world_cy (int cyReal) {return (double)(cyReal / scale) ;}

#ifdef DESIGNER
void world_to_grid_pt (double *pxWorld, double *pyWorld)
  {
  if (NULL == snap_source) return ;
  else
    qcad_substrate_snap_point (snap_source, pxWorld, pyWorld) ;
  }

//!Zooms out a little
void zoom_out ()
  {
  // In this case, the aspect_ratio is meaningless
  if (0 == cyClientArea) return ;
  zoom_window(-30, -30 * aspect_ratio, cxClientArea + 30, cyClientArea + 30 * aspect_ratio);
  }

//!Zooms in a little
void zoom_in ()
  {
  // In this case, the aspect_ratio is meaningless
  if (0 == cyClientArea) return ;
  zoom_window(30, 30 * aspect_ratio, cxClientArea - 30, cyClientArea - 30 * aspect_ratio);
  }

void reset_zoom ()
  {scale = 10.0 ;}

//!Zooms to the provided window dimensions.
void zoom_window (int top_x, int top_y, int bot_x, int bot_y)
  {
  int xMin, yMin, xMax, yMax, cxWindow, cyWindow ;
  double
    dcxWindow = (double)(cxWindow = (xMax = MAX (top_x, bot_x)) - (xMin = MIN (top_x, bot_x))),
    dcyWindow = (double)(cyWindow = (yMax = MAX (top_y, bot_y)) - (yMin = MIN (top_y, bot_y))),
    dx = 0, dy = 0,
    dcxArea = (double)cxClientArea,
    dcyArea = (double)cyClientArea,
    scale_factor = 0;

  fit_rect_inside_rect (dcxArea, dcyArea, &dx, &dy, &dcxWindow, &dcyWindow) ;

  scale_factor = dcxWindow / (double)cxWindow ;

  pan (
    dx - ((xMin - subs_top_x) * scale_factor + subs_top_x),
    dy - ((yMin - subs_top_y) * scale_factor + subs_top_y)) ;

  scale = scale * scale_factor ;
  }

void set_snap_source (void *new_snap_source)
  {
  if (NULL != (snap_source = new_snap_source))
    g_object_weak_ref (G_OBJECT (new_snap_source), (GWeakNotify)snap_source_is_gone, NULL) ;
  }
#endif /* def DESIGNER */

// cx and cy are in real coordinates
void pan (int cx, int cy)
  {
  subs_top_x += cx ;
  subs_top_y += cy ;
  }

void set_client_dimension (int cxNew, int cyNew)
  {
  cxClientArea = cxNew ;
  cyClientArea = cyNew ;
  if (0 != cyNew)
    aspect_ratio = ((double)cxNew) / ((double)cyNew) ;
  }

#ifdef DESIGNER
static void snap_source_is_gone (gpointer data, GObject *obj)
  {
  if (snap_source == (QCADSubstrate *)obj)
    snap_source = NULL ;
  }
#endif /*def DESIGNER */

gboolean is_real_point_visible (int xReal, int yReal)
  {return (xReal >= 0 && xReal < cxClientArea && yReal >= 0 && yReal < cxClientArea) ;}

gboolean is_real_rect_visible (GdkRectangle *rc)
  {return (!(rc->x + rc->width < 0 || rc->y + rc->height < 0 || rc->x >= cxClientArea || rc->y >= cyClientArea)) ;}

void get_world_viewport (double *pxMin, double *pyMin, double *pxMax, double *pyMax)
  {
  (*pxMin) = real_to_world_x (0) ;
  (*pyMin) = real_to_world_y (0) ;
  (*pxMax) = real_to_world_x (cxClientArea - 1) ;
  (*pyMax) = real_to_world_y (cyClientArea - 1) ;
  }

void world_rectangle_union (WorldRectangle *rc1, WorldRectangle *rc2, WorldRectangle *rcDst)
  {
  double xWorld, yWorld ;

  if (NULL == rc1 || NULL == rc2 || NULL == rcDst) return ;

  xWorld = MIN (rc1->xWorld, rc2->xWorld) ;
  yWorld = MIN (rc1->yWorld, rc2->yWorld) ;
  rcDst->cxWorld = MAX (rc1->xWorld + rc1->cxWorld, rc2->xWorld + rc2->cxWorld) - xWorld ;
  rcDst->cyWorld = MAX (rc1->yWorld + rc1->cyWorld, rc2->yWorld + rc2->cyWorld) - yWorld ;
  rcDst->xWorld = xWorld ;
  rcDst->yWorld = yWorld ;
  }

gboolean world_rectangle_intersect (WorldRectangle *src1, WorldRectangle *src2, WorldRectangle *dest)
  {
  double dest_x, dest_y;
  double dest_w, dest_h;
  gboolean return_val;

  if (NULL == src1 || NULL == src2 || NULL == dest) return FALSE ;

  return_val = FALSE;

  dest_x = MAX (src1->xWorld, src2->xWorld);
  dest_y = MAX (src1->yWorld, src2->yWorld);
  dest_w = MIN (src1->xWorld + src1->cxWorld, src2->xWorld + src2->cxWorld) - dest_x;
  dest_h = MIN (src1->yWorld + src1->cyWorld, src2->yWorld + src2->cyWorld) - dest_y;

  if (dest_w > 0 && dest_h > 0)
    {
    dest->xWorld = dest_x;
    dest->yWorld = dest_y;
    dest->cxWorld = dest_w;
    dest->cyWorld = dest_h;
    return_val = TRUE;
    }
  else
    {
    dest->cxWorld = 0;
    dest->cyWorld = 0;
    }

  return return_val;
  }

int world_rectangle_subtract (WorldRectangle *rc1, WorldRectangle *rc2, WorldRectangle new_rcs[4])
  {
  int Nix, idx = 0 ;
  int icRects = 0 ;
  double coords[4][4] = {{ 0.0 }} ;
  double xt1, yt1, xb1, yb1, xt2, yt2, xb2, yb2 ;
  gboolean bPIR0 = FALSE, bPIR1 = FALSE, bPIR2 = FALSE, bPIR3 = FALSE ;
  WorldRectangle subtrahend = {0} ;

  if (NULL == rc1 || NULL == rc2) return 0 ;

  // rc1 minus rc2 = rc1, if they don't intersect
  if (!world_rectangle_intersect (rc1, rc2, &subtrahend))
    {
    memcpy (&(new_rcs[0]), rc1, sizeof (WorldRectangle)) ;
    return 1 ;
    }
  else
  // if rc1 is inside rc2 then rc1 - rc2 = NULL
  if (RECT_IN_RECT (rc1->xWorld, rc1->yWorld, rc1->cxWorld, rc1->cyWorld, rc2->xWorld, rc2->yWorld, rc2->cxWorld, rc2->cyWorld))
    return 0 ;

  xb1 = (xt1 = rc1->xWorld) + rc1->cxWorld ; xb2 = (xt2 = subtrahend.xWorld) + subtrahend.cxWorld ; 
  yb1 = (yt1 = rc1->yWorld) + rc1->cyWorld ; yb2 = (yt2 = subtrahend.yWorld) + subtrahend.cyWorld ;

  // Depending on which corner points are in the rect, up to 4 new rectangles can form
  bPIR0 = PT_IN_RECT (xt2, yt2, rc1->xWorld, rc1->yWorld, rc1->cxWorld, rc1->cyWorld) ;
  bPIR1 = PT_IN_RECT (xt2, yb2, rc1->xWorld, rc1->yWorld, rc1->cxWorld, rc1->cyWorld) ;
  bPIR2 = PT_IN_RECT (xb2, yb2, rc1->xWorld, rc1->yWorld, rc1->cxWorld, rc1->cyWorld) ;
  bPIR3 = PT_IN_RECT (xb2, yt2, rc1->xWorld, rc1->yWorld, rc1->cxWorld, rc1->cyWorld) ;

  if (bPIR0)
    {
    coords[0][0] = xt1 ; coords[0][1] = yt1 ; coords[0][2] = xb1 ; coords[0][3] = yt2 ;
    coords[1][0] = xt1 ; coords[1][1] = yt2 ; coords[1][2] = xt2 ; coords[1][3] = yb1 ;
    icRects = 2 ;
    if (bPIR1)
      {
      coords[1][3] = yb2 ; // Update from yb1
      coords[2][0] = xt1 ; coords[2][1] = yb2 ; coords[2][2] = xb1 ; coords[2][3] = yb1 ;
      icRects = 3 ;
      if (bPIR2)
        {
        coords[3][0] = xb2 ; coords[3][1] = yt2 ; coords[3][2] = xb1 ; coords[3][3] = yb2 ;
        icRects = 4 ;
        }
    //else (!bPIR2) nothing
      }
    else // !bPIR1
      {
      // if (bPIR2) is impossible
      if (!bPIR2)
        {
        if (bPIR3)
          {
          coords[2][0] = xb2 ; coords[2][1] = yt2 ; coords[2][2] = xb1 ; coords[2][3] = yb1 ;
          icRects = 3 ;
          }
      //else (!bPIR3) nothing
        }
      }
    }
  else // !bPIR0
    {
    if (bPIR1)
      {
      coords[0][0] = xt1 ; coords[0][1] = yt1 ; coords[0][2] = xt2 ; coords[0][3] = yb2 ;
      coords[1][0] = xt1 ; coords[1][1] = yb2 ; coords[1][2] = xb1 ; coords[1][3] = yb1 ;
      icRects = 2 ;
      if (bPIR2)
        {
        coords[2][0] = xb2 ; coords[2][1] = yt1 ; coords[2][2] = xb1 ; coords[2][3] = yb2 ;
        icRects = 3 ;
        }
    //else (!bPIR2) nothing
      }
    else // (!bPIR1)
      {
      if (bPIR2)
        {
        coords[0][0] = xt1 ; coords[0][1] = yb2 ; coords[0][2] = xb1 ; coords[0][3] = yb1 ;
        coords[1][0] = xb2 ; coords[1][1] = yt1 ; coords[1][2] = xb1 ; coords[1][3] = yb2 ;
        icRects = 2 ;
        if (bPIR3)
          {
          coords[1][1] = yt2 ; // Update from yt1
          coords[2][0] = xt1 ; coords[2][1] = yt1 ; coords[2][2] = xb1 ; coords[2][3] = yt2 ;
          icRects = 3 ;
          }
      //else (!bPIR3) nothing
        }
      else // (!bPIR2)
        {
        if (bPIR3)
          {
          coords[0][0] = xb2 ; coords[0][1] = yt2 ; coords[0][2] = xb1 ; coords[0][3] = yb1 ;
          coords[1][0] = xt1 ; coords[1][1] = yt1 ; coords[1][2] = xb1 ; coords[1][3] = yt2 ;
          icRects = 2 ;
          }
      //else (!bPIR3) nothing
        }
      }
    }

  // Use only those resulting rectangles which do not have 0 widths and/or heights
  for (Nix = 0 ; Nix < 4 ; Nix++)
    if (coords[Nix][0] < coords[Nix][2] && coords[Nix][1] < coords[Nix][3])
      {
      new_rcs[idx].xWorld = coords[Nix][0] ;
      new_rcs[idx].yWorld = coords[Nix][1] ;
      new_rcs[idx].cxWorld = coords[Nix][2] - coords[Nix][0] ;
      new_rcs[idx].cyWorld = coords[Nix][3] - coords[Nix][1] ;
      idx++ ;
      }

  return idx ;
  }
