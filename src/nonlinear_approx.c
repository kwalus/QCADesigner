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


#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "globals.h"
#include "nonlinear_approx.h"
#include "simulation.h"
#include "stdqcell.h"
#include "cad.h"


//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_nonlinear_approx(qcell *first_cell, nonlinear_approx_OP *options){
	
	int i, j, total_cells;
	qcell *cell;
	qcell **input_cells = NULL;
	qcell **output_cells = NULL;
	int total_number_of_inputs = 0;
	int total_number_of_outputs = 0;
	double input = 0;
	int *neighbour_count = NULL;
	simulation_data *sim_data = malloc(sizeof(simulation_data));
	char text[100] = "" ;
 
	cell = first_cell;

	// -- check if there are cells to simulate -- //
	if (cell == NULL){
		printf ("There are no cells available for simulation\n");
		return NULL;
		}
  
  	// -- count the total number of cells, inputs, and outputs -- //
  	total_cells = 0;
	while (cell != NULL){
		
		if (cell->is_input){
			total_number_of_inputs++;
		
		}else if (cell->is_output){
			total_number_of_outputs++;
	
		}

		// attach the model parameters to each of the simulation cells //
		cell->cell_model = malloc(sizeof(nonlinear_approx_model));
		
		// -- Clear the model pointers so they are not dangling -- //
		((nonlinear_approx_model *)cell->cell_model)->neighbours = NULL;
		((nonlinear_approx_model *)cell->cell_model)->Ek = NULL;
		
		total_cells++;
		cell = cell->next;
    }

	// if we are performing a vector table simulation we consider only the activated inputs //
	if(SIMULATION_TYPE == VECTOR_TABLE)total_number_of_inputs = active_inputs.num_activated;
	
	// write message to the command history window //
	g_snprintf (text, 100, "Simulation found %d inputs %d outputs %d total cells\n", total_number_of_inputs, total_number_of_outputs, total_cells);
  	gtk_text_insert (GTK_TEXT (main_window.command_history), NULL, NULL, NULL, text, strlen (text));

  	// -- allocate memory for array of pointers to the input and output cells in the design -- //
  	input_cells = calloc (total_number_of_inputs, sizeof (qcell *));
  	output_cells = calloc (total_number_of_outputs, sizeof (qcell *));


  	// -- filling in the input and output pointer arrays -- //
  	i = 0;
  	j = 0;
  	cell = first_cell;

  	while (cell != NULL){

		if (cell->is_input && SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION){
	  		input_cells[i] = cell;
	  		i++;

		}else if (cell->is_output){

	  		output_cells[j] = cell;
	  		j++;
			
		}

      	cell = cell->next;
		
	}

	// if vector table simulation copy the activated cells to the inputs cells array //
	if(SIMULATION_TYPE == VECTOR_TABLE){
		for(i = 0; i < total_number_of_inputs; i++)
			input_cells[i] = active_inputs.activated_cells[i];
	}
	
	// -- Initialize the simualtion data structure -- //
  	sim_data->number_of_traces = total_number_of_inputs + total_number_of_outputs;
  	sim_data->number_samples = 3200;
  	sim_data->trace = malloc (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

  	// create and initialize the inputs into the sim data structure //      
  	for (i = 0; i < total_number_of_inputs; i++){
      	sim_data->trace[i].data_labels = malloc (sizeof (char) * (strlen (input_cells[i]->label) + 1));
      	strcpy (sim_data->trace[i].data_labels, input_cells[i]->label);
      	sim_data->trace[i].drawtrace = TRUE;
      	sim_data->trace[i].trace_color = BLUE;
      	sim_data->trace[i].data = malloc (sizeof (double) * sim_data->number_samples);
    }

  	// create and initialize the outputs into the sim data structure //     
	for (i = 0; i < total_number_of_outputs; i++){
		sim_data->trace[i + total_number_of_inputs].data_labels = malloc (sizeof (char) * (strlen (output_cells[i]->label) + 1));
      	strcpy (sim_data->trace[i + total_number_of_inputs].data_labels, output_cells[i]->label);
      	sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
      	sim_data->trace[i + total_number_of_inputs].trace_color = YELLOW;
      	sim_data->trace[i + total_number_of_inputs].data = malloc (sizeof (double) * sim_data->number_samples);
    }

  	// create and initialize the clock data //
  	sim_data->clock_data = malloc (sizeof (struct TRACEDATA) * 4);

  	for (i = 0; i < 4; i++){
    	sim_data->clock_data[i].data_labels = malloc (10);
    	g_snprintf (sim_data->clock_data[i].data_labels, 10, "CLOCK %d", i);
    	sim_data->clock_data[i].drawtrace = 1;
    	sim_data->clock_data[i].trace_color = RED;
    
    	sim_data->clock_data[i].data = malloc (sizeof (double) * sim_data->number_samples);
  
    	if(SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)for (j = 0; j < sim_data->number_samples; j++){
    		sim_data->clock_data[i].data[j] = cos (((double) pow (2, total_number_of_inputs)) * (double) j * 4.0 * PI / (double) sim_data->number_samples - PI * i / 2) + 0.1;
    		if (sim_data->clock_data[i].data[j] > 0.6)
    			sim_data->clock_data[i].data[j] = 0.6;
    		if (sim_data->clock_data[i].data[j] < 0.000000001)
    			sim_data->clock_data[i].data[j] = 0.000000001;
    	}
		
		if(SIMULATION_TYPE == VECTOR_TABLE)for (j = 0; j < sim_data->number_samples; j++){
    		sim_data->clock_data[i].data[j] = cos (((double)vector_table.num_of_vectors) * (double) j * 2.0 * PI / (double) sim_data->number_samples - PI * i / 2) + 0.1;
    		if (sim_data->clock_data[i].data[j] > 0.6)
    			sim_data->clock_data[i].data[j] = 0.6;
    		if (sim_data->clock_data[i].data[j] < 0.000000001)
    			sim_data->clock_data[i].data[j] = 0.000000001;
    	}
  
	}
  	
	// -- refresh all the kink energies -- //
	refresh_all_Ek (options);

	neighbour_count = calloc (total_cells, sizeof (int));
	sorted_cells = calloc (total_cells, sizeof (qcell *));

  	i = 0;
  	cell = first_cell;
  	
	while (cell != NULL){
		neighbour_count[i] = ((nonlinear_approx_model *)cell->cell_model)->number_of_neighbours;
      	sorted_cells[i] = cell;
      	i++;
      	cell = cell->next;
    	}

	// -- sort the cells with respect to the neighbour count -- //
	// -- this is done so that majority gates are evalulated last -- //
	// -- to ensure that all the signals have arrived first -- //
	// -- kept getting wrong answers without this -- //
	uglysort (sorted_cells, neighbour_count, total_cells);

	// perform the iterations over all samples //
	for (j = 0; j < sim_data->number_samples; j++){
		
		if (j % 100 == 0){
			// write the completion percentage to the command history window //
			g_snprintf(text, 100, "%3.0f%% complete\n", 100 * (float) j / (float) sim_data->number_samples);
			gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL,text, strlen(text));
	    }
	  
	  	// -- for each of the inputs -- //
		for (i = 0; i < total_number_of_inputs; i++){

	      	if(SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION){
				// -- create a square wave at a given frequency then double the frequency for each input -- //
				// -- this way we cover the entire input space for proper digital verification -- //
				input = 0;
				if (-1 * sin (((double) pow (2, i)) * (double) j * 4.0 * PI / (double) sim_data->number_samples) > 0){
					input = 1;
				}else{
					input = -1;
				}
			}
			
			if(SIMULATION_TYPE == VECTOR_TABLE){
				if((input = vector_table.data[(j*vector_table.num_of_vectors) / sim_data->number_samples][i]) == 0)input = -1;
			}

	      	// -- set the inputs cells with the input data -- //
	      	set_cell_polarization (input_cells[i], input);
	      	sim_data->trace[i].data[j] = input;
		}

		// -- run the iteration with the given clock value -- //
		run_nonlinear_approx_iteration (j, total_cells, sim_data);
		
//		if(options->animate_simulation){
//		
//			// -- draw the cell dots showing the new polarizations -- //
//			cell = first_cell;
//		
//			while(cell != NULL){
//				draw_cell_dots_showing_polarization(cell);
//				cell = cell->next;
//			}
//			
//			usleep(10000);
//		
//		}
		
		// -- collect all the output data from the simulation -- //
		for (i = 0; i < total_number_of_outputs; i++){
			sim_data->trace[total_number_of_inputs + i].data[j] =
			calculate_polarization (output_cells[i]);
			
			
		}
		
		

	}//for number of samples

	free (sorted_cells);
	free (neighbour_count);
	free (input_cells);
	free (output_cells);
	
	sorted_cells = NULL;
	neighbour_count = NULL;
	input_cells = NULL;
	output_cells = NULL;
	cell = NULL;
	
	return sim_data;

}//run_mean_field




// -- completes one simuolation iteration performs the approximations untill the entire design has stabalized -- //
void run_nonlinear_approx_iteration (int sample_number, int number_of_sorted_cells, simulation_data *sim_data){

	int j = 0, q = 0;
  	int stable = FALSE;
  	double old_polarization;
  	double new_polarization;
  	double tolerance = 0.0001;
  	double drive = 0;		// = SUM(Eki/2gamma * Pi)

	// -- iterate until the entire design has stabalized -- //
  	while (!stable){
		
		// -- assume that it is stable -- //
      	stable = TRUE;

		for (j = 0; j < number_of_sorted_cells; j++){

			if (sorted_cells[j]->is_input == FALSE && sorted_cells[j]->is_fixed == FALSE){

	      		old_polarization = calculate_polarization (sorted_cells[j]);

	      		drive = 0;
	     		for (q = 0; q < ((nonlinear_approx_model *)sorted_cells[j]->cell_model)->number_of_neighbours; q++){

		  			assert (((nonlinear_approx_model *)sorted_cells[j]->cell_model)->neighbours[q] != NULL);

		  			drive += (((nonlinear_approx_model *)sorted_cells[j]->cell_model)->Ek[q] / sim_data->clock_data[sorted_cells[j]->clock].data[sample_number]) * 
		  			(calculate_polarization(((nonlinear_approx_model *)sorted_cells[j]->cell_model)->neighbours[q]));
		
					}

	      		// -- calculate the new cell polarization -- //
	      		new_polarization = drive / sqrt (1 + drive * drive);

	      		// -- set the polarization of this cell -- //
	      		set_cell_polarization (sorted_cells[j], new_polarization);
				
				// -- check if it really is stable -- //
	      		//printf("tolerance = %f\n", new_polarization - old_polarization);
	      		if (fabs (new_polarization - old_polarization) > tolerance)
				stable = FALSE;

	    	}


		}

    }				//WHILE !STABLE

}//run_iteration



//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
void refresh_all_Ek (nonlinear_approx_OP *options){

  	int k;
  	qcell *cell = first_cell;

  	// calculate the Ek for each cell //
  	while (cell != NULL){

      	// select all neighbour within the provided radius //
      	((nonlinear_approx_model *)cell->cell_model)->number_of_neighbours = select_cells_in_radius (cell, 70.0);

     	free (((nonlinear_approx_model *)cell->cell_model)->neighbours);
      	free (((nonlinear_approx_model *)cell->cell_model)->Ek);
		
		((nonlinear_approx_model *)cell->cell_model)->neighbours = NULL;
		((nonlinear_approx_model *)cell->cell_model)->Ek = NULL;

		if (number_of_selected_cells > 0){
	
			((nonlinear_approx_model *)cell->cell_model)->neighbours = malloc (sizeof (qcell *) * number_of_selected_cells);
			((nonlinear_approx_model *)cell->cell_model)->Ek = malloc (sizeof (double) * number_of_selected_cells);
		
			// make sure that there was no memory allocation error //	
			if (((nonlinear_approx_model *)cell->cell_model)->neighbours == NULL || ((nonlinear_approx_model *)cell->cell_model)->Ek == NULL){
				printf ("memory allocation error in refresh_all_Ek()\n");
				exit (1);
			}
	
			for (k = 0; k < number_of_selected_cells; k++){
				assert (selected_cells[k] != NULL);
				
				// set the Ek of this cell and its neighbour //
				((nonlinear_approx_model *)cell->cell_model)->Ek[k] = determine_Ek (cell, selected_cells[k], options);
	
				// record the pointer of the neighbour //
				((nonlinear_approx_model *)cell->cell_model)->neighbours[k] = selected_cells[k];
	
			}
		}

      cell = cell->next;
    }

}//refresh_all_Ek


//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
double determine_Ek (qcell * cell1, qcell * cell2, nonlinear_approx_OP *options){

  int k;
  int j;

  double distance = 0;

  double charge1[4] = { -1, 1, -1, 1 };
  double charge2[4] = { 1, -1, 1, -1 };

  double Ek = 0;
  double E = 0;

  assert (cell1 != NULL && cell2 != NULL);
  assert (cell1 != cell2);

  for (k = 0; k < cell1->number_of_dots; k++)
    {
      for (j = 0; j < cell2->number_of_dots; j++)
	{

	  // determine the distance between the dots //
	  distance = determine_distance (cell1, cell2, k, j);
	  assert (distance != 0);

	  //printf("dist = %lf\n", distance);             
	  Ek +=
	    options->K * (charge1[k] * charge2[j]) / pow (distance, options->decay_exponent);
	  E += options->K * (charge1[k] * charge1[j]) / pow (distance, options->decay_exponent);
	  //printf("Ek = .. %lf\n", Ek);

	}			//for other dots

    }				//for these dots

  //printf("Cell[%d] -> Cell[%d] has Ek-E == %f \n", index1, index2, 10000*(Ek-E));

  return Ek - E;

}// determine_Ek


/*void calculate_ground_state(nonlinear_approx_OP *options){
	
	int i,j,total_cells;
	qcell *cell;
	int *neighbour_count;

 
	cell = first_cell;

	// -- check if there are cells to simulate -- //
	if (cell == NULL){
		printf ("There are no cells available for ground state calculation\n");
		return;
		}
  
  	// -- count the total number of cells -- //
  	total_cells = 0;
	while (cell != NULL){

		// attach the model parameters to each of the simulation cells //
		cell->cell_model = malloc(sizeof(nonlinear_approx_model));
		
		// -- Clear the model pointers so they are not dangling -- //
		((nonlinear_approx_model *)cell->cell_model)->neighbours = NULL;
		((nonlinear_approx_model *)cell->cell_model)->Ek = NULL;
		
		total_cells++;
		cell = cell->next;
    }


  	// create and initialize the clock data //
  	clock_data = malloc (sizeof (struct TRACEDATA) * 4);

  	for (i = 0; i < 4; i++){
    	
    	clock_data[i].data = malloc (sizeof (double)*100);
		
		for(j = 0; j < 100; j++)
		clock_data[i].data[j] = 1.0/(100.0*((double)j+1.0));
   
    }

  	
	// -- refresh all the kink energies -- //
	refresh_all_Ek (options);

	neighbour_count = calloc (total_cells, sizeof (int));
	sorted_cells = calloc (total_cells, sizeof (qcell *));

  	i = 0;
  	cell = first_cell;
  	
	while (cell != NULL){
		neighbour_count[i] = ((nonlinear_approx_model *)cell->cell_model)->number_of_neighbours;
      	sorted_cells[i] = cell;
      	i++;
      	cell = cell->next;
    	}

	// -- sort the cells with respect to the neighbour count -- //
	// -- this is done so that majority gates are evalulated last -- //
	// -- to ensure that all the signals have arrived first -- //
	// -- kept getting wrong answers without this -- //
	uglysort (sorted_cells, neighbour_count, total_cells);


	// -- run the iteration with the given clock value -- //
	for(j = 0; j < 100; j++)run_nonlinear_approx_iteration (j, total_cells);

	// -- draw the cell dots showing the new polarizations -- //
	cell = first_cell;
	while(cell != NULL){
		draw_cell_dots_showing_polarization(cell);
		cell = cell->next;
	}

	free (sorted_cells);
	free (neighbour_count);
	sorted_cells = NULL;
	neighbour_count = NULL;
	cell = NULL;

}
*/

//!Compare two instances of sortstruct to determine which has more neighbours
int compareSortStructs (const void *p1, const void *p2){
  
  SORTSTRUCT *pss1 = (SORTSTRUCT *) p1;
  SORTSTRUCT *pss2 = (SORTSTRUCT *) p2;

  return (pss1->number_of_neighbours > pss2->number_of_neighbours) ? 1 :
    (pss1->number_of_neighbours == pss2->number_of_neighbours) ? 0 : -1;
}//compareSortStructs

//!Sorts a list of cells according to the number of neighbours
inline void uglysort (qcell ** sorted_cells, int *number_of_neighbours, int NumberElements){

  int Nix;

  SORTSTRUCT *pss =
    (SORTSTRUCT *) malloc (NumberElements * sizeof (SORTSTRUCT));
  for (Nix = 0; Nix < NumberElements; Nix++)
    {
      pss[Nix].cell = sorted_cells[Nix];
      pss[Nix].number_of_neighbours = number_of_neighbours[Nix];
    }

  qsort (pss, NumberElements, sizeof (SORTSTRUCT), compareSortStructs);

  for (Nix = 0; Nix < NumberElements; Nix++)
    {
      sorted_cells[Nix] = pss[Nix].cell;
      number_of_neighbours[Nix] = pss[Nix].number_of_neighbours;
    }

  free (pss);
	pss = NULL;
}//uglysort
