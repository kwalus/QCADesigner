#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "print.h"
#include "print_util.h"
#include "simulation.h"
#include "global_consts.h"

#define GAP_PERCENT 0.01
#define TRACE_GAP_PERCENT 0.005
#define FONT_SIZE 12 /* points */
#define MAX_SLOPE_DIFF 0.1

#define OBJ_TYPE_PATH 0

#define DBG_PG(s)

typedef struct
  {
  double x ;
  double y ;
  } DPOINT ;

typedef struct
  {
  int icObjects ;
  gchar **ppObjects ;
  } PAGEDATA ;

typedef struct
  {
  int icPages ;
  PAGEDATA *pdPages ;
  } PAGES ;

static void SimDataToPageData (PAGES *pPages, print_graph_OP *pPO, simulation_data *sim_data) ;
static void PlaceSingleTrace (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, int idx, int icVisibleTraces, int icSamples) ;
static void PrintSingleGraphPage (print_graph_OP *pPO, FILE *pfile, PAGEDATA *pPage, int idx) ;
static int GetPageIndex (double dCoord, int icPages, double dEffectiveLength) ;
static void CreateTrace (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, double dxMin, double dyMin, double dxMax, double dyMax, int icSamples) ;
static inline gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2) ;

void print_graphs (print_graph_OP *pPrintOpts, simulation_data *sim_data)
  {
  int Nix, Nix1 ;
  FILE *pfile = NULL ;
  PAGES pg = {0, NULL} ;
  
  if (NULL == (pfile = OpenPrintStream ((print_OP *)pPrintOpts)))
    {
    fprintf (stderr, "Failed to open print stream.\n") ;
    return ;
    }

  SimDataToPageData (&pg, pPrintOpts, sim_data) ;

  if (NULL == pg.pdPages)
    {
    fprintf (stderr, "Failed to create pages.\n") ;
    fclose (pfile) ;
    return ;
    }
  
  fprintf (pfile,
    "%%!PS-Adobe 3.0\n"
    "%%%%Pages: (atend)\n"
    "%%%%BoundingBox: 0 0 %d %d\n"
    "%%%%HiResBoundingBox: %f %f %f %f\n"
    "%%........................................................\n"
    "%%%%Creator: QCADesigner\n"
    "%%%%EndComments\n",
    (int)(pPrintOpts->po.dPaperCX), (int)(pPrintOpts->po.dPaperCY),
    0.0, 0.0, pPrintOpts->po.dPaperCX, pPrintOpts->po.dPaperCY) ;
  
  /* Print prolog at this point, if any */
  fprintf (pfile,
    "%%%%BeginProlog\n"
    "/labelfontsize %d def\n"
    "/txtlt { gsave dup 0 -1 labelfontsize mul rmoveto show grestore } def\n"
    "/txtlm { gsave dup 0 labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtlb { gsave dup 0 0 rmoveto show grestore } def\n"
    "/txtct { gsave dup stringwidth exch 2 div -1 mul exch pop labelfontsize -1 mul rmoveto show grestore } def\n"
    "/txtcm { gsave dup stringwidth exch 2 div -1 mul exch pop labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtcb { gsave dup stringwidth pop 2 div -1 mul 0 rmoveto show grestore } def\n"
    "/txtrt { gsave dup stringwidth exch -1 mul exch pop labelfontsize -1 mul rmoveto show grestore } def\n"
    "/txtrm { gsave dup stringwidth exch -1 mul exch pop labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtrt { gsave dup stringwidth exch -1 mul exch pop 0 rmoveto show grestore } def\n"
    "/point {} def\n"
    "%%/point { -3 -3 rmoveto 6 6 rlineto -6 0 rmoveto 6 -6 rlineto -3 3 rmoveto } def\n"
    "%%%%EndProlog\n", FONT_SIZE) ;

  for (Nix = 0 ; Nix < pg.icPages ; Nix++)
    {
    PrintSingleGraphPage (pPrintOpts, pfile, &(pg.pdPages[Nix]), Nix) ;
    for (Nix1 = 0 ; Nix1 < pg.pdPages[Nix].icObjects ; Nix1++)
      g_free (pg.pdPages[Nix].ppObjects[Nix1]) ;
    free (pg.pdPages[Nix].ppObjects) ;
    pg.pdPages[Nix].ppObjects = NULL ;
    pg.pdPages[Nix].icObjects = 0 ;
    }
  
  fprintf (pfile,
    "%%%%Trailer\n"
    "%%%%Pages: %d\n"
    "%%%%EOF\n",
    pg.icPages) ;

  fclose (pfile) ;
  }

static void PrintSingleGraphPage (print_graph_OP *pPO, FILE *pfile, PAGEDATA *pPage, int idx)
  {
  int Nix ;
  
  fprintf (pfile,
    "%%%%Page: %d %d\n"
    "gsave\n"
    "newpath\n" /* The margins */
    "%f %f moveto\n"
    "%f %f lineto\n"
    "%f %f lineto\n"
    "%f %f lineto\n"
    "closepath eoclip\n\n",
    idx + 1, idx + 1,
    pPO->po.dLMargin, pPO->po.dBMargin,
    pPO->po.dLMargin, pPO->po.dPaperCY - pPO->po.dTMargin,
    pPO->po.dPaperCX - pPO->po.dRMargin, pPO->po.dPaperCY - pPO->po.dTMargin,
    pPO->po.dPaperCX - pPO->po.dRMargin, pPO->po.dBMargin) ;

  DBG_PG (fprintf (stderr, "There are %d objects on page %d\n", pPage->icObjects, idx)) ;

  for (Nix = 0 ; Nix < pPage->icObjects ; Nix++)
    fprintf (pfile, "%s", pPage->ppObjects[Nix]) ;

  fprintf (pfile,
    "grestore\n"
    "showpage\n"
    "%%%%PageTrailer\n") ;
  }

static void SimDataToPageData (PAGES *pPages, print_graph_OP *pPO, simulation_data *sim_data)
  {
  int idxTrace = -1 ;
  int icVisibleTraces = 0 ;
  int Nix, xIdxTitle, yIdxTitle, idxPg ;
  double 
    dcxEffective = pPO->iCXPages * (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin),
    dcyEffective = pPO->iCYPages * (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin),
    dxTitle, dyTitle ;
  
  pPages->icPages = pPO->iCXPages * pPO->iCYPages ;

  pPages->pdPages = malloc (pPages->icPages * sizeof (PAGEDATA)) ;
  memset (pPages->pdPages, 0, pPages->icPages * sizeof (PAGEDATA)) ;
  
  for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
    if (sim_data->trace[Nix].drawtrace)
      icVisibleTraces++ ;
  for (Nix = 0 ; Nix < 4 ; Nix++)
    if (sim_data->clock_data[Nix].drawtrace)
      icVisibleTraces++ ;
  
  if (!icVisibleTraces) return ;
  
  /* Place objects */
  
  for (Nix = 0 ; Nix < sim_data->number_of_traces ; Nix++)
    if (sim_data->trace[Nix].drawtrace)
      PlaceSingleTrace (pPO, pPages, &(sim_data->trace[Nix]), ++idxTrace, icVisibleTraces, sim_data->number_samples) ;
  
  for (Nix = 0 ; Nix < 4 ; Nix++)
    if (sim_data->trace[Nix].drawtrace)
      PlaceSingleTrace (pPO, pPages, &(sim_data->clock_data[Nix]), ++idxTrace, icVisibleTraces, sim_data->number_samples) ;

  /* Graph title ("Simulation Results")*/
  dxTitle = dcxEffective / 2 ;
  dyTitle = 0 ;
  xIdxTitle = GetPageIndex (dxTitle, pPO->iCXPages, dcxEffective) ;
  yIdxTitle = GetPageIndex (dyTitle, pPO->iCYPages, dcyEffective) ;
  idxPg = pPO->bPrintOrderOver ? yIdxTitle * pPO->iCXPages + xIdxTitle : xIdxTitle * pPO->iCYPages + yIdxTitle ;
  pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects,
    ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
  pPages->pdPages[idxPg].ppObjects[pPages->pdPages[idxPg].icObjects - 1] =
    g_strdup_printf (
      "gsave\n"
      "newpath\n"
      "(Courier) findfont labelfontsize scalefont setfont\n"
      "%lf %lf moveto\n"
      "(Simulation Results) txtct\n"
      "stroke\n"
      "grestore\n",
      dxTitle - xIdxTitle * (dcxEffective / pPO->iCXPages) + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dyTitle - yIdxTitle * (dcyEffective / pPO->iCYPages) + pPO->po.dTMargin)) ;
  }

static void PlaceSingleTrace (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, int idx, int icVisibleTraces, int icSamples)
  {
  double 
    dcxEffective = pPO->iCXPages * (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin),
    dcyEffective = pPO->iCYPages * (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin),
    dcyTrace = (dcyEffective - (GAP_PERCENT * dcyEffective * icVisibleTraces)) / icVisibleTraces - 
      /*Graph title ("Simulation Results"):*/((FONT_SIZE + GAP_PERCENT * dcyEffective) / icVisibleTraces),
    dcxTraceHeader = 108.0, /* points */
    dyTraceTop = idx * (GAP_PERCENT * dcyEffective + dcyTrace) + (FONT_SIZE + GAP_PERCENT * dcyEffective),
    dxMin = -1, dyMin = -1, dxMax = -1, dyMax = -1,
    dxMinPg, dyMinPg, dxMaxPg, dyMaxPg, dTraceMaxVal = 0, dTraceMinVal = 0 ;
  int idxXPgMin = -1, idxYPgMin = -1, idxXPgMax = -1, idxYPgMax = -1, Nix, Nix1, idxPg = -1, idxObj ;
  char *pszClr =
    RED     == ptd->trace_color ? PS_RED    :
    YELLOW  == ptd->trace_color ? PS_YELLOW :
    BLUE    == ptd->trace_color ? PS_BLUE   : PS_BLACK ;

  
  DBG_PG (fprintf (stderr, "Entering PlaceSingleTrace\n")) ;
  
  /* Trace header box */
  dxMin = 0 ;
  dyMin = dyTraceTop ;
  dxMax = dcxTraceHeader ;
  dyMax = dyTraceTop + dcyTrace ;
  
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;
  
  DBG_PG (fprintf (stderr, "trace %d (header) ranges from (%d,%d) to (%d,%d)\n", 
    idx, idxXPgMin, idxYPgMin, idxXPgMax, idxYPgMax)) ;
  
  /* Place onto every page it appears on */
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;
      idxObj = pPages->pdPages[idxPg].icObjects ;
      
      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMaxPg = pPO->po.dPaperCY -
               (dyMax - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      
      pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
        ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
      pPages->pdPages[idxPg].ppObjects[idxObj] =
        g_strdup_printf (
          "gsave\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto\n"
          "%lf %lf lineto\n"
          "%lf %lf lineto\n"
          "%lf %lf lineto\n"
          "closepath\n"
          "stroke\n"
          "grestore\n",
          pPO->bPrintClr ? PS_GREEN : "0.00", 
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dxMinPg, dyMinPg, dxMaxPg, dyMinPg, dxMaxPg, dyMaxPg, dxMinPg, dyMaxPg) ;
      }

  /* Trace body box */
  dxMin = dcxTraceHeader + dcxEffective * GAP_PERCENT ;
  dxMax = dcxEffective ;
  /* dyMin = same as above */
  /* dyMax = same as above */

  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;
  
  DBG_PG (fprintf (stderr, "trace %d (body) ranges from (%d,%d) to (%d,%d)\n", 
    idx, idxXPgMin, idxYPgMin, idxXPgMax, idxYPgMax)) ;
  
  /* Place onto every page it appears on */
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;
      idxObj = pPages->pdPages[idxPg].icObjects ;
      
      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMaxPg = pPO->po.dPaperCY -
               (dyMax - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      
      pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects,
        ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
      pPages->pdPages[idxPg].ppObjects[idxObj] =
        g_strdup_printf (
          "gsave\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto\n"
          "%lf %lf lineto\n"
          "%lf %lf lineto\n"
          "%lf %lf lineto\n"
          "closepath\n"
          "stroke\n"
          "grestore\n",
          pPO->bPrintClr ? PS_GREEN : "0.00", 
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dxMinPg, dyMinPg, dxMaxPg, dyMinPg, dxMaxPg, dyMaxPg, dxMinPg, dyMaxPg) ;
      }
  
  /* Trace max */
  dxMin = dcxTraceHeader + dcxEffective * (GAP_PERCENT + TRACE_GAP_PERCENT) ;
  dyMin = dyTraceTop + dcyEffective * TRACE_GAP_PERCENT ;
  dxMax = dcxEffective * (1 - TRACE_GAP_PERCENT) ;
  /* dyMax = same as dyMin */ ;
  
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;
  
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;
      idxObj = pPages->pdPages[idxPg].icObjects ;
      
      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      /* dyMaxPg = same as dyMinPg */
      
      pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
        ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
      pPages->pdPages[idxPg].ppObjects[idxObj] =
        g_strdup_printf (
          "gsave\n"
          "[2 2 2 2] 0 setdash\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto\n"
          "%lf %lf lineto\n"
          "closepath\n"
          "stroke\n"
          "grestore\n",
          pPO->bPrintClr ? PS_GREEN : "0.00",
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dxMinPg, dyMinPg, dxMaxPg, dyMinPg) ;
      }
  
  /* Trace min */
  dxMin = dcxTraceHeader + dcxEffective * (GAP_PERCENT + TRACE_GAP_PERCENT) ;
  dyMin = dyTraceTop + dcyTrace - dcyEffective * TRACE_GAP_PERCENT ;
  dxMax = dcxEffective * (1 - TRACE_GAP_PERCENT) ;
  /* dyMax = same as dyMin */ ;
  
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;
  
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;
      idxObj = pPages->pdPages[idxPg].icObjects ;
      
      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      /* dyMaxPg = same as dyMinPg */
      
      pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
        ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
      pPages->pdPages[idxPg].ppObjects[idxObj] =
        g_strdup_printf (
          "gsave\n"
          "[2 2 2 2] 0 setdash\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto\n"
          "%lf %lf lineto\n"
          "closepath\n"
          "stroke\n"
          "grestore\n",
          pPO->bPrintClr ? PS_GREEN : "0.00",
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dxMinPg, dyMinPg, dxMaxPg, dyMinPg) ;
      }
  
  CreateTrace (pPO, pPages, ptd, dxMin, dyMin, dxMax, dyTraceTop + dcyEffective * TRACE_GAP_PERCENT, icSamples) ;

  /* Trace mid */
  dxMin = dcxTraceHeader + dcxEffective * (GAP_PERCENT + TRACE_GAP_PERCENT) ;
  dyMin = dyTraceTop +  dcyTrace / 2 ;
  dxMax = dcxEffective * (1 - TRACE_GAP_PERCENT) ;
  /* dyMax = same as dyMin */ ;
  
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;
  idxXPgMax = GetPageIndex (dxMax, pPO->iCXPages, dcxEffective) ;
  idxYPgMax = GetPageIndex (dyMax, pPO->iCYPages, dcyEffective) ;
  
  for (Nix = idxYPgMin ; Nix <= idxYPgMax ; Nix++)
    for (Nix1 = idxXPgMin ; Nix1 <= idxXPgMax ; Nix1++)
      {
      idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + Nix1 : Nix1 * pPO->iCYPages + Nix ;
      idxObj = pPages->pdPages[idxPg].icObjects ;
      
      dxMinPg = dxMin - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      dyMinPg = pPO->po.dPaperCY -
               (dyMin - (Nix  * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin)  ;
      dxMaxPg = dxMax - (Nix1 * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin ;
      /* dyMaxPg = same as dyMinPg */
      
      pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
        ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
      pPages->pdPages[idxPg].ppObjects[idxObj] =
        g_strdup_printf (
          "gsave\n"
          "[2 2 2 2] 0 setdash\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto\n"
          "%lf %lf lineto\n"
          "closepath\n"
          "stroke\n"
          "grestore\n",
          pPO->bPrintClr ? PS_GREEN : "0.00",
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dxMinPg, dyMinPg, dxMaxPg, dyMinPg) ;
      }

  /* Filling in the trace header */
  tracedata_get_min_max (ptd, 0, icSamples -1, &dTraceMinVal, &dTraceMaxVal) ;

  dxMin = TRACE_GAP_PERCENT * dcxEffective ;
  dyMin = dyTraceTop + TRACE_GAP_PERCENT * dcyEffective ;
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;

  idxPg = pPO->bPrintOrderOver ? idxYPgMin * pPO->iCXPages + idxXPgMin : idxXPgMin * pPO->iCYPages + idxYPgMin ;
  idxObj = pPages->pdPages[idxPg].icObjects ;
  pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
    ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
  pPages->pdPages[idxPg].ppObjects[idxObj] =
    g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "newpath\n"
      "%lf %lf moveto\n"
      "(Courier) findfont labelfontsize scalefont setfont\n"
      "(max: %1.2lf) txtlt\n"
      "stroke\n"
      "grestore\n",
      pPO->bPrintClr ? pszClr : "0.00",
      pPO->bPrintClr ? "setrgbcolor" : "setgray",
      dxMin - (idxXPgMin * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dyMin - (idxYPgMin * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin),
      dTraceMaxVal) ;

  dxMin = TRACE_GAP_PERCENT * dcxEffective ;
  dyMin = dyTraceTop + dcyTrace - TRACE_GAP_PERCENT * dcyEffective ;
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;

  idxPg = pPO->bPrintOrderOver ? idxYPgMin * pPO->iCXPages + idxXPgMin : idxXPgMin * pPO->iCYPages + idxYPgMin ;
  idxObj = pPages->pdPages[idxPg].icObjects ;
  pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
    ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
  pPages->pdPages[idxPg].ppObjects[idxObj] =
    g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "newpath\n"
      "%lf %lf moveto\n"
      "(Courier) findfont labelfontsize scalefont setfont\n"
      "(min: %1.2lf) txtlb\n"
      "stroke\n"
      "grestore\n",
      pPO->bPrintClr ? PS_GREEN : "0.00",
      pPO->bPrintClr ? "setrgbcolor" : "setgray",
      dxMin - (idxXPgMin * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dyMin - (idxYPgMin * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin),
      dTraceMinVal) ;

  dxMin = TRACE_GAP_PERCENT * dcxEffective ;
  dyMin = dyTraceTop +  dcyTrace / 2 ;
  idxXPgMin = GetPageIndex (dxMin, pPO->iCXPages, dcxEffective) ;
  idxYPgMin = GetPageIndex (dyMin, pPO->iCYPages, dcyEffective) ;

  idxPg = pPO->bPrintOrderOver ? idxYPgMin * pPO->iCXPages + idxXPgMin : idxXPgMin * pPO->iCYPages + idxYPgMin ;
  idxObj = pPages->pdPages[idxPg].icObjects ;
  pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects, 
    ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
  pPages->pdPages[idxPg].ppObjects[idxObj] =
    g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "newpath\n"
      "%lf %lf moveto\n"
      "(Courier) findfont labelfontsize scalefont setfont\n"
      "(%s) txtlm\n"
      "stroke\n"
      "grestore\n",
      pPO->bPrintClr ? pszClr : "0.00",
      pPO->bPrintClr ? "setrgbcolor" : "setgray",
      dxMin - (idxXPgMin * dcxEffective) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dyMin - (idxYPgMin * dcyEffective) / pPO->iCYPages + pPO->po.dTMargin),
      ptd->data_labels) ;
  }

static int GetPageIndex (double dCoord, int icPages, double dEffectiveLength)
  {
  double dIdx = dCoord * icPages / dEffectiveLength ;
  if (fabs (((double)(int)dIdx) - dIdx) < 0.00001) dIdx-- ;
  return CLAMP ((int)dIdx, 0, icPages - 1) ;
  }

static void CreateTrace (print_graph_OP *pPO, PAGES *pPages, struct TRACEDATA *ptd, double dxMin, double dyMin, double dxMax, double dyMax, int icSamples)
  {
  /*
  The box the graph must fit into is given in effective
  coordinates as follows:

                                           (dxMax, dyMin)
    +-------------------------------------------+
    |        ___                       __       |
    |       /   \__                   /  \      |
    |  ____/       \                 /    \_....|
    | /             \      _________/           |
    |/               \____/                     |
    +-------------------------------------------+
  (dxMin, dyMax)
  */
  double
    dcx = pPO->iCXPages * (pPO->po.dPaperCX - pPO->po.dLMargin - pPO->po.dRMargin),
    dcy = pPO->iCYPages * (pPO->po.dPaperCY - pPO->po.dTMargin - pPO->po.dBMargin) ;
  int 
    xIdxMin = GetPageIndex (dxMin, pPO->iCXPages, dcx), 
    yIdxMin = GetPageIndex (dyMin, pPO->iCYPages, dcy), 
    yIdxMax = GetPageIndex (dyMax, pPO->iCYPages, dcy),
    Nix, Nix1, xIdx = xIdxMin, xIdxNew, idxPg, idxStr ;
  double
    dTraceMin, dTraceMax, dYScale, dxInc,
    dx0, dy0, dx1, dy1, dx2, dy2 ; /* The 2 line segments (dx,dy)0 -> (dx,dy)1 -> (dx,dy)2 */
  gchar *psz = NULL, *pszNew = NULL, *pszClr = NULL ;
  
  pszClr =
    RED     == ptd->trace_color ? PS_RED    :
    YELLOW  == ptd->trace_color ? PS_YELLOW :
    BLUE    == ptd->trace_color ? PS_BLUE   : PS_BLACK ;
  
  tracedata_get_min_max (ptd, 0, icSamples - 1, &dTraceMin, &dTraceMax) ;
  
  if (dTraceMax == dTraceMin) return ;
  if (icSamples < 2) return ;
  
  dxInc = (dxMax - dxMin) / icSamples ;
  
  DBG_PG (fprintf (stderr, "dxMax = %lf, dxMin = %lf, dxInc = %lf\n", dxMax, dxMin, dxInc)) ;
  
  dYScale = (dyMin - dyMax) / (dTraceMax - dTraceMin) ;
  
  DBG_PG (fprintf (stderr, "dyMax = %lf, dyMin = %lf, dTraceMax = %lf, dTraceMin = %lf, dYScale = %lf\n",
    dyMax, dyMin, dTraceMax, dTraceMin, dYScale)) ;
  
  for (Nix = yIdxMin ; Nix <= yIdxMax ; Nix++)
    {
    dx0 = dx1 = dx2 = dxMin ;
    dy0 = dy1 = dy2 = dyMin - (ptd->data[0] - dTraceMin) * dYScale ;
    psz = g_strdup_printf (
      "gsave\n"
      "%s %s\n"
      "newpath\n"
      "%lf %lf moveto point\n",
      pPO->bPrintClr ? pszClr : "0.00",
      pPO->bPrintClr ? "setrgbcolor" : "setgray",
      dx0 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy0 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
    for (Nix1 = 1 ; Nix1 < icSamples ; Nix1++)
      {
      dx2 = dx2 + dxInc ;
      dy2 = dyMin - (ptd->data[Nix1] - dTraceMin) * dYScale ;
      
      /* If we cross over onto a new page, finish off one segment, change pages, and start another */
      if (xIdx != (xIdxNew = GetPageIndex (dx2, pPO->iCXPages, dcx)))
        {
        pszNew = g_strdup_printf (
          "%s"
          "%lf %lf lineto point\n"
          "%lf %lf lineto point\n"
          "stroke\n"
          "grestore\n",
          psz,
          dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
          dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
        g_free (psz) ;
        pszNew = psz ;
        
        idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + xIdx : xIdx * pPO->iCYPages + Nix ;
        idxStr = pPages->pdPages[idxPg].icObjects ;
        pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects,
          ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
        pPages->pdPages[idxPg].ppObjects[idxStr] = psz ;
        
        xIdx = xIdxNew ;
        
        psz = g_strdup_printf (
          "gsave\n"
          "%s %s\n"
          "newpath\n"
          "%lf %lf moveto point\n"
          "%lf %lf lineto point\n",
          pPO->bPrintClr ? pszClr : "0.00",
          pPO->bPrintClr ? "setrgbcolor" : "setgray",
          dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
          dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
          pPO->po.dPaperCY -
         (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
        dx0 = dx1 = dx2 ;
        dy0 = dy1 = dy2 ;
        }

      if (LineSegmentCanBeSkipped (dx0, dy0, dx1, dy1, dx2, dy2))
        {
        dx1 = dx2 ;
        dy1 = dy2 ;
        continue ;
        }
      pszNew = g_strdup_printf (
        "%s"
        "%lf %lf lineto point\n",
        psz,
      dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;

      g_free (psz) ;
      psz = pszNew ;

      dx0 = dx1 ;
      dy0 = dy1 ;
      dx1 = dx2 ;
      dy1 = dy2 ;
      }
    pszNew = g_strdup_printf (
      "%s"
      "%lf %lf lineto point\n"
      "%lf %lf lineto point\n"
      "stroke\n"
      "grestore\n",
      psz,
      dx1 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy1 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin),
      dx2 - (xIdx * dcx) / pPO->iCXPages + pPO->po.dLMargin,
      pPO->po.dPaperCY -
     (dy2 - (Nix  * dcy) / pPO->iCYPages + pPO->po.dTMargin)) ;
    
    g_free (psz) ;
    psz = pszNew ;
    
    idxPg = pPO->bPrintOrderOver ? Nix * pPO->iCXPages + xIdx : xIdx * pPO->iCYPages + Nix ;
    idxStr = pPages->pdPages[idxPg].icObjects ;
    pPages->pdPages[idxPg].ppObjects = realloc (pPages->pdPages[idxPg].ppObjects,
      ++(pPages->pdPages[idxPg].icObjects) * sizeof (gchar *)) ;
    pPages->pdPages[idxPg].ppObjects[idxStr] = psz ;
    }
  }

static inline gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2)
  {
  if (dx0 == dx1 && dx1 == dx2) return TRUE ;
  if (dy0 == dy1 && dy1 == dy2) return TRUE ;
  
  DBG_PG (fprintf (stderr, "slope diff: %lf\n", fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)))) ;
  
  return (fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)) < MAX_SLOPE_DIFF) ;
  }
