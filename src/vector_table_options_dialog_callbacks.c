#include <string.h>
#include "intl.h"
#include "design.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "objects/QCADScrolledWindow.h"
#include "objects/QCADCellRendererVT.h"
#include "file_selection_window.h"
#include "vector_table_options_dialog_callbacks.h"
#include "vector_table_options_dialog_callback_helpers.h"
#include "vector_table_options_dialog_interface.h"

#define B_VECTOR_COL_KEY "bVectorCol"
#define TP_KEY "tp"
#define CR_KEY "cr"
#define COL_KEY "col"
#define TREEVIEW_COLUMN_WIDTHS_KEY "treeview_column_widths"

static int tree_view_column_get_width (GtkTreeViewColumn *col) ;
static GtkTreeViewColumn *vector_table_tree_view_column_new (vector_table_options_D *dialog, int idxVector, gboolean bClickable) ;
static void update_treeview (vector_table_options_D *dialog) ;
static void set_cell_data (GtkTreeViewColumn *tvc, GtkCellRenderer *cell, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;
static guint64 calculate_bus_value (VectorTable *pvt, int idx, BUS *bus) ;
static void vector_column_edit (GtkTreeView *tv, GtkWidget *sw, GtkTreePath *tp, int idxDst, gboolean bForward) ;

static void vector_value_editing_started (GtkCellRendererText *cr, GtkCellEditable *editable, char *pszPath, gpointer data) ;
static void vector_cell_editable_move_cursor (GtkEntry *entry, GtkMovementStep movement_step, gint count, gboolean arg3, gpointer data) ;
static void vector_value_edited (GtkCellRendererText *cr, char *pszPath, char *pszNewText, gpointer data) ;
static void vector_column_clicked (GtkObject *obj, gpointer data) ;
static void vector_column_hilight (GtkTreeView *tv, int idxVector) ;

#ifdef STDIO_FILEIO
void vtod_actOpen_activate (GtkAction *action, gpointer data)
  {
  VTL_RESULT vtl_result = VTL_OK ;
  char *pszFName = NULL, *pszOldFName = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;

  fprintf (stderr, "vtod_actOpen_activate\n") ;

  if (NULL == (pszFName = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Open Vector Table"), dialog_data->pvt->pszFName, FALSE)))
    return ;

  pszOldFName = dialog_data->pvt->pszFName ;
  dialog_data->pvt->pszFName = pszFName ;

  vtl_result = VectorTable_load (dialog_data->pvt) ;

  if (VTL_FILE_FAILED == vtl_result || VTL_MAGIC_FAILED == vtl_result)
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = 
      gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
          (VTL_FILE_FAILED == vtl_result 
            ? _("Failed to open vector table file \"%s\"!")
            : _("File \"%s\" does not appear to be a vector table file !")), pszFName))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    g_free (pszFName) ;
    dialog_data->pvt->pszFName = pszOldFName ;

    return ;
    }
  else
    {
    if (VTL_SHORT == vtl_result || VTL_TRUNC == vtl_result)
      {
      GtkWidget *msg = NULL ;

      gtk_dialog_run (GTK_DIALOG (msg = 
        gtk_message_dialog_new (GTK_WINDOW (dialog->dialog), 
          GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, 
            (VTL_SHORT == vtl_result 
              ? _("File \"%s\" contains fewer inputs than the current vector table. Padding with zeroes.")
              : _("File \"%s\" contains more inputs than the current design. Truncating.")), pszFName))) ;

      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }

    g_object_notify (G_OBJECT (dialog->vector_table_action_group), "sensitive") ;

    g_free (pszOldFName) ;
    }
  }
void vtod_actSave_activate (GtkAction *action, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  char *psz = NULL, *psz1 = NULL ;

  fprintf (stderr, "vtod_actSave_activate\n") ;

  if (EXHAUSTIVE_VERIFICATION == (*(dialog_data->sim_type_p))) return ;

  if (NULL == (psz = get_file_name_from_user (GTK_WINDOW (dialog->dialog), _("Save Vector Table"), dialog_data->pvt->pszFName, TRUE)))
    return ;

  psz1 = dialog_data->pvt->pszFName ;
  dialog_data->pvt->pszFName = psz ;

  if (!VectorTable_save (dialog_data->pvt))
    {
    GtkWidget *msg = NULL ;

    gtk_dialog_run (GTK_DIALOG (msg = 
      gtk_message_dialog_new (GTK_WINDOW (data), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
           _("Failed to save vector table file \"%s\"!"), psz))) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    dialog_data->pvt->pszFName = psz1 ;
    g_free (psz) ;
    }

  vtod_reflect_number_of_vectors_changed (dialog, dialog_data) ;
  }
#endif /* def STDIO_FILEIO */

void vector_actions_notify_sensitive (GtkActionGroup *ag, GParamSpec *spec, gpointer data)
  {
  GList *llCols = NULL, *llItr = NULL ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  gboolean bSensitive = FALSE ;

  g_object_get (G_OBJECT (ag), "sensitive", &bSensitive, NULL) ;

  if (NULL != (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv))))
    {
    for (llItr = llCols ; llItr->next != NULL ; llItr = llItr->next)
      if ((gboolean)g_object_get_data (G_OBJECT (llItr->data), B_VECTOR_COL_KEY))
        gtk_tree_view_remove_column (GTK_TREE_VIEW (dialog->tv), llItr->data) ;
    g_list_free (llCols) ;
    }

  g_object_set_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY, (gpointer)(-1)) ;
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tv)), 
    bSensitive ? GTK_SELECTION_SINGLE : GTK_SELECTION_NONE) ;

  update_treeview (dialog) ;

  gtk_adjustment_set_value (gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (dialog->sw)), 0) ;
  vtod_reflect_number_of_vectors_changed (dialog, g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data")) ;
  }

void vtod_actAdd_activate (GtkAction *action, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;

  fprintf (stderr, "vtod_actAdd_activate\n") ;

  if (NULL == dialog_data) return ;

  VectorTable_add_vector (dialog_data->pvt, dialog_data->pvt->vectors->icUsed) ;

  vtod_reflect_number_of_vectors_changed (dialog, dialog_data) ;
  }

void vtod_actInsert_activate (GtkAction *action, gpointer data)
  {
  int idxVector = -1 ;
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;

  fprintf (stderr, "vtod_actInsert_activate\n") ;

  if (NULL == dialog_data) return ;

  VectorTable_add_vector (dialog_data->pvt, (int)g_object_get_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY)) ;

  g_object_set_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY,
    (gpointer)(idxVector = ((int)g_object_get_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY) + 1))) ;
  vector_column_hilight (GTK_TREE_VIEW (dialog->tv), idxVector) ;
  vtod_reflect_number_of_vectors_changed (dialog, dialog_data) ;
//  update_treeview (dialog) ;
  }
void vtod_actDelete_activate (GtkAction *action, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  int idxVector = (int)g_object_get_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY) ;
  GList *llCols = NULL, *llItr = NULL ;
  GtkAdjustment *adj = NULL ;

  if (0 == dialog_data->pvt->vectors->icUsed) return ;

  fprintf (stderr, "vtod_actDelete_activate\n") ;

  VectorTable_del_vector (dialog_data->pvt, idxVector) ;

  if (NULL != (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv))))
    {
    for (llItr = llCols ; llItr->next != NULL ; llItr = llItr->next)
      if ((gboolean)g_object_get_data (G_OBJECT (llItr->data), B_VECTOR_COL_KEY))
        if (idxVector == (int)g_object_get_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY))
          gtk_tree_view_remove_column (GTK_TREE_VIEW (dialog->tv), GTK_TREE_VIEW_COLUMN (llItr->data)) ;
    g_list_free (llCols) ;
    }

  if (NULL != (adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (dialog->sw))))
    gtk_adjustment_value_changed (adj) ;

  g_object_set_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY, 
    (gpointer)(idxVector = MIN (idxVector, dialog_data->pvt->vectors->icUsed - 1))) ;
  vector_column_hilight (GTK_TREE_VIEW (dialog->tv), idxVector) ;
  vtod_reflect_number_of_vectors_changed (dialog, dialog_data) ;

  drain_gtk_events () ;
  }

void vtod_actClose_activate (GtkAction *action, gpointer data)
  {
  GtkWidget *dlgVTO = GTK_WIDGET (g_object_get_data (G_OBJECT (action), "dlgVTO")) ;

  fprintf (stderr, "vtod_actClose_activate\n") ;

  // Since action can be one of actClose (if called via "activate") or the main window
  // (if called via delete_event), we must make sure we have a pointer to the main
  // window.  Since actClose has a data member dlgGraphs and the main window does not,
  // and the main window /is/ action when called via delete_event, the following will
  // make sure that dlgGraphs always points to the main window
  if (NULL == dlgVTO) dlgVTO = GTK_WIDGET (action) ;

  gtk_widget_hide (dlgVTO) ;
  }

void vtod_actSimType_changed (GtkRadioAction *radio_action, GtkRadioAction *current_action, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  fprintf (stderr, "vtod_actSimType_changed\n") ;

  g_print ("radio_action = %s, current_action = %s\n",
    gtk_action_get_name (GTK_ACTION (radio_action)), gtk_action_get_name (GTK_ACTION (current_action))) ;

  g_object_set (G_OBJECT (dialog->vector_table_action_group), "sensitive", 
    VECTOR_TABLE == 
      ((*(((VECTOR_TABLE_OPTIONS_DIALOG_DATA *)g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data"))->sim_type_p)) = 
        (dialog->actVectorTable == GTK_ACTION (current_action) ? VECTOR_TABLE : EXHAUSTIVE_VERIFICATION)), NULL) ;
  }

gboolean vtod_treeview_focus (GtkWidget *widget, GdkEventFocus *event, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;

  vector_column_hilight (GTK_TREE_VIEW (dialog->tv), (int)g_object_get_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY)) ;
  return FALSE ;
  }

void vtod_treeview_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;

  if (NULL == dialog_data) return ;

  update_treeview (dialog) ;
  }

void vtod_hadj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  char *psz = NULL ;
  int Nix ;
  gboolean bDirty = FALSE ;
  GList *llCols = NULL, *llItr = NULL ;
  GtkCellRenderer *cr = NULL ;

  if (NULL == (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (data)))) return ;
  if (NULL == llCols->next) return ;
  if (NULL == (llItr = llCols->next->next)) return ;

  for (Nix = adj->value; llItr->next != NULL ; llItr = llItr->next, Nix++)
    if (((int)g_object_get_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY)) != Nix)
      {
      bDirty = TRUE ;
      g_object_set_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY, (gpointer)Nix) ;
      if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), CR_KEY)))
        g_object_set_data (G_OBJECT (cr), IDX_VECTOR_KEY, (gpointer)Nix) ;
      gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (llItr->data), psz = g_strdup_printf ("%d", Nix)) ;
      g_free (psz) ;
      }

  if (bDirty)
    {
    gtk_tree_view_columns_autosize (GTK_TREE_VIEW (data)) ;
    vector_column_hilight (GTK_TREE_VIEW (data), (int)g_object_get_data (G_OBJECT (data), IDX_VECTOR_KEY)) ;
    }
  }

static void set_cell_data (GtkTreeViewColumn *tvc, GtkCellRenderer *cell, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ; ;
  int row_type = ROW_TYPE_CLOCK, idx = -1, idxVector = -1, idx_dfs = -1 ;

  idxVector = (int)g_object_get_data (G_OBJECT (tvc), IDX_VECTOR_KEY) ;
  gtk_tree_model_get (tm, itr, 
    BUS_LAYOUT_MODEL_COLUMN_TYPE,      &row_type, 
    BUS_LAYOUT_MODEL_COLUMN_INDEX,     &idx, 
    VECTOR_TABLE_MODEL_COLUMN_IDX_DFS, &idx_dfs,
    -1) ;

  if (VECTOR_TABLE == (*(dialog_data->sim_type_p)))
    {
    if (row_type & ROW_TYPE_CELL)
      g_object_set (G_OBJECT (cell), 
        "active",    exp_array_index_2d (dialog_data->pvt->vectors, gboolean, idxVector, idx),
        "row-type",  row_type,
        "sensitive", exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, idx).active_flag,
        "editable",  TRUE,
        "mode",      GTK_CELL_RENDERER_MODE_ACTIVATABLE,
        NULL) ;
    else
      {
      gboolean bSensitive = FALSE ;
      int Nix ;
      char *psz = NULL ;
      BUS *bus = &exp_array_index_1d (dialog_data->bus_layout->buses, BUS, idx) ;

      for (Nix = 0 ; Nix < bus->cell_indices->icUsed ; Nix++)
        if (exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, Nix)).active_flag)
          break ;

      bSensitive = (Nix < bus->cell_indices->icUsed) ;

      g_object_set (G_OBJECT (cell), 
        "text",      psz = g_strdup_printf ("%llu", calculate_bus_value (dialog_data->pvt, idxVector, &exp_array_index_1d (dialog_data->bus_layout->buses, BUS, idx))),
        "row-type",  row_type,
        "sensitive", bSensitive,
        "editable",  bSensitive,
        "mode",      GTK_CELL_RENDERER_MODE_EDITABLE,
        NULL) ;

      g_free (psz) ;
      }
    }
  else
    {
    int value = idxVector % (1 << dialog_data->bus_layout->inputs->icUsed) ;
    if (row_type & ROW_TYPE_CELL)
      g_object_set (G_OBJECT (cell), 
        "active",    (gboolean)((value >> (dialog_data->bus_layout->inputs->icUsed - 1 - idx_dfs)) & 0x1),
        "row-type",  row_type,
        "sensitive", FALSE,
        "editable",  FALSE,
        "mode",      GTK_CELL_RENDERER_MODE_INERT,
        NULL) ;
    else
      {
      char *psz = NULL ;

      g_object_set (G_OBJECT (cell), 
        "text",      psz = g_strdup_printf ("%d", value),
        "row-type",  row_type,
        "sensitive", FALSE,
        "editable",  FALSE,
        "mode",      GTK_CELL_RENDERER_MODE_INERT,
        NULL) ;

      g_free (psz) ;
      }
    }

//  gtk_widget_queue_draw (dialog->tv) ;
  }

static void vector_value_editing_started (GtkCellRendererText *cr, GtkCellEditable *editable, char *pszPath, gpointer data)
  {
  if (GTK_IS_ENTRY (editable))
    {
//    vector_table_options_D *dialog = (vector_table_options_D *)data ;
//    VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
    int idxVector = (int)g_object_get_data (G_OBJECT (cr), IDX_VECTOR_KEY) ;
//    GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tv)) ;
//    GtkTreeIter itr ;
//    int bus_idx = -1 ;

//    gtk_tree_model_get_iter (tm, &itr, tp = gtk_tree_path_new_from_string (pszPath)) ;
//    gtk_tree_path_free (tp) ;

//    gtk_tree_model_get (tm, &itr, BUS_LAYOUT_MODEL_COLUMN_INDEX, &bus_idx, -1) ;

    fprintf (stderr, "editing_started:Setting editable idxVector = %d\n", idxVector) ;
    g_object_set_data (G_OBJECT (editable), IDX_VECTOR_KEY, (gpointer)idxVector) ;
    g_object_set_data_full (G_OBJECT (editable), TP_KEY, gtk_tree_path_new_from_string (pszPath), (GDestroyNotify)gtk_tree_path_free) ;

    if (!g_signal_handler_find (G_OBJECT (editable), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, vector_cell_editable_move_cursor, data))
      g_signal_connect (G_OBJECT (editable), "move-cursor", (GCallback)vector_cell_editable_move_cursor, data) ;
    }
  }

static void vector_cell_editable_move_cursor (GtkEntry *entry, GtkMovementStep movement_step, gint count, gboolean arg3, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  int position = -1 ;
  int idxVector = (int)g_object_get_data (G_OBJECT (entry), IDX_VECTOR_KEY) ;
  gboolean bMoveToPrevColumn = FALSE ; // TRUE <=> move to prev col and FALSE <=> move to next col

  g_object_get (G_OBJECT (entry), "cursor-position", &position, NULL) ;

  // If the insertion point is bumping into the text box edge
  if ((bMoveToPrevColumn = (0 == position && count < 0 && idxVector > 0)) ||
      (strlen (gtk_entry_get_text (entry)) == position && count > 0 && idxVector < dialog_data->pvt->vectors->icUsed - 1))
    {
    GtkTreePath *tp = g_object_get_data (G_OBJECT (entry), TP_KEY) ;
    GList *llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv)),
          *llThis = NULL,
          *llDst = NULL ;

    if (NULL == llCols) return ;

    for (llThis = llCols ; llThis->next != NULL ; llThis = llThis->next)
      if ((gboolean)g_object_get_data (G_OBJECT (llThis->data), B_VECTOR_COL_KEY))
        if (idxVector == (int)g_object_get_data (G_OBJECT (llThis->data), IDX_VECTOR_KEY))
          break ;

    g_print ("I should be moving to the %s column (idxVector is %d, llThis is %sNULL)\n", bMoveToPrevColumn ? "previous" : "next", idxVector, NULL == llThis ? "" : "not ") ;

    if (NULL == llThis) return ;

    gtk_cell_editable_editing_done (GTK_CELL_EDITABLE (entry)) ;
    gtk_cell_editable_remove_widget (GTK_CELL_EDITABLE (entry)) ;

    llDst = (bMoveToPrevColumn ? llThis->prev : llThis->next) ;

    if (NULL != llDst)
      {
      if ((gboolean)g_object_get_data (G_OBJECT (llDst->data), B_VECTOR_COL_KEY))
        gtk_tree_view_set_cursor (GTK_TREE_VIEW (dialog->tv), tp, GTK_TREE_VIEW_COLUMN (llDst->data), TRUE) ;
      else
        vector_column_edit (GTK_TREE_VIEW (dialog->tv), dialog->sw, tp, idxVector + (bMoveToPrevColumn ? -1 : 1), FALSE) ;
      }
    else // This is the last visible column, so the next is not present - scroll forward
        vector_column_edit (GTK_TREE_VIEW (dialog->tv), dialog->sw, tp, idxVector + (bMoveToPrevColumn ? -1 : 1), TRUE) ;
    if (NULL != llCols)
      g_list_free (llCols) ;
    }
  }

static void vector_value_edited (GtkCellRendererText *cr, char *pszPath, char *pszNewText, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  int idxVector = (int)g_object_get_data (G_OBJECT (cr), IDX_VECTOR_KEY) ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tv)) ;
  GtkTreeIter itr ;
  int row_type = ROW_TYPE_CLOCK, idx = -1 ;

  if (!gtk_tree_model_get_iter_from_string (tm, &itr, pszPath)) return ;

  gtk_tree_model_get (tm, &itr,
    BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx,
    -1) ;

  if (row_type & ROW_TYPE_CELL)
    g_object_get (G_OBJECT (cr), "active", &exp_array_index_2d (dialog_data->pvt->vectors, gboolean, idxVector, idx), NULL) ;
  else  
    {
    int Nix ;
    guint64 new_value = g_ascii_strtoull (pszNewText, NULL, 10) ;
    BUS *bus = &exp_array_index_1d (dialog_data->bus_layout->buses, BUS, idx) ;

    for (Nix = bus->cell_indices->icUsed - 1 ; Nix > -1 ; Nix--)
      if (exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, Nix)).active_flag)
        {
        exp_array_index_2d (dialog_data->pvt->vectors, gboolean, idxVector, exp_array_index_1d (bus->cell_indices, int, Nix)) = (gboolean)(new_value & 0x1) ;
        new_value = new_value >> 1 ;
        }
    }

  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (dialog->tv)) ;
  // Wait for all columns to get resized
//  g_print ("Waiting for columns to be resized\n") ;
  drain_gtk_events () ;
//  g_print ("Columns resized\n") ;
//  update_treeview (dialog) ;
  }

static void vector_column_clicked (GtkObject *obj, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  GtkTreeViewColumn *col = NULL ;
  int idx = -1 ;

  if (NULL == (col = GTK_IS_TREE_VIEW_COLUMN (obj) 
                 ? GTK_TREE_VIEW_COLUMN (obj) 
                 : g_object_get_data (G_OBJECT (obj), COL_KEY))) return ;

  idx = (int)g_object_get_data (G_OBJECT (col), IDX_VECTOR_KEY) ;

  g_object_set_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY, (gpointer)idx) ;
  vector_column_hilight (GTK_TREE_VIEW (dialog->tv), idx) ;
  vtod_reflect_number_of_vectors_changed (dialog, g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data")) ;
  }

void vtod_active_flag_data_func (GtkTreeViewColumn *col, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data)
  {
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (((vector_table_options_D *)data)->dialog), "vector_table_options_dialog_data") ;
  int idx = -1 ;
  int row_type = ROW_TYPE_CLOCK ;

  gtk_tree_model_get (tm, itr,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx,
    BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type, -1) ;

  if (VECTOR_TABLE == (*(dialog_data->sim_type_p)))
    {
    if (ROW_TYPE_CELL & row_type)
      g_object_set (G_OBJECT (cr), 
        "inconsistent", FALSE,
        "active", exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, idx).active_flag, 
        "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
#if (GTK_MINOR_VERSION > 4)
        "sensitive", TRUE,
#endif /* (GTK_MINOR_VERSION > 4) */
        NULL) ;
    else
      {
      int Nix ;
      gboolean bActive = FALSE, bInconsistent = FALSE ;
      BUS *bus = &exp_array_index_1d (dialog_data->bus_layout->buses, BUS, idx) ;

      if (0 == bus->cell_indices->icUsed)
        // We should never get here (TM)
        return ;
      bActive = exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, 0)).active_flag ;
      for (Nix = 1 ; Nix < bus->cell_indices->icUsed ; Nix++)
        if ((bInconsistent = exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, Nix)).active_flag != bActive))
          break ;

      g_object_set (G_OBJECT (cr), 
        "active", bActive, 
        "inconsistent", bInconsistent, 
        "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
#if (GTK_MINOR_VERSION > 4)
        "sensitive", TRUE,
#endif /* (GTK_MINOR_VERSION > 4) */
        NULL) ;
      }
    }
  else
    g_object_set (G_OBJECT (cr), "inconsistent", FALSE, "active", TRUE,
#if (GTK_MINOR_VERSION > 4)
      "sensitive", FALSE,
#endif /* (GTK_MINOR_VERSION > 4) */
      "mode", GTK_CELL_RENDERER_MODE_INERT, NULL) ;
  }

void vtod_active_flag_toggled (GtkCellRendererToggle *cr, char *pszPath, gpointer data)
  {
  vector_table_options_D *dialog = (vector_table_options_D *)data ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->tv)) ;
  GtkTreeIter itr ;
  int row_type = ROW_TYPE_CLOCK, idx = -1 ;

  if (!gtk_tree_model_get_iter_from_string (tm, &itr, pszPath)) return ;

  gtk_tree_model_get (tm, &itr,
    BUS_LAYOUT_MODEL_COLUMN_TYPE, &row_type,
    BUS_LAYOUT_MODEL_COLUMN_INDEX, &idx,
    -1) ;

  if (ROW_TYPE_CELL & row_type)
    exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, idx).active_flag = 
      !exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, idx).active_flag ;
  else
    {
    int Nix ;
    gboolean bTurnVectorsOn = FALSE, bInconsistent, bActive ;
    BUS *bus = &exp_array_index_1d (dialog_data->bus_layout->buses, BUS, idx) ;

    g_object_get (G_OBJECT (cr), "inconsistent", &bInconsistent, "active", &bActive, NULL) ;

    bTurnVectorsOn = bInconsistent ? bActive : !bActive ;

    for (Nix = 0 ; Nix < bus->cell_indices->icUsed ; Nix++)
      exp_array_index_1d (dialog_data->pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, Nix)).active_flag = bTurnVectorsOn ;
    }

  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (dialog->tv)) ;
  }

static GtkTreeViewColumn *vector_table_tree_view_column_new (vector_table_options_D *dialog, int idxVector, gboolean bClickable)
  {
  char *psz = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkTreeViewColumn *col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, 
    "title", psz = g_strdup_printf ("%d", idxVector), "clickable", bClickable, "visible", TRUE, NULL) ;

  g_free (psz) ;
  g_object_set_data (G_OBJECT (col), IDX_VECTOR_KEY, (gpointer)idxVector) ;
  g_object_set_data (G_OBJECT (col), B_VECTOR_COL_KEY, (gpointer)TRUE) ;
  gtk_tree_view_column_pack_start (col, cr = qcad_cell_renderer_vt_new (), TRUE) ;
  g_object_set_data (G_OBJECT (col), CR_KEY, cr) ;
  g_object_set_data (G_OBJECT (cr), IDX_VECTOR_KEY, (gpointer)idxVector) ;
  g_object_set_data (G_OBJECT (cr), COL_KEY, col) ;
  g_object_set (G_OBJECT (cr), 
    "editable",            TRUE, 
//    "cell-background-gdk", &((gtk_widget_get_style (dialog->tv))->base[3]),
//    "cell-background-set", FALSE, 
    NULL) ;
  gtk_tree_view_column_set_cell_data_func (col, cr, set_cell_data, dialog, NULL) ;

  g_signal_connect (G_OBJECT (col), "clicked",         (GCallback)vector_column_clicked,        dialog) ;
  g_signal_connect (G_OBJECT (cr),  "toggled",         (GCallback)vector_column_clicked,        dialog) ;
  g_signal_connect (G_OBJECT (cr),  "edited",          (GCallback)vector_value_edited,          dialog) ;
  g_signal_connect (G_OBJECT (cr),  "editing-started", (GCallback)vector_value_editing_started, dialog) ;

  return col ;
  }

static int tree_view_column_get_width (GtkTreeViewColumn *col)
  {
  int col_width = 0 ;

  if (NULL == col) return 0 ;

  col_width = MAX (col->requested_width, col->button_request) ;
  if (-1 != col->min_width)
    col_width = MAX (col_width, col->min_width) ;
  if (-1 != col->max_width)
    col_width = MIN (col_width, col->max_width) ;

  return col_width ;
  }

static void update_treeview (vector_table_options_D *dialog)
  {
  int icVectorCols = 0, cxLastWidth = 0, idxLastCol = -1, idxFirstCol = -1 ;
  GdkRectangle rc ;
  GList *llCols = NULL, *llItr = NULL, *llFirstVectorCol = NULL, *llLastVectorCol = NULL ;
  GtkTreeViewColumn *col = NULL ;
  VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data = g_object_get_data (G_OBJECT (dialog->dialog), "vector_table_options_dialog_data") ;
  GtkAdjustment *adj = NULL ;
  double old_value, old_upper, old_page_size ;
  gboolean bIncreasedLast = TRUE ;
  int number_of_vectors = 
    VECTOR_TABLE == (*(dialog_data->sim_type_p)) 
      ? dialog_data->pvt->vectors->icUsed 
      : dialog_data->bus_layout->inputs->icUsed > 0
        ? (1 << (dialog_data->bus_layout->inputs->icUsed + 1))
        : 0 ;
  EXP_ARRAY *column_widths = NULL ;

  gtk_tree_view_get_visible_rect (GTK_TREE_VIEW (dialog->tv), &rc) ;
  g_object_get (G_OBJECT (dialog->sw), "hadjustment", &adj, NULL) ;

  if (NULL == (column_widths = g_object_get_data (G_OBJECT (dialog->tv), TREEVIEW_COLUMN_WIDTHS_KEY)))
    {
    column_widths = exp_array_new (sizeof (int), 1) ;
    g_object_set_data_full (G_OBJECT (dialog->tv), TREEVIEW_COLUMN_WIDTHS_KEY, column_widths, (GDestroyNotify)exp_array_free) ;
    }

  if (column_widths->icUsed != number_of_vectors)
    {
    int Nix ;

    exp_array_remove_vals (column_widths, 1, 0, column_widths->icUsed) ;
    exp_array_1d_insert_vals (column_widths, NULL, number_of_vectors, 0) ;
    for (Nix = 0 ; Nix < number_of_vectors ; Nix++)
      exp_array_index_1d (column_widths, int, Nix) = 0 ;
    }

  if (GTK_WIDGET_REALIZED (dialog->sw))
    drain_gtk_events () ;

  if (NULL == (llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv)))) return ;

  if (NULL != GTK_WIDGET (dialog->sw)->window)
    gdk_window_freeze_updates (GTK_WIDGET (dialog->sw)->window) ;

  if (NULL != llCols->next)
    if (NULL != (llFirstVectorCol = llCols->next->next))
      if (NULL == llFirstVectorCol->next)
        {
        if (NULL != llFirstVectorCol->prev)
          {
          if ((gboolean)g_object_get_data (G_OBJECT (llFirstVectorCol->prev->data), B_VECTOR_COL_KEY))
            llFirstVectorCol = llFirstVectorCol->prev ;
          else
            llFirstVectorCol = NULL ;
          }
        else
          llFirstVectorCol = NULL ;
        }

  // Remove width of main column
  rc.width -= tree_view_column_get_width (GTK_TREE_VIEW_COLUMN (llCols->data)) ;
  if (NULL != (llItr = llCols->next))
    {
    // Remove width of "active" column
    rc.width -= tree_view_column_get_width (GTK_TREE_VIEW_COLUMN (llItr->data)) ;
    if (NULL != (llItr = llItr->next))
      for (; llItr->next != NULL ; llItr = llItr->next)
        {
        rc.width -= 
        (exp_array_index_1d (column_widths, int, (int)g_object_get_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY)) =
        tree_view_column_get_width (GTK_TREE_VIEW_COLUMN (llItr->data))) ;
        // If this column disappears into the right margin, remove the rest of the columns
        if (rc.width < 0 && llItr != llFirstVectorCol)
          {
          for (; llItr->next != NULL ; llItr = llItr->next)
            gtk_tree_view_remove_column (GTK_TREE_VIEW (dialog->tv), GTK_TREE_VIEW_COLUMN (llItr->data)) ;

          // Make sure the columns are removed and all
          g_signal_handlers_block_matched (G_OBJECT (dialog->tv), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_treeview_size_allocate, NULL) ;
//          g_print ("Waiting for columns to be removed\n") ;
          if (GTK_WIDGET_REALIZED (dialog->tv))
            drain_gtk_events () ;
//          g_print ("Columns removed\n") ;
          g_signal_handlers_unblock_matched (G_OBJECT (dialog->tv), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_treeview_size_allocate, NULL) ;

          break ;
          }
        // Otherwise, chalk it up as a vector-displaying column
        icVectorCols++ ;
        }
    }

  g_list_free (llCols) ;

  llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (dialog->tv)) ;

  if (NULL != llCols->next)
    if (NULL != (llFirstVectorCol = llCols->next->next))
      if (NULL == llFirstVectorCol->next)
        {
        if (NULL != llFirstVectorCol->prev)
          {
          if ((gboolean)g_object_get_data (G_OBJECT (llFirstVectorCol->prev->data), B_VECTOR_COL_KEY))
            llFirstVectorCol = llFirstVectorCol->prev ;
          else
            llFirstVectorCol = NULL ;
          }
        else
          llFirstVectorCol = NULL ;
        }

  if (NULL != llFirstVectorCol)
    {
    if (NULL != (llLastVectorCol = g_list_last (llFirstVectorCol)))
      llLastVectorCol = llLastVectorCol->prev ;
    idxFirstCol = (int)g_object_get_data (G_OBJECT (llFirstVectorCol->data), IDX_VECTOR_KEY) ;
    }
  if (NULL != llLastVectorCol)
    {
    idxLastCol = (int)g_object_get_data (G_OBJECT (llLastVectorCol->data), IDX_VECTOR_KEY) ;
    cxLastWidth = 
      exp_array_index_1d (column_widths, int, idxLastCol) =
      tree_view_column_get_width (GTK_TREE_VIEW_COLUMN (llLastVectorCol->data)) ;
    }

  // if there is room left
  if (rc.width > 0 || (NULL == llFirstVectorCol && 0 == icVectorCols))
    {
    // if there are more vectors we could display
    if (icVectorCols < number_of_vectors)
      {
      while (rc.width > cxLastWidth || (NULL == llFirstVectorCol && 0 == icVectorCols))
        {
        // If we can display more vectors towards the end, let's
        if (idxLastCol + 1 < number_of_vectors)
          {
          bIncreasedLast = TRUE ;
          idxLastCol++ ;
          }
        else
        if (idxFirstCol - 1 > -1)
          {
          bIncreasedLast = FALSE ;
          idxFirstCol-- ;
          }
        else 
          break ;

        col = NULL ;
        if (rc.width > exp_array_index_1d (column_widths, int, bIncreasedLast ? idxLastCol : idxFirstCol) || (NULL == llFirstVectorCol && 0 == icVectorCols))
          {
          gtk_tree_view_insert_column (GTK_TREE_VIEW (dialog->tv), 
            col = vector_table_tree_view_column_new (dialog, idxLastCol, (VECTOR_TABLE == (*(dialog_data->sim_type_p)))), 
            icVectorCols + 2) ;

          // Make sure the column is added and displayed ... (?)
          g_signal_handlers_block_matched (G_OBJECT (dialog->tv), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_treeview_size_allocate, NULL) ;
//          g_print ("Waiting for columns to be displayed\n") ;
          if (GTK_WIDGET_REALIZED (dialog->tv))
            drain_gtk_events () ;
//          g_print ("Columns displayed\n") ;
          g_signal_handlers_unblock_matched (G_OBJECT (dialog->tv), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_treeview_size_allocate, NULL) ;
          }
        else
          {
          // If we can't display more vectors towards the end, let's not
          if (bIncreasedLast)
            idxLastCol-- ;
          else
            idxFirstCol++ ;
          }

        if (NULL != col)
          {
          rc.width -= 
            (cxLastWidth = 
             exp_array_index_1d (column_widths, int, idxLastCol) = tree_view_column_get_width (col)) ;

          icVectorCols++ ;

          // If this is the first vector we've inserted
          if (NULL == llFirstVectorCol)
            idxFirstCol = 0 ;
          }
        else
          break ;
        }
      }
    }

  g_list_free (llCols) ;

  old_value = adj->value ;
  old_upper = adj->upper ;
  old_page_size = adj->page_size ;

  adj->lower = 0 ;
  adj->value = idxFirstCol ;
  adj->upper = number_of_vectors ;
  adj->step_increment = 1 ;
  adj->page_increment = 10 ;
  adj->page_size = idxLastCol - idxFirstCol + 1 ;

//  g_signal_handlers_block_matched (G_OBJECT (adj), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_hadj_value_changed, NULL) ;
  if (!((adj->upper == old_upper) && (adj->page_size == old_page_size)))
    gtk_adjustment_changed (adj) ;
  if (old_value != adj->value)
    gtk_adjustment_value_changed (adj) ;
//  g_signal_handlers_unblock_matched (G_OBJECT (adj), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (GCallback)vtod_hadj_value_changed, NULL) ;

  if (NULL != GTK_WIDGET (dialog->sw)->window)
    gdk_window_thaw_updates (GTK_WIDGET (dialog->sw)->window) ;
  }

static guint64 calculate_bus_value (VectorTable *pvt, int idx, BUS *bus)
  {
  guint64 ret = 0 ;
  int Nix ;

  for (Nix = 0 ; Nix < bus->cell_indices->icUsed && Nix < sizeof (guint64) * 8 ; Nix++)
    if (exp_array_index_1d (pvt->inputs, VT_INPUT, exp_array_index_1d (bus->cell_indices, int, Nix)).active_flag)
      {
      ret = ret << 1 ;
      if (exp_array_index_2d (pvt->vectors, gboolean, idx, exp_array_index_1d (bus->cell_indices, int, Nix)))
        ret |= 0x1 ;
      }

  return ret ;
  }

static void vector_column_edit (GtkTreeView *tv, GtkWidget *sw, GtkTreePath *tp, int idxDst, gboolean bForward)
  {
  GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw)) ;
  double old_value = adj->value ;

  if (NULL != sw->window)
    gdk_window_freeze_updates (sw->window) ;
  while (TRUE)
    {
    // Determine new adjustment value
    if (bForward)
      adj->value = MIN (adj->value + 1, adj->upper - adj->page_size) ;
    else
      adj->value = MAX (adj->value - 1, 0) ;

    if (adj->value != old_value)
      {
      GList *llItr = NULL, *llCols = NULL ;

      gtk_adjustment_value_changed (adj) ;
      // Wait for adjustment changes to take effect
//      g_print ("Waiting for adjustment changes to take effect\n") ;
      drain_gtk_events () ;
//      g_print ("Adjustment changes taken effect\n") ;

      llCols = gtk_tree_view_get_columns (tv) ;
      for (llItr = g_list_last (llCols) ; llItr != NULL ; llItr = llItr->prev)
        if ((gboolean)g_object_get_data (G_OBJECT (llItr->data), B_VECTOR_COL_KEY))
          if (idxDst == (int)g_object_get_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY))
            {
            g_print ("Editing newly uncovered column!\n") ;
            gtk_tree_view_set_cursor (tv, tp, GTK_TREE_VIEW_COLUMN (llItr->data), TRUE) ;
            break ;
            }
      g_list_free (llCols) ;
      // I can still use llItr below, despite freeing llCols, as long as I don't dereference it in any way
      if (llItr == NULL)
        g_print ("Newly uncovered column not found!\n") ;
      else 
        break ;
      }
    else
      break ;
    }

  vector_column_hilight (tv, (int)g_object_get_data (G_OBJECT (tv), IDX_VECTOR_KEY)) ;

  if (NULL != sw->window)
    gdk_window_thaw_updates (sw->window) ;
  }

static void vector_column_hilight (GtkTreeView *tv, int idxVector)
  {
  GtkCellRenderer *cr = NULL ;
  GList *llCols = NULL, *llItr = NULL ;

  if (NULL == (llCols = gtk_tree_view_get_columns (tv))) return ;

  for (llItr = llCols ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (cr = g_object_get_data (G_OBJECT (llItr->data), CR_KEY)))
      g_object_set (G_OBJECT (cr), 
        "active-column", (idxVector == ((int)g_object_get_data (G_OBJECT (llItr->data), IDX_VECTOR_KEY))),
        NULL) ;

  g_list_free (llCols) ;

  gtk_widget_queue_draw (GTK_WIDGET (tv)) ;
  }

void vtod_reflect_number_of_vectors_changed (vector_table_options_D *dialog, VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data)
  {
  char *psz = NULL ;
  GtkAction *action = NULL ;
  int number_of_vectors = 
    VECTOR_TABLE == (*(dialog_data->sim_type_p)) 
      ? dialog_data->pvt->vectors->icUsed 
      : dialog_data->bus_layout->inputs->icUsed > 0
        ? (1 << (dialog_data->bus_layout->inputs->icUsed + 1))
        : 0 ;
  gboolean bHaveColumn = (dialog_data->pvt->vectors->icUsed > 0 && (-1 != (int)g_object_get_data (G_OBJECT (dialog->tv), IDX_VECTOR_KEY))) ;

  if (VECTOR_TABLE == (*(dialog_data->sim_type_p)))
    {
    if (NULL != (action = gtk_action_group_get_action (dialog->vector_table_action_group, "DeleteVectorAction")))
      g_object_set (G_OBJECT (action), "sensitive", bHaveColumn, NULL) ;
    if (NULL != (action = gtk_action_group_get_action (dialog->vector_table_action_group, "InsertVectorAction")))
      g_object_set (G_OBJECT (action), "sensitive", bHaveColumn, NULL) ;
    }
  

  gtk_label_set_text (GTK_LABEL (dialog->lblVectorCount), 
    psz = g_strdup_printf ("%d %s", number_of_vectors, 
      1 == number_of_vectors ? _("vector") : _("vectors"))) ;
  g_free (psz) ;

  gtk_label_set_text (GTK_LABEL (dialog->lblFileName), 
    psz = g_strdup_printf ("%s", 
      (VECTOR_TABLE == (*(dialog_data->sim_type_p)))
        ? (NULL == dialog_data->pvt->pszFName ? "" : dialog_data->pvt->pszFName)
        : _("Exhaustive Verification"))) ;
  g_free (psz) ;

  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (dialog->tv)) ;
  }
