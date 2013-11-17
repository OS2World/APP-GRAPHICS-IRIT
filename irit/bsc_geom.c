/******************************************************************************
* Bsc_Geom.c - Basic geometry interface.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "program.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "bool_lib.h"
#include "user_lib.h"
#include "bsc_geom.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to compute the distance between two 3d points.         M
*                                                                            *
* PARAMETERS:                                                                M
*   P1, P2:   Two points to compute the distance between.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    Computed distance.                                            M
*									     M
* SEE ALSO:                                                                  M
*   GMDistPointPoint                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   DistPointPoint, point point distance                                     M
*****************************************************************************/
double DistPointPoint(IrtPtType P1, IrtPtType P2)
{
    return IRIT_PT_PT_DIST(P1, P2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to construct the plane from given 3 points. If two of  M
* the points are the same it returns FALSE, otherwise (successful) returns   M
* TRUE.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:  Three points to fit a plane through.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The Plane, or NULL if invalid.			     M
*									     M
* SEE ALSO:                                                                  M
*   GMPlaneFrom3Points                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   PlaneFrom3Points, plane                                                  M
*****************************************************************************/
IPObjectStruct *PlaneFrom3Points(IrtPtType Pt1, IrtPtType Pt2, IrtPtType Pt3)
{
    IrtPlnType Plane;

    if (GMPlaneFrom3Points(Plane, Pt1, Pt2, Pt3))
        return IPGenPLANEObject(&Plane[0], &Plane[1], &Plane[2], &Plane[3]);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to compute the closest point on a given 3d line to a   M
* given 3d point. the line is prescribed using a point on it (Pl) and vector M
* (Vl).									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the closest to on the line.                       M
*   Pl, Vl:        Position and direction that defines the line.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The closest point.				     M
*									     M
* SEE ALSO:                                                                  M
*   GMPointFromPointLine                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PointFromPointLine, point line distance                                  M
*****************************************************************************/
IPObjectStruct *PointFromPointLine(IrtPtType Point, IrtPtType Pl, IrtPtType Vl)
{
    IrtPtType ClosestPt;

    GMPointFromPointLine(Point, Pl, Vl, ClosestPt);

    return IPGenPTObject(&ClosestPt[0], &ClosestPt[1], &ClosestPt[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to compute the disstance between a 3d point and a 3d   M
* line.									     M
*   The line is prescribed using a point on it (Pl) and vector (Vl).         M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the distance to on the line.                      M
*   Pl, Vl:        Position and direction that defines the line.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:        The computed distance.                                    M
*									     M
* SEE ALSO:                                                                  M
*   GMDistPointLine                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   DistPointLine, point line distance                                       M
*****************************************************************************/
double DistPointLine(IrtPtType Point, IrtPtType Pl, IrtPtType Vl)
{
    return GMDistPointLine(Point, Pl, Vl);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to compute the distance between a Point and a Plane.   M
* The Plane is prescribed using its four coefficients : Ax + By + Cz + D = 0 M
* given as four elements vector.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the distance to on the plane.                     M
*   Plane:         To find the distance to on the point.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:        The computed distance.                                    M
*									     M
* SEE ALSO:                                                                  M
*   GMDistPointPlane                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   DistPointPlane, point plane distance                                     M
*****************************************************************************/
double DistPointPlane(IrtPtType Point, IrtPlnType Plane)
{
    return GMDistPointPlane(Point, Plane);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to find the intersection point of a line and a plane   M
* (if any). 								     M
*   The Plane is prescribed using four coefficients : Ax + By + Cz + D = 0   M
* given as four elements vector. The line is define via a point on it Pl and M
* a direction vector Vl. Returns TRUE only if such point exists.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl, Vl:        Position and direction that defines the line.             M
*   Plane:         To find the intersection with the line.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The intersection point, or NULL if none.              M
*									     M
* SEE ALSO:                                                                  M
*   GMPointFromLinePlane                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PointFromLinePlane, line plane intersection                              M
*****************************************************************************/
IPObjectStruct *PointFromLinePlane(IrtPtType Pl, IrtPtType Vl, IrtPlnType Plane)
{
    IrtRType t;
    IrtPtType InterPoint;

    if (GMPointFromLinePlane(Pl, Vl, Plane, InterPoint, &t))
        return IPGenPTObject(&InterPoint[0], &InterPoint[1], &InterPoint[2]);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to find the two points Pti on the lines (Pli, Vli) ,   M
*   i = 1, 2 with the minimal Euclidian distance between them. In other      M
* words, the distance between Pt1 and Pt2 is defined as distance between the M
* two lines.								     M
*   The two points are calculated using the fact that if V = (Vl1 cross Vl2) M
* then these two points are the intersection point between the following:    M
* Point 1 - a plane (defined by V and line1) and the line line2.             M
* Point 2 - a plane (defined by V and line2) and the line line1.             M
*   This function returns TRUE iff the two lines are not parallel!           M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Vl1:  Position and direction defining the first line.               M
*   Pl2, Vl2:  Position and direction defining the second line.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     List of two closest points, NULL if none.	     M
*									     M
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoPointsFromLineLine, line line distance                                M
*****************************************************************************/
IPObjectStruct *TwoPointsFromLineLine(IrtPtType Pl1,
				      IrtPtType Vl1,
				      IrtPtType Pl2,
				      IrtPtType Vl2)
{
    IrtPtType Pt1, Pt2;
    IrtRType t1, t2;

    if (GM2PointsFromLineLine(Pl1, Vl1, Pl2, Vl2, Pt1, &t1, Pt2, &t2)) {
	IPObjectStruct
	    *PObj = IPGenLISTObject(IPGenPTObject(&Pt1[0], &Pt1[1], &Pt1[2]));

	IPListObjectInsert(PObj, 1, IPGenPTObject(&Pt2[0], &Pt2[1], &Pt2[2]));
	IPListObjectInsert(PObj, 2, NULL);
	return PObj;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to find the two points Pti on the circles              M
* (Cntri, Nrmli, Radi), i = 1, 2, if any.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cntr1, Nrml1, Rad1:  Position, orientation, and size of first circle.    M
*   Cntr2, Nrml2, Rad2:  Position, orientation, and size of second circle.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     List of two closest points, NULL if none.	     M
*									     M
* SEE ALSO:                                                                  M
*   GM2PointsFromCircCirc                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoPointsFromCircCirc, circle circle distance                            M
*****************************************************************************/
IPObjectStruct *TwoPointsFromCircCirc(IrtPtType Cntr1,
				      IrtVecType Nrml1,
				      IrtRType *Rad1,
				      IrtPtType Cntr2,
				      IrtVecType Nrml2,
				      IrtRType *Rad2)
{
    int i;
    IrtPtType Pt1, Pt2;
    IPObjectStruct
	*PObj = IPGenLISTObject(NULL);

    if ((i = GM2PointsFromCircCirc3D(Cntr1, Nrml1, *Rad1, Cntr2, Nrml2, *Rad2,
				     Pt1, Pt2)) > 0) {
        IPListObjectInsert(PObj, 0, IPGenPTObject(&Pt1[0], &Pt1[1], &Pt1[2]));

	if (i == 2) {
	    IPListObjectInsert(PObj, 1, 
			       IPGenPTObject(&Pt2[0], &Pt2[1], &Pt2[2]));
	    IPListObjectInsert(PObj, 2, NULL);
	}
	else
	    IPListObjectInsert(PObj, 1, NULL);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the two tangents lines to given two circles, if any.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Cntr1, Rad1:  Position and radius of first circle.			     M
*   Cntr2, Rad2:  Position and radius of second circle.			     M
*   ROuterTans:   TRUE for outer two bitangents, FALSE for inner pair.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of two line segments, if exists.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2BiTansFromCircCirc                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoTangentsFromCircCirc                                                  M
*****************************************************************************/
IPObjectStruct *TwoTangentsFromCircCirc(IrtPtType Cntr1,
					IrtRType *Rad1,
					IrtPtType Cntr2,
					IrtRType *Rad2,
					IrtRType *ROuterTans)
{
    IrtPtType TanPts[2][2];

    if (GM2BiTansFromCircCirc(Cntr1, *Rad1, Cntr2, *Rad2,
			      IRIT_REAL_PTR_TO_INT(ROuterTans), TanPts)) {
        IPVertexStruct *V;
        IPObjectStruct
	    *PLn1 = IPGenPOLYLINEObject(
		IPAllocPolygon(0, IPAllocVertex2(IPAllocVertex2(NULL)), NULL)),
	    *PLn2 = IPGenPOLYLINEObject(
		IPAllocPolygon(0, IPAllocVertex2(IPAllocVertex2(NULL)), NULL)),
	    *PObj = IPGenLISTObject(PLn1);

	IPListObjectInsert(PObj, 1, PLn2);
	IPListObjectInsert(PObj, 2, NULL);

	V = PLn1 -> U.Pl -> PVertex;
	IRIT_PT_COPY(V -> Coord, TanPts[0][0]);
	IRIT_PT_COPY(V -> Pnext -> Coord, TanPts[0][1]);

	V = PLn2 -> U.Pl -> PVertex;
	IRIT_PT_COPY(V -> Coord, TanPts[1][0]);
	IRIT_PT_COPY(V -> Pnext -> Coord, TanPts[1][1]);

	return PObj;
    }
    else
        return IPGenLISTObject(NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface routine to find the distance between two lines (Pli, Vli) ,    M
* i = 1, 2.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Vl1:  Position and direction defining the first line.               M
*   Pl2, Vl2:  Position and direction defining the second line.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:   Distance between the two lines.		                     M
*									     M
* SEE ALSO:                                                                  M
*   GMDistLineLine                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   DistLineLine, line line distance                                         M
*****************************************************************************/
double DistLineLine(IrtPtType Pl1, IrtPtType Vl1, IrtPtType Pl2, IrtPtType Vl2)
{
    return GMDistLineLine(Pl1, Vl1, Pl2, Vl2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interface to compute the Barycentric coordinates of given point Pt with  M
* respect to given Trainagle Pt1 Pt2 Pt3. All points are assumed to be	     M
* coplanar.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:   Three points forming a triangular in general position.  M
*   Pt:		     A point for which the barycentric coordinates are to be M
*		     computed.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A vector holding the three Barycentric		     M
*	coefficients, or NULL if point Pt is outside the triangle	     M
*	Pt1 Pt2 Pt3.							     M
*									     M
* SEE ALSO:                                                                  M
*   GMBaryCentric3Pts                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BaryCentric3Pts				                             M
*****************************************************************************/
IPObjectStruct *BaryCentric3Pts(IrtPtType Pt1,
				IrtPtType Pt2,
				IrtPtType Pt3,
				IrtPtType Pt)
{
    IrtRType *V;

    if ((V = GMBaryCentric3Pts(Pt1, Pt2, Pt3, Pt)) != NULL)
        return IPGenPTObject(&V[0], &V[1], &V[2]);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection, if any, of two polygons.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2:   The two polygons to intersect.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Intersection edge as a polyline object, NULL if none. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolInterPolyPoly                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyPolyIntersection                                                     M
*****************************************************************************/
IPObjectStruct *PolyPolyIntersection(IPObjectStruct *Pl1, IPObjectStruct *Pl2)
{
    IPPolygonStruct
	*InterPl = BoolInterPolyPoly(Pl1 -> U.Pl, Pl2 -> U.Pl);

    if (InterPl)
	return IPGenPOLYLINEObject(InterPl);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
   Computes the maximal Z motion to move PObj2 down (-Z direction) so that   M
* it does not intersect PObj1.  Second object is converted to its Bbox.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:     First static object to place PObj2 on.                        M
*   PObj2:     Second dynamic object that is to be moved down (-Z direction) M
*	       until it is tangent to PObj1.				     M
*   FineNess:  Of polygonal approximation of PObj1.                          M
*   NumIters:  In the bisectioning of collision's test. 10 is a good start.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Maximal Z motion possible.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserTwoObjMaxZRelMotion                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeZCollisions                                                       M
*****************************************************************************/
IrtRType ComputeZCollisions(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *FineNess,
			    IrtRType *NumIters)
{
    return UserTwoObjMaxZRelMotion(PObj1, PObj2, *FineNess,
				   IRIT_REAL_PTR_TO_INT(NumIters));
}
