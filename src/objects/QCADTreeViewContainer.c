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
// A treeviewcontainer that allows "freeze columns".    //
// That is, the horizontal scrolling does not scroll    //
// the entire tree view but, instead, it hides and      //
// shows columns as appropriate, keeping the first n    //
// columns always visible.                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "QCADTreeViewContainer.h"

static void qcad_tree_view_container_instance_init (QCADTreeViewContainer *tvc) ;
static void qcad_tree_view_container_class_init (QCADTreeViewContainerClass *klass) ;
static void qcad_tree_view_container_add (GtkContainer *container, GtkWidget *child) ;

static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void tree_view_size_allocate (GtkTreeView *tv, GtkAllocation *alloc, gpointer data) ;

static int tree_view_count_columns (GtkTreeView *tv) ;
static int tree_view_column_get_width (GtkTreeView *tv, GtkTreeViewColumn *col, gboolean bLastCol) ;
static void set_hscroll_upper (QCADTreeViewContainer *tvc) ;

GType qcad_tree_view_container_get_type ()
  {
  static GType tvc_type = 0 ;

  if (0 == tvc_type)
    {
    static GTypeInfo tvc_info = 
      {
      sizeof (QCADTreeViewContainerClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_tree_view_container_class_init,
      NULL,
      NULL,
      sizeof (QCADTreeViewContainer),
      0,
      (GInstanceInitFunc)qcad_tree_view_container_instance_init
      } ;

    if ((tvc_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW, QCAD_TYPE_STRING_TREE_VIEW_CONTAINER, &tvc_info, 0)))
      g_type_class_ref (tvc_type) ;
    }
  return tvc_type ;
  }

static void qcad_tree_view_container_class_init (QCADTreeViewContainerClass *klass)
  {GTK_CONTAINER_CLASS (klass)->add = qcad_tree_view_container_add ;}

static void qcad_tree_view_container_instance_init (QCADTreeViewContainer *tvc)
  {tvc->n_frozen_columns = 0 ;}

static void qcad_tree_view_container_add (GtkContainer *container, GtkWidget *child)
  {
  GtkWidget *old_child = NULL ;
  if (!GTK_IS_TREE_VIEW (child)) return ;

  if (NULL != (old_child = GTK_BIN (container)->child))
    g_signal_handlers_disconnect_matched (G_OBJECT (old_child), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, tree_view_size_allocate, container) ;

  GTK_BIN (container)->child = child ;
  gtk_widget_set_parent (child, GTK_WIDGET (container)) ;
  gtk_tree_view_set_vadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->vscrollbar))) ;
  gtk_tree_view_set_hadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->hscrollbar))) ;

  g_signal_connect (G_OBJECT (child), "size-allocate", (GCallback)tree_view_size_allocate, container) ;
  }

GtkWidget *qcad_tree_view_container_new ()
  {
  GtkWidget *ret = g_object_new (QCAD_TYPE_TREE_VIEW_CONTAINER, "hadjustment", NULL, "vadjustment", NULL, NULL) ;

  g_signal_connect (G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (ret)->hscrollbar))),
    "value-changed", (GCallback)hscroll_adj_value_changed, ret) ;
  return ret ;
  }

void qcad_tree_view_container_freeze_columns (QCADTreeViewContainer *tvc, int n_columns)
  {
  int n_tv_cols = 0 ;
  GtkTreeView *tv = NULL ;
  GtkAdjustment *adjHScroll = NULL ;

  if (!QCAD_IS_TREE_VIEW_CONTAINER (tvc)) return ;
  if (NULL == (tv = GTK_TREE_VIEW (GTK_BIN (tvc)->child))) return ;

  n_columns = 
  QCAD_TREE_VIEW_CONTAINER (tvc)->n_frozen_columns = 
    CLAMP (n_columns, 0, n_tv_cols = tree_view_count_columns (tv)) ;

  // Set the tree view hadjustment if we're going back to normal scrolling
  adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar)) ;
  if (0 == n_columns && (gtk_tree_view_get_hadjustment (tv) != adjHScroll))
    gtk_tree_view_set_hadjustment (tv, adjHScroll) ;
  else
  // Remove the hadjustment if we're freezing columns
  if (n_columns > 0 && gtk_tree_view_get_hadjustment (tv) == adjHScroll)
    gtk_tree_view_set_hadjustment (tv, NULL) ;

  if (n_columns > 0 && NULL != (adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar))))
    {
    adjHScroll->value = 0 ;
    adjHScroll->lower = 0 ;
    adjHScroll->upper = 1 ;
    adjHScroll->step_increment =
    adjHScroll->page_increment =
    adjHScroll->page_size = 1 ;
    set_hscroll_upper (tvc) ;
    gtk_adjustment_value_changed (adjHScroll) ;
    }
  }

static void tree_view_size_allocate (GtkTreeView *tv, GtkAllocation *alloc, gpointer data)
  {set_hscroll_upper (QCAD_TREE_VIEW_CONTAINER (data)) ;}

static void hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  int Nix ;
  GList *llCols = NULL, *llItr = NULL ;
  QCADTreeViewContainer *tvc = QCAD_TREE_VIEW_CONTAINER (data) ;

  llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (GTK_BIN (tvc)->child)) ;

  for (Nix = 0, llItr = llCols ; Nix < tvc->n_frozen_columns && NULL != llItr ; llItr = llItr->next, Nix++) ;

  for (Nix = adj->value ; llItr != NULL ; Nix--, llItr = llItr->next)
    gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN (llItr->data), (Nix <= 0)) ;

  g_list_free (llCols) ;
  }

static int tree_view_count_columns (GtkTreeView *tv)
  {
  int n_columns = 0 ;
  GList *llCols = NULL ;

  if (NULL == tv) return 0 ;

  llCols = gtk_tree_view_get_columns (tv) ;

  n_columns = g_list_length (llCols) ;

  g_list_free (llCols) ;

  return n_columns ;
  }

static void set_hscroll_upper (QCADTreeViewContainer *tvc)
  {
  GtkAdjustment *adjHScroll = NULL ;
  gboolean bUpperChanged = FALSE, bValueChanged = FALSE ;
  GdkRectangle rcTV = {0} ;
  GtkTreeView *tv = NULL ;
  GList *llCols = NULL, *llItr = NULL ;
  int Nix, icToHide = 0, icCols = 0, new_value = -1 ;

  if (0 == tvc->n_frozen_columns) return ;
  if (NULL == GTK_SCROLLED_WINDOW (tvc)->hscrollbar) return ;
  if ((icCols = tree_view_count_columns (tv = GTK_TREE_VIEW (GTK_BIN (tvc)->child))) <= tvc->n_frozen_columns) return ;
  if (NULL == (llCols = gtk_tree_view_get_columns (tv))) return ;

  gtk_tree_view_get_visible_rect (tv, &rcTV) ;

  for (Nix = 0, llItr = llCols; Nix < tvc->n_frozen_columns && llItr != NULL ; Nix++, llItr = llItr->next)
    rcTV.width -= tree_view_column_get_width (tv, GTK_TREE_VIEW_COLUMN (llItr->data), (NULL == llItr->next)) ;

  for (Nix = 0, llItr = g_list_last (llCols) ; llItr != NULL && Nix < icCols - tvc->n_frozen_columns ; llItr = llItr->prev, Nix++)
    if ((rcTV.width -= tree_view_column_get_width (tv, GTK_TREE_VIEW_COLUMN (llItr->data), (NULL == llItr->next))) < 0)
      icToHide++ ;

  g_list_free (llCols) ;

  adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar)) ;

  if ((bUpperChanged = (icToHide + 1 != adjHScroll->upper)))
    adjHScroll->upper = icToHide + 1 ;
  if ((bValueChanged = (adjHScroll->value != (new_value = CLAMP (adjHScroll->value, 0, adjHScroll->upper - adjHScroll->page_size)))))
    adjHScroll->value = new_value ;
  if (bUpperChanged)
    gtk_adjustment_changed (adjHScroll) ;
  if (bValueChanged)
    gtk_adjustment_value_changed (adjHScroll) ;
  }

static int tree_view_column_get_width (GtkTreeView *tv, GtkTreeViewColumn *col, gboolean bLastCol)
  {
  int col_width = 0 ;

  if (NULL == col) return 0 ;

  col_width = MAX (col->requested_width, col->button_request) ;
  if (-1 != col->min_width)
    col_width = MAX (col_width, col->min_width) ;
  if (-1 != col->max_width)
    col_width = MIN (col_width, col->max_width) ;

  return col_width ;
  }
