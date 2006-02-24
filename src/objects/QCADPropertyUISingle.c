#include <stdarg.h>
#include <string.h>
#include "../custom_widgets.h"
#include "../support.h"
#include "QCADObject.h"
#include "QCADParamSpecObjectList.h"
#include "QCADPropertyUISingle.h"
#include "QCADPropertyUIInt.h"
#include "QCADPropertyUIEnum.h"
#include "QCADPropertyUIText.h"
#include "QCADPropertyUIDouble.h"
#include "QCADPropertyUIObjectList.h"
#include "QCADPropertyUIBoolean.h"

enum
  {
  QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL=1,
#ifdef GTK_GUI
  QCAD_PROPERTY_UI_SINGLE_PROPERTY_TOOLTIP,
#endif /* def GTK_GUI */
  QCAD_PROPERTY_UI_LAST
  } ;

static void qcad_property_ui_single_class_init (QCADPropertyUISingleClass *klass) ;
static void qcad_property_ui_single_instance_init (QCADPropertyUISingle *property_ui_single) ;

#ifdef GTK_GUI
static void finalize     (GObject *object) ;
#endif /* def GTK_GUI */
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
#ifdef GTK_GUI
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
#endif /* def GTK_GUI */

static void set_pspec (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec) ;
#ifdef GTK_GUI
static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;
#endif /* def GTK_GUI */

static void qcad_property_ui_single_do_behaviour (QCADPropertyUISingle *property_ui_single, GObject *instance, gboolean bDisconnect) ;
static void qcad_property_ui_single_do_properties (QCADPropertyUISingle *property_ui_single, GObject *instance) ;

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
  if (G_TYPE_STRING == value_type)
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_TEXT, NULL) ;
  else
  if (G_TYPE_DOUBLE == value_type || G_TYPE_FLOAT == value_type)
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_DOUBLE, NULL) ;
  else
  if (G_TYPE_BOOLEAN == value_type)
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_BOOLEAN, NULL) ;
  else
  if (QCAD_TYPE_OBJECT_LIST == value_type)
    pui = g_object_new (QCAD_TYPE_PROPERTY_UI_OBJECT_LIST, NULL) ;
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
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
#endif /* def GTK_GUI */
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;

#ifdef GTK_GUI
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
#endif /* def GTK_GUI */
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec   = set_pspec ;
#ifdef GTK_GUI
  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;
#endif /* def GTK_GUI */

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL,
    g_param_spec_boolean ("show-label", _("Show Label"), _("Display property label"),
      TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE)) ;

#ifdef GTK_GUI
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_SINGLE_PROPERTY_TOOLTIP,
    g_param_spec_pointer ("tooltip", _("Tooltip"), _("Tooltip"), G_PARAM_WRITABLE)) ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_single_instance_init (QCADPropertyUISingle *property_ui_single)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_single) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;

  property_ui_single->pspec      = NULL ;
  property_ui_single->bShowLbl   = TRUE ;
#ifdef GTK_GUI
  property_ui_single->lbl.widget = gtk_label_new ("") ;
  g_object_ref (G_OBJECT (property_ui_single->lbl.widget)) ;
  gtk_widget_show (property_ui_single->lbl.widget) ;
  gtk_label_set_justify (GTK_LABEL (property_ui_single->lbl.widget), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (property_ui_single->lbl.widget), 1.0, 0.5) ;
  property_ui_single->lbl.idxX   = 0 ;
  property_ui_single->lbl.idxY   = 0 ;

  property_ui_single->tooltip = NULL ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  g_object_unref (G_OBJECT (QCAD_PROPERTY_UI_SINGLE (object)->lbl.widget)) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_SINGLE)))->finalize (object) ;
  }
#endif /* def GTK_GUI */

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
#ifdef GTK_GUI
  QCADPropertyUI       *property_ui        = QCAD_PROPERTY_UI (object) ;
#endif /* def GTK_GUI */
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_SINGLE_PROPERTY_SHOW_LABEL:
      property_ui_single->bShowLbl = g_value_get_boolean (value) ;
#ifdef GTK_GUI
      GTK_WIDGET_SET_VISIBLE (property_ui_single->lbl.widget, property_ui->bVisible && property_ui_single->bShowLbl) ;
#endif /* def GTK_GUI */
      break ;

#ifdef GTK_GUI
    case QCAD_PROPERTY_UI_SINGLE_PROPERTY_TOOLTIP:
      {
      GtkTooltips *tooltip = g_value_get_pointer (value) ;

      if (GTK_IS_TOOLTIPS (tooltip))
        if (tooltip != property_ui_single->tooltip)
          {
          property_ui_single->tooltip = tooltip ;
          QCAD_PROPERTY_UI_SINGLE_GET_CLASS (property_ui)->set_tooltip (property_ui_single, tooltip) ;
          g_object_notify (object, "tooltip") ;
          }
      break ;
      }
#endif /* def GTK_GUI */
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
#ifdef GTK_GUI
    char *psz = NULL ;
#endif /* def GTK_GUI */
    g_param_spec_ref (new_pspec) ;
    property_ui_single->pspec = new_pspec ;
#ifdef GTK_GUI
    gtk_label_set_text (GTK_LABEL (property_ui_single->lbl.widget), 
      psz = g_strdup_printf ("%s:", g_param_spec_get_nick (new_pspec))) ;
    g_free (psz) ;
    if (NULL != property_ui_single->tooltip)
      QCAD_PROPERTY_UI_SINGLE_GET_CLASS (property_ui_single)->set_tooltip (property_ui_single, property_ui_single->tooltip) ;
#endif /* def GTK_GUI */
    }
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  bVisible = (bVisible && property_ui_single->bShowLbl) ;
#ifdef GTK_GUI
  GTK_WIDGET_SET_VISIBLE (property_ui_single->lbl.widget, bVisible) ;
#endif /* def GTK_GUI */
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_SINGLE)))->set_instance (property_ui, new_instance, old_instance))
    {
    if (NULL != old_instance)
      qcad_property_ui_single_do_behaviour (QCAD_PROPERTY_UI_SINGLE (property_ui), old_instance, TRUE) ;

    if (NULL != new_instance)
      {
      qcad_property_ui_single_do_behaviour (QCAD_PROPERTY_UI_SINGLE (property_ui), new_instance, FALSE) ;
      qcad_property_ui_single_do_properties (QCAD_PROPERTY_UI_SINGLE (property_ui), new_instance) ;
      }

    return TRUE ;
    }

  return FALSE ;
  }

#ifdef GTK_GUI
static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {gtk_widget_set_sensitive (QCAD_PROPERTY_UI_SINGLE (property_ui)->lbl.widget, bSensitive) ;}

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  (*col_span) = 1 ;
  return ((idxX == property_ui_single->lbl.idxX && idxY == property_ui_single->lbl.idxY)
    ? property_ui_single->lbl.widget 
    : NULL) ;
  }
#endif /* def GTK_GUI */

// This function is very similar to its QCADPropertyUIGroup cousin, except it assigns those behaviours which
// tie the properties of a QCADPropertyUISingle to the properties of an instance. It is unaware of other
// QCADPropertyUISingle objects.
static void qcad_property_ui_single_do_behaviour (QCADPropertyUISingle *property_ui_single, GObject *instance, gboolean bDisconnect)
  {
  // (Dis)connect the rules from the instance
  if (QCAD_IS_OBJECT (instance))
    {
    int Nix ;
    QCADObjectClass *klass = QCAD_OBJECT_GET_CLASS (instance) ;
    GObject *instance1 = NULL, *instance2 = NULL ;
    char *name1 = NULL, *name2 = NULL ;
    QCADPropertyUIBehaviour *behaviour = NULL ;

    if (NULL == klass->property_ui_behaviour) return ;

    for (Nix = 0 ; Nix < klass->property_ui_behaviour->icUsed ; Nix++)
      {
      instance1 = instance2 = NULL ;
      name1 = name2 = NULL ;

      behaviour = &exp_array_index_1d (klass->property_ui_behaviour, QCADPropertyUIBehaviour, Nix) ;

      if (NULL == behaviour->ui_property_name1 &&
          !(NULL == behaviour->instance_property_name1 ||
            NULL == behaviour->instance_property_name2 ||
            NULL == behaviour->ui_property_name2))
        {
        instance1 = instance ;
        if (!strcmp (property_ui_single->pspec->name, behaviour->instance_property_name2))
          instance2 = G_OBJECT (property_ui_single) ;
        name1 = behaviour->instance_property_name1 ;
        name2 = behaviour->ui_property_name2 ;
        }

      if (!(NULL == instance1 || NULL == instance2))
        {
        if (bDisconnect)
          disconnect_object_properties (instance1, name1, instance2, name2,
            behaviour->fn_forward, behaviour->data_forward, behaviour->destroy_forward,
            behaviour->fn_reverse, behaviour->data_reverse, behaviour->destroy_reverse) ;
        else
          {
          connect_object_properties (instance1, name1, instance2, name2,
            behaviour->fn_forward, behaviour->data_forward, behaviour->destroy_forward,
            behaviour->fn_reverse, behaviour->data_reverse, behaviour->destroy_reverse) ;
          g_object_notify (instance1, name1) ;
          g_object_notify (instance2, name2) ;
          }
        }
      }
    }
  }

static void qcad_property_ui_single_do_properties (QCADPropertyUISingle *property_ui_single, GObject *instance)
  {
  if (QCAD_IS_OBJECT (instance))
    {
    QCADObjectClass *klass = QCAD_OBJECT_GET_CLASS (instance) ;
    QCADPropertyUIProperty *puip = NULL ;
    int Nix ;

    if (NULL == klass->property_ui_properties) return ;

    for (Nix = 0 ; Nix < klass->property_ui_properties->icUsed ; Nix++)
      {
      puip = &exp_array_index_1d (klass->property_ui_properties, QCADPropertyUIProperty, Nix) ;
      if (!(NULL == puip->instance_property_name || NULL == puip->ui_property_name))
        if (!strcmp (puip->instance_property_name, property_ui_single->pspec->name))
          g_object_set_property (G_OBJECT (property_ui_single), puip->ui_property_name, &(puip->ui_property_value)) ;
      }
    }
  }

#ifdef GTK_GUI
static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {gtk_tooltips_set_tip (tooltip, property_ui_single->lbl.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;}
#endif /* def GTK_GUI */
