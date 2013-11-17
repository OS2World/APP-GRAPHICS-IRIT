/******************************************************************************
* MvarCmpt.c - Make multivariates compatible.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 97.					      *
******************************************************************************/

#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two multi-variates, makes them compatible by:			     M
* 1. Coercing their point type to be the same.				     M
* 2. Making them have the same multi-variate type.			     M
* 3. Raising the degree of the lower one to be the same as the higher.	     M
* 4. Refining them to a common knot vector (If Bspline and SameOrder).	     M
*                                                                            M
* Note 3 is performed if SameOrder TRUE, 4 if SameKV TRUE.		     M
* Both multi-variates are modified IN PLACE.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:   Two surfaces to be made compatible, in place.                M
*   SameOrders: If TRUE, this routine make sure they share the same orders.  M
*   SameKVs:    If TRUE, this routine make sure they share the same KVs.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMakeMVsCompatible, compatibility                                     M
*****************************************************************************/
CagdBType MvarMakeMVsCompatible(MvarMVStruct **MV1,
				MvarMVStruct **MV2,
				CagdBType SameOrders,
				CagdBType SameKVs)
{
    int l, KV1Len, KV2Len, RefLen;
    CagdRType *KV1, *KV2, *RefKV;
    MvarMVStruct *TmpMV;
    MvarPointType CommonPType;

    if ((*MV1 == NULL) || (*MV2 == NULL))
	return TRUE;

    if ((*MV1) -> Dim != (*MV2) -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);
	return FALSE;
    }

    CommonPType = MvarMergeIrtPtType((*MV1) -> PType, (*MV2) -> PType);

    /* Make the point types compatible. */
    if (CommonPType != (*MV1) -> PType) {
	TmpMV = MvarCoerceMVTo(*MV1, (MvarPointType) CommonPType);
	MvarMVFree(*MV1);
	*MV1 = TmpMV;
    }
    if (CommonPType != (*MV2) -> PType) {
	TmpMV = MvarCoerceMVTo(*MV2, CommonPType);
	MvarMVFree(*MV2);
	*MV2 = TmpMV;
    }

    if (SameOrders) {
	CagdBType DoRaise;
        int *NewOrders = (int *) IritMalloc(sizeof(int) * (*MV1) -> Dim);

	/* Raise the degree of the lower one. */
	for (l = 0, DoRaise = FALSE; l < (*MV1) -> Dim; l ++)
	    NewOrders[l] = IRIT_MAX((*MV1) -> Orders[l], (*MV2) -> Orders[l]);

	for (l = 0, DoRaise = FALSE; l < (*MV1) -> Dim; l ++) {
	    if ((*MV1) -> Orders[l] != NewOrders[l]) {
		DoRaise = TRUE;
	    }
	}

	if (DoRaise) {
	    TmpMV = MvarMVDegreeRaiseN(*MV1, NewOrders);
	    MvarMVFree(*MV1);
	    *MV1 = TmpMV;
	}

	for (l = 0, DoRaise = FALSE; l < (*MV2) -> Dim; l ++) {
	    if ((*MV2) -> Orders[l] != NewOrders[l]) {
		DoRaise = TRUE;
	    }
	}

	if (DoRaise) {
	    TmpMV = MvarMVDegreeRaiseN(*MV2, NewOrders);
	    MvarMVFree(*MV2);
	    *MV2 = TmpMV;
	}

	IritFree(NewOrders);
    }

    /* If incompatible multi-variate type - make it the same as well. */
    if ((*MV1) -> GType != (*MV2) -> GType) {
	/* If Bezier basis - promote to Bspline: */
	if ((*MV1) -> GType == MVAR_BEZIER_TYPE) {
	    TmpMV = MvarCnvrtBzr2BspMV(*MV1);
	    MvarMVFree(*MV1);
	    *MV1 = TmpMV;
	}
	if ((*MV2) -> GType == MVAR_BEZIER_TYPE) {
	    TmpMV = MvarCnvrtBzr2BspMV(*MV2);
	    MvarMVFree(*MV2);
	    *MV2 = TmpMV;
	}
    }

    if ((*MV1) -> GType == MVAR_BSPLINE_TYPE) {
	/* If bspline surface - make sure knot vectors are the same. */

	if (SameKVs && SameOrders) {
	    for (l = 0; l < (*MV1) -> Dim; l ++) {
		int Order = (*MV1) -> Orders[l];

		KV1 = (*MV1) -> KnotVectors[l];
		KV2 = (*MV2) -> KnotVectors[l];
		KV1Len = (*MV1) -> Lengths[l] + Order;
		KV2Len = (*MV2) -> Lengths[l] + Order;

		/* Affine map second knot vector to span same param. domain. */
		BspKnotAffineTrans(KV2, KV2Len,
				   KV1[Order - 1] - KV2[Order - 1],
				   (KV1[KV1Len - Order] - KV1[Order - 1]) /
				   (KV2[KV2Len - Order] - KV2[Order - 1]));

		/* Find knots in KV2 which are not in KV1 and refine MV1. */
		RefKV  = BspKnotSubtrTwo(&KV2[Order - 1],
					 KV2Len - Order * 2 + 2,
					 &KV1[Order - 1],
					 KV1Len - Order * 2 + 2,
					 &RefLen);
		if (RefLen > 0) {
		    TmpMV = MvarMVRefineAtParams(*MV1, l,
						 FALSE, RefKV, RefLen);
		    MvarMVFree(*MV1);
		    *MV1 = TmpMV;
		    KV1 = (*MV1) -> KnotVectors[l];
		    KV1Len = (*MV1) -> Lengths[l] + Order;
		}
		IritFree(RefKV);

		/* Find knots in KV1 which are not in KV2 and refine MV2. */
		RefKV  = BspKnotSubtrTwo(&KV1[Order - 1],
					 KV1Len - Order * 2 + 2,
					 &KV2[Order - 1],
					 KV2Len - Order * 2 + 2,
					 &RefLen);
		if (RefLen > 0) {
		    TmpMV = MvarMVRefineAtParams(*MV2, l,
						 FALSE, RefKV, RefLen);
		    MvarMVFree(*MV2);
		    *MV2 = TmpMV;
		}
		IritFree(RefKV);
	    }
	}
    }

    return TRUE;
}
