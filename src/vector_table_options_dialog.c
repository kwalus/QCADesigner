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


#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>


#include "support.h"
#include "stdqcell.h"
#include "cad.h"
#include "globals.h"
#include "vector_table_options_dialog.h"
#include "fileio.h"
#include "file_selection_window.h"

static GtkAdjustment *available_adjust = NULL;
static GtkAdjustment *activated_adjust = NULL;

extern vector_data vector_table ;
extern activated_input_list active_inputs ;

void vector_table_add_button_clicked(GtkWidget *widget, GtkWidget *list);
void vector_table_remove_button_clicked(GtkWidget *widget, GtkWidget *list);
void vector_table_select_activated_row(GtkWidget *widget,gint row,gint column, GdkEvent *event, gpointer data);
void vector_table_select_available_row(GtkWidget *widget,gint row,gint column, GdkEvent *event, gpointer data);
void vector_table_file_operations (GtkButton *button, gpointer user_data);
void on_vector_table_options_ok_button_clicked(GtkButton *button, gpointer user_data);
void destroy_vector_table_options_dialog(GtkWidget *widget, gpointer user_data);
void parse_vector_table();
void vector_table_clear_lists(GtkWidget *list);

void create_vector_table_options_dialog(vector_table_options_D *dialog, GtkWindow *parent) {
	
	int number_of_inputs = 0;
	qcell *cell = first_cell;
	static gchar *titles1[] = {"AVAILABLE"};
	static gchar *titles2[] = {"ACTIVATED"};
	
	//count number of inputs
	while(cell != NULL){
			if(cell->is_input == TRUE)number_of_inputs++;
			cell=cell->next;
	}
	
	dialog->available_adjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, number_of_inputs, 0.1, 1.0, 1.0));
	dialog->activated_adjust = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, number_of_inputs, 0.1, 1.0, 1.0));
	
	dialog->vector_table_options_dialog = gtk_dialog_new ();
//	gtk_widget_set_usize(dialog->vector_table_options_dialog, 450, 750);
	gtk_object_set_data (GTK_OBJECT (dialog->vector_table_options_dialog), "vector_table_options_dialog", dialog->vector_table_options_dialog);
	gtk_window_set_title (GTK_WINDOW (dialog->vector_table_options_dialog), "Vector Table Options");
	gtk_window_set_policy (GTK_WINDOW (dialog->vector_table_options_dialog), FALSE, FALSE, FALSE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog->vector_table_options_dialog), parent) ;
	gtk_window_set_modal (GTK_WINDOW (dialog->vector_table_options_dialog), TRUE) ;
	
	dialog->dialog_vbox1 = GTK_DIALOG (dialog->vector_table_options_dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (dialog->vector_table_options_dialog), "dialog_vbox1", dialog->dialog_vbox1);
	gtk_widget_show (dialog->dialog_vbox1);
	
	dialog->top_vbox = gtk_table_new (3, 1, FALSE) ;
	gtk_widget_ref (dialog->top_vbox);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "top_vbox", dialog->top_vbox,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->top_vbox);
	gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->top_vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->top_vbox), 2) ;
	
	dialog->instructions_label = gtk_label_new ("Select and activate inputs which you wish to use with the vector table.\nNOTE: The input which is at the top of the list is connected to the Most Significant Bit.");
	gtk_widget_ref (dialog->instructions_label);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "instructions_label", dialog->instructions_label,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->instructions_label);
	gtk_table_attach (GTK_TABLE (dialog->top_vbox), dialog->instructions_label, 0, 1, 0, 1,
	  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
	  (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
	gtk_misc_set_alignment (GTK_MISC (dialog->instructions_label), 0.0, 0.5) ;
	gtk_label_set_justify (GTK_LABEL (dialog->instructions_label), GTK_JUSTIFY_LEFT) ;
//        gtk_label_set_line_wrap (GTK_LABEL (dialog->instructions_label), TRUE);
	
	dialog->list_frame = gtk_frame_new ("Input List");
	gtk_widget_ref (dialog->list_frame);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "list_frame", dialog->list_frame,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->list_frame);
	gtk_table_attach (GTK_TABLE (dialog->top_vbox), dialog->list_frame, 0, 1, 1, 2,
	  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
	  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
	gtk_container_set_border_width (GTK_CONTAINER (dialog->list_frame), 2);
	
	dialog->vector_table_frame = gtk_frame_new ("Vector Table");
	gtk_widget_ref (dialog->vector_table_frame);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "vector_table_frame", dialog->vector_table_frame,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->vector_table_frame);
	gtk_table_attach (GTK_TABLE (dialog->top_vbox), dialog->vector_table_frame, 0, 1, 2, 3,
	  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
	  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
	gtk_container_set_border_width (GTK_CONTAINER (dialog->vector_table_frame), 2);
	
	dialog->scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_usize (dialog->scrolledwindow1, 300, 200) ;
	gtk_widget_ref (dialog->scrolledwindow1);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "scrolledwindow1", dialog->scrolledwindow1,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->scrolledwindow1);
	gtk_container_add (GTK_CONTAINER (dialog->vector_table_frame), dialog->scrolledwindow1);
	gtk_widget_set_usize (dialog->scrolledwindow1, 300, 200) ;
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->scrolledwindow1), 2) ;
	
	dialog->vector_table_textbox = gtk_text_new (NULL, NULL);
	gtk_widget_ref (dialog->vector_table_textbox);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "vector_table_textbox", dialog->vector_table_textbox,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->vector_table_textbox);
	gtk_container_add (GTK_CONTAINER (dialog->scrolledwindow1), dialog->vector_table_textbox);
	gtk_text_set_editable (GTK_TEXT (dialog->vector_table_textbox), TRUE);
	gtk_text_insert (GTK_TEXT (dialog->vector_table_textbox), NULL, NULL, NULL,
			   "#<-is a comment. Any lines with this are ignored.\n#The following is an example table assuming three inputs are active.\n#All the numbers must be in binary.\n001\n110\n010\n111\n#Pressing \"Close\" will load the vector table to memory", -1);
	
	dialog->dialog_action_area1 = GTK_DIALOG (dialog->vector_table_options_dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dialog->vector_table_options_dialog), "dialog_action_area1", dialog->dialog_action_area1);
	gtk_widget_show (dialog->dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog_action_area1), 0);
	
	dialog->hbox1 = gtk_hbutton_box_new ();
	gtk_widget_ref (dialog->hbox1);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "hbox1", dialog->hbox1,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->hbox1);
	gtk_box_pack_start (GTK_BOX (dialog->dialog_action_area1), dialog->hbox1, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->hbox1), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (dialog->hbox1), 0);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (dialog->hbox1), 0, 0);
	
	dialog->vector_table_options_load_table_button = gtk_button_new_with_label ("Load From File...");
	gtk_widget_ref (dialog->vector_table_options_load_table_button);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "vector_table_options_load_table_button", dialog->vector_table_options_load_table_button,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->vector_table_options_load_table_button);
	gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vector_table_options_load_table_button, TRUE, TRUE, 0);
        GTK_WIDGET_SET_FLAGS (dialog->vector_table_options_load_table_button, GTK_CAN_DEFAULT);
	
	dialog->vector_table_options_save_table_button = gtk_button_new_with_label ("Save To File...");
	gtk_widget_ref (dialog->vector_table_options_save_table_button);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "vector_table_options_save_table_button", dialog->vector_table_options_save_table_button,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->vector_table_options_save_table_button);
	gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vector_table_options_save_table_button, TRUE, TRUE, 0);
      	GTK_WIDGET_SET_FLAGS (dialog->vector_table_options_save_table_button, GTK_CAN_DEFAULT);
	
	dialog->vector_table_options_ok_button = gtk_button_new_with_label ("Close");
	gtk_widget_ref (dialog->vector_table_options_ok_button);
	gtk_object_set_data_full (GTK_OBJECT (dialog->vector_table_options_dialog), "dialog->vector_table_options_ok_button", dialog->vector_table_options_ok_button,
						(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog->vector_table_options_ok_button);
	gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vector_table_options_ok_button, TRUE, TRUE, 0);
      	GTK_WIDGET_SET_FLAGS (dialog->vector_table_options_ok_button, GTK_CAN_DEFAULT);
	
	
	dialog->list_table = gtk_table_new(15,5,TRUE);
	
	gtk_widget_show(dialog->list_table);
	
	dialog->list_hbox = gtk_hbox_new(FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(dialog->list_table), dialog->list_hbox,0,5,14,15);
		
	gtk_widget_show(dialog->list_hbox);
	
	dialog->list_vbox = gtk_vbox_new(FALSE,0);
	gtk_table_attach(GTK_TABLE(dialog->list_table), dialog->list_vbox,2,3,5,10,0,0,5,5);
	
	dialog->list_add_button = gtk_button_new_with_label("Add");
	gtk_box_pack_start(GTK_BOX(dialog->list_vbox), dialog->list_add_button,FALSE,FALSE,10);
	gtk_widget_show(dialog->list_add_button);
	
	dialog->list_remove_button = gtk_button_new_with_label("Remove");
	gtk_box_pack_start(GTK_BOX(dialog->list_vbox),dialog->list_remove_button,FALSE,FALSE,0);
	gtk_widget_show(dialog->list_remove_button);
	
	gtk_widget_show(dialog->list_vbox);
	
	dialog->available_box = gtk_hbox_new(FALSE,0);
	gtk_table_attach(GTK_TABLE(dialog->list_table),dialog->available_box,0,2,0,13,GTK_FILL,GTK_FILL,0,0);
	
	dialog->available_list = gtk_clist_new_with_titles(1,titles1);
	gtk_box_pack_start(GTK_BOX(dialog->available_box),dialog->available_list,TRUE,TRUE,0);
		
	vector_table_clear_lists(dialog->available_list);
	gtk_widget_show(dialog->available_list);
	gtk_clist_set_selection_mode (GTK_CLIST(dialog->available_list),GTK_SELECTION_SINGLE);
	
	dialog->available_scrbar = gtk_vscrollbar_new(available_adjust);
	gtk_box_pack_start(GTK_BOX(dialog->available_box),dialog->available_scrbar,FALSE,TRUE,4);
	gtk_widget_show(dialog->available_scrbar);
	
	gtk_widget_show(dialog->available_box);
	
	dialog->activated_box = gtk_hbox_new(FALSE,0);
	gtk_table_attach(GTK_TABLE(dialog->list_table),dialog->activated_box,3,5,0,13,GTK_FILL,GTK_FILL,0,0);
	
	dialog->activated_list = gtk_clist_new_with_titles(1,titles2);
	gtk_box_pack_start(GTK_BOX(dialog->activated_box),dialog->activated_list,TRUE,TRUE,0);

	gtk_widget_show(dialog->activated_list);
	gtk_clist_set_selection_mode (GTK_CLIST(dialog->activated_list),GTK_SELECTION_SINGLE);
	
	dialog->activated_scrbar = gtk_vscrollbar_new(activated_adjust);
	gtk_box_pack_start(GTK_BOX(dialog->activated_box),dialog->activated_scrbar,FALSE,TRUE,4);
	gtk_widget_show(dialog->activated_scrbar);
	
	gtk_widget_show(dialog->activated_box);
	
	gtk_container_add(GTK_CONTAINER(dialog->list_frame),dialog->list_table);
	
	gtk_signal_connect(GTK_OBJECT(dialog->list_add_button), "clicked",
	(GtkSignalFunc)vector_table_add_button_clicked, dialog->activated_list);
	
	gtk_signal_connect(GTK_OBJECT(dialog->list_remove_button), "clicked",
	(GtkSignalFunc)vector_table_remove_button_clicked, dialog->activated_list);
	
	gtk_signal_connect(GTK_OBJECT(dialog->activated_list), "select_row",
	(GtkSignalFunc)vector_table_select_activated_row, dialog->list_table);
	
	gtk_signal_connect(GTK_OBJECT(dialog->available_list), "select_row",
	(GtkSignalFunc)vector_table_select_available_row, dialog->list_table);

	gtk_signal_connect (GTK_OBJECT (dialog->vector_table_options_load_table_button), "clicked",
	  GTK_SIGNAL_FUNC (vector_table_file_operations), (gpointer)OPEN);
	
	gtk_signal_connect (GTK_OBJECT (dialog->vector_table_options_save_table_button), "clicked",
	  GTK_SIGNAL_FUNC (vector_table_file_operations), (gpointer)SAVE);
	
	gtk_signal_connect (GTK_OBJECT (dialog->vector_table_options_ok_button), "clicked",
	  GTK_SIGNAL_FUNC (on_vector_table_options_ok_button_clicked),
	dialog->vector_table_options_dialog);
	
	// connect the destroy function for when the user clicks the "x" to close the window //
	gtk_signal_connect (GTK_OBJECT (dialog->vector_table_options_dialog), "destroy",
	GTK_SIGNAL_FUNC (destroy_vector_table_options_dialog), dialog->vector_table_options_dialog);
	
	gtk_clist_set_vadjustment (GTK_CLIST(dialog->available_list),available_adjust);

	gtk_clist_set_vadjustment (GTK_CLIST(dialog->activated_list),activated_adjust);
		
	
}

void destroy_vector_table_options_dialog(GtkWidget *widget, gpointer user_data){
	gtk_widget_destroy(GTK_WIDGET(vector_table_options.vector_table_options_dialog));
	vector_table_options.vector_table_options_dialog = NULL;
}

void vector_table_file_operations (GtkButton *button, gpointer user_data)
  {
  char szFName[PATH_LENGTH] = "" ;
  int fFileOp = (int)user_data ;
  get_file_name_from_user (GTK_WINDOW (vector_table_options.vector_table_options_dialog),
    fFileOp == OPEN ? "Load Vector Table" :
    fFileOp == SAVE ? "Save Vector Table" :
    "Select File",
    szFName, PATH_LENGTH) ;
  
  if (szFName[0] != 0)
    {
    if (OPEN == fFileOp)
      open_vector_file (szFName) ;
    else if (SAVE == fFileOp)
      create_vector_file (szFName) ;
    }
  }
	
void on_vector_table_options_ok_button_clicked(GtkButton *button, gpointer user_data){
	parse_vector_table();
	gtk_widget_hide(GTK_WIDGET(user_data));
}

void vector_table_options_refresh_input_list(){
	
	qcell *cell = first_cell;
	int i = 0;
	
	free(active_inputs.available_cells);
	active_inputs.num_available = 0;
	
	gtk_clist_clear(GTK_CLIST(vector_table_options.available_list));
	
	// count the total number of inputs in the design //
	while(cell != NULL){
		if(cell->is_input == TRUE){
			active_inputs.num_available++;
		}
		cell = cell->next;
	}
	
	cell = first_cell;
	
	// allocate memory for the available cells list //
	active_inputs.available_cells = malloc(active_inputs.num_available*sizeof(qcell *));
	
	// copy the pointers to the inputs into the available input cells list //
	while(cell != NULL){
		if(cell->is_input == TRUE){
			active_inputs.available_cells[i] = cell;
			gtk_clist_append(GTK_CLIST(vector_table_options.available_list),&cell->label);
			i++;
		}
		cell = cell->next;
	}
	
}//vector_table_options_refresh_input_list

// fills the list of available inputs //
void vector_table_clear_lists(GtkWidget *list){
	
	active_inputs.activated_cells = NULL;
	active_inputs.available_cells = NULL;
	vector_table.data = NULL;
	vector_table.num_of_vectors = 0;
	vector_table.num_of_bits = 0;
	active_inputs.num_available = 0;	
	active_inputs.num_activated = 0;
	

}//vector_table_fill_available_list

// Handles event when a row of the activated list is clicked.
void vector_table_select_activated_row(GtkWidget *widget,gint row,gint column, GdkEvent *event, gpointer data){
	
	active_inputs.activated_selected_row = row;
	active_inputs.available_selected_row = -1;
	
}

// Handles event when a row of the available list is clicked.
void vector_table_select_available_row(GtkWidget *widget,gint row,gint column, GdkEvent *event, gpointer data){

	active_inputs.available_selected_row = row;
	active_inputs.activated_selected_row = -1;
	
}

void vector_table_add_button_clicked(GtkWidget *widget, GtkWidget *list){
	
	int i;
	qcell **temp_cells;
	
	if(active_inputs.available_selected_row  != -1){
		
		// check if the selected item is already in the activated list //
		for(i = 0; i < active_inputs.num_activated; i++){
				if(active_inputs.activated_cells[i] == active_inputs.available_cells[active_inputs.available_selected_row])return;
		}
		
		active_inputs.num_activated++;
		
		// resize the memory for the activated cell list //
		temp_cells = realloc(active_inputs.activated_cells, active_inputs.num_activated * sizeof(qcell *));
		active_inputs.activated_cells = temp_cells;
		
		// add the newly selected item to the activated list //
		active_inputs.activated_cells[active_inputs.num_activated - 1] =  active_inputs.available_cells[active_inputs.available_selected_row ];
		
		// show the new active input in the list //
		gtk_clist_append(GTK_CLIST(list),&active_inputs.activated_cells[active_inputs.num_activated - 1]->label);
		
		// clear the current selection //
		active_inputs.available_selected_row  = -1;
		
	}
}

void vector_table_remove_button_clicked(GtkWidget *widget, GtkWidget *list){
	int i;
	qcell **temp_cells;
	
	if(active_inputs.activated_selected_row != -1){
		
		// move all the items in the list up to fill the gap //
		for(i = active_inputs.activated_selected_row; i < active_inputs.num_activated - 1; i++)
				active_inputs.activated_cells[i] = active_inputs.activated_cells[i+1];
		
		// resize the memory for the activated cell list //
		active_inputs.num_activated--;
		temp_cells = realloc(active_inputs.activated_cells, active_inputs.num_activated * sizeof(qcell *));
		active_inputs.activated_cells = temp_cells;
		
		// remove the item from the list //
		gtk_clist_remove (GTK_CLIST(list), active_inputs.activated_selected_row);
		
		active_inputs.activated_selected_row = -1;
	
	}
}

void parse_vector_table(){
	
	int i, row, column;
	int text_length;
	char *table_text;
	
	vector_table.num_of_vectors = 0;
		
	// get the total length of text in the text box including all newline chars //
	text_length = gtk_text_get_length (GTK_TEXT(vector_table_options.vector_table_textbox));
	
	//allocate memory for text //
	table_text = malloc(text_length*sizeof(char));
	
	// get all the text contained in the text box
	table_text = gtk_editable_get_chars(GTK_EDITABLE(vector_table_options.vector_table_textbox),0,-1);
	
	// loop throught the entire text //
	for(i = 0; i < text_length; i++){
		
		//if first character in line is a # skip that line because its a comment //
		while(table_text[i]=='#'){
			
			// find the \n char to indicate that this is the end of the line //
			while(table_text[i]!='\n'){
				i++;
				if(i >= text_length)break;
			}
			
			// currently pointing to the \n char have to advance one //
			i++;
		}
		
		// if this is the end of the text exit //
		if(i >= text_length)break;
			
		vector_table.num_of_vectors++;
		
		// find the total number of bits
		vector_table.num_of_bits=0;
			
		while(table_text[i]!='\n'){
			
			// make sure that only 1's and 0's are in the vector //
			if(table_text[i]!='0' && table_text[i]!='1'){
				
				 // write message to the command history window //
				char *text = "ERROR: Vector table invalid character in binary representation !\n";
				gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL,text, strlen(text));
				return;
			}
			
			vector_table.num_of_bits++;
			i++;
			if(i >= text_length)break;
				
		}
				
	}
	
	// make sure that the number of bits in the vector table matches the number of activated inputs //
	if(vector_table.num_of_bits != active_inputs.num_activated){
		// write message to the command history window //
		char *text = "ERROR: Number of active inputs does not match number of bits in vector table!\nThe vector table was not loaded!\n" ;
		gtk_text_insert(GTK_TEXT(main_window.command_history), NULL, NULL, NULL, text, strlen(text));
		return;
	}
	
	// initialize the vector table data 2D array //
	vector_table.data = calloc(vector_table.num_of_vectors, sizeof(int *));
	for(i = 0; i < vector_table.num_of_vectors; i++){
		vector_table.data[i] = calloc(vector_table.num_of_bits, sizeof(int));
	}
	
	// used to index the data array //
	row = 0;
	column = 0;
			
	// loop throught the entire text this time filling in the vector table data array //
	for(i = 0; i < text_length; i++){
		
		//if first character in line is a # skip that line because its a comment //
		while(table_text[i]=='#'){
			
			// find the \n char to indicate that this is the end of the line //
			while(table_text[i]!='\n'){
				i++;
				if(i >= text_length)break;
			}
			
			// currently pointing to the \n char have to advance one //
			i++;
		}
		
		// if this is the end of the text exit //
		if(i >= text_length)break;
		
		column = 0;
		
		while(table_text[i]!='\n'){
			
			if(table_text[i]=='1')vector_table.data[row][column] = 1;
			if(table_text[i]=='0')vector_table.data[row][column] = 0;	
			
			column++;
			i++;
			if(i >= text_length)break;
				
		}
		row++;
				
	}
	
	/*
	for(i = 0; i < vector_table.num_of_vectors; i++){
		for(j = 0; j < vector_table.num_of_bits; j++)
			printf("%d", vector_table.data[i][j]);
		printf("\n");
	}
	*/
	//printf("number bits = %d, number vectors = %d\n", vector_table.num_of_bits,vector_table.num_of_vectors);
	
}//parse_vector_table
