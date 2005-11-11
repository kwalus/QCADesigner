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
#include <string.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "about.h"
#include "print.h"
#include "coherence_vector.h"
#include "bistable_simulation.h"
#include "recent_files.h"
#include "vector_table.h"
#include "qcadstock.h"
#include "global_consts.h"
#include "gtk_preamble.h"
#include "fileio.h"

#define DBG_MAIN(s)

//!Print options
print_design_OP print_options ;


extern main_W main_window ;
extern coherence_OP coherence_options ;


static void parse_cmdline (int argc, char **argv, char **pszFileToOpen, char **pszCoherenceOptionsFile) ;

#ifndef WIN32
  // Can't use WinMain without Win32
  #undef QCAD_NO_CONSOLE
#endif  /* ifndef WIN32 */

#ifdef QCAD_NO_CONSOLE
// Use WinMain and set argc and argv to reasonable values
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
#else /* ifndef QCAD_NO_CONSOLE */
// Normally, we have a console
int main (int argc, char *argv[])
#endif /* def QCAD_NO_CONSOLE */
  {
  GtkWindow *wndAbout = NULL ;
#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
  int argc = 0 ;
  char **argv = NULL ;
#endif
#endif /* ifdef WIN32 */
  char *pszFileToOpen = NULL ;
  char *pszCoherenceOptionsFile = NULL ;
  coherence_OP *co = NULL ;

#ifdef QCAD_NO_CONSOLE
  gtk_preamble (&argc, &argv, "QCADesigner", pszCmdLine) ;
#else
  gtk_preamble (&argc, &argv, "QCADesigner") ;
#endif /* def QCAD_NO_CONSOLE */

  parse_cmdline (argc, argv, &pszFileToOpen, &pszCoherenceOptionsFile) ;
	
	if (NULL != pszCoherenceOptionsFile)
    if (NULL != (co = open_coherence_options_file (pszCoherenceOptionsFile)))
      {
      memcpy (&coherence_options, co, sizeof (coherence_OP)) ;
      g_free (co) ;
      }

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
  if (NULL != pszFileToOpen)
    file_operations ((GtkWidget *)pszFileToOpen, (gpointer)FILEOP_CMDLINE) ;
  else
    file_operations (main_window.main_window, (gpointer)FILEOP_AUTOLOAD) ;
#endif /* def STDIO_FILEIO */
  // -- LET'S GO -- //
  gtk_main ();

  // -- Exit -- //
  return 0;
  }//main

static void parse_cmdline (int argc, char **argv, char **pszFileToOpen, char **pszCoherenceOptionsFile)
  {
  gboolean bDie = FALSE ;
  int Nix ;

  for (Nix = 1 ; Nix < argc && !bDie ; Nix++)
    {
    if (!(strcmp (argv[Nix], "--coherence-opts") && strcmp (argv[Nix], "-c")))
      {
      if (Nix++ < argc)
        (*pszCoherenceOptionsFile) = argv[Nix] ;
      else
        bDie = TRUE ;
      }
    else
    if (!(strcmp (argv[Nix], "--help") && strcmp (argv[Nix], "-h")))
      bDie = TRUE ;
    else
    if (!(strcmp (argv[Nix], "--version") && strcmp (argv[Nix], "-v")))
      {
      printf (PACKAGE " " VERSION "\n") ;
      exit (0) ;
      }
    else
      (*pszFileToOpen) = argv[Nix] ;
    }

  if (bDie)
    {
    printf (
"Usage: QCADesigner [options] [qca_file]\n"
"\n"
"Options are:\n"
"  -c file  --coherence-opts file  Optional: Coherence vector simulation engine options file.\n"
"  -h       --help                 Optional: Print this information and exit.\n"
"  -v       --version              Optional: Print " PACKAGE " version and exit.\n"
"  qca_file                        Optional: QCA circuit file.\n") ;
    exit (0) ;
    }
  }
