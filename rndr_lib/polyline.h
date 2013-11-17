/******************************************************************************
* Polyline.h - header file for the polyline processing in the RNDR library.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_POLYLINE_H_
#define _IRNDR_POLYLINE_H_

#include "rndr_loc.h"

/* In this module we work with homogenous coordinates in screen space. */

typedef struct IRndrPolylineOptionsStruct {
    IrtRType ZNear;
    IrtRType ZFar;
    IrtRType MinWidth;
    IrtRType MaxWidth;

} IRndrPolylineOptionsStruct;

typedef struct IRndrLineSegmentStruct{
    IPPolygonStruct *Tri;
    IrtPtType4 Vertex[5]; /* 4 for the rectangle + one extra for sharp bends.*/
    IrtVecType Normal[5]; /* 4 for the rectangle + one extra for sharp bends.*/
    IrtPtType4 LastPoint;
    IrtPtType4 LastDelta;
    IPVertexStruct *TriVertex[3];
    IRndrPolylineOptionsStruct PolyOptions;
    IrtRType k;
    int NumVertex;
    int TrianglesNum;           /* No. of triangles constructing the segment */
    int SharpBend;                    /* TRUE for more than 90 degrees turn. */
} IRndrLineSegmentStruct;

void LineSegmentInit(IRndrLineSegmentStruct *Seg,
                     IRndrPolylineOptionsStruct *PolyOptions);

void LineSegmentSetOptions(IRndrLineSegmentStruct *Seg,
                           IRndrPolylineOptionsStruct *PolyOptions);

void LineSegmentStart(IRndrLineSegmentStruct *Seg);

void LineSegmentSet(IRndrLineSegmentStruct *Seg,
                    IrtPtType4 Vertex);

IPPolygonStruct *LineSegmentGetTri(IRndrLineSegmentStruct *Seg,
                                   int NumTri);

void LineSegmentEnd(IRndrLineSegmentStruct *Seg);

void LineSegmentRelease(IRndrLineSegmentStruct *Seg);

#endif /* _IRNDR_POLYLINE_H_ */
