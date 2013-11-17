/******************************************************************************
* Trim_aux.c - auxiliary routine to interface to different free from types.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include "trim_loc.h"

#define TRIM_CRV_PT_DIFF	1e-4
#define INSIDE_TCRV_IRIT_EPS	1e-3
#define TRIM_CRV_MIN_SAMPLES	2

#define SRF_EVAL_INTO_CRV_CTLPT(Srf, u, v, Points, Index) { \
        int j; \
        CagdRType *R = CagdSrfEval(Srf, u, v); \
	if (IsRational) { \
	    for (j = 1; j <= MaxAxis; j++) \
	        Points[j][Index] = R[j] / R[0]; \
	} \
	else { \
	    for (j = 1; j <= MaxAxis; j++) \
	        Points[j][Index] = R[j]; \
	} \
    }


IRIT_GLOBAL_DATA CagdBType
    _TrimEuclidComposedFromUV = FALSE;

static TrimCrvStruct *TrimChainTrimmingCurves2LoopsAux(TrimCrvStruct *TrimCrvs,
						       IrtRType Tol);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the parametric domain of a trimmed surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   To get its parametric domain.                                 M
*   UMin:      Where to put the minimal U domain's boundary.                 M
*   UMax:      Where to put the maximal U domain's boundary.                 M
*   VMin:      Where to put the minimal V domain's boundary.                 M
*   VMax:      Where to put the maximal V domain's boundary.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDomain					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfDomain, domain, parametric domain                                 M
*****************************************************************************/
void TrimSrfDomain(const TrimSrfStruct *TrimSrf,
		   CagdRType *UMin,
		   CagdRType *UMax,
		   CagdRType *VMin,
		   CagdRType *VMax)
{
    CagdSrfDomain(TrimSrf -> Srf, UMin, UMax, VMin, VMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trimmed surface and parameter values u, v, evaluate the surface at M
* (u, v).  No test is made to make sure (u, v) is in the untrimmed domain.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf: To evaluate at the given parametric location (u, v).            M
*   u, v:    The parameter values at which TrimSrf is to be evaluated.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: A vector holding all the coefficients of all components     M
*                of surface TrimSrf's point type. If, for example, TrimSrf's M
*                point type is P2, the W, X, and Y will be saved in the      M
*                first three locations of the returned vector. The first     M
*                location (index 0) of the returned vector is reserved for   M
*                the rational coefficient W and XYZ always starts at second  M
*                location of the returned vector (index 1).                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfEval						                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfEval, evaluation                                                  M
*****************************************************************************/
CagdRType *TrimSrfEval(const TrimSrfStruct *TrimSrf, CagdRType u, CagdRType v)
{
    return CagdSrfEval(TrimSrf -> Srf, u, v);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new trimmed surface representing the same surface as TrimSrf but M
* with its degree raised by one.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       To raise its degree.                                      M
*   Dir:       Direction of degree raising. Either U or V.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  A surface with same geometry as Srf but with one       M
*                     degree higher.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfDegreeRaise, degree raising                                       M
*****************************************************************************/
TrimSrfStruct *TrimSrfDegreeRaise(const TrimSrfStruct *TrimSrf,
				  CagdSrfDirType Dir)
{
    return TrimSrfNew(CagdSrfDegreeRaise(TrimSrf -> Srf, Dir),
		      TrimCrvCopyList(TrimSrf -> TrimCrvList),
		      FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trimmed surface - extracts a sub-region within the domain          M
* specified by t1 and t2, in the direction Dir.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   To extract a sub-region from.                                 M
*   t1, t2:    Parametric domain boundaries of sub-region.                   M
*   Dir:       Direction of region extraction. Either U or V.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  Sub-region extracted from TrimSrf from t1 to t2.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfSubdivAtParam, CagdSrfRegionFromSrf		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfRegionFromTrimSrf, regions, subdivision                           M
*****************************************************************************/
TrimSrfStruct *TrimSrfRegionFromTrimSrf(TrimSrfStruct *TrimSrf,
					CagdRType t1,
					CagdRType t2,
					CagdSrfDirType Dir)
{
    CagdRType TMin, TMax, R1, R2;
    TrimSrfStruct *TrimSrfs;
    CagdSrfStruct
	*Srf = TrimSrf -> Srf;
    TrimSrfStruct
        *CpTrimSrf;

    if (t1 > t2)
	IRIT_SWAP(CagdRType, t1, t2);

    if (Dir == CAGD_CONST_U_DIR)
	TrimSrfDomain(TrimSrf, &TMin, &TMax, &R1, &R2);
    else
	TrimSrfDomain(TrimSrf, &R1, &R2, &TMin, &TMax);
    CAGD_DOMAIN_T_VERIFY(t1, TMin, TMax);
    CAGD_DOMAIN_T_VERIFY(t2, TMin, TMax);

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    /* Update t2 to be between t1 and TMax as it will come back     */
	    /* after the first subdivision to be between zero and one.      */
	    t2 = 1.0 - (1.0 - t2) / (1.0 - t1);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    break;
	case CAGD_SPOWER_TYPE:
	    TRIM_FATAL_ERROR(TRIM_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    TRIM_FATAL_ERROR(TRIM_ERR_UNDEF_SRF);
	    return NULL;
    }

    if (!IRIT_APX_EQ(t1, TMin)) {
	TrimSrfs = TrimSrfSubdivAtParam(TrimSrf, t1, Dir);
	TrimSrf = CpTrimSrf = TrimSrfs -> Pnext;
	TrimSrfs -> Pnext = NULL;
	if (TRIM_IS_FIRST_SRF(TrimSrfs))
	    TrimSrfFree(TrimSrfs);		   /* Free the first region. */
	if (TrimSrf == NULL)
	    return NULL;      /* No second region - completely trimmed away. */
    }
    else
        CpTrimSrf = NULL;

    if (IRIT_APX_EQ(t2, TMax))
	return CpTrimSrf != NULL ? CpTrimSrf : TrimSrfCopy(TrimSrf);
    else {
	TrimSrfs = TrimSrfSubdivAtParam(TrimSrf, t2, Dir);

	if (CpTrimSrf != NULL)
	    TrimSrfFree(CpTrimSrf);

    	if (TrimSrfs -> Pnext != NULL)
	    TrimSrfFree(TrimSrfs -> Pnext);	  /* Free the second region. */
    	TrimSrfs -> Pnext = NULL;
	return TrimSrfs;			/* Returns the first region. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds a point inside a set of trimmed crvs.  Returned is a UV location   M
* allocated statically.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList:       To find a location inside it.                         M
*   TSrf:	       If provided, will attempt to find a point inside the  M
*		       trimmed curve on the boundary. If NULL, an interior   M
*		       point will be selected.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    A location in the parametric space of the surface that   M
*		    is part of the valid trimmed surface domain.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimPointInsideTrimmedCrvs                                               M
*****************************************************************************/
CagdRType *TrimPointInsideTrimmedCrvs(TrimCrvStruct *TrimCrvList,
				      const TrimSrfStruct *TSrf)
{
    IRIT_STATIC_DATA CagdUVType UVRetVal;
    int i;
    CagdRType XLevel, YLevel, *X, *Y,
	UMin = -IRIT_INFNTY,
	UMax = IRIT_INFNTY,
	VMin = -IRIT_INFNTY,
        VMax = IRIT_INFNTY;
    TrimSrfStruct *CpTSrf;
    CagdCrvStruct *TmpCrv, *UVCrv;
    TrimIsoInterStruct **IntersVec, *Inters;

    if (TSrf != NULL) {
        CagdSrfDomain(TSrf -> Srf, &UMin, &UMax, &VMin, &VMax);
	if (TrimCrvList == NULL) {
	    UVRetVal[0] = (UMin + UMax) * 0.5;
	    UVRetVal[1] = (VMin + VMax) * 0.5;

	    return UVRetVal;
	}
	CpTSrf = NULL;
    }
    else
        TSrf = CpTSrf = TrimSrfNew(NULL, TrimCrvList, TRUE);

    /* Find the average point of the control polygon of UVCrv and use it to  */
    /* construct a vertical line to +/-Y.				     */
    UVCrv = TrimCrvList -> TrimCrvSegList -> UVCrv;
    TmpCrv = CagdCoerceCrvTo(UVCrv, CAGD_PT_E2_TYPE, FALSE);
    X = TmpCrv -> Points[1];
    Y = TmpCrv -> Points[2];
    for (i = 0, XLevel = 0.0, YLevel = 0.0; i < TmpCrv -> Length; i++) {
        XLevel += *X++;
        YLevel += *Y++;
    }
    XLevel /= TmpCrv -> Length;
    YLevel /= TmpCrv -> Length;
    CagdCrvFree(TmpCrv);

    /* Get all intersections along this iso-direction. */
    IntersVec = TrimIntersectTrimCrvIsoVals(TSrf, CAGD_CONST_U_DIR,
					    &XLevel, 1, FALSE);
    Inters = IntersVec[0];
    IritFree(IntersVec);

    if (Inters != NULL) {
        if (Inters -> Pnext == NULL) {
	    TRIM_FATAL_ERROR(TRIM_ERR_ODD_NUM_OF_INTER);
	    return NULL;
	}
        UVRetVal[0] = XLevel;
	UVRetVal[1] = (Inters -> Param + Inters -> Pnext -> Param) * 0.5;
    }
    else {
        IntersVec = TrimIntersectTrimCrvIsoVals(TSrf, CAGD_CONST_V_DIR,
						&YLevel, 1, FALSE);
	Inters = IntersVec[0];
	IritFree(IntersVec);

	if (Inters != NULL) {
	    if (Inters -> Pnext == NULL) {
	        TRIM_FATAL_ERROR(TRIM_ERR_ODD_NUM_OF_INTER);
		return NULL;
	    }
	    UVRetVal[0] = (Inters -> Param + Inters -> Pnext -> Param) * 0.5;
	    UVRetVal[1] = YLevel;
	}
    }

    while (Inters) {
        TrimIsoInterStruct *Inter;

        IRIT_LIST_POP(Inter, Inters);
	IritFree(Inter);
    }

    if (CpTSrf != NULL) {
        CpTSrf -> TrimCrvList = NULL;
	TrimSrfFree(CpTSrf);
    }

    return UVRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trimmed surface - refines it at the given n knots as defined by    M
* vector t.								     M
*   If Replace is TRUE, the values in t replaces current knot vector.	     M
*   Returns pointer to refined surface (Note a Bezier surface will be        M
* converted into a Bspline surface).                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   To refine.                                                    M
*   Dir:       Direction of refinement. Either U or V.                       M
*   Replace:   If TRUE, t holds knots in exactly the same length as the      M
*              length of the knot vector of Srf and t simply replaces the    M
*              knot vector.                                                  M
*   t:         Vector of knots with length of n.                             M
*   n:         Length of vector t.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  A refined surface of TrimSrf after insertion of all    M
*                     the knots as specified by vector t of length n.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfRefineAtParams				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfRefineAtParams, refinement, subdivision                           M
*****************************************************************************/
TrimSrfStruct *TrimSrfRefineAtParams(const TrimSrfStruct *TrimSrf,
				     CagdSrfDirType Dir,
				     CagdBType Replace,
				     CagdRType *t,
				     int n)
{
    return TrimSrfNew(CagdSrfRefineAtParams(TrimSrf -> Srf, Dir, Replace, t, n),
		      TrimCrvCopyList(TrimSrf -> TrimCrvList), FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new trimmed surface that is the reversed surface of TrimSrf by   M
* reversing the control mesh and the knot vector (if Bspline surface) of     M
* TrimSrf in the U direction, as well as its trimming curves. See also	     M
* CagdSrfReverse and BspKnotReverse.      		                     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       To be reversed.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   Reversed surface of TrimSrf.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfReverse2, CagdSrfReverse			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfReverse, reverse                                                  M
*****************************************************************************/
TrimSrfStruct *TrimSrfReverse(const TrimSrfStruct *TrimSrf)
{
    CagdRType UMin, UMax, VMin, VMax;
    TrimCrvStruct *TrimCrv,
	*TrimCrvList = TrimCrvCopyList(TrimSrf -> TrimCrvList);

    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);

    for (TrimCrv = TrimCrvList; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct *TrimCrvSeg;

	for (TrimCrvSeg = TrimCrv -> TrimCrvSegList;
	     TrimCrvSeg != NULL;
	     TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int i,
		Length = TrimCrvSeg -> UVCrv -> Length;
	    CagdRType
		**Points = TrimCrvSeg -> UVCrv -> Points,
		*UPts = Points[1];

	    for (i = 0; i < Length; i++)
		UPts[i] = UMax - (UPts[i] - UMin);
	}
    }

    return TrimSrfNew(CagdSrfReverse(TrimSrf -> Srf), TrimCrvList, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new trimmed surface that is the reversed surface of Srf by       M
* flipping the U and the V directions of the surface, as well as flipping    M
* them in the trimming curves.						     M
* See also BspKnotReverse.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       To be reversed.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   Reversed surface of TrimSrf.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfReverse, CagdSrfReverse2			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfReverse2, reverse                                                 M
*****************************************************************************/
TrimSrfStruct *TrimSrfReverse2(const TrimSrfStruct *TrimSrf)
{
    TrimCrvStruct *TrimCrv,
	*TrimCrvList = TrimCrvCopyList(TrimSrf -> TrimCrvList);

    for (TrimCrv = TrimCrvList; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct *TrimCrvSeg;

	for (TrimCrvSeg = TrimCrv -> TrimCrvSegList;
	     TrimCrvSeg != NULL;
	     TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int i,
		Length = TrimCrvSeg -> UVCrv -> Length;
	    CagdRType
		**Points = TrimCrvSeg -> UVCrv -> Points,
		*UPts = Points[1],
		*VPts = Points[2];

	    if (TrimCrvSeg -> UVCrv -> PType != CAGD_PT_E2_TYPE)
		TRIM_FATAL_ERROR(TRIM_ERR_TRIM_CRV_E2);
	    for (i = 0; i < Length; i++)
		IRIT_SWAP(CagdRType, UPts[i], VPts[i]);
	}
    }

    return TrimSrfNew(CagdSrfReverse2(TrimSrf -> Srf), TrimCrvList, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts the trimming curves of the given trimmed surface.               M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       Trimmed surface to extract trimming curves from.          M
*   ParamSpace:    TRUE for curves in parameteric space, FALSE of 3D         M
*		   Euclidean space.					     M
*   EvalEuclid:    If TRUE and ParamSpace is FALSE, evaluate Euclidean curve M
*		   even if one exists.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of trimming curves of TrimSrf.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimPiecewiseLinearTrimmingCurves, TrimGetTrimmingCurves2		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimGetTrimmingCurves, trimming curves                                   M
*****************************************************************************/
CagdCrvStruct *TrimGetTrimmingCurves(const TrimSrfStruct *TrimSrf,
				     CagdBType ParamSpace,
				     CagdBType EvalEuclid)
{
    return TrimGetTrimmingCurves2(TrimSrf -> TrimCrvList, TrimSrf,
				  ParamSpace, EvalEuclid);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts the trimming curves as curves from the given trimming curves.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList:   Trimming curves to extract trimming curves as curves.     M
*   TrimSrf:       Trimmed surface to extract trimming curves from.  This    M
*		   parameter is optional and used only if EvalEuclid and/or  M
8		   !ParamSpace.						     M
*   ParamSpace:    TRUE for curves in parameteric space, FALSE of 3D         M
*		   Euclidean space.					     M
*   EvalEuclid:    If TRUE and ParamSpace is FALSE, evaluate Euclidean curve M
*		   even if one exists.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of trimming curves as curves.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimPiecewiseLinearTrimmingCurves, TrimGetTrimmingCurves                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimGetTrimmingCurves2, trimming curves                                  M
*****************************************************************************/
CagdCrvStruct *TrimGetTrimmingCurves2(const TrimCrvStruct *TrimCrvList,
				      const TrimSrfStruct *TrimSrf,
				      CagdBType ParamSpace,
				      CagdBType EvalEuclid)
{
    CagdCrvStruct
	*NewTrimCrvList = NULL;

    for ( ; TrimCrvList != NULL; TrimCrvList = TrimCrvList -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegList = TrimCrvList -> TrimCrvSegList;

	for (;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    CagdCrvStruct *TCrv;

	    if (ParamSpace)
		TCrv = CagdCrvCopy(TrimCrvSegList -> UVCrv);
	    else {
		if (EvalEuclid && TrimSrf != NULL) {
		    TCrv = TrimEvalTrimCrvToEuclid(TrimSrf,
						   TrimCrvSegList -> UVCrv);
		}
		else {
		    if (TrimCrvSegList -> EucCrv == NULL && TrimSrf != NULL) {
			TrimCrvSegList -> EucCrv =
			    TrimEvalTrimCrvToEuclid(TrimSrf,
						    TrimCrvSegList -> UVCrv);
		    }
		    TCrv = TrimCrvSegList -> EucCrv != NULL ?
				CagdCrvCopy(TrimCrvSegList -> EucCrv) : NULL;
		}
	    }
	    IRIT_LIST_PUSH(TCrv, NewTrimCrvList);
	}
    }

    return NewTrimCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Converts all trimming curve of given surface to piecewise linear	     M
* (approximation), in place.  The trimming curves are approximated as        M
* piecewise linear using method and tolerance that is set via the function   M
* TrimSetTrimCrvLinearApprox. 					             M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       Trimmed surface to extract trimming curves from.          M
*   EvalEuclid:    If TRUE reevaluate Euclidean curve as well.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   The trimmed surface, modified in place, that holds    M
*		       piecewise linear trimming curves only.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimGetTrimmingCurves, TrimSetTrimCrvLinearApprox, TrimCrv2Polyline      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimPiecewiseLinearTrimmingCurves, trimming curves                       M
*****************************************************************************/
TrimSrfStruct *TrimPiecewiseLinearTrimmingCurves(TrimSrfStruct *TrimSrf,
						 CagdBType EvalEuclid)
{
    TrimCrvStruct
	*TrimCrvList = TrimSrf -> TrimCrvList,
        *TCrvs = TrimChainTrimmingCurves2Loops(TrimCrvList);

    if (TCrvs != NULL) {
	TrimCrvFreeList(TrimCrvList);
	TrimSrf -> TrimCrvList = TrimCrvList = TCrvs;
    }

    for ( ; TrimCrvList != NULL; TrimCrvList = TrimCrvList -> Pnext) {
	TrimCrvSegStruct *TrimCrvSegList;
	CagdPType LastPt;
	CagdRType **Points;

	LastPt[0] = LastPt[1] = IRIT_INFNTY;

	for (TrimCrvSegList = TrimCrvList -> TrimCrvSegList;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    int Len;

	    if (TrimCrvSegList -> UVCrv -> Order > 2) {
	        CagdPolylineStruct
		    *UVPoly = TrimCrv2Polyline(TrimCrvSegList -> UVCrv,
					       _TrimUVCrvApproxTolSamples,
					       _TrimUVCrvApproxMethod, TRUE);

		CagdCrvFree(TrimCrvSegList -> UVCrv);
		TrimCrvSegList -> UVCrv = CagdCnvrtPolyline2LinBspCrv(UVPoly);
		CagdPolylineFree(UVPoly);
	    }

	    if (CAGD_IS_RATIONAL_CRV(TrimCrvSegList -> UVCrv)) {
	        CagdCrvStruct
		    *TCrv = CagdCoerceCrvTo(TrimCrvSegList -> UVCrv,
					    CAGD_PT_E2_TYPE, FALSE);

		CagdCrvFree(TrimCrvSegList -> UVCrv);
		TrimCrvSegList -> UVCrv = TCrv;
	    }

	    /* Make sure end points are the same if a closed loop. */
	    Points = TrimCrvSegList -> UVCrv -> Points;
	    Len = TrimCrvSegList -> UVCrv -> Length;

	    if (TrimCrvSegList != TrimCrvList -> TrimCrvSegList) {
	        Points[1][0] = LastPt[0];
	        Points[2][0] = LastPt[1];
	    }
	    LastPt[0] = Points[1][Len - 1];
	    LastPt[1] = Points[2][Len - 1];

	    if (TrimCrvSegList -> EucCrv != NULL) {
	        CagdCrvFree(TrimCrvSegList -> EucCrv);
		TrimCrvSegList -> EucCrv = NULL;
	    }

	    if (EvalEuclid) {
	        TrimCrvSegList -> EucCrv = TrimEvalTrimCrvToEuclid(TrimSrf,
						     TrimCrvSegList -> UVCrv);
	    }
	}

	Points = TrimCrvList -> TrimCrvSegList -> UVCrv -> Points;
	Points[1][0] = LastPt[0];
	Points[2][0] = LastPt[1];
    }

    return TrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Chains all given trimming curves into closed loops.			     M
*  Only UV curves are chained and Euclidean trimming curves are purged.      M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvs:   Trimming curves to chain into loops.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:   Trimming curves chained into loops.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimChainTrimmingCurves2Loops2			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimChainTrimmingCurves2Loops, trimming curves		             M
*****************************************************************************/
TrimCrvStruct *TrimChainTrimmingCurves2Loops(TrimCrvStruct *TrimCrvs)
{
    IrtRType Tol;

    /* Try different tolerances to merge the trimming loops. */
    for (Tol = IRIT_UEPS; Tol < 1.0; Tol *= 10) {
        TrimCrvStruct
	    *TCrvs = TrimChainTrimmingCurves2LoopsAux(TrimCrvs, Tol);

	if (TCrvs)
	    return TCrvs;
    }

    TRIM_FATAL_ERROR(TRIM_ERR_TRIMS_NOT_LOOPS);
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of TrimChainTrimmingCurves2Loops, attempting          *
* different tolerances.							     *
*****************************************************************************/
static TrimCrvStruct *TrimChainTrimmingCurves2LoopsAux(TrimCrvStruct *TrimCrvs,
						       IrtRType Tol)
{
    TrimCrvStruct *TrimCrv;
    CagdCrvStruct *UVCrv,
	*UVCrvs = NULL,
	*UVLoops = NULL;

    /* Chain all trimming curves into one linear list. */
    for (TrimCrv = TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct *TrimCrvSegList;

	for (TrimCrvSegList = TrimCrv -> TrimCrvSegList;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    UVCrv = CagdCrvCopy(TrimCrvSegList -> UVCrv);
	    IRIT_LIST_PUSH(UVCrv, UVCrvs);
	}
    }

    if ((UVLoops = TrimChainTrimmingCurves2Loops2(UVCrvs, Tol)) != NULL) {
        /* Convert to trimming curves. */
        TrimCrvs = NULL;
	while (UVLoops != NULL) {
	    UVCrv = UVLoops;
	    UVLoops = UVLoops -> Pnext;
	    UVCrv -> Pnext = NULL;

	    TrimCrv = TrimCrvNew(TrimCrvSegNew(UVCrv, NULL));
	    IRIT_LIST_PUSH(TrimCrv, TrimCrvs);
	}

	return TrimCrvs;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Chains all given curves into closed loops.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrvs:   Curves to chain into loops.				     M
*   Tol:      Tolerance of end points comparisons.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Curves chained into loops.	                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimChainTrimmingCurves2Loops			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimChainTrimmingCurves2Loops2, trimming curves		             M
*****************************************************************************/
CagdCrvStruct *TrimChainTrimmingCurves2Loops2(CagdCrvStruct *UVCrvs,
					      CagdRType Tol)
{
    CagdCrvStruct *UVCrv,
	*UVLoops = NULL;

    while (UVCrvs != NULL) {
        CagdBType
	    FoundMatch = FALSE;
	CagdUVType UV1Start, UV1End;
        CagdCrvStruct
	    *UVLoop = UVCrvs;

	if (AttrGetIntAttrib(UVLoop -> Attr, "_used") == TRUE) {
	    UVCrvs = UVCrvs -> Pnext;
	    CagdCrvFree(UVLoop);
	    continue;
	}

	/* Evaluate the end points of the first curve. */
	CagdCoerceToE2(UV1Start, UVLoop -> Points, 0, UVLoop -> PType);
	CagdCoerceToE2(UV1End, UVLoop -> Points, UVLoop -> Length - 1,
		       UVLoop -> PType);
	UVCrvs = UVCrvs -> Pnext;
	UVLoop -> Pnext = NULL;
	if (IRIT_PT_APX_EQ_E2_EPS(UV1Start, UV1End, Tol)) {
	    /* It is a closed loop - place in output set and continue. */
	    IRIT_LIST_PUSH(UVLoop, UVLoops);
	    continue;
	}

	for (UVCrv = UVCrvs; UVCrv != NULL; UVCrv = UVCrv -> Pnext) {
	    CagdUVType UV2Start, UV2End;
	    CagdCrvStruct *TCrv1, *TCrv2;

	    if (AttrGetIntAttrib(UVCrv -> Attr, "_used") == TRUE)
	        continue;
	    
	    /* Evaluate the end points of the second curve. */
	    CagdCoerceToE2(UV2Start, UVCrv -> Points, 0, UVCrv -> PType);
	    CagdCoerceToE2(UV2End, UVCrv -> Points, UVCrv -> Length - 1,
			   UVCrv -> PType);

	    if (IRIT_PT_APX_EQ_E2_EPS(UV1Start, UV2Start, Tol)) {
	        TCrv1 = CagdCrvReverse(UVLoop);
		CagdCrvFree(UVLoop);
		UVLoop = CagdMergeCrvCrv(TCrv1, UVCrv, FALSE);
		CagdCrvFree(TCrv1);
		FoundMatch = TRUE;
	    }
	    else if (IRIT_PT_APX_EQ_E2_EPS(UV1Start, UV2End, Tol)) {
	        TCrv1 = CagdCrvReverse(UVLoop);
	        TCrv2 = CagdCrvReverse(UVCrv);
		CagdCrvFree(UVLoop);
		UVLoop = CagdMergeCrvCrv(TCrv1, TCrv2, FALSE);
		CagdCrvFree(TCrv1);
		CagdCrvFree(TCrv2);
		FoundMatch = TRUE;
	    }
	    else if (IRIT_PT_APX_EQ_E2_EPS(UV1End, UV2Start, Tol)) {
		TCrv1 = CagdMergeCrvCrv(UVLoop, UVCrv, FALSE);
		CagdCrvFree(UVLoop);
		UVLoop = TCrv1;
		FoundMatch = TRUE;
	    }
	    else if (IRIT_PT_APX_EQ_E2_EPS(UV1End, UV2End, Tol)) {
	        TCrv2 = CagdCrvReverse(UVCrv);
		TCrv1 = CagdMergeCrvCrv(UVLoop, TCrv2, FALSE);
		CagdCrvFree(UVLoop);
		UVLoop = TCrv1;
		CagdCrvFree(TCrv2);
		FoundMatch = TRUE;
	    }

	    if (FoundMatch) {
	        AttrSetIntAttrib(&UVCrv -> Attr, "_used", TRUE);
		break;
	    }
	}
	
	if (!FoundMatch) {
	    CagdCrvFreeList(UVLoops);
	    return NULL;
	}
	else {
	    IRIT_LIST_PUSH(UVLoop, UVCrvs);
	}
    }


    return UVLoops;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Heal the given trimming curves by making sure they are valid and form    M
* closed loops.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvs:       Trimming curves to heal.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:  Healed trimming curves.		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimHealTrimmingCurves, trimming curves		                     M
*****************************************************************************/
TrimCrvStruct *TrimHealTrimmingCurves(TrimCrvStruct *TrimCrvs)
{
    TrimCrvStruct
	*TCrvs = TrimChainTrimmingCurves2Loops(TrimCrvs);

    TrimCrvs = TCrvs;

    for ( ; TrimCrvs != NULL; TrimCrvs = TrimCrvs -> Pnext) {
	TrimCrvSegStruct *TrimCrvSegList;
	CagdCrvStruct
	    *LastUVCrv = NULL;
	CagdPType FirstPt, LastPt;

	LastPt[0] = LastPt[1] = IRIT_INFNTY;

	for (TrimCrvSegList = TrimCrvs -> TrimCrvSegList;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    CagdRType *R, TMin, TMax;
	    CagdPType PtStart, PtEnd;
	    CagdCrvStruct
		*UVCrv = TrimCrvSegList -> UVCrv;

	    CagdCrvDomain(UVCrv, &TMin, &TMax);

	    R = CagdCrvEval(UVCrv, TMin);
	    CagdCoerceToE2(PtStart, &R, -1, UVCrv -> PType);

	    R = CagdCrvEval(UVCrv, TMax);
	    CagdCoerceToE2(PtEnd, &R, -1, UVCrv -> PType);

	    if (LastUVCrv == NULL)
		IRIT_PT2D_COPY(FirstPt, PtStart);
	    else {
		if (FirstPt[0] != LastPt[0] || FirstPt[1] != LastPt[1]) {
		    if (UVCrv -> Order == 2 || BspCrvHasOpenEC(UVCrv)) {
		        UVCrv -> Points[1][0] = LastPt[0];
		        UVCrv -> Points[2][0] = LastPt[1];
			if (CAGD_IS_RATIONAL_CRV(UVCrv))
			    UVCrv -> Points[0][0] = 1.0;
		    }
		    else if (LastUVCrv -> Order == 2 ||
			     BspCrvHasOpenEC(LastUVCrv)) {
		        int Idx = LastUVCrv -> Length - 1;

		        LastUVCrv -> Points[1][Idx] = FirstPt[0];
		        LastUVCrv -> Points[2][Idx] = FirstPt[1];
			if (CAGD_IS_RATIONAL_CRV(LastUVCrv))
			    LastUVCrv -> Points[0][Idx] = 1.0;
		    }
		    else {
			TRIM_FATAL_ERROR(TRIM_ERR_TRIM_OPEN_LOOP);
		    }
		}
	    }

	    if (TrimCrvSegList -> Pnext == NULL) {
	        CagdCrvStruct
		    *FirstUVCrv = TrimCrvs -> TrimCrvSegList -> UVCrv;

		/* Compare last point of all the chain with first. */
		CagdCrvDomain(FirstUVCrv, &TMin, &TMax);

		R = CagdCrvEval(FirstUVCrv, TMin);
		CagdCoerceToE2(FirstPt, &R, -1, FirstUVCrv -> PType);

		if (FirstPt[0] != PtEnd[0] || FirstPt[1] != PtEnd[1]) {
		    if (UVCrv -> Order == 2 || BspCrvHasOpenEC(UVCrv)) {
		        UVCrv -> Points[1][0] = PtEnd[0];
			UVCrv -> Points[2][0] = PtEnd[1];
			if (CAGD_IS_RATIONAL_CRV(UVCrv))
			    UVCrv -> Points[0][0] = 1.0;
		    }
		    else if (FirstUVCrv -> Order == 2 ||
			     BspCrvHasOpenEC(FirstUVCrv)) {
		        int Idx = FirstUVCrv -> Length - 1;

			FirstUVCrv -> Points[1][Idx] = FirstPt[0];
			FirstUVCrv -> Points[2][Idx] = FirstPt[1];
			if (CAGD_IS_RATIONAL_CRV(FirstUVCrv))
			    FirstUVCrv -> Points[0][Idx] = 1.0;
		    }
		    else {
		        TRIM_FATAL_ERROR(TRIM_ERR_TRIM_OPEN_LOOP);
		    }
		}
	    }
	    else {
	        IRIT_PT2D_COPY(LastPt, PtEnd);
	    }
	}
    }

    return TCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Map the given trimming curves into a new domain, in place.               M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList:       Trimming curves to affinely map.			     M
*   OldUMin, OldUMax, OldVMin, OldVMax:  Domain to map trimming curves from. M
*   NewUMin, NewUMax, NewVMin, NewVMax:  Domain to map trimming curves to.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimAffineTransTrimSrf				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimAffineTransTrimCurves, trimming curves                               M
*****************************************************************************/
void TrimAffineTransTrimCurves(TrimCrvStruct *TrimCrvList,
			       CagdRType OldUMin,
			       CagdRType OldUMax,
			       CagdRType OldVMin,
			       CagdRType OldVMax,
			       CagdRType NewUMin,
			       CagdRType NewUMax,
			       CagdRType NewVMin,
			       CagdRType NewVMax)
{
    IrtHmgnMatType Mat, Mat1, Mat2;

    /* Construct the transformation matrix. */
    MatGenMatTrans(-OldUMin, -OldVMin, 0.0, Mat);  /* Move corner to origin. */
    MatGenMatScale((NewUMax - NewUMin) / (OldUMax - OldUMin),
		   (NewVMax - NewVMin) / (OldVMax - OldVMin),
		   1.0, Mat1);			      /* Scale to new sizes. */
    MatGenMatTrans(NewUMin, NewVMin, 0.0, Mat2);  /* Move corner to new loc. */
    MatMultTwo4by4(Mat, Mat, Mat1);
    MatMultTwo4by4(Mat, Mat, Mat2);

    /* And map all UV corves. */
    for ( ; TrimCrvList != NULL; TrimCrvList = TrimCrvList -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSegList = TrimCrvList -> TrimCrvSegList;

	for ( ;
	     TrimCrvSegList != NULL;
	     TrimCrvSegList = TrimCrvSegList -> Pnext) {
	    CagdCrvStruct *TCrv;

	    TCrv = CagdCrvMatTransform(TrimCrvSegList -> UVCrv, Mat);
	    CagdCrvFree(TrimCrvSegList -> UVCrv);
	    TrimCrvSegList -> UVCrv = TCrv;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Maps the given trimmed surface into a new domain.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   CTrimSrf:      Trimmed surface to affinely map its parametric domain.    M
*   NewUMin, NewUMax, NewVMin, NewVMax:  New parametric domain to map to.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:	A trimmed surface that is geometrically identical    M
*			but with new different parametric domain.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotAffineTransOrder2, TrimAffineTransTrimCurves                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimAffineTransTrimSrf, trimming curves                                  M
*****************************************************************************/
TrimSrfStruct *TrimAffineTransTrimSrf(const TrimSrfStruct *CTrimSrf,
				      CagdRType NewUMin,
				      CagdRType NewUMax,
				      CagdRType NewVMin,
				      CagdRType NewVMax)
{
    CagdRType OldUMin, OldUMax, OldVMin, OldVMax;
    CagdSrfStruct *Srf;
    TrimSrfStruct
        *TrimSrf = TrimSrfCopy(CTrimSrf);

    Srf = TrimSrf -> Srf;
    CagdSrfDomain(Srf, &OldUMin, &OldUMax, &OldVMin, &OldVMax);

    /* Map the know sequences to their new values. */
    if (CAGD_IS_BEZIER_SRF(Srf)) {
	Srf = CagdCnvrtBzr2BspSrf(Srf);
	CagdSrfFree(TrimSrf -> Srf);
	TrimSrf -> Srf = Srf;
    }

    BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
			     CAGD_SRF_UPT_LST_LEN(Srf) + Srf -> UOrder,
			     NewUMin, NewUMax);
    BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
			     CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder,
			     NewVMin, NewVMax);
    
    /* Update the trimming curves to the new domain. */
    TrimAffineTransTrimCurves(TrimSrf -> TrimCrvList,
			      OldUMin, OldUMax, OldVMin, OldVMax,
			      NewUMin, NewUMax, NewVMin, NewVMax);

    return TrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert the trimming curves of a trimmed surface to polylines.M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. 							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       To extract isoparametric curves from.                     M
*   ParamSpace:    TRUE for curves in parameteric space, FALSE of 3D         M
*		   Euclidean space.					     M
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
*   TrimCrv2Polyline, TrimEvalTrimCrvToEuclid		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvs2Polylines, trimming curves 	                             M
*****************************************************************************/
CagdPolylineStruct *TrimCrvs2Polylines(TrimSrfStruct *TrimSrf,
				       CagdBType ParamSpace,
				       CagdRType TolSamples,
				       SymbCrvApproxMethodType Method)
{
    CagdRType OldApproxTol;
    CagdCrvStruct *Crv, *Crvs;
    CagdPolylineStruct *Poly,
	*Polys = NULL;
    SymbCrvApproxMethodType OldApproxMethod;

    OldApproxMethod = _TrimUVCrvApproxMethod;
    OldApproxTol = _TrimUVCrvApproxTolSamples;
    TrimSetTrimCrvLinearApprox(TolSamples, Method);

    Crvs = TrimGetTrimmingCurves(TrimSrf, ParamSpace, TRUE);

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	Poly = TrimCrv2Polyline(Crv, TolSamples, Method, TRUE);

	IRIT_LIST_PUSH(Poly, Polys);
    }

    CagdCrvFreeList(Crvs);

    TrimSetTrimCrvLinearApprox(OldApproxTol, OldApproxMethod);

    return Polys;	
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to approx. a single curve as a polyline with TolSamples          M
* samples/tolerance. Polyline is always E3 CagdPolylineStruct type.	     M
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrv:       To approximate as a polyline.                             M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*   OptiLin:       If TRUE, optimize linear curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing the piecewise linear      M
*                          approximation from, or NULL in case of an error.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrCrv2Polyline, IritCurve2Polylines, SymbCrv2Polyline, M
*   TrimCrvs2Polylines							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrv2Polyline, piecewise linear approximation, polyline               M
*****************************************************************************/
CagdPolylineStruct *TrimCrv2Polyline(const CagdCrvStruct *TrimCrv,
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method,
				     CagdBType OptiLin)
{
    int i, j;
    CagdPolylineStruct *Poly;

    if (TrimCrv -> Length > SYMB_MAX_CRV_SUBDIV_LEN) {   /* Way too large! */
        CagdPolylineStruct
            *Params = NULL;
	CagdPtStruct
	    *Pts = SymbHugeCrv2Polyline(TrimCrv, SYMB_MAX_CRV_SUBDIV_LEN,
					TRUE, TRUE, FALSE);

	
	Poly = CagdCnvrtPtList2Polyline(Pts, &Params);
	CagdPtFreeList(Pts);
	if (Params != NULL)
	    CagdPolylineFree(Params);

	return Poly;
    }
    else if (TrimCrv -> Order <= 2)
	Poly = CagdCnvrtLinBspCrv2Polyline(TrimCrv);
    else {
	if (Method == SYMB_CRV_APPROX_UNIFORM) {
	    Poly = SymbCrv2Polyline(TrimCrv,
				    IRIT_MAX(TRIM_CRV_MIN_SAMPLES, TolSamples),
				    SYMB_CRV_APPROX_UNIFORM, TRUE);
	}
	else {
	    int LastLen = -1;

	    do {
	        Poly = SymbCrv2Polyline(TrimCrv, TolSamples, Method, TRUE);
		if ((TrimCrv -> Order > 2 || LastLen < Poly -> Length) &&
		    Poly -> Length < TRIM_CRV_MIN_SAMPLES &&
		    TolSamples > IRIT_UEPS) {
		    /* Make sure we have enough samples along the curve. */
		    LastLen = Poly -> Length;
		    CagdPolylineFree(Poly);
		    Poly = NULL;
		    TolSamples *= 0.5;
	        }
	    }
	    while (Poly == NULL);
	}

	/* Filter duplicated points. */
	for (i = 1, j = 0; i < Poly -> Length; ) {
	    if (IRIT_PT_APX_EQ_EPS(Poly -> Polyline[j].Pt, Poly -> Polyline[i].Pt,
							    TRIM_CRV_PT_DIFF))
	        i++;
	    else {
	        j++;
		if (i != j)
		    IRIT_PT_COPY(Poly -> Polyline[j].Pt, Poly -> Polyline[i].Pt);
		i++;
	    }
	}
	/* Make sure last point is same as first if we skipped last few pts. */
	if (--i != j)
	    IRIT_PT_COPY(Poly -> Polyline[j].Pt, Poly -> Polyline[i].Pt);

	Poly -> Length = j + 1;
    }

    if (Poly -> Length > 1)
	return Poly;
    else {
        CagdPolylineFree(Poly);
        return CagdCrv2CtrlPoly(TrimCrv);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the composed Euclidean curve of Srf(UVCrv).		     M
* The resulting curve is either computed using a piecewise linear            M
* approximation or by symbolically composing it onto the surface. See        M
* TrimSetEuclidComposedFromUV for a way to control this computation.         M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:    To compute the Euclidean UVCrv for.                          M
*   UVCrv:      A curve in the parametric space of Srf.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A Euclidean curve in Srf, following UVCrv.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrvs2Polylines, TrimSetEuclidComposedFromUV,			     M
*   TrimEvalTrimCrvToEuclid2						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimEvalTrimCrvToEuclid                                                  M
*****************************************************************************/
CagdCrvStruct *TrimEvalTrimCrvToEuclid(const TrimSrfStruct *TrimSrf,
				       const CagdCrvStruct *UVCrv)
{
    return TrimEvalTrimCrvToEuclid2(TrimSrf -> Srf, UVCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the composed Euclidean curve of TrimSrf(UVCrv).		     M
* The resulting curve is either computed using a piecewise linear            M
* approximation or by symbolically composing it onto the surface. See        M
* TrimSetEuclidComposedFromUV for a way to control this computation.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute the Euclidean UVCrv for.                          M
*   UVCrv:      A curve in the parametric space of TrimSrf.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A Euclidean curve in TrimSrf, following UVCrv.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrvs2Polylines, TrimSetEuclidComposedFromUV, TrimEvalTrimCrvToEuclid M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimEvalTrimCrvToEuclid2                                                 M
*****************************************************************************/
CagdCrvStruct *TrimEvalTrimCrvToEuclid2(const CagdSrfStruct *Srf,
					const CagdCrvStruct *UVCrv)
{
    if (_TrimEuclidComposedFromUV) {
	return SymbComposeSrfCrv(Srf, UVCrv);
    }
    else {
	int i, l, Length,
	    MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType),
	    IsRational = CAGD_IS_RATIONAL_PT(Srf -> PType);
        CagdPointType
	    PType = IsRational ? CAGD_MAKE_PT_TYPE(FALSE, MaxAxis) :
				 Srf -> PType;
	CagdRType UMin, UMax, VMin, VMax, MinAdjPtsDist;
	CagdPolylineStruct *UVPoly;
	CagdPolylnStruct *UVPll;

	CagdCrvStruct *EucCrv;
	CagdRType **Points;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	UMin += IRIT_EPS;
	UMax -= IRIT_EPS;
	VMin += IRIT_EPS;
	VMax -= IRIT_EPS;
	/* Distance between adjacent points along trimming curve should be   */
	/* larger than MinAdjPtsDist as we compute it here.		     */
	switch (_TrimUVCrvApproxMethod) {
	    case SYMB_CRV_APPROX_UNIFORM:
	    case SYMB_CRV_APPROX_CURVATURE:
	        MinAdjPtsDist = 2 * IRIT_MIN(UMax - UMin, VMax - VMin) /
						_TrimUVCrvApproxTolSamples;
	        break;
	    case SYMB_CRV_APPROX_TOLERANCE:
		MinAdjPtsDist = IRIT_MIN(UMax - UMin, VMax - VMin) *
						_TrimUVCrvApproxTolSamples;
	        break;
	    default:
	        assert(0);
	        MinAdjPtsDist = 0.0;
		break;
	}

	UVPoly = TrimCrv2Polyline(UVCrv, _TrimUVCrvApproxTolSamples,
				         _TrimUVCrvApproxMethod, FALSE);
	UVPll = UVPoly -> Polyline;

	for (i = 0; i < UVPoly -> Length; i++) {
	    UVPll[i].Pt[0] = IRIT_BOUND(UVPll[i].Pt[0], UMin, UMax);
	    UVPll[i].Pt[1] = IRIT_BOUND(UVPll[i].Pt[1], VMin, VMax);
	}

	/* Estimate how many points will be in the output. */
	Length = UVPoly -> Length;
	for (i = 1; i < UVPoly -> Length; i++) {
	    CagdRType
	        Dist = IRIT_FABS(UVPll[i].Pt[0] - UVPll[i - 1].Pt[0]) +
		       IRIT_FABS(UVPll[i].Pt[1] - UVPll[i - 1].Pt[1]);

	    if (MinAdjPtsDist < Dist)
		Length += (int) (Dist / MinAdjPtsDist);
	}

	/* Create the trimming curve approximation in Euclidean space. */
	EucCrv = CagdCrvNew(UVCrv -> GType, PType, Length);
	EucCrv -> Order = 2;
	Points = EucCrv -> Points;

	if (CAGD_IS_BSPLINE_CRV(UVCrv)) {
	    EucCrv -> KnotVector = BspKnotUniformOpen(Length, 2, NULL);
	}

	/* Evaluate the first point. */
	SRF_EVAL_INTO_CRV_CTLPT(Srf, UVPll[0].Pt[0], UVPll[0].Pt[1], Points, 0);

	/* Evaluate the rest of the points. */
	for (i = l = 1; i < UVPoly -> Length; i++, l++) {
	    CagdRType
	        Dist = IRIT_FABS(UVPll[i].Pt[0] - UVPll[i - 1].Pt[0]) +
		       IRIT_FABS(UVPll[i].Pt[1] - UVPll[i - 1].Pt[1]);

	    if (MinAdjPtsDist < Dist) {
	        /* Blend between previous UV point and this one. */
	        int k,
		    NumMidPts = (int) (Dist / MinAdjPtsDist);

		for (k = 1; k <= NumMidPts; k++, l++) {
		    CagdUVType UV;

		    IRIT_UV_BLEND(UV, UVPll[i].Pt, UVPll[i - 1].Pt,
			     k / (NumMidPts + 1.0));
		    SRF_EVAL_INTO_CRV_CTLPT(Srf, UV[0], UV[1], Points, l);
		}
	    }

	    SRF_EVAL_INTO_CRV_CTLPT(Srf, UVPll[i].Pt[0], UVPll[i].Pt[1],
				    Points, l);
	}

	CagdPolylineFree(UVPoly);

	return EucCrv;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the way Euclidean trimming curves are computed from parametric      M
* trimming curves. Either by symbolic composition (TRUE) or by piecewise     M
* linear approximation of trimming curves (FALSE).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   EuclidComposedFromUV:   Do we want symbolic composition for Euclidean    M
*		curves, or should we piecewise linear sample the UV trimming M
*		curves.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value of way of Euclidean curve's computation                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrvs2Polylines					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSetEuclidComposedFromUV                                              M
*****************************************************************************/
int TrimSetEuclidComposedFromUV(int EuclidComposedFromUV)
{
    int OldEuclidComposedFromUV = _TrimEuclidComposedFromUV;

    _TrimEuclidComposedFromUV = EuclidComposedFromUV;

    return OldEuclidComposedFromUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the given UV value is inside the trimmed surface's       M
* parametric domain.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:        Trimmed surface to consider.                             M
*   UV:             Parametric location.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if inside, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIsPointInsideTrimUVCrv, TrimIsPointInsideTrimCrvs                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimIsPointInsideTrimSrf, inclusion		                             M
*****************************************************************************/
CagdBType TrimIsPointInsideTrimSrf(const TrimSrfStruct *TrimSrf, CagdUVType UV)
{
    return TrimIsPointInsideTrimCrvs(TrimSrf -> TrimCrvList, UV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the given UV value is inside the domain prescribed by    M
* the trimming curves.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvs:       Trimming curves to consider.                             M
*   UV:             Parametric location.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if inside, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIsPointInsideTrimUVCrv, TrimIsPointInsideTrimSrf,		     M
*   MdlIsPointInsideTrimSrf				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimIsPointInsideTrimCrvs, inclusion	                             M
*****************************************************************************/
CagdBType TrimIsPointInsideTrimCrvs(const TrimCrvStruct *TrimCrvs,
				    CagdUVType UV)
{
    int NumInters = 0;
    const TrimCrvStruct *TrimCrv;

    for (TrimCrv = TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	const TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    const CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;

	    NumInters += TrimIsPointInsideTrimUVCrv(UVCrv, UV);
	}
    }

    return NumInters & 0x01;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the number of times a ray in the +V direction from UV crosses    M
* UVCrv.		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrv:          Trimming curve to consider.                              M
*   UV:             Parametric location.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            Number of crossings of UVCrv by a ray from UV in -V dir. M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIsPointInsideTrimCrvs, TrimIsPointInsideTrimSrf,		     M
*   MdlIsPointInsideTrimSrf				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimIsPointInsideTrimUVCrv, inclusion	                             M
*****************************************************************************/
int TrimIsPointInsideTrimUVCrv(const CagdCrvStruct *UVCrv,
			       CagdUVType UV)
{
    int i, UVSize,
	NumInters = 0;

    if (UVCrv -> Order > 2) {
        CagdPolylineStruct
	    *UVPoly = TrimCrv2Polyline(UVCrv,
				       _TrimUVCrvApproxTolSamples,
				       _TrimUVCrvApproxMethod, TRUE);
	CagdPolylnStruct
	    *Polyline = UVPoly -> Polyline;

	UVSize = UVPoly -> Length;

	for (i = 1; i < UVSize; i++) {
	    if ((Polyline[i - 1].Pt[0] < UV[0] &&
		 Polyline[i].Pt[0] >= UV[0]) ||
		(Polyline[i - 1].Pt[0] >= UV[0] &&
		 Polyline[i].Pt[0] < UV[0])) {
	        CagdRType
		    t = (UV[0] - Polyline[i].Pt[0]) /
		                (Polyline[i - 1].Pt[0] - Polyline[i].Pt[0]),
		    UV1 = t * Polyline[i - 1].Pt[1] +
			                      (1.0 - t) * Polyline[i].Pt[1];

		if (UV[1] > UV1)
		    NumInters++;
	    }
	}

	CagdPolylineFree(UVPoly);
    }
    else {
        CagdRType
	    * const *Points = UVCrv -> Points;

	UVSize = UVCrv -> Length;

	for (i = 1; i < UVSize; i++) {
	    if ((Points[1][i - 1] < UV[0] && Points[1][i] >= UV[0]) ||
		(Points[1][i - 1] >= UV[0] && Points[1][i] < UV[0])) {
	        CagdRType
		    t = (UV[0] - Points[1][i]) /
			        (Points[1][i - 1] - Points[1][i]),
		    UV1 = t * Points[2][i - 1] +
			         (1.0 - t) * Points[2][i];

		if (UV[1] > UV1)
		    NumInters++;
	    }
	}
    }

    return NumInters;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the trimming curves of the given trimmed surface and returns     M
* TRUE iff the trimmed domain is a sub isoparametric square.  In such a      M
* case the U/VMin/Max are set to the domain of the square.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvList:   Trimming curves to examine.                               M
*   UMin, UMax, VMin, VMax:   Domain of square, if return TRUE               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a isoparametric square domain, FALSE otherwise.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfTrimCrvAllDomain, TrimClipSrfToTrimCrvs                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfTrimCrvSquareDomain                                               M
*****************************************************************************/
CagdBType TrimSrfTrimCrvSquareDomain(const TrimCrvStruct *TrimCrvList,
				     CagdRType *UMin,
				     CagdRType *UMax,
				     CagdRType *VMin,
				     CagdRType *VMax)
{
    const TrimCrvStruct *TrimCrv;

    *UMin = *VMin = IRIT_INFNTY;
    *UMax = *VMax = -IRIT_INFNTY;

    /* Compute the bounding box of the trimming curve's domain. */
    for (TrimCrv = TrimCrvList; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int i;
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;

	    /* Look for conditions that are very unlikely to be found on     */
	    /* on boundary/square trimming curves.			     */
	    if (UVCrv -> Order > 2 ||
		UVCrv -> Length > 5 ||
		CAGD_IS_RATIONAL_CRV(UVCrv))
	        return FALSE;
	    else {
	        CagdRType
		    **UVPts = UVCrv -> Points;

		for (i = 0; i < UVCrv -> Length; i++) {
		    if (*UMin > UVPts[1][i])
		        *UMin = UVPts[1][i];
		    if (*UMax < UVPts[1][i])
		        *UMax = UVPts[1][i];
		    if (*VMin > UVPts[2][i])
		        *VMin = UVPts[2][i];
		    if (*VMax < UVPts[2][i])
		        *VMax = UVPts[2][i];
		}
	    }
	}
    }

    /* Now lets see if all trimming curves are on the bounding volume. */
    for (TrimCrv = TrimCrvList; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int i;
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;
	    CagdRType
	        **UVPts = UVCrv -> Points;
	    CagdUVType PrevUV, MidUV;

	    PrevUV[0] = UVPts[1][UVCrv -> Length - 1];
	    PrevUV[1] = UVPts[2][UVCrv -> Length - 1];
	    for (i = 0; i < UVCrv -> Length; i++) {
	        if (!IRIT_APX_EQ(*UMin, UVPts[1][i]) &&
		    !IRIT_APX_EQ(*UMax, UVPts[1][i]) &&
		    !IRIT_APX_EQ(*VMin, UVPts[2][i]) &&
		    !IRIT_APX_EQ(*VMax, UVPts[2][i]))
		    return FALSE;  /* This point is not on bounding square. */

		/* Compute middle point on line segment and check that too. */
		MidUV[0] = (PrevUV[0] + UVPts[1][i]) * 0.5;
		MidUV[1] = (PrevUV[1] + UVPts[2][i]) * 0.5;
		if (!IRIT_APX_EQ(*UMin, MidUV[0]) &&
		    !IRIT_APX_EQ(*UMax, MidUV[0]) &&
		    !IRIT_APX_EQ(*VMin, MidUV[1]) &&
		    !IRIT_APX_EQ(*VMax, MidUV[1]))
		    return FALSE;   /* Mid point is not on bounding square. */

		PrevUV[0] = UVPts[1][i];
		PrevUV[1] = UVPts[2][i];
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the trimming curves of the given trimmed surface and returns     M
* TRUE iff the valid domain of trimming equals the entire surface domain.    M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   Trimmed surface to examine.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if entire surface domain, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfTrimCrvSquareDomain, TrimClipSrfToTrimCrvs                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfTrimCrvAllDomain                                                  M
*****************************************************************************/
CagdBType TrimSrfTrimCrvAllDomain(const TrimSrfStruct *TrimSrf)
{
    CagdRType UMin, UMax, VMin, VMax;

    if (TrimSrfTrimCrvSquareDomain(TrimSrf -> TrimCrvList,
				   &UMin, &UMax, &VMin, &VMax)) {
	CagdRType SrfUMin, SrfUMax, SrfVMin, SrfVMax;

	TrimSrfDomain(TrimSrf, &SrfUMin, &SrfUMax, &SrfVMin, &SrfVMax);

	return IRIT_APX_EQ(SrfUMin, UMin) &&
	       IRIT_APX_EQ(SrfUMax, UMax) &&
	       IRIT_APX_EQ(SrfVMin, VMin) &&
	       IRIT_APX_EQ(SrfVMax, VMax);
    }
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract the minimal region of the tensor product surface that contains   M
* the domain that is prescribed by the trimming curves.  The return surface  M
* represents the exact same geometry as the input surface but possible with  M
* a small size.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:        Input trimmed surface to extract a minimal valid region  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   Returns a possible smaller surface with similar       M
*		       geometry.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfTrimCrvSquareDomain, TrimSrfTrimCrvAllDomain                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimClipSrfToTrimCrvs                                                    M
*****************************************************************************/
TrimSrfStruct *TrimClipSrfToTrimCrvs(TrimSrfStruct *TrimSrf)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdBBoxStruct BBox;
    CagdCrvStruct
	*TrimCrvs = TrimGetTrimmingCurves(TrimSrf, TRUE, FALSE);
    CagdSrfStruct *Srf;
    TrimSrfStruct *NewTrimSrf;

    CagdCrvListBBox(TrimCrvs, &BBox);

    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);

    /* Do not attempt to extract domain outside the surface domain. */
    if (BBox.Min[0] < UMin)
	BBox.Min[0] = UMin;
    if (BBox.Max[0] > UMax)
	BBox.Max[0] = UMax;

    if (BBox.Min[0] > UMin || BBox.Max[0] < UMax) {
	/* Trimming U domain is smaller than surface - extract region. */
	Srf = CagdSrfRegionFromSrf(TrimSrf -> Srf, BBox.Min[0], BBox.Max[0],
				   CAGD_CONST_U_DIR);
    }
    else
	Srf = CagdSrfCopy(TrimSrf -> Srf);

    /* Do not attempt to extract domain outside the surface domain. */
    if (BBox.Min[1] < VMin)
	BBox.Min[1] = VMin;
    if (BBox.Max[1] > VMax)
	BBox.Max[1] = VMax;

    /* Lets see if we need only a portion of the surface. */
    if (BBox.Min[1] > VMin || BBox.Max[1] < VMax) {
	CagdSrfStruct *TSrf;

	/* Trimming V domain is smaller than surface - extract region. */
	TSrf = CagdSrfRegionFromSrf(Srf, BBox.Min[1], BBox.Max[1],
				    CAGD_CONST_V_DIR);
	CagdSrfFree(Srf);
	Srf = TSrf;
    }

    NewTrimSrf = (TrimSrfStruct *) IritMalloc(sizeof(TrimSrfStruct));
    NewTrimSrf -> TrimCrvList = TrimCrvCopyList(TrimSrf -> TrimCrvList);
    NewTrimSrf -> Srf = Srf;
    NewTrimSrf -> Pnext = NULL;
    NewTrimSrf -> Attr = NULL;

    CagdCrvFreeList(TrimCrvs);

    return NewTrimSrf;
}
