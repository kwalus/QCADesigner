#include "../support.h"
#include "QCADToggleToolButton.h"

enum
  {
  QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_FIRST=1,

  QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE,

  QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_LAST
  } ;

static void class_init (QCADToggleToolButtonClass *klass, gpointer data) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

GType qcad_toggle_tool_button_get_type ()
  {
#if (GTK_MINOR_VERSION >= 10)
  return GTK_TYPE_TOGGLE_TOOL_BUTTON ;
#else
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADToggleToolButtonClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADToggleToolButton),
      0,
      (GInstanceInitFunc)NULL
      } ;

    if ((the_type = g_type_register_static (GTK_TYPE_TOGGLE_TOOL_BUTTON, QCAD_TYPE_STRING_TOGGLE_TOOL_BUTTON, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
#endif /* (GTK_MINOR_VERSION >= 10) */
  }

static void class_init (QCADToggleToolButtonClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

#if (GTK_MINOR_VERSION <= 4)
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE,
    g_param_spec_boolean ("active", _("Active"), _("Active"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
#else
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE,
    g_param_spec_boolean ("real-active", _("Active"), _("Active"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  g_object_class_override_property (G_OBJECT_CLASS (klass), QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE, "active") ;
#endif
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  if (QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE == property_id)
    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (object), g_value_get_boolean (value)) ;
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  if (QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE == property_id)
    g_value_set_boolean (value, gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (object))) ;
  }
