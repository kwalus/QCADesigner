//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////


// -- includes -- //
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <math.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "stdqcell.h"
#include "simulation.h"
#include "cad.h"
#include "fileio.h"
#include "fileio_helpers.h"
#include "print_preview.h"
#include "print.h"
#include "recent_files.h"
#include "vector_table.h"
#include "undo_redo.h"
#include "undo_create.h"
#include "callback_helpers.h"

// dialogs and windows used //
#include "about.h"
#include "file_selection_window.h"
#include "project_properties_dialog.h"
#include "cell_properties_dialog.h"
#include "grid_spacing_dialog.h"
#include "scale_dialog.h"
#include "print_properties_dialog.h"
#include "sim_engine_setup_dialog.h"
#include "clock_select_dialog.h"
#include "sim_type_setup_dialog.h"
#include "graph_dialog.h"
#include "random_fault_setup_dialog.h"
#include "command_history.h"
#include "custom_widgets.h"
#include "scale_dialog.h"
#include "cell_function_dialog.h"

#define DBG_CB(s)
#define DBG_CB_HERE(s)

#define NUMBER_OF_RULER_SUBDIVISIONS 3

// Store the graphics context globally and pass it to each drawing function
static GdkGC *global_gc = NULL;

static DESIGN design = {NULL, NULL} ;

// This is turned off during panning
static gboolean bSynchRulers = TRUE ;

static print_design_OP print_options =
  {{612, 792, 72, 72, 72, 72, TRUE, NULL}, 3, TRUE, FALSE, TRUE, NULL, 0, 1, 1} ;
  
VectorTable *pvt = NULL ;

extern cell_OP cell_options ;
extern double subs_width ;
extern double subs_height ;
extern main_W main_window ;
extern double grid_spacing ;
extern int AREA_WIDTH ;
extern int AREA_HEIGHT ;

static project_OP project_options =
  {
  {0, 0x0000, 0xffff, 0xffff},   // clrCyan
  {0, 0xffff, 0xffff, 0xffff},   // clrWhite
//!Currently selection action to perform by the CAD interface (ex Move Cell)
  ACTION_DEFAULT,           // selected_action
//!Current simulation engine.
  COHERENCE_VECTOR,         // SIMULATION_ENGINE
//!Maximum random response function shift.
  0.0,                      // max_response_shift
//!Probability that a design cell will be affected by the random response function shift.
  0.0,                      // affected_cell_probability
//!Has the design been altered ?
  FALSE,                    // bDesignAltered
//!Currently selected cell type
  1,                        // selected_cell_type
//!General CAD options
  TRUE,                     // SHOW_GRID
  TRUE,                     // SNAP_TO_GRID
//!Current project file name.
  NULL,                     // pszCurrentFName
//!Pointer to the cell which was clicked on when moving many cells.
//!This is the cell that is kept under the pointer when many are moved.
  NULL,                     // window_move_selected_cell
// an area of selected cells used in window selection and also in simulation for all cells within a radius //
  NULL,                     // selected_cells
  0,                        // number_of_selected_cells
//!Current simulation type.
  EXHAUSTIVE_VERIFICATION,
// When copying cells or importing a block, the newly created cells must first be placed
// before they can be added to the undo stack.  Upon dropping them onto the design without
// overlap, when set, this variable causes a "Create" event to be added to the undo stack,
// rather than a "Move" event.
  FALSE,
// The vector table
  NULL
  } ;

void setup_rulers () ;
static void set_ruler_scale (GtkRuler *ruler, double dXLower, double dYLower) ;
static void tabula_rasa (GtkWindow *wndMain) ; /* "Clean slate" - reset QCADesigner for a new project */
static void move_selected_cells_to_pointer () ;
void release_selection () ;
static void set_clock_for_selected_cells(int selected_clock) ;
static gboolean DoSave (GtkWindow *parent, gboolean bSaveAs) ;
static gboolean SaveDirtyUI (GtkWindow *parent, char *szMsg) ;
//static void fill_layers_combo (main_W *wndMain, DESIGN *pDesign) ;
//static void remove_single_item (GtkWidget *item, gpointer data) ;
//static GtkWidget *add_layer_to_combo (GtkCombo *combo, LAYER *layer) ;
//static void layer_status_change (GtkWidget *widget, gpointer data) ;
//static void layer_selected (GtkWidget *widget, gpointer data) ;
static gboolean redraw_async_cb (GtkWidget *widget) ;
void redraw_async (GtkWidget *widget) ;
static gboolean bHaveIdler = FALSE ;
static void change_cursor (GtkWidget *widget, GdkCursor *new_cursor) ;
static void ChildPreRun (gpointer p) ;

void main_window_show (GtkWidget *widget, gpointer data)
  {
  global_gc = gdk_gc_new (main_window.drawing_area->window) ;
  
  // This vector table is NEVER CLEARED ! Attach the appropriate signal and clear it there
  // (like destory_event, or hide, or something)
  pvt = project_options.pvt = VectorTable_new () ;

  fill_recent_files_menu (main_window.recent_files_menu, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;

  gdk_color_alloc (gdk_colormap_get_system (), &(project_options.clrWhite)) ;
  gdk_color_alloc (gdk_colormap_get_system (), &(project_options.clrCyan)) ;
  print_options.pbPrintedObjs = malloc (3 * sizeof (gboolean)) ;
  print_options.icPrintedObjs = 3 ;
  print_options.pbPrintedObjs[PRINTED_OBJECTS_CELLS] = TRUE ;
  print_options.pbPrintedObjs[PRINTED_OBJECTS_DIE] = FALSE ;
  print_options.pbPrintedObjs[PRINTED_OBJECTS_COLOURS] = TRUE ;
  
  // Initialize the design
  design.first_layer = malloc (sizeof (LAYER)) ;
  design.first_layer->type = LAYER_TYPE_CELLS ;
  design.first_layer->status = LAYER_STATUS_ACTIVE ;
  design.first_layer->pszDescription = g_strdup_printf ("%s", "Main Cell Layer") ;
  design.first_layer->first_obj = 
  design.first_layer->last_obj = NULL ;
  design.first_layer->prev = NULL ;
  design.first_layer->next =
  design.last_layer = malloc (sizeof (LAYER)) ;
  design.last_layer->type = LAYER_TYPE_CLOCKING ;
  design.last_layer->status = LAYER_STATUS_ACTIVE ;
  design.last_layer->pszDescription = g_strdup_printf ("%s", "Main Clocking Layer") ;
  design.last_layer->first_obj = 
  design.last_layer->last_obj = NULL ;
  design.last_layer->prev = design.first_layer ;
  design.last_layer->next = NULL ;

//  fill_layers_combo (&main_window, &design) ;
  
  action_button_clicked (main_window.default_action_button, (gpointer)run_action_DEFAULT) ;
  
  gtk_paned_set_position (GTK_PANED (main_window.vpaned1), gtk_paned_get_position (GTK_PANED (main_window.vpaned1))) ;
  }

// This function gets called during a resize
gboolean main_window_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering main_window_configure_event\n")) ;
  setup_rulers () ;
  // This function needs to return a value.
  // this is the source of one of the compiler warnings.
  return FALSE;
  }

gboolean synchronize_rulers (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
  {
  if (!bSynchRulers) return FALSE ;
  propagate_motion_to_rulers (widget, event) ;
  return FALSE;
  }

gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering scroll_event\n")) ;
  if (GDK_SCROLL_UP == event->direction) /* Mouse wheel clicked away from the user */
    {
    if (event->state & GDK_CONTROL_MASK)
      pan (10 * PAN_STEP, 0) ;
    else
      pan (0, 10 * PAN_STEP) ;
    redraw_async (widget) ;
    setup_rulers () ;
    }
  else if (GDK_SCROLL_DOWN == event->direction) /* Mouse wheel clicked towards the user */
    {
    if (event->state & GDK_CONTROL_MASK)
      pan (-10 * PAN_STEP, 0) ;
    else
      pan (0, -10 * PAN_STEP) ;
    redraw_async (widget) ;
    setup_rulers () ;
    }
  return FALSE ;
  }

gboolean expose_event(GtkWidget * widget, GdkEventExpose *event, gpointer user_data)
{
    GdkGC *gc = NULL ;
    DBG_CB_HERE (fprintf (stderr, "Entering expose_event\n")) ;
    GdkColor clr = {0, 0, 0, 0} ;

    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clr, FALSE, TRUE) ;
    
    AREA_WIDTH = (int) widget->allocation.width;
    AREA_HEIGHT = (int) widget->allocation.height;

    gc = gdk_gc_new (widget->window) ;
    gdk_gc_set_foreground (gc, &clr) ;
    gdk_gc_set_background (gc, &clr) ;
    gdk_draw_rectangle (widget->window, gc, TRUE, 0, 0, AREA_WIDTH, AREA_HEIGHT) ;

    gdk_window_begin_paint_region (widget->window, event->region) ;
    redraw_world (widget->window, global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
    gdk_window_end_paint (widget->window) ;
    
    return TRUE;
}

gboolean configure_event(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering configure_event\n")) ;
    setup_rulers () ;
    return FALSE;
}

void on_preview_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_preview_menu_item_activate\n")) ;
  init_print_design_options (&print_options, (GQCell *)(design.first_layer->first_obj)) ;
  do_print_preview ((print_OP *)(&print_options), GTK_WINDOW (main_window.main_window), (void *)(GQCell *)(design.first_layer->first_obj), (PrintFunction)print_world) ;
  }

void on_grid_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_grid_properties_menu_item_activate\n")) ;
  get_grid_spacing_from_user (GTK_WINDOW (main_window.main_window), &grid_spacing) ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}

void on_scale_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  static double scale = 1.0;
  GQCell **cell_array = NULL, *cell = NULL ;
  int icCells = 0, idx = 0 ;
	
  DBG_CB_HERE (fprintf (stderr, "Entering on_scale_menu_item_activate\n")) ;
  get_scale_from_user (GTK_WINDOW (main_window.main_window), &scale) ;
  
  if (1.0 == scale) return ;
  
  cell = (GQCell *)(design.first_layer->first_obj) ;
  
  while (NULL != cell)
    {
    icCells++ ;
    cell = cell->next ;
    }
  
  cell_array = malloc (icCells * sizeof (GQCell *)) ;
  cell = (GQCell *)(design.first_layer->first_obj) ;
  
  while (NULL != cell)
    {
    cell_array[idx] = cell ;
    idx++ ;
    if (idx == icCells)
      break ; // We should never get here (tm)
    cell = cell->next ;
    }
  scale_design ((GQCell *)(design.first_layer->first_obj), scale) ;

  gui_add_to_undo (Undo_CreateAction_CellParamChange (cell_array, icCells)) ;
  
  free (cell_array) ;
}

void on_cell_properties_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_cell_properties_menu_item_activate\n")) ;
  get_cell_properties_from_user (GTK_WINDOW (main_window.main_window), &cell_options) ;
  }
/*
void on_window_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_window_properties_menu_item_activate\n")) ;
  }

void on_layer_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_layer_properties_menu_item_activate\n")) ;
  }
*/
void on_show_tb_icons_menu_item_activate (GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_show_tb_icons_menu_item_activate\n")) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (main_window.toolbar),
    gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem)) ? 
    GTK_TOOLBAR_BOTH_HORIZ : GTK_TOOLBAR_TEXT) ;
  }

// toggle the snap to grid option //
void on_snap_to_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_snap_to_grid_menu_item_activate\n")) ;
  project_options.SNAP_TO_GRID = ((GtkCheckMenuItem *) menuitem)->active;
  }

// toggle the show grid option //
void on_show_grid_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_show_grid_menu_item_activate\n")) ;
  project_options.SHOW_GRID = ((GtkCheckMenuItem *) menuitem)->active;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
  }

void file_operations (GtkWidget *widget, gpointer user_data)
  {
  char *pszFName = NULL, *pszCurrent = (NULL == project_options.pszCurrentFName ? "" : project_options.pszCurrentFName) ;
  int fFileOp = (int)user_data ;

  if (FILEOP_SAVE == fFileOp || FILEOP_SAVE_AS == fFileOp)
    {
    DoSave (GTK_WINDOW (main_window.main_window), (FILEOP_SAVE_AS == fFileOp)) ;
    return ;
    }

  if (FILEOP_OPEN == fFileOp || FILEOP_OPEN_RECENT == fFileOp || FILEOP_NEW == fFileOp || FILEOP_CLOSE == fFileOp)
    if (!(SaveDirtyUI (GTK_WINDOW (main_window.main_window),
      FILEOP_OPEN_RECENT == fFileOp || 
             FILEOP_OPEN == fFileOp ? "You have made changes to your design.  Opening another design will discard those changes. Save first ?" :
              FILEOP_NEW == fFileOp ? "You have made changes to your design.  Starting a new design will discard those changes.  Save first ?" :
                                      "You have made changes to your design.  Closing your design will discard those changes.  Save first ?")))
      return ;
  
  if (!(FILEOP_OPEN     == fFileOp || 
        FILEOP_IMPORT   == fFileOp || 
        FILEOP_EXPORT   == fFileOp || 
        FILEOP_CMDLINE  == fFileOp))
    tabula_rasa (GTK_WINDOW (main_window.main_window)) ;
    
  if (FILEOP_NEW == fFileOp || FILEOP_CLOSE == fFileOp)
    {
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
    return ;
    }

  if (FILEOP_OPEN_RECENT == fFileOp)
    pszFName = g_strdup_printf ("%s", (char *)gtk_object_get_data (GTK_OBJECT (widget), "file")) ;
  else 
  if (FILEOP_CMDLINE == fFileOp)
    pszFName = g_strdup_printf ("%s", (char *)widget) ;
  else
    {
    if (NULL == (pszFName = get_file_name_from_user (GTK_WINDOW (main_window.main_window),
        FILEOP_OPEN == fFileOp ? "Open Design" :
      FILEOP_IMPORT == fFileOp ? "Import Block" :
      FILEOP_EXPORT == fFileOp ? "Export Block" : "????????", pszCurrent, FALSE)))
      return ;
    }
  
  if (g_file_test (pszFName, G_FILE_TEST_EXISTS) && 
    !(g_file_test (pszFName, G_FILE_TEST_IS_REGULAR) || 
      g_file_test (pszFName, G_FILE_TEST_IS_SYMLINK)))
    {
    GtkWidget *msg = NULL ;
    
    gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
      "The file \"%s\" is not a regular file.  Maybe it's a directory, or a device file.  Please choose a regular file.", pszFName))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    g_free (pszFName) ;
    return ;
    }
  
  if (FILEOP_OPEN == fFileOp || FILEOP_OPEN_RECENT == fFileOp || FILEOP_CMDLINE == fFileOp)
    {
    change_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;
    if (NULL != open_project_file(pszFName, 
      (GQCell **)(&((design.first_layer->first_obj))), 
      (GQCell **)(&((design.first_layer->last_obj))), 
      &grid_spacing))
      {
      char *pszTitle = NULL ;
      VectorTable_fill (pvt, (GQCell *)(design.first_layer->first_obj)) ;
      add_to_recent_files (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
      gtk_window_set_title (GTK_WINDOW (main_window.main_window),
        pszTitle = g_strdup_printf ("%s - %s", base_name (pszFName), MAIN_WND_BASE_TITLE)) ;
      g_free (pszTitle) ;
      if (NULL != project_options.pszCurrentFName)
        g_free (project_options.pszCurrentFName) ;
      project_options.pszCurrentFName = pszFName ;
      if (FILEOP_CMDLINE != fFileOp)
        redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
      }
    else
      {
      GtkWidget *msg = NULL ;
      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "Failed to open file \"%s\"!", pszFName))) ;
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      remove_recent_file (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
      g_free (pszFName) ;
      }
    change_cursor (main_window.main_window, NULL) ;
    return ;
    }
  else
  if (FILEOP_EXPORT == fFileOp)
    {
    change_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;
    export_block (pszFName, project_options.selected_cells, project_options.number_of_selected_cells, grid_spacing) ;
    change_cursor (main_window.main_window, NULL) ;
    g_free (pszFName) ;
    return ;
    }
  else
  if (FILEOP_IMPORT == fFileOp)
    {
    GQCell *pqc = NULL ;

    release_selection () ;

    change_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;
    if (NULL != (pqc = import_block (pszFName, &(project_options.selected_cells), &(project_options.number_of_selected_cells), 
      (GQCell **)(&((design.first_layer->last_obj))))))
      {
      int Nix ;

      for (Nix = 0 ; Nix < project_options.number_of_selected_cells ; Nix++)
        if (NULL != project_options.selected_cells[Nix])
          gqcell_select (project_options.selected_cells[Nix]) ;

      if (NULL == (GQCell *)(design.first_layer->first_obj))
        (GQCell *)(design.first_layer->first_obj) = pqc ;
      VectorTable_add_inputs (pvt, pqc) ;
      move_selected_cells_to_pointer () ;
      project_options.drop_new_cells = TRUE ;
      move_selected_cells_to_pointer () ;
      }
    else
      {
      GtkWidget *msg = NULL ;
      gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (main_window.main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "Failed to import block from file \"%s\"!", pszFName))) ;
      gtk_widget_hide (msg) ;
      gtk_widget_destroy (msg) ;
      }
    change_cursor (main_window.main_window, NULL) ;
    g_free (pszFName) ;
    return ;
    }
  }

void on_cell_function_menu_item_activate (GtkMenuItem *menuitem, gpointer user_data)
  {
  command_history_message ("Please select cell to edit...") ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.cell_properties_button), TRUE) ;
  }

void on_clock_select_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_clock_select_menu_item_activate\n")) ;
  gtk_widget_show (create_clock_select_dialog (GTK_WINDOW (main_window.main_window), set_clock_for_selected_cells)) ;
}

void on_clock_increment_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{

    int i;

  DBG_CB_HERE (fprintf (stderr, "Entering on_clock_increment_menu_item_activate\n")) ;
    if (project_options.number_of_selected_cells > 0) {
      // -- increment the clock for each of the selected cells -- //
      for (i = 0; i < project_options.number_of_selected_cells; i++) {
        g_assert(project_options.selected_cells[i] != NULL);

        project_options.selected_cells[i]->clock++;
        project_options.selected_cells[i]->clock %= 4 ;
        
      }
      update_clock_select_dialog (get_clock_from_selected_cells (project_options.selected_cells, project_options.number_of_selected_cells)) ;
      gui_add_to_undo (Undo_CreateAction_CellParamChange (project_options.selected_cells, project_options.number_of_selected_cells)) ;
      project_options.bDesignAltered = TRUE ;
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
    }

}//on_clock_increment_menu_item_activate

void on_measurement_preferences1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_measurement_preferences1_activate\n")) ;
}
void on_draw_dimensions_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_draw_dimensions_menu_item_activate\n")) ;
}
void on_dimension_properties1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_dimension_properties1_activate\n")) ;
}
void on_draw_text_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_draw_text_menu_item_activate\n")) ;
}
void on_text_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_text_properties_menu_item_activate\n")) ;
}
void on_draw_arrow_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_draw_arrow_menu_item_activate\n")) ;
}
void on_arrow_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_arrow_properties_menu_item_activate\n")) ;
}
void on_draw_line_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_draw_line_menu_item_activate\n")) ;
}
void on_line_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_line_properties_menu_item_activate\n")) ;
}
void on_draw_rectangle_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_draw_rectangle_menu_item_activate\n")) ;
}
void on_rectangle_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_rectangle_properties_menu_item_activate\n")) ;
}
void on_save_output_to_file_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_save_output_to_file_menu_item_activate\n")) ;
}
void on_logging_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_logging_properties_menu_item_activate\n")) ;
}

void on_simulation_type_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_simulation_type_setup_menu_item_activate\n")) ;
  get_sim_type_from_user (GTK_WINDOW (main_window.main_window), &(project_options.SIMULATION_TYPE), pvt) ;
}  //on_simulation_properties_menu_item_activate

void on_simulation_engine_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_simulation_engine_setup_menu_item_activate\n")) ;
get_sim_engine_from_user (GTK_WINDOW (main_window.main_window), &(project_options.SIMULATION_ENGINE)) ;
}  //on_simulation_engine_setup_menu_item_activate

void on_zoom_in_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_in_menu_item_activate\n")) ;
    zoom_in(main_window.drawing_area);
    setup_rulers () ;
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}

void on_zoom_out_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_out_menu_item_activate\n")) ;
    zoom_out(main_window.drawing_area);
    setup_rulers () ;
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}

// when activated will zoom to fit the entire die in the window
void on_zoom_die_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_die_menu_item_activate\n")) ;
  zoom_die ();
  setup_rulers () ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}

void on_zoom_extents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_zoom_extents_menu_item_activate\n")) ;
  zoom_extents ((GQCell *)(design.first_layer->first_obj)) ;
  setup_rulers () ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}

void on_copy_cell_button_clicked(GtkButton * button, gpointer user_data)
{
    int i;
    
  DBG_CB_HERE (fprintf (stderr, "Entering on_copy_cell_button_clicked\n")) ;
    if (project_options.number_of_selected_cells > 0) {

      g_assert(project_options.selected_cells != NULL);
  
      for (i = 0; i < project_options.number_of_selected_cells; i++) {
          gqcell_reset_colour (project_options.selected_cells[i]) ;
          draw_stdqcell (main_window.drawing_area->window, global_gc, project_options.selected_cells[i]) ;
          project_options.selected_cells[i] = add_stdqcell(
            (GQCell **)(&((design.first_layer->first_obj))), 
            (GQCell **)(&((design.first_layer->last_obj))), 
            project_options.selected_cells[i], FALSE, project_options.selected_cells[i]->x, project_options.selected_cells[i]->y, &cell_options, TYPE_USECELL);
          gqcell_select (project_options.selected_cells[i]) ;

          if (project_options.selected_cells[i] == NULL) {
            printf ("memory allocation error in on_copy_cell_button_clicked\n");
            exit(1);
          }

          g_assert(project_options.selected_cells[i] != NULL);
    }

//    gui_add_to_undo (Undo_CreateAction_CreateCells (project_options.selected_cells, project_options.number_of_selected_cells)) ;

    move_selected_cells_to_pointer () ;
    project_options.drop_new_cells = TRUE ;
    }
}

void on_command_entry_changed(GtkEditable * editable, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_command_entry_changed\n")) ;
}

void on_print_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_print_menu_item_activate\n")) ;
  if (get_print_design_properties_from_user (GTK_WINDOW (main_window.main_window), &print_options, (GQCell *)(design.first_layer->first_obj)))
    print_world (&print_options, (GQCell *)(design.first_layer->first_obj)) ;
  }

void on_project_properties_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering on_project_properties_menu_item_activate\n")) ;
  get_project_properties_from_user (GTK_WINDOW (main_window.main_window), &subs_width, &subs_height) ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
  }

// quit QCADesigner selected from menu //
gboolean on_quit_menu_item_activate(GtkWidget *main_window, gpointer user_data)
  {
  if (!SaveDirtyUI (GTK_WINDOW (main_window),
    "You have made changes to your design.  Quitting QCADesigner will discard those changes.  Save first ?"))
      return TRUE ;
  else
    gtk_main_quit () ;
  return FALSE ;
  }


void on_undo_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_undo_menu_item_activate\n")) ;
  release_selection () ;
  Undo_Undo () ;
  gtk_widget_set_sensitive (GTK_WIDGET (menuitem), Undo_CanUndo ()) ;
  gtk_widget_set_sensitive (main_window.redo_menu_item, Undo_CanRedo ()) ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}
void on_redo_meu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_redo_menu_item_activate\n")) ;
  release_selection () ;
  Undo_Redo () ;
  gtk_widget_set_sensitive (GTK_WIDGET (menuitem), Undo_CanRedo ()) ;
  gtk_widget_set_sensitive (main_window.undo_menu_item, Undo_CanUndo ()) ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
}
void on_copy_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_copy_menu_item_activate\n")) ;
}
void on_cut_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_cut_menu_item_activate\n")) ;
}
void on_paste_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_paste_menu_item_activate\n")) ;
}

void on_delete_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_delete_menu_item_activate\n")) ;
  	gui_add_to_undo (Undo_CreateAction_DeleteCells (project_options.selected_cells, project_options.number_of_selected_cells)) ;
    
  	// -- if there are cells already selected delete them first -- //
    gui_delete_cells (project_options.selected_cells, project_options.number_of_selected_cells) ;

    // free up the memory from the selected cell array of pointers //
    release_selection () ;

    // -- redraw so that the deleted cells disappear from the screen -- //
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;

}

void on_preferences_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_preferences_menu_item_activate\n")) ;
}
void on_start_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  simulation_data *sim_data = NULL ;
  
  DBG_CB_HERE (fprintf (stderr, "Entering on_start_simulation_menu_item_activate\n")) ;
  change_cursor (main_window.main_window, gdk_cursor_new (GDK_WATCH)) ;

  sim_data = run_simulation(project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, (GQCell *)(design.first_layer->first_obj));
  
  change_cursor (main_window.main_window, NULL) ;
  
  if (NULL != sim_data)
    show_graph_dialog (GTK_WINDOW (main_window.main_window), sim_data) ;
}

void on_random_fault_setup_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_random_fault_setup_menu_item_activate\n")) ;
  get_random_fault_params_from_user (GTK_WINDOW (main_window.main_window),
    &(project_options.max_response_shift), &(project_options.affected_cell_probability)) ;
}

void on_pause_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_pause_simulation_menu_item_activate\n")) ;
}
void on_stop_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_stop_simulation_menu_item_activate\n")) ;
}
void on_reset_simulation_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_reset_simulation_menu_item_activate\n")) ;
}

void on_calculate_ground_state_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data){
  DBG_CB_HERE (fprintf (stderr, "Entering on_calculate_ground_state_menu_item_activate\n")) ;
  calculate_ground_state(project_options.SIMULATION_ENGINE);
}

void on_animate_test_simulation_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data){
  DBG_CB_HERE (fprintf (stderr, "Entering on_animate_test_simulation_menu_item_activate\n")) ;
/*
  nonlinear_approx_options.animate_simulation = 1;
  run_simulation(project_options.SIMULATION_ENGINE);
  nonlinear_approx_options.animate_simulation = 0;
*/
}

void on_contents_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
char *pszCmdLine = NULL ;
#ifdef WIN32
char *pszBrowser = 
  get_external_app (GTK_WINDOW (main_window.main_window), "Please Select Web Browser", "browser", 
    "C:\\Program Files\\Internet Explorer\\iexplore.exe", FALSE) ;
#else
char *pszBrowser = 
  get_external_app (GTK_WINDOW (main_window.main_window), "Please Select Web Browser", "browser", 
    "/usr/bin/mozilla", FALSE) ;
#endif

if (NULL == pszBrowser) return ;

#ifdef WIN32
pszCmdLine = g_strdup_printf ("%s file://%s%c..%cshare%cdoc%cQCADesigner%cmanual%cindex.html",
  pszBrowser, getenv ("MY_PATH"), G_DIR_SEPARATOR, G_DIR_SEPARATOR, G_DIR_SEPARATOR, G_DIR_SEPARATOR, 
  G_DIR_SEPARATOR, G_DIR_SEPARATOR) ;
//pszCmdLine = g_strdup_printf ("%s file://..\\share\\doc\\QCADesigner\\manual\\index.html", pszBrowser) ;
#else
pszCmdLine = g_strdup_printf ("%s %s%cdoc%cQCADesigner%cmanual%cindex.html",
  pszBrowser, PACKAGE_DATA_DIR, G_DIR_SEPARATOR, G_DIR_SEPARATOR, G_DIR_SEPARATOR, G_DIR_SEPARATOR) ;
#endif

// fprintf (stderr, "Proceeding with command line |%s|\n", pszCmdLine) ;

RunCmdLineAsync (pszCmdLine) ;

g_free (pszCmdLine) ;
}

void on_search_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_search_menu_item_activate\n")) ;
}
void on_about_menu_item_activate(GtkMenuItem * menuitem, gpointer user_data)
{
  DBG_CB_HERE (fprintf (stderr, "Entering on_about_menu_item_activate\n")) ;
show_about_dialog (GTK_WINDOW (main_window.main_window), FALSE) ;
}

void rotate_selection_menu_item_activate (GtkWidget *widget, gpointer user_data)
  {
  int Nix ;
  
  DBG_CB_HERE (fprintf (stderr, "Entering rotate_selection_item_activate\n")) ;
  gdk_gc_set_function (global_gc, GDK_XOR) ;
  redraw_selected_cells (main_window.drawing_area->window, global_gc, 
    project_options.selected_cells, project_options.number_of_selected_cells) ;
  gdk_gc_set_function (global_gc, GDK_COPY) ;

  if (project_options.number_of_selected_cells > 0)
    for (Nix = 0 ; Nix < project_options.number_of_selected_cells ; Nix++)
      if (NULL != project_options.selected_cells[Nix])
        gqcell_move_to_location (project_options.selected_cells[Nix], (-1) * project_options.selected_cells[Nix]->y, project_options.selected_cells[Nix]->x) ;
  
  
  move_selected_cells_to_pointer () ;
  }
/*
static void layer_selected (GtkWidget *widget, gpointer data)
  {
  LAYER *layer = (LAYER *)g_object_get_data (G_OBJECT (widget), "layer") ;

  if (NULL == layer) return ;

  if (LAYER_TYPE_CELLS == layer->type)
    {
    gtk_widget_hide (main_window.oval_zone_button) ;
    gtk_widget_hide (main_window.polygon_zone_button) ;

    gtk_widget_show (main_window.insert_type_1_cell_button) ;
    gtk_widget_show (main_window.insert_type_2_cell_button) ;
    gtk_widget_show (main_window.cell_properties_button) ;
    gtk_widget_show (main_window.insert_cell_array_button) ;
    gtk_widget_show (main_window.copy_cell_button) ;
    gtk_widget_show (main_window.rotate_cell_button) ;
    gtk_widget_show (main_window.mirror_button) ;
    gtk_widget_show (main_window.delete_cells_button) ;
    }
  else
  if (LAYER_TYPE_CLOCKING == layer->type)
    {
    gtk_widget_show (main_window.oval_zone_button) ;
    gtk_widget_show (main_window.polygon_zone_button) ;

    gtk_widget_hide (main_window.insert_type_1_cell_button) ;
    gtk_widget_hide (main_window.insert_type_2_cell_button) ;
    gtk_widget_hide (main_window.cell_properties_button) ;
    gtk_widget_hide (main_window.insert_cell_array_button) ;
    gtk_widget_hide (main_window.copy_cell_button) ;
    gtk_widget_hide (main_window.rotate_cell_button) ;
    gtk_widget_hide (main_window.mirror_button) ;
    gtk_widget_hide (main_window.delete_cells_button) ;
    }
  gtk_widget_queue_draw (main_window.toolbar) ;
  }
*/
void action_button_clicked (GtkWidget *widget, gpointer data)
  {
  int Nix ;
  static MOUSE_HANDLERS mh = {-1, -1, -1} ;
  int new_action = (int)data ;
  
  if (NULL != widget)
    {
    if (GTK_IS_TOGGLE_BUTTON (widget))
      if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
        return ;
    }
  
  change_cursor (main_window.drawing_area,
    run_action_PAN == (ActionHandler)data ? gdk_cursor_new (GDK_FLEUR) : NULL) ;
  
  bSynchRulers = (run_action_PAN != (ActionHandler)data) ;
  
  if (run_action_DELETE == (ActionHandler)data && project_options.number_of_selected_cells > 0)
    {
    //Delete selected cells before switching action handlers
    gui_add_to_undo (Undo_CreateAction_DeleteCells (project_options.selected_cells, project_options.number_of_selected_cells)) ;
    gui_delete_cells (project_options.selected_cells, project_options.number_of_selected_cells) ;
    release_selection () ;
    redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, 
      (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
    }
  else
  if (run_action_ROTATE == (ActionHandler)data && project_options.number_of_selected_cells > 0)
    {
    // Rotate dots inside all selected cells before switching action handlers
    for (Nix = 0 ; Nix < project_options.number_of_selected_cells ; Nix++)
      gqcell_rotate (project_options.selected_cells[Nix], 3.14159 / 4.0);
    gui_add_to_undo (Undo_CreateAction_CellParamChange (project_options.selected_cells, 
      project_options.number_of_selected_cells)) ;
    redraw_async (main_window.drawing_area) ;
    }
  
  if (-1 != mh.lIDButtonPressed)  g_signal_handler_disconnect (main_window.drawing_area, mh.lIDButtonPressed) ;
  if (-1 != mh.lIDMotionNotify)   g_signal_handler_disconnect (main_window.drawing_area, mh.lIDMotionNotify) ;
  if (-1 != mh.lIDButtonReleased) g_signal_handler_disconnect (main_window.drawing_area, mh.lIDButtonReleased) ;

  project_options.selected_cell_type =
    main_window.insert_type_1_cell_button == widget ? TYPE_1 :
    main_window.insert_type_2_cell_button == widget ? TYPE_2 : 
    project_options.selected_cell_type ;
  
  // Connect the appropriate mouse handlers
  (*((ActionHandler)(data))) (&mh, main_window.drawing_area, global_gc, &design, &project_options, &main_window) ;

  project_options.selected_action = new_action ;
  }

///////////////////////////////////////////////////////////////////
///////////////////////// HELPERS /////////////////////////////////
///////////////////////////////////////////////////////////////////

void setup_rulers ()
  {
  GdkModifierType mask ;
  double world_x1, world_y1, world_x2, world_y2, world_x, world_y ;
  int xOffset = 0, yOffset = 0, x = 0, y = 0,
    xFrame = main_window.drawing_area_frame->allocation.x,
    yFrame = main_window.drawing_area_frame->allocation.y,
    cxFrame = main_window.drawing_area_frame->allocation.width,
    cyFrame = main_window.drawing_area_frame->allocation.height,
    xDA = main_window.drawing_area->allocation.x,
    yDA = main_window.drawing_area->allocation.y ;
    
  gdk_window_get_pointer (main_window.drawing_area->window, &x, &y, &mask) ;
  
  world_x1 = calc_world_x (xOffset = xFrame - xDA) ;
  world_y1 = calc_world_y (yOffset = yFrame - yDA) ;
  world_x = calc_world_x (x - xOffset) ;
  world_y = calc_world_y (y - yOffset) ;
  world_x2 = calc_world_x (xOffset + cxFrame + 1) ;
  world_y2 = calc_world_y (yOffset + cyFrame + 1) ;
  
  world_x = CLAMP (world_x, world_x1, world_x2) ;
  world_y = CLAMP (world_y, world_y1, world_y2) ;

  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2) ;
  set_ruler_scale (GTK_RULER (main_window.horizontal_ruler), world_y1, world_y2) ;

  gtk_ruler_set_range (GTK_RULER (main_window.horizontal_ruler), world_x1, world_x2, world_x, world_x2) ;
  gtk_ruler_set_range (GTK_RULER (main_window.vertical_ruler), world_y1, world_y2, world_y, world_y2) ;
  
  gdk_flush () ;
  }

void gui_add_to_undo (void *p)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering gui_add_to_undo\n")) ;
  Undo_AddAction (p) ;
  gtk_widget_set_sensitive (main_window.undo_menu_item, Undo_CanUndo ()) ;
  gtk_widget_set_sensitive (main_window.redo_menu_item, Undo_CanRedo ()) ;
  }

void gui_create_cells (GQCell **ppqc, int ic)
  {
  int Nix ;
  
  DBG_CB_HERE (fprintf (stderr, "Entering gui_create_cells\n")) ;
  for (Nix = 0 ; Nix < ic ; Nix++)
    if (NULL != ppqc[Nix])
      {
      gqcell_link (ppqc[Nix], (GQCell *)(((design.first_layer->last_obj))), NULL) ;

      if (NULL == design.first_layer->first_obj)
        ((GQCell *)(design.first_layer->first_obj)) = ppqc[Nix] ;

      ((GQCell *)(((design.first_layer->last_obj)))) = ppqc[Nix] ;

      if (ppqc[Nix]->is_input)
        VectorTable_add_input (pvt, ppqc[Nix]) ;

      project_options.bDesignAltered = TRUE ;
      }
  clean_up_colors ((GQCell *)(design.first_layer->first_obj)) ;
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
  }

void gui_delete_cells (GQCell **ppqc, int ic)
  {
  int Nix ;
  
  DBG_CB_HERE (fprintf (stderr, "Entering gui_delete_cells\n")) ;
  for (Nix = 0 ; Nix < ic ; Nix++)
    if (ppqc[Nix] != NULL)
      {
      delete_stdqcell (ppqc[Nix], pvt, 
        (GQCell **)(&((design.first_layer->first_obj))), 
        (GQCell **)(&((design.first_layer->last_obj)))) ;
      project_options.bDesignAltered = TRUE ;
      }
  redraw_world (GDK_DRAWABLE (main_window.drawing_area->window), global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
  }

void release_selection ()
  {
  int Nix ;
  
  DBG_CB_HERE (fprintf (stderr, "Entering release_selection\n")) ;
  if (0 == project_options.number_of_selected_cells || NULL == project_options.selected_cells)
    return ;
  
  for (Nix = 0 ; Nix < project_options.number_of_selected_cells ; Nix++)
    {
    gqcell_reset_colour (project_options.selected_cells[Nix]) ;
    draw_stdqcell (main_window.drawing_area->window, global_gc, project_options.selected_cells[Nix]) ;
    }
  // free up the memory from the selected cell array of pointers //
  free(project_options.selected_cells);
  project_options.selected_cells = NULL;
  project_options.number_of_selected_cells = 0 ;
  project_options.window_move_selected_cell = NULL;
  gdk_flush () ;
  }

void propagate_motion_to_rulers (GtkWidget *widget, GdkEventMotion *event)
  {
  GdkEventMotion *pevVRule = NULL ;
  GdkEventMotion *pevHRule = NULL ;
  
  pevVRule = (GdkEventMotion *)gdk_event_copy ((GdkEvent *)event) ;
  pevVRule->window = (main_window.vertical_ruler)->window ;

  pevHRule = (GdkEventMotion *)gdk_event_copy ((GdkEvent *)event) ;
  pevHRule->window = (main_window.horizontal_ruler)->window ;

  gtk_main_do_event ((GdkEvent *)pevVRule) ;
  gtk_main_do_event ((GdkEvent *)pevHRule) ;

  pevHRule->window = pevVRule->window = widget->window ;

  gdk_event_free ((GdkEvent *)pevHRule) ;
  gdk_event_free ((GdkEvent *)pevVRule) ;
  }

void redraw_async (GtkWidget *widget)
  {
  if (!bHaveIdler)
    {
    bHaveIdler = TRUE ;
    g_idle_add ((GSourceFunc)redraw_async_cb, widget) ;
    }
  }

///////////////////////////////////////////////////////////////////
///////////////////// STATIC HELPERS //////////////////////////////
///////////////////////////////////////////////////////////////////

static gboolean redraw_async_cb (GtkWidget *widget)
  {
  GdkRectangle rc = {0, 0, 0, 0} ;

  gdk_window_get_size (widget->window, &(rc.width), &(rc.height)) ;

  gdk_window_begin_paint_rect (widget->window, &rc) ;
  redraw_world (widget->window, global_gc, (GQCell *)(design.first_layer->first_obj), project_options.SHOW_GRID) ;
  gdk_window_end_paint (widget->window) ;
  
  return (bHaveIdler = FALSE) ;
  }

static void set_ruler_scale (GtkRuler *ruler, double dLower, double dUpper)
  {
#ifdef WIN32
  return ;
  }
#else
  double dRange = dUpper - dLower ;
  int iPowerOfTen = ceil (log10 (dRange)), Nix = 0, iPowerOfDivisor = 0 ;
  double dScale = pow (10, iPowerOfTen) ;

  DBG_CB_HERE (fprintf (stderr, "Entering set_ruler_scale\n")) ;

  if (dRange < dScale / 2)
    {
    dScale /= 2 ;
    iPowerOfDivisor = 1 ;
    }

  for (Nix = 9 ; Nix > -1 ; Nix--)
    {
    ruler->metric->ruler_scale[Nix] = floor (dScale / ((double)(1 << iPowerOfDivisor))) ;
    iPowerOfDivisor++ ;
    iPowerOfDivisor %= NUMBER_OF_RULER_SUBDIVISIONS ;
    if (0 == iPowerOfDivisor)
      dScale = pow (10, floor (log10 (dScale / NUMBER_OF_RULER_SUBDIVISIONS))) ;
    }
  }
#endif /* WIN32 => Don't set_ruler_scale */

static gboolean SaveDirtyUI (GtkWindow *parent, char *szMsg)
  {
  int msgVal = GTK_RESPONSE_CANCEL ;
  if (project_options.bDesignAltered)
    {
    GtkWidget *msg = NULL ;
    msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, szMsg) ;
    gtk_dialog_add_action_widget (GTK_DIALOG (msg), gtk_button_new_with_stock_image (GTK_STOCK_NO, _("Don't save")), GTK_RESPONSE_NO) ;
    gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
    gtk_widget_grab_default (gtk_dialog_add_button (GTK_DIALOG (msg), GTK_STOCK_SAVE, GTK_RESPONSE_YES)) ;
    
    msgVal = gtk_dialog_run (GTK_DIALOG (msg)) ;

    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;

    if (GTK_RESPONSE_YES == msgVal)
      return DoSave (parent, FALSE) ;
    else
    if (GTK_RESPONSE_NO != msgVal)
      return FALSE ;
    }
  return TRUE ;
  }

static gboolean DoSave (GtkWindow *parent, gboolean bSaveAs)
  {
  char *pszFName = project_options.pszCurrentFName, *pszCurrent = (NULL == project_options.pszCurrentFName ? "" : project_options.pszCurrentFName) ;
  
  if ((NULL == project_options.pszCurrentFName) || bSaveAs)
    if (NULL == (pszFName = get_file_name_from_user (parent, "Save As", pszCurrent, TRUE)))
      return FALSE ;
  
  if (create_file (pszFName, (GQCell *)(design.first_layer->first_obj), grid_spacing))
    {
    char *pszTitle = NULL ;
    project_options.bDesignAltered = FALSE ;
    gtk_window_set_title (parent, pszTitle = g_strdup_printf ("%s - %s", base_name (pszFName), MAIN_WND_BASE_TITLE)) ;
    g_free (pszTitle) ;
    add_to_recent_files (main_window.recent_files_menu, pszFName, GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN_RECENT) ;
    if (NULL != project_options.pszCurrentFName && project_options.pszCurrentFName != pszFName)
      g_free (project_options.pszCurrentFName) ;
    project_options.pszCurrentFName = pszFName ;
    return TRUE ;
    }
  else
    {
    GtkWidget *msg = NULL ;
    gtk_dialog_run (GTK_DIALOG (
      msg = gtk_message_dialog_new (parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "Failed to create file \"%s\" !", pszFName))) ;
    gtk_widget_hide (msg) ;
    gtk_widget_destroy (msg) ;
    }
  if (pszFName != project_options.pszCurrentFName) g_free (pszFName) ;
  return FALSE ;
  }

/* Cleans out the linked list of cells, clears the undo stack, and does whatever else must
   be done to restore QCADesigner to its initial, pristine state */
static void tabula_rasa (GtkWindow *wndMain)
  {
  DBG_CB_HERE (fprintf (stderr, "Entering tabula_rasa\n")) ;
  release_selection () ;
  clear_all_cells ((GQCell **)(&((design.first_layer->first_obj))), (GQCell **)(&((design.first_layer->last_obj)))) ;
  project_options.bDesignAltered = FALSE ;
  g_free (project_options.pszCurrentFName) ;
  project_options.pszCurrentFName = NULL ;
  gtk_window_set_title (wndMain, "Untitled - " MAIN_WND_BASE_TITLE) ;
  Undo_Clear () ;
  gtk_widget_set_sensitive (main_window.undo_menu_item, Undo_CanUndo ()) ;
  gtk_widget_set_sensitive (main_window.redo_menu_item, Undo_CanRedo ()) ;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.default_action_button), TRUE) ;
  }

static void move_selected_cells_to_pointer ()
  {
  int i, x, y ;
  double x_offset, y_offset ;
  GdkModifierType state ;
  int xScreen, yScreen ;
  
  if (project_options.number_of_selected_cells <= 0) return ;

  DBG_CB_HERE (fprintf (stderr, "Entering move_selected_cells_to_pointer\n")) ;
  project_options.window_move_selected_cell = project_options.selected_cells[0];
  
  gdk_window_get_pointer(main_window.drawing_area->window, &x, &y, &state);
  
  gdk_window_get_origin (main_window.drawing_area->window, &xScreen, &yScreen) ;
  
  x_offset = grid_world_x (calc_world_x (x)) ;
  y_offset = grid_world_y (calc_world_y (y)) ;
  
  x_offset -= project_options.window_move_selected_cell->x ;
  y_offset -= project_options.window_move_selected_cell->y ;
  
  for (i = 0 ; i < project_options.number_of_selected_cells ; i++)
    gqcell_move_by_offset (project_options.selected_cells[i], x_offset, y_offset) ;

  gdk_gc_set_function (global_gc, GDK_XOR) ;
  redraw_selected_cells (main_window.drawing_area->window, global_gc, 
    project_options.selected_cells, project_options.number_of_selected_cells) ;
  gdk_gc_set_function (global_gc, GDK_COPY) ;
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (main_window.default_action_button), TRUE) ;
  
  run_action_DEFAULT_sel_changed () ;
  }

static void set_clock_for_selected_cells (int selected_clock)
{

    int i;

  DBG_CB_HERE (fprintf (stderr, "Entering set_clock_for_selected_cells\n")) ;
    g_assert(selected_clock < 4 && selected_clock >= 0);

    for (i = 0; i < project_options.number_of_selected_cells; i++) {

	g_assert(project_options.selected_cells[i] != NULL);

	project_options.selected_cells[i]->clock = selected_clock;
        
        gqcell_reset_colour (project_options.selected_cells[i]) ;
        
    }
  gui_add_to_undo (Undo_CreateAction_CellParamChange (project_options.selected_cells, project_options.number_of_selected_cells)) ;
  release_selection () ;
} //set_clock_for_selected_cells

/*
static void fill_layers_combo (main_W *wndMain, DESIGN *pDesign)
  {
  LAYER *layer = pDesign->first_layer ;
  int icLayers = 0 ;
  GtkWidget *first_layer_item = NULL ;
  
  gtk_container_foreach (GTK_CONTAINER (GTK_COMBO (wndMain->layers_combo)->list), (GtkCallback)remove_single_item, GTK_COMBO (wndMain->layers_combo)->list) ;
  
  if (NULL != layer)
    {
    gtk_list_item_select (GTK_LIST_ITEM (
      first_layer_item = add_layer_to_combo (GTK_COMBO (wndMain->layers_combo), layer))) ;
    layer = layer->next ;
    icLayers++ ;
    }

  while (NULL != layer)
    {
    add_layer_to_combo (GTK_COMBO (wndMain->layers_combo), layer) ;
    layer = layer->next ;
    icLayers++ ;
    }

  if (1 == icLayers)
    {
    GtkWidget 
      *chkVisible = g_object_get_data (G_OBJECT (first_layer_item), "chkVisible"),
      *chkActive = g_object_get_data (G_OBJECT (first_layer_item), "chkActivate") ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible), TRUE) ;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActive), TRUE) ;
    
    gtk_widget_set_sensitive (chkVisible, FALSE) ;
    gtk_widget_set_sensitive (chkActive, FALSE) ;
    }
  }

static void remove_single_item (GtkWidget *item, gpointer data)
  {
  GtkContainer *container = GTK_CONTAINER (data) ;
  
  gtk_container_remove (container, item) ;
  }

static GtkWidget *add_layer_to_combo (GtkCombo *combo, LAYER *layer)
  {
  GtkWidget *item = NULL, *chkVisible = NULL, *chkActive = NULL, *vsep = NULL, *lbl = NULL, *tbl = NULL ;
  
  item = gtk_list_item_new () ;
  gtk_widget_show (item) ;
  g_object_set_data (G_OBJECT (item), "layer", layer) ;
  g_object_set_data (G_OBJECT (item), "combo", combo) ;
  
  tbl = gtk_table_new (1, 4, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (item), tbl) ;
  
  chkVisible = gtk_check_button_new_with_label ("Visible") ;
  g_object_set_data (G_OBJECT (item), "chkVisible", chkVisible) ;
  gtk_widget_show (chkVisible) ;
  gtk_table_attach (GTK_TABLE (tbl), chkVisible, 0, 1, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;
  
  chkActive = gtk_check_button_new_with_label ("Active") ;
  g_object_set_data (G_OBJECT (item), "chkActivate", chkActive) ;
  gtk_widget_show (chkActive) ;
  gtk_table_attach (GTK_TABLE (tbl), chkActive, 1, 2, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(0), 2, 2) ;

  vsep = gtk_vseparator_new () ;
  gtk_widget_show (vsep) ;
  gtk_table_attach (GTK_TABLE (tbl), vsep, 2, 3, 0, 1,
    (GtkAttachOptions)(0),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;

  lbl = gtk_label_new (_(NULL == layer->pszDescription ? "" : layer->pszDescription)) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 3, 4, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;    
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkVisible),
    (LAYER_STATUS_ACTIVE == layer->status || LAYER_STATUS_VISIBLE == layer->status)) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActive), (LAYER_STATUS_ACTIVE == layer->status)) ;

  g_object_set_data (G_OBJECT (chkVisible), "bWasActive", (gpointer)(LAYER_STATUS_ACTIVE == layer->status)) ;

  g_signal_connect (G_OBJECT (chkVisible),    "toggled", (GCallback)layer_status_change, item) ;
  g_signal_connect (G_OBJECT (chkActive),     "toggled", (GCallback)layer_status_change, item) ;
  g_signal_connect (G_OBJECT (item),          "select",  (GCallback)layer_selected,      item) ;
  
  gtk_container_add (GTK_CONTAINER (combo->list), item) ;
  
  gtk_combo_set_item_string (combo, GTK_ITEM (item), NULL == layer->pszDescription ? "" : layer->pszDescription) ;
  return item ;
  }

static void layer_status_change (GtkWidget *widget, gpointer data)
  {
  GtkWidget 
    *chkActivate = GTK_WIDGET (g_object_get_data (G_OBJECT (data), "chkActivate")),
    *chkVisible  = GTK_WIDGET (g_object_get_data (G_OBJECT (data), "chkVisible")) ;
  LAYER *layer = (LAYER *)g_object_get_data (G_OBJECT (data), "layer") ;
  gboolean bActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ;
  
  if (NULL == layer) return ;
  
  if (chkVisible == widget)
    {
    if (bActive)
      {
      layer->status = LAYER_STATUS_VISIBLE ;
      gtk_widget_set_sensitive (chkActivate, TRUE) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActivate),
        (gboolean)g_object_get_data (G_OBJECT (widget), "bWasActive")) ;
      }
    else
      {
      layer->status = LAYER_STATUS_HIDDEN ;
      g_object_set_data (G_OBJECT (widget), "bWasActive",
        (gpointer)gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkActivate))) ;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chkActivate), FALSE) ;
      gtk_widget_set_sensitive (chkActivate, FALSE) ;
      }
    }
  else
  if (chkActivate == widget)
    layer->status = bActive ? LAYER_STATUS_ACTIVE : LAYER_STATUS_VISIBLE ;
  }
*/

// Set the cursor for the drawing area
static void change_cursor (GtkWidget *widget, GdkCursor *new_cursor)
  {
  GdkCursor *old_cursor = (GdkCursor *)g_object_get_data (G_OBJECT (widget), "old_cursor") ;
  
  gdk_window_set_cursor (widget->window, new_cursor) ;
  gdk_flush () ;

  if (NULL != old_cursor)
    {
    gdk_cursor_unref (old_cursor) ;
    old_cursor = NULL ;
    }

  g_object_set_data (G_OBJECT (widget), "old_cursor", new_cursor) ;
  }

static void ChildPreRun (gpointer p){}
