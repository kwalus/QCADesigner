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

#ifndef _OBJECTS_QCADElectrode_H_
#define _OBJECTS_QCADElectrode_H_

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
  double relative_permittivity ;
  double z_to_ground ; // nm
  } QCADElectrodeOptions ;

typedef struct
  {
  double permittivity ; // epsilon_nought * epsilon_r
  double two_z_to_ground ; // nm
  double capacitance ; // Farads
  } QCADElectrodePrecompute ;

typedef struct
  {
  double min ;
  double max ;
  } EXTREME_POTENTIALS ;

typedef struct
  {
  QCADDesignObject parent_instance ;
  QCADElectrodeOptions electrode_options ;
  QCADElectrodePrecompute precompute_params ;
  } QCADElectrode ;

typedef struct
  {
  /* public */
  QCADDesignObjectClass parent_class ;
  QCADElectrodeOptions default_electrode_options ;

  double (*get_potential) (QCADElectrode *electrode, double x, double y, double z, double t) ;
  double (*get_voltage) (QCADElectrode *electrode, double t) ;
  double (*get_area) (QCADElectrode *electrode) ;
  EXTREME_POTENTIALS (*extreme_potential) (QCADElectrode *electrode, double z) ;
  void (*precompute) (QCADElectrode *electrode) ;
  } QCADElectrodeClass ;

GType qcad_electrode_get_type () ;

double qcad_electrode_get_potential (QCADElectrode *electrode, double x, double y, double z, double t) ;
double qcad_electrode_get_voltage (QCADElectrode *electrode, double t) ;
double qcad_electrode_get_area (QCADElectrode *electrode) ;
EXTREME_POTENTIALS qcad_electrode_get_extreme_potential (QCADElectrode *electrode, double z) ;

#define QCAD_TYPE_STRING_ELECTRODE "QCADElectrode"
#define QCAD_TYPE_ELECTRODE (qcad_electrode_get_type ())
#define QCAD_ELECTRODE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_ELECTRODE, QCADElectrode))
#define QCAD_ELECTRODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), QCAD_TYPE_ELECTRODE, QCADElectrodeClass))
#define QCAD_IS_ELECTRODE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_ELECTRODE))
#define QCAD_IS_ELECTRODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), QCAD_TYPE_ELECTRODE))
#define QCAD_ELECTRODE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), QCAD_TYPE_ELECTRODE, QCADElectrodeClass))

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif /* _OBJECTS_QCADElectrode_H_ */
