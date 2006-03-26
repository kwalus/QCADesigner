#ifdef WIN32
  #include <windows.h>
#endif
#include <stdio.h>
#include <gtk/gtk.h>
#include "intl.h"
#include "fileio.h"
#include "design.h"
#include "interface.h"
#include "preamble.h"
#include "layers_combo.h"

/*
const char *rc_string = ""
"style \"my_combo\""
"{"
"  GtkComboBox::appears-as-list = 1"
"}"
"widget \"*.my_combo\" style \"my_combo\"";
 
gtk_widget_set_name (combo, "my_combo");
gtk_rc_parse_string (rc_string);
*/

#ifdef QCAD_NO_CONSOLE
// Use WinMain and set argc and argv to reasonable values
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int iCmdShow)
#else /* ifndef QCAD_NO_CONSOLE */
// Normally, we have a console
int main (int argc, char *argv[])
#endif /* def QCAD_NO_CONSOLE */
  {
#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
  int argc = 0 ;
  char **argv = NULL ;
#endif
#endif /* ifdef WIN32 */
  DESIGN *design = NULL ;
  GtkWidget *wnd = NULL, *cb = NULL ;
  LAYERS_COMBO layers_combo = {NULL} ;

#ifdef QCAD_NO_CONSOLE
  gtk_preamble (&argc, &argv, NULL, pszCmdLine) ;
#else
  gtk_preamble (&argc, &argv, NULL) ;
#endif /* def QCAD_NO_CONSOLE */

  if (argc <= 1)
    {
    g_print ("Usage: %s filename\n", argv[0]) ;
    return 1 ;
    }

  wnd = g_object_new (GTK_TYPE_WINDOW, "title", "Combo Test", NULL) ;

  cb = layers_combo_new (&layers_combo) ;
  gtk_container_add (GTK_CONTAINER (wnd), cb) ;

  if (!open_project_file (argv[1], &design))
    {
    g_print ("Failed to open \"%s\"\n", argv[1]) ;
    return 2 ;
    }

  layers_combo_set_design (&layers_combo, design) ;
  layers_combo_select_layer (&layers_combo, NULL) ;

  g_signal_connect (G_OBJECT (wnd), "delete-event", (GCallback)gtk_main_quit, NULL) ;

  gtk_widget_show (wnd) ;

  gtk_main () ;

  return 0 ;
  }
