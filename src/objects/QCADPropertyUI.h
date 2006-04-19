#ifndef _OBJECTS_QCADPropertyUI_H_
#define _OBJECTS_QCADPropertyUI_H_

#include <stdarg.h>
#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif /* def GTK_GUI */
#include "../exp_array.h"
#include "../generic_utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _QCADPropertyUI          QCADPropertyUI ;
typedef struct _QCADPropertyUIClass     QCADPropertyUIClass ;
typedef struct _QCADPropertyUIBehaviour QCADPropertyUIBehaviour ;
typedef struct _QCADPropertyUIProperty  QCADPropertyUIProperty ;

/**
 * QCADPropertyUIBehaviour:
 * @instance_property_name1: Connection endpoint 1 instance property name.
 * @ui_property_name1: Connection endpoint 1 UI property name.
 * @instance_property_name2: Connection endpoint 2 instance property name.
 * @ui_property_name2: Connection endpoint 2 UI property name.
 * @fn_forward: Forward mapping function, or %NULL.
 * @data_forward: User data to pass to forward mapping function, or %NULL.
 * @destroy_forward: Function to call when @data_forward needs to be destroyed, or %NULL.
 * @fn_reverse: Reverse mapping function, or %NULL.
 * @data_reverse: User data to pass to reverse mapping function, or %NULL.
 * @destroy_reverse: Function to call when @data_reverse needs to be destroyed, or %NULL.
 *
 * This structure stores a single property UI behaviour. A behaviour is defined as a connection between an
 * instance property and a property UI property, or two property UI properties. In essence, this structure holds
 * all the parameters necessary for a call to connect_object_properties() or disconnect_object_properties(),
 * without making reference to object instances themselves.
 *
 * The #QCADObject class for each #QCADObject subtype holds an array of these structures. This array is
 * consulted whenever a property UI for a #QCADObject instance is created. At this time, the parameters to the
 * connect_object_properties() function are resolved as follows:
 * <variablelist>
 * <varlistentry>
 * <term>@src</term>
 * <listitem><para>
 * If @ui_property_name1 is %NULL, the instance is used as @src. Otherwise, the property UI is used as @src.
 * </para></listitem>
 * </varlistentry>
 * <varlistentry>
 * <term>@pszSrc</term>
 * <listitem><para>
 * If @ui_property_name1 is %NULL, @instance_property_name1 is passed as @pszSrc. Otherwise, @ui_property_name1
 * is passed as @pszSrc.
 * </para></listitem>
 * </varlistentry>
 * <varlistentry>
 * <term>@dst</term>
 * <listitem><para>
 * If @ui_property_name2 is %NULL, the instance is used as @dst. Otherwise, the property UI is used as @dst.
 * is passed as @pszSrc.
 * </para></listitem>
 * </varlistentry>
 * <varlistentry>
 * <term>@pszDst</term>
 * <listitem><para>
 * If @ui_property_name2 is %NULL, @instance_property_name2 is passed as @pszDst. Otherwise, @ui_property_name2
 * is passed as @pszDst.
 * </para></listitem>
 * </varlistentry>
 * </variablelist>
 * The rest of the parameters to connect_object_properties() (@fn_forward, @data_forward, @destroy_forward,
 * @fn_reverse, @data_revers, and @destroy_reverse) are passed as found in the structure.
 * These semantics apply to disconnect_object_properties() as well.
 *
 * See also: #QCADObject, qcad_property_ui_group_newv(), qcad_property_ui_single_newv()
 */
struct _QCADPropertyUIBehaviour
  {
  char *instance_property_name1 ;
  char *ui_property_name1 ;
  char *instance_property_name2 ;
  char *ui_property_name2 ;
  PropertyConnectFunction fn_forward ;
  gpointer data_forward ;
  GDestroyNotify destroy_forward ;
  PropertyConnectFunction fn_reverse ;
  gpointer data_reverse ;
  GDestroyNotify destroy_reverse ;
  } ;

/**
 * QCADPropertyUIProperty:
 * @instance_property_name: Instance property whose UI the value refers to.
 * @ui_property_name: Property UI property name the value refers to.
 * @ui_property_value: Value to assign to property upon UI creation.
 *
 * This structure holds a single property UI property. In essence, this structure holds all the parameters
 * necessary for a call to g_object_set_property() without making reference to object instances themselves.
 * The #QCADObject class for each #QCADObject subtype holds an array of these structures. This array is
 * consulted whenever a property UI for a #QCADObject instance is created. At this time, the structure is used
 * as follows:
 *   <itemizedlist><listitem><para>
 * If @instance_property_name is %NULL, g_object_set_property() is called only if a #QCADPropertyUIGroup is
 * being created and, if so, the parameters to g_object_set_property() are as follow:
 *   <variablelist><varlistentry><term>
 * @object
 *   </term><listitem><para>
 * The #QCADPropertyUIGroup being created is passed.
 *   </para></listitem></varlistentry>
 *   <varlistentry><term>
 * @property_name
 *   </term><listitem><para>
 * @ui_property_name is passed.
 *   </para></listitem></varlistentry>
 *   <varlistentry><term>
 * @value
 *   </term><listitem><para>
 * @ui_property_value is passed by reference.
 *   </para></listitem></varlistentry></variablelist></para></listitem>
 *   <listitem><para>
 * If @instance_property_name is not %NULL, g_object_set_property() is called with parameters are as follow:
 *   <variablelist><varlistentry><term>
 * @object
 *   </term><listitem><para>
 * The #QCADPropertyUISingle being created for @instance_property_name is passed.
 *   </para></listitem></varlistentry>
 *   <varlistentry><term>
 * @property_name
 *   </term><listitem><para>
 * @ui_property_name is passed.
 *   </para></listitem></varlistentry>
 *   <varlistentry><term>
 * @value
 *   </term><listitem><para>
 * @ui_property_value is passed by reference.
 *   </para></listitem></varlistentry></variablelist></para></listitem></itemizedlist>
 *
 * See also: #QCADObject, qcad_property_ui_group_newv(), qcad_property_ui_single_newv()
 */
struct _QCADPropertyUIProperty
  {
  char *instance_property_name ;
  char *ui_property_name ;
  GValue ui_property_value ;
  } ;

struct _QCADPropertyUI
  {
  GObject parent_instance ;

  int cxWidgets ;
  int cyWidgets ;
  gboolean bSensitive ;
  gboolean bVisible ;
  GObject *instance ;
  } ;

struct _QCADPropertyUIClass
  {
  GObjectClass parent_class ;

  gboolean (*set_instance) (QCADPropertyUI *property_ui, GObject *new_instance, GObject *old_instance) ;
#ifdef GTK_GUI
  GtkWidget *(*get_widget) (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
#endif /* def GTK_GUI */
  void (*set_visible) (QCADPropertyUI *property_ui, gboolean bVisible) ;
  void (*set_sensitive) (QCADPropertyUI *property_ui, gboolean bSensitive) ;
  } ;

GType qcad_property_ui_get_type () ;

#define QCAD_TYPE_STRING_PROPERTY_UI "QCADPropertyUI"
#define QCAD_TYPE_PROPERTY_UI (qcad_property_ui_get_type ())
#define QCAD_PROPERTY_UI(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), QCAD_TYPE_PROPERTY_UI, QCADPropertyUI))
#define QCAD_IS_PROPERTY_UI(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), QCAD_TYPE_PROPERTY_UI))
#define QCAD_PROPERTY_UI_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS  ((object), QCAD_TYPE_PROPERTY_UI, QCADPropertyUIClass))
#define QCAD_PROPERTY_UI_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST    ((class),  QCAD_TYPE_PROPERTY_UI, QCADPropertyUIClass))
#define QCAD_IS_PROPERTY_UI_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE    ((class),  QCAD_TYPE_PROPERTY_UI))

QCADPropertyUI *qcad_property_ui_new (GObject *instance, char *property_name, ...) ;
void qcad_property_ui_set_instance (QCADPropertyUI *property_ui, GObject *instance) ;
int qcad_property_ui_get_cx_widgets (QCADPropertyUI *property_ui) ;
int qcad_property_ui_get_cy_widgets (QCADPropertyUI *property_ui) ;
#ifdef GTK_GUI
GtkWidget *qcad_property_ui_get_widget (QCADPropertyUI *property_ui, int idxX, int idxY, int *col_span) ;
#endif /* def GTK_GUI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OBJECTS_QCADPropertyUI_H_ */
