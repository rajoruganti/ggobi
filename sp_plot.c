/* sp_plot.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static void splot_draw_border (splotd *, GdkDrawable *, ggobid *);
static void edges_draw (splotd *, ggobid *gg);

#ifdef _WIN32
extern void win32_draw_to_pixmap_binned (icoords *, icoords *, gint, splotd *, ggobid *gg);
extern void win32_draw_to_pixmap_unbinned (gint, splotd *, ggobid *gg);
#endif


/* colors_used now contains integers, 0:ncolors-1 */
void
splot_colors_used_get (splotd *sp, gint *ncolors_used,
  gushort *colors_used, datad *d, ggobid *gg) 
{
  gboolean new_color;
  gint i, k, m;

  *ncolors_used = 0;

  if (d == NULL || d->nrows == 0)
/**/return;
          
  /*
   * Loop once through d->color_now[], collecting the colors currently
   * in use into the colors_used[] vector.
  */
  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];
    if (d->hidden_now.els[m]) {  /*-- if it's hidden, we don't care --*/
      new_color = false;
    } else {
      new_color = true;
      for (k=0; k<*ncolors_used; k++) {
        if (colors_used[k] == d->color_now.els[m]) {
          new_color = false;
          break;
        }
      }
    }
    if (new_color) {
      colors_used[*ncolors_used] = d->color_now.els[m];
      (*ncolors_used)++;
    }
  }

  /*
   * Make sure that the current brushing color is
   * last in the list, so that it is drawn on top of
   * the pile of points.
  */
  for (k=0; k<(*ncolors_used-1); k++) {
    if (colors_used[k] == gg->color_id) {
      colors_used[k] = colors_used[*ncolors_used-1];
      colors_used[*ncolors_used-1] = gg->color_id;
      break;
    }
  }

  /* insurance -- eg if using mono drawing on a color screen */
  if (*ncolors_used == 0) {
    *ncolors_used = 1;
    colors_used[0] = d->color_now.els[0];
  }
}

gboolean
splot_plot_case (gint m, datad *d, splotd *sp, displayd *display, ggobid *gg)
{
  gboolean draw_case;

  /*-- determine whether case m should be plotted --*/
  draw_case = true;
  if (d->hidden_now.els[m])
    draw_case = false;

  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
  else if (!display->options.missings_show_p && d->nmissing > 0) {
    switch (display->displaytype) {
      case parcoords:
        if (d->missing.vals[m][sp->p1dvar])
          draw_case = false;
        break;

      case scatmat:
        if (sp->p1dvar != -1) {
          if (d->missing.vals[m][sp->p1dvar])
            draw_case = false;
        } else {
          if (d->missing.vals[m][sp->xyvars.x] ||
              d->missing.vals[m][sp->xyvars.y])
          {
            draw_case = false;
          }
        }
        break;

      case tsplot:
        if (d->missing.vals[m][sp->xyvars.y]|| 
            d->missing.vals[m][sp->xyvars.x])
          draw_case = false;
        break;

      case scatterplot:
        break;
      default:
	break;
    }
  }
  return draw_case;
}


void
splot_draw_to_pixmap0_unbinned (splotd *sp, ggobid *gg)
{
  gint k;
#ifndef _WIN32
  gint i, m, n;
#endif
  gushort current_color;
  gint ncolors_used;
  gushort colors_used[NCOLORS+2];
  GtkWidget *da = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gboolean draw_case;
  /*
   * since parcoords and tsplot each have their own weird way
   * of drawing line segments, it's necessary to get the point
   * colors before drawing those lines even if we're not drawing
   * points.
  */
  gboolean loop_over_points =
    display->options.points_show_p ||
    display->displaytype == parcoords ||
    display->displaytype == tsplot;

  if (gg->plot_GC == NULL) {
    init_plot_GC (sp->pixmap0, gg);
  }

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);

  if (!gg->mono_p && loop_over_points) {
    splot_colors_used_get (sp, &ncolors_used, colors_used, d, gg);

    /*
     * Now loop through colors_used[], plotting the points of each
     * color.  This avoids the need to reset the foreground so often.
     * On the other hand, it requires more looping.
    */
    for (k=0; k<ncolors_used; k++) {
      current_color = colors_used[k];
      gdk_gc_set_foreground (gg->plot_GC,
        &gg->default_color_table[current_color]);

#ifdef _WIN32
      win32_draw_to_pixmap_unbinned (current_color, sp, gg);
#else
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        draw_case = splot_plot_case (m, d, sp, display, gg);

        if (draw_case && d->color_now.els[m] == current_color) {
          if (display->options.points_show_p)
            draw_glyph (sp->pixmap0, &d->glyph_now[m], sp->screen, m, gg);

          /*-- whiskers: parallel coordinate and time series plots --*/
          if (display->displaytype == parcoords ||
              display->displaytype == tsplot)
          {
            if (display->options.whiskers_show_p) {
              if (display->displaytype == parcoords) {
                n = 2*m;
                gdk_draw_line (sp->pixmap0, gg->plot_GC,
                  sp->whiskers[n].x1, sp->whiskers[n].y1,
                  sp->whiskers[n].x2, sp->whiskers[n].y2);
                n++;
                gdk_draw_line (sp->pixmap0, gg->plot_GC,
                  sp->whiskers[n].x1, sp->whiskers[n].y1,
                  sp->whiskers[n].x2, sp->whiskers[n].y2);
              }
              else {
                gdk_draw_line (sp->pixmap0, gg->plot_GC,
                  sp->whiskers[m].x1, sp->whiskers[m].y1,
                  sp->whiskers[m].x2, sp->whiskers[m].y2);
              }
            }
          }
        }
      }
#endif
    }  /* deal with mono later */
  }

  return;
}

void
splot_draw_to_pixmap0_binned (splotd *sp, ggobid *gg)
{
  icoords loc_clear0, loc_clear1;
#ifndef _WIN32
  gint ih, iv;
  gint i, m, n;
#endif
  gint k;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  icoords *bin0 = &gg->plot.bin0;
  icoords *bin1 = &gg->plot.bin1;
  icoords *loc0 = &gg->plot.loc0;
  icoords *loc1 = &gg->plot.loc1;

  gushort current_color;
  gint ncolors_used;
  gushort colors_used[NCOLORS+2];

  if (gg->plot_GC == NULL)
    init_plot_GC (sp->pixmap0, gg);

/*
 * Instead of clearing and redrawing the entire pixmap0, only
 * clear what's necessary.
*/

  get_extended_brush_corners (bin0, bin1, d, sp);

/*
 * Determine locations of bin corners: upper left edge of loc0;
 * lower right edge of loc1.
*/
  loc0->x = (gint)
    ((gfloat) bin0->x / (gfloat) d->brush.nbins * (sp->max.x+1.0));
  loc0->y = (gint)
    ((gfloat) bin0->y / (gfloat) d->brush.nbins * (sp->max.y+1.0));
  loc1->x = (gint)
    ((gfloat) (bin1->x+1) / (gfloat) d->brush.nbins * (sp->max.x+1.0));
  loc1->y = (gint)
    ((gfloat) (bin1->y+1) / (gfloat) d->brush.nbins * (sp->max.y+1.0));

/*
 * Clear an area a few pixels inside that region.  Watch out
 * for border effects.
*/
  loc_clear0.x = (bin0->x == 0) ? 0 : loc0->x + BRUSH_MARGIN;
  loc_clear0.y = (bin0->y == 0) ? 0 : loc0->y + BRUSH_MARGIN;
  loc_clear1.x = (bin1->x == d->brush.nbins-1) ? sp->max.x :
                                               loc1->x - BRUSH_MARGIN;
  loc_clear1.y = (bin1->y == d->brush.nbins-1) ? sp->max.y :
                                               loc1->y - BRUSH_MARGIN;

  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      true,  /* fill */
                      loc_clear0.x, loc_clear0.y,
                      1 + loc_clear1.x - loc_clear0.x ,
                      1 + loc_clear1.y - loc_clear0.y);

  if (display->options.points_show_p) {
    if (!gg->mono_p) {

      splot_colors_used_get (sp, &ncolors_used, colors_used, d, gg); 

      /*
       * Now loop through colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        gdk_gc_set_foreground (gg->plot_GC,
          &gg->default_color_table[current_color]);

#ifdef _WIN32
        win32_draw_to_pixmap_binned (bin0, bin1, current_color, sp, gg);
#else
        for (ih=bin0->x; ih<=bin1->x; ih++) {
          for (iv=bin0->y; iv<=bin1->y; iv++) {
            for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
              i = d->rows_in_plot[d->brush.binarray[ih][iv].els[m]];
              if (!d->hidden_now.els[i] &&
                   d->color_now.els[i] == current_color)
              {
                draw_glyph (sp->pixmap0, &d->glyph_now[i], sp->screen, i, gg);

                /* parallel coordinate plot whiskers */
                if (display->displaytype == parcoords) {
                  if (display->options.whiskers_show_p) {
                    n = 2*i;
                    gdk_draw_line (sp->pixmap0, gg->plot_GC,
                      sp->whiskers[n].x1, sp->whiskers[n].y1,
                      sp->whiskers[n].x2, sp->whiskers[n].y2);
                    n++;
                    gdk_draw_line (sp->pixmap0, gg->plot_GC,
                      sp->whiskers[n].x1, sp->whiskers[n].y1,
                      sp->whiskers[n].x2, sp->whiskers[n].y2);
                  }
                } else if(display->displaytype == tsplot) {
                  gdk_draw_line (sp->pixmap0, gg->plot_GC,
                    sp->whiskers[m].x1, sp->whiskers[m].y1,
                    sp->whiskers[m].x2, sp->whiskers[m].y2);
                }
              }
            }
          }
        }
#endif
      }
    }
  }

  return;
}


/*------------------------------------------------------------------------*/
/*                   plot labels: variables                               */
/*------------------------------------------------------------------------*/

static void
splot_add_plot_labels (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint dtype = display->displaytype;
  datad *d = display->d;

  gboolean proceed = (cpanel->projection == XYPLOT ||
                      cpanel->projection == P1PLOT ||
                      cpanel->projection == PCPLOT ||
                      cpanel->projection == TSPLOT ||
                      cpanel->projection == SCATMAT);
  if (!proceed)
    return;

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);

  if (dtype == scatterplot || dtype == scatmat) {
    if ((dtype == scatterplot && cpanel->projection == XYPLOT) ||
        (dtype == scatmat && sp->p1dvar == -1))
    {
      gdk_text_extents (style->font, 
        d->vartable[ sp->xyvars.x ].collab_tform,
        strlen (d->vartable[ sp->xyvars.x ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        d->vartable[ sp->xyvars.x ].collab_tform);

      gdk_text_extents (style->font, 
        d->vartable[ sp->xyvars.y ].collab_tform,
        strlen (d->vartable[ sp->xyvars.y ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        5, 5 + ascent + descent,
        d->vartable[ sp->xyvars.y ].collab_tform);
    }

    if ((dtype == scatterplot && cpanel->projection == P1PLOT) ||
        (dtype == scatmat && sp->p1dvar != -1))
    {
      gdk_text_extents (style->font,
        d->vartable[ sp->p1dvar ].collab_tform,
        strlen (d->vartable[ sp->p1dvar ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        d->vartable[ sp->p1dvar ].collab_tform);
    }

  } else if (dtype == parcoords) {

    gdk_text_extents (style->font,
      d->vartable[ sp->p1dvar ].collab_tform,
      strlen (d->vartable[ sp->p1dvar ].collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      5,
      sp->max.y - 5,
      d->vartable[ sp->p1dvar ].collab_tform);

  } else if (dtype ==tsplot) {

    GList *l = display->splots;
    if (l->data == sp) {
      gdk_text_extents (style->font, 
        d->vartable[ sp->xyvars.x ].collab_tform,
        strlen (d->vartable[ sp->xyvars.x ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        d->vartable[ sp->xyvars.x ].collab_tform);
    }
    gdk_text_extents (style->font, 
      d->vartable[ sp->xyvars.y ].collab_tform,
      strlen (d->vartable[ sp->xyvars.y ].collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      5, 5 + ascent + descent,
      d->vartable[ sp->xyvars.y ].collab_tform);
  }

}

/*------------------------------------------------------------------------*/
/*               case highlighting for points (and edges?)                */
/*------------------------------------------------------------------------*/

static void
splot_add_whisker_cues (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint n;
  displayd *display = sp->displayptr;
  datad *d = display->d;

  if (display->options.whiskers_show_p) {
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC,
      &gg->default_color_table[d->color_now.els[k]]);

    n = 2*k;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}

#define DIAMOND_DIM 5

/*-- draw a diamond around the current case --*/
static void
splot_add_diamond_cue (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint diamond_dim = DIAMOND_DIM;
  GdkPoint diamond[5];

  diamond[0].x = diamond[4].x = sp->screen[k].x - diamond_dim;
  diamond[0].y = diamond[4].y = sp->screen[k].y;
  diamond[1].x = sp->screen[k].x;
  diamond[1].y = sp->screen[k].y - diamond_dim;
  diamond[2].x = sp->screen[k].x + diamond_dim;
  diamond[2].y = sp->screen[k].y;
  diamond[3].x = sp->screen[k].x;
  diamond[3].y = sp->screen[k].y + diamond_dim;

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_lines (drawable, gg->plot_GC, diamond, 5);
}

static void
splot_add_record_label (gboolean nearest, gint k, splotd *sp,
  GdkDrawable *drawable, ggobid *gg)
{
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  datad *d = dsp->d;
  gint j;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint diamond_dim = DIAMOND_DIM;
  gchar *lbl;

  /*-- add the label last so it will be in front of other markings --*/
  if (cpanel->identify_display_type == ID_CASE_LABEL)
    lbl = (gchar *) g_array_index (d->rowlab, gchar *, k);
  else {
    gint proj = projection_get (gg);
    switch (proj) {
      case P1PLOT:
        lbl = g_strdup_printf ("%g", d->tform.vals[k][sp->p1dvar]);
      break;
      case XYPLOT:
        lbl = g_strdup_printf ("%g, %g",
          d->tform.vals[k][sp->xyvars.x], d->tform.vals[k][sp->xyvars.y]);
      break;

      case TOUR1D:
        for (j=0; j<dsp->t1d.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->t1d.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->t1d.vars.els[j]]);
        }
      break;

      case TOUR2D:
        for (j=0; j<dsp->t2d.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->t2d.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->t2d.vars.els[j]]);
        }
      break;

      case COTOUR:
        for (j=0; j<dsp->tcorr1.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->tcorr1.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->tcorr1.vars.els[j]]);
        }
        for (j=0; j<dsp->tcorr2.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%s; %g", lbl,
              d->tform.vals[k][dsp->tcorr2.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->tcorr2.vars.els[j]]);
        }
      break;
    }
  }
  gdk_text_extents (style->font, lbl, strlen (lbl),
    &lbearing, &rbearing, &width, &ascent, &descent);

  /*-- underline the nearest point label?  --*/
  if (sp->screen[k].x <= sp->max.x/2) {
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      sp->screen[k].x+diamond_dim,
      sp->screen[k].y-diamond_dim,
      lbl);
    if (nearest)
      gdk_draw_line (drawable, gg->plot_GC,
        sp->screen[k].x+diamond_dim, sp->screen[k].y-diamond_dim+1,
        sp->screen[k].x+diamond_dim+width, sp->screen[k].y-diamond_dim+1);
      
  } else {
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      sp->screen[k].x - width - diamond_dim,
      sp->screen[k].y - diamond_dim,
      lbl);
    if (nearest)
      gdk_draw_line (drawable, gg->plot_GC,
        sp->screen[k].x - width - diamond_dim, sp->screen[k].y - diamond_dim+1,
        sp->screen[k].x - diamond_dim, sp->screen[k].y - diamond_dim+1);
  }

}

/*-- add the nearest_point and sticky labels, plus a diamond for emphasis --*/
void
splot_add_identify_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;

/*--
   unsatisfactory: use ids and work out whether we're talking
   about display->d or display->e.  nearest has to be 
   propagated for linking.  [don't quite understand this comment;
   time will tell]
--*/
  if (k >= d->nrows)
    return;

  /*-- draw a thickened line to highlight the current case --*/
  if (nearest) {
    if (dsp->displaytype == parcoords) {
      splot_add_whisker_cues (k, sp, drawable, gg);
    } else {  /*-- for all displays other than the parcoords plot --*/
      splot_add_diamond_cue (k, sp, drawable, gg);
    }
  }

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  splot_add_record_label (nearest, k, sp, drawable, gg);
}

void
splot_add_edgeedit_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  if (cpanel->ee_adding_p) {
    if (gg->edgeedit.a == -1) {  /*-- looking for starting point --*/
      splot_add_diamond_cue (k, sp, drawable, gg);
    }
  }
}

static void
splot_add_point_cues (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  gint id;
  GSList *l;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint mode = mode_get (gg);

  if (d->nearest_point != -1) {
    if (mode == IDENT || mode == MOVEPTS)
      splot_add_identify_cues (sp, drawable, d->nearest_point, true, gg);
    else if (mode == EDGEED)
      splot_add_edgeedit_cues (sp, drawable, d->nearest_point, true, gg);
  }

  if (d->sticky_ids != NULL &&
      g_slist_length (d->sticky_ids) > 0)
  {
    for (l = d->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      /*-- false = !nearest --*/
      splot_add_identify_cues (sp, drawable, id, false, gg);
    }
  }
}


/*------------------------------------------------------------------------*/
/*                   line drawing routines                                */
/*------------------------------------------------------------------------*/


void
edges_draw (splotd *sp, ggobid *gg)
{
  gint j, k;
  gint a, b;
  gushort current_color;
  gushort colors_used[NCOLORS+2];
  gint ncolors_used = 0;
  gboolean doit;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;
  endpointsd *endpoints;
  gboolean edges_show_p, arrowheads_show_p;
  gint nels = d->rowid.idv.nels;

  if (e == (datad *) NULL || e->edge.n == 0) {
/**/return;
  }

  if (nels == 0) {  /*-- d has no record ids --*/
/**/return;
  }

  edges_show_p = (display->options.edges_directed_show_p ||
                  display->options.edges_undirected_show_p);
  arrowheads_show_p = (display->options.edges_directed_show_p ||
                       display->options.edges_arrowheads_show_p);
  if (!gg->mono_p && (edges_show_p || arrowheads_show_p)) {

    splot_colors_used_get (sp, &ncolors_used, colors_used, e, gg);

    /*
     * Now loop through colors_used[], plotting the glyphs of each
     * color in a group.
    */
    endpoints = e->edge.endpoints;
    for (k=0; k<ncolors_used; k++) {
      gint nl = 0;
      current_color = colors_used[k];

      for (j=0; j<e->edge.n; j++) {
        if (e->hidden_now.els[j]) {
          doit = false;
        } else {
          if (endpoints[j].a >= nels || endpoints[j].b >= nels) {
            doit = false;
            break;
          }
          a = d->rowid.idv.els[endpoints[j].a];
          b = d->rowid.idv.els[endpoints[j].b];

          doit = (!d->hidden_now.els[a] && !d->hidden_now.els[b]);

        /* If not plotting imputed values, and one is missing, skip it */
/*
          if (!plot_imputed_values && plotted_var_missing(from, to, gg))
            doit = false;
*/
        }

        if (doit) {
          if (e->color_now.els[j] == current_color) {

            if (edges_show_p) {
              sp->edges[nl].x1 = sp->screen[a].x;
              sp->edges[nl].y1 = sp->screen[a].y;
              sp->edges[nl].x2 = sp->screen[b].x;
              sp->edges[nl].y2 = sp->screen[b].y;
            }

            if (arrowheads_show_p) {
              /*
               * Add thick piece of the lines to suggest a directional arrow
              */
              sp->arrowheads[nl].x1 =
                (gint) (.2*sp->screen[a].x + .8*sp->screen[b].x);
              sp->arrowheads[nl].y1 =
                (gint) (.2*sp->screen[a].y + .8*sp->screen[b].y);
              sp->arrowheads[nl].x2 = sp->screen[b].x;
              sp->arrowheads[nl].y2 = sp->screen[b].y;
            }
            nl++;
          }  /*-- end if ... == current_color --*/
        }
      }
      if (!gg->mono_p)
        gdk_gc_set_foreground (gg->plot_GC,
          &gg->default_color_table[current_color]);

      if (edges_show_p)
        gdk_draw_segments (sp->pixmap1, gg->plot_GC, sp->edges, nl);

      if (arrowheads_show_p) {
        gdk_gc_set_line_attributes (gg->plot_GC,
          3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
        gdk_draw_segments (sp->pixmap1, gg->plot_GC, sp->arrowheads, nl);
        gdk_gc_set_line_attributes (gg->plot_GC,
          0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
      }
    }
  }
}

void
splot_nearest_edge_highlight (splotd *sp, gint k, gboolean nearest, ggobid *gg) {
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  datad *e = dsp->e;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gchar *lbl;
  gint xp, yp;
  gboolean draw_edge;

  if (k >= e->edge.n)
    return;

  /*-- not yet using rowid.idv --*/
  if (e->hidden_now.els[k])
    return;

  draw_edge = (dsp->options.edges_undirected_show_p ||
               dsp->options.edges_directed_show_p);

  /*-- draw a thickened line, and add a label if nearest==true --*/
  if (draw_edge) {
    endpointsd *endpoints = e->edge.endpoints;
    gint a = d->rowid.idv.els[endpoints[k].a];
    gint b = d->rowid.idv.els[endpoints[k].b];

    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC,
      &gg->default_color_table[e->color_now.els[k]]);

    gdk_draw_line (sp->pixmap1, gg->plot_GC,
      sp->screen[a].x, sp->screen[a].y,
      sp->screen[b].x, sp->screen[b].y);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);

    if (nearest) {
      /*-- add the label last so it will be in front of other markings --*/
      lbl = (gchar *) g_array_index (e->rowlab, gchar *, k);
      gdk_text_extents (style->font,
        lbl, strlen (lbl),
        &lbearing, &rbearing, &width, &ascent, &descent);

      if (sp->screen[a].x > sp->screen[b].x) {gint itmp=b; b=a; a=itmp;}
      xp = (sp->screen[b].x - sp->screen[a].x)/2 + sp->screen[a].x;
      if (sp->screen[a].y > sp->screen[b].y) {gint itmp=b; b=a; a=itmp;}
      yp = (sp->screen[b].y - sp->screen[a].y)/2 + sp->screen[a].y;

      /*-- underline the nearest point label?  --*/
      gdk_draw_string (sp->pixmap1, style->font, gg->plot_GC,
        xp, yp, lbl);
      if (nearest)
        gdk_draw_line (sp->pixmap1, gg->plot_GC,
          xp, yp+1,
          xp+width, yp+1);
    }
  }
}

/*------------------------------------------------------------------------*/
/*                 draw the border indicating current splot               */
/*------------------------------------------------------------------------*/

static void
splot_draw_border (splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

    gdk_draw_rectangle (drawable, gg->plot_GC,
      FALSE, 1, 1, sp->da->allocation.width-3, sp->da->allocation.height-3);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}


/*------------------------------------------------------------------------*/
/*    getting from pixmap0 to pixmap1, then pixmap1 to the window         */
/*------------------------------------------------------------------------*/

void
splot_pixmap0_to_pixmap1 (splotd *sp, gboolean binned, ggobid *gg) {
  GtkWidget *w = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  icoords *loc0 = &gg->plot.loc0;
  icoords *loc1 = &gg->plot.loc1;
  datad *e = display->e;
  datad *d = display->d;
  gint mode = mode_get (gg);

#if 0
      if(gg->plot_GC == NULL) {
	  init_plot_GC(w->window, gg);
	  fprintf(stderr, "Mmmm\n");
	  return;
      }
#endif
  if (!binned) {
    gdk_draw_pixmap (sp->pixmap1, gg->plot_GC, sp->pixmap0,
                     0, 0, 0, 0,
                     w->allocation.width,
                     w->allocation.height);
  }
  else {
    gdk_draw_pixmap (sp->pixmap1, gg->plot_GC, sp->pixmap0,
                      loc0->x, loc0->y,
                      loc0->x, loc0->y,
                      1 + loc1->x - loc0->x, 1 + loc1->y - loc0->y);
  }

  if (display->options.edges_undirected_show_p ||
      display->options.edges_arrowheads_show_p ||
      display->options.edges_directed_show_p)
  {
    if (display->displaytype == scatterplot || display->displaytype == scatmat)
    {
        edges_draw (sp, gg);
        if (e->nearest_point != -1)
          splot_nearest_edge_highlight (sp, e->nearest_point, true, gg);
    }
  }
     
  splot_add_plot_labels (sp, sp->pixmap1, gg);  /*-- axis labels --*/

  /*-- identify, move points, edge editing --*/
  splot_add_point_cues (sp, sp->pixmap1, gg);  

  if (sp == gg->current_splot) {
    splot_draw_border (sp, sp->pixmap1, gg);

    switch (mode) {
      case BRUSH:
        brush_draw_brush (sp, sp->pixmap1, d, gg);
        brush_draw_label (sp, sp->pixmap1, d, gg);
      break;
      case SCALE:
        scaling_visual_cues_draw (sp, sp->pixmap1, gg);
      break;
    }
  }
}

void
splot_pixmap1_to_window (splotd *sp, ggobid *gg) {
  GtkWidget *w = sp->da;
#if 0
      if(gg->plot_GC == NULL) {
	  init_plot_GC(w->window, gg);
	 fprintf(stderr, "Mmmm");
         return;
      }
#endif
  gdk_draw_pixmap (sp->da->window, gg->plot_GC, sp->pixmap1,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

/*------------------------------------------------------------------------*/
/*                   convenience routine                                  */
/*------------------------------------------------------------------------*/

void
splot_redraw (splotd *sp, enum redrawStyle redraw_style, ggobid *gg) {

  /*-- sometimes the first draw happens before configure is called --*/
  if (sp == NULL || sp->da == NULL || sp->pixmap0 == NULL) {
    return;
  }

  switch (redraw_style) {
    case FULL:
      splot_draw_to_pixmap0_unbinned (sp, gg);
      splot_pixmap0_to_pixmap1 (sp, false, gg);  /* false = not binned */
    break;
    case QUICK:
      splot_pixmap0_to_pixmap1 (sp, false, gg);  /* false = not binned */
    break;
    case BINNED:
      splot_draw_to_pixmap0_binned (sp, gg);
      splot_pixmap0_to_pixmap1 (sp, true, gg);  /* true = binned */
    break;
    case EXPOSE:
    break;

    case NONE:
    break;
  }

  if (redraw_style != NONE) {
    splot_pixmap1_to_window (sp, gg);

    /*
     * Somehow the very first window is initially drawn without a border. 
     * I ought to be able to fix that more nicely some day, but in the
     * meantime, what's an extra rectangle?
    */
    if (sp == gg->current_splot) 
      splot_draw_border (sp, sp->da->window, gg);
  }

  sp->redraw_style = EXPOSE;
}
