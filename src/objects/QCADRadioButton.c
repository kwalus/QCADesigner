#include "QCADRadioButton.h"
/**
 * SECTION:QCADRadioButton
 * @short_description: QCADesigner workaround class for #GtkRadioButton.
 *
 * #GtkRadioButton widgets in versions of GTK <= 2.8.0 did not emit the "notify::active" signal when
 * their active state was changed. A #QCADRadioButton will emit such a signal by connecting to #GtkRadioButton's
 * "toggled" signal, and emitting its "notify::active" signal:
 * <informalexample><programlisting>
 * static void instance_init (GObject *obj)
 *   {g_signal_connect (obj, "toggled", (GCallback)g_object_notify, "active") ;}
 * </programlisting></informalexample>
 *
 * For versions of GTK > 2.8.0 detected at compile time, the definition of this widget evaluates to
 * #GtkRadioButton and none of the #QCADRadioButton code gets compiled.
 */
#if (GTK_MINOR_VERSION <= 8 || defined (GTK_DOC))
static void instance_init (GObject *obj) ;

GType qcad_radio_button_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADRadioButtonClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)NULL,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADRadioButton),
      0,
      (GInstanceInitFunc)instance_init
      } ;

//    g_print ("QCADRadioButton::get_type\n") ;

    if ((the_type = g_type_register_static (GTK_TYPE_RADIO_BUTTON, QCAD_TYPE_STRING_RADIO_BUTTON, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void instance_init (GObject *obj)
  {g_signal_connect (obj, "toggled", (GCallback)g_object_notify, "active") ;}
#endif
