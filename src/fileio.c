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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"
#include "stdqcell.h"
#include "gqcell.h"
#include "undo_create.h"
#include "fileio_helpers.h"

#define DBG_FIO(s)

// ---------------------------------------------------------------------------------------- //

static void read_options(FILE *project_file, double *pgrid_spacing);
static int read_properties(FILE *project_file);
static void write_header (FILE *pfile, double dVersion, double dGridSpacing, int icCells) ;
static char *get_identifier(char *buffer);

static double qcadesigner_version = 1.3 ;

// ---------------------------------------------------------------------------------------- //

// creates a file and stores all the cells in the simulation //
gboolean create_file(gchar *file_name, GQCell *first_cell, double grid_spacing){
	FILE *project_file = NULL ;
	int total_cells =0;
	GQCell *cell = first_cell;
	
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
      	      	gqcell_serialize (cell, project_file) ;
		cell = cell->next;
	}

	printf("saved project as %s\n", file_name);
	fclose(project_file);
	return TRUE ;
}//create_file

// ---------------------------------------------------------------------------------------- //


void export_block(gchar *file_name, GQCell **selected_cells, int number_of_selected_cells, double grid_spacing){
	
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
      	  gqcell_serialize (selected_cells[j], block_file) ;

	printf("saved block as %s\n", file_name);
	fclose(block_file);

}//export_block

static void write_header (FILE *project_file, double dVersion, double dGridSpacing, int icCells)
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

GQCell *import_block(gchar *file_name, GQCell ***p_selected_cells, int *p_number_of_selected_cells, GQCell **p_last_cell){
	
	int number_of_cells;
	GQCell qc ;
	GQCell *qc_start = &qc ;
	// -- file from which to import -- //
	FILE *import_file = fopen(file_name, "r");
	
        DBG_FIO (fprintf (stderr, "Importing block file %s\n", file_name)) ;
        
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
		  (*p_selected_cells) = calloc(number_of_cells, sizeof(GQCell *)); 

		  // -- Reset the file pointer so as to start reading from the first cell -- /
		  rewind(import_file);

		  // -- Read in all the cells in the project file -- //
		  while(!feof(import_file) && (*p_number_of_selected_cells) < number_of_cells)
      	      	    if(((*p_selected_cells)[(*p_number_of_selected_cells)] = qc_start->next = gqcell_new_from_stream (import_file))==NULL)
		      break;
		    else
		      {
		      qc_start->next->prev = qc_start ;
		      qc_start->next->next = NULL ;
		      qc_start = qc_start->next ;
    		      (*p_number_of_selected_cells)++;
		      }
                  
                  if (NULL != (*p_last_cell))
  		    (*p_last_cell)->next = qc.next ;
		  if (NULL != qc.next)
                    qc.next->prev = (*p_last_cell) ;
                  if (qc_start != &qc)
                    (*p_last_cell) = qc_start ;
		  }
		  
	}
	
	
	return qc.next ;
	
	fclose(import_file);

}//import_block

// ---------------------------------------------------------------------------------------- //

// opens a selected file //
GQCell *open_project_file(gchar *file_name, GQCell **p_first_cell, GQCell **p_last_cell, double *pgrid_spacing){
	FILE *project_file = NULL ;
	GQCell qc ;
	GQCell *pqc = &qc ;
	project_file = fopen(file_name, "r");
	
	if(project_file == NULL){
		g_print("cannot open that file!\n");
		return NULL ;
	}else{
		DBG_FIO (fprintf (stderr, "Attempting to open project file \"%s\"\n", file_name)) ;
		// -- Read in the design options from the project file -- //
		read_options(project_file, pgrid_spacing);
		DBG_FIO (fprintf (stderr, "After read_options\n")) ;
				
		// -- Reset the file pointer so as to start reading from the first cell -- /
		rewind(project_file);
		
		// -- Read in all the cells in the project file -- //
		while(!feof(project_file)){
			DBG_FIO (fprintf (stderr, "Calling qcell_new_from_stream\n")) ;
			if (NULL == (pqc->next = gqcell_new_from_stream (project_file)))
			  break ;
			else
			  {
                          InitUndoHistory (pqc->next) ;
			  pqc->next->prev = pqc ;
			  pqc->next->next = NULL ;
			  pqc = pqc->next ;
			  }
			DBG_FIO (fprintf (stderr, "Called qcell_new_from_stream\n")) ;
		}
		if (NULL != qc.next) qc.next->prev = NULL ;
		(*p_first_cell) = qc.next ;
		(*p_last_cell) = (&qc == pqc ? NULL : pqc) ;
				
	}
	
	fclose(project_file);
	
	printf("Finished opening the project file\n");
	return (*p_first_cell) ;
	
}//open_project_file

//!Reads the project options from the project file
static void read_options(FILE *project_file, double *pgrid_spacing){

	char *identifier = NULL;

	//The file read buffer is 80 characters long any lines in the file longer then this will
	//result in unexpected outputs.
	char *buffer = NULL ;
	
	// start from the beginning of the file //
	rewind(project_file);
	
	// -- make sure the file is not NULL -- //
	if(project_file == NULL){
		printf("Cannot extract options from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_options()\n");
		return;
	}
	
	// -- read the first line in the file to the buffer -- //
        buffer = ReadLine (project_file, 0) ;
	
	// -- check whether that was the end of the file -- //
	if(feof(project_file)){
		printf("Premature end of file reached your options may not have been loaded\n");
		printf("End of file reached after reading in the first line of the file\n");
		return;
		}
	
	// -- find the design [DESIGN_OPTIONS] tag -- //	
	while(strcmp(buffer, "[DESIGN_OPTIONS]") != 0){
                buffer = ReadLine (project_file, 0) ;
		
		if(feof(project_file)){
			printf("Premature end of file reached your options may not have been loaded\n");
			printf("End of file reached while searching for the opening [DESIGN_OPTIONS] tag\n");
			return;
			}
	}
		
	// -- read in the first option --//
        buffer = ReadLine (project_file, 0) ;
		
	// -- keep reading in lines of the file until the end tag is reached -- //
	while(strcmp(buffer, "[#DESIGN_OPTIONS]") != 0){
		
		// -- get the identifier from the buffer --//
		if((identifier = get_identifier(buffer)) != NULL){
				
				if(!strncmp(identifier, "grid_spacing", sizeof("grid_spacing"))){
					(*pgrid_spacing) = atof(strchr(buffer,'=')+sizeof(char));
					}
				
				free(identifier);
				identifier = NULL;
			}
		
		if(feof(project_file)){
			printf("Premature end of file reached while trying to locate the [#DESIGN_OPTIONS] tag\n");
			return;
			}
		
                buffer = ReadLine (project_file, 0) ;	
	}
	
	
}//read_options


// ---------------------------------------------------------------------------------------- //

//!Reads the project properties from the project file; returns the number of cells saved in the file.
static int read_properties(FILE *project_file){

	char *identifier = NULL;
	
	int total_number_of_cells = 0;
        
        char *buffer = NULL ;

	// start from the beginning of the file //
	rewind(project_file);
	
	// -- make sure the file is not NULL -- //
	if(project_file == NULL){
		printf("Cannot extract options from a NULL file\n");
		printf("The pointer to the project file was passed as NULL to read_properties()\n");
		return 0;
	}
	
	// -- read the first line in the file to the buffer -- //
        
        buffer = ReadLine (project_file, 0) ;
        
        DBG_FIO (fprintf (stderr, "Retrieved buffer |%s| from file\n", buffer)) ;
	
	// -- check whether that was the end of the file -- //
	if(feof(project_file)){
		printf("Premature end of file reached your properties may not have been loaded\n");
		printf("End of file reached after reading in the first line of the file\n");
		return 0;
		}
	
	// -- find the design [DESIGN_OPTIONS] tag -- //	
	while(strcmp(buffer, "[DESIGN_PROPERTIES]") != 0){
		buffer = ReadLine (project_file, 0) ;
		
		if(feof(project_file)){
			printf("Premature end of file reached your design properties may not have been loaded\n");
			printf("End of file reached while searching for the opening [DESIGN_PROPERTIES] tag\n");
			return 0;
			}
	}
		
	// -- read in the first option --//
        buffer = ReadLine (project_file, 0) ;
		
	// -- keep reading in lines of the file until the end tag is reached -- //
	while(strcmp(buffer, "[#DESIGN_PROPERTIES]") != 0){
		
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
		
                buffer = ReadLine (project_file, 0) ;	
	}
	
	return total_number_of_cells;
	
}//read_properties


// ---------------------------------------------------------------------------------------- //

//!Gets the variable identifier from a text string
static char *get_identifier(char *buffer){
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
