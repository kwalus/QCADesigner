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
// The layer properties dialog. This allows the user to //
// set the name of the layer, set the status of the la- //
// yer, and set the default properties for the various  //
// object types admissible in the layer.                //
//                                                      //
//////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <string.h>
#include "../support.h"
#include "QCADLayer_properties.h"
#include "../design.h"
#include "../custom_widgets.h"
#include "QCADLayer.h"

#define DBG_LPD_FLAGS(s)
#define DBG_LPD_PROPS(s)

typedef struct
 {
 GtkWidget *dlg ;
 GtkWidget *lblLayerType ;
 GtkWidget *cbLayerStatus ;
 GtkWidget *txtLayerDescription ;
 GtkWidget *swObjs ;
 GtkWidget *lstObjs ;
 GtkWidget *tblObjUI ;
 GHashTable *htObjUI ;
 GHashTable *htObjCB ;
 GHashTable *htObjCBData ;
 } layer_properties_D ;

static char *pszLayerTypes[] = 
  {
  "Substrate",
  "Cells",
  "Clocking",
  "Drawing",
  "Distribution"
  } ;

char *pszLayerStati[] =
  {
  "Active",
  "Visible",
  "Hidden"
  } ;

static layer_properties_D layer_properties_dialog ;

static void create_layer_properties_dialog (layer_properties_D *dialog) ;
static void build_default_properties_hash_tables (GType type, GtkWidget *tblObjUI, int *pidx, GHashTable *htObjUI, GHashTable *htObjCB, GHashTable *htObjCBData) ;
static LayerType layer_type_from_description (char *pszDescription) ;

static void reflect_layer_type (GtkWidget *widget, gpointer data) ;
static void object_type_selected (GtkWidget *widget, gpointer data) ;
static void hide_object_UIs (gpointer key, gpointer value, gpointer data) ;

#ifdef UNDO_REDO
gboolean qcad_layer_properties (QCADDesignObject *obj, GtkWidget *widget, QCADUndoEntry **pentry)
#else
gboolean qcad_layer_properties (QCADDesignObject *obj, GtkWidget *widget)
#endif
  {
  gboolean bRet = FALSE ;
  char *pszText = NULL ;
  GList *llItr = NULL ;
  void (*cb) (void *) = NULL ;
  void *current_default_properties = NULL,
       *layer_default_properties = NULL ;
  QCADDesignObjectClass *klass = NULL ;
  QCADLayer *layer = QCAD_LAYER (obj) ;

#ifdef UNDO_REDO
  if (NULL != pentry)
    (*pentry) = NULL ;
#endif

  if (NULL == layer_properties_dialog.dlg)
    create_layer_properties_dialog (&layer_properties_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (layer_properties_dialog.dlg), GTK_WINDOW (widget)) ;

  gtk_label_set_text (GTK_LABEL (layer_properties_dialog.lblLayerType), _(pszLayerTypes[layer->type])) ;
  gtk_list_select_item (GTK_LIST (GTK_COMBO (layer_properties_dialog.cbLayerStatus)->list), layer->status) ;
  gtk_entry_set_text (GTK_ENTRY (layer_properties_dialog.txtLayerDescription), NULL == layer->pszDescription ? "" : layer->pszDescription) ;

  reflect_layer_type (NULL, &layer_properties_dialog) ;

  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (layer_properties_dialog.dlg)))))
    {
    if (NULL != layer->pszDescription)
      g_free (layer->pszDescription) ;
    layer->pszDescription = gtk_editable_get_chars (GTK_EDITABLE (layer_properties_dialog.txtLayerDescription), 0, -1) ;

    layer->status =
      layer_status_from_description (pszText =
        gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (layer_properties_dialog.cbLayerStatus)->entry), 0, -1)) ;
    g_free (pszText) ;

//    set_layer_type (GTK_WINDOW (widget), layer,
//      layer_type_from_description (pszText =
//        gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (layer_properties_dialog.cbLayerType)->entry), 0, -1))) ;
//    g_free (pszText) ;

    // Grab the layer-specific default properties for each object type in this layer
    for (llItr = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)(layer->type)) ; llItr != NULL ; llItr = llItr->next)
      {
      if (NULL != (cb = (void (*) (void *))g_hash_table_lookup (layer_properties_dialog.htObjCB, llItr->data)))
        {
        klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek ((GType)(llItr->data))) ;
        current_default_properties = qcad_design_object_class_get_properties (klass) ;
        (*cb) (g_hash_table_lookup (layer_properties_dialog.htObjCBData, llItr->data)) ;
        if (NULL != (layer_default_properties = g_hash_table_lookup (layer->default_properties, llItr->data)))
          qcad_design_object_class_destroy_properties (klass, layer_default_properties) ;
        g_hash_table_replace (layer->default_properties, llItr->data, qcad_design_object_class_get_properties (klass)) ;
        qcad_design_object_class_set_properties (klass, current_default_properties) ;
        }
      }

    DBG_LPD_FLAGS (fprintf (stderr, "get_layer_propeties_from_user:layer->status = %d, layer->type = %d\n",
      layer->status, layer->type)) ;
    }

  gtk_widget_hide (layer_properties_dialog.dlg) ;

  return bRet ;
  }

static LayerType layer_type_from_description (char *pszDescription)
  {
  int Nix ;

  for (Nix = 0 ; Nix < LAYER_TYPE_LAST_TYPE ; Nix++)
    if (!strcmp (pszDescription, _(pszLayerTypes[Nix])))
      return Nix ;

  return LAYER_TYPE_LAST_TYPE ;
  }

LayerStatus layer_status_from_description (char *pszDescription)
  {
  int Nix ;

  for (Nix = 0 ; Nix < LAYER_STATUS_LAST_STATUS ; Nix++)
    if (!strcmp (pszDescription, _(pszLayerStati[Nix])))
      return Nix ;

  return LAYER_STATUS_LAST_STATUS ;
  }

static void create_layer_properties_dialog (layer_properties_D *dialog)
  {
  int Nix, idx = 0 ;
  GtkWidget
    *tbl = NULL,
    *tblMain = NULL,
    *frm = NULL,
    *lbl = NULL ;
  GList *lstComboItems = NULL ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Entering\n")) ;

  dialog->dlg = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlg), _("Layer Properties")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlg), TRUE) ;

  tblMain = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tblMain) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlg)->vbox), tblMain, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblMain), 2) ;

  tbl = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_table_attach (GTK_TABLE (tblMain), tbl, 0, 1, 0, 1,
    (GtkAttachOptions)0,
    (GtkAttachOptions)0, 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  lbl = gtk_label_new (_("Layer Type:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->lblLayerType = gtk_label_new ("") ;
  gtk_widget_show (dialog->lblLayerType) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->lblLayerType, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblLayerType), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblLayerType), 0.0, 0.5) ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  DBG_LPD_PROPS (fprintf (stderr, "create_layer_properties_dialog:Continuing...\n")) ;

  lbl = gtk_label_new (_("Layer Status:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->cbLayerStatus = gtk_combo_new () ;
  gtk_widget_show (dialog->cbLayerStatus) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->cbLayerStatus, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_combo_set_value_in_list (GTK_COMBO (dialog->cbLayerStatus), TRUE, FALSE) ;
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (dialog->cbLayerStatus)->entry), FALSE) ;
  lstComboItems = NULL ;
  for (Nix = 0 ; Nix < LAYER_STATUS_LAST_STATUS ; Nix++)
    lstComboItems = g_list_append (lstComboItems, _(pszLayerStati[Nix])) ;
  gtk_combo_set_popdown_strings (GTK_COMBO (dialog->cbLayerStatus), lstComboItems) ;

  lbl = gtk_label_new (_("Layer Description:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->txtLayerDescription = gtk_entry_new () ;
  gtk_widget_show (dialog->txtLayerDescription) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->txtLayerDescription, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->txtLayerDescription), TRUE) ;

  frm = gtk_frame_new (_("Object Properties")) ;
  gtk_widget_show (frm) ;
  gtk_table_attach (GTK_TABLE (tblMain), frm, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (frm), 2) ;

  tbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (frm), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  dialog->swObjs = gtk_scrolled_window_new (
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 1)),
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 1))) ;
  gtk_widget_show (dialog->swObjs) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->swObjs, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->swObjs), 2) ;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dialog->swObjs), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC) ;

  dialog->lstObjs = gtk_list_new () ;
  gtk_widget_show (dialog->lstObjs) ;
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (dialog->swObjs), dialog->lstObjs) ;

  dialog->tblObjUI = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (dialog->tblObjUI) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->tblObjUI, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;

  dialog->htObjUI = g_hash_table_new (NULL, NULL) ;
  dialog->htObjCB = g_hash_table_new (NULL, NULL) ;
  dialog->htObjCBData = g_hash_table_new (NULL, NULL) ;

  lstComboItems = NULL ;

  // Fill the hash tables with the UIs and callbacks, respectively, for each of the classes
  build_default_properties_hash_tables (QCAD_TYPE_DESIGN_OBJECT, dialog->tblObjUI, &idx, dialog->htObjUI, dialog->htObjCB, dialog->htObjCBData) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlg), GTK_RESPONSE_OK) ;
  }

static void reflect_layer_type (GtkWidget *widget, gpointer data)
  {
  GtkWidget *li = NULL ;
  layer_properties_D *dialog = (layer_properties_D *)data ;
  char *pszLayerType = (char *)gtk_label_get_text (GTK_LABEL (dialog->lblLayerType)) ;
  const char *pszObjName = NULL ;
  LayerType layer_type = layer_type_from_description (pszLayerType) ;
  GList *llObjs = g_hash_table_lookup (qcad_layer_object_containment_rules (), (gpointer)layer_type),
        *llItr = NULL, *llLstItems = NULL ;

  // Add the appropriate entries to the object list combo
  gtk_list_clear_items (GTK_LIST (dialog->lstObjs), 0, -1) ;

  g_hash_table_foreach (dialog->htObjUI, hide_object_UIs, NULL) ;

  for (llItr = llObjs ; llItr != NULL ; llItr = llItr->next)
    if (NULL != (pszObjName = g_type_name ((GType)(llItr->data))))
      {
      li = gtk_list_item_new_with_label (pszObjName) ;
      gtk_widget_show (li) ;
      llLstItems = g_list_prepend (llLstItems, li) ;
      g_signal_connect (GTK_LIST_ITEM (li), "select", (GCallback)object_type_selected, g_hash_table_lookup (dialog->htObjUI, (gpointer)llItr->data)) ;
      }

  if (NULL != llLstItems)
    {
    gtk_list_append_items (GTK_LIST (dialog->lstObjs), llLstItems) ;
    gtk_list_item_select (GTK_LIST_ITEM (llLstItems->data)) ;
    }

  scrolled_window_set_size (GTK_SCROLLED_WINDOW (dialog->swObjs), dialog->lstObjs, 0.4, 0.4) ;
  }

static void object_type_selected (GtkWidget *widget, gpointer data)
  {
  if (NULL != data)
    gtk_widget_show (GTK_WIDGET (data)) ;
  }

static void hide_object_UIs (gpointer key, gpointer value, gpointer data)
  {gtk_widget_hide (GTK_WIDGET (value)) ;}

static void build_default_properties_hash_tables (GType type, GtkWidget *tblObjUI, int *pidx, GHashTable *htObjUI, GHashTable *htObjCB, GHashTable *htObjCBData)
  {
  int Nix ;
  int icChildren = 0 ;
  GType *types = NULL ;
  GCallback cbObj = NULL ;
  GtkWidget *objUI = NULL ;
  gpointer data = NULL ;
  QCADDesignObjectClass *klass = NULL ;

  types = g_type_children (type, &icChildren) ;

  if (NULL != (klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (type))))
    {
    // FIXME: Pass current layer-stored properties to this function, so the UI may properly init itself
//    fprintf (stderr, "create_layer_properties_dialog: Adding UI for type \"%s\"\n", g_type_name (type)) ;
    cbObj = qcad_design_object_class_get_properties_ui (klass, NULL, &objUI, &data) ;
    if (!(NULL == cbObj || NULL == objUI))
      {
      g_hash_table_insert (htObjUI, (gpointer)type, objUI) ;
      g_hash_table_insert (htObjCB, (gpointer)type, (gpointer)cbObj) ;
      g_hash_table_insert (htObjCBData, (gpointer)type, data) ;

      gtk_table_attach (GTK_TABLE (tblObjUI), objUI, 0, 1, (*pidx), (*pidx) + 1,
        (GtkAttachOptions)0,
        (GtkAttachOptions)0, 2, 2) ;
      (*pidx)++ ;
      }
    }

  for (Nix = 0 ; Nix < icChildren && 0 != types[Nix] ; Nix++)
/*
    {
    cbObj = NULL ;
    objUI = NULL ;
    if (NULL != (klass = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (types[Nix]))))
      {
      // FIXME: Pass current layer-stored properties to this function, so the UI may properly init itself
      fprintf (stderr, "create_layer_properties_dialog: Adding UI for type \"%s\"\n", g_type_name (types[Nix])) ;
      cbObj = qcad_design_object_class_get_properties_ui (klass, NULL, &objUI, &data) ;
      if (!(NULL == cbObj || NULL == objUI))
        {
        g_hash_table_insert (htObjUI, (gpointer)types[Nix], objUI) ;
        g_hash_table_insert (htObjCB, (gpointer)types[Nix], (gpointer)cbObj) ;
        g_hash_table_insert (htObjCBData, (gpointer)types[Nix], data) ;

        gtk_table_attach (GTK_TABLE (tblObjUI), objUI, 0, 1, (*pidx), (*pidx) + 1,
          (GtkAttachOptions)0,
          (GtkAttachOptions)0, 2, 2) ;
        (*pidx)++ ;
        }
      }
*/
    build_default_properties_hash_tables (types[Nix], tblObjUI, pidx, htObjUI, htObjCB, htObjCBData) ;
//    }

  g_free (types) ;
  }
