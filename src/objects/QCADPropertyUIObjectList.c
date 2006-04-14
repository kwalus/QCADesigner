#include <string.h>
#include "../custom_widgets.h"
#include "QCADObject.h"
#include "QCADPropertyUIObjectList.h"
#include "QCADPropertyUIGroup.h"

/**
 * SECTION:QCADPropertyUIObjectList
 * @short_description: Property UI for a param of type #QCADParamSpecObjectList
 *
 * Parameters of type #QCADParamSpecObjectList return a #GList of #GObject items. This property UI
 * creates a #QCADPropertyUIGroup for each object in the list and assembles them all into two widgets:
 * A #GtkTreeView with the friendly names of the object classes in the #GList on the left, and a #GtkVBox
 * with each object's #QCADPropertyUIGroup. Only the #QCADPropertyUIGroup for the currently hilighted
 * #GtkTreeView item is displayed.
 *
 */

static void qcad_property_ui_object_list_class_init (QCADPropertyUIObjectListClass *propert_ui_object_list_class) ;
static void qcad_property_ui_object_list_instance_init (QCADPropertyUIObjectList *propert_ui_object_list) ;

#ifdef GTK_GUI
static void finalize (GObject *obj) ;

static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;

static void tree_view_row_activated (GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, gpointer data) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_object_list_get_type ()
  {
  static GType qcad_property_ui_object_list_type = 0 ;

  if (0 == qcad_property_ui_object_list_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIObjectListClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_property_ui_object_list_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADPropertyUIObjectList),
      0,
      (GInstanceInitFunc)qcad_property_ui_object_list_instance_init
      } ;

    if ((qcad_property_ui_object_list_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_OBJECT_LIST, &info, 0)))
      g_type_class_ref (qcad_property_ui_object_list_type) ;
    }

  return qcad_property_ui_object_list_type ;
  }

static void qcad_property_ui_object_list_class_init (QCADPropertyUIObjectListClass *property_ui_object_list_class)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (property_ui_object_list_class)->finalize = finalize ;

  QCAD_PROPERTY_UI_CLASS (property_ui_object_list_class)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (property_ui_object_list_class)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (property_ui_object_list_class)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (property_ui_object_list_class)->get_widget    = get_widget ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_object_list_instance_init (QCADPropertyUIObjectList *property_ui_object_list)
  {
#ifdef GTK_GUI
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkWidget *tbl = NULL ;
  GtkTreeSelection *ts = NULL ;
  GtkTreeView *tv = NULL ;
#endif /* def GTK_GUI */
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_object_list) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

#ifdef GTK_GUI
  property_ui_object_list->frame.widget = gtk_frame_new (NULL) ;
  g_object_ref (property_ui_object_list->frame.widget) ;
  property_ui_object_list->frame.idxX = 0 ;
  property_ui_object_list->frame.idxY = 0 ;
  gtk_widget_show (property_ui_object_list->frame.widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_object_list->frame.widget), 2) ;

  tbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;
  gtk_container_add (GTK_CONTAINER (property_ui_object_list->frame.widget), tbl) ;

  property_ui_object_list->sw_tv = gtk_scrolled_window_new (NULL, NULL) ;
  gtk_widget_show (property_ui_object_list->sw_tv) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_object_list->sw_tv), 2) ;
  gtk_table_attach (GTK_TABLE (tbl), property_ui_object_list->sw_tv, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (property_ui_object_list->sw_tv), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC) ;
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (property_ui_object_list->sw_tv), GTK_SHADOW_IN) ;

  property_ui_object_list->tvObjects = gtk_tree_view_new () ;
  gtk_widget_show (property_ui_object_list->tvObjects) ;
  gtk_container_add (GTK_CONTAINER (property_ui_object_list->sw_tv), property_ui_object_list->tvObjects) ;
  gtk_tree_view_append_column (tv = GTK_TREE_VIEW (property_ui_object_list->tvObjects), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_text_new (), TRUE) ;
  gtk_tree_view_column_add_attribute (col, cr, "text", 0) ;
  gtk_tree_view_set_headers_visible (tv, FALSE) ;
  if (NULL != (ts = gtk_tree_view_get_selection (tv)))
    gtk_tree_selection_set_mode (ts, GTK_SELECTION_BROWSE) ;

  property_ui_object_list->tblObjects = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (property_ui_object_list->tblObjects) ;
  gtk_table_attach (GTK_TABLE (tbl), property_ui_object_list->tblObjects, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  g_signal_connect (G_OBJECT (property_ui_object_list->tvObjects), "row-activated", (GCallback)tree_view_row_activated, NULL) ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  GtkTreeModel *tm = NULL ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;
  QCADPropertyUIObjectList *property_ui_object_list = QCAD_PROPERTY_UI_OBJECT_LIST (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST)))->set_instance (property_ui, new_instance, old_instance))
    {
    GtkTreeView *tv = GTK_TREE_VIEW (property_ui_object_list->tvObjects) ;

    if (NULL != (tm = gtk_tree_view_get_model (tv)))
      {
      QCADPropertyUI *pui = NULL ;
      GtkTreeIter itr ;

      if (gtk_tree_model_get_iter_first (tm, &itr))
        do
          {
          gtk_tree_model_get (tm, &itr, 1, &pui, -1) ;
          if (NULL != pui)
            g_object_unref (pui) ;
          }
        while (gtk_tree_model_iter_next (tm, &itr)) ;
      gtk_tree_view_set_model (tv, NULL) ;
      }

    if (!(NULL == new_instance || NULL == property_ui_single->pspec))
      {
      int idx = 0, Nix ;
      GList *llItr = NULL ;
      GtkTreeIter itr ;
      QCADPropertyUI *pui ;
      GtkListStore *ls = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER) ;
      GtkWidget *widget = NULL ;
      char *pszTitle = NULL ;
      QCADPropertyUIProperty *puip = NULL ;
      EXP_ARRAY *property_ui_properties = NULL ;

      g_object_get (G_OBJECT (new_instance), property_ui_single->pspec->name, &llItr, NULL) ;
      property_ui_object_list->llPUIs = NULL ;
      for (; llItr != NULL ; llItr = llItr->next)
        if (NULL != llItr->data)
          {
          gtk_list_store_append (ls, &itr) ;

          pui = qcad_property_ui_group_new (G_OBJECT (llItr->data), "render-as", GTK_TYPE_FRAME, "visible", FALSE, NULL) ;
          property_ui_object_list->llPUIs = g_list_prepend (property_ui_object_list->llPUIs, pui) ;

          // Initially, set the title to the type name
          pszTitle = (char *)g_type_name (G_TYPE_FROM_INSTANCE (llItr->data)) ;

          // However, we should try to find a nice name for the list entry by checking whether the type sets 
          // the title for its QCADPropertyUIGroup
          if (QCAD_IS_OBJECT (llItr->data))
            if (NULL != (property_ui_properties = QCAD_OBJECT_GET_CLASS (llItr->data)->property_ui_properties))
              for (Nix = 0 ; Nix < property_ui_properties->icUsed ; Nix++)
                if (NULL == (puip = &exp_array_index_1d (property_ui_properties, QCADPropertyUIProperty, Nix))->instance_property_name)
                  if (NULL != puip->ui_property_name)
                    if (!strcmp (puip->ui_property_name, "title"))
                      {
                      pszTitle = (char *)g_value_get_string (&(puip->ui_property_value)) ;
                      break ;
                      }

          gtk_list_store_set (ls, &itr, 
             0, pszTitle,
             1, pui,
            -1) ;
          widget = qcad_property_ui_get_widget (pui, 0, 0, NULL) ;
          gtk_table_attach (GTK_TABLE (property_ui_object_list->tblObjects), widget, 0, 1, idx, idx + 1,
            (GtkAttachOptions)(GTK_FILL),
            (GtkAttachOptions)(GTK_FILL), 2, 2) ;
          idx++ ;
          }
      gtk_tree_view_set_model (tv, GTK_TREE_MODEL (ls)) ;
      // Pretend to click on the first entry in the list, so as to bring up its interface
      if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ls), &itr))
        {
        GtkTreePath *tp = gtk_tree_model_get_path (GTK_TREE_MODEL (ls), &itr) ;
        
        gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tv), &itr) ;
        gtk_tree_view_row_activated (tv, tp, gtk_tree_view_get_column (tv, 0)) ;
        gtk_tree_path_free (tp) ;
        }
      scrolled_window_set_size (GTK_SCROLLED_WINDOW (property_ui_object_list->sw_tv), property_ui_object_list->tvObjects, 0.8, 0.8) ;
      }

    gtk_frame_set_label (GTK_FRAME (property_ui_object_list->frame.widget), g_param_spec_get_nick (property_ui_single->pspec)) ;
    return TRUE ;
    }
  return FALSE ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST)))->set_visible (property_ui, bVisible) ;
  GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_OBJECT_LIST (property_ui)->frame.widget, bVisible) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIObjectList *property_ui_object_list = QCAD_PROPERTY_UI_OBJECT_LIST (property_ui) ;

  if (property_ui_object_list->frame.idxX == idxX && property_ui_object_list->frame.idxY == idxY)
    {
    (*col_span) = -1 ;
    return property_ui_object_list->frame.widget ;
    }
  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST)))->get_widget (property_ui, idxX, idxY, col_span) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_OBJECT_LIST (property_ui)->frame.widget, bSensitive) ;
  }

static void tree_view_row_activated (GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, gpointer data)
  {
  GObject *pui = NULL ;
  GtkTreeIter itr ;
  GtkTreeModel *tm = gtk_tree_view_get_model (tv) ;

  if (gtk_tree_model_get_iter_first (tm, &itr))
    do
      {
      gtk_tree_model_get (tm, &itr, 1, &pui, -1) ;
      g_object_set (pui, "visible", FALSE, NULL) ;
      }
    while (gtk_tree_model_iter_next (tm, &itr)) ;

  if (gtk_tree_model_get_iter (tm, &itr, tp))
    {
    gtk_tree_model_get (tm, &itr, 1, &pui, -1) ;
    g_object_set (pui, "visible", TRUE, NULL) ;
    }
  }

static void finalize (GObject *obj)
  {
  GObject *pui = NULL ;
  QCADPropertyUIObjectList *property_ui_object_list = QCAD_PROPERTY_UI_OBJECT_LIST (obj) ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;

  if (NULL != (tm = gtk_tree_view_get_model (GTK_TREE_VIEW (property_ui_object_list->tvObjects))))
    if (gtk_tree_model_get_iter_first (tm, &itr))
      do
        {
        gtk_tree_model_get (tm, &itr, 1, &pui, -1) ;
        g_object_unref (pui) ;
        }
      while (gtk_tree_model_iter_next (tm, &itr)) ;

  gtk_tree_view_set_model (GTK_TREE_VIEW (property_ui_object_list->tvObjects), NULL) ;

  g_object_unref (property_ui_object_list->frame.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST)))->finalize (obj) ;
  }
#endif /* def GTK_GUI */
