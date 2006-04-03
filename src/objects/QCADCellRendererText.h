#ifndef _OBJECTS_QCADCellRendererText_H_
#define _OBJECTS_QCADCellRendererText_H_

#include <gtk/gtk.h>

#if (GTK_MINOR_VERSION <= 4)
G_BEGIN_DECLS

typedef struct _QCADCellRendererText      QCADCellRendererText ;
typedef struct _QCADCellRendererTextClass QCADCellRendererTextClass ;
#else
  #define QCADCellRendererText      GtkCellRendererText
  #define QCADCellRendererTextClass GtkCellRendererTextClass
#endif

#if (GTK_MINOR_VERSION <= 4)
struct _QCADCellRendererText
  {
  GtkCellRendererText parent_instance ;
  gboolean sensitive ;
  } ;

struct _QCADCellRendererTextClass
  {
  GtkCellRendererTextClass parent_class ;

  void (*editing_started) (QCADCellRendererText *cr, GtkCellEditable *ce, char *pszPath) ;
  } ;

GType qcad_cell_renderer_text_get_type () ;

  #define QCAD_TYPE_STRING_CELL_RENDERER_TEXT "QCADCellRendererText"
  #define QCAD_TYPE_CELL_RENDERER_TEXT (qcad_cell_renderer_text_get_type ())
  #define QCAD_CELL_RENDERER_TEXT_SENSITIVITY_SOURCE(object) (QCAD_CELL_RENDERER_TEXT((object)))
#else
  #define QCAD_TYPE_CELL_RENDERER_TEXT GTK_TYPE_CELL_RENDERER_TEXT
  #define QCAD_CELL_RENDERER_TEXT_SENSITIVITY_SOURCE(object) (GTK_CELL_RENDERER((object)))
#endif /* (GTK_MINOR_VERSION <= 4) */

#if (GTK_MINOR_VERSION <= 4)
  #define QCAD_CELL_RENDERER_TEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CELL_RENDERER_TEXT, QCADCellRendererText))
  #define QCAD_IS_CELL_RENDERER_TEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CELL_RENDERER_TEXT))
  #define QCAD_CELL_RENDERER_TEXT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_CELL_RENDERER_TEXT, QCADCellRendererTextClass))
  #define QCAD_CELL_RENDERER_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_CELL_RENDERER_TEXT, QCADCellRendererTextClass))
  #define QCAD_IS_CELL_RENDERER_TEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_CELL_RENDERER_TEXT))

G_END_DECLS
#endif /* (GTK_MINOER_VERSION <= 4) */

#endif /* def _OBJECTS_QCADCellRendererText_H_ */
