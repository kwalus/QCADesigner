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

#define MAX_UNDOS 10

undo_struct undo_stack[MAX_UNDOS_REDOS]={{NULL}};
int num_undo = 0;

typedef void (* undo_function)(void *undo_args) UNDO_FUNCTION ;

typedef struct{	
	void *undo_args;
	UNDO_FUNCTION fn;
}undo_struct;

typedef struct{
	
	qcell **moved_cells;
	double *x;
	double *y;

}move_args;

typedef struct{
	qcell **deleted_cells;
}delete_args;

// problems when pointers to cells are used becuse any steps that end in delete will 
// not be reversable. The first step would be a create and the cell might not be in the
// same memory location so all the undo steps before that would have dangling pointers.

void add_to_undo(UNDO_FUNCTION fn, void *undo_args){
		undo_stack[num_undo].fn = fn;
		undo_stack[num_undo].undo_args = undo_args;
//if the undo is about to wrap and the last function was a delete then the cells 
//must be actually deleted to avoid a memory leak.	
		num_undo++;
		num_undo%=MAX_UNDOS;	
}//add_to_undo

void undo(){
	(* (undo_stack[num_undo-1].fn))(undo_stack[num_undo-1].undo_args);
	num_undo--;
	if(num_undo<0)num_undo=MAX_UNDOS-1;
}//undo

void redo(){
	(* (undo_stack[num_undo-1].fn))(undo_stack[num_undo-1].undo_args);
	num_undo++;
	num_undo%=MAX_UNDOS;
}//redo
	
