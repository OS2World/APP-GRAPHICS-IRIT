/******************************************************************************
* Mv_Crvtr.c - curvatrue analysis of freeforms.			              *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 04.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the radial curvature lines on surface Srf viewed from ViewDir.  M
* This amounts to finding the asymptotic directions of Srf that are in also  M
* the projection of ViewDir onto the tangent plane.			     M
*   Let a and b be two parameters that prescribe the tanget space.   Then,   M
* the solution is derived as the solution of the following three equations:  M
*									     M
*        dS     dS						 	     V
*  < ( a -- + b -- ) x N, V > = 0,  (the tangent direction is the projection V
*        du     dv		     of V onto the tangent plane.) 	     V
*									     M
*             [ L M ] [ a ]						     V
*    [ a  b ] [     ] [   ] = 0,    (This location/direction is asymp.)      V
*             [ M N ] [ b ]	    (L, M, N are the coef. of SFF.)	     V
*									     M
* where a and b are normalized so					     M
*									     M
*     a^2 + b^2 = 1.							     V
*									     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       To compute the radial curvature lines for.		     M
*   ViewDir:    View direction to consider.				     M
*   SubdivTol:  Accuracy of the subdivision stage of the approximation.      M
*   NumerTol:   Accuracy of numeric approx.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Polylines in the parameter space, depicting the	     M
*		 radial curvature lines.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfTopoAspectGraph, SymbEvalSrfAsympDir       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfRadialCurvature                                                   M
*****************************************************************************/
MvarPtStruct *MvarSrfRadialCurvature(const CagdSrfStruct *CSrf,
				     const CagdVType ViewDir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol)
{
    CagdBType
	OldCagdInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
	OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType UMin, UMax, VMin, VMax, Translate[3];
    CagdCrvStruct *LinCrv;
    CagdSrfStruct *Srf, *DuSrf, *DvSrf, *Nrml, *Du2Srf, *DuvSrf, *Dv2Srf;
    MvarConstraintType Constraints[3];
    MvarMVStruct *A, *B, *A2, *AB, *B2, *LinMV,
        *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVConst[3];
    MvarPtStruct *Pts, *Pt;

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

    /* Construct A:[-1, 1], B:[-1, 1] as third and fourth variables.       */
    /* Note we are having for A/B a domain of [0, 1] but range of [-1, 1]  */
    /* so we will need to compensate at the end...		           */
    LinCrv = BspCrvNew(2, 2, CAGD_PT_E1_TYPE);
    BspKnotUniformOpen(2, 2, LinCrv -> KnotVector);
    LinCrv -> Points[1][0] = -1.0;
    LinCrv -> Points[1][1] =  1.0;

    LinMV = MvarCrvToMV(LinCrv);
    A = MvarPromoteMVToMV2(LinMV, 4, 2);

    LinCrv -> Points[1][0] = 0.0;
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

    /* Compute the products of A & B. */
    A2 = MvarMVMult(A, A);
    AB = MvarMVMult(A, B);
    B2 = MvarMVMult(B, B);
    MvarMVTransform(AB, NULL, 2.0);

    /* 1. The second-order asymptotic direction constraint. */
    MVTmp1 = MvarSrfToMV(DuSrf),
    MVTmp2 = MvarPromoteMVToMV2(MVTmp1, 4, 0);
    MvarMVFree(MVTmp1);

    MVTmp3 = MvarSrfToMV(DuSrf),
    MVTmp4 = MvarPromoteMVToMV2(MVTmp3, 4, 0);
    MvarMVFree(MVTmp3);

    MVTmp1 = MvarMVMultScalar(MVTmp2, A);
    MvarMVFree(MVTmp2);

    MVTmp3 = MvarMVMultScalar(MVTmp4, B);
    MvarMVFree(MVTmp4);

    MVTmp2 = MvarMVAdd(MVTmp1, MVTmp3);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp3);

    MVTmp3 = MvarSrfToMV(Nrml),
    MVTmp4 = MvarPromoteMVToMV2(MVTmp3, 4, 0);
    MvarMVFree(MVTmp3);

    MVTmp1 = MvarMVCrossProd(MVTmp2, MVTmp4);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp4);

    MVConst[0] = MvarMVVecDotProd(MVTmp1, ViewDir);
    MvarMVFree(MVTmp1);

    /* 2. The second-order asymptotic direction constraint. */
    MVTmp1 = MVarProjNrmlPrmt2MVScl(Du2Srf, Nrml, A2);
    CagdSrfFree(Du2Srf);
    MVTmp2 = MVarProjNrmlPrmt2MVScl(DuvSrf, Nrml, AB);
    CagdSrfFree(DuvSrf);
    MVTmp3 = MVarProjNrmlPrmt2MVScl(Dv2Srf, Nrml, B2);
    CagdSrfFree(Dv2Srf);

    MVTmp4 = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVConst[1] = MvarMVAdd(MVTmp3, MVTmp4);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);

    /* 3. The normalization constraint. */
    MVConst[2] = MvarMVAdd(A2, B2);
    Translate[0] = -1;
    MvarMVTransform(MVConst[2], Translate, 1.0);

    /* Free auxiliary data. */
    CagdSrfFree(Nrml);

    MvarMVFree(A);
    MvarMVFree(B);	

    MvarMVFree(A2);
    MvarMVFree(AB);
    MvarMVFree(B2);

    BspMultComputationMethod(OldCagdInterpFlag);
    MvarBspMultComputationMethod(OldMvarInterpFlag);

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVConst, Constraints, 3, SubdivTol, NumerTol);

    MvarMVFree(MVConst[0]);
    MvarMVFree(MVConst[1]);
    MvarMVFree(MVConst[2]);

    CagdSrfFree(Srf);

    /* Compensate and map A/B from [0, 1] to [-1, 1]: */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	Pt -> Pt[2] = Pt -> Pt[2] * 2.0 - 1.0;
	Pt -> Pt[3] = Pt -> Pt[3] * 2.0 - 1.0;
    }

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a parametric curve, Crv, and a control point index CtlPtIdx,       M
* compute the curvature sign field of the curve as function of the Euclidean M
* locations of control point index CtlPtIdx.  Returned is a multivariate of  M
* dimension "1 + Dim(Crv)", where Dim(Crv) is the dimension of the curve     M
* (E2, E3, etc.).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute its curvature behaviour (convex vs. concave) as a  M
*	       function of the curve parameter and the Euclidean coordiate   M
*	       of the CtlPtIdx's control point.				     M
*   CtlPtIdx:  Index of control point to make a parameter for the curvature. M
*   Min, Max:  Domain each coordinate of CtlPtIdx point should vary.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The computed curvature field of Crv.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DCurvatureSign, MvarCrvMakeCtlPtParam, UserCrvCrvtrByOneCtlPt    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvCrvtrByOneCtlPt, curvature                                        M
*****************************************************************************/
MvarMVStruct *MvarCrvCrvtrByOneCtlPt(const CagdCrvStruct *Crv,
				     int CtlPtIdx,
				     CagdRType Min,
				     CagdRType Max)
{
    MvarMVStruct *MV1Deriv, *MV2Deriv, *MVTmp1, *MVTmp2,
	*MV1DerivScalars[MVAR_MAX_PT_SIZE], *MV2DerivScalars[MVAR_MAX_PT_SIZE],
        *MV = MvarCrvMakeCtlPtParam(Crv, CtlPtIdx, Min, Max);

    /* Now compute the curvature sign field as (derived with respect to t)  */
    /*      .  ..    .  ..						    */
    /*      X  Y  -  Y  X						    */

    MV1Deriv = MvarMVDerive(MV, 0);
    MV2Deriv = MvarMVDerive(MV1Deriv, 0);
    MVAR_SPLIT_SCALARS(MV1Deriv, MV1DerivScalars);
    MVAR_SPLIT_SCALARS(MV2Deriv, MV2DerivScalars);
    MvarMVFree(MV);
    MvarMVFree(MV1Deriv);
    MvarMVFree(MV2Deriv);

    MVTmp1 = MvarMVMult(MV1DerivScalars[1], MV2DerivScalars[2]);
    MVTmp2 = MvarMVMult(MV2DerivScalars[1], MV1DerivScalars[2]);
    MVAR_FREE_SCALARS(MV1DerivScalars);
    MVAR_FREE_SCALARS(MV2DerivScalars);
    MV = MvarMVSub(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    return MV;
}
