#ifndef _UNDO_CREATE_H_
#define _UNDO_CREATE_H_

#include "gqcell.h"

void *Undo_CreateAction_CreateCells (GQCell **ppqc, int ic) ;
void *Undo_CreateAction_DeleteCells (GQCell **ppqc, int ic) ;
void *Undo_CreateAction_CellParamChange (GQCell **ppqc, int ic) ;
void InitUndoHistory (GQCell *gqc) ;

#endif /* _UNDO_CREATE_H_ */
