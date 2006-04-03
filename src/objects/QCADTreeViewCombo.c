#include <gdk/gdkkeysyms.h>
#include "../intl.h"
#include "../custom_widgets.h"
#include "QCADTreeViewCombo.h"

typedef struct
  {
  GtkWidget *btn ;
  GtkWidget *frm ;
  GtkWidget *popup ;
  guint hide_grab_widget_handler_id ;
  guint entry_activate_handler_id ;
  guint tree_sel_changed_handler_id ;
  guint tree_view_key_press_handler_id ;
  guint tree_view_row_activated_handler_id ;
  guint tree_view_button_release_handler_id ;
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
static void qcad_tree_view_combo_add    (GtkContainer *container, GtkWidget *widget) ;
static void qcad_tree_view_combo_remove (GtkContainer *container, GtkWidget *widget) ;
static void forall                      (GtkContainer *container, gboolean bPrivate, GtkCallback cb, gpointer data) ;

static void arrow_btn_clicked (GtkWidget *btn, gpointer data) ;
static gboolean hide_grab_widget (GtkWidget *popup, GdkEventButton *event, gpointer data) ;
static gboolean tree_view_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data) ;
static gboolean tree_view_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static void tree_selection_changed (GtkTreeSelection *sel, gpointer data) ;
static void tree_view_row_activated (GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, gpointer data) ;
static void entry_activate (GtkWidget *widget, gpointer data) ;

static void qcad_tree_view_combo_update_label (QCADTreeViewCombo *tvc, int new_column) ;
static void qcad_tree_view_combo_show_popup (QCADTreeViewCombo *tvc, gboolean bShow) ;
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

    if (0 != (the_type = g_type_register_static (GTK_TYPE_HBOX, QCAD_TYPE_STRING_TREE_VIEW_COMBO, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void qcad_tree_view_combo_class_init (QCADTreeViewComboClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  GTK_WIDGET_CLASS (klass)->size_request  = size_request ;
  GTK_WIDGET_CLASS (klass)->size_allocate = size_allocate ;
  GTK_CONTAINER_CLASS (klass)->add    = qcad_tree_view_combo_add ;
  GTK_CONTAINER_CLASS (klass)->remove = qcad_tree_view_combo_remove ;
  GTK_CONTAINER_CLASS (klass)->forall = forall ;

  g_type_class_add_private (klass, sizeof (QCADTreeViewComboPrivate)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TREE_VIEW_COMBO_PROPERTY_TEXT_COLUMN_IDX,
    g_param_spec_int ("text-column", _("Text Column Index"), _("Index of column to set entry text from"),
      -1, G_MAXINT, -1, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_tree_view_combo_instance_init (QCADTreeViewCombo *instance)  
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (instance) ;
  GtkWidget
    *frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_OUT, NULL),
    *arr = g_object_new (GTK_TYPE_ARROW, "visible", TRUE, "arrow-type", GTK_ARROW_DOWN, "shadow-type", GTK_SHADOW_OUT, NULL) ;

  private->hide_grab_widget_handler_id =
  private->tree_sel_changed_handler_id =
  private->tree_view_key_press_handler_id =
  private->tree_view_button_release_handler_id = 0 ;
  private->text_column = -1 ;

  instance->entry = g_object_new (GTK_TYPE_ENTRY, "visible", TRUE, "editable", FALSE, "xalign", 0.0, NULL),
  g_signal_connect (G_OBJECT (instance->entry), "activate", (GCallback)entry_activate, instance) ;
  private->btn    = g_object_new (GTK_TYPE_BUTTON, "visible", TRUE, NULL),
  gtk_container_add (GTK_CONTAINER (private->btn), arr) ;
  private->popup  = g_object_new (GTK_TYPE_WINDOW, 
    "type", GTK_WINDOW_POPUP, "modal", TRUE, 
#ifndef WIN32
    "type-hint", GDK_WINDOW_TYPE_HINT_MENU, "resizable", FALSE, "decorated", FALSE,
#endif /* def WIN32 */
    NULL) ;

  private->popup = g_object_ref (private->popup) ;
  gtk_object_sink (GTK_OBJECT (private->popup)) ;
  private->btn = g_object_ref (private->btn) ;
  gtk_object_sink (GTK_OBJECT (private->btn)) ;
  instance->entry = g_object_ref (instance->entry) ;
  gtk_object_sink (GTK_OBJECT (instance->entry)) ;

  private->frm = g_object_new (GTK_TYPE_FRAME, "visible", TRUE, "shadow-type", GTK_SHADOW_IN, NULL) ;
  gtk_container_add (GTK_CONTAINER (private->popup), frm) ;
  gtk_container_add (GTK_CONTAINER (frm), private->frm) ;

  GTK_WIDGET_UNSET_FLAGS (private->btn, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;

  gtk_widget_set_parent (instance->entry, GTK_WIDGET (instance)) ;
  gtk_widget_set_parent (private->btn, GTK_WIDGET (instance)) ;

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
  QCADTreeViewCombo        *tvc     = QCAD_TREE_VIEW_COMBO (object) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (object) ;

  if (NULL != private->popup) g_object_unref (private->popup) ;
  if (NULL != private->btn) g_object_unref (private->btn) ;
  if (NULL != tvc->entry) g_object_unref (tvc->entry) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_TREE_VIEW_COMBO)))->finalize (object) ;
  }

static void size_request (GtkWidget *widget, GtkRequisition *rq)
  {
  QCADTreeViewCombo        *tvc     = QCAD_TREE_VIEW_COMBO (widget) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (widget) ;
  GtkRequisition rq_btn, rq_entry ;

  gtk_widget_size_request (private->btn, &rq_btn) ;
  gtk_widget_size_request (tvc->entry, &rq_entry) ;
  rq->width  = rq_btn.width + rq_entry.width        + (GTK_CONTAINER (widget)->border_width << 1) ;
  rq->height = MAX (rq_btn.height, rq_entry.height) + (GTK_CONTAINER (widget)->border_width << 1) ;
  }

static void size_allocate (GtkWidget *widget, GtkAllocation *allocation)
  {
  QCADTreeViewCombo        *tvc     = QCAD_TREE_VIEW_COMBO (widget) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (widget) ;
  GtkRequisition rq_btn, rq_entry ;
  GtkAllocation alloc_btn, alloc_entry, 
    alloc_child =
      {
      allocation->x      +  GTK_CONTAINER (widget)->border_width,
      allocation->y      +  GTK_CONTAINER (widget)->border_width,
      allocation->width  - (GTK_CONTAINER (widget)->border_width << 1),
      allocation->height - (GTK_CONTAINER (widget)->border_width << 1),
      } ;

  gtk_widget_size_request (private->btn, &rq_btn) ;
  gtk_widget_size_request (tvc->entry, &rq_entry) ;

  widget->allocation = (*allocation) ;

  alloc_entry.x = alloc_child.x ;
  alloc_entry.y = alloc_child.y ;
  alloc_entry.width = MAX (0, alloc_child.width - rq_btn.width) ;
  alloc_entry.height = MIN (alloc_child.height, rq_entry.height) ;

  alloc_btn.x = alloc_entry.x + alloc_entry.width ;
  alloc_btn.y = alloc_entry.y ;
  alloc_btn.width = alloc_child.width - alloc_entry.width ;
  alloc_btn.height = alloc_entry.height ;

  gtk_widget_size_allocate (tvc->entry, &alloc_entry) ;
  gtk_widget_size_allocate (private->btn, &alloc_btn) ;
  }

static void forall (GtkContainer *container, gboolean bPrivate, GtkCallback cb, gpointer data)
  {
  QCADTreeViewCombo *tvc = QCAD_TREE_VIEW_COMBO (container) ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (container) ;

  if (bPrivate)
    {
    if (NULL != tvc->entry) (*cb) (tvc->entry, data) ;
    if (NULL != private->btn) (*cb) (private->btn, data) ;
    }
  }

static void qcad_tree_view_combo_add (GtkContainer *container, GtkWidget *widget)
  {
  GtkTreeSelection *sel = NULL ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (container) ;

  if (NULL == widget) return ;
  if (!GTK_IS_TREE_VIEW (widget)) return ;

  gtk_container_add (GTK_CONTAINER (private->frm), widget) ;

  private->tree_view_row_activated_handler_id = 
    g_signal_connect (G_OBJECT (widget), "row-activated", (GCallback)tree_view_row_activated, container) ;
  private->tree_view_key_press_handler_id = 
    g_signal_connect (G_OBJECT (widget), "key-press-event", (GCallback)tree_view_key_press_event, container) ;
  private->tree_view_button_release_handler_id =
    g_signal_connect (G_OBJECT (widget), "button-release-event", (GCallback)tree_view_button_release_event, container) ;
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

  g_signal_handler_disconnect (widget, private->tree_view_key_press_handler_id) ;
  g_signal_handler_disconnect (widget, private->tree_view_row_activated_handler_id) ;
  g_signal_handler_disconnect (widget, private->tree_view_button_release_handler_id) ;

  if (private->tree_sel_changed_handler_id > 0)
    g_signal_handler_disconnect (widget, private->tree_sel_changed_handler_id) ;

  private->hide_grab_widget_handler_id =
  private->tree_sel_changed_handler_id = 
  private->tree_view_row_activated_handler_id =
  private->tree_view_button_release_handler_id = 0 ;
  }

static void arrow_btn_clicked (GtkWidget *btn, gpointer data)
  {qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), !GTK_WIDGET_VISIBLE (QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data)->popup)) ;}

static void tree_selection_changed (GtkTreeSelection *sel, gpointer data)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data) ;

  qcad_tree_view_combo_update_label (QCAD_TREE_VIEW_COMBO (data), private->text_column) ;
  }

static gboolean tree_view_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
  {
  if (0 == event->state && GDK_Escape == event->keyval)
    qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), FALSE) ;
  else
    return FALSE ;

  return TRUE ;
  }

static gboolean tree_view_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  GtkCellRendererMode mode = 0 ;
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data) ;
  GtkCellRenderer *cr = NULL ;
  GtkWidget *tv = NULL ;

  if (NULL == (tv = gtk_bin_get_child (GTK_BIN (private->frm)))) return FALSE ;
  if (NULL == (cr = gtk_tree_view_get_cell_renderer_at_point (GTK_TREE_VIEW (tv), event->x, event->y, NULL, NULL))) return FALSE ;

  g_object_get (G_OBJECT (cr), "mode", &mode, NULL) ;
  if (mode == GTK_CELL_RENDERER_MODE_INERT)
//    {
//    g_print ("tree_view_button_release_event: Hiding popup\n") ;
    qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), FALSE) ;
//    }
  return TRUE ;
  }
static gboolean hide_grab_widget (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (data) ;
  int x, y, cx, cy ;

  gdk_window_get_position (private->popup->window, &x, &y) ;
  gdk_window_get_size (private->popup->window, &cx, &cy) ;

  if (event->x < x || event->x > cx || event->y < y || event->y > cy)
    qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), FALSE) ;

  return FALSE ;
  }

static void tree_view_row_activated (GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, gpointer data)
  {
//  g_print ("tree_view_row_activated: Hiding popup\n") ;
  qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), FALSE) ;
  }

static void entry_activate (GtkWidget *widget, gpointer data)
  {qcad_tree_view_combo_show_popup (QCAD_TREE_VIEW_COMBO (data), TRUE) ;}

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

  if (NULL != (tv = GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (private->frm)))))
    if (NULL != (tm = gtk_tree_view_get_model (tv)))
      {
      if (new_column < 0 || new_column > gtk_tree_model_get_n_columns (tm) - 1) return ;
      if (G_TYPE_STRING != gtk_tree_model_get_column_type (tm, new_column)) return ;
      }

  private->text_column = new_column ;
  if (new_column != old_column)
    g_object_notify (G_OBJECT (tvc), "text-column") ;

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

static void qcad_tree_view_combo_show_popup (QCADTreeViewCombo *tvc, gboolean bShow)
  {
  QCADTreeViewComboPrivate *private = QCAD_TREE_VIEW_COMBO_GET_PRIVATE (tvc) ;
  GtkWidget *tv = gtk_bin_get_child (GTK_BIN (private->frm)) ;
  GtkWidget *grab_widget = private->popup ;

  if (NULL == tv)
    bShow = FALSE ;

  if (bShow)
    {
    int x, y ;

    gtk_widget_get_root_origin (GTK_WIDGET (tvc), &x, &y) ;
    y += GTK_WIDGET (tvc)->allocation.height - GTK_CONTAINER (tvc)->border_width ;
    x += GTK_CONTAINER (tvc)->border_width ;
    gtk_window_move (GTK_WINDOW (private->popup), x, y) ;
    gtk_widget_show (private->popup) ;
/*
    if (GTK_WIDGET_CAN_FOCUS (tv))
      {
      g_print ("Using gtk_widget_grab_focus\n") ;
      private->hide_grab_widget_handler_id = 
        g_signal_connect (G_OBJECT (tv), "focus-out-event", (GCallback)hide_grab_widget, tvc) ;
      gtk_widget_grab_focus (tv) ;
      }
    else
*/
      {
      g_print ("Using grab_input\n") ;
      private->hide_grab_widget_handler_id = 
        g_signal_connect (G_OBJECT (grab_widget), "button-press-event", (GCallback)hide_grab_widget, tvc) ;
      if (!grab_input (grab_widget->window, GDK_CURRENT_TIME))
        gtk_widget_hide (private->popup) ;
      }
    while (!GTK_WIDGET_VISIBLE (private->popup))
      gtk_main_iteration () ;
    }
  else
    {
//    if (!GTK_WIDGET_CAN_FOCUS (tv))
//      {
      gdk_display_pointer_ungrab (gtk_widget_get_display (grab_widget), GDK_CURRENT_TIME);
      gdk_display_keyboard_ungrab (gtk_widget_get_display (grab_widget), GDK_CURRENT_TIME);
//      }
    gtk_widget_hide (private->popup) ;
    while (GTK_WIDGET_VISIBLE (private->popup))
      gtk_main_iteration () ;
#ifdef USE_GRAB
    g_signal_handler_disconnect (G_OBJECT (grab_widget), private->hide_grab_widget_handler_id) ;
#endif /* def USE_GRAB */
    }
  }
