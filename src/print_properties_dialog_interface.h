#ifndef _PRINT_PROPERTIES_DIALOG_INTERFACE_H_
#define _PRINT_PROPERTIES_DIALOG_INTERFACE_H_

#include <gtk/gtk.h>

typedef struct
  {
  GtkWidget *dlgPrintProps ;
  GtkAdjustment *adjCXPages ;
  GtkAdjustment *adjCYPages ;
  GtkAdjustment *adjNanoToUnits ;
  GtkWidget *fmFit ;
  GtkWidget *fmPrintOrder ;
  GtkWidget *fmScale ;
  GSList    *grpScaleOpts ;
  GtkWidget *lblNanoIs ;
  GtkWidget *lblPgsTall ;
  GtkWidget *lblPgsWide ;
  GtkWidget *lblPrintOrder ;
  GtkWidget *lblScale ;
  GtkWidget *lstPrintedObjs ;
  GtkWidget *rbFitPages ;
  GtkWidget *rbFixedScale ;
  GtkWidget *scrwPrintedObjs ;
  GtkWidget *spnCXPages ;
  GtkWidget *spnCYPages ;
  GtkWidget *spnNanoToUnits ;
  GtkWidget *table2 ;
  GtkWidget *table3 ;
  GtkWidget *table4 ;
  GtkWidget *tblScale ;
  GtkWidget *tbtnPrintOrder ;
  GtkWidget *vbPrintedObjs ;
  GtkWidget *vpPrintedObjs ;
  GtkWidget *vbScale ;
  GtkWidget *tbtnCenter ;
  GtkWidget *tblCenter ;
  GtkWidget *fmCenter ;
  GtkWidget *lblCenter ;
  GtkWidget **ppPrintedObjs ;
  int icPrintedObjs ;
  } print_properties_D ;

void create_print_design_properties_dialog (print_properties_D *dialog, print_design_OP *pPO) ;

#endif /* _PRINT_PROPERTIES_DIALOG_INTERFACE_H_*/
