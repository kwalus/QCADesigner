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

#include "../generic_utils.h"
#include "../intl.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "QCADCell.h"
#include "QCADDOContainer.h"
//#ifdef GTK_GUI
//  #include "QCADClockCombo.h"
//#endif /* def GTK_GUI */
#include "mouse_handlers.h"
#include "QCADCompoundDO.h"
#include "QCADPropertyUI.h"
#include "object_helpers.h"
#include "../fileio_helpers.h"

/**
 * SECTION:QCADCell
 * @short_description: QCA Cell
 *
 * Implementation of the QCA cell. Every cell is assumed to have 4 dots, located either inside the cell's
 * corners, or inside the midpoints of the cell's edges. Cells can serve 4 different functions
 * (%QCAD_CELL_NORMAL, %QCAD_CELL_INPUT, %QCAD_CELL_OUTPUT, %QCAD_CELL_FIXED) and 3 different appearances
 * (%QCAD_CELL_MODE_NORMAL, %QCAD_CELL_MODE_CROSSOVER, %QCAD_CELL_MODE_VERTICAL).
 *
 */

#define QCAD_CELL_LABEL_DEFAULT_OFFSET_Y 1

#define DBG_VAL(s)

#ifdef DESIGNER
extern DropFunction drop_function ;
#endif /* def DESIGNER */

static void qcad_cell_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_cell_instance_init (GObject *object, gpointer data) ;
static void qcad_cell_instance_finalize (GObject *object) ;
static void qcad_cell_compound_do_interface_init (gpointer iface, gpointer interface_data) ;
//static void qcad_cell_do_container_interface_init (gpointer iface, gpointer interface_data) ;
static void qcad_cell_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void qcad_cell_get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
#endif /* def GTK_GUI */

static void copy (QCADObject *src, QCADObject *dst) ;
static QCADObject *class_get_default_object () ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
static void move (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static GList *add_unique_types (QCADDesignObject *obj, GList *lst) ;
static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected) ;
static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rc) ;
static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal) ;
static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22) ;
static QCADDesignObject *qcad_cell_compound_do_first (QCADCompoundDO *cdo) ;
static QCADDesignObject *qcad_cell_compound_do_next (QCADCompoundDO *cdo) ;
static gboolean qcad_cell_compound_do_last (QCADCompoundDO *cdo) ;
//static gboolean qcad_cell_do_container_add (QCADDOContainer *container, QCADDesignObject *obj) ;
//static gboolean qcad_cell_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj) ;

#ifdef DESIGNER
static void qcad_cell_array_next_coord (int idx[2], double coord[2], double length[2], double dDir) ;
#endif /* def DESIGNER */
#ifdef STDIO_FILEIO
static gboolean qcad_cell_dot_unserialize (FILE *fp, QCADCellDot *pdots, int idxDot) ;
#endif
static void qcad_cell_apply_transformation (QCADCell *cell, double xOld, double yOld) ;
static void qcad_cell_calculate_coords (QCADCell *cell) ;
static void qcad_cell_set_size (QCADCell *cell, double cx, double cy, double dot_diam) ;

GdkColor clrBlack  = {0, 0x0000, 0x0000, 0x0000} ;
GdkColor clrOrange = {0, 0xFFFF, 0x8000, 0x0000} ;
GdkColor clrYellow = {0, 0xFFFF, 0xFFFF, 0x0000} ;
GdkColor clrBlue   = {0, 0x0000, 0x0000, 0xFFFF} ;

static GdkColor clrClock[4] =
  {
  {0, 0x0000, 0xFFFF, 0x0000},
  {0, 0xFFFF, 0x0000, 0xFFFF},
  {0, 0x0000, 0xFFFF, 0xFFFF},
  {0, 0xFFFF, 0xFFFF, 0xFFFF},
  } ;

enum
  {
  QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL,
  QCAD_CELL_LAST_SIGNAL
  } ;

enum
  {
  QCAD_CELL_PROPERTY_FIRST=1,

  QCAD_CELL_PROPERTY_FUNCTION,
  QCAD_CELL_PROPERTY_CLOCK,
  QCAD_CELL_PROPERTY_MODE,
  QCAD_CELL_PROPERTY_LABEL,
  QCAD_CELL_PROPERTY_POLARIZATION,
  QCAD_CELL_PROPERTY_CX,
  QCAD_CELL_PROPERTY_CY,
  QCAD_CELL_PROPERTY_DOT_DIAM,

  QCAD_CELL_PROPERTY_LAST
  } ;

static guint qcad_cell_signals[QCAD_CELL_LAST_SIGNAL] = {0} ;

#ifdef PROPERTY_UIS
int label_enabled_if[] =
  {
  QCAD_CELL_INPUT,
  QCAD_CELL_OUTPUT
  } ;

INT_IN_LIST_PARAMS label_enabled_if_list =
  {
  G_N_ELEMENTS (label_enabled_if),
  label_enabled_if
  } ;

int polarization_enabled_if[] =
  {
  QCAD_CELL_FIXED
  } ;

INT_IN_LIST_PARAMS polarization_enabled_if_list =
  {
  G_N_ELEMENTS (polarization_enabled_if),
  polarization_enabled_if
  } ;

// Gotta be static so the strings don't die
static QCADPropertyUIBehaviour behaviour[] =
  {
    // cell.label.sensitive = (cell.function == QCAD_CELL_INPUT || cell.function == QCAD_CELL_OUTPUT)
    {
    "function", NULL, "label", "sensitive", 
    CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_IN_LIST_P, &label_enabled_if_list, NULL,
    NULL, NULL, NULL
    },
    {
    // cell.polarization.sensitive = (cell.function == QCAD_CELL_FIXED)
    "function", NULL, "polarization", "sensitive", 
    CONNECT_OBJECT_PROPERTIES_ASSIGN_INT_IN_LIST_P, &polarization_enabled_if_list, NULL,
    NULL, NULL, NULL
    }
  } ;
#endif /* def PROPERTY_UIS */

GType qcad_cell_get_type ()
  {
  static GType qcad_cell_type = 0 ;

  if (!qcad_cell_type)
    {
    static const GTypeInfo qcad_cell_info =
      {
      sizeof (QCADCellClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_cell_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADCell),
      0,
      (GInstanceInitFunc)qcad_cell_instance_init
      } ;

    static GInterfaceInfo qcad_cell_compound_do_info =
      {
      (GInterfaceInitFunc)qcad_cell_compound_do_interface_init,
      NULL,
      NULL
      } ;
/*
    static GInterfaceInfo qcad_cell_do_container_info =
      {
      (GInterfaceInitFunc)qcad_cell_do_container_interface_init,
      NULL,
      NULL
      } ;
*/
    if ((qcad_cell_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_CELL, &qcad_cell_info, 0)))
      {
      g_type_add_interface_static (qcad_cell_type, QCAD_TYPE_COMPOUND_DO, &qcad_cell_compound_do_info) ;
//      g_type_add_interface_static (qcad_cell_type, QCAD_TYPE_DO_CONTAINER, &qcad_cell_do_container_info) ;
      g_type_class_ref (qcad_cell_type) ;
      }
    }
  return qcad_cell_type ;
  }

GType qcad_cell_function_get_type ()
  {
  static GType qcad_cell_function_type = 0 ;

  if (!qcad_cell_function_type)
    {
    static GEnumValue values[] =
      {
      {QCAD_CELL_NORMAL, "QCAD_CELL_NORMAL", NULL},
      {QCAD_CELL_INPUT,  "QCAD_CELL_INPUT",  NULL},
      {QCAD_CELL_OUTPUT, "QCAD_CELL_OUTPUT", NULL},
      {QCAD_CELL_FIXED,  "QCAD_CELL_FIXED",  NULL},
      {0, NULL, NULL}
      } ;

    values[0].value_nick = _("Normal") ;
    values[1].value_nick = _("Input") ;
    values[2].value_nick = _("Output") ;
    values[3].value_nick = _("Fixed Polarization") ;

    qcad_cell_function_type = g_enum_register_static (QCAD_TYPE_STRING_CELL_FUNCTION, values) ;
    }

  return qcad_cell_function_type ;
  }

GType qcad_cell_mode_get_type ()
  {
  static GType qcad_cell_mode_type = 0 ;

  if (!qcad_cell_mode_type)
    {
    static GEnumValue values[] =
      {
      {QCAD_CELL_MODE_NORMAL,    "QCAD_CELL_MODE_NORMAL",    NULL},
      {QCAD_CELL_MODE_CROSSOVER, "QCAD_CELL_MODE_CROSSOVER", NULL},
      {QCAD_CELL_MODE_VERTICAL,  "QCAD_CELL_MODE_VERTICAL",  NULL},
      {0, NULL, NULL}
      } ;

    values[0].value_nick = _("Normal") ;
    values[1].value_nick = _("Crossover") ;
    values[2].value_nick = _("Vertical") ;

    qcad_cell_mode_type = g_enum_register_static (QCAD_TYPE_STRING_CELL_MODE, values) ;
    }

  return qcad_cell_mode_type ;
  }

static void qcad_cell_compound_do_interface_init (gpointer iface, gpointer interface_data)
  {
  QCADCompoundDOClass *klass = (QCADCompoundDOClass *)iface ;

  klass->first = qcad_cell_compound_do_first ;
  klass->next = qcad_cell_compound_do_next ;
  klass->last = qcad_cell_compound_do_last ;
  }
/*
static void qcad_cell_do_container_interface_init (gpointer iface, gpointer interface_data)
  {
  QCADDOContainerClass *klass = (QCADDOContainerClass *)iface ;

  klass->add = qcad_cell_do_container_add ;
  klass->remove = qcad_cell_do_container_remove ;
  }
*/
static void qcad_cell_class_init (GObjectClass *klass, gpointer data)
  {
#ifdef GTK_GUI
  GdkColormap *clrmap = gdk_colormap_get_system () ;
#endif /* def GTK_GUI */
#ifdef PROPERTY_UIS
  // Gotta be static so the strings don't die
  static QCADPropertyUIProperty properties[] =
    {
    {NULL,           "title",     {0, }},
    {"width",        "units",     {0, }},
    {"height",       "units",     {0, }},
    {"dot-diameter", "units",     {0, }},
#ifdef GTK_GUI
    {"clock",        "render-as", {0, }},
#endif /* def GTK_GUI */
    } ;

  // cell.title = "QCA Cell"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), _("QCA Cell")) ;
  // cell.width.units = "nm"
  g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;
  // cell.height.units = "nm"
  g_value_set_string (g_value_init (&(properties[2].ui_property_value), G_TYPE_STRING), "nm") ;
  // cell.dot-diameter.units = "nm"
  g_value_set_string (g_value_init (&(properties[3].ui_property_value), G_TYPE_STRING), "nm") ;
#ifdef GTK_GUI
  // cell.clock.render-as = GTK_TYPE_COMBO_BOX
  g_value_set_uint   (g_value_init (&(properties[4].ui_property_value), G_TYPE_UINT), (guint)GTK_TYPE_COMBO_BOX) ;
#endif /* def GTK_GUI */

  qcad_object_class_install_ui_behaviour (QCAD_OBJECT_CLASS (klass), behaviour, G_N_ELEMENTS (behaviour)) ;
  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
#endif /* def PROPERTY_UIS */

#ifdef GTK_GUI
  if (0 == clrOrange.pixel)
    gdk_colormap_alloc_color (clrmap, &clrOrange, FALSE, TRUE) ;
  if (0 == clrYellow.pixel)
    gdk_colormap_alloc_color (clrmap, &clrYellow, FALSE, TRUE) ;
  if (0 == clrBlue.pixel)
    gdk_colormap_alloc_color (clrmap, &clrBlue, FALSE, TRUE) ;
  if (0 == clrBlack.pixel)
    gdk_colormap_alloc_color (clrmap, &clrBlack, FALSE, TRUE) ;

  gdk_colormap_alloc_color (clrmap, &(clrClock[0]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[1]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[2]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[3]), FALSE, TRUE) ;
#endif /* def GTK_GUI */

  QCAD_OBJECT_CLASS (klass)->copy                     = copy ;
  QCAD_OBJECT_CLASS (klass)->class_get_default_object = class_get_default_object ;
  memcpy (&(QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault), &(clrClock[0]), sizeof (GdkColor)) ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->hit_test                   = hit_test ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->get_bounds_box             = get_bounds_box ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->set_selected               = set_selected ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->add_unique_types           = add_unique_types ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble        = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance        = PostScript_instance ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                       = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed          = (GCallback)button_pressed ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize                  = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize                = unserialize ;
#endif /* def STDIO_FILEIO */
  QCAD_DESIGN_OBJECT_CLASS (klass)->move                       = move ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->transform                  = transform ;
  G_OBJECT_CLASS (klass)->finalize                             = qcad_cell_instance_finalize ;
  G_OBJECT_CLASS (klass)->set_property                         = qcad_cell_set_property ;
  G_OBJECT_CLASS (klass)->get_property                         = qcad_cell_get_property ;

  /**
   * QCADCell:function:
   *
   * The cell's function. One of %QCAD_CELL_NORMAL, %QCAD_CELL_INPUT, %QCAD_CELL_OUTPUT, or %QCAD_CELL_FIXED
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_FUNCTION,
    g_param_spec_enum ("function", _("Function"), _("Cell Function:Normal/Input/Output/Fixed"),
      QCAD_TYPE_CELL_FUNCTION, QCAD_CELL_NORMAL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_CLOCK,
    g_param_spec_uint ("clock", _("Clock"), _("Cell Clock"),
      0, 3, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_MODE,
    g_param_spec_enum ("mode", _("Mode"), _("Cell Drawing Mode"),
      QCAD_TYPE_CELL_MODE, QCAD_CELL_MODE_NORMAL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_LABEL,
    g_param_spec_string ("label", _("Label"), _("Cell Label"),
      _("Untitled"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_POLARIZATION,
    g_param_spec_double ("polarization", _("Polarization"), _("Cell Polarization"),
      -1.0, 1.0, 0.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_CX,
    g_param_spec_double ("width", _("Cell Width"), _("Cell Width"),
      0.1, 1e9, 18.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_CY,
    g_param_spec_double ("height", _("Cell Height"), _("Cell Height"),
      0.1, 1e9, 18.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_DOT_DIAM,
    g_param_spec_double ("dot-diameter", _("Dot Diameter"), _("Diameter of the quantum dot"),
      0.1, 1e9, 5.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  /**
   * QCADCell::cell-function-changed:
   * @QCADCell: the cell whose function has changed.
   *
   * This signal is emitted when the cell function changes.
   */
  qcad_cell_signals[QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL] =
    g_signal_new ("cell-function-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADCellClass, cell_function_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0) ;
  }

static void qcad_cell_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADCell *cell = QCAD_CELL (object) ;

  switch (property_id)
    {
    case QCAD_CELL_PROPERTY_FUNCTION:
      qcad_cell_set_function (cell, g_value_get_enum (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell function to %s\n",
        g_enum_get_value (g_type_class_peek (QCAD_TYPE_CELL_FUNCTION), g_value_get_enum (value))->value_name)) ;
      break ;

    case QCAD_CELL_PROPERTY_CLOCK:
      qcad_cell_set_clock (cell, g_value_get_uint (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell clock to %d\n", g_value_get_uint (value))) ;
      break ;

    case QCAD_CELL_PROPERTY_MODE:
      qcad_cell_set_display_mode (cell, g_value_get_enum (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell mode to %s\n",
        g_enum_get_value (g_type_class_peek (QCAD_TYPE_CELL_MODE), g_value_get_enum (value))->value_name)) ;
      break ;

    case QCAD_CELL_PROPERTY_LABEL:
      qcad_cell_set_label (cell, (char *)g_value_get_string (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell label to \"%s\"\n", (char *)g_value_get_string (value))) ;
      break ;

    case QCAD_CELL_PROPERTY_POLARIZATION:
      qcad_cell_set_polarization (cell, g_value_get_double (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell polarization to %lf\n", g_value_get_double (value))) ;
      break ;

    case QCAD_CELL_PROPERTY_CX:
      qcad_cell_set_size (cell, g_value_get_double (value), cell->cell_options.cyCell, cell->cell_options.dot_diameter) ;
      break ;

    case QCAD_CELL_PROPERTY_CY:
      qcad_cell_set_size (cell, cell->cell_options.cxCell, g_value_get_double (value), cell->cell_options.dot_diameter) ;
      break ;

    case QCAD_CELL_PROPERTY_DOT_DIAM:
      qcad_cell_set_size (cell, cell->cell_options.cxCell, cell->cell_options.cyCell, g_value_get_double (value)) ;
      break ;
    }
  }

static void qcad_cell_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADCell *cell = QCAD_CELL (object) ;

  switch (property_id)
    {
    case QCAD_CELL_PROPERTY_FUNCTION:
      g_value_set_enum (value, cell->cell_function) ;
      break ;

    case QCAD_CELL_PROPERTY_CLOCK:
      g_value_set_uint (value, cell->cell_options.clock) ;
      break ;

    case QCAD_CELL_PROPERTY_MODE:
      g_value_set_enum (value, cell->cell_options.mode) ;
      break ;

    case QCAD_CELL_PROPERTY_LABEL:
      g_value_set_string (value, qcad_cell_get_label (cell)) ;
      break ;

    case QCAD_CELL_PROPERTY_POLARIZATION:
      g_value_set_double (value, qcad_cell_calculate_polarization (cell)) ;
      break ;

    case QCAD_CELL_PROPERTY_CX:
      g_value_set_double (value, cell->cell_options.cxCell) ;
      break ;

    case QCAD_CELL_PROPERTY_CY:
      g_value_set_double (value, cell->cell_options.cyCell) ;
      break ;

    case QCAD_CELL_PROPERTY_DOT_DIAM:
      g_value_set_double (value, cell->cell_options.dot_diameter) ;
      break ;
    }
  }

static void qcad_cell_instance_init (GObject *object, gpointer data)
  {
  double dcx, dcy, ddiam ;
//  QCADCellClass *klass = QCAD_CELL_GET_CLASS (object) ;
  QCADCell *cell = QCAD_CELL (object) ;

  dcx = 
  cell->cell_options.cxCell         = 18.00 ;
  dcy = 
  cell->cell_options.cyCell         = 18.00 ;
  ddiam =
  cell->cell_options.dot_diameter   = 5.0 ;
  cell->cell_options.mode           = QCAD_CELL_MODE_NORMAL ;
  cell->cell_options.clock          = 0 ;
  
  memcpy (&(QCAD_DESIGN_OBJECT (object)->clr), &(clrClock[0]), sizeof (GdkColor)) ;
  cell->id = (int)object ;
  cell->host_name = NULL ;
  cell->cell_function = QCAD_CELL_NORMAL ;
  cell->cell_model = NULL ;
  cell->cell_dots = malloc (4 * sizeof (QCADCellDot)) ;
  cell->number_of_dots = 4 ;
  cell->label = NULL ;
  cell->bLabelRemoved = TRUE ;

  qcad_cell_calculate_coords (cell) ;

  cell->cell_dots[0].charge =
  cell->cell_dots[1].charge =
  cell->cell_dots[2].charge =
  cell->cell_dots[3].charge = HALF_QCHARGE ;
  }

static void qcad_cell_instance_finalize (GObject *object)
  {
  if (NULL != QCAD_CELL (object)->cell_dots)
    free (QCAD_CELL (object)->cell_dots) ;
  if (NULL != QCAD_CELL (object)->label)
    g_object_unref (QCAD_CELL (object)->label) ;
  if (NULL != QCAD_CELL (object)->cell_model)
    free (QCAD_CELL (object)->cell_model) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->finalize (object) ;
  }

///////////////////////////////////////////////////////////////////////////////
/*
QCADDesignObject *qcad_cell_new_with_function (QCADCellFunction cell_function, char *pszLabel)
  {return g_object_new (QCAD_TYPE_CELL, "function", cell_function, "label", pszLabel, NULL) ;}

QCADDesignObject *qcad_cell_new (double x, double y)
  {
  double dcx, dcy, ddiam ;
  QCADDesignObject *ret = QCAD_DESIGN_OBJECT (g_object_new (QCAD_TYPE_CELL, NULL)) ;
  QCADCellClass *klass = g_type_class_peek (QCAD_TYPE_CELL) ;
  QCADCell *cell = QCAD_CELL (ret) ;

  dcx = klass->default_cell_options.cxCell ;
  dcy = klass->default_cell_options.cyCell ;
  ddiam = klass->default_cell_options.dot_diameter ;

  ret->x = x ;
  ret->y = y ;
  ret->bounding_box.xWorld = x - dcx / 2.0 ;
  ret->bounding_box.yWorld = y - dcy / 2.0 ;
  ret->bounding_box.cxWorld = dcx ;
  ret->bounding_box.cyWorld = dcy ;

  QCAD_CELL (ret)->cell_dots[0].charge =
  QCAD_CELL (ret)->cell_dots[1].charge =
  QCAD_CELL (ret)->cell_dots[2].charge =
  QCAD_CELL (ret)->cell_dots[3].charge = HALF_QCHARGE ;

  QCAD_CELL (ret)->cell_dots[0].diameter =
  QCAD_CELL (ret)->cell_dots[1].diameter =
  QCAD_CELL (ret)->cell_dots[2].diameter =
  QCAD_CELL (ret)->cell_dots[3].diameter = ddiam ;

  cell->cell_dots[3].x = x - (dcx / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[3].y = y - (dcy / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[0].x = x + (dcx / 2 - cell->cell_dots[0].diameter) / 2 + cell->cell_dots[0].diameter / 2;
  cell->cell_dots[0].y = y - (dcy / 2 - cell->cell_dots[0].diameter) / 2 - cell->cell_dots[0].diameter / 2;
  cell->cell_dots[2].x = x - (dcx / 2 - cell->cell_dots[2].diameter) / 2 - cell->cell_dots[2].diameter / 2;
  cell->cell_dots[2].y = y + (dcy / 2 - cell->cell_dots[2].diameter) / 2 + cell->cell_dots[2].diameter / 2;
  cell->cell_dots[1].x = x + (dcx / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;
  cell->cell_dots[1].y = y + (dcy / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;

  return ret ;
  }
*/
const char *qcad_cell_get_label (QCADCell *cell)
  {
  if (NULL == cell) return "" ;
  else
  if (cell->bLabelRemoved) return "" ;
  else
  if (NULL == cell->label) return "" ;
  else
  if (NULL == cell->label->psz) return "" ;
  else
    return cell->label->psz ;
  }

void qcad_cell_set_display_mode (QCADCell *cell, int cell_mode)
  {cell->cell_options.mode = cell_mode ;}

#ifdef DESIGNER
#ifdef GTK_GUI
void qcad_cell_drexp_array (GdkDrawable *dst, GdkFunction rop, GtkOrientation orientation, double dRangeBeg, double dRangeEnd, double dOtherCoord)
  {
  int idx[2] = {-1} ;
  GdkGC *gc = NULL ;
  QCADCell *default_cell = NULL ;
  double
    dDir = 0,
    coord[2], length[2] = {-1} ;

  if (NULL == (QCAD_OBJECT_CLASS (g_type_class_peek (QCAD_TYPE_CELL)))->default_object) return ;

  default_cell = QCAD_CELL ((QCAD_OBJECT_CLASS (g_type_class_peek (QCAD_TYPE_CELL)))->default_object) ;

  gc = gdk_gc_new (dst) ;

  idx[0] = GTK_ORIENTATION_HORIZONTAL == orientation ? 0 : 1 ;
  idx[1] = GTK_ORIENTATION_HORIZONTAL == orientation ? 1 : 0 ;
  dDir = dRangeBeg < dRangeEnd ? 1 : -1 ;
  length[0] = default_cell->cell_options.cxCell ;
  length[1] = default_cell->cell_options.cyCell ;

  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_foreground (gc, &(clrClock[default_cell->cell_options.clock])) ;

  coord[idx[0]] = dRangeBeg ;
  coord[idx[1]] = dOtherCoord ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (dDir * coord[idx[0]] < dDir * dRangeEnd)
    {
    gdk_draw_rectangle (dst, gc, FALSE,
      world_to_real_x (coord[0] - length[0] / 2.0),
      world_to_real_y (coord[1] - length[1] / 2.0),
      world_to_real_cx (length[0]),
      world_to_real_cy (length[1])) ;
    qcad_cell_array_next_coord (idx, coord, length, dDir) ;
    }
  }
#endif /* def GTK_GUI */

EXP_ARRAY *qcad_cell_create_array (gboolean bHorizontal, double dRangeBeg, double dRangeEnd, double dOtherCoord)
  {
  int idx[2] = {-1} ;
  double
    dDir = dRangeBeg < dRangeEnd ? 1 : -1,
    coord[2], length[2] = {-1} ;
  EXP_ARRAY *ret = NULL ;
  QCADCell *default_cell = NULL ;
  QCADDesignObject *cell = NULL ;

  if (NULL == (QCAD_OBJECT_CLASS (g_type_class_peek (QCAD_TYPE_CELL)))->default_object) return NULL ;

  default_cell = QCAD_CELL ((QCAD_OBJECT_CLASS (g_type_class_peek (QCAD_TYPE_CELL)))->default_object) ;

  idx[0] = bHorizontal ? 0 : 1 ;
  idx[1] = bHorizontal ? 1 : 0 ;
  dDir = dRangeBeg < dRangeEnd ? 1 : -1 ;
  length[0] = default_cell->cell_options.cxCell ;
  length[1] = default_cell->cell_options.cyCell ;

  coord[idx[0]] = dRangeBeg ;
  coord[idx[1]] = dOtherCoord ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (dDir * coord[idx[0]] < dDir * dRangeEnd)
    {
    if (NULL == ret)
      ret = exp_array_new (sizeof (QCADCell *), 1) ;
    cell = QCAD_DESIGN_OBJECT (qcad_object_new_from_object (qcad_object_get_default (QCAD_TYPE_CELL))) ;
    qcad_design_object_move_to (cell, coord[0], coord[1]) ;
    exp_array_1d_insert_vals (ret, &cell, 1, -1) ;
    qcad_cell_array_next_coord (idx, coord, length, dDir) ;
    }
  return ret ;
  }
#endif /* def DESIGNER */

void qcad_cell_rotate_dots (QCADCell *cell, double angle)
  {
  int i;
  double x, cell_x = QCAD_DESIGN_OBJECT (cell)->x ;
  double y, cell_y = QCAD_DESIGN_OBJECT (cell)->y ;

  // -- uses standard rotational transform -- //
  for (i = 0; i < cell->number_of_dots; i++)
    {
    x = cell_x + (cell->cell_dots[i].x -
      cell_x) * (float) cos(angle) -
      (cell->cell_dots[i].y - cell_y) * (float) sin(angle);
    y = cell_y + (cell->cell_dots[i].y -
      cell_y) * (float) cos(angle) +
      (cell->cell_dots[i].x - cell_x) * (float) sin(angle);
    cell->cell_dots[i].x = x;
    cell->cell_dots[i].y = y;
    }
  }

void qcad_cell_set_host_name (QCADCell *cell, char *pszHostName)
  {cell->host_name = g_strdup (pszHostName) ;}

///////////////////////////////////////////////////////////////////////////////

static QCADObject *class_get_default_object ()
  {return g_object_new (QCAD_TYPE_CELL, NULL) ;}
/*
static gboolean qcad_cell_do_container_add (QCADDOContainer *container, QCADDesignObject *obj)
  {
  QCADCell *cell = QCAD_CELL (container) ;

  if (NULL == cell->label) return FALSE ;
  if (QCAD_DESIGN_OBJECT (cell->label) == obj && cell->bLabelRemoved)
    {
    g_object_ref (cell->label) ;
    cell->bLabelRemoved = FALSE ;
    return TRUE ;
    }
  return FALSE ;
  }

static gboolean qcad_cell_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj)
  {
  QCADCell *cell = QCAD_CELL (container) ;

  if (NULL == cell->label) return FALSE ;
  if (QCAD_DESIGN_OBJECT (cell->label) == obj && !(cell->bLabelRemoved))
    {
    cell->bLabelRemoved = TRUE ;
    return TRUE ;
    }
  return FALSE ;
  }
*/
static QCADDesignObject *qcad_cell_compound_do_first (QCADCompoundDO *cdo)
  {return (QCAD_CELL (cdo)->bLabelRemoved ? NULL : (QCADDesignObject *)(QCAD_CELL (cdo)->label)) ;}
static QCADDesignObject *qcad_cell_compound_do_next (QCADCompoundDO *cdo)
  {return NULL ;}
static gboolean qcad_cell_compound_do_last (QCADCompoundDO *cdo)
  {return TRUE ;}

static void copy (QCADObject *src, QCADObject *dst)
  {
  int Nix = -1 ;
  QCADCell *cellSrc = QCAD_CELL (src), *cellDst = QCAD_CELL (dst) ;

  QCAD_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->copy (src, dst) ;

  cellDst->cell_options.cxCell       = cellSrc->cell_options.cxCell ;
  cellDst->cell_options.clock        = cellSrc->cell_options.clock ;
  cellDst->cell_options.cyCell       = cellSrc->cell_options.cyCell ;
  cellDst->cell_options.dot_diameter = cellSrc->cell_options.dot_diameter ;
  cellDst->cell_options.mode         = cellSrc->cell_options.mode ;
  cellDst->cell_function             = cellSrc->cell_function ;
  cellDst->bLabelRemoved             = FALSE ;
  // CAREFUL ! This is a shallow copy of the cell model
  cellDst->cell_model                = cellSrc->cell_model ;
  if (NULL != cellSrc->label)
    {
    cellDst->label = QCAD_LABEL (qcad_object_new_from_object (QCAD_OBJECT (cellSrc->label))) ;
    g_object_add_weak_pointer (G_OBJECT (cellDst->label), (gpointer *)&(cellDst->label)) ;
    }
  else
    cellDst->label = NULL ;

  if (NULL != cellDst->cell_dots)
    free (cellDst->cell_dots) ;
  cellDst->cell_dots = NULL ;
  cellDst->number_of_dots = 0 ;

  if (cellSrc->number_of_dots > 0)
    {
    cellDst->cell_dots = malloc (cellSrc->number_of_dots * sizeof (QCADCellDot)) ;
    cellDst->number_of_dots = cellSrc->number_of_dots ;

    for (Nix = 0 ; Nix < cellDst->number_of_dots ; Nix++)
      {
      cellDst->cell_dots[Nix].x         = cellSrc->cell_dots[Nix].x ;
      cellDst->cell_dots[Nix].y         = cellSrc->cell_dots[Nix].y ;
      cellDst->cell_dots[Nix].diameter  = cellSrc->cell_dots[Nix].diameter ;
      cellDst->cell_dots[Nix].charge    = cellSrc->cell_dots[Nix].charge ;
      cellDst->cell_dots[Nix].spin      = cellSrc->cell_dots[Nix].spin ;
      cellDst->cell_dots[Nix].potential = cellSrc->cell_dots[Nix].potential ;
      }
    }
  }

static GList *add_unique_types (QCADDesignObject *obj, GList *lst)
  {
  QCADCell *cell = QCAD_CELL (obj) ;
  GList *lstItr = NULL ;
  gboolean bHaveLabel = FALSE, bHaveCell = FALSE ;

  // If there's no label, then perform default behaviour
  if (NULL == cell->label)
    return QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->add_unique_types (obj, lst) ;
  else
    {
    //Look for "QCADCell" and "QCADLabel"
    for (lstItr = lst ; lstItr != NULL && !(bHaveCell && bHaveLabel) ; lstItr = lstItr->next)
      if (QCAD_TYPE_CELL == G_TYPE_FROM_INSTANCE (lstItr->data))
        bHaveCell = TRUE ;
      else
      if (QCAD_TYPE_LABEL == G_TYPE_FROM_INSTANCE (lstItr->data))
        bHaveLabel = TRUE ;
    }

  if (bHaveCell && bHaveLabel) return lst ;

  if (!bHaveCell)
    lst = g_list_prepend (lst, cell) ;

  if (!bHaveLabel)
    lst = g_list_prepend (lst, cell->label) ;

  return lst ;
  }

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  char pszDouble[20][G_ASCII_DTOSTR_BUF_SIZE] = {""} ;
  char *pszRet = NULL, *pszLabel = NULL ;
  QCADCell *cell = QCAD_CELL (obj) ;
  GdkColor clrBak = {0}, clr = {0} ;

  float fclr[3] = {0.00, 0.00, 0.00} ;

  // Clock colours and their grayscale counterparts
  float fclrClock[4][2][3] = {
    {{0.00, 0.50, 0.00}, {0.45, 0.45, 0.45}}, // Clock 0: dark green
    {{0.50, 0.00, 0.50}, {0.65, 0.65, 0.65}}, // Clock 1: dark purple
    {{0.00, 0.50, 0.50}, {0.85, 0.85, 0.85}}, // Clock 2: turquoise
    {{0.95, 0.95, 0.95}, {0.95, 0.95, 0.95}}  // Clock 3: white
    } ;

  float fclrCellFunction[QCAD_CELL_LAST_FUNCTION][3] = {
    {0.00, 0.50, 0.00}, // QCAD_CELL_NORMAL: dark green (not used - clock takes precedence)
    {0.21, 0.39, 0.70}, // QCAD_CELL_INPUT:  dark azure blue
    {0.66, 0.66, 0.00}, // QCAD_CELL_OUTPUT: maroonish yellow
    {0.83, 0.44, 0.00}  // QCAD_CELL_FIXED:  dark orange
    } ;

  if (QCAD_CELL_NORMAL == cell->cell_function)
    {
    fclr[0] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][0] ;
    fclr[1] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][1] ;
    fclr[2] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][2] ;
    }
   else
    {
    fclr[0] = bColour ? fclrCellFunction[cell->cell_function][0] : fclrClock[cell->cell_options.clock][1][0] ;
    fclr[1] = bColour ? fclrCellFunction[cell->cell_function][1] : fclrClock[cell->cell_options.clock][1][1] ;
    fclr[2] = bColour ? fclrCellFunction[cell->cell_function][2] : fclrClock[cell->cell_options.clock][1][2] ;
    }

  if (NULL != cell->label)
    {
    // Back up the color of the label, then convert the PostScript colour to a GdkColor
    // and apply it to the label.  Labels don't perform colour lookups so it'll print
    // out the colour exactly as it finds it.  Afterwards, restore label colour from
    // the backup
    memcpy (&clrBak, &(QCAD_DESIGN_OBJECT (cell->label)->clr), sizeof (GdkColor)) ;
    clr.red = fclr[0] * 65535 ;
    clr.green = fclr[1] * 65535 ;
    clr.blue = fclr[2] * 65535 ;
#ifdef GTK_GUI
    // Is this alloc necessary ?
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clr, FALSE, TRUE) ;
#endif /* def GTK_GUI */
    memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clr, sizeof (GdkColor)) ;
    pszLabel = qcad_design_object_get_PostScript_instance (QCAD_DESIGN_OBJECT (cell->label), bColour) ;
    memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clrBak, sizeof (GdkColor)) ;
    }

  pszRet =
    g_strdup_printf (
      "%s nmx %s nmy %s nm %s nm " // x y cx cy
      "%s nm "
      "%s nmx %s nmy %s nmx %s nmy %s nmx %s nmy %s nmx %s nmy " //dot0_x dot0_y dot1_x dot1_y dot2_x dot2_y dot3_x dot3_y
      "%s nm %s nm %s nm %s nm %s %s %s %d QCADCell%s%s", //charge0_diam charge1_diam charge2_diam charge3_diam r g b mode
      g_ascii_dtostr (pszDouble[0],  G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.xWorld), 
      g_ascii_dtostr (pszDouble[1],  G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.yWorld), 
      g_ascii_dtostr (pszDouble[2],  G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cxWorld), 
      g_ascii_dtostr (pszDouble[3],  G_ASCII_DTOSTR_BUF_SIZE, obj->bounding_box.cyWorld),
      g_ascii_dtostr (pszDouble[4],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_options.dot_diameter),
      g_ascii_dtostr (pszDouble[5],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[0].x), 
      g_ascii_dtostr (pszDouble[6],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[0].y), 
      g_ascii_dtostr (pszDouble[7],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[1].x), 
      g_ascii_dtostr (pszDouble[8],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[1].y),
      g_ascii_dtostr (pszDouble[9],  G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[2].x), 
      g_ascii_dtostr (pszDouble[10], G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[2].y), 
      g_ascii_dtostr (pszDouble[11], G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[3].x), 
      g_ascii_dtostr (pszDouble[12], G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[3].y),
      g_ascii_dtostr (pszDouble[13], G_ASCII_DTOSTR_BUF_SIZE, 
        (cell->cell_dots[0].diameter * cell->cell_dots[0].charge / QCHARGE)),
      g_ascii_dtostr (pszDouble[14], G_ASCII_DTOSTR_BUF_SIZE, 
        (cell->cell_dots[1].diameter * cell->cell_dots[1].charge / QCHARGE)),
      g_ascii_dtostr (pszDouble[15], G_ASCII_DTOSTR_BUF_SIZE, 
        (cell->cell_dots[2].diameter * cell->cell_dots[2].charge / QCHARGE)),
      g_ascii_dtostr (pszDouble[16], G_ASCII_DTOSTR_BUF_SIZE, 
        (cell->cell_dots[3].diameter * cell->cell_dots[3].charge / QCHARGE)),
      g_ascii_dtostr (pszDouble[17], G_ASCII_DTOSTR_BUF_SIZE, fclr[0]), 
      g_ascii_dtostr (pszDouble[18], G_ASCII_DTOSTR_BUF_SIZE, fclr[1]), 
      g_ascii_dtostr (pszDouble[19], G_ASCII_DTOSTR_BUF_SIZE, fclr[2]),
      cell->cell_options.mode,
      NULL == pszLabel ? "" : "\n  ",
      NULL == pszLabel ?  "" : pszLabel) ;

  g_free (pszLabel) ;

  return pszRet ;
  }

static const char *PostScript_preamble ()
  {
  return
    "/point\n"
    "  {\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  newpath\n"
    "  x 3 sub y 3 sub moveto\n"
    "  x 3 add y 3 add lineto\n"
    "  x 3 sub y 3 add moveto\n"
    "  x 3 add y 3 sub lineto\n"
    "  stroke\n"
    "  }\n"
    "\n"
    "% x y cx cy dot_diam dot0_x dot0_y dot1_x dot1_y dot2_x dot2_y dot3_x dot3_y charge0_diam charge1_diam charge2_diam charge3_diam r g b mode QCADCell\n"
    "/QCADCell\n"
    "  {\n"
    "  gsave\n"
    "  /mode exch def\n"
    "  /b exch def\n"
    "  /g exch def\n"
    "  /r exch def\n"
    "  /charge3_diam exch def\n"
    "  /charge2_diam exch def\n"
    "  /charge1_diam exch def\n"
    "  /charge0_diam exch def\n"
    "  /dot3_y exch def\n"
    "  /dot3_x exch def\n"
    "  /dot2_y exch def\n"
    "  /dot2_x exch def\n"
    "  /dot1_y exch def\n"
    "  /dot1_x exch def\n"
    "  /dot0_y exch def\n"
    "  /dot0_x exch def\n"
    "  /dot_diam exch def\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  % Filler\n"
    "  newpath\n"
    "  x y moveto\n"
    "  x cx add y lineto\n"
    "  x cx add y cy sub lineto\n"
    "  x y cy sub lineto\n"
    "  x y lineto\n"
    "  mode 0 eq dot_diam epsilon gt and\n"
    "    {\n"
    "    dot0_x dot_diam 2 div add dot0_y moveto\n"
    "    dot0_x dot0_y dot_diam 2 div 0 360 arc\n"
    "    dot1_x dot_diam 2 div add dot1_y moveto\n"
    "    dot1_x dot1_y dot_diam 2 div 0 360 arc\n"
    "    dot2_x dot_diam 2 div add dot2_y moveto\n"
    "    dot2_x dot2_y dot_diam 2 div 0 360 arc\n"
    "    dot3_x dot_diam 2 div add dot3_y moveto\n"
    "    dot3_x dot3_y dot_diam 2 div 0 360 arc\n"
    "    }\n"
    "  if\n"
    "  r g b setrgbcolor\n"
    "  fill\n"
    "  grestore\n"
    "\n"
    "  linewidth epsilon gt\n"
    "    {\n"
    "    % Cell outline\n"
    "    newpath\n"
    "    x y moveto\n"
    "    x cx add y lineto\n"
    "    x cx add y cy sub lineto\n"
    "    x y cy sub lineto\n"
    "    closepath\n"
    "    stroke\n"
    "    }\n"
    "  if\n"
    "\n"
    "  gsave\n"
    "\n"
    "  mode 0 eq\n"
    "    {\n"
    "    linewidth epsilon gt\n"
    "      {\n"
    "      %dot0 outline\n"
    "      newpath\n"
    "      dot0_x dot0_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot1 outline\n"
    "      newpath\n"
    "      dot1_x dot1_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot2 outline\n"
    "      newpath\n"
    "      dot2_x dot2_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot3 outline\n"
    "      newpath\n"
    "      dot3_x dot3_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "      }\n"
    "    if\n"
    "\n"
    "    dot_diam epsilon gt\n"
    "      {\n"
    "      %dot0 charge\n"
    "      newpath\n"
    "      dot0_x dot0_y charge0_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot1 charge\n"
    "      newpath\n"
    "      dot1_x dot1_y charge1_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot2 charge\n"
    "      newpath\n"
    "      dot2_x dot2_y charge2_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot3 charge\n"
    "      newpath\n"
    "      dot3_x dot3_y charge3_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "      }\n"
    "    if\n"
    "    }\n"
    "    {\n"
    "    linewidth epsilon gt\n"
    "      {\n"
    "      mode 1 eq\n"
    "        {\n"
    "        % draw an X across the cell\n"
    "        newpath\n"
    "        x y moveto\n"
    "        x cx add y cy sub lineto\n"
    "        stroke\n"
    "\n"
    "        newpath\n"
    "        x cx add y moveto\n"
    "        x y cy sub lineto\n"
    "        stroke\n"
    "        }\n"
    "        {\n"
    "        % draw a circle\n"
    "        newpath\n"
    "        x cx 2 div add y cy 2 div sub cx 2 div 0 360 arc\n"
    "        closepath stroke\n"
    "        }\n"
    "      ifelse\n"
    "      } % linewidth epsilon gt\n"
    "    if\n"
    "    }\n"
    "  ifelse\n"
    "\n"
    "  grestore\n"
    "  } def\n" ;
  }

static void move (QCADDesignObject *obj, double dxDelta, double dyDelta)
  {
  int Nix ;
  QCADCell *cell = QCAD_CELL (obj) ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek_parent (QCAD_DESIGN_OBJECT_GET_CLASS (obj)))->move (obj, dxDelta, dyDelta) ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), dxDelta, dyDelta) ;

  for (Nix = 0 ; Nix < cell->number_of_dots ; Nix++)
    {
    cell->cell_dots[Nix].x += dxDelta ;
    cell->cell_dots[Nix].y += dyDelta ;
    }
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop, GdkRectangle *rcClip)
  {
  GdkRectangle rc ;
  QCADCell *cell = QCAD_CELL (obj) ;
  int Nix ;
  GdkGC *gc = NULL ;

  world_to_real_rect (&(obj->bounding_box), &rc) ;

  // If this cell is not visible, return
  if (!is_real_rect_visible (&rc)) return ;

//  fprintf (stderr, "QCADCell::draw (0x%08X):Entering\n", (int)obj) ;

  gc = gdk_gc_new (dst) ;

  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;

  // The cell outline
  gdk_draw_rectangle (dst, gc, FALSE, rc.x, rc.y, rc.width, rc.height) ;

  if (QCAD_CELL_MODE_NORMAL == cell->cell_options.mode)
    // The cell dots
    for (Nix = 0 ; Nix < cell->number_of_dots ; Nix++)
      {
      // Blank out the previous contents of the dot
      gdk_gc_set_foreground (gc, &clrBlack) ;

      gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
      // draw the dot in its current state
      gdk_draw_arc (dst, gc, FALSE,
      world_to_real_x (cell->cell_dots[Nix].x - cell->cell_dots[Nix].diameter / 2.0),
      world_to_real_y (cell->cell_dots[Nix].y - cell->cell_dots[Nix].diameter / 2.0),
      world_to_real_cx (cell->cell_dots[Nix].diameter),
      world_to_real_cy (cell->cell_dots[Nix].diameter), 0, 23040) ; // 23040 = 360 * 64
      gdk_draw_arc (dst, gc, TRUE,
      world_to_real_x (cell->cell_dots[Nix].x - cell->cell_dots[Nix].diameter / 2.0 * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_y (cell->cell_dots[Nix].y - cell->cell_dots[Nix].diameter / 2.0 * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_cx (cell->cell_dots[Nix].diameter * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_cy (cell->cell_dots[Nix].diameter * cell->cell_dots[Nix].charge/QCHARGE), 0, 23040) ; // 23040 = 360 * 64
      }
    else
    if (QCAD_CELL_MODE_CROSSOVER == cell->cell_options.mode)
      // draw the diagonals of the cell outline
      {
      gdk_draw_line (dst, gc, rc.x, rc.y, rc.x + rc.width, rc.y + rc.height) ;
      gdk_draw_line (dst, gc, rc.x + rc.width, rc.y, rc.x, rc.y + rc.height) ;
      }
    else
      gdk_draw_arc (dst, gc, FALSE, rc.x, rc.y, rc.width, rc.width, 0, 23040) ;

  g_object_unref (gc) ;

  if (!(cell->bLabelRemoved || NULL == cell->label) && QCAD_DESIGN_OBJECT (cell)->bSelected == QCAD_DESIGN_OBJECT (cell->label)->bSelected)
    qcad_design_object_draw (QCAD_DESIGN_OBJECT (cell->label), dst, rop, rcClip) ;
  }
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADCell *cell = QCAD_CELL (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;
  int current_dot = 0 ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_CELL "]", NULL)) return FALSE ;

  g_free (cell->cell_dots) ;
  cell->cell_dots = NULL ;
  cell->number_of_dots = 0 ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;
    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_CELL "]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        tokenize_line_type (pszLine, strlen (pszLine), &pszValue, ':') ;

        if (!strcmp (pszValue, QCAD_TYPE_STRING_DESIGN_OBJECT))
          {
          if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->unserialize (obj, fp)))
            bStopReading = TRUE ;
          }
        else
        if (!strcmp (pszValue, QCAD_TYPE_STRING_LABEL))
          {
          cell->label = QCAD_LABEL (qcad_design_object_new_from_stream (fp)) ;
          qcad_label_shrinkwrap (cell->label) ;
          g_object_add_weak_pointer (G_OBJECT (cell->label), (gpointer *)&(cell->label)) ;
          cell->bLabelRemoved = FALSE ;
          }
        else
        if (!strcmp (pszValue, "CELL_DOT"))
          {
          if (0 == cell->number_of_dots)
            bStopReading = TRUE ;
          else
          if (current_dot < cell->number_of_dots)
            {
            qcad_cell_dot_unserialize (fp, cell->cell_dots, current_dot) ;
            current_dot++ ;
            }
          else
            bStopReading = TRUE ;
          }
        }
      else
      if (!strcmp (pszLine, "cell_options.cxCell"))
        cell->cell_options.cxCell = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.cyCell"))
        cell->cell_options.cyCell = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.clock"))
        cell->cell_options.clock = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.mode"))
        cell->cell_options.mode =
          !strcmp (pszValue, "TRUE")  ? QCAD_CELL_MODE_NORMAL :
          !strcmp (pszValue, "FALSE") ? QCAD_CELL_MODE_CROSSOVER :
            get_enum_value_from_string (QCAD_TYPE_CELL_MODE, pszValue) ;
      else
      if (!strcmp (pszLine, "cell_options.dot_diameter"))
        cell->cell_options.dot_diameter = g_ascii_strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "cell_function"))
        cell->cell_function = get_enum_value_from_string (QCAD_TYPE_CELL_FUNCTION, pszValue) ;
      else
      if (!strcmp (pszLine, "number_of_dots"))
        {
        cell->number_of_dots = atoi (pszValue) ;
        cell->cell_dots = g_malloc0 (cell->number_of_dots * sizeof (QCADCellDot)) ;
        }
      else
      if (!strcmp (pszLine, "label"))
        if (pszValue[0] != 0)
          qcad_cell_set_label (cell, pszValue) ;
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  return (bParentInit && !bStopReading) ;
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  char pszDouble[G_ASCII_DTOSTR_BUF_SIZE] = "" ;
  int i;
  char *psz = NULL ;
  QCADCell *cell = QCAD_CELL (obj) ;

  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_CELL);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->serialize (obj, fp) ;

  // output variables
  fprintf(fp, "cell_options.cxCell=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_options.cxCell));
  fprintf(fp, "cell_options.cyCell=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_options.cyCell));
  fprintf(fp, "cell_options.dot_diameter=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_options.dot_diameter));
  fprintf(fp, "cell_options.clock=%d\n", cell->cell_options.clock);
  fprintf(fp, "cell_options.mode=%s\n", psz = get_enum_string_from_value (QCAD_TYPE_CELL_MODE, cell->cell_options.mode));
  g_free (psz) ;
  fprintf(fp, "cell_function=%s\n", psz = get_enum_string_from_value (QCAD_TYPE_CELL_FUNCTION, cell->cell_function));
  g_free (psz) ;
  fprintf(fp, "number_of_dots=%d\n", cell->number_of_dots);

  for(i = 0; i < cell->number_of_dots; i++)
    {
    fprintf(fp, "[TYPE:CELL_DOT]\n");
    fprintf(fp, "x=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].x));
    fprintf(fp, "y=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].y));
    fprintf(fp, "diameter=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].diameter));
    fprintf(fp, "charge=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].charge));
    fprintf(fp, "spin=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].spin));

    fprintf(fp, "potential=%s\n", g_ascii_dtostr (pszDouble, G_ASCII_DTOSTR_BUF_SIZE, cell->cell_dots[i].potential));
    fprintf(fp, "[#TYPE:CELL_DOT]\n");
    }

  if (!(NULL == cell->label || cell->bLabelRemoved))
    qcad_design_object_serialize (QCAD_DESIGN_OBJECT (cell->label), fp) ;

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_CELL);
  }
#endif /* def STDIO_FILEIO */

static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected)
  {
  gboolean bRet = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->set_selected (obj, bSelected) ;
  if (NULL != QCAD_CELL (obj)->label)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), bSelected) ;
  return bRet ;
  }

static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rc)
  {
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->get_bounds_box (obj, rc) ;
  if (NULL != QCAD_CELL (obj)->label)
    {
    double x = rc->xWorld, y = rc->yWorld ;
    WorldRectangle rcLabel ;
    qcad_design_object_get_bounds_box (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), &rcLabel) ;

    rc->xWorld = MIN (rc->xWorld, rcLabel.xWorld) ;
    rc->yWorld = MIN (rc->yWorld, rcLabel.yWorld) ;
    rc->cxWorld = MAX (rc->cxWorld + x, rcLabel.cxWorld + rcLabel.xWorld) - rc->xWorld ;
    rc->cyWorld = MAX (rc->cyWorld + y, rcLabel.cyWorld + rcLabel.yWorld) - rc->yWorld ;
    }
  }

static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22)
  {
  double xOld = obj->x ;
  double yOld = obj->y ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->transform (obj, m11, m12, m21, m22) ;

  // dots are neither rotated nor mirrored when the cell is

  qcad_cell_apply_transformation (QCAD_CELL (obj), xOld, yOld) ;
  }

static void qcad_cell_apply_transformation (QCADCell *cell, double xOld, double yOld)
  {
  QCADDesignObject *obj = QCAD_DESIGN_OBJECT (cell) ;
  double dx = obj->x - xOld, dy = obj->y - yOld ;

  cell->cell_dots[0].x += dx ;
  cell->cell_dots[1].x += dx ;
  cell->cell_dots[2].x += dx ;
  cell->cell_dots[3].x += dx ;

  cell->cell_dots[0].y += dy ;
  cell->cell_dots[1].y += dy ;
  cell->cell_dots[2].y += dy ;
  cell->cell_dots[3].y += dy ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), dx, dy) ;
  }

static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal)
  {
  QCADDesignObject *obj_from_parent =
    QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->hit_test (obj, xReal, yReal) ;

  return NULL != obj_from_parent ? obj_from_parent :
    NULL == QCAD_CELL (obj)->label ? NULL :
    qcad_design_object_hit_test (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), xReal, yReal) ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADDesignObject *obj = NULL ;
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;

  if (1 != event->button) return FALSE ;

#ifdef DESIGNER
  world_to_grid_pt (&xWorld, &yWorld) ;
#endif /* def DESIGNER */

  obj = QCAD_DESIGN_OBJECT (qcad_object_new_from_object (qcad_object_get_default (QCAD_TYPE_CELL))) ;
  qcad_design_object_move_to (obj, xWorld, yWorld) ;

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
#ifdef DESIGNER
static void qcad_cell_array_next_coord (int idx[2], double coord[2], double length[2], double dDir)
  {
  double coord_last, coord_old ;

  coord_last = coord[idx[0]] ;
  coord[idx[0]] += length[idx[0]] * dDir ;
  coord_old = coord[idx[0]] ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (coord[idx[0]] * dDir < coord_old * dDir)
    {
    coord[idx[0]] = coord_last + dDir * length[idx[0]] + 2 * (coord_old - coord[idx[0]]) ;
    coord_old = coord[idx[0]] ;
    world_to_grid_pt (&(coord[0]), &(coord[1])) ;
    }
  }
#endif /* def DESIGNER */
#endif /* def GTK_GUI */

double qcad_cell_calculate_polarization (QCADCell *cell)
  {
  // I have made an optimization here but it assumes that the cell polarization quality is perfect. //
  return (cell->cell_dots[0].charge - cell->cell_dots[1].charge) * OVER_QCHARGE;
  // The old way of calculating polarization assumes nothing about quality:
  // return ((cell->cell_dots[0].charge + cell->cell_dots[2].charge) - (cell->cell_dots[1].charge + cell->cell_dots[3].charge)) * ONE_OVER_FOUR_HALF_QCHARGE ;
  }

void qcad_cell_set_function (QCADCell *cell, QCADCellFunction function)
  {
  QCADCellFunction old_function = cell->cell_function ;

  if (old_function == function) return ;

  cell->cell_function = function ;
  if (QCAD_CELL_NORMAL == function)
    {
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &(clrClock[cell->cell_options.clock]), sizeof (GdkColor)) ;
    if (NULL != cell->label)
      qcad_cell_set_label (cell, "") ;
    }
  else
  if (QCAD_CELL_FIXED == function)
    {
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &clrOrange, sizeof (GdkColor)) ;
    if (NULL != cell->label)
      {
      memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clrOrange, sizeof (GdkColor)) ;
      qcad_cell_set_polarization (cell, qcad_cell_calculate_polarization (cell)) ;
      }
    }
  else
  if (QCAD_CELL_INPUT == function || QCAD_CELL_OUTPUT == function)
    {
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), (QCAD_CELL_INPUT == function) ? &clrBlue : &clrYellow, sizeof (GdkColor)) ;
    if (NULL != cell->label)
      memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), (QCAD_CELL_INPUT == function) ? &clrBlue : &clrYellow, sizeof (GdkColor)) ;
    }

  if (NULL != cell->label)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (cell->label), QCAD_DESIGN_OBJECT (cell)->bSelected) ;

  if (old_function != function)
    g_signal_emit (cell, qcad_cell_signals[QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL], 0) ;

  g_object_notify (G_OBJECT (cell), "function") ;
  }

void qcad_cell_scale (QCADCell *cell, double dScale, double dxOrigin, double dyOrigin)
  {
  double xOld = 0.0, yOld = 0.0 ;
  QCADDesignObject *obj = NULL ;

  if (NULL == cell) return ;

  obj = QCAD_DESIGN_OBJECT (cell) ;

  xOld = obj->bounding_box.xWorld ;
  yOld = obj->bounding_box.yWorld ;

  obj->bounding_box.xWorld = dScale * obj->bounding_box.xWorld - dxOrigin ;
  obj->bounding_box.yWorld = dScale * obj->bounding_box.yWorld - dyOrigin ;
  obj->bounding_box.cxWorld *= dScale ;
  obj->bounding_box.cyWorld *= dScale ;
  cell->cell_dots[0].x = dScale * cell->cell_dots[0].x - dxOrigin ;
  cell->cell_dots[0].y = dScale * cell->cell_dots[0].y - dyOrigin ;
  cell->cell_dots[0].diameter *= dScale ;
  cell->cell_dots[1].x = dScale * cell->cell_dots[1].x - dxOrigin ;
  cell->cell_dots[1].y = dScale * cell->cell_dots[1].y - dyOrigin ;
  cell->cell_dots[1].diameter *= dScale ;
  cell->cell_dots[2].x = dScale * cell->cell_dots[2].x - dxOrigin ;
  cell->cell_dots[2].y = dScale * cell->cell_dots[2].y - dyOrigin ;
  cell->cell_dots[2].diameter *= dScale ;
  cell->cell_dots[3].x = dScale * cell->cell_dots[3].x - dxOrigin ;
  cell->cell_dots[3].y = dScale * cell->cell_dots[3].y - dyOrigin ;
  cell->cell_dots[3].diameter *= dScale ;

  obj->x = obj->bounding_box.xWorld + obj->bounding_box.cxWorld / 2.0 ;
  obj->y = obj->bounding_box.yWorld + obj->bounding_box.cyWorld / 2.0 ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), obj->bounding_box.xWorld - xOld, obj->bounding_box.yWorld - yOld) ;
  }

void qcad_cell_set_clock (QCADCell *cell, int iClock)
  {
  if (cell->cell_options.clock == iClock) return ;

  cell->cell_options.clock = iClock ;
  if (QCAD_CELL_NORMAL == cell->cell_function)
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &(clrClock[iClock]), sizeof (GdkColor)) ;

  g_object_notify (G_OBJECT (cell), "clock") ;
  }

void qcad_cell_set_label (QCADCell *cell, char *pszLabel)
  {
  gboolean bNewLabel = FALSE ;

  if (NULL == pszLabel || NULL == cell) return ;

  if (0 == pszLabel[0])
    if (NULL != cell->label)
      {
      g_signal_emit_by_name (G_OBJECT (cell), "removed", cell->label) ;
      g_object_unref (cell->label) ;
      cell->bLabelRemoved = TRUE ;
      g_object_notify (G_OBJECT (cell), "label") ;
      return ;
      }

  if ((bNewLabel = (NULL == cell->label)))
    {
    cell->label = QCAD_LABEL (g_object_new (QCAD_TYPE_LABEL, NULL)) ;
    g_object_add_weak_pointer (G_OBJECT (cell->label), (gpointer *)&(cell->label)) ;
    g_signal_emit_by_name (G_OBJECT (cell), "added", cell->label) ;
    }

  memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &(QCAD_DESIGN_OBJECT (cell)->clr), sizeof (GdkColor)) ;
  qcad_label_set_text (cell->label, pszLabel) ;
  cell->bLabelRemoved = FALSE ;

  if (bNewLabel)
    qcad_design_object_move_to (QCAD_DESIGN_OBJECT (cell->label),
      QCAD_DESIGN_OBJECT (cell)->bounding_box.xWorld + QCAD_DESIGN_OBJECT (cell->label)->bounding_box.cxWorld / 2.0,
      QCAD_DESIGN_OBJECT (cell)->bounding_box.yWorld - QCAD_DESIGN_OBJECT (cell->label)->bounding_box.cyWorld / 2.0 - QCAD_CELL_LABEL_DEFAULT_OFFSET_Y) ;

  if (QCAD_DESIGN_OBJECT (cell)->bSelected)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (cell->label), TRUE) ;

  g_object_notify (G_OBJECT (cell), "label") ;
  }

void qcad_cell_set_polarization (QCADCell *cell, double new_polarization)
  {
  char *psz = NULL ;
  double half_charge_polarization = HALF_QCHARGE * new_polarization;

  cell->cell_dots[0].charge =
  cell->cell_dots[2].charge = half_charge_polarization + HALF_QCHARGE;
  cell->cell_dots[1].charge =
  cell->cell_dots[3].charge = HALF_QCHARGE - half_charge_polarization;

  if (QCAD_CELL_FIXED == cell->cell_function)
    {
    psz = g_strdup_printf ("%.2lf", new_polarization) ;
    qcad_cell_set_label (cell, psz) ;
    g_free (psz) ;
    }
  g_object_notify (G_OBJECT (cell), "polarization") ;
  }

#ifdef STDIO_FILEIO
static gboolean qcad_cell_dot_unserialize (FILE *fp, QCADCellDot *pdots, int idxDot)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bHaveDot = FALSE ;
  QCADCellDot dot = {0} ;

  if (!SkipPast (fp, '\0', "[TYPE:CELL_DOT]", NULL))
    return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;

    if (!strcmp (pszLine, "[#TYPE:CELL_DOT]"))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if ((bHaveDot = !strcmp (pszLine, "x")))
      dot.x = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "y")))
      dot.y = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "diameter")))
      dot.diameter = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "charge")))
      dot.charge = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "spin")))
      dot.spin = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "potential")))
      dot.potential = g_ascii_strtod (pszValue, NULL) ;

    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  if (!bHaveDot) return FALSE ;
  else
    memcpy (&(pdots[idxDot]), &dot, sizeof (QCADCellDot));

  return TRUE ;
  }

#endif /* STDIO_FILEIO */

// Calculate all coordinates given a cell x, y, cx, cy and a dot diam
static void qcad_cell_calculate_coords (QCADCell *cell)
  {
  QCADDesignObject *qcad_design_object = QCAD_DESIGN_OBJECT (cell) ;
  double
    x = qcad_design_object->x,
    y = qcad_design_object->y,
    dcx   = cell->cell_options.cxCell,
    dcy   = cell->cell_options.cyCell,
    ddiam = cell->cell_options.dot_diameter ;

  qcad_design_object->bounding_box.xWorld = x - dcx / 2.0 ;
  qcad_design_object->bounding_box.yWorld = y - dcy / 2.0 ;
  qcad_design_object->bounding_box.cxWorld = dcx ;
  qcad_design_object->bounding_box.cyWorld = dcy ;

  cell->cell_dots[0].diameter =
  cell->cell_dots[1].diameter =
  cell->cell_dots[2].diameter =
  cell->cell_dots[3].diameter = ddiam ;

  cell->cell_dots[3].x = x - (dcx / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[3].y = y - (dcy / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[0].x = x + (dcx / 2 - cell->cell_dots[0].diameter) / 2 + cell->cell_dots[0].diameter / 2;
  cell->cell_dots[0].y = y - (dcy / 2 - cell->cell_dots[0].diameter) / 2 - cell->cell_dots[0].diameter / 2;
  cell->cell_dots[2].x = x - (dcx / 2 - cell->cell_dots[2].diameter) / 2 - cell->cell_dots[2].diameter / 2;
  cell->cell_dots[2].y = y + (dcy / 2 - cell->cell_dots[2].diameter) / 2 + cell->cell_dots[2].diameter / 2;
  cell->cell_dots[1].x = x + (dcx / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;
  cell->cell_dots[1].y = y + (dcy / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;
  }

// Make sure the dots don't overlap when setting the cell size, and emit g_object_notify accordingly
static void qcad_cell_set_size (QCADCell *cell, double cx, double cy, double dot_diam)
  {
  double old_cx = cell->cell_options.cxCell, old_cy = cell->cell_options.cyCell, old_dot_diameter = dot_diam ;

  cell->cell_options.cxCell = cx ;
  cell->cell_options.cyCell = cy ;
  cell->cell_options.dot_diameter = MIN (dot_diam, MIN (cell->cell_options.cxCell, cell->cell_options.cyCell) / 2.0) ;

  if (old_cx != cell->cell_options.cxCell)
    g_object_notify (G_OBJECT (cell), "width") ;
  if (old_cy != cell->cell_options.cyCell)
    g_object_notify (G_OBJECT (cell), "height") ;
  if (old_dot_diameter != cell->cell_options.dot_diameter)
    g_object_notify (G_OBJECT (cell), "dot-diameter") ;
  qcad_cell_calculate_coords (cell) ;
  }
