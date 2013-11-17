/******************************************************************************
* MergePly.c - merge points and polylines into as-large-as-possible polylines.*
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb 2006.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"
#include "bool_lib.h"

#define GM_TEST_CLOSEST_DIST(V1, V2) { \
	IrtRType DstSqr; \
	if ((DstSqr = IRIT_PT_PT_DIST_SQR(V1 -> Coord, \
					  V2 -> Coord)) < MinSqr) { \
	    MinSqr = DstSqr; \
	} \
    }

#define GM_SET_LAST_VERTEX(Pl) (Pl) -> PAux = IPGetLastVrtx((Pl) -> PVertex)
#define GM_SET_LAST_VERTEX2(Pl, VLast) (Pl) -> PAux = VLast
#define GM_GET_LAST_VERTEX(Pl) ((IPVertexStruct *) (Pl) -> PAux)

typedef struct GMMergeEntitiesStruct {
    IrtRType MinDistSqr;      /* Minimal distance squared to another entity. */
    int MinDistSqrEntity;	/* Index of minimal distance squared entity. */
    int Valid;
    int Entity;					   /* This geometric entity. */
} GMMergeEntitiesStruct;

IRIT_STATIC_DATA GMMergeGeomKeyFuncType
    GMGlblKeyFunc = NULL;

static IrtRType GMMergeGeomDistSqr2Polys(VoidPtr VPl1, VoidPtr VPl2);
static IrtRType GMMergeGeomKeyPoly(VoidPtr GE);
static int GMMergeGeomMerge2Polys(void **VPl1, void **VPl2);
static int GMMergeGeometryAux(void **GeomEntities,
			      int NumOfGEntities,
			      IrtRType Eps,
			      IrtRType IdenticalEps,
			      GMMergeGeomInitFuncType InitFunc,
			      GMMergeGeomDistFuncType DistSqrFunc,
			      GMMergeGeomKeyFuncType KeyFunc,
			      GMMergeGeomMergeFuncType MergeFunc);
#if defined(ultrix) && defined(mips)
static int GMCompareEntities(void *GE1, void *GE2);
#else
static int GMCompareEntities(const void *GE1, const void *GE2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initialize the polylines to be merged.  Here, it amounts to caching the  *
* last vertex for fast access.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   VPl:    The polyline to initialize.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void GMMergeGeomInitPolys(VoidPtr VPl)
{
    IPPolygonStruct
	*Pl = (IPPolygonStruct *) VPl;

    GM_SET_LAST_VERTEX(Pl);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes minimal distance squared between end vertices of two polylines. *
*                                                                            *
* PARAMETERS:                                                                *
*   VPl1, VPl2:  To compute the minimal distance between the end points of.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Minimal distance squared computed.			     *
*****************************************************************************/
static IrtRType GMMergeGeomDistSqr2Polys(VoidPtr VPl1, VoidPtr VPl2)
{
    IrtRType
	MinSqr = IRIT_INFNTY;
    IPPolygonStruct
	*Pl1 = (IPPolygonStruct *) VPl1,
	*Pl2 = (IPPolygonStruct *) VPl2;
    IPVertexStruct
	*V1Start = Pl1 -> PVertex,
	*V1End = GM_GET_LAST_VERTEX(Pl1),
        *V2Start = Pl2 -> PVertex,
        *V2End = GM_GET_LAST_VERTEX(Pl2);

    GM_TEST_CLOSEST_DIST(V1Start, V2Start);
    GM_TEST_CLOSEST_DIST(V1End,   V2Start);
    GM_TEST_CLOSEST_DIST(V1Start, V2End);
    GM_TEST_CLOSEST_DIST(V1End,   V2End);

    return MinSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns a key to sort the polylines accordingly.			     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   VPl:  Polyline to return its key.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:                                                                *
*****************************************************************************/
static IrtRType GMMergeGeomKeyPoly(VoidPtr VPl)
{
    IPPolygonStruct
        *Pl = (IPPolygonStruct *) VPl;
    IrtRType
	*Coord = Pl -> PVertex -> Coord;

    /* Use the XY coordinates of the first vertex as the key. */
    return Coord[0] * 3 + Coord[1] * 7;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges two polylines into one.  The address that references Pl2 is set   *
* to NULL after Pl2 is freed.  Pl1 will hold the merged poly.                *
*                                                                            *
* PARAMETERS:                                                                *
*   VPl1, VPl2:    The two polylines to merge into one.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if successful, FALSE otherwise.                            *
*****************************************************************************/
static int GMMergeGeomMerge2Polys(void **VPl1, void **VPl2)
{
    IPPolygonStruct
	**Pl1 = (IPPolygonStruct **) VPl1,
	**Pl2 = (IPPolygonStruct **) VPl2;
    IPVertexStruct
	*V1Start = (*Pl1) -> PVertex,
	*V1End = GM_GET_LAST_VERTEX(*Pl1),
        *V2Start = (*Pl2) -> PVertex,
        *V2End = GM_GET_LAST_VERTEX(*Pl2);
    IrtRType
	d00 = IRIT_PT_PT_DIST_SQR(V1Start -> Coord, V2Start -> Coord),
	d10 = IRIT_PT_PT_DIST_SQR(V1End -> Coord,   V2Start -> Coord),
	d01 = IRIT_PT_PT_DIST_SQR(V1Start -> Coord, V2End -> Coord),
	d11 = IRIT_PT_PT_DIST_SQR(V1End -> Coord,   V2End -> Coord);

    if (d00 < d10 && d00 < d01 && d00 < d11) {/* Merge Start Pl1 & Start Pl2.*/
        (*Pl1) -> PVertex = V1Start = IPReverseVrtxList2(V1End = V1Start);
    }
    else if (d10 < d01 && d10 < d11) {	       /* Merge End Pl1 & Start Pl2. */
    }
    else if (d01 < d11) {		       /* Merge Start Pl1 & End Pl2. */
        IRIT_SWAP(IPVertexStruct *, (*Pl1) -> PVertex, (*Pl2) -> PVertex);
	IRIT_SWAP(IPVertexStruct *, V1Start, V2Start);
	IRIT_SWAP(IPVertexStruct *, V1End, V2End);
    }
    else {				         /* Merge End Pl1 & End Pl2. */
        (*Pl2) -> PVertex = V2Start = IPReverseVrtxList2(V2End = V2Start);
    }

    /* Is merged point identical?  If so purge one of the two point. */
    if (IRIT_PT_APX_EQ_EPS(V1End -> Coord, V2Start -> Coord, IRIT_UEPS)) {
	V1End -> Pnext = V2Start -> Pnext; 
	V2Start -> Pnext = NULL;
    }
    else {
	V1End -> Pnext = V2Start;
	(*Pl2) -> PVertex = NULL;
    }
    IPFreePolygon(*Pl2);
    *Pl2 = NULL;

    GM_SET_LAST_VERTEX2(*Pl1, V2End);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two geometric entities' minimal distance for sorting  *
* purposes.							             *
*                                                                            *
* PARAMETERS:                                                                *
*   GE1, GE2:  Two pointers to GMMergeEntitiesStruct structs.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two distances.           *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int GMCompareEntities(void *GE1, void *GE2)
#else
static int GMCompareEntities(const void *GE1, const void *GE2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = GMGlblKeyFunc(*((void **) GE1)) -
	       GMGlblKeyFunc(*((void **) GE2));

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merges separated geometric entities into longer ones, in place, as much  M
* as possible. Given a list of entities (typically points or polylines),     M
* find and merge closest ones as possible and in place.  This function       M
* should only be used to merge geometry that is very precise and adjacent    M
* entities distance is very small.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   GeomEntities:    Geometry to merge, typically points or polylines, as a  M
*		     vector of reference pointers to the entities.           M
*		       Returned, merged, data will also be stored here.      M
*   NumOfGEntities:  Number of geometric entities.  Also length of	     M
*		     GeomEntities.  					     M
*   IdenticalEps:    Epsilon to consider two entities' distance as zero.     M
*		     This epsilon is typically very small, i.e., IRIT_UEPS.  M
*   InitFunc:        A function to initialize all geometry.  NULL for the    M
*		     initialization of IPPolygonStruct polylines.	     M
*   DistSqrFunc:     A distance computation function. NULL for two	     M
*		     IPPolygonStruct polylines compare.  This function       M
*		     computes the minimal distance squared between two	     M
*		     entities.						     M
*   KeyFunc:	     A function to return a Key to sort the entities so      M
*		     that similar/adjacent entities will get similar a Key.  M
*   MergeFunc:       A merge function. NULL to merge two IPPolygonStructs    M
*		     polylines merge.  This function returns a merged entity M
*		     while destroying the two input entities.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	     Number of merged entities in the end or 0 if error.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMergeGeometry                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMergeSameGeometry                                                      M
*****************************************************************************/
int GMMergeSameGeometry(void **GeomEntities,
			int NumOfGEntities,
			IrtRType IdenticalEps,
			GMMergeGeomInitFuncType InitFunc,
			GMMergeGeomDistFuncType DistSqrFunc,
			GMMergeGeomKeyFuncType KeyFunc,
			GMMergeGeomMergeFuncType MergeFunc)
{
    int i, j,
	N = NumOfGEntities;
    IrtRType
        IdenticalEpsSqr = IRIT_SQR(IdenticalEps);

    if (NumOfGEntities <= 1)
        return NumOfGEntities;

    if (InitFunc == NULL)
        InitFunc = GMMergeGeomInitPolys;
    if (DistSqrFunc == NULL)
        DistSqrFunc = GMMergeGeomDistSqr2Polys;
    if (KeyFunc == NULL)
        KeyFunc = GMMergeGeomKeyPoly;
    if (MergeFunc == NULL)
        MergeFunc = GMMergeGeomMerge2Polys;

    /* Initialize all geometry. */
    for (i = 0; i < N; i++)
	GMMergeGeomInitPolys(GeomEntities[i]);

    /* Sort the data to localize the entities. */
    GMGlblKeyFunc = KeyFunc;
    qsort(GeomEntities, N, sizeof(VoidPtr), GMCompareEntities);

    for (i = 0; i < N && GeomEntities[i] != NULL; i++) {
        for (j = i + 1; j < N && GeomEntities[j] != NULL; j++) {
	    if (DistSqrFunc(GeomEntities[i],
			    GeomEntities[j]) < IdenticalEpsSqr) {
	        MergeFunc(&GeomEntities[i], &GeomEntities[j]);
	    }
	}
    }

    /* Clean the vector from NULL entities. */
    for (j = N - 1; GeomEntities[j] == NULL; N = j--);
    for (i = 0; i < j; i++) {
        if (GeomEntities[i] == NULL) {
	    GeomEntities[i] = GeomEntities[N = j--];
	    for ( ; GeomEntities[j] == NULL; N = j--);
	}
    }

    return N;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merges separated geometric entities into longer ones, in place, as much  M
* aspossible. Given a list of entities (typically points or polylines),      M
* find and merge closest ones as possible and in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   GeomEntities:    Geometry to merge, typically points or polylines, as a  M
*		     vector of reference pointers to the entities.           M
*		       Returned, merged, data will also be stored here.      M
*   NumOfGEntities:  Number of geometric entities.  Also length of	     M
*		     GeomEntities.  					     M
*   Eps:	     Epsilon of similarity to merge entities at.  Entities   M
*		     farther than Eps will not be merged.                    M
*   IdenticalEps:    Epsilon to consider two entities' distance as zero.     M
*   InitFunc:        A function to initialize all geometry.  NULL for the    M
*		     initialization of IPPolygonStruct polylines.	     M
*   DistSqrFunc:     A distance computation function. NULL for two	     M
*		     IPPolygonStruct polylines compare.  This function       M
*		     computes the minimal distance squared between two	     M
*		     entities.						     M
*   KeyFunc:	     A function to return a Key to sort the entities so      M
*		     that similar/adjacent entities will get similar a Key.  M
*   MergeFunc:       A merge function. NULL to merge two IPPolygonStructs    M
*		     polylines merge.  This function returns a merged entity M
*		     while destroying the two input entities.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	     Number of merged entities in the end or 0 if error.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPolyMergePolylines, GMMergePolylines, GMMergeSameGeometry            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMergeGeometry, merge, points, polylines                                M
*****************************************************************************/
int GMMergeGeometry(void **GeomEntities,
		    int NumOfGEntities,
		    IrtRType Eps,
		    IrtRType IdenticalEps,
		    GMMergeGeomInitFuncType InitFunc,
		    GMMergeGeomDistFuncType DistSqrFunc,
		    GMMergeGeomKeyFuncType KeyFunc,
		    GMMergeGeomMergeFuncType MergeFunc)
{
    /* Since the complexity is quadratic and gets really slow above a few   */
    /* thousands elements, split the problem recursively.		    */
    if (NumOfGEntities > 1000) {
        int N1, N2,
	    N = NumOfGEntities / 2;

	N1 = GMMergeGeometry(GeomEntities, N, Eps,
			     IdenticalEps, InitFunc, DistSqrFunc,
			     KeyFunc, MergeFunc);

	N2 = GMMergeGeometry(&GeomEntities[N], NumOfGEntities - N, Eps,
			     IdenticalEps, InitFunc, DistSqrFunc,
			     KeyFunc, MergeFunc);

	/* And now merge the results. */
	IRIT_GEN_COPY(&GeomEntities[N1], &GeomEntities[N], sizeof(VoidPtr) * N2);

	NumOfGEntities = N1 + N2;	
    }

    return GMMergeGeometryAux(GeomEntities, NumOfGEntities, Eps,
			      IdenticalEps, InitFunc, DistSqrFunc,
			      KeyFunc, MergeFunc);
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function to function GMMergeGeometry.  Does all the hard work.   *
*   Merges separated geometric entities into longer ones, in place, as much  *
* aspossible. Given a list of entities (typically points or polylines),      *
* find ad merge closest ones as possible and in place.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   GeomEntities:    Geometry to merge, typically points or polylines, as a  *
*		     vector of reference pointers to the entities.           *
*		       Returned, merged, data will also be stored here.      *
*   NumOfGEntities:  Number of geometric entities.  Also length of	     *
*		     GeomEntities.  					     *
*   Eps:	     Epsilon of similarity to merge entities at.  Entities   *
*		     farther than Eps will not be merged.                    *
*   IdenticalEps:    Epsilon to consider two entities' distance as zero.     *
*   InitFinc:        A function to initialize all geometry.  NULL for the    *
*		     initialization of IPPolygonStruct polylines.	     *
*   DistSqrFunc:     A distance computation function. NULL for two	     *
*		     IPPolygonStruct polylines compare.  This function       *
*		     computes the minimal distance squared between two	     *
*		     entities.						     *
*   KeyFunc:	     A function to return a Key to sort the entities so      *
*		     that similar/adjacent entities will get similar a Key.  *
*   MergeFunc:       A merge function. NULL to merge two IPPolygonStructs    *
*		     polylines merge.  This function returns a merged entity *
*		     while destroying the two input entities.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	     Number of merged entities in the end or 0 if error.     *
*****************************************************************************/
static int GMMergeGeometryAux(void **GeomEntities,
			      int NumOfGEntities,
			      IrtRType Eps,
			      IrtRType IdenticalEps,
			      GMMergeGeomInitFuncType InitFunc,
			      GMMergeGeomDistFuncType DistSqrFunc,
			      GMMergeGeomKeyFuncType KeyFunc,
			      GMMergeGeomMergeFuncType MergeFunc)
{
    int i, j, N;
    IrtRType
	EpsSqr = IRIT_SQR(Eps),
        IdenticalEpsSqr = IRIT_SQR(IdenticalEps);
    GMMergeEntitiesStruct *GE;

    if (NumOfGEntities <= 1)
        return NumOfGEntities;

    if (InitFunc == NULL)
        InitFunc = GMMergeGeomInitPolys;
    if (DistSqrFunc == NULL)
        DistSqrFunc = GMMergeGeomDistSqr2Polys;
    if (KeyFunc == NULL)
        KeyFunc = GMMergeGeomKeyPoly;
    if (MergeFunc == NULL)
        MergeFunc = GMMergeGeomMerge2Polys;

    /* Merge, as possible, segments that are close to within IdenticalEps. */
    do {
        NumOfGEntities = GMMergeSameGeometry(GeomEntities, i = NumOfGEntities,
					     IdenticalEps, InitFunc,
					     DistSqrFunc, KeyFunc, MergeFunc);
    }
    while (i > NumOfGEntities * 1.2);
    if (NumOfGEntities <= 1)
        return NumOfGEntities;

    GE = (GMMergeEntitiesStruct *)
	IritMalloc(sizeof(GMMergeEntitiesStruct) * NumOfGEntities);

    /* Initialize all geometry. */
    N = NumOfGEntities;
    for (i = 0; i < N; i++)
	GMMergeGeomInitPolys(GeomEntities[i]);

    /* Entering main iteration loop: */
    do {
        NumOfGEntities = N;

        /* Initialize the data structure. */
        for (i = 0; i < N; i++) {
	    GE[i].Entity = i;
	    GE[i].MinDistSqr = IRIT_INFNTY;
	    GE[i].MinDistSqrEntity = -1;
	    GE[i].Valid = TRUE;
	}

        /* Compute minima distance between entity i and all other entities. */
        for (i = 0; i < N; i++) {
	    for (j = i + 1; j < N; j++) {
	        IrtRType
		    Dst = DistSqrFunc(GeomEntities[i], GeomEntities[j]);

		if (GE[i].MinDistSqr > Dst) {
		    GE[i].MinDistSqr = Dst;
		    GE[i].MinDistSqrEntity = j;
		}
		if (GE[j].MinDistSqr > Dst) {
		    GE[j].MinDistSqr = Dst;
		    GE[j].MinDistSqrEntity = i;
		}
	    }
	}

	/* Merge pairs of closest entities. A merged entity will not be     */
	/* remerged in the same cycle.					    */
	for (i = 0; i < N; i++) {
	    /* Examine the validity of this pair. Further, also examine if  */
	    /* the distance is virtually zero (common in chaining precise   */
	    /* data sets) or they both see each other as their closests.    */
	    j = GE[i].MinDistSqrEntity;
	    if (j >= 0 &&
		GE[i].Valid &&
		GE[j].Valid &&
		(GE[i].MinDistSqr < IdenticalEpsSqr ||
		 (GE[j].MinDistSqrEntity == i &&
		  GE[i].MinDistSqr <= EpsSqr))) {
	        MergeFunc(&GeomEntities[GE[i].Entity],
			  &GeomEntities[GE[j].Entity]);
		GE[i].Valid = GE[j].Valid = FALSE;
	    }
	}

	/* Clean the vector from NULL entities. */
	for (j = N - 1; GeomEntities[j] == NULL; N = j--);
	for (i = 0; i < j; i++) {
	    if (GeomEntities[i] == NULL) {
		GeomEntities[i] = GeomEntities[N = j--];
		for ( ; GeomEntities[j] == NULL; N = j--);
	    }
	}

#       ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMergeGeometry, FALSE) {
	        IRIT_INFO_MSG_PRINTF("%d input entites, merged into %d entities\n",
				     NumOfGEntities, N);
	    }
	}
#       endif /* DEBUG */
    }
    while (NumOfGEntities != N);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMergeGeometry, FALSE) {
	    IRIT_INFO_MSG_PRINTF("%d input entites, merged into %d entities\n",
				 NumOfGEntities, N);
	    for (i = 0; i < N; i++) {
	        IPPolygonStruct
		    *Pl = (IPPolygonStruct *) GeomEntities[i];
		IPVertexStruct
		    *V1 = Pl -> PVertex,
		    *V2 = IPGetLastVrtx(V1);

		IRIT_INFO_MSG_PRINTF(
			"Poly %d [%.8lg %.8lg %.8lg] [%.8lg %.8lg %.8lg]\n", i,
			V1 -> Coord[0], V1 -> Coord[1], V1 -> Coord[2],
			V2 -> Coord[0], V2 -> Coord[1], V2 -> Coord[2]);
	    }
	}
    }
#   endif /* DEBUG */

    IritFree(GE);

    return N;
}
