#include "support.h"
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
    if (NULL == (psz = get_file_name_from_user (NULL, _("Open Simulation Results"), NULL, FALSE)))
      return 1 ;
    }
  else
    psz = absolute_path (argv[1]) ;

  if (NULL == (sim_output = open_simulation_output_file (psz)))
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
      _("Failed to open simulation data file %s !\n"), psz))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    return 2 ;
    }

  set_file_selection_file_name (psz) ;
  g_free (psz) ;

  show_graph_dialog (NULL, sim_output->sim_data, sim_output->bus_layout, TRUE, TRUE) ;

  return 0 ;
  }
