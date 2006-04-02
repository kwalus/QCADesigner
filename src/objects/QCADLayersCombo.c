#include "QCADLayersCombo.h"
#include "QCADCellRendererVT.h"
#include "../intl.h"
#include "../design.h"

typedef struct
  {
  GtkWidget *tv_current ;
  GtkWidget *tv ;
  GtkTreeModel *model ;
  DESIGN *design ;
  } QCADLayersComboPrivate ;

#define QCAD_LAYERS_COMBO_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_LAYERS_COMBO, QCADLayersComboPrivate))

enum
  {
  QCAD_LAYERS_COMBO_PROPERTY_FIRST = 1,

  QCAD_LAYERS_COMBO_PROPERTY_DESIGN,
  QCAD_LAYERS_COMBO_PROPERTY_LAYER,

  QCAD_LAYERS_COMBO_PROPERTY_LAST,
  } ;

static void class_init (QCADLayersComboClass *klass) ;

static void instance_init (QCADLayersCombo *instance) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void finalize     (GObject *object) ;

static gboolean layer_list_model_current_layer_p (GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;

static void set_design (QCADLayersCombo *layers_combo, DESIGN *design) ;

GType qcad_layers_combo_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADLayersComboClass),
      NULL,
      NULL,
      (GClassInitFunc)class_init,
      NULL,
      NULL,
      sizeof (QCADLayersCombo),
      0,
      (GInstanceInitFunc)instance_init
      } ;

    if (0 != (the_type = g_type_register_static (QCAD_TYPE_FLEXI_COMBO, QCAD_TYPE_STRING_LAYERS_COMBO, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void class_init (QCADLayersComboClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_LAYERS_COMBO_PROPERTY_DESIGN,
    g_param_spec_pointer ("design", _("Design"), _("Design whose layers to reflect"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_LAYERS_COMBO_PROPERTY_DESIGN,
    g_param_spec_pointer ("layer", _("Layer"), _("Current layer"), G_PARAM_READABLE)) ;

  g_type_class_add_private (klass, sizeof (QCADLayersComboPrivate)) ;
  }

static void instance_init (QCADLayersCombo *instance)
  {
  GtkCellLayout *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (instance) ;
  GtkWidget *frm = NULL ;

  frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (instance), frm) ;
  private->tv_current = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", FALSE, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), private->tv_current) ;
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv_current), GTK_TREE_VIEW_COLUMN (col)) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "text", _("Visible"), NULL) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "text", _("Active"), NULL) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;

  private->tv = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", TRUE, NULL) ;
  g_object_set (G_OBJECT (instance), "popup-widget", private->tv, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv_current), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Layer"), NULL))) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv_current), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Visible"), NULL))) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv_current), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Active"), NULL))) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;

  private->design = NULL ;
  private->model = NULL ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (object) ;

  switch (property_id)
    {
    case QCAD_LAYERS_COMBO_PROPERTY_DESIGN:
      {
      DESIGN *new_design = g_value_get_pointer (value) ;

      if (new_design == private->design) return ;

      set_design (QCAD_LAYERS_COMBO (object), new_design) ;
      g_object_notify (object, "design") ;
      break ;
      }
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (object) ;

  switch (property_id)
    {
    case QCAD_LAYERS_COMBO_PROPERTY_DESIGN:
      g_value_set_pointer (value, private->design) ;
      break ;

    case QCAD_LAYERS_COMBO_PROPERTY_LAYER:
      {
      QCADLayer *layer = NULL ;

      if (NULL != private->design) 
        layer = private->design->lstCurrentLayer->data ;
      g_value_set_object (value, layer) ;
      break ;
      }
    }
  }

static void finalize (GObject *object)
  {G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_LAYERS_COMBO)))->finalize (object) ;}

static void set_design (QCADLayersCombo *layers_combo, DESIGN *design)
  {
  GtkTreeModel *tm_filter = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (layers_combo) ;

  private->model = GTK_TREE_MODEL (design_layer_list_store_new (private->design = design, 0)) ;

  g_object_set (private->tv, "model", private->model, NULL) ;

  tm_filter = g_object_new (GTK_TYPE_TREE_MODEL_FILTER, "child-model", private->model, NULL) ;
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (tm_filter), layer_list_model_current_layer_p, layers_combo, NULL) ;

  gtk_widget_queue_resize (GTK_WIDGET (layers_combo)) ;
  }

static gboolean layer_list_model_current_layer_p (GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
  QCADLayer *layer = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;

  g_print ("layer_list_model_current_layer_p: Design is NULL\n") ;

  if (NULL == private->design) return FALSE ;

  gtk_tree_model_get (tm, itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;

  g_print ("layer_list_model_current_layer_p: Returning %s for layer %s\n",
    (layer == private->design->lstCurrentLayer->data) ? "TRUE" : "FALSE", layer->pszDescription) ;

  return (layer == private->design->lstCurrentLayer->data) ;
  }
