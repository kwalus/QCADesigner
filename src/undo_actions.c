#include "undo_actions.h"
#include "undo_structs.h"
#include "cad.h"
#include "stdqcell.h"
#include "callback_helpers.h"
#include "vector_table.h"

#define DBG_UDA(s)

extern VectorTable *pvt ;

static void RestoreHistoryItem (GQCell *gqc, CELL_HISTORY_STACK *phistack) ;

void UndoAction_CreateCells (void *p)
  {
  UNDO_ACTION_CREATE_CELLS *puacc = (UNDO_ACTION_CREATE_CELLS *)p ;
  if (puacc->bInverse)
    gui_create_cells (puacc->uac.ppqc, puacc->uac.ic) ;
  else
    gui_delete_cells (puacc->uac.ppqc, puacc->uac.ic) ;
  }

void RedoAction_CreateCells (void *p)
  {
  UNDO_ACTION_CREATE_CELLS *puacc = (UNDO_ACTION_CREATE_CELLS *)p ;
  if (puacc->bInverse)
    gui_delete_cells (puacc->uac.ppqc, puacc->uac.ic) ;
  else
    gui_create_cells (puacc->uac.ppqc, puacc->uac.ic) ;
  }

void UndoAction_CellParamChange (void *p)
  {
  int Nix ;
  UNDO_ACTION_CELLS *puac = (UNDO_ACTION_CELLS *)p ;
  CELL_HISTORY_STACK *phistack = NULL ;
  
  DBG_UDA (fprintf (stderr, "Inside UndoAction_CellParamChange:  This action has %d cells\n", puac->ic)) ;
  
  for (Nix = 0 ; Nix < puac->ic ; Nix++)
    if (NULL != puac->ppqc[Nix])
      {
      phistack = (CELL_HISTORY_STACK *)g_object_get_data (G_OBJECT (puac->ppqc[Nix]), "phistack") ;
      (phistack->idx)-- ;
      DBG_UDA (fprintf (stderr, "About to call RestoreHistoryItem on history idx %d\n", phistack->idx)) ;
      if (phistack->idx > -1)
        RestoreHistoryItem (puac->ppqc[Nix], phistack) ;
      }
  }

void RedoAction_CellParamChange (void *p)
  {
  int Nix ;
  UNDO_ACTION_CELLS *puac = (UNDO_ACTION_CELLS *)p ;
  CELL_HISTORY_STACK *phistack = NULL ;
  
  
  DBG_UDA (fprintf (stderr, "Inside RedoAction_CellParamChange\n")) ;
  
  for (Nix = 0 ; Nix < puac->ic ; Nix++)
    if (NULL != puac->ppqc[Nix])
      {
      phistack = (CELL_HISTORY_STACK *)g_object_get_data (G_OBJECT (puac->ppqc[Nix]), "phistack") ;
      (phistack->idx)++ ;
      if (phistack->idx < phistack->icEntries)
        RestoreHistoryItem (puac->ppqc[Nix], phistack) ;
      }
  }

static void RestoreHistoryItem (GQCell *gqc, CELL_HISTORY_STACK *phistack)
  {
  int Nix ;
  
  DBG_UDA (fprintf (stderr, "RHI:Restoring history item %d for cell 0x%08X (total items:%d)\n", phistack->idx, (int)gqc, phistack->icEntries)) ;
  DBG_UDA (fprintf (stderr, "RHI:Cell is currently at (%lf,%lf)\n", gqc->x, gqc->y)) ;

  gqc->x = phistack->history[(phistack->idx)].x ;
  gqc->y = phistack->history[(phistack->idx)].y ;
  gqc->top_x = phistack->history[(phistack->idx)].top_x ;
  gqc->bot_x = phistack->history[(phistack->idx)].bot_x ;
  gqc->top_y = phistack->history[(phistack->idx)].top_y ;
  gqc->bot_y = phistack->history[(phistack->idx)].bot_y ;
  gqc->cell_width = phistack->history[(phistack->idx)].cell_width ;
  gqc->cell_height = phistack->history[(phistack->idx)].cell_height ;
  gqc->orientation = phistack->history[(phistack->idx)].orientation ;
  gqc->number_of_dots = phistack->history[(phistack->idx)].number_of_dots ;
  
  if (NULL == gqc->cell_dots)
    gqc->cell_dots = g_malloc (phistack->history[(phistack->idx)].number_of_dots * sizeof (qdot)) ;
  
  for (Nix = 0 ; Nix < gqc->number_of_dots ; Nix++)
    {
    gqc->cell_dots[Nix].x = phistack->history[(phistack->idx)].cell_dots[Nix].x ;
    gqc->cell_dots[Nix].y = phistack->history[(phistack->idx)].cell_dots[Nix].y ;
    gqc->cell_dots[Nix].diameter = phistack->history[(phistack->idx)].cell_dots[Nix].diameter ;
    gqc->cell_dots[Nix].charge = phistack->history[(phistack->idx)].cell_dots[Nix].charge ;
    gqc->cell_dots[Nix].spin = phistack->history[(phistack->idx)].cell_dots[Nix].spin ;
    gqc->cell_dots[Nix].potential = phistack->history[(phistack->idx)].cell_dots[Nix].potential ;
    }

  gqc->color = phistack->history[(phistack->idx)].color ;
  gqc->clock = phistack->history[(phistack->idx)].clock ;
  
  if (phistack->history[(phistack->idx)].is_input && !(gqc->is_input))
    VectorTable_add_input (pvt, gqc) ;
  else if (!(phistack->history[(phistack->idx)].is_input) && gqc->is_input)
    VectorTable_del_input (pvt, gqc) ;
  
  gqc->is_input = phistack->history[(phistack->idx)].is_input ;
  gqc->is_output = phistack->history[(phistack->idx)].is_output ;
  gqc->is_fixed = phistack->history[(phistack->idx)].is_fixed ;
  if (NULL != gqc->label) g_free (gqc->label) ;
  gqc->label = g_strdup (phistack->history[(phistack->idx)].label) ;
  
  DBG_UDA (fprintf (stderr, "RHI:Cell is now at (%lf,%lf)\n", gqc->x, gqc->y)) ;
  }
