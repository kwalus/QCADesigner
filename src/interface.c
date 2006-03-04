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
// The main UI. This creates the main QCADesigner win-  //
// dow. main() calls create_main_winow().               //
//                                                      //
//////////////////////////////////////////////////////////

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

// QCADesigner includes //
#include "support.h"
#include "callbacks.h"
#include "actions.h"
#include "interface.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "qcadstock.h"
#include "objects/QCADObject.h"
#include "objects/QCADCell.h"
#include "objects/QCADLayer.h"
#include "objects/QCADClockingLayer.h"
#include "objects/QCADPropertyUISingle.h"
#include "objects/QCADToggleToolButton.h"
#include "objects/QCADRadioToolButton.h"

main_W main_window = {NULL} ;
extern char *layer_stock_id[] ;
extern int n_layer_stock_id ;

// -- creates the main application window and returns a pointer to it -- //
void create_main_window (main_W *main_window){
  // All the objects that appear in the main window, that we do not refer to in the UI code //
  GtkWidget
    *widget                          = NULL, *mnuiSep                   = NULL, *about_menu_item                   = NULL, 
    *zoom_extents_menu_item          = NULL, *help_menu_menu            = NULL, *zoom_out_menu_item                = NULL,
    *progress                        = NULL, *tbl                       = NULL, *vbProgress                        = NULL,
    *toolbar_item                    = NULL, *clock_increment_menu_item = NULL, *delete_menu_item                  = NULL,
    *close_menu_item                 = NULL, *command_history           = NULL, *contents_menu_item                = NULL,
    *edit_menu                       = NULL, *edit_menu_menu            = NULL, *file_menu                         = NULL,
    *file_menu_menu                  = NULL, *hbox1                     = NULL, *help_menu                         = NULL,
    *layer_properties_menu_item      = NULL, *layers_toolbar            = NULL, *main_menubar                      = NULL,
    *print_menu_item                 = NULL, *quit_menu_item            = NULL, *view_menu_menu                    = NULL,
    *reset_zoom_menu_item            = NULL, *zoom_layer_menu_item      = NULL, *new_menu_item                     = NULL,
    *rotate_selection_menu_item      = NULL, *zoom_in_menu_item         = NULL, *mnui                              = NULL, 
    *scale_menu_item                 = NULL, *show_tb_icons_menu_item   = NULL, *simulation_engine_setup_menu_item = NULL,
    *simulation_menu                 = NULL, *simulation_menu_menu      = NULL, *simulation_type_setup_menu_item   = NULL,
    *start_simulation_menu_item      = NULL, *status_bar                = NULL, *stop_simulation_menu_item         = NULL,
    *table1                          = NULL, *tools_menu                = NULL, *tools_menu_menu                   = NULL,
    *view_menu                       = NULL, *hide_layers_menu_item     = NULL, *mnu                               = NULL,
    *show_scrollbars_menu_item       = NULL,
#ifdef STDIO_FILEIO
    *open_menu_item                  = NULL, *save_menu_item            = NULL, *save_as_menu_item                 = NULL,
    *recent_files_menu_item          = NULL, *import_block_menu_item    = NULL, *create_block_menu_item            = NULL,
    *load_output_from_file_menu_item = NULL, *preview_menu_item         = NULL, *img                               = NULL ;
#endif /* def STDIO_FILEIO */

  GtkAccelGroup *accel_group = NULL ;
  GtkTooltips *tooltips;
  char *psz = NULL ;
  int Nix, Nix1, x_offset = 0 ;

  tooltips = gtk_tooltips_new ();

  accel_group = gtk_accel_group_new ();

  // create the main window //
  main_window->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data (G_OBJECT (main_window->main_window), "main_window", main_window->main_window);
  gtk_window_set_default_size (GTK_WINDOW (main_window->main_window), 1024, 768);
  psz = g_strdup_printf ("%s - %s", _("Untitled"), MAIN_WND_BASE_TITLE) ;
  gtk_window_set_title (GTK_WINDOW (main_window->main_window), psz);
  g_free (psz) ;

  vbProgress = gtk_vbox_new (FALSE, 0) ;
  gtk_widget_show (vbProgress) ;
  gtk_container_add (GTK_CONTAINER (main_window->main_window), vbProgress) ;

  // create the vertical box and add it to the main window //
  main_window->vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (main_window->vbox1);
  gtk_box_pack_start (GTK_BOX (vbProgress), main_window->vbox1, TRUE, TRUE, 0) ;

  // create and add the main menubar to the main window //
  main_menubar = gtk_menu_bar_new ();
  gtk_widget_show (main_menubar);
  gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_menubar, FALSE, FALSE, 0);

  // Layers toolbar
  layers_toolbar = gtk_toolbar_new () ;
  gtk_widget_show (layers_toolbar) ;
  gtk_box_pack_start (GTK_BOX (main_window->vbox1), layers_toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_orientation (GTK_TOOLBAR (layers_toolbar), GTK_ORIENTATION_HORIZONTAL) ;
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (layers_toolbar), TRUE) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (layers_toolbar), GTK_TOOLBAR_ICONS) ;

  mnu = gtk_menu_new () ;
  for (Nix = 0 ; Nix < n_layer_stock_id ; Nix++)
    if (Nix != LAYER_TYPE_DISTRIBUTION)
      {
      mnui = gtk_image_menu_item_new_from_stock (layer_stock_id[Nix], NULL) ;
      gtk_widget_show (mnui) ;
      gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
      g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)type_for_new_layer_chosen, (gpointer)Nix) ;
      }

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_ADD, 
        "label",    _("Add Layer"),
        "visible",  TRUE,
        NULL)), -1) ;
//  g_object_set_data (G_OBJECT (toolbar_item), "mnu", mnu) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Add Layer to Design"), _("Adds a layer to your design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)popup_menu_button_clicked, mnu) ;

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar),
    GTK_TOOL_ITEM (toolbar_item = main_window->remove_layer_button = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_REMOVE, 
        "label",    _("Remove Layer"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Remove Layer from Design"), _("Remove a layer from your design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)remove_layer_button_clicked, NULL) ;

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_PROPERTIES, 
        "label",    _("Layer Properties"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Layer Properties"), _("Modify the settings for the current layer.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)layer_properties_button_clicked, NULL) ;

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar), GTK_TOOL_ITEM (toolbar_item = g_object_new (GTK_TYPE_TOOL_ITEM, "visible", TRUE, NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Layers"), _("Lists the layers in the current design and allows you to switch between them.")) ;

  main_window->layers_combo = gtk_combo_new () ;
  GTK_WIDGET_UNSET_FLAGS (main_window->layers_combo, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
  GTK_WIDGET_UNSET_FLAGS (GTK_COMBO (main_window->layers_combo)->entry, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
  gtk_widget_show (main_window->layers_combo) ;
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (main_window->layers_combo)->entry), FALSE) ;
  gtk_container_set_border_width (GTK_CONTAINER (main_window->layers_combo), 4) ;
  gtk_container_add (GTK_CONTAINER (toolbar_item), main_window->layers_combo) ;

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_REORDER_LAYERS, 
        "label",    _("Reorder"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Reorder Layers"), _("Change the order of layers in your design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)reorder_layers_button_clicked, NULL) ;

  // This will separate the layers combo from the clocks combo
  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar), GTK_TOOL_ITEM (g_object_new (GTK_TYPE_SEPARATOR_TOOL_ITEM, "visible", TRUE, NULL)), -1) ;

  main_window->pui_show_potential = qcad_object_create_property_ui_for_default_object (QCAD_TYPE_CLOCKING_LAYER, "show-potential", 
    "render-as",  GTK_TYPE_TOOL_ITEM,
    "stock-up",   QCAD_STOCK_NO_SHOW_POTENTIAL,
    "stock-down", QCAD_STOCK_SHOW_POTENTIAL,
    "show-label", FALSE, 
    "visible",    TRUE,
    "tooltip",    GTK_TOOLBAR (layers_toolbar)->tooltips,
    NULL) ;
  if (NULL != (widget = qcad_property_ui_get_widget (main_window->pui_show_potential, 0, 0, NULL)))
    if (GTK_IS_TOOL_ITEM (widget))
      {
      gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar), GTK_TOOL_ITEM (toolbar_item = widget), -1) ;
      gtk_widget_show (toolbar_item) ;
      }

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar), GTK_TOOL_ITEM (toolbar_item = g_object_new (GTK_TYPE_TOOL_ITEM, NULL)), -1) ;
  gtk_widget_show (toolbar_item) ;
  main_window->layer_toolbar_table = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (main_window->layer_toolbar_table) ;
  gtk_container_set_border_width (GTK_CONTAINER (main_window->layer_toolbar_table), 2) ;
  gtk_container_add (GTK_CONTAINER (toolbar_item), main_window->layer_toolbar_table) ;

  main_window->pui_clock = qcad_object_create_property_ui_for_default_object (QCAD_TYPE_CELL, "clock", 
    "render-as",  GTK_TYPE_OPTION_MENU, 
    "show-label", FALSE, 
    "visible",    TRUE,
    "tooltip",    GTK_TOOLBAR (layers_toolbar)->tooltips,
    NULL) ;
  for (Nix = 0 ; Nix < qcad_property_ui_get_cx_widgets (main_window->pui_clock) ; Nix++)
    for (Nix1 = 0 ; Nix1 < qcad_property_ui_get_cy_widgets (main_window->pui_clock) ; Nix1++)
      if (NULL != (widget = qcad_property_ui_get_widget (main_window->pui_clock, Nix, Nix1, NULL)))
        gtk_table_attach (GTK_TABLE (main_window->layer_toolbar_table), widget, Nix, Nix + 1, Nix1, Nix1 + 1, GTK_FILL, GTK_FILL, 2, 2) ;
  x_offset += Nix ;

  // This will separate the layer-specific ops from the bus layout
  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar), GTK_TOOL_ITEM (toolbar_item = GTK_WIDGET (g_object_new (GTK_TYPE_SEPARATOR_TOOL_ITEM, "visible", TRUE, NULL))), -1) ;
  gtk_widget_show (toolbar_item) ;

  gtk_toolbar_insert (GTK_TOOLBAR (layers_toolbar),
    GTK_TOOL_ITEM (toolbar_item = main_window->bus_layout_button = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_BUS, 
        "label",    _("Bus Layout"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (layers_toolbar)->tooltips, 
    _("Bus Layout"), _("Group individual input and output cells into buses.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)bus_layout_button_clicked, NULL) ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// FILE MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the file menu to the menubar //
  file_menu = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_widget_show (file_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), file_menu);

  file_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), file_menu_menu);

  // create and add the New project file menu item to the menubar //
  new_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, accel_group);
  gtk_widget_show (new_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), new_menu_item);
  gtk_tooltips_set_tip (tooltips, new_menu_item, _("Create a new project file"), NULL);

#ifdef STDIO_FILEIO
  // create and add the open project menu item to the menubar //
  open_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, accel_group);
  gtk_widget_show (open_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), open_menu_item);
  gtk_tooltips_set_tip (tooltips, open_menu_item, _("Open a project file"), NULL);

  // create and add the save menu item to the menubar //
  save_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, accel_group);
  gtk_widget_show (save_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), save_menu_item);
  gtk_tooltips_set_tip (tooltips, save_menu_item, _("Save current project"), NULL);

  // create and add the save as meny item to the menubar //
  save_as_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE_AS, accel_group);
  gtk_widget_show (save_as_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), save_as_menu_item);
  gtk_tooltips_set_tip (tooltips, save_as_menu_item, _("Save project file as ..."), NULL);
#endif /* def STDIO_FILEIO */

  // create and add a seperator to the file menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the print menu item to the menu bar //
  print_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PRINT, accel_group);
  gtk_widget_show (print_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), print_menu_item);

#ifdef STDIO_FILEIO
  // create and add the preview item to the menu bar //
  preview_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PRINT_PREVIEW, accel_group);
  gtk_widget_show (preview_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), preview_menu_item);
  gtk_tooltips_set_tip (tooltips, preview_menu_item, _("Preview the print layout"), NULL);
#endif /* def STDIO_FILEIO */

  // create and add a seperator to the file menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the close menu item to the file menu //
  close_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, accel_group);
  gtk_widget_show (close_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), close_menu_item);
  gtk_tooltips_set_tip (tooltips, close_menu_item, _("Close the project file"), NULL);

  // create and add a seperator to the file menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);
#ifdef STDIO_FILEIO
  recent_files_menu_item = gtk_menu_item_new_with_mnemonic (_("_Recent Files")) ;
  gtk_widget_show (recent_files_menu_item) ;
  gtk_container_add (GTK_CONTAINER (file_menu_menu), recent_files_menu_item) ;

  main_window->recent_files_menu = gtk_menu_new () ;
  gtk_widget_show (main_window->recent_files_menu) ;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (recent_files_menu_item), main_window->recent_files_menu) ;
#endif /* def STDIO_FILEIO */
  // create and add a seperator to the file menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the quit menu item to the file menu //
  quit_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, accel_group);
  gtk_widget_show (quit_menu_item);
  gtk_container_add (GTK_CONTAINER (file_menu_menu), quit_menu_item);
  gtk_tooltips_set_tip (tooltips, quit_menu_item, _("Quit QCADesigner"), NULL);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// EDIT MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the edit menu to the menubar //
  edit_menu = gtk_menu_item_new_with_mnemonic (_("_Edit"));
  gtk_widget_show (edit_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), edit_menu);

  edit_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (edit_menu), edit_menu_menu);

#ifdef UNDO_REDO
  // create and add the undo menu item to the menu bar //
  main_window->undo_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_UNDO, accel_group);
  gtk_widget_show (main_window->undo_menu_item);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), main_window->undo_menu_item);
  // initially, there is nothing to undo //
  gtk_widget_set_sensitive (GTK_WIDGET (main_window->undo_menu_item), FALSE);

  //create and add the redu menu item to the menu bar //
  main_window->redo_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REDO, accel_group);
  gtk_widget_show (main_window->redo_menu_item);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), main_window->redo_menu_item);
  // initially, there's nothing to redo //
  gtk_widget_set_sensitive (GTK_WIDGET (main_window->redo_menu_item), FALSE);

  // create and add a seperator to the edit menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);
#endif /* def UNDO_REDO */

  // create and add the delete item to the edit menu //
  delete_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, accel_group);
  gtk_widget_show (delete_menu_item);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), delete_menu_item);

  // create and add a seperator to the edit menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the layer properties menu item to the edit menu //
  layer_properties_menu_item = gtk_image_menu_item_new_with_label (_("Layer Properties..."));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (layer_properties_menu_item),
    img = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (layer_properties_menu_item);
  gtk_container_add (GTK_CONTAINER (edit_menu_menu), layer_properties_menu_item);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// VIEW MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the view menu //
  view_menu = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_widget_show (view_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), view_menu);

  view_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu), view_menu_menu);

  // create and add the zoom in menu item to the view menu //
  zoom_in_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ZOOM_IN, accel_group);
  gtk_widget_show (zoom_in_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), zoom_in_menu_item);

  // create and add the zoom out menu item to the view menu //
  zoom_out_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ZOOM_OUT, accel_group);
  gtk_widget_show (zoom_out_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), zoom_out_menu_item);

  // create and add the zoom extents menu item to the view menu //
  zoom_extents_menu_item = gtk_image_menu_item_new_with_label (_("Zoom Extents"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (zoom_extents_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_ZOOM_FIT, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (zoom_extents_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), zoom_extents_menu_item);

  // create and add the zoom extents menu item to the view menu //
  reset_zoom_menu_item = gtk_image_menu_item_new_with_label (_("Reset Zoom"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (reset_zoom_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (reset_zoom_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), reset_zoom_menu_item);

  // create and add the zoom extents menu item to the view menu //
  zoom_layer_menu_item = gtk_image_menu_item_new_with_label (_("Zoom Layer Extents"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (zoom_layer_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_ZOOM_FIT, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (zoom_layer_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), zoom_layer_menu_item);

  // create and add a seperator to the view menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the show toolbar icons menu item to the view menu //
  show_tb_icons_menu_item = gtk_check_menu_item_new_with_label (_("Show Toolbar Icons"));
  gtk_widget_show (show_tb_icons_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), show_tb_icons_menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (show_tb_icons_menu_item), TRUE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(show_tb_icons_menu_item), TRUE);

  // create and add the show toolbar icons menu item to the view menu //
  show_scrollbars_menu_item = gtk_check_menu_item_new_with_label (_("Show Scrollbars"));
  gtk_widget_show (show_scrollbars_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), show_scrollbars_menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (show_scrollbars_menu_item), TRUE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(show_scrollbars_menu_item), TRUE);

  // create and add a seperator to the view menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the snap to grid menu item to the view menu //
  main_window->snap_to_grid_menu_item = gtk_check_menu_item_new_with_label (_("Snap To Grid"));
  gtk_widget_show (main_window->snap_to_grid_menu_item);
  gtk_container_add (GTK_CONTAINER (view_menu_menu), main_window->snap_to_grid_menu_item);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window->snap_to_grid_menu_item), TRUE);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->snap_to_grid_menu_item), TRUE);
  g_object_add_weak_pointer (G_OBJECT (main_window->snap_to_grid_menu_item), (gpointer *)&(main_window->snap_to_grid_menu_item)) ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// TOOLS MENU ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the tools menu //
  tools_menu = gtk_menu_item_new_with_mnemonic (_("_Tools"));
  gtk_widget_show (tools_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), tools_menu);

  tools_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (tools_menu), tools_menu_menu);
#ifdef STDIO_FILEIO
  // create and add the create block menu item to the tools menu //
  create_block_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Create Block..."));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (create_block_menu_item),
    img = gtk_image_new_from_stock (QCAD_STOCK_BLOCK_WRITE, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (create_block_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), create_block_menu_item);

  // create and add the import block menu item to the tools menu //
  import_block_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Import Block..."));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (import_block_menu_item),
    img = gtk_image_new_from_stock (QCAD_STOCK_BLOCK_READ, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (import_block_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), import_block_menu_item);
#endif /* def STDIO_FILEIO */
  // create and add a seperator to the tools menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  main_window->bus_layout_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Bus Layout...")) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (main_window->bus_layout_menu_item),
    img = gtk_image_new_from_stock (QCAD_STOCK_BUS, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (main_window->bus_layout_menu_item) ;
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), main_window->bus_layout_menu_item);

  mnuiSep = gtk_menu_item_new () ;
  gtk_widget_show (mnuiSep) ;
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), mnuiSep) ;
  gtk_widget_set_sensitive (mnuiSep, FALSE) ;

  // create and add the scale menu item to the tools menu //
  scale_menu_item = gtk_menu_item_new_with_mnemonic (_("_Scale All Cells In Design..."));
  gtk_widget_show (scale_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), scale_menu_item);

  // create and add the rotate selection menu item to the tools menu //
  rotate_selection_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Rotate Selection"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (rotate_selection_menu_item),
    img = gtk_image_new_from_stock (QCAD_STOCK_ROTATE_SELECTION, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (rotate_selection_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), rotate_selection_menu_item);

  // create and add a seperator to the tools menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the clock increment menu item to the tools menu //
  clock_increment_menu_item = gtk_menu_item_new_with_mnemonic (_("I_ncrement Cell Clocks"));
  gtk_widget_show (clock_increment_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), clock_increment_menu_item);

  // create and add the clock increment menu item to the tools menu //
  hide_layers_menu_item = gtk_menu_item_new_with_mnemonic (_("_Hide All But The Selected Layer"));
  gtk_widget_show (hide_layers_menu_item);
  gtk_container_add (GTK_CONTAINER (tools_menu_menu), hide_layers_menu_item);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// SIMULATION MENU //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the simulation menu //
  simulation_menu = gtk_menu_item_new_with_mnemonic (_("_Simulation"));
  gtk_widget_show (simulation_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), simulation_menu);

  simulation_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (simulation_menu), simulation_menu_menu);

  // create and add the start simulation menu item to the simulation menu //
  start_simulation_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Start Simulation"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (start_simulation_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (start_simulation_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), start_simulation_menu_item);

  //create and add the stop simulation menu item to the simulation menu //
  stop_simulation_menu_item = gtk_image_menu_item_new_with_mnemonic (_("S_top Simulation"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stop_simulation_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_STOP, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (stop_simulation_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), stop_simulation_menu_item);

  // create and add the show simulation results menu item to the simulatio menu //
  main_window->show_simulation_results_menu_item = gtk_menu_item_new_with_label (_("Simulation Results..."));
  gtk_widget_show (main_window->show_simulation_results_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), main_window->show_simulation_results_menu_item);
  gtk_widget_set_sensitive (GTK_WIDGET (main_window->show_simulation_results_menu_item), FALSE);

#ifdef STDIO_FILEIO
  // create and add a seperator to the simulation menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add save output to file menu item to the simulation menu //
  load_output_from_file_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Open Simulation Results..."));
  gtk_widget_show (load_output_from_file_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), load_output_from_file_menu_item);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (load_output_from_file_menu_item),
    img = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;

  // create and add save output to file menu item to the simulation menu //
  main_window->save_output_to_file_menu_item = gtk_image_menu_item_new_with_mnemonic (_("Save Output To _File..."));
  gtk_widget_show (main_window->save_output_to_file_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), main_window->save_output_to_file_menu_item);
  gtk_widget_set_sensitive (main_window->save_output_to_file_menu_item, FALSE) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (main_window->save_output_to_file_menu_item),
    img = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU)) ;
  gtk_widget_show (img) ;
#endif /* def STDIO_FILEIO */

  // create and add a seperator to the simulation menu //
  mnuiSep = gtk_menu_item_new ();
  gtk_widget_show (mnuiSep);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), mnuiSep);
  gtk_widget_set_sensitive (mnuiSep, FALSE);

  // create and add the simulation type setup menu item to the simulation menu //
  simulation_type_setup_menu_item = gtk_menu_item_new_with_mnemonic (_("Simulation T_ype Setup..."));
  gtk_widget_show (simulation_type_setup_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), simulation_type_setup_menu_item);

  // create and add the simulation engine setup menu item to the simulation menu //
  simulation_engine_setup_menu_item = gtk_menu_item_new_with_mnemonic (_("Simulation _Engine Setup..."));
  gtk_widget_show (simulation_engine_setup_menu_item);
  gtk_container_add (GTK_CONTAINER (simulation_menu_menu), simulation_engine_setup_menu_item);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// HELP MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create and add the help menu //
  help_menu = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_widget_show (help_menu);
  gtk_container_add (GTK_CONTAINER (main_menubar), help_menu);

  help_menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu), help_menu_menu);

	// create and add the contents menu item to the help menu
	contents_menu_item = gtk_image_menu_item_new_with_label (_("Contents"));
	gtk_widget_show (contents_menu_item);
	gtk_container_add (GTK_CONTAINER (help_menu_menu), contents_menu_item);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (contents_menu_item),
    gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU)) ;

  // create and add the about menu item to the help menu //
  about_menu_item = gtk_menu_item_new_with_label (_("About..."));
  gtk_widget_show (about_menu_item);
  gtk_container_add (GTK_CONTAINER (help_menu_menu), about_menu_item);
  gtk_tooltips_set_tip (tooltips, about_menu_item, _("About QCADesigner"), NULL);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// WINDOW WIDGETS //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  main_window->vpaned1 = gtk_vpaned_new () ;
  gtk_widget_show (main_window->vpaned1) ;
  gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->vpaned1, TRUE, TRUE, 0) ;
  gtk_widget_set_size_request (main_window->vpaned1, 1, 1) ;

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_paned_pack1 (GTK_PANED (main_window->vpaned1), hbox1, TRUE, TRUE);

  // create and add the toolbar to the left hand side of the main window //
  main_window->toolbar = gtk_toolbar_new () ;
  gtk_widget_show (main_window->toolbar) ;
  gtk_toolbar_set_orientation (GTK_TOOLBAR (main_window->toolbar), GTK_ORIENTATION_VERTICAL) ;
  gtk_toolbar_set_tooltips (GTK_TOOLBAR (main_window->toolbar), TRUE) ;
  gtk_toolbar_set_style (GTK_TOOLBAR (main_window->toolbar), GTK_TOOLBAR_BOTH_HORIZ) ;
  gtk_box_pack_start (GTK_BOX (hbox1), main_window->toolbar, FALSE, FALSE, 0);
  static int toolbar_styles[2] = {GTK_TOOLBAR_BOTH_HORIZ, GTK_TOOLBAR_TEXT} ;
  connect_object_properties (G_OBJECT (show_tb_icons_menu_item), "active", G_OBJECT (main_window->toolbar), "toolbar-style",
    CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_FROM_BOOLEAN_DATA, toolbar_styles, NULL,
    NULL, NULL, NULL) ;

  // create and add the default button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->default_action_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_SELECT, 
        "label",    _("Select"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Make Selections And Manipulate Design"), _("This button allows you to manipulate your design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_SELECT) ;

  // create and add the type 1 cell button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->insert_type_1_cell_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_CELL, 
        "label",    _("Cell"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Add Cell To Design"), _("Click here, then click on the design to add cells to it.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_QCELL) ;

  // create and add the array button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->insert_cell_array_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_ARRAY, 
        "label",    _("Array"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Add Cell Array"), _("Add horizontal or vertical arrays of cells to your design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_ARRAY) ;

  // create and add the rotate button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->rotate_cell_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_ROTATE_CELL,
        "label",    _("Rotate"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Rotate Cell"), _("Click on a cell to rotate it (repeatedly).")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_ROTATE) ;

  // create and add the alternate display button to the toolbar //
    mnu = gtk_menu_new () ;

    mnui = gtk_image_menu_item_new_with_label (_("Crossover")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL_ALT_CROSSOVER, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_CROSSOVER) ;

    mnuiSep = gtk_menu_item_new () ;
    gtk_widget_show (mnuiSep) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnuiSep) ;

    mnui = gtk_image_menu_item_new_with_label (_("Normal")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_NORMAL) ;

    mnuiSep = gtk_menu_item_new () ;
    gtk_widget_show (mnuiSep) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnuiSep) ;

    mnui = gtk_image_menu_item_new_with_label (_("Vertical")) ;
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_CELL_ALT_VERTICAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
    gtk_widget_show (mnui) ;
    gtk_widget_show (img) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)cell_display_mode_chosen, (gpointer)QCAD_CELL_MODE_VERTICAL) ;

  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->toggle_alt_display_button = toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_CELL_ALT_CROSSOVER, 
        "label",    _("Alt Style"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Alternate Drawing Style"), _("Toggle between two different display modes.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)popup_menu_button_clicked, mnu) ;

  // Create and add the "Add Substrate" button
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->substrate_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_SUBSTRATE,
        "label",    _("Substrate"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Create Substrate"), _("Creates a new rectangular substrate.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_SUBSTRATE) ;

  // Drawing layer buttons
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->label_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_LABEL,
        "label",    _("Label"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Create Label"), _("Creates a new label.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_LABEL) ;

  // Clocking layer buttons
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (main_window->rectangle_electrode_button = toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_RECT_ELECTRODE,
        "label",    _("Rectangle"),
        "group",   main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Rectangular Electrode"), _("Creates a new rectangular electrode.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_RECTANGLE_ELECTRODE) ;

  // This will separate layer-specific commands from generic ones like zoom & pan
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar), GTK_TOOL_ITEM (toolbar_item = GTK_WIDGET (gtk_separator_tool_item_new ())), -1) ;
  gtk_widget_show (toolbar_item) ;

  // create and add the copy button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_COPY,
        "label",    _("Copy"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Copy Selection"), _("Makes a copy of your selection.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_copy_cell_button_clicked, NULL) ;

  // create and add the translate button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_TRANSLATE,
        "label",    _("Translate"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Translate Selection"), _("Translate your selection horizontally and/or vertically.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_translate_selection_button_clicked, NULL) ;

  // create and add the mirror button to the toolbar //
  mnu = gtk_menu_new () ;

  mnui = gtk_image_menu_item_new_with_label (_("Vertically")) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_MIRROR_VERTICAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (mnui) ;
  gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
  g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)mirror_selection_direction_chosen, (gpointer)TRUE) ;

  mnui = gtk_menu_item_new () ;
  gtk_widget_show (mnui) ;
  gtk_container_add (GTK_CONTAINER (mnu), mnui) ;

  mnui = gtk_image_menu_item_new_with_label (_("Horizontally")) ;
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnui),
    img = gtk_image_new_from_stock (QCAD_STOCK_MIRROR_HORIZONTAL, QCAD_ICON_SIZE_SIDE_TOOLBAR)) ;
  gtk_widget_show (img) ;
  gtk_widget_show (mnui) ;
  gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
  g_signal_connect (G_OBJECT (mnui), "activate", (GCallback)mirror_selection_direction_chosen, (gpointer)FALSE) ;

  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_MIRROR_VERTICAL, 
        "label",    _("Mirror"),
        "visible",  TRUE,
        NULL)), -1) ;
//  g_object_set_data (G_OBJECT (toolbar_item), "mnu", mnu) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Mirror Selection"), _("Create a mirrored copy of your selection.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)popup_menu_button_clicked, mnu) ;

  // create and add the measure button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (QCAD_TYPE_RADIO_TOOL_BUTTON,
        "stock-id", QCAD_STOCK_MEASURE,
        "label",    _("Distance"),
        "group",    main_window->default_action_button,
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Measure Distance"), _("Measure the distance from a given point")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)action_button_clicked, (gpointer)ACTION_RULER) ;

  // create and add the zoom + button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_ZOOM_IN,
        "label",    _("Zoom In"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Increase The Magnification"), _("Increase the magnification so you may concentrate on specific parts of the design.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_zoom_in_menu_item_activate, NULL) ;

  // create and add the zoom - button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_ZOOM_OUT,
        "label",    _("Zoom Out"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Decrease The Magnification"), _("Decrease the magnification to get an overall picture.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_zoom_out_menu_item_activate, NULL) ;

  // create and add the zoom extents button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_ZOOM_FIT,
        "label",    _("Extents"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Fit Design To Window"), _("Calculates and sets the magnification such that your design fits inside the window.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_zoom_extents_menu_item_activate, NULL) ;

  // create and add the zoom extents button to the toolbar //
  gtk_toolbar_insert (GTK_TOOLBAR (main_window->toolbar),
    GTK_TOOL_ITEM (toolbar_item = 
      g_object_new (GTK_TYPE_TOOL_BUTTON,
        "stock-id", GTK_STOCK_ZOOM_FIT,
        "label",    _("Layer"),
        "visible",  TRUE,
        NULL)), -1) ;
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbar_item), GTK_TOOLBAR (main_window->toolbar)->tooltips, 
    _("Fit Layer To Window"), _("Calculates and sets the magnification such that the currently topmost layer fits inside the window.")) ;
  g_signal_connect (G_OBJECT (toolbar_item), "clicked", (GCallback)on_zoom_layer_menu_item_activate, NULL) ;

  // create and add the table widget to the hbox //
  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (hbox1), table1, TRUE, TRUE, 0);

  // Add a frame around the drawing area, so it makes a nice border with the other widgets
  main_window->drawing_area_frame = gtk_frame_new (NULL);
  gtk_widget_show (main_window->drawing_area_frame);
  gtk_table_attach (GTK_TABLE (table1), main_window->drawing_area_frame, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (main_window->drawing_area_frame), GTK_SHADOW_IN);

  widget = GTK_WIDGET (g_object_new (GTK_TYPE_HSCROLLBAR, 
    "visible",    TRUE, 
    "adjustment", main_window->adjHScroll = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, PAN_STEP, 10 * PAN_STEP, 0.0)),
    NULL)) ;
  gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  g_signal_connect (G_OBJECT (widget), "adjust-bounds", (GCallback)scrollbar_adjust_bounds, (gpointer)0) ;
  connect_object_properties (G_OBJECT (show_scrollbars_menu_item), "active", G_OBJECT (widget), "visible",
    CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL,
    CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

  widget = GTK_WIDGET (g_object_new (GTK_TYPE_VSCROLLBAR, 
    "visible",    TRUE, 
    "adjustment", main_window->adjVScroll = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, PAN_STEP, 10 * PAN_STEP, 0.0)),
    NULL)) ;
  gtk_table_attach (GTK_TABLE (table1), widget, 2, 3, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  g_signal_connect (G_OBJECT (widget), "adjust-bounds", (GCallback)scrollbar_adjust_bounds, (gpointer)1) ;
  connect_object_properties (G_OBJECT (show_scrollbars_menu_item), "active", G_OBJECT (widget), "visible",
    CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL,
    CONNECT_OBJECT_PROPERTIES_ASSIGN, NULL, NULL) ;

  // create and add the drawing area to the table //
  // this is the widget where all the action happens //
  main_window->drawing_area = gtk_drawing_area_new ();
  GTK_WIDGET_SET_FLAGS (main_window->drawing_area, GTK_CAN_FOCUS);
  gtk_widget_show (main_window->drawing_area);
  gtk_container_add (GTK_CONTAINER (main_window->drawing_area_frame), main_window->drawing_area) ;
  set_widget_background_colour (main_window->drawing_area, 0, 0, 0) ;
  gtk_widget_set_double_buffered (main_window->drawing_area, FALSE) ;
  gtk_widget_set_redraw_on_allocate (main_window->drawing_area, FALSE) ;

  // create and add the horizontal ruler to the table //
  // purpose to provide a real time ruler for measuring the design //
  main_window->horizontal_ruler = gtk_hruler_new ();
  gtk_widget_show (main_window->horizontal_ruler);
  gtk_table_attach (GTK_TABLE (table1), main_window->horizontal_ruler, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_ruler_set_metric (GTK_RULER (main_window->horizontal_ruler), GTK_PIXELS) ;
  gtk_ruler_set_range (GTK_RULER (main_window->horizontal_ruler), 0, 100, 0, 1);

  // create and add the vertical ruler to the table //
  // purpose to provide a real time ruler for measuring the design //
  main_window->vertical_ruler = gtk_vruler_new ();
  gtk_widget_show (main_window->vertical_ruler);
  gtk_table_attach (GTK_TABLE (table1), main_window->vertical_ruler, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_ruler_set_metric (GTK_RULER (main_window->vertical_ruler), GTK_PIXELS) ;
  gtk_ruler_set_range (GTK_RULER (main_window->vertical_ruler), 0, 100, 0, 1);

  // create and add the command history text box to the scrolled window //
  command_history = command_history_create () ;
  gtk_widget_show (command_history);
  gtk_paned_pack2 (GTK_PANED (main_window->vpaned1), command_history, TRUE, TRUE);

  tbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_box_pack_start (GTK_BOX (vbProgress), tbl, FALSE, FALSE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  progress = create_labelled_progress_bar () ;
  gtk_table_attach (GTK_TABLE (tbl), progress, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;

  // create and add tbe status bar to the main window //
  status_bar = gtk_statusbar_new ();
  gtk_widget_show (status_bar);
  gtk_table_attach (GTK_TABLE (tbl), status_bar, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 0, 0) ;
#ifdef STDIO_FILEIO
  gtk_timeout_add (60000, (GtkFunction)autosave_timer_event, NULL) ;
#endif /* def STDIO_FILEIO */

////////////////////////////////////////////////////////////////////////////////////////
// Set up accelerators for the various menu items                                   ////
////////////////////////////////////////////////////////////////////////////////////////

#ifdef STDIO_FILEIO
  gtk_widget_add_accelerator (save_as_menu_item,                 "activate", GTK_ACCEL_GROUP (accel_group), GDK_s,      GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
#endif /* def STDIO_FILEIO */
#ifdef UNDO_REDO
  gtk_widget_add_accelerator (main_window->undo_menu_item,       "activate", GTK_ACCEL_GROUP (accel_group), GDK_z,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (main_window->redo_menu_item,       "activate", GTK_ACCEL_GROUP (accel_group), GDK_z,      GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
#endif /* def UNDO_REDO */
  gtk_widget_add_accelerator (delete_menu_item,                  "activate", GTK_ACCEL_GROUP (accel_group), GDK_Delete, 0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_in_menu_item,                 "activate", GTK_ACCEL_GROUP (accel_group), GDK_w,      0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_in_menu_item,                 "activate", GTK_ACCEL_GROUP (accel_group), GDK_plus,   0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_out_menu_item,                "activate", GTK_ACCEL_GROUP (accel_group), GDK_q,      0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_out_menu_item,                "activate", GTK_ACCEL_GROUP (accel_group), GDK_minus,  0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_extents_menu_item,            "activate", GTK_ACCEL_GROUP (accel_group), GDK_e,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (reset_zoom_menu_item,              "activate", GTK_ACCEL_GROUP (accel_group), GDK_1,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoom_layer_menu_item,              "activate", GTK_ACCEL_GROUP (accel_group), GDK_l,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (rotate_selection_menu_item ,       "activate", GTK_ACCEL_GROUP (accel_group), GDK_r,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (clock_increment_menu_item,         "activate", GTK_ACCEL_GROUP (accel_group), GDK_i,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (start_simulation_menu_item,        "activate", GTK_ACCEL_GROUP (accel_group), GDK_m,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (stop_simulation_menu_item,         "activate", GTK_ACCEL_GROUP (accel_group), GDK_t,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (contents_menu_item,                "activate", GTK_ACCEL_GROUP (accel_group), GDK_F1,     0,                                 GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (hide_layers_menu_item,             "activate", GTK_ACCEL_GROUP (accel_group), GDK_h,      GDK_CONTROL_MASK,                  GTK_ACCEL_VISIBLE);

////////////////////////////////////////////////////////////////////////////////////////
// Connect the callback signals to each of the buttons and menu items in the menus  ////
////////////////////////////////////////////////////////////////////////////////////////

  g_signal_connect (G_OBJECT (new_menu_item),                                  "activate", (GCallback)file_operations, (gpointer)FILEOP_NEW);
  g_signal_connect (G_OBJECT (close_menu_item),                                "activate", (GCallback)file_operations, (gpointer)FILEOP_CLOSE);
#ifdef STDIO_FILEIO
  g_signal_connect (G_OBJECT (open_menu_item),                                 "activate", (GCallback)file_operations, (gpointer)FILEOP_OPEN);
  g_signal_connect (G_OBJECT (save_menu_item),                                 "activate", (GCallback)file_operations, (gpointer)FILEOP_SAVE);
  g_signal_connect (G_OBJECT (save_as_menu_item),                              "activate", (GCallback)file_operations, (gpointer)FILEOP_SAVE_AS);
  g_signal_connect (G_OBJECT (create_block_menu_item),                         "activate", (GCallback)file_operations, (gpointer)FILEOP_EXPORT);
  g_signal_connect (G_OBJECT (import_block_menu_item),                         "activate", (GCallback)file_operations, (gpointer)FILEOP_IMPORT);
  g_signal_connect (G_OBJECT (load_output_from_file_menu_item),                "activate", (GCallback)on_load_output_from_file_menu_item_activate,   NULL);
  g_signal_connect (G_OBJECT (main_window->save_output_to_file_menu_item),     "activate", (GCallback)on_save_output_to_file_menu_item_activate,     NULL);
  g_signal_connect (G_OBJECT (preview_menu_item),                              "activate", (GCallback)on_preview_menu_item_activate,                 NULL);
#endif /* def STDIO_FILEIO */
#ifdef UNDO_REDO
  g_signal_connect (G_OBJECT (main_window->undo_menu_item),                    "activate", (GCallback)on_undo_menu_item_activate,                    NULL);
  g_signal_connect (G_OBJECT (main_window->redo_menu_item),                    "activate", (GCallback)on_redo_menu_item_activate,                     NULL);
#endif /* def UNDO_REDO */
  g_signal_connect (G_OBJECT (hide_layers_menu_item),                          "activate", (GCallback)on_hide_layers_menu_item_activate,             NULL);
  g_signal_connect (G_OBJECT (print_menu_item),                                "activate", (GCallback)on_print_menu_item_activate,                   NULL);
  g_signal_connect (G_OBJECT (delete_menu_item),                               "activate", (GCallback)on_delete_menu_item_activate,                  NULL);
  g_signal_connect (G_OBJECT (layer_properties_menu_item),                     "activate", (GCallback)layer_properties_button_clicked,               NULL);
  g_signal_connect (G_OBJECT (main_window->snap_to_grid_menu_item),            "activate", (GCallback)on_snap_to_grid_menu_item_activate,            NULL);
  g_signal_connect (G_OBJECT (scale_menu_item),                                "activate", (GCallback)on_scale_menu_item_activate,                   NULL);
  g_signal_connect (G_OBJECT (rotate_selection_menu_item),                     "activate", (GCallback)rotate_selection_menu_item_activate,           NULL);
  g_signal_connect (G_OBJECT (clock_increment_menu_item),                      "activate", (GCallback)on_clock_increment_menu_item_activate,         NULL);
  g_signal_connect (G_OBJECT (start_simulation_menu_item),                     "activate", (GCallback)on_start_simulation_menu_item_activate,        NULL);
  g_signal_connect (G_OBJECT (stop_simulation_menu_item),                      "activate", (GCallback)on_stop_simulation_menu_item_activate,         NULL);
  g_signal_connect (G_OBJECT (main_window->show_simulation_results_menu_item), "activate", (GCallback)on_show_simulation_results_menu_item_activate, NULL);
  g_signal_connect (G_OBJECT (simulation_type_setup_menu_item),                "activate", (GCallback)on_simulation_type_setup_menu_item_activate,   NULL);
  g_signal_connect (G_OBJECT (simulation_engine_setup_menu_item),              "activate", (GCallback)on_simulation_engine_setup_menu_item_activate, NULL);
  g_signal_connect (G_OBJECT (reset_zoom_menu_item),                           "activate", (GCallback)on_reset_zoom_menu_item_activate,              NULL);
  g_signal_connect (G_OBJECT (zoom_in_menu_item),                              "activate", (GCallback)on_zoom_in_menu_item_activate,                 NULL);
  g_signal_connect (G_OBJECT (zoom_out_menu_item),                             "activate", (GCallback)on_zoom_out_menu_item_activate,                NULL);
  g_signal_connect (G_OBJECT (zoom_extents_menu_item),                         "activate", (GCallback)on_zoom_extents_menu_item_activate,            NULL);
  g_signal_connect (G_OBJECT (zoom_layer_menu_item),                           "activate", (GCallback)on_zoom_layer_menu_item_activate,              NULL);
  g_signal_connect (G_OBJECT (contents_menu_item),                             "activate", (GCallback)on_contents_menu_item_activate,                NULL);
  g_signal_connect (G_OBJECT (main_window->bus_layout_menu_item),              "activate", (GCallback)bus_layout_button_clicked,                     NULL);
  g_signal_connect (G_OBJECT (about_menu_item),                                "activate", (GCallback)on_about_menu_item_activate,                   NULL);
  g_signal_connect (G_OBJECT (main_window->main_window),                       "show",     (GCallback)main_window_show,                              NULL);

  g_signal_connect_swapped (G_OBJECT (quit_menu_item),                         "activate", (GCallback)on_quit_menu_item_activate, main_window->main_window);
  // attach the necessary signals to the drawing area widget //
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "scroll-event",         (GCallback)scroll_event,                      NULL);
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "expose-event",         (GCallback)expose_event,                      NULL);
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "motion-notify-event",  (GCallback)drawing_area_motion_notify,        NULL);
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "button-press-event",   (GCallback)drawing_area_button_pressed,       NULL);
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "button-release-event", (GCallback)drawing_area_button_released,      NULL);
  g_signal_connect (G_OBJECT (main_window->drawing_area),                      "configure-event",      (GCallback)configure_event,                   NULL);

  // -- Trap the act of hiding the popup window for the layers combo
  g_signal_connect (G_OBJECT (GTK_COMBO (main_window->layers_combo)->popwin),  "hide", (GCallback)layer_selected, NULL) ;
  // -- Connect the shutdown callback signal to the main window -- //
  g_signal_connect_swapped (G_OBJECT (main_window->main_window),               "delete-event", (GCallback)on_quit_menu_item_activate, main_window->main_window);
  qcad_object_connect_signal_to_default_object (QCAD_TYPE_CELL, "notify::clock", (GCallback)qcad_cell_default_clock_changed, NULL) ;
////////////////////////////////////////////////////////////////////////////////////////
// Connect the callback signals to each of the buttons and menu items in the menus  ////
////////////////////////////////////////////////////////////////////////////////////////

  gtk_window_add_accel_group (GTK_WINDOW (main_window->main_window), GTK_ACCEL_GROUP (accel_group));
// activate the necessary events for the drawing area such as expose, mouse motion, mouse click, etc //
  gtk_widget_set_events (GTK_WIDGET (main_window->drawing_area),
    GDK_EXPOSURE_MASK     | GDK_LEAVE_NOTIFY_MASK   |
    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
    GDK_KEY_PRESS_MASK    | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
}
