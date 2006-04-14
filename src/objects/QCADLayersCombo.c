#include "QCADLayersCombo.h"
#include "QCADCellRendererVT.h"
#include "../intl.h"
#include "../design.h"
#include "../custom_widgets.h"

/**
 * SECTION:QCADLayersCombo
 * @short_description: QCADesigner layers combo box
 *
 * The layers combo takes a design and renders it as a list of layers in a dropdown. It maintains the design's
 * current layer in its "layer" property and provides checkboxes for (de)activating and hiding/showing a layer.
 * When either of those checkboxes is deactivated, the "deactivate-layer" signal is emitted and the layer is
 * only deactivated/hidden if those signals return %TRUE.
 *
 */

typedef struct
  {
  GtkWidget *tv_current ;
  GtkWidget *tv ;
  GtkTreeModel *model ;
  GtkTreeModel *tm_filter ;
  GtkCellRenderer *crVisible ;
  GtkCellRenderer *crActive ;
  DESIGN *design ;
  } QCADLayersComboPrivate ;

#define QCAD_LAYERS_COMBO_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_LAYERS_COMBO, QCADLayersComboPrivate))

G_BEGIN_DECLS

/* BOOLEAN:OBJECT,BOOLEAN (marshal:1) */
extern void g_cclosure_user_marshal_BOOLEAN__OBJECT_BOOLEAN (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

G_END_DECLS

enum
  {
  LAYERS_COMBO_MODEL_COLUMN_WAS_ACTIVE = LAYER_MODEL_COLUMN_LAST,
  LAYERS_COMBO_MODEL_COLUMN_LAST
  } ;

enum
  {
  QCAD_LAYERS_COMBO_SIGNAL_DEACTIVATE_LAYER,
  QCAD_LAYERS_COMBO_LAST_SIGNAL
  } ;

enum
  {
  QCAD_LAYERS_COMBO_PROPERTY_FIRST = 1,

  QCAD_LAYERS_COMBO_PROPERTY_DESIGN,
  QCAD_LAYERS_COMBO_PROPERTY_LAYER,

  QCAD_LAYERS_COMBO_PROPERTY_LAST,
  } ;

static guint layers_combo_signals[QCAD_LAYERS_COMBO_LAST_SIGNAL] = {0} ;

static void class_init (QCADLayersComboClass *klass) ;

static void instance_init (QCADLayersCombo *instance) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void finalize     (GObject *object) ;

static gboolean deactivate_layer (QCADLayersCombo *layers_combo, QCADLayer *layer, gpointer data) ;
static gboolean layer_list_model_current_layer_p (GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;
static void layers_list_set_toggle_data (GtkCellLayout *col, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;
static void layers_selection_changed (GtkTreeSelection *sel, gpointer data) ;
static gboolean tree_view_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static void layer_list_state_toggled (GtkCellRenderer *cr, char *pszPath, gpointer data) ;

static void set_design (QCADLayersCombo *layers_combo, DESIGN *design) ;
static void set_layer (QCADLayersCombo *layers_combo, QCADLayer *layer) ;

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

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_LAYERS_COMBO_PROPERTY_LAYER,
    g_param_spec_object ("layer", _("Layer"), _("Current layer"), 
      QCAD_TYPE_LAYER, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  QCAD_LAYERS_COMBO_CLASS (klass)->deactivate_layer = deactivate_layer ;

  layers_combo_signals[QCAD_LAYERS_COMBO_SIGNAL_DEACTIVATE_LAYER] = 
    g_signal_new ("deactivate-layer",
      G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (QCADLayersComboClass, deactivate_layer),
      g_signal_accumulator_true_handled, NULL,
      g_cclosure_user_marshal_BOOLEAN__OBJECT_BOOLEAN,
      G_TYPE_BOOLEAN, 2, QCAD_TYPE_LAYER, G_TYPE_BOOLEAN) ;

  g_type_class_add_private (klass, sizeof (QCADLayersComboPrivate)) ;
  }

static void instance_init (QCADLayersCombo *instance)
  {
  GtkCellLayout *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (instance) ;
  GtkWidget *frm = NULL, *frm1 ;

  frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (instance), frm) ;
//  gtk_container_add (GTK_CONTAINER (instance), g_object_new (GTK_TYPE_ENTRY, "visible", TRUE, NULL)) ;
  private->tv_current = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", FALSE, NULL) ;
  GTK_WIDGET_UNSET_FLAGS (private->tv_current, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (private->tv_current)), GTK_SELECTION_NONE) ;
  gtk_container_add (GTK_CONTAINER (frm), private->tv_current) ;
//  gtk_container_add (GTK_CONTAINER (instance), private->tv_current) ;
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv_current), GTK_TREE_VIEW_COLUMN (col)) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;
/*gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "text", _("Visible"), NULL) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "text", _("Active"), NULL) ;
  gtk_cell_layout_pack_start (col, cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
*/
  frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_OUT, NULL) ;
  frm1 = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), frm1) ;
  private->tv = g_object_new (GTK_TYPE_TREE_VIEW, "visible", TRUE, "headers-visible", TRUE, NULL) ;
  g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (private->tv))), "changed", (GCallback)layers_selection_changed, instance) ;
  g_signal_connect (G_OBJECT (private->tv), "button-release-event", (GCallback)tree_view_button_release, instance) ;
  gtk_container_add (GTK_CONTAINER (frm1), private->tv) ;
  g_object_set (G_OBJECT (instance), "popup-widget", frm, NULL) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Layer"), NULL))) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_pixbuf_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "stock-id", LAYER_MODEL_COLUMN_ICON) ;
  gtk_cell_layout_pack_start (col, cr = gtk_cell_renderer_text_new (), FALSE) ;
  gtk_cell_layout_add_attribute (col, cr, "text", LAYER_MODEL_COLUMN_NAME) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Visible"), NULL))) ;
  gtk_cell_layout_pack_start (col, private->crVisible = cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
  gtk_cell_layout_set_cell_data_func (col, cr, layers_list_set_toggle_data, instance, NULL) ;
  g_signal_connect (G_OBJECT (cr), "toggled",  (GCallback)layer_list_state_toggled, instance) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tv), 
    GTK_TREE_VIEW_COLUMN (col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Active"), NULL))) ;
  gtk_cell_layout_pack_start (col, private->crActive = cr = qcad_cell_renderer_vt_new (), FALSE) ;
  g_object_set (G_OBJECT (cr), "row-type", ROW_TYPE_CELL, NULL) ;
  gtk_cell_layout_set_cell_data_func (col, cr, layers_list_set_toggle_data, instance, NULL) ;
  g_signal_connect (G_OBJECT (cr), "toggled",  (GCallback)layer_list_state_toggled, instance) ;

  private->design = NULL ;
  private->model = NULL ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_LAYERS_COMBO_PROPERTY_DESIGN:
      set_design (QCAD_LAYERS_COMBO (object), g_value_get_pointer (value)) ;
      break ;

    case QCAD_LAYERS_COMBO_PROPERTY_LAYER:
      set_layer (QCAD_LAYERS_COMBO (object), g_value_get_object (value)) ;
      break ;
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

static void layers_list_set_toggle_data (GtkCellLayout *col, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
//  QCADLayersCombo *layers_combo = QCAD_LAYERS_COMBO (data) ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;
  QCADLayer *layer = NULL ;
  gboolean bVisibleCR = (private->crVisible == cr), bSensitive = FALSE ;

  if (NULL == private->design) return ;

  gtk_tree_model_get (tm, itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;

  if (bVisibleCR)
    {
    gboolean bLayerVisible = (QCAD_LAYER_STATUS_VISIBLE == layer->status || QCAD_LAYER_STATUS_ACTIVE == layer->status) ;
    gboolean bMultipleLayers = (private->design->lstLayers != private->design->lstLastLayer) ;

    bSensitive = (bMultipleLayers && design_multiple_visible_layers_p (private->design)) || !bLayerVisible ;

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

static void layers_selection_changed (GtkTreeSelection *sel, gpointer data)
  {
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;
  GtkTreeIter itr ;
  GList *llSel = gtk_tree_selection_get_selected_rows (sel, NULL) ;
  QCADLayer *layer = NULL ;

//  g_print ("layers_selection_changed:Entering\n") ;

  if (NULL == llSel) return ;
  if (NULL != private->model)
    if (gtk_tree_model_get_iter (private->model, &itr, (GtkTreePath *)(llSel->data)))
      {
      gtk_tree_model_get (private->model, &itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;
      if (layer != private->design->lstCurrentLayer->data)
        {
//        g_print ("layers_selection_changed:Setting current layer: %s\n", layer->pszDescription) ;
        design_set_current_layer (private->design, layer) ;
        g_object_notify (G_OBJECT (data), "layer") ;
        if (NULL != private->tm_filter)
          gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (private->tm_filter)) ;
        }
      }

  g_list_foreach (llSel, (GFunc)gtk_tree_path_free, NULL) ;
  g_list_free (llSel) ;
  }

static gboolean tree_view_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GtkCellRendererMode mode = 0 ;
  GtkCellRenderer *cr = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;
  GtkTreePath *tp = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkTreeIter itr ;

  if (NULL == private->model) return FALSE ;
  if (NULL == (cr = gtk_tree_view_get_cell_renderer_at_point (GTK_TREE_VIEW (widget), event->x, event->y, &tp, &col))) return FALSE ;
  if (NULL != tp)
    {
    if (gtk_tree_model_get_iter (private->model, &itr, tp))
      if (NULL != col)
        gtk_tree_view_column_cell_set_cell_data (col, private->model, &itr, FALSE, FALSE) ;
    gtk_tree_path_free (tp) ;
    }

  g_object_get (G_OBJECT (cr), "mode", &mode, NULL) ;
  if (GTK_CELL_RENDERER_MODE_INERT == mode)
    {
    qcad_flexi_combo_show_popup (QCAD_FLEXI_COMBO (data), FALSE) ;
    return TRUE ;
    }

  return FALSE ;
  }

static gboolean deactivate_layer (QCADLayersCombo *layers_combo, QCADLayer *layer, gpointer data) 
  {return FALSE ;}

static void layer_list_state_toggled (GtkCellRenderer *cr, char *pszPath, gpointer data)
  {
  QCADLayer *layer = NULL ;
  gboolean may_deactivate_layer = FALSE ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;
  GtkTreeIter itr ;
  gboolean bVisibleCR = (private->crVisible == cr) ;

  if (NULL == private->model) return ;
  if (!gtk_tree_model_get_iter_from_string (private->model, &itr, pszPath)) return ;
  gtk_tree_model_get (private->model, &itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;
  if (NULL == layer) return ;

  g_signal_emit (data, layers_combo_signals[QCAD_LAYERS_COMBO_SIGNAL_DEACTIVATE_LAYER], 0, layer, bVisibleCR, &may_deactivate_layer) ;
  if (!may_deactivate_layer) return ;

  if (bVisibleCR)
    {
    if (QCAD_LAYER_STATUS_HIDDEN == layer->status)
      {
      gboolean bWasActive = FALSE ;

      gtk_tree_model_get (private->model, &itr, LAYERS_COMBO_MODEL_COLUMN_WAS_ACTIVE, &bWasActive, -1) ;

      layer->status = bWasActive ? QCAD_LAYER_STATUS_ACTIVE : QCAD_LAYER_STATUS_VISIBLE ;
      }
    else
    if (design_multiple_visible_layers_p (private->design))
      {
      gtk_list_store_set (GTK_LIST_STORE (private->model), &itr, LAYERS_COMBO_MODEL_COLUMN_WAS_ACTIVE, (QCAD_LAYER_STATUS_ACTIVE == layer->status), -1) ;
      layer->status = QCAD_LAYER_STATUS_HIDDEN ;
      }
    }
  else
    layer->status = 
      QCAD_LAYER_STATUS_ACTIVE == layer->status ? QCAD_LAYER_STATUS_VISIBLE : QCAD_LAYER_STATUS_ACTIVE ;

  gtk_widget_queue_draw (private->tv) ;

  g_object_notify (G_OBJECT (data), "layer") ;
  }

static gboolean layer_list_model_current_layer_p (GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
  QCADLayer *layer = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (data) ;

//  g_print ("layer_list_model_current_layer_p: Design is NULL\n") ;

  if (NULL == private->design) return FALSE ;

  gtk_tree_model_get (tm, itr, LAYER_MODEL_COLUMN_LAYER, &layer, -1) ;

//  g_print ("layer_list_model_current_layer_p: Returning %s for layer %s\n",
//    (layer == private->design->lstCurrentLayer->data) ? "TRUE" : "FALSE", layer->pszDescription) ;

  return (layer == private->design->lstCurrentLayer->data) ;
  }

static void set_design (QCADLayersCombo *layers_combo, DESIGN *design)
  {
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (layers_combo) ;

  g_object_set (private->tv, "model", NULL, NULL) ;
  g_object_set (private->tv_current, "model", NULL, NULL) ;
  private->model = NULL ;

  if (NULL != design)
    {
    private->model = GTK_TREE_MODEL (design_layer_list_store_new (private->design = design, 1, G_TYPE_BOOLEAN)) ;

    g_object_set (private->tv, "model", private->model, NULL) ;

    private->tm_filter = g_object_new (GTK_TYPE_TREE_MODEL_FILTER, "child-model", private->model, NULL) ;
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (private->tm_filter), layer_list_model_current_layer_p, layers_combo, NULL) ;

    g_object_set (G_OBJECT (private->tv_current), "model", private->tm_filter, NULL) ;
    }

  g_object_notify (G_OBJECT (layers_combo), "design") ;

  if (NULL != design)
    set_layer (layers_combo, private->design->lstCurrentLayer->data) ;
//  g_print ("QCADLayersCombo::set_design: Queueing resize\n") ;

  gtk_widget_queue_resize (GTK_WIDGET (layers_combo)) ;
  }

static void set_layer (QCADLayersCombo *layers_combo, QCADLayer *layer)
  {
  QCADLayer *model_layer = NULL ;
  QCADLayersComboPrivate *private = QCAD_LAYERS_COMBO_GET_PRIVATE (layers_combo) ;
  GtkTreeIter itr ;
  // If the layer we're selecting is already selected, then issuing gtk_tree_selection_select_iter and its
  // subsequent invocation of the layers_selection_changed callback will not result in a notify
  gboolean bNotify = FALSE ;

  if (NULL == private->design) return ;

  bNotify = (layer == private->design->lstCurrentLayer->data) ;

  if (gtk_tree_model_get_iter_first (private->model, &itr))
    do
      {
      gtk_tree_model_get (private->model, &itr, LAYER_MODEL_COLUMN_LAYER, &model_layer, -1) ;
//      g_print ("QCADLayersCombo::set_layer:Deciding model_layer = %s vs. layer->data = %s\n",
//        model_layer->pszDescription, layer->pszDescription) ;
      if (model_layer == layer)
        {
        gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (private->tv)), &itr) ;
        break ;
        }
      }
    while (gtk_tree_model_iter_next (private->model, &itr)) ;

  if (bNotify)
    g_object_notify (G_OBJECT (layers_combo), "layer") ;
  }

/* BOOLEAN:OBJECT,BOOLEAN (marshal:1) */
void g_cclosure_user_marshal_BOOLEAN__OBJECT_BOOLEAN (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data)
  {
  typedef gboolean (*GMarshalFunc_BOOLEAN__OBJECT_BOOLEAN) (gpointer     data1,
                                                            gpointer     arg_1,
                                                            gboolean     arg_2,
                                                            gpointer     data2);
  register GMarshalFunc_BOOLEAN__OBJECT_BOOLEAN callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__OBJECT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_value_get_object  (param_values + 1),
                       g_value_get_boolean (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
  }
