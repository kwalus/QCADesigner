#include "QCADComboBox.h"
/**
 * SECTION:QCADComboBox
 * @short_description: QCADesigner workaround class for #GtkComboBox.
 *
 * #GtkComboBox widgets in versions of GTK prior to 2.8.0 did not emit the "notify::active" signal when
 * the active entry was changed. A #QCADComboBox will emit such a signal by connecting to #GtkComboBox's
 * "<link linkend="GtkComboBox-changed">changed</link>" signal, and emitting its "notify::active" signal:
 * <informalexample><programlisting>
 * static void instance_init (QCADComboBox *instance, gpointer data)
 *   {g_signal_connect (G_OBJECT (instance), "changed", (GCallback)g_object_notify, "active") ;}
 * </programlisting></informalexample>
 *
 * For versions of GTK >= 2.8.0 detected at compile time, the definition of this widget evaluates to
 * #GtkComboBox and none of the #QCADComboBox code gets compiled.
 */
#if (GTK_MINOR_VERSION < 8 || defined (GTK_DOC))

static void instance_init (QCADComboBox *instance, gpointer data) ;

GType qcad_combo_box_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADComboBoxClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)NULL,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADComboBox),
      0,
      (GInstanceInitFunc)instance_init
      } ;

    if ((the_type = g_type_register_static (GTK_TYPE_COMBO_BOX, QCAD_TYPE_STRING_COMBO_BOX, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void instance_init (QCADComboBox *instance, gpointer data)
  {g_signal_connect (G_OBJECT (instance), "changed", (GCallback)g_object_notify, "active") ;}
#endif /* (GTK_MINOR_VERION < 8) */
