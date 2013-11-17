/******************************************************************************
* CBsp-Aux.c - Bspline curve auxilary routines.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define MOEBIUS_MEU(t)		(1 + (t) * (1 - c) / c)
#define MOEBIUS_REPARAM(t)	((t) / ((t) + c * (1 - (t))))
#define CAGD_SAME_PT_EPS	1e-6

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

static CagdVecStruct *BspCrvTangentAux(const CagdCrvStruct *Crv,
				       CagdRType t,
				       CagdBType Normalize);
static CagdVecStruct *BspCrvBiNormalAux(const CagdCrvStruct *Crv,
					CagdRType t,
					CagdBType Normalize);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply B-spline subdivision to the given curve Crv at parameter value t,  M
* and save the result in curves' LPoints/RPoints.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:               To subdivide at parametr value t.                     M
*   LPoints, RPoints:  Where the results are kept.			     M
*   LLength, RLength:  Lengths of respective vectors.			     M
*   t:                 Parameter value to subdivide Crv at.                  M
*   Mult:	       Current multiplicity of t in the knot sequence.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvSubdivCtlPoly, BspCrvSubdivAtParam                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvSubdivCtlPoly                                                      M
*****************************************************************************/
void BspCrvSubdivCtlPoly(const CagdCrvStruct *Crv,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int LLength,
			 int RLength,
			 CagdRType t,
			 int Mult)
{
    CagdBType IsNotRational;
    int i, j, Len, MaxCoord,
	k = Crv -> Order;
    CagdRType TMin, TMax;
    CagdRType * const *Points;
    BspKnotAlphaCoeffStruct *A;

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* If it is a Bezier knot sequence, use the simpler Bezier subdivision. */
    CAGD_DOMAIN_GET_AND_VERIFY_CRV(t, Crv, TMin, TMax);
    if (BspCrvHasBezierKV(Crv)) {
        BzrCrvSubdivCtlPoly(Crv -> Points, LPoints, RPoints, k, Crv -> PType,
			    (t - TMin) / (TMax - TMin));
        return;
    }

    Len = Crv -> Length;
    IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    Points = Crv -> Points;

    if (Mult > 0) {
	CagdRType
	    *NewKV = IritMalloc(sizeof(CagdRType) * Mult);

	for (i = 0; i < Mult; i++)
	    NewKV[i] = (t == TMax ? t - CAGD_DOMAIN_IRIT_EPS : t);
	A = BspKnotEvalAlphaCoefMerge(k, Crv -> KnotVector, Len, NewKV,
				      Mult, FALSE);
	IritFree(NewKV);
    }
    else {
	A = BspKnotEvalAlphaCoef(k, Crv -> KnotVector, Len,
				    Crv -> KnotVector, Len, FALSE);
    }

    /* Note that Mult can be negative in cases where original       */
    /* multiplicity was order or more and we need to compensate     */
    /* here, since Alpha matrix will be just a unit matrix then.    */
    Mult = Mult >= 0 ? 0 : -Mult;

    /* Blend Crv into LCrv. */
    for (j = IsNotRational; j <= MaxCoord; j++) {
	BspKnotAlphaLoopBlendNotPeriodic(A, 0, LLength,
					 Points[j], LPoints[j]);
    }

    /* Blend Crv into RCrv. */
    for (j = IsNotRational; j <= MaxCoord; j++) {
	BspKnotAlphaLoopBlendNotPeriodic(A, LLength - 1 + Mult,
					 LLength + RLength - 1 + Mult,
					 Points[j], RPoints[j]);
    }

    BspKnotFreeAlphaCoef(A);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline curve - subdivides it into two sub-curves at the given     M
* parametric value.                                                          M
*   Returns pointer to first curve in a list of two subdivided curves.       M
*   The subdivision is achieved by inserting (order-1) knot at the given     M
* parameter value t and spliting the control polygon and knot vector at that M
* location.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To subdivide at parametr value t.                             M
*   t:         Parameter value to subdivide Crv at.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of the two subdivided curves.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvSubdivAtParam, subdivision, refinement                             M
*****************************************************************************/
CagdCrvStruct *BspCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    int i, j, Len, KVLen, Index1, Index2, Mult,
	k = Crv -> Order;
    CagdRType TMin, TMax;
    CagdCrvStruct *LCrv, *RCrv, *CpCrv;

    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    Len = Crv -> Length;
    KVLen = k + Len;

    i = BspKnotLastIndexLE(Crv -> KnotVector, KVLen, t);
    if (IRIT_APX_EQ_EPS(t, Crv -> KnotVector[i], CAGD_EPS_ROUND_KNOT))
        t = Crv -> KnotVector[i];
    else if (i < KVLen && IRIT_APX_EQ_EPS(t, Crv -> KnotVector[i + 1],
				     CAGD_EPS_ROUND_KNOT))
        t = Crv -> KnotVector[i + 1];

    Index1 = BspKnotLastIndexL(Crv -> KnotVector, KVLen, t);
    if (Index1 + 1 < k)
	Index1 = k - 1;
    Index2 = BspKnotFirstIndexG(Crv -> KnotVector, KVLen, t);
    if (Index2 > Len)
	Index2 = Len;
    Mult = k - 1 - (Index2 - Index1 - 1);

    CAGD_DOMAIN_GET_AND_VERIFY_CRV(t, Crv, TMin, TMax);

    LCrv = BspCrvNew(Index1 + 1, k, Crv -> PType);
    RCrv = BspCrvNew(Len - Index2 + k, k, Crv -> PType);

    /* Update the new knot vectors. */
    CAGD_GEN_COPY(LCrv -> KnotVector, Crv -> KnotVector,
		  sizeof(CagdRType) * (Index1 + 1));
    /* Close the knot vector with multiplicity Order: */
    for (j = Index1 + 1; j <= Index1 + k; j++)
	LCrv -> KnotVector[j] = t;
    CAGD_GEN_COPY(&RCrv -> KnotVector[k], &Crv -> KnotVector[Index2],
		  sizeof(CagdRType) * (Len + k - Index2));
    /* Make sure knot vector starts with multiplicity Order: */
    for (j = 0; j < k; j++)
	RCrv -> KnotVector[j] = t;

    BspKnotMakeRobustKV(RCrv -> KnotVector,
			RCrv -> Order + RCrv -> Length);
    BspKnotMakeRobustKV(LCrv -> KnotVector,
			LCrv -> Order + LCrv -> Length);

    /* Now handle the control polygon refinement. */
    BspCrvSubdivCtlPoly(Crv, LCrv -> Points, RCrv -> Points,
			LCrv -> Length, RCrv -> Length, t, Mult);

    LCrv -> Pnext = RCrv;

    CAGD_PROPAGATE_ATTR(LCrv, Crv);
    CAGD_PROPAGATE_ATTR(RCrv, Crv);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return LCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Inserts n knot, all with the value t. In no case will the multiplicity of M
* a knot be greater or equal to the curve order.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To refine by insertion (upto) n knot of value t.             M
*   t:          Parameter value of new knot to insert.                       M
*   n:          Maximum number of times t should be inserted.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Refined Crv with n knots of value t.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotInsertNSame, refinement, subdivision                           M
*****************************************************************************/
CagdCrvStruct *BspCrvKnotInsertNSame(const CagdCrvStruct *Crv,
				     CagdRType t,
				     int n)
{
    int i,
	CrntMult = BspKnotFindMult(Crv -> KnotVector, Crv -> Order,
							   Crv -> Length, t),
	Mult = IRIT_MIN(n, Crv -> Order - CrntMult - 1);
    CagdCrvStruct *RefinedCrv;

    if (Mult > 0) {
	CagdRType
	    *NewKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * Mult);

	for (i = 0; i < Mult; i++)
	    NewKV[i] = t;

	RefinedCrv = BspCrvKnotInsertNDiff(Crv, FALSE, NewKV, Mult);

	IritFree(NewKV);
    }
    else {
	RefinedCrv = CagdCrvCopy(Crv);
    }

    return RefinedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Inserts n knot with different values as defined by the vector t. If,      M
* however, Replace is TRUE, the knot are simply replacing the current knot   M
* vector.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To refine by insertion (upto) n knot of value t.             M
*   Replace:    if TRUE, the n knots in t should replace the knot vector     M
*               of size n of Crv. Sizes must match. If False, n new knots    M
*               as defined by t will be introduced into Crv.                 M
*   t:          New knots to introduce/replace knot vector of Crv.           M
*   n:          Size of t.                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Refined Crv with n new knots.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotInsertNDiff, refinement, subdivision                           M
*****************************************************************************/
CagdCrvStruct *BspCrvKnotInsertNDiff(const CagdCrvStruct *Crv,
				     CagdBType Replace,
				     CagdRType *t,
				     int n)
{
    CagdBType
	IsPeriodic = CAGD_IS_PERIODIC_CRV(Crv),
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdRType
	*KnotVector = Crv -> KnotVector;
    int Len = Crv -> Length,
	Length = CAGD_CRV_PT_LST_LEN(Crv),
	Order = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RefinedCrv;

    if (Replace) {
	int i;

	if (Order + Length != n)
	    CAGD_FATAL_ERROR(CAGD_ERR_NUM_KNOT_MISMATCH);
	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1])
		CAGD_FATAL_ERROR(CAGD_ERR_KNOT_NOT_ORDERED);

    	RefinedCrv = CagdCrvCopy(Crv);
	for (i = 0; i < n; i++)
	    RefinedCrv -> KnotVector[i] = *t++;
    }
    else if (n <= 0) {
	RefinedCrv = CagdCrvCopy(Crv);
    }
    else {
	int i, j, LengthKVt, NewLen;
	BspKnotAlphaCoeffStruct *A;
	CagdRType *MergedKVt, TMin, TMax;

	BspCrvDomain(Crv, &TMin, &TMax);

	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1])
		CAGD_FATAL_ERROR(CAGD_ERR_KNOT_NOT_ORDERED);
	for (i = 0; i < n; i++)
	    if (t[i] >= TMax)
	        t[i] = TMax - CAGD_DOMAIN_IRIT_EPS;

	/* Compute the Alpha refinement matrix. */
	MergedKVt = BspKnotMergeTwo(KnotVector, Length + Order,
				    t, n, 0, &LengthKVt);
	A = BspKnotEvalAlphaCoef(Order, KnotVector, Length,
				 MergedKVt, LengthKVt - Order, IsPeriodic);

	NewLen = Len + n;
	RefinedCrv = BspPeriodicCrvNew(NewLen, Order, 
				       IsPeriodic, Crv -> PType);

	/* Update the knot vector. */
	IritFree(RefinedCrv -> KnotVector);
	RefinedCrv -> KnotVector = MergedKVt;

	if (IsPeriodic) {
	    /* Make sure the knot vector is indeed periodic. */
	    BspKnotVerifyPeriodicKV(RefinedCrv -> KnotVector, Order,
				    CAGD_CRV_PT_LST_LEN(RefinedCrv));
	}

	/* Update the control mesh */
	for (j = IsNotRational; j <= MaxCoord; j++) {
	    CagdRType
		*ROnePts = RefinedCrv -> Points[j],
		*OnePts = Crv -> Points[j];
	    if (CAGD_IS_PERIODIC_CRV(Crv)) {
	        BspKnotAlphaLoopBlendPeriodic(A, 0, NewLen, OnePts,
					      Len, ROnePts);
	    }
	    else {
	        BspKnotAlphaLoopBlendNotPeriodic(A, 0, NewLen,
						 OnePts, ROnePts);
	    }
	}
	BspKnotFreeAlphaCoef(A);
    }

    BspKnotMakeRobustKV(RefinedCrv -> KnotVector,
			RefinedCrv -> Order + RefinedCrv -> Length);

    CAGD_PROPAGATE_ATTR(RefinedCrv, Crv);

    return RefinedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with order N.	     M
*   Degree raise is computed by multiplying by a constant 1 curve of order   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree to a NewOrder.                           M
*   NewOrder:   NewOrder for Crv.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of order NewOrder representing the same        M
*                     geometry as Crv.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdCrvStruct *BspCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder)
{
    int i, j, RaisedOrder, Length, KvLen1,
	Order = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RaisedCrv, *UnitCrv, *CpCrv;
    CagdRType *Kv;

    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    Length = Crv -> Length;
    KvLen1 = Order + Length - 1;
    Kv = Crv -> KnotVector;

    if (NewOrder < Order) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedOrder = NewOrder - Order + 1;

    UnitCrv = BspCrvNew(RaisedOrder, RaisedOrder,
			CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
    for (i = 0; i < RaisedOrder * 2; i++)
	UnitCrv -> KnotVector[i] = i >= RaisedOrder ? Kv[KvLen1] : Kv[0];
    for (i = 1; i <= MaxCoord; i++)
	for (j = 0; j < RaisedOrder; j++)
	    UnitCrv -> Points[i][j] = 1.0;

    RaisedCrv = BspCrvMult(Crv, UnitCrv);

    CagdCrvFree(UnitCrv);

    CAGD_PROPAGATE_ATTR(RaisedCrv, Crv);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return RaisedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with one degree higher. M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise it degree by one.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve with one degree higher representing the same   M
*                     geometry as Crv.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdCrvStruct *BspCrvDegreeRaise(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, i2, j, RaisedLen, Length,
	Order = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RaisedCrv, *CpCrv;

    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    Length = Crv -> Length;

    if (Order > 2)
	return BspCrvDegreeRaiseN(Crv, Order + 1);

    /* If curve is linear, degree raising means basically to increase the    */
    /* knot multiplicity of each segment by one and add a middle point for   */
    /* each such segment.						     */
    RaisedLen = Length * 2 - 1;
    RaisedCrv = BspCrvNew(RaisedLen, Order + 1, Crv -> PType);

    /* Update the control polygon; */
    for (j = IsNotRational; j <= MaxCoord; j++)		     /* First point. */
	RaisedCrv -> Points[j][0] = Crv -> Points[j][0];

    for (i = 1, i2 = 1; i < Length; i++, i2 += 2)
	for (j = IsNotRational; j <= MaxCoord; j++) {
	    RaisedCrv -> Points[j][i2] =
		Crv -> Points[j][i-1] * 0.5 + Crv -> Points[j][i] * 0.5;
	    RaisedCrv -> Points[j][i2 + 1] = Crv -> Points[j][i];
	}

    /* Update the knot vector. */
    for (i = 0; i < 3; i++)
	RaisedCrv -> KnotVector[i] = Crv -> KnotVector[0];

    for (i = 2, j = 3; i < Length; i++, j += 2)
	RaisedCrv -> KnotVector[j] = RaisedCrv -> KnotVector[j + 1] = 
	    Crv -> KnotVector[i];

    for (i = j; i < j + 3; i++)
	RaisedCrv -> KnotVector[i] = Crv -> KnotVector[Length];

    CAGD_PROPAGATE_ATTR(RaisedCrv, Crv);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return RaisedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the tangent to Crv at parameter value t. M
*   Algorithm: insert (order - 1) knots and return control polygon tangent.  M
*   The unnormalized normal does not equal dC/dt in its magnitude, only in   M
* its direction.							     M 
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) tangent.                    M
*   t:         The parameter at which to compute the unit tangent.           M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, returned is an unnormalized vector in the right     M
*	       direction of the tangent.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the tangent       M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvTangent, tangent                                                   M
*****************************************************************************/
CagdVecStruct *BspCrvTangent(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize)
{
    CagdVecStruct
        *V = BspCrvTangentAux(Crv, t, Normalize);

    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_UEPS && Crv -> Order > 1) {
        CagdRType TMin, TMax;

        /* Try to move a little. This location has zero speed. However,     */
        /* do it only once since we can be here forever. The "_tan"         */
        /* attribute guarantee we will try to move IRIT_EPS only once.      */
	CagdCrvDomain(Crv, &TMin, &TMax);
        V = BspCrvTangentAux(Crv,
			     t - TMin < TMax - t ? t + IRIT_EPS
					         : t - IRIT_EPS, Normalize);
    }

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a (unit) vector, equal to the tangent to Crv at parameter value t. *
*   Algorithm: insert (order - 1) knots and return control polygon tangent.  *
*   The unnormalized normal does not equal dC/dt in its magnitude, only in   *
* its direction.							     * 
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       Crv for which to compute a (unit) tangent.                    *
*   t:         The parameter at which to compute the unit tangent.           *
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    *
*              If FALSE, returned is an unnormalized vector in the right     *
*	       direction of the tangent.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdVecStruct *:  A pointer to a static vector holding the tangent       *
*                     information.                                           *
*****************************************************************************/
static CagdVecStruct *BspCrvTangentAux(const CagdCrvStruct *Crv,
				       CagdRType t,
				       CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct Tan;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdRType TMin, TMax, DKnots, Pt0[4], Pt1[4];
    CagdPType P0, P1;
    CagdCrvStruct *CpCrv;
    CagdPointType
        PType = Crv -> PType;
    int i, Index, OpenEnd, Len,
	k = Crv -> Order,
	FlipPts = FALSE,
	MaxCoord = IRIT_MIN(CAGD_NUM_OF_PT_COORD(PType), 3);
    CagdCrvStruct *RefinedCrv;

    /* Vanishing for constant curves. */
    if (k <= 1) {
	IRIT_PT_RESET(Tan.Vec);
	return &Tan;
    }

    if (CAGD_IS_PERIODIC_CRV(Crv))
        Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    OpenEnd = BspCrvHasOpenEC(Crv);
    Len = Crv -> Length;
    Index = BspKnotLastIndexL(Crv -> KnotVector, k + Len, t);

    CAGD_DOMAIN_GET_AND_VERIFY_CRV(t, Crv, TMin, TMax);

    if (IRIT_APX_EQ(t, TMin) && OpenEnd) {
	/* Use Crv starting tangent direction. */
	if (IsNotRational) {
	    CagdCoerceToE3(&Pt0[1], Crv -> Points, 0, PType);
	    CagdCoerceToE3(&Pt1[1], Crv -> Points, 1, PType);
	}
	else {
	    CagdCoerceToP3(Pt0, Crv -> Points, 0, PType);
	    CagdCoerceToP3(Pt1, Crv -> Points, 1, PType);
	}

	DKnots = Crv -> KnotVector[k] - Crv -> KnotVector[k - 1];
    }
    else if (IRIT_APX_EQ(t, TMax) && OpenEnd) {
	/* Use Crv ending tangent direction. */
	if (IsNotRational) {
	    CagdCoerceToE3(&Pt0[1], Crv -> Points, Len - 2, PType);
	    CagdCoerceToE3(&Pt1[1], Crv -> Points, Len - 1, PType);
	}
	else {
	    CagdCoerceToP3(Pt0, Crv -> Points, Len - 2, PType);
	    CagdCoerceToP3(Pt1, Crv -> Points, Len - 1, PType);
	}
	DKnots = Crv -> KnotVector[Len] - Crv -> KnotVector[Len - 1];
	FlipPts = TRUE;                       /* Pt1 is the point on Crv(t). */
    }
    else {
        int j,
	    Mult = BspKnotFindMult(Crv -> KnotVector, k, Len, t);

	/* Note (k - Mult - 1) could be negative in which case no refinement */
	/* will actually take place: RefinedCrv will be copied from Crv.     */
	RefinedCrv = BspCrvKnotInsertNSame(Crv, t, k - Mult - 1);

	for (j = IsNotRational; j <= MaxCoord; j++) {
	    Pt0[j] = RefinedCrv -> Points[j][Index - 1];
	    Pt1[j] = RefinedCrv -> Points[j][Index];
	}
	for (j = MaxCoord + 1; j <= 3; j++)
	    Pt0[j] = Pt1[j] = 0.0;

	DKnots = RefinedCrv -> KnotVector[Index + 1] - 
		 RefinedCrv -> KnotVector[Index];
	FlipPts = TRUE;                       /* Pt1 is the point on Crv(t). */

	CagdCrvFree(RefinedCrv);
    }

    IRIT_PT_COPY(P0, &Pt0[1]);
    IRIT_PT_COPY(P1, &Pt1[1]);
    if (IsNotRational) {
	IRIT_PT_SUB(Tan.Vec, P1, P0);
    }
    else if (Pt0[0] == 0.0 || Pt1[0] == 0.0) {
        IRIT_VEC_RESET(Tan.Vec);
    }
    else {
        IRIT_PT_SCALE(P0, 1.0 / Pt0[0]);
        IRIT_PT_SCALE(P1, 1.0 / Pt1[0]);
	IRIT_PT_SUB(Tan.Vec, P1, P0);
    }

    if (CAGD_SQR_LEN_VECTOR(Tan) > IRIT_UEPS) {
        if (IsNotRational) {
	    if (Normalize)
	        CAGD_NORMALIZE_VECTOR_MSG_ZERO(Tan)/* Normalize the vector. */
	    else
	        IRIT_VEC_SCALE(Tan.Vec, (k - 1) / DKnots);
	}
	else {
	    /* Make P1 hold the derivative.  P0 already holds the position. */
	    if (FlipPts) {
	        for (i = 0; i < 4; i++) {
		    Pt0[i] = (Pt1[i] - Pt0[i]) * (k - 1) / DKnots;
		    IRIT_SWAP(CagdRType, Pt1[i], Pt0[i]);
		}
	    }
	    else {
	        for (i = 0; i < 4; i++)
		    Pt1[i] = (Pt1[i] - Pt0[i]) * (k - 1) / DKnots;
	    }

	    /* And use to quotient rule to get the exact tangent. */
	    for (i = 1; i <= 3; i++)
	        Tan.Vec[i - 1] = (Pt1[i] * Pt0[0] - Pt1[0] * Pt0[i])
							       / IRIT_SQR(Pt0[0]);

	    if (Normalize)
	        CAGD_NORMALIZE_VECTOR_MSG_ZERO(Tan);/* Normalize the vector. */
	}

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
    }

    return &Tan;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the binormal to Crv at parameter value t.M
*   Algorithm: insert (order - 1) knots and using 3 consecutive control      M
* points at the refined location (p1, p2, p3), compute to binormal to be the M
* cross product of the two vectors (p1 - p2) and (p2 - p3).		     M
*   Since a curve may have not BiNormal at inflection points or if the 3     M
* points are collinear, NULL will be returned at such cases.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) binormal.                   M
*   t:         The parameter at which to compute the unit binormal.          M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the binormal      M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvBiNormal, binormal                                                 M
*****************************************************************************/
CagdVecStruct *BspCrvBiNormal(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize)
{
    CagdVecStruct
        *V = BspCrvBiNormalAux(Crv, t, Normalize);

    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_UEPS && Crv -> Order > 2) {
        CagdRType TMin, TMax;

        /* Try to move a little. This location has zero speed. However,     */
        /* do it only once since we can be here forever. The "_tan"         */
        /* attribute guarantee we will try to move IRIT_EPS only once.      */
	CagdCrvDomain(Crv, &TMin, &TMax);
        V = BspCrvBiNormalAux(Crv,
			      t - TMin < TMax - t ? t + IRIT_EPS
					          : t - IRIT_EPS, Normalize);
    }

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a (unit) vector, equal to the binormal to Crv at parameter value t.*
*   Algorithm: insert (order - 1) knots and using 3 consecutive control      *
* points at the refined location (p1, p2, p3), compute to binormal to be the *
* cross product of the two vectors (p1 - p2) and (p2 - p3).		     *
*   Since a curve may have not BiNormal at inflection points or if the 3     *
* points are collinear, NULL will be returned at such cases.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       Crv for which to compute a (unit) binormal.                   *
*   t:         The parameter at which to compute the unit binormal.          *
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    *
*              If FALSE, length is a function of given parametrization.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdVecStruct *:  A pointer to a static vector holding the binormal      *
*                     information.                                           *
*****************************************************************************/
CagdVecStruct *BspCrvBiNormalAux(const CagdCrvStruct *Crv,
				 CagdRType t,
				 CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct P3;
    CagdVecStruct P1, P2;
    CagdRType TMin, TMax;
    CagdCrvStruct *CpCrv;
    int Len, OpenEnd, Index,
        k = Crv -> Order;
    CagdPointType
	PType = Crv -> PType;
    CagdCrvStruct *RefinedCrv;

    /* Vanishing for linear curves. */
    if (k <= 2) {
	IRIT_PT_RESET(P3.Vec);
	return &P3;
    }

    if (CAGD_IS_PERIODIC_CRV(Crv))
        Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    OpenEnd = BspCrvHasOpenEC(Crv);
    Len = Crv -> Length;
    Index = BspKnotLastIndexL(Crv -> KnotVector, k + Len, t);

    CAGD_DOMAIN_GET_AND_VERIFY_CRV(t, Crv, TMin, TMax);

    if (IRIT_APX_EQ(t, TMin) && OpenEnd) {
	/* Use Crv starting tangent direction. */
	CagdCoerceToE3(P1.Vec, Crv -> Points, 0, PType);
	CagdCoerceToE3(P2.Vec, Crv -> Points, 1, PType);
	CagdCoerceToE3(P3.Vec, Crv -> Points, 2, PType);
    }
    else if (IRIT_APX_EQ(t, TMax) && OpenEnd) {
	/* Use Crv ending tangent direction. */
	CagdCoerceToE3(P1.Vec, Crv -> Points, Len - 3, PType);
	CagdCoerceToE3(P2.Vec, Crv -> Points, Len - 2, PType);
	CagdCoerceToE3(P3.Vec, Crv -> Points, Len - 1, PType);
    }
    else {
        int i,
	    Mult = BspKnotFindMult(Crv -> KnotVector, k, Len, t);

	/* Note (k - Mult - 1) could be negative in which case no refinement */
	/* will actually take place: RefinedCrv will be copied from Crv.     */
	RefinedCrv = BspCrvKnotInsertNSame(Crv, t, k - Mult - 1);

	CagdCoerceToE3(P1.Vec, RefinedCrv -> Points, Index, PType);
	CagdCoerceToE3(P2.Vec, RefinedCrv -> Points, Index + 1, PType);
	CagdCoerceToE3(P3.Vec, RefinedCrv -> Points, Index + 2, PType);

	for (i = Index - 1; IRIT_PT_APX_EQ(P1.Vec, P2.Vec) && i >= 0; i--)
	     CagdCoerceToE3(P1.Vec, RefinedCrv -> Points, i, PType);
	for (i = Index + 3;
	     IRIT_PT_APX_EQ(P2.Vec, P3.Vec) && i < RefinedCrv -> Length;
	     i++)
	     CagdCoerceToE3(P3.Vec, RefinedCrv -> Points, i, PType);

	CagdCrvFree(RefinedCrv);
    }

    CAGD_SUB_VECTOR(P1, P2);
    CAGD_SUB_VECTOR(P2, P3);

    IRIT_CROSS_PROD(P3.Vec, P1.Vec, P2.Vec);

    if (CAGD_SQR_LEN_VECTOR(P3) > IRIT_UEPS) {
        if (Normalize)
	    CAGD_NORMALIZE_VECTOR_MSG_ZERO(P3);    /* Normalize the vector. */

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
    }

    return &P3;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the normal of Crv at parameter value t.  M
*   Algorithm: returns the cross product of the curve tangent and binormal.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) normal.                     M
*   t:         The parameter at which to compute the unit normal.            M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the normal        M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvNormal, normal                                                     M
*****************************************************************************/
CagdVecStruct *BspCrvNormal(const CagdCrvStruct *Crv,
			    CagdRType t,
			    CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct N, *T, *B;

    T = BspCrvTangent(Crv, t, FALSE);
    B = BspCrvBiNormal(Crv, t, FALSE);

    if (T == NULL || B == NULL)
	return NULL;

    IRIT_CROSS_PROD(N.Vec, B -> Vec, T -> Vec);

    if (Normalize)
	CAGD_NORMALIZE_VECTOR_MSG_ZERO(N);         /* Normalize the vector. */

    return &N;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = Degree * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.   V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, CagdCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational, M
*   BspCrvDeriveScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDerive, derivatives                                                M
*****************************************************************************/
CagdCrvStruct *BspCrvDerive(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, Len,
	k = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType *Kv;
    CagdCrvStruct *DerivedCrv, *CpCrv;

    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (!GlblDeriveScalar && !IsNotRational) {
	DerivedCrv = BspCrvDeriveRational(Crv);
	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	return DerivedCrv;
    }

    Len = Crv -> Length;
    Kv = Crv -> KnotVector;
    DerivedCrv = BspCrvNew(IRIT_MAX(Len - 1, 1), IRIT_MAX(k - 1, 1),
		           Crv -> PType);

    if (k >= 2) {
	for (i = 0; i < Len - 1; i++) {
	    CagdRType
		Denom = Kv[i + k] - Kv[i + 1];

	    if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
		Denom = IRIT_UEPS;

	    for (j = IsNotRational; j <= MaxCoord; j++)
		DerivedCrv -> Points[j][i] = (k - 1) *
		    (Crv -> Points[j][i + 1] - Crv -> Points[j][i]) / Denom;
	}
    }
    else {
	for (i = 0; i < IRIT_MAX(Len - 1, 1); i++)
	    for (j = IsNotRational; j <= MaxCoord; j++)
		DerivedCrv -> Points[j][i] = 0.0;
		 
    }

    CAGD_GEN_COPY(DerivedCrv -> KnotVector,
		  &Crv -> KnotVector[k < 2 ? 0 : 1],
		  sizeof(CagdRType) * (IRIT_MAX(k - 1, 1) +
				       IRIT_MAX(Len - 1, 1)));

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return DerivedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*   For a Euclidean curve this is the same as CagdCrvDerive but for a        M
* rational curve the returned curve is not the vector field but simply the   M
* derivatives of all the curve's coefficients, including the weights.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, CagdCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational  M
*   BspCrvDerive, BzrCrvDeriveScalar, CagdCrvDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDeriveScalar, derivatives                                          M
*****************************************************************************/
CagdCrvStruct *BspCrvDeriveScalar(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *TCrv;

    GlblDeriveScalar = TRUE;

    TCrv = BspCrvDerive(Crv);

    GlblDeriveScalar = FALSE;

    return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline curve, equal to the integral of the given Bspline    M
* crv.		                                                             M
* The given Bspline curve should be nonrational.			     M
*									     V
* 	      l		   l		   l	   l+1			     V
*   /	     /-		   -	  /	   -   P    -			     V
*  |	    | \	    n	   \	 |  n	   \    i   \              n+1	     V
*  | C(t) = | / P  B (t) = / P   | B (t) = / -----  / ( t   - t ) B   (t) =  V
* / 	   /  -	 i  i	   -  i /   i	   - n + 1  -    j+n   j   j	     V
*            i=0          i=0             i=0     j=i+1			     V
*									     V
*        l+1 j-1							     V
*         -   -								     V
*     1   \   \			n+1					     V
* = ----- /   / P  ( t   - t ) B   (t)					     V
*   n + 1 -   -  i    j+n   j	j					     V
*        j=1 i=0							     V
*									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to integrate.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Integrated curve.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvIntegrate, BspSrfIntegrate, CagdCrvIntegrate                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvIntegrate, integrals                                               M
*****************************************************************************/
CagdCrvStruct *BspCrvIntegrate(const CagdCrvStruct *Crv)
{
    int i, j, k, Len,
	n = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *IntCrv, *CpCrv;
    CagdRType *Kv;

    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (CAGD_IS_RATIONAL_CRV(Crv))
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

    Len = Crv -> Length;
    Kv = Crv -> KnotVector;

    IntCrv = BspCrvNew(Len + 1, n + 1, Crv -> PType);

    /* Copy the knot vector and duplicate the two end knots. */
    CAGD_GEN_COPY(&IntCrv -> KnotVector[1], Kv, sizeof(CagdRType) * (Len + n));
    IntCrv -> KnotVector[0] = Kv[0];
    IntCrv -> KnotVector[Len + n + 1] = Kv[Len + n - 1];
    Kv = IntCrv -> KnotVector;

    for (k = 1; k <= MaxCoord; k++) {
	CagdRType
	    *Points = Crv -> Points[k],
	    *IntPoints = IntCrv -> Points[k];

	for (j = 0; j < Len + 1; j++) {
	    IntPoints[j] = 0.0;
	    for (i = 0; i < j; i++)
	        IntPoints[j] += Points[i] * (Kv[i + n + 1] - Kv[i + 1]);
	    IntPoints[j] /= n;
	}
    }

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return IntCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply the Moebius transformation to a rational Bspline curve.            M
* See "Moebius reparametrization of rational Bsplines", by Lee & Lucian,     M
* CAGD 8 (1991) pp 213-215.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:       Curve to apply the Moebius transformation to.                M
*   c:          The scaling coefficient - c^n is the ratio between the first M
*	        and last weight of the curve.  				     M
*		If c == 0, the first and last weights are made equal.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The modified curve with the same shape but different  M
*		speed.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvMoebiusTransform, BspSrfMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvMoebiusTransform                                                   M
*****************************************************************************/
CagdCrvStruct *BspCrvMoebiusTransform(const CagdCrvStruct *CCrv, CagdRType c)
{
    int i, j,
	Order = CCrv -> Order,
	Length = CCrv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType);
    CagdRType c0, TMin, TMax, *KV, **Points,
	MaxW = IRIT_UEPS;
    CagdCrvStruct *Crv;

    if (!CAGD_IS_BSPLINE_CRV(CCrv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_CRV);
	return NULL;
    }

    if (!CAGD_IS_RATIONAL_CRV(CCrv)) {
        if (c == 1.0)
	    return CagdCrvCopy(CCrv);
	else
	    Crv = CagdCoerceCrvTo(CCrv, CAGD_MAKE_PT_TYPE(TRUE, MaxCoord), FALSE);
    }
    else
        Crv = CagdCrvCopy(CCrv);
    Points = Crv -> Points;

    /* Save original curve domain and temporary map to zero to one. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    KV = Crv -> KnotVector;
    BspKnotAffineTransOrder2(KV, Order, CAGD_CRV_PT_LST_LEN(Crv) + Order,
			     0.0, 1.0);

    if (Points[0][0] == 0 || Points[0][Length - 1] == 0) {
	CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
	return NULL;
    }

    if (c == 0.0) {
        c = Points[0][0] / Points[0][Length - 1];
	c = pow(c, 1.0 / (Order - 1.0));
    }

    /* Update the control points of the curve. */
    for (i = 1, c0 = 1.0; i < Order; i++)
	c0 *= MOEBIUS_MEU(KV[i]);
    for (i = 0; i < Length; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] /= c0;
	c0 *= MOEBIUS_MEU(KV[Order + i]) / MOEBIUS_MEU(KV[i + 1]);
    }

    /* Normalize all weights so largest has magnitude of one. */
    for (i = 0; i < Length; i++) {
	if (MaxW < IRIT_FABS(Points[0][i]))
	    MaxW = IRIT_FABS(Points[0][i]);
    }
    for (i = 0; i < Length; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] /= MaxW;
    }

    /* Update the knot sequence of the curve. */
    for (i = 0; i < CAGD_CRV_PT_LST_LEN(Crv) + Order; i++)
	KV[i] = MOEBIUS_REPARAM(KV[i]);

    BspKnotAffineTransOrder2(KV, Order, CAGD_CRV_PT_LST_LEN(Crv) + Order,
			     TMin, TMax);    
    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a list of points into a polyline.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:      Input point list to convert into a polyline.                   M
*   Params:   Optional polylines of parameters if found is saved here.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:   Converted polyline.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtLinBspCrv2Polyline, CagdCnvrtPolyline2LinBspCrv,		     M
*   CagdCnvrtPolyline2PtList                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPtList2Polyline                                                 M
*****************************************************************************/
CagdPolylineStruct *CagdCnvrtPtList2Polyline(const CagdPtStruct *Pts,
					     CagdPolylineStruct **Params)
{
    int PtsLen = CagdListLength(Pts);
    CagdBType
        SaveParamVals = Params != NULL &&
                        !IP_ATTR_IS_BAD_REAL(AttrGetRealAttrib(Pts -> Attr,
							    "SaveParamVals"));

    if (SaveParamVals)
        *Params = NULL;

    if (PtsLen > 0) {
        int i;
	const CagdPtStruct *Pt;
	CagdPolylineStruct
	    *Poly = CagdPolylineNew(PtsLen);

	if (SaveParamVals)
	    *Params = CagdPolylineNew(PtsLen);

	for (Pt = Pts, i = 0; Pt != NULL; Pt = Pt -> Pnext, i++) {
	    IRIT_PT_COPY(Poly -> Polyline[i].Pt, Pt -> Pt);
	    if (SaveParamVals) {
	        IRIT_PT_RESET((*Params) -> Polyline[i].Pt);
		(*Params) -> Polyline[i].Pt[0] =
		    AttrGetRealAttrib(Pt -> Attr, "SaveParamVals");
	    }
	}

	return Poly;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a polyline into a list of points.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:   Input polyline to convert into a point list.	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   Converted point list.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtLinBspCrv2Polyline, CagdCnvrtPolyline2LinBspCrv,		     M
*   CagdCnvrtPtList2Polyline				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPolyline2PtList                                                 M
*****************************************************************************/
CagdPtStruct *CagdCnvrtPolyline2PtList(const CagdPolylineStruct *Poly)
{
    int i,
	PlLen = Poly -> Length;
    CagdPtStruct *Pt,
	*Pts = NULL;

    for (i = 0; i < PlLen; i++) {
        Pt = CagdPtNew();
        IRIT_PT_COPY(Pt -> Pt, Poly -> Polyline[i].Pt);
	IRIT_LIST_PUSH(Pt, Pts);
    }

    if (Pts != NULL)
        CagdListReverse(Pts);

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new linear Bspline curve constructed from the given polyline.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:     To convert to a linear bspline curve.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A linear Bspline curve representing Poly.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserPolylines2LinBsplineCrvs, CagdCnvrtPolyline2LinBspCrv,		     M
*   CagdCnvrtLinBspCrv2Polyline, CagdCnvrtPtList2Polyline	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPolyline2LinBspCrv, linear curves, conversion                   M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtPolyline2LinBspCrv(const CagdPolylineStruct *Poly)
{
    int i, j,
	Length = Poly -> Length;
    CagdCrvStruct *NCrv,
	*Crv = BspCrvNew(Length, 2, CAGD_PT_E3_TYPE);
    CagdRType
	**Points = Crv -> Points;
    CagdPolylnStruct
	*Pts = Poly -> Polyline;

    for (i = j = 0; i < Length; i++, Pts++) {
	if (j > 0 &&
	    IRIT_APX_EQ_EPS(Points[1][j - 1], Pts -> Pt[0], CAGD_SAME_PT_EPS) &&
	    IRIT_APX_EQ_EPS(Points[2][j - 1], Pts -> Pt[1], CAGD_SAME_PT_EPS) &&
	    IRIT_APX_EQ_EPS(Points[3][j - 1], Pts -> Pt[2], CAGD_SAME_PT_EPS)) {
	    /* Must skip identical points. */
	    continue;
	}

	Points[1][j] = Pts -> Pt[0];
	Points[2][j] = Pts -> Pt[1];
	Points[3][j++] = Pts -> Pt[2];
    }

    if (Crv -> Length != j) {
        int Size = sizeof(CagdRType) * j;

	NCrv = BspCrvNew(j, j > 1 ? 2 : 1, CAGD_PT_E3_TYPE);
	for (i = 1; i <= 3; i++)
	CAGD_GEN_COPY(NCrv -> Points[i], Crv -> Points[i], Size);
	CagdCrvFree(Crv);
	Crv = NCrv;
    }

    if (j == 1) {
	Crv -> Order = 1;
	BspKnotUniformOpen(1, 1, Crv -> KnotVector);
    }
    else {
	BspKnotUniformOpen(j, 2, Crv -> KnotVector);
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new polyline representing same geometry as the given linear      M
* Bspline curve.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     A linear Bspline curve to convert to a polyline.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:   A polyline same as linear curve Crv.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserPolylines2LinBsplineCrvs, UserPolyline2LinBsplineCrv,		     M
*   CagdCnvrtPolyline2LinBspCrv, CagdCnvrtPtList2Polyline                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtLinBspCrv2Polyline, linear curves, conversion                   M
*****************************************************************************/
CagdPolylineStruct *CagdCnvrtLinBspCrv2Polyline(const CagdCrvStruct *Crv)
{
    int i, j,
	Length = Crv -> Length;
    CagdPolylineStruct
        *Poly = CagdPolylineNew(Length);
    CagdRType
	* const *CrvPts = Crv -> Points;
    CagdPolylnStruct
	*PlPts = Poly -> Polyline;

    for (i = j = 0; i < Length; i++) {
	CagdPType Pt;

	CagdCoerceToE3(Pt, CrvPts, i, Crv -> PType);

	if (j > 0 &&
	    IRIT_APX_EQ_EPS(Pt[0], PlPts[-1].Pt[0], CAGD_SAME_PT_EPS) &&
	    IRIT_APX_EQ_EPS(Pt[1], PlPts[-1].Pt[1], CAGD_SAME_PT_EPS) &&
	    IRIT_APX_EQ_EPS(Pt[2], PlPts[-1].Pt[2], CAGD_SAME_PT_EPS)) {
	    /* Must skip identical points. */
	    continue;
	}

	PlPts -> Pt[0] = Pt[0];
	PlPts -> Pt[1] = Pt[1];
	PlPts -> Pt[2] = Pt[2];
	PlPts++;
	j++;
    }

    Poly -> Length = j;

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline curve to a Bspline curve with floating end conditions.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Bspline curve to convert to floating end conditions. Assume   M
*              Crv is either periodic or has floating end condition.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A Bspline curve with floating end conditions,          M
*                     representing the same geometry as Crv.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPeriodic2FloatCrv, conversion                                   M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtPeriodic2FloatCrv(const CagdCrvStruct *Crv)
{
    int i,
	Order = Crv -> Order,
	Length = Crv -> Length,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *NewCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_CRV_EXPECT);
	return NULL;
    }

    if (!CAGD_IS_PERIODIC_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_PERIODIC_EXPECTED);
	return NULL;
    }

    NewCrv = BspCrvNew(Length + Order - 1, Order, Crv -> PType);

    CAGD_GEN_COPY(NewCrv -> KnotVector, Crv -> KnotVector,
		  sizeof(CagdRType) * (Length + Order + Order - 1));

    for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType); i <= MaxAxis; i++) {
	CAGD_GEN_COPY(NewCrv -> Points[i], Crv -> Points[i],
		      sizeof(CagdRType) * Length);
	CAGD_GEN_COPY(&NewCrv -> Points[i][Length], Crv -> Points[i],
		      sizeof(CagdRType) * (Order - 1));
    }

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewCrv -> Points[i] = NULL;

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a float Bspline curve to a Bspline curve with open end	     M
* conditions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Bspline curve to convert to open end conditions.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A Bspline curve with open end conditions,	             M
*                     representing the same geometry as Crv.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtFloat2OpenCrv, conversion                                       M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtFloat2OpenCrv(const CagdCrvStruct *Crv)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *NewCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_CRV_EXPECT);
	return NULL;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    NewCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);

    /* If curve conversion was initialized as a periodic curve last contrl  */
    /* points should be identical to first one.  Make it so.		    */
    {
        CagdPointType
	    PtType = NewCrv -> PType;
	CagdBType
	    IsRational = CAGD_IS_RATIONAL_PT(PtType);
	int j,
	    Last = NewCrv -> Length - 1,
	    NumCoords = CAGD_NUM_OF_PT_COORD(PtType);
	CagdRType
	    **Points = NewCrv -> Points;

	for (j = !IsRational; j <= NumCoords; j++)
	    if (!IRIT_APX_EQ(Points[j][Last], Points[j][0]))
	        break;

	if (j > NumCoords) {		   /* Very similar - make identical. */
	    for (j = !IsRational; j <= NumCoords; j++)
	        Points[j][Last] = Points[j][0];
	}
    }

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline curve to a Bspline curve with open end conditions.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Bspline curve to convert to open end conditions.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A Bspline curve with open end conditions,	             M
*                     representing the same geometry as Crv.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBsp2OpenCrv, conversion                                         M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtBsp2OpenCrv(const CagdCrvStruct *Crv)
{
    CagdBType Periodic;
    CagdRType TMin, TMax;
    CagdCrvStruct *NewCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_CRV_EXPECT);
	return NULL;
    }

    if ((Periodic = CAGD_IS_PERIODIC_CRV(Crv)) != FALSE) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtPeriodic2FloatCrv(Crv);

	CagdCrvDomain(TCrv, &TMin, &TMax);
	NewCrv = CagdCrvRegionFromCrv(TCrv, TMin, TMax);
	CagdCrvFree(TCrv);
    }
    else if (!BspCrvHasOpenEC(Crv)) {
	CagdCrvDomain(Crv, &TMin, &TMax);
	NewCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);
    }
    else
        NewCrv = CagdCrvCopy(Crv);

    if (Periodic) {   /* Force last control point to be identical to first. */
        CagdPointType
	    PtType = NewCrv -> PType;
	CagdBType
	    IsRational = CAGD_IS_RATIONAL_PT(PtType);
	int j,
	    Last = NewCrv -> Length - 1,
	    NumCoords = CAGD_NUM_OF_PT_COORD(PtType);
	CagdRType
	    **Points = NewCrv -> Points;

	for (j = !IsRational; j <= NumCoords; j++)
	    Points[j][Last] = Points[j][0];
    }

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert a curve into a list of control points.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   To the curve to convert to list of control points.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCtlPtStruct *:    List of control points of curve Crv.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtCrvToCtlPts                                                     M
*****************************************************************************/
CagdCtlPtStruct *CagdCnvrtCrvToCtlPts(const CagdCrvStruct *Crv)
{
    CagdPointType
	PtType = Crv -> PType;
    int i, j,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtType);
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PtType);
    CagdRType
	* const *Points = Crv -> Points;
    CagdCtlPtStruct *Pt,
	*CtlPts = NULL;

    for (i = 0; i < Crv -> Length; i++) {
	Pt = CagdCtlPtNew(PtType);

	for (j = !IsRational; j <= NumCoords; j++) {
	    Pt -> Coords[j] = Points[j][i];
	}

	IRIT_LIST_PUSH(Pt, CtlPts);
    }
    
    return CagdListReverse(CtlPts);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reparameterize a curve to follow a desired parametrization.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        The curve to update its paraametrization.                    M
*   ParamType:  The desired parametrization type: uniform, chord len., etc.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpBuildKVs, BspReparameterizeSrf                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspReparameterizeCrv                                                     M
*****************************************************************************/
void BspReparameterizeCrv(CagdCrvStruct *Crv,
			  CagdParametrizationType ParamType)
{
    CagdRType *NewPtKnots, *NewKV;
    CagdCtlPtStruct *CtlPts;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_BSP_CRV_EXPECT);
	return;
    }

    CtlPts = CagdCnvrtCrvToCtlPts(Crv);
    BspCrvInterpBuildKVs(CtlPts, Crv -> Order, Crv -> Length, ParamType,
			 Crv -> Periodic, &NewPtKnots, &NewKV);
    IritFree(NewPtKnots);
    CagdCtlPtFreeList(CtlPts);

    IritFree(Crv -> KnotVector);

    Crv -> KnotVector = NewKV;
}
