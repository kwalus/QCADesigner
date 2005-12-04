#include "support.h"
#include "../generic_utils.h"
#include "../custom_widgets.h"
#include "QCADPropertyUIInt.h"

static void qcad_property_ui_int_class_init (QCADPropertyUIIntClass *klass) ;
static void qcad_property_ui_int_instance_init (QCADPropertyUIInt *property_ui_int) ;

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance) ;
static void set_pspec (QCADPropertyUI *property_ui) ;
static void finalize (GObject *object) ;
static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY) ;

GType qcad_property_ui_int_get_type ()
  {
  static GType qcad_property_ui_int_type = 0 ;

  if (0 == qcad_property_ui_int_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIIntClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_int_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIInt),
      0,
      (GInstanceInitFunc)qcad_property_ui_int_instance_init
      } ;
    if (0 != (qcad_property_ui_int_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_NUMERIC, QCAD_TYPE_STRING_PROPERTY_UI_INT, &info, 0)))
      g_type_class_ref (qcad_property_ui_int_type) ;
    }
  return qcad_property_ui_int_type ;
  }

static void qcad_property_ui_int_class_init (QCADPropertyUIIntClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize = finalize ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_pspec     = set_pspec ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;
  }

static void qcad_property_ui_int_instance_init (QCADPropertyUIInt *property_ui_int)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_int) ;
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (property_ui_int) ;

  property_ui->cxWidgets  = 3 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

  property_ui_int->render_as           = GTK_TYPE_SPIN_BUTTON ;
  property_ui_int->spn.widget          = gtk_spin_button_new (property_ui_int->adj = 
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 0)), 1, 0) ;
  gtk_widget_show (property_ui_int->spn.widget) ;
  property_ui_int->spn.idxX            = 1 ;
  property_ui_int->spn.idxY            = 0 ;
  property_ui_numeric->lblUnits.widget = gtk_label_new ("") ;
  property_ui_numeric->lblUnits.idxX   = 2 ;
  property_ui_numeric->lblUnits.idxY   = 0 ;
  gtk_widget_show (property_ui_numeric->lblUnits.widget) ;

  property_ui_int->option_menu.widget = gtk_option_menu_new () ;
  property_ui_int->option_menu.idxX   = 1 ;
  property_ui_int->option_menu.idxY   = 0 ;
  gtk_widget_show (property_ui_int->option_menu.widget) ;
  }

static void finalize (GObject *object)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (object) ;

  g_object_unref (property_ui_int->spn.widget) ;
  g_object_unref (property_ui_int->option_menu.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->finalize (object) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;
  if (idxX == property_ui_int->spn.idxX && idxY == property_ui_int->spn.idxY)
    return property_ui_int->spn.widget ;
  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->get_widget (property_ui, idxX, idxY) ;
  }

static void set_pspec (QCADPropertyUI *property_ui)
  {
  char *psz = NULL ;
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  property_ui_int->adj->lower = ((GParamSpecInt *)(property_ui->pspec))->minimum ;
  property_ui_int->adj->upper = ((GParamSpecInt *)(property_ui->pspec))->maximum ;
  property_ui_int->adj->value = ((GParamSpecInt *)(property_ui->pspec))->default_value ;
  gtk_label_set_text (GTK_LABEL (property_ui->lbl.widget), 
    psz = g_strdup_printf ("%s:", g_param_spec_get_nick (property_ui->pspec))) ;
  g_free (psz) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  if (NULL != property_ui->pspec)
    {
    if (NULL != property_ui->instance)
      disconnect_object_properties (property_ui->instance, property_ui->pspec->name, G_OBJECT (property_ui_int->adj), "value", NULL, NULL, NULL, NULL, NULL, NULL) ;

    connect_object_properties (instance, property_ui->pspec->name, G_OBJECT (property_ui_int->adj), "value", NULL, NULL, NULL, NULL, NULL, NULL) ;
    g_object_notify (instance, property_ui->pspec->name) ;
    }

  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_instance (property_ui, instance) ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_visible (property_ui, bVisible) ;
  GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_INT (property_ui)->spn.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_INT (property_ui)->spn.widget, bSensitive) ;
  }
