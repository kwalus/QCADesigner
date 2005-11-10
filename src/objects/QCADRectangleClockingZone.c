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
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                  = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_ui = default_properties_ui ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed     = (GCallback)button_pressed ;
#endif /* def GTK_GUI */
  }

static void qcad_rectangle_clocking_zone_instance_init (GObject *object, gpointer data)
  {
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Entering\n")) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_init:Leaving\n")) ;
  }

static void qcad_rectangle_clocking_zone_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Entering\n")) ;
  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADClockingZone::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADDesignObject *qcad_rectangle_clocking_zone_new (double (*clock_function) (double), double amplitude, double frequency, double phase, double min_clock, double max_clock, double dc_offset, double angle)
  {
  QCADDesignObject *obj = g_object_new (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE, NULL) ;
  QCADClockingZone *clocking_zone = QCAD_CLOCKING_ZONE (obj) ;
  QCADRectangleClockingZone *rc_clocking_zone = QCAD_RECTANGLE_CLOCKING_ZONE (obj) ;

  clocking_zone->clocking_zone_options.clock_function = clock_function ;
  clocking_zone->clocking_zone_options.amplitude      = amplitude ;
  clocking_zone->clocking_zone_options.frequency      = frequency ;
  clocking_zone->clocking_zone_options.phase          = phase ;
  clocking_zone->clocking_zone_options.min_clock      = min_clock ;
  clocking_zone->clocking_zone_options.max_clock      = max_clock ;
  clocking_zone->clocking_zone_options.dc_offset      = dc_offset ;
  rc_clocking_zone->angle                             = angle ;

  obj->clr.red = (int)(clocking_zone->clocking_zone_options.phase * 65535.0) ;
  obj->clr.green = 0xFFFF ;
  obj->clr.blue = 0xC0C0 ;
  HSLToRGB (&(obj->clr)) ;
#ifdef GTK_GUI
  gdk_colormap_alloc_color (gdk_colormap_get_system (), &(obj->clr), FALSE, FALSE) ;
#endif /* def GTK_GUI */

  return obj ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static GCallback default_properties_ui (QCADDesignObjectClass *klass, void *default_properties, GtkWidget **pTopContainer, gpointer *pData)
  {
  QCADClockingZoneClass *clocking_zone_class = QCAD_CLOCKING_ZONE_CLASS (klass) ;
  static DEFAULT_PROPERTIES dialog = {NULL} ;
  QCADClockingZoneOptions *default_clocking_zone_options = 
    (NULL == default_properties) 
      ? &(clocking_zone_class->default_clocking_zone_options)
      : default_properties ;

  if (NULL == dialog.tbl)
    create_default_properties_dialog (&dialog) ;

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

  obj = qcad_rectangle_clocking_zone_new (sin, 1, 1, 0, -1, 1, 0, 0) ;
  obj->bounding_box.xWorld = xWorld - 0.5 ;
  obj->bounding_box.yWorld = yWorld - 2.5 ;
  obj->bounding_box.cxWorld = 1.0 ;
  obj->bounding_box.cyWorld = 5.0 ;

#ifdef DESIGNER
  if (NULL != drop_function)
    if ((*drop_function) (obj))
      return FALSE ;
#endif /* def DESIGNER */

  g_object_unref (obj) ;

  return FALSE ;
  }
#endif /* def GTK_GUI */

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

  lbl = gtk_label_new (_("degrees")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 2, 3, 3, 4, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 0.0, 0.5) ;

  lbl = gtk_label_new (_("DC Offset:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (dialog->tbl), lbl, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (dialog->adjFrequency = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 0.1, 1, 0)), 0.1, 3, ISB_DIR_UP | ISB_DIR_DN) ;
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
  }

static void create_properties_dialog (PROPERTIES *dialog)
  {
  }

static void default_properties_apply (gpointer data)
  {
  DEFAULT_PROPERTIES *dialog = (DEFAULT_PROPERTIES *)dialog ;
  QCADClockingZoneClass *clocking_zone_class = QCAD_CLOCKING_ZONE_CLASS (g_type_class_peek (QCAD_TYPE_RECTANGLE_CLOCKING_ZONE)) ;

  clocking_zone_class->default_clocking_zone_options.clock_function =
    clock_functions[gtk_option_menu_get_history (GTK_OPTION_MENU (dialog->clock_function_option_menu))].clock_function ;
  clocking_zone_class->default_clocking_zone_options.amplitude =
    gtk_adjustment_get_value (dialog->adjAmplitude) ;
  clocking_zone_class->default_clocking_zone_options.frequency =
    gtk_adjustment_get_value (dialog->adjFrequency) * 1000000.0 ;
  clocking_zone_class->default_clocking_zone_options.phase =
    (gtk_adjustment_get_value (dialog->adjPhase) * PI) / 180.0 ;
  clocking_zone_class->default_clocking_zone_options.dc_offset =
    gtk_adjustment_get_value (dialog->adjDCOffset) ;
  clocking_zone_class->default_clocking_zone_options.min_clock =
    gtk_adjustment_get_value (dialog->adjMinClock) ;
  clocking_zone_class->default_clocking_zone_options.max_clock =
    gtk_adjustment_get_value (dialog->adjMaxClock) ;
  }
#endif
