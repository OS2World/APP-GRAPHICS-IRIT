/******************************************************************************
* MvBisect.c - Compute bisectors of multivariates.   			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "geom_lib.h"
#include "mvar_loc.h"

static MvarMVStruct **MvarCrvCrvBisector2DCreateMVs(CagdCrvStruct *Crv1,
						    CagdCrvStruct *Crv2, 
						    CagdRType *BBoxMin,
						    CagdRType *BBoxMax);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute bisector to two given multivariates.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:    The two multivariates to compute the bisector for.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    The result bisector.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfPtBisectorSrf3D, SymbCrvPtBisectorSrf3D, SymbCrvCrvBisectorSrf3D  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsBisector, bisectors                                               M
*****************************************************************************/
MvarMVStruct *MvarMVsBisector(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    MvarMVStruct
	*MVBisect = NULL;

    /* Find the case in hand and dispatch to the proper routine. */
    if (MV1 -> Dim == 1 && MV2 -> Dim == 1) {           /* Crv-Crv bisector. */
	if (MVAR_NUM_OF_MV_COORD(MV1) != 3 && MVAR_NUM_OF_MV_COORD(MV2) != 3) {
	    MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	    return NULL;
	}
	else {
	    CagdCrvStruct
		*Crv1 = MvarMVToCrv(MV1),
		*Crv2 = MvarMVToCrv(MV2);
	    CagdSrfStruct
		*Srf = SymbCrvCrvBisectorSrf3D(Crv1, Crv2, 0.5);

	    CagdCrvFree(Crv1);
	    CagdCrvFree(Crv2);

	    MVBisect = MvarSrfToMV(Srf);
	    CagdSrfFree(Srf);
	}
    }
    else if ((MV1 -> Dim == 1 && MV2 -> Dim == 2) ||
	     (MV1 -> Dim == 2 && MV2 -> Dim == 1)) {    /* Crv-Srf bisector. */
	if (MV1 -> Dim == 2 && MV2 -> Dim == 1) {
	    MvarMVStruct
		*MVTmp1 = MvarCrvSrfBisector(MV1, MV2),
		*MVTmp2 = MvarMVReverse(MVTmp1, 0, 2);

	    MVBisect =  MvarMVReverse(MVTmp2, 1, 2);

	    MvarMVFree(MVTmp1);
	    MvarMVFree(MVTmp2);
	}
	else
	    MVBisect = MvarCrvSrfBisector(MV1, MV2);
    }
    else if (MV1 -> Dim == 2 && MV2 -> Dim == 2) {      /* Srf-Srf bisector. */
	MVBisect = MvarSrfSrfBisector(MV1, MV2);
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    return MVBisect;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisectors of a curve and a surface in R^4.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1:	The univariate (curve) in R^4.                               M
*   CMV2:	The bivariate (surface) in R^4.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The resulting bisector.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsBisector, MvarSrfSrfBisector, MvarCrvSrfBisectorApprox            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvSrfBisector, bisectors                                            M
*****************************************************************************/
MvarMVStruct *MvarCrvSrfBisector(const MvarMVStruct *CMV1,
				 const MvarMVStruct *CMV2)
{
    int i, j;
    MvarMVStruct *MVTmp, *MVTmp2, *A[4][4], *B[4], *MVs[MVAR_MAX_PT_SIZE],
	**MVSplit, *MVBisect, *MV1, *MV2;

    if (MVAR_NUM_OF_MV_COORD(CMV1) != 4 && MVAR_NUM_OF_MV_COORD(CMV2) != 4) {
	MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    if (CMV1 -> GType != CMV2 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_SAME_GTYPE_EXPECTED);
	return NULL;
    }

    /* Bring both the curve and the surface into a trivariate form. */
    if (CMV1 -> Dim == 1 && CMV2 -> Dim == 2) {         /* Crv-Srf bisector. */
	CagdRType Min, Max;

	MV1 = MvarPromoteMVToMV2(CMV1, 3, 0);       /* Trivariate at axis 0. */
	MV2 = MvarPromoteMVToMV2(CMV2, 3, 1);     /* Trivariate at axes 1,2. */

	/* Make sure domain are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    MvarMVDomain(MV1, &Min, &Max, 0);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[0],
				MV2 -> Lengths[0] + MV2 -> Orders[0],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 1);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[1],
				MV1 -> Lengths[1] + MV1 -> Orders[1],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 2);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[2],
				MV1 -> Lengths[2] + MV1 -> Orders[2],
				Min, Max);
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    /* Compute the derivative of the curve. */
    MVTmp = MvarMVDerive(MV1, 0);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 4; i++)
	A[0][i] = MVSplit[i + 1];

    B[0] = MvarMVDotProd(MVTmp, MV1);
    MvarMVFree(MVTmp);

    /* Compute the partial derivatives of the surface. */
    MVTmp = MvarMVDerive(MV2, 1);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 4; i++)
	A[1][i] = MVSplit[i + 1];

    B[1] = MvarMVDotProd(MVTmp, MV2);
    MvarMVFree(MVTmp);


    MVTmp = MvarMVDerive(MV2, 2);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 4; i++)
	A[2][i] = MVSplit[i + 1];

    B[2] = MvarMVDotProd(MVTmp, MV2);
    MvarMVFree(MVTmp);

    /* Compute the distance constraint. */
    MVTmp = MvarMVSub(MV1, MV2);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 4; i++)
	A[3][i] = MVSplit[i + 1];

    MVTmp2 = MvarMVAdd(MV1, MV2);
    MvarMVTransform(MVTmp2, NULL, 0.5);
    B[3] = MvarMVDotProd(MVTmp, MVTmp2);
    MvarMVFree(MVTmp);
    MvarMVFree(MVTmp2);

    /* Done with preparations - compute solution of the 4x4 linear system. */
    MVAR_CLEAR_SCALARS(MVs);
    MVs[0] = MvarMVDeterminant4(A[0][0], A[0][1], A[0][2], A[0][3],
				A[1][0], A[1][1], A[1][2], A[1][3],
				A[2][0], A[2][1], A[2][2], A[2][3],
				A[3][0], A[3][1], A[3][2], A[3][3]);
    MVs[1] = MvarMVDeterminant4(B[0],    A[0][1], A[0][2], A[0][3],
				B[1],    A[1][1], A[1][2], A[1][3],
				B[2],    A[2][1], A[2][2], A[2][3],
				B[3],    A[3][1], A[3][2], A[3][3]);
    MVs[2] = MvarMVDeterminant4(A[0][0], B[0],    A[0][2], A[0][3],
				A[1][0], B[1],    A[1][2], A[1][3],
				A[2][0], B[2],    A[2][2], A[2][3],
				A[3][0], B[3],    A[3][2], A[3][3]);
    MVs[3] = MvarMVDeterminant4(A[0][0], A[0][1], B[0],    A[0][3],
				A[1][0], A[1][1], B[1],    A[1][3],
				A[2][0], A[2][1], B[2],    A[2][3],
				A[3][0], A[3][1], B[3],    A[3][3]);
    MVs[4] = MvarMVDeterminant4(A[0][0], A[0][1], A[0][2], B[0],
				A[1][0], A[1][1], A[1][2], B[1],
				A[2][0], A[2][1], A[2][2], B[2],
				A[3][0], A[3][1], A[3][2], B[3]);

    for (i = 0; i < 4; i++) {
	MvarMVFree(B[i]);

	for (j = 0; j < 4; j++)
	    MvarMVFree(A[i][j]);
    }

    MVBisect = MvarMVMergeScalar(MVs);

    MVAR_FREE_SCALARS(MVs);

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return MVBisect;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisectors of two surfaces in R^5.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2:	The two bivariates (surfaces) in R^5.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The resulting bisector.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsBisector, MvarCrvSrfBisector, MvarSrfSrfBisectorApprox            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfBisector, bisectors                                            M
*****************************************************************************/
MvarMVStruct *MvarSrfSrfBisector(const MvarMVStruct *CMV1,
				 const MvarMVStruct *CMV2)
{
    int i, j;
    MvarMVStruct *MVTmp, *MVTmp2, *A[5][5], *B[5], *MVs[MVAR_MAX_PT_SIZE],
	**MVSplit, *MVBisect, *MV1, *MV2;

    if (MVAR_NUM_OF_MV_COORD(CMV1) != 5 && MVAR_NUM_OF_MV_COORD(CMV2) != 5) {
	MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    if (CMV1 -> GType != CMV2 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_SAME_GTYPE_EXPECTED);
	return NULL;
    }

    /* Bring both surfaces into a four-variate form. */
    if (CMV1 -> Dim == 2 && CMV2 -> Dim == 2) {
	CagdRType Min, Max;

	MV1 = MvarPromoteMVToMV2(CMV1, 4, 0);  /* Four variate at axes 0,1. */
	MV2 = MvarPromoteMVToMV2(CMV2, 4, 2);  /* Four variate at axes 2,3. */

	/* Make sure domain are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    MvarMVDomain(MV1, &Min, &Max, 0);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[0],
				MV2 -> Lengths[0] + MV2 -> Orders[0],
				Min, Max);
	    MvarMVDomain(MV1, &Min, &Max, 1);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[1],
				MV2 -> Lengths[1] + MV2 -> Orders[1],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 2);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[2],
				MV1 -> Lengths[2] + MV1 -> Orders[2],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 3);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[3],
				MV1 -> Lengths[3] + MV1 -> Orders[3],
				Min, Max);
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    /* Compute the partial derivatives of the first surface. */
    MVTmp = MvarMVDerive(MV1, 0);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 5; i++)
	A[0][i] = MVSplit[i + 1];

    B[0] = MvarMVDotProd(MVTmp, MV1);
    MvarMVFree(MVTmp);


    MVTmp = MvarMVDerive(MV1, 1);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 5; i++)
	A[1][i] = MVSplit[i + 1];

    B[1] = MvarMVDotProd(MVTmp, MV1);
    MvarMVFree(MVTmp);

    /* Compute the partial derivatives of the second surface. */
    MVTmp = MvarMVDerive(MV2, 2);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 5; i++)
	A[2][i] = MVSplit[i + 1];

    B[2] = MvarMVDotProd(MVTmp, MV2);
    MvarMVFree(MVTmp);


    MVTmp = MvarMVDerive(MV2, 3);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 5; i++)
	A[3][i] = MVSplit[i + 1];

    B[3] = MvarMVDotProd(MVTmp, MV2);
    MvarMVFree(MVTmp);

    /* Compute the distance constraint. */
    MVTmp = MvarMVSub(MV1, MV2);
    MVSplit = MvarMVSplitScalar(MVTmp);
    for (i = 0; i < 5; i++)
	A[4][i] = MVSplit[i + 1];

    MVTmp2 = MvarMVAdd(MV1, MV2);
    MvarMVTransform(MVTmp2, NULL, 0.5);
    B[4] = MvarMVDotProd(MVTmp, MVTmp2);
    MvarMVFree(MVTmp);
    MvarMVFree(MVTmp2);

    /* Done with preparations - compute solution of the 4x4 linear system. */
    MVAR_CLEAR_SCALARS(MVs);
    MVs[0] = MvarMVDeterminant5(A[0][0], A[0][1], A[0][2], A[0][3], A[0][4],
				A[1][0], A[1][1], A[1][2], A[1][3], A[1][4],
				A[2][0], A[2][1], A[2][2], A[2][3], A[2][4],
				A[3][0], A[3][1], A[3][2], A[3][3], A[3][4],
				A[4][0], A[4][1], A[4][2], A[4][3], A[4][4]);
    MVs[1] = MvarMVDeterminant5(B[0],    A[0][1], A[0][2], A[0][3], A[0][4],
				B[1],    A[1][1], A[1][2], A[1][3], A[1][4],
				B[2],    A[2][1], A[2][2], A[2][3], A[2][4],
				B[3],    A[3][1], A[3][2], A[3][3], A[3][4],
				B[4],    A[4][1], A[4][2], A[4][3], A[4][4]);
    MVs[2] = MvarMVDeterminant5(A[0][0], B[0],    A[0][2], A[0][3], A[0][4],
				A[1][0], B[1],    A[1][2], A[1][3], A[1][4],
				A[2][0], B[2],    A[2][2], A[2][3], A[2][4],
				A[3][0], B[3],    A[3][2], A[3][3], A[3][4],
				A[4][0], B[4],    A[4][2], A[4][3], A[4][4]);
    MVs[3] = MvarMVDeterminant5(A[0][0], A[0][1], B[0],    A[0][3], A[0][4],
				A[1][0], A[1][1], B[1],    A[1][3], A[1][4],
				A[2][0], A[2][1], B[2],    A[2][3], A[2][4],
				A[3][0], A[3][1], B[3],    A[3][3], A[3][4],
				A[4][0], A[4][1], B[4],    A[4][3], A[4][4]);
    MVs[4] = MvarMVDeterminant5(A[0][0], A[0][1], A[0][2], B[0],    A[0][4],
				A[1][0], A[1][1], A[1][2], B[1],    A[1][4],
				A[2][0], A[2][1], A[2][2], B[2],    A[2][4],
				A[3][0], A[3][1], A[3][2], B[3],    A[3][4],
				A[4][0], A[4][1], A[4][2], B[4],    A[4][4]);
    MVs[0] = MvarMVDeterminant5(A[0][0], A[0][1], A[0][2], A[0][3], B[0],
				A[1][0], A[1][1], A[1][2], A[1][3], B[1],
				A[2][0], A[2][1], A[2][2], A[2][3], B[2],
				A[3][0], A[3][1], A[3][2], A[3][3], B[3],
				A[4][0], A[4][1], A[4][2], A[4][3], B[4]);

    for (i = 0; i < 5; i++) {
	MvarMVFree(B[i]);

	for (j = 0; j < 5; j++)
	    MvarMVFree(A[i][j]);
    }

    MVBisect = MvarMVMergeScalar(MVs);

    MVAR_FREE_SCALARS(MVs);

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return MVBisect;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an approximation to the bisector of a curve and a surface.      M
* Let C(t) be the parametric curve and T(t) its unnormalized tangent field.  M
* Let S(u,v) be the parametric surface and n(u, v) its unnormalized normal.  M
* Then:									     M
*									     M
*	< P - C(t), T(t) > = 0    defines the normal plane of T(t),	     V
*									     M
* and the solution of							     M
*									     M
*       < S(u, v) + n(u, v) Alpha - C(t), T(t) > = 0			     V
*									     M
* finds the intersection point of of the normal of the surface with the      M
* normal plane of the curve.  Then,					     M
*									     M
*                   < C(t) - S(u, v), T(t) >				     V
*  Alpha(u, v, t) = ------------------------				     V
*                       < n(u, v), T(t) >				     V
*									     M
* We now can define this intersection point, P, as			     M
*									     M
*  P(u, v, t) = S(u, v) + Alpha(u, v, t) n(u, v)			     M
*									     M
* and end up with a single function we must extract its zero set	     M
*									     M
*  < C(t) - P(u, v, t), C(t) - P(u, v, t) > -				     V
*                    < S(u, v) - P(u, v, t), S(u, v) - P(u, v, t) > = 0      V
*									     M
* or									     M
*									     M
*  < C(t) - P(u, v, t), C(t) - P(u, v, t) > -				     V
*                    < Alpha(u, v, t) n(u, v), Alpha(u, v, t) n(u, v) > = 0  V
*									     M
* Finding the zero set of the last equation provides the correspondance      M
* between the (u, v) location and the surface and (t) locations on the curve M
* that serve as mutual foot point for some bisector point.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1:	The univariate (curve) in R^3.                               M
*   CMV2:	The bivariate (surface) in R^3.                              M
*   OutputType: Expected output type:					     M
*		1. For the computed multivariate constraints.		     M
*		2. For the computed point cloud on the bisector.	     M
*		3. Points in a form of (u, v, x, y, z) where (u, v) are      M
*		   the parameter space of the surface.			     M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Following OutputType, either a set of multivariates (as a    M
*		linked list of MvarMVStruct), or a cloud of points on the    M
*		bisector (as a linked list of MvarPtStruct).		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsBisector, MvarSrfSrfBisector, MvarCrvSrfBisector                  M
*   MvarSrfSrfBisectorApprox						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvSrfBisectorApprox, bisectors                                      M
*****************************************************************************/
VoidPtr MvarCrvSrfBisectorApprox(const MvarMVStruct *CMV1,
				 const MvarMVStruct *CMV2,
				 int OutputType,
				 CagdRType SubdivTol,
				 CagdRType NumericTol)
{
    IRIT_STATIC_DATA MvarConstraintType
	Constraint = MVAR_CNSTRNT_ZERO;
    MvarMVStruct *MVTmp1, *MVTmp2, *MVTmp3, *Alpha, *P,
	*DMV1, *NrmlMV2, *Final, *MV1, *MV2;
    MvarPtStruct *Pts, *Pt, *NewPts;

    if (MVAR_NUM_OF_MV_COORD(CMV1) != 3 && MVAR_NUM_OF_MV_COORD(CMV2) != 3) {
	MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    if (CMV1 -> GType != CMV2 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_SAME_GTYPE_EXPECTED);
	return NULL;
    }

    /* Bring both the curve and the surface into a trivariate form. */
    if (CMV1 -> Dim == 1 && CMV2 -> Dim == 2) {         /* Crv-Srf bisector. */
	CagdRType Min, Max;

	MV1 = MvarPromoteMVToMV2(CMV1, 3, 0);       /* Trivariate at axis 0. */
	MV2 = MvarPromoteMVToMV2(CMV2, 3, 1);     /* Trivariate at axes 1,2. */

	/* Make sure domains are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    MvarMVDomain(MV1, &Min, &Max, 0);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[0],
				MV2 -> Lengths[0] + MV2 -> Orders[0],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 1);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[1],
				MV1 -> Lengths[1] + MV1 -> Orders[1],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 2);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[2],
				MV1 -> Lengths[2] + MV1 -> Orders[2],
				Min, Max);
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    DMV1 = MvarMVDerive(MV1, 0);
    MVTmp1 = MvarMVDerive(MV2, 1);
    MVTmp2 = MvarMVDerive(MV2, 2);
    NrmlMV2 = MvarMVCrossProd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    /* Compute Alpha(u, v, t). */
    MVTmp1 = MvarMVSub(MV1, MV2);
    MVTmp2 = MvarMVDotProd(MVTmp1, DMV1);          /* We have the numerator! */
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarMVDotProd(NrmlMV2, DMV1);       /* We have the denominator! */

    if (MVAR_IS_RATIONAL_MV(MVTmp1) || MVAR_IS_RATIONAL_MV(MVTmp2)) {
        MvarMVStruct **ScalarMVs, *MVs[MVAR_MAX_PT_SIZE],
	    *MVTmp1W, *MVTmp1X, *MVTmp2W, *MVTmp2X;

	ScalarMVs = MvarMVSplitScalar(MVTmp1);
	MVTmp1W = ScalarMVs[0];
	MVTmp1X = ScalarMVs[1];
	ScalarMVs = MvarMVSplitScalar(MVTmp2);
	MVTmp2W = ScalarMVs[0];
	MVTmp2X = ScalarMVs[1];

	if (MVTmp1W != NULL) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVTmp1W, MVTmp2X);

	    MvarMVFree(MVTmp2X);
	    MVTmp2X = MVTmp;
	}
	if (MVTmp2W != NULL) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVTmp2W, MVTmp1X);

	    MvarMVFree(MVTmp1X);
	    MVTmp1X = MVTmp;
	}
	MvarMVFree(MVTmp1W);
	MvarMVFree(MVTmp2W);

	MVAR_CLEAR_SCALARS(MVs);
	MVs[0] = MVTmp1X;
	MVs[1] = MVTmp2X;

        Alpha = MvarMVMergeScalar(MVs);

	MvarMVFree(MVTmp1X);
	MvarMVFree(MVTmp2X);
    }
    else {
        MvarMVStruct *MVs[MVAR_MAX_PT_SIZE];

	MVAR_CLEAR_SCALARS(MVs);
	MVs[0] = MVTmp1;
	MVs[1] = MVTmp2;

        Alpha = MvarMVMergeScalar(MVs);
    }
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    /* Compute S(u, v) + Alpha(u, v, t) n(u, v). */
    MVTmp1 = MvarMVMultScalar(NrmlMV2, Alpha);
    P = MvarMVAdd(MV2, MVTmp1);
    MvarMVFree(MVTmp1);

    /* Compute the final function, we seek its zero set. */
    MVTmp1 = MvarMVSub(MV1, P);
    MVTmp2 = MvarMVDotProd(MVTmp1, MVTmp1);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarMVSub(MV2, P);
    MVTmp3 = MvarMVDotProd(MVTmp1, MVTmp1);
    MvarMVFree(MVTmp1);

    Final = MvarMVSub(MVTmp2, MVTmp3);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);

    MvarMVFree(DMV1);
    MvarMVFree(NrmlMV2);
    MvarMVFree(Alpha);

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    if (OutputType == 1) {
        /* Place "S(u, v) + Alpha(u, v, t) n(u, v)" as "MVEuclid" attrib. */
        AttrSetPtrAttrib(&Final -> Attr, "MVEuclid", P);
	return Final;
    }

    /* Compute the Zero set for Final. */
    Pts = MvarMVsZeros(&Final, &Constraint, 1, SubdivTol, NumericTol);
    MvarMVFree(Final);

    /* Evaluate these points at P for the points on the bisector surface. */
    NewPts = NULL;
    while (Pts != NULL) {
	IrtRType *R;

	IRIT_LIST_POP(Pt, Pts);

	R = MvarMVEval(P, Pt -> Pt);
	if (OutputType == 2) {
	    CagdCoerceToE3(Pt -> Pt, &R, -1, (CagdPointType) P -> PType);
	    Pt -> Dim = 3;
	}
	else { /* OutputType == 3 */
	    /* Allocate a longer vector to hold (u, v, x, y, z): */
	    Pt = MvarPtRealloc(Pt, 5);
	    Pt -> Pt[0] = Pt -> Pt[1];
	    Pt -> Pt[1] = Pt -> Pt[2];
	    CagdCoerceToE3(&Pt -> Pt[2], &R, -1, (CagdPointType) P -> PType);
	}

	IRIT_LIST_PUSH(Pt, NewPts);
    }

    MvarMVFree(P);

    return CagdListReverse(NewPts);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an approximation to the bisector of two surfaces. 		     M
* Let S1(u, v) and S2(r, s) be two parametric surfaces and let n1(u, v) and  M
* n2(r, s) be their unnormalized normal fields.				     M
*   Becuase the two normals of the two surfaces must be coplanar we          M
* introduce the following constraint, forcing the three vectors n1, n2, and  M
* S1 - S2 to all be in the same plane.					     M
*									     M
*  < ( S1(u, v) - S2(r, s) ) x n1(u, v), n2(r, s) > = 0.		     V
*									     M
*  To make sure the distance to the intersection point of the normals, from  M
* both surface's foot points we also coerces these three vectors to form a   M
* isosceles triangle:							     M
*									     M
*  || n2(r, s) ||^2 < S1(u, v) - S2(r, s), n1(u, v) > ^ 2 -		     V
*                 || n1(r, s) ||^2 < S1(u, v) - S2(r, s), n2(u, v) > ^ 2     V
*									     M
* Finding the zero set of the last equation provides the correspondance      M
* between the (u, v) location and the first surface and (r, s) locations on  M
* the second surface that serve as mutual foot point for some bisector       M
* point.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2:	The two bivariates (surfaces) in R^3.                        M
*   OutputType: Expected output type:					     M
*		1. For the computed multivariate constraints.		     M
*		2. For the computed point cloud on the bisector.	     M
*		3. Points in a form of (u1, v2, x, y, z) where (u1, v1) are  M
*		   the parameter space of the first surface.		     M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Following OutputType, either a set of multivariates (as a    M
*		linked list of MvarMVStruct), or a cloud of points on the    M
*		bisector (as a linked list of MvarPtStruct).		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsBisector, MvarCrvSrfBisector, MvarSrfSrfBisector                  M
*   MvarCrvSrfBisectorApprox						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfBisectorApprox, bisectors                                      M
*****************************************************************************/
VoidPtr MvarSrfSrfBisectorApprox(const MvarMVStruct *CMV1,
				 const MvarMVStruct *CMV2,
				 int OutputType,
				 CagdRType SubdivTol,
				 CagdRType NumericTol)
{
    IRIT_STATIC_DATA MvarConstraintType
	Constraints[2] = { MVAR_CNSTRNT_ZERO, MVAR_CNSTRNT_ZERO };
    MvarMVStruct *MV1, *MV2, *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5,
	*NrmlMV1, *NrmlMV2, *AlphaNumer, *AlphaDenom, *Finals[2];
    MvarPtStruct *Pts, *Pt, *NewPts;

    if (MVAR_NUM_OF_MV_COORD(CMV1) != 3 && MVAR_NUM_OF_MV_COORD(CMV2) != 3) {
	MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    if (CMV1 -> GType != CMV2 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_SAME_GTYPE_EXPECTED);
	return NULL;
    }

    /* Bring both surfaces into a four-variate form. */
    if (CMV1 -> Dim == 2 && CMV2 -> Dim == 2) {
	CagdRType Min, Max;

	MV1 = MvarPromoteMVToMV2(CMV1, 4, 0);   /* Four variate at axes 0,1. */
	MV2 = MvarPromoteMVToMV2(CMV2, 4, 2);   /* Four variate at axes 2,3. */

	/* Make sure domains are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    MvarMVDomain(MV1, &Min, &Max, 0);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[0],
				MV2 -> Lengths[0] + MV2 -> Orders[0],
				Min, Max);
	    MvarMVDomain(MV1, &Min, &Max, 1);
	    BspKnotAffineTrans2(MV2 -> KnotVectors[1],
				MV2 -> Lengths[1] + MV2 -> Orders[1],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 2);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[2],
				MV1 -> Lengths[2] + MV1 -> Orders[2],
				Min, Max);
	    MvarMVDomain(MV2, &Min, &Max, 3);
	    BspKnotAffineTrans2(MV1 -> KnotVectors[3],
				MV1 -> Lengths[3] + MV1 -> Orders[3],
				Min, Max);
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    /* Compute normal fields. */ 
    MVTmp1 = MvarMVDerive(MV1, 0);
    MVTmp2 = MvarMVDerive(MV1, 1);
    NrmlMV1 = MvarMVCrossProd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp4 = MvarMVDerive(MV2, 2);
    MVTmp5 = MvarMVDerive(MV2, 3);
    NrmlMV2 = MvarMVCrossProd(MVTmp4, MVTmp5);

    /* Compute Alpha(u, v, r, s) = <S1 - S2, S1 - S2> / -2<n1, S1 - S2> .*/
    MVTmp1 = MvarMVSub(MV1, MV2);
    AlphaNumer = MvarMVDotProd(MVTmp1, MVTmp1);
    MVTmp2 = MvarMVDotProd(NrmlMV1, MVTmp1);
    AlphaDenom = MvarMVScalarScale(MVTmp2, -2);
    MvarMVFree(MVTmp2);

    /* Compute Delta = -2 (S1 - S2) <n1, S1 - S2> + n1 <S1 - S2, S1 - S2>. */
    MVTmp2 = MvarMVMultScalar(MVTmp1, AlphaDenom);
    MVTmp3 = MvarMVMultScalar(NrmlMV1, AlphaNumer);
    MvarMVFree(MVTmp1);
    MVTmp1 = MvarMVAdd(MVTmp2, MVTmp3);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);

    /* Compute <Delta, dS2/du>, and <Delta, dS2/dv>. */
    Finals[0] = MvarMVDotProd(MVTmp1, MVTmp4);
    Finals[1] = MvarMVDotProd(MVTmp1, MVTmp5);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp4);
    MvarMVFree(MVTmp5);

    if (OutputType == 1) {
	MvarMVFree(AlphaDenom);
	MvarMVFree(AlphaNumer);
	MvarMVFree(NrmlMV1);
	MvarMVFree(NrmlMV2);
	MvarMVFree(MV1);
	MvarMVFree(MV2);
    
        Finals[0] -> Pnext = Finals[1];
	return Finals[0];
    }

    /* Compute the Zero set for Finals. */
    Pts = MvarMVsZeros(Finals, Constraints, 2, SubdivTol, NumericTol);

    MvarMVFree(Finals[0]);
    MvarMVFree(Finals[1]);

    /* Evaluate these points for the points on the bisector surface. */
    NewPts = NULL;
    while (Pts != NULL) {
	IrtRType *R, t1;
	IrtPtType Pt1E3;
	IrtVecType Nrml1E3;

	IRIT_LIST_POP(Pt, Pts);

	R = MvarMVEval(MV1, Pt -> Pt);
	CagdCoerceToE3(Pt1E3, &R, -1, (CagdPointType) MV1 -> PType);

	R = MvarMVEval(AlphaNumer, Pt -> Pt);
	t1 = R[1];
	R = MvarMVEval(AlphaDenom, Pt -> Pt);
	t1 /= R[1];

	R = MvarMVEval(NrmlMV1, Pt -> Pt);
	CagdCoerceToE3(Nrml1E3, &R, -1, (CagdPointType) NrmlMV1 -> PType);

	IRIT_PT_SCALE(Nrml1E3, t1);
	IRIT_PT_ADD(Pt1E3, Pt1E3, Nrml1E3);

	if (OutputType == 2) {
	    IRIT_PT_COPY(Pt -> Pt, Pt1E3);
	    Pt -> Dim = 3;
	}
	else { /* OutputType == 3 */
	    /* Allocate a longer vector to hold (u, v, x, y, z): */
	    Pt = MvarPtRealloc(Pt, 5);
	    IRIT_PT_COPY(&Pt -> Pt[2], Pt1E3);
	}

	IRIT_LIST_PUSH(Pt, NewPts);
    }

    MvarMVFree(AlphaDenom);
    MvarMVFree(AlphaNumer);
    MvarMVFree(NrmlMV1);
    MvarMVFree(NrmlMV2);
    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return CagdListReverse(NewPts);
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a polyn. system for computation of a bisector curve of two	     M
*   parametric curves Crv1, Crv2. The bisector (a locus of all centers of    M
*   bitangential circles) is considered to lie inside planar rectangle BBox. M
*									     *
* PARAMETERS:                                                                M
*   Crv1, Crv2:	        Planar curves to compute their bisector.	     M
*   BBoxMin, BBoxMax:   The bounding box, where the bisector is computed.    M
*									     *
* RETURN VALUE:                                                              M
*   MvarMVStruct **:  The multivar system, its solution is desired bisector  M
*			curve.						     M
*									     *
* KEYWORDS:                                                                  M
*   MvarCrvCrvBisector2DCreateMVs					     M
*****************************************************************************/
static MvarMVStruct **MvarCrvCrvBisector2DCreateMVs(CagdCrvStruct *Crv1,
						    CagdCrvStruct *Crv2, 
						    CagdRType *BBoxMin,
						    CagdRType *BBoxMax)
{
    MvarMVStruct **MVs, *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4,
	*MV1, *MV2, *MVDer1, *MVDer2, *MVBis, *MVVec[MVAR_MAX_PT_SIZE];
    CagdRType DomainMin[4], DomainMax[4], TMin, TMax;

    IRIT_ZAP_MEM(MVVec, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);  

    /* Defining the domain. */
    CagdCrvDomain(Crv1, &TMin, &TMax);
    DomainMin[0] = TMin;
    DomainMax[0] = TMax;

    CagdCrvDomain(Crv2, &TMin, &TMax);
    DomainMin[1] = TMin;
    DomainMax[1] = TMax;

    DomainMin[2] = BBoxMin[0];
    DomainMax[2] = BBoxMax[0];
    DomainMin[3] = BBoxMin[1];
    DomainMax[3] = BBoxMax[1];

    MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * 3);

    MVTmp1 = MvarCrvToMV(Crv1);
    MVTmp2 = MvarCrvToMV(Crv2);

    MV1 = MvarPromoteMVToMV2(MVTmp1, 4, 0);
    MV2 = MvarPromoteMVToMV2(MVTmp2, 4, 1);

    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MV1 = MvarMVSetAllDomains(MV1, DomainMin, DomainMax, TRUE);
    MV2 = MvarMVSetAllDomains(MV2, DomainMin, DomainMax, TRUE);

    MVDer1 = MvarMVDerive(MV1, 0);
    MVDer2 = MvarMVDerive(MV2, 1);

    /* Vector function MVBis = [0, 0, x, y]; a center of bitang. circle. */
    MVVec[1] = MvarBuildParamMV(4, 2, BBoxMin[0], BBoxMax[0]);
    MVVec[1] = MvarMVSetAllDomains(MVVec[1], DomainMin, DomainMax, TRUE);

    MVVec[2] = MvarBuildParamMV(4, 3, BBoxMin[1], BBoxMax[1]);
    MVVec[2] = MvarMVSetAllDomains(MVVec[2], DomainMin, DomainMax, TRUE);

    MVBis = MvarMVMergeScalar(MVVec);

    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /* 3 Constraints: */

    /* <Crv1 - Bis, Crv1'> = 0.  */
    MVTmp1 = MvarMVSub(MV1, MVBis);
    MVs[0] = MvarMVDotProd(MVTmp1, MVDer1);

    /* <Crv2 - Bis, Crv2'> = 0.  */
    MVTmp2 = MvarMVSub(MV2, MVBis);
    MVs[1] = MvarMVDotProd(MVTmp2, MVDer2);

    /* ||Crv1 - Bis|| = ||Crv2 - Bis||. */
    MVTmp3 = MvarMVDotProd(MVTmp1, MVTmp1);
    MVTmp4 = MvarMVDotProd(MVTmp2, MVTmp2);
    MVs[2] = MvarMVSub(MVTmp3, MVTmp4);
     
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);
    MvarMVFree(MVBis);

    return MVs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a bisector curve of two planar curves. Curves are assumed to    M
*   to lie in z = 0 plane. The bisector curve is computed inside BBox.	     M
*									     *
* PARAMETERS:                                                                M
*   Crv1, Crv2:	    Planar curves to compute their bisector.		     M
*   Step:			Stepsize for curve tracing.		     M
*   SubdivTol:	    The subdivision tolerance to use.			     M
*   NumericTol:	    The numerical tolerance to use.			     M
*   BBoxMin, BBoxMax: The bounding box, where the bisector is computed.	     M
*   SupportPrms:    TRUE to return a curve in E4 as (X, Y, y1, t2), FALSE    M
*		    to return a curve in E2 as (X, Y).			     M
*									     *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A (list of) piecewise linear curves that approximates  M
*		      the bisector curve.				     M
*									     *
* KEYWORDS:                                                                  M
*   MvarCrvCrvBisector2D						     M
*****************************************************************************/
CagdCrvStruct *MvarCrvCrvBisector2D(CagdCrvStruct *Crv1,
				    CagdCrvStruct *Crv2, 
				    CagdRType Step, 
				    CagdRType SubdivTol,
				    CagdRType NumericTol, 
				    CagdRType *BBoxMin,
				    CagdRType *BBoxMax,
				    CagdBType SupportPrms)
{
    int i;
    MvarMVStruct
	**MVs = MvarCrvCrvBisector2DCreateMVs(Crv1, Crv2, BBoxMin, BBoxMax);
    MvarPolyStruct
	*BisecPls = MvarMVUnivarInter(MVs, Step, SubdivTol, NumericTol);
    IPPolygonStruct *Pl;
    IPObjectStruct *PBis;
    CagdCrvStruct
        *RetCrvs = NULL;

    for (i = 0; i < 3; i++)
	MvarMVFree(MVs[i]);	    	    
    IritFree(MVs);
 
    PBis = MvarCnvrtMVPolysToIritPolys2(BisecPls, TRUE);
    MvarPolyFreeList(BisecPls);

    for (Pl = PBis -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        CagdCrvStruct *PLCrv;
	IPVertexStruct *V;
	int Len = IPVrtxListLen(Pl -> PVertex);
	CagdRType **Points;

	if (Len < 2)
	    continue;
 
	PLCrv = BspCrvNew(Len, 2, SupportPrms ? CAGD_PT_E4_TYPE
			  		      : CAGD_PT_E2_TYPE);
	Points = PLCrv -> Points;
	BspKnotUniformOpen(Len, 2, PLCrv -> KnotVector);

	for (V = Pl -> PVertex, i = 0; V != NULL; V = V -> Pnext, i++) {
	    CagdRType
	        *R = SymbBsctComputeInterMidPoint(Crv1, V -> Coord[0],
						  Crv2, V -> Coord[1]);

	    Points[1][i] = R[0];
	    Points[2][i] = R[1];
		
	    if (SupportPrms) {
		Points[3][i] = V -> Coord[0];
		Points[4][i] = V -> Coord[1];
	    }
	}

	IRIT_LIST_PUSH(PLCrv, RetCrvs);
    }
    IPFreeObject(PBis);

    return RetCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a set of MV constraints for the computation of a trisector curve M
*  of three objects (bspsline curves or surfaces) in R3. The trisector is    M
* considered to  lie inside a spatial domain BBox.			     M
*									     *
* PARAMETERS:                                                                M
*   FF1, FF2, FF3:     Freeform objects (curve or surface) to compute        M
*		       their trisector.					     M
*   BBoxMin, BBoxMax:  The bounding box, where the trisector is computed.    M
*   Eqns:	       Will be updated with the number of MVs constraints.   M
*									     *
* RETURN VALUE:                                                              M
*   MvarMVStruct **:  The multivar system, its solution is the desired       M
*		      trisector curve in R3.				     M
*									     *
* KEYWORDS:                                                                  M
*   MvarTrisector3DCreateMVs						     M
*****************************************************************************/
MvarMVStruct **MvarTrisector3DCreateMVs(VoidPtr FF1, 
					VoidPtr FF2,
					VoidPtr FF3,
					CagdRType *BBoxMin,
					CagdRType *BBoxMax,
					int *Eqns)
{
    MvarMVStruct **MVs, *MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5, *MVTmp6,
        *MVVs[3], *MV1dt, *MV1du, *MV2dt, *MV2du, *MV3dt, *MV3du, *MVTmp, 
        *MVTris, *MVVec[MVAR_MAX_PT_SIZE];
    int i, NumOfVars, NumOfEqns,
	ShiftIndx = 0;
    CagdRType DomainMin[9], DomainMax[9], TMin, TMax, UMin, UMax;
    CagdBType FFIsCrv[3];
    VoidPtr Ptrs[3];

    Ptrs[0] = FF1;
    Ptrs[1] = FF2;
    Ptrs[2] = FF3;

    IRIT_ZAP_MEM(MVVec, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);  

    /* Defining the domain. */
    for (i = 0; i < 3; i++) {
	const CagdCrvStruct
	    *Crv = (const CagdCrvStruct *) Ptrs[i];
	const CagdSrfStruct
	    *Srf = (const CagdSrfStruct *) Ptrs[i];

	switch (Crv -> GType) {
	    case CAGD_CBEZIER_TYPE: 
	    case CAGD_CBSPLINE_TYPE:
	    case CAGD_CPOWER_TYPE:
		FFIsCrv[i] = TRUE;
		CagdCrvDomain(Crv, &TMin, &TMax);
		DomainMin[3 + i + ShiftIndx] = TMin;
		DomainMax[3 + i + ShiftIndx] = TMax;
		break;
	    case CAGD_SBEZIER_TYPE:
	    case CAGD_SBSPLINE_TYPE:
	    case CAGD_SPOWER_TYPE:
		FFIsCrv[i] = FALSE;
		CagdSrfDomain(Srf, &TMin, &TMax, &UMin, &UMax);
		DomainMin[3 + i + ShiftIndx] = TMin;
		DomainMax[3 + i + ShiftIndx] = TMax;
		DomainMin[3 + i + 1 + ShiftIndx] = UMin;
		DomainMax[3 + i + 1 + ShiftIndx] = UMax;
		ShiftIndx++;
		break;
	    default:
		assert(0);
		break;
	}
    }

    /* Defining the domain. */
    DomainMin[0] = BBoxMin[0];
    DomainMax[0] = BBoxMax[0];
    DomainMin[1] = BBoxMin[1];
    DomainMax[1] = BBoxMax[1];
    DomainMin[2] = BBoxMin[2];
    DomainMax[2] = BBoxMax[2];

    NumOfVars = 6 + ShiftIndx;
    NumOfEqns = 5 + ShiftIndx;
    *Eqns = NumOfEqns;
    ShiftIndx = 0;

    MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * NumOfEqns);

    for (i = 0; i < 3; i++) {
	if (FFIsCrv[i]) {
	    MVTmp = MvarCrvToMV((const CagdCrvStruct *) Ptrs[i]);
	    MVVs[i] = MvarPromoteMVToMV2(MVTmp, NumOfVars, 3 + i + ShiftIndx);
	}
	else {
	    MVTmp = MvarSrfToMV((const CagdSrfStruct *) Ptrs[i]);
	    MVVs[i] = MvarPromoteMVToMV2(MVTmp, NumOfVars, 3 + i + ShiftIndx);
	    ShiftIndx++;
	}
	MVVs[i] = MvarMVSetAllDomains(MVVs[i], DomainMin, DomainMax, TRUE);
	MvarMVFree(MVTmp);
    }

    ShiftIndx = 0;

    /* Partial Crv/Srf derivatives. */
    MV1dt = MvarMVDerive(MVVs[0], 3);
    if (!FFIsCrv[0]) {
	MV1du = MvarMVDerive(MVVs[0], 4);
	ShiftIndx++;
    }
    else
	MV1du = NULL;

    MV2dt = MvarMVDerive(MVVs[1], 3 + 1 + ShiftIndx);
    if (!FFIsCrv[1]) {
	MV2du = MvarMVDerive(MVVs[1], 3 + 2 + ShiftIndx);
	ShiftIndx++;
    }
    else
	MV2du = NULL;

    MV3dt = MvarMVDerive(MVVs[2], 3 + 2 + ShiftIndx);
    if (!FFIsCrv[2]) {
	MV3du = MvarMVDerive(MVVs[2], 3 + 3 + ShiftIndx);
	ShiftIndx++;
    }
    else
	MV3du = NULL;

    /* Vector function MVTris = [x, y, z, ...]; a center of contact sphere. */
    MVVec[1] = MvarBuildParamMV(NumOfVars, 0, BBoxMin[0], BBoxMax[0]);
    MVVec[1] = MvarMVSetAllDomains(MVVec[1], DomainMin, DomainMax, TRUE);
    MVVec[2] = MvarBuildParamMV(NumOfVars, 1, BBoxMin[1], BBoxMax[1]);
    MVVec[2] = MvarMVSetAllDomains(MVVec[2], DomainMin, DomainMax, TRUE);
    MVVec[3] = MvarBuildParamMV(NumOfVars, 2, BBoxMin[2], BBoxMax[2]);
    MVVec[3] = MvarMVSetAllDomains(MVVec[3], DomainMin, DomainMax, TRUE);

    for (i = 4; i < MVAR_MAX_PT_SIZE; i++) {    
	MvarMVFree(MVVec[i]);
    }
    MVTris = MvarMVMergeScalar(MVVec);

    /* Constraints: */
    ShiftIndx = 0;

    /* <Crv1/Srf1 - Tris, Crv1'/Srf1'> = 0.  */
    MVTmp1 = MvarMVSub(MVVs[0], MVTris);
    MVs[0] = MvarMVDotProd(MVTmp1, MV1dt);
    if (!FFIsCrv[0]) {
	MVs[1] = MvarMVDotProd(MVTmp1, MV1du);
	ShiftIndx++;
    }

    MVTmp2 = MvarMVSub(MVVs[1], MVTris);
    MVs[1 + ShiftIndx] = MvarMVDotProd(MVTmp2, MV2dt);
    if (!FFIsCrv[1]) {
	MVs[2 + ShiftIndx] = MvarMVDotProd(MVTmp2, MV2du);
	ShiftIndx++;
    }

    MVTmp3 = MvarMVSub(MVVs[2], MVTris);
    MVs[2 + ShiftIndx] = MvarMVDotProd(MVTmp3, MV3dt);
    if (!FFIsCrv[2]) {
	MVs[3 + ShiftIndx] = MvarMVDotProd(MVTmp3, MV3du);
	ShiftIndx++;
    }

    MVTmp4 = MvarMVDotProd(MVTmp1, MVTmp1);
    MVTmp5 = MvarMVDotProd(MVTmp2, MVTmp2);
    MVTmp6 = MvarMVDotProd(MVTmp3, MVTmp3);

    /* ||Crv/Srf1 - Tris|| = ||Crv/Srf2 - Tris||. */    
    MVs[3 + ShiftIndx] = MvarMVSub(MVTmp4, MVTmp5);

    /* ||Crv/Srf2 - Tris|| = ||Crv/Srf3 - Tris||. */  
    MVs[4 + ShiftIndx] = MvarMVSub(MVTmp5, MVTmp6);

    for (i = 0; i < 3; i++)
	MvarMVFree(MVVs[i]);
   
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);
    MvarMVFree(MVTmp4);
    MvarMVFree(MVTmp5);
    MvarMVFree(MVTmp6);

    MvarMVFree(MV1dt);
    MvarMVFree(MV2dt);
    MvarMVFree(MV3dt);
    if (!FFIsCrv[0])
	MvarMVFree(MV1du);
    if (!FFIsCrv[1])
	MvarMVFree(MV2du);
    if (!FFIsCrv[2])
	MvarMVFree(MV3du);

    return MVs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a trisector curve of three objects (curves or surfaces).	     M
*   The trisector is considered to lie inside a spatial domain BBox.	     M
*									     *
* PARAMETERS:                                                                M
*   FF1, FF2, FF3:  Objects to compute their trisector.			     M
*   Step:	    Stepsize for curve tracing.				     M
*   SubdivTol:	    The subdivision tolerance to use.			     M
*   NumericTol:	    The numerical tolerance to use.			     M
*   BBoxMin, BBoxMax: The bounding box, where the trisector is computed.     M
*									     *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:	A (list of) polyline(s) that approximates	     M
*			the trisector curve.				     M
*									     *
* KEYWORDS:                                                                  M
*   MvarTrisectorCrvs							     M
*****************************************************************************/
MvarPolyStruct *MvarTrisectorCrvs(VoidPtr FF1,
				  VoidPtr FF2,
				  VoidPtr FF3,
				  CagdRType Step, 
				  CagdRType SubdivTol,
				  CagdRType NumericTol,
				  CagdRType *BBoxMin,
				  CagdRType *BBoxMax)
{
    MvarMVStruct **MVs;
    MvarPolyStruct *TrisecCrv;
    int i, Eqns;

    MVs = MvarTrisector3DCreateMVs(FF1, FF2, FF3, BBoxMin,
				   BBoxMax, &Eqns);
    TrisecCrv = MvarMVUnivarInter(MVs, Step, SubdivTol, NumericTol);

    for (i = 0; i < Eqns; i++)
	MvarMVFree(MVs[i]);	    	    
    IritFree(MVs);

    return TrisecCrv;
}
