/* scatterplot.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "gtkext.h"
#include "externs.h"

#define WIDTH   370
#define HEIGHT  370

void
scatterplot_show_hrule (displayd *display, gboolean show) 
{
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_show (display->hrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_hide (display->hrule);
  }
}
void
scatterplot_show_vrule (displayd *display, gboolean show) {
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_show (display->vrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_hide (display->vrule);
  }
}
void
scatterplot_show_rulers (displayd *display, gint projection)
{
/*
 * Retrieve the size of the drawing area before the axes are added
 * or removed, and then set the da size afterwards.  This prevents
 * plots that have been reduced in size from suddenly being resized
 * up to the original default size.
*/
  splotd *sp = display->splots->data;
  gint width = sp->da->allocation.width;
  gint height = sp->da->allocation.height;

  switch (projection) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL) {
        scatterplot_show_vrule (display, true);
        scatterplot_show_hrule (display, false);
      } else {
        scatterplot_show_vrule (display, false);
        scatterplot_show_hrule (display, true);
      }
    break;

    case XYPLOT:
      scatterplot_show_vrule (display, true);
      scatterplot_show_hrule (display, true);
    break;

    case TOUR1D:
    case TOUR2D:
    case COTOUR:
    default:  /* in any other projection, no rulers */
      scatterplot_show_vrule (display, false);
      scatterplot_show_hrule (display, false);
    break;
  }

  gtk_drawing_area_size (GTK_DRAWING_AREA (sp->da), width, height);
}

void
ruler_ranges_set (gboolean force, displayd *display, splotd *sp, ggobid *gg) {
  /*
   * Run 0 and sp->max through the reverse pipeline to find out
   * what their values should be in terms of the data.  Set the
   * ruler min and max to those values.
   * Force the ranges to be set when a display is being initialized.
  */
  icoords scr;
  fcoords tfmin, tfmax;
  cpaneld *cpanel = &display->cpanel;

  if (display->hrule == NULL)
    return;

  tfmin.x = tfmin.y = tfmax.x = tfmax.y = 0.0;

  scr.x = scr.y = 0;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmin, gg);

  scr.x = sp->max.x;
  scr.y = sp->max.y;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmax, gg);

  /*
   * Reset only if necessary:  if the ruler is visible and the
   * ranges have changed.  Force when initializing display.
  */
  if (force || GTK_WIDGET_VISIBLE (display->hrule)) {
    if (((gfloat) GTK_EXT_RULER (display->hrule)->lower != tfmin.x) ||
        ((gfloat) GTK_EXT_RULER (display->hrule)->upper != tfmax.x))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x);
    }
  }

  if (force || GTK_WIDGET_VISIBLE (display->vrule)) {
    if (((gfloat) GTK_EXT_RULER (display->vrule)->upper != tfmin.y) ||
        ((gfloat) GTK_EXT_RULER (display->vrule)->lower != tfmax.y))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->vrule),
                               (gdouble) tfmax.y, (gdouble) tfmin.y);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static GtkItemFactoryEntry menu_items[] = {
  {"/_File",        NULL,    NULL,            0,            "<Branch>" },
#ifdef PRINTING_IMPLEMENTED
  {"/File/Print",   "",      
    (GtkItemFactoryCallback)display_print_cb,
    0, "<Item>" },
  {"/File/sep",     NULL,    NULL,            0, "<Separator>" },
#endif
  {"/File/Close",   "",      
    (GtkItemFactoryCallback) display_close_cb,
    0, "<Item>" },
};


displayd *
scatterplot_new (gboolean missing_p, splotd *sp, datad *d, ggobid *gg) {
  GtkWidget *table, *vbox, *w;
  GtkItemFactory *factory;
  displayd *display;

  if (d == NULL || d->ncols < 1)
    return (NULL);

  if (sp == NULL) {
    display = display_alloc_init (scatterplot, missing_p, d, gg);
  } else {
    display = (displayd*) sp->displayptr;
    display->d = d;
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
    display->options.axes_center_p = true;
   */

  scatterplot_cpanel_init (&display->cpanel,
    (d->ncols >= 2) ? XYPLOT : P1PLOT, gg);


  display_window_init (display, 3, gg);  /*-- 3 = width = any small int --*/

  /*-- Add the main menu bar --*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  gg->app.sp_accel_group = gtk_accel_group_new ();
  factory = get_main_menu (menu_items,
    sizeof (menu_items) / sizeof (menu_items[0]),
    gg->app.sp_accel_group, display->window, &display->menubar,
    (gpointer) display);

  /*-- add a tooltip to the file menu --*/
  w = gtk_item_factory_get_widget (factory, "<main>/File");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    gtk_menu_get_attach_widget (GTK_MENU(w)),
    "File menu for this display", NULL);

  /*
   * After creating the menubar, and populating the file menu,
   * add the other menus manually
  */
  scatterplot_display_menus_make (display, gg->app.sp_accel_group,
   (GtkSignalFunc) display_options_cb, gg);
  gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);


  /*-- Initialize a single splot --*/
  if (sp == NULL) {
    sp = splot_new (display, WIDTH, HEIGHT, gg);
  }

  display->splots = NULL;
  display->splots = g_list_append (display->splots, (gpointer) sp);

  /*-- Initialize tours if possible --*/
  if (display->displaytype == scatterplot) {
    display_tour1d_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_TOUR1D)
      display_tour1d_init (display, gg);
  }
  if (display->displaytype == scatterplot) {
    display_tour2d_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_TOUR2D)
      display_tour2d_init (display, gg);
  }
  if (display->displaytype == scatterplot) {
    display_tourcorr_init_null (display, gg);
    if (d->ncols >= MIN_NVARS_FOR_COTOUR)
      display_tourcorr_init (display, gg);
  }

  table = gtk_table_new (3, 2, false);  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, true, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    0, 0 );


  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->hrule = gtk_ext_hruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->hrule, motion_notify_event),
    GTK_OBJECT (display->hrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL), 
                    (GtkAttachOptions) GTK_FILL,
                    0, 0);

  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->vrule = gtk_ext_vruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da),
                             "motion_notify_event",
                             (GtkSignalFunc) EVENT_METHOD (display->vrule,
                                                           motion_notify_event),
                             GTK_OBJECT (display->vrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL, 
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    0, 0 );

  gtk_widget_show_all (display->window);
  
  /*-- hide any extraneous rulers --*/
  scatterplot_show_rulers (display, projection_get (gg));
  ruler_ranges_set (true, display, sp, gg);

  return display;
}
