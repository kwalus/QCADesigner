#include "../custom_widgets.h"
#include "QCADPropertyUIBoolean.h"

static void qcad_property_ui_boolean_class_init (QCADPropertyUIBooleanClass *klass) ;
static void qcad_property_ui_boolean_instance_init (QCADPropertyUIBoolean *property_ui_boolean) ;

#ifdef GTK_GUI
static void finalize (GObject *object) ;
static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec) ;

static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_boolean_get_type ()
  {
  static GType qcad_property_ui_boolean_type = 0 ;

  if (0 == qcad_property_ui_boolean_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIBooleanClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_boolean_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIBoolean),
      0,
      (GInstanceInitFunc)qcad_property_ui_boolean_instance_init
      } ;

    if (0 != (qcad_property_ui_boolean_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_BOOLEAN, &info, 0)))
      g_type_class_ref (qcad_property_ui_boolean_type) ;
    }

  return qcad_property_ui_boolean_type ;
  }

static void qcad_property_ui_boolean_class_init (QCADPropertyUIBooleanClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize = finalize ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec = set_pspec ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_boolean_instance_init (QCADPropertyUIBoolean *property_ui_boolean)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_boolean) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

#ifdef GTK_GUI
  gtk_widget_hide (QCAD_PROPERTY_UI_SINGLE (property_ui_boolean)->lbl.widget) ;

  property_ui_boolean->check_button.widget = gtk_check_button_new () ;
  g_object_ref (property_ui_boolean->check_button.widget) ;
  gtk_widget_show (property_ui_boolean->check_button.widget) ;
  property_ui_boolean->check_button.idxX = 0 ;
  property_ui_boolean->check_button.idxY = 0 ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (object) ;

  g_object_unref (property_ui_boolean->check_button.widget) ;
  }

static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec)
  {gtk_button_set_label (GTK_BUTTON (QCAD_PROPERTY_UI_BOOLEAN (property_ui)->check_button.widget), g_param_spec_get_nick (new_pspec)) ;}

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  QCADPropertyUIBoolean *property_ui_boolean  = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;
  QCADPropertyUISingle  *property_ui_single   = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_instance (property_ui, new_instance, old_instance))
    {
    if (NULL != old_instance)
      disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->check_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

    if (NULL != new_instance)
      {
      connect_object_properties (new_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->check_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

      g_object_notify (new_instance, property_ui_single->pspec->name) ;
      }
    return TRUE ;
    }
  return FALSE ;
  }

// No point calling the parent class' methods because we've suppressed its widgets
static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_BOOLEAN (property_ui)->check_button.widget, bVisible) ;}

// No point calling the parent class' methods because we've suppressed its widgets
static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {gtk_widget_set_sensitive (QCAD_PROPERTY_UI_BOOLEAN (property_ui)->check_button.widget, bSensitive) ;}

// No point calling the parent class' methods because we've suppressed its widgets
static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;

  (*col_span) = -1 ;
  if (property_ui_boolean->check_button.idxX == idxX && property_ui_boolean->check_button.idxY == idxY)
    return property_ui_boolean->check_button.widget ;

  return NULL ;
  }
#endif /* def GTK_GUI */
