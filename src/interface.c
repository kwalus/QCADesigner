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
#include "globals.h"
#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "custom_widgets.h"

// -- creates the main application window and returns a pointer to it -- //

GtkWidget *create_pixmap_button (GtkWidget *pixmap, gchar *text, gboolean bToggle) ;

void create_main_window (main_W *main_window){

    // All the objects that appear in the main window //

    main_window->tooltips = gtk_tooltips_new ();

    main_window->accel_group = gtk_accel_group_new ();

    // create the main window //
    main_window->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data (GTK_OBJECT (main_window->main_window), "main_window", main_window->main_window);
    gtk_widget_set_usize (main_window->main_window, 1024, 768);
    gtk_window_set_title (GTK_WINDOW (main_window->main_window), _("QCADesigner"));

    // create the vertical box and add it to the main window //
    main_window->vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (main_window->vbox1);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "vbox1", main_window->vbox1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->vbox1);
    gtk_container_add (GTK_CONTAINER (main_window->main_window), main_window->vbox1);

    // create and add the main menubar to the main window //
    main_window->main_menubar = gtk_menu_bar_new ();
    gtk_widget_ref (main_window->main_menubar);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "main_menubar", main_window->main_menubar, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->main_menubar);
    gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->main_menubar, FALSE, FALSE, 0);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// FILE MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

    // create and add the file menu to the menubar //
    main_window->file_menu = gtk_menu_item_new_with_label (_("File"));
    gtk_widget_ref (main_window->file_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "file_menu", main_window->file_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->file_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->file_menu);

    main_window->file_menu_menu = gtk_menu_new ();
    gtk_widget_ref (main_window->file_menu_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "file_menu_menu", main_window->file_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->file_menu), main_window->file_menu_menu);
    main_window->file_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->file_menu_menu));

	// create and add the New project file menu item to the menubar //
    main_window->new_menu_item = gtk_menu_item_new_with_label (_("New"));
    gtk_widget_ref (main_window->new_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "new_menu_item", main_window->new_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->new_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->new_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->new_menu_item, _("Create a new project file"), NULL);

	// create and add the open project menu item to the menubar //
    main_window->open_menu_item = gtk_menu_item_new_with_label (_("Open..."));
    gtk_widget_ref (main_window->open_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "open_menu_item", main_window->open_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->open_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->open_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->open_menu_item, _("Open a project file"), NULL);

	// create and add the save menu item to the menubar //
    main_window->save_menu_item = gtk_menu_item_new_with_label (_("Save"));
    gtk_widget_ref (main_window->save_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "save_menu_item", main_window->save_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->save_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->save_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->save_menu_item, _("Save current project"), NULL);

	// create and add the save as meny item to the menubar //
    main_window->save_as_menu_item = gtk_menu_item_new_with_label (_("Save As..."));
    gtk_widget_ref (main_window->save_as_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "save_as_menu_item", main_window->save_as_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->save_as_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->save_as_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->save_as_menu_item, _("Save project file as ..."), NULL);

	// create and add a seperator to the file menu //
    main_window->separator2 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator2);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator2", main_window->separator2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator2);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->separator2);
    gtk_widget_set_sensitive (main_window->separator2, FALSE);

	// create and add the print menu item to the menu bar //
    main_window->print_menu_item = gtk_menu_item_new_with_label (_("Print..."));
    gtk_widget_ref (main_window->print_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "print_menu_item", main_window->print_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->print_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->print_menu_item);
    
//    gtk_widget_set_sensitive (main_window->print_menu_item, FALSE) ;

	// create and add the preview item to the menu bar //
    main_window->preview_menu_item = gtk_menu_item_new_with_label (_("Preview"));
    gtk_widget_ref (main_window->preview_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "preview_menu_item", main_window->preview_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->preview_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->preview_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->preview_menu_item, _("Preview the print layout"), NULL);

	// create and add a seperator to the file menu //
    main_window->separator3 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator3);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator3", main_window->separator3, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator3);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->separator3);
    gtk_widget_set_sensitive (main_window->separator3, FALSE);

	// create and add the project properties menu item to the menubar //
    main_window->project_properties_menu_item = gtk_menu_item_new_with_label (_("Project Properties..."));
    gtk_widget_ref (main_window->project_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "project_properties_menu_item", main_window->project_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->project_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->project_properties_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->project_properties_menu_item, _("Show project properties"), NULL);

	// create and add the close menu item to the file menu //
    main_window->close_menu_item = gtk_menu_item_new_with_label (_("Close"));
    gtk_widget_ref (main_window->close_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "close_menu_item", main_window->close_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->close_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->close_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->close_menu_item, _("Close the project file"), NULL);

	// create and add a seperator to the file menu //
    main_window->mnuSepPreRecentFiles = gtk_menu_item_new ();
    gtk_widget_ref (main_window->mnuSepPreRecentFiles);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "mnuSepPreRecentFiles", main_window->mnuSepPreRecentFiles, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->mnuSepPreRecentFiles);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->mnuSepPreRecentFiles);
    gtk_widget_set_sensitive (main_window->mnuSepPreRecentFiles, FALSE);
    
    main_window->recent_files_menu_item = gtk_menu_item_new_with_label (_("Recent Files")) ;
    gtk_widget_ref (main_window->recent_files_menu_item) ;
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "recent_files_menu_item", main_window->recent_files_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->recent_files_menu_item) ;
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->recent_files_menu_item) ;

    main_window->recent_files_menu = gtk_menu_new () ;
    gtk_widget_ref (main_window->recent_files_menu) ;
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "recent_files_menu", main_window->recent_files_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->recent_files_menu) ;
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->recent_files_menu_item), main_window->recent_files_menu) ;

	// create and add a seperator to the file menu //
    main_window->mnuSepPostRecentFiles = gtk_menu_item_new ();
    gtk_widget_ref (main_window->mnuSepPostRecentFiles);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "mnuSepPostRecentFiles", main_window->mnuSepPostRecentFiles, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->mnuSepPostRecentFiles);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->mnuSepPostRecentFiles);
    gtk_widget_set_sensitive (main_window->mnuSepPostRecentFiles, FALSE);

	// create and add the quit menu item to the file menu //
    main_window->quit_menu_item = gtk_menu_item_new_with_label (_("Quit"));
    gtk_widget_ref (main_window->quit_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "quit_menu_item", main_window->quit_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->quit_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->file_menu_menu), main_window->quit_menu_item);
    gtk_tooltips_set_tip (main_window->tooltips, main_window->quit_menu_item, _("quit QCADesigner"), NULL);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// EDIT MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the edit menu to the menubar //
    main_window->edit_menu = gtk_menu_item_new_with_label (_("Edit"));
    gtk_widget_ref (main_window->edit_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "edit_menu", main_window->edit_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->edit_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->edit_menu);

    main_window->edit_menu_menu = gtk_menu_new ();
    gtk_widget_ref (main_window->edit_menu_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "edit_menu_menu", main_window->edit_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->edit_menu), main_window->edit_menu_menu);
    main_window->edit_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->edit_menu_menu));

	// create and add the undo menu item to the menu bar //
    main_window->undo_menu_item = gtk_menu_item_new_with_label (_("Undo"));
    gtk_widget_ref (main_window->undo_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "undo_menu_item", main_window->undo_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->undo_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->undo_menu_item);
    gtk_widget_add_accelerator (main_window->undo_menu_item, "activate", main_window->accel_group, GDK_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->undo_menu_item), FALSE);

	//create and add the redu menu item to the menu bar //
    main_window->redo_menu_item = gtk_menu_item_new_with_label (_("Redo"));
    gtk_widget_ref (main_window->redo_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "redo_meu_item", main_window->redo_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->redo_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->redo_menu_item);
    gtk_widget_add_accelerator (main_window->redo_menu_item, "activate", main_window->accel_group, GDK_z, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->redo_menu_item), FALSE);

	// create and add a seperator to the edit menu //
    main_window->separator7 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator7);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator7", main_window->separator7, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator7);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->separator7);
    gtk_widget_set_sensitive (main_window->separator7, FALSE);

	// create and add the copy menu item to the edit menu //
    main_window->copy_menu_item = gtk_menu_item_new_with_label (_("Copy"));
    gtk_widget_ref (main_window->copy_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "copy_menu_item", main_window->copy_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->copy_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->copy_menu_item);
    gtk_widget_add_accelerator (main_window->copy_menu_item, "activate", main_window->accel_group, GDK_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->copy_menu_item), FALSE);

	// create and add the cut menu item to the edit menu //
    main_window->cut_menu_item = gtk_menu_item_new_with_label (_("Cut"));
    gtk_widget_ref (main_window->cut_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "cut_menu_item", main_window->cut_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->cut_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->cut_menu_item);
    gtk_widget_add_accelerator (main_window->cut_menu_item, "activate", main_window->accel_group, GDK_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->cut_menu_item), FALSE);

	// create and add the paste item to the edit menu //
    main_window->paste_menu_item = gtk_menu_item_new_with_label (_("Paste"));
    gtk_widget_ref (main_window->paste_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "paste_menu_item", main_window->paste_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->paste_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->paste_menu_item);
    gtk_widget_add_accelerator (main_window->paste_menu_item, "activate", main_window->accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->paste_menu_item), FALSE);

	// create and add a seperator to the edit menu //
    main_window->separator6 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator6);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator6", main_window->separator6, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator6);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->separator6);
    gtk_widget_set_sensitive (main_window->separator6, FALSE);

	// create and add the grid properties menu item to the edit menu //
    main_window->grid_properties_menu_item = gtk_menu_item_new_with_label (_("Grid Properties..."));
    gtk_widget_ref (main_window->grid_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "grid_properties_menu_item", main_window->grid_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->grid_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->grid_properties_menu_item);

	// create and add the snap properties menu item to the edit menu //
    main_window->snap_properties_menu_item = gtk_menu_item_new_with_label (_("Snap Properties..."));
    gtk_widget_ref (main_window->snap_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "snap_properties_menu_item", main_window->snap_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->snap_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->snap_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->snap_properties_menu_item), FALSE);

	// create and add the cell properties menu item to the edit menu //
    main_window->cell_properties_menu_item = gtk_menu_item_new_with_label (_("Cell Properties..."));
    gtk_widget_ref (main_window->cell_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "cell_properties_menu_item", main_window->cell_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->cell_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->cell_properties_menu_item);

	// create and add the window properties menu item to the edit menu //
    main_window->window_properties_menu_item = gtk_menu_item_new_with_label (_("Window Properties"));
    gtk_widget_ref (main_window->window_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "window_properties_menu_item", main_window->window_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->window_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->window_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->window_properties_menu_item), FALSE);

	// create and add the layer properties menu item to the edit menu //
    main_window->layer_properties_menu_item = gtk_menu_item_new_with_label (_("Layer Properties"));
    gtk_widget_ref (main_window->layer_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "layer_properties_menu_item", main_window->layer_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->layer_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->layer_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->layer_properties_menu_item), FALSE);

	// create and add the preferences menu item to the edit menu //
    main_window->preferences_menu_item = gtk_menu_item_new_with_label (_("Preferences"));
    gtk_widget_ref (main_window->preferences_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "preferences_menu_item", main_window->preferences_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->preferences_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->edit_menu_menu), main_window->preferences_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->preferences_menu_item), FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// VIEW MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// create and add the view menu //
	main_window->view_menu = gtk_menu_item_new_with_label (_("View"));
	gtk_widget_ref (main_window->view_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "view_menu", main_window->view_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->view_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->view_menu);
	
	main_window->view_menu_menu = gtk_menu_new ();
	gtk_widget_ref (main_window->view_menu_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "view_menu_menu", main_window->view_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->view_menu), main_window->view_menu_menu);
	main_window->view_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->view_menu_menu));
	
	// create and add the zoom in menu item to the view menu //
	main_window->zoom_in_menu_item = gtk_menu_item_new_with_label (_("Zoom In"));
	gtk_widget_ref (main_window->zoom_in_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "zoom_in_menu_item", main_window->zoom_in_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_in_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_in_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_in_menu_item, "activate", main_window->accel_group, GDK_w, 0, GTK_ACCEL_VISIBLE);
	
	// create and add the zoom out menu item to the view menu //
	main_window->zoom_out_menu_item = gtk_menu_item_new_with_label (_("Zoom Out"));
	gtk_widget_ref (main_window->zoom_out_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "zoom_out_menu_item", main_window->zoom_out_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_out_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_out_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_out_menu_item, "activate", main_window->accel_group, GDK_q, 0, GTK_ACCEL_VISIBLE);
/*	
	// create and add the zoom window menu item to the view menu //
	zoom_window_menu_item = gtk_menu_item_new_with_label (_("Zoom Window"));
	gtk_widget_ref (zoom_window_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window), "zoom_window_menu_item", zoom_window_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (zoom_window_menu_item);
	gtk_container_add (GTK_CONTAINER (view_menu_menu), zoom_window_menu_item);
	
	// *********** remove when finished **************** //
	gtk_widget_set_sensitive (GTK_WIDGET (zoom_window_menu_item), FALSE);
*/
	// create and add the zoom die menu item to the view menu //
	main_window->zoom_die_menu_item = gtk_menu_item_new_with_label (_("Zoom Die"));
	gtk_widget_ref (main_window->zoom_die_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "zoom_die_menu_item", main_window->zoom_die_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_die_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_die_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_die_menu_item, "activate", main_window->accel_group, GDK_d, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// create and add the zoom extents menu item to the view menu //
	main_window->zoom_extents_menu_item = gtk_menu_item_new_with_label (_("Zoom Extents"));
	gtk_widget_ref (main_window->zoom_extents_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "zoom_extents_menu_item", main_window->zoom_extents_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_extents_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->zoom_extents_menu_item);
	gtk_widget_add_accelerator (main_window->zoom_extents_menu_item, "activate", main_window->accel_group, GDK_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
/*	
	// create and add the pan menu item to the view menu //
	pan_menu_item = gtk_menu_item_new_with_label (_("Pan"));
	gtk_widget_ref (pan_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window), "pan_menu_item", pan_menu_item,
	(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (pan_menu_item);
	gtk_container_add (GTK_CONTAINER (view_menu_menu), pan_menu_item);
*/	
    // create and add a seperator to the edit menu //
    main_window->separator4 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator4);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator4", main_window->separator4, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator4);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->separator4);
    gtk_widget_set_sensitive (main_window->separator4, FALSE);

    // create and add the snap to grid menu item to the edit menu //
    main_window->snap_to_grid_menu_item = gtk_check_menu_item_new_with_label (_("Snap To Grid"));
    gtk_widget_ref (main_window->snap_to_grid_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "snap_to_grid_menu_item", main_window->snap_to_grid_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->snap_to_grid_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->snap_to_grid_menu_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window->snap_to_grid_menu_item), TRUE);
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->snap_to_grid_menu_item), TRUE);

    // create and add the show grid menu item to the edit menu //
    main_window->show_grid_menu_item = gtk_check_menu_item_new_with_label (_("Show Grid"));
    gtk_widget_ref (main_window->show_grid_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "show_grid_menu_item", main_window->show_grid_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->show_grid_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->view_menu_menu), main_window->show_grid_menu_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(main_window->show_grid_menu_item), TRUE);
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(main_window->show_grid_menu_item), TRUE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// TOOLS MENU ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the tools menu //
    main_window->tools_menu = gtk_menu_item_new_with_label (_("Tools"));
    gtk_widget_ref (main_window->tools_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "tools_menu", main_window->tools_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->tools_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->tools_menu);

    main_window->tools_menu_menu = gtk_menu_new ();
    gtk_widget_ref (main_window->tools_menu_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "tools_menu_menu", main_window->tools_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->tools_menu), main_window->tools_menu_menu);
    main_window->tools_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->tools_menu_menu));

	// create and add the create block menu item to the tools menu //
    main_window->create_block_menu_item = gtk_menu_item_new_with_label (_("Create Block..."));
    gtk_widget_ref (main_window->create_block_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "create_block_menu_item", main_window->create_block_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->create_block_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->create_block_menu_item);

	// create and add the import block menu item to the tools menu //
    main_window->import_block_menu_item = gtk_menu_item_new_with_label (_("Import Block..."));
    gtk_widget_ref (main_window->import_block_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "import_block_menu_item", main_window->import_block_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->import_block_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->import_block_menu_item);

	// create and add a seperator to the tools menu //
    main_window->separator8 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator8);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator8", main_window->separator8, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator8);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator8);
    gtk_widget_set_sensitive (main_window->separator8, FALSE);

	// create and add the create input menu item to the tools menu //
    main_window->create_input_menu_item = gtk_menu_item_new_with_label (_("Create Input..."));
    gtk_widget_ref (main_window->create_input_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "create_input_menu_item", main_window->create_input_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->create_input_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->create_input_menu_item);

	// create and add the input properties menu item to the tools menu //
    main_window->input_properties_menu_item = gtk_menu_item_new_with_label (_("Input Properties..."));
    gtk_widget_ref (main_window->input_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "input_properties_menu_item", main_window->input_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->input_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->input_properties_menu_item);

	// create and add a seperator to the tools menu //
    main_window->separator9 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator9);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator9", main_window->separator9, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator9);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator9);
    gtk_widget_set_sensitive (main_window->separator9, FALSE);

	// create and add the connect output menu item to the tools menu //
    main_window->connect_output_menu_item = gtk_menu_item_new_with_label (_("Connect Output..."));
    gtk_widget_ref (main_window->connect_output_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "connect_output_menu_item", main_window->connect_output_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->connect_output_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->connect_output_menu_item);

	// create and add the clock select menu item to the tools menu //
    main_window->clock_select_menu_item = gtk_menu_item_new_with_label (_("Select Clock..."));
    gtk_widget_ref (main_window->clock_select_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "clock_select_menu_item", main_window->clock_select_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->clock_select_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->clock_select_menu_item);

	// create and add the clock increment menu item to the tools menu //
    main_window->clock_increment_menu_item = gtk_menu_item_new_with_label (_("Increment Cell Clocks"));
    gtk_widget_ref (main_window->clock_increment_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "clock_increment_menu_item", main_window->clock_increment_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->clock_increment_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->clock_increment_menu_item);

	// create and add the fixed polarization menu item to the tools menu //
    main_window->fixed_polarization = gtk_menu_item_new_with_label (_("Fix Cell Polarization..."));
    gtk_widget_ref (main_window->fixed_polarization);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "fixed_polarization", main_window->fixed_polarization, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->fixed_polarization);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->fixed_polarization);

	// create and add a seperator to the tools menu //
    main_window->separator11 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator11);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator11", main_window->separator11, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator11);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->separator11);
    gtk_widget_set_sensitive (main_window->separator11, FALSE);

	// create and add the measure distance menu item to the tools menu //
    main_window->measure_distance_menu_item = gtk_menu_item_new_with_label (_("Measure Distance"));
    gtk_widget_ref (main_window->measure_distance_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "measure_distance_menu_item", main_window->measure_distance_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->measure_distance_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->measure_distance_menu_item);

	// create and add the mesurement preferences menu item to the tools menu //
    main_window->measurement_preferences1 = gtk_menu_item_new_with_label (_("Measurement Preferences"));
    gtk_widget_ref (main_window->measurement_preferences1);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "measurement_preferences1", main_window->measurement_preferences1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->measurement_preferences1);
    gtk_container_add (GTK_CONTAINER (main_window->tools_menu_menu), main_window->measurement_preferences1);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->measurement_preferences1), FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// DRAW MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the draw menu //
    main_window->draw_menu = gtk_menu_item_new_with_label (_("Draw"));
    gtk_widget_ref (main_window->draw_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_menu", main_window->draw_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_menu);
    gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->draw_menu);

    main_window->draw_menu_menu = gtk_menu_new ();
    gtk_widget_ref (main_window->draw_menu_menu);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_menu_menu", main_window->draw_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->draw_menu), main_window->draw_menu_menu);
    main_window->draw_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->draw_menu_menu));

	// create and add the draw dimensions menu item to the draw menu //
    main_window->draw_dimensions_menu_item = gtk_menu_item_new_with_label (_("Draw Dimensions"));
    gtk_widget_ref (main_window->draw_dimensions_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_dimensions_menu_item", main_window->draw_dimensions_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_dimensions_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_dimensions_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_dimensions_menu_item), FALSE);

	// create and add the dimension properties menu item to the draw menu //
    main_window->dimension_properties1 = gtk_menu_item_new_with_label (_("Dimension Properties"));
    gtk_widget_ref (main_window->dimension_properties1);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "dimension_properties1", main_window->dimension_properties1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->dimension_properties1);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->dimension_properties1);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->dimension_properties1), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator16 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator16);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator16", main_window->separator16, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator16);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator16);
    gtk_widget_set_sensitive (main_window->separator16, FALSE);

	// create and add the draw text menu item to the draw menu //
    main_window->draw_text_menu_item = gtk_menu_item_new_with_label (_("Draw Text"));
    gtk_widget_ref (main_window->draw_text_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_text_menu_item", main_window->draw_text_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_text_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_text_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_text_menu_item), FALSE);

	// create and add the text properties menu item to the draw menu //
    main_window->text_properties_menu_item = gtk_menu_item_new_with_label (_("Text Properties"));
    gtk_widget_ref (main_window->text_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "text_properties_menu_item", main_window->text_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->text_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->text_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->text_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator10 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator10);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator10", main_window->separator10, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator10);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator10);
    gtk_widget_set_sensitive (main_window->separator10, FALSE);

	// create and add the draw arrow menu item to the draw menu //
    main_window->draw_arrow_menu_item = gtk_menu_item_new_with_label (_("Draw Arrow"));
    gtk_widget_ref (main_window->draw_arrow_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_arrow_menu_item", main_window->draw_arrow_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_arrow_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_arrow_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_arrow_menu_item), FALSE);

	// create and add the arrow properties menu item to the draw menu //
    main_window->arrow_properties_menu_item = gtk_menu_item_new_with_label (_("Arrow Properties"));
    gtk_widget_ref (main_window->arrow_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "arrow_properties_menu_item", main_window->arrow_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->arrow_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->arrow_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->arrow_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator14 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator14);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator14", main_window->separator14, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator14);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator14);
    gtk_widget_set_sensitive (main_window->separator14, FALSE);

	// create and add the draw line menu item to the draw menu //
    main_window->draw_line_menu_item = gtk_menu_item_new_with_label (_("Draw Line"));
    gtk_widget_ref (main_window->draw_line_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_line_menu_item", main_window->draw_line_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_line_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_line_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_line_menu_item), FALSE);

	// create and add the line properties menu item to the draw menu //
    main_window->line_properties_menu_item = gtk_menu_item_new_with_label (_("Line Properties"));
    gtk_widget_ref (main_window->line_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "line_properties_menu_item", main_window->line_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->line_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->line_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->line_properties_menu_item), FALSE);

	// create and add a seperator to the draw menu //
    main_window->separator15 = gtk_menu_item_new ();
    gtk_widget_ref (main_window->separator15);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator15", main_window->separator15, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->separator15);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->separator15);
    gtk_widget_set_sensitive (main_window->separator15, FALSE);

	// create and add the draw rectangle menu item to the draw menu //
    main_window->draw_rectangle_menu_item = gtk_menu_item_new_with_label (_("Draw Rectangle"));
    gtk_widget_ref (main_window->draw_rectangle_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "draw_rectangle_menu_item", main_window->draw_rectangle_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->draw_rectangle_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->draw_rectangle_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->draw_rectangle_menu_item), FALSE);

	// create and add the rectangle properties menu item to the draw menu //
    main_window->rectangle_properties_menu_item = gtk_menu_item_new_with_label (_("Rectangle Properties"));
    gtk_widget_ref (main_window->rectangle_properties_menu_item);
    gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "rectangle_properties_menu_item", main_window->rectangle_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (main_window->rectangle_properties_menu_item);
    gtk_container_add (GTK_CONTAINER (main_window->draw_menu_menu), main_window->rectangle_properties_menu_item);

    // *********** remove when finished ****************//
    gtk_widget_set_sensitive (GTK_WIDGET (main_window->rectangle_properties_menu_item),FALSE);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// SIMULATION MENU //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// create and add the simulation menu //
	main_window->simulation_menu = gtk_menu_item_new_with_label (_("Simulation"));
	gtk_widget_ref (main_window->simulation_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "simulation_menu", main_window->simulation_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->simulation_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->simulation_menu);
	
	main_window->simulation_menu_menu = gtk_menu_new ();
	gtk_widget_ref (main_window->simulation_menu_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "simulation_menu_menu", main_window->simulation_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->simulation_menu), main_window->simulation_menu_menu);
	main_window->simulation_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->simulation_menu_menu));
	
	// create and add the start simulation menu item to the simulation menu //
	main_window->start_simulation_menu_item = gtk_menu_item_new_with_label (_("Start Simulation"));
	gtk_widget_ref (main_window->start_simulation_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "start_simulation_menu_item", main_window->start_simulation_menu_item,(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->start_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->start_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->start_simulation_menu_item, "activate", main_window->accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	//create and add the stop simulation menu item to the simulation menu //
	main_window->stop_simulation_menu_item = gtk_menu_item_new_with_label (_("Stop Simulation"));
	gtk_widget_ref (main_window->stop_simulation_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "stop_simulation_menu_item", main_window->stop_simulation_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->stop_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->stop_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->stop_simulation_menu_item, "activate", main_window->accel_group, GDK_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->stop_simulation_menu_item), FALSE);
	
	// create and add the pause simulation menu item to the simulatio menu //
	main_window->pause_simulation_menu_item = gtk_menu_item_new_with_label (_("Pause Simulation"));
	gtk_widget_ref (main_window->pause_simulation_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "pause_simulation_menu_item", main_window->pause_simulation_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->pause_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->pause_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->pause_simulation_menu_item, "activate", main_window->accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->pause_simulation_menu_item), FALSE);
	
	// create and add the reset simulation menu item to the simulation menu //
	main_window->reset_simulation_menu_item = gtk_menu_item_new_with_label (_("Reset Simulation"));
	gtk_widget_ref (main_window->reset_simulation_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "reset_simulation_menu_item", main_window->reset_simulation_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->reset_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->reset_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->reset_simulation_menu_item, "activate", main_window->accel_group, GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->reset_simulation_menu_item), FALSE);

	// create and add the calculate ground state menu item to the simulation menu //
	main_window->calculate_ground_state_menu_item = gtk_menu_item_new_with_label (_("Calculate Ground State"));
	gtk_widget_ref (main_window->calculate_ground_state_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "calculate_ground_state_menu_item", main_window->calculate_ground_state_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->calculate_ground_state_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->calculate_ground_state_menu_item);
	gtk_widget_add_accelerator (main_window->calculate_ground_state_menu_item, "activate", main_window->accel_group, GDK_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	// create and add the animate_test_simulation menu item to the simulation menu //
	main_window->animate_test_simulation_menu_item = gtk_menu_item_new_with_label (_("Animate Test Simulation"));
	gtk_widget_ref (main_window->animate_test_simulation_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "calculate_ground_state_menu_item", main_window->animate_test_simulation_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->animate_test_simulation_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->animate_test_simulation_menu_item);
	gtk_widget_add_accelerator (main_window->animate_test_simulation_menu_item, "activate", main_window->accel_group, GDK_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// create and add a seperator to the simulation menu //
	main_window->separator12 = gtk_menu_item_new ();
	gtk_widget_ref (main_window->separator12);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator12", main_window->separator12, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->separator12);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->separator12);
	gtk_widget_set_sensitive (main_window->separator12, FALSE);
	
	// create and add save output to file menu item to the simulation menu //
	main_window->save_output_to_file_menu_item = gtk_menu_item_new_with_label (_("Save Output To File"));
	gtk_widget_ref (main_window->save_output_to_file_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "save_output_to_file_menu_item", main_window->save_output_to_file_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->save_output_to_file_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->save_output_to_file_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->save_output_to_file_menu_item), FALSE);
	
	// create and add a seperator to the simulation menu //
	main_window->separator13 = gtk_menu_item_new ();
	gtk_widget_ref (main_window->separator13);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "separator13", main_window->separator13, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->separator13);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->separator13);
	gtk_widget_set_sensitive (main_window->separator13, FALSE);
	
	// create and add the logging properties menu item to the simulation menu //
	main_window->logging_properties_menu_item = gtk_menu_item_new_with_label (_("Logging Properties"));
	gtk_widget_ref (main_window->logging_properties_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "logging_properties_menu_item", main_window->logging_properties_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->logging_properties_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->logging_properties_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->logging_properties_menu_item), FALSE);
	
	// create and add the simulation type setup menu item to the simulation menu //
	main_window->simulation_type_setup_menu_item = gtk_menu_item_new_with_label (_("Simulation Type Setup..."));
	gtk_widget_ref (main_window->simulation_type_setup_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "simulation_type_setup_menu_item", main_window->simulation_type_setup_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->simulation_type_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->simulation_type_setup_menu_item);
	
	// create and add the simulation engine setup menu item to the simulation menu //
	main_window->simulation_engine_setup_menu_item = gtk_menu_item_new_with_label (_("Simulation Engine Setup..."));
	gtk_widget_ref (main_window->simulation_engine_setup_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "simulation_engine_setup_menu_item", main_window->simulation_engine_setup_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->simulation_engine_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->simulation_engine_setup_menu_item);
	
	// create and add the random fault setup menu item to the simulation menu //
	main_window->random_fault_setup_menu_item = gtk_menu_item_new_with_label (_("Random Fault Setup..."));
	gtk_widget_ref (main_window->random_fault_setup_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "random_fault_setup_menu_item", main_window->random_fault_setup_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->random_fault_setup_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->simulation_menu_menu), main_window->random_fault_setup_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->random_fault_setup_menu_item), FALSE);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// HELP MENU ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// create and add the help menu //
	main_window->help_menu = gtk_menu_item_new_with_label (_("Help"));
	gtk_widget_ref (main_window->help_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "help_menu", main_window->help_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->help_menu);
	gtk_container_add (GTK_CONTAINER (main_window->main_menubar), main_window->help_menu);
	
	main_window->help_menu_menu = gtk_menu_new ();
	gtk_widget_ref (main_window->help_menu_menu);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "help_menu_menu", main_window->help_menu_menu, (GtkDestroyNotify) gtk_widget_unref);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (main_window->help_menu), main_window->help_menu_menu);
	main_window->help_menu_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (main_window->help_menu_menu));
	
	// create and add the contents menu item to the help menu 
	main_window->contents_menu_item = gtk_menu_item_new_with_label (_("Contents"));
	gtk_widget_ref (main_window->contents_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "contents_menu_item", main_window->contents_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->contents_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->contents_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->contents_menu_item), FALSE);
	
	// create and add the search menu item to the help menu //
	main_window->search_menu_item = gtk_menu_item_new_with_label (_("Search"));
	gtk_widget_ref (main_window->search_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "search_menu_item", main_window->search_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->search_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->search_menu_item);
	
	// *********** remove when finished ****************//
	gtk_widget_set_sensitive (GTK_WIDGET (main_window->search_menu_item), FALSE);
	
	// create and add the about menu item to the help menu // 
	main_window->about_menu_item = gtk_menu_item_new_with_label (_("About..."));
	gtk_widget_ref (main_window->about_menu_item);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "about_menu_item", main_window->about_menu_item, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->about_menu_item);
	gtk_container_add (GTK_CONTAINER (main_window->help_menu_menu), main_window->about_menu_item);
	gtk_tooltips_set_tip (main_window->tooltips, main_window->about_menu_item, _("About QCADesigner"), NULL);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// WINDOW WIDGETS //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	main_window->hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (main_window->hbox1);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "hbox1", main_window->hbox1, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->hbox1);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->hbox1, TRUE, TRUE, 0);
	
	// create and add the toolbar to the left hand side of the main window //
	main_window->toolbar2 = gtk_vbutton_box_new ();
	gtk_widget_ref (main_window->toolbar2);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "toolbar2", main_window->toolbar2, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->toolbar2);
	gtk_box_pack_start (GTK_BOX (main_window->hbox1), main_window->toolbar2, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (main_window->toolbar2), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (main_window->toolbar2), 0);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (main_window->toolbar2), 0, 0);
	
	// create and add the type 1 cell button to the toolbar //
	main_window->insert_type_1_cell_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_def.xpm"), _("Type 1"), TRUE)) ;
	gtk_widget_ref (main_window->insert_type_1_cell_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "insert_type_1_cell_button", main_window->insert_type_1_cell_button,(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->insert_type_1_cell_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->insert_type_1_cell_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->insert_type_1_cell_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the type 2 cell button to the toolbar //
	main_window->insert_type_2_cell_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_def.xpm"), _("Type 2"), TRUE)) ;
	gtk_widget_ref (main_window->insert_type_2_cell_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "insert_type_2_cell_button", main_window->insert_type_2_cell_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->insert_type_2_cell_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->insert_type_2_cell_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->insert_type_2_cell_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the properties button to the toolbar //
	main_window->cell_properties_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "properties.xpm"), _("Properties"), FALSE)) ;
	gtk_widget_ref (main_window->cell_properties_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "cell_properties_button", main_window->cell_properties_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->cell_properties_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->cell_properties_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->cell_properties_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the array button to the toolbar //
	main_window->insert_cell_array_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_array.xpm"), _("Array"), TRUE)) ;
	gtk_widget_ref (main_window->insert_cell_array_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "insert_cell_array_button", main_window->insert_cell_array_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->insert_cell_array_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->insert_cell_array_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->insert_cell_array_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the copy button to the toolbar //
	main_window->copy_cell_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_move.xpm"), _("Copy"), FALSE)) ;
	gtk_widget_ref (main_window->copy_cell_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "copy_cell_button", main_window->copy_cell_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->copy_cell_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->copy_cell_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->copy_cell_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the move button to the toolbar //
	main_window->move_cell_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_move.xpm"), _("Move"), TRUE)) ;
	gtk_widget_ref (main_window->move_cell_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "move_cell_button", main_window->move_cell_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->move_cell_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->move_cell_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->move_cell_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the rotate button to the toolbar //
	main_window->rotate_cell_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_rotate.xpm"), _("Rotate"), TRUE)) ;
	gtk_widget_ref (main_window->rotate_cell_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "rotate_cell_button", main_window->rotate_cell_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->rotate_cell_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->rotate_cell_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->rotate_cell_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the mirror button to the toolbar //
	main_window->mirror_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "q_cell_mirror.xpm"), _("Mirror"), FALSE)) ;
	gtk_widget_ref (main_window->mirror_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "mirror_button", main_window->mirror_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->mirror_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->mirror_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->mirror_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the delete button to the toolbar //
	main_window->delete_cells_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "eraser.xpm"), _("Delete"), TRUE)) ;
	gtk_widget_ref (main_window->delete_cells_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "delete_cells_button", main_window->delete_cells_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->delete_cells_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->delete_cells_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->delete_cells_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the zoom + button to the toolbar //
	main_window->zoom_plus_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "zoom_in.xpm"), _("Zoom +"), FALSE)) ;
	gtk_widget_ref (main_window->zoom_plus_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "zoom_plus_button", main_window->zoom_plus_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_plus_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->zoom_plus_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->zoom_plus_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the zoom - button to the toolbar //
	main_window->zoom_minus_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "zoom_out.xpm"), _("Zoom -"), FALSE)) ;
	gtk_widget_ref (main_window->zoom_minus_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window),"zoom_minus_button", main_window->zoom_minus_button,(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_minus_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->zoom_minus_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->zoom_minus_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the zoom extents button to the toolbar //
	main_window->zoom_extents_button = GTK_WIDGET (create_pixmap_button (create_pixmap (main_window->main_window, "zoom_extents.xpm"), _("Extents"), FALSE)) ;
	gtk_widget_ref (main_window->zoom_extents_button);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window),"zoom_extents_button", main_window->zoom_extents_button,(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->zoom_extents_button);
	gtk_container_add (GTK_CONTAINER (main_window->toolbar2), main_window->zoom_extents_button) ;
	GTK_WIDGET_UNSET_FLAGS (main_window->zoom_extents_button, GTK_CAN_FOCUS | GTK_CAN_DEFAULT) ;
	
	// create and add the table widget to the hbox //
	main_window->table1 = gtk_table_new (2, 2, FALSE);
	gtk_widget_ref (main_window->table1);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "table1", main_window->table1, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->table1);
	gtk_box_pack_start (GTK_BOX (main_window->hbox1), main_window->table1, TRUE, TRUE, 0);
	
        // Add a frame around the drawing area, so it makes a nice border with the other widgets
	main_window->drawing_area_frame = gtk_frame_new (NULL);
	gtk_widget_ref (main_window->drawing_area_frame);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "drawing_area_frame", main_window->drawing_area_frame, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->drawing_area_frame);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->drawing_area_frame, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (main_window->drawing_area_frame), GTK_SHADOW_IN);
	
	// create and add the drawing area to the table //
	// this is the widget where all the action happens //
	main_window->drawing_area = gtk_drawing_area_new ();
	gtk_widget_ref (main_window->drawing_area);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "drawing_area", main_window->drawing_area, (GtkDestroyNotify) gtk_widget_unref);
	GTK_WIDGET_SET_FLAGS (main_window->drawing_area, GTK_CAN_FOCUS);
	gtk_widget_show (main_window->drawing_area);
	gtk_container_add (GTK_CONTAINER (main_window->drawing_area_frame), main_window->drawing_area) ;
	
	// create and add the horizontal ruler to the table //
	// purpose to provide a real time ruler for measuring the design //
	main_window->horizontal_ruler = gtk_hruler_new ();
	gtk_widget_ref (main_window->horizontal_ruler);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "horizontal_ruler", main_window->horizontal_ruler, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->horizontal_ruler);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->horizontal_ruler, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_ruler_set_metric (GTK_RULER (main_window->horizontal_ruler), GTK_PIXELS) ;
	gtk_ruler_set_range (GTK_RULER (main_window->horizontal_ruler), 0, 100, 0, 1);
	
	// create and add the vertical ruler to the table //
	// purpose to provide a real time ruler for measuring the design //
	main_window->vertical_ruler = gtk_vruler_new ();
	gtk_widget_ref (main_window->vertical_ruler);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "vertical_ruler", main_window->vertical_ruler, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->vertical_ruler);
	gtk_table_attach (GTK_TABLE (main_window->table1), main_window->vertical_ruler, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_ruler_set_metric (GTK_RULER (main_window->vertical_ruler), GTK_PIXELS) ;
	gtk_ruler_set_range (GTK_RULER (main_window->vertical_ruler), 0, 100, 0, 1);

	// create and add a scroll window in which to put the command history //
	main_window->scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (main_window->scrolledwindow1);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "scrolledwindow1", main_window->scrolledwindow1, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (main_window->scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	
	// create and add the command history text box to the scrolled window //
	main_window->command_history = gtk_text_new (NULL, NULL);
	gtk_widget_ref (main_window->command_history);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "command_history", main_window->command_history, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->command_history);
	gtk_container_add (GTK_CONTAINER (main_window->scrolledwindow1), main_window->command_history);
	
	// create and add the command entry to the main window //
	main_window->command_entry = gtk_entry_new ();
	gtk_widget_ref (main_window->command_entry);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "command_entry", main_window->command_entry, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->command_entry);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->command_entry, FALSE, FALSE, 0);
	
	// create and add tbe status bar to the main window //
	main_window->status_bar = gtk_statusbar_new ();
	gtk_widget_ref (main_window->status_bar);
	gtk_object_set_data_full (GTK_OBJECT (main_window->main_window), "status_bar", main_window->status_bar, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (main_window->status_bar);
	gtk_box_pack_start (GTK_BOX (main_window->vbox1), main_window->status_bar, FALSE, FALSE, 0);


////////////////////////////////////////////////////////////////////////////////////////
// Connect the callback signals to each of the buttons and menu items in the menus  ////
////////////////////////////////////////////////////////////////////////////////////////


    gtk_signal_connect (GTK_OBJECT (main_window->new_menu_item), "activate", GTK_SIGNAL_FUNC (on_new_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->open_menu_item), "activate",GTK_SIGNAL_FUNC (file_operations), (gpointer)OPEN);
    gtk_signal_connect (GTK_OBJECT (main_window->save_menu_item), "activate", GTK_SIGNAL_FUNC (on_save_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->save_as_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)SAVE);
    gtk_signal_connect (GTK_OBJECT (main_window->print_menu_item), "activate", GTK_SIGNAL_FUNC (on_print_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->preview_menu_item), "activate", GTK_SIGNAL_FUNC (on_preview_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->project_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_project_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->close_menu_item), "activate", GTK_SIGNAL_FUNC (on_close_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->quit_menu_item), "activate", GTK_SIGNAL_FUNC (on_quit_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->undo_menu_item), "activate", GTK_SIGNAL_FUNC (on_undo_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->redo_menu_item), "activate", GTK_SIGNAL_FUNC (on_redo_meu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->copy_menu_item), "activate", GTK_SIGNAL_FUNC (on_copy_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->cut_menu_item), "activate", GTK_SIGNAL_FUNC (on_cut_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->paste_menu_item), "activate", GTK_SIGNAL_FUNC (on_paste_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->grid_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_grid_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->snap_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_snap_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->cell_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_cell_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->window_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_window_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->layer_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_layer_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->preferences_menu_item), "activate", GTK_SIGNAL_FUNC (on_preferences_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->snap_to_grid_menu_item), "activate", GTK_SIGNAL_FUNC (on_snap_to_grid_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->show_grid_menu_item), "activate", GTK_SIGNAL_FUNC (on_show_grid_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->create_block_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)EXPORT);
    gtk_signal_connect (GTK_OBJECT (main_window->import_block_menu_item), "activate", GTK_SIGNAL_FUNC (file_operations), (gpointer)IMPORT);
    gtk_signal_connect (GTK_OBJECT (main_window->create_input_menu_item), "activate", GTK_SIGNAL_FUNC (on_create_input_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->input_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_input_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->connect_output_menu_item), "activate", GTK_SIGNAL_FUNC (on_connect_output_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->clock_select_menu_item), "activate", GTK_SIGNAL_FUNC (on_clock_select_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->clock_increment_menu_item), "activate", GTK_SIGNAL_FUNC (on_clock_increment_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->fixed_polarization), "activate", GTK_SIGNAL_FUNC (on_fixed_polarization_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->measure_distance_menu_item), "activate", GTK_SIGNAL_FUNC (on_measure_distance_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->measurement_preferences1), "activate", GTK_SIGNAL_FUNC (on_measurement_preferences1_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->draw_dimensions_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_dimensions_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->dimension_properties1), "activate", GTK_SIGNAL_FUNC (on_dimension_properties1_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->draw_text_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_text_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->text_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_text_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->draw_arrow_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_arrow_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->arrow_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_arrow_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->draw_line_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_line_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->line_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_line_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->draw_rectangle_menu_item), "activate", GTK_SIGNAL_FUNC (on_draw_rectangle_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->rectangle_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_rectangle_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->start_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_start_simulation_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->stop_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_stop_simulation_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->pause_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_pause_simulation_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->reset_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_reset_simulation_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->calculate_ground_state_menu_item), "activate", GTK_SIGNAL_FUNC (on_calculate_ground_state_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->animate_test_simulation_menu_item), "activate", GTK_SIGNAL_FUNC (on_animate_test_simulation_menu_item_activate), NULL);	
    gtk_signal_connect (GTK_OBJECT (main_window->save_output_to_file_menu_item), "activate", GTK_SIGNAL_FUNC (on_save_output_to_file_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->logging_properties_menu_item), "activate", GTK_SIGNAL_FUNC (on_logging_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->simulation_type_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_simulation_type_setup_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->simulation_engine_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_simulation_engine_setup_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->random_fault_setup_menu_item), "activate", GTK_SIGNAL_FUNC (on_random_fault_setup_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_in_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_in_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_out_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_out_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_die_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_die_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_extents_menu_item), "activate", GTK_SIGNAL_FUNC (on_zoom_extents_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->contents_menu_item), "activate", GTK_SIGNAL_FUNC (on_contents_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->search_menu_item), "activate", GTK_SIGNAL_FUNC (on_search_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->about_menu_item), "activate", GTK_SIGNAL_FUNC (on_about_menu_item_activate), NULL);

    gtk_signal_connect (GTK_OBJECT (main_window->insert_type_1_cell_button), "clicked", GTK_SIGNAL_FUNC (on_insert_type_1_cell_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->insert_type_2_cell_button), "clicked", GTK_SIGNAL_FUNC (on_insert_type_2_cell_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->insert_cell_array_button), "clicked", GTK_SIGNAL_FUNC (on_insert_cell_array_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->move_cell_button), "clicked", GTK_SIGNAL_FUNC (on_move_cell_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->rotate_cell_button), "clicked", GTK_SIGNAL_FUNC (on_rotate_cell_button_clicked), NULL);

    gtk_signal_connect (GTK_OBJECT (main_window->copy_cell_button), "clicked", GTK_SIGNAL_FUNC (on_copy_cell_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->cell_properties_button), "clicked", GTK_SIGNAL_FUNC (on_cell_properties_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->mirror_button), "clicked", GTK_SIGNAL_FUNC (on_mirror_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->delete_cells_button), "clicked", GTK_SIGNAL_FUNC (on_delete_cells_button_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_plus_button), "clicked", GTK_SIGNAL_FUNC (on_zoom_in_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_minus_button), "clicked", GTK_SIGNAL_FUNC (on_zoom_out_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->zoom_extents_button), "clicked", GTK_SIGNAL_FUNC (on_zoom_extents_menu_item_activate), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->command_entry), "changed", GTK_SIGNAL_FUNC (on_command_entry_changed), NULL);

    gtk_object_set_data (GTK_OBJECT (main_window->main_window), "tooltips", main_window->tooltips);

    gtk_window_add_accel_group (GTK_WINDOW (main_window->main_window), main_window->accel_group);

    // attach the necessary signals to the drawing area widget //
    gtk_signal_connect (GTK_OBJECT (main_window->drawing_area), "expose_event", (GtkSignalFunc) expose_event, NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->drawing_area), "configure_event", (GtkSignalFunc) configure_event, NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->drawing_area), "motion_notify_event", (GtkSignalFunc) motion_notify_event, NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->drawing_area), "button_press_event", (GtkSignalFunc) button_press_event, NULL);
    gtk_signal_connect (GTK_OBJECT (main_window->drawing_area), "button_release_event", (GtkSignalFunc) button_release_event, NULL);

    // attach the callback signal for key press //
    gtk_signal_connect(GTK_OBJECT (main_window->main_window), "key_press_event", (GtkSignalFunc) key_press_event, NULL);

    // attach the callback signal for key press //
    gtk_signal_connect(GTK_OBJECT (main_window->main_window), "configure_event", (GtkSignalFunc) main_window_configure_event, NULL);

    // -- Connect the shutdown callback signal to the main window -- //
    gtk_signal_connect(GTK_OBJECT(main_window->main_window), "destroy", GTK_SIGNAL_FUNC (on_quit_menu_item_activate), NULL);

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
