#if (GTK_MINOR_VERSION <= 4)
#include "QCADCellRendererText.h"

G_BEGIN_DECLS
extern void g_cclosure_user_marshal_VOID__OBJECT_STRING (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

G_END_DECLS

enum
  {
  QCAD_CELL_RENDERER_TEXT_EDITING_STARTED_SIGNAL,
  QCAD_CELL_RENDERER_TEXT_LAST_SIGNAL
  } ;

static guint cell_renderer_text_signals[QCAD_CELL_RENDERER_TEXT_LAST_SIGNAL] = {0} ;

enum
  {
  QCAD_CELL_RENDERER_TEXT_PROPERTY_FIRST = 1,

  QCAD_CELL_RENDERER_TEXT_PROPERTY_SENSITIVE,

  QCAD_CELL_RENDERER_TEXT_PROPERTY_LAST
  } ;

static void class_init (QCADCellRendererTextClass *klass) ;
static void instance_init (QCADCellRendererText *instance) ;
static void get_property (GObject *object, guint param_id,       GValue *value, GParamSpec *pspec) ;
static void set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec) ;
static void             render        (GtkCellRenderer *cr, GdkWindow *window, GtkWidget *widget, GdkRectangle *background_area, GdkRectangle *cell_area, GdkRectangle *expose_area, GtkCellRendererState flags) ;
static GtkCellEditable *start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags) ;

GType qcad_cell_renderer_text_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info =
      {
      sizeof (QCADCellRendererTextClass),
      NULL,
      NULL,
      (GClassInitFunc)class_init,
      NULL,
      NULL,
      sizeof (QCADCellRendererText),
      0,
      (GInstanceInitFunc)instance_init
      } ;

    if (0 != (the_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, QCAD_TYPE_STRING_CELL_RENDERER_TEXT, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void class_init (QCADCellRendererTextClass *klass)
  {
  GObjectClass *object_klass = G_OBJECT_CLASS (klass) ;
  GtkCellRendererClass *cr_klass = GTK_CELL_RENDERER_CLASS (klass) ;

  object_klass->set_property = set_property ;
  object_klass->get_property = get_property ;

  cr_klass->render        = render ;
  cr_klass->start_editing = start_editing ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_RENDERER_TEXT_PROPERTY_SENSITIVE,
    g_param_spec_boolean ("sensitive", "Sensitive", "Whether this text cell renderer is sensitive",
      TRUE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  cell_renderer_text_signals[QCAD_CELL_RENDERER_TEXT_EDITING_STARTED_SIGNAL] =
    g_signal_new ("editing-started", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADCellRendererTextClass, editing_started), NULL, NULL, g_cclosure_user_marshal_VOID__OBJECT_STRING,
        G_TYPE_NONE, 2, GTK_TYPE_CELL_EDITABLE, G_TYPE_STRING) ;
  }

static void instance_init (QCADCellRendererText *instance)
  {
  instance->sensitive = TRUE ;
  }

static void get_property (GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
  {
  switch (param_id)
    {
    case QCAD_CELL_RENDERER_TEXT_PROPERTY_SENSITIVE:
      g_value_set_boolean (value, QCAD_CELL_RENDERER_TEXT (object)->sensitive) ;
      break ;
    }
  }

static void set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
  {
  switch (param_id)
    {
    case QCAD_CELL_RENDERER_TEXT_PROPERTY_SENSITIVE:
      QCAD_CELL_RENDERER_TEXT (object)->sensitive = g_value_get_boolean (value) ;
      g_object_notify (object, "sensitive") ;
      break ;
    }
  }

static void render (GtkCellRenderer *cr, GdkWindow *window, GtkWidget *widget, GdkRectangle *background_area, GdkRectangle *cell_area, GdkRectangle *expose_area, GtkCellRendererState flags)
  {
  if (!(QCAD_CELL_RENDERER_TEXT (cr)->sensitive))
    flags = GTK_CELL_RENDERER_INSENSITIVE ;

  GTK_CELL_RENDERER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL_RENDERER_TEXT)))->render (cr, window, widget, background_area, cell_area, expose_area, flags) ;
  }

static GtkCellEditable *start_editing (GtkCellRenderer *cell, GdkEvent *event, GtkWidget *widget, const gchar *path, GdkRectangle *background_area, GdkRectangle *cell_area, GtkCellRendererState flags)
  {
  GtkCellEditable *ce = NULL ;

  if (!QCAD_CELL_RENDERER_TEXT (cell)->sensitive) return NULL ;

  if (NULL != (ce = (GTK_CELL_RENDERER_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL_RENDERER_TEXT))))->start_editing (cell, event, widget, path, background_area, cell_area, flags)))
    g_signal_emit (G_OBJECT (cell), cell_renderer_text_signals[QCAD_CELL_RENDERER_TEXT_EDITING_STARTED_SIGNAL], 0, ce, path) ;

  return ce ;
  }

void
g_cclosure_user_marshal_VOID__OBJECT_STRING (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data)
  {
  typedef void (*GMarshalFunc_VOID__OBJECT_STRING) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
  register GMarshalFunc_VOID__OBJECT_STRING callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
    data1 = closure->data;
    data2 = g_value_get_object (param_values + 0);
    }
  else
    {
    data1 = g_value_get_object (param_values + 0);
    data2 = closure->data;
    }

  callback = (GMarshalFunc_VOID__OBJECT_STRING) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_object (param_values + 1),
            (char *)g_value_get_string (param_values + 2),
            data2);
  }
#endif /* (GTK_MINOR_VERSION <= 4) */
