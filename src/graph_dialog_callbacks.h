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
// Header for the the graph dialog callbacks to be used //
// during creation (graph_dialog_interface.c). These    //
// functions contain all the dialog behaviour.          //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _GRAPH_DIALOG_CALLBACKS_H_
#define _GRAPH_DIALOG_CALLBACKS_H_

#include <gtk/gtk.h>

#ifdef STDIO_FILEIO
void gd_actOpen_activate (GtkWidget *widget, gpointer user_data) ;
void gd_actSave_activate (GtkWidget *widget, gpointer user_data) ;
void gd_actPreview_activate (GtkWidget *widget, gpointer user_data) ;
#endif /* def STDIO_FILEIO */
void gd_actClose_activate (GtkWidget *widget, gpointer user_data) ;
void gd_actPrint_activate (GtkWidget *widget, gpointer user_data) ;
void gd_actThresh_activate (GtkWidget *widget, gpointer user_data) ;
void gd_hscroll_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
void gd_set_bus_expanded (GtkTreeView *tview, GtkTreeIter *itrBus, GtkTreePath *tpath, gpointer data) ;
void gd_model_visible_toggled (GtkCellRenderer *cr, char *pszTreePath, gpointer data) ;
void gd_actShowBase_activate (GtkRadioAction *action, GtkRadioAction *current, gpointer data) ;
void gd_actZoom100_activate (GtkWidget *widget, gpointer data) ;
gboolean gd_graph_widget_size_allocate (GtkWidget *widget, GtkAllocation *alloc, gpointer data) ;
gboolean gd_waveform_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
gboolean gd_honeycomb_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
gboolean gd_viewport_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data) ;
//gboolean trace_ruler_motion_event (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
//gboolean trace_motion_update_rulers (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
//gboolean gd_graph_widget_one_time_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
gboolean gd_graph_widget_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
gboolean gd_graph_widget_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
gboolean gd_graph_widget_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
gboolean gd_graph_widget_enter_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;
gboolean gd_graph_widget_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;

#endif /* def _GRAPH_DIALOG_CALLBACKS_H_ */
