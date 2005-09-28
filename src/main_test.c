#include <stdio.h>
#include "global_consts.h"
#include "exp_array.h"
#include "custom_widgets.h"
#include "hsl2rgb.h"
#include "drag_rect.h"
#include "gtk_preamble.h"
#include "support.h"
#include "hsl2rgb.h"
#include "generic_utils.h"
#include "objects/QCADDensityMap.h"

static QCADDensityMap *qcaddm = NULL ;
static DRAG_RECT *drag_rect = NULL ;
static GdkPixbuf *pb = NULL ;
static GdkColor clrBlack = {0} ;

//static void draw_background (GtkWidget *widget) ;
//static GdkColor *make_clr (GdkColor *clr, int icNeighbours, int min_neighbours, int max_neighbours) ;

static gboolean da_mouse_enter (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;
static gboolean da_mouse_leave (GtkWidget *widget, GdkEventCrossing *event, gpointer data) ;
static gboolean da_mouse_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data) ;
static gboolean da_mouse_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean da_mouse_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
static gboolean da_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data) ;
static void btnNewRC_clicked (GtkWidget *widget, gpointer data) ;
static void btnDump_clicked (GtkWidget *widget, gpointer data) ;
static void dimension_adj_value_changed (GtkAdjustment *adj, gpointer data) ;
static void min_neighbours_adj_value_changed (GtkAdjustment *adj, gpointer data) ;

static void btnDump_clicked (GtkWidget *widget, gpointer data)
  {
  fprintf (stderr, "btnDump_clicked: Dumping wdg:\n") ;
  qcad_density_map_dump (qcaddm, stderr, 0) ;
  }

static void min_neighbours_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  qcad_density_map_set_min_neighbours_to_draw (qcaddm, gtk_adjustment_get_value (adj)) ;
  gtk_widget_queue_draw (GTK_WIDGET (data)) ;
  }

static void dimension_adj_value_changed (GtkAdjustment *adj, gpointer data)
  {
  GtkAdjustment *cx_adj = g_object_get_data (G_OBJECT (data), "cx_adj"), 
                *cy_adj = g_object_get_data (G_OBJECT (data), "cy_adj") ;

  gtk_widget_set_sensitive (GTK_WIDGET (data), 
    ((gtk_adjustment_get_value (cx_adj) > 0) && (gtk_adjustment_get_value (cy_adj) > 0))) ;
  }

static gboolean da_mouse_enter (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  char *psz = NULL ;

  gtk_label_set_text (GTK_LABEL (data), psz = g_strdup_printf ("(%d,%d)", ((int)(event->x)), ((int)(event->y)))) ;
  g_free (psz) ;

  return FALSE ;
  }

static gboolean da_mouse_leave (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
  {
  gtk_label_set_text (GTK_LABEL (data), "") ;

  return FALSE ;
  }

static gboolean da_mouse_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (1 == event->button)
    {
    char *psz = NULL ;

    // Make sure drag_rect is NULL
    drag_rect = drag_rect_free (drag_rect) ;
    drag_rect = drag_rect_new (widget->window, event->x, event->y) ;

    gtk_label_set_text (GTK_LABEL (data), psz = g_strdup_printf ("(%d,%d)<->(%d,%d)=(%d,%d)[%dx%d]",
      drag_rect->ptBeg.x, drag_rect->ptBeg.y, ((int)(event->x)), ((int)(event->y)),
      drag_rect->rcDraw.x, drag_rect->rcDraw.y, drag_rect->rcDraw.width, drag_rect->rcDraw.height)) ;
    g_free (psz) ;
    }
  else
  if (3 == event->button)
    {
    g_object_unref (qcaddm) ;
    qcaddm = qcad_density_map_new () ;
    qcad_density_map_set_draw_borders (qcaddm, TRUE) ;
    gtk_widget_queue_draw (widget) ;
    }
  else
  if (2 == event->button)
    {
    int Nix ;
    QCADDensityMapEntry *wdge = NULL/*, *neighbour = NULL*/ ;

    for (Nix = 0 ; Nix < qcaddm->ar_entries->icUsed ; Nix++)
      {
      wdge = exp_array_index_1d (qcaddm->ar_entries, QCADDensityMapEntry *, Nix) ;
      if (PT_IN_RECT (event->x, event->y, wdge->rc.xWorld, wdge->rc.yWorld, wdge->rc.cxWorld, wdge->rc.cyWorld))
        fprintf (stderr, "wdge is (%d,%d)[%dx%d]:{icNeighbours = %d, idxChunk = %d, bVisited = %s}\n", (int)(wdge->rc.xWorld), (int)(wdge->rc.yWorld), (int)(wdge->rc.cxWorld), (int)(wdge->rc.cyWorld), wdge->icNeighbours, wdge->idxChunk, wdge->bVisited ? "TRUE" : "FALSE") ;
      }
    }

  return FALSE ;
  }

static gboolean da_mouse_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data)
  {
  GdkModifierType mask ;
  int x, y ;
  char *psz = NULL ;

  if ((event->state & GDK_BUTTON1_MASK) && NULL != drag_rect)
    {
    GdkColor *clr = NULL ;
    GdkGC *gc = gdk_gc_new (widget->window) ;

    drag_rect_move (drag_rect, widget->window, event->x, event->y, 1, 1) ;

    if (NULL == drag_rect->rgnDraw)
      gdk_gc_set_clip_rectangle (gc, &(drag_rect->rcDraw)) ;
    else
      {
      GdkRectangle *rects = NULL ;
      int icRects = 0 ;

      gdk_region_get_rectangles (drag_rect->rgnDraw, &rects, &icRects) ;
      if (icRects > 0 && rects != NULL)
        {
        int Nix ;

        for (Nix = 0 ; Nix < icRects ; Nix++)
          {
          gdk_gc_set_clip_rectangle (gc, &(rects[Nix])) ;
          gdk_draw_drawable (widget->window, gc, drag_rect->src, rects[Nix].x, rects[Nix].y, rects[Nix].x, rects[Nix].y, rects[Nix].width, rects[Nix].height) ;
          tile_pixbuf (widget->window, gc, pb, PIXBUF_BRUSH_CX, PIXBUF_BRUSH_CY, &(rects[Nix])) ;
          }
        g_free (rects) ;
        }
      }

    // Draw the outline rectangle
    g_object_unref (gc) ;
    gc = gdk_gc_new (widget->window) ;

    gdk_gc_set_foreground (gc, clr = clr_idx_to_clr_struct (BLACK)) ;
    gdk_gc_set_foreground (gc, clr) ;
    gdk_draw_rectangle (widget->window, gc, FALSE, drag_rect->rcDraw.x, drag_rect->rcDraw.y, drag_rect->rcDraw.width - 1, drag_rect->rcDraw.height - 1) ;

    g_object_unref (gc) ;

    gtk_label_set_text (GTK_LABEL (data), psz = g_strdup_printf ("(%d,%d)<->(%d,%d)=(%d,%d)[%dx%d]",
      drag_rect->ptBeg.x, drag_rect->ptBeg.y, ((int)(event->x)), ((int)(event->y)), 
      drag_rect->rcDraw.x, drag_rect->rcDraw.y, drag_rect->rcDraw.width, drag_rect->rcDraw.height)) ;
    g_free (psz) ;
    }
  else
    {
    gtk_label_set_text (GTK_LABEL (data), psz = g_strdup_printf ("(%d,%d)", ((int)(event->x)), ((int)(event->y)))) ;
    g_free (psz) ;
    }

  gdk_window_get_pointer (widget->window, &x, &y, &mask) ;

  return FALSE ;
  }

static gboolean da_mouse_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  if (1 == event->button)
    {
    WorldRectangle rc = {0} ;
    GtkAdjustment *min_neighbours_adj = g_object_get_data (G_OBJECT (widget), "min_neighbours_adj") ;
    char *psz = NULL ;
/*
    GdkColor *clr = NULL ;
    GdkGC *gc = gdk_gc_new (widget->window) ;

    gdk_gc_set_foreground (gc, clr = clr_idx_to_clr_struct (BLUE)) ;
    gdk_gc_set_foreground (gc, clr) ;
    gdk_draw_rectangle (widget->window, gc, TRUE, drag_rect->rcDraw.x, drag_rect->rcDraw.y, drag_rect->rcDraw.width, drag_rect->rcDraw.height) ;
    gdk_gc_set_foreground (gc, clr = clr_idx_to_clr_struct (BLACK)) ;
    gdk_gc_set_foreground (gc, clr) ;
    gdk_draw_rectangle (widget->window, gc, FALSE, drag_rect->rcDraw.x, drag_rect->rcDraw.y, MAX (drag_rect->rcDraw.width - 1, 0), MAX (drag_rect->rcDraw.height - 1, 0)) ;

    g_object_unref (gc) ;
*/
//    exp_array_insert_vals (drawn_rcs, &(drag_rect->rcDraw), 1, 1, -1) ;
    rc.xWorld = drag_rect->rcDraw.x ;
    rc.yWorld = drag_rect->rcDraw.y ;
    rc.cxWorld = drag_rect->rcDraw.width ;
    rc.cyWorld = drag_rect->rcDraw.height ;

    qcad_density_map_merge_rectangle (qcaddm, &rc, 1, TRUE) ;
    if (NULL != min_neighbours_adj)
      {
      min_neighbours_adj->lower = qcaddm->min_neighbours ;
      min_neighbours_adj->upper = qcaddm->max_neighbours ;
      gtk_adjustment_changed (min_neighbours_adj) ;
      }

    gtk_widget_queue_draw (widget) ;

    // Make sure drag_rect is NULL
    drag_rect = drag_rect_free (drag_rect) ;
    gtk_label_set_text (GTK_LABEL (data), psz = g_strdup_printf ("(%d,%d)", ((int)(event->x)), ((int)(event->y)))) ;
    g_free (psz) ;
    }

  return FALSE ;
  }

static gboolean da_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {
  qcad_design_object_draw (QCAD_DESIGN_OBJECT (qcaddm), widget->window, GDK_COPY, NULL) ;

  return TRUE ;
  }

static void btnNewRC_clicked (GtkWidget *widget, gpointer data)
  {
  WorldRectangle rc ;
  GtkAdjustment
    *x_adj  = g_object_get_data (G_OBJECT (widget), "x_adj"),
    *y_adj  = g_object_get_data (G_OBJECT (widget), "y_adj"),
    *cx_adj = g_object_get_data (G_OBJECT (widget), "cx_adj"),
    *cy_adj = g_object_get_data (G_OBJECT (widget), "cy_adj"),
    *neighbours = g_object_get_data (G_OBJECT (widget), "neighbours") ;
  GtkWidget *da = g_object_get_data (G_OBJECT (widget), "da") ;

  if (NULL == x_adj || NULL == y_adj || NULL == cx_adj || NULL == cy_adj || NULL == da || NULL == neighbours) return ;

  rc.xWorld = gtk_adjustment_get_value (x_adj) ;
  rc.yWorld = gtk_adjustment_get_value (y_adj) ;
  rc.cxWorld = gtk_adjustment_get_value (cx_adj) ;
  rc.cyWorld = gtk_adjustment_get_value (cy_adj) ;

  qcad_density_map_merge_rectangle (qcaddm, &rc, gtk_adjustment_get_value (neighbours), TRUE) ;

  gtk_widget_queue_draw (da) ;
  }
/*
static void draw_background (GtkWidget *widget)
  {
  GdkGC *gc = NULL ;
  GtkWidget *img = g_object_get_data (G_OBJECT (widget), "img") ;
  GdkPixbuf *pixbuf = NULL ;
  GdkRectangle rc = {0} ;
  int cxImg, cyImg ;

  if (NULL == img) return ;

  pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (img)) ;

  if (NULL == pixbuf) return ;

  cxImg = gdk_pixbuf_get_width (pixbuf) ;
  cyImg = gdk_pixbuf_get_height (pixbuf) ;
  gdk_window_get_size (widget->window, &(rc.width), &(rc.height)) ;

  gc = gdk_gc_new (widget->window) ;
  
  tile_pixbuf (widget->window, gc, pixbuf, cxImg, cyImg, &rc) ;
  g_object_unref (gc) ;
  }

static void draw_world_density_graph (GdkDrawable *dst, WORLD_DENSITY_GRAPH *wdg, int min_neighbours)
  {
  int Nix ;
  GdkGC *gc = NULL ;
  WORLD_DENSITY_GRAPH_ENTRY *wdge = NULL ;
  GdkColor clr, *pclr = NULL ;

  gc = gdk_gc_new (dst) ;

  for (Nix = 0 ; Nix < wdg->ar_entries->icUsed ; Nix++)
    {
    wdge = exp_array_index_1d (wdg->ar_entries, WORLD_DENSITY_GRAPH_ENTRY *, Nix) ;
    gdk_gc_set_foreground (gc, &clrBlack) ;
    gdk_gc_set_background (gc, &clrBlack) ;

    gdk_draw_rectangle (dst, gc, FALSE, (int)(wdge->rc.xWorld), (int)(wdge->rc.yWorld), (int)(wdge->rc.cxWorld), (int)(wdge->rc.cyWorld)) ;

    if (wdge->bUsed && wdge->icNeighbours >= min_neighbours)
      {
      // These are really HSL
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
        make_clr (&clr, wdge->icNeighbours, wdg->min_neighbours, wdg->max_neighbours), FALSE, FALSE) ;
      pclr = &clr ;

      gdk_gc_set_foreground (gc, pclr) ;
      gdk_gc_set_background (gc, pclr) ;

      gdk_draw_rectangle (dst, gc, TRUE, (int)(wdge->rc.xWorld) + 1, (int)(wdge->rc.yWorld) + 1, (int)(wdge->rc.cxWorld) - 1, (int)(wdge->rc.cyWorld) - 1) ;
      }
    }

  g_object_unref (gc) ;
  }

static GdkColor *make_clr (GdkColor *clr, int icNeighbours, int min_neighbours, int max_neighbours)
  {
  clr->red = ((int)(((((double)icNeighbours) / ((double)(ABS (max_neighbours - min_neighbours))))) * 32768.0) + 43690) % 65536 ;
//  clr->red = (int)((double)(((int)(((double)icNeighbours / (double)(ABS (max_neighbours - min_neighbours))) * 180.0 + 240)) % 360) * 65535.0 / 360.0) ;
//  fprintf (stderr, "(%d<->%d<->%d) => %d\n", min_neighbours, icNeighbours, max_neighbours, clr->red >> 8) ;
  clr->green = 0xFFFF ;
  clr->blue = 0x7FFF ;
  HSLToRGB (clr) ;

  return clr ;
  }
*/
int main (int argc, char **argv)
  {
  GtkWidget *align = NULL, *wnd = NULL, *da = NULL, *lbl = NULL, *tbl = NULL, *lbl_msg, *frm = NULL, *tblNB = NULL, *spn = NULL, *btnDump = NULL, *btnNewRC = NULL, *dlg = NULL, *nb = NULL ;
//  GtkWidget *img = NULL ;
  GtkAdjustment *adj = NULL ;
  EXP_ARRAY *sorted_ints = NULL ;
  int the_int = -1, Nix ;
  gboolean bAllowDupes = TRUE ;

  sorted_ints = exp_array_new (sizeof (int), 1) ;
  the_int = 3 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  the_int = 5 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  the_int = 5 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  the_int = 5 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  the_int = 9 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  the_int = 7 ;
  fprintf (stderr, "idx = %d\n", exp_array_1d_insert_val_sorted (sorted_ints, &the_int, compare_ints, bAllowDupes)) ;
  for (Nix = 0 ; Nix < sorted_ints->icUsed ; Nix++)
    fprintf (stderr, "%d ", exp_array_index_1d (sorted_ints, int, Nix)) ;
  fprintf (stderr, "\n") ;
  the_int = 5 ;
  fprintf (stderr, "find closest to %d returns idx %d\n", the_int, exp_array_1d_find (sorted_ints, &the_int, compare_ints, TRUE)) ;
  exp_array_free (sorted_ints) ;

  gtk_preamble (&argc, &argv, "QCADesigner") ;

  qcaddm = qcad_density_map_new () ;
  qcad_density_map_set_draw_borders (qcaddm, TRUE) ;

  gdk_colormap_alloc_color (gdk_colormap_get_system (), &clrBlack, FALSE, FALSE) ;

  pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, PIXBUF_BRUSH_CX, PIXBUF_BRUSH_CY) ;
  gdk_pixbuf_fill (pb, 0xff808080) ;

  wnd = gtk_window_new (GTK_WINDOW_TOPLEVEL) ;
  gtk_window_set_title (GTK_WINDOW (wnd), "World Density Graph") ;
  gtk_window_set_resizable (GTK_WINDOW (wnd), TRUE) ;
  gtk_window_set_default_size (GTK_WINDOW (wnd), 640, 400) ;

  tbl = gtk_table_new (2, 1, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_container_add (GTK_CONTAINER (wnd), tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  frm = gtk_frame_new (NULL) ;
  gtk_widget_show (frm) ;
  gtk_table_attach (GTK_TABLE (tbl), frm, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_frame_set_shadow_type (GTK_FRAME (frm), GTK_SHADOW_IN) ;
  gtk_container_set_border_width (GTK_CONTAINER (frm), 2) ;

  da = gtk_drawing_area_new () ;
  gtk_widget_show (da) ;
  gtk_container_add (GTK_CONTAINER (frm), da) ;
  set_widget_background_colour (da, 0xC0C0, 0xC0C0, 0xC0C0) ;

  lbl_msg = gtk_label_new ("") ;
  gtk_widget_show (lbl_msg) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl_msg, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl_msg), GTK_JUSTIFY_LEFT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl_msg), 0.0, 0.5) ;

  dlg = gtk_dialog_new () ;
  gtk_dialog_set_has_separator (GTK_DIALOG (dlg), FALSE) ;
  gtk_window_set_title (GTK_WINDOW (dlg), "World Density Graph Actions") ;
  gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE) ;

  btnDump = gtk_button_new_with_stock_image (GTK_STOCK_ZOOM_IN, "Dump") ;
  gtk_widget_show (btnDump) ;
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dlg)->action_area), btnDump) ;

  gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (wnd)) ;

  nb = gtk_notebook_new () ;
  gtk_widget_show (nb) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), nb, TRUE, TRUE, 0) ;

  tblNB = gtk_table_new (6, 2, FALSE) ;
  gtk_widget_show (tblNB) ;

  lbl = gtk_label_new_with_mnemonic ("New _Rectangle") ;
  gtk_widget_show (lbl) ;

  gtk_notebook_append_page (GTK_NOTEBOOK (nb), tblNB, lbl) ;

  align = gtk_alignment_new (1.0, 0.5, 1.0, 1.0) ;
  gtk_widget_show (align) ;
  gtk_table_attach (GTK_TABLE (tblNB), align, 0, 2, 5, 6, GTK_FILL, GTK_FILL, 2, 2) ;

  btnNewRC = gtk_button_new_with_stock_image (GTK_STOCK_NEW, "Create") ;
  gtk_widget_show (btnNewRC) ;
  gtk_widget_set_sensitive (btnNewRC, FALSE) ;
  gtk_container_add (GTK_CONTAINER (align), btnNewRC) ;
  g_object_set_data (G_OBJECT (btnNewRC), "da", da) ;

  lbl = gtk_label_new ("x:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 0, 1, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 1, 1, 0)), 1, 0, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 0, 1, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (btnNewRC), "x_adj", adj) ;

  lbl = gtk_label_new ("y:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 1, 2, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 1, 1, 0)), 1, 0, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 1, 2, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (btnNewRC), "y_adj", adj) ;

  lbl = gtk_label_new ("cx:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 2, 3, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 0)), 1, 0, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 2, 3, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (btnNewRC), "cx_adj", adj) ;
  g_signal_connect (G_OBJECT (adj), "value-changed", (GCallback)dimension_adj_value_changed, btnNewRC) ;

  lbl = gtk_label_new ("cy:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 3, 4, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 0)), 1, 0, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 3, 4, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (btnNewRC), "cy_adj", adj) ;
  g_signal_connect (G_OBJECT (adj), "value-changed", (GCallback)dimension_adj_value_changed, btnNewRC) ;

  lbl = gtk_label_new ("neighbours:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 4, 5, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new_infinite (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 1, 1, 0)), 1, 0, ISB_DIR_UP | ISB_DIR_DN) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 4, 5, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (btnNewRC), "neighbours", adj) ;

  tblNB = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tblNB) ;

  lbl = gtk_label_new_with_mnemonic ("_Flooding") ;
  gtk_widget_show (lbl) ;

  gtk_notebook_append_page (GTK_NOTEBOOK (nb), tblNB, lbl) ;

  lbl = gtk_label_new ("Minimum neighbours:") ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tblNB), lbl, 0, 1, 0, 1, GTK_FILL, 0, 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  spn = gtk_spin_button_new (adj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 1, 0)), 1, 0) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tblNB), spn, 1, 2, 0, 1, GTK_FILL, 0, 2, 2) ;
  g_object_set_data (G_OBJECT (da), "min_neighbours_adj", adj) ;
  g_object_set_data (G_OBJECT (btnNewRC), "min_neighbours_adj", adj) ;
  g_signal_connect (G_OBJECT (adj), "value-changed", (GCallback)min_neighbours_adj_value_changed, da) ;
/*
  if (g_file_test ("/usr/share/backgrounds/tiles/blobl.png", G_FILE_TEST_EXISTS))
    img = gtk_image_new_from_file ("/usr/share/backgrounds/tiles/blobl.png") ;
  else
  if (g_file_test ("/usr/share/pixmaps/backgrounds/gnome/tiles/tiles-trans.png", G_FILE_TEST_EXISTS))
    img = gtk_image_new_from_file ("/usr/share/pixmaps/backgrounds/gnome/tiles/tiles-trans.png") ;
  else
    {
    GdkPixbuf *pb_bkgnd = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, PIXBUF_BRUSH_CX, PIXBUF_BRUSH_CY) ;
    gdk_pixbuf_fill (pb_bkgnd, 0xC0C0C0FF) ;
    gtk_image_new_from_pixbuf (pb_bkgnd) ;
    }
  gtk_widget_show (img) ;
  g_object_set_data (G_OBJECT (da), "img", img) ;
*/
  gtk_widget_add_events (da,
    GDK_ENTER_NOTIFY_MASK | GDK_POINTER_MOTION_MASK      | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK |
    GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_RELEASE_MASK) ;

  g_signal_connect (G_OBJECT (wnd),      "delete-event",         (GCallback)gtk_main_quit,           NULL) ;
  g_signal_connect (G_OBJECT (da),       "expose-event",         (GCallback)da_expose,               lbl_msg) ;
  g_signal_connect (G_OBJECT (da),       "enter-notify-event",   (GCallback)da_mouse_enter,          lbl_msg) ;
  g_signal_connect (G_OBJECT (da),       "leave-notify-event",   (GCallback)da_mouse_leave,          lbl_msg) ;
  g_signal_connect (G_OBJECT (da),       "motion-notify-event",  (GCallback)da_mouse_motion_notify,  lbl_msg) ;
  g_signal_connect (G_OBJECT (da),       "button-press-event",   (GCallback)da_mouse_button_press,   lbl_msg) ;
  g_signal_connect (G_OBJECT (da),       "button-release-event", (GCallback)da_mouse_button_release, lbl_msg) ;
  g_signal_connect (G_OBJECT (btnNewRC), "clicked",              (GCallback)btnNewRC_clicked,        NULL) ;
  g_signal_connect (G_OBJECT (btnDump),  "clicked",              (GCallback)btnDump_clicked,         NULL) ;

  gtk_widget_show (wnd) ;
  gtk_widget_show (dlg) ;

  gtk_main () ;

  g_object_unref (pb) ;

  return 0 ;
  }
