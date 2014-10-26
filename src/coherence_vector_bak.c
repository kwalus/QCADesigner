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
// The coherence vector time-dependent simulation       //
// engine.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "objects/QCADCell.h"
#include "simulation.h"
#include "coherence_vector.h"
#include "custom_widgets.h"
#include "global_consts.h"
#ifdef GTK_GUI
#include "callback_helpers.h"
#endif /* def GTK_GUI */
#include "intl.h"

// Calculates the magnitude of the 3D energy vector
#define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
//(sqrt((4.0*(G)*(G) + (P)*(P))*over_hbar_sqr))
// Calculates the temperature ratio
#define temp_ratio(P,G,T) (hypot((G),(P)*0.5)/((T) * kB))

//!Options for the coherence simulation engine
//Added by Marco default values for phase shift (0,0,0,0)
//Added by Faizal: wave_numbers (defaults kx=0, ky=0)
coherence_OP coherence_options = {1, 1.11e-15, 1.11e-18, 1.11e-12, 9.43e-19, 1.41e-20, 0.0, 2.0, 200, 1, 1.15, EULER_METHOD, FALSE, FALSE,0,0,0,0,0,0,CONT_CLOCKING} ;

typedef struct
  {
  int number_of_neighbours;
  QCADCell **neighbours;
  int *neighbour_layer;
  double *Ek;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  } coherence_model;

#ifdef GTK_GUI
extern int STOP_SIMULATION;
#else
static int STOP_SIMULATION = 0 ;
#endif /* def GTK_GUI */

// some often used variables that can be precalculated
typedef struct
  {
  double clock_prefactor;
  double clock_shift;
  double four_pi_over_number_samples;
  double two_pi_over_number_samples;
  double hbar_over_kBT;
  } coherence_optimizations;

// instance of the optimization options;
static coherence_optimizations optimization_options;

static double coherence_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, coherence_OP *options);
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_OP *options);

static void run_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy);

static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static inline double calculate_clock_value_cc (QCADCell *cell, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static inline double lambda_ss_x (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_ss_z (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static int compareCoherenceQCells (const void *p1, const void *p2) ;
static inline void logic_sim (int number_of_cell_layers, int *number_of_cells_in_layer, double **Ek_matrix, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt, double *Pol, double *lss);
static inline void calc_pol (double *Pol, double **Ek_matrix, double *Pdriver, double *driver_ind, int num_drivers, double *fork, double Ek_max, int num_cells);
static inline void calc_poli (double *Pol, double **Ek_matrix, double EK_min, double Ek_max, int num_cells, double *fork);
static inline int search_matrix_row(int **A, int cmp, int row, int num_elements);
static inline int find_matrix_row(int **A, int cmp, int row, int num_elements, int select);
static inline int search_array(double *A, double cmp, int num_elements );
static inline int find_array(double *A, double cmp, int num_elements);
static inline int find_in_array(double *A, double cmp, int num_elements, int select);
static inline double get_max(double **A, int num_elements);
static inline double get_min(double **A, int num_elements);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_coherence_simulation (int SIMULATION_TYPE, DESIGN *design, coherence_OP *options, VectorTable *pvt)
  {
  int i, j, k, l, q, number_of_cell_layers, *number_of_cells_in_layer;
  QCADCell ***sorted_cells = NULL ;
  int total_number_of_inputs = design->bus_layout->inputs->icUsed;
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
  double *energy;
  double average_power=0;
  time_t start_time, end_time;
  simulation_data *sim_data = NULL ;
  // for randomization
  int Nix, Nix1, idxCell1, idxCell2 ;
  QCADCell *swap = NULL ;
  BUS_LAYOUT_ITER bli ;
  double dPolarization = 2.0 ;
  int idxMasterBitOrder = -1.0 ;

  double **Ek_matrix = NULL;	  
  double *Pol = NULL;
  double *lss = NULL;	  
	  
  STOP_SIMULATION = FALSE;

  // -- get the starting time for the simulation -- //
  if ((start_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get start time\n");

  // determine the number of samples from the user options //
  number_samples = (unsigned long int)(ceil (options->duration/options->time_step));

  energy = malloc(number_samples*sizeof(double));

  // if the number of samples is larger then the number of recorded samples then change the
  // time step to ensure only number_recorded_samples is used //
  if (number_recorded_samples >= number_samples)
    {
    number_recorded_samples = number_samples;
    record_interval = 1;
    }
  else
    record_interval = (unsigned long int)ceil ((double)(number_samples - 1) / (double)(number_recorded_samples));

  //fill in some of the optimizations
  optimization_options.clock_prefactor = (options->clock_high - options->clock_low) * options->clock_amplitude_factor;
  optimization_options.clock_shift = (options->clock_high + options->clock_low) * 0.5;
  optimization_options.four_pi_over_number_samples = FOUR_PI / (double)number_samples;
  optimization_options.two_pi_over_number_samples = TWO_PI / (double)number_samples;
  optimization_options.hbar_over_kBT = hbar / (kB * options->T);

  // -- spit out some messages for possible debugging -- //
  command_history_message ("About to start the coherence vector simulation with %d samples\n", number_samples);
  command_history_message ("%d samples will be recorded for graphing.\n", number_recorded_samples);
  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label ("Coherence vector simulation:") ;
  set_progress_bar_fraction (0.0) ;

  // Fill in the cell arrays necessary for conducting the simulation
  simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

	  
  // determine which cells are inputs and which are outputs //
  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      {
      // attach the model parameters to each of the simulation cells //
      sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(coherence_model));

      // -- Clear the model pointers so they are not dangling -- //
      ((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek = NULL;
      }

  // if we are performing a vector table simulation we consider only the activated inputs //
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (total_number_of_inputs = 0, Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
      {
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag)
        total_number_of_inputs++ ;
      else
        // Kill the input flag for inactive inputs, so they may be correctly simulated
        exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input->cell_function = QCAD_CELL_NORMAL ;
      }

  // write message to the command history window //
  command_history_message ("Simulation found %d inputs %d outputs\n", total_number_of_inputs, design->bus_layout->outputs->icUsed) ;

  // -- Allocate memory to hold the simulation data -- //
  sim_data = g_malloc0 (sizeof(simulation_data)) ;

  // -- Initialize the simualtion data structure -- //
  sim_data->number_of_traces = design->bus_layout->inputs->icUsed + design->bus_layout->outputs->icUsed;

  // set the number of simulation samples to be the desired number of recorded samples //
  sim_data->number_samples = number_recorded_samples;

  // allocate the memory for each trace //
  sim_data->trace = g_malloc0 (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

  // create and initialize the inputs into the sim data structure //
  for (i = 0; i < design->bus_layout->inputs->icUsed; i++)
    {
    sim_data->trace[i].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[i].drawtrace = TRUE;
    sim_data->trace[i].trace_function = QCAD_CELL_INPUT;
    sim_data->trace[i].data = g_malloc0 (sim_data->number_samples * sizeof (double));
    }

  // create and initialize the outputs into the sim data structure //
  for (i = 0; i < design->bus_layout->outputs->icUsed; i++)
    {
    sim_data->trace[i + total_number_of_inputs].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
    sim_data->trace[i + total_number_of_inputs].trace_function = QCAD_CELL_OUTPUT;
    sim_data->trace[i + total_number_of_inputs].data = g_malloc0 (sim_data->number_samples * sizeof (double));
    }

//  if (options->clocking == four_phase_clocking) {
  // create and initialize the clock data //
  sim_data->clock_data = g_malloc0 (sizeof (struct TRACEDATA) * 4);

  for (i = 0; i < 4; i++)
    {
    sim_data->clock_data[i].data_labels = g_strdup_printf ("CLOCK %d", i);
    sim_data->clock_data[i].drawtrace = 1;
    sim_data->clock_data[i].trace_function = QCAD_CELL_FIXED;
    if (NULL == (sim_data->clock_data[i].data = g_malloc0 (sim_data->number_samples * sizeof (double))))
      printf("Could not allocate memory for clock data\n");

    // fill in the clock data for the simulation results //
    for (j = 0; j<sim_data->number_samples; j++)
      //printf("j=%d, j*record_interval = %d\n",j,j*record_interval);
      sim_data->clock_data[i].data[j] = calculate_clock_value(i, j * record_interval, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
 //   }
  }
  // -- refresh all the kink energies and neighbours-- //
  coherence_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);

  // -- sort the cells with respect to the neighbour count -- //
  // -- this is done so that majority gates are evalulated last -- //
  // -- to ensure that all the signals have arrived first -- //
  // -- kept getting wrong answers without this -- //

  // The following line causes a segfault when the design consists of a single cell
//  printf("The Ek to the first cells neighbour is %e [eV]\n",((coherence_model *)sorted_cells[0][0]->cell_model)->Ek[0]/1.602e-19);

  // randomize the cells in the design as to minimize any numerical problems associated //
  // with having cells simulated in some predefined order: //
  // for each layer ...
  for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
    // ...perform as many swaps as there are cells therein
    for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
      {
      idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
      idxCell2 = rand () % number_of_cells_in_layer[Nix] ;

      swap = sorted_cells[Nix][idxCell1] ;
      sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
      sorted_cells[Nix][idxCell2] = swap ;
      }
	  
	  //Create Ek_matrix
	  Ek_matrix = (double**)malloc(number_of_cells_in_layer[0]*sizeof(double*));
	  for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		  Ek_matrix[i] = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));
	  }
	  
	  for(i = 0; i < number_of_cells_in_layer[0]; i++) {
		  for(j = 0; j < number_of_cells_in_layer[0]; j++) {
			  if (i == j) {
				  Ek_matrix[i][j] = 0;
			  }
			  else {
				  Ek_matrix[i][j] = coherence_determine_Ek ((coherence_model *)sorted_cells[0][i]->cell_model, (coherence_model *)sorted_cells[0][j]->cell_model, 0, coherence_OP *options);
			  }
		  }
	  }
	
	Pol = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));	  
    lss = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));			  

	logic_sim (number_of_cell_layers, number_of_cells_in_layer, Ek_matrix, total_number_of_inputs, options, SIMULATION_TYPE, pvt, Pol, lss);

	for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		printf("%f\n",Pol[i]);
		}
	  pause();

  if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell, 
        sim_data->trace[i].data[0] = -1) ;
  else
//  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
        qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
          sim_data->trace[i].data[0] = exp_array_index_2d (pvt->vectors, gboolean, 0, i) ? 1 : -1) ;


 // Converge the steady state coherence vector for each cell so that the simulation starts without any transients //
  stable = FALSE;
  k = 0;
  while (!stable)
    {
    stable = TRUE;

    for (i = 0; i < number_of_cell_layers; i++)
      for (j = 0; j < number_of_cells_in_layer[i]; j++)
        {
        if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function)||
             (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
          {
          j++;
          continue;
          }

        PEk = 0;
        // Calculate the sum of neighboring polarizations * the kink energy between them//
        for (q = 0; q < ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++)
          PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q])) * ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

        old_lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
        old_lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
        old_lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_ss_x(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_ss_y(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_ss_z(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);

        qcad_cell_set_polarization(sorted_cells[i][j], ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z);

        // if the lambda values are different by more then the tolerance then they have not converged //
        stable =
          !(fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x - old_lambda_x) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y - old_lambda_y) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z - old_lambda_z) > 1e-7) ;
        }
    k++;
    }

  command_history_message ("It took %d iterations to converge the initial steady state polarization\n", k);

  // perform the iterations over all samples //
  for (j = 0; j < number_samples; j++)
    {
    if (0 == j % 10000 || j == number_samples - 1)
      {
      // Update the progress bar
      set_progress_bar_fraction ((float) j / (float) number_samples) ;
      // redraw the design if the user wants it to appear animated or if this is the last sample //
#ifdef DESIGNER
      if(options->animate_simulation || j == number_samples - 1)
        {
        redraw_async(NULL);
        gdk_flush () ;
        }
#endif /* def DESIGNER */
      }
      // -- for each of the inputs -- //

    if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
      for (idxMasterBitOrder = 0, design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i), idxMasterBitOrder++)
        {
        qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell,
          dPolarization = (-sin (((double) (1 << idxMasterBitOrder)) * (double) j * optimization_options.four_pi_over_number_samples)) > 0 ? 1 : -1) ;
        if (0 == j % record_interval)
          sim_data->trace[i].data[j/record_interval] = dPolarization ;
        }
    else
//    if (VECTOR_TABLE == SIMULATION_TYPE)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
          {
          qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
            dPolarization = exp_array_index_2d (pvt->vectors, gboolean, (j*pvt->vectors->icUsed) / number_samples, i) ? 1 : -1) ;
          if (0 == j % record_interval)
            sim_data->trace[i].data[j/record_interval] = dPolarization ;
          }
/*
    if (0 == j % record_interval)
      {
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[i].data[j/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell) ;
      }
*/
    // Randomize cells so as to minimize numerical error
    if (options->randomize_cells)
      // for each layer ...
      for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
        // ...perform as many swaps as there are cells therein
        for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
          {
          idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
          idxCell2 = rand () % number_of_cells_in_layer[Nix] ;

          swap = sorted_cells[Nix][idxCell1] ;
          sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
          sorted_cells[Nix][idxCell2] = swap ;
          }

    // -- run the iteration with the given clock value -- //
    run_coherence_iteration (j, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, total_number_of_inputs, number_samples, options, sim_data, SIMULATION_TYPE, pvt, energy);

	// -- Calculate Power -- //
	if(j>=1)
		//printf("%e\n",(energy[j]-energy[j-1])/options->time_step);
		//printf("%e\n", energy[j]);
		average_power+=(energy[j]-energy[j-1])/options->time_step;

    // -- Set the cell polarizations to the lambda_z value -- //
    for (k = 0; k < number_of_cell_layers; k++)
      for (l = 0; l < number_of_cells_in_layer[k]; l++)
        {
        // don't simulate the input and fixed cells //
        if (((QCAD_CELL_INPUT == sorted_cells[k][l]->cell_function) ||
             (QCAD_CELL_FIXED == sorted_cells[k][l]->cell_function)))
          continue;
        if (fabs (((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z) > 1.0)
          {
          command_history_message ("I had to abort the simulation at iteration %d because the polarization = %e was diverging.\nPossible cause is the time step is too large.\nAlternatively, you can decrease the relaxation time to reduce oscillations.\n",j, ((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);
          command_history_message ("time step was set to %e\n", options->time_step);
          return sim_data;
          }
        qcad_cell_set_polarization (sorted_cells[k][l], -((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);
        }

    // -- collect all the output data from the simulation -- //
    if (0 == j % record_interval)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[total_number_of_inputs + i].data[j/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell) ;

  if (TRUE == STOP_SIMULATION) return sim_data;

  }//for number of samples

  // Free the neigbours and Ek array introduced by this simulation//
  for (k = 0; k < number_of_cell_layers; k++)
    for (l = 0; l < number_of_cells_in_layer[k]; l++)
      {
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbours);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->Ek);
      }

  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  g_free(energy);

  // Restore the input flag for the inactive inputs
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (i = 0 ; i < pvt->inputs->icUsed ; i++)
      exp_array_index_1d (pvt->inputs, BUS_LAYOUT_CELL, i).cell->cell_function = QCAD_CELL_INPUT ;

  // -- get and print the total simulation time -- //
  if ((end_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get end time\n");

  command_history_message (_("Total simulation time: %g s\n"), (double)(end_time - start_time));
  set_progress_bar_visible (FALSE) ;
  return sim_data;
  }//run_coherence

// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //
static void run_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy)
  {
  unsigned int i,j,q;
  double lambda_x_new;
  double lambda_y_new;
  double lambda_z_new;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  double PEk;
  double t;
  double clock_value;
  unsigned int num_neighbours;
  double Gamma[4][3];
	  unsigned long int number_recorded_samples = 3000;
	  unsigned long int record_interval;
	  	  
	  // if the number of samples is larger then the number of recorded samples then change the
	  // time step to ensure only number_recorded_samples is used //
	  if (number_recorded_samples >= number_samples)
	  {
		  number_recorded_samples = number_samples;
		  record_interval = 1;
	  }
	  else
		  record_interval = (unsigned long int)ceil ((double)(number_samples - 1) / (double)(number_recorded_samples));
	  
	  
  t = options->time_step * (double)sample_number;

	// precalculte the clock value for each of the four clocking zones
	Gamma[0][0] = calculate_clock_value(0, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[1][0] = calculate_clock_value(1, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[2][0] = calculate_clock_value(2, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[3][0] = calculate_clock_value(3, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	
	if(options->algorithm == RUNGE_KUTTA){
		Gamma[0][2] = calculate_clock_value(0, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[0][1] = (Gamma[0][0]+Gamma[0][2])/2.0;
	
		Gamma[1][2] = calculate_clock_value(1, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[1][1] = (Gamma[1][0]+Gamma[1][2])/2.0;
	
		Gamma[2][2] = calculate_clock_value(2, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[2][1] = (Gamma[2][0]+Gamma[2][2])/2.0;
	
		Gamma[3][2] = calculate_clock_value(3, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[3][1] = (Gamma[3][0]+Gamma[3][2])/2.0;	
	}
	
  energy[sample_number]=0;
  // loop through all the cells in the design //
  for (i = 0 ; i < number_of_cell_layers ; i++)
    for (j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // don't simulate the input and fixed cells //
      if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
           (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
        continue;

 //Added by Faizal for cont. clocking	
	if(FOUR_PHASE_CLOCKING == options->clocking){
      clock_value = calculate_clock_value(sorted_cells[i][j]->cell_options.clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt); 
    }
    else {  
	  clock_value = calculate_clock_value_cc(sorted_cells[i][j], sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	}
//	  printf("%e\n", clock_value/1e-19);
//End added by Faizal
	  
      PEk = 0;
      // Calculate the sum of neighboring polarizations //
      num_neighbours = ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours;
      for (q = 0 ; q < num_neighbours ; q++)
        PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

      lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
      lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
      lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

      lambda_x_new = lambda_x_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_y_new = lambda_y_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_z_new = lambda_z_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);

		  if (0 == sample_number % record_interval) {  
		printf("%e %e %e %e %e %e\n", - 2.0 *clock_value, 0.0, PEk, lambda_x_new, lambda_y_new, lambda_z_new);
		  }
			  
		energy[sample_number] += (-Gamma[sorted_cells[i][j]->cell_options.clock][0]*lambda_x_new + PEk*lambda_z_new/2);

      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_x_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_y_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_z_new;
      }
}//run_iteration

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_OP *options)
  {
  int icNeighbours = 0 ;
  coherence_model *cell_model = NULL ;
  int i,j,k;

  // calculate the Ek for each cell //
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // free up memory from previous simulations //
      g_free ((cell_model = (coherence_model *)sorted_cells[i][j]->cell_model)->neighbours);
      g_free (cell_model->Ek);
      g_free (cell_model->neighbour_layer);
      cell_model->neighbours = NULL;
      cell_model->neighbour_layer = NULL;
      cell_model->Ek = NULL;

      // select all neighbours within the provided radius //
      cell_model->number_of_neighbours = icNeighbours =
        select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((coherence_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
             ((coherence_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));

      //printf("number of neighbors = %d\n", icNeighbours);

      if (icNeighbours > 0)
        {
        cell_model->Ek = g_malloc0 (sizeof (double) * icNeighbours);

        // ensure no memory allocation error has ocurred //
        if (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
            ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek == NULL)
          //printf ("memory allocation error in refresh_all_Ek()\n");
          exit (1);

        for (k = 0; k < icNeighbours; k++)
          //if(cell_model->neighbours[k]==NULL)printf("Null neighbour prior to passing into determine Ek for k = %d\n", k);
          // set the Ek of this cell and its neighbour //
          cell_model->Ek[k] = coherence_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS(i-cell_model->neighbour_layer[k]), options);
          //printf("Ek = %e\n", cell_model->Ek[k]/1.602e-19);
        }
      }
  }//refresh_all_Ek

//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double coherence_determine_Ek (QCADCell * cell1, QCADCell * cell2, int layer_separation, coherence_OP *options)
  {
  int k;
  int j;

  double distance = 0;
  double Constant = 1 / (4 * PI * EPSILON * options->epsilonR);

  double charge1[4] = { -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE };
  double charge2[4] = {  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE };

  double EnergyDiff = 0;
  double EnergySame = 0;

  g_assert (cell1 != NULL);
  g_assert (cell2 != NULL);
  g_assert (cell1 != cell2);

  for (k = 0; k < cell1->number_of_dots; k++)
    for (j = 0; j < cell2->number_of_dots; j++)
      {
      // determine the distance between the dots //
      // printf("layer seperation = %d\n", layer_seperation);
      distance = determine_distance (cell1, cell2, k, j, (double)layer_separation * ((coherence_OP *)options)->layer_separation);
      g_assert (distance != 0);

      EnergyDiff += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
      EnergySame += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
      }//for other dots

  //printf("Ek = %e\n", (EnergyDiff - EnergySame)/ 1.602e-19);

  return EnergyDiff - EnergySame;
  }// coherence_determine_Ek

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt)
  {
  double clock = 0;
  int jitter_phases[4] = {options->jitter_phase_0, options->jitter_phase_1,
                          options->jitter_phase_2, options->jitter_phase_3} ;

//Added by Marco: phase shift included in (-PI/2, +P/2) with steps of (1/200)PI
//Edited by Konrad; Above is wrong, changed jitter to be actual phase shift shift = jitter/180*PI

  if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)(1 << total_number_of_inputs)) * (double)sample * optimization_options.four_pi_over_number_samples - (double)((jitter_phases[clock_num]) / 180.0) * PI  - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
  else
  if (SIMULATION_TYPE == VECTOR_TABLE)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)pvt->vectors->icUsed) * (double)sample * optimization_options.two_pi_over_number_samples - (double)((jitter_phases[clock_num]) / 180.0) * PI  - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
    
//End added by Marco
  return clock;
  }// calculate_clock_value


static inline double calculate_clock_value_cc (QCADCell *cell, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt) //Added by Faizal for cont. clocking
  {
  double clock = 0;

  if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)(1 << total_number_of_inputs)) * (double)sample * optimization_options.four_pi_over_number_samples - options->wave_number_kx * QCAD_DESIGN_OBJECT (cell)->x - options->wave_number_ky * QCAD_DESIGN_OBJECT (cell)->y) + optimization_options.clock_shift + options->clock_shift;
	  
    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
  else
  if (SIMULATION_TYPE == VECTOR_TABLE)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)pvt->vectors->icUsed) * (double)sample * optimization_options.two_pi_over_number_samples - options->wave_number_kx * QCAD_DESIGN_OBJECT (cell)->x - options->wave_number_ky * QCAD_DESIGN_OBJECT (cell)->y) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
    
  return clock;
  }// calculate_clock_value_cc


//-------------------------------------------------------------------//

// Next value of lambda x with choice of algorithm
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_x (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k1/2, lambda_y, lambda_z, options);
    k3 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k2/2, lambda_y, lambda_z, options);
    k4 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k3,   lambda_y, lambda_z, options);
    return lambda_x + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_x + k1;
  else
    command_history_message ("coherence vector undefined algorithm\n");

  return 0;
  }

// Next value of lambda y with choice of algorithm
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k1/2, lambda_z, options);
    k3 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k2/2, lambda_z, options);
    k4 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k3,   lambda_z, options);
    return lambda_y + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_y + k1;
  else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }

// Next value of lambda z with choice of algorithm
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_z (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k1/2, options);
    k3 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k2/2, options);
    k4 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k3,   options);
    return lambda_z + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_z + k1;
  else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }

static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double mag = magnitude_energy_vector (PEk, Gamma);
  return ((2.0 * Gamma * over_hbar / mag * tanh (optimization_options.hbar_over_kBT * mag * 0.5) - lambda_x) / options->relaxation - (PEk * lambda_y * over_hbar));
  }

static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {return (options->relaxation * (PEk * lambda_x + 2.0 * Gamma * lambda_z) - hbar * lambda_y) / (options->relaxation * hbar);}

static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double mag = magnitude_energy_vector (PEk, Gamma);
  return (-PEk * tanh (optimization_options.hbar_over_kBT * mag * 0.5) - mag * (2.0 * Gamma * options->relaxation * lambda_y + hbar * lambda_z)) / (options->relaxation * hbar * mag);
  }

//-------------------------------------------------------------------------------------------------------------------------//

// Steady-State Coherence Vector X component
static inline double lambda_ss_x(double t, double PEk, double Gamma, const coherence_OP *options)
  {return 2.0 * Gamma * over_hbar / magnitude_energy_vector(PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));}

// Steady-State Coherence Vector y component
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_OP *options)
  {return 0.0;}

// Steady-State Coherence Vector Z component
static inline double lambda_ss_z(double t, double PEk, double Gamma, const coherence_OP *options)
  {return - PEk * over_hbar / magnitude_energy_vector (PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));}

static int compareCoherenceQCells (const void *p1, const void *p2)
  {
  return
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours >
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours <
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs

static inline void logic_sim (int number_of_cell_layers, int *number_of_cells_in_layer, double **Ek_matrix, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt, double *Pol, double *lss)
	{
		double Ek_max, Ek_min;
		double *fork;
		double *Pdriver;
		double *driver_ind;
		int done;
		
		int i, j, k;
		int num_NN, num_pol, NN_pols;
		int num_drivers = 0;
		int num_cells = number_of_cells_in_layer[0];
		int Pol_sum;
		int un_pol, un_Ek, done_ind;
		
		Ek_max = get_max(Ek_matrix, num_cells);
		Ek_min = get_min(Ek_matrix, num_cells);
		
		
		fork = (double*)malloc(num_cells*sizeof(double));
		Pdriver = (double*)malloc(num_cells*sizeof(double));
		driver_ind = (double*)malloc(num_cells*sizeof(double));
		
		for(i = 0 ; i < number_of_cell_layers ; i++)
			for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
			{
				if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
					 (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function))) {
					Pol[j] = qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]));
					Pdriver[j] = Pol[j];
					driver_ind[j] = j;
					num_drivers += 1;
				}										   
				else {
					Pol[j] = 2;
					driver_ind[j] = 0;
				}
				fork[j] = 0;
				lss[j] = 0;
			}
		
		done = 1;
		while (done) {
			calc_pol(Pol, Ek_matrix, Pdriver, driver_ind, num_drivers, fork, Ek_max, num_cells);
			calc_poli(Pol,Ek_matrix, Ek_min, Ek_max, num_cells, fork);
			
			Pol_sum = 0;
			
			for (i = 0; i < num_cells; i++) {
				if (fork[i] == 0) {
					continue;
				}
				else {
					num_NN = search_matrix_row(Ek_matrix, Ek_max, fork[i], num_cells);
					num_pol = find_array(Pol, 2.0, num_cells);
					NN_pols = num_NN - num_pol;
					if (NN_pols < num_pol) {
						continue;
					}
					else {
						for (j = 0; j < num_pol; j++) {
							un_pol = find_in_array(Pol,2.0,num_cells,j);
							Pol[un_pol] = 0;
						}
						for (j = 0; j < num_NN; j++) {
							pol_val = find_matrix_row(Ek_matrix,Ek_max,fork[i],num_cells,j);
							Pol_sum += Pol[pol_val];
						}
						
						if (Pol_sum == 0) {
							Pol[fork[i]] = 2;
							for (j = 0; j < num_pol; j++) {
								un_pol = find_in_array(Pol,0,num_cells,j);
								Pol[un_pol] = 2.0;
							}
						}
						else {
							for (j = 0; j < num_NN; j++) {
								un_Ek = find_matrix_row(Ek_matrix,Ek_max,fork[i],num_cells,j);
								Ek_matrix[fork[i]][un_Ek] = 0;
							}						
							if (Pol_sum >= 1) {
								Pol[fork[i]] = 1.0;
								for (j = 0; j < num_pol; j++) {
									un_pol = find_in_array(Pol,0,num_cells,j);
									Pol[un_pol] = 2.0;
									Ek_matrix[fork[i]][un_pol] = Ek_max;
								}
							}
							else {
								Pol[fork[i]] = -1;
								for (j = 0; j < num_pol; j++) {
									un_pol = find_in_array(Pol,0,num_cells,j);
									Pol[un_pol] = 2.0;
									Ek_matrix[fork[i]][un_pol] = Ek_max;
								}	
							}
						}
					}
				}
			}
			done_ind = search_array(Pol,2.0,num_cells);
			if (done_ind == -1) {
				done = 0;
			}
			else {
				num_drivers = 0;
				for (j = 0; j < num_cells; j++) {
					if (fork[j] == 0) {
						driver_ind[j] = 0;
						Pdriver[j] = 0;
					}
					else {
						num_drivers += 1;
						driver_ind[j] = fork[j];
						Pdriver[j] = Pol[fork[k]];
						fork[j] = 0;
					}
				}
			}
		}
		free(Pdriver); Pdriver = NULL;
		free(driver_ind); driver_ind = NULL;
		free(fork); fork = NULL;
	}
	

static inline void calc_pol (double *Pol, double **Ek_matrix, double *Pdriver, double *driver_ind, int num_drivers, double *fork, double Ek_max, int num_cells)
	{
		int i, j;
		int num_NN;
		int neigh_ind;
		int fork_ind;
		
		int *new_ind;
		int *new_Pdriver;
		
		for (i = 0; i < num_drivers; i++) {
			if (driver_ind[i] == 0) {
				continue;
			}
			num_NN = search_matrix_row(Ek_matrix, Ek_max, driver_ind[i], num_cells);
			
			new_ind = (int*)malloc(num_NN*sizeof(int));
			new_Pdriver = (int*)malloc(num_NN*sizeof(int));
			
			if (num_NN > 2) {
				Pol[driver_ind[i]] = 2;
				fork_ind = search_array(fork, driver_ind[i], num_cells);
				if (fork_ind == -1) {
					fork[i] = driver_ind[i];
				}
			}
			else {
				for (j = 0; j < num_NN; j++) {
					neigh_ind = find_matrix_row(Ek_matrix, Ek_max, driver_ind[i], num_cells, j);
					if (fabs(Pol[neigh_ind]) > 1) {
						Pol[neigh_ind] = Pdriver[i];
						new_ind[j] = neigh_ind;
						new_Pdriver[j] = Pol[neigh_ind];
					}
					else {
						new_ind[j] = 0;
						new_Pdriver[j] = 0;
					}
				}
				calc_pol(Pol, Ek_matrix, new_Pdriver, new_ind, num_NN, fork, Ek_max, num_cells);
			}
				
		}
		
		free(new_ind); new_ind = NULL;
		free(new_Pdriver); new_Pdriver = NULL;
	}
				

static inline void calc_poli (double *Pol, double **Ek_matrix, double EK_min, double Ek_max, int num_cells, double *fork) 
	{
		int i, j;
		int num_pos, num_neg, inv;
		
		for (i = 0; i < num_cells; i++) {
			if (fabs(Pol[i]) == 1) {
				num_pos = search_matrix_row(Ek_matrix, Ek_max, i, num_cells);
				num_neg = search_matrix_row(Ek_matrix, Ek_min, i, num_cells);
				if (num_pos + num_neg > 2) {
					continue;
				}
				else {
					if (num_neg > 0) {
						for (j = 0; j < num_neg; j++) {
							inv = find_matrix_row(Ek_matrix, Ek_min, i, num_cells, j);
							if (fabs(Pol[inv]) > 1) {
								Pol[inv] = -Pol[i];
								calc_pol(Pol,Ek_matrix,Pol[inv],inv,1,fork,Ek_max,num_cells);
							}
						}
					}
				}
			}
		}
	}
		

static inline int search_matrix_row(int **A, int cmp, int row, int num_elements)
{
	
	int i;
	int num_NN = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[row][i] == cmp) {
			num_NN += 1;
		}
	}
	return num_NN;	
}

static inline int find_matrix_row(int **A, int cmp, int row, int num_elements, int select)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[row][i] == cmp) {
			if (j == select) {
				return i;
			}
			else {
				j += 1;
			}
		}
	}
}

static inline int search_array(double *A, double cmp, int num_elements )
{
	
	int i;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			return i;
		}
	}
	return -1;	
}				

static inline int find_array(double *A, double cmp, int num_elements)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			j += 1;
		}
	}
	return j;	
}	
					
static inline int find_in_array(double *A, double cmp, int num_elements, int select)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			if (j == select) {
				return j;
			}
			else {
				j += 1;
			}
		}
	}
}						

static inline double get_max(double **A, int num_elements) {
	
	int i,j;
	double max = -32000;
	
	for (i = 0; i < num_elements; i++) {
		for (j = 0; j < num_elements; j++) {
			if (A[i][j] > max) {
				max = A[i][j];
			}
		}
	}
	return max;
}

static inline double get_min(double **A, int num_elements) {
	
	int i,j;
	double min = 32000;
	
	for (i = 0; i < num_elements; i++) {
		for (j = 0; j < num_elements; j++) {
			if (A[i][j] < min) {
				min = A[i][j];
			}
		}
	}
	return min;
}

void coherence_options_dump (coherence_OP *coherence_options, FILE *pfile)
  {
  fprintf (stderr, "coherence_options_dump:\n") ;
	fprintf (stderr, "coherence_options->T                         = %e [K]\n",  coherence_options->T) ;
	fprintf (stderr, "coherence_options->relaxation                = %e [s]\n",  coherence_options->relaxation) ;
	fprintf (stderr, "coherence_options->time_step                 = %e [s]\n",  coherence_options->time_step) ;
	fprintf (stderr, "coherence_options->duration                  = %e [s]\n",  coherence_options->duration) ;
	fprintf (stderr, "coherence_options->clock_high                = %e [J]\n",  coherence_options->clock_high) ;
	fprintf (stderr, "coherence_options->clock_low                 = %e [J]\n",  coherence_options->clock_low) ;
	fprintf (stderr, "coherence_options->clock_shift               = %e [J]\n",  coherence_options->clock_shift) ;
	fprintf (stderr, "coherence_options->clock_amplitude_factor    = %e\n",      coherence_options->clock_amplitude_factor) ;
	fprintf (stderr, "coherence_options->radius_of_effect          = %e [nm]\n", coherence_options->radius_of_effect) ;
	fprintf (stderr, "coherence_options->epsilonR                  = %e\n",      coherence_options->epsilonR) ;
	fprintf (stderr, "coherence_options->layer_separation          = %e [nm]\n", coherence_options->layer_separation) ;
	fprintf (stderr, "coherence_options->algorithm                 = %d\n",      coherence_options->algorithm) ;
	fprintf (stderr, "coherence_options->randomize_cells           = %s\n",      coherence_options->randomize_cells ? "TRUE" : "FALSE") ;
	fprintf (stderr, "coherence_options->animate_simulation        = %s\n",      coherence_options->animate_simulation ? "TRUE" : "FALSE") ;
// Added by Marco
	fprintf (stderr, "coherence_options->jitter_phase_0            = %f degrees\n",      coherence_options->jitter_phase_0) ;
	fprintf (stderr, "coherence_options->jitter_phase_1            = %f degrees\n",      coherence_options->jitter_phase_1) ;
	fprintf (stderr, "coherence_options->jitter_phase_2            = %f degrees\n",      coherence_options->jitter_phase_2) ;
	fprintf (stderr, "coherence_options->jitter_phase_3            = %f degrees\n",      coherence_options->jitter_phase_3) ;
// End added by Marco
//Added by Faizal
	fprintf (stderr, "coherence_options->wave_number_kx            = %lf [1/nm]\n",      coherence_options->wave_number_kx) ;
	fprintf (stderr, "coherence_options->wave_number_ky            = %lf [1/nm]\n",      coherence_options->wave_number_ky) ;
	fprintf (stderr, "coherence_options->clocking                  = %d\n",             coherence_options->clocking) ;
//End added by Faizal	
  }
