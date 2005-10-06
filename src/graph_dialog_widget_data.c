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
// Functions related to maintaining HONEYCOMB_DATA and  //
// WAVEFORM_DATA structures necessary for drawing the   //
// various traces in the graph dialog.                  //
//                                                      //
//////////////////////////////////////////////////////////

#include <string.h>
#include <math.h>
#include "gdk_structs.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "simulation.h"
#include "exp_array.h"
#include "generic_utils.h"
#include "graph_dialog_widget_data.h"
#include "objects/QCADCell.h"

// Bits per decimal digit == ln(10)/ln(2)
#define BITS_PER_DEC_DIGIT 3.321928095

typedef enum
  {
  HCT_NONE,
  HCT_GARBAGE_TO_HONEYCOMB,
  HCT_HONEYCOMB_TO_GARBAGE,
  HCT_HONEYCOMB_TO_HONEYCOMB
  } HoneycombTransition ;

static long long unsigned int calculate_honeycomb_value (EXP_ARRAY *bits) ;
static HoneycombTransition calculate_honeycomb_transition (EXP_ARRAY *old_bits, EXP_ARRAY *new_bits) ;
static void calculate_trace_bits (EXP_ARRAY *arTraces, EXP_ARRAY *bits, int idxSample, int icSamples, double dThreshLower, double dThreshUpper, int icAverageSamples) ;
static double calculate_average_polarization (double *data, int idxSample, int icSamples, int icAverageSamples) ;
/*
void fit_graph_data_to_window (GRAPH_DATA *graph_data, int cxWindow, int cxWanted, int beg_sample, int end_sample, int icSamples)
  {
  graph_data->cxGiven = 
    MAX (graph_data->cxWanted, 
      (int)((((double)(icSamples)) * ((double)(cxWanted))) /
        ((double)(end_sample - beg_sample)))) ;

  graph_data->xOffset = -(((double)(graph_data->cxGiven) * (double)beg_sample) / (double)icSamples) ;

  graph_data->bNeedCalc = TRUE ;
  }
*/
HONEYCOMB_DATA *honeycomb_data_new (GdkColor *clr)
  {
  HONEYCOMB_DATA *hc = NULL ;

  hc = g_malloc0 (sizeof (HONEYCOMB_DATA)) ;
  hc->graph_data.data_type = GRAPH_DATA_TYPE_BUS ;
  hc->graph_data.bNeedCalc = FALSE ;
  hc->graph_data.cxWanted  =
  hc->graph_data.cyWanted  =
  hc->graph_data.cxGiven   =
  hc->graph_data.cyGiven   = -1 ;
//  hc->graph_data.xOffset   =  0 ;
  hc->graph_data.bVisible  = TRUE ;
  hc->arTraces             = exp_array_new (sizeof (struct TRACEDATA *), 1) ;
  memcpy (&(hc->graph_data.clr), clr, sizeof (GdkColor)) ;

  return hc ;
  }

HONEYCOMB_DATA *honeycomb_data_new_with_array (GdkColor *clr, simulation_data *sim_data, BUS *bus, int offset, double thresh_lower, double thresh_upper, int icAverageSamples, int base)
  {
  struct TRACEDATA *the_trace = NULL ;
  int Nix2 ;
  HONEYCOMB_DATA *hc = honeycomb_data_new (clr) ;

  for (Nix2 = 0 ; Nix2 < bus->cell_indices->icUsed ; Nix2++)
    {
    the_trace = &(sim_data->trace[exp_array_index_1d (bus->cell_indices, int, Nix2) + offset]) ;
    exp_array_insert_vals (hc->arTraces, &the_trace, 1, -1) ;
    }
  calculate_honeycomb_array (hc, sim_data->number_samples, thresh_lower, thresh_upper, icAverageSamples, base) ;

  return hc ;
  }

WAVEFORM_DATA *waveform_data_new (struct TRACEDATA *trace, GdkColor *clr, gboolean bStretch)
  {
  WAVEFORM_DATA *wf = NULL ;

  wf = g_malloc0 (sizeof (WAVEFORM_DATA)) ;
  wf->graph_data.data_type = GRAPH_DATA_TYPE_CELL ;
  wf->graph_data.bNeedCalc = FALSE ;
  wf->graph_data.cxWanted  =
  wf->graph_data.cyWanted  =
  wf->graph_data.cxGiven   =
  wf->graph_data.cyGiven   = -1 ;
//  wf->graph_data.xOffset   =  0 ;
  memcpy (&(wf->graph_data.clr), clr, sizeof (GdkColor)) ;
  wf->trace                = trace ;
  wf->bStretch             = bStretch ;
  wf->graph_data.bVisible  = wf->trace->drawtrace ;
  wf->arPoints             = exp_array_new (sizeof (GdkPoint), 1) ;

  return wf ;
  }

// Returns an array of honeycombs and sets hc->cxWanted based on the array
void calculate_honeycomb_array (HONEYCOMB_DATA *hc, int icSamples, double dThreshLower, double dThreshUpper, int icAverageSamples, int base)
  {
  HoneycombTransition hct = HCT_NONE ;
  int Nix ;
  HONEYCOMB hcNew = {0} ;
  EXP_ARRAY *bits = NULL ;
  EXP_ARRAY *old_bits = NULL ;
  int idxStart = -1 ; // -1 == honeycomb has not started

  hc->icHCSamples = 0 ;

  fprintf (stderr, "calculate_honeycomb_array: Entering with icAverageSamples = %d\n", icAverageSamples) ;

  if (NULL == hc->arHCs)
    hc->arHCs = exp_array_new (sizeof (HONEYCOMB), 1) ;
  else
    exp_array_remove_vals (hc->arHCs, 1, 0, hc->arHCs->icUsed) ;

  bits = exp_array_new (sizeof (int), 1) ;

  exp_array_insert_vals (bits, NULL, hc->arTraces->icUsed, 1, 0) ;
  for (Nix = hc->arTraces->icUsed - 1 ; Nix > -1  ; Nix--)
    exp_array_index_1d (bits, int, Nix) = -1 ;

  old_bits = exp_array_copy (bits) ;

  for (Nix = 0 ; Nix < icSamples ; Nix++)
    {
    calculate_trace_bits (hc->arTraces, bits, Nix, icSamples, dThreshLower, dThreshUpper, icAverageSamples) ;
    hct = calculate_honeycomb_transition (old_bits, bits) ;
    // First, eliminate impossible transitions
    if (HCT_HONEYCOMB_TO_GARBAGE == hct && idxStart < 0)
      fprintf (stderr, "honeycomb_display_data_new: O_o How can a honeycomb end before it starts ?!\n") ;
    else
    if (HCT_GARBAGE_TO_HONEYCOMB == hct && idxStart >= 0)
      fprintf (stderr, "honeycomb_display_data_new: O_o How can a honeycomb start when one is already in progress ?!\n") ;
    else
    if (HCT_HONEYCOMB_TO_HONEYCOMB == hct && idxStart < 0)
      fprintf (stderr, "honeycomb_display_data_new: O_o How can there be a switch between honeycombs when there's no honeycomb to begin with ?!\n") ;
    else
    // Remember honeycomb start
    if (HCT_GARBAGE_TO_HONEYCOMB == hct && idxStart < 0)
      idxStart = Nix ;
    // Create a new honeycomb
    if ((HCT_HONEYCOMB_TO_GARBAGE == hct || HCT_HONEYCOMB_TO_HONEYCOMB == hct || Nix == icSamples - 1) && idxStart >= 0)
      {
      hc->icHCSamples += Nix - idxStart + 1 ;
      hcNew.idxBeg = idxStart ;
      hcNew.idxEnd = Nix ;
      hcNew.value = calculate_honeycomb_value (old_bits) ;
      exp_array_insert_vals (hc->arHCs, &hcNew, 1, 1, -1) ;
      idxStart = (HCT_HONEYCOMB_TO_GARBAGE == hct) ? -1 : Nix ;
      }
    memcpy (old_bits->data, bits->data, bits->icUsed * bits->cbSize) ;
    }
#ifdef GTK_GUI
  hc->graph_data.cxWanted = calculate_honeycomb_cxWanted (hc, icSamples, base) ;
#endif /* def GTK_GUI */
  }

static void calculate_trace_bits (EXP_ARRAY *arTraces, EXP_ARRAY *bits, int idxSample, int icSamples, double dThreshLower, double dThreshUpper, int icAverageSamples)
  {
  double polarization = 0 ;
  int Nix ;

  // bits[idx], for all valid idx can have 3 values:
  //   0 represents logic 0
  //   1 represents logic 1
  //  -1 represents the indeterminate state (the cell is not polarized enough for there to be a clear logic interpretation)

  for (Nix = 0 ; Nix < arTraces->icUsed ; Nix++)
    {
    polarization = calculate_average_polarization (exp_array_index_1d (arTraces, struct TRACEDATA *, Nix)->data, idxSample, icSamples, icAverageSamples) ;
    exp_array_index_1d (bits, int, Nix) =
      polarization < dThreshLower ? 0 :
      polarization > dThreshUpper ? 1 : -1 ;
    }
  }

static double calculate_average_polarization (double *data, int idxSample, int icSamples, int icAverageSamples)
  {
  double dSum = 0 ;
  int Nix, icSamplesUsed ;

  if (icAverageSamples < 2 || idxSample == icSamples - 1) return data[idxSample] ;
  else
    {
    for (Nix = idxSample, icSamplesUsed = 0 ; Nix < icSamples && Nix < idxSample + icAverageSamples ; Nix++, icSamplesUsed++)
      dSum += data[Nix] ;
    if (icSamplesUsed > 0)
      return dSum / (double)icSamplesUsed ;
    else
      return data[idxSample] ;
    }
  }

static HoneycombTransition calculate_honeycomb_transition (EXP_ARRAY *old_bits, EXP_ARRAY *new_bits)
  {
  int Nix ;
  gboolean old_has_garbage = FALSE, new_has_garbage = FALSE ;

  for (Nix = 0 ; Nix < old_bits->icUsed ; Nix++)
    {
    if (exp_array_index_1d (old_bits, int, Nix) < 0) old_has_garbage = TRUE ;
    if (exp_array_index_1d (new_bits, int, Nix) < 0) new_has_garbage = TRUE ;
    if (old_has_garbage && new_has_garbage) return HCT_NONE ;
    }

  if (old_has_garbage && !new_has_garbage) return HCT_GARBAGE_TO_HONEYCOMB ;
  else
  if (!old_has_garbage && new_has_garbage) return HCT_HONEYCOMB_TO_GARBAGE ;

  for (Nix = 0 ; Nix < new_bits->icUsed ; Nix++)
    if (exp_array_index_1d (old_bits, int, Nix) != exp_array_index_1d (new_bits, int, Nix))
      return HCT_HONEYCOMB_TO_HONEYCOMB ;

  return HCT_NONE ;
  }

void honeycomb_data_free (HONEYCOMB_DATA *hc)
  {
  exp_array_free (hc->arTraces) ;
  exp_array_free (hc->arHCs) ;
  g_free (hc) ;
  }

static long long unsigned int calculate_honeycomb_value (EXP_ARRAY *bits)
  {
  int icBits = bits->icUsed ;
  int Nix ;
  int exponent = 0 ;
  long long unsigned int value = 0 ;

  if (icBits > sizeof (long long unsigned int) * 8 - 1)
    {
    icBits = sizeof (long long unsigned int) * 8 - 1 ;
    fprintf (stderr, "WARNING! Truncating honeycomb value to %d bits for fear of overflow!\n", icBits) ;
    }

  for (Nix = icBits - 1 ; Nix > -1 ; Nix--)
    {
    if (-1 == exp_array_index_1d (bits, int, Nix))
      return -1 ;
    value |= ((long long unsigned int)exp_array_index_1d (bits, int, Nix)) << exponent ;
    exponent++ ;
    }

  return value ;
  }

/* The array of 6 GdkPoint structures is used as follows:
     1------2
    /        \
   0          3
    \        /
     5------4    */
void calculate_honeycomb_coords (HONEYCOMB_DATA *hc, int icSamples)
  {
  HONEYCOMB *hcCalc = NULL ;
  int Nix ;
  int cxHC = 0 ;
  int cyHC = 0 ;
  int icSlopePixels = 0 ;

  for (Nix = hc->arHCs->icUsed - 1 ; Nix > -1 ; Nix--)
    {
    hcCalc = &(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix)) ;

    hcCalc->pts[0].x = ((((double)(((double)(hcCalc->idxBeg)) * ((double)(hc->graph_data.cxGiven)))) / ((double)(icSamples - 1)))) ;
    hcCalc->pts[3].x = ((((double)(((double)(hcCalc->idxEnd)) * ((double)(hc->graph_data.cxGiven)))) / ((double)(icSamples - 1)))) ;
    hcCalc->pts[0].y =
    hcCalc->pts[3].y = hc->graph_data.cyGiven >> 1 ;

    cxHC = hcCalc->pts[3].x - hcCalc->pts[0].x ;
    cyHC = (hc->graph_data.cyGiven * (1.0 - 2.0 * MIN_MAX_OFFSET)) / 2.0 ;
    if (cxHC < 2.0 * cyHC / tan (HONEYCOMB_ANGLE))
      {
      cyHC = cxHC * tan (HONEYCOMB_ANGLE) ;

      hcCalc->pts[1].x =
      hcCalc->pts[5].x =
      hcCalc->pts[2].x =
      hcCalc->pts[4].x = (hcCalc->pts[3].x + hcCalc->pts[0].x) >> 1 ;
      }
    else
      {
      icSlopePixels = cyHC / tan (HONEYCOMB_ANGLE) ;
      hcCalc->pts[1].x =
      hcCalc->pts[5].x = hcCalc->pts[0].x + icSlopePixels ;
      hcCalc->pts[2].x =
      hcCalc->pts[4].x = hcCalc->pts[3].x - icSlopePixels ;
      }
    hcCalc->pts[1].y =
    hcCalc->pts[2].y = hcCalc->pts[0].y - cyHC ;
    hcCalc->pts[4].y =
    hcCalc->pts[5].y = hcCalc->pts[0].y + cyHC ;
    }

  hc->graph_data.bNeedCalc = FALSE ;
  }

#ifdef GTK_GUI
int calculate_honeycomb_cxWanted (HONEYCOMB_DATA *hc, int icSamples, int base)
  {
  char *psz = NULL ;
  int cxText = 0, cyText = 0 ;

  // Calculate cxWanted
  psz = g_strnfill ((int)ceil (hc->arTraces->icUsed /
    (16 == base ? 4.0 :
     10 == base ? BITS_PER_DEC_DIGIT : 1.0)), '0') ;
  get_string_dimensions (psz, FONT_STRING, &cxText, &cyText) ;
  g_free (psz) ;
  cxText += ceil (cyText * cos (HONEYCOMB_ANGLE)) ;
  return (int)ceil (((((double)(hc->arHCs->icUsed)) * ((double)cxText) * ((double)icSamples))) / ((double)hc->icHCSamples)) ;
  }
#endif /* def GTK_GUI */

void calculate_waveform_coords (WAVEFORM_DATA *wf, int icSamples)
  {
  double dxInc = 0.0, dyInc = 0.0 ;
  GdkPoint pt1 = {0, 0}, pt2 = {0, 0}, pt3 = {0, 0} ;
  int Nix ;
  double dMinTrace, dMaxTrace ;

  if (wf->bStretch)
    tracedata_get_min_max (wf->trace, 0, icSamples - 1, &dMinTrace, &dMaxTrace) ;
  else
    {
    dMinTrace = -1.0 ;
    dMaxTrace =  1.0 ;
    }

  if (dMinTrace == dMaxTrace) return ;

  dxInc = ((double)(wf->graph_data.cxGiven - 1)) / (double)((icSamples)) ;
  dyInc = ((double)((1 - 2 * MIN_MAX_OFFSET) * wf->graph_data.cyGiven - 1)) / (dMaxTrace - dMinTrace) ;

  exp_array_remove_vals (wf->arPoints, 1, 0, wf->arPoints->icUsed) ;

  pt1.x = pt2.x = pt3.x = 0 ;
  pt1.y = pt2.y = pt3.y =
    wf->graph_data.cyGiven -
    ((wf->trace->data[0] - dMinTrace) * dyInc + MIN_MAX_OFFSET * wf->graph_data.cyGiven) ;

  exp_array_insert_vals (wf->arPoints, &pt3, 1, 1, -1) ;

  for (Nix = 1 ; Nix < icSamples ; Nix++)
    {
    pt2.x = pt3.x ;
    pt2.y = pt3.y ;

    pt3.x = Nix * dxInc ;
    pt3.y = wf->graph_data.cyGiven - ((wf->trace->data[Nix] - dMinTrace) * dyInc + MIN_MAX_OFFSET * wf->graph_data.cyGiven) ;

    if (!LineSegmentCanBeSkipped (
      ((double)(pt1.x)), ((double)(pt1.y)),
      ((double)(pt2.x)), ((double)(pt2.y)),
      ((double)(pt3.x)), ((double)(pt3.y)), MAX_SLOPE_DIFF))
      {
      exp_array_insert_vals (wf->arPoints, &pt2, 1, 1, -1) ;
      pt1.x = pt2.x ;
      pt1.y = pt2.y ;
      }
    }

  exp_array_insert_vals (wf->arPoints, &pt3, 1, 1, -1) ;

  wf->graph_data.bNeedCalc = FALSE ;
  }

void waveform_data_free (WAVEFORM_DATA *wf)
  {
  exp_array_free (wf->arPoints) ;
  g_free (wf) ;
  }
