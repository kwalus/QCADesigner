//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// **** Please use complete names in variables and      //
// **** functions. This will reduce ramp up time for new//
// **** people trying to contribute to the project.     //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "cad.h"
#include "vector_table.h"
#include "gqcell.h"
#include "simulation.h"
#include "scqca_math.h"
#include "scqca_simulation.h"
#include "scqca_constants.h"
#include "command_history.h"

#define PROGRESS_INTERVAL 100

/* number of dots in a cell; this should never be changed */
#define SCQCA_NUM_DOTS 4

scqca_OP scqca_options = {100, FALSE, 1e-11, 1e-12, 0.01, 100, 10, 10, 100, 1e-6, 293, 0.1,
0.0285 * P_E, 0.1, 0.0,0,0,0,0} ;

static void run_scqca_iteration(int sample_number, int number_of_sorted_cells, GQCell **sorted_cells, scqca_OP *options, simulation_data *sim_data) ;
static void scqca_refresh_all_neighbors (GQCell *cell, scqca_OP *options) ;
static int compareSCQCAQCells (const void *p1, const void *p2) ;

simulation_data *run_scqca_simulation(int SIMULATION_TYPE, GQCell *first_cell, scqca_OP *options, VectorTable *pvt)
  {
  
	int i, j, total_cells;
	GQCell *cell;
	GQCell **input_cells = NULL;
	GQCell **output_cells = NULL;
        GQCell **sorted_cells = NULL ;
	int total_number_of_inputs = 0;
	int total_number_of_outputs = 0;
	double input = 0;
	gchar* filename; 
	FILE* data_file = NULL;
	
        time_t start_time, end_time;
	
	// -- get the starting time for the simulation -- //
        if((start_time = time (NULL)) < 0) {
    	fprintf(stderr, "Could not get start time, exiting\n");
    	exit(1);
  	}
	
	// -- open the data file to store the intermediate results in case of QCADesigner crash -- //
	if((data_file = fopen(filename=g_strdup_printf("SIMULATION_DATA_%d", (int)time(NULL)), "w")) == NULL){
		printf("Cannot open file %s\n", filename);
		g_free(filename);
		return FALSE;
	}
	
	// -- free the filename as it is no longer in use -- //
	g_free(filename);
	// initialize the device parameters //
	
	options->Epk2 = 0.2*P_E;
	options->Epk1 = options->Epk2 - P_E*options->device_voltage/3;
	options->U = 2*(options->Epk1 - P_E*options->device_voltage/3);
	options->electron_lifetime = P_HBAR/options->gamma;
	
	//	int *neighbour_count = NULL;
	simulation_data *sim_data = malloc(sizeof(simulation_data));
 
	cell = first_cell;
	
	// -- check if there are cells to simulate -- //
	if (cell == NULL){
		command_history_message ("There are no cells available for simulation\n");
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
		cell->cell_model = malloc(sizeof(scqca_model));
		
		// -- Clear the model pointers so they are not dangling -- //
		((scqca_model *)cell->cell_model)->neighbours = NULL;
		
	
		((scqca_model *)cell->cell_model)->current[0] = 0.0;
		((scqca_model *)cell->cell_model)->current[1] = 0.0;
		((scqca_model *)cell->cell_model)->current[2] = 0.0;
		((scqca_model *)cell->cell_model)->current[3] = 0.0;
				
		total_cells++;
		cell = cell->next;
        }

	
	// if we are performing a vector table simulation we consider only the activated inputs //
	if(SIMULATION_TYPE == VECTOR_TABLE)
	  {
	  int Nix ;
      	  
	  total_number_of_inputs = 0 ;
	  for (Nix = 0 ; Nix < pvt->num_of_inputs ; Nix++)
	    if (pvt->active_flag[Nix])
	      total_number_of_inputs++ ;
	    else
	      /* Kill the input flag for inactive inputs, so they may be correctly simulated */
	      pvt->inputs[Nix]->is_input = FALSE ;
	  }
	
	// write message to the command history window //
	command_history_message ("Simulation found %d inputs %d outputs %d total cells\n", total_number_of_inputs, total_number_of_outputs, total_cells) ;

  	// -- allocate memory for array of pointers to the input and output cells in the design -- //
  	input_cells = calloc (total_number_of_inputs, sizeof (GQCell *));
  	output_cells = calloc (total_number_of_outputs, sizeof (GQCell *));

  	
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
	j = -1 ;
	if(SIMULATION_TYPE == VECTOR_TABLE)
	  for(i = 0; i < pvt->num_of_inputs; i++)
	    if (pvt->active_flag[i])
	      input_cells[++j] = pvt->inputs[i];
	
	// -- Initialize the simualtion data structure -- //
  	sim_data->number_of_traces = total_number_of_inputs + 2*total_number_of_outputs;
        // This values was so far hard-coded at 3200 - Let's find out why.
  	sim_data->number_samples = options->number_of_samples;
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
	for (i = 0; i < 2*total_number_of_outputs; i+=2){
	  sim_data->trace[i + total_number_of_inputs].data_labels = malloc (sizeof (char) * (strlen (output_cells[i >> 1]->label) + 1));
	  sim_data->trace[i + 1 + total_number_of_inputs].data_labels = malloc (sizeof (char) * (strlen ("Quality") + 1));
      	  
		  strcpy (sim_data->trace[i + total_number_of_inputs].data_labels, output_cells[i >> 1]->label);
		  strcpy (sim_data->trace[i + 1 + total_number_of_inputs].data_labels, "Quality");
		  
      	  sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
		  sim_data->trace[i + 1 + total_number_of_inputs].drawtrace = TRUE;
      	  sim_data->trace[i + total_number_of_inputs].trace_color = YELLOW;
		  sim_data->trace[i + 1 + total_number_of_inputs].trace_color = YELLOW;
		  
      	  sim_data->trace[i + total_number_of_inputs].data = malloc (sizeof (double) * sim_data->number_samples);
		  sim_data->trace[i + 1 + total_number_of_inputs].data = malloc (sizeof (double) * sim_data->number_samples);
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
    		  sim_data->clock_data[i].data[j] = 0.3 * cos (((double) pow (2, total_number_of_inputs)) * (double) j * 4.0 * PI / (double) sim_data->number_samples - PI * i / 2) + 0.05;
    		  if (sim_data->clock_data[i].data[j] > options->clock_high)
    			  sim_data->clock_data[i].data[j] = options->clock_high;
    		  if (sim_data->clock_data[i].data[j] < options->clock_low)
    			  sim_data->clock_data[i].data[j] = options->clock_low;
    	  }

		  if(SIMULATION_TYPE == VECTOR_TABLE)for (j = 0; j < sim_data->number_samples; j++){
    		  sim_data->clock_data[i].data[j] = 0.3 * cos (((double)pvt->num_of_vectors) * (double) j * 2.0 * PI / (double) sim_data->number_samples - PI * i / 2) + 0.05;
    		  if (sim_data->clock_data[i].data[j] > options->clock_high)
    			  sim_data->clock_data[i].data[j] = options->clock_high;
    		  if (sim_data->clock_data[i].data[j] < options->clock_low)
    			  sim_data->clock_data[i].data[j] = options->clock_low;
    	  }
  
	}
	
	scqca_refresh_all_neighbors(first_cell, options);
	
	sorted_cells = calloc (total_cells, sizeof (GQCell *));

  	i = 0;
  	cell = first_cell;
	while (cell != NULL){
//		neighbour_count[i] = ((bistable_model *)cell->cell_model)->number_of_neighbours;
      	sorted_cells[i] = cell;
      	i++;
      	cell = cell->next;
    	}

	// -- sort the cells with respect to the neighbour count -- //
	// -- this is done so that majority gates are evalulated last -- //
	// -- to ensure that all the signals have arrived first -- //
	// -- kept getting wrong answers without this -- //
	qsort (sorted_cells, total_cells, sizeof (GQCell *), compareSCQCAQCells) ;
//	uglysort (sorted_cells, neighbour_count, total_cells);
	
	// perform the iterations over all samples //
	for (j = 0; j < sim_data->number_samples; j++){
		
		// change this for larger sample simulations//
		if (j % PROGRESS_INTERVAL == 0){
			// write the completion percentage to the command history window //
			command_history_message ("%3.0f%% complete\n", 100 * (float) j / (float) sim_data->number_samples) ;
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
			
		else if(SIMULATION_TYPE == VECTOR_TABLE)
		  input = pvt->vectors[(j*pvt->num_of_vectors) / sim_data->number_samples][i] ? 1 : -1 ;

	      	// -- set the inputs cells with the input data -- //
	      	scqca_set_polarization (input_cells[i], input, options);
	      	sim_data->trace[i].data[j] = input;
		}

		// -- run the iteration with the given clock value -- //
		run_scqca_iteration (j, total_cells, sorted_cells, options, sim_data);
		
		/*
		if(options->animate_simulation){
		
			// -- draw the cell dots showing the new polarizations -- //
			cell = first_cell;
		
			while(cell != NULL){
				draw_cell_dots_showing_polarization(cell);
				cell = cell->next;
			}
			
			usleep(10000);
		
		}
		*/
		// -- collect all the output data from the simulation -- //
		for (i = 0; i < 2*total_number_of_outputs; i+=2){
			sim_data->trace[total_number_of_inputs + i].data[j] = scqca_calculate_polarization (output_cells[i >> 1]);
			sim_data->trace[total_number_of_inputs + i + 1].data[j] = scqca_calculate_quality (output_cells[i >> 1]);
			
		}
		
		//print the inputs and outputs to file for the majority gate
		//if(total_number_of_inputs==3&&total_number_of_outputs>=1)
			//fprintf(data_file,"%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\n", sim_data->clock_data[0].data[j],sim_data->clock_data[1].data[j],sim_data->clock_data[2].data[j],sim_data->clock_data[3].data[j],sim_data->trace[0].data[j],sim_data->trace[1].data[j],sim_data->trace[2].data[j],sim_data->trace[3].data[j],sim_data->trace[4].data[j]);
			
			
		//printf("Cell Polarizatio = %e Quality = %e\n",scqca_calculate_polarization (output_cells[0]),scqca_calculate_quality (output_cells[0]));
		
		

	}//for number of samples
	
	// -- get and print the total simulation time -- //
        if((end_time = time (NULL)) < 0) {
    	fprintf(stderr, "Could not get end time, exiting\n");
    	exit(1);
  	}
  
        printf("Total time: %g s\n", (double)(end_time - start_time));

	free (sorted_cells);
//	free (neighbour_count);
	free (input_cells);
	free (output_cells);
	
	sorted_cells = NULL;
//	neighbour_count = NULL;
	input_cells = NULL;
	output_cells = NULL;
	cell = NULL;
	
	/* Fix the input flag for the inactive inputs */	
	for (i = 0 ; i < pvt->num_of_inputs ; i++)
	  pvt->inputs[i]->is_input = TRUE ;
	
	return sim_data;

  }

static void run_scqca_iteration(int sample_number, int number_of_sorted_cells, GQCell **sorted_cells, scqca_OP *options, simulation_data *sim_data){

	int iteration=0;
	int stable = FALSE;
	double old_polarization;
  	double new_polarization;
	double temp;
	int i, j, l, q;
  	int iteration_count, stuck;
  	double old_current[SCQCA_NUM_DOTS];
  	double old_self_current;

	//printf ( "starting sample number %d\n", sample_number);	
	
	// -- iterate until the entire design has stabalized -- //

  	while (!stable && iteration < ((scqca_OP *)options)->max_iterations_per_sample){
		iteration++;

		// -- assume that it is stable -- //
      	stable = TRUE;

		for (j = 0; j < number_of_sorted_cells; j++){

			if (sorted_cells[j]->is_input == FALSE && sorted_cells[j]->is_fixed == FALSE){

	      		old_polarization = scqca_calculate_polarization (sorted_cells[j]);

	      		temp = 0;
				
				// -- Clear the current and delta values of each dot in each cell //
				for(l=0 ; l<SCQCA_NUM_DOTS ; l++){
					((scqca_model *)sorted_cells[j]->cell_model)->external_delta[l] = 0.0;
					((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[l] = 0.0;
					}
				
				// for each of the neighbors //
	     		for (q = 0; q < ((scqca_model *)sorted_cells[j]->cell_model)->number_of_neighbours; q++){

		  			g_assert (((scqca_model *)sorted_cells[j]->cell_model)->neighbours[q] != NULL);

					
					scqca_calculate_ext_deltas(sorted_cells[j],
						((scqca_model *)sorted_cells[j]->cell_model)->neighbours[q],
						options);
					}
					
				// reset all the currents
				for(i = 0; i < SCQCA_NUM_DOTS; i++)
      				old_current[i] = 1.0;
    			iteration_count = 0;
				
				while(fabs(old_current[0] - ((scqca_model *)sorted_cells[j]->cell_model)->current[0]) > options->cell_convergence_tolerance ||
	 					 fabs(old_current[1] - ((scqca_model *)sorted_cells[j]->cell_model)->current[1]) > options->cell_convergence_tolerance ||
	 					 fabs(old_current[2] - ((scqca_model *)sorted_cells[j]->cell_model)->current[2]) > options->cell_convergence_tolerance ||
	 					 fabs(old_current[3] - ((scqca_model *)sorted_cells[j]->cell_model)->current[3]) > options->cell_convergence_tolerance) {

      				/* calculations for each current */
      				for(i = 0; i < SCQCA_NUM_DOTS; i++) {
						old_current[i]= ((scqca_model *)sorted_cells[j]->cell_model)->current[i];
				
						old_self_current = 1.0;
						stuck = 0;
						
						//for each of the dots within the cell //
						scqca_calculate_int_deltas(sorted_cells[j],i, options);
						
						//printf("internals are %e %e %e %e\n",((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[0],((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[1],((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[2],((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[3]);
						
						
						while(fabs(((scqca_model *)sorted_cells[j]->cell_model)->current[i] - old_self_current) > options->dot_convergence_tolerance) {
	  						old_self_current = ((scqca_model *)sorted_cells[j]->cell_model)->current[i];
							((scqca_model *)sorted_cells[j]->cell_model)->current[i] = 
								scqca_calculate_current((((scqca_model *)sorted_cells[j]->cell_model)->internal_delta[i] + 
									((scqca_model *)sorted_cells[j]->cell_model)->external_delta[i]) *
							          ((scqca_model *)sorted_cells[j]->cell_model)->current[i] * options->electron_lifetime, 
							sim_data->clock_data[sorted_cells[j]->clock].data[sample_number], 
							(sorted_cells[j]->cell_dots[0].diameter/2)*(sorted_cells[j]->cell_dots[0].diameter/2)* 1e-18 * PI, options);
	  						
							//
							//printf("calculated current for dot %d is %e\n",i, ((scqca_model *)sorted_cells[j]->cell_model)->current[i]);
							stuck++;
	  						if(stuck >= options->max_dot_loops) break; 
							}//while 
							//printf("stuck is at: %d\n", stuck);
							
	    			}/* for(i = 0; i < SCQCA_NUM_DOTS; i++) */

      				iteration_count++;
      				if(iteration_count >= options->max_cell_loops) {
						/*printf("Cell %d convergence busted! dot 1 current difference = %e dot 2 = %e dot 3 = %e dot 4 = %e with repsect to a tolerance of %e The average current for the dots was %e\n", j,
						 fabs(old_current[0] - ((scqca_model *)sorted_cells[j]->cell_model)->current[0]),
	 					 fabs(old_current[1] - ((scqca_model *)sorted_cells[j]->cell_model)->current[1]),
	 					 fabs(old_current[2] - ((scqca_model *)sorted_cells[j]->cell_model)->current[2]),
	 					 fabs(old_current[3] - ((scqca_model *)sorted_cells[j]->cell_model)->current[3]), options->cell_convergence_tolerance,
						 (((scqca_model *)sorted_cells[j]->cell_model)->current[0]+((scqca_model *)sorted_cells[j]->cell_model)->current[1]+
						 ((scqca_model *)sorted_cells[j]->cell_model)->current[2]+((scqca_model *)sorted_cells[j]->cell_model)->current[3])/4);*/
						break;
      				}
				} /* while(fabs(oldcurrent[i] - target.current[i]) > CURRENT_CONVERGENCE */
				//printf("interation count = %d\n", iteration_count);
				//printf("current 0 = %e, current 1 = %e current 2 = %e current 3 = %e\n", ((scqca_model *)sorted_cells[j]->cell_model)->current[0], ((scqca_model *)sorted_cells[j]->cell_model)->current[1], ((scqca_model *)sorted_cells[j]->cell_model)->current[2], ((scqca_model *)sorted_cells[j]->cell_model)->current[3]);
				
//printf("Cell %d supposedly converged dot 1 current difference = %e dot 2 = %e dot 3 = %e dot 4 = %e with repsect to a tolerance of %e The average current for the dots was %e\n", j,
//						 fabs(old_current[0] - ((scqca_model *)sorted_cells[j]->cell_model)->current[0]),
//	 					 fabs(old_current[1] - ((scqca_model *)sorted_cells[j]->cell_model)->current[1]),
//	 					 fabs(old_current[2] - ((scqca_model *)sorted_cells[j]->cell_model)->current[2]),
//	 					 fabs(old_current[3] - ((scqca_model *)sorted_cells[j]->cell_model)->current[3]), options->cell_convergence_tolerance,
//						 (((scqca_model *)sorted_cells[j]->cell_model)->current[0]+((scqca_model *)sorted_cells[j]->cell_model)->current[1]+
//						 ((scqca_model *)sorted_cells[j]->cell_model)->current[2]+((scqca_model *)sorted_cells[j]->cell_model)->current[3])/4);
						 
			//printf("interation count = %d\n", iteration_count);
				//printf("Polarization=%f\n", Psi[0][index[0]]*Psi[0][index[0]] - Psi[1][index[0]]*Psi[1][index[0]]);

	      		// -- calculate the new cell polarization -- //
	      		new_polarization = scqca_calculate_polarization(sorted_cells[j]);

	      		// -- check if it really is stable -- //
	      		//printf("tolerance = %f\n", new_polarization - old_polarization);
	      		if (fabs (new_polarization - old_polarization) > options->circuit_convergence_tolerance)
				stable = FALSE;

	    	}


		}

    }//WHILE !STABLE
	
}

static void scqca_refresh_all_neighbors (GQCell *cell, scqca_OP *options){
    
	int icNeighbours = 0 ;
	scqca_model *cell_model = NULL ;
	GQCell *first_cell = cell ;

	while (cell != NULL){

		// free up memory for cell model variables //
		free ((cell_model = (scqca_model *)cell->cell_model)->neighbours);
		cell_model->neighbours = NULL;
		
		// select all neighbours within the provided radius //
		cell_model->number_of_neighbours = icNeighbours = 
		  select_cells_in_radius (first_cell, cell, ((scqca_OP *)options)->radius_of_effect, &(cell_model->neighbours));
		
		
		cell = cell->next;
	}

}//refresh_all_neighbors

static int compareSCQCAQCells (const void *p1, const void *p2)
  {
  return
    ((scqca_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours > 
    ((scqca_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((scqca_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours < 
    ((scqca_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs
