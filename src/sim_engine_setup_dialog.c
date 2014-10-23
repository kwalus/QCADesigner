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
// Contents:                                            //
//                                                      //
// The simulation engine setup dialog. This dialog      //
// allows the user to choose from among the available   //
// simulation engines.                                  //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gtk/gtk.h>
#include "support.h"
#include "global_consts.h"
#include "sim_engine_setup_dialog.h"
#include "coherence_vector_properties_dialog.h"
#include "bistable_properties_dialog.h"
#include "semi_coherent_properties_dialog.h"
#include "ts_coherence_vector_properties_dialog.h"
#include "ts_fc_properties_dialog.h"

typedef struct
  {
  GtkWidget *sim_engine_setup_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GSList    *vbox1_group;
  GtkWidget *bistable_radio;
  GtkWidget *coherence_radio;
  GtkWidget *semi_coherent_radio;
  GtkWidget *ts_coherence_radio;
  GtkWidget *ts_fc_radio;  
  GtkWidget *digital_radio;
  GtkWidget *scqca_radio;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbox1;
  GtkWidget *sim_engine_ok_button;
  GtkWidget *sim_engine_cancel_button;
  GtkWidget *options_button ;
  } sim_engine_setup_D;

extern bistable_OP bistable_options ;
extern coherence_OP coherence_options ;
extern ts_coherence_OP ts_coherence_options ;
#ifdef HAVE_FORTRAN
extern semi_coherent_OP semi_coherent_options ;
extern ts_fc_OP ts_fc_options ;
#endif /* HAVE_FORTRAN */

static sim_engine_setup_D sim_engine_setup_dialog = {NULL} ;

static void create_sim_engine_dialog(sim_engine_setup_D *dialog);
static void options_button_clicked(GtkButton *button, gpointer user_data);
static void engine_toggled (GtkWidget *button,gpointer user_data);

void get_sim_engine_from_user (GtkWindow *parent, int *piSimEng)
  {
  if (NULL == sim_engine_setup_dialog.sim_engine_setup_dialog)
    create_sim_engine_dialog (&sim_engine_setup_dialog) ;
  gtk_window_set_transient_for (GTK_WINDOW (sim_engine_setup_dialog.sim_engine_setup_dialog), parent) ;

  g_object_set_data (G_OBJECT (sim_engine_setup_dialog.options_button), "which_options", (gpointer)*piSimEng) ;

  if (COHERENCE_VECTOR == *piSimEng)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.coherence_radio), TRUE) ;
    gtk_widget_set_sensitive (sim_engine_setup_dialog.options_button, TRUE) ;
    }
  else    
  if (BISTABLE == *piSimEng)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.bistable_radio), TRUE) ;
    gtk_widget_set_sensitive (sim_engine_setup_dialog.options_button, TRUE) ;
    }
  else
  if (TS_COHERENCE_VECTOR == *piSimEng)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.ts_coherence_radio), TRUE) ;
    gtk_widget_set_sensitive (sim_engine_setup_dialog.options_button, TRUE) ;
    }
#ifdef HAVE_FORTRAN
  else
  if (SEMI_COHERENT == *piSimEng)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.semi_coherent_radio), TRUE) ;
    gtk_widget_set_sensitive (sim_engine_setup_dialog.options_button, TRUE) ;
    }
  else
  if (TS_FIELD_CLOCK == *piSimEng)
    {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.ts_fc_radio), TRUE) ;
    gtk_widget_set_sensitive (sim_engine_setup_dialog.options_button, TRUE) ;
    }
#endif /* HAVE_FORTRAN */

  g_object_set_data (G_OBJECT (sim_engine_setup_dialog.sim_engine_setup_dialog), "piSimEng", piSimEng) ;
  g_object_set_data (G_OBJECT (sim_engine_setup_dialog.sim_engine_setup_dialog), "dialog", &sim_engine_setup_dialog) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (sim_engine_setup_dialog.sim_engine_setup_dialog)))
    if (NULL != piSimEng)
#ifdef HAVE_FORTRAN
      *piSimEng = 
          gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.coherence_radio))     ? COHERENCE_VECTOR 
        : gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.bistable_radio))      ? BISTABLE
	: gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.ts_coherence_radio))  ? TS_COHERENCE_VECTOR
	: gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.semi_coherent_radio)) ? SEMI_COHERENT
	: TS_FIELD_CLOCK ;
#else /* !HAVE_FORTRAN */
      *piSimEng = 
          gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.coherence_radio))     ? COHERENCE_VECTOR 
        : gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_engine_setup_dialog.bistable_radio))      ? BISTABLE
	: TS_COHERENCE_VECTOR ;
#endif /* HAVE_FORTRAN */

  gtk_widget_hide (sim_engine_setup_dialog.sim_engine_setup_dialog) ;

  if (NULL != parent)
    gtk_window_present (parent) ;
  }

static void create_sim_engine_dialog (sim_engine_setup_D *dialog)
  {
  GtkWidget *imgProps = NULL, *hbox = NULL, *lbl = NULL ;

  if (NULL != dialog->sim_engine_setup_dialog) return ;

  dialog->sim_engine_setup_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->sim_engine_setup_dialog), _("Set Simulation Engine"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->sim_engine_setup_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->sim_engine_setup_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->sim_engine_setup_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->vbox1 = gtk_table_new (5, 1, FALSE);
  gtk_widget_show (dialog->vbox1);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->vbox1, TRUE, TRUE, 0);
 
  dialog->bistable_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Bistable Approximation"));
  g_object_set_data (G_OBJECT (dialog->bistable_radio), "which_options", (gpointer)BISTABLE) ;
  dialog->vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->bistable_radio));
  gtk_widget_show (dialog->bistable_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->bistable_radio, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);
      
  dialog->coherence_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Coherence Vector"));
  g_object_set_data (G_OBJECT (dialog->coherence_radio), "which_options", (gpointer)COHERENCE_VECTOR) ;
  dialog->vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->coherence_radio));
  gtk_widget_show (dialog->coherence_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->coherence_radio, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);    
										
  dialog->ts_coherence_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Three State Coherence"));
  g_object_set_data (G_OBJECT (dialog->ts_coherence_radio), "which_options", (gpointer)TS_COHERENCE_VECTOR) ;
  dialog->vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->ts_coherence_radio));
  gtk_widget_show (dialog->ts_coherence_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->ts_coherence_radio, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);
					
  dialog->semi_coherent_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Semi-Coherent/Bistable"));
#ifdef HAVE_FORTRAN
  g_object_set_data (G_OBJECT (dialog->semi_coherent_radio), "which_options", (gpointer)SEMI_COHERENT) ;
#endif /* HAVE_FORTRAN */
  dialog->vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->semi_coherent_radio));
  gtk_widget_show (dialog->semi_coherent_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->semi_coherent_radio, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 2, 2);
  gtk_widget_set_sensitive(dialog->semi_coherent_radio,
#ifdef HAVE_FORTRAN
    TRUE
#else /* !HAVE_FORTRAN */
    FALSE
#endif /* HAVE_FORTRAN */
  );
	  
  dialog->ts_fc_radio = gtk_radio_button_new_with_label (dialog->vbox1_group, _("Three State Field Clocked"));
#ifdef HAVE_FORTRAN
  g_object_set_data (G_OBJECT (dialog->ts_fc_radio), "which_options", (gpointer)TS_FIELD_CLOCK) ;
#endif /* HAVE_FORTRAN */
  dialog->vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->ts_fc_radio));
  gtk_widget_show (dialog->ts_fc_radio);
  gtk_table_attach (GTK_TABLE (dialog->vbox1), dialog->ts_fc_radio, 0, 1, 4, 5,
					(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					(GtkAttachOptions) (GTK_FILL), 2, 2);
  gtk_widget_set_sensitive(dialog->ts_fc_radio,
#ifdef HAVE_FORTRAN
    TRUE
#else /* !HAVE_FORTRAN */
    FALSE
#endif /* HAVE_FORTRAN */
  );

  // Options
  dialog->options_button = gtk_button_new ();
  gtk_widget_show (dialog->options_button);
  imgProps = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_SMALL_TOOLBAR) ;
  gtk_widget_show (imgProps) ;
  hbox = gtk_hbox_new (FALSE, 2) ;
  gtk_widget_show (hbox) ;
  gtk_box_pack_start (GTK_BOX (hbox), imgProps, FALSE, FALSE, 2) ;
  lbl = gtk_label_new (_("Options")) ;
  gtk_widget_show (lbl) ;
  gtk_box_pack_start (GTK_BOX (hbox), lbl, TRUE, TRUE, 0) ;
  gtk_container_add (GTK_CONTAINER (dialog->options_button), hbox) ;

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog->sim_engine_setup_dialog)->action_area), dialog->options_button) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->sim_engine_setup_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->sim_engine_setup_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->sim_engine_setup_dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->options_button),  "clicked", (GCallback)(options_button_clicked), dialog->sim_engine_setup_dialog);
  g_signal_connect (G_OBJECT (dialog->bistable_radio),  "toggled", (GCallback)(engine_toggled),         dialog->options_button) ;
  g_signal_connect (G_OBJECT (dialog->coherence_radio), "toggled", (GCallback)(engine_toggled),         dialog->options_button) ;
  g_signal_connect (G_OBJECT (dialog->ts_coherence_radio), "toggled", (GCallback)(engine_toggled),      dialog->options_button) ;
  g_signal_connect (G_OBJECT (dialog->semi_coherent_radio), "toggled", (GCallback)(engine_toggled),      dialog->options_button) ;
  g_signal_connect (G_OBJECT (dialog->ts_fc_radio), "toggled", (GCallback)(engine_toggled),      dialog->options_button) ;
  }

static void options_button_clicked (GtkButton *button, gpointer user_data)
  {
  int sim_engine = (int)g_object_get_data (G_OBJECT (button), "which_options") ;

  switch (sim_engine)
    {
    case COHERENCE_VECTOR:
      get_coherence_properties_from_user (GTK_WINDOW (user_data), &coherence_options) ;
      break;

    case BISTABLE:
      get_bistable_properties_from_user (GTK_WINDOW (user_data), &bistable_options) ;
      break ;
		
    case TS_COHERENCE_VECTOR:
      get_ts_coherence_properties_from_user (GTK_WINDOW (user_data), &ts_coherence_options) ;
      break;
#ifdef HAVE_FORTRAN
    case SEMI_COHERENT:
      get_semi_coherent_properties_from_user (GTK_WINDOW (user_data), &semi_coherent_options) ;
      break;
    case TS_FIELD_CLOCK:
      get_ts_fc_properties_from_user (GTK_WINDOW (user_data), &ts_fc_options) ;
      break;
#endif /* HAVE_FORTRAN */
    case DIGITAL_SIM:
      break;
    }//switch
  }

static void engine_toggled (GtkWidget *button,gpointer user_data)
  {
  int iSimEng ;

  g_object_set_data (G_OBJECT (user_data), "which_options", (gpointer)(iSimEng = (int)g_object_get_data (G_OBJECT (button), "which_options"))) ;
  gtk_widget_set_sensitive (GTK_WIDGET (user_data), !(DIGITAL_SIM == iSimEng)) ;
  }
