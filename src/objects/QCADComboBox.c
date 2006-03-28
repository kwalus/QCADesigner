#include "QCADComboBox.h"

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
