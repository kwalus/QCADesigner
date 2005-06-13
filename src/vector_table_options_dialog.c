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
// An alternative interface for specifying vector       //
// tables, which works with the new vector table        //
// library.                                             //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "fileio_helpers.h"
#include "support.h"
#include "global_consts.h"
#include "file_selection_window.h"
#include "vector_table_options_dialog.h"

#define VTTBL_X_OFF 1
#define VTTBL_Y_OFF 2
#define VTTBL_H_PAD 1
#define VTTBL_V_PAD 0

#define DBG_NVTO(s)
#define DBG_VTO_SIGNAL(s)

typedef struct
  {
  GtkWidget *cb ;
  GtkWidget *tb ;
  GtkWidget *tblbl ;
  } INPUT_HEADING ;

typedef struct
  {
  GtkWidget *eb ;
  GtkWidget *lbl ;
  } VECTOR_IDX ;

typedef struct
  {
  GtkWidget *tb ;
  GtkWidget *tblbl ;
  } VECTOR_BIT ;

typedef struct
  {
  GtkWidget *dlgVectorTable;
  GtkWidget *dialog_vbox1;
  GtkWidget *tblVTMain;
  GtkWidget *fmVTOps;
  GtkWidget *tblOps;
  GtkWidget *lblMsg;
  GtkWidget *hbVTOps;
#ifdef STDIO_FILEIO
  GtkWidget *btnLoad;
  GtkWidget *btnSave;
  GtkWidget *btnSaveAs;
#endif /* def STDIO_FILEIO */
  GtkWidget *swndVT;
  GtkWidget *viewport1;
  GtkWidget *ebVT;
  GtkWidget *tblVT;
  VECTOR_IDX *pIdx ;
  INPUT_HEADING *pInput ;
  VECTOR_BIT **ppBit ;
  GtkWidget *hbVTFile ;
  GtkWidget *lblVTFileMsg ;
  GtkWidget *lblVTFile;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbuttonbox1;
  GtkWidget *btnOK;
  GtkWidget *btnCancel;
  GtkWidget *mnuVT ;
  GtkWidget *mnuDeact ;
  GtkWidget *mnuAct ;
  GtkWidget *mnuInsBefore ;
  GtkWidget *spInsBefore ;
  GtkWidget *mnuInsAfter ;
  GtkWidget *mnuDel ;
  GtkWidget *mnuAdd ;
  int icInputs ;
  int icInputsUsed ;
  int icVectors ;
  int icVectorsUsed ;
  } vector_table_options_D ;

VectorTable *pvt ;

static vector_table_options_D nvto = {NULL} ;

static void create_vector_table_options_dialog (vector_table_options_D *pnvto) ;
static gboolean Vector_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data) ;
static gboolean VT_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data) ;
static gboolean Bit_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data) ;
static void create_vector (GtkWidget *widget, gpointer user_data) ;
static void delete_vector (GtkWidget *widget, gpointer user_data) ;
static void click_bit_button (GtkWidget *widget, gpointer user_data) ;
static void ActiveFlag_toggled (GtkWidget *widget, gpointer user_data) ;
#ifdef STDIO_FILEIO
static void load_vector_table (GtkWidget *widget, gpointer user_data) ;
static void save_vector_table (GtkWidget *widget, gpointer user_data) ;
#endif /* def STDIO_FILEIO */
static void InputCBEntry_changed (GtkWidget *widget, gpointer user_data) ;
static void InputCBPopwin_show (GtkWidget *widget, gpointer user_data) ;
static void InputCBPopwin_hide (GtkWidget *widget, gpointer user_data) ;

static void VectorTableToDialog (vector_table_options_D *dialog, VectorTable *pvt) ;
static void DialogToVectorTable (vector_table_options_D *dialog, VectorTable *pvt) ;
static void SetColumnActive (vector_table_options_D *dialog, int idx, gboolean bActive) ;
static void CreateIdxLabel (vector_table_options_D *dialog, int idx) ;
static void CreateVectorToggle (vector_table_options_D *dialog, gboolean bValue, int idxRow, int idxCol) ;
static void CreateInputHeading (vector_table_options_D *dialog, VectorTable *pvt, int idx) ;
static void ReuseVectorToggle (vector_table_options_D *dialog, gboolean bValue, int idxRow, int idxCol) ;
static void ReuseInputHeading (vector_table_options_D *dialog, VectorTable *pvt, int idx) ;
static void DestroyIdxLabel (vector_table_options_D *dialog, int idx) ;
static void DestroyVectorToggle (vector_table_options_D *dialog, int idxRow, int idxCol) ;
static void DestroyInputHeading (vector_table_options_D *dialog, int idx) ;
static void CreateVector (vector_table_options_D *dialog, int idx) ;
static void DeleteVector (vector_table_options_D *dialog, int idx) ;
static void SetCurrentFileName (vector_table_options_D *dialog, char *pszFName) ;
static int CountActiveInputs (vector_table_options_D *dialog) ;
static void SwapColumns (vector_table_options_D *dialog, int idxSrc, int idxDst) ;
static int GetInputIdx (vector_table_options_D *dialog, char *pszInput) ;

void get_vector_table_options_from_user (GtkWindow *parent, VectorTable *pvt)
  {
  VectorTable *pvtDlg = VectorTable_copy (pvt) ;
  char *pszCurrentFName = NULL ;

  // We want an inputs-only copy of the vector table passed to us.  It is this pvtDlg that
  // we shall load new vector tables from files into.  Thus, we delete all the vectors from
  // pvtDlg.
  while (pvtDlg->vectors->icUsed > 0)
    VectorTable_del_vector (pvtDlg, pvtDlg->vectors->icUsed - 1) ;

  if (NULL == nvto.dlgVectorTable)
    create_vector_table_options_dialog (&nvto) ;

  g_object_set_data (G_OBJECT (nvto.dlgVectorTable), "dialog", &nvto) ;
  g_object_set_data (G_OBJECT (nvto.dlgVectorTable), "idxInput", (gpointer)-1) ;
  g_object_set_data (G_OBJECT (nvto.dlgVectorTable), "idxVector", (gpointer)-1) ;
  g_object_set_data (G_OBJECT (nvto.dlgVectorTable), "pvtDlg", pvtDlg) ;
  g_object_set_data (G_OBJECT (nvto.dlgVectorTable), "pszCurrentFName", pszCurrentFName) ;

  gtk_window_set_transient_for (GTK_WINDOW (nvto.dlgVectorTable), parent) ;

  VectorTableToDialog (&nvto, pvt) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (nvto.dlgVectorTable)))
    DialogToVectorTable (&nvto, pvt) ;

  gtk_widget_hide (nvto.dlgVectorTable) ;
  VectorTable_free (pvtDlg) ;
  }

static void VectorTableToDialog (vector_table_options_D *dialog, VectorTable *pvt)
  {
  int Nix, Nix1 ;

  if (pvt->inputs->icUsed > dialog->icInputs)
    {
    dialog->pInput = realloc (dialog->pInput, pvt->inputs->icUsed * sizeof (INPUT_HEADING)) ;
    for (Nix = 0 ; Nix < dialog->icVectors ; Nix++)
      dialog->ppBit[Nix] = realloc (dialog->ppBit[Nix], pvt->inputs->icUsed * sizeof (VECTOR_BIT)) ;
    dialog->icInputs = pvt->inputs->icUsed ;
    }

  if (pvt->vectors->icUsed > dialog->icVectors)
    {
    dialog->pIdx = realloc (dialog->pIdx, pvt->vectors->icUsed * sizeof (VECTOR_IDX)) ;
    dialog->ppBit = realloc (dialog->ppBit, pvt->vectors->icUsed * sizeof (VECTOR_BIT *)) ;
    for (Nix = dialog->icVectors ; Nix < pvt->vectors->icUsed ; Nix++)
      dialog->ppBit[Nix] = g_malloc0 (pvt->inputs->icUsed * sizeof (VECTOR_BIT)) ;
    dialog->icVectors = pvt->vectors->icUsed ;
    }

  if (dialog->icInputsUsed > pvt->inputs->icUsed)
    {
    for (Nix = pvt->inputs->icUsed ; Nix < dialog->icInputsUsed ; Nix++)
      {
      DestroyInputHeading (dialog, Nix) ;
      for (Nix1 = 0 ; Nix1 < dialog->icVectorsUsed ; Nix1++)
      	DestroyVectorToggle (dialog, Nix1, Nix) ;
      }
    dialog->icInputsUsed = pvt->inputs->icUsed ;
    }

  if (dialog->icVectorsUsed > pvt->vectors->icUsed)
    {
    for (Nix = pvt->vectors->icUsed ; Nix < dialog->icVectorsUsed ; Nix++)
      {
      DestroyIdxLabel (dialog, Nix) ;
      for (Nix1 = 0 ; Nix1 < dialog->icInputsUsed ; Nix1++)
      	DestroyVectorToggle (dialog, Nix, Nix1) ;
      }
    dialog->icVectorsUsed = pvt->vectors->icUsed ;
    }

  // create new widgets if this time around we have more than last time around.  Reuse existing
  // widgets first.

  for (Nix = 0 ; Nix < dialog->icInputsUsed ; Nix++)
    {
    ReuseInputHeading (dialog, pvt, Nix) ;
    for (Nix1 = 0 ; Nix1 < dialog->icVectorsUsed ; Nix1++)
      ReuseVectorToggle (dialog, exp_array_index_2d (pvt->vectors, gboolean, Nix1, Nix), Nix1, Nix) ;
    }

  for (; Nix < pvt->inputs->icUsed ; Nix++)
    {
    CreateInputHeading (dialog, pvt, Nix) ;
    for (Nix1 = 0 ; Nix1 < dialog->icVectorsUsed ; Nix1++)
      CreateVectorToggle (dialog, exp_array_index_2d (pvt->vectors, gboolean, Nix1, Nix), Nix1, Nix) ;
    }

  dialog->icInputsUsed = pvt->inputs->icUsed ;

  for (Nix = dialog->icVectorsUsed ; Nix < pvt->vectors->icUsed ; Nix++)
    {
    CreateIdxLabel (dialog, Nix) ;
    for (Nix1 = 0 ; Nix1 < dialog->icInputsUsed ; Nix1++)
      CreateVectorToggle (dialog, exp_array_index_2d (pvt->vectors, gboolean, Nix, Nix1), Nix, Nix1) ;
    }

  dialog->icVectorsUsed = pvt->vectors->icUsed ;

  gtk_table_resize (GTK_TABLE (dialog->tblVT), dialog->icInputsUsed + 1, dialog->icVectorsUsed + 1) ;

  // First, make sure /everything/ is visible

  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    {
    SetColumnActive (dialog, Nix, exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pInput[Nix].tb), exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag) ;
    }

  SetCurrentFileName (dialog, pvt->pszFName) ;
  }

static void DialogToVectorTable (vector_table_options_D *dialog, VectorTable *pvt)
  {
  int idx = -1, Nix, Nix1 ;
  gboolean bVal = FALSE ;

  g_free (pvt->pszFName) ;
  pvt->pszFName = g_strdup ((char *)g_object_get_data (G_OBJECT (dialog->dlgVectorTable), "pszCurrentFName")) ;

  VectorTable_empty (pvt) ;

  for (Nix = 0 ; Nix < dialog->icInputsUsed ; Nix++)
    VectorTable_add_input (pvt, QCAD_CELL (g_object_get_data (G_OBJECT (dialog->pInput[Nix].cb), "input"))) ;

  for (Nix = 0 ; Nix < dialog->icVectorsUsed ; Nix++)
    {
    idx = VectorTable_add_vector (pvt, Nix) ;
    for (Nix1 = 0 ; Nix1 < MIN (dialog->icInputsUsed, pvt->inputs->icUsed) ; Nix1++)
      {
      bVal = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ppBit[Nix][Nix1].tb)) ;
      exp_array_index_2d (pvt->vectors, gboolean, idx, Nix1) = bVal ;
//      exp_array_insert_vals (pvt->vectors, &bVal, 1, 2, Nix, Nix1) ;
      }
    }

  for (Nix = 0 ; Nix < MIN (dialog->icInputsUsed, pvt->inputs->icUsed) ; Nix++)
    exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->pInput[Nix].tb)) ;
  }

static void SetColumnActive (vector_table_options_D *dialog, int idx, gboolean bActive)
  {
  int Nix ;

  gtk_widget_set_sensitive (dialog->pInput[idx].cb, bActive) ;

  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bActive", (gpointer)bActive) ;

  for (Nix = 0 ; Nix < dialog->icVectorsUsed ; Nix++)
    {
    gtk_widget_set_sensitive (dialog->ppBit[Nix][idx].tb, bActive) ;
    gtk_widget_set_sensitive (dialog->ppBit[Nix][idx].tblbl, bActive) ;
    }
  }

static void CreateIdxLabel (vector_table_options_D *dialog, int idx)
  {
  char sz[16] = "" ;

  dialog->pIdx[idx].eb = gtk_event_box_new () ;
  gtk_widget_ref (dialog->pIdx[idx].eb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pIdx[idx].eb) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->pIdx[idx].eb, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->pIdx[idx].eb) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pIdx[idx].eb, 0, 1, VTTBL_Y_OFF + idx, VTTBL_Y_OFF + idx + 1,
      (GtkAttachOptions)(GTK_FILL),
      (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  g_object_set_data (G_OBJECT (dialog->pIdx[idx].eb), "idx", (gpointer)idx) ;

  g_snprintf (sz, 16, "%d", idx) ;
  dialog->pIdx[idx].lbl = gtk_label_new (sz) ;
  gtk_widget_ref (dialog->pIdx[idx].lbl) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pIdx[idx].lbl) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->pIdx[idx].lbl,
      	      	      	    (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->pIdx[idx].lbl) ;
  gtk_container_add (GTK_CONTAINER (dialog->pIdx[idx].eb), dialog->pIdx[idx].lbl) ;
  gtk_label_set_justify (GTK_LABEL (dialog->pIdx[idx].lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->pIdx[idx].lbl), 2, 0.5) ;

  DBG_VTO_SIGNAL(fprintf (stderr, "CreateIdxLabel:gtk_signal_connect\n")) ;
  g_signal_connect (G_OBJECT (dialog->pIdx[idx].eb), "button_press_event", (GCallback)Vector_buttondown, dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateIdxLabel:gtk_signal_connect done\n")) ;
  }

static void DestroyIdxLabel (vector_table_options_D *dialog, int idx)
  {
  char sz[16] = "" ;

  g_signal_handler_disconnect (G_OBJECT (dialog->pIdx[idx].eb),
    g_signal_handler_find (G_OBJECT (dialog->pIdx[idx].eb),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)Vector_buttondown, dialog->dlgVectorTable)) ;

  gtk_container_remove (GTK_CONTAINER (dialog->pIdx[idx].eb), dialog->pIdx[idx].lbl) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pIdx[idx].lbl) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;

  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pIdx[idx].eb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pIdx[idx].eb) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;
  }

static void CreateVectorToggle (vector_table_options_D *dialog, gboolean bValue, int idxRow, int idxCol)
  {
  char sz[16] = "" ;
  dialog->ppBit[idxRow][idxCol].tb = gtk_toggle_button_new () ;
  gtk_widget_ref (dialog->ppBit[idxRow][idxCol].tb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->ppBit[idxRow][idxCol].tb) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->ppBit[idxRow][idxCol].tb, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->ppBit[idxRow][idxCol].tb) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->ppBit[idxRow][idxCol].tb,
    VTTBL_X_OFF + idxCol, VTTBL_X_OFF + idxCol + 1, VTTBL_Y_OFF + idxRow, VTTBL_Y_OFF + idxRow + 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppBit[idxRow][idxCol].tb), bValue) ;
  g_object_set_data (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb), "idxRow", (gpointer)idxRow) ;
  g_object_set_data (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb), "idxCol", (gpointer)idxCol) ;
  GTK_WIDGET_UNSET_FLAGS (dialog->ppBit[idxRow][idxCol].tb, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  dialog->ppBit[idxRow][idxCol].tblbl = gtk_label_new (bValue ? "1" : "0") ;
  gtk_widget_ref (dialog->ppBit[idxRow][idxCol].tblbl) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->ppBit[idxRow][idxCol].tblbl) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->ppBit[idxRow][idxCol].tblbl,
      	      	      	    (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->ppBit[idxRow][idxCol].tblbl) ;
  gtk_container_add (GTK_CONTAINER (dialog->ppBit[idxRow][idxCol].tb), dialog->ppBit[idxRow][idxCol].tblbl) ;

  DBG_VTO_SIGNAL(fprintf (stderr, "CreateVectorToggle:gtk_signal_connect\n")) ;
  g_signal_connect (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb), "toggled", (GCallback)(click_bit_button), dialog->ppBit[idxRow][idxCol].tblbl) ;
  g_signal_connect (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb), "button_press_event", (GCallback)(Bit_buttondown), dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateVectorToggle:gtk_signal_connect done\n")) ;
  }

static void ReuseVectorToggle (vector_table_options_D *dialog, gboolean bValue, int idxRow, int idxCol)
  {
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->ppBit[idxRow][idxCol].tb), bValue) ;
  gtk_label_set_text (GTK_LABEL (dialog->ppBit[idxRow][idxCol].tblbl), bValue ? "1" : "0") ;
  }

static void DestroyVectorToggle (vector_table_options_D *dialog, int idxRow, int idxCol)
  {
  char sz[16] = "" ;

  g_signal_handler_disconnect (G_OBJECT(dialog->ppBit[idxRow][idxCol].tb),
    g_signal_handler_find (G_OBJECT(dialog->ppBit[idxRow][idxCol].tb),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)click_bit_button, dialog->ppBit[idxRow][idxCol].tblbl)) ;
  gtk_container_remove (GTK_CONTAINER (dialog->ppBit[idxRow][idxCol].tb), dialog->ppBit[idxRow][idxCol].tblbl) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->ppBit[idxRow][idxCol].tblbl) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;

  g_signal_handler_disconnect (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb),
    g_signal_handler_find (G_OBJECT (dialog->ppBit[idxRow][idxCol].tb),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)Bit_buttondown, dialog->dlgVectorTable)) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->ppBit[idxRow][idxCol].tb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->ppBit[idxRow][idxCol].tb) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;
  }

static void CreateInputHeading (vector_table_options_D *dialog, VectorTable *pvt, int idx)
  {
  GList *pglInputs = NULL ;
  int Nix ;
  char sz[16] = "" ;

  dialog->pInput[idx].tb = gtk_toggle_button_new () ;
  gtk_widget_ref (dialog->pInput[idx].tb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pInput[idx].tb) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->pInput[idx].tb, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->pInput[idx].tb) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idx].tb, VTTBL_X_OFF + idx, VTTBL_X_OFF + idx + 1, VTTBL_Y_OFF - 2, VTTBL_Y_OFF - 1,
      	      	    (GtkAttachOptions)(GTK_FILL),
		    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pInput[idx].tb), exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idx].tb), "idx", (gpointer)idx) ;
  GTK_WIDGET_UNSET_FLAGS (dialog->pInput[idx].tb, (GtkWidgetFlags)(GTK_CAN_FOCUS | GTK_CAN_DEFAULT)) ;

  dialog->pInput[idx].tblbl = gtk_label_new (exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag ? _("Active") : _("Inactive")) ;
  gtk_widget_ref (dialog->pInput[idx].tblbl) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pInput[idx].tblbl) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->pInput[idx].tblbl, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->pInput[idx].tblbl) ;
  gtk_container_add (GTK_CONTAINER (dialog->pInput[idx].tb), dialog->pInput[idx].tblbl) ;

  dialog->pInput[idx].cb = gtk_combo_new () ;
  gtk_widget_ref (dialog->pInput[idx].cb) ;
  g_snprintf (sz, 16, "0x%08X", (int)dialog->pInput[idx].cb) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, dialog->pInput[idx].cb, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (dialog->pInput[idx].cb) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idx].cb, VTTBL_X_OFF + idx, VTTBL_X_OFF + idx + 1, VTTBL_Y_OFF - 1, VTTBL_Y_OFF,
      	      	    (GtkAttachOptions)(GTK_FILL),
		    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_widget_set_usize (dialog->pInput[idx].cb, 90, -2) ;
  gtk_combo_set_value_in_list (GTK_COMBO (dialog->pInput[idx].cb), TRUE, FALSE);
  gtk_combo_set_use_arrows_always (GTK_COMBO (dialog->pInput[idx].cb), TRUE);
  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    pglInputs = g_list_append (pglInputs, (char *)qcad_cell_get_label (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input)) ;
  gtk_combo_set_popdown_strings (GTK_COMBO (dialog->pInput[idx].cb), pglInputs) ;
  g_list_free (pglInputs) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "input", exp_array_index_1d (pvt->inputs, VT_INPUT, idx).input) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore", (gpointer)FALSE) ;

  gtk_widget_ref (GTK_COMBO (dialog->pInput[idx].cb)->entry) ;
  g_snprintf (sz, 16, "0x%08X", (int)GTK_COMBO (dialog->pInput[idx].cb)->entry) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, GTK_COMBO (dialog->pInput[idx].cb)->entry, (GtkDestroyNotify) gtk_widget_unref) ;
  gtk_widget_show (GTK_COMBO (dialog->pInput[idx].cb)->entry) ;
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (dialog->pInput[idx].cb)->entry), FALSE) ;
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dialog->pInput[idx].cb)->entry), qcad_cell_get_label (exp_array_index_1d (pvt->inputs, VT_INPUT, idx).input)) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->entry), "idx", (gpointer)idx) ;

  gtk_widget_ref (GTK_COMBO (dialog->pInput[idx].cb)->popwin) ;
  g_snprintf (sz, 16, "0x%08X", (int)GTK_COMBO (dialog->pInput[idx].cb)->popwin) ;
  g_object_set_data_full (G_OBJECT (dialog->dlgVectorTable), sz, GTK_COMBO (dialog->pInput[idx].cb)->popwin, (GtkDestroyNotify) gtk_widget_unref) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin), "idx", (gpointer)idx) ;

  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(tb)\n")) ;
  g_signal_connect (G_OBJECT (dialog->pInput[idx].tb), "toggled", (GCallback)(ActiveFlag_toggled), dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(tb) done\n")) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(entry)\n")) ;
  g_signal_connect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->entry), "changed", (GCallback)(InputCBEntry_changed), dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(entry) done\n")) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(popwin)\n")) ;
  g_signal_connect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin), "show", (GCallback)(InputCBPopwin_show), dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin), "hide", (GCallback)(InputCBPopwin_hide), dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(popwin) done\n")) ;
  }

static void ReuseInputHeading (vector_table_options_D *dialog, VectorTable *pvt, int idx)
  {
  int Nix ;
  GList *pglInputs = NULL ;

  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore", (gpointer)TRUE) ;
  for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
    pglInputs = g_list_append (pglInputs, (char *)qcad_cell_get_label (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input)) ;

  gtk_combo_set_popdown_strings (GTK_COMBO (dialog->pInput[idx].cb), pglInputs) ;
  g_list_free (pglInputs) ;
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dialog->pInput[idx].cb)->entry), qcad_cell_get_label (exp_array_index_1d (pvt->inputs, VT_INPUT, idx).input)) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore", (gpointer)FALSE) ;
  }

static void DestroyInputHeading (vector_table_options_D *dialog, int idx)
  {
  char sz[16] = "" ;

  g_signal_handler_disconnect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->entry),
    g_signal_handler_find (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->entry),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)InputCBEntry_changed, dialog->dlgVectorTable)) ;

  g_signal_handler_disconnect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin),
    g_signal_handler_find (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)InputCBPopwin_hide, dialog->dlgVectorTable)) ;

  g_signal_handler_disconnect (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin),
    g_signal_handler_find (G_OBJECT (GTK_COMBO (dialog->pInput[idx].cb)->popwin),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)InputCBPopwin_show, dialog->dlgVectorTable)) ;

  g_signal_handler_disconnect (G_OBJECT (dialog->pInput[idx].tb),
    g_signal_handler_find (G_OBJECT (dialog->pInput[idx].tb),
      G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer)ActiveFlag_toggled, dialog->dlgVectorTable)) ;

  g_snprintf (sz, 16, "0x%08X", (int)GTK_COMBO (dialog->pInput[idx].cb)->entry) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;

  g_snprintf (sz, 16, "0x%08X", (int)GTK_COMBO (dialog->pInput[idx].cb)->popwin) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;

  g_snprintf (sz, 16, "0x%08X", (int)dialog->pInput[idx].cb) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idx].cb) ;

  g_snprintf (sz, 16, "0x%08X", (int)dialog->pInput[idx].tb) ;
//  gtk_object_remove_data (GTK_OBJECT (dialog->dlgVectorTable), sz) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idx].tb) ;
  }

static void create_vector_table_options_dialog (vector_table_options_D *dialog)
  {
  GtkWidget *mnuSp = NULL, *spacer = NULL,/* *mnuhbox = NULL, *lbl = NULL, */*img = NULL ;

  dialog->dlgVectorTable = gtk_dialog_new ();
  gtk_widget_set_usize (dialog->dlgVectorTable, 640, 480);
  gtk_window_set_modal (GTK_WINDOW (dialog->dlgVectorTable), TRUE);
  gtk_window_set_title (GTK_WINDOW (dialog->dlgVectorTable), _("Vector Table Setup"));
  gtk_window_set_default_size (GTK_WINDOW (dialog->dlgVectorTable), 640, 480);
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlgVectorTable), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->dlgVectorTable)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->tblVTMain = gtk_table_new (3, 1, FALSE);
  gtk_widget_show (dialog->tblVTMain);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->tblVTMain, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblVTMain), 2);

  dialog->fmVTOps = gtk_frame_new (_("Vector Table Operations"));
  gtk_widget_show (dialog->fmVTOps);
  gtk_table_attach (GTK_TABLE (dialog->tblVTMain), dialog->fmVTOps, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmVTOps), 2);

  dialog->tblOps = gtk_table_new (2, 1, FALSE);
  gtk_widget_show (dialog->tblOps);
  gtk_container_add (GTK_CONTAINER (dialog->fmVTOps), dialog->tblOps);

  dialog->lblMsg = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (dialog->lblMsg),
    _("<big><span foreground=\"red\"><b>Note:</b></span> Right-click the vector table for adding/removing vectors.</big>")) ;
  gtk_widget_show (dialog->lblMsg);
  gtk_table_attach (GTK_TABLE (dialog->tblOps), dialog->lblMsg, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lblMsg), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblMsg), 0, 0.5);

  dialog->hbVTOps = gtk_hbutton_box_new ();
  gtk_widget_show (dialog->hbVTOps);
  gtk_table_attach (GTK_TABLE (dialog->tblOps), dialog->hbVTOps, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->hbVTOps), 2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbVTOps), GTK_BUTTONBOX_START);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbVTOps), 5);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbVTOps), 0, 0);

#ifdef STDIO_FILEIO
  dialog->btnLoad = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_widget_show (dialog->btnLoad);
  gtk_container_add (GTK_CONTAINER (dialog->hbVTOps), dialog->btnLoad);
  GTK_WIDGET_SET_FLAGS (dialog->btnLoad, GTK_CAN_DEFAULT);

  dialog->btnSave = gtk_button_new_from_stock (GTK_STOCK_SAVE);
  gtk_widget_show (dialog->btnSave);
  gtk_container_add (GTK_CONTAINER (dialog->hbVTOps), dialog->btnSave);
  GTK_WIDGET_SET_FLAGS (dialog->btnSave, GTK_CAN_DEFAULT);

  dialog->btnSaveAs = gtk_button_new_from_stock (GTK_STOCK_SAVE_AS);
  gtk_widget_show (dialog->btnSaveAs);
  gtk_container_add (GTK_CONTAINER (dialog->hbVTOps), dialog->btnSaveAs);
  GTK_WIDGET_SET_FLAGS (dialog->btnSaveAs, GTK_CAN_DEFAULT);
#endif /* def STDIO_FILEIO */

  dialog->swndVT = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (dialog->swndVT);
  gtk_table_attach (GTK_TABLE (dialog->tblVTMain), dialog->swndVT, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swndVT), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (dialog->viewport1);
  gtk_container_add (GTK_CONTAINER (dialog->swndVT), dialog->viewport1);

  dialog->ebVT = gtk_event_box_new () ;
  gtk_widget_show (dialog->ebVT) ;
  gtk_container_add (GTK_CONTAINER (dialog->viewport1), dialog->ebVT) ;

  dialog->tblVT = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (dialog->tblVT);
  gtk_container_add (GTK_CONTAINER (dialog->ebVT), dialog->tblVT);

  spacer = gtk_label_new ("") ;
  gtk_widget_show (spacer) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), spacer, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_widget_set_size_request (spacer, 60, 0) ;

  dialog->hbVTFile = gtk_hbox_new (FALSE, 2) ;
  gtk_widget_show (dialog->hbVTFile) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVTMain), dialog->hbVTFile, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);

  dialog->lblVTFileMsg = gtk_label_new (_("File:")) ;
  gtk_widget_show (dialog->lblVTFileMsg) ;
  gtk_box_pack_start (GTK_BOX (dialog->hbVTFile), dialog->lblVTFileMsg, FALSE, TRUE, 0) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblVTFileMsg), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblVTFileMsg), 1.0, 0.5) ;

  dialog->lblVTFile = gtk_label_new ("");
  gtk_widget_show (dialog->lblVTFile);
  gtk_box_pack_start (GTK_BOX (dialog->hbVTFile), dialog->lblVTFile, FALSE, TRUE, 0) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblVTFile), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblVTFile), 0.0, 0.5) ;

  dialog->dialog_action_area1 = GTK_DIALOG (dialog->dlgVectorTable)->action_area;
  gtk_widget_show (dialog->dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 1);

  dialog->mnuVT = gtk_menu_new () ;

  dialog->mnuAdd = gtk_image_menu_item_new_with_label (_("Add Vector")) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (dialog->mnuAdd),
    img = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (dialog->mnuAdd) ;
  gtk_container_add (GTK_CONTAINER (dialog->mnuVT), dialog->mnuAdd) ;

  dialog->mnuInsBefore = gtk_menu_item_new_with_label (_("Insert Before")) ;
  gtk_widget_show (dialog->mnuInsBefore) ;
  gtk_container_add (GTK_CONTAINER (dialog->mnuVT), dialog->mnuInsBefore) ;

  dialog->mnuInsAfter = gtk_menu_item_new_with_label (_("Insert Vector After")) ;
  gtk_widget_show (dialog->mnuInsAfter) ;
  gtk_container_add (GTK_CONTAINER (dialog->mnuVT), dialog->mnuInsAfter) ;


  mnuSp = gtk_menu_item_new () ;
  gtk_widget_show (mnuSp) ;
  gtk_container_add (GTK_CONTAINER (dialog->mnuVT), mnuSp) ;
  dialog->mnuDel = gtk_image_menu_item_new_with_label (_("Remove Vector")) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (dialog->mnuDel),
    img = gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (dialog->mnuDel) ;
  gtk_container_add (GTK_CONTAINER (dialog->mnuVT), dialog->mnuDel) ;

  dialog->btnCancel = gtk_dialog_add_button (GTK_DIALOG (dialog->dlgVectorTable), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  dialog->btnOK = gtk_dialog_add_button (GTK_DIALOG (dialog->dlgVectorTable), GTK_STOCK_OK, GTK_RESPONSE_OK);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgVectorTable), GTK_RESPONSE_OK) ;
#ifdef STDIO_FILEIO
  g_signal_connect (G_OBJECT (dialog->btnLoad),      "clicked",            (GCallback)(load_vector_table), dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (dialog->btnSave),      "clicked",            (GCallback)(save_vector_table), dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (dialog->btnSaveAs),    "clicked",            (GCallback)(save_vector_table), dialog->dlgVectorTable) ;
#endif /* def STDIO_FILEIO */
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(ebVT)\n")) ;
  g_signal_connect (G_OBJECT (dialog->ebVT),         "button_press_event", (GCallback)(VT_buttondown),     dialog->dlgVectorTable) ;
  DBG_VTO_SIGNAL(fprintf (stderr, "CreateInputHeading:gtk_signal_connect(mnuInsBefore)\n")) ;
  g_signal_connect (G_OBJECT (dialog->mnuInsBefore), "activate",           (GCallback)(create_vector),     dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (dialog->mnuInsAfter),  "activate",           (GCallback)(create_vector),     dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (dialog->mnuAdd),       "activate",           (GCallback)(create_vector),     dialog->dlgVectorTable) ;
  g_signal_connect (G_OBJECT (dialog->mnuDel),       "activate",           (GCallback)(delete_vector),     dialog->dlgVectorTable) ;
  }

static gboolean Vector_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;

  if (3 == ev->button) // right-click
    {
    g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "idxVector",
      (gpointer)g_object_get_data (G_OBJECT (widget), "idx")) ;
    gtk_widget_set_sensitive (dialog->mnuInsBefore, TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuInsAfter,  TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuAdd,       FALSE) ;
    gtk_widget_set_sensitive (dialog->mnuDel,       TRUE) ;
    gtk_menu_popup (GTK_MENU (dialog->mnuVT), NULL, NULL, NULL, NULL, ev->button, ev->time) ;

    return TRUE ;
    }
  return FALSE ;
  }

static void InputCBEntry_changed (GtkWidget *widget, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  int idx = (int)g_object_get_data (G_OBJECT (widget), "idx") ;
  gchar *pszText = NULL ;

  if ((gboolean)g_object_get_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore")) return ;

  SwapColumns (dialog, idx, GetInputIdx (dialog, pszText = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1))) ;
  g_free (pszText) ;
  }

static void InputCBPopwin_show (GtkWidget *widget, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  int idx = (int)g_object_get_data (G_OBJECT (widget), "idx") ;

  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore", (gpointer)TRUE) ;
  }

static void InputCBPopwin_hide (GtkWidget *widget, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  int idx = (int)g_object_get_data (G_OBJECT (widget), "idx") ;

  g_object_set_data (G_OBJECT (dialog->pInput[idx].cb), "bIgnore", (gpointer)FALSE) ;
  InputCBEntry_changed (GTK_COMBO (dialog->pInput[idx].cb)->entry, user_data) ;
  }

static gboolean VT_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;

  if (3 == ev->button) // right-click
    {
    g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "idxVector", (gpointer)-1) ;
    g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "idxInput", (gpointer)-1) ;
    gtk_widget_set_sensitive (dialog->mnuInsBefore, FALSE) ;
    gtk_widget_set_sensitive (dialog->mnuInsAfter,  FALSE) ;
    gtk_widget_set_sensitive (dialog->mnuAdd,       TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuDel,       FALSE) ;
    gtk_menu_popup (GTK_MENU (dialog->mnuVT), NULL, NULL, NULL, NULL, ev->button, ev->time) ;

    return TRUE ;
    }
  return FALSE ;
  }

static gboolean Bit_buttondown (GtkWidget *widget, GdkEventButton *ev, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;

  if (3 == ev->button) // right-click
    {
    g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "idxVector",  g_object_get_data (G_OBJECT (widget), "idxRow")) ;
    g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "idxInput",   g_object_get_data (G_OBJECT (widget), "idxCol")) ;

    gtk_widget_set_sensitive (dialog->mnuInsBefore, TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuInsAfter,  TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuAdd,       TRUE) ;
    gtk_widget_set_sensitive (dialog->mnuDel,       TRUE) ;
    gtk_menu_popup (GTK_MENU (dialog->mnuVT), NULL, NULL, NULL, NULL, ev->button, ev->time) ;

    return TRUE ;
    }
  return FALSE ;
  }

static void click_bit_button (GtkWidget *widget, gpointer user_data)
  {gtk_label_set_text (GTK_LABEL (user_data), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? "1" : "0") ;}

static void create_vector (GtkWidget *widget, gpointer user_data)
  {
  int idx = (int)g_object_get_data (G_OBJECT (user_data), "idxVector") ;
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;

  if (-1 == idx) idx = dialog->icVectorsUsed ;
  if (dialog->mnuInsAfter == widget) idx++ ;
  idx = CLAMP (idx, 0, dialog->icVectorsUsed) ;

  CreateVector (dialog, idx) ;
  }

static void delete_vector (GtkWidget *widget, gpointer user_data)
  {
  int idx = (int)g_object_get_data (G_OBJECT (user_data), "idxVector") ;
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;

  if (-1 != idx)
    DeleteVector (dialog, idx) ;
  }

#ifdef STDIO_FILEIO
static void load_vector_table (GtkWidget *widget, gpointer user_data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  VectorTable *pvtDlg = (VectorTable *)g_object_get_data (G_OBJECT (user_data), "pvtDlg") ;
  VTL_RESULT vtlr = VTL_OK ;
  char *pszFName = NULL ;

  if (NULL == (pszFName = get_file_name_from_user (GTK_WINDOW (dialog->dlgVectorTable), "Load Vector Table", pszFName, FALSE)))
    return ;

  g_free (pvtDlg->pszFName) ;
  pvtDlg->pszFName = g_strdup (pszFName) ;

  if (VTL_OK != (vtlr = VectorTable_load (pvtDlg)))
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (dialog->dlgVectorTable), GTK_DIALOG_MODAL,
      (VTL_FILE_FAILED == vtlr || VTL_MAGIC_FAILED == vtlr) ? GTK_MESSAGE_ERROR : GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
       VTL_FILE_FAILED == vtlr ? "Failed to open file \"%s\"." :
      VTL_MAGIC_FAILED == vtlr ? "File \"%s\" does not appear to be a vector table file (Invalid magic)." :
             VTL_SHORT == vtlr ? "File \"%s\" contained fewer inputs than the current design.  The vectors were padded with 0s." :
      	      	      		 "File \"%s\" contained more inputs than the current design.  The vectors were truncated.",
      pvtDlg->pszFName))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    }

  DBG_NVTO (VectorTable_dump (pvtDlg, stderr)) ;

  if (VTL_OK == vtlr || VTL_SHORT == vtlr || VTL_TRUNC == vtlr)
    VectorTableToDialog (dialog, pvtDlg) ;

  while (pvtDlg->vectors->icUsed > 0)
    VectorTable_del_vector (pvtDlg, pvtDlg->vectors->icUsed - 1) ;
  }

static void save_vector_table (GtkWidget *widget, gpointer user_data)
  {
  char *pszFName = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  VectorTable *pvtDlg = (VectorTable *)g_object_get_data (G_OBJECT (user_data), "pvtDlg") ;

  DialogToVectorTable (dialog, pvtDlg) ;

  if (dialog->btnSaveAs == widget)
    {
    g_free (pvtDlg->pszFName) ;
    pvtDlg->pszFName = NULL ;
    }

  if (NULL == pvtDlg->pszFName)
    if (NULL == (pvtDlg->pszFName = get_file_name_from_user (GTK_WINDOW (dialog->dlgVectorTable), "Save Vector Table As", pszFName, TRUE)))
      return ;

  if (!VectorTable_save (pvtDlg))
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (dialog->dlgVectorTable), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
      GTK_BUTTONS_CLOSE, "Failed to save vector table to file \"%s\".", pvtDlg->pszFName))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    }
  else
    SetCurrentFileName (dialog, pvtDlg->pszFName) ;
  }
#endif /* def STDIO_FILEIO */

static void ActiveFlag_toggled (GtkWidget *widget, gpointer user_data)
  {
  gboolean bActive = FALSE ;
  vector_table_options_D *dialog = (vector_table_options_D *)g_object_get_data (G_OBJECT (user_data), "dialog") ;
  int idx = (int)g_object_get_data (G_OBJECT (widget), "idx") ;

  if (CountActiveInputs (dialog) < 1)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE) ;
    gtk_label_set_text (GTK_LABEL (dialog->pInput[idx].tblbl), _("Active")) ;
    }
  else
    {
    SetColumnActive (dialog, idx, bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) ;
    gtk_label_set_text (GTK_LABEL (dialog->pInput[idx].tblbl), bActive ? _("Active") : _("Inactive")) ;
    }
  }

static int GetInputIdx (vector_table_options_D *dialog, gchar *pszText)
  {
  int Nix ;

  for (Nix = 0 ; Nix < dialog->icInputsUsed ; Nix++)
    if (!strcmp (qcad_cell_get_label ((QCADCell *)g_object_get_data (G_OBJECT (dialog->pInput[Nix].cb), "input")), pszText))
      return Nix ;

  return -1 ;
  }

static void SwapColumns (vector_table_options_D *dialog, int idxSrc, int idxDst)
  {
  INPUT_HEADING ihTmp = {NULL} ;
  VECTOR_BIT vbTmp = {NULL} ;
  int Nix ;

  g_object_set_data (G_OBJECT (dialog->pInput[idxSrc].cb), "bIgnore", (gpointer)TRUE) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idxDst].cb), "bIgnore", (gpointer)TRUE) ;

  if (idxSrc == idxDst ||
      idxSrc < 0 || idxSrc >= dialog->icInputsUsed ||
      idxDst < 0 || idxDst >= dialog->icInputsUsed) return ;

  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idxSrc].cb) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idxSrc].tb) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idxDst].cb) ;
  gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pInput[idxDst].tb) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idxSrc].cb,
    VTTBL_X_OFF + idxDst, VTTBL_X_OFF + idxDst + 1, VTTBL_Y_OFF - 1, VTTBL_Y_OFF,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idxSrc].tb,
    VTTBL_X_OFF + idxDst, VTTBL_X_OFF + idxDst + 1, VTTBL_Y_OFF - 2, VTTBL_Y_OFF - 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idxDst].cb,
    VTTBL_X_OFF + idxSrc, VTTBL_X_OFF + idxSrc + 1, VTTBL_Y_OFF - 1, VTTBL_Y_OFF,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
  gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pInput[idxDst].tb,
    VTTBL_X_OFF + idxSrc, VTTBL_X_OFF + idxSrc + 1, VTTBL_Y_OFF - 2, VTTBL_Y_OFF - 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;

  memcpy (&ihTmp, &(dialog->pInput[idxSrc]), sizeof (INPUT_HEADING)) ;
  memcpy (&(dialog->pInput[idxSrc]), &(dialog->pInput[idxDst]), sizeof (INPUT_HEADING)) ;
  memcpy (&(dialog->pInput[idxDst]), &ihTmp, sizeof (INPUT_HEADING)) ;

  g_object_set_data (G_OBJECT (dialog->pInput[idxSrc].tb), "idx", (gpointer)idxSrc) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idxSrc].cb)->entry), "idx", (gpointer)idxSrc) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idxSrc].cb)->popwin), "idx", (gpointer)idxSrc) ;
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dialog->pInput[idxSrc].cb)->entry), qcad_cell_get_label ((QCADCell *)g_object_get_data (G_OBJECT (dialog->pInput[idxSrc].cb), "input"))) ;

  g_object_set_data (G_OBJECT (dialog->pInput[idxDst].tb), "idx", (gpointer)idxDst) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idxDst].cb)->entry), "idx", (gpointer)idxDst) ;
  g_object_set_data (G_OBJECT (GTK_COMBO (dialog->pInput[idxDst].cb)->popwin), "idx", (gpointer)idxDst) ;
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dialog->pInput[idxDst].cb)->entry), qcad_cell_get_label ((QCADCell *)g_object_get_data (G_OBJECT (dialog->pInput[idxDst].cb), "input"))) ;

  for (Nix = 0 ; Nix < dialog->icVectorsUsed ; Nix++)
    {
    gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->ppBit[Nix][idxSrc].tb) ;
    gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->ppBit[Nix][idxDst].tb) ;
    gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->ppBit[Nix][idxSrc].tb,
      VTTBL_X_OFF + idxDst, VTTBL_X_OFF + idxDst + 1, VTTBL_Y_OFF + Nix, VTTBL_Y_OFF + Nix + 1,
      (GtkAttachOptions)(GTK_FILL),
      (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
    gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->ppBit[Nix][idxDst].tb,
      VTTBL_X_OFF + idxSrc, VTTBL_X_OFF + idxSrc + 1, VTTBL_Y_OFF + Nix, VTTBL_Y_OFF + Nix + 1,
      (GtkAttachOptions)(GTK_FILL),
      (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;

    memcpy (&vbTmp, &(dialog->ppBit[Nix][idxSrc]), sizeof (VECTOR_BIT)) ;
    memcpy (&(dialog->ppBit[Nix][idxSrc]), &(dialog->ppBit[Nix][idxDst]), sizeof (VECTOR_BIT)) ;
    memcpy (&(dialog->ppBit[Nix][idxDst]), &vbTmp, sizeof (VECTOR_BIT)) ;

    g_object_set_data (G_OBJECT (dialog->ppBit[Nix][idxSrc].tb), "idxCol", (gpointer)idxSrc) ;
    g_object_set_data (G_OBJECT (dialog->ppBit[Nix][idxDst].tb), "idxCol", (gpointer)idxDst) ;
    }
  g_object_set_data (G_OBJECT (dialog->pInput[idxSrc].cb), "bIgnore", (gpointer)FALSE) ;
  g_object_set_data (G_OBJECT (dialog->pInput[idxDst].cb), "bIgnore", (gpointer)FALSE) ;
  }

static void DeleteVector (vector_table_options_D *dialog, int idx)
  {
  char sz[16] = "" ;
  int Nix, Nix1 ;

  DestroyIdxLabel (dialog, idx) ;
  for (Nix = 0 ; Nix < dialog->icInputsUsed ; Nix++)
    DestroyVectorToggle (dialog, idx, Nix) ;

  for (Nix = idx + 1 ; Nix < dialog->icVectorsUsed ; Nix++)
    {
    g_snprintf (sz, 16, "%d", Nix - 1) ;

    gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pIdx[Nix].eb) ;
    gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pIdx[Nix].eb,
      0, 1, VTTBL_Y_OFF + Nix - 1, VTTBL_Y_OFF + Nix, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
    gtk_label_set_text (GTK_LABEL (dialog->pIdx[Nix].lbl), sz) ;
    g_object_set_data (G_OBJECT (dialog->pIdx[Nix].eb), "idx", (gpointer)(Nix - 1)) ;
    for (Nix1 = 0 ; Nix1 < dialog->icInputsUsed ; Nix1++)
      {
      gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->ppBit[Nix][Nix1].tb) ;
      gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->ppBit[Nix][Nix1].tb,
        VTTBL_X_OFF + Nix1, VTTBL_X_OFF + Nix1 + 1, VTTBL_Y_OFF + Nix - 1, VTTBL_Y_OFF + Nix, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
      g_object_set_data (G_OBJECT (dialog->ppBit[Nix][Nix1].tb), "idxRow", (gpointer)(Nix - 1)) ;
      }
    }

  if (idx < dialog->icVectorsUsed - 1)
    {
    VECTOR_BIT *p = NULL ;
    memmove (&(dialog->pIdx[idx]), &(dialog->pIdx[idx + 1]), (dialog->icVectorsUsed - idx) * sizeof (VECTOR_IDX)) ;

    p = dialog->ppBit[idx] ;
    memmove (&(dialog->ppBit[idx]), &(dialog->ppBit[idx + 1]), (dialog->icVectorsUsed - idx) * sizeof (VECTOR_BIT *)) ;
    dialog->ppBit[dialog->icVectorsUsed - 1] = p ;
    }
  dialog->icVectorsUsed-- ;

  gtk_table_resize (GTK_TABLE (dialog->tblVT), dialog->icInputsUsed + 1, dialog->icVectorsUsed + 1) ;
  }

static void CreateVector (vector_table_options_D *dialog, int idx)
  {
  char sz[16] = "" ;
  int Nix, Nix1 ;
  gboolean bActive = TRUE ;

  if (dialog->icVectorsUsed == dialog->icVectors)
    {
    dialog->icVectors = dialog->icVectors * 2 + 1 ;
    dialog->pIdx = realloc (dialog->pIdx, dialog->icVectors * sizeof (VECTOR_IDX)) ;
    dialog->ppBit = realloc (dialog->ppBit, dialog->icVectors * sizeof (VECTOR_BIT *)) ;
    for (Nix = dialog->icVectorsUsed ; Nix < dialog->icVectors ; Nix++)
      dialog->ppBit[Nix] = g_malloc0 (dialog->icInputs * sizeof (VECTOR_BIT)) ;
    }

  if (idx < dialog->icVectorsUsed)
    {
    VECTOR_BIT *p = NULL ;
    for (Nix = dialog->icVectorsUsed - 1 ; Nix > idx - 1 ; Nix--)
      {
      gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->pIdx[Nix].eb) ;
      gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->pIdx[Nix].eb, 0, 1, VTTBL_Y_OFF + Nix + 1, VTTBL_Y_OFF + Nix + 2,
      	(GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
      g_snprintf (sz, 16, "%d", Nix + 1) ;
      gtk_label_set_text (GTK_LABEL (dialog->pIdx[Nix].lbl), sz) ;
      g_object_set_data (G_OBJECT (dialog->pIdx[Nix].eb), "idx", (gpointer)(Nix + 1)) ;
      for (Nix1 = 0 ; Nix1 < dialog->icInputsUsed ; Nix1++)
        {
        gtk_container_remove (GTK_CONTAINER (dialog->tblVT), dialog->ppBit[Nix][Nix1].tb) ;
        gtk_table_attach (GTK_TABLE (dialog->tblVT), dialog->ppBit[Nix][Nix1].tb,
	        VTTBL_X_OFF + Nix1, VTTBL_X_OFF + Nix1 + 1, VTTBL_Y_OFF + Nix + 1, VTTBL_Y_OFF + Nix + 2, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), VTTBL_H_PAD, VTTBL_V_PAD) ;
            g_object_set_data (G_OBJECT (dialog->ppBit[Nix][Nix1].tb), "idxRow", (gpointer)(Nix + 1)) ;
        }
      }
    memmove (&(dialog->pIdx[idx + 1]), &(dialog->pIdx[idx]), (dialog->icVectorsUsed - idx) * sizeof (VECTOR_IDX)) ;

    p = dialog->ppBit[dialog->icVectorsUsed] ;
    memmove (&(dialog->ppBit[idx + 1]), &(dialog->ppBit[idx]), (dialog->icVectorsUsed - idx) * sizeof (VECTOR_BIT *)) ;
    dialog->ppBit[idx] = p ;
    }
  dialog->icVectorsUsed++ ;

  CreateIdxLabel (dialog, idx) ;
  for (Nix = 0 ; Nix < dialog->icInputsUsed ; Nix++)
    {
    CreateVectorToggle (dialog, FALSE, idx, Nix) ;
    gtk_widget_set_sensitive (dialog->ppBit[idx][Nix].tb, bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->pInput[Nix].tb))) ;
    gtk_widget_set_sensitive (dialog->ppBit[idx][Nix].tblbl, bActive) ;
    }

  gtk_table_resize (GTK_TABLE (dialog->tblVT), dialog->icInputsUsed + 1, dialog->icVectorsUsed + 1) ;
  }

static void SetCurrentFileName (vector_table_options_D *dialog, char *pszFName)
  {
  char *pszCurrentFName = (char *)g_object_get_data (G_OBJECT (dialog->dlgVectorTable), "pszCurrentFName") ;

  if (NULL != pszCurrentFName) g_free (pszCurrentFName) ;

  gtk_label_set_text (GTK_LABEL (dialog->lblVTFile), pszFName) ;
  g_object_set_data (G_OBJECT (dialog->dlgVectorTable), "pszCurrentFName", g_strdup (pszFName)) ;
  }

static int CountActiveInputs (vector_table_options_D *dialog)
  {
  int iRet = 0, Nix ;

  for (Nix = 0 ; Nix < dialog->icInputs ; Nix++)
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->pInput[Nix].tb)))
      iRet++ ;

  return iRet ;
  }
