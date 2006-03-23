#include "intl.h"
#include "layers_combo.h"
#include "custom_widgets.h"
#include "objects/QCADCellRendererVT.h"
#include "objects/QCADTreeViewCombo.h"

static void layers_list_set_toggle_data (GtkCellLayout *layout, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;

static void create_columns (LAYERS_COMBO *layers_combo) ;

GtkWidget *layers_combo_new (LAYERS_COMBO *layers_combo)
  {
  layers_combo->cb = g_object_new (QCAD_TYPE_TREE_VIEW_COMBO, "visible", TRUE, "border-width", 5, "text-column", LAYER_MODEL_COLUMN_NAME, NULL) ;
  layers_combo->tv = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", TRUE, NULL) ;
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (layers_combo->tv)), GTK_SELECTION_BROWSE) ;

  gtk_container_add (GTK_CONTAINER (layers_combo->cb), layers_combo->tv) ;

  create_columns (layers_combo) ;

  return layers_combo->cb ;
  }

GtkTreeModel *layers_combo_set_design (LAYERS_COMBO *layers_combo, DESIGN *design)
  {
  GtkTreeModel *tm = GTK_TREE_MODEL (design_layer_list_store_new (design, 0)) ;
  GObject *obj = G_OBJECT (GTK_IS_CELL_LAYOUT (layers_combo->cb) ? layers_combo->cb : layers_combo->tv) ;

  g_object_set_data (G_OBJECT (layers_combo->cb), "design", design) ;
  g_object_set (obj, "model", tm, NULL) ;

  return tm ;
  }

// Select the given layer or, if not found (NULL), the last layer
void layers_combo_select_layer (LAYERS_COMBO *layers_combo, QCADLayer *layer)
  {
  QCADLayer *layer_comp = NULL ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (layers_combo->tv)) ;
  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (layers_combo->tv)) ;
  GtkTreeIter itr, itrLast ;

  if (NULL == tm) return ;
  if (NULL == sel) return ;

  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      itrLast = itr ;
      gtk_tree_model_get (tm, &itr, LAYER_MODEL_COLUMN_LAYER, &layer_comp, -1) ;
      if (layer_comp == layer)
        break ;
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;

  gtk_tree_selection_select_iter (sel, &itr) ;
  }

void layers_combo_remove_layer (LAYERS_COMBO *layers_combo, QCADLayer *layer)
  {
  DESIGN *design = g_object_get_data (G_OBJECT (layers_combo->cb), "design") ;
  QCADLayer *model_layer = NULL ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (layers_combo->tv)) ;
  GtkTreeIter itr ;

  if (NULL != tm)
    if (gtk_tree_model_get_iter_first (tm, &itr))
      do
        {
        gtk_tree_model_get (tm, &itr, LAYER_MODEL_COLUMN_LAYER, &model_layer, -1) ;
        if (layer == model_layer)
          {
          layer = design_layer_remove (design, layer) ;
          gtk_list_store_remove (GTK_LIST_STORE (tm), &itr) ;
          break ;
          }
        }
      while (gtk_tree_model_iter_next (tm, &itr)) ;

  layers_combo_select_layer (layers_combo, layer) ;
  }

static void layers_list_set_toggle_data (GtkCellLayout *layout, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
  // Is this the "Visible" cell renderer, or the "Active" one ?
  QCADLayer *layer = NULL ;
  LAYERS_COMBO *layers_combo = (LAYERS_COMBO *)data ;
  gboolean bVisibleCR = (layers_combo->crVisible == cr) ;
  DESIGN *design = g_object_get_data (G_OBJECT (layers_combo->cb), "design") ;
  gboolean bSensitive ;

  gtk_tree_model_get (tm, itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;

  if (bVisibleCR)
    {
    gboolean bLayerVisible = (QCAD_LAYER_STATUS_VISIBLE == layer->status || QCAD_LAYER_STATUS_ACTIVE == layer->status) ;
    gboolean bMultipleLayers = (design->lstLayers != design->lstLastLayer) ;

    bSensitive = (bMultipleLayers && design_multiple_visible_layers_p (design)) || !bLayerVisible ;

    g_object_set (G_OBJECT (cr),
      "mode",      bSensitive ? GTK_CELL_RENDERER_MODE_ACTIVATABLE : GTK_CELL_RENDERER_MODE_INERT,
      "active",    bLayerVisible,
      "sensitive", bSensitive,
    NULL) ;
    }
  else
    {
    bSensitive = QCAD_LAYER_STATUS_HIDDEN != layer->status ;

    g_object_set (G_OBJECT (cr),
      "mode",      bSensitive ? GTK_CELL_RENDERER_MODE_ACTIVATABLE : GTK_CELL_RENDERER_MODE_INERT,
      "active",    QCAD_LAYER_STATUS_ACTIVE == layer->status,
      "sensitive", bSensitive,
    NULL) ;
    }
  }

static void create_columns (LAYERS_COMBO *layers_combo)
  {
  GtkCellLayout *cell_layout = NULL ;
  GtkCellRenderer *cr = NULL ;

  // The column listing the layer name
  gtk_tree_view_append_column (GTK_TREE_VIEW (layers_combo->tv), 
    GTK_TREE_VIEW_COLUMN (cell_layout = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Layer"), NULL))) ;
  // The layer icon
  gtk_cell_layout_pack_start (cell_layout, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_cell_layout_add_attribute (cell_layout, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  // The layer name
  gtk_cell_layout_pack_start (cell_layout, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (cr, "editable", FALSE, NULL) ;
  gtk_cell_layout_add_attribute (cell_layout, cr, "text", LAYER_MODEL_COLUMN_NAME) ;

  // The column showing whether the layer is visible
  gtk_tree_view_append_column (GTK_TREE_VIEW (layers_combo->tv), 
    GTK_TREE_VIEW_COLUMN (cell_layout = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Visible"), NULL))) ;
#if (GTK_MINOR_VERSION <= 4)
  cr = qcad_cell_renderer_vt_new () ;
  g_object_set (cr, "row-type", ROW_TYPE_CELL, NULL) ;
  // Have to use QCADCellRendererVT, because GtkCellRendererToggle doesn't have "sensitive"
#else
  cr = gtk_cell_renderer_toggle_new () ;
#endif /* (GTK_MINOR_VERSION <= 4) */
  layers_combo->crVisible = cr ;
  gtk_cell_layout_pack_start (cell_layout, cr, TRUE) ;
  gtk_cell_layout_set_cell_data_func (cell_layout, cr, layers_list_set_toggle_data, (gpointer)layers_combo, NULL) ;

  // The column showing whether the layer is visible
  gtk_tree_view_append_column (GTK_TREE_VIEW (layers_combo->tv), 
    GTK_TREE_VIEW_COLUMN (cell_layout = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Active"), NULL))) ;
#if (GTK_MINOR_VERSION <= 4)
  cr = qcad_cell_renderer_vt_new () ;
  g_object_set (cr, "row-type", ROW_TYPE_CELL, NULL) ;
  // Have to use QCADCellRendererVT, because GtkCellRendererToggle doesn't have "sensitive"
#else
  cr = gtk_cell_renderer_toggle_new () ;
#endif /* (GTK_MINOR_VERSION <= 4) */
  layers_combo->crActive = cr ;
  gtk_cell_layout_pack_start (cell_layout, cr, TRUE) ;
  gtk_cell_layout_set_cell_data_func (cell_layout, cr, layers_list_set_toggle_data, (gpointer)layers_combo, NULL) ;
  }
