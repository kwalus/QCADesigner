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
// A tree view container that allows "freeze columns".  //
// That is, the horizontal scrolling does not scroll    //
// the entire tree view but, instead, it hides and      //
// shows columns as appropriate, keeping the first n    //
// columns always visible.                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include "../generic_utils.h"
#include "../support.h"
#include "QCADScrolledWindow.h"

enum
  {
  QCAD_TREE_VIEW_CONTAINER_PROPERTY_FIRST=1,

  QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_HADJ,
  QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_VADJ,

  QCAD_TREE_VIEW_CONTAINER_PROPERTY_LAST
  } ;

static void qcad_scrolled_window_class_init (QCADScrolledWindowClass *klass) ;
static void qcad_scrolled_window_instance_init (QCADScrolledWindow *instance) ;

static void finalize (GObject *obj) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void add (GtkContainer *container, GtkWidget *child) ;

static void fake_hadj_value_changed (GtkAdjustment *adj, gpointer data) ;

static void qcad_scrolled_window_update_custom_adjustments (QCADScrolledWindow *qsw) ;

GType qcad_scrolled_window_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADScrolledWindowClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_scrolled_window_class_init,
      NULL,
      NULL,
      sizeof (QCADScrolledWindow),
      0,
      (GInstanceInitFunc)qcad_scrolled_window_instance_init
      } ;

    if (0 != (the_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW, QCAD_TYPE_STRING_SCROLLED_WINDOW, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void qcad_scrolled_window_class_init (QCADScrolledWindowClass *klass)
  {
  GObjectClass *object_class = G_OBJECT_CLASS (klass) ;

  object_class->finalize     = finalize ;
  object_class->get_property = get_property ;
  object_class->set_property = set_property ;

  GTK_CONTAINER_CLASS (klass)->add = add ;

  g_object_class_install_property (object_class, QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_HADJ,
    g_param_spec_boolean ("custom-hadjustment", _("Custom Horizontal Adjustment"), _("If not set, horizontal scrolling is handled automatically"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (object_class, QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_VADJ,
    g_param_spec_boolean ("custom-vadjustment", _("Custom Vertical Adjustment"), _("If not set, vertical scrolling is handled automatically"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_scrolled_window_instance_init (QCADScrolledWindow *instance)
  {
  instance->bCustomHScroll = FALSE ;
  instance->bCustomVScroll = FALSE ;
  instance->fake_hadj = GTK_ADJUSTMENT (g_object_ref (G_OBJECT (gtk_adjustment_new (0, 0, 1, 1, 1, 1)))) ;
  instance->fake_vadj = GTK_ADJUSTMENT (g_object_ref (G_OBJECT (gtk_adjustment_new (0, 0, 1, 1, 1, 1)))) ;
  g_signal_connect (G_OBJECT (instance->fake_hadj), "value-changed", (GCallback)fake_hadj_value_changed, NULL) ;
  g_signal_connect (G_OBJECT (instance->fake_vadj), "value-changed", (GCallback)fake_hadj_value_changed, NULL) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADScrolledWindow *scrolled_window = QCAD_SCROLLED_WINDOW (object) ;

  switch (property_id)
    {
    case QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_HADJ:
      if (scrolled_window->bCustomHScroll != g_value_get_boolean (value))
        {
        scrolled_window->bCustomHScroll = (!(scrolled_window->bCustomHScroll)) ;
        qcad_scrolled_window_update_custom_adjustments (scrolled_window) ;
        g_object_notify (object, "custom-hadjustment") ;
        }
      break ;

    case QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_VADJ:
      if (scrolled_window->bCustomVScroll != g_value_get_boolean (value))
        {
        scrolled_window->bCustomVScroll = (!(scrolled_window->bCustomVScroll)) ;
        qcad_scrolled_window_update_custom_adjustments (scrolled_window) ;
        g_object_notify (object, "custom-vadjustment") ;
        }
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADScrolledWindow *scrolled_window = QCAD_SCROLLED_WINDOW (object) ;

  switch (property_id)
    {
    case QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_HADJ:
      g_value_set_boolean (value, scrolled_window->bCustomHScroll) ;
      break ;

    case QCAD_SCROLLED_WINDOW_PROPERTY_CUSTOM_VADJ:
      g_value_set_boolean (value, scrolled_window->bCustomVScroll) ;
      break ;
    }
  }

static void finalize (GObject *obj)
  {
  QCADScrolledWindow *scrolled_window = QCAD_SCROLLED_WINDOW (obj) ;

  g_object_unref (G_OBJECT (scrolled_window->fake_hadj)) ;
  g_object_unref (G_OBJECT (scrolled_window->fake_vadj)) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_SCROLLED_WINDOW)))->finalize (obj) ;
  }

static void add (GtkContainer *container, GtkWidget *child)
  {
  GtkScrolledWindow *sw = NULL ;
  QCADScrolledWindow *qsw = NULL ;

  sw  = GTK_SCROLLED_WINDOW (container) ;
  qsw = QCAD_SCROLLED_WINDOW (container) ;

  GTK_BIN (container)->child = child ;
  gtk_widget_set_parent (child, GTK_WIDGET (container)) ;
  gtk_widget_set_scroll_adjustments (child, 
    gtk_range_get_adjustment (GTK_RANGE (sw->hscrollbar)), 
    gtk_range_get_adjustment (GTK_RANGE (sw->vscrollbar))) ;

  if (qsw->bCustomHScroll || qsw->bCustomVScroll)
    qcad_scrolled_window_update_custom_adjustments (qsw) ;
/*
  if (-1 == tvc->notify_id)
    tvc->notify_id = 
      g_signal_connect (G_OBJECT (gtk_range_get_adjustment (GTK_RANGE (sw->hscrollbar))),
        "value-changed", (GCallback)hscroll_adj_value_changed, container) ;
*/
  }

GtkWidget *qcad_scrolled_window_new ()
  {
  GtkWidget *ret = g_object_new (QCAD_TYPE_SCROLLED_WINDOW, "hadjustment", NULL, "vadjustment", NULL, NULL) ;
  return ret ;
  }

static void qcad_scrolled_window_update_custom_adjustments (QCADScrolledWindow *qsw)
  {
  GtkAdjustment *adjHScroll = NULL ;
  GtkAdjustment *adjVScroll = NULL ;
  GtkWidget *child = GTK_BIN (qsw)->child ;

  if (NULL == child) return ;

  // Set the tree view hadjustment if we're going back to normal scrolling, and show all tree view columns
  adjHScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (qsw)->hscrollbar)) ;
  adjVScroll = gtk_range_get_adjustment (GTK_RANGE (GTK_SCROLLED_WINDOW (qsw)->vscrollbar)) ;

  if (qsw->bCustomHScroll) adjHScroll = qsw->fake_hadj ;
  if (qsw->bCustomVScroll) adjVScroll = qsw->fake_vadj ;

  gtk_widget_set_scroll_adjustments (child, adjHScroll, adjVScroll) ;
  GTK_WIDGET_GET_CLASS (child)->size_allocate (child, &(child->allocation)) ;
  }

static void fake_hadj_value_changed (GtkAdjustment *adj, gpointer data)
  {if (adj->value != adj->lower) gtk_adjustment_set_value (adj, adj->lower) ;}
