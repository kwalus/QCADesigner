#ifndef _PRINT_H_
#define _PRINT_H_

#include "simulation.h"
#include "print_dialog.h"
#include "gqcell.h"

typedef struct
  {
  print_OP po ;
  /* Units are in points == 1/72 inches */
  double dPointsPerNano ;
  gboolean bPrintOrderOver ;
  gboolean bCenter ;
  gboolean bFit ;
  gboolean *pbPrintedObjs ;
  int icPrintedObjs ;
  int iCXPages ;
  int iCYPages ;
  } print_design_OP ;

typedef struct
  {
  print_OP po ;
  /* Units are in points == 1/72 inches */
  gboolean bPrintOrderOver ;
  gboolean bCenter ;
  gboolean bPrintClr ;
  int iCXPages ;
  int iCYPages ;
  } print_graph_OP ;

typedef void (*PrintFunction) (print_OP *pPO, void *data) ;

void print_world (print_design_OP *pPrintOpts, GQCell *first_cell) ;
void print_graphs (print_graph_OP *pPrintOpts, simulation_data *sim_data) ;

#endif /*_PRINT_H_*/
