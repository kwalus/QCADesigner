#include "../support.h"
#include "QCADToggleToolButton.h"
/**
 * SECTION:QCADToggleToolButton
 * @short_description: QCADesigner workaround class for #GtkToggleToolButton
 *
 * #GtkToggleToolButton widgets in versions of GTK <= 2.8.0 did not have the "active" property until versions
 * > 2.6.0. Between those versions and versions <= 2.8.0 the "notify::active" signal was not emitted. The
 * property documented here as "workaround-active" will be implemented into as "active" if, at compile time,
 * GTK's version is found to be <= 2.6.0 and it will be overridden to ensure that the "notify::active" signal
 * is emitted if, at compile time, GTK's version is found to be <= 2.8.0.
 *
 * For versions of GTK > 2.8.0 detected at compile time, the definition of this widget evaluates to
 * #GtkToggleToolButton and none of the #QCADToggleToolButton code gets compiled.
 */
#if (GTK_MINOR_VERSION <= 8 || defined (GTK_DOC))
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
  }

static void class_init (QCADToggleToolButtonClass *klass, gpointer data)
  {
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

#if (GTK_MINOR_VERSION <= 6)
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE,
    g_param_spec_boolean (
#ifdef GTK_DOC
      "workaround-active", 
#else
      "active", 
#endif
      _("Active"), _("Active"),
      FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
#else
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_TOGGLE_TOOL_BUTTON_PROPERTY_ACTIVE,
    g_param_spec_boolean (
#ifdef GTK_DOC
      "workaround-active", 
#else
      "real-active", 
#endif
      _("Active"), _("Active"),
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
#endif
