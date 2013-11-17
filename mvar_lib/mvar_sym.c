/******************************************************************************
* Mvar_sym.c - Generic symbolic computation for the multivariate library.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 97.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

static MvarMVStruct *MvarMVAddSubAux(const MvarMVStruct *MV1,
				     const MvarMVStruct *MV2,
				     CagdBType OperationAdd);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - add them coordinatewise.			     M
*   The two multivariates are promoted to same point type before the	     M
* multiplication can take place. Furthermore, order and continuity are	     M
* matched as well.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariate to add up coordinatewise.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The summation of MV1 + MV2 coordinatewise.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVSub, MvarMeshAddSub, MvarMVMult                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAdd, addition, symbolic computation, multivariates                 M
*****************************************************************************/
MvarMVStruct *MvarMVAdd(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    int i;

    /* Check for same domain.*/
    if (MV1 -> Dim != MV2 -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	return NULL;
    }
    for (i = 0; i < MV1 -> Dim; i++) {
	CagdRType Min1, Max1, Min2, Max2;

	MvarMVDomain(MV1, &Min1, &Max1, i);
	MvarMVDomain(MV2, &Min2, &Max2, i);
	if (!IRIT_APX_EQ(Min1, Min2) || !IRIT_APX_EQ(Max1, Max2)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	    return NULL;
	}
    }

    return MvarMVAddSubAux(MV1, MV2, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - subtract them coordinatewise.		     M
*   The two multivariates are promoted to same point type before the	     M
* multiplication can take place. Furthermore, order and continuity are	     M
* matched as well.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariate to subtract coordinatewise.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The difference of MV1 - MV2 coordinatewise.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAdd, MvarMeshAddSub, MvarMVMult                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVSub, subtraction, symbolic computation, multivariates              M
*****************************************************************************/
MvarMVStruct *MvarMVSub(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    int i;

    /* Check for same domain.*/
    if (MV1 -> Dim != MV2 -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	return NULL;
    }
    for (i = 0; i < MV1 -> Dim; i++) {
	CagdRType Min1, Max1, Min2, Max2;

	MvarMVDomain(MV1, &Min1, &Max1, i);
	MvarMVDomain(MV2, &Min2, &Max2, i);
	if (!IRIT_APX_EQ(Min1, Min2) || !IRIT_APX_EQ(Max1, Max2)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	    return NULL;
	}
    }

    return MvarMVAddSubAux(MV1, MV2, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - multiply them coordinatewise.		     M
*   The two multivariates are promoted to same point type before the 	     M
* multiplication can take place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariate to multiply coordinatewise.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The product of MV1 * MV2 coordinatewise.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,    M
*   MvarMVInvert							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVMult, product, symbolic computation, multivariates                 M
*****************************************************************************/
MvarMVStruct *MvarMVMult(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    MvarMVStruct
	*ProdMV = NULL;

    if (MV1 -> GType == MVAR_BEZIER_TYPE &&
	MV2 -> GType == MVAR_BEZIER_TYPE)
	ProdMV = MvarBzrMVMult(MV1, MV2);
    else if ((MV1 -> GType == MVAR_BEZIER_TYPE ||
	      MV1 -> GType == MVAR_BSPLINE_TYPE) &&
	     (MV2 -> GType == MVAR_BEZIER_TYPE ||
	      MV2 -> GType == MVAR_BSPLINE_TYPE))
	ProdMV = MvarBspMVMult(MV1, MV2);
    else
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);

    return ProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar multivariate, returns a scalar multivariate representing    M
* the reciprocal values, by making it rational (if was not one) and flipping M
* the numerator and the denominator.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       A scalar multivariate to compute a reciprocal value for.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A rational scalar multivariate that is equal to the    M
*                      reciprocal value of MV.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,    M
*   MvarMVMult, MvarMVCrossProd                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVInvert, division, symbolic computation, reciprocal value,          M
* multivariates								     M
*****************************************************************************/
MvarMVStruct *MvarMVInvert(const MvarMVStruct *MV)
{
    int i;
    CagdRType *R;
    MvarMVStruct
	*NewMV = MvarMVNew(MV -> Dim, MV -> GType, MVAR_PT_P1_TYPE,
			   MV -> Lengths);

    switch (MV -> PType) {
	case CAGD_PT_E1_TYPE:
	    CAGD_GEN_COPY(NewMV -> Points[0], MV -> Points[1],
			  sizeof(CagdRType) * MVAR_CTL_MESH_LENGTH(MV));
	    for (i = 0, R = NewMV -> Points[1];
		 i < MVAR_CTL_MESH_LENGTH(MV);
		 i++)
		*R++ = 1.0;
	    break;
	case CAGD_PT_P1_TYPE:
	    CAGD_GEN_COPY(NewMV -> Points[0], MV -> Points[1],
			  sizeof(CagdRType) * MVAR_CTL_MESH_LENGTH(MV));
	    CAGD_GEN_COPY(NewMV -> Points[1], MV -> Points[0],
			  sizeof(CagdRType) * MVAR_CTL_MESH_LENGTH(MV));
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
	    break;
    }

    if (MVAR_IS_BSPLINE_MV(MV)) {
        for (i = 0; i < MV -> Dim; i++) {
	    assert(MV -> KnotVectors[i] != NULL);
	    NewMV -> KnotVectors[i] =
	        BspKnotCopy(NULL, MV -> KnotVectors[i],
			    MVAR_MVAR_ITH_PT_LST_LEN(MV, i) + MV -> Orders[i]);
	}

        CAGD_GEN_COPY(NewMV -> Orders, MV -> Orders, MV -> Dim * sizeof(int));
    }

    CAGD_PROPAGATE_ATTR(NewMV, MV);

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multivariate, scale it by Scale.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV:      A multivariate to scale by magnitude Scale.                    M
*   Scale:    Scaling factor.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A multivariates scaled by Scale compared to MV.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVMult, MvarMVMultScalar,           M
*   MvarMVInvert, MvarMVCrossProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVScalarScale, scaling, symbolic computation, multivariates          M
*****************************************************************************/
MvarMVStruct *MvarMVScalarScale(const MvarMVStruct *CMV, CagdRType Scale)
{
    int i;
    CagdRType *R;
    MvarMVStruct
	*MV = MvarMVCopy(CMV);

    switch (MV -> PType) {
	case CAGD_PT_P1_TYPE:
	case CAGD_PT_E1_TYPE:
	    for (i = 0, R = MV -> Points[1]; i < MVAR_CTL_MESH_LENGTH(MV); i++)
		*R++ *= Scale;
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
	    break;
    }

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariate - a vector multivariate MV1 and a scalar	     M
* multivariate MV2, multiply all MV1's coordinates by the scalar	     M
* multivariate MV2.							     M
*   Returned multivariate is a multivariate representing the product of the  M
* two given multivariates.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariates to multiply.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate representing the product of MV1 and MV2. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVMult, MvarMVCrossProd,            M
*   MvarCrvMultScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVMultScalar, product, symbolic computation, multivariates           M
*****************************************************************************/
MvarMVStruct *MvarMVMultScalar(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    int i;
    MvarMVStruct *ProdMV, *MV1Scalars[MVAR_MAX_PT_SIZE],
        *MV2Scalars[MVAR_MAX_PT_SIZE],
	*ProdMVScalars[MVAR_MAX_PT_SIZE];

    MVAR_CLEAR_SCALARS(ProdMVScalars);
    MVAR_SPLIT_SCALARS(MV1, MV1Scalars);
    MVAR_SPLIT_SCALARS(MV2, MV2Scalars);

    for (i = 1;
	 i <= MVAR_MAX_PT_COORD &&
	     MV1Scalars[i] != NULL &&
	     MV2Scalars[1] != NULL;
	 i++) {
	ProdMVScalars[i] = MvarMVMult(MV1Scalars[i], MV2Scalars[1]);
    }

    /* Product of W axes. */
    if (MV1Scalars[0] || MV2Scalars[0]) {
	if (MV1Scalars[0] == NULL)
	    ProdMVScalars[0] = MvarMVCopy(MV2Scalars[0]);
	else if (MV2Scalars[0] == NULL)
	    ProdMVScalars[0] = MvarMVCopy(MV1Scalars[0]);
	else
	    ProdMVScalars[0] = MvarMVMult(MV1Scalars[0], MV2Scalars[0]);
    }

    MVAR_FREE_SCALARS(MV1Scalars);
    MVAR_FREE_SCALARS(MV2Scalars);

    ProdMV = MvarMVMergeScalar(ProdMVScalars);

    MVAR_FREE_SCALARS(ProdMVScalars);

    return ProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - computes their dot product.			     M
*   Returned multivariate is a scalar multivariate representing the dot      M
* product of the two given multivariates.				     M
*   While typically in R3, the dot product can be computed for any           M
* dimension of MV1 and MV2.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariate to multiply and compute a dot product for.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A scalar multivariate representing the dot product of  M
*                     MV1 . MV2.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVMult, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,       M
*   MvarMVInvert, MvarMVCrossProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDotProd, product, dot product, symbolic computation, multivariates M
*****************************************************************************/
MvarMVStruct *MvarMVDotProd(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    int i;
    MvarMVStruct *DProdMV, *Tmp1MV, *Tmp2MV, *MV1Scalars[MVAR_MAX_PT_SIZE],
        *MV2Scalars[MVAR_MAX_PT_SIZE],
	*DProdMVScalars[MVAR_MAX_PT_SIZE];

    MVAR_CLEAR_SCALARS(DProdMVScalars);
    MVAR_SPLIT_SCALARS(MV1, MV1Scalars);
    MVAR_SPLIT_SCALARS(MV2, MV2Scalars);

    DProdMVScalars[1] = MvarMVMult(MV1Scalars[1], MV2Scalars[1]);

    for (i = 2;
	 i <= MVAR_MAX_PT_COORD &&
	     MV1Scalars[i] != NULL &&
	     MV2Scalars[i] != NULL;
	 i++) {
	Tmp1MV = MvarMVMult(MV1Scalars[i], MV2Scalars[i]);

	Tmp2MV = MvarMVAdd(DProdMVScalars[1], Tmp1MV);
        MvarMVFree(Tmp1MV);
        MvarMVFree(DProdMVScalars[1]);

	DProdMVScalars[1] = Tmp2MV;
    }

    /* Product of W axes. */
    if (MV1Scalars[0] || MV2Scalars[0]) {
	if (MV1Scalars[0] == NULL)
	    DProdMVScalars[0] = MvarMVCopy(MV2Scalars[0]);
	else if (MV2Scalars[0] == NULL)
	    DProdMVScalars[0] = MvarMVCopy(MV1Scalars[0]);
	else
	    DProdMVScalars[0] = MvarMVMult(MV1Scalars[0], MV2Scalars[0]);
    }

    MVAR_FREE_SCALARS(MV1Scalars);
    MVAR_FREE_SCALARS(MV2Scalars);

    DProdMV = MvarMVMergeScalar(DProdMVScalars);

    MVAR_FREE_SCALARS(DProdMVScalars);

    return DProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multivariate and a vector - computes their dot product.            M
*   Returned multivariate is a scalar multivariate representing the dot      M
* product.								     M
*   While typically in R3, the dot product can be computed for any           M
* dimension of MV, and Vec should be of the appropriate size.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   Multivarients to multiply and compute a dot product for.           M
*   Vec:  Vector to project MV onto.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A scalar multivariate representing the dot product of  M
*                     MV . Vec.                                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVMult, MvarMVScalarScale, MvarMVMultScalar,          M
*   MvarMVInvert, MvarMVCrossProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVVecDotProd, product, dot product, symbolic computation,	     M
* multivariates							             M
*****************************************************************************/
MvarMVStruct *MvarMVVecDotProd(const MvarMVStruct *MV, const CagdRType *Vec)
{
    int i;
    MvarMVStruct *DProdMV, *Tmp1MV, *Tmp2MV, *MVScalars[MVAR_MAX_PT_SIZE],
	*DProdMVScalars[MVAR_MAX_PT_SIZE];

    MVAR_CLEAR_SCALARS(DProdMVScalars);
    MVAR_SPLIT_SCALARS(MV, MVScalars);

    DProdMVScalars[1] = MvarMVScalarScale(MVScalars[1], Vec[0]);

    for (i = 2; i <= MVAR_MAX_PT_COORD && MVScalars[i] != NULL; i++) {
	Tmp1MV = MvarMVScalarScale(MVScalars[i], Vec[i - 1]);

	Tmp2MV = MvarMVAdd(DProdMVScalars[1], Tmp1MV);
        MvarMVFree(Tmp1MV);
        MvarMVFree(DProdMVScalars[1]);

	DProdMVScalars[1] = Tmp2MV;
    }
    if (MVScalars[0] != NULL)
	DProdMVScalars[0] = MVScalars[0];
    DProdMV = MvarMVMergeScalar(DProdMVScalars);
    DProdMVScalars[0] = NULL;

    MVAR_FREE_SCALARS(MVScalars);
    MVAR_FREE_SCALARS(DProdMVScalars);

    return DProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - computes their cross product.		     M
*   Returned multivariate is a vector multivariate representing the cross    M
* product of the two given multivariates.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  Two multivariate to multiply and compute a cross product for. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A vector multivariate representing the cross product   M
*                     of MV1 x MV2.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,    M
*   MvarMVInvert, MvarMVCrossProd2D                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVCrossProd, product, cross product, multivariates,	             M
* symbolic computation						             M
*****************************************************************************/
MvarMVStruct *MvarMVCrossProd(const MvarMVStruct *MV1,
			      const MvarMVStruct *MV2)
{
    MvarMVStruct *CpMV1, *CpMV2, *TMV1, *TMV2,
	*CrossProdMV, *CProdMVScalars[MVAR_MAX_PT_SIZE],
	*MV1Scalars[MVAR_MAX_PT_SIZE], *MV2Scalars[MVAR_MAX_PT_SIZE];

    if (MV1 -> GType != CAGD_PT_E3_TYPE && MV1 -> GType != CAGD_PT_P3_TYPE) {
	MV1 = CpMV1 = MvarCoerceMVTo(MV1,
			         MVAR_IS_RATIONAL_MV(MV1) ? CAGD_PT_P3_TYPE
						          : CAGD_PT_E3_TYPE);
    }
    else
	CpMV1 = NULL;

    if (MV2 -> GType != CAGD_PT_E3_TYPE && MV2 -> GType != CAGD_PT_P3_TYPE) {
	MV2 = CpMV2 = MvarCoerceMVTo(MV2,
			         MVAR_IS_RATIONAL_MV(MV2) ? CAGD_PT_P3_TYPE
						          : CAGD_PT_E3_TYPE);
    }
    else
	CpMV2 = NULL;

    MVAR_SPLIT_SCALARS(MV1, MV1Scalars);
    MVAR_SPLIT_SCALARS(MV2, MV2Scalars);

    if (CpMV1 != NULL)
	MvarMVFree(CpMV1);
    if (CpMV2 != NULL)
	MvarMVFree(CpMV2);

    MVAR_CLEAR_SCALARS(CProdMVScalars);

    /* Cross product X axis. */
    TMV1 = MvarMVMult(MV1Scalars[2], MV2Scalars[3]);
    TMV2 = MvarMVMult(MV2Scalars[2], MV1Scalars[3]);
    CProdMVScalars[1] = MvarMVSub(TMV1, TMV2);
    MvarMVFree(TMV1);
    MvarMVFree(TMV2);

    /* Cross product Y axis. */
    TMV1 = MvarMVMult(MV1Scalars[3], MV2Scalars[1]);
    TMV2 = MvarMVMult(MV2Scalars[3], MV1Scalars[1]);
    CProdMVScalars[2] = MvarMVSub(TMV1, TMV2);
    MvarMVFree(TMV1);
    MvarMVFree(TMV2);

    /* Cross product Z axis. */
    TMV1 = MvarMVMult(MV1Scalars[1], MV2Scalars[2]);
    TMV2 = MvarMVMult(MV2Scalars[1], MV1Scalars[2]);
    CProdMVScalars[3] = MvarMVSub(TMV1, TMV2);
    MvarMVFree(TMV1);
    MvarMVFree(TMV2);

    /* Product of W axes. */
    if (MV1Scalars[0] || MV2Scalars[0]) {
	if (MV1Scalars[0] == NULL)
	    CProdMVScalars[0] = MvarMVCopy(MV2Scalars[0]);
	else if (MV2Scalars[0] == NULL)
	    CProdMVScalars[0] = MvarMVCopy(MV1Scalars[0]);
	else
	    CProdMVScalars[0] = MvarMVMult(MV1Scalars[0], MV2Scalars[0]);
    }

    MVAR_FREE_SCALARS(MV1Scalars);
    MVAR_FREE_SCALARS(MV2Scalars);

    CrossProdMV = MvarMVMergeScalar(CProdMVScalars);
    MVAR_FREE_SCALARS(CProdMVScalars);

    return CrossProdMV;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
* Given four multivariates - computes their 2D cross product.		     M
*   Returned multivariate is a scalar multivariate representing the cross    M
* product of the four given multivariates, as X1 * Y2 - X2 * Y1.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1X, MV1Y:  First pair of scalar multivariates (X, Y) of first funcs.   M
*   MV2X, MV2Y:  Second pair of scalar multivariates (X, Y) of second funcs. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A scalar multivariate representing the cross product   M
*                     of MV1X * MV2Y - MV2X * MV1Y.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,    M
*   MvarMVInvert, MvarMVCrossProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVCrossProd2D, product, cross product, multivariates,	             M
* symbolic computation						             M
*****************************************************************************/
MvarMVStruct *MvarMVCrossProd2D(const MvarMVStruct *MV1X,
				const MvarMVStruct *MV1Y,
				const MvarMVStruct *MV2X,
				const MvarMVStruct *MV2Y)
{
    MvarMVStruct *MVRet,
	*MVTmp1 = MvarMVMult(MV1X, MV2Y),
	*MVTmp2 = MvarMVMult(MV2X, MV1Y);

    MVRet = MvarMVSub(MVTmp1, MVTmp2);

    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    return MVRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multivariates - multiply them using the quotient product rule:   M
*  X = X1 W2 +/- X2 W1							     V
*   All provided multivariates are assumed to be non rational scalar         M
* multivariates.							     M
*   Returned is a non rational scalar multivariate (CAGD_PT_E1_TYPE).	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1X:     Numerator of first multivariate.                               M
*   MV1W:     Denominator of first multivariate.  Can be NULL.               M
*   MV2X:     Numerator of second multivariate.                              M
*   MV2W:     Denominator of second multivariate.  Can be NULL.              M
*   OperationAdd:   TRUE for addition, FALSE for subtraction.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  The result of  MV1X MV2W +/- MV2X MV1W.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDotProd, MvarMVVecDotProd, MvarMVScalarScale, MvarMVMultScalar,    M
*   MvarMVInvert, MvarMVCrossProd2D                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVRtnlMult, product, multivariates, symbolic computation             M
*****************************************************************************/
MvarMVStruct *MvarMVRtnlMult(const MvarMVStruct *MV1X,
			     const MvarMVStruct *MV1W,
			     const MvarMVStruct *MV2X,
			     const MvarMVStruct *MV2W,
			     CagdBType OperationAdd)
{
    MvarMVStruct *CTmp1, *CTmp2, *CTmp3;

    if (MV1X == NULL || MV2X == NULL)
	return NULL;

    /* Make the two multivariates - same order and point type. */
    CTmp1 = MV2W == NULL ? MvarMVCopy(MV1X) : MvarMVMult(MV1X, MV2W);
    CTmp2 = MV1W == NULL ? MvarMVCopy(MV2X) : MvarMVMult(MV2X, MV1W);

    if (!MvarMakeMVsCompatible(&CTmp1, &CTmp2, FALSE, FALSE))
	MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);

    CTmp3 = MvarMVAddSubAux(CTmp1, CTmp2, OperationAdd);
    MvarMVFree(CTmp1);
    MvarMVFree(CTmp2);

    return CTmp3;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given two multivariates - add or subtract them as prescribed by	     *
* OperationAdd, coordinatewise.			      			     *
*   The two multivariates are promoted to same type, point type, and order   *
* before the addition can take place.					     *
*   Returned is a multivariates representing their sum or difference.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   CMV1, CMV2:    Two multivariate to subtract coordinatewise.              *
*   OperationAdd:  TRUE of addition, FALSE for subtraction.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:   The summation or difference of MV1 and MV2.            *
*****************************************************************************/
static MvarMVStruct *MvarMVAddSubAux(const MvarMVStruct *CMV1,
				     const MvarMVStruct *CMV2,
				     CagdBType OperationAdd)
{
    CagdBType
        SameWeights = FALSE,
	MV1Rational = MVAR_IS_RATIONAL_MV(CMV1),
	MV2Rational = MVAR_IS_RATIONAL_MV(CMV2),
	NoneRational = !MV1Rational && !MV2Rational,
	BothRational = MV1Rational && MV2Rational;
    int i, Len,
	Dim = CMV1 -> Dim;
    MvarMVStruct *SumMV, *MV1, *MV2;
    CagdRType **Points1, **Points2;

    /* Make the two multivariates have same point type/multivariate type. */
    MV1 = MvarMVCopy(CMV1);
    MV2 = MvarMVCopy(CMV2);
    if (!MvarMakeMVsCompatible(&MV1, &MV2, NoneRational, NoneRational))
	MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);

    Points1 = MV1 -> Points;
    Points2 = MV2 -> Points;

    /* Check if both are rational with identical weights. */
    if (BothRational) {
        for (i = 0; i < Dim; i++) {
	    if (MV1 -> Orders[i] != MV2 -> Orders[i])
	        break;
	    if (MV1 -> GType == CAGD_CBSPLINE_TYPE &&
		(MV1 -> Lengths[i] != MV2 -> Lengths[i] ||
		 !BspKnotVectorsSame(MV1 -> KnotVectors[i],
				     MV2 -> KnotVectors[i],
				     MV1 -> Orders[i] + MV1 -> Lengths[i],
				     IRIT_EPS)))
	        break;
	}
	/* Maybe the weights are identical, in which we can still add the   */
	/* the respective control polygons.				    */
	if (i >= Dim) {
	    int MeshSize = MVAR_CTL_MESH_LENGTH(MV1);

	    for (i = 0; i < MeshSize; i++)
	        if (!IRIT_APX_EQ(Points1[0][i], Points2[0][i]))
		    break;
	    if (i >= MeshSize)
	        SameWeights = TRUE;
	}
    }

    if (NoneRational || SameWeights) {
        if (!MvarMakeMVsCompatible(&MV1, &MV2, TRUE, TRUE))
	    MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);

	Len = MVAR_CTL_MESH_LENGTH(MV1);
	
	SumMV = MvarMVNew(Dim, MV1 -> GType, MV1 -> PType, MV1 -> Lengths);
	CAGD_GEN_COPY(SumMV -> Orders, MV1 -> Orders, Dim * sizeof(int));

	for (i = 0; i < MV1 -> Dim; i++) {
	    if (MV1 -> KnotVectors[i] != NULL)
	        SumMV -> KnotVectors[i] =
		    BspKnotCopy(NULL, MV1 -> KnotVectors[i],
				MV1 -> Lengths[i] + MV1 -> Orders[i]);
	}

	/* Simply add the respective control polygons. */
	SymbMeshAddSub(SumMV -> Points, MV1 -> Points, MV2 -> Points,
		       (CagdPointType) SumMV -> PType, Len, OperationAdd);
    }
    else {
        MvarMVStruct *SumMVScalars[MVAR_MAX_PT_SIZE],
		     *MV1Scalars[MVAR_MAX_PT_SIZE],
		     *MV2Scalars[MVAR_MAX_PT_SIZE];

	/* Weights are different. Must use the addition of rationals         */
	/* rule ( we invoke MvarMVMult here):				     */
	/*								     */
	/*  x1     x2   x1 w2 +/- x2 w1					     */
	/*  -- +/- -- = ---------------					     */
	/*  w1     w2        w1 w2					     */
	/*								     */

	MVAR_SPLIT_SCALARS(MV1, MV1Scalars);
	MVAR_SPLIT_SCALARS(MV2, MV2Scalars);
	MVAR_CLEAR_SCALARS(SumMVScalars);

	SumMVScalars[0] = MvarMVMult(MV1Scalars[0], MV2Scalars[0]);
	for (i = 1; i <= MVAR_MAX_PT_COORD && MV1Scalars[i] != NULL; i++) {
	    SumMVScalars[i] = MvarMVRtnlMult(MV1Scalars[i], MV1Scalars[0],
					     MV2Scalars[i], MV2Scalars[0],
					     OperationAdd);
	    MvarMVFree(MV1Scalars[i]);
	    MvarMVFree(MV2Scalars[i]);
	}
	MvarMVFree(MV1Scalars[0]);
	MvarMVFree(MV2Scalars[0]);

	SumMV = MvarMVMergeScalar(SumMVScalars);
	MVAR_FREE_SCALARS(SumMVScalars);
    }

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return SumMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multivariate, splits it to its scalar component multivariates.     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      Multivariate to split.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct **:   A static array holding the dynamically allocated MVs. M
*		The zero entry would hold the W, or NULL otherwise.  The     M
*		first entry would hold X axis, etc.			     M
*	        This vector would have MVAR_MAX_PT_COORD coordinates.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVMergeScalar			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVSplitScalar, split, multivariates, symbolic computation            M
*****************************************************************************/
MvarMVStruct **MvarMVSplitScalar(const MvarMVStruct *MV)
{
    IRIT_STATIC_DATA MvarMVStruct
	*MVs[MVAR_MAX_PT_SIZE];
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i,
        Size = MVAR_CTL_MESH_LENGTH(MV),
	NumCoords = MVAR_NUM_OF_MV_COORD(MV);

    MVAR_CLEAR_SCALARS(MVs);

    for (i = IsNotRational; i <= NumCoords; i++) {
	int j;

	MVs[i] = MvarMVNew(MV -> Dim, MV -> GType, MVAR_PT_E1_TYPE,
			   MV -> Lengths);
	CAGD_GEN_COPY(MVs[i] -> Orders, MV -> Orders, MV -> Dim * sizeof(int));

	for (j = 0; j < MV -> Dim; j++) {
	    if (MV -> KnotVectors[j] != NULL)
		MVs[i] -> KnotVectors[j] =
		    BspKnotCopy(NULL, MV -> KnotVectors[j],
				MV -> Lengths[j] + MV -> Orders[j]);
	    else
		MVs[i] -> KnotVectors[j] = NULL;
	}

	CAGD_GEN_COPY(MVs[i] -> Points[1], MV -> Points[i],
		      sizeof(CagdRType) * Size);
    }

    return MVs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of scalar multivariates, treat them as coordinates into a new  M
* multivariates								     M
*   Assumes at least X axis not NULL when a scalar multivariate is returned. M
*   Assumes all axes are either E1 or P1 in which the weights are assumed    M
* to be identical and can be ignored if W axis exists or copied otherwise.   M
*                                                                            *
* PARAMETERS:                                                                M
*   ScalarMVs:     A vector of scalar MVs.  Location 0 holds the W or NULL   M
*	       otherwise, Location 1 holds the X axis and so on.	     M
*	       This vector is assumed to have MVAR_MAX_PT_COORD coordinates. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A new multivariates constructed from given scalar	     M
*			multivariates.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVSplitScalar			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVMergeScalar, merge, multivariates, symbolic computation            M
*****************************************************************************/
MvarMVStruct *MvarMVMergeScalar(MvarMVStruct * const *ScalarMVs)
{
    CagdBType
	WeightCopied = FALSE,
	IsRational = ScalarMVs[0] != NULL;
    int i, Size, NumCoords;
    MvarMVStruct *MV, *LocalMVs[MVAR_MAX_PT_SIZE];

    /* Figure out the dimensionality of the given scalar fields. */
    for (NumCoords = 1;
	 ScalarMVs[NumCoords] != NULL && NumCoords <= MVAR_MAX_PT_COORD; )
	 NumCoords++;
    if (--NumCoords < 1) {
	MVAR_FATAL_ERROR(MVAR_ERR_TOO_FEW_PARAMS);
	return NULL;
    }

    /* Make a duplication as we are making them compatible, in place. */
    for (i = 0; i <= MVAR_MAX_PT_COORD; i++)
	LocalMVs[i] = ScalarMVs[i] == NULL ? NULL : MvarMVCopy(ScalarMVs[i]);
    for (i = !IsRational; i <= NumCoords; i++) {
	int j;

	for (j = i + 1; j <= NumCoords; j++)
	    MvarMakeMVsCompatible(&LocalMVs[i], &LocalMVs[j], TRUE, TRUE);
    }

    MV = MvarMVNew(LocalMVs[1] -> Dim, LocalMVs[1] -> GType,
		   MVAR_MAKE_PT_TYPE(IsRational, NumCoords),
		   LocalMVs[1] -> Lengths);
    CAGD_GEN_COPY(MV -> Orders, LocalMVs[1] -> Orders,
		  MV -> Dim * sizeof(int));

    for (i = 0; i < MV -> Dim; i++) {
        if (LocalMVs[1] -> KnotVectors[i] != NULL)
	    MV -> KnotVectors[i] = BspKnotCopy(NULL,
					       LocalMVs[1] -> KnotVectors[i],
					       LocalMVs[1] -> Lengths[i] +
						   LocalMVs[1] -> Orders[i]);
	else
	    MV -> KnotVectors[i] = NULL;
    }

    Size = MVAR_CTL_MESH_LENGTH(LocalMVs[1]);

    for (i = !IsRational; i <= NumCoords; i++) {
	if (LocalMVs[i] != NULL) {
	    if (LocalMVs[i] -> PType != CAGD_PT_E1_TYPE) {
		if (LocalMVs[i] -> PType != CAGD_PT_P1_TYPE)
		    MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
		else if (LocalMVs[0] == NULL && WeightCopied == FALSE) {
		    CAGD_GEN_COPY(MV -> Points[0], LocalMVs[i] -> Points[0],
				  sizeof(CagdRType) * Size);
		    WeightCopied = TRUE;
		}
	    }

	    CAGD_GEN_COPY(MV -> Points[i], LocalMVs[i] -> Points[1],
			  sizeof(CagdRType) * Size);
	}
    }

    for (i = 0; i <= MVAR_MAX_PT_COORD; i++)
	if (LocalMVs[i] != NULL)
	    MvarMVFree(LocalMVs[i]);

    return MV;
}
