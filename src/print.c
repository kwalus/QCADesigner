#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "globals.h"
#include "print.h"
#include "cad.h"
#include "stdqcell.h"

void AddQCellToPage (qcell *pqc, qcell ***pppPage, int *picOnPage) ;
void GetPrintRange (print_OP *pPO, double *pdxMin, double *pdyMin, double *pdxMax, double *pdyMax) ;
void PrintPages (print_OP *pPO, qcell ***pppqcPages, int *pcCellsOnPage, int icCells,
  double dxMinNm, double dyMinNm, double dxMaxNm, double dyMaxNm, double dxDiffMinNm, double dyDiffMinNm) ;
void PrintProlog (FILE *pfile, print_OP *pPO, double dxMinNm, double dyMinNm, double dxDiffMinNm,
  double dyDiffMinNm, double dCYPageNm) ;
void PlaceQCellsOnPage (qcell *first_cell, qcell ***pppqcPages, int *pcCellsOnPage, int *picCells, double dPageWidthNm,
  double dPageHeightNm, int iCXPages, int iCYPages, double dXOffsetNm, double dYOffsetNm, gboolean bRowMajor) ;
void PrintSinglePage (FILE *pfile, print_OP *pPO, qcell ***pppqcPage, int *picQC,
  double dCXPageNm, double dCYPageNm, int idx, int idxX, int idxY, double dXMedian, double dYMedian) ;
void PrintSingleCell (FILE *pfile, qcell *pqc, double xOffset, double yOffset, double dXMedian,
  double dYMedian, gboolean bColour) ;
void PrintCellColour (FILE *pfile, qcell *pqc, gboolean bPrintColour) ;
void PrintCellSides (FILE *pfile, qcell *pqc, double xOffset, double yOffset) ;
void PrintCellLabel (FILE *pfile, qcell *pqc, double dXMedian, double dYMedian, double xOffset, double yOffset) ;

void print_world (print_OP *pPO, qcell *first_cell)
  {
  double 
    dxMinNm = 0, dyMinNm = 0, dxMaxNm = 0, dyMaxNm = 0, dxDiffMinNm = 0, dyDiffMinNm = 0,
    dEffPageCXPts = pPO->dPaperWidth - pPO->dLeftMargin - pPO->dRightMargin,
    dEffPageCYPts = pPO->dPaperHeight - pPO->dTopMargin - pPO->dBottomMargin ;
  qcell ***pppqcPages = NULL ;
  int *pcCellsOnPage = NULL ;
  int iPages = 0 ;
  int icCells = 0 ;
  
  GetPrintRange (pPO, &dxMinNm, &dyMinNm, &dxMaxNm, &dyMaxNm) ;
  
  if (pPO->bCenter)
    {
    double dWidthPoints = (dxMaxNm - dxMinNm) * pPO->dPointsPerNano, 
           dHeightPoints = (dyMaxNm - dyMinNm) * pPO->dPointsPerNano ;
    dxDiffMinNm = (((dEffPageCXPts * pPO->iCXPages) - dWidthPoints) / 2) / pPO->dPointsPerNano ;
    dyDiffMinNm = (((dEffPageCYPts * pPO->iCYPages) - dHeightPoints) / 2) / pPO->dPointsPerNano ;
    }
  
  iPages = pPO->iCXPages * pPO->iCYPages ;
  
  if (pPO->pbPrintedObjs[PRINTED_OBJECTS_CELLS])
    {
    int Nix ;
    pppqcPages = malloc (iPages * sizeof (qcell *)) ;
    pcCellsOnPage = malloc (iPages * sizeof (int)) ;
    
    for (Nix = 0 ; Nix < iPages ; Nix++)
      {
      pppqcPages[Nix] = NULL ;
      pcCellsOnPage[Nix] = 0 ;
      }
    
    PlaceQCellsOnPage (first_cell, pppqcPages, pcCellsOnPage, &icCells, dEffPageCXPts / pPO->dPointsPerNano, 
      dEffPageCYPts / pPO->dPointsPerNano, pPO->iCXPages, pPO->iCYPages, dxMinNm - dxDiffMinNm, 
      dyMinNm - dyDiffMinNm, pPO->bPrintOrderOver) ;
    }
  
  PrintPages (pPO, pppqcPages, pcCellsOnPage, icCells, dxMinNm, dyMinNm, dxMaxNm, dyMaxNm, dxDiffMinNm, dyDiffMinNm) ;
  }

void PlaceQCellsOnPage (qcell *first_cell, qcell ***pppqcPages, int *pcCellsOnPage, int *picCells, double dPageWidthNm, 
  double dPageHeightNm, int iCXPages, int iCYPages, double dXOffsetNm, double dYOffsetNm, gboolean bRowMajor)
  {
  qcell *pqc = NULL ;
  int Nix, Nix1 ;
  int idxX1 = -1, idxY1 = -1, idxX2 = -1, idxY2 = 1, idx = -1 ;

  double dCellX1 = -1, dCellY1 = -1, dCellX2 = -1, dCellY2 = -1 ;
  
  *picCells = 0 ;
  
  for (pqc = first_cell ; NULL != pqc ; pqc = pqc->next)
    {
    // This is adding to the pointer and then dereferencing it
    // I suspect you need paranthesis around the *picCells
    *picCells++ ;
    dCellX1 = pqc->x - (pqc->cell_width / 2) ;
    dCellY1 = pqc->y - (pqc->cell_height / 2) ;
    dCellX2 = pqc->x + (pqc->cell_width / 2) ;
    dCellY2 = pqc->y + (pqc->cell_height / 2) ;
    
    idxX1 = (int) ((dCellX1 - dXOffsetNm) / dPageWidthNm) ;
    idxX1 = idxX1 < 0 ? 0 : idxX1 >= iCXPages ? iCXPages - 1 : idxX1 ; /* Paranoia check */
    idxY1 = (int) ((dCellY1 - dYOffsetNm) / dPageHeightNm) ;
    idxY1 = idxY1 < 0 ? 0 : idxY1 >= iCYPages ? iCYPages - 1 : idxY1 ; /* Paranoia check */
    idxX2 = (int) ((dCellX2 - dXOffsetNm) / dPageWidthNm) ;
    idxX2 = idxX2 < 0 ? 0 : idxX2 >= iCXPages ? iCXPages - 1 : idxX2 ; /* Paranoia check */
    idxY2 = (int) ((dCellY2 - dYOffsetNm) / dPageHeightNm) ;
    idxY2 = idxY2 < 0 ? 0 : idxY2 >= iCYPages ? iCYPages - 1 : idxY2 ; /* Paranoia check */
    
    /* Add the cell to every page it appears on */
    for (Nix = idxX1 ; Nix <= idxX2 ; Nix++)
      for (Nix1 = idxY1 ; Nix1 <= idxY2 ; Nix1++)
        {
	idx = bRowMajor ? Nix1 * iCXPages + Nix : Nix * iCYPages + Nix1 ;
	AddQCellToPage (pqc, &pppqcPages[idx], &pcCellsOnPage[idx]) ;
	}
    }
  }

void AddQCellToPage (qcell *pqc, qcell ***pppPage, int *picOnPage)
  {
  (*pppPage) = realloc ((*pppPage), ++(*picOnPage) * sizeof (qcell *)) ;
  (*pppPage)[(*picOnPage) - 1] = pqc ;
  }

void GetPrintRange (print_OP *pPO, double *pdxMin, double *pdyMin, double *pdxMax, double *pdyMax)
  {
  *pdxMin = *pdyMin = *pdxMax = *pdyMax = 0 ;
  
  if (pPO->pbPrintedObjs[PRINTED_OBJECTS_CELLS])
    get_extents (pdxMin, pdyMin, pdxMax, pdyMax) ;

  if (pPO->pbPrintedObjs[PRINTED_OBJECTS_DIE])
    {
    *pdxMin = *pdxMin > 0 ? 0 : *pdxMin ;
    *pdyMin = *pdyMin > 0 ? 0 : *pdyMin ;
    *pdxMax = *pdxMax < subs_width ? subs_width : *pdxMax ;
    *pdyMax = *pdyMax < subs_height ? subs_height : *pdyMax ;
    }
  }

void PrintPages (print_OP *pPO, qcell ***pppqcPages, int *pcCellsOnPage, int icCells,
  double dxMinNm, double dyMinNm, double dxMaxNm, double dyMaxNm, double dxDiffMinNm, double dyDiffMinNm)
  {
  FILE *pfile = NULL ;
  int Nix[2] = {0, 0}, limit[2] = {pPO->iCXPages, pPO->iCYPages}, inner = -1, outer = -1, idx = 0 ;
  double dPageWidthNm = (pPO->dPaperWidth - pPO->dLeftMargin - pPO->dRightMargin) / pPO->dPointsPerNano,
         dPageHeightNm = (pPO->dPaperHeight - pPO->dTopMargin - pPO->dBottomMargin) / pPO->dPointsPerNano,
	 dXMedian = 0, dYMedian = 0 ;
  
  inner = pPO->bPrintOrderOver ? 0 : 1 ;
  outer = pPO->bPrintOrderOver ? 1 : 0 ;
  
  if (pPO->bPrintFile)
    pfile = fopen (pPO->szPrintString, "w") ;
  else
    pfile = popen (pPO->szPrintString, "w") ;
  
  if (NULL == pfile)
    {
    fprintf (stderr, "Failed to open file/command for writing.\n") ;
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
    (int)(pPO->dPaperWidth), (int)(pPO->dPaperHeight),
    0.0, 0.0, pPO->dPaperWidth, pPO->dPaperHeight) ;
  
  PrintProlog (pfile, pPO, dxMinNm, dyMinNm, dxDiffMinNm, dyDiffMinNm, dPageHeightNm) ;
  
  dXMedian = (dxMaxNm + dxMinNm) / 2 ;
  dYMedian = (dyMaxNm + dyMinNm) / 2 ;
  
  for (Nix[0] = 0 ; Nix[0] < limit[outer] ; Nix[0]++)
    for (Nix[1] = 0 ; Nix[1] < limit[inner] ; Nix[1]++)
      {
      PrintSinglePage (pfile, pPO, pppqcPages, pcCellsOnPage, dPageWidthNm, dPageHeightNm, idx, 
        Nix[outer], Nix[inner], dXMedian, dYMedian) ;
      idx++ ;
      }

  fprintf (pfile,
    "%%%%Trailer\n"
    "%%%%Pages: %d\n"
    "%%%%EOF\n",
    pPO->iCXPages * pPO->iCYPages) ;

  if (pPO->bPrintFile)
    fclose (pfile) ;
  else
    pclose (pfile) ;
  }

void PrintProlog (FILE *pfile, print_OP *pPO, double dxMinNm, double dyMinNm, double dxDiffMinNm,
  double dyDiffMinNm, double dCYPageNm)
  {
  fprintf (pfile,
    "%%%%BeginProlog\n"
    "/nm { %f mul } def\n"
    "/nmx { %f sub %f mul %f add %f add} def\n"
    "/nmy { %f sub %f sub -1 mul %f mul %f add %f sub } def\n"
    "/labelfontsize 12 nm def\n"
    "/txtleftcenter   { gsave dup 0 labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txttopcenter    { gsave dup stringwidth pop 2 div -1 mul 0 rmoveto show grestore } def\n"
    "/txtrightcenter  { gsave dup stringwidth exch -1 mul exch pop labelfontsize 2 div -1 mul rmoveto show grestore } def\n"
    "/txtbottomcenter { gsave dup stringwidth exch 2 div -1 mul exch pop labelfontsize -1 mul rmoveto show grestore } def\n"
    "%%%%EndProlog\n",
    pPO->dPointsPerNano,
    dxMinNm, pPO->dPointsPerNano, pPO->dLeftMargin, dxDiffMinNm * pPO->dPointsPerNano,
    dCYPageNm, dyMinNm, pPO->dPointsPerNano, pPO->dTopMargin, dyDiffMinNm * pPO->dPointsPerNano) ;
  }

void PrintSinglePage (FILE *pfile, print_OP *pPO, qcell ***pppqcPage, int *picQC, double dCXPageNm,
  double dCYPageNm, int idx, int idxX, int idxY, double dXMedian, double dYMedian)
  {
  int Nix ;
  double xOffset = dCXPageNm * idxX,
         yOffset = dCYPageNm * idxY ;
  
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
    pPO->dLeftMargin, pPO->dBottomMargin,
    pPO->dLeftMargin, pPO->dPaperHeight - pPO->dTopMargin,
    pPO->dPaperWidth - pPO->dRightMargin, pPO->dPaperHeight - pPO->dTopMargin,
    pPO->dPaperWidth - pPO->dRightMargin, pPO->dBottomMargin) ;

/*
  fprintf (pfile,
    "newpath\n"
    "%lf nmx %lf nmy 12 sub moveto\n"
    "0 24 rlineto\n"
    "%lf nmx 12 sub %lf nmy moveto\n"
    "24 0 rlineto\n"
    "stroke\n\n",
    dXMedian - xOffset, dYMedian - yOffset,
    dXMedian - xOffset, dYMedian - yOffset) ;
*/
  if (pPO->pbPrintedObjs[PRINTED_OBJECTS_CELLS])
    for (Nix = 0 ; Nix < picQC[idx] ; Nix++)
      PrintSingleCell (pfile, pppqcPage[idx][Nix], xOffset, yOffset, dXMedian, dYMedian,
        pPO->pbPrintedObjs[PRINTED_OBJECTS_COLOURS]) ;
  if (pPO->pbPrintedObjs[PRINTED_OBJECTS_DIE])
    {
    fprintf (pfile,
      "gsave\n"
      "2 setlinewidth\n"
      "newpath\n"
      "%f nmx %f nmy moveto\n"
      "%f nmx %f nmy lineto\n"
      "%f nmx %f nmy lineto\n"
      "%f nmx %f nmy lineto\n"
      "%f nmx %f nmy lineto\n"
      "stroke\n"
      "grestore\n",
      0.0 - xOffset, 0.0 - yOffset,
      subs_width - xOffset, 0.0 - yOffset,
      subs_width - xOffset, subs_height - yOffset,
      0.0 - xOffset, subs_height - yOffset,
      0.0 - xOffset, 0.0 - yOffset) ;
    }

  fprintf (pfile,
    "grestore\n"
    "showpage\n"
    "%%%%PageTrailer\n") ;
  }

void PrintCellCorner (FILE *pfile, qcell *pqc, double xOffset, double yOffset, double dArcOffset, int iCorner)
  {
  if (0 == dArcOffset) return ;

  if (pqc->is_input)
    fprintf (pfile, "%f nmx %f nmy %f nm %d %d arc\n",
      pqc->x + (iCorner < 1 || iCorner > 2 ? (-1) : (1)) * (pqc->cell_width * 0.5) - xOffset,
      pqc->y + (iCorner < 2 ? (-1) : (1)) * (pqc->cell_height * 0.5) - yOffset, dArcOffset,
      270 - iCorner * 90, (360 - iCorner * 90) % 360) ;
  else if (pqc->is_output)
    {
    fprintf (pfile, "%f nmx %f nmy %f nm %d %d arcn\n",
      pqc->x + (iCorner < 1 || iCorner > 2 ? (-1) : (1)) * (pqc->cell_width * 0.5) - xOffset +
        dArcOffset * (iCorner >= 1 && iCorner <= 2 ? (-1) : (1)),
      pqc->y + (iCorner < 2 ? (-1) : (1)) * (pqc->cell_height * 0.5) - yOffset +
        dArcOffset * (iCorner >= 2 ? (-1) : (1)), dArcOffset,
      (540 - iCorner * 90) % 360, (450 - iCorner * 90) % 360) ;
    }
  else if (pqc->is_fixed)
    fprintf (pfile, "%f %f lineto\n",
      pqc->x + (iCorner < 1 || iCorner > 2 ? (-1) : (1)) * (pqc->cell_width * 0.5) - xOffset +
        dArcOffset * (iCorner < 2 ? (1) : (-1)),
      pqc->y + (iCorner < 2 ? (-1) : (1)) * (pqc->cell_height * 0.5) - yOffset +
      dArcOffset * (iCorner >= 1 && iCorner <= 2 ? (1) : (-1))) ;
  }

void PrintCellSides (FILE *pfile, qcell *pqc, double xOffset, double yOffset)
  {
  double dArcOffset = 0 ;

//  dArcOffset = pqc->is_input || pqc->is_output || pqc->is_fixed ? 3 : 0 ;

  fprintf (pfile, "%f nmx %f nmy moveto\n",
    (pqc->x - (pqc->cell_width * 0.5)) - xOffset + dArcOffset,
    (pqc->y - (pqc->cell_height * 0.5)) - yOffset) ;
  fprintf (pfile, "%f nmx %f nmy lineto\n",
    (pqc->x + (pqc->cell_width * 0.5)) - xOffset - 2 * dArcOffset,
    (pqc->y - (pqc->cell_height * 0.5)) - yOffset) ;
//  PrintCellCorner (pfile, pqc, xOffset, yOffset, dArcOffset, 1) ;
  fprintf (pfile, "%f nmx %f nmy lineto\n",
    (pqc->x + (pqc->cell_width * 0.5)) - xOffset,
    (pqc->y + (pqc->cell_height * 0.5)) - yOffset - 2 * dArcOffset) ;
//  PrintCellCorner (pfile, pqc, xOffset, yOffset, dArcOffset, 2) ;
  fprintf (pfile, "%f nmx %f nmy lineto\n",
    (pqc->x - (pqc->cell_width * 0.5)) - xOffset + dArcOffset,
    (pqc->y + (pqc->cell_height * 0.5)) - yOffset) ;
//  PrintCellCorner (pfile, pqc, xOffset, yOffset, dArcOffset, 3) ;
  fprintf (pfile, "%f nmx %f nmy lineto\n",
    (pqc->x - (pqc->cell_width * 0.5)) - xOffset,
    (pqc->y - (pqc->cell_height * 0.5)) - yOffset + dArcOffset) ;
//  PrintCellCorner (pfile, pqc, xOffset, yOffset, dArcOffset, 0) ;
  }

void PrintSingleCell (FILE *pfile, qcell *pqc, double xOffset, double yOffset, double dXMedian,
  double dYMedian, gboolean bColour)
  {
  int Nix ;

  fprintf (pfile, "%%Begin Cell\n") ;

  /* Cell outline */
  fprintf (pfile, "newpath\n") ;
  PrintCellSides (pfile, pqc, xOffset, yOffset) ;
  fprintf (pfile, "stroke\n") ;

  /* Cell shading */
  fprintf (pfile,
    "gsave\n"
    "newpath\n") ;
  PrintCellSides (pfile, pqc, xOffset, yOffset) ;

  for (Nix = 0 ; Nix < pqc->number_of_dots ; Nix++)
    {
    fprintf (pfile,
      "%f nmx %f nmy moveto\n"
      "%f nmx %f nmy %f nm 0 360 arc\n",
      pqc->cell_dots[Nix].x + pqc->cell_dots[Nix].diameter / 2.0 - xOffset,
      pqc->cell_dots[Nix].y - yOffset,
      pqc->cell_dots[Nix].x - xOffset, pqc->cell_dots[Nix].y - yOffset, pqc->cell_dots[Nix].diameter / 2.0) ;
    }
  PrintCellColour (pfile, pqc, bColour) ;
  fprintf (pfile,
    "fill\n"
    "grestore\n") ;
  PrintCellLabel (pfile, pqc, dXMedian, dYMedian, xOffset, yOffset) ;

  for (Nix = 0 ; Nix < pqc->number_of_dots ; Nix++)
    {
    fprintf (pfile,
      "newpath\n" /* dot outline */
      "%f nmx %f nmy %f nm 0 360 arc\n"
      "closepath stroke\n\n"
      "newpath\n" /* dot charge */
      "%f nmx %f nmy %f nm 0 360 arc\n"
      "closepath fill\n\n",
      pqc->cell_dots[Nix].x - xOffset, pqc->cell_dots[Nix].y - yOffset, pqc->cell_dots[Nix].diameter / 2.0,
      pqc->cell_dots[Nix].x - xOffset, pqc->cell_dots[Nix].y - yOffset,
      (pqc->cell_dots[Nix].diameter * pqc->cell_dots[Nix].charge / QCHARGE) / 2.0) ;
    }
  fprintf (pfile, "%%End Cell\n") ;
  }

void PrintCellColour (FILE *pfile, qcell *pqc, gboolean bPrintColour)
  {
  float clr[4][3] = {
    {0.0, 0.5, 0.0}, /* dark green */
    {0.5, 0.0, 0.5}, /* dark purple */
    {0.0, 0.5, 0.5}, /* turquoise */
    {1.0, 1.0, 1.0}  /* white */
    } ;
  float gray[4] = {0.15, 0.50, 0.85, 1.00} ; /* equivalent gray values in lieu of colours */

  if (!bPrintColour)
    fprintf (pfile, "%f setgray fill\n", gray[pqc->clock]) ;
  else
    {
    if (pqc->is_fixed)
      fprintf (pfile, "0.83 0.44 0 setrgbcolor\n") ; /* dark orange */
    else if (pqc->is_input)
      fprintf (pfile, "0.21 0.39 0.70 setrgbcolor\n") ; /* dark azure blue */
    else if (pqc->is_output)
      fprintf (pfile, "0.66 0.66 0 setrgbcolor\n") ; /* maroonish yellow */
    else
      fprintf (pfile, "%f %f %f setrgbcolor\n", clr[pqc->clock][0], clr[pqc->clock][1], clr[pqc->clock][2]) ;
    }
  }

void PrintCellLabel (FILE *pfile, qcell *pqc, double dXMedian, double dYMedian, double xOffset, double yOffset)
  {
  char szLabel[256] = "" ;

  if (!(pqc->is_fixed || pqc->is_input || pqc->is_output)) return ;

  fprintf (pfile, "(Courier) findfont labelfontsize scalefont setfont\n") ;

  if (pqc->is_fixed)
    g_snprintf (szLabel, 256, "(%1.2f)", calculate_polarization (pqc)) ;
  else
    g_snprintf (szLabel, 256, "(%s)", pqc->label) ;

  if (fabs (pqc->x - dXMedian) > fabs (pqc->y - dYMedian))
    {
    fprintf (pfile,
      "%f nmx %f nmy moveto\n"
      "%s %s\n",
      pqc->x + (2 + pqc->cell_width / 2) * (dXMedian > pqc->x ? -1 : 1) - xOffset, pqc->y - yOffset,
      szLabel, (dXMedian > pqc->x ? "txtrightcenter" : "txtleftcenter")) ;
    }
  else
    {
    fprintf (pfile,
      "%f nmx %f nmy 2 add moveto\n"
      "%s %s\n",
      pqc->x - xOffset, pqc->y + (2 + pqc->cell_height / 2) * (dYMedian > pqc->y ? -1 : 1) - yOffset,
      szLabel, (dYMedian > pqc->y ? "txttopcenter" : "txtbottomcenter")) ;
    }
  }
