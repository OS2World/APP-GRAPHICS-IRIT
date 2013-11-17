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
* Bool1Low.c - mainly handles the intersecting polyline between the oprnds.  *
* Bool2Low.c - mainly handles the polygon extraction from operands given the *
*	       polyline of intersection and the adjacencies (see ADJACNCY.C) *
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
/* If defined, return intersection POLYLINE object. */
IRIT_SET_DEBUG_PARAMETER(_DebugBoolPrint, FALSE);
/* Print messages to entry/exit from routines. */
IRIT_SET_DEBUG_PARAMETER(_DebugBoolPrint2, FALSE);
#endif /* DEBUG */

IRIT_STATIC_DATA IrtHmgnMatType GlblClosedLoopsMat;

static IPVertexStruct *InterLoopToVrtxList(InterSegmentStruct *PIHead);
static int TestAinB(IPVertexStruct *V, IPPolygonStruct *Pl, int AinB);
static IPPolygonStruct *ClipOpenLoopFromPoly(IPPolygonStruct *Pl,
					     InterSegListStruct *OLoop,
					     int AinB);
static void UpdateNewerEdge(IPVertexStruct **V,
			    IPPolygonStruct *CrntPl,
			    IrtRType *Pos);
static void ClosedLoopsToPolys(InterSegListStruct *PClosed,
			       IPPolygonStruct *Pl,
			       int AinB);
static IPVertexStruct *GenReverseVrtxList(IPVertexStruct *VIn);
static int CombineClosedLoops(IPPolygonStruct *Pl,
			      InterSegListStruct *PClosed,
			      int AinB,
			      int TopLevel);
static void PushAdjOnStack(IPPolygonStruct *Pl,
			   IPPolygonStruct *AdjStack[],
			   int *StackPointer);
static IPPolygonStruct *ChainPolyLists(IPPolygonStruct *Pl1,
				       IPPolygonStruct *Pl2);

#ifdef DEBUG
static void BoolPrintVrtxList(IPVertexStruct *V);
static void BoolPrintInterList(InterSegListStruct *PIntList);
static void BoolPrintInterSegment(InterSegmentStruct *PInt);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               *
* Test on which side of polygon Pl, given Point V is, and according to       *
* the requirement (AinB) returns TRUE/FALSE.				     *
*   If test on V fails, we tries the next one V -> Pnext until success...    *
*                                                                            *
* PARAMETERS:                                                                *
*   V:       Is V inside/outside polygon Pl?                                 *
*   Pl:      The polygon to test for inclusion.                              *
*   AinB:    Either test for inclusion (TRUE) or exclusion (FALSE).          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if V relates to PL as AinB, FALSE otherwise.               *
*****************************************************************************/
static int TestAinB(IPVertexStruct *V, IPPolygonStruct *Pl, int AinB)
{
    int In;
    IrtRType Distance;
    IPVertexStruct
	*VHead = V;

    do {
	Distance = Pl -> Plane[0] * V -> Coord[0] +
		   Pl -> Plane[1] * V -> Coord[1] +
		   Pl -> Plane[2] * V -> Coord[2] + Pl -> Plane[3];
	In = Distance > 0.0;
	V = V -> Pnext;
    }
    while (IRIT_FABS(Distance) < BOOL_IRIT_REL_EPS && V != NULL && V != VHead);

    if (IRIT_FABS(Distance) < BOOL_IRIT_REL_EPS)
	BOOL_FATAL_ERROR(BOOL_ERR_NO_COPLANAR_VRTX);

    return (In && AinB) || (!In && !AinB);   /* I wish I had logical XOR ... */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts an intersection loop into an open vertex list.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PIHead:  Intersection list to be converted.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *: The same loop as a vertex list.                        *
*                                                                            *
* KEYWORDS:                                                                  *
*   InterLoopToVrtxList                                                      *
*****************************************************************************/
static IPVertexStruct *InterLoopToVrtxList(InterSegmentStruct *PIHead)
{
    IPVertexStruct *VHead, *V;

    VHead = V = IPAllocVertex2(NULL);

    IRIT_PT_COPY(VHead -> Coord, PIHead -> PtSeg[0]);
    IRIT_VEC_RESET(VHead -> Normal);

    while (PIHead != NULL) {
	V -> Pnext = IPAllocVertex2(NULL);
	V = V -> Pnext;
	IRIT_PT_COPY(V -> Coord, PIHead -> PtSeg[1]);
	IRIT_VEC_RESET(V -> Normal);

	PIHead = PIHead -> Pnext;
    }

    V -> Pnext = NULL;

    return VHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Clip an open loop from a polygon:					     *
* 1. Clip the section (S) of the polygon between the two end points of the   *
*    loop end points and replace it by the loop itself.			     *
* 2. If the polygon formed from S and the loop should be in the output       *
*    (as tested by AinB) returns that polygon. Otherwise returns NULL.	     *
*   The open loop itself (excluding the header OLoop) is freed.		     *
* Note it is assumed (ordered by the sorting routines above) that the open   *
* loop starts from the second end back to first end:			     *
*									     *
*			       L1-----------------------L2		     *
*				|			|		     *
*				|L0			|L3		     *
*	*---------------*-------+----*-------------*----+-----------*	     *
*	P0		P1	     P2		   P3		    P0	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:        To clip against the given open loop.                          *
*   OLoop:     The given open loop (must start/end on Pl's boundary).        *
*   AinB:      TRUE for inclusion, FALSE for exclusion.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct: The newly created clipped polygon if AinB, or NULL      *
*                    otherwise.                                              *
*****************************************************************************/
static IPPolygonStruct *ClipOpenLoopFromPoly(IPPolygonStruct *Pl,
					     InterSegListStruct *OLoop,
					     int AinB)
{
    int GenClipPoly;	    /* TRUE if needs to form polygon from S & OLoop. */
    IPVertexStruct *VStart, *VEnd, *VEnd1,    /* Corresponds to L0 and L3... */
	*ClipPoly,			/* The clipped element from polygon. */
	*PLoop,				  /* The loop itself as vertex list. */
	*PLoopEnd, *PLoopEnd1, *Ptemp1, *Ptemp2,
	*PRevLoop = NULL;
    InterSegmentStruct *PISeg, *PItemp;
    IPPolygonStruct *ClipPl;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugBoolPrint2)
        IRIT_INFO_MSG("Enter ClipOpenLoopFromPoly\n");
#endif /* DEBUG */

    PISeg = OLoop -> PISeg;

    VStart = PISeg -> V[0];
    UpdateNewerEdge(&VStart, Pl, PISeg -> PtSeg[0]);

    while (PISeg -> Pnext != NULL)
        PISeg = PISeg -> Pnext;
    VEnd = PISeg -> V[1];
    UpdateNewerEdge(&VEnd, Pl, PISeg -> PtSeg[1]);
    
    if (VStart == NULL || VEnd == NULL)
	BOOL_FATAL_ERROR(BOOL_ERR_NO_OPEN_LOOP);
    VEnd1 = VEnd;		   /* Find the pointer that points on VEnd. */
    while (VEnd1 -> Pnext != VEnd)
        VEnd1 = VEnd1 -> Pnext;

    /* Make sure we never propagate through fractions of edges. */
    VStart -> PAdj = NULL;
    VEnd -> PAdj = NULL;

    PLoop = InterLoopToVrtxList(OLoop -> PISeg);/* Make vertex list out of it*/
    PLoopEnd = PLoop;			  /* Prepare pointer on last vertex. */
    while (PLoopEnd -> Pnext != NULL)
        PLoopEnd = PLoopEnd -> Pnext;
    PLoopEnd1 = PLoop;
    while (PLoopEnd1 -> Pnext != PLoopEnd)
        PLoopEnd1 = PLoopEnd1 -> Pnext;

    if (VStart != VEnd) {
	/* Now we test if we need to create a polygon formed from S & open   */
	/* loop by testing on which side of the polygon that caused	     */
	/* intersection L0L1, point P2 is, and compare with requirement AinB.*/
	GenClipPoly = TestAinB(VStart -> Pnext, OLoop -> PISeg -> Pl, AinB);

	/* Keep the clipped VertexList P2, P3 & substitute with open loop:   */
	/* Note we must keep vertex VEnd in the polygon as another InterSeg. */
	/* may point on it, so we replace InterSeg point (L3) by (P3) and    */
	/* leave VEnd intact.						     */
	Ptemp1 = VEnd -> Pnext;		     /* Save that pointer temporary. */
	VEnd -> Pnext = NULL;		   /* Close the clipped vertex list. */

	IRIT_PT_SWAP(VEnd -> Coord, PLoopEnd -> Coord);
	IRIT_PT_SWAP(VEnd -> Normal, PLoopEnd -> Normal);
	VEnd1 -> Pnext = PLoopEnd;
	PLoopEnd1 -> Pnext = VEnd;
	PLoopEnd -> Tags = VEnd -> Tags;

	Ptemp2 = VEnd;
	VEnd = PLoopEnd;
	PLoopEnd = Ptemp2;

	ClipPoly = VStart -> Pnext;

	/* New ClipPoly is isolated (Open loop of P2, P3 only). Save	     */
	/* reversed list of open loop if we need to form an S/OLoop polygon, */
	/* otherwise free ClipPoly. Chain the OLoop instead of S.	     */
	if (GenClipPoly)
	    PRevLoop = GenReverseVrtxList(PLoop);
	else
	    IPFreeVertexList(ClipPoly);
	VStart -> Pnext = PLoop;	    /* Chain the OLoop instead of S. */
	PLoopEnd -> Pnext = Ptemp1;
    }
    else { /* VStart == VEnd */
	/* Now we test if we need to create a polygon formed from S & open   */
	/* loop by testing on which side of the polygon that caused	     */
	/* intersection L0L1, point L3 is, and compare with requirement AinB.*/
	GenClipPoly = TestAinB(PLoopEnd, OLoop -> PISeg -> Pl, AinB);

	/* In this case the clipped part is empty, so its simpler: */
	ClipPoly = NULL;

	/* Save reversed list of open loop if we need to form an S/OLoop     */
	/* polygon. Chain the OLoop instead of S.			     */
	if (GenClipPoly)
	    PRevLoop = GenReverseVrtxList(PLoop);

	PLoopEnd -> Pnext = VEnd -> Pnext;
	PLoopEnd -> Tags = VEnd -> Tags;
	VStart -> Pnext = PLoop;	    /* Chain the OLoop instead of S. */
    }

    /* Time to free the InterSegment list pointed by OLoop: */
    PISeg = OLoop -> PISeg;
    while (PISeg != NULL) {
	PItemp = PISeg;
	PISeg = PISeg -> Pnext;
	IritFree(PItemp);
    }
    OLoop -> PISeg = NULL;			/* To be on the safe side... */

    /* There is a chance that Pl -> PVertex will point on vertex in clipped  */
    /* part so we update it to point on VStart, which for sure is in polygon.*/
    Pl -> PVertex = VStart;
    if (GenClipPoly) {	   /* Generate the polygon from ClipPoly & PRevLoop: */
	PLoopEnd = PRevLoop;
	while (PLoopEnd -> Pnext != NULL)
	    PLoopEnd = PLoopEnd -> Pnext;

	if (ClipPoly == NULL) {
	    PLoopEnd -> Pnext = PRevLoop;  /* Close that loop and return it. */
	    ClipPl = IPAllocPolygon((IrtBType) (IS_COPLANAR_POLY(Pl)),
				    PRevLoop, NULL);
	    IRIT_PLANE_COPY(ClipPl -> Plane, Pl -> Plane);
	    IP_RST_CONVEX_POLY(ClipPl);		   /* May be not convex now. */
	    return ClipPl;
	}

	PLoopEnd -> Pnext = ClipPoly;
	PLoopEnd -> Tags = VStart -> Tags;
	Ptemp1 = ClipPoly;
	while (Ptemp1 -> Pnext != NULL)
	    Ptemp1 = Ptemp1 -> Pnext;
	Ptemp1 -> Pnext = PRevLoop;

	ClipPl = IPAllocPolygon((IrtBType) (IS_COPLANAR_POLY(Pl)),
				ClipPoly, NULL);
	IRIT_PLANE_COPY(ClipPl -> Plane, Pl -> Plane);
	IP_RST_CONVEX_POLY(ClipPl);		   /* May be not convex now. */
	return ClipPl;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scan for the edge that hold point Pos. and update V to point to it.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:      Vertex to update so (with V -> Pnext) holds Pos.		     *
*   CrntPl: The current polygon which suppose to contain V unless trimmed.   *
*   Pos:    Position to verify to be on edge (V, V -> Pnext).		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateNewerEdge(IPVertexStruct **V,
			    IPPolygonStruct *CrntPl,
			    IrtRType *Pos)
{
    IrtVecType Dir1, Dir2;
    IrtRType *R1, *R2, Dist,
	MinDist = IRIT_INFNTY;
    IPVertexStruct
	*VTmp = CrntPl -> PVertex;;

    *V = NULL;
    do {
        R1 = VTmp -> Coord;
        R2 = VTmp -> Pnext -> Coord;

	IRIT_PT_SUB(Dir1, R2, R1);
	if ((Dist = GMDistPointLine(Pos, R1, Dir1)) < MinDist) {
	    /* Pos is close to line R3R4. */
	    IRIT_PT_SUB(Dir1, R1, Pos);
	    IRIT_PT_SUB(Dir2, R2, Pos);
	    if (IRIT_DOT_PROD(Dir1, Dir2) <= BOOL_IRIT_REL_EPS) {
	        /* Pos is in the middle between R3 and R4. */
	        *V = VTmp;
		MinDist = Dist;
	    }
	}

	VTmp = VTmp -> Pnext;
    }
    while (VTmp != CrntPl -> PVertex);

    if (*V == NULL) {
	/* Failed to find it.  This is a fatal error! */
	BOOL_FATAL_ERROR(BOOL_ERR_NO_NEWER_VERTEX);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates a new vertex list, in reverse order. The original list is not    *
* modified.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   VIn:      To create a reversed list for.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *: The newly created reversed list.                       *
*****************************************************************************/
static IPVertexStruct *GenReverseVrtxList(IPVertexStruct *VIn)
{
    IPVertexStruct *VOutHead, *VOut;

    VOutHead = IPAllocVertex2(NULL);

    IRIT_PT_COPY(VOutHead -> Coord, VIn -> Coord);
    IRIT_VEC_RESET(VOutHead -> Normal);
    IP_SET_NORMAL_VRTX(VOutHead);

    VIn = VIn -> Pnext;

    while (VIn != NULL) {
	VOut = IPAllocVertex2(NULL);
	IRIT_PT_COPY(VOut -> Coord, VIn -> Coord);
	IRIT_VEC_COPY(VOut -> Normal, VIn -> Normal);
	IP_SET_NORMAL_VRTX(VOut);

	VOut -> Pnext = VOutHead;	     /* Chain them in reverse order. */
	VOutHead = VOut;

	VIn = VIn -> Pnext;
    }

    return VOutHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the intersection of the ray fired from Pt to +X direction with the M
* given polygon. Note Pt MUST be in the polygon. Two vertices equal to	     M
* ray/polygon intersection point are added to polygon vertex list, and a     M
* pointer to the first one is also returned.				     M
*   The polygon is NOT assumed to be convex and we look for the minimum X    M
* intersection. The polygon might not be convex as a result of combining     M
* some other closed loop before we got to do this test.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:     The polygon to compute the ray intersection with.                M
*   Pt:     The origin of the ray point.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  The added vertex on Pl where the intersection with    M
*                      the ray occured.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolCutPolygonAtRay, Booleans                                            M
*****************************************************************************/
IPVertexStruct *BoolCutPolygonAtRay(IPPolygonStruct *Pl, IrtPtType Pt)
{
    int OnVertex = FALSE;
    IrtRType X,
	MinX = IRIT_INFNTY;
    IPVertexStruct *V, *Vnext,
	*VMinX = NULL;

    V = Pl -> PVertex;
    do {
	Vnext = V -> Pnext;

	/* A polygon edge might intersect the ray iff one of the following:  */
	/* 1. The first vertex is exactly on the ray Y level. (if this is    */
	/*    true for the second edge, it will be detected next iteration). */
	/* 2. The first vertex is below ray Y level, and second is above.    */
	/* 3. The first vertex is above ray Y level, and second is below.    */
	if (IRIT_BOOL_APX_EQ(V -> Coord[1], Pt[1])) {	    /* Case 1 above. */
	    if (MinX > V -> Coord[0] && Pt[0] < V -> Coord[0]) {
		OnVertex = TRUE;
		MinX = V -> Coord[0];
		VMinX = V;
	    }
	}
	else if ((V -> Coord[1] < Pt[1] &&
		  Vnext -> Coord[1] > Pt[1]) || /* Case 2. */
		 (V -> Coord[1] > Pt[1] &&
		  Vnext -> Coord[1] < Pt[1])) { /* Case 3. */
	    X = ((Vnext -> Coord[1] - Pt[1]) * V -> Coord[0] +
		 (Pt[1] - V -> Coord[1]) * Vnext -> Coord[0]) /
		(Vnext -> Coord[1] - V -> Coord[1]);
	    if (MinX > X && Pt[0] < X) {
		OnVertex = FALSE;
		MinX = X;
		VMinX = V;
	    }

	}

	V = Vnext;
    }
    while (V != NULL && V != Pl -> PVertex);

    if ((V = VMinX) == NULL)
	BOOL_FATAL_ERROR(BOOL_ERR_NO_INTERSECTION);

    /* Now that we have the intersection point - create two new vertices     */
    /* (one if OnVertex), insert them (it) after V and return the first.     */
    if (OnVertex) {
	V -> Pnext = IPAllocVertex(V -> Tags, NULL, V -> Pnext);
	IRIT_PT_COPY(V -> Pnext -> Coord, V -> Coord);
	IRIT_VEC_RESET(V -> Pnext -> Normal);
	V -> Tags = 0;
    }
    else {
	V -> Pnext = IPAllocVertex(V -> Tags, NULL, V -> Pnext);
	Vnext = V -> Pnext;
	Vnext -> Coord[0] = MinX;	 /* X - as intersection point found. */
	Vnext -> Coord[1] = Pt[1];	      /* Y - must be as ray Y level. */
	Vnext -> Coord[2] = V -> Coord[2];/* Z - all polys has same Z value. */

	V -> Pnext = IPAllocVertex2(V -> Pnext);
	V = V -> Pnext;
	IRIT_PT_COPY(V -> Coord, Vnext -> Coord);
	IRIT_VEC_RESET(V -> Normal);
    }

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the given closed loop list to polygons, and returns them. The   *
* original given polygon's vertex list is freed, and the first loop is       *
* substituted instead.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   PClosed:   To substitute into polygon Pl.                                *
*   Pl:        Polygon whose vertices are to be replaced by PClosed loops.   *
*   AinB:      Do we want inclusion or exclusion?                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ClosedLoopsToPolys(InterSegListStruct *PClosed,
			       IPPolygonStruct *Pl,
			       int AinB)
{
    int LoopNum = 0;
    IPVertexStruct *V, *VHead;
    IPPolygonStruct
	*PlCrnt = Pl;
    InterSegmentStruct *PISeg, *PItemp;
    InterSegListStruct *PClosedTemp;

    IPFreeVertexList(Pl -> PVertex);
    SET_INOUTPUT_POLY(Pl);		   /* Mark the polygon as in output. */

    while (PClosed != NULL) {
	InterSegListStruct *PClosedPrev,
	    *PInClosed = NULL;

	/* Convert the InterList to vertex list and free the first: */
	V = VHead = InterLoopToVrtxList(PClosed -> PISeg);
	if (V -> Pnext == NULL || V -> Pnext -> Pnext == NULL)
	    BOOL_FATAL_ERROR(BOOL_ERR_LOOP_LESS_3_VRTCS);

	PISeg = PClosed -> PISeg;      /* Time to free the InterSegmentList: */
	while (PISeg != NULL) {
	    PItemp = PISeg;
	    PISeg = PISeg -> Pnext;
	    IritFree(PItemp);
	}

	while (V -> Pnext -> Pnext != NULL)
	    V = V -> Pnext;				   /* Go to last pt. */
	IPFreeVertex(V -> Pnext);		/* and free - same as first. */
	V -> Pnext = VHead;		       /* Make vertex list circular. */

	if (LoopNum++) {
	    Pl -> Pnext = IPAllocPolygon((IrtBType) (IS_COPLANAR_POLY(Pl)),
					 VHead, Pl -> Pnext);
	    PlCrnt = Pl -> Pnext;
	    IRIT_PLANE_COPY(PlCrnt -> Plane, Pl -> Plane);
	    SET_INOUTPUT_POLY(PlCrnt);     /* Mark the polygon as in output. */
	}
	else {
	    Pl -> PVertex = VHead;
	}

	PClosedTemp = PClosed;
	PClosed = PClosed -> Pnext;
	IritFree(PClosedTemp);

	/* Go over the rest of the loops and find loops that are contained   */
	/* in this closed loop.  If found - embed them recursively.	     */
	PClosedPrev = NULL;
	for (PClosedTemp = PClosed; PClosedTemp != NULL; ) {
	    InterSegListStruct
		*PClosedNext = PClosedTemp -> Pnext;
	    IrtPtType PBlend;

	    IRIT_PT_BLEND(PBlend, PClosed -> PISeg -> PtSeg[0],
		             PClosed -> PISeg -> PtSeg[1], BOOL_MID_BLEND);
	    if ((GMPolygonRayInter(PlCrnt, PBlend, 0) & 0x01) == 1) {
		/* This closed loop is indeed inside the outer closed loop. */
		if (PClosedTemp == PClosed) {
		    PClosedNext = PClosed = PClosed -> Pnext;
		}
		else {
		    PClosedPrev -> Pnext = PClosedTemp -> Pnext;
		    PClosedNext = PClosedPrev -> Pnext;
		}
		IRIT_LIST_PUSH(PClosedTemp, PInClosed)
	    }
	    else
		PClosedPrev = PClosedTemp;

	    PClosedTemp = PClosedNext;
	}

	/* The recursive call... */
	if (PInClosed != NULL) {
	    IrtHmgnMatType Mat;

	    /* Save and later resture the transform. */
	    IRIT_HMGN_MAT_COPY(Mat, GlblClosedLoopsMat);
	    MatGenUnitMat(GlblClosedLoopsMat);
	    CombineClosedLoops(PlCrnt, PInClosed, AinB, FALSE);
	    IRIT_HMGN_MAT_COPY(GlblClosedLoopsMat, Mat);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This routines cuts the given polygon into its interior closed loops by   *
* adding an edge from the polygon boundary to each of its closed loops.      *
*   For example:							     *
*									     *
* +-----------------------+      +-----------------------+		     *
* |                       |	 |                       |		     *
* |  / \         / \      |	 |  / \________ / \      |		     *
* |  \ /        |   |     |	 |  \ /        |   |_____|		     *
* |      _       \ /      | -->  |      _       \ /      |		     *
* |     / \_              | -->  |     / \_              |		     *
* |    |    |             |	 |    |    |_____________|		     *
* |     \__/              |	 |     \__/              |		     *
* |                       |      |                       |		     *
* +-----------------------+      +-----------------------+		     *
*									     *
*   Algorithm:								     *
* 1. Transform the polygon and the closed loops to a plane parallel to XY    *
*    plane (Z = Const plane).						     *
* 2. For each loop find its MaxX while keeping the information on the        *
*    vertex on that extremum.						     *
* 3. For the loop with the biggest MaxX:				     *
*    3.1. Use that extremum vertex (which must be non concave corner) to     *
*         test if loop is in the reverse direction the polygon itself is,    *
*         and reverse it if not.					     *
*    3.2. Fire a ray from the extremum vertex, to the +X direction outside   *
*         of the loop till it intersect the polygon, break the polygon at    *
*         that point and add two edges to beginning of loop from breaking    *
*         point and from end of loop to breaking point (beginning/end point  *
*	  of loop is the extremum vertex point).			     *
* 4. Repeat step 3, with all loops.					     *
* 5. Transfrom the new polygon back (using the inverse matrix of step 1)     *
*    to its original orientation.					     *
*									     *
* PARAMETERS:                                                                *
*   Pl:        Polygon to comine with the given closed loops, in place.      *
*   PClosed:   Closed loops in polygon Pl.                                   *
*   AinB:      Do we want inclusion or exclusion?                            *
*   TopLevel:  If top of recursion.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE iff the original polygon boundary is in output.	     *
*****************************************************************************/
static int CombineClosedLoops(IPPolygonStruct *Pl,
			      InterSegListStruct *PClosed,
			      int AinB,
			      int TopLevel)
{
    int FirstTime;
    IrtRType MaxX, MinDistSqr, d;
    IrtPtType V1, V2, Normal, PlNormal;
    IPVertexStruct *V, *Vnext, *Vprev, *VMin, *VHead, *VMaxX, VStruct;
    InterSegListStruct *PClosedTemp, *PClosedMaxX, *PClosedLast,
	*PClosedMaxXLast = NULL;
    InterSegmentStruct *PISeg, *PItemp, *PISegMaxX;

    /* Stage 1 - Transform the vertices to a plane parallel to XY plane: */
    if (TopLevel) {
	IrtHmgnMatType RotMat;

	GMGenRotateMatrix(RotMat, Pl -> Plane);
	V = VHead = Pl -> PVertex;
	do {				    /* Transform the polygon itself. */
	    MatMultPtby4by4(V -> Coord, V -> Coord, RotMat);
	    V = V -> Pnext;
	}
	while (V != NULL && V != VHead);

	PClosedTemp = PClosed;
	while (PClosedTemp != NULL) {	    /* And transform the loops also. */
	    PISeg = PClosedTemp -> PISeg;
	    while (PISeg != NULL) {
		MatMultPtby4by4(PISeg -> PtSeg[0], PISeg -> PtSeg[0], RotMat);
		MatMultPtby4by4(PISeg -> PtSeg[1], PISeg -> PtSeg[1], RotMat);

		PISeg = PISeg -> Pnext;
	    }

	    PClosedTemp = PClosedTemp -> Pnext;
	}

	MatTranspMatrix(RotMat, GlblClosedLoopsMat); /* Compute the inverse. */
    }

    /* Evalaute the normal to the polygon (which must be convex!). Note we   */
    /* cannt simply use the Plane normal as the polygon was transformed.     */
    V = Pl -> PVertex;
    do {
	Vnext = V -> Pnext;
	IRIT_PT_SUB(V1, Vnext -> Coord, V -> Coord);
	IRIT_PT_SUB(V2, Vnext -> Pnext -> Coord, Vnext -> Coord);
	GMVecCrossProd(PlNormal, V1, V2);
	V = Vnext;
    }
    while (IRIT_PT_SQR_LENGTH(PlNormal) < IRIT_SQR(BOOL_IRIT_REL_EPS) &&
	   V != Pl -> PVertex);
    if (IRIT_PT_SQR_LENGTH(PlNormal) < IRIT_SQR(BOOL_IRIT_REL_EPS))
	return FALSE;

    PClosedTemp = PClosed;
    while (PClosedTemp != NULL) {
	/* Stage 2 - find MaxX extremum value of given loop. */
	PISegMaxX = PISeg = PClosedTemp -> PISeg;
	MaxX = PISeg -> PtSeg[0][0];		  /* Get first vertex X val. */
	PISeg = PISeg -> Pnext;
	while (PISeg)
	{
	    if (PISeg -> PtSeg[0][0] > MaxX) {
		MaxX = PISeg -> PtSeg[0][0];
		PISegMaxX = PISeg;
	    }
	    PISeg = PISeg -> Pnext;
	}
	PClosedTemp -> PISegMaxX = PISegMaxX;

	PClosedTemp = PClosedTemp -> Pnext;
    }

    /* Stage 3/4 - find the loop with biggest MaxX and combine it with the   */
    /* polygon itself. Do it for all closed loops, in list:		     */
    FirstTime = TRUE;
    while (PClosed != NULL) {
	/* Find the one with maximum MaxX, and remove it from PClosed list.  */
	PClosedLast = PClosedMaxX = PClosedTemp = PClosed;
	MaxX = PClosedMaxX -> PISegMaxX -> PtSeg[0][0];
	while (PClosedTemp != NULL)
	{
	    if (PClosedTemp -> PISegMaxX -> PtSeg[0][0] > MaxX) {
		PClosedMaxX = PClosedTemp;
		PClosedMaxXLast = PClosedLast;
	    }
	    PClosedLast = PClosedTemp;
	    PClosedTemp = PClosedTemp -> Pnext;
	}
	if (PClosedMaxX == PClosed)
	    PClosed = PClosed -> Pnext;			      /* Remove from */
	else
	    PClosedMaxXLast -> Pnext = PClosedMaxX -> Pnext; /* PClosed list.*/

	/* Create a vertex list from the loop: */
	V = VHead = InterLoopToVrtxList(PClosedMaxX -> PISeg);
	if (V -> Pnext == NULL || V -> Pnext -> Pnext == NULL)
	    BOOL_FATAL_ERROR(BOOL_ERR_LOOP_LESS_3_VRTCS);

	V = VHead;
	while (V -> Pnext -> Pnext != NULL)
	    V = V -> Pnext;				   /* Go to last pt. */
	IPFreeVertex(V -> Pnext);		/* and free - same as first. */
	V -> Pnext = VHead;		       /* Make vertex list circular. */

	PISegMaxX = PClosedMaxX -> PISegMaxX;

	/* Now test if the vertex list should be reversed. Find the vertices */
	/* which form the PISegMaxX segment, so V -> Pnext is the first      */
        /* vertex in PISegMaxX segment. Then the 3 vertices V , V -> Pnext   */
        /* (on PISegMaxX), V -> Pnext -> Pnext (on PISegMaxX), must form     */
	/* convex corner which we use to test if loop needs to be reversed:  */
	VMin = V;
	MinDistSqr = IRIT_PT_PT_DIST_SQR(VMin -> Pnext -> Coord,
				    PISegMaxX -> PtSeg[0]);
	do {
	    d = IRIT_PT_PT_DIST_SQR(V -> Pnext -> Coord,
				    PISegMaxX -> PtSeg[0]);
	    if (d < MinDistSqr) {
	        MinDistSqr = d;
		VMin = V;
	    }
    	    V = V -> Pnext;
	}
	while (V -> Pnext != VHead);
	V = VMin;
	VMaxX = V -> Pnext;

	/* Prepare in point in REAL position. */
	IRIT_PT_COPY(VStruct.Coord, V -> Coord);
	MatMultPtby4by4(VStruct.Coord, VStruct.Coord, GlblClosedLoopsMat);
	VStruct.Pnext = NULL;
	if (TestAinB(&VStruct, PISegMaxX -> Pl, AinB)) {
	    /* The Inside of the object is actually the loop itself. In that */
	    /* case we simply return all the loops converted into polygon.   */
	    /* This case is simple...					     */
	    IPFreeVertexList(VHead);		   /* Free loop vertex list. */
	    PClosedMaxX -> Pnext = PClosed;/* Put back first loop into list. */
	    PClosedTemp = PClosed = PClosedMaxX;
	    while (PClosedTemp != NULL) {	/* Transform the loops back. */
		PISeg = PClosedTemp -> PISeg;
		while (PISeg != NULL) {
		    MatMultPtby4by4(PISeg -> PtSeg[0], PISeg -> PtSeg[0],
				    GlblClosedLoopsMat);
		    MatMultPtby4by4(PISeg -> PtSeg[1], PISeg -> PtSeg[1],
				    GlblClosedLoopsMat);

		    PISeg = PISeg -> Pnext;
		}
		PClosedTemp = PClosedTemp -> Pnext;
	    }

	    if (FirstTime) {
		ClosedLoopsToPolys(PClosedMaxX, Pl, AinB);       /* To polys.*/
		IP_RST_CONVEX_POLY(Pl);   /* Polygon is not convex any more. */
		return FALSE;	   /* Boundary is NOT part of object result. */
	    }
	    else {
		IPPolygonStruct
		    *PlInterior = IPAllocPolygon(Pl -> Tags, NULL, NULL);

		IRIT_PLANE_COPY(PlInterior -> Plane, Pl -> Plane);
		ClosedLoopsToPolys(PClosedMaxX, PlInterior, AinB);
		IP_RST_CONVEX_POLY(PlInterior);    /* Polygon is not convex. */

		V = VHead = Pl -> PVertex;
		do {
		    MatMultPtby4by4(V -> Coord, V -> Coord,
				    GlblClosedLoopsMat);
		    V = V -> Pnext;
		}
		while (V != NULL && V != VHead);
		SET_INOUTPUT_POLY(Pl);         /* Mark polygon as in output. */
		IP_RST_CONVEX_POLY(Pl);            /* Polygon is not convex. */

		Pl -> Pnext = PlInterior;
		return TRUE;	       /* Boundary is part of object result. */
	    }
	}
	IRIT_PT_SUB(V1, VMaxX -> Coord, V -> Coord);
	IRIT_PT_SUB(V2, VMaxX -> Pnext -> Coord, VMaxX -> Coord);
	GMVecCrossProd(Normal, V1, V2);
	if (IRIT_DOT_PROD(Normal, PlNormal) > 0) {  /* Need to reverse list. */
	    Vprev = VHead;
	    V = Vprev -> Pnext;
	    Vnext = V -> Pnext;
	    do {
		V -> Pnext = Vprev;/* Reverse to point on prev instead next. */
		Vprev = V;
		V = Vnext;
		Vnext = V -> Pnext;
	    }
	    while (Vprev != VHead);
	}

	PISeg = PClosedMaxX -> PISeg;  /* Time to free the InterSegmentList: */
	while (PISeg != NULL) {
	    PItemp = PISeg;
	    PISeg = PISeg -> Pnext;
	    IritFree(PItemp);
	}
	IritFree(PClosedMaxX);

	/* O.k. lets fire a ray from VMaxX to +X direction and see where it  */
	/* intersects the polygon. The routine BoolCutPolygonAtRay will add  */
	/* two vertices at the ray intersection into polygon vertex list and */
	/* return a pointer to first one, so we can chain our loop directly  */
	/* between these two new vertices.				     */
	V = BoolCutPolygonAtRay(Pl, VMaxX -> Coord);
	Vnext = V -> Pnext;
	/* Introduce a copy of VMaxX and successor to VMaxX: */
	VMaxX -> Pnext = IPAllocVertex(VMaxX -> Tags, NULL, VMaxX -> Pnext);
	IRIT_PT_COPY(VMaxX -> Pnext -> Coord, VMaxX -> Coord);
	IRIT_VEC_RESET(VMaxX -> Pnext -> Normal);
	V -> Pnext = VMaxX -> Pnext;
	IP_SET_INTERNAL_VRTX(V);
	VMaxX -> Pnext = Vnext;
	IP_SET_INTERNAL_VRTX(VMaxX);

	FirstTime = FALSE;
    }

    /* Stage 5 - Time to rotate polygon back to its original position. */
    V = VHead = Pl -> PVertex;
    do {
	MatMultPtby4by4(V -> Coord, V -> Coord, GlblClosedLoopsMat);
	V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    SET_INOUTPUT_POLY(Pl);		   /* Mark the polygon as in output. */
    IP_RST_CONVEX_POLY(Pl);	     /* This polygon is not convex any more. */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Pushes on the adjacency stack, all adjacent polygons which are complete  *
* (no intersection) and are adjacent to complete edges (originated from	     *
* input polygons) of the given polygon.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:           To be added to the adjacency stack.                        *
*   AdjStack:     The adjacency stack to put Pl in.                          *
*   StackPointer: Where exactly in the stack Pl is going to be pushed?       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PushAdjOnStack(IPPolygonStruct *Pl,
			   IPPolygonStruct *AdjStack[],
			   int *StackPointer)
{
    IPVertexStruct
	*V = Pl -> PVertex;

    do {
	if (V -> PAdj != NULL)
	    AdjStack[++*StackPointer] = V -> PAdj;
	if (*StackPointer >= ADJ_STACK_SIZE) {
	    BOOL_FATAL_ERROR(BOOL_ERR_ADJ_STACK_OF);
	}
	V = V -> Pnext;
    }
    while (V != Pl -> PVertex);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Chains two polygonal lists into one. For fast processing it is prefered  *
* that the first one is shorter. Returns pointer to chained list.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1, Pl2:   The two polygonal objects to be chained together.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: The combined/chained together polygonal list.         *
*****************************************************************************/
static IPPolygonStruct *ChainPolyLists(IPPolygonStruct *Pl1,
				       IPPolygonStruct *Pl2)
{
    IPPolygonStruct *Ptemp;

    if (Pl1 == NULL)
        return Pl2;
    else if (Pl2 == NULL)
	return Pl1;
    else {
	Ptemp = Pl1;
	while (Ptemp -> Pnext != NULL)
	    Ptemp = Ptemp -> Pnext;
	Ptemp -> Pnext = Pl2;
	return Pl1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This routine coordinates all the extraction of the polygons from the     M
* intersecting lists. Does it in the following steps:			     M
* 1. Mark all polygons with no intersection at all as complete polygons.     M
*    (this is because this polygon will be totally in or out, according      M
*    to inter-polygon adjacencies propagation...)			     M
*    Also mark them as undefined (if in output or undefined) yet.	     M
*    Uses IPPolygonStruct Tags to save these bits.			     M
* 2. do									     M
*    2.1. Convert the unordered segment list of each polygon to closed loops M
*         (if create a hole in polygon) or open (if crosses its boundary).   M
*    2.2. Order the open loops along the perimeter of the polygon (note      M
*	  these loops cannt intersect. For example (5 vertices polygon):     M
*	         -----------------------------L3			     V
*		|  ---------------L1  -----L2 |          --------L4   --L5   V
*		| |               |  |     |  |         |        |   |  |    V
*	  P0 ------ P1 ------- P2 ----- P3 -------- P4 ------ P5 -------- P0 V
*         Note L1, L2 are enclosed in L3 loop, and that the order is         M
*         circular.							     M
*    2.3. "Break" the polygon at each open loop that has no enclosed loops   M
*	  in it. For example we can start at L1, L2, L4, L5 and then L3.     M
*	  "Break" means - replace the vertex chain between the two loop end  M
*	  points, with the loops itself. Depends upon the relation required  M
*	  we may need to output a new polygon form from the deleted chain    M
*	  and that loop. In addition we may form a new polygon from last     M
*	  loop and was was left from the original polygon		     M
*	  For each formed polygon, for each complete edge of it (i.e. edge   M
*	  which was originally in the polygon) test the adjacent polygon     M
*	  if it is complete (as marked in 1.) and if in or undefined (marked M
*	  undefined in 1.) is still undefined:				     M
*	  2.3.1. set it to be in.					     M
*	  2.3.2. push it on adjacency stack.				     M
*    2.4. For each closed loop - find in which polygon (from polygons	     M
*	  created in 2.3.) it is enclosed, and decompose it.		     M
* 3. While adjacencies stack not empty do:				     M
*    3.1. pop top polygon from stack and output it.			     M
*    3.2. For each of its edges (which obviousely must be complete edges)    M
*	  if adjacent polygon is complete and undefined:		     M
*	  3.3.1. set it to be in.					     M
*	  3.3.2. push it on adjacency stack.				     M
*    3.3  go back to 3.							     M
*									     M
*   The above algorithm defines in as in output, but dont be confused with   M
* the required inter-object AinB (or AoutB if FALSE), which used to	     M
* determine which side of the trimming loop should be output.		     M
*   Note this routine may return non-convex polygons (but marked as so) even M
* though the input for the booleans must be convex polygons only!	     M
*   In order to keep the given object unchanged, a whole new copy off the    M
* polygon list is made. The polygons of the list that are not in the output  M
* are freed: a global list of all polygons (pointers) is used to scan them   M
* in the end and free the unused ones (list PolysPtr).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object that need to be rebuilt according to the intersection    M
*            curves that were found, both closed and open loops.             M
*   AinB:    Type of inclusion/exclusion requested.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The newly created clipped object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolExtractPolygons, Booleans                                            M
*****************************************************************************/
IPObjectStruct *BoolExtractPolygons(IPObjectStruct *PObj, int AinB)
{
    int NumOfPolys = 0,
	StackPointer = -1;
    IPPolygonStruct **AdjStack, **PolysPtr, *OriginalPl,
		*Pl, *PlHead, *PlNext, *OutputPl = NULL, *SplPl, *NewPl;
    IPVertexStruct *V, *Vnext;
    InterSegListStruct *PClosed, *POpen, *Ptemp;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugBoolPrint2)
        IRIT_INFO_MSG("Enter BoolExtractPolygons\n");
#endif /* DEBUG */

    AdjStack = (IPPolygonStruct **) IritMalloc(sizeof(IPPolygonStruct *)
					                    * ADJ_STACK_SIZE);

    /* Stage 1. - mark all polygons as needed: */
    PlHead = Pl = PObj -> U.Pl;

    /* Gen. a copy of Pl, so we can modify the original polygon list: */
    PObj -> U.Pl = IPCopyPolygonList(Pl);

    while (Pl != NULL) {
	NumOfPolys++;	     /* Count number of polygons in original object. */
	if (Pl -> PAux != NULL) {     /* The intersection list is not empty! */
	    RST_COMPLETE_POLY(Pl);	 /* Mark it as non complete polygon. */
	    IP_RST_CONVEX_POLY(Pl);	     /* Mark it as possibly concave. */
	    V = Pl -> PVertex;
	    do {
		SET_ORIGINAL_VRTX(V); /* Mark vertices from original object. */
		V = V -> Pnext;
	    }
	    while (V != Pl -> PVertex);
	}
	else {
	    SET_COMPLETE_POLY(Pl);	     /* Mark it as complete polygon. */
	    RST_INOUTPUT_POLY(Pl);	 /* And as undefined (if in output). */
	    RST_ADJPUSHED_POLY(Pl);	 /* And as not pushed on adj. stack. */
	}
	Pl = Pl -> Pnext;
    }

    /* Stage 2. - scan the polygons and subdivide the intersecting ones: */
    Pl = PlHead;

    /* We will save pointers to ALL polygons in list so we could free in the */
    /* end the ones that are not in the output list, and therefore unused.   */
    PolysPtr = (IPPolygonStruct **)
		IritMalloc(sizeof(IPPolygonStruct *) * NumOfPolys);
    NumOfPolys = 0;


    while (Pl != NULL) {
	PolysPtr[NumOfPolys++] = Pl;	    /* Save pointer to this polygon. */

	PlNext = Pl -> Pnext;
	Pl -> Pnext = NULL;

#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugBoolPrint) {
	    IRIT_INFO_MSG("Polygon in:\n");
	    BoolPrintVrtxList(Pl -> PVertex);
	    IRIT_INFO_MSG("Polygon inters:\n");
	    BoolPrintInterSegment((InterSegmentStruct *) Pl -> PAux);
	}
#	endif /* DEBUG */

	if (!IS_COMPLETE_POLY(Pl) &&
	    BoolLoopsFromInterList(Pl, &PClosed, &POpen)) {
	    /* They are intersections with this polygon - convert the        */
	    /* intersecting segments into open/closed loops.                 */

	    /* Save copy of original polygon vertex list as we need its      */
	    /* normals to interpolate for the internal ones.		     */
	    OriginalPl = IPAllocPolygon(Pl -> Tags,
					IPCopyVertexList(Pl -> PVertex), NULL);
	    IP_RST_BBOX_POLY(OriginalPl);
	    IRIT_PLANE_COPY(OriginalPl -> Plane, Pl -> Plane);
	    IP_SET_PLANE_POLY(OriginalPl);
	    OriginalPl -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr);

	    if (POpen != NULL) {
		/* Sort the Open loops into an order we can handle... */
		BoolSortOpenInterList(Pl, &POpen);
		SplPl = NewPl = NULL;	 /* Keep the list of split polygons. */

		while (POpen != NULL) {	/* Clip the open loops from polygon: */
		    /* Note ClipOpenLoopFromPoly also frees the InterSegment */
		    /* list pointed by POpen (but not POpen itself). 	     */
		    NewPl = ClipOpenLoopFromPoly(Pl, POpen, AinB);

		    if (NewPl != NULL) {   /* If loop clipped a new polygon, */
			IRIT_PLANE_COPY(NewPl -> Plane, OriginalPl -> Plane);
			IP_SET_PLANE_POLY(NewPl);

			NewPl -> Pnext = SplPl;/* Add to split polygon list. */
			SplPl = NewPl;
			/* And push adj. polygons of complete edges on stack.*/
			PushAdjOnStack(NewPl, AdjStack, &StackPointer);
		    }
		    Ptemp = POpen;
		    POpen = POpen -> Pnext;
		    IritFree(Ptemp);
		}

		/* Last clip generated nothing (NewPl == NULL) so part that  */
		/* left from Pl (PlCopy) is IN output list! Add this poly:   */
		if (NewPl == NULL) {
		    IRIT_PLANE_COPY(Pl -> Plane, OriginalPl -> Plane);
		    IP_SET_PLANE_POLY(Pl);

		    Pl -> Pnext = SplPl; /* And chain what was left from it. */
		    SplPl = Pl;
		    /* And push adjacent polygons of complete edges on stack.*/
		    PushAdjOnStack(Pl, AdjStack, &StackPointer);
		    SET_INOUTPUT_POLY(Pl);/* So we wouldnt free that in end. */
		    IP_RST_CONVEX_POLY(Pl);	   /* May be not convex now. */
		}

		GMUpdateVerticesByInterp(SplPl, OriginalPl);
	    }
	    else
		SplPl = Pl;

	    if (PClosed != NULL) {
		IPPolygonStruct
		    *SplPlInterior = NULL;

		for (Pl = SplPl; Pl != NULL; Pl = Pl -> Pnext)
		    Pl -> PAux = NULL;

		/* Classify the closed loops into the appropriate polygon. */
		while (PClosed != NULL) {
		    Ptemp = PClosed -> Pnext;
		    PClosed -> Pnext = NULL;

		    for (Pl = SplPl; Pl != NULL; Pl = Pl -> Pnext) {
			IrtPtType PBlend;

			IRIT_PT_BLEND(PBlend, PClosed -> PISeg -> PtSeg[0],
				 PClosed -> PISeg -> PtSeg[1], BOOL_MID_BLEND);
			if (GMPolygonRayInter3D(Pl, PBlend, 0) % 2 == 1) {
			    /* This closed loop is contained in this polygon.*/
			    PClosed -> Pnext =
			        (InterSegListStruct *) Pl -> PAux;
			    Pl -> PAux = (VoidPtr) PClosed;
			    break;
			}
		    }
		    if (Pl == NULL) {
			/* This closed loop is part of a trimmed out by open */
			/* loop region. It should be only the island formed  */
			/* by this closed loop then.			     */
			/* Make a new polygon - a copy of the original and   */
			/* Put this loop in it.				     */
			NewPl = IPAllocPolygon(OriginalPl -> Tags,
			       IPCopyVertexList(OriginalPl -> PVertex), NULL);
			IP_RST_BBOX_POLY(NewPl);
			IRIT_PLANE_COPY(NewPl -> Plane, OriginalPl -> Plane);
			IP_SET_PLANE_POLY(NewPl);

			NewPl -> PAux = (VoidPtr) PClosed;
			NewPl -> Pnext = SplPl;
			SplPl = NewPl;
		    }
		    PClosed = Ptemp;
		}

		for (Pl = SplPl; Pl != NULL; Pl = Pl -> Pnext) {
		    /* Make a "cut" from the loop(s)  +-------+    +-------+ */
		    /* to boundary if possible, and   |       |    |       | */
		    /* converting Pl to a non convex  |  / \  | -> |  / \__| */
		    /* polygon, that has an edge (the |  \ /  | -> |  \ /  | */
		    /* cut) which is shared twice in  |       |    |       | */
		    /* the same polygon		      +-------+    +-------+ */
		    PClosed = (InterSegListStruct *) Pl -> PAux;
		    Pl -> PAux = NULL;
		    if (CombineClosedLoops(Pl, PClosed, AinB, TRUE)) {
			/* If returned with TRUE -  polygon boundary is in   */
			/* output, so add all its neighbors to adj. stack.   */
			PushAdjOnStack(Pl, AdjStack, &StackPointer);
		    }

		    GMUpdateVerticesByInterp(Pl, OriginalPl);
		}
		if (SplPlInterior != NULL)
		    SplPl = ChainPolyLists(SplPl, SplPlInterior);

	    }

	    OutputPl = ChainPolyLists(SplPl, OutputPl);

	    /* Free the original polygon vertex list. */
	    IPFreePolygon(OriginalPl);
	}

	Pl = PlNext;
    }

    /* Stage 3. - handling adjacencies and propagate them in polygons:	     */
    /* Pop off the elements from the stack, and propagate them using their   */
    /* adjacencies.							     */
    while (StackPointer >= 0) {
	Pl = AdjStack[StackPointer--];			 /* Pop top element. */
	if (!IS_COMPLETE_POLY(Pl) ||	    /* Ignore non complete polygons. */
	     IS_INOUTPUT_POLY(Pl))
	    continue;				      /* If already handled. */

	SET_INOUTPUT_POLY(Pl);	  /* Mark this one as handled for next time. */

	V = Pl -> PVertex;   /* Push all adjacent ones that not handled yet. */
	do {
	    if (V -> PAdj &&
		IS_COMPLETE_POLY(V -> PAdj) &&
		!IS_INOUTPUT_POLY(V -> PAdj) &&
		!IS_ADJPUSHED_POLY(V -> PAdj)) {
		SET_ADJPUSHED_POLY(V -> PAdj);
		AdjStack[++StackPointer] = V -> PAdj;   /* Push it on stack. */
		if (StackPointer >= ADJ_STACK_SIZE)
		    BOOL_FATAL_ERROR(BOOL_ERR_ADJ_STACK_OF);
	    }
	    V = V -> Pnext;
	}
	while (V != Pl -> PVertex);

	Pl -> Pnext = OutputPl;		   /* And chain it into output list. */
	OutputPl = Pl;
    }

    /* Free all polygons which are not in the output list: */
    while (--NumOfPolys >= 0) {
	if (!IS_INOUTPUT_POLY(PolysPtr[NumOfPolys])) {
	    IPFreePolygon(PolysPtr[NumOfPolys]);
	}
    }
    IritFree(PolysPtr);

    /* Another floating point kludge: a polygon may have zero length edge so */
    /* search for those and remove them - someone may die because of one...  */
    Pl = OutputPl;
    while (Pl != NULL) {
	V = Pl -> PVertex;
	do {
	    Vnext = V -> Pnext;
	    if (BOOL_IRIT_PT_APX_EQ(V -> Coord, Vnext -> Coord)) {
		/* Ahh - we got you. Simply skip Vnext vertex and free it: */
		V -> Pnext = Vnext -> Pnext;
		/* Update polygon vertex pointer if point on freed vertex: */
		if (Pl -> PVertex == Vnext)
		    Pl -> PVertex = Vnext -> Pnext;
		IPFreeVertex(Vnext);
		Vnext = V -> Pnext;
	    }
	    V = Vnext;
	}
	while (V != Pl -> PVertex && V != NULL);

	Pl = Pl -> Pnext;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugBoolPrint2)
        IRIT_INFO_MSG("Exit BoolExtractPolygons\n");
#endif /* DEBUG */

    IritFree(AdjStack);

    return IPGenPolyObject("", OutputPl, NULL);	 /* Return resulting object. */
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
