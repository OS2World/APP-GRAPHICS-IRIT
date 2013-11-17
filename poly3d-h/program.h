/*****************************************************************************
* Definitions for the Poly3D-H program:                                      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef POLY_3D_H_H
#define POLY_3D_H_H

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "grap_lib.h"
#include "attribut.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "symb_lib.h"

#ifdef  IRIT_EPS
#undef  IRIT_EPS
#endif  /* IRIT_EPS */
#define IRIT_EPS	1e-4

#define  EDGE_HASH_TABLE_SIZE	500 /* Number of entries in edge hash table. */
#define  EDGE_HASH_TABLE_SIZE1	499		     /* One below the above. */
#define  EDGE_HASH_TABLE_SIZE2	250			   /* Half of above. */
#define  POLY_HASH_TABLE_SIZE	500 /* Number of entries in poly hash table. */
#define  POLY_HASH_TABLE_SIZE1	499		     /* One below the above. */
#define  POLY_HASH_TABLE_SIZE2	250			   /* Half of above. */

#define  VISIBLE_COLOR	IG_IRIT_YELLOW /* Color to draw visible line result. */
#define  HIDDEN_COLOR	IG_IRIT_BLUE    /* Color to draw hidden line result. */
#define  HIDDEN_COLOR_RATIO	2         /* Color to dim visible RGB color. */

#define  VISIBLE_WIDTH		0.05
#define  HIDDEN_WIDTH_RATIO	10

#define  DEFAULT_FINENESS	4	  /* Default fineness of srf subdiv. */

typedef struct EdgeStruct {
    struct EdgeStruct *Pnext;
    IPVertexStruct *Vertex[2];			   /* The two edge vertices. */
    unsigned char Internal;	       /* If edge is Internal (IRIT output). */
} EdgeStruct;

/* The following are global setable variables (via config file poly3d-h.cfg) */
IRIT_GLOBAL_DATA_HEADER int
    GlblMore,
    GlblClipScreen,
    GlblQuiet,
    GlblOutputHasRGB,
    GlblOutputRGB[3],
    GlblOutputColor,
    GlblNumEdge,
    GlblBackFacing,
    GlblInternal,
    GlblOutputHiddenData,
    NumOfPolygons;		      /* Total number of polygons to handle. */

IRIT_GLOBAL_DATA_HEADER IrtRType
    GlblOutputWidth;

/* Global transformation matrix: */
IRIT_GLOBAL_DATA_HEADER IrtHmgnMatType
    GlblViewMat;			   /* Current view/persp. of object. */

/* Data structures used by the hidden line modules: */
IRIT_GLOBAL_DATA_HEADER int EdgeCount;
IRIT_GLOBAL_DATA_HEADER EdgeStruct *EdgeHashTable[];
IRIT_GLOBAL_DATA_HEADER IPPolygonStruct *PolyHashTable[];

/* And finally the prototypes of the Poly3D-H.c module: */
void Poly3dhExit(int ExitCode);

/* Prototypes of the PrepData.c module: */
void PrepareViewData(IPObjectStruct *Objects);

/* Prototypes of the Out-Edge.c module: */
void OutVisibleEdges(FILE *OutFile);

#endif /* POLY_3D_H_H */
