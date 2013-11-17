/******************************************************************************
* MvBzrSym.c - Bezier symbolic computation for multivariate library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 97. 					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

#define NODE_EQUAL_SHIFT 0.8

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier multivariates - multiply them coordinatewise.	     M
*   The two multivariates are promoted to same point type before             M
* multiplication can take place.					     M
*   See also BzrMultInterpFlag.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:   The two multivariates to multiply.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  The product MV1 * MV2 coordinatewise.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVMult, product                                                   M
*****************************************************************************/
MvarMVStruct *MvarBzrMVMult(const MvarMVStruct *MV1, const MvarMVStruct *MV2)
{
    int i, *Indices1, *Indices2, Index1, Index2, UseCagdIcKJcMIJcKMTableCache,
	ProdIndex = 0;
    CagdBType UseCagdIChooseKTable, IsNotRational;
    MvarMVStruct *ProdMV, *CpMV1, *CpMV2;
    int MaxCoord, *Lengths, Size, *Orders1, *Orders2, *ProdOrders;
    CagdRType **Points, * const *Points1, * const *Points2;

    if (!MVAR_IS_BEZIER_MV(MV1) || !MVAR_IS_BEZIER_MV(MV2)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BEZIER_EXPECTED);
	return NULL;
    }

    if (MV1 -> PType != MV2 -> PType) {
        MV1 = CpMV1 = MvarMVCopy(MV1);
	MV2 = CpMV2 = MvarMVCopy(MV2);
	if (!MvarMakeMVsCompatible(&CpMV1, &CpMV2, FALSE, FALSE)) {
	    MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);
	    return NULL;
	}
    }
    else
	CpMV1 = CpMV2 = NULL;

    Lengths = (int *) IritMalloc(sizeof(int) * MV1 -> Dim * 2);
    for (i = 0; i < MV1 -> Dim; i++)
	Lengths[i] = MV1 -> Lengths[i] +  MV2 -> Lengths[i] - 1;
    ProdMV = MvarBzrMVNew(MV1 -> Dim, Lengths, MV1 -> PType);

    Indices1 = Lengths;
    Indices2 = &Lengths[MV1 -> Dim];

    IsNotRational = !MVAR_IS_RATIONAL_MV(ProdMV);
    MaxCoord = MVAR_NUM_OF_MV_COORD(ProdMV);
    Size = MVAR_CTL_MESH_LENGTH(ProdMV);

    Points = ProdMV -> Points;
    Points1 = MV1 -> Points;
    Points2 = MV2 -> Points;

    Orders1 = MV1 -> Orders;
    Orders2 = MV2 -> Orders;
    ProdOrders = ProdMV -> Orders;
	   
    for (i = IsNotRational; i <= MaxCoord; i++)
	IRIT_ZAP_MEM(Points[i], sizeof(CagdRType) * Size);

    /* Check if the maximal order is less than the orders in the caches. */
    UseCagdIChooseKTable = TRUE;
    for (i = 0; i < ProdMV -> Dim; i++) {
        if (ProdMV -> Orders[i] >= CAGD_MAX_BEZIER_CACHE_ORDER) {
	    UseCagdIChooseKTable = FALSE;
	    break;
	}
    }

    UseCagdIcKJcMIJcKMTableCache = TRUE;
    for (i = 1; i < ProdMV -> Dim; i++) {
        if (Orders1[i] >= CAGD_MAX_BEZIER_CACHE_ORDER2 ||
            Orders2[i] >= CAGD_MAX_BEZIER_CACHE_ORDER2) {
            UseCagdIcKJcMIJcKMTableCache = FALSE;
	    break;
        }
    }

    /* Reset all indices - note Indices2/Index2 will zeroed every loop. */
    IRIT_ZAP_MEM(Indices1, sizeof(int) * MV1 -> Dim * 2);
    Index1 = Index2 = 0;
    if (UseCagdIChooseKTable) {		        /* Use the precomputed table */
        do {
	    CagdRType
		Coef0 = 1.0;

	    do {
	        CagdRType Coef;

		if (Indices2[0] == 0) {
		    int *I1 = &Indices1[1],
		        *I2 = &Indices2[1],
		        *O1 = Orders1,
		        *O2 = Orders2,
		        *ProdSubSpcs = ProdMV -> SubSpaces;

		    Coef0 = 1.0;

		    if (UseCagdIcKJcMIJcKMTableCache) {
		        for (i = 1; i < ProdMV -> Dim; i++, I1++, I2++) {
			    Coef0 *= CagdIcKJcMIJcKMTable[*++O1 - 1]
			                                 [*++O2 - 1]
			                                 [*I1]
			                                 [*I2];
			}
		    }
		    else {
		        int *OP = ProdOrders;

		        for (i = 1; i < ProdMV -> Dim; i++, I1++, I2++) {
			    Coef0 *= CagdIChooseKTable[*++O1 - 1][*I1] *
		                     CagdIChooseKTable[*++O2 - 1][*I2] /
		                     CagdIChooseKTable[*++OP - 1][*I1 + *I2];
			}
		    }

		    for (i = ProdIndex = 0; i < ProdMV -> Dim; i++)
		        ProdIndex += *ProdSubSpcs++
					* (Indices1[i] + Indices2[i]);
		}
		else {
		    ProdIndex++;
		}

		if (UseCagdIcKJcMIJcKMTableCache) {
		    Coef = Coef0 * CagdIcKJcMIJcKMTable[Orders1[0] - 1]
			                               [Orders2[0] - 1]
			                               [Indices1[0]]
			                               [Indices2[0]];
		}
		else {
		    Coef = Coef0 *
		        CagdIChooseKTable[Orders1[0] - 1][Indices1[0]] *
		        CagdIChooseKTable[Orders2[0] - 1][Indices2[0]] /
		        CagdIChooseKTable[ProdOrders[0] - 1]
	                                 [Indices1[0] + Indices2[0]];
		}

		for (i = IsNotRational; i <= MaxCoord; i++)
		    Points[i][ProdIndex] +=
		        Coef * Points1[i][Index1] * Points2[i][Index2];
	    }
	    while (MVAR_INCREMENT_MESH_INDICES(MV2, Indices2, Index2));
	}
	while (MVAR_INCREMENT_MESH_INDICES(MV1, Indices1, Index1));
    }
    else {
        do {
	    CagdRType
		Coef0 = 1.0;

	    do {
	        CagdRType Coef;

		if (Indices2[0] == 0) {
		    int *I1 = &Indices1[1],
		        *I2 = &Indices2[1],
		        *O1 = Orders1,
		        *O2 = Orders2,
		        *OP = ProdOrders,
		        *ProdSubSpcs = ProdMV -> SubSpaces;

		    Coef0 = 1.0;

		    for (i = 1; i < ProdMV -> Dim; i++, I1++, I2++) {
		        Coef0 *= CagdIChooseK(*I1, *++O1 - 1) *
		                 CagdIChooseK(*I2, *++O2 - 1) /
		                 CagdIChooseK(*I1 + *I2, *++OP - 1);
		    }

		    for (i = ProdIndex = 0; i < ProdMV -> Dim; i++)
		        ProdIndex += *ProdSubSpcs++
					* (Indices1[i] + Indices2[i]);
		}
		else {
		    ProdIndex++;
		}

		Coef = Coef0 *
		    CagdIChooseK(Indices1[0], Orders1[0] - 1) *
		    CagdIChooseK(Indices2[0], Orders2[0] - 1) /
		    CagdIChooseK(Indices1[0] + Indices2[0], ProdOrders[0] - 1);

		for (i = IsNotRational; i <= MaxCoord; i++)
		    Points[i][ProdIndex] +=
		        Coef * Points1[i][Index1] * Points2[i][Index2];
	    }
	    while (MVAR_INCREMENT_MESH_INDICES(MV2, Indices2, Index2));
	}
	while (MVAR_INCREMENT_MESH_INDICES(MV1, Indices1, Index1));
    }

    IritFree(Indices1);

    if (CpMV1 != NULL)
        MvarMVFree(CpMV1);
    if (CpMV2 != NULL)
	MvarMVFree(CpMV2);

    return ProdMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bezier multivariates - computes its derivative surface    M
* in direction Dir, using the quotient rule for differentiation.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Rational Bezier multivariate to differentiate.               M
*   Dir:        Direction of Differentiation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    Differentiated rational Bezier  multivariate.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDerive, MvarBzrMVDerive, MvarBzrMVDeriveRational  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVDeriveRational, derivatives                                     M
*****************************************************************************/
MvarMVStruct *MvarBzrMVDeriveRational(MvarMVStruct *MV, MvarMVDirType Dir)
{
    int i;
    MvarMVStruct *DMVScalars[MVAR_MAX_PT_SIZE], *MVScalars[MVAR_MAX_PT_SIZE],
	*TMV1, *TMV2, *DeriveMV;

    MVAR_SPLIT_SCALARS(MV, MVScalars);

    if (MVScalars[0])
	DMVScalars[0] = MvarBzrMVDerive(MVScalars[0], Dir);
    else
	MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_EXPECTED);

    for (i = 1; i <= MVAR_MAX_PT_COORD && MVScalars[i] != NULL; i++) {
	DMVScalars[i] = MvarBzrMVDerive(MVScalars[i], Dir);

	TMV1 = MvarBzrMVMult(DMVScalars[i], MVScalars[0]);
	TMV2 = MvarBzrMVMult(MVScalars[i],  DMVScalars[0]);

	MvarMVFree(DMVScalars[i]);
	DMVScalars[i] = MvarMVSub(TMV1, TMV2);
	MvarMVFree(TMV1);
	MvarMVFree(TMV2);
    }
    
    TMV1 = MvarBzrMVMult(MVScalars[0], MVScalars[0]);
    MvarMVFree(MVScalars[0]);
    MVScalars[0] = TMV1;

    MVAR_FREE_SCALARS(MVScalars);

    DeriveMV = MvarMVMergeScalar(DMVScalars);

    MVAR_FREE_SCALARS(DMVScalars);

    return DeriveMV;
}
