#include <gtk/gtk.h>
#include "cell_function_dialog.h"
#include "support.h"
#include "gqcell.h"

typedef struct
  {
  GtkWidget *dlgCellFunction ;
  GtkAdjustment *adjPolarization ;
  GtkWidget *spnPolarization ;
  GtkWidget *txtName ;
  GtkWidget *rbNormal ;
  GtkWidget *rbFixed ;
  GtkWidget *rbIO ;
  GtkWidget *rbInput ;
  GtkWidget *rbOutput ;
  GtkWidget *fmFixed ;
  GtkWidget *lblName ;
  GtkWidget *fmIO ;
  } cell_function_D ;

static void create_cell_function_dialog (cell_function_D *dialog) ;
static void cell_mode_toggled (GtkWidget *widget, gpointer data) ;

static cell_function_D cell_function_dialog = {NULL} ;

gboolean get_cell_function_from_user (GtkWidget *parent, GQCell *pqc)
  {
  gboolean bRet = FALSE ;
  GtkToggleButton *tb = NULL ;

  if (NULL == cell_function_dialog.dlgCellFunction)
    create_cell_function_dialog (&cell_function_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (cell_function_dialog.dlgCellFunction), GTK_WINDOW (parent)) ;
  
  g_object_set_data (G_OBJECT (cell_function_dialog.dlgCellFunction), "dialog", &cell_function_dialog) ;
  
  gtk_entry_set_text (GTK_ENTRY (cell_function_dialog.txtName), NULL == pqc->label ? "" : pqc->label) ;
  
  gtk_adjustment_set_value (GTK_ADJUSTMENT (cell_function_dialog.adjPolarization),
    gqcell_calculate_polarization (pqc)) ;
  
  if (pqc->is_input)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cell_function_dialog.rbInput), TRUE) ;
  else
  if (pqc->is_output)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cell_function_dialog.rbOutput), TRUE) ;
  
  gtk_toggle_button_set_active (tb = GTK_TOGGLE_BUTTON (
    pqc->is_input || pqc->is_output ? cell_function_dialog.rbIO :
    pqc->is_fixed ? cell_function_dialog.rbFixed : cell_function_dialog.rbNormal), TRUE) ;
  cell_mode_toggled (GTK_WIDGET (tb), cell_function_dialog.dlgCellFunction) ;
  
  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (cell_function_dialog.dlgCellFunction)))))
    {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cell_function_dialog.rbNormal)))
      gqcell_set_as_normal (pqc) ;
    else
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cell_function_dialog.rbFixed)))
      {
      gqcell_set_as_fixed (pqc) ;
      gqcell_set_polarization (pqc, gtk_adjustment_get_value (GTK_ADJUSTMENT (cell_function_dialog.adjPolarization))) ;
      }
    else // rbIO must be the case
      {
      char *pszLabel = gtk_editable_get_chars (GTK_EDITABLE (cell_function_dialog.txtName), 0, -1) ;
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cell_function_dialog.rbInput)))
        gqcell_set_as_input (pqc) ;
      else
        gqcell_set_as_output (pqc) ;
      gqcell_set_label (pqc, pszLabel) ;
      g_free (pszLabel) ;
      }
    }
  gtk_widget_hide (cell_function_dialog.dlgCellFunction) ;
  return bRet ;
  }

static void create_cell_function_dialog (cell_function_D *dialog)
  {
  GtkWidget *tbl = NULL, *tblFm = NULL, *tbllbl = NULL ;

  dialog->dlgCellFunction = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlgCellFunction), _("Cell Function")) ;
  gtk_window_set_policy (GTK_WINDOW (dialog->dlgCellFunction), FALSE, FALSE, TRUE) ;
  
  tbl = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlgCellFunction)->vbox), tbl, TRUE, TRUE, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;
  
  dialog->rbNormal = gtk_radio_button_new_with_label (NULL, _("Normal Cell")) ;
  gtk_widget_show (dialog->rbNormal) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbNormal, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->rbFixed = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbNormal), 
    _("Fixed Polarization")) ;
  gtk_widget_show (dialog->rbFixed) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbFixed, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->rbIO = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbFixed),
    _("Input/Output")) ;
  gtk_widget_show (dialog->rbIO) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbIO, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->fmFixed = gtk_frame_new (_("Polarization")) ;
  gtk_widget_show (dialog->fmFixed) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->fmFixed, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmFixed), 2) ;

  tblFm = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tblFm) ;
  gtk_container_add (GTK_CONTAINER (dialog->fmFixed), tblFm) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblFm), 2) ;
  
  dialog->adjPolarization = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, -1.00, 1.00, 0.0001, 0.1, 0.1)) ;
  dialog->spnPolarization = gtk_spin_button_new (dialog->adjPolarization, 0.0001, 4) ;
  gtk_widget_show (dialog->spnPolarization) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->spnPolarization, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  
  dialog->fmIO = gtk_frame_new (_("I/O")) ;
  gtk_widget_show (dialog->fmIO) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->fmIO, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmIO), 2) ;
  
  tblFm = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tblFm) ;
  gtk_container_add (GTK_CONTAINER (dialog->fmIO), tblFm) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblFm), 2) ;
  
  dialog->rbInput = gtk_radio_button_new_with_label (NULL, _("Input")) ;
  gtk_widget_show (dialog->rbInput) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->rbInput, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  
  dialog->rbOutput = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbInput), _("Output")) ;
  gtk_widget_show (dialog->rbOutput) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->rbOutput, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  
  tbllbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbllbl) ;
  gtk_table_attach (GTK_TABLE (tblFm), tbllbl, 0, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbllbl), 2) ;
  
  dialog->lblName = gtk_label_new (_("Cell Label:")) ;
  gtk_widget_show (dialog->lblName) ;
  gtk_table_attach (GTK_TABLE (tbllbl), dialog->lblName, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblName), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblName), 1.0, 0.5) ;
  
  dialog->txtName = gtk_entry_new () ;
  gtk_widget_show (dialog->txtName) ;
  gtk_table_attach (GTK_TABLE (tbllbl), dialog->txtName, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->txtName), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgCellFunction), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgCellFunction), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgCellFunction), GTK_RESPONSE_OK) ;
  
  g_signal_connect (G_OBJECT (dialog->rbNormal), "toggled", (GCallback)cell_mode_toggled, dialog->dlgCellFunction) ;
  g_signal_connect (G_OBJECT (dialog->rbFixed), "toggled", (GCallback)cell_mode_toggled, dialog->dlgCellFunction) ;
  g_signal_connect (G_OBJECT (dialog->rbIO), "toggled", (GCallback)cell_mode_toggled, dialog->dlgCellFunction) ;
  }

static void cell_mode_toggled (GtkWidget *widget, gpointer data)
  {
  cell_function_D *dialog = (cell_function_D *)g_object_get_data (G_OBJECT (data), "dialog") ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  gtk_widget_set_sensitive (dialog->fmFixed, dialog->rbFixed == widget) ;
  gtk_widget_set_sensitive (dialog->spnPolarization, dialog->rbFixed == widget) ;
  gtk_widget_set_sensitive (dialog->fmIO, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->rbInput, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->rbOutput, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->lblName, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->txtName, dialog->rbIO == widget) ;
  }
