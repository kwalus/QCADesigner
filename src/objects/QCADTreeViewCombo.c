#include "intl.h"
#include "custom_widgets.h"
#include "QCADTreeViewCombo.h"

typedef struct
  {
  GtkWidget *btn ;
  GtkWidget *frm ;
  GtkWidget *tbl ;
  GtkWidget *popup ;
  guint button_press_handler_id ;
  guint tree_sel_changed_handler_id ;
  int text_column ;
  } QCADTreeViewComboPrivate ;

enum
  {
  QCAD_TREE_VIEW_COMBO_PROPERTY_FIRST = 1,

  QCAD_TREE_VIEW_COMBO_PROPERTY_TEXT_COLUMN_IDX,

  QCAD_TREE_VIEW_COMBO_PROPERTY_LAST
  } ;

#define QCAD_TREE_VIEW_COMBO_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_TREE_VIEW_COMBO, QCADTreeViewComboPrivate))

static void qcad_tree_view_combo_class_init (QCADTreeViewComboClass *klass) ;
static void qcad_tree_view_combo_instance_init (QCADTreeViewCombo *instance) ;

static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void     size_request  (GtkWidget *widget, GtkRequisition *rq) ;
static void     size_allocate (GtkWidget *widget, GtkAllocation *alloc) ;
static gboolean expose_event  (GtkWidget *widget, GdkEventExpose *event) ;
static void qcad_tree_view_combo_add    (GtkContainer *container, GtkWidget *widget) ;
static void qcad_tree_view_combo_remove (GtkContainer *container, GtkWidget *widget) ;

static void arrow_btn_clicked (GtkWidget *btn, gpointer data) ;
static gboolean popup_button_pressed (GtkWidget *popup, GdkEventButton *event, gpointer data) ;
static void tree_selection_changed (GtkTreeSelection *sel, gpointer data) ;

static void qcad_tree_view_combo_update_label (QCADTreeViewCombo *tvc, int new_column) ;

// Copied from gtk+-2.4.14/gtk/gtkmenu.c
static gboolean grab_input (GdkWindow *window, guint32 activate_time) ;

GType qcad_tree_view_combo_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADTreeViewComboClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_tree_view_combo_class_init,
      NULL,
      NULL,
      sizeof (QCADTreeViewCombo),
      0,
      (GInstanceInitFunc)qcad_tree_view_combo_instance_init
      } ;

    if (0 != (the_type = g_type_register_static (GTK_TYPE_BIN, QCAD_TYPE_STRING_TREE_VIEW_COMBO, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void qcad_tree_view_combo_class_init (QCADTreeViewComboClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  GTK_WIDGET_CLASS (klass)->expose_event  = expose_event ;
  GTK_WIDGET_CLASS (klass)->size_request  = size_request ;
  GTK_WIDGET_CLASS (klass)->size_allocate = size_allocate ;
  GTK_CONTAINER_CLASS (klass)->add    = qcad_tree_view_combo_add ;
  GTK_CONTAINER_CLASS (klass)->remove = qcad_tree_view_combo_remove ;

  g_type_class_add_private (klass, sizeof (QCADTreeViewComboPrivate)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TREE_VIEW_COMBO_PROPERTY_TEXT_COLUMN_IDX,
    g_param_spec_int ("text-column-index", _("Text Column Index"), _("Index of column to set entry text from"),
      -1, G_MAXINT, -1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_tree_view_combo_instance_init (QCADTreeViewCombo *instance)  
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (instance) ;
  GtkWidget
    *frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_OUT, NULL),
    *arr = g_object_new (GTK_TYPE_ARROW, "visible", TRUE, "arrow-type", GTK_ARROW_DOWN, "shadow-type", GTK_SHADOW_IN, NULL) ;

  gtk_widget_add_events (GTK_WIDGET (instance), GDK_EXPOSURE_MASK) ;

  private->button_press_handler_id =
  private->tree_sel_changed_handler_id = 0 ;
  private->text_column = -1 ;
  instance->entry = g_object_new (GTK_TYPE_ENTRY, "visible", TRUE, "editable", FALSE, "xalign", 0.0, NULL),
  GTK_WIDGET_UNSET_FLAGS (instance->entry, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
  private->btn    = g_object_new (GTK_TYPE_TOGGLE_BUTTON, "visible", TRUE, "active", FALSE, NULL),
  private->popup  = g_object_new (GTK_TYPE_WINDOW, "type", GTK_WINDOW_POPUP, "type-hint", GDK_WINDOW_TYPE_HINT_MENU, "resizable", FALSE, "modal", TRUE, "decorated", FALSE, NULL) ;
  private->tbl    = g_object_new (GTK_TYPE_TABLE, "visible", TRUE, "n-rows", 1, "n-columns", 1, "homogeneous", FALSE, NULL),

  private->popup = g_object_ref (private->popup) ;
  gtk_object_sink (GTK_OBJECT (private->popup)) ;

  private->frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (private->popup), frm) ;
  gtk_container_add (GTK_CONTAINER (frm), private->frm) ;

  GTK_WIDGET_UNSET_FLAGS (private->btn, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  GTK_CONTAINER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_TREE_VIEW_COMBO)))->add (GTK_CONTAINER (instance), private->tbl) ;

  gtk_table_attach (GTK_TABLE (private->tbl), instance->entry, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL), 0, 2) ;

  gtk_container_add (GTK_CONTAINER (private->btn), arr) ;
  gtk_table_attach (GTK_TABLE (private->tbl), private->btn, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 2) ;

  g_signal_connect (G_OBJECT (private->btn), "clicked", (GCallback)arrow_btn_clicked, instance) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_TREE_VIEW_COMBO_PROPERTY_TEXT_COLUMN_IDX:
      qcad_tree_view_combo_update_label (QCAD_TREE_VIEW_COMBO (object), g_value_get_int (value)) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (object) ;

  switch (property_id)
    {
    case QCAD_TREE_VIEW_COMBO_PROPERTY_TEXT_COLUMN_IDX:
      g_value_set_int (value, private->text_column) ;
      break ;
    }
  }

static void finalize (GObject *object)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (object) ;

  if (NULL != private->popup)
    g_object_unref (private->popup) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_TREE_VIEW_COMBO)))->finalize (object) ;
  }

static gboolean expose_event (GtkWidget *widget, GdkEventExpose *event)
  {
  QCADTreeViewCombo        *cb      = QCAD_TREE_VIEW_COMBO (widget) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (widget) ;

  g_print ("QCADTreeViewCombo::expose_event\n") ;

  gtk_container_propagate_expose (GTK_CONTAINER (widget), cb->entry,      event) ;
  gtk_container_propagate_expose (GTK_CONTAINER (widget), private->tbl,   event) ;
  gtk_container_propagate_expose (GTK_CONTAINER (widget), private->btn,   event) ;
  gtk_container_propagate_expose (GTK_CONTAINER (widget), private->popup, event) ;
  gtk_container_propagate_expose (GTK_CONTAINER (widget), private->frm,   event) ;

  return FALSE ;
  }

static void size_request (GtkWidget *widget, GtkRequisition *rq)
  {
  gtk_widget_size_request (GTK_BIN (widget)->child, rq) ;

  rq->width  += (GTK_CONTAINER (widget)->border_width << 1) ;
  rq->height += (GTK_CONTAINER (widget)->border_width << 1) ;
  }

static void size_allocate (GtkWidget *widget, GtkAllocation *alloc)
  {
  alloc->x      += GTK_CONTAINER (widget)->border_width ;
  alloc->y      += GTK_CONTAINER (widget)->border_width ;
  alloc->width  -= (GTK_CONTAINER (widget)->border_width << 1) ;
  alloc->height -= (GTK_CONTAINER (widget)->border_width << 1) ;
  gtk_widget_size_allocate (GTK_BIN (widget)->child, alloc) ;
  }

static void qcad_tree_view_combo_add (GtkContainer *container, GtkWidget *widget)
  {
  GtkTreeSelection *sel = NULL ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (container) ;

  if (NULL == widget) return ;
  if (!GTK_IS_TREE_VIEW (widget)) return ;

  gtk_container_add (GTK_CONTAINER (private->frm), widget) ;

  if (NULL != (sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget))))
    private->tree_sel_changed_handler_id = 
      g_signal_connect (G_OBJECT (sel), "changed", (GCallback)tree_selection_changed, container) ;
  else
    private->tree_sel_changed_handler_id = 0 ;
  }

static void qcad_tree_view_combo_remove (GtkContainer *container, GtkWidget *widget)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (container) ;

  if (NULL == widget) return ;
  if (!GTK_IS_TREE_VIEW (widget)) return ;

  gtk_container_remove (GTK_CONTAINER (private->frm), widget) ;

  g_signal_handler_disconnect (widget, private->button_press_handler_id) ;
  if (private->tree_sel_changed_handler_id > 0)
    g_signal_handler_disconnect (widget, private->tree_sel_changed_handler_id) ;
  }

static void arrow_btn_clicked (GtkWidget *btn, gpointer data)
  {
  QCADTreeViewCombo *tvc = QCAD_TREE_VIEW_COMBO (data) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (tvc) ;
  GtkWidget *tv = gtk_bin_get_child (GTK_BIN (private->frm)) ;
  GtkWidget *widget = gtk_bin_get_child (GTK_BIN (data)) ;
  GtkWidget *grab_widget = private->popup ;

  if (NULL == tv)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), FALSE) ;
    return ;
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (btn)))
    {
    int x, y ;

    gtk_widget_get_root_origin (widget, &x, &y) ;
    y += widget->allocation.height ;
    gtk_window_move (GTK_WINDOW (private->popup), x, y) ;
    gtk_widget_show (private->popup) ;
    private->button_press_handler_id = 
      g_signal_connect (G_OBJECT (grab_widget), "button-press-event", (GCallback)popup_button_pressed, tvc) ;
    while (!GTK_WIDGET_VISIBLE (private->popup))
      gtk_main_iteration () ;
    if (!grab_input (grab_widget->window, GDK_CURRENT_TIME))
      gtk_widget_hide (private->popup) ;
    }
  else
    {
    gdk_display_pointer_ungrab (gtk_widget_get_display (grab_widget), GDK_CURRENT_TIME);
    gdk_display_keyboard_ungrab (gtk_widget_get_display (grab_widget), GDK_CURRENT_TIME);
    gtk_widget_hide (private->popup) ;
    while (GTK_WIDGET_VISIBLE (private->popup))
      gtk_main_iteration () ;
    g_signal_handler_disconnect (G_OBJECT (grab_widget), private->button_press_handler_id) ;
    }
  }

static void tree_selection_changed (GtkTreeSelection *sel, gpointer data)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data) ;

  qcad_tree_view_combo_update_label (QCAD_TREE_VIEW_COMBO (data), private->text_column) ;
  }

static gboolean popup_button_pressed (GtkWidget *popup, GdkEventButton *event, gpointer data)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data) ;
  int x, y, cx, cy ;

  gdk_window_get_position (popup->window, &x, &y) ;
  gdk_window_get_size (popup->window, &cx, &cy) ;

  if (event->x < x || event->x > cx || event->y < y || event->y > cy)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (private->btn), FALSE) ;

  return FALSE ;
  }

// Copied from gtk+-2.4.14/gtk/gtkmenu.c
static gboolean grab_input (GdkWindow *window, guint32 activate_time)
  {
  if ((gdk_pointer_grab (window, TRUE,
         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
         GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK   |
         GDK_POINTER_MOTION_MASK,
         NULL, NULL, activate_time) == 0))
    {
    if (gdk_keyboard_grab (window, TRUE, activate_time) == 0)
      return TRUE;
    else
      {
      gdk_display_pointer_ungrab (gdk_drawable_get_display (window), activate_time);
  	  return FALSE;
      }
    }

  return FALSE;
  }

static void qcad_tree_view_combo_update_label (QCADTreeViewCombo *tvc, int new_column)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (tvc) ;
  GtkTreeView *tv = NULL ;
  GtkTreeSelection *ts = NULL ;
  GtkTreeModel *tm = NULL ;
  GtkTreeIter itr ;
  int old_column = private->text_column ;
  char *psz = NULL ;
  GList *llSels = NULL ;

//  g_print ("qcad_tree_view_combo_update_label: Entering with new_column = %d\n", new_column) ;

  if (NULL != (tv = GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (private->frm)))))
    if (NULL != (tm = gtk_tree_view_get_model (tv)))
      {
      if (new_column < 0 || new_column > gtk_tree_model_get_n_columns (tm) - 1) return ;
      if (G_TYPE_STRING != gtk_tree_model_get_column_type (tm, new_column)) return ;
      }

  private->text_column = new_column ;
  if (new_column != old_column)
    g_object_notify (G_OBJECT (tvc), "text-column-index") ;

  if (-1 == new_column || NULL == tm || NULL == tv) 
    {
    g_object_set (G_OBJECT (tvc->entry), "text", "", NULL) ;
    return ;
    }

  if (NULL == (ts = gtk_tree_view_get_selection (tv))) return ;
  if (NULL == (llSels = gtk_tree_selection_get_selected_rows (ts, NULL))) return ;

  if (gtk_tree_model_get_iter (tm, &itr, ((GtkTreePath *)(llSels->data))))
    gtk_tree_model_get (tm, &itr, new_column, &psz, -1) ;

  g_list_foreach (llSels, (GFunc)gtk_tree_path_free, NULL) ;
  g_list_free (llSels) ;

  if (NULL == psz)
    g_object_set (G_OBJECT (tvc->entry), "text", "", NULL) ;
  else
    {
    g_object_set (G_OBJECT (tvc->entry), "text", psz, NULL) ;
    g_free (psz) ;
    }
  }
