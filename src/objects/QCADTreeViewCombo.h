#ifndef _OBJECTS_QCADTreeViewCombo_H_
#define _OBJECTS_QCADTreeViewCombo_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADTreeViewCombo      QCADTreeViewCombo ;
typedef struct _QCADTreeViewComboClass QCADTreeViewComboClass ;

typedef char *(*QCADTreeViewComboLabelTextFunction) (GtkTreeView *tv) ;

struct _QCADTreeViewCombo
  {
  GtkTable parent_instance ;
  GtkWidget *entry ;
  } ;

struct _QCADTreeViewComboClass
  {
  GtkTableClass parent_class ;
  } ;

GType qcad_tree_view_combo_get_type () ;

#define QCAD_TYPE_STRING_TREE_VIEW_COMBO "QCADTreeViewCombo"
#define QCAD_TYPE_TREE_VIEW_COMBO (qcad_tree_view_combo_get_type ())
#define QCAD_TREE_VIEW_COMBO(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_TREE_VIEW_COMBO, QCADTreeViewCombo))
#define QCAD_IS_TREE_VIEW_COMBO(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_TREE_VIEW_COMBO))
#define QCAD_TREE_VIEW_COMBO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_TREE_VIEW_COMBO, QCADTreeViewComboClass))
#define QCAD_TREE_VIEW_COMBO_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_TREE_VIEW_COMBO, QCADTreeViewComboClass))
#define QCAD_IS_TREE_VIEW_COMBO_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_TREE_VOEW_COMBO))

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* _OBJECTS_QCADTreeViewCombo_H_ */
