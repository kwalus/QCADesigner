//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// #include <sunperf.h>

//-------------------------------------------------------------------//

#include "simulation.h"
#include "cad.h"
#include "stdqcell.h"
#include "vector_table.h"
#include "gqcell.h"

#define DBG_SQC(s)

extern cell_OP cell_options ;
extern double grid_spacing ;

/* Finds the first (of possibly more than one) cell that gqc overlaps) */
GQCell *find_overlapping (GQCell *first_cell, GQCell *gqc)
  {
  GQCell *cell = first_cell ;
  
  while (NULL != cell)
    {
    if (cell != gqc && gqcell_overlaps (cell, gqc))
      return cell ;
    cell = cell->next ;
    }
  
  return NULL ;
  }

// -- Clears the entire design space -- //
void clear_all_cells(GQCell **pfirst_cell, GQCell **plast_cell)
{
    DBG_SQC (int ic = 0) ;
    GQCell *cell = (*pfirst_cell);
    GQCell *next = NULL;
    
    DBG_SQC (fprintf (stderr, "Entering clear_all_cells\n")) ;

    // -- free the memory from the cell list -- //
    while (cell != NULL) {
        DBG_SQC (fprintf (stderr, "Unlinking %d\n", ++ic)) ;
	next = cell->next;
        DBG_SQC (fprintf (stderr, "Unlinking cell at 0x%08X\n", (int)cell)) ;
        gqcell_unlink (cell) ;
	cell = next;
    }

    (*pfirst_cell) = NULL;
    (*plast_cell) = NULL;

    DBG_SQC (fprintf (stderr, "Leaving clear_all_cells\n")) ;
	
}//clear_all_cells

// adds a qcell with parameters from the qcell argument //
GQCell *add_stdqcell(GQCell **pfirst_cell, GQCell **plast_cell, GQCell *template, gboolean check_overlap, double x, double y, cell_OP *pco, int type)
  {
  GQCell *gqcRet = gqcell_new (template, x, y, pco, type) ;
  
  DBG_SQC (fprintf (stderr, "Entering add_stdqcell\n")) ;
  
  if (check_overlap && NULL != find_overlapping ((*pfirst_cell), gqcRet))
    {
    DBG_SQC (fprintf (stderr, "add_stdqcell: Found overlapping qcell !\n")) ;
    g_object_unref (gqcRet) ;
    return NULL ;
    }
  
  DBG_SQC (fprintf (stderr, "add_stdqcell: qcell does not overlap !\n")) ;
  
  /* Linking the new cell into the array */
  if (NULL == (*pfirst_cell) && NULL == (*plast_cell))
    {
    gqcRet->prev = gqcRet->next = NULL ;
    (*pfirst_cell) = (*plast_cell) = gqcRet ;
    }
  else /* Append the cell to the end of the list */
    {
    gqcRet->prev = (*plast_cell) ;
    (*plast_cell)->next = gqcRet ;
    gqcRet->next = NULL ;
    (*plast_cell) = gqcRet ;
    }
  
  return gqcRet ;
}//add_stdqcell


// -- creates a linear array of cells from one point to another -- //
int create_array_of_cells(GQCell **pfirst_cell, GQCell **plast_cell, double x0, double y0, double x1, double y1, cell_OP *pco, int type, GQCell ***pppqcNewCells, gboolean bSnapToGrid)
{
    GQCell *pqc = NULL;
    int icNewCells = 0 ;
    double x;
    double y;
    double temp;
    double cell_x, cell_y ;
    // Default cell width is TYPE_1
    double cell_width = (TYPE_2 == type) ? pco->type_2_cell_width : pco->type_1_cell_width ;
    double cell_height = (TYPE_2 == type) ? pco->type_2_cell_height : pco->type_1_cell_height ;
    double inc = 0 ;
    int Nix ;
		
    if (NULL != pppqcNewCells) (*pppqcNewCells) = NULL ;

    if (x0 > x1) {
      temp = x1;
      x1 = x0;
      x0 = temp;
    }

    if (y0 > y1) {
      temp = y1;
      y1 = y0;
      y0 = temp;
    }
    // if the user drew a horizontal line //
    if ((y0 - y1) == 0) {
      if (bSnapToGrid)
        {
        x = grid_world_x (calc_world_x (x0)) ;
        inc = ceil (cell_width / grid_spacing) * grid_spacing ;
        }
      else
        {
        x = calc_world_x (x0) ;
        // cells have to be /some/ distance apart - might as well be 1 nm
        inc = cell_width + 1.0 ;
        }
      icNewCells = (int)ceil ((calc_world_x (x1) - calc_world_x (x0)) / inc) ;
      
      if (NULL != pppqcNewCells)
        (*pppqcNewCells) = malloc (icNewCells * sizeof (GQCell *)) ;
      
      for (Nix = 0 ; Nix < icNewCells; Nix++, x += inc) {

	cell_x = grid_world_x(x);
	cell_y = grid_world_y(calc_world_y(y0));
        
        pqc = add_stdqcell(pfirst_cell, plast_cell, NULL, TRUE, cell_x, cell_y, pco, type) ;

	if (NULL != pppqcNewCells)
	  (*pppqcNewCells)[Nix] = pqc ;
      }

	// if the user drew a vertical line //
    } else if ((x0 - x1) == 0) {
      if (bSnapToGrid)
        {
        y = grid_world_y (calc_world_y (y0)) ;
        inc = ceil (cell_height / grid_spacing) * grid_spacing ;
        }
      else
        {
        y = calc_world_y (y0) ;
        // cells have to be /some/ distance apart - might as well be 1 nm
        inc = cell_height + 1.0 ;
        }
      icNewCells = (int)ceil ((calc_world_y (y1) - calc_world_y (y0)) / inc) ;
      
      if (NULL != pppqcNewCells)
        (*pppqcNewCells) = malloc (icNewCells * sizeof (GQCell *)) ;
        
      for (Nix = 0 ; Nix < icNewCells ; Nix++, y += inc) {
        cell_x = grid_world_x(calc_world_x(x0));
        cell_y = grid_world_y(y);

        pqc = add_stdqcell(pfirst_cell, plast_cell, NULL, TRUE, cell_x, cell_y, pco, type) ;

	if (NULL != pppqcNewCells)
	  (*pppqcNewCells)[Nix] = pqc ;
      }
    }

return icNewCells ;
}

//-------------------------------------------------------------------//
// -- Mirrors Cells About the given line -- //

int mirror_cells_about_line(GQCell **pfirst_cell, GQCell **plast_cell, int mirror_x0, int mirror_y0, int mirror_x1, int mirror_y1, GQCell **selected_cells, int number_of_selected_cells, cell_OP *pco, int type, GQCell ***pppqcNewCells){

  int i = 0;
  GQCell *gqcNew = NULL ;
  float m_x0, m_x1, m_y0, m_y1 ;
  
  if (NULL != pppqcNewCells)
    (*pppqcNewCells) = malloc (number_of_selected_cells * sizeof (GQCell *)) ;

  m_x0 = grid_world_x (calc_world_x(mirror_x0));
  m_x1 = grid_world_x (calc_world_x(mirror_x1));
  m_y0 = grid_world_y (calc_world_y(mirror_y0));
  m_y1 = grid_world_y (calc_world_y(mirror_y1));

  //x' = x0 + 2(a-x0)

  // mirror horizontally //
  if (m_x0 == m_x1) {

    // -- mirror each of the selected cells -- //
    for (i = 0; i < number_of_selected_cells; i++) {

      g_assert(selected_cells[i] != NULL);
      
      gqcNew = gqcell_copy (selected_cells[i]) ;
      
      gqcell_move_by_offset (gqcNew, 2 * (m_x0 - selected_cells[i]->x), 0) ;
      
      DBG_SQC (fprintf (stderr, "qcell is now at (%lf,%lf)\n", gqcNew->x, gqcNew->y)) ;
      
      if (NULL != find_overlapping ((*pfirst_cell), gqcNew))
        {
        g_object_unref (gqcNew) ;
        gqcNew = NULL ;
        }

      if (NULL != pppqcNewCells)
        (*pppqcNewCells)[i] = gqcNew ;
      
      if (NULL != gqcNew)
        {
        gqcNew->prev = (*plast_cell) ;
        (*plast_cell)->next = gqcNew ;
        gqcNew->next = NULL ;
        (*plast_cell) = gqcNew ;
        }
    }

  // mirror vertically
  } else {
    // -- mirror each of the selected cells -- //
    for (i = 0; i < number_of_selected_cells; i++) {

      g_assert(selected_cells[i] != NULL);
      
      gqcNew = gqcell_copy (selected_cells[i]);
      
      gqcell_move_by_offset (gqcNew, 0, 2 * (m_y0 - selected_cells[i]->y)) ;
      
      if (NULL != find_overlapping ((*pfirst_cell), gqcNew))
        {
        g_object_unref (gqcNew) ;
        gqcNew = NULL ;
        }
      
      if (NULL != pppqcNewCells)
        (*pppqcNewCells)[i] = gqcNew ;

      if (NULL != gqcNew)
        {
        gqcNew->prev = (*plast_cell) ;
        (*plast_cell)->next = gqcNew ;
        gqcNew->next = NULL ;
        (*plast_cell) = gqcNew ;
        }
    }
  }

return number_of_selected_cells ;

}//mirror_cells_about_line

void delete_stdqcell (GQCell *gqc, VectorTable *pvt, GQCell **p_first_cell, GQCell **p_last_cell)
  {
  if (gqc->is_input)
    VectorTable_del_input (pvt, gqc) ;
  if ((*p_first_cell) == gqc)
    (*p_first_cell) = gqc->next ;
  if ((*p_last_cell) == gqc)
    (*p_last_cell) = gqc->prev ;
  gqcell_unlink (gqc) ;
  }

// sets a cell as an input cell //
int get_clock_from_selected_cells (GQCell **selected_cells, int number_of_selected_cells)
  {
  int Nix ;
  int iClock = -1 ;
  
  for (Nix = 0 ; Nix < number_of_selected_cells ; Nix++)
    if (NULL != selected_cells[Nix])
      {
      iClock = selected_cells[Nix]->clock ;
      break ;
      }
  for (; Nix < number_of_selected_cells ; Nix++)
    if (NULL != selected_cells[Nix])
      {
      if (selected_cells[Nix]->clock != iClock)
      return -1 ;
      }
  return iClock ;
  }
