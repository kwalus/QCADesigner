//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
//////////////////////////////////////////////////////////

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <gtk/gtk.h>

#define MAIN_WND_BASE_TITLE "QCADesigner"

//!Structure holding main window widgets
typedef struct
  {
    GtkWidget *main_window;
    GtkWidget *vbox1;
    GtkWidget *vpaned1;
    GtkWidget *main_menubar;
    GtkTreeModel *layers_list_store ;
    GtkWidget *layers_toolbar ;
    GtkWidget *layers_combo ;
    GtkWidget *file_menu;
    GtkWidget *file_menu_menu;
    GtkAccelGroup *file_menu_menu_accels;
    GtkWidget *new_menu_item;
    GtkWidget *open_menu_item;
    GtkWidget *save_menu_item;
    GtkWidget *save_as_menu_item;
    GtkWidget *separator2;
    GtkWidget *print_menu_item;
    GtkWidget *preview_menu_item;
    GtkWidget *separator3;
    GtkWidget *project_properties_menu_item;
    GtkWidget *close_menu_item;
    GtkWidget *quit_menu_item;
    GtkWidget *edit_menu;
    GtkWidget *edit_menu_menu;
    GtkAccelGroup *edit_menu_menu_accels;
    GtkWidget *undo_menu_item;
    GtkWidget *redo_menu_item;
    GtkWidget *separator7;
    GtkWidget *copy_menu_item;
    GtkWidget *cut_menu_item;
    GtkWidget *paste_menu_item;
    GtkWidget *delete_menu_item;
    GtkWidget *separator6;
    GtkWidget *grid_properties_menu_item;
    GtkWidget *snap_properties_menu_item;
    GtkWidget *cell_properties_menu_item;
    GtkWidget *window_properties_menu_item;
    GtkWidget *layer_properties_menu_item;
    GtkWidget *preferences_menu_item;
    GtkWidget *separator4;
    GtkWidget *show_tb_icons_menu_item;
    GtkWidget *snap_to_grid_menu_item;
    GtkWidget *show_grid_menu_item;
    GtkWidget *tools_menu;
    GtkWidget *tools_menu_menu;
    GtkAccelGroup *tools_menu_menu_accels;
    GtkWidget *create_block_menu_item;
    GtkWidget *import_block_menu_item;
    GtkWidget *scale_menu_item;
    GtkWidget *rotate_selection_menu_item;
    GtkWidget *separator8;
    GtkWidget *cell_function_menu_item ;
    GtkWidget *separator9;
    GtkWidget *clock_select_menu_item;
    GtkWidget *clock_increment_menu_item;
    GtkWidget *fixed_polarization;
    GtkWidget *separator11;
    GtkWidget *measure_distance_menu_item;
    GtkWidget *measurement_preferences1;
    GtkWidget *draw_menu;
    GtkWidget *draw_menu_menu;
    GtkAccelGroup *draw_menu_menu_accels;
    GtkWidget *draw_dimensions_menu_item;
    GtkWidget *dimension_properties1;
    GtkWidget *separator16;
    GtkWidget *draw_text_menu_item;
    GtkWidget *text_properties_menu_item;
    GtkWidget *separator10;
    GtkWidget *draw_arrow_menu_item;
    GtkWidget *arrow_properties_menu_item;
    GtkWidget *separator14;
    GtkWidget *draw_line_menu_item;
    GtkWidget *line_properties_menu_item;
    GtkWidget *separator15;
    GtkWidget *draw_rectangle_menu_item;
    GtkWidget *rectangle_properties_menu_item;
    GtkWidget *simulation_menu;
    GtkWidget *simulation_menu_menu;
    GtkAccelGroup *simulation_menu_menu_accels;
    GtkWidget *start_simulation_menu_item;
    GtkWidget *stop_simulation_menu_item;
    GtkWidget *pause_simulation_menu_item;
    GtkWidget *reset_simulation_menu_item;
    GtkWidget *calculate_ground_state_menu_item;
    GtkWidget *animate_test_simulation_menu_item;
    GtkWidget *separator12;
    GtkWidget *save_output_to_file_menu_item;
    GtkWidget *separator13;
    GtkWidget *logging_properties_menu_item;
    GtkWidget *simulation_type_setup_menu_item;
    GtkWidget *simulation_engine_setup_menu_item;
    GtkWidget *random_fault_setup_menu_item;
    GtkWidget *view_menu;
    GtkWidget *view_menu_menu;
    GtkAccelGroup *view_menu_menu_accels;
    GtkWidget *zoom_in_menu_item;
    GtkWidget *zoom_out_menu_item;
    GtkWidget *zoom_window_menu_item;
    GtkWidget *zoom_die_menu_item;
    GtkWidget *zoom_extents_menu_item;
    GtkWidget *pan_menu_item;
    GtkWidget *help_menu;
    GtkWidget *help_menu_menu;
    GtkAccelGroup *help_menu_menu_accels;
    GtkWidget *contents_menu_item;
    GtkWidget *search_menu_item;
    GtkWidget *about_menu_item;
    GtkWidget *hbox1;
    GtkWidget *toolbar ;
    GtkWidget *toolbar_hsep ;
    GtkWidget *default_action_button ;
    GtkWidget *insert_type_1_cell_button;
    GtkWidget *insert_type_2_cell_button;
    GtkWidget *insert_cell_array_button;
    GtkWidget *copy_cell_button;
    GtkWidget *cell_properties_button;
//    GtkWidget *move_cell_button;
    GtkWidget *rotate_cell_button;
    GtkWidget *mirror_button;
    GtkWidget *delete_cells_button;
    GtkWidget *oval_zone_button;
    GtkWidget *polygon_zone_button;
    GtkWidget *zoom_plus_button;
    GtkWidget *pan_button;
    GtkWidget *zoom_minus_button;
    GtkWidget *zoom_extents_button;
    GtkWidget *table1;
    GtkWidget *drawing_area_frame;
    GtkWidget *drawing_area;
    GtkWidget *horizontal_ruler;
    GtkWidget *vertical_ruler;
    GtkWidget *scrolledwindow1;
    GtkWidget *command_history;
    GtkWidget *command_entry;
    GtkWidget *status_bar;
    GtkAccelGroup *accel_group;
    GtkTooltips *tooltips;
    GtkWidget *mnuSepPreRecentFiles ;
    GtkWidget *mnuSepPostRecentFiles ;
    GtkWidget *recent_files_menu ;
    GtkWidget *recent_files_menu_item ;
  } main_W ;

void create_main_window (main_W *main_window);

#endif /* _INTERFACE_H_ */
