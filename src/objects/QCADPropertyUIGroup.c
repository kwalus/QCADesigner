#include <stdarg.h>
#include "custom_widgets.h"
#include "support.h"
#include "QCADPropertyUI.h"

typedef struct
  {
  char *pszPropName ;
  QCADPropertyUI *property_ui ;
  } QCADPropertyUIGroupEntry ;

static void qcad_property_ui_group_class_init (QCADPropertyUIGroupClass *klass) ;

static int get_cx_widgets (QCADPropertyUI *property_ui) ;
static int get_cy_widgets (QCADPropertyUI *property_ui) ;
static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY) ;
static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance) ;

static void qcad_property_ui_group_create_from_instance (QCADPropertyUIGroup *property_ui_group, GObject *instance) ;

GType qcad_property_ui_group_get_type ()
  {
  static GType qcad_property_ui_group_type = 0 ;

  if (0 == qcad_property_ui_group_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIGroupClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_group_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIGroup),
      0,
      NULL
      } ;
    if (0 != (qcad_property_ui_group_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI, QCAD_TYPE_STRING_PROPERTY_UI_GROUP, &info, 0)))
      g_type_class_ref (qcad_property_ui_group_type) ;
    }
  return qcad_property_ui_group_type ;
  }

static void qcad_property_ui_group_class_init (QCADPropertyUIClass *klass)
  {
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance   = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_cx_widgets = get_cx_widgets ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_cy_widgets = get_cy_widgets ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget     = get_widget ;
  }

static int get_cx_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cxWidgets ;}

static int get_cy_widgets (QCADPropertyUI *property_ui)
  {return property_ui->cyWidgets ;}

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY)
  {return exp_array_index_1d (property_ui->widgets, GtkWidget *, idxY * property_ui->cxWidgets + idxX) ;}

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *instance)
  {
  if (NULL == property_ui->instance)
    qcad_property_ui_group_create_from_instance (QCAD_PROPERTY_UI_GROUP (property_ui), instance) ;
  return TRUE ;
  }

static void qcad_property_ui_group_create_from_instance (QCADPropertyUIGroup *property_ui_group, GObject *instance)
  {
  int Nix ;
  GParamSpec *param_specs = NULL ;
  int icParamSpecs = -1 ;
  QCADPropertyUIGroupEntry puig_entry = {NULL, NULL} ;

  property_ui_group->property_uis = exp_array_new (sizeof (QCADPropertyUIGroupEntry), 1) ;

  g_object_class_list_properties (G_OBJECT_GET_CLASS (instance), &icParamSpecs) ;

  for (Nix = 0 ; Nix < icParamSpecs ; Nix++)
    if (NULL != (puig_entry.property_ui = qcad_property_ui_new_from_instance (instance, param_specs[Nix]->name, NULL)))
      {
      puig_entry.pszPropName = g_strdup (param_specs[Nix]->name) ;
      exp_array_insert_vals (property_ui_group->property_uis, &puig_entry, 1, 1, -1) ;
      }
  }
