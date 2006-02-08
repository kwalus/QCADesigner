#include <string.h>
#include "../support.h"
#include "../generic_utils.h"
#include "../custom_widgets.h"
#include "QCADPropertyUIInt.h"
#include "QCADParamSpecTypeList.h"

enum
  {
  QCAD_PROPERTY_UI_INT_PROPERTY_RENDER_AS = 1,
  QCAD_PROPERTY_UI_INT_PROPERTY_LAST
  } ;

static void qcad_property_ui_int_class_init (QCADPropertyUIIntClass *klass) ;
static void qcad_property_ui_int_instance_init (QCADPropertyUIInt *property_ui_int) ;

#ifdef GTK_GUI
static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
#endif /* def GTK_GUI */

#ifdef GTK_GUI
static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
#endif /* def GTK_GUI */

#ifdef GTK_GUI
static void set_pspec   (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec) ;
static void set_tooltip (QCADPropertyUISingle *property_ui, GtkTooltips *tooltip) ;
#endif /* def GTK_GUI */

#ifdef GTK_GUI
static void qcad_property_ui_int_instance_notify (GObject *obj, GParamSpec *pspec, gpointer data) ;
static void qcad_property_ui_int_option_menu_item_activate (GtkWidget *widget, gpointer data) ;
static void qcad_property_ui_int_set_render_as (QCADPropertyUIInt *property_ui_int, GType type) ;
static void qcad_property_ui_int_create_option_menu (QCADPropertyUIInt *property_ui_int) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_int_get_type ()
  {
  static GType qcad_property_ui_int_type = 0 ;

  if (0 == qcad_property_ui_int_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUIIntClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_int_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIInt),
      0,
      (GInstanceInitFunc)qcad_property_ui_int_instance_init
      } ;
    if (0 != (qcad_property_ui_int_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_NUMERIC, QCAD_TYPE_STRING_PROPERTY_UI_INT, &info, 0)))
      g_type_class_ref (qcad_property_ui_int_type) ;
    }
  return qcad_property_ui_int_type ;
  }

static void qcad_property_ui_int_class_init (QCADPropertyUIIntClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
  G_OBJECT_CLASS (klass)->finalize     = finalize ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec    = set_pspec ;
  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_INT_PROPERTY_RENDER_AS,
    qcad_param_spec_type_list ("render-as", _("Render As"), _("Render as widget"),
      GTK_TYPE_SPIN_BUTTON, G_PARAM_READABLE | G_PARAM_WRITABLE, GTK_TYPE_SPIN_BUTTON, GTK_TYPE_OPTION_MENU, 0)) ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_int_instance_init (QCADPropertyUIInt *property_ui_int)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_int) ;
#ifdef GTK_GUI
  QCADPropertyUINumeric *property_ui_numeric = QCAD_PROPERTY_UI_NUMERIC (property_ui_int) ;
#endif /* def GTK_GUI */

  property_ui->cxWidgets  = 2 ;
  property_ui->cyWidgets  = 1 ;
  property_ui->bSensitive = TRUE ;
  property_ui->bVisible   = TRUE ;

#ifdef GTK_GUI
  property_ui_int->render_as           = GTK_TYPE_SPIN_BUTTON ;

  property_ui_int->spn.widget          = gtk_spin_button_new (property_ui_int->adj = 
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 0)), 1, 0) ;
  g_object_ref (G_OBJECT (property_ui_int->spn.widget)) ;
  gtk_widget_show (property_ui_int->spn.widget) ;
  gtk_entry_set_activates_default (GTK_ENTRY (property_ui_int->spn.widget), TRUE) ;
  property_ui_int->spn.idxX            = 1 ;
  property_ui_int->spn.idxY            = 0 ;
  property_ui_numeric->lblUnits.idxX   = 2 ;
  property_ui_numeric->lblUnits.idxY   = 0 ;

  property_ui_int->option_menu.widget = gtk_option_menu_new () ;
  g_object_ref (G_OBJECT (property_ui_int->option_menu.widget)) ;
  property_ui_int->option_menu.idxX   = 1 ;
  property_ui_int->option_menu.idxY   = 0 ;
  gtk_widget_show (property_ui_int->option_menu.widget) ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_INT_PROPERTY_RENDER_AS:
      qcad_property_ui_int_set_render_as (QCAD_PROPERTY_UI_INT (object), (GType)g_value_get_uint (value)) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_INT_PROPERTY_RENDER_AS:
      g_value_set_uint (value, (guint)(QCAD_PROPERTY_UI_INT (object)->render_as)) ;
      break ;
    }
  }

static void finalize (GObject *object)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (object) ;

  set_instance (QCAD_PROPERTY_UI (object), NULL, QCAD_PROPERTY_UI (object)->instance) ;

  g_object_unref (property_ui_int->spn.widget) ;
  g_object_unref (property_ui_int->option_menu.widget) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->finalize (object) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  (*col_span) = 1 ;
  if (property_ui_int->render_as == GTK_TYPE_SPIN_BUTTON)
    {
    if (idxX == property_ui_int->spn.idxX && idxY == property_ui_int->spn.idxY)
      return property_ui_int->spn.widget ;
    }
  else
  if (property_ui_int->render_as == GTK_TYPE_OPTION_MENU)
    {
    if (idxX == property_ui_int->option_menu.idxX && idxY == property_ui_int->option_menu.idxY)
      return property_ui_int->option_menu.widget ;
    }
  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->get_widget (property_ui, idxX, idxY, col_span) ;
  }

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui_single) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_tooltip (property_ui_single, tooltip) ;

  if (NULL != property_ui_single->pspec)
    {
    gtk_tooltips_set_tip (tooltip, property_ui_int->spn.widget,         g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    gtk_tooltips_set_tip (tooltip, property_ui_int->option_menu.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    }
  }

static void set_pspec (QCADPropertyUISingle *property_ui, GParamSpec *new_pspec)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  if (G_TYPE_INT == G_PARAM_SPEC_VALUE_TYPE (new_pspec))
    {
    property_ui_int->adj->lower = ((GParamSpecInt *)(new_pspec))->minimum ;
    property_ui_int->adj->upper = ((GParamSpecInt *)(new_pspec))->maximum ;
    property_ui_int->adj->value = ((GParamSpecInt *)(new_pspec))->default_value ;
    }
  else
  if (G_TYPE_UINT == G_PARAM_SPEC_VALUE_TYPE (new_pspec))
    {
    property_ui_int->adj->lower = ((GParamSpecUInt *)(new_pspec))->minimum ;
    property_ui_int->adj->upper = ((GParamSpecUInt *)(new_pspec))->maximum ;
    property_ui_int->adj->value = ((GParamSpecUInt *)(new_pspec))->default_value ;
    }

  if (GTK_TYPE_OPTION_MENU == property_ui_int->render_as)
    qcad_property_ui_int_create_option_menu (property_ui_int) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_pspec (property_ui, new_pspec) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  char *psz = NULL ;
  QCADPropertyUIInt    *property_ui_int    = QCAD_PROPERTY_UI_INT (property_ui) ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_instance (property_ui, new_instance, old_instance))
    if (NULL != property_ui_single->pspec)
      {
      if (NULL != old_instance)
        {
        if (0 != property_ui_int->notify_id)
          g_signal_handler_disconnect (old_instance, property_ui_int->notify_id) ;
        disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_int->adj), "value",
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
        }

      if (NULL != new_instance)
        {
        connect_object_properties (new_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_int->adj), "value", 
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
          CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
        psz = g_strdup_printf ("notify::%s", property_ui_single->pspec->name) ;
        property_ui_int->notify_id = 
          g_signal_connect (G_OBJECT (new_instance), psz, (GCallback)qcad_property_ui_int_instance_notify, property_ui) ;
        g_free (psz) ;

        g_object_notify (new_instance, property_ui_single->pspec->name) ;
        }
      return TRUE ;
      }
  return FALSE ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_visible (property_ui, bVisible) ;

  if (GTK_TYPE_SPIN_BUTTON == property_ui_int->render_as)
    GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_INT (property_ui)->spn.widget, bVisible) ;
  else
  if (GTK_TYPE_OPTION_MENU == property_ui_int->render_as)
    GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_INT (property_ui)->option_menu.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (property_ui) ;

  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_INT)))->set_sensitive (property_ui, bSensitive) ;

  if (GTK_TYPE_SPIN_BUTTON == property_ui_int->render_as)
    gtk_widget_set_sensitive (QCAD_PROPERTY_UI_INT (property_ui)->spn.widget, bSensitive) ;
  else
  if (GTK_TYPE_OPTION_MENU == property_ui_int->render_as)
    gtk_widget_set_sensitive (QCAD_PROPERTY_UI_INT (property_ui)->option_menu.widget, bSensitive) ;
  }

static void qcad_property_ui_int_instance_notify (GObject *obj, GParamSpec *pspec, gpointer data)
  {
  GValue val = {0, }, val_int = {0, } ;
  QCADPropertyUIInt *property_ui_int = QCAD_PROPERTY_UI_INT (data) ;

  if (pspec != QCAD_PROPERTY_UI_SINGLE (data)->pspec) return ;

  g_value_init (&val,     G_PARAM_SPEC_VALUE_TYPE (pspec)) ;
  g_value_init (&val_int, G_TYPE_INT) ;

  g_object_get_property (obj, pspec->name, &val) ;

  g_value_transform (&val, &val_int) ;

  if (gtk_option_menu_get_history (GTK_OPTION_MENU (property_ui_int->option_menu.widget)) != (g_value_get_int (&val_int) - property_ui_int->adj->lower))
    gtk_option_menu_set_history (GTK_OPTION_MENU (property_ui_int->option_menu.widget), g_value_get_int (&val_int) - property_ui_int->adj->lower) ;

  g_value_unset (&val) ;
  g_value_unset (&val_int) ;
  }

static void qcad_property_ui_int_option_menu_item_activate (GtkWidget *widget, gpointer data)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (data) ;

  if (NULL != property_ui->instance)
    g_object_set (property_ui->instance, QCAD_PROPERTY_UI_SINGLE (data)->pspec->name, (int)g_object_get_data (G_OBJECT (widget), "val"), NULL) ;
  }

static void qcad_property_ui_int_set_render_as (QCADPropertyUIInt *property_ui_int, GType type)
  {
  if (type == property_ui_int->render_as) return ;

  if (type == GTK_TYPE_OPTION_MENU)
    qcad_property_ui_int_create_option_menu (property_ui_int) ;

  property_ui_int->render_as = type ;
  g_object_notify (G_OBJECT (property_ui_int), "render-as") ;
  }

static void qcad_property_ui_int_create_option_menu (QCADPropertyUIInt *property_ui_int)
  {
  int Nix ;
  char *psz = NULL ;
  char *pszPrefix = NULL ;
  GtkWidget *mnu = NULL, *mnui = NULL ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui_int) ;

  mnu = gtk_menu_new () ;
  gtk_widget_show (mnu) ;

  if (NULL != property_ui_single->pspec)
    if (NULL != (pszPrefix = (char *)g_param_spec_get_nick (property_ui_single->pspec)))
      if (0 == pszPrefix[0])
        pszPrefix = NULL ;

  for (Nix = property_ui_int->adj->lower ; Nix <= property_ui_int->adj->upper ; Nix++)
    {
    if (NULL == pszPrefix)
      psz = g_strdup_printf ("%d", Nix) ;
    else
      psz = g_strdup_printf ("%s %d", pszPrefix, Nix) ;
    mnui = gtk_menu_item_new_with_label (psz) ;
    gtk_widget_show (mnui) ;
    g_object_set_data (G_OBJECT (mnui), "val", (gpointer)Nix) ;
    g_free (psz) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)qcad_property_ui_int_option_menu_item_activate, property_ui_int) ;
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (property_ui_int->option_menu.widget), mnu) ;
  gtk_option_menu_set_history (GTK_OPTION_MENU (property_ui_int->option_menu.widget), property_ui_int->adj->value - property_ui_int->adj->lower) ;
  }
#endif /* def GTK_GUI */
