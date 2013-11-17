/******************************************************************************
* CagdCmpt.c - Make objects compatible.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves, makes them compatible by:				     M
* 1. Coercing their point type to be the same.				     M
* 2. Making them have the same curve type.				     M
* 3. Raising the degree of the lower one to be the same as the higher.	     M
* 4. Refining them to a common knot vector (If Bspline and SameOrder).	     M
*									     M
* Note 3 is performed if SameOrder TRUE, 4 if SameKV TRUE.		     M
* Both curves are modified IN PLACE.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curves to be made compatible, in place.                 M
*   SameOrder:   If TRUE, this routine make sure they share the same order.  M
*   SameKV:      If TRUE, this routine make sure they share the same         M
*                knot vector and hence continuity.                           *
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMakeCrvsCompatible, compatibility                                    M
*****************************************************************************/
CagdBType CagdMakeCrvsCompatible(CagdCrvStruct **Crv1,
				 CagdCrvStruct **Crv2,
				 CagdBType SameOrder,
				 CagdBType SameKV)
{
    int KV1Len, KV2Len, RefLen;
    CagdRType *KV1, *KV2, *RefKV;
    CagdCrvStruct *TCrv;
    CagdPointType CommonPType;

    if ((*Crv1 == NULL) || (*Crv2 == NULL))
	return TRUE;

    CommonPType = CagdMergeIrtPtType((*Crv1) -> PType, (*Crv2) -> PType);

    if (CAGD_IS_PERIODIC_CRV(*Crv1) != CAGD_IS_PERIODIC_CRV(*Crv2)) {
        if (CAGD_IS_PERIODIC_CRV(*Crv1)) {
	    TCrv = CagdCnvrtBsp2OpenCrv(*Crv1);
	    CagdCrvFree(*Crv1);
	    *Crv1 = TCrv;
	}
        else {
	    TCrv = CagdCnvrtBsp2OpenCrv(*Crv2);
	    CagdCrvFree(*Crv2);
	    *Crv2 = TCrv;
	}
    }

    /* Make the point types compatible. */
    if (CommonPType != (*Crv1) -> PType) {
	TCrv = CagdCoerceCrvTo(*Crv1, CommonPType, FALSE);
	CagdCrvFree(*Crv1);
	*Crv1 = TCrv;
    }
    if (CommonPType != (*Crv2) -> PType) {
	TCrv = CagdCoerceCrvTo(*Crv2, CommonPType, FALSE);
	CagdCrvFree(*Crv2);
	*Crv2 = TCrv;
    }

    if (SameOrder) {
	/* Raise the degree of the lower one. */
	if ((*Crv1) -> Order < (*Crv2) -> Order) {
	    TCrv = CagdCrvDegreeRaiseN(*Crv1, (*Crv2) -> Order);
	    CagdCrvFree(*Crv1);
	    *Crv1 = TCrv;
	}
	else if ((*Crv2) -> Order < (*Crv1) -> Order) {
	    TCrv = CagdCrvDegreeRaiseN(*Crv2, (*Crv1) -> Order);
	    CagdCrvFree(*Crv2);
	    *Crv2 = TCrv;
	}
    }

    /* If incompatible curve type - make it the same as well. */
    if ((*Crv1) -> GType != (*Crv2) -> GType) {
	/* If power basis - promote to bezier: */
	if ((*Crv1) -> GType == CAGD_CPOWER_TYPE) {
	    TCrv = CagdCnvrtPwr2BzrCrv(*Crv1);
	    CagdCrvFree(*Crv1);
	    *Crv1 = TCrv;
	}
	if ((*Crv2) -> GType == CAGD_CPOWER_TYPE) {
	    TCrv = CagdCnvrtPwr2BzrCrv(*Crv2);
	    CagdCrvFree(*Crv2);
	    *Crv2 = TCrv;
	}

	/* Now both curves may be either bezier or bspline curves. */
	if ((*Crv1) -> GType != (*Crv2) -> GType) {
	    /* If bezier basis - promote to bspline: */
	    if ((*Crv1) -> GType == CAGD_CBEZIER_TYPE) {
		TCrv = CagdCnvrtBzr2BspCrv(*Crv1);
		CagdCrvFree(*Crv1);
		*Crv1 = TCrv;
	    }
	    if ((*Crv2) -> GType == CAGD_CBEZIER_TYPE) {
		TCrv = CagdCnvrtBzr2BspCrv(*Crv2);
		CagdCrvFree(*Crv2);
		*Crv2 = TCrv;
	    }
	}
    }

    if (SameKV && SameOrder) {
	/* If bspline curve - make sure knot vectors are the same. */
	if ((*Crv1) -> GType == CAGD_CBSPLINE_TYPE) {
	    int Order = (*Crv1) -> Order;

	    KV1 = (*Crv1) -> KnotVector;
	    KV2 = (*Crv2) -> KnotVector;
	    KV1Len = CAGD_CRV_PT_LST_LEN(*Crv1) + Order;
	    KV2Len = CAGD_CRV_PT_LST_LEN(*Crv2) + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine Crv1 there. */
	    RefKV  = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TCrv = CagdCrvRefineAtParams(*Crv1, FALSE, RefKV, RefLen);
		CagdCrvFree(*Crv1);
		*Crv1 = TCrv;
		KV1 = (*Crv1) -> KnotVector;
		KV1Len = (*Crv1) -> Length + (*Crv1) -> Order;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine Crv2 there. */
	    RefKV  = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TCrv = CagdCrvRefineAtParams(*Crv2, FALSE, RefKV, RefLen);
		CagdCrvFree(*Crv2);
		*Crv2 = TCrv;
	    }
	    IritFree(RefKV);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces, makes them compatible by:				     M
* 1. Coercing their point type to be the same.				     M
* 2. Making them have the same curve type.				     M
* 3. Raising the degree of the lower one to be the same as the higher.	     M
* 4. Refining them to a common knot vector (If Bspline and SameOrder).	     M
*                                                                            M
* Note 3 is performed if SameOrder TRUE, 4 if SameKV TRUE.		     M
* Both surface are modified IN PLACE.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surfaces to be made compatible, in place.               M
*   SameUOrder:  If TRUE, this routine make sure they share the same U       M
*                order.							     M
*   SameVOrder:  If TRUE, this routine make sure they share the same V       M
*                order.							     M
*   SameUKV:     If TRUE, this routine make sure they share the same U       M
*                knot vector and hence continuity.                           *
*   SameVKV:     If TRUE, this routine make sure they share the same V       M
*                knot vector and hence continuity.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMakeSrfsCompatible, compatibility                                    M
*****************************************************************************/
CagdBType CagdMakeSrfsCompatible(CagdSrfStruct **Srf1,
				 CagdSrfStruct **Srf2,
				 CagdBType SameUOrder,
				 CagdBType SameVOrder,
				 CagdBType SameUKV,
				 CagdBType SameVKV)
{
    int i, KV1Len, KV2Len, RefLen;
    CagdRType *KV1, *KV2, *RefKV;
    CagdSrfStruct *TSrf;
    CagdPointType CommonPType;

    if ((*Srf1 == NULL) || (*Srf2 == NULL))
	return TRUE;

    if (CAGD_IS_UPERIODIC_SRF(*Srf1) != CAGD_IS_UPERIODIC_SRF(*Srf2) ||
	CAGD_IS_VPERIODIC_SRF(*Srf1) != CAGD_IS_VPERIODIC_SRF(*Srf2)) {
        if (CAGD_IS_UPERIODIC_SRF(*Srf1) || CAGD_IS_VPERIODIC_SRF(*Srf1)) {
	    TSrf = CagdCnvrtBsp2OpenSrf(*Srf1);
	    CagdSrfFree(*Srf1);
	    *Srf1 = TSrf;
	}
        if (CAGD_IS_UPERIODIC_SRF(*Srf2) || CAGD_IS_VPERIODIC_SRF(*Srf2)) {
	    TSrf = CagdCnvrtBsp2OpenSrf(*Srf2);
	    CagdSrfFree(*Srf2);
	    *Srf2 = TSrf;
	}
    }

    CommonPType = CagdMergeIrtPtType((*Srf1) -> PType, (*Srf2) -> PType);

    /* Make the point types compatible. */
    if (CommonPType != (*Srf1) -> PType) {
	TSrf = CagdCoerceSrfTo(*Srf1, CommonPType, FALSE);
	CagdSrfFree(*Srf1);
	*Srf1 = TSrf;
    }
    if (CommonPType != (*Srf2) -> PType) {
	TSrf = CagdCoerceSrfTo(*Srf2, CommonPType, FALSE);
	CagdSrfFree(*Srf2);
	*Srf2 = TSrf;
    }

    if (SameUOrder && SameVOrder) {
	/* Raise the degree of the lower one. */
	if ((*Srf1) -> UOrder < (*Srf2) -> UOrder ||
	    (*Srf1) -> VOrder < (*Srf2) -> VOrder) {
	    TSrf = CagdSrfDegreeRaiseN(*Srf1,
				       IRIT_MAX((*Srf1) -> UOrder,
					   (*Srf2) -> UOrder),
				       IRIT_MAX((*Srf1) -> VOrder,
					   (*Srf2) -> VOrder));
	    CagdSrfFree(*Srf1);
	    *Srf1 = TSrf;
	}

	if ((*Srf1) -> UOrder > (*Srf2) -> UOrder ||
	    (*Srf1) -> VOrder > (*Srf2) -> VOrder) {
	    TSrf = CagdSrfDegreeRaiseN(*Srf2,
				       IRIT_MAX((*Srf1) -> UOrder,
					   (*Srf2) -> UOrder),
				       IRIT_MAX((*Srf1) -> VOrder,
					   (*Srf2) -> VOrder));
	    CagdSrfFree(*Srf2);
	    *Srf2 = TSrf;
	}
    }
    else {
        if (SameUOrder) {
	    /* Raise the degree of the lower one. */
	    for (i = (*Srf1) -> UOrder; i < (*Srf2) -> UOrder; i++) {
	        TSrf = CagdSrfDegreeRaise(*Srf1, CAGD_CONST_U_DIR);
		CagdSrfFree(*Srf1);
		*Srf1 = TSrf;
	    }
	    for (i = (*Srf2) -> UOrder; i < (*Srf1) -> UOrder; i++) {
	        TSrf = CagdSrfDegreeRaise(*Srf2, CAGD_CONST_U_DIR);
		CagdSrfFree(*Srf2);
		*Srf2 = TSrf;
	    }
	}
        if (SameVOrder) {
	    for (i = (*Srf1) -> VOrder; i < (*Srf2) -> VOrder; i++) {
	        TSrf = CagdSrfDegreeRaise(*Srf1, CAGD_CONST_V_DIR);
		CagdSrfFree(*Srf1);
		*Srf1 = TSrf;
	    }
	    for (i = (*Srf2) -> VOrder; i < (*Srf1) -> VOrder; i++) {
	        TSrf = CagdSrfDegreeRaise(*Srf2, CAGD_CONST_V_DIR);
		CagdSrfFree(*Srf2);
		*Srf2 = TSrf;
	    }
	}
    }

    /* If incompatible surface type - make it the same as well. */
    if ((*Srf1) -> GType != (*Srf2) -> GType) {
	/* If power basis - promote to bezier: */
	if ((*Srf1) -> GType == CAGD_SPOWER_TYPE) {
	    TSrf = CagdCnvrtPwr2BzrSrf(*Srf1);
	    CagdSrfFree(*Srf1);
	    *Srf1 = TSrf;
	}
	if ((*Srf2) -> GType == CAGD_SPOWER_TYPE) {
	    TSrf = CagdCnvrtPwr2BzrSrf(*Srf2);
	    CagdSrfFree(*Srf2);
	    *Srf2 = TSrf;
	}

	/* Now both surfaces may be either bezier or bspline surfaces. */
	if ((*Srf1) -> GType != (*Srf2) -> GType) {
	    /* If bezier basis - promote to bspline: */
	    if ((*Srf1) -> GType == CAGD_SBEZIER_TYPE) {
		TSrf = CagdCnvrtBzr2BspSrf(*Srf1);
		CagdSrfFree(*Srf1);
		*Srf1 = TSrf;
	    }
	    if ((*Srf2) -> GType == CAGD_SBEZIER_TYPE) {
		TSrf = CagdCnvrtBzr2BspSrf(*Srf2);
		CagdSrfFree(*Srf2);
		*Srf2 = TSrf;
	    }
	}
    }

    if ((*Srf1) -> GType == CAGD_SBSPLINE_TYPE) {
	/* If bspline surface - make sure knot vectors are the same. */

	if (SameUKV && SameUOrder) {
	    /* Handle the U Direction. */
	    int Order = (*Srf1) -> UOrder;
	    KV1 = (*Srf1) -> UKnotVector;
	    KV2 = (*Srf2) -> UKnotVector;
	    KV1Len = CAGD_SRF_UPT_LST_LEN(*Srf1) + Order;
	    KV2Len = CAGD_SRF_UPT_LST_LEN(*Srf2) + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine Srf1 there. */
	    RefKV  = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TSrf = CagdSrfRefineAtParams(*Srf1, CAGD_CONST_U_DIR,
					     FALSE, RefKV, RefLen);
		CagdSrfFree(*Srf1);
		*Srf1 = TSrf;
		KV1 = (*Srf1) -> UKnotVector;
		KV1Len = (*Srf1) -> ULength + (*Srf1) -> UOrder;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine Srf2 there. */
	    RefKV  = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TSrf = CagdSrfRefineAtParams(*Srf2, CAGD_CONST_U_DIR,
					     FALSE, RefKV, RefLen);
		CagdSrfFree(*Srf2);
		*Srf2 = TSrf;
	    }
	    IritFree(RefKV);
	}

	if (SameVKV && SameVOrder) {
	    /* Handle the V Direction. */
	    int Order = (*Srf1) -> VOrder;

	    KV1 = (*Srf1) -> VKnotVector;
	    KV2 = (*Srf2) -> VKnotVector;
	    KV1Len = CAGD_SRF_VPT_LST_LEN(*Srf1) + Order;
	    KV2Len = CAGD_SRF_VPT_LST_LEN(*Srf2) + Order;

	    /* Affine map second knot vector to span same parametric domain. */
	    BspKnotAffineTrans(KV2, KV2Len, KV1[Order - 1] - KV2[Order - 1],
			       (KV1[KV1Len - Order] - KV1[Order - 1]) /
			       (KV2[KV2Len - Order] - KV2[Order - 1]));

	    /* Find knots in KV2 which are not in KV1 and refine Srf1 there. */
	    RefKV  = BspKnotSubtrTwo(&KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TSrf = CagdSrfRefineAtParams(*Srf1, CAGD_CONST_V_DIR,
					     FALSE, RefKV, RefLen);
		CagdSrfFree(*Srf1);
		*Srf1 = TSrf;
		KV1 = (*Srf1) -> VKnotVector;
		KV1Len = (*Srf1) -> VLength + (*Srf1) -> VOrder;
	    }
	    IritFree(RefKV);

	    /* Find knots in KV1 which are not in KV2 and refine Srf2 there. */
	    RefKV  = BspKnotSubtrTwo(&KV1[Order - 1], KV1Len - Order * 2 + 2,
				     &KV2[Order - 1], KV2Len - Order * 2 + 2,
				     &RefLen);
	    if (RefLen > 0) {
		TSrf = CagdSrfRefineAtParams(*Srf2, CAGD_CONST_V_DIR,
					     FALSE, RefKV, RefLen);
		CagdSrfFree(*Srf2);
		*Srf2 = TSrf;
	    }
	    IritFree(RefKV);
	}
    }

    return TRUE;
}

