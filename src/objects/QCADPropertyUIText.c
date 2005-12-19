#include <stdarg.h>
#include "custom_widgets.h"
#include "../support.h"
#include "QCADPropertyUISingle.h"
#include "QCADPropertyUIInt.h"
#include "QCADPropertyUIEnum.h"

enum
  {
  QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL=1,
  QCAD_PROPERTY_UI_LAST
  } ;

static void qcad_property_ui_single_class_init (QCADPropertyUISingleClass *klass) ;
static void qcad_property_ui_single_instance_init (QCADPropertyUISingle *qcad_property_ui) ;

static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY) ;

static void set_pspec (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec) ;

GType qcad_property_ui_single_get_type ()
  {
  static GType qcad_property_ui_single_type = 0 ;

  if (0 == qcad_property_ui_single_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUISingleClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_single_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUISingle),
      0,
      (GInstanceInitFunc)qcad_property_ui_single_instance_init
      } ;
    if (0 != (qcad_property_ui_single_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI, QCAD_TYPE_STRING_PROPERTY_UI_SINGLE, &info, 0)))
      g_type_class_ref (qcad_property_ui_single_type) ;
    }
  return qcad_property_ui_single_type ;
  }

QCADPropertyUI *qcad_property_ui_single_new (GObject *instance, char *property, ...)
  {
  GType value_type = 0 ;
  QCADPropertyUI *pui = NULL ;
  GParamSpec *pspec = NULL ;
  va_list va ;
  char *pszFirstProperty = NULL ;

  if (NULL == instance) return NULL ;
  if (NULL == property) return NULL ;
  if (NULL == (pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (instance), property))) return NULL ;
  if (0 == (value_type = G_PARAM_SPEC_VALUE_TYPE (pspec))) return NULL ;

  if (G_TYPE_INT == value_type || G_TYPE_UINT == value_type)
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_INT, NULL) ;
  else
  if (G_IS_ENUM_CLASS (g_type_class_peek (value_type)))
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_ENUM, NULL) ;
  else
    g_warning ("Cannot create property UI for property \"%s\" of instance 0x%08X of type \"%s\"\n", property, (int)instance, g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec))) ;
//    fprintf (stderr, "Warning: Cannot create property UI for property \"%s\" of instance 0x%08X of type \"%s\"\n", property, (int)instance, g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec))) ;

  if (NULL != pui)
    {
    va_start (va, property) ;
    if (NULL != (pszFirstProperty = va_arg (va, char *)))
      g_object_set_valist (G_OBJECT (pui), pszFirstProperty, va) ;
    va_end (va) ;

    QCAD_PROPERTY_UI_SINGLE_GET_CLASS (pui)->set_pspec (QCAD_PROPERTY_UI_SINGLE (pui), pspec) ;
    qcad_property_ui_set_instance (pui, instance) ;
    }

  return pui ;
  }

static void qcad_property_ui_single_class_init (QCADPropertyUISingleClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;

  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec = set_pspec ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL,
    g_param_spec_boolean ("show-label", _("Show Label"), _("Display property label"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;
  }

static void qcad_property_ui_single_instance_init (QCADPropertyUISingle *property_ui_single)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_single) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;

  property_ui_single->pspec      = NULL ;
  property_ui_single->bShowLbl   = TRUE ;

  property_ui_single->lbl.widget = gtk_label_new ("") ;
  g_object_ref (G_OBJECT (property_ui_single->lbl.widget)) ;
  gtk_widget_show (property_ui_single->lbl.widget) ;
  gtk_label_set_justify (GTK_LABEL (property_ui_single->lbl.widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (property_ui_single->lbl.widget), 1.0, 0.5) ;
  property_ui_single->lbl.idxX   = 0 ;
  property_ui_single->lbl.idxY   = 0 ;
  }

static void finalize (GObject *object)
  {g_object_unref (G_OBJECT (QCAD_PROPERTY_UI_SINGLE (object)->lbl.widget)) ;}

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUI       *property_ui        = QCAD_PROPERTY_UI (object) ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL:
      property_ui_single->bShowLbl = g_value_get_boolean (value) ;
      GTK_WIDGET_SET_VISIBLE (property_ui_single->lbl.widget, property_ui->bVisible && property_ui_single->bShowLbl) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL:
      g_value_set_boolean (value, QCAD_PROPERTY_UI_SINGLE (object)->bShowLbl) ;
      break ;
    }
  }

static void set_pspec (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec)
  {
  if (NULL != property_ui_single->pspec)
    g_param_spec_unref (property_ui_single->pspec) ;
  if (NULL != new_pspec)
    {
    char *psz = NULL ;

    g_param_spec_ref (new_pspec) ;
    property_ui_single->pspec = new_pspec ;
    gtk_label_set_text (GTK_LABEL (property_ui_single->lbl.widget), 
      psz = g_strdup_printf ("%s:", g_param_spec_get_nick (new_pspec))) ;
    g_free (psz) ;
    }
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  bVisible = (bVisible && property_ui_single->bShowLbl) ;

  GTK_WIDGET_SET_VISIBLE (property_ui_single->lbl.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {gtk_widget_set_sensitive (QCAD_PROPERTY_UI_SINGLE (property_ui)->lbl.widget, bSensitive) ;}

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  return ((idxX == property_ui_single->lbl.idxX && idxY == property_ui_single->lbl.idxY)
    ? property_ui_single->lbl.widget 
    : NULL) ;
  }
