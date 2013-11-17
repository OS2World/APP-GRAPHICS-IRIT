/******************************************************************************
* MS_Circ.c - minimum spanning circle for curves.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct 2004.					      *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"
#include "allocate.h"
#include "geom_lib.h"

#define MVAR_MSC_SAMPLES_PER_CRV	8

#ifdef DEBUG
IRIT_STATIC_DATA int
    GlblCurveInCircTests = 0,
    GlblCircleWith2TansTests = 0,
    GlblCircleWith3TansTests = 0;
IRIT_SET_DEBUG_PARAMETER(_DebugMSCStatistics, FALSE);
#endif /* DEBUG */

static int MvarMinSpanCircCHPurge(IPObjectStruct **ObjsVec, int VecLen);
static int MvarMinSpanCircWithObj(IPObjectStruct **ObjsVec,
				  int NumOfObjs,
				  IPObjectStruct *BObj,
				  CagdRType *Center,
				  CagdRType *Radius,
				  CagdRType SubdivTol,
				  CagdRType NumerTol);
static int MvarMinSpanCircWith2Objs(IPObjectStruct **ObjsVec,
				    int NumOfObjs,
				    IPObjectStruct *BObj1,
				    IPObjectStruct *BObj2,
				    CagdRType *Center,
				    CagdRType *Radius,
				    CagdRType SubdivTol,
				    CagdRType NumerTol);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimum spanning circle to two curves that are disjoint.    M
* Assumption is made that the MSC is tangent to both curves.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv1, OrigCrv2:    The two curves to consider.                       M
*   Center:                Center of the computed MSC.                       M
*   Radius:                Radius of the computed MSC.                       M
*   SubdivTol, NumerTol:   Of computation.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMSCircOfThreeCurves, MvarMinSpanCirc, MVarIsCrvInsideCirc            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMSCircOfTwoCurves                                                    M
*****************************************************************************/
int MvarMSCircOfTwoCurves(const CagdCrvStruct *OrigCrv1,
			  const CagdCrvStruct *OrigCrv2,
			  CagdRType Center[2],
			  CagdRType *Radius,
			  CagdRType SubdivTol,
			  CagdRType NumerTol)
{
    CagdCrvStruct *DCrv, *Crv1, *Crv2;
    MvarMVStruct *MVCrv1, *MVCrv2, *MVTan1, *MVTan2,
	*MVVec[2], *MVTmp1, *MVTmp2;
    MvarConstraintType Constraints[2];
    MvarPtStruct *MVPts, *MVPt;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMSCStatistics)
	GlblCircleWith2TansTests++;
#   endif /* DEBUG */

    /* Make sure all curves are with domain zero to one. */
    Crv1 = CagdCrvCopy(OrigCrv1);
    Crv2 = CagdCrvCopy(OrigCrv2);
    if (CAGD_IS_BSPLINE_CRV(Crv1))
	BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
				 Crv1 -> Length + Crv1 -> Order, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(Crv2))
	BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);

    /* Convert position curves. */
    MVTmp1 = MvarCrvToMV(Crv1);
    MVCrv1 = MvarPromoteMVToMV2(MVTmp1, 2, 0);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTmp1, 2, 1);
    MvarMVFree(MVTmp1);

    /* Convert tangent curves. */
    DCrv = CagdCrvDerive(Crv1);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan1 = MvarPromoteMVToMV2(MVTmp1, 2, 0);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv2);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan2 = MvarPromoteMVToMV2(MVTmp1, 2, 1);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    /* Formulate the constraints for the MSC of C1(u), C2(v):		     */
    /*									     */
    /* < C1'(u), C1(u) - P > = 0,					     */
    /* < C2'(v), C2(v) - P > = 0,					     */
    /* where P = (C1(u) + C2(v)) / 2.0     or				     */
    /* < C1'(u), C1(u) - (C1(u) + C2(v)) / 2.0 > = 0,			     */
    /* < C2'(v), C2(v) - (C1(u) + C2(v)) / 2.0 > = 0,			     */
    /* providing two constraints (u and v) and two equations.		     */
    MVTmp1 = MvarMVAdd(MVCrv2, MVCrv1);
    MvarMVTransform(MVTmp1, NULL, 0.5);

    /* Now formulate out the two constraints. */
    MVTmp2 = MvarMVSub(MVCrv1, MVTmp1);
    MVVec[0] = MvarMVDotProd(MVTmp2, MVTan1);
    MvarMVFree(MVTmp2);

    MVTmp2 = MvarMVSub(MVCrv2, MVTmp1);
    MVVec[1] = MvarMVDotProd(MVTmp2, MVTan2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);
    MvarMVFree(MVTan1);
    MvarMVFree(MVTan2);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    
    MVPts = MvarMVsZeros(MVVec, Constraints, 2, SubdivTol, NumerTol);
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);

    /* Test all solutions for the one that contains all three curves. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	CagdRType *R;
	CagdPType Pos1, Pos2;

	/* Compute the singular equadistant point. */
	R = CagdCrvEval(Crv1, MVPt -> Pt[0]);
	CagdCoerceToE2(Pos1, &R, -1, Crv1 -> PType);

	R = CagdCrvEval(Crv2, MVPt -> Pt[1]);
	CagdCoerceToE2(Pos2, &R, -1, Crv2 -> PType);

	IRIT_PT2D_BLEND(Center, Pos1, Pos2, 0.5);

	*Radius = IRIT_PT2D_DIST(Pos1, Center);

	if (MVarIsCrvInsideCirc(Crv1, Center, *Radius, NumerTol) &&
	    (OrigCrv2 == OrigCrv1 ||
	     MVarIsCrvInsideCirc(Crv2, Center, *Radius, NumerTol))) {
	    break;
	}
    }

    /* If we failed, we should try tri-tangency as Crv1/Crv1/Crv2 or        */
    /* Crv1/Crv2/Crv2 and select the minimum between the two.		    */
    if (MVPt == NULL) {
	CagdRType Rad1, Rad2;
	CagdPType Cntr1, Cntr2;

	MvarMSCircOfThreeCurves(Crv1, Crv1, Crv2, Cntr1, &Rad1,
				SubdivTol, NumerTol);
	MvarMSCircOfThreeCurves(Crv1, Crv2, Crv2, Cntr2, &Rad2,
				SubdivTol, NumerTol);

	MvarPtFreeList(MVPts);
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

	if (Rad1 == IRIT_INFNTY && Rad2 == IRIT_INFNTY)
	    return FALSE;

	if (Rad1 > Rad2) {
	    *Radius = Rad2;
	    IRIT_PT2D_COPY(Center, Cntr2);
	}
	else {
	    *Radius = Rad1;
	    IRIT_PT2D_COPY(Center, Cntr1);
	}

        return TRUE;
    }
    else {
        MvarPtFreeList(MVPts);
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

        return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimum spanning circle to three curves that are disjoint.  M
* Assumption is made that the MSC is tangent to all three curves.            M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv1, OrigCrv2, OrigCrv3:    The three curves to consider.           M
*			             Curves could be identical.		     M
*   Center:                Center of the computed MSC.                       M
*   Radius:                Radius of the computed MSC.                       M
*   SubdivTol, NumerTol:   Of computation.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMSCircOfTwoCurves, MvarMinSpanCirc, MVarIsCrvInsideCirc              M
*   MvarSkel2DInter3Prims						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMSCircOfThreeCurves                                                  M
*****************************************************************************/
int MvarMSCircOfThreeCurves(const CagdCrvStruct *OrigCrv1,
			    const CagdCrvStruct *OrigCrv2,
			    const CagdCrvStruct *OrigCrv3,
			    CagdRType Center[2],
			    CagdRType *Radius,
			    CagdRType SubdivTol,
			    CagdRType NumerTol)
{
    int TwoIdenticalCrvs;
    CagdCrvStruct *DCrv, *Crv1, *Crv2, *Crv3;
    MvarMVStruct *MVCrv1, *MVCrv2, *MVCrv3, *MVTan1, *MVTan2, *MVTan3,
	*MVVec[MVAR_MAX_PT_SIZE], *MVA1Split[MVAR_MAX_PT_SIZE],**MVA2Split,
	*MVTmp1, *MVTmp2, *MVb1, *MVb2, *MVA1, *MVA2, *MVPDenom, *MVPNumer;
    MvarConstraintType Constraints[3];
    MvarPtStruct *MVPts, *MVPt;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMSCStatistics)
	GlblCircleWith3TansTests++;
#   endif /* DEBUG */

    /* Make sure all curves are with domain zero to one. */
    Crv1 = CagdCrvCopy(OrigCrv1);
    Crv2 = CagdCrvCopy(OrigCrv2);
    Crv3 = CagdCrvCopy(OrigCrv3);
    if (CAGD_IS_BSPLINE_CRV(Crv1))
	BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
				 Crv1 -> Length + Crv1 -> Order, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(Crv2))
	BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(Crv3))
	BspKnotAffineTransOrder2(Crv3 -> KnotVector, Crv3 -> Order,
				 Crv3 -> Length + Crv3 -> Order, 0.0, 1.0);

    /* Convert position curves. */
    MVTmp1 = MvarCrvToMV(Crv1);
    MVCrv1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv3);
    MVCrv3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);

    /* Convert tangent curves. */
    DCrv = CagdCrvDerive(Crv1);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv2);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv3);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    /* Formulate the constraints for the MSC of C1(u), C2(v), C3(w):	     */
    /*									     */
    /* < P - C1(u), P - C1(u) > = < P - C2(v), P - C2(v) >,		     */
    /* < P - C1(u), P - C1(u) > = < P - C3(w), P - C3(w) >,  or		     */
    /*									     */
    /* 2(C2(v) - C1(u)) P = C2(v)^2 - C1(u)^2,               		     */
    /* 2(C3(w) - C1(u)) P = C3(w)^2 - C1(u)^2, and solve for P = P(u, v, w). */
    MVA1 = MvarMVSub(MVCrv2, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv2, MVCrv1);
    MVb1 = MvarMVDotProd(MVA1, MVTmp1);
    MvarMVTransform(MVA1, NULL, 2.0);
    MvarMVFree(MVTmp1);
    
    MVA2 = MvarMVSub(MVCrv3, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv3, MVCrv1);
    MVb2 = MvarMVDotProd(MVA2, MVTmp1); 
    MvarMVTransform(MVA2, NULL, 2.0);
    MvarMVFree(MVTmp1);

    IRIT_GEN_COPY(MVA1Split, MvarMVSplitScalar(MVA1),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVA2Split = MvarMVSplitScalar(MVA2);
    MvarMVFree(MVA1);
    MvarMVFree(MVA2);

    /* Solve for P = P(u, v, w). */
    IRIT_ZAP_MEM(MVVec, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVPDenom = MvarMVDeterminant2(MVA1Split[1], MVA1Split[2],
				  MVA2Split[1], MVA2Split[2]);
    MVVec[1] = MvarMVDeterminant2(MVb1, MVA1Split[2],
				  MVb2, MVA2Split[2]);
    MVVec[2] = MvarMVDeterminant2(MVA1Split[1], MVb1,
				  MVA2Split[1], MVb2);
    MvarMVFree(MVA1Split[1]);
    MvarMVFree(MVA1Split[2]);
    if (MVA1Split[3] != NULL)
	MvarMVFree(MVA1Split[3]);

    MvarMVFree(MVA2Split[1]);
    MvarMVFree(MVA2Split[2]);
    if (MVA2Split[3] != NULL)
	MvarMVFree(MVA2Split[3]);

    MvarMVFree(MVb1);
    MvarMVFree(MVb2);

    MVPNumer = MvarMVMergeScalar(MVVec);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /* Now formulate out the three following constraints with P's solution. */
    MVTmp1 = MvarMVMultScalar(MVCrv1, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[0] = MvarMVDotProd(MVTmp2, MVTan1);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv2, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[1] = MvarMVDotProd(MVTmp2, MVTan2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv3, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[2] = MvarMVDotProd(MVTmp2, MVTan3);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);
    MvarMVFree(MVCrv3);
    MvarMVFree(MVTan1);
    MvarMVFree(MVTan2);
    MvarMVFree(MVTan3);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
    
    MVPts = MvarMVsZeros(MVVec, Constraints, 3, SubdivTol, NumerTol);
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    TwoIdenticalCrvs = OrigCrv2 == OrigCrv1 || OrigCrv3 == OrigCrv2;
    *Radius = IRIT_INFNTY;

    /* Test all solutions for the one that contains all three curves. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	CagdRType *R, t, Rad;
	CagdPType Pos, Cntr;

	/* And compute the singular equadistant point. */
	R = MvarMVEval(MVPNumer, MVPt -> Pt);
	CagdCoerceToE2(Cntr, &R, -1, (CagdPointType) MVPNumer -> PType);

	R = MvarMVEval(MVPDenom, MVPt -> Pt);
	t = R[1] == 0 ? IRIT_INFNTY : 1.0 / R[1];
	IRIT_PT2D_SCALE(Cntr, t);

	R = CagdCrvEval(Crv1, MVPt -> Pt[0]);
	CagdCoerceToE2(Pos, &R, -1, Crv1 -> PType);
	Rad = IRIT_PT2D_DIST(Pos, Cntr);

	if (Rad < *Radius &&
	    MVarIsCrvInsideCirc(Crv1, Cntr, Rad, NumerTol) &&
	    (OrigCrv2 == OrigCrv1 ||
	     MVarIsCrvInsideCirc(Crv2, Cntr, Rad, NumerTol)) &&
	    (OrigCrv3 == OrigCrv2 ||
	     MVarIsCrvInsideCirc(Crv3, Cntr, Rad, NumerTol))) {
	    *Radius = Rad;
	    IRIT_PT2D_COPY(Center, Cntr);
	    if (!TwoIdenticalCrvs)
	        break;
	}
    }

    MvarMVFree(MVPNumer);
    MvarMVFree(MVPDenom);

    MvarPtFreeList(MVPts);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Crv3);

    return MVPt != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if a circle is contained in the given prescribed curve.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           Curve to test for containment in the circle.              M
*   Center:        Center of the circle to test against.                     M
*   Radius:        Radius of the circle to test against.                     M
*   Tolerance:     Of computation.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if Crv is indeed inside the circle, FALSE otherwise.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMSCircOfThreeCurves, MvarMSCircOfThreeCurves, MvarMinSpanCirc        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarIsCrvInsideCirc                                                      M
*****************************************************************************/
int MVarIsCrvInsideCirc(const CagdCrvStruct *Crv,
			CagdRType Center[2],
			CagdRType Radius,
			CagdRType Tolerance)
{
    CagdRType MaxDistParam, *R;
    CagdPType Pt, MaxDistPos;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMSCStatistics)
	GlblCurveInCircTests++;
#   endif /* DEBUG */

    /* Easy test - check if all control points are inside the circle. */
    if (CagdIsCrvInsideCirc(Crv, Center, Radius))
        return TRUE;

    Pt[0] = Center[0];
    Pt[1] = Center[1];
    Pt[2] = 0.0;

    MaxDistParam = SymbDistCrvPoint(Crv, Pt, FALSE, IRIT_FABS(Tolerance));

    R = CagdCrvEval(Crv, MaxDistParam);
    CagdCoerceToE2(MaxDistPos, &R, -1, Crv -> PType);

    return IRIT_PT2D_DIST(MaxDistPos, Center) - IRIT_FABS(Tolerance) * 10 <
								       Radius;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute a convex hull though sampled points in the given set of curves   *
* and purge any curve found completely inside this convex hull - this curve  *
* cannot be in the minimum spanning circle.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   ObjsVec:   Vector of curve objects to examine.                           *
*   VecLen:    Length of ObjsVec.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int MvarMinSpanCircCHPurge(IPObjectStruct **ObjsVec, int VecLen)
{
    int i, j, l,
	CHVecLen = VecLen * MVAR_MSC_SAMPLES_PER_CRV;
    IrtE2PtStruct
        *CHPts = (IrtE2PtStruct *) IritMalloc(sizeof(IrtE2PtStruct) *
					                           CHVecLen);

    /* Sample curves and substitute the samples into the CH data struct. */
    for (i = l = 0; i < VecLen; i++) {
	CagdRType TMin, TMax, Dt, t, *R;
	CagdCrvStruct
	    *Crv = ObjsVec[i] -> U.Crvs;

	CagdCrvDomain(Crv, &TMin, &TMax);
	Dt = (TMax - TMin) / MVAR_MSC_SAMPLES_PER_CRV;

	for (j = 0, t = 0.0; j < MVAR_MSC_SAMPLES_PER_CRV; j++, l++, t += Dt) {
	    R = CagdCrvEval(Crv, t);
	    if (CAGD_IS_RATIONAL_CRV(Crv)) {
	        CHPts[l].Pt[0] = R[1] / R[0];
		CHPts[l].Pt[1] = R[2] / R[0];
	    }
	    else {
	        CHPts[l].Pt[0] = R[1];
		CHPts[l].Pt[1] = R[2];
	    }
	}
    }

    if (GMConvexHull(CHPts, &l)) {
        for (i = 0; i < VecLen; ) {
	    CagdCrvStruct
	        *Crv = ObjsVec[i] -> U.Crvs;

	    /* Examine curve if totally on one side of all edges of CH. */
	    for (j = 0; j < l; j++) {
	        if (!CagdCrvOnOneSideOfLine(Crv,
					    CHPts[j].Pt[0],
					    CHPts[j].Pt[1],
					    CHPts[(j + 1) % l].Pt[0],
					    CHPts[(j + 1) % l].Pt[1]))
		    break;
	    }
	    if (j >= l) {
	        /* Purge this curve. */
	        IPFreeObject(ObjsVec[i]);
		ObjsVec[i] = ObjsVec[--VecLen];
		ObjsVec[VecLen] = NULL;
	    }
	    else
	        i++;
	}
    }

    IritFree(CHPts);

    return VecLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning circle (MSC) computation of given Objs geometry.	     M
* Geometry could be freeform C^1 curves.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Objs:       The geometry to compute the MSC for as a list object.	     M
*   Center:     Of computed MSC.					     M
*   Radius:	Of computed MSC.					     M
*   SubdivTol, NumerTol:   Of computation.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMSCircOfThreeCurves, MvarMSCircOfThreeCurves, MvarMinSpanCirc,       M
*   MVarMSCircCurveInCirc, GMMinSpanCirc                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMinSpanCirc, minimum spanning circle                                 M
*****************************************************************************/
int MvarMinSpanCirc(IPObjectStruct *Objs,
		    CagdRType *Center,
		    CagdRType *Radius,
		    CagdRType SubdivTol,
		    CagdRType NumerTol)
{
    int i, Len;
    IPObjectStruct *Obj, **ObjsVec;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMSCStatistics) {
	GlblCurveInCircTests = 0;
	GlblCircleWith2TansTests = 0;
	GlblCircleWith3TansTests = 0;
    }
#   endif /* DEBUG */

    /* Make a vector out of this list of objects. */
    Len = IPListObjectLength(Objs);
    if (Len < 2) {
        Center[0] = Center[1] = 0.0;
	*Radius = IRIT_INFNTY;
	return FALSE;
    }
    ObjsVec = (IPObjectStruct **) IritMalloc(sizeof(IPObjectStruct *) * Len);
    for (i = 0; i < Len && (Obj = IPListObjectGet(Objs, i)) != NULL; i++) {
        ObjsVec[i] = IPCopyObject(NULL, Obj, TRUE);

	if (CAGD_IS_BSPLINE_CRV(ObjsVec[i] -> U.Crvs)) {
	    CagdRType TMin, TMax;
	    CagdCrvStruct
		*Crv = ObjsVec[i] -> U.Crvs;

	    if (CAGD_IS_PERIODIC_CRV(Crv)) {
	        Crv = CagdCnvrtPeriodic2FloatCrv(Crv);
		CagdCrvFree(ObjsVec[i] -> U.Crvs);
		ObjsVec[i] -> U.Crvs = Crv;
	    }

	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (!IRIT_APX_EQ_EPS(TMin, 0.0, IRIT_UEPS) ||
		!IRIT_APX_EQ_EPS(TMax, 1.0, IRIT_UEPS))
	        BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
					 Crv -> Length + Crv -> Order,
					 0.0, 1.0);
	}
    }

    /* COmpute a convex hull for a set of points sampled over the curves and */
    /* purge any curve found to be completely inside the convex hull.        */
    Len = MvarMinSpanCircCHPurge(ObjsVec, Len);

    /* Compute a trivial bound to first two objects. */
    if (!MvarMSCircOfTwoCurves(ObjsVec[0] -> U.Crvs,
			       ObjsVec[1] -> U.Crvs,
			       Center, Radius, SubdivTol, NumerTol)) {
        *Radius = IRIT_INFNTY;
	return FALSE;
    }

    /* And examine the rest if inside. */
    for (i = 2; i < Len; i++) {
	if (!MVarIsCrvInsideCirc(ObjsVec[i] -> U.Crvs,
				 Center, *Radius, NumerTol)) {
	    if (!MvarMinSpanCircWithObj(ObjsVec, i, ObjsVec[i],
					Center, Radius,
					SubdivTol, NumerTol)) {
	        *Radius = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }

    for (i = 0; i < Len; i++)
	IPFreeObject(ObjsVec[i]);
    IritFree(ObjsVec);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMSCStatistics) {
	printf("\tNumber of curve-in-circle tests = %d\n",
	       GlblCurveInCircTests);
	printf("\tNumber of 2-Tangents-circle tests = %d\n",
	       GlblCircleWith2TansTests);
	printf("\tNumber of 3-Tangents-circle tests = %d\n",
	       GlblCircleWith3TansTests);
    }
#   endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of MvarMinSpanCirc.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   ObjsVec:         The geometry to compute the MSC for as linked list.     *
*   NumOfObjs:       Number of objects in vector ObjVec.		     *
*   BObj:	     Extra object that must be on the boundary of the MSC.   *
*   Center:          Of computed MSC.					     *
*   Radius:	     Of computed MSC.					     *
*   NumerTol, SubdivTol:   Of computation.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if successful, FALSE otherwise.                       *
*****************************************************************************/
static int MvarMinSpanCircWithObj(IPObjectStruct **ObjsVec,
				  int NumOfObjs,
				  IPObjectStruct *BObj,
				  CagdRType *Center,
				  CagdRType *Radius,
				  CagdRType SubdivTol,
				  CagdRType NumerTol)
{
    int i;

    if (NumOfObjs < 1) {
        *Radius = IRIT_INFNTY;
	return FALSE;
    }

    /* Compute a trivial bound to first two objects. */
    if (!MvarMSCircOfTwoCurves(ObjsVec[0] -> U.Crvs, BObj -> U.Crvs,
			       Center, Radius, SubdivTol, NumerTol)) {
        *Radius = IRIT_INFNTY;
	return FALSE;
    }

    /* And examine the rest if inside. */
    for (i = 1; i < NumOfObjs; i++) {
	if (!MVarIsCrvInsideCirc(ObjsVec[i] -> U.Crvs,
				 Center, *Radius, NumerTol)) {
	    if (!MvarMinSpanCircWith2Objs(ObjsVec, i, ObjsVec[i], BObj,
					  Center, Radius,
					  SubdivTol, NumerTol)) {
	        *Radius = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of MvarMinSpanCirc.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   ObjsVec:         The geometry to compute the MSC for as linked list.     *
*   NumOfObjs:       Number of objects in vector ObjVec.		     *
*   BObj1, BObj2:    Extra two objects that must be on the boundary of MSC.  *
*   Center:          Of computed MSC.					     *
*   Radius:	     Of computed MSC.					     *
*   NumerTol, SubdivTol:   Of computation.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if successful, FALSE otherwise.                       *
*****************************************************************************/
static int MvarMinSpanCircWith2Objs(IPObjectStruct **ObjsVec,
				    int NumOfObjs,
				    IPObjectStruct *BObj1,
				    IPObjectStruct *BObj2,
				    CagdRType *Center,
				    CagdRType *Radius,
				    CagdRType SubdivTol,
				    CagdRType NumerTol)
{
    int i;

    /* Compute a trivial bound to first two objects. */
    MvarMSCircOfTwoCurves(BObj1 -> U.Crvs, BObj2 -> U.Crvs,
			  Center, Radius, SubdivTol, NumerTol);

    /* And examine the rest if inside. */
    for (i = 0; i < NumOfObjs; i++) {
        if (!MVarIsCrvInsideCirc(ObjsVec[i] -> U.Crvs, Center,
				 *Radius, NumerTol)) {
	    if (!MvarMSCircOfThreeCurves(BObj1 -> U.Crvs, BObj2 -> U.Crvs,
					 ObjsVec[i] -> U.Crvs,
					 Center, Radius,
					 SubdivTol, NumerTol)) {
	        *Radius = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }

    return TRUE;
}
