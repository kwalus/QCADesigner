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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif /* ifdef WIN32 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "about.h"
#include "print.h"
#include "stdqcell.h"
#include "nonlinear_approx.h"
#include "bistable_simulation.h"
#include "recent_files.h"
#include "vector_table.h"
#include "init.h"

#define NO_CONSOLE

#define DBG_MAIN(s)

//!Print options
print_design_OP print_options ;

//!Options for the cells
cell_OP cell_options = {0, 18, 18, 5, 9, 9, 2} ;

extern main_W main_window ;

static void QCADesigner_static_init () ;

#ifndef WIN32
  // Can't use WinMain without Win32
  #undef NO_CONSOLE
#endif  /* ifndef WIN32 */

#ifdef NO_CONSOLE
// Use WinMain and set argc and argv to reasonable values
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
  {
  GError *err = NULL ;
#else /* ifndef NO_CONSOLE */
// Normally, we have a console
int main (int argc, char *argv[])
  {
#endif /* ifdef NO_CONSOLE */
  // Our windows will get icons from this list
  GList *icon_list = NULL ;
  gchar *pszIconFile = NULL ;

#ifdef WIN32
  char *psz = NULL, *pszModuleFName = NULL, szBuf[MAX_PATH] = "" ;
  int Nix ;
  // Need this buffer later on for the pixmap dirs
  char szMyPath[PATH_LENGTH] = "" ;

  char *pszHomeHDD = getenv ("HOMEDRIVE") ;
  char *pszHomeDIR = getenv ("HOMEPATH") ;
#endif
#ifdef NO_CONSOLE
  // If we don't have a console, we need to create argv and argc
  char **argv = NULL ;
  int argc = 0 ;
#endif
#ifdef WIN32
  GetModuleFileName (NULL, szBuf, MAX_PATH) ;
  pszModuleFName = g_strdup_printf ("%s", szBuf) ;
  GetShortPathName (pszModuleFName, szBuf, MAX_PATH) ;
  g_free (pszModuleFName) ;
  pszModuleFName = g_strdup_printf ("%s", szBuf) ;
#endif
#ifdef NO_CONSOLE
  if (pszCmdLine[0] != 0)
    psz = g_strdup_printf ("%s %s", pszModuleFName, pszCmdLine) ;
  else
    psz = g_strdup_printf ("%s", pszModuleFName) ;
  if (!g_shell_parse_argv (psz, &argc, &argv, &err))
    exit (1) ;
  g_free (psz) ;
#endif

#ifdef WIN32
#ifndef NO_CONSOLE
  fprintf (stderr, "Running in Win32 in a console\n") ;
#endif
  // Must set the home directory to a reasonable value.  If all else fails,
  // set it to the current directory
  if (!(NULL == pszHomeHDD || NULL == pszHomeDIR))
    {
    putenv (psz = g_strdup_printf ("HOME=%s%s", pszHomeHDD, pszHomeDIR)) ;
    g_free (psz) ;
    }
  else
    putenv ("HOME=.") ;
#endif /* ifdef WIN32 */

	// -- GTKWidgets -- //
	
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

	gtk_set_locale ();
	gtk_init (&argc, &argv);

#ifdef NO_CONSOLE
  g_strfreev (argv) ;
#endif

  // call all the static initializers from init.h
  QCADesigner_static_init () ;

#ifdef WIN32
  g_snprintf (szMyPath, MAX_PATH, "%s", pszModuleFName) ;

  /* After the following line, make no more references to pszModuleFName ! */
  g_free (pszModuleFName) ;

  for (Nix = strlen (szMyPath) ; Nix > -1 ; Nix--)
    if (G_DIR_SEPARATOR == szMyPath[Nix])
      {
      szMyPath[Nix] = 0 ;
      break ;
      }

  psz = g_strdup_printf ("MY_PATH=%s", szMyPath) ;
  putenv (psz) ;
  g_free (psz) ;

  psz = g_strdup_printf ("%s%spixmaps", szMyPath, G_DIR_SEPARATOR_S) ;
  add_pixmap_directory (psz) ;
  g_free (psz) ;
  psz = g_strdup_printf ("%s%s..%sshare%spixmaps%sQCADesigner", szMyPath,
    G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S) ;
  add_pixmap_directory (psz) ;
  g_free (psz) ;
#else /* ifndef WIN32 */
  // -- Pixmaps used by the buttons in the main window -- //
	add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps/QCADesigner");
	add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
#endif /* ifdef WIN32 */           

// Can't do SVG icons yet.  gtk+-2.0 doesn't support them.  This means that
// QCADesigner won't work on RH 8.0
// Uncomment below if-wrapper when safe.

//  pszIconFile = find_pixmap_file ("QCADesigner.svg") ;
//  DBG_MAIN (fprintf (stderr, "Found SVG icon at %s\n", pszIconFile)) ;
//  if (!gtk_window_set_default_icon_from_file (pszIconFile, NULL))
//    {
    DBG_MAIN (fprintf (stderr, "Failed to set icon using SVG\n")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_8_16x16x8.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_7_32x32x8.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_6_48x48x8.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_5_16x16x24.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_4_32x32x24.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_3_48x48x24.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_2_16x16x24a.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_1_32x32x24a.png")) ;
    icon_list = g_list_append (icon_list, create_pixbuf ("QCADesigner_0_48x48x24a.png")) ;
    gtk_window_set_default_icon_list (icon_list) ;
//    }
  
  g_free (pszIconFile) ;
  g_list_free (icon_list) ;

	// -- Create the main window and the about dialog -- //
	create_main_window (&main_window);

  DBG_MAIN (fprintf (stderr, "Created main window\n")) ;
	
	// -- Show the main window and the about dialog -- //
	gtk_widget_show (main_window.main_window);
	
	show_about_dialog (GTK_WINDOW (main_window.main_window), TRUE) ;

  DBG_MAIN (fprintf (stderr, "Show(ing/n) about dialog\n")) ;

  /* The first command line argument is assumed to be a file name */
  if (argc >= 2)
    file_operations ((GtkWidget *)argv[1], (gpointer)FILEOP_CMDLINE) ;

	// -- LET'S GO -- //
	gtk_main ();
	
	// -- Exit -- //
  return 0;

  }//main

// call the various <module_name>_init () ; functions
static void QCADesigner_static_init ()
  {
  cad_init () ;
  }
