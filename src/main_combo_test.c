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
#include "custom_widgets.h"
#include "objects/QCADLayersCombo.h"

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
  GtkWidget *wnd = NULL, *cb = NULL, *box = NULL, *gtk_cb = NULL, *tv = NULL, *frm = NULL ;
  GtkListStore *ls = NULL ;
  GtkTreeIter itr, itrNext ;
  GtkCellLayout *layout = NULL ;
  GtkCellRenderer *cr = NULL ;

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

  box = gtk_vbox_new (FALSE, 2) ;
  gtk_widget_show (box) ;
  gtk_container_add (GTK_CONTAINER (wnd), box) ;

  cb = g_object_new (QCAD_TYPE_LAYERS_COMBO, "visible", TRUE, "border-width", 0, NULL) ;
  gtk_box_pack_start (GTK_BOX (box), cb, TRUE, TRUE, 2) ;

  gtk_cb = g_object_new (GTK_TYPE_COMBO_BOX_ENTRY, "visible", TRUE, "border-width", 5, NULL) ;
  gtk_box_pack_start (GTK_BOX (box), gtk_cb, TRUE, TRUE, 2) ;

  tv = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", FALSE, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), GTK_TREE_VIEW_COLUMN (layout = GTK_CELL_LAYOUT (gtk_tree_view_column_new ()))) ;
  cr = gtk_cell_renderer_pixbuf_new () ;
  gtk_cell_layout_pack_start (layout, cr, FALSE) ;
  gtk_cell_layout_add_attribute (layout, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  cr = gtk_cell_renderer_text_new () ;
  gtk_cell_layout_pack_start (layout, cr, FALSE) ;
  gtk_cell_layout_add_attribute (layout, cr, "text", LAYER_MODEL_COLUMN_NAME) ;

  frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), tv) ;

  gtk_container_add (GTK_CONTAINER (gtk_cb), frm) ;

  if (!open_project_file (argv[1], &design))
    {
    g_print ("Failed to open \"%s\"\n", argv[1]) ;
    return 2 ;
    }

  ls = design_layer_list_store_new (design, 0) ;
  while (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ls), &itr))
    {
    itrNext = itr ;
    if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (ls), &itrNext))
      break ;
    else
      gtk_list_store_remove (ls, &itr) ;
    }
  

  g_object_set (G_OBJECT (cb), "design", design, NULL) ;
  g_object_set (G_OBJECT (tv), "model", ls, NULL) ;

  g_signal_connect_swapped (G_OBJECT (cb), "notify::layer", (GCallback)g_print, "QCADLayersCombo::notify::layer\n") ;

  g_signal_connect (G_OBJECT (wnd), "delete-event", (GCallback)gtk_main_quit, NULL) ;

  gtk_widget_show (wnd) ;

  gtk_main () ;

  return 0 ;
  }
