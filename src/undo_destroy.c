#include <glib.h>
#include <stdlib.h>
#include "undo_destroy.h"

#define DBG_UD(s)

static void DestroyCellsAction (UNDO_ACTION_CELLS *puac) ;

void DestroyAction_CreateCells (void *p)
  {
  DestroyCellsAction ((UNDO_ACTION_CELLS *)p) ;
  }

void DestroyAction_CellParamChange (void *p)
  {
  DestroyCellsAction ((UNDO_ACTION_CELLS *)p) ;
  }

static void DestroyCellsAction (UNDO_ACTION_CELLS *puac)
  {
  int Nix ;
  
  for (Nix = 0 ; Nix < puac->ic ; Nix++)
    if (NULL != puac->ppqc[Nix])
      g_object_unref (G_OBJECT (puac->ppqc[Nix])) ;
  
  free (puac->ppqc) ;
  free (puac) ;
  }
