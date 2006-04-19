#include <stdarg.h>
#include <string.h>
#include "../custom_widgets.h"
#include "../support.h"
#include "QCADObject.h"
#include "QCADParamSpecTypeList.h"
#include "QCADPropertyUISingle.h"
#include "QCADPropertyUIGroup.h"

/**
 * SECTION:QCADPropertyUIGroup
 * @short_description: Property UI group.
 *
 * A propert UI group covers all the properties of a #GObject instance. It creates
 * a list of #QCADPropertyUISingle objects, each covering one of the instance's properties. It then concatenates
 * the 2D grid of widgets created by each "single" property UI vertically, into a larger 2D widget grid.
 *
 * In addition, if the instance happens to be of type #QCADObject, the property hints specific to the property
 * UI group, rather than any of its components, are applied. The behaviour hints connecting the various "single"
 * property UIs contained within the group are also applied.
 */


typedef struct
  {
  char *pszPropName ;
  QCADPropertyUI *property_ui ;
  } QCADPropertyUIGroupEntry ;

enum
  {
  QCAD_PROPERTY_UI_GROUP_PROPERTY_FIRST=1,
#ifdef GTK_GUI
  QCAD_PROPERTY_UI_GROUP_PROPERTY_RENDER_AS,
#endif /* def GTK_GUI */
  QCAD_PROPERTY_UI_GROUP_PROPERTY_TITLE,
  QCAD_PROPERTY_UI_GROUP_PROPERTY_LAST
  } ;

static void qcad_property_ui_group_class_init (QCADPropertyUIGroupClass *klass) ;
static void qcad_property_ui_group_instance_init (QCADPropertyUIGroup *property_ui_group) ;

static void finalize     (GObject *object) ;
#ifdef GTK_GUI
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
#endif /* def GTK_GUI */

static gboolean   set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
#ifdef GTK_GUI
static GtkWidget *get_widget   (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
#endif /* def GTK_GUI */

static void qcad_property_ui_group_create_from_instance      (QCADPropertyUIGroup *property_ui_group, GObject *instance) ;
#ifdef GTK_GUI
static void qcad_property_ui_group_set_render_as             (QCADPropertyUIGroup *property_ui_group, GType new_type) ;
static void qcad_property_ui_group_add_widgets_to_table      (QCADPropertyUIGroup *property_ui_group) ;
static void qcad_property_ui_group_remove_widgets_from_table (QCADPropertyUIGroup *property_ui_group) ;
#endif /* def GTK_GUI */
static QCADPropertyUI *qcad_property_ui_group_get_property_ui (QCADPropertyUIGroup *property_ui_group, char *pszPropertyName) ;
static void qcad_property_ui_group_do_behaviour (QCADPropertyUIGroup *property_ui_group, GObject *instance, gboolean bDisconnect) ;
static void qcad_property_ui_group_do_properties (QCADPropertyUIGroup *property_ui_group, GObject *instance) ;

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
      (GInstanceInitFunc)qcad_property_ui_group_instance_init
      } ;
    if (0 != (qcad_property_ui_group_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI, QCAD_TYPE_STRING_PROPERTY_UI_GROUP, &info, 0)))
      g_type_class_ref (qcad_property_ui_group_type) ;
    }
  return qcad_property_ui_group_type ;
  }

/**
 * qcad_property_ui_group_newv:
 * @instance: Instance to create the UI for.
 * @va: #va_list of %NULL-terminated property name - property value pairs to set on the newly created UI.
 *
 * Creates a new property UI exposing the properties of #GObject instance @instance.
 *
 * If @instance happens to be of type #QCADObject, then this function looks up those #QCADPropertyUIBehaviour
 * entries which connect the new property UI's properties to @instance's properties and those that connect to 
 * one another the properties of #QCADPropertyUISingle objects contained within the newly created
 * #QCADPropertyUIGroup object and makes the appropriate connections.
 *
 * If @instance happens to be of type #QCADObject, then this function looks up those #QCADPropertyUIProperty
 * entries which apply to the new property UI's properties and not to any of the properties for its contained
 * #QCADPropertyUISingle objects and sets the values as prescribed in the matching #QCADPropertyUIProperty 
 * entries.
 *
 * See also: qcad_property_ui_new(), #QCADPropertyUIBehaviour, #QCADPropertyUIProperty
 *
 * Returns: A newly created #QCADPropertyUIGroup for #GObject instance @instance.
 */
QCADPropertyUI *qcad_property_ui_group_newv (GObject *instance, va_list va)
  {
  char *pszFirstProperty = NULL ;
  QCADPropertyUI *pui = NULL ;

  if (NULL == instance) return NULL ;
  if (NULL == (pui = g_object_new (QCAD_TYPE_PROPERTY_UI_GROUP, NULL))) return NULL ;

  qcad_property_ui_set_instance (pui, instance) ;

  if (NULL != (pszFirstProperty = va_arg (va, char *)))
    g_object_set_valist (G_OBJECT (pui), pszFirstProperty, va) ;

  return pui ;
  }

static void qcad_property_ui_group_class_init (QCADPropertyUIGroupClass *klass)
  {
  G_OBJECT_CLASS (klass)->finalize     = finalize ;
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;
#endif /* def GTK_GUI */

  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;
#ifdef GTK_GUI
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;

  /**
   * QCADPropertyUIGroup:render-as:
   *
   * This property UI can be rendered as either a #GtkDialog, a #GtkButton that runs a #GtkDialog, a
   * #GtkFrame, a #GtkExpander, or a property UI - that is, a 2D grid of widgets accessible via
   * qcad_property_ui_get_cx_widgets(), qcad_property_ui_get_cy_widgets(), and qcad_property_ui_get_widget().
   *
   * Valid values: %GTK_TYPE_DIALOG, %GTK_TYPE_BUTTON, %0, %GTK_TYPE_FRAME, %GTK_TYPE_EXPANDER
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_GROUP_PROPERTY_RENDER_AS,
    qcad_param_spec_type_list ("render-as", _("Render As"), _("Render as widget"),
      GTK_TYPE_DIALOG, G_PARAM_READABLE | G_PARAM_WRITABLE, 0, GTK_TYPE_DIALOG, GTK_TYPE_BUTTON, GTK_TYPE_FRAME, GTK_TYPE_EXPANDER, 0)) ;
#endif /* def GTK_GUI */

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_PROPERTY_UI_GROUP_PROPERTY_TITLE,
    g_param_spec_string ("title", _("Title"), _("UI Title"), "", G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_property_ui_group_instance_init (QCADPropertyUIGroup *property_ui_group)
  {
  property_ui_group->property_uis = exp_array_new (sizeof (QCADPropertyUIGroupEntry), 1) ;
#ifdef GTK_GUI
  property_ui_group->tbl = gtk_table_new (1, 1, FALSE) ;
  g_object_ref (G_OBJECT (property_ui_group->tbl)) ;
  gtk_widget_show (property_ui_group->tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_group->tbl), 2) ;

  property_ui_group->frm.widget = gtk_frame_new (NULL) ;
  g_object_ref (G_OBJECT (property_ui_group->frm.widget)) ;
  gtk_widget_show (property_ui_group->frm.widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_group->frm.widget), 2) ;
  property_ui_group->frm.idxX   = 0 ;
  property_ui_group->frm.idxY   = 0 ;

  property_ui_group->xpd.widget = gtk_expander_new (NULL) ;
  g_object_ref (G_OBJECT (property_ui_group->xpd.widget)) ;
  gtk_widget_show (property_ui_group->xpd.widget) ;
  gtk_container_set_border_width (GTK_CONTAINER (property_ui_group->xpd.widget), 2) ;
  property_ui_group->frm.idxX   = 0 ;
  property_ui_group->frm.idxY   = 0 ;

  property_ui_group->dlg.widget = gtk_dialog_new () ;
  gtk_window_set_resizable (GTK_WINDOW (property_ui_group->dlg.widget), FALSE) ;
  g_object_ref (G_OBJECT (property_ui_group->dlg.widget)) ;
  property_ui_group->frm.idxX   = 0 ;
  property_ui_group->frm.idxY   = 0 ;
  gtk_dialog_add_button (GTK_DIALOG (property_ui_group->dlg.widget), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE) ;
  gtk_dialog_set_default_response (GTK_DIALOG (property_ui_group->dlg.widget), GTK_RESPONSE_CLOSE) ;

  property_ui_group->btn.widget = gtk_button_new () ;
  g_object_ref (G_OBJECT (property_ui_group->btn.widget)) ;
  gtk_widget_show (property_ui_group->btn.widget) ;
  property_ui_group->frm.idxX   = 0 ;
  property_ui_group->frm.idxY   = 0 ;

  // The table is initially inside the frame - not really significant, but it has to be inside one of the
  // top-level widgets
  gtk_container_add (GTK_CONTAINER (property_ui_group->frm.widget), property_ui_group->tbl) ;
#endif /* def GTK_GUI */
  property_ui_group->render_as = 0 ;
  }

static void finalize (GObject *object)
  {
  int Nix ;
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (object) ;
  QCADPropertyUIGroupEntry *property_ui_group_entry = NULL ;

  if (NULL != QCAD_PROPERTY_UI (object)->instance)
    qcad_property_ui_group_do_behaviour (property_ui_group, QCAD_PROPERTY_UI (object)->instance, TRUE) ;

  for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
    {
    property_ui_group_entry = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix) ;
    g_free (property_ui_group_entry->pszPropName) ;
    g_object_unref (property_ui_group_entry->property_ui) ;
    }
#ifdef GTK_GUI
  g_object_unref (G_OBJECT (property_ui_group->tbl)) ;
  g_object_unref (G_OBJECT (property_ui_group->frm.widget)) ;
  g_object_unref (G_OBJECT (property_ui_group->xpd.widget)) ;
  g_object_unref (G_OBJECT (property_ui_group->dlg.widget)) ;
  g_object_unref (G_OBJECT (property_ui_group->btn.widget)) ;
#endif /* def GTK_GUI */

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_GROUP)))->finalize (object) ;
  }

#ifdef GTK_GUI
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (object) ;
  switch (property_id)
    {
    case QCAD_PROPERTY_UI_GROUP_PROPERTY_RENDER_AS:
      qcad_property_ui_group_set_render_as (property_ui_group, (GType)g_value_get_uint (value)) ;
      g_object_notify (object, "render-as") ;
      break ;

    case QCAD_PROPERTY_UI_GROUP_PROPERTY_TITLE:
      if (NULL != property_ui_group->pszTitle)
        g_free (property_ui_group->pszTitle) ;
      property_ui_group->pszTitle = g_strdup (g_value_get_string (value)) ;
      gtk_frame_set_label    (GTK_FRAME    (property_ui_group->frm.widget), property_ui_group->pszTitle) ;
      gtk_expander_set_label (GTK_EXPANDER (property_ui_group->xpd.widget), property_ui_group->pszTitle) ;
      gtk_window_set_title   (GTK_WINDOW   (property_ui_group->dlg.widget), property_ui_group->pszTitle) ;
      gtk_button_set_label   (GTK_BUTTON   (property_ui_group->btn.widget), property_ui_group->pszTitle) ;
      g_object_notify (object, "title") ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (object) ;

  switch (property_id)
    {
    case QCAD_PROPERTY_UI_GROUP_PROPERTY_RENDER_AS:
      g_value_set_uint (value, (guint)(property_ui_group->render_as)) ;
      break ;

    case QCAD_PROPERTY_UI_GROUP_PROPERTY_TITLE:
      g_value_set_string (value, property_ui_group->pszTitle) ;
      break ;
    }
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  int idxY_so_far = 0 ;
  QCADPropertyUIGroupEntry *property_ui_group_entry = NULL ;
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (property_ui) ;
  int cxWidgets, cyWidgets ;
  int Nix ;

  (*col_span) = 1 ;

  if (0 == property_ui_group->render_as)
    for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
      {
      property_ui_group_entry = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix) ;
      cxWidgets = qcad_property_ui_get_cx_widgets (property_ui_group_entry->property_ui) ;
      cyWidgets = qcad_property_ui_get_cy_widgets (property_ui_group_entry->property_ui) ;
      if (idxY >= idxY_so_far && idxY <= idxY_so_far + cyWidgets)
        return qcad_property_ui_get_widget (property_ui_group_entry->property_ui, idxX, idxY - idxY_so_far, col_span) ;
      idxY_so_far += cyWidgets ;
      }
  else
  if (GTK_TYPE_FRAME == property_ui_group->render_as)
    {
    if (idxX == property_ui_group->frm.idxX && idxY == property_ui_group->frm.idxY)
      {
      (*col_span) = -1 ;
      return property_ui_group->frm.widget ;
      }
    }
  else
  if (GTK_TYPE_EXPANDER == property_ui_group->render_as)
    {
    if (idxX == property_ui_group->xpd.idxX && idxY == property_ui_group->xpd.idxY)
      {
      (*col_span) = -1 ;
      return property_ui_group->xpd.widget ;
      }
    }
  else
  if (GTK_TYPE_BUTTON == property_ui_group->render_as)
    {
    if (idxX == property_ui_group->btn.idxX && idxY == property_ui_group->btn.idxY)
      return property_ui_group->btn.widget ;
    }
  else
  if (GTK_TYPE_DIALOG == property_ui_group->render_as)
    {
    if (idxX == property_ui_group->dlg.idxX && idxY == property_ui_group->dlg.idxY)
      return property_ui_group->dlg.widget ;
    }
  return NULL ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  int Nix ;
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (property_ui) ;

  if (GTK_TYPE_FRAME == property_ui_group->render_as)
    GTK_WIDGET_SET_VISIBLE (property_ui_group->frm.widget, bVisible) ;
  else
  if (GTK_TYPE_BUTTON == property_ui_group->render_as)
    GTK_WIDGET_SET_VISIBLE (property_ui_group->btn.widget, bVisible) ;
  else
  if (GTK_TYPE_EXPANDER == property_ui_group->render_as)
    GTK_WIDGET_SET_VISIBLE (property_ui_group->xpd.widget, bVisible) ;
  else
  if (GTK_TYPE_DIALOG == property_ui_group->render_as)
    GTK_WIDGET_SET_VISIBLE (property_ui_group->dlg.widget, bVisible) ;
  else
  if (0 == property_ui_group->render_as)
    for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
      g_object_set (exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix).property_ui, "visible", bVisible, NULL) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  int Nix ;
  QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (property_ui) ;

  if (GTK_TYPE_FRAME == property_ui_group->render_as)
    gtk_widget_set_sensitive (property_ui_group->frm.widget, bSensitive) ;
  else
  if (GTK_TYPE_BUTTON == property_ui_group->render_as)
    gtk_widget_set_sensitive (property_ui_group->btn.widget, bSensitive) ;
  else
  if (GTK_TYPE_EXPANDER == property_ui_group->render_as)
    gtk_widget_set_sensitive (property_ui_group->xpd.widget, bSensitive) ;
  else
  if (GTK_TYPE_DIALOG == property_ui_group->render_as)
    gtk_widget_set_sensitive (property_ui_group->dlg.widget, bSensitive) ;
  else
  if (0 == property_ui_group->render_as)
    for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
      g_object_set (exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix).property_ui, "sensitive", bSensitive, NULL) ;
  }
#endif /* def GTK_GUI */

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_GROUP)))->set_instance (property_ui, new_instance, old_instance))
    {
    int Nix ;
    QCADPropertyUIGroup *property_ui_group = QCAD_PROPERTY_UI_GROUP (property_ui) ;

    if (0 == property_ui_group->type)
      {
      property_ui_group->type = G_TYPE_FROM_INSTANCE (new_instance) ;
      qcad_property_ui_group_create_from_instance (QCAD_PROPERTY_UI_GROUP (property_ui), new_instance) ;
      qcad_property_ui_group_do_behaviour (property_ui_group, new_instance, FALSE) ;
      qcad_property_ui_group_do_properties (property_ui_group, new_instance) ;
      }
    else
    if (property_ui_group->type != G_TYPE_FROM_INSTANCE (new_instance))
      return FALSE ;
    else
      {
      if (NULL != old_instance)
        qcad_property_ui_group_do_behaviour (property_ui_group, old_instance, TRUE) ;

      if (NULL != new_instance)
        qcad_property_ui_group_do_behaviour (property_ui_group, new_instance, FALSE) ;

      qcad_property_ui_group_do_properties (property_ui_group, new_instance) ;

      for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
        qcad_property_ui_set_instance (exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix).property_ui, new_instance) ;
      }
    return TRUE ;
    }
  return FALSE ;
  }

static void qcad_property_ui_group_create_from_instance (QCADPropertyUIGroup *property_ui_group, GObject *instance)
  {
  int Nix ;
  GParamSpec **param_specs = NULL ;
  guint icParamSpecs = 0 ;
  QCADPropertyUIGroupEntry puig_entry = {NULL, NULL} ;

  param_specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (instance), &icParamSpecs) ;

  for (Nix = 0 ; Nix < icParamSpecs ; Nix++)
    if (NULL != (puig_entry.property_ui = qcad_property_ui_new (instance, param_specs[Nix]->name, NULL)))
      {
      puig_entry.pszPropName = g_strdup (param_specs[Nix]->name) ;
      exp_array_1d_insert_vals (property_ui_group->property_uis, &puig_entry, 1, -1) ;
//      g_object_add_weak_pointer (G_OBJECT (puig_entry.property_ui), 
//        (gpointer *)&(exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, property_ui_group->property_uis->icUsed - 1).property_ui)) ;
      }
  }

#ifdef GTK_GUI
static void qcad_property_ui_group_set_render_as (QCADPropertyUIGroup *property_ui_group, GType new_type)
  {
  if (NULL == property_ui_group) return ;
  if (new_type == property_ui_group->render_as) return ;

  // 0 -> non-0 => we're gonna use our table
  if (property_ui_group->render_as == 0 && new_type != 0)
    qcad_property_ui_group_add_widgets_to_table (property_ui_group) ;
  else
  // non-0 -> 0 => we're gonna use the widgets themselves
  if (property_ui_group->render_as != 0 && new_type == 0)
    qcad_property_ui_group_remove_widgets_from_table (property_ui_group) ;

  if (new_type != 0)
    {
    g_object_ref (property_ui_group->tbl) ;
    gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (property_ui_group->tbl)), property_ui_group->tbl) ;
    if (GTK_TYPE_BUTTON == new_type || GTK_TYPE_DIALOG == new_type)
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (property_ui_group->dlg.widget)->vbox), property_ui_group->tbl, FALSE, FALSE, 0) ;
    else
    if (GTK_TYPE_FRAME == new_type)
      gtk_container_add (GTK_CONTAINER (property_ui_group->frm.widget), property_ui_group->tbl) ;
    else
    if (GTK_TYPE_EXPANDER == new_type)
      gtk_container_add (GTK_CONTAINER (property_ui_group->xpd.widget), property_ui_group->tbl) ;
    g_object_unref (property_ui_group->tbl) ;
    }

  property_ui_group->render_as = new_type ;
  }

// remove widgets from whatever container and add widgets to tbl container
static void qcad_property_ui_group_add_widgets_to_table (QCADPropertyUIGroup *property_ui_group)
  {
  QCADPropertyUIGroupEntry *property_ui_group_entry = NULL ;
  int Nix, Nix1, Nix2 ;
  int idxY = 0, cxWidgets = 0, cyWidgets = 0 ;
  GtkWidget *widget = NULL, *parent = NULL ;
  int col_span = 1 ;
  int max_cx_widgets = 0 ;

  for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
    {
    property_ui_group_entry = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix) ;
    cxWidgets = qcad_property_ui_get_cx_widgets (property_ui_group_entry->property_ui) ;
    max_cx_widgets = MAX (max_cx_widgets, cxWidgets) ;
    }

  for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++,idxY++)
    {
    property_ui_group_entry = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix) ;
    cxWidgets = qcad_property_ui_get_cx_widgets (property_ui_group_entry->property_ui) ;
    cyWidgets = qcad_property_ui_get_cy_widgets (property_ui_group_entry->property_ui) ;
    for (Nix1 = 0 ; Nix1 < cyWidgets ; Nix1++, idxY++)
      for (Nix2 = 0 ; Nix2 < cxWidgets ; Nix2++)
        {
        col_span = 1 ;
        if (NULL != (widget = qcad_property_ui_get_widget (property_ui_group_entry->property_ui, Nix2, Nix1, &col_span)))
          {
          g_object_ref (G_OBJECT (widget)) ;
          // Remove widget from whatever container it's in
          if (NULL != (parent = gtk_widget_get_parent (widget)))
            if (GTK_IS_CONTAINER (parent))
              gtk_container_remove (GTK_CONTAINER (parent), widget) ;

          if (0 == col_span) col_span = 1 ;

          if (col_span > 0)
            gtk_table_attach (GTK_TABLE (property_ui_group->tbl), widget, Nix2, Nix2 + col_span, idxY, idxY + 1,
              (GtkAttachOptions)(GTK_FILL),
              (GtkAttachOptions)(GTK_FILL), 2, 2) ;
          // Stretch widgets that span to the end of the column
          else
            gtk_table_attach (GTK_TABLE (property_ui_group->tbl), widget, Nix2, max_cx_widgets, idxY, idxY + 1,
              (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
              (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

          g_object_unref (G_OBJECT (widget)) ;
          }
        }
    }
  }

// remove widgets from tbl container
static void qcad_property_ui_group_remove_widgets_from_table (QCADPropertyUIGroup *property_ui_group)
  {
  QCADPropertyUIGroupEntry *property_ui_group_entry = NULL ;
  int Nix, Nix1, Nix2 ;
  int idxY = 0, cxWidgets = 0, cyWidgets = 0 ;
  GtkWidget *widget = NULL ;

  for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++,idxY++)
    {
    property_ui_group_entry = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix) ;
    cxWidgets = qcad_property_ui_get_cx_widgets (property_ui_group_entry->property_ui) ;
    cyWidgets = qcad_property_ui_get_cy_widgets (property_ui_group_entry->property_ui) ;
    for (Nix1 = 0 ; Nix1 < cyWidgets ; Nix1++, idxY++)
      for (Nix2 = 0 ; Nix2 < cxWidgets ; Nix2++)
        if (NULL != (widget = qcad_property_ui_get_widget (property_ui_group_entry->property_ui, Nix2, Nix1, NULL)))
          gtk_container_remove (GTK_CONTAINER (property_ui_group->tbl), widget) ;
    }
  }
#endif /* def GTK_GUI */

static QCADPropertyUI *qcad_property_ui_group_get_property_ui (QCADPropertyUIGroup *property_ui_group, char *pszPropertyName)
  {
  int Nix ;
  QCADPropertyUIGroupEntry *puige = NULL ;

  for (Nix = 0 ; Nix < property_ui_group->property_uis->icUsed ; Nix++)
    if (!strcmp ((puige = &exp_array_index_1d (property_ui_group->property_uis, QCADPropertyUIGroupEntry, Nix))->pszPropName, pszPropertyName))
      return puige->property_ui ;

  return NULL ;
  }

// This function is very similar to its QCADPropertyUISingle cousin, except it assigns those behaviours which
// tie a property of one QCADPropertyUISingle to a property of another QCADPropertyUISingle. It does not handle
// linking instance properties to QCADProperyUISingle properties.
static void qcad_property_ui_group_do_behaviour (QCADPropertyUIGroup *property_ui_group, GObject *instance, gboolean bDisconnect)
  {
  // (Dis)connect the rules between QCADPropertyUISingle objects
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

      if (!(NULL == behaviour->instance_property_name1 ||
            NULL == behaviour->ui_property_name1       ||
            NULL == behaviour->instance_property_name2 ||
            NULL == behaviour->ui_property_name2))
        {
        instance1 = G_OBJECT (qcad_property_ui_group_get_property_ui (property_ui_group, behaviour->instance_property_name1)) ;
        instance2 = G_OBJECT (qcad_property_ui_group_get_property_ui (property_ui_group, behaviour->instance_property_name2)) ;
        name1 = behaviour->ui_property_name1 ;
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

static void qcad_property_ui_group_do_properties (QCADPropertyUIGroup *property_ui_group, GObject *instance)
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
      if (NULL == puip->instance_property_name)
        g_object_set_property (G_OBJECT (property_ui_group), puip->ui_property_name, &(puip->ui_property_value)) ;
      }
    }
  }
