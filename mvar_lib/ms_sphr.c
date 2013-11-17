/******************************************************************************
* MS_sphr.c - minimum spanning (hyper) spheres.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec 2006.					      *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"

static MvarPtStruct *MvarTanHyperSpheresofNManifoldsMV(MvarMVStruct **MVs,
						       int NumOfMVs,
						       CagdRType SubdivTol,
						       CagdRType NumericTol);
static MvarPtStruct *MvarTanHyperSpheresofNManifoldsET(MvarMVStruct **MVs,
						       int NumOfMVs,
						       CagdRType SubdivTol,
						       CagdRType NumericTol);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all the hyper-spheres that are tangent to all the given set of  M
* manifolds.  All manifolds should share the same range space, R^d, space in M
* which	the hyper-spheres are sought.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:    	  The manifolds to consider in R^d.		             M
*   NumOfMVs:	  Number of multivariates we need to process.		     M
*   SubdivTol, NumericTol:   Of computation.		                     M
*   UseExprTree:  TRUE to use expression trees in the computation, FALSE to  M
*		  use regular multivariate expressions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:      List of tangent hyper-spheres, NULL if error.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMSCircOfThreeCurves, MvarMSCircOfTwoCurves, MvarMinSpanCirc          M
*   MvarTanHyperSpheresofNManifoldsET					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarTanHyperSpheresofNManifolds                                          M
*****************************************************************************/
MvarPtStruct *MvarTanHyperSpheresofNManifolds(MvarMVStruct **MVs,
					      int NumOfMVs,
					      CagdRType SubdivTol,
					      CagdRType NumericTol,
					      CagdBType UseExprTree)
{
    return UseExprTree ?
	MvarTanHyperSpheresofNManifoldsET(MVs, NumOfMVs,
					  SubdivTol, NumericTol):
	MvarTanHyperSpheresofNManifoldsMV(MVs, NumOfMVs,
					  SubdivTol, NumericTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes all the hyper-spheres that are tangent to all the given set of  *
* manifolds.  All manifolds should share the same range space, R^d, space in *
* which	the hyper-spheres are sought.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:    		     The manifolds to consider in R^d.               *
*   NumOfMVs:		     Number of multivariates we need to process.     *
*   SubdivTol, NumericTol:   Of computation.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:      List of tangent hyper-spheres, NULL if error.       *
*****************************************************************************/
static MvarPtStruct *MvarTanHyperSpheresofNManifoldsMV(MvarMVStruct **MVs,
						       int NumOfMVs,
						       CagdRType SubdivTol,
						       CagdRType NumericTol)
{
    int i, j, d, c,
	Range = MVAR_NUM_OF_PT_COORD(MVs[0] -> PType),
	NumOfConstraints = NumOfMVs - 1;        /* N-1 equality constraints. */
    MvarBBoxStruct MVsBBox;
    MvarMVStruct **MVsConstraints, *MVCenter, *MVCntrTmp, *MViTmp,
	*MVTmp0, *MVTmp1, *MVTmp2;
    MvarConstraintType *MVsCnstrntsTypes;
    MvarPtStruct *MVPts, *MVPt;

    /* In R^d we should have d+1 multivariates. */
    if (Range != NumOfMVs - 1) {
        MVAR_FATAL_ERROR(MVAR_ERR_MSS_INCONSISTENT_NUM_OBJ);
	return NULL;
    }

    /* Make sure all manifolds are with the same range. */
    for (i = 1; i < NumOfMVs; i++) {
	if (Range != MVAR_NUM_OF_PT_COORD(MVs[i] -> PType)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_SAME_PTYPE_EXPECTED);
	    return NULL;
	}
    }

    /* Make sure all manifolds are with domains zero to one, for simplicity. */
    for (i = 0; i < NumOfMVs; i++) {
        MVs[i] = MvarMVCopy(MVs[i]);
	if (MVAR_IS_BSPLINE_MV(MVs[i])) {
	    NumOfConstraints += MVs[i] -> Dim;

	    for (j = 0; j < MVs[i] -> Dim; j++)
	        BspKnotAffineTransOrder2(MVs[i] -> KnotVectors[j],
					 MVs[i] -> Orders[j],
					 MVs[i] -> Lengths[j] +
					     MVs[i] -> Orders[j], 0.0, 1.0);
	}
    }

    MVsConstraints = (MvarMVStruct **)
		 	 IritMalloc(sizeof(MvarMVStruct *) * NumOfConstraints);
    MVsCnstrntsTypes = (MvarConstraintType *)
	             IritMalloc(sizeof(MvarConstraintType) * NumOfConstraints);

    /* Create a mulit-linear MVs to represent the center.  This MV will span */
    /* the bounding box of all input MVs.				     */
    MvarMVBBox(MVs[0], &MVsBBox);
    for (i = 1; i < NumOfMVs; i++) {
	MvarBBoxStruct BBox;

	MvarMVBBox(MVs[i], &BBox);
	MvarMergeBBox(&MVsBBox, &BBox);
    }
    MVCenter = MvarMVMultiLinearMV(MVsBBox.Min, MVsBBox.Max, Range);

    /* Search for the multivariate with minimal dimension, for efficiency. */
    for (i = 1; i < NumOfMVs; i++) {
        if (MVs[i] -> Dim < MVs[0] -> Dim)
	    IRIT_SWAP(MvarMVStruct *, MVs[0], MVs[i]);
    }

    /* Build the NumOfMVs-1 equality constraints:			   */
    /* "MVs[0] - Center ||^2 == "MVs[i] - Center ||^2, i > 0.		   */
    MVCntrTmp = MvarPromoteMVToMV2(MVCenter, NumOfConstraints, 0);
    MVTmp0 = MvarPromoteMVToMV2(MVs[0], NumOfConstraints, Range);
    MVTmp1 = MvarMVSub(MVTmp0, MVCntrTmp);
    MvarMVFree(MVTmp0);
    MVTmp0 = MvarMVDotProd(MVTmp1, MVTmp1);
    MvarMVFree(MVTmp1);

    d = Range + MVs[0] -> Dim;      /* The dimention to promote MVs[i] to. */
    for (c = 0, i = 1; i < NumOfMVs; i++) {
        MViTmp = MvarPromoteMVToMV2(MVs[i], NumOfConstraints, d);
	d += MVs[i] -> Dim;

	MVTmp1 = MvarMVSub(MViTmp, MVCntrTmp);
	MvarMVFree(MViTmp);
	MVTmp2 = MvarMVDotProd(MVTmp1, MVTmp1);
	MvarMVFree(MVTmp1);
	MVsConstraints[c++] = MvarMVSub(MVTmp2, MVTmp0);
	MvarMVFree(MVTmp2);
    }
    MvarMVFree(MVTmp0);

    /* Build the tangential constraints. */
    for (i = 0, d = Range; i < NumOfMVs; i++) {
        MViTmp = MvarPromoteMVToMV2(MVs[i], NumOfConstraints, d);

	MVTmp1 = MvarMVSub(MVCntrTmp, MViTmp);
        for (j = 0; j < MVs[i] -> Dim; j++) {
	    MVTmp2 = MvarMVDerive(MViTmp, d + j);
	    MVsConstraints[c++] = MvarMVDotProd(MVTmp1, MVTmp2);
	    MvarMVFree(MVTmp2);
	}
	MvarMVFree(MVTmp1);

	d += MVs[i] -> Dim;
    }
    assert(c == NumOfConstraints);
    MvarMVFree(MVCntrTmp);

    /* Invoke the zero set solver. */
    for (i = 0; i < NumOfConstraints; i++)
        MVsCnstrntsTypes[i] = MVAR_CNSTRNT_ZERO;

    MVPts = MvarMVsZeros(MVsConstraints, MVsCnstrntsTypes, NumOfConstraints,
			 SubdivTol, NumericTol);

    for (i = 0; i < NumOfConstraints; i++)
	MvarMVFree(MVsConstraints[i]);
    IritFree(MVsConstraints);
    IritFree(MVsCnstrntsTypes);

    /* Convert the solution points to center/radius results. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        IrtRType Radius, Center[ MVAR_MAX_PT_COORD],
	    *R = MvarMVEval(MVCenter, MVPt -> Pt);

	CAGD_GEN_COPY(Center, &R[1], sizeof(IrtRType) * Range);
	R = MvarMVEval(MVs[0], &MVPt -> Pt[Range]);
	for (Radius = 0.0, i = 0; i < Range; i++)
	    Radius += IRIT_SQR(R[i + 1] - Center[i]);
	Radius = sqrt(Radius);

	AttrSetRealAttrib(&MVPt -> Attr, "radius", Radius);
	MVPt -> Dim = Range;
	CAGD_GEN_COPY(MVPt -> Pt, Center, sizeof(IrtRType) * Range);
    }
    MvarMVFree(MVCenter);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes all the hyper-spheres that are tangent to all the given set of  *
* manifolds.  All manifolds should share the same range space, R^d, space in *
* which	the hyper-spheres are sought. Uses Multivariate Expression Trees.    *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:    		     The manifolds to consider in R^d.               *
*   NumOfMVs:		     Number of multivariates we need to process.     *
*   Center:                  Center of the computed MSS.                     *
*   Radius:                  Radius of the computed MSS.                     *
*   SubdivTol, NumericTol:   Of computation.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:      List of tangent hyper-spheres, NULL if error.       *
*****************************************************************************/
static MvarPtStruct *MvarTanHyperSpheresofNManifoldsET(MvarMVStruct **MVs,
						       int NumOfMVs,
						       CagdRType SubdivTol,
						       CagdRType NumericTol)
{
    int i, j, d, c,
	Range = MVAR_NUM_OF_PT_COORD(MVs[0] -> PType),
	NumOfConstraints = NumOfMVs - 1;        /* N-1 equality constraints. */
    MvarBBoxStruct MVsBBox;
    MvarMVStruct *MVCenter, *MVCntrTmp, *MViTmp,
	*MVTmp0, *MVTmp1, *MVTmp2, *MVTmp3;
    MvarExprTreeStruct **MVETsConstraints;
    MvarConstraintType *MVsCnstrntsTypes;
    MvarPtStruct *MVPts, *MVPt;

    /* In R^d we should have d+1 multivariates. */
    if (Range != NumOfMVs - 1) {
        MVAR_FATAL_ERROR(MVAR_ERR_MSS_INCONSISTENT_NUM_OBJ);
	return NULL;
    }

    /* Make sure all manifolds are with the same range. */
    for (i = 1; i < NumOfMVs; i++) {
	if (Range != MVAR_NUM_OF_PT_COORD(MVs[i] -> PType)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_SAME_PTYPE_EXPECTED);
	    return NULL;
	}
    }

    /* Make sure all manifolds are with domains zero to one, for simplicity. */
    for (i = 0; i < NumOfMVs; i++) {
	if (MVAR_IS_BSPLINE_MV(MVs[i]))
	    MVs[i] = MvarMVCopy(MVs[i]);
	else if  (MVAR_IS_BEZIER_MV(MVs[i]))
	    MVs[i] = MvarCnvrtBzr2BspMV(MVs[i]);

	NumOfConstraints += MVs[i] -> Dim;

	if (MVAR_IS_BSPLINE_MV(MVs[i])) {
	    for (j = 0; j < MVs[i] -> Dim; j++)
	        BspKnotAffineTransOrder2(MVs[i] -> KnotVectors[j],
					 MVs[i] -> Orders[j],
					 MVs[i] -> Lengths[j] +
					     MVs[i] -> Orders[j], 0.0, 1.0);
	}
    }

    MVETsConstraints = (MvarExprTreeStruct **)
		   IritMalloc(sizeof(MvarExprTreeStruct *) * NumOfConstraints);
    MVsCnstrntsTypes = (MvarConstraintType *)
	             IritMalloc(sizeof(MvarConstraintType) * NumOfConstraints);

    /* Create a mulit-linear MVs to represent the center.  This MV will span */
    /* the bounding box of all input MVs.				     */
    MvarMVBBox(MVs[0], &MVsBBox);
    for (i = 1; i < NumOfMVs; i++) {
	MvarBBoxStruct BBox;

	MvarMVBBox(MVs[i], &BBox);
	MvarMergeBBox(&MVsBBox, &BBox);
    }
    MVCenter = MvarMVMultiLinearMV(MVsBBox.Min, MVsBBox.Max, Range);

    /* Search for the multivariate with minimal dimension, for efficiency. */
    for (i = 1; i < NumOfMVs; i++) {
        if (MVs[i] -> Dim < MVs[0] -> Dim)
	    IRIT_SWAP(MvarMVStruct *, MVs[0], MVs[i]);
    }

    /* Build the NumOfMVs-1 equality constraints:			   */
    /* "MVs[0] - Center ||^2 == "MVs[i] - Center ||^2, i > 0.		   */
    MVCntrTmp = MvarPromoteMVToMV2(MVCenter, NumOfConstraints, 0);
    MVTmp0 = MvarPromoteMVToMV2(MVs[0], NumOfConstraints, Range);
    MVTmp1 = MvarMVSub(MVTmp0, MVCntrTmp);
    MvarMVFree(MVTmp0);
    MVTmp0 = MvarMVDotProd(MVTmp1, MVTmp1);
    MvarMVFree(MVTmp1);

    d = Range + MVs[0] -> Dim;      /* The dimension to promote MVs[i] to. */
    for (c = 0, i = 1; i < NumOfMVs; i++) {
        MViTmp = MvarPromoteMVToMV2(MVs[i], NumOfConstraints, d);
	d += MVs[i] -> Dim;

	MVTmp1 = MvarMVSub(MViTmp, MVCntrTmp);
	MvarMVFree(MViTmp);
	MVTmp2 = MvarMVDotProd(MVTmp1, MVTmp1);
	MvarMVFree(MVTmp1);
	MVETsConstraints[c++] =
	    MvarExprTreeSub(MvarExprTreeFromMV(MVTmp2, NumOfConstraints, 0),
			    MvarExprTreeFromMV(MVTmp0, NumOfConstraints, 0));
	MvarMVFree(MVTmp2);
    }
    MvarMVFree(MVTmp0);

    /* Build the tangential constraints. */
    for (i = 0, d = Range; i < NumOfMVs; i++) {
        MViTmp = MvarPromoteMVToMV2(MVs[i], NumOfConstraints, d);

	MVTmp1 = MvarMVSub(MVCntrTmp, MViTmp);
        for (j = 0; j < MVs[i] -> Dim; j++) {
	    MVTmp2 = MvarMVDerive(MViTmp, d + j);
	    MVTmp3 = MvarMVDotProd(MVTmp1, MVTmp2);
	    MVETsConstraints[c++] = MvarExprTreeFromMV(MVTmp3,
						       NumOfConstraints, 0);
	    MvarMVFree(MVTmp3);
	    MvarMVFree(MVTmp2);
	}
	MvarMVFree(MVTmp1);

	d += MVs[i] -> Dim;
    }
    assert(c == NumOfConstraints);
    MvarMVFree(MVCntrTmp);

    /* Invoke the zero set solver. */
    for (i = 0; i < NumOfConstraints; i++)
        MVsCnstrntsTypes[i] = MVAR_CNSTRNT_ZERO;

    MVPts = MvarExprTreesZeros(MVETsConstraints, MVsCnstrntsTypes,
			       NumOfConstraints, SubdivTol, NumericTol);

    for (i = 0; i < NumOfConstraints; i++)
	MvarExprTreeFree(MVETsConstraints[i], FALSE);
    IritFree(MVETsConstraints);
    IritFree(MVsCnstrntsTypes);

    /* Convert the solution points to center/radius results. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        IrtRType Radius, Center[ MVAR_MAX_PT_COORD],
	    *R = MvarMVEval(MVCenter, MVPt -> Pt);

	CAGD_GEN_COPY(Center, &R[1], sizeof(IrtRType) * Range);
	R = MvarMVEval(MVs[0], &MVPt -> Pt[Range]);
	for (Radius = 0.0, i = 0; i < Range; i++)
	    Radius += IRIT_SQR(R[i + 1] - Center[i]);
	Radius = sqrt(Radius);

	AttrSetRealAttrib(&MVPt -> Attr, "radius", Radius);
	MVPt -> Dim = Range;
	CAGD_GEN_COPY(MVPt -> Pt, Center, sizeof(IrtRType) * Range);
    }
    MvarMVFree(MVCenter);

    return MVPts;
}
