//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// **** Please use complete names in variables and      //
// **** functions. This will reduce ramp up time for new//
// **** people trying to contribute to the project.     //
//////////////////////////////////////////////////////////

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <gtk/gtk.h>
#include "globals.h"

// -- Prototypes -- //
gboolean create_file(gchar * file_name);
gboolean open_project_file(gchar *file_name);
void export_block(gchar *file_name);
int import_block(gchar *file_name);
char *base_name (char *pszFile) ;

void create_vector_file();
void open_vector_file();

///////////////////////////////////////////////////////////////////////////////
// These next two structures were used in the first version of QCADesigner  ///
// I have added them here so that new versions can open the saved designs   ///
// old files where saved by fwriting the structures to the file             ///
// New files are saved as text to ensure cross platform compatibility       ///
///////////////////////////////////////////////////////////////////////////////

// -- Quantum Dot Structure used in the qcell structure -- //
typedef struct{

	// absolute world qdot coords //
	double x;
	double y;
	
	// qdot diameter //
	double diameter;
	
	// qdot charge //
	double charge;
	
	// quantum spin of charge within dot //
	float spin;

}version1_qdot;


// standard qcell type //
typedef struct version1_qcell{

// center coords //
	double x;
	double y;

// corner coords //
	double top_x;
	double top_y;
	double bot_x;
	double bot_y;
	
// -- cell physical parameters -- //
	double cell_width;
	double cell_height;
			
// all the dots within this cell  //
	version1_qdot *cell_dots;
	int number_of_dots;
	
// current cell color //
	int color;
	
// the clock that this cell is linked to //
	int clock;
	
// simulation parameter are only filled in during a simulation //
	int number_of_neighbours;
	struct version1_qcell **neighbours; // array of pointers to the neighbour cells
	double *Ek;  // the associated Ek values for those neighbours
		
// cell type flags //
	gint is_input;
	gint is_output;
	gint is_fixed;

// cell label used to store input name or output name //
	char *label;
	
// pointers to the previous and next cell //
// needed since all the cells form a doubly linked list //	
	struct version1_qcell *previous;
	struct version1_qcell *next;
	
}version1_qcell;

/////////////////////////////////////////////////////////////////////////////////////
#endif /* _FILEIO_H_ */
