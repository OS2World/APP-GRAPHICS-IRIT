/******************************************************************************
* Curvature.c - curvature computation of curves and surfaces.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 93.					      *
******************************************************************************/

#include "symb_loc.h"

#define MAX_POS_REF_ITERATION 20

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a scalar curve representing the curvature of a planar curve.      M
*   The given curve is assumed to be planar and only its x and y coordinates M
* are considered.							     M
*   Then the curvature k is equal to					     M
*      .  ..    .  ..							     V
*      X  Y  -  Y  X							     V
* k =  -------------							     V
*         .2   .2  3/2 							     V
*      (  X  + Y  )							     V
*   Since we cannot represent k because of the square root, we compute and   M
* represent k^2.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute the square of the curvature field for.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The square of the curvature field of Crv.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv3DCurvatureSqr, SymbCrv3DRadiusNormal,			     M
*   SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign,			     M
*   SymbCrv2DInflectionPts, SymbCrvExtremCrvtrPts 			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2DCurvatureSqr, curvature                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrv2DCurvatureSqr(const CagdCrvStruct *Crv)
{
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct *Crv1W, *Crv1X, *Crv1Y, *Crv1Z, *Crv1Deriv, *Crv2Deriv,
		  *Crv2W, *Crv2X, *Crv2Y, *Crv2Z,
		  *CTmp1, *CTmp2, *CTmp3, *Numer, *Denom, *CurvatureSqr;

    if (Crv -> Order <= 2) {			       /* Make a zero curve. */
	int i;
	CagdRType *Points;

	CTmp1 = CagdCrvCopy(Crv);
	CTmp2 = CagdCoerceCrvTo(CTmp1, CAGD_PT_E1_TYPE, FALSE);
	CagdCrvFree(CTmp1);
	for (i = 0, Points = CTmp2 -> Points[1]; i < CTmp2 -> Length; i++)
	    *Points++ = 0.0;
	return CTmp2;
    }

    Crv1Deriv = CagdCrvDerive(Crv);
    Crv2Deriv = CagdCrvDerive(Crv1Deriv);

    SymbCrvSplitScalar(Crv1Deriv, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);
    SymbCrvSplitScalar(Crv2Deriv, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);
    CagdCrvFree(Crv1Deriv);
    CagdCrvFree(Crv2Deriv);

    CTmp1 = SymbCrvMult(Crv1X, Crv2Y);
    CTmp2 = SymbCrvMult(Crv2X, Crv1Y);
    CTmp3 = SymbCrvSub(CTmp1, CTmp2);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp2);
    Numer = SymbCrvMult(CTmp3, CTmp3);
    CagdCrvFree(CTmp3);

    CTmp1 = SymbCrvMult(Crv1X, Crv1X);
    CTmp2 = SymbCrvMult(Crv1Y, Crv1Y);
    CTmp3 = SymbCrvAdd(CTmp1, CTmp2);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp2);
    CTmp1 = SymbCrvMult(CTmp3, CTmp3);
    Denom = SymbCrvMult(CTmp1, CTmp3);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp3);
    if (IsRational) {
	CTmp1 = SymbCrvMult(Crv1W, Crv1W);
	CTmp2 = SymbCrvMult(CTmp1, CTmp1);
	CTmp3 = SymbCrvMult(CTmp2, Numer);
	CagdCrvFree(CTmp1);
	CagdCrvFree(CTmp2);
	CagdCrvFree(Numer);
	Numer = CTmp3;

	CTmp1 = SymbCrvMult(Crv2W, Crv2W);
	CTmp2 = SymbCrvMult(CTmp1, Denom);
	CagdCrvFree(CTmp1);
	CagdCrvFree(Denom);
	Denom = CTmp2;
    }

#ifdef SYMB_CURVATURE_POS_POS_COEFS
    if (CAGD_IS_BSPLINE_CRV(Denom)) {
	CTmp1 = SymbMakePosCrvCtlPolyPos(Denom);
	CagdCrvFree(Denom);
	Denom = CTmp1;
    }
#endif /* SYMB_CURVATURE_POS_POS_COEFS */

    CagdMakeCrvsCompatible(&Denom, &Numer, TRUE, TRUE);
    CurvatureSqr = SymbCrvMergeScalar(Denom, Numer, NULL, NULL);
    CagdCrvFree(Denom);
    CagdCrvFree(Numer);

    CagdCrvFree(Crv1X);
    CagdCrvFree(Crv1Y);
    CagdCrvFree(Crv2X);
    CagdCrvFree(Crv2Y);
    if (Crv1Z)
	CagdCrvFree(Crv1Z);
    if (Crv2Z)
	CagdCrvFree(Crv2Z);
    if (Crv1W)
	CagdCrvFree(Crv1W);
    if (Crv2W)
	CagdCrvFree(Crv2W);

    return CurvatureSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a scalar field curve representing the square of the curvature     M
* of a given 3D curve.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute scalar field of curvatrue square for.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Computed scalar field of curvature square of Crv.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DRadiusNormal,			     M
*   SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign,			     M
*   SymbCrv2DInflectionPts, SymbCrvExtremCrvtrPts 			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv3DCurvatureSqr, curvature                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrv3DCurvatureSqr(const CagdCrvStruct *Crv)
{
    CagdCrvStruct
	*CTmp, *CTmp2, *Numer, *Denom, *CurvatureSqr, *Crv1Deriv, *Crv2Deriv;

    if (Crv -> Order <= 2) {			       /* Make a zero curve. */
	int i;
	CagdRType *Points;

	CTmp = CagdCrvCopy(Crv);
	CTmp2 = CagdCoerceCrvTo(CTmp, CAGD_PT_E1_TYPE, FALSE);
	CagdCrvFree(CTmp);
	for (i = 0, Points = CTmp2 -> Points[1]; i < CTmp2 -> Length; i++)
	    *Points++ = 0.0;
	return CTmp2;
    }

    Crv1Deriv = CagdCrvDerive(Crv);
    Crv2Deriv = CagdCrvDerive(Crv1Deriv);

    CTmp = SymbCrvCrossProd(Crv1Deriv, Crv2Deriv);
    CagdCrvFree(Crv2Deriv);
    Numer = SymbCrvDotProd(CTmp, CTmp);
    CagdCrvFree(CTmp);

    CTmp = SymbCrvDotProd(Crv1Deriv, Crv1Deriv);
    CagdCrvFree(Crv1Deriv);
    CTmp2 = SymbCrvMult(CTmp, CTmp);
    Denom = SymbCrvMult(CTmp, CTmp2);
    CagdCrvFree(CTmp);
    CagdCrvFree(CTmp2);

    if (CAGD_IS_RATIONAL_CRV(Denom) || CAGD_IS_RATIONAL_CRV(Numer)) {
	CTmp = SymbCrvInvert(Denom);
	CurvatureSqr = SymbCrvMult(CTmp, Numer);
	CagdCrvFree(CTmp);
    }
    else {
	CagdCrvStruct *PCrvW, *PCrvX, *PCrvY, *PCrvZ;

	SymbCrvSplitScalar(Numer, &PCrvW, &PCrvX, &PCrvY, &PCrvZ);
	CagdMakeCrvsCompatible(&Denom, &PCrvX, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &PCrvY, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &PCrvZ, TRUE, TRUE);
	CurvatureSqr = SymbCrvMergeScalar(Denom, PCrvX, PCrvY, PCrvZ);
	CagdCrvFree(PCrvX);
	CagdCrvFree(PCrvY);
	CagdCrvFree(PCrvZ);
    }

    CagdCrvFree(Denom);
    CagdCrvFree(Numer);

    return CurvatureSqr;
}

/******************************************************************************
* DESCRIPTION:                                                               M
* Computes a vector field curve representing the radius (1/curvature) of a   M
* curve, in the normal direction, that is N / k:			     M
*									     M
*                   .   ..    .	     .  6        .   ..    .     .  2	     V
*          k N     (C x C ) x C	   | C |      ( (C x C ) x C ) | C |	     V
* N / k =  ----- = ------------ . --------- = -----------------------	     V
*            2          .  4       .   .. 2           .   .. 2		     V
*           k         | C |       (C x C )           (C x C )		     V
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute the normal field with radius as magnitude.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Computed normal field with 1 / k as magnitude.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DCurvatureSqr, SymbCrv3DCurvatureSqr,     M
*   SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign,			     M
*   SymbCrv2DInflectionPts, SymbCrvExtremCrvtrPts, SymbCrv2DUnnormNormal     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv3DRadiusNormal, curvature                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrv3DRadiusNormal(const CagdCrvStruct *Crv)
{
    CagdCrvStruct
	*PCrvW, *PCrvX, *PCrvY, *PCrvZ, *Crv1Deriv, *Crv2Deriv,
	*CTmp, *CTmp2, *Numer, *Denom, *RadiusNormal;

    if (Crv -> Order <= 2) {			       /* Make a zero curve. */
	int i;
	CagdRType *Points;

	CTmp = CagdCrvCopy(Crv);
	CTmp2 = CagdCoerceCrvTo(CTmp, CAGD_PT_E1_TYPE, FALSE);
	CagdCrvFree(CTmp);
	for (i = 0, Points = CTmp2 -> Points[1]; i < CTmp2 -> Length; i++)
	    *Points++ = 0.0;
	return CTmp2;
    }

    Crv1Deriv = CagdCrvDerive(Crv);
    Crv2Deriv = CagdCrvDerive(Crv1Deriv);

    CTmp = SymbCrvCrossProd(Crv1Deriv, Crv2Deriv);
    CagdCrvFree(Crv2Deriv);
    Denom = SymbCrvDotProd(CTmp, CTmp);
    CTmp2 = SymbCrvCrossProd(CTmp, Crv1Deriv);
    CagdCrvFree(CTmp);
    CTmp = SymbCrvDotProd(Crv1Deriv, Crv1Deriv);
    CagdCrvFree(Crv1Deriv);
    Numer = SymbCrvMultScalar(CTmp2, CTmp);
    CagdCrvFree(CTmp2);
    CagdCrvFree(CTmp);

    if (CAGD_IS_RATIONAL_CRV(Denom) || CAGD_IS_RATIONAL_CRV(Numer)) {
	CTmp = SymbCrvInvert(Denom);
	RadiusNormal = SymbCrvMult(CTmp, Numer);
	CagdCrvFree(CTmp);
    }
    else {
	SymbCrvSplitScalar(Numer, &PCrvW, &PCrvX, &PCrvY, &PCrvZ);
	CagdMakeCrvsCompatible(&Denom, &PCrvX, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &PCrvY, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &PCrvZ, TRUE, TRUE);
	RadiusNormal = SymbCrvMergeScalar(Denom, PCrvX, PCrvY, PCrvZ);
	CagdCrvFree(PCrvX);
	CagdCrvFree(PCrvY);
	CagdCrvFree(PCrvZ);
    }

    CagdCrvFree(Denom);
    CagdCrvFree(Numer);

    return RadiusNormal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the unnormalized normal of a planar 2D curve as a 90 rotation   M
* in the plane of the tangent field.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Planar curve to compute unnormalized normal field for.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The normal field.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv3DRadiusNormal                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2DUnnormNormal                                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrv2DUnnormNormal(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *TCrv, *NCrv;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2) {
        SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D);
	return NULL;
    }

    TCrv = CagdCrvDerive(Crv);
    SymbCrvSplitScalar(TCrv, &CrvW, &CrvX, &CrvY, &CrvZ);
    CagdCrvFree(TCrv);

    CagdCrvTransform(CrvX, NULL, -1.0);

    NCrv = SymbCrvMergeScalar(CrvW, CrvY, CrvX, NULL);

    if (CrvW != NULL)
        CagdCrvFree(CrvW);
    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    if (CrvZ != NULL)
        CagdCrvFree(CrvZ);

    return NCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a vector field curve representing the curvature of a curve, in    M
* the normal direction, that is kN.					     M
*                .   ..      .       .   ..     .			     V
*                C x C       C     ( C x C  ) x C     			     V
* kN = kB x T =  -----  x  ----- = --------------			     V
*                  .  3      .           .  4				     V
*                | C |     | C |       | C |				     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute the normal curvature field.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Computed normal curvature field.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DCurvatureSqr, SymbCrv3DCurvatureSqr,     M
*   SymbCrv3DRadiusNormal, SymbCrv2DCurvatureSign, SymbCrv2DInflectionPts,   M
*   SymbCrvExtremCrvtrPts						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv3DCurvatureNormal, curvature                                      M
*****************************************************************************/
CagdCrvStruct *SymbCrv3DCurvatureNormal(const CagdCrvStruct *Crv)
{
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *Crv1Deriv, *Crv2Deriv,
	*CTmp, *CTmp2, *Numer, *Denom, *CurvatureNormal;

    if (Crv -> Order <= 2) {			       /* Make a zero curve. */
	int i;
	CagdRType *Points;

	CTmp = CagdCrvCopy(Crv);
	CTmp2 = CagdCoerceCrvTo(CTmp, CAGD_PT_E1_TYPE, FALSE);
	CagdCrvFree(CTmp);
	for (i = 0, Points = CTmp2 -> Points[1]; i < CTmp2 -> Length; i++)
	    *Points++ = 0.0;
	return CTmp2;
    }

    Crv1Deriv = CagdCrvDerive(Crv);
    Crv2Deriv = CagdCrvDerive(Crv1Deriv);

    CTmp = SymbCrvCrossProd(Crv1Deriv, Crv2Deriv);
    CagdCrvFree(Crv2Deriv);
    Numer = SymbCrvCrossProd(CTmp, Crv1Deriv);
    CagdCrvFree(CTmp);
    SymbCrvSplitScalar(Numer, &CrvW, &CrvX, &CrvY, &CrvZ);
    CagdCrvFree(Numer);

    CTmp = SymbCrvDotProd(Crv1Deriv, Crv1Deriv);
    CagdCrvFree(Crv1Deriv);
    Denom = SymbCrvMult(CTmp, CTmp);
    CagdCrvFree(CTmp);

    if (IsRational) {
	CagdCrvStruct *DenomCrvW, *DenomCrvX, *DenomCrvY, *DenomCrvZ;

	SymbCrvSplitScalar(Denom,
			   &DenomCrvW, &DenomCrvX, &DenomCrvY, &DenomCrvZ);
	CagdCrvFree(Denom);

	CTmp = SymbCrvMult(CrvW, DenomCrvX);
	CagdCrvFree(CrvW);
	CrvW = CTmp;

	CTmp = SymbCrvMult(CrvX, DenomCrvW);
	CagdCrvFree(CrvX);
	CrvX = CTmp;

	CTmp = SymbCrvMult(CrvY, DenomCrvW);
	CagdCrvFree(CrvY);
	CrvY = CTmp;

	CTmp = SymbCrvMult(CrvZ, DenomCrvW);
	CagdCrvFree(CrvZ);
	CrvZ = CTmp;

	CagdCrvFree(DenomCrvW);
	CagdCrvFree(DenomCrvX);
    }
    else {
	CagdMakeCrvsCompatible(&Denom, &CrvX, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &CrvY, TRUE, TRUE);
	CagdMakeCrvsCompatible(&Denom, &CrvZ, TRUE, TRUE);
	CrvW = Denom;
    }
    CurvatureNormal = SymbCrvMergeScalar(CrvW, CrvX, CrvY, CrvZ);
    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    CagdCrvFree(CrvZ);
    CagdCrvFree(CrvW);

    return CurvatureNormal;
}

/******************************************************************************
* DESCRIPTION:                                                               M
* Computes a scalar curve representing the curvature sign of a planar curve. M
*   The given curve is assumed to be planar and only its x and y coordinates M
* are considered.							     M
*   Then the curvature sign is equal to					     M
*      .  ..    .  ..							     V
* s =  X  Y  -  Y  X							     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute the curvature sign field.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Computed curvature sign field.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DCurvatureSqr, SymbCrv3DCurvatureSqr,     M
*   SymbCrv3DRadiusNormal, SymbCrv3DCurvatureNormal, SymbCrv2DInflectionPts, M
*   SymbCrvExtremCrvtrPts						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2DCurvatureSign, curvature                                        M
*****************************************************************************/
CagdCrvStruct *SymbCrv2DCurvatureSign(const CagdCrvStruct *Crv)
{
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct *Crv1W, *Crv1X, *Crv1Y, *Crv1Z, *Crv1Deriv, *Crv2Deriv,
		  *Crv2W, *Crv2X, *Crv2Y, *Crv2Z,
		  *CTmp1, *CTmp2, *Numer, *Denom, *CurvatureSign;

    if (Crv -> Order <= 2) {			       /* Make a zero curve. */
	int i;
	CagdRType *Points;

	CTmp1 = CagdCrvCopy(Crv);
	CTmp2 = CagdCoerceCrvTo(CTmp1, CAGD_PT_E1_TYPE, FALSE);
	CagdCrvFree(CTmp1);
	for (i = 0, Points = CTmp2 -> Points[1]; i < CTmp2 -> Length; i++)
	    *Points++ = 0.0;
	return CTmp2;
    }

    Crv1Deriv = CagdCrvDerive(Crv);
    Crv2Deriv = CagdCrvDerive(Crv1Deriv);

    SymbCrvSplitScalar(Crv1Deriv, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);
    SymbCrvSplitScalar(Crv2Deriv, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);
    CagdCrvFree(Crv1Deriv);
    CagdCrvFree(Crv2Deriv);

    CTmp1 = SymbCrvMult(Crv1X, Crv2Y);
    CTmp2 = SymbCrvMult(Crv2X, Crv1Y);
    Numer = SymbCrvSub(CTmp1, CTmp2);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp2);

    if (IsRational) {
	Denom = SymbCrvMult(Crv1W, Crv2W);

	CagdMakeCrvsCompatible(&Denom, &Numer, TRUE, TRUE);
	CurvatureSign = SymbCrvMergeScalar(Denom, Numer, NULL, NULL);
	CagdCrvFree(Denom);
	CagdCrvFree(Numer);
    }
    else {
	CurvatureSign = Numer;
    }

    CagdCrvFree(Crv1X);
    CagdCrvFree(Crv1Y);
    CagdCrvFree(Crv2X);
    CagdCrvFree(Crv2Y);
    if (Crv1Z)
	CagdCrvFree(Crv1Z);
    if (Crv2Z)
	CagdCrvFree(Crv2Z);
    if (Crv1W)
	CagdCrvFree(Crv1W);
    if (Crv2W)
	CagdCrvFree(Crv2W);

    return CurvatureSign;
}

/******************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar curve that is positive, refine it until all its control     M
* points has positive coefficients. Always returns a Bspline curve.          M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:    To refine until all its control points are non negative.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Refined positive curve with positive control points. M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbMakePosCrvCtlPolyPos, refinement                                     M
*****************************************************************************/
CagdCrvStruct *SymbMakePosCrvCtlPolyPos(const CagdCrvStruct *OrigCrv)
{
    int l;
    CagdCrvStruct
	*RefCrv = NULL;

    switch (OrigCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    RefCrv = CagdCnvrtBzr2BspCrv(OrigCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    RefCrv = CagdCrvCopy(OrigCrv);
	    break;
	case CAGD_CPOWER_TYPE:
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    break;
    }

    for (l = 0; l < MAX_POS_REF_ITERATION; l++) {
	int i, j,
	    Len = RefCrv -> Length,
	    Order = RefCrv -> Order;
	CagdRType
	    *KV = RefCrv -> KnotVector,
	    *Nodes = BspKnotNodes(KV, Len + Order, Order),
	    *Pts = RefCrv -> Points[1];

	for (i = j = 0; i < Len; i++) {
	    if (IRIT_FABS(Pts[i]) < IRIT_SQR(IRIT_EPS))
		Pts[i] = 0.0;
	    if (Pts[i] < 0)	    /* To refine at negative control points. */
	        Nodes[j++] = Nodes[i];
	}
	if (j == 0) {
	    IritFree(Nodes);
	    break;
	}
	else {
	    CagdCrvStruct
	        *NewCrv = CagdCrvRefineAtParams(RefCrv, FALSE, Nodes, j);

	    CagdCrvFree(RefCrv);
	    RefCrv = NewCrv;
	    IritFree(Nodes);
	}
    }

    return RefCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a planar curve, finds all its inflection points by finding the zero  M
* set of the sign of the curvature function of the curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To find all its inflection points.                           M
*   Epsilon:    Accuracy control.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:    A list of parameter values on Crv that are inflection M
*                      points.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DCurvatureSqr, SymbCrv3DCurvatureSqr,     M
*   SymbCrv3DRadiusNormal, SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign, M
*   SymbCrvExtremCrvtrPts 						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2DInflectionPts, curvature, inflection points                     M
*****************************************************************************/
CagdPtStruct *SymbCrv2DInflectionPts(const CagdCrvStruct *Crv,
				     CagdRType Epsilon)
{
    int i, n;
    CagdCrvStruct
	*CrvtrSign = SymbCrv2DCurvatureSign(Crv);
    CagdPtStruct
	*InflectionPts = SymbCrvZeroSet(CrvtrSign, 1, Epsilon, FALSE);
    CagdRType
        *C0Disconts = CAGD_IS_BSPLINE_CRV(CrvtrSign) ?
		      BspKnotAllC0Discont(CrvtrSign -> KnotVector,
					  CrvtrSign -> Order,
					  CrvtrSign -> Length, &n) :
		      NULL;

    if (C0Disconts != NULL) {
        /* We also need to examine C0 discontinuities in curvature field if */
        /* two sides of that C0 discontinuity has different curvature signs.*/
        for (i = 0; i < n; i++) {
	    CagdRType *R, R1, R2, TMin, TMax;

	    CagdCrvDomain(CrvtrSign, &TMin, &TMax);
	    if (C0Disconts[i] - IRIT_EPS > TMin &&
		C0Disconts[i] + IRIT_EPS < TMax) {
	        /* Interior to the domain. */
	        R = CagdCrvEval(CrvtrSign, C0Disconts[i] - IRIT_EPS);
		R1 = CAGD_IS_RATIONAL_CRV(CrvtrSign) ? R[1] / R[0] : R[1];
		R = CagdCrvEval(CrvtrSign, C0Disconts[i] + IRIT_EPS);
		R2 = CAGD_IS_RATIONAL_CRV(CrvtrSign) ? R[1] / R[0] : R[1];

		if (R1 * R2 < 0.0) {  /* Found an inflection point at knot. */
		    InflectionPts = SymbInsertNewParam2(InflectionPts,
							C0Disconts[i]);
		}
	    }
	}

	IritFree(C0Disconts);
    }

    CagdCrvFree(CrvtrSign);

    return InflectionPts;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a planar curve, finds all its extreme curvature points by finding    M
* the set of extreme locations on the curvature function of Crv.	     M
*  Extreme curvature is computed as the zeros of <(kN)', kN> = k'k.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To find all int extrem curvature locations.                   M
*   Epsilon:   Accuracy control.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter values on Crv that have extrem     M
*                     curvature values.  An int attribute named "ExtremType" M
*		      is placed on each parameter value with a value of -1,  M
*		      0, or 1 for minimum curvature location, zero curvature M
*		      location and maximum curvature location, respectively. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSqr, SymbCrv3DCurvatureSqr, SymbCrv3DCurvatureSqr,     M
*   SymbCrv3DRadiusNormal, SymbCrv3DCurvatureNormal, SymbCrv2DCurvatureSign, M
*   SymbCrv2DInflectionPts						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvExtremCrvtrPts, curvature                                         M
*****************************************************************************/
CagdPtStruct *SymbCrvExtremCrvtrPts(const CagdCrvStruct *Crv,
				    CagdRType Epsilon)
{
    int OldProdMethod = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);/* Fastest. */
    CagdRType TMin, TMax, TMid;
    CagdCrvStruct
	*CrvtrNormalCrv = SymbCrv3DCurvatureNormal(Crv),
	*CrvtrMag = SymbCrvDotProd(CrvtrNormalCrv, CrvtrNormalCrv),
	*DCrvtrMag = CagdCrvDerive(CrvtrMag);
    CagdPtStruct *Pt,
        *ExtremCrvtrPts = SymbCrvZeroSet(DCrvtrMag, 1, Epsilon, FALSE);

    BspMultComputationMethod(OldProdMethod);

    CagdCrvDomain(Crv, &TMin, &TMax);
    TMid = (TMin + TMax) * 0.5;
    for (Pt = ExtremCrvtrPts; Pt != NULL; Pt = Pt -> Pnext) {
        CagdRType *R, V, V1, V2,
	    t = Pt -> Pt[0];

	R = CagdCrvEval(CrvtrMag, t);
	V = R[1] / R[0];
	if (IRIT_APX_EQ_EPS(V, 0.0, Epsilon * 100)) {
	    AttrSetIntAttrib(&Pt -> Attr, "ExtremType", 0);
	}
	else {
	    R = CagdCrvEval(DCrvtrMag, t > TMin + IRIT_EPS ? t - IRIT_EPS : t);
	    V1 = R[1] / R[0];
	    R = CagdCrvEval(DCrvtrMag, t < TMax - IRIT_EPS ? t + IRIT_EPS : t);
	    V2 = R[1] / R[0];

	    AttrSetIntAttrib(&Pt -> Attr, "ExtremType", V1 > V2 ? 1 : -1);
	}
    }

    CagdCrvFree(CrvtrNormalCrv);
    CagdCrvFree(CrvtrMag);
    CagdCrvFree(DCrvtrMag);

    return ExtremCrvtrPts;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes coefficients of the first fundamental form of given surface Srf.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Do compute the coefficients of the FFF for.                 M
*   DuSrf:       First derivative of Srf with respect to U goes to here.     M
*   DvSrf:       First derivative of Srf with respect to V goes to here.     M
*   FffG11:      FFF G11 scalar field.                                       M
*   FffG12:      FFF G12 scalar field.                                       M
*   FffG22:      FFF G22 scalar field.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSff, SymbSrfTff, SymbSrfDeterminant2, SymbSrfGaussCurvature,	     M
*   SymbSrfMeanEvolute, SymbSrfMeanCurvatureSqr, SymbSrfIsoFocalSrf,	     M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfFff, first fundamental form                                       M
*****************************************************************************/
void SymbSrfFff(const CagdSrfStruct *Srf,
		CagdSrfStruct **DuSrf,
		CagdSrfStruct **DvSrf,
		CagdSrfStruct **FffG11,
		CagdSrfStruct **FffG12,
		CagdSrfStruct **FffG22)
{
    *DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    *DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

    *FffG11 = SymbSrfDotProd(*DuSrf, *DuSrf);
    *FffG12 = SymbSrfDotProd(*DuSrf, *DvSrf);
    *FffG22 = SymbSrfDotProd(*DvSrf, *DvSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes coefficients of the second fundamental form of given surface Srf  M
* that is prescribed via its two partial derivatives DuSrf and DvSrf.	     M
*   These coefficients are using non normalized normal that is also returned.M
*                                                                            *
* PARAMETERS:                                                                M
*   DuSrf:       First derivative of Srf with respect to U.		     M
*   DvSrf:       First derivative of Srf with respect to V.		     M
*   SffL11:      SFF L11 scalar field returned herein.                       M
*   SffL12:      SFF L12 scalar field returned herein.                       M
*   SffL22:      SFF L22 scalar field returned herein.                       M
*   SNormal:     Unnormalized normal vector field returned herein.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfTff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanEvolute, SymbSrfMeanCurvatureSqr, SymbSrfIsoFocalSrf,	     M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSff, second fundamental form                                      M
*****************************************************************************/
void SymbSrfSff(const CagdSrfStruct *DuSrf,
		const CagdSrfStruct *DvSrf,
		CagdSrfStruct **SffL11,
		CagdSrfStruct **SffL12,
		CagdSrfStruct **SffL22,
		CagdSrfStruct **SNormal)
{
    CagdSrfStruct
	*DuuSrf = CagdSrfDerive(DuSrf, CAGD_CONST_U_DIR),
	*DuvSrf = CagdSrfDerive(DuSrf, CAGD_CONST_V_DIR),
	*DvvSrf = CagdSrfDerive(DvSrf, CAGD_CONST_V_DIR);

    *SNormal = SymbSrfCrossProd(DvSrf, DuSrf);
    *SffL11 = SymbSrfDotProd(DuuSrf, *SNormal);
    *SffL12 = SymbSrfDotProd(DuvSrf, *SNormal);
    *SffL22 = SymbSrfDotProd(DvvSrf, *SNormal);

    CagdSrfFree(DuuSrf);
    CagdSrfFree(DuvSrf);
    CagdSrfFree(DvvSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes coefficients of the third fundamental form of given surface Srf.  M
*   These coefficients are using non normalized normal that is also returned.M
* The coefficients of the TFF equal:					     M
*									     M
*                        d m  d m                   d m        d m           V
*                      < ---, --- > < m, m > - < m, --- > < m, --- >         V
*         d n  d n       dui  duj                   dui        duj           V
* Lij = < --- ,--- > = ---------------------------------------------         V
*         dui  duj                    < m, m > ^ 2                           V
*									     M
* where n is the unit normal of Srf and m = dSrf/dui x dSrf/duj, the         M
* unnormalized normal field of Srf.				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Surface to compute the coefficents of the TFF for.	     M
*   TffL11:      TFF L11 scalar field returned herein.                       M
*   TffL12:      TFF L12 scalar field returned herein.                       M
*   TffL22:      TFF L22 scalar field returned herein.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2,			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfTff, third fundamental form                                       M
*****************************************************************************/
void SymbSrfTff(const CagdSrfStruct *Srf,
		CagdSrfStruct **TffL11,
		CagdSrfStruct **TffL12,
		CagdSrfStruct **TffL22)
{
    CagdSrfStruct *TSrf1, *TSrf2, *TSrf3,
	*MSrf = SymbSrfNormalSrf(Srf),
	*DuMSrf = CagdSrfDerive(MSrf, CAGD_CONST_U_DIR),
	*DvMSrf = CagdSrfDerive(MSrf, CAGD_CONST_V_DIR),
        *M2Srf = SymbSrfDotProd(MSrf, MSrf),
	*M4Srf = SymbSrfMult(M2Srf, M2Srf),
        *MDuMSrf = SymbSrfDotProd(MSrf, DuMSrf),
        *MDvMSrf = SymbSrfDotProd(MSrf, DvMSrf);

    /* L11 */
    TSrf1 = SymbSrfDotProd(DuMSrf, DuMSrf);
    TSrf2 = SymbSrfMult(TSrf1, M2Srf);
    CagdSrfFree(TSrf1);
    TSrf1 = SymbSrfMult(MDuMSrf, MDuMSrf);
    TSrf3 = SymbSrfSub(TSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdMakeSrfsCompatible(&M4Srf, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    *TffL11 = SymbSrfMergeScalar(M4Srf, TSrf3, NULL, NULL);
    CagdSrfFree(TSrf3);

    /* L12 */
    TSrf1 = SymbSrfDotProd(DuMSrf, DvMSrf);
    TSrf2 = SymbSrfMult(TSrf1, M2Srf);
    CagdSrfFree(TSrf1);
    TSrf1 = SymbSrfMult(MDuMSrf, MDvMSrf);
    TSrf3 = SymbSrfSub(TSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdMakeSrfsCompatible(&M4Srf, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    *TffL12 = SymbSrfMergeScalar(M4Srf, TSrf3, NULL, NULL);
    CagdSrfFree(TSrf3);

    /* L22 */
    TSrf1 = SymbSrfDotProd(DvMSrf, DvMSrf);
    TSrf2 = SymbSrfMult(TSrf1, M2Srf);
    CagdSrfFree(TSrf1);
    TSrf1 = SymbSrfMult(MDvMSrf, MDvMSrf);
    TSrf3 = SymbSrfSub(TSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdMakeSrfsCompatible(&M4Srf, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    *TffL22 = SymbSrfMergeScalar(M4Srf, TSrf3, NULL, NULL);
    CagdSrfFree(TSrf3);

    CagdSrfFree(MSrf);
    CagdSrfFree(DuMSrf);
    CagdSrfFree(DvMSrf);
    CagdSrfFree(M2Srf);
    CagdSrfFree(M4Srf);
    CagdSrfFree(MDuMSrf);
    CagdSrfFree(MDvMSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the expression of Srf11 * Srf22 - Srf12 * Srf21, which is a	     M
* determinant of a 2 by 2 matrix.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf11, Srf12, Srf21, Srf22:  The four factors of the determinant.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: A scalar field representing the determinant computation.M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfGaussCurvature, SymbSrfMeanEvolute,	     M
*   SymbSrfMeanCurvatureSqr, SymbSrfIsoFocalSrf, SymbSrfCurvatureUpperBound, M
*   SymbSrfIsoDirNormalCurvatureBound, SymbSrfDeterminant3,		     M
*   SymbCrvDeterminant2							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDeterminant2, determinant                                         M
*****************************************************************************/
CagdSrfStruct *SymbSrfDeterminant2(const CagdSrfStruct *Srf11,
				   const CagdSrfStruct *Srf12,
				   const CagdSrfStruct *Srf21,
				   const CagdSrfStruct *Srf22)
{
    CagdSrfStruct
	*Prod1 = SymbSrfMult(Srf11, Srf22),
	*Prod2 = SymbSrfMult(Srf21, Srf12),
	*Add12 = SymbSrfSub(Prod1, Prod2);

    CagdSrfFree(Prod1);
    CagdSrfFree(Prod2);
    return Add12;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the Gaussian curvature of a given surface.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to compute Gaussian curvature for.                    M
*   NumerOnly: If TRUE, only the numerator component of K is returned.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surface representing the Gaussian curvature field.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfMeanEvolute,	     M
*   SymbSrfMeanCurvatureSqr, SymbSrfIsoFocalSrf, SymbSrfCurvatureUpperBound, M
*   SymbSrfIsoDirNormalCurvatureBound					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfGaussCurvature, curvature                                         M
*****************************************************************************/
CagdSrfStruct *SymbSrfGaussCurvature(const CagdSrfStruct *Srf,
				     CagdBType NumerOnly)
{
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*Numer, *Denom, *FffDeterminant, *SffDeterminant, *Gauss,
	*SrfX, *SrfY, *SrfZ, *SrfW, *SNormalSize,
	*FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22;

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    if (NumerOnly)
	FffDeterminant = NULL;
    else
	FffDeterminant = SymbSrfDeterminant2(FffG11, FffG12, FffG12, FffG22);
    SffDeterminant = SymbSrfDeterminant2(SffL11, SffL12, SffL12, SffL22);

    CagdSrfFree(FffG11);
    CagdSrfFree(FffG12);
    CagdSrfFree(FffG22);
    CagdSrfFree(SffL11);
    CagdSrfFree(SffL12);
    CagdSrfFree(SffL22);

    /* Normalize the Fff with respect to the Sff. */
    if (!NumerOnly) {
        SNormalSize = SymbSrfDotProd(SNormal, SNormal);
	STmp1 = SymbSrfMult(SNormalSize, FffDeterminant);
	CagdSrfFree(FffDeterminant);
	CagdSrfFree(SNormalSize);
	FffDeterminant = STmp1;
    }
    CagdSrfFree(SNormal);

    if (!NumerOnly && CAGD_IS_RATIONAL_SRF(FffDeterminant)) {
	SymbSrfSplitScalar(FffDeterminant, &SrfW, &SrfX, &SrfY, &SrfZ);
	SymbSrfSplitScalar(SffDeterminant, &STmp1, &STmp2, &STmp3, &STmp4);
	Numer = SymbSrfMult(STmp2, SrfW);
	Denom = SymbSrfMult(STmp1, SrfX);

	CagdSrfFree(FffDeterminant);
	CagdSrfFree(SffDeterminant);

	CagdSrfFree(STmp1);
	CagdSrfFree(STmp2);
	CagdSrfFree(SrfW);
	CagdSrfFree(SrfX);
    }
    else {
	Denom = FffDeterminant;
	Numer = SffDeterminant;
    }

    if (Denom == NULL)
	Gauss = Numer;
    else {
	CagdMakeSrfsCompatible(&Denom, &Numer, TRUE, TRUE, TRUE, TRUE);
	Gauss = SymbSrfMergeScalar(Denom, Numer, NULL, NULL);
	CagdSrfFree(Denom);
	CagdSrfFree(Numer);
    }

    return NumerOnly ? CagdSrfUnitMaxCoef(Gauss) : Gauss;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the numerator expression of the Mean as:			     M
*									     M
* H(u, v) = G11 L22 + G22 L11 - 2 G12 L12				     V
*									     M
* PARAMETERS:                                                                M
*   Srf:       Surface to compute mean evolute.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surface representing the mean evolute surface.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanCurvatureSqr, SymbSrfMeanEvolute, SymbSrfIsoFocalSrf,         M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMeanNumer, curvature, evolute                                     M
*****************************************************************************/
CagdSrfStruct *SymbSrfMeanNumer(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*Numer, *FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22;
    
    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    STmp1 = SymbSrfMult(FffG11, SffL22);
    STmp2 = SymbSrfMult(FffG22, SffL11);
    STmp3 = SymbSrfMult(FffG12, SffL12);
    STmp4 = SymbSrfScalarScale(STmp3, 2.0);
    CagdSrfFree(STmp3);
    STmp3 = SymbSrfAdd(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);
    Numer = SymbSrfSub(STmp3, STmp4);
    CagdSrfFree(STmp3);
    CagdSrfFree(STmp4);

    CagdSrfFree(FffG11);
    CagdSrfFree(FffG12);
    CagdSrfFree(FffG22);
    CagdSrfFree(SffL11);
    CagdSrfFree(SffL12);
    CagdSrfFree(SffL22);
    CagdSrfFree(SNormal);

    return CagdSrfUnitMaxCoef(Numer);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an "evolute surface" to a given surface using twice the Mean      M
* curvature as magnitude.		 				     M
*									     M
*            	        1                             |G|		     V
* E(u, v) = n(u, v) --------- = n(u, v) ---------------------------------    V
*	            2 H(u, v)           ( G11 L22 + G22 L11 - 2 G12 L12 )    V
*									     M
* Becuase H(u,v) also has n(u,v) we can use the nonnormalized surface normal M
* to compute E(u, v), which is therefore computable and representable.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to compute mean evolute.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surface representing the mean evolute surface.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanCurvatureSqr, SymbSrfMeanNumer, SymbSrfIsoFocalSrf,           M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMeanEvolute, curvature, evolute                                   M
*****************************************************************************/
CagdSrfStruct *SymbSrfMeanEvolute(const CagdSrfStruct *Srf)
{
    int i;
    CagdRType **Points, *PtX, *PtY, *PtZ;
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*Numer, *FffDeterminant, *MeanEvolute, *SrfX, *SrfY, *SrfZ, *SrfW,
	*Denom, *FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22;

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    STmp1 = SymbSrfMult(FffG11, SffL22);
    STmp2 = SymbSrfMult(FffG22, SffL11);
    STmp3 = SymbSrfMult(FffG12, SffL12);
    STmp4 = SymbSrfScalarScale(STmp3, 2.0);
    CagdSrfFree(STmp3);
    STmp3 = SymbSrfAdd(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);
    Denom = SymbSrfSub(STmp3, STmp4);
    CagdSrfFree(STmp3);
    CagdSrfFree(STmp4);

    FffDeterminant = SymbSrfDeterminant2(FffG11, FffG12, FffG12, FffG22);

    CagdSrfFree(FffG11);
    CagdSrfFree(FffG12);
    CagdSrfFree(FffG22);
    CagdSrfFree(SffL11);
    CagdSrfFree(SffL12);
    CagdSrfFree(SffL22);

    if (CAGD_IS_RATIONAL_SRF(FffDeterminant))
	STmp1 = CagdCoerceSrfTo(FffDeterminant, CAGD_PT_P3_TYPE, FALSE);
    else
	STmp1 = CagdCoerceSrfTo(FffDeterminant, CAGD_PT_E3_TYPE, FALSE);
    Points = STmp1 -> Points;
    PtX = Points[1];
    PtY = Points[2];
    PtZ = Points[3];
    for (i = STmp1 -> ULength * STmp1 -> VLength; i > 0; i--)
	*PtY++ = *PtZ++ = *PtX++;
    CagdSrfFree(FffDeterminant);
    FffDeterminant = STmp1;

    Numer = SymbSrfMult(FffDeterminant, SNormal);
    CagdSrfFree(FffDeterminant);
    CagdSrfFree(SNormal);

    SymbSrfSplitScalar(Numer, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(Numer);
    if (SrfW) {
	SymbSrfSplitScalar(Denom, &STmp1, &STmp2, &STmp3, &STmp4);
	if (STmp1 != NULL) {
	    STmp4 = SymbSrfMult(SrfX, STmp1);
	    CagdSrfFree(SrfX);
	    SrfX = STmp4;

	    STmp4 = SymbSrfMult(SrfY, STmp1);
	    CagdSrfFree(SrfY);
	    SrfY = STmp4;

	    if (SrfZ != NULL) {
		STmp4 = SymbSrfMult(SrfZ, STmp1);
		CagdSrfFree(SrfZ);
		SrfZ = STmp4;
	    }

	    CagdSrfFree(STmp1);
	}

	CagdSrfFree(Denom);
	Denom = SymbSrfMult(STmp2, SrfW);
	CagdSrfFree(STmp2);
	CagdSrfFree(SrfW);
    }

    CagdMakeSrfsCompatible(&Denom, &SrfX, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&Denom, &SrfY, TRUE, TRUE, TRUE, TRUE);
    if (SrfZ != NULL)
	CagdMakeSrfsCompatible(&Denom, &SrfZ, TRUE, TRUE, TRUE, TRUE);

    MeanEvolute = SymbSrfMergeScalar(Denom, SrfX, SrfY, SrfZ);
    CagdSrfFree(Denom);
    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    if (SrfZ != NULL)
	CagdSrfFree(SrfZ);

    return MeanEvolute;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the Mean curvature square of a given surface.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to compute Mean curvature square for.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: A surface representing the Mean curvature square field. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanEvolute, SymbSrfMeanNumer, SymbSrfIsoFocalSrf,                M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMeanCurvatureSqr, curvature                                       M
*****************************************************************************/
CagdSrfStruct *SymbSrfMeanCurvatureSqr(const CagdSrfStruct *Srf)
{
    CagdSrfStruct
	*MeanEvolSrf = SymbSrfMeanEvolute(Srf),
	*Mean2SqrRecip = SymbSrfDotProd(MeanEvolSrf, MeanEvolSrf),
        *MeanSqrRecip = SymbSrfScalarScale(Mean2SqrRecip, 4),
	*MeanSqr = SymbSrfInvert(MeanSqrRecip);

    CagdSrfFree(MeanEvolSrf);
    CagdSrfFree(Mean2SqrRecip);
    CagdSrfFree(MeanSqrRecip);

    return MeanSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a focal surface for a principal curvature in an isoparametric     M
* direction. For the u isoparametric direction,				     M
*									     M
*            	        1               G11		                     V
* F(u, v) = n(u, v) --------- = n(u, v) ---                                  V
*                     u                                                      V
*	             k (u, v)           L11                                  V
*		      n						             V
*									     M
* Because Lii also has n(u,v) we can use the nonnormalized surface normal    M
* to compute F(u, v), which is therefore computable and representable.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      Surface to compute iso focal surface.	                     M
*   Dir:      Direction to compute iso focal surface. Either U or V.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surface representing the iso focal surface.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanEvolute, SymbSrfMeanCurvatureSqr, SymbSrfMeanNumer,	     M
*   SymbSrfCurvatureUpperBound, SymbSrfIsoDirNormalCurvatureBound	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfIsoFocalSrf, curvature, focal surface, evolute                    M
*****************************************************************************/
CagdSrfStruct *SymbSrfIsoFocalSrf(const CagdSrfStruct *Srf,
				  CagdSrfDirType Dir)
{
    int i;
    CagdRType **Points, *PtX, *PtY, *PtZ;
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*Numer, *IsoFocalSrf, *SrfX, *SrfY, *SrfZ, *SrfW,
	*Denom, *FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22;

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Numer = FffG11;
	    Denom = SffL11;

	    CagdSrfFree(FffG12);
	    CagdSrfFree(FffG22);

	    CagdSrfFree(SffL12);
	    CagdSrfFree(SffL22);
	    break;
	case CAGD_CONST_V_DIR:
	    Numer = FffG22;
	    Denom = SffL22;

	    CagdSrfFree(FffG11);
	    CagdSrfFree(FffG12);

	    CagdSrfFree(SffL11);
	    CagdSrfFree(SffL12);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
	    Numer = Denom = NULL;
	    break;	    
    }


    if (CAGD_IS_RATIONAL_SRF(Numer))
	STmp1 = CagdCoerceSrfTo(Numer, CAGD_PT_P3_TYPE, FALSE);
    else
	STmp1 = CagdCoerceSrfTo(Numer, CAGD_PT_E3_TYPE, FALSE);
    Points = STmp1 -> Points;
    PtX = Points[1];
    PtY = Points[2];
    PtZ = Points[3];
    for (i = STmp1 -> ULength * STmp1 -> VLength; i > 0; i--)
	*PtY++ = *PtZ++ = *PtX++;
    CagdSrfFree(Numer);
    Numer = STmp1;

    STmp1 = SymbSrfMult(Numer, SNormal);
    CagdSrfFree(Numer);
    CagdSrfFree(SNormal);
    Numer = STmp1;

    SymbSrfSplitScalar(Numer, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(Numer);
    if (SrfW) {
	SymbSrfSplitScalar(Denom, &STmp1, &STmp2, &STmp3, &STmp4);

	if (STmp1 != NULL) {
	    STmp4 = SymbSrfMult(SrfX, STmp1);
	    CagdSrfFree(SrfX);
	    SrfX = STmp4;

	    STmp4 = SymbSrfMult(SrfY, STmp1);
	    CagdSrfFree(SrfY);
	    SrfY = STmp4;

	    if (SrfZ != NULL) {
		STmp4 = SymbSrfMult(SrfZ, STmp1);
		CagdSrfFree(SrfZ);
		SrfZ = STmp4;
	    }

	    CagdSrfFree(STmp1);
	}

	CagdSrfFree(Denom);
	Denom = SymbSrfMult(STmp2, SrfW);
	CagdSrfFree(STmp2);
	CagdSrfFree(SrfW);
    }

    CagdMakeSrfsCompatible(&Denom, &SrfX, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&Denom, &SrfY, TRUE, TRUE, TRUE, TRUE);
    if (SrfZ != NULL)
	CagdMakeSrfsCompatible(&Denom, &SrfZ, TRUE, TRUE, TRUE, TRUE);

    IsoFocalSrf = SymbSrfMergeScalar(Denom, SrfX, SrfY, SrfZ);
    CagdSrfFree(Denom);
    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    if (SrfZ != NULL)
	CagdSrfFree(SrfZ);

    return IsoFocalSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes curvature upper bound as Xi = k1^2 + k2^2, where k1 and k2 are    M
* the principal curvatures. 						     M
* Gij are the coefficients of the first fundamental form and Lij are of the  M
* second, using non unit normal n,					     M
*									     M
*      ( G11 L22 + G22 L11 - 2 G12 L12 )^2 - 2 |G| |L|			     V
* Xi = -----------------------------------------------			     V
*			     |G|^2 ||n||^2				     V
*									     M
* See: "Second Order Surface Analysis Using Hybrid of Symbolic and Numeric   M
* Operators", By Gershon Elber and Elaine Cohen, Transaction on graphics,    M
* Vol. 12, No. 2, pp 160-178, April 1993.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to compute curvature bound for.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar field representing the curvature bound.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanEvolute, SymbSrfMeanCurvatureSqr, SymbSrfMeanNumer,	     M
*   SymbSrfIsoFocalSrf, SymbSrfIsoDirNormalCurvatureBound 		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfCurvatureUpperBound, curvature                                    M
*****************************************************************************/
CagdSrfStruct *SymbSrfCurvatureUpperBound(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*Numer, *FffDeterminant, *SffDeterminant, *CurvatureBound,
	*Denom, *FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22;

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    STmp1 = SymbSrfMult(FffG11, SffL22);
    STmp2 = SymbSrfMult(FffG22, SffL11);
    STmp3 = SymbSrfMult(FffG12, SffL12);
    STmp4 = SymbSrfScalarScale(STmp3, 2.0);
    CagdSrfFree(STmp3);
    STmp3 = SymbSrfAdd(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);
    STmp1 = SymbSrfSub(STmp3, STmp4);
    CagdSrfFree(STmp3);
    CagdSrfFree(STmp4);
    STmp2 = SymbSrfMult(STmp1, STmp1);
    CagdSrfFree(STmp1);

    FffDeterminant = SymbSrfDeterminant2(FffG11, FffG12, FffG12, FffG22);
    SffDeterminant = SymbSrfDeterminant2(SffL11, SffL12, SffL12, SffL22);
    CagdSrfFree(FffG11);
    CagdSrfFree(FffG12);
    CagdSrfFree(FffG22);
    CagdSrfFree(SffL11);
    CagdSrfFree(SffL12);
    CagdSrfFree(SffL22);

    STmp1 = SymbSrfMult(FffDeterminant, SffDeterminant);
    STmp3 = SymbSrfScalarScale(STmp1, 2.0);
    CagdSrfFree(STmp1);
    Numer = SymbSrfSub(STmp2, STmp3);
    CagdSrfFree(STmp2);
    CagdSrfFree(STmp3);

    STmp1 = SymbSrfDotProd(SNormal, SNormal);
    CagdSrfFree(SNormal);

    STmp2 = SymbSrfMult(FffDeterminant, FffDeterminant);
    CagdSrfFree(FffDeterminant);
    CagdSrfFree(SffDeterminant);
    Denom = SymbSrfMult(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);

    CagdMakeSrfsCompatible(&Denom, &Numer, TRUE, TRUE, TRUE, TRUE);
    CurvatureBound = SymbSrfMergeScalar(Denom, Numer, NULL, NULL);
    CagdSrfFree(Denom);
    CagdSrfFree(Numer);

    return CurvatureBound;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes normal curvature bound in given isoparametric direction.          M
*   This turns out to be (L11 . n) / G11 for u and (L22 . n) / G22 for v.    M
*   Herein the square of these equations is computed symbolically and        M
* returned.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute normal curvature in an isoparametric direction Dir. M
*   Dir:      Direction to compute normal curvature. Either U or V.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A scalar field representing the normal curvature       M
*                     square of Srf in dirction Dir.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfDeterminant2, SymbSrfGaussCurvature,      M
*   SymbSrfMeanEvolute, SymbSrfMeanCurvatureSqr, SymbSrfMeanNumer,           M
*   SymbSrfIsoFocalSrf, SymbSrfCurvatureUpperBound			     M 
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfIsoDirNormalCurvatureBound, curvature                             M
*****************************************************************************/
CagdSrfStruct *SymbSrfIsoDirNormalCurvatureBound(const CagdSrfStruct *Srf,
						 CagdSrfDirType Dir)
{
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*SNormalSize, *FffG11, *FffG12, *FffG22, *SffL11, *SffL12, *SffL22,
	*CurvatureBound;

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffG11, &FffG12, &FffG22);
    SymbSrfSff(DuSrf, DvSrf, &SffL11, &SffL12, &SffL22, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    SNormalSize = SymbSrfDotProd(SNormal, SNormal);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    STmp2 = SymbSrfMult(SffL11, SffL11);
	    STmp3 = SymbSrfMult(FffG11, FffG11);
	    STmp4 = SymbSrfMult(SNormalSize, STmp3);
	    CagdSrfFree(STmp3);
	    STmp1 = SymbSrfInvert(STmp4);
	    CagdSrfFree(STmp4);
	    break;
	case CAGD_CONST_V_DIR:
	    STmp2 = SymbSrfMult(SffL22, SffL22);
	    STmp3 = SymbSrfMult(FffG22, FffG22);
	    STmp4 = SymbSrfMult(SNormalSize, STmp3);
	    CagdSrfFree(STmp3);
	    STmp1 = SymbSrfInvert(STmp4);
	    CagdSrfFree(STmp4);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
	    STmp1 = STmp2 = NULL;
    }

    CurvatureBound = SymbSrfMult(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);
    CagdSrfFree(SNormal);
    CagdSrfFree(SNormalSize);
    CagdSrfFree(FffG11);
    CagdSrfFree(FffG12);
    CagdSrfFree(FffG22);
    CagdSrfFree(SffL11);
    CagdSrfFree(SffL12);
    CagdSrfFree(SffL22);

    return CurvatureBound;
}
