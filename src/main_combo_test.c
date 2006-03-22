#include <stdio.h>
#include <gtk/gtk.h>
#include "intl.h"
#include "fileio.h"
#include "design.h"
#include "interface.h"
#include "gtk_preamble.h"
#include "objects/QCADTreeViewCombo.h"

GtkWidget *create_layers_treeview () ;

int main (int argc, char **argv)
  {
  DESIGN *design = NULL ;
  GtkWidget *wnd = NULL, *cb = NULL, *tv = NULL ;
  GtkTreeModel *tm = NULL ;

  gtk_preamble (&argc, &argv, NULL) ;

  if (argc <= 1)
    {
    g_print ("Usage: %s filename\n", argv[0]) ;
    return 1 ;
    }

  wnd = g_object_new (GTK_TYPE_WINDOW, "title", QCAD_TYPE_STRING_TREE_VIEW_COMBO " Test", NULL) ;

  cb = g_object_new (QCAD_TYPE_TREE_VIEW_COMBO, "visible", TRUE, NULL) ;
  gtk_container_add (GTK_CONTAINER (wnd), cb) ;

  tv = create_layers_treeview () ;
  gtk_widget_show (tv) ;
  gtk_container_add (GTK_CONTAINER (cb), tv) ;

  if (!open_project_file (argv[1], &design))
    {
    g_print ("Failed to open \"%s\"\n", argv[1]) ;
    return 2 ;
    }

  if (NULL != (tm = GTK_TREE_MODEL (design_layer_list_store_new (design, 0))))
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv), tm) ;

  g_signal_connect (G_OBJECT (wnd), "delete-event", (GCallback)gtk_main_quit, NULL) ;

  gtk_widget_show (wnd) ;

  gtk_main () ;

  return 0 ;
  }

GtkWidget *create_layers_treeview ()
  {
  GtkTreeView *tv = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", TRUE, NULL) ;
  GtkTreeSelection *sel = gtk_tree_view_get_selection (tv) ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;

  if (NULL != sel)
    gtk_tree_selection_set_mode (sel, GTK_SELECTION_BROWSE) ;

  // The column listing the layer name
  gtk_tree_view_append_column (tv, col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Layer"), NULL)) ;
  // The layer icon
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_tree_view_column_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  // The layer name
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (cr, "editable", FALSE, NULL) ;
  gtk_tree_view_column_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;

  // The column showing whether the layer is visible
  gtk_tree_view_append_column (tv, col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Visible"), NULL)) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  g_object_set (cr, "xalign", 0.5, NULL) ;

  // The column showing whether the layer is visible
  gtk_tree_view_append_column (tv, col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Active"), NULL)) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  g_object_set (cr, "xalign", 0.5, NULL) ;

  return GTK_WIDGET (tv) ;
  }
