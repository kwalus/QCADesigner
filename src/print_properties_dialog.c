//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@vlsi.enel.ucalgary.ca). It implements a    //
// (fairly) complete print settings dialog with mar-    //
// gins, Center Page, paper size, user-selectable units //
// (cm/in/pt), etc.                                     //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <math.h>

#include "support.h"
#include "stdqcell.h"
#include "cad.h"
#include "print_preview.h"
#include "message_box.h"

#include "blocking_dialog.h"
#include "custom_widgets.h"
#include "file_selection_window.h"
#include "print_properties_dialog.h"
#include "print_properties_dialog_private.h"

#define MIN_MARGIN_GAP 72 /* points */

#define STATUS_OK 0
#define STATUS_NEED_FILE_NAME 1
#define STATUS_NEED_PIPE 2

static print_properties_D print_properties = {NULL} ;
static GtkWidget *old_preferred_units_menu_item = NULL ;
static gboolean bSpinButtonsDoNotReact = FALSE ;
static double
  world_extents_x1 = 0,
  world_extents_y1 = 0,
  world_extents_x2 = 0,
  world_extents_y2 = 0 ;

void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight) ;
void init_print_properties_dialog (print_properties_D *dialog, print_OP *print_op, GtkWindow *parent, qcell *first_cell, gboolean *pbOK) ;
void create_print_properties_dialog (print_properties_D *dialog) ;
void on_print_properties_dialog_btnOK_clicked(GtkButton *button, gpointer user_data) ;
void on_print_properties_dialog_btnPreview_clicked(GtkButton *button, gpointer user_data) ;
void on_daPreview_expose (GtkWidget *widget, GdkEventExpose *event, gpointer user_data) ;
void redraw_preview (GtkWidget *daPreview, double dPaperWidth, double dPaperHeight, double dLeftMargin, double dTopMargin, double dRightMargin, double dBottomMargin) ;
void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data) ;
void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data) ;
void toggle_scale_mode (GtkWidget *widget, gpointer user_data) ;
void toggle_print_mode (GtkWidget *widget, gpointer user_data) ;
void fill_printed_objects_list (GtkWidget *list, print_properties_D *dialog) ;
void calc_world_size (qcell *first_cell, int *piWidth, int *piHeight, print_properties_D *dialog) ;
void change_preferred_units (GtkWidget *widget, gpointer user_data) ;
double get_conversion_factor (GtkWidget *old_widget, GtkWidget *new_widget, print_properties_D *dialog) ;
void adjustment_set_value_and_step (GtkAdjustment *padj, double dValue, double dStep) ;
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data) ;
void check_margins (print_properties_D *dialog, double dLRatio, double dRRatio, double dTRatio, double dBRatio) ;
void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data) ;
void browse_for_output (GtkWidget *widget, gpointer user_data) ;
void browse_dialog_ok (GtkWidget *widget, gpointer user_data) ;
void check_scale (print_properties_D *dialog) ;
gboolean check_status (print_properties_D *dialog) ;

/* The main function */
gboolean get_print_properties_from_user (GtkWindow *parent, print_OP *ppo, qcell *first_cell)
  {
  gboolean bOK = FALSE ;
  
  if (NULL == print_properties.dlgPrintProps)
    create_print_properties_dialog (&print_properties) ;

  init_print_properties_dialog (&print_properties, ppo, parent, first_cell, &bOK) ;
  
  show_dialog_blocking (print_properties.dlgPrintProps) ;
  
  return bOK ;
  }

/* Create it */
void create_print_properties_dialog (print_properties_D *dialog){

  if (NULL != dialog->dlgPrintProps) return ;
  
  /* The dialog window */
  dialog->dlgPrintProps = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "dlgPrintProps", dialog->dlgPrintProps);
  gtk_window_set_title (GTK_WINDOW (dialog->dlgPrintProps), _("Printer Setup"));
  gtk_window_set_policy (GTK_WINDOW (dialog->dlgPrintProps), FALSE, FALSE, FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->dlgPrintProps), TRUE) ;

  dialog->vbMain = GTK_DIALOG (dialog->dlgPrintProps)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "vbMain", dialog->vbMain);
  gtk_widget_show (dialog->vbMain);
  
  /* The main table */
  dialog->tblMain = gtk_table_new (2, 3, FALSE) ;
  gtk_widget_ref (dialog->tblMain) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblMain", dialog->tblMain,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblMain);
  gtk_box_pack_start (GTK_BOX (dialog->vbMain), dialog->tblMain, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblMain), 2);
  
  /* Preferred units combo and menu */
  dialog->cbPrefUnits = gtk_option_menu_new () ;
  gtk_widget_ref (dialog->cbPrefUnits) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "cbPrefUnits", dialog->cbPrefUnits,
    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->cbPrefUnits) ;
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->cbPrefUnits, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  dialog->cbPrefUnitsMenu = gtk_menu_new ();
  dialog->cbmPUCentis = gtk_menu_item_new_with_label (_("Centimetres"));
  gtk_object_set_data (GTK_OBJECT (dialog->cbmPUCentis), "short_name", "cm") ;
  gtk_widget_show (dialog->cbmPUCentis);
  gtk_menu_append (GTK_MENU (dialog->cbPrefUnitsMenu), dialog->cbmPUCentis);
  dialog->cbmPUInches = gtk_menu_item_new_with_label (_("Inches"));
  gtk_object_set_data (GTK_OBJECT (dialog->cbmPUInches), "short_name", "in") ;
  gtk_widget_show (dialog->cbmPUInches);
  gtk_menu_append (GTK_MENU (dialog->cbPrefUnitsMenu), dialog->cbmPUInches);
  dialog->cbmPUPoints = gtk_menu_item_new_with_label (_("Points"));
  gtk_object_set_data (GTK_OBJECT (dialog->cbmPUPoints), "short_name", "pt") ;
  gtk_widget_show (dialog->cbmPUPoints);
  gtk_menu_append (GTK_MENU (dialog->cbPrefUnitsMenu), dialog->cbmPUPoints);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (dialog->cbPrefUnits), dialog->cbPrefUnitsMenu);

  dialog->label2 = gtk_label_new (_("Preferred Units:"));
  gtk_widget_ref (dialog->label2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label2", dialog->label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label2);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->label2, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label2), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label2), 1, 0.5);
  
  /* The tabs */
  dialog->notebook = gtk_notebook_new ();
  gtk_widget_ref (dialog->notebook);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "notebook", dialog->notebook,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->notebook);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->notebook, 0, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  
  /* Tab 0 - Printer device setup */
  dialog->fmPrinter = gtk_frame_new (_("Print To"));
  gtk_widget_ref (dialog->fmPrinter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmPrinter", dialog->fmPrinter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmPrinter);
  gtk_container_add (GTK_CONTAINER (dialog->notebook), dialog->fmPrinter);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmPrinter), 2);

  dialog->tblPrinter = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (dialog->tblPrinter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblPrinter", dialog->tblPrinter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblPrinter);
  gtk_container_add (GTK_CONTAINER (dialog->fmPrinter), dialog->tblPrinter);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblPrinter), 2);

  dialog->rbPrintFile = gtk_radio_button_new_with_label (dialog->grpPrinter, _("File"));
  dialog->grpPrinter = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbPrintFile));
  gtk_widget_ref (dialog->rbPrintFile);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "rbPrintFile", dialog->rbPrintFile,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->rbPrintFile);
  gtk_table_attach (GTK_TABLE (dialog->tblPrinter), dialog->rbPrintFile, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK), 2, 2);

  dialog->rbPrintPipe = gtk_radio_button_new_with_label (dialog->grpPrinter, _("Command"));
  dialog->grpPrinter = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbPrintPipe));
  gtk_widget_ref (dialog->rbPrintPipe);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "rbPrintPipe", dialog->rbPrintPipe,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->rbPrintPipe);
  gtk_table_attach (GTK_TABLE (dialog->tblPrinter), dialog->rbPrintPipe, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  dialog->fmFileSelect = gtk_frame_new (_("Select File"));
  gtk_widget_ref (dialog->fmFileSelect);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmFileSelect", dialog->fmFileSelect,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmFileSelect);
  gtk_table_attach (GTK_TABLE (dialog->tblPrinter), dialog->fmFileSelect, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmFileSelect), 2);

  dialog->tblFName = gtk_table_new (1, 3, FALSE);
  gtk_widget_ref (dialog->tblFName);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblFName", dialog->tblFName,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblFName);
  gtk_container_add (GTK_CONTAINER (dialog->fmFileSelect), dialog->tblFName);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblFName), 2);

  dialog->txtFName = gtk_entry_new ();
  gtk_widget_ref (dialog->txtFName);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "txtFName", dialog->txtFName,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->txtFName);
  gtk_table_attach (GTK_TABLE (dialog->tblFName), dialog->txtFName, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->btnFNBrowse = gtk_button_new_with_label (_("Browse..."));
  gtk_widget_ref (dialog->btnFNBrowse);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "btnFNBrowse", dialog->btnFNBrowse,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnFNBrowse);
  gtk_table_attach (GTK_TABLE (dialog->tblFName), dialog->btnFNBrowse, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblFName = gtk_label_new (_("File name:"));
  gtk_widget_ref (dialog->lblFName);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblFName", dialog->lblFName,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblFName);
  gtk_table_attach (GTK_TABLE (dialog->tblFName), dialog->lblFName, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblFName), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblFName), 1, 0.5);

  dialog->fmPipeSelect = gtk_frame_new (_("Specify Command"));
  gtk_widget_ref (dialog->fmPipeSelect);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmPipeSelect", dialog->fmPipeSelect,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmPipeSelect);
  gtk_table_attach (GTK_TABLE (dialog->tblPrinter), dialog->fmPipeSelect, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmPipeSelect), 2);

  dialog->tblPipeCmd = gtk_table_new (2, 3, FALSE);
  gtk_widget_ref (dialog->tblPipeCmd);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblPipeCmd", dialog->tblPipeCmd,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblPipeCmd);
  gtk_container_add (GTK_CONTAINER (dialog->fmPipeSelect), dialog->tblPipeCmd);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblPipeCmd), 2);

  dialog->txtPipeCmd = gtk_entry_new ();
  gtk_widget_ref (dialog->txtPipeCmd);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "txtPipeCmd", dialog->txtPipeCmd,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->txtPipeCmd);
  gtk_table_attach (GTK_TABLE (dialog->tblPipeCmd), dialog->txtPipeCmd, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_entry_set_text (GTK_ENTRY (dialog->txtPipeCmd), _("lpr"));

  dialog->lblPipeCmd = gtk_label_new (_("Command:"));
  gtk_widget_ref (dialog->lblPipeCmd);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPipeCmd", dialog->lblPipeCmd,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPipeCmd);
  gtk_table_attach (GTK_TABLE (dialog->tblPipeCmd), dialog->lblPipeCmd, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblPipeCmd), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPipeCmd), 1, 0.5);

  dialog->lblPipeNote = gtk_label_new (_(
    "Note: The PostScript data will be piped to your specified command. "
    "Any valid command line is accepted."));
  gtk_widget_ref (dialog->lblPipeNote);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPipeNote", dialog->lblPipeNote,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPipeNote);
  gtk_table_attach (GTK_TABLE (dialog->tblPipeCmd), dialog->lblPipeNote, 0, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblPipeNote), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (dialog->lblPipeNote), TRUE);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPipeNote), 0, 0);

  dialog->btnPipeCmdBrowse = gtk_button_new_with_label (_("Browse..."));
  gtk_widget_ref (dialog->btnPipeCmdBrowse);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "btnPipeCmdBrowse", dialog->btnPipeCmdBrowse,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnPipeCmdBrowse);
  gtk_table_attach (GTK_TABLE (dialog->tblPipeCmd), dialog->btnPipeCmdBrowse, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblPrinterTab = gtk_label_new (_("Printer"));
  gtk_widget_ref (dialog->lblPrinterTab);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPrinterTab", dialog->lblPrinterTab,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPrinterTab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook), 0), dialog->lblPrinterTab);

  /* Tab 1 - Paper size */
  dialog->tblPaperSize = gtk_table_new (3, 3, FALSE);
  gtk_widget_ref (dialog->tblPaperSize);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblPaperSize", dialog->tblPaperSize,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblPaperSize);
  gtk_container_add (GTK_CONTAINER (dialog->notebook), dialog->tblPaperSize);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblPaperSize), 2);

  dialog->label6 = gtk_label_new (_("Paper Width:"));
  gtk_widget_ref (dialog->label6);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label6", dialog->label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label6);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->label6, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label6), 1, 0.5);

  dialog->adjPaperWidth = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 1000, 0.1, 2, 10));
  dialog->spnPaperWidth = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjPaperWidth), 1, 0);
  gtk_widget_ref (dialog->spnPaperWidth);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnPaperWidth", dialog->spnPaperWidth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnPaperWidth);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->spnPaperWidth, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnPaperWidth), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnPaperWidth), 2) ;

  dialog->lblPaperWidth = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblPaperWidth);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPaperWidth", dialog->lblPaperWidth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPaperWidth);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->lblPaperWidth, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblPaperWidth), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPaperWidth), 0, 0.5);

  dialog->label8 = gtk_label_new (_("Paper Height:"));
  gtk_widget_ref (dialog->label8);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label8", dialog->label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label8);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->label8, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label8), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label8), 1, 0.5);

  dialog->adjPaperHeight = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 1000, 0.1, 2, 10));
  dialog->spnPaperHeight = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjPaperHeight), 1, 0);
  gtk_widget_ref (dialog->spnPaperHeight);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnPaperHeight", dialog->spnPaperHeight,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnPaperHeight);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->spnPaperHeight, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnPaperHeight), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnPaperHeight), 2) ;

  dialog->lblPaperHeight = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblPaperHeight);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPaperHeight", dialog->lblPaperHeight,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPaperHeight);
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->lblPaperHeight, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblPaperHeight), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPaperHeight), 0.0, 0.5);
  
  dialog->lblNote = gtk_label_new (_(
    "Note: If you change the paper size, please also have a look at the margins. "
    "They may have changed as a result of your changing the paper size.")) ;
  gtk_widget_ref (dialog->lblNote) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblNote", dialog->lblNote,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblNote) ;
  gtk_table_attach (GTK_TABLE (dialog->tblPaperSize), dialog->lblNote, 0, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblNote), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblNote), 0.0, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (dialog->lblNote), TRUE) ;

  dialog->label3 = gtk_label_new (_("Paper Size"));
  gtk_widget_ref (dialog->label3);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label3", dialog->label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook), 1), dialog->label3);

  /* Tab 2 - Margins */
  dialog->tblMargins = gtk_table_new (4, 3, FALSE);
  gtk_widget_ref (dialog->tblMargins);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblMargins", dialog->tblMargins,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblMargins);
  gtk_container_add (GTK_CONTAINER (dialog->notebook), dialog->tblMargins);

  dialog->label10 = gtk_label_new (_("Left Margin:"));
  gtk_widget_ref (dialog->label10);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label10", dialog->label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label10);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->label10, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label10), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label10), 1, 0.5);

  dialog->label11 = gtk_label_new (_("Top Margin:"));
  gtk_widget_ref (dialog->label11);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label11", dialog->label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label11);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->label11, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label11), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label11), 1, 0.5);

  dialog->label12 = gtk_label_new (_("Right Margin:"));
  gtk_widget_ref (dialog->label12);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label12", dialog->label12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label12);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->label12, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label12), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label12), 1, 0.5);

  dialog->label13 = gtk_label_new (_("Bottom Margin:"));
  gtk_widget_ref (dialog->label13);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label13", dialog->label13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label13);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->label13, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label13), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label13), 1, 0.5);

  dialog->adjLeftMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  dialog->spnLeftMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjLeftMargin), 1, 0);
  gtk_widget_ref (dialog->spnLeftMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnLeftMargin", dialog->spnLeftMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnLeftMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->spnLeftMargin, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnLeftMargin), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnLeftMargin), 2) ;

  dialog->adjTopMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  dialog->spnTopMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjTopMargin), 1, 0);
  gtk_widget_ref (dialog->spnTopMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnTopMargin", dialog->spnTopMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnTopMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->spnTopMargin, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnTopMargin), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnTopMargin), 2) ;

  dialog->adjRightMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  dialog->spnRightMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjRightMargin), 1, 0);
  gtk_widget_ref (dialog->spnRightMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnRightMargin", dialog->spnRightMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnRightMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->spnRightMargin, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnRightMargin), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnRightMargin), 2) ;

  dialog->adjBottomMargin = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1000, 0.1, 2, 10));
  dialog->spnBottomMargin = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjBottomMargin), 1, 0);
  gtk_widget_ref (dialog->spnBottomMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnBottomMargin", dialog->spnBottomMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnBottomMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->spnBottomMargin, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnBottomMargin), TRUE) ;
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (dialog->spnBottomMargin), 2) ;

  dialog->lblLeftMargin = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblLeftMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblLeftMargin", dialog->lblLeftMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblLeftMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->lblLeftMargin, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblLeftMargin), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dialog->lblLeftMargin), GTK_JUSTIFY_LEFT);

  dialog->lblTopMargin = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblTopMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblTopMargin", dialog->lblTopMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblTopMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->lblTopMargin, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblTopMargin), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dialog->lblTopMargin), GTK_JUSTIFY_LEFT);

  dialog->lblRightMargin = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblRightMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblRightMargin", dialog->lblRightMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblRightMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->lblRightMargin, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblRightMargin), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dialog->lblRightMargin), GTK_JUSTIFY_LEFT);

  dialog->lblBottomMargin = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblBottomMargin);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblBottomMargin", dialog->lblBottomMargin,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblBottomMargin);
  gtk_table_attach (GTK_TABLE (dialog->tblMargins), dialog->lblBottomMargin, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblBottomMargin), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dialog->lblBottomMargin), GTK_JUSTIFY_LEFT);

  dialog->label4 = gtk_label_new (_("Margins"));
  gtk_widget_ref (dialog->label4);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "label4", dialog->label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook), 2), dialog->label4);
  
  /* Tab 3 - Scale */
  dialog->tblScale = gtk_table_new (2, 3, FALSE);
  gtk_widget_ref (dialog->tblScale);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblScale", dialog->tblScale,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblScale);
  gtk_container_add (GTK_CONTAINER (dialog->notebook), dialog->tblScale) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblScale), 2);

  dialog->fmScale = gtk_frame_new (_("Scale"));
  gtk_widget_ref (dialog->fmScale);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmScale", dialog->fmScale,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->fmScale, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmScale), 2);

  dialog->table2 = gtk_table_new (1, 3, FALSE);
  gtk_widget_ref (dialog->table2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "table2", dialog->table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table2);
  gtk_container_add (GTK_CONTAINER (dialog->fmScale), dialog->table2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table2), 2);

  dialog->lblNanoIs = gtk_label_new (_("1 nm is"));
  gtk_widget_ref (dialog->lblNanoIs);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblNanoIs", dialog->lblNanoIs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblNanoIs);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->lblNanoIs, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblNanoIs), 1, 0.5);

  dialog->lblScale = gtk_label_new (_("cm"));
  gtk_widget_ref (dialog->lblScale);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblScale", dialog->lblScale,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblScale);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->lblScale, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblScale), 0, 0.5);

  dialog->adjNanoToUnits = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 100, 0.01, 1, 1));
  dialog->spnNanoToUnits = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjNanoToUnits), 1, 3);
  gtk_widget_ref (dialog->spnNanoToUnits);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnNanoToUnits", dialog->spnNanoToUnits,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnNanoToUnits);
  gtk_table_attach (GTK_TABLE (dialog->table2), dialog->spnNanoToUnits, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), TRUE);

  dialog->fmFit = gtk_frame_new (_("Number of Pages"));
  gtk_widget_ref (dialog->fmFit);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmFit", dialog->fmFit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmFit);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->fmFit, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmFit), 2);

  dialog->table3 = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (dialog->table3);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "table3", dialog->table3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table3);
  gtk_container_add (GTK_CONTAINER (dialog->fmFit), dialog->table3);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table3), 2);

  dialog->lblPgsWide = gtk_label_new (_("page(s) wide"));
  gtk_widget_ref (dialog->lblPgsWide);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPgsWide", dialog->lblPgsWide,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPgsWide);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->lblPgsWide, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPgsWide), 0, 0.5);

  dialog->adjCYPages = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10));
  dialog->spnCYPages = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjCYPages), 1, 0);
  gtk_widget_ref (dialog->spnCYPages);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnCYPages", dialog->spnCYPages,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnCYPages);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->spnCYPages, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);

  dialog->adjCXPages = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10));
  dialog->spnCXPages = gtk_spin_button_new (GTK_ADJUSTMENT (dialog->adjCXPages), 1, 0);
  gtk_widget_ref (dialog->spnCXPages);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "spnCXPages", dialog->spnCXPages,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->spnCXPages);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->spnCXPages, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 0);

  dialog->lblPgsTall = gtk_label_new (_("page(s) tall"));
  gtk_widget_ref (dialog->lblPgsTall);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPgsTall", dialog->lblPgsTall,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPgsTall);
  gtk_table_attach (GTK_TABLE (dialog->table3), dialog->lblPgsTall, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPgsTall), 0, 0.5);

  dialog->rbFixedScale = gtk_radio_button_new_with_label (dialog->grpScaleOpts, _("Fixed"));
  dialog->grpScaleOpts = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbFixedScale));
  gtk_widget_ref (dialog->rbFixedScale);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "rbFixedScale", dialog->rbFixedScale,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->rbFixedScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->rbFixedScale, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);

  dialog->rbFitPages = gtk_radio_button_new_with_label (dialog->grpScaleOpts, _("Fit"));
  dialog->grpScaleOpts = gtk_radio_button_group (GTK_RADIO_BUTTON (dialog->rbFitPages));
  gtk_widget_ref (dialog->rbFitPages);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "rbFitPages", dialog->rbFitPages,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->rbFitPages);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->rbFitPages, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);

  dialog->vbScale = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (dialog->vbScale);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "vbScale", dialog->vbScale,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbScale);
  gtk_table_attach (GTK_TABLE (dialog->tblScale), dialog->vbScale, 2, 3, 0, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);

  dialog->fmPrintOrder = gtk_frame_new (_("Print Order"));
  gtk_widget_ref (dialog->fmPrintOrder);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmPrintOrder", dialog->fmPrintOrder,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmPrintOrder);
  gtk_box_pack_start (GTK_BOX (dialog->vbScale), dialog->fmPrintOrder, TRUE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmPrintOrder), 2);

  dialog->table4 = gtk_table_new (2, 1, FALSE);
  gtk_widget_ref (dialog->table4);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "table4", dialog->table4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->table4);
  gtk_container_add (GTK_CONTAINER (dialog->fmPrintOrder), dialog->table4);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table4), 2);

  dialog->tbtnPrintOrder = create_two_pixmap_toggle_button (
    GTK_PIXMAP (create_pixmap (dialog->dlgPrintProps, "print_over_then_down.xpm")),
    GTK_PIXMAP (create_pixmap (dialog->dlgPrintProps, "print_down_then_over.xpm")), NULL) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tbtnPrintOrder", dialog->tbtnPrintOrder,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tbtnPrintOrder);
  gtk_table_attach (GTK_TABLE (dialog->table4), dialog->tbtnPrintOrder, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblPrintOrder = gtk_label_new (_("Over, then down"));
  gtk_widget_ref (dialog->lblPrintOrder);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPrintOrder", dialog->lblPrintOrder,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPrintOrder);
  gtk_table_attach (GTK_TABLE (dialog->table4), dialog->lblPrintOrder, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblPrintOrder), 0.5, 1);
  
  dialog->fmCenter = gtk_frame_new (_("Center On Pages"));
  gtk_widget_ref (dialog->fmCenter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmCenter", dialog->fmCenter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmCenter);
  gtk_box_pack_start (GTK_BOX (dialog->vbScale), dialog->fmCenter, TRUE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmCenter), 2);

  dialog->tblCenter = gtk_table_new (2, 1, FALSE);
  gtk_widget_ref (dialog->tblCenter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tblCenter", dialog->tblCenter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblCenter);
  gtk_container_add (GTK_CONTAINER (dialog->fmCenter), dialog->tblCenter);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblCenter), 2);

  dialog->tbtnCenter = create_two_pixmap_toggle_button (
    GTK_PIXMAP (create_pixmap (dialog->dlgPrintProps, "no_center_on_pages.xpm")),
    GTK_PIXMAP (create_pixmap (dialog->dlgPrintProps, "center_on_pages.xpm")), NULL) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "tbtnCenter", dialog->tbtnCenter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tbtnCenter);
  gtk_table_attach (GTK_TABLE (dialog->tblCenter), dialog->tbtnCenter, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblCenter = gtk_label_new (_("Do Not Center"));
  gtk_widget_ref (dialog->lblCenter);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblCenter", dialog->lblCenter,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblCenter);
  gtk_table_attach (GTK_TABLE (dialog->tblCenter), dialog->lblCenter, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblCenter), 0.5, 1);

  dialog->lblScaleTab = gtk_label_new (_("Scale"));
  gtk_widget_ref (dialog->lblScaleTab);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblScaleTab", dialog->lblScaleTab,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblScaleTab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook), 3), dialog->lblScaleTab);
  
  /* Tab 4 - Printed Objects */
  dialog->scrwPrintedObjs = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (dialog->scrwPrintedObjs);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "scrwPrintedObjs", dialog->scrwPrintedObjs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->scrwPrintedObjs);
  gtk_container_add (GTK_CONTAINER (dialog->notebook), dialog->scrwPrintedObjs);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->scrwPrintedObjs), 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->scrwPrintedObjs), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->vpPrintedObjs = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (dialog->vpPrintedObjs);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "vpPrintedObjs", dialog->vpPrintedObjs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vpPrintedObjs);
  gtk_container_add (GTK_CONTAINER (dialog->scrwPrintedObjs), dialog->vpPrintedObjs);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vpPrintedObjs), 2);

  dialog->vbPrintedObjs = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (dialog->vbPrintedObjs);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "vbPrintedObjs", dialog->vbPrintedObjs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbPrintedObjs);
  gtk_container_add (GTK_CONTAINER (dialog->vpPrintedObjs), dialog->vbPrintedObjs);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbPrintedObjs), 2);

  dialog->lblPrintedObjs = gtk_label_new (_("Printed Objects"));
  gtk_widget_ref (dialog->lblPrintedObjs);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "lblPrintedObjs", dialog->lblPrintedObjs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblPrintedObjs);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (dialog->notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (dialog->notebook), 4), dialog->lblPrintedObjs);
  
  dialog->fmPreview = gtk_frame_new (NULL) ;
  gtk_widget_ref (dialog->fmPreview) ;
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "fmPreview", dialog->fmPreview,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->fmPreview) ;
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->fmPreview, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_frame_set_shadow_type (GTK_FRAME (dialog->fmPreview), GTK_SHADOW_IN) ;
  
  dialog->daPreview = gtk_drawing_area_new ();
  gtk_widget_ref (dialog->daPreview);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "daPreview", dialog->daPreview,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->daPreview);
  gtk_container_add (GTK_CONTAINER (dialog->fmPreview), dialog->daPreview) ;
  gtk_widget_set_usize (dialog->daPreview, 200, 200) ;
  
  dialog->dlgAA = GTK_DIALOG (dialog->dlgPrintProps)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "dlgAA", dialog->dlgAA);
  gtk_widget_show (dialog->dlgAA);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dlgAA), 0);

  dialog->hbox2 = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbox2);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "hbox2", dialog->hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbox2);
  gtk_box_pack_start (GTK_BOX (dialog->dlgAA), dialog->hbox2, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox2), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox2), 0, 0);

  dialog->btnPreview = gtk_button_new_with_label (_("Preview"));
  gtk_widget_ref (dialog->btnPreview);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "btnPreview", dialog->btnPreview,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnPreview);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->btnPreview) ;
  GTK_WIDGET_SET_FLAGS (dialog->btnPreview, GTK_CAN_DEFAULT);
  
  dialog->btnOK = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->btnOK);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "btnOK", dialog->btnOK,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnOK);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->btnOK) ;
  GTK_WIDGET_SET_FLAGS (dialog->btnOK, GTK_CAN_DEFAULT);
  
  dialog->btnCancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->btnCancel);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgPrintProps), "btnCancel", dialog->btnCancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnCancel);
  gtk_container_add (GTK_CONTAINER (dialog->hbox2), dialog->btnCancel) ;
  GTK_WIDGET_SET_FLAGS (dialog->btnCancel, GTK_CAN_DEFAULT);

  /* The main buttons */
  gtk_signal_connect (GTK_OBJECT (dialog->btnOK), "clicked", GTK_SIGNAL_FUNC (on_print_properties_dialog_btnOK_clicked), GTK_OBJECT (dialog->dlgPrintProps));
  gtk_signal_connect (GTK_OBJECT (dialog->btnPreview), "clicked", GTK_SIGNAL_FUNC (on_print_properties_dialog_btnPreview_clicked), GTK_OBJECT (dialog->dlgPrintProps));
  gtk_signal_connect_object (GTK_OBJECT (dialog->btnCancel), "clicked", GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dialog->dlgPrintProps));
  
  /* Changing units */
  gtk_signal_connect (GTK_OBJECT (dialog->cbmPUCentis), "activate", GTK_SIGNAL_FUNC (change_preferred_units),  dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->cbmPUInches), "activate", GTK_SIGNAL_FUNC (change_preferred_units), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->cbmPUPoints), "activate", GTK_SIGNAL_FUNC (change_preferred_units), dialog->dlgPrintProps) ;

  /* The various spin buttons */  
  gtk_signal_connect (GTK_OBJECT (dialog->adjPaperWidth), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjPaperHeight), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjLeftMargin), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjTopMargin), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjRightMargin), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjBottomMargin), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjNanoToUnits), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjCXPages), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->adjCYPages), "value_changed", GTK_SIGNAL_FUNC (validate_value_change), dialog->dlgPrintProps) ;

  /* Painting the preview window */
  gtk_signal_connect (GTK_OBJECT (dialog->daPreview), "expose_event", GTK_SIGNAL_FUNC (on_daPreview_expose), dialog->dlgPrintProps) ;

  gtk_signal_connect (GTK_OBJECT (dialog->tbtnPrintOrder), "toggled", GTK_SIGNAL_FUNC (on_tbtnPrintOrder_toggled), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->tbtnCenter), "toggled", GTK_SIGNAL_FUNC (on_tbtnCenter_toggled), dialog->dlgPrintProps) ;

  gtk_signal_connect (GTK_OBJECT (dialog->rbFixedScale), "toggled", GTK_SIGNAL_FUNC (toggle_scale_mode), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->rbFitPages), "toggled", GTK_SIGNAL_FUNC (toggle_scale_mode), dialog->dlgPrintProps) ;

  gtk_signal_connect (GTK_OBJECT (dialog->rbPrintFile), "toggled", GTK_SIGNAL_FUNC (toggle_print_mode), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->rbPrintPipe), "toggled", GTK_SIGNAL_FUNC (toggle_print_mode), dialog->dlgPrintProps) ;

  gtk_signal_connect (GTK_OBJECT (dialog->btnFNBrowse), "clicked", GTK_SIGNAL_FUNC (browse_for_output), dialog->dlgPrintProps) ;
  gtk_signal_connect (GTK_OBJECT (dialog->btnPipeCmdBrowse), "clicked", GTK_SIGNAL_FUNC (browse_for_output), dialog->dlgPrintProps) ;
  // connect the destroy function for when the user clicks the "x" to close the window //
  gtk_signal_connect_object (GTK_OBJECT (dialog->dlgPrintProps), "delete_event", GTK_SIGNAL_FUNC (gtk_widget_hide), GTK_OBJECT (dialog->dlgPrintProps));
  }

/* initialize the dialog - whether to display it, or to simply ensure correct print_op values */
void init_print_properties_dialog (print_properties_D *dialog, print_OP *print_op, GtkWindow *parent, qcell *first_cell, gboolean *pbOK)
  {
  int Nix ;
  double factor ;
  
  if (NULL == dialog->dlgPrintProps)
    create_print_properties_dialog (dialog) ;
    
  /* The static data needs to be set right away, because signals will come up empty */
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "first_cell", first_cell) ;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "pbOK", pbOK) ;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "ppo", print_op) ;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgPrintProps), "dialog", dialog) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (dialog->dlgPrintProps), parent) ;

  old_preferred_units_menu_item = GTK_OPTION_MENU (dialog->cbPrefUnits)->menu_item ;

  get_extents (&world_extents_x1, &world_extents_y1, &world_extents_x2, &world_extents_y2) ;
  
  factor = get_conversion_factor (dialog->cbmPUPoints, old_preferred_units_menu_item, dialog) ;

  /* Fill in the dialog from the print_op values */
  bSpinButtonsDoNotReact = TRUE ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnPaperWidth), print_op->dPaperWidth * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnPaperHeight), print_op->dPaperHeight * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnLeftMargin), print_op->dLeftMargin * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnTopMargin), print_op->dTopMargin * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnRightMargin), print_op->dRightMargin * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnBottomMargin), print_op->dBottomMargin * factor) ;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), print_op->dPointsPerNano * factor) ;
  bSpinButtonsDoNotReact = FALSE ;
  
  if (NULL != dialog->ppPrintedObjs)
    for (Nix = 0 ; Nix < print_op->icPrintedObjs ; Nix++)
      gtk_widget_destroy (dialog->ppPrintedObjs[Nix]) ;

  fill_printed_objects_list (dialog->lstPrintedObjs, dialog) ;
  
  for (Nix = 0 ; Nix < print_op->icPrintedObjs ; Nix++)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix]),
      print_op->pbPrintedObjs[Nix]) ;
    gtk_signal_connect (GTK_OBJECT (dialog->ppPrintedObjs[Nix]), "toggled",
      (GtkSignalFunc)chkPrintedObj_toggled, dialog->dlgPrintProps) ;
    }
    
  /* If there are no cells in the design, disallow checking the "The Cells" checkbox by
     programmatically unchecking it and then disabling it */
  if (NULL == first_cell)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS]), FALSE) ;
    gtk_widget_set_sensitive (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS], FALSE) ;
    }
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (print_op->bPrintFile ? dialog->rbPrintFile : dialog->rbPrintPipe), TRUE) ;
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnPrintOrder), !print_op->bPrintOrderOver) ;
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->tbtnCenter), print_op->bCenter) ;
  
  gtk_entry_set_text (GTK_ENTRY (print_op->bPrintFile ? dialog->txtFName : dialog->txtPipeCmd), print_op->szPrintString) ;
  
  toggle_scale_mode (NULL, dialog->dlgPrintProps) ;
  toggle_print_mode (NULL, dialog->dlgPrintProps) ;
  check_status (dialog) ;
  }

void chkPrintedObj_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  int Nix ;
  
  /* Ensure that there's always /something/ to print */
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE])))
    {
    for (Nix = 0 ; Nix < dialog->icPrintedObjs ; Nix++)
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[Nix])) &&
          Nix != PRINTED_OBJECTS_COLOURS)
	break ;

    if (Nix >= dialog->icPrintedObjs)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE]), TRUE) ;
    }
  
  bSpinButtonsDoNotReact = TRUE ;
  check_scale (dialog) ;
  bSpinButtonsDoNotReact = FALSE ;
  }

void change_preferred_units (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  double dFactor ;
  char *pszShortName = NULL ;
  GtkWidget *wConv = old_preferred_units_menu_item ;
  
  if (old_preferred_units_menu_item == widget) return ;
  
  
  /* THIS IS A HACK ! Why doesn't the conversion factor from pt to cm and vice versa
     work properly ? */
  if ((dialog->cbmPUCentis == old_preferred_units_menu_item && dialog->cbmPUPoints == widget) ||
      (dialog->cbmPUPoints == old_preferred_units_menu_item && dialog->cbmPUCentis == widget))
    change_preferred_units (wConv = dialog->cbmPUInches, user_data) ;
    
  dFactor = get_conversion_factor (wConv, widget, dialog) ;
  
  pszShortName = gtk_object_get_data (GTK_OBJECT (widget), "short_name") ;
  
  bSpinButtonsDoNotReact = TRUE ;
  
  if (widget == dialog->cbmPUPoints)
    {
    adjustment_set_value_and_step (dialog->adjPaperWidth,
      ceil (dialog->adjPaperWidth->value * dFactor), 1.0) ;
    adjustment_set_value_and_step (dialog->adjPaperHeight,
      ceil (dialog->adjPaperHeight->value * dFactor), 1.0) ;
    
    /* The following conversions must be floor and not ceil in order to
       preserve the MIN_MARGIN_GAP */
      
    adjustment_set_value_and_step (dialog->adjLeftMargin,
      floor (dialog->adjLeftMargin->value * dFactor), 1.0) ;
    adjustment_set_value_and_step (dialog->adjTopMargin,
      floor (dialog->adjTopMargin->value * dFactor), 1.0) ;
    adjustment_set_value_and_step (dialog->adjRightMargin,
      floor (dialog->adjRightMargin->value * dFactor), 1.0) ;
    adjustment_set_value_and_step (dialog->adjBottomMargin,
      floor (dialog->adjBottomMargin->value * dFactor), 1.0) ;
    }
  else
    {
    adjustment_set_value_and_step (dialog->adjPaperWidth,
      dialog->adjPaperWidth->value * dFactor, 0.1) ;
    adjustment_set_value_and_step (dialog->adjPaperHeight,
      dialog->adjPaperHeight->value * dFactor, 0.1) ;
    adjustment_set_value_and_step (dialog->adjLeftMargin,
      dialog->adjLeftMargin->value * dFactor, 0.1) ;
    adjustment_set_value_and_step (dialog->adjTopMargin,
      dialog->adjTopMargin->value * dFactor, 0.1) ;
    adjustment_set_value_and_step (dialog->adjRightMargin,
      dialog->adjRightMargin->value * dFactor, 0.1) ;
    adjustment_set_value_and_step (dialog->adjBottomMargin,
      dialog->adjBottomMargin->value * dFactor, 0.1) ;
    }

  adjustment_set_value_and_step (dialog->adjNanoToUnits,
    dialog->adjNanoToUnits->value * dFactor, 0.01) ;

  bSpinButtonsDoNotReact = FALSE ;
  
  gtk_label_set_text (GTK_LABEL (dialog->lblPaperWidth), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblPaperHeight), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblLeftMargin), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblTopMargin), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblRightMargin), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblBottomMargin), pszShortName) ;
  gtk_label_set_text (GTK_LABEL (dialog->lblScale), pszShortName) ;
  
  old_preferred_units_menu_item = widget ;
  }

void adjustment_set_value_and_step (GtkAdjustment *padj, double dValue, double dStep)
  {
  padj->value = dValue ;
  padj->step_increment = dStep ;
  gtk_adjustment_changed (padj) ;
  gtk_adjustment_value_changed (padj) ;
  }

double get_conversion_factor (GtkWidget *old_widget, GtkWidget *new_widget, print_properties_D *dialog)
  {
  if (dialog->cbmPUCentis == old_widget && dialog->cbmPUInches == new_widget)
    return 1.0 / 2.54 ;
  else if (dialog->cbmPUInches == old_widget && dialog->cbmPUCentis == new_widget)
    return 2.54 ;
  else if (dialog->cbmPUInches == old_widget && dialog->cbmPUPoints == new_widget)
    return 72.0 ;
  else if (dialog->cbmPUPoints == old_widget && dialog->cbmPUInches == new_widget)
    return 1.0 / 72.0 ;
  else if (dialog->cbmPUCentis == old_widget && dialog->cbmPUPoints == new_widget)
    return 72.0 / 2.54 ;
  else if (dialog->cbmPUPoints == old_widget && dialog->cbmPUCentis == new_widget)
    return 2.54 / 72.0 ;
  
  /* if all else fails ... */
  return 1 ;
  }

gboolean check_status (print_properties_D *dialog)
  {
  if ((gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbPrintFile)) && 
       0 == strlen (gtk_entry_get_text (GTK_ENTRY (dialog->txtFName)))))
    return STATUS_NEED_FILE_NAME ;
  else if ((gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbPrintPipe)) && 
       0 == strlen (gtk_entry_get_text (GTK_ENTRY (dialog->txtPipeCmd)))))
    return STATUS_NEED_PIPE ;
  return STATUS_OK ;
  }

/* Used when the print properties dialog is to be initialized but not displayed - like
   when the user clicks "Preview" before ever having used the dialog.  Also used for
   filling out the print_OP structure */
void init_print_options (print_OP *pPrintOp, qcell *first_cell)
  {
  double dFactor ;
  int Nix ;
  gboolean bOK = FALSE ;

  if (NULL == print_properties.dlgPrintProps)
    {
    create_print_properties_dialog (&print_properties) ;
    init_print_properties_dialog (&print_properties, pPrintOp, NULL, first_cell, &bOK) ;
    }
  
  dFactor = get_conversion_factor (old_preferred_units_menu_item, print_properties.cbmPUPoints, &print_properties) ;

  /* Print to file ? */
  pPrintOp->bPrintFile = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.rbPrintFile)) ;
  
  /* The filename/command to print to */
  g_snprintf (pPrintOp->szPrintString, 2048, "%s",
    gtk_entry_get_text (GTK_ENTRY (pPrintOp->bPrintFile ? print_properties.txtFName : print_properties.txtPipeCmd))) ;
  
  /* paper width */
  pPrintOp->dPaperWidth =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnPaperWidth)) ;
  
  /* paper height */
  pPrintOp->dPaperHeight =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnPaperHeight)) ;
  
  /*left margin */
  pPrintOp->dLeftMargin = 
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnLeftMargin)) ;
  
  /* top margin */
  pPrintOp->dTopMargin =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnTopMargin)) ;
  
  /* right margin */
  pPrintOp->dRightMargin =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnRightMargin)) ;
  
  /* bottom margin */
  pPrintOp->dBottomMargin =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnBottomMargin)) ;

  /* points per nanometer */
  pPrintOp->dPointsPerNano =
    dFactor * gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_properties.spnNanoToUnits)) ;
  
  /* The various layers */
  for (Nix = 0 ; Nix < print_properties.icPrintedObjs ; Nix++)
    pPrintOp->pbPrintedObjs[Nix] =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.ppPrintedObjs[Nix])) ;
  
  /* Print over than down ? */
  pPrintOp->bPrintOrderOver = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnPrintOrder)) ;
  
  /* Center on pages ? */
  pPrintOp->bCenter = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_properties.tbtnCenter)) ;
  
  /* Number of horizontal pages - takes precedence over the scaling factor */
  pPrintOp->iCXPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCXPages)) ;
  
  /* Number of vertical pages - takes precedence over the scaling factor */
  pPrintOp->iCYPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (print_properties.spnCYPages)) ;
  }

void on_print_properties_dialog_btnOK_clicked(GtkButton *button, gpointer user_data)
  {
  int status ;
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  print_OP *ppo = (print_OP *)gtk_object_get_data (GTK_OBJECT (dialog->dlgPrintProps), "ppo") ;
  gboolean *pbOK = (gboolean *)gtk_object_get_data (GTK_OBJECT (dialog->dlgPrintProps), "pbOK") ;
  
  if (STATUS_OK != (status = check_status (dialog)))
    {
    message_box (GTK_WINDOW (dialog->dlgPrintProps), MB_OK, "Error",
      STATUS_NEED_FILE_NAME == status ? "Please specify a file name." :
      STATUS_NEED_PIPE == status ? "Please specify a print command." :
      "Unknown error.  Cannot print.") ;
    return ;
    }
  
  /* Filling in the print_options structure before calling the print function */

  init_print_options (ppo, (qcell *)gtk_object_get_data (GTK_OBJECT (user_data), "first_cell")) ;  
  *pbOK = TRUE ;
  gtk_widget_hide(GTK_WIDGET(dialog->dlgPrintProps));
  }

void on_tbtnPrintOrder_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblPrintOrder),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    "Down, then over" : "Over, then down") ;
  }

void on_tbtnCenter_toggled (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gtk_label_set_text (GTK_LABEL (dialog->lblCenter),
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ?
    "Center" : "Do Not Center") ;
  }

void on_daPreview_expose (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  
  redraw_preview (dialog->daPreview, 
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperWidth)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperHeight)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnLeftMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnTopMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnRightMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnBottomMargin))) ;
  }

void redraw_preview (GtkWidget *daPreview, double dPaperWidth, double dPaperHeight, 
  double dLeftMargin, double dTopMargin, double dRightMargin, double dBottomMargin)
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

void calc_world_size (qcell *first_cell, int *piWidth, int *piHeight, print_properties_D *dialog)
  {
  int xMin = 0, yMin = 0, xMax = 0, yMax = 0 ;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS])))
    {
    if (NULL != first_cell)
      {
      xMin = world_extents_x1 ;
      yMin = world_extents_y1 ;
      xMax = world_extents_x2 ;
      yMax = world_extents_y2 ;
      }
    }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE])))
    {
    if (xMin > 0) xMin = 0 ;
    if (yMin > 0) yMin = 0 ;
    if (xMax < subs_width) xMax = subs_width ;
    if (yMax < subs_height) yMax = subs_height ;
    }
  *piWidth = xMax - xMin ;
  *piHeight = yMax - yMin ;
  }

/* So far, there are only 3 hardcoded layers */
void fill_printed_objects_list (GtkWidget *ls, print_properties_D *dialog)
  {
  if (NULL == dialog->ppPrintedObjs)
    {
    dialog->icPrintedObjs = 3 ;
    dialog->ppPrintedObjs = malloc (3 * sizeof (GtkWidget *)) ;
    }

  dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE] = gtk_check_button_new_with_label ("The Die") ;
  gtk_widget_ref (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE]) ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_DIE], FALSE, FALSE, 0) ;
  
  dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS] = gtk_check_button_new_with_label ("The Cells") ;
  gtk_widget_ref (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS]) ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_CELLS], FALSE, FALSE, 0) ;
  
  dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS] = gtk_check_button_new_with_label ("Colours") ;
  gtk_widget_ref (dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS]) ;
  gtk_widget_show (dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS]) ;
  gtk_box_pack_start (GTK_BOX (dialog->vbPrintedObjs), dialog->ppPrintedObjs[PRINTED_OBJECTS_COLOURS], FALSE, FALSE, 0) ;
  }

void toggle_print_mode (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gboolean bPipe = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbPrintPipe)) ;
  
  gtk_widget_set_sensitive (dialog->fmFileSelect, !bPipe) ;
  gtk_widget_set_sensitive (dialog->lblFName, !bPipe) ;
  gtk_widget_set_sensitive (dialog->txtFName, !bPipe) ;
  gtk_widget_set_sensitive (dialog->btnFNBrowse, !bPipe) ;
  
  gtk_widget_set_sensitive (dialog->fmPipeSelect, bPipe) ;
  gtk_widget_set_sensitive (dialog->lblPipeCmd, bPipe) ;
  gtk_widget_set_sensitive (dialog->txtPipeCmd, bPipe) ;
  gtk_widget_set_sensitive (dialog->lblPipeNote, bPipe) ;
  gtk_widget_set_sensitive (dialog->btnPipeCmdBrowse, bPipe) ;
  }

void toggle_scale_mode (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  gboolean bAuto = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFitPages)) ;
  
  gtk_widget_set_sensitive (dialog->fmScale, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblNanoIs, !bAuto) ;
  gtk_widget_set_sensitive (dialog->spnNanoToUnits, !bAuto) ;
  gtk_widget_set_sensitive (dialog->lblScale, !bAuto) ;
  
  gtk_widget_set_sensitive (dialog->fmFit, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCXPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->spnCYPages, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsWide, bAuto) ;
  gtk_widget_set_sensitive (dialog->lblPgsTall, bAuto) ;
  
  bSpinButtonsDoNotReact = TRUE ;
  check_scale (dialog) ;
  bSpinButtonsDoNotReact = FALSE ;
  }

/* Make sure the margins do not overlap */
void check_margins (print_properties_D *dialog, double dLRatio, double dRRatio, double dTRatio, double dBRatio)
  {
  double
    diff = 0,
    dLeftMargin   = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnLeftMargin)),
    dTopMargin    = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnTopMargin)),
    dRightMargin  = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnRightMargin)),
    dBottomMargin = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnBottomMargin)),
    dPaperWidth   = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperWidth)),
    dPaperHeight  = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperHeight)),
    dFactor = get_conversion_factor (dialog->cbmPUPoints, old_preferred_units_menu_item, dialog) ;
    
  /* Horizontal */
  if ((diff = dLeftMargin + dRightMargin + MIN_MARGIN_GAP * dFactor - dPaperWidth) > 0)
    {
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnLeftMargin),
      dLeftMargin - (diff * dLRatio)) ;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnRightMargin),
      dRightMargin - (diff * dRRatio)) ;
    }
  
  /* Vertical */
  if ((diff = dTopMargin + dBottomMargin + MIN_MARGIN_GAP * dFactor - dPaperHeight ) > 0)
    {
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnTopMargin),
      dTopMargin - (diff * dTRatio)) ;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnBottomMargin),
      dBottomMargin - (diff * dBRatio)) ;
    }
  }

/* Make sure the scale and the number of pages tall/wide agree */
void check_scale (print_properties_D *dialog)
  {
  int iWidthNano = 0, iHeightNano = 0 ;
  gboolean bEnablePrintOrder = FALSE ;

  calc_world_size ((qcell *)gtk_object_get_data (GTK_OBJECT (dialog->dlgPrintProps), "first_cell"), &iWidthNano, &iHeightNano, dialog) ;
  if (0 == iWidthNano || 0 == iHeightNano)
    {
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), 0.0) ;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCXPages), 0.0) ;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCYPages), 0.0) ;
    return ;
    }
  /* if (TRUE) */
    {
    double  
      dPageWidth = 
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperWidth)) -
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnLeftMargin)) -
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnRightMargin)),
      dPageHeight =
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperHeight)) -
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnTopMargin)) -
	gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnBottomMargin)) ;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbFixedScale)))
      {
      double
	dUnitsPerNano = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnNanoToUnits)),
	dWidth = iWidthNano * dUnitsPerNano,
	dHeight = iHeightNano * dUnitsPerNano ;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCXPages), ceil (dWidth / dPageWidth)) ;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCYPages), ceil (dHeight / dPageHeight)) ;
      }
    else
      {
      int iPageDiff = 0, iCXPages = 0, iCYPages = 0 ;
      double x = 0, y = 0, dWorldWidth = iWidthNano, dWorldHeight = iHeightNano,
        dAvailableHSpace =
	  dPageWidth * (iCXPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages))),
	dAvailableVSpace =
	  dPageHeight * (iCYPages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages))) ;
      
      fit_rect_inside_rect (dAvailableHSpace, dAvailableVSpace, &x, &y, &dWorldWidth, &dWorldHeight) ;
      if ((iPageDiff = (int) floor ((dAvailableHSpace - dWorldWidth) / dPageWidth)) > 0)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCXPages), iCXPages - iPageDiff) ;
      if ((iPageDiff = (int) floor ((dAvailableVSpace - dWorldHeight) / dPageHeight)) > 0)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnCYPages), iCYPages - iPageDiff) ;

      if (0 == x)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), dWorldWidth / iWidthNano) ;
      else
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->spnNanoToUnits), dWorldHeight / iHeightNano) ;
      }
    }

  /* There is no print order if there is only one page */
  bEnablePrintOrder = 
    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCXPages)) > 1 ||
    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->spnCYPages)) > 1 ;
  gtk_widget_set_sensitive (dialog->tbtnPrintOrder, bEnablePrintOrder) ;
  gtk_widget_set_sensitive (dialog->lblPrintOrder, bEnablePrintOrder) ;
  }

/* Ask the user for a filename/pipeline to print to */
void browse_for_output (GtkWidget *widget, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  char szFName[PATH_LENGTH] = "" ;
  gboolean bPrintFile = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rbPrintFile)) ;
  
  if (bPrintFile)
    g_snprintf (szFName, PATH_LENGTH, "%s", gtk_entry_get_text (GTK_ENTRY (dialog->txtFName))) ;
  
  get_file_name_from_user (GTK_WINDOW (dialog->dlgPrintProps),
    bPrintFile ? "Select Filename" : "Select Program", szFName, PATH_LENGTH) ;
  
  if (szFName[0] != 0)
    {
    gtk_entry_set_text (GTK_ENTRY (bPrintFile ? dialog->txtFName : dialog->txtPipeCmd), szFName) ;
    check_status (dialog) ;
    }
  }

/* Make sure all spin buttons everywhere always have correct values */
void validate_value_change (GtkAdjustment *adj_changed, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  
  if (bSpinButtonsDoNotReact) return ;
  
  bSpinButtonsDoNotReact = TRUE ;
  
  if (adj_changed == dialog->adjPaperWidth)
    {
    check_margins (dialog, 0.5, 0.5, 0.0, 0.0) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjPaperHeight)
    {
    check_margins (dialog, 0.0, 0.0, 0.5, 0.5) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjLeftMargin)
    {
    check_margins (dialog, 1.0, 0.0, 0.0, 0.0) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjRightMargin)
    {
    check_margins (dialog, 0.0, 1.0, 0.0, 0.0) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjTopMargin)
    {
    check_margins (dialog, 0.0, 0.0, 1.0, 0.0) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjBottomMargin)
    {
    check_margins (dialog, 0.0, 0.0, 0.0, 1.0) ;
    check_scale (dialog) ;
    }
  else if (adj_changed == dialog->adjNanoToUnits)
    check_scale (dialog) ;
  else if (adj_changed == dialog->adjCXPages)
    check_scale (dialog) ;
  else if (adj_changed == dialog->adjCYPages)
    check_scale (dialog) ;
  
  redraw_preview (dialog->daPreview, 
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperWidth)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnPaperHeight)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnLeftMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnTopMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnRightMargin)),
    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (dialog->spnBottomMargin))) ;
  
  bSpinButtonsDoNotReact = FALSE ;
  }

void on_print_properties_dialog_btnPreview_clicked(GtkButton *button, gpointer user_data)
  {
  print_properties_D *dialog = (print_properties_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  qcell *first_cell = (qcell *)gtk_object_get_data (GTK_OBJECT (user_data), "first_cell") ;
  print_OP po ;
  
  po.pbPrintedObjs = malloc (dialog->icPrintedObjs * sizeof (gboolean)) ;
  po.icPrintedObjs = dialog->icPrintedObjs ;
  
  init_print_options (&po, first_cell) ;
  
  do_print_preview (&po, GTK_WINDOW (dialog->dlgPrintProps), first_cell) ;
  
  free (po.pbPrintedObjs) ;
  }

/* General-purpose function to scale one rectangle until it is inscribed in another rectangle */
void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight)
  {
  double dAspectRatio, dRectAspectRatio ;
  
  if (0 == dWidth || 0 == dHeight || 0 == *pdRectWidth || 0 == *pdRectHeight) return ;
  
  dAspectRatio = dWidth / dHeight ;
  dRectAspectRatio = *pdRectWidth / *pdRectHeight ;
  
  if (dRectAspectRatio > dAspectRatio)
    {
    *px = 0 ;
    *pdRectWidth = dWidth ;
    *pdRectHeight = *pdRectWidth / dRectAspectRatio ;
    *py = (dHeight - *pdRectHeight) / 2 ;
    }
  else
    {
    *py = 0 ;
    *pdRectHeight = dHeight ;
    *pdRectWidth = *pdRectHeight * dRectAspectRatio ;
    *px = (dWidth - *pdRectWidth) / 2 ;
    }
  }
