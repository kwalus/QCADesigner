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

#ifndef _OBJECTS_QCADClockingZone_H_
#define _OBJECTS_QCADClockingZone_H_

#include <glib-object.h>
#include "../gdk_structs.h"
#include "../exp_array.h"
#include "QCADDesignObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef double (*ClockFunction) (double) ;

typedef struct
  {
  ClockFunction clock_function ;
  double amplitude ;
  double frequency ; // Hz
  double phase ;     // radians
  double dc_offset ; // Volts
  double min_clock ; // Volts
  double max_clock ; // Volts

  double distance_to_draw ;
  } QCADClockingZoneOptions ;

typedef struct
  {
  QCADDesignObject parent_instance ;
  QCADClockingZoneOptions clocking_zone_options ;
  } QCADClockingZone ;

typedef struct
  {
  /* public */
  QCADDesignObjectClass parent_class ;
  QCADClockingZoneOptions default_clocking_zone_options ;
  } QCADClockingZoneClass ;

GType qcad_clocking_zone_get_type () ;

#define QCAD_TYPE_STRING_CLOCKING_ZONE "QCADClockingZone"
#define QCAD_TYPE_CLOCKING_ZONE (qcad_clocking_zone_get_type ())
#define QCAD_CLOCKING_ZONE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_CLOCKING_ZONE, QCADClockingZone))
#define QCAD_CLOCKING_ZONE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_CLOCKING_ZONE, QCADClockingZoneClass))
#define QCAD_IS_CLOCKING_ZONE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_CLOCKING_ZONE))
#define QCAD_IS_CLOCKING_ZONE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_CLOCKING_ZONE))
#define QCAD_CLOCKING_ZONE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_CLOCKING_ZONE, QCADClockingZoneClass))

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADClockingZone_H_ */
