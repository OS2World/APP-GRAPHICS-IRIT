/******************************************************************************
* UserPick.c - handle of generic function of picking/distance events.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 97.  					      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "user_loc.h"
#include "geom_lib.h"
#include "cagd_lib.h"

#define INSIDE_DOMAIN(t, TMin, TMax)  ((t) >= (TMin) && (t) <= (TMax))

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the closest distance between a 3-space line and a bounding      M
* box.                                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   LinePos:    Point on the 3-space line.                                   M
*   LineDir:    Direction of the 3-space line.                               M
*   BBox:       Bounding box, parallel to main planes.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Minimal distance between the line and the bbox.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMinDistLinePolylineList, UserMinDistLinePolygonList 	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMinDistLineBBox                                                      M
*****************************************************************************/
IrtRType UserMinDistLineBBox(const IrtPtType LinePos,
			     const IrtVecType LineDir,
			     IrtBboxType BBox)
{
    int i, l, InX, InY, InZ;
    IrtRType D[3][2],
	MinDist = IRIT_INFNTY;

    if (BBox[0][2] > BBox[1][2]) {
	/* It is a bbox of a planar object - zap the Z values. */
	BBox[0][2] = BBox[1][2] = 0.0;
    }

    D[0][0] = BBox[0][0] - LinePos[0];
    D[0][1] = BBox[1][0] - LinePos[0];
    D[1][0] = BBox[0][1] - LinePos[1];
    D[1][1] = BBox[1][1] - LinePos[1];
    D[2][0] = BBox[0][2] - LinePos[2];
    D[2][1] = BBox[1][2] - LinePos[2];

    for (l = 0; l < 3; l++) {
	for (i = 0; i < 2; i++) {
	    IrtRType t,
		Dist = IRIT_INFNTY;
	    IrtPtType Pt;

	    if (IRIT_APX_EQ(LineDir[l], 0.0))
	        continue;

	    t = D[l][i] / LineDir[l];
	    IRIT_PT_COPY(Pt, LineDir);
	    IRIT_PT_SCALE(Pt, t);
	    IRIT_PT_ADD(Pt, Pt, LinePos);

	    switch (l) {
		case 0:
		    InY = INSIDE_DOMAIN(Pt[1], BBox[0][1], BBox[1][1]);
		    InZ = INSIDE_DOMAIN(Pt[2], BBox[0][2], BBox[1][2]);

		    if (InY && InZ)
		        return 0.0;         /* The line punctures this face. */
		    if (InY) {
			if (Pt[2] > BBox[1][2])
			    Dist = Pt[2] - BBox[1][2];
			else
			    Dist = BBox[0][2] - Pt[2];
		    }
		    else if (InZ) {
			if (Pt[1] > BBox[1][1])
			    Dist = Pt[1] - BBox[1][1];
			else
			    Dist = BBox[0][1] - Pt[1];
		    }
		    else {
			if (Pt[1] > BBox[1][1])
			    Dist = IRIT_SQR(Pt[1] - BBox[1][1]);
			else
			    Dist = IRIT_SQR(Pt[1] - BBox[0][1]);

			if (Pt[2] > BBox[1][2])
			    Dist += IRIT_SQR(Pt[2] - BBox[1][2]);
			else
			    Dist += IRIT_SQR(Pt[1] - BBox[0][1]);

			Dist = sqrt(Dist);
		    }
		    break;
		case 1:
		    InX = INSIDE_DOMAIN(Pt[0], BBox[0][0], BBox[1][0]);
		    InZ = INSIDE_DOMAIN(Pt[2], BBox[0][2], BBox[1][2]);

		    if (InX && InZ)
		        return 0.0;         /* The line punctures this face. */
		    if (InX) {
			if (Pt[2] > BBox[1][2])
			    Dist = Pt[2] - BBox[1][2];
			else
			    Dist = BBox[0][2] - Pt[2];
		    }
		    else if (InZ) {
			if (Pt[0] > BBox[1][0])
			    Dist = Pt[0] - BBox[1][0];
			else
			    Dist = BBox[0][0] - Pt[0];
		    }
		    else {
			if (Pt[0] > BBox[1][0])
			    Dist = IRIT_SQR(Pt[0] - BBox[1][0]);
			else
			    Dist = IRIT_SQR(Pt[0] - BBox[0][0]);

			if (Pt[2] > BBox[1][2])
			    Dist += IRIT_SQR(Pt[2] - BBox[1][2]);
			else
			    Dist += IRIT_SQR(Pt[2] - BBox[0][2]);

			Dist = sqrt(Dist);
		    }
		    break;
		case 2:
		    InX = INSIDE_DOMAIN(Pt[0], BBox[0][0], BBox[1][0]);
		    InY = INSIDE_DOMAIN(Pt[1], BBox[0][1], BBox[1][1]);

		    if (InX && InY)
		        return 0.0;         /* The line punctures this face. */
		    if (InX) {
			if (Pt[1] > BBox[1][1])
			    Dist = Pt[1] - BBox[1][1];
			else
			    Dist = BBox[0][1] - Pt[1];
		    }
		    else if (InY) {
			if (Pt[0] > BBox[1][0])
			    Dist = Pt[0] - BBox[1][0];
			else
			    Dist = BBox[0][0] - Pt[0];
		    }
		    else {
			if (Pt[0] > BBox[1][0])
			    Dist = IRIT_SQR(Pt[0] - BBox[1][0]);
			else
			    Dist = IRIT_SQR(Pt[0] - BBox[0][0]);

			if (Pt[1] > BBox[1][1])
			    Dist += IRIT_SQR(Pt[1] - BBox[1][1]);
			else
			    Dist += IRIT_SQR(Pt[1] - BBox[0][1]);

			Dist = sqrt(Dist);
		    }
		    break;

	    }

	    if (MinDist > Dist)
	        MinDist = Dist;
	}
    }

    return MinDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the closest distance between a line in three space and a list    M
* of polygons.  The polygons are assumed to be convex.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   LinePos:    Point on the 3-space line.                                   M
*   LineDir:    Direction of the 3-space line.                               M
*   Pls:        List of polygons.                                            M
*   MinPl:      Will be set to the closest polyline.                         M
*   MinPt:      Will be set to the closest point.                            M
*   HitDepth:   If a direct hit (zero distance is returned), the depth of    M
*		the hit is returned here.		                     M
*   IndexFrac:  Will be set to the index of the closest point, starting from M
*		zero.  Index of 3.5 means the closest is in the mid point    M
*		from point 3 to point 4.  This value should be ignored for   M
*		a direct hit (return value equal zero).			     M
*		Valid for one polygon in input.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Minimal distance from the 3-space line to the polygons.      M
*		In case of a direct hit, zero is returned.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMinDistLineBBox, UserMinDistLinePolylineList  		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMinDistLinePolygonList                                               M
*****************************************************************************/
IrtRType UserMinDistLinePolygonList(const IrtPtType LinePos,
				    const IrtVecType LineDir,
				    IPPolygonStruct *Pls,
				    IPPolygonStruct **MinPl,
				    IrtPtType MinPt,
				    IrtRType *HitDepth,
				    IrtRType *IndexFrac)
{
    int DirectHit = FALSE;
    IPPolygonStruct *Pl;

    *HitDepth = -IRIT_INFNTY;
    *IndexFrac = -IRIT_INFNTY;
    *MinPl = NULL;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	IrtRType t;
	IrtPtType InterPt;

	if (GMPointFromLinePlane(LinePos, LineDir, Pl -> Plane, InterPt, &t) &&
	    GMPointInsideCnvxPolygon(InterPt, Pl)) {
	    /* We are inside this polygon! */
	    if (*HitDepth < t) {
	        /* This is the closest so far. */
	        *HitDepth = t;
		*MinPl = Pl;
		DirectHit = TRUE;
		IRIT_PT_COPY(MinPt, InterPt);
	    }
	}
    }

    if (DirectHit) {
	return 0.0;
    }
    else {
	return UserMinDistLinePolylineList(LinePos, LineDir, Pls, TRUE,
					   MinPl, MinPt, IndexFrac);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the closest distance between a line in three space and a list    M
* of polyline. 								     M
*                                                                            *
* PARAMETERS:                                                                M
*   LinePos:    Point on the 3-space line.                                   M
*   LineDir:    Direction of the 3-space line.                               M
*   Pls:        List of polylines.                                           M
*   PolyClosed: TRUE if polyline is a closed polygon, FALSE otherwise.       M
*   MinPl:      Will be set to the closest polyline.                         M
*   MinPt:      Will be set to the closest point.                            M
*   IndexFrac:  Will be set to the index of the closest point, starting from M
*		zero.  Index of 3.5 means the closest is in the mid point    M
*		from point 3 to point 4.  Valid for one polyline in input.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Minimal distance from the 3-space line to the polylines.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMinDistLineBBox, UserMinDistLinePolygonList,			     M
*   UserMinDistPointPolylineList  		   		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMinDistLinePolylineList                                              M
*****************************************************************************/
IrtRType UserMinDistLinePolylineList(const IrtPtType LinePos,
				     const IrtVecType LineDir,
				     IPPolygonStruct *Pls,
				     int PolyClosed,
				     IPPolygonStruct **MinPl,
				     IrtPtType MinPt,
				     IrtRType *IndexFrac)
{
    int Index;
    IrtRType
	Dist = IRIT_INFNTY;
    IPPolygonStruct *Pl;

    *IndexFrac = -IRIT_INFNTY;
    *MinPl = NULL;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct *VPrev,
	    *VLast = NULL,
	    *V = Pl -> PVertex;

	Index = 0;

	if (V -> Pnext == NULL)
	    continue;

	do {
	    IrtRType d, t1, t2;
	    IrtPtType Pt1, Pt2;
	    IrtVecType Dir;

	    if (V -> Pnext == NULL) {
		if (PolyClosed) {
		    /* Termporarily close the polyline into a polygon. */
		    VLast = V;
		    V -> Pnext = Pl -> PVertex;
		}
		else
		    break;
	    }
	    VPrev = V;
	    V = V -> Pnext;

	    IRIT_PT_SUB(Dir, V -> Coord, VPrev -> Coord);

	    if (IRIT_VEC_SQR_LENGTH(Dir) > IRIT_UEPS) {
	        if (GM2PointsFromLineLine(LinePos, LineDir,
					  VPrev -> Coord, Dir,
					  Pt1, &t1, Pt2, &t2)) {
		    if (t2 < 0.0)
		        d = GMDistPointLine(VPrev -> Coord, LinePos, LineDir);
		    else if (t2 > 1.0)
		        d = GMDistPointLine(V -> Coord, LinePos, LineDir);
		    else
		        d = IRIT_PT_PT_DIST(Pt1, Pt2);

		    if (Dist > d) {
		        IrtRType r1, r2;

			Dist = d;
			*MinPl = Pl;

			/* See if this polyline has crv "Param" attributes. */
			r1 = AttrGetRealAttrib(V -> Attr, "Param");
			r2 = AttrGetRealAttrib(VPrev -> Attr, "Param");
			if (!IP_ATTR_IS_BAD_REAL(r1) && 
			    !IP_ATTR_IS_BAD_REAL(r2) ) {
			    t2 = IRIT_BOUND(t2, 0, 1);
			    *IndexFrac = r1 * t2 + r2 * (1.0 - t2);
			}
			else {
			    if (t2 < 0.0)
			        *IndexFrac = Index;
			    else if (t2 > 1.0)
			        *IndexFrac = Index + 1;
			    else
			        *IndexFrac = Index + t2;
			    t2 = IRIT_BOUND(t2, 0, 1);
			}

			IRIT_PT_BLEND(MinPt, V -> Coord, VPrev -> Coord, t2);
		    }
		}
	    }

	    Index++;
	}
	while (V != NULL && V != Pl -> PVertex);

	/* In case we termporarily closed the polyline into a polygon. */
	if (VLast != NULL)
	    VLast -> Pnext = NULL;
    }

    return Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the closest distance between a point in three space and list of  M
* polys.  Only vertices in the polylines are considered.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Point in 3-space.	                                     M
*   Pls:        List of polylines.                                           M
*   MinPl:      Will be set to the closest poly.                             M
*   MinV:       Will be set to the closest vertex.                           M
*   Index:      Will be set to the index of the closest vertex, starting     M
*		from zero.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Minimal distance from the 3-space point to the polys.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMinDistLineBBox, UserMinDistLinePolygonList  		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMinDistPointPolylineList                                             M
*****************************************************************************/
IrtRType UserMinDistPointPolylineList(const IrtPtType Pt,
				      IPPolygonStruct *Pls,
				      IPPolygonStruct **MinPl,
				      IPVertexStruct **MinV,
				      int *Index)
{
    IPPolygonStruct *Pl;
    IrtRType MinDistSqr;

    /* Initialize with the first vertex. */
    *MinPl = Pls;
    *MinV = Pls -> PVertex;
    MinDistSqr = IRIT_PT_PT_DIST_SQR(Pt, (*MinV) -> Coord);
    *Index = 0;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
        int i = 0;
        IPVertexStruct
	     *V = Pl -> PVertex;

	do {
	    IrtRType
		d = IRIT_PT_PT_DIST_SQR(Pt, V -> Coord);

	    if (d < MinDistSqr) {
	        *MinPl = Pl;
		*MinV = Pl -> PVertex;
		MinDistSqr = d;
		*Index = i;
	    }

	    V = V-> Pnext;
	    i++;
	}
	while (V != Pl -> PVertex && V != NULL);
    }

    return sqrt(MinDistSqr);
}

