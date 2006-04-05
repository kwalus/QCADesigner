#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "QCADFlexiCombo.h"
#include "../intl.h"
#include "../custom_widgets.h"

typedef struct
  {
  GtkWidget *combo_box_btn ;
  GtkWidget *btn ;
  GtkWidget *popup ;
  guint hide_grab_widget_handler_id ;
  gboolean bParentWidgetsVisible ;
  } QCADFlexiComboPrivate ;

#define QCAD_FLEXI_COMBO_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_FLEXI_COMBO, QCADFlexiComboPrivate))

enum
  {
  QCAD_FLEXI_COMBO_PROPERTY_FIRST = 1,

  QCAD_FLEXI_COMBO_PROPERTY_POPUP_WIDGET,

  QCAD_FLEXI_COMBO_PROPERTY_LAST
  } ;

static void class_init (QCADFlexiComboClass *klass) ;
static void instance_init (QCADFlexiCombo *instance) ;

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void finalize     (GObject *obj) ;
static void size_request  (GtkWidget *widget, GtkRequisition *rq) ;
static void size_allocate (GtkWidget *widget, GtkAllocation *alloc) ;
static void forall (GtkContainer *container, gboolean bInternals, GtkCallback callback, gpointer data) ;

static void arrow_btn_clicked    (GtkWidget *widget, gpointer data) ;
static void forall_callback      (GtkWidget *widget, gpointer data) ;
static gboolean hide_grab_widget (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean popup_key_release (GtkWidget *widget, GdkEventKey *event, gpointer data) ;

void qcad_flexi_combo_show_popup (QCADFlexiCombo *flexi_combo, gboolean bShow) ;
// Copied from gtk+-2.4.14/gtk/gtkmenu.c
static gboolean grab_input (GdkWindow *window, guint32 activate_time) ;

GType qcad_flexi_combo_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADFlexiComboClass),
      NULL,
      NULL,
      (GClassInitFunc)class_init,
      NULL,
      NULL,
      sizeof (QCADFlexiCombo),
      0,
      (GInstanceInitFunc)instance_init
      } ;

    if (0 != (the_type = g_type_register_static (GTK_TYPE_COMBO_BOX_ENTRY, QCAD_TYPE_STRING_FLEXI_COMBO, &the_type_info, 0)))
//    if (0 != (the_type = g_type_register_static (GTK_TYPE_BIN, QCAD_TYPE_STRING_FLEXI_COMBO, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void class_init (QCADFlexiComboClass *klass)
  {
/*
  const char *rc_string = ""
  "style \"my_combo\""
  "{"
  "  GtkComboBox::appears-as-list = 1"
  "}"
  "widget_class \"*.MyComboBox\" style \"my_combo\"";

  gtk_rc_parse_string (rc_string);
*/
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  GTK_WIDGET_CLASS (klass)->size_request  = size_request ;
  GTK_WIDGET_CLASS (klass)->size_allocate = size_allocate ;
  GTK_WIDGET_CLASS (klass)->expose_event  = GTK_WIDGET_CLASS (g_type_class_peek (g_type_parent (GTK_TYPE_COMBO_BOX)))->expose_event ;
  GTK_WIDGET_CLASS (klass)->style_set     = GTK_WIDGET_CLASS (g_type_class_peek (g_type_parent (GTK_TYPE_COMBO_BOX)))->style_set ;
  GTK_CONTAINER_CLASS (klass)->add    = GTK_CONTAINER_CLASS (g_type_class_peek (g_type_parent (GTK_TYPE_COMBO_BOX)))->add ;
  GTK_CONTAINER_CLASS (klass)->remove = GTK_CONTAINER_CLASS (g_type_class_peek (g_type_parent (GTK_TYPE_COMBO_BOX)))->remove ;
  GTK_CONTAINER_CLASS (klass)->forall = forall ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_FLEXI_COMBO_PROPERTY_POPUP_WIDGET,
    g_param_spec_object ("popup-widget", _("Popup Widget"), _("The widget to show when the user clicks the button"),
      GTK_TYPE_WIDGET, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_type_class_add_private (klass, sizeof (QCADFlexiComboPrivate)) ;
  }

static void instance_init (QCADFlexiCombo *instance)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (instance) ;

  private->bParentWidgetsVisible = TRUE ;
  private->combo_box_btn = NULL ;

  gtk_container_remove (GTK_CONTAINER (instance), GTK_BIN (instance)->child) ;
  GTK_BIN (instance)->child = NULL ;

  private->btn = g_object_new (GTK_TYPE_BUTTON, "visible", TRUE, NULL) ;
  gtk_container_add (GTK_CONTAINER (private->btn), 
    g_object_new (GTK_TYPE_ARROW, "visible", TRUE, "arrow-type", GTK_ARROW_DOWN, "shadow-type", GTK_SHADOW_NONE, NULL)) ;
  gtk_widget_set_parent (private->btn, GTK_WIDGET (instance)) ;
  g_object_ref (private->btn) ;
  gtk_object_sink (GTK_OBJECT (private->btn)) ;

  private->popup  = g_object_new (GTK_TYPE_WINDOW, 
    "type", GTK_WINDOW_POPUP, "modal", TRUE, 
#ifndef WIN32
    "type-hint", GDK_WINDOW_TYPE_HINT_MENU, "resizable", FALSE, "decorated", FALSE,
#endif /* def WIN32 */
    NULL) ;
  gtk_widget_add_events (private->popup, GDK_KEY_RELEASE_MASK) ;
  private->popup = g_object_ref (private->popup) ;
  gtk_object_sink (GTK_OBJECT (private->popup)) ;
  g_signal_connect (G_OBJECT (private->btn), "clicked", (GCallback)arrow_btn_clicked, instance) ;
  g_signal_connect (G_OBJECT (private->popup), "key-release-event", (GCallback)popup_key_release, instance) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (object) ;

  switch (property_id)
    {
    case QCAD_FLEXI_COMBO_PROPERTY_POPUP_WIDGET:
      {
      GtkWidget *new_child = GTK_WIDGET (g_value_get_object (value)) ;

      if (new_child == GTK_BIN (private->popup)->child) return ;

      if (NULL != GTK_BIN (private->popup)->child)
        gtk_container_remove (GTK_CONTAINER (private->popup), GTK_BIN (private->popup)->child) ;

      if (NULL != new_child)
        {
        gtk_container_add (GTK_CONTAINER (private->popup), new_child) ;
        gtk_widget_show (new_child) ;
        }

      g_object_notify (object, "popup-widget") ;
      break ;
      }
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (object) ;

  switch (property_id)
    {
    case QCAD_FLEXI_COMBO_PROPERTY_POPUP_WIDGET:
      g_value_set_object (value, GTK_BIN (private->popup)->child) ;
      break ;
    }
  }

static void finalize (GObject *obj)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (obj) ;

  gtk_widget_unparent (private->btn) ;
  g_object_unref (private->btn) ;
  }

static void size_request (GtkWidget *widget, GtkRequisition *rq)
  {
  GtkWidget *bin_child = GTK_BIN (widget)->child ;
  GtkRequisition rq_btn, rq_child = {0, 0} ;
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (widget) ;

  gtk_widget_size_request (private->btn, &rq_btn) ;

//  g_print ("size_request: Button wants [%dx%d]\n", rq_btn.width, rq_btn.height) ;

  if (NULL != bin_child)
    {
    gtk_widget_size_request (bin_child, &rq_child) ;

//    g_print ("size_request: Child wants [%dx%d]\n", rq_child.width, rq_child.height) ;
    }

  rq->width  =     rq_btn.width  + rq_child.width   + (GTK_CONTAINER (widget)->border_width << 1) ;
  rq->height = (0 == rq_child.height ? rq_btn.height : rq_child.height) + (GTK_CONTAINER (widget)->border_width << 1) ;

//  g_print ("size_request: Setting rq to [%dx%d]\n", rq->width, rq->height) ;
  }

static void size_allocate (GtkWidget *widget, GtkAllocation *alloc)
  {
  GtkWidget *bin_child = GTK_BIN (widget)->child ;
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (widget) ;
  GtkAllocation alloc_inside, alloc_btn, alloc_child = {0, 0, 0, 0} ;

  widget->allocation = (*alloc) ;
  
//  g_print ("size_allocate: Gotten (%d,%d)[%dx%d]\n", alloc->x, alloc->y, alloc->width, alloc->height) ;
//  g_print ("size_allocate: GTK_CONTAINER (widget)->border_width = %d\n", GTK_CONTAINER (widget)->border_width) ;

  alloc_inside.x      = alloc->x      + GTK_CONTAINER (widget)->border_width ;
  alloc_inside.y      = alloc->y      + GTK_CONTAINER (widget)->border_width ;
  alloc_inside.width  = alloc->width  - (GTK_CONTAINER (widget)->border_width << 1) ;
  alloc_inside.height = alloc->height - (GTK_CONTAINER (widget)->border_width << 1) ;

//  g_print ("size_allocate: Minus border width: (%d,%d)[%dx%d]\n", alloc_inside.x, alloc_inside.y, alloc_inside.width, alloc_inside.height) ;

  alloc_child.x      = alloc_inside.x ;
  alloc_child.y      = alloc_inside.y ;
  if (NULL != bin_child)
    {
    alloc_child.width  = MAX (0, alloc_inside.width - private->btn->requisition.width) ;
    alloc_child.height = MIN (alloc_inside.height, bin_child->requisition.height) ;
    }
  
  alloc_btn.x      = alloc_child.x + alloc_child.width ;
  alloc_btn.height = (0 == alloc_child.height ? private->btn->requisition.height : alloc_child.height) ;
  alloc_btn.y      = alloc_child.y + ((alloc_child.height - alloc_btn.height) >> 1) ;
  alloc_btn.width  = MAX (0, alloc_inside.width - alloc_child.width) ;

//  g_print ("size_allocate: Giving the button (%d,%d)[%dx%d]\n", alloc_btn.x, alloc_btn.y, alloc_btn.width, alloc_btn.height) ;  
//  g_print ("size_allocate: Giving the child (%d,%d)[%dx%d]\n", alloc_child.x, alloc_child.y, alloc_child.width, alloc_child.height) ;

  gtk_widget_size_allocate (private->btn, &alloc_btn) ;
  if (NULL != bin_child)
    gtk_widget_size_allocate (bin_child, &alloc_child) ;

//  GTK_WIDGET_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_COMBO_BOX)))->size_allocate (widget, &alloc_dummy) ;
  }

static void forall (GtkContainer *container, gboolean bInternals, GtkCallback callback, gpointer data)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (container) ;
  if (bInternals)
    if (NULL != private->btn) (*callback) (private->btn, data) ;

  if (private->bParentWidgetsVisible && GTK_WIDGET_REALIZED (GTK_WIDGET (container)))
    {
    GTK_CONTAINER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_FLEXI_COMBO)))->forall (container, TRUE, forall_callback, container) ;
    private->bParentWidgetsVisible = FALSE ;
    }

  if (NULL != GTK_BIN (container)->child)
    (*callback) (GTK_BIN (container)->child, data) ;
  }

static gboolean popup_key_release (GtkWidget *widget, GdkEventKey *event, gpointer data)
  {
  if (0 == event->state && GDK_Escape == event->keyval)
    qcad_flexi_combo_show_popup (QCAD_FLEXI_COMBO (data), FALSE) ;
  return FALSE ;
  }

static void arrow_btn_clicked (GtkWidget *widget, gpointer data)
  {
  GtkWidget *popup = QCAD_FLEXI_COMBO_GET_PRIVATE (data)->popup ;

  if (NULL != GTK_BIN (popup)->child)
    qcad_flexi_combo_show_popup (QCAD_FLEXI_COMBO (data), !GTK_WIDGET_VISIBLE (popup)) ;
  }

static void forall_callback (GtkWidget *widget, gpointer data)
  {
  QCADFlexiCombo *flexi_combo = QCAD_FLEXI_COMBO (data) ;
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (flexi_combo) ;

  if (private->btn == widget || GTK_BIN (flexi_combo)->child == widget) return ;
  if (GTK_TYPE_TOGGLE_BUTTON == G_TYPE_FROM_INSTANCE (widget)) private->combo_box_btn = widget ;
//  g_print ("forall_callback: Hiding widget %s\n", g_type_name (G_TYPE_FROM_INSTANCE (widget))) ;
  g_signal_connect (G_OBJECT (widget), "show", (GCallback)gtk_widget_hide, NULL) ;
  gtk_widget_hide (widget) ;
  }

static gboolean hide_grab_widget (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (data) ;
  int x, y, cx, cy ;

  gdk_window_get_position (private->popup->window, &x, &y) ;
  gdk_window_get_size (private->popup->window, &cx, &cy) ;

  if (event->x < x || event->x > cx || event->y < y || event->y > cy)
    qcad_flexi_combo_show_popup (QCAD_FLEXI_COMBO (data), FALSE) ;

  return FALSE ;
  }

void qcad_flexi_combo_show_popup (QCADFlexiCombo *flexi_combo, gboolean bShow)
  {
  QCADFlexiComboPrivate *private = QCAD_FLEXI_COMBO_GET_PRIVATE (flexi_combo) ;
//  GtkWidget *tv = gtk_bin_get_child (GTK_BIN (private->frm)) ;
  GtkWidget 
    *grab_widget = private->popup, 
    *popup_ref_widget = (NULL == (GTK_BIN (flexi_combo)->child) ? private->btn : GTK_BIN (flexi_combo)->child) ;

  if (NULL == GTK_BIN (private->popup)->child)
    bShow = FALSE ;

  if (bShow)
    {
    int x, y ;

    gtk_widget_get_root_origin (popup_ref_widget, &x, &y) ;
    y += popup_ref_widget->allocation.height + 1 ;
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
        g_signal_connect (G_OBJECT (grab_widget), "button-press-event", (GCallback)hide_grab_widget, flexi_combo) ;
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
