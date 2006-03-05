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

// percentage of the total neutralizing charge that is located in the active dots
#define CHARGE_DIST 0.0
//!Options for the coherence simulation engine
ts_coherence_OP ts_coherence_options = {1, 1e-15, 1e-16, 7e-11, 3.96e-20, 2.3e-19, -2.3e-19, 0.0, 2.0, 80, 1.0, 0.639, 6.66, 1.2625, EULER_METHOD, ELECTRODE_CLOCKING, TRUE, TRUE} ;

typedef struct
  {
		int number_of_neighbours;
		QCADCell **neighbours;
		int *neighbour_layer;
		double **potentials_plus;
		double **potentials_minus;
		double **potentials_null;
		double self_energy_plus;
		double self_energy_minus;
		double self_energy_null;
		double lambda[8];
		double Gamma[8];
		complex Hamiltonian[3][3];
		QCADCellDot extra_dots[2];
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

void ts_coherence_determine_potentials (QCADCell * cell1, QCADCell * cell2, int neighbour, int layer_separation, ts_coherence_OP *options);
void ts_calculate_self_energies(QCADCell *cell, ts_coherence_OP *options);
static void ts_coherence_refresh_all_potentials (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_coherence_OP *options);
static void run_ts_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const ts_coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy, complex generator[8][3][3], double structureConst[8][8][8], QCADLayer *clocking_layer);
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const ts_coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
double ts_determine_distance(QCADCell *cell1, QCADCell *cell2, int dot_cell_1, int dot_cell_2, double z1, double z2, double cell_height);
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
	double overKBT = 1.0 / (kB*options->T);
  double *energy;
  double average_power=0;
	double two_over_root_3_hbar = -2.0/sqrt(3.0)*OVER_HBAR;
	double two_over_hbar = 2.0 * OVER_HBAR;
	double overRootThree = 1.0 / sqrt(3);
	double tempMath1,tempMath2,tempMath3,tempMath4,tempMath5;
	double neighbour_polarization;
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
	double min_polarization=0, max_polarization=0;
	
	double lambdaSS[8];
	double old_lambda[8];
	double omega[8][8];
	double omegaDotLambda[8];
	complex densityMatrixSS[3][3];
	complex minusOverKBT;
	double potential[6]={1,1,1,1,1,1};
  static double chargePlus[6]	 = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
  static double chargeMinus[6] = {CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
	static double chargeNull[6]  = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE};
	double total_charge[3] = {0,0,0};
	double energyPlus = 0, energyMinus = 0, energyNull = 0;
	
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

	for(i = 0; i < 6; i++){
		total_charge[0] += chargePlus[i];
		total_charge[1] += chargeMinus[i];
		total_charge[2] += chargeNull[i];
	}
	
	printf("total charges %e %e %e\n", total_charge[0], total_charge[1], total_charge[2]);

  // determine which cells are inputs and which are outputs //
  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      {
      // attach the model parameters to each of the simulation cells //
      sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(ts_coherence_model));

      // -- Clear the model pointers so they are not dangling -- //
      ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
      ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus = NULL;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus = NULL;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null = NULL;
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

  // -- refresh all the potential and self energies-- //
  ts_coherence_refresh_all_potentials (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);
  
	// This must go after ts_coherence_refresh_all_potentials 
	for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      ts_calculate_self_energies(sorted_cells[i][j], options);

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
	
	// if users wants to use the electrode clocking then find the clocking layer
	if(options->clocking_scheme == ELECTRODE_CLOCKING){
		command_history_message ("Simulation will use the electrode clocking scheme.\n");
		// find the clocking layer
		for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next){
			if(LAYER_TYPE_CLOCKING == QCAD_LAYER (llItr->data)->type)
				clocking_layer = (QCADLayer *)(llItr->data);
		}
		
		//check to make sure there actually is a clocking layer
		if(clocking_layer == NULL){
			command_history_message ("Critical Error: No clocking layer found!\n");
			set_progress_bar_visible (FALSE) ;
			return(sim_data);
		}
	
		//setup the electrodes and ground plate
//		for (llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next)
//      if (NULL != llItr->data)
//        g_object_set (G_OBJECT (llItr->data), "relative-permittivity", options->epsilonR, NULL) ;
		}else{
			command_history_message ("Simulation will use the zone clocking scheme.\n");
		}
		
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
          continue;
          }

				//find the poential at the location of each of the celll dots
				potential[0] = potential[1] = potential[2] = potential[3] = potential[4] = potential[5] = 0;
 				
				
				//gather the potentials from all the cells neighbours
				for (q = 0; q < ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++){
					neighbour_polarization = (qcad_cell_calculate_polarization (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]));
					
					if(neighbour_polarization > 0){
						potential[0] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][0];	
						potential[1] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][1];
						potential[2] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][2];
						potential[3] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][3];
						potential[4] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][4];
						potential[5] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][5];
					
					}else if(neighbour_polarization < 0){
						potential[0] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][0];	
						potential[1] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][1];
						potential[2] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][2];
						potential[3] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][3];
						potential[4] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][4];
						potential[5] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][5];
					}
						
					potential[0] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][0];	
					potential[1] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][1];
					potential[2] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][2];
					potential[3] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][3];
					potential[4] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][4];
					potential[5] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][5];
				
				}
					
				//get energy values depending on the user clocking scheme choice
				if(options->clocking_scheme == ELECTRODE_CLOCKING){
						
					//gather the potentials from all the clocking electrodes
					for(llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next){
						if(llItr->data != NULL){
							potential[0] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[0].x, sorted_cells[i][j]->cell_dots[0].y, options->cell_elevation+fabs((double)i*options->layer_separation), 0.0);
							potential[1] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[1].x, sorted_cells[i][j]->cell_dots[1].y, options->cell_elevation+fabs((double)i*options->layer_separation), 0.0);
							potential[2] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[2].x, sorted_cells[i][j]->cell_dots[2].y, options->cell_elevation+fabs((double)i*options->layer_separation), 0.0);
							potential[3] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[3].x, sorted_cells[i][j]->cell_dots[3].y, options->cell_elevation+fabs((double)i*options->layer_separation), 0.0);
							potential[4] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y, options->cell_elevation+fabs((double)i*options->layer_separation) - options->cell_height, 0.0);
							potential[5] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y, options->cell_elevation+fabs((double)i*options->layer_separation) - options->cell_height, 0.0);
						}
					}
				
					energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
					energyPlus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
					energyNull = potential[0] * chargeNull[0] + potential[1] * chargeNull[1] + potential[2] * chargeNull[2] + potential[3] * chargeNull[3] + potential[4] * chargeNull[4] + potential[5] * chargeNull[5];
			
					// add the self energies
					energyPlus  += ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_plus;		
					energyMinus += ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_minus;	
					energyNull	+= ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_null;

				}
				
				else if(options->clocking_scheme == ZONE_CLOCKING){
					
					energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
					energyPlus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
					energyNull = sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0];
				}
				
				//generate the hamiltonian in the space of SU(3)
				
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1] = -two_over_hbar * options->gamma ;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2] = -two_over_hbar * options->gamma ;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5] = 0;
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6] = OVER_HBAR * (energyMinus-energyPlus);
				((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7] = two_over_root_3_hbar * (energyNull-energyMinus*0.5-energyPlus*0.5);
				
													
				// generate the steady-state density matrix
				tempMath1 = exp(-energyPlus * overKBT) + exp(-energyMinus * overKBT) + exp(-energyNull * overKBT);
				tempMath2 = exp((energyPlus+energyNull) * overKBT);
				tempMath3 = exp((energyPlus+energyMinus) * overKBT);
				tempMath4 = exp((energyMinus+energyNull) * overKBT);
				tempMath5 = tempMath2 + tempMath3 + tempMath4;
				
				lambdaSS[0] = 2.0 / tempMath1;
				lambdaSS[1] = 2.0 * exp(options->gamma * overKBT) / tempMath1;
				lambdaSS[2] = 2.0 * exp(options->gamma * overKBT) / tempMath1;
				lambdaSS[3] = 0;
				lambdaSS[4] = 0;
				lambdaSS[5] = 0;
				lambdaSS[6] = exp(energyNull * overKBT) * (exp(energyPlus * overKBT) - exp(energyMinus * overKBT)) / tempMath5;
				lambdaSS[7] = (2.0 * tempMath3 - tempMath2 - tempMath4) * overRootThree / tempMath5;
				
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
        if(options->clocking_scheme == ELECTRODE_CLOCKING)
          {
					g_object_set((GObject*)clocking_layer, "time-coord", (double)j * options->time_step, NULL);
  				redraw_sync(NULL, FALSE);
          }
        else
          redraw_async(NULL);
        gdk_flush () ;
        while (gtk_events_pending ())
          gtk_main_iteration () ;
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
    run_ts_coherence_iteration (j, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, total_number_of_inputs, number_samples, options, sim_data, SIMULATION_TYPE, pvt, energy, generator, structureConst, clocking_layer);

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
        if(isnan(((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6])){
					command_history_message ("Critical Error: Simulation engine was trying to set cell polarization to NaN at sample number %d\n", j);
					command_history_message ("This can happen if the cells are to close to the electrodes. Try increasing the Cell Elevation. %d\n", j);
					set_progress_bar_visible (FALSE) ;
          return sim_data;
				}
				
				if (fabs (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6]) > 1.0)
          {
          command_history_message ("I had to abort the simulation at iteration %d because the polarization = %e was diverging.\nPossible cause is the time step is too large.\nAlternatively, you can decrease the relaxation time to reduce oscillations.\n",j, -1*((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[7]);
          command_history_message ("time step was set to %e\n", options->time_step);
					set_progress_bar_visible (FALSE) ;
          return sim_data;
          }
					
				//if(j>100)if(-1 * ((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6] < min_polarization)min_polarization = -1 * ((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6];
				//if(j>100)if(-1 * ((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6] > max_polarization)max_polarization = -1 * ((ts_coherence_model *)sorted_cells[k][l]->cell_model)->lambda[6];
			
				//if(j%1000==0)printf("min = %e max =%e\n", min_polarization, max_polarization);
				
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
      g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->potentials_plus);
			g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->potentials_minus);
			g_free (((ts_coherence_model *)sorted_cells[k][l]->cell_model)->potentials_null);
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
	
  command_history_message ("Total simulation time: %g s\n", (long double)(end_time - start_time));
  set_progress_bar_visible (FALSE) ;
  return sim_data;
  }//run_ts_coherence

// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //

static void run_ts_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const ts_coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy, complex generator[8][3][3], double structureConst[8][8][8], QCADLayer *clocking_layer)
  {
  unsigned int i,j,q, row, col, Nix, Nix1;
	double t = 0;
  unsigned int num_neighbours;
	double overRootThree = 1.0 / sqrt(3);
	double tempMath1,tempMath2,tempMath3,tempMath4,tempMath5;
	double neighbour_polarization;
	double lambda[8];
	complex tempMatrix1[3][3];
	double over_relaxation = 1.0/options->relaxation;
  double lambdaSS[8];
	double omega[8][8];
	double omegaDotLambda[8];
	complex densityMatrixSS[3][3];
	double overKBT = 1.0 / (kB * options->T);
	complex minusOverKBT;
	double two_over_root_3_hbar = -2.0/sqrt(3.0)*OVER_HBAR;
	double two_over_hbar = 2.0 * OVER_HBAR;
	GList *llItr = NULL;
	double potential[6]={0,0,0,0,0,0};
  //static double chargePlus[6]	= {THIRD_QCHARGE, -TWO_THIRDS_QCHARGE, THIRD_QCHARGE, -TWO_THIRDS_QCHARGE, THIRD_QCHARGE, THIRD_QCHARGE};
  //static double chargeMinus[6] = {-TWO_THIRDS_QCHARGE, THIRD_QCHARGE, -TWO_THIRDS_QCHARGE, THIRD_QCHARGE, THIRD_QCHARGE, THIRD_QCHARGE};
	//static double chargeNull[6]  = {THIRD_QCHARGE, THIRD_QCHARGE, THIRD_QCHARGE, THIRD_QCHARGE, -TWO_THIRDS_QCHARGE, -TWO_THIRDS_QCHARGE};
  static double chargePlus[6]	 = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
  static double chargeMinus[6] = {CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
	static double chargeNull[6]  = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE};
	
	double energyPlus = 0, energyMinus = 0, energyNull = 0;
	double cellenergyPlus = 0, cellenergyMinus = 0, cellenergyNull = 0;
	double elevation = 0;
	static double energyNullMin = 0, energyNullMax = 0;

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
				
			//How far above the electrode are the cell active dots
			elevation = options->cell_elevation+fabs((double)i*options->layer_separation);

			//find the poential at the location of each of the celll dots
			potential[0] = potential[1] = potential[2] = potential[3] = potential[4] = potential[5] = 0;
 				
			//gather the potentials from all the cells neighbours
			for (q = 0; q < ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++){
				neighbour_polarization = (qcad_cell_calculate_polarization (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]));
				//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
				//printf("neighbour polarization = %e\n", neighbour_polarization);
				if(neighbour_polarization > 0){
					potential[0] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][0];	
					potential[1] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][1];
					potential[2] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][2];
					potential[3] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][3];
					potential[4] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][4];
					potential[5] += neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus[q][5];
					//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);
				}else if(neighbour_polarization < 0){
					potential[0] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][0];	
					potential[1] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][1];
					potential[2] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][2];
					potential[3] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][3];
					potential[4] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][4];
					potential[5] += -neighbour_polarization * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus[q][5];
					//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);
				}
					
				potential[0] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][0];	
				potential[1] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][1];
				potential[2] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][2];
				potential[3] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][3];
				potential[4] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][4];
				potential[5] += (1 - fabs(neighbour_polarization)) * ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null[q][5];
				//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);
			}
			
			//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);
			//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
			//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);		
			//get energy values depending on the user clocking scheme choice
			cellenergyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
			cellenergyMinus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
			cellenergyNull = potential[0] * chargeNull[0] + potential[1] * chargeNull[1] + potential[2] * chargeNull[2] + potential[3] * chargeNull[3] + potential[4] * chargeNull[4] + potential[5] * chargeNull[5];
			//printf("PLUS %e \tMINUS %e \tNUll %e \n", energyPlus, energyMinus, energyNull);		
			
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//potential[0] = potential[1] = potential[2] = potential[3] = potential[4] = potential[5] = 0;
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			//************************************************* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			
			if(options->clocking_scheme == ELECTRODE_CLOCKING){
						
			//gather the potentials from all the clocking electrodes
			for(llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next){
				if(llItr->data != NULL){
					//printf("ElectrodeNULL =%e\n",qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y, elevation - options->cell_height, t)*(-TWO_THIRDS_QCHARGE));
					potential[0] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[0].x, sorted_cells[i][j]->cell_dots[0].y, elevation, t);
					potential[1] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[1].x, sorted_cells[i][j]->cell_dots[1].y, elevation, t);
					potential[2] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[2].x, sorted_cells[i][j]->cell_dots[2].y, elevation, t);
					potential[3] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	sorted_cells[i][j]->cell_dots[3].x, sorted_cells[i][j]->cell_dots[3].y, elevation, t);
					potential[4] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y, elevation - options->cell_height, t);
					potential[5] += qcad_electrode_get_potential((QCADElectrode *)(llItr->data),	((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y, elevation - options->cell_height, t);
				}
			}
			
			//printf("%e %e %e %e %e %e\n", potential[0], potential[1], potential[2], potential[3], potential[4], potential[5]);		
			//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");	
				energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
				energyMinus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
				energyNull = potential[0] * chargeNull[0] + potential[1] * chargeNull[1] + potential[2] * chargeNull[2] + potential[3] * chargeNull[3] + potential[4] * chargeNull[4] + potential[5] * chargeNull[5];
				
				//if(sample_number%1000==0)printf("El+ %e\tEl- %e\tEln %e\tDEl %e\tInt+ %e\tInt- %e\tIntn %e\tSel+ %e\tSel- %e\tSeln %e\n", energyPlus, energyMinus, energyNull, energyMinus-energyNull, cellenergyPlus, cellenergyMinus, cellenergyNull, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_plus, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_minus,((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_null);
				
				// add the self energies
				energyPlus  += ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_plus;		
				energyMinus += ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_minus;	
				energyNull	+= ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->self_energy_null;
				
			}
				
			else if(options->clocking_scheme == ZONE_CLOCKING){
					
				energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
				energyMinus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
				energyNull = calculate_clock_value(sorted_cells[i][j]->cell_options.clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);

			}
			
										
			//if(energyNull<energyNullMin)energyNullMin = energyNull;
			//if(energyNull>energyNullMax)energyNullMax = energyNull;
			//printf("MIN = %e MAX = %e\n", energyNullMin, energyNullMax);	
				
			//printf("Ep %e\t Em %e\t En %e\tpPLUS %e\tpMINUS %e\tpNUll %e\tEp-En %e\tEm-En %e\n", energyPlus, energyMinus, energyNull, (energyPlus-cellenergyPlus)/energyPlus * 100, (energyMinus-cellenergyMinus)/energyMinus * 100, (energyNull-cellenergyNull)/energyNull *100, energyPlus - energyNull, energyMinus-energyNull);			
			//generate the hamiltonian in the space of SU(3)
			//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
				
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[0] = 0;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[1] = -two_over_hbar * options->gamma ;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[2] = -two_over_hbar * options->gamma ;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[3] = 0;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[4] = 0;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[5] = 0;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[6] = OVER_HBAR * (energyMinus-energyPlus);
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->Gamma[7] = two_over_root_3_hbar * (energyNull-energyMinus*0.5-energyPlus*0.5);
				
													
			// generate the steady-state density matrix
			tempMath1 = exp(-energyPlus * overKBT) + exp(-energyMinus * overKBT) + exp(-energyNull * overKBT);
			tempMath2 = exp((energyPlus+energyNull) * overKBT);
			tempMath3 = exp((energyPlus+energyMinus) * overKBT);
			tempMath4 = exp((energyMinus+energyNull) * overKBT);
			tempMath5 = tempMath2 + tempMath3 + tempMath4;
				
			lambdaSS[0] = 2.0 / tempMath1;
			lambdaSS[1] = 2.0 * exp(options->gamma * overKBT) / tempMath1;
			lambdaSS[2] = 2.0 * exp(options->gamma * overKBT) / tempMath1;
			lambdaSS[3] = 0;
			lambdaSS[4] = 0;
			lambdaSS[5] = 0;
			lambdaSS[6] = exp(energyNull * overKBT) * (exp(energyPlus * overKBT) - exp(energyMinus * overKBT)) / tempMath5;
			lambdaSS[7] = (2.0 * tempMath3 - tempMath2 - tempMath4) * overRootThree / tempMath5;
			
			if(isnan(lambdaSS[0]))printf("lambdaSS[0] is NaN\n");
			if(isnan(lambdaSS[1]))printf("lambdaSS[1] is NaN\n");	
			if(isnan(lambdaSS[2]))printf("lambdaSS[2] is NaN\n");	
			if(isnan(lambdaSS[3]))printf("lambdaSS[3] is NaN\n");	
			if(isnan(lambdaSS[4]))printf("lambdaSS[4] is NaN\n");	
			if(isnan(lambdaSS[5]))printf("lambdaSS[5] is NaN\n");	
			if(isnan(lambdaSS[6]))printf("lambdaSS[6] is NaN\n");	
			if(isnan(lambdaSS[7]))printf("lambdaSS[7] is NaN\n");		
			
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
	  //printf("\n\n\n");
 }//run_iteration

//-------------------------------------------------------------------//
// Refreshes the interaction energies between cells in the design
// This function is specific to the six dot cells
static void ts_coherence_refresh_all_potentials (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_coherence_OP *options)
  {
  int icNeighbours = 0 ;
  ts_coherence_model *cell_model = NULL ;
  int i,j,k;

  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
			// determine the x,y coordinates of the 2 extra dots required by this model
			// the z-coordinate is set by the user and available in the options structure
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x = sorted_cells[i][j]->cell_dots[0].x; 
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y = (sorted_cells[i][j]->cell_dots[1].y+sorted_cells[i][j]->cell_dots[0].y)/2.0;
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x = sorted_cells[i][j]->cell_dots[3].x; 
			((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y = (sorted_cells[i][j]->cell_dots[2].y+sorted_cells[i][j]->cell_dots[3].y)/2.0;
			}
			
  // calculate the potentials for each cell //
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // free up memory from previous simulations //
      g_free ((cell_model = (ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours);
      g_free (cell_model->potentials_plus);
			g_free (cell_model->potentials_minus);
			g_free (cell_model->potentials_null);
      g_free (cell_model->neighbour_layer);
      cell_model->neighbours = NULL;
      cell_model->neighbour_layer = NULL;
      cell_model->potentials_plus = NULL;
			cell_model->potentials_minus = NULL;
			cell_model->potentials_null = NULL;

      // select all neighbours within the provided radius //
      cell_model->number_of_neighbours = icNeighbours =
        select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((ts_coherence_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
             ((ts_coherence_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));
			
			//printf("cell dot[0] is located at x=%e y=%e, extra dot x=%e y=%e and x=%e y=%e\n", sorted_cells[i][j]->cell_dots[0].x,sorted_cells[i][j]->cell_dots[0].y, ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x,((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y,((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x,((ts_coherence_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y);
			
			
      // for each of the cell neighbours
			if (icNeighbours > 0)
        {
        cell_model->potentials_plus  = g_malloc0 (sizeof (double *) * icNeighbours);
				cell_model->potentials_minus = g_malloc0 (sizeof (double *) * icNeighbours);
				cell_model->potentials_null  = g_malloc0 (sizeof (double *) * icNeighbours);
				
        // ensure no memory allocation error has ocurred //
        if (((ts_coherence_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
            ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_plus == NULL ||
            ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_minus == NULL ||
            ((ts_coherence_model *)sorted_cells[i][j]->cell_model)->potentials_null == NULL)
          exit (1);

        for (k = 0; k < icNeighbours; k++){
          //allocate memory for the potential at each dot in the six dot cell
					cell_model->potentials_plus[k]  = g_malloc0 (sizeof (double) * 6);
					cell_model->potentials_minus[k] = g_malloc0 (sizeof (double) * 6);
					cell_model->potentials_null[k]  = g_malloc0 (sizeof (double) * 6);
					}
					
        for (k = 0; k < icNeighbours; k++){
          
					// set the potentials of this cell and its neighbour //
          ts_coherence_determine_potentials (sorted_cells[i][j], cell_model->neighbours[k], k, ABS(i-cell_model->neighbour_layer[k]), options);
					}
          
        }
      }
  }//refresh_all_potentials


//-------------------------------------------------------------------//
// calculates the self energy of the cell for each of the three states
// i.e. the interaction energy of the electrons and neutralizing charges within the cell
void ts_calculate_self_energies(QCADCell *cell, ts_coherence_OP *options){
	
	int i,j;
	double distance;
	double Constant = 1 / (4 * PI * EPSILON * options->epsilonR * 1e-9);
	// these variables apply only to the six dot cells!!
  static double chargePlus[6]	 = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
  static double chargeMinus[6] = {CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
	static double chargeNull[6]  = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE};

	g_assert(cell != NULL);
	
	((ts_coherence_model *)cell->cell_model)->self_energy_plus	= 0;
	((ts_coherence_model *)cell->cell_model)->self_energy_minus = 0;
	((ts_coherence_model *)cell->cell_model)->self_energy_null	= 0;
	
	for(i = 0; i < 5; i++)
		for(j = i+1; j < 6; j++){
			distance = ts_determine_distance (cell, cell, i, j, 0.0, 0.0, options->cell_height);
			((ts_coherence_model *)cell->cell_model)->self_energy_plus	+= (chargePlus[i] * chargePlus[j]) / distance;
			((ts_coherence_model *)cell->cell_model)->self_energy_minus += (chargeMinus[i] * chargeMinus[j]) / distance;
			((ts_coherence_model *)cell->cell_model)->self_energy_null	+= (chargeNull[i] * chargeNull[j]) / distance;
			
			}
			
	((ts_coherence_model *)cell->cell_model)->self_energy_plus	*= Constant;
	((ts_coherence_model *)cell->cell_model)->self_energy_minus *= Constant;
	((ts_coherence_model *)cell->cell_model)->self_energy_null	*= Constant;
	
	printf("Self Energies +=%e -=%e Null=%e\n", ((ts_coherence_model *)cell->cell_model)->self_energy_plus, ((ts_coherence_model *)cell->cell_model)->self_energy_minus, ((ts_coherence_model *)cell->cell_model)->self_energy_null);
}


//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
// this function applies only to the six dot cells!!
void ts_coherence_determine_potentials (QCADCell * cell1, QCADCell * cell2, int neighbour, int layer_separation, ts_coherence_OP *options)
  {
  int k;
  int j;

  double distance = 0;
	//1e-9 in following is to convert from nm to meters in the final result
  double Constant = 1 / (4 * PI * EPSILON * options->epsilonR * 1e-9);
	
	// these variables apply only to the six dot cells!!
  static double chargePlus[6]	 = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
  static double chargeMinus[6] = {CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0 - QCHARGE, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE};
	static double chargeNull[6]  = {CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, CHARGE_DIST * QCHARGE / 2.0, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE, (1.0 - CHARGE_DIST) * QCHARGE - QCHARGE};
	
  g_assert (cell1 != NULL);
  g_assert (cell2 != NULL);
  g_assert (cell1 != cell2);

	// potential_{plus,minus,null}[(int) which neighbour][(int) which dot in my cell]
	//the potential energy of the three basis states of neigbouring cells cast onto the six dots of this cell
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][0]  = 
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][1]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][2]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][3]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][4]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][5]  = 0;
	
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][0]  = 
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][1]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][2]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][3]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][4]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][5]  = 0;
	
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][0]  = 
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][1]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][2]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][3]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][4]	= 
	((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][5]  = 0;
	
  for (k = 0; k < 6; k++){
    for (j = 0; j < 6; j++){
		
      // determine the distance between the dots //
      // printf("layer seperation = %d\n", layer_seperation);
			distance = ts_determine_distance (cell1, cell2, k, j, 0.0, (double)layer_separation * ((ts_coherence_OP *)options)->layer_separation, options->cell_height);
      g_assert (distance > 0);
			//printf("distance = %e\n",distance);
      ((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][k]		+= chargePlus[j] / distance;
      ((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][k]		+= chargeMinus[j] / distance;
			((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][k]		+= chargeNull[j] / distance;
      }//for other dots
	
		((ts_coherence_model *)cell1->cell_model)->potentials_plus[neighbour][k]		*= Constant;
		((ts_coherence_model *)cell1->cell_model)->potentials_minus[neighbour][k]		*= Constant;
		((ts_coherence_model *)cell1->cell_model)->potentials_null[neighbour][k]		*= Constant;
		}
		/*printf("PLUS %e %e %e %e %e %e\n"
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][0]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][1]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][2]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][3]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][4]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_plus[0][5]
		);
		
				printf("MINUS %e %e %e %e %e %e\n"
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][0]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][1]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][2]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][3]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][4]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_minus[0][5]
		);
		
				printf("NULL %e %e %e %e %e %e\n"
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][0]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][1]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][2]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][3]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][4]
		, ((ts_coherence_model *)cell1->cell_model)->potentials_null[0][5]
		);
		*/				
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

// -- determine the distance between the centers of two qdots in different cells **** [IN nm]****** -- //
// -- this function is particular to the six dot cells
double ts_determine_distance(QCADCell *cell1, QCADCell *cell2, int dot_cell_1, int dot_cell_2, double z1, double z2, double cell_height)
  {
  double x, y, z;

	// make sure we are not trying to determine the distance between the same dot in the same cell
	// The problem is that this distance is generally used in the denominator of some other calculations
	// and may result in infinities
	if(cell1 == cell2)
		g_assert(dot_cell_1 != dot_cell_2);
	
	if(dot_cell_1 < 4 && dot_cell_2 < 4){
		x = cell1->cell_dots[dot_cell_1].x - cell2->cell_dots[dot_cell_2].x;
		y = cell1->cell_dots[dot_cell_1].y - cell2->cell_dots[dot_cell_2].y;
		z = z1-z2;

	}else if(dot_cell_1 < 4 && dot_cell_2 >= 4){
		x = cell1->cell_dots[dot_cell_1].x - ((ts_coherence_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].x;
		y = cell1->cell_dots[dot_cell_1].y - ((ts_coherence_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].y;
		z = z1 - (z2-cell_height); 

	}else if(dot_cell_1 >= 4 && dot_cell_2 < 4){ 
		x = ((ts_coherence_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].x - cell2->cell_dots[dot_cell_2].x;
		y = ((ts_coherence_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].y - cell2->cell_dots[dot_cell_2].y;
		z = (z1-cell_height) - z2;

	}else if(dot_cell_1 >= 4 && dot_cell_2 >= 4){ 
		x = ((ts_coherence_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].x - ((ts_coherence_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].x;
		y = ((ts_coherence_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].y - ((ts_coherence_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].y;
		z = z1-z2;	
	}
	
	//printf("x = %e y = %e z = %e\n",x,y,z);
	//printf("dist = %e\n",sqrt (x * x + y * y + z * z));
  return sqrt (x * x + y * y + z * z);
  }//determine_distance
	

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
