#ifndef _GQCELL_H_
#define _GQCELL_H_

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include "global_consts.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  int default_clock ;
  double type_1_cell_width ;
  double type_1_cell_height ;
  double type_1_dot_diameter ;
  double type_2_cell_width ;
  double type_2_cell_height ;
  double type_2_dot_diameter ;
  } cell_OP ;

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

// Standard qcell type
typedef struct GQCell
  {
  GObject parent_instance ;
  
  // Cell Model
  void *cell_model ;
  
  // center coords //
  double x ;
  double y ;
  
  // corner coords //
  double top_x ;
  double top_y ;
  double bot_x ;
  double bot_y ;
  
  // -- cell physical parameters -- //
  double cell_width ;
  double cell_height ;

  // cell orientation
  int orientation ;

  // all the dots within this cell  //
  qdot *cell_dots ;
  int number_of_dots ;
  
  // current cell colour //
  GdkColor *color ;

  // the clock that this cell is linked to //
  int clock ;

  // Is this cell selected ?
  gboolean bSelected ;

  // cell type flags //
  gint is_input ;
  gint is_output ;
  gint is_fixed ;

  // cell label used to store input name or output name //
  gchar *label ;

  // pointers to the previous and next cell //
  // needed since all the cells form a doubly linked list //      
  struct GQCell *next ;
  struct GQCell *prev ;
  } GQCell ;

// Stripped down version of the GQCell structure to be used as an undo history entry
typedef struct
  {
  // center coords //
  double x ;
  double y ;
  
  // corner coords //
  double top_x ;
  double top_y ;
  double bot_x ;
  double bot_y ;
  
  // -- cell physical parameters -- //
  double cell_width ;
  double cell_height ;

  // cell orientation
  int orientation ;

  // all the dots within this cell  //
  qdot *cell_dots ;
  int number_of_dots ;
  
  // current cell colour //
  GdkColor *color ;

  // the clock that this cell is linked to //
  int clock ;

  // cell type flags //
  gint is_input ;
  gint is_output ;
  gint is_fixed ;

  // cell label used to store input name or output name //
  gchar *label ;
  } qcell ;

typedef struct
  {
  GObjectClass parent_class ;
  } GQCellClass ;

GType gqcell_get_type () ;

GQCell *gqcell_new (GQCell *tmplt, double x, double y, cell_OP *pco, int type) ;
GQCell *gqcell_copy (GQCell *tmplt) ;
GQCell *gqcell_new_from_stream (FILE *stream) ;
void gqcell_link (GQCell *gqc, GQCell *prev, GQCell *next) ;
void gqcell_unlink (GQCell *gqc) ;
void gqcell_rotate (GQCell * cell, float angle) ;
void gqcell_set_label (GQCell * cell, char *label) ;
void gqcell_move_by_offset (GQCell *cell, float x_offset, float y_offset) ;
void gqcell_move_to_location (GQCell *cell, float x, float y) ;
void gqcell_set_as_normal (GQCell *cell) ;
void gqcell_set_as_input (GQCell *cell) ;
void gqcell_set_as_output (GQCell *cell) ;
void gqcell_set_as_fixed (GQCell *cell) ;
double gqcell_calculate_polarization (GQCell *cell) ;
void gqcell_set_polarization (GQCell *cell, double new_polarization) ;
void gqcell_serialize (GQCell *cell, FILE *stream) ;
gboolean gqcell_overlaps (GQCell *gqcL, GQCell *gqcR) ;
gboolean gqcell_point_in_cell (GQCell *gqc, double x, double y) ;
void gqcell_select (GQCell *gqc) ;
void gqcell_reset_colour (GQCell *gqc) ;
void gqcell_scale (GQCell *cell, double scale) ;

#define TYPE_GQCELL (gqcell_get_type())
#define GQCELL(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GQCELL, GQCell))
#define GQCELL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GQCELL, GQcellClass))
#define IS_GQCELL(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GQCELL))
#define IS_GQCELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GQCELL))
#define GQCELL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GQCELL, GQCellClass))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GQCELL_H_ */
