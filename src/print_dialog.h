#ifndef _PRINT_DIALOG_H_
#define _PRINT_DIALOG_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
  {
  PD_UNITS_CENTIS,
  PD_UNITS_INCHES,
  PD_UNITS_POINTS
  } PrintDialogUnits ;

typedef struct
  {
  double dPaperCX ;
  double dPaperCY ;
  double dLMargin ;
  double dTMargin ;
  double dRMargin ;
  double dBMargin ;
  gboolean bPrintFile ;
  char *pszPrintString ;
  } print_OP ;

typedef struct PrintDialog
  {
  GtkDialog dlg ;

  GtkWidget *optUnits ;
  GtkWidget *mnuiCentis ;
  GtkWidget *mnuiInches ;
  GtkWidget *mnuiPoints ;
  GtkWidget *mnuiCurrent ;

  GtkWidget *nbPropPages ;
  
  GtkWidget *rbPrintFile ;
  GtkWidget *fmFileSelect ;
  GtkWidget *txtFileSelect ;
  GtkWidget *btnFileSelect ;
  GtkWidget *lblFileSelect ;
  GtkWidget *rbPrintPipe ;
  GtkWidget *fmPipeSelect ;
  GtkWidget *txtPipeSelect ;
  GtkWidget *btnPipeSelect ;
  GtkWidget *lblPipeSelect ;
  GtkWidget *lblPipeSelectBlurb ;
  
  GtkWidget *optPaperSize ;
  GtkWidget *mnuiPaperSize[24] ;
  GtkAdjustment *adjPaperCX ;
  GtkAdjustment *adjPaperCY ;
  GtkWidget *spnPaperCX ;
  GtkWidget *spnPaperCY ;
  GtkWidget *lblPaperCX ;
  GtkWidget *lblPaperCY ;
  GtkWidget *rbPortrait ;
  GtkWidget *rbLandscape ;

  GtkAdjustment *adjLMargin ;
  GtkAdjustment *adjTMargin ;
  GtkAdjustment *adjRMargin ;
  GtkAdjustment *adjBMargin ;
  GtkWidget *spnLMargin ;
  GtkWidget *spnTMargin ;
  GtkWidget *spnRMargin ;
  GtkWidget *spnBMargin ;
  GtkWidget *lblLMargin ;
  GtkWidget *lblTMargin ;
  GtkWidget *lblRMargin ;
  GtkWidget *lblBMargin ;

  GtkWidget *daPreview ;

  GtkWidget *btnCancel ;
  GtkWidget *btnPrint ;
  GtkWidget *btnPreview ;
  } PrintDialog ;

typedef struct
  {
  GtkDialogClass parent_class ;
  void (*changed) (PrintDialog *pd, gpointer data) ;
  void (*units_changed) (PrintDialog *pd, gpointer data) ;
  void (*preview) (PrintDialog *pd, gpointer data) ;
  } PrintDialogClass ;

GType print_dialog_get_type () ;

// Public function
GtkWidget *print_dialog_new (print_OP *pPO) ;
void print_dialog_add_page (PrintDialog *pd, GtkWidget *contents, char *pszLbl) ;
void print_dialog_get_options (PrintDialog *pd, print_OP *pPO) ;
double print_dialog_to_current_units (PrintDialog *pd, double dPoints) ;
double print_dialog_from_current_units (PrintDialog *pd, double dPoints) ;
char *print_dialog_get_units_short_string (PrintDialog *pd) ;
PrintDialogUnits print_dialog_get_units (PrintDialog *pd) ;

#define TYPE_PRINT_DIALOG (print_dialog_get_type ())
#define PRINT_DIALOG(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_PRINT_DIALOG, PrintDialog))
#define PRINT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PRINT_DIALOG, PrintDialogClass))
#define IS_PRINT_DIALOG(object) (G_TYPE_CHECK_INSTANCE_TYPE (object, TYPE_PRINT_DIALOG))
#define IS_PRINT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PRINT_DIALOG))
#define PRINT_DIALOG_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_PRINT_DIALOG, PrintDialogClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PRINT_DIALOG_H_ */
