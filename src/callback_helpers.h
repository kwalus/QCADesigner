#ifndef _CALLBACK_HELPERS_H_
#define _CALLBACK_HELPERS_H_

#include "gqcell.h"
#include "vector_table.h"

typedef struct
  {
  GdkColor clrCyan ;
  GdkColor clrWhite ;
  int selected_action ;
  int SIMULATION_ENGINE ;
  float max_response_shift ;
  float affected_cell_probability ;
  gboolean bDesignAltered ;
  int selected_cell_type ;
  gboolean SHOW_GRID ;
  gboolean SNAP_TO_GRID ;
  char *pszCurrentFName ;
  GQCell *window_move_selected_cell ;
  GQCell **selected_cells ;
  int number_of_selected_cells ;
  int SIMULATION_TYPE ;
  gboolean drop_new_cells ;
  VectorTable *pvt ;
  } project_OP ;

typedef struct
  {
  gulong lIDButtonPressed ;
  gulong lIDMotionNotify ;
  gulong lIDButtonReleased ;
  } MOUSE_HANDLERS ;

/* Actions requiring mouse handler registration */
#include "actions/action_handlers.h"

void gui_delete_cells (GQCell **ppqc, int ic) ;
void gui_create_cells (GQCell **ppqc, int ic) ;
void gui_add_to_undo (void *p) ;
void release_selection () ;
void propagate_motion_to_rulers (GtkWidget *widget, GdkEventMotion *event) ;
void setup_rulers () ;
void redraw_async (GtkWidget *widget) ;

#endif /* _CALLBACK_HELPERS_H_ */
