/* tour2d_pp_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   200
#define HEIGHT  100

#define HOLES           0
#define CENTRAL_MASS    1
#define SKEWNESS        2

/* terms in expansion, bandwidth */
/*
static GtkWidget *param_vb, *param_lbl, *param_scale;
static GtkAdjustment *param_adj;
*/

/*-- called when closed from the close menu item --*/
static void close_menuitem_cb (ggobid *gg, gint action, GtkWidget *w) {
  /*  free_optimize0_p(&dsp->t2d_pp_op); should this go here? */
  displayd *dsp = gg->current_display;
  gtk_widget_hide (dsp->t2d_window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  displayd *dsp = gg->current_display;
  gtk_widget_hide (dsp->t2d_window);
}

/*
static void
hide_cb (GtkWidget *w) {
  gtk_widget_hide (w);
}
*/

static void
options_cb(ggobid *gg, guint action, GtkCheckMenuItem *w) {
  displayd *dsp = gg->current_display; 
  
  switch (action) {

    case 0:
      if (w->active)
        gtk_widget_show (dsp->t2d_control_frame);
      else
        gtk_widget_hide (dsp->t2d_control_frame);
      break;

    case 1:
    case 2:
    default:
      fprintf(stderr, "Unhandled switch-case in options_cb\n");
  }
}

/*static void
line_options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {

  switch (action) {
    case 0:
    case 1:
    case 2:
    default:
      fprintf(stderr, "Unhandled switch-case in line_options_cb\n");
  }
  }*/

/*static void
bitmap_size_cb(gpointer data, guint action, GtkCheckMenuItem *w) {

  switch (action) {
    case 0:
    case 1:
    case 2:
    default:
      fprintf(stderr, "Unhandled switch-case in bitmap_size_cb\n");
  }
  }*/

/*static void
replot_freq_cb(gpointer data, guint action, GtkCheckMenuItem *w) {

  switch (action) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
      break;
    default:
      fprintf(stderr, "Unhandled switch-case in replot_freq_cb\n");
  }
  }*/

static void
t2d_optimz_cb (GtkToggleButton  *w, ggobid *gg) {
  displayd *dsp = gg->current_display; 

  t2d_optimz(w->active, &dsp->t2d.get_new_target, 
    &dsp->t2d.target_selection_method, dsp);
}

/*
static void
sphere_cb (GtkWidget  *w, ggobid *gg) {

  sphere_panel_open(gg);
}
*/

gchar *t2d_pp_func_lbl[] = {"Holes","Central Mass","Skewness"};
void t2d_pp_func_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  displayd *dsp = gg->current_display;
  gint indx = GPOINTER_TO_INT (cbd);
  gchar *label = g_strdup("PP index: (0.000) 0.0000 (0.000)");

  cpanel->t2d.pp_indx = indx;
  dsp->t2d.get_new_target = true;

  dsp->t2d.ppval = 0.00;
  dsp->t2d.oppval = -999.0;
  dsp->t2d_pp_op.index_best = -100.0;
  sprintf(label,"PP index: (%3.1f) %5.3f (%3.1f) ",0.0,dsp->t2d.ppval,0.0);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  t2d_clear_ppda(gg);

  /*  if (indx == SUBD || LDA || CART_GINI || CART_ENTROPY || CART_VAR || PCA)
    gtk_widget_hide (param_vb);
  else {
  gtk_widget_show (param_vb);
  }*/
}

/*
static void bitmap_cb (GtkButton *button)
{
  g_printerr ("drop a new bitmp\n");
}
static void
return_to_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("return to bitmap?  %d\n", w->active);
}
static void
record_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("record bitmap?  %d\n", w->active);
}
*/

static gint
t2d_ppda_configure_cb (GtkWidget *w, GdkEventConfigure *event, displayd *dsp)
{
  gint wid = w->allocation.width, hgt = w->allocation.height;

  if (dsp->t2d_pp_pixmap != NULL)
    gdk_pixmap_unref (dsp->t2d_pp_pixmap);

  dsp->t2d_pp_pixmap = gdk_pixmap_new (dsp->t2d_ppda->window,
    wid, hgt, -1);

  return false;
}

static gint
t2d_ppda_expose_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  displayd *dsp = gg->current_display;
/*
  gint margin=10;
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (dsp->t2d_ppda);
  datad *d = gg->current_display->d;
*/
  gint wid = w->allocation.width, hgt = w->allocation.height;
  /*  static gboolean init = true;*/

  /*  if (init) {
    t2d_clear_ppda(gg);
    init=false;
    }*/

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);

  return false;
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,         NULL, 0, "<Branch>" },
  { "/File/Close",  
         "",         (GtkItemFactoryCallback) close_menuitem_cb, 0, "<Item>" },
  { "/_Options",      NULL,         NULL, 0, "<Branch>" },
  { "/Options/Show controls",  
         "v",         (GtkItemFactoryCallback) options_cb, 0, "<CheckItem>" },
/* -- comment the rest out for now -- */
/*
  { "/Options/Show lines",  
         "",         (GtkItemFactoryCallback) options_cb, 1, "<CheckItem>" },
  { "/Options/Show points",  
         "" ,        (GtkItemFactoryCallback) options_cb, 2, "<CheckItem>" },
  { "/Options/Line thickness",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Line thickness/Thin",  
         "" ,        (GtkItemFactoryCallback) line_options_cb,   0, "<RadioItem>" },
  { "/Options/Line thickness/Medium",  
         "" ,        (GtkItemFactoryCallback) line_options_cb,   1, "/Options/Line thickness/Thin" },
  { "/Options/Line thickness/Thick",  
         "" ,        (GtkItemFactoryCallback) line_options_cb,   2, "/Options/Line thickness/Thin" },
  { "/Options/Bitmap size",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Bitmap size/Small",  
         "" ,        (GtkItemFactoryCallback) bitmap_size_cb, 0, "<RadioItem>" },
  { "/Options/Bitmap size/Medium", 
         "" ,        (GtkItemFactoryCallback) bitmap_size_cb, 1, "/Options/Bitmap size/Small" },
  { "/Options/Bitmap size/Large",  
         "" ,        (GtkItemFactoryCallback) bitmap_size_cb, 2, "/Options/Bitmap size/Small" },
  { "/Options/Replot frequency",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Replot frequency/1",  
         "" ,        (GtkItemFactoryCallback) replot_freq_cb, 1, "<RadioItem>" },
  { "/Options/Replot frequency/2",  
         "" ,        (GtkItemFactoryCallback) replot_freq_cb, 2, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/4",  
         "" ,        (GtkItemFactoryCallback) replot_freq_cb, 4, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/8",  
         "" ,        (GtkItemFactoryCallback) replot_freq_cb, 8, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/16",  
         "" ,        (GtkItemFactoryCallback) replot_freq_cb, 16,"/Options/Replot frequency/1" },
*/
};

void
tour2dpp_window_open (ggobid *gg) {
  /*GtkWidget **btn, *label, *da, *entry;*/
  GtkWidget *hbox, *vbox, *vbc, *vb, *frame, *tgl;
  GtkWidget *hb, *opt;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint i, j;
  gboolean vars_sphered = true;
  /*-- to initialize the checkboxes in the menu --*/
  GtkItemFactory *factory;
  GtkWidget *item;

  /* check if selected variables are sphered beforeing allowing window
     to popup */
  /*  if (dsp->t2d.nactive > d->sphere.pcvars.nels)
    vars_sphered = false;
  for (j=0; j<dsp->t2d.nactive; j++) 
  {
    for (i=0; i<d->sphere.pcvars.nels; i++) 
      if (dsp->t2d.active_vars.els[j] == d->sphere.pcvars.els[i])
        break;
    if ((i == d->sphere.pcvars.nels-1) && 
       (dsp->t2d.active_vars.els[j] != d->sphere.pcvars.els[i]))
    {
      vars_sphered = false;
      break;
    }
    }*/

  if (!vars_sphered) {

    quick_message ("The selected variables have to be sphered first. Use the Tools menu and select the sphering option.", false);

  } else {

    if (dsp->t2d_window == NULL) {

      dsp->t2d_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (dsp->t2d_window), 
        "projection pursuit - 2D");
      gtk_signal_connect (GTK_OBJECT (dsp->t2d_window), "delete_event",
                          GTK_SIGNAL_FUNC (close_wmgr_cb), (gpointer) gg);
      /*gtk_window_set_policy (GTK_WINDOW (dsp->t2d_window), true, true, false);*/
      gtk_container_set_border_width (GTK_CONTAINER (dsp->t2d_window), 10);

/*
 * Add the main menu bar
*/
      vbox = gtk_vbox_new (FALSE, 1);
      gtk_container_border_width (GTK_CONTAINER (vbox), 1);
      gtk_container_add (GTK_CONTAINER (dsp->t2d_window), vbox);

      dsp->t2d_pp_accel_group = gtk_accel_group_new ();
      factory = get_main_menu (menu_items,
        sizeof (menu_items) / sizeof (menu_items[0]),
        dsp->t2d_pp_accel_group, dsp->t2d_window, &dsp->t2d_mbar,
        (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vbox), dsp->t2d_mbar, false, true, 0);


/*
 * Divide the window:  controls on the left, plot on the right
*/
      hbox = gtk_hbox_new (false, 1);
      gtk_container_border_width (GTK_CONTAINER (hbox), 1);
      gtk_box_pack_start (GTK_BOX (vbox),
                        hbox, true, true, 1);
/*
 * Controls
*/
      dsp->t2d_control_frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (dsp->t2d_control_frame), GTK_SHADOW_IN);
      gtk_container_set_border_width (GTK_CONTAINER (dsp->t2d_control_frame), 5);
      gtk_box_pack_start (GTK_BOX (hbox),
                        dsp->t2d_control_frame, false, false, 1);

      vbc = gtk_vbox_new (false, 5);
      gtk_container_set_border_width (GTK_CONTAINER (vbc), 5);
      gtk_container_add (GTK_CONTAINER (dsp->t2d_control_frame), vbc);

/*
 * Optimize toggle
*/
      tgl = gtk_check_button_new_with_label ("Optimize");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
        "Guide the tour using projection pursuit optimization or tour passively",
        NULL);
      gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                          GTK_SIGNAL_FUNC (t2d_optimz_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vbc),
                        tgl, false, false, 1);
/*
 * Index value with label
*/
      hb = gtk_hbox_new (false, 3);
      gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 2);
  
      dsp->t2d_pplabel = gtk_label_new ("PP index: 0.0000");
      gtk_misc_set_alignment (GTK_MISC (dsp->t2d_pplabel), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hb), dsp->t2d_pplabel, false, false, 0);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), dsp->t2d_pplabel,
        "The value of the projection pursuit index for the current projection",
        NULL);

    /*    entry = gtk_entry_new_with_max_length (32);
    gtk_entry_set_editable (GTK_ENTRY (entry), false);
    gtk_entry_set_text (GTK_ENTRY (entry), "0");
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "The value of the projection pursuit index for the current projection",
      NULL);
    gtk_signal_connect (GTK_OBJECT (entry), "value_changed",
    GTK_SIGNAL_FUNC (t2d_writeindx_cb), gg);*/
    /*    gtk_signal_connect (GTK_OBJECT (dsp->t2d.ppval), "value_changed",
          GTK_SIGNAL_FUNC (t2d_writeindx_cb), gg);*/

/*
 * pp index menu and scale inside frame
*/
    /*    frame = gtk_frame_new ("PP index function");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbc), frame, false, false, 0);
    */

    /*    vb = gtk_vbox_new (false, 3);*/
      vb = gtk_vbox_new (true, 2);
      gtk_box_pack_start (GTK_BOX (vbc), vb, false, false, 2);
    /*    gtk_container_add (GTK_CONTAINER (frame), vb);*/

      /*      btn = gtk_button_new_with_label ("Sphere Vars");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
        "Interface to sphering variables (principal components analysis)", NULL);
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
         GTK_SIGNAL_FUNC (sphere_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 1);
      */

      opt = gtk_option_menu_new ();
      gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
/*
      gtk_misc_set_alignment (opt, 0, 0.5);
*/
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "Set the projection pursuit index", NULL);
      gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
      /*    gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);*/
      populate_option_menu (opt, t2d_pp_func_lbl,
                          sizeof (t2d_pp_func_lbl) / sizeof (gchar *),
                          (GtkSignalFunc) t2d_pp_func_cb, gg);

    /*    param_vb = gtk_vbox_new (false, 3);
    gtk_container_set_border_width (GTK_CONTAINER (param_vb), 4);
    gtk_box_pack_start (GTK_BOX (vb), param_vb, false, false, 2);

    param_lbl = gtk_label_new ("Terms in expansion:");
    gtk_misc_set_alignment (GTK_MISC (param_lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (param_vb), param_lbl, false, false, 0);

    param_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                                                      1.0, 30.0,
                                                      1.0, 1.0, 0.0);
    param_scale = gtk_hscale_new (GTK_ADJUSTMENT (param_adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), param_scale,
      "Set number of terms in the expansion for some indices; bandwidth for others", NULL);
    gtk_range_set_update_policy (GTK_RANGE (param_scale),
                                 GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (param_scale), 0);
    gtk_scale_set_value_pos (GTK_SCALE (param_scale), GTK_POS_BOTTOM);

    gtk_box_pack_start (GTK_BOX (param_vb), param_scale, true, true, 0);
    */

/*
 * Drawing area in a frame
*/
      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      gtk_box_pack_start (GTK_BOX (hbox),
                          frame, true, true, 1);

      dsp->t2d_ppda = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (dsp->t2d_ppda), WIDTH, HEIGHT);
      gtk_signal_connect (GTK_OBJECT (dsp->t2d_ppda),
                          "configure_event",
                          (GtkSignalFunc) t2d_ppda_configure_cb,
                          (gpointer) dsp);

      gtk_signal_connect (GTK_OBJECT (dsp->t2d_ppda),
                          "expose_event",
                          (GtkSignalFunc) t2d_ppda_expose_cb,
                          (gpointer) gg);
  
      gtk_container_add (GTK_CONTAINER (frame), dsp->t2d_ppda);
      gtk_widget_show_all (dsp->t2d_window);

      /*-- Set the appropriate check menu items to true. -- dfs --*/
      item = gtk_item_factory_get_widget (factory, "/Options/Show controls");
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
/* comment out until the options are implemented
      item = gtk_item_factory_get_widget (factory, "/Options/Show lines");
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
      item = gtk_item_factory_get_widget (factory, "/Options/Show points");
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), true);
*/
    }

    alloc_optimize0_p(&dsp->t2d_pp_op, d->nrows_in_plot, dsp->t2d.nactive, 2);

    gtk_widget_show_all (dsp->t2d_window);
  }
}

#undef HOLES
#undef CENTRAL_MASS
#undef SKEWNESS
