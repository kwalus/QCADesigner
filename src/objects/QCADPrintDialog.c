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
// Print dialog. This is a basic print dialog derived   //
// from GtkDialog. It is a dialog box with 3 property   //
// pages and a facility for adding more.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "QCADPrintDialog.h"
#include "../generic_utils.h"
#include "../file_selection_window.h"
#include "../support.h"

typedef struct
  {
  GtkWidget *cbUnits ;

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

  GtkWidget *cbPaperSize ;
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
  int current_units ;
  gboolean bIgnorePaperSizeChanges ;
  } QCADPrintDialogPrivate ;

#define QCAD_PRINT_DIALOG_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_PRINT_DIALOG, QCADPrintDialogPrivate))

#define MIN_MARGIN_SEPARATION 72 /* points */

static void qcad_print_dialog_class_init (QCADPrintDialogClass *klass, gpointer data) ;
static void qcad_print_dialog_instance_init (QCADPrintDialog *dlg, gpointer data) ;
//Helpers
static void print_op_to_dialog (QCADPrintDialog *pd, print_OP *pPO) ;
static int get_paper_index (double cx, double cy) ;
static void redraw_preview (GtkWidget *daPreview, double dPaperWidth, double dPaperHeight, double dLeftMargin, double dTopMargin, double dRightMargin, double dBottomMargin) ;
void emit_changed_signal (QCADPrintDialog *pd) ;
//Callbacks
static void print_destination_toggled (GtkWidget *widget, gpointer data) ;
static void btnPrint_clicked (GtkWidget *widget, gpointer data) ;
static void daPreview_expose (GtkWidget *widget, GdkEventExpose *ev, gpointer data) ;
static void cbPaperSize_changed (GtkWidget *widget, gpointer data) ;
static void paper_orientation_toggled (GtkWidget *widget, gpointer data) ;
static void paper_size_changed (GtkWidget *widget, gpointer data) ;
static void margins_changed (GtkWidget *widget, gpointer data) ;
static void units_changed (GtkWidget *widget, gpointer data) ;
static void browse_for_file (GtkWidget *widget, gpointer data) ;
static void btnPreview_clicked (GtkWidget *widget, gpointer data) ;
static void daPreview_size_request (GtkWidget *widget, GtkRequisition *rq, gpointer data) ;

static struct
  {
  char *pszName ;
  double cx ;
  double cy ;
  } paper_types[] =
    {
    {N_("Custom"),   0.00,    0.00},
    {N_("Letter"), 612.00,  792.00}, {N_("Legal"),  612.00, 1008.00}, {N_("Ledger"),  1224.57,  790.86},
    {"10×14",      720.00, 1008.00}, {"11×17",      792.00, 1224.00}, {"A0",          2381.10, 3367.56},
    {"A1",        1683.78, 2381.10}, {"A2",        1190.55, 1683.78}, {"A3",           841.89, 1190.55},
    {"A4",         595.28,  841.89}, {"A5",         419.53,  595.28}, {"A6",           297.64,  419.53},
    {"A7",         209.76,  297.64}, {"A8",         147.40,  209.76}, {"A9",           104.88,  147.40},
    {"A10",         73.70,  104.88}, {"B0",        2834.65, 4008.19}, {"B1",          2004.09, 2834.65},
    {"B2",        1417.32, 2004.09}, {"B3",         997.79, 1417.32}, {"B4",           708.66,  997.79},
    {"B5",         498.90,  708.66}, {"B6",         283.46,  498.90}
    } ;
static int n_paper_types = G_N_ELEMENTS (paper_types) ;

static struct
  {
  char *pszDescription ;
  char *pszShort ;
  int unit_val ;
  double dConv[PD_UNITS_LAST] ;
  } units[] =
    {
    {N_("Centimeters"), "cm", PD_UNITS_CENTIS, {    1.0,    (1.0 / 2.54), (72.0 / 2.54)}},
    {N_("Inches"),      "in", PD_UNITS_INCHES, {    2.54,       1.0,           72.0    }},
    {N_("Points"),      "pt", PD_UNITS_POINTS, {2.54 / 72.0, 1.0 / 72.0,        1.0    }},
    } ;
static int n_units = G_N_ELEMENTS (units) ;

enum
  {
  QCAD_PRINT_DIALOG_CHANGED_SIGNAL,
  QCAD_PRINT_DIALOG_UNITS_CHANGED_SIGNAL,
  QCAD_PRINT_DIALOG_PREVIEW_SIGNAL,
  QCAD_PRINT_DIALOG_LAST_SIGNAL
  };

static guint qcad_print_dialog_signals[QCAD_PRINT_DIALOG_LAST_SIGNAL] = {0} ;

GType qcad_print_dialog_get_type ()
  {
  static GType qcad_print_dialog_type = 0 ;

  if (!qcad_print_dialog_type)
    {
    static const GTypeInfo qcad_print_dialog_info =
      {
      sizeof (QCADPrintDialogClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_print_dialog_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADPrintDialog),
      0,
      (GInstanceInitFunc)qcad_print_dialog_instance_init
      } ;
    qcad_print_dialog_type = g_type_register_static (GTK_TYPE_DIALOG, QCAD_TYPE_STRING_PRINT_DIALOG, &qcad_print_dialog_info, 0) ;
    }
  return qcad_print_dialog_type ;
  }

static void qcad_print_dialog_class_init (QCADPrintDialogClass *klass, gpointer data)
  {
  qcad_print_dialog_signals[QCAD_PRINT_DIALOG_CHANGED_SIGNAL] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS  (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADPrintDialogClass, changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0) ;
  qcad_print_dialog_signals[QCAD_PRINT_DIALOG_UNITS_CHANGED_SIGNAL] =
    g_signal_new ("units_changed", G_TYPE_FROM_CLASS  (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADPrintDialogClass, units_changed), NULL, NULL, g_cclosure_marshal_VOID__DOUBLE, G_TYPE_NONE, 1, G_TYPE_DOUBLE) ;
  qcad_print_dialog_signals[QCAD_PRINT_DIALOG_PREVIEW_SIGNAL] =
    g_signal_new ("preview", G_TYPE_FROM_CLASS  (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADPrintDialogClass, preview), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0) ;

  g_type_class_add_private (klass, sizeof (QCADPrintDialogPrivate)) ;
  }

static void qcad_print_dialog_instance_init (QCADPrintDialog *print_dialog, gpointer data)
  {
  QCADPrintDialogPrivate *dlg = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;
  GtkWidget *tbl = NULL, *widget = NULL, *frame = NULL, *tblPg = NULL, *tblFm = NULL ;
  GSList *grp = NULL ;
  int Nix, Nix1 ;
  GType *types = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkListStore *ls = NULL ;
  GtkTreeIter itr ;

  dlg->current_units = 0 ;
  dlg->bIgnorePaperSizeChanges = FALSE ;

  gtk_window_set_title (GTK_WINDOW (print_dialog), QCAD_TYPE_STRING_PRINT_DIALOG) ;
  gtk_window_set_resizable (GTK_WINDOW (print_dialog), FALSE) ;
  gtk_window_set_modal (GTK_WINDOW (print_dialog), TRUE) ;

  tbl = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (print_dialog)->vbox), tbl, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  widget = dlg->cbUnits = gtk_combo_box_new () ;
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (widget), cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (widget), cr, "text", 0) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 1, 2, 0, 1,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (0), 2, 2) ;
  types = g_malloc0 ((n_units + 3) * sizeof (GType)) ;
  types[0] = G_TYPE_STRING ;
  types[1] = G_TYPE_STRING ;
  types[2] = G_TYPE_INT ;
  for (Nix = 3 ; Nix < n_units + 3 ; Nix++)
    types[Nix] = G_TYPE_DOUBLE ;
  ls = gtk_list_store_newv (n_units + 3, types) ;
  g_free (types) ;
  for (Nix = 0 ; Nix < n_units ; Nix++)
    {
    gtk_list_store_append (ls, &itr) ;
    gtk_list_store_set (ls, &itr, 0, units[Nix].pszDescription, -1) ;
    gtk_list_store_set (ls, &itr, 1, units[Nix].pszShort,       -1) ;
    gtk_list_store_set (ls, &itr, 2, units[Nix].unit_val,       -1) ;
    for (Nix1 = 3 ; Nix1 < n_units + 3 ; Nix1++)
      gtk_list_store_set (ls, &itr, Nix1, units[Nix].dConv[Nix1 - 3], -1) ;
    }
  g_object_set (G_OBJECT (widget), "model", ls, NULL) ;
  g_object_set (G_OBJECT (widget), "active", 0, NULL) ;

  widget = gtk_label_new (_("Preferred Units:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 0, 1, 0, 1,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

  widget = dlg->nbPropPages = gtk_notebook_new () ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 0, 2, 1, 2,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2) ;

  //Notebook page 0: Printer
  widget = frame = gtk_frame_new (_("Print To")) ;
  gtk_widget_show (widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = gtk_label_new (_("Printer")) ;
  gtk_widget_show (widget) ;

  gtk_notebook_append_page (GTK_NOTEBOOK (dlg->nbPropPages), frame, widget) ;

  widget = tblPg = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tblPg) ;
  gtk_container_add (GTK_CONTAINER (frame), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = dlg->rbPrintFile = gtk_radio_button_new_with_label (grp, _("File")) ;
  grp = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget)) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK), 2, 2) ;

#ifdef WIN32
  print_dialog->rbPrintPipe =
#endif /* def WIN32 */
  widget = dlg->rbPrintPipe = gtk_radio_button_new_with_label (grp, _("Command")) ;
  grp = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget)) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK), 2, 2) ;

  widget = dlg->fmFileSelect = gtk_frame_new (_("Select File")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 0, 1,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = tblFm = gtk_table_new (1, 3, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (dlg->fmFileSelect), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = dlg->lblFileSelect = gtk_label_new (_("File Name:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 0, 1,
    (GtkAttachOptions) (GTK_FILL),
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

  widget = dlg->txtFileSelect = gtk_entry_new () ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 0, 1,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (0), 2, 2);

  widget = dlg->btnFileSelect = gtk_button_new_with_label (_("Browse...")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 2, 3, 0, 1,
    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
    (GtkAttachOptions) (0), 2, 2);

  widget = dlg->fmPipeSelect = gtk_frame_new (_("Command Line")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 1, 2,
    (GtkAttachOptions) (GTK_FILL),
    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = tblFm = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (dlg->fmPipeSelect), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = dlg->lblPipeSelect = gtk_label_new (_("Command:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 0, 1,
    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

  widget = dlg->txtPipeSelect = gtk_entry_new () ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 0, 1,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_text (GTK_ENTRY (widget), "lpr") ;

  widget = dlg->btnPipeSelect = gtk_button_new_with_label (_("Browse...")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 2, 3, 0, 1,
    (GtkAttachOptions) (GTK_FILL),
    (GtkAttachOptions) (0), 2, 2);

  widget = dlg->lblPipeSelectBlurb = gtk_label_new (_("Note: The PostScript data will be piped to your specified command. Any valid command line is accepted.")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 3, 1, 2,
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
  gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

  //Notebook page 1: Paper size:
  widget = tblPg = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblPg), 2) ;

  widget = gtk_label_new (_("Paper Size")) ;
  gtk_widget_show (widget) ;

  gtk_notebook_append_page (GTK_NOTEBOOK (dlg->nbPropPages), tblPg, widget) ;

  widget = frame = gtk_frame_new (_("Common Paper Sizes")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2) ;

  widget = tblFm = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (frame), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblFm), 2) ;

  widget = dlg->cbPaperSize = gtk_combo_box_new () ;
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (widget), cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (widget), cr, "text", 0) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  ls = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_DOUBLE) ;
  for (Nix = 0 ; Nix < n_paper_types ; Nix++)
    {
    gtk_list_store_append (ls, &itr) ;
    gtk_list_store_set (ls, &itr, 0, paper_types[Nix].pszName, 1, paper_types[Nix].cx, 2, paper_types[Nix].cy, -1) ;
    }
  g_object_set (G_OBJECT (widget), "model", ls, NULL) ;

  widget = frame = gtk_frame_new (_("Orientation")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 1, 2,
    (GtkAttachOptions) (GTK_FILL),
    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = tblFm = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (frame), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  grp = NULL ;
  widget = dlg->rbPortrait = gtk_radio_button_new_with_label (grp, _("Portrait")) ;
  grp = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget)) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

#ifdef HAVE_LIBRSVG
  widget = create_pixmap (GTK_WIDGET (print_dialog), "portrait.svg") ;
#else
  widget = create_pixmap (GTK_WIDGET (print_dialog), "portrait.png") ;
#endif /* def HAVE_LIBRSVG */
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  widget = dlg->rbLandscape = gtk_radio_button_new_with_label (grp, _("Landscape")) ;
  grp = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget)) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

#ifdef HAVE_LIBRSVG
  widget = create_pixmap (GTK_WIDGET (print_dialog), "landscape.svg") ;
#else
  widget = create_pixmap (GTK_WIDGET (print_dialog), "landscape.png") ;
#endif /* def HAVE_LIBRSVG */
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  widget = frame = gtk_frame_new (_("Custom")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 0, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = tblFm = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (frame), widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (widget), 2) ;

  widget = gtk_label_new (_("Paper Width:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

  dlg->adjPaperCX = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 1000, 0.1, 2, 10));
  widget = dlg->spnPaperCX = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjPaperCX), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblPaperCX = gtk_label_new ("cm") ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 2, 3, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = gtk_label_new (_("Paper Height:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

  dlg->adjPaperCY = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 1000, 0.1, 2, 10));
  widget = dlg->spnPaperCY = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjPaperCY), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblFm), widget, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblPaperCY = gtk_label_new ("cm") ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblFm), widget, 2, 3, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = gtk_label_new (_("Note: If you change the paper size, please also have a look at the margins. They may have changed as a result.")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  //Notebook page 2: Margins:
  widget = tblPg = gtk_table_new (4, 3, FALSE) ;
  gtk_widget_show (widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblPg), 2) ;

  widget = gtk_label_new (_("Margins")) ;
  gtk_widget_show (widget) ;

  gtk_notebook_append_page (GTK_NOTEBOOK (dlg->nbPropPages), tblPg, widget) ;

  widget = gtk_label_new (_("Left Margin:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 1.0, 0.5) ;

  dlg->adjLMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  widget = dlg->spnLMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjLMargin), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblLMargin = gtk_label_new (_("cm")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 2, 3, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = gtk_label_new (_("Top Margin:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 1.0, 0.5) ;

  dlg->adjTMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  widget = dlg->spnTMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjTMargin), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblTMargin = gtk_label_new (_("cm")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 2, 3, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = gtk_label_new (_("Right Margin:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 1.0, 0.5) ;

  dlg->adjRMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  widget = dlg->spnRMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjRMargin), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblRMargin = gtk_label_new (_("cm")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 2, 3, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = gtk_label_new (_("Bottom Margin:")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 0, 1, 3, 4,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 1.0, 0.5) ;

  dlg->adjBMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  widget = dlg->spnBMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dlg->adjBMargin), 1, 0);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (tblPg), widget, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (widget), 2) ;

  widget = dlg->lblBMargin = gtk_label_new (_("cm")) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tblPg), widget, 2, 3, 3, 4,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_SHRINK), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5) ;

  widget = frame = gtk_frame_new (NULL) ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_frame_set_shadow_type (GTK_FRAME (widget), GTK_SHADOW_IN) ;
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2) ;

  widget = dlg->daPreview = gtk_drawing_area_new ();
  gtk_widget_show (widget);
  gtk_container_add (GTK_CONTAINER (frame), widget) ;
//  gtk_widget_set_usize (widget, 200, 200) ;

  widget = dlg->btnPreview = gtk_button_new_from_stock (GTK_STOCK_PRINT_PREVIEW) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (print_dialog)->action_area), widget) ;

  gtk_dialog_add_button (GTK_DIALOG (print_dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL) ;

  widget = dlg->btnPrint = gtk_button_new_from_stock (GTK_STOCK_PRINT) ;
  gtk_widget_show (widget) ;
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (print_dialog)->action_area), widget) ;

  g_signal_connect (G_OBJECT (dlg->rbPrintFile),   "toggled",       (GCallback)print_destination_toggled, print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->btnPrint),      "clicked",       (GCallback)btnPrint_clicked,          print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->daPreview),     "expose-event",  (GCallback)daPreview_expose,          dlg) ;
  g_signal_connect (G_OBJECT (dlg->cbPaperSize),   "changed",       (GCallback)cbPaperSize_changed,       print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->rbPortrait),    "toggled",       (GCallback)paper_orientation_toggled, print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->rbLandscape),   "toggled",       (GCallback)paper_orientation_toggled, print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjPaperCX),    "value-changed", (GCallback)paper_size_changed,        print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjPaperCY),    "value-changed", (GCallback)paper_size_changed,        print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjLMargin),    "value-changed", (GCallback)margins_changed,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjTMargin),    "value-changed", (GCallback)margins_changed,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjRMargin),    "value-changed", (GCallback)margins_changed,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->adjBMargin),    "value-changed", (GCallback)margins_changed,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->cbUnits),       "changed",       (GCallback)units_changed,             print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->btnFileSelect), "clicked",       (GCallback)browse_for_file,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->btnPipeSelect), "clicked",       (GCallback)browse_for_file,           print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->btnPreview),    "clicked",       (GCallback)btnPreview_clicked,        print_dialog) ;
  g_signal_connect (G_OBJECT (dlg->daPreview),     "size-request",  (GCallback)daPreview_size_request,    NULL) ;
  }

/*****************************************************************************
* PUBLIC FUNCTIONS                                                           *
*****************************************************************************/

GtkWidget *qcad_print_dialog_new (print_OP *pPO)
  {
  QCADPrintDialog *ret = QCAD_PRINT_DIALOG (g_object_new (QCAD_TYPE_PRINT_DIALOG, NULL)) ;
  print_op_to_dialog (ret, pPO) ;
  return GTK_WIDGET (ret) ;
  }

void qcad_print_dialog_add_page (QCADPrintDialog *print_dialog, GtkWidget *contents, char *pszLbl)
  {
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;
  GtkWidget *lbl = gtk_label_new (pszLbl) ;
  gtk_widget_show (lbl) ;
  gtk_notebook_append_page (GTK_NOTEBOOK (pd->nbPropPages), contents, lbl) ;
  }

void qcad_print_dialog_get_options (QCADPrintDialog *print_dialog, print_OP *pPO)
  {
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;

  pPO->dPaperCX = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCX))) ;
  pPO->dPaperCY = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCY))) ;
  pPO->dLMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjLMargin))) ;
  pPO->dTMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjTMargin))) ;
  pPO->dRMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjRMargin))) ;
  pPO->dBMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjBMargin))) ;
  pPO->bPortrait = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbPortrait)) ;
  pPO->bPrintFile = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbPrintFile)) ;
  pPO->pszPrintString =
    gtk_editable_get_chars (GTK_EDITABLE (pPO->bPrintFile ? pd->txtFileSelect : pd->txtPipeSelect), 0, -1) ;
  }

QCADPrintDialogUnits qcad_print_dialog_get_units (QCADPrintDialog *print_dialog)
  {
  int val ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;

  g_object_get (G_OBJECT (pd->cbUnits), "model", &tm, NULL) ;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (pd->cbUnits), &itr)) return -1 ;

  gtk_tree_model_get (GTK_TREE_MODEL (tm), &itr, 2, &val, -1) ;

  return val ;
  }

char *qcad_print_dialog_get_units_short_string (QCADPrintDialog *print_dialog)
  {
  char *psz = NULL ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;

  g_object_get (G_OBJECT (pd->cbUnits), "model", &tm, NULL) ;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (pd->cbUnits), &itr)) return g_strdup ("??") ;

  gtk_tree_model_get (GTK_TREE_MODEL (tm), &itr, 1, &psz, -1) ;

  return psz ;
  }

double qcad_print_dialog_to_current_units (QCADPrintDialog *print_dialog, double dPoints)
  {
  int unit_type ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;
  double conversion_factor ;

  g_object_get (G_OBJECT (pd->cbUnits), "model", &tm, NULL) ;
  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      gtk_tree_model_get (tm, &itr, 2, &unit_type, -1) ;
      if (PD_UNITS_POINTS == unit_type) break ;
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;

  gtk_tree_model_get (tm, &itr, pd->current_units + 3, &conversion_factor, -1) ;

  return dPoints * conversion_factor ;
  }

double qcad_print_dialog_from_current_units (QCADPrintDialog *print_dialog, double dUnits)
  {
  char *psz = NULL ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;
  double conversion_factor = 1 ;

  g_object_get (G_OBJECT (pd->cbUnits), "model", &tm, NULL) ;
  if (gtk_tree_model_get_iter_from_string (tm, &itr, psz = g_strdup_printf ("%d", pd->current_units)))
    gtk_tree_model_get (tm, &itr, 0, &psz, 5, &conversion_factor, -1) ;
  g_free (psz) ;

  return dUnits * conversion_factor ;
  }

/*****************************************************************************
* HELPERS                                                                    *
*****************************************************************************/

static void redraw_preview (GtkWidget *daPreview, double dPaperWidth, double dPaperHeight, double dLeftMargin, double dTopMargin, double dRightMargin, double dBottomMargin)
  {
  int iWidth = 0, iHeight = 0 ;
  double x = 0, y = 0, rectWidth = dPaperWidth, rectHeight = dPaperHeight ;
  int xL, yL, xR, yR ;

  if (0 == dPaperWidth || 0 == dPaperHeight)
    return ;

  if (NULL == daPreview) return ;
  if (NULL == daPreview->window) return ;

  gdk_window_get_size (daPreview->window, &iWidth, &iHeight) ;
  if (0 == iWidth || 0 == iHeight) return ;

  fit_rect_inside_rect ((double)iWidth, (double)iHeight, &x, &y, &rectWidth, &rectHeight) ;

  gdk_window_clear (daPreview->window) ;

  gdk_draw_rectangle (daPreview->window, daPreview->style->white_gc, TRUE,
    (int)x, (int)y, (int)rectWidth, (int)rectHeight) ;

  xL = x + rectWidth * dLeftMargin / dPaperWidth ;
  yL = y + rectHeight * dTopMargin / dPaperHeight ;
  xR = x + rectWidth * (1.0 - dRightMargin / dPaperWidth) ;
  yR = y + rectHeight * (1.0 - dBottomMargin / dPaperHeight) ;

  gdk_draw_line (daPreview->window, daPreview->style->black_gc, xL, y, xL, y + rectHeight) ; /*left margin */
  gdk_draw_line (daPreview->window, daPreview->style->black_gc, x, yL, x + rectWidth, yL) ;  /*top margin */
  gdk_draw_line (daPreview->window, daPreview->style->black_gc, xR, y, xR, y + rectHeight) ; /* right margin */
  gdk_draw_line (daPreview->window, daPreview->style->black_gc, x, yR, x + rectWidth, yR) ;  /* bottom margin */
  }

static void print_op_to_dialog (QCADPrintDialog *print_dialog, print_OP *pPO)
  {
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (print_dialog) ;
  int idxPaper = 0 ;
  // The first tab
  if (pPO->bPrintFile)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->rbPrintFile), TRUE) ;
    print_destination_toggled (pd->rbPrintFile, print_dialog) ;
    if (NULL != pPO->pszPrintString)
      gtk_entry_set_text (GTK_ENTRY (pd->txtFileSelect), pPO->pszPrintString) ;
    }
  else
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->rbPrintPipe), TRUE) ;
    print_destination_toggled (pd->rbPrintPipe, print_dialog) ;
    if (NULL != pPO->pszPrintString)
      gtk_entry_set_text (GTK_ENTRY (pd->txtPipeSelect), pPO->pszPrintString) ;
    }

  // the second tab
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), qcad_print_dialog_to_current_units (print_dialog, pPO->dPaperCX)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), qcad_print_dialog_to_current_units (print_dialog, pPO->dPaperCY)) ;
  if ((idxPaper = get_paper_index (pPO->dPaperCX, pPO->dPaperCY)))
    g_object_set (G_OBJECT (pd->cbPaperSize), "active", idxPaper, NULL) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pPO->bPortrait ? pd->rbPortrait : pd->rbLandscape), TRUE) ;

  // the third tab
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjLMargin), qcad_print_dialog_to_current_units (print_dialog, pPO->dLMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjTMargin), qcad_print_dialog_to_current_units (print_dialog, pPO->dTMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjRMargin), qcad_print_dialog_to_current_units (print_dialog, pPO->dRMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjBMargin), qcad_print_dialog_to_current_units (print_dialog, pPO->dBMargin)) ;

  // Repaint the drawing area
  gtk_widget_queue_draw (pd->daPreview) ;
  }

static int get_paper_index (double cx, double cy)
  {
  int Nix ;

  for (Nix = 1 ; Nix < 24 ; Nix++)
    if (paper_types[Nix].cx == cx && paper_types[Nix].cy == cy)
      return Nix ;
    else
    if (paper_types[Nix].cy == cx && paper_types[Nix].cx == cy)
      return -1 * Nix ;
  return 0 ;
  }

void emit_changed_signal (QCADPrintDialog *pd)
  {
  print_OP po = {0} ;
  qcad_print_dialog_get_options (pd, &po) ;
  g_signal_emit (G_OBJECT (pd), qcad_print_dialog_signals[QCAD_PRINT_DIALOG_CHANGED_SIGNAL], 0) ;
  g_free (po.pszPrintString) ;
  }

/*****************************************************************************
* CALLBACKS                                                                  *
*****************************************************************************/

static void daPreview_size_request (GtkWidget *widget, GtkRequisition *rq, gpointer data)
  {
  rq->width = MAX (rq->width, 200) ;
  rq->height = MAX (rq->height, 200) ;
  }

static void browse_for_file (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  gboolean bPrintFile = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbPrintFile)) ;
  GtkWidget *txt = bPrintFile ? pd->txtFileSelect : pd->txtPipeSelect ;
  gchar
    *pszOld = gtk_editable_get_chars (GTK_EDITABLE (txt), 0, -1),
    *pszPrintString = get_file_name_from_user (GTK_WINDOW (print_dialog), bPrintFile ? _("Select File") : _("Select Command"), pszOld, bPrintFile) ;
  g_free (pszOld) ;
  if (NULL != pszPrintString)
    {
    gtk_entry_set_text (GTK_ENTRY (txt), pszPrintString) ;
    g_free (pszPrintString) ;
    emit_changed_signal (data) ;
    }
  }

static void units_changed (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  double
    dPaperCX = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCX))),
    dPaperCY = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCY))),
    dLMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnLMargin))),
    dTMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnTMargin))),
    dRMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnRMargin))),
    dBMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnBMargin))),
    convert  = qcad_print_dialog_from_current_units (print_dialog, 1.0) ;
  char *pszUnits = NULL ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;

  g_object_get (G_OBJECT (pd->cbUnits), "model", &tm, NULL) ;
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (pd->cbUnits), &itr))
    gtk_tree_model_get (tm, &itr, 1, &pszUnits, -1) ;
  else
    pszUnits = g_strdup ("??") ;

  gtk_label_set_text (GTK_LABEL (pd->lblPaperCX), pszUnits) ;
  gtk_label_set_text (GTK_LABEL (pd->lblPaperCY), pszUnits) ;
  gtk_label_set_text (GTK_LABEL (pd->lblLMargin), pszUnits) ;
  gtk_label_set_text (GTK_LABEL (pd->lblTMargin), pszUnits) ;
  gtk_label_set_text (GTK_LABEL (pd->lblRMargin), pszUnits) ;
  gtk_label_set_text (GTK_LABEL (pd->lblBMargin), pszUnits) ;

  g_free (pszUnits) ;

  // set pd->current_units so (to|from)_current_units continues to work correctly
  g_object_get (G_OBJECT (pd->cbUnits), "active", &(pd->current_units), NULL) ;

  convert = qcad_print_dialog_to_current_units (print_dialog, convert) ;

  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), qcad_print_dialog_to_current_units (print_dialog, dPaperCX)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), qcad_print_dialog_to_current_units (print_dialog, dPaperCY)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjLMargin), qcad_print_dialog_to_current_units (print_dialog, dLMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjTMargin), qcad_print_dialog_to_current_units (print_dialog, dTMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjRMargin), qcad_print_dialog_to_current_units (print_dialog, dRMargin)) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjBMargin), qcad_print_dialog_to_current_units (print_dialog, dBMargin)) ;

  g_signal_emit (G_OBJECT (print_dialog), qcad_print_dialog_signals[QCAD_PRINT_DIALOG_UNITS_CHANGED_SIGNAL], 0, convert) ;
  }

static void margins_changed (GtkWidget *widget, gpointer data)
  {
  GtkAdjustment *adj = GTK_ADJUSTMENT (widget) ;
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  double
    dLMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnLMargin))),
    dTMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnTMargin))),
    dRMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnRMargin))),
    dBMargin = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnBMargin))),
    dPaperCX = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCX))),
    dPaperCY = qcad_print_dialog_from_current_units (print_dialog, gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCY))),
    dDiff ;

  // This ensures that all spin buttons (margins AND paper size) can go up forever
  if (gtk_adjustment_get_value (adj) / adj->upper > 0.9)
    adj->upper += adj->page_increment ;

  if ((dDiff = dPaperCX - dLMargin - dRMargin) < MIN_MARGIN_SEPARATION)
    {
    if (pd->adjLMargin == adj)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (widget), qcad_print_dialog_to_current_units (print_dialog, dLMargin + dDiff - MIN_MARGIN_SEPARATION)) ;
    else
    if (pd->adjRMargin == adj)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (widget), qcad_print_dialog_to_current_units (print_dialog, dRMargin + dDiff - MIN_MARGIN_SEPARATION)) ;
    else
      {
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjLMargin), qcad_print_dialog_to_current_units (print_dialog, dLMargin + (dDiff - MIN_MARGIN_SEPARATION) / 2.0)) ;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjRMargin), qcad_print_dialog_to_current_units (print_dialog, dRMargin + (dDiff - MIN_MARGIN_SEPARATION) / 2.0)) ;
      }
    }

  if ((dDiff = dPaperCY - dTMargin - dBMargin) < MIN_MARGIN_SEPARATION)
    {
    if (pd->adjTMargin == adj)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (widget), qcad_print_dialog_to_current_units (print_dialog, dTMargin + dDiff - MIN_MARGIN_SEPARATION)) ;
    else
    if (pd->adjBMargin == adj)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (widget), qcad_print_dialog_to_current_units (print_dialog, dBMargin + dDiff - MIN_MARGIN_SEPARATION)) ;
    else
      {
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjTMargin), qcad_print_dialog_to_current_units (print_dialog, dTMargin + (dDiff - MIN_MARGIN_SEPARATION) / 2.0)) ;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjBMargin), qcad_print_dialog_to_current_units (print_dialog, dBMargin + (dDiff - MIN_MARGIN_SEPARATION) / 2.0)) ;
      }
    }
  gtk_widget_queue_draw (pd->daPreview) ;
  emit_changed_signal (print_dialog) ;
  }

static void cbPaperSize_changed (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  int idx = -1 ;

  g_object_get (G_OBJECT (widget), "active", &idx, NULL) ;

  // idx == 0 => custom
  if (idx > 0)
    {
    double cx, cy ;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbLandscape)))
      {
      cx = paper_types[idx].cy ;
      cy = paper_types[idx].cx ;
      }
    else
      {
      cx = paper_types[idx].cx ;
      cy = paper_types[idx].cy ;
      }

    gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), qcad_print_dialog_to_current_units (print_dialog, cx)) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), qcad_print_dialog_to_current_units (print_dialog, cy)) ;
    }

  emit_changed_signal (print_dialog) ;
  gtk_widget_queue_draw (pd->daPreview) ;
  }

static void daPreview_expose (GtkWidget *widget, GdkEventExpose *ev, gpointer data)
  {
  QCADPrintDialogPrivate *pd = (QCADPrintDialogPrivate *)data ;

  redraw_preview (pd->daPreview,
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCX)),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCY)),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjLMargin)),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjTMargin)),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjRMargin)),
    gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjBMargin))) ;
  }

static void paper_size_changed (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  double cx, cy ;
  int idx ;

  if (pd->bIgnorePaperSizeChanges) return ;

  cx = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCX))),
  cy = qcad_print_dialog_from_current_units (print_dialog, gtk_adjustment_get_value (GTK_ADJUSTMENT (pd->adjPaperCY))) ;
  idx = get_paper_index (cx, cy) ;

  margins_changed (widget, data) ;

  if (0 == idx)
    g_object_set (G_OBJECT (pd->cbPaperSize), "active", 0, NULL) ;
  else
  if (idx > 0)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->rbPortrait), TRUE) ;
    g_object_set (G_OBJECT (pd->cbPaperSize), "active", idx, NULL) ;
    }
  else
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pd->rbLandscape), TRUE) ;
    idx *= -1 ;
    g_object_set (G_OBJECT (pd->cbPaperSize), "active", idx, NULL) ;
    }

  emit_changed_signal (print_dialog) ;
  }

static void print_destination_toggled (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialogPrivate *dlg = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;

  gtk_widget_set_sensitive (dlg->fmFileSelect,  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintFile))) ;
  gtk_widget_set_sensitive (dlg->txtFileSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintFile))) ;
  gtk_widget_set_sensitive (dlg->btnFileSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintFile))) ;
  gtk_widget_set_sensitive (dlg->lblFileSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintFile))) ;
  gtk_widget_set_sensitive (dlg->fmPipeSelect,  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintPipe))) ;
  gtk_widget_set_sensitive (dlg->txtPipeSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintPipe))) ;
  gtk_widget_set_sensitive (dlg->btnPipeSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintPipe))) ;
  gtk_widget_set_sensitive (dlg->lblPipeSelect, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->rbPrintPipe))) ;

  emit_changed_signal (QCAD_PRINT_DIALOG (data)) ;
  }

static void paper_orientation_toggled (GtkWidget *widget, gpointer data)
  {
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  int idx = -1 ;
  float cx, cy ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  g_object_get (G_OBJECT (pd->cbPaperSize), "active", &idx, NULL) ;
  cx = gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCX)) ;
  cy = gtk_spin_button_get_value (GTK_SPIN_BUTTON (pd->spnPaperCY)) ;

  if (!idx)
    {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), cy) ;
    gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), cx) ;
    }
  else
    {
    pd->bIgnorePaperSizeChanges = TRUE ;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbPortrait)))
      {
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), qcad_print_dialog_to_current_units (print_dialog, paper_types[idx].cx)) ;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), qcad_print_dialog_to_current_units (print_dialog, paper_types[idx].cy)) ;
      }
    else
      {
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCX), qcad_print_dialog_to_current_units (print_dialog, paper_types[idx].cy)) ;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (pd->adjPaperCY), qcad_print_dialog_to_current_units (print_dialog, paper_types[idx].cx)) ;
      }
    pd->bIgnorePaperSizeChanges = TRUE ;
    }

  emit_changed_signal (print_dialog) ;
  gtk_widget_queue_draw (pd->daPreview) ;
  }

static void btnPrint_clicked (GtkWidget *widget, gpointer data)
  {
  GtkWidget *msg ;
  QCADPrintDialog *print_dialog = QCAD_PRINT_DIALOG (data) ;
  QCADPrintDialogPrivate *pd = QCAD_PRINT_DIALOG_GET_PRIVATE (data) ;
  gchar *pszText = NULL ;
  gboolean bPrintFile = TRUE ;

  if (!strlen (pszText =
    gtk_editable_get_chars (GTK_EDITABLE ((bPrintFile =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pd->rbPrintFile))) ? pd->txtFileSelect : pd->txtPipeSelect), 0, -1)))
    {
    gtk_dialog_run (GTK_DIALOG (msg =
      gtk_message_dialog_new (GTK_WINDOW (pd), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        _("Cannot print without having %s to print to!"), bPrintFile ? _("a file") : _("a pipe")))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszText) ;
    return ;
    }
  gtk_dialog_response (GTK_DIALOG (print_dialog), GTK_RESPONSE_OK) ;
  }

static void btnPreview_clicked (GtkWidget *widget, gpointer data)
  {g_signal_emit (G_OBJECT (data), qcad_print_dialog_signals[QCAD_PRINT_DIALOG_PREVIEW_SIGNAL], 0) ;}
