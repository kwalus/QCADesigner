#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACK_HELPERS_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACK_HELPERS_H_

#include "vector_table_options_dialog_interface.h"

#define IDX_VECTOR_KEY "idxVector"

enum
  {
  VECTOR_TABLE_MODEL_COLUMN_IDX_DFS = BUS_LAYOUT_MODEL_COLUMN_LAST,
  VECTOR_TABLE_MODEL_COLUMN_LAST
  } ;

void vtod_reflect_number_of_vectors_changed (vector_table_options_D *dialog, VECTOR_TABLE_OPTIONS_DIALOG_DATA *dialog_data) ;

#endif /* _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACK_HELPERS_H_ */
