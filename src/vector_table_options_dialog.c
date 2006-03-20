#include "global_consts.h"
#include "custom_widgets.h"
#include "bus_layout_dialog.h"
#include "vector_table_options_dialog.h"
#include "vector_table_options_dialog_interface.h"
#include "vector_table_options_dialog_callback_helpers.h"
#include "vector_table_options_dialog_callbacks.h"

VectorTable *pvt ;

static vector_table_options_D vto = {NULL} ;

static void vector_table_options_dialog_set_data (vector_table_options_D *dialog, VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data) ;

void get_vector_table_options_from_user (GtkWindow *parent, BUS_LAYOUT *bus_layout, int *sim_type, VectorTable *pvt_user)
  {
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = NULL ;

  if (NULL == vto.dialog)
    create_vector_table_options_dialog (&vto) ;

  gtk_window_set_transient_for (GTK_WINDOW (vto.dialog), parent) ;

  if (NULL != (dialog_data = g_malloc0 (sizeof (VECTOR_TABLE_OPTIONS_DIALOG_DATA))))
    {
    dialog_data->bus_layout = bus_layout ;
    dialog_data->sim_type_p = sim_type ;
    dialog_data->pvt        = pvt_user ;

    vector_table_options_dialog_set_data (&vto, dialog_data) ;

    gtk_widget_show (vto.dialog) ;

    while (GTK_WIDGET_VISIBLE (vto.dialog))
      gtk_main_iteration () ;

    if (NULL != parent)
      gtk_window_present (parent) ;
    }
  }

static void vector_table_options_dialog_set_data (vector_table_options_D *dialog, VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data)
  {
  GtkTreeModel *tm = NULL ;

  g_object_set_data_full (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data", dialog_data, (GDestroyNotify)g_free) ;

  if (NULL != (tm = GTK_TREE_MODEL (design_bus_layout_tree_store_new (dialog_data->bus_layout, ROW_TYPE_INPUT, 1, G_TYPE_INT))))
    {
    int idx = 0, row_type ;
    GtkTreeIter itr ;

    if (gtk_tree_model_get_iter_first (tm, &itr))
      do
        {
        gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;
        gtk_tree_store_set (GTK_TREE_STORE (tm), &itr, 
          VECTOR_TABLE_MODEL_COLUMN_IDX_DFS, (ROW_TYPE_BUS & row_type) ? -1 : idx++, 
          -1) ;
        }
      while (gtk_tree_model_iter_next_dfs (tm, &itr)) ;

    gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->tv), tm) ;
    gtk_tree_view_expand_all (GTK_TREE_VIEW (dialog->tv)) ;
    }

  if (0 == dialog_data->pvt->inputs->icUsed)
    (*(dialog_data->sim_type_p)) = EXHAUSTIVE_VERIFICATION ;

  g_object_set (G_OBJECT (dialog->actVectorTable),
    "value",     (*(dialog_data->sim_type_p)), 
    "sensitive", (dialog_data->pvt->inputs->icUsed > 0),
    NULL) ;
  vtod_actSimType_changed (GTK_RADIO_ACTION (dialog->actVectorTable), 
    GTK_RADIO_ACTION (VECTOR_TABLE == (*(dialog_data->sim_type_p)) 
      ? dialog->actVectorTable 
      : dialog->actExhaustive), dialog) ;
  gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 640, 480) ;
  }
