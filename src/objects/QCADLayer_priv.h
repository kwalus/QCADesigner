#ifndef _OBJECTS_QCADLayer_PRIV_H_
#define _OBJECTS_QCADLayer_PRIV_H_

#include <glib.h>
#include "QCADLayer.h"

GHashTable *qcad_layer_create_default_properties (LayerType type) ;
GHashTable *qcad_layer_free_default_properties (GHashTable *ht) ;

#endif /* def _OBJECTS_QCADLayer_PRIV_H_ */
