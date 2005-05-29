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
// layer_mapping_dialog.c                               //
// 2004.10.01                                           //
// author: Mike Mazur                                   //
//                                                      //
// description                                          //
//                                                      //
// Layer mapping dialog. When importing a new block,    //
// the user must select, for each block layer, a design //
// layer to merge the block layer's content into.       //
// Alternatively, the user can choose to create a new   //
// layer for a given block layer.                       //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "exp_array.h"
#include "custom_widgets.h"
#include "layer_mapping_dialog.h"

#include "design.h"

typedef struct
  {
  GtkWidget *layerImport;
  GtkWidget *blockLayer;
  GtkWidget *designLayer;
  } LAYER_LINE;

typedef struct
  {
  GtkWidget *dlgLayerMapping;
  GtkWidget *tblLayer;
  GtkWidget *sw ;
  LAYER_LINE *layerLines;
  int totalLayerLines;
  } layer_mapping_D;

static layer_mapping_D layer_mapping_dialog = { NULL };

// function prototypes
static void create_layer_mapping_dialog (layer_mapping_D *dialog);
static void add_layer_line (layer_mapping_D *dialog, QCADLayer *block_layer, int idx, DESIGN *design);
static void set_design_combo_layer_data (GtkWidget *widget, gpointer data);
static void import_layer_toggled (GtkWidget *widget, gpointer data);

EXP_ARRAY *get_layer_mapping_from_user (GtkWidget *parent, DESIGN *design, DESIGN *block)
  {
  int idx = -1;
  GList *llItr = NULL;
  EXP_ARRAY *layer_mappings = NULL ;
  LAYER_MAPPING *layer_mapping = NULL ;
  int number_of_block_layers = 0;		// total number of layers in new block

  // create dialog if it doesn't already exist
  if(NULL == layer_mapping_dialog.dlgLayerMapping)
  	create_layer_mapping_dialog(&layer_mapping_dialog);

  // some boilerplate things
  gtk_window_set_transient_for(GTK_WINDOW(layer_mapping_dialog.dlgLayerMapping), GTK_WINDOW(parent));

  if(NULL != layer_mapping_dialog.layerLines && layer_mapping_dialog.totalLayerLines > 0)
	  {
	  // empty table if it already contains entries
	  for(idx = 0; idx < layer_mapping_dialog.totalLayerLines; idx++)
      {
      gtk_container_remove (GTK_CONTAINER (layer_mapping_dialog.tblLayer), layer_mapping_dialog.layerLines[idx].blockLayer);
      gtk_container_remove (GTK_CONTAINER (layer_mapping_dialog.tblLayer), layer_mapping_dialog.layerLines[idx].designLayer);
      gtk_container_remove (GTK_CONTAINER (layer_mapping_dialog.tblLayer), layer_mapping_dialog.layerLines[idx].layerImport);
      } // for

	  g_free(layer_mapping_dialog.layerLines);
	  layer_mapping_dialog.layerLines = NULL;
	  layer_mapping_dialog.totalLayerLines = 0;
	  } // if

  // count total number of layers on our imported block
  for (llItr = block->lstLayers; llItr != NULL; llItr = llItr->next)
  	number_of_block_layers++;

  // add layer lines to table
  layer_mapping_dialog.layerLines = g_malloc0(number_of_block_layers * sizeof(LAYER_LINE));

  for (llItr = block->lstLayers, idx = 0 ; llItr != NULL ; llItr = llItr->next, idx++)
    {
    add_layer_line(&layer_mapping_dialog, llItr->data, idx, design);
    if(LAYER_TYPE_SUBSTRATE == (QCAD_LAYER (llItr->data))->type)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(layer_mapping_dialog.layerLines[idx].layerImport), FALSE);
    }

  // adjust dialog window size
  scrolled_window_set_size (layer_mapping_dialog.sw, layer_mapping_dialog.tblLayer, 0.8, 0.8) ;

  // only now can we set the total number of lines, otherwise the callbacks
  // iterate over null array entries
  layer_mapping_dialog.totalLayerLines = number_of_block_layers;

  if (GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(layer_mapping_dialog.dlgLayerMapping)))
	  {
    layer_mappings = exp_array_new (sizeof (LAYER_MAPPING), 1) ;
	  for (idx = 0; idx < number_of_block_layers; idx++)
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(layer_mapping_dialog.layerLines[idx].layerImport)))
	      {
        exp_array_insert_vals (layer_mappings, NULL, 1, 1, -1) ;
        layer_mapping = &(exp_array_index_1d (layer_mappings, LAYER_MAPPING, layer_mappings->icUsed - 1)) ;
	      layer_mapping->design_layer = QCAD_LAYER (g_object_get_data (G_OBJECT (layer_mapping_dialog.layerLines[idx].designLayer), "layer"));
	      layer_mapping->block_layer  = QCAD_LAYER (g_object_get_data (G_OBJECT (layer_mapping_dialog.layerLines[idx].blockLayer),  "layer"));
	      }
	  }

  gtk_widget_hide (layer_mapping_dialog.dlgLayerMapping);

  return layer_mappings ;
  }

static void create_layer_mapping_dialog (layer_mapping_D *dialog)
  {
  GtkWidget *lbl = NULL;

  // create the dialog itself
  dialog->dlgLayerMapping = gtk_dialog_new ();
  gtk_window_set_title(GTK_WINDOW (dialog->dlgLayerMapping), _("Map Layers"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlgLayerMapping), TRUE);

  // create the scroll window and add to dialog
  dialog->sw = gtk_scrolled_window_new (
  	GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 1, 1)),
  	GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 1, 1)));
  gtk_widget_show (dialog->sw);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlgLayerMapping)->vbox), dialog->sw, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->sw), 2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  // create table and add it to scroll window
  dialog->tblLayer = gtk_table_new(1, 3, FALSE);
  gtk_widget_show(dialog->tblLayer);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (dialog->sw), dialog->tblLayer) ;
  gtk_container_set_border_width(GTK_CONTAINER(dialog->tblLayer), 2);

  // add headings
  lbl = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (lbl), _("<b>Import</b>")) ;
  gtk_widget_show (lbl);
  gtk_table_attach (GTK_TABLE (dialog->tblLayer), lbl, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
  gtk_label_set_justify (GTK_LABEL(lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC(lbl), 0.0, 0.5);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (lbl), _("<b>Source Layer</b>")) ;
  gtk_widget_show (lbl);
  gtk_table_attach (GTK_TABLE (dialog->tblLayer), lbl, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (lbl), _("<b>Destination Layer</b>")) ;
  gtk_widget_show (lbl);
  gtk_table_attach (GTK_TABLE (dialog->tblLayer), lbl, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5);

  // add OK and Cancel buttons
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgLayerMapping), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlgLayerMapping), GTK_STOCK_OK,     GTK_RESPONSE_OK);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlgLayerMapping), GTK_RESPONSE_OK);
  }

static void add_layer_line (layer_mapping_D *dialog, QCADLayer *block_layer, int idx, DESIGN *design)
  {
  GList *llItr = NULL;
  GList *comboItems = NULL;
  GtkWidget *item = NULL;
  GtkWidget *selected_item = NULL ;

  // add the import check box
  dialog->layerLines[idx].layerImport = gtk_check_button_new ();
  gtk_widget_show (dialog->layerLines[idx].layerImport);
  gtk_table_attach (GTK_TABLE (dialog->tblLayer), dialog->layerLines[idx].layerImport, 0, 1, idx + 1, idx + 2, GTK_FILL, GTK_FILL, 2, 2);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->layerLines[idx].layerImport), TRUE);

  // create and add text box containing block layer
  dialog->layerLines[idx].blockLayer = gtk_entry_new ();
  gtk_widget_show (dialog->layerLines[idx].blockLayer);
  gtk_table_attach (GTK_TABLE (dialog->tblLayer), dialog->layerLines[idx].blockLayer, 1, 2, idx + 1, idx + 2, GTK_FILL, GTK_FILL, 2, 2);
  gtk_entry_set_text (GTK_ENTRY (dialog->layerLines[idx].blockLayer), block_layer->pszDescription);
  gtk_entry_set_editable (GTK_ENTRY (dialog->layerLines[idx].blockLayer), FALSE);
  g_object_set_data (G_OBJECT (dialog->layerLines[idx].blockLayer), "layer", block_layer);

  // combo box containing design layers
  dialog->layerLines[idx].designLayer = gtk_combo_new ();
  gtk_widget_show (dialog->layerLines[idx].designLayer);
  gtk_table_attach (GTK_TABLE(dialog->tblLayer), dialog->layerLines[idx].designLayer, 2, 3, idx + 1, idx + 2, GTK_FILL, GTK_FILL, 2, 2);

  // fill combo box with design layers
  for (llItr = design->lstLayers; llItr != NULL; llItr = llItr->next)
    if ((QCAD_LAYER (llItr->data))->type == block_layer->type)
      {
	    item = gtk_list_item_new_with_label ((QCAD_LAYER (llItr->data))->pszDescription);
      if (!strcmp (block_layer->pszDescription, (QCAD_LAYER (llItr->data))->pszDescription))
        selected_item = item ;
	    gtk_widget_show (item);
	    g_object_set_data (G_OBJECT (item), "layer", llItr->data);
	    g_object_set_data (G_OBJECT (item), "dialog", dialog);
	    g_signal_connect (G_OBJECT (item), "select", (GCallback)(set_design_combo_layer_data), dialog->layerLines[idx].designLayer);
      comboItems = g_list_prepend(comboItems, item);
  	  } // if

  item = gtk_list_item_new_with_label (_("Create New..."));
  if (NULL == selected_item) selected_item = item ;
  gtk_widget_show (item);
  g_object_set_data (G_OBJECT (dialog->layerLines[idx].designLayer), "new_layer_item", item);
  g_object_set_data (G_OBJECT (item), "dialog", dialog);
  g_signal_connect (G_OBJECT (item), "select", (GCallback)(set_design_combo_layer_data), dialog->layerLines[idx].designLayer);
  comboItems = g_list_prepend (comboItems, item);

  if(NULL != comboItems)
    gtk_list_insert_items (GTK_LIST (GTK_COMBO (dialog->layerLines[idx].designLayer)->list), comboItems, 0);

  g_signal_connect (G_OBJECT (dialog->layerLines[idx].layerImport), "toggled", (GCallback)import_layer_toggled, &(dialog->layerLines[idx])) ;
  }

static void set_design_combo_layer_data(GtkWidget *widget, gpointer data)
  {
  layer_mapping_D *dialog = (layer_mapping_D *) g_object_get_data (G_OBJECT (widget), "dialog");
  int idx = -1;
  QCADLayer *layer = NULL;

  g_object_set_data (G_OBJECT (data), "layer", layer = QCAD_LAYER (g_object_get_data (G_OBJECT (widget), "layer")));

  for (idx = 0 ; idx < dialog->totalLayerLines ; idx++)
    if (dialog->layerLines[idx].designLayer != GTK_WIDGET(data))
  	  if (g_object_get_data (G_OBJECT (dialog->layerLines[idx].designLayer), "layer") == layer)
		    if (NULL != g_object_get_data (G_OBJECT (dialog->layerLines[idx].designLayer), "layer"))
		      gtk_list_item_select (GTK_LIST_ITEM (g_object_get_data (G_OBJECT (dialog->layerLines[idx].designLayer), "new_layer_item")));
  }

static void import_layer_toggled (GtkWidget *widget, gpointer data)
  {
  LAYER_LINE *line = (LAYER_LINE *)data;

  gtk_widget_set_sensitive (line->blockLayer,  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
  gtk_widget_set_sensitive (line->designLayer, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
  }
