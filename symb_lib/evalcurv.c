/******************************************************************************
* EvalCurv.c - Evaluate curvature properties of curves and surfaces.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 96.					      *
******************************************************************************/

#include <string.h>
#include "symb_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

IRIT_STATIC_DATA CagdVType
    GlblNrml, GlblTan, GlblBiNrml, GlblDSrfU, GlblDSrfV;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Preprocess a given curve so we can evaluate curvature properties from    M
* it efficiently, at every point.  See SymbEvalCurvature for actual          M
* curvature at curve point evaluations.				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to preprocess.                                         M
*   Init:	TRUE for initializing, FALSE for clearing out.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalCurvature, SymbEvalCrvCurvTN                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalCrvCurvPrep, curvature                                           M
*****************************************************************************/
void SymbEvalCrvCurvPrep(CagdCrvStruct *Crv, CagdBType Init)
{
    CagdCrvStruct **DCrvs;

    if (Init) {
        int OldInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

        DCrvs = (CagdCrvStruct **) IritMalloc(2 * sizeof(CagdCrvStruct *));
	DCrvs[0] = CagdCrvDerive(Crv);
	DCrvs[1] = CagdCrvDerive(DCrvs[0]);

	BspMultComputationMethod(OldInterp);

	AttrSetPtrAttrib(&Crv -> Attr, "_EvalCurv", DCrvs);
    }
    else {
        int i;

      	if ((DCrvs = (CagdCrvStruct **) AttrGetPtrAttrib(Crv -> Attr,
							 "_EvalCurv")) == NULL)
	    return;

	/* Release the three preprocessed derivative curves. */
	for (i = 0; i < 2; i++)
	    CagdCrvFree(DCrvs[i]);
	IritFree(DCrvs);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate a given curve's curvature.  Returns the curvature for the       M
* given curve location.				   		             M
*   This function must be invoked after SymbEvalCrvCurvPrep was called to    M
* initialize the proper data structures, for fast curvature evaluations at   M
* many points.  SymbEvalCrvCurvPrep should be called at the end to release   M
* these data structures.						     M
*   As a bi-product, function SymbEvalCrvCurvTN could be invoked 	     M
* immediately after this function to fetch the tangent and the normal of     M
* the curve at the given location.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to evaluate its curvature properties.                  M
*   t:		Location of evaluation.					     M
*   k:		The returned curvature at Crv(t).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalCrvCurvPrep, SymbEvalCrvCurvTN                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalCrvCurvature, curvature                                          M
*****************************************************************************/
int SymbEvalCrvCurvature(const CagdCrvStruct *Crv, CagdRType t, CagdRType *k)
{
    CagdRType *R, Denom, Numer;
    CagdVType DDCrv;
    CagdCrvStruct **DCrvs;

    if ((DCrvs = (CagdCrvStruct **) AttrGetPtrAttrib(Crv -> Attr, "_EvalCurv"))
								   == NULL)
        return FALSE;

    R = CagdCrvEval(DCrvs[0], t);
    CagdCoerceToE3(GlblTan, &R, -1, DCrvs[0] -> PType);

    R = CagdCrvEval(DCrvs[1], t);
    CagdCoerceToE3(DDCrv, &R, -1, DCrvs[1] -> PType);

    IRIT_CROSS_PROD(GlblBiNrml, GlblTan, DDCrv);

    Denom = IRIT_DOT_PROD(GlblTan, GlblTan);
    Denom *= IRIT_SQR(Denom);

    Numer = IRIT_DOT_PROD(GlblBiNrml, GlblBiNrml);

    *k = sqrt(Numer/ Denom);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   As bi-product, return the last tangent and normal of the curve evaluated M
* last by SymbEvalCrvCurvature.				          	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Nrml:       The normal to return.                                        M
*   Tan:        The tangent to return.		                             M
*   Normalize:  TRUE to normalize the vectors.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalCrvCurvPrep, SymbEvalCrvCurvature                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalCrvCurvTN, curvature                                             M
*****************************************************************************/
void SymbEvalCrvCurvTN(CagdVType Nrml, CagdVType Tan, int Normalize)
{
    IRIT_CROSS_PROD(Nrml, GlblBiNrml, GlblTan);
    IRIT_VEC_COPY(Tan, GlblTan);

    if (Normalize) {
        IRIT_VEC_NORMALIZE(Tan);
        IRIT_VEC_NORMALIZE(Nrml);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Preprocess a given surface so we can evaluate curvature properties from  M
* it efficiently, at every point.  See SymbEvalCurvature for actual          M
* curvature at surface point evaluations.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   Init:	TRUE for initializing, FALSE for clearing out.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalCurvature, SymbEvalSrfCurvTN                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalSrfCurvPrep, curvature                                           M
*****************************************************************************/
void SymbEvalSrfCurvPrep(CagdSrfStruct *Srf, CagdBType Init)
{
    CagdSrfStruct **DSrfs;

    if (Init) {
        int OldInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

        DSrfs = (CagdSrfStruct **) IritMalloc(6 * sizeof(CagdSrfStruct *));
	DSrfs[0] = SymbSrfNormalSrf(Srf);
	DSrfs[1] = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	DSrfs[2] = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	DSrfs[3] = CagdSrfDerive(DSrfs[1], CAGD_CONST_U_DIR);
	DSrfs[4] = CagdSrfDerive(DSrfs[2], CAGD_CONST_V_DIR);
	DSrfs[5] = CagdSrfDerive(DSrfs[2], CAGD_CONST_U_DIR);

	BspMultComputationMethod(OldInterp);

	AttrSetPtrAttrib(&Srf -> Attr, "_EvalCurv", DSrfs);
    }
    else {
        int i;

      	if ((DSrfs = (CagdSrfStruct **) AttrGetPtrAttrib(Srf -> Attr,
							 "_EvalCurv")) == NULL)
	    return;

	/* Release the three preprocessed derivative surfaces. */
	for (i = 0; i < 6; i++)
	    CagdSrfFree(DSrfs[i]);
	IritFree(DSrfs);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate a given surface's curvature properties.  Returns the principal  M
* curvatures and directions for the given surface location.		     M
*   This function must be invoked after SymbEvalSrfCurvPrep was called to    M
* initialize the proper data structures, for fast curvature evaluations at   M
* many points.  SymbEvalSrfCurvPrep should be called at the end to release   M
* these data structures.						     M
*   As a bi-product, function SymbEvalSrfCurvTN could be invoked 	     M
* immediately after this function to fetch the tangents and the normal of    M
* the surface at the given UV location.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to evaluate its curvature properties.                M
*   U, V:	Location of evaluation.					     M
*   DirInUV:	If TRUE principal directions are given in UV, otherwise in   M
*		Euclidean 3-space.					     M
*   K1, K2:	Principal curvatures.					     M
*   D1, D2:	Principal directions.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvPrep, SymbEvalSrfCurvTN                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalSrfCurvature, curvature                                          M
*****************************************************************************/
int SymbEvalSrfCurvature(const CagdSrfStruct *Srf,
			 CagdRType U,
			 CagdRType V,
			 CagdBType DirInUV,
			 CagdRType *K1,
			 CagdRType *K2,
			 CagdVType D1,
			 CagdVType D2)
{
    int RetVal = FALSE;
    CagdRType *R, E, F, G, L, M, N, A, B, C, Disc, v1, v2;
    CagdVType DSrfUU, DSrfVV, DSrfUV, Vec;
    CagdSrfStruct **DSrfs;

    if ((DSrfs = (CagdSrfStruct **) AttrGetPtrAttrib(Srf -> Attr, "_EvalCurv"))
								   == NULL)
        return RetVal;

    R = CagdSrfEval(DSrfs[0], U, V);
    CagdCoerceToE3(GlblNrml, &R, -1, DSrfs[0] -> PType);
    IRIT_PT_NORMALIZE(GlblNrml);

    R = CagdSrfEval(DSrfs[1], U, V);
    CagdCoerceToE3(GlblDSrfU, &R, -1, DSrfs[1] -> PType);

    R = CagdSrfEval(DSrfs[2], U, V);
    CagdCoerceToE3(GlblDSrfV, &R, -1, DSrfs[2] -> PType);

    R = CagdSrfEval(DSrfs[3], U, V);
    CagdCoerceToE3(DSrfUU, &R, -1, DSrfs[3] -> PType);

    R = CagdSrfEval(DSrfs[4], U, V);
    CagdCoerceToE3(DSrfVV, &R, -1, DSrfs[4] -> PType);

    R = CagdSrfEval(DSrfs[5], U, V);
    CagdCoerceToE3(DSrfUV, &R, -1, DSrfs[5] -> PType);

    /* Compute the coefficients of the first and second fundamental forms.   */
    E = IRIT_DOT_PROD(GlblDSrfU, GlblDSrfU);
    F = IRIT_DOT_PROD(GlblDSrfU, GlblDSrfV);
    G = IRIT_DOT_PROD(GlblDSrfV, GlblDSrfV);

    L = IRIT_DOT_PROD(DSrfUU, GlblNrml);
    M = IRIT_DOT_PROD(DSrfUV, GlblNrml);
    N = IRIT_DOT_PROD(DSrfVV, GlblNrml);

    /* Solve the quadratic equation for the principal curvature:             */
    /* (EG - F^2) k^2 - (GL + EN - 2FM) k + (LN - M^2) = 0.		     */
    A = E * G - F * F;
    B = -(G * L + E * N - 2 * F * M);
    C = L * N - M * M;

    if ((Disc = B * B - 4 * A * C) < 0.0) {
	if (Disc < -IRIT_EPS)
	    RetVal = FALSE;
	Disc = 0.0;
    }
    else
	RetVal = TRUE;
    Disc = sqrt(Disc);

    *K1 = (-B - Disc) / (2 * A);
    *K2 = (-B + Disc) / (2 * A);

    /* Compute the principal directions - K1. */
    A = L - *K1 * E;
    B = M - *K1 * F;
    C = N - *K1 * G;
    if (IRIT_FABS(A) > IRIT_FABS(C)) {
        v1 = B;
        v2 = -A;
    }
    else {
        v1 = C;
	v2 = -B;
    }
    if (DirInUV) {
	D1[0] = v1;
	D1[1] = v2;
	D1[2] = 0.0;
    }
    else {
	IRIT_PT_COPY(D1, GlblDSrfU);
	IRIT_PT_SCALE(D1, v1);
	IRIT_PT_COPY(Vec, GlblDSrfV);
	IRIT_PT_SCALE(Vec, v2);
	IRIT_PT_ADD(D1, D1, Vec);
	IRIT_PT_NORMALIZE(D1);
    }

    /* Compute the principal directions - K2. */
    A = L - *K2 * E;
    B = M - *K2 * F;
    C = N - *K2 * G;
    if (IRIT_FABS(A) > IRIT_FABS(C)) {
        v1 = B;
        v2 = -A;
    }
    else {
        v1 = C;
	v2 = -B;
    }
    if (DirInUV) {
	D2[0] = v1;
	D2[1] = v2;
	D2[2] = 0.0;
    }
    else {
	IRIT_PT_COPY(D2, GlblDSrfU);
	IRIT_PT_SCALE(D2, v1);
	IRIT_PT_COPY(Vec, GlblDSrfV);
	IRIT_PT_SCALE(Vec, v2);
	IRIT_PT_ADD(D2, D2, Vec);
	IRIT_PT_NORMALIZE(D2);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   As bi-product, return the last normal and partial tangents of the        m
* surface evaluated last by SymbEvalSrfCurvature.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Nrml:          The normal to return.                                     M
*   DSrfU, DSrfV:  The two partials of the surface.                          M
*   Normalize:     TRUE to normalize the vectors.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvPrep, SymbEvalSrfCurvature                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalSrfCurvTN, curvature                                             M
*****************************************************************************/
void SymbEvalSrfCurvTN(CagdVType Nrml,
		       CagdVType DSrfU,
		       CagdVType DSrfV,
		       int Normalize)
{
    IRIT_VEC_COPY(Nrml, GlblNrml);
    IRIT_VEC_COPY(DSrfU, GlblDSrfU);
    IRIT_VEC_COPY(DSrfV, GlblDSrfV);

    if (Normalize) {
        IRIT_VEC_NORMALIZE(DSrfU);
        IRIT_VEC_NORMALIZE(DSrfV);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the asymptotic directions of the surface in the given U, V        M
* location, if exists.  If DirInUV, the returned asymptotic direction is     M
* in UV space, otherwise, in Euclidean surface tangent plane space.	     M
*   This function must be invoked after SymbEvalSrfCurvPrep was called to    M
* initialize the proper data structures, for fast curvature evaluations at   M
* many points.  SymbEvalSrfCurvPrep should be called at the end to release   M
* these data structures.						     M
*   The asymptotic direction(s) is the direction for which the normal        M
* curvature vanish and hence can exist for hyperbolic regions.  We solve:    M
*									     M
* [t  (1-t)] [ L  M ] [t  ]						     V
*            [      ] [   ]						     V
*            [ M  N ] [1-t]						     V
*									     M
* and look for solution of t between zero and one as:			     M
*									     M
* t = (N-M +/- sqrt(M^2-LN)) / (L-2M+N).				     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To compute asymptotic directions for.			     M
*   U, V:        Location of evaluation.				     M
*   DirInUV:     If TRUE asymptotic direction is given in UV, otherwise in   M
*	         Euclidean 3-space.					     M
*   AsympDir1, AsympDir2: Returned values.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Number of asymptotic directions found, zero for none.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvature, SymbEvalSrfCurvPrep                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEvalSrfAsympDir, curvature                                           M
*****************************************************************************/
int SymbEvalSrfAsympDir(const CagdSrfStruct *Srf,
			CagdRType U,
			CagdRType V,
			CagdBType DirInUV,
			CagdVType AsympDir1,
			CagdVType AsympDir2)
{
    int NumSol = 0;
    CagdRType *R, L, M, N, Desc, t1, t2;
    CagdVType Nrml, DSrfU, DSrfV, DSrfUU, DSrfVV, DSrfUV;
    CagdSrfStruct **DSrfs;

    if ((DSrfs = (CagdSrfStruct **) AttrGetPtrAttrib(Srf -> Attr, "_EvalCurv"))
								   == NULL)
        return NumSol;

    R = CagdSrfEval(DSrfs[0], U, V);
    CagdCoerceToE3(Nrml, &R, -1, DSrfs[0] -> PType);
    IRIT_PT_NORMALIZE(Nrml);

    R = CagdSrfEval(DSrfs[1], U, V);
    CagdCoerceToE3(DSrfU, &R, -1, DSrfs[1] -> PType);

    R = CagdSrfEval(DSrfs[2], U, V);
    CagdCoerceToE3(DSrfV, &R, -1, DSrfs[2] -> PType);

    R = CagdSrfEval(DSrfs[3], U, V);
    CagdCoerceToE3(DSrfUU, &R, -1, DSrfs[3] -> PType);

    R = CagdSrfEval(DSrfs[4], U, V);
    CagdCoerceToE3(DSrfVV, &R, -1, DSrfs[4] -> PType);

    R = CagdSrfEval(DSrfs[5], U, V);
    CagdCoerceToE3(DSrfUV, &R, -1, DSrfs[5] -> PType);

    /* Compute the coefficients of the second fundamental forms.   */
    L = IRIT_DOT_PROD(DSrfUU, Nrml);
    M = IRIT_DOT_PROD(DSrfUV, Nrml);
    N = IRIT_DOT_PROD(DSrfVV, Nrml);

    Desc = IRIT_SQR(M) - L * N;
    if (Desc < 0)
	return NumSol;				 /* No asymptotic direction. */

    t1 = (N - M + sqrt(Desc)) / (L - 2 * M + N);	
    t2 = (N - M - sqrt(Desc)) / (L - 2 * M + N);

    /* Valid solution. */
    AsympDir1[0] = t1;
    AsympDir1[1] = 1 - t1;
    AsympDir1[2] = 0;
    NumSol++;

    if (!IRIT_APX_EQ(t1, t2)) {
        AsympDir2[0] = t2;
	AsympDir2[1] = 1 - t2;
	AsympDir2[2] = 0;
	NumSol++;
    }

    if (DirInUV)
	return NumSol;

    if (NumSol >= 1) {
        IRIT_VEC_BLEND(AsympDir1, DSrfU, DSrfV, t1);
	IRIT_VEC_NORMALIZE(AsympDir1);
    }
    if (NumSol >= 2) {
        IRIT_VEC_BLEND(AsympDir2, DSrfU, DSrfV, t2);
	IRIT_VEC_NORMALIZE(AsympDir2);
    }
    return NumSol;
}
