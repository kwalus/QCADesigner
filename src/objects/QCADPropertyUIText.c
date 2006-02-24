#include <stdarg.h>
#include "../custom_widgets.h"
#include "../support.h"
#include "../generic_utils.h"
#include "QCADPropertyUIText.h"

static void qcad_property_ui_text_class_init (QCADPropertyUITextClass *klass) ;
static void qcad_property_ui_text_instance_init (QCADPropertyUIText *qcad_property_ui_text) ;

#ifdef GTK_GUI
static void finalize (GObject *object) ;

static void       set_visible   (QCADPropertyUI *property_ui, gboolean bVisible) ;
static void       set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive) ;
static GtkWidget *get_widget    (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
static gboolean   set_instance  (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip) ;
#endif /* def GTK_GUI */

GType qcad_property_ui_text_get_type ()
  {
  static GType qcad_property_ui_text_type = 0 ;

  if (0 == qcad_property_ui_text_type)
    {
    static GTypeInfo info =
      {
      sizeof (QCADPropertyUITextClass),
      NULL,
      NULL,
      (GClassInitFunc)qcad_property_ui_text_class_init,
      NULL,
      NULL,
      sizeof (QCADPropertyUIText),
      0,
      (GInstanceInitFunc)qcad_property_ui_text_instance_init
      } ;
    if (0 != (qcad_property_ui_text_type = g_type_register_static (QCAD_TYPE_PROPERTY_UI_SINGLE, QCAD_TYPE_STRING_PROPERTY_UI_TEXT, &info, 0)))
      g_type_class_ref (qcad_property_ui_text_type) ;
    }
  return qcad_property_ui_text_type ;
  }

static void qcad_property_ui_text_class_init (QCADPropertyUITextClass *klass)
  {
#ifdef GTK_GUI
  G_OBJECT_CLASS (klass)->finalize     = finalize ;

  QCAD_PROPERTY_UI_CLASS (klass)->get_widget    = get_widget ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_visible   = set_visible ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_sensitive = set_sensitive ;
  QCAD_PROPERTY_UI_CLASS (klass)->set_instance  = set_instance ;

  QCAD_PROPERTY_UI_SINGLE_CLASS (klass)->set_tooltip = set_tooltip ;
#endif /* def GTK_GUI */
  }

static void qcad_property_ui_text_instance_init (QCADPropertyUIText *property_ui_text)
  {
  QCADPropertyUI *property_ui = QCAD_PROPERTY_UI (property_ui_text) ;

  property_ui->cxWidgets  = 2 ;
  property_ui->cyWidgets  = 1 ;

#ifdef GTK_GUI
  property_ui_text->entry.widget = gtk_entry_new () ;
  g_object_ref (G_OBJECT (property_ui_text->entry.widget)) ;
  gtk_widget_show (property_ui_text->entry.widget) ;
  property_ui_text->entry.idxX   = 1 ;
  property_ui_text->entry.idxY   = 0 ;
#endif /* def GTK_GUI */
  }

#ifdef GTK_GUI
static void finalize (GObject *object)
  {
  set_instance (QCAD_PROPERTY_UI (object), NULL, QCAD_PROPERTY_UI (object)->instance) ;
  g_object_unref (G_OBJECT (QCAD_PROPERTY_UI_TEXT (object)->entry.widget)) ;
  }

static void set_visible (QCADPropertyUI *property_ui, gboolean bVisible)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_TEXT)))->set_visible (property_ui, bVisible) ;
  GTK_WIDGET_SET_VISIBLE (QCAD_PROPERTY_UI_TEXT (property_ui)->entry.widget, bVisible) ;
  }

static void set_sensitive (QCADPropertyUI *property_ui, gboolean bSensitive)
  {
  QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_TEXT)))->set_sensitive (property_ui, bSensitive) ;
  gtk_widget_set_sensitive (QCAD_PROPERTY_UI_TEXT (property_ui)->entry.widget, bSensitive) ;
  }

static gboolean set_instance (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance)
  {
  QCADPropertyUIText *property_ui_text = QCAD_PROPERTY_UI_TEXT (property_ui) ;
  QCADPropertyUISingle *property_ui_single = QCAD_PROPERTY_UI_SINGLE (property_ui) ;

  if (NULL == property_ui_single->pspec) return FALSE ;

  if (QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_TEXT)))->set_instance (property_ui, new_instance, old_instance))
    {
    if (NULL != old_instance)
      disconnect_object_properties (old_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_text->entry.widget), "text", 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

    if (NULL != new_instance)
      {
      connect_object_properties (new_instance, property_ui_single->pspec->name, G_OBJECT (property_ui_text->entry.widget), "text", 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL, 
        CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;
      g_object_notify (new_instance, property_ui_single->pspec->name) ;
      }

    return TRUE ;
    }
  return FALSE ;
  }

static GtkWidget *get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span)
  {
  QCADPropertyUIText *property_ui_text = QCAD_PROPERTY_UI_TEXT (property_ui) ;

  (*col_span) = 1 ;
  return ((idxX == property_ui_text->entry.idxX && idxY == property_ui_text->entry.idxY)
    ? property_ui_text->entry.widget 
    : QCAD_PROPERTY_UI_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_TEXT)))->get_widget (property_ui, idxX, idxY, col_span)) ;
  }

static void set_tooltip (QCADPropertyUISingle *property_ui_single, GtkTooltips *tooltip)
  {
  QCAD_PROPERTY_UI_SINGLE_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_PROPERTY_UI_TEXT)))->set_tooltip (property_ui_single, tooltip) ;

  if (NULL != property_ui_single->pspec)
    gtk_tooltips_set_tip (tooltip, QCAD_PROPERTY_UI_TEXT (property_ui_single)->entry.widget, g_param_spec_get_nick (property_ui_single->pspec), g_param_spec_get_blurb (property_ui_single->pspec)) ;
  }
#endif /* def GTK_GUI */
