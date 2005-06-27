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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "support.h"
#include "global_consts.h"
#include "vector_table_options_dialog.h"

typedef struct
  {
  GtkWidget *dialog;
  GtkWidget *tbtnVT ;
  GtkWidget *tbtnExhaustive ;
  GtkWidget *btnOpen ;
  GtkWidget *btnSave ;
  } vector_table_options_D ;

VectorTable *pvt ;

static vector_table_options_D vto = {NULL} ;

static void btnOpen_clicked (GtkWidget *widget, gpointer data) ;
static void btnSave_clicked (GtkWidget *widget, gpointer data) ;
static void btnClose_clicked (GtkWidget *widget, gpointer data) ;
static void btnSimType_clicked (GtkWidget *widget, gpointer data) ;

static void create_vector_table_options_dialog (vector_table_options_D *pnvto) ;
static void VectorTableToDialog (vector_table_options_D *dialog, int *sim_type, VectorTable *pvt) ;
static void DialogToVectorTable (vector_table_options_D *dialog) ;

void get_vector_table_options_from_user (GtkWindow *parent, int *sim_type, VectorTable *pvt)
  {
  if (NULL == vto.dialog)
    create_vector_table_options_dialog (&vto) ;

  gtk_window_set_transient_for (GTK_WINDOW (vto.dialog), parent) ;

  VectorTableToDialog (&vto, sim_type, pvt) ;

  gtk_widget_show (vto.dialog) ;

  while (GTK_WIDGET_VISIBLE (vto.dialog))
    gtk_main_iteration () ;
  }

static void btnClose_clicked (GtkWidget *widget, gpointer data)
  {
  VectorTable *pvt = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;

  if (NULL == dialog) return ;

  DialogToVectorTable (dialog) ;

  if (NULL != dialog)
    gtk_widget_hide (dialog->dialog) ;
  }

static void btnSimType_clicked (GtkWidget *widget, gpointer data) 
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  int sim_type = (int)g_object_get_data (G_OBJECT (widget), "sim_type") ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  if (VECTOR_TABLE == sim_type)
    {
    gtk_widget_set_sensitive (dialog->btnOpen, TRUE) ;
    gtk_widget_set_sensitive (dialog->btnSave, TRUE) ;
    }
  else
    {
    gtk_widget_set_sensitive (dialog->btnOpen, FALSE) ;
    gtk_widget_set_sensitive (dialog->btnSave, FALSE) ;
    }
  }

static void btnOpen_clicked (GtkWidget *widget, gpointer data) {}
static void btnSave_clicked (GtkWidget *widget, gpointer data) {}

static void VectorTableToDialog (vector_table_options_D *dialog, int *sim_type, VectorTable *pvt)
  {
  if (NULL == dialog || NULL == sim_type || NULL == pvt) return ;

  g_object_set_data (G_OBJECT (dialog->dialog), "user_sim_type", sim_type) 
  g_object_set_data (G_OBJECT (dialog->dialog), "user_pvt", pvt) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (VECTOR_TABLE == (*sim_type) ? dialog->tbtnVT : dialog->tbtnExhaustive), TRUE) ;
  }

static void DialogToVectorTable (vector_table_options_D *dialog)
  {
  int *sim_type = NULL ;
  VectorTable *pvt = NULL ;

  if (NULL == dialog) return ;

  sim_type = g_object_get_data (G_OBJECT (dialog->dialog), 

  if (NULL == sim_type || NULL == pvt) return ;

  (*sim_type) = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->tbtnVT)) ? VECTOR_TABLE : EXHAUSTIVE_VERIFICATION ;
  }

static void create_vector_table_options_dialog (vector_table_options_D *dialog)
  {
  GtkWidget *tbl = NULL, *toolbar = NULL, *btn = NULL, *btnBaseRadioSource = NULL ;
  GtkAccelGroup *accel_group = NULL ;

  accel_group = gtk_accel_group_new () ;

  dialog->dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (dialog->dialog), TRUE);
  gtk_window_set_title (GTK_WINDOW (dialog->dialog), _("Vector Table Setup"));
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 640, 480);
  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), TRUE) ;

  tbl = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (dialog->dialog), tbl) ;

  toolbar = gtk_toolbar_new () ;
  gtk_widget_show (toolbar) ;
  gtk_table_attach (GTK_TABLE (tbl), toolbar, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL) ;
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar), TRUE) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH) ;

  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Close"),
    _("Close Window"),
    _("Close vector table editor."),
    gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnClose_clicked,
    dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

#ifdef STDIO_FILEIO
  dialog->btnOpen =
  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Open"),
    _("Open Simulation Results"),
    _("Open and display another set of simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnOpen_clicked,
    dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;

  dialog->btnSave =
  btn = gtk_toolbar_append_element (
    GTK_TOOLBAR (toolbar),
    GTK_TOOLBAR_CHILD_BUTTON,
    NULL,
    _("Save"),
    _("Save Simulation Results"),
    _("Save the displayed simulation results."),
    gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_LARGE_TOOLBAR),
    (GCallback)btnSave_clicked,
    dialog->dialog) ;
	gtk_widget_add_accelerator (btn, "clicked", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE) ;
#endif /* def STDIO_FILEIO */

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar)) ;

  g_object_set_data (G_OBJECT (
    dialog->tbtnExhaustive =
    btnBaseRadioSource = gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      NULL,
      _("Exhaustive"),
      _("Exhaustive Verification"),
      _("Attempt all possible inputs."),
      gtk_image_new_from_stock (GTK_STOCK_YES, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnSimType_clicked,
      dialog)),
    "sim_type", (gpointer)EXHAUSTIVE_VERIFICATION) ;

  g_object_set_data (G_OBJECT (
    dialog->tbtnVT =
    gtk_toolbar_append_element (
      GTK_TOOLBAR (toolbar),
      GTK_TOOLBAR_CHILD_RADIOBUTTON,
      btnBaseRadioSource,
      _("Vector Table"),
      _("Vector Table Simulation"),
      _("Create a sequence of inputs."),
      gtk_image_new_from_stock (GTK_STOCK_NO, GTK_ICON_SIZE_LARGE_TOOLBAR),
      (GCallback)btnSimType_clicked,
      dialog)),
    "sim_type", (gpointer)VECTOR_TABLE) ;

  g_signal_connect (G_OBJECT (dialog->dialog), "delete-event", (GCallback)btnClose_clicked, dialog) ;

  gtk_window_add_accel_group (GTK_WINDOW (dialog->dialog), accel_group) ;
  }
