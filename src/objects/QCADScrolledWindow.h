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
// Header for a tree view container that allows "freeze //
// columns". That is, the horizontal scrolling does not //
// scroll the entire tree view but, instead, it hides   //
// and shows columns as appropriate, keeping the first  //
// n columns always visible.                            //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _OBJECTS_QCADScrolledWindow_H_
#define _OBJECTS_QCADScrolledWindow_H_

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADScrolledWindowClass QCADScrolledWindowClass ;
typedef struct _QCADScrolledWindow      QCADScrolledWindow ;

struct _QCADScrolledWindowClass
  {
  GtkScrolledWindowClass parent_klass ;
  } ;

struct _QCADScrolledWindow
  {
  GtkScrolledWindow parent_instance ;
  GtkAdjustment *fake_hadj ;
  GtkAdjustment *fake_vadj ;
  gboolean bCustomHScroll ;
  gboolean bCustomVScroll ;
  } ;

GType qcad_scrolled_window_get_type () ;

GtkWidget *qcad_scrolled_window_new () ;

#define QCAD_TYPE_STRING_SCROLLED_WINDOW "QCADScrolledWindow"
#define QCAD_TYPE_SCROLLED_WINDOW (qcad_scrolled_window_get_type ())
#define QCAD_SCROLLED_WINDOW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_SCROLLED_WINDOW, QCADScrolledWindow))
#define QCAD_IS_SCROLLED_WINDOW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_SCROLLED_WINDOW))
#define QCAD_SCROLLED_WINDOW_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_SCROLLED_WINDOW, QCADScrolledWindowClass))
#define QCAD_SCROLLED_WINDOW_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_SCROLLED_WINDOW, QCADScrolledwindowClass))
#define QCAD_IS_SCROLLED_WINDOW_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((klass),  QCAD_TYPE_SCROLLED_WINDOW))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* def _OBJECTS_QCADTreeViewContainer_H_ */
