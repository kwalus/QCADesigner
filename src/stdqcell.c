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


#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
// #include <sunperf.h>

//-------------------------------------------------------------------//

#include "globals.h"
#include "simulation.h"
#include "cad.h"
#include "stdqcell.h"

extern cell_OP cell_options ;

// -- Clears the entire design space -- //
void clear_all_cells()
{

    qcell *cell = first_cell;
    qcell *next = NULL;

    // -- free the memory from the cell list -- //
    while (cell != NULL) {

	next = cell->next;
	free_qcell(cell);
	cell = next;
    }

    first_cell = NULL;
    last_cell = NULL;

    // -- free up the selected cells -- //
    free(selected_cells);
    selected_cells = NULL;
    number_of_selected_cells = 0;

	
	
}//clear_all_cells

//-------------------------------------------------------------------//
// frees the memory of a qcell but it assumes it is not in the linked list //
void free_qcell(qcell * cell)
{

    assert(cell != NULL);
    free(cell->cell_dots);
    free(cell->label);
    free(cell);
    cell = NULL;

}				//free_qcell

//-------------------------------------------------------------------//

// -- copies one qcell to another -- //
void cell_copy(qcell * to_cell, qcell * from_cell)
{

    int i;

    // debug //
    assert(to_cell != NULL && from_cell != NULL);
    assert(to_cell != from_cell);

    // -- copy all the constants -- //

    to_cell->x = from_cell->x;
    to_cell->y = from_cell->y;
    to_cell->top_x = from_cell->top_x;
    to_cell->top_y = from_cell->top_y;
    to_cell->bot_x = from_cell->bot_x;
    to_cell->bot_y = from_cell->bot_y;
    to_cell->color = from_cell->color;
    to_cell->is_input = from_cell->is_input;
    to_cell->is_output = from_cell->is_output;
    to_cell->is_fixed = from_cell->is_fixed;
    to_cell->number_of_dots = from_cell->number_of_dots;
    to_cell->clock = from_cell->clock;


    // free up the name memory //
    free(to_cell->label);

    // allocate the appropriate amount of memory //
    to_cell->label = malloc(sizeof(char) * (strlen(from_cell->label) + 1));

    // check for allocation error //
    if (to_cell->label == NULL) {
		printf("#101 Memory allocation error in cell_copy()\n");
		exit(1);
    }
    // copy the label //
    strcpy(to_cell->label, from_cell->label);


    // copy physical cell params //
    to_cell->cell_width = from_cell->cell_width;
    to_cell->cell_height = from_cell->cell_height;


    // free up the memory from the old dots //              
    free(to_cell->cell_dots);

    // allocate the memory for the new dots //
    to_cell->cell_dots = malloc(sizeof(qdot) * from_cell->number_of_dots);
    to_cell->number_of_dots = from_cell->number_of_dots;

    // check for allocation error //
    if (to_cell->cell_dots == NULL) {
	printf("#103 Memory allocation error in cell_copy()\n");
	exit(1);
    }
    // copy the dots //
    for (i = 0; i < to_cell->number_of_dots; i++)
	copy_dot(&to_cell->cell_dots[i], &from_cell->cell_dots[i]);

}				//cell_copy

//-------------------------------------------------------------------//

// -- Copies Quantum dot properties -- //
void copy_dot(qdot * to_dot, qdot * from_dot)
{

    // debug //
    assert(to_dot != NULL && from_dot != NULL);
    assert(to_dot != from_dot);

    // copy all dot variables //
    to_dot->x = from_dot->x;
    to_dot->y = from_dot->y;
    to_dot->charge = from_dot->charge;
    to_dot->spin = from_dot->spin;
    to_dot->diameter = from_dot->diameter;

}				//copy_dot

//-------------------------------------------------------------------//

// rotates the qdots within a cell by a given angle //
void rotate_cell(qcell * cell, float angle)
{

    int i;
    double x;
    double y;

    assert(cell != NULL);

    // -- uses standard rotational transform -- //
    for (i = 0; i < cell->number_of_dots; i++) {

	assert((cell->cell_dots + i) != NULL);

	x =
	    cell->x + (cell->cell_dots[i].x -
		       cell->x) * (float) cos(angle) -
	    (cell->cell_dots[i].y - cell->y) * (float) sin(angle);
	y =
	    cell->y + (cell->cell_dots[i].y -
		       cell->y) * (float) cos(angle) +
	    (cell->cell_dots[i].x - cell->x) * (float) sin(angle);
	cell->cell_dots[i].x = x;
	cell->cell_dots[i].y = y;

    }

}				//rotate_cell

//-------------------------------------------------------------------//

// -- If a cell is made an input then this is used to set its input name -- //
void set_cell_label(qcell * cell, char *label)
{

    assert(cell != NULL);
    assert(label != NULL);

    // free up the old name //
    free(cell->label);

    // allocate memory for the new name //
    cell->label = malloc(sizeof(char) * (strlen(label) + 1));

    // check for allocation error //
    if (cell->label == NULL) {
	printf("Memory allocation error in set_cell_label()\n");
	exit(1);
    }
    // copy the text //
    strcpy(cell->label, label);
}				//set_cell_label


//-------------------------------------------------------------------//

// -- deletes a qcell from the array of cells, then moves all cells down to maintain a full array --//
void delete_stdqcell(qcell * cell)
{

    assert(cell != NULL);

    // if this is that last cell in the list //
    if (cell->next == NULL && cell->previous != NULL) {

	// make the previous cell the last in the list //
	cell->previous->next = NULL;
	last_cell = cell->previous;

	// if this is the first cell in the list //     
    } else if (cell->previous == NULL && cell->next != NULL) {

	// make the next cell the first in the list //
	cell->next->previous = NULL;
	first_cell = cell->next;

	// if this cell is somewhere in the middle of the list //
    } else if (cell->previous != NULL && cell->next != NULL) {

	cell->next->previous = cell->previous;
	cell->previous->next = cell->next;

	// the cell was the only one in the design //
    } else {

	first_cell = NULL;
	last_cell = NULL;
    }

    // free the memory from the cell //
    free_qcell(cell);


}				//delete_stdqcell

//-------------------------------------------------------------------//

// adds a qcell with parameters from the qcell argument //
qcell *add_stdqcell(qcell * cell, int check_overlap, int type){

    qcell *new_cell = NULL;

    assert(cell != NULL);

    // verify that no other cell is currently at this location //
    if (check_overlap){
		if(type == TYPE_1){
			if (select_cell_at_coords(cell->x - cell_options.type_1_cell_width/2, cell->y - cell_options.type_1_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x - cell_options.type_1_cell_width/2, cell->y + cell_options.type_1_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x + cell_options.type_1_cell_width/2, cell->y - cell_options.type_1_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x + cell_options.type_1_cell_width/2, cell->y + cell_options.type_1_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x, cell->y) != NULL) {
			
				return NULL;
			}
		}else if(type == TYPE_2){
			if (select_cell_at_coords(cell->x - cell_options.type_2_cell_width/2, cell->y - cell_options.type_2_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x - cell_options.type_2_cell_width/2, cell->y + cell_options.type_2_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x + cell_options.type_2_cell_width/2, cell->y - cell_options.type_2_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x + cell_options.type_2_cell_width/2, cell->y + cell_options.type_2_cell_height/2) != NULL
				|| select_cell_at_coords(cell->x, cell->y) != NULL) {
				
				return NULL;
			}
		}
	}//check_overlap
    
	// allocate memory for the new cell //
    new_cell = calloc(1, sizeof(qcell));

    	// check for allocation error //
    if (new_cell == NULL) {
		printf("Memory allocation error in add_stdqcell() could not allocate memory for new cell\n");
		exit(1);
	}//check_allocation

	new_cell->clock = cell_options.default_clock;
    new_cell->x = cell->x;
    new_cell->y = cell->y;
	
    // if a cell width or height has not been assigned use the defaults
    if (type == TYPE_1) {
		new_cell->cell_width = cell_options.type_1_cell_width;
    }else if(type == TYPE_2){
		new_cell->cell_width = cell_options.type_2_cell_width;
    }else{
		new_cell->cell_width = cell_options.type_1_cell_width;
		printf("incorrect type to stdqcell type=%d, setting to type 1\n", type);
	}

	if (type == TYPE_1) {
		new_cell->cell_height = cell_options.type_1_cell_height;
    }else if(type == TYPE_2){
		new_cell->cell_height = cell_options.type_2_cell_height;
    }else{
		new_cell->cell_height = cell_options.type_1_cell_height;
	}

	// the default cell will have 4 dots //
	new_cell->number_of_dots = 4;

	// make memory for the cell dots //
	new_cell->cell_dots = calloc(new_cell->number_of_dots, sizeof(qdot));

	// check for allocation error //
	if (new_cell->cell_dots == NULL) {
	    printf("Memory allocation error in add_stdqcell() could not allocate the memory for cell qdots\n");
	    exit(1);
	}
	
	// set all the dot diameters to default //      
	if (type == TYPE_1) {
		
		new_cell->cell_dots[0].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[1].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[2].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[3].diameter = cell_options.type_1_dot_diameter;
		
		new_cell->cell_dots[3].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[3].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[0].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[0].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[2].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[2].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[1].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[1].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
    
	}else if(type == TYPE_2){
		
		new_cell->cell_dots[0].diameter = cell_options.type_2_dot_diameter;
		new_cell->cell_dots[1].diameter = cell_options.type_2_dot_diameter;
		new_cell->cell_dots[2].diameter = cell_options.type_2_dot_diameter;
		new_cell->cell_dots[3].diameter = cell_options.type_2_dot_diameter;
    
		new_cell->cell_dots[3].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 - cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[3].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 - cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[0].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 + cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[0].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 - cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[2].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 - cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[2].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 + cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[1].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 + cell_options.type_2_dot_diameter / 2;
		new_cell->cell_dots[1].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_2_dot_diameter) / 2 + cell_options.type_2_dot_diameter / 2;
	
	}else{
		
		new_cell->cell_dots[0].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[1].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[2].diameter = cell_options.type_1_dot_diameter;
		new_cell->cell_dots[3].diameter = cell_options.type_1_dot_diameter;
		
		new_cell->cell_dots[3].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[3].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[0].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[0].y = new_cell->y - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[2].x = new_cell->x - (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 - cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[2].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[1].x = new_cell->x + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
		new_cell->cell_dots[1].y = new_cell->y + (new_cell->cell_width / 2 - cell_options.type_1_dot_diameter) / 2 + cell_options.type_1_dot_diameter / 2;
	}
	
	

	new_cell->cell_dots[0].charge = HALF_QCHARGE;
	new_cell->cell_dots[1].charge = HALF_QCHARGE;
	new_cell->cell_dots[2].charge = HALF_QCHARGE;
	new_cell->cell_dots[3].charge = HALF_QCHARGE;

    // set the top left and bottom right coords //
    // when searching for cells at a location these will speed up the algorithm //
    new_cell->top_x = new_cell->x - new_cell->cell_width / 2;
    new_cell->top_y = new_cell->y - new_cell->cell_height / 2;

    new_cell->bot_x = new_cell->x + new_cell->cell_width / 2;
    new_cell->bot_y = new_cell->y + new_cell->cell_height / 2;

    new_cell->is_fixed = FALSE;
    new_cell->is_input = FALSE;
    new_cell->is_output = FALSE;

    new_cell->label = calloc(10, sizeof(char));
    strcpy(new_cell->label, "NO NAME");

    new_cell->color = GREEN;

    // -- put the cell into the linked list -- //
    new_cell->next = NULL;
    new_cell->previous = last_cell;

    last_cell = new_cell;
    if (first_cell == NULL)
		first_cell = new_cell;

    if (new_cell->previous != NULL) {
		new_cell->previous->next = new_cell;
    }

    return new_cell;

}//add_stdqcell


// -- creates a linear array of cells from one point to another -- //
void create_array_of_cells(double x0, double y0, double x1, double y1)
{

    qcell cell;

    double x;
    double y;
    double temp;

    if (x0 > x1) {
	temp = x1;
	x1 = x0;
	x0 = temp;
    }

    if (y0 > y1) {
	temp = y1;
	y1 = y0;
	y0 = temp;
    }
    // if the user drew a horizantle line //
    if ((y0 - y1) == 0) {
		for (x = calc_world_x(x0); x <= calc_world_x(x1); x += grid_spacing) {
	
			if (SNAP_TO_GRID) {
				cell.x = grid_world_x(x);
				cell.y = grid_world_y(calc_world_y(y0));
			} else {
				cell.x = x;
				cell.y = calc_world_y(y0);
			}
	
			add_stdqcell(&cell, TRUE, selected_cell_type);
		}

	// if the user drew a vertical line //
    } else if ((x0 - x1) == 0) {
		for (y = calc_world_y(y0); y <= calc_world_y(y1); y += grid_spacing) {
			if (SNAP_TO_GRID) {
				cell.x = grid_world_x(calc_world_x(x0));
				cell.y = grid_world_y(y);
			} else {
				cell.x = calc_world_x(x0);
				cell.y = y;
			}
	
			add_stdqcell(&cell, TRUE, selected_cell_type);
		}
    }

}

//-------------------------------------------------------------------//
// -- Mirrors Cells About the given line -- //

void mirror_cells_about_line(int mirror_x0, int mirror_y0, int mirror_x1, int mirror_y1){

    qcell cell;
    int i = 0;

    mirror_x0 = calc_world_x(mirror_x0);
    mirror_x1 = calc_world_x(mirror_x1);
    mirror_y0 = calc_world_y(mirror_y0);
    mirror_y1 = calc_world_y(mirror_y1);

    // snap the line to grid if grid snap is on //
    if (SNAP_TO_GRID) {
		mirror_x0 = grid_world_x(mirror_x0);
		mirror_x1 = grid_world_x(mirror_x1);
		mirror_y0 = grid_world_y(mirror_y0);
		mirror_y1 = grid_world_y(mirror_y1);
    }
    //x' = x0 + 2(a-x0)

    // mirror horizantaly //
    if (mirror_x0 == mirror_x1) {

		// -- mirror each of the selected cells -- //
		for (i = 0; i < number_of_selected_cells; i++) {
	
			assert(selected_cells[i] != NULL);
	
			cell.x =
			selected_cells[i]->x + 2 * (mirror_x0 -
							selected_cells[i]->x);
			cell.y = selected_cells[i]->y;
	
			cell.cell_width = selected_cells[i]->cell_width;
			cell.cell_height = selected_cells[i]->cell_height;
			cell.number_of_dots = 0;
	
			add_stdqcell(&cell, TRUE, selected_cell_type);
		}

	// mirror vertically
    } else {
		// -- mirror each of the selected cells -- //
		for (i = 0; i < number_of_selected_cells; i++) {
	
			assert(selected_cells[i] != NULL);
	
			cell.y =
			selected_cells[i]->y + 2 * (mirror_y0 -
							selected_cells[i]->y);
			cell.x = selected_cells[i]->x;
	
			cell.cell_width = selected_cells[i]->cell_width;
			cell.cell_height = selected_cells[i]->cell_height;
			cell.number_of_dots = 0;
	
			add_stdqcell(&cell, TRUE, selected_cell_type);
		}
    }

    redraw_world();

}//mirror_cells_about_line

// -- moves a cell by a given offset -- //
void move_cell_by_offset(qcell * cell, float x_offset, float y_offset)
{

    int i;

    assert(cell != NULL);

    for (i = 0; i < cell->number_of_dots; i++) {
	cell->cell_dots[i].x += x_offset;
	cell->cell_dots[i].y += y_offset;
    }

    cell->x += x_offset;
    cell->y += y_offset;

    cell->top_x = cell->x - cell->cell_width / 2;
    cell->top_y = cell->y - cell->cell_height / 2;

    cell->bot_x = cell->x + cell->cell_width / 2;
    cell->bot_y = cell->y + cell->cell_height / 2;


}				//move_cell_by_offset


void move_cell_to_location(qcell * cell, float x, float y)
{

    int i = 0;

    assert(cell != NULL);

    // make sure that no other cells reside at these coords //
    if (select_cell_at_coords(x, y) != NULL
	&& select_cell_at_coords(x, y) != cell) {
	return;
    }

    for (i = 0; i < cell->number_of_dots; i++) {
	cell->cell_dots[i].x += x - cell->x;
	cell->cell_dots[i].y += y - cell->y;
    }

    cell->x = x;
    cell->y = y;

    cell->top_x = x - cell->cell_width / 2;
    cell->top_y = y - cell->cell_height / 2;

    cell->bot_x = x + cell->cell_width / 2;
    cell->bot_y = y + cell->cell_height / 2;


}				//move_cell_to_location

// sets a cell as an input cell //
void set_cell_as_input(qcell * cell)
{

    assert(cell != NULL);

    cell->is_output = FALSE;
    cell->is_fixed = FALSE;

    cell->is_input = TRUE;
    cell->color = BLUE;

}				//set_cell_as_input

// sets a cell as an output cell and gives it a unique output number //
void set_cell_as_output(qcell * cell)
{

    assert(cell != NULL);

    cell->is_input = FALSE;
    cell->is_fixed = FALSE;

    cell->is_output = TRUE;
    cell->color = YELLOW;

}				//set_cell_as_output

void set_cell_as_fixed(qcell * cell)
{

    assert(cell != NULL);

    cell->is_input = FALSE;
    cell->is_output = FALSE;

    cell->is_fixed = TRUE;
    cell->color = ORANGE;

}				//set_cell_as_fixed

int get_clock_from_selected_cells ()
  {
  int Nix ;
  int iClock = -1 ;
  
  for (Nix = 0 ; Nix < number_of_selected_cells ; Nix++)
    if (NULL != selected_cells[Nix])
      {
      iClock = selected_cells[Nix]->clock ;
      break ;
      }
  for (; Nix < number_of_selected_cells ; Nix++)
    if (NULL != selected_cells[Nix])
      {
      if (selected_cells[Nix]->clock != iClock)
      return -1 ;
      }
  return iClock ;
  }

void set_clock_for_selected_cells(int selected_clock)
{

    int i;

    assert(selected_clock < 4 && selected_clock >= 0);

    for (i = 0; i < number_of_selected_cells; i++) {

	assert(selected_cells[i] != NULL);

	selected_cells[i]->clock = selected_clock;
	switch (selected_clock) {
	case 0:
	    selected_cells[i]->color = GREEN;
	    break;
	case 1:
	    selected_cells[i]->color = GREEN1;
	    break;
	case 2:
	    selected_cells[i]->color = GREEN2;
	    break;
	case 3:
	    selected_cells[i]->color = GREEN3;
	    break;
	}
    }

}				//set_clock_for_selected_cells

//-------------------------------------------------------------------//

// calculates the polarization of a qcell //
double calculate_polarization (qcell * cell){

  assert (cell != NULL);
  assert (cell->number_of_dots == 4);

  return ((cell->cell_dots[0].charge + cell->cell_dots[2].charge) -
	  (cell->cell_dots[1].charge +
	   cell->cell_dots[3].charge)) / (4 * HALF_QCHARGE);
}				//calculate_polarization

//-------------------------------------------------------------------//

//!Sets the polarization of the given cell by setting the appropriate charges to each of the quantum-dots.
//!At this time only works with cells that have 4 QD's.
void set_cell_polarization (qcell * cell, double new_polarization){

  assert (cell != NULL);
  assert (cell->number_of_dots == 4);

  cell->cell_dots[0].charge = HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[2].charge = HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[1].charge = -1 * HALF_QCHARGE * new_polarization + HALF_QCHARGE;
  cell->cell_dots[3].charge = -1 * HALF_QCHARGE * new_polarization + HALF_QCHARGE;

}//set_cell_polarization

//-------------------------------------------------------------------//
