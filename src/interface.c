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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// Standard Includes //

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

// GTK includes //

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

// QCADesigner includes //
#include "support.h"
#include "callbacks.h"
#include "actions/action_handlers.h"
#include "interface.h"
#include "custom_widgets.h"
#include "command_history.h"
#include "global_consts.h"

// -- creates the main application window and returns a pointer to it -- //

main_W main_window = {NULL} ;

void create_main_window (main_W *main_window){
    GtkWidget *img = NULL ;
    GtkRcStyle *rcstyle = NULL ;
    char *psz = NULL ;
    // All the objects that appear in the main window //

    main_window->tooltips = gtk_tooltips_new ();

    main_window->accel_group = gtk_accel_group_new ();

    // create the main window //
    main_window->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data (GTK_OBJECT (main_window->main_window), "main_window", main_window->main_window);
    gtk_window_set_default_size (GTK_WINDOW (main_window->main_window), 1024, 768);
    psz = g_strdup_printf ("%s - %s", _("Untitled"), MAIN_WND_BASE_TITLE) ;
    gtk_window_set_title (GTK_WINDOW (main_window->main_window), psz);
    g_free (psz) ;

    // create the vertical box and add it to the main window //
    main_window->vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (main_window->vbox1);
    gtk_container_add (GTK_CONTAINER (main_window->main_window), main_window->vbox1);

    // create and add the main menubar to the main window //
    main_window->main_menubar = gtk_menu_bar_new ();
    gtk_widget_show (main_window->main_menubar);
    gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->main_menubar, FALSE, FALSE, 0);
/*
    // Layers toolbar
    main_window->layers_toolbar = gtk_toolbar_new () ;
    gtk_widget_show (main_window->layers_toolbar) ;
    gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->layers_toolbar, FALSE, FALSE, 0);
    gtk_toolbar_set_orientation (GTK_TOOLBAR (main_window->layers_toolbar), GTK_ORIENTATION_HORIZONTAL) ;
    gtk_toolbar_set_tooltips (GTK_TOOLBAR (main_window->layers_toolbar), TRUE) ;
    
    main_window->layers_combo = gtk_combo_new () ;
    gtk_widget_show (main_window->layers_combo) ;
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (main_window->layers_combo)->entry), FALSE) ;
    gtk_container_set_border_width (GTK_CONTAINER (main_window->layers_combo), 2) ;
    gtk_toolbar_append_element (
      GTK_TOOLBAR (main_window->layers_toolbar),
      GTK_TOOLBAR_CHILD_WIDGET,
      main_window->layers_combo,
      "",
      _("Layers"),
      _("Lists the layers in the current design and allows you to switch between them."),
      NULL,
      NULL,
      NULL) ;
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// FILE MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

    // create and add the file menu to the menubar //
    main_window->file_menu = gtk_menu_item_new_with_mnemonic (_("_File"));
    gtk_widget_show (main_window->file_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->file_menu);

    main_window->file_menu_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->file_menu), main_window->file_menu_menu);
//    main_window->file_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->file_menu_menu));

	// create and add the New project file menu item to the menubar //
    main_window->new_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, main_window->accel_group);
    gtk_widget_show (main_window->new_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->new_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->new_menu_item, _("Create a new project file"), NULL);

	// create and add the open project menu item to the menubar //
    main_window->open_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, main_window->accel_group);
    gtk_widget_show (main_window->open_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->open_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->open_menu_item, _("Open a project file"), NULL);

	// create and add the save menu item to the menubar //
    main_window->save_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, main_window->accel_group);
    gtk_widget_show (main_window->save_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->save_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->save_menu_item, _("Save current project"), NULL);

	// create and add the save as meny item to the menubar //
    main_window->save_as_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE_AS, main_window->accel_group);
    gtk_widget_show (main_window->save_as_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->save_as_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->save_as_menu_item, _("Save project file as ..."), NULL);

	// create and add a seperator to the file menu //
    main_window->separator2 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator2);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->separator2);
    gtk_widget_set_sensitive (main_window->separator2, FALSE);

	// create and add the print menu item to the menu bar //
    main_window->print_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PRINT, main_window->accel_group);
    gtk_widget_show (main_window->print_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->print_menu_item);
    
//    gtk_widget_set_sensitive (main_window->print_menu_item, FALSE) ;

	// create and add the preview item to the menu bar //
    main_window->preview_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PRINT_PREVIEW, main_window->accel_group);
    gtk_widget_show (main_window->preview_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->preview_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->preview_menu_item, _("Preview the print layout"), NULL);

	// create and add a seperator to the file menu //
    main_window->separator3 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator3);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->separator3);
    gtk_widget_set_sensitive (main_window->separator3, FALSE);

	// create and add the project properties menu item to the menubar //
    main_window->project_properties_menu_item = gtk_menu_item_new_with_label (_("Project Properties..."));
    gtk_widget_show (main_window->project_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->project_properties_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->project_properties_menu_item, _("Show project properties"), NULL);

	// create and add the close menu item to the file menu //
    main_window->close_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, main_window->accel_group);
    gtk_widget_show (main_window->close_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->close_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->close_menu_item, _("Close the project file"), NULL);

	// create and add a seperator to the file menu //
    main_window->mnuSepPreRecentFiles = gtk_menu_item_new ();
    gtk_widget_show (main_window->mnuSepPreRecentFiles);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->mnuSepPreRecentFiles);
    gtk_widget_set_sensitive (main_window->mnuSepPreRecentFiles, FALSE);
    
    main_window->recent_files_menu_item = gtk_menu_item_new_with_mnemonic (_("_Recent Files")) ;
    gtk_widget_show (main_window->recent_files_menu_item) ;
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->recent_files_menu_item) ;

    main_window->recent_files_menu = gtk_menu_new () ;
    gtk_widget_show (main_window->recent_files_menu) ;
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->recent_files_menu_item), main_window->recent_files_menu) ;

	// create and add a seperator to the file menu //
    main_window->mnuSepPostRecentFiles = gtk_menu_item_new ();
    gtk_widget_show (main_window->mnuSepPostRecentFiles);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->mnuSepPostRecentFiles);
    gtk_widget_set_sensitive (main_window->mnuSepPostRecentFiles, FALSE);

	// create and add the quit menu item to the file menu //
    main_window->quit_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, main_window->accel_group);
    gtk_widget_show (main_window->quit_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->quit_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->quit_menu_item, _("Quit QCADesigner"), NULL);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// EDIT MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the edit menu to the menubar //
    main_window->edit_menu = gtk_menu_item_new_with_mnemonic (_("_Edit"));
    gtk_widget_show (main_window->edit_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->edit_menu);

    main_window->edit_menu_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->edit_menu), main_window->edit_menu_menu);
//    main_window->edit_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->edit_menu_menu));

    // create and add the undo menu item to the menu bar //
    main_window->undo_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_UNDO, main_window->accel_group);
    gtk_widget_show (main_window->undo_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->undo_menu_item);
    gtk_widget_add_accelerator (main_window->undo_menu_item, "activate", main_window->accel_group, GDK_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // initially, there is nothing to undo //
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->undo_menu_item), FALSE);

    //create and add the redu menu item to the menu bar //
    main_window->redo_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REDO, main_window->accel_group);
    gtk_widget_show (main_window->redo_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->redo_menu_item);
    gtk_widget_add_accelerator (main_window->redo_menu_item, "activate", main_window->accel_group, GDK_z, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    // initially, there's nothing to redo //
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->redo_menu_item), FALSE);

	// create and add a seperator to the edit menu //
    main_window->separator7 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator7);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->separator7);
    gtk_widget_set_sensitive (main_window->separator7, FALSE);

	// create and add the copy menu item to the edit menu //
    main_window->copy_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_COPY, main_window->accel_group);
    gtk_widget_show (main_window->copy_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->copy_menu_item);
//    gtk_widget_add_accelerator (main_window->copy_menu_item, "activate", main_window->accel_group, GDK_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->copy_menu_item), FALSE);

	// create and add the cut menu item to the edit menu //
    main_window->cut_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CUT, main_window->accel_group);
    gtk_widget_show (main_window->cut_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->cut_menu_item);
//    gtk_widget_add_accelerator (main_window->cut_menu_item, "activate", main_window->accel_group, GDK_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->cut_menu_item), FALSE);

	// create and add the paste item to the edit menu //
    main_window->paste_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PASTE, main_window->accel_group);
    gtk_widget_show (main_window->paste_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->paste_menu_item);
//    gtk_widget_add_accelerator (main_window->paste_menu_item, "activate", main_window->accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// create and add the delete item to the edit menu //
    main_window->delete_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, main_window->accel_group);
    gtk_widget_show (main_window->delete_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->delete_menu_item);
    gtk_widget_add_accelerator (main_window->delete_menu_item, "activate", main_window->accel_group, GDK_Delete, 0, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->paste_menu_item), FALSE);

	// create and add a seperator to the edit menu //
    main_window->separator6 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator6);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->separator6);
    gtk_widget_set_sensitive (main_window->separator6, FALSE);

	// create and add the grid properties menu item to the edit menu //
    main_window->grid_properties_menu_item = gtk_menu_item_new_with_label (_("Grid Properties..."));
    gtk_widget_show (main_window->grid_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->grid_properties_menu_item);

	// create and add the cell properties menu item to the edit menu //
    main_window->cell_properties_menu_item = gtk_menu_item_new_with_label (_("Cell Properties..."));
    gtk_widget_show (main_window->cell_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->cell_properties_menu_item);
/*
	// create and add the window properties menu item to the edit menu //
    main_window->window_properties_menu_item = gtk_menu_item_new_with_label (_("Window Properties"));
    gtk_widget_show (main_window->window_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->window_properties_menu_item);

    // *********** remove when finished **************** //
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->window_properties_menu_item), FALSE);

	// create and add the layer properties menu item to the edit menu //
    main_window->layer_properties_menu_item = gtk_menu_item_new_with_label (_("Layer Properties"));
    gtk_widget_show (main_window->layer_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->layer_properties_menu_item);

    // *********** remove when finished **************** //
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->layer_properties_menu_item), FALSE);
*/
	// create and add the preferences menu item to the edit menu //
    main_window->preferences_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, main_window->accel_group);
    gtk_widget_show (main_window->preferences_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->preferences_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->preferences_menu_item), FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// VIEW MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// create and add the view menu //
	main_window->view_menu = gtk_menu_item_new_with_mnemonic (_("_View"));
	gtk_widget_show (main_window->view_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->view_menu);
	
	main_window->view_menu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->view_menu), main_window->view_menu_menu);
//	main_window->view_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->view_menu_menu));
	
	// create and add the zoom in menu item to the view menu //
	main_window->zoom_in_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ZOOM_IN, main_window->accel_group);
	gtk_widget_show (main_window->zoom_in_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_in_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_in_menu_item, "activate", main_window->accel_group, GDK_w, 0, GTK_ACCEL_VISIBLE);
	
	// create and add the zoom out menu item to the view menu //
	main_window->zoom_out_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ZOOM_OUT, main_window->accel_group);
	gtk_widget_show (main_window->zoom_out_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_out_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_out_menu_item, "activate", main_window->accel_group, GDK_q, 0, GTK_ACCEL_VISIBLE);

	// create and add the zoom die menu item to the view menu //
	main_window->zoom_die_menu_item = gtk_menu_item_new_with_label (_("Zoom Die"));
	gtk_widget_show (main_window->zoom_die_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_die_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_die_menu_item, "activate", main_window->accel_group, GDK_d, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// create and add the zoom extents menu item to the view menu //
	main_window->zoom_extents_menu_item = gtk_image_menu_item_new_with_label (_("Zoom Extents"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (main_window->zoom_extents_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_ZOOM_FIT, GTK_ICON_SIZE_MENU)) ;
	gtk_widget_show (img) ;
	gtk_widget_show (main_window->zoom_extents_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_extents_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_extents_menu_item, "activate", main_window->accel_group, GDK_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // create and add a seperator to the view menu //
    main_window->separator4 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator4);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->separator4);
    gtk_widget_set_sensitive (main_window->separator4, FALSE);

    // create and add the show toolbar icons menu item to the view menu //
    main_window->show_tb_icons_menu_item = gtk_check_menu_item_new_with_label (_("Show Toolbar Icons"));
    gtk_widget_show (main_window->show_tb_icons_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->show_tb_icons_menu_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window->show_tb_icons_menu_item), TRUE);
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->show_tb_icons_menu_item), TRUE);

    // create and add a seperator to the view menu //
    main_window->separator4 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator4);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->separator4);
    gtk_widget_set_sensitive (main_window->separator4, FALSE);

    // create and add the snap to grid menu item to the view menu //
    main_window->snap_to_grid_menu_item = gtk_check_menu_item_new_with_label (_("Snap To Grid"));
    gtk_widget_show (main_window->snap_to_grid_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->snap_to_grid_menu_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window->snap_to_grid_menu_item), TRUE);
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->snap_to_grid_menu_item), TRUE);

    // create and add the show grid menu item to the view menu //
    main_window->show_grid_menu_item = gtk_check_menu_item_new_with_label (_("Show Grid"));
    gtk_widget_show (main_window->show_grid_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->show_grid_menu_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(main_window->show_grid_menu_item), TRUE);
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->show_grid_menu_item), TRUE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// TOOLS MENU ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the tools menu //
    main_window->tools_menu = gtk_menu_item_new_with_mnemonic (_("_Tools"));
    gtk_widget_show (main_window->tools_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->tools_menu);

    main_window->tools_menu_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->tools_menu), main_window->tools_menu_menu);
//    main_window->tools_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->tools_menu_menu));

	// create and add the create block menu item to the tools menu //
    main_window->create_block_menu_item = gtk_menu_item_new_with_label (_("Create Block..."));
    gtk_widget_show (main_window->create_block_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->create_block_menu_item);

	// create and add the import block menu item to the tools menu //
    main_window->import_block_menu_item = gtk_menu_item_new_with_label (_("Import Block..."));
    gtk_widget_show (main_window->import_block_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->import_block_menu_item);

	// create and add a seperator to the tools menu //
    main_window->separator8 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator8);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator8);
    gtk_widget_set_sensitive (main_window->separator8, FALSE);

	// create and add the scale menu item to the tools menu //
    main_window->scale_menu_item = gtk_menu_item_new_with_label (_("Scale All Cells In Design..."));
    gtk_widget_show (main_window->scale_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->scale_menu_item);

	// create and add the create input menu item to the tools menu //
    main_window->rotate_selection_menu_item = gtk_menu_item_new_with_label (_("Rotate Selection"));
    gtk_widget_show (main_window->rotate_selection_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->rotate_selection_menu_item);

	// create and add a seperator to the tools menu //
    main_window->separator8 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator8);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator8);
    gtk_widget_set_sensitive (main_window->separator8, FALSE);
    
        // create and add the cell functions menu item to the tools menu //
    main_window->cell_function_menu_item = gtk_menu_item_new_with_label (_("Cell Functions...")) ;
    gtk_widget_show (main_window->cell_function_menu_item) ;
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->cell_function_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->cell_function_menu_item, 
      _("Modify a cell's function (Normal/Input/Output/Fixed)"), NULL);

	// create and add a seperator to the tools menu //
    main_window->separator9 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator9);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator9);
    gtk_widget_set_sensitive (main_window->separator9, FALSE);
    
	// create and add the clock select menu item to the tools menu //
    main_window->clock_select_menu_item = gtk_menu_item_new_with_label (_("Select Clock..."));
    gtk_widget_show (main_window->clock_select_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->clock_select_menu_item);

	// create and add the clock increment menu item to the tools menu //
    main_window->clock_increment_menu_item = gtk_menu_item_new_with_label (_("Increment Cell Clocks"));
    gtk_widget_show (main_window->clock_increment_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->clock_increment_menu_item);
    gtk_widget_add_accelerator (main_window->clock_increment_menu_item, "activate", main_window->accel_group, GDK_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// create and add a seperator to the tools menu //
    main_window->separator11 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator11);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator11);
    gtk_widget_set_sensitive (main_window->separator11, FALSE);

	// create and add the measure distance menu item to the tools menu //
    main_window->measure_distance_menu_item = gtk_menu_item_new_with_label (_("Measure Distance"));
    gtk_widget_show (main_window->measure_distance_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->measure_distance_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->measure_distance_menu_item), FALSE);

	// create and add the mesurement preferences menu item to the tools menu //
    main_window->measurement_preferences1 = gtk_menu_item_new_with_label (_("Measurement Preferences"));
    gtk_widget_show (main_window->measurement_preferences1);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->measurement_preferences1);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->measurement_preferences1), FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// DRAW MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the draw menu //
    main_window->draw_menu = gtk_menu_item_new_with_mnemonic (_("_Draw"));
    gtk_widget_show (main_window->draw_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->draw_menu);
    gtk_widget_set_sensitive (main_window->draw_menu, FALSE) ;

    main_window->draw_menu_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->draw_menu), main_window->draw_menu_menu);
//    main_window->draw_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->draw_menu_menu));

	// create and add the draw dimensions menu item to the draw menu //
    main_window->draw_dimensions_menu_item = gtk_menu_item_new_with_label (_("Draw Dimensions"));
    gtk_widget_show (main_window->draw_dimensions_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_dimensions_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_dimensions_menu_item), FALSE);

	// create and add the dimension properties menu item to the draw menu //
    main_window->dimension_properties1 = gtk_menu_item_new_with_label (_("Dimension Properties"));
    gtk_widget_show (main_window->dimension_properties1);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->dimension_properties1);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->dimension_properties1), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator16 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator16);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator16);
    gtk_widget_set_sensitive (main_window->separator16, FALSE);

	// create and add the draw text menu item to the draw menu //
    main_window->draw_text_menu_item = gtk_menu_item_new_with_label (_("Draw Text"));
    gtk_widget_show (main_window->draw_text_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_text_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_text_menu_item), FALSE);

	// create and add the text properties menu item to the draw menu //
    main_window->text_properties_menu_item = gtk_menu_item_new_with_label (_("Text Properties"));
    gtk_widget_show (main_window->text_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->text_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->text_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator10 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator10);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator10);
    gtk_widget_set_sensitive (main_window->separator10, FALSE);

	// create and add the draw arrow menu item to the draw menu //
    main_window->draw_arrow_menu_item = gtk_menu_item_new_with_label (_("Draw Arrow"));
    gtk_widget_show (main_window->draw_arrow_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_arrow_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_arrow_menu_item), FALSE);

	// create and add the arrow properties menu item to the draw menu //
    main_window->arrow_properties_menu_item = gtk_menu_item_new_with_label (_("Arrow Properties"));
    gtk_widget_show (main_window->arrow_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->arrow_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->arrow_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator14 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator14);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator14);
    gtk_widget_set_sensitive (main_window->separator14, FALSE);

	// create and add the draw line menu item to the draw menu //
    main_window->draw_line_menu_item = gtk_menu_item_new_with_label (_("Draw Line"));
    gtk_widget_show (main_window->draw_line_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_line_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_line_menu_item), FALSE);

	// create and add the line properties menu item to the draw menu //
    main_window->line_properties_menu_item = gtk_menu_item_new_with_label (_("Line Properties"));
    gtk_widget_show (main_window->line_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->line_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->line_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator15 = gtk_menu_item_new ();
    gtk_widget_show (main_window->separator15);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator15);
    gtk_widget_set_sensitive (main_window->separator15, FALSE);

	// create and add the draw rectangle menu item to the draw menu //
    main_window->draw_rectangle_menu_item = gtk_menu_item_new_with_label (_("Draw Rectangle"));
    gtk_widget_show (main_window->draw_rectangle_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_rectangle_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_rectangle_menu_item), FALSE);

	// create and add the rectangle properties menu item to the draw menu //
    main_window->rectangle_properties_menu_item = gtk_menu_item_new_with_label (_("Rectangle Properties"));
    gtk_widget_show (main_window->rectangle_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->rectangle_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->rectangle_properties_menu_item),FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// SIMULATION MENU //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the simulation menu //
	main_window->simulation_menu = gtk_menu_item_new_with_mnemonic (_("_Simulation"));
	gtk_widget_show (main_window->simulation_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->simulation_menu);
	
	main_window->simulation_menu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->simulation_menu), main_window->simulation_menu_menu);
//	main_window->simulation_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->simulation_menu_menu));
	
	// create and add the start simulation menu item to the simulation menu //
	main_window->start_simulation_menu_item = gtk_image_menu_item_new_with_label (_("Start Simulation"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (main_window->start_simulation_menu_item), img = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU)) ;
	gtk_widget_show (img) ;
	gtk_widget_show (main_window->start_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->start_simulation_menu_item);
//	gtk_widget_add_accelerator (main_window->start_simulation_menu_item, "activate", main_window->accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	//create and add the stop simulation menu item to the simulation menu //
	main_window->stop_simulation_menu_item = gtk_menu_item_new_with_label (_("Stop Simulation"));
//	gtk_widget_show (main_window->stop_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->stop_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->stop_simulation_menu_item, "activate", main_window->accel_group, GDK_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->stop_simulation_menu_item), FALSE);
	
	// create and add the pause simulation menu item to the simulatio menu //
	main_window->pause_simulation_menu_item = gtk_menu_item_new_with_label (_("Pause Simulation"));
//	gtk_widget_show (main_window->pause_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->pause_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->pause_simulation_menu_item, "activate", main_window->accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->pause_simulation_menu_item), FALSE);
	
	// create and add the reset simulation menu item to the simulation menu //
	main_window->reset_simulation_menu_item = gtk_menu_item_new_with_label (_("Reset Simulation"));
//	gtk_widget_show (main_window->reset_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->reset_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->reset_simulation_menu_item, "activate", main_window->accel_group, GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->reset_simulation_menu_item), FALSE);

	// create and add the calculate ground state menu item to the simulation menu //
	main_window->calculate_ground_state_menu_item = gtk_menu_item_new_with_label (_("Calculate Ground State"));
//	gtk_widget_show (main_window->calculate_ground_state_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->calculate_ground_state_menu_item);
	gtk_widget_add_accelerator (main_window->calculate_ground_state_menu_item, "activate", main_window->accel_group, GDK_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
        // Disable this until it's properly implemented
        gtk_widget_set_sensitive (main_window->calculate_ground_state_menu_item, FALSE) ;

	// create and add the animate_test_simulation menu item to the simulation menu //
	main_window->animate_test_simulation_menu_item = gtk_menu_item_new_with_label (_("Animate Test Simulation"));
//	gtk_widget_show (main_window->animate_test_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->animate_test_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->animate_test_simulation_menu_item, "activate", main_window->accel_group, GDK_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
        
    // Disable this until it's properly implemented
    gtk_widget_set_sensitive (main_window->animate_test_simulation_menu_item, FALSE) ;

	// create and add a seperator to the simulation menu //
	main_window->separator12 = gtk_menu_item_new ();
	gtk_widget_show (main_window->separator12);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->separator12);
	gtk_widget_set_sensitive (main_window->separator12, FALSE);
	
	// create and add save output to file menu item to the simulation menu //
	main_window->save_output_to_file_menu_item = gtk_menu_item_new_with_label (_("Save Output To File"));
//	gtk_widget_show (main_window->save_output_to_file_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->save_output_to_file_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->save_output_to_file_menu_item), FALSE);
	
	// create and add a seperator to the simulation menu //
	main_window->separator13 = gtk_menu_item_new ();
//	gtk_widget_show (main_window->separator13);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->separator13);
	gtk_widget_set_sensitive (main_window->separator13, FALSE);
	
	// create and add the logging properties menu item to the simulation menu //
	main_window->logging_properties_menu_item = gtk_menu_item_new_with_label (_("Logging Properties"));
//	gtk_widget_show (main_window->logging_properties_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->logging_properties_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->logging_properties_menu_item), FALSE);
	
	// create and add the simulation type setup menu item to the simulation menu //
	main_window->simulation_type_setup_menu_item = gtk_menu_item_new_with_label (_("Simulation Type Setup..."));
	gtk_widget_show (main_window->simulation_type_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->simulation_type_setup_menu_item);
	
	// create and add the simulation engine setup menu item to the simulation menu //
	main_window->simulation_engine_setup_menu_item = gtk_menu_item_new_with_label (_("Simulation Engine Setup..."));
	gtk_widget_show (main_window->simulation_engine_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->simulation_engine_setup_menu_item);
	
	// create and add the random fault setup menu item to the simulation menu //
	main_window->random_fault_setup_menu_item = gtk_menu_item_new_with_label (_("Random Fault Setup..."));
	gtk_widget_show (main_window->random_fault_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->random_fault_setup_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->random_fault_setup_menu_item), FALSE);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// HELP MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// create and add the help menu //
	main_window->help_menu = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_widget_show (main_window->help_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->help_menu);
	
	main_window->help_menu_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->help_menu), main_window->help_menu_menu);
//	main_window->help_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->help_menu_menu));
	
	// create and add the contents menu item to the help menu 
	main_window->contents_menu_item = gtk_image_menu_item_new_with_label (_("Contents"));
	gtk_widget_show (main_window->contents_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->contents_menu_item);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (main_window->contents_menu_item),
          gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU)) ;
	
	// create and add the search menu item to the help menu //
	main_window->search_menu_item = gtk_menu_item_new_with_label (_("Search"));
	gtk_widget_show (main_window->search_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->search_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->search_menu_item), FALSE);
	
	// create and add the about menu item to the help menu // 
	main_window->about_menu_item = gtk_menu_item_new_with_label (_("About..."));
	gtk_widget_show (main_window->about_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->about_menu_item);
	gtk_tooltips_set_tip (main_window->tooltips, main_window->about_menu_item, _("About QCADesigner"), NULL);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// WINDOW WIDGETS //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

  main_window->vpaned1 = gtk_vpaned_new () ;
	gtk_widget_show (main_window->vpaned1) ;
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->vpaned1, TRUE, TRUE, 0) ;
	
	main_window->hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (main_window->hbox1);
	gtk_paned_pack1 (GTK_PANED (main_window->vpaned1), main_window->hbox1, TRUE, TRUE);
        
	// create and add the toolbar to the left hand side of the main window //
        main_window->toolbar = gtk_toolbar_new () ;
        gtk_widget_show (main_window->toolbar) ;
        gtk_toolbar_set_orientation (GTK_TOOLBAR (main_window->toolbar), GTK_ORIENTATION_VERTICAL) ;
        gtk_toolbar_set_tooltips (GTK_TOOLBAR (main_window->toolbar), TRUE) ;
        gtk_toolbar_set_style (GTK_TOOLBAR (main_window->toolbar), GTK_TOOLBAR_BOTH_HORIZ) ;
	gtk_box_pack_start (GTK_BOX (main_window->hbox1), main_window->toolbar, FALSE, FALSE, 0);
	
	// CELL LAYER MANIPULATION BUTTONS
        // create and add the default button to the toolbar //
        main_window->default_action_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            NULL,
            _("Select"),
            _("Select And Manipulate Design"),
            _("This button allows you to manipulate your design."),
            create_pixmap (main_window->main_window, "default.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_DEFAULT) ;
	
        // create and add the type 1 cell button to the toolbar //
        main_window->insert_type_1_cell_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->default_action_button,
            _("Type 1"),
            _("Add Type 1 Cell To Design"),
            _("Click here, then click on the design to add Type 1 cells to it."),
            create_pixmap (main_window->main_window, "q_cell_def.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_SINGLE_CELL) ;
	
	// create and add the type 2 cell button to the toolbar //
        main_window->insert_type_2_cell_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Type 2"),
            _("Add Type 2 Cell To Design"),
            _("Click here, then click on the design to add Type 2 cells to it."),
            create_pixmap (main_window->main_window, "q_cell_def.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_SINGLE_CELL) ;
	
	// create and add the properties button to the toolbar //
        main_window->cell_properties_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Function"),
            _("Modify Cell Function"),
            _("Modify the function of one of the cells (Input/Output/Fixed)."),
            create_pixmap (main_window->main_window, "properties.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), run_action_CELL_FUNCTION) ;
	
	// create and add the array button to the toolbar //
        main_window->insert_cell_array_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Array"),
            _("Add Cell Array"),
            _("Add horizontal or vertical arrays of Type 1 or Type 2 cells to your design."),
            create_pixmap (main_window->main_window, "q_cell_array.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_ARRAY) ;
	
	// create and add the copy button to the toolbar //
        main_window->copy_cell_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_BUTTON,
            NULL,
            _("Copy"),
            _("Copy Selection"),
            _("Makes a copy of your selection."),
            create_pixmap (main_window->main_window, "q_cell_move.xpm"),
            GTK_SIGNAL_FUNC (on_copy_cell_button_clicked), (gpointer)COPY_CELL) ;
	
	// create and add the move button to the toolbar //
//        main_window->move_cell_button = 
//          gtk_toolbar_append_element (
//            GTK_TOOLBAR (main_window->toolbar), 
//            GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
//            NULL,
//            _("Move"),
//            _("Move Selection"),
//            _("Click and drag your selection to its new location."),
//            create_pixmap (main_window->main_window, "q_cell_move.xpm"),
//            GTK_SIGNAL_FUNC (on_move_cell_button_clicked), NULL) ;

	// create and add the rotate button to the toolbar //
        main_window->rotate_cell_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Rotate"),
            _("Rotate Cell"),
            _("Click on a cell to rotate it (repeatedly)."),
            create_pixmap (main_window->main_window, "q_cell_rotate.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_ROTATE) ;

	// create and add the translate button to the toolbar //
        main_window->translate_cell_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_BUTTON,
            NULL,
            _("Translate"),
            _("Translate Selection"),
            _("Translate your selection horizontally and/or vertically."),
            create_pixmap (main_window->main_window, "q_cell_move.xpm"),
            GTK_SIGNAL_FUNC (on_translate_selection_button_clicked), NULL) ;

	// create and add the mirror button to the toolbar //
	main_window->mirror_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Mirror"),
            _("Mirror Selection (Hold down Ctrl to make a mirrored copy)"),
            _("Mirror you selection across a vertical line.  Hold down Ctrl to create a mirrored copy."),
            create_pixmap (main_window->main_window, "q_cell_mirror.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_MIRROR) ;
	
	// create and add the delete button to the toolbar //
	main_window->delete_cells_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Delete"),
            _("Delete Cells"),
            _("Click on this button to delete your selection.  Afterwards, click on individual cells to delete them."),
            create_pixmap (main_window->main_window, "eraser.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_DELETE) ;
/*
	// CLOCKING LAYER MANIPULATION BUTTONS
	// create and add the circular clocking zone button to the toolbar //
	main_window->oval_zone_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Oval"),
            _("Oval Clocking Zone"),
            _("Creates an oval or a circular clocking zone."),
            create_pixmap (main_window->main_window, "clocking_oval.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)CLOCKING_ZONE) ;

	// create and add the circular clocking zone button to the toolbar //
	main_window->polygon_zone_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Polygon"),
            _("Polygonal Clocking Zone"),
            _("Creates a polygonal clocking zone."),
            create_pixmap (main_window->main_window, "clocking_polygon.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)CLOCKING_ZONE) ;
*/
        // This will separate layer-specific commands from generic ones like zoom & pan
        gtk_toolbar_append_space (GTK_TOOLBAR (main_window->toolbar)) ;
		
        // create and add the pan button to the toolbar //
	main_window->pan_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_RADIOBUTTON,
            main_window->insert_type_1_cell_button,
            _("Pan"),
            _("Pan Design"),
            _("Bring various parts of the design into view by dragging visible parts of the design out of view."),
            create_pixmap (main_window->main_window, "q_cell_pan.xpm"),
            GTK_SIGNAL_FUNC (action_button_clicked), (gpointer)run_action_PAN) ;

	// create and add the zoom + button to the toolbar //
	main_window->zoom_plus_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_BUTTON,
            NULL,
            _("Zoom In"),
            _("Increase The Magnification"),
            _("Increase the magnification so you may concentrate on specific parts of the design."),
            create_pixmap (main_window->main_window, "zoom_in.xpm"),
            GTK_SIGNAL_FUNC (on_zoom_in_menu_item_activate), NULL) ;

	// create and add the zoom - button to the toolbar //
	main_window->zoom_minus_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_BUTTON,
            NULL,
            _("Zoom Out"),
            _("Decreate The Magnification"),
            _("Decrease the magnification to get an overall picture."),
            create_pixmap (main_window->main_window, "zoom_out.xpm"),
            GTK_SIGNAL_FUNC (on_zoom_out_menu_item_activate), NULL) ;

	// create and add the zoom extents button to the toolbar //
	main_window->zoom_extents_button = 
          gtk_toolbar_append_element (
            GTK_TOOLBAR (main_window->toolbar), 
            GTK_TOOLBAR_CHILD_BUTTON,
            NULL,
            _("Extents"),
            _("Fit Design To Window"),
            _("Calculates and sets the magnification such that your design fits inside the window."),
            create_pixmap (main_window->main_window, "zoom_extents.xpm"),
            GTK_SIGNAL_FUNC (on_zoom_extents_menu_item_activate), NULL) ;
	
	// create and add the table widget to the hbox //
	main_window->table1 = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (main_window->table1);
	gtk_box_pack_start (GTK_BOX (main_window->hbox1), main_window->table1, TRUE, TRUE, 0);
	
        // Add a frame around the drawing area, so it makes a nice border with the other widgets
	main_window->drawing_area_frame = gtk_frame_new (NULL);
	gtk_widget_show (main_window->drawing_area_frame);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->drawing_area_frame, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (main_window->drawing_area_frame), GTK_SHADOW_IN);
	
	// create and add the drawing area to the table //
	// this is the widget where all the action happens //
	main_window->drawing_area = gtk_drawing_area_new ();
	GTK_WIDGET_SET_FLAGS (main_window->drawing_area, GTK_CAN_FOCUS);
	gtk_widget_show (main_window->drawing_area);
	gtk_container_add (GTK_CONTAINER (main_window->drawing_area_frame), main_window->drawing_area) ;

        rcstyle = gtk_widget_get_modifier_style (main_window->drawing_area) ;
        rcstyle->color_flags[0] |= GTK_RC_BG ;
        rcstyle->color_flags[1] |= GTK_RC_BG ;
        rcstyle->color_flags[2] |= GTK_RC_BG ;
        rcstyle->color_flags[3] |= GTK_RC_BG ;
        rcstyle->bg[0].pixel = rcstyle->bg[0].red = rcstyle->bg[0].green = rcstyle->bg[0].blue = 0 ;
        gdk_colormap_alloc_color (gdk_colormap_get_system (), &(rcstyle->bg[0]), FALSE, TRUE) ;
        rcstyle->bg[1].pixel = rcstyle->bg[2].pixel = rcstyle->bg[3].pixel = rcstyle->bg[4].pixel = rcstyle->bg[0].pixel ;
        rcstyle->bg[1].red = rcstyle->bg[1].green = rcstyle->bg[1].blue = 
        rcstyle->bg[2].red = rcstyle->bg[2].green = rcstyle->bg[2].blue = 
        rcstyle->bg[3].red = rcstyle->bg[3].green = rcstyle->bg[3].blue = 
        rcstyle->bg[4].red = rcstyle->bg[4].green = rcstyle->bg[4].blue = 0 ;
        gtk_widget_modify_style (main_window->drawing_area, rcstyle) ;

	// create and add the horizontal ruler to the table //
	// purpose to provide a real time ruler for measuring the design //
	main_window->horizontal_ruler = gtk_hruler_new ();
	gtk_widget_show (main_window->horizontal_ruler);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->horizontal_ruler, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_ruler_set_metric (GTK_RULER (main_window->horizontal_ruler), GTK_PIXELS) ;
	gtk_ruler_set_range (GTK_RULER (main_window->horizontal_ruler), 0, 100, 0, 1);
	
	// create and add the vertical ruler to the table //
	// purpose to provide a real time ruler for measuring the design //
	main_window->vertical_ruler = gtk_vruler_new ();
	gtk_widget_show (main_window->vertical_ruler);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->vertical_ruler, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_ruler_set_metric (GTK_RULER (main_window->vertical_ruler), GTK_PIXELS) ;
	gtk_ruler_set_range (GTK_RULER (main_window->vertical_ruler), 0, 100, 0, 1);

	// create and add a scroll window in which to put the command history //
//	main_window->scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
//	gtk_widget_show (main_window->scrolledwindow1);
//	gtk_paned_pack2 (GTK_PANED (main_window->vpaned1), main_window->scrolledwindow1, FALSE, TRUE);
//	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (main_window->scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	
	// create and add the command history text box to the scrolled window //
	main_window->command_history = command_history_create () ;
	gtk_widget_show (main_window->command_history);
	gtk_paned_pack2 (GTK_PANED (main_window->vpaned1), main_window->command_history, TRUE, TRUE);
	
	// create and add the command entry to the main window //
	main_window->command_entry = gtk_entry_new ();
	gtk_widget_show (main_window->command_entry);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->command_entry, FALSE, FALSE, 0);
	
	// create and add tbe status bar to the main window //
	main_window->status_bar = gtk_statusbar_new ();
	gtk_widget_show (main_window->status_bar);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->status_bar, FALSE, FALSE, 0);


////////////////////////////////////////////////////////////////////////////////////////
// Connect the callback signals to each of the buttons and menu items in the menus  ////
////////////////////////////////////////////////////////////////////////////////////////


    g_signal_connect ((gpointer)(main_window->new_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_NEW);
    g_signal_connect ((gpointer)(main_window->open_menu_item), "activate",GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_OPEN);
    g_signal_connect ((gpointer)(main_window->save_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_SAVE);
    g_signal_connect ((gpointer)(main_window->save_as_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_SAVE_AS);
    g_signal_connect ((gpointer)(main_window->print_menu_item), "activate", GTK_SIGNAL_FUNC (on_print_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->preview_menu_item), "activate", GTK_SIGNAL_FUNC (on_preview_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->project_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_project_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->close_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_CLOSE);
    g_signal_connect_swapped ((gpointer)(main_window->quit_menu_item), "activate", GTK_SIGNAL_FUNC (on_quit_menu_item_activate), main_window->main_window);
    g_signal_connect ((gpointer)(main_window->undo_menu_item), "activate", GTK_SIGNAL_FUNC (on_undo_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->redo_menu_item), "activate", GTK_SIGNAL_FUNC (on_redo_meu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->copy_menu_item), "activate", GTK_SIGNAL_FUNC (on_copy_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->cut_menu_item), "activate", GTK_SIGNAL_FUNC (on_cut_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->paste_menu_item), "activate", GTK_SIGNAL_FUNC (on_paste_menu_item_activate), NULL);
	g_signal_connect ((gpointer)(main_window->delete_menu_item), "activate", GTK_SIGNAL_FUNC (on_delete_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->grid_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_grid_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->cell_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_cell_properties_menu_item_activate), NULL);
//    g_signal_connect ((gpointer)(main_window->window_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_window_properties_menu_item_activate), NULL);
//    g_signal_connect ((gpointer)(main_window->layer_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_layer_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->preferences_menu_item), "activate", GTK_SIGNAL_FUNC (on_preferences_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->show_tb_icons_menu_item), "activate", GTK_SIGNAL_FUNC (on_show_tb_icons_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->snap_to_grid_menu_item), "activate", GTK_SIGNAL_FUNC (on_snap_to_grid_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->show_grid_menu_item), "activate", GTK_SIGNAL_FUNC (on_show_grid_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->create_block_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_EXPORT);
    g_signal_connect ((gpointer)(main_window->import_block_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)FILEOP_IMPORT);
	g_signal_connect ((gpointer)(main_window->scale_menu_item), "activate", GTK_SIGNAL_FUNC (on_scale_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->rotate_selection_menu_item), "activate", GTK_SIGNAL_FUNC (rotate_selection_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->cell_function_menu_item), "activate", GTK_SIGNAL_FUNC (on_cell_function_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->clock_select_menu_item), "activate", GTK_SIGNAL_FUNC (on_clock_select_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->clock_increment_menu_item), "activate", GTK_SIGNAL_FUNC (on_clock_increment_menu_item_activate), NULL);
//    g_signal_connect ((gpointer)(main_window->measure_distance_menu_item), "activate", GTK_SIGNAL_FUNC (action_button_clicked), NULL);
    g_signal_connect ((gpointer)(main_window->measurement_preferences1), "activate", GTK_SIGNAL_FUNC (on_measurement_preferences1_activate), NULL);
    g_signal_connect ((gpointer)(main_window->draw_dimensions_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_dimensions_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->dimension_properties1), "activate", GTK_SIGNAL_FUNC (on_dimension_properties1_activate), NULL);
    g_signal_connect ((gpointer)(main_window->draw_text_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_text_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->text_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_text_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->draw_arrow_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_arrow_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->arrow_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_arrow_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->draw_line_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_line_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->line_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_line_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->draw_rectangle_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_rectangle_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->rectangle_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_rectangle_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->start_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_start_simulation_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->stop_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_stop_simulation_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->pause_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_pause_simulation_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->reset_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_reset_simulation_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->calculate_ground_state_menu_item), "activate", GTK_SIGNAL_FUNC (on_calculate_ground_state_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->animate_test_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_animate_test_simulation_menu_item_activate), NULL);	
    g_signal_connect ((gpointer)(main_window->save_output_to_file_menu_item), "activate", GTK_SIGNAL_FUNC (on_save_output_to_file_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->logging_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_logging_properties_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->simulation_type_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_simulation_type_setup_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->simulation_engine_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_simulation_engine_setup_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->random_fault_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_random_fault_setup_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->zoom_in_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_in_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->zoom_out_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_out_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->zoom_die_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_die_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->zoom_extents_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_extents_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->contents_menu_item), "activate", GTK_SIGNAL_FUNC (on_contents_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->search_menu_item), "activate", GTK_SIGNAL_FUNC (on_search_menu_item_activate), NULL);
    g_signal_connect ((gpointer)(main_window->about_menu_item), "activate", GTK_SIGNAL_FUNC (on_about_menu_item_activate), NULL);

    g_signal_connect ((gpointer)(main_window->command_entry), "changed", GTK_SIGNAL_FUNC (on_command_entry_changed), NULL);
    
    g_signal_connect ((gpointer)(main_window->main_window), "show", (GCallback)main_window_show, NULL) ;

    gtk_object_set_data (GTK_OBJECT (main_window->main_window), "tooltips", main_window->tooltips);

    gtk_window_add_accel_group (GTK_WINDOW (main_window->main_window), main_window->accel_group);

    // attach the necessary signals to the drawing area widget //
    g_signal_connect ((gpointer)(main_window->drawing_area), "scroll_event", (GtkSignalFunc) scroll_event, NULL);
    g_signal_connect ((gpointer)(main_window->drawing_area), "expose_event", (GtkSignalFunc) expose_event, NULL);
    g_signal_connect ((gpointer)(main_window->drawing_area), "configure_event", (GtkSignalFunc) configure_event, NULL);
    g_signal_connect ((gpointer)(main_window->drawing_area), "motion_notify_event", (GtkSignalFunc) synchronize_rulers, NULL);
//    g_signal_connect ((gpointer)(main_window->drawing_area), "button_press_event", (GtkSignalFunc) button_press_event, NULL);
//    g_signal_connect ((gpointer)(main_window->drawing_area), "button_release_event", (GtkSignalFunc) button_release_event, NULL);

    // attach the callback signal for key press //
//    g_signal_connect ((gpointer)(main_window->main_window), "key_press_event", (GtkSignalFunc) key_press_event, NULL);

    // attach the callback signal for key press //
    g_signal_connect ((gpointer)(main_window->main_window), "configure_event", (GtkSignalFunc) main_window_configure_event, NULL);

    // -- Connect the shutdown callback signal to the main window -- //
    g_signal_connect_swapped ((gpointer)(main_window->main_window), "delete_event", GTK_SIGNAL_FUNC (on_quit_menu_item_activate), main_window->main_window);

// activate the necessary events for the drawing area such as expose, mouse motion, mouse click, etc //
    gtk_widget_set_events (GTK_WIDGET (main_window->drawing_area), GDK_EXPOSURE_MASK
		       | GDK_LEAVE_NOTIFY_MASK
		       | GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK
		       | GDK_KEY_PRESS_MASK
		       | GDK_POINTER_MOTION_MASK
		       | GDK_POINTER_MOTION_HINT_MASK);

	// Set the global pointers to the main window, drawing area, and the history text box //
}
