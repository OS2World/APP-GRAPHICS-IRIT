/******************************************************************************
* SBsp-Aux.c - Bspline surface auxilary routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define BSP_MESH_NORMAL_SRF_THRU_CRVS
#define SRF_TANGENT_MIN_LEN		1e-10

/* Define some marcos to make some of the routines below look better. They  */
/* calculate the index of the U, V point of the control mesh in Points.	    */
#define DERIVED_SRF(U, V)	CAGD_MESH_UV(DerivedSrf, U, V)
#define INT_SRF(U, V)		CAGD_MESH_UV(IntSrf, U, V)
#define RAISED_SRF(U, V)	CAGD_MESH_UV(RaisedSrf, U, V)
#define SRF(U, V)		CAGD_MESH_UV(Srf, U, V)

#define U_INDEX_MIN(i)		(i > 0 ? i - 1 : (UClosed ? ULength - 1 : 0))
#define U_INDEX_MAX(i)		(i < ULength - 1 ? i + 1 : (UClosed ? 0 : i))
#define V_INDEX_MIN(j)		(j > 0 ? j - 1 : (VClosed ? VLength - 1 : 0))
#define V_INDEX_MAX(j)		(j < VLength - 1 ? j + 1 : (VClosed ? 0 : j))

#define MOEBIUS_MEU(t)		(1 + (t) * (1 - c) / c)
#define MOEBIUS_REPARAM(t)	((t) / ((t) + c * (1 - (t))))

#define NORMAL_IRIT_EPS		1e-4

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

#ifdef BSP_MESH_NORMAL_SRF_THRU_CRVS
static CagdBType EvalTangentVector(CagdVType Vec,
				   CagdCrvStruct *Crv,
				   CagdRType t,
				   CagdRType TMin,
				   CagdRType TMax);
#endif /* BSP_MESH_NORMAL_SRF_THRU_CRVS */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline surface - subdivides it into two sub-surfaces at the given M
* parametric value.                                                          M
*   Returns pointer to first surface in a list of two subdivided surfaces.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To subdivide at parameter value t.                             M
*   t:        Parameter value to subdivide Srf at.                           M
*   Dir:      Direction of subdivision. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A list of the two subdivided surfaces.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfSubdivAtParam, BzrSrfSubdivAtParam, TrimSrfSubdivAtParam          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfSubdivAtParam, subdivision, refinement                             M
*****************************************************************************/
CagdSrfStruct *BspSrfSubdivAtParam(const CagdSrfStruct *Srf,
				   CagdRType t,
				   CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Srf);
    int i, j, Row, Col, KVLen, Index1, Index2, Mult,
	LULength, RULength, LVLength, RVLength,	ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *RefKV, **LPts, **RPts,
    	UMin, UMax, VMin, VMax;
    CagdRType * const *Pts;
    CagdSrfStruct *RSrf, *LSrf, *CpSrf;
    BspKnotAlphaCoeffStruct *A;

    if (CAGD_IS_PERIODIC_SRF(Srf))
	Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    ULength = Srf -> ULength;
    VLength = Srf -> VLength;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    RefKV = Srf -> UKnotVector;
	    KVLen = UOrder + ULength;

	    i = BspKnotLastIndexLE(RefKV, KVLen, t);
	    if (IRIT_APX_EQ_EPS(t, RefKV[i], CAGD_EPS_ROUND_KNOT))
		t = RefKV[i];
	    else if (i + 1 < KVLen && IRIT_APX_EQ_EPS(t, RefKV[i + 1],
						 CAGD_EPS_ROUND_KNOT))
		t = RefKV[i + 1];

	    Index1 = BspKnotLastIndexL(RefKV, KVLen, t);
	    if (Index1 + 1 < UOrder)
		Index1 = UOrder - 1;
	    Index2 = BspKnotFirstIndexG(RefKV, KVLen, t);
	    if (Index2 > ULength)
		Index2 = ULength;
	    LSrf = BspSrfNew(Index1 + 1, VLength,
			     UOrder, VOrder, Srf -> PType);
	    RSrf = BspSrfNew(ULength - Index2 + UOrder, VLength,
			     UOrder, VOrder, Srf -> PType);
	    Mult = UOrder - 1 - (Index2 - Index1 - 1);

	    /* Update the new knot vectors. */
	    CAGD_GEN_COPY(LSrf -> UKnotVector,
			  Srf -> UKnotVector,
			  sizeof(CagdRType) * (Index1 + 1));
	    /* Close the knot vector with multiplicity Order: */
	    for (j = Index1 + 1; j <= Index1 + UOrder; j++)
		LSrf -> UKnotVector[j] = t;
	    CAGD_GEN_COPY(&RSrf -> UKnotVector[UOrder],
			  &Srf -> UKnotVector[Index2],
			  sizeof(CagdRType) * (ULength + UOrder - Index2));
	    /* Make sure knot vector starts with multiplicity Order: */
	    for (j = 0; j < UOrder; j++)
		RSrf -> UKnotVector[j] = t;

	    /* And copy the other direction knot vectors. */
	    CAGD_GEN_COPY(LSrf -> VKnotVector,
			  Srf -> VKnotVector,
			  sizeof(CagdRType) * (VOrder + VLength));
	    CAGD_GEN_COPY(RSrf -> VKnotVector,
			  Srf -> VKnotVector,
			  sizeof(CagdRType) * (VOrder + VLength));
	    break;
	case CAGD_CONST_V_DIR:
	    RefKV = Srf -> VKnotVector;
	    KVLen = VOrder + VLength;

	    i = BspKnotLastIndexLE(RefKV, KVLen, t);
	    if (IRIT_APX_EQ_EPS(t, RefKV[i], CAGD_EPS_ROUND_KNOT))
		t = RefKV[i];
	    else if (i + 1 < KVLen && IRIT_APX_EQ_EPS(t, RefKV[i + 1],
						 CAGD_EPS_ROUND_KNOT))
		t = RefKV[i + 1];

	    Index1 = BspKnotLastIndexL(RefKV, KVLen, t);
	    if (Index1 + 1 < VOrder)
		Index1 = VOrder - 1;
	    Index2 = BspKnotFirstIndexG(RefKV, KVLen, t);
	    if (Index2 > VLength)
		Index2 = VLength;
	    LSrf = BspSrfNew(ULength, Index1 + 1,
			     UOrder, VOrder, Srf -> PType);
	    RSrf = BspSrfNew(ULength, VLength - Index2 + VOrder,
			     UOrder, VOrder, Srf -> PType);
	    Mult = VOrder - 1 - (Index2 - Index1 - 1);

	    /* Update the new knot vectors. */
	    CAGD_GEN_COPY(LSrf -> VKnotVector,
			  Srf -> VKnotVector,
			  sizeof(CagdRType) * (Index1 + 1));
	    /* Close the knot vector with multiplicity Order: */
	    for (j = Index1 + 1; j <= Index1 + VOrder; j++)
		LSrf -> VKnotVector[j] = t;
	    CAGD_GEN_COPY(&RSrf -> VKnotVector[VOrder],
			  &Srf -> VKnotVector[Index2],
			  sizeof(CagdRType) * (VLength + VOrder - Index2));
	    /* Make sure knot vector starts with multiplicity Order: */
	    for (j = 0; j < VOrder; j++)
		RSrf -> VKnotVector[j] = t;
	    
	    /* And copy the other direction knot vectors. */
	    CAGD_GEN_COPY(LSrf -> UKnotVector,
			  Srf -> UKnotVector,
			  sizeof(CagdRType) * (UOrder + ULength));
	    CAGD_GEN_COPY(RSrf -> UKnotVector,
			  Srf -> UKnotVector,
			  sizeof(CagdRType) * (UOrder + ULength));
	    break;
	default:
	    Mult = 1;
	    RefKV = NULL;
	    LSrf = RSrf = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    Pts = Srf -> Points;
    LPts = LSrf -> Points;
    RPts = RSrf -> Points;
    LULength = LSrf -> ULength,
    RULength = RSrf -> ULength;
    LVLength = LSrf -> VLength,
    RVLength = RSrf -> VLength;

    BspKnotMakeRobustKV(RSrf -> UKnotVector,
			RSrf -> UOrder + RSrf -> ULength);
    BspKnotMakeRobustKV(RSrf -> VKnotVector,
			RSrf -> VOrder + RSrf -> VLength);

    BspKnotMakeRobustKV(LSrf -> UKnotVector,
			LSrf -> UOrder + LSrf -> ULength);
    BspKnotMakeRobustKV(LSrf -> VKnotVector,
			LSrf -> VOrder + LSrf -> VLength);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    /* If is a Bezier knot sequence, use simpler Bezier subdivision. */
	    if (BspKnotHasBezierKV(RefKV, Srf -> ULength, Srf -> UOrder)) {
		BzrSrfSubdivCtlMesh(Pts, LPts, RPts, Srf -> ULength,
				    Srf -> VLength, Srf -> PType,
				    (t - UMin) / (UMax - UMin), Dir);
	    }
	    else {
	        /* Do the B-spline div. - compute Alpha refinement matrix.  */
	        if (Mult > 0) {
		    CagdRType
		        *NewKV = (CagdRType *)
		            IritMalloc(sizeof(CagdRType) * Mult);

		    CAGD_DOMAIN_T_VERIFY(t, UMin, UMax);
		    if (t == UMax)
		        t -= CAGD_DOMAIN_IRIT_EPS;
		    for (i = 0; i < Mult; i++)
		        NewKV[i] = t;
		    A = BspKnotEvalAlphaCoefMerge(UOrder, RefKV, ULength,
						  NewKV, Mult, FALSE);
		    IritFree(NewKV);
		}
		else
		    A = BspKnotEvalAlphaCoefMerge(UOrder, RefKV, ULength,
						  NULL, 0, FALSE);

		/* Now work on the two new surfaces meshes. */

		/* Note that Mult can be negative in cases where original    */
		/* multiplicity was order or more and we need to compensate  */
		/* here, since Alpha matrix will be just a unit matrix then. */
		Mult = Mult >= 0 ? 0 : -Mult;

		for (Row = 0; Row < VLength; Row++) {
		    /* Blend Srf into LSrf. */
		    for (j = IsNotRational; j <= MaxCoord; j++) {
		        BspKnotAlphaLoopBlendNotPeriodic(A, 0, LULength,
						     &Pts[j][Row * ULength],
						     &LPts[j][Row * LULength]);
		    }

		    /* Blend Srf into RSrf. */
		    for (j = IsNotRational; j <= MaxCoord; j++) {
		        BspKnotAlphaLoopBlendNotPeriodic(A,
			     LSrf -> ULength - 1 + Mult,
			     LSrf -> ULength + RSrf -> ULength - 1 + Mult,
			     &Pts[j][Row * ULength], &RPts[j][Row * RULength]);
		    }
		}

		BspKnotFreeAlphaCoef(A);
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    /* If is a Bezier knot sequence, use simpler Bezier subdivision. */
	    if (BspKnotHasBezierKV(RefKV, Srf -> VLength, Srf -> VOrder)) {
		BzrSrfSubdivCtlMesh(Pts, LPts, RPts, Srf -> ULength,
				    Srf -> VLength, Srf -> PType,
				    (t - VMin) / (VMax - VMin), Dir);
	    }
	    else {
	        /* Do the B-spline div. - compute Alpha refinement matrix.  */
	        if (Mult > 0) {
		    CagdRType
		        *NewKV = (CagdRType *)
		            IritMalloc(sizeof(CagdRType) * Mult);

		    CAGD_DOMAIN_T_VERIFY(t, VMin, VMax);
		    if (t == VMax)
		        t -= CAGD_DOMAIN_IRIT_EPS;
		    for (i = 0; i < Mult; i++)
		        NewKV[i] = t;
		    A = BspKnotEvalAlphaCoefMerge(VOrder, RefKV, VLength,
						  NewKV, Mult, FALSE);
		    IritFree(NewKV);
		}
		else
		    A = BspKnotEvalAlphaCoefMerge(VOrder, RefKV, VLength,
						  NULL, 0, FALSE);

		/* Now work on the two new surfaces meshes. */

		/* Note that Mult can be negative in cases where original    */
		/* multiplicity was order or more and we need to compensate  */
		/* here, since Alpha matrix will be just a unit matrix then. */
		Mult = Mult >= 0 ? 0 : -Mult;

		for (Col = 0; Col < ULength; Col++) {
		    LULength = LSrf -> ULength;
		    RULength = RSrf -> ULength;

		    /* Blend Srf into LSrf. */
		    for (j = IsNotRational; j <= MaxCoord; j++) {
		        BspKnotAlphaLoopBlendStep(A, 0, LVLength,
						  &Pts[j][Col], ULength, -1,
						  &LPts[j][Col], LULength);
		    }

		    /* Blend Srf into RSrf. */
		    for (j = IsNotRational; j <= MaxCoord; j++) {
		        BspKnotAlphaLoopBlendStep(A, LVLength - 1 + Mult,
					      LVLength + RVLength - 1 + Mult,
					      &Pts[j][Col], ULength, -1,
					      &RPts[j][Col], RULength);
		    }
		}

		BspKnotFreeAlphaCoef(A);
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    LSrf -> Pnext = RSrf;

    CAGD_PROPAGATE_ATTR(LSrf, Srf);
    CAGD_PROPAGATE_ATTR(RSrf, Srf);

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return LSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Inserts n knot, all with the value t in direction Dir. In no case will    M
* the multiplicity of a knot be greater or equal to the curve order.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To refine by insertion (upto) n knot of value t.             M
*   Dir:        Direction of refinement. Either U or V.                      M
*   t:          Parameter value of new knot to insert.                       M
*   n:          Maximum number of times t should be inserted.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Refined Srf with n knots of value t in direction Dir.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfKnotInsertNSame, refinement, subdivision                           M
*****************************************************************************/
CagdSrfStruct *BspSrfKnotInsertNSame(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdRType t,
				     int n)
{
    int i, CrntMult, Mult;
    CagdSrfStruct *RefinedSrf, *CpSrf;

    if (CAGD_IS_PERIODIC_SRF(Srf))
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    CrntMult = BspKnotFindMult(Srf -> UKnotVector, Srf -> UOrder,
							 Srf -> ULength, t),
	    Mult = IRIT_MIN(n, Srf -> UOrder - CrntMult - 1);
	    break;
	case CAGD_CONST_V_DIR:
	    CrntMult = BspKnotFindMult(Srf -> VKnotVector, Srf -> VOrder,
							 Srf -> VLength, t),
	    Mult = IRIT_MIN(n, Srf -> VOrder - CrntMult - 1);
	    break;
	default:
	    Mult = 0;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (Mult > 0) {
	CagdRType
	    *NewKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * Mult);

	for (i = 0; i < Mult; i++)
	    NewKV[i] = t;

	RefinedSrf = BspSrfKnotInsertNDiff(Srf, Dir, FALSE, NewKV, Mult);

	IritFree(NewKV);
    }
    else {
	RefinedSrf = CagdSrfCopy(Srf);
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return RefinedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Inserts n knot with different values as defined by the vector t. If,      M
* however, Replace is TRUE, the knot are simply replacing the current knot   M
* vector.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To refine by insertion (upto) n knot of value t.             M
*   Dir:        Direction of refinement. Either U or V.                      M
*   Replace:    if TRUE, the n knots in t should replace the knot vector     M
*               of size n of Srf. Sizes must match. If False, n new knots    M
*               as defined by t will be introduced into Srf.                 M
*   t:          New knots to introduce/replace knot vector of Srf.           M
*   n:          Size of t.                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Refined Srf with n new knots in direction Dir.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfKnotInsertNDiff, refinement, subdivision                           M
*****************************************************************************/
CagdSrfStruct *BspSrfKnotInsertNDiff(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     int Replace,
				     CagdRType *t,
				     int n)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, Row, Col, ULen, VLen, ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*RefSrf = NULL;

    ULen = Srf -> ULength;
    VLen = Srf -> VLength;
    ULength = CAGD_SRF_UPT_LST_LEN(Srf);
    VLength = CAGD_SRF_VPT_LST_LEN(Srf);

    if (Replace) {
	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1])
		CAGD_FATAL_ERROR(CAGD_ERR_KNOT_NOT_ORDERED);

    	switch (Dir) {
	    case CAGD_CONST_U_DIR:
		if (UOrder + ULength != n)
		    CAGD_FATAL_ERROR(CAGD_ERR_NUM_KNOT_MISMATCH);

		RefSrf = CagdSrfCopy(Srf);
		for (i = 0; i < n; i++)
		    RefSrf -> UKnotVector[i] = *t++;
		break;
	    case CAGD_CONST_V_DIR:
		if (VOrder + VLength != n)
		    CAGD_FATAL_ERROR(CAGD_ERR_NUM_KNOT_MISMATCH);

		RefSrf = CagdSrfCopy(Srf);
		for (i = 0; i < n; i++)
		    RefSrf -> VKnotVector[i] = *t++;
		break;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
		break;
	}
    }
    else if (n == 0) {
	RefSrf = CagdSrfCopy(Srf);
    }
    else {
	int j, NewULen, NewVLen, LengthKVt, RULength;
	BspKnotAlphaCoeffStruct *A;
	CagdRType *MergedKVt, UMin, UMax, VMin, VMax,
	    *UKnotVector = Srf -> UKnotVector,
	    *VKnotVector = Srf -> VKnotVector;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1])
		CAGD_FATAL_ERROR(CAGD_ERR_KNOT_NOT_ORDERED);

	switch (Dir) {
	    case CAGD_CONST_U_DIR:
	        for (i = 0; i < n; i++) {
		    if (t[i] >= UMax)
			t[i] = UMax - CAGD_DOMAIN_IRIT_EPS;
		}

		/* Compute the Alpha refinement matrix. */
		MergedKVt = BspKnotMergeTwo(UKnotVector,
					    ULength + UOrder,
					    t, n, 0, &LengthKVt);
		A = BspKnotEvalAlphaCoef(UOrder, UKnotVector, ULength,
					 MergedKVt, LengthKVt - UOrder,
					 Srf -> UPeriodic);

		NewULen = ULen + n;
	        RefSrf = BspPeriodicSrfNew(NewULen, VLen,
					   UOrder, VOrder,
					   Srf -> UPeriodic, Srf -> VPeriodic,
					   Srf -> PType);
		IritFree(RefSrf -> UKnotVector);
		RefSrf -> UKnotVector = MergedKVt;
		if (RefSrf -> UPeriodic) {
		    /* Make sure the knot vector is indeed periodic. */
		    BspKnotVerifyPeriodicKV(RefSrf -> UKnotVector,
					    UOrder,
					    CAGD_SRF_UPT_LST_LEN(RefSrf));
		}
		CAGD_GEN_COPY(RefSrf -> VKnotVector,
			      Srf -> VKnotVector,
			      sizeof(CagdRType) * (VLength + VOrder));

		RULength = RefSrf -> ULength;

		/* Update the control mesh */
		for (Row = 0; Row < VLen; Row++) {
		    for (j = IsNotRational; j <= MaxCoord; j++) {
			CagdRType
			    *ROnePts = &RefSrf -> Points[j][Row * RULength],
			    *OnePts = &Srf -> Points[j][Row * ULen];

			if (CAGD_IS_UPERIODIC_SRF(Srf)) {
			    BspKnotAlphaLoopBlendPeriodic(A, 0, NewULen,
							  OnePts, ULen,
							  ROnePts);
			}
			else {
			    BspKnotAlphaLoopBlendNotPeriodic(A, 0, NewULen,
							     OnePts, ROnePts);
			}
		    }
		}

		BspKnotFreeAlphaCoef(A);
		break;
	    case CAGD_CONST_V_DIR:
		for (i = 0; i < n; i++) {
		    if (t[i] >= VMax)
			t[i] = VMax - CAGD_DOMAIN_IRIT_EPS;
		}

		/* Compute the Alpha refinement matrix. */
		MergedKVt = BspKnotMergeTwo(VKnotVector, VLen + VOrder,
					    t, n, 0, &LengthKVt);
		A = BspKnotEvalAlphaCoef(VOrder, VKnotVector, VLen,
					 MergedKVt, LengthKVt - VOrder,
					 Srf -> VPeriodic);

		NewVLen = VLen + n;
	        RefSrf = BspPeriodicSrfNew(ULen, NewVLen,
					   UOrder, VOrder,
					   Srf -> UPeriodic, Srf -> VPeriodic,
					   Srf -> PType);
		CAGD_GEN_COPY(RefSrf -> UKnotVector, 
			      Srf -> UKnotVector,
			      sizeof(CagdRType) * (ULength + UOrder));
		IritFree(RefSrf -> VKnotVector);
		RefSrf -> VKnotVector = MergedKVt;
		if (RefSrf -> VPeriodic) {
		    /* Make sure the knot vector is indeed periodic. */
		    BspKnotVerifyPeriodicKV(RefSrf -> VKnotVector,
					    VOrder,
					    CAGD_SRF_VPT_LST_LEN(RefSrf));
		}

		RULength = RefSrf -> ULength;

		/* Update the control mesh */
		for (Col = 0; Col < ULen; Col++) {
		    for (j = IsNotRational; j <= MaxCoord; j++) {
			CagdRType
			    *ROnePts = &RefSrf -> Points[j][Col],
			    *OnePts = &Srf -> Points[j][Col];

			BspKnotAlphaLoopBlendStep(A, 0, NewVLen,
						  OnePts, ULen, -1,
						  ROnePts, RULength);
		    }
		}

		BspKnotFreeAlphaCoef(A);
		break;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
		break;
	}
    }

    BspKnotMakeRobustKV(RefSrf -> UKnotVector,
			RefSrf -> UOrder + RefSrf -> ULength);
    BspKnotMakeRobustKV(RefSrf -> VKnotVector,
			RefSrf -> VOrder + RefSrf -> VLength);

    CAGD_PROPAGATE_ATTR(RefSrf, Srf);

    return RefSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline surface, identical to the original but with one      M
* degree higher, in the requested direction Dir.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise it degree by one.                                   M
*   Dir:        Direction of degree raising. Either U or V.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with one degree higher in direction Dir,     M
*                     representing the same geometry as Srf.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise                M
*   BspSrfDegreeRaiseN							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdSrfStruct *BspSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, i2, j, RaisedLen, Row, Col, Length,
	Order = Dir == CAGD_CONST_V_DIR ? Srf -> UOrder : Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *CpSrf,
	*RaisedSrf = NULL;

    if (CAGD_IS_PERIODIC_SRF(Srf))
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    Length = Dir == CAGD_CONST_V_DIR ? Srf -> ULength : Srf -> VLength;

    if (Order != 2) {
	CagdSrfStruct *UnitSrf;
	int UKvLen1 = Srf -> UOrder + Srf -> ULength - 1,
	    VKvLen1 = Srf -> VOrder + Srf -> VLength - 1;
	CagdRType
	    *UKv = Srf -> UKnotVector,
	    *VKv = Srf -> VKnotVector;

	/* Degree raise by multiplying by a constant 1 linear surface in the */
	/* raised direction and constant 1 constant surface in the other.    */

	switch (Dir) {
	    case CAGD_CONST_U_DIR:
		UnitSrf = BspSrfNew(2, 1, 2, 1,
				    CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
		for (i = 0; i < 4; i++)
		    UnitSrf -> UKnotVector[i] = i > 1 ? UKv[UKvLen1] : UKv[0];
		for (i = 0; i < 2; i++)
		    UnitSrf -> VKnotVector[i] = i > 0 ? VKv[VKvLen1] : VKv[0];
		break;
	    case CAGD_CONST_V_DIR:
		UnitSrf = BspSrfNew(1, 2, 1, 2,
				    CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
		for (i = 0; i < 2; i++)
		    UnitSrf -> UKnotVector[i] = i > 0 ? UKv[UKvLen1] : UKv[0];
		for (i = 0; i < 4; i++)
		    UnitSrf -> VKnotVector[i] = i > 1 ? VKv[VKvLen1] : VKv[0];
		break;
	    default:
		UnitSrf = NULL;
		CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
		break;
	}
	for (i = 1; i <= MaxCoord; i++)
	    UnitSrf -> Points[i][0] = UnitSrf -> Points[i][1] = 1.0;

	RaisedSrf = BspSrfMult(Srf, UnitSrf);

	CagdSrfFree(UnitSrf);

	if (CpSrf)
	    CagdSrfFree(CpSrf);

	return RaisedSrf;
    }

    /* If surface is linear, degree raising means basically to increase the  */
    /* knot multiplicity of each segment by one and add a middle point for   */
    /* each such segment.						     */
    RaisedLen = Length * 2 - 1;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    RaisedSrf = BspSrfNew(RaisedLen, Srf -> VLength,
				  Order + 1, Srf -> VOrder, Srf -> PType);

	    /* Update the knot vectors. */
	    CAGD_GEN_COPY(RaisedSrf -> VKnotVector,
			  Srf -> VKnotVector,
			  sizeof(CagdRType) * (Srf -> VLength + Srf -> VOrder));
	    for (i = 0; i < 3; i++)
		RaisedSrf -> UKnotVector[i] = Srf -> UKnotVector[0];
	    for (i = 2, j = 3; i < Length; i++, j += 2)
		RaisedSrf -> UKnotVector[j] = RaisedSrf -> UKnotVector[j + 1] = 
		    Srf -> UKnotVector[i];
	    for (i = j; i < j + 3; i++)
		RaisedSrf -> UKnotVector[i] = Srf -> UKnotVector[Length];

	    /* Update the mesh. */
       	    for (Row = 0; Row < Srf -> VLength; Row++) {
		for (j = IsNotRational; j <= MaxCoord; j++)  /* First point. */
		    RaisedSrf -> Points[j][RAISED_SRF(0, Row)] =
			Srf -> Points[j][SRF(0, Row)];

		for (i = 1, i2 = 1; i < Length; i++, i2 += 2)
		    for (j = IsNotRational; j <= MaxCoord; j++) {
			RaisedSrf -> Points[j][RAISED_SRF(i2, Row)] =
				Srf -> Points[j][SRF(i - 1, Row)] * 0.5 +
				Srf -> Points[j][SRF(i, Row)] * 0.5;
			RaisedSrf -> Points[j][RAISED_SRF(i2 + 1, Row)] =
				Srf -> Points[j][SRF(i, Row)];
		    }
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    RaisedSrf = BspSrfNew(Srf -> ULength, RaisedLen,
				  Srf -> UOrder, Order + 1, Srf -> PType);

	    /* Update the knot vectors. */
	    CAGD_GEN_COPY(RaisedSrf -> UKnotVector,
			  Srf -> UKnotVector,
			  sizeof(CagdRType) * (Srf -> ULength + Srf -> UOrder));
	    for (i = 0; i < 3; i++)
		RaisedSrf -> VKnotVector[i] = Srf -> VKnotVector[0];
	    for (i = 2, j = 3; i < Length; i++, j += 2)
		RaisedSrf -> VKnotVector[j] = RaisedSrf -> VKnotVector[j + 1] = 
		    Srf -> VKnotVector[i];
	    for (i = j; i < j + 3; i++)
		RaisedSrf -> VKnotVector[i] = Srf -> VKnotVector[Length];

	    /* Update the mesh. */
       	    for (Col = 0; Col < Srf -> ULength; Col++) {
		for (j = IsNotRational; j <= MaxCoord; j++)  /* First point. */
		    RaisedSrf -> Points[j][RAISED_SRF(Col, 0)] =
			Srf -> Points[j][SRF(Col, 0)];

		for (i = 1, i2 = 1; i < Length; i++, i2 += 2)
		    for (j = IsNotRational; j <= MaxCoord; j++) {
			RaisedSrf -> Points[j][RAISED_SRF(Col, i2)] =
				Srf -> Points[j][SRF(Col, i - 1)] * 0.5 +
				Srf -> Points[j][SRF(Col, i)] * 0.5;
			RaisedSrf -> Points[j][RAISED_SRF(Col, i2 + 1)] =
				Srf -> Points[j][SRF(Col, i)];
		    }
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    CAGD_PROPAGATE_ATTR(RaisedSrf, Srf);

    if (CpSrf)
	CagdSrfFree(CpSrf);

    return RaisedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline surface, identical to the original but with higher   M
* degrees, as prescribed by NewUOrder, NewVOrder.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise its degree.                                         M
*   NewUOrder:  New U order of Srf.					     M
*   NewVOrder:  New V order of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with higher degrees as prescribed by	     M
*                     NewUOrder/NewVOrder.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise                M
*   BspSrfDegreeRaise, BzrSrfDegreeRaiseN, CagdSrfDegreeRaiseN               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdSrfStruct *BspSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder)
{
    int i, j, RaisedUOrder, RaisedVOrder, UKvLen1, VKvLen1, ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *RaisedSrf, *UnitSrf, *CpSrf;
    CagdRType *UKv, *VKv;

    if (CAGD_IS_PERIODIC_SRF(Srf))
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    ULength = Srf -> ULength;
    VLength = Srf -> VLength;
    UKvLen1 = UOrder + ULength - 1;
    UKv = Srf -> UKnotVector;
    VKvLen1 = VOrder + VLength - 1;
    VKv = Srf -> VKnotVector;

    if (NewUOrder < UOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedUOrder = NewUOrder - UOrder + 1;

    if (NewVOrder < VOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedVOrder = NewVOrder - VOrder + 1;

    UnitSrf = BspSrfNew(RaisedUOrder, RaisedVOrder, RaisedUOrder, RaisedVOrder,
			CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
    for (i = 0; i < RaisedUOrder * 2; i++)
	UnitSrf -> UKnotVector[i] = i >= RaisedUOrder ? UKv[UKvLen1] : UKv[0];
    for (i = 0; i < RaisedVOrder * 2; i++)
	UnitSrf -> VKnotVector[i] = i >= RaisedVOrder ? VKv[VKvLen1] : VKv[0];
    for (i = 1; i <= MaxCoord; i++)
	for (j = 0; j < RaisedUOrder * RaisedVOrder; j++)
	    UnitSrf -> Points[i][j] = 1.0;

    RaisedSrf = BspSrfMult(Srf, UnitSrf);

    CagdSrfFree(UnitSrf);

    CAGD_PROPAGATE_ATTR(RaisedSrf, Srf);

    if (CpSrf)
	CagdSrfFree(CpSrf);

    return RaisedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface equal to the given surface, differentiated once in   M
* the direction Dir.                                                         M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
* This is applied to all rows/cols of the surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To differentiate.                                            M
*   Dir:        Direction of differentiation. Either U or V.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Differentiated surface.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDerive, BzrSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational  M
*   BspSrfDeriveScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDerive, derivatives, partial derivatives                           M
*****************************************************************************/
CagdSrfStruct *BspSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, Row, Col, NewUOrder, NewVOrder, NewULength, NewVLength,
	ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType **DPoints;
    CagdRType const *UKv, *VKv;
    CagdRType * const *Points;
    CagdSrfStruct *CpSrf,
        *DerivedSrf = NULL;

    if (CAGD_IS_PERIODIC_SRF(Srf))
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    if (!GlblDeriveScalar && !IsNotRational)
	return BspSrfDeriveRational(Srf, Dir);

    ULength = Srf -> ULength;
    VLength = Srf -> VLength;
    UKv = Srf -> UKnotVector;
    VKv = Srf -> VKnotVector;
    Points = Srf -> Points;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    NewULength = UOrder < 2 ? ULength : ULength - 1;
	    NewUOrder = IRIT_MAX(UOrder - 1, 1);
	    DerivedSrf = BspSrfNew(NewULength, VLength,
				   NewUOrder, VOrder, Srf -> PType);
	    CAGD_GEN_COPY(DerivedSrf -> UKnotVector, &UKv[UOrder < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewULength + NewUOrder));
	    CAGD_GEN_COPY(DerivedSrf -> VKnotVector, VKv,
			  sizeof(CagdRType) * (VLength + VOrder));
	    DPoints = DerivedSrf -> Points;

       	    for (Row = 0; Row < VLength; Row++)
		for (i = 0; i < NewULength; i++) {
		    CagdRType
			Denom = UKv[i + UOrder] - UKv[i + 1];

		    if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
		        Denom = IRIT_UEPS;

		    for (j = IsNotRational; j <= MaxCoord; j++) {
			DPoints[j][DERIVED_SRF(i, Row)] =
			    UOrder < 2 ? 0.0
				       : (UOrder - 1) *
					    (Points[j][SRF(i + 1, Row)] -
					     Points[j][SRF(i, Row)]) / Denom;
		    }
		}
	    break;
	case CAGD_CONST_V_DIR:
	    NewVLength = VOrder < 2 ? VLength : VLength - 1;
	    NewVOrder = IRIT_MAX(VOrder - 1, 1);
	    DerivedSrf = BspSrfNew(ULength, NewVLength,
	    			   UOrder, NewVOrder, Srf -> PType);
	    CAGD_GEN_COPY(DerivedSrf -> UKnotVector, UKv,
			  sizeof(CagdRType) * (ULength + UOrder));
	    CAGD_GEN_COPY(DerivedSrf -> VKnotVector, &VKv[VOrder < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewVLength + NewVOrder));
	    DPoints = DerivedSrf -> Points;

	    for (Col = 0; Col < ULength; Col++)
		for (i = 0; i < NewVLength; i++) {
		    CagdRType
			Denom = VKv[i + VOrder] - VKv[i + 1];

		    if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
		        Denom = IRIT_UEPS;

		    for (j = IsNotRational; j <= MaxCoord; j++) {
			DPoints[j][DERIVED_SRF(Col, i)] =
			    VOrder < 2 ? 0.0
				       : (VOrder - 1) *
					    (Points[j][SRF(Col, i + 1)] -
					     Points[j][SRF(Col, i)]) / Denom;
		    }
		}
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return DerivedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface equal to the given surface, differentiated once in   M
* the direction Dir.							     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
* This is applied to all rows/cols of the surface.			     M
*   For a Euclidean surface this is the same as CagdCrvDerive but for a      M
* rational surface the returned surface is not the vector field but simply   M
* the derivatives of all the surface's coefficients, including the weights.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To differentiate.                                            M
*   Dir:        Direction of differentiation. Either U or V.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfDerive, CagdSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational  M
*   BspSrfDerive, BzrSrfDeriveScalar, CagdSrfDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDeriveScalar, derivatives                                          M
*****************************************************************************/
CagdSrfStruct *BspSrfDeriveScalar(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf;
    
    GlblDeriveScalar = TRUE;

    TSrf = BspSrfDerive(Srf, Dir);

    GlblDeriveScalar = FALSE;

    return TSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline surface, equal to the integral of the given Bspline  M
* srf.		                                                             M
* The given Bspline surface should be nonrational.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to integrate.                                        M
*   Dir:        Direction of integration. Either U or V.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Integrated surface.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfIntegrate, BspCrvIntegrate, CagdSrfIntegrate                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfIntegrate, integrals                                               M
*****************************************************************************/
CagdSrfStruct *BspSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    int i, j, k, Row, Col, ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *IntSrf, *CpSrf;
    CagdRType *IntUKv, *IntVKv;

    if (CAGD_IS_PERIODIC_SRF(Srf))
	Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
	CpSrf = NULL;

    if (CAGD_IS_RATIONAL_SRF(Srf))
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

    ULength = Srf -> ULength;
    VLength = Srf -> VLength;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    IntSrf = BspSrfNew(ULength + 1, VLength,
			       UOrder + 1, VOrder, Srf -> PType);
	    IntUKv = IntSrf -> UKnotVector;
	    IntVKv = IntSrf -> VKnotVector;

	    /* Update knot sequences. */
	    CAGD_GEN_COPY(&IntUKv[1], Srf -> UKnotVector,
			  sizeof(CagdRType) * (ULength + UOrder));
	    IntUKv[0] = IntUKv[1];
	    IntUKv[ULength + UOrder + 1] = IntUKv[ULength + UOrder];

	    CAGD_GEN_COPY(IntVKv, Srf -> VKnotVector,
			  sizeof(CagdRType) * (VLength + VOrder));

	    /* Update control mesh. */
	    for (k = 1; k <= MaxCoord; k++) {
		CagdRType *R,
		    *Points = Srf -> Points[k],
		    *IntPoints = IntSrf -> Points[k];

		for (Row = 0; Row < VLength; Row++) {
		    for (j = 0; j < ULength + 1; j++) {
			R = &IntPoints[INT_SRF(j, Row)];

			*R = 0.0;
			for (i = 0; i < j; i++)
			    *R += Points[SRF(i, Row)] *
				     (IntUKv[i + UOrder + 1] - IntUKv[i + 1]);
			*R /= UOrder;
		    }
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    IntSrf = BspSrfNew(ULength, VLength + 1,
			       UOrder, VOrder + 1, Srf -> PType);
	    IntUKv = IntSrf -> UKnotVector;
	    IntVKv = IntSrf -> VKnotVector;

	    /* Update knot sequences. */
	    CAGD_GEN_COPY(IntUKv, Srf -> UKnotVector,
			  sizeof(CagdRType) * (ULength + UOrder));

	    CAGD_GEN_COPY(&IntVKv[1], Srf -> VKnotVector,
			  sizeof(CagdRType) * (VLength + VOrder));
	    IntVKv[0] = IntVKv[1];
	    IntVKv[VLength + VOrder + 1] = IntVKv[VLength + VOrder];

	    /* Update control mesh. */
	    for (k = 1; k <= MaxCoord; k++) {
		CagdRType *R,
		    *Points = Srf -> Points[k],
		    *IntPoints = IntSrf -> Points[k];

		for (Col = 0; Col < ULength; Col++) {
		    for (j = 0; j < VLength + 1; j++) {
			R = &IntPoints[INT_SRF(Col, j)];

			*R = 0.0;
			for (i = 0; i < j; i++)
			    *R += Points[SRF(Col, i)] *
				     (IntVKv[i + VOrder + 1] - IntVKv[i + 1]);
			*R /= VOrder;
		    }
		}
	    }
	    break;
	default:
	    IntSrf = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return IntSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the (unit) tangent to a surface at a given parametric location   M
* (u, v) and given direction Dir.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Bspline surface to evaluate (unit) tangent vector for.       M
*   u, v:       Parametric location of required (unit) tangent.              M
*   Dir:        Direction of tangent vector. Either U or V.                  M
*   Normalize:  If TRUE, attempt is made to normalize the returned vector.   M
*               If FALSE, length is a function of given parametrization.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the (unit)        M
*                     tangent information.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfTangent, BzrSrfTangent					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfTangent, tangent                                                   M
*****************************************************************************/
CagdVecStruct *BspSrfTangent(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdSrfDirType Dir,
			     CagdBType Normalize)
{
    CagdVecStruct
	*Tangent = NULL;
    CagdCrvStruct *Crv;

    switch (Dir) {
	case CAGD_CONST_V_DIR:
	    Crv = BspSrfCrvFromSrf(Srf, v, Dir);
	    Tangent = BspCrvTangent(Crv, u, Normalize);
	    CagdCrvFree(Crv);
	    break;
	case CAGD_CONST_U_DIR:
	    Crv = BspSrfCrvFromSrf(Srf, u, Dir);
	    Tangent = BspCrvTangent(Crv, v, Normalize);
	    CagdCrvFree(Crv);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return Tangent;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluate the (unit) normal of a surface at a given parametric location.    M
*   If we fail to compute the normal at given location we retry by moving a  M
* tad.                                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Bspline surface to evaluate (unit) normal vector for.        M
*   u, v:       Parametric location of required (unit) normal.               M
*   Normalize:  If TRUE, attempt is made to normalize the returned vector.   M
*               If FALSE, length is a function of given parametrization.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the (unit) normal M
*                     information.                                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BzrSrfNormal, BspSrfMeshNormals, SymbSrfNormalSrf	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfNormal, normal                                                     M
*****************************************************************************/
CagdVecStruct *BspSrfNormal(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct Normal;
    CagdVecStruct *V, T1;
    CagdRType UMin, UMax, VMin, VMax, UEps, VEps;

    CAGD_DOMAIN_GET_AND_VERIFY_SRF(u, v, Srf, UMin, UMax, VMin, VMax);
    UEps = (UMax - UMin) * NORMAL_IRIT_EPS;
    VEps = (VMax - VMin) * NORMAL_IRIT_EPS;

    V = BspSrfTangent(Srf, u, v, CAGD_CONST_U_DIR, FALSE);
    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_SQR(IRIT_EPS))
	V = BspSrfTangent(Srf,
			  u > UMin + UEps ? u - UEps : u + UEps,
			  v > VMin + VEps ? v - VEps : v + VEps,
			  CAGD_CONST_U_DIR, FALSE);
    CAGD_COPY_VECTOR(T1, *V);
	   
    V = BspSrfTangent(Srf, u, v, CAGD_CONST_V_DIR, FALSE);

    /* The normal is the cross product of T1 and V: */
    IRIT_CROSS_PROD(Normal.Vec, T1.Vec, V -> Vec);

    if (CAGD_SQR_LEN_VECTOR(Normal) < IRIT_SQR(IRIT_EPS)) {
	V = BspSrfTangent(Srf,
			  u > UMin + UEps ? u - UEps : u + UEps,
			  v > VMin + VEps ? v - VEps : v + VEps,
			  CAGD_CONST_V_DIR, FALSE);
	IRIT_CROSS_PROD(Normal.Vec, T1.Vec, V -> Vec);
    }

    if (Normalize)
	CAGD_NORMALIZE_VECTOR(Normal);		   /* Normalize the vector. */

    return &Normal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the unit normals of a surface at a mesh defined by subdividing   M
* the parametric space into a grid of size UFineNess by VFineNess.	     M
*   The normals are saved in a linear CagdVecStruct vector which is          M
* allocated dynamically. Data is saved u inc. first.			     M
*   This routine is much faster than evaluating normal for each point,       M
* individually.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute normals on a grid of its parametric domain.     M
*   UFineNess:    U Fineness of imposed grid on Srf's parametric domain.     M
*   VFineNess:    V Fineness of imposed grid on Srf's parametric domain.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:   An vector of unit normals (u increments first).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BspSrfNormal, SymbSrfNormalSrf, BzrSrfMeshNormals,	     M
*   BspSrfMeshNormalsSymb						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMeshNormals, normal                                                M
*****************************************************************************/
CagdVecStruct *BspSrfMeshNormals(const CagdSrfStruct *Srf,
				 int UFineNess,
				 int VFineNess)
{
    CagdRType UMin, UMax, VMin, VMax, UEps, VEps;
    CagdVecStruct *Normals, *NPtr;
    int i, j,
	OldMultMethod = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

    UFineNess = IRIT_BOUND(UFineNess, 2, CAGD_MAX_FINENESS);
    VFineNess = IRIT_BOUND(VFineNess, 2, CAGD_MAX_FINENESS);

    Normals = CagdVecArrayNew(UFineNess * VFineNess);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    UEps = (UMax - UMin) * NORMAL_IRIT_EPS;
    VEps = (VMax - VMin) * NORMAL_IRIT_EPS;

    NPtr = Normals;

#ifdef BSP_MESH_NORMAL_SRF_THRU_CRVS
    /* Evaluate the mesh by extracting curves and evaluating them. */
    {
        CagdSrfStruct
	    *DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
	    *DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

	for (i = 0; i < UFineNess; i++) {
	    CagdRType
	        U = UMin + (UMax - UMin) * i / (UFineNess - 1);
	    CagdCrvStruct *DuCrv, *DvCrv,
	        *DuCrv2 = NULL,
	        *DvCrv2 = NULL;

	    if (U > UMax)                /* Due to floating point round off. */
	        U = UMax;

	    DuCrv = CagdCrvFromSrf(DuSrf, U, CAGD_CONST_U_DIR),
	    DvCrv = CagdCrvFromSrf(DvSrf, U, CAGD_CONST_U_DIR);

	    for (j = 0; j < VFineNess; j++) {
	        CagdVType Du, Dv;
		CagdRType
		    V = VMin + (VMax - VMin) * j / (VFineNess - 1);

		if (V > VMax)            /* Due to floating point round off. */
		    V = VMax;
		
		if (!EvalTangentVector(Du, DuCrv, V, VMin, VMax)) {
		    if (DuCrv2 == NULL) {
		        DuCrv2 = CagdCrvFromSrf(DuSrf,
						U > UMin + UEps ? U - UEps
					                        : U + UEps,
						CAGD_CONST_U_DIR);
		    }
		    EvalTangentVector(Du, DuCrv2, V, VMin, VMax);
		}

		if (!EvalTangentVector(Dv, DvCrv, V, VMin, VMax)) {
		    if (DvCrv2 == NULL) {
		        DvCrv2 = CagdCrvFromSrf(DvSrf,
						U > UMin + UEps ? U - UEps
								: U + UEps,
						CAGD_CONST_U_DIR);
		    }
		    EvalTangentVector(Dv, DvCrv2, V, VMin, VMax);
		}

		IRIT_CROSS_PROD(NPtr -> Vec, Dv, Du);
		NPtr++;
	    }

	    if (DuCrv2 != NULL)
	        CagdCrvFree(DuCrv2);
	    if (DvCrv2 != NULL)
	        CagdCrvFree(DvCrv2);
	    CagdCrvFree(DuCrv);
	    CagdCrvFree(DvCrv);
	}

	CagdSrfFree(DuSrf);
	CagdSrfFree(DvSrf);
    }
#else
    /* Evaluate the mesh by global refinements of the contol mesh. */
    {
        int ULength, VLength,
	    FineNessURef = UFineNess - Srf -> ULength,
	    FineNessVRef = VFineNess - Srf -> VLength;
	CagdBType
	    UClosed = CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR),
	    VClosed = CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR);
	CagdRType **Points, du, dv;
	CagdSrfStruct *TSrf, *TSrf2;

	if (FineNessURef > 0 || FineNessVRef > 0) {
	    CagdRType u, v, du, dv,
	        *RefKV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
					      IRIT_MAX(FineNessURef, FineNessVRef));

	    if (FineNessURef > 0) {
	        du = (UMax - UMin) / (FineNessURef + 1);
		for (i = 0, u = UMin + du; i < FineNessURef; i++, u += du)
		    RefKV[i] = u;

		TSrf = CagdSrfRefineAtParams(Srf, CAGD_CONST_U_DIR, FALSE,
					     RefKV, FineNessURef);
	    }
	    else
	        TSrf = CagdSrfCopy(Srf);
	
	    if (FineNessVRef > 0) {
	        dv = (VMax - VMin) / (FineNessVRef + 1);
		for (i = 0, v = VMin + dv; i < FineNessVRef; i++, v += dv)
		    RefKV[i] = v;

		TSrf2 = CagdSrfRefineAtParams(TSrf, CAGD_CONST_V_DIR, FALSE,
					      RefKV, FineNessVRef);
		CagdSrfFree(TSrf);
		TSrf = TSrf2;
	    }

	    IritFree(RefKV);

	    TSrf2 = CagdCoerceSrfTo(TSrf, CAGD_PT_E3_TYPE, FALSE);
	    CagdSrfFree(TSrf);
	    TSrf = TSrf2;
	}
	else
	    TSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);

	/* Copy the refined control mesh to the sampled mesh in E3 form. */
	Points = TSrf -> Points;
	ULength = TSrf -> ULength;
	VLength = TSrf -> VLength;
	du = (ULength - 1.0) / (UFineNess - 1.0);
	dv = (VLength - 1.0) / (VFineNess - 1.0);
	for (i = 0; i < UFineNess; i++) {
	    int ii = (int) (i * du + 0.5);

	    for (j = 0; j < VFineNess; j++, NPtr++) {
	        int l, Idx1, Idx2,
		    jj = (int) (j * dv + 0.5);
	        CagdVType DuVec, DvVec;

		Idx1 = jj * ULength + U_INDEX_MIN(ii);
		Idx2 = jj * ULength + U_INDEX_MAX(ii);
		for (l = 0; l < 3; l++)
		    DuVec[l] = Points[l + 1][Idx2] - Points[l + 1][Idx1];

		Idx1 = V_INDEX_MIN(jj) * ULength + ii;
		Idx2 = V_INDEX_MAX(jj) * ULength + ii;
		for (l = 0; l < 3; l++)
		    DvVec[l] = Points[l + 1][Idx2] - Points[l + 1][Idx1];

		IRIT_CROSS_PROD(NPtr -> Vec, DvVec, DuVec);
	    }
	}

	CagdSrfFree(TSrf);
    }
#endif /* BSP_MESH_NORMAL_SRF_THRU_CRVS */

    /* Normalize the results. */
    NPtr = Normals;
    for (i = 0; i < UFineNess; i++) {
	for (j = 0; j < VFineNess; j++) {
	    CagdRType
		Len = IRIT_PT_LENGTH(NPtr -> Vec);

	    if (Len < NORMAL_IRIT_EPS * 0.1) {
		CagdVecStruct *NPtrTmp;
		CagdRType
		    u = UMin + (UMax - UMin) * i / (UFineNess - 1),
		    v = VMin + (VMax - VMin) * j / (VFineNess - 1);

		/* Evaluate normal directly, nearby. */
		u += (u > (UMin + UMax) * 0.5) ? -UEps : UEps;
		v += (v > (VMin + VMax) * 0.5) ? -VEps : VEps;
		*NPtr = *BspSrfNormal(Srf, u, v, FALSE);
		Len = IRIT_PT_LENGTH(NPtr -> Vec);

		/* Try its neighbors. */
		if (Len < SRF_TANGENT_MIN_LEN && i > 0) {
		    NPtrTmp = NPtr - VFineNess;
		    IRIT_PT_COPY(NPtr -> Vec, NPtrTmp -> Vec);
		    Len = IRIT_PT_LENGTH(NPtr -> Vec);
		}
		if (Len < SRF_TANGENT_MIN_LEN && i < UFineNess - 1) {
		    NPtrTmp = NPtr + VFineNess;
		    IRIT_PT_COPY(NPtr -> Vec, NPtrTmp -> Vec);
		    Len = IRIT_PT_LENGTH(NPtr -> Vec);
		}
		if (Len < SRF_TANGENT_MIN_LEN && j > 0) {
		    NPtrTmp = NPtr - 1;
		    IRIT_PT_COPY(NPtr -> Vec, NPtrTmp -> Vec);
		    Len = IRIT_PT_LENGTH(NPtr -> Vec);
		}
		if (Len < SRF_TANGENT_MIN_LEN && j < VFineNess - 1) {
		    NPtrTmp = NPtr + 1;
		    IRIT_PT_COPY(NPtr -> Vec, NPtrTmp -> Vec);
		    Len = IRIT_PT_LENGTH(NPtr -> Vec);
		}

		if (Len > SRF_TANGENT_MIN_LEN) {
		    Len = 1.0 / Len;
		    IRIT_PT_SCALE(NPtr -> Vec, Len);
		}
		else {
		    /* Do something. */
		    NPtr -> Vec[0] = NPtr -> Vec[1] = 0.0;
		    NPtr -> Vec[2] = 1.0;
		}
	    }
	    else {
		Len = 1 / Len;
		IRIT_PT_SCALE(NPtr -> Vec, Len);
	    }

	    NPtr++;
	}
    }

    BspMultComputationMethod(OldMultMethod);

    return Normals;
}

#ifdef BSP_MESH_NORMAL_SRF_THRU_CRVS

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates a vector field Crv at param t. If zero length, tries to move a *
* tad.                                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Vec:     Where to place the result.                                      *
*   Crv:     Vector field to evaluate.                                       *
*   t:       Parameter value where to evaluate the vector field.             *
*   TMin:    Minimum of vector field domain.                                 *
*   TMax:    Maximum of vector field domain.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE, if successful, FALSE otherwise.                        *
*****************************************************************************/
static CagdBType EvalTangentVector(CagdVType Vec,
				   CagdCrvStruct *Crv,
				   CagdRType t,
				   CagdRType TMin,
				   CagdRType TMax)
{
    CagdRType Len, Eps,
	*R = CagdCrvEval(Crv, t);
    
    CagdCoerceToE3(Vec, &R, -1, Crv -> PType);
    Len = IRIT_PT_LENGTH(Vec);

    Eps = (TMax - TMin) * NORMAL_IRIT_EPS;

    if (Len < SRF_TANGENT_MIN_LEN && t - Eps > TMin) {
	CagdCrvEval(Crv, t - Eps);
	CagdCoerceToE3(Vec, &R, -1, Crv -> PType);
	Len = IRIT_PT_LENGTH(Vec);
    }
    if (Len < SRF_TANGENT_MIN_LEN && t + Eps < TMax) {
	CagdCrvEval(Crv, t + Eps);
	CagdCoerceToE3(Vec, &R, -1, Crv -> PType);
	Len = IRIT_PT_LENGTH(Vec);
    }

    if (Len > SRF_TANGENT_MIN_LEN) {
	Len = 1.0 / Len;
	IRIT_PT_SCALE(Vec, Len);
	return TRUE;
    }
    else
        return FALSE;
}

#endif /*  BSP_MESH_NORMAL_SRF_THRU_CRVS */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the unit normals of a surface at a mesh defined by subdividing   M
* the parametric space into a grid of size UFineNess by VFineNess.	     M
*   The normals are saved in a linear CagdVecStruct vector which is          M
* allocated dynamically. Data is saved u inc. first.			     M
*   This routine is much faster than evaluating normal for each point,       M
* individually.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute normals on a grid of its parametric domain.     M
*   UFineNess:    U Fineness of imposed grid on Srf's parametric domain.     M
*   VFineNess:    V Fineness of imposed grid on Srf's parametric domain.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:   An vector of unit normals (u increments first).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BspSrfNormal, SymbSrfNormalSrf, BspSrfMeshNormals	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMeshNormalsSymb, normal                                            M
*****************************************************************************/
CagdVecStruct *BspSrfMeshNormalsSymb(CagdSrfStruct *Srf,
				     int UFineNess,
				     int VFineNess)
{
    int i, j;
    CagdRType U, V, UMin, UMax, VMin, VMax, **Points;
    CagdVecStruct *Normals, *NPtr;
    CagdSrfStruct *TstNormalSrf,
	*NormalSrf = SymbSrfNormalSrf(Srf);

    /* Verify we have a valid regular surface. */
    TstNormalSrf = CagdCoerceSrfTo(NormalSrf, CAGD_PT_E3_TYPE, FALSE);
    for (Points = TstNormalSrf -> Points, i = 0;
	 i < TstNormalSrf -> ULength * TstNormalSrf -> VLength;
	 i++)
	if (!IRIT_APX_EQ(Points[1][i], 0.0 ) ||
	    !IRIT_APX_EQ(Points[2][i], 0.0 ) ||
	    !IRIT_APX_EQ(Points[3][i], 0.0 ))
	    break;
    CagdSrfFree(TstNormalSrf);
    if (i >= TstNormalSrf -> ULength * TstNormalSrf -> VLength) {
	/* Not a regular surface - ignore it, */
	return NULL;
    }

    UFineNess = IRIT_BOUND(UFineNess, 2, CAGD_MAX_FINENESS);
    VFineNess = IRIT_BOUND(VFineNess, 2, CAGD_MAX_FINENESS);

    Normals = CagdVecArrayNew(UFineNess * VFineNess);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    NPtr = Normals;
    for (i = 0; i < UFineNess; i++) {
	for (j = 0; j < VFineNess; j++) {
	    U = UMin + (UMax - UMin) * i / (UFineNess - 1);
	    V = VMin + (VMax - VMin) * j / (VFineNess - 1);
	    CagdEvaluateSurfaceVecField(NPtr -> Vec, NormalSrf, U, V);

	    NPtr++;
	}
    }

    CagdSrfFree(NormalSrf);

    return Normals;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply the Moebius transformation to a ration Bspline surface.	     M
* See "Moebius reparametrization of rational Bsplines", by Lee & Lucian,     M
* CAGD 8 (1991) pp 213-215.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:        Surface to apply the Moebius transformation to.              M
*   c:          The scaling coefficient - c^n is the ratio between the first M
*	        and last weight of the surface, along each row or column.    M
*		If c == 0, the first and last weights are made equal, in the M
*		first row/column.				             M
*   Dir:        Direction to apply the Moebius transformation, row or col.   M
*		If Dir == CAGD_BOTH_DIR, the transformation is applied to    M
*		both the row and column directions, in this order.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The modified surface with the same shape but different M
*		speeds.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvMoebiusTransform, BzrSrfMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMoebiusTransform                                                   M
*****************************************************************************/
CagdSrfStruct *BspSrfMoebiusTransform(const CagdSrfStruct *CSrf,
				      CagdRType c,
				      CagdSrfDirType Dir)
{
    int i, j, l,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CSrf -> PType),
	UOrder = -1,
	VOrder = -1,
	ULength = -1,
	VLength = -1;
    CagdRType c0, **Points, *KV, UMin, UMax, VMin, VMax,
	MaxW = IRIT_UEPS;    
    CagdSrfStruct *T1Srf, *T2Srf, *Srf;

    if (CSrf -> GType != CAGD_SBSPLINE_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SRF);
	return NULL;
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	case CAGD_CONST_V_DIR:
	    UOrder = CSrf -> UOrder;
	    VOrder = CSrf -> VOrder;
	    ULength = CSrf -> ULength;
	    VLength = CSrf -> VLength;
	    break;
	case CAGD_BOTH_DIR:
	    T1Srf = BspSrfMoebiusTransform(CSrf, c, CAGD_CONST_U_DIR);
	    T2Srf = BspSrfMoebiusTransform(T1Srf, c, CAGD_CONST_V_DIR);
	    CagdSrfFree(T1Srf);
	    return T2Srf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (!CAGD_IS_RATIONAL_SRF(CSrf)) {
        if (c == 1.0)
	    return CagdSrfCopy(CSrf);
	else
	    Srf = CagdCoerceSrfTo(CSrf, CAGD_MAKE_PT_TYPE(TRUE, MaxCoord), FALSE);
    }
    else
        Srf = CagdSrfCopy(CSrf);
    Points = Srf -> Points;

    /* Save original curve domain and temporary map to zero to one. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
            KV = Srf -> UKnotVector;
	    BspKnotAffineTransOrder2(KV, UOrder,
				     CAGD_SRF_UPT_LST_LEN(Srf) + UOrder,
				     0.0, 1.0);

	    if (Points[0][0] == 0 ||
		Points[0][CAGD_MESH_UV(Srf, ULength - 1, 0)] == 0) {
	        CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
		return NULL;
	    }
	    if (c == 0.0) {
	        c = Points[0][0] / Points[0][CAGD_MESH_UV(Srf, ULength - 1, 0)];
		c = pow(IRIT_FABS(c), 1.0 / (UOrder - 1.0));
	    }

	    /* Update the control points of the surface. */
	    for (i = 1, c0 = 1.0; i < UOrder; i++)
	        c0 *= MOEBIUS_MEU(KV[i]);
	    for (i = 0; i < ULength; i++) {
	        for (j = 0; j < VLength; j++)
		   for (l = 0; l <= MaxCoord; l++)
		       Points[l][CAGD_MESH_UV(Srf, i, j)] /= c0;
		c0 *= MOEBIUS_MEU(KV[UOrder + i]) / MOEBIUS_MEU(KV[i + 1]);
	    }

	    /* Update the knot sequence of the curve. */
	    for (i = 0; i < CAGD_SRF_UPT_LST_LEN(Srf) + UOrder; i++)
	        KV[i] = MOEBIUS_REPARAM(KV[i]);

	    BspKnotAffineTransOrder2(KV, UOrder,
				     CAGD_SRF_UPT_LST_LEN(Srf) + UOrder,
				     UMin, UMax);
	    break;
	case CAGD_CONST_V_DIR:
            KV = Srf -> VKnotVector;
	    BspKnotAffineTransOrder2(KV, VOrder,
				     CAGD_SRF_VPT_LST_LEN(Srf) + VOrder,
				     0.0, 1.0);

	    if (Points[0][0] == 0 ||
		Points[0][CAGD_MESH_UV(Srf, 0, VLength - 1)] == 0) {
	        CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
		return NULL;
	    }
	    if (c == 0.0) {
	        c = Points[0][0] / Points[0][CAGD_MESH_UV(Srf, 0, VLength - 1)];
		c = pow(IRIT_FABS(c), 1.0 / (VOrder - 1.0));
	    }

	    /* Update the control points of the surface. */
	    for (j = 1, c0 = 1.0; j < VOrder; j++)
	        c0 *= MOEBIUS_MEU(KV[j]);
	    for (j = 0; j < VLength; j++) {
	        for (i = 0; i < ULength; i++)
		   for (l = 0; l <= MaxCoord; l++)
		       Points[l][CAGD_MESH_UV(Srf, i, j)] /= c0;
		c0 *= MOEBIUS_MEU(KV[VOrder + j]) / MOEBIUS_MEU(KV[j + 1]);
	    }

	    /* Update the knot sequence of the curve. */
	    for (i = 0; i < CAGD_SRF_VPT_LST_LEN(Srf) + VOrder; i++)
	        KV[i] = MOEBIUS_REPARAM(KV[i]);

	    BspKnotAffineTransOrder2(KV, VOrder,
				     CAGD_SRF_VPT_LST_LEN(Srf) + VOrder,
				     VMin, VMax);
	    break;
	default:
	    assert(0);
    }

    /* Normalize all weights so largest has magnitude of one. */
    for (i = 0; i < ULength * VLength; i++) {
	if (MaxW < IRIT_FABS(Points[0][i]))
	    MaxW = IRIT_FABS(Points[0][i]);
    }
    for (i = 0; i < ULength * VLength; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] /= MaxW;
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline surface into a Bspline surface with floating end        M
* conditions.                                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Bspline surface to convert to floating end conditions. Assume M
*              Srf is either periodic or has floating end condition.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface with floating end conditions,        M
*                     representing the same geometry as Srf.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtFloat2OpenSrf						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPeriodic2FloatSrf, conversion                                   M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtPeriodic2FloatSrf(const CagdSrfStruct *Srf)
{
    int i, j,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *NewSrf;

    if (!CAGD_IS_BSPLINE_SRF(Srf)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_SRF_EXPECT);
	return NULL;
    }

    if (!CAGD_IS_UPERIODIC_SRF(Srf) && !CAGD_IS_VPERIODIC_SRF(Srf)) {
	CAGD_FATAL_ERROR(CAGD_ERR_PERIODIC_EXPECTED);
	return NULL;
    }

    NewSrf = BspSrfNew(CAGD_SRF_UPT_LST_LEN(Srf), CAGD_SRF_VPT_LST_LEN(Srf),
		       UOrder, VOrder, Srf -> PType);

    CAGD_GEN_COPY(NewSrf -> UKnotVector, Srf -> UKnotVector,
		  sizeof(CagdRType) * (CAGD_SRF_UPT_LST_LEN(Srf) + UOrder));
    CAGD_GEN_COPY(NewSrf -> VKnotVector, Srf -> VKnotVector,
		  sizeof(CagdRType) * (CAGD_SRF_VPT_LST_LEN(Srf) + VOrder));

    for (i = !CAGD_IS_RATIONAL_PT(Srf -> PType); i <= MaxAxis; i++) {
	CagdRType
	    *Pts = Srf -> Points[i],
	    *NewPts = NewSrf -> Points[i];

	for (j = 0; j < VLength; j++, Pts += ULength) {
	    CAGD_GEN_COPY(NewPts, Pts, sizeof(CagdRType) * ULength);
	    NewPts += ULength;
	    if (Srf -> UPeriodic) {
		CAGD_GEN_COPY(NewPts, Pts,
			      sizeof(CagdRType) * (UOrder - 1));
		NewPts += UOrder - 1;
	    }
	}
	if (Srf -> VPeriodic) {
	    CAGD_GEN_COPY(NewPts, NewSrf -> Points[i],
			  sizeof(CagdRType) * (VOrder - 1) *
			  CAGD_SRF_UPT_LST_LEN(Srf));
	}
    }

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewSrf -> Points[i] = NULL;

    CAGD_PROPAGATE_ATTR(NewSrf, Srf);

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a float Bspline surface to a Bspline surface with open end	     M
* conditions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Bspline surface to convert to open end conditions.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface with open end conditions,	     M
*                     representing the same geometry as Srf.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtPeriodic2FloatSrf                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtFloat2OpenSrf, conversion                                       M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtFloat2OpenSrf(const CagdSrfStruct *Srf)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *TSrf1, *TSrf2;

    if (!CAGD_IS_BSPLINE_SRF(Srf)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_SRF_EXPECT);
	return NULL;
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    TSrf1 = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR);
    TSrf2 = CagdSrfRegionFromSrf(TSrf1, VMin, VMax, CAGD_CONST_V_DIR);
    CagdSrfFree(TSrf1);

    CAGD_PROPAGATE_ATTR(TSrf2, Srf);

    return TSrf2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline surface to a Bspline surface with open end conditions.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Bspline surface to convert to open end conditions.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface with open end conditions,	     M
*                     representing the same geometry as Srf.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBsp2OpenSrf, conversion                                         M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtBsp2OpenSrf(const CagdSrfStruct *Srf)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *TSrf1, *TSrf2;

    if (!CAGD_IS_BSPLINE_SRF(Srf)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_SRF_EXPECT);
	return NULL;
    }

    if (CAGD_IS_PERIODIC_SRF(Srf)) {
	CagdSrfStruct
	    *TSrf = CagdCnvrtPeriodic2FloatSrf(Srf);

	CagdSrfDomain(TSrf, &UMin, &UMax, &VMin, &VMax);

	TSrf1 = CagdSrfRegionFromSrf(TSrf, UMin, UMax, CAGD_CONST_U_DIR);
	TSrf2 = CagdSrfRegionFromSrf(TSrf1, VMin, VMax, CAGD_CONST_V_DIR);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf);
    }
    else if (!BspSrfHasOpenEC(Srf)) {
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	TSrf1 = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR);
	TSrf2 = CagdSrfRegionFromSrf(TSrf1, VMin, VMax, CAGD_CONST_V_DIR);
	CagdSrfFree(TSrf1);
    }
    else {
        TSrf2 = CagdSrfCopy(Srf);
    }

    CAGD_PROPAGATE_ATTR(TSrf2, Srf);

    return TSrf2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reparameterize a surface to follow a desired parametrization.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        The surface to update its paraametrization.                  M
*   Dir:        Parametric direction to reparameterize.                      M
*   ParamType:  The desired parametrization type: uniform, chord len., etc.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpBuildKVs, BspReparameterizeCrv                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspReparameterizeSrf                                                     M
*****************************************************************************/
void BspReparameterizeSrf(CagdSrfStruct *Srf,
			  CagdSrfDirType Dir,
			  CagdParametrizationType ParamType)
{
    CagdPointType
	PtType = Srf -> PType;
    int i, j, l,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtType);
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PtType);
    CagdRType *NewPtKnots, *NewKV;
    CagdRType * const * const Points = Srf -> Points;
    CagdCtlPtStruct *CtlPts, *Pt;
    CagdCrvStruct *Crv;

    if (!CAGD_IS_BSPLINE_SRF(Srf)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_SRF_EXPECT);
	return;
    }

    Crv = CagdCrvFromMesh(Srf, 0, CAGD_OTHER_DIR(Dir));
    CtlPts = CagdCnvrtCrvToCtlPts(Crv);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
            /* Compute average distances in the mesh along the U direction. */
	    for (i = 0, Pt = CtlPts; i < ULength; i++, Pt = Pt -> Pnext) {
	        for (j = 1; j < VLength; j++) {
		    int Idx = CAGD_MESH_UV(Srf, i, j);

		    for (l = !IsRational; l <= NumCoords; l++) {
		        Pt -> Coords[l] += Points[l][Idx];
		    }
		}
	    }

	    for (i = 0, Pt = CtlPts; i < ULength; i++, Pt = Pt -> Pnext) {
	        for (l = !IsRational; l <= NumCoords; l++) {
		    Pt -> Coords[l] /= VLength;
		}
	    }

	    /* Construct the new KV. */
	    BspCrvInterpBuildKVs(CtlPts, Srf -> UOrder, ULength, ParamType,
				 Srf -> UPeriodic, &NewPtKnots, &NewKV);
	    IritFree(NewPtKnots);
	    CagdCtlPtFreeList(CtlPts);

	    IritFree(Srf -> UKnotVector);
	    Srf -> UKnotVector = NewKV;
	    break;
	case CAGD_CONST_V_DIR:
            /* Compute average distances in the mesh along the V direction. */
	    for (j = 0, Pt = CtlPts; j < VLength; j++, Pt = Pt -> Pnext) {
	        for (i = 1; i < ULength; i++) {
		    int Idx = CAGD_MESH_UV(Srf, i, j);

		    for (l = !IsRational; l <= NumCoords; l++) {
		        Pt -> Coords[l] += Points[l][Idx];
		    }
		}
	    }

	    for (j = 0, Pt = CtlPts; j < VLength; j++, Pt = Pt -> Pnext) {
	        for (l = !IsRational; l <= NumCoords; l++) {
		    Pt -> Coords[l] /= VLength;
		}
	    }

	    /* Construct the new KV. */
	    BspCrvInterpBuildKVs(CtlPts, Srf -> VOrder, VLength, ParamType,
				 Srf -> VPeriodic, &NewPtKnots, &NewKV);
	    IritFree(NewPtKnots);
	    CagdCtlPtFreeList(CtlPts);

	    IritFree(Srf -> VKnotVector);
	    Srf -> VKnotVector = NewKV;
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
}
