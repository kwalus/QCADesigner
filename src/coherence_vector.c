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

#include "gqcell.h"
#include "simulation.h"
#include "stdqcell.h"
#include "coherence_vector.h"
#include "nonlinear_approx.h"
#include "command_history.h"
#include "cad.h"

//!Options for the bistable simulation engine
coherence_OP coherence_options = {1, 1.34344e-11, 1e-15, 6.71719e-11, 45e-5 * 1.602e-19, 40e-5 * 1.602e-19, 60, 12.9, FALSE} ;

typedef struct{
	int number_of_neighbours;
	struct GQCell **neighbours;
	double *Ek;
	double lambda_x;
	double lambda_y;
	double lambda_z;
}coherence_model;

static double coherence_determine_Ek(GQCell * cell1, GQCell * cell2, coherence_OP *options);
static void coherence_refresh_all_Ek(GQCell *cell, coherence_OP *options);
static void run_coherence_iteration (int sample_number, int number_of_sorted_cells, int total_number_of_inputs, unsigned long int number_samples, GQCell **sorted_cells, coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt);
double calculate_clock_value(unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static int compareCoherenceQCells (const void *p1, const void *p2) ;
inline double magnitude_energy_vector(double t, double PEk, double Gamma, const coherence_OP *options);
double temp_ratio(double t, double PEk, double Gamma, const coherence_OP *options);
inline double lambda_ss_x(double t, double PEk, double Gamma, const coherence_OP *options);
inline double lambda_ss_y(double t, double PEk, double Gamma, const coherence_OP *options);
inline double lambda_ss_z(double t, double PEk, double Gamma, const coherence_OP *options);
inline double lambda_x_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
inline double lambda_y_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
inline double lambda_z_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_coherence_simulation(int SIMULATION_TYPE, GQCell *first_cell, coherence_OP *options, VectorTable *pvt){
	
	int i, j, q, total_cells;
	GQCell *cell;
	GQCell **input_cells = NULL;
	GQCell **output_cells = NULL;
    GQCell **sorted_cells = NULL ;
	int total_number_of_inputs = 0;
	int total_number_of_outputs = 0;
	double input = 0;
	unsigned long int number_samples;
	//number of points to record in simulation results //
	//simulations can have millions of points and there is no need to plot them all //
	unsigned long int number_recorded_samples = 3000;
	unsigned long int record_interval;
	double PEk = 0;
	gboolean stable;
	double old_lambda_x;
	double old_lambda_y;
	double old_lambda_z;
		
	// determine the number of samples from the user options //
	number_samples = (unsigned long int)(ceil(options->duration/options->time_step));
	if(number_recorded_samples >= number_samples){
		number_recorded_samples = number_samples;
		record_interval = 1;
	}else{
		record_interval = (unsigned long int)ceil((double)number_samples/(double)(number_recorded_samples-1));
	}
	
	command_history_message ("About to start the coherence vector simulation with %d samples\n", number_samples);
	printf("number_recorded_samples = %d record_interval = %d nrs*ri = %d\n",(int)number_recorded_samples, (int)record_interval,(int)(number_recorded_samples * record_interval) );
	
	// -- Allocate memory to hold the simulation data -- //
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
		
		// Clear the polarization of each cell //
		gqcell_set_polarization(cell, 0);
		
		if (cell->is_input){
			total_number_of_inputs++;
		
		}else if (cell->is_output){
			total_number_of_outputs++;
	
		}

		// attach the model parameters to each of the simulation cells //
		cell->cell_model = malloc(sizeof(coherence_model));
		
		// -- Clear the model pointers so they are not dangling -- //
		((coherence_model *)cell->cell_model)->neighbours = NULL;
		((coherence_model *)cell->cell_model)->Ek = NULL;
		
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
        
	// set the number of simulation samples to be the desired number of recorded samples //
  	sim_data->number_samples = number_recorded_samples;
	// allocate the memory for each trace //
  	sim_data->trace = malloc (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

  	// create and initialize the inputs into the sim data structure //      
  	for (i = 0; i < total_number_of_inputs; i++){
      	  sim_data->trace[i].data_labels = malloc (sizeof (char) * (strlen (input_cells[i]->label) + 1));
      	  strcpy (sim_data->trace[i].data_labels, input_cells[i]->label);
      	  sim_data->trace[i].drawtrace = TRUE;
      	  sim_data->trace[i].trace_color = BLUE;
      	  sim_data->trace[i].data = calloc (sim_data->number_samples, sizeof (double));
		  
      	}

  	// create and initialize the outputs into the sim data structure //     
	for (i = 0; i < total_number_of_outputs; i++){
	  sim_data->trace[i + total_number_of_inputs].data_labels = malloc (sizeof (char) * (strlen (output_cells[i]->label) + 1));
      	  strcpy (sim_data->trace[i + total_number_of_inputs].data_labels, output_cells[i]->label);
      	  sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
      	  sim_data->trace[i + total_number_of_inputs].trace_color = YELLOW;
      	  sim_data->trace[i + total_number_of_inputs].data = calloc (sim_data->number_samples, sizeof (double));
      	}
	
  	// create and initialize the clock data //
  	sim_data->clock_data = malloc (sizeof (struct TRACEDATA) * 4);

  	for (i = 0; i < 4; i++){
    	  sim_data->clock_data[i].data_labels = malloc (10);
    	  g_snprintf (sim_data->clock_data[i].data_labels, 10, "CLOCK %d", i);
    	  sim_data->clock_data[i].drawtrace = 1;
    	  sim_data->clock_data[i].trace_color = RED;
    	  if((sim_data->clock_data[i].data = calloc (sim_data->number_samples, sizeof (double))) == NULL)printf("Could not allocate memory for clock data\n");
		
		// fill in the clock data for the simulation results //
		for(j = 0; j<sim_data->number_samples; j++){
		//printf("j=%d, j*record_interval = %d\n",j,j*record_interval);
		sim_data->clock_data[i].data[j] = calculate_clock_value(i, j*record_interval, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		}
  
	}
	
	// -- refresh all the kink energies -- //
	coherence_refresh_all_Ek (first_cell, options);

	sorted_cells = calloc (total_cells, sizeof (GQCell *));

  	i = 0;
  	cell = first_cell;
	while (cell != NULL){
      	sorted_cells[i] = cell;
      	i++;
      	cell = cell->next;
    	}

	// -- sort the cells with respect to the neighbour count -- //
	// -- this is done so that majority gates are evalulated last -- //
	// -- to ensure that all the signals have arrived first -- //
	// -- kept getting wrong answers without this -- //
	qsort (sorted_cells, total_cells, sizeof (GQCell *), compareCoherenceQCells) ;
	
	// -- Set the inital input values so that the steady state vector can be calculated correctly -- //
	for (i = 0; i < total_number_of_inputs; i++){

	      	if(SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION){
				// Allways evaluates to this for exhaustive //
				input = -1;
			}
			
			else if(SIMULATION_TYPE == VECTOR_TABLE)
		  		input = pvt->vectors[0][i] ? 1 : -1 ;

	      		// -- set the inputs cells with the input data -- //
	      		gqcell_set_polarization (input_cells[i], input);
	      		sim_data->trace[i].data[0] = input;
		}
	
	// Reset all the cell polarizations to their associated steady state values //
	cell = first_cell;
	printf("Ek of the first two cells wrt each other is = %e[eV] Clock Low = %e[eV] \n", ((coherence_model *)cell->cell_model)->Ek[0]/(1.602e-19), options->clock_low/(1.602e-19));
	
	// Converge the steady state coherence vector for each cell so that the simulation starts without any transients //
	stable = FALSE;
	i = 0;
	while(stable == FALSE){
		stable = TRUE;
		cell = first_cell;
		while (cell != NULL){
			
			if(cell->is_input || cell->is_fixed)cell = cell->next;
		
			PEk = 0;
			// Calculate the sum of neighboring polarizations * the kink energy between them//
			for (q = 0; q < ((coherence_model *)cell->cell_model)->number_of_neighbours; q++){
		  		PEk += (gqcell_calculate_polarization(((coherence_model *)cell->cell_model)->neighbours[q]))*((coherence_model *)cell->cell_model)->Ek[q];
			}
		
			old_lambda_x = ((coherence_model *)cell->cell_model)->lambda_x;
			old_lambda_y = ((coherence_model *)cell->cell_model)->lambda_y;
			old_lambda_z = ((coherence_model *)cell->cell_model)->lambda_z;
		
			((coherence_model *)cell->cell_model)->lambda_x = lambda_ss_x(0, PEk, sim_data->clock_data[cell->clock].data[0], options);
			((coherence_model *)cell->cell_model)->lambda_y = lambda_ss_y(0, PEk, sim_data->clock_data[cell->clock].data[0], options);
			((coherence_model *)cell->cell_model)->lambda_z = lambda_ss_z(0, PEk, sim_data->clock_data[cell->clock].data[0], options);
		
			gqcell_set_polarization(cell, ((coherence_model *)cell->cell_model)->lambda_z);
		
			// if the lambda values are different by more then the tolerance then they have not converged //
			if(fabs(((coherence_model *)cell->cell_model)->lambda_x - old_lambda_x) > 0.0000001 || 
			fabs(((coherence_model *)cell->cell_model)->lambda_y - old_lambda_y) > 0.0000001 ||
			fabs(((coherence_model *)cell->cell_model)->lambda_z - old_lambda_z) > 0.0000001)stable = FALSE;
			
			cell = cell->next;
    	}
		i++;
	}
	
	printf("It took %d itterations to converge the steady state vector\n", i);
	
	// perform the iterations over all samples //
	for (j = 0; j < number_samples; j++){
		
		if (j % 10000 == 0){
			// write the completion percentage to the command history window //
			command_history_message ("%3.0f%% complete\n", 100 * (float) j / (float) number_samples) ;
	    }
	  
	  	// -- for each of the inputs -- //
		for (i = 0; i < total_number_of_inputs; i++){

	      	if(SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION){
				// -- create a square wave at a given frequency then double the frequency for each input -- //
				// -- this way we cover the entire input space for proper digital verification -- //
				input = 0;
				if (-1 * sin (((double) pow (2, i)) * (double) j * 4.0 * PI / (double) number_samples) > 0){
					input = 1;
				}else{
					input = -1;
				}
			}
			
			else if(SIMULATION_TYPE == VECTOR_TABLE)
		  		input = pvt->vectors[(j*pvt->num_of_vectors) / number_samples][i] ? 1 : -1 ;

	      		// -- set the inputs cells with the input data -- //
	      		gqcell_set_polarization (input_cells[i], input);
	      		if(0==j%record_interval){
					sim_data->trace[i].data[j/record_interval] = input; 
					//printf("j=%d j/=%d\n",j, j/record_interval);
				}
		}

		// -- run the iteration with the given clock value -- //
		
		run_coherence_iteration (j, total_cells, total_number_of_inputs, number_samples, sorted_cells, options, sim_data, SIMULATION_TYPE, pvt);
		
		// -- Set the cell polarizations to the lambda_z value -- //
		for(i = 0; i < total_cells; i++){
			// don't simulate the input and fixed cells //
			if(sorted_cells[i]->is_input || sorted_cells[i]->is_fixed)continue;
			gqcell_set_polarization(sorted_cells[i], ((coherence_model *)sorted_cells[i]->cell_model)->lambda_z);
		}
		
		// -- collect all the output data from the simulation -- //
		for (i = 0; i < total_number_of_outputs; i++){
			if(0==j%record_interval)sim_data->trace[total_number_of_inputs + i].data[j/record_interval] =
			gqcell_calculate_polarization (output_cells[i]);
			
			
		}
		
		

	}//for number of samples

	free (sorted_cells);
	free (input_cells);
	free (output_cells);
	
	sorted_cells = NULL;
	input_cells = NULL;
	output_cells = NULL;
	cell = NULL;
	
	/* Fix the input flag for the inactive inputs */	
	for (i = 0 ; i < pvt->num_of_inputs ; i++)
	  pvt->inputs[i]->is_input = TRUE ;
	
	return sim_data;

}//run_coherence


// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //
static void run_coherence_iteration (int sample_number, int number_of_sorted_cells, int total_number_of_inputs, unsigned long int number_samples, GQCell **sorted_cells, coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt){
	
	unsigned int i,q;
	double lambda_x_new;
	double lambda_y_new;
	double lambda_z_new;
	double PEk;
	double t;
	
	t = options->time_step * sample_number;
	
	// loop through all the cells in the design //
	for(i = 0; i < number_of_sorted_cells; i++){
		
		// don't simulate the input and fixed cells //
		if(sorted_cells[i]->is_input || sorted_cells[i]->is_fixed)continue;
		
		PEk = 0;
		// Calculate the sum of neighboring polarizations //
		for (q = 0; q < ((coherence_model *)sorted_cells[i]->cell_model)->number_of_neighbours; q++){
		  	PEk += (gqcell_calculate_polarization(((coherence_model *)sorted_cells[i]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i]->cell_model)->Ek[q];
		}
		
		lambda_x_new = lambda_x_next(t, PEk, calculate_clock_value(sorted_cells[i]->clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt), 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_x, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_y, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_z, options);
		
		lambda_y_new = lambda_y_next(t, PEk, calculate_clock_value(sorted_cells[i]->clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt), 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_x, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_y, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_z, options);
		
		lambda_z_new = lambda_z_next(t, PEk, calculate_clock_value(sorted_cells[i]->clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt), 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_x, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_y, 
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_z, options);
		
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_x = lambda_x_new;
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_y = lambda_y_new;
		((coherence_model *)sorted_cells[i]->cell_model)->lambda_z = lambda_z_new;
		
		
	}
	

}//run_iteration

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void coherence_refresh_all_Ek (GQCell *cell, coherence_OP *options){
    int icNeighbours = 0 ;
	coherence_model *cell_model = NULL ;
	GQCell *first_cell = cell ;
	int k;

	// calculate the Ek for each cell //
	while (cell != NULL){

		// free up memory for cell model variables //
		free ((cell_model = (coherence_model *)cell->cell_model)->neighbours);
		free (cell_model->Ek);
		cell_model->neighbours = NULL;
		cell_model->Ek = NULL;
		
		// select all neighbours within the provided radius //
		cell_model->number_of_neighbours = icNeighbours = 
		  select_cells_in_radius (first_cell, cell, ((coherence_OP *)options)->radius_of_effect, &(cell_model->neighbours));
		
		if (icNeighbours > 0){
			cell_model->Ek = malloc (sizeof (double) * icNeighbours);
			
			// ensure no memory allocation error has ocurred //
			if (((coherence_model *)cell->cell_model)->neighbours == NULL || ((coherence_model *)cell->cell_model)->Ek == NULL){
				printf ("memory allocation error in refresh_all_Ek()\n");
				exit (1);
			}
			
			for (k = 0; k < icNeighbours; k++){
			
			// set the Ek of this cell and its neighbour //
			cell_model->Ek[k] = coherence_determine_Ek (cell, cell_model->neighbours[k], options);
			
			}
		}
		
		cell = cell->next;
	}

}//refresh_all_Ek


//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double coherence_determine_Ek (GQCell * cell1, GQCell * cell2, coherence_OP *options){

	int k;
	int j;
	
	double distance = 0;
	double Constant = 1/(4*PI*EPSILON*options->epsilonR);
	
	double charge1[4] = { -HALF_QCHARGE, HALF_QCHARGE, -HALF_QCHARGE, HALF_QCHARGE };
	double charge2[4] = { HALF_QCHARGE, -HALF_QCHARGE, HALF_QCHARGE, -HALF_QCHARGE };
	
	double Ek = 0;
	double Energy = 0;
	
	g_assert (cell1 != NULL && cell2 != NULL);
	g_assert (cell1 != cell2);
	
	for (k = 0; k < cell1->number_of_dots; k++){
		for (j = 0; j < cell2->number_of_dots; j++){
		
			// determine the distance between the dots //
			distance = determine_distance (cell1, cell2, k, j);
			g_assert (distance != 0);
	            
			Ek += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
			Energy += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
			
		}//for other dots
	
	}//for these dots
	
	return Ek - Energy;

}// bistable_determine_Ek

//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample
double calculate_clock_value(unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt){

	double clock = 0;
	
	if(SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION){
		clock = (options->clock_high - options->clock_low)/0.5 * 
		cos (((double) pow (2, total_number_of_inputs)) * (double) sample * 4.0 * PI / (double)number_samples - PI * clock_num / 2) + (options->clock_high + options->clock_low)/2;
    	if (clock > options->clock_high)
    		clock = options->clock_high;
    	if (clock < options->clock_low)
    		clock = options->clock_low;
	}

	if(SIMULATION_TYPE == VECTOR_TABLE){
    		  clock = (options->clock_high - options->clock_low)/0.5 * 
			  cos (((double)pvt->num_of_vectors) * (double) sample * 2.0 * PI / (double)number_samples - PI * clock_num / 2) + (options->clock_high + options->clock_low)/2;
    	if (clock > options->clock_high)
    		clock = options->clock_high;
    	if (clock < options->clock_low)
    		clock = options->clock_low;
    }
	
	return clock;
}
// calculate clock value

//-------------------------------------------------------------------//

static int compareCoherenceQCells (const void *p1, const void *p2)
  {
  return
    ((coherence_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours > 
    ((coherence_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((coherence_model *)((*((GQCell **)(p1)))->cell_model))->number_of_neighbours < 
    ((coherence_model *)((*((GQCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs
  
// Next value of lambda x using the time marching algorithm
inline double lambda_x_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options){
	return lambda_x + options->time_step * 
	(-(2.0*Gamma*over_hbar * 1.0/magnitude_energy_vector(t, PEk, Gamma, options) * 
	tanh(temp_ratio(t, PEk, Gamma, options))+lambda_x)/options->relaxation 
	+ (PEk*lambda_y*over_hbar));
}

// Next value of lambda y using the time marching algorithm
inline double lambda_y_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options){
	return lambda_y + (-options->time_step*options->relaxation*
	PEk*lambda_x-options->time_step*hbar*lambda_y-2.0*Gamma*options->time_step*options->relaxation*lambda_z)/(options->relaxation*hbar);
}

// Next value of lambda z using the time marching algorithm
inline double lambda_z_next(double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options){
	double mag = magnitude_energy_vector(t, PEk, Gamma, options);
	
	return lambda_z + 1.0/(options->relaxation*hbar*mag) * 
	(options->time_step*PEk*tanh(temp_ratio(t, PEk, Gamma, options)) +
	2.0*Gamma*options->time_step*options->relaxation*mag*lambda_y	-
	options->time_step*hbar*mag*lambda_z);

}

// Calculates the magnitude of the 3D energy vector
inline double magnitude_energy_vector(double t, double PEk, double Gamma, const coherence_OP *options){
	return sqrt((4.0*Gamma*Gamma + PEk*PEk)*over_hbar_sqr);
}

// The temperature ratio
double temp_ratio(double t, double PEk, double Gamma, const coherence_OP *options){
	return 0.5 * hbar/(options->T * kB) * magnitude_energy_vector(t, PEk, Gamma, options);
} 

//-------------------------------------------------------------------------------------------------------------------------//

// Steady-State Coherence Vector X component
inline double lambda_ss_x(double t, double PEk, double Gamma, const coherence_OP *options){
	return -2.0*Gamma*over_hbar * 1.0/magnitude_energy_vector(t, PEk, Gamma, options) * tanh(temp_ratio(t, PEk, Gamma, options));
}

// Steady-State Coherence Vector y component
inline double lambda_ss_y(double t, double PEk, double Gamma, const coherence_OP *options){
	return 0.0;
}

// Steady-State Coherence Vector Z component
inline double lambda_ss_z(double t, double PEk, double Gamma, const coherence_OP *options){
	return PEk*over_hbar * 1.0/magnitude_energy_vector(t, PEk, Gamma, options) * tanh(temp_ratio(t, PEk, Gamma, options));
}


