/*****************************************************************************
* Computes all intersection points in given set of polylines.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1993   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "misc_lib.h"
#include "geom_lib.h"

#define DIST_PT_PT_2D(Pt1, Pt2) sqrt(IRIT_SQR(Pt1[0] - Pt2[0]) + \
				     IRIT_SQR(Pt1[1] - Pt2[1]))

typedef struct PolylineSegStruct {
    struct PolylineSegStruct *Pnext;
    GMLsIntersectStruct *Inter;
    IPPolygonStruct *Poly;
    int Index;				 /* i'th linear segment in polyline. */
    IrtRType t;		  /* Parameter value between 0 to 1 of intersection. */
} PolylineSegStruct;

static IPPolygonStruct *SplitPolyline(IPPolygonStruct *Poly,
				      PolylineSegStruct *Ps, 
				      IrtRType *StartIndex);
static IrtRType AngularDistanceMeasure(IPVertexStruct *V,
				       GMLsLineSegStruct *Seg);
static IPVertexStruct *PolylineLastSeg(IPPolygonStruct *Poly);
static IrtRType DistPsPs(PolylineSegStruct *Ps1,
			 PolylineSegStruct *Ps2,
			 IPPolygonStruct *Poly,
			 IrtRType StartIndex);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates the appropriate data structures and invokes a plane sweep to find  M
* all intersection points in scene. Intersection points that are found valid M
* (see below) are treated as follows:					     M
* 1. The polyline that is closer to the viewer is not modified.		     M
* 2. The polyline that is far from the viewer is split at the intersection   M
*    location and GlblTrimIntersect amount from both sides of intersection   M
*    is trimmed away as well.						     M
*									     M
*   An intersection is considered valid if the following holds:		     M
* The intersection did not occur between two polylines with the same         M
* Z value, considered with respect to GlblInterSameZ tolerance.	     	     M
*									     M
*   The scene is assumed to be in view space, that is -1 to 1 in X, Y & Z.   M
*   This assumption is used maily so set the GlblInterSameZ tolerances to    M
* reasonable values.							     M
*   Polys are modified in palce and their PAux is exploited.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:     Process poly objects in this list and intersect all their  M
*		  edges.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ProcessIntersections                                                     M
*****************************************************************************/
void ProcessIntersections(IPObjectStruct *PObjects)
{
    IPObjectStruct *PObj;
    int PolyID = 0;
    GMLsLineSegStruct *Ln,
        *LnList = NULL;

    /* Convert the data to the format used by the plane sweep. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	IPPolygonStruct *PPoly;

	if (!IP_IS_POLY_OBJ(PObj))
	    continue;

	if (IP_IS_POLYGON_OBJ(PObj)) {
	    IPPolygonStruct *Polygons, *Polygon;

	    /* Convert to polylines (lines actually since every polyline has */
	    /* only one segment). Allow only lines for which second point    */
	    /* Z value is larger than first.				     */
	    Polygons = PObj -> U.Pl;
	    IP_SET_POLYLINE_OBJ(PObj);
	    PObj -> U.Pl = NULL;
	    for (Polygon = Polygons;
		 Polygon != NULL;
		 Polygon = Polygon -> Pnext) {
		IPVertexStruct *V, *VTmp1, *VTmp2;

		for (V = Polygon -> PVertex;
		     V != NULL && V -> Pnext != NULL;
		     V = V -> Pnext) {
		    if (!IP_IS_INTERNAL_VRTX(V) &&
			(GlblOpenPolyData ||
			 V -> Coord[2] <= V -> Pnext -> Coord[2])) {
			VTmp2 = IPAllocVertex2(NULL);
			VTmp1 = IPAllocVertex2(VTmp2);

			IRIT_PT_COPY(VTmp1 -> Coord, V -> Pnext -> Coord);
			IRIT_PT_COPY(VTmp2 -> Coord, V -> Coord);
			PObj -> U.Pl = IPAllocPolygon(0, VTmp1, PObj -> U.Pl);
		    }
		}

		/* Add the last edge. */
		if (!IP_IS_INTERNAL_VRTX(V) &&
		    (GlblOpenPolyData ||
		     V -> Coord[2] <= Polygon -> PVertex -> Coord[2])) {
		    VTmp2 = IPAllocVertex2(NULL);
		    VTmp1 = IPAllocVertex2(VTmp2);

		    IRIT_PT_COPY(VTmp1 -> Coord, Polygon -> PVertex -> Coord);
		    IRIT_PT_COPY(VTmp2 -> Coord, V -> Coord);
		    PObj -> U.Pl = IPAllocPolygon(0, VTmp1, PObj -> U.Pl);
		}
	    }
	}

	if (!IP_IS_POLYLINE_OBJ(PObj))
	    continue;

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    IPVertexStruct *V;
	    int i = 0;

	    PPoly -> PAux = NULL; /* Will use it later - make sure its NULL. */

	    for (V = PPoly -> PVertex, PolyID++;
		 V != NULL && V -> Pnext != NULL;
		 V = V -> Pnext) {
		PolylineSegStruct
		    *Ps = (PolylineSegStruct *)
			IritMalloc(sizeof(PolylineSegStruct));

		Ln = (GMLsLineSegStruct *)
		    IritMalloc(sizeof(GMLsLineSegStruct));
		IRIT_PT_COPY(Ln -> Pts[0], V -> Coord);
		IRIT_PT_COPY(Ln -> Pts[1], V -> Pnext -> Coord);
		Ln -> Id = PolyID;
		Ln -> Inters = NULL;		 /* No intersections so far. */
		Ln -> Pnext = LnList;
		Ps -> Poly = PPoly;
		Ps -> Index = i++;
		Ln -> PAux = Ps;/* A backpointer to original polyline/index. */
		LnList = Ln;
	    }
	}
    }

    GMLineSweep(&LnList);

    /* Eliminate invalid intersections. */
    for (Ln = LnList; Ln != NULL; Ln = Ln -> Pnext) {
	GMLsIntersectStruct *PrevInter, *Inter;

	for (Inter = Ln -> Inters, PrevInter = NULL; Inter != NULL;) {
	    IrtRType
		Z1 = Ln -> Pts[0][2] * (1.0 - Inter -> t) +
		     Ln -> Pts[1][2] * Inter -> t,
		Z2 = Inter -> OtherSeg -> Pts[0][2] * (1.0 - Inter -> OtherT) +
		     Inter -> OtherSeg -> Pts[1][2] * Inter -> OtherT;

	    if (Z2 - Z1 < GlblInterSameZ) {
		/* Eliminate this intersection - it is invalid. */
		if (PrevInter) {
		    PrevInter -> Pnext = Inter -> Pnext;
		    IritFree(Inter);
		    Inter = PrevInter -> Pnext;
		}
		else {
		    GMLsIntersectStruct
			*TempInter = Inter -> Pnext;

		    IritFree(Inter);
		    Inter = Ln -> Inters = TempInter;
		}
	    }
	    else {
		PrevInter = Inter;
		Inter = Inter -> Pnext;
	    }
	}
    }

    /* Split the polylines at the valid intersections and trim the ends      */
    /* GlblTrimIntersect amount from both sides.			     */
    /*   We first scan the entire data set and update each polyline with its */
    /* set of intersection locations (saved in Polyline PAux pointer).       */
    for (Ln = LnList; Ln != NULL; Ln = Ln -> Pnext) {
	GMLsIntersectStruct
	    *Inter = Ln -> Inters;

	for (Inter = Ln -> Inters; Inter != NULL; Inter = Inter -> Pnext) {
	    PolylineSegStruct
		*Ps = (PolylineSegStruct *) Ln -> PAux,
		*NewPs = (PolylineSegStruct *)
		    IritMalloc(sizeof(PolylineSegStruct));
	    IPPolygonStruct
		*Poly = Ps -> Poly;

	    IRIT_GEN_COPY(NewPs, Ps, sizeof(PolylineSegStruct));
	    NewPs -> t = Inter -> t;
	    NewPs -> Inter = Inter;
	    NewPs -> Pnext = NULL;

	    if (Poly -> PAux) {
		PolylineSegStruct *StepPs, *PrevPs,
		    *HeadPs = (PolylineSegStruct *) Poly -> PAux;

		/* Put this new intersection point in the right order. */
		if (NewPs -> Index + NewPs -> t <
		    HeadPs -> Index + HeadPs -> t) {
		    /* Put it as first one. */
		    Poly -> PAux = NewPs;
		    NewPs -> Pnext = HeadPs;
		}
		else {
		    for (StepPs = HeadPs -> Pnext, PrevPs = HeadPs;
			 StepPs != NULL;
			 PrevPs = StepPs, StepPs = StepPs -> Pnext) {
			if (NewPs -> Index + NewPs -> t <
			    StepPs -> Index + StepPs -> t) {
			    PrevPs -> Pnext = NewPs;
			    NewPs -> Pnext = StepPs;
			    break;
			}
		    }
		    if (StepPs == NULL)
			PrevPs -> Pnext = NewPs;
		}
	    }
	    else
		Poly -> PAux = NewPs;
	}
    }

    /* The second stage - split each polyline at each intersection point. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	IPPolygonStruct *PPoly, *NewPoly;

	if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYLINE_OBJ(PObj))
	    continue;

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    PolylineSegStruct
		*HeadPs = (PolylineSegStruct *) PPoly -> PAux;
	    IPPolygonStruct
		*PStart = PPoly,
		*Pnext = PPoly -> Pnext;
	    IrtRType
		StartIndex = 0.0;

	    while (HeadPs != NULL) {
		PolylineSegStruct
		    *NextPs = HeadPs -> Pnext;

		if ((NewPoly = SplitPolyline(PPoly, HeadPs, &StartIndex))
								     != NULL) {
		    NewPoly -> Pnext = PPoly -> Pnext;
		    PPoly -> Pnext = NewPoly;
		    PPoly = NewPoly;
		}

		IritFree(HeadPs);
		HeadPs = NextPs;
	    }
	    if (PStart -> Pnext != Pnext) {
		IPPolygonStruct  *PTmp;

		/* Polyline was split. Add attributes to signal that. */
		AttrSetIntAttrib(&PStart -> Attr, "widenend", WIDEN_END_END);
		for (PTmp = PStart -> Pnext;
		     PTmp != Pnext;
		     PTmp = PTmp -> Pnext) {
		    AttrSetIntAttrib(&PTmp -> Attr, "widenend",
				     WIDEN_END_START |
				         (PTmp -> Pnext == Pnext ?
					  0 : WIDEN_END_END) );
		}
	    }
	}
    }

#ifdef DEBUG
#define DEBUG_CROSS_SIZE 0.02    
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpInters, FALSE) {
	    IRIT_INFO_MSG(
		    "gsave\n0 setlinewidth\n72 72 scale\n4 5.5 translate\n3 3 scale\n\n");

	    for (Ln = LnList; Ln != NULL; Ln = Ln -> Pnext) {
	        GMLsIntersectStruct *Inter;

		for (Inter = Ln -> Inters;
		     Inter != NULL;
		     Inter = Inter -> Pnext) {
		    IrtRType
		        x = Ln -> Pts[0][0] + Inter -> t * Ln -> _Vec[0],
		        y = Ln -> Pts[0][1] + Inter -> t * Ln -> _Vec[1];

		    IRIT_INFO_MSG("255 0 0 setrgbcolor\n");
		    IRIT_INFO_MSG_PRINTF(
			    "newpath %lf %lf moveto %lf %lf lineto stroke\n",
			    x - DEBUG_CROSS_SIZE, y, x + DEBUG_CROSS_SIZE, y);
		    IRIT_INFO_MSG_PRINTF(
			    "newpath %lf %lf moveto %lf %lf lineto stroke\n",
			    x, y - DEBUG_CROSS_SIZE, x, y + DEBUG_CROSS_SIZE);
		    IRIT_INFO_MSG("0 0 0 setrgbcolor\n");
		}
	    }
	}
    }
#endif /* DEBUG */

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpPolys, FALSE) {
	    IRIT_INFO_MSG("gsave\n");

	    /* if _DebugDumpInters:
	    IRIT_INFO_MSG(
		    "0 setlinewidth\n72 72 scale\n4 5.5 translate\n3 3 scale\n\n");
	    */

	    /* Convert the data to the format used by the plane sweep. */
	    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	        IPPolygonStruct *PPoly;

		if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYLINE_OBJ(PObj))
		    continue;

		for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
		    IPVertexStruct
		        *V = PPoly -> PVertex;

		    if (V != NULL) {
		        IRIT_INFO_MSG_PRINTF("newpath %lf %lf moveto\n",
				             V -> Coord[0], V -> Coord[1]);
			for (V = V -> Pnext; V != NULL; V = V -> Pnext)
			    IRIT_INFO_MSG_PRINTF(" %lf %lf lineto\n",
				                 V -> Coord[0], V -> Coord[1]);
			IRIT_INFO_MSG("stroke\n");
		    }
		}
	    }

	    IRIT_INFO_MSG("grestore\nshowpage\n");
	}
    }
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Splits Poly into two at the specified location by Ps. Location is defined  *
* by a point Index and a parameter (between zero and one) along the line     *
* from point Index to point Index+1.					     *
*   The split point is trimmed with extra distance as specified by the       *
* GlblTrimIntersect global variable.					     *
*   First point of Point is given by StartIndex which is also updated to     *
* hold the new StartIndex after trimming occured.			     *
*   NULL is returned if split is too close to the starting point of Poly.    *
*   Note Poly may result in a polyline that is empty (i.e. no vertices).     *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:        Poly to split into two at Ps.                               *
*   Ps:          Location where to split Poly. Index of edge (vertex) in     *
*		 Poly and parameter between zero and one to that edge.	     *
*   StartIndex:  What we trimmed already. Do nothing if Ps below StartIndex. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   Second half of split poly if split, NULL otherwise. *
*                        First half is in Poly, modified in place.           *
*****************************************************************************/
static IPPolygonStruct *SplitPolyline(IPPolygonStruct *Poly,
				      PolylineSegStruct *Ps, 
				      IrtRType *StartIndex)
{
    int i;
    IrtRType t, TrimmedDist, AngularMeasure;
    IrtPtType InterPt;
    IPVertexStruct *NewV, *InterV,
	*V = Poly -> PVertex;
    IPPolygonStruct *NewPoly;

    if (Ps -> Index + Ps -> t < *StartIndex)
	return NULL;

    /* Find the vertex, the intersection point is between it and the next. */
    for (i = Ps -> Index - ((int) *StartIndex); i > 0 && V != NULL; i--)
	V = V -> Pnext;
    if (V == NULL || V -> Pnext == NULL)
	IRIT_FATAL_ERROR("Vertex index is too large.\n");
    InterV = V;
    AngularMeasure = AngularDistanceMeasure(InterV, Ps -> Inter -> OtherSeg);

    /* Find the exact intersection location and split the polyline. */
    if (((int) *StartIndex) == Ps -> Index) {
	t = *StartIndex - ((int) *StartIndex);
	t = (Ps -> t - t) / (1.0 - t);
    }
    else
	t = Ps -> t;
    IRIT_PT_BLEND(InterPt, InterV -> Pnext -> Coord, InterV -> Coord, t);
    *StartIndex = Ps -> t + Ps ->Index;

    NewV = IPAllocVertex2(InterV -> Pnext);
    IRIT_PT_COPY(NewV -> Coord, InterPt);
    NewPoly = IPAllocPolygon(0, NewV, NULL);

    NewV = IPAllocVertex2(NULL);
    IRIT_PT_COPY(NewV -> Coord, InterPt);
    InterV -> Pnext = NewV;

    /* Trim the end of the first polyline (the original). */
    TrimmedDist = GlblTrimIntersect * AngularMeasure;
    while (Poly -> PVertex != NULL &&
	   Poly -> PVertex -> Pnext != NULL &&
	   TrimmedDist > 0.0) {
	IrtRType Dist;

	V = PolylineLastSeg(Poly);
	Dist = SegmentLength(V);
	if (TrimmedDist > Dist) {
	    /* Eliminate the entire segment. */
	    TrimmedDist -= Dist;
	    V -> Pnext -> Pnext = NULL;
	    IPFreeVertexList(V -> Pnext);
	    V -> Pnext = NULL;
	    if (V == Poly -> PVertex) {   /* Eliminated the entire polyline. */
		V -> Pnext = NULL;
		IPFreeVertexList(V);
		Poly -> PVertex = NULL;
	    }
	}
	else {
	    /* Update the last point to the right distance. */
	    t = TrimmedDist / Dist;
	    TrimmedDist = 0.0;
	    IRIT_PT_BLEND(V -> Pnext -> Coord, V -> Coord, V -> Pnext -> Coord, t);
	}
    }

    /* Trim the beginning of the second polyline. */
    TrimmedDist = GlblTrimIntersect * AngularMeasure;
    if (Ps -> Pnext == NULL ||
	DistPsPs(Ps, Ps -> Pnext, NewPoly, *StartIndex) > TrimmedDist) {
	while (NewPoly -> PVertex != NULL &&
	       NewPoly -> PVertex -> Pnext != NULL &&
	       TrimmedDist > 0.0) {
	    IrtRType Dist;

	    V = NewPoly -> PVertex;
	    Dist = SegmentLength(V);
	    if (TrimmedDist > Dist) {
		/* Eliminate the entire segment. */
		TrimmedDist -= Dist;
		NewPoly -> PVertex = V -> Pnext;
		V -> Pnext = NULL;
		IPFreeVertexList(V);
		V = NewPoly -> PVertex;
		if (V -> Pnext == NULL) { /* Eliminated the entire polyline. */
		    IPFreeVertexList(V);
		    NewPoly -> PVertex = NULL;
		}
		*StartIndex = 1 + ((int) *StartIndex);
	    }
	    else {
		/* Update the first point with the right distance. */
		t = TrimmedDist / Dist;
		TrimmedDist = 0.0;
		IRIT_PT_BLEND(V -> Coord, V -> Pnext -> Coord, V -> Coord, t);
		*StartIndex += t * (1.0 - (*StartIndex - ((int) *StartIndex)));
	    }
	}
    }

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns one over the sine of the angle between the line segment from V     *
* to V -> Pnext and the line segment specified by Seg.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         Vertex to compute one of the sine of angle with Seg.          *
*   Seg:       Segment to compute one of the sine of angle with V.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  One over the sine of the angle between Vand Seg.              *
*****************************************************************************/
static IrtRType AngularDistanceMeasure(IPVertexStruct *V,
				       GMLsLineSegStruct *Seg)
{
    IrtRType CosAngle, AngularMeasure;
    IrtVecType V1, V2;

    if (!GlblAngularDistance)
	return 1.0;

    IRIT_PT_SUB(V1, V -> Pnext -> Coord, V -> Coord);
    IRIT_PT_SUB(V2, Seg -> Pts[0], Seg -> Pts[1]);
    V1[2] = V2[2] = 0.0;
    if (IRIT_PT_LENGTH(V1) <= IRIT_PT_NORMALIZE_ZERO ||
	IRIT_PT_LENGTH(V2) <= IRIT_PT_NORMALIZE_ZERO)
	return 1.0;

    IRIT_PT_NORMALIZE(V1);
    IRIT_PT_NORMALIZE(V2);

    CosAngle = IRIT_DOT_PROD(V1, V2);

    AngularMeasure = 1.0 / sqrt(1.0 + IRIT_EPS - IRIT_SQR(CosAngle));

    return IRIT_MIN(AngularMeasure, 5.0);/* Make sure it is not too far off. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a pointer to last line segment of polyline. The returned pointer   *
* is the address of V where V -> Pnext is the last vertex in polyline.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:        To find its last segment.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   First vertex of last segment (together with its      *
*                       Pnext).                                              *
*****************************************************************************/
static IPVertexStruct *PolylineLastSeg(IPPolygonStruct *Poly)
{
    IPVertexStruct
	*V = Poly -> PVertex;

    if (V == NULL || V -> Pnext == NULL)
	return NULL;

    while (V -> Pnext -> Pnext != NULL)
	V = V -> Pnext;

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the XY length of the line segment from V to V -> Pnext vertices.  M
*                                                                            *
* PARAMETERS:                                                                M
*   V:         First vertex of edge (together with its Pnext).   	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  Length of edge.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SegmentLength                                                            M
*****************************************************************************/
IrtRType SegmentLength(IPVertexStruct *V)
{
    return DIST_PT_PT_2D(V -> Coord, V -> Pnext -> Coord);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the XY length along Poly from Ps1 to Ps2			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Ps1, Ps2:     Locations on Poly (index of edge + parameter along edge)   *
*   Poly:         For which distance along of two points is to be computed.  *
*   StartIndex:   Index in Ps1/2 is relative to this.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:     XY distance.                                               *
*****************************************************************************/
static IrtRType DistPsPs(PolylineSegStruct *Ps1,
			 PolylineSegStruct *Ps2,
			 IPPolygonStruct *Poly,
			 IrtRType StartIndex)
{
    int i;
    IrtRType Dist;
    IrtPtType Pt;
    IPVertexStruct
	*V = Poly -> PVertex;

    /* Find length of segment of first Ps. */
    for (i = Ps1 -> Index - ((int) StartIndex);
	 i > 0 && V != NULL && V -> Pnext != NULL;
	 i--)
	V = V -> Pnext;
    IRIT_PT_BLEND(Pt, V -> Pnext -> Coord, V -> Coord, Ps1 -> t);
    Dist = DIST_PT_PT_2D(Pt, V -> Pnext -> Coord);

    /* Accumulates the full line segments in between. */
    for (i = Ps2 -> Index - Ps1 -> Index, V = V -> Pnext;
	 i > 1 && V != NULL && V -> Pnext != NULL;
	 i++, V = V -> Pnext) {
	Dist += SegmentLength(V);
    }

    /* Add the Ps2 partial length. */
    if (Ps1 -> Index != Ps2 -> Index && V != NULL && V -> Pnext != NULL) {
	IRIT_PT_BLEND(Pt, V -> Pnext -> Coord, V -> Coord, Ps2 -> t);
	Dist += DIST_PT_PT_2D(Pt, V -> Coord);
    }

    return Dist;
}
