#include <gtk/gtk.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include "vars.h"
#include "externs.h"
#include "writedata.h"

gboolean write_binary_data (gchar *, gint *, gint, gint *, gint, ggobid *);
gboolean write_ascii_data (gchar *, gint *, gint, gint *, gint, ggobid *);
gboolean save_collabels (gchar *, gint *colv, gint nc, ggobid *gg);
gboolean save_rowlabels (gchar *, gint *rowv, gint nr, ggobid *gg);
gboolean brush_save_colors (gchar *, gint *, gint, ggobid *);
gboolean brush_save_erase (gchar *, gint *, gint, ggobid *);
gboolean brush_save_glyphs (gchar *, gint *, gint, ggobid *);
gboolean save_lines (gchar *, gboolean, gboolean, gint *, gint, ggobid *);
gint linedata_get (endpointsd *, gshort *, gint *, gint, ggobid *);

static gint
set_rowv (gint *rowv, gchar *rootname, ggobid *gg)
{
  gint i, j, k;
  gint nrows = 0;
  gchar *fname;
  FILE *fp;
  gchar *message;
  GSList *l;

  switch (gg->save.row_ind) {

    case DISPLAYEDROWS:
    /*
     * Otherwise just copy the row numbers representing unerased
     * points into rowv, and return their count.
    */

      for (i=0, j=0; i<gg->nrows_in_plot; i++) {
        k = gg->rows_in_plot[i];
        if (!gg->hidden_now[k])
          rowv[j++] = k;
      }
      nrows = j;
      break;

    case LABELLEDROWS:
      /*
       * Otherwise just copy the row numbers representing sticky
       * labels into rowv, and return their count.
      */
      for (l = gg->sticky_ids; l; l = l->next)
        rowv[i] = GPOINTER_TO_INT (l->data);
      nrows = g_slist_length (gg->sticky_ids);
      break;

    case ALLROWS:
    /* 
     * Finally, let rowv be (0,1,2,,,,gg->nrows)
    */
      for (i=0; i<gg->nrows; i++)
        rowv[i] = i;
      nrows = gg->nrows;
      break;

    default:
      fprintf (stderr, "error in row_ind; impossible type %d\n",
        gg->save.row_ind);
      break;
  }

  return (nrows);
}

static gint
set_colv (gint *colv, gchar *rootname, ggobid *gg)
{
  gint i;
  gint ncols = 0;
  gchar *fname;
  gchar *message;
  FILE *fp;

  switch (gg->save.column_ind) {

    case ALLCOLS:
      /* 
       * let colv be (0,1,2,,,,gg->ncols)
      */
      for (i=0; i<gg->ncols; i++)
        colv[i] = i;
      ncols = gg->ncols;
      break;

    case SELECTEDCOLS:
      ncols = selected_cols_get (colv, false, gg);
      if (ncols == 0)
        ncols = plotted_cols_get (colv, false, gg);
      break;
    

    default:
      fprintf (stderr, "error in col_ind; impossible type %d\n",
        gg->save.column_ind);
      break;
  }
  
  return (ncols);
}

gboolean
write_ascii_data (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  ggobid *gg)
{
  gchar fname[164];
  gchar *message;
  FILE *fp;
  gint i, j, ir, jc;
  gfloat **fdatap;

  if (gg->save.stage == RAWDATA || gg->save.stage == TFORMDATA)
    sprintf (fname, "%s.dat", rootname);

  if ((fp = fopen (fname, "w")) == NULL) {
    message = g_strdup_printf ("The file '%s' can not be created\n", fname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {
    fdatap = (gg->save.stage == RAWDATA) ? gg->raw.data : gg->tform2.data;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++) {
        jc = colv[j];
        if (gg->nmissing > 0 && gg->missing.data[ir][jc]) {
          if (gg->save.missing_ind == MISSINGSNA) {
            fprintf (fp, "NA ");
          }  else if (gg->save.missing_ind == MISSINGSDOT) {
            fprintf (fp, ". ");
          } 
        } 
        else {
          fprintf (fp, "%g ", fdatap[ir][jc]);
        } 
      }
      fprintf (fp, "\n");
    }

    fclose(fp);
    return true;
  }
}

void
strip_blanks (gchar *str)
{
  gint src, dest;

  for (src=0, dest=0; src<(gint)(strlen(str)+1); src++)
    if (str[src] != ' ')
      str[dest++] = str[src];
}

gboolean
ggobi_file_set_create (gchar *rootname, ggobid *gg)
{
  gint nr, nc, nvgr;
  gint *rowv, *colv;
  gchar *fname;
  FILE *fp;
  gint i, j, k;
  gboolean skipit;

  /*
   * An inconsistency:  Can't save binary data and still
   * write out "na"
  */
  if (gg->save.format == BINARYDATA &&
      gg->nmissing > 0 &&
        (gg->save.missing_ind == MISSINGSNA ||
         gg->save.missing_ind == MISSINGSDOT))
  {
    gchar *message = g_strdup_printf (
      "Sorry, GGobi can't write 'NA' or '.' in binary format.");
    quick_message (message, false);
    g_free (message);
    return false;
  }

/* Step 1: verify that the rootname is writable */

  if ((fp = fopen (rootname, "w")) == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s' can not be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {
    fclose (fp);
  }

/* Determine the rows to be saved */
  rowv = (gint *) g_malloc (gg->nrows * sizeof (gint));
  nr = set_rowv (rowv, rootname, gg);
  if (nr == 0) {
    gchar *message = g_strdup_printf (
      "You have not successfully specified any rows; sorry");
    quick_message (message, false);
    g_free (message);
    g_free ((gchar *) rowv);
    return false;
  }

/* Determine the columns to be saved */
  colv = (gint *) g_malloc (gg->ncols * sizeof (gint));
  nc = set_colv (colv, rootname, gg);
  if (nc == 0) {
    gchar *message = g_strdup_printf (
      "You have not successfully specified any columns; sorry");
    quick_message (message, false);
    g_free (message);
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

  /*
   * Save .dat first:  ascii or binary, raw or tform, missings as
   * 'na' or as currently imputed values
  */
  if (gg->save.format == BINARYDATA) { 
    if (write_binary_data (rootname, rowv, nr, colv, nc, gg) == 0) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  } else {
    if (write_ascii_data (rootname, rowv, nr, colv, nc, gg) == 0) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/* Save column labels */
  if (!save_collabels (rootname, colv, nc, gg)) {
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

/* Save row labels */
  if (!save_rowlabels (rootname, rowv, nr, gg)) {
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

/* Save colors */
  skipit = true;
  /*-- if no color differs from the default color, don't save colors --*/
  for (i=0; i<nr; i++) {
    if (gg->color_now[rowv[i]] != 0) {
      skipit = false;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_colors (rootname, rowv, nr, gg)) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/* Save glyphs */
  skipit = true;
  /*-- if no glyph differs from the default, don't save glyphs --*/
  for (i=0; i<nr; i++) {
    if (gg->glyph_now[rowv[i]].type != gg->glyph_0.type ||
        gg->glyph_now[rowv[i]].size != gg->glyph_0.size)
    {
      skipit = false;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_glyphs (rootname, rowv, nr, gg)) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/* Save erase -- unless using erase vector to choose what to copy */
  if (gg->save.row_ind != DISPLAYEDROWS) {
    skipit = true;
    /*-- if nothing is erased, skip it --*/
    for (i=0; i<nr; i++) {
      if (gg->hidden[rowv[i]] == 1) {
        skipit = false;
        break;
      }
    }
    if (!skipit) {
      if (!brush_save_erase (rootname, rowv, nr, gg)) {
        g_free ((gchar *) rowv);
        g_free ((gchar *) colv);
        return false;
      }
    }
  }

/* Save lines -- and line colors */
  if (gg->save.lines_p) {

    /*-- decide whether to save line colors --*/
    skipit = true;
    for (k=0; k<gg->nsegments; k++) {
      if (gg->line.color_now.data[k] != 0) {
        skipit = false;
        break;
      }
    }

    fprintf(stderr, ".. saving %s.lines ...\n", rootname);
    if (!save_lines (rootname, true, !skipit, rowv, nr, gg)) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }


/* Save vgroups */
  nvgr = nvgroups (gg);
  if (nvgr != gg->ncols) {
    fname = g_strdup_printf ("%s.vgroups", rootname);
    fp = fopen (fname, "w");
    g_free (fname);

    if (fp == NULL) {
      gchar *message = g_strdup_printf (
        "The file '%s.vgroups' can not be opened for writing\n", rootname);
      quick_message (message, false);
      g_free (message);
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    } else {
      for (j=0; j<nc; j++)
        fprintf (fp, "%d ", gg->vardata[colv[j]].groupid + 1);
      fprintf (fp, "\n");
      fclose (fp);
    }
  }

/* Save rgroups */
  if (gg->nrgroups > 0) {
    sprintf (fname, "%s.rgroups", rootname);
    if ( (fp = fopen (fname, "w")) == NULL) {
      gchar *message = g_strdup_printf (
        "The file '%s' can not be opened for writing\n", fname);
      quick_message (message, false);
      g_free (message);
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    } else {
      for (j=0; j<nr; j++)
        fprintf (fp, "%ld ", gg->rgroup_ids[rowv[j]] + 1);
      fprintf (fp, "\n");
      fclose (fp);
    }
  }

/*
 * Continue saving files: .doc?
 * Don't bother with .missing, .nlinkable
*/

  g_free ((char *) rowv);
  g_free ((char *) colv);

  return true;
}

/*
 * Third section: routines that will be used by both the
 * first and the second sections to save individual files.
*/

gboolean
write_binary_data (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  ggobid *gg)
{
  gchar *fname;
  FILE *fp;
  gint i, j, ir, jc;
  gfloat xfoo;
  gfloat **datap;

  if (rowv == (gint *) NULL) {
    rowv = (gint *) g_malloc (nr * sizeof(gint));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = g_strdup_printf ("%s.bin", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s.bin' can not be created\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {

    /* First the number of rows and columns */
    fwrite ((gchar *) &nr, sizeof (nr), 1, fp);
    fwrite ((gchar *) &nc, sizeof (nc), 1, fp);

    datap = (gg->save.stage == RAWDATA) ? gg->raw.data : gg->tform2.data;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++)
      {
        if (colv == (gint *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */
        if (gg->nmissing > 0 && gg->missing.data[i][j])
          xfoo = FLT_MAX;
        else
          xfoo = datap[ir][jc];
        fwrite ((gchar *) &xfoo, sizeof (xfoo), 1, fp);
      }
    }

    fclose (fp);
    return (true);
  }
}

gboolean
save_collabels (gchar *rootname, gint *colv, gint nc, ggobid *gg)
{
  gint j;
  FILE *fp;
  gchar *fname;

  fname = g_strdup_printf ("%s.col", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "Failed to open %s.col for writing.\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  }
  else {
    if (gg->save.stage == RAWDATA) {
      for (j=0; j<nc; j++)
        fprintf (fp, "%s\n", gg->vardata[colv[j]].collab);
    } else {  /*-- TFORMDATA --*/
      for (j=0; j<nc; j++)
        fprintf (fp, "%s\n", gg->vardata[colv[j]].collab_tform);
    }
    fclose (fp);
    return true;
  }
}

gboolean
save_rowlabels (gchar *rootname, gint *rowv, gint nr, ggobid *gg)
{
  gint i;
  FILE *fp;
  gchar *fname;

  fname = g_strdup_printf ("%s.row", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf ("Failed to open %s.row for writing.\n",
      rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  }
  else
  {
    for (i=0; i<nr; i++)
      fprintf (fp, "%s\n",  g_array_index (gg->rowlab, gchar *, rowv[i]));
    fclose(fp);
    return true;
  }
}

gboolean
brush_save_colors (gchar *rootname, gint *rowv, gint nr, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;

  if (gg->mono_p)
    return true;

  fname = g_strdup_printf ("%s.colors", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL)
  {
    gchar *message = g_strdup_printf (
      "The file '%s.colors' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return (false);
  }
  else
  {
    for (i=0; i<nr; i++)
      fprintf (fp, "%d\n", gg->color_now[rowv[i]]);

    if (fclose (fp) == EOF)
      fprintf(stderr, "error in writing color vector\n");
  }

  return (true);
}

gboolean
brush_save_glyphs (gchar *rootname, gint *rowv, gint nr, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;
  gchar *gstr;

  fname = g_strdup_printf ("%s.glyphs", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL)
  {
    gchar *message = g_strdup_printf (
      "The file '%s.colors' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    g_free (fname);
    return false;

  } else {

    for (i=0; i<nr; i++) {
      switch (gg->glyph_ids[i].type) {
        case PLUS_GLYPH:
          gstr = "+";
          break;
        case X_GLYPH:
          gstr = "x";
          break;
        case OPEN_RECTANGLE:
          gstr = "or";
          break;
        case FILLED_RECTANGLE:
          gstr = "fr";
          break;
        case OPEN_CIRCLE:
          gstr = "oc";
          break;
        case FILLED_CIRCLE:
          gstr = "fc";
          break;
        case POINT_GLYPH:
          gstr = ".";
          break;
      }

      fprintf (fp, "%s %d\n", gstr, gg->glyph_ids[i].size);
    }
    if (fclose (fp) == EOF)
      fprintf (stderr, "error in writing glyphs vector\n");
    return true;
  }
}

gboolean
brush_save_erase (gchar *rootname, gint *rowv, gint nr, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;

  fname = g_strdup_printf ("%s.erase", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s.erase' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  }

  for (i=0; i<nr; i++)
    fprintf(fp, "%ld\n", (glong) gg->hidden[rowv[i]]);

  fclose(fp);
  return true;
}

/*------------------------------------------------------------------------*/
/*                 Saving lines and line attributes                       */
/*------------------------------------------------------------------------*/

gint
linedata_get (endpointsd *tlinks, gshort *tcolors,
  gint *rowv, gint nr, ggobid *gg)
{
/*
 * For each end of the link, determine whether the
 * point is included or not.  This could be darn slow.
 * Rely on a < b.
*/
  gint nl = 0;
  gint i, k;
  gint a, b, start_a, start_b;

  for (k=0; k<gg->nsegments; k++) {
    start_a = start_b = -1;
    a = gg->segment_endpoints[k].a - 1;
    b = gg->segment_endpoints[k].b - 1;
    for (i=0; i<nr; i++) {
      if (rowv[i] == a) {
        start_a = i;
        break;
      }
    }
    if (start_a != -1) {
      for (i=start_a; i<nr; i++) {
        if (rowv[i] == b) {
          start_b = i;
          break;
        }
      }
    }
    if (start_a != -1 && start_b != -1) {  /* Both ends included */
      tlinks[nl].a = start_a + 1;
      tlinks[nl].b = start_b + 1;
      tcolors[nl] = gg->line.color_now.data[k];
      nl++;
    }
  }
  return (nl);
}

gboolean
save_lines (gchar *rootname, gboolean lines_p, gboolean colors_p,
  gint *rowv, gint nr, ggobid *gg)
{
  gchar *fname;
  gint k, nl;
  FILE *fp;
  endpointsd *tlinks;
  gshort *linecolors;

  if (lines_p || colors_p) {

    if (nr == gg->nrows) {
      nl = gg->nsegments;
      tlinks = gg->segment_endpoints;
      if (!gg->mono_p)
        linecolors = gg->line.color_now.data;

    } else {
      /*
       *
       * Determine the number of links to be saved -- may as
       * well build a temporary links structure to use, actually.
      */
      tlinks = (endpointsd *) g_malloc (gg->nsegments * sizeof (endpointsd));
      if (!gg->mono_p)
        linecolors = (gshort *) g_malloc (gg->nsegments * sizeof (gshort));
      nl = linedata_get (tlinks, linecolors, rowv, nr, gg);
    }
  }

  /*-- save the lines themselves --*/
  if (lines_p) {

    fname = g_strdup_printf ("%s.lines", rootname);
    fp = fopen (fname, "w");
    g_free (fname);

    if (fp == NULL) {
      gchar *message = g_strdup_printf (
        "The file '%s.lines' can not be opened for writing\n", rootname);
      quick_message (message, false);
      g_free (message);
      if (nr != gg->nrows) {
        g_free (tlinks);
        if (!gg->mono_p)
          g_free (linecolors);
      }
      return false;

    } else {

      for (k=0; k<nl; k++)
        fprintf (fp, "%d %d\n", tlinks[k].a, tlinks[k].b);

      if (nr != gg->nrows) {
        g_free ((gchar *) tlinks);
      }
      fclose (fp);
    }
  }


  /*-- save the line colors --*/
  if (colors_p && !gg->mono_p) {

    fname = g_strdup_printf ("%s.linecolors", rootname);
    fp = fopen (fname, "w");
    g_free (fname);

    if (fp == NULL) {
      gchar *message = g_strdup_printf (
        "The file '%s.linecolors' can not be opened for writing\n", rootname);
      quick_message (message, false);
      g_free (message);
      if (nr != gg->nrows) {
        g_free (tlinks);
        g_free (linecolors);
      }
      return false;

    } else {

      for (k=0; k<nl; k++)
        fprintf (fp, "%d\n", linecolors[k]);
  
      if (nr != gg->nrows)
        g_free ((gchar *) linecolors);
      fclose (fp);
    }
  }

  return true;
}

/*------------------------------------------------------------------------*/
/*                 End of lines section                                   */
/*------------------------------------------------------------------------*/

gboolean
save_missing (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  ggobid *gg)
{
  gint i;
  gchar *fname;
  gboolean success = true;
  FILE *fp;

  if (rowv == (gint *) NULL) {
    rowv = (gint *) g_malloc (nr * sizeof (gint));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = g_strdup_printf ("%s.missing", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "Problem writing out the missing file, %s\n", fname);
    quick_message (message, false);
    g_free (message);
    return false;
  }
  else
  {
    gint i, j, jc, m;
    for (i=0; i<nr; i++)
    {
      m = rowv[i];

      for (j=0; j<nc; j++) {
        if (colv == (gint *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */
          fprintf (fp, "%d ", gg->missing.data[m][jc]);
      }
      fprintf (fp, "\n");
    }
    fclose (fp);
  }

  return (success);
}

