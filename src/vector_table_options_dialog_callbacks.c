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
#include "global_consts.h"
#include "custom_widgets.h"
#include "vector_table.h"
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
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  int sim_type = (int)g_object_get_data (G_OBJECT (widget), "sim_type") ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  if (VECTOR_TABLE == sim_type)
    {
    gtk_widget_set_sensitive (dialog->btnOpen, TRUE) ;
    gtk_widget_set_sensitive (dialog->btnSave, TRUE) ;
    gtk_widget_show (dialog->tblVT) ;
    scrolled_window_set_size (dialog->sw, dialog->tv, 0.8, 0.8) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;
    }
  else
    {
    gtk_widget_set_sensitive (dialog->btnOpen, FALSE) ;
    gtk_widget_set_sensitive (dialog->btnSave, FALSE) ;
    gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE) ;
    gtk_widget_hide (dialog->tblVT) ;
    }
  }

void vector_table_options_dialog_btnOpen_clicked (GtkWidget *widget, gpointer data) {}
void vector_table_options_dialog_btnSave_clicked (GtkWidget *widget, gpointer data) {}

void vector_table_options_dialog_btnAdd_clicked (GtkWidget *widget, gpointer data)
  {
  vector_table_options_D *dialog = g_object_get_data (G_OBJECT (data), "dialog") ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;

  if (NULL == (pvt = g_object_get_data (G_OBJECT (dialog->dialog), "user_pvt"))) return ;


  VectorTable_add_vector (pvt, -1) ;

  add_vector_to_dialog (dialog, pvt, -1) ;
  }

void vector_column_clicked (GtkTreeViewColumn *col, gpointer data)
  {
  GList *llCols = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;

  if (NULL == dialog) return ;

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
            // Zee ozer beeg fleep (1 == value ) would be noop
            exp_array_index_2d (pvt->vectors, gboolean, idxVector, idx) = (0 == new_value) ;
        }
    }

  gtk_tree_path_free (tp) ;  
  }
