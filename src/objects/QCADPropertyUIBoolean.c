#include <string.h>
#include "../support.h"
#include "../custom_widgets.h"
#include "QCADParamSpecTypeList.h"
#include "QCADPropertyUIBoolean.h"
#ifdef GTK_GUI
  #include "QCADToggleToolButton.h"

enum
  {
  QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_RENDER_AS=1,
  QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_UP,
  QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_DOWN,
  QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_LAST
  } ;
#endif /* def GTK_GUI */

static void qcad_property_ui_boolean_class_init (QCADPropertyUIBooleanClass *klass) ;
static void qcad_property_ui_boolean_instance_init (QCADPropertyUIBoolean *property_ui_boolean) ;

#ifdef GTK_GUI
static void finalize (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void set_pspec   (QCADPropertyUISingle *property_ui,        GParamSpec *new_pspec) ;
static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;

static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;

static gboolean set_stock_id (QCADPropertyUIBoolean *property_ui_boolean, char **ppszStockID, const char *pszVal) ;
static void toggle_button_toggled (GtkWidget *widget, gpointer data) ;
static void qcad_property_ui_boolean_set_button_appearance (QCADPropertyUIBoolean *property_ui_boolean, gboolean bActive) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_boolean_get_type ()
  {
  static GType qcad_property_ui_boolean_type = 0 ;

  if (0 == qcad_property_ui_boolean_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIBooleanClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_boolean_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIBoolean),
      0,
      (GInstanceInitFunc)qcad_property_ui_boolean_instance_init
      } ;

    if (0 != (qcad_property_ui_boolean_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_BOOLEAN, &info, 0)))
      g_type_class_ref (qcad_property_ui_boolean_type) ;
    }

  return qcad_property_ui_boolean_type ;
  }

static void qcad_property_ui_boolean_class_init (QCADPropertyUIBooleanClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->finalize = finalize ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec   = set_pspec ;
  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;

  /**
   * QCADPropertyUIBoolean:render-as:
   *
   * This property UI can be rendered as either a #GtkCheckButton (default) or a #GtkToolItem (a 
   * #GtkToggleToolButton).
   *
   * Valid values: %GTK_TYPE_CHECK_BUTTON, %GTK_TYPE_TOOL_ITEM
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_RENDER_AS,
    qcad_param_spec_type_list ("render-as", _("Render As"), _("Render as widget"),
      GTK_TYPE_CHECK_BUTTON, G_PARAM_READABLE | G_PARAM_WRITABLE, GTK_TYPE_CHECK_BUTTON, GTK_TYPE_TOOL_ITEM, 0)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_UP,
    g_param_spec_string ("stock-up", _("Stock (Unchecked)"), _("Icon to display when unchecked"),
      NULL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_DOWN,
    g_param_spec_string ("stock-down", _("Stock (Checked)"), _("Icon to display when checked"),
      NULL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_boolean_instance_init (QCADPropertyUIBoolean *property_ui_boolean)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_boolean) ;

  property_ui->cxWidgets  = 1 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

#ifdef GTK_GUI
  property_ui_boolean->render_as = GTK_TYPE_CHECK_BUTTON ;

  property_ui_boolean->check_button.widget = gtk_check_button_new () ;
  g_object_ref (property_ui_boolean->check_button.widget) ;
  gtk_widget_show (property_ui_boolean->check_button.widget) ;
  property_ui_boolean->check_button.idxX = 0 ;
  property_ui_boolean->check_button.idxY = 0 ;

  property_ui_boolean->toggle_button.widget = GTK_WIDGET (g_object_new (QCAD_TYPE_TOGGLE_TOOL_BUTTON, NULL)) ;
  g_object_ref (property_ui_boolean->toggle_button.widget) ;
  gtk_widget_show (property_ui_boolean->toggle_button.widget) ;
  property_ui_boolean->toggle_button.idxX = 0 ;
  property_ui_boolean->toggle_button.idxY = 0 ;
  g_signal_connect (G_OBJECT (property_ui_boolean->toggle_button.widget), "toggled", (GCallback)toggle_button_toggled, property_ui_boolean) ;

//  property_ui_boolean->notify_id = -1 ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (object) ;

  set_instance (property_ui, NULL, property_ui->instance) ;

  g_object_unref (property_ui_boolean->check_button.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->finalize (object) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (object) ;
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_RENDER_AS:
      {
      GType new_render_as = (GType)g_value_get_uint (value) ;

      if (new_render_as == property_ui_boolean->render_as) break ;
      property_ui_boolean->render_as = new_render_as ;
      set_visible (property_ui, property_ui->bVisible) ;
      set_sensitive (property_ui, property_ui->bSensitive) ;
      g_object_notify (object, "render-as") ;
      break ;
      }

    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_UP:
      if (set_stock_id (property_ui_boolean, &(property_ui_boolean->pszStockUp), g_value_get_string (value)))
        g_object_notify (object, "stock-up") ;
      break ;

    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_DOWN:
      if (set_stock_id (property_ui_boolean, &(property_ui_boolean->pszStockDown), g_value_get_string (value)))
        g_object_notify (object, "stock-down") ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_RENDER_AS:
      g_value_set_uint (value, (guint)(property_ui_boolean->render_as)) ;
      break ;

    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_UP:
      g_value_set_string (value, property_ui_boolean->pszStockUp) ;
      break ;

    case QCAD_PROPERTY_UI_BOOLEAN_PROPERTY_STOCK_DOWN:
      g_value_set_string (value, property_ui_boolean->pszStockDown) ;
      break ;
    }
  }

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (property_ui_single) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_tooltip (property_ui_single, tooltip) ;

  if (NULL != property_ui_single->pspec)
    {
    gtk_tooltips_set_tip (tooltip, property_ui_boolean->check_button.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (property_ui_boolean->toggle_button.widget), tooltip, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    }
  }

static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_pspec (property_ui, new_pspec) ;
  gtk_button_set_label (GTK_BUTTON (property_ui_boolean->check_button.widget), g_param_spec_get_nick (new_pspec)) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  QCADPropertyUIBoolean *property_ui_boolean  = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;
  QCADPropertyUISingle  *property_ui_single   = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_instance (property_ui, new_instance, old_instance))
    {
    if (NULL != old_instance)
      {
      disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->check_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
      disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->toggle_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
      }

    if (NULL != new_instance)
      {
//      gboolean bActive ;

      connect_object_properties (new_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->check_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
      connect_object_properties (new_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_boolean->toggle_button.widget), "active",
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

//      g_object_get (new_instance, property_ui_single->pspec->name, &bActive, NULL) ;
//      qcad_property_ui_boolean_set_button_appearance (property_ui_boolean, bActive) ;

      g_object_notify (new_instance, property_ui_single->pspec->name) ;
      }
    return TRUE ;
    }
  return FALSE ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;

  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_visible (property_ui, FALSE) ;
  if (GTK_TYPE_TOOL_ITEM == property_ui_boolean->render_as)
    {
    GTK_WIDGET_SET_VISIBLE (property_ui_boolean->check_button.widget, FALSE) ;
    GTK_WIDGET_SET_VISIBLE (property_ui_boolean->toggle_button.widget, bVisible) ;
    }
  else
  if (GTK_TYPE_CHECK_BUTTON == property_ui_boolean->render_as)
    {
    GTK_WIDGET_SET_VISIBLE (property_ui_boolean->check_button.widget, bVisible) ;
    GTK_WIDGET_SET_VISIBLE (property_ui_boolean->toggle_button.widget, FALSE) ;
    }
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_BOOLEAN (property_ui)->check_button.widget, bSensitive) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIBoolean *property_ui_boolean = QCAD_PROPERTY_UI_BOOLEAN (property_ui) ;

  if (GTK_TYPE_CHECK_BUTTON == property_ui_boolean->render_as)
    {
    (*col_span) = -1 ;
    if (property_ui_boolean->check_button.idxX == idxX && property_ui_boolean->check_button.idxY == idxY)
      return property_ui_boolean->check_button.widget ;
    }
  else
  if (GTK_TYPE_TOOL_ITEM == property_ui_boolean->render_as)
    {
    (*col_span) = 1 ;
    if (property_ui_boolean->toggle_button.idxX == idxX && property_ui_boolean->toggle_button.idxY == idxY)
      return property_ui_boolean->toggle_button.widget ;
    else
      return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_BOOLEAN)))->get_widget (property_ui, idxX, idxY, col_span) ;
    }
  return NULL ;
  }

static gboolean set_stock_id (QCADPropertyUIBoolean *property_ui_boolean, char **ppszStockID, const char *pszVal)
  {
  GObject *instance = NULL ;
  GParamSpec *pspec = NULL ;

  if (NULL == (*ppszStockID))
    {
    if (NULL == pszVal) return FALSE ;
    (*ppszStockID) = g_strdup (pszVal) ;
    }
  else
    {
    if (NULL != pszVal)
      if (!strcmp (pszVal, (*ppszStockID))) return FALSE ;
    g_free ((*ppszStockID)) ;
    (*ppszStockID) = NULL ;
    if (NULL != pszVal)
      if (strcmp (pszVal, ""))
        (*ppszStockID) = g_strdup (pszVal) ;
    }

  if (!(NULL == (instance = QCAD_PROPERTY_UI (property_ui_boolean)->instance) || 
        NULL == (pspec = QCAD_PROPERTY_UI_SINGLE (property_ui_boolean)->pspec)))
    {
    gboolean bActive ;

    g_object_get (G_OBJECT (instance), pspec->name, &bActive, NULL) ;
    qcad_property_ui_boolean_set_button_appearance (property_ui_boolean, bActive) ;
    }

  return TRUE ;
  }

static void toggle_button_toggled (GtkWidget *widget, gpointer data)
  {
  qcad_property_ui_boolean_set_button_appearance (QCAD_PROPERTY_UI_BOOLEAN (data), 
    gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (widget))) ;
  }

static void qcad_property_ui_boolean_set_button_appearance (QCADPropertyUIBoolean *property_ui_boolean, gboolean bActive)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui_boolean) ;
  char *pszStock = NULL ;
  char *pszLabel = NULL ;

  pszStock = 
    bActive 
      ? NULL == property_ui_boolean->pszStockDown
        ? property_ui_boolean->pszStockUp
        : property_ui_boolean->pszStockDown
      : NULL == property_ui_boolean->pszStockUp
        ? property_ui_boolean->pszStockDown
        : property_ui_boolean->pszStockUp ;

  if (NULL != property_ui_single->pspec)
    if (NULL != (pszLabel = (char *)g_param_spec_get_nick (property_ui_single->pspec)))
      if (0 == pszLabel[0])
        pszLabel = NULL ;

  g_object_set (G_OBJECT (property_ui_boolean->toggle_button.widget),
    "stock-id", pszStock,
    "label",    pszLabel,
    NULL) ;
  }

#endif /* def GTK_GUI */
