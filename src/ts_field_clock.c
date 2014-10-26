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

#ifdef HAVE_FORTRAN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <blaswrap.h>
#include <f2c.h>
#include <clapack.h>

#include "objects/QCADCell.h"
#include "objects/QCADElectrode.h"
#include "objects/QCADRectangleElectrode.h"
#include "objects/QCADClockingLayer.h"
#include "simulation.h"
#include "ts_field_clock.h"
#include "custom_widgets.h"
#include "global_consts.h"
#ifdef GTK_GUI
#ifdef DESIGNER
#include "custom_widgets.h"
#endif /* def DESIGNER */
#include "callback_helpers.h"
#endif /* def GTK_GUI */
#include "intl.h"

// Calculates the magnitude of the 3D energy vector
#define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
//(sqrt((4.0*(G)*(G) + (P)*(P))*over_hbar_sqr))
// Calculates the temperature ratio
#define temp_ratio(P,G,T) (hypot((G),(P)*0.5)/((T) * kB))
#define isnan(x) ((x) != (x))

//!Options for the coherence simulation engine
ts_fc_OP ts_fc_options = {1, 7e-15, 7e-11, 1e-4, 6.0e-20, 2.3e-19, -2.3e-19, 0.0, 1.4285e10,2.0, 1.0, 1, 8, 1, 5, 1, 1, 1, 200, 1e6, EULER_METHOD, ELECTRODE_CLOCKING, FALSE, TRUE, TRUE} ;

typedef struct
	{
		int number_of_neighbours;
		QCADCell **neighbours;
		int *neighbour_layer;
		double wavefunction[3];
		double **potentials_plus;
		double **potentials_minus;
		double **potentials_null;
		double self_energy_plus;
		double self_energy_minus;
		double self_energy_null;
		double polarization;
		QCADCellDot extra_dots[4];
	} ts_fc_model;

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
		double over_kBT;
		double over_lambda_x;
		double over_lambda_y;
		double over_duration;
	} ts_fc_optimizations;

// instance of the optimization options;
static ts_fc_optimizations optimization_options;
void ts_fc_determine_potentials (QCADCell * cell1, QCADCell * cell2, int neighbour, int layer_separation, ts_fc_OP *options);
//void ts_fc_determine_potential (double *Grid, int Nx, int Ny, int Nz, double dx, double dy, double dz, int xmin, int ymin, double t, QCADLayer *clocking_layer, ts_fc_OP *options);
void ts_fc_calculate_self_energies(QCADCell *cell, ts_fc_OP *options);
static void ts_fc_refresh_all_potentials (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_fc_OP *options);
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const ts_fc_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
double ts_fc_determine_distance(QCADCell *cell1, QCADCell *cell2, int dot_cell_1, int dot_cell_2, double z1, double z2, double cell_height, double counter_ion);
//double *compare_arr(double *A, double *B, int length);
//static inline int search_array(double *A, double cmp, int num_elements );
static inline void array_copy(double *Arr1, double *Arr0, int length);
static inline double find_min(double *array, int num_elements);
static inline double expectation (double **psi, double **Mat, int dim1x, int dim1y, int dim2x, int dim2y);
static inline double** matrix_mult(double **Mat1, double **Mat2, int dim1x, int dim1y, int dim2x, int dim2y);
static inline double** transpose (double **Mat_in, int dimX, int dimY);
static inline void array_mult_by_const(double *arr1, double constant, int length, double *arr0);
static inline void array_add(double *arr1, double *arr2, int length, double *arr0);
static inline double interp(double *Grid, double x1, double y1, double z1, double x2, double y2, double z2, int Nx, int Ny, int Nz, double dx, double dy, double dz);
static inline double calculate_clock_value_cc (double x, double y, double t, const ts_fc_OP *options, VectorTable *pvt);

#define size 3

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_ts_fc_simulation (int SIMULATION_TYPE, DESIGN *design, ts_fc_OP *options, VectorTable *pvt)
{
	int i, j, k, l, q, row, col, number_of_cell_layers, *number_of_cells_in_layer;
	QCADCell ***sorted_cells = NULL ;
	int total_number_of_inputs = design->bus_layout->inputs->icUsed;
	unsigned long int number_samples;
	//number of points to record in simulation results //
	//simulations can have millions of points and there is no need to plot them all //
	unsigned long int number_recorded_samples = 3000;
	unsigned long int record_interval;
	double average_power=0;
	double two_over_root_3_hbar = -2.0/sqrt(3.0)*OVER_HBAR;
	double two_over_hbar = 2.0 * OVER_HBAR;
	double overRootThree = 1.0 / sqrt(3);
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
	
	double t;
	double old_polarization;
	double new_polarization;
	double driver_polarization;
	//double tolerance = options->convergence_tolerance;
	double **H = NULL;
	double **A = NULL;
	double **generator = NULL;
	doublereal *HT = NULL;
	doublereal *DUMMY = NULL;
	doublereal *VR = NULL;
	doublereal *br = NULL;
	doublereal *bi = NULL;
	doublereal *WORK = NULL;
	int h_iter1, h_iter2, cmp, fb1, fb2; 
	integer ok, c1, c2, c3;
	char c4, c5;
	double dx = options->dx;
	double dy = options->dy;
	double dz = options->dz;
	double over_dx = 1/dx;
	double over_dy = 1/dy;
	double over_dz = 1/dz;
	
	double P_plus, P_minus, P_null, Z, over_Z;
	double Pplus, Pminus, Pnull;
	
	int xmin, xmax, ymin, ymax, Nx, Ny, Nz;
	int loc1, loc2, x1, x2, y1, y2, z1, z2;
	double pot1, pot2, temp_pot;
	QCADCell *cell;
	QCADCell *cell_neighbour;
	
	ts_fc_model *current_cell_model = NULL ;
	int s_flag = 0;
	int sim_flag = 0;

	generator = (double**)malloc(size*sizeof(double*));
	for (i = 0; i < size; i++) {
		generator[i] = (double*)malloc(size*sizeof(double));
	}
	
	generator[0][0] = -1;
	generator[0][1] = 0;
	generator[0][2] = 0;
	generator[1][0] = 0;
	generator[1][1] = 1;
	generator[1][2] = 0;
	generator[2][0] = 0;
	generator[2][1] = 0;
	generator[2][2] = 0;
	
	
	double potential[8]={1,1,1,1,1,1,1,1};
	
	
	double chargePlus[8]	 = { 0,  - QCHARGE,  0,  - QCHARGE, 0, 0, QCHARGE, QCHARGE};
	double chargeMinus[8] = {- QCHARGE,  0,   - QCHARGE,  0, 0, 0, QCHARGE, QCHARGE};
	double chargeNull[8]  = {0,  0, 0,  0, - QCHARGE, - QCHARGE, QCHARGE, QCHARGE};
		
	
	double total_charge[3] = {0,0,0};
	double energyPlus = 0, energyMinus = 0, energyNull = 0;
	
	double *Grid = NULL;
    double xd, yd, zd;
	double E_val;
	
	
	//!Maximum exponent to the exponential function that will not result in infinity
	max_exponent = log(G_MAXDOUBLE);
	

	STOP_SIMULATION = FALSE;
	
	// -- get the starting time for the simulation -- //
	if ((start_time = time (NULL)) < 0)
		fprintf (stderr, "Could not get start time\n");
	
	// determine the number of samples from the user options //
	number_samples = (unsigned long int)(ceil (options->duration/options->time_step));
	

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
	optimization_options.clock_prefactor = (options->clock_high - options->clock_low) * options->Emax;
	optimization_options.clock_shift = (options->clock_high + options->clock_low) * 0.5;
	optimization_options.four_pi_over_number_samples = FOUR_PI / (double)number_samples;
	optimization_options.two_pi_over_number_samples = TWO_PI / (double)number_samples;
	optimization_options.over_kBT = 1/(options->temperature * 1.3806503e-23);
	optimization_options.over_lambda_x = 1/(options->lambda_x);
	optimization_options.over_lambda_y = 1/(options->lambda_y);
	optimization_options.over_duration = 1/(options->duration);
	
	// -- spit out some messages for possible debugging -- //
	command_history_message ("About to start the three-state field clocked simulation with %d samples\n", number_samples);
	command_history_message ("%d samples will be recorded for graphing.\n", number_recorded_samples);
	set_progress_bar_visible (TRUE) ;
	set_progress_bar_label ("Three-State Field Clocked simulation:") ;
	set_progress_bar_fraction (0.0) ;
	
	// Fill in the cell arrays necessary for conducting the simulation
	simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;
	
	for(i = 0; i < 8; i++){
		total_charge[0] += chargePlus[i];
		total_charge[1] += chargeMinus[i];
		total_charge[2] += chargeNull[i];
	}
	
	//printf("total charges %e %e %e\n", total_charge[0], total_charge[1], total_charge[2]);
	
	// determine which cells are inputs and which are outputs //
	for(i = 0; i < number_of_cell_layers; i++)
		for(j = 0; j < number_of_cells_in_layer[i]; j++)
		{
			// attach the model parameters to each of the simulation cells //
			sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(ts_fc_model));
			
			// -- Clear the model pointers so they are not dangling -- //
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_plus = NULL;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_minus = NULL;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_null = NULL;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->wavefunction[0] = 0;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->wavefunction[1] = 0;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->wavefunction[2] = 1;
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
	ts_fc_refresh_all_potentials (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);
	
	// This must go after ts_fc_refresh_all_potentials 
	for(i = 0; i < number_of_cell_layers; i++)
		for(j = 0; j < number_of_cells_in_layer[i]; j++)
			ts_fc_calculate_self_energies(sorted_cells[i][j], options);
	

	
	// -- sort the cells with respect to the neighbour count -- //
	// -- this is done so that majority gates are evalulated last -- //
	// -- to ensure that all the signals have arrived first -- //
	// -- kept getting wrong answers without this -- //
	
	// The following line causes a segfault when the design consists of a single cell
	//  printf("The Ek to the first cells neighbour is %e [eV]\n",((ts_fc_model *)sorted_cells[0][0]->cell_model)->Ek[0]/1.602e-19);
	
	// randomize the cells in the design as to minimize any numerical problems associated //
	// with having cells simulated in some predefined order: //
	// for each layer ...
	/*
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
	*/
	
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
		
			
		xmin = 1000;
		xmax = -1000;
		ymin = 1000;
		ymax = -1000;
		
		for (i = 0; i < number_of_cell_layers; i++) {
			for (j = 0; j < number_of_cells_in_layer[i]; j++)
			{
				cell = sorted_cells[i][j] ;
				if (cell->cell_dots[3].x < xmin)
					xmin = cell->cell_dots[3].x;
				if (cell->cell_dots[1].x > xmax)
					xmax = cell->cell_dots[1].x;
				if (cell->cell_dots[0].y < ymin)
					ymin = cell->cell_dots[0].y;
				if (cell->cell_dots[2].y > ymax)
					ymax = cell->cell_dots[2].y;
			}
		}
		
		QCADRectangleElectrode *rc_electrode;
		QCADElectrode *electrode_src;
		
		for(llItr = clocking_layer->lstObjs; llItr != NULL; llItr = llItr->next){
			if(llItr->data != NULL){
				rc_electrode = (QCADRectangleElectrode *)(llItr->data);
				electrode_src = (QCADElectrode *)(llItr->data);
				//printf("%f\n",electrode_src->electrode_options.phase);
				if ( rc_electrode->precompute_params.pt[0].xWorld < xmin )
					xmin = rc_electrode->precompute_params.pt[0].xWorld;
				if ( rc_electrode->precompute_params.pt[1].xWorld > xmax )
					xmax = rc_electrode->precompute_params.pt[1].xWorld;
				if ( rc_electrode->precompute_params.pt[0].yWorld < ymin )
					ymin = rc_electrode->precompute_params.pt[0].yWorld;
				if ( rc_electrode->precompute_params.pt[2].yWorld > ymax )
					ymax = rc_electrode->precompute_params.pt[2].yWorld;
			}
		}
		
		xmin = xmin-3;
		xmax = xmax+3;
		ymin = ymin-3;
		ymax = ymax+3;
		
		//printf("xmin = %d, xmax = %d, ymin = %d, ymax = %d\n", xmin, xmax, ymin, ymax);
		
		
		Nx = ceil((xmax-xmin)/dx)+1;
		Ny = ceil((ymax-ymin)/dy)+1;
		
		llItr = clocking_layer->lstObjs;			
		QCADElectrode *electrode = (QCADElectrode *)(llItr->data);
		Nz = ceil((electrode->precompute_params.two_z_to_ground)/(2*dz))+1;
		
		//double init_voltage = qcad_electrode_get_voltage (electrode, 2E-12);
		Grid = (double*)malloc(Nx*Ny*Nz*sizeof(double));
		for (i = 0; i < Nx*Ny*Nz; i++) {
			Grid[i] = 0;
		}
		get_grid_param(Nx,Ny,Nz,dx,dy,dz,xmin,ymin); 
		create_grid(Nx,Ny,Nz);
		
	}
	else{
		command_history_message ("Simulation will use the continuous clocking scheme.\n");
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
	
	int iter;
	// perform the iterations over all samples //
	for (j = 0; j < number_samples; j++)
    {
		if (0 == j % 100 || j == number_samples - 1)
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
				drain_gtk_events () ;
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
		
		
		
		// Converge the steady state coherence vector for each cell so that the simulation starts without any transients //
		
		t = options->time_step * j; 
		
		if(options->clocking_scheme == ELECTRODE_CLOCKING){
			ts_fc_determine_potential (Grid, Nx, Ny, Nz, dx, dy, dz, xmin, ymin, t, clocking_layer, options);
		}
		
		
		 int disp_loc;
		 
		/*
		if (j == 0) {
		for (fb1 = 0; fb1 < Nx; fb1++) {
			for (fb2 = 0; fb2 < Nz; fb2++) {
				disp_loc = fb1*Ny*Nz+(Ny/2)*Nz+fb2;
				printf("%f\n", Grid[disp_loc]);
			}
		}
		STOP_SIMULATION = TRUE;
		}
		*/
		
		iter = 0;
		stable = FALSE;
		s_flag = 1;
		
		while (s_flag)
		{
			iter = iter+1;
			s_flag = 0;
			
			for (fb1 = 0; fb1 < number_of_cell_layers; fb1++)
				for (fb2 = 0; fb2 < number_of_cells_in_layer[fb1]; fb2++)
				{
					cell = sorted_cells[fb1][fb2] ;
					current_cell_model = ((ts_fc_model *)cell->cell_model) ;
					if (((QCAD_CELL_INPUT == sorted_cells[fb1][fb2]->cell_function)||
						 (QCAD_CELL_FIXED == sorted_cells[fb1][fb2]->cell_function)))
					{
						driver_polarization = (qcad_cell_calculate_polarization (((ts_fc_model *)sorted_cells[fb1][fb2])));
						if (driver_polarization >= 0) {
							current_cell_model->wavefunction[0] = driver_polarization;
							current_cell_model->wavefunction[1] = 0;
							current_cell_model->wavefunction[2] = 1 - driver_polarization;
						}
						else {
							current_cell_model->wavefunction[0] = 0;
							current_cell_model->wavefunction[1] = fabs(driver_polarization);
							current_cell_model->wavefunction[2] = 1 - fabs(driver_polarization);
						}
						continue;
					}
														
					old_polarization = current_cell_model->polarization;
					
					//find the poential at the location of each of the celll dots
					potential[0] = potential[1] = potential[2] = potential[3] = potential[4] = potential[5] = 0;
					
					
					//gather the potentials from all the cells neighbours
					for (q = 0; q < ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->number_of_neighbours; q++){
						cell_neighbour = ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->neighbours[q];
						
						//printf("%f, %f, %f\n", ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0],((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1],((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2]);
													
						potential[0] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][0];	
						potential[1] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][1];
						potential[2] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][2];
						potential[3] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][3];
						potential[4] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][4];
						potential[5] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[0] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_plus[q][5];
						
						
						potential[0] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][0];	
						potential[1] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][1];
						potential[2] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][2];
						potential[3] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][3];
						potential[4] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][4];
						potential[5] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[1] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_minus[q][5];
						
						
						potential[0] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][0];	
						potential[1] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][1];
						potential[2] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][2];
						potential[3] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][3];
						potential[4] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][4];
						potential[5] += ((ts_fc_model *)cell_neighbour->cell_model)->wavefunction[2] * ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->potentials_null[q][5];
						
					}
					
					//get energy values depending on the user clocking scheme choice
					if(options->clocking_scheme == ELECTRODE_CLOCKING){
						
						for (k = 0; k < 6; k++) {
							if (k < 4) {
								if (k == 1 || k == 3) {
									continue;
								}
								xd = sorted_cells[fb1][fb2]->cell_dots[k].x;
								yd = sorted_cells[fb1][fb2]->cell_dots[k].y;
								zd = options->cell_elevation+options->cell_height;
								
								x1 = (xd-xmin)*over_dx;
								x2 = (x1-floor(x1))*dx;
								y1 = (yd-ymin)*over_dy;
								y2 = (y1-floor(y1))*dy;
								z1 = zd*over_dz;
								z2 = (z1-floor(z1))*dz;
								
								potential[k] += interp(Grid,x1,y1,z1,x2,y2,z2,Nx,Ny,Nz,dx,dy,dz);
								potential[k+1] += interp(Grid,x1,y1,z1,x2,y2,z2,Nx,Ny,Nz,dx,dy,dz);
								//printf("%d = %e\n", k, potential[k]);
							}
							else {
								xd = ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->extra_dots[k-4].x;
								yd = ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->extra_dots[k-4].y;
								zd = options->cell_elevation;
								
								x1 = (xd-xmin)*over_dx;
								x2 = (x1-floor(x1))*dx;
								y1 = (yd-ymin)*over_dy;
								y2 = (y1-floor(y1))*dy;
								z1 = zd*over_dz;
								z2 = (z1-floor(z1))*dz;
								
								potential[k] += interp(Grid,x1,y1,z1,x2,y2,z2,Nx,Ny,Nz,dx,dy,dz);
								//printf("%d = %e\n", k, potential[k]);
							}
						}
						
						
						energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
						energyMinus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
						energyNull = potential[0] * chargeNull[0] + potential[1] * chargeNull[1] + potential[2] * chargeNull[2] + potential[3] * chargeNull[3] + potential[4] * chargeNull[4] + potential[5] * chargeNull[5];
						
												
						// add the self energies
						energyPlus  += ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_plus;		
						energyMinus += ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_minus;	
						energyNull	+= ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_null;
						
					}
					
					else if(options->clocking_scheme == CONT_CLOCKING){
						
						for (k = 0; k < 6; k++) {
							if (k < 4) {
								E_val = calculate_clock_value_cc (sorted_cells[fb1][fb2]->cell_dots[k].x, sorted_cells[fb1][fb2]->cell_dots[k].y, t, options, pvt);
							}
							else {
								E_val = 0;
							}
							potential[k] += 1e-9*E_val*options->cell_height;
							//printf("%d = %e\n",k, potential[k]);
						}
						
						energyPlus = potential[0] * chargePlus[0] + potential[1] * chargePlus[1] + potential[2] * chargePlus[2] + potential[3] * chargePlus[3] + potential[4] * chargePlus[4] + potential[5] * chargePlus[5];
						energyMinus = potential[0] * chargeMinus[0] + potential[1] * chargeMinus[1] + potential[2] * chargeMinus[2] + potential[3] * chargeMinus[3] + potential[4] * chargeMinus[4] + potential[5] * chargeMinus[5];
						energyNull = potential[0] * chargeNull[0] + potential[1] * chargeNull[1] + potential[2] * chargeNull[2] + potential[3] * chargeNull[3] + potential[4] * chargeNull[4] + potential[5] * chargeNull[5];
						
						// add the self energies
						energyPlus  += ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_plus;		
						energyMinus += ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_minus;	
						energyNull	+= ((ts_fc_model *)sorted_cells[fb1][fb2]->cell_model)->self_energy_null;
						
					}
					
					if (options->temp_model) {
						P_plus = exp(-energyPlus*optimization_options.over_kBT);
						P_minus = exp(-energyMinus*optimization_options.over_kBT);
						P_null = exp(-energyNull*optimization_options.over_kBT);
						
						//Z = exp(-(energyPlus + energyMinus + energyNull)*optimization_options.over_kBT);
						Z = P_plus + P_minus + P_null;
						over_Z = 1.0/Z;
						
						Pplus = P_plus*over_Z;
						Pminus = P_minus*over_Z;
						Pnull = P_null*over_Z;
						
						//printf("Pplus = %f, Pminus = %f, Pnull = %f\n", P_plus, P_minus, P_null);
						//printf("Pplus = %f, Pminus = %f, Pnull = %f\n", Pplus, Pminus, Pnull);
						
						/*
						 rho[0] = Pplus*chargePlus[0] + Pminus*chargeMinus[0] + Pnull*chargeNull[0];
						 rho[1] = Pplus*chargePlus[1] + Pminus*chargeMinus[1] + Pnull*chargeNull[1];
						 rho[2] = Pplus*chargePlus[2] + Pminus*chargeMinus[2] + Pnull*chargeNull[2];
						 rho[3] = Pplus*chargePlus[3] + Pminus*chargeMinus[3] + Pnull*chargeNull[3];
						 rho[4] = Pplus*chargePlus[4] + Pminus*chargeMinus[4] + Pnull*chargeNull[4];
						 rho[5] = Pplus*chargePlus[5] + Pminus*chargeMinus[5] + Pnull*chargeNull[5];
						 
						 new_polarization = ((rho[1] + rho[3]) - (rho[0] + rho[2]))/(rho[0] + rho[1] + rho[2] + rho[3] + rho[5] + rho[6]);
						 */
						new_polarization = (Pplus - Pminus)/(Pplus + Pminus + Pnull); 
						current_cell_model->polarization = new_polarization;
						
					}
					
					else {
						
						H = (double**)malloc(size*sizeof(double*));
						for (h_iter1 = 0; h_iter1 < size; h_iter1++) {
							H[h_iter1] = (double*)malloc(size*sizeof(double));
						}
						
						H[0][0] = energyPlus;
						H[0][1] = 0;
						H[0][2] = -(options->gamma);
						H[1][0] = 0;
						H[1][1] = energyMinus;
						H[1][2] = -(options->gamma);
						H[2][0] = -(options->gamma);
						H[2][1] = -(options->gamma);
						H[2][2] = energyNull;
						
						HT = (doublereal*)malloc(size*size*sizeof(doublereal));
						DUMMY = (doublereal*)malloc(sizeof(doublereal));
						VR = (doublereal*)malloc(size*size*sizeof(doublereal));
						br = (doublereal*)malloc(size*sizeof(doublereal));
						bi = (doublereal*)malloc(size*sizeof(doublereal));
						WORK = (doublereal*)malloc(4*size*sizeof(doublereal));	
						
						
						for (h_iter1=0; h_iter1<size; h_iter1++)          /* to call a Fortran routine from C we */
						{													/* have to transform the matrix */
							for(h_iter2=0; h_iter2<size; h_iter2++)
							{
								HT[h_iter2+size*h_iter1]=H[h_iter2][h_iter1];
							}
						}
						
						c1=size;                        /* and put all numbers and characters */
						c2=4*size;                      /* we want to pass */
						c3=1;                           /* to the routine in variables */
						c4='N';
						c5='V';
						
						
						dgeev_(&c4, &c5,&c1, HT, &c1, br, bi, DUMMY, &c3, VR, &c1, WORK, &c2, &ok);
						
						A = (double**)malloc(sizeof(double*));
						for (h_iter1 = 0; h_iter1 < 1; h_iter1++) {
							A[h_iter1] = (double*)malloc(size*sizeof(double));
						} //end allocating memory for ground state wavefunction (h_iter1)
						
						if (ok==0) {
							cmp = find_min(br, size);
							
							for (h_iter1 = 0; h_iter1 < size; h_iter1++) {
								A[0][h_iter1] = VR[size*cmp+h_iter1];
							} //end getting ground state wavefunction (h_iter1)
						} //if ok
						else {
							STOP_SIMULATION = TRUE;
							fb2 = number_of_cells_in_layer[fb1];
							command_history_message ("Singularity in Hamiltonian. Simulation aborted. Try smaller dx, dy, dz values.\n") ;
						}
						
						new_polarization = -expectation(A,generator,c3,size,size,size);
						current_cell_model->polarization = new_polarization;
						
						current_cell_model->wavefunction[0] = (A[0][0])*(A[0][0]);
						current_cell_model->wavefunction[1] = (A[0][1])*(A[0][1]);
						current_cell_model->wavefunction[2] = (A[0][2])*(A[0][2]);
						
						qcad_cell_set_polarization(sorted_cells[fb1][fb2], new_polarization);
												
						if (!s_flag) {
							stable = (fabs (new_polarization - old_polarization) <= 1e-3) ;
							if (!stable) {
								s_flag = 1;
							}
						}
						
						if (iter == 1000) {
							STOP_SIMULATION = TRUE;
							command_history_message ("Simulation could not find a stable solution. Try increasing cell-cell spacing.\n") ;
							s_flag = 0;
							break;
						}
						
						free(HT); HT = NULL;
						free(DUMMY); DUMMY = NULL;
						free(VR); VR = NULL;
						free(br); br = NULL;
						free(bi); bi = NULL;
						free(WORK); WORK = NULL;
						
						free(A[0]); A[0] = NULL;
						free(A); A = NULL;
						
						for (h_iter1 = 0; h_iter1 < size; h_iter1++) {
							free(H[h_iter1]); H[h_iter1] = NULL;
						}
						free(H); H = NULL;
					}
				}
		}
		//printf("%e\n", calculate_clock_value_cc (sorted_cells[0][10]->cell_dots[0].x, t, options, pvt));
		// -- collect all the output data from the simulation -- //
		if (0 == j % record_interval) {
			for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
				sim_data->trace[total_number_of_inputs + i].data[j/record_interval] =
				qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell) ;
			/*
			for (fb1 = 0; fb1 < number_of_cell_layers; fb1++) {
				for (fb2 = 0; fb2 < number_of_cells_in_layer[fb1]; fb2++) {
					cell = sorted_cells[fb1][fb2] ;
					current_cell_model = ((ts_fc_model *)cell->cell_model) ;
					if (QCAD_CELL_OUTPUT == cell->cell_function) {
						printf("%f\n",current_cell_model->polarization);
					}
				}
			}
			 */
		}
		
		if (TRUE == STOP_SIMULATION) return sim_data;
		
	}//for number of samples
	
	// Free the neigbours and Ek array introduced by this simulation//
	for (k = 0; k < number_of_cell_layers; k++)
		for (l = 0; l < number_of_cells_in_layer[k]; l++)
		{
			g_free (((ts_fc_model *)sorted_cells[k][l]->cell_model)->neighbours);
			g_free (((ts_fc_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
			g_free (((ts_fc_model *)sorted_cells[k][l]->cell_model)->potentials_plus);
			g_free (((ts_fc_model *)sorted_cells[k][l]->cell_model)->potentials_minus);
			g_free (((ts_fc_model *)sorted_cells[k][l]->cell_model)->potentials_null);
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
	
	command_history_message (_("Total simulation time: %d s\n"), (end_time - start_time));
	set_progress_bar_visible (FALSE) ;
	return sim_data;
}//run_ts_fc

//-------------------------------------------------------------------//
// Refreshes the interaction energies between cells in the design
// This function is specific to the six dot cells
static void ts_fc_refresh_all_potentials (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, ts_fc_OP *options)
{
	int icNeighbours = 0 ;
	ts_fc_model *cell_model = NULL ;
	int i,j,k;
	
	for(i = 0 ; i < number_of_cell_layers ; i++)
		for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
		{
			// determine the x,y coordinates of the 2 extra dots required by this model
			// the z-coordinate is set by the user and available in the options structure
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x = sorted_cells[i][j]->cell_dots[0].x; 
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y = (sorted_cells[i][j]->cell_dots[1].y+sorted_cells[i][j]->cell_dots[0].y)/2.0;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x = sorted_cells[i][j]->cell_dots[3].x; 
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y = (sorted_cells[i][j]->cell_dots[2].y+sorted_cells[i][j]->cell_dots[3].y)/2.0;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[2].x = ((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x; 
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[2].y = ((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[3].x = ((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x;
			((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[3].y = ((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y;
		}
	
	// calculate the potentials for each cell //
	for(i = 0 ; i < number_of_cell_layers ; i++)
		for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
		{
			// free up memory from previous simulations //
			g_free ((cell_model = (ts_fc_model *)sorted_cells[i][j]->cell_model)->neighbours);
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
			select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((ts_fc_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
								   ((ts_fc_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));
			
			//printf("cell dot[0] is located at x=%e y=%e, extra dot x=%e y=%e and x=%e y=%e\n", sorted_cells[i][j]->cell_dots[0].x,sorted_cells[i][j]->cell_dots[0].y, ((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].x,((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[0].y,((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].x,((ts_fc_model *)sorted_cells[i][j]->cell_model)->extra_dots[1].y);
			
			
			// for each of the cell neighbours
			if (icNeighbours > 0)
			{
				cell_model->potentials_plus  = g_malloc0 (sizeof (double *) * icNeighbours);
				cell_model->potentials_minus = g_malloc0 (sizeof (double *) * icNeighbours);
				cell_model->potentials_null  = g_malloc0 (sizeof (double *) * icNeighbours);
				
				// ensure no memory allocation error has ocurred //
				if (((ts_fc_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
					((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_plus == NULL ||
					((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_minus == NULL ||
					((ts_fc_model *)sorted_cells[i][j]->cell_model)->potentials_null == NULL)
					exit (1);
				
				for (k = 0; k < icNeighbours; k++){
					//allocate memory for the potential at each dot in the six dot cell
					cell_model->potentials_plus[k]  = g_malloc0 (sizeof (double) * 6);
					cell_model->potentials_minus[k] = g_malloc0 (sizeof (double) * 6);
					cell_model->potentials_null[k]  = g_malloc0 (sizeof (double) * 6);
				}
				
				for (k = 0; k < icNeighbours; k++){
					
					// set the potentials of this cell and its neighbour //
					ts_fc_determine_potentials (sorted_cells[i][j], cell_model->neighbours[k], k, ABS(i-cell_model->neighbour_layer[k]), options);
				}
				
			}
		}
}//refresh_all_potentials


//-------------------------------------------------------------------//
// calculates the self energy of the cell for each of the three states
// i.e. the interaction energy of the electrons and neutralizing charges within the cell
void ts_fc_calculate_self_energies(QCADCell *cell, ts_fc_OP *options){
	
	int i,j;
	double distance;
	double Constant = 1 / (4 * PI * EPSILON * options->epsilonR * 1e-9);
	// these variables apply only to the six dot cells!!
	
	double chargePlus[8]  = { 0,  - QCHARGE,  0,  - QCHARGE, 0, 0, QCHARGE, QCHARGE};
	double chargeMinus[8] = {- QCHARGE,  0,   - QCHARGE,  0, 0, 0, QCHARGE, QCHARGE};
	double chargeNull[8]  = {0,  0, 0,  0, - QCHARGE, - QCHARGE, QCHARGE, QCHARGE};
	
	 
	g_assert(cell != NULL);
	
	((ts_fc_model *)cell->cell_model)->self_energy_plus	= 0;
	((ts_fc_model *)cell->cell_model)->self_energy_minus = 0;
	((ts_fc_model *)cell->cell_model)->self_energy_null	= 0;
	
	for(i = 0; i < 7; i++)
		for(j = i+1; j < 8; j++){
			distance = ts_fc_determine_distance (cell, cell, i, j, 0.0, 0.0, options->cell_height, options->counter_ion);
			((ts_fc_model *)cell->cell_model)->self_energy_plus	+= (chargePlus[i] * chargePlus[j]) / distance;
			((ts_fc_model *)cell->cell_model)->self_energy_minus += (chargeMinus[i] * chargeMinus[j]) / distance;
			((ts_fc_model *)cell->cell_model)->self_energy_null	+= (chargeNull[i] * chargeNull[j]) / distance;
			
		}
	
	((ts_fc_model *)cell->cell_model)->self_energy_plus	*= Constant;
	((ts_fc_model *)cell->cell_model)->self_energy_minus *= Constant;
	((ts_fc_model *)cell->cell_model)->self_energy_null	*= Constant;
	
	//printf("Self Energies +=%e -=%e Null=%e\n", ((ts_fc_model *)cell->cell_model)->self_energy_plus, ((ts_fc_model *)cell->cell_model)->self_energy_minus, ((ts_fc_model *)cell->cell_model)->self_energy_null);
}


//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
// this function applies only to the six dot cells!!
void ts_fc_determine_potentials (QCADCell * cell1, QCADCell * cell2, int neighbour, int layer_separation, ts_fc_OP *options)
{
	int k;
	int j;
	
	double distance = 0;
	//1e-9 in following is to convert from nm to meters in the final result
	double Constant = 1 / (4 * PI * EPSILON * options->epsilonR * 1e-9);
		
	// these variables apply only to the six dot cells!!
	
	double chargePlus[8]  = { 0,  - QCHARGE,  0,  - QCHARGE, 0, 0, QCHARGE, QCHARGE};
	double chargeMinus[8] = {- QCHARGE,  0,   - QCHARGE,  0, 0, 0, QCHARGE, QCHARGE};
	double chargeNull[8]  = {0,  0, 0,  0, - QCHARGE, - QCHARGE, QCHARGE, QCHARGE};
	
	g_assert (cell1 != NULL);
	g_assert (cell2 != NULL);
	g_assert (cell1 != cell2);
	
	// potential_{plus,minus,null}[(int) which neighbour][(int) which dot in my cell]
	//the potential energy of the three basis states of neigbouring cells cast onto the six dots of this cell
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][0]  = 
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][1]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][2]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][3]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][4]	=
	((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][5]  = 0;
	
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][0]  = 
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][1]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][2]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][3]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][4]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][5]  = 0;
	
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][0]  = 
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][1]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][2]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][3]	= 
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][4]	=
	((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][5]  = 0;
	
	for (k = 0; k < 6; k++){
		for (j = 0; j < 8; j++){
			
			// determine the distance between the dots //
			// printf("layer seperation = %d\n", layer_seperation);
			distance = ts_fc_determine_distance (cell1, cell2, k, j, 0.0, (double)layer_separation * ((ts_fc_OP *)options)->layer_separation, options->cell_height, options->counter_ion);
			g_assert (distance > 0);
			//printf("distance = %e\n",distance);
			((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][k]		+= chargePlus[j] / distance;
			((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][k]		+= chargeMinus[j] / distance;
			((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][k]		+= chargeNull[j] / distance;
		}//for other dots
		
		((ts_fc_model *)cell1->cell_model)->potentials_plus[neighbour][k]		*= Constant;
		((ts_fc_model *)cell1->cell_model)->potentials_minus[neighbour][k]		*= Constant;
		((ts_fc_model *)cell1->cell_model)->potentials_null[neighbour][k]		*= Constant;
	}
	/*
	printf("PLUS %e %e %e %e %e %e\n"
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][0]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][1]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][2]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][3]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][4]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_plus[0][5]
	 );
	 
	 printf("MINUS %e %e %e %e %e %e\n"
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][0]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][1]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][2]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][3]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][4]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_minus[0][5]
	 );
	 
	 printf("NULL %e %e %e %e %e %e\n"
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][0]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][1]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][2]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][3]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][4]
	 , ((ts_fc_model *)cell1->cell_model)->potentials_null[0][5]
	 );
	 */	
	 
}// ts_fc_determine_Ek



//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const ts_fc_OP *options, int SIMULATION_TYPE, VectorTable *pvt)
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
double ts_fc_determine_distance(QCADCell *cell1, QCADCell *cell2, int dot_cell_1, int dot_cell_2, double z1, double z2, double cell_height, double counter_ion)
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
	}
	else {
		if(dot_cell_1 < 4 && dot_cell_2 >= 4){
			x = cell1->cell_dots[dot_cell_1].x - ((ts_fc_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].x;
			y = cell1->cell_dots[dot_cell_1].y - ((ts_fc_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].y;
			if (dot_cell_2 <= 5) {
				z = z1 - (z2-cell_height);
			}
			else {
				z = z1 - (z2-cell_height-counter_ion);
			}
		}
		else {
			if(dot_cell_1 >= 4 && dot_cell_2 < 4){ 
				x = ((ts_fc_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].x - cell2->cell_dots[dot_cell_2].x;
				y = ((ts_fc_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].y - cell2->cell_dots[dot_cell_2].y;
				z = (z1-cell_height) - z2;
			}	
			else { 
				if(dot_cell_1 >= 4 && dot_cell_2 >= 4){ 
					x = ((ts_fc_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].x - ((ts_fc_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].x;
					y = ((ts_fc_model *)cell1->cell_model)->extra_dots[dot_cell_1 - 4].y - ((ts_fc_model *)cell2->cell_model)->extra_dots[dot_cell_2 - 4].y;
					if (dot_cell_2 <= 5) {
						z = z1-z2;
					}
					else {
						z = z1 - (z2-counter_ion);
					}
				}
				else {
					printf("i dont know what to do...\n");
				}
			}
		}
	}
	
	//printf("x = %e y = %e z = %e\n",x,y,z);
	//printf("dist = %e\n",sqrt (x * x + y * y + z * z));
	return sqrt (x * x + y * y + z * z);
}//determine_distance


static int compareCoherenceQCells (const void *p1, const void *p2)
{
	return
    ((ts_fc_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours >
    ((ts_fc_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((ts_fc_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours <
    ((ts_fc_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
}//compareSortStructs

/*
double *compare_arr(double *A, double *B, int length)
{
	int i;
	double *Out = NULL;
	
	Out = (double*)malloc(length*sizeof(double));
	
	for (i = 0; i < length; i++) {
		Out[i] = A[i] - B[i];
	}
	return Out;
}

static inline int search_array(double *A, double cmp, int num_elements )
{
	
	int i;
	for (i = 0; i<num_elements; i++) {
		if (fabs(A[i]) > cmp) {
			return i;
		}
	}
	return -1;	
}
*/
 
static inline void array_copy(double *Arr1, double *Arr0, int length)
{
	
	int i = 0;
	
	for(i = 0; i < length; i++) {
			Arr0[i] = Arr1[i];
		}
	
}


static inline double find_min(double *array, int num_elements) {
	
	int i;
	double min = 32000;
	int ind = 0;
	
	for (i = 0; i < num_elements; i++) {
        if (array[i] < min) {
			ind = i;
			min = array[i];
        }
	}
	
	return ind;
}

static inline double expectation (double **psi, double **Mat, int dim1x, int dim1y, int dim2x, int dim2y)
{
	
	int i;
	double **Mat1 = NULL;
	double **transMat = NULL;
	double **Mat2 = NULL;
	double output;
	
	Mat1 = matrix_mult(psi, Mat, dim1x, dim1y, dim2x, dim2y);
	transMat = transpose(psi, dim1x, dim1y);
	Mat2 = matrix_mult(Mat1, transMat, dim1x, dim2y, dim1y, dim1x);
	
	output = Mat2[0][0];
	
	for (i = 0; i < dim1x; i++) {
		free(Mat1[i]);
	}
	
	for (i = 0; i < dim1y; i++) {
		free(transMat[i]);
	}
	
	free(Mat2[0]);
	
	free(Mat1);
	free(Mat2);
	free(transMat);
	
	return output;
}

static inline double** matrix_mult(double **Mat1, double **Mat2, int dim1x, int dim1y, int dim2x, int dim2y)
{
	
	int m = 0;
	double **Mat0;
	Mat0 = (double**)malloc(dim1x*sizeof(double*));
	for (m = 0; m < dim1x; m++) {
        Mat0[m] = (double*)malloc(dim2y*sizeof(double));
	}
	
	int i = 0;
	int j = 0;
	int k = 0;
	for(i = 0; i < dim1x; i++)
		for( j = 0; j < dim2y; j++) {
			Mat0[i][j] = 0;
			for( k = 0; k < dim1y; k++) {
				Mat0[i][j] +=  Mat1[i][k] * Mat2[k][j];
			}		
		}
	
	return Mat0;
}

static inline double** transpose (double **Mat_in, int dimX, int dimY)
{
	int j = 0;
	int k = 0;
	
	int m = 0;
	double **Mat_out;
	Mat_out = (double**)malloc(dimY*sizeof(double*));
	for (m = 0; m < dimY; m++) {
        Mat_out[m] = (double*)malloc(dimX*sizeof(double));
	}
	
	
	for (j = 0; j < dimY; j++)
        for (k = 0; k < dimX; k++)
        {
			Mat_out[j][k] = Mat_in[k][j];
        }
	
    return Mat_out;
}//transpose

static inline void array_mult_by_const(double *arr1, double constant, int length, double *arr0)
{
	
	int i = 0;
	for(i = 0; i < length; i++) {
			arr0[i] =  constant*arr1[i];
		}		
}

static inline void array_add(double *arr1, double *arr2, int length, double *arr0)
{
	
	int i = 0;
	for(i = 0; i < length; i++) {
			arr0[i] =  arr1[i] + arr2[i];
		}		
}


static inline double interp(double *Grid, double x1, double y1, double z1, double x2, double y2, double z2, int Nx, int Ny, int Nz, double dx, double dy, double dz)
{	
	
	double i1, i2, j1, j2, w1, w2;
	int loc1, loc2;
	double pot1, pot2;
	double over_dx = 1/dx;
	double over_dy = 1/dy;
	double over_dz = 1/dy;
	
	loc1 = floor(x1)*Ny*Nz + floor(y1)*Nz + floor(z1);
	loc2 = floor(x1)*Ny*Nz + floor(y1)*Nz + ceil(z1);
	
	pot1 = Grid[loc1];
	pot2 = Grid[loc2];
	
	i1 = (pot1*(dz-z2) + pot2*z2)*over_dz;
	
	loc1 = floor(x1)*Ny*Nz + ceil(y1)*Nz + floor(z1);
	loc2 = floor(x1)*Ny*Nz + ceil(y1)*Nz + ceil(z1);
	
	pot1 = Grid[loc1];
	pot2 = Grid[loc2];
	
	i2 = (pot1*(dz-z2) + pot2*z2)*over_dz;
	
	loc1 = ceil(x1)*Ny*Nz + floor(y1)*Nz + floor(z1);
	loc2 = ceil(x1)*Ny*Nz + floor(y1)*Nz + ceil(z1);
	
	pot1 = Grid[loc1];
	pot2 = Grid[loc2];
	
	j1 = (pot1*(dz-z2) + pot2*z2)*over_dz;
	
	loc1 = ceil(x1)*Ny*Nz + ceil(y1)*Nz + floor(z1);
	loc2 = ceil(x1)*Ny*Nz + ceil(y1)*Nz + ceil(z1);
	
	pot1 = Grid[loc1];
	pot2 = Grid[loc2];
	
	j2 = (pot1*(dz-z2) + pot2*z2)*over_dz;
	
	w1 = (i1*(dy-y2) + i2*y2)*over_dy;
	w2 = (j1*(dy-y2) + j2*y2)*over_dy;
	
	return (w1*(dx-x2) + w2*x2)*over_dx;	
	
}

static inline double calculate_clock_value_cc (double x, double y, double t, const ts_fc_OP *options, VectorTable *pvt) //Added by Faizal for cont. clocking
{
	double clock = 0;
	
	clock = options->Emax * sin (2*PI*(x * optimization_options.over_lambda_x + y * optimization_options.over_lambda_y - ((double)pvt->vectors->icUsed) * optimization_options.over_duration * t)) + options->clock_shift;
	
    
	return clock;
}// calculate_clock_value_cc

void ts_fc_options_dump (ts_fc_OP *ts_fc_options, FILE *pfile)
{
	fprintf (stderr, "ts_fc_options_dump:\n") ;
	fprintf (stderr, "ts_fc_options->time_step                 = %e [s]\n",  ts_fc_options->time_step) ;
	fprintf (stderr, "ts_fc_options->duration                  = %e [s]\n",  ts_fc_options->duration) ;
	fprintf (stderr, "ts_fc_options->gamma					   = %e [J]\n",  ts_fc_options->gamma) ;
	fprintf (stderr, "ts_fc_options->clock_high                = %e [J]\n",  ts_fc_options->clock_high) ;
	fprintf (stderr, "ts_fc_options->clock_low                 = %e [J]\n",  ts_fc_options->clock_low) ;
	fprintf (stderr, "ts_fc_options->clock_shift               = %e [J]\n",  ts_fc_options->clock_shift) ;
	fprintf (stderr, "ts_fc_options->Emax					   = %e\n",      ts_fc_options->Emax) ;
	fprintf (stderr, "ts_fc_options->radius_of_effect          = %e [nm]\n", ts_fc_options->radius_of_effect) ;
	fprintf (stderr, "ts_fc_options->epsilonR                  = %e\n",      ts_fc_options->epsilonR) ;
	fprintf (stderr, "ts_fc_options->layer_separation          = %e [nm]\n", ts_fc_options->layer_separation) ;
	fprintf (stderr, "ts_fc_options->counter_ion               = %e\n",      ts_fc_options->counter_ion) ;
	fprintf (stderr, "ts_fc_options->dx						   = %f\n",      ts_fc_options->dx) ;
	fprintf (stderr, "ts_fc_options->dy						   = %f\n",      ts_fc_options->dy) ;
	fprintf (stderr, "ts_fc_options->dz						   = %f\n",      ts_fc_options->dz) ;
	fprintf (stderr, "ts_fc_options->lambda_x				   = %f\n",      ts_fc_options->lambda_x) ;
	fprintf (stderr, "ts_fc_options->lambda_y				   = %f\n",      ts_fc_options->lambda_y) ;
	fprintf (stderr, "ts_fc_options->algorithm                 = %d\n",      ts_fc_options->algorithm) ;
	fprintf (stderr, "ts_fc_options->temp_model				   = %s\n",      ts_fc_options->temp_model ? "TRUE" : "FALSE") ;
	fprintf (stderr, "ts_fc_options->randomize_cells           = %s\n",      ts_fc_options->randomize_cells ? "TRUE" : "FALSE") ;
	fprintf (stderr, "ts_fc_options->animate_simulation        = %s\n",      ts_fc_options->animate_simulation ? "TRUE" : "FALSE") ;
}

#endif /* HAVE_FORTRAN */
