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
  GtkWidget *hbboxOKCancel;
  GtkWidget *btnOK;
  GtkWidget *btnCancel;
  GtkWidget **pchkTraces ;
  int icTraces ;
  GtkWidget **pchkClocks ;
  int icClocks ;
  } graph_options_D ;

static graph_options_D graph_options = {NULL} ;

static void create_graph_options_dialog (graph_options_D *dialog) ;

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
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "There are no available traces !"))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return ;
    }
  
  if (NULL == graph_options.dlgGraphOptions)
    create_graph_options_dialog (&graph_options) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (graph_options.dlgGraphOptions), parent) ;

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

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (graph_options.dlgGraphOptions)))
    {
    if (NULL != traces && 0 != icTraces)
      for (Nix = 0 ; Nix < graph_options.icTraces ; Nix++)
        traces[Nix].drawtrace = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (graph_options.pchkTraces[Nix])) ;
    if (NULL != clocks && 0 != icClocks)
      for (Nix = 0 ; Nix < graph_options.icClocks ; Nix++)
        clocks[Nix].drawtrace = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (graph_options.pchkClocks[Nix])) ;
    }

  /* Clean out traces */
  if (0 != graph_options.icTraces && NULL != graph_options.pchkTraces)
    {
    for (Nix = 0 ; Nix < graph_options.icTraces ; Nix++)
      gtk_container_remove (GTK_CONTAINER (graph_options.vbTraceList), graph_options.pchkTraces[Nix]) ;
    free (graph_options.pchkTraces) ;
    }

  /* Clean out clocks */
  if (0 != graph_options.icClocks && NULL != graph_options.pchkClocks)
    {
    for (Nix = 0 ; Nix < graph_options.icClocks ; Nix++)
      gtk_container_remove (GTK_CONTAINER (graph_options.vbTraceList), graph_options.pchkClocks[Nix]) ;
    free (graph_options.pchkClocks) ;
    }
  
  graph_options.pchkClocks = NULL ;
  graph_options.icClocks = 0 ;
  graph_options.pchkTraces = NULL ;
  graph_options.icTraces = 0 ;
  
  gtk_widget_hide (graph_options.dlgGraphOptions) ;
  }

static void create_graph_options_dialog (graph_options_D *dialog)
  {
  /* The dialog */
  dialog->dlgGraphOptions = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphOptions), "dlgGraphOptions", dialog->dlgGraphOptions);
  gtk_window_set_title (GTK_WINDOW (dialog->dlgGraphOptions), _("Graph Options"));
//  GTK_WINDOW (dialog->dlgGraphOptions)->type = GTK_WINDOW_DIALOG;
  gtk_window_set_modal (GTK_WINDOW (dialog->dlgGraphOptions), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog->dlgGraphOptions), FALSE, FALSE, FALSE);
  gtk_widget_set_usize (dialog->dlgGraphOptions, 300, 200) ;

  dialog->vbdlgGraphOptions = GTK_DIALOG (dialog->dlgGraphOptions)->vbox;
  gtk_object_set_data (GTK_OBJECT (dialog->dlgGraphOptions), "vbdlgGraphOptions", dialog->vbdlgGraphOptions);
  gtk_widget_show (dialog->vbdlgGraphOptions);

  /* The main table */
  dialog->tblMain = gtk_table_new (2, 1, FALSE);
  gtk_widget_show (dialog->tblMain);
  gtk_box_pack_start (GTK_BOX (dialog->vbdlgGraphOptions), dialog->tblMain, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tblMain), 2);

  dialog->lblGraphOptions = gtk_label_new (_("Please select which traces to plot:"));
  gtk_widget_show (dialog->lblGraphOptions);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->lblGraphOptions, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 2, 2);
  gtk_misc_set_alignment (GTK_MISC (dialog->lblGraphOptions), 0, 0.5);

  /* The trace list containers */
  dialog->swndTraceList = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (dialog->swndTraceList);
  gtk_table_attach (GTK_TABLE (dialog->tblMain), dialog->swndTraceList, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swndTraceList), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  dialog->vpTraceList = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (dialog->vpTraceList);
  gtk_container_add (GTK_CONTAINER (dialog->swndTraceList), dialog->vpTraceList);

  /* The trace list vbox (which will contain all the checkboxen */
  dialog->vbTraceList = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (dialog->vbTraceList);
  gtk_container_add (GTK_CONTAINER (dialog->vpTraceList), dialog->vbTraceList);

  dialog->btnCancel = gtk_dialog_add_button (GTK_DIALOG (dialog->dlgGraphOptions), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  dialog->btnOK = gtk_dialog_add_button (GTK_DIALOG (dialog->dlgGraphOptions), GTK_STOCK_OK, GTK_RESPONSE_OK);
  
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgGraphOptions), GTK_RESPONSE_OK) ;
  }
