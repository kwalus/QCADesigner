#include <stdio.h>
#include <stdlib.h>
#include "undo_redo.h"
#include "undo_actions.h"
#include "undo_structs.h"
#include "undo_create.h"
#include "undo_destroy.h"

#define DBG_UR(s)

typedef struct
  {
  int icEntries ;
  void **pStart ;
  int idx ;
  } UNDO_STACK ;

static void DestroyAction (void *p) ;
static void UndoAction (void *p) ;
static void RedoAction (void *p) ;

static UNDO_STACK ur = {0, NULL, -1} ;

gboolean Undo_CanUndo ()
  {
  return (ur.idx > -1) ;
  }

gboolean Undo_CanRedo ()
  {
  gboolean bRet = (ur.idx < ur.icEntries - 1) ;

  if (bRet) bRet = bRet && (NULL != ur.pStart[ur.idx + 1]) ;

  return bRet ;
  }

void Undo_Undo ()
  {
  if (-1 == ur.idx) return ;
  UndoAction (ur.pStart[ur.idx]) ;
  ur.idx-- ;
  }

void Undo_Redo ()
  {
  ur.idx++ ;
  if (ur.icEntries == ur.idx) return ;
  RedoAction (ur.pStart[ur.idx]) ;
  }

void Undo_AddAction (void *p)
  {
  int Nix ;
  
  DBG_UR (fprintf (stderr, "Entering Undo_AddAction ()\n")) ;
  
  if (ur.idx == ur.icEntries - 1)
    {
    DBG_UR (fprintf (stderr, "ur.idx has reached ur.icEntries, so reallocating\n")) ;
    ur.pStart = realloc (ur.pStart, (++(ur.icEntries)) * sizeof (void *)) ;
    ur.pStart[ur.icEntries - 1] = NULL ;
    }
  
  DBG_UR (fprintf (stderr, "About to destroy event indices %d through %d\n", ur.idx + 1, ur.icEntries - 1)) ;
  
  for (Nix = ++(ur.idx) ; Nix < ur.icEntries ; Nix++)
    {
    DestroyAction (ur.pStart[Nix]) ;
    ur.pStart[Nix] = NULL ;
    }
  
  DBG_UR (fprintf (stderr, "Setting entry %d (ur.icEntries = %d) to the event\n", ur.idx, ur.icEntries)) ;
  
  ur.pStart[ur.idx] = p ;
  }

void Undo_Clear ()
  {
  int Nix ;
  
  for (Nix = 0 ; Nix < ur.icEntries ; Nix++)
    if (NULL != ur.pStart[Nix])
      {
      DestroyAction (ur.pStart[Nix]) ;
      ur.pStart[Nix] = NULL ;
      }
  ur.idx = -1 ;
  }

static void DestroyAction (void *p)
  {
  if (NULL == p) return ;
  switch (((UNDO_ACTION *)p)->fActionType)
    {
    case UA_CREATE_CELLS:
      DestroyAction_CreateCells (p) ;
      break ;
    
    case UA_CELL_PARAM_CHANGE:
      DestroyAction_CellParamChange (p) ;
      break ;
    }
  }

static void UndoAction (void *p)
  {
  DBG_UR (fprintf (stderr, "In UndoAction: ((UNDO_ACTION *)p)->fActionType = %d\n", ((UNDO_ACTION *)p)->fActionType)) ;
  switch (((UNDO_ACTION *)p)->fActionType)
    {
    case UA_CREATE_CELLS:
      DBG_UR (fprintf (stderr, "In UndoAction: Calling UndoAction_CreateCells\n")) ;
      UndoAction_CreateCells (p) ;
      break ;
    
    case UA_CELL_PARAM_CHANGE:
      DBG_UR (fprintf (stderr, "In UndoAction: Calling UndoAction_CellParamChange\n")) ;
      UndoAction_CellParamChange (p) ;
      break ;
    }
  }

static void RedoAction (void *p)
  {
  DBG_UR (fprintf (stderr, "In RedoAction: ((UNDO_ACTION *)p)->fActionType = %d\n", ((UNDO_ACTION *)p)->fActionType)) ;
  switch (((UNDO_ACTION *)p)->fActionType)
    {
    case UA_CREATE_CELLS:
      DBG_UR (fprintf (stderr, "In RedoAction: Calling RedoAction_CreateCells\n")) ;
      RedoAction_CreateCells (p) ;
      break ;
    
    case UA_CELL_PARAM_CHANGE:
      DBG_UR (fprintf (stderr, "In RedoAction: Calling RedoAction_CellParamChange\n")) ;
      RedoAction_CellParamChange (p) ;
      break ;
    }
  }
