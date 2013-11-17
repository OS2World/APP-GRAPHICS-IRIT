/******************************************************************************
* Trim_gen.c - generic routine to interface to different free from types.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include "trim_loc.h"

#define VEC_FIELD_TRIES	10
#define VEC_FIELD_START_STEP 1e-6
#define TRIM_CRV_TOL	1e-4

#ifdef DEBUG
#undef TrimCrvSegFree
#undef TrimCrvSegFreeList
#undef TrimCrvFree
#undef TrimCrvFreeList
#undef TrimSrfFree
#undef TrimSrfFreeList
#endif /* DEBUG */

static void UpdateTrimCrvDomains(TrimCrvStruct *TrimCrvs);
static int UpdateMatchTrimCrv(TrimSrfStruct *TrimSrf,
			      TrimCrvSegStruct *UpdatedTrimCrvSegs,
			      CagdPType TrimCrvEndPt);
static int UpdateSeamTrimCrv(TrimSrfStruct *TrimSrf, CagdPType TrimCrvEndPt);
static int UpdateOneSeamTrimCrv(TrimSrfStruct *TrimSrf, CagdSrfBndryType Bndry);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates a trimming curve segment structure. Allows periodic and float    M
* end conditions - converts them to open end.				     M
*   Input curves are used in place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrv:     A UV curve. Only the E2/P2 potion of the curve is considered. M
*   EucCrv:    Optional Euclidean curve. Must be an E3 curve.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvSegStruct *:   A trimming curve segment structure.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvSegNew, allocation                                                M
*****************************************************************************/
TrimCrvSegStruct *TrimCrvSegNew(CagdCrvStruct *UVCrv, CagdCrvStruct *EucCrv)
{
    TrimCrvSegStruct
	*TrimCrvSeg = (TrimCrvSegStruct *)
	    IritMalloc(sizeof(TrimCrvSegStruct));

    if (UVCrv != NULL) {
        if (CAGD_IS_BSPLINE_CRV(UVCrv) && !BspCrvHasOpenEC(UVCrv)) {
	    /* Must be open end trimming crv. */
	    CagdCrvStruct *TCrv;

	    if (CAGD_IS_PERIODIC_CRV(UVCrv)) {
	        TCrv = CagdCnvrtPeriodic2FloatCrv(UVCrv);
		CagdCrvFree(UVCrv);
		UVCrv = TCrv;
	    }
	    TCrv = CagdCnvrtFloat2OpenCrv(UVCrv);
	    CagdCrvFree(UVCrv);
	    UVCrv = TCrv;
	}

	UVCrv -> Pnext = NULL;
    }

    TrimCrvSeg -> UVCrv = UVCrv;

    if (EucCrv != NULL) {
        if (CAGD_IS_BSPLINE_CRV(EucCrv) && !BspCrvHasOpenEC(EucCrv)) {
	    /* Must be open end trimming crv. */
	    CagdCrvStruct *TCrv;

	    if (CAGD_IS_PERIODIC_CRV(EucCrv)) {
	        TCrv = CagdCnvrtPeriodic2FloatCrv(EucCrv);
		CagdCrvFree(EucCrv);
		EucCrv = TCrv;
	    }
	    TCrv = CagdCnvrtFloat2OpenCrv(EucCrv);
	    CagdCrvFree(EucCrv);
	    EucCrv = TCrv;
	}

	EucCrv -> Pnext = NULL;
    }

    TrimCrvSeg -> EucCrv = EucCrv;

    TrimCrvSeg -> Pnext = NULL;
    TrimCrvSeg -> Attr = NULL;

    return TrimCrvSeg;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a trimming curve segment structure.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSeg: A trimming curve segment to duplicate.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvSegStruct *:   A trimming curve segment structure.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvSegCopy, allocation                                               M
*****************************************************************************/
TrimCrvSegStruct *TrimCrvSegCopy(TrimCrvSegStruct *TrimCrvSeg)
{
    TrimCrvSegStruct
	*NewTrimCrvSeg = (TrimCrvSegStruct *)
	    IritMalloc(sizeof(TrimCrvSegStruct));

    NewTrimCrvSeg -> UVCrv = CagdCrvCopy(TrimCrvSeg -> UVCrv);
    if (TrimCrvSeg -> EucCrv)
	NewTrimCrvSeg -> EucCrv = CagdCrvCopy(TrimCrvSeg -> EucCrv);
    else
	NewTrimCrvSeg -> EucCrv = NULL;
    NewTrimCrvSeg -> Pnext = NULL;
    NewTrimCrvSeg -> Attr = NULL;

    return NewTrimCrvSeg;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of trimming curve segment structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSegList:   To be copied.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvSegStruct *:  A duplicated list of trimming curve segments.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvSegCopyList, copy                                                 M
*****************************************************************************/
TrimCrvSegStruct *TrimCrvSegCopyList(TrimCrvSegStruct *TrimCrvSegList)
{
    TrimCrvSegStruct *TrimCrvSegTemp, *NewTrimCrvSegList;

    if (TrimCrvSegList == NULL)
	return NULL;
    TrimCrvSegTemp = NewTrimCrvSegList = TrimCrvSegCopy(TrimCrvSegList);
    TrimCrvSegList = TrimCrvSegList -> Pnext;
    while (TrimCrvSegList) {
	TrimCrvSegTemp -> Pnext = TrimCrvSegCopy(TrimCrvSegList);
	TrimCrvSegTemp = TrimCrvSegTemp -> Pnext;
	TrimCrvSegList = TrimCrvSegList -> Pnext;
    }
    return NewTrimCrvSegList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a trimming curve segment structure.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSeg: A trimming curve segment to free.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvSegFree, allocation                                               M
*****************************************************************************/
void TrimCrvSegFree(TrimCrvSegStruct *TrimCrvSeg)
{
    CagdCrvFree(TrimCrvSeg -> UVCrv);
    CagdCrvFree(TrimCrvSeg -> EucCrv);
    IP_ATTR_FREE_ATTRS(TrimCrvSeg -> Attr);
    IritFree(TrimCrvSeg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a list of trimming curve segment structures.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSegList: A list of trimming curve segments to free.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvSegFreeList, allocation                                           M
*****************************************************************************/
void TrimCrvSegFreeList(TrimCrvSegStruct *TrimCrvSegList)
{
    TrimCrvSegStruct *TrimCrvTemp;

    while (TrimCrvSegList) {
	TrimCrvTemp = TrimCrvSegList -> Pnext;
	TrimCrvSegFree(TrimCrvSegList);
	TrimCrvSegList = TrimCrvTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates a trimming curve structure.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSegList:    List of trimming curve segments forming the trimming  M
*                      curve.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:   A trimmig curve.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvNew, allocation                                                   M
*****************************************************************************/
TrimCrvStruct *TrimCrvNew(TrimCrvSegStruct *TrimCrvSegList)
{
   TrimCrvStruct
	*TrimCrv = (TrimCrvStruct *)
	    IritMalloc(sizeof(TrimCrvStruct));

    TrimCrv -> TrimCrvSegList = TrimCrvSegList;
    TrimCrv -> Pnext = NULL;
    TrimCrv -> Attr = NULL;

    return TrimCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a trimming curve structure.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrv:   A trimming curve to duplicate.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:   A trimming curve structure.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvCopy, allocation                                                  M
*****************************************************************************/
TrimCrvStruct *TrimCrvCopy(TrimCrvStruct *TrimCrv)
{
    TrimCrvStruct
	*NewTrimCrv = (TrimCrvStruct *)
	    IritMalloc(sizeof(TrimCrvStruct));

    NewTrimCrv -> TrimCrvSegList =
	TrimCrvSegCopyList(TrimCrv -> TrimCrvSegList);
    NewTrimCrv -> Pnext = NULL;
    NewTrimCrv -> Attr = NULL;

    return NewTrimCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of trimming curve structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList:   To be copied.                                	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:  A duplicated list of trimming curves.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvCopyList, copy                                                    M
*****************************************************************************/
TrimCrvStruct *TrimCrvCopyList(TrimCrvStruct *TrimCrvList)
{
    TrimCrvStruct *TrimCrvTemp, *NewTrimCrvList;

    if (TrimCrvList == NULL)
	return NULL;
    TrimCrvTemp = NewTrimCrvList = TrimCrvCopy(TrimCrvList);
    TrimCrvList = TrimCrvList -> Pnext;
    while (TrimCrvList) {
	TrimCrvTemp -> Pnext = TrimCrvCopy(TrimCrvList);
	TrimCrvTemp = TrimCrvTemp -> Pnext;
	TrimCrvList = TrimCrvList -> Pnext;
    }
    return NewTrimCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a trimming curve structure.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrv: A trimming curve to free.                   	      	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvFree, allocation                                                  M
*****************************************************************************/
void TrimCrvFree(TrimCrvStruct *TrimCrv)
{
    TrimCrvSegFreeList(TrimCrv -> TrimCrvSegList);
    IP_ATTR_FREE_ATTRS(TrimCrv -> Attr);
    IritFree(TrimCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a list of trimming curve structures.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList: A list of trimming curve to free.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvFreeList, allocation                     	                     M
*****************************************************************************/
void TrimCrvFreeList(TrimCrvStruct *TrimCrvList)
{
    TrimCrvStruct *TrimCrvTemp;

    while (TrimCrvList) {
	TrimCrvTemp = TrimCrvList -> Pnext;
	TrimCrvFree(TrimCrvList);
	TrimCrvList = TrimCrvTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a trimmed surface.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             Surface to make into a trimmed surface.                 M
*   TrimCrvList:     A list of trimming curves.				     M
*   HasTopLvlTrim:   Do we have a top level outer most trimming curve?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *: The trimmed surface.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfNew, allocation,                                                  M
*****************************************************************************/
TrimSrfStruct *TrimSrfNew(CagdSrfStruct *Srf,
			  TrimCrvStruct *TrimCrvList,
			  CagdBType HasTopLvlTrim)
{
    TrimSrfStruct
	*TrimSrf = (TrimSrfStruct *) IritMalloc(sizeof(TrimSrfStruct));

    TrimSrf -> Srf = Srf;
    TrimSrf -> Pnext = NULL;
    TrimSrf -> Attr = NULL;
    TrimSrf -> Tags = 0;

    if (!HasTopLvlTrim) {
	CagdRType **Points, UMin, UMax, VMin, VMax;
	TrimCrvStruct *TrimCrv;
	CagdCrvStruct
	    *Crv = BspCrvNew(5, 2, CAGD_PT_E2_TYPE);

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	BspKnotUniformOpen(5, 2, Crv -> KnotVector);
	Points = Crv -> Points;

	Points[1][0] = UMin;
	Points[2][0] = VMin;
	Points[1][1] = UMax;
	Points[2][1] = VMin;
	Points[1][2] = UMax;
	Points[2][2] = VMax;
	Points[1][3] = UMin;
	Points[2][3] = VMax;
	Points[1][4] = UMin;
	Points[2][4] = VMin;

	TrimCrv = TrimCrvNew(TrimCrvSegNew(Crv, NULL));

	TrimCrv -> Pnext = TrimCrvList;
	TrimSrf -> TrimCrvList = TrimCrv;
    }
    else
	TrimSrf -> TrimCrvList = TrimCrvList;

    /* Make sure the domains of the trimming curves are reasonable. */
    UpdateTrimCrvDomains(TrimSrf -> TrimCrvList);

    TrimSrfVerifyTrimCrvsValidity(TrimSrf);

    return TrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a trimmed surface.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             Surface to make into a trimmed surface.                 M
*   TrimCrvList:     A list of trimming curves, as regular curves.	     M
*   HasTopLvlTrim:   Do we have a top level outer most trimming curve?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *: The trimmed surface.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfNew2, allocation                                                  M
*****************************************************************************/
TrimSrfStruct *TrimSrfNew2(CagdSrfStruct *Srf,
			   CagdCrvStruct *TrimCrvList,
			   CagdBType HasTopLvlTrim)
{
    TrimCrvStruct
	*TrimCrvList2 = NULL;

    while (TrimCrvList) {
	CagdCrvStruct
	    *TrimCrvNext = TrimCrvList -> Pnext;
	TrimCrvStruct
	    *TrimCrv = TrimCrvNew(TrimCrvSegNew(TrimCrvList, NULL));

	TrimCrvList = TrimCrvNext;

	IRIT_LIST_PUSH(TrimCrv, TrimCrvList2);
    }

    return TrimSrfNew(Srf, TrimCrvList2, HasTopLvlTrim);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Affine transform the trimming curves' domains to a reasonable value.     *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimCrvs:   To measure their length and adjust their domain.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateTrimCrvDomains(TrimCrvStruct *TrimCrvs)
{
    for ( ; TrimCrvs != NULL; TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv,
		*EucCrv = TrimCrvSegs -> EucCrv;

	    if (CAGD_IS_BEZIER_CRV(UVCrv)) {
		UVCrv -> Order = UVCrv -> Length;
		UVCrv -> KnotVector = BspKnotUniformOpen(UVCrv -> Length,
					                 UVCrv -> Order, NULL);
		UVCrv -> GType = CAGD_CBSPLINE_TYPE;
	    }
	    BspKnotAffineTrans2(UVCrv -> KnotVector,
				UVCrv -> Length + UVCrv -> Order, 0.0, 1.0);

	    if (EucCrv != NULL) {
		if (CAGD_IS_BEZIER_CRV(EucCrv)) {
		    EucCrv -> Order = EucCrv -> Length;
		    EucCrv -> KnotVector = BspKnotUniformOpen(EucCrv -> Length,
							      EucCrv -> Order,
							      NULL);
		    EucCrv -> GType = CAGD_CBSPLINE_TYPE;
		}
		BspKnotAffineTrans2(EucCrv -> KnotVector, 
				    EucCrv -> Length + EucCrv -> Order,
				    0.0, 1.0);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Verify that all trimming curves are indeed in the parametric domain of   M
* the surface and that all of them matches neighboring curves.               M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   To verify the validity of the trimming curves.  This includes M
*	       the verification of the continuity of the trimming loops and  M
*	       the inclusion in the domain of the trimming curves.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if valid, FALSE if cannot correct the trimming curves.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfVerifyTrimCrvsValidity                                            M
*****************************************************************************/
int TrimSrfVerifyTrimCrvsValidity(TrimSrfStruct *TrimSrf)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct
	*Srf = TrimSrf -> Srf;
    TrimCrvStruct *TrimCrvs;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax); 

    /* Verify that trimming curves are indeed in the surface domain . */
    for (TrimCrvs = TrimSrf -> TrimCrvList;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv;
	    int i,
		Len = UVCrv -> Length;

	    for (i = 0; i < Len; i++) {
	        CagdPType Pt;

		CagdCoerceToE2(Pt, UVCrv -> Points, i, UVCrv -> PType);

		if (Pt[0] < UMin - IRIT_EPS ||
		    Pt[0] > UMax + IRIT_EPS ||
		    Pt[1] < VMin - IRIT_EPS ||
		    Pt[1] > VMax + IRIT_EPS) {
		    /* Out of the domain by too much - can not do much. */
		    return FALSE;
		}
		if (Pt[0] < UMin)
		    Pt[0] = UMin;
		if (Pt[0] > UMin)
		    Pt[0] = UMax;
		if (Pt[1] < UMin)
		    Pt[1] = VMin;
		if (Pt[1] > UMax)
		    Pt[1] = VMax;
	    }
	}
    }

    /* Verify the continuity between the different trimming curves. */
    for (TrimCrvs = TrimSrf -> TrimCrvList;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv;
	    int Len = UVCrv -> Length;
	    CagdPType Pt1, Pt2, Pt12;

	    CagdCoerceToE2(Pt1, UVCrv -> Points, 0, UVCrv -> PType);
	    CagdCoerceToE2(Pt2, UVCrv -> Points, Len - 1, UVCrv -> PType);

	    /* Lets see if it is a closed curve. */
	    if (IRIT_PT_APX_EQ_E2_EPS(Pt1, Pt2, TRIM_CRV_TOL)) {
	        IRIT_PT_BLEND(Pt12, Pt1, Pt2, 0.5);

		/* Make precisely the same. */
		switch (UVCrv -> PType) {
		    case CAGD_PT_P2_TYPE:
		    case CAGD_PT_P3_TYPE:
		        UVCrv -> Points[0][0] = UVCrv -> Points[0][Len - 1];
			IRIT_PT_SCALE(Pt12, UVCrv -> Points[0][0]);
		    case CAGD_PT_E2_TYPE:
		    case CAGD_PT_E3_TYPE:
		        UVCrv -> Points[1][0] = Pt12[0];
			UVCrv -> Points[2][0] = Pt12[1];
			UVCrv -> Points[1][Len - 1] = Pt12[0];
			UVCrv -> Points[2][Len - 1] = Pt12[1];
			break;
		    default:
		        assert(0);
			return FALSE;
		}
	        continue;
	    }

	    /* Find the closest neighbor and update to be identical. */
	    if ((!UpdateMatchTrimCrv(TrimSrf, TrimCrvSegs, Pt1) &&
		 !UpdateSeamTrimCrv(TrimSrf, Pt1)) ||
		((!UpdateMatchTrimCrv(TrimSrf, TrimCrvSegs, Pt2) &&
		  !UpdateSeamTrimCrv(TrimSrf, Pt2)))) {
		/* If a single trimming curve, try to close it. */
	        if (TrimSrf -> TrimCrvList -> Pnext == NULL &&
		    TrimSrf -> TrimCrvList -> TrimCrvSegList -> Pnext == NULL) {
		    IRIT_PT_BLEND(Pt12, Pt1, Pt2, 0.5);

		    switch (UVCrv -> PType) {
			case CAGD_PT_P2_TYPE:
			case CAGD_PT_P3_TYPE:
		            UVCrv -> Points[0][0] =
			        UVCrv -> Points[0][Len - 1];
			    IRIT_PT_SCALE(Pt12, UVCrv -> Points[0][0]);
			case CAGD_PT_E2_TYPE:
			case CAGD_PT_E3_TYPE:
		            UVCrv -> Points[1][0] = Pt12[0];
		            UVCrv -> Points[2][0] = Pt12[1];
		            UVCrv -> Points[1][Len - 1] = Pt12[0];
		            UVCrv -> Points[2][Len - 1] = Pt12[1];
			    break;
		        default:
			    assert(0);
			    return FALSE;
		    }
		}
		else
		    return FALSE;
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the given trimmed surface so that the given end point of the     *
* trimming curve segment will match another trimming curve, precisely.       *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:            The updated trimmed surface.                         *
*   UpdatedTrimCrvSegs: The updated trimmed curve segment.                   *
*   TrimCrvEndPt:       End point of updated trimmed curve segment.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.                          *
*****************************************************************************/
static int UpdateMatchTrimCrv(TrimSrfStruct *TrimSrf,
			      TrimCrvSegStruct *UpdatedTrimCrvSegs,
			      CagdPType TrimCrvEndPt)
{
    TrimCrvStruct *TrimCrvs;

    /* Verify that trimming curves are indeed in the surface domain . */
    for (TrimCrvs = TrimSrf -> TrimCrvList;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv;
	    int Indx,
		Len = UVCrv -> Length;
	    CagdPType Pt1, Pt2;
	    CagdRType
		**Points = UVCrv -> Points;

	    if (TrimCrvSegs == UpdatedTrimCrvSegs)
	        continue;

	    CagdCoerceToE2(Pt1, Points, 0, UVCrv -> PType);
	    CagdCoerceToE2(Pt2, Points, Len - 1, UVCrv -> PType);

	    if (IRIT_PT_APX_EQ_E2_EPS(TrimCrvEndPt, Pt1, TRIM_CRV_TOL))
		Indx = 0;
	    else if (IRIT_PT_APX_EQ_E2_EPS(TrimCrvEndPt, Pt2, TRIM_CRV_TOL))
		Indx = Len - 1;
	    else
	        Indx = -1;

	    if (Indx >= 0) {
		/* We found other edge that end at TrimCrvEndPt - make same. */
		if (CAGD_IS_RATIONAL_CRV(UVCrv)) {
		    Points[1][Indx] = TrimCrvEndPt[0] * Points[0][Indx];
		    Points[2][Indx] = TrimCrvEndPt[1] * Points[0][Indx];
		}
		else {
		    Points[1][Indx] = TrimCrvEndPt[0];
		    Points[2][Indx] = TrimCrvEndPt[1];
		}

		return TRUE;
	    }
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   We have end point that is connected to nothing else.  If it is on a      *
* zero length ,degenerated, boundary curve or along a shared seam of the     *
* surface, we allow some freedom here to be corrected by automatically       *
* adding the seam edges at the proper ranges.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:            The updated trimmed surface.                         *
*   TrimCrvEndPt:       End point of updated trimmed curve segment.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.                          *
*****************************************************************************/
static int UpdateSeamTrimCrv(TrimSrfStruct *TrimSrf,
			     CagdPType TrimCrvEndPt)
{
    int NumOfSeamPoints = 0,
	Axis = -1;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct
	*Srf = TrimSrf -> Srf;
    TrimCrvStruct *TrimCrvs;
    CagdPType SeamPoints[6];

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax); 

    if (IRIT_APX_EQ_EPS(TrimCrvEndPt[0], UMin, TRIM_CRV_TOL) &&
	CagdIsZeroLenSrfBndry(Srf, CAGD_U_MIN_BNDRY, TRIM_CRV_TOL)) {
	/* Find a match along the UMin seam and add the missing segment. */
        if (UpdateOneSeamTrimCrv(TrimSrf, CAGD_U_MIN_BNDRY))
	    return TRUE;
    }

    if (IRIT_APX_EQ_EPS(TrimCrvEndPt[0], UMax, TRIM_CRV_TOL) &&
	CagdIsZeroLenSrfBndry(Srf, CAGD_U_MAX_BNDRY, TRIM_CRV_TOL)) {
	/* Find a match along the UMax seam and add the missing segment. */
	if (UpdateOneSeamTrimCrv(TrimSrf, CAGD_U_MAX_BNDRY))
	    return TRUE;
    }

    if (IRIT_APX_EQ_EPS(TrimCrvEndPt[1], VMin, TRIM_CRV_TOL) &&
	CagdIsZeroLenSrfBndry(Srf, CAGD_V_MIN_BNDRY, TRIM_CRV_TOL)) {
	/* Find a match along the VMin seam and add the missing segment. */
	if (UpdateOneSeamTrimCrv(TrimSrf, CAGD_V_MIN_BNDRY))
	    return TRUE;
    }

    if (IRIT_APX_EQ_EPS(TrimCrvEndPt[1], VMax, TRIM_CRV_TOL) &&
	CagdIsZeroLenSrfBndry(Srf, CAGD_V_MAX_BNDRY, TRIM_CRV_TOL)) {
	/* Find a match along the VMax seam and add the missing segment. */
	if (UpdateOneSeamTrimCrv(TrimSrf, CAGD_V_MAX_BNDRY))
	    return TRUE;
    }

    if ((IRIT_APX_EQ_EPS(TrimCrvEndPt[0], UMin, TRIM_CRV_TOL) ||
	 IRIT_APX_EQ_EPS(TrimCrvEndPt[0], UMax, TRIM_CRV_TOL)) &&
	CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR)) {
	/* The point is on UMin/UMax boundary that is also a shared seam. */
        Axis = 0;
    }
    else if ((IRIT_APX_EQ_EPS(TrimCrvEndPt[1], VMin, TRIM_CRV_TOL) ||
	      IRIT_APX_EQ_EPS(TrimCrvEndPt[1], VMax, TRIM_CRV_TOL)) &&
	     CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR)) {
	/* The point is on VMin/VMax boundary that is also a shared seam. */
        Axis = 1;
    }
    else
        return FALSE;

    /* Search all end points of trimming curves for points on shared seam. */
    for (TrimCrvs = TrimSrf -> TrimCrvList;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv;
	    int Len = UVCrv -> Length;
	    CagdPType Pt1, Pt2;
	    CagdRType
		**Points = UVCrv -> Points;

	    CagdCoerceToE2(Pt1, Points, 0, UVCrv -> PType);
	    CagdCoerceToE2(Pt2, Points, Len - 1, UVCrv -> PType);

	    switch (Axis) {
		case 0:
		    if (IRIT_APX_EQ_EPS(Pt1[0], UMin, TRIM_CRV_TOL) ||
			IRIT_APX_EQ_EPS(Pt1[0], UMax, TRIM_CRV_TOL)) {
			IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[0], UMin, TRIM_CRV_TOL) ||
			IRIT_APX_EQ_EPS(Pt2[0], UMax, TRIM_CRV_TOL)) {
			IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
		case 1:
		    if (IRIT_APX_EQ_EPS(Pt1[1], VMin, TRIM_CRV_TOL) ||
			IRIT_APX_EQ_EPS(Pt1[1], VMax, TRIM_CRV_TOL)) {
			IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[1], VMin, TRIM_CRV_TOL) ||
			IRIT_APX_EQ_EPS(Pt2[1], VMax, TRIM_CRV_TOL)) {
			IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
	    }
	    if (NumOfSeamPoints > 4)
	        return FALSE;
	}
    }

    if (NumOfSeamPoints == 4) {
        int i,
	    OtherAxis = 1 - Axis;
	CagdRType
	    Min = IRIT_INFNTY,
	    Max = -IRIT_INFNTY;
	CagdPtStruct CagdPt1, CagdPt2;
	CagdCrvStruct *Crv1, *Crv2;
	TrimCrvStruct *TrimCrv;

	for (i = 0; i < 4; i++) {
	    if (Min > SeamPoints[i][OtherAxis])
	        Min = SeamPoints[i][OtherAxis];
	    if (Max < SeamPoints[i][OtherAxis])
	        Max = SeamPoints[i][OtherAxis];
	}

	/* Build the two missing edges on the seam. */
	CagdPt1.Pt[2] = CagdPt2.Pt[2] = 0.0;
	switch (Axis) {
	    case 0:
		CagdPt1.Pt[0] = CagdPt2.Pt[0] = UMin;
		CagdPt1.Pt[1] = Min;
		CagdPt2.Pt[1] = Max;
		Crv1 = CagdMergePtPt(&CagdPt1, &CagdPt2);
		CagdPt1.Pt[0] = CagdPt2.Pt[0] = UMax;
		Crv2 = CagdMergePtPt(&CagdPt1, &CagdPt2);
		break;
	    case 1:
		CagdPt1.Pt[1] = CagdPt2.Pt[1] = VMin;
		CagdPt1.Pt[0] = Min;
		CagdPt2.Pt[0] = Max;
		Crv1 = CagdMergePtPt(&CagdPt1, &CagdPt2);
		CagdPt1.Pt[1] = CagdPt2.Pt[1] = VMax;
		Crv2 = CagdMergePtPt(&CagdPt1, &CagdPt2);
		break;
	    default:
		return FALSE;
	}

	TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCoerceCrvTo(Crv1,
							   CAGD_PT_E2_TYPE,
							   FALSE),
					   NULL));
	CagdCrvFree(Crv1);
	((TrimCrvStruct *) CagdListLast(TrimSrf -> TrimCrvList)) -> Pnext =
									TrimCrv;
	TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCoerceCrvTo(Crv2,
							   CAGD_PT_E2_TYPE,
							   FALSE),
					   NULL));
	CagdCrvFree(Crv2);
	((TrimCrvStruct *) CagdListLast(TrimSrf -> TrimCrvList)) -> Pnext =
									TrimCrv;
	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add a missing trimming curve along the prescribed boundary.              *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:    Trimmed surface to add a missing trimming edge along bndry.  *
*   Bndry:      The Boundary to add the missing edge along.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int UpdateOneSeamTrimCrv(TrimSrfStruct *TrimSrf,
				CagdSrfBndryType Bndry)
{
    int OtherAxis = -1,
	NumOfSeamPoints = 0;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct
	*Srf = TrimSrf -> Srf;
    TrimCrvStruct *TrimCrvs;
    CagdPType SeamPoints[4];

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax); 

    /* Search for all end points on that boundary. */
    for (TrimCrvs = TrimSrf -> TrimCrvList;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;

	for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSegs -> UVCrv;
	    int Len = UVCrv -> Length;
	    CagdPType Pt1, Pt2;
	    CagdRType
		**Points = UVCrv -> Points;

	    CagdCoerceToE2(Pt1, Points, 0, UVCrv -> PType);
	    CagdCoerceToE2(Pt2, Points, Len - 1, UVCrv -> PType);

	    switch (Bndry) {
		case CAGD_U_MIN_BNDRY:
		    OtherAxis = 1;
		    if (IRIT_APX_EQ_EPS(Pt1[0], UMin, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[0], UMin, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
		case CAGD_U_MAX_BNDRY:
		    OtherAxis = 1;
		    if (IRIT_APX_EQ_EPS(Pt1[0], UMax, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[0], UMax, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
		case CAGD_V_MIN_BNDRY:
		    OtherAxis = 0;
		    if (IRIT_APX_EQ_EPS(Pt1[1], VMin, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[1], VMin, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
		case CAGD_V_MAX_BNDRY:
		    OtherAxis = 0;
		    if (IRIT_APX_EQ_EPS(Pt1[1], VMax, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt1);
			NumOfSeamPoints++;
		    }
		    if (IRIT_APX_EQ_EPS(Pt2[1], VMax, TRIM_CRV_TOL)) {
		        IRIT_PT_COPY(SeamPoints[NumOfSeamPoints], Pt2);
			NumOfSeamPoints++;
		    }
		    break;
		default:
		    return FALSE;
	    }
	    if (NumOfSeamPoints > 2)
	        return FALSE;
	}
    }

    if (NumOfSeamPoints == 2) {
        int i;
	CagdRType
	    Min = IRIT_INFNTY,
	    Max = -IRIT_INFNTY;
	CagdPtStruct CagdPt1, CagdPt2;
	CagdCrvStruct *Crv;
	TrimCrvStruct *TrimCrv;

	for (i = 0; i < 2; i++) {
	    if (Min > SeamPoints[i][OtherAxis])
	        Min = SeamPoints[i][OtherAxis];
	    if (Max < SeamPoints[i][OtherAxis])
	        Max = SeamPoints[i][OtherAxis];
	}

	/* Build the two missing edges on the seam. */
	CagdPt1.Pt[2] = CagdPt2.Pt[2] = 0.0;

	switch (Bndry) {
	    case CAGD_U_MIN_BNDRY:
		CagdPt1.Pt[0] = CagdPt2.Pt[0] = UMin;
		CagdPt1.Pt[1] = Min;
		CagdPt2.Pt[1] = Max;
		break;
	    case CAGD_U_MAX_BNDRY:
		CagdPt1.Pt[0] = CagdPt2.Pt[0] = UMax;
		CagdPt1.Pt[1] = Min;
		CagdPt2.Pt[1] = Max;
		break;
	    case CAGD_V_MIN_BNDRY:
		CagdPt1.Pt[1] = CagdPt2.Pt[1] = VMin;
		CagdPt1.Pt[0] = Min;
		CagdPt2.Pt[0] = Max;
		break;
	    case CAGD_V_MAX_BNDRY:
		CagdPt1.Pt[1] = CagdPt2.Pt[1] = VMax;
		CagdPt1.Pt[0] = Min;
		CagdPt2.Pt[0] = Max;
		break;
	    default:
		return FALSE;
	}

	Crv = CagdMergePtPt(&CagdPt1, &CagdPt2);

	TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCoerceCrvTo(Crv,
							   CAGD_PT_E2_TYPE,
							   FALSE),
					   NULL));
	CagdCrvFree(Crv);
	((TrimCrvStruct *) CagdListLast(TrimSrf -> TrimCrvList)) -> Pnext =
									TrimCrv;
	return TRUE;
    }
    else
        return FALSE;
    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a trimming surface structure.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   A trimming surface to duplicate.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   A trimming surface structure.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfCopy, allocation                                                  M
*****************************************************************************/
TrimSrfStruct *TrimSrfCopy(const TrimSrfStruct *TrimSrf)
{
    TrimSrfStruct
	*NewTrimSrf = (TrimSrfStruct *)
	    IritMalloc(sizeof(TrimSrfStruct));

    NewTrimSrf -> TrimCrvList =
	TrimCrvCopyList(TrimSrf -> TrimCrvList);
    NewTrimSrf -> Srf = TrimSrf -> Srf ? CagdSrfCopy(TrimSrf -> Srf) : NULL;
    NewTrimSrf -> Pnext = NULL;
    NewTrimSrf -> Attr = NULL;

    return NewTrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of trimming surface structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfList:   To be copied.                                	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  A duplicated list of trimming surfaces.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfCopyList, copy                                                    M
*****************************************************************************/
TrimSrfStruct *TrimSrfCopyList(const TrimSrfStruct *TrimSrfList)
{
    TrimSrfStruct *TrimSrfTemp, *NewTrimSrfList;

    if (TrimSrfList == NULL)
	return NULL;
    TrimSrfTemp = NewTrimSrfList = TrimSrfCopy(TrimSrfList);
    TrimSrfList = TrimSrfList -> Pnext;
    while (TrimSrfList) {
	TrimSrfTemp -> Pnext = TrimSrfCopy(TrimSrfList);
	TrimSrfTemp = TrimSrfTemp -> Pnext;
	TrimSrfList = TrimSrfList -> Pnext;
    }
    return NewTrimSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a trimmed surface structure.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf: A trimmed surface to free.                   	      	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfFree, allocation                                                  M
*****************************************************************************/
void TrimSrfFree(TrimSrfStruct *TrimSrf)
{
    TrimCrvFreeList(TrimSrf -> TrimCrvList);
    if (TrimSrf -> Srf != NULL)
	CagdSrfFree(TrimSrf -> Srf);
    IP_ATTR_FREE_ATTRS(TrimSrf -> Attr);
    IritFree(TrimSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a list of trimmed surface structures.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfList: A list of trimmed surface to free.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfFreeList, allocation                     	                     M
*****************************************************************************/
void TrimSrfFreeList(TrimSrfStruct *TrimSrfList)
{
    TrimSrfStruct *TrimSrfTemp;

    while (TrimSrfList) {
	TrimSrfTemp = TrimSrfList -> Pnext;
	TrimSrfFree(TrimSrfList);
	TrimSrfList = TrimSrfTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Linearly transforms, in place, given trimmed surface as specified by       M
* Translate and Scale.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       Trimmed surface to transform.                             M
*   Translate:     Translation factor. Can be NULL for non.                  M
*   Scale:         Scaling factor.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfTransform		                                             M
*****************************************************************************/
void TrimSrfTransform(TrimSrfStruct *TrimSrf,
		      CagdRType *Translate,
		      CagdRType Scale)
{
    TrimCrvStruct
	*TrimCrvList = TrimSrf -> TrimCrvList;

    /* Transform the geometry. */
    CagdSrfTransform(TrimSrf -> Srf, Translate, Scale);

    /* And remove all Euclidean trimming curves. */
    for ( ; TrimCrvList != NULL; TrimCrvList = TrimCrvList -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegList = TrimCrvList -> TrimCrvSegList;

	for (;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    if (TrimCrvSegList -> EucCrv != NULL) {
		CagdCrvFree(TrimCrvSegList -> EucCrv);
		TrimCrvSegList -> EucCrv = NULL;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms, in place, the given trimmed surface as specified by a	     M
* homogeneous matrix Mat.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       Trimmed surface to transform.                             M
*   Mat:           Homogeneous transformation to apply to trimmed surface.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfMatTransform, Trimmed surface                                     M
*****************************************************************************/
void TrimSrfMatTransform(TrimSrfStruct *TrimSrf, CagdMType Mat)
{
    TrimCrvStruct
	*TrimCrvList = TrimSrf -> TrimCrvList;
    CagdSrfStruct *Srf;

    /* Transform the geometry. */
    Srf = CagdSrfMatTransform(TrimSrf -> Srf, Mat);
    CagdSrfFree(TrimSrf -> Srf);
    TrimSrf -> Srf = Srf;

    /* And remove all Euclidean trimming curves. */
    for ( ; TrimCrvList != NULL; TrimCrvList = TrimCrvList -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegList = TrimCrvList -> TrimCrvSegList;

	for (;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    if (TrimCrvSegList -> EucCrv != NULL) {
		CagdCrvFree(TrimCrvSegList -> EucCrv);
		TrimCrvSegList -> EucCrv = NULL;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two trimmed surfaces for similarity.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf1, TSrf2:     The two trimmed surfaces to compare.                   M
*   Eps:              Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if trimmed surfaces are the same, FALSE otehrwise.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfsSame					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfsSame                                                             M
*****************************************************************************/
CagdBType TrimSrfsSame(const TrimSrfStruct *TSrf1,
		       const TrimSrfStruct *TSrf2,
		       CagdRType Eps)
{
    TrimCrvStruct *TCrv1, *TCrv2;

    do {
        /* Compare the surfaces. */
        if ((TSrf1 -> Srf == NULL && TSrf2 -> Srf != NULL) ||
	    (TSrf1 -> Srf != NULL && TSrf2 -> Srf == NULL) ||
	    (TSrf1 -> Srf != NULL && TSrf2 -> Srf != NULL &&
	     !CagdSrfsSame(TSrf1 -> Srf, TSrf2 -> Srf, Eps)))
	    return FALSE;

	/* Compare the trimming curves. */
	for (TCrv1 = TSrf1 -> TrimCrvList, TCrv2 = TSrf2 -> TrimCrvList;
	     TCrv1 != NULL;
	     TCrv1 = TCrv1 -> Pnext, TCrv2 = TCrv2 -> Pnext) {
	    TrimCrvSegStruct *TSeg1, *TSeg2;

	    if (TCrv2 == NULL)
	        return FALSE;

	    for (TSeg1 = TCrv1 -> TrimCrvSegList,
		     TSeg2 = TCrv2 -> TrimCrvSegList;
		 TSeg1 != NULL;
		 TSeg1 = TSeg1 -> Pnext, TSeg2 = TSeg2 -> Pnext) {
	        if (TSeg2 == NULL)
		    return FALSE;

		if ((TSeg1 -> EucCrv == NULL && TSeg2 -> EucCrv != NULL) ||
		    (TSeg1 -> EucCrv != NULL && TSeg2 -> EucCrv == NULL) ||
		    (TSeg1 -> EucCrv != NULL && TSeg2 -> EucCrv != NULL &&
		     !CagdCrvsSame(TSeg1 -> EucCrv, TSeg2 -> EucCrv, Eps)))
		    return FALSE;

		if ((TSeg1 -> UVCrv == NULL && TSeg2 -> UVCrv != NULL) ||
		    (TSeg1 -> UVCrv != NULL && TSeg2 -> UVCrv == NULL) ||
		    (TSeg1 -> UVCrv != NULL && TSeg2 -> UVCrv != NULL &&
		     !CagdCrvsSame(TSeg1 -> UVCrv, TSeg2 -> UVCrv, Eps)))
		    return FALSE;
	    }
	}

	TSrf1 = TSrf1 -> Pnext;
	TSrf2 = TSrf2 -> Pnext;
    }
    while (TSrf1 != NULL && TSrf2 != NULL);

    return TSrf1 == NULL && TSrf2 == NULL;
}
