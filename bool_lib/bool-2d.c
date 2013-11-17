/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle Boolean operation between two polygons in the XY plane. *
* The Z coords. are totally ignored. Input polygons are assumed to be convex.*
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "allocate.h"
#include "bool_loc.h"
#include "geom_lib.h"

static IPPolygonStruct *MergeTwoPolygons(IPPolygonStruct *Pl1,
					 IPPolygonStruct *Pl2);
static IPPolygonStruct *Boolean2DCombine(IPPolygonStruct *Pl1,
					 IPPolygonStruct *Pl2,
					 IPPolygonStruct *Pl3);
static int IsInterValid(IrtRType t1,
			IrtRType t2,
			IPVertexStruct *V1,
			IPVertexStruct *V1Prev,
			IPVertexStruct *V2,
			IPVertexStruct *V2Prev);
static void SortParam(Bool2DInterStruct **Bool2D, int First);
static int Boolean2DComputeSegments(IPPolygonStruct *Pl1,
				    IPPolygonStruct *Pl2,
				    Bool2DInterStruct **Bool2D,
				    int Pl1First,
				    IPPolygonStruct **Pl1InPl2,
				    IPPolygonStruct **Pl1OutPl2,
				    IPPolygonStruct **Pl1SharedPl2,
				    IPPolygonStruct **Pl1AntiSharedPl2);

#ifdef DEBUG
static void PrintVrtxList(IPVertexStruct *V);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two convex polygons assumed to be in the same plane, compute their M
* 2D Boolean operation BoolOper and return it as a new polygon(s).	     M
* NULL is returned if an error occur (No intersection or invalid BoolOper).  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1:        First convex polygon to compute 2D Boolean for.              M
*   Pl2:        Second convex polygon to compute 2D Boolean for.             M
*   BoolOper:   Boolean operation requested (and, or, etc.)                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The resulting Boolean operation.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, M
*   BoolSetHandleCoplanarPoly, BoolSetOutputInterCurve,			     M
*   Boolean2DComputeInters						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Boolean2D, Booleans                                                      M
*****************************************************************************/
IPPolygonStruct *Boolean2D(IPPolygonStruct *Pl1,
			   IPPolygonStruct *Pl2,
			   BoolOperType BoolOper)
{
    IPPolygonStruct *Pls, *Pl, *Pl1InPl2, *Pl1OutPl2, *Pl1SharedPl2,
        *Pl1AntiSharedPl2, *Pl2InPl1, *Pl2OutPl1,
	*RetVal = NULL;
    Bool2DInterStruct *Bool2D;

    Bool2D = Boolean2DComputeInters(Pl1, Pl2, TRUE, FALSE);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolysInters, FALSE) {
	    Bool2DInterStruct *B;

	    IRIT_INFO_MSG("***************** Poly1:\n");
	    PrintVrtxList(Pl1 -> PVertex);
	    IRIT_INFO_MSG("***************** Poly2:\n");
	    PrintVrtxList(Pl2 -> PVertex);

	    for (B = Bool2D; B != NULL; B = B -> Pnext) {
	        IRIT_INFO_MSG_PRINTF("Inter: (%10lg  %10lg) Param1/2 = %10lg, %10lg (%d)\n",
		       B -> InterPt[0], B -> InterPt[1],
		       B -> Param1, B -> Param2, B -> DualInter);
	        IRIT_INFO_MSG_PRINTF("\t Poly1/2Vrtx:  (%10lg  %10lg)  (%10lg  %10lg)\n",
		       B -> Poly1Vrtx -> Coord[0],
		       B -> Poly1Vrtx -> Coord[1],
		       B -> Poly2Vrtx -> Coord[0],
		       B -> Poly2Vrtx -> Coord[1]);
		if (B -> DualInter) {
		    IRIT_INFO_MSG_PRINTF("\t Poly1/2Vrtx2:  (%10lg  %10lg)  (%10lg  %10lg)\n",
			   B -> Poly1Vrtx2 -> Coord[0],
			   B -> Poly1Vrtx2 -> Coord[1],
			   B -> Poly2Vrtx2 -> Coord[0],
			   B -> Poly2Vrtx2 -> Coord[1]);
		}
	    }
	}

    }
#endif /* DEBUG */

    if (Bool2D == NULL) {
	IPPolygonStruct *PlOut, *PlIn;
	IrtPtType Mid1, Mid2;

	IRIT_PT_BLEND(Mid1, Pl1 -> PVertex -> Coord,
		       Pl1 -> PVertex -> Pnext -> Coord, BOOL_MID_BLEND);
	IRIT_PT_BLEND(Mid2, Pl2 -> PVertex -> Coord,
		       Pl2 -> PVertex -> Pnext -> Coord, BOOL_MID_BLEND);

	/* Coplanar polygons have no intersection. Test for inclusion. */
	if (GMPointInsideCnvxPolygon(Mid2, Pl1)) {
	    /* Pl2 is enclosed within Pl1. */
	    PlOut = Pl1;
	    PlIn = Pl2;
	}
	else if (GMPointInsideCnvxPolygon(Mid1, Pl2)) {
	    /* Pl1 is enclosed within Pl2. */
	    PlOut = Pl2;
	    PlIn = Pl1;
	}
	else {
	    PlOut = NULL;
	    PlIn = NULL;
	}

	Pls = NULL;
	switch (BoolOper) {
	    case BOOL_OPER_OR:
	        if (PlOut != NULL) {
		    Pls = IPAllocPolygon(PlOut -> Tags,
				     IPCopyVertexList(PlOut -> PVertex), NULL);
		    IRIT_PLANE_COPY(Pls -> Plane, PlOut -> Plane);
		    IP_SET_PLANE_POLY(Pls);

		}
		else if (PlOut == NULL) {
		    /* Disjoint polygons - simply link list them. */
		    Pls = IPAllocPolygon(Pl1 -> Tags,
					 IPCopyVertexList(Pl1 -> PVertex),
			      IPAllocPolygon(Pl2 -> Tags,
				     IPCopyVertexList(Pl2 -> PVertex), NULL));
		    IRIT_PLANE_COPY(Pls -> Plane, Pl1 -> Plane);
		    IRIT_PLANE_COPY(Pls -> Pnext -> Plane, Pl1 -> Plane);
		    IP_SET_PLANE_POLY(Pls);
		    IP_SET_PLANE_POLY(Pls -> Pnext);
		}
		break;
	    case BOOL_OPER_AND:
	        if (PlIn != NULL) {
		    Pls = IPAllocPolygon(PlIn -> Tags,
				      IPCopyVertexList(PlIn -> PVertex), NULL);
		    IRIT_PLANE_COPY(Pls -> Plane, PlIn -> Plane);
		    IP_SET_PLANE_POLY(Pls);
		}
		break;
	    case BOOL_OPER_SUB:
	        if (PlOut == Pl1) {
		    Pls = MergeTwoPolygons(
			IPAllocPolygon(PlOut -> Tags,
				     IPCopyVertexList(PlOut -> PVertex), NULL),
			IPAllocPolygon(PlIn -> Tags,
			    IPReverseVrtxList2(IPCopyVertexList(PlIn -> PVertex)),
				       NULL));
		    IRIT_PLANE_COPY(Pls -> Plane, PlOut -> Plane);
		    IP_SET_PLANE_POLY(Pls);
		}
		else if (PlOut == NULL) {
		    Pls = IPAllocPolygon(Pl1 -> Tags,
				       IPCopyVertexList(Pl1 -> PVertex), NULL);
		    IRIT_PLANE_COPY(Pls -> Plane, Pl1 -> Plane);
		    IP_SET_PLANE_POLY(Pls);
		}
		break;
	    default:
		BOOL_FATAL_ERROR(BOOL_ERR_NO_2D_OP_SUPPORT);
		break;
	}

	return Pls;
    }

    switch (BoolOper) {
	case BOOL_OPER_OR:
	    Boolean2DComputeSegments(Pl1, Pl2, &Bool2D, TRUE,
				     NULL, &Pl1OutPl2, &Pl1SharedPl2, NULL);
	    Boolean2DComputeSegments(Pl2, Pl1, &Bool2D, FALSE,
				     NULL, &Pl2OutPl1, NULL, NULL);
	    Pls = Boolean2DCombine(Pl1OutPl2, Pl2OutPl1, Pl1SharedPl2);
	    break;
	case BOOL_OPER_AND:
	    Boolean2DComputeSegments(Pl1, Pl2, &Bool2D, TRUE,
				     &Pl1InPl2, NULL, &Pl1SharedPl2, NULL);
	    Boolean2DComputeSegments(Pl2, Pl1, &Bool2D, FALSE,
				     &Pl2InPl1, NULL, NULL, NULL);
	    Pls = Boolean2DCombine(Pl1InPl2, Pl2InPl1, Pl1SharedPl2);
	    break;
	case BOOL_OPER_SUB:
	    Boolean2DComputeSegments(Pl1, Pl2, &Bool2D, TRUE,
				     NULL, &Pl1OutPl2, NULL, &Pl1AntiSharedPl2);
	    Boolean2DComputeSegments(Pl2, Pl1, &Bool2D, FALSE,
				     &Pl2InPl1, NULL, NULL, NULL);
	    for (Pl = Pl2InPl1; Pl != NULL; Pl = Pl -> Pnext)
	        Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);
	    Pls = Boolean2DCombine(Pl1OutPl2, Pl2InPl1, Pl1AntiSharedPl2);
	    break;
	default:
	    BOOL_FATAL_ERROR(BOOL_ERR_NO_2D_OP_SUPPORT);
	    Pls = NULL;
	    break;
    }

    while (Bool2D) {
	Bool2DInterStruct
	    *NextBool2D = Bool2D -> Pnext;

	IritFree(Bool2D);
	Bool2D = NextBool2D;
    }

    while (Pls != NULL) {
	Pl = Pls;
	Pls = Pls -> Pnext;

	BoolFilterCollinearities(Pl);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolylines, FALSE) {
	        IRIT_INFO_MSG("Returned Polygon:\n");
		if (Pl -> PVertex)
		    PrintVrtxList(Pl->PVertex);
		else
		    IRIT_INFO_MSG("\tEmpty Polygon:\n");
	    }
	}
#	endif /* DEBUG */

	if (Pl -> PVertex == NULL) {
	    IPFreePolygon(Pl);
	}
	else
	    IRIT_LIST_PUSH(Pl, RetVal);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Filters out collinear edges and duplicated vertices, in the given	     M
* polygon.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:     To filter, in place. The polygon is assumed to have a circular   M
*	    vertex list.	                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if the polygon has been modified, FALSE otherwise.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolFilterCollinearities                                                 M
*****************************************************************************/
int BoolFilterCollinearities(IPPolygonStruct *Pl)
{
    int NumOfVertices = 0,
        VerticesRemoved = 0,
	Count = 0,
	AnyChange = FALSE;
    IPVertexStruct
	*VHead = Pl -> PVertex,
	*V = VHead;

    /* Count how many vertices we have. */
    do {
	NumOfVertices++;
	V = V -> Pnext;
    }
    while (V != VHead && V != NULL);

    /* Loop on the vertex list until we iterate NumOfVertices times without  */
    /* any vertex removal.						     */
    V = VHead;
    while (Count <= NumOfVertices) {
	IrtRType Size1, Size2;
	IrtVecType V1, V2;
	IPVertexStruct *VTmp,
	    *VNext = V -> Pnext;

	if (NumOfVertices - VerticesRemoved < 3) {
	    /* This whole polygon is zero area - purge it completely. */
	    IPFreeVertexList(Pl -> PVertex);
	    Pl -> PVertex = NULL;
	    return TRUE;
	}

	IRIT_PT_SUB(V1, V -> Coord, VNext -> Coord);
	if ((Size1 = IRIT_PT_LENGTH(V1)) < IRIT_EPS) {
	    /* V and VNext are identical vertices - purge VNext.             */
	    V -> Pnext = VNext -> Pnext;
	    IPFreeVertex(VNext);
	    AnyChange = TRUE;
	    VerticesRemoved++;
	    Count = 0;
	}
	else {
	    IRIT_PT_SUB(V2, VNext -> Coord, VNext -> Pnext -> Coord);
	    if ((Size2 = IRIT_PT_LENGTH(V2)) < IRIT_EPS) {
		/* VNext and VNext->Pnext are identical vertices.	     */
		/* Purge VNext->Pnext.					     */
		VTmp = VNext -> Pnext;
		VNext -> Pnext = VTmp -> Pnext;
		IPFreeVertex(VTmp);
		AnyChange = TRUE;
		VerticesRemoved++;
		Count = 0;
	    }
	    else {
		Size1 = 1.0 / Size1;
		Size2 = 1.0 / Size2;
		IRIT_PT_SCALE(V1, Size1);
		IRIT_PT_SCALE(V2, Size2);

		if (IRIT_FABS(IRIT_CROSS_PROD_2D(V1, V2)) < IRIT_EPS) {
		    /* V, VNext and VNext->Pnext are all collinear.          */
		    /* Purge VNext.					     */
		    V -> Pnext = VNext -> Pnext;
		    IPFreeVertex(VNext);
		    AnyChange = TRUE;
		    VerticesRemoved++;
		    Count = 0;
		}
		else {
		    V = VNext;
		    Count++;
		}
	    }
	}	
    }

    Pl -> PVertex = V;            /* Note we might have purged P -> PVertex! */

    return AnyChange;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges the two provided polygons. Pl1 is assumed to fully contain Pl2.   *
*   Pl1/2 are assumed to be convex. Pl2 vertex list is reversed and the two  *
* polygon's vertex lists are connected via the maximum X vertices.	     *
*   This function is destructive and Pl1/2 are modified in place.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1, Pl2:    The two polygons to merge into one.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct:  The merged polygon.                                    *
*****************************************************************************/
static IPPolygonStruct *MergeTwoPolygons(IPPolygonStruct *Pl1,
					 IPPolygonStruct *Pl2)
{
    IrtRType MaxX;
    IPVertexStruct *V, *Vnext,
	*VMaxX = NULL;

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolylines, FALSE) {
	    IRIT_INFO_MSG("MERGING:\nPoly1:\n");
	    PrintVrtxList(Pl1 -> PVertex);
	    IRIT_INFO_MSG("Poly2:\n");
	    PrintVrtxList(Pl2 -> PVertex);
	}
    }
#endif /* DEBUG */

    IPUpdatePolyPlane(Pl1); /* Make sure polygon has plane definition. */

    /* Find the vertex in Pl2 with the maximum X value. */
    V = Pl2 -> PVertex;
    MaxX = -IRIT_INFNTY;
    do {
	if (V -> Coord[0] > MaxX) {
	    VMaxX = V;
	    MaxX = V -> Coord[0];
	}
	V = V -> Pnext;
    }
    while (V != NULL && V != Pl2 -> PVertex);

    V = BoolCutPolygonAtRay(Pl1, VMaxX -> Coord);
    IRIT_PT_COPY(V -> Normal, Pl1 -> Plane);
    Vnext = V -> Pnext;
    IRIT_PT_COPY(Vnext -> Normal, Pl1 -> Plane);

    /* Duplicate VMaxX vertex. */
    VMaxX -> Pnext = IPAllocVertex(VMaxX -> Tags, NULL, VMaxX -> Pnext);
    IRIT_PT_COPY(VMaxX -> Pnext -> Coord, VMaxX -> Coord);
    IRIT_PT_COPY(VMaxX -> Pnext -> Normal, VMaxX -> Normal);

    /* And exchange pointers. */
    V -> Pnext = VMaxX -> Pnext;
    IP_SET_INTERNAL_VRTX(V);
    VMaxX -> Pnext = Vnext;
    IP_SET_INTERNAL_VRTX(VMaxX);

    Pl2 -> PVertex = NULL;
    IPFreePolygon(Pl2);

    IP_RST_CONVEX_POLY(Pl1);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolylines, FALSE) {
	    IRIT_INFO_MSG("RESULT OF MERGING:\n");
	    PrintVrtxList(Pl1->PVertex);
	}
    }
#endif /* DEBUG */

    return Pl1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Connects the provided list of polylines into a closed polygon.	     *
* Pl1/2/3 are being used by this routine and being destroyed.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1, Pl2, Pl3:  The lists of polylines to merge into one closed polygon. *
*		    Any one of these lists of polylines may be NULL.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct:  The constructed polygon.                               *
*****************************************************************************/
static IPPolygonStruct *Boolean2DCombine(IPPolygonStruct *Pl1,
					 IPPolygonStruct *Pl2,
					 IPPolygonStruct *Pl3)
{
    IPVertexStruct *VTail;
    IPPolygonStruct *Pl, *PlLast,
	*PlOut = NULL;

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolylineLists, FALSE) {
	    for (Pl = Pl1; Pl != NULL; Pl = Pl -> Pnext) {
	        IRIT_INFO_MSG("**** Polyline1 ****\n");
	        PrintVrtxList(Pl -> PVertex);
	    }
	    for (Pl = Pl2; Pl != NULL; Pl = Pl -> Pnext) {
	        IRIT_INFO_MSG("**** Polyline2 ****\n");
	        PrintVrtxList(Pl -> PVertex);
	    }
	    for (Pl = Pl3; Pl != NULL; Pl = Pl -> Pnext) {
	        IRIT_INFO_MSG("**** Polyline3 ****\n");
	        PrintVrtxList(Pl -> PVertex);
	    }
	}
    }
#endif /* DEBUG */

    /* Chain the lists into one: */
    if (Pl1 == NULL)
        IRIT_SWAP(IPPolygonStruct *, Pl1, Pl3);
    if ((Pl1 = IPAppendPolyLists(Pl1, Pl2)) == NULL)
	return NULL;
    Pl1 = IPAppendPolyLists(Pl1, Pl3);

    VTail = IPGetLastVrtx(Pl1 -> PVertex);
    while (Pl1 != NULL) {
	for (PlLast = Pl1, Pl = Pl1 -> Pnext;
	     Pl != NULL &&
	     !BOOL_IRIT_PT_APX_EQ(Pl -> PVertex -> Coord, VTail -> Coord);
	     PlLast = Pl, Pl = Pl -> Pnext);
	if (Pl == NULL) {
	    IPVertexStruct
	        *V = Pl1 -> PVertex;

	    /* If Pl1 is a zero length list of vertices, we better ignore. */
	    for ( ; V -> Pnext != NULL; V = V -> Pnext) {
		if (!BOOL_IRIT_PT_APX_EQ(V -> Coord, V -> Pnext -> Coord))
		    break;
	    }
	    if (V -> Pnext == NULL) {
		Pl = Pl1 -> Pnext;
		IPFreePolygon(Pl1);
		Pl1 = Pl;
		continue;
	    }
	    else {
		BOOL_FATAL_ERROR(BOOL_ERR_NO_PLLN_MATCH);
		return NULL;
	    }
	}

	VTail -> Pnext = Pl -> PVertex -> Pnext;

	/* Free the merged polyline (with its first vertex). */
	PlLast -> Pnext = Pl -> Pnext;
	Pl -> PVertex -> Pnext = NULL;
	IPFreePolygon(Pl);

	/* Update the Tail pointer. */
	VTail = IPGetLastVrtx(VTail);

	if (BOOL_IRIT_PT_APX_EQ(VTail -> Coord, Pl1 -> PVertex -> Coord)) {
	    /* We closed a loop here. Add to output list. */
	    Pl = Pl1 -> Pnext;
	    Pl1 -> Pnext = PlOut;
	    PlOut = Pl1;

	    /* Close the loop and remove the duplicate vertex. */
	    VTail -> Pnext = Pl1 -> PVertex -> Pnext;
	    IPFreeVertex(Pl1 -> PVertex);
	    Pl1 -> PVertex = VTail;

	    /* Continue with next polygon. */
	    if ((Pl1 = Pl) != NULL) {
		for (VTail = Pl1 -> PVertex;
		     VTail -> Pnext != NULL;
		     VTail = VTail -> Pnext);
	    }
	}
    }

    return PlOut;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two polygons/lines, Detect all edges in Pl1 that intersect with    M
* edges in Pl2. Returned is the information about all intersections as a     M
* Bool2DInter structure list.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly1, Poly2:    The two polygons/lines to intersect.                    M
*   HandlePolygons:  If polygons, needs to handle normals etc.               M
*   DetectIntr:	     If TRUE, return non NULL dummy pointer if the two polys M
*		     do indeed intersect.  For detection of intersection!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   Bool2DInterStruct *:  Intersection information.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   Boolean2D                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   Boolean2DComputeInters, Booleans                                         M
*****************************************************************************/
Bool2DInterStruct *Boolean2DComputeInters(IPPolygonStruct *Poly1,
					  IPPolygonStruct *Poly2,
					  int HandlePolygons,
					  int DetectIntr)
{
    IrtRType Pl1Param, Pl2Param, *Pl1, *Pl2, t1, t2;
    IrtPtType Pt1, Pt2;
    IrtVecType Vl1, Vl2;
    Bool2DInterStruct *Bool2D, *Bool2DTmp,
	*Bool2DHead = NULL;
    IPVertexStruct *V1, *V2, *V1Prev, *V2Prev,
	*V1Head = Poly1 -> PVertex,
	*V2Head = Poly2 -> PVertex;

    /* If this poly list is not circular, no previous vertex to first one. */
    V1 = V1Head;
    V1Prev = IPGetLastVrtx(V1);
    if (V1Prev -> Pnext != V1)
	V1Prev = NULL;
    Pl1Param = 0.0;
    do {
	if (V1 == NULL || V1 -> Pnext == NULL)
	    break;
	Pl1 = V1 -> Coord;
	IRIT_PT_SUB(Vl1, V1 -> Pnext -> Coord, Pl1);

	/* If poly list is not circular, no previous vertex to first one. */
	V2 = V2Head;
	V2Prev = IPGetLastVrtx(V2);
	if (V2Prev -> Pnext != V2)
	    V2Prev = NULL;
        Pl2Param = 0.0;
	do {
	    if (V2 == NULL || V2 -> Pnext == NULL)
	        break;
	    Pl2 = V2 -> Coord;
	    IRIT_PT_SUB(Vl2, V2 -> Pnext -> Coord, Pl2);

	    if (GM2PointsFromLineLine(Pl1, Vl1, Pl2, Vl2, Pt1, &t1, Pt2, &t2) &&
		t1 > -IRIT_UEPS && t1 < 1.0 + IRIT_UEPS &&
		t2 > -IRIT_UEPS && t2 < 1.0 + IRIT_UEPS) {
		/* We detected an intersection here. */
		t1 = IRIT_BOUND(t1, 0.0, 1.0);
		t2 = IRIT_BOUND(t2, 0.0, 1.0);

		if (DetectIntr)
		    return ((Bool2DInterStruct *) TRUE);

		/* Validate the intersection on vertices. */
		if (IsInterValid(t1, t2, V1, V1Prev, V2, V2Prev)) {
		    /* Make sure it is a new intersection. */
		    for (Bool2DTmp = Bool2DHead;
			 Bool2DTmp != NULL;
			 Bool2DTmp = Bool2DTmp -> Pnext) {
		        if ((IRIT_APX_EQ(Bool2DTmp -> Param1, Pl1Param + t1) ||
			     IRIT_APX_EQ(IRIT_FABS(Bool2DTmp -> Param1 -
							     (Pl1Param + t1)),
				    4.0)) &&
			    (IRIT_APX_EQ(Bool2DTmp -> Param2, Pl2Param + t2) ||
			     IRIT_APX_EQ(IRIT_FABS(Bool2DTmp -> Param2 -
							     (Pl2Param + t2)),
				    4.0)))
			    break;
		    }

		    if (Bool2DTmp == NULL) {	      /* A new intersection! */
		        Bool2D = (Bool2DInterStruct *)
					IritMalloc(sizeof(Bool2DInterStruct));
			IRIT_PT_COPY(Bool2D -> InterPt, Pt1);
			if (HandlePolygons)
			    GMInterpVrtxNrmlBetweenTwo2(Pt1, Bool2D -> Normal,
							V1, V2);

			Bool2D -> Poly1Vrtx = V1;
			Bool2D -> Param1 = Pl1Param + t1;
			Bool2D -> Poly2Vrtx = V2;
			Bool2D -> Param2 = Pl2Param + t2;
			Bool2D -> DualInter = FALSE;

			IRIT_LIST_PUSH(Bool2D, Bool2DHead);
		    }
		    else {
		        /* Keep this intersection in addition to old one. */
		        Bool2DTmp -> Poly1Vrtx2 = V1;
			Bool2DTmp -> Poly2Vrtx2 = V2;
			Bool2DTmp -> DualInter = TRUE;
		    }
	        }
	    }

	    Pl2Param += 1.0;
	    V2Prev = V2;
	    V2 = V2 -> Pnext;
	}
	while (V2 != NULL && V2 != V2Head);

	Pl1Param += 1.0;
	V1Prev = V1;
	V1 = V1 -> Pnext;
    }
    while (V1 != NULL && V1 != V1Head);

    if (HandlePolygons && Bool2DHead != NULL && Bool2DHead -> Pnext == NULL) {
	/* If only one intersection - ignore it (point intersection). */
	IritFree(Bool2DHead);
	Bool2DHead = NULL;
    }
    
    return Bool2DHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Validates an intersection location.   Looks for intersection on vertices *
* that occurs so that the edge before and the edge after are having the same *
* directions as the other polygon.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   t1:   Betweeon zero and one where intersection occur on (V1, V1->Pnext). *
*   t2:   Betweeon zero and one where intersection occur on (V2, V2->Pnext). *
*   V1:     Edge of intersection on first polygon.                           *
*   V1Prev: Vertex previous to V2, or NULL if none.                          *
*   V2:     Edge of intersection on second polygon.                          *
*   V2Prev: Vertex previous to V2, or NULL if none.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if intersection is valid, FALSE otherwise.                   *
*****************************************************************************/
static int IsInterValid(IrtRType t1,
			IrtRType t2,
			IPVertexStruct *V1,
			IPVertexStruct *V1Prev,
			IPVertexStruct *V2,
			IPVertexStruct *V2Prev)
{
    IrtVecType Dir1Prev, Dir1Next, Dir2Prev, Dir2Next, TVec1, TVec2;
    IPVertexStruct *V1Next, *V2Next;

    /* Both intersections are interior to edge - this intersection is fine.  */
    if ((t1 < 1.0 - IRIT_UEPS && t1 > IRIT_UEPS) &&
	(t2 < 1.0 - IRIT_UEPS && t2 > IRIT_UEPS))
        return TRUE;

    /* Only one polygon intersects on its vertex and the other is interior.  */
    /* Validate only if vertex intersection occurs at the beginning of edge. */
    if ((t1 >= 1.0 - IRIT_UEPS || t1 <= IRIT_UEPS) &&
	(t2 < 1.0 - IRIT_UEPS && t2 > IRIT_UEPS)) {
        /* Verify V1Prev & V1 -> Pnext on opposite sides of V2/V2 -> Pnext. */
        IRIT_VEC2D_SUB(Dir2Next, V2 -> Pnext -> Coord, V2 -> Coord);
	IRIT_VEC2D_SUB(TVec1, V1Prev -> Coord, V2 -> Coord);
	IRIT_VEC2D_SUB(TVec2, V1 -> Pnext -> Coord, V2 -> Coord);
	if (IRIT_CROSS_PROD_2D(Dir2Next, TVec1) *
	    IRIT_CROSS_PROD_2D(Dir2Next, TVec2) > IRIT_UEPS)
	    return FALSE;   /* Prev and next are on the same side - invalid. */

        if (t1 <= IRIT_UEPS)
	    return TRUE;

	/* If previous edges are coplanar, we still need to validate this. */
        if ((V1Prev = IPGetPrevVrtx(V1 -> Pnext, V1)) == NULL)
	    return FALSE;
	IRIT_VEC_SUB(Dir1Prev, V1 -> Coord, V1Prev -> Coord);
	if ((V2Next = V2 -> Pnext) == NULL)
	    return FALSE;
	IRIT_VEC_SUB(Dir2Prev, V2Next -> Coord, V2 -> Coord);
	IRIT_PT_NORMALIZE(Dir1Prev);
	IRIT_PT_NORMALIZE(Dir2Prev);
	return IRIT_FABS(IRIT_DOT_PROD(Dir1Prev, Dir2Prev)) > 1.0 - IRIT_UEPS;
    }
    if ((t2 >= 1.0 - IRIT_UEPS || t2 <= IRIT_UEPS) &&
	(t1 < 1.0 - IRIT_UEPS && t1 > IRIT_UEPS)) {
        /* Verify V2Prev & V2 -> Pnext on opposite sides of V1/V1 -> Pnext. */
        IRIT_VEC2D_SUB(Dir1Next, V1 -> Pnext -> Coord, V1 -> Coord);
	IRIT_VEC2D_SUB(TVec1, V2Prev -> Coord, V1 -> Coord);
	IRIT_VEC2D_SUB(TVec2, V2 -> Pnext -> Coord, V1 -> Coord);
	if (IRIT_CROSS_PROD_2D(Dir1Next, TVec1) *
	    IRIT_CROSS_PROD_2D(Dir1Next, TVec2) > IRIT_UEPS)
	    return FALSE;   /* Prev and next are on the same side - invalid. */

        if (t1 <= IRIT_UEPS)
	    return TRUE;

	/* If previous edges are coplanar, we still need to validate this. */
	if (V2Prev == NULL)
	    return TRUE;

	IRIT_VEC_SUB(Dir2Prev, V2 -> Coord, V2Prev -> Coord);
	V1Next = V1 -> Pnext;
	IRIT_VEC_SUB(Dir1Prev, V1Next -> Coord, V1 -> Coord);
	IRIT_PT_NORMALIZE(Dir1Prev);
	IRIT_PT_NORMALIZE(Dir2Prev);
	return IRIT_FABS(IRIT_DOT_PROD(Dir1Prev, Dir2Prev)) > 1.0 - IRIT_UEPS;
    }

    /* If we are here, then both polygons intersect on a vertex. */

    /* Eval directions of prev/next edges to intersection.   */
    V1Next = V1 -> Pnext;
    if (t1 < IRIT_UEPS) {
	if (V1Prev == NULL)
	    return TRUE;

	IRIT_VEC_SUB(Dir1Prev, V1 -> Coord, V1Prev -> Coord);
	IRIT_VEC_SUB(Dir1Next, V1Next -> Coord, V1 -> Coord);
    }
    else { /* t1 ~= 1.0. */
	IRIT_VEC_SUB(Dir1Prev, V1Next -> Coord, V1 -> Coord);
	IRIT_VEC_SUB(Dir1Next, V1Next  -> Pnext-> Coord, V1Next -> Coord);
    }
    IRIT_PT_NORMALIZE(Dir1Prev);
    IRIT_PT_NORMALIZE(Dir1Next);

    V2Next = V2 -> Pnext;
    if (t2 < IRIT_UEPS) {
	if (V2Prev == NULL)
	    return TRUE;

	IRIT_VEC_SUB(Dir2Prev, V2 -> Coord, V2Prev -> Coord);
	IRIT_VEC_SUB(Dir2Next, V2Next -> Coord, V2 -> Coord);
    }
    else { /* t2 ~= 1.0. */
	IRIT_VEC_SUB(Dir2Prev, V2Next -> Coord, V2 -> Coord);
	IRIT_VEC_SUB(Dir2Next, V2Next -> Pnext -> Coord, V2Next -> Coord);
    }
    IRIT_PT_NORMALIZE(Dir2Prev);
    IRIT_PT_NORMALIZE(Dir2Next);

    /* Two previous edges and two next edges are identical. */
    if (IRIT_FABS(IRIT_DOT_PROD(Dir1Prev, Dir2Prev)) > 1.0 - IRIT_UEPS &&
	IRIT_FABS(IRIT_DOT_PROD(Dir1Next, Dir2Next)) > 1.0 - IRIT_UEPS)
        return FALSE;

    /* Prev edge of one poly is same as next of other poly and vice versa. */
    if (IRIT_FABS(IRIT_DOT_PROD(Dir1Prev, Dir2Next)) > 1.0 - IRIT_UEPS &&
	IRIT_FABS(IRIT_DOT_PROD(Dir1Next, Dir2Prev)) > 1.0 - IRIT_UEPS)
        return FALSE;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorts the provided list with according to Param1 (First == TRUE) or      *
* Param2 (First == FALSE). Bool2D is sorted in place.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Bool2D:      List of intersection locations to sort.                     *
*   First:       TRUE if sort according to first, FALSE otherwise.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SortParam(Bool2DInterStruct **Bool2D, int First)
{
    Bool2DInterStruct *BTmp,
	*Bool2DSorted = NULL;

    while (*Bool2D != NULL) {
	Bool2DInterStruct
	    *B = *Bool2D;

	*Bool2D = (*Bool2D) -> Pnext;
	B -> Pnext = NULL;

	if (Bool2DSorted) {
	    if ((First && Bool2DSorted -> Param1 > B -> Param1) ||
		(!First && Bool2DSorted -> Param2 > B -> Param2)) {
		/* Put it as first in list. */
		B -> Pnext = Bool2DSorted;
		Bool2DSorted = B;
	    }
	    else {
		for (BTmp = Bool2DSorted;
		     BTmp -> Pnext != NULL;
		     BTmp = BTmp -> Pnext)
		    if ((First && BTmp -> Pnext -> Param1 > B -> Param1) ||
			(!First && BTmp -> Pnext -> Param2 > B -> Param2))
			break;
		B -> Pnext = BTmp -> Pnext;
		BTmp -> Pnext = B;
	    }
	}
	else
	    Bool2DSorted = B;
    }

    *Bool2D = Bool2DSorted;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given all the intersection locations between Pl1 and Pl2 (we assume we   *
* have intersections) in Bool2D, extract the In, Out, Shared, AntiShared     *
* segments out of Pl1 with respect to Pl2.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl1:             First convex intersecting polygon. The In, Out, Shared, *
*		     AntiShared segments are all extract from Pl1.	     *
*   Pl2:             The other convex intersecting polygon.		     *
*   Bool2D:          The intersection locations.			     *
*   Pl1First:        If original Pl1 is first or second in Bool2D's data.    *
*   Pl1InPl2:        Boundary regions of Pl1 that are inside Pl2.	     *
*		     Can be NULL if this data is not needed.		     *
*   Pl1OutPl2:       Boundary regions of Pl1 that are outside Pl2.	     *
*		     Can be NULL if this data is not needed.		     *
*   Pl1SharedPl2:    Boundary regions of Pl1 that are shared by Pl2.         *
*		     Can be NULL if this data is not needed.		     *
*   Pl1AntiSharedPl2: Boundary regions of Pl1 that are antishared by Pl2.    *
*		     Can be NULL if this data is not needed.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE, if successful, FALSE otherwise.                          *
*****************************************************************************/
static int Boolean2DComputeSegments(IPPolygonStruct *Pl1,
				    IPPolygonStruct *Pl2,
				    Bool2DInterStruct **Bool2D,
				    int Pl1First,
				    IPPolygonStruct **Pl1InPl2,
				    IPPolygonStruct **Pl1OutPl2,
				    IPPolygonStruct **Pl1SharedPl2,
				    IPPolygonStruct **Pl1AntiSharedPl2)
{
    IrtRType
        Param = 0.0;
    Bool2DInterStruct *B;
    IPPolygonStruct *Pl,
        *Pls = NULL;
    IPVertexStruct *VHead, *VTail,
	*V = IPCopyVertexList(Pl1 -> PVertex);

    /* Break the vertices list from circular to linear, duplicating last. */
    VTail = IPGetLastVrtx(V);
    VTail -> Pnext = IPAllocVertex(VTail -> Tags, NULL, NULL);
    VTail = VTail -> Pnext;
    IRIT_PT_COPY(VTail -> Coord, V -> Coord);
    IRIT_PT_COPY(VTail -> Normal, V -> Normal);

    /* Sort the intersection parameters according to considered Pl1 or Pl2. */
    SortParam(Bool2D, Pl1First);

    /* Split Pl1 into segments at all the intersections. */
    for (B = *Bool2D; B != NULL && V  != NULL; B = B -> Pnext) {
        IrtRType
	    BParam = Pl1First ? B -> Param1 : B -> Param2;

	/* Verify the order here. */
	assert(B -> Pnext == NULL ||
	   (Pl1First ? B -> Pnext -> Param1 : B -> Pnext -> Param2) >= BParam);

	/* Skip interior vertices between intersections. */
	for (VTail = V; Param < (int) BParam; VTail = VTail -> Pnext, Param++);

	/* Prepare the head of the next segment. */
	if (VTail -> Pnext != NULL &&
	    IRIT_PT_PT_DIST_SQR(VTail -> Pnext -> Coord,
			   B -> InterPt) > IRIT_SQR(BOOL_IRIT_EPS)) {
	    VHead = IPAllocVertex(VTail -> Tags, NULL, VTail -> Pnext);
	    IRIT_PT_COPY(VHead -> Coord, B -> InterPt);
	    IRIT_PT_COPY(VHead -> Normal, B -> Normal);
	}
	else {
	    VHead = VTail -> Pnext;
	}

	/* And close the current segment. */
	if (IRIT_PT_PT_DIST_SQR(VTail -> Coord,
			   B -> InterPt) > IRIT_SQR(BOOL_IRIT_EPS)) {
	    VTail -> Pnext = IPAllocVertex(VTail -> Tags, NULL, NULL);
	    VTail = VTail -> Pnext;
	    IRIT_PT_COPY(VTail -> Coord, B -> InterPt);
	    IRIT_PT_COPY(VTail -> Normal, B -> Normal);
	}
	else
	    VTail -> Pnext = NULL;

	/* Allocate the polyline and save for next stage. */
	Pl = IPAllocPolygon(0, V, NULL);
	IRIT_PLANE_COPY(Pl -> Plane, Pl1 -> Plane);
	IP_SET_PLANE_POLY(Pl);
	IRIT_LIST_PUSH(Pl, Pls);

	V = VHead; /* Continue with next segment. */
	Param = BParam;
    }
    /* Allocate the last polyline and save for next stage. */
    if (V != NULL) {
        Pl = IPAllocPolygon(0, V, NULL);
	IRIT_PLANE_COPY(Pl -> Plane, Pl1 -> Plane);
	IP_SET_PLANE_POLY(Pl);
	IRIT_LIST_PUSH(Pl, Pls);
    }

    /* Classify the different segments. */
    if (Pl1InPl2 != NULL)
        *Pl1InPl2 = NULL;
    if (Pl1OutPl2 != NULL)
        *Pl1OutPl2 = NULL;
    if (Pl1SharedPl2 != NULL)
        *Pl1SharedPl2 = NULL;
    if (Pl1AntiSharedPl2 != NULL)
        *Pl1AntiSharedPl2 = NULL;

    while (Pls != NULL) {
	int Pl1InPl2Count = 0,
	    Pl1OutPl2Count = 0,
	    Pl1SharedPl2Count = 0,
	    Pl1AntiSharedPl2Count = 0;
        IPVertexStruct *V, *VNext, *V2, *V2Head, *V2Next;

        IRIT_LIST_POP(Pl, Pls);

	for (V = Pl -> PVertex; V -> Pnext != NULL; V = V -> Pnext) {
	    int AntiShared, Shared;
	    IrtPtType PMid, Ptemp;
	    IrtVecType Vec, Vec2;

	    VNext = V -> Pnext;

	    IRIT_PT_BLEND(PMid, VNext -> Coord, V -> Coord, BOOL_MID_BLEND);

	    /* Examine if PMid is on the boundary of Pl2. */
	    V2 = V2Head = Pl2 -> PVertex;
	    Shared = AntiShared = FALSE;
	    do {
	        V2Next = V2 -> Pnext ? V2 -> Pnext : V2Head;

		IRIT_PT_SUB(Vec2, V2Next -> Coord, V2 -> Coord);

		/* Find closest point on the line. */
		GMPointFromPointLine(PMid, V2 -> Coord, Vec2, Ptemp);
		if (IRIT_PT_PT_DIST_SQR(PMid, Ptemp) < IRIT_SQR(BOOL_IRIT_EPS)) {
		    /* On the line - lets see if in the segment (V, VNext). */
		    IRIT_PT_SUB(Vec, PMid, V2 -> Coord);
		    if (IRIT_DOT_PROD(Vec2, Vec) > -BOOL_IRIT_EPS &&
			IRIT_PT_PT_DIST_SQR(PMid, V2 -> Coord) <
			   BOOL_IRIT_EPS +
			   IRIT_PT_PT_DIST_SQR(V2Next -> Coord, V2 -> Coord)) {
		        IRIT_PT_SUB(Vec, VNext -> Coord, V -> Coord);
			if (IRIT_DOT_PROD(Vec2, Vec) < 0)
			    AntiShared = TRUE;
			else
			    Shared = TRUE;
		        break;
		    }
		}

		V2 = V2Next;
	    }
	    while (V2 != V2Head);

	    if (AntiShared) {
	        /* V is on the boundary of Pl2, on (V2, V2Next). */
		Pl1AntiSharedPl2Count++;
	    }
	    else if (Shared) {
	        /* V is on the boundary of Pl2, on (V2, V2Next). */
		Pl1SharedPl2Count++;
	    }
	    else if (GMPointInsideCnvxPolygon(PMid, Pl2))
	        Pl1InPl2Count++;
	    else
	        Pl1OutPl2Count++;
	}

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolyTest, FALSE) {
	        IRIT_INFO_MSG_PRINTF("Polyline under test [In %d, Out %d, Shared %d, AntiShared %d]\n",
		       Pl1InPl2Count, Pl1OutPl2Count,
		       Pl1SharedPl2Count, Pl1AntiSharedPl2Count);
		PrintVrtxList(Pl -> PVertex);
	    }
	}
#	endif /* DEBUG */

	/* Examine the tests and place in the proper output bucket. */
	if ((Pl1InPl2Count > 0) +
	    (Pl1OutPl2Count > 0) + 
	    (Pl1SharedPl2Count > 0) + 
	    (Pl1AntiSharedPl2Count > 0) > 1) {
#	    ifdef DEBUG
	        IRIT_INFO_MSG("Warning: More than one classification detected in Bool2D\n");
#	    endif /* DEBUG */
	    /* Compute the maximum and select that, zeroing everything else. */
	    if (Pl1InPl2Count > Pl1OutPl2Count)
	        Pl1OutPl2Count = 0;
	    else
	        Pl1InPl2Count = 0;

	    if (Pl1SharedPl2Count > Pl1AntiSharedPl2Count)
	        Pl1AntiSharedPl2Count = 0;
	    else
	        Pl1SharedPl2Count = 0;

	    if (IRIT_MAX(Pl1InPl2Count, Pl1OutPl2Count) >
		IRIT_MAX(Pl1SharedPl2Count, Pl1AntiSharedPl2Count))
	        Pl1SharedPl2Count = Pl1AntiSharedPl2Count = 0;
	    else
	        Pl1InPl2Count = Pl1OutPl2Count = 0;
	}

	if (Pl1InPl2Count) {
	    if (Pl1InPl2 != NULL) {
	        IRIT_LIST_PUSH(Pl, *Pl1InPl2);
	    }
	    else
	        IPFreePolygon(Pl);
	}
	else if (Pl1OutPl2Count) {
	    if (Pl1OutPl2 != NULL) {
	        IRIT_LIST_PUSH(Pl, *Pl1OutPl2);
	    }
	    else
	        IPFreePolygon(Pl);
	}
	else if (Pl1SharedPl2Count) {
	    if (Pl1SharedPl2 != NULL) {
	        IRIT_LIST_PUSH(Pl, *Pl1SharedPl2);
	    }
	    else
	        IPFreePolygon(Pl);
	}
	else if (Pl1AntiSharedPl2Count) {
	    if (Pl1AntiSharedPl2 != NULL) {
	        IRIT_LIST_PUSH(Pl, *Pl1AntiSharedPl2);
	    }
	    else
	        IPFreePolygon(Pl);
	}
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintPolySegs, FALSE) {
	    if (Pl1InPl2 != NULL && *Pl1InPl2 != NULL) {
	        IRIT_INFO_MSG("Pl1 In Pl2 extracted:\n");

		for (Pl = *Pl1InPl2; Pl != NULL; Pl = Pl -> Pnext) {
		    IRIT_INFO_MSG("Polyline\n");
		    PrintVrtxList(Pl -> PVertex);
		}
	    }
	    if (Pl1OutPl2 != NULL && *Pl1OutPl2 != NULL) {
	        IRIT_INFO_MSG("Pl1 Out Pl2 extracted:\n");

		for (Pl = *Pl1OutPl2; Pl != NULL; Pl = Pl -> Pnext) {
		    IRIT_INFO_MSG("Polyline\n");
		    PrintVrtxList(Pl -> PVertex);
		}
	    }
	    if (Pl1SharedPl2 != NULL && *Pl1SharedPl2 != NULL) {
	        IRIT_INFO_MSG("Pl1 Shared Pl2 extracted:\n");

		for (Pl = *Pl1SharedPl2; Pl != NULL; Pl = Pl -> Pnext) {
		    IRIT_INFO_MSG("Polyline\n");
		    PrintVrtxList(Pl -> PVertex);
		}
	    }
	    if (Pl1AntiSharedPl2 != NULL && *Pl1AntiSharedPl2 != NULL) {
	        IRIT_INFO_MSG("Pl1 AntiShared Pl2 extracted:\n");

		for (Pl = *Pl1AntiSharedPl2; Pl != NULL; Pl = Pl -> Pnext) {
		    IRIT_INFO_MSG("Polyline\n");
		    PrintVrtxList(Pl -> PVertex);
		}
	    }
	    IRIT_INFO_MSG("**********************************\n");
	}
    }
#endif /* DEBUG */

    return TRUE;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given vertex list, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:        Vertex list to print.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintVrtxList(IPVertexStruct *V)
{
    IPVertexStruct
	*VHead = V;

    do {
	IRIT_INFO_MSG_PRINTF("    %12lf %12lf %12lf",
	       V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	if (IP_IS_INTERNAL_VRTX(V))
	    IRIT_INFO_MSG(" (Internal)\n");
	else
	    IRIT_INFO_MSG("\n");
	V = V -> Pnext;
    }
    while (V!= NULL && V != VHead);
}

#endif /* DEBUG */
