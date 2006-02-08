#include <string.h>
#include "../custom_widgets.h"
#include "../support.h"
#include "QCADPropertyUINumeric.h"

enum
  {
  QCAD_PROPERTY_UI_NUMERIC_UNITS = 1,
  QCAD_PROPERTY_UI_NUMERIC_LAST
  } ;

static void qcad_property_ui_numeric_class_init (QCADPropertyUINumericClass *klass) ;
static void qcad_property_ui_numeric_instance_init (QCADPropertyUINumeric *property_ui_numeric) ;

#ifdef GTK_GUI
static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_numeric_get_type ()
  {
  static GType qcad_property_ui_numeric_type = 0 ;

  if (0 == qcad_property_ui_numeric_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUINumericClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_numeric_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUINumeric),
      0,
      (GInstanceInitFunc)qcad_property_ui_numeric_instance_init
      } ;
    if (0 != (qcad_property_ui_numeric_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_NUMERIC, &info, 0)))
      g_type_class_ref (qcad_property_ui_numeric_type) ;
    }
  return qcad_property_ui_numeric_type ;
  }

static void qcad_property_ui_numeric_class_init (QCADPropertyUINumericClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;

  QCAD_PROPERTY_UI_CLASS (klass)->get_widget  = get_widget ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;
#endif /* def GTK_GUI */

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_NUMERIC_UNITS,
    g_param_spec_string ("units", _("Units"), _("Property units"),
      "", G_PARAM_WRITABLE | G_PARAM_READABLE)) ;
  }

static void qcad_property_ui_numeric_instance_init (QCADPropertyUINumeric *property_ui_numeric)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_numeric) ;

#ifdef GTK_GUI
  property_ui_numeric->lblUnits.widget = gtk_label_new ("") ;
  g_object_ref (G_OBJECT (property_ui_numeric->lblUnits.widget)) ;
  gtk_widget_show (property_ui_numeric->lblUnits.widget) ;
  property_ui_numeric->lblUnits.idxX = 1 ;
  property_ui_numeric->lblUnits.idxY = 0 ;
  gtk_label_set_justify (GTK_LABEL (property_ui_numeric->lblUnits.widget), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (property_ui_numeric->lblUnits.widget), 0.0, 0.5) ;
#endif /* def GTK_GUI */
  property_ui->cxWidgets = 2 ;
  property_ui->cyWidgets = 1 ;
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (object) ;

  g_object_unref (property_ui_numeric->lblUnits.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_NUMERIC)))->finalize (object) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  gboolean bHadText = FALSE ;
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_NUMERIC_UNITS:
      if (NULL != property_ui_numeric->lblUnits.widget)
        {
        char *psz = NULL ;

        bHadText = (strlen (gtk_label_get_text (GTK_LABEL (property_ui_numeric->lblUnits.widget))) > 0) ;
        gtk_label_set_markup (GTK_LABEL (property_ui_numeric->lblUnits.widget), psz = (char *)g_value_get_string (value)) ;
        if (!bHadText && strlen (psz) > 0)
          (QCAD_PROPERTY_UI (object)->cxWidgets)++ ;
        else
        if (bHadText && strlen (psz) <= 0)
          (QCAD_PROPERTY_UI (object)->cxWidgets)-- ;
        }
      break ;
    }
  }
static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_NUMERIC_UNITS:
      if (NULL != property_ui_numeric->lblUnits.widget)
        g_value_set_string (value, gtk_label_get_text (GTK_LABEL (property_ui_numeric->lblUnits.widget))) ;
      break ;
    }
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_NUMERIC)))->set_visible (property_ui, bVisible) ;
  GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_NUMERIC (property_ui)->lblUnits.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_NUMERIC)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_NUMERIC (property_ui)->lblUnits.widget, bSensitive) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (property_ui) ;

  (*col_span) = 1 ;
  if ((idxY == property_ui_numeric->lblUnits.idxY) && 
      (property_ui_numeric->lblUnits.idxX == idxX) && 
      (strlen (gtk_label_get_text (GTK_LABEL (property_ui_numeric->lblUnits.widget))) > 0))
    return QCAD_PROPERTY_UI_NUMERIC (property_ui)->lblUnits.widget ;
  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_NUMERIC)))->get_widget (property_ui, idxX, idxY, col_span) ;
  }

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (property_ui_single) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_NUMERIC)))->set_tooltip (property_ui_single, tooltip) ;

  if (NULL != property_ui_single->pspec)
    gtk_tooltips_set_tip (tooltip, property_ui_numeric->lblUnits.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
  }
#endif /* def GTK_GUI */
