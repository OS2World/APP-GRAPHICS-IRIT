/******************************************************************************
* GeomVals.c - Area, Volume on polygonal objects.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "allocate.h"
#include "geom_loc.h"

static IrtRType PolygonXYArea(IPVertexStruct *VHead);
static IrtRType Polygon3VrtxXYArea(IrtPtType Pt1,
				   IrtPtType Pt2,
				   IrtPtType Pt3);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the lengths of a poly, first vertex to last vertex.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl: The poly to compute its length.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   The length of the poly(line).                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyLength                                                             M
*****************************************************************************/
IrtRType GMPolyLength(const IPPolygonStruct *Pl)
{
    IPVertexStruct *V, *VLast;
    IrtRType
	Len = 0.0;

    if (Pl == NULL || (VLast = Pl -> PVertex) == NULL)
        return 0.0;

    for (V = VLast -> Pnext; V != NULL; VLast = V, V = V -> Pnext) {
	Len += IRIT_PT_PT_DIST(V -> Coord, VLast -> Coord);
    }

    return Len;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the centroid of a poly, as an average of all input vertices.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        The poly to compute its centroid.                             M
*   Centroid:  Computed center point.  Note it can be outside Pl!            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if at least one vertex in input Pl, FALSE otherwise.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMComputeAverageVertex						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyCentroid                                                           M
*****************************************************************************/
int GMPolyCentroid(const IPPolygonStruct *Pl, IrtPtType Centroid)
{
    int n = 0;
    IPVertexStruct *V;

    IRIT_PT_RESET(Centroid);

    if (Pl == NULL || (V = Pl -> PVertex) == NULL)
        return FALSE;

    do {
        IRIT_PT_ADD(Centroid, Centroid, V -> Coord);
	n++;

	V = V -> Pnext;
    }
    while (V != Pl -> PVertex && V != NULL);

    IRIT_PT_SCALE(Centroid, 1.0 / n);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to evaluate the Area of the given geom. object, in object unit.   M
* Algorithm (for each polygon):					V3	     V
* 1. Set Polygon Area to be zero.			       /\	     V
*    Make a copy of the original polygon		     /	  \ 	     V
*    and transform it to a XY parallel plane.		   /	    \V2	     V
*    Find the minimum Y value of the polygon	       V4/	     |	     V
*    in the XY plane.					 \	     |	     V
* 2. Let V(0) be the first vertex, V(n) the last one.      \         |	     V
*    For i goes from 0 to n-1 add to Area the area   	     \_______|       V
*    below edge V(i), V(i+1):				     V0      V1      V
*    PolygonArea += (V(i+1)x - V(i)x) * (V(i+1)y' - V(i)y') / 2		     V
*    where V(i)y' is V(i)y - MinY, where MinY is polygon minimum Y value.    V
* 3. The result of step 2 is the area of the polygon itself. 		     V
*    However, it might be negative, so take the absolute result of step 2    V
*    and add it to the global ObjectArea.				     V
* Note step 2 is performed by another auxiliary routine: PolygonXYArea.      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A polyhedra object to compute its surface area.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:   The area of object PObj.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyObjectArea, area                                                   M
*****************************************************************************/
double GMPolyObjectArea(const IPObjectStruct *PObj)
{
    IrtRType
	ObjectArea = 0.0;
    IPPolygonStruct *Pl;

    if (!IP_IS_POLY_OBJ(PObj))
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	return 0.0;
    }

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	ObjectArea += GMPolyOnePolyArea(Pl);

    return ObjectArea;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the area of a single polygon.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:    To compute its area.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    area of polygon Pl                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyObjectArea                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyOnePolyArea                                                        M
*****************************************************************************/
double GMPolyOnePolyArea(const IPPolygonStruct *Pl)
{
    IrtHmgnMatType RotMat;
    IPVertexStruct
	*VHead = IPCopyVertexList(Pl -> PVertex),/* Dont trans orig. object. */
	*V = VHead;
    IrtRType ObjectArea;

    if (IRIT_PT_EQ_ZERO(Pl -> Plane))
	return 0.0;

    /* Create the trans matrix to transform the polygon to XY parallel plane */
    GMGenRotateMatrix(RotMat, Pl -> Plane);
    do {
	MatMultPtby4by4(V -> Coord, V -> Coord, RotMat);

	V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    ObjectArea = PolygonXYArea(VHead);

    IPFreeVertexList(VHead);			  /* Free the vertices list. */

    return ObjectArea;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to evaluate the area of the given polygon projected on the XY    *
* plane.	                                                             *
*   The polygon does not have to be on a XY parallel plane, as only its XY   *
* projection is considered (Z is ignored).				     *
*   Returned the area of its XY parallel projection.			     *
*   See GeomObjectArea above for algorithm.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   VHead:     Of vertex list to compute area for.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  Computed area.                                                *
*****************************************************************************/
static IrtRType PolygonXYArea(IPVertexStruct *VHead)
{
    IrtRType MinY,
	PolygonArea = 0.0;
    IPVertexStruct *Vnext,
	*V = VHead;

    MinY = V -> Coord[1];
    V = V -> Pnext;
    while (V != VHead && V != NULL) {
	if (MinY > V -> Coord[1])
	    MinY = V -> Coord[1];

	V = V -> Pnext;
    }

    if (V == NULL)
	V = VHead;
    Vnext = V -> Pnext;
    MinY *= 2.0;		  /* Instead of subtracting twice each time. */
    do {
	/* Evaluate area below edge V->Vnext relative to Y level MinY. Note  */
	/* it can come out negative, but thats o.k. as the sum of all these  */
	/* quadraliterals should be exactly the area (up to correct sign).   */
	PolygonArea += (Vnext -> Coord[0] - V -> Coord[0]) *
			(Vnext -> Coord[1] + V -> Coord[1] - MinY);
	V = Vnext;
	Vnext = V -> Pnext == NULL ? VHead : V -> Pnext;
    }
    while (V != VHead && V != NULL);

    return IRIT_FABS(PolygonArea) * 0.5;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to evaluate the Volume of the given geom object, in object unit. M
*   This routine has a side effect that all non-convex polygons will be      M
* splitted to convex ones.						     M
* Algorithm (for each polygon, and let ObjMinY be the minimum OBJECT Y):     M
*								V3	     V
* 1. Set Polygon Area to be zero.			       /\	     V
*    Let V(0) be the first vertex, V(n) the last.	     /	  \ 	     V
*    For i goes from 1 to n-1 form triangles		   /	    \V2	     V
*                   by V(0), V(i), V(i+1).             V4/	     |	     V
*    For each such triangle di:				 \	     |	     V
*    1.1. Find the vertex (out of V(0), V(i), V(i+1))      \         |	     V
*         with the minimum Z - TriMinY.			     \_______|       V
*    1.2. The volume below V(0), V(i), V(i+1) triangle,	     V0      V1      V
*	  relative to ObjMinZ level, is the sum of:			     V
*	  1.2.1. volume of V'(0), V'(i), V'(i+1) - the			     V
*		 area of projection of V(0), V(i), V(i+1) on XY parallel     V
*		 plane, times (TriMinZ - ObjMinZ).			     V
*	  1.2.2. Assume V(0) is the one with the PolyMinZ. Let V"(i) and     V
*		 V"(i+1) be the projections of V(i) and V(i+1) on the plane  V
*		 Z = PolyZMin. The volume above 1.2.1. and below the polygon V
*		 (triangle!) will be: the area of quadraliteral V(i), V(i+1),V
*		 V"(i+1), V"(i), times distance of V(0) for quadraliteral    V
*		 plane divided by 3.					     V
*    1.3. If Z component of polygon normal is negative add 1.2. result to    V
*	  ObjectVolume, else subtract it.				     V
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To compute volume for.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    Computed volume.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyObjectVolume, volume                                               M
*****************************************************************************/
double GMPolyObjectVolume(IPObjectStruct *PObj)
{
    int PlaneExists;
    IrtRType ObjMinZ, TriMinZ, Area, PolygonVolume, Dist,
	ObjVolume = 0.0;
    IrtPtType Pt1;
    IrtPlnType Plane;
    IPPolygonStruct *Pl;
    IPVertexStruct *V, *VHead, *Vnext, *Vtemp;

    if (!IP_IS_POLY_OBJ(PObj))
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	return 0.0;
    }

    ObjMinZ = IRIT_INFNTY;	 /* Find Object minimum Z value (used as min level). */
    Pl = PObj -> U.Pl;
    while (Pl != NULL) {
	V = VHead = Pl -> PVertex;
	do {
	    if (V -> Coord[2] < ObjMinZ)
		ObjMinZ = V -> Coord[2];
	    V = V -> Pnext;
	}
	while (V != VHead && V != NULL);
	if (V == NULL)
	    GEOM_FATAL_ERROR(GEOM_ERR_OPEN_OBJ_VOL_COMP);

	Pl = Pl -> Pnext;
    }

    GMConvexPolyObject(PObj);	       /* Make sure all polygons are convex. */
    Pl = PObj -> U.Pl;
    while (Pl != NULL) {
	PolygonVolume = 0.0; /* Volume below poly relative to ObjMinZ level. */
	/* We set VHead to be vertex with min Z: */
	V = Vtemp = VHead = Pl -> PVertex;
	do {
	    if (V -> Coord[2] < Vtemp -> Coord[2])
		Vtemp = V;
	    V = V -> Pnext;
	}
	while (V != VHead && V != NULL);
	VHead = Vtemp;	   /* Now VHead is the one with lowest Z in polygon! */
	TriMinZ = VHead -> Coord[2];	     /* Save this Z for fast access. */

	V = VHead -> Pnext;
	Vnext = V -> Pnext;
	do {
	    /* VHead, V, Vnext form the triangle - find volume 1.2.1. above: */
	    Area = Polygon3VrtxXYArea(VHead -> Coord, V -> Coord,
							       Vnext -> Coord);
	    PolygonVolume += Area * (TriMinZ - ObjMinZ);

	    /* VHead, V, Vnext form the triangle - find volume 1.2.2. above: */
	    Area = sqrt(IRIT_SQR(V -> Coord[0] - Vnext -> Coord[0]) +
			IRIT_SQR(V -> Coord[1] - Vnext -> Coord[1])) *
		   ((V -> Coord[2] + Vnext -> Coord[2]) * 0.5 - TriMinZ);
	    IRIT_PT_COPY(Pt1, V -> Coord);
	    Pt1[2] = TriMinZ;
	    if ((PlaneExists =
		 GMPlaneFrom3Points(Plane, V -> Coord,
				    Vnext -> Coord, Pt1)) == 0) {
		/* Try second pt projected to Z = TriMinZ plane as third pt. */
		IRIT_PT_COPY(Pt1, Vnext -> Coord);
		Pt1[2] = TriMinZ;
		PlaneExists =
			GMPlaneFrom3Points(Plane, V -> Coord, Vnext -> Coord,
									Pt1);
	    }
	    if (PlaneExists) {
		Dist = GMDistPointPlane(VHead -> Coord, Plane);
		PolygonVolume += Area * IRIT_FABS(Dist) / 3.0;
	    }

	    V = Vnext;
	    Vnext = V -> Pnext;
	}
	while (Vnext != VHead);

	if (Pl -> Plane[2] < 0.0)
	    ObjVolume += PolygonVolume;
	else
	    ObjVolume -= PolygonVolume;

	Pl = Pl -> Pnext;
    }

    return ObjVolume;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Routine to evaluate the area of the given triangle projected to the XY    *
* plane, given as 3 Points. Only the X & Y components are considered.	     *
*  See PolyObjectArea above for algorithm.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2, Pt3: Vertices of triangle to compute area for.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:      Area of triangle in the XY plane.                         *
*****************************************************************************/
static IrtRType Polygon3VrtxXYArea(IrtPtType Pt1,
				   IrtPtType Pt2,
				   IrtPtType Pt3)
{
    IrtRType MinY,
	PolygonArea = 0.0;

    MinY = IRIT_MIN(Pt1[1], IRIT_MIN(Pt2[1], Pt3[1])) * 2.0;

    PolygonArea += (Pt2[0] - Pt1[0]) * (Pt2[1] + Pt1[1] - MinY);
    PolygonArea += (Pt3[0] - Pt2[0]) * (Pt3[1] + Pt2[1] - MinY);
    PolygonArea += (Pt1[0] - Pt3[0]) * (Pt1[1] + Pt3[1] - MinY);

    return IRIT_FABS(PolygonArea) * 0.5;
}
