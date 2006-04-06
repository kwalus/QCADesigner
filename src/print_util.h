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
// Header for small wrapper functions for opening and   //
// closing print file descriptors based on whether they //
// were process pipes or files.                         //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _PRINT_UTIL_H_
#define _PRINT_UTIL_H_

#include "print.h"

#define PS_RED    "1.00 0.00 0.00"
#define PS_ORANGE "0.83 0.44 0.00"
#define PS_YELLOW "0.66 0.66 0.00"
#define PS_GREEN  "0.00 0.50 0.00"
#define PS_BLUE   "0.21 0.39 0.70"
#define PS_BLACK  "0.00 0.00 0.00"
#define PS_WHITE  "1.00 1.00 1.00"
#define PS_HCFILL "0.90 0.90 0.90"

#define PS_TEXT_PLACEMENT_PREAMBLE \
  "/fontdescent %% font_name font_size fontdescent\n" \
  "  {\n" \
  "  /font_size exch def\n" \
  "  /font_name exch def\n" \
  "\n" \
  "  font_name findfont dup\n" \
  "  /FontMatrix get aload pop pop pop \n" \
  "  /y_scale exch def pop pop pop\n" \
  "  /FontBBox get aload pop pop pop\n" \
  "  y_scale mul exch pop\n" \
  "  font_size mul\n" \
  "  } def\n" \
  "\n" \
  "%% alignment:\n" \
  "%%\n" \
  "%%  x   x   x\n" \
  "%%   0  3  6\n" \
  "%%\n" \
  "%%  x1  x  7x\n" \
  "%%\n" \
  "%%   2  5  8\n" \
  "%%  x   x   x\n" \
  "%%\n" \
  "/txt %% font_name font_size string alignment txt\n" \
  "  {\n" \
  "  /the_align  exch def\n" \
  "  /the_string exch def\n" \
  "  /the_size   exch def\n" \
  "  /the_name   exch def\n" \
  "\n" \
  "  /the_descent the_name the_size fontdescent def\n" \
  "\n" \
  "  the_name findfont the_size scalefont setfont\n" \
  "  /the_width the_string stringwidth pop def\n" \
  "\n" \
  "  the_align 0 ge the_align 2 le and\n" \
  "    {/x_coord 0 def}\n" \
  "    {\n" \
  "    /x_coord the_width -1 mul def\n" \
  "    the_align 3 ge the_align 5 le and\n" \
  "      {/x_coord x_coord 2 div def}\n" \
  "    if\n" \
  "    }\n" \
  "  ifelse\n" \
  "\n" \
  "  /y_coord the_descent -1 mul def\n" \
  "  the_align 0 eq the_align 3 eq or the_align 6 eq or\n" \
  "    {/y_coord y_coord the_size sub def}\n" \
  "    {\n" \
  "    /y_coord y_coord the_size 2 div sub def\n" \
  "    the_align 2 eq the_align 5 eq or the_align 8 eq or\n" \
  "      {/y_coord y_coord the_size 2 div add def}\n" \
  "    if\n" \
  "    }\n" \
  "  ifelse\n" \
  "\n" \
  "  gsave\n" \
  "  x_coord y_coord rmoveto the_string show\n" \
  "  grestore\n" \
  "  } def\n" \
  "\n" \
  "/point {} def\n" \
  "%%/point { gsave currentpoint /y exch def /x exch def newpath x 1.5 add y moveto x y 1.5 0 360 arc stroke grestore } def\n" \
  "\n"

FILE *OpenPrintStream (print_OP *pPO) ;

void PrintMagic (FILE *pfile, gboolean bPortrait, double dPaperCX, double dPaperCY) ;

void PrintTrailer (FILE *pfile, int icPages) ;

void ClosePrintStream  (FILE *pfile, print_OP *pPO) ;

#endif /* _PRINT_UTIL_H_ */
