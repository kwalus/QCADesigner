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
// Contents:                                            //
//                                                      //
// File selection window. This wraps up a               //
// GtkFileSelection. After all, all we want is          //
// something that'll return a string containing the ab- //
// solute path to a user-selected file name.            //
//                                                      //
//////////////////////////////////////////////////////////
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#include "support.h"
#include "global_consts.h"
#include "file_selection_window.h"
#include "fileio_helpers.h"
#include "custom_widgets.h"

#define DBG_FSW(s) s

#define QCA_CELL_LIBRARY PACKAGE_DATA_DIR G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "qca_cell_library"

typedef struct
  {
  GtkWidget *dialog ;
  GtkWidget *ok_button ;
  } file_selection_D ;

static void create_file_selection_dialog (file_selection_D *dialog) ;
static char *get_default_file_name (GtkFileChooser *file_chooser, char *pszFName) ;

static file_selection_D file_selection_dialog = {NULL} ;

gchar *get_file_name_from_user (GtkWindow *parent, char *pszWinTitle, char *pszFName, gboolean bSave)
  {
  int response = GTK_RESPONSE_NONE ;
  GtkWidget *msg = NULL ;
  gchar *pszRet = NULL ;

  if (NULL == file_selection_dialog.dialog)
    create_file_selection_dialog (&file_selection_dialog) ;

  g_object_set (G_OBJECT (file_selection_dialog.dialog),
    "title",  (NULL == pszWinTitle) ? g_type_name (G_TYPE_FROM_INSTANCE (file_selection_dialog.dialog)) : pszWinTitle,
    "action", bSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
    NULL) ;

  g_object_set (G_OBJECT (file_selection_dialog.ok_button),
    "label",     bSave ? GTK_STOCK_SAVE : GTK_STOCK_OPEN,
    "use-stock", TRUE,
    NULL) ;

  gtk_window_set_transient_for (GTK_WINDOW (file_selection_dialog.dialog), parent) ;

  if (NULL != (pszRet = get_default_file_name (GTK_FILE_CHOOSER (file_selection_dialog.dialog), pszFName)))
    if (0 != pszRet[0])
      {
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_selection_dialog.dialog), pszRet) ;
      g_free (pszRet) ;
      pszRet = NULL ;
      }

  while (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (file_selection_dialog.dialog)))
    {
    if (NULL == (pszRet = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selection_dialog.dialog)))) break ;

    if (!bSave) break ;

    if (g_file_test (pszRet, G_FILE_TEST_EXISTS))
      {
      msg = gtk_message_dialog_new (GTK_WINDOW (file_selection_dialog.dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("A file named \"%s\" already exists.\n\nDo you want to replace it with the one you are saving?"), pszRet) ;

      gtk_window_set_transient_for (GTK_WINDOW (msg), GTK_WINDOW (file_selection_dialog.dialog)) ;
      gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
      gtk_dialog_add_action_widget (GTK_DIALOG (msg), gtk_button_new_with_stock_image (GTK_STOCK_REFRESH, _("_Replace")), GTK_RESPONSE_OK) ;

      response = gtk_dialog_run (GTK_DIALOG (msg)) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;

      if (GTK_RESPONSE_OK == response) break ;
      }
    else
      break ;

    g_free (pszRet) ;
    pszRet = NULL ;

    }
  gtk_widget_hide (file_selection_dialog.dialog) ;

  if (NULL != parent)
    gtk_window_present (parent) ;

  return pszRet ;
  }

void set_file_selection_file_name (char *pszFName)
  {
  if (NULL == pszFName) return ;

  if (NULL == file_selection_dialog.dialog)
    create_file_selection_dialog (&file_selection_dialog) ;

  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_selection_dialog.dialog), pszFName) ;
  }

gchar *get_external_app (GtkWindow *parent, char *pszWinTitle, char *pszCfgFName, char *pszDefaultContents, gboolean bForceNew)
  {
  char *pszRet = NULL ;
  char *pszUserFName = NULL ;
  char *pszLine = NULL ;
  FILE *pfile = NULL ;
  int Nix, ic ;
#ifdef WIN32
  char szBuf[PATH_LENGTH] = "" ;
#endif

  pszUserFName = CreateUserFName (pszCfgFName) ;
  if (NULL == (pfile = fopen (pszUserFName, "r")))
    pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
  else
    {
    if (NULL == (pszLine = ReadLine (pfile, 0, FALSE)))
      {
      fclose (pfile) ;
      pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
      }
    else
      pszRet = g_strdup (pszLine) ;
    fclose (pfile) ;
    }

  if (NULL == pszRet)
    return NULL ;

  if (0 == pszRet[0]) // grabbed empty string from file
    {
    g_free (pszRet) ;
    pszRet = get_file_name_from_user (parent, pszWinTitle, pszDefaultContents, FALSE) ;
    }

  if (NULL == pszRet) // User clicked cancel
    return NULL ;

  if (0 == pszRet[0])
    {
    // After much effort, a command line could not be conjured up.
    // Give the user the bad news.
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK, _("Unable to locate path!")))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszRet) ;
    return NULL ;
    }

  ic = strlen (pszRet) ;

  for (Nix = 0 ; Nix < ic ; Nix++)
    if ('\r' == pszRet[Nix] || '\n' == pszRet[Nix])
      {
      pszRet[Nix] = 0 ;
      break ;
      }

#ifdef WIN32
  // In Windoze, we need to perform the extra step of grabbing
  // the DOS-style path corresponding to the app path
  GetShortPathName (pszRet, szBuf, PATH_LENGTH) ;
  g_free (pszRet) ;
  pszRet = g_strdup_printf ("%s", szBuf) ;
#endif

  /* Save the app to the config file */
  if (NULL != (pfile = fopen (pszUserFName, "w")))
    {
    fprintf (pfile, "%s\n", pszRet) ;
    fclose (pfile) ;
    }

  return pszRet ;
  }

static char *get_default_file_name (GtkFileChooser *file_chooser, char *pszFName)
  {
  gboolean bHaveCellLib = FALSE ;
  char *pszCellLibrary = NULL ;
  char *pszOldFName = NULL ;
#ifdef WIN32
  char szPath[PATH_LENGTH] = "" ;
  g_snprintf (szPath, PATH_LENGTH, "%s%s..%sshare%s%s%sqca_cell_library", 
    getenv ("MY_PATH"), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, PACKAGE, G_DIR_SEPARATOR_S) ;
  pszCellLibrary = szPath ;
#else
  pszCellLibrary = QCA_CELL_LIBRARY ;
#endif /* def WIN32 */

  pszOldFName = gtk_file_chooser_get_filename (file_chooser) ;

  bHaveCellLib = g_file_test (pszCellLibrary, G_FILE_TEST_IS_DIR) ;

  if (NULL != pszFName)
    if (0 != pszFName[0])
      return strdup (pszFName) ;

  if (g_file_test (pszOldFName, G_FILE_TEST_EXISTS) && !g_file_test (pszOldFName, G_FILE_TEST_IS_DIR))
    return pszOldFName ;

  if (bHaveCellLib)
    return g_strdup_printf ("%s%s", pszCellLibrary, G_DIR_SEPARATOR_S) ;

  if (NULL == pszFName) return NULL ;

  return strdup (pszFName) ;
  }

static void create_file_selection_dialog (file_selection_D *dialog)
  {
  dialog->dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, 
    "modal",           TRUE,
    "select-multiple", FALSE,
    "local-only",      TRUE,
    "show-hidden",     TRUE,
    NULL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  dialog->ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog->dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_OK) ;
  }
