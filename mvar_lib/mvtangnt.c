/******************************************************************************
* MvTangnt.c - Compute bi-tangents and tri-tangents of freeform surfaces.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 97.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

#define SUBDIV_REL_DISTANCE_TOL		3
#define MVAR_CIRC_TAN_2CRVS_NUMER_TOL	-1e-10

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes bi-tangents of freeform bivariate.                              M
* Let,									     M
*									     M
*	DMV = MV1(u, v) - MV2(r, s)					     V
*									     M
* then, computed the simultaneous solution of the following three equations: M
*									     M
*   d MV1   d MV1  d MV2	                 			     V
* < ----- x -----, ----- > = 0,				                     V
*     du      dv     dr				                             V
*									     M
*   d MV1   d MV1  d MV2	                 			     V
* < ----- x -----, ----- > = 0,				                     V
*     du      dv     ds				                             V
*									     M
*   d MV1   d MV1		                 			     V
* < ----- x -----, DMV > = 0.				                     V
*     du      dv					                     V
*									     M
*   If an orientation" value 1 or -1 is prescribed, then only bitangent      M
* surfaces with similar (1) or opposite (-1) orientations are computed and   M
* returned.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2:   The two multivariates to compute the bi-tangents for.      M
*		  If MV2 == NULL, the self bi-tangents of MV1 are computed.  M
*   Orientation:  0 for no effect, -1 or +1 for a request to get opposite    M
*		  or similar normal orientation bi tangencies only.          M
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        M
*		  measured in the parametric space of the multivariates.     M
*   NumericTol:   Numeric tolerance of the numeric stage.  The numeric stage M
*		  is employed only if NumericTol < SubdivTol.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    Points on the bi-tangents of the two multivariates.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTangentToCrvAtTwoPts, MvarMVBiTangents2, MvarMVTriTangents           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVBiTangents, bi-tangent                                             M
*****************************************************************************/
MvarPtStruct *MvarMVBiTangents(const MvarMVStruct *CMV1,
			       const MvarMVStruct *CMV2,
			       int Orientation,
			       CagdRType SubdivTol,
			       CagdRType NumericTol)
{
    int i,
	Cnst = 0;
    CagdBType SelfBiTangent;
    IRIT_STATIC_DATA CagdVType
	Translate = { 0.0, 0.0, 0.0 };
    MvarMVStruct *MV1, *MV2, *DMV, *MVTmp1, *MVTmp2,
        *DuMV1, *DvMV1, *DuMV2, *DvMV2, *MVs[5];
    MvarConstraintType Constraints[5];
    MvarPtStruct *Solution;

    if (CMV2 == NULL) {					/* Do the self test. */
	CMV2 = CMV1;
	SelfBiTangent = TRUE;
    }
    else {
	SelfBiTangent = FALSE;
    }

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
	MV1 = MvarPromoteMVToMV2(CMV1, 4, 0);   /* Four variate at axes 0,1. */
	MV2 = MvarPromoteMVToMV2(CMV2, 4, 2);   /* Four variate at axes 2,3. */

	/* Make sure domain are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    int i;
	    CagdRType Min, Max;

	    for (i = 0; i < 2; i++) {
		MvarMVDomain(MV1, &Min, &Max, i);
		BspKnotAffineTrans2(MV2 -> KnotVectors[i],
				    MV2 -> Lengths[i] + MV2 -> Orders[i],
				    Min, Max);
	    }
	    for (i = 2; i < 4; i++) {
		MvarMVDomain(MV2, &Min, &Max, i);
		BspKnotAffineTrans2(MV1 -> KnotVectors[i],
				    MV1 -> Lengths[i] + MV1 -> Orders[i],
				    Min, Max);
	    }
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    /* Compute the partial derivatives of the surfaces. */
    DuMV1 = MvarMVDerive(MV1, 0);
    DvMV1 = MvarMVDerive(MV1, 1);
    DuMV2 = MvarMVDerive(MV2, 2);
    DvMV2 = MvarMVDerive(MV2, 3);

    MVTmp1 = MvarMVCrossProd(DuMV1, DvMV1);

    MVs[Cnst] = MvarMVDotProd(MVTmp1, DuMV2);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    MVs[Cnst] = MvarMVDotProd(MVTmp1, DvMV2);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    DMV = MvarMVSub(MV1, MV2);
    MVs[Cnst] = MvarMVDotProd(MVTmp1, DMV);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    if (Orientation) {
        MVTmp2 = MvarMVCrossProd(DuMV2, DvMV2);

	MVs[Cnst] = MvarMVDotProd(MVTmp1, MVTmp2);
	Constraints[Cnst++] = Orientation > 0 ? MVAR_CNSTRNT_POSITIVE
					      : MVAR_CNSTRNT_NEGATIVE;

	MvarMVFree(MVTmp2);
    }

    MvarMVFree(MVTmp1);
    MvarMVFree(DuMV1);
    MvarMVFree(DvMV1);
    MvarMVFree(DuMV2);
    MvarMVFree(DvMV2);

    /* If this is a self bi-tangent test, make sure diagonal results are     */
    /* purged away by adding a fifth positive constraint for DMV distance.   */
    if (SelfBiTangent) {
	MVs[Cnst] = MvarMVDotProd(DMV, DMV);
	Translate[0] = -IRIT_SQR(SubdivTol * SUBDIV_REL_DISTANCE_TOL);
	MvarMVTransform(MVs[Cnst], Translate, 1.0);
	Constraints[Cnst++] = MVAR_CNSTRNT_POSITIVE;

	Solution = MvarMVsZeros(MVs, Constraints, Cnst, SubdivTol, NumericTol);
    }
    else {
	Solution = MvarMVsZeros(MVs, Constraints, Cnst, SubdivTol, NumericTol);
    }

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    MvarMVFree(DMV);
    for (i = 0; i < Cnst; i++)
        MvarMVFree(MVs[i]);

    return Solution;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes bi-tangents of freeform bivariate.                              M
* Let,									     M
*									     M
*	DMV = MV1(u, v) - MV2(r, s)					     V
*									     M
* then, computed the simultaneous solution of the following two equations:   M
*									     M
*   d MV1   d MV1		                 			     V
* < ----- x -----, DMV > = 0,				                     V
*     du      dv					                     V
*									     M
*   d MV2   d MV2		                 			     V
* < ----- x -----, DMV > = 0,				                     V
*     dr      ds					                     V
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2:   The two multivariates to compute the bi-tangents for.      M
*		  If MV2 == NULL, the self bi-tangents of MV1 are computed.  M
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        M
*		  measured in the parametric space of the multivariates.     M
*   NumericTol:   Numeric tolerance of the numeric stage.  The numeric stage M
*		  is employed only if NumericTol < SubdivTol.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    Points on the bi-tangents of the two multivariates.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTangentToCrvAtTwoPts, MvarMVBiTangents, MvarMVTriTangents            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVBiTangents2, bi-tangent                                            M
*****************************************************************************/
MvarPtStruct *MvarMVBiTangents2(const MvarMVStruct *CMV1,
				const MvarMVStruct *CMV2,
				CagdRType SubdivTol,
				CagdRType NumericTol)
{
    CagdBType SelfBiTangent;
    IRIT_STATIC_DATA CagdVType
	Translate = { 0.0, 0.0, 0.0 };
    MvarMVStruct *MV1, *MV2, *DMV, *MVTmp1, *MVTmp2, *MVTmp3, *MVs[3];
    MvarConstraintType Constraints[3];
    MvarPtStruct *Solution;

    if (CMV2 == NULL) {					/* Do the self test. */
	CMV2 = CMV1;
	SelfBiTangent = TRUE;
    }
    else {
	SelfBiTangent = FALSE;
    }

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
	MV1 = MvarPromoteMVToMV2(CMV1, 4, 0);   /* Four variate at axes 0,1. */
	MV2 = MvarPromoteMVToMV2(CMV2, 4, 2);   /* Four variate at axes 2,3. */

	/* Make sure domain are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    int i;
	    CagdRType Min, Max;

	    for (i = 0; i < 2; i++) {
		MvarMVDomain(MV1, &Min, &Max, i);
		BspKnotAffineTrans2(MV2 -> KnotVectors[i],
				    MV2 -> Lengths[i] + MV2 -> Orders[i],
				    Min, Max);
	    }
	    for (i = 2; i < 4; i++) {
		MvarMVDomain(MV2, &Min, &Max, i);
		BspKnotAffineTrans2(MV1 -> KnotVectors[i],
				    MV1 -> Lengths[i] + MV1 -> Orders[i],
				    Min, Max);
	    }
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    DMV = MvarMVSub(MV1, MV2);

    /* Compute the partial derivatives of the first surface. */
    MVTmp1 = MvarMVDerive(MV1, 0);
    MVTmp2 = MvarMVDerive(MV1, 1);
    MVTmp3 = MvarMVCrossProd(MVTmp1, MVTmp2);
    MVs[0] = MvarMVDotProd(MVTmp3, DMV);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);
    Constraints[0] = MVAR_CNSTRNT_ZERO;

    /* Compute the partial derivatives of the second surface. */
    MVTmp1 = MvarMVDerive(MV2, 2);
    MVTmp2 = MvarMVDerive(MV2, 3);
    MVTmp3 = MvarMVCrossProd(MVTmp1, MVTmp2);
    MVs[1] = MvarMVDotProd(MVTmp3, DMV);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);
    Constraints[1] = MVAR_CNSTRNT_ZERO;

    /* If this is a self bi-tangent test, make sure diagonal results are     */
    /* purged away by adding a third positive constraint for DMV distance.   */
    if (SelfBiTangent) {
	MVs[2] = MvarMVDotProd(DMV, DMV);
	Translate[0] = -IRIT_SQR(SubdivTol * SUBDIV_REL_DISTANCE_TOL);
	MvarMVTransform(MVs[2], Translate, 1.0);
	Constraints[2] = MVAR_CNSTRNT_POSITIVE;

	Solution = MvarMVsZeros(MVs, Constraints, 3, SubdivTol, NumericTol);
    }
    else {
	Solution = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumericTol);
    }

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    MvarMVFree(DMV);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    if (SelfBiTangent)
	MvarMVFree(MVs[2]);

    return Solution;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tri-tangents of freeform bivariate. In other words, compute the M
* tangent plane at three points to the surface(s).                           M
* Let,									     M
*									     M
*	DMV12 = MV1(u, v) - MV2(r, s)					     V
*	DMV13 = MV1(u, v) - MV3(x, y)					     V
*	DMV23 = MV2(r, s) - MV3(x, y)					     V
*									     M
* then, computed the simultaneous solution of the following six equations:   M
*									     M
*   d MV1   d MV1		                 			     V
* < ----- x -----, DMV12 > = 0,				                     V
*     du      dv					                     V
*									     M
*   d MV1   d MV1		                 			     V
* < ----- x -----, DMV13 > = 0,				                     V
*     du      dv					                     V
*									     M
*   d MV2   d MV2		                 			     V
* < ----- x -----, DMV23 > = 0,				                     V
*     dr      ds					                     V
*                                                                            *
*   d MV2   d MV2		                 			     V
* < ----- x -----, DMV12 > = 0,				                     V
*     dr      ds					                     V
*                                                                            *
*   d MV3   d MV3		                 			     V
* < ----- x -----, DMV13 > = 0,				                     V
*     dx      dy					                     V
*                                                                            *
*   d MV3   d MV3		                 			     V
* < ----- x -----, DMV23 > = 0,				                     V
*     dx      dy					                     V
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2, CMV3:  The 3 multivariates to compute the tri-tangents for.  M
*		   If MV2 == MV2 ==NULL, the self tri-tangents of MV1 are    M
*		   computed.						     M
*   Orientation:   0 for no effect, -1 or +1 for a request to get opposite   M
*		   or similar normal orientation bi tangencies only.         M
*   SubdivTol:     Tolerance of the subdivision process.  Tolerance is       M
*		   measured in the parametric space of the multivariates.    M
*   NumericTol:    Numeric tolerance of the numeric stage.  The numeric      M
*		   stage is employed only if NumericTol < SubdivTol.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    Points on the bi-tangents of the two multivariates.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTangentToCrvAtTwoPts, MvarMVBiTangents, MvarMVBiTangents2            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVTriTangents, tri-tangent                                           M
*****************************************************************************/
MvarPtStruct *MvarMVTriTangents(const MvarMVStruct *CMV1,
				const MvarMVStruct *CMV2,
				const MvarMVStruct *CMV3,
				int Orientation,
				CagdRType SubdivTol,
				CagdRType NumericTol)
{
    int i,
	Cnst = 0;
    CagdBType
	SelfTriTangent = FALSE;
    IRIT_STATIC_DATA CagdVType
	Translate = { 0.0, 0.0, 0.0 };
    MvarMVStruct *MV1, *MV2, *MV3, *DMV12, *DMV13, *DMV23,
	*MVTmp1, *MVTmp2, *MVs[11], *MV1Nrml, *MV2Nrml, *MV3Nrml;
    MvarConstraintType Constraints[11];
    MvarPtStruct *Solution;

    if (CMV2 == NULL && CMV3 == NULL) {			/* Do the self test. */
	CMV2 = CMV3 = CMV1;
	SelfTriTangent = TRUE;
    }
    if (CMV2 == NULL || CMV3 == NULL) {
	MVAR_FATAL_ERROR(MVAR_ERR_ONE_OR_THREE_EXPECTED);
	return NULL;
    }

    if (MVAR_NUM_OF_MV_COORD(CMV1) != 3 &&
	MVAR_NUM_OF_MV_COORD(CMV2) != 3 &&
	MVAR_NUM_OF_MV_COORD(CMV3) != 3) {
	MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    if (CMV1 -> GType != CMV2 -> GType || CMV1 -> GType != CMV3 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_SAME_GTYPE_EXPECTED);
	return NULL;
    }

    /* Bring all surfaces into a six-variate form. */
    if (CMV1 -> Dim == 2 && CMV2 -> Dim == 2 && CMV3 -> Dim == 2) {
	MV1 = MvarPromoteMVToMV2(CMV1, 6, 0);    /* Six variate at axes 0,1. */
	MV2 = MvarPromoteMVToMV2(CMV2, 6, 2);    /* Six variate at axes 2,3. */
	MV3 = MvarPromoteMVToMV2(CMV3, 6, 4);    /* Six variate at axes 4,5. */

	/* Make sure domain are the same. */
	if (MV1 -> GType == MVAR_BSPLINE_TYPE) {
	    int i;
	    CagdRType Min, Max;

	    for (i = 0; i < 2; i++) {
		MvarMVDomain(MV1, &Min, &Max, i);
		BspKnotAffineTrans2(MV2 -> KnotVectors[i],
				    MV2 -> Lengths[i] + MV2 -> Orders[i],
				    Min, Max);
		BspKnotAffineTrans2(MV3 -> KnotVectors[i],
				    MV3 -> Lengths[i] + MV3 -> Orders[i],
				    Min, Max);
	    }
	    for (i = 2; i < 4; i++) {
		MvarMVDomain(MV2, &Min, &Max, i);
		BspKnotAffineTrans2(MV1 -> KnotVectors[i],
				    MV1 -> Lengths[i] + MV1 -> Orders[i],
				    Min, Max);
		BspKnotAffineTrans2(MV3 -> KnotVectors[i],
				    MV3 -> Lengths[i] + MV3 -> Orders[i],
				    Min, Max);
	    }
	    for (i = 4; i < 6; i++) {
		MvarMVDomain(MV3, &Min, &Max, i);
		BspKnotAffineTrans2(MV1 -> KnotVectors[i],
				    MV1 -> Lengths[i] + MV1 -> Orders[i],
				    Min, Max);
		BspKnotAffineTrans2(MV2 -> KnotVectors[i],
				    MV2 -> Lengths[i] + MV2 -> Orders[i],
				    Min, Max);
	    }
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	return NULL;
    }

    DMV12 = MvarMVSub(MV1, MV2);
    DMV13 = MvarMVSub(MV1, MV3);
    DMV23 = MvarMVSub(MV2, MV3);

    /* Compute the partial derivatives of the first surface. */
    MVTmp1 = MvarMVDerive(MV1, 0);
    MVTmp2 = MvarMVDerive(MV1, 1);
    MV1Nrml = MvarMVCrossProd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MVs[Cnst] = MvarMVDotProd(MV1Nrml, DMV12);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;
    MVs[Cnst] = MvarMVDotProd(MV1Nrml, DMV13);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    /* Compute the partial derivatives of the second surface. */
    MVTmp1 = MvarMVDerive(MV2, 2);
    MVTmp2 = MvarMVDerive(MV2, 3);
    MV2Nrml = MvarMVCrossProd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MVs[Cnst] = MvarMVDotProd(MV2Nrml, DMV12);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;
    MVs[Cnst] = MvarMVDotProd(MV2Nrml, DMV23);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    /* Compute the partial derivatives of the third surface. */
    MVTmp1 = MvarMVDerive(MV3, 4);
    MVTmp2 = MvarMVDerive(MV3, 5);
    MV3Nrml = MvarMVCrossProd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);
    MVs[Cnst] = MvarMVDotProd(MV3Nrml, DMV13);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;
    MVs[Cnst] = MvarMVDotProd(MV3Nrml, DMV23);
    Constraints[Cnst++] = MVAR_CNSTRNT_ZERO;

    if (Orientation) {
	MVs[Cnst] = MvarMVDotProd(MV1Nrml, MV2Nrml);
	Constraints[Cnst++] = Orientation > 0 ? MVAR_CNSTRNT_POSITIVE
					      : MVAR_CNSTRNT_NEGATIVE;
	MVs[Cnst] = MvarMVDotProd(MV1Nrml, MV3Nrml);
	Constraints[Cnst++] = Orientation > 0 ? MVAR_CNSTRNT_POSITIVE
					      : MVAR_CNSTRNT_NEGATIVE;
    }

    MvarMVFree(MV1Nrml);
    MvarMVFree(MV2Nrml);
    MvarMVFree(MV3Nrml);

    /* If this is a self bi-tangent test, make sure diagonal results are     */
    /* purged away by adding a fifth positive constraint for DMV distance.   */
    if (SelfTriTangent) {
	MVs[Cnst    ] = MvarMVDotProd(DMV12, DMV12);
	MVs[Cnst + 1] = MvarMVDotProd(DMV13, DMV13);
	MVs[Cnst + 2] = MvarMVDotProd(DMV23, DMV23);
	Translate[0] = -IRIT_SQR(SubdivTol * SUBDIV_REL_DISTANCE_TOL);
	MvarMVTransform(MVs[Cnst    ], Translate, 1.0);
	MvarMVTransform(MVs[Cnst + 1], Translate, 1.0);
	MvarMVTransform(MVs[Cnst + 2], Translate, 1.0);
	Constraints[Cnst++] = MVAR_CNSTRNT_POSITIVE;
	Constraints[Cnst++] = MVAR_CNSTRNT_POSITIVE;
	Constraints[Cnst++] = MVAR_CNSTRNT_POSITIVE;

	Solution = MvarMVsZeros(MVs, Constraints, Cnst, SubdivTol, NumericTol);
    }
    else {
	Solution = MvarMVsZeros(MVs, Constraints, Cnst, SubdivTol, NumericTol);
    }

    MvarMVFree(MV1);
    MvarMVFree(MV2);
    MvarMVFree(MV3);

    MvarMVFree(DMV12);
    MvarMVFree(DMV13);
    MvarMVFree(DMV23);

    for (i = 0; i < Cnst; i++)
	MvarMVFree(MVs[i]);

    return Solution;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all circles of prescribed radius that are tangent to given two  M
* curves.								     M
*   Solves for circles' centers P(x, y), using the following four equations  M
* in four unknowns (x, y, r, t), and R is the desired circle radius:	     M
*	||C1(t) - P||^2 = R^2,						     V
*	||C2(r) - P||^2 = R^2,						     V
*	< C1(t) - P, C1'(t) > = 0,					     V
*	< C2(t) - P, C2'(t) > = 0.					     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2: The two curves to find the circles that is tangent to both.  M
*   Radius:     Of all the circle(s) that is tangent to Crv1/2.		     M
*   Tol:	Tolerance of approximation.  Subdiv Tol of MV Zeros.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    List of the 4-tuples as (r, t, x, y).		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCircTanTo2Crvs		                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCircTanTo2Crvs, bi-tangent                                           M
*****************************************************************************/
MvarPtStruct *MvarCircTanTo2Crvs(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2,
				 CagdRType Radius,
				 CagdRType Tol)
{
    int i;
    CagdRType DmnMin[4], DmnMax[4], Trans;
    CagdBBoxStruct BBox1, BBox2;
    CagdSrfStruct *CircCenterSrf;
    MvarMVStruct *MTmp, *MTmp1, *MTmp2, *MCrv1, *MCrv2, *MDCrv1, *MDCrv2,
        *MCircCenter, *MVs[4];
    MvarConstraintType Constraints[4];
    MvarPtStruct *Solution, *Pt;

    /* COmpute the region in the XY plane where the circlecenter can be     */
    /* as the expanded bbox pf the curves by (twice for safety) the radius. */
    CagdCrvBBox(Crv1, &BBox1);
    CagdCrvBBox(Crv2, &BBox2);

    for (i = 0; i < 2; i++) {
        if (BBox1.Min[i] > BBox2.Min[i])
	    BBox1.Min[i] = BBox2.Min[i] - Radius * 2.0;
	else
	    BBox1.Min[i] -= Radius * 2.0;

	if (BBox1.Max[i] < BBox2.Max[i])
	    BBox1.Max[i] = BBox2.Max[i] + Radius * 2.0;
	else
	    BBox1.Max[i] += Radius * 2.0;	
    }

    CircCenterSrf = CagdPrimPlaneSrf(BBox1.Min[0], BBox1.Min[1],
				     BBox1.Max[0], BBox1.Max[1], 0.0);

    MTmp = MvarCrvToMV(Crv1);  
    MCrv1 = MvarPromoteMVToMV2(MTmp, 4, 0);  
    MvarMVFree(MTmp);

    MTmp = MvarCrvToMV(Crv2);  
    MCrv2 = MvarPromoteMVToMV2(MTmp, 4, 1);  
    MvarMVFree(MTmp);

    MTmp = MvarSrfToMV(CircCenterSrf);  
    MCircCenter = MvarPromoteMVToMV2(MTmp, 4, 2);  
    MvarMVFree(MTmp);

    /* Make sure domains are consistent. */
    CagdCrvDomain(Crv1, &DmnMin[0], &DmnMax[0]);
    CagdCrvDomain(Crv2, &DmnMin[1], &DmnMax[1]);
    CagdSrfDomain(CircCenterSrf, &DmnMin[2], &DmnMax[2],
				&DmnMin[3], &DmnMax[3]);
    MCrv1 = MvarMVSetAllDomains(MCrv1, DmnMin, DmnMax, TRUE);
    MCrv2 = MvarMVSetAllDomains(MCrv2, DmnMin, DmnMax, TRUE);
    MCircCenter = MvarMVSetAllDomains(MCircCenter, DmnMin, DmnMax, TRUE);

    /* Build the constraints. */
    MDCrv1 = MvarMVDerive(MCrv1, 0);
    MDCrv2 = MvarMVDerive(MCrv2, 1);
    MTmp1 = MvarMVSub(MCrv1, MCircCenter);
    MTmp2 = MvarMVSub(MCrv2, MCircCenter);

    Trans = -IRIT_SQR(Radius);
    MVs[0] = MvarMVDotProd(MTmp1, MTmp1);
    MvarMVTransform(MVs[0], &Trans, 1.0);
    MVs[1] = MvarMVDotProd(MTmp2, MTmp2);
    MvarMVTransform(MVs[1], &Trans, 1.0);

    MVs[2] = MvarMVDotProd(MDCrv1, MTmp1);
    MVs[3] = MvarMVDotProd(MDCrv2, MTmp2);

    MvarMVFree(MDCrv1);
    MvarMVFree(MDCrv2);
    MvarMVFree(MTmp1);
    MvarMVFree(MTmp2);
    MvarMVFree(MCrv1);
    MvarMVFree(MCrv2);
    MvarMVFree(MCircCenter);

    Constraints[0] = 
        Constraints[1] = 
	    Constraints[2] = 
		Constraints[3] = MVAR_CNSTRNT_ZERO;

    Solution = MvarMVsZeros(MVs, Constraints, 4, Tol,
			    MVAR_CIRC_TAN_2CRVS_NUMER_TOL);

    for (i = 0; i < 4; i++)
	MvarMVFree(MVs[i]);

    /* Map the Circle centers to Eucliean space. */
    for (Pt = Solution; Pt != NULL; Pt = Pt -> Pnext) {
        CagdRType
	    *R = CagdSrfEval(CircCenterSrf, Pt -> Pt[2], Pt -> Pt[3]);

	Pt -> Pt[2] = R[1];
	Pt -> Pt[3] = R[2];
    }

    CagdSrfFree(CircCenterSrf);

    return Solution;
}
