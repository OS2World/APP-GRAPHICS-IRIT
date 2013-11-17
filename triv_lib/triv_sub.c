/******************************************************************************
* Triv_Sub.c - Computes subdivision of tri-variates.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a tri-variate, subdivides it at parameter value t in direction	     M
* Dir.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to subdivide.                                       M
*   t:        Parameter to subdivide at.                                     M
*   Dir:      Direction of subdivision.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   A list of two trivariates, result of the subdivision.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVSubdivAtParam, trivariates                                         M
*****************************************************************************/
TrivTVStruct *TrivTVSubdivAtParam(const TrivTVStruct *TV,
				  CagdRType t,
				  TrivTVDirType Dir)
{
    int i, j, KVLen, Index1, Index2,
	UOrder = TV -> UOrder,
	VOrder = TV -> VOrder,
	WOrder = TV -> WOrder,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength;
    CagdRType *RefKV;
    CagdSrfStruct *Srf, *LSrf, *RSrf;
    TrivTVStruct *RTV, *LTV;

    if (TRIV_IS_BEZIER_TV(TV)) {
	RTV = TrivBzrTVNew(ULength, VLength, WLength, TV -> PType);
	LTV = TrivBzrTVNew(ULength, VLength, WLength, TV -> PType);
    }
    else if (TRIV_IS_BSPLINE_TV(TV)) {
	switch (Dir) {
	    case TRIV_CONST_U_DIR:
		RefKV = TV -> UKnotVector;
		KVLen = UOrder + ULength;
		Index1 = BspKnotLastIndexL(RefKV, KVLen, t);
		if (Index1 + 1 < UOrder)
		    Index1 = UOrder - 1;
		Index2 = BspKnotFirstIndexG(RefKV, KVLen, t);
		if (Index2 > ULength)
		    Index2 = ULength;
		LTV = TrivBspTVNew(Index1 + 1, VLength, WLength,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);
		RTV = TrivBspTVNew(ULength - Index2 + UOrder, VLength, WLength,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);

		/* Update the new knot vectors. */
		CAGD_GEN_COPY(LTV -> UKnotVector,
			      TV -> UKnotVector,
			      sizeof(CagdRType) * (Index1 + 1));
		/* Close the knot vector with multiplicity Order: */
		for (j = Index1 + 1; j <= Index1 + UOrder; j++)
		    LTV -> UKnotVector[j] = t;
		CAGD_GEN_COPY(&RTV -> UKnotVector[UOrder],
			      &TV -> UKnotVector[Index2],
			      sizeof(CagdRType) *
				  (ULength + UOrder - Index2));
		/* Make sure knot vector starts with multiplicity Order: */
		for (j = 0; j < UOrder; j++)
		    RTV -> UKnotVector[j] = t;
	    
		/* And copy the other direction knot vectors. */
		CAGD_GEN_COPY(LTV -> VKnotVector,
			      TV -> VKnotVector,
			      sizeof(CagdRType) * (VOrder + VLength));
		CAGD_GEN_COPY(RTV -> VKnotVector,
			      TV -> VKnotVector,
			      sizeof(CagdRType) * (VOrder + VLength));
		CAGD_GEN_COPY(LTV -> WKnotVector,
			      TV -> WKnotVector,
			      sizeof(CagdRType) * (WOrder + WLength));
		CAGD_GEN_COPY(RTV -> WKnotVector,
			      TV -> WKnotVector,
			      sizeof(CagdRType) * (WOrder + WLength));
		break;
	    case TRIV_CONST_V_DIR:
		RefKV = TV -> VKnotVector;
		KVLen = VOrder + VLength;
		Index1 = BspKnotLastIndexL(RefKV, KVLen, t);
		if (Index1 + 1 < VOrder)
		    Index1 = VOrder - 1;
		Index2 = BspKnotFirstIndexG(RefKV, KVLen, t);
		if (Index2 > VLength)
		    Index2 = VLength;
		LTV = TrivBspTVNew(ULength, Index1 + 1, WLength,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);
		RTV = TrivBspTVNew(ULength, VLength - Index2 + VOrder, WLength,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);

		/* Update the new knot vectors. */
		CAGD_GEN_COPY(LTV -> VKnotVector,
			      TV -> VKnotVector,
			      sizeof(CagdRType) * (Index1 + 1));
		/* Close the knot vector with multiplicity Order: */
		for (j = Index1 + 1; j <= Index1 + VOrder; j++)
		    LTV -> VKnotVector[j] = t;
		CAGD_GEN_COPY(&RTV -> VKnotVector[VOrder],
			      &TV -> VKnotVector[Index2],
			      sizeof(CagdRType) *
				  (VLength + VOrder - Index2));
		/* Make sure knot vector starts with multiplicity Order: */
		for (j = 0; j < VOrder; j++)
		    RTV -> VKnotVector[j] = t;
	    
		/* And copy the other direction knot vectors. */
		CAGD_GEN_COPY(LTV -> UKnotVector,
			      TV -> UKnotVector,
			      sizeof(CagdRType) * (UOrder + ULength));
		CAGD_GEN_COPY(RTV -> UKnotVector,
			      TV -> UKnotVector,
			      sizeof(CagdRType) * (UOrder + ULength));
		CAGD_GEN_COPY(LTV -> WKnotVector,
			      TV -> WKnotVector,
			      sizeof(CagdRType) * (WOrder + WLength));
		CAGD_GEN_COPY(RTV -> WKnotVector,
			      TV -> WKnotVector,
			      sizeof(CagdRType) * (WOrder + WLength));
		break;
	    case TRIV_CONST_W_DIR:
		RefKV = TV -> WKnotVector;
		KVLen = WOrder + WLength;
		Index1 = BspKnotLastIndexL(RefKV, KVLen, t);
		if (Index1 + 1 < WOrder)
		    Index1 = WOrder - 1;
		Index2 = BspKnotFirstIndexG(RefKV, KVLen, t);
		if (Index2 > WLength)
		    Index2 = WLength;
		LTV = TrivBspTVNew(ULength, VLength, Index1 + 1,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);
		RTV = TrivBspTVNew(ULength, VLength, WLength - Index2 + WOrder,
				   TV -> UOrder, TV -> VOrder, TV -> WOrder,
				   TV -> PType);

		/* Update the new knot vectors. */
		CAGD_GEN_COPY(LTV -> WKnotVector,
			      TV -> WKnotVector,
			      sizeof(CagdRType) * (Index1 + 1));
		/* Close the knot vector with multiplicity Order: */
		for (j = Index1 + 1; j <= Index1 + WOrder; j++)
		    LTV -> WKnotVector[j] = t;
		CAGD_GEN_COPY(&RTV -> WKnotVector[WOrder],
			      &TV -> WKnotVector[Index2],
			      sizeof(CagdRType) *
				  (WLength + WOrder - Index2));
		/* Make sure knot vector starts with multiplicity Order: */
		for (j = 0; j < WOrder; j++)
		    RTV -> WKnotVector[j] = t;
	    
		/* And copy the other direction knot vectors. */
		CAGD_GEN_COPY(LTV -> UKnotVector,
			      TV -> UKnotVector,
			      sizeof(CagdRType) * (UOrder + ULength));
		CAGD_GEN_COPY(RTV -> UKnotVector,
			      TV -> UKnotVector,
			      sizeof(CagdRType) * (UOrder + ULength));
		CAGD_GEN_COPY(LTV -> VKnotVector,
			      TV -> VKnotVector,
			      sizeof(CagdRType) * (VOrder + VLength));
		CAGD_GEN_COPY(RTV -> VKnotVector,
			      TV -> VKnotVector,
			      sizeof(CagdRType) * (VOrder + VLength));
		break;
	    default:
		TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
		RTV = LTV = NULL;
		break;
	}
    }
    else {
	TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_TRIVAR);
	return NULL;
    }

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    for (i = 0; i < WLength; i++) {
		Srf = TrivSrfFromMesh(TV, i, TRIV_CONST_W_DIR);
		LSrf = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR);
		RSrf = LSrf -> Pnext;
		TrivSrfToMesh(LSrf, i, TRIV_CONST_W_DIR, LTV);
		TrivSrfToMesh(RSrf, i, TRIV_CONST_W_DIR, RTV);

		CagdSrfFree(Srf);
		CagdSrfFree(LSrf);
		CagdSrfFree(RSrf);
	    }
	    break;
	case TRIV_CONST_V_DIR:
	    for (i = 0; i < ULength; i++) {
		Srf = TrivSrfFromMesh(TV, i, TRIV_CONST_U_DIR);
		LSrf = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR);
		RSrf = LSrf -> Pnext;
		TrivSrfToMesh(LSrf, i, TRIV_CONST_U_DIR, LTV);
		TrivSrfToMesh(RSrf, i, TRIV_CONST_U_DIR, RTV);

		CagdSrfFree(Srf);
		CagdSrfFree(LSrf);
		CagdSrfFree(RSrf);
	    }
	    break;
	case TRIV_CONST_W_DIR:
	    for (i = 0; i < ULength; i++) {
		Srf = TrivSrfFromMesh(TV, i, TRIV_CONST_U_DIR);
		LSrf = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_V_DIR);
		RSrf = LSrf -> Pnext;
		TrivSrfToMesh(LSrf, i, TRIV_CONST_U_DIR, LTV);
		TrivSrfToMesh(RSrf, i, TRIV_CONST_U_DIR, RTV);

		CagdSrfFree(Srf);
		CagdSrfFree(LSrf);
		CagdSrfFree(RSrf);
	    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_CONST_UVW);
	    break;
    }

    LTV -> Pnext = RTV;
    return LTV;
}
