#ifndef _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_
#define _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_

#include <gtk/gtk.h>

void     vtod_actOpen_activate           (GtkAction *action, gpointer data) ;
void     vtod_actSave_activate           (GtkAction *action, gpointer data) ;
void     vtod_actAdd_activate            (GtkAction *action, gpointer data) ;
void     vtod_actInsert_activate         (GtkAction *action, gpointer data) ;
void     vtod_actDelete_activate         (GtkAction *action, gpointer data) ;
void     vtod_actClose_activate          (GtkAction *action, gpointer data) ;
void     vtod_actSimType_changed         (GtkRadioAction *radio_action, GtkRadioAction *current_action, gpointer data) ;
void     vtod_treeview_size_allocate     (GtkWidget *widget, GtkAllocation *allocation, gpointer data) ;
void     vtod_hadj_value_changed         (GtkAdjustment *adj, gpointer data) ;
gboolean vtod_treeview_focus             (GtkWidget *widget, GdkEventFocus *event, gpointer data) ;
void     vtod_active_flag_data_func      (GtkTreeViewColumn *col, GtkCellRenderer *cr, GtkTreeModel *tm, GtkTreeIter *itr, gpointer data) ;
void     vtod_active_flag_toggled        (GtkCellRendererToggle *cr, char *pszPath, gpointer data) ;
void     vector_actions_notify_sensitive (GtkActionGroup *ag, GParamSpec *spec, gpointer data) ;

#endif /* def _VECTOR_TABLE_OPTIONS_DIALOG_CALLBACKS_H_ */
