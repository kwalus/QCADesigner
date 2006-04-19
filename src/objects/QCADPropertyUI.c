#include <stdarg.h>
#include "../custom_widgets.h"
#include "../support.h"
#include "QCADPropertyUI.h"
#include "QCADPropertyUIGroup.h"
#include "QCADPropertyUISingle.h"

/**
 * SECTION:QCADPropertyUI
 * @short_description: Base class for property UIs.
 *
 * A property UI is a collection of widgets bound to either a #GObject instance as a whole
 * (#QCADPropertyUIGroup), or to a specific property of the instance (#QCADPropertyUISingle). The widgets are 
 * arranged logically in a 2D grid whose dimensions can be retrieved with qcad_property_ui_get_cx_widgets() 
 * and qcad_property_ui_get_cy_widgets(). You can then retrieve the widgets one-by-one with
 * qcad_property_ui_get_widget().
 *
 * When bound to a #QCADObject instance, property UIs can behave more intelligently by applying the behaviour
 * and property hints stored within the instance's class as an array of #QCADPropertyUIBehaviour and
 * #QCADPropertyUIProperty entries, respectively.
 *
 * See also: #QCADObject
 */

enum
  {
  QCAD_PROPERTY_UI_SENSITIVE = 1,
  QCAD_PROPERTY_UI_VISIBLE,
  QCAD_PROPERTY_UI_LAST
  } ;

static void qcad_property_ui_class_init (QCADPropertyUIClass *klass) ;
static void qcad_property_ui_instance_init (QCADPropertyUI *qcad_property_ui) ;

static void finalize (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;

void property_ui_instance_has_died (gpointer data, GObject *dead_object) ;

GType qcad_property_ui_get_type ()
  {
  static GType qcad_property_ui_type = 0 ;

  if (0 == qcad_property_ui_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUI),
      0,
      (GInstanceInitFunc)qcad_property_ui_instance_init
      } ;
    if (0 != (qcad_property_ui_type = g_type_register_static (G_TYPE_OBJECT, QCAD_TYPE_STRING_PROPERTY_UI, &info, 0)))
      g_type_class_ref (qcad_property_ui_type) ;
    }
  return qcad_property_ui_type ;
  }

static void qcad_property_ui_class_init (QCADPropertyUIClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_instance   = set_instance ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SENSITIVE,
    g_param_spec_boolean ("sensitive", _("Sensitive"), _("Enable Property UI"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_VISIBLE,
    g_param_spec_boolean ("visible", _("Visible"), _("Show/Hide Property UI"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;
  }

static void qcad_property_ui_instance_init (QCADPropertyUI *property_ui)
  {
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;
  property_ui->cxWidgets  = 0 ;
  property_ui->cyWidgets  = 0 ;
  }

/**
 * qcad_property_ui_new:
 * @instance: Instance to create the UI for.
 * @property_name: Property of @instance to create the UI for.
 * @...: %NULL-terminated list of property name - property value pairs to set on the newly created property UI.
 *
 * Creates a new property UI for #GObject instance @instance. If @property_name is %NULL, it creates a
 * #QCADPropertyUIGroup covering all of @instance's properties. Otherwise, it creates a #QCADPropertyUISingle
 * covering only @property_name.
 *
 * See also: qcad_property_ui_single_newv(), qcad_property_ui_group_newv()
 *
 * Returns: A newly created #QCADPropertyUI.
 */
QCADPropertyUI *qcad_property_ui_new (GObject *instance, char *property_name, ...)
  {
  QCADPropertyUI *pui = NULL ;
  va_list va ;

  va_start (va, property_name) ;
  if (NULL == property_name)
    pui = qcad_property_ui_group_newv (instance, va) ;
  else
    pui = qcad_property_ui_single_newv (instance, property_name, va) ;
  va_end (va) ;

  return pui ;
  }

void qcad_property_ui_set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  if (NULL != QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_instance)
    QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_instance (property_ui, instance, property_ui->instance) ;
  }

int qcad_property_ui_get_cx_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cxWidgets ;}

int qcad_property_ui_get_cy_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cyWidgets ;}

#ifdef GTK_GUI
GtkWidget *qcad_property_ui_get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  GtkWidget *ret = NULL ;
  int the_col_span = 1 ;

  ret = QCAD_PROPERTY_UI_GET_CLASS (property_ui)->get_widget (property_ui, idxX, idxY, &the_col_span) ;
  if (NULL != col_span)
    (*col_span) = the_col_span ;

  return ret ;
  }
#endif /* def GTK_GUI */

static void finalize (GObject *object)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  if (NULL != property_ui->instance)
    {
//    fprintf (stderr, "%s::finalize::g_object_weak_unref (property_ui->instance<%s><0x%x>, property_ui_instance_has_died<0x%x>, property_ui<0x%x>) ;\n",
//      g_type_name (G_TYPE_FROM_INSTANCE (object)), g_type_name (G_TYPE_FROM_INSTANCE (property_ui->instance)),
//      (int)(property_ui->instance), (int)property_ui_instance_has_died, (int)property_ui) ;
    g_object_weak_unref (property_ui->instance, property_ui_instance_has_died, property_ui) ;
    }
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SENSITIVE:
      QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_sensitive (property_ui, property_ui->bSensitive = g_value_get_boolean (value)) ;
      g_object_notify (object, "sensitive") ;
      break ;

    case QCAD_PROPERTY_UI_VISIBLE:
      if (NULL == QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_visible)
        fprintf (stderr, "->set_visible member for type %s is NULL ?!\n", g_type_name (G_TYPE_FROM_INSTANCE (property_ui))) ;
      else
      QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_visible (property_ui, property_ui->bVisible = g_value_get_boolean (value)) ;
      g_object_notify (object, "visible") ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SENSITIVE:
      g_value_set_boolean (value, QCAD_PROPERTY_UI (object)->bSensitive) ;
      break ;

    case QCAD_PROPERTY_UI_VISIBLE:
      g_value_set_boolean (value, QCAD_PROPERTY_UI (object)->bVisible) ;
      break ;
    }
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  if (NULL != old_instance)
    {
//    fprintf (stderr, "%s::set_instance::g_object_weak_unref (old_instance<%s><0x%x>, property_ui_instance_has_died<0x%x>, property_ui<0x%x>) ;\n",
//      g_type_name (G_TYPE_FROM_INSTANCE (property_ui)), g_type_name (G_TYPE_FROM_INSTANCE (old_instance)),
//      (int)old_instance, (int)property_ui_instance_has_died, (int)property_ui) ;
    g_object_weak_unref (old_instance, property_ui_instance_has_died, property_ui) ;
    property_ui->instance = NULL ;
    }

  if (NULL != new_instance)
    {
//    fprintf (stderr, "%s::set_instance::g_object_weak_ref (new_instance<%s><0x%x>, property_ui_instance_has_died<0x%x>, property_ui<0x%x>) ;\n",
//      g_type_name (G_TYPE_FROM_INSTANCE (property_ui)), g_type_name (G_TYPE_FROM_INSTANCE (new_instance)),
//      (int)new_instance, (int)property_ui_instance_has_died, (int)property_ui) ;
    g_object_weak_ref (new_instance, property_ui_instance_has_died, property_ui) ;
    property_ui->instance = new_instance ;
    }
  return TRUE ;
  }

void property_ui_instance_has_died (gpointer data, GObject *dead_object)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (data) ;

  if (property_ui->instance == dead_object)
    property_ui->instance = NULL ;
  }
