/******************************************************************************
* Triangle.h - header file for triangle processing in the RNDR library.       *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef IRNDR_TRIANGLE_H
#define IRNDR_TRIANGLE_H

#include "rndr_loc.h"
#include "color.h"
#include "scene.h"

/* Working structure describing polygon edge; some fields are used in scan   */
/* line converting and interpolation.                                        */
typedef struct IRndrEdgeStruct {
    int x;/* Current X coordinate on the scan line, the lowest end at start. */
    int dx, dy, Inc; /* Scan line converting integer algorithm data members. */
    int YMin;                             /* The lowest endpoint coordinate. */
    IRndrInterpolStruct Value;    /* Starting and later crnt interpo. value. */
    IRndrInterpolStruct dValue;         /* Increment of interpolation value. */
} IRndrEdgeStruct;

/* Triangle, which is a polygon abstraction. */
typedef struct IRndrTriangleStruct {
    IRndrEdgeStruct Edge[3];        /* Array of edges representing Triangle. */
    IRndrEdgeStruct *SortedEdge[3];
    int YMin, YMax;               /* Scan line range Triangle is located in. */
    IrtRType ZMin, ZMax;                       /* Z limits of this triangle. */
    IPPolygonStruct *Poly;         /* Pointer to the compliant Irit polygon. */
    IRndrObjectStruct *Object;      /* Object that Triangle is contained in. */
    IRndrIntensivityStruct **Vals;
    IRndrIntensivityStruct **dVals;
    IrtRType dz;
    int IsBackFaced;
    IRndrVisibleValidityType Validity;
} IRndrTriangleStruct;

void TriangleInit(IRndrTriangleStruct *Tri);

int TriangleSet(IRndrTriangleStruct *Tri,
                IPPolygonStruct *Poly,
                IRndrObjectStruct *o,
                IRndrSceneStruct *Scene);

void TriangleRelease(IRndrTriangleStruct *Tri);

#endif /* IRNDR_TRIANGLE_H */
