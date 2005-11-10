//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Header for the QCA cell.                             //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADRectangleClockingZone_H_
#define _OBJECTS_QCADRectangleClockingZone_H_

#include <glib-object.h>
#include "../gdk_structs.h"
#include "../exp_array.h"
#include "QCADClockingZone.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
  {
  QCADClockingZone parent_instance ;
  double angle ;
  int n_x_divisions ;
  int n_y_divisions ;
  } QCADRectangleClockingZone ;

typedef struct
  {
  /* public */
  QCADClockingZoneClass parent_class ;
  double default_angle ;
  int default_n_x_divisions ;
  int default_n_y_divisions ;
  } QCADRectangleClockingZoneClass ;

GType qcad_rectangle_clocking_zone_get_type () ;

#define QCAD_TYPE_STRING_RECTANGLE_CLOCKING_ZONE "QCADRectangleClockingZone"
#define QCAD_TYPE_RECTANGLE_CLOCKING_ZONE (qcad_rectangle_clocking_zone_get_type ())
#define QCAD_RECTANGLE_CLOCKING_ZONE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_RECTANGLE_CLOCKING_ZONE, QCADRectangleClockingZone))
#define QCAD_RECTANGLE_CLOCKING_ZONE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_RECTANGLE_CLOCKING_ZONE, QCADRectangleClockingZoneClass))
#define QCAD_IS_RECTANGLE_CLOCKING_ZONE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_RECTANGLE_CLOCKING_ZONE))
#define QCAD_IS_RECTANGLE_CLOCKING_ZONE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_RECTANGLE_CLOCKING_ZONE))
#define QCAD_RECTANGLE_CLOCKING_ZONE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_RECTANGLE_CLOCKING_ZONE, QCADRectangleClockingZoneClass))

QCADDesignObject *qcad_rectangle_clocking_zone_new (double (*clock_function) (double), double amplitude, double frequency, double phase, double min_clock, double max_clock, double dc_offset, double angle) ;

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADClockingZone_H_ */
