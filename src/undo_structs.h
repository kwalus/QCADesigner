#ifndef _UNDO_STRUCTS_H_
#define _UNDO_STRUCTS_H_

#include <glib.h>
#include "gqcell.h"

// The various actions we can Un(Re)do
#define UA_NONE -1
#define UA_CREATE_CELLS 0
#define UA_CELL_PARAM_CHANGE 1

typedef struct
  {
  /* Determines both the forward and reverse application function */
  int fActionType ;
  /* Events such as create/delete are inverses of each other.
    Thus, the same application function can be used to deal with them.
	        Thus is better than defininf two different event types. */
  } UNDO_ACTION ;

typedef struct
  {
  UNDO_ACTION ua ;
  GQCell **ppqc ;
  int ic ;
  } UNDO_ACTION_CELLS ;

typedef struct
  {
  /* UNDO_ACTION_* structures must begin with an UNDO_ACTION structure
     so they may be cast as such a one */
  UNDO_ACTION_CELLS uac ;
  gboolean bInverse ;
  } UNDO_ACTION_CREATE_CELLS ;

typedef struct
  {
  qcell *history ;
  int icEntries ;
  int idx ;
  } CELL_HISTORY_STACK ;

#endif /* _UNDO_STRUCTS_H_ */
