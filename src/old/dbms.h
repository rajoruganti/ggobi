/* dbms.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef DBMS_H
#define DBMS_H

/**
 This and the dbms_gui.[ch] files define utilities for helping to connect
 to a relational database management system (DBMS). The purpose is to
 help get the inputs from the user that parameterize the connection
 to the database and the queries for the data they want to manipulate
 in GGobi.

 In a perfect world, these DBMS uilities might be in a separate library
 that would be linked by the plugins needing them.
 However, for the immediate future, we will simply compile them into
 GGobi so that they are available to the plugins without any additional
 effort.
 */

typedef struct _DBMSLoginInfo DBMSLoginInfo; 

 struct _DBMSLoginInfo {
  char *host;
  char *user;
  char *password;
  char *dbname;
  unsigned int   port;
  char *socket;
  unsigned int   flags;

  char *dataQuery;
  char *segmentsQuery;
  char *appearanceQuery;
  char *colorQuery;
  
  int (*dbms_read_input)(DBMSLoginInfo *, gboolean, ggobid *);
  InputDescription *desc;
};

/**
 Symbolic constants identifying the fields 
  Must correspond to the fieldNames in read_mysql.c 
*/
typedef enum {HOST, USER, PASSWORD, DATABASE, PORT, SOCKET, FLAGS, MISS, 
              DATA_QUERY, SEGMENTS_QUERY, APPEARANCE_QUERY, COLOR_QUERY, NUM_DBMS_FIELDS} 
           DBMSInfoElement;

extern const char * const DBMSFieldNames[NUM_DBMS_FIELDS];

extern DBMSLoginInfo DefaultDBMSInfo;

DBMSLoginInfo * initDBMSLoginInfo(DBMSLoginInfo *login, GHashTable *);
DBMSInfoElement getDBMSLoginElementIndex(const char *name);
char *getDBMSLoginElement(DBMSInfoElement i, int *isCopy, DBMSLoginInfo *info);
int setDBMSLoginElement(DBMSInfoElement i, char *val, DBMSLoginInfo *info);
#endif


