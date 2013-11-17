/******************************************************************************
* MvBspSym.c - Bspline symbolic computation for multivariate library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 97..					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

#define NODE_EQUAL_SHIFT 0.8
#define DISCONT_KNOT(KV, t, Len, Order) \
	((BspKnotFirstIndexG(KV, Len + Order, t) - \
	  BspKnotLastIndexL(KV, Len + Order, t)) > Order)

IRIT_STATIC_DATA int
    BspMultUsingInterpolation = 0;

static MvarMVStruct *MvarBspMVMultAux(MvarMVStruct *MV1, MvarMVStruct *MV2);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets method of Bspline product computation.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   BspMultUsingInter:  If TRUE, Bspline product is computed by setting an   M
*                       interpolation problem. Otherwise, by decomposing the M
*                       Bspline geometry to Bezier geometry.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         Previous setting.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMultComputationMethod, product 		                     M
*****************************************************************************/
int MvarBspMultComputationMethod(int BspMultUsingInter)
{
    int i = BspMultUsingInterpolation;

    BspMultUsingInterpolation = BspMultUsingInter;

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bspline multivariates - multiply them coordinatewise.	     M
*   The two multivariates are promoted to same point type before             M
* multiplication can take place.					     M
*   See also BspMultComputationMethod.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1, CMV2:   The two multivariates to multiply.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  The product MV1 * MV2 coordinatewise.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVMult, product                                                   M
*****************************************************************************/
MvarMVStruct *MvarBspMVMult(const MvarMVStruct *CMV1, const MvarMVStruct *CMV2)
{
    int i;
    MvarMVStruct *ProdMV, *TMV, *MV1, *MV2;

    /* Check for same domain.*/
    if (CMV1 -> Dim != CMV2 -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	return NULL;
    }
    for (i = 0; i < CMV1 -> Dim; i++) {
	CagdRType Min1, Max1, Min2, Max2;

	MvarMVDomain(CMV1, &Min1, &Max1, i);
	MvarMVDomain(CMV2, &Min2, &Max2, i);
	if (!IRIT_APX_EQ(Min1, Min2) || !IRIT_APX_EQ(Max1, Max2)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
	    return NULL;
	}
    }

    MV1 = MvarMVCopy(CMV1);
    MV2 = MvarMVCopy(CMV2);

    if (!MvarMakeMVsCompatible(&MV1, &MV2, FALSE, FALSE)) {
	MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);
	return NULL;
    }

    TMV = MvarBspMVMultAux(MV1, MV2);

    if (MVAR_IS_BEZIER_MV(TMV)) {
	ProdMV = MvarCnvrtBzr2BspMV(TMV);
	MvarMVFree(TMV);
    }
    else
	ProdMV = TMV;

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return ProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary routine. Subdivides the multivariates into Bezier multivariates, *
* multiply the Bezier multivariates and merge them back. All is done         *
* simultaneously.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   MV1, MV2:   The two multivariates to multiply.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:  The product MV1 * MV2 coordinatewise.                   *
*****************************************************************************/
static MvarMVStruct *MvarBspMVMultAux(MvarMVStruct *MV1, MvarMVStruct *MV2)
{
    MvarMVStruct *MVProd, *MVTmp;
    int i;

    for (i = 0; i < MV1 -> Dim; i++) {
	if (MV1 -> Lengths[i] != MV1 -> Orders[i] ||
	    MV2 -> Lengths[i] != MV2 -> Orders[i]) {
	    CagdRType
		t = MV1 -> Lengths[i] != MV1 -> Orders[i] ?
		        MV1 -> KnotVectors[i][(MV1 -> Lengths[i] +
					       MV1 -> Orders[i]) >> 1] :
		        MV2 -> KnotVectors[i][(MV2 -> Lengths[i] +
					       MV2 -> Orders[i]) >> 1];
	    MvarMVStruct *MV1Prod, *MV2Prod,
		*MV1a = MvarMVSubdivAtParam(MV1, t, i),
		*MV1b = MV1a -> Pnext,
		*MV2a = MvarMVSubdivAtParam(MV2, t, i),
		*MV2b = MV2a -> Pnext;
	    CagdBType
		DiscontMerge = DISCONT_KNOT(MV1 -> KnotVectors[i], t,
					    MV1 -> Lengths[i],
					    MV1 -> Orders[i]) ||
			       DISCONT_KNOT(MV2 -> KnotVectors[i], t,
					    MV2 -> Lengths[i],
					    MV2 -> Orders[i]);

	    MV1a -> Pnext = NULL;
	    MV2a -> Pnext = NULL;

	    MV1Prod = MvarBspMVMultAux(MV1a, MV2a);
	    MvarMVFree(MV1a);
	    MvarMVFree(MV2a);

	    MV2Prod = MvarBspMVMultAux(MV1b, MV2b);
	    MvarMVFree(MV1b);
	    MvarMVFree(MV2b);

	    MVProd = MvarMergeMVMV(MV1Prod, MV2Prod, i, DiscontMerge);
	    MvarMVFree(MV1Prod);
	    MvarMVFree(MV2Prod);

	    return MVProd;
	}
    }

    /* It is actually a Bezier multivariate - fool them to thing so! */
    MV1 -> GType = MVAR_BEZIER_TYPE;
    MV2 -> GType = MVAR_BEZIER_TYPE;

    MVTmp = MvarBzrMVMult(MV1, MV2);

    MV1 -> GType = MVAR_BSPLINE_TYPE;
    MV2 -> GType = MVAR_BSPLINE_TYPE;

    /* Make the parametric domain equal that of the two given multivariates. */
    MVProd = MvarCnvrtBzr2BspMV(MVTmp);
    MvarMVFree(MVTmp);
    for (i = 0; i < MVProd -> Dim; i++) {
	CagdRType Min, Max;

	MvarMVDomain(MV1, &Min, &Max, i);
	BspKnotAffineTrans2(MVProd -> KnotVectors[i],
			    MVProd -> Orders[i] + MVProd -> Lengths[i],
			    Min, Max);
    }

    return MVProd;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bspline multivariates - computes its derivative surface   M
* in direction Dir, using the quotient rule for differentiation.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Rational Bspline multivariate to differentiate.              M
*   Dir:        Direction of Differentiation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    Differentiated rational Bspline multivariate.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDerive, MvarBspMVDerive, MvarBzrMVDeriveRational  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVDeriveRational, derivatives                                     M
*****************************************************************************/
MvarMVStruct *MvarBspMVDeriveRational(MvarMVStruct *MV, MvarMVDirType Dir)
{
    int i;
    MvarMVStruct *DMVScalars[MVAR_MAX_PT_SIZE], *MVScalars[MVAR_MAX_PT_SIZE],
	*TMV1, *TMV2, *DeriveMV;

    MVAR_SPLIT_SCALARS(MV, MVScalars);

    if (MVScalars[0])
	DMVScalars[0] = MvarBspMVDerive(MVScalars[0], Dir);
    else
	MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_EXPECTED);

    for (i = 1; i <= MVAR_MAX_PT_COORD && MVScalars[i] != NULL; i++) {
	DMVScalars[i] = MvarBspMVDerive(MVScalars[i], Dir);

	TMV1 = MvarBspMVMult(DMVScalars[i], MVScalars[0]);
	TMV2 = MvarBspMVMult(MVScalars[i],  DMVScalars[0]);

	MvarMVFree(DMVScalars[i]);
	DMVScalars[i] = MvarMVSub(TMV1, TMV2);
	MvarMVFree(TMV1);
	MvarMVFree(TMV2);
    }
    
    TMV1 = MvarBspMVMult(MVScalars[0], MVScalars[0]);
    MvarMVFree(MVScalars[0]);
    MVScalars[0] = TMV1;

    MVAR_FREE_SCALARS(MVScalars);

    DeriveMV = MvarMVMergeScalar(DMVScalars);

    MVAR_FREE_SCALARS(DMVScalars);

    return DeriveMV;
}
