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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>

#include "gqcell.h"
#include "simulation.h"
#include "stdqcell.h"
#include "bistable_simulation.h"
#include "nonlinear_approx.h"
#include "command_history.h"
#include "eigenvalues.h"
#include "cad.h"
#include "command_history.h"

//!Options for the bistable simulation engine
//This variable is used by multiple source files
bistable_OP bistable_options = {3200, FALSE, 1e-3, 65, 12.9, 1e-20, 3e-34, 100} ;

typedef struct{
	int number_of_neighbours;
	struct GQCell **neighbours;
	double *Ek;
}bistable_model;

typedef struct
{
  int index;
  float energy;
}
ENERGYSORTSTRUCT;

static int compare_energy_sort_structs (const void *p1, const void *p2);
static inline void sort_energies (int *index, float *energy, int NumberElements);
static double bistable_determine_Ek(GQCell * cell1, GQCell * cell2, bistable_OP *options);
static void bistable_refresh_all_Ek(GQCell *cell, bistable_OP *options);
static void run_bistable_iteration(int sample_number, int number_of_sorted_cells, GQCell **sorted_cells, bistable_OP *options, simulation_data *sim_data);
static int compareBistableQCells (const void *p1, const void *p2) ;
static void randomize_sorted_cells(GQCell **sorted_cells, int number_of_sorted_cells);
static void flip_sorted_cells(GQCell **sorted_cells, int i, int j);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_bistable_simulation(int SIMULATION_TYPE, GQCell *first_cell, bistable_OP *options, VectorTable *pvt){
	
	int i, j, total_cells;
	GQCell *cell;
	GQCell **input_cells = NULL;
	GQCell **output_cells = NULL;
        GQCell **sorted_cells = NULL ;
	int total_number_of_inputs = 0;
	int total_number_of_outputs = 0;
	double input = 0;
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
		cell->cell_model = malloc(sizeof(bistable_model));
		
		// -- Clear the model pointers so they are not dangling -- //
		((bistable_model *)cell->cell_model)->neighbours = NULL;
		((bistable_model *)cell->cell_model)->Ek = NULL;
		
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
  	sim_data->number_of_traces = total_number_of_inputs + total_number_of_outputs;
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
    		  sim_data->clock_data[i].data[j] = (options->clock_high - options->clock_low)/0.5 * cos (((double) pow (2, total_number_of_inputs)) * (double) j * 4.0 * PI / (double) sim_data->number_samples - PI * i / 2) + (options->clock_high + options->clock_low)/2;
    		  if (sim_data->clock_data[i].data[j] > options->clock_high)
    			  sim_data->clock_data[i].data[j] = options->clock_high;
    		  if (sim_data->clock_data[i].data[j] < options->clock_low)
    			  sim_data->clock_data[i].data[j] = options->clock_low;
    	  }

		  if(SIMULATION_TYPE == VECTOR_TABLE)for (j = 0; j < sim_data->number_samples; j++){
    		  sim_data->clock_data[i].data[j] = (options->clock_high - options->clock_low)/0.5 * cos (((double)pvt->num_of_vectors) * (double) j * 2.0 * PI / (double) sim_data->number_samples - PI * i / 2) + (options->clock_high + options->clock_low)/2;
    		  if (sim_data->clock_data[i].data[j] > options->clock_high)
    			  sim_data->clock_data[i].data[j] = options->clock_high;
    		  if (sim_data->clock_data[i].data[j] < options->clock_low)
    			  sim_data->clock_data[i].data[j] = options->clock_low;
    	  }
  
	}
	
	// -- refresh all the kink energies -- //
	bistable_refresh_all_Ek (first_cell, options);

//	neighbour_count = calloc (total_cells, sizeof (int));

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
	qsort (sorted_cells, total_cells, sizeof (GQCell *), compareBistableQCells) ;
//	uglysort (sorted_cells, neighbour_count, total_cells);
	
	//randomize_sorted_cells(sorted_cells, total_cells);
	
	// perform the iterations over all samples //
	for (j = 0; j < sim_data->number_samples; j++){
		
		if (j % 100 == 0){
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
	      	gqcell_set_polarization (input_cells[i], input);
	      	sim_data->trace[i].data[j] = input;
		}

		// randomize the order in which the cells are simulated //
		//randomize_sorted_cells(sorted_cells, total_cells);
		
		// -- run the iteration with the given clock value -- //
		run_bistable_iteration (j, total_cells, sorted_cells, options, sim_data);
		
		// -- collect all the output data from the simulation -- //
		for (i = 0; i < total_number_of_outputs; i++){
			sim_data->trace[total_number_of_inputs + i].data[j] =
			gqcell_calculate_polarization (output_cells[i]);
			
			
		}
		
		

	}//for number of samples

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

}//run_bistable


// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //
static void run_bistable_iteration (int sample_number, int number_of_sorted_cells, GQCell **sorted_cells, bistable_OP *options, simulation_data *sim_data){

	int i,j, q, iteration = 0;
  	int stable = FALSE;
  	double old_polarization;
  	double new_polarization;
  	double tolerance = ((bistable_OP *)options)->convergence_tolerance;
	double temp;
	int ntot;
	int *index;
	float *Energy;
	float **Psi;
	float **Hamiltonian;
	
	if((index = malloc(2 * sizeof(int))) == NULL){
		printf("malloc failed in run_bistable_iteration: index==NULL\n");
		exit(1);
	}
	
	if((Energy = malloc(2 * sizeof(float))) == NULL){
		printf("malloc failed in run_bistable_iteration: Energy==NULL\n");
		exit(1);
	}
	
	if((Psi = malloc(2 * sizeof(float *))) == NULL){
		printf("malloc failed in run_bistable_iteration: Psi==NULL\n");
		exit(1);
	}
	
	if((Hamiltonian = malloc(2 * sizeof(float *))) == NULL){
		printf("malloc failed in run_bistable_iteration: Hamiltonian==NULL\n");
		exit(1);
	}
	

	for(i = 0; i < 2; i++){
		Hamiltonian[i] = malloc(2 * sizeof(float));
		Psi[i] = malloc(2 * sizeof(float));
	}
	
	// -- iterate until the entire design has stabalized -- //

  	while (!stable && iteration < ((bistable_OP *)options)->max_iterations_per_sample){
		iteration++;

		// -- assume that it is stable -- //
      	stable = TRUE;

		for (j = 0; j < number_of_sorted_cells; j++){

			if (sorted_cells[j]->is_input == FALSE && sorted_cells[j]->is_fixed == FALSE){

	      		old_polarization = gqcell_calculate_polarization (sorted_cells[j]);

	      		temp = 0;
	     		for (q = 0; q < ((bistable_model *)sorted_cells[j]->cell_model)->number_of_neighbours; q++){

		  			g_assert (((bistable_model *)sorted_cells[j]->cell_model)->neighbours[q] != NULL);

					// H[0][0] && H[1][1] == 1/2 Ek * P
		  			temp += (1.0/2.0)*(((bistable_model *)sorted_cells[j]->cell_model)->Ek[q]  * (gqcell_calculate_polarization(((bistable_model *)sorted_cells[j]->cell_model)->neighbours[q])));
					//printf("Ek = %f Polarization = %f temp = %f",((bistable_model *)sorted_cells[j]->cell_model)->Ek[q],(calculate_polarization(((bistable_model *)sorted_cells[j]->cell_model)->neighbours[q])), temp);
					}
				
				Hamiltonian[0][0] = -temp;
				Hamiltonian[1][1] = temp;					
				Hamiltonian[0][1] = - sim_data->clock_data[sorted_cells[j]->clock].data[sample_number];
				Hamiltonian[1][0] = - sim_data->clock_data[sorted_cells[j]->clock].data[sample_number];					
				/*
				printf("\n\n***********************************************************\n");
				
				printf("The Hamiltonian is:\n");
				printf("*****************************\n");
				printf("*  %f  *  %f  *\n", Hamiltonian[0][0], Hamiltonian[0][1]);
				printf("*****************************\n");
				printf("*  %f  *  %f  *\n", Hamiltonian[1][0], Hamiltonian[1][1]);
				printf("*****************************\n");						
				*/	
					
				jacobi(Hamiltonian,2,Energy,Psi,&ntot);
				
				index[0] = 0;
				index[1] = 1;					
									
				//printf("Energies are %f, %f\n", Energy[0], Energy[1]);
				//printf("Psi are Psi1 = %f,%f Psi2 = %f,%f\n", Psi[0][0], Psi[1][0], Psi[0][1], Psi[1][1]);
				
				sort_energies(index, Energy, 2);
					
				//printf("Energies are %f, %f\n", Energy[0], Energy[1]);
				//printf("The final state is %f,%f\n", Psi[0][index[0]], Psi[1][index[0]]);
				
				//printf("Polarization=%f\n", Psi[0][index[0]]*Psi[0][index[0]] - Psi[1][index[0]]*Psi[1][index[0]]);

	      		// -- calculate the new cell polarization -- //
	      		new_polarization = Psi[0][index[0]]*Psi[0][index[0]] - Psi[1][index[0]]*Psi[1][index[0]];

	      		// -- set the polarization of this cell -- //
	      		gqcell_set_polarization (sorted_cells[j], new_polarization);
				
				// -- check if it really is stable -- //
	      		//printf("tolerance = %f\n", new_polarization - old_polarization);
	      		if (fabs (new_polarization - old_polarization) > tolerance)
				stable = FALSE;

	    	}


		}

    }				//WHILE !STABLE
	
	for(i = 0; i < 2; i++){
		free(Hamiltonian[i]);
		free(Psi[i]);
	}
	
	free(index);
	free(Energy);
	free(Psi);
	free(Hamiltonian);
	
	index = NULL;
	Energy = NULL;
	Psi = NULL;
	Hamiltonian = NULL;
	

}//run_iteration

//-------------------------------------------------------------------//

//!Compare two instances of energysortstruct to determine which has a higher energy
static int compare_energy_sort_structs (const void *p1, const void *p2){
  
  ENERGYSORTSTRUCT *pss1 = (ENERGYSORTSTRUCT *) p1;
  ENERGYSORTSTRUCT *pss2 = (ENERGYSORTSTRUCT *) p2;

  return (pss1->energy > pss2->energy) ? 1 :
    (pss1->energy == pss2->energy) ? 0 : -1;
}//compare_energy_sort_structs

//-------------------------------------------------------------------//

//!Sorts a list of indicies according to the energy
static inline void sort_energies (int *index, float *energy, int NumberElements){

	int Nix;
	
	ENERGYSORTSTRUCT *pss = (ENERGYSORTSTRUCT *) malloc (NumberElements * sizeof (ENERGYSORTSTRUCT));
	
	for (Nix = 0; Nix < NumberElements; Nix++){
	  	pss[Nix].index = index[Nix];
	  	pss[Nix].energy = energy[Nix];
	}
	
	qsort (pss, NumberElements, sizeof (ENERGYSORTSTRUCT), compare_energy_sort_structs);
	
	for (Nix = 0; Nix < NumberElements; Nix++){
	  	index[Nix] = pss[Nix].index;
	  	energy[Nix] = pss[Nix].energy;
	}
	
	free (pss);
	pss = NULL;
}//uglysort

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void bistable_refresh_all_Ek (GQCell *cell, bistable_OP *options){
      	int icNeighbours = 0 ;
	bistable_model *cell_model = NULL ;
	GQCell *first_cell = cell ;
	int k;

	// calculate the Ek for each cell //
	while (cell != NULL){

		// free up memory for cell model variables //
		free ((cell_model = (bistable_model *)cell->cell_model)->neighbours);
		free (cell_model->Ek);
		cell_model->neighbours = NULL;
		cell_model->Ek = NULL;
		
		// select all neighbours within the provided radius //
		cell_model->number_of_neighbours = icNeighbours = 
		  select_cells_in_radius (first_cell, cell, ((bistable_OP *)options)->radius_of_effect, &(cell_model->neighbours));
		
		if (icNeighbours > 0){
			cell_model->Ek = malloc (sizeof (double) * icNeighbours);
			
			// ensure no memory allocation error has ocurred //
			if (((bistable_model *)cell->cell_model)->neighbours == NULL || ((bistable_model *)cell->cell_model)->Ek == NULL){
				printf ("memory allocation error in refresh_all_Ek()\n");
				exit (1);
			}
			
			for (k = 0; k < icNeighbours; k++){
			
			// set the Ek of this cell and its neighbour //
			cell_model->Ek[k] = bistable_determine_Ek (cell, cell_model->neighbours[k], options);
			
			}
		}
		
		cell = cell->next;
	}

}//refresh_all_Ek


//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double bistable_determine_Ek (GQCell * cell1, GQCell * cell2, bistable_OP *options){

	int k;
	int j;
	
	double distance = 0;
	double Constant = 1/(4*PI*EPSILON*options->epsilonR);
	
	double charge1[4] = { -HALF_QCHARGE, HALF_QCHARGE, -HALF_QCHARGE, HALF_QCHARGE };
	double charge2[4] = { HALF_QCHARGE, -HALF_QCHARGE, HALF_QCHARGE, -HALF_QCHARGE };
	
	double Ek = 0;
	double E = 0;
	
	g_assert (cell1 != NULL && cell2 != NULL);
	g_assert (cell1 != cell2);
	
	for (k = 0; k < cell1->number_of_dots; k++){
		for (j = 0; j < cell2->number_of_dots; j++){
		
			// determine the distance between the dots //
			distance = determine_distance (cell1, cell2, k, j);
			g_assert (distance != 0);
	            
			Ek += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
			E += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
			
		}//for other dots
	
	}//for these dots
	
	return Ek - E;

}// bistable_determine_Ek

//-------------------------------------------------------------------//

static int compareBistableQCells (const void *p1, const void *p2)
  {
  return
    ((bistable_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours > 
    ((bistable_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((bistable_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours < 
    ((bistable_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs

//-------------------------------------------------------------------//

static void randomize_sorted_cells(GQCell **sorted_cells, int number_of_sorted_cells){
	int i;
	for(i = 0; i <=number_of_sorted_cells; i++)
		flip_sorted_cells(sorted_cells, rand()%number_of_sorted_cells, rand()%number_of_sorted_cells);
}//randomize_sorted_cells

//-------------------------------------------------------------------//
static void flip_sorted_cells(GQCell **sorted_cells, int i, int j){

	GQCell *tempcell = sorted_cells[i];
	
	sorted_cells[i] = sorted_cells[j];
	sorted_cells[j] = tempcell;

}
