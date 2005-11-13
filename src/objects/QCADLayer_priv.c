#include "QCADLayer_priv.h"

static void free_default_properties (gpointer key, gpointer value, gpointer user_data) ;

GHashTable *qcad_layer_create_default_properties (LayerType type)
  {
  GHashTable *props = NULL ;
  GList *llItr = NULL ;

  props = g_hash_table_new (NULL, NULL) ;
  for (llItr = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)(type)) ; llItr != NULL ; llItr = llItr->next)
    g_hash_table_insert (props, llItr->data, qcad_design_object_class_get_properties (QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek ((GType)(llItr->data))))) ;

  return props ;
  }

// This function destroys the default_properies hash table and always returns NULL
GHashTable *qcad_layer_free_default_properties (GHashTable *ht)
  {
  g_hash_table_foreach (ht, free_default_properties, NULL) ;
  g_hash_table_destroy (ht) ;
  return NULL ;
  }

static void free_default_properties (gpointer key, gpointer value, gpointer user_data)
  {if (NULL != value) g_free (value) ;}
