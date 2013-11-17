/******************************************************************************
* Poly_cln.c - Clean polygonal data/massage it.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 1993.					      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_loc.h"

#define MAX_FILTER_INTERIOR_ITERS 10

#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2);
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compare two polygons if share the same geometry.   Two polygons are      M
* considered same if the share the same vertices in order (or in reverse).   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2:     Two polygons to compare.                                   M
*   Eps:	  Tolerance of vertices equality, etc.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if two polygons posses same geometry, FALSE otherwise.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpDupPolys                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTwoPolySameGeom                                                        M
*****************************************************************************/
int GMTwoPolySameGeom(const IPPolygonStruct *Pl1,
		      const IPPolygonStruct *Pl2,
		      IrtRType Eps)
{
    static int
	VRevSize = 0;
    static IPVertexStruct
	**VRevList = NULL;
    IPVertexStruct *V1, *V2, *V2Match,
	*V1Head = Pl1 -> PVertex,
	*V2Head = Pl2 -> PVertex;
    int i, Found,
	Len1 = IPVrtxListLen(V1Head),
	Len2 = IPVrtxListLen(V2Head);

    if (Len1 != Len2)
	return FALSE;

    /* Find an identical vertex at the second polygon, if exists. */
    V1 = V1Head;
    V2 = V2Head;
    Found = FALSE;
    do {
        if (IRIT_PT_APX_EQ_EPS(V1 -> Coord, V2 -> Coord, Eps)) {
	    Found = TRUE;
	    break;
	}
	V2 = V2 -> Pnext;
    }
    while (V2 != NULL && V2 != V2Head);

    if (!Found)
        return FALSE;

    V2Match = V2;

    /* Scan forward on both polys. */
    V1 = V1Head -> Pnext;
    Found = FALSE;
    do {
	V2 = V2 -> Pnext;
	if (V2 == NULL)
	    V2 = V2Head;

        if (!IRIT_PT_APX_EQ_EPS(V1 -> Coord, V2 -> Coord, Eps)) {
	    Found = TRUE;
	    break;
	}

	V1 = V1 -> Pnext;
    }
    while (V1 != NULL && V1 != V1Head);

    if (!Found)
	return TRUE; /* No difference was found between the two. */

    /* Scan backward on both polys. */
    if (VRevSize < Len1) {
	if (VRevList)
	    IritFree(VRevList);
	VRevSize = Len1 * 2;
	VRevList = IritMalloc(sizeof(IPVertexStruct *) * VRevSize);
    }
    for (V1 = V1Head, i = Len1 - 1; i >= 0; V1 = V1 -> Pnext, i--)
        VRevList[i] = V1;

    V2 = V2Match;

    Found = FALSE;
    i = 0;
    do {
	V2 = V2 -> Pnext;
	if (V2 == NULL)
	    V2 = V2Head;

        if (!IRIT_PT_APX_EQ_EPS(VRevList[i] -> Coord, V2 -> Coord, Eps)) {
	    Found = TRUE;
	    break;
	}

	V1 = V1 -> Pnext;
	i++;
    }
    while (i < Len1);

    return !Found;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to search and remove duplicated identical polygons in the input  M
* model, in place.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolygons:    List of polygons to clean, in place.                       M
*   Eps:	  Tolerance of vertices equality, etc.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Reference to the filtered polygons.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolylineList, GMVrtxListToCircOrLin, GMFilterInteriorVertices   M
*   GMCleanUpPolygonList, GMTwoPolySameGeom				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCleanUpDupPolys, duplicated polygons, cleaning                         M
*****************************************************************************/
IPPolygonStruct *GMCleanUpDupPolys(IPPolygonStruct **PPolygons, IrtRType Eps)
{
    IPPolygonStruct *Pl, *Pl2,
	*PlCleaned = NULL;

    while (*PPolygons != NULL) {
        IRIT_LIST_POP(Pl, *PPolygons);

	for (Pl2 = *PPolygons; Pl2 != NULL; Pl2 = Pl2 -> Pnext) {
	    if (GMTwoPolySameGeom(Pl, Pl2, Eps))
		break;
	}

	if (Pl2 == NULL) {
	    IRIT_LIST_PUSH(Pl, PlCleaned);
	}
	else
	    IPFreePolygon(Pl);
    }

    *PPolygons = PlCleaned;

    return PlCleaned;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to clean up polygons - delete zero length edges, and polygons    M
* with less than 3 vertices, in place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolygons:    List of polygons to clean, in place.                       M
*   Eps:	  Tolerance of vertices equality, etc.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Reference to the filtered polygons.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolylineList, GMVrtxListToCircOrLin, GMFilterInteriorVertices   M
*   GMCleanUpDupPolys							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCleanUpPolygonList, zero length edges, cleaning                        M
*****************************************************************************/
IPPolygonStruct *GMCleanUpPolygonList(IPPolygonStruct **PPolygons,
				      IrtRType Eps)
{
    IPPolygonStruct *PPHead, *PPLast;
    IPVertexStruct *PVHead, *PVTemp, *PVNext;

    PPLast = PPHead = *PPolygons;

    while (PPHead != NULL) {
	PVHead = PPHead -> PVertex;

	/* Look for zero length edges (V == V -> Pnext): */
	while (PVHead -> Pnext != NULL &&
	       PVHead -> Pnext != PVHead &&
	       IRIT_PT_APX_EQ_EPS(PVHead -> Coord, PVHead -> Pnext -> Coord,
				  Eps)) {
	     PVTemp = PVHead -> Pnext;
	     PVHead -> Pnext = PVTemp -> Pnext;
	     IPFreeVertex(PVTemp);
	}

	if (PVHead -> Pnext != NULL && PVHead -> Pnext != PVHead) {
	    PVTemp = PVHead;
	    do {
	        PVNext = PVTemp -> Pnext;
		if (IRIT_PT_APX_EQ_EPS(PVTemp -> Coord, PVNext -> Coord,
				       Eps)) {
		    /* Delete PVNext. */
		    PVTemp -> Pnext = PVNext -> Pnext;

		    if (PVHead == PVNext) {   /* If we actually kill header. */
		        PPHead -> PVertex = PVHead = PVTemp;
			IPFreeVertex(PVNext);
			break;
		    }
		    IPFreeVertex(PVNext);
		}
		else
		    PVTemp = PVTemp -> Pnext;
	    }
	    while (PVTemp != NULL &&
		   PVTemp -> Pnext != NULL &&
		   PVTemp != PVHead &&
		   PVHead -> Pnext != PVHead);
	}

	/* Now test if at least 3 vertices in polygon, otherwise delete it:  */
	if (PVHead == PVHead -> Pnext ||		 /* One vertex only. */
	    PVHead == PVHead -> Pnext -> Pnext ||      /* Two vertices only. */
	    PVHead -> Pnext == NULL ||			 /* One vertex only. */
	    PVHead -> Pnext -> Pnext == NULL) {        /* Two vertices only. */
	    if (PPHead == *PPolygons) {
		*PPolygons = (*PPolygons) -> Pnext;
		IPFreePolygon(PPHead);
		PPHead = (*PPolygons);
	    }
	    else {
		PPLast -> Pnext = PPHead -> Pnext;
		IPFreePolygon(PPHead);
		PPHead = PPLast -> Pnext;
	    }
	}
	else {
	    PPLast = PPHead;
	    PPHead = PPHead -> Pnext;
	}
    }

    return *PPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to clean up polylines of zero length, in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolylines:    List of polylines to clean, in place.                     M
*   Eps:	   Tolerance of vertices equality, etc.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Reference to the filtered polylines.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolygonList, GMVrtxListToCircOrLin, GMFilterInteriorVertices    M
*   GMCleanUpPolylineList2						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCleanUpPolylineList, zero length polyline, cleaning                    M
*****************************************************************************/
IPPolygonStruct *GMCleanUpPolylineList(IPPolygonStruct **PPolylines,
				       IrtRType Eps)
{
    IPPolygonStruct
	*Poly = *PPolylines;

    /* Remove empty sized polylines. */
    while (Poly != NULL &&
	   (Poly -> PVertex == NULL ||
	    Poly -> PVertex -> Pnext == NULL)) {
	*PPolylines = (*PPolylines) -> Pnext;
	IPFreePolygon(Poly);
	Poly = *PPolylines;
    }

    if (Poly && Poly -> Pnext) {
	while (Poly -> Pnext != NULL) {
	    if (Poly -> Pnext -> PVertex == NULL ||
		Poly -> Pnext -> PVertex -> Pnext == NULL) {
		IPPolygonStruct
		    *TmpPoly = Poly -> Pnext;

		Poly -> Pnext = TmpPoly -> Pnext;
		IPFreePolygon(TmpPoly);
	    }
	    else
		Poly = Poly ->Pnext;
	}
    }

    /* Filter out zero length edges. */
    for (Poly = *PPolylines; Poly != NULL; Poly = Poly -> Pnext) {
        IPVertexStruct
	    *V = Poly -> PVertex;

	while (V -> Pnext != NULL) {
	    IPVertexStruct
	        *VNext = V -> Pnext;

	    if (IRIT_PT_APX_EQ_EPS(V -> Coord, VNext -> Coord, Eps)) {
		V -> Pnext = VNext -> Pnext;
		IPFreeVertex(VNext);
	    }
	    else
		V = V -> Pnext;
	}
    }

    return *PPolylines;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to clean up colinear points in polylines, in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolylines:    List of polylines to clean, in place.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Reference to the filtered polylines.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolygonList, GMVrtxListToCircOrLin, GMFilterInteriorVertices    M
*   GMCleanUpPolylineList						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCleanUpPolylineList2, colinear points, cleaning	                     M
*****************************************************************************/
IPPolygonStruct *GMCleanUpPolylineList2(IPPolygonStruct *PPolylines)
{
    IPPolygonStruct *Poly;

    for (Poly = PPolylines; Poly != NULL; Poly = Poly -> Pnext) {
        IPVertexStruct
	    *V = Poly -> PVertex;

	if (V == NULL || V -> Pnext == NULL || V -> Pnext -> Pnext == NULL)
	    continue;

	while (V -> Pnext -> Pnext != NULL) {
	    IPVertexStruct
	        *VNext = V -> Pnext,
	        *VNextNext = VNext -> Pnext;

	    if (GMCollinear3Pts(V -> Coord, VNext -> Coord,
				VNextNext -> Coord)) {
	        /* Purge VNext. */
	        V -> Pnext = VNextNext;
	        IPFreeVertex(VNext);
	    }
	    else
		V = V -> Pnext;
	}
    }

    return PPolylines;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to make sure all polygons given are circular/linear.  Update in  M
* place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:    List of polygons to make sure are circular/linear, in place.     M
*   DoCirc: If TRUE, list are made circular.  If FALSE, vertices are NULL    M
*	    terminated.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolylineList, GMCleanUpPolygonList, GMFilterInteriorVertices    M
*   GMVrtxListToCircOrLinDup						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVrtxListToCircOrLin, circular lists		                     M
*****************************************************************************/
void GMVrtxListToCircOrLin(IPPolygonStruct *Pls, int DoCirc)
{
    for ( ; Pls != NULL; Pls = Pls -> Pnext) {
        IPVertexStruct
	    *V = Pls -> PVertex;

	if (V == NULL)
	    continue;

	for ( ;
	     V -> Pnext != NULL && V -> Pnext != Pls -> PVertex;
	     V = V -> Pnext);

	V -> Pnext = DoCirc ? Pls -> PVertex : NULL;
    }
} 

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to make sure all polylines given are circular/linear.  Update in M
* place.								     M
*   If circular and made linear, first vertex is duplicated as last and same M
* when linear is made circular.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:    List of polylines to make sure are circular/linear, in place.    M
*   DoCirc: If TRUE, list are made circular.  If FALSE, vertices are NULL    M
*	    terminated.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolylineList, GMCleanUpPolygonList, GMFilterInteriorVertices    M
*   GMVrtxListToCircOrLin						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVrtxListToCircOrLinDup, circular lists		                     M
*****************************************************************************/
void GMVrtxListToCircOrLinDup(IPPolygonStruct *Pls, int DoCirc)
{
    for ( ; Pls != NULL; Pls = Pls -> Pnext) {
        IPVertexStruct
	    *V = Pls -> PVertex;

	if (V == NULL)
	    continue;

	for ( ;
	     V -> Pnext != NULL && V -> Pnext != Pls -> PVertex;
	     V = V -> Pnext);

	if (DoCirc) {
	    if (IRIT_PT_APX_EQ(V -> Coord, Pls -> PVertex -> Coord)) {
	        IPVertexStruct
		    *VPrev = IPGetPrevVrtx(Pls -> PVertex, V);

		IPFreeVertex(V);
		VPrev -> Pnext = Pls -> PVertex;
	    }
	    else
	        V -> Pnext = Pls -> PVertex;
	}
	else {
	    if (!IRIT_PT_APX_EQ(V -> Coord, Pls -> PVertex -> Coord)) {
	        V -> Pnext = IPCopyVertex(Pls -> PVertex);
		V = V -> Pnext;
	    }
	    V -> Pnext = NULL;
	}
    }
} 

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two real numbers for sorting purposes.                *
*                                                                            *
* PARAMETERS:                                                                *
*   PReal1, PReal2:  Two pointers to real numbers.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two reals.               *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2)
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
	Diff = (*((CagdRType *) PReal1)) - (*((CagdRType *) PReal2));

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Filters out interior vertices to upto n interior vertices, in place.     M
*   Computes the angle between adjacent edges and purge the almost collinear M
* ones until we have n interior vertices.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   VHead:   Pointer to head of NULL terminated list of vertices.            M
*   MinTol:  Vertices that the inner product of previous edge direction and  M
*	     next edge direction is more than MinTol are purged.	     M
*   n:       Number of interior vertices to keep.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: Similar list modified in place with only n interior    M
*		      vertices.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCleanUpPolylineList, GMCleanUpPolygonList, GMVrtxListToCircOrLin       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFilterInteriorVertices                                                 M
*****************************************************************************/
IPVertexStruct *GMFilterInteriorVertices(IPVertexStruct *VHead,
					 IrtRType MinTol,
					 int n)
{
    int l;

    /* Be on the safe side - should not do all iterations. */
    for (l = 1; l < MAX_FILTER_INTERIOR_ITERS; l++) {
	int n1,
	    Len = IPVrtxListLen(VHead) - 2,
	    i = 0;
	IrtRType RemoveAngle, *Angles;
	IrtVecType V1, V2;
	IPVertexStruct
	    *V = VHead -> Pnext;

	if (Len <= n)
	    return VHead;
	Angles = (IrtRType *) IritMalloc(sizeof(IrtRType) * Len);
	n1 = Len - n;	    /* Now n holds the number of vertices to remove. */

	/* Computer angular deviation as cosine of the angle. */
	IRIT_VEC_SUB(V2, V -> Coord, VHead -> Coord);
	IRIT_VEC_NORMALIZE(V2);
	for ( ; V -> Pnext != NULL; V = V -> Pnext) {
	    IRIT_VEC_COPY(V1, V2);
	    IRIT_VEC_SUB(V2, V -> Pnext -> Coord, V -> Coord);
	    IRIT_VEC_NORMALIZE(V2);
	    AttrSetRealAttrib(&V -> Attr, "_Angle",
			      Angles[i++] = -IRIT_DOT_PROD(V1, V2));
	}

	/* Sort the angles and pick the n to remove. */
	qsort(Angles, i, sizeof(CagdRType), CompareReal);
	RemoveAngle = IRIT_MAX(Angles[n1], -MinTol);
	IritFree(Angles);

	/* Remove vertices with angular deviation of less than RemoveAngle. */
	for (V = VHead; V -> Pnext -> Pnext != NULL; ) {
	    if (AttrGetRealAttrib(V -> Pnext -> Attr, "_Angle") < RemoveAngle) {
		IPVertexStruct
		    *VNext = V -> Pnext;

		V -> Pnext = VNext -> Pnext;
		IPFreeVertex(VNext);
		if (V -> Pnext -> Pnext != NULL)
		    V = V -> Pnext;   /* Do not purge two vertices in a row. */
	    }
	    else {
		AttrFreeOneAttribute(&V -> Pnext -> Attr, "_Angle");
		V = V -> Pnext;
	    }
	}
    }

    return VHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clips polygons that are at the negative side of the plane foreach        M
* Ax + By + Cz + D < 0.  Clipped polygons are returned in PClipped list      M
* whereas polygons that intersects the plane Plane are returned in PInter.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PHead:    Pointer to head of a NULL terminated list of polygons.         M
*   PClipped: List of clipped polygons on the negative side of Plane, if any.M
*   PInter:   List of polygons that intersects Plane, if any.		     M
*   Plane:    Plane to clip against.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: List of polygons in the positive domain of the Plane. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMClipPolysAgainstPlane                                                  M
*****************************************************************************/
IPPolygonStruct *GMClipPolysAgainstPlane(IPPolygonStruct *PHead,
					 IPPolygonStruct **PClipped,
					 IPPolygonStruct **PInter,
					 IrtPlnType Plane)
{
    IPPolygonStruct
	*Pl = IPCopyPolygonList(PHead);

    *PClipped = *PInter = PHead = NULL;

    /* Be on the safe side - should not do all iterations. */
    while (Pl != NULL) {
	int Below = FALSE,
	    Above = FALSE;
        IPPolygonStruct
	    *PNext = Pl -> Pnext;
	IPVertexStruct
	    *V = Pl -> PVertex;

	Pl -> Pnext = NULL;

	do {
	    IrtRType
		R = IRIT_DOT_PROD(Plane, V -> Coord) + Plane[3];

	    if (R < 0)
		Below = TRUE;
	    if (R > 0)
		Above = TRUE;
	  
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);

	if ((Above && Below) || (!Above && !Below)) {
	    IRIT_LIST_PUSH(Pl, *PInter);
	}
	else if (Above) {
	    IRIT_LIST_PUSH(Pl, PHead);
	}
	else if (Below) {
	    IRIT_LIST_PUSH(Pl, *PClipped);
	}

	Pl = PNext;
    }

    return PHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two points in triangle, find the third (other) point (vertex) in   M 
*   the triangle.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:		 Triangle containing edge (V, VNext).			     M
*   V, VNext:    Two given points in triangle.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: Pointer to the third vertex, or NULL if error.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindThirdPointInTriangle                                               M
*****************************************************************************/
IPVertexStruct *GMFindThirdPointInTriangle(const IPPolygonStruct *Pl,
					   const IPVertexStruct *V,
					   const IPVertexStruct *VNext)
{
    IPVertexStruct *VTriangle; 
   
    /* Find other point in polygon. */
    VTriangle = Pl -> PVertex;
    do {
         if (!IRIT_PT_APX_EQ(V -> Coord, VTriangle -> Coord) &&
    	     !IRIT_PT_APX_EQ(VNext -> Coord, VTriangle -> Coord)) {
	     return VTriangle; 
	 }
	 VTriangle = VTriangle -> Pnext;
    }
    while (VTriangle != NULL && VTriangle != Pl -> PVertex);
		
    return NULL;
}
