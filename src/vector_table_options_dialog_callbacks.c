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
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "vector_table.h"
#include "fileio_helpers.h"
#include "file_selection_window.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog_data.h"
#include "vector_table_options_dialog_interface.h"
#include "vector_table_options_dialog_callbacks.h"

void vt_model_active_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data)
  {
  VectorTable *pvt = g_object_get_data (G_OBJECT (cr), "pvt") ;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data)) ;
  GtkTreeIter itr, itrOther ;
  GtkTreePath *tp = NULL ;
  gboolean bActive = TRUE ;
  QCADCell *cell = NULL ;
  int idx ;

  if (NULL == model || NULL == pvt || NULL == (tp = gtk_tree_path_new_from_string (pszTreePath))) return ;
  gtk_tree_model_get_iter (model, &itr, tp) ;

  gtk_tree_model_get (model, &itr, 
    BUS_LAYOUT_MODEL_COLUMN_CELL, &cell,
    VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bActive, -1) ;

  // Zee beeg fleep
  bActive = !bActive ;

  // Update the vector table input flag
  if (NULL != cell)
    if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
      exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag = bActive ;
  gtk_tree_store_set (GTK_TREE_STORE (model), &itr, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bActive, -1) ;

  // If this node hash children (IOW it's a bus node) make all the children be toggled the same way
  // as the bus node, and reflect the state in the vector table
  if (gtk_tree_model_iter_children (model, &itrOther, &itr))
    while (TRUE)
      {
      gtk_tree_model_get (model, &itrOther, BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;

      // Update the vector table input flag
      if (NULL != cell)
        if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
          exp_array_index_1d (pvt->inputs, VT_INPUT, idx).active_flag = bActive ;
      gtk_tree_store_set (GTK_TREE_STORE (model), &itrOther, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bActive, -1) ;
      if (!gtk_tree_model_iter_next (model, &itrOther)) break ;
      }
  else
  if (gtk_tree_model_iter_parent (model, &itrOther, &itr))
    {
    gboolean bBusActive = FALSE ;
    gboolean bChildActive = FALSE ;
    GtkTreeIter itrChild ;

    // If all of a bus' children are deselected, so is the bus
    if (gtk_tree_model_iter_children (model, &itrChild, &itrOther))
      {
      while (TRUE)
        {
        gtk_tree_model_get (model, &itrChild, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, &bChildActive, -1) ;
        if ((bBusActive = bChildActive)) break ;
        if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
        }
      gtk_tree_store_set (GTK_TREE_STORE (model), &itrOther, VECTOR_TABLE_MODEL_COLUMN_ACTIVE, bBusActive, -1) ;
      }
    }

  gtk_tree_path_free (tp) ;
  }

void vector_table_options_dialog_btnClose_clicked (GtkWidget *widget, gpointer data)
  {
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (widget), "dialog") ;

  if (NULL == dialog) return ;

  if (NULL != dialog)
    gtk_widget_hide (dialog->dialog) ;
  }

void vector_table_options_dialog_btnSimType_clicked (GtkWidget *widget, gpointer data) 
  {
  char *psz = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  int sim_type = (int)g_object_get_data (G_OBJECT (widget), "sim_type") ;
  int *user_sim_type = NULL ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;
  if (NULL == dialog) return ;

  if (NULL != (user_sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type")))
    (*user_sim_type) = sim_type ;

  if (VECTOR_TABLE == sim_type)
    {
    VectorTable *pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt") ;

    gtk_widget_set_sensitive (dialog->btnOpen, TRUE) ;
    gtk_widget_set_sensitive (dialog->btnSave, TRUE) ;
    gtk_widget_show (dialog->tblVT) ;
    scrolled_window_set_size (dialog->sw, dialog->tv, 0.8, 0.8) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;
    if (NULL != pvt->pszFName)
      {
      char *psz1 = NULL ;

      psz = base_name (pvt->pszFName) ;
      gtk_window_set_title (GTK_WINDOW (dialog->dialog), 
        psz1 = g_strdup_printf ("%s - %s", psz, _("Vector Table Setup"))) ;
      g_free (psz1) ;
      }
    else
      gtk_window_set_title (GTK_WINDOW (dialog->dialog), 
        psz = g_strdup_printf ("%s - %s", _("Untitled"), _("Vector Table Setup"))) ;
    }
  else
    {
    gtk_window_set_title (GTK_WINDOW (dialog->dialog), 
      psz = g_strdup_printf ("%s - %s", _("Exhaustive Verification"), _("Vector Table Setup"))) ;
    gtk_widget_set_sensitive (dialog->btnOpen, FALSE) ;
    gtk_widget_set_sensitive (dialog->btnSave, FALSE) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE) ;
    gtk_widget_hide (dialog->tblVT) ;
    }
  g_free (psz) ;
  }

void vector_table_options_dialog_btnOpen_clicked (GtkWidget *widget, gpointer data)
  {
  VTL_RESULT vtl_result = VTL_FILE_FAILED ;
  char *pszOldFName = NULL, *psz = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  BUS_LAYOUT *bus_layout = NULL ;
  int *sim_type = NULL ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;
  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;
  if (NULL == (psz = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Open Vector Table"), pvt->pszFName, FALSE))) return ;

  pszOldFName = pvt->pszFName ;
  pvt->pszFName = psz ;

  vtl_result = VectorTable_load (pvt) ;

  if (VTL_FILE_FAILED == vtl_result || VTL_MAGIC_FAILED == vtl_result)
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = 
      gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
          (VTL_FILE_FAILED == vtl_result 
            ? _("Failed to open vector table file \"%s\"!")
            : _("File \"%s\" does not appear to be a vector table file !")), psz))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    g_free (psz) ;
    pvt->pszFName = pszOldFName ;

    return ;
    }
  else
    {
    char *psz1 = NULL ;
    if (VTL_SHORT == vtl_result || VTL_TRUNC == vtl_result)
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = 
        gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
          GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, 
            (VTL_SHORT == vtl_result 
              ? _("File \"%s\" contains fewer inputs than the current vector table. Padding with zeroes.")
              : _("File \"%s\" contains more inputs than the current design. Truncating.")), psz))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }

    if (NULL != (bus_layout = g_object_get_data (G_OBJECT (dialog->dialog), "user_bus_layout")) &&
        NULL != (sim_type = g_object_get_data (G_OBJECT (dialog->dialog), "user_sim_type")))
      VectorTableToDialog (dialog, bus_layout, sim_type, pvt) ;

    g_free (pszOldFName) ;

    gtk_window_set_title (GTK_WINDOW (dialog->dialog), psz = g_strdup_printf ("%s - %s", psz1 = base_name (psz), _("Vector Table Setup"))) ;
    g_free (psz) ;
    g_free (psz1) ;
    }
  }

void vector_table_options_dialog_btnSave_clicked (GtkWidget *widget, gpointer data)
  {
  char *psz = NULL, *psz1 = NULL ;
  VectorTable *pvt = g_object_get_data (G_OBJECT (data), "user_pvt") ;
  int sim_type = (int)g_object_get_data (G_OBJECT (data), "user_sim_type") ;

  if (EXHAUSTIVE_VERIFICATION == sim_type) return ;

  if (NULL == (psz = get_file_name_from_user (GTK_WINDOW (data), _("Save Vector Table"), pvt->pszFName, TRUE)))
    return ;
  else

  psz1 = pvt->pszFName ;
  pvt->pszFName = psz ;

  if (VectorTable_save (pvt))
    {
    g_free (psz1) ;
    // Overwriting psz here is OK, because pvt->pszFName keeps a reference.
    psz = base_name (psz) ;

    gtk_window_set_title (GTK_WINDOW (data), psz1 = g_strdup_printf ("%s - %s", psz, _("Vector Table Setup"))) ;

    g_free (psz1) ;
    g_free (psz) ;
    }
  else
    {
    pvt->pszFName = psz1 ;
    g_free (psz) ;
    }
  }

void vector_table_options_dialog_btnAdd_clicked (GtkWidget *widget, gpointer data)
  {
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (data), "dialog") ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;

  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;


  VectorTable_add_vector (pvt, -1) ;

  add_vector_to_dialog (dialog, pvt, -1) ;
  }

void vector_column_clicked (GtkObject *obj, gpointer data)
  {
  GList *llCols = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  GtkTreeViewColumn *col = NULL ;

  if (NULL == dialog) return ;

  col = GTK_TREE_VIEW_COLUMN (GTK_IS_CELL_RENDERER (obj) ? g_object_get_data (G_OBJECT (obj), "col") : obj) ;

  if (NULL != (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv))))
    {
    GList *llItr = NULL ;
    GtkCellRenderer *cr = NULL ;

    for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
      if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), "cr")))
        g_object_set (G_OBJECT (cr), "cell-background-set", (llItr->data == col), NULL) ;

    g_list_free (llCols) ;

    gtk_widget_queue_draw (dialog->tv) ;
    }
  }

void vector_data_func (GtkTreeViewColumn *col, GtkCellRenderer *cr, GtkTreeModel *model, GtkTreeIter *itr, gpointer data)
  {
  GtkTreeIter itrChild ;
  long long bus_value = 0 ;
  int idxInput = -1 ;
  int idxVector = (int)g_object_get_data (G_OBJECT (cr), "idxVector") ;
  QCADCell *cell = NULL ;
  VectorTable *pvt = (VectorTable *)data ;

  gtk_tree_model_get (model, itr, 
    BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;

  if (NULL != cell)
    if (-1 != (idxInput = VectorTable_find_input_idx (pvt, cell)))
      {
      g_object_set (cr, "active", exp_array_index_2d (pvt->vectors, gboolean, idxVector, idxInput), NULL) ;
      return ;
      }

  if (gtk_tree_model_iter_children (model, &itrChild, itr))
    {
    char *psz = NULL ;
    while (TRUE)
      {
      bus_value = bus_value << 1 ;
      gtk_tree_model_get (model, &itrChild, 
        BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, 
        BUS_LAYOUT_MODEL_COLUMN_NAME, &psz, 
        -1) ;
      if (-1 != (idxInput = VectorTable_find_input_idx (pvt, cell)))
        bus_value += exp_array_index_2d (pvt->vectors, gboolean, idxVector, idxInput) ? 1 : 0 ;
      if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
      }
    g_object_set (cr, "text", psz = g_strdup_printf ("%llu", bus_value), NULL) ;
    g_free (psz) ;
    return ;
    }
  g_object_set (cr, "text", "", NULL) ;
  }

void tree_view_style_set (GtkWidget *widget, GtkStyle *old_style, gpointer data)
  {
  gboolean bUseBackground = FALSE ;
  GdkColor *clrNew = &((gtk_widget_get_style (widget))->base[3]) ;
  GList *llCols = NULL, *llItr = NULL ;
  GtkCellRenderer *cr = NULL ;

  if (NULL == (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (widget)))) return ;

  for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), "cr")))
      {
      // Why does cell-background-gdk modify cell-background-set ?
      g_object_get (G_OBJECT (cr), "cell-background-set", &bUseBackground, NULL) ;
      g_object_set (G_OBJECT (cr), 
        "cell-background-gdk", clrNew, 
        "cell-background-set", bUseBackground, NULL) ;
      }
  }

void vector_value_edited (GtkCellRendererText *cr, char *pszPath, char *pszNewText, gpointer data)
  {
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data)) ;
  VectorTable *pvt = g_object_get_data (G_OBJECT (cr), "pvt") ;
  int idxVector = (int)g_object_get_data (G_OBJECT (cr), "idxVector") ;
  GtkTreePath *tp = NULL ;
  GtkTreeIter itr, itrChild ;
  int idx = -1, the_shift = 0 ;
  long long new_value = -1 ;
  QCADCell *cell = NULL ;
  GtkTreeViewColumn *col = g_object_get_data (G_OBJECT (cr), "col") ;

  if (NULL != col)
    gtk_tree_view_column_clicked (col) ;

  if (NULL == (tp = gtk_tree_path_new_from_string (pszPath))) return ;
  gtk_tree_model_get_iter (model, &itr, tp) ;

  new_value = (long long)atoi (pszNewText) ;

  if (gtk_tree_model_iter_children (model, &itrChild, &itr))
    {
    long long max_val = (1 << (the_shift = gtk_tree_model_iter_n_children (model, &itr))) - 1 ;
    new_value = CLAMP (new_value, 0, max_val) ;
    while (TRUE)
      {
      gtk_tree_model_get (model, &itrChild, BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;
      if (NULL != cell)
        if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
          exp_array_index_2d (pvt->vectors, gboolean, idxVector, idx) = (new_value >> (--the_shift)) & 0x1 ;
      if (!gtk_tree_model_iter_next (model, &itrChild)) break ;
      }
    }
  else
    {
    if (new_value == 0 || new_value == 1)
      if (!(NULL == model || NULL == pvt))
        {
        gtk_tree_model_get (model, &itr, BUS_LAYOUT_MODEL_COLUMN_CELL, &cell, -1) ;
        if (NULL != cell)
          if (-1 != (idx = VectorTable_find_input_idx (pvt, cell)))
            {
            tree_model_row_changed (model, &itr) ;
            // Zee ozer beeg fleep (1 == value ) would be noop
            exp_array_index_2d (pvt->vectors, gboolean, idxVector, idx) = (0 == new_value) ;
            }
        }
    }

  gtk_tree_path_free (tp) ;  
  }
