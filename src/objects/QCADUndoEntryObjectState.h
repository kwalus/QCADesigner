#ifndef _OBJECTS_QCADUndoEntryObjectState_H_
#define _OBJECTS_QCADUndoEntryObjectState_H_

#ifdef UNDO_REDO

#include "../exp_array.h"
#include "QCADUndoEntry.h"

#ifdef __cplusplus
extern "C" {
#endif /* def __cplusplus */

typedef struct _QCADUndoEntryObjectState      QCADUndoEntryObjectState ;
typedef struct _QCADUndoEntryObjectStateClass QCADUndoEntryObjectStateClass ;

struct _QCADUndoEntryObjectState
  {
  QCADUndoEntry parent_instance ;

  GObject *instance ;
  EXP_ARRAY *parameters_delta ;
  gboolean bFrozen ;
  } ;

struct _QCADUndoEntryObjectStateClass
  {
  QCADUndoEntryClass parent_class ;
  } ;

GType qcad_undo_entry_object_state_get_type () ;

#define QCAD_TYPE_STRING_UNDO_ENTRY_OBJECT_STATE "QCADUndoEntryObjectState"
#define QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE (qcad_undo_entry_object_state_get_type ())
#define QCAD_UNDO_ENTRY_OBJECT_STATE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE, QCADUndoEntryObjectState))
#define QCAD_IS_UNDO_ENTRY_OBJECT_STATE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE))
#define QCAD_UNDO_ENTRY_OBJECT_STATE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE, QCADUndoEntryObjectStateClass))
#define QCAD_UNDO_ENTRY_OBJECT_STATE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST    ((klass),  QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE, QCADUndoEntryObjectStateClass))
#define QCAD_IS_UNDO_ENTRY_OBJECT_STATE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE))

QCADUndoEntry *qcad_undo_entry_object_state_new (GObject *instance) ;
gboolean qcad_undo_entry_object_state_get_changed (QCADUndoEntryObjectState *undo_entry_os) ;

#ifdef __cplusplus
}
#endif /* def __cplusplus */

#endif /* def UNDO_REDO */

#endif /* _OBJECTS_QCADUndoEntryObjectState_H_ */
