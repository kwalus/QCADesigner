#include <gtk/gtk.h>
#include "simulation.h"
#include "print_dialog.h"
#include "print_graph_properties_dialog.h"
#include "print.h"
#include "print_preview.h"

typedef struct
  {
  GtkWidget *dlgPrintGraphProps ;
  GtkWidget *chkPrintClr ;
  } print_graph_properties_D ;

static print_graph_properties_D print_graph_props = {NULL} ;

static void create_print_graph_properties_dialog (print_graph_properties_D *dialog, print_OP *pPO) ;
static void graphs_preview (GtkWidget *widget, gpointer user_data) ;

gboolean get_print_graph_properties_from_user (GtkWindow *parent, print_graph_OP *pPO, simulation_data *sim_data)
  {
  gboolean bRet = FALSE ;

  if (NULL == print_graph_props.dlgPrintGraphProps)
    create_print_graph_properties_dialog (&print_graph_props, &(pPO->po)) ;
  
  g_object_set_data (G_OBJECT (print_graph_props.dlgPrintGraphProps), "sim_data", sim_data) ;
  
  gtk_window_set_transient_for (GTK_WINDOW (print_graph_props.dlgPrintGraphProps), parent) ;
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (print_graph_props.chkPrintClr), pPO->bPrintClr) ;
  
  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (print_graph_props.dlgPrintGraphProps)))))
    init_print_graph_options (pPO, sim_data) ;
  
  gtk_widget_hide (print_graph_props.dlgPrintGraphProps) ;

  return bRet ;
  }

static void create_print_graph_properties_dialog (print_graph_properties_D *dialog, print_OP *pPO)
  {
  GtkWidget *tbl = NULL, *widget = NULL ;
  
  
  dialog->dlgPrintGraphProps = print_dialog_new (pPO) ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlgPrintGraphProps), "Printer Setup") ;
  
  widget = tbl = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;
  
  widget = dialog->chkPrintClr = gtk_check_button_new_with_label ("Print Colours") ;
  gtk_widget_show (widget) ;
  gtk_table_attach (GTK_TABLE (tbl), widget, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  print_dialog_add_page (PRINT_DIALOG (dialog->dlgPrintGraphProps), tbl, "Document Options") ;

  g_signal_connect (G_OBJECT (dialog->dlgPrintGraphProps), "preview", (GCallback)graphs_preview, dialog->dlgPrintGraphProps) ;
  }

void init_print_graph_options (print_graph_OP *pPO, simulation_data *sim_data)
  {
  if (NULL == print_graph_props.dlgPrintGraphProps)
    create_print_graph_properties_dialog (&print_graph_props, &(pPO->po)) ;

  print_dialog_get_options (PRINT_DIALOG (print_graph_props.dlgPrintGraphProps), &(pPO->po)) ;
  
  pPO->bPrintClr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (print_graph_props.chkPrintClr)) ;
  }

static void graphs_preview (GtkWidget *widget, gpointer user_data)
  {
  print_graph_OP po = {{0, 0, 0, 0, 0, 0, FALSE, NULL}, TRUE, FALSE, TRUE, 1, 1} ;
  simulation_data *sim_data = (simulation_data *)g_object_get_data (G_OBJECT (user_data), "sim_data") ;
  
  init_print_graph_options (&po, sim_data) ;
  
  do_print_preview ((print_OP *)&po, GTK_WINDOW (user_data), sim_data, (PrintFunction)print_graphs) ;
  }
