#ifndef _QCELL_H_
#define _QCELL_H_

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DEVICE TYPEDEFS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// -- Quantum Dot Structure used in the qcell structure -- //
typedef struct {
  
  // absolute world qdot coords //
  double x;
  double y;
  
  // qdot diameter //
  double diameter;
  
  // qdot charge //
  double charge;
  
  // quantum spin of charge within dot //
  float spin;

  /* electrostatic potential induced by all other cells on
     this dot. */
  double potential;

} qdot;


// standard qcell type //
typedef struct qcell {

// Cell Model

	void *cell_model;

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

// cell orientation
     int orientation;
  
// all the dots within this cell  //
    qdot *cell_dots;
    int number_of_dots;

  //ENGINE SPECIFIC (NEXT 3) -- Leave until I grab these for the 
  // full physical model. --TD
  /* distance between the centers of two closest quantum dots */
  //double intradot_distance;
  /* Number of dots with spin up/down */
  //int num_dots_spin_up;
  //int num_dots_spin_down;
     
// current cell color //
    int color;

// the clock that this cell is linked to //
    int clock;

  /* ENGINE SPECIFIC??*/
// response shift is used in the random fault simulation //
	//double response_shift;

// cell type flags //
    gint is_input;
    gint is_output;
    gint is_fixed;

// cell label used to store input name or output name //
    char *label;

// pointers to the previous and next cell //
// needed since all the cells form a doubly linked list //      
    struct qcell *previous;
    struct qcell *next;
	
} qcell;

#endif /* _QCELL_H_ */
