#include "../support.h"
#include "QCADUndoEntryObjectState.h"

typedef struct
  {
  char *pszName ;
  GValue val_before ;
  GValue val_after ;
  gboolean bChanged ;
  gulong notify_id ;
  } GParameterDelta ;

enum
  {
  QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_FIRST = 1,
  QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_INSTANCE,
  QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_FROZEN,
  QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_LAST
  } ;

static void qcad_undo_entry_object_state_instance_init (GObject *obj, gpointer data) ;
static void qcad_undo_entry_object_state_class_init (GObjectClass *obj, gpointer data) ;

static void finalize     (GObject *obj) ;
static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void get_property (GObject *object, guint property_id,       GValue *value, GParamSpec *pspec) ;
static void fire (QCADUndoEntry *undo_entry, gboolean bUndo) ;

static void instance_notify (GObject *instance, GParamSpec *pspec, gpointer data) ;

static void qcad_undo_entry_object_state_set_frozen (QCADUndoEntryObjectState *undo_entry, gboolean bFrozen) ;
static void qcad_undo_entry_object_state_set_instance (QCADUndoEntryObjectState *undo_entry, GObject *instance) ;

GType qcad_undo_entry_object_state_get_type ()
  {
  static GType the_type = 0 ;

  if (0 == the_type)
    {
    static GTypeInfo the_type_info = 
      {
      sizeof (QCADUndoEntryClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_undo_entry_object_state_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADUndoEntry),
      0,
      (GInstanceInitFunc)qcad_undo_entry_object_state_instance_init
      } ;

    if ((the_type = g_type_register_static (QCAD_TYPE_UNDO_ENTRY, QCAD_TYPE_STRING_UNDO_ENTRY_OBJECT_STATE, &the_type_info, 0)))
      g_type_class_ref (the_type) ;
    }

  return the_type ;
  }

static void qcad_undo_entry_object_state_class_init (GObjectClass *klass, gpointer data)
  {
  klass->finalize = finalize ;
  klass->set_property = set_property ;
  klass->get_property = get_property ;

  QCAD_UNDO_ENTRY_CLASS (klass)->fire = fire ;

  g_object_class_install_property (klass, QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_INSTANCE,
    g_param_spec_pointer ("instance", _("Instance"), _("Instance to track state changes for"), 
      G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (klass, QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_INSTANCE,
    g_param_spec_boolean ("frozen", _("Frozen"), _("Indicates whether the entry records state changes"), 
      TRUE, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;
  }

static void qcad_undo_entry_object_state_instance_init (GObject *obj, gpointer data)
  {
  QCAD_UNDO_ENTRY_OBJECT_STATE (obj)->instance         = NULL ;
  QCAD_UNDO_ENTRY_OBJECT_STATE (obj)->parameters_delta = NULL ;
  QCAD_UNDO_ENTRY_OBJECT_STATE (obj)->bFrozen          = TRUE ;
  }

gboolean qcad_undo_entry_object_state_get_changed (QCADUndoEntryObjectState *undo_entry_os)
  {
  int Nix ;
  gboolean bChanged = FALSE ;

  for (Nix = 0 ; Nix < undo_entry_os->parameters_delta->icUsed ; Nix++)
    if ((bChanged = exp_array_index_1d (undo_entry_os->parameters_delta, GParameterDelta, Nix).bChanged))
      break ;

  return bChanged ;
  }

static void set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADUndoEntryObjectState *undo_entry = QCAD_UNDO_ENTRY_OBJECT_STATE (object) ;
  switch (property_id)
    {
    case QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_INSTANCE:
      qcad_undo_entry_object_state_set_instance (undo_entry, g_value_get_pointer (value)) ;
      break ;

    case QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_FROZEN:
      qcad_undo_entry_object_state_set_frozen (undo_entry, g_value_get_boolean (value)) ;
      break ;
    }
  }

static void get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADUndoEntryObjectState *undo_entry = QCAD_UNDO_ENTRY_OBJECT_STATE (object) ;
  switch (property_id)
    {
    case QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_INSTANCE:
      g_value_set_pointer (value, undo_entry->instance) ;
      break ;

    case QCAD_UNDO_ENTRY_OBJECT_STATE_PROPERTY_FROZEN:
      qcad_undo_entry_object_state_set_frozen (undo_entry, g_value_get_boolean (value)) ;
      break ;
    }
  }

static void finalize (GObject *obj)
  {qcad_undo_entry_object_state_set_instance (QCAD_UNDO_ENTRY_OBJECT_STATE (obj), NULL) ;}

static void fire (QCADUndoEntry *undo_entry, gboolean bUndo)
  {
  GParameterDelta *parameter_delta = NULL ;
  int Nix ;
  QCADUndoEntryObjectState *undo_entry_os = QCAD_UNDO_ENTRY_OBJECT_STATE (undo_entry) ;

  // Object must have an instance to undo state for, and it must be frozen
  if (NULL != undo_entry_os->instance && undo_entry_os->bFrozen) 
    for (Nix = 0 ; Nix < undo_entry_os->parameters_delta->icUsed ; Nix++)
      if ((parameter_delta = &exp_array_index_1d (undo_entry_os->parameters_delta, GParameterDelta, Nix))->bChanged)
        g_object_set_property (G_OBJECT (undo_entry_os->instance), parameter_delta->pszName, 
          bUndo ? &(parameter_delta->val_before) : &(parameter_delta->val_after)) ;

  // Call parent class' method
  QCAD_UNDO_ENTRY_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_UNDO_ENTRY_OBJECT_STATE)))->fire (undo_entry, bUndo) ;
  }

static void instance_notify (GObject *instance, GParamSpec *pspec, gpointer data)
  {
  GParameterDelta *parameter_delta = (GParameterDelta *)data ;

  g_object_get_property (instance, parameter_delta->pszName, &(parameter_delta->val_after)) ;

  parameter_delta->bChanged = 
    (0 == g_param_values_cmp (pspec, &(parameter_delta->val_after), &(parameter_delta->val_before))) ;
  }

static void qcad_undo_entry_object_state_set_instance (QCADUndoEntryObjectState *undo_entry, GObject *instance)
  {
  GParameterDelta *parameter_delta = NULL ;
  int Nix ;

  if (undo_entry->instance == instance) return ;

  if (NULL != undo_entry->instance)
    {
    qcad_undo_entry_object_state_set_frozen (undo_entry, TRUE) ;
    for (Nix = 0 ; Nix < undo_entry->parameters_delta->icUsed ; Nix++)
      {
      parameter_delta = &exp_array_index_1d (undo_entry->parameters_delta, GParameterDelta, Nix) ;
      g_free (parameter_delta->pszName) ;
      g_value_unset (&(parameter_delta->val_before)) ;
      g_value_unset (&(parameter_delta->val_after)) ;
      }
    exp_array_remove_vals (undo_entry->parameters_delta, 1, 0, undo_entry->parameters_delta->icUsed) ;
    g_object_unref (undo_entry->instance) ;
    undo_entry->instance = NULL ;
    }

  if (NULL == instance)
    undo_entry->parameters_delta = exp_array_free (undo_entry->parameters_delta) ;
  else
    {
    int icProperties = 0 ;
    GParamSpec **class_properties = NULL ;

    class_properties = g_object_class_list_properties (G_OBJECT_GET_CLASS (instance), &icProperties) ;

    if (0 == icProperties)
      undo_entry->parameters_delta = exp_array_free (undo_entry->parameters_delta) ;
    else    
      {
      undo_entry->instance = g_object_ref (instance) ;

      if (NULL == undo_entry->parameters_delta)
        undo_entry->parameters_delta = exp_array_new (sizeof (GParameterDelta), 1) ;
      exp_array_1d_insert_vals (undo_entry->parameters_delta, NULL, icProperties, -1) ;

      for (Nix = 0 ; Nix < icProperties ; Nix++)
        {
        parameter_delta = &exp_array_index_1d (undo_entry->parameters_delta, GParameterDelta, Nix) ;
        parameter_delta->pszName = g_strdup (g_param_spec_get_name (class_properties[Nix])) ;
        g_value_init (&(parameter_delta->val_before), G_PARAM_SPEC_VALUE_TYPE (class_properties[Nix])) ;
        g_object_get_property (instance, g_param_spec_get_name (class_properties[Nix]), &(parameter_delta->val_before)) ;
        g_value_init (&(parameter_delta->val_after), G_PARAM_SPEC_VALUE_TYPE (class_properties[Nix])) ;
        g_value_copy (&(parameter_delta->val_before), &(parameter_delta->val_after)) ;
        parameter_delta->bChanged = FALSE ;
        }
      qcad_undo_entry_object_state_set_frozen (undo_entry, FALSE) ;
      }
    }

  g_object_notify (G_OBJECT (undo_entry), "instance") ;
  }

static void qcad_undo_entry_object_state_set_frozen (QCADUndoEntryObjectState *undo_entry, gboolean bFrozen)
  {
  char *psz = NULL ;
  int Nix ;
  GParameterDelta *parameter_delta = NULL ;

  if (undo_entry->bFrozen == bFrozen) return ;
  if (NULL == undo_entry->instance) return ;

  if (bFrozen)
    for (Nix = 0 ; Nix < undo_entry->parameters_delta->icUsed ; Nix++)
      g_signal_handler_disconnect (G_OBJECT (undo_entry->instance), 
        exp_array_index_1d (undo_entry->parameters_delta, GParameterDelta, Nix).notify_id) ;
  else
    for (Nix = 0 ; Nix < undo_entry->parameters_delta->icUsed ; Nix++)
      {
      parameter_delta = &exp_array_index_1d (undo_entry->parameters_delta, GParameterDelta, Nix) ;
      parameter_delta->notify_id =
        g_signal_connect (G_OBJECT (undo_entry->instance), psz = g_strdup_printf ("notify::%s", parameter_delta->pszName),
          (GCallback)instance_notify, parameter_delta) ;
      }

  undo_entry->bFrozen = bFrozen ;
  g_object_notify (G_OBJECT (undo_entry), "frozen") ;
  }
