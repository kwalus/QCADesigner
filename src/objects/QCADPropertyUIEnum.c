#include "../support.h"
#include "QCADPropertyUIEnum.h"
#include "QCADParamSpecTypeList.h"

enum
  {
  QCAD_PROPERTY_UI_ENUM_PROPERTY_RENDER_AS=1
  } ;

static void qcad_property_ui_enum_class_init (QCADPropertyUIEnumClass *klass) ;
static void qcad_property_ui_enum_instance_init (QCADPropertyUIEnum *property_ui_enum) ;

#ifdef GTK_GUI
static void finalize     (GObject *object) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;

static void set_pspec   (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec) ;
static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;

static void option_menu_item_activate (GtkWidget *widget, gpointer data) ;
static void radio_button_toggled (GtkWidget *widget, gpointer data) ;
static void qcad_property_ui_enum_instance_notify (GObject *obj, GParamSpec *pspec, gpointer data) ;

void set_enum_property_cond (QCADPropertyUI *property_ui, gint value) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_enum_get_type ()
  {
  static GType qcad_property_ui_enum_type = 0 ;

  if (0 == qcad_property_ui_enum_type)
    {
    static GTypeInfo qcad_property_ui_enum_info =
      {
      sizeof (QCADPropertyUIEnumClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_enum_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIEnum),
      0,
      (GInstanceInitFunc)qcad_property_ui_enum_instance_init
      } ;

    if (0 != (qcad_property_ui_enum_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_ENUM, &qcad_property_ui_enum_info, 0)))
      g_type_class_ref (qcad_property_ui_enum_type) ;
    }
  return qcad_property_ui_enum_type ;
  }

static void qcad_property_ui_enum_class_init (QCADPropertyUIEnumClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  QCAD_PROPERTY_UI_CLASS (klass)->set_instance   = set_instance ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible    = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive  = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget     = get_widget ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_pspec   = set_pspec ;
  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_ENUM_PROPERTY_RENDER_AS,
    qcad_param_spec_type_list ("render-as", _("Render As"), _("Render as widget"), 
      GTK_TYPE_OPTION_MENU, G_PARAM_READABLE | G_PARAM_WRITABLE, GTK_TYPE_OPTION_MENU, GTK_TYPE_RADIO_BUTTON, 0)) ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_enum_instance_init (QCADPropertyUIEnum *property_ui_enum)
  {
  QCAD_PROPERTY_UI (property_ui_enum)->cxWidgets = 2 ;
  QCAD_PROPERTY_UI (property_ui_enum)->cyWidgets = 1 ;

#ifdef GTK_GUI
  property_ui_enum->option_menu.widget = gtk_option_menu_new () ;
  g_object_ref (G_OBJECT (property_ui_enum->option_menu.widget)) ;
  gtk_widget_show (property_ui_enum->option_menu.widget) ;
  property_ui_enum->option_menu.idxX = 1 ;
  property_ui_enum->option_menu.idxY = 0 ;

  property_ui_enum->frame.widget = gtk_frame_new (NULL) ;
  g_object_ref (G_OBJECT (property_ui_enum->frame.widget)) ;
  gtk_widget_show (property_ui_enum->frame.widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_enum->frame.widget), 2) ;
  property_ui_enum->frame.idxX = 0 ;
  property_ui_enum->frame.idxY = 0 ;

  property_ui_enum->tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (property_ui_enum->tbl) ;
  gtk_container_add (GTK_CONTAINER (property_ui_enum->frame.widget), property_ui_enum->tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_enum->tbl), 2) ;

  property_ui_enum->render_as = GTK_TYPE_OPTION_MENU ;
  property_ui_enum->rb = NULL ;
#endif /* def GTK_GUI */
  property_ui_enum->notify_id = 0 ;
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (object) ;

  set_instance (QCAD_PROPERTY_UI (object), NULL, QCAD_PROPERTY_UI (object)->instance) ;

  g_object_unref (G_OBJECT (property_ui_enum->option_menu.widget)) ;
  g_object_unref (G_OBJECT (property_ui_enum->frame.widget)) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->finalize (object) ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_ENUM_PROPERTY_RENDER_AS:
      if (GTK_TYPE_OPTION_MENU == (QCAD_PROPERTY_UI_ENUM (object)->render_as = (GType)g_value_get_uint (value)))
        {
        QCAD_PROPERTY_UI (object)->cxWidgets = 2 ;
        g_object_set (G_OBJECT (object), "show-label", TRUE, NULL) ;
        }
      else
      if (GTK_TYPE_RADIO_BUTTON == QCAD_PROPERTY_UI_ENUM (object)->render_as)
        {
        QCAD_PROPERTY_UI (object)->cxWidgets = 1 ;
        g_object_set (G_OBJECT (object), "show-label", FALSE, NULL) ;
        }
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_ENUM_PROPERTY_RENDER_AS:
      g_value_set_uint (value, (guint)(QCAD_PROPERTY_UI_ENUM (object)->render_as)) ;
      break ;
    }
  }

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui_single) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->set_tooltip (property_ui_single, tooltip) ;

  if (NULL != property_ui_single->pspec)
    {
    gtk_tooltips_set_tip (tooltip, property_ui_enum->frame.widget,       g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    gtk_tooltips_set_tip (tooltip, property_ui_enum->option_menu.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
    }
  }

static void set_pspec (QCADPropertyUISingle *property_ui_single, GParamSpec *new_pspec)
  {
  int Nix ;
  GtkWidget *mnu = NULL, *mnui = NULL ;
  GParamSpecEnum *new_pspec_enum = NULL ;
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui_single) ;

  if (!G_IS_PARAM_SPEC_ENUM (new_pspec)) return ;

  new_pspec_enum = G_PARAM_SPEC_ENUM (new_pspec) ;

  gtk_container_remove (GTK_CONTAINER (property_ui_enum->frame.widget), property_ui_enum->tbl) ;

  mnu = g_object_new (GTK_TYPE_MENU, "visible", TRUE, NULL) ;

  property_ui_enum->tbl = g_object_new (GTK_TYPE_TABLE, "visible", TRUE, "n-columns", 1, "n-rows", 1, "border-width", 2, NULL) ;
  gtk_container_add (GTK_CONTAINER (property_ui_enum->frame.widget), property_ui_enum->tbl) ;
  gtk_frame_set_label (GTK_FRAME (property_ui_enum->frame.widget), g_param_spec_get_nick (new_pspec)) ;

  for (Nix = 0 ; Nix < new_pspec_enum->enum_class->n_values ; Nix++)
    {
    mnui = gtk_menu_item_new_with_label ((new_pspec_enum->enum_class->values)[Nix].value_nick) ;
    gtk_widget_show (mnui) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_object_set_data (G_OBJECT (mnui), "enum-value", (gpointer)((new_pspec_enum->enum_class->values)[Nix].value)) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)option_menu_item_activate, property_ui_enum) ;

    property_ui_enum->rb = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (property_ui_enum->rb), (new_pspec_enum->enum_class->values)[Nix].value_nick) ;
    gtk_widget_show (property_ui_enum->rb) ;
    gtk_table_attach (GTK_TABLE (property_ui_enum->tbl), property_ui_enum->rb, 0, 1, Nix, Nix + 1, GTK_FILL, GTK_FILL, 2, 2) ;
    g_object_set_data (G_OBJECT (property_ui_enum->rb), "enum-value", (gpointer)((new_pspec_enum->enum_class->values)[Nix].value)) ;
    g_signal_connect (G_OBJECT (property_ui_enum->rb), "toggled", (GCallback)radio_button_toggled, property_ui_enum) ;
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (property_ui_enum->option_menu.widget), mnu) ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->set_pspec (property_ui_single, new_pspec) ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui) ;

  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->set_visible (property_ui, bVisible) ;

  if (bVisible)
    {
    gtk_widget_show (property_ui_enum->option_menu.widget) ;
    gtk_widget_show (property_ui_enum->frame.widget) ;
    }
  else
    {
    gtk_widget_hide (property_ui_enum->option_menu.widget) ;
    gtk_widget_hide (property_ui_enum->frame.widget) ;
    }
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui) ;

  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->set_sensitive (property_ui, bSensitive) ;

  gtk_widget_set_sensitive (property_ui_enum->option_menu.widget, bSensitive) ;
  gtk_widget_set_sensitive (property_ui_enum->frame.widget, bSensitive) ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui) ;

  (*col_span) = 1 ;
  if (GTK_TYPE_OPTION_MENU == property_ui_enum->render_as)
    {
    if (idxX == property_ui_enum->option_menu.idxX && idxY == property_ui_enum->option_menu.idxY)
      return property_ui_enum->option_menu.widget ;
    }
  else
  if (GTK_TYPE_RADIO_BUTTON)
    {
    (*col_span) = -1 ;
    if (idxX == property_ui_enum->frame.idxX && idxY == property_ui_enum->frame.idxY)
      return property_ui_enum->frame.widget ;
    }

  return QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->get_widget (property_ui, idxX, idxY, col_span) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (property_ui) ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_ENUM)))->set_instance (property_ui, new_instance, old_instance))
    {
    char *psz = NULL ;

    if (NULL != old_instance)
      g_signal_handler_disconnect (G_OBJECT (old_instance), property_ui_enum->notify_id) ;

    if (NULL == property_ui_single->pspec) return FALSE ;

    if (NULL != new_instance)
      {
      property_ui_enum->notify_id = 
        g_signal_connect (G_OBJECT (new_instance), 
          psz = g_strdup_printf ("notify::%s", property_ui_single->pspec->name),
          (GCallback)qcad_property_ui_enum_instance_notify, property_ui) ;
      g_free (psz) ;
      g_object_notify (G_OBJECT (new_instance), property_ui_single->pspec->name) ;
      }
    return TRUE ;
    }

  return FALSE ;
  }

static void option_menu_item_activate (GtkWidget *widget, gpointer data)
  {set_enum_property_cond (QCAD_PROPERTY_UI (data), (gint)g_object_get_data (G_OBJECT (widget), "enum-value")) ;}

static void radio_button_toggled (GtkWidget *widget, gpointer data)
  {
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;
  set_enum_property_cond (QCAD_PROPERTY_UI (data), (gint)g_object_get_data (G_OBJECT (widget), "enum-value")) ;
  }

static void qcad_property_ui_enum_instance_notify (GObject *obj, GParamSpec *pspec, gpointer data)
  {
  QCADPropertyUIEnum *property_ui_enum = QCAD_PROPERTY_UI_ENUM (data) ;
  GParamSpecEnum *pspec_enum = G_PARAM_SPEC_ENUM (pspec) ;
  int idx ;
  gint value ;
  GSList *sllItr = NULL ;

  g_object_get (obj, pspec->name, &value, NULL) ;

  for (idx = 0 ; idx < pspec_enum->enum_class->n_values ; idx++)
    if (value == (pspec_enum->enum_class->values)[idx].value)
      break ;

  if (idx == pspec_enum->enum_class->n_values) return ;

  if (gtk_option_menu_get_history (GTK_OPTION_MENU (property_ui_enum->option_menu.widget)) == idx) return ;
  gtk_option_menu_set_history (GTK_OPTION_MENU (property_ui_enum->option_menu.widget), idx) ;

  for (sllItr = gtk_radio_button_get_group (GTK_RADIO_BUTTON (property_ui_enum->rb)) ; sllItr != NULL ; sllItr = sllItr->next)
    if (value == (int)g_object_get_data (G_OBJECT (sllItr->data), "enum-value"))
      if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sllItr->data)))
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sllItr->data), TRUE) ;
  }

void set_enum_property_cond (QCADPropertyUI *property_ui, gint value)
  {
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;
  int current_val ;

  if (NULL == property_ui) return ;
  if (NULL == property_ui->instance) return ;
  if (NULL == property_ui_single->pspec) return ;

  g_object_get (G_OBJECT (property_ui->instance), property_ui_single->pspec->name, &current_val, NULL) ;

  if (value == current_val) return ;

  g_object_set (G_OBJECT (property_ui->instance), property_ui_single->pspec->name, value, NULL) ;
  }
#endif /* def GTK_GUI */
