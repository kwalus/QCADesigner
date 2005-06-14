#include "file_selection_window.h"
#include "graph_dialog.h"
#include "simulation_data.h"
#include "fileio.h"
#include "gtk_preamble.h"
#include "fileio_helpers.h"

int main (int argc, char **argv)
  {
  char *psz = NULL ;

  gtk_preamble (&argc, &argv) ;
  SIMULATION_OUTPUT *sim_output = NULL ;

  if (1 == argc)
    {
    fprintf (stderr, "Usage: %s <simulation_output_file>\n", argv[0]) ;
    return 1 ;
    }

  if (NULL == (sim_output = open_simulation_output_file (argv[1])))
    {
    fprintf (stderr, "Failed to open simulation output file \"%s\"!\n", argv[1]) ;
    return 2 ;
    }

  set_file_selection_file_name (psz = absolute_path (argv[1])) ;
  g_free (psz) ;

  show_graph_dialog (NULL, sim_output->sim_data, sim_output->bus_layout, TRUE, TRUE) ;

  return 0 ;
  }
