#include <stdarg.h>
#include "custom_widgets.h"
#include "support.h"
#include "QCADPropertyUI.h"
#include "QCADPropertyUIInt.h"
#include "QCADParamSpecTypeList.h"

enum
  {
  QCAD_PROPERTY_UI_SENSITIVE = 1,
  QCAD_PROPERTY_UI_VISIBLE,
  QCAD_PROPERTY_UI_SHOW_LABEL,
  QCAD_PROPERTY_UI_LAST
  } ;

static void qcad_property_ui_class_init (QCADPropertyUIClass *klass) ;
static void qcad_property_ui_instance_init (QCADPropertyUI *qcad_property_ui) ;

static void finalize (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY) ;
static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance) ;
static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;

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
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget     = get_widget ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible    = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive  = set_sensitive ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SENSITIVE,
    g_param_spec_boolean ("sensitive", _("Sensitive"), _("Enable Property UI"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_VISIBLE,
    g_param_spec_boolean ("visible", _("Visible"), _("Show/Hide Property UI"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SHOW_LABEL,
    g_param_spec_boolean ("show-label", _("Show label"), _("Show property label"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;
  }

static void qcad_property_ui_instance_init (QCADPropertyUI *qcad_property_ui)
  {
  qcad_property_ui->pspec          = NULL ;
  qcad_property_ui->bSensitive     = TRUE ;
  qcad_property_ui->bVisible       = TRUE ;
  qcad_property_ui->bShowLbl       = TRUE ;
  qcad_property_ui->cxWidgets      = 1 ;
  qcad_property_ui->cyWidgets      = 1 ;

  qcad_property_ui->lbl.widget = gtk_label_new ("") ;
  gtk_widget_show (qcad_property_ui->lbl.widget) ;
  qcad_property_ui->lbl.idxX = 0 ;
  qcad_property_ui->lbl.idxY = 0 ;
  }

QCADPropertyUI *qcad_property_ui_new_from_instance (GObject *instance, char *property, ...)
  {
  char *pszFirstProp = NULL ;
  QCADPropertyUI *pui = NULL ;
  GParamSpec *pspec = NULL ;
  va_list va ;

  if (NULL == instance || NULL == property) return NULL ;
  if (NULL == (pspec = g_object_class_find_property (G_TYPE_INSTANCE_GET_CLASS (instance, G_TYPE_OBJECT, GObjectClass), property))) return NULL ;

  va_start (va, property) ;
  pszFirstProp = va_arg (va, char *) ;

  switch (G_PARAM_SPEC_VALUE_TYPE (pspec))
    {
    case G_TYPE_UINT:
    case G_TYPE_INT:
      pui = QCAD_PROPERTY_UI (g_object_new_valist (QCAD_TYPE_PROPERTY_UI_INT, pszFirstProp, va)) ;
      break ;

    default:
      g_warning ("qcad_property_ui_new: Cannot create property UI for type %s\n", g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec))) ;
      break ;
    }

  va_end (va) ;

  pui->pspec = g_param_spec_ref (pspec) ;
  if (NULL != QCAD_PROPERTY_UI_GET_CLASS (pui)->set_pspec)
    QCAD_PROPERTY_UI_GET_CLASS (pui)->set_pspec (pui) ;

  qcad_property_ui_set_instance (pui, instance) ;

  return pui ;
  }

void qcad_property_ui_set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  if (NULL != QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_instance)
    QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_instance (property_ui, instance) ;
  }

int qcad_property_ui_get_cx_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cxWidgets ;}

int qcad_property_ui_get_cy_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cxWidgets ;}

GtkWidget *qcad_property_ui_get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {return QCAD_PROPERTY_UI_GET_CLASS (property_ui)->get_widget (property_ui, idxX, idxY) ;}

static void finalize (GObject *object)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  if (NULL != property_ui->pspec)
    g_param_spec_unref (QCAD_PROPERTY_UI (object)->pspec) ;
  g_object_unref (property_ui->lbl.widget) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SENSITIVE:
      QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_sensitive (property_ui, property_ui->bSensitive = g_value_get_boolean (value)) ;
      break ;

    case QCAD_PROPERTY_UI_VISIBLE:
      QCAD_PROPERTY_UI_GET_CLASS (property_ui)->set_visible (property_ui, property_ui->bVisible = g_value_get_boolean (value)) ;
      break ;

    case QCAD_PROPERTY_UI_SHOW_LABEL:
      if (NULL != property_ui->lbl.widget)
        GTK_WIDGET_SET_VISIBLE (property_ui->lbl.widget, property_ui->bShowLbl = g_value_get_boolean (value)) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SENSITIVE:
      g_value_set_boolean (value, QCAD_PROPERTY_UI (object)->bSensitive) ;
      break ;

    case QCAD_PROPERTY_UI_VISIBLE:
      g_value_set_boolean (value, QCAD_PROPERTY_UI (object)->bVisible) ;
      break ;

    case QCAD_PROPERTY_UI_SHOW_LABEL:
      if (NULL != property_ui->lbl.widget)
        g_value_set_boolean (value, property_ui->bShowLbl) ;
      break ;
    }
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {return ((property_ui->lbl.idxX == idxX && property_ui->lbl.idxY == idxY) ? property_ui->lbl.widget : NULL) ;}

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {GTK_WIDGET_SET_VISIBLE (property_ui->lbl.widget, bVisible && property_ui->bShowLbl) ;}

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {gtk_widget_set_sensitive (property_ui->lbl.widget, bSensitive) ;}

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  property_ui->instance = instance ;
  g_object_weak_ref (instance, property_ui_instance_has_died, property_ui) ;
  return TRUE ;
  }

void property_ui_instance_has_died (gpointer data, GObject *dead_object)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (data) ;

  if (property_ui->instance == dead_object)
    property_ui->instance = NULL ;
  }
