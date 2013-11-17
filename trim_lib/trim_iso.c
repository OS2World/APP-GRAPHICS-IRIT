/******************************************************************************
* Trim_Iso.c - Computes iso parametric curves of trimmed surfaces.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 94.					      *
******************************************************************************/

#include "trim_loc.h"
#include "symb_lib.h"

#define MIN_SIZE_CRV		1e-5
#define TRIM_SAME_PT_EPS	1e-4

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugTrimDebugInters, FALSE);
#endif /* DEBUG */

IRIT_GLOBAL_DATA CagdRType
    _TrimUVCrvApproxTolSamples = 128;
IRIT_GLOBAL_DATA SymbCrvApproxMethodType
    _TrimUVCrvApproxMethod = SYMB_CRV_APPROX_UNIFORM;

static void InsertIntersections(TrimIsoInterStruct **Inters,
				CagdRType *IsoParams,
				int Index1,
				int Index2,
				CagdRType Val1,
				CagdRType Val2,
				CagdRType OVal1,
				CagdRType OVal2);
static int FindLocationInIsoParams(CagdRType Val,
				   CagdRType *IsoParams,
				   int NumOfIsocurves);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single trimmed surface to NumOfIsolines polylines   M
* in each parametric direction with TolSamples samples/tolerance in each     M
* isoparametric curve.							     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. Attempt is made to extract isolines along C1           M
* discontinuities first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:        To extract isoparametric curves from.                    M
*   NumOfIsocurves: In each (U or V) direction.                              M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *: List of polylines representing a piecewise linear  M
*                         approximation of the extracted isoparamteric       M
*                         curves or NULL is case of an error.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrv2Polyline					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrf2Polylines, isoparametric curves                                  M
*****************************************************************************/
CagdPolylineStruct *TrimSrf2Polylines(TrimSrfStruct *TrimSrf,
				      int NumOfIsocurves[2],
				      CagdRType TolSamples,
				      SymbCrvApproxMethodType Method)
{
    CagdCrvStruct *Crv,
        *Crvs = TrimSrf2Curves(TrimSrf, NumOfIsocurves);
    CagdPolylineStruct *Poly,
	*Polys = NULL;

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	Poly = TrimCrv2Polyline(Crv, TolSamples, Method, TRUE);
	Poly -> Pnext = Polys;
	Polys = Poly;
    }

    CagdCrvFreeList(Crvs);
    return Polys;	
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a trimmed surface NumOfIsoline isocurve list     M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
* As the isoparametric curves are trimmed according to the trimming curves   M
* the resulting number of curves is arbitrary.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:         To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In each (U or V) direction.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrf2Curves, curves, isoparametric curves                             M
*****************************************************************************/
CagdCrvStruct *TrimSrf2Curves(TrimSrfStruct *TrimSrf,
			      int NumOfIsocurves[2])
{
    int i, NumC1Disconts,
	ULength = TrimSrf -> Srf -> ULength,
	VLength = TrimSrf -> Srf -> VLength,
	UOrder = TrimSrf -> Srf -> UOrder,
	VOrder = TrimSrf -> Srf -> VOrder;
    CagdRType UMin, UMax, VMin, VMax, *C1Disconts,
	*UIsoParams, *VIsoParams;
    TrimIsoInterStruct **UInters, **VInters;
    CagdCrvStruct *Crv,
	*CrvList = NULL;
    TrimCrvStruct
	*TCrvs = TrimHealTrimmingCurves(TrimSrf -> TrimCrvList);

    if (TCrvs != NULL) {
	TrimCrvFreeList(TrimSrf -> TrimCrvList);
	TrimSrf -> TrimCrvList = TCrvs;
    }

    /* Make sure requested format is something reasonable. */
    if (NumOfIsocurves[0] < 2)
	NumOfIsocurves[0] = 2;
    if (NumOfIsocurves[1] <= 0)
	NumOfIsocurves[1] = NumOfIsocurves[0];
    else if (NumOfIsocurves[1] < 2)
	NumOfIsocurves[1] = 2;

    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);

    if (CAGD_IS_BSPLINE_SRF(TrimSrf -> Srf)) {
	/* Compute discontinuities along u axis and use that to determine    */
	/* where to extract isolines along u.				     */
	/* Note C1Disconts is freed by BspKnotParamValues.		     */
	C1Disconts = BspKnotAllC1Discont(TrimSrf -> Srf -> UKnotVector, UOrder,
					 ULength, &NumC1Disconts);
	UIsoParams = BspKnotParamValues(UMin, UMax, NumOfIsocurves[0],
					C1Disconts, NumC1Disconts);


	/* Compute discontinuities along v axis and use that to determine    */
	/* where to extract isolines along v.				     */
	/* Note C1Disconts is freed by BspKnotParamValues.		     */
	C1Disconts = BspKnotAllC1Discont(TrimSrf -> Srf -> VKnotVector, VOrder,
					 VLength, &NumC1Disconts);
	VIsoParams = BspKnotParamValues(VMin, VMax, NumOfIsocurves[1],
					C1Disconts, NumC1Disconts);
    }
    else if (CAGD_IS_BEZIER_SRF(TrimSrf -> Srf)) {
	UIsoParams = (CagdRType *) IritMalloc(sizeof(IrtRType) *
							NumOfIsocurves[0]);
	for (i = 0; i < NumOfIsocurves[0]; i++)
	    UIsoParams[i] = ((CagdRType) i) / (NumOfIsocurves[0] - 1);

	VIsoParams = (CagdRType *) IritMalloc(sizeof(IrtRType) *
							NumOfIsocurves[1]);
	for (i = 0; i < NumOfIsocurves[1]; i++)
	    VIsoParams[i] = ((CagdRType) i) / (NumOfIsocurves[1] - 1);
    }
    else {
	TRIM_FATAL_ERROR(TRIM_ERR_BZR_BSP_EXPECT);
	return NULL;
    }

    UInters = TrimIntersectTrimCrvIsoVals(TrimSrf, CAGD_CONST_U_DIR,
					  UIsoParams, NumOfIsocurves[0], TRUE);
    VInters = TrimIntersectTrimCrvIsoVals(TrimSrf, CAGD_CONST_V_DIR,
					  VIsoParams, NumOfIsocurves[1], TRUE);

    for (i = 0; i < NumOfIsocurves[0]; i++) {
	CagdCrvStruct *Crvs, *CrvsLast;

	Crv = CagdCrvFromSrf(TrimSrf -> Srf, UIsoParams[i],
			     CAGD_CONST_U_DIR);
	/* Both Crv and UInters[i] are freed by TrimCrvTrimParamList: */
	if ((Crvs = TrimCrvTrimParamList(Crv, UInters[i])) != NULL) {
	    CrvsLast = (CagdCrvStruct *) CagdListLast(Crvs);
	    CrvsLast -> Pnext = CrvList;
	    CrvList = Crvs;
	}
    }

    for (i = 0; i < NumOfIsocurves[1]; i++) {
	CagdCrvStruct *Crvs, *CrvsLast;

	Crv = CagdCrvFromSrf(TrimSrf -> Srf, VIsoParams[i],
			     CAGD_CONST_V_DIR);
	/* Both Crv and VInters[i] are freed by TrimCrvTrimParamList: */
	if ((Crvs = TrimCrvTrimParamList(Crv, VInters[i])) != NULL) {
	    CrvsLast = (CagdCrvStruct *) CagdListLast(Crvs);
	    CrvsLast -> Pnext = CrvList;
	    CrvList = Crvs;
	}
    }

    IritFree(UInters);
    IritFree(VInters);
    IritFree(UIsoParams);
    IritFree(VIsoParams);

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trim Crv at the domains prescribed in the intersection list InterList    M
* Both Crv and InterList are FREED in this routine.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         To trim out according to the prescribed intersections.      M
*   InterList:   List of intersections, as parameters into Crv.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of trimmed curves. May be empty (NULL).           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvTrimParamList, curves, isoparametric curves                       M
*****************************************************************************/
CagdCrvStruct *TrimCrvTrimParamList(CagdCrvStruct *Crv,
				    TrimIsoInterStruct *InterList)
{
    CagdCrvStruct
	*CrvList = NULL;

    while (InterList != NULL) {
	if (InterList -> Pnext == NULL) {
	    TRIM_FATAL_ERROR(TRIM_ERR_ODD_NUM_OF_INTER);
	    return NULL;
	}
	else {
	    TrimIsoInterStruct
		*InterNext = InterList -> Pnext -> Pnext;
	    CagdRType TMin, TMax,
	        T1 = InterList -> Param,
		T2 = InterList -> Pnext -> Param;

	    /* Especially in rational trimming curves, we might get round-   */
	    /* offs that are outside the domain.			     */
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (T1 < TMin)
		T1 = TMin;
	    if (T2 > TMax)
		T2 = TMax;

	    if (T2 - T1 > MIN_SIZE_CRV) {
		CagdCrvStruct
		    *TCrv = CagdCrvRegionFromCrv(Crv, T1, T2);

		IRIT_LIST_PUSH(TCrv, CrvList);
	    }

	    IritFree(InterList -> Pnext);
	    IritFree(InterList);
	    InterList = InterNext;
	}
    }

    CagdCrvFree(Crv);

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersections of the trimming curves of TrimSrf with the    M
* ordered isoparametric values prescribed by IsoParams, in axis Axis.        M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:        Trimmed surface to consider.                             M
*   Dir:            Either U or V.			                     M
*   OrigIsoParams:  Vectors of isoparametric values in direction Dir.        M
*   NumOfIsocurves: Size of vector IsoParams.                                M
*   Perturb:	    TRUE to epsilon-perturb the iso-param values.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimIsoInterStruct **:   A vector of size NumOfIsocurves, each contains  M
*		  a list of intersection parameter values.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrf2Polylines, TrimCrvAgainstTrimCrvs		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimIntersectTrimCrvIsoVals, isoparametric curves                        M
*****************************************************************************/
TrimIsoInterStruct **TrimIntersectTrimCrvIsoVals(const TrimSrfStruct *TrimSrf,
						 int Dir,
						 CagdRType *OrigIsoParams,
						 int NumOfIsocurves,
						 CagdBType Perturb)
{
    int i, Axis, OAxis,
	NumOfIsocurves1 = IRIT_MAX(NumOfIsocurves - 1, 1);
    TrimIsoInterStruct
	**Inters = (TrimIsoInterStruct **)
		     IritMalloc(sizeof(TrimIsoInterStruct *) * NumOfIsocurves);
    TrimCrvStruct
	*TrimCrv = TrimSrf -> TrimCrvList;
    CagdRType
	EpsPerturb = Perturb ? TRIM_ISO_PARAM_PERTURB : 0.0,
	*IsoParams = (CagdRType *) IritMalloc(sizeof(IrtRType) *
							       NumOfIsocurves);

    for (i = 0; i < NumOfIsocurves; i++) {
	Inters[i] = NULL;

	IsoParams[i] = OrigIsoParams[i] +
	    EpsPerturb * (i / ((CagdRType) NumOfIsocurves1) - 0.51);
#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrimDebugInters)
	    IRIT_INFO_MSG_PRINTF("Iso %d = %.15f\n", i, IsoParams[i]);
#	endif /* DEBUG */
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Axis = 1;
	    OAxis = 2;
	    break;
	case CAGD_CONST_V_DIR:
	    Axis = 2;
	    OAxis = 1;
	    break;
	default:
	    TRIM_FATAL_ERROR(TRIM_ERR_DIR_NOT_CONST_UV);
	    return NULL;
    }

    for ( ; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int UVSize, Index1, Index2;
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;

	    if (UVCrv -> Order > 2) {
		CagdPolylineStruct
		    *UVPoly = TrimCrv2Polyline(UVCrv,
					       _TrimUVCrvApproxTolSamples,
					       _TrimUVCrvApproxMethod, TRUE);

		UVSize = UVPoly -> Length;

		if (TrimSrf -> Srf != NULL) {
		    CagdRType UMin, UMax, VMin, VMax;

		    CagdSrfDomain(TrimSrf -> Srf, &UMin, &UMax, &VMin, &VMax);
		    for (i = 0; i < UVSize; i++) {
		        UVPoly -> Polyline[i].Pt[0] =
			    IRIT_BOUND(UVPoly -> Polyline[i].Pt[0], UMin, UMax);
			UVPoly -> Polyline[i].Pt[1] =
			    IRIT_BOUND(UVPoly -> Polyline[i].Pt[1], VMin, VMax);
		    }
		}

		/* It is absolutely crucial that the first and last points */
		/* are exactly the same.  Make sure it is so.		   */
		if (IRIT_PT_APX_EQ_EPS(UVPoly -> Polyline[0].Pt,
				  UVPoly -> Polyline[UVSize - 1].Pt,
				  TRIM_SAME_PT_EPS)) {
		    IRIT_PT_COPY(UVPoly -> Polyline[0].Pt,
			    UVPoly -> Polyline[UVSize - 1].Pt);
		}
		Axis--;		      /* XYZ is counted from zero, not one. */
		OAxis--;

		Index1 = FindLocationInIsoParams(UVPoly ->
						     Polyline[0].Pt[Axis],
						 IsoParams, NumOfIsocurves);
		for (i = 1; i < UVSize; i++) {
		    Index2 = FindLocationInIsoParams(UVPoly ->
						      Polyline[i].Pt[Axis],
						     IsoParams,
						     NumOfIsocurves);
		    if (Index1 != Index2) {
			InsertIntersections(Inters, IsoParams, Index1, Index2,
					    UVPoly -> Polyline[i-1].Pt[Axis],
					    UVPoly -> Polyline[i].Pt[Axis],
					    UVPoly -> Polyline[i-1].Pt[OAxis],
					    UVPoly -> Polyline[i].Pt[OAxis]);
		    }
		    Index1 = Index2;
		}

		Axis++;				      /* Recover the axes. */
		OAxis++;

		CagdPolylineFree(UVPoly);
	    }
	    else {
		CagdRType
		    **Points = UVCrv -> Points;

		UVSize = UVCrv -> Length;

		Index1 = FindLocationInIsoParams(Points[Axis][0],
						 IsoParams, NumOfIsocurves);
		for (i = 1; i < UVSize; i++) {
		    Index2 = FindLocationInIsoParams(Points[Axis][i],
						     IsoParams,
						     NumOfIsocurves);
		    if (Index1 != Index2) {
			InsertIntersections(Inters, IsoParams, Index1, Index2,
					    Points[Axis][i-1],
					    Points[Axis][i],
					    Points[OAxis][i-1],
					    Points[OAxis][i]);
		    }
		    Index1 = Index2;
		}
	    }
	}
    }

    IritFree(IsoParams);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrimDebugInters) {
        for (i = 0; i < NumOfIsocurves; i++) {
	    TrimIsoInterStruct
	        *Inter = Inters[i];

	    IRIT_INFO_MSG_PRINTF("Iso %d, Inters = ", i);
	    while (Inter) {
	        IRIT_INFO_MSG_PRINTF("%17.15f ", Inter -> Param);
		Inter = Inter -> Pnext;
	    }
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    return Inters;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersections of given UV curves with the ordered           M
* isoparametric values prescribed by IsoParams, in axis Axis. 		     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrvs:         UV curves to intersect. Must be piecwise linear.         M
*   Dir:            Either U or V.			                     M
*   IsoParams:      Vector of isoparametric values in direction Dir.         M
*   NumOfIsocurves: Size of vector IsoParams.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimIsoInterStruct **:   A vector of size NumOfIsocurves, each slot      M
*		 contains a list of intersection parameter values.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIntersectTrimCrvIsoVals				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimIntersectCrvsIsoVals, isoparametric curves                           M
*****************************************************************************/
TrimIsoInterStruct **TrimIntersectCrvsIsoVals(const CagdCrvStruct *UVCrvs,
					      int Dir,
					      CagdRType *IsoParams,
					      int NumOfIsocurves)
{
    int i, Axis, OAxis, Index1, Index2;
    const CagdCrvStruct *UVCrv;
    TrimIsoInterStruct
	**Inters = (TrimIsoInterStruct **)
		     IritMalloc(sizeof(TrimIsoInterStruct *) * NumOfIsocurves);

    for (i = 0; i < NumOfIsocurves; i++)
	Inters[i] = NULL;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Axis = 1;
	    OAxis = 2;
	    break;
	case CAGD_CONST_V_DIR:
	    Axis = 2;
	    OAxis = 1;
	    break;
	default:
	    TRIM_FATAL_ERROR(TRIM_ERR_DIR_NOT_CONST_UV);
	    return NULL;
    }

    for (UVCrv = UVCrvs; UVCrv != NULL; UVCrv = UVCrv -> Pnext) {
        CagdRType
	    * const *Points = UVCrv -> Points;
	int UVSize = UVCrv -> Length;

	Index1 = FindLocationInIsoParams(Points[Axis][0],
					 IsoParams, NumOfIsocurves);
	for (i = 1; i < UVSize; i++) {
	    Index2 = FindLocationInIsoParams(Points[Axis][i],
					     IsoParams,
					     NumOfIsocurves);
	    if (Index1 != Index2) {
	        InsertIntersections(Inters, IsoParams, Index1, Index2,
				    Points[Axis][i-1],
				    Points[Axis][i],
				    Points[OAxis][i-1],
				    Points[OAxis][i]);
	    }
	    Index1 = Index2;
	}
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrimDebugInters) {
        for (i = 0; i < NumOfIsocurves; i++) {
	    TrimIsoInterStruct
	        *Inter = Inters[i];

	    IRIT_INFO_MSG_PRINTF("Iso %d, Inters = ", i);
	    while (Inter) {
	        IRIT_INFO_MSG_PRINTF("%17.15f ", Inter -> Param);
		Inter = Inter -> Pnext;
	    }
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    return Inters;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trim a given curve in UV space of a trimmed surface to the valid domain  M
* only.  Returned is a list of segments of UV curve that is inside TrimSrf.  M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrv:      A curve in the parametric space to trim.  Freed.             M
*   TrimSrf:    A trimmed surface.                                           M
*   Eps:	Tolerance of approximation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A list of trimmed segments of UVCrv inside TrimSrf.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIntersectTrimCrvIsoVals                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvAgainstTrimCrvs                                                   M
*****************************************************************************/
CagdCrvStruct *TrimCrvAgainstTrimCrvs(CagdCrvStruct *UVCrv,
				      const TrimSrfStruct *TrimSrf,
				      CagdRType Eps)
{
    CagdCrvStruct *UVCrvs,
	*UVValidCrvs = NULL;
    const TrimCrvStruct
	*TrimCrv = TrimSrf -> TrimCrvList;

    /* Split the given curve at all intersections with trimming curves. */
    for ( ; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
        TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    CagdCrvStruct
		*UVCrvsNew = NULL,
		*UVTrim = TrimCrvSeg -> UVCrv;
	    CagdPtStruct *Inters, *Inter;

	    UVCrvs = UVCrv;
	    while (UVCrvs) {
	        IRIT_LIST_POP(UVCrv, UVCrvs);

		if ((Inters = CagdCrvCrvInter(UVCrv, UVTrim, Eps)) != NULL) {
		    while (Inters) {
		        CagdRType TMin, TMax;
		        CagdCrvStruct *UVCrv1, *UVCrv2;

		        IRIT_LIST_POP(Inter, Inters);

			CagdCrvDomain(UVCrv, &TMin, &TMax);
			if (Inter -> Pt[0] > TMin &&
			    Inter -> Pt[0] < TMax &&
			    !IRIT_APX_EQ_EPS(Inter -> Pt[0], TMin, Eps) &&
			    !IRIT_APX_EQ_EPS(Inter -> Pt[0], TMax, Eps)) {
			    UVCrv1 = CagdCrvSubdivAtParam(UVCrv,
							  Inter -> Pt[0]);
			    UVCrv2 = UVCrv1 -> Pnext;
			    UVCrv1 -> Pnext = NULL;

			    IRIT_LIST_PUSH(UVCrv1, UVCrvsNew);
			    CagdCrvFree(UVCrv);
			    UVCrv = UVCrv2;			      
			}
			IritFree(Inter);
		    }
		    IRIT_LIST_PUSH(UVCrv, UVCrvsNew);
	        }
		else
		    IRIT_LIST_PUSH(UVCrv, UVCrvsNew);
	    }
	}
    }

    /* Check each curve segment for inclusion in the valid surface domain. */
    UVCrvs = UVCrv;
    while (UVCrvs) {
	CagdRType *R, TMin, TMax;
	CagdUVType UV;

        IRIT_LIST_POP(UVCrv, UVCrvs);

	CagdCrvDomain(UVCrv, &TMin, &TMax);
	R = CagdCrvEval(UVCrv, (TMin + TMax) * 0.5);
	CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);
	
	if (TrimIsPointInsideTrimSrf(TrimSrf, UV)) {
	    IRIT_LIST_PUSH(UVCrv, UVValidCrvs);
	}
	else
	    CagdCrvFree(UVCrv);
    }

    return UVValidCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the intersection vector of lists Inters with the linear segment  *
* of the trimming curves from (Val1, OVal1) to (Val2, OVal2) affecting       *
* indices from Index1 to Index2 in IsoParams.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Inters:     Vector of intersections with isoparameter values.            *
*   IsoParams:  Vectors isoparameter values.                                 *
*   Index1:	Index of first coordinate of intersection into IsoParams.    *
*   Index2:	Index of second coordinate of intersection into IsoParams.   *
*   Val1:       Parameter value of first intersection.                       *
*   Val2:       Parameter value of second intersection.                      *
*   OVal1:      Other parameter value of first intersection.                 *
*   OVal2:      Other parameter value of second intersection.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertIntersections(TrimIsoInterStruct **Inters,
				CagdRType *IsoParams,
				int Index1,
				int Index2,
				CagdRType Val1,
				CagdRType Val2,
				CagdRType OVal1,
				CagdRType OVal2)
{
    int i;

    if (Index1 > Index2)
	IRIT_SWAP(int, Index1, Index2);
    if (Index1 < 0)
        Index1 = 0;

    for (i = Index1; i <= Index2; i++) {
	TrimIsoInterStruct *Inter;
	CagdRType
	    t = (Val1 - IsoParams[i]) / (Val1 - Val2);

	if (t < 0.0 || t > 1.0)
	    continue;

	Inter = (TrimIsoInterStruct *) IritMalloc(sizeof(TrimIsoInterStruct));
	Inter -> Param = OVal1 * (1.0 - t) + OVal2 * t;
	Inter -> Pnext = NULL;

	/* Insert into the vector of intersection lists. */
	if (Inters[i] == NULL)
	    Inters[i] = Inter;
	else if (Inters[i] -> Param > Inter -> Param) {
	    Inter -> Pnext = Inters[i];
	    Inters[i] = Inter;
	}
	else {
	    TrimIsoInterStruct *InterTmp;

	    for (InterTmp = Inters[i];
		 InterTmp -> Pnext != NULL;
		 InterTmp = InterTmp -> Pnext) {
		if (InterTmp -> Pnext -> Param > Inter -> Param)
		    break;
	    }

	    if (InterTmp -> Pnext == NULL)
		InterTmp -> Pnext = Inter;
	    else {
		Inter -> Pnext = InterTmp -> Pnext;
		InterTmp -> Pnext = Inter;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the index in IsoParams, that is the maximal value that is less     *
* than or equal to Val.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Val:          Real value to search in vector.                            *
*   IsoParams:    Vectors of isoparametric values in direction Dir.          *
*   NumOfIsocurves:   Size of vector IsoParams.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Found index.                                               *
*****************************************************************************/
static int FindLocationInIsoParams(CagdRType Val,
				   CagdRType *IsoParams,
				   int NumOfIsocurves)
{
    int MidIndex,
	MinIndex = 0,
	MaxIndex = NumOfIsocurves - 1;

    while (MaxIndex - MinIndex >= 2) {
	MidIndex = ((MinIndex + MaxIndex) >> 1);

	if (IsoParams[MidIndex] <= Val)
	    MinIndex = MidIndex;
	else
	    MaxIndex = MidIndex;
    }

    if (MinIndex + 1 <= MaxIndex && IsoParams[MinIndex + 1] <= Val)
	return MinIndex + 1;
    else if (IsoParams[MinIndex] <= Val)
	return MinIndex;
    else
        return -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the tolerances to use when approximating higher order trimming      M
* curves using piecewise linear approximation, for intersection computation. M
*                                                                            *
* PARAMETERS:                                                                M
*   UVTolSamples:  Piecewise linear approximation of high order trimming     M
*		   curves - number of samples per curve or tolerance.        M
*   UVMethod:      Method of sampling.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbCrvApproxMethodType:  Old method of curve sampling.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimGetTrimCrvLinearApprox, SymbCrv2Polyline, TrimCrv2Polyline           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSetTrimCrvLinearApprox                                               M
*****************************************************************************/
SymbCrvApproxMethodType TrimSetTrimCrvLinearApprox(CagdRType UVTolSamples,
					   SymbCrvApproxMethodType UVMethod)
{
    SymbCrvApproxMethodType
	OldMethod = _TrimUVCrvApproxMethod;

    _TrimUVCrvApproxTolSamples = UVTolSamples,
    _TrimUVCrvApproxMethod = UVMethod;

    return OldMethod;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the current tolerance used when approximating higher order trimming  M
* curves using piecewise linear approximation.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   None				                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Sought tolerance.			                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSetTrimCrvLinearApprox				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimGetTrimCrvLinearApprox                                               M
*****************************************************************************/
CagdRType TrimGetTrimCrvLinearApprox(void)
{
    return _TrimUVCrvApproxTolSamples;
}

