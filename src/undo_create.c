#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "undo_structs.h"
#include "undo_create.h"

#define DBG_UC(s)
#define DBG_UC_CLR(s)

static void *DoCreateCells (GQCell **ppqc, int ic, gboolean bInverse) ;
static void CreateCellsAction (UNDO_ACTION_CELLS *puac, GQCell **ppqc, int ic) ;
static void AddHistoryItem (GQCell *gqc) ;
static void DestroyHistory (gpointer p) ;

void *Undo_CreateAction_CreateCells (GQCell **ppqc, int ic)
  {
  return DoCreateCells (ppqc, ic, FALSE) ;
  }

void *Undo_CreateAction_DeleteCells (GQCell **ppqc, int ic)
  {
  return DoCreateCells (ppqc, ic, TRUE) ;
  }

static void *DoCreateCells (GQCell **ppqc, int ic, gboolean bInverse)
  {
  UNDO_ACTION_CREATE_CELLS *puacc = g_malloc (sizeof (UNDO_ACTION_CREATE_CELLS)) ;
  
  CreateCellsAction ((UNDO_ACTION_CELLS *)puacc, ppqc, ic) ;
  puacc->uac.ua.fActionType = UA_CREATE_CELLS ;
  puacc->bInverse = bInverse ;
  
  return (void *)puacc ;
  }

void *Undo_CreateAction_CellParamChange (GQCell **ppqc, int ic)
  {
  UNDO_ACTION_CELLS *puac = g_malloc (sizeof (UNDO_ACTION_CELLS)) ;
  
  DBG_UC (fprintf (stderr, "Creating ParamChange action\n")) ;
  
  CreateCellsAction (puac, ppqc, ic) ;
  puac->ua.fActionType = UA_CELL_PARAM_CHANGE ;
  return (void *)puac ;
  }

static void CreateCellsAction (UNDO_ACTION_CELLS *puac, GQCell **ppqc, int ic)
  {
  int Nix ;
  CELL_HISTORY_STACK *phistack = NULL ;
  
  // We cannot know specifically what action we're creating, so we set the flag to 
  puac->ua.fActionType = UA_NONE ;
  puac->ppqc = malloc (ic * sizeof (GQCell *)) ;
  puac->ic = ic ;
  memcpy (puac->ppqc, ppqc, ic * sizeof (GQCell *)) ;
  
  for (Nix = 0 ; Nix < ic ; Nix++)
    if (NULL != ppqc[Nix])
      {
      g_object_ref (ppqc[Nix]) ;
      if (NULL == (phistack = g_object_get_data (G_OBJECT (ppqc[Nix]), "phistack")))
        InitUndoHistory (ppqc[Nix]) ;
      else
        AddHistoryItem (ppqc[Nix]) ;
      }
  }

void InitUndoHistory (GQCell *gqc)
  {
  CELL_HISTORY_STACK *phistack = g_malloc (sizeof (CELL_HISTORY_STACK)) ;

  phistack->history = NULL ;
  phistack->icEntries = 0 ;
  phistack->idx = -1 ;
  DBG_UC (fprintf (stderr, "About to set the history for cell 0x%08x\n", (int)gqc)) ;
  g_object_set_data_full (G_OBJECT (gqc), "phistack", phistack, (GDestroyNotify)DestroyHistory) ;
  DBG_UC (fprintf (stderr, "Done setting the cell's history\n")) ;
  
  AddHistoryItem (gqc) ;
  }

static void AddHistoryItem (GQCell *gqc)
  {
  int Nix ;
  CELL_HISTORY_STACK *phistack = (CELL_HISTORY_STACK *)g_object_get_data (G_OBJECT (gqc), "phistack") ;
  
  if (phistack->idx == phistack->icEntries - 1)
    {
    phistack->history = g_realloc (phistack->history, (++(phistack->icEntries)) * sizeof (qcell)) ;
    memset (&(phistack->history[(phistack->icEntries) - 1]), 0, sizeof (qcell)) ;
    }
  
  (phistack->idx)++ ;
  
  DBG_UC (fprintf (stderr, "Adding %s to position %d\n", gqc->label, phistack->idx)) ;
  
  phistack->history[(phistack->idx)].x = gqc->x ;
  phistack->history[(phistack->idx)].y = gqc->y ;
  phistack->history[(phistack->idx)].top_x = gqc->top_x ;
  phistack->history[(phistack->idx)].bot_x = gqc->bot_x ;
  phistack->history[(phistack->idx)].top_y = gqc->top_y ;
  phistack->history[(phistack->idx)].bot_y = gqc->bot_y ;
  phistack->history[(phistack->idx)].cell_width = gqc->cell_width ;
  phistack->history[(phistack->idx)].cell_height = gqc->cell_height ;
  phistack->history[(phistack->idx)].orientation = gqc->orientation ;
  phistack->history[(phistack->idx)].number_of_dots = gqc->number_of_dots ;
  
  if (NULL == phistack->history[(phistack->idx)].cell_dots)
    phistack->history[(phistack->idx)].cell_dots = g_malloc (gqc->number_of_dots * sizeof (qdot)) ;
  
  for (Nix = 0 ; Nix < phistack->history[(phistack->idx)].number_of_dots ; Nix++)
    {
    phistack->history[(phistack->idx)].cell_dots[Nix].x = gqc->cell_dots[Nix].x ;
    phistack->history[(phistack->idx)].cell_dots[Nix].y = gqc->cell_dots[Nix].y ;
    phistack->history[(phistack->idx)].cell_dots[Nix].diameter = gqc->cell_dots[Nix].diameter ;
    phistack->history[(phistack->idx)].cell_dots[Nix].charge = gqc->cell_dots[Nix].charge ;
    phistack->history[(phistack->idx)].cell_dots[Nix].spin = gqc->cell_dots[Nix].spin ;
    phistack->history[(phistack->idx)].cell_dots[Nix].potential = gqc->cell_dots[Nix].potential ;
    }

  DBG_UC_CLR(fprintf (stderr, "AddHistoryItem: Adding item with colour %d\n", gqc->color)) ;
  phistack->history[(phistack->idx)].color = gqc->color ;
  phistack->history[(phistack->idx)].clock = gqc->clock ;
  phistack->history[(phistack->idx)].is_input = gqc->is_input ;
  phistack->history[(phistack->idx)].is_output = gqc->is_output ;
  phistack->history[(phistack->idx)].is_fixed = gqc->is_fixed ;
  
  if (NULL != phistack->history[(phistack->idx)].label)
    g_free (phistack->history[(phistack->idx)].label) ;
  
  phistack->history[(phistack->idx)].label = g_strdup (gqc->label) ;
  }

static void DestroyHistory (gpointer p)
  {
  int Nix ;
  
  DBG_UC (fprintf (stderr, "Destroying cell history\n")) ;
  
  CELL_HISTORY_STACK *phistack = (CELL_HISTORY_STACK *)p ;
  
  for (Nix = 0 ; Nix < phistack->icEntries ; Nix++)
    {
    g_free (phistack->history[Nix].label) ;
    g_free (phistack->history[Nix].cell_dots) ;
    }
  
  g_free (phistack->history) ;
  g_free (p) ;

  DBG_UC (fprintf (stderr, "Done destroying cell history\n")) ;
  }
