#include <string.h>
#include "../support.h"
#include "../generic_utils.h"
#include "../custom_widgets.h"
#include "QCADPropertyUIDouble.h"

static void qcad_property_ui_double_class_init (QCADPropertyUIDoubleClass *klass) ;
static void qcad_property_ui_double_instance_init (QCADPropertyUIDouble *property_ui_double) ;

#ifdef GTK_GUI
static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *instance) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;

static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec) ;
#endif /* def GTK_GUI */

enum
  {
  QCAD_PROPERTY_UI_DOUBLE_PROPERTY_DIGITS=1,
  QCAD_PROPERTY_UI_DOUBLE_PROPERTY_STEP_INCREMENT
  } ;

GType qcad_property_ui_double_get_type ()
  {
  static GType qcad_property_ui_double_type = 0 ;

  if (0 == qcad_property_ui_double_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIDoubleClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_double_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIDouble),
      0,
      (GInstanceInitFunc)qcad_property_ui_double_instance_init
      } ;
    if (0 != (qcad_property_ui_double_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_NUMERIC, QCAD_TYPE_STRING_PROPERTY_UI_DOUBLE, &info, 0)))
      g_type_class_ref (qcad_property_ui_double_type) ;
    }
  return qcad_property_ui_double_type ;
  }

static void qcad_property_ui_double_class_init (QCADPropertyUIDoubleClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec = set_pspec ;
#endif /* def GTK_GUI */

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_DOUBLE_PROPERTY_DIGITS,
    g_param_spec_uint ("digits", _("Digits"), _("Number of decimal digits to display"),
      0, 10, 2, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_DOUBLE_PROPERTY_STEP_INCREMENT,
    g_param_spec_double ("step-increment", _("Increment"), _("Amount to modify value by"),
      0, G_MAXDOUBLE, 0.01, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_property_ui_double_instance_init (QCADPropertyUIDouble *property_ui_double)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_double) ;
#ifdef GTK_GUI
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (property_ui_double) ;
#endif /* def GTK_GUI */

  property_ui->cxWidgets  = 2 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

#ifdef GTK_GUI
  property_ui_double->spn.widget = gtk_spin_button_new (property_ui_double->adj = 
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 0.01, 1, 0)), 1, 2) ;
  g_object_ref (G_OBJECT (property_ui_double->spn.widget)) ;
  gtk_widget_show (property_ui_double->spn.widget) ;
  gtk_entry_set_activates_default (GTK_ENTRY (property_ui_double->spn.widget), TRUE) ;
  property_ui_double->spn.idxX   = 1 ;
  property_ui_double->spn.idxY   = 0 ;
  property_ui_numeric->lblUnits.idxX = 2 ;
  property_ui_numeric->lblUnits.idxY = 0 ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  set_instance (QCAD_PROPERTY_UI (object), NULL) ;

  g_object_unref (QCAD_PROPERTY_UI_DOUBLE (object)->spn.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->finalize (object) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_DOUBLE_PROPERTY_DIGITS:
      g_object_set_property (G_OBJECT (QCAD_PROPERTY_UI_DOUBLE (object)->spn.widget), "digits", value) ;
      break ;

    case QCAD_PROPERTY_UI_DOUBLE_PROPERTY_STEP_INCREMENT:
      g_object_set_property (G_OBJECT (QCAD_PROPERTY_UI_DOUBLE (object)->adj), "step-increment", value) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_DOUBLE_PROPERTY_DIGITS:
      g_object_get_property (G_OBJECT (QCAD_PROPERTY_UI_DOUBLE (object)->spn.widget), "digits", value) ;
      break ;

    case QCAD_PROPERTY_UI_DOUBLE_PROPERTY_STEP_INCREMENT:
      g_object_get_property (G_OBJECT (QCAD_PROPERTY_UI_DOUBLE (object)->adj), "step-increment", value) ;
      break ;
    }
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {
  QCADPropertyUIDouble *property_ui_double = QCAD_PROPERTY_UI_DOUBLE (property_ui) ;

  if (idxX == property_ui_double->spn.idxX && idxY == property_ui_double->spn.idxY)
    return property_ui_double->spn.widget ;

  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->get_widget (property_ui, idxX, idxY) ;
  }

static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec)
  {
  QCADPropertyUIDouble *property_ui_double = QCAD_PROPERTY_UI_DOUBLE (property_ui) ;

  if (G_TYPE_DOUBLE == G_PARAM_SPEC_VALUE_TYPE (new_pspec))
    {
    property_ui_double->adj->lower = ((GParamSpecDouble *)(new_pspec))->minimum ;
    property_ui_double->adj->upper = ((GParamSpecDouble *)(new_pspec))->maximum ;
    property_ui_double->adj->value = ((GParamSpecDouble *)(new_pspec))->default_value ;
    }
  else
  if (G_TYPE_FLOAT == G_PARAM_SPEC_VALUE_TYPE (new_pspec))
    {
    property_ui_double->adj->lower = ((GParamSpecFloat *)(new_pspec))->minimum ;
    property_ui_double->adj->upper = ((GParamSpecFloat *)(new_pspec))->maximum ;
    property_ui_double->adj->value = ((GParamSpecFloat *)(new_pspec))->default_value ;
    }

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->set_pspec (property_ui, new_pspec) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  GObject *old_instance = property_ui->instance ;
  QCADPropertyUIDouble *property_ui_double = QCAD_PROPERTY_UI_DOUBLE (property_ui) ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->set_instance (property_ui, instance))
    if (NULL != property_ui_single->pspec)
      {
      if (NULL != old_instance)
        disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_double->adj), "value", NULL, NULL, NULL, NULL, NULL, NULL) ;

      if (NULL != instance)
        {
        connect_object_properties (instance, property_ui_single->pspec->name, G_OBJECT (property_ui_double->adj), "value", 
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
        g_object_notify (instance, property_ui_single->pspec->name) ;
        }
      return TRUE ;
      }
  return FALSE ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->set_visible (property_ui, bVisible) ;
  GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_DOUBLE (property_ui)->spn.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_DOUBLE)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_DOUBLE (property_ui)->spn.widget, bSensitive) ;
  }
#endif /* def GTK_GUI */
