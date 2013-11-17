/******************************************************************************
* CnvxHull.c -  convex hull computations.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec 1987.					      *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"

#define GM_CH_EXTRA_STACK	100		  /* Spare number of points. */
#define GM_CH_COLLINEAR_EPS	1e-10

IRIT_STATIC_DATA IrtRType Ymin, Xmin; /* Global min Y point used by CompareAngle. */
IRIT_STATIC_DATA int *LocalStack,
    GlblCHError = FALSE,
    StackPointer = 0;

static int IsConvex(IrtRType p1x,
		    IrtRType p1y,
		    IrtRType p2x,
		    IrtRType p2y,
		    IrtRType p3x,
		    IrtRType p3y);
static void ResetLocalStack(void);
static void PushIndex(int i);
static int PopIndex(void);
#if defined(ultrix) && defined(mips)
static int CompareAngle(VoidPtr Pt1, VoidPtr Pt2);
#else
static int CompareAngle(const VoidPtr Pt1, const VoidPtr Pt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convex Hull computation of a set of points. The Convex Hull is returned  M
* in place, updating NumOfPoints.					     M
* Algorithm is based on two articles:                                        M
* 1. An Efficient Algorithm For Determining The convex Hull of a Finite Set. V
*    By R.L. Graham, Information processing letters (1972) 132-133.          V
* 2. A Reevolution of an Efficient Algorithm For Determining The Convex Hull V
*    of a Finite Planar Set. By K.R. Anderson, Information Processing        V
*    Letters, January 1978, Vol. 7, Num. 1, 53-55.                           V
*                                                                            *
* PARAMETERS:                                                                M
*   DTPts:           The set of point to compute their convex hull.          M
*   NumOfPoints:     Number of points in set DTPts.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMonotonePolyConvex                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMConvexHull                                                             M
*****************************************************************************/
int GMConvexHull(IrtE2PtStruct *DTPts, int *NumOfPoints)
{
    int i, j, YminIndex, Index1, Index2, Index3, CHIndex;
    IrtRType p1x, p1y, p2x, p2y, p3x, p3y;
    IrtE2PtStruct *CHPts;

    if (*NumOfPoints < 4) /* 3 or less points - must all be on convex hull. */
	return TRUE;

    LocalStack = IritMalloc((GM_CH_EXTRA_STACK + *NumOfPoints) * sizeof(int));

    Xmin = DTPts[0].Pt[0];
    Ymin = DTPts[0].Pt[1];		                 /* Find Y minimum: */
    YminIndex = 0;
    for (i = 1; i < *NumOfPoints; i++)
        if (Ymin >= DTPts[i].Pt[1]) {
            /* If Y levels are the same pick the one leftmost on X axis: */
            if (Ymin == DTPts[i].Pt[1] && Xmin < DTPts[i].Pt[0])
	        continue;
            Xmin = DTPts[i].Pt[0];
            Ymin = DTPts[i].Pt[1];
	    YminIndex = i;
        }

    /* Make sure first point in array is at Y minimum. */
    IRIT_SWAP(IrtRType, DTPts[0].Pt[0], DTPts[YminIndex].Pt[0]);
    IRIT_SWAP(IrtRType, DTPts[0].Pt[1], DTPts[YminIndex].Pt[1]);

    /* Sort all the other elements but the first one (with Ymin) according  */
    /* to angle (0..180) relative to Ymin point:			    */
    qsort(&DTPts[1], (*NumOfPoints) - 1,
	  sizeof(IrtE2PtStruct), CompareAngle);

    /* Eliminate duplicated points. */
    for (i = j = 1; i < *NumOfPoints; ) {
        IrtRType
	    Dx = DTPts[i].Pt[0] - DTPts[0].Pt[0],
	    Dy = DTPts[i].Pt[1] - DTPts[0].Pt[1],
	    LastDx = DTPts[j - 1].Pt[0] - DTPts[0].Pt[0],
	    LastDy = DTPts[j - 1].Pt[1] - DTPts[0].Pt[1];

        if ((IRIT_APX_EQ_EPS(Dx, 0.0, IRIT_UEPS) &&
	     IRIT_APX_EQ_EPS(Dy, 0.0, IRIT_UEPS))) {
	    /* Similar point - do not copy. */
	    i++;
	}
	else if (j > 1 &&
		 IRIT_APX_EQ_EPS(LastDy * Dx, Dy * LastDx, GM_CH_COLLINEAR_EPS)) {
	    /* Collinear points - keep furthest point only. */
	    if (IRIT_FABS(Dx) + IRIT_FABS(Dy) >
		IRIT_FABS(LastDx) + IRIT_FABS(LastDy)) {
	        DTPts[j - 1].Pt[0] = DTPts[i].Pt[0];
		DTPts[j - 1].Pt[1] = DTPts[i].Pt[1];
	    }
    	    i++;
	}
	else {
	    DTPts[j].Pt[0] = DTPts[i].Pt[0];
	    DTPts[j].Pt[1] = DTPts[i].Pt[1];
	    i++;
	    j++;
	}
    }
    *NumOfPoints = j;

    CHPts = (IrtE2PtStruct *)		   /* Allocate mem. for CH. */
	IritMalloc(sizeof(IrtE2PtStruct) * *NumOfPoints);

    /* Now the points are sorted by angle, so we can scan them and drop non */
    /* convex ones (by using 3 "running" points and testing middles angle). */
    p1x = DTPts[0].Pt[0];
    p1y = DTPts[0].Pt[1];
    p2x = DTPts[1].Pt[0];
    p2y = DTPts[1].Pt[1];
    p3x = DTPts[2].Pt[0];
    p3y = DTPts[2].Pt[1];

    Index1 = 0;  /* Indices of points.*/
    Index2 = 1;
    Index3 = 2;
    CHIndex = 0;

    ResetLocalStack();
    GlblCHError = FALSE;

    while (Index1 < *NumOfPoints) {
	if (IsConvex(p1x, p1y, p2x, p2y, p3x, p3y)) {         /* Go forward. */
	    CHPts[CHIndex].Pt[0] = p1x;
	    CHPts[CHIndex++].Pt[1] = p1y;
	    PushIndex(Index1);     /* Save index, we might need that ... */
	    p1x = p2x;
	    p1y = p2y;
	    Index1 = Index2;
	    p2x = p3x;
	    p2y = p3y;
	    Index2 = Index3++;
	    p3x = DTPts[Index3 % *NumOfPoints].Pt[0];
	    p3y = DTPts[Index3 % *NumOfPoints].Pt[1];
	}
        else if (CHIndex == 0) {	     /* First location - cannot pop! */
	    p2x = p3x;
	    p2y = p3y;
	    Index2 = Index3++;
            p3x = DTPts[Index3 % *NumOfPoints].Pt[0];
            p3y = DTPts[Index3 % *NumOfPoints].Pt[1];
	}
	else {    /* Go backward until convex corner, using last pushed pos. */
	    p2x = p1x;
	    p2y = p1y;
	    Index2 = Index1;
	    Index1 = PopIndex();
	    CHIndex--;
	    p1x = DTPts[Index1].Pt[0];
	    p1y = DTPts[Index1].Pt[1];
	}

	if (GlblCHError) {
	    IritFree(LocalStack);
	    return FALSE;
	}
    }

    IRIT_GEN_COPY(DTPts, CHPts, sizeof(IrtE2PtStruct) * CHIndex);
    *NumOfPoints = CHIndex;
    IritFree(CHPts);

    IritFree(LocalStack);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to determine if given 3 points form convex angle using cross     *
* prod.									     *
*                                                                            *
* PARAMETERS:                                                                *
*   p1x, p1y:  First point.						     *
*   p2x, p2y:  Second point.                                                 *
*   p3x, p3y:  Third point.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if convex, FALSE otherwise.                               *
*****************************************************************************/
static int IsConvex(IrtRType p1x,
		    IrtRType p1y,
		    IrtRType p2x,
		    IrtRType p2y,
		    IrtRType p3x,
		    IrtRType p3y)

{
    return (p1x - p2x) * (p2y - p3y) - (p2x - p3x) * (p1y - p2y)
							    <= IRIT_SQR(IRIT_UEPS);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Resetsthe local stack so ConvexHull routine might use it.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ResetLocalStack(void)
{
    StackPointer = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Push one integer on local stack, assuming there is no overflow.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   i:      To push.                                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PushIndex(int i)
{
    LocalStack[StackPointer++] = i;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Pop one integer from local stack, assuming the stack never empty.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       Poped value                                                   *
*****************************************************************************/
static int PopIndex(void)
{
    if (StackPointer <= 0) {
        GEOM_FATAL_ERROR(GEOM_ERR_CH_STACK_UNDERFLOW);
	GlblCHError = TRUE;
	return 0;
    }
    return LocalStack[--StackPointer];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compares two angles of two points relative to Xmin,Ymin point.	     *
*   This routine is used by the qsort routine in ConvexHull routine.         *
* Algorithm: -IRIT_SIGN(cos(teta)) * cos(teta)^2, see second reference, page 54.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2:   Two points to compare their angles.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0 or 1 based on the angle ratios of the two points.        *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int CompareAngle(VoidPtr Pt1, VoidPtr Pt2)
#else
static int CompareAngle(const VoidPtr Pt1, const VoidPtr Pt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType Dx, Value1, Value2,
	*Point1 = (IrtRType *) Pt1,
	*Point2 = (IrtRType *) Pt2;

    if ((IRIT_FABS(Point1[0] - Xmin) < IRIT_UEPS) &&
        (IRIT_FABS(Point1[1] - Ymin) < IRIT_UEPS))
	return -1; /* Make Min first */
    if ((IRIT_FABS(Point2[0] - Xmin) < IRIT_UEPS) &&
        (IRIT_FABS(Point2[1] - Ymin) < IRIT_UEPS))
	return 1; /* Make Min first */

    Dx = Point1[0] - Xmin;
    Value1 = IRIT_SQR(Dx) * (-IRIT_SIGN(Dx)) /
				   (IRIT_SQR(Dx) + IRIT_SQR(Point1[1] - Ymin));

    Dx = Point2[0] - Xmin;
    Value2 = IRIT_SQR(Dx) * (-IRIT_SIGN(Dx)) /
				   (IRIT_SQR(Dx) + IRIT_SQR(Point2[1] - Ymin));

    if (Value1 > Value2 + IRIT_UEPS)
        return -1;
    else if (Value1 < Value2 - IRIT_UEPS)
        return 1;
    else
        return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the convex (concave) envelope of an X-monotone shape, in the XY  M
* plane, by purging all concave (convex) vertices in the sequence, in place. M
*                                                                            *
* PARAMETERS:                                                                M
*   VHead:    An X-monotone polyline to make into a convex (concave) one.    M
*             Note the first and last vertices will always be in the output  M
*	      sequence.							     M
*   Cnvx:     TRUE to make the vertex sequence Vhead convex, FALSE to make   M
*	      it concave, in the functional sense.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if succesful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvexHull                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMonotonePolyConvex                                                     M
*****************************************************************************/
int GMMonotonePolyConvex(IPVertexStruct *VHead, int Cnvx)
{
    int PurgeThis, OnePurged;

    /* Note we iterate until nothing is purged as when a vertex is purged, */
    /* one should do back tracking.  We simply re-iterate then which is    */
    /* less efficient but simpler...					   */
    do {
        IrtVecType V1, V2;
        IPVertexStruct *VNext, *V,
	    *VPrev = VHead;

	OnePurged = FALSE;

	/* Make sure we have at least 3 points. */
        if ((V = VHead -> Pnext) == NULL || V -> Pnext == NULL)
	    return TRUE;

	IRIT_VEC2D_SUB(V1, V -> Coord, VPrev -> Coord);

	VNext = V -> Pnext;

	while (VNext != NULL) {
	    /* Examine if the trio (VPrev, V, VNext) fail to satisfy        */
	    /* convexity (concavity) and if so purge V.			    */
	    IRIT_VEC2D_SUB(V2, VNext -> Coord, V -> Coord);

	    if (Cnvx)
	        PurgeThis = IRIT_CROSS_PROD_2D(V1, V2) >= 0;
	    else
	        PurgeThis = IRIT_CROSS_PROD_2D(V1, V2) <= 0;

	    if (PurgeThis) {
	        /* Purge V. */
		IPFreeVertex(V);
		VPrev -> Pnext = V = VNext;
		IRIT_VEC2D_SUB(V1, V -> Coord, VPrev -> Coord);
		OnePurged = TRUE;
	    }
	    else {
	        VPrev = V;
		V = VNext;
		IRIT_VEC2D_COPY(V1, V2);
	    }

	    VNext = V -> Pnext;
	}
    }
    while (OnePurged);

    return TRUE;
}
