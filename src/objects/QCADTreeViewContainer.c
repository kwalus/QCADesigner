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
static void qcad_tree_view_container_set_initial_hadjustment (QCADTreeViewContainer *tvc) ;

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
  if (!GTK_IS_TREE_VIEW (child)) return ;
  GTK_BIN (container)->child = child ;
  gtk_widget_set_parent (child, GTK_WIDGET (container)) ;
  gtk_tree_view_set_vadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->vscrollbar))) ;
  gtk_tree_view_set_hadjustment (GTK_TREE_VIEW (child), gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (container)->hscrollbar))) ;
  }

GtkWidget *qcad_tree_view_container_new ()
  {return g_object_new (QCAD_TYPE_TREE_VIEW_CONTAINER, "hadjustment", NULL, "vadjustment", NULL, NULL) ;}

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
  if (0 == n_columns && 
    (gtk_tree_view_get_hadjustment (tv) != (adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (tvc)->hscrollbar)))))
    gtk_tree_view_set_hadjustment (tv, adjHScroll) ;
  else
  // Remove the hadjustment if we're freezing columns
  if (n_columns > 0 && gtk_tree_view_get_hadjustment (tv) == adjHScroll)
    gtk_tree_view_set_hadjustment (tv, NULL) ;

  if (n_columns > 0)
    {
    adjHScroll->value = 0 ;
    adjHScroll->lower = 0 ;
    adjHScroll->upper = n_tv_cols - n_columns ;
    adjHScroll->step_increment =
    adjHScroll->page_increment =
    adjHScroll->page_size = 1 ;
    gtk_adjustment_changed (adjHScroll) ;
    gtk_adjustment_value_changed (adjHScroll) ;
    }
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
