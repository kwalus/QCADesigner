#ifndef _GLOBAL_CONSTS_H_
#define _GLOBAL_CONSTS_H_

#ifdef WIN32
  #include <windows.h>
#endif

///////////////////////////////////////////////////////////////////
///////////////////////// DEFINES /////////////////////////////////
///////////////////////////////////////////////////////////////////

//!File OPEN/SAVE actions used by file save dialog to determing whether to open or save
//!the argument file.
#define FILEOP_NO_ACTION 0
#define FILEOP_OPEN 1
#define FILEOP_SAVE_AS 2
#define FILEOP_EXPORT 3
#define FILEOP_IMPORT 4
#define FILEOP_OPEN_RECENT 5
#define FILEOP_NEW 6
#define FILEOP_SAVE 7
#define FILEOP_CLOSE 8
#define FILEOP_CMDLINE 9

#define PRINTED_OBJECTS_DIE     0
#define PRINTED_OBJECTS_CELLS   1
#define PRINTED_OBJECTS_COLOURS 2

//!Color defines
#define GREEN 0
#define GREEN1 5
#define GREEN2 6
#define GREEN3 7
#define RED 1
#define BLUE 2
#define YELLOW 3
#define WHITE 4
#define ORANGE 8
#define BLACK 9

//!Qcell Types
#define TYPE_USECELL 0
#define TYPE_1       1
#define TYPE_2       2

//!Simulation Engines
#define NONLINEAR_APPROXIMATION 0
#define BISTABLE 2
#define DIGITAL_SIM 3
#define SCQCA 4

//!Simulation Types
#define EXHAUSTIVE_VERIFICATION 0
#define VECTOR_TABLE 1

//!Simulation Algorithms
enum {CRANK_NICHOLSON,SPECTRAL_DECOMPOSITION,ADIABATIC};

//!Some usefull physical constants
#define QCHARGE 1.6021892e-19
#define HALF_QCHARGE 0.8e-19
#define EPSILON 8.8541878e-12
#define PI (double)(3.1415926535897932384626433832795)
#define HBAR 1.0545887e-34
#define PRECISION 1e-5

// Used by graphing window
#define BOUNDARY_RECT  15
#define BOUNDARY_GRAPH 20
#define DLG_HEIGHT 400
#define DLG_WIDTH 500

// -- menu choices -- //
enum
  {
  ACTION_DEFAULT,
  ACTION_DIAG_CELL,
  ACTION_DRAW_CELL_ARRAY,
  ACTION_MIRROR_CELLS,
  ACTION_ROTATE_CELL,
  ACTION_LAST_ACTION
  } ;

#define DEFAULT 0
#define HORIZ_CELL 1
#define DIAG_CELL 2
#define ZOOM_WINDOW 3
#define SELECT_CELL 4
#define MOVE_CELL 5
#define SELECT_CELL_AS_INPUT 6
#define SELECT_CELL_AS_OUTPUT 7
#define CHANGE_INPUT_PROPERTIES 8
#define DRAW_CELL_ARRAY 9
#define DELETE_CELL 10
#define MEASURE_DISTANCE 11
#define ROTATE_CELL 12
#define MIRROR_CELLS 13
#define CUSTOM_CELL 14
#define CELL_PROPERTIES 15
#define SELECT_CELL_AS_FIXED 16
#define CLEAR 17
#define PAN 18
#define COPY_CELL 19
#define CLOCKING_ZONE 20

// Maximum length of a file system path

#ifdef MAX_PATH
  // The Win32 headers have a similar variable already defined, so
  // let's use it
  #define PATH_LENGTH MAX_PATH
#else
  #define PATH_LENGTH 1024
#endif

// Font to use
#ifndef WIN32
  #define QCAD_GDKFONT "-adobe-courier-medium-r-normal--12-*-*-*-*-*-*"
#else
  #define QCAD_GDKFONT "Courier"
#endif

#endif /* _GLOBAL_CONSTS_H_ */
