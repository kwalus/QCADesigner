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
#include <assert.h>
#include <string.h>

//#include "globals.h"
#include "stdio.h"
#include "stdlib.h"
#include "fileio.h"
#include "stdqcell.h"

#define DBG_FIO(s)

// ---------------------------------------------------------------------------------------- //

void read_options(FILE *project_file);
int read_properties(FILE *project_file);
qcell *read_next_qcell(FILE *project_file);
void write_header (FILE *pfile, double dVersion, double dGridSpacing, int icCells) ;
void write_single_cell (FILE *project_file, qcell *cell) ;
char *get_identifier(char *buffer);

// ---------------------------------------------------------------------------------------- //

// creates a file and stores all the cells in the simulation //
gboolean create_file(gchar *file_name, qcell *first_cell){
	FILE *project_file = NULL ;
	int total_cells =0;
	qcell *cell = first_cell;
	
		// -- count the number of cells -- //
	while(cell != NULL){
		total_cells++;
		cell = cell->next;
	}
	
	cell = first_cell;
	
	// -- open the project file -- //
	if((project_file = fopen(file_name, "w")) == NULL){
		printf("Cannot open file %s\n", file_name);
		return FALSE;
	}
      	
	write_header (project_file, qcadesigner_version, grid_spacing, total_cells) ;

	while(cell != NULL){
      	      	write_single_cell (project_file, cell) ;
		cell = cell->next;
	}

	printf("saved project as %s\n", file_name);
	fclose(project_file);
	return TRUE ;
}//create_file

// ---------------------------------------------------------------------------------------- //


void export_block(gchar *file_name, qcell **selected_cells, int number_of_selected_cells){
	
	int j ;
	FILE *block_file = NULL;
	
	if(number_of_selected_cells == 0){
		printf("No cells from which to make a block file\n");
		return ;
	}
	
	if (NULL == (block_file = fopen(file_name, "w+"))){
		fprintf (stderr, "Cannot create that file !\n") ;
		return ;
	}
	
	write_header (block_file, qcadesigner_version, grid_spacing, number_of_selected_cells) ;

	for (j = 0 ; j < number_of_selected_cells ; j++)
      	  write_single_cell (block_file, selected_cells[j]) ;

	printf("saved block as %s\n", file_name);
	fclose(block_file);

}//export_block

void write_header (FILE *project_file, double dVersion, double dGridSpacing, int icCells)
  {
  // -- Write the software version to file -- //
  fprintf(project_file, "[VERSION]\n");
  fprintf(project_file, "qcadesigner_version=%f\n", dVersion);
  fprintf(project_file, "[#VERSION]\n");

  // --write the project options and properties to the file -- //
  fprintf(project_file, "[DESIGN_OPTIONS]\n");
  fprintf(project_file, "grid_spacing=%e\n", dGridSpacing);
  fprintf(project_file, "[#DESIGN_OPTIONS]\n");

  // --write the project properties to the file -- //
  fprintf(project_file, "[DESIGN_PROPERTIES]\n");
  fprintf(project_file, "total_number_of_cells=%d\n", icCells);
  fprintf(project_file, "[#DESIGN_PROPERTIES]\n");
  }

void write_single_cell (FILE *project_file, qcell *cell)
  {
  int i ;
  fprintf(project_file, "[QCELL]\n");

  fprintf(project_file, "x=%e\n",cell->x);
  fprintf(project_file, "y=%e\n",cell->y);

  fprintf(project_file, "top_x=%e\n",cell->top_x);
  fprintf(project_file, "top_y=%e\n",cell->top_y);
  fprintf(project_file, "bot_x=%e\n",cell->bot_x);
  fprintf(project_file, "bot_y=%e\n",cell->bot_y);

  fprintf(project_file, "cell_width=%e\n",cell->cell_width);
  fprintf(project_file, "cell_height=%e\n",cell->cell_height);

  fprintf(project_file, "orientation=%d\n", cell->orientation);

  fprintf(project_file, "color=%d\n", cell->color);
  fprintf(project_file, "clock=%d\n", cell->clock);

  fprintf(project_file, "is_input=%d\n", cell->is_input);
  fprintf(project_file, "is_output=%d\n", cell->is_output);
  fprintf(project_file, "is_fixed=%d\n", cell->is_fixed);

  if(cell->label[strlen(cell->label)-1] != '\n')fprintf(project_file, "label=%s\n", cell->label);
  if(cell->label[strlen(cell->label)-1] == '\n')fprintf(project_file, "label=%s", cell->label);

  fprintf(project_file, "number_of_dots=%d\n", cell->number_of_dots);


  // -- write the dots to the file -- //
  for(i = 0; i < cell->number_of_dots; i++){
	  fprintf(project_file, "[QDOT]\n");
	  fprintf(project_file, "x=%e\n",cell->cell_dots[i].x);
	  fprintf(project_file, "y=%e\n",cell->cell_dots[i].y);
	  fprintf(project_file, "diameter=%e\n",cell->cell_dots[i].diameter);
	  fprintf(project_file, "charge=%e\n",cell->cell_dots[i].charge);
	  fprintf(project_file, "spin=%e\n",cell->cell_dots[i].spin);
	  fprintf(project_file, "potential=%e\n",cell->cell_dots[i].potential);
	  fprintf(project_file, "[#QDOT]\n");
	  }

  fprintf(project_file, "[#QCELL]\n");
  }

qcell *import_block(gchar *file_name, qcell ***p_selected_cells, int *p_number_of_selected_cells, qcell **p_last_cell){
	
	int number_of_cells;
	qcell qc = {NULL} ;
	qcell *qc_start = &qc ;
	// -- file from which to import -- //
	FILE *import_file = fopen(file_name, "r");
	
	// -- free up any currently selected cells -- //
	
	if (!(NULL == (*p_selected_cells) && 0 == (*p_number_of_selected_cells)))
	  {
	  (*p_number_of_selected_cells) = 0;
	  free((*p_selected_cells));
	  (*p_selected_cells) = NULL;
	  }
	
	if(import_file == NULL){
		g_print("cannot open that file!\n");
	}else{
		
		number_of_cells = read_properties(import_file);
		
		if(number_of_cells == 0){
			printf("It appears that the import file has no cells to import\n");
			printf("When creating import files it is important to include and set the total_number_of_cells\n");
			printf("within the [DESIGN_PROPERTIES] tags\n");
			return NULL;
		}
		else
		  {
		  (*p_selected_cells) = calloc(number_of_cells, sizeof(qcell *)); 

		  // -- Reset the file pointer so as to start reading from the first cell -- /
		  rewind(import_file);

		  // -- Read in all the cells in the project file -- //
		  while(!feof(import_file) && (*p_number_of_selected_cells) < number_of_cells)
      	      	    if(((*p_selected_cells)[number_of_selected_cells] = qc_start->next = read_next_qcell(import_file))==NULL)
		      break;
		    else
		      {
		      qc_start->next->previous = qc_start ;
		      qc_start->next->next = NULL ;
		      qc_start = qc_start->next ;
    		      (*p_number_of_selected_cells)++;
		      }

		  (*p_last_cell)->next = qc.next ;
		  if (NULL != qc.next) qc.next->previous = (*p_last_cell) ;
		  (*p_last_cell) = (qc_start == &qc ? NULL : qc_start) ;
		  }
		  
	}
	
	
	return qc.next ;
	
	fclose(import_file);

}//import_block

// ---------------------------------------------------------------------------------------- //

// opens a selected file //
qcell *open_project_file(gchar *file_name, qcell **p_first_cell, qcell **p_last_cell){
	FILE *project_file = NULL ;
	project_file = fopen(file_name, "r");
	qcell qc = {NULL} ;
	qcell *pqc = &qc ;
	
	if(project_file == NULL){
		g_print("cannot open that file!\n");
		return NULL ;
	}else{
		
		// -- Read in the design options from the project file -- //
		read_options(project_file);
				
		// -- Reset the file pointer so as to start reading from the first cell -- /
		rewind(project_file);
		
		// -- Read in all the cells in the project file -- //
		while(!feof(project_file)){
			if (NULL == (pqc->next = read_next_qcell(project_file)))
			  break ;
			else
			  {
			  pqc->next->previous = pqc ;
			  pqc->next->next = NULL ;
			  pqc = pqc->next ;
			  }
		}
		if (NULL != qc.next) qc.next->previous = NULL ;
		(*p_first_cell) = qc.next ;
		(*p_last_cell) = (&qc == pqc ? NULL : pqc) ;
				
	}
	
	fclose(project_file);
	
	printf("Finished opening the project file\n");
	return (*p_first_cell) ;
	
}//open_project_file

// ---------------------------------------------------------------------------------------- //

//!Reads in a cell from a .qca file and returns a pointer to it
qcell *read_next_qcell(FILE *project_file){

	char *identifier = NULL;

	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = calloc(80, sizeof(char));
	
	qcell *cell = NULL;
	
	// -- The dot that is currently being read to -- //
	int current_dot = 0;
	
	// -- make sure the file is not NULL -- //
	if(project_file == NULL){
		printf("Cannot extract the design from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_design()\n");
		return NULL;
	}
	
	// -- read the first line in the file to the buffer -- //
	fgets(buffer, 80, project_file);
	
	// -- check whether that was the end of the file -- //
	if(feof(project_file)){
		//printf("Premature end of file reached your design may not have been loaded\n");
		return NULL;
		}
	
	// -- find the design [DESIGN_OPTIONS] tag -- //	
	while(strcmp(buffer, "[QCELL]\n") != 0){
		fgets(buffer, 80, project_file);
		
		if(feof(project_file)){
			printf("Premature end of file reached your design may not have been loaded\n");
			printf("End of file reached while searching for the opening [QCELL] tag\n");
			return NULL;
			}
	}//while ![QCELL]
	
	//Allocate memory for the cell
	cell = malloc(sizeof(qcell));
	cell->cell_dots = NULL;
			
	// -- read in the first option --//
	fgets(buffer, 80, project_file);
		
	// -- keep reading in lines of the file until the end tag is reached -- //
	while(strcmp(buffer, "[#QCELL]\n") != 0){
		
		// -- get the identifier from the buffer --//
		if((identifier = get_identifier(buffer)) != NULL){
				
				// -- Find and set the appropriate property of the cell from the buffer -- //
				if(!strncmp(identifier, "x", sizeof("x"))){
					cell->x = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "y", sizeof("y"))){
					cell->y = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "top_x", sizeof("top_x"))){
					cell->top_x = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "top_y", sizeof("top_y"))){
					cell->top_y = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "bot_x", sizeof("bot_x"))){
					cell->bot_x = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "bot_y", sizeof("bot_y"))){
					cell->bot_y = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "cell_width", sizeof("cell_width"))){
					cell->cell_width = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "cell_height", sizeof("cell_height"))){
					cell->cell_height = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "orientation", sizeof("orientation"))){
					cell->orientation = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "clock", sizeof("clock"))){
					cell->clock = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "color", sizeof("color"))){
					cell->color = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "is_input", sizeof("is_input"))){
					cell->is_input = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "is_output", sizeof("is_output"))){
					cell->is_output = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "is_fixed", sizeof("is_fixed"))){
					cell->is_fixed = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "number_of_dots", sizeof("number_of_dots"))){
					cell->number_of_dots = atoi(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "label", sizeof("label"))){
					cell->label = calloc(80, sizeof(char));
					// copy the label except for the \n character which shows up as a little box when drawing the label //
					strncpy(cell->label, strchr(buffer,'=')+sizeof(char), strlen(strchr(buffer,'=')+sizeof(char)) - 1);
				}
				
				free(identifier);
				identifier = NULL;				
		
		// -- else check if this the opening tag for a QDOT -- //
		}else if(strcmp(buffer, "[QDOT]\n") == 0){
			
			// -- allocate the memory for the cell dots -- //
			if(cell->cell_dots == NULL){
				if(cell->number_of_dots <= 0 ){
					printf("Error attempting to load the dots into a cell: number_of_dots <=0\n");
					printf("Possibly due to having [QDOT] definition before number_of_dots definition\n");
					printf("The file has failed to load\n");
					clear_all_cells();
					return NULL;
				}
				cell->cell_dots = malloc(sizeof(qdot) * cell->number_of_dots);
				current_dot = 0;
			}
			
			fgets(buffer, 80, project_file);
			
			// -- extract all the data within the [QDOT] tags -- //
			while(strcmp(buffer, "[#QDOT]\n") != 0){
			
				// -- get the identifier from the buffer --//
				if((identifier = get_identifier(buffer)) != NULL){
				
				
				// -- Find and set the appropriate property of the cell from the buffer -- //
				if(!strncmp(identifier, "x", sizeof("x"))){
					cell->cell_dots[current_dot].x = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "y", sizeof("y"))){
					cell->cell_dots[current_dot].y = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "diameter", sizeof("diameter"))){
					cell->cell_dots[current_dot].diameter = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "charge", sizeof("charge"))){
					cell->cell_dots[current_dot].charge = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "spin", sizeof("spin"))){
					cell->cell_dots[current_dot].spin = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				if(!strncmp(identifier, "potential", sizeof("potenital"))){
					cell->cell_dots[current_dot].potential = atof(strchr(buffer,'=')+sizeof(char));
				}
				
				free(identifier);
				identifier = NULL;
				}
				
				if(feof(project_file)){
					printf("Premature end of file reached while trying to locate the [#QDOT] tag\n");
					return NULL;
					}
			
				fgets(buffer, 80, project_file);
			
			}//while not [#QDOT]
			
			// -- Increment the dot counter and check if it is out of the bounds set by number_of_dots -- //
			if(++current_dot > cell->number_of_dots){
				printf("There appear to be more [QDOTS] then the set number_of_dots in one of the cells\n");
				printf("The file has failed to load\n");
				clear_all_cells();
				return NULL;
			}
		}
		
		// -- Make sure that the terminating [#QCELL] tag was found prior to the end of the file -- //		
		if(feof(project_file)){
			printf("Premature end of file reached while trying to locate the [#QCELL] tag\n");
			return NULL;
			}
			
		fgets(buffer, 80, project_file);
	}
	
	// -- Make sure that the dots within the cell have been initalized -- //
	if(cell->cell_dots == NULL){
		printf("QCELL appears to have been loaded without any dots\n");
		printf("Edit the project file and check for [QDOT] tags\n");
		clear_all_cells();
		return NULL;
	}
	
	// -- fill in the pointers for the linked list of cells -- //
	/*
	if(first_cell == NULL){
		first_cell = cell;
		last_cell = cell;
				
		cell->previous = NULL;
		cell->next = NULL;
				
	}else{
	
		cell->previous = last_cell;
		cell->previous->next = cell;
		cell->next = NULL;
		last_cell = cell;
			
	}//else 
	*/
	return cell;

}//read_design

// ---------------------------------------------------------------------------------------- //

//!Reads the project options from the project file
void read_options(FILE *project_file){

	char *identifier = NULL;

	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = calloc(80, sizeof(char));
	
	// start from the beginning of the file //
	rewind(project_file);
	
	// -- make sure the file is not NULL -- //
	if(project_file == NULL){
		printf("Cannot extract options from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_options()\n");
		return;
	}
	
	// -- read the first line in the file to the buffer -- //
	fgets(buffer, 80, project_file);
	
	// -- check whether that was the end of the file -- //
	if(feof(project_file)){
		printf("Premature end of file reached your options may not have been loaded\n");
		printf("End of file reached after reading in the first line of the file\n");
		return;
		}
	
	// -- find the design [DESIGN_OPTIONS] tag -- //	
	while(strcmp(buffer, "[DESIGN_OPTIONS]\n") != 0){
		fgets(buffer, 80, project_file);
		
		if(feof(project_file)){
			printf("Premature end of file reached your options may not have been loaded\n");
			printf("End of file reached while searching for the opening [DESIGN_OPTIONS] tag\n");
			return;
			}
	}
		
	// -- read in the first option --//
	fgets(buffer, 80, project_file);
		
	// -- keep reading in lines of the file until the end tag is reached -- //
	while(strcmp(buffer, "[#DESIGN_OPTIONS]\n") != 0){
		
		// -- get the identifier from the buffer --//
		if((identifier = get_identifier(buffer)) != NULL){
				
				if(!strncmp(identifier, "grid_spacing", sizeof("grid_spacing"))){
					grid_spacing = atof(strchr(buffer,'=')+sizeof(char));
					}
				
				free(identifier);
				identifier = NULL;
			}
		
		if(feof(project_file)){
			printf("Premature end of file reached while trying to locate the [#DESIGN_OPTIONS] tag\n");
			return;
			}
			
		fgets(buffer, 80, project_file);
	}
	
	
}//read_options


// ---------------------------------------------------------------------------------------- //

//!Reads the project properties from the project file; returns the number of cells saved in the file.
int read_properties(FILE *project_file){

	char *identifier = NULL;
	
	int total_number_of_cells = 0;

	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = calloc(80, sizeof(char));
	
	// start from the beginning of the file //
	rewind(project_file);
	
	// -- make sure the file is not NULL -- //
	if(project_file == NULL){
		printf("Cannot extract options from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_properties()\n");
		return 0;
	}
	
	// -- read the first line in the file to the buffer -- //
	fgets(buffer, 80, project_file);
	
	// -- check whether that was the end of the file -- //
	if(feof(project_file)){
		printf("Premature end of file reached your properties may not have been loaded\n");
		printf("End of file reached after reading in the first line of the file\n");
		return 0;
		}
	
	// -- find the design [DESIGN_OPTIONS] tag -- //	
	while(strcmp(buffer, "[DESIGN_PROPERTIES]\n") != 0){
		fgets(buffer, 80, project_file);
		
		if(feof(project_file)){
			printf("Premature end of file reached your design properties may not have been loaded\n");
			printf("End of file reached while searching for the opening [DESIGN_PROPERTIES] tag\n");
			return 0;
			}
	}
		
	// -- read in the first option --//
	fgets(buffer, 80, project_file);
		
	// -- keep reading in lines of the file until the end tag is reached -- //
	while(strcmp(buffer, "[#DESIGN_PROPERTIES]\n") != 0){
		
		// -- get the identifier from the buffer --//
		if((identifier = get_identifier(buffer)) != NULL){
				
				if(!strncmp(identifier, "total_number_of_cells", sizeof("total_number_of_cells"))){
					total_number_of_cells = atof(strchr(buffer,'=')+sizeof(char));
					}
				
				free(identifier);
				identifier = NULL;
			}
		
		if(feof(project_file)){
			printf("Premature end of file reached while trying to locate the [#DESIGN_PROPERTIES] tag\n");
			return 0;
			}
			
		fgets(buffer, 80, project_file);
	}
	
	return total_number_of_cells;
	
}//read_properties


// ---------------------------------------------------------------------------------------- //

//!Gets the variable identifier from a text string
char *get_identifier(char *buffer){
	int i = 0;
	char *identifier = NULL;
	int passed = FALSE;
	
	// get all the characters up the the =, this should be the entire identifier -- //
	while(i < strlen(buffer)){
	
		if(buffer[i] == '='){
			passed = TRUE;
			break;
			}
				
	 	i++;
	 	}
		
	//If = was found allocate and copy the identifier to the string
	if(passed == TRUE){
		identifier = calloc(80, sizeof(char));
		strncpy(identifier, buffer, i);
	}
		
	return identifier;
}//get_identifier

// ---------------------------------------------------------------------------------------- //

// creates a file and stores the vector table //
void create_vector_file(gchar *file_name){
	
	FILE *vector_file;
	
	// -- open the vector file -- //
	if((vector_file = fopen(file_name, "w")) == NULL){
		printf("Cannot open that file\n");
		return;
		}

	fprintf(vector_file, "$$VECTOR_TABLE$$\n");	
	fprintf(vector_file, gtk_editable_get_chars(GTK_EDITABLE(vector_table_options.vector_table_textbox),0,-1));
		
	printf("saved vector table as %s\n", file_name);
	
	fclose(vector_file);
	
}//create_vector_file

// opens a file and loads a vector table //
void open_vector_file(gchar *file_name){
	
	FILE *vector_file;
	gint position=0;
	
	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = calloc(80, sizeof(char));
	
	// -- open the vector table file -- //
	if((vector_file = fopen(file_name, "r")) == NULL){
		printf("Cannot open that file\n");
		return;
		}

	// start from the beginning of the file //
	rewind(vector_file);	
		
	// -- read the first line in the file to the buffer -- //
	fgets(buffer, 80, vector_file);
	
	if(strcmp(buffer, "$$VECTOR_TABLE$$\n") != 0){
		printf("It does not appear that this is a valid vector table file\nA valid vector table file begins with $$VECTOR_TABLE$$\n");
		fclose(vector_file);
		return;
	}

	// -- check whether that was the end of the file -- //
	if(feof(vector_file)){
		printf("Premature end of vector file reached\n");
		printf("End of file reached after reading in the first line of the file\n");
		fclose(vector_file);
		return;
		}	
		
	while(!feof(vector_file)){
		fgets(buffer, 80, vector_file);
		gtk_editable_insert_text(GTK_EDITABLE(vector_table_options.vector_table_textbox), buffer, strlen(buffer), &position);
	}		
		
	fclose(vector_file);	
	
}//open_vector_file

char *base_name (char *pszFile)
  {
  char *pszRet = &(pszFile[strlen (pszFile)]) ;
  while (--pszRet > pszFile)
    if (*pszRet == '/')
      return pszRet + 1 ;
  return pszFile ;
  }
