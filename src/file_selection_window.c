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



#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <stdio.h>

#include "support.h"
#include "fileio.h"
#include "file_selection_window.h"
#include "blocking_dialog.h"

#define DBG_FSW(s)

typedef struct{
	GtkWidget *fileselection;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
}file_select;

//!File selection dialog for opening/saving/importing/exporting files.
static file_select file_selection_dialog = {NULL};

void create_file_selection_dialog(file_select *select) ;
void file_selection_ok_button_clicked(GtkWidget *widget, gpointer data);
void file_selection_cancel_button_clicked(GtkWidget *widget, gpointer data);

gboolean get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, int cb)
  {
  gboolean bRet = FALSE ;
  
  if (NULL == file_selection_dialog.fileselection)
    create_file_selection_dialog (&file_selection_dialog) ;
  gtk_window_set_transient_for (GTK_WINDOW (file_selection_dialog.fileselection), parent) ;
  gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selection_dialog.fileselection), NULL != pszFName ? pszFName : "") ;
  gtk_window_set_title (GTK_WINDOW (file_selection_dialog.fileselection), pszWinTitle) ;
  DBG_FSW (fprintf (stderr, "Setting pszFName to 0x%08X\n", (int)pszFName)) ;
  gtk_object_set_data (GTK_OBJECT (file_selection_dialog.fileselection), "pszFName", pszFName) ;
  gtk_object_set_data (GTK_OBJECT (file_selection_dialog.fileselection), "pcb", &cb) ;
  gtk_object_set_data (GTK_OBJECT (file_selection_dialog.fileselection), "pbRet", &bRet) ;
  show_dialog_blocking (file_selection_dialog.fileselection) ;
  return bRet ;
  }

void create_file_selection_dialog(file_select *select){
	if (NULL != select->fileselection) return ;

	select->fileselection = gtk_file_selection_new("Select File");
	gtk_window_set_modal (GTK_WINDOW (select->fileselection), TRUE) ;
	
	select->ok_button = GTK_FILE_SELECTION(select->fileselection)->ok_button;
	select->cancel_button = GTK_FILE_SELECTION(select->fileselection)->cancel_button;
	
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(select->fileselection), "project.qca");
	
	gtk_widget_show(select->fileselection);

	gtk_signal_connect_object (GTK_OBJECT(select->fileselection), "delete_event",
	(GtkSignalFunc)gtk_widget_hide, GTK_OBJECT (select->fileselection));
	
	gtk_signal_connect (GTK_OBJECT(select->cancel_button), "clicked",
	(GtkSignalFunc)file_selection_cancel_button_clicked, GTK_OBJECT (select->fileselection));

	gtk_signal_connect (GTK_OBJECT(select->ok_button), "clicked",
	(GtkSignalFunc)file_selection_ok_button_clicked, GTK_OBJECT (select->fileselection));
}

void file_selection_ok_button_clicked(GtkWidget *widget, gpointer data)
  {
  GtkObject *pobj = GTK_OBJECT (data) ;
  char *pszFName = gtk_object_get_data (pobj, "pszFName") ;
  int *pcb = gtk_object_get_data (pobj, "pcb") ;
  gboolean *pbRet = gtk_object_get_data (pobj, "pbRet") ;
  char *pszFileSel = gtk_file_selection_get_filename (GTK_FILE_SELECTION (data)) ;
  
  DBG_FSW (fprintf (stderr, "pszFName = 0x%08X\n", (int)pszFName)) ;
  
  g_snprintf (pszFName, *pcb, "%s", pszFileSel) ;
  *pbRet = TRUE ;
  gtk_widget_hide (GTK_WIDGET (data)) ;
  }

void file_selection_cancel_button_clicked(GtkWidget *widget, gpointer data)
  {
  GtkObject *pobj = GTK_OBJECT (data) ;
  char *pszFName = gtk_object_get_data (pobj, "pszFName") ;
  int *pcb = gtk_object_get_data (pobj, "pcb") ;
  gboolean *pbRet = gtk_object_get_data (pobj, "pbRet") ;
  
  if (*pcb > 0)
  pszFName[0] = 0 ; /* Kill whatever string was stored in the user-provided buffer */
  *pbRet = FALSE ;
  gtk_widget_hide (GTK_WIDGET (data)) ;
  }
