/* main_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "display_tree.h"


const char *const GGOBI(OpModeNames)[] = {
  "1D Plot",
  "XYPlot",
  "Rotation",
  "1D Tour",
  "2D Tour",
  "Correlation Tour",
  "Scale",
  "Brush",
  "Identify",
  "Edit Lines",
  "Move Points",

  "Scatmat",
  "Parcoords",
  "TSplot",
};

static const char *const *mode_name = GGOBI(OpModeNames);

void
make_control_panels (ggobid *gg) {

  cpanel_p1dplot_make (gg);
  cpanel_xyplot_make (gg);
  cpanel_rotation_make (gg);
  cpanel_tour1d_make (gg);
  cpanel_tour2d_make (gg);
  cpanel_ctour_make (gg);

  cpanel_brush_make (gg);
  cpanel_scale_make (gg);
  cpanel_identify_make (gg);
  cpanel_lineedit_make (gg);
  cpanel_movepts_make (gg);

  cpanel_parcoords_make (gg);
  cpanel_scatmat_make (gg);
  cpanel_tsplot_make (gg);
}


void
main_display_options_cb (ggobid *gg, guint action, GtkCheckMenuItem *w) 
{
  GSList *l;
  datad *d;

  if (gg->mode_frame == NULL)  /* so it isn't executed on startup */
    return;

  switch (action) {

    case 0:
      if (w->active) gtk_tooltips_enable (gg->tips);
      else gtk_tooltips_disable (gg->tips);
      break;

    case 1:
      if (w->active)
        gtk_widget_show (gg->mode_frame);
      else
        gtk_widget_hide (gg->mode_frame);
      break;

    case 2:
      if (gg->current_display != NULL) {
        displayd *display = gg->current_display;

        if (display->displaytype == scatterplot) {
          if (display->hrule != NULL &&
              display->cpanel.projection == XYPLOT)
          {
            if (w->active) {
              gtk_widget_show (display->hrule);
              gtk_widget_show (display->vrule);
            }
            else {
              gtk_widget_hide (display->hrule);
              gtk_widget_hide (display->vrule);
            }
          }
        }
      }
    break;

    case 3:
      gg->varpanel_ui.layoutByRow = !gg->varpanel_ui.layoutByRow;
      for (l = gg->d; l; l = l->next) {
        d = (datad *) l->data;
        varcircles_layout_reset (d->ncols, d, gg);
      }
    break;

/*
    case 4:
      g_printerr ("toggle centering axes\n");
      break;
    case 5:
      g_printerr ("toggle plotting points\n");
      break;
    case 6:
      g_printerr ("toggle plotting lines\n");
      break;
    case 7:
      g_printerr ("toggle plotting directed lines\n");
      break;
*/
  }
}


void
mode_submenus_activate (splotd *sp, gint m, gboolean state, ggobid *gg)
{
  if (state == off) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case TSPLOT:
      case XYPLOT:
      case LINEED:
      case MOVEPTS:
      break;

      case ROTATE:
        submenu_destroy (gg->mode_menu.io_item);
      break;

      case TOUR1D:
        submenu_destroy (gg->mode_menu.io_item);
      break;

      case TOUR2D:
        submenu_destroy (gg->mode_menu.io_item);
      break;

      case COTOUR:
        submenu_destroy (gg->mode_menu.io_item);
      break;

      case SCALE:
        submenu_destroy (gg->mode_menu.reset_item);
      break;

      case BRUSH:
        submenu_destroy (gg->mode_menu.reset_item);
#ifdef BRUSHING_OPTIONS_IMPLEMENTED
        submenu_destroy (gg->mode_menu.link_item);
#endif
      break;

      case IDENT:
#ifdef UNLINKING_IMPLEMENTED
        submenu_destroy (gg->mode_menu.link_item);
#endif
      break;
    }
  } else if (state == on) {

    switch (m) {
      case PCPLOT:
      case P1PLOT:
      case XYPLOT:
      case TSPLOT:
      case LINEED:
      case MOVEPTS:
      break;

      case ROTATE:
        rotation_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->app.rotation_io_menu); 
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
      break;

      case TOUR1D:
        tour1d_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->tour1d.io_menu); 
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
      break;

      case TOUR2D:
        tour2d_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->tour2d.io_menu); 
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
      break;

      case COTOUR:
        tourcorr_menus_make (gg);

        gg->mode_menu.io_item = submenu_make ("_I/O", 'I',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.io_item),
          gg->tourcorr.io_menu); 
          submenu_insert (gg->mode_menu.io_item, gg->main_menubar, -1);
      break;

      case SCALE :
        scale_menus_make (gg);

        gg->mode_menu.reset_item = submenu_make ("_Reset", 'R',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.reset_item),
          gg->scale.scale_reset_menu); 
          submenu_insert (gg->mode_menu.reset_item, gg->main_menubar, -1);
      break;

      case BRUSH :
        brush_menus_make (gg);

        gg->mode_menu.reset_item = submenu_make ("_Reset", 'R',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.reset_item),
          gg->brush.reset_menu);
          submenu_insert (gg->mode_menu.reset_item, gg->main_menubar, -1);

#ifdef BRUSHING_OPTIONS_IMPLEMENTED
        gg->mode_menu.link_item = submenu_make ("_Link", 'L',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.link_item),
          gg->brush.link_menu); 
          submenu_insert (gg->mode_menu.link_item, gg->main_menubar, -1);
#endif
      break;

      case IDENT:
        identify_menus_make (gg);

#ifdef UNLINKING_IMPLEMENTED
        gg->mode_menu.link_item = submenu_make ("_Link", 'L',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->mode_menu.link_item),
          gg->identify.link_menu); 
          submenu_insert (gg->mode_menu.link_item, gg->main_menubar, -1);
#endif
      break;
    }
  }
}

gint
mode_get (ggobid* gg) {
  return gg->mode;
}
gint
projection_get (ggobid* gg) {
  return gg->projection;
}

/*
 * Use the mode to determine whether the variable selection
 * panel should display checkboxes or circles
*/
gboolean
varpanel_highd (gint mode)
{
  return (mode == TOUR1D || mode == TOUR2D || mode == COTOUR);
}
gboolean
varpanel_permits_circles_or_checkboxes (gint mode)
{
  return (mode > COTOUR && mode < SCATMAT);
}
/*
 * Use the widget state to figure out which is currently displayed.
*/
gboolean
varpanel_shows_circles (ggobid *gg)
{
  datad *d = gg->current_display->d;
  return GTK_WIDGET_MAPPED (d->vcirc_ui.vbox);
}
gboolean
varpanel_shows_checkboxes (ggobid *gg)
{
  datad *d = gg->current_display->d;
  return GTK_WIDGET_MAPPED (d->vcbox_ui.swin);
}

static void
varpanel_reinit (ggobid *gg)
{
  GSList *l;
  datad *d;

  if (varpanel_permits_circles_or_checkboxes (gg->mode))
  {
    ;  /*-- no change required either way --*/
  }

  else if (varpanel_highd(gg->mode) && varpanel_shows_checkboxes (gg))
  {  /*-- remove checkboxes and add circles --*/
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      /*
       * add a reference to the checkboxes' scrolled window
       * (so it won't disappear), then remove it from the ebox.
      */
      gtk_widget_ref (d->vcbox_ui.swin);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.ebox),
                                           d->vcbox_ui.swin);
      /*
       * Now add the parent vbox for the table of variable circles
       * to the ebox
      */
      gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox),
                                        d->vcirc_ui.vbox);
      /*-- update the reference count for the vbox --*/
      if (GTK_OBJECT (d->vcirc_ui.vbox)->ref_count > 1)
        gtk_widget_unref (d->vcirc_ui.vbox);
    }
  } else if (!varpanel_highd(gg->mode) && varpanel_shows_circles (gg))
  {  /*-- remove circles and add checkboxes --*/
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      gtk_widget_ref (d->vcirc_ui.vbox);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.ebox),
                                           d->vcirc_ui.vbox);
      gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox),
                                        d->vcbox_ui.swin);
      if (GTK_OBJECT (d->vcbox_ui.swin)->ref_count > 1)
        gtk_widget_unref (d->vcbox_ui.swin);
    }
  }
}

/*
 * Set the mode for the current display
*/
void 
mode_set (gint m, ggobid *gg) {
  displayd *display = gg->current_display;

  gg->mode = m;
  if (gg->mode != gg->prev_mode) {

    /* Add a reference to the widget so it isn't destroyed */
    gtk_widget_ref (gg->control_panel[gg->prev_mode]);
    gtk_container_remove (GTK_CONTAINER (gg->mode_frame),
                          gg->control_panel[gg->prev_mode]);
  
    gtk_frame_set_label (GTK_FRAME (gg->mode_frame), mode_name[gg->mode]);
    gtk_container_add (GTK_CONTAINER (gg->mode_frame),
      gg->control_panel[gg->mode]);

    /*-- avoid increasing the object's ref_count infinitely  --*/
    if (GTK_OBJECT (gg->control_panel[gg->mode])->ref_count > 1)
      gtk_widget_unref (gg->control_panel[gg->mode]);

    /* 
     * If moving between modes whose variable selection interface
     * differs, swap in the correct display.
     */
    varpanel_reinit (gg);
  }

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * TOUR1D, TOUR2D or COTOUR.  It only changes if another projection
   * type is selected.  (For parcoords and scatmat plots, the
   * value of projection is irrelevant.)
  */
  if (display->displaytype == scatterplot) {
    if (gg->mode <= COTOUR)
      gg->projection = display->cpanel.projection = gg->mode;

    if (gg->projection != gg->prev_projection) {
      scatterplot_show_rulers (display, gg->projection);
      gg->prev_projection = gg->projection;
    }
  }

  gg->prev_mode = gg->mode;

  varpanel_tooltips_set (gg);
  varpanel_refresh (gg);
}

/*
 * Turn the tour procs on and off here
*/
static void
procs_activate (gboolean state, displayd *display, ggobid *gg)
{
  switch (gg->mode) {
    case TOUR2D:
      if (!display->cpanel.t2d_paused)
        tour2d_func (state, display, gg);
    break;
    case TOUR1D:
      if (!display->cpanel.t1d_paused)
        tour1d_func (state, display, gg);
    break;
    case COTOUR:
      if (!display->cpanel.tcorr1_paused)
        tourcorr_func (state, display, gg);
    break;
    default:
    break;
  }
}

/*
    if (gg->mode == TOUR2D || prev_mode == TOUR2D) {
      if (gg->mode == TOUR2D && prev_mode != TOUR2D) {
        if (!display->cpanel.t2d_paused)
          tour2d_func (on, display, gg);
      } else if (prev_mode == TOUR2D && gg->mode != TOUR2D) {
        tour2d_func (off, display, gg);
      }
    }

    if (gg->mode == TOUR1D || prev_mode == TOUR1D) {
      if (gg->mode == TOUR1D && prev_mode != TOUR1D) {
        if (!display->cpanel.t1d_paused)
          tour1d_func (on, display, gg);
      } else if (prev_mode == TOUR1D && gg->mode != TOUR1D) {
        tour1d_func (off, display, gg);
      }
    }

    if (gg->mode == COTOUR || prev_mode == COTOUR) {
      if (gg->mode == COTOUR && prev_mode != COTOUR) {
        if (!display->cpanel.tcorr1_paused)
          tourcorr_func (on, display, gg);
      } else if (prev_mode == COTOUR && gg->mode != COTOUR) {
        tourcorr_func (off, display, gg);
      }
    }
*/

void
mode_activate (splotd *sp, gint m, gboolean state, ggobid *gg) {
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  if (state == off) {
    switch (m) {
      case BRUSH:
        brush_activate (state, d, gg);
      break;
      default:
      break;
    }
  } else if (state == on) {
    switch (m) {
      case BRUSH:
        brush_activate (state, d, gg);
      break;
      default:
      break;
    }
  }
}

void
mode_set_cb (GtkWidget *widget, gint action)
{
  ggobid *gg = GGobiFromWidget(widget,true);
  GGOBI(full_mode_set)(action, gg);
}

gint
GGOBI(full_mode_set)(gint action, ggobid *gg)
{
  if (gg->current_display != NULL && gg->current_splot != NULL) {
    splotd *sp = gg->current_splot;
    displayd *display = gg->current_display;

    sp_event_handlers_toggle (sp, off);
    mode_activate (sp, gg->mode, off, gg);
    mode_submenus_activate (sp, gg->mode, off, gg);
    procs_activate (off, display, gg);

    display->cpanel.mode = action;
    mode_set (action, gg);  /* mode = action */

    sp_event_handlers_toggle (sp, on);
    mode_activate (sp, gg->mode, on, gg);
    mode_submenus_activate (sp, gg->mode, on, gg);
    procs_activate (on, display, gg);

    display_tailpipe (display, gg);
    return (action);
  } else
    return(-1);
}

extern void display_write_svg (ggobid *);
static GtkItemFactoryEntry menu_items[] = {
  { "/_File",            NULL,     NULL,             0, "<Branch>" },
  { "/File/Read ...",   
       NULL,    
       (GtkItemFactoryCallback) filename_get_r,  
       0 },
/*
 *{ "/File/Save (extend file set) ...",   
 *     NULL,    
 *     filename_get,    
 *     1 },
*/
  { "/File/Save ...",   
       NULL,    
       (GtkItemFactoryCallback) writeall_window_open,    
       2 },

#ifdef PRINTING_IMPLEMENTED
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",
       NULL,    
       (GtkItemFactoryCallback) display_write_svg,         
       0 },
#endif

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Quit",   
       "<ctrl>Q",   
       (GtkItemFactoryCallback) quit_ggobi, 
       0 },

  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Variable selection and statistics ...", 
       NULL,        
       (GtkItemFactoryCallback) vartable_open,   
       0,
       NULL },
  { "/Tools/Variable transformation ...", 
       NULL,        
       (GtkItemFactoryCallback) transform_window_open,
       0,
       NULL },
  { "/Tools/Sphering ...", 
       NULL,        
       (GtkItemFactoryCallback) sphere_panel_open,
       0,
       NULL },
#ifdef INFERENCE_IMPLEMENTED
  { "/Tools/Inference ...", 
       NULL,        
       (GtkItemFactoryCallback) NULL,  /*-- inference_window_open --*/
       0,
       NULL },
#endif
  { "/Tools/Variable jittering ...", 
       NULL,        
       (GtkItemFactoryCallback) jitter_window_open,   
       0,
       NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Case hiding and exclusion ...", 
       NULL,        
       (GtkItemFactoryCallback) exclusion_window_open,
       0,
       NULL },
  { "/Tools/Case subsetting and sampling ...", 
       NULL,        
       (GtkItemFactoryCallback) subset_window_open,   
       0,
       NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
#ifdef SMOOTH_IMPLEMENTED
  { "/Tools/Smooth ...", 
       NULL,        
       (GtkItemFactoryCallback) smooth_window_open,   
       0,
       NULL },
#endif

  { "/Tools/Missing values ...", 
       NULL,        
       (GtkItemFactoryCallback) impute_window_open,   
       0,
       NULL },

/*
 * this code in display.c actually corresponds to the "Window" menu,
 * not this one, which is really just display options.
*/
  { "/_Options", NULL, NULL, 0, "<Branch>" },
  { "/Options/Show tooltips",  
       "<ctrl>t",   
       (GtkItemFactoryCallback) main_display_options_cb,
       0,
       "<CheckItem>" },
  { "/Options/Show control _panel",  
       "<ctrl>p",   
       (GtkItemFactoryCallback) main_display_options_cb,
       1,
       "<CheckItem>" },
  { "/Options/Show _axes",  
       "<ctrl>a",   
       (GtkItemFactoryCallback) main_display_options_cb,
       2,
       "<CheckItem>" },
  { "/Options/Lay out variable circles by _row",  
       "<ctrl>r",   
       (GtkItemFactoryCallback) main_display_options_cb,
       3,
       "<CheckItem>" },

#ifdef BRUSHING_OPTIONS_IMPLEMENTED
  { "/Options/sep",  
       NULL,    
       NULL,         
       0,
       "<Separator>" },

  { "/Options/Brushing", NULL, NULL, 0, "<Branch>" },
  { "/Options/Brushing/Brush jumps to cursor",  
       NULL ,       
       (GtkItemFactoryCallback) brush_options_cb,
       0,
       "<CheckItem>" },
  { "/Options/Brushing/Update linked brushing continuously",  
       NULL ,      
       (GtkItemFactoryCallback) brush_options_cb,
       1,
       "<CheckItem>" },
#endif


  {"/Dis_playTree", NULL, NULL, 0, "<Branch>"},
  { "/DisplayTree/Displays",    
       NULL, 
       (GtkItemFactoryCallback) show_display_tree,
       2},


  { "/_Help",                NULL, NULL, 0, "<LastBranch>" },
  { "/Help/About GGobi",     NULL, NULL, 0, NULL },
  { "/Help/About help ...",  NULL, NULL, 0, NULL },
};


#ifndef AS_GGOBI_LIBRARY
/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  gtk_main_quit();
}

#endif

void 
make_ui (ggobid *gg) {
  GtkWidget *window;
  GtkWidget *hbox, *vbox;
  GList *items, *subitems;
  GtkWidget *item, *submenu;
  gchar *name;

  gg->tips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gg->main_window = window;
  GGobi_widget_set (window, gg, true);

  gtk_window_set_policy (GTK_WINDOW (window), true, true, false);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (ggobi_close), gg);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (ggobi_close), gg);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gg->main_accel_group = gtk_accel_group_new ();
  gg->main_menu_factory = get_main_menu (menu_items,
                            sizeof (menu_items) / sizeof (menu_items[0]),
                            gg->main_accel_group, window,
                            &gg->main_menubar, (gpointer) gg);

  display_menu_init (gg);

  gtk_box_pack_start (GTK_BOX (vbox), gg->main_menubar, false, false, 0);

  gtk_accel_group_lock (gg->main_accel_group);

/*
 * Step through the option menu, setting the values of toggle items
 * as appropriate.
*/
  items = gtk_container_children (GTK_CONTAINER (gg->main_menubar));
  while (items) {
    item = (GtkWidget *) items->data;
    gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child ), &name);
    if (strcmp (name, "Options") == 0) {
      submenu = GTK_MENU_ITEM (item)->submenu;
      subitems = gtk_container_children (GTK_CONTAINER (submenu));
      while (subitems) {
        item = (GtkWidget*) subitems->data;

        if (GTK_IS_CHECK_MENU_ITEM (item)) {
          gtk_label_get (GTK_LABEL (GTK_MENU_ITEM (item)->item.bin.child),
            &name);
          if (g_strcasecmp (name, "show tooltips") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
          else if (g_strcasecmp (name, "show control panel") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
          else if (g_strcasecmp (name, "show axes") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
          else if (g_strcasecmp (name, "lay out variable circles by row") == 0)
            GTK_CHECK_MENU_ITEM (item)->active = true;
  
        } else {
          /* If there are to be submenus, I have to dig them out as well. */
        }
        subitems = subitems->next;
      }
      g_list_free (subitems);
      break;  /* Finished with options menu */
    }

    items = items->next;
  }
  g_list_free (items);
/* */

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * Create a frame to hold the mode panels, set its label
 * and contents, using the default mode for the default display.
*/
  gg->mode_frame = gtk_frame_new (mode_name[gg->mode]);

  gtk_box_pack_start (GTK_BOX (hbox), gg->mode_frame, false, false, 3);
  gtk_container_set_border_width (GTK_CONTAINER (gg->mode_frame), 3);
  gtk_frame_set_shadow_type (GTK_FRAME (gg->mode_frame), GTK_SHADOW_IN);

  make_control_panels (gg);
  gtk_container_add (GTK_CONTAINER (gg->mode_frame),
                     gg->control_panel[gg->mode]);

  /*-- Variable selection panel --*/
  varpanel_make (hbox, gg);

  gtk_widget_show_all (hbox);

  gtk_widget_show_all (window);
}


const gchar * const* 
GGOBI(getOpModeNames)(int *n)
{
  /*  extern const gchar *const* GGOBI(ModeNames); */
  *n = sizeof(GGOBI(OpModeNames))/sizeof(GGOBI(OpModeNames)[0]);
  return (GGOBI(OpModeNames));
}
