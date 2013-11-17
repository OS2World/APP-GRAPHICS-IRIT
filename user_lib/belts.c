/******************************************************************************
* belts.c - constructs crvs of prescribed offsets around a sequence of circs. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, January 2010.				      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "trim_lib.h"
#include "user_loc.h"

/* #define DEBUG_DUMP_BELT */

static IPObjectStruct *UserBeltBoundingArcs(CagdCrvStruct *BeltCrvs, 
					    IrtRType BoundingArcs,
					    int Right);
static CagdCrvStruct *UserBeltOffset(CagdCrvStruct *Belt, CagdRType Offset);
static CagdCrvStruct *UserBeltCreateArc(const CagdPtStruct *Start,
					const CagdPtStruct *Center,
					const CagdPtStruct *End,
					IrtRType Radius,
					IrtRType BeltThickness);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Builds a belt-curve formed out of a pair of sequences of lines/arcs      M
* around the given set of circles (pulleys), in order.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Circs:           A sequence of circles (pulleys), each as (x, y, r).     M
*                    If r is positive is it a CW pulley, otherwise CCW.      M
*   BeltThickness:   The thickness of the constructed belt.                  M
*   BoundingArcs:    If non zero, bounding arcs are computed for each linear M
*                    segment in belt, for each of the two sides of the belt. M
*   ReturnCrvs:      TRUE to simply return two closed curves with the left   M
*                    and right sides of the belt.  FALSE to return a list    M
*                    with the individual arcs and lines and their attributes.M
*   Intersects:      TRUE if left and right sides intersects.                M
*   Error:           If not set to NULL, holds an error description.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Two lists of lines/arcs representing the two sides   M
*                    the belt around the given circles (pulleys).  If        M
*		     BoundingArcs is positive, two additional lists of arcs, M
*                    each bounding a line segment in the belt, are build.    M
8                       The bounding arc is expanding the belt's domain with M
8                    at most distance BoundingArc, from the linear segment.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserBeltCreate                                                           M
*****************************************************************************/
IPObjectStruct *UserBeltCreate(IPVertexStruct *Circs,
			       IrtRType BeltThickness,
			       IrtRType BoundingArcs,
			       int ReturnCrvs,
			       int *Intersects,
			       const char **Error)
{
    int HasLastLinSeg = FALSE;
    IrtRType Orient, Rad1, Rad2;
    CagdPtStruct LastLinSegPt, FirstLinSegPt, Pt1, Pt2, Cntr1, Cntr2;
    CagdCrvStruct *Crv,
        *MergedLeft = NULL,
        *MergedRight = NULL,
	*Left = NULL,
	*Right = NULL;
    IPVertexStruct *V2,
        *V = Circs;
    IPObjectStruct *PObjs;

    *Error = NULL;
    *Intersects = FALSE;

    /* Needs at least two pulleys. */
    if (V == NULL || V -> Pnext == NULL) {
	*Error = "Not enough pulleys specified (at least two are needed).";
        return NULL;
    }

    Cntr1.Pt[2] = Cntr2.Pt[2] =
	Pt1.Pt[2] = Pt2.Pt[2] =
	    FirstLinSegPt.Pt[2] = LastLinSegPt.Pt[2] = 0.0;

    /* GO over all pulleys and generate lines and arcs of the right side. */
    do {
        IrtPtType TanPts[2][2];
	IrtVecType Vec1, Vec2;

        V2  = V -> Pnext != NULL ? V -> Pnext : Circs;

	/* Compute the two inner or outer tangents between V and V2. */
	IRIT_PT2D_COPY(Cntr1.Pt, V -> Coord);
	IRIT_PT2D_COPY(Cntr2.Pt, V2 -> Coord);
	Rad1 = V -> Coord[2];
	Rad2 = V2 -> Coord[2];
	if (!GM2BiTansFromCircCirc(Cntr1.Pt,
				   Rad1 < 0.0 ? -Rad1 + BeltThickness : Rad1,
				   Cntr2.Pt,
				   Rad2 < 0.0 ? -Rad2 + BeltThickness : Rad2,
				   IRIT_SIGN(Rad1) == IRIT_SIGN(Rad2),
				   TanPts)) {
	    CagdCrvFreeList(Right);
	    *Error = "Failed to compute a bitangent to two circles (concentric circles?).";
	    return NULL;
	}

	/* Decide which of the two tangents to exploit. */
	IRIT_VEC2D_SUB(Vec1, Cntr1.Pt, TanPts[0][0]);
	IRIT_VEC2D_SUB(Vec2, TanPts[0][0], TanPts[0][1]);
	Orient = IRIT_CROSS_PROD_2D(Vec2, Vec1);
	if (IRIT_SIGN(Orient) == IRIT_SIGN(Rad1)) {
	    IRIT_PT2D_COPY(Pt1.Pt, TanPts[0][0]);
	    IRIT_PT2D_COPY(Pt2.Pt, TanPts[0][1]);
	}
	else {
	    IRIT_PT2D_COPY(Pt1.Pt, TanPts[1][0]);
	    IRIT_PT2D_COPY(Pt2.Pt, TanPts[1][1]);
	}

	/* Create an arc of the desired radius between last linear segment  */
	/* and this one, if had a last linear segment.			    */
	if (HasLastLinSeg) {
	    if ((Crv = UserBeltCreateArc(&LastLinSegPt, &Cntr1, &Pt1,
					 Rad1, BeltThickness)) == NULL) {
	        CagdCrvFreeList(Right);
		*Error = "Failed to create an arc.";
		return NULL;
	    }
	    IRIT_LIST_PUSH(Crv, Right);
	}
	else {
	    IRIT_PT2D_COPY(FirstLinSegPt.Pt, Pt1.Pt);
	}
	HasLastLinSeg = TRUE;
	IRIT_PT2D_COPY(LastLinSegPt.Pt, Pt2.Pt);

	/* Construct the bi-tangent line and push into curves' list. */
	Crv = CagdMergePtPt(&Pt1, &Pt2);
	IRIT_LIST_PUSH(Crv, Right);

#ifdef DEBUG_DUMP_BELT
	{
	    CagdPtStruct Cntr;
	    
	    /* Dump a circle at Z = -1 (also mem. leak...). */
	    Cntr.Pt[0] = V -> Coord[0];
	    Cntr.Pt[1] = V -> Coord[1];
	    Cntr.Pt[2] = -1.0;
	    IPStdoutObject(IPGenCRVObject(BspCrvCreateCircle(&Cntr,
							     V -> Coord[2])),
			   FALSE);
	}
#endif /* DEBUG_DUMP_BELT */

        V = V2;
    }
    while (V != Circs);
    
    /* Constructs the arc from last pulley to first, closing the loop. */
    if ((Crv = UserBeltCreateArc(&LastLinSegPt, &Cntr2, &FirstLinSegPt,
			         Rad2, BeltThickness)) == NULL) {
        CagdCrvFreeList(Right);
	*Error = "Failed to create an arc.";
	return NULL;
    }
    IRIT_LIST_PUSH(Crv, Right);

    /* Reverse the curves' list to the correct order. */
    Right = CagdListReverse(Right);

    if (BeltThickness < IRIT_UEPS) {
	Left = CagdCrvCopyList(Right);
    }
    else if ((Left = UserBeltOffset(Right, BeltThickness)) == NULL) {
        CagdCrvFreeList(Right);
	*Error = "Failed to compute offset.";
	return NULL;
    }
    
#ifdef DEBUG_DUMP_BELT
    /* Dump the curves (also mem. leak...). */
    printf("Debug right *************************************************:\n");
    IPStdoutObject(IPGenCRVObject(Right), FALSE);
    printf("Debug left *************************************************:\n");
    IPStdoutObject(IPGenCRVObject(Left), FALSE);
#endif /* DEBUG_DUMP_BELT */

    /* process the result into two list-objects, in a list object. */
    if (ReturnCrvs) {
        PObjs = IPGenLISTObject(
	      IPGenCRVObject(MergedLeft = CagdMergeCrvList(Left, FALSE)));
	IPListObjectAppend(PObjs,
	      IPGenCRVObject(MergedRight = CagdMergeCrvList(Right, FALSE)));
    }
    else {
        PObjs = IPGenLISTObject(IPLnkListToListObject(Left, IP_OBJ_CURVE));
	IPListObjectAppend(PObjs, IPLnkListToListObject(Right, IP_OBJ_CURVE));
    }

    if (BoundingArcs > 0.0) {
        /* Constructs bounding arcs for the linear segments. */
        IPListObjectAppend(PObjs, UserBeltBoundingArcs(Right, BoundingArcs,
						       TRUE));
        IPListObjectAppend(PObjs, UserBeltBoundingArcs(Left, BoundingArcs,
						       FALSE));
    }

    /* Test if belt self intersects by examining if left & right intersect. */
    if (BeltThickness > IRIT_UEPS) {
	CagdPtStruct *Pts;

        if (MergedLeft != NULL && MergedRight != NULL) {
	    Pts = CagdCrvCrvInter(MergedLeft, MergedRight, IRIT_EPS);
	}
	else {
	    MergedLeft = CagdMergeCrvList(Left, FALSE);
	    MergedRight = CagdMergeCrvList(Right, FALSE);

	    Pts = CagdCrvCrvInter(MergedLeft, MergedRight, IRIT_EPS);

	    CagdCrvFree(MergedLeft);
	    CagdCrvFree(MergedRight);
	}

	if (Pts != NULL) {
	    *Intersects = TRUE;
	    CagdPtFreeList(Pts);
	}
    }
        
    if (ReturnCrvs) {
	CagdCrvFreeList(Left);
	CagdCrvFreeList(Right);
    }

    return PObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Build arcs to bound the linear segments curves of the belt.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   BeltCrvs:      List of arcs and lines forming the belt.                  *
*   BoundingArcs:  The middle distance for the arc off the line segment.     *
*   Right:         TRUE for right side of the belt, FALSE for left side.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A list objects holding the bounding arcs.             *
*****************************************************************************/
static IPObjectStruct *UserBeltBoundingArcs(CagdCrvStruct *BeltCrvs, 
					    IrtRType BoundingArcs,
					    int Right)
{
    CagdCrvStruct *Crv, *ArcCrv,
        *BoundingArcsList = NULL;

    for (Crv = BeltCrvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdRType L2, Rad;
        CagdPType PtMid;
        CagdPtStruct Pt1, Pt2, Center;
	CagdVType Vec;

        if (Crv -> Order != 2)
	    continue;

	/* Crv holds a linear segment - construct a bounding arc for it. */

	/* Compute a mid point and a vector in the XY plane orthogonal to */
	/* Pt1-Pt2 and in the proper right/left orientation.              */
	CagdCoerceToE2(Pt1.Pt, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE2(Pt2.Pt, Crv -> Points, 1, Crv -> PType);
	IRIT_PT2D_BLEND(PtMid, Pt1.Pt, Pt2.Pt, 0.5);

	if (Right) {
	    Vec[0] = Pt1.Pt[1] - Pt2.Pt[1];
	    Vec[1] = Pt2.Pt[0] - Pt1.Pt[0];
	}
	else {
	    Vec[0] = Pt2.Pt[1] - Pt1.Pt[1];
	    Vec[1] = Pt1.Pt[0] - Pt2.Pt[0];
	}
	Vec[2] = 0.0;
	IRIT_VEC2D_NORMALIZE(Vec);

	/* Computes the arcs' center. */
	L2 = IRIT_PT2D_DIST_SQR(PtMid, Pt1.Pt);
	Rad = (L2 + IRIT_SQR(BoundingArcs)) / (2 * BoundingArcs);
	IRIT_VEC2D_SCALE(Vec, Rad - BoundingArcs);
	IRIT_PT2D_ADD(Center.Pt, Vec, PtMid);

	Pt1.Pt[2] = Pt2.Pt[2] = Center.Pt[2] = 0.0;
	if (Right)
	    ArcCrv = CagdCrvCreateArcCCW(&Pt1, &Center, &Pt2);
	else
	    ArcCrv = CagdCrvCreateArcCW(&Pt1, &Center, &Pt2);
	IRIT_LIST_PUSH(ArcCrv, BoundingArcsList);
    }

    return IPLnkListToListObject(CagdListReverse(BoundingArcsList),
				 IP_OBJ_CURVE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes an offset to a belt - a list of line segments and arcs.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Belt:   A list of lines and arcs form a closed loop belt.                *
*   Offset: Offset amount to compute.  Positive only (left to given curve    *
*           when moving forward).					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Computed offset.  Also a list of lines and arcs.      *
*****************************************************************************/
static CagdCrvStruct *UserBeltOffset(CagdCrvStruct *Belt, CagdRType Offset)
{
    CagdCrvStruct *Crv, *TCrv,
	*OffBelt = NULL;

    if (Offset <= 0)
        return CagdCrvCopyList(Belt);

    for (Crv = Belt; Crv != NULL; Crv = Crv -> Pnext) {
        IrtVecType Vec1;

        if (Crv -> Order == 2) {
	    /* It is a linear segment.  Compute the offset vector and      */
	    /* create an offset linear segment.				   */
	    Vec1[0] = Crv -> Points[2][0] - Crv -> Points[2][1];
	    Vec1[1] = Crv -> Points[1][1] - Crv -> Points[1][0];
	    Vec1[2] = 0.0;
	    IRIT_VEC2D_NORMALIZE(Vec1);
	    IRIT_VEC2D_SCALE(Vec1, Offset);

	    TCrv = CagdCrvCopy(Crv);
	    TCrv -> Points[1][0] += Vec1[0];
	    TCrv -> Points[2][0] += Vec1[1];
	    TCrv -> Points[1][1] += Vec1[0];
	    TCrv -> Points[2][1] += Vec1[1];
	}
	else {
	    IrtRType
	        Radius = AttrGetRealAttrib(Crv -> Attr, "Radius");
	    IPObjectStruct
	        *CntrObj = AttrGetObjAttrib(Crv -> Attr, "Center");
	    IrtHmgnMatType Mat1, Mat2, Mat;

	    assert(!IP_ATTR_IS_BAD_REAL(Radius) &&
		   CntrObj != NULL &&
		   Crv -> Order == 3);                 /* A quadratic arc. */

	    /* Scale arc around its center so it will be in right offset.  */
	    MatGenMatTrans(-CntrObj -> U.Pt[0], -CntrObj -> U.Pt[1], 0.0,
			   Mat1);
	    if (Radius > 0.0)                 /* CW - increase the radius. */
	        MatGenMatUnifScale((Radius + Offset) / Radius, Mat2);
	    else {			       /* CCW - shrink the radius. */
	        MatGenMatUnifScale(-Radius / (-Radius + Offset), Mat2);
	    }
	    MatMultTwo4by4(Mat, Mat1, Mat2);

	    MatGenMatTrans(CntrObj -> U.Pt[0], CntrObj -> U.Pt[1], 0.0,
			   Mat1);
    	    MatMultTwo4by4(Mat, Mat, Mat1);

	    TCrv = CagdCrvMatTransform(Crv, Mat);
	}
	IRIT_LIST_PUSH(TCrv, OffBelt);
    }

    return CagdListReverse(OffBelt);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs the desired arc from Start to End with center Center.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Start:   Starting location of arc.					     *
*   Center:  Center location of arc.					     *
*   End:     End location of arc.					     *
*   Radius:  Radius of arc.  Positive for a CW arc, negative for a CCW arc.  *
*   BeltThickness:   The thickness of the constructed belt.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Constructed arc or NULL if error.                      *
*****************************************************************************/
static CagdCrvStruct *UserBeltCreateArc(const CagdPtStruct *Start,
					const CagdPtStruct *Center,
					const CagdPtStruct *End,
					IrtRType Radius,
					IrtRType BeltThickness)
{
    IrtRType OriginalRadius = Radius;
    CagdCrvStruct *Arc;

    if (Radius > 0.0) {
        Arc = CagdCrvCreateArcCW(Start, Center, End);
    }
    else if (Radius == 0) {
	return NULL;
    }
    else {
        Arc = CagdCrvCreateArcCCW(Start, Center, End);
	Radius = BeltThickness - Radius;
    }

    assert(IRIT_APX_EQ(Radius, IRIT_PT2D_DIST(Start -> Pt, Center -> Pt)) &&
	   IRIT_APX_EQ(Radius, IRIT_PT2D_DIST(End -> Pt, Center -> Pt)));
    AttrSetRealAttrib(&Arc -> Attr, "Radius", OriginalRadius);

    AttrSetObjAttrib(&Arc -> Attr, "Center",
		     IPGenPTObject(&Center -> Pt[0],
				   &Center -> Pt[1],
				   &Center -> Pt[2]), FALSE);

    return Arc;
}
