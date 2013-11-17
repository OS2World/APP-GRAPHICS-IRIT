/******************************************************************************
* Mvar_Rev.c - Reverses, promotes, and project multivariates.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June. 97.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverse the role of the given two axis by flipping them out.               M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:            Multi-Variate to reverse.	                             M
*   Axis1, Axis2:  Two axis to flip over.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Reversed multi-variate.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPromoteMVToMV, MvarMVShiftAxes					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVReverse, multi-variates                                            M
*****************************************************************************/
MvarMVStruct *MvarMVReverse(const MvarMVStruct *MV, int Axis1, int Axis2)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    MvarMVStruct *MVRev;
    CagdRType * const *Points, **RevPoints;
    int i, *Indices, Index,
        MaxCoord = MVAR_NUM_OF_MV_COORD(MV);

    if (Axis1 == Axis2)
	return MvarMVCopy(MV);

    if (Axis1 < 0 || Axis1 >= MV -> Dim || Axis2 < 0 || Axis2 >= MV -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INVALID_AXIS);
	return NULL;
    }

    /* Duplicate the multivariate and flip all relevant material. */
    MVRev = MvarMVCopy(MV);

    IRIT_SWAP(int, MVRev -> Lengths[Axis1],  MVRev -> Lengths[Axis2]);
    IRIT_SWAP(int, MVRev -> Orders[Axis1],   MVRev -> Orders[Axis2]);
    IRIT_SWAP(int, MVRev -> Periodic[Axis1], MVRev -> Periodic[Axis2]);
    IRIT_SWAP(CagdRType *, MVRev -> KnotVectors[Axis1],
			   MVRev -> KnotVectors[Axis2]);
    for (i = 0; i < MVRev -> Dim; i++)
	MVRev -> SubSpaces[i] = i == 0 ? 1 : MVRev -> SubSpaces[i - 1]
					         * MVRev -> Lengths[i - 1];

    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);

    Index = 0;
    Points = MV -> Points;
    RevPoints = MVRev -> Points;
    do {
	int RevIndex;

	IRIT_SWAP(int, Indices[Axis1], Indices[Axis2]);
	RevIndex = MvarGetPointsMeshIndices(MVRev, Indices);
	IRIT_SWAP(int, Indices[Axis1], Indices[Axis2]);

	for (i = IsNotRational; i <= MaxCoord; i++)
	    RevPoints[i][RevIndex] = Points[i][Index];
    }
    while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));

    IritFree(Indices);

    return MVRev;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Shift the last index in, instead of index Axis.  All axes after Axis are   M
* shifted forward one location as well.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to shift axes.	                             M
*   Axis:     From where to shift forward until last Axis and put last Axis  M
*	      Here instead.						     M
*	                                                                     *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Multi-variate, with shifted axes			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVReverse, MvarPromoteMVToMV					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVShiftAxes, multi-variates                                          M
*****************************************************************************/
MvarMVStruct *MvarMVShiftAxes(const MvarMVStruct *MV, int Axis)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    MvarMVStruct *MVRev;
    CagdRType *SaveKV, * const *Points, **RevPoints;
    int i, SaveIndex, *Indices, Index,
        Dim = MV -> Dim,
        MaxCoord = MVAR_NUM_OF_MV_COORD(MV);

    if (Axis == Dim - 1)
	return MvarMVCopy(MV);

    if (Axis < 0 || Axis >= Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INVALID_AXIS);
	return NULL;
    }

    /* Duplicate the multivariate and flip all relevant material. */
    MVRev = MvarMVCopy(MV);

    SaveIndex = MVRev -> Lengths[Dim - 1];
    for (i = Dim - 1; i > Axis; i--)
	MVRev -> Lengths[i] = MVRev -> Lengths[i - 1];
    MVRev -> Lengths[Axis] = SaveIndex;

    SaveIndex = MVRev -> Orders[Dim - 1];
    for (i = Dim - 1; i > Axis; i--)
	MVRev -> Orders[i] = MVRev -> Orders[i - 1];
    MVRev -> Orders[Axis] = SaveIndex;

    SaveIndex = MVRev -> Periodic[Dim - 1];
    for (i = Dim - 1; i > Axis; i--)
	MVRev -> Periodic[i] = MVRev -> Periodic[i - 1];
    MVRev -> Periodic[Axis] = SaveIndex;

    SaveKV = MVRev -> KnotVectors[Dim - 1];
    for (i = Dim - 1; i > Axis; i--)
	MVRev -> KnotVectors[i] = MVRev -> KnotVectors[i - 1];
    MVRev -> KnotVectors[Axis] = SaveKV;

    for (i = 0; i < MVRev -> Dim; i++)
	MVRev -> SubSpaces[i] = i == 0 ? 1 : MVRev -> SubSpaces[i - 1]
					         * MVRev -> Lengths[i - 1];

    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);

    Index = 0;
    Points = MV -> Points;
    RevPoints = MVRev -> Points;
    do {
	int RevIndex;

	/* Shift the indices from Axis to Dim-1. */
	SaveIndex = Indices[Dim - 1];
	for (i = Dim - 1; i > Axis; i--)
	    Indices[i] = Indices[i - 1];
	Indices[Axis] = SaveIndex;

	RevIndex = MvarGetPointsMeshIndices(MVRev, Indices);

	/* Shift the indices back from Axis to Dim-1. */
	SaveIndex = Indices[Axis];
	for (i = Axis; i < Dim - 1; i++)
	    Indices[i] = Indices[i + 1];
	Indices[Dim - 1] = SaveIndex;

	/* Copy the control point. */
	for (i = IsNotRational; i <= MaxCoord; i++)
	    RevPoints[i][RevIndex] = Points[i][Index];
    }
    while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));

    IritFree(Indices);

    return MVRev;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Increase by one the dimensionality of the given multivariate, by	     M
* introducing a new constant (degree zero) axis with one control point in    M
* direction Axis.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:          Multi-Variate to promote.	                             M
*   Axis: 	 Axis of promotion.  Between zero and MV -> Dim.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Promoted multi-variate.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVShiftAxes, MvarMVFromMV, MvarPromoteMVToMV2			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPromoteMVToMV, multi-variates                                        M
*****************************************************************************/
MvarMVStruct *MvarPromoteMVToMV(const MvarMVStruct *MV, int Axis)
{
    MvarMVStruct *ProMV;

    if (Axis < 0 || Axis > MV -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INVALID_AXIS);
	return NULL;
    }

    ProMV = MvarMVFromMV(MV, 0.0, -1);
    if (Axis != ProMV -> Dim - 1) {
	MvarMVStruct
	    *ProMV2 = MvarMVShiftAxes(ProMV, Axis);

	MvarMVFree(ProMV);
	ProMV = ProMV2;
    }

    return ProMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Increase by the dimensionality of the given multivariate to NewDim, by     M
* introducing new constant (degree zero) axes with one control points in all M
* new directions.							     M
*   The Axis of the original MV will be starting at StartAxis.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        Multi-Variate to promote.	                             M
*   NewDim:    New dimension of the promoted multivariate.                   M
*   StartAxis: Original MV would span axes StartAxis to StartAxis+MV->Dim-1. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Promoted multi-variate.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVShiftAxes, MvarMVFromMV, MvarPromoteMVToMV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPromoteMVToMV2, multi-variates                                       M
*****************************************************************************/
MvarMVStruct *MvarPromoteMVToMV2(const MvarMVStruct *MV,
				 int NewDim,
				 int StartAxis)
{
    int i;
    MvarMVStruct *MVTmp2,
	*MVTmp1 = NULL;

    if (StartAxis + MV -> Dim > NewDim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INVALID_AXIS);
	return NULL;
    }

    for (i = 0; i < StartAxis; i++) {
	if (i == 0)
	    MVTmp2 = MvarPromoteMVToMV(MV, 0);
	else {
	    MVTmp2 = MvarPromoteMVToMV(MVTmp1, 0);
	    MvarMVFree(MVTmp1);
	}
	MV = MVTmp1 = MVTmp2;
    }

    for (i = MV -> Dim; i < NewDim; i++) {
	MVTmp2 = MvarPromoteMVToMV(MV, MV -> Dim);
	if (MVTmp1 != NULL)
	    MvarMVFree(MVTmp1);
	MV = MVTmp1 = MVTmp2;
    }

    return MVTmp1;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a polynomal curve and an index of a control point, CtlPtIdx,	     M
* construct a new multiariate where the i'th control points is mapped to 2nd M
* and above parameters of the returned multivariate.  The first parameter of M
* the multivariate remains the input curves' parameter.  For example, if the M
* input curve is E2 (planar), a trivariate will be returned, M(t, x, y),     M
* where t is the original curve's parameter and x and y parametrize the      M
* Euclidean values of the CtlPtIdx control point.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to make its i'th control point a parameter.           M
*   CtlPtIdx:    Index of control point to make a parameter.                 M
*   Min, Max:    Domain each coordinate of CtlPtIdx point should vary.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A multivariate parametrizing both the original curve   M
8		      and the CtlPtIdx's control points Euclidean values.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvMakeCtlPtParam                                                    M
*****************************************************************************/
MvarMVStruct *MvarCrvMakeCtlPtParam(const CagdCrvStruct *Crv,
				    int CtlPtIdx,
				    CagdRType Min,
				    CagdRType Max)
{
    int i, j,
	Length = Crv -> Length,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *TCrv, *Crv1, *Crv2;
    MvarMVStruct *MVEuclid, *MV, *MVTemp, *MVTemp2, *MV1, *MV2, **MV2Scalars;

    if (CtlPtIdx < 0 || CtlPtIdx >= Length) {
        MVAR_FATAL_ERROR(MVAR_ERR_WRONG_INDEX);
	return NULL;
    }

    Crv1 = CagdCrvCopy(Crv);
    Crv2 = CagdCrvCopy(Crv);

    for (i = 1; i <= MaxAxis; i++) {
        /* In Crv1, zero out influence of selected ctlpt. */
        Crv1 -> Points[i][CtlPtIdx] = 0.0;
    }

    for (j = 0; j < Length; j++) {
        for (i = 1; i <= MaxAxis; i++) {
	    /* In Crv2, zero out all ctlpts but selected one. */
	    Crv2 -> Points[i][j] = j == CtlPtIdx;
	}
    }

    /* Promote the two curves to multivariate with proper dimensions. */
    MVTemp = MvarCrvToMV(Crv1);
    CagdCrvFree(Crv1);
    MV1 = MvarPromoteMVToMV2(MVTemp, 1 + MaxAxis, 0);
    MvarMVFree(MVTemp);

    MVTemp = MvarCrvToMV(Crv2);
    CagdCrvFree(Crv2);
    MV2 = MvarPromoteMVToMV2(MVTemp, 1 + MaxAxis, 0);
    MvarMVFree(MVTemp);

    /* Parameterize the CtlPtIdx Euclidean values in the higher dimensions. */
    TCrv = BzrCrvNew(2, CAGD_PT_E1_TYPE);
    TCrv -> Points[1][0] = Min;
    TCrv -> Points[1][1] = Max;
    MVEuclid = MvarCrvToMV(TCrv);

    MV2Scalars = MvarMVSplitScalar(MV2);
    MvarMVFree(MV2);

    for (i = 1; i <= MaxAxis; i++) {
        MVTemp = MvarPromoteMVToMV2(MVEuclid, 1 + MaxAxis, i);
        MVTemp2 = MvarMVMult(MVTemp, MV2Scalars[i]);
	MvarMVFree(MV2Scalars[i]);
	MvarMVFree(MVTemp);
	MV2Scalars[i] = MVTemp2;
    }

    MV2 = MvarMVMergeScalar(MV2Scalars);
    MVAR_FREE_SCALARS(MV2Scalars);

    MvarMVFree(MVEuclid);

    MV = MvarMVAdd(MV1, MV2);
    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return MV;
}

