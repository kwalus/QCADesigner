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
// (schulhof@vlsi.enel.ucalgary.ca).  The graph options //
// dialog box code was initially part of the original   //
// graph dialog code, but was re-written using a clean- //
// er user interface and far fewer global variables     //
//////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "blocking_dialog.h"
#include "message_box.h"
#include "graph_options_dialog.h"

/* minumum number of traces to be visible in the list */
#define MIN_NUMBER_VISIBLE_TRACES 5

typedef struct
  {
  GtkWidget *dlgGraphOptions;
  GtkWidget *vbdlgGraphOptions;
  GtkWidget *tblMain;
  GtkWidget *lblGraphOptions;
  GtkWidget *swndTraceList;
  GtkWidget *vpTraceList;
  GtkWidget *vbTraceList;
  GtkWidget *chkTraceList;
  GtkWidget *aadlgGraphOptions;
  GtkWidget *hbboxOKCancel;
  GtkWidget *btnOK;
  GtkWidget *btnCancel;
  GtkWidget **pchkTraces ;
  int icTraces ;
  GtkWidget **pchkClocks ;
  int icClocks ;
  } graph_options_D ;

static graph_options_D graph_options = {NULL} ;

void create_graph_options_dialog (graph_options_D *dialog) ;
void graph_options_btnOK_clicked (GtkWidget *widget, gpointer user_data) ;

/* The main function */
void get_graph_options_from_user (GtkWindow *parent, struct TRACEDATA *traces, int icTraces, struct TRACEDATA *clocks, int icClocks)
  {
  int Nix ;
  
  if ((NULL == traces && NULL == clocks) ||
      (NULL == traces && 0 == icClocks) ||
      (0 == icTraces && NULL == clocks) ||
      (0 == icTraces && 0 == icClocks))
    {
    /* We should never get here(tm) */
    message_box (parent, MB_OK, "What Graph Options ?", "There are no traces available for selection !") ;
    return ;
    }
  
  if (NULL == graph_options.dlgGraphOptions)
    create_graph_options_dialog (&graph_options) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (graph_options.dlgGraphOptions), parent) ;
  gtk_object_set_data (GTK_OBJECT (graph_options.dlgGraphOptions), "dialog", &graph_options) ;
  gtk_object_set_data (GTK_OBJECT (graph_options.dlgGraphOptions), "traces", traces) ;
  gtk_object_set_data (GTK_OBJECT (graph_options.dlgGraphOptions), "picTraces", &icTraces) ;
  gtk_object_set_data (GTK_OBJECT (graph_options.dlgGraphOptions), "clocks", clocks) ;
  gtk_object_set_data (GTK_OBJECT (graph_options.dlgGraphOptions), "picClocks", &icClocks) ;

  /* Clean out old traces */
  if (0 != graph_options.icTraces && NULL != graph_options.pchkTraces)
    {
    for (Nix = 0 ; Nix < graph_options.icTraces ; Nix++)
      gtk_widget_destroy (graph_options.pchkTraces[Nix]) ;
    free (graph_options.pchkTraces) ;
    }
  /* Clean out old clocks */
  if (0 != graph_options.icClocks && NULL != graph_options.pchkClocks)
    {
    for (Nix = 0 ; Nix < graph_options.icClocks ; Nix++)
      gtk_widget_destroy (graph_options.pchkClocks[Nix]) ;
    free (graph_options.pchkClocks) ;
    }
  
  /* Add checkboxen for all the traces */
  if (0 != icTraces && NULL != traces)
    {
    graph_options.icTraces = icTraces ;
    graph_options.pchkTraces = malloc (icTraces * sizeof (GtkWidget *)) ;
    for (Nix = 0 ; Nix < icTraces ; Nix++)
      {
      graph_options.pchkTraces[Nix] = gtk_check_button_new_with_label (_(traces[Nix].data_labels)) ;
      gtk_widget_show (graph_options.pchkTraces[Nix]) ;
      gtk_container_add (GTK_CONTAINER (graph_options.vbTraceList), graph_options.pchkTraces[Nix]) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (graph_options.pchkTraces[Nix]), traces[Nix].drawtrace) ;
      }
    }
  
  /* Add checkboxen for all the clocks */
  if (0 != icClocks && NULL != clocks)
    {
    graph_options.icClocks = icClocks ;
    graph_options.pchkClocks = malloc (icClocks * sizeof (GtkWidget *)) ;
    for (Nix = 0 ; Nix < icClocks ; Nix++)
      {
      graph_options.pchkClocks[Nix] = gtk_check_button_new_with_label (_(clocks[Nix].data_labels)) ;
      gtk_widget_show (graph_options.pchkClocks[Nix]) ;
      gtk_container_add (GTK_CONTAINER (graph_options.vbTraceList), graph_options.pchkClocks[Nix]) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (graph_options.pchkClocks[Nix]), clocks[Nix].drawtrace) ;
      }
    }
  
  show_dialog_blocking (graph_options.dlgGraphOptions) ;
  }

void create_graph_options_dialog (graph_options_D *dialog)
  {
  /* The dialog */
  dialog->dlgGraphOptions = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphOptions), "dlgGraphOptions", dialog->dlgGraphOptions);
  gtk_window_set_title (GTK_WINDOW (dialog->dlgGraphOptions), _("Graph Options"));
  GTK_WINDOW (dialog->dlgGraphOptions)->type = GTK_WINDOW_DIALOG;
  gtk_window_set_modal (GTK_WINDOW (dialog->dlgGraphOptions), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog->dlgGraphOptions), FALSE, FALSE, FALSE);
  gtk_widget_set_usize (dialog->dlgGraphOptions, 300, 200) ;

  dialog->vbdlgGraphOptions = GTK_DIALOG (dialog->dlgGraphOptions)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphOptions), "vbdlgGraphOptions", dialog->vbdlgGraphOptions);
  gtk_widget_show (dialog->vbdlgGraphOptions);

  /* The main table */
  dialog->tblMain = gtk_table_new (2, 1, FALSE);
  gtk_widget_ref (dialog->tblMain);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "tblMain", dialog->tblMain,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->tblMain);
  gtk_box_pack_start (GTK_BOX (dialog->vbdlgGraphOptions), dialog->tblMain, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblMain), 2);

  dialog->lblGraphOptions = gtk_label_new (_("Please select which traces to plot:"));
  gtk_widget_ref (dialog->lblGraphOptions);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "lblGraphOptions", dialog->lblGraphOptions,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->lblGraphOptions);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->lblGraphOptions, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblGraphOptions), 0, 0.5);

  /* The trace list containers */
  dialog->swndTraceList = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (dialog->swndTraceList);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "swndTraceList", dialog->swndTraceList,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->swndTraceList);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->swndTraceList, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swndTraceList), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->vpTraceList = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (dialog->vpTraceList);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "vpTraceList", dialog->vpTraceList,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vpTraceList);
  gtk_container_add (GTK_CONTAINER (dialog->swndTraceList), dialog->vpTraceList);

  /* The trace list vbox (which will contain all the checkboxen */
  dialog->vbTraceList = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (dialog->vbTraceList);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "vbTraceList", dialog->vbTraceList,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->vbTraceList);
  gtk_container_add (GTK_CONTAINER (dialog->vpTraceList), dialog->vbTraceList);

  /* The main button containers */
  dialog->aadlgGraphOptions = GTK_DIALOG (dialog->dlgGraphOptions)->action_area;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphOptions), "aadlgGraphOptions", dialog->aadlgGraphOptions);
  gtk_widget_show (dialog->aadlgGraphOptions);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->aadlgGraphOptions), 0) ;

  dialog->hbboxOKCancel = gtk_hbutton_box_new ();
  gtk_widget_ref (dialog->hbboxOKCancel);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "hbboxOKCancel", dialog->hbboxOKCancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->hbboxOKCancel);
  gtk_box_pack_start (GTK_BOX (dialog->aadlgGraphOptions), dialog->hbboxOKCancel, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbboxOKCancel), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbboxOKCancel), 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbboxOKCancel), 0, -1);

  /* The buttons */
  dialog->btnOK = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (dialog->btnOK);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "btnOK", dialog->btnOK,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnOK);
  gtk_container_add (GTK_CONTAINER (dialog->hbboxOKCancel), dialog->btnOK);
  GTK_WIDGET_SET_FLAGS (dialog->btnOK, GTK_CAN_DEFAULT);

  dialog->btnCancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (dialog->btnCancel);
  gtk_object_set_data_full (GTK_OBJECT (dialog->dlgGraphOptions), "btnCancel", dialog->btnCancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dialog->btnCancel);
  gtk_container_add (GTK_CONTAINER (dialog->hbboxOKCancel), dialog->btnCancel);
  GTK_WIDGET_SET_FLAGS (dialog->btnCancel, GTK_CAN_DEFAULT);
  
  /* The default hiding behaviour */
  gtk_signal_connect_object (GTK_OBJECT (dialog->dlgGraphOptions), "delete_event", (GtkSignalFunc)gtk_widget_hide, GTK_OBJECT (dialog->dlgGraphOptions)) ;
  gtk_signal_connect_object (GTK_OBJECT (dialog->btnCancel), "clicked", (GtkSignalFunc)gtk_widget_hide, GTK_OBJECT (dialog->dlgGraphOptions)) ;
  /* The OK button */
  gtk_signal_connect (GTK_OBJECT (dialog->btnOK), "clicked", (GtkSignalFunc)graph_options_btnOK_clicked, dialog->dlgGraphOptions) ;
  }

/* (Re)set the "drawtrace" flag for all traces based on the state of the checkboxen */
void graph_options_btnOK_clicked (GtkWidget *widget, gpointer user_data)
  {
  int Nix ;
  graph_options_D *dialog = (graph_options_D *)gtk_object_get_data (GTK_OBJECT (user_data), "dialog") ;
  struct TRACEDATA *traces = (struct TRACEDATA *)gtk_object_get_data (GTK_OBJECT (user_data), "traces") ;
  struct TRACEDATA *clocks = (struct TRACEDATA *)gtk_object_get_data (GTK_OBJECT (user_data), "clocks") ;
  int *picTraces = (int *)gtk_object_get_data (GTK_OBJECT (user_data), "picTraces") ;
  int *picClocks = (int *)gtk_object_get_data (GTK_OBJECT (user_data), "picClocks") ;
  
  if (NULL != traces && 0 != *picTraces)
    for (Nix = 0 ; Nix < dialog->icTraces ; Nix++)
      traces[Nix].drawtrace = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->pchkTraces[Nix])) ;
  if (NULL != clocks && 0 != *picClocks)
    for (Nix = 0 ; Nix < dialog->icClocks ; Nix++)
      clocks[Nix].drawtrace = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->pchkClocks[Nix])) ;

  gtk_widget_hide (dialog->dlgGraphOptions) ;
  }
