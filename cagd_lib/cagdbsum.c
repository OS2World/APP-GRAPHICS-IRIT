/******************************************************************************
* CagdBSum.c - Boolean sum surface constructor out of given four curves.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a Boolean sum surface using the four provided boundary curves.  M
*   Curve's end points must meet at the four surface corners if surface      M
* boundary are to be identical to the four given curves.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrvLeft:   Left boundary curve of Boolean sum surface to be created.    M
*   CCrvRight:  Right boundary curve of Boolean sum surface to be created.   M
*   CCrvTop:    Top boundary curve of Boolean sum surface to be created.     M
*   CCrvBottom: Bottom boundary curve of Boolean sum surface to be created.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Boolean sum surface constructed using given four     M
*                     curves.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBoolSumSrf, Boolean sum, surface constructors                        M
*****************************************************************************/
CagdSrfStruct *CagdBoolSumSrf(const CagdCrvStruct *CCrvLeft,
			      const CagdCrvStruct *CCrvRight,
			      const CagdCrvStruct *CCrvTop,
			      const CagdCrvStruct *CCrvBottom)
{
    int i, j;
    CagdCrvStruct *Crv1, *Crv2, *CrvLeft, *CrvRight, *CrvTop, *CrvBottom;
    CagdSrfStruct *Ruled1, *Ruled2, *Ruled3, *Srf;
    CagdPtStruct Pt1, Pt2;
    CagdRType **SrfPoints, **Ruled1Pts, **Ruled2Pts, **Ruled3Pts;

    if (CAGD_IS_PERIODIC_CRV(CCrvLeft) ||
	CAGD_IS_PERIODIC_CRV(CCrvRight) ||
	CAGD_IS_PERIODIC_CRV(CCrvTop) ||
	CAGD_IS_PERIODIC_CRV(CCrvBottom)) {
	CAGD_FATAL_ERROR(CAGD_ERR_PERIODIC_NO_SUPPORT);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_CRV(CCrvLeft) && !BspCrvHasOpenEC(CCrvLeft))
        CrvLeft = CagdCnvrtFloat2OpenCrv(CCrvLeft);
    else
        CrvLeft = CagdCrvCopy(CCrvLeft);
    if (CAGD_IS_BSPLINE_CRV(CCrvRight) && !BspCrvHasOpenEC(CCrvRight))
        CrvRight = CagdCnvrtFloat2OpenCrv(CCrvRight);
    else
        CrvRight = CagdCrvCopy(CCrvRight);
    if (CAGD_IS_BSPLINE_CRV(CCrvTop) && !BspCrvHasOpenEC(CCrvTop))
        CrvTop = CagdCnvrtFloat2OpenCrv(CCrvTop);
    else
        CrvTop = CagdCrvCopy(CCrvTop);
    if (CAGD_IS_BSPLINE_CRV(CCrvBottom) && !BspCrvHasOpenEC(CCrvBottom))
        CrvBottom = CagdCnvrtFloat2OpenCrv(CCrvBottom);
    else
        CrvBottom = CagdCrvCopy(CCrvBottom);

    if (CAGD_IS_BSPLINE_CRV(CrvLeft))
        BspKnotAffineTrans2(CrvLeft -> KnotVector,
			    CrvLeft -> Order + CrvLeft -> Length, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(CrvRight))
        BspKnotAffineTrans2(CrvRight -> KnotVector,
			    CrvRight -> Order + CrvRight -> Length, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(CrvTop))
        BspKnotAffineTrans2(CrvTop -> KnotVector,
			    CrvTop -> Order + CrvTop -> Length, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(CrvBottom))
        BspKnotAffineTrans2(CrvBottom -> KnotVector,
			    CrvBottom -> Order + CrvBottom -> Length, 0.0, 1.0);

    /* The Left-Right and Top-Bottom curves should share same point/curve    */
    /* type as well as same order and knot vector (if Bspline representation)*/
    CagdMakeCrvsCompatible(&CrvLeft, &CrvRight, TRUE, TRUE);
    CagdMakeCrvsCompatible(&CrvTop, &CrvBottom, TRUE, TRUE);

    /* The Left-Right and Top-Bottom pairs must share same point/curve type. */
    CagdMakeCrvsCompatible(&CrvLeft, &CrvTop, FALSE, FALSE);
    CagdMakeCrvsCompatible(&CrvLeft, &CrvBottom, FALSE, FALSE);
    CagdMakeCrvsCompatible(&CrvRight, &CrvTop, FALSE, FALSE);
    CagdMakeCrvsCompatible(&CrvRight, &CrvBottom, FALSE, FALSE);

    /* Now that the curves are in the right representation, form surface(s). */
    /* The two ruled surface between the respective curves, in right orders: */
    Ruled1 = CagdRuledSrf(CrvLeft, CrvRight, 2, 2);
    Ruled2 = CagdRuledSrf(CrvTop, CrvBottom, 2, 2);
    Srf = CagdSrfReverse2(Ruled2);
    CagdSrfFree(Ruled2);
    Ruled2 = Srf;
    CagdMakeSrfsCompatible(&Ruled1, &Ruled2, TRUE, TRUE, TRUE, TRUE);

    /* The ruled surface between the four corner points in the right orders. */
    CagdCoerceToE3(Pt1.Pt, CrvLeft -> Points, 0, CrvLeft -> PType);
    CagdCoerceToE3(Pt2.Pt, CrvLeft -> Points, CrvLeft -> Length - 1,
							CrvLeft -> PType);
    Crv1 = CagdMergePtPt(&Pt1, &Pt2);

    CagdCoerceToE3(Pt1.Pt, CrvRight -> Points, 0, CrvRight -> PType);
    CagdCoerceToE3(Pt2.Pt, CrvRight -> Points, CrvRight -> Length - 1,
							CrvRight -> PType);
    Crv2 = CagdMergePtPt(&Pt1, &Pt2);

    /* Should not change CrvLeft/Right only Crv1/2: */
    Ruled3 = CagdRuledSrf(Crv1, Crv2, 2, 2);
    if (CAGD_IS_BSPLINE_SRF(Ruled3)) {
        BspKnotAffineTrans2(Ruled3 -> UKnotVector,
			    Ruled3 -> UOrder + Ruled3 -> ULength, 0.0, 1.0);
	BspKnotAffineTrans2(Ruled3 -> VKnotVector,
			    Ruled3 -> VOrder + Ruled3 -> VLength, 0.0, 1.0);
    }
    CagdMakeSrfsCompatible(&Ruled1, &Ruled3, TRUE, TRUE, TRUE, TRUE);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    /* The boolean sum is equal to Ruled1 + Ruled2 - Ruled3. This boolean    */
    /* sum as computed is not exactly as defined in the literature for non   */
    /* uniform Bsplines since the ruled surfaces are computed with uniform   */
    /* distribution along the other axis even if it is non uniform.	     */
    if (CrvRight -> GType == CAGD_CBSPLINE_TYPE) {
	Srf = BspSrfNew(Ruled1 -> ULength, Ruled1 -> VLength,
			Ruled1 -> UOrder, Ruled1 -> VOrder, Ruled1 -> PType);
	BspKnotCopy(Srf -> UKnotVector, Ruled1 -> UKnotVector,
		    Ruled1 -> ULength + Ruled1 -> UOrder);
	BspKnotCopy(Srf -> VKnotVector, Ruled1 -> VKnotVector,
		    Ruled1 -> VLength + Ruled1 -> VOrder);
    }
    else if (CrvRight -> GType == CAGD_CBEZIER_TYPE)
	Srf = BzrSrfNew(Ruled1 -> ULength, Ruled1 -> VLength, Ruled1 -> PType);
    else
	CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
    SrfPoints = Srf -> Points;

    Ruled1Pts = Ruled1 -> Points;
    Ruled2Pts = Ruled2 -> Points;
    Ruled3Pts = Ruled3 -> Points;

    for (i = !CAGD_IS_RATIONAL_SRF(Srf);
	 i <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	 i++) {
	CagdRType
	    *Ruled1PtsPtr = Ruled1Pts[i],
	    *Ruled2PtsPtr = Ruled2Pts[i],
	    *Ruled3PtsPtr = Ruled3Pts[i],
	    *SrfPointsPtr = SrfPoints[i];

	 for (j = Srf -> ULength * Srf -> VLength; j > 0; j--)
	     *SrfPointsPtr++ =
		 *Ruled1PtsPtr++ + *Ruled2PtsPtr++ - *Ruled3PtsPtr++;
    }

    CagdSrfFree(Ruled1);
    CagdSrfFree(Ruled2);
    CagdSrfFree(Ruled3);

    CagdCrvFree(CrvTop);
    CagdCrvFree(CrvRight);
    CagdCrvFree(CrvBottom);
    CagdCrvFree(CrvLeft);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_BOOL_SUM);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a Boolean sum surface using the single boundary curve.	     M
*   The curve is subdivided into four, equally spaced in parameter space,    M
* sub-regions which are used as the four curves to the Boolean sum           M
* constructor. See CagdBoolSumSrf.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   BndryCrv:   To be subdivided into four curves for a Boolean sum          M
*               construction.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Boolean sum surface constructed using given curve    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdOneBoolSumSrf, Boolean sum, surface constructors                     M
*****************************************************************************/
CagdSrfStruct *CagdOneBoolSumSrf(const CagdCrvStruct *BndryCrv)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *Crvs, *TCrv, *CrvLeft, *CrvRight, *CrvTop, *CrvBottom;
    CagdSrfStruct *BoolSumSrf;

    CagdCrvDomain(BndryCrv, &TMin, &TMax);

    Crvs = CagdCrvSubdivAtParam(BndryCrv, TMin * 0.5 + TMax * 0.5);
    CrvLeft = CagdCrvSubdivAtParam(Crvs, TMin * 0.75 + TMax * 0.25);
    CrvRight = CagdCrvSubdivAtParam(Crvs -> Pnext, TMin * 0.25 + TMax * 0.75);

    CagdCrvFreeList(Crvs);

    CrvBottom = CrvLeft -> Pnext;
    CrvLeft -> Pnext = NULL;
    CrvTop = CrvRight -> Pnext;
    CrvRight -> Pnext = NULL;

    /* Reverse CrvBottom and CrvLeft: */
    TCrv = CagdCrvReverse(CrvTop);
    CagdCrvFree(CrvTop);
    CrvTop = TCrv;

    TCrv = CagdCrvReverse(CrvRight);
    CagdCrvFree(CrvRight);
    CrvRight = TCrv;

    BoolSumSrf = CagdBoolSumSrf(CrvLeft, CrvRight, CrvTop, CrvBottom);

    CagdCrvFree(CrvTop);
    CagdCrvFree(CrvRight);
    CagdCrvFree(CrvBottom);
    CagdCrvFree(CrvLeft);

    CAGD_SET_GEOM_TYPE(BoolSumSrf, CAGD_GEOM_BOOL_SUM);

    return BoolSumSrf;
}
