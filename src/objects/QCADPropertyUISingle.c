#include <stdarg.h>
#include "custom_widgets.h"
#include "support.h"
#include "QCADPropertyUISingle.h"

enum
  {
  QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL=1,
  QCAD_PROPERTY_UI_LAST
  } ;

static void qcad_property_ui_single_class_init (QCADPropertySingleUIClass *klass) ;
static void qcad_property_ui_single_instance_init (QCADPropertySingleUI *qcad_property_ui) ;

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
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
    if (0 != (qcad_property_single_ui_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI, QCAD_TYPE_STRING_PROPERTY_UI_SINGLE, &info, 0)))
      g_type_class_ref (qcad_property_ui_single_type) ;
    }
  return qcad_property_ui_single_type ;
  }

static void qcad_property_ui_single_class_init (QCADPropertyUISingleClass *klass)
  {
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec = set_pspec ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL,
    g_param_spec_boolean ("show-label", _("Show Label"), _("Display property label"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;
  }

static void qcad_property_ui_instance_init (QCADPropertyUISingle *property_ui)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;

  property_ui_single->bShowLbl   = TRUE ;
  property_ui_single->lbl.widget = gtk_label_new ("") ;
  gtk_widget_show (property_ui_single->lbl.widget) ;
  gtk_label_set_justify (GTK_LABEL (property_ui_single->lbl.widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (property_ui_single->lbl.widget), 1.0, 0.5) ;
  property_ui_single->lbl.idxX   = 0 ;
  property_ui_single->lbl.idxY   = 0 ;
  }

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
      psz = g_strdup_printf ("%s:", g_param_spec_get_nick (new_pspec)) ;
    }
  }
