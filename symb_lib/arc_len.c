/******************************************************************************
* Arc_len.c - arc length functions and unit length normalization.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Apr. 93.					      *
******************************************************************************/

#include "symb_loc.h"

#define MAX_ALEN_IMPROVE_ITERS 			5
#define ARCLEN_CONST_SET_IRIT_EPS 		0.001

/*****************************************************************************
* DESCRIPTION:                                                               M
* Normalizes the given vector field curve to be a unit length curve, by      M
* computing a scalar curve to multiply with this vector field curve.         M
*   Returns the multiplied curve if Mult, or otherwise just the scalar       M
* curve.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:     Curve to approximate a unit size for.			     M
*   Mult:        Do we want to multiply the computed scalar curve with Crv?  M
*   Epsilon:     Accuracy required of this approximation.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A scalar curve to multiply OrigCrv so a unit size      M
*                     curve will result if Mult is FALSE, or the actual      M
*                     unit size vector field curve, if Mult.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvUnitLenCtlPts			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvUnitLenScalar, unit vector field                                  M
*****************************************************************************/
CagdCrvStruct *SymbCrvUnitLenScalar(const CagdCrvStruct *OrigCrv,
				    CagdBType Mult,
				    CagdRType Epsilon)
{
    int i, j;
    CagdCrvStruct
	*ScalarCrv = NULL,
	*Crv = CAGD_IS_BEZIER_CRV(OrigCrv) ? CagdCnvrtBzr2BspCrv(OrigCrv)
					   : CagdCrvCopy(OrigCrv);
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(Crv);

    for (i = 0; i < MAX_ALEN_IMPROVE_ITERS; i++) {
	CagdCrvStruct *UnitCrvAprx, *ScalarCrvSqr,
	    *DotProdCrv = SymbCrvDotProd(Crv, Crv);
	CagdRType Min, Max, *SCPoints,
	    *DPPoints = DotProdCrv -> Points[1];

	if (ScalarCrv)
	    CagdCrvFree(ScalarCrv);
	ScalarCrv = CagdCrvCopy(DotProdCrv);
	SCPoints = ScalarCrv -> Points[1];

	/* Compute an approximation to the inverse function. */
	for (j = 0; j < ScalarCrv -> Length; j++, DPPoints++) {
	    *SCPoints++ = *DPPoints > 0.0 ? 1.0 / sqrt(*DPPoints) : 1.0;
	}
	ScalarCrvSqr = SymbCrvMult(ScalarCrv, ScalarCrv);

	/* Multiply the two to test the error. */
	UnitCrvAprx = SymbCrvMult(ScalarCrvSqr, DotProdCrv);
	CagdCrvFree(ScalarCrvSqr);

	CagdCrvMinMax(UnitCrvAprx, 1, &Min, &Max);

	if (1.0 - Min < Epsilon && Max - 1.0 < Epsilon) {
	    CagdCrvFree(UnitCrvAprx);
	    CagdCrvFree(DotProdCrv);
	    break;
	}
	else {
	    /* Refine in regions where the error is too large. */
	    int k,
	        Length = UnitCrvAprx -> Length,
	        Order = UnitCrvAprx -> Order,
	        KVLen = Length + Order;
	    CagdRType
		*KV = UnitCrvAprx -> KnotVector,
		*RefKV = IritMalloc(sizeof(CagdRType) * 2 * Length),
	        *Nodes = BspKnotNodes(KV, KVLen, Order),
		**Points = UnitCrvAprx -> Points;

	    for (j = k = 0; j < Length; j++) {
		CagdRType
		    V = 1.0 - (IsRational ? Points[1][j] / Points[0][j]
					  : Points[1][j]);

		if (IRIT_FABS(V) > Epsilon) {
		    int Index = BspKnotLastIndexLE(KV, KVLen, Nodes[j]);

		    if (IRIT_APX_EQ(KV[Index], Nodes[j])) {
			if (j > 0)
			    RefKV[k++] = (Nodes[j] + Nodes[j - 1]) * 0.5;
			if (j < Length - 1)
			    RefKV[k++] = (Nodes[j] + Nodes[j + 1]) * 0.5;
		    }
		    else
			RefKV[k++] = Nodes[j];
		}
	    }
	    CagdCrvFree(UnitCrvAprx);
	    CagdCrvFree(DotProdCrv);

	    IritFree(Nodes);

	    if (k == 0) {
		/* No refinement was found needed - return current curve. */
		IritFree(RefKV);
		break;
	    }
	    else {
		CagdCrvStruct
		    *CTmp = CagdCrvRefineAtParams(Crv, FALSE, RefKV, k);

		IritFree(RefKV);
		CagdCrvFree(Crv);
		Crv = CTmp;
	    }

	}
    }

    CagdCrvFree(Crv);

    /* Error is probably within bounds - returns this unit length approx. */
    if (Mult) {
	CagdCrvStruct *CrvX, *CrvY, *CrvZ, *CrvW, *CTmp;
	int MaxCoord = CAGD_NUM_OF_PT_COORD(OrigCrv -> PType);

	SymbCrvSplitScalar(ScalarCrv, &CrvW, &CrvX, &CrvY, &CrvZ);
	CagdCrvFree(ScalarCrv);
	ScalarCrv = SymbCrvMergeScalar(CrvW,
				       CrvX,
				       MaxCoord > 1 ? CrvX : NULL,
				       MaxCoord > 2 ? CrvX : NULL);
	CagdCrvFree(CrvX);
	if (CrvW)
	    CagdCrvFree(CrvW);

	CTmp = SymbCrvMult(ScalarCrv, OrigCrv);
	CagdCrvFree(ScalarCrv);

	return CTmp;
    }
    else {
	return ScalarCrv;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the curve which is a square root approximation to a given scalar  M
* curve, to within epsilon.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:    Scalar curve to approximate its square root function.        M
*   Epsilon:    Accuracy of approximation.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve approximating the square root of OrigCrv.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSqrtScalar, square root                                           M
*****************************************************************************/
CagdCrvStruct *SymbCrvSqrtScalar(const CagdCrvStruct *OrigCrv,
				 CagdRType Epsilon)
{
    int i, j;
    CagdCrvStruct
	*ScalarCrv = NULL,
	*Crv = CAGD_IS_BEZIER_CRV(OrigCrv) ? CagdCnvrtBzr2BspCrv(OrigCrv)
					   : CagdCrvCopy(OrigCrv);
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(Crv);

    for (i = 0; i < MAX_ALEN_IMPROVE_ITERS; i++) {
	CagdCrvStruct *ScalarCrvSqr, *ErrorCrv;
	CagdRType Min, Max, *SCPoints,
	    *Points = Crv -> Points[1],
	    *WPoints = IsRational ? Crv -> Points[0] : NULL;

	if (ScalarCrv)
	    CagdCrvFree(ScalarCrv);
	ScalarCrv = CagdCrvCopy(Crv);
	SCPoints = ScalarCrv -> Points[1];

	/* Compute an approximation to the inverse function. */
	for (j = 0; j < ScalarCrv -> Length; j++) {
	    CagdRType
		V = IsRational ? *Points++ / *WPoints++ : *Points++;

	    *SCPoints++ = V >= 0.0 ? sqrt(V) : 0.0;
	}
	ScalarCrvSqr = SymbCrvMult(ScalarCrv, ScalarCrv);

	/* Compare the two to test the error. */
	ErrorCrv = SymbCrvSub(ScalarCrvSqr, Crv);
	CagdCrvFree(ScalarCrvSqr);

	CagdCrvMinMax(ErrorCrv, 1, &Min, &Max);

	if (Min > -Epsilon && Max < Epsilon) {
	    CagdCrvFree(ErrorCrv);
	    break;
	}
	else {
	    /* Refine in regions where the error is too large. */
	    int k,
	        Length = ErrorCrv -> Length,
	        Order = ErrorCrv -> Order,
	        KVLen = Length + Order;
	    CagdRType
		*KV = ErrorCrv -> KnotVector,
		*RefKV = IritMalloc(sizeof(CagdRType) * 2 * Length),
	        *Nodes = BspKnotNodes(KV, KVLen, Order),
		**ErrPoints = ErrorCrv -> Points;

	    for (j = k = 0; j < Length; j++) {
		CagdRType
		    V = 1.0 - (IsRational ? ErrPoints[1][j] / ErrPoints[0][j]
					  : ErrPoints[1][j]);

		if (IRIT_FABS(V) > Epsilon) {
		    int Index = BspKnotLastIndexLE(KV, KVLen, Nodes[j]);

		    if (IRIT_APX_EQ(KV[Index], Nodes[j])) {
			if (j > 0)
			    RefKV[k++] = (Nodes[j] + Nodes[j - 1]) * 0.5;
			if (j < Length - 1)
			    RefKV[k++] = (Nodes[j] + Nodes[j + 1]) * 0.5;
		    }
		    else
			RefKV[k++] = Nodes[j];
		}
	    }
	    CagdCrvFree(ErrorCrv);

	    IritFree(Nodes);

	    if (k == 0) {
		/* No refinement was found needed - return current curve. */
		IritFree(RefKV);
		break;
	    }
	    else {
		CagdCrvStruct
		    *CTmp = CagdCrvRefineAtParams(Crv, FALSE, RefKV, k);

		IritFree(RefKV);
		CagdCrvFree(Crv);
		Crv = CTmp;
	    }

	}
    }

    CagdCrvFree(Crv);

    return ScalarCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a scalar curve approximating the arc length of given curve Crv.   M
* Arc length is astimated by computing the square of Crv's first derivative  M
* approximating its square root and integrating symbolically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To approximate its arc length scalar field.                   M
*   Epsilon:   Accuracy of approximating.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A scalar field approximating Crv arc length.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvArcLenSclrCrv, arc length                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrvArcLenSclrCrv(const CagdCrvStruct *Crv,
				    CagdRType Epsilon)
{
    CagdCrvStruct
	*DerivCrv = CagdCrvDerive(Crv),
        *DerivMagSqrCrv = SymbCrvDotProd(DerivCrv, DerivCrv),
	*DerivMagCrv = SymbCrvSqrtScalar(DerivMagSqrCrv, Epsilon),
        *ArcLenCrv = CagdCrvIntegrate(DerivMagCrv);

    CagdCrvFree(DerivCrv);
    CagdCrvFree(DerivMagSqrCrv);
    CagdCrvFree(DerivMagCrv);

    return ArcLenCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a tight approximation to the arc length of a curve.		     M
*   Estimates the arc length scalar field of Crv using SymbCrvArcLenSclrCrv  M
* and evaluate the estimate on the curve's domain boundary.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to compute a tight approximation on arc length.        M
*   Epsilon:    Accuracy control.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The approximated arc length of the given curve Crv.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvArcLen, arc length                                                M
*****************************************************************************/
CagdRType SymbCrvArcLen(const CagdCrvStruct *Crv, CagdRType Epsilon)
{
    CagdRType TMin, TMax, *Pt;
    CagdCrvStruct
	*ArcLenCrv = SymbCrvArcLenSclrCrv(Crv, Epsilon);

    CagdCrvDomain(ArcLenCrv, &TMin, &TMax);
    Pt = CagdCrvEval(ArcLenCrv, TMax);
    CagdCrvFree(ArcLenCrv);

    return CAGD_IS_RATIONAL_CRV(ArcLenCrv) ? Pt[1] / Pt[0] : Pt[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes parameter values to move steps of Length at a time on curve Crv.  M
*    Returned is a list of parameter values to move along.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to compute constant arc Length steps.                  M
*   Length:     The step size.                                               M
*   Epsilon:    Accuracy control.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  List of parameter values to march along Crv with arc    M
*                    Length between them.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvArcLenSteps, arc length                                           M
*****************************************************************************/
CagdPtStruct *SymbCrvArcLenSteps(const CagdCrvStruct *Crv,
				 CagdRType Length,
				 CagdRType Epsilon)
{
    CagdRType TMin, TMax, *Pt, CrvLength, Len;
    CagdPtStruct *Param,
	*ParamList = NULL;
    CagdCrvStruct
	*ArcLenCrv = SymbCrvArcLenSclrCrv(Crv, Epsilon);

    CagdCrvDomain(ArcLenCrv, &TMin, &TMax);
    Pt = CagdCrvEval(ArcLenCrv, TMax);

    CrvLength = CAGD_IS_RATIONAL_CRV(ArcLenCrv) ? Pt[1] / Pt[0] : Pt[1];

    for (Len = CrvLength - Length; Len > 0.0; Len -= Length) {
	Param = SymbCrvConstSet(ArcLenCrv, 1, ARCLEN_CONST_SET_IRIT_EPS,
				Len, TRUE);
	if (Param == NULL || Param -> Pnext != NULL)
	    SYMB_FATAL_ERROR(SYMB_ERR_REPARAM_NOT_MONOTONE);

	Param -> Pnext = ParamList;
	ParamList = Param;
    }

    CagdCrvFree(ArcLenCrv);

    return ParamList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an approximated arc length curve from a given curve.		     M
*   Approximation is acheived by least square fitting of points sampled      M
* along the curve that are arc length parameterized.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To approximate as an arc length curve.	                     M
*   Fineness:   Tolerance to use is sampling the original curve.	     M
*   Order:      Order of least square fit curve.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A polynomial curve similar to input curve but with    M
*		       almost arc length parametrization.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvArcLenCrv, arc length                                             M
*****************************************************************************/
CagdCrvStruct *SymbCrvArcLenCrv(const CagdCrvStruct *Crv,
				CagdRType Fineness,
				int Order)
{
    int i, j, NumSamples, NumCtlPts,
	NumDims = IRIT_MIN(CAGD_NUM_OF_PT_COORD(Crv -> PType), 3);
    CagdRType t, *Params, *KV;
    CagdPolylineStruct *Pl;
    CagdPolylnStruct *Plln;
    CagdCtlPtStruct *Pt,
	*PtList = NULL;
    CagdCrvStruct *LstSqrCrv;
    CagdPointType
	PType = CAGD_MAKE_PT_TYPE(0, NumDims);

    /* Adaptively sample points so we have more points near curved regions. */
    Pl = SymbCrv2Polyline(Crv, Fineness, SYMB_CRV_APPROX_TOLERANCE, TRUE);
    NumSamples = Pl -> Length;
    NumCtlPts = IRIT_MAX(NumSamples / 10, 2);
    Order = IRIT_MIN(Order, NumCtlPts);
    Plln = Pl -> Polyline;

    for (i = 0; i < NumSamples; i++) {
	Pt = CagdCtlPtNew(PType);
	for (j = 0; j < NumDims; j++)
	    Pt -> Coords[j + 1] = Plln[i].Pt[j];

	IRIT_LIST_PUSH(Pt, PtList);
    }
    PtList = CagdListReverse(PtList);
    CagdPolylineFree(Pl);

    /* Compute the parameters. */
    Params = (CagdRType *) IritMalloc(sizeof(CagdRType) * NumSamples);
    Params[0] = 0.0;
    for (i = 1, Pt = PtList; i < NumSamples; i++, Pt = Pt -> Pnext) {
	CagdRType
	    *R1 = Pt -> Coords,
	    *R2 = Pt -> Pnext -> Coords;
	CagdPType Pt1, Pt2;
        CagdVType Dst;

	CagdCoerceToE3(Pt1, &R1, -1, Pt -> PtType);
	CagdCoerceToE3(Pt2, &R2, -1, Pt -> Pnext -> PtType);
	IRIT_PT_SUB(Dst, Pt2, Pt1);

	Params[i] = Params[i - 1] + sqrt(IRIT_DOT_PROD(Dst, Dst));
    }

    /* Constructs a knot sequence. */
    KV = (CagdRType *) IritMalloc(sizeof(CagdRType) * (NumCtlPts + Order));
    for (i = 0; i < Order; i++)
	KV[i] = Params[0];
    t = (NumSamples - IRIT_EPS) / (NumCtlPts - Order + 1.0);
    for (i = Order; i < NumCtlPts; i++) {
        int r = (int) ((i - Order + 1) * t);

	KV[i] = Params[IRIT_BOUND(r, 0, NumSamples - 1)];
    }
    for (i = NumCtlPts; i < NumCtlPts + Order; i++)
	KV[i] = Params[NumSamples - 1];

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugLstSqrArclen, FALSE) {
	    IRIT_INFO_MSG("\n\nSample points' parameters:\n");
	    for (i = 0; i < NumSamples; i++)
	        IRIT_INFO_MSG_PRINTF("%f  ", Params[i]);

	    IRIT_INFO_MSG("\n\nKnot Sequence:\n");
	    for (i = 0; i < NumCtlPts + Order; i++)
	        IRIT_INFO_MSG_PRINTF("%f  ", KV[i]);
	}
    }
#endif /* DEBUG */

    LstSqrCrv = BspCrvInterpolate(PtList, Params, KV,
				  NumCtlPts, Order, Crv -> Periodic);

    IritFree(KV);
    IritFree(Params);
    CagdCtlPtFreeList(PtList);

    return LstSqrCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Normalizes all the control points of the given (vector field) curve.       M
* This results in an approximated unit speed vector field.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:     Curve to approximate a unit magnitude for.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Similar curve of Crv, but with unit length             M
*                     control points.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvUnitLenScalar			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvUnitLenCtlPts, unit vector field                                  M
*****************************************************************************/
CagdCrvStruct *SymbCrvUnitLenCtlPts(const CagdCrvStruct *CCrv)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(CCrv);
    int i, j,
	Dim = CAGD_NUM_OF_PT_COORD(CCrv -> PType),
	Len = CCrv -> Length;
    CagdRType t;
    CagdCrvStruct
	*Crv = CagdCrvCopy(CCrv);
    CagdRType
	* const *CtlPts = Crv -> Points;

    for (i = 0; i < Len; i++) {
	if (IsRational) {
	    for (j = 1; j <= Dim; j++)
	        CtlPts[j][i] /= CtlPts[0][i];
	}

	t = 0.0;
	for (j = 1; j <= Dim; j++)
	    t += IRIT_SQR(CtlPts[j][i]);
	t = sqrt(t);
	for (j = 1; j <= Dim; j++)
	    CtlPts[j][i] /= t;

	if (IsRational) {
	    for (j = 1; j <= Dim; j++)
	        CtlPts[j][i] *= CtlPts[0][i];
	}
    }

    return Crv;
}
