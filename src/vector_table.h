#ifndef _VECTOR_TABLE_DATA_H_
#define _VECTOR_TABLE_DATA_H_

#include <glib.h>

#include "globals.h" /* for PATH_LENGTH and qcell */

typedef enum
  {
  VTL_FILE_FAILED  = -3,
  VTL_MAGIC_FAILED = -2,
  VTL_SHORT        = -1,
  VTL_OK           =  0,
  VTL_TRUNC        =  1
  } VTL_RESULT ;

typedef struct
  {
  char szFName[PATH_LENGTH] ;
  
  int num_of_vectors ;
  gboolean **vectors ;

  int num_of_inputs ;
  qcell **inputs ;
  gboolean *active_flag ;
  } VectorTable ;

VectorTable *VectorTable_new () ;
VectorTable *VectorTable_clear (VectorTable *pvt) ;
VectorTable *VectorTable_copy (VectorTable *pvt) ;
void VectorTable_fill (VectorTable *pvt, qcell *first_cell) ;
void VectorTable_add_input (VectorTable *pvt, qcell *new_input) ;
void VectorTable_del_input (VectorTable *pvt, qcell *old_input) ;
void VectorTable_add_inputs (VectorTable *pvt, qcell *first_cell) ;
int VectorTable_add_vector (VectorTable *pvt, int idx) ;
void VectorTable_del_vector (VectorTable *pvt, int idx) ;
gboolean VectorTable_save (VectorTable *pvt) ;
VTL_RESULT VectorTable_load (VectorTable *pvt) ;
void VectorTable_dump (VectorTable *pvt, FILE *pfile) ;

#endif /* _VECTOR_TABLE_DATA_H_ */
