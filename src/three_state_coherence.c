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
// The Three statecoherence vector time-dependent       //
// simulation engine.                                   //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "objects/QCADCell.h"
#include "objects/QCADElectrode.h"
#include "matrixlib_3x3.h"
#include "simulation.h"
#include "three_state_coherence.h"
#include "custom_widgets.h"
#include "global_consts.h"
#ifdef GTK_GUI
  #include "callback_helpers.h"
#endif /* def GTK_GUI */

// Calculates the magnitude of the 3D energy vector
#define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
//(sqrt((4.0*(G)*(G) + (P)*(P))*over_hbar_sqr))
// Calculates the temperature ratio
#define temp_ratio(P,G,T) (hypot((G),(P)*0.5)/((T) * kB))

//!Options for the coherence simulation engine
ts_coherence_OP ts_coherence_options = {1, 1e-15, 1e-16, 7e-11, 1e-22, 2e-22, -2e-22, 0.0, 2.0, 80, 12.9, 11.5, EULER_METHOD, TRUE, FALSE} ;

typedef struct
  {
		int number_of_neighbours;
		QCADCell **neighbours;
		int *neighbour_layer;
		double *Ek;
		double lambda[8];
		double Gamma[8];
		complex Hamiltonian[3][3];
	} ts_coherence_model;

#ifdef GTK_GUI
extern int STOP_SIMULATION;
#else
static int STOP_SIMULATION = 0 ;
#endif /* def GTK_GUI */
//!Maximum exponent to the exponential function that will not result in infinity
static double max_exponent ;

// some often used variables that can be precalculated
typedef struct
  {
  double clock_prefactor;
  double clock_shift;
  double four_pi_over_number_samples;
  double two_pi_over_number_samples;
  double hbar_over_kBT;
  } ts_coherence_optimizations;

// instance of the optimization options;
static ts_coherence_optimizations optimization_options;

static double ts_coherence_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, ts_coherence_OP *options);
static void ts_coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_coherence_OP *options);
static void run_ts_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const ts_coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy, complex generator[8][3][3], double structureConst[8][8][8]);
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const ts_coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static int compareCoherenceQCells (const void *p1, const void *p2) ;
void dump_3x3_complexMatrix(complex A[3][3]);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_ts_coherence_simulation (int SIMULATION_TYPE, DESIGN *design, ts_coherence_OP *options, VectorTable *pvt)
  {
  int i, j, k, l, q, row, col, number_of_cell_layers, *number_of_cells_in_layer;
  QCADCell ***sorted_cells = NULL ;
  int total_number_of_inputs = design->bus_layout->inputs->icUsed;
  unsigned long int number_samples;
  //number of points to record in simulation results //
  //simulations can have millions of points and there is no need to plot them all //
  unsigned long int number_recorded_samples = 3000;
  unsigned long int record_interval;
  double PEk = 0;
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
  int stable=0;
	GList *llItr = NULL;
	QCADLayer *clocking_layer = NULL;
	
	double lambdaSS[8];
	double old_lambda[8];
	double omega[8][8];
	double omegaDotLambda[8];
	complex densityMatrixSS[3][3];
	complex minusOverKBT;
	double potential;
	
	// variables required for generating the structure constants
	complex structureC = {0,-0.25};
	complex tempMatrix1[3][3];
	complex tempMatrix2[3][3];
	complex tempMatrix3[3][3];
	complex tempMatrix4[3][3];
	
	double structureConst[8][8][8];
	// Create the generators of the speacial unitary group SU(3)
	// Refer to "Maxwell's demon and quantum-dot cellular automata", J. Timler and C.S. Lent, Appl. Phys. Lett. 2003
	static complex generator[8][3][3]={
	
	//generator[0]
	{
	{{0,0},{1,0},{0,0}},
	{{1,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,0}}
	},
	
	//generator[1]
	{
	{{0,0},{0,0},{1,0}},
	{{0,0},{0,0},{0,0}},
	{{1,0},{0,0},{0,0}}
	},
	
	//generator[2]
	{
	{{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{1,0}},
	{{0,0},{1,0},{0,0}}
	},
	
	//generator[3]
	{
	{{0,0},{0,1},{0,0}},
	{{0,-1},{0,0},{0,0}},
	{{0,0},{0,0},{0,0}}
	},
	
	//generator[4]
	{
	{{0,0},{0,0},{0,1}},
	{{0,0},{0,0},{0,0}},
	{{0,-1},{0,0},{0,0}}
	},
	
	//generator[5]
	{
	{{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,1}},
	{{0,0},{0,-1},{0,0}}
	},
	
	//generator[6]
	{
	{{-1,0},{0,0},{0,0}},
	{{0,0},{1,0},{0,0}},
	{{0,0},{0,0},{0,0}}
	},
	
	//generator[7]
	{
	{{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,0}}
	}
	};

	// fill in the elements that could not be done at initialization
	generator[7][0][0].re = -1.0/sqrt(3.0);
	generator[7][1][1].re = -1.0/sqrt(3.0);
	generator[7][2][2].re =  2.0/sqrt(3.0);
	
	//!Maximum exponent to the exponential function that will not result in infinity
	max_exponent = log(G_MAXDOUBLE);
	
	//fill in the complex constants
	minusOverKBT.re = -1.0 / (kB * options->T);
	minusOverKBT.im = 0;
	
  STOP_SIMULATION = FALSE;

  // -- get the starting time for the simulation -- //
  if ((start_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get start time\n");

  // determine the number of samples from the user options //
  number_samples = (unsigned long int)(ceil (options->duration/options->time_step));
			
	// array of energy values for each simulation sample //
	// later used to determine power flow P = dE/dt
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

  //fill in some of the optimization variables to reduce future calculations
  optimization_options.clock_prefactor = (options->clock_high - options->clock_low) * options->clock_amplitude_factor;
  optimization_options.clock_shift = (options->clock_high + options->clock_low) * 0.5;
  optimization_options.four_pi_over_number_samples = FOUR_PI / (double)number_samples;
  optimization_options.two_pi_over_number_samples = TWO_PI / (double)number_samples;
  optimization_options.hbar_over_kBT = hbar / (kB * options->T);

  // -- spit out some messages for possible debugging -- //
  command_history_message ("About to start the three-state coherence vector simulation with %d samples\n", number_samples);
  command_history_message ("%d samples will be recorded for graphing.\n", number_recorded_samples);
  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label ("Three-State Coherence simulation:") ;
  set_progress_bar_fraction (0.0) ;

  // Fill in the cell arrays necessary for conducting the simulation
  simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  // determine which cells are inputs and which are outputs //
  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      {
      // attach the model parameters to each of the simulation cells //
      sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(ts_coherence_model));

      // -- Clear the model pointers so they are not dangling -- //
      ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
      ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Ek = NULL;
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
    }

  // -- refresh all the kink energies and neighbours-- //
  ts_coherence_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);

  // -- sort the cells with respect to the neighbour count -- //
  // -- this is done so that majority gates are evalulated last -- //
  // -- to ensure that all the signals have arrived first -- //
  // -- kept getting wrong answers without this -- //

  // The following line causes a segfault when the design consists of a single cell
//  printf("The Ek to the first cells neighbour is %e [eV]\n",((ts_coherence_model *)sorted_cells[0][0]->cell_model)->Ek[0]/1.602e-19);

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

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			for (k = 0; k < 8; k++){
				complexMatrixMultiplication(generator[i], generator[j], tempMatrix1);
				complexMatrixMultiplication(generator[j], generator[i], tempMatrix2);
				complexMatrixSubtraction(tempMatrix1, tempMatrix2, tempMatrix3);
				complexMatrixMultiplication(tempMatrix3, generator[k], tempMatrix4);
				structureConst[i][j][k] = (complexMultiply(structureC, complexTr(tempMatrix4))).re;
				//printf("structure constant %d %d %d is : %lf\n", i,j,k,structureConst[i][j][k]);
			}
	
	// find the clocking layer
	for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next){
		if(LAYER_TYPE_CLOCKING == QCAD_LAYER (llItr->data)->type)
			clocking_layer = (QCADLayer *)(llItr->data);
		}
		
	//check to make sure there actually is a clocking layer
	if(clocking_layer == NULL){
		command_history_message ("Critical Error: No clocking layer found!\n");
		return(sim_data);
	}
	
	printf("Debug point A\n");
	//setup the electrodes and ground plate
	for(llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next)
		qcad_electrode_set_capacitance ((QCADElectrode *)(llItr->data), options->epsilonR, 20e-9);
	printf("Debug point B\n");
		
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
        for (q = 0; q < ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++)
          PEk += (qcad_cell_calculate_polarization (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q])) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];
					PEk *= 0.5;
				//printf("This cell PEk = %e\n", PEk);
				
				//find the poential
				for(llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next)
					potential = qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	
																									sorted_cells[i][j]->cell_dots[0].x,
																									sorted_cells[i][j]->cell_dots[0].y,
																									options->layer_separation,
																									1e-9);
																																				
				printf("Electrode Potential is: %e", potential);
				
				// Fill in the Hamiltonian for each cell **each element is a complex number since it will be multiplied by the complex generators**//
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][0].re= -PEk;							((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][1].re = 0;								((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][2].re = -options->gamma;	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][2].im = 0;
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][0].re = 0;								((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][1].re = PEk;							((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][2].re = -options->gamma;	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][2].im = 0;
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][0].re = -options->gamma;	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][1].re = -options->gamma;	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][2].re = sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0]; ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][2].im = 0;
				
				//printf("Hamiltonian:\n");
				//dump_3x3_complexMatrix(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian);
				
				//generate the hamiltonian in the space of SU(3)
				
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[0],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[1],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[2],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[3],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[4],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[5],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[6],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, generator[7],tempMatrix1);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7] = OVER_HBAR * (complexTr(tempMatrix1)).re;
				
				/*
				printf("Gamma: [ %e, %e, %e, %e, %e, %e, %e, %e,]\n", ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0], 
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7]); 
				*/																											
				
				// generate the steady-state density matrix
				
				complexConstMatrixMultiplication(minusOverKBT, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, tempMatrix1);
				//printf("Density Matrix [step1 division H/kBT]:\n");
				//dump_3x3_complexMatrix(tempMatrix1);
				if(tempMatrix1[2][0].re >= max_exponent){
					command_history_message ("Critical simulation error! You will have to choose a smaller value for the tunneling energy.\n", number_samples);
					return sim_data;
					}
				
				complexMatrixRealExponential(tempMatrix1);
				//printf("Density Matrix [step2 Real exponential]:\n");
				//dump_3x3_complexMatrix(tempMatrix1);
								
				//printf("Trace of step2: %e+j%e\n",complexTr(tempMatrix1).re, complexTr(tempMatrix1).im);
				complexConstMatrixDivision(complexTr(tempMatrix1), tempMatrix1, densityMatrixSS);
				
				//printf("Density Matrix:\n");
				//dump_3x3_complexMatrix(densityMatrixSS);
				
				// generate the steady-state coherence vector
				
				complexMatrixMultiplication(densityMatrixSS, generator[0], tempMatrix1);
				lambdaSS[0] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[1], tempMatrix1);
				lambdaSS[1] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[2], tempMatrix1);
				lambdaSS[2] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[3], tempMatrix1);
				lambdaSS[3] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[4], tempMatrix1);
				lambdaSS[4] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[5], tempMatrix1);
				lambdaSS[5] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[6], tempMatrix1);
				lambdaSS[6] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[7], tempMatrix1);
				lambdaSS[7] = (complexTr(tempMatrix1)).re;
				
				//printf("LambdaSS: [ %e, %e, %e, %e, %e, %e, %e, %e,]\n", lambdaSS[0], lambdaSS[1], lambdaSS[2], lambdaSS[3], lambdaSS[4], lambdaSS[5], lambdaSS[6], lambdaSS[7] );
				
				// keep track of old state for checking convergence
				old_lambda[0] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[0];
				old_lambda[1] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[1];
				old_lambda[2] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[2];
				old_lambda[3] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[3];
				old_lambda[4] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[4];
				old_lambda[5] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[5];
				old_lambda[6] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6];
				old_lambda[7] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[7];
				
				// generate the omega matrix
				for(row = 0; row < 8; row++)
					for(col = 0; col < 8; col++){
							omega[row][col] = 0;
							for(Nix=0; Nix < 8; Nix++)
								omega[row][col] = structureConst[row][Nix][col]*((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[Nix];
						}
				
				// calculate the dot product of omega and lambda				
				for(Nix=0; Nix < 8; Nix++){
					omegaDotLambda[Nix] = 0;
					for(Nix1=0; Nix1 < 8; Nix1++)
						omegaDotLambda[Nix]+=omega[Nix][Nix1]*((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[Nix1];
					}
					
					
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[0] = lambdaSS[0];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[1] = lambdaSS[1];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[2] = lambdaSS[2];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[3] = lambdaSS[3];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[4] = lambdaSS[4];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[5] = lambdaSS[5];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6] = lambdaSS[6];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[7] = lambdaSS[7];
						
        //printf("\n\n\n");
				
        qcad_cell_set_polarization(sorted_cells[i][j], -1 * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6]);

        // if the lambda values are different by more then the tolerance then they have not converged //
        stable =
          !(fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[0]-old_lambda[0]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[1]-old_lambda[1]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[2]-old_lambda[2]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[3]-old_lambda[3]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[4]-old_lambda[4]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[5]-old_lambda[5]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6]-old_lambda[6]) > 1e-7 ||
					fabs (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[7]-old_lambda[7]) > 1e-7);
            
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

    if (0 == j % record_interval)
      {
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[i].data[j/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell) ;
      }

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
    run_ts_coherence_iteration (j, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, total_number_of_inputs, number_samples, options, sim_data, SIMULATION_TYPE, pvt, energy, generator, structureConst);

	// -- Calculate Power -- //
	if(j>=1)
		//printf("%e\n",(energy[j]-energy[j-1])/options->time_step);
		average_power+=(energy[j]-energy[j-1])/options->time_step;
    // -- Set the cell polarizations to the lambda_z value -- //
    for (k = 0; k < number_of_cell_layers; k++)
      for (l = 0; l < number_of_cells_in_layer[k]; l++)
        {
        // don't simulate the input and fixed cells //
        if (((QCAD_CELL_INPUT == sorted_cells[k][l]->cell_function) ||
             (QCAD_CELL_FIXED == sorted_cells[k][l]->cell_function)))
          continue;
        if (fabs (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6]) > 1.0)
          {
          command_history_message ("I had to abort the simulation at iteration %d because the polarization = %e was diverging.\nPossible cause is the time step is too large.\nAlternatively, you can decrease the relaxation time to reduce oscillations.\n",j, -1*((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[7]);
          command_history_message ("time step was set to %e\n", options->time_step);
          return sim_data;
          }
        qcad_cell_set_polarization (sorted_cells[k][l], -1 * ((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6]);
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
      g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->neighbours);
      g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
      g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->Ek);
      }

  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  // Restore the input flag for the inactive inputs
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (i = 0 ; i < pvt->inputs->icUsed ; i++)
      exp_array_index_1d (pvt->inputs, BUS_LAYOUT_CELL, i).cell->cell_function = QCAD_CELL_INPUT ;
  
  //command_history_message ("Average Power: %e Watts\n", average_power/(number_samples-1));
  
  // -- get and print the total simulation time -- //
  if ((end_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get end time\n");
	
  command_history_message ("Total simulation time: %g s\n", (double)(end_time - start_time));
  set_progress_bar_visible (FALSE) ;
  return sim_data;
  }//run_ts_coherence

// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //

static void run_ts_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const ts_coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy, complex generator[8][3][3], double structureConst[8][8][8])
  {
  unsigned int i,j,q, row, col, Nix, Nix1;
  double PEk;
	double t;
  double clock_value;
  unsigned int num_neighbours;
	double lambda[8];
	complex tempMatrix1[3][3];
	double over_relaxation = 1.0/options->relaxation;
  double lambdaSS[8];
	double omega[8][8];
	double omegaDotLambda[8];
	complex densityMatrixSS[3][3];
	complex minusOverKBT;
	double two_over_root_3 = 2.0/sqrt(3.0);
	double two_over_hbar = 2.0 * OVER_HBAR;

	//fill in the complex constants
	minusOverKBT.re = -1.0 / (kB * options->T);
	minusOverKBT.im = 0;
	
  t = options->time_step * (double)sample_number;
  //energy[sample_number]=0;
  
  // loop through all the cells in the design //
  for (i = 0 ; i < number_of_cell_layers ; i++)
    for (j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // don't simulate the input and fixed cells //
      if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
           (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
        continue;

      clock_value = calculate_clock_value(sorted_cells[i][j]->cell_options.clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);

      PEk = 0;
      // Calculate the sum of neighboring polarizations //
      num_neighbours = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours;
      for (q = 0 ; q < num_neighbours ; q++)
        PEk += (qcad_cell_calculate_polarization (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];
				PEk *= 0.5;
		  
				//printf("This cell PEk = %e\n", PEk);
				
				// Fill in the Hamiltonian for each cell **each element is a complex number since it will be multiplied by the complex generators**//
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][0].re= -PEk;								((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][1].re = 0;									((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][2].re = -options->gamma;		((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[0][2].im = 0;
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][0].re = 0;									((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][1].re = PEk;								((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][2].re = -options->gamma;		((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[1][2].im = 0;
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][0].re = -options->gamma;		((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][0].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][1].re = -options->gamma;		((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][1].im = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][2].re = clock_value;				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian[2][2].im = 0;
				
				//printf("Hamiltonian:\n");
				//dump_3x3_complexMatrix(((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian);
				
				//generate the hamiltonian in the space of SU(3)
				
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1] = -two_over_hbar * options->gamma ;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2] = -two_over_hbar * options->gamma ;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6] = OVER_HBAR * PEk;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7] = two_over_root_3 * clock_value;
				
				/*printf("Gamma: [ %e, %e, %e, %e, %e, %e, %e, %e,]\n", ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0], 
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6],
																															((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7]); 
																															
				*/
				
				// generate the steady-state density matrix
				
				complexConstMatrixMultiplication(minusOverKBT, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Hamiltonian, tempMatrix1);
				//printf("Density Matrix [step1 division H/kBT]:\n");
				//dump_3x3_complexMatrix(tempMatrix1);
				if(tempMatrix1[2][0].re >= max_exponent){
					command_history_message ("Critical simulation error! You will have to choose a smaller value for the tunneling energy.\n", number_samples);
					return;
					}
				
				complexMatrixRealExponential(tempMatrix1);
				//printf("Density Matrix [step2 Real exponential]:\n");
				//dump_3x3_complexMatrix(tempMatrix1);
								
				//printf("Trace of step2: %e+j%e\n",complexTr(tempMatrix1).re, complexTr(tempMatrix1).im);
				complexConstMatrixDivision(complexTr(tempMatrix1), tempMatrix1, densityMatrixSS);
				
				//printf("Density Matrix:\n");
				//dump_3x3_complexMatrix(densityMatrixSS);
				
				// generate the steady-state coherence vector
	
				complexMatrixMultiplication(densityMatrixSS, generator[0], tempMatrix1);
				lambdaSS[0] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[1], tempMatrix1);
				lambdaSS[1] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[2], tempMatrix1);
				lambdaSS[2] = (complexTr(tempMatrix1)).re;
				lambdaSS[3] = 0;
				lambdaSS[4] = 0;
				lambdaSS[5] = 0;
				complexMatrixMultiplication(densityMatrixSS, generator[6], tempMatrix1);
				lambdaSS[6] = (complexTr(tempMatrix1)).re;
				complexMatrixMultiplication(densityMatrixSS, generator[7], tempMatrix1);
				lambdaSS[7] = (complexTr(tempMatrix1)).re;
				
				//printf("LambdaSS: [ %e, %e, %e, %e, %e, %e, %e, %e,]\n", lambdaSS[0], lambdaSS[1], lambdaSS[2], lambdaSS[3], lambdaSS[4], lambdaSS[5], lambdaSS[6], lambdaSS[7] );
				
				// generate the omega matrix
				for(row = 0; row < 8; row++)
					for(col = 0; col < 8; col++){
							omega[row][col] = 0;
							for(Nix=0; Nix < 8; Nix++)
								omega[row][col] = structureConst[row][Nix][col]*((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[Nix];
						}
				
				// calculate the dot product of omega and lambda				
				for(Nix=0; Nix < 8; Nix++){
					omegaDotLambda[Nix] = 0;
					for(Nix1=0; Nix1 < 8; Nix1++)
						omegaDotLambda[Nix]+=omega[Nix][Nix1]*((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[Nix1];
					}
					
				lambda[0] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[0];
				lambda[1] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[1];
				lambda[2] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[2];
				lambda[3] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[3];
				lambda[4] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[4];
				lambda[5] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[5];
				lambda[6] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6];
				lambda[7] = ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[7];
					
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[0] = options->time_step * (omegaDotLambda[0] - over_relaxation*(lambda[0] - lambdaSS[0])) + lambda[0];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[1] = options->time_step * (omegaDotLambda[1] - over_relaxation*(lambda[1] - lambdaSS[1])) + lambda[1];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[2] = options->time_step * (omegaDotLambda[2] - over_relaxation*(lambda[2] - lambdaSS[2])) + lambda[2];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[3] = options->time_step * (omegaDotLambda[3] - over_relaxation*(lambda[3] - lambdaSS[3])) + lambda[3];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[4] = options->time_step * (omegaDotLambda[4] - over_relaxation*(lambda[4] - lambdaSS[4])) + lambda[4];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[5] = options->time_step * (omegaDotLambda[5] - over_relaxation*(lambda[5] - lambdaSS[5])) + lambda[5];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[6] = options->time_step * (omegaDotLambda[6] - over_relaxation*(lambda[6] - lambdaSS[6])) + lambda[6];
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->lambda[7] = options->time_step * (omegaDotLambda[7] - over_relaxation*(lambda[7] - lambdaSS[7])) + lambda[7];

						
        //printf("\n\n\n");
			
				//energy[sample_number] += (-clock_value*lambda_x_new + PEk*lambda_z_new/2);
					
			}
	  
  }//run_iteration

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void ts_coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_coherence_OP *options)
  {
  int icNeighbours = 0 ;
  ts_coherence_model *cell_model = NULL ;
  int i,j,k;

  // calculate the Ek for each cell //
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // free up memory from previous simulations //
      g_free ((cell_model = (ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours);
      g_free (cell_model->Ek);
      g_free (cell_model->neighbour_layer);
      cell_model->neighbours = NULL;
      cell_model->neighbour_layer = NULL;
      cell_model->Ek = NULL;

      // select all neighbours within the provided radius //
      cell_model->number_of_neighbours = icNeighbours =
        select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((ts_coherence_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
             ((ts_coherence_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));

      //printf("number of neighbors = %d\n", icNeighbours);

      if (icNeighbours > 0)
        {
        cell_model->Ek = g_malloc0 (sizeof (double) * icNeighbours);

        // ensure no memory allocation error has ocurred //
        if (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
            ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Ek == NULL)
          //printf ("memory allocation error in refresh_all_Ek()\n");
          exit (1);

        for (k = 0; k < icNeighbours; k++)
          //if(cell_model->neighbours[k]==NULL)printf("Null neighbour prior to passing into determine Ek for k = %d\n", k);
          // set the Ek of this cell and its neighbour //
          cell_model->Ek[k] = ts_coherence_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS(i-cell_model->neighbour_layer[k]), options);
          //printf("Ek = %e\n", cell_model->Ek[k]/1.602e-19);
        }
      }
  }//refresh_all_Ek

//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double ts_coherence_determine_Ek (QCADCell * cell1, QCADCell * cell2, int layer_separation, ts_coherence_OP *options)
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
      distance = determine_distance (cell1, cell2, k, j, (double)layer_separation * ((ts_coherence_OP *)options)->layer_separation);
      g_assert (distance != 0);

      EnergyDiff += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
      EnergySame += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
      }//for other dots

  //printf("Ek = %e\n", (EnergyDiff - EnergySame)/ 1.602e-19);

  return EnergyDiff - EnergySame;
  }// ts_coherence_determine_Ek

//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const ts_coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt)
  {
  double clock = 0;

  if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double) (1 << total_number_of_inputs)) * (double) sample * optimization_options.four_pi_over_number_samples - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
  else
  if (SIMULATION_TYPE == VECTOR_TABLE)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)pvt->vectors->icUsed) * (double) sample * optimization_options.two_pi_over_number_samples - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }

  return clock;
  }// calculate_clock_value

//-------------------------------------------------------------------//


static int compareCoherenceQCells (const void *p1, const void *p2)
  {
  return
    ((ts_coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours >
    ((ts_coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((ts_coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours <
    ((ts_coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs

void ts_coherence_options_dump (ts_coherence_OP *ts_coherence_options, FILE *pfile)
  {
  fprintf (stderr, "ts_coherence_options_dump:\n") ;
	fprintf (stderr, "ts_coherence_options->T                         = %e [K]\n",  ts_coherence_options->T) ;
	fprintf (stderr, "ts_coherence_options->relaxation                = %e [s]\n",  ts_coherence_options->relaxation) ;
	fprintf (stderr, "ts_coherence_options->time_step                 = %e [s]\n",  ts_coherence_options->time_step) ;
	fprintf (stderr, "ts_coherence_options->duration                  = %e [s]\n",  ts_coherence_options->duration) ;
	fprintf (stderr, "ts_coherence_options->clock_high                = %e [J]\n",  ts_coherence_options->clock_high) ;
	fprintf (stderr, "ts_coherence_options->clock_low                 = %e [J]\n",  ts_coherence_options->clock_low) ;
	fprintf (stderr, "ts_coherence_options->clock_shift               = %e [J]\n",  ts_coherence_options->clock_shift) ;
	fprintf (stderr, "ts_coherence_options->clock_amplitude_factor    = %e\n",      ts_coherence_options->clock_amplitude_factor) ;
	fprintf (stderr, "ts_coherence_options->radius_of_effect          = %e [nm]\n", ts_coherence_options->radius_of_effect) ;
	fprintf (stderr, "ts_coherence_options->epsilonR                  = %e\n",      ts_coherence_options->epsilonR) ;
	fprintf (stderr, "ts_coherence_options->layer_separation          = %e [nm]\n", ts_coherence_options->layer_separation) ;
	fprintf (stderr, "ts_coherence_options->algorithm                 = %d\n",      ts_coherence_options->algorithm) ;
	fprintf (stderr, "ts_coherence_options->randomize_cells           = %s\n",      ts_coherence_options->randomize_cells ? "TRUE" : "FALSE") ;
	fprintf (stderr, "ts_coherence_options->animate_simulation        = %s\n",      ts_coherence_options->animate_simulation ? "TRUE" : "FALSE") ;
  }

void dump_3x3_complexMatrix(complex A[3][3]){
	printf("-------------------------------------\n");
	printf("%e+j%e, %e+j%e, %e+j%e\n", A[0][0].re, A[0][0].im, A[0][1].re, A[0][1].im, A[0][2].re, A[0][2].im);
	printf("%e+j%e, %e+j%e, %e+j%e\n", A[1][0].re, A[1][0].im, A[1][1].re, A[1][1].im, A[1][2].re, A[1][2].im);
	printf("%e+j%e, %e+j%e, %e+j%e\n", A[2][0].re, A[2][0].im, A[2][1].re, A[2][1].im, A[2][2].re, A[2][2].im);
	printf("-------------------------------------\n");
}
