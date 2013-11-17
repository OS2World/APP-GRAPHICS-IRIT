/******************************************************************************
* MvAccess.c - evaluate the accessibility of surfaces/check surfaces.         *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 99.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "mvar_loc.h"

#define MVAR_TRIVIAL_SOL_RELTOL 100

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Derives the visibility domain of PosSrf, when accessed from the	     M
* direction that is specified by OrientSrf, not gouging into CheckSrf.	     M
*   The boundary of the domain is prescribed by the following set of         M
* three constraints:							     M
*       F1(u, v, s, t):  <O1(u, v), S(u, v) - K(s, t)> = 0		     V
*       F2(u, v, s, t):  <O2(u, v), S(u, v) - K(s, t)> = 0		     V
*       F3(u, v, s, t):  <Nk(s, t), S(u, v) - K(s, t)> = 0		     V
* where O1(u, v), O2(u, v) are two vector fields orthogonal to O(u, v), the  M
* orientation field, K(s, t) is the check surface and Nk(s, t) is its normal M
* field, and S(u, v) is the position surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CPosSrf:      The position surface to examine.                           M
*   COrientSrf:   The orientation field of the access to the position        M
*		  surface.  Must be in same function space and PosSrf.       M
*		  If NULL, the normal field of PosSrf is empoloyed.	     M
*   CCheckSrf:    The surface to check gouging against.                      M
*   AccessDir:    Limit on a hemisphere of directions around AccessDir, if   M
*		  not NULL.						     M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   The boundary of the accessible regions of PosSrf.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfAccessibility                                                     M
*****************************************************************************/
MvarPtStruct *MvarSrfAccessibility(const CagdSrfStruct *CPosSrf,
				   const CagdSrfStruct *COrientSrf,
				   const CagdSrfStruct *CCheckSrf,
				   const CagdRType *AccessDir,
				   CagdRType SubdivTol,
				   CagdRType NumerTol)
{
    CagdRType UMin[2], UMax[2], VMin[2], VMax[2];
    CagdSrfStruct *PosSrf, *OrientSrf, *CheckSrf,
        *SrfTmp, *OrientOrtho1, *OrientOrtho2, *CheckNormalSrf;
    MvarConstraintType Constraints[4];
    MvarMVStruct *MVF[4], *MVAccessDir, *MVTmp, *MVSrfCheck, *MVSrf, *MVCheck,
	*MVOrientOrtho1, *MVOrientOrtho2, *MVCheckNrml;
    MvarPtStruct *Pts, *Pt;

    CagdSrfDomain(CPosSrf, &UMin[0], &UMax[0], &VMin[0], &VMax[0]);
    if (COrientSrf) {
        CagdSrfDomain(COrientSrf, &UMin[1], &UMax[1], &VMin[1], &VMax[1]);
	if (UMin[0] != UMin[1] || UMax[0] != UMax[1] ||
	    VMin[0] != VMin[1] || VMax[0] != VMax[1])
	    MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
    }
    CagdSrfDomain(CCheckSrf, &UMin[1], &UMax[1], &VMin[1], &VMax[1]);

    /* Make sure all domains are zero to one in all axes. */
    PosSrf = CagdSrfCopy(CPosSrf);
    OrientSrf = COrientSrf == NULL ? NULL : CagdSrfCopy(COrientSrf);
    CheckSrf = CagdSrfCopy(CCheckSrf);

    if (CAGD_IS_BSPLINE_SRF(PosSrf)) {
	BspKnotAffineTransOrder2(PosSrf -> UKnotVector, PosSrf -> UOrder,
				 PosSrf -> ULength + PosSrf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(PosSrf -> VKnotVector, PosSrf -> VOrder,
				 PosSrf -> VLength + PosSrf -> VOrder,
				 0.0, 1.0);
    }
    if (OrientSrf && CAGD_IS_BSPLINE_SRF(OrientSrf)) {
        BspKnotAffineTransOrder2(OrientSrf -> UKnotVector, OrientSrf -> UOrder,
				 OrientSrf -> ULength + OrientSrf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(OrientSrf -> VKnotVector, OrientSrf -> VOrder,
				 OrientSrf -> VLength + OrientSrf -> VOrder,
				 0.0, 1.0);
    }
    if (CAGD_IS_BSPLINE_SRF(CheckSrf)) {
	BspKnotAffineTransOrder2(CheckSrf -> UKnotVector, CheckSrf -> UOrder,
				 CheckSrf -> ULength + CheckSrf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(CheckSrf -> VKnotVector, CheckSrf -> VOrder,
				 CheckSrf -> VLength + CheckSrf -> VOrder,
				 0.0, 1.0);
    }
    
    /* Evaluate the auxiliary surface fields and convert to multivariates. */
    CheckNormalSrf = SymbSrfNormalSrf(CheckSrf);

    MVSrf = MvarSrfToMV(PosSrf);
    MVCheck = MvarSrfToMV(CheckSrf);
    MVCheckNrml = MvarSrfToMV(CheckNormalSrf);
    
    CagdSrfFree(CheckNormalSrf);

    if (OrientSrf == NULL) {
	OrientOrtho1 = CagdSrfDerive(PosSrf, CAGD_CONST_U_DIR);
	OrientOrtho2 = CagdSrfDerive(PosSrf, CAGD_CONST_V_DIR);
    }
    else {
        /* Construct two fields orthogonal to orientation surface. */
        SrfTmp = CagdSrfDerive(PosSrf, CAGD_CONST_U_DIR);
	OrientOrtho1 = SymbSrfCrossProd(SrfTmp, OrientSrf);
	OrientOrtho2 = SymbSrfCrossProd(OrientOrtho1, OrientSrf);
	CagdSrfFree(SrfTmp);
    }
    MVOrientOrtho1 = MvarSrfToMV(OrientOrtho1);
    MVOrientOrtho2 = MvarSrfToMV(OrientOrtho2);
    CagdSrfFree(OrientOrtho1);
    CagdSrfFree(OrientOrtho2);

    /* Handle the accessibility direction limitation, if any. */
    if (AccessDir != NULL) {
        CagdSrfStruct
	    *NSrf = OrientSrf == NULL ? SymbSrfNormalSrf(PosSrf) : OrientSrf;
	    
	SrfTmp = SymbSrfVecDotProd(NSrf, AccessDir);
	MVTmp = MvarSrfToMV(SrfTmp);
	CagdSrfFree(SrfTmp);

	MVAccessDir = MvarPromoteMVToMV2(MVTmp, 4, 0);
	MvarMVFree(MVTmp);

	if (OrientSrf == NULL)
	    CagdSrfFree(NSrf);
    }
    else
        MVAccessDir = NULL;

    /* Promote to the proper axes, into (u, v, s, t) space. */
    MVTmp = MvarPromoteMVToMV2(MVSrf, 4, 0);
    MvarMVFree(MVSrf);
    MVSrf = MVTmp;

    MVTmp = MvarPromoteMVToMV2(MVOrientOrtho1, 4, 0);
    MvarMVFree(MVOrientOrtho1);
    MVOrientOrtho1 = MVTmp;

    MVTmp = MvarPromoteMVToMV2(MVOrientOrtho2, 4, 0);
    MvarMVFree(MVOrientOrtho2);
    MVOrientOrtho2 = MVTmp;

    MVTmp = MvarPromoteMVToMV2(MVCheck, 4, 2);
    MvarMVFree(MVCheck);
    MVCheck = MVTmp;

    MVTmp = MvarPromoteMVToMV2(MVCheckNrml, 4, 2);
    MvarMVFree(MVCheckNrml);
    MVCheckNrml = MVTmp;

    MVSrfCheck = MvarMVSub(MVSrf, MVCheck);
    MvarMVFree(MVSrf);
    MvarMVFree(MVCheck);

    MVF[0] = MvarMVDotProd(MVOrientOrtho1, MVSrfCheck);
    MVF[1] = MvarMVDotProd(MVOrientOrtho2, MVSrfCheck);
    MVF[2] = MvarMVDotProd(MVCheckNrml, MVSrfCheck);

    MvarMVFree(MVSrfCheck);
    MvarMVFree(MVOrientOrtho1);
    MvarMVFree(MVOrientOrtho2);
    MvarMVFree(MVCheckNrml);

    CagdSrfFree(OrientSrf);
    CagdSrfFree(PosSrf);
    CagdSrfFree(CheckSrf);

    /* Build the zero constraints. */
    if (MVAccessDir != NULL) {
	MVF[3] = MVAccessDir;
        Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
        Constraints[3] = MVAR_CNSTRNT_POSITIVE;
	Pts = MvarMVsZeros(MVF, Constraints, 4, SubdivTol, NumerTol);
    }
    else {
        Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
	Pts = MvarMVsZeros(MVF, Constraints, 3, SubdivTol, NumerTol);
    }

    MvarMVFree(MVF[0]);
    MvarMVFree(MVF[1]);
    MvarMVFree(MVF[2]);
    if (MVAccessDir != NULL)
        MvarMVFree(MVAccessDir);

    /* Recover the solution's domain as we transformed to (0, 1) domains. */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        Pt -> Pt[0] = Pt -> Pt[0] * (UMax[0] - UMin[0]) + UMin[0];
        Pt -> Pt[1] = Pt -> Pt[1] * (VMax[0] - VMin[0]) + VMin[0];
        Pt -> Pt[2] = Pt -> Pt[2] * (UMax[1] - UMin[1]) + UMin[1];
        Pt -> Pt[3] = Pt -> Pt[3] * (VMax[1] - VMin[1]) + VMin[1];
    }

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the silhouette locations on surface Srf viewed from ViewDir     M
* that has inflection points or contact for second order or more. This       M
* amounts to finding the asymptotic directions of Srf that are in the        M
* direction of ViewDir.							     M
*   Let V(s, t) be a parametrization of all possible viewing directions.     M
*   Solution is derived as the solution of the following three equations:    M
*									     M
*      dS   dS							 	     V
*    < -- x -- , V > = 0,           (View direction is in the tangent space) V
*      du   dv								     V
*									     M
*             [ L M ] [ a ]						     V
*    [ a  b ] [     ] [   ] = 0,    (This location/direction is asymp.)      V
*             [ M N ] [ b ]	    (L, M, N are the coef. of SFF.)	     V
*									     M
* where a and b are the solutions of					     M
*									     M
*      dS     dS						 	     V
*    a -- + b -- = V.					 		     V
*      du     dv						 	     V
*									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute the silhouette higher orders' contact points.     M
*   ViewDir:    View direction to consider.				     M
*   SubdivTol:  Accuracy of the subdivision stage of the approximation.      M
*   NumerTol:   Accuracy of numeric approx.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Polylines on the unit sphere, depicting the	     M
*		 flecnodal's partitioning lines.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfTopoAspectGraph, SymbEvalSrfAsympDir       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSilhInflections                                                   M
*****************************************************************************/
MvarPtStruct *MvarSrfSilhInflections(const CagdSrfStruct *Srf,
				     const CagdVType ViewDir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol)
{
    CagdBType
	OldCagdInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
	OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdSrfStruct
	*DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
	*DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR),
	*Nrml = SymbSrfCrossProd(DuSrf, DvSrf),
	*Du2Srf = CagdSrfDerive(DuSrf, CAGD_CONST_U_DIR),
	*DuDvSrf = CagdSrfDerive(DuSrf, CAGD_CONST_V_DIR),
	*Dv2Srf = CagdSrfDerive(DvSrf, CAGD_CONST_V_DIR);
    MvarPtStruct *Pts;
    MvarConstraintType Constraints[2];
    MvarMVStruct *A, *B, *A2, *AB, *B2, *MVNrml, *MVSplitDuSrf[4], *MVConst[2],
	*MVSplitDvSrf[4], **MVSplit, *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4;

    /* The first constraint for V to be in the tangent space. */
    MVNrml = MvarSrfToMV(Nrml);
    CagdSrfFree(Nrml);
    MVConst[0] = MvarMVVecDotProd(MVNrml, ViewDir);

    /* The second constraints... */
    MVTmp1 = MvarSrfToMV(DuSrf);
    CagdSrfFree(DuSrf);
    MVSplit = MvarMVSplitScalar(MVTmp1);
    IRIT_GEN_COPY(MVSplitDuSrf, MVSplit, 4 * sizeof(MvarMVStruct *));
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarSrfToMV(DvSrf);
    CagdSrfFree(DvSrf);
    MVSplit = MvarMVSplitScalar(MVTmp1);
    IRIT_GEN_COPY(MVSplitDvSrf, MVSplit, 4 * sizeof(MvarMVStruct *));
    MvarMVFree(MVTmp1);

    /* Cramer's rule solve for A and B from largest coefficients of ViewDir. */
    if (IRIT_FABS(ViewDir[0]) >= IRIT_FABS(ViewDir[2]) &&
	IRIT_FABS(ViewDir[1]) >= IRIT_FABS(ViewDir[2])) { /* Use X & Y axes. */
	MVTmp1 = MvarMVScalarScale(MVSplitDvSrf[2], ViewDir[0]);
	MVTmp2 = MvarMVScalarScale(MVSplitDvSrf[1], ViewDir[1]);
	A = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);

	MVTmp1 = MvarMVScalarScale(MVSplitDuSrf[1], ViewDir[1]);
	MVTmp2 = MvarMVScalarScale(MVSplitDuSrf[2], ViewDir[0]);
	B = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
    }
    else if (IRIT_FABS(ViewDir[0]) >= IRIT_FABS(ViewDir[1]) &&
	     IRIT_FABS(ViewDir[2]) >= IRIT_FABS(ViewDir[1])) { /* Use X & Z. */
	MVTmp1 = MvarMVScalarScale(MVSplitDvSrf[3], ViewDir[0]);
	MVTmp2 = MvarMVScalarScale(MVSplitDvSrf[1], ViewDir[2]);
	A = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);

	MVTmp1 = MvarMVScalarScale(MVSplitDuSrf[1], ViewDir[2]);
	MVTmp2 = MvarMVScalarScale(MVSplitDuSrf[3], ViewDir[0]);
	B = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
    }
    else {					         /* Use Y & Z axes. */
	MVTmp1 = MvarMVScalarScale(MVSplitDvSrf[3], ViewDir[1]);
	MVTmp2 = MvarMVScalarScale(MVSplitDvSrf[2], ViewDir[2]);
	A = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);

	MVTmp1 = MvarMVScalarScale(MVSplitDuSrf[2], ViewDir[2]);
	MVTmp2 = MvarMVScalarScale(MVSplitDuSrf[3], ViewDir[1]);
	B = MvarMVSub(MVTmp1, MVTmp2);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
    }

    MvarMVFree(MVSplitDuSrf[0]);
    MvarMVFree(MVSplitDuSrf[1]);
    MvarMVFree(MVSplitDuSrf[2]);
    MvarMVFree(MVSplitDuSrf[3]);

    MvarMVFree(MVSplitDvSrf[0]);
    MvarMVFree(MVSplitDvSrf[1]);
    MvarMVFree(MVSplitDvSrf[2]);
    MvarMVFree(MVSplitDvSrf[3]);

    /* Compute the products of A & B. */
    A2 = MvarMVMult(A, A);
    B2 = MvarMVMult(B, B);
    AB = MvarMVMult(A, B);
    MvarMVTransform(AB, NULL, 2.0);

    /* The second order asymptotic constraint. */
    MVTmp1 = MvarSrfToMV(Du2Srf);
    CagdSrfFree(Du2Srf);
    MVTmp2 = MvarMVMultScalar(MVTmp1, A2);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarSrfToMV(DuDvSrf);
    CagdSrfFree(DuDvSrf);
    MVTmp3 = MvarMVMultScalar(MVTmp1, AB);
    MvarMVFree(MVTmp1);
    MVTmp4 = MvarMVAdd(MVTmp3, MVTmp2);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);

    MVTmp1 = MvarSrfToMV(Dv2Srf);
    CagdSrfFree(Dv2Srf);
    MVTmp2 = MvarMVMultScalar(MVTmp1, B2);
    MvarMVFree(MVTmp1);
    MVTmp3 = MvarMVAdd(MVTmp4, MVTmp2);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp4);

    MVConst[1] = MvarMVDotProd(MVNrml, MVTmp3);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVNrml);

    MvarMVFree(A2);
    MvarMVFree(AB);
    MvarMVFree(B2);

    BspMultComputationMethod(OldCagdInterpFlag);
    MvarBspMultComputationMethod(OldMvarInterpFlag);

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVConst, Constraints, 2, SubdivTol, NumerTol);

    if (IRIT_FABS(NumerTol) < SubdivTol) {
        MvarPtStruct *FilteredPts, *Pt;

        /* Filter points for which A = B = 0.0, trivial undesired solution. */
        for (FilteredPts = NULL; Pts != NULL; ) {
	    CagdRType *R, AVal, BVal;
	    
	    IRIT_LIST_POP(Pt, Pts);

	    R = MvarMVEval(A, Pt -> Pt);
	    AVal = R[1];
	    R = MvarMVEval(B, Pt -> Pt);
	    BVal = R[1];

	    if (IRIT_APX_EQ_EPS(AVal, 0.0,
			   IRIT_FABS(NumerTol) * MVAR_TRIVIAL_SOL_RELTOL) &&
		IRIT_APX_EQ_EPS(BVal, 0.0,
			   IRIT_FABS(NumerTol) * MVAR_TRIVIAL_SOL_RELTOL)) {
	        MvarPtFree(Pt);
	    }
	    else {
	        IRIT_LIST_PUSH(Pt, FilteredPts);
	    }
	}

	Pts = FilteredPts;
    }

    MvarMVFree(A);
    MvarMVFree(B);	

    MvarMVFree(MVConst[0]);
    MvarMVFree(MVConst[1]);

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs the following steps in order:                                   M
* 1. Project Srf onto NrmlSrf by computing their inner product.              M
* 2. Promote the surface to a 4-variate with the surface the first 2 vars.   M
* 3. Scale the new 4-variate by a scalar product with MVScl.		     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to project promote and scale.                           M
*   NrmlSrf: Normal field to project along.		                     M
*   MVScl:   Scale field to scale with.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Resulting multivariate.                                 *
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarProjNrmlPrmt2MVScl                                                   M
*****************************************************************************/
MvarMVStruct *MVarProjNrmlPrmt2MVScl(const CagdSrfStruct *Srf,
				     const CagdSrfStruct *NrmlSrf,
				     const MvarMVStruct *MVScl)
{
    CagdSrfStruct
        *DotProd = SymbSrfDotProd(Srf, NrmlSrf);
    MvarMVStruct *MVRes,
	*MVDotProd = MvarSrfToMV(DotProd),
	*MVPrmtDotProd = MvarPromoteMVToMV2(MVDotProd, 4, 0);

    MVRes = MvarMVMult(MVPrmtDotProd, MVScl);

    CagdSrfFree(DotProd);
    MvarMVFree(MVDotProd);
    MvarMVFree(MVPrmtDotProd);

    return MVRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the MV constraints of flecnodal curves on surface S(u, v).      M
*   Denote by Suv the second order derivatives of S(u, v) and by Suvv the    M
* third order derivatives.  Then, solution of the following 3 equations in 4 M
* unknowns (u, v, a, b) provides the flecnodal curves:			     M
*									     M
*             [ L M ] [ a ]						     V
* 1. [ a  b ] [     ] [   ] =       (This location/direction is asymp.)      V
*             [ M N ] [ b ]	    (L, M, N are the coef. of SFF.)	     V
*									     M
*                [ Suu Suv ]           [ a ]				     V
*     = [ a  b ] [         ] . n(u, v) [   ] = 0,			     V
*                [ Suv Svv ]           [ b ]				     V
*									     M
*    or									     M
*									     M
*    < a^2 Suu + 2ab Suv + b^2 Svv, n(u, v) > = 0.			     V
*									     M
*             (   [ Suuu Suuv ]       [ Svuu Svuv ] )            [ a ]       V
* 2. [ a  b ] ( a [           ]  +  b [           ] )  . n(u, v) [   ] = 0,  V
*             (   [ Suuv Suvv ]       [ Svuv Svvv ] )            [ b ]       V
*									     M
*		                   (A third derivative zero contact.)        M
*									     M
*    or									     M
*									     M
*    < a^3 Suuu + 3a^2b Suuv + 3ab^2 Suvv + b^3 Svvv, n(u, v) > = 0.	     V
*									     M
* 3. a * a + b * b = 1             (A normalization constraint.)	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       To compute the (MV constraints of the) flecnodals for.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct **:  The set of 3 constraints in 4 unknowns to derive the   M
*		 flecnodal curves of CSrf.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfTopoAspectGraph, SymbEvalSrfAsympDir,      M
*   MvarSrfSilhInflections, MvarSrfFlecnodalCrvs			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarFlecnodalCrvsCreateMVCnstrnts                                        M
*****************************************************************************/
MvarMVStruct **MvarFlecnodalCrvsCreateMVCnstrnts(const CagdSrfStruct *CSrf)
{
    CagdBType
	OldCagdInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
	OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType UMin, UMax, VMin, VMax, Translate[3];
    CagdSrfStruct *Srf, *DuSrf, *DvSrf, *Nrml, *Du2Srf, *DuvSrf, *Dv2Srf,
	*Du3Srf, *Du2vSrf, *Duv2Srf, *Dv3Srf;
    MvarMVStruct *A, *B, *A2, *AB, *B2, *A3, *AB2, *A2B, *B3,
        *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5, **MVConst;

    MVConst = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * 3);

    IRIT_PT_RESET(Translate);

    if (CAGD_IS_BEZIER_SRF(CSrf))
        Srf = CagdCnvrtBzr2BspSrf(CSrf);
    else
        Srf = CagdSrfCopy(CSrf);

    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
    Nrml = SymbSrfCrossProd(DuSrf, DvSrf);
    Du2Srf = CagdSrfDerive(DuSrf, CAGD_CONST_U_DIR);
    DuvSrf = CagdSrfDerive(DuSrf, CAGD_CONST_V_DIR);
    Dv2Srf = CagdSrfDerive(DvSrf, CAGD_CONST_V_DIR);
    Du3Srf = CagdSrfDerive(Du2Srf, CAGD_CONST_U_DIR);
    Du2vSrf = CagdSrfDerive(DuvSrf, CAGD_CONST_U_DIR);
    Duv2Srf = CagdSrfDerive(DuvSrf, CAGD_CONST_V_DIR);
    Dv3Srf = CagdSrfDerive(Dv2Srf, CAGD_CONST_V_DIR);

    /* Construct A:[-1, 1], B:[-1, 1] as third and fourth variables.       */
    /* Note we are having for A a domain of [0, 1] but range of [-1, 1] so */
    /* we will need to compensate at the end...			           */
    A = MvarCnvrtBzr2BspMV(MVTmp1 = MvarBuildParamMV(4, 2, -1, 1));
    B = MvarCnvrtBzr2BspMV(MVTmp2 = MvarBuildParamMV(4, 3, 0, 1));
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    /* Make sure domain are the same. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    BspKnotAffineTrans2(A -> KnotVectors[0],
			A -> Lengths[0] + A -> Orders[0],
			UMin, UMax);
    BspKnotAffineTrans2(B -> KnotVectors[0],
			B -> Lengths[0] + B -> Orders[0],
			UMin, UMax);
    BspKnotAffineTrans2(A -> KnotVectors[1],
			A -> Lengths[1] + A -> Orders[1],
			VMin, VMax);
    BspKnotAffineTrans2(B -> KnotVectors[1],
			B -> Lengths[1] + B -> Orders[1],
			VMin, VMax);

    /* Compute the products of A & B. */
    A2 = MvarMVMult(A, A);
    AB = MvarMVMult(A, B);
    B2 = MvarMVMult(B, B);

    A3 = MvarMVMult(A2, A);
    A2B = MvarMVMult(A2, B);
    AB2 = MvarMVMult(A, B2);
    B3 = MvarMVMult(B2, B);

    MvarMVTransform(AB, NULL, 2.0);
    MvarMVTransform(A2B, NULL, 3.0);
    MvarMVTransform(AB2, NULL, 3.0);
    MvarMVFree(A);
    MvarMVFree(B);	

    /* 1. The second-order asymptotic direction constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du2Srf, Nrml, A2);
    CagdSrfFree(Du2Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(DuvSrf, Nrml, AB);
    CagdSrfFree(DuvSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Dv2Srf, Nrml, B2);
    CagdSrfFree(Dv2Srf);

    MVTmp4 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVConst[0] = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    /* 2. The third-order flecnodal constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du3Srf, Nrml, A3);
    CagdSrfFree(Du3Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(Du2vSrf, Nrml, A2B);
    CagdSrfFree(Du2vSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Duv2Srf, Nrml, AB2);
    CagdSrfFree(Duv2Srf);
    MVTmp4 = MVarProjNrmlPrmt2MVScl(Dv3Srf, Nrml, B3);
    CagdSrfFree(Dv3Srf);

    MVTmp5 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    MVConst[1] = MvarMVAdd(MVTmp1, MVTmp5);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp5);

    /* 3. The normalization constraint. */
    MVConst[2] = MvarMVAdd(A2, B2);
    Translate[0] = -1;
    MvarMVTransform(MVConst[2], Translate, 1.0);

    /* Free auxiliary data. */
    CagdSrfFree(Nrml);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    MvarMVFree(A2);
    MvarMVFree(AB);
    MvarMVFree(B2);

    MvarMVFree(A3);
    MvarMVFree(AB2);
    MvarMVFree(A2B);
    MvarMVFree(B3);

    BspMultComputationMethod(OldCagdInterpFlag);
    MvarBspMultComputationMethod(OldMvarInterpFlag);

    CagdSrfFree(Srf);

    return MVConst;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the flecnodal curves on surface Srf = S(u, v). Uses the zero-   M
* dimension MV solver.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute the flecnodal curves for.			     M
*   SubdivTol:  Accuracy of the subdivision stage of the approximation.      M
*   NumerTol:   Accuracy of numeric approx.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Flecnodal curves as polylines in (a, b, u, v) space.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarFlecnodalCrvsCreateMVs, MvarSrfFlecnodalCrvs2                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfFlecnodalCrvs                                                     M
*****************************************************************************/
MvarPtStruct *MvarSrfFlecnodalCrvs(const CagdSrfStruct *Srf,
				   CagdRType SubdivTol,
				   CagdRType NumerTol)
{
    MvarConstraintType Constraints[3];
    MvarPtStruct *Pts, *Pt;
    MvarMVStruct
        **MVConst = MvarFlecnodalCrvsCreateMVCnstrnts(Srf);

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO; 
    Pts = MvarMVsZeros(MVConst, Constraints, 3, SubdivTol, NumerTol);

    MvarMVFree(MVConst[0]);
    MvarMVFree(MVConst[1]);
    MvarMVFree(MVConst[2]);
    IritFree(MVConst);

    /* Compensate and map A from [0, 1] to [-1, 1]: */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext)
	Pt -> Pt[2] = Pt -> Pt[2] * 2.0 - 1.0;

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the flecnodal curves on surface Srf = S(u, v).  Uses the        M
* univariate MV solver.			       				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute the flecnodal curves for.			     M
*   Step:	Stepsize in the curve-tracing stage.			     M
*   SubdivTol:  Accuracy of the subdivision stage of the approximation.      M
*   NumerTol:   Accuracy of numeric approx.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:  Flecnodal curves as polylines in (a, b, u, v) space.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarFlecnodalCrvsCreateMVs, MvarSrfFlecnodalCrvs                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfFlecnodalCrvs2                                                    M
*****************************************************************************/
MvarPolyStruct *MvarSrfFlecnodalCrvs2(const CagdSrfStruct *Srf, 
				      CagdRType Step, 
				      CagdRType SubdivTol, 
				      CagdRType NumerTol)
{
    MvarPolyStruct *FlecCrv, *Poly;
    int i;
    MvarPtStruct *Pt;
    MvarMVStruct
	**MVs = MvarFlecnodalCrvsCreateMVCnstrnts(Srf);

    FlecCrv = MvarMVUnivarInter(MVs, Step, SubdivTol, NumerTol);

    for (i = 0; i < 3; i++)
        MvarMVFree(MVs[i]);	    	    
    IritFree(MVs);

    /* Compensate and map A from [0, 1] to [-1, 1]: */
    for (Poly = FlecCrv; Poly != NULL; Poly = Poly -> Pnext) {
	for (Pt = Poly -> Pl; Pt != NULL; Pt = Pt -> Pnext)
	    Pt -> Pt[2] = Pt -> Pt[2] * 2.0 - 1.0;
    }

    return FlecCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the flecnodal points on surface Srf = S(u, v).		     M
*   Denote by Suv the second order derivatives of S(u, v) and by Suvv the    M
* third order derivatives.  Then, solution of the following 4 equations in 4 M
* unknowns (u, v, a, b) provides the flecnodal points:			     M
*									     M
*             [ L M ] [ a ]						     V
* 1. [ a  b ] [     ] [   ] =       (This location/direction is asymp.)      V
*             [ M N ] [ b ]	    (L, M, N are the coef. of SFF.)	     V
*									     M
*                [ Suu Suv ]           [ a ]				     V
*     = [ a  b ] [         ] . n(u, v) [   ] = 0,			     V
*                [ Suv Svv ]           [ b ]				     V
*									     M
*    or									     M
*									     M
*    < a^2 Suu + 2ab Suv + b^2 Svv, n(u, v) > = 0.			     V
*									     M
*             (   [ Suuu Suuv ]       [ Svuu Svuv ] )            [ a ]       V
* 2. [ a  b ] ( a [           ]  +  b [           ] )  . n(u, v) [   ] = 0,  V
*             (   [ Suuv Suvv ]       [ Svuv Svvv ] )            [ b ]       V
*									     M
*		                   (A third derivative zero contact.)        M
*									     M
*    or									     M
*									     M
*    < a^3 Suuu + 3a^2b Suuv + 3ab^2 Suvv + b^3 Svvv, n(u, v) > = 0.	     V
*									     M
* 3. < a^4 Suuuu + 4 a^3b Suuuv + 6 a^2b^2 suuvv + 4ab^3 suvvv + b^4 Svvvv,  V
*                                                           n(u, v) > = 0.   V
*									     M
*		                   (A fourth derivative zero contact.)       M
*									     M
* 4. a * a + b * b = 1             (A normalization nconstraint.)	     V
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       To compute the silhouette higher orders' contact points.     M
*   SubdivTol:  Accuracy of the subdivision stage of the approximation.      M
*   NumerTol:   Accuracy of numeric approx.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Polylines on the unit sphere, depicting the	     M
*		 flecnodal's partitioning lines.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfTopoAspectGraph, SymbEvalSrfAsympDir,      M
*   MvarSrfSilhInflections						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfFlecnodalPts                                                      M
*****************************************************************************/
MvarPtStruct *MvarSrfFlecnodalPts(const CagdSrfStruct *CSrf,
				  CagdRType SubdivTol,
				  CagdRType NumerTol)
{
    CagdBType
	OldCagdInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
	OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType UMin, UMax, VMin, VMax, Translate[3];
    CagdCrvStruct *LinCrv;
    CagdSrfStruct *Srf, *DuSrf, *DvSrf, *Nrml, *Du2Srf, *DuvSrf, *Dv2Srf,
	*Du3Srf, *Du2vSrf, *Duv2Srf, *Dv3Srf, *Du4Srf, *Du3vSrf, *Du2v2Srf,
	*Duv3Srf, *Dv4Srf;
    MvarConstraintType Constraints[4];
    MvarMVStruct *A, *B, *A2, *AB, *B2, *A3, *AB2, *A2B, *B3,
	*A4, *AB3, *A2B2, *A3B, *B4, *LinMV,
        *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5, *MVTmp6, *MVConst[4];
    MvarPtStruct *Pts;

    IRIT_PT_RESET(Translate);

    if (CAGD_IS_BEZIER_SRF(CSrf))
        Srf = CagdCnvrtBzr2BspSrf(CSrf);
    else
        Srf = CagdSrfCopy(CSrf);

    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
    Nrml = SymbSrfCrossProd(DuSrf, DvSrf);
    Du2Srf = CagdSrfDerive(DuSrf, CAGD_CONST_U_DIR);
    DuvSrf = CagdSrfDerive(DuSrf, CAGD_CONST_V_DIR);
    Dv2Srf = CagdSrfDerive(DvSrf, CAGD_CONST_V_DIR);
    Du3Srf = CagdSrfDerive(Du2Srf, CAGD_CONST_U_DIR);
    Du2vSrf = CagdSrfDerive(DuvSrf, CAGD_CONST_U_DIR);
    Duv2Srf = CagdSrfDerive(DuvSrf, CAGD_CONST_V_DIR);
    Dv3Srf = CagdSrfDerive(Dv2Srf, CAGD_CONST_V_DIR);
    Du4Srf = CagdSrfDerive(Du3Srf, CAGD_CONST_U_DIR);
    Du3vSrf = CagdSrfDerive(Du2vSrf, CAGD_CONST_U_DIR);
    Du2v2Srf = CagdSrfDerive(Du2vSrf, CAGD_CONST_V_DIR);
    Duv3Srf = CagdSrfDerive(Duv2Srf, CAGD_CONST_V_DIR);
    Dv4Srf = CagdSrfDerive(Dv3Srf, CAGD_CONST_V_DIR);

    /* Construct A:[-1, 1], B:[0, 1] as third and fourth variables. */
    LinCrv = BspCrvNew(2, 2, CAGD_PT_E1_TYPE);
    BspKnotUniformOpen(2, 2, LinCrv -> KnotVector);
    LinCrv -> Points[1][0] = 0.0;
    LinCrv -> Points[1][1] = 1.0;
    LinMV = MvarCrvToMV(LinCrv);
    A = MvarPromoteMVToMV2(LinMV, 4, 2);

    BspKnotAffineTransOrder2(LinCrv -> KnotVector, LinCrv -> Order,
			     LinCrv -> Length + LinCrv -> Order, -1.0, 1.0);
    LinCrv -> Points[1][0] = -1.0;
    LinMV = MvarCrvToMV(LinCrv);
    B = MvarPromoteMVToMV2(LinMV, 4, 3);

    CagdCrvFree(LinCrv);
    MvarMVFree(LinMV);

    /* Make sure domain are the same. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    BspKnotAffineTrans2(A -> KnotVectors[0],
			A -> Lengths[0] + A -> Orders[0],
			UMin, UMax);
    BspKnotAffineTrans2(B -> KnotVectors[0],
			B -> Lengths[0] + B -> Orders[0],
			UMin, UMax);
    BspKnotAffineTrans2(A -> KnotVectors[1],
			A -> Lengths[1] + A -> Orders[1],
			VMin, VMax);
    BspKnotAffineTrans2(B -> KnotVectors[1],
			B -> Lengths[1] + B -> Orders[1],
			VMin, VMax);

    BspKnotAffineTrans2(A -> KnotVectors[3],
			A -> Lengths[3] + A -> Orders[3],
			-1.0, 1.0);

    /* Compute the products of A & B. */
    A2 = MvarMVMult(A, A);
    AB = MvarMVMult(A, B);
    B2 = MvarMVMult(B, B);

    A3 = MvarMVMult(A2, A);
    A2B = MvarMVMult(A2, B);
    AB2 = MvarMVMult(A, B2);
    B3 = MvarMVMult(B2, B);

    A4 = MvarMVMult(A3, A);
    A3B = MvarMVMult(A3, B);
    A2B2 = MvarMVMult(A2, B2);
    AB3 = MvarMVMult(A, B3);
    B4 = MvarMVMult(B3, B);

    MvarMVTransform(AB, NULL, 2.0);
    MvarMVTransform(A2B, NULL, 3.0);
    MvarMVTransform(AB2, NULL, 3.0);
    MvarMVTransform(A3B, NULL, 4.0);
    MvarMVTransform(A2B2, NULL, 6.0);
    MvarMVTransform(AB3, NULL, 4.0);
    MvarMVFree(A);
    MvarMVFree(B);	

    /* 1. The second-order asymptotic direction constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du2Srf, Nrml, A2);
    CagdSrfFree(Du2Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(DuvSrf, Nrml, AB);
    CagdSrfFree(DuvSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Dv2Srf, Nrml, B2);
    CagdSrfFree(Dv2Srf);

    MVTmp4 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVConst[0] = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    /* 2. The third-order flecnodal constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du3Srf, Nrml, A3);
    CagdSrfFree(Du3Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(Du2vSrf, Nrml, A2B);
    CagdSrfFree(Du2vSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Duv2Srf, Nrml, AB2);
    CagdSrfFree(Duv2Srf);
    MVTmp4 = MVarProjNrmlPrmt2MVScl(Dv3Srf, Nrml, B3);
    CagdSrfFree(Dv3Srf);

    MVTmp5 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    MVConst[1] = MvarMVAdd(MVTmp1, MVTmp5);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp5);

    /* 3. The fourth-order flecnodal constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du4Srf, Nrml, A4);
    CagdSrfFree(Du4Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(Du3vSrf, Nrml, A3B);
    CagdSrfFree(Du3vSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Du2v2Srf, Nrml, A2B2);
    CagdSrfFree(Du2v2Srf);
    MVTmp4 = MVarProjNrmlPrmt2MVScl(Duv3Srf, Nrml, AB3);
    CagdSrfFree(Duv3Srf);
    MVTmp5 = MVarProjNrmlPrmt2MVScl(Dv4Srf, Nrml, B4);
    CagdSrfFree(Dv4Srf);

    MVTmp6 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    MVTmp2 = MvarMVAdd(MVTmp1, MVTmp6);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp6);

    MVConst[2] = MvarMVAdd(MVTmp2, MVTmp5);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp5);

    /* 4. The normalization constraint. */
    MVConst[3] = MvarMVAdd(A2, B2);
    Translate[0] = -1;
    MvarMVTransform(MVConst[2], Translate, 1.0);

    /* Free auxiliary data. */
    CagdSrfFree(Nrml);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    MvarMVFree(A2);
    MvarMVFree(AB);
    MvarMVFree(B2);

    MvarMVFree(A3);
    MvarMVFree(AB2);
    MvarMVFree(A2B);
    MvarMVFree(B3);

    MvarMVFree(A4);
    MvarMVFree(AB3);
    MvarMVFree(A2B2);
    MvarMVFree(A3B);
    MvarMVFree(B4);

    BspMultComputationMethod(OldCagdInterpFlag);
    MvarBspMultComputationMethod(OldMvarInterpFlag);

    /* Solve the constraints. */
    Constraints[0] =
        Constraints[1] =
	    Constraints[2] =
	        Constraints[3] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVConst, Constraints, 4, SubdivTol, NumerTol);

    MvarMVFree(MVConst[0]);
    MvarMVFree(MVConst[1]);
    MvarMVFree(MVConst[2]);
    MvarMVFree(MVConst[3]);

    CagdSrfFree(Srf);

    return Pts;
}
