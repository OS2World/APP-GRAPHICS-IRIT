/******************************************************************************
* Hasdrf2d.c - computes Hausdorff distance between freeforms in 2D.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 2006.					      *
******************************************************************************/

#include <assert.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "mvar_loc.h"

#define MVAR_HF_DIST_SUBDIV_TOL		1e-2
#define MVAR_HF_DIST_OFFSET_TOL		1e-4
#define MVAR_MIN_DIST_SAMPLES		10
#define MVAR_HDIST2D_SOLVE_ET		TRUE

#define MVAR_MV_HAS_NO_KNOT(MV, Axis, t) ( \
    i = BspKnotLastIndexLE(MV -> KnotVectors[Axis], \
 			   MV -> Lengths[Axis] + MV -> Orders[Axis], t), \
    !IRIT_APX_EQ_EPS(MV -> KnotVectors[Axis][i], t, IRIT_UEPS) )

#define MVAR_MV_AFFINE_TRANS_KV(MV, i, TMin, TMax) \
	BspKnotAffineTransOrder2(MV -> KnotVectors[i], MV -> Orders[i], \
				 MV -> Lengths[i] + MV -> Orders[i], \
			         TMin, TMax)

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the min or max distance between a point and a C^1 cont. curve.  M
*   The curve and point are assumed to be either in R2 or R3.		     M
*   The extreme distance between a point and a curve could happen either     M
* at the end points of curve C or when					     M
*   C'(t) <C(t) - P> = 0.						     V
*                                                                            *
* PARAMETERS:                                                                M
*   P:        Point to measure its Hausdorff distance to curve Crv.	     M
*   Crv:      Crv to measure its hausdorff distance to point Pt.	     M
*   Param:    Where to return the parameter value with the maximal distance. M
*   MinDist:  TRUE for minimal distance, FALSE for maximal.		     M
*   Epsilon:  Tolerance of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvPoint                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDistPointCrvC1                                                       M
*****************************************************************************/
CagdRType MvarDistPointCrvC1(CagdPType P,
			     const CagdCrvStruct *Crv,
			     MvarHFDistParamStruct *Param,
			     CagdBType MinDist,
			     CagdRType Epsilon)
{
    CagdRType *R;
    CagdPType Pt;

    Param -> T[0] = SymbDistCrvPoint(Crv, P, MinDist, Epsilon);
    Param -> NumOfParams = 1;
    Param -> ManifoldDim = 1;

    R = CagdCrvEval(Crv, Param -> T[0]);
    CagdCoerceToE3(Pt, &R, -1, Crv -> PType);

    return IRIT_PT_PT_DIST(P, Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the local extreme distance between a point P of Crv1 and Crv2.  M
* At the local extreme distance location on Crv2, denoted Q, verify that Q   M
* is closest to P than to any other location on Crv1. 			     M
*   Returns maximal local extreme distance found, 0.0 if none.   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   P:        Point on Crv1 to measure its extreme distance to curve Crv2.   M
*   Crv1:     First curve that contains P.				     M
*   Crv2:     Second curve to measure extreme distance to from P.	     M
*   Param2:   Where to return the parameter value of Crv2 is returned.	     M
*   Epsilon:  Tolerance of computation.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The local extreme distance found, 0.0 if none.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbLclDistCrvPoint                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFExtremeLclDistPointCrvC1                                           M
*****************************************************************************/
CagdRType MvarHFExtremeLclDistPointCrvC1(CagdPType P,
					 const CagdCrvStruct *Crv1,
					 const CagdCrvStruct *Crv2,
					 MvarHFDistParamStruct *Param2,
					 CagdRType Epsilon)
{
    CagdRType *R, Dist, MinDist,
	MaxDist = 0.0;
    CagdPType Q;
    CagdPtStruct *Pt,
	*Pts = SymbLclDistCrvPoint(Crv2, P, Epsilon);
    MvarHFDistParamStruct Param1;

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        R = CagdCrvEval(Crv2, Pt -> Pt[0]);
	CagdCoerceToE3(Q, &R, - 1, Crv2 -> PType);
	Dist = IRIT_PT_PT_DIST(P, Q);

	if (Dist > MaxDist) {
	    MinDist = MvarDistPointCrvC1(Q, Crv1, &Param1, TRUE, Epsilon);

	    if (IRIT_APX_EQ_EPS(MinDist, Dist, Epsilon * 10)) {
	        /* Consider this solution which is a valid maximum! */
	        MaxDist = IRIT_MAX(MaxDist, Dist);
		Param2 -> T[0] = Pt -> Pt[0];
		Param2 -> NumOfParams = 1;
		Param2 -> ManifoldDim = 1;
	    }
	}
    }

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the antipodal points of the given two curves by solving:        M
*     < C1(t) - C2(r), C1'(t) > = 0,				             V
*     < C1(t) - C2(r), C2'(r) > = 0.				             V
*     < C1(t) - C2O(r), C1'(t) > = 0,       (Only during subdivision step)   V
*     < C1(t) - C2O(r), C2'(r) > = 0,	    (Only during subdivision step)   V
* where C2O is an offset curve of C2 by amount larger than subdivison tol.   M
* The last two equations are used to purge intersection locations as they    M
* cause the first two equations to vanish.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves to solve for their antipodal locations.      M
*   Epsilon:     Tolerance of computation.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of pairs of parameters in the t & r coefficients. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints, MvarCntctTangentialCrvCrvC1                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistAntipodalCrvCrvC1                                              M
*****************************************************************************/
MvarPtStruct *MvarHFDistAntipodalCrvCrvC1(const CagdCrvStruct *Crv1,
					  const CagdCrvStruct *Crv2,
					  CagdRType Epsilon)
{
    CagdBType BspGeom, OffsetContstraints;
    int MaxDim1 = CAGD_NUM_OF_PT_COORD(Crv1 -> PType),
        MaxDim2 = CAGD_NUM_OF_PT_COORD(Crv2 -> PType);
    CagdRType t, TMin1, TMax1, TMin2, TMax2;
    MvarPtStruct *AntiPodalPoints;
    CagdCrvStruct *Crv1Off, *Crv2Off, *TCrv1, *TCrv2;
    MvarMVStruct *MVCrv1, *MVDCrv1, *MVCrv2, *MVDCrv2, *MVTemp, *MVCrvDiff,
	*MVs[4];
    MvarConstraintType Constrs[4];

    /* Make sure curves have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_CRV(Crv1))
	TCrv1 = CagdCnvrtBzr2BspCrv(Crv1);
    else
        TCrv1 = CagdCnvrtBsp2OpenCrv(Crv1);

    if (CAGD_IS_BEZIER_CRV(Crv2))
	TCrv2 = CagdCnvrtBzr2BspCrv(Crv2);
    else
	TCrv2 = CagdCnvrtBsp2OpenCrv(Crv2);

    CagdCrvDomain(TCrv1, &TMin1, &TMax1);
    CagdCrvDomain(TCrv2, &TMin2, &TMax2);

    BspGeom = CAGD_IS_BSPLINE_CRV(TCrv1);

    /* Compute Crv1(t) - Crv2(r). */
    MVTemp = MvarCrvToMV(TCrv1);
    MVCrv1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    if (BspGeom)
        BspKnotAffineTransOrder2(MVCrv1 -> KnotVectors[1], MVCrv1 -> Orders[1],
				 MVCrv1 -> Lengths[1] + MVCrv1 -> Orders[1],
				 TMin2, TMax2);    
    MvarMVFree(MVTemp);
    MVTemp = MvarCrvToMV(TCrv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    if (BspGeom)
        BspKnotAffineTransOrder2(MVCrv2 -> KnotVectors[0], MVCrv2 -> Orders[0],
				 MVCrv2 -> Lengths[0] + MVCrv2 -> Orders[0],
				 TMin1, TMax1);  
    MvarMVFree(MVTemp);

    MVCrvDiff = MvarMVSub(MVCrv1, MVCrv2);

    /* Compute derivatives of Crv1, DCrv2. */
    MVDCrv1 = MvarMVDerive(MVCrv1, 0);
    MVDCrv2 = MvarMVDerive(MVCrv2, 1);

    /* < C1(t) - C2(r), C1'(t) >. */
    MVs[0] = MvarMVDotProd(MVCrvDiff, MVDCrv1);

    /* < C1(t) - C2(r), C2'(r) >. */
    MVs[1] = MvarMVDotProd(MVCrvDiff, MVDCrv2);

    OffsetContstraints = TRUE;
    /* Verify that the normal field will be continuous. */
    if ((MaxDim1 == 2 && !BspCrvKnotC1Discont(TCrv1, &t)) ||
	(MaxDim1 == 3 && TCrv1 -> Order > 2 && !BspCrvKnotC2Discont(TCrv1, &t))) {
        /* Compute Offset(Crv1(t)) - Crv2(r). */
        Crv1Off = SymbCrvSubdivOffset(TCrv1, MVAR_HF_DIST_SUBDIV_TOL * 3,
				      MVAR_HF_DIST_OFFSET_TOL, FALSE);
	MVTemp = MvarCrvToMV(Crv1Off);
	MvarMVFree(MVCrv1);
	CagdCrvFree(Crv1Off);
	MVCrv1 = MvarPromoteMVToMV2(MVTemp, 2, 0);

	if (BspGeom)
	    BspKnotAffineTransOrder2(MVCrv1 -> KnotVectors[1],
				     MVCrv1 -> Orders[1],
				     MVCrv1 -> Lengths[1] +
				                          MVCrv1 -> Orders[1],
				     TMin2, TMax2); 

	MvarMVFree(MVTemp);
    }
    else if ((MaxDim2 == 2 && !BspCrvKnotC1Discont(TCrv2, &t)) ||
	     (MaxDim2 == 3 && TCrv2 -> Order > 2 && !BspCrvKnotC2Discont(TCrv2, &t))) {
        /* Compute Crv1(t) - Offset(Crv2(r)). */
        Crv2Off = SymbCrvSubdivOffset(TCrv2, MVAR_HF_DIST_SUBDIV_TOL * 3,
				      MVAR_HF_DIST_OFFSET_TOL, FALSE);
	MVTemp = MvarCrvToMV(Crv2Off);
	MvarMVFree(MVCrv2);
	CagdCrvFree(Crv2Off);
	MVCrv2 = MvarPromoteMVToMV2(MVTemp, 2, 1);

	if (BspGeom)
	    BspKnotAffineTransOrder2(MVCrv2 -> KnotVectors[0],
				     MVCrv2 -> Orders[0],
				     MVCrv2 -> Lengths[0] +
				                          MVCrv2 -> Orders[0],
				     TMin1, TMax1);  
	MvarMVFree(MVTemp);
    }
    else
        OffsetContstraints = FALSE;

    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    Constrs[0] = Constrs[1] = MVAR_CNSTRNT_ZERO;

    if (OffsetContstraints) {
        MVCrvDiff = MvarMVSub(MVCrv1, MVCrv2);

	/* < C1(t) - Offset(C2(r)), C1'(t) >. */
	MVs[2] = MvarMVDotProd(MVCrvDiff, MVDCrv1);

	/* < C1(t) - Offset(C2(r)), C2'(r) >. */
	MVs[3] = MvarMVDotProd(MVCrvDiff, MVDCrv2);

	MvarMVFree(MVCrvDiff);

	Constrs[2] = Constrs[3] = MVAR_CNSTRNT_ZERO_SUBDIV;

	AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 4,
				       MVAR_HF_DIST_SUBDIV_TOL, Epsilon);

	MvarMVFree(MVs[2]);
	MvarMVFree(MVs[3]);
    }
    else {
	AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 2,
				       MVAR_HF_DIST_SUBDIV_TOL, Epsilon);
    }

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);
    MvarMVFree(MVDCrv1);
    MvarMVFree(MVDCrv2);

    return AntiPodalPoints;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Old version of MvarHFDistInterBisectCrvCrvC1 that splits the input       M
* curves at curvature max. locations whereas MvarHFDistInterBisectCrvCrvC1   M
* eliminates the (u - v) terms directly.				     M
*  Computes the intersection locations of (C^1 cont) Crv2 with the self      M
* bisectors of (C^1 cont.) Crv1, if any.  The solution is computed by        M
* solving the following cases:						     M
* 1. The curve-curve self bisector of Crv1, intersected with Crv2:	     M
*     || C2(w) - C1(u) ||^2 == || C2(w) - C1(v) ||^2,		             V
*     < C2(w) - C1(u), C1'(u) > = 0,				             V
*     < C2(w) - C1(v), C1'(v) > = 0.				             V
*    The first equations above (equal distance to two different locations in M
*    C1) could be rewritten as:						     M
*     < C1(u) + C1(v) - 2C2(w), C1(u) - C1(v) > = 0,                         V
*    which hints to the fact that this equation vanish for (u == v).         M
*      To speed up the process we also add a constraint that the distance    M
*    from C2 to its two foot point should be greater than the radius of      M
*    curvature of C1.							     M
* 2. Endpoint-curve self bisectors of Crv1, intersected with Crv2 (2 cases): M
*    Let B(t) be equal to the self bisector of Crv1 with one of its end      M
*    points.  Then, solve for B(t) = C2(w).				     M
* 3. Endpoint-Endpoint self bisectors of Crv1, intersected with Crv2:        M
*    solved as a line (Endpoint-Endpoint bisector) - Crv2 intersection.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:    First curve to intersect its self-bisector with Crv2.	     M
*   Crv2:    Second curve to intersect against the self bisector of Crv1.    M
*   Epsilon:     Tolerance of computation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarHFDistPairParamStruct *:  Linked list of all detected intersections. M
*	     Note each detected intersection holds two parameters of Crv1.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarHFDistInterBisectCrvCrvC1                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistInterBisectCrvCrvC1Crvtr                                       M
*****************************************************************************/
MvarHFDistPairParamStruct *MvarHFDistInterBisectCrvCrvC1Crvtr(
							  CagdCrvStruct *Crv1,
							  CagdCrvStruct *Crv2,
							  CagdRType Epsilon)
{
    int i, Proximity;
    CagdRType TMin1, TMax1, TMin2, TMax2, *R;
    CagdLType Line;
    CagdPType P, Pt1, Pt2;
    CagdVType V;
    CagdPtStruct *Pt, *Pts, *PtsCrvtr;
    MvarPtStruct *InterPts, *IPt;
    CagdCrvStruct *B[2],
	*Crv1a, *Crv1b, *Crv1Segs;
    MvarMVStruct *MVCrv1a, *MVCrv1b, *MVCrv2, *MVDCrv1a, *MVDCrv1b,
	*MVTemp, *MVDiff1, *MVDiff1a, *MVDiff1b,  *MVs[4];
    MvarConstraintType Constrs[4];
    MvarHFDistPairParamStruct *PP,
	*PPList = NULL;

    /* Make sure curves have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_CRV(Crv1))
        Crv1 = CagdCnvrtBzr2BspCrv(Crv1);
    else
        Crv1 = CagdCnvrtBsp2OpenCrv(Crv1);

    if (CAGD_IS_BEZIER_CRV(Crv2))
        Crv2 = CagdCnvrtBzr2BspCrv(Crv2);
    else
	Crv2 = CagdCnvrtBsp2OpenCrv(Crv2);

    /* Split Crv1 at points of maximum curvature.  Then self bisectors in   */
    /* Crv1 will reduce to bisectors between different segments of Crv1.    */
    Pts = NULL;
    PtsCrvtr = SymbCrvExtremCrvtrPts(Crv1, Epsilon);
    while (PtsCrvtr) {
        IRIT_LIST_POP(Pt, PtsCrvtr);
	if (AttrGetIntAttrib(Pt -> Attr, "ExtremType") == 1) {
	    IRIT_LIST_PUSH(Pt, Pts);
	}
	else
	    CagdPtFree(Pt);
    }
    Pts = CagdListReverse(Pts);
    Crv1Segs = CagdCrvSubdivAtParams(Crv1, Pts, Epsilon, &Proximity);
    CagdPtFreeList(Pts);

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    /* Stage 1: Intersect self crv-crv bisector of Crv1 & Crv2: */
    InterPts = NULL;
    for (Crv1a = Crv1Segs; Crv1a != NULL; Crv1a = Crv1a -> Pnext) {
        for (Crv1b = Crv1a -> Pnext; Crv1b != NULL; Crv1b = Crv1b -> Pnext) {
	    CagdRType TMin1a, TMax1a, TMin1b, TMax1b;

	    CagdCrvDomain(Crv1a, &TMin1a, &TMax1a);
	    CagdCrvDomain(Crv1b, &TMin1b, &TMax1b);

	    /* Convert Crv1 and Crv2 to multivars, Crv1a/b in first 2 prms. */
	    MVTemp = MvarCrvToMV(Crv1a);
	    MVCrv1a = MvarPromoteMVToMV2(MVTemp, 3, 0);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv1a, 1, TMin1b, TMax1b);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv1a, 2, TMin2, TMax2);
	    MvarMVFree(MVTemp);

	    MVTemp = MvarCrvToMV(Crv1b);
	    MVCrv1b = MvarPromoteMVToMV2(MVTemp, 3, 1);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv1b, 0, TMin1a, TMax1a);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv1b, 2, TMin2, TMax2);
	    MvarMVFree(MVTemp);

	    MVTemp = MvarCrvToMV(Crv2);
	    MVCrv2 = MvarPromoteMVToMV2(MVTemp, 3, 2);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv2, 0, TMin1a, TMax1a);
	    MVAR_MV_AFFINE_TRANS_KV(MVCrv2, 1, TMin1b, TMax1b);
	    MvarMVFree(MVTemp);

	    /* Differentiate Crv1. */
	    MVDCrv1a = MvarMVDerive(MVCrv1a, 0);
	    MVDCrv1b = MvarMVDerive(MVCrv1b, 1);

	    /* Build the constraints. */
	    MVDiff1 = MvarMVSub(MVCrv1a, MVCrv1b);
	    MVDiff1a = MvarMVSub(MVCrv2, MVCrv1a);
	    MVDiff1b = MvarMVSub(MVCrv2, MVCrv1b);

	    MVTemp = MvarMVAdd(MVDiff1a, MVDiff1b);
	    MVs[0] = MvarMVDotProd(MVDiff1, MVTemp);
	    MVs[1] = MvarMVDotProd(MVDiff1a, MVDCrv1a);
	    MVs[2] = MvarMVDotProd(MVDiff1b, MVDCrv1b);

	    MvarMVFree(MVDiff1);
	    MvarMVFree(MVDiff1a);
	    MvarMVFree(MVDiff1b);
	    MvarMVFree(MVCrv1a);
	    MvarMVFree(MVCrv1b);
	    MvarMVFree(MVCrv2);
	    MvarMVFree(MVDCrv1a);
	    MvarMVFree(MVDCrv1b);

	    Constrs[0] = Constrs[1] = Constrs[2] = MVAR_CNSTRNT_ZERO;
	    IPt = MvarMVsZeros(MVs, Constrs, 3, MVAR_HF_DIST_SUBDIV_TOL,
			       -Epsilon);
	    InterPts = CagdListAppend(IPt, InterPts);

	    MvarMVFree(MVs[0]);
	    MvarMVFree(MVs[1]);
	    MvarMVFree(MVs[2]);
	}
    }

    for (IPt = InterPts; IPt != NULL; IPt = IPt -> Pnext) {
	if (IRIT_APX_EQ_EPS(IPt -> Pt[0], IPt -> Pt[1], Epsilon * 10) ||
	    IPt -> Pt[0] > IPt -> Pt[1])
	    continue;

	PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	PP -> Param1.NumOfParams = 2;
	PP -> Param1.T[0] = IPt -> Pt[0];
	PP -> Param1.T[1] = IPt -> Pt[1];
	PP -> Param2.NumOfParams = 1;
	PP -> Param2.T[0] = IPt -> Pt[2];

	/* Compute distance. */
	R = CagdCrvEval(Crv1, IPt -> Pt[0]);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, IPt -> Pt[2]);
	CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	IRIT_LIST_PUSH(PP, PPList);
    }

    MvarPtFreeList(InterPts);

    /* Stage 2: Intersect the two endpoint-crv bisectors of Crv1 with Crv2: */
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    B[0] = SymbCrvPtBisectorCrv2D(Crv1, P, 0.5);
    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    B[1] = SymbCrvPtBisectorCrv2D(Crv1, P, 0.5);
    for (i = 0; i < 2; i++) {
        Pts = CagdCrvCrvInter(B[i], Crv2, Epsilon);
	for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	    PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	    PP -> Param1.NumOfParams = 2;
	    PP -> Param1.T[0] = i == 0 ? TMin1 : TMax1;
	    PP -> Param1.T[1] = Pt -> Pt[0];
	    PP -> Param2.NumOfParams = 1;
	    PP -> Param2.T[0] = Pt -> Pt[1];

	    /* Compute distance. */
	    R = CagdCrvEval(Crv1, Pt -> Pt[0]);
	    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	    R = CagdCrvEval(Crv2, Pt -> Pt[1]);
	    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	    PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	    IRIT_LIST_PUSH(PP, PPList);
	}

	CagdPtFreeList(Pts);
	CagdCrvFree(B[i]);
    }

    /* Stage 3: Intersect endpoint-endpoint bisector line of Crv1 with Crv2: */
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(V, &R, -1, Crv1 -> PType);
    IRIT_VEC2D_SUB(Line, V, P);
    IRIT_PT2D_BLEND(P, P, V, 0.5);
    Line[2] = -Line[0] * P[0] - Line[1] * P[1];

    Pts = SymbLclDistCrvLine(Crv2, Line, Epsilon, TRUE, FALSE);
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	PP -> Param1.NumOfParams = 2;
	PP -> Param1.T[0] = TMin1;
	PP -> Param1.T[1] = TMax1;
	PP -> Param2.NumOfParams = 1;
	PP -> Param2.T[0] = Pt -> Pt[0];

	/* Compute distance. */
	R = CagdCrvEval(Crv1, TMin1);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, Pt -> Pt[0]);
	CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	IRIT_LIST_PUSH(PP, PPList);
    }

    CagdPtFreeList(Pts);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return PPList;  
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes a (u - v) factor from the given scalar three-multivariate which  M
* is assumed to be a 3-variate F(u, v, w).  The u, v parameters are the      M
* first two parameters of the 3-variate.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   3-variate (rep. as multivariate) to factor out (u - v) term from.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Factored out 3-variate, (rep. as a multivriate).         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorUMinusV, MvarMVBivarFactorUMinusV       M
*   MvarMV4VarFactorUMinusV, MvarMV4VarFactorUMinusR			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMV3VarFactorUMinusV                                                  M
*****************************************************************************/
MvarMVStruct *MvarMV3VarFactorUMinusV(MvarMVStruct *MV)
{
    CagdPointType
        PType = MV -> PType;
    CagdBType
        IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i, k, Len,
        NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	SrfMeshSize = 0;
    CagdSrfStruct *FctrSrf,
	*Srf = NULL;
    MvarMVStruct
	*FctrMV = NULL;

    assert(MV -> Dim == 3);

    for (k = Len = 0; k < MV -> Lengths[2]; k++) {
        if (k == 0) {
	    Srf = MvarMVToSrf(MV);
	    SrfMeshSize = Srf-> ULength * Srf -> VLength;
	}
	else {
	    /* Copy the next bivariate mesh from the MV onto our Srf. */
	    Len += SrfMeshSize;
	    CAGD_GEN_COPY(Srf -> Points[1], &FctrMV -> Points[1][Len],
			  sizeof(CagdRType) * SrfMeshSize);
	}

	FctrSrf = BspSrfFactorUMinusV(Srf);

	if (k == 0) {
	    int Lengths[3], Orders[3];

	    Lengths[0] = FctrSrf -> ULength;
	    Lengths[1] = FctrSrf -> VLength;
	    Lengths[2] = MV -> Lengths[2];
	    Orders[0] = FctrSrf -> UOrder;
	    Orders[1] = FctrSrf -> VOrder;
	    Orders[2] = MV -> Orders[2];
	    FctrMV = MvarBspMVNew(3, Lengths, Orders, MV -> PType);
	    BspKnotCopy(FctrMV -> KnotVectors[0], FctrSrf -> UKnotVector,
			FctrSrf -> UOrder + FctrSrf -> ULength);
	    BspKnotCopy(FctrMV -> KnotVectors[1], FctrSrf -> VKnotVector,
			FctrSrf -> VOrder + FctrSrf -> VLength);
	    BspKnotCopy(FctrMV -> KnotVectors[2], MV -> KnotVectors[2],
			MV -> Orders[2] + MV -> Lengths[2]);
	}

	for (i = !IsRational; i <= NumCoords; i++)
	    CAGD_GEN_COPY(&FctrMV -> Points[i][Len], FctrSrf -> Points[i],
			  sizeof(CagdRType) * FctrSrf -> ULength *
					      FctrSrf -> VLength);
	CagdSrfFree(FctrSrf);
    }

    if (Srf != NULL)
	CagdSrfFree(Srf);

    return FctrMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the intersection locations of (C^1 cont) Crv2 with the self	     M
* bisectors of (C^1 cont.) Crv1, if any.  The solution is computed by        M
* solving the following cases:						     M
* 1. The curve-curve self bisector of Crv1, intersected with Crv2:	     M
*     || C2(w) - C1(u) ||^2 == || C2(w) - C1(v) ||^2,		             V
*     < C2(w) - C1(u), C1'(u) > = 0,				             V
*     < C2(w) - C1(v), C1'(v) > = 0.				             V
*    The first equations above (equal distance to two different locations in M
*    C1) could be rewritten as:						     M
*     < C1(u) + C1(v) - 2C2(w), C1(u) - C1(v) > = 0,                         V
*    which hints to the fact that this equation vanish for (u == v).  Hence, M
*    in the solution process, we eliminate the (u - v) factors from it.	     M
* 2. Endpoint-curve self bisectors of Crv1, intersected with Crv2 (2 cases): M
*    Let B(t) be equal to the self bisector of Crv1 with one of its end      M
*    points.  Then, solve for B(t) = C2(w).				     M
* 3. Endpoint-Endpoint self bisectors of Crv1, intersected with Crv2:        M
*    solved as a line (Endpoint-Endpoint bisector) - Crv2 intersection.      M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1:     First curve to intersect its self-bisector with Crv2.	     M
*   CCrv2:     Second curve to intersect against the self bisector of Crv1.  M
*   Epsilon:   Tolerance of computation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarHFDistPairParamStruct *:  Linked list of all detected intersections. M
*	     Note each detected intersection holds two parameters of Crv1.   M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistInterBisectCrvCrvC1                                            M
*****************************************************************************/
MvarHFDistPairParamStruct *MvarHFDistInterBisectCrvCrvC1(
						   const CagdCrvStruct *CCrv1,
						   const CagdCrvStruct *CCrv2,
						   CagdRType Epsilon)
{
    int i;
    CagdRType TMin1, TMax1, TMin2, TMax2, *R;
    CagdLType Line;
    CagdPType P, Pt1, Pt2;
    CagdVType V;
    CagdPtStruct *Pt, *Pts;
    MvarPtStruct *InterPts, *IPt;
    CagdCrvStruct *B[2], *Crv1, *Crv2;
    MvarMVStruct *MVCrv1a, *MVCrv1b, *MVCrv2, *MVDCrv1a, *MVDCrv1b,
	*MVTemp, *MVDiff1a1b;
    MvarConstraintType Constrs[3];
    MvarHFDistPairParamStruct *PP,
	*PPList = NULL;

    /* Make sure curves have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_CRV(CCrv1))
        Crv1 = CagdCnvrtBzr2BspCrv(CCrv1);
    else
        Crv1 = CagdCnvrtBsp2OpenCrv(CCrv1);

    if (CAGD_IS_BEZIER_CRV(CCrv2))
        Crv2 = CagdCnvrtBzr2BspCrv(CCrv2);
    else
	Crv2 = CagdCnvrtBsp2OpenCrv(CCrv2);

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    /* Convert Crv1 and Crv2 to multivariates, Crv1 in first two             */
    /* parameters.  Crv2 as third parameter.				     */
    MVTemp = MvarCrvToMV(Crv1);
    MVCrv1a = MvarPromoteMVToMV2(MVTemp, 3, 0);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv1a, 1, TMin1, TMax1);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv1a, 2, TMin2, TMax2);
    MvarMVFree(MVTemp);

    MVTemp = MvarCrvToMV(Crv1);
    MVCrv1b = MvarPromoteMVToMV2(MVTemp, 3, 1);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv1b, 0, TMin1, TMax1);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv1b, 2, TMin2, TMax2);
    MvarMVFree(MVTemp);

    MVTemp = MvarCrvToMV(Crv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTemp, 3, 2);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv2, 0, TMin1, TMax1);
    MVAR_MV_AFFINE_TRANS_KV(MVCrv2, 1, TMin1, TMax1);
    MvarMVFree(MVTemp);

    /* Differentiate Crv1. */
    MVDCrv1a = MvarMVDerive(MVCrv1a, 0);
    MVDCrv1b = MvarMVDerive(MVCrv1b, 1);

    /* Build the (C1(u) - C2(v)) and reduce (u - v). */
    MVTemp = MvarMVSub(MVCrv1a, MVCrv1b);
    MVDiff1a1b = MvarMV3VarFactorUMinusV(MVTemp);
    MvarMVFree(MVTemp);

    Constrs[0] = Constrs[1] = Constrs[2] = MVAR_CNSTRNT_ZERO;

#ifdef MVAR_HDIST2D_SOLVE_ET
    /* Convert Crv1 and Crv2 to multivariate expression trees, Crv1 in first */
    /* two parameters.  Crv2 as third parameter.			     */
    {
        MvarExprTreeStruct *MVETs[3], *MVETTmp1, *MVETTmp2;

	MVETTmp1 = MvarExprTreeAdd(MvarExprTreeFromMV(MVCrv1a, 3, 0),
				   MvarExprTreeFromMV(MVCrv1b, 3, 0));
	MVTemp = MvarMVCopy(MVCrv2);
	MvarMVTransform(MVTemp, NULL, 2.0);
	MVETTmp2 = MvarExprTreeSub(MVETTmp1,
				   MvarExprTreeFromMV(MVTemp, 3, 0));
	MvarMVFree(MVTemp);
	MVETs[0] = MvarExprTreeDotProd(MvarExprTreeFromMV(MVDiff1a1b, 3, 0),
				       MVETTmp2);

	MVETTmp1 = MvarExprTreeSub(MvarExprTreeFromMV(MVCrv1a, 3, 0),
				   MvarExprTreeFromMV(MVCrv2, 3, 0));
	MVETs[1] = MvarExprTreeDotProd(MVETTmp1,
				       MvarExprTreeFromMV(MVDCrv1a, 3, 0));

	MVETTmp1 = MvarExprTreeSub(MvarExprTreeFromMV(MVCrv1b, 3, 0),
				   MvarExprTreeFromMV(MVCrv2, 3, 0));
	MVETs[2] = MvarExprTreeDotProd(MVETTmp1,
				       MvarExprTreeFromMV(MVDCrv1b, 3, 0));

	InterPts = MvarExprTreesZeros(MVETs, Constrs, 3,
				      MVAR_HF_DIST_SUBDIV_TOL, -Epsilon);

	for (i = 0; i < 3; i++)
	    MvarExprTreeFree(MVETs[i], FALSE);
    }
#else
    {
        MvarMVStruct *MVDiff21a, *MVDiff21b, *MVs[3];

        /* Build the constraints. */
        MVDiff21a = MvarMVSub(MVCrv2, MVCrv1a);
	MVDiff21b = MvarMVSub(MVCrv2, MVCrv1b);
	MVTemp = MvarMVAdd(MVDiff21a, MVDiff21b);

	MVs[0] = MvarMVDotProd(MVDiff1a1b, MVTemp);
	MVs[1] = MvarMVDotProd(MVDiff21a, MVDCrv1a);
	MVs[2] = MvarMVDotProd(MVDiff21b, MVDCrv1b);

	MvarMVFree(MVTemp);
	MvarMVFree(MVDiff21a);
	MvarMVFree(MVDiff21b);
	MvarMVFree(MVDiff1a1b);
	MvarMVFree(MVCrv1a);
	MvarMVFree(MVCrv1b);
	MvarMVFree(MVCrv2);
	MvarMVFree(MVDCrv1a);
	MvarMVFree(MVDCrv1b);

	InterPts = MvarMVsZeros(MVs, Constrs, 3, MVAR_HF_DIST_SUBDIV_TOL,
				-Epsilon);

	for (i = 0; i < 3; i++)
	    MvarMVFree(MVs[i]);
    }
#endif /* MVAR_HDIST2D_SOLVE_ET */

    for (IPt = InterPts; IPt != NULL; IPt = IPt -> Pnext) {
	if (IRIT_APX_EQ_EPS(IPt -> Pt[0], IPt -> Pt[1], Epsilon * 10) ||
	    IPt -> Pt[0] > IPt -> Pt[1])
	    continue;

	PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	PP -> Param1.NumOfParams = 2;
	PP -> Param1.T[0] = IPt -> Pt[0];
	PP -> Param1.T[1] = IPt -> Pt[1];
	PP -> Param2.NumOfParams = 1;
	PP -> Param2.T[0] = IPt -> Pt[2];

	/* Compute distance. */
	R = CagdCrvEval(Crv1, IPt -> Pt[0]);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, IPt -> Pt[2]);
	CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	IRIT_LIST_PUSH(PP, PPList);
    }

    MvarPtFreeList(InterPts);

    /* Stage 2: Intersect the two endpoint-crv bisectors of Crv1 with Crv2: */
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    B[0] = SymbCrvPtBisectorCrv2D(Crv1, P, 0.5);
    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    B[1] = SymbCrvPtBisectorCrv2D(Crv1, P, 0.5);
    for (i = 0; i < 2; i++) {
        Pts = CagdCrvCrvInter(B[i], Crv2, Epsilon);
	for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	    PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	    PP -> Param1.NumOfParams = 2;
	    PP -> Param1.T[0] = i == 0 ? TMin1 : TMax1;
	    PP -> Param1.T[1] = Pt -> Pt[0];
	    PP -> Param2.NumOfParams = 1;
	    PP -> Param2.T[0] = Pt -> Pt[1];

	    /* Compute distance. */
	    R = CagdCrvEval(Crv1, Pt -> Pt[0]);
	    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	    R = CagdCrvEval(Crv2, Pt -> Pt[1]);
	    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	    PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	    IRIT_LIST_PUSH(PP, PPList);
	}

	CagdPtFreeList(Pts);
	CagdCrvFree(B[i]);
    }

    /* Stage 3: Intersect endpoint-endpoint bisector line of Crv1 with Crv2: */
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(V, &R, -1, Crv1 -> PType);
    IRIT_VEC2D_SUB(Line, V, P);
    IRIT_PT2D_BLEND(P, P, V, 0.5);
    Line[2] = -Line[0] * P[0] - Line[1] * P[1];

    Pts = SymbLclDistCrvLine(Crv2, Line, Epsilon, TRUE, FALSE);
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	PP = (MvarHFDistPairParamStruct *)
				IritMalloc(sizeof(MvarHFDistPairParamStruct));
	PP -> Param1.NumOfParams = 2;
	PP -> Param1.T[0] = TMin1;
	PP -> Param1.T[1] = TMax1;
	PP -> Param2.NumOfParams = 1;
	PP -> Param2.T[0] = Pt -> Pt[0];

	/* Compute distance. */
	R = CagdCrvEval(Crv1, TMin1);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, Pt -> Pt[0]);
	CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	PP -> Dist = IRIT_PT_PT_DIST(Pt1, Pt2);

	IRIT_LIST_PUSH(PP, PPList);
    }

    CagdPtFreeList(Pts);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return PPList;  
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the one sided Hausdorff distance between two C^1 cont. curves,  M
* from C1 to C2.							     M
*   The curves are assumed to be either in R2 or R3.			     M
*   The one sided extreme distance between two curves could happen at:       M
* + The end points of curve C1 or curve C2.				     M
* + Antipodal locations or locations where:				     M
*     C1'(t) <C1(t) - C2(r)> = 0,					     V
*     C2'(r) <C1(t) - C2(r)> = 0,					     V
*   that are also global distance minima from C1(t) to any point on C2.	     M
* + Locations where C1 crosses the self bisector of C2 that are also local   M
*   distance minima from C1 to any point on C2.				     M
*									     M
*   All the above equations hold for R2 and for R3.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1:     First Crv to measure its hausdorff distance to CCrv2.	     M
*   CCrv2:     Second Crv to measure its hausdorff distance from CCrv1.	     M
*   Param1:   Where to return the parameter value(s) of Crv1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Crv2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Epsilon:  Tolerance of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistFromCrvToCrvC1                                                 M
*****************************************************************************/
CagdRType MvarHFDistFromCrvToCrvC1(const CagdCrvStruct *CCrv1,
				   const CagdCrvStruct *CCrv2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2,
				   CagdRType Epsilon)
{
    CagdBType BspGeom;
    CagdRType *R, TMin1, TMax1, Dist,
	MaxDist = 0.0;
    CagdPType P, Pt1, Pt2;
    MvarPtStruct *AntiPodalPoints, *MVPt;
    MvarHFDistPairParamStruct *PPList, *PP;
    MvarHFDistParamStruct Param;
    CagdCrvStruct *Crv1, *Crv2;

    Param1 -> NumOfParams = 1;
    Param2 -> NumOfParams = 1;

    /* Make sure curves have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_CRV(CCrv1)) {
        if (CAGD_IS_BSPLINE_CRV(CCrv2))
	    Crv1 = CagdCnvrtBzr2BspCrv(CCrv1);
	else
	    Crv1 = CagdCrvCopy(CCrv1);
    }
    else
        Crv1 = CagdCnvrtBsp2OpenCrv(CCrv1);

    if (CAGD_IS_BEZIER_CRV(CCrv2)) {
        if (CAGD_IS_BSPLINE_CRV(CCrv1))
	    Crv2 = CagdCnvrtBzr2BspCrv(CCrv2);
	else
	    Crv2 = CagdCrvCopy(CCrv2);
    }
    else
	Crv2 = CagdCnvrtBsp2OpenCrv(CCrv2);

    BspGeom = CAGD_IS_BSPLINE_CRV(Crv1);

    /* Stage 1. Examine end points of Crv1 vs. curve Crv2. */
    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    if (MaxDist < (Dist = MvarDistPointCrvC1(P, Crv2, &Param,
					     TRUE, Epsilon))) {
	MaxDist = Dist;
	*Param2 = Param;
	Param1 -> T[0] = TMin1;
	Param1 -> NumOfParams = 1;
	Param1 -> ManifoldDim = 1;

    }
    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(P, &R, -1, Crv1 -> PType);
    if (MaxDist < (Dist = MvarDistPointCrvC1(P, Crv2, &Param,
					     TRUE, Epsilon))) {
	MaxDist = Dist;
	*Param2 = Param;
	Param1 -> T[0] = TMax1;
	Param1 -> NumOfParams = 1;
	Param1 -> ManifoldDim = 1;
    }

    /* Stage 1. Examine end points of Crv2 vs. curve Crv1. */
    CagdCrvDomain(Crv2, &TMin1, &TMax1);
    R = CagdCrvEval(Crv2, TMin1);
    CagdCoerceToE3(P, &R, -1, Crv2 -> PType);
    if (MaxDist < (Dist = MvarHFExtremeLclDistPointCrvC1(P, Crv2, Crv1,
							 &Param, Epsilon))) {
	MaxDist = Dist;
	*Param1 = Param;
	Param2 -> T[0] = TMin1;
	Param2 -> NumOfParams = 1;
	Param2 -> ManifoldDim = 1;

    }
    R = CagdCrvEval(Crv2, TMax1);
    CagdCoerceToE3(P, &R, -1, Crv2 -> PType);
    if (MaxDist < (Dist = MvarHFExtremeLclDistPointCrvC1(P, Crv2, Crv1,
							 &Param, Epsilon))) {
	MaxDist = Dist;
	*Param1 = Param;
	Param2 -> T[0] = TMax1;
	Param2 -> NumOfParams = 1;
	Param2 -> ManifoldDim = 1;
    }

    /* Stage 2. Solve for the antipodal locations. */
    AntiPodalPoints = MvarHFDistAntipodalCrvCrvC1(Crv1, Crv2, Epsilon);

    /* Examine all antipodal points we got. */
    for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
        R = CagdCrvEval(Crv1, MVPt -> Pt[0]);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
        R = CagdCrvEval(Crv2, MVPt -> Pt[1]);
	CagdCoerceToE3(Pt2, &R, -1, Crv1 -> PType);

	if (MaxDist < (Dist = IRIT_PT_PT_DIST(Pt1, Pt2))) {
	    MvarHFDistParamStruct TParam;
	    CagdRType
	        MinDist = MvarDistPointCrvC1(Pt1, Crv2, &TParam,
					     TRUE, Epsilon);

	    /* Accept this solution only if a local minima. */
	    if (IRIT_APX_EQ_EPS(MinDist, Dist, Epsilon * 10)) {
	        MaxDist = Dist;
		Param1 -> T[0] = MVPt -> Pt[0];
		Param2 -> T[0] = MVPt -> Pt[1];
		Param1 -> ManifoldDim = Param2 -> ManifoldDim = 1;
		Param1 -> NumOfParams =	Param2 -> NumOfParams = 1;
	    }
	}
    }
    MvarPtFreeList(AntiPodalPoints);

    /* Stage 3. Compute the intersections of C1 with self bisector of C2. */
    PPList = MvarHFDistInterBisectCrvCrvC1(Crv2, Crv1, Epsilon);

    /* Examine all C1 -- self-bisectors-of-C2 intersection locations.     */
    /* Note Crv1 and Crv2 are reversed here for proper function call.     */
    while (PPList) {
        IRIT_LIST_POP(PP, PPList);

        R = CagdCrvEval(Crv1, PP -> Param2.T[0]);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
        R = CagdCrvEval(Crv2, PP -> Param1.T[0]);
	CagdCoerceToE3(Pt2, &R, -1, Crv1 -> PType);

        if (MaxDist < (Dist = IRIT_PT_PT_DIST(Pt1, Pt2))) {
	    MvarHFDistParamStruct TParam;
	    CagdRType
	        MinDist = MvarDistPointCrvC1(Pt1, Crv2, &TParam,
					     TRUE, Epsilon);

	    /* Accept this solution only if a local minima. */
	    if (IRIT_APX_EQ_EPS(MinDist, Dist, Epsilon * 10)) {
	        MaxDist = Dist;
		Param1 -> T[0] = PP -> Param2.T[0];
		Param2 -> T[0] = PP -> Param1.T[0];
		Param2 -> T[1] = PP -> Param1.T[1];
		Param1 -> NumOfParams = 1;
		Param2 -> NumOfParams = 2;
		Param1 -> ManifoldDim = Param2 -> ManifoldDim = 1;
	    }
	}

	IritFree(PP);
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Hausdorff distance between two C^1 cont. curves, C1 and C2. M
*   The curves are assumed to be either in R2 or R3.			     M
*   The extreme distance between two curves could happen at:                 M
* + The end points of the curves.					     M
* + Antipodal locations where:						     M
*     C1'(t) <C1(t) - C2(r)> = 0,					     V
*     C2'(r) <C1(t) - C2(r)> = 0.					     V
* + Locations where C1 crosses the self bisector of C2 (or vice versa):      M
*   Let Bi(x,y) = 0 be self bisector of Ci(t), and Cj(r) = (xj(r), yj(r)).   M
*   Then, solve for:						             M
*     || C2(r) - C1(s) ||^2 == || C2(r) - C1(t) ||^2,		             V
*     < C2(r) - C1(s), C1'(s) > = 0,				             V
*     < C2(r) - C1(t), C1'(t) > = 0.				             V
*   All the above equations hold for R2 and for R3.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:     First Crv to measure its hausdorff distance to Crv2.	     M
*   Crv2:     Second Crv to measure its hausdorff distance to Crv1.	     M
*   Param1:   Where to return the parameter value(s) of Crv1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Crv2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Epsilon:  Tolerance of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistCrvCrvC1                                                       M
*****************************************************************************/
CagdRType MvarHFDistCrvCrvC1(const CagdCrvStruct *Crv1,
			     const CagdCrvStruct *Crv2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2,
			     CagdRType Epsilon)
{
    MvarHFDistParamStruct ParamA1, ParamA2, ParamB1, ParamB2;
    CagdRType
	Dist1 = MvarHFDistFromCrvToCrvC1(Crv1, Crv2,
					 &ParamA1, &ParamA2, Epsilon),
	Dist2 = MvarHFDistFromCrvToCrvC1(Crv2, Crv1,
					 &ParamB2, &ParamB1, Epsilon);


    if (Dist1 > Dist2) {
	*Param1 = ParamA1;
	*Param2 = ParamA2;
	return Dist1;
    }
    else {
	*Param1 = ParamB1;
	*Param2 = ParamB2;
	return Dist2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal distance between two given planar C1 curves.        M
*   This minimal distance can occur:					     M
*	1. At intersection locations, if any.				     M
*	2. At end points vs. end points.				     M
*	3. At the end points vs interior locations.			     M
*	4. At antipodal interior locations.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:         To detect its minimal distance to Crv2.                    M
*   Crv2:         To detect its minimal distance to Crv1.                    M
*   MinDist:	  Upon return, is set to the minimal distance detected.      M
*   ComputeAntipodals:  TRUE to compute antipodal locations as well in the   M
*		  search for minimal distance.  If FALSE, samples are made   M
*		  along both curves and closest locations to other curve     M
*		  are computed.						     M
*   Eps:          Numeric tolerance of the computation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Pairs of parameters at the minimal distance.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvAntipodalPoints, SymbDistCrvPoint, CagdCrvCrvInter             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvCrvMinimalDist                                                    M
*****************************************************************************/
MvarPtStruct *MvarCrvCrvMinimalDist(const CagdCrvStruct *Crv1,
				    const CagdCrvStruct *Crv2,
				    CagdRType *MinDist,
				    CagdBType ComputeAntipodals,
				    CagdRType Eps)
{
    int Dim1 = CAGD_NUM_OF_PT_COORD(Crv1 -> PType),
        Dim2 = CAGD_NUM_OF_PT_COORD(Crv2 -> PType);
    CagdRType *R, t, Dt, TMin1, TMax1, TMin2, TMax2, MinDistParam;
    CagdPType Pt;
    CagdPtStruct *CPts;
    MvarPtStruct *MPts, *MPt, *RetVal;
    CagdCrvStruct
        *CpCrv1 = NULL,
        *CpCrv2 = NULL;

    if (Dim1 != Dim2) {
        if (Dim1 < Dim2) {
	    Crv1 = CpCrv1 = 
	        CagdCoerceCrvTo(Crv1,
				CAGD_MAKE_PT_TYPE(CAGD_IS_RATIONAL_CRV(Crv1),
						  Dim2),
				FALSE);
	}
	else {
	    Crv2 = CpCrv2 =
	        CagdCoerceCrvTo(Crv2,
				CAGD_MAKE_PT_TYPE(CAGD_IS_RATIONAL_CRV(Crv2),
						  Dim1),
				FALSE);
	}
    }

    /* 1. Test for possible intersection locations. */
    if ((CPts = CagdCrvCrvInter(Crv1, Crv2, Eps)) != NULL) {
        MPt = MvarPtNew(2);
	MPt -> Pt[0] = CPts -> Pt[0];
	MPt -> Pt[1] = CPts -> Pt[1];
	CagdPtFreeList(CPts);

	*MinDist = 0.0;
	if (CpCrv1 != NULL)
	    CagdCrvFree(CpCrv1);
	if (CpCrv2 != NULL)
	    CagdCrvFree(CpCrv2);
	return MPt;
    }

    MPts = NULL;
    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    /* 2. Set to check End points vs. End Points. */
    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMin1;
    MPt -> Pt[1] = TMin2;
    IRIT_LIST_PUSH(MPt, MPts);

    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMin1;
    MPt -> Pt[1] = TMax2;
    IRIT_LIST_PUSH(MPt, MPts);

    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMax1;
    MPt -> Pt[1] = TMin2;
    IRIT_LIST_PUSH(MPt, MPts);

    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMax1;
    MPt -> Pt[1] = TMax2;
    IRIT_LIST_PUSH(MPt, MPts);

    /* 3. Set to check End points vs. Interior points. */
    R = CagdCrvEval(Crv1, TMin1);
    CagdCoerceToE3(Pt, &R, -1, Crv1 -> PType);
    MinDistParam = SymbDistCrvPoint(Crv2, Pt, TRUE, Eps);
    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMin1;
    MPt -> Pt[1] = MinDistParam;
    IRIT_LIST_PUSH(MPt, MPts);

    R = CagdCrvEval(Crv1, TMax1);
    CagdCoerceToE3(Pt, &R, -1, Crv1 -> PType);
    MinDistParam = SymbDistCrvPoint(Crv2, Pt, TRUE, Eps);
    MPt = MvarPtNew(2);
    MPt -> Pt[0] = TMax1;
    MPt -> Pt[1] = MinDistParam;
    IRIT_LIST_PUSH(MPt, MPts);

    R = CagdCrvEval(Crv2, TMin2);
    CagdCoerceToE3(Pt, &R, -1, Crv2 -> PType);
    MinDistParam = SymbDistCrvPoint(Crv1, Pt, TRUE, Eps);
    MPt = MvarPtNew(2);
    MPt -> Pt[0] = MinDistParam;
    MPt -> Pt[1] = TMin2;
    IRIT_LIST_PUSH(MPt, MPts);

    R = CagdCrvEval(Crv2, TMax2);
    CagdCoerceToE3(Pt, &R, -1, Crv2 -> PType);
    MinDistParam = SymbDistCrvPoint(Crv1, Pt, TRUE, Eps);
    MPt = MvarPtNew(2);
    MPt -> Pt[0] = MinDistParam;
    MPt -> Pt[1] = TMax2;
    IRIT_LIST_PUSH(MPt, MPts);

    /* Add the antipodal points. */
    if (ComputeAntipodals) {
        MPt = MvarHFDistAntipodalCrvCrvC1(Crv1, Crv2, Eps);
	MPts = CagdListAppend(MPt, MPts);
    }
    else {
        /* Sample points along one curve and compute minimal distance to */
        /* the other, and vice versa.					 */
        Dt = (TMax1 - TMin1) / MVAR_MIN_DIST_SAMPLES;
	for (t = TMin1 + Dt * 0.5; t < TMax1; t += Dt) {
	    R = CagdCrvEval(Crv1, t);
	    CagdCoerceToE3(Pt, &R, -1, Crv1 -> PType);
	    MinDistParam = SymbDistCrvPoint(Crv2, Pt, TRUE, Eps);
	    MPt = MvarPtNew(2);
	    MPt -> Pt[0] = t;
	    MPt -> Pt[1] = MinDistParam;
	    IRIT_LIST_PUSH(MPt, MPts);
	}

	Dt = (TMax2 - TMin2) / MVAR_MIN_DIST_SAMPLES;
	for (t = TMin2 + Dt * 0.5; t < TMax2; t += Dt) {
	    R = CagdCrvEval(Crv2, t);
	    CagdCoerceToE3(Pt, &R, -1, Crv2 -> PType);
	    MinDistParam = SymbDistCrvPoint(Crv1, Pt, TRUE, Eps);
	    MPt = MvarPtNew(2);
	    MPt -> Pt[0] = MinDistParam;
	    MPt -> Pt[1] = t;
	    IRIT_LIST_PUSH(MPt, MPts);
	}
    }

    /* Time to examine all the events we collected.  Find the minimum       */
    /* distance and return it as the minimal distance between this pair.    */
    RetVal = MvarPtNew(2);
    *MinDist = IRIT_INFNTY;
    for (MPt = MPts; MPt != NULL; MPt = MPt -> Pnext) {
        CagdRType d;
        CagdPType Pt1, Pt2;

        R = CagdCrvEval(Crv1, MPt -> Pt[0]);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
        R = CagdCrvEval(Crv2, MPt -> Pt[1]);
	CagdCoerceToE3(Pt2, &R, -1, Crv1 -> PType);

	d = IRIT_PT_PT_DIST(Pt1, Pt2);

	if (*MinDist > d) {
	    *MinDist = d;
	    IRIT_PT2D_COPY(RetVal -> Pt, MPt -> Pt);
	}
    }
    MvarPtFreeList(MPts);

    if (CpCrv1 != NULL)
        CagdCrvFree(CpCrv1);
    if (CpCrv2 != NULL)
        CagdCrvFree(CpCrv2);

    return RetVal;
}
