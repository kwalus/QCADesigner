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
// The file containing main(). It is responsible for    //
// initializing gtk, dealing with the command line,     //
// loading stock items, and creating the main window.   //
//                                                      //
//////////////////////////////////////////////////////////

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
#include "bistable_simulation.h"
#include "recent_files.h"
#include "vector_table.h"
#include "qcadstock.h"
#include "global_consts.h"
#include "gtk_preamble.h"

#define NO_CONSOLE

#define DBG_MAIN(s)

//!Print options
print_design_OP print_options ;

extern main_W main_window ;

#ifndef WIN32
  // Can't use WinMain without Win32
  #undef NO_CONSOLE
#endif  /* ifndef WIN32 */

#ifdef WIN32
#ifdef NO_CONSOLE
static char **CmdLineToArgv (char *pszCmdLine, int *pargc) ;
static void my_logger (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) ;
#endif /* ifdef NO_CONSOLE */
#endif /* ifdef WIN32 */

#ifdef NO_CONSOLE
// Use WinMain and set argc and argv to reasonable values
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
#else /* ifndef NO_CONSOLE */
// Normally, we have a console
int main (int argc, char *argv[])
#endif
  {
  GtkWindow *wndAbout = NULL ;
#ifdef WIN32
  char *pszHomeHDD = getenv ("HOMEDRIVE") ;
  char *pszHomeDIR = getenv ("HOMEPATH") ;
#ifdef NO_CONSOLE
  if (pszCmdLine[0] != 0)
    psz = g_strdup_printf ("%s %s", pszModuleFName, pszCmdLine) ;
  else
    psz = g_strdup_printf ("%s", pszModuleFName) ;
  argv = (char **)CmdLineToArgv (psz, &argc) ;
  g_free (psz) ;
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

  gtk_preamble (&argc, &argv) ;

  wndAbout = show_about_dialog (&(main_window.main_window), TRUE) ;

  // -- Create the main window and the about dialog -- //
  create_main_window (&main_window);

  gtk_window_set_transient_for (wndAbout, GTK_WINDOW (main_window.main_window)) ;

  DBG_MAIN (fprintf (stderr, "Created main window\n")) ;

  // -- Show the main window and the about dialog -- //
  gtk_widget_show (main_window.main_window);

  DBG_MAIN (fprintf (stderr, "Show(ing/n) about dialog\n")) ;
#ifdef STDIO_FILEIO
  // The first command line argument is assumed to be a file name
  if (argc >= 2)
    file_operations ((GtkWidget *)argv[1], (gpointer)FILEOP_CMDLINE) ;
  else
    file_operations (main_window.main_window, (gpointer)FILEOP_AUTOLOAD) ;
#endif /* def STDIO_FILEIO */
  // -- LET'S GO -- //
  gtk_main ();

  // -- Exit -- //
  return 0;
  }//main

#ifdef WIN32
#ifdef NO_CONSOLE
// Turn a string into an argv-style array
char **CmdLineToArgv (char *pszTmp, int *pargc)
  {
  char **argv = NULL, *psz = g_strdup_printf ("%s", pszTmp), *pszAt = psz, *pszStart = psz ;
  gboolean bString = FALSE ;

  (*pargc) = 0 ;

  for (pszAt = psz ; ; pszAt++)
    {
    if (0 == (*pszAt)) break ;
    if (' ' == (*pszAt))
      {
      if (!bString)
        {
        (*pszAt) = 0 ;
        argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
        argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
        pszAt++ ;
        while (' ' == (*pszAt))
          pszAt++ ;
        pszStart = pszAt ;
        }
      }

    if ('\"' == (*pszAt))
      {
      if (!bString)
        pszStart = pszAt = pszAt + 1 ;
      else
        {
        (*pszAt) = 0 ;
        argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
        argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
        pszAt++ ;
        while (' ' == (*pszAt))
          pszAt++ ;
        pszStart = pszAt ;
        }
      bString = !bString ;
      }
    }

  argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
  argv[(*pargc) - 1] = g_strdup_printf ("%s", pszStart) ;
  argv = g_realloc (argv, ++(*pargc) * sizeof (char *)) ;
  argv[(*pargc) - 1] = NULL ;

  (*pargc)-- ;

  g_free (psz) ;
  return argv ;
  }
#endif /* NO_CONSOLE */
#endif /* ifdef WIN32 */

#ifdef NO_CONSOLE
static void my_logger (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
  {
  // Handle log messages here
  // This logger ignores all messages, so as not to produce a console window
  }
#endif /* ifdef NO_CONSOLE */
