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
// GTK preamble. This is where we call gtk_init,        //
// register all the stock items, add the pixmap         //
// directories.                                         //
//                                                      //
//////////////////////////////////////////////////////////

#ifdef GTK_GUI

#include "support.h"
#include "gtk_preamble.h"
#include "qcadstock.h"

void gtk_preamble (int *pargc, char ***pargv)
  {
  // Our windows will get icons from this list
  GList *icon_list = NULL ;
#ifdef HAVE_LIBRSVG
  gchar *pszIconFile = NULL ;
#endif

#ifdef WIN32
  char *psz = NULL, *pszModuleFName = NULL, szBuf[MAX_PATH] = "" ;
  int Nix ;
  // Need this buffer later on for the pixmap dirs
  char szMyPath[PATH_LENGTH] = "" ;

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

  gtk_init (pargc, pargv) ;

// Add pixmap directories
#ifdef WIN32
  g_snprintf (szMyPath, MAX_PATH, "%s", pszModuleFName) ;

  // After the following line, make no more references to pszModuleFName !
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
  psz = g_strdup_printf ("%s%s..%sshare%s%s%spixmaps", szMyPath, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, PACKAGE, G_DIR_SEPARATOR_S) ;
  add_pixmap_directory (psz) ;
  g_free (psz) ;
  psz = g_strdup_printf ("%s\\..\\pixmaps", szMyPath) ;
  add_pixmap_directory (psz) ;
  g_free (psz) ;
#else /* ifndef WIN32 */
  // -- Pixmaps used by the buttons in the main window -- //
  add_pixmap_directory (PACKAGE_DATA_DIR "/QCADesigner/pixmaps");
  add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
#endif /* ifdef WIN32 */
// Done adding pixmap directories

#ifdef WIN32
#ifdef NO_CONSOLE
  // Turn off logging by setting it to an empty function
  // This prevents a console from popping up when QCADesigner quits
  g_log_set_handler (NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Gdk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("GdkPixbuf", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Pango", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
#endif /* ifdef NO_CONSOLE */
#endif /* ifdef WIN32 */

#ifdef HAVE_LIBRSVG
  pszIconFile = find_pixmap_file ("QCADesigner.svg") ;
  if (!gtk_window_set_default_icon_from_file (pszIconFile, NULL))
    {
#endif
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
    g_list_free (icon_list) ;
#ifdef HAVE_LIBRSVG
    }

  g_free (pszIconFile) ;
#endif
  add_stock_icon ("drawing_layer.png",      QCAD_STOCK_DRAWING_LAYER) ;
  add_stock_icon ("clocks_layer.png",       QCAD_STOCK_CLOCKING_LAYER) ;
  add_stock_icon ("cells_layer.png",        QCAD_STOCK_CELL_LAYER) ;
  add_stock_icon ("substrate_layer.png",    QCAD_STOCK_SUBSTRATE_LAYER) ;
  add_stock_icon ("label.png",              QCAD_STOCK_LABEL) ;
  add_stock_icon ("substrate.png",          QCAD_STOCK_SUBSTRATE) ;
#ifdef HAVE_LIBRSVG
  add_stock_icon ("reorder_layers.svg",     QCAD_STOCK_REORDER_LAYERS) ;
  add_stock_icon ("bus.svg",                QCAD_STOCK_BUS) ;
  add_stock_icon ("clock.svg",              QCAD_STOCK_CLOCK) ;
  add_stock_icon ("bus_input.svg",          QCAD_STOCK_BUS_INPUT) ;
  add_stock_icon ("bus_output.svg",         QCAD_STOCK_BUS_OUTPUT) ;
  add_stock_icon ("cell_input.svg",         QCAD_STOCK_CELL_INPUT) ;
  add_stock_icon ("cell_output.svg",        QCAD_STOCK_CELL_OUTPUT) ;
  add_stock_icon ("graph_data_bin.svg",     QCAD_STOCK_GRAPH_BIN) ;
  add_stock_icon ("graph_data_hex.svg",     QCAD_STOCK_GRAPH_HEX) ;
  add_stock_icon ("graph_data_dec.svg",     QCAD_STOCK_GRAPH_DEC) ;
  add_stock_icon ("puzzle_piece_green.svg", QCAD_STOCK_BLOCK_READ) ;
  add_stock_icon ("puzzle_piece_red.svg",   QCAD_STOCK_BLOCK_WRITE) ;
  add_stock_icon ("q_cell_def.svg",         QCAD_STOCK_CELL) ;
  add_stock_icon ("q_cell_array.svg",       QCAD_STOCK_ARRAY) ;
  add_stock_icon ("q_cell_rotate.svg",      QCAD_STOCK_ROTATE_CELL) ;
  add_stock_icon ("default.svg",            QCAD_STOCK_SELECT) ;
  add_stock_icon ("q_cell_alt.svg",         QCAD_STOCK_CELL_ALT_CROSSOVER) ;
  add_stock_icon ("q_cell_alt_circle.svg",  QCAD_STOCK_CELL_ALT_VERTICAL) ;
  add_stock_icon ("q_cell_copy.svg",        QCAD_STOCK_COPY) ;
  add_stock_icon ("q_cell_translate.svg",   QCAD_STOCK_TRANSLATE) ;
  add_stock_icon ("q_cell_mirror.svg",      QCAD_STOCK_MIRROR_VERTICAL) ;
  add_stock_icon ("q_cell_mirror_other.svg",QCAD_STOCK_MIRROR_HORIZONTAL) ;
  add_stock_icon ("q_cell_pan.svg",         QCAD_STOCK_PAN) ;
  add_stock_icon ("ruler.svg",              QCAD_STOCK_MEASURE) ;
#else
  add_stock_icon ("reorder_layers.png",     QCAD_STOCK_REORDER_LAYERS) ;
  add_stock_icon ("bus.png",                QCAD_STOCK_BUS) ;
  add_stock_icon ("clock.png",              QCAD_STOCK_CLOCK) ;
  add_stock_icon ("bus_input.png",          QCAD_STOCK_BUS_INPUT) ;
  add_stock_icon ("bus_output.png",         QCAD_STOCK_BUS_OUTPUT) ;
  add_stock_icon ("cell_input.png",         QCAD_STOCK_CELL_INPUT) ;
  add_stock_icon ("cell_output.png",        QCAD_STOCK_CELL_OUTPUT) ;
  add_stock_icon ("graph_data_bin.png",     QCAD_STOCK_GRAPH_BIN) ;
  add_stock_icon ("graph_data_hex.png",     QCAD_STOCK_GRAPH_HEX) ;
  add_stock_icon ("graph_data_dec.png",     QCAD_STOCK_GRAPH_DEC) ;
  add_stock_icon ("puzzle_piece_green.png", QCAD_STOCK_BLOCK_READ) ;
  add_stock_icon ("puzzle_piece_red.png",   QCAD_STOCK_BLOCK_WRITE) ;
  add_stock_icon ("q_cell_def.png",         QCAD_STOCK_CELL) ;
  add_stock_icon ("q_cell_array.png",       QCAD_STOCK_ARRAY) ;
  add_stock_icon ("q_cell_rotate.png",      QCAD_STOCK_ROTATE_CELL) ;
  add_stock_icon ("default.png",            QCAD_STOCK_SELECT) ;
  add_stock_icon ("q_cell_alt.png",         QCAD_STOCK_CELL_ALT_CROSSOVER) ;
  add_stock_icon ("q_cell_alt_circle.png",  QCAD_STOCK_CELL_ALT_VERTICAL) ;
  add_stock_icon ("q_cell_move.png",        QCAD_STOCK_COPY) ;
  add_stock_icon ("q_cell_move.png",        QCAD_STOCK_TRANSLATE) ;
  add_stock_icon ("q_cell_mirror.png",      QCAD_STOCK_MIRROR_VERTICAL) ;
  add_stock_icon ("q_cell_mirror_other.png",QCAD_STOCK_MIRROR_HORIZONTAL) ;
  add_stock_icon ("q_cell_pan.png",         QCAD_STOCK_PAN) ;
  add_stock_icon ("ruler.png",              QCAD_STOCK_MEASURE) ;
#endif
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  gtk_set_locale ();
  }

#endif /* def GTK_GUI */
