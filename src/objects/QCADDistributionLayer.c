#include <stdlib.h>
#include <string.h>
#include "../intl.h"
#include "../simulation.h"
#include "QCADDistributionLayer.h"

typedef struct
  {
  double radius_of_effect ;
  double layer_separation ;
  } QCADDistributionLayerPrivate ;

enum
  {
  QCAD_DISTRIBUTION_LAYER_PROPERTY_FIRST = 1,
  QCAD_DISTRIBUTION_LAYER_PROPERTY_RADIUS_OF_EFFECT,
  QCAD_DISTRIBUTION_LAYER_PROPERTY_LAYER_SEPARATION,
  QCAD_DISTRIBUTION_LAYER_PROPERTY_LAST
  } ;

typedef struct
  {
  int icNeighbours ;
  QCADCell **neighbours ;
  double *neighbour_distances ;
  int idxChunk ;
  } DistributionData ;

#define QCAD_DISTRIBUTION_LAYER_GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE ((instance), QCAD_TYPE_DISTRIBUTION_LAYER, QCADDistributionLayerPrivate))

static void qcad_distribution_layer_class_init (QCADDistributionLayerClass *klass) ;
static void qcad_distribution_layer_instance_init (GObject *object) ;

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;

GType qcad_distribution_layer_get_type ()
  {
  static GType qcad_distribution_layer_type = 0 ;

  if (0 == qcad_distribution_layer_type)
    {
    static const GTypeInfo qcad_distribution_layer_info =
      {
      sizeof (QCADDistributionLayerClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_distribution_layer_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADDistributionLayer),
      0,
      (GInstanceInitFunc)qcad_distribution_layer_instance_init
      } ;

    if ((qcad_distribution_layer_type = g_type_register_static (QCAD_TYPE_LAYER, QCAD_TYPE_STRING_DISTRIBUTION_LAYER, &qcad_distribution_layer_info, 0)))
      g_type_class_ref (qcad_distribution_layer_type) ;
    }

  return qcad_distribution_layer_type ;
  }

static void qcad_distribution_layer_class_init (QCADDistributionLayerClass *klass)
  {
#ifdef PROPERTY_UIS
  // Gotta be static so the strings don't die
  static QCADPropertyUIProperty properties[] =
    {
    {NULL,               "title",     {0, }},
    {"radius-of-effect", "units",     {0, }},
    {"layer-separation", "units",     {0, }},
    } ;

  // cell.title = "QCA Cell"
  g_value_set_string (g_value_init (&(properties[0].ui_property_value), G_TYPE_STRING), _("Distribution Layer")) ;
  // cell.width.units = "nm"
  g_value_set_string (g_value_init (&(properties[1].ui_property_value), G_TYPE_STRING), "nm") ;
  // cell.height.units = "nm"
  g_value_set_string (g_value_init (&(properties[2].ui_property_value), G_TYPE_STRING), "nm") ;

  qcad_object_class_install_ui_properties (QCAD_OBJECT_CLASS (klass), properties, G_N_ELEMENTS (properties)) ;
#endif /* def PROPERTY_UIS */

  G_OBJECT_CLASS (klass)->set_property = set_property ;
  G_OBJECT_CLASS (klass)->get_property = get_property ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_DISTRIBUTION_LAYER_PROPERTY_RADIUS_OF_EFFECT,
    g_param_spec_double ("radius-of-effect", _("Radius Of Effect"), _("Radius limiting mutual cell influence"),
      1.0, 1e6, 65.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_DISTRIBUTION_LAYER_PROPERTY_LAYER_SEPARATION,
    g_param_spec_double ("layer-separation", _("Layer Separation"), _("Vertical separation between layers"),
      1.0, 1e6, 11.5, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_type_class_add_private (klass, sizeof (QCADDistributionLayerPrivate)) ;
  }

int compare_cell_neighbour_counts (const void *p1, const void *p2)
  {
  int icNeighbours1 = ((DistributionData *)((*((QCADCell **)p1))->cell_model))->icNeighbours, 
      icNeighbours2 = ((DistributionData *)((*((QCADCell **)p2))->cell_model))->icNeighbours ;

  // The more neighbours, the "smaller" the cell, because we want the cells with the most neighbours first
  return (icNeighbours1 > icNeighbours2) ? -1 :
         (icNeighbours1 < icNeighbours2) ?  1 : 0 ;
  }

void qcad_distribution_layer_generate_distribution (QCADDistributionLayer *layer, DESIGN *design)
  {
  QCADCell *the_current_cell = NULL ;
  int idxLayer = -1, idxCell = -1, idxDst = -1, idxNeighbour = -1, idxChunk = 0 ;
  int number_of_cell_layers = 0, *number_of_cells_in_layer = NULL, total_number_of_cells = 0 ;
  QCADCell ***sorted_cells = NULL, **sorted_cells_flat_list = NULL ;
  DistributionData *the_current_cell_model = NULL, *the_neighbour_cell_model = NULL ;
  QCADDistributionLayerPrivate *private = NULL ;
  int icCurrentNeighbours = -1 ;

  if (NULL == layer || NULL == design) return ;
  if (NULL == (private = QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (layer))) return ;

  simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  // Find each cell's neighbours
  for (idxLayer = 0 ; idxLayer < number_of_cell_layers ; idxLayer++)
    {
    total_number_of_cells += number_of_cells_in_layer[idxLayer] ;
    for (idxCell = 0 ; idxCell < number_of_cells_in_layer[idxLayer] ; idxCell++)
      {
      the_current_cell = QCAD_CELL (sorted_cells[idxLayer][idxCell]) ;
      the_current_cell->cell_model = g_malloc0 (sizeof (DistributionData)) ;
      the_current_cell_model = ((DistributionData *)(the_current_cell->cell_model)) ;
      the_current_cell_model->icNeighbours =
        select_cells_in_radius (sorted_cells, the_current_cell, private->radius_of_effect, idxLayer, number_of_cell_layers, number_of_cells_in_layer, private->layer_separation, 
          &(the_current_cell_model->neighbours), NULL, &(the_current_cell_model->neighbour_distances)) ;
      // The resulting list of neighbours should be sorted in increasing order of distance
      the_current_cell_model->idxChunk = -1 ;
      }
    }

  // Create the sorted cells flat list and sort by decreasing number of neighbours
  sorted_cells_flat_list = g_malloc0 (total_number_of_cells * sizeof (QCADCell *)) ;
  for (idxLayer = 0, idxDst = 0 ; idxLayer < number_of_cell_layers ; idxLayer++, idxDst += number_of_cells_in_layer[idxLayer])
    memcpy (&sorted_cells_flat_list[idxDst], sorted_cells[idxLayer], number_of_cells_in_layer[idxLayer] * sizeof (QCADCell *)) ;
  qsort (sorted_cells_flat_list, total_number_of_cells, sizeof (QCADCell *), compare_cell_neighbour_counts) ;

  icCurrentNeighbours = ((DistributionData *)(sorted_cells_flat_list[0]->cell_model))->icNeighbours ;
  for (idxCell = 0 ; ; )
    {
    if ((the_current_cell_model = ((DistributionData *)(sorted_cells_flat_list[0]->cell_model)))->icNeighbours < icCurrentNeighbours) 
      break ;
    the_current_cell_model->idxChunk = idxChunk++ ;

    // For the nearest neighbours
    for (idxNeighbour = 0 ; (the_current_cell_model->neighbour_distances[idxNeighbour] == 
                             the_current_cell_model->neighbour_distances[0]) && 
                            (idxNeighbour < the_current_cell_model->icNeighbours)          ; idxNeighbour++)
      if (the_current_cell_model->icNeighbours == 
        (the_neighbour_cell_model = ((DistributionData *)(the_current_cell_model->neighbours[idxNeighbour]->cell_model)))->icNeighbours)
        the_neighbour_cell_model->idxChunk = the_current_cell_model->idxChunk ;
      else
        break ;
    }

  g_print ("We have %d chunks\n", idxChunk) ;
/*
  for (idxCell = 0 ; idxCell < total_number_of_cells ; )
    if (NULL != (the_current_cell_model = ((DistributionData *)(sorted_cells_flat_list[0]->cell_model)))->neighbours)
      {
      (the_current_cell_model = ((DistributionData *)(sorted_cells_flat_list[0]->cell_model)))->idxChunk = idxChunk ;
      // While we're still doing the most connected cells, we start new chunks
      if (icCurrentNeighbours == the_current_cell_model->icNeighbours)
        idxChunk++ ;

      // For the nearest neighbours
      for (idxNeighbour = 0 ; the_current_cell_model->neighbour_distances[idxNeighbour] == 
                              the_current_cell_model->neighbour_distances[0]               ; idxNeighbour++)
        {
        
        }
        
      }
    else
      {
      // If the cell has no neighbours, add it to whatever chunk we're currently building
      the_current_cell_model->idxChunk = idxChunk ;
      idxCell-- ;
      }
*/
  g_free (sorted_cells_flat_list) ;
  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;
  }

static void qcad_distribution_layer_instance_init (GObject *object)
  {
  QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->radius_of_effect = 65.0 ;
  QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->layer_separation = 11.5 ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_DISTRIBUTION_LAYER_PROPERTY_RADIUS_OF_EFFECT:
      QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->radius_of_effect = g_value_get_double (value) ;
        break ;

    case QCAD_DISTRIBUTION_LAYER_PROPERTY_LAYER_SEPARATION:
      QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->layer_separation = g_value_get_double (value) ;
        break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  switch (property_id)
    {
    case QCAD_DISTRIBUTION_LAYER_PROPERTY_RADIUS_OF_EFFECT:
      g_value_set_double (value, QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->radius_of_effect) ;
        break ;

    case QCAD_DISTRIBUTION_LAYER_PROPERTY_LAYER_SEPARATION:
      g_value_set_double (value, QCAD_DISTRIBUTION_LAYER_GET_PRIVATE (object)->layer_separation) ;
        break ;
    }
  }
