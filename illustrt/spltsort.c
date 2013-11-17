/*****************************************************************************
* Data processing: Split all line segments that are too long into shorter    *
* pieces and sort the lines segments in Z depth order.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1993   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "misc_lib.h"

#if defined(ultrix) && defined(mips)
static int SortCompare(VoidPtr Ptr1, VoidPtr Ptr2);
#else
static int SortCompare(const VoidPtr Ptr1, const VoidPtr Ptr2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of polyline objects splits every line segment in them so that  M
* it is shorter that MaxLen.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:  Make sure all polys are shorter than MaxLen.                  M
*   MaxLen:    Maximum allowed length of edges.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SplitLongLines                                                           M
*****************************************************************************/
void SplitLongLines(IPObjectStruct *PObjects, IrtRType MaxLen)
{
    IPObjectStruct *PObj;

    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	IPPolygonStruct *PPoly;

	if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYLINE_OBJ(PObj))
	    continue;

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    IPVertexStruct *V;

	    /* Can have empty polylines. Beware! */
	    if (PPoly -> PVertex == NULL)
		continue;

	    for (V = PPoly -> PVertex; V != NULL && V -> Pnext != NULL;) {
		if (SegmentLength(V) > MaxLen) {
		    IPVertexStruct
			*NewV = IPAllocVertex2(V -> Pnext);

		    /* Split it. */
		    IRIT_PT_BLEND(NewV -> Coord,
			     V -> Coord, V -> Pnext -> Coord, 0.5);

		    V -> Pnext = NewV;
		}
		else
		    V = V -> Pnext;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Remove vertices that are tagged as internal edges. Given an internal edge, M
* the two vertices of its end are removed.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To remove internal edges from, in place.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RemoveInternalVertices                                                   M
*****************************************************************************/
void RemoveInternalVertices(IPObjectStruct *PObj)
{
    IPPolygonStruct *P;

    for (P = PObj -> U.Pl; P != NULL; P = P -> Pnext) {
	IPVertexStruct *V;
	int RemoveFirst = FALSE,
	    RemoveLast = FALSE;
	
	/* Remove vertices at the beginning. */
	while (P -> PVertex && IP_IS_INTERNAL_VRTX(P -> PVertex)) {
	    RemoveLast = TRUE;
	    if (P -> PVertex -> Pnext != NULL) {
		V = P -> PVertex -> Pnext -> Pnext;
		P -> PVertex -> Pnext -> Pnext = NULL;
	    }
	    else {
		V = NULL;
	    }			    
	    IPFreeVertexList(P -> PVertex);
	    P -> PVertex = V;
	}
	
	/* Remove vertices at the middle. */
	if (P -> PVertex != NULL) {
	    for (V = P -> PVertex; V -> Pnext != NULL;) {
		if (IP_IS_INTERNAL_VRTX(V -> Pnext) ||
		    (V -> Pnext -> Pnext == NULL && RemoveLast)) {
		    IPVertexStruct
			*VTmp = V -> Pnext;
		    
		    if (IP_IS_INTERNAL_VRTX(V -> Pnext) &&
			(V -> Pnext -> Pnext == NULL ||
			 V -> Pnext -> Pnext -> Pnext == NULL))
			RemoveFirst = TRUE;
		    
		    V -> Pnext = VTmp -> Pnext ? VTmp -> Pnext -> Pnext : NULL;
		    VTmp -> Pnext = NULL;
		    IPFreeVertexList(VTmp);
		}
		else
		    V = V -> Pnext;
	    }
	}
	
	/* Remove first vertices, if was not removed already. */
	if (RemoveFirst && !RemoveLast) {
	    V = P -> PVertex;
	    P -> PVertex = V -> Pnext;
	    V -> Pnext = NULL;
	    IPFreeVertexList(V);			
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two objects's Z values for sorting purposes as save   *
* in _sort attribute.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Ptr1, Ptr2:  Two pointers to objects.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two polygons.            *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int SortCompare(VoidPtr Ptr1, VoidPtr Ptr2)
#else
static int SortCompare(const VoidPtr Ptr1, const VoidPtr Ptr2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = AttrGetObjectRealAttrib((*(IPObjectStruct **) Ptr1), "_sort") -
	       AttrGetObjectRealAttrib((*(IPObjectStruct **) Ptr2), "_sort");

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Depth Z sort sort all data in PObjects.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:     To sort according toe Z, in place                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SortOutput                                                               M
*****************************************************************************/
void SortOutput(IPObjectStruct **PObjects)
{
    int NumObjs;
    IPObjectStruct *PObj, **PObjPtr, **PObjArray;

    /* Convert all objects to atomic objects (multi polylines -> lines etc.).*/
    /* First make sure that all polyline objects have exactly one polyline.  */
    for (PObj = *PObjects; PObj != NULL;) {
	if (IP_IS_POLY_OBJ(PObj)) {
	    while (PObj -> U.Pl -> Pnext != NULL) {
		IPObjectStruct
		    *PObjTemp = IPAllocObject("", IP_OBJ_POLY, NULL);

		PObjTemp -> Tags = PObj -> Tags;
		PObjTemp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		PObjTemp -> U.Pl = PObj -> U.Pl -> Pnext;
		IP_SET_OBJ_NAME2(PObjTemp, IP_GET_OBJ_NAME(PObj));
		PObj -> U.Pl -> Pnext = NULL;

		PObjTemp -> Pnext = PObj -> Pnext;
		PObj -> Pnext = PObjTemp;
		PObj = PObjTemp;
	    }
	    PObj = PObj -> Pnext;
	}
	else
	    PObj = PObj -> Pnext;
    }
    /* Second, split every poly to a discrete set.			     */
    /* Polylines are converted to list of lines, Pointlist to a list of	     */
    /* points.								     */
    for (PObj = *PObjects; PObj != NULL;) {
	if (IP_IS_POLY_OBJ(PObj)) {
	    if (IP_IS_POLYLINE_OBJ(PObj)) {
		if (PObj -> U.Pl -> PVertex != NULL &&
		    PObj -> U.Pl -> PVertex -> Pnext != NULL) {
		    while (PObj -> U.Pl -> PVertex -> Pnext -> Pnext != NULL) {
			IPVertexStruct *OrigV, *NewV;
			IPObjectStruct
			    *PObjTemp = IPAllocObject("", IP_OBJ_POLY, NULL);

			IP_SET_POLYLINE_OBJ(PObjTemp);
			IP_SET_OBJ_NAME2(PObjTemp, IP_GET_OBJ_NAME(PObj));
			PObjTemp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
			OrigV = PObj -> U.Pl -> PVertex -> Pnext;
			NewV = IPAllocVertex(OrigV -> Tags, NULL,
					     OrigV -> Pnext);
			IRIT_PT_COPY(NewV -> Coord, OrigV -> Coord);
			OrigV -> Pnext = NULL;
			PObjTemp -> U.Pl = IPAllocPolygon(0, NewV, NULL);
			
			PObjTemp -> Pnext = PObj -> Pnext;
			PObj -> Pnext = PObjTemp;
			PObj = PObjTemp;
		    }
		    PObj = PObj -> Pnext;
		}
		else
		    PObj = PObj -> Pnext;
	    }
	    else if (IP_IS_POINTLIST_OBJ(PObj)) {
		if (PObj -> U.Pl -> PVertex != NULL) {
		    IPObjectStruct
			*PObjNext = PObj -> Pnext;
		    IPVertexStruct *V,
			*VHead = PObj -> U.Pl -> PVertex;

		    PObj -> ObjType = IP_OBJ_POINT;
		    IRIT_PT_COPY(PObj -> U.Pt, VHead -> Coord);

		    for (V = VHead -> Pnext; V != NULL; V = V -> Pnext) {
			IPObjectStruct
			    *PObjTemp = IPAllocObject("", IP_OBJ_POINT, NULL);

			PObjTemp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
			IP_SET_OBJ_NAME2(PObjTemp, IP_GET_OBJ_NAME(PObj));
			IRIT_PT_COPY(PObjTemp -> U.Pt, V -> Coord);

			PObjTemp -> Pnext = PObj -> Pnext;
			PObj -> Pnext = PObjTemp;
			PObj = PObjTemp;
		    }

		    IPFreeVertexList(VHead);

		    PObj = PObjNext;
		}
		else
		    PObj = PObj -> Pnext;
	    }
	    else {
		IRIT_FATAL_ERROR("Only pointlist and polylines expected,\n");
	    }
	}
	else
	    PObj = PObj -> Pnext;
    }

    /* Compute a sorting key for each object in the list. */
    for (PObj = *PObjects, NumObjs = 0;
	 PObj != NULL;
	 PObj = PObj ->Pnext, NumObjs++) {
	IrtRType
	    Key = -IRIT_INFNTY;

	if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYLINE_OBJ(PObj)) {
	    if (PObj -> U.Pl -> PVertex != NULL &&
		PObj -> U.Pl -> PVertex -> Pnext != NULL)
		Key = (PObj -> U.Pl -> PVertex -> Coord[2] +
		       PObj -> U.Pl -> PVertex -> Pnext -> Coord[2]) * 0.5;
	}
	else if (IP_IS_POINT_OBJ(PObj))
	    Key = PObj -> U.Pt[2];
	else if (IP_IS_CTLPT_OBJ(PObj)) {
	    CagdPType E3Point;
	    CagdRType
		*R = PObj -> U.CtlPt.Coords;

	    CagdCoerceToE3(E3Point, &R, -1, PObj -> U.CtlPt.PtType);
	    Key = E3Point[2];
	}
	else if (IP_IS_VEC_OBJ(PObj))
	    Key = PObj -> U.Vec[2];

	AttrSetObjectRealAttrib(PObj, "_sort", Key);
    }

    /* Actually sort... */
    PObjArray = (IPObjectStruct **)
	IritMalloc(sizeof(IPObjectStruct *) * NumObjs);
    for (PObj = *PObjects, PObjPtr = PObjArray;
	 PObj != NULL;
	 PObj = PObj -> Pnext)
	*PObjPtr++ = PObj;
    qsort(PObjArray, NumObjs, sizeof(IPObjectStruct *), SortCompare);
    *PObjects = *PObjArray;
    for (PObjPtr = PObjArray; --NumObjs; PObjPtr++)
	PObjPtr[0] -> Pnext = PObjPtr[1];
    PObjPtr[0] -> Pnext = NULL;
    IritFree(PObjArray);
}
