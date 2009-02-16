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
// The bistable simulation engine. This engine treats   //
// the circuit in a time-independent fashion, as a      //
// system capable of 2 basis states.                    //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../CLAPACK/SRC/blaswrap.h"
#include "../CLAPACK/SRC/f2c.h"
#include "../CLAPACK/SRC/clapack.h"
#include <glib-object.h>

#ifdef GTK_GUI
#include <gtk/gtk.h>
#include "callback_helpers.h"
#endif /* def GTK_GUI */

#include "intl.h"
#include "objects/QCADCell.h"
#include "simulation.h"
#include "semi_coherent.h"
#include "custom_widgets.h"
#include "global_consts.h"

//!Options for the semi_coherent simulation engine
//This variable is used by multiple source files
// Added by Marco March 3 : last four arguments (phase shifts)

semi_coherent_OP semi_coherent_options = {12800, FALSE, 1e-3, 200, 1, 9.43e-19, 1.41e-20, 0.0, 2.0, 100, 1.15, 3, 0,0,0,0, TRUE, FALSE, TRUE} ;


#ifdef GTK_GUI
extern int STOP_SIMULATION;
#else
static int STOP_SIMULATION = 0 ;
#endif /* def GTK_GUI */

// To each cell this structure is connected in order that this particular simulation engine can have its own variables. //
typedef struct
	{
		int number_of_neighbours;
		QCADCell **neighbours;
		int *neighbour_layer;
		double *Ek;
		double polarization;
	} semi_coherent_model;

// Allows us to create matrices on the fly
struct cluster {
	double **Mat;
	struct cluster *next;
};

#define size 2
GdkColor clrGroup = {0, 42405,10794,10794} ;
GdkColor clrYellowBU = {0, 0xFFFF, 0xFFFF, 0x0000} ;
static GdkColor clrClockBU[4] =
{
{0, 0x0000, 0xFFFF, 0x0000},
{0, 0xFFFF, 0x0000, 0xFFFF},
{0, 0x0000, 0xFFFF, 0xFFFF},
{0, 0xFFFF, 0xFFFF, 0xFFFF},
} ;

static inline double semi_coherent_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, semi_coherent_OP *options);
static inline void semi_coherent_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, semi_coherent_OP *options);
static inline double** transpose (double **Mat_in, int dimX, int dimY);
static inline double** matrix_mult(double **Mat1, double **Mat2, int dim1x, int dim1y, int dim2x, int dim2y);
static inline void matrix_mult_by_const(double **Mat1, double constant, int dimx, int dimy, double **Mat0);
static inline void matrix_add(double **Mat1, double **Mat2, int dimx, int dimy, double **Mat0);
static inline void matrix_copy(double **Mat1, int dimx, int dimy, double **Mat0);
static inline void kron (double **MatA, double **MatB, int dimAx, int dimAy, int dimBx, int dimBy, double **Mat0);
static inline double get_max(double *array, int num_elements);
static inline double find_min(double *array, int num_elements);
static inline int search_array(int *A, int cmp, int num_elements );
static inline int search_matrix_row(int **A, int cmp, int row, int num_elements);
static inline int search_matrix_col(int **A, int cmp, int col, int num_elements);
static inline struct cluster* search_linked_list(struct cluster *head, int element);
static inline void free_linked_list(struct cluster *head, int **Hg, int num_elements, int option);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_semi_coherent_simulation (int SIMULATION_TYPE, DESIGN *design, semi_coherent_OP *options, VectorTable *pvt)
{
#ifdef GTK_GUI
	GdkColormap *clrmap = gdk_colormap_get_system () ;
#endif /* def GTK_GUI */
	
#ifdef GTK_GUI
	if (0 == clrGroup.pixel)
		gdk_colormap_alloc_color (clrmap, &clrGroup, FALSE, TRUE) ;
	if (0 == clrYellowBU.pixel)
		gdk_colormap_alloc_color (clrmap, &clrYellowBU, FALSE, TRUE) ;
	if (0 == clrClockBU[0].pixel)
		gdk_colormap_alloc_color (clrmap, &clrClockBU[0], FALSE, TRUE) ;
	if (0 == clrClockBU[1].pixel)
		gdk_colormap_alloc_color (clrmap, &clrClockBU[1], FALSE, TRUE) ;
	if (0 == clrClockBU[2].pixel)
		gdk_colormap_alloc_color (clrmap, &clrClockBU[2], FALSE, TRUE) ;
	if (0 == clrClockBU[3].pixel)
		gdk_colormap_alloc_color (clrmap, &clrClockBU[3], FALSE, TRUE) ;
#endif /* def GTK_GUI */
	
	int i, j, k, l, total_cells = 0 ;
	int icLayers, icCellsInLayer;
	time_t start_time, end_time;
	simulation_data *sim_data = NULL ;
	// optimization variables //
	int number_of_cell_layers = 0, *number_of_cells_in_layer = NULL ;
	QCADCell ***sorted_cells = NULL ;
	double clock_shift = (options->clock_high + options->clock_low)/2 + options->clock_shift;
	double clock_prefactor = (options->clock_high - options->clock_low) * options->clock_amplitude_factor;
	double four_pi_over_number_samples = 4.0 * PI / (double) options->number_of_samples;
	double two_pi_over_number_samples = 2.0 * PI / (double) options->number_of_samples;
	int idxMasterBitOrder = -1 ;
	int max_iterations_per_sample = ((semi_coherent_OP *)options)->max_iterations_per_sample;
	BUS_LAYOUT_ITER bli ;
	double thresh_den = options->threshold;
	// For randomization
	int Nix, Nix1;
	int fb1, fb2;
	int init_size, init_size_x, init_size_p;
	int cmp;
	int h_row, h_col;
	int num_inputs = 0; 
	int num_fixed_cells = 0;
	int num_normal = 0;
	// -- these used to be inside run_semi_coherent_iteration -- //
	int q, iteration = 0;
	int stable = FALSE;
	double old_polarization;
	double new_polarization;
	double tolerance = ((semi_coherent_OP *)options)->convergence_tolerance;
	double polarization_math;
	double EkP;
	double Ek0;
	double Ek_sum;
	double thresh;
	double dim;
	double num_mats, num_mats_prev;
	double **H = NULL;
	double **A = NULL;
	double **pauli_z = NULL;
	double **pauli_x = NULL;
	double **MatOut1 = NULL;
	double **MatOut2 = NULL;
	double **transA = NULL;
	double **H_init = NULL;
	double **H_init_x = NULL;
	double **H_init_p = NULL;
	double **H_init_x_temp = NULL;
	double **H_init_p_temp = NULL;
	double **Hz = NULL;
	double **Hx = NULL;
	double **Hp = NULL;
	double **H_group = NULL;
	double **Eye = NULL;
	int *H_group_cell = NULL;
	GdkColor *H_group_color = NULL;
	int **Hg = NULL;
	int *Hg_used = NULL;
	doublereal *HT = NULL;
	doublereal *DUMMY = NULL;
	doublereal *VR = NULL;
	doublereal *br = NULL;
	doublereal *bi = NULL;
	doublereal *WORK = NULL;
	int h_iter1, h_iter2, k_iter1, k_iter2, k_iter, row_iter, col_iter; 
	integer ok, c1, c2, c3;
	char c4, c5;
	
	pauli_x = (double**)malloc(size*sizeof(double*));
	pauli_z = (double**)malloc(size*sizeof(double*));
	Eye = (double**)malloc(size*sizeof(double*));
	for (i = 0; i < size; i++) {
		pauli_x[i] = (double*)malloc(size*sizeof(double));
		pauli_z[i] = (double*)malloc(size*sizeof(double));
		Eye[i] = (double*)malloc(size*sizeof(double));
	}
	
	pauli_x[0][0] = 0;
	pauli_x[0][1] = 1;
	pauli_x[1][0] = 1;
	pauli_x[1][1] = 0;
	
	pauli_z[0][0] = -1;
	pauli_z[0][1] = 0;
	pauli_z[1][0] = 0;
	pauli_z[1][1] = 1;
	
	Eye[0][0] = 1;
	Eye[0][1] = 0;
	Eye[1][0] = 0;
	Eye[1][1] = 1;
	
	semi_coherent_model *current_cell_model = NULL ;
	QCADCell *cell;
	int jitter_phases[4] = {options->jitter_phase_0, options->jitter_phase_1, options->jitter_phase_2, options->jitter_phase_3} ;
	
	STOP_SIMULATION = FALSE;
	
	// -- get the starting time for the simulation -- //
	if((start_time = time (NULL)) < 0)
		fprintf(stderr, "Could not get start time\n");
	
	// Create per-layer cell arrays to be used by the engine
	simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;
	
	for(i = 0; i < number_of_cell_layers; i++)
    {
		for(j = 0; j < number_of_cells_in_layer[i] ; j++)
		{
			// attach the model parameters to each of the simulation cells //
			current_cell_model = g_malloc0 (sizeof(semi_coherent_model)) ;
			sorted_cells[i][j]->cell_model = current_cell_model;
			
			// -- Clear the model pointers so they are not dangling -- //
			current_cell_model->neighbours = NULL;
			current_cell_model->Ek = NULL;
			
			// -- set polarization in cell model for fixed cells since they are set with actual dot charges by the user -- //
			if(QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function) {
				current_cell_model->polarization = qcad_cell_calculate_polarization(sorted_cells[i][j]); 
				num_fixed_cells++;
			}
			if (QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) {
				num_inputs++;
			}
			
			total_cells++;
		}
    }
	
	num_normal = total_cells-num_fixed_cells-num_inputs;
	// if we are performing a vector table simulation we consider only the activated inputs //
	if(SIMULATION_TYPE == VECTOR_TABLE)
	{
		for (Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
			if (!exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag)
				exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input->cell_function = QCAD_CELL_NORMAL ;
	}
	
	// write message to the command history window //
	command_history_message (_("Simulation found %d inputs %d outputs %d total cells\n"), design->bus_layout->inputs->icUsed, design->bus_layout->outputs->icUsed, total_cells) ;
	
	command_history_message(_("Starting initialization\n"));
	set_progress_bar_visible (TRUE) ;
	set_progress_bar_label (_("semi_coherent simulation:")) ;
	
	// -- Initialize the simualtion data structure -- //
	sim_data = g_malloc0 (sizeof(simulation_data));
	sim_data->number_of_traces = design->bus_layout->inputs->icUsed + design->bus_layout->outputs->icUsed;
	sim_data->number_samples = options->number_of_samples;
	sim_data->trace = g_malloc0 (sizeof (struct TRACEDATA) * sim_data->number_of_traces);
	
	// create and initialize the inputs into the sim data structure //
	for (i = 0; i < design->bus_layout->inputs->icUsed; i++)
    {
		sim_data->trace[i].data_labels = g_strdup (qcad_cell_get_label (QCAD_CELL (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell))) ;
		sim_data->trace[i].drawtrace = TRUE;
		sim_data->trace[i].trace_function = QCAD_CELL_INPUT;
		sim_data->trace[i].data = g_malloc0 (sizeof (double) * sim_data->number_samples);
    }
	
	// create and initialize the outputs into the sim data structure //
	for (i = 0; i < design->bus_layout->outputs->icUsed; i++)
    {
		sim_data->trace[i + design->bus_layout->inputs->icUsed].data_labels = g_strdup (qcad_cell_get_label(QCAD_CELL(exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell))) ;
		sim_data->trace[i + design->bus_layout->inputs->icUsed].drawtrace = TRUE;
		sim_data->trace[i + design->bus_layout->inputs->icUsed].trace_function = QCAD_CELL_OUTPUT;
		sim_data->trace[i + design->bus_layout->inputs->icUsed].data = g_malloc0 (sizeof (double) * sim_data->number_samples);
    }
	
	// create and initialize the clock data //
	sim_data->clock_data = g_malloc0 (sizeof (struct TRACEDATA) * 4);
	
	for (i = 0; i < 4; i++)
    {
		sim_data->clock_data[i].data_labels = g_strdup_printf ("CLOCK %d", i);
		sim_data->clock_data[i].drawtrace = 1;
		sim_data->clock_data[i].trace_function = QCAD_CELL_FIXED; // Abusing the notation here
		
		sim_data->clock_data[i].data = g_malloc0 (sizeof (double) * sim_data->number_samples);
		
		//Added by Marco : phase shift included in (-PI/2, +P/2) with steps of (1/200)PI
		//Edited by Konrad: above is incorrect changed jitter to actual phase shift phase shift = jitter/180*PI
		if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
			for (j = 0; j < sim_data->number_samples; j++)
			{
				sim_data->clock_data[i].data[j] = clock_prefactor * cos (((double)(1 << design->bus_layout->inputs->icUsed)) * (double)j * four_pi_over_number_samples - (double)((jitter_phases[i]) / 180.0) * PI - PI * i / 2) + clock_shift ;
				sim_data->clock_data[i].data[j] = CLAMP (sim_data->clock_data[i].data[j], options->clock_low, options->clock_high) ;
			}
		else
			//if (SIMULATION_TYPE == VECTOR_TABLE)
			for (j = 0; j < sim_data->number_samples; j++)
			{
				sim_data->clock_data[i].data[j] = clock_prefactor * cos (((double)pvt->vectors->icUsed) * (double)j * two_pi_over_number_samples - (double)((jitter_phases[i]) / 180.0) * PI -  PI * i / 2) + clock_shift ;
				sim_data->clock_data[i].data[j] = CLAMP (sim_data->clock_data[i].data[j], options->clock_low, options->clock_high) ;
			}
    }
	//End added by Marco
	
	// -- refresh all the kink energies to all the cells neighbours within the radius of effect -- //
	semi_coherent_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);
	
	
	// randomize the cells in the design so as to minimize any numerical problems associated //
	// with having cells simulated in some predefined order. //
	// randomize the order in which the cells are simulated //
	//if (options->randomize_cells)
	// for each layer ...
	int num_elements = 0;
	
	H_group_cell = (int*)malloc(num_normal*sizeof(int));
	H_group_color = (GdkColor*)malloc(num_normal*sizeof(GdkColor));
	Hg_used = (int*)malloc(num_normal*sizeof(int));
	Hg = (int**)malloc(num_normal*sizeof(int*));
	for (i = 0; i < num_normal; i++) {
		Hg[i] = (int*)malloc(num_normal*sizeof(int));
	}
	
	for (h_iter1 = 0; h_iter1 < num_normal; h_iter1++) {
		H_group_cell[h_iter1] = -1;
		Hg_used[h_iter1] = -1;
		for (h_iter2 = 0; h_iter2 < num_normal; h_iter2++) {
			Hg[h_iter1][h_iter2] = -1;
		}
	}
	
	if (options->manual_group) {
		for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[0] ; Nix1++)
		{
			cell = sorted_cells[0][Nix1] ;
			
			if (!((QCAD_CELL_INPUT == cell->cell_function)||
				  (QCAD_CELL_FIXED == cell->cell_function)))
			{
				
				if (QCAD_CELL_MODE_CLUSTER == cell->cell_options.mode) {
					H_group_cell[num_elements] = Nix1;
					
					if (QCAD_CELL_NORMAL == cell->cell_function) {
						H_group_color[num_elements] = clrClockBU[cell->cell_options.clock];
					}
					else {
						H_group_color[num_elements] = clrYellowBU;
					}		
					
					if (!(options->color_group)) {
						cell->cell_options.mode = QCAD_CELL_MODE_NORMAL;
						memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &H_group_color[num_elements], sizeof (GdkColor)) ;
					}
					num_elements++;
				}
			}
		}
	}
	
	if (options->auto_group) {
		for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[0] ; Nix1++)
		{
			cell = sorted_cells[0][Nix1] ;
			
			if (!((QCAD_CELL_INPUT == cell->cell_function)||
				  (QCAD_CELL_FIXED == cell->cell_function)))
			{
				
				current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
				
				cmp = search_array(H_group_cell, Nix1, num_elements);
				if (cmp == -1) {
					
					if (QCAD_CELL_NORMAL == cell->cell_function) {
						H_group_color[num_elements] = clrClockBU[cell->cell_options.clock];
					}
					else {
						H_group_color[num_elements] = clrYellowBU;
					}
					
					
					Ek0 = get_max(current_cell_model->Ek, current_cell_model->number_of_neighbours);
					thresh = Ek0/thresh_den;
					Ek_sum = 0;       
					
					for (q = 0; q < current_cell_model->number_of_neighbours; q++) {
						Ek_sum += fabs(current_cell_model->Ek[q]);
					}
					if (Ek_sum > (2*Ek0 + thresh)) {
						H_group_cell[num_elements] = Nix1;
						qcad_cell_set_mode (cell, QCAD_CELL_MODE_CLUSTER) ;
						if (options->color_group) {
							cell->cell_options.mode = QCAD_CELL_MODE_CLUSTER;
							memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &clrGroup, sizeof (GdkColor)) ;
						}
						num_elements++;
					}
					else {
						cell->cell_options.mode = QCAD_CELL_MODE_NORMAL;
						memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &H_group_color[num_elements], sizeof (GdkColor)) ;
					}
				}
			}
		}
	}
#ifdef DESIGNER
	redraw_async(NULL);
	gdk_flush () ;
#endif /* def DESIGNER */
	
	k_iter = 0;
	cmp = 0;
	int done = 0;
	int used;
	double Ek_temp;

	if (H_group_cell[0] != -1) {
		
		for (h_iter1 = 0; h_iter1 < num_normal; h_iter1++) {
			for (h_iter2 = 0; h_iter2 < num_normal; h_iter2++) {
				if (Hg[h_iter1][h_iter2] == -1) {
					if (h_iter2 == 0) {
						while (cmp != -1) {
							cmp = search_array(Hg_used, H_group_cell[k_iter], num_elements);
							k_iter++;
							if (k_iter == num_elements) {
								done = 1;
								break;
							}
						}
					}
					else {
						break;
					}
					if (done == 1) {
						break;
					}
					cmp = search_array(Hg_used, -1, num_elements);
					Hg_used[cmp] = H_group_cell[k_iter-1];
					Hg[h_iter1][h_iter2] = H_group_cell[k_iter-1];
					cell = sorted_cells[0][H_group_cell[k_iter-1]];
					current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
					Ek0 = get_max(current_cell_model->Ek, current_cell_model->number_of_neighbours); 
					for (k_iter1 = 0; k_iter1 < num_elements; k_iter1++) {
						if (k_iter1 != (k_iter-1)) {
							cmp = search_array(Hg_used, H_group_cell[k_iter1], num_elements);
							if (cmp == -1) {
								Ek_temp = semi_coherent_determine_Ek (cell, sorted_cells[0][H_group_cell[k_iter1]], 0, options);
								if (fabs(Ek0/Ek_temp) < 5) {
									used = search_matrix_row(Hg, -1, h_iter1, num_normal);
									Hg[h_iter1][used] = H_group_cell[k_iter1];
									cmp = search_array(Hg_used, -1, num_elements);
									Hg_used[cmp] = H_group_cell[k_iter1];
								}
							}
						}
					}
				}
				else {
					cell = sorted_cells[0][Hg[h_iter1][h_iter2]];
					current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
					Ek0 = get_max(current_cell_model->Ek, current_cell_model->number_of_neighbours); 
					for (k_iter1 = 0; k_iter1 < num_elements; k_iter1++) {
						if (H_group_cell[k_iter1] != Hg[h_iter1][h_iter2]) {
							cmp = search_array(Hg_used, H_group_cell[k_iter1], num_elements);
							if (cmp == -1) {
								Ek_temp = semi_coherent_determine_Ek (cell, sorted_cells[0][H_group_cell[k_iter1]], 0, options);
								if (fabs(Ek0/Ek_temp) < 5) {
									used = search_matrix_row(Hg, -1, h_iter1, num_normal);
									Hg[h_iter1][used] = H_group_cell[k_iter1];
									cmp = search_array(Hg_used, -1, num_elements);
									Hg_used[cmp] = H_group_cell[k_iter1];							
								}
							}
						}
					}
				}
				cmp = 0;	
				k_iter = 0;
			}
			if (done == 1) {
				break;
			}
		}
	}
	

/*
	h_row = search_matrix_col(Hg,-1,0,num_normal);
	for (fb1 = 0; fb1 < h_row; fb1++) {
		h_col = search_matrix_row(Hg,-1,fb1,num_normal);
		for (fb2 = 0; fb2 < h_col; fb2++) {
			if (fb2 == 0) {
				printf("\nCell Group: %d\n", fb1+1);
				printf("-----------------\n");
			}
			printf("Hg[%d][%d] = %d\n", fb1, fb2, Hg[fb1][fb2]);
		}
	}
*/
	
	struct cluster *new;
	struct cluster *current;
	struct cluster *headz;
	struct cluster *headx;
	struct cluster *headp;
	headz = NULL;
	headx = NULL;
	headp = NULL;
	
	h_row = search_matrix_col(Hg, -1, 0, num_normal);
	
	for (row_iter = 0; row_iter < h_row; row_iter++) {
		h_col = search_matrix_row(Hg,-1,row_iter,num_normal);
		
		dim = pow(2,h_col);
		Hz = (double**)malloc(dim*sizeof(double*));
		H_init = (double**)malloc(dim*sizeof(double*));
		H_init_x = (double**)malloc(dim*sizeof(double*));
		H_init_p = (double**)malloc(dim*sizeof(double*));
		for (h_iter1 = 0; h_iter1 < dim; h_iter1++) {
			Hz[h_iter1] = (double*)malloc(dim*sizeof(double));
			H_init[h_iter1] = (double*)malloc(dim*sizeof(double));
			H_init_x[h_iter1] = (double*)malloc(dim*sizeof(double));
			H_init_p[h_iter1] = (double*)malloc(dim*sizeof(double));
		} //end allocating memory (h_iter1)
		
		for (k_iter1 = 0; k_iter1 < (int)dim; k_iter1++) {
			for (k_iter2 = 0; k_iter2 < (int)dim; k_iter2++) {
				Hz[k_iter1][k_iter2] = 0;
				H_init[k_iter1][k_iter2] = 0;
				H_init_x[k_iter1][k_iter2] = 0;
				H_init_p[k_iter1][k_iter2] = 0;
			}
		} //end filling in matrices with 0 (k_iter1, k_iter2)
		
		H_init[0][0] = 1;
		init_size = 1;
		
		H_init_x[0][0] = 1;
		init_size_x = 1;
		
		H_init_p[0][0] = 1;
		init_size_p = 1;
		
		//Pauli-z/Pauli-z Kroenekers
		for (k_iter1 = 0 ; k_iter1 < h_col-1 ; k_iter1++) {
			for (k_iter2 = k_iter1+1; k_iter2 < h_col; k_iter2++) {
				for (k_iter = 0; k_iter < h_col; k_iter++) {
					if ((k_iter1 == k_iter) || (k_iter2 == k_iter)) {
						kron(H_init,pauli_z,init_size,init_size,size,size,H_init);
						init_size = init_size*size;
					}
					else {
						kron(H_init,Eye,init_size,init_size,size,size,H_init);
						init_size = init_size*size;
					}
				} //end k_iter
				
				matrix_mult_by_const(H_init, -0.5*semi_coherent_determine_Ek (sorted_cells[0][Hg[row_iter][k_iter1]], sorted_cells[0][Hg[row_iter][k_iter2]], 0, options), init_size, init_size, H_init);	
				matrix_add(Hz, H_init, init_size, init_size, Hz);
				
				for (fb1 = 0; fb1 < dim; fb1++) {
					for (fb2 = 0; fb2 < dim; fb2++) {
						H_init[fb1][fb2] = 0;
					}
				} //end filling in matrices with 0 (fb1)
				
				H_init[0][0] = 1;
				init_size = 1;
			} //end k_iter2
		} //end k_iter1
		
		if (headz == NULL) {
			new = (struct cluster*)malloc(sizeof(struct cluster));
			new->next = headz;
			headz = new;
		}
		else {
			current = headz;
			while (current->next != NULL) {
				current = current->next;
			}
			new = (struct cluster*)malloc(sizeof(struct cluster));
			current->next = new;
			new->next = NULL;
		}
		
		new->Mat = (double**)malloc(dim*sizeof(double*));
		for (fb1 = 0; fb1 < dim; fb1++) {
			new->Mat[fb1] = (double*)malloc(dim*sizeof(double));
		}
			
		matrix_copy(Hz, (int)dim, (int)dim, new->Mat);
		//End Pauli-z/Pauli-z Kroenekers
		
		// Pauli-x/Identity & Pauli-z/Identity Kroenekers
		for (k_iter1 = 0; k_iter1 < h_col; k_iter1++) {
			cell = sorted_cells[0][Hg[row_iter][k_iter1]];
			for (k_iter2 = 0; k_iter2 < h_col; k_iter2++) {
				if (k_iter1 == k_iter2) {
					kron(H_init_x,pauli_x,init_size_x,init_size_x,size,size,H_init_x);
					init_size_x = init_size_x*size;
					kron(H_init_p,pauli_z,init_size_p,init_size_p,size,size,H_init_p);
					init_size_p = init_size_p*size;
				}
				else {
					kron(H_init_x,Eye,init_size_x,init_size_x,size,size,H_init_x);
					init_size_x = init_size_x*size;
					kron(H_init_p,Eye,init_size_p,init_size_p,size,size,H_init_p);
					init_size_p = init_size_p*size;
				}
			} //end k_iter2
			
			if (headx == NULL) {
				new = (struct cluster*)malloc(sizeof(struct cluster));
				new->next = headx;
				headx = new;
			}
			else {
				current = headx;
				while (current->next != NULL) {
					current = current->next;
				}
				new = (struct cluster*)malloc(sizeof(struct cluster));
				current->next = new;
				new->next = NULL;
			}
			
			new->Mat = (double**)malloc(dim*sizeof(double*));
			for (fb1 = 0; fb1 < dim; fb1++) {
				new->Mat[fb1] = (double*)malloc(dim*sizeof(double));
			}
			matrix_copy(H_init_x, init_size_x, init_size_x, new->Mat);
			
			if (headp == NULL) {
				new = (struct cluster*)malloc(sizeof(struct cluster));
				new->next = headp;
				headp = new;
			}
			else {
				current = headp;
				while (current->next != NULL) {
					current = current->next;
				}
				new = (struct cluster*)malloc(sizeof(struct cluster));
				current->next = new;
				new->next = NULL;
			}
			
			new->Mat = (double**)malloc(dim*sizeof(double*));
			for (fb1 = 0; fb1 < dim; fb1++) {
				new->Mat[fb1] = (double*)malloc(dim*sizeof(double));
			}
			matrix_copy(H_init_p, init_size_p, init_size_p, new->Mat);
			
			//clear H_init_x, H_init_p
			for (fb1 = 0; fb1 < dim; fb1++) {
				for (fb2 = 0; fb2 < dim; fb2++) {
					H_init_x[fb1][fb2] = 0;
					H_init_p[fb1][fb2] = 0;
				}
			} //end filling in matrices with 0 (fb1, fb2)
			
			H_init_x[0][0] = 1;
			init_size_x = 1;
			
			H_init_p[0][0] = 1;
			init_size_p = 1;
			
		} //end k_iter1	
		//End Pauli-x/Identity & Pauli-z/Identity Kroenekers
		
		for (fb1 = 0; fb1 < (int)dim; fb1++) {
			free(Hz[fb1]); Hz[fb1] = NULL;
			free(H_init[fb1]); H_init[fb1] = NULL;
			free(H_init_x[fb1]); H_init_x[fb1] = NULL;
			free(H_init_p[fb1]); H_init_p[fb1] = NULL;
		} //end free memory (fb1)
		
		free(Hz); Hz = NULL;
		free(H_init); H_init = NULL;
		free(H_init_x); H_init_x = NULL;
		free(H_init_p); H_init_p = NULL;
		
	} //end row_iter
	
	
	
	// -- get and print the total initialization time -- //
	if((end_time = time (NULL)) < 0)
		fprintf(stderr, "Could not get end time\n");
	
	command_history_message(_("Total initialization time: %g s\n"), (double)(end_time - start_time));
	
	command_history_message(_("Starting Simulation\n"));
	
	set_progress_bar_fraction (0.0) ;
	
	// perform the iterations over all samples //
	for (j = 0; j < sim_data->number_samples ; j++)
    {
		if (j % 10 == 0 || j == sim_data->number_samples - 1)
		{
			// write the completion percentage to the command history window //
			set_progress_bar_fraction ((float) j / (float)sim_data->number_samples) ;
			// redraw the design if the user wants it to appear animated or if this is the last sample //
			if(options->animate_simulation || j == sim_data->number_samples - 1)
			{
				// update the charges to reflect the polarizations so that they can be animated //
				for(icLayers = 0; icLayers < number_of_cell_layers; icLayers++)
				{
					for(icCellsInLayer = 0; icCellsInLayer < number_of_cells_in_layer[icLayers]; icCellsInLayer++)
						qcad_cell_set_polarization(sorted_cells[icLayers][icCellsInLayer],((semi_coherent_model *)sorted_cells[icLayers][icCellsInLayer]->cell_model)->polarization);
				}
#ifdef DESIGNER
				redraw_async(NULL);
				gdk_flush () ;
#endif /* def DESIGNER */
			}
		}
		
		// -- for each of the (VECTOR_TABLE => active?) inputs -- //
		if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
			for (idxMasterBitOrder = 0, design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i), idxMasterBitOrder++)
				((semi_coherent_model *)exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell->cell_model)->polarization =
				sim_data->trace[i].data[j] = (-1 * sin (((double)(1 << idxMasterBitOrder)) * (double)j * FOUR_PI / (double)sim_data->number_samples) > 0) ? 1 : -1 ;
		else
			//    if (VECTOR_TABLE == SIMULATION_TYPE)
			for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
				if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
					((semi_coherent_model *)exp_array_index_1d (pvt->inputs, VT_INPUT, i).input->cell_model)->polarization =
					sim_data->trace[i].data[j] = exp_array_index_2d (pvt->vectors, gboolean, (j * pvt->vectors->icUsed) / sim_data->number_samples, i) ? 1 : -1 ;
		
		int index = 0;
		num_mats_prev = 0;
		h_row = search_matrix_col(Hg, -1, 0, num_normal);
		
		
		for (row_iter = 0; row_iter < h_row; row_iter++) {
			h_col = search_matrix_row(Hg,-1,row_iter,num_normal);
			num_mats = h_col;
			
			dim = pow(2,h_col);
			Hx = (double**)malloc(dim*sizeof(double*));
			Hp = (double**)malloc(dim*sizeof(double*));
			H_group = (double**)malloc(dim*sizeof(double*));
			H_init_x_temp = (double**)malloc(dim*sizeof(double*));
			H_init_p_temp = (double**)malloc(dim*sizeof(double*));
			for (h_iter1 = 0; h_iter1 < dim; h_iter1++) {
				Hx[h_iter1] = (double*)malloc(dim*sizeof(double));
				Hp[h_iter1] = (double*)malloc(dim*sizeof(double));
				H_group[h_iter1] = (double*)malloc(dim*sizeof(double));
				H_init_x_temp[h_iter1] = (double*)malloc(dim*sizeof(double));
				H_init_p_temp[h_iter1] = (double*)malloc(dim*sizeof(double));
			} //end allocating memory (h_iter1)
			
			for (k_iter1 = 0; k_iter1 < (int)dim; k_iter1++) {
				for (k_iter2 = 0; k_iter2 < (int)dim; k_iter2++) {
					Hx[k_iter1][k_iter2] = 0;
					Hp[k_iter1][k_iter2] = 0;
					H_group[k_iter1][k_iter2] = 0;
				}
			} //end filling in matrices with 0 (k_iter1, k_iter2)
			
			index = index+num_mats_prev;
			for (col_iter = 0; col_iter < num_mats; col_iter++) {
				//multiply pauli-x by gamma
				cell = sorted_cells[0][Hg[row_iter][col_iter]];
				new = search_linked_list(headx, index+col_iter);
				matrix_copy(new->Mat, (int)dim, (int)dim, H_init_x_temp);
				matrix_mult_by_const(H_init_x_temp, -(sim_data->clock_data[cell->cell_options.clock].data[j]), (int)dim, (int)dim, H_init_x_temp);
				matrix_add(Hx, H_init_x_temp, (int)dim, (int)dim, Hx);
				//multiply pauli-z by EkP
				new = search_linked_list(headp, index+col_iter);
				matrix_copy(new->Mat, (int)dim, (int)dim, H_init_p_temp);
				for (fb1 = 0 ; fb1 < number_of_cells_in_layer[0] ; fb1++)
				{
					cell = sorted_cells[0][fb1] ;
					current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
					cmp = search_matrix_row(Hg,fb1, row_iter, num_normal);
					if (cmp == -1) {
						matrix_mult_by_const(H_init_p_temp, 0.5*semi_coherent_determine_Ek (sorted_cells[0][Hg[row_iter][col_iter]], sorted_cells[0][fb1], 0, options), (int)dim, (int)dim, H_init_p_temp);					
						matrix_mult_by_const(H_init_p_temp, current_cell_model->polarization,(int)dim, (int)dim, H_init_p_temp);
						matrix_add(Hp,H_init_p_temp, (int)dim, (int)dim, Hp);
						matrix_copy(new->Mat, (int)dim, (int)dim, H_init_p_temp);
					} 
				} //end getting Ek/Pols for all cells (fb1)
			} //end col_iter
				
			new = search_linked_list(headz,row_iter);
			matrix_add(Hx, new->Mat, dim, dim, H_group);
			matrix_add(H_group, Hp, dim, dim, H_group);
			
			iteration = 0;
			stable = FALSE;
			while (!stable && iteration < max_iterations_per_sample)
			{
				iteration++;
				// -- assume that the circuit is stable -- //
				stable = TRUE;
				
				HT = (doublereal*)malloc(dim*dim*sizeof(doublereal));
				DUMMY = (doublereal*)malloc(sizeof(doublereal));
				VR = (doublereal*)malloc(dim*dim*sizeof(doublereal));
				br = (doublereal*)malloc(dim*sizeof(doublereal));
				bi = (doublereal*)malloc(dim*sizeof(doublereal));
				WORK = (doublereal*)malloc(4*dim*sizeof(doublereal));
				
				for (h_iter1=0; h_iter1<dim; h_iter1++)         
				{				                 
					for(h_iter2=0; h_iter2<dim; h_iter2++)
					{
						HT[h_iter2+(int)dim*h_iter1]=H_group[h_iter2][h_iter1];
					}
				} //end recreating Hamiltonian (h_iter1, h_iter2)
				
				c1=(int)dim;                         /* and put all numbers and characters */
				c2=4*(int)dim;                       /* we want to pass */
				c3=1;                           /* to the routine in variables */
				c4='N';
				c5='V';
				
				dgeev_(&c4, &c5,&c1, HT, &c1, br, bi, DUMMY, &c3, VR, &c1, WORK, &c2, &ok);
				
				A = (double**)malloc(sizeof(double*));
				for (h_iter1 = 0; h_iter1 < 1; h_iter1++) {
					A[h_iter1] = (double*)malloc(dim*sizeof(double));
				} //end allocating memory for ground state wavefunction (h_iter1)
				
				if (ok==0) {
					cmp = find_min(br, (int)dim);
					
					for (h_iter1 = 0; h_iter1 < (int)dim; h_iter1++) {
						A[0][h_iter1] = -VR[(int)dim*cmp+h_iter1];
					} //end getting ground state wavefunction (h_iter1)
				} //if ok
				else {
					printf("Something screwed up...\n");
				}
				
				for (col_iter = 0; col_iter < h_col; col_iter++) {
					cell = sorted_cells[0][Hg[row_iter][col_iter]] ;
					current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
					old_polarization = current_cell_model->polarization;
					
					new = search_linked_list(headp, index+col_iter);
					
					MatOut1 = matrix_mult(A,new->Mat,c3,(int)dim,(int)dim,(int)dim);
					transA = transpose(A,c3,(int)dim);
					MatOut2 = matrix_mult(MatOut1,transA,c3,(int)dim,(int)dim,c3);
					
					new_polarization = -MatOut2[0][0];
					// -- set the polarization of this cell -- //
					current_cell_model->polarization = new_polarization;
					
					free(MatOut1[0]); MatOut1[0] = NULL;
					free(MatOut2[0]); MatOut2[0] = NULL;
					for (fb1 = 0; fb1 < (int)dim; fb1++) {
						free(transA[fb1]); transA[fb1] = NULL;
					} //end free memory (fb1)
					
					free(MatOut1); MatOut1 = NULL;
					free(MatOut2); MatOut2 = NULL;
					free(transA); transA = NULL;
			
					
					// If any cells polarization has changed beyond this threshold
					// then the entire circuit is assumed to have not converged.
					stable = (fabs (new_polarization - old_polarization) <= tolerance) ;
					
				} //end col_iter 
				
				free(HT); HT = NULL;
				free(DUMMY); DUMMY = NULL;
				free(VR); VR = NULL;
				free(br); br = NULL;
				free(bi); bi = NULL;
				free(WORK); WORK = NULL;
				
				free(A[0]); A[0] = NULL;
				free(A); A = NULL;
				
			} //!WHILE STABLE
			
			for (fb1 = 0; fb1 < (int)dim; fb1++) {
				free(Hx[fb1]); Hx[fb1] = NULL;
				free(Hp[fb1]); Hp[fb1] = NULL;
				free(H_group[fb1]); H_group[fb1] = NULL;
				free(H_init_x_temp[fb1]); H_init_x_temp[fb1] = NULL;
				free(H_init_p_temp[fb1]); H_init_p_temp[fb1] = NULL;
			} //end free memory (fb1)
			
			free(Hx); Hx = NULL;
			free(Hp); Hp = NULL;
			free(H_group); H_group = NULL;
			free(H_init_x_temp); H_init_x_temp = NULL;
			free(H_init_p_temp); H_init_p_temp = NULL;
			
			num_mats_prev = num_mats;
			
		}//FOR EACH ROW IN Hg (row_iter)
		
		// -- run the iteration with the given clock value -- //
		// -- iterate until the entire design has stabalized -- //
		iteration = 0;
		stable = FALSE;
		while (!stable && iteration < max_iterations_per_sample)
		{
			iteration++;
			// -- assume that the circuit is stable -- //
			stable = TRUE;
			
			for (icLayers = 0; icLayers < number_of_cell_layers; icLayers++)
			{
				for (icCellsInLayer = 0 ; icCellsInLayer < number_of_cells_in_layer[icLayers] ; icCellsInLayer++)
				{
					cell = sorted_cells[icLayers][icCellsInLayer] ;
					cmp = search_array(H_group_cell,icCellsInLayer, num_elements);
					
					if (cmp == -1) 
					{
						if (!((QCAD_CELL_INPUT == cell->cell_function)||
							  (QCAD_CELL_FIXED == cell->cell_function)))
						{
							
							
							current_cell_model = ((semi_coherent_model *)cell->cell_model) ;
							old_polarization = current_cell_model->polarization;
							polarization_math = 0;
							EkP = 0;
							
							for (q = 0; q < current_cell_model->number_of_neighbours; q++)
								EkP += (current_cell_model->Ek[q] * ((semi_coherent_model *)current_cell_model->neighbours[q]->cell_model)->polarization) ;
							
							H = (double**)malloc(size*sizeof(double*));
							for (h_iter1 = 0; h_iter1 < size; h_iter1++) {
								H[h_iter1] = (double*)malloc(size*sizeof(double));
							}
							
							H[0][0] = -EkP/2;
							H[0][1] = -(sim_data->clock_data[cell->cell_options.clock].data[j]);
							H[1][0] = -(sim_data->clock_data[cell->cell_options.clock].data[j]);
							H[1][1] = EkP/2;
							
							
							
							HT = (doublereal*)malloc(size*size*sizeof(doublereal));
							DUMMY = (doublereal*)malloc(sizeof(doublereal));
							VR = (doublereal*)malloc(size*size*sizeof(doublereal));
							br = (doublereal*)malloc(size*sizeof(doublereal));
							bi = (doublereal*)malloc(size*sizeof(doublereal));
							WORK = (doublereal*)malloc(4*size*sizeof(doublereal));	
							
							
							for (h_iter1=0; h_iter1<size; h_iter1++)          /* to call a Fortran routine from C we */
							{                               /* have to transform the matrix */
								for(h_iter2=0; h_iter2<size; h_iter2++)
								{
									HT[h_iter2+size*h_iter1]=H[h_iter2][h_iter1];
									//				printf("HT = %f\n", HT[h_iter2+size*h_iter1]);
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
							}
							
							
							if (ok==0) {
								if (br[0] < br[1]) {
									A[0][0] = VR[0];
									A[0][1] = VR[1];
								}
								else {
									A[0][0] = VR[2];
									A[0][1] = VR[3];
								}
							}
							else {
								fprintf(stderr, "Something screwed up...\n");
							}
							
							
							
							
							EkP /= (2.0 * sim_data->clock_data[cell->cell_options.clock].data[j]);
							
							// -- calculate the new cell polarization -- //
							if (EkP > 1000.0) {
								new_polarization = 1;
							}
							else {
								if (EkP < -1000.0) {
									new_polarization = -1;
								}
								else {
									MatOut1 = matrix_mult(A,pauli_z,c3,size,size,size);
									transA = transpose(A,c3,size);
									MatOut2 = matrix_mult(MatOut1,transA,c3,size,size,c3);
									
									
									if (MatOut2[0][0] > 1) {
										new_polarization = -1;
									}
									else {
										if (MatOut2[0][0] < -1) {
											new_polarization = 1;
										}
										else {    
											new_polarization = -MatOut2[0][0];
										}
									}
									
									
									free(MatOut1[0]); MatOut1[0] = NULL;
									free(MatOut2[0]); MatOut2[0] = NULL;
									for (fb1 = 0; fb1 < size; fb1++) {
										free(transA[fb1]); transA[fb1] = NULL;
									}
									
									free(MatOut1); MatOut1 = NULL;
									free(MatOut2); MatOut2 = NULL;
									free(transA); transA = NULL;
								}
							}
							
							// -- set the polarization of this cell -- //
							current_cell_model->polarization = new_polarization;
							
							
							// If any cells polarization has changed beyond this threshold
							// then the entire circuit is assumed to have not converged.
							stable = (fabs (new_polarization - old_polarization) <= tolerance) ;
							
							free(HT); HT = NULL;
							free(DUMMY); DUMMY = NULL;
							free(VR); VR = NULL;
							free(br); br = NULL;
							free(bi); bi = NULL;
							free(WORK); WORK = NULL;
							
							free(A[0]); A[0] = NULL;
							free(A); A = NULL;
							
							for (fb1 = 0; fb1 < size; fb1++) {
								free(H[fb1]); H[fb1] = NULL;
							}
							free(H); H = NULL;
						}	
					}
				}
			}
		}//WHILE !STABLE
		
		if (VECTOR_TABLE == SIMULATION_TYPE)
			for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
				if (!exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
					sim_data->trace[i].data[j] = ((semi_coherent_model *)exp_array_index_1d (pvt->inputs, VT_INPUT, i).input->cell_model)->polarization;
		
		// -- collect all the output data from the simulation -- //
		for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
			sim_data->trace[design->bus_layout->inputs->icUsed + i].data[j] = ((semi_coherent_model *)exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell->cell_model)->polarization;
		
		// -- if the user wants to stop the simulation then exit. -- //
		if(TRUE == STOP_SIMULATION)
			j = sim_data->number_samples ;
    }//for number of samples
	
	for (fb1 = 0; fb1 < num_elements; fb1++) {
		cell = sorted_cells[0][H_group_cell[fb1]];
		qcad_cell_set_mode (cell, QCAD_CELL_MODE_NORMAL);
	}

#ifdef DESIGNER
	redraw_async(NULL);
	gdk_flush () ;
#endif /* def DESIGNER */
	
	// Free the neigbours and Ek array introduced by this simulation//
	for (k = 0; k < number_of_cell_layers; k++)
    {
		for (l = 0 ; l < number_of_cells_in_layer[k] ; l++)
		{
			g_free(((semi_coherent_model *)sorted_cells[k][l]->cell_model)->neighbours);
			g_free(((semi_coherent_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
			g_free(((semi_coherent_model *)sorted_cells[k][l]->cell_model)->Ek);
		}
    }
	
	simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

	free_linked_list(headz, Hg, num_normal, 0);
	free_linked_list(headx, Hg, num_normal, 1);
	free_linked_list(headp, Hg, num_normal, 1);

	free(H_group_cell); 
	
	// Restore the input flag for the inactive inputs
	if (VECTOR_TABLE == SIMULATION_TYPE)
		for (i = 0 ; i < pvt->inputs->icUsed ; i++)
			exp_array_index_1d (pvt->inputs, VT_INPUT, i).input->cell_function = QCAD_CELL_INPUT ;
	
	// -- get and print the total simulation time -- //
	if ((end_time = time (NULL)) < 0)
		fprintf(stderr, "Could not get end time\n");
	
	command_history_message (_("Total simulation time: %g s\n"), (double)(end_time - start_time));
	
	set_progress_bar_visible (FALSE) ;
	
	return sim_data;
}//run_semi_coherent

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static inline void semi_coherent_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, semi_coherent_OP *options)
{
	int icNeighbours = 0 ;
	semi_coherent_model *cell_model = NULL ;
	int i, j, k, idx = 0, total_number_of_cells = 0;
	
	for(i = 0; i < number_of_cell_layers; i++)
		total_number_of_cells+= number_of_cells_in_layer[i];
	
	// calculate the Ek for each cell //
	for(i = 0; i < number_of_cell_layers; i++)
    {
		for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
		{
			if (0 == (idx++) % 100)
				set_progress_bar_fraction((double)idx / (double)total_number_of_cells);
			
			cell_model = (semi_coherent_model *)sorted_cells[i][j]->cell_model ;
			
			// free up memory for cell model variables //
			g_free (cell_model->neighbours);
			g_free (cell_model->Ek);
			g_free (cell_model->neighbour_layer);
			cell_model->neighbours = NULL;
			cell_model->neighbour_layer = NULL;
			cell_model->Ek = NULL;
			
			// select all neighbours within the provided radius //
			cell_model->number_of_neighbours = icNeighbours =
			select_cells_in_radius (sorted_cells, sorted_cells[i][j], options->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
									((semi_coherent_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));
			
			if (icNeighbours > 0)
			{
				cell_model->Ek = g_malloc0 (sizeof (double) * icNeighbours);
				
				// ensure no memory allocation error has ocurred //
				if (NULL == cell_model->neighbours ||
					NULL == cell_model->Ek)
					exit (1);
				
				for (k = 0; k < icNeighbours; k++)
					//if(cell_model->neighbours[k]==NULL)printf("Null neighbour prior to passing into determine Ek for k = %d\n", k);
					// set the Ek of this cell and its neighbour //
					
					cell_model->Ek[k] = semi_coherent_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS (i - cell_model->neighbour_layer[k]), options);
			}
		}
    }
}//refresh_all_Ek

//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static inline double semi_coherent_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, semi_coherent_OP *options)
{
	int k;
	int j;
	
	double distance = 0;
	double Constant = 1 / (FOUR_PI_EPSILON * options->epsilonR);
	double vertical_separation = (double)layer_separation * ((semi_coherent_OP *)options)->layer_separation;
	
	static double same_polarization[4][4] =
    {{  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR },
		{ -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR },
		{  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR },
		{ -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR }};
	
	static double diff_polarization[4][4] =
    {{ -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR },
		{  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR },
		{ -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR },
		{  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR,  QCHARGE_SQUAR_OVER_FOUR, -QCHARGE_SQUAR_OVER_FOUR }};
	
	double EnergyDiff = 0;
	double EnergySame = 0;
	
	g_assert (cell1 != NULL);
	g_assert (cell2 != NULL);
	g_assert (cell1 != cell2);
	
	for (k = 0; k < cell1->number_of_dots; k++)
		for (j = 0; j < cell2->number_of_dots; j++)
		{
			// determine the distance between the dots //
			distance = 1e-9 * determine_distance (cell1, cell2, k, j, vertical_separation);
			g_assert (distance != 0);
			
			EnergyDiff += diff_polarization[k][j] / distance;
			EnergySame += same_polarization[k][j] / distance;
		}//for other dots
	
	//printf("Kink Energy = %e \n", Constant * (EnergyDiff-EnergySame)/1.602e-19);
	
	return Constant * (EnergyDiff - EnergySame);
	
}// semi_coherent_determine_Ek

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

static inline void matrix_mult_by_const(double **Mat1, double constant, int dimx, int dimy, double **Mat0)
{
	
	int i = 0;
	int j = 0;
	for(i = 0; i < dimx; i++)
		for( j = 0; j < dimy; j++) {
			Mat0[i][j] =  constant*Mat1[i][j];
		}		
}

static inline void matrix_add(double **Mat1, double **Mat2, int dimx, int dimy, double **Mat0)
{

	int i = 0;
	int j = 0;
	for(i = 0; i < dimx; i++)
		for( j = 0; j < dimy; j++) {
			Mat0[i][j] =  Mat1[i][j] + Mat2[i][j];
		}		
}

static inline void matrix_copy(double **Mat1, int dimx, int dimy, double **Mat0)
{

	int i = 0;
	int j = 0;
	for(i = 0; i < dimx; i++)
		for( j = 0; j < dimy; j++) {
			Mat0[i][j] =  Mat1[i][j];
		}		
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

static inline void kron (double **MatA, double **MatB, int dimAx, int dimAy, int dimBx, int dimBy, double **Mat0)
{
	
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int m, n;
	
	double **TEMP;
	TEMP = (double**)malloc(dimAx*dimBx*sizeof(double*));
	for (m = 0; m < dimAx*dimBx; m++) {
		TEMP[m] = (double*)malloc(dimAy*dimBy*sizeof(double));
	}
	
	
	
	for(i = 0; i < dimAx; i++)
		for( j = 0; j < dimAy; j++)
			for( k = 0; k < dimBx; k++)
				for( l = 0; l < dimBy; l++) {
					TEMP[dimBx*i+k][dimBy*j+l] =  MatA[i][j] * MatB[k][l];
					//printf("Mat0[%d][%d] = %f\n", dimBx*i+k, dimBy*j+l, Mat0[dimBx*i+k][dimBy*j+l]);
				}
	
	for (m = 0; m < dimAx*dimBx; m++) {
		for (n = 0; n < dimAy*dimBy; n++) {
			Mat0[m][n] = TEMP[m][n];
		}
	}
	
	for (m = 0; m < dimAx*dimBx; m++) {
		free(TEMP[m]); TEMP[m] = NULL;
	}
	free(TEMP); TEMP = NULL;
}

static inline double get_max(double *array, int num_elements) {
	
	int i;
	double max = -32000;
	
	for (i = 0; i < num_elements; i++) {
        if (array[i] > max) {
			max = array[i];
        }
	}
	
	return max;
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

static inline int search_array(int *A, int cmp, int num_elements )
{
	
	int i;
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			return i;
		}
	}
	return -1;	
}

static inline int search_matrix_row(int **A, int cmp, int row, int num_elements)
{
	
	int i;
	for (i = 0; i<num_elements; i++) {
		if (A[row][i] == cmp) {
			return i;
		}
	}
	return -1;	
}

static inline int search_matrix_col(int **A, int cmp, int col, int num_elements)
{
	
	int i;
	for (i = 0; i<num_elements; i++) {
		if (A[i][col] == cmp) {
			return i;
		}
	}
	return -1;	
}

static inline struct cluster* search_linked_list(struct cluster *head, int element)
{
	struct cluster *new;
	struct cluster *current;
	current = head;
	int i = 0;
	
	while (element != i) {
		current = current->next;
		i++;
	}
	new = current;
	return new;
}

static inline void free_linked_list(struct cluster *head, int **Hg, int num_elements, int option)
{
	struct cluster* cur_ptr; 
	struct cluster* next_ptr;
	cur_ptr = head;
	
	int i, j, k;
	int rows, cols;
	double dim;
	rows = search_matrix_col(Hg, -1, 0, num_elements);
	
	if (!option) {
		for (i = 0; i < rows; i++) {
			cols = search_matrix_row(Hg, -1, i, num_elements);
			dim = pow(2,cols);
			if (cur_ptr != NULL) {
				next_ptr = cur_ptr->next;
				for (j = 0; j < dim; j++) {
					free(cur_ptr->Mat[j]);
				}
				free(cur_ptr->Mat);
				free(cur_ptr);
				cur_ptr = next_ptr;
			}
		}
	}
	else {
		for (i = 0; i < rows; i++) {
			cols = search_matrix_row(Hg, -1, i, num_elements);
			dim = pow(2,cols);
			for (j = 0; j < cols; j++) {
				next_ptr = cur_ptr->next;
				for (k = 0; k < dim; k++) {
					free(cur_ptr->Mat[k]);
				}
				free(cur_ptr->Mat);
				free(cur_ptr);
				cur_ptr = next_ptr;
			}
		}
	}
}
	

void semi_coherent_options_dump (semi_coherent_OP *semi_coherent_options, FILE *pfile)
{
	fprintf (stderr, "semi_coherent_options_dump:\n") ;
	fprintf (stderr, "semi_coherent_options->number_of_samples         = %d\n",      semi_coherent_options->number_of_samples) ;
	fprintf (stderr, "semi_coherent_options->animate_simulation        = %s\n",      semi_coherent_options->animate_simulation ? "TRUE" : "FALSE") ;
	fprintf (stderr, "semi_coherent_options->convergence_tolerance     = %e\n",      semi_coherent_options->convergence_tolerance) ;
	fprintf (stderr, "semi_coherent_options->radius_of_effect          = %e [nm]\n", semi_coherent_options->radius_of_effect) ;
	fprintf (stderr, "semi_coherent_options->epsilonR                  = %e\n",      semi_coherent_options->epsilonR) ;
	fprintf (stderr, "semi_coherent_options->clock_high                = %e [J]\n",  semi_coherent_options->clock_high) ;
	fprintf (stderr, "semi_coherent_options->clock_low                 = %e [J]\n",  semi_coherent_options->clock_low) ;
	fprintf (stderr, "semi_coherent_options->clock_shift               = %e [J]\n",  semi_coherent_options->clock_shift) ;
	fprintf (stderr, "semi_coherent_options->clock_amplitude_factor    = %e\n",      semi_coherent_options->clock_amplitude_factor) ;
	fprintf (stderr, "semi_coherent_options->max_iterations_per_sample = %d\n",      semi_coherent_options->max_iterations_per_sample) ;
	fprintf (stderr, "semi_coherent_options->layer_separation          = %e [nm]\n", semi_coherent_options->layer_separation) ;
	// Added by Marco
	fprintf (stderr, "semi_coherent_options->jitter_phase_0            = %f degrees\n",      semi_coherent_options->jitter_phase_0) ;
 	fprintf (stderr, "semi_coherent_options->jitter_phase_1            = %f degrees\n",      semi_coherent_options->jitter_phase_1) ;
 	fprintf (stderr, "semi_coherent_options->jitter_phase_2            = %f degrees\n",      semi_coherent_options->jitter_phase_2) ;
 	fprintf (stderr, "semi_coherent_options->jitter_phase_3            = %f degrees\n",      semi_coherent_options->jitter_phase_3) ;
	// End added by Marco
}
