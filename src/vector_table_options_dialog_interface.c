#include "intl.h"
#include "global_consts.h"
#include "bus_layout_dialog.h"
#include "qcadstock.h"
#include "objects/QCADScrolledWindow.h"
#include "vector_table_options_dialog_interface.h"
#include "vector_table_options_dialog_callbacks.h"

static char *vector_table_options_dialog_ui_xml =
  "<ui>"
    "<menubar>"
      "<menu name=\"FileMenu\" action=\"FileMenuAction\">"
#ifdef STDIO_FILEIO
        "<menuitem name=\"FileOpen\" action=\"FileOpenAction\"/>"
        "<separator/>"
        "<menuitem name=\"FileSave\" action=\"FileSaveAction\"/>"
        "<separator/>"
#endif /* def STDIO_FILEIO */
        "<menuitem name=\"FileClose\" action=\"FileCloseAction\"/>"
      "</menu>"
      "<menu name=\"ToolsMenu\" action=\"ToolsMenuAction\">"
          "<menuitem name=\"Exhaustive\" action=\"ExhaustiveAction\"/>"
          "<menuitem name=\"VectorTable\" action=\"VectorTableAction\"/>"
        "<separator/>"
        "<menuitem name=\"AddVector\" action=\"AddVectorAction\"/>"
        "<menuitem name=\"InsertVector\" action=\"InsertVectorAction\"/>"
        "<menuitem name=\"DeleteVector\" action=\"DeleteVectorAction\"/>"
      "</menu>"
    "</menubar>"
    "<toolbar name=\"MainToolBar\" action=\"MainToolBarAction\">"
      "<placeholder name=\"ToolItems\">"
        "<separator/>"
        "<toolitem name=\"FileClose\" action=\"FileCloseAction\"/>"
        "<separator/>"
#ifdef STDIO_FILEIO
        "<toolitem name=\"FileOpen\" action=\"FileOpenAction\"/>"
        "<separator/>"
        "<toolitem name=\"FileSave\" action=\"FileSaveAction\"/>"
        "<separator/>"
#endif /* def STDIO_FILEIO */
        "<toolitem name=\"Exhaustive\" action=\"ExhaustiveAction\"/>"
        "<toolitem name=\"VectorTable\" action=\"VectorTableAction\"/>"
        "<separator/>"
      "</placeholder>"
    "</toolbar>"
    "<toolbar name=\"VectorToolBar\" action=\"VectorToolBarAction\">"
      "<placeholder name=\"ToolItems\">"
        "<toolitem name=\"AddVector\" action=\"AddVectorAction\"/>"
        "<toolitem name=\"InsertVector\" action=\"InsertVectorAction\"/>"
        "<toolitem name=\"DeleteVector\" action=\"DeleteVectorAction\"/>"
      "</placeholder>"
    "</toolbar>"
  "</ui>" ;

static GtkActionEntry menu_action_entries[] =
  {
  {"FileMenuAction",     NULL,                         N_("_File")},
  {"ToolsMenuAction",    NULL,                         N_("_Tools")}
  } ;
static int n_menu_action_entries = G_N_ELEMENTS (menu_action_entries) ;

static GtkActionEntry vector_action_entries[] =
  {
  {"FileOpenAction",     GTK_STOCK_OPEN,               NULL,                NULL, NULL,                (GCallback)vtod_actOpen_activate},
  {"FileSaveAction",     GTK_STOCK_SAVE,               NULL,                NULL, NULL,                (GCallback)vtod_actSave_activate},
  {"AddVectorAction",    GTK_STOCK_ADD,                N_("Add Vector"),    NULL, N_("Add Vector"),    (GCallback)vtod_actAdd_activate},
  {"InsertVectorAction", QCAD_STOCK_INSERT_COL_BEFORE, N_("Insert Vector"), NULL, N_("Insert Vector"), (GCallback)vtod_actInsert_activate},
  {"DeleteVectorAction", GTK_STOCK_DELETE,             N_("Delete Vector"), NULL, N_("Delete Vector"), (GCallback)vtod_actDelete_activate}
  } ;
static int n_vector_action_entries = G_N_ELEMENTS (vector_action_entries) ;

static GtkActionEntry sim_type_action_entries[] =
  {
  {"FileCloseAction", GTK_STOCK_CLOSE, NULL, NULL, NULL, (GCallback)vtod_actClose_activate},
  } ;
static int n_sim_type_action_entries = G_N_ELEMENTS (sim_type_action_entries) ;

static GtkRadioActionEntry sim_type_radio_action_entries[] =
  {
  {"ExhaustiveAction",  QCAD_STOCK_EXHAUSTIVE_SIM,    N_("Exhaustive"),   NULL, N_("Exhaustive Verification"), EXHAUSTIVE_VERIFICATION},
  {"VectorTableAction", QCAD_STOCK_VECTOR_TABLE_SIM,  N_("Vector Table"), NULL, N_("Vector Table Simulation"), VECTOR_TABLE}
  } ;
static int n_sim_type_radio_action_entries = G_N_ELEMENTS (sim_type_radio_action_entries) ;

static void force_adj_to_upper (GtkAdjustment *adj, gpointer data) ;

void create_vector_table_options_dialog (vector_table_options_D *dialog)
  {
  GError *error = NULL ;
  GtkUIManager *ui_mgr = NULL ;
  GtkActionGroup *actions = NULL ;
  GtkWidget *tbl = NULL, *frm = NULL, *status_table = NULL ;
  GtkTreeViewColumn *col = NULL ;
  GtkCellRenderer *cr = NULL ;
  GtkAdjustment *fake_hadj = NULL, *hadj = NULL ;

  dialog->dialog = g_object_new (GTK_TYPE_WINDOW, "type",  GTK_WINDOW_TOPLEVEL, "modal", TRUE, "title", _("Vector Table Setup"), "resizable", TRUE, NULL) ;

  tbl = g_object_new (GTK_TYPE_TABLE, "visible", TRUE, "n-columns", 2, "n-rows", 4, "homogeneous", FALSE) ;
  gtk_container_add (GTK_CONTAINER (dialog->dialog), tbl) ;

  ui_mgr = gtk_ui_manager_new () ;
  actions = gtk_action_group_new ("MenuActions") ;
  gtk_action_group_set_translation_domain (actions, PACKAGE) ;
  gtk_action_group_add_actions (actions, menu_action_entries, n_menu_action_entries, dialog) ;
  gtk_ui_manager_insert_action_group (ui_mgr, actions, -1) ;

  actions = gtk_action_group_new ("SimTypeActions") ;
  gtk_action_group_set_translation_domain (actions, PACKAGE) ;
  gtk_action_group_add_actions (actions, sim_type_action_entries, n_sim_type_action_entries, dialog) ;
  gtk_action_group_add_radio_actions (actions, sim_type_radio_action_entries, n_sim_type_radio_action_entries, EXHAUSTIVE_VERIFICATION, (GCallback)vtod_actSimType_changed, dialog) ;
  gtk_ui_manager_insert_action_group (ui_mgr, actions, -1) ;

  actions =
  dialog->vector_table_action_group = gtk_action_group_new ("VectorTableActions") ;
  gtk_action_group_set_translation_domain (actions, PACKAGE) ;
  gtk_action_group_add_actions (actions, vector_action_entries, n_vector_action_entries, dialog) ;
  gtk_ui_manager_insert_action_group (ui_mgr, actions, -1) ;

  gtk_ui_manager_add_ui_from_string (ui_mgr, vector_table_options_dialog_ui_xml, -1, &error) ;

  if (error != NULL)
    {
    g_message ("Failed to create UI: %s\n", error->message) ;
    g_error_free (error) ;
    }
  else
    {
    GtkWidget *widget = NULL ;

    widget = gtk_ui_manager_get_widget (ui_mgr, "/ui/menubar") ;
    gtk_widget_show (widget) ;
    gtk_table_attach (GTK_TABLE (tbl), widget, 0, 2, 0, 1, 
      (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
      (GtkAttachOptions)(GTK_FILL), 0, 0) ;

    widget = gtk_ui_manager_get_widget (ui_mgr, "/ui/MainToolBar") ;
    gtk_widget_show (widget) ;
    gtk_table_attach (GTK_TABLE (tbl), widget, 0, 2, 1, 2, 
      (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
      (GtkAttachOptions)(GTK_FILL), 0, 0) ;

    dialog->vector_ops = 
    widget = gtk_ui_manager_get_widget (ui_mgr, "/ui/VectorToolBar") ;
    g_object_set (G_OBJECT (widget), 
      "visible", TRUE, "toolbar-style", GTK_TOOLBAR_ICONS, "orientation", GTK_ORIENTATION_VERTICAL, NULL) ;
    gtk_table_attach (GTK_TABLE (tbl), widget, 0, 1, 2, 3, 
      (GtkAttachOptions)(GTK_FILL),
      (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 0, 0) ;

    dialog->actExhaustive  = gtk_ui_manager_get_action (ui_mgr, "/ui/menubar/ToolsMenu/Exhaustive") ;
    dialog->actVectorTable = gtk_ui_manager_get_action (ui_mgr, "/ui/menubar/ToolsMenu/VectorTable") ;

    g_object_set_data (G_OBJECT (gtk_ui_manager_get_action (ui_mgr, "/ui/menubar/FileMenu/FileClose")), "dlgVTO", dialog->dialog) ;
    }

  gtk_window_add_accel_group (GTK_WINDOW  (dialog->dialog), gtk_ui_manager_get_accel_group (ui_mgr)) ;

  dialog->sw = g_object_new (QCAD_TYPE_SCROLLED_WINDOW, "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
    "vscrollbar_policy", GTK_POLICY_AUTOMATIC,          "custom-hadjustment", TRUE,
    "visible",           TRUE,                          "shadow_type",        GTK_SHADOW_IN,
    NULL) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->sw, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND), 2, 2) ;

  dialog->tv = create_bus_layout_tree_view (TRUE, _("Inputs"), GTK_SELECTION_SINGLE) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tv), col = gtk_tree_view_column_new ()) ;
  gtk_tree_view_column_set_title (col, _("Active")) ;
  gtk_tree_view_column_pack_start (col, cr = gtk_cell_renderer_toggle_new (), TRUE) ;
  g_object_set (G_OBJECT (cr), "activatable", TRUE, "active", TRUE, NULL) ;
  gtk_tree_view_column_set_cell_data_func (col, cr, vtod_active_flag_data_func, dialog, NULL) ;
  g_signal_connect (G_OBJECT (cr), "toggled", (GCallback)vtod_active_flag_toggled, dialog) ;
  gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tv), col = gtk_tree_view_column_new ()) ;
  gtk_widget_show (dialog->tv) ;
  gtk_container_add (GTK_CONTAINER (dialog->sw), dialog->tv) ;

  g_object_get (G_OBJECT (dialog->tv), "hadjustment", &fake_hadj, NULL) ;
  g_object_get (G_OBJECT (dialog->sw), "hadjustment", &hadj, NULL) ;

  status_table = g_object_new (GTK_TYPE_TABLE, 
    "n-columns", 2, "n-rows", 1, "homogeneous", FALSE, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (tbl), status_table, 0, 2, 3, 4,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL), 0, 2) ;

  frm = g_object_new (GTK_TYPE_FRAME, "shadow-type", GTK_SHADOW_IN, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (status_table), frm, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 1, 0) ;

  dialog->lblVectorCount = g_object_new (GTK_TYPE_LABEL, 
    "label", "0 vectors", "visible", TRUE, "justify", GTK_JUSTIFY_LEFT, 
    "xalign", 0.0, "yalign", 0.5, "xpad", 2, "ypad", 2, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), dialog->lblVectorCount) ;

  frm = g_object_new (GTK_TYPE_FRAME, "shadow-type", GTK_SHADOW_IN, "visible", TRUE, NULL) ;
  gtk_table_attach (GTK_TABLE (status_table), frm, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
    (GtkAttachOptions)(GTK_FILL), 1, 0) ;

  dialog->lblFileName = g_object_new (GTK_TYPE_LABEL,
    "label", "", "visible", TRUE, "justify", GTK_JUSTIFY_LEFT,
    "xalign", 0.0, "yalign", 0.5, "xpad", 2, "ypad", 2, NULL) ;
  gtk_container_add (GTK_CONTAINER (frm), dialog->lblFileName) ;

  g_signal_connect (G_OBJECT (dialog->dialog), "show",            (GCallback)vtod_dialog_show,            dialog) ;
  g_signal_connect (G_OBJECT (dialog->dialog), "delete-event",    (GCallback)vtod_actClose_activate,      NULL) ;
  g_signal_connect (G_OBJECT (dialog->tv),     "size-allocate",   (GCallback)vtod_treeview_size_allocate, dialog) ;
  g_signal_connect (G_OBJECT (dialog->tv),     "focus-out-event", (GCallback)vtod_treeview_focus,         dialog) ;
  g_signal_connect (G_OBJECT (dialog->tv),     "focus-in-event",  (GCallback)vtod_treeview_focus,         dialog) ;
  g_signal_connect (G_OBJECT (hadj),           "value-changed",   (GCallback)vtod_hadj_value_changed,     dialog->tv) ;
  g_signal_connect (G_OBJECT (fake_hadj),      "changed",         (GCallback)force_adj_to_upper,          dialog->tv) ;
  g_signal_connect (G_OBJECT (fake_hadj),      "value-changed",   (GCallback)force_adj_to_upper,          dialog->tv) ;
  g_signal_connect (G_OBJECT (dialog->vector_table_action_group), "notify::sensitive", (GCallback)vector_actions_notify_sensitive, dialog) ;
  }

static void force_adj_to_upper (GtkAdjustment *adj, gpointer data)
  {
  GList *llCols = gtk_tree_view_get_columns (GTK_TREE_VIEW (data)) ;

  if (NULL == g_list_nth (llCols, 3))
    if (adj->value != adj->upper - adj->page_size) gtk_adjustment_set_value (adj, adj->upper - adj->page_size) ;

  g_list_free (llCols) ;
  }
