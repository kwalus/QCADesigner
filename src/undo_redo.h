#ifndef _UNDO_REDO_H_
#define _UNDO_REDO_H_

#include <glib.h>

gboolean Undo_CanUndo () ;
/* The return value is the answer to the question "Are there any more undo events ?" */
void Undo_Undo () ;
gboolean Undo_CanRedo () ;
/* The return value is the answer to the question "Are there any more redo events ?" */
void Undo_Redo () ;
void Undo_AddAction (void *p) ;
void Undo_Clear () ;

#endif /* _UNDO_REDO_H_ */
