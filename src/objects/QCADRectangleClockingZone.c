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
// The QCA cell.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif

#include "objects_debug.h"

#include "../generic_utils.h"
#include "../support.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "QCADRectangleClockingZone.h"
#include "mouse_handlers.h"

#ifdef GTK_GUI
typedef struct
  {
  GtkWidget *tbl ;
  GtkWidget *clock_function_option_menu ;
  GtkAdjustment *adjAmplitude ;
  GtkAdjustment *adjFrequency ;
  GtkAdjustment *adjPhase ;
  GtkAdjustment *adjMinClock ;
  GtkAdjustment *adjMaxClock ;
  GtkAdjustment *adjDCOffset ;
  GtkAdjustment *adjAngle ;
  GtkAdjustment *adjNXDivisions ;
  GtkAdjustment *adjNYDivisions ;
  GtkAdjustment *adjCX ;
  GtkAdjustment *adjCY ;
  } DEFAULT_PROPERTIES ;

typedef struct
  {
  GtkWidget *dlg ;
  } PROPERTIES ;
#endif /* def GTK_GUI */

#ifdef DESIGNER
extern DropFunction drop_function ;
#endif /* def DESIGNER */

static struct { ClockFunction clock_function ; char *pszDescription ; } clock_functions[1]  =
  {
  {sin, "sin"}
  } ;

int n_clock_functions = G_N_ELEMENTS (clock_functions) ;

static void qcad_rectangle_clocking_zone_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_rectangle_clocking_zone_instance_init (GObject *object, gpointer data) ;
static void qcad_rectangle_clocking_zone_instance_finalize (GObject *object) ;

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static GCallback default_properties_ui (QCADDesignObjectClass *klass, void *default_properties, GtkWidget **pTopContainer, gpointer *pData) ;
#endif /* def GTK_GUI */
static void *default_properties_get (struct QCADDesignObjectClass *klass) ;
static void default_properties_set (struct QCADDesignObjectClass *klass, void *props) ;
static void default_properties_destroy (struct QCADDesignObjectClass *klass, void *props) ;

#ifdef GTK_GUI
static void create_default_properties_dialog (DEFAULT_PROPERTIES *dialog) ;
static void create_properties_dialog (PROPERTIES *dialog) ;
static void default_properties_apply (gpointer data) ;
#endif

GType qcad_rectangle_clocking_zone_get_type ()
  {
  static GType qcad_rectangle_clocking_zone_type = 0 ;

  if (!qcad_rectangle_clocking_zone_type)
    {
    static const GTypeInfo qcad_rectangle_clocking_zone_info =
      {
      sizeof (QCADRectangleClockingZoneClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_rectangle_clocking_zone_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADRectangleClockingZone),
      0,
      (GInstanceInitFunc)qcad_rectangle_clocking_zone_instance_init
      } ;

    if ((qcad_rectangle_clocking_zone_type = g_type_register_static (QCAD_TYPE_CLOCKING_ZONE, QCAD_TYPE_STRING_RECTANGLE_CLOCKING_ZONE, &qcad_rectangle_clocking_zone_info, 0)))
      g_type_class_ref (qcad_rectangle_clocking_zone_type) ;
    DBG_OO (fprintf (stderr, "Registered QCADClockingZone as %d\n", qcad_cell_type)) ;
    }
  return qcad_rectangle_clocking_zone_type ;
  }

static void qcad_rectangle_clocking_zone_class_init (GObjectClass *klass, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADClockingZone::class_init:Leaving\n")) ;
  G_OBJECT_CLASS (klass)->finalize = qcad_rectangle_clocking_zone_instance_finalize ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                       = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_ui      = default_properties_ui ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed          = (GCallback)button_pressed ;
#endif /* def GTK_GUI */
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_get     = default_properties_get ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_set     = default_properties_set ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_destroy = default_properties_destroy ;

  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_angle = 0.0 ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_x_divisions = 24 ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_y_divisions = 80 ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cxWorld =  6.0 ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cyWorld = 20.0 ;
  }

static void qcad_rectangle_clocking_zone_instance_init (GObject *object, gpointer data)
  {
  QCADRectangleClockingZone *rc_clocking_zone = QCAD_RECTANGLE_CLOCKING_ZONE (object) ;
  QCADRectangleClockingZoneClass *klass = QCAD_RECTANGLE_CLOCKING_ZONE_GET_CLASS (object) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Entering\n")) ;

  rc_clocking_zone->angle         = klass->default_angle ;
  rc_clocking_zone->n_x_divisions = klass->default_n_x_divisions ;
  rc_clocking_zone->n_y_divisions = klass->default_n_y_divisions ;
  QCAD_DESIGN_OBJECT (object)->bounding_box.cxWorld = klass->default_cxWorld ;
  QCAD_DESIGN_OBJECT (object)->bounding_box.cyWorld = klass->default_cyWorld ;

  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Leaving\n")) ;
  }

static void qcad_rectangle_clocking_zone_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Entering\n")) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADDesignObject *qcad_rectangle_clocking_zone_new ()
  {return QCAD_DESIGN_OBJECT (g_object_new (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE, NULL)) ;}

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static GCallback default_properties_ui (QCADDesignObjectClass *klass, void *default_properties, GtkWidget **pTopContainer, gpointer *pData)
  {
  int Nix ;
  QCADClockingZoneClass *clocking_zone_class = QCAD_CLOCKING_ZONE_CLASS (klass) ;
  QCADRectangleClockingZoneClass *rc_clocking_zone_class = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass) ;
  static DEFAULT_PROPERTIES dialog = {NULL} ;
  QCADRectangleClockingZoneOptions rcz_options ;

  if (NULL == dialog.tbl)
    create_default_properties_dialog (&dialog) ;

  if (NULL == default_properties)
    {
    memcpy (&rcz_options, &(clocking_zone_class->default_clocking_zone_options), sizeof (QCADClockingZoneOptions)) ;
    rcz_options.angle         = rc_clocking_zone_class->default_angle ;
    rcz_options.n_x_divisions = rc_clocking_zone_class->default_n_x_divisions ;
    rcz_options.n_y_divisions = rc_clocking_zone_class->default_n_y_divisions ;
    rcz_options.cxWorld       = rc_clocking_zone_class->default_cxWorld ;
    rcz_options.cyWorld       = rc_clocking_zone_class->default_cyWorld ;
    }
  else
    memcpy (&rcz_options, default_properties, sizeof (QCADRectangleClockingZoneOptions)) ;

  for (Nix = 0 ; Nix < n_clock_functions ; Nix++)
    if (clock_functions[Nix].clock_function == rcz_options.clocking_zone_options.clock_function) break ;
  if (Nix == n_clock_functions) Nix = -1 ;

  gtk_option_menu_set_history (GTK_OPTION_MENU (dialog.clock_function_option_menu), Nix) ;
  gtk_adjustment_set_value_infinite (dialog.adjAmplitude,   rcz_options.clocking_zone_options.amplitude) ;
  gtk_adjustment_set_value_infinite (dialog.adjFrequency,   rcz_options.clocking_zone_options.frequency / 1000000.0) ;
  gtk_adjustment_set_value_infinite (dialog.adjDCOffset,    rcz_options.clocking_zone_options.dc_offset) ;
  gtk_adjustment_set_value_infinite (dialog.adjMinClock,    rcz_options.clocking_zone_options.min_clock) ;
  gtk_adjustment_set_value_infinite (dialog.adjMaxClock,    rcz_options.clocking_zone_options.max_clock) ;
  gtk_adjustment_set_value_infinite (dialog.adjNXDivisions, rcz_options.n_x_divisions) ;
  gtk_adjustment_set_value_infinite (dialog.adjNYDivisions, rcz_options.n_y_divisions) ;
  gtk_adjustment_set_value_infinite (dialog.adjCX,          rcz_options.cxWorld) ;
  gtk_adjustment_set_value_infinite (dialog.adjCY,          rcz_options.cyWorld) ;
  gtk_adjustment_set_value (dialog.adjPhase,       (rcz_options.clocking_zone_options.phase * 180.0) / PI) ;
  gtk_adjustment_set_value (dialog.adjAngle,       (rcz_options.angle * 180.0) / PI) ;

  (*pTopContainer) = dialog.tbl ;
  (*pData) = &dialog ;
  return (GCallback)default_properties_apply ;
  }

static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  GdkGC *gc = NULL ;
  GdkRectangle rcReal ;

  world_to_real_rect (&(obj->bounding_box), &rcReal) ;

  if (!RECT_INTERSECT_RECT (rcReal.x, rcReal.y, rcReal.width, rcReal.height, rcClip->x, rcClip->y, rcClip->width, rcClip->height))
    return ;

  gc = gdk_gc_new (dst) ;
  gdk_gc_set_foreground (gc, &(obj->clr)) ;
  gdk_gc_set_background (gc, &(obj->clr)) ;
  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_clip_rectangle (gc, rcClip) ;

  gdk_draw_rectangle (dst, gc, FALSE, rcReal.x, rcReal.y, rcReal.width, rcReal.height) ;

  g_object_unref (gc) ;
  }

static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADDesignObject *obj = NULL ;
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;

  if (1 != event->button) return FALSE ;

#ifdef DESIGNER
  world_to_grid_pt (&xWorld, &yWorld) ;
#endif /* def DESIGNER */

  obj = qcad_rectangle_clocking_zone_new () ;
  obj->bounding_box.xWorld = xWorld - obj->bounding_box.cxWorld / 2.0 ;
  obj->bounding_box.yWorld = yWorld - obj->bounding_box.cyWorld / 2.0 ;

#ifdef DESIGNER
  if (NULL != drop_function)
    if ((*drop_function) (obj))
      return FALSE ;
#endif /* def DESIGNER */

  g_object_unref (obj) ;

  return FALSE ;
  }
#endif /* def GTK_GUI */

static void *default_properties_get (struct QCADDesignObjectClass *klass)
  {
  QCADClockingZoneOptions *pDefaults = g_malloc (sizeof (QCADRectangleClockingZoneOptions)) ;
  QCADRectangleClockingZoneOptions *p_rcz_Defaults = (QCADRectangleClockingZoneOptions *)pDefaults ;

  memcpy (pDefaults, &(QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options), sizeof (QCADClockingZoneOptions)) ;
  p_rcz_Defaults->angle         = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_angle ;
  p_rcz_Defaults->n_x_divisions = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_x_divisions ;
  p_rcz_Defaults->n_y_divisions = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_y_divisions ;
  p_rcz_Defaults->cxWorld       = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cxWorld ;
  p_rcz_Defaults->cyWorld       = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cyWorld ;
  return (void *)pDefaults ;
  }

static void default_properties_set (struct QCADDesignObjectClass *klass, void *props)
  {
  memcpy (&(QCAD_CLOCKING_ZONE_CLASS (klass)->default_clocking_zone_options), props, sizeof (QCADClockingZoneOptions)) ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_angle = ((QCADRectangleClockingZoneOptions *)props)->angle ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_x_divisions = ((QCADRectangleClockingZoneOptions *)props)->n_x_divisions ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_n_y_divisions = ((QCADRectangleClockingZoneOptions *)props)->n_y_divisions ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cxWorld = ((QCADRectangleClockingZoneOptions *)props)->cxWorld ;
  QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (klass)->default_cyWorld = ((QCADRectangleClockingZoneOptions *)props)->cyWorld ;
  }

static void default_properties_destroy (struct QCADDesignObjectClass *klass, void *props)
  {g_free (props) ;}

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static void create_default_properties_dialog (DEFAULT_PROPERTIES *dialog)
  {
  int Nix ;
  GtkWidget *lbl = NULL, *mnu = NULL, *mnui = NULL, *spn = NULL ;

  dialog->tbl = gtk_table_new (7, 3, FALSE) ;
  gtk_widget_show (dialog->tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->tbl), 2) ;

  lbl = gtk_label_new (_("Clock Function:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->clock_function_option_menu = gtk_option_menu_new () ;
  gtk_widget_show (dialog->clock_function_option_menu) ;
  mnu = gtk_menu_new () ;
  gtk_widget_show (mnu) ;
  for (Nix = 0 ; Nix < n_clock_functions ; Nix++)
    {
    mnui = gtk_menu_item_new_with_label (clock_functions[Nix].pszDescription) ;
    gtk_widget_show (mnui) ;
    gtk_container_add (GTK_CONTAINER (mnu), mnui) ;
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (dialog->clock_function_option_menu), mnu) ;
  gtk_option_menu_set_history (GTK_OPTION_MENU (dialog->clock_function_option_menu), 0) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), dialog->clock_function_option_menu, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("Amplitude:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjAmplitude = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 0.0001, 0.001, 0)), 0.0001, 4, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("Frequency:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjFrequency = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 0.1, 1, 0)), 0.1, 4, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("MHz")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 2, 3, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Phase:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new (dialog->adjPhase = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 360, 1, 5, 0)), 1, 0) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 3, 4, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("°")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 3, 4, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("DC Offset:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjDCOffset = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 0.1, 1, 0)), 0.1, 3, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 4, 5, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("V")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 4, 5, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Minimum voltage:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 5, 6, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjMinClock = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 0.1, 1, 0)), 0.1, 3, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 5, 6, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("V")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 5, 6, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Maximum voltage:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 6, 7, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjMaxClock = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 0.1, 1, 0)), 0.1, 3, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 6, 7, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("V")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 6, 7, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Angle:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 7, 8, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new (dialog->adjAngle = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 360, 1, 5, 0)), 1, 0) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 7, 8, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("°")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 7, 8, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Number of x divisions:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 8, 9, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjNXDivisions = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 2, 1, 1, 0)), 1, 0, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 8, 9, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("Number of y divisions:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 9, 10, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjNYDivisions = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 2, 1, 1, 0)), 1, 0, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 9, 10, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("Width:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 10, 11, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjCX = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 10, 0)), 1, 1, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 10, 11, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("nm")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 10, 11, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("Height:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 11, 12, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjCY = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 2, 1, 10, 0)), 1, 1, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), spn, 1, 2, 11, 12, GTK_FILL, GTK_FILL, 2, 2) ;

  lbl = gtk_label_new (_("nm")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 11, 12, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;
  }

static void create_properties_dialog (PROPERTIES *dialog)
  {
  }

static void default_properties_apply (gpointer data)
  {
  DEFAULT_PROPERTIES *dialog = (DEFAULT_PROPERTIES *)data ;
  QCADClockingZoneClass *clocking_zone_class = QCAD_CLOCKING_ZONE_CLASS (g_type_class_peek (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE)) ;
  QCADRectangleClockingZoneClass *rc_clocking_zone_class = QCAD_RECTANGLE_CLOCKING_ZONE_CLASS (clocking_zone_class) ;

  clocking_zone_class->default_clocking_zone_options.clock_function = clock_functions[gtk_option_menu_get_history (GTK_OPTION_MENU (dialog->clock_function_option_menu))].clock_function ;
  clocking_zone_class->default_clocking_zone_options.amplitude      = gtk_adjustment_get_value (dialog->adjAmplitude) ;
  clocking_zone_class->default_clocking_zone_options.frequency      = gtk_adjustment_get_value (dialog->adjFrequency) * 1000000.0 ;
  clocking_zone_class->default_clocking_zone_options.phase          = (gtk_adjustment_get_value (dialog->adjPhase) * PI) / 180.0 ;
  clocking_zone_class->default_clocking_zone_options.dc_offset      = gtk_adjustment_get_value (dialog->adjDCOffset) ;
  clocking_zone_class->default_clocking_zone_options.min_clock      = gtk_adjustment_get_value (dialog->adjMinClock) ;
  clocking_zone_class->default_clocking_zone_options.max_clock      = gtk_adjustment_get_value (dialog->adjMaxClock) ;
  rc_clocking_zone_class->default_angle         = gtk_adjustment_get_value (dialog->adjAngle) ;
  rc_clocking_zone_class->default_n_x_divisions = (int)gtk_adjustment_get_value (dialog->adjNXDivisions) ;
  rc_clocking_zone_class->default_n_y_divisions = (int)gtk_adjustment_get_value (dialog->adjNYDivisions) ;
  rc_clocking_zone_class->default_cxWorld       = gtk_adjustment_get_value (dialog->adjCX) ;
  rc_clocking_zone_class->default_cyWorld       = gtk_adjustment_get_value (dialog->adjCY) ;
  }
#endif
