#ifndef _UNDO_ACTIONS_H_
#define _UNDO_ACTIONS_H_

void UndoAction_CreateCells (void *p) ;
void RedoAction_CreateCells (void *p) ;
void RedoAction_CellParamChange (void *p) ;
void UndoAction_CellParamChange (void *p) ;

#endif /* _UNDO_ACTIONS_H_ */
