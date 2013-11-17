/******************************************************************************
* Crv_Krnl.c - computation of the kernel of a 2D curve.                       *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb 12.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the kernel of the given curve, or the points in the plane that  M
* have a line of sight with all the points of the curve.		     M
*   The curve is assumed to be closed and simple.			     M
*   Let the input curve be C(t) and let S(u, v) = (u, v), the XY plane.      M
*   Then, let F(u, v, t) = (C(t) - S(u, v)) x C'(t)    (only the Z component M
*                                                             of crossprod). M
*   The zero set of F projected over the XY plane defines all the domain     M
* that is NOT in the kernel of C(t).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Simple closed curve to compute its kernel.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The trivariate function F(u, v, t) whose zero set,     M
*	projected on the XY plane, is the domain that is not in the kernel.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvKernelSilhouette, MVarCrvGammaKernel                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarCrvKernel                                                            M
*****************************************************************************/
MvarMVStruct *MVarCrvKernel(const CagdCrvStruct *Crv)
{
    return MVarCrvGammaKernel(Crv, 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the gamma-kernel of the given curve, or the points in the plane M
* that have a line of sight (rotated gamma degrees) with all the points of   M
* the curve.								     M
*   The curve is assumed to be closed and simple.			     M
*   Let the input curve be C(t) and let S(u, v) = (u, v), the XY plane.      M
*   Further let D'(t) = Rot(Gamma)[C'(t)].				     M
*   Then, let F(u, v, t) = (C(t) - S(u, v)) x D'(t)    (only the Z component M
*                                                             of crossprod). M
*   									     M
*   The zero set of F projected over the XY plane defines all the domain     M
* that is NOT in the gamma-kernel of C(t).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Simple closed curve to compute its gamma-kernel.                  M
*   Gamma: Angular deviation of the gamma-kernel, in degrees.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The trivariate function F(u, v, t) whose zero set,     M
*	projected on the XY plane, is the domain that is not in the 	     M
*	gamma-kernel.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvKernelSilhouette, MVarCrvKernel, MVarCrvGammaKernelSrf            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarCrvGammaKernel                                                       M
*****************************************************************************/
MvarMVStruct *MVarCrvGammaKernel(const CagdCrvStruct *Crv, CagdRType Gamma)
{
    CagdMType Mat;
    CagdBBoxStruct BBox;
    CagdCrvStruct *CpCrv;
    CagdSrfStruct *SPlane;
    MvarMVStruct *MVCrv, *MVDCrv, *MVPlane, **MVSplit, *MVTmp1, *MVTmp2;

    /* Make sure curve has open end condition. */
    if (CAGD_IS_BEZIER_CRV(Crv))
	Crv = CpCrv = CagdCnvrtBzr2BspCrv(Crv);
    else if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (!BspCrvHasOpenEC(Crv)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtFloat2OpenCrv(Crv);

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = TCrv;
    }

    /* Build a planar surface that spans beyond the curve's domain. */
    CagdCrvBBox(Crv, &BBox);
    SPlane = CagdPrimPlaneSrf(BBox.Min[0] - 1.0,
			      BBox.Min[1] - 1.0,
			      BBox.Max[0] + 1.0,
			      BBox.Max[1] + 1.0, 0);

    /* Convert the curve to a trivariate as third parameter. */
    MVTmp1 = MvarCrvToMV(Crv);
    MVCrv = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);
    BspKnotAffineTransOrder2(MVCrv -> KnotVectors[2], MVCrv -> Orders[2],
			     MVCrv -> Orders[2] + MVCrv -> Lengths[2],
			     0.0, 1.0);

    /* Convert the planar surface to a trivariate as two first parameters. */
    MVTmp1 = MvarSrfToMV(SPlane);
    CagdSrfFree(SPlane);
    MVPlane = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);

    MVDCrv = MvarMVDerive(MVCrv, 2);
    MatGenMatRotZ1(IRIT_DEG2RAD(Gamma), Mat);
    MvarMVMatTransform(MVDCrv, Mat);

    MvarMakeMVsCompatible(&MVCrv, &MVPlane, TRUE, TRUE);
    MVTmp1 = MvarMVSub(MVCrv, MVPlane);
    MvarMVFree(MVCrv);
    MvarMVFree(MVPlane);

    MVTmp2 = MvarMVCrossProd(MVTmp1, MVDCrv);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVDCrv);
    MVSplit = MvarMVSplitScalar(MVTmp2);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVSplit[1]);
    MvarMVFree(MVSplit[2]);

    MVTmp1 = MVSplit[3];

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    BspKnotAffineTransOrder2(MVTmp1 -> KnotVectors[0], MVTmp1 -> Orders[0],
			     MVTmp1 -> Lengths[0] + MVTmp1 -> Orders[0],
			     BBox.Min[0] - 1.0, BBox.Max[0] + 1.0);
    BspKnotAffineTransOrder2(MVTmp1 -> KnotVectors[1], MVTmp1 -> Orders[1],
			     MVTmp1 -> Lengths[1] + MVTmp1 -> Orders[1],
			     BBox.Min[1] - 1.0, BBox.Max[1] + 1.0);

    return MVTmp1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a gamma-kernel surfaces the given curve and as a function of  M
* gamma.								     M
*   Let the input curve be C(t).					     M
*   If C(t) is a linear curve then we compute the surface:		     M
*       Let P = (Px, Py) be the initial point and (Dx, Dy) the slope of the  M
*	line.  Then the constructed surface equals (ES == ExtentScale):	     M
*       X(r, Gamma) = Px + r * Dx * ES         + r * Gamma * Dy * ES	     M
*       Y(r, Gamma) = Py - r * Gamma * Dx * ES + r * Dy * ES                 M
*       Z(r, Gamma) = Gamma						     M
*   If C(t) is a higher order, non linear, curve then we compute trivar:     M
*       X(t, r, Gamma) = Cx(t) + r * Cx'(t) * ES   + r * Gamma * Cy'(t) * ES M
*       Y(t, r, Gamma) = Cy(t) - r * Gamma * Cx'(t) * ES + r * Cy'(t) * ES   M
*       Z(t, r, Gamma) = Gamma						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Simple curve to compute its gamma-kernel surface.          M
*   ExtentScale:  To scale the constructed surface as function of Gamma.     M
*   GammaMax:     Max gamma deviation of the gamma-kernel, in degrees, for   M
*		  this curve.  If negative, does so to opposite direction.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The surface S(u, v) or trivariate function F(u, v, t)  M
*       representing the gamma surface as a function of gamma.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvKernelSilhouette, MVarCrvKernel, MVarCrvGammaKernel               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarCrvGammaKernelSrf                                                    M
*****************************************************************************/
MvarMVStruct *MVarCrvGammaKernelSrf(const CagdCrvStruct *Crv,
				    CagdRType ExtentScale,
				    CagdRType GammaMax)
{
    CagdCrvStruct *LinCrv;
    MvarMVStruct *MV;

    /* In a rotation (+ scaling) matrix of the form			    */
    /* [1   Gamma]							    */
    /* [         ]							    */
    /* [-Gamma  1]							    */
    /* Gamma equal "atan(Gamma)" - multiply all elements of this matrix by  */
    /* cos(Gamma) to be convinced.					    */
    GammaMax = tan(IRIT_DEG2RAD(GammaMax));

    if (Crv -> Order == 2) {
        CagdRType Dist;
	CagdPType Pt1, Pt2, Pt, Dir;
	CagdCrvStruct *GCrv;
	CagdSrfStruct *LinSrf, *GSrf, *GammaSrf;

	CagdCoerceToE2(Pt1, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE2(Pt2, Crv -> Points, 1, Crv -> PType);
	
	if (GammaMax > 0) {
	    IRIT_PT2D_COPY(Pt, Pt1);
	    IRIT_VEC2D_SUB(Dir, Pt2, Pt1);
	}
	else {
	    IRIT_PT2D_COPY(Pt, Pt2);
	    IRIT_VEC2D_SUB(Dir, Pt1, Pt2);
	}
	Dist = IRIT_VEC2D_LENGTH(Dir);
	IRIT_VEC2D_NORMALIZE(Dir);

	/* Construct a linear curve from zero to GammaMax. */
	GCrv = BzrCrvNew(2, CAGD_PT_E3_TYPE);
	GCrv -> Points[1][0] = Dir[0];
	GCrv -> Points[1][1] = Dir[0] + GammaMax * Dir[1];
	GCrv -> Points[2][0] = Dir[1];
	GCrv -> Points[2][1] = Dir[1] - GammaMax * Dir[0];
	GCrv -> Points[3][0] = 0.0;
	GCrv -> Points[3][1] = IRIT_FABS( GammaMax );
	GSrf = CagdPromoteCrvToSrf(GCrv, CAGD_CONST_U_DIR);
	CagdCrvFree(GCrv);

	/* Construct a linear curve parametrizing the lines. */
	LinCrv = BzrCrvNew(2, CAGD_PT_E3_TYPE);
	LinCrv -> Points[1][0] = -ExtentScale;
	LinCrv -> Points[1][1] = Dist + ExtentScale;
	LinCrv -> Points[2][0] = -ExtentScale;
	LinCrv -> Points[2][1] = Dist + ExtentScale;
	LinCrv -> Points[3][0] = 1.0;
	LinCrv -> Points[3][1] = 1.0;
	LinSrf = CagdPromoteCrvToSrf(LinCrv, CAGD_CONST_V_DIR);
	CagdCrvFree(LinCrv);
	GammaSrf = SymbSrfMult(GSrf, LinSrf);
	CagdSrfFree(GSrf);
	CagdSrfFree(LinSrf);

	Pt[2] = 0.0;
	CagdSrfTransform(GammaSrf, Pt, 1.0);

	MV = MvarSrfToMV(GammaSrf);
	CagdSrfFree(GammaSrf);

	return MV;
    }
    else {
        CagdCrvStruct
	    *DCrv = CagdCrvDerive(Crv);
	MvarMVStruct *DMV, **DMVs, *TmpMV1, *TmpMV2, *TmpMV3, *MVGamma, *MVr,
	    *MVScalars[MVAR_MAX_PT_SIZE];

	TmpMV1 = MvarCrvToMV(DCrv);
	CagdCrvFree(DCrv);
	DMV = MvarPromoteMVToMV2(TmpMV1, 3, 0);
	MvarMVFree(TmpMV1);
	DMVs = MvarMVSplitScalar(DMV);
	MvarMVFree(DMV);

	/* Create the gamma parameter curve. */
	LinCrv = BzrCrvNew(2, CAGD_PT_E1_TYPE);
	LinCrv -> Points[1][0] = 0.0;
	LinCrv -> Points[1][1] = GammaMax;

	TmpMV1 = MvarCrvToMV(LinCrv);
	CagdCrvFree(LinCrv);
	MVGamma = MvarPromoteMVToMV2(TmpMV1, 3, 1);
	MvarMVFree(TmpMV1);

	TmpMV3 = MvarMVMult(MVGamma, DMVs[2]);
	TmpMV1 = MvarMVAdd(DMVs[1], TmpMV3);
	MvarMVFree(TmpMV3);

	TmpMV3 = MvarMVMult(MVGamma, DMVs[1]);
	TmpMV2 = MvarMVSub(DMVs[2], TmpMV3);
	MvarMVFree(TmpMV3);

	MVAR_FREE_SCALARS(DMVs);

	/* Create the parameter r along the line. */
	LinCrv = BzrCrvNew(2, CAGD_PT_E1_TYPE);
	LinCrv -> Points[1][0] = -ExtentScale;
	LinCrv -> Points[1][1] = ExtentScale;

	TmpMV3 = MvarCrvToMV(LinCrv);
	CagdCrvFree(LinCrv);
	MVr = MvarPromoteMVToMV2(TmpMV3, 3, 2);
	MvarMVFree(TmpMV3);

	MVAR_CLEAR_SCALARS(MVScalars);
	MVScalars[1] = MvarMVMult(TmpMV1, MVr);
	MvarMVFree(TmpMV1);
	MVScalars[2] = MvarMVMult(TmpMV2, MVr);
	MvarMVFree(TmpMV2);
	MvarMVFree(MVr);

	/* Z is always in the positive Z direction. */
	MVGamma -> Points[1][1] = IRIT_FABS(MVGamma -> Points[1][1]);
	MVScalars[3] = MVGamma;

	TmpMV3 = MvarMVMergeScalar(MVScalars);

	/* Translate the (tangent) line to the curve's position. */
	TmpMV1 = MvarCrvToMV(Crv);
	TmpMV2 = MvarPromoteMVToMV2(TmpMV1, 3, 0);
	MvarMVFree(TmpMV1);
	MV = MvarMVAdd(TmpMV2, TmpMV3);
	MvarMVFree(TmpMV2);
	MvarMVFree(TmpMV3);

	MVAR_FREE_SCALARS(MVScalars);

        return MV;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the kernel of the given curve, or the points in the plane that  M
* have a line of sight with all the points of the curve.		     M
*   The curve is assumed to be closed and simple.			     M
*   Let the input curve be C(t) and let S(u, v) = (u, v), the XY plane.      M
*   Then, let f(u, v, t) = < C(t) - S(u, v), C'(t) >.                        M
*   The zero set of f projected over the XY plane defines all the domain     M
* that is NOT in the kernel of C(t).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     Simple closed curve to compute the silhouette of the kernel.    M
*   Gamma:   Angular deviation of the gamma-kernel, in degrees.              M
*   SubEps:  Subdivision epsilon.					     M
*   NumEps:  Numeric marching tolerance.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The silhouette along the third, t, parameter of the  M
	trivariate function f(u, v, t).					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvKernel, MVarCrvGammaKernel                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarCrvKernelSilhouette                                                  M
*****************************************************************************/
IPObjectStruct *MVarCrvKernelSilhouette(const CagdCrvStruct *Crv,
					CagdRType Gamma,
					CagdRType SubEps,
					CagdRType NumEps)
{
    int i = 0;
    IrtRType
	R = 0.0;
    MvarConstraintType Constraints[2];
    MvarMVStruct *MVVec[2];
    MvarPtStruct *MVPts, *MVPt;
    IPObjectStruct
	*ObjPtList = IPGenLISTObject(NULL);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVVec[0] = MVarCrvGammaKernel(Crv, Gamma);
    MVVec[1] = MvarMVDerive(MVVec[0], 2);
    MVPts = MvarMVsZeros(MVVec, Constraints, 2, SubEps, NumEps);
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);

    /* Convert the computed points to our form. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	IPListObjectInsert(ObjPtList, i++,
			   IPGenPTObject(&MVPt -> Pt[0],
					 &MVPt -> Pt[1],
					 &R));
    }
    IPListObjectInsert(ObjPtList, i, NULL);
    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal and maximal diameter of the given curve.            M
*   Let the input curve be C(t) and let f(t,r) = < C(t)-C(r), C(t)-C(r) >.   M
*   Then, df/dt = df/dr = 0 find all the finite set of line segments that    M
* connects two points on the curve orthogonaly to the curve.		     M
*   The min/max diameter is part of this set.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     Simple closed curve to compute the diameter for.                M
*   SubEps:  Subdivision epsilon.					     M
*   NumEps:  Numeric marching tolerance.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of pair of parameter values between which the   M
*			local diameter could be found.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvKernel, MvarCrvAntipodalPoints                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarCrvDiameter                                                          M
*****************************************************************************/
IPObjectStruct *MVarCrvDiameter(const CagdCrvStruct *Crv,
				CagdRType SubEps,
				CagdRType NumEps)
{
    int i = 0;
    IrtRType Eps, Dt, TMax, TMin,
	R = 0.0;
    CagdCrvStruct *CpCrv;
    MvarConstraintType Constraints[2];
    MvarMVStruct *MVVec[2], *MVCrv1, *MVCrv2, *MVTmp1, *MVTmp2;
    MvarPtStruct *MVPts, *MVPt;
    IPObjectStruct
	*ObjPtList = IPGenLISTObject(NULL);

    /* Make sure curve has open end condition. */
    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (!BspCrvHasOpenEC(Crv)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtFloat2OpenCrv(Crv);

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = TCrv;
    }

    /* Convert the curve to a bivariate in first and second parameters. */
    MVTmp1 = MvarCrvToMV(Crv);
    MVCrv1 = MvarPromoteMVToMV2(MVTmp1, 2, 0);
    MVCrv2 = MvarPromoteMVToMV2(MVTmp1, 2, 1);
    MvarMVFree(MVTmp1);
    MVTmp1 = MvarMVSub(MVCrv1, MVCrv2);
    MVTmp2 = MvarMVDotProd(MVTmp1, MVTmp1);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVVec[0] = MvarMVDerive(MVTmp2, 0);
    MVVec[1] = MvarMVDerive(MVTmp2, 1);
    MvarMVFree(MVTmp2);
    MVPts = MvarMVsZeros(MVVec, Constraints, 2, SubEps, NumEps);
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);

    /* Convert the computed points to our form. */
    Eps = IRIT_FABS(NumEps) * 10.0;
    CagdCrvDomain(Crv, &TMin, &TMax);
    Dt = TMax - TMin;
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	if (MVPt -> Pt[1] > MVPt -> Pt[0] &&
	    !IRIT_APX_EQ_EPS(MVPt -> Pt[0], MVPt -> Pt[1], Eps) &&
	    !IRIT_APX_EQ_EPS(MVPt -> Pt[0], MVPt -> Pt[1] - Dt, Eps))	    
	    IPListObjectInsert(ObjPtList, i++,
			       IPGenPTObject(&MVPt -> Pt[0],
					     &MVPt -> Pt[1],
					     &R));
    }
    IPListObjectInsert(ObjPtList, i, NULL);
    MvarPtFreeList(MVPts);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return ObjPtList;
}
