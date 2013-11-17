/******************************************************************************
* CrvtrRec.c - curvature Reconstruction computation of freeforms.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 04.					      *
******************************************************************************/

#include "symb_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a signed curvature signature scalar curve to the given planar   M
* curve.  The returned signature is parameterized by the curve's arc-length  M
* even if the original curve is not arc length, if ArcLen is TRUE.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      Curve to compute signed curvature signature approximation for. M
*   Samples:  Number of samples to use in the approximation.		     M
*   Order:    Order of signed curvature approximating curve.		     M
*   ArcLen:   TRUE if Crv is to be assumed arc-length, FALSE if needs to     M
*	      compensate for a non arc-length parametrization.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Scalar polynomial curve of length Samples and of      M
*             order Order that represents the signed curvature field of Crv. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSignedCrvtrGenCrv, SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvGenSignedCrvtr                                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrvGenSignedCrvtr(const CagdCrvStruct *CCrv,
				     int Samples,
				     int Order,
				     int ArcLen)
{
    int i, k;
    CagdRType t, TMin, TMax, Dt, *Pts, *KV;
    CagdCrvStruct *CrvtrCrv, *DCrv, *DDCrv, *Crv;

    if (CAGD_IS_BSPLINE_CRV(CCrv) && !BspCrvHasOpenEC(CCrv))
	Crv = BspCrvOpenEnd(CCrv);
    else
	Crv = CagdCrvCopy(CCrv);

    DCrv = CagdCrvDerive(Crv);
    DDCrv = CagdCrvDerive(DCrv);

    CagdCrvDomain(Crv, &TMin, &TMax);
    Dt = (TMax - TMin) / (Samples - 1.0) - IRIT_UEPS;

    CrvtrCrv = BspCrvNew(Samples, Order, CAGD_PT_E1_TYPE);
    CrvtrCrv -> Order = Order;

    if (ArcLen) {
        KV = CrvtrCrv -> KnotVector;
	KV[0] = 0.0;
    }
    else {
	KV = NULL;
        BspKnotUniformOpen(Samples, Order, CrvtrCrv -> KnotVector);
    }

    Pts = CrvtrCrv -> Points[1];
    for (i = 0, k = 1, t = TMin; i < Samples; i++, k++, t += Dt) {
        CagdRType *R;
	CagdPType D, DD;

	R = CagdCrvEval(DCrv, t);
	CagdCoerceToE2(D, &R, -1, DCrv -> PType);

	R = CagdCrvEval(DDCrv, t);
	CagdCoerceToE2(DD, &R, -1, DDCrv -> PType);

	*Pts++ = IRIT_CROSS_PROD_2D(D, DD) / pow(IRIT_DOT_PROD_2D(D, D), 1.5);

	if (ArcLen) {
	    CagdRType
		Step = IRIT_EPS + IRIT_PT2D_LENGTH(D) * Dt;

	    KV[k] = KV[k - 1] + Step;
	    if (i == 0) {
		while (k < (Order >> 1)) {
		    k++;
	            KV[k] = KV[k - 1] + Step;
		}
	    }
	    else if (i == Samples - 1) {
		while (k < Samples + Order - 1) {
		    k++;
	            KV[k] = KV[k - 1] + Step;
		}
	    }
	}
    }

    CagdCrvFree(Crv);
    CagdCrvFree(DCrv);
    CagdCrvFree(DDCrv);

    if (!ArcLen)
        BspKnotAffineTransOrder2(CrvtrCrv -> KnotVector, CrvtrCrv -> Order,
				 CrvtrCrv -> Order + CrvtrCrv -> Length,
				 TMin, TMax);

    return CrvtrCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reconstructs a planar curve from the given arc-length curvature          M
* signature.  The reconstruction is conducted as follows:                    M
* 1. Tetha(s) = Integral(Crvtr(s)), the angular changes as function of s.    V
* 2. T(s) = Circ(Tetha(s)), the tangent field of the reconstructed curve.    V
* 3. C(s) = Integral(T(s)), the final curve.				     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crvtr:      The signed curvature signature to reconstruct.  Assumed to   M
*	        be a piecewise polynomial arc-length function.               M
*   Tol:        Tolerance of approximations (arclen/subdivision) of curves.  M
*   Order:      Order of output, approximated curve.  At least quadratic.    M
*   Periodic:   If TRUE, teh recostructed curve is periodic so shift the     M
*		result to be closed.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Reconstructed planar curve, in the XY plane.         M
*		This result is invariant to rotations and translations.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvGenSignedCrvtr                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSignedCrvtrGenCrv                                                    M
*****************************************************************************/
CagdCrvStruct *SymbSignedCrvtrGenCrv(const CagdCrvStruct *Crvtr,
				     CagdRType Tol,
				     int Order,
				     int Periodic)
{
    int ArcLenCircOrder = IRIT_MAX(Order - 1, 1);
    CagdCrvStruct
	*Theta = CagdCrvIntegrate(Crvtr),
	*Circ = BspCrvCreateUnitCircle(),
	*ArcLenCirc = SymbCrvArcLenCrv(Circ, Tol, ArcLenCircOrder),
	*T = SymbComposePeriodicCrvCrv(ArcLenCirc, Theta, Tol),
	*C = CagdCrvIntegrate(T);

    CagdCrvFree(Theta);
    CagdCrvFree(Circ);
    CagdCrvFree(ArcLenCirc);
    CagdCrvFree(T);

    if (Periodic) {			      /* Make sure end points meet. */
        int i,
	    Len = C -> Length;
	CagdRType
	    **Pts = C -> Points;
	CagdPType P0, Pn;

	CagdCoerceToE2(P0, Pts, 0, C -> PType);
	CagdCoerceToE2(Pn, Pts, Len - 1, C -> PType);

	IRIT_PT2D_SUB(P0, P0, Pn);
	IRIT_PT2D_SCALE(P0, 1.0 / Len);

	for (i = 1; i < Len; i++) {
	    Pts[1][i] += P0[0] * i;
	    Pts[2][i] += P0[1] * i;
	}
    }

    return C;
}

