/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle the low level boolean operations. The routines in this  *
* module should be accessed from "bool-hi.c" module only.		     *
*   Note the polygons of the two given objects may not be convex, and must   *
* be subdivided into convex ones in the boolean upper level routines (module *
* bool-hi.c). All the routines of this module expects objects with convex    *
* polygons only although they might return objects with non convex polygons, *
* but marked as so (via polygons CONVEX tags - see IPPolygonStruct def.)!    *
*   Because Bool-low.c module was too big, it was subdivided to two:	     *
* Bool1Low.c - mainly handles the intersection polyline between the oper.    *
* Bool2Low.c - mainly handles the polygon extraction from operands given the *
*	       polyline of intersection and the adjacencies (see ADJACNCY.C) *
* Note we said mainly has routines CAN call one another!		     *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "allocate.h"
#include "bool_loc.h"
#include "geom_lib.h"

#ifdef DEBUG
/* Print messages to entry/exit from routines. */
IRIT_SET_DEBUG_PARAMETER(_DebugEntryExit, FALSE);
#endif /* DEBUG */

IRIT_STATIC_DATA char
    *GlblDisjointParts = NULL;
IRIT_STATIC_DATA int
    GlblWarningWasIssued = FALSE,
    GlblObjsIntersects = FALSE,
    GlblKeepEdgeIntersection = FALSE,
    GlblPolySortAxis = 0;

static IPObjectStruct *BooleanLowGenInOut(IPObjectStruct *PObj1, int InOut);
static void BooleanLowInterAll(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
static void BooleanLowInterSelf(IPObjectStruct *PObj);
static int BooleanLowAdjacentPolys(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2);
static void BooleanLowInterOne(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2);
static InterSegmentStruct *InterSegmentPoly(IPPolygonStruct *Pl,
					    IPPolygonStruct *SegPl,
					    IrtPtType Segment[2]);
static void SwapPointInterList(InterSegmentStruct *PSeg);
static void RemoveSegInterList(InterSegmentStruct *PSeg,
			       InterSegmentStruct **PSegList);
static InterSegmentStruct *FindMatchInterList(IrtPtType Pt,
					      InterSegmentStruct **PSegList);
static void SortOpenReverseLoop(SortOpenStruct *PSHead);
static IrtRType SortOpenInsertOne(SortOpenStruct **PSHead,
				  SortOpenStruct *PSTemp,
				  IrtPtType Pt,
				  IPVertexStruct *V,
				  IPPolygonStruct *Pl);
static IPObjectStruct *PolylineFromInterSeg(IPObjectStruct *PObj);
static IPPolygonStruct *PolylineFromInterSegAux(InterSegListStruct *PSegs);
static IrtRType *EvalUVFromBaryCentric(IrtRType *Pt, IPPolygonStruct *Pl);
#if defined(ultrix) && defined(mips)
static int BooleanPrepObjectCmpr(VoidPtr VPoly1, VoidPtr VPoly2);
#else
static int BooleanPrepObjectCmpr(const VoidPtr VPoly1, const VoidPtr VPoly2);
#endif /* ultrix && mips (no const support) */

#ifdef DEBUG
static void BoolVerifyInterList(InterSegListStruct *PIntList);
static void BoolPrintVrtxList(IPVertexStruct *V);
static void BoolPrintInterList(InterSegListStruct *PIntList);
static void BoolPrintInterSegment(InterSegmentStruct *PInt);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the part of PObj1 which is out of PObj2:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object of Boolean operation.                             M
*   PObj2:    Second object of Boolean operation.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Result of one out two.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanLow1In2, BooleanLowSelfInOut                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanLow1Out2, Booleans                                                M
*****************************************************************************/
IPObjectStruct *BooleanLow1Out2(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    int i, n;
    IPObjectStruct *PObj;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLow1Out2\n");
#endif /* DEBUG */

    BoolGenAdjacencies(PObj1);
    n = BoolMarkDisjointParts(PObj1);

    GlblDisjointParts = (char *) IritMalloc((n + 1) * sizeof(char));
    IRIT_ZAP_MEM(GlblDisjointParts, (n + 1) * sizeof(char));

    /* Find all intersections of PObj1 polygons with PObj2 polygons. */
    GlblObjsIntersects = GlblWarningWasIssued = FALSE;
    BooleanLowInterAll(PObj1, PObj2);

    /* Generate all the polygons in PObj1 which are out of PObj2. */
    if (!GlblObjsIntersects) {
	FatalBooleanError(FTL_BOOL_NO_INTER);
    }

    PObj = BooleanLowGenInOut(PObj1, FALSE);

    /* Append the disjoint parts outside the intersection zone. */
    for (i = 1; i <= n; i++) {
	if (!GlblDisjointParts[i]) {
	    /* This disjoint part is not part of intersection - fetch it. */
	    PObj -> U.Pl = IPAppendPolyLists(PObj -> U.Pl,
					     BoolGetDisjointPart(PObj1, i));
	}
    }

    IritFree(GlblDisjointParts);
    GlblDisjointParts = NULL;

    BoolClnAdjacencies(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the part of PObj1 which is in of PObj2:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object of Boolean operation.                             M
*   PObj2:    Second object of Boolean operation.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Result of one in two.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanLow1Out2, BooleanLowSelfInOut                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanLow1In2, Booleans                                                 M
*****************************************************************************/
IPObjectStruct *BooleanLow1In2(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLow1In2\n");
#endif /* DEBUG */

    BoolGenAdjacencies(PObj1);
    GlblDisjointParts = NULL;

    /* Find all intersections of PObj1 polygons with PObj2 polygons. */
    GlblObjsIntersects = GlblWarningWasIssued = FALSE;
    BooleanLowInterAll(PObj1, PObj2);

    /* Generate all the polygons in PObj1 which are in PObj2. */
    if (!GlblObjsIntersects) {
	FatalBooleanError(FTL_BOOL_NO_INTER);
    }

    PObj = BooleanLowGenInOut(PObj1, TRUE);

    BoolClnAdjacencies(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the part of PObj which is in/out of itself:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object of Boolean operation with itself (self intersection).  M
*   InOut:     What are we looking for? in or out.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Result of Boolean in/out with self.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanLow1Out2, BooleanLow1In2                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanLowSelfInOut, self intersection, Booleans                         M
*****************************************************************************/
IPObjectStruct *BooleanLowSelfInOut(IPObjectStruct *PObj, int InOut)
{
#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLow1Out2\n");
#endif /* DEBUG */

    BoolGenAdjacencies(PObj);

    /* Find all intersections of PObj1 polygons with PObj2 polygons. */
    GlblObjsIntersects = GlblWarningWasIssued = FALSE;
    BooleanLowInterSelf(PObj);

    /* Generate all the polygons in PObj1 which are in/out of PObj2. */
    if (!GlblObjsIntersects) {
	FatalBooleanError(FTL_BOOL_NO_INTER);
    }

    PObj = BooleanLowGenInOut(PObj, FALSE);

    BoolClnAdjacencies(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans the InterSegmentList of each polygon and decides using Inter.      *
* list, if it is IN relative to the other object. Note that for polygons     *
* that do not intersect at all, we use the polygon adjacencies to decide     *
* if they are IN or not.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      To decide its portion that is in or out.                      *
*   InOut:     What are we looking for? in or out.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *: Resulting set of polygons that are in the requested    *
*                     half space.                                            *
*****************************************************************************/
static IPObjectStruct *BooleanLowGenInOut(IPObjectStruct *PObj, int InOut)
{
    if (BoolOutputInterCurve) {
	/* Return a polyline object - extract the InterSegment list of each */
	/* polygon into open/closed polyline loops object.		    */
	return PolylineFromInterSeg(PObj);
    }
    else {
	IPObjectStruct *PObjNew;
	IPPolygonStruct *Pl;

	if (BoolHandleCoplanarPoly) {
	    /* Purge all polygons that were tagged being coplanar. */
	    PObjNew = BoolExtractPolygons(PObj, InOut);
	    Pl = PObjNew -> U.Pl;

	    while (Pl != NULL && IS_COPLANAR_POLY(Pl)) {
	        PObjNew -> U.Pl = PObjNew -> U.Pl -> Pnext;
	        IPFreePolygon(Pl);
		Pl = PObjNew -> U.Pl;
	    }
	    if (Pl != NULL) {
		while (Pl -> Pnext != NULL) {
		    if (IS_COPLANAR_POLY(Pl -> Pnext)) {
			IPPolygonStruct
			    *PlTemp = Pl -> Pnext;

			Pl -> Pnext = PlTemp -> Pnext;
			IPFreePolygon(PlTemp);
	            }
	            else
		        Pl = Pl -> Pnext;
		}
	    }
	}
	else
	    PObjNew = BoolExtractPolygons(PObj, InOut);

	for (Pl = PObjNew -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    RST_BOOL_FLAGS_POLY(Pl);
	}

	return PObjNew;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates a polyline object out of the intersection list of the polygons.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:     Polygon for which the intersection loops are to be formed.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Resulting loops.                                      *
*****************************************************************************/
static IPObjectStruct *PolylineFromInterSeg(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjRet;
    IPPolygonStruct
	*PlObj = PObj -> U.Pl,
	*PlHead = NULL;

    while (PlObj != NULL) {
	IPPolygonStruct *Pl;
	InterSegListStruct *PClosed, *POpen;

	BoolLoopsFromInterList(PlObj, &PClosed, &POpen);

	if ((Pl = PolylineFromInterSegAux(POpen)) != NULL) {
	    IPPolygonStruct
		*PlLast = IPGetLastPoly(Pl);

	    PlLast -> Pnext = PlHead;
	    PlHead = Pl;
	}
	if ((Pl = PolylineFromInterSegAux(PClosed)) != NULL) {
	    IPPolygonStruct
		*PlLast = IPGetLastPoly(Pl);

	    PlLast -> Pnext = PlHead;
	    PlHead = Pl;
	}

	PlObj = PlObj -> Pnext;
    }

    PObjRet = IPGenPolyObject("", GMMergePolylines(PlHead, IRIT_EPS), NULL);
    IP_SET_POLYLINE_OBJ(PObjRet);	     /* Mark it as polyline object. */
    return PObjRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of PolylineFromInterSeg.                              *
*****************************************************************************/
static IPPolygonStruct *PolylineFromInterSegAux(InterSegListStruct *PSegs)
{
    IPPolygonStruct
	*PlHead = NULL;

    while (PSegs != NULL) {
	InterSegmentStruct
	    *PInter = PSegs -> PISeg;
	InterSegListStruct 
	    *PSegsNext = PSegs -> Pnext;

	while (PInter) {
	    InterSegmentStruct
		*PInterNext = PInter -> Pnext;
	    IPVertexStruct
	        *V2 = IPAllocVertex2(NULL),
	        *V1 = IPAllocVertex2(V2);
	    IPPolygonStruct
	        *PlTemp = IPAllocPolygon(0, V1, PlHead);

	    PlHead = PlTemp;

	    if (BoolParamSurfaceUVVals) {
	        IrtRType *UVVals;

		UVVals = EvalUVFromBaryCentric(PInter -> PtSeg[0],
					       PInter -> Pl);
		IRIT_UV_COPY(V1 -> Coord, UVVals);

		UVVals = EvalUVFromBaryCentric(PInter -> PtSeg[1],
					       PInter -> Pl);
		IRIT_UV_COPY(V2 -> Coord, UVVals);

		V1 -> Coord[2] = V2 -> Coord[2] = 0.0;
	    }
	    else {
		IRIT_PT_COPY(V1 -> Coord, PInter -> PtSeg[0]);
		IRIT_PT_COPY(V2 -> Coord, PInter -> PtSeg[1]);
	    }
	    IRIT_VEC_RESET(V1 -> Normal);
	    IRIT_VEC_RESET(V2 -> Normal);

	    IritFree(PInter);
	    PInter = PInterNext;
	}

	IritFree(PSegs);
	PSegs = PSegsNext;
    }

    return PlHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the UV value of the given point Pt in triangle Pl. Pl is assumed  *
* to have UV uvvals attributes on its vertices.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:      To compute its UV values from the UV values of vertices pf Pl.  *
*   Pl:      Polygon assumed to contain Pt with UV uvvals attributes.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:  The UV values in a statically allocated point.              *
*****************************************************************************/
static IrtRType *EvalUVFromBaryCentric(IrtRType *Pt, IPPolygonStruct *Pl)
{
    IRIT_STATIC_DATA IrtUVType RetVal;
    int i,
        n = IPVrtxListLen(Pl -> PVertex);
    IrtRType
        *Wgt = (IrtRType *) IritMalloc(sizeof(IrtRType) * n);
    IPVertexStruct *V;

    if (GMEvalWeightsVFromPl(Pt, Pl, Wgt)) {
        IRIT_UV_RESET(RetVal);

        for (i = 0, V = Pl -> PVertex; i < n; i++, V = V -> Pnext) {
	    if (Wgt[i] >= 0.0) {
	        float
		    *UVs = AttrGetUVAttrib(V -> Attr, "uvvals");

		if (UVs == NULL) {
		    if (!GlblWarningWasIssued) {
		        IRIT_WARNING_MSG("Boolean: Failed to find UV attribute values.");
			GlblWarningWasIssued = TRUE;
		    }

		    IRIT_UV_RESET(RetVal);
		    IritFree(Wgt);
		    return RetVal;
		}
		else {
		    RetVal[0] += Wgt[i] * UVs[0];
		    RetVal[1] += Wgt[i] * UVs[1];
		}
	    }
	}
	IritFree(Wgt);
    }
    else
    {
	if (!GlblWarningWasIssued) {
	    IRIT_WARNING_MSG("Boolean: Failed to compute barycentric coordinates.");
	    GlblWarningWasIssued = TRUE;
	}
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two polygon's bbox value for sorting purposes.        *
*                                                                            *
* PARAMETERS:                                                                *
*   VPoly1, VPoly2:  Two pointers to polygons.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two polygons.            *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int BooleanPrepObjectCmpr(VoidPtr VPoly1, VoidPtr VPoly2)
#else
static int BooleanPrepObjectCmpr(const VoidPtr VPoly1, const VoidPtr VPoly2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((IPPolygonStruct **) VPoly1)) -> BBox[0][GlblPolySortAxis] -
	       (*((IPPolygonStruct **) VPoly2)) -> BBox[0][GlblPolySortAxis];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute BBox for all polygons in provided object PObj. Also,  M
* the polygons are sorted in the list with according to their minimal BBox   M
* value in GlblPolySortAxis axis.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    To prepare.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanPrepObject                                                        M
*****************************************************************************/
void BooleanPrepObject(IPObjectStruct *PObj)
{
    int i,
	PolyCount = 0;
    IPPolygonStruct **PlArray, **PlTemp,
	*Pl = PObj -> U.Pl;

    /* Count how many polygons this object have and make sure all have bbox. */
    while (Pl != NULL) {
	PolyCount++;

	if (BoolHandleCoplanarPoly)
	    RST_COPLANAR_POLY(Pl);

	if (!IP_HAS_BBOX_POLY(Pl)) {			  /* Compute it now. */
	    GMBBBboxStruct
		*BBox = GMBBComputeOnePolyBbox(Pl);

	    IRIT_PT_COPY(Pl -> BBox[0], BBox -> Min);
	    IRIT_PT_COPY(Pl -> BBox[1], BBox -> Max);

	    IP_SET_BBOX_POLY(Pl);
	}

#	ifdef DEBUG
	/* Sanity check - make sure the plane equation is right. */
	{
	    IPVertexStruct 
	        *V = Pl -> PVertex;

	    do {
	        assert(IRIT_FABS(IRIT_DOT_PROD(Pl -> Plane,
					       V -> Coord) + Pl -> Plane[3])
								  < IRIT_EPS);
		V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);

	}
#	endif /* DEBUG */

    	Pl = Pl -> Pnext;
    }

    if (PolyCount == 0) {
	IRIT_WARNING_MSG("Boolean: empty object has been provided.\n");
	return;
    }

    /* Sort the polygons with according to minimal BBox in GlblPolySortAxis. */
    PlArray = (IPPolygonStruct **) IritMalloc(sizeof(IPPolygonStruct *) *
							PolyCount);
    for (Pl = PObj -> U.Pl, PlTemp = PlArray; Pl != NULL; Pl = Pl -> Pnext)
	*PlTemp++ = Pl;

    qsort(PlArray, PolyCount, sizeof(IPPolygonStruct *),
	  BooleanPrepObjectCmpr);

    for (i = 1; i < PolyCount; i++)
	PlArray[i - 1] -> Pnext = PlArray[i];
    PlArray[PolyCount - 1] -> Pnext = NULL;
    PObj -> U.Pl = PlArray[0];
    IritFree(PlArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to set polygonal sorting axis.	  			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolySortAxis:    Sorting axis. Either 0(x), 1 (y), or 2 (z).             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Old value.                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetPolySortAxis, Booleans                                            M
*****************************************************************************/
int BoolSetPolySortAxis(int PolySortAxis)
{
    int Old = GlblPolySortAxis;

    GlblPolySortAxis = PolySortAxis;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to find all the intersections between all PObj1 polygons with    *
* PObj2 polygons. The intersections are saved as a list of segments (struct  *
* InterSegmentStruct) in each of PObj1 polygons using the PAux pointer (see  *
* IPPolygonStruct). Note PObj2 is not modified at all, and in PObj1, only    *
* PAux of each polygon is set to the segment list, or NULL if none.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1:        First object to compute intersection for.                  *
*   PObj2:        Second object to compute intersection for.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BooleanLowInterAll(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPPolygonStruct *Pl1, *Pl2, *Pl2Start;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLowInterAll\n");
#endif /* DEBUG */

    /* Sort polygons and compute BBox for them if none exists. */
    BooleanPrepObject(PObj1);
    BooleanPrepObject(PObj2);
    Pl2Start = PObj2 -> U.Pl;
    
    Pl1 = PObj1 -> U.Pl;
    while (Pl1 != NULL) {
	Pl1 -> PAux = NULL;	   /* Empty InterSegment list to start with: */

	/* Intersect Pl1 against the Pl2 list until end of Pl2 list or       */
	/* until the minimum of Pl2 polygon is larger than Pl1 maximum.      */
	Pl2 = Pl2Start;
	while (Pl2 != NULL &&
	       Pl1 -> BBox[1][GlblPolySortAxis] >=
					Pl2 -> BBox[0][GlblPolySortAxis]) {
	    if (Pl1 -> BBox[1][0] < Pl2 -> BBox[0][0] ||
		Pl1 -> BBox[1][1] < Pl2 -> BBox[0][1] ||
		Pl1 -> BBox[1][2] < Pl2 -> BBox[0][2] ||
		Pl2 -> BBox[1][0] < Pl1 -> BBox[0][0] ||
		Pl2 -> BBox[1][1] < Pl1 -> BBox[0][1] ||
		Pl2 -> BBox[1][2] < Pl1 -> BBox[0][2]) {
		/* The Bounding boxes do not intersect, skip polygons. */
	    }
	    else {
	        IrtRType MinDist;

		if ((!GMPolygonPlaneInter(Pl1, Pl2 -> Plane, &MinDist) &&
		     MinDist > IRIT_EPS) ||
		    (!GMPolygonPlaneInter(Pl2, Pl1 -> Plane, &MinDist) &&
		     MinDist > IRIT_EPS)) {
		    /* Plane of one polygon does not intersect other, skip. */
		}
		else
		    BooleanLowInterOne(Pl1, Pl2);
	    }

	    /* If Pl2 maximum is smaller than Pl1 minimum there is no reason */
	    /* to consider this Pl2 polygon any more as it cannot intersect  */
	    /* any more polygons in the Pl1 list.                            */
	    if (Pl2 -> BBox[1][GlblPolySortAxis] <
                          Pl1 -> BBox[0][GlblPolySortAxis] &&
		Pl2Start == Pl2)
		Pl2Start = Pl2 -> Pnext;
                                                        
	    Pl2 = Pl2 -> Pnext;
	}

	if (Pl1 -> PAux != NULL) {		     /* If any intersection. */
	    GlblObjsIntersects = TRUE;
	    if (GlblDisjointParts)
	        GlblDisjointParts[BOOL_DISJ_GET_INDEX(Pl1)] = TRUE;
	}

	Pl1 = Pl1 -> Pnext;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Exit BooleanLowInterAll\n");
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to find all the intersections between all PObj's polygons with   *
* themselves. The intersections are saved as a list of segments (struct      *
* InterSegmentStruct) in PObj's polygons using the PAux pointer (see         *
* IPPolygonStruct).							     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Object to compute self intersection for.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BooleanLowInterSelf(IPObjectStruct *PObj)
{
    IPPolygonStruct *Pl1, *Pl2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLowInterSelf\n");
#endif /* DEBUG */

    /* Sort polygons and compute BBox for them if none exists. */
    BooleanPrepObject(PObj);
        
    for (Pl1 = PObj -> U.Pl; Pl1 != NULL; Pl1 = Pl1 -> Pnext)
	Pl1 -> PAux = NULL;	   /* Empty InterSegment list to start with: */

    for (Pl1 = PObj -> U.Pl; Pl1 != NULL; Pl1 = Pl1 -> Pnext) {
 	/* Intersect Pl1 against the Pl2 list until end of Pl2 list or       */
	/* until the minimum of Pl2 polygon is larger than Pl1 maximum.      */
	Pl2 = Pl1 -> Pnext;
	while (Pl2 != NULL &&
	       Pl1 -> BBox[1][GlblPolySortAxis] >=
					Pl2 -> BBox[0][GlblPolySortAxis]) {
	    if ((Pl1 -> BBox[1][0] < Pl2 -> BBox[0][0] ||
		 Pl1 -> BBox[1][1] < Pl2 -> BBox[0][1] ||
		 Pl1 -> BBox[1][2] < Pl2 -> BBox[0][2] ||
		 Pl2 -> BBox[1][0] < Pl1 -> BBox[0][0] ||
		 Pl2 -> BBox[1][1] < Pl1 -> BBox[0][1] ||
		 Pl2 -> BBox[1][2] < Pl1 -> BBox[0][2]) ||
		BooleanLowAdjacentPolys(Pl1, Pl2)) {
		/* The Bounding boxes do not intersect or the two polygons   */
		/* share a common edge or vertex, skip these polygons.	     */
	    }
	    else {
		BooleanLowInterOne(Pl1, Pl2);

		/* If we are to compute a self intersection for a surface,   */
		/* one needs to keep both intersecting edges. Invoke again.  */
		if (BoolParamSurfaceUVVals)
		    BooleanLowInterOne(Pl2, Pl1);
	    }

	    Pl2 = Pl2 -> Pnext;
	}

	if (Pl1 -> PAux != NULL)		     /* If any intersection. */
	    GlblObjsIntersects = TRUE;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Exit BooleanLowInterSelf\n");
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to test if the given two polygons share a vertex or not.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1, Pl2:  Two polygons to test for a shared vertex.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if two polygons share a vertex, FALSE otherwise.		     *
*****************************************************************************/
static int BooleanLowAdjacentPolys(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2)
{
    IPVertexStruct
	*V1 = Pl1 -> PVertex;

    do {
	IPVertexStruct
	    *V2 = Pl2 -> PVertex;

	do {
	    if (IRIT_PT_APX_EQ(V1 -> Coord, V2 -> Coord))
	        return TRUE;

	    V2 = V2 -> Pnext;
	}
	while (V2 != NULL && V2 != Pl2 -> PVertex);

	V1 = V1 -> Pnext;
    }
    while (V1 != NULL && V1 != Pl1 -> PVertex);
    
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to intersect polygon Pl1, with polygon Pl2. If found common      M
* intersection, that segment will be added to the InterSegmentStruct list    M
* saved in Pl1 PAux list.						     M
*   Note that as the two polygons convex, at most one line segment can       M
* result from such intersection (of two non coplanar polygons).		     M
*   Algorithm: intersect all Pl2 edges with Pl1 plane. If found that         M
* (exactly) two vertices (one segment) of Pl2 do intersect Pl1 plane then:   M
* Perform clipping of the segment against Pl1. If result is not empty, add   M
* the result segment to Pl1 InterSegmentStruct list (saved at PAux of	     M
* polygon - see IPPolygonStruct).					     M
*									     *
* PARAMETERS:                                                                M
*   Pl1:       First polygon to compute intersection for.                    M
*   Pl2:       Second polygon to compute intersection for.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The intersection segment, if any, NULL otherwise.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolInterPolyPoly		                                             M
*****************************************************************************/
IPPolygonStruct *BoolInterPolyPoly(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2)
{
    int KeepEdgeIntersection = GlblKeepEdgeIntersection;
    IPPolygonStruct
	*Pl = NULL;
    InterSegmentStruct *PLSeg;

    /* Compute the intersection edges. */
    GlblKeepEdgeIntersection = TRUE;
    BooleanLowInterOne(Pl1, Pl2);
    GlblKeepEdgeIntersection = KeepEdgeIntersection;

    /* Convert the intersection edges into regular polylines. */
    if ((PLSeg = (InterSegmentStruct *) Pl1 -> PAux) != NULL) {
	IPVertexStruct
	    *V2 = IPAllocVertex2(NULL),
	    *V1 = IPAllocVertex2(V2);

	IRIT_PT_COPY(V1 -> Coord, PLSeg -> PtSeg[0]);
	IRIT_PT_COPY(V2 -> Coord, PLSeg -> PtSeg[1]);
	Pl = IPAllocPolygon(0, V1, NULL);

	IritFree(PLSeg);
	Pl1 -> PAux = NULL;
    }

    return Pl;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to intersect polygon Pl1, with polygon Pl2. If found common      *
* intersection, that segment will be added to the InterSegmentStruct list    *
* saved in Pl1 PAux list.						     *
*   Note that as the two polygons convex, at most one line segment can       *
* result from such intersection (of two non coplanar polygons).		     *
*   Algorithm: intersect all Pl2 edges with Pl1 plane. If found that         *
* (exactly) two vertices (one segment) of Pl2 do intersect Pl1 plane then:   *
* Perform clipping of the segment against Pl1. If result is not empty, add   *
* the result segment to Pl1 InterSegmentStruct list (saved at PAux of	     *
* polygon - see IPPolygonStruct).					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1:       First polygon to compute intersection for.                    *
*   Pl2:       Second polygon to compute intersection for.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BooleanLowInterOne(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2)
{
    int NumOfInter = 0;
    IrtRType TInter[2],
	*Plane1 = Pl1 -> Plane,			       /* For faster access. */
	*Plane2 = Pl2 -> Plane;
    IrtPtType Inter[2], Dir;
    IPVertexStruct *Vnext,
	*V = Pl2 -> PVertex;
    InterSegmentStruct *PSeg, *PLSeg;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BooleanLowInterOne\n");
#endif /* DEBUG */

    if (IRIT_PLANE_APX_EQ_EPS(Plane1, Plane2, BOOL_IRIT_EPS)) {
	/* The two polygons are coplanar, do not attempt to intersect. */
	if (BoolHandleCoplanarPoly) {
	    IrtHmgnMatType RotMat;
	    IPPolygonStruct *Pl1XY, *Pl2XY;
	    IrtPtType Mid1, Mid2;

	    GMGenRotateMatrix(RotMat, Pl1 -> Plane);
	    Pl1XY = BooleanComputeRotatedPolys(Pl1, TRUE, RotMat);
	    Pl2XY = BooleanComputeRotatedPolys(Pl2, TRUE, RotMat);

	    IRIT_PT_BLEND(Mid1, Pl1XY -> PVertex -> Coord,
			   Pl1XY -> PVertex -> Pnext -> Coord, BOOL_MID_BLEND);
	    IRIT_PT_BLEND(Mid1, Mid1,
		     Pl1XY -> PVertex -> Pnext -> Pnext -> Coord,
		     BOOL_MID_BLEND);
	    IRIT_PT_BLEND(Mid2, Pl2XY -> PVertex -> Coord,
			   Pl2XY -> PVertex -> Pnext -> Coord, BOOL_MID_BLEND);
	    IRIT_PT_BLEND(Mid2, Mid2,
		     Pl2XY -> PVertex -> Pnext -> Pnext -> Coord,
		     BOOL_MID_BLEND);

	    if (Boolean2DComputeInters(Pl1XY, Pl2XY, TRUE, TRUE) ||
		(GMPolygonRayInter(Pl1XY, Mid2, 0) & 0x01) == 1 ||
		(GMPolygonRayInter(Pl2XY, Mid1, 0) & 0x01) == 1) {
		SET_COPLANAR_POLY(Pl1);
		SET_COPLANAR_POLY(Pl2);
		BoolFoundCoplanarPoly = TRUE;
	    }
	    IPFreePolygonList(Pl1XY);
	    IPFreePolygonList(Pl2XY);
	}
	else {
	    if (!GlblWarningWasIssued) {
		IRIT_WARNING_MSG("Boolean: coplanar polygons detected. Enable COPLANAR state.");
		GlblWarningWasIssued = TRUE;
	    }
	}
    }
    else {
	do {
	    Vnext = V -> Pnext;
	    IRIT_PT_SUB(Dir, Vnext -> Coord, V -> Coord);
	    if (GMPointFromLinePlane01(V -> Coord, Dir, Plane1,
				 Inter[NumOfInter], &TInter[NumOfInter]) &&
	        ((NumOfInter == 0) || (NumOfInter == 1 &&
				       !BOOL_IRIT_PT_APX_EQ(Inter[0], Inter[1]))))
		NumOfInter++;
	    if (NumOfInter == 2)
		break;			  /* Cannot have more intersections. */

	    V = Vnext;
	}
	while (V != NULL && V != Pl2 -> PVertex);
    }

    switch (NumOfInter) {
	case 0:
	    break;
	case 1:
	    /* One intersection is possible if only one vertex of Pl2 is in  */
	    /* the plane of Pl1, all other vertices are on one side of plane.*/
	    break;
	case 2:
	    if (GlblKeepEdgeIntersection &&
		((IRIT_APX_EQ_EPS(TInter[0], 0.0, BOOL_IRIT_REL_EPS) &&
		  IRIT_APX_EQ_EPS(TInter[1], 1.0, BOOL_IRIT_REL_EPS)) ||
		 (IRIT_APX_EQ_EPS(TInter[1], 0.0, BOOL_IRIT_REL_EPS) &&
		  IRIT_APX_EQ_EPS(TInter[0], 1.0, BOOL_IRIT_REL_EPS)))) {
		/* The intersection segment is on an edge of Pl2.            */
		/* Will have two such occurances as we have two polygons     */
		/* that share this edge - keep only one (that is inside).    */
		IrtVecType Diff;

		V = Pl2 -> PVertex;
		do {
		    if (!IRIT_PT_APX_EQ_EPS(V -> Coord, Inter[0],
					    BOOL_IRIT_REL_EPS) &&
			!IRIT_PT_APX_EQ_EPS(V -> Coord, Inter[1],
					    BOOL_IRIT_REL_EPS))
			break;

		    V = V -> Pnext;
		}
		while (V != NULL && V != Pl2 -> PVertex);
		IRIT_PT_SUB(Diff, V -> Coord, Inter[0]);
		if (IRIT_DOT_PROD(Diff, Plane1) < 0.0)
		    break;
	    }

	    /* Clip the segment against the polygon and insert if not empty: */
	    if ((PSeg = InterSegmentPoly(Pl1, Pl2, Inter)) != NULL) {
		/* insert that segment to list of Pl1. Note however that the */
		/* intersection may be exactly on 2 other polygons boundary, */
		/* And therefore creates the same intersection edge TWICE!   */
		/* Another possiblity is on same case, the other polygon     */
		/* will have that inter. edge on its edge, and its ignored.  */
		/* We therefore test for duplicates and ignore edge if so.   */
		if (PSeg -> V[0] != NULL && PSeg -> V[0] == PSeg -> V[1]) {
		    IritFree(PSeg);		       /* Ignore it! */
		    return;
		}
		PLSeg = (InterSegmentStruct *) Pl1 -> PAux;
		while (PLSeg != NULL) {
		    if ((BOOL_IRIT_PT_APX_EQ(PSeg -> PtSeg[0],
					     PLSeg -> PtSeg[0]) &&
			 BOOL_IRIT_PT_APX_EQ(PSeg -> PtSeg[1],
					     PLSeg -> PtSeg[1])) ||
			(BOOL_IRIT_PT_APX_EQ(PSeg -> PtSeg[0],
					     PLSeg -> PtSeg[1]) &&
			 BOOL_IRIT_PT_APX_EQ(PSeg -> PtSeg[1],
					     PLSeg -> PtSeg[0]))) {
			IritFree(PSeg);	       /* Ignore it! */
			return;
		    }
		    PLSeg = PLSeg -> Pnext;
		}

		PSeg -> Pnext = (InterSegmentStruct *) Pl1 -> PAux;
		Pl1 -> PAux = (VoidPtr) PSeg;
	    }
	    break;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Exit BooleanLowInterOne\n");
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Intersects the given segment (given as two end points), with the given   *
* polygon (which must be convex). Upto two intersections are possible, as    *
* again, the polygon is convex. Note Segment polygon is given as SegPl.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:           Full complete polygon.                                     *
*   SegPl:        Origin of Segment.                                         *
*   Segment:      A single linear segment as two points.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   InterSegmentStruct *:    The (upto) two intersections information.       *
*****************************************************************************/
static InterSegmentStruct *InterSegmentPoly(IPPolygonStruct *Pl,
					    IPPolygonStruct *SegPl,
					    IrtPtType Segment[2])
{
    int i, Reverse, Res,
	NumOfInter = 0;
    IrtRType TInter[3], Temp, Min, Max, *PtSeg0, *PtSeg1;
    IrtPtType Dir, Inter[2], SegDir, Pt1, Pt2;
    IPVertexStruct *VInter[2], *Vnext,
	*V = Pl -> PVertex;
    InterSegmentStruct *PSeg;

    /* Find the segment direction vector: */
    IRIT_PT_SUB(SegDir, Segment[1], Segment[0]);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter InterSegmentPoly\n");
#endif /* DEBUG */

    do {
	Vnext = V -> Pnext;
	IRIT_PT_SUB(Dir, Vnext -> Coord, V -> Coord);
	/* Find the intersection of the segment with all the polygon edges: */
	/* Note the t parameter value of the edge (Temp) must be in [0..1]. */
	if ((Res = GM2PointsFromLineLine(Segment[0], SegDir,
		V -> Coord, Dir, Pt1, &TInter[NumOfInter], Pt2, &Temp)) != 0 &&
	    (Temp > 0.0 || IRIT_BOOL_APX_EQ(Temp, 0.0)) &&
	    (Temp < 1.0 || IRIT_BOOL_APX_EQ(Temp, 1.0)) &&
	    (NumOfInter == 0 ||
	     (NumOfInter == 1 && !IRIT_BOOL_APX_EQ(TInter[0], TInter[1])))) {
	    if (IRIT_PT_PT_DIST_SQR(Pt1, Pt2) <
					IRIT_SQR(BOOL_IRIT_REL_EPS_SAME_PT)) {
		IRIT_PT_COPY(Inter[NumOfInter], Pt1);
		VInter[NumOfInter++] = V;/* Save pointer to intersected edge.*/
	    }
	}
	else {
	    /* If Res == 0 its parallel to edge. If distance is zero then    */
	    /* line is on the edge line itself - quit from this one!         */
	    if (!Res &&
		GMDistPointLine(Segment[0], V -> Coord,
				Dir) < BOOL_IRIT_REL_EPS) {
		IrtVecType Diff;
		IPVertexStruct
		    *VTmp = Vnext -> Pnext;

		IRIT_PT_SUB(Diff, VTmp -> Coord, Pt1);
		if (GlblKeepEdgeIntersection &&
		    IRIT_DOT_PROD(Diff, SegPl -> Plane) > 0.0) {
		    /* The intersection segment is on an edge of Pl.         */
		    /* Will have two such occurances as we have two polygons */
		    /* that share this edge - keep only one (that is inside).*/
		}
		else {
		    /* Wipe out adjacency of this vertex if not shared. */
		    IPVertexStruct *VTemp;

		    if (V -> PAdj == NULL)
			return NULL;

		    VTemp = V -> PAdj -> PVertex;
		    do {/* Find edge on the other polygon to wipe out first. */
			if (VTemp -> PAdj == Pl) {
			    VTemp -> PAdj = NULL;
			    break;
			}
			VTemp = VTemp -> Pnext;
		    }
		    while (VTemp != NULL && VTemp != V -> PAdj -> PVertex);
		    V -> PAdj = NULL;		/* And wipe out ours also... */
		    return NULL;
		}
	    }
	}

	V = Vnext;
    }
    while (V != NULL && V != Pl -> PVertex);

    switch (NumOfInter) {
	case 0:
	    return NULL;
	case 1:
	    /* One intersection is possible if segment intersects one vertex */
	    /* of polygon and all other vertices are on same side of segment.*/
	    return NULL;
	case 2:
	    /* In order the segment to really intersect the polygon, it must */
	    /* have at least part of t in [0..1] in the polygon. Test it:    */
	    Min = IRIT_MIN(TInter[0], TInter[1]);
	    Max = IRIT_MAX(TInter[0], TInter[1]);
	    Reverse = TInter[0] > TInter[1];
	    if (Min >= 1.0 || IRIT_BOOL_APX_EQ(Min, 1.0) ||
		Max <= 0.0 || IRIT_BOOL_APX_EQ(Max, 0.0))
		return NULL;

	    PSeg = (InterSegmentStruct *)
		IritMalloc(sizeof(InterSegmentStruct));
	    PSeg -> Pl = SegPl;     /* Pointer to other (intersect) polygon. */

	    /* Handle the Min end point: */
	    if (IRIT_BOOL_APX_EQ(Min, 0.0)) {
		PtSeg0 = Segment[0];
		PSeg -> V[0] = (Reverse ? VInter[1] : VInter[0]);
	    }
	    else if (Min < 0.0) {
		PtSeg0 = Segment[0];
		PSeg -> V[0] = NULL;			 /* End is internal. */
	    }
	    else { /* Min > 0.0 */
		PtSeg0 = (Reverse ? Inter[1] : Inter[0]);
		PSeg -> V[0] = (Reverse ? VInter[1] : VInter[0]);
	    }

	    /* Handle the Max end point: */
	    if (IRIT_BOOL_APX_EQ(Max, 1.0)) {
		PtSeg1 = Segment[1];
		PSeg -> V[1] = (Reverse ? VInter[0] : VInter[1]);
	    }
	    else if (Max > 1.0) {
		PtSeg1 = Segment[1];
		PSeg -> V[1] = NULL;			 /* End is internal. */
	    }
	    else { /* Max < 1.0 */
		PtSeg1 = (Reverse ? Inter[0] : Inter[1]);
		PSeg -> V[1] = (Reverse ? VInter[0] : VInter[1]);
	    }

	    IRIT_PT_COPY(PSeg -> PtSeg[0], PtSeg0); /* The two segs. end pt. */
            IRIT_PT_COPY(PSeg -> PtSeg[1], PtSeg1);

	    for (i = 0; i < 3; i++) {		 /* Make zeros look nicer... */
		if (IRIT_FABS(PSeg -> PtSeg[0][i]) < BOOL_IRIT_REL_EPS)
		    PSeg -> PtSeg[0][i] = 0.0;
		if (IRIT_FABS(PSeg -> PtSeg[1][i]) < BOOL_IRIT_REL_EPS)
		    PSeg -> PtSeg[1][i] = 0.0;
	    }
	    if (BOOL_IRIT_PT_APX_EQ(PSeg -> PtSeg[0], PSeg -> PtSeg[1])) {
		IritFree(PSeg);
		return NULL;
	    }
	    return PSeg;
    }
    return NULL;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a polygon with the intersection list, creates the polylines        M
* loop(s) out of it, which can be one of the two:			     M
* 1. Closed loop - all the intersections create a loop in one polygon.	     M
* 2. Open polyline - if the intersections cross the polygon boundary. In     M
*    this case the two end point of the polyline, must lay on polygon	     M
*    boundary.								     M
*									     M
* In both cases, the polyline will be as follows:			     M
* First point at first list element at PtSeg[0] (see InterSegmentStruct).    M
* Second point at first list element at PtSeg[1] (see InterSegmentStruct).   M
* Point i at list element (i-1) at PtSeg[0] (PtSeg[1] is not used!).         M
* In the closed loop case the last point is equal to first.		     M
* Both cases returns NULL terminated list.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:         Polygon with intersection information in its PAux slot.      M
*   PClosed:    To be updated with the closed loops found in Pl.             M
*   POpen:      To be updated with the open loops found in Pl.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if has loops.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolLoopsFromInterList                                                   M
*****************************************************************************/
int BoolLoopsFromInterList(IPPolygonStruct *Pl,
			   InterSegListStruct **PClosed,
			   InterSegListStruct **POpen)
{
    InterSegmentStruct *PSeg, *PSegHead, *PSegTemp, *PSegNewTemp;
    InterSegListStruct *PSLTemp;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BoolLoopsFromInterList\n");
#endif /* DEBUG */

    *PClosed = (*POpen) = NULL;

    if ((PSeg = (InterSegmentStruct *) Pl -> PAux) == NULL) {
	return FALSE;
    }
    else {
	/* Do intersect - find if there are closed loops and/or open ones:   */
	Pl -> PAux = NULL;
	while (TRUE) {
	    /* First, we look for open loops - scans linearly (maybe should  */
	    /* be done more efficiently) for segment on boundary and start   */
	    /* build chain from it (up to the other end, on the boundary).   */
	    PSegHead = PSeg;
	    while (PSegHead) {
		if (PSegHead -> V[0] != NULL || PSegHead -> V[1] != NULL) {
		    /* Found one - make it in correct order, del. from list: */
		    if (PSegHead -> V[0] == NULL)
			SwapPointInterList(PSegHead);
		    RemoveSegInterList(PSegHead, &PSeg);
		    break;
		}
		else
		    PSegHead = PSegHead -> Pnext;
	    }
	    if (PSegHead == NULL)
		break;			    /* No more open segments here... */

	    PSegTemp = PSegHead;
	    while (PSegTemp -> V[1] == NULL) {
		/* Search for matching to the second boundary end: */
		if ((PSegNewTemp = FindMatchInterList(PSegTemp -> PtSeg[1],
						      &PSeg)) == NULL)
		    break;
		PSegTemp -> Pnext = PSegNewTemp;
		PSegTemp = PSegNewTemp;
	    }
	    PSegTemp -> Pnext = NULL;
	    PSLTemp = (InterSegListStruct *)
		IritMalloc(sizeof(InterSegListStruct));
	    PSLTemp -> PISeg = PSegHead;
	    PSLTemp -> PISegMaxX = NULL;
	    PSLTemp -> Pnext = *POpen;
	    *POpen = PSLTemp;
	}

	while (TRUE) {
	    /* Now, we look for closed loops - pick one segment and search   */
	    /* for matching until you close the loop.			     */
	    /*   Note that a closed loop might be an interior intersection   */
	    /* curve that is actually open if we are searching for the       */
	    /* intersection curves only, or in the case of a CUT operation.  */
	    PSegHead = PSeg;
	    if (PSegHead == NULL)
		break;			  /* No more closed segments here... */
	    RemoveSegInterList(PSegHead, &PSeg);

	    PSegTemp = PSegHead;
	    while (!BOOL_IRIT_PT_APX_EQ(PSegTemp -> PtSeg[1],
				   PSegHead -> PtSeg[0])) {
		/* Search for matching until we back at first point: */
		if ((PSegNewTemp = FindMatchInterList(PSegTemp -> PtSeg[1],
						      &PSeg)) == NULL)
		    break;
		PSegTemp -> Pnext = PSegNewTemp;
		PSegTemp = PSegNewTemp;
	    }

#	    ifdef DEBUG
	    {
	        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBoolFoundMatch, FALSE) {
		    if (!IRIT_BOOL_PT_APX_EQ_EPS(PSegTemp -> PtSeg[1],
					    PSegHead -> PtSeg[0],
					    BOOL_IRIT_REL_EPS_SAME_PT)) {
		        /* A closed loop that is actually open!?. */
		        BOOL_FATAL_ERROR(BOOL_ERR_NO_MATCH_POINT);
		    }
		}
	    }
#	    endif /* DEBUG */

	    PSegTemp -> Pnext = NULL;
	    PSLTemp = (InterSegListStruct *)
		IritMalloc(sizeof(InterSegListStruct));
	    PSLTemp -> PISeg = PSegHead;
	    PSLTemp -> PISegMaxX = NULL;
	    PSLTemp -> Pnext = *PClosed;
	    *PClosed = PSLTemp;
	}
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Exit BoolLoopsFromInterList\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Swaps the two points in an InterSegmentStruct (modifies PtSeg & V entries) *
*                                                                            *
* PARAMETERS:                                                                *
*   PSeg:   To swap the two points in it.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SwapPointInterList(InterSegmentStruct *PSeg)
{
    IrtPtType Pt;
    IPVertexStruct *V;

    IRIT_PT_COPY(Pt,		      PSeg -> PtSeg[0]);
    IRIT_PT_COPY(PSeg -> PtSeg[0], PSeg -> PtSeg[1]);
    IRIT_PT_COPY(PSeg -> PtSeg[1], Pt);

    V		 = PSeg -> V[0];
    PSeg -> V[0] = PSeg -> V[1];
    PSeg -> V[1] = V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Removes one InterSegment PSeg, from InterSegmentList PSegList.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PSeg:         To be removed from the list (not delete itself).           *
*   PSegList:     List containing PSeg. Updated in place.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RemoveSegInterList(InterSegmentStruct *PSeg,
			       InterSegmentStruct **PSegList)
{
    InterSegmentStruct *PSegTemp;

    if (*PSegList == PSeg) { /* Its the first one - simply advance list ptr: */
	*PSegList = (*PSegList) -> Pnext;
    }
    else {
	PSegTemp = (*PSegList);
	while (PSegTemp -> Pnext != NULL && PSegTemp -> Pnext != PSeg)
	    PSegTemp = PSegTemp -> Pnext;
	if (PSegTemp -> Pnext == PSeg)
	    PSegTemp -> Pnext = PSegTemp -> Pnext -> Pnext;
	else
	    BOOL_FATAL_ERROR(BOOL_ERR_NO_ELMNT_TO_DEL);
    }
    PSeg -> Pnext = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Searches for matching point, in the given inter segment list. Returns    *
* the pointer to that element after swapping its points if needed (the match *
* must be with point 0 of new segment returned), and deletes it from list.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:          To find a match for.                                        *
*   PSegList:    To search for a match for Pt at.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   InterSegmentStruct *:  Matching edge with mtching point to Pt, if any.   *
*****************************************************************************/
static InterSegmentStruct *FindMatchInterList(IrtPtType Pt,
					      InterSegmentStruct **PSegList)
{
    IrtRType
        Eps = BOOL_IRIT_REL_EPS;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter FindMatchInterList\n");
#endif /* DEBUG */

    /* Search for a match with increasing epsilon. */
    while (Eps < BOOL_IRIT_REL_EPS_SAME_PT) {
	InterSegmentStruct
	    *PSegTemp = *PSegList;

        while (PSegTemp != NULL) {
	    if (IRIT_BOOL_PT_APX_EQ_EPS(Pt, PSegTemp -> PtSeg[0], Eps)) {
	        RemoveSegInterList(PSegTemp, PSegList);
		return PSegTemp;
	    }
	    if (IRIT_BOOL_PT_APX_EQ_EPS(Pt, PSegTemp -> PtSeg[1], Eps)) {
	        RemoveSegInterList(PSegTemp, PSegList);
		SwapPointInterList(PSegTemp);
		return PSegTemp;
	    }
	    PSegTemp = PSegTemp -> Pnext;
	}

	Eps *= 10.0;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sorts the open loops of given polygon to an order that can be used in    M
* subdividing into sub polygons later (see comment of BoolExtractPolygons).  M
*   This order is such that each loops will have no other loop between its   M
* end points, if we walk along the polygon in the (linked list direction)    M
* perimeter from one end to the other, before it. For example:		     M
*	         -----------------------------L3			     V
*		|  ---------------L1  -----L2 |          --------L4   --L5   V
*		| |               |  |     |  |         |        |   |  |    V
*	  P0 ------ P1 ------- P2 ----- P3 -------- P4 ------ P5 -------- P0 V
* In this case, any order such that L1, L2 are before L3 will do. Obviously  M
* this is not a total order, and they are few correct ways to sort it.	     M
* Algorithm:								     M
*   For each open loop, for each of its two end, evaluate a IrtRType key for M
* the end point P between segment P(i) .. P(i+1) to be i + t, where:	     M
* t is the ratio  (P - P(i)) / (P(i+1) - P(i)) . This maps the all perimeter M
* of the polygon onto 0..N-1, where N is number of vertices of that polygon. M
*   Sort the keys, and while they are keys in data structure, search and     M
* remove a consecutive pair of keys associated with same loop, and output it.M
*   Note that each open loop point sequence is tested to be such that it     M
* starts on the first point (first and second along vertex list) on polygon  M
* perimeter, and the sequence end is on the second point, and the sequence   M
* is reversed if not so. This order will make the replacement of the         M
* perimeter from first to second points by the open loop much easier.	     M
*   This may be real problem if there are two intersection points almost     M
* identical - floating point errors may cause it to loop forever. We use     M
* some reordering heuristics in this case, and return fatal error if fail!   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:       To sort the loops for.                                         M
*   POpen:    The set of open loops. Updated in place.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSortOpenInterList                                                    M
*****************************************************************************/
void BoolSortOpenInterList(IPPolygonStruct *Pl, InterSegListStruct **POpen)
{
    int Found = TRUE,
	ReOrderFirst = FALSE,
	NumOfReOrders = 0;
    IrtRType Key1, Key2;
    InterSegmentStruct *PSeg;
    InterSegListStruct *PLSeg, *PLNext,
	*PResTemp = NULL,
	*PResHead = NULL;
    SortOpenStruct *PSTemp, *PSTemp1, *PSTemp2,
	*PSHead = NULL;

#ifdef DEBUG
    BoolVerifyInterList(*POpen);

    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Enter BoolSortOpenInterList\n");
#endif /* DEBUG */

    PLSeg = (*POpen);
    while (PLSeg != NULL) {    /* Evaluate the two end keys and insert them: */
        PSeg = PLSeg -> PISeg;
	PLNext = PLSeg -> Pnext;
	PLSeg -> Pnext = NULL;

	PSTemp1 = (SortOpenStruct *) IritMalloc(sizeof(SortOpenStruct));
	PSTemp1 -> PLSeg = PLSeg;
	/* Insert PSTemp1 into PLHead list according to position of Pt in Pl.*/
	Key1 = SortOpenInsertOne(&PSHead, PSTemp1, PSeg -> PtSeg[0],
						   PSeg -> V[0], Pl);

	while (PSeg -> Pnext != NULL)
	    PSeg = PSeg -> Pnext;			    /* Go other end. */
	PSTemp2 = (SortOpenStruct *) IritMalloc(sizeof(SortOpenStruct));
	PSTemp2 -> PLSeg = PLSeg;
	/* Insert PSTemp2 into PLHead list according to position of Pt in Pl.*/
	Key2 = SortOpenInsertOne(&PSHead, PSTemp2, PSeg -> PtSeg[1],
						   PSeg -> V[1], Pl);

	if (Key1 > Key2)	      /* Reverse the open loop points order: */
	    SortOpenReverseLoop(PSTemp1);

	PLSeg = PLNext;
    }

    while (PSHead != NULL) {	/* Search for consecutive pair of same loop. */
	if (NumOfReOrders++ > 10)
	    BOOL_FATAL_ERROR(BOOL_ERR_SORT_INTER_LIST);
	if (Found)
	    NumOfReOrders = 0;

	Found = FALSE;
	PSTemp = PSHead;
	if (PSTemp -> PLSeg == PSTemp -> Pnext -> PLSeg) { /* First is pair! */
	    if (PResHead == NULL)
		PResHead = PResTemp = PSTemp -> PLSeg;
	    else {
		PResTemp -> Pnext = PSTemp -> PLSeg;
		PResTemp = PSTemp -> PLSeg;
	    }
	    PSHead = PSHead -> Pnext -> Pnext;	     /* Skip the first pair. */
	    IritFree(PSTemp -> Pnext);
	    IritFree(PSTemp);
	    Found = TRUE;
	    continue;
	}
	/* If we are here, first pair is not of same loop - search on: */
	while (PSTemp -> Pnext -> Pnext != NULL) {
	    if (PSTemp -> Pnext -> PLSeg == PSTemp -> Pnext -> Pnext -> PLSeg) {
		if (PResHead == NULL)
		    PResHead = PResTemp = PSTemp -> Pnext -> PLSeg;
		else {
		    PResTemp -> Pnext = PSTemp -> Pnext -> PLSeg;
		    PResTemp = PSTemp -> Pnext -> PLSeg;
		}
		PSTemp2 = PSTemp -> Pnext;
		PSTemp -> Pnext = PSTemp -> Pnext -> Pnext -> Pnext;
		IritFree(PSTemp2 -> Pnext);
		IritFree(PSTemp2);
		Found = TRUE;
		break;
	    }
	    PSTemp = PSTemp -> Pnext;
	}
	/* The only way we might found nothing is in floating point round */
	/* off error - two curve ends has almost the same Key...	  */
	/* Note, obviously, that there are at list 4 entries in list.     */
	if (!Found) {
	    if (!ReOrderFirst &&
		IRIT_BOOL_APX_EQ(PSHead -> Pnext -> Key, PSHead -> Key)) {
		ReOrderFirst = TRUE;
		PSTemp1 = PSHead -> Pnext;
		PSHead -> Pnext = PSTemp1 -> Pnext;
		PSTemp1 -> Pnext = PSHead;
		PSHead = PSTemp1;
		continue;
	    }
	    else
		ReOrderFirst = FALSE;
	    PSTemp = PSHead;
	    while (PSTemp -> Pnext -> Pnext != NULL) {
		if (IRIT_BOOL_APX_EQ(PSTemp -> Pnext -> Key,
				PSTemp -> Pnext -> Pnext -> Key)) {
		    PSTemp1 = PSTemp -> Pnext;
		    PSTemp2 = PSTemp1 -> Pnext;
		    PSTemp1 -> Pnext = PSTemp2 -> Pnext;
		    PSTemp2 -> Pnext = PSTemp1;
		    PSTemp -> Pnext = PSTemp2;
		    break;
		}
		PSTemp = PSTemp -> Pnext;
	    }
	}
    }

    *POpen = PResHead;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugEntryExit)
        IRIT_INFO_MSG("Exit BoolSortOpenInterList\n");
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reverses the order of the open loop pointed by PSHead.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PSHead:    Head of open loop to be reversed, in place.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SortOpenReverseLoop(SortOpenStruct *PSHead)
{
    InterSegmentStruct *PIHead, *PITemp,
	*PINewHead = NULL;

    PIHead = PSHead -> PLSeg -> PISeg;

    while (PIHead != NULL) {
	PITemp = PIHead;
	PIHead = PIHead -> Pnext;
	SwapPointInterList(PITemp);
	PITemp -> Pnext = PINewHead;
	PINewHead = PITemp;
    }

    PSHead -> PLSeg -> PISeg = PINewHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts new loop PSTemp, with defining key of Pt and V (V defines the    *
* edge and Pt is the point on it), into (decreasingly) ordered list PSHead.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PSHead:      Ordered list of loops.                                      *
*   PSTemp:      Loop to be inserted.                                        *
*   Pt, V:       Defining the order's key.                                   *
*   Pl:          The polygon that is responsible for all the mess.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    The key that the new loop was inserted according to.        *
*****************************************************************************/
static IrtRType SortOpenInsertOne(SortOpenStruct **PSHead,
				  SortOpenStruct *PSTemp,
				  IrtPtType Pt,
				  IPVertexStruct *V,
				  IPPolygonStruct *Pl)
{
    int i = 0;
    IrtRType Key;
    IrtPtType Pt1, Pt2;
    IPVertexStruct
	*VTemp = Pl -> PVertex;
    SortOpenStruct *PSTail;

    PSTemp -> Pnext = NULL;

    do {
	if (VTemp == V)
	    break;
	i++;
	VTemp = VTemp -> Pnext;
    }
    while (VTemp != V && VTemp != NULL && VTemp != Pl -> PVertex);
    if (VTemp != V)
	BOOL_FATAL_ERROR(BOOL_ERR_FIND_VERTEX_FAILED);

    IRIT_PT_SUB(Pt1, V -> Pnext -> Coord, V -> Coord);
    IRIT_PT_SUB(Pt2, Pt, V -> Coord);
    Key = PSTemp -> Key = i + sqrt(IRIT_PT_SQR_LENGTH(Pt2) / IRIT_PT_SQR_LENGTH(Pt1));

    /* Now insert PSTemp into the ordered list: */
    if (*PSHead == NULL) {
	*PSHead = PSTemp;
	return Key;
    }
    if (PSTemp -> Key > (*PSHead) -> Key) {		 /* Insert as first? */
	PSTemp -> Pnext = (*PSHead);
	*PSHead = PSTemp;
	return Key;
    }
    PSTail = (*PSHead);
    while (PSTail -> Pnext != NULL && PSTemp -> Key < PSTail -> Pnext -> Key)
	PSTail = PSTail -> Pnext;
    PSTemp -> Pnext = PSTail -> Pnext;
    PSTail -> Pnext = PSTemp;

    return Key;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given vertex list, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:       The vertex list to be printed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolPrintVrtxList(IPVertexStruct *V)
{
    IPVertexStruct
	*VHead = V;

    fprintf(stderr, "[OBJECT NONE\n    [POLYGON %d\n", IPVrtxListLen(V));
    do {
	if (IP_IS_INTERNAL_VRTX(V))
	    fprintf(stderr, "\t[[Internal] ");
	else
	    fprintf(stderr, "\t[ ");

	fprintf(stderr, "%7.4lg %7.4lg %7.4lg]\n",
		V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	V = V -> Pnext;
    }
    while (V != NULL && V != VHead);
    fprintf(stderr, "    ]\n]\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify/print content of the given InterSegment list, to standard output. *
*                                                                            *
* PARAMETERS:                                                                *
*   PIntList:       The InterSegment list to be printed.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolVerifyInterList(InterSegListStruct *PIntList)
{
    InterSegmentStruct
        *PISeg = PIntList -> PISeg;

    if (PIntList == NULL)
        return;

    /* Verify that the first and last vertices are on real edges. */
    if (PIntList -> PISeg -> V[0] == NULL) {
        BoolPrintInterList(PIntList);
        assert(0);
    }

    while (PISeg -> Pnext != NULL)
	PISeg = PISeg -> Pnext;
    if (PISeg -> V[1] == NULL) {
        BoolPrintInterList(PIntList);
        assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given InterSegment list, to standard output.    *
*                                                                            *
* PARAMETERS:                                                                *
*   PIntList:       The InterSegment list to be printed.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolPrintInterList(InterSegListStruct *PIntList)
{
    int i = 1;

    while (PIntList != NULL) {
        fprintf(stderr, "******* INTERSECTION SEGMENT %d: ********\n",
				i++);
	BoolPrintInterSegment(PIntList -> PISeg);

	PIntList = PIntList -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given InterSegment, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PInt:       The InterSegment to be printed.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolPrintInterSegment(InterSegmentStruct *PInt)
{
    fprintf(stderr, "INTER SEGMENT LIST:\n");

    if (PInt)
      fprintf(stderr, "Entry vertex pointer %08lx\n",
	      (unsigned long) PInt -> V[0]);

    while (PInt) {
        fprintf(stderr, "[OBJECT NONE\n    [POLYLINE 2\n");
	fprintf(stderr, "\t[[Ptr %08lx] %7.4lg %7.4lg %7.4lg]\n",
	       (IritIntPtrSizeType) PInt -> V[0],
	       PInt -> PtSeg[0][0], PInt -> PtSeg[0][1], PInt -> PtSeg[0][2]);
	fprintf(stderr, "\t[[Ptr %08lx] %7.4lg %7.4lg %7.4lg]\n",
	       (IritIntPtrSizeType) PInt -> V[1],
	       PInt -> PtSeg[1][0], PInt -> PtSeg[1][1], PInt -> PtSeg[1][2]);
        fprintf(stderr, "    ]\n]\n");

	if (PInt -> Pnext == NULL)
	    fprintf(stderr, "Exit vertex pointer %08lx\n",
		    (unsigned long) PInt -> V[1]);

	PInt = PInt -> Pnext;
    }
}

#endif /* DEBUG */
