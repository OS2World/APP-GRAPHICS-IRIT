/******************************************************************************
* CagdCMrg.c - Curve/Point merging routines.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 91.					      *
******************************************************************************/

#include "string.h"
#include "cagd_loc.h"
#include "geom_lib.h"

#define CAGD_MERGE_MIN_DIST 1e-2

static void CopyCrvOnCrv(CagdCrvStruct *DestCrv,
			 int Index,
			 const CagdCrvStruct *SrcCrv);
static void InterpolateLinearSeg(CagdCrvStruct *Crv, int Index1, int Index2);
static CagdCrvStruct *ConvertToProperCrv(const CagdCrvStruct *Crv,
					 CagdBType *NewCrv,
					 CagdRType *CrvArcLen,
					 CagdRType *CrvDomainLen);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two curves by connecting the end of Crv1 to the beginning of Crv2.  M
*  If the end of Crv1 is identical to the beginning of Crv2 then the result  M
* is as expected. However, if the curves do not meet, their end points are   M
* linearly interpolated if InterpolateDiscont is TRUE or simply blended out  M
* in a freeform shape if InterpolateDiscont is FALSE.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1:                To connect to Crv1's starting location at its end. M
*   CCrv2:                To connect to Crv2's end location at its start.    M
*   InterpolateDiscont:   If TRUE, linearly interpolate discontinuity.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtPt, CagdMergePtCrv, CagdMergeCrvPt,      M
*   CagdMergeCrvList							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeCrvCrv, merge                                                   M
*****************************************************************************/
CagdCrvStruct *CagdMergeCrvCrv(const CagdCrvStruct *CCrv1,
			       const CagdCrvStruct *CCrv2,
			       int InterpolateDiscont)
{
    CagdBType CrvsSharePt, Crv1New, Crv2New;
    int Length, Order, Len1, Len2;
    CagdRType E3Pt1[3], E3Pt2[3], Crv1ArcLen, Crv2ArcLen, DistPtPt,
	Crv1DomainLen, Crv2DomainLen, MergeStep;
    CagdPointType CrvPType;
    CagdCrvStruct *Crv, *Crv1, *Crv2;

    Crv1 = ConvertToProperCrv(CCrv1, &Crv1New, &Crv1ArcLen, &Crv1DomainLen);
    Crv2 = ConvertToProperCrv(CCrv2, &Crv2New, &Crv2ArcLen, &Crv2DomainLen);
    if (!Crv1New)
        Crv1 = CagdCrvCopy(CCrv1);
    if (!Crv2New)
        Crv2 = CagdCrvCopy(CCrv2);

    CagdMakeCrvsCompatible(&Crv1, &Crv2, TRUE, FALSE);

    Order = Crv1 -> Order;
    Len1 = Crv1 -> Length;
    Len2 = Crv2 -> Length;

    /* Compute curve point types. */
    CrvPType = Crv1 -> PType;

    /* Figure out if end point of Crv1 is equal to start point of Crv2 and   */
    /* Compute the length of the resulting curve accordingly.		     */
    /* If the same point, then the result can omit one copy and therefore    */
    /* has Len1 + Len2 - 1 Ctl points. If however a linear segment should be */
    /* introduced between the two curves, it should hold Order collinear pts */
    /* including 2 end points shared with curves or Order - 2 new pts.       */
    CagdCoerceToE3(E3Pt1, Crv1 -> Points, Len1 - 1, Crv1 -> PType);
    CagdCoerceToE3(E3Pt2, Crv2 -> Points, 0, Crv2 -> PType);
    if ((DistPtPt = IRIT_PT_PT_DIST(E3Pt1, E3Pt2)) < CAGD_MERGE_MIN_DIST)
        DistPtPt = CAGD_MERGE_MIN_DIST;
    CrvsSharePt = Order > 1 && IRIT_PT_APX_EQ(E3Pt1, E3Pt2);
    Length = CrvsSharePt ? Len1 + Len2 - 1
			 : InterpolateDiscont ? Len1 + Len2 + Order - 2
					      : Len1 + Len2;

    Crv = BspCrvNew(Length, Order, CrvPType);
    CopyCrvOnCrv(Crv, 0, Crv1);
    CopyCrvOnCrv(Crv, Length - Len2, Crv2);
    InterpolateLinearSeg(Crv, Len1 - 1, Length - Len2);

    MergeStep = (Crv1DomainLen + Crv2DomainLen) * DistPtPt /
						(Crv1ArcLen + Crv2ArcLen);

    /* Update the knot vector. We assume open end condition here... */
    CAGD_GEN_COPY(Crv -> KnotVector, Crv1 -> KnotVector,
		  (Len1 + Order - 1) * sizeof(CagdRType));
    if (CrvsSharePt) {
	CAGD_GEN_COPY(&Crv -> KnotVector[Len1 + Order - 1],
		      &Crv2 -> KnotVector[Order],
		      Len2 * sizeof(CagdRType));
	BspKnotAffineTrans(&Crv -> KnotVector[Len1 + Order - 1],
			   Len2,
			   Crv -> KnotVector[Len1 + Order - 2] -
			       Crv2 -> KnotVector[0],
			   1.0);
    }
    else if (InterpolateDiscont) {
	CAGD_GEN_COPY(&Crv -> KnotVector[Len1 + Order - 1],
		      &Crv2 -> KnotVector[1],
		      (Len2 + Order - 1) * sizeof(CagdRType));
	BspKnotAffineTrans(&Crv -> KnotVector[Len1 + Order - 1],
			   Len2 + Order - 1,
			   Crv -> KnotVector[Len1 + Order - 2] -
			      Crv -> KnotVector[Len1 + Order - 1] + MergeStep,
			   1.0);
    }
    else {
	CAGD_GEN_COPY(&Crv -> KnotVector[Len1 + Order - 1],
		      &Crv2 -> KnotVector[Order - 1],
		      (Len2 + 1) * sizeof(CagdRType));
	BspKnotAffineTrans(&Crv -> KnotVector[Len1 + Order - 1],
			   Len2 + 1,
			   Crv1 -> KnotVector[Len1 + Order - 1] -
			       Crv2 -> KnotVector[Order - 1],
			   1.0);
    }

    /* Make sure the connection is non decreasingly monotone. */
    BspKnotMakeRobustKV(&Crv -> KnotVector[Len1 + Order - 2], Order);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges a list of curves by connecting the end of one curve to the begining M
* of the next. 				                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:         To connect into one curve.                              M
*   InterpDiscont:   If TRUE, linearly interpolate discontinuity.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtPt, CagdMergePtCrv, CagdMergeCrvCrv      M
*   CagdMergeCrvCrv, CagdMergeCrvList2                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeCrvList, merge                                                  M
*****************************************************************************/
CagdCrvStruct *CagdMergeCrvList(const CagdCrvStruct *CrvList, int InterpDiscont)
{
    if (CrvList != NULL && CrvList -> Pnext != NULL) {
	CagdCrvStruct
	    *MergedCrv = CagdCrvCopy(CrvList);

	for (CrvList = CrvList -> Pnext;
	     CrvList != NULL;
	     CrvList = CrvList -> Pnext) {
	    CagdCrvStruct
		*TmpCrv = CagdMergeCrvCrv(MergedCrv, CrvList,
					  InterpDiscont);

	    CagdCrvFree(MergedCrv);
	    MergedCrv = TmpCrv;
	}
	return MergedCrv;
    }
    else
	return CrvList ? CagdCrvCopy(CrvList) : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges a list of curves by connecting end points that are same, in place.  M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:         To connect into larger curves.                          M
*   Tolerance:       To consider two end points the same.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curves.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtPt, CagdMergePtCrv, CagdMergeCrvCrv      M
*   CagdMergeCrvCrv, CagdMergeCrvList                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeCrvList2, merge                                                 M
*****************************************************************************/
CagdCrvStruct *CagdMergeCrvList2(CagdCrvStruct *CrvList,
				 IrtRType Tolerance)
{
    CagdCrvStruct *Crv, *Crv2, *Crv2Prev, *TCrv,
        *MergedCrvs = NULL;

    while (CrvList != NULL) {
        CagdBType RevCrv, RevCrv2,
	    Merge = FALSE;
	CagdPType PtStart, PtEnd, Pt2Start, Pt2End;

        Crv = CrvList;

	CagdCoerceToE3(PtStart, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE3(PtEnd, Crv -> Points, Crv -> Length - 1, Crv -> PType);

	for (Crv2Prev = Crv, Crv2 = CrvList -> Pnext;
	     Crv2 != NULL;
	     Crv2Prev = Crv2, Crv2 = Crv2 -> Pnext) {
	    CagdCoerceToE3(Pt2Start, Crv2 -> Points, 0, Crv2 -> PType);
	    CagdCoerceToE3(Pt2End, Crv2 -> Points, Crv2 -> Length - 1, 
			   Crv2 -> PType);

	    if (IRIT_PT_APX_EQ_EPS(PtStart, Pt2Start, Tolerance)) {
	        Merge = TRUE;
		RevCrv = TRUE;
		RevCrv2 = FALSE;
	    }
	    else if (IRIT_PT_APX_EQ_EPS(PtStart, Pt2End, Tolerance)) {
	        Merge = TRUE;
		RevCrv = TRUE;
		RevCrv2 = TRUE;
	    }
	    else if (IRIT_PT_APX_EQ_EPS(PtEnd, Pt2Start, Tolerance)) {
	        Merge = TRUE;
		RevCrv = FALSE;
		RevCrv2 = FALSE;
	    }
	    else if (IRIT_PT_APX_EQ_EPS(PtEnd, Pt2End, Tolerance)) {
	        Merge = TRUE;
		RevCrv = FALSE;
		RevCrv2 = TRUE;
	    }

	    if (Merge) {
	        Crv2 = Crv2Prev -> Pnext;
		Crv2Prev -> Pnext = Crv2 -> Pnext;
		Crv2 -> Pnext = NULL;
		if (RevCrv2) {
		    TCrv = CagdCrvReverse(Crv2);
		    CagdCrvFree(Crv2);
		    Crv2 = TCrv;
		}

		IRIT_LIST_POP(Crv, CrvList);
		if (RevCrv) {
		    TCrv = CagdCrvReverse(Crv);
		    CagdCrvFree(Crv);
		    Crv = TCrv;
		}

		TCrv = CagdMergeCrvCrv(Crv, Crv2, FALSE);
		CagdCrvFree(Crv2);
		CagdCrvFree(Crv);
		IRIT_LIST_PUSH(TCrv, CrvList);
		break;
	    }
	}

	if (!Merge) {
	    IRIT_LIST_POP(Crv, CrvList);
	    IRIT_LIST_PUSH(Crv, MergedCrvs);
	}
    }

    return MergedCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges a curve and a point by connecting the end of Crv to Pt, using a     M
* linear segment.                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          To connect to Pt its end.				     M
*   Pt:           To connect to Crv's end point.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtPt, CagdMergePtCrv, CagdMergeCrvCrv      M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergeCrvPt, merge                                                    M
*****************************************************************************/
CagdCrvStruct *CagdMergeCrvPt(const CagdCrvStruct *Crv, const CagdPtStruct *Pt)
{
    CagdBType
	CrvNew = FALSE,
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i, NewLen, NewMaxCoord, Len,
	Order = Crv -> Order,
	PtMaxCoord = IRIT_APX_EQ(Pt -> Pt[2], 0.0) ? 2 : 3,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdPType CrvPt;
    CagdPointType CrvPType;
    CagdRType t, **Points, DistPtCrv, CrvArcLen, CrvDomainLen;
    CagdCrvStruct *NewCrv, *CpCrv;

    CpCrv = ConvertToProperCrv(Crv, &CrvNew, &CrvArcLen, &CrvDomainLen);
    if (CrvNew)
	Crv  = CpCrv;
    Len = Crv -> Length;

    /* Compute curve point types. */
    NewMaxCoord = IRIT_MAX(PtMaxCoord, MaxCoord);
    CrvPType = CAGD_MAKE_PT_TYPE(IsRational, NewMaxCoord);

    /* A linear segment is added at the end with Order collinear pts.       */
    /* However since first point is curve last point, only Order - 1 new.   */
    NewLen = Len + Order - 1;

    NewCrv = BspCrvNew(NewLen, Order, CrvPType);
    Points = NewCrv -> Points;

    CopyCrvOnCrv(NewCrv, 0, Crv);
    for (i = 1; i <= NewMaxCoord; i++)
	Points[i][NewLen - 1] = Pt -> Pt[i - 1];
    if (IsRational)
	Points[W][NewLen - 1] = 1.0;
    InterpolateLinearSeg(NewCrv, Len - 1, NewLen - 1);

    /* Update the knot vector. We assume open end condition here... */
    CagdCoerceToE3(CrvPt, Crv -> Points, Crv -> Length - 1, Crv -> PType);
    if ((DistPtCrv = IRIT_PT_PT_DIST(CrvPt, Pt -> Pt)) < CAGD_MERGE_MIN_DIST)
        DistPtCrv = CAGD_MERGE_MIN_DIST;

    CAGD_GEN_COPY(NewCrv -> KnotVector, Crv -> KnotVector,
		  (Len + Order - 1) * sizeof(CagdRType));
    t = Crv -> KnotVector[Len + Order - 1] +
        CrvDomainLen * DistPtCrv / CrvArcLen;

    for (i = Len + Order - 1; i < NewLen + Order; i++)
	NewCrv -> KnotVector[i] = t;

    if (CrvNew)
	CagdCrvFree(CpCrv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges a point and a curve by connecting Pt to the starting point of Crv,  M
* using a linear segment.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:           To connect to Crv's starting point.			     M
*   Crv:          To connect to Pt its starting point.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtPt, CagdMergeCrvPt, CagdMergeCrvCrv      M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergePtCrv, merge                                                    M
*****************************************************************************/
CagdCrvStruct *CagdMergePtCrv(const CagdPtStruct *Pt, const CagdCrvStruct *Crv)
{
    CagdBType
	CrvNew = FALSE,
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i, NewLen, NewMaxCoord, Len,
	Order = Crv -> Order,
	PtMaxCoord = IRIT_APX_EQ(Pt -> Pt[2], 0.0) ? 2 : 3,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdPType CrvPt;
    CagdPointType CrvPType;
    CagdRType t, **Points, DistPtCrv, CrvArcLen, CrvDomainLen;
    CagdCrvStruct *NewCrv, *CpCrv;

    CpCrv = ConvertToProperCrv(Crv, &CrvNew, &CrvArcLen, &CrvDomainLen);
    if (CrvNew)
	Crv  = CpCrv;
    Len = Crv -> Length;

    /* Compute curve point types. */
    NewMaxCoord = IRIT_MAX(PtMaxCoord, MaxCoord);
    CrvPType = CAGD_MAKE_PT_TYPE(IsRational, NewMaxCoord);

    /* A linear segment is added at the end with Order collinear pts.       */
    /* However since first point is curve last point, only Order - 1 new.   */
    NewLen = Len + Order - 1;

    NewCrv = BspCrvNew(NewLen, Order, CrvPType);
    Points = NewCrv -> Points;

    CopyCrvOnCrv(NewCrv, Order - 1, Crv);
    for (i = 1; i <= NewMaxCoord; i++)
	Points[i][0] = Pt -> Pt[i - 1];
    if (IsRational)
	Points[W][0] = 1.0;
    InterpolateLinearSeg(NewCrv, 0, Order - 1);

    /* Update the knot vector. We assume open end condition here... */
    CagdCoerceToE3(CrvPt, Crv -> Points, 0, Crv -> PType);
    if ((DistPtCrv = IRIT_PT_PT_DIST(CrvPt, Pt -> Pt)) < CAGD_MERGE_MIN_DIST)
        DistPtCrv = CAGD_MERGE_MIN_DIST;

    CAGD_GEN_COPY(&NewCrv -> KnotVector[Order], &Crv -> KnotVector[1],
		  (Len + Order - 1) * sizeof(CagdRType));
    t = Crv -> KnotVector[0] - CrvDomainLen * DistPtCrv / CrvArcLen;
    for (i = 0; i < Order; i++)
	NewCrv -> KnotVector[i] = t;

    if (CrvNew)
	CagdCrvFree(CpCrv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two points by connecting Pt1 to Pt2, using a linear segment.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2:     Two points to connect using a linear segment.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtCrv, CagdMergeCrvPt, CagdMergeCrvCrv     M
*   CagdMergePtPt2, CagdMergeUvUv					     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergePtPt, merge                                                     M
*****************************************************************************/
CagdCrvStruct *CagdMergePtPt(const CagdPtStruct *Pt1, const CagdPtStruct *Pt2)
{
    CagdPointType CrvPType;
    CagdCrvStruct *Crv;
    CagdRType **Points;

    if (Pt1 -> Pt[2] == 0.0 && Pt2 -> Pt[2] == 0.0) {
	CrvPType = Pt1 -> Pt[1] == 0.0 && Pt2 -> Pt[1] == 0.0 ?
					CAGD_PT_E1_TYPE : CAGD_PT_E2_TYPE;
    }
    else
        CrvPType = CAGD_PT_E3_TYPE;

    Crv = BzrCrvNew(2, CrvPType);

    Points = Crv -> Points;

    Points[1][0] = Pt1 -> Pt[0];
    Points[1][1] = Pt2 -> Pt[0];
    if (CrvPType == CAGD_PT_E2_TYPE || CrvPType == CAGD_PT_E3_TYPE) {
        Points[2][0] = Pt1 -> Pt[1];
	Points[2][1] = Pt2 -> Pt[1];
    }
    if (CrvPType == CAGD_PT_E3_TYPE) {
    	Points[3][0] = Pt1 -> Pt[2];
    	Points[3][1] = Pt2 -> Pt[2];
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two points by connecting Pt1 to Pt2, using a linear segment.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2:     Two points to connect using a linear segment.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtCrv, CagdMergeCrvPt, CagdMergeCrvCrv     M
*   CagdMergePtPt, CagdMergeUvUv					     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergePtPt2, merge                                                    M
*****************************************************************************/
CagdCrvStruct *CagdMergePtPt2(const CagdPType Pt1, const CagdPType Pt2)
{
    CagdPtStruct P1, P2;

    IRIT_PT_COPY(P1.Pt, Pt1);
    IRIT_PT_COPY(P2.Pt, Pt2);
    return CagdMergePtPt(&P1, &P2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two UV coordinates by connecting UV1, UV2, using a linear segment.  M
*                                                                            *
* PARAMETERS:                                                                M
*  UV1, UV2:     Two UV coordinates to connect using a linear segment.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeCtlPtCtlPt, CagdMergePtCrv, CagdMergeCrvPt, CagdMergeCrvCrv     M
*   CagdMergePtPt, CagdMergePtPt2					     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergeUvUv, merge                                                     M
*****************************************************************************/
CagdCrvStruct *CagdMergeUvUv(const CagdUVType UV1, const CagdUVType UV2)
{
    CagdPtStruct P1, P2;

    IRIT_UV_COPY(P1.Pt, UV1);
    IRIT_UV_COPY(P2.Pt, UV2);
    P1.Pt[2] = P2.Pt[2] = 0.0;
    return CagdMergePtPt(&P1, &P2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two control points by connecting Pt1 to Pt2, using linear segment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2:     Two control points to connect using a linear segment.      M
*   MinDim:       Minimal ctlpts dimension to build the curve with, 2 for E2 M
*		  or P2, etc.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:     The merged curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergePtPt, CagdMergePtCrv, CagdMergeCrvPt, CagdMergeCrvCrv           M
*   CagdMergePtPt2							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeCtlPtCtlPt, merge                                               M
*****************************************************************************/
CagdCrvStruct *CagdMergeCtlPtCtlPt(const CagdCtlPtStruct *Pt1,
				   const CagdCtlPtStruct *Pt2,
				   int MinDim)
{

    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(Pt1 -> PtType) ||
		     CAGD_IS_RATIONAL_PT(Pt2 -> PtType);
    int i, Max1, Max2,
        MaxCoord = IRIT_MAX(Max1 = CAGD_NUM_OF_PT_COORD(Pt1 -> PtType),
		       Max2 = CAGD_NUM_OF_PT_COORD(Pt2 -> PtType));
    CagdPointType CrvPType;
    CagdCrvStruct *Crv;
    CagdRType **Points;

    /* Reduce the dimension if last dimension is identically zero. */
    while (MaxCoord > 1 &&
	   Pt1 -> Coords[MaxCoord] == 0 &&
	   Pt2 -> Coords[MaxCoord] == 0)
        MaxCoord--;
    MaxCoord = IRIT_MAX(MaxCoord, MinDim);

    CrvPType = CAGD_MAKE_PT_TYPE(IsRational, MaxCoord);
    Crv = BzrCrvNew(2, CrvPType);
    Points = Crv -> Points;

    if (IsRational) {
        Points[0][0] = CAGD_IS_RATIONAL_PT(Pt1 -> PtType) ? Pt1 -> Coords[0]
							  : 1.0;
	Points[0][1] = CAGD_IS_RATIONAL_PT(Pt2 -> PtType) ? Pt2 -> Coords[0]
							  : 1.0;
    }

    for (i = 1; i <= MaxCoord; i++) {
        Points[i][0] = i <= Max1 ? Pt1 -> Coords[i] : 0.0;
	Points[i][1] = i <= Max2 ? Pt2 -> Coords[i] : 0.0;
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Copies SrcCrv into DestCrv at point index Index.			     *
*   DestCrv PType is assumed to hold Src PType.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   DestCrv:    Where the copied control points should go to.                *
*   Index:      Index into DestCrv's control polygon.                        *
*   SrcCrv:     Where the copied control points should come from.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CopyCrvOnCrv(CagdCrvStruct *DestCrv,
			 int Index,
			 const CagdCrvStruct *SrcCrv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(SrcCrv);
    int i, j,
	Len = SrcCrv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(SrcCrv -> PType);
    CagdRType
	* const *SrcPoints = SrcCrv -> Points;
    CagdRType
	**DestPoints = DestCrv -> Points;

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(&DestPoints[i][Index], SrcPoints[i],
		      Len * sizeof(CagdRType));

    /* Fix the weights if source did not have them. */
    if (IsNotRational && CAGD_IS_RATIONAL_CRV(DestCrv))
	for (i = Index; i < Index + Len; i++)
	    DestPoints[W][i] = 1.0;

    /* Fix the higher coordinates (by coercing them to zero.) */
    for (i = MaxCoord + 1; i <= CAGD_NUM_OF_PT_COORD(DestCrv -> PType); i++)
	for (j = Index; j < Index + Len; j++)
	    DestPoints[i][j] = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Linearly interpolates between the Crv points indices Index1 and Index2     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:            To add a linear segment between points of Index1 and     *
*                   Index2.						     *
*   Index1, Index2: Indices of first and last points that form the linear    *
*                   segment.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InterpolateLinearSeg(CagdCrvStruct *Crv, int Index1, int Index2)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	DIndex = Index2 - Index1,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType
	**Points = Crv -> Points;

    if (DIndex < 2)
	return;				     /* No middle points to interp. */

    for (i = Index1 + 1; i < Index2; i++) {
	CagdRType
	    t1 = ((CagdRType) (i - Index1)) / DIndex,
	    t2 = 1.0 - t1;

	for (j = IsNotRational; j <= MaxCoord; j++)
	    Points[j][i] = t2 * Points[j][Index1] + t1 * Points[j][Index2];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the given curve to the proper length and computes its domain    *
* and arclength lengthes.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:          To convert to open end Bspline curve.                      *
*   NewCrv:       TRUE if returned curve is a new curve.                     *
*   CrvArcLen:    Length of curve (approximated using the control polygon).  *
*   CrvDomainLen: Length of parametric domain.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Converted curve.  An open end Bspline curve.          *
*****************************************************************************/
static CagdCrvStruct *ConvertToProperCrv(const CagdCrvStruct *Crv,
					 CagdBType *NewCrv,
					 CagdRType *CrvArcLen,
					 CagdRType *CrvDomainLen)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *TCrv, *CpCrv;

    *NewCrv = FALSE;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CpCrv = CagdCnvrtBzr2BspCrv(Crv);
	    *NewCrv = TRUE;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    if (CAGD_IS_PERIODIC_CRV(Crv)) {
		Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
		*NewCrv = TRUE;
	    }
	    else
		CpCrv = NULL;

	    if (!BspCrvHasOpenEC(Crv)) {
		TCrv = CagdCnvrtFloat2OpenCrv(Crv);

		if (CpCrv != NULL)
		    CagdCrvFree(CpCrv);
		Crv = CpCrv = TCrv;
		*NewCrv = TRUE;
	    }
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    *CrvDomainLen = TMax - TMin;

    if ((*CrvArcLen = CagdCrvArcLenPoly(Crv)) < CAGD_MERGE_MIN_DIST)
        *CrvArcLen = CAGD_MERGE_MIN_DIST;

    return CpCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bound on the arc length of a curve by computing the length of   M
* its control polygon.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To bound its length.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   An upper bound on the curve Crv length as the length of     M
*                Crv's control polygon.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdLimitCrvArcLen, CagdSrfAvgArgLenMesh                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvArcLenPoly, arc length                                            M
*****************************************************************************/
CagdRType CagdCrvArcLenPoly(const CagdCrvStruct *Crv)
{
    int i;
    CagdCrvStruct
	*CrvE3 = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, TRUE);
    CagdRType
	Len = 0.0,
	**Points = CrvE3 -> Points;    

    for (i = 1; i < CrvE3 -> Length; i++)
        Len += sqrt(IRIT_SQR(Points[1][i] - Points[1][i - 1]) +
		    IRIT_SQR(Points[2][i] - Points[2][i - 1]) +
		    IRIT_SQR(Points[3][i] - Points[3][i - 1]));

    CagdCrvFree(CrvE3);

    return Len;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Subdivides the given curves to curves, each with size of control polygon   M
* less than or equal to MaxLen.	Returned is a list of curves.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To subdivide into curves, each with control polygon length   M
*               less than MaxLen.					     M
*   MaxLen:     Maximum length of control polygon to allow.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of subdivided curves from Crv, each with control M
*                      polygon size of less than MaxLen.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvArcLenPoly, CagdSrfAvgArgLenMesh                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdLimitCrvArcLen, arc length                                           M
*****************************************************************************/
CagdCrvStruct *CagdLimitCrvArcLen(const CagdCrvStruct *Crv, CagdRType MaxLen)
{
    if (CagdCrvArcLenPoly(Crv) > MaxLen) {
	CagdRType TMin, TMax;
	CagdCrvStruct *Crv1, *Crv2, *Crv1MaxLen, *Crv2MaxLen;

	CagdCrvDomain(Crv, &TMin, &TMax);

	Crv1 = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5);
	Crv2 = Crv1 -> Pnext;

	Crv1MaxLen = CagdLimitCrvArcLen(Crv1, MaxLen);
	Crv2MaxLen = CagdLimitCrvArcLen(Crv2, MaxLen);

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

	for (Crv1 = Crv1MaxLen; Crv1 -> Pnext != NULL; Crv1 = Crv1 -> Pnext);
	Crv1 -> Pnext = Crv2MaxLen;
	return Crv1MaxLen;
    }
    else
        return CagdCrvCopy(Crv);
}
