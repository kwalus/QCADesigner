#include "QCADRadioButton.h"

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

    g_print ("QCADRadioButton::get_type\n") ;

    if ((the_type = g_type_register_static (GTK_TYPE_RADIO_BUTTON, QCAD_TYPE_STRING_RADIO_BUTTON, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void instance_init (GObject *obj)
  {g_signal_connect (obj, "toggled", (GCallback)g_object_notify, "active") ;}
