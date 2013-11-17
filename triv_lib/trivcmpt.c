/******************************************************************************
* TrivCmpt.c - Make objects compatible.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two trivariates, makes them compatible by:			     M
* 1. Coercing their point type to be the same.				     M
* 2. Making them have the same tri-variate type.			     M
* 3. Raising the degree of the lower one to be the same as the higher.	     M
* 4. Refining them to a common knot vector (If Bspline and SameOrder).	     M
*                                                                            M
* Note 3 is performed if SameOrder TRUE, 4 if SameKV TRUE.		     M
* Both trivariates are modified IN PLACE.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV1, TV2:  Two surfaces to be made compatible, in place.                 M
*   SameUOrder:  If TRUE, this routine make sure they share the same U       M
*                order.							     M
*   SameVOrder:  If TRUE, this routine make sure they share the same V       M
*                order.							     M
*   SameWOrder:  If TRUE, this routine make sure they share the same W       M
*                order.							     M
*   SameUKV:     If TRUE, this routine make sure they share the same U       M
*                knot vector and hence continuity.                           *
*   SameVKV:     If TRUE, this routine make sure they share the same V       M
*                knot vector and hence continuity.                           M
*   SameWKV:     If TRUE, this routine make sure they share the same W       M
*                knot vector and hence continuity.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivMakeTVsCompatible, compatibility                                     M
*****************************************************************************/
CagdBType TrivMakeTVsCompatible(TrivTVStruct **TV1,
				TrivTVStruct **TV2,
				CagdBType SameUOrder,
				CagdBType SameVOrder,
				CagdBType SameWOrder,
				CagdBType SameUKV,
				CagdBType SameVKV,
				CagdBType SameWKV)
{
    int i, KV1Len, KV2Len, RefLen;
    CagdRType *KV1, *KV2, *RefKV;
    TrivTVStruct *TmpTV;
    CagdPointType CommonPType;

    if ((*TV1 == NULL) || (*TV2 == NULL))
	return TRUE;

    CommonPType = CagdMergeIrtPtType((*TV1) -> PType, (*TV2) -> PType);

    /* Make the point types compatible. */
    if (CommonPType != (*TV1) -> PType) {
	TmpTV = TrivCoerceTVTo(*TV1, CommonPType);
	TrivTVFree(*TV1);
	*TV1 = TmpTV;
    }
    if (CommonPType != (*TV2) -> PType) {
	TmpTV = TrivCoerceTVTo(*TV2, CommonPType);
	TrivTVFree(*TV2);
	*TV2 = TmpTV;
    }

    if (SameUOrder) {
	/* Raise the degree of the lower one. */
	for (i = (*TV1) -> UOrder; i < (*TV2) -> UOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV1, TRIV_CONST_U_DIR);
	    TrivTVFree(*TV1);
	    *TV1 = TmpTV;
	}
	for (i = (*TV2) -> UOrder; i < (*TV1) -> UOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV2, TRIV_CONST_U_DIR);
	    TrivTVFree(*TV2);
	    *TV2 = TmpTV;
	}
    }
    if (SameVOrder) {
	for (i = (*TV1) -> VOrder; i < (*TV2) -> VOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV1, TRIV_CONST_V_DIR);
	    TrivTVFree(*TV1);
	    *TV1 = TmpTV;
	}
	for (i = (*TV2) -> VOrder; i < (*TV1) -> VOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV2, TRIV_CONST_V_DIR);
	    TrivTVFree(*TV2);
	    *TV2 = TmpTV;
	}
    }
    if (SameWOrder) {
	for (i = (*TV1) -> WOrder; i < (*TV2) -> WOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV1, TRIV_CONST_W_DIR);
	    TrivTVFree(*TV1);
	    *TV1 = TmpTV;
	}
	for (i = (*TV2) -> WOrder; i < (*TV1) -> WOrder; i++) {
	    TmpTV = TrivTVDegreeRaise(*TV2, TRIV_CONST_W_DIR);
	    TrivTVFree(*TV2);
	    *TV2 = TmpTV;
	}
    }

    /* If incompatible trivar type - make it the same as well. */
    if ((*TV1) -> GType != (*TV2) -> GType) {
	/* If Bezier basis - promote to bspline: */
	if ((*TV1) -> GType == TRIV_TVBEZIER_TYPE) {
	    TmpTV = TrivCnvrtBzr2BspTV(*TV1);
	    TrivTVFree(*TV1);
	    *TV1 = TmpTV;
	}
	if ((*TV2) -> GType == TRIV_TVBEZIER_TYPE) {
	    TmpTV = TrivCnvrtBzr2BspTV(*TV2);
	    TrivTVFree(*TV2);
	    *TV2 = TmpTV;
	}
    }

    if ((*TV1) -> GType == TRIV_TVBSPLINE_TYPE) {
	/* If bspline surface - make sure knot vectors are the same. */

	if (SameUKV && SameUOrder) {
	    /* Handle the U Direction. */
	    int Order = (*TV1) -> UOrder;

	    KV1 = (*TV1) -> UKnotVector;
	    KV2 = (*TV2) -> UKnotVector;
	    KV1Len = (*TV1) -> ULength + Order;
	    KV2Len = (*TV2) -> ULength + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine TV1 there. */
	    RefKV  = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV1, TRIV_CONST_U_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV1);
		*TV1 = TmpTV;
		KV1 = (*TV1) -> UKnotVector;
		KV1Len = (*TV1) -> ULength + Order;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine TV2 there. */
	    RefKV  = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV2, TRIV_CONST_U_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV2);
		*TV2 = TmpTV;
	    }
	    IritFree(RefKV);
	}

	if (SameVKV && SameVOrder) {
	    /* Handle the V Direction. */
	    int Order = (*TV1) -> VOrder;

	    KV1 = (*TV1) -> VKnotVector;
	    KV2 = (*TV2) -> VKnotVector;
	    KV1Len = (*TV1) -> VLength + Order;
	    KV2Len = (*TV2) -> VLength + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine TV1 there. */
	    RefKV  = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV1, TRIV_CONST_V_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV1);
		*TV1 = TmpTV;
		KV1 = (*TV1) -> VKnotVector;
		KV1Len = (*TV1) -> VLength + Order;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine TV2 there. */
	    RefKV = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				    &KV2[Order - 1], KV2Len - Order * 2 + 2,
				    &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV2, TRIV_CONST_V_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV2);
		*TV2 = TmpTV;
	    }
	    IritFree(RefKV);
	}

	if (SameWKV && SameWOrder) {
	    /* Handle the V Direction. */
	    int Order = (*TV1) -> WOrder;

	    KV1 = (*TV1) -> WKnotVector;
	    KV2 = (*TV2) -> WKnotVector;
	    KV1Len = (*TV1) -> WLength + Order;
	    KV2Len = (*TV2) -> WLength + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine TV1 there. */
	    RefKV = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				    &KV1[Order - 1], KV1Len - Order * 2 + 2,
				    &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV1, TRIV_CONST_W_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV1);
		*TV1 = TmpTV;
		KV1 = (*TV1) -> WKnotVector;
		KV1Len = (*TV1) -> WLength + Order;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine TV2 there. */
	    RefKV  = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TmpTV = TrivTVRefineAtParams(*TV2, TRIV_CONST_W_DIR,
					     FALSE, RefKV, RefLen);
		TrivTVFree(*TV2);
		*TV2 = TmpTV;
	    }
	    IritFree(RefKV);
	}
    }

    return TRUE;
}

