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
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// A batch mode simulator aimed specifically at         //
// verifying outputs against their corresponding        //
// inputs.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "fileio.h"
#include "design.h"
#include "graph_dialog.h"
#include "global_consts.h"
#include "simulation.h"
#include "simulation_data.h"
#include "coherence_vector.h"
#include "graph_dialog_widget_data.h"
#include "bistable_simulation.h"

extern bistable_OP bistable_options ;
extern coherence_OP coherence_options ;

typedef struct
  {
  gboolean bExitOnFailure ;
  int number_of_sims ;
  int sim_engine ;
  double dTolerance ;
  double dThreshLower ;
  double dThreshUpper ;
  int icAverageSamples ;
  char *pszFailureFNamePrefix ;
  char *pszSimOptsFName ;
  char *pszReferenceSimOutputFName ;
  char *pszFName ;
  } CMDLINE_ARGS ;

static void randomize_design_cells (GRand *rnd, DESIGN *design, double dMinRadius, double dMaxRadius) ;
static EXP_ARRAY *create_honeycombs_from_buses (simulation_data *sim_data, BUS_LAYOUT *bus_layout, int bus_function, double dThreshLower, double dThreshUpper, int icAverageSamples) ;
static int determine_success (HONEYCOMB_DATA *hcIn, HONEYCOMB_DATA *hcOut) ;
static void parse_cmdline (int argc, char **argv, CMDLINE_ARGS *cmdline_args) ;

int main (int argc, char **argv)
  {
  int Nix, Nix1, Nix2 ;
  DESIGN *design = NULL, *working_design = NULL ;
  simulation_data *sim_data = NULL ;
  GRand *rnd = NULL ;
  EXP_ARRAY *input_hcs = NULL, *output_hcs = NULL ;
  HONEYCOMB_DATA *hc = NULL ;
  EXP_ARRAY *icSuccesses = NULL ;
  int icOutputBuses = 0 ;
  SIMULATION_OUTPUT *sim_output = NULL ;
  int single_success = 0 ;
  gboolean bDie = FALSE ;
  CMDLINE_ARGS cmdline_args = 
    {
    .bExitOnFailure             = FALSE,
    .number_of_sims             =  1,
    .sim_engine                 = BISTABLE,
    .dTolerance                 =  0.0,
    .dThreshLower               = -0.5,
    .dThreshUpper               =  0.5,
    .icAverageSamples           =  1,
    .pszFailureFNamePrefix      = NULL,
    .pszSimOptsFName            = NULL,
    .pszReferenceSimOutputFName = NULL,
    .pszFName                   = NULL
    } ;

  parse_cmdline (argc, argv, &cmdline_args) ;

  fprintf (stderr,
    "Simulation engine               : %s\n"
    "Simulation options file         : %s\n"
    "Circuit file                    : %s\n"
    "Reference simulation output file: %s\n"
    "number of simulations           : %d\n"
    "tolerance                       : %lf\n"
    "lower polarization threshold    : %lf\n"
    "upper polarization threshold    : %lf\n"
    "samples for running average     : %d\n"
    "exit on failure ?               : %s\n"
    "  failure output file name        : %s\n",
    COHERENCE_VECTOR == cmdline_args.sim_engine ? "COHERENCE_VECTOR" : "BISTABLE",
    cmdline_args.pszSimOptsFName,         cmdline_args.pszFName,         cmdline_args.pszReferenceSimOutputFName, 
    cmdline_args.number_of_sims,          cmdline_args.dTolerance,       cmdline_args.dThreshLower, 
    cmdline_args.dThreshUpper,            cmdline_args.icAverageSamples, cmdline_args.bExitOnFailure ? "TRUE" : "FALSE",
    cmdline_args.pszFailureSimOutputFName) ;

#ifdef GTK_GUI
  gtk_init (&argc, &argv) ;
#else
  g_type_init () ;
#endif /* def GTK_GUI */

  if (!open_project_file (cmdline_args.pszFName, &design))
    {
    fprintf (stderr, "Failed to open the circuit file !\n") ;
    return 1 ;
    }
  else
  if (NULL == design)
    {
    fprintf (stderr, "Failed to open the circuit file !\n") ;
    return 2 ;
    }

  if (NULL == (sim_output = open_simulation_output_file (cmdline_args.pszReferenceSimOutputFName)))
    {
    fprintf (stderr, "Failed to open the reference simulation output file !\n") ;
    return 3 ;
    }

  if (BISTABLE == cmdline_args.sim_engine)
    {
    bistable_OP *bo = NULL ;

    if (NULL == (bo = open_bistable_options_file (cmdline_args.pszSimOptsFName)))
      {
      fprintf (stderr, "Failed to open simulation options file !\n") ;
      return 4 ;
      }
    bistable_options_dump (bo, stderr) ;
    memcpy (&bistable_options, bo, sizeof (bistable_OP)) ;
    }
  else
  if (COHERENCE_VECTOR == cmdline_args.sim_engine)
    {
    coherence_OP *co = NULL ;

    if (NULL == (co = open_coherence_options_file (cmdline_args.pszSimOptsFName)))
      {
      fprintf (stderr, "Failed to open simulation options file !\n") ;
      return 5 ;
      }
    coherence_options_dump (co, stderr) ;
    memcpy (&coherence_options, co, sizeof (coherence_OP)) ;
    }

  printf ("Running %d simulations with a radial tolerance of %lf\n", cmdline_args.number_of_sims, cmdline_args.dTolerance) ;

  input_hcs = create_honeycombs_from_buses (sim_output->sim_data, sim_output->bus_layout, QCAD_CELL_OUTPUT, cmdline_args.dThreshLower, cmdline_args.dThreshUpper, cmdline_args.icAverageSamples) ;

  rnd = g_rand_new () ;

  icSuccesses = exp_array_new (sizeof (int), 1) ;
  for (Nix = 0 ; Nix < design->bus_layout->buses->icUsed ; Nix++)
    if (QCAD_CELL_OUTPUT == exp_array_index_1d (design->bus_layout->buses, BUS, Nix).bus_function)
      icOutputBuses++ ;
  exp_array_insert_vals (icSuccesses, NULL, icOutputBuses, 1, 0) ;
  for (Nix = 0 ; Nix < icSuccesses->icUsed ; Nix++)
    exp_array_index_1d (icSuccesses, int, Nix) = 0 ;

  for (Nix = 0 ; Nix < cmdline_args.number_of_sims && !bDie ; Nix++)
    {
    fprintf (stderr, "Running simulation %d\n", Nix) ;
    if (NULL != (working_design = design_copy (design)))
      {
      randomize_design_cells (rnd, working_design, 0.0, cmdline_args.dTolerance) ;

      if (NULL != (sim_data = run_simulation (cmdline_args.sim_engine, EXHAUSTIVE_VERIFICATION, working_design, NULL)))
        {
        output_hcs = create_honeycombs_from_buses (sim_data, working_design->bus_layout, QCAD_CELL_OUTPUT, cmdline_args.dThreshLower, cmdline_args.dThreshUpper, cmdline_args.icAverageSamples) ;
        // Compare the output honeycombs to the input honeycombs
        for (Nix1 = 0 ; Nix1 < output_hcs->icUsed && !bDie ; Nix1++)
          {
          single_success =
            determine_success (
              exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1),
              exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1)) ;

          if (0 == single_success && cmdline_args.bExitOnFailure)
            {
            if (NULL != cmdline_args.pszFailureSimOutputFName)
              {
              SIMULATION_OUTPUT failed_sim_output = {sim_data, design->bus_layout, FALSE} ;

              create_simulation_output_file (cmdline_args.pszFailureSimOutputFName, &failed_sim_output) ;
              }
            bDie = TRUE ;
            }
          else
            exp_array_index_1d (icSuccesses, int, Nix1) += single_success ;
          }

        // Print out the results
        for (Nix1 = 0 ; Nix1 < MAX (input_hcs->icUsed, output_hcs->icUsed) ; Nix1++)
          {
          if (Nix1 < input_hcs->icUsed)
            {
            hc = exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1) ;
            fprintf (stderr, "First trace in this reference output bus is \"%s\"\n", exp_array_index_1d (hc->arTraces, struct TRACEDATA *, 0)->data_labels) ;
            for (Nix2 = 0 ; Nix2 < hc->arHCs->icUsed ; Nix2++)
              fprintf (stderr, "%d ", (int)(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix2).value)) ;
            fprintf (stderr, "\n") ;
            }
          if (Nix1 < output_hcs->icUsed)
            {
            hc = exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1) ;
            fprintf (stderr, "First trace in this output bus is \"%s\"\n", exp_array_index_1d (hc->arTraces, struct TRACEDATA *, 0)->data_labels) ;
            for (Nix2 = 0 ; Nix2 < hc->arHCs->icUsed ; Nix2++)
              fprintf (stderr, "%d ", (int)(exp_array_index_1d (hc->arHCs, HONEYCOMB, Nix2).value)) ;
            fprintf (stderr, "\n") ;
            }
          }

        fprintf (stderr, "*******************\n") ;
        for (Nix1 = 0 ; Nix1 < output_hcs->icUsed ; Nix1++)
          honeycomb_data_free (exp_array_index_1d (output_hcs, HONEYCOMB_DATA *, Nix1)) ;
        output_hcs = exp_array_free (output_hcs) ;
        sim_data = simulation_data_destroy (sim_data) ;
        }
      working_design = design_destroy (working_design) ;
      }
    }

  for (Nix1 = 0 ; Nix1 < input_hcs->icUsed ; Nix1++)
    honeycomb_data_free (exp_array_index_1d (input_hcs, HONEYCOMB_DATA *, Nix1)) ;
  input_hcs = exp_array_free (input_hcs) ;

  for (Nix = 0 ; Nix < icSuccesses->icUsed ; Nix++)
    printf ("success_rate[%d] = %.2lf%%\n", Nix, (double)(exp_array_index_1d (icSuccesses, int, Nix)) / ((double)(cmdline_args.number_of_sims)) * 100.0) ;

  g_rand_free (rnd) ;

  return 0 ;
  }

static EXP_ARRAY *create_honeycombs_from_buses (simulation_data *sim_data, BUS_LAYOUT *bus_layout, int bus_function, double dThreshLower, double dThreshUpper, int icAverageSamples)
  {
  GdkColor clr = {0, 0, 0, 0} ;
  int Nix1 ;
  HONEYCOMB_DATA *hc = NULL ;
  BUS *bus = NULL ;
  EXP_ARRAY *output_hcs = exp_array_new (sizeof (HONEYCOMB_DATA *), 1) ;
  // For each bus, create the appropriate HONEYCOMB_DATA, fill it in with
  // the TRACEDATA structures, and place each HONEYCOMB_DATA into its
  // appropriate EXP_ARRAY.
  for (Nix1 = 0 ; Nix1 < bus_layout->buses->icUsed ; Nix1++)
    if (bus_function == (bus = &exp_array_index_1d (bus_layout->buses, BUS, Nix1))->bus_function)
      {
      hc = honeycomb_data_new_with_array (&clr, sim_data, bus, (QCAD_CELL_INPUT == bus->bus_function ? 0 : bus_layout->inputs->icUsed), dThreshLower, dThreshUpper, icAverageSamples, 2) ;
      exp_array_insert_vals (output_hcs, &hc, 1, -1) ;
      }
  return output_hcs ;
  }

static void randomize_design_cells (GRand *rnd, DESIGN *design, double dMinRadius, double dMaxRadius)
  {
  double dRadius = -1.0, dAngle = 0.0 ;
  double dx = 0.0, dy = 0.0 ;
  GList *llItr = NULL, *llItrObj = NULL ;
  QCADLayer *layer = NULL ;

  if (NULL == rnd || NULL == design) return ;

  for (llItr = design->lstLayers ; llItr != NULL ; llItr = llItr->next)
    if (LAYER_TYPE_CELLS == (layer = QCAD_LAYER (llItr->data))->type)
      for (llItrObj = layer->lstObjs ; llItrObj != NULL ; llItrObj = llItrObj->next)
        if (NULL != llItrObj->data)
          {
          dRadius = g_rand_double_range (rnd, dMinRadius, dMaxRadius) ;
          dAngle = g_rand_double_range (rnd, 0, 2.0 * PI) ;

          dx = dRadius * cos (dAngle) ;
          dy = dRadius * sin (dAngle) ;

          qcad_design_object_move (QCAD_DESIGN_OBJECT (llItrObj->data), dx, dy) ;
          }
  }

static void parse_cmdline (int argc, char **argv, CMDLINE_ARGS *cmdline_args)
  {
  gboolean bDie = FALSE ;
  int icParms = 0 ;
  int Nix ;

  // defaults
  cmdline_args->sim_engine = BISTABLE ;

  for (Nix = 0 ; Nix < argc ; Nix++)
    {
    if (!(strcmp (argv[Nix], "-a") && strcmp (argv[Nix], "--average")))
      {
      if (++Nix < argc)
        cmdline_args->icAverageSamples = atoi (argv[Nix]) ;
      }
    else
    if (!(strcmp (argv[Nix], "-e") && strcmp (argv[Nix], "--engine")))
      {
      if (++Nix < argc)
        cmdline_args->sim_engine =
          !strcmp (argv[Nix], "BISTABLE")
            ? BISTABLE
            : !strcmp (argv[Nix], "COHERENCE_VECTOR")
              ? COHERENCE_VECTOR
              : BISTABLE /* default */ ;
      }
    else
    if (!(strcmp (argv[Nix], "-f") && strcmp (argv[Nix], "--file")))
      {
      if (++Nix < argc)
        {
        cmdline_args->pszFName = argv[Nix] ;
        icParms++ ;
        }
      }
    else
    if (!(strcmp (argv[Nix], "-l") && strcmp (argv[Nix], "--lower")))
      {
      if (++Nix < argc)
        cmdline_args->dThreshLower = g_ascii_strtod (argv[Nix], NULL) ;
      }
    else
    if (!(strcmp (argv[Nix], "-n") && strcmp (argv[Nix], "--number")))
      {
      if (++Nix < argc)
        {
        cmdline_args->number_of_sims = atoi (argv[Nix]) ;
        icParms++ ;
        }
      }
    else
    if (!(strcmp (argv[Nix], "-o") && strcmp (argv[Nix], "--options")))
      {
      if (++Nix < argc)
        {
        cmdline_args->pszSimOptsFName = argv[Nix] ;
        icParms++ ;
        }
      }
    else
    if (!(strcmp (argv[Nix], "-r") && strcmp (argv[Nix], "--results")))
      {
      if (++Nix < argc)
        {
        cmdline_args->pszReferenceSimOutputFName = argv[Nix] ;
        icParms++ ;
        }
      }
    else
    if (!(strcmp (argv[Nix], "-t") && strcmp (argv[Nix], "--tolerance")))
      {
      if (++Nix < argc)
        {
        cmdline_args->dTolerance = g_ascii_strtod (argv[Nix], NULL) ;
        icParms++ ;
        }
      }
    else
    if (!(strcmp (argv[Nix], "-u") && strcmp (argv[Nix], "--upper")))
      {
      if (++Nix < argc)
        cmdline_args->dThreshUpper = g_ascii_strtod (argv[Nix], NULL) ;
      }
    else
    if (!(strcmp (argv[Nix], "-x") && strcmp (argv[Nix], "--exit")))
      cmdline_args->bExitOnFailure = TRUE ;
    else
    if (!(strcmp (argv[Nix], "-s") && strcmp (argv[Nix], "--save")))
      {
      if (++Nix < argc)
        cmdline_args->pszFailureSimOutputFName = argv[Nix] ;
      }
    }

  if (icParms < 5 || bDie)
    {
    printf (
"Usage: batch_sim options...\n"
"\n"
"Options are:\n"
"  -a  --average   samples       Optional: Number of samples to use for running average. Default is 1.\n"
"  -e  --engine    engine        Optional: The simulation engine. One of BISTABLE (default) or COHERENCE_VECTOR.\n"
"  -f  --file      file          Required: The circuit file.\n"
"  -l  --lower     polarization  Optional: Lower polarization threshold. Between -1.00 and 1.00. Default is -0.5.\n"
"  -n  --number    number        Required: Number of simulations to perform.\n"
"  -o  --options   file          Required: Simulation engine options file.\n"
"  -r  --results   file          Required: Simulation results file to compare generated results to.\n"
"  -t  --tolerance tolerance     Required: Radial tolerance. Non-negative floating point value.\n"
"  -u  --upper     polarization  Optional: Upper polarization threshold. Between -1.00 and 1.00. Default is 0.5.\n"
"  -x  --exit                    Optional: Turn on exit-on-first-failure.\n"
"    -s  --save      file_prefix   Optional: Save failing simulation output to file_prefix.sim_output\n"
"                                            and the circuit to file_prefix.qca.\n"
) ;
    exit (1) ;
    }
  }

static int determine_success (HONEYCOMB_DATA *hcdIn, HONEYCOMB_DATA *hcdOut)
  {
  int Nix ;
  int idxIn = 0 ;
  HONEYCOMB *hcIn = NULL, *hcOut = NULL ;

  if (NULL == hcdIn || NULL == hcdOut) return 0 ;
  if (NULL == hcdIn->arHCs || NULL == hcdOut->arHCs) return 0 ;
  if (0 == hcdIn->arHCs->icUsed || 0 == hcdOut->arHCs->icUsed) return 0 ;

  // If both honeycomb arrays have the same number of honeycombs, compare the values
  if (hcdIn->arHCs->icUsed == hcdOut->arHCs->icUsed)
    {
    for (Nix = 0 ; Nix < hcdIn->arHCs->icUsed ; Nix++)
      {
      fprintf (stderr, "Honeycomb value %d: %llu vs %llu\n", Nix,
        exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, Nix).value, exp_array_index_1d (hcdOut->arHCs, HONEYCOMB, Nix).value) ;
      if (exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, Nix).value != exp_array_index_1d (hcdOut->arHCs, HONEYCOMB, Nix).value)
        break ;
      }
    fprintf (stderr, "Nix = %d vs. hcdIn->arHCs->icUsed = %d\n", Nix, hcdIn->arHCs->icUsed) ;
    return (Nix == hcdIn->arHCs->icUsed) ? 1 : 0 ;
    }

  hcIn = &exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, 0) ;

  for (Nix = 0 ; Nix < hcdOut->arHCs->icUsed ; Nix++)
    {
    hcOut = &exp_array_index_1d (hcdOut->arHCs, HONEYCOMB, Nix) ;

    // This output honeycomb may be contained withing the next input honeycomb
    if (hcOut->idxBeg > hcIn->idxEnd)
      {
      if (++idxIn == hcdIn->arHCs->icUsed) return 0 ;
      hcIn = &exp_array_index_1d (hcdIn->arHCs, HONEYCOMB, idxIn) ;
      }

    // The output honeycomb is not entirely contained within the input honeycomb
    if (hcOut->idxBeg < hcIn->idxBeg || hcOut->idxEnd > hcIn->idxEnd)
      return 0 ;

    if (hcOut->value != hcIn->value)
      return 0 ;
    }

  return 1 ;
  }
