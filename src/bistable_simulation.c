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

#ifdef GTK_GUI
  #include "callback_helpers.h"
#endif /* def GTK_GUI */
#include "intl.h"
#include "objects/QCADCell.h"
#include "simulation.h"
#include "bistable_simulation.h"
#include "custom_widgets.h"
#include "global_consts.h"

//!Options for the bistable simulation engine
//This variable is used by multiple source files
// Added by Marco March 3 : last four arguments (phase shifts)

bistable_OP bistable_options = {12800, FALSE, 1e-3, 200, 1, 9.43e-19, 1.41e-20, 0.0, 2.0, 100, 1.15, TRUE,0,0,0,0} ;


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
  } bistable_model;

static inline double bistable_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, bistable_OP *options);
static inline void bistable_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, bistable_OP *options);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_bistable_simulation (int SIMULATION_TYPE, DESIGN *design, bistable_OP *options, VectorTable *pvt)
  {
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
  int max_iterations_per_sample = ((bistable_OP *)options)->max_iterations_per_sample;
  BUS_LAYOUT_ITER bli ;
  // For randomization
  int Nix, Nix1, idxCell1, idxCell2 ;
  QCADCell *swap = NULL ;
  // -- these used to be inside run_bistable_iteration -- //
  int q, iteration = 0;
  int stable = FALSE;
  double old_polarization;
  double new_polarization;
  double tolerance = ((bistable_OP *)options)->convergence_tolerance;
  double polarization_math;
  bistable_model *current_cell_model = NULL ;
  QCADCell *cell;
  int jitter_phases[4] = {options->jitter_phase_0, options->jitter_phase_1, 
                             options->jitter_phase_2, options->jitter_phase_3} ;

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
      current_cell_model = g_malloc0 (sizeof(bistable_model)) ;
      sorted_cells[i][j]->cell_model = current_cell_model;

      // -- Clear the model pointers so they are not dangling -- //
      current_cell_model->neighbours = NULL;
      current_cell_model->Ek = NULL;

      // -- set polarization in cell model for fixed cells since they are set with actual dot charges by the user -- //
      if(QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)
       current_cell_model->polarization = qcad_cell_calculate_polarization(sorted_cells[i][j]);

      total_cells++;
      }
    }

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
  set_progress_bar_label (_("Bistable simulation:")) ;

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
  bistable_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);

  // randomize the cells in the design so as to minimize any numerical problems associated //
  // with having cells simulated in some predefined order. //
  // randomize the order in which the cells are simulated //
  //if (options->randomize_cells)
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
            qcad_cell_set_polarization(sorted_cells[icLayers][icCellsInLayer],((bistable_model *)sorted_cells[icLayers][icCellsInLayer]->cell_model)->polarization);
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
        ((bistable_model *)exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell->cell_model)->polarization =
          sim_data->trace[i].data[j] = (-1 * sin (((double)(1 << idxMasterBitOrder)) * (double)j * FOUR_PI / (double)sim_data->number_samples) > 0) ? 1 : -1 ;
    else
//    if (VECTOR_TABLE == SIMULATION_TYPE)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
          ((bistable_model *)exp_array_index_1d (pvt->inputs, VT_INPUT, i).input->cell_model)->polarization =
            sim_data->trace[i].data[j] = exp_array_index_2d (pvt->vectors, gboolean, (j * pvt->vectors->icUsed) / sim_data->number_samples, i) ? 1 : -1 ;

    // randomize the order in which the cells are simulated to try and minimize numerical errors
    // associated with the imposed simulation order.
    if(options->randomize_cells)
      // for each layer ...
      for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
        {
        // ...perform as many swaps as there are cells therein
        for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
          {
          idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
          idxCell2 = rand () % number_of_cells_in_layer[Nix] ;

          swap = sorted_cells[Nix][idxCell1] ;
          sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
          sorted_cells[Nix][idxCell2] = swap ;
          }
        }

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

          if (!((QCAD_CELL_INPUT == cell->cell_function)||
                (QCAD_CELL_FIXED == cell->cell_function)))
            {
            current_cell_model = ((bistable_model *)cell->cell_model) ;
            old_polarization = current_cell_model->polarization;
            polarization_math = 0;

            for (q = 0; q < current_cell_model->number_of_neighbours; q++)
              polarization_math += (current_cell_model->Ek[q] * ((bistable_model *)current_cell_model->neighbours[q]->cell_model)->polarization) ;

            // math = math / 2 * gamma
            polarization_math /= (2.0 * sim_data->clock_data[cell->cell_options.clock].data[j]);

            // -- calculate the new cell polarization -- //
            // if math < 0.05 then math/sqrt(1+math^2) ~= math with error <= 4e-5
            // if math > 100 then math/sqrt(1+math^2) ~= +-1 with error <= 5e-5
            new_polarization =
              (polarization_math        >  1000.0)   ?  1                 :
              (polarization_math        < -1000.0)   ? -1                 :
              (fabs (polarization_math) <     0.001) ?  polarization_math :
                polarization_math / sqrt (1 + polarization_math * polarization_math) ;

            // -- set the polarization of this cell -- //
            current_cell_model->polarization = new_polarization;

            // If any cells polarization has changed beyond this threshold
            // then the entire circuit is assumed to have not converged.
            stable = (fabs (new_polarization - old_polarization) <= tolerance) ;
            }
          }
        }
      }//WHILE !STABLE

    if (VECTOR_TABLE == SIMULATION_TYPE)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        if (!exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
          sim_data->trace[i].data[j] = ((bistable_model *)exp_array_index_1d (pvt->inputs, VT_INPUT, i).input->cell_model)->polarization;

    // -- collect all the output data from the simulation -- //
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      sim_data->trace[design->bus_layout->inputs->icUsed + i].data[j] = ((bistable_model *)exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell->cell_model)->polarization;

    // -- if the user wants to stop the simulation then exit. -- //
    if(TRUE == STOP_SIMULATION)
      j = sim_data->number_samples ;
    }//for number of samples

  // Free the neigbours and Ek array introduced by this simulation//
  for (k = 0; k < number_of_cell_layers; k++)
    {
    for (l = 0 ; l < number_of_cells_in_layer[k] ; l++)
      {
      g_free(((bistable_model *)sorted_cells[k][l]->cell_model)->neighbours);
      g_free(((bistable_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
      g_free(((bistable_model *)sorted_cells[k][l]->cell_model)->Ek);
      }
    }

  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

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
  }//run_bistable

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static inline void bistable_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, bistable_OP *options)
  {
  int icNeighbours = 0 ;
  bistable_model *cell_model = NULL ;
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

      cell_model = (bistable_model *)sorted_cells[i][j]->cell_model ;

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
          ((bistable_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));

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
          
          cell_model->Ek[k] = bistable_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS (i - cell_model->neighbour_layer[k]), options);
        }
      }
    }
  }//refresh_all_Ek

//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static inline double bistable_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, bistable_OP *options)
  {
  int k;
  int j;

  double distance = 0;
  double Constant = 1 / (FOUR_PI_EPSILON * options->epsilonR);
  double vertical_separation = (double)layer_separation * ((bistable_OP *)options)->layer_separation;

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

  }// bistable_determine_Ek

void bistable_options_dump (bistable_OP *bistable_options, FILE *pfile)
  {
  fprintf (stderr, "bistable_options_dump:\n") ;
	fprintf (stderr, "bistable_options->number_of_samples         = %d\n",      bistable_options->number_of_samples) ;
	fprintf (stderr, "bistable_options->animate_simulation        = %s\n",      bistable_options->animate_simulation ? "TRUE" : "FALSE") ;
	fprintf (stderr, "bistable_options->convergence_tolerance     = %e\n",      bistable_options->convergence_tolerance) ;
	fprintf (stderr, "bistable_options->radius_of_effect          = %e [nm]\n", bistable_options->radius_of_effect) ;
	fprintf (stderr, "bistable_options->epsilonR                  = %e\n",      bistable_options->epsilonR) ;
	fprintf (stderr, "bistable_options->clock_high                = %e [J]\n",  bistable_options->clock_high) ;
	fprintf (stderr, "bistable_options->clock_low                 = %e [J]\n",  bistable_options->clock_low) ;
	fprintf (stderr, "bistable_options->clock_shift               = %e [J]\n",  bistable_options->clock_shift) ;
	fprintf (stderr, "bistable_options->clock_amplitude_factor    = %e\n",      bistable_options->clock_amplitude_factor) ;
	fprintf (stderr, "bistable_options->max_iterations_per_sample = %d\n",      bistable_options->max_iterations_per_sample) ;
	fprintf (stderr, "bistable_options->layer_separation          = %e [nm]\n", bistable_options->layer_separation) ;
	fprintf (stderr, "bistable_options->randomize_cells           = %s\n",      bistable_options->randomize_cells ? "TRUE" : "FALSE") ;
// Added by Marco
	fprintf (stderr, "bistable_options->jitter_phase_0            = %f degrees\n",      bistable_options->jitter_phase_0) ;
 	fprintf (stderr, "bistable_options->jitter_phase_1            = %f degrees\n",      bistable_options->jitter_phase_1) ;
 	fprintf (stderr, "bistable_options->jitter_phase_2            = %f degrees\n",      bistable_options->jitter_phase_2) ;
 	fprintf (stderr, "bistable_options->jitter_phase_3            = %f degrees\n",      bistable_options->jitter_phase_3) ;
// End added by Marco
  }
