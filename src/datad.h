/*-- datad.h --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef DATAD_H
#define DATAD_H

#include "defines.h"
#include "brushing.h"
#include "vartable.h"

#include <libxml/parser.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _XMLUserData;

struct _ggobid;

#include "fileio.h"

typedef struct _datad datad;

#define GGOBI_TYPE_DATA	 (ggobi_data_get_type ())
#define GGOBI_DATA(obj)	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_DATA, datad))
#define GGOBI_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_DATA, GGobiDataClass))
#define GGOBI_IS_DATA(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_DATA))
#define GGOBI_IS_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_DATA))
#define GGOBI_DATA_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_DATA, GGobiDataClass))

GType ggobi_data_get_type(void);

datad *ggobi_data_new(guint, guint);

/**
  Identifiers for the signal types generated by the datad class
 */
enum { ROWS_IN_PLOT_CHANGED_SIGNAL,
       MAX_GGOBI_DATA_SIGNALS
};

typedef struct _GGobiDataClass {
  GObjectClass parent_class;
  guint signals[MAX_GGOBI_DATA_SIGNALS];

} GGobiDataClass;


struct _datad {

  GObject object;

  /* All the variables are left public since this the way they were in the
     C structure. Adding accessor routines and using those would be "good",
     but tedious.
   */

  /* Holds the name given to the dataset in an XML file and by which it
     can be indexed in the list of data elements within the ggobid structure.
   */
  const gchar *name;
  const gchar *nickname;
  InputDescription *input;

  struct _ggobid *gg; /*-- a pointer to the parent --*/

  guint nrows;
  GArray *rowlab; /*-- allocates memory in chunks --*/

 /*-- row ids to support generalized linking --*/
  GHashTable *idTable;
  char **rowIds;

 /*-- --*/

 /*-- to support brushing by categorical variable --*/
  vartabled *linkvar_vt;  /*-- the linking variable --*/
 /*-- --*/

  gint ncols;
  GSList *vartable;
  GtkWidget *vartable_tree_view[all_vartypes];
  GtkTreeModel *vartable_tree_model; /* the root model, with all vars */

  array_f raw, tform;
  array_g world, jitdata;

 /*----------------------- missing values ---------------------------*/

  gint nmissing;
  array_s missing; /*-- array of shorts --*/
  gboolean missings_show_p; /*-- show/hide per datad, not per display --*/

 /*---------------- deleting the hidden points; subsetting ----------*/

  vector_i rows_in_plot;  /*-- always of length d->nrows --*/
  gint nrows_in_plot;     /*-- how many elements of rows_in_plot to use --*/
  vector_b sampled, excluded;

  struct _Subset {
    gint random_n;
    gint string_pos;
   /*-- adjustments from which to get values for blocksize, everyn --*/
    GtkAdjustment *bstart_adj, *bsize_adj;
    GtkAdjustment *estart_adj, *estep_adj;
  } subset;

 /*--------------- clusters: hiding, excluding ----------------------*/

  symbol_cell symbol_table[NGLYPHTYPES][NGLYPHSIZES][MAXNCOLORS];

  GtkWidget *cluster_table; /*-- table of symbol groups from brushing --*/

  gint nclusters;
  clusterd *clusv;
  clusteruid *clusvui;
  vector_i clusterid;  /* cluster membership for each record */

 /*------------------------ jittering --------------------------------*/

  struct _Jitterd {
    gfloat factor;
    gboolean type;
    gboolean convex;
    gfloat *jitfacv;
  } jitter;

/*------------------------ brushing ----------------------------------*/

 /*-- it's odd to have these in datad; let me think about that --*/
  gint npts_under_brush;
  vector_b pts_under_brush;
  struct _BrushBins {
    gint nbins;
    bin_struct **binarray;
    icoords bin0, bin1;
  } brush;
 /*-- --*/

  vector_s color, color_now, color_prev;
  vector_b hidden, hidden_now, hidden_prev;
  vector_g glyph, glyph_now, glyph_prev;


/*---------------------- identification ------------------------------*/

 /*-- used in identification, line editing, and point motion --*/
  gint nearest_point, nearest_point_prev;
  GSList *sticky_ids;

/*-------------------- moving points ---------------------------------*/

  GSList *movepts_history; /*-- a list of elements of type celld --*/

/*----------------- variable selection panel -------------------------*/

  struct _Varpanel_cboxd {
    GtkWidget *ebox;   /*-- child1 of pane widget --*/
    GtkWidget *swin;   /*-- child of ebox --*/
    GtkWidget *vbox;   /*-- child of swin --*/
    GSList *box;       /*-- single column of hboxes --*/
  } vcbox_ui;

  struct _Varpanel_circd {
    GtkWidget *ebox;        /*-- child2 of pane widget --*/
    GtkWidget *vbox;        /*-- child of ebox --*/
    GtkWidget *swin, *hbox; /*-- children of vbox --*/
    GtkWidget *table;       /*-- sole child of swin; now a vbox --*/
    GtkWidget *manip_btn, *freeze_btn; /*-- children of hbox --*/

    GdkCursor *cursor;
    gint jcursor;

   /*-- components and properties of the table --*/
    GSList *vb, *da, *label;
    GSList *da_pix;         /*-- backing pixmaps --*/
    gint nvars;
  } vcirc_ui;

  struct _Varpaneld {
    GtkWidget *hpane;  /*-- child of the ebox --*/
  } varpanel_ui;

/*-------------------- transformation --------------------------------*/

  /* sphering transformation */
  struct _Sphere_d {
    vector_i vars;        /*-- vars available to be sphered --*/
    vector_i vars_sphered;/*-- vars that have been sphered --*/
    gint npcs;      /*-- the first npcs vars of vars will be sphered --*/
    vector_i pcvars;/*-- vars into which sphered data is written --*/

    vector_f eigenval;
    array_d eigenvec;
    array_f vc;
    vector_f tform_mean;
    vector_f tform_stddev;

    gboolean vars_stdized;
  } sphere;

/*----------------- segments in scatterplots -----------------------------*/

 /*-- edges --*/
  struct _EdgeData {
    gint n;
    SymbolicEndpoints *sym_endpoints;
    GList *endpointList;   
      /* a list of endpointsd elements corresponding to the resolved 
         record ids relative to a given datad. This is akin to a table
         indexed by datad elements. */

    gint nxed_by_brush;
    vector_b xed_by_brush;
  } edge;

/*------------------------------------------------------------------------*/

  /* Instead of a method, use a function pointer which can be set
     for the different types.
   */
   gboolean(*readXMLRecord) (const xmlChar ** attrs,
                             struct _XMLUserData * data);
};


#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
void datad_instance_init(datad * d);


void freeLevelHashEntry(gpointer key, gpointer value, gpointer data);

/*-- used as an attribute of variable notebooks --*/
typedef enum {no_edgesets, edgesets_only, all_datatypes} datatyped;

extern endpointsd *resolveEdgePoints(datad *e, datad *d);
void unresolveAllEdgePoints(datad *e);

void datad_record_ids_set(datad *d, gchar **ids, gboolean duplicate);
void ggobi_data_set_integer_column(datad *d, gint j, gint *values);
void ggobi_data_set_double_column(datad *d, gint j, gdouble *values);
void ggobi_data_set_row_labels(datad *d, gchar **labels);
void ggobi_data_set_name(datad *d, const gchar *name);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif
