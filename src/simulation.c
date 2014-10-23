//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// The main entry point for the simulation engines, as  //
// well as functions common to all engines.             //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <glib.h>

#include "global_consts.h"
#include "objects/QCADCell.h"
#include "simulation.h"
#include "coherence_vector.h"
#include "bistable_simulation.h"
#include "semi_coherent.h"
#include "three_state_coherence.h"
#include "ts_field_clock.h"
#include "vector_table.h"

extern coherence_OP coherence_options ;
extern bistable_OP bistable_options ;
extern ts_coherence_OP ts_coherence_options ;
#ifdef HAVE_FORTRAN
extern semi_coherent_OP semi_coherent_options ;
extern ts_fc_OP ts_fc_options;
#endif /* HAVE_FORTRAN */
gboolean STOP_SIMULATION = FALSE;

// -- this is the main simulation procedure -- //
simulation_data *run_simulation (int sim_engine, int sim_type, DESIGN *design, VectorTable *pvt)
  {
  switch (sim_engine)
    {
    case BISTABLE:
      return run_bistable_simulation(sim_type, design, &bistable_options, pvt);

    case COHERENCE_VECTOR:
      return run_coherence_simulation(sim_type, design, &coherence_options, pvt);
			
    case TS_COHERENCE_VECTOR:
      return run_ts_coherence_simulation(sim_type, design, &ts_coherence_options, pvt);

#ifdef HAVE_FORTRAN
    case SEMI_COHERENT:
      return run_semi_coherent_simulation(sim_type, design, &semi_coherent_options, pvt);

    case TS_FIELD_CLOCK:
      return run_ts_fc_simulation(sim_type, design, &ts_fc_options, pvt);	
#endif /* HAVE_FORTRAN */
    }
  return NULL ;
  }//run_simualtion

//-------------------------------------------------------------------//
//!Finds all cells within a specified radius and sets the selected cells array//
int select_cells_in_radius(QCADCell ***sorted_cells,
  QCADCell *cell,
  double world_radius,
  int the_cells_layer,
  int number_of_cell_layers,
  int *number_of_cells_in_layer,
  double layer_separation,
  QCADCell ***p_selected_cells,
  int **p_neighbour_layer)
  {
  int i,j,k;
  int number_of_selected_cells = 0 ;

  g_assert (cell != NULL);

  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      if (sorted_cells[i][j] != cell)
        if (sqrt ((QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) *
                  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) +
                  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) *
                  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) +
                  ((double) ABS (the_cells_layer-i) * layer_separation) *
                  ((double) ABS (the_cells_layer-i) * layer_separation)) < world_radius)
          number_of_selected_cells++;

  if (number_of_selected_cells > 0 && !(NULL == p_selected_cells || NULL == p_neighbour_layer))
    {
    //printf("there were %d neighours\n", number_of_selected_cells);

    (*p_selected_cells) = g_malloc0 (sizeof (QCADCell *) * number_of_selected_cells);
    (*p_neighbour_layer) = g_malloc0 (sizeof (int) * number_of_selected_cells);	

    // catch any memory allocation errors //
    if ((*p_selected_cells) == NULL)
      {
      fprintf (stderr, "memory allocation error in select_cells_in_radius();\n");
      exit (1);
      }

    k = 0;
    for(i = 0; i < number_of_cell_layers; i++)
      for(j = 0; j < number_of_cells_in_layer[i]; j++)
        {
        if (sorted_cells[i][j] != cell)
          {
          if (sqrt ((QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) *
                    (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) +
                    (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) *
                    (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) +
                    ((double) ABS (the_cells_layer-i) * layer_separation) *
                    ((double) ABS (the_cells_layer-i) * layer_separation)) < world_radius)
            {
            //if(sorted_cells[i][j] == NULL)printf("sorted cells appear to have null member\n");
            (*p_selected_cells)[k] = sorted_cells[i][j];
            (*p_neighbour_layer)[k] = i;
            k++;
            }
          }
        }
    }

  return number_of_selected_cells;
  } //select_cells_in_radius

//!Finds all cells within a specified radius and sets the selected cells array//
int select_cells_in_radius_cv(QCADCell ***sorted_cells,
						   QCADCell *cell,
						   double world_radius,
						   int the_cells_layer,
						   int number_of_cell_layers,
						   int *number_of_cells_in_layer,
						   double layer_separation,
						   QCADCell ***p_selected_cells,
						   int **p_neighbour_layer,
						   int **p_neighbour_index)
{
	int i,j,k;
	int number_of_selected_cells = 0 ;
	
	g_assert (cell != NULL);
	
	for(i = 0; i < number_of_cell_layers; i++)
		for(j = 0; j < number_of_cells_in_layer[i]; j++)
			if (sorted_cells[i][j] != cell)
				if (sqrt ((QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) *
						  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) +
						  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) *
						  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) +
						  ((double) ABS (the_cells_layer-i) * layer_separation) *
						  ((double) ABS (the_cells_layer-i) * layer_separation)) < world_radius)
					number_of_selected_cells++;
	
	if (number_of_selected_cells > 0 && !(NULL == p_selected_cells || NULL == p_neighbour_layer))
    {
		//printf("there were %d neighours\n", number_of_selected_cells);
		
		(*p_selected_cells) = g_malloc0 (sizeof (QCADCell *) * number_of_selected_cells);
		(*p_neighbour_layer) = g_malloc0 (sizeof (int) * number_of_selected_cells);
		(*p_neighbour_index) = g_malloc0 (sizeof (int) * number_of_selected_cells);	
		
		// catch any memory allocation errors //
		if ((*p_selected_cells) == NULL)
		{
			fprintf (stderr, "memory allocation error in select_cells_in_radius();\n");
			exit (1);
		}
		
		k = 0;
		for(i = 0; i < number_of_cell_layers; i++)
			for(j = 0; j < number_of_cells_in_layer[i]; j++)
			{
				if (sorted_cells[i][j] != cell)
				{
					if (sqrt ((QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) *
							  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->x - QCAD_DESIGN_OBJECT (cell)->x) +
							  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) *
							  (QCAD_DESIGN_OBJECT (sorted_cells[i][j])->y - QCAD_DESIGN_OBJECT (cell)->y) +
							  ((double) ABS (the_cells_layer-i) * layer_separation) *
							  ((double) ABS (the_cells_layer-i) * layer_separation)) < world_radius)
					{
						//if(sorted_cells[i][j] == NULL)printf("sorted cells appear to have null member\n");
						(*p_selected_cells)[k] = sorted_cells[i][j];
						(*p_neighbour_layer)[k] = i;
						(*p_neighbour_index)[k] = j;
						k++;
					}
				}
			}
    }
	
	return number_of_selected_cells;
} //select_cells_in_radius_cv



//-------------------------------------------------------------------//

// -- determine the distance between the centers of two qdots in different cells **** [IN nm]****** -- //
double determine_distance(QCADCell *cell1, QCADCell *cell2, int dot_cell_1, int dot_cell_2, double layer_separation)
  {
  double x, y ;

  x = cell1->cell_dots[dot_cell_1].x - cell2->cell_dots[dot_cell_2].x;
  y = cell1->cell_dots[dot_cell_1].y - cell2->cell_dots[dot_cell_2].y;

	//printf("x = %e y = %e z = %e\n",x,y,z);

  return sqrt (x * x + y * y + layer_separation * layer_separation);
  }//determine_distance

// Used by all simulation engines (so far) to assemble design into desirable structures
void simulation_inproc_data_new (DESIGN *design, int *p_number_of_cell_layers, int **p_number_of_cells_in_layer, QCADCell ****p_sorted_cells)
  {
  int Nix, Nix1 ;
  GList *llItr = NULL, *llItrObj = NULL ;

  if (NULL == design) return ;

  (*p_number_of_cell_layers) = 0 ;

  // Count number of cell layers
  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_TYPE_CELLS == QCAD_LAYER (llItr->data)->type)
      (*p_number_of_cell_layers)++ ;

  (*p_number_of_cells_in_layer) = g_malloc0 ((*p_number_of_cell_layers) * sizeof (int)) ;
  (*p_sorted_cells) = g_malloc0 ((*p_number_of_cell_layers) * sizeof (QCADCell **)) ;

  // Count number of cells in each layer
  for (Nix = 0, llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    {
    if (LAYER_TYPE_CELLS == QCAD_LAYER (llItr->data)->type)
      {
      (*p_number_of_cells_in_layer)[Nix] = 0 ;
      for (llItrObj = QCAD_LAYER (llItr->data)->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
        if (NULL != llItrObj->data)
          ((*p_number_of_cells_in_layer)[Nix])++ ;
      if ((*p_number_of_cells_in_layer)[Nix] > 0)
        {
        // ... and create and fill out the array to hold all the cells for said layer
        (*p_sorted_cells)[Nix] = g_malloc0 ((*p_number_of_cells_in_layer)[Nix] * sizeof (QCADCell *)) ; ;
        for (Nix1 = 0, llItrObj = QCAD_LAYER (llItr->data)->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
          if (NULL != llItrObj->data)
            (*p_sorted_cells)[Nix][Nix1++] = llItrObj->data ;
        }
      else
        (*p_sorted_cells)[Nix] = NULL ;
      Nix++ ;
      }
    }
  }

// Correspoding free function for destroying structures used by simulation engine main routine
void simulation_inproc_data_free (int *p_number_of_cell_layers, int **p_number_of_cells_in_layer, QCADCell ****p_sorted_cells)
  {
  int Nix ;

  for (Nix = 0 ; Nix < (*p_number_of_cell_layers) ; Nix++)
    if (NULL != (*p_sorted_cells)[Nix])
      g_free ((*p_sorted_cells)[Nix]) ;

  g_free (*p_number_of_cells_in_layer) ;
  g_free (*p_sorted_cells) ;

  (*p_number_of_cell_layers) = 0 ;
  (*p_number_of_cells_in_layer) = NULL ;
  (*p_sorted_cells) = NULL ;
  }
