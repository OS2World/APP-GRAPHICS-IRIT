/******************************************************************************
* Trim2Ply.c - Converts a trimmed surface into polygons, adaptively.          *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June. 2001.					      *
******************************************************************************/

#include <math.h>
#include "trim_loc.h"
#include "allocate.h"
#include "iritprsr.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "trim_loc.h"
#include "ip_cnvrt.h"

#define TRIM_EVAL_CLASSIFY_BLEND	1.56668*.24
#define TRIM_BNDRY_EPS			(IRIT_EPS * 10)
#define TRIM_DOMAIN_MIN_TOL		5e-3
#define TRIM_FILTER_SAME_VRTX_EPS	1e-8
#define TRIM_TEST_DISCONT_NRML(Nrml) (Nrml[0] == IRIT_INFNTY ? NULL : Nrml)

#define TRIM_VRTX_ON_BNDRY(V) (IRIT_APX_EQ_EPS((V) -> Coord[0], UMin, IRIT_UEPS) || \
			       IRIT_APX_EQ_EPS((V) -> Coord[0], UMax, IRIT_UEPS) || \
			       IRIT_APX_EQ_EPS((V) -> Coord[1], VMin, IRIT_UEPS) || \
			       IRIT_APX_EQ_EPS((V) -> Coord[1], VMax, IRIT_UEPS))

IRIT_STATIC_DATA int
    GlblComputeNormal = FALSE,
    GlblComputeUV = FALSE;

static int TrimSrfAdapSubdivTrimCurves(const CagdSrfStruct *Srf,
				       VoidPtr AuxSrfData,
				       CagdRType t,
				       CagdSrfDirType Dir,
				       CagdSrfStruct *Srf1,
				       VoidPtr *AuxSrf1Data,
				       CagdSrfStruct *Srf2,
				       VoidPtr *AuxSrf2Data);
static int TrimSrfAdapTrimAllDomain(CagdRType SrfUMin,
				    CagdRType SrfUMax,
				    CagdRType SrfVMin,
				    CagdRType SrfVMax,
				    TrimCrvStruct *TrimCrvs);
static CagdPolygonStruct *TrimSrfAdapPolyGen(const CagdSrfStruct *Srf,
					     CagdSrfPtStruct *SrfPtList,
					     const CagdSrfAdapRectStruct *Rect);
static void TrimRefineTrimLoops(CagdRType SrfUMin,
				CagdRType SrfUMax,
				CagdRType SrfVMin,
				CagdRType SrfVMax,
				TrimCrvStruct *TrimLoops,
				CagdSrfPtStruct *SrfPtList);
static CagdBType TrimExamineNonCornerPoints(CagdCrvStruct **UVCrv,
					    int Index,
					    CagdSrfPtStruct *NonCornerPts,
					    CagdSrfBndryType Bndry);
static CagdPolygonStruct *TrimCagdPllnLoops2CagdPlgns(
					     const CagdSrfStruct *Srf,
					     CagdRType UMin,
					     CagdRType UMax,
					     CagdRType VMin,
					     CagdRType VMax,
					     CagdPolylineStruct *PllnLoops);
static void TrimEvalOneVertexPosition(const CagdSrfStruct *Srf,
				      CagdRType U,
				      CagdRType V,
				      CagdVType Pt);
static CagdPolylineStruct *TrimSubTrimCrv2TrimCrv(CagdPolylineStruct *Loop,
						  CagdPolylineStruct *SubLoop);
static int TrimCutPolygonAtRay(CagdPolylineStruct *Poly,
			       IrtPtType Pt,
			       IrtPtType InterPt,
			       int *HitVertex);


#ifdef DEBUG
static void TrimDbgPrintCagdPolyline(CagdPolylineStruct *Poly);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single trimmed surface to set of polygons	     M
* approximating it.							     M
*    Tolerance is a tolerance control on result, typically related to the    M
* the accuracy of the apporximation. A value of 0.1 is a good rough start.   M
*   NULL is returned in case of an error or use of call back function to get M
* a hold over the created polygons, otherwise list of CagdPolygonStruct.     M
*   This routine looks for C1 discontinuities in the surface and splits it   M
* into C1 continuous patches first.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:          To approximate into triangles.                         M
*   Tolerance:        of approximation - a value that depends on the error   M
*		      function used.					     M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error or if use of  M
*			  call back function to collect the polygons.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2PolygonSetErrFunc, CagdSrfAdap2PolyDefErrFunc,		     M
*   CagdSrf2PolyAdapSetErrFunc, CagdSrf2PolyAdapSetAuxDataFunc		     M
*   CagdSrf2PolyAdapSetPolyGenFunc, CagdSrfAdap2Polygons, TrimSrf2Polygons   M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfAdap2Polygons, polygonization, surface approximation              M
*****************************************************************************/
CagdPolygonStruct *TrimSrfAdap2Polygons(const TrimSrfStruct *TrimSrf,
					CagdRType Tolerance,
					CagdBType ComputeNormals,
					CagdBType ComputeUV)
{
    SymbCrvApproxMethodType OldApproxMethod;
    CagdRType OldApproxTol, CrvTol, UMin, UMax, VMin, VMax;
    CagdSrfAdapAuxDataFuncType OldAuxDatFunc;
    CagdSrfAdapPolyGenFuncType OldPolyGenFunc;
    TrimSrfStruct *TmpTSrf;
    TrimCrvStruct *TrimCrvs;
    CagdPolygonStruct *RetVal;

    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);

    GlblComputeNormal = ComputeNormals;
    GlblComputeUV = ComputeUV;

    /* Convert the trimming curves into piecewise linear. */
    OldApproxMethod = _TrimUVCrvApproxMethod;
    OldApproxTol = _TrimUVCrvApproxTolSamples;

    /* Make sure we do not use too crude of an approximation for the         */
    /* trimming curves - should be reasonably exact to begin with.           */
    CrvTol = IRIT_MIN(IRIT_MIN(UMax - UMin, VMax - VMin) * TRIM_DOMAIN_MIN_TOL,
		 Tolerance);

    TrimSetTrimCrvLinearApprox(CrvTol, SYMB_CRV_APPROX_TOLERANCE);
    TmpTSrf = TrimSrfCopy(TrimSrf);
    TrimPiecewiseLinearTrimmingCurves(TmpTSrf, FALSE);
    TrimCrvs = TrimChainTrimmingCurves2Loops(TmpTSrf -> TrimCrvList);
    TrimSetTrimCrvLinearApprox(OldApproxTol, OldApproxMethod);

    /* Set the call back function to subdivide the trimming curves. */
    OldAuxDatFunc = CagdSrf2PolyAdapSetAuxDataFunc(TrimSrfAdapSubdivTrimCurves);

    /* Set the call back function to get flat domains to convert to polys. */
    OldPolyGenFunc = CagdSrf2PolyAdapSetPolyGenFunc(TrimSrfAdapPolyGen);

    RetVal = CagdSrfAdap2Polygons(TmpTSrf -> Srf, Tolerance, ComputeNormals,
				  FALSE, ComputeUV, TrimCrvs);

    TrimSrfFree(TmpTSrf);

    CagdSrf2PolyAdapSetAuxDataFunc(OldAuxDatFunc);
    CagdSrf2PolyAdapSetPolyGenFunc(OldPolyGenFunc);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if the given trimming domain is the complete surface domain         *
*                                                                            *
* PARAMETERS:                                                                *
*   SrfUMin, SrfUMax, SrfVMin, SrfVMaSrf:  Tensor product surface domain.    *
*   TrimCrvs:   The trimming curve to examine if same as surface boundary.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if entire surface rectangular boundary, FALSE otherwise.   *
*****************************************************************************/
static int TrimSrfAdapTrimAllDomain(CagdRType SrfUMin,
				    CagdRType SrfUMax,
				    CagdRType SrfVMin,
				    CagdRType SrfVMax,
				    TrimCrvStruct *TrimCrvs)
{
    CagdRType DmnUMin, DmnUMax, DmnVMin, DmnVMax;

    if (TrimCrvs == NULL)
	return TRUE;

    if (TrimSrfTrimCrvSquareDomain(TrimCrvs,
				   &DmnUMin, &DmnUMax, &DmnVMin, &DmnVMax)) {
	return (IRIT_APX_EQ_EPS(SrfUMin, DmnUMin, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(SrfUMax, DmnUMax, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(SrfVMin, DmnVMin, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(SrfVMax, DmnVMax, IRIT_UEPS));
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Subdivides the given trimming curves into two groups based upon the      *
* subdivision location of the surface itself.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:           Original surface before the subdivision.                  *
*   AuxSrfData:    Its auxiliary data as given by CagdSrfAdap2Polygons       *
*                  The input trimming curves that are freed on the fly here. *
*   t:             Parameter at which Srf was subdivided.                    *
*   Dir:           Direction at which Srf was subdivided.                    *
*   Srf1:          First surface resulting from subdivision.                 *
*   AuxSrf1Data:   Auxiliary data to update for Srf1 (to hold Srf1's         *
*		   trimming curves).					     *
*   Srf2:          Second surface resulting from subdivision.                *
*   AuxSrf2Data:   Auxiliary data to update for Srf2 (to hold Srf2's         *
*		   trimming curves).					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if successful, FALSE otherwise.                           *
*****************************************************************************/
static int TrimSrfAdapSubdivTrimCurves(const CagdSrfStruct *Srf,
				       VoidPtr AuxSrfData,
				       CagdRType t,
				       CagdSrfDirType Dir,
				       CagdSrfStruct *Srf1,
				       VoidPtr *AuxSrf1Data,
				       CagdSrfStruct *Srf2,
				       VoidPtr *AuxSrf2Data)
{
    CagdRType SrfUMin, SrfUMax, SrfVMin, SrfVMax;
    TrimCrvStruct *TrimCrvs1, *TrimCrvs2,
	*TrimCrvs = (TrimCrvStruct *) AuxSrfData;

    CagdSrfDomain(Srf, &SrfUMin, &SrfUMax, &SrfVMin, &SrfVMax);
    if (TrimSrfAdapTrimAllDomain(SrfUMin, SrfUMax, SrfVMin, SrfVMax,
				 TrimCrvs)) {
        /* Trimming domain is all the domain - no trimming from now on. */
        *AuxSrf1Data = *AuxSrf2Data = NULL;
	TrimCrvFreeList(TrimCrvs);
	return TRUE;
    }

    TrimSrfSubdivTrimmingCrvs(TrimCrvs, t, Dir, &TrimCrvs1, &TrimCrvs2);

    TrimCrvFreeList(TrimCrvs);

    *AuxSrf1Data = (VoidPtr) TrimCrvs1;
    *AuxSrf2Data = (VoidPtr) TrimCrvs2;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function to handle flat surface domains and convert them into  *
* polygons.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:         Surface patch that is flat enough to be approximated as     *
*		 polygons.						     *
*   SrfPtList:   A list of surface points on the rectangular domain of the   *
*		 current patch, due to the subdivision process.              *
*		 Can hold more than the four corners due to neighboring      *
*	         polygons that could be more refined.			     *
*   Rect:        A description of the rectangular parameteric domain that    *
*		 also holds the trimming curves, if has any.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:  List of polygons out of the closed srf pt list;    *
*		 Could be NULL if polygons are call back created.	     *
*****************************************************************************/
static CagdPolygonStruct *TrimSrfAdapPolyGen(const CagdSrfStruct *Srf,
					     CagdSrfPtStruct *SrfPtList,
					     const CagdSrfAdapRectStruct *Rect)
{
    TrimCrvStruct *TrimLoops,
	*TrimCrvs = (TrimCrvStruct *) Rect -> AuxSrfData;

    if (TrimCrvs == NULL) {
	/* All the rectangular domain should be converted into polygons. */
        return CagdSrfAdapRectPolyGen(Srf, SrfPtList, Rect);
    }
    else {
        CagdRType RctUMin, RctUMax, RctVMin, RctVMax;
	CagdSrfPtStruct *SrfPt;
	CagdPolylineStruct *PolyLoops;

	/* Compute the domain of this tensor product flat surface. */
	RctUMin = RctVMin = IRIT_INFNTY;
	RctUMax = RctVMax = -IRIT_INFNTY;
	SrfPt = SrfPtList;
	do {
	    if (RctUMin > SrfPt -> Uv[0])
	        RctUMin = SrfPt -> Uv[0];
	    if (RctUMax < SrfPt -> Uv[0])
	        RctUMax = SrfPt -> Uv[0];
	    if (RctVMin > SrfPt -> Uv[1])
	        RctVMin = SrfPt -> Uv[1];
	    if (RctVMax < SrfPt -> Uv[1])
	        RctVMax = SrfPt -> Uv[1];

	    SrfPt = SrfPt -> Pnext;
	}
	while (SrfPt != SrfPtList && SrfPt != NULL);

	if (TrimSrfAdapTrimAllDomain(RctUMin, RctUMax, RctVMin, RctVMax,
				     TrimCrvs)) {
	    TrimCrvFreeList(TrimCrvs);

	    /* All the rectangular domain should be converted into polygons. */
	    return CagdSrfAdapRectPolyGen(Srf, SrfPtList, Rect);
	}

	/* Chain the trimming curves into closed loops. */
	TrimLoops = TrimChainTrimmingCurves2Loops(TrimCrvs);
	TrimCrvFreeList(TrimCrvs);

	/* Refine the loops with non corner boundary points. */
	TrimRefineTrimLoops(RctUMin, RctUMax, RctVMin, RctVMax,
			    TrimLoops, SrfPtList);

	/* Classify into a hierarchy of loop islands. */
	TrimClassifyTrimmingLoops(&TrimLoops);
	PolyLoops = TrimCrvsHierarchy2Polys(TrimLoops);

	return TrimCagdPllnLoops2CagdPlgns(Srf,
					   RctUMin, RctUMax, RctVMin, RctVMax,
					   PolyLoops);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Refine the trimming loops that are on the boundary to contain non-corner *
* boundary points listing in the Rect structure, due to different, adaptive, *
* subdivision of the surface, closing "black holes".			     *
*                                                                            *
* PARAMETERS:                                                                *
*   RctUMin, RctUMax, RctVMin, RctVMax:  Boundary of rectangular domain.     *
*   TrimLoops:   The list of loops to examine.                               *
*   SrfPtList:   A list of surface points on the rectangular domain of the   *
*		 current patch, due to the subdivision process.              *
*		 Can hold more than the four corners due to neighboring      *
*	         polygons that could be more refined.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrimRefineTrimLoops(CagdRType RctUMin,
				CagdRType RctUMax,
				CagdRType RctVMin,
				CagdRType RctVMax,
				TrimCrvStruct *TrimLoops,
				CagdSrfPtStruct *SrfPtList)
{
    CagdSrfPtStruct *Pt,
	*SrfPtListOrig = SrfPtList,
	*SrfPtUMin = NULL,
	*SrfPtUMax = NULL,
	*SrfPtVMin = NULL,
	*SrfPtVMax = NULL;

    /* Make a copy of this SrfPtList circular list as a linear list. */
    Pt = SrfPtList -> Pnext;
    SrfPtList -> Pnext = NULL;
    SrfPtList = CagdSrfPtCopyList(Pt);
    SrfPtListOrig -> Pnext = Pt;

    /* Classify the non corner points of the boundary, if any. */
    while (SrfPtList != NULL) {
	Pt = SrfPtList;
	SrfPtList = SrfPtList -> Pnext;
	Pt -> Pnext = NULL;

	if (IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMin, IRIT_UEPS) &&
	    !IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMin, IRIT_UEPS) &&
	    !IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMax, IRIT_UEPS)) {
	    IRIT_LIST_PUSH(Pt, SrfPtUMin);
	}
	else if (IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMax, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMin, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMax, IRIT_UEPS)) {
	    IRIT_LIST_PUSH(Pt, SrfPtUMax);
	}
	else if (IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMin, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMin, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMax, IRIT_UEPS)) {
	    IRIT_LIST_PUSH(Pt, SrfPtVMin);
	}
	else if (IRIT_APX_EQ_EPS(Pt -> Uv[1], RctVMax, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMin, IRIT_UEPS) &&
		 !IRIT_APX_EQ_EPS(Pt -> Uv[0], RctUMax, IRIT_UEPS)) {
	    IRIT_LIST_PUSH(Pt, SrfPtVMax);
	}
	else 
	    CagdSrfPtFree(Pt);
    }

    /* If no non-corner points in this boundary, return. */
    if (SrfPtUMin == NULL && SrfPtUMax == NULL &&
	SrfPtVMin == NULL && SrfPtVMax == NULL)
	return;

    /* Scan all the trimming loops and add the non-corner points. */
    for ( ; TrimLoops != NULL; TrimLoops = TrimLoops -> Pnext) {
	TrimCrvSegStruct *TrimCrvSeg;

	for (TrimCrvSeg = TrimLoops -> TrimCrvSegList;
	     TrimCrvSeg != NULL;
	     TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    CagdBType
		UVCrvUpdated = FALSE;
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;

	    do {
	        CagdRType
		    **Points = UVCrv -> Points,
		    *UPts = Points[1],
		    *VPts = Points[2];

		UVCrvUpdated = FALSE;

		if (UVCrv -> Order == 2) {
		    int i;

		    for (i = 0; i < UVCrv -> Length - 1; i++) {
		        /* Look for a segment [i, i+1] that is on boundary. */
		        if (SrfPtUMin != NULL &&
			    IRIT_APX_EQ_EPS(UPts[i], RctUMin, IRIT_UEPS) &&
			    IRIT_APX_EQ_EPS(UPts[i + 1], RctUMin, IRIT_UEPS) &&
			    TrimExamineNonCornerPoints(&UVCrv, i, SrfPtUMin,
						       CAGD_U_MIN_BNDRY)) {
			    UVCrvUpdated = TRUE;
			    break;
			}
			else if (SrfPtUMax != NULL &&
				 IRIT_APX_EQ_EPS(UPts[i], RctUMax, IRIT_UEPS) &&
				 IRIT_APX_EQ_EPS(UPts[i + 1], RctUMax, IRIT_UEPS) &&
				 TrimExamineNonCornerPoints(&UVCrv, i,
						SrfPtUMax, CAGD_U_MAX_BNDRY)) {
			    UVCrvUpdated = TRUE;
			    break;
			}
			else if (SrfPtVMin != NULL &&
				 IRIT_APX_EQ_EPS(VPts[i], RctVMin, IRIT_UEPS) &&
				 IRIT_APX_EQ_EPS(VPts[i + 1], RctVMin, IRIT_UEPS) &&
				 TrimExamineNonCornerPoints(&UVCrv, i,
						SrfPtVMin, CAGD_V_MIN_BNDRY)) {
			    UVCrvUpdated = TRUE;
			    break;
			}
			else if (SrfPtVMax != NULL &&
				 IRIT_APX_EQ_EPS(VPts[i], RctVMax, IRIT_UEPS) &&
				 IRIT_APX_EQ_EPS(VPts[i + 1], RctVMax, IRIT_UEPS) &&
				 TrimExamineNonCornerPoints(&UVCrv, i,
						SrfPtVMax, CAGD_V_MAX_BNDRY)) {
			    UVCrvUpdated = TRUE;
			    break;
			}
		    }
		}
		else {
		    TRIM_FATAL_ERROR(TRIM_ERR_LINEAR_TRIM_EXPECT);
		}

		if (UVCrvUpdated) {
#		    ifdef DEBUG
		    {
		        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintRefinedCurve,
						       FALSE) {
			    IRIT_INFO_MSG("Before point insertion:\n");
			    CagdDbg(TrimCrvSeg -> UVCrv);
			    IRIT_INFO_MSG("After point insertion:\n");
			    CagdDbg(UVCrv);
			}
		    }
#		    endif /* DEBUG */

		    CagdCrvFree(TrimCrvSeg -> UVCrv);
		    TrimCrvSeg -> UVCrv = UVCrv;
		}
	    }
	    while (UVCrvUpdated);
	}
    }

    CagdSrfPtFreeList(SrfPtUMin);
    CagdSrfPtFreeList(SrfPtUMax);
    CagdSrfPtFreeList(SrfPtVMin);
    CagdSrfPtFreeList(SrfPtVMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine the prescribed Index'th segment of the trimming curve UVCrv and  *
* add non corner points as prescribed by the list in NonCornerPts.           *
*                                                                            *
* PARAMETERS:                                                                *
*   UVCrv:        The trrimming curve to consider.                           *
*   Index:        of the segment in UVCrv to consider as [Index, Index+1].   *
*   NonCornerPts: A list of non corner points along the given boundary.      *
*   Bndry:        The boundary the UVCrv segment and NonCornerPts are on.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:    TRUE if UVCrv was updated in place, FALSE if unchanged.    *
*****************************************************************************/
static CagdBType TrimExamineNonCornerPoints(CagdCrvStruct **UVCrv,
					    int Index,
					    CagdSrfPtStruct *NonCornerPts,
					    CagdSrfBndryType Bndry)
{
    CagdBType
	UVCrvUpdated = FALSE;
    CagdRType u1, u2, v1, v2,
	**Points = (*UVCrv) -> Points,
	*UPts = Points[1],
	*VPts = Points[2];
    CagdPType Uv;
    CagdSrfPtStruct *Pts;

    Uv[2] = 0.0;

    switch (Bndry) {
	case CAGD_U_MIN_BNDRY:
	case CAGD_U_MAX_BNDRY:
	    v1 = VPts[Index];
	    v2 = VPts[Index + 1];
	    if (v2 < v1)
		IRIT_SWAP(CagdRType, v1, v2);

	    /* Scan all non corner points and look for pts between v1 & v2. */
	    for (Pts = NonCornerPts; Pts != NULL; Pts = Pts -> Pnext) {
		if (Pts -> Uv[1] > v1 && Pts -> Uv[1] < v2) {
		    /* Insert Pts into UVCrv as a new point at Index+1. */
		    IRIT_UV_COPY(Uv, Pts -> Uv);
		    *UVCrv = CagdCrvInsertPoint(*UVCrv, Index + 1, Uv);
		    UVCrvUpdated = TRUE;
		    break;
		}
	    }
	    break;
	case CAGD_V_MIN_BNDRY:
	case CAGD_V_MAX_BNDRY:
	    u1 = UPts[Index];
	    u2 = UPts[Index + 1];
	    if (u2 < u1)
		IRIT_SWAP(CagdRType, u1, u2);

	    /* Scan all non corner points and look for pts between u1 & u2. */
	    for (Pts = NonCornerPts; Pts != NULL; Pts = Pts -> Pnext) {
		if (Pts -> Uv[0] > u1 && Pts -> Uv[0] < u2) {
		    /* Insert Pts into UVCrv as a new point at Index+1. */
		    IRIT_UV_COPY(Uv, Pts -> Uv);
		    *UVCrv = CagdCrvInsertPoint(*UVCrv, Index + 1, Uv);
		    UVCrvUpdated = TRUE;
		    break;
		}
	    }
	    break;
	default:
	    break;
    }

    return UVCrvUpdated;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts closed polyline loops into triangles.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:         Surface patch that is flat enough to be approximated as     *
*		 polygons.						     *
*   UMin, UMax, VMin, VMax:       Surface patch boundary.                    *
*   PllnLoops: List of polylines, each a loop, to convert. Freed at the end! *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:  Triangles representing the geometry in the loops.  *
*****************************************************************************/
static CagdPolygonStruct *TrimCagdPllnLoops2CagdPlgns(
					     const CagdSrfStruct *Srf,
					     CagdRType UMin,
					     CagdRType UMax,
					     CagdRType VMin,
					     CagdRType VMax,
					     CagdPolylineStruct *PllnLoops)
{
#ifdef TRIM_2_POLY_SHIFT_VERTICES_IN
    CagdBType PrevOnBndry, OnBndry, NextOnBndry, NextNextOnBndry;
    IPVertexStruct *V, *VHead;
#endif /* TRIM_2_POLY_SHIFT_VERTICES_IN */
    CagdBType 
	OldCirc = IPSetPolyListCirc(TRUE);
    CagdPolygonStruct
	*RetVal = NULL;
    IPPolygonStruct *Pl, *Pls;
    IPObjectStruct *PTmp2,
	*PTmp1 = IPGenPOLYObject(IPCagdPllns2IritPllns(PllnLoops));

    /* Fixup the polygons - set plane equation, close them, and clean up. */
    for (Pl = PTmp1 -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        Pl -> Plane[0] = Pl -> Plane[1] = Pl -> Plane[3] = 0;
        Pl -> Plane[2] = 1;
	IP_SET_PLANE_POLY(Pl);
    }

    IPOpenPolysToClosed(PTmp1 -> U.Pl);

    GMCleanUpPolygonList(&PTmp1 -> U.Pl, TRIM_FILTER_SAME_VRTX_EPS);

    if (PTmp1 -> U.Pl == NULL) {
	IPFreeObject(PTmp1);
	IPSetPolyListCirc(OldCirc);

	return NULL;
    }

#ifdef TRIM_2_POLY_SHIFT_VERTICES_IN
    /* Make sure no interior point is on the boundary. */
    VHead = PTmp1 -> U.Pl -> PVertex;
    V = IPGetLastVrtx(VHead);
    OnBndry = TRIM_VRTX_ON_BNDRY(V);
    V = VHead;
    NextOnBndry = TRIM_VRTX_ON_BNDRY(V);
    NextNextOnBndry = TRIM_VRTX_ON_BNDRY(V -> Pnext);
    do {
        PrevOnBndry = OnBndry;
	OnBndry = NextOnBndry;
	NextOnBndry = NextNextOnBndry;
	NextNextOnBndry = TRIM_VRTX_ON_BNDRY(V -> Pnext -> Pnext);

	if (!PrevOnBndry && OnBndry && (!NextOnBndry || !NextNextOnBndry)) {
	    /* Move this vertex back from the boundary. */
	    if (IRIT_APX_EQ_EPS(V -> Coord[0], UMin, IRIT_UEPS))
	        V -> Coord[0] += TRIM_BNDRY_EPS;
	    else if (IRIT_APX_EQ_EPS(V -> Coord[0], UMax, IRIT_UEPS))
	        V -> Coord[0] -= TRIM_BNDRY_EPS;
	    if (IRIT_APX_EQ_EPS(V -> Coord[1], VMin, IRIT_UEPS))
	        V -> Coord[1] += TRIM_BNDRY_EPS;
	    else if (IRIT_APX_EQ_EPS(V -> Coord[1], VMax, IRIT_UEPS))
	        V -> Coord[1] -= TRIM_BNDRY_EPS;
	}

        V = V -> Pnext;
    }
    while (V != VHead);
#endif /* TRIM_2_POLY_SHIFT_VERTICES_IN */

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintTrimLoops1, FALSE) {
	    IRIT_INFO_MSG("Final trimming loops:\n");
	    IPStderrObject(PTmp1);
	}
    }
#   endif /* DEBUG */

    /* Convert the set of general loop polygons to triangles only. */
    PTmp2 = GMConvertPolysToTriangles2(PTmp1);
    IPFreeObject(PTmp1);
    Pls = PTmp2 -> U.Pl;

    /* Convert to cagd type triangles. */
    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	CagdBType GenPoly;
	IPVertexStruct
	    *V1 = Pl -> PVertex,
	    *V2 = V1 -> Pnext,
	    *V3 = V2 -> Pnext;
	CagdPType Pt1, Pt2, Pt3;
	CagdVType Nl1, Nl2, Nl3;
	CagdUVType UV1, UV2, UV3;
	CagdPolygonStruct *Tri;

	/* Make sure the trimming curves at at least within the srf domain. */
	V1 -> Coord[0] = IRIT_BOUND(V1 -> Coord[0], UMin, UMax);
	V1 -> Coord[1] = IRIT_BOUND(V1 -> Coord[1], VMin, VMax);
	V2 -> Coord[0] = IRIT_BOUND(V2 -> Coord[0], UMin, UMax);
	V2 -> Coord[1] = IRIT_BOUND(V2 -> Coord[1], VMin, VMax);
	V3 -> Coord[0] = IRIT_BOUND(V3 -> Coord[0], UMin, UMax);
	V3 -> Coord[1] = IRIT_BOUND(V3 -> Coord[1], VMin, VMax);

	TrimEvalOneVertexPosition(Srf, V1 -> Coord[0], V1 -> Coord[1], Pt1);
	TrimEvalOneVertexPosition(Srf, V2 -> Coord[0], V2 -> Coord[1], Pt2);
	TrimEvalOneVertexPosition(Srf, V3 -> Coord[0], V3 -> Coord[1], Pt3);

	if (GlblComputeNormal) {
	    CagdRType *N;

	    /* We move the UV location a tad toward the interior of the     */
	    /* triangle so we will not be at singularities/discontinuities. */
	    N = CagdSrfAdap2PolyEvalNrmlBlendedUV(V1 -> Coord,
						  V2 -> Coord,
						  V3 -> Coord);
	    IRIT_VEC_COPY(Nl1, N);

	    N = CagdSrfAdap2PolyEvalNrmlBlendedUV(V2 -> Coord,
						  V3 -> Coord,
						  V1 -> Coord);
	    IRIT_VEC_COPY(Nl2, N);

	    N = CagdSrfAdap2PolyEvalNrmlBlendedUV(V3 -> Coord,
						  V1 -> Coord,
						  V2 -> Coord);
	    IRIT_VEC_COPY(Nl3, N);
	}

	if (GlblComputeUV) {
	    UV1[0] = V1 -> Coord[0];
	    UV1[1] = V1 -> Coord[1];
	    UV2[0] = V2 -> Coord[0];
	    UV2[1] = V2 -> Coord[1];
	    UV3[0] = V3 -> Coord[0];
	    UV3[1] = V3 -> Coord[1];
	}

	
	if ((Tri = _CagdSrfMakeTriFunc(GlblComputeNormal, GlblComputeUV,
				       Pt1, Pt2, Pt3,
				       TRIM_TEST_DISCONT_NRML(Nl1),
				       TRIM_TEST_DISCONT_NRML(Nl2),
				       TRIM_TEST_DISCONT_NRML(Nl3),
				       UV1, UV2, UV3, &GenPoly)) != NULL)
	    IRIT_LIST_PUSH(Tri, RetVal);
    }

    IPFreeObject(PTmp2);
    IPSetPolyListCirc(OldCirc);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates position for a given (U, V) location.      		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:          Surface to evaluate position and normal for.               *
*   U, V:         Domain coordinate to evaluate Srf at.			     *
*   Pt:           Evaluated position.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrimEvalOneVertexPosition(const CagdSrfStruct *Srf,
				      CagdRType U,
				      CagdRType V,
				      CagdVType Pt)
{
    CagdRType
        *R = CagdSrfEval(Srf, U, V);

    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a hierarchy of trimming curves/loops into closed, simple,       M
* polygons.  The input trimming curves are destroyed by this function.       M
    A trimming curve inside another trimming curve are chained into one by   M
* adding a bidirection line segment between the two curves.		     M
*   A trimming loop will have its contained trimming loops in an attribute   M
* "_subTrims".								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimLoops:  A linked list of trimming loops hierarchy (a 'forrest').     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:    Piecewise linear polylines approximating the    M
*			     given trimming curves hierarcy.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimClassifyTrimmingLoops                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimCrvsHierarchy2Polys, polygonization, surface approximation           M
*****************************************************************************/
CagdPolylineStruct *TrimCrvsHierarchy2Polys(TrimCrvStruct *TrimLoops)
{
    TrimCrvStruct *TrimLoop, *TrimSubLoops;
    CagdPolylineStruct
	*PolyLoops = NULL;

    while (TrimLoops != NULL) {
	CagdCrvStruct *CrvLoop;
	CagdPolylineStruct *PolyLoop;

	TrimLoop = TrimLoops;
	TrimLoops = TrimLoops -> Pnext;
	TrimLoop -> Pnext = NULL;
	 
	CrvLoop = TrimGetTrimmingCurves2(TrimLoop, NULL, TRUE, FALSE);
	PolyLoop = CagdCnvrtLinBspCrv2Polyline(CrvLoop);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintTrimLoops2, FALSE) {
	    IRIT_INFO_MSG("PolyLoop linear trimming curve:\n");
	    CagdDbg(CrvLoop);
	}
    }
#   endif /* DEBUG */

	CagdCrvFreeList(CrvLoop);

	if ((TrimSubLoops =
	     (TrimCrvStruct *) AttrGetPtrAttrib(TrimLoop -> Attr,
						"_subTrims")) != NULL) {
	    CagdPolylineStruct *PolySubLoops, *PolyNewLoop, *Pl;

	    PolySubLoops = TrimCrvsHierarchy2Polys(TrimSubLoops);

	    /* Compute the maximum X value for each interior sub loop. */
	    for (Pl = PolySubLoops; Pl != NULL; Pl = Pl -> Pnext) {
	        int i,
		    PtMaxXIndx = 0;

		for (i = 1; i < Pl -> Length; i++) {
		    if (Pl -> Polyline[i].Pt[0] >
			Pl -> Polyline[PtMaxXIndx].Pt[0])
		        PtMaxXIndx = i;
		}
		AttrSetIntAttrib(&Pl -> Attr, "_MaxXIndx", PtMaxXIndx);
	    }

	    /* Connect the inner sub loops to the outer loop, starting with */
	    /* the loop with the maximal X value.			    */
	    while (PolySubLoops) {
	        int PtMaxXIndx = AttrGetIntAttrib(PolySubLoops -> Attr,
						  "_MaxXIndx");
		IrtRType
		    LoopMaxXVal = PolySubLoops -> Polyline[PtMaxXIndx].Pt[0];
		CagdPolylineStruct
		    *LoopMaxX = PolySubLoops;

	        for (Pl = PolySubLoops -> Pnext;
		     Pl != NULL;
		     Pl = Pl -> Pnext) {
		    int PtMaxXIndx2 = AttrGetIntAttrib(Pl -> Attr,
						       "_MaxXIndx");
		    IrtRType
		        LoopMaxXVal2 = Pl -> Polyline[PtMaxXIndx2].Pt[0];

		    if (LoopMaxXVal < LoopMaxXVal2) {
		        LoopMaxXVal = LoopMaxXVal2;
			LoopMaxX = Pl;
		    }
		}

		/* Remove the sub loop with maximal X from the list and      */
		/* connect it with the outer loop.			     */
		if (LoopMaxX == PolySubLoops) {
		    PolySubLoops = PolySubLoops -> Pnext;
		}
		else {
		    for (Pl = PolySubLoops;
			 Pl -> Pnext != LoopMaxX;
			 Pl = Pl -> Pnext);
		    Pl -> Pnext = LoopMaxX -> Pnext;
		}
		LoopMaxX -> Pnext = NULL;

		PolyNewLoop = TrimSubTrimCrv2TrimCrv(PolyLoop, LoopMaxX);
		CagdPolylineFree(PolyLoop);
		CagdPolylineFree(LoopMaxX);
		PolyLoop = PolyNewLoop;
	    }
	}

	IRIT_LIST_PUSH(PolyLoop, PolyLoops);
	TrimCrvFree(TrimLoop);
    }

    return PolyLoops;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create a new polyline holding a closed loop that contains both Loop and  *
* SubLoop by adding a linear segment between the two loops.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Loop:    Outer loop to combine.                                          *
*   SubLoop: Inner loop to combine. Order of vertices is reversed, in place. *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolylineStruct *:  Combined loop.                                    *
*****************************************************************************/
static CagdPolylineStruct *TrimSubTrimCrv2TrimCrv(CagdPolylineStruct *Loop,
						  CagdPolylineStruct *SubLoop)
{
    int i, SubLoopMaxXIndx, LoopIndx, HitVertex;
    CagdRType *SubLoopMaxPt;
    CagdPType InterPt;
    CagdPolylineStruct *MergedLoop;

    /* Find the maximum X value point of SubLoop. */
    SubLoopMaxXIndx = 0;
    for (i = 1; i < SubLoop -> Length; i++) {
	if (SubLoop -> Polyline[i].Pt[0] >
	    SubLoop -> Polyline[SubLoopMaxXIndx].Pt[0])
	    SubLoopMaxXIndx = i;
    }
    SubLoopMaxPt = SubLoop -> Polyline[SubLoopMaxXIndx].Pt;

    if ((LoopIndx = TrimCutPolygonAtRay(Loop, SubLoopMaxPt,
					InterPt, &HitVertex)) < 0) {
	/* A classification error - purge the subloop. */
	return CagdPolylineCopy(Loop);
    }

    /* Create a new, combined polyline. */
    MergedLoop = 
        CagdPolylineNew(Loop -> Length + SubLoop -> Length +
			(HitVertex ? 1 : 2));         /* Loop new vertices. */

    /* Copy the first portion of Loop upto and including LoopIndx. */
    IRIT_GEN_COPY(&MergedLoop -> Polyline[0], &Loop -> Polyline[0],
	     sizeof(CagdPolylnStruct) * (LoopIndx + 1));
    i = LoopIndx + 1;
    if (!HitVertex) {
	IRIT_PT_COPY(&MergedLoop -> Polyline[i].Pt, InterPt);
	i++;
    }

    /* Copy SubLoop, in two parts, to MergedLoop. */
    IRIT_GEN_COPY(&MergedLoop -> Polyline[i],
	     &SubLoop -> Polyline[SubLoopMaxXIndx],
	     sizeof(CagdPolylnStruct) * (SubLoop -> Length - SubLoopMaxXIndx));
    i += SubLoop -> Length - SubLoopMaxXIndx;

    if (SubLoopMaxXIndx > 0) {
        IRIT_GEN_COPY(&MergedLoop -> Polyline[i], &SubLoop -> Polyline[1],
		      sizeof(CagdPolylnStruct) * SubLoopMaxXIndx);
	i += SubLoopMaxXIndx;
    }

    /* Copy the second portion of Loop from and including LoopIndx. */
    if (!HitVertex) {
	IRIT_PT_COPY(&MergedLoop -> Polyline[i].Pt, InterPt);
	i++;
    }
    IRIT_GEN_COPY(&MergedLoop -> Polyline[i],
		  &Loop -> Polyline[LoopIndx + !HitVertex],
		  sizeof(CagdPolylnStruct) * 
				    (Loop -> Length - LoopIndx - !HitVertex));

    return MergedLoop;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the intersection of the ray fired from Pt to +X direction with the *
* given closed polyline. Pt MUST be in the polygon. 			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:      The polygon to compute the ray intersection with.             *
*   Pt:        The origin of the ray point.                                  *
*   InterPt:   The intersection point on the edge thet ray cuts first.       *
*   HitVertex: Returns TRUE if hit a vertex on outer loop, FALSE if cut edge.*
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    Index of point in Pl whose edge (Idx, Idx+1) is the first cut,   *
*           or index of the vertex we cut if hit a vertex,		     *
*           or -1 in case of classification error.			     *
*****************************************************************************/
static int TrimCutPolygonAtRay(CagdPolylineStruct *Poly,
			       IrtPtType Pt,
			       IrtPtType InterPt,
			       int *HitVertex)
{
    int i,
	MinIdx = -1;
    IrtRType X,
	MinX = IRIT_INFNTY;
    CagdPolylnStruct
	*Pl = Poly -> Polyline;

    *HitVertex = FALSE;

    for (i = 0; i < Poly -> Length - 1; i++) {
        CagdRType
	    *V1 = Pl[i].Pt,
	    *V2 = Pl[i + 1].Pt;

	/* A polygon edge might intersect the ray iff one of the following:  */
	/* 1. The first vertex is exactly on the ray Y level. (if this is    */
	/*    true for the second edge, it will be detected next iteration). */
	/* 2. The first vertex is below ray Y level, and second is above.    */
	/* 3. The first vertex is above ray Y level, and second is below.    */
	if (IRIT_APX_EQ_EPS(V1[1], Pt[1], IRIT_UEPS)) {	    /* Case 1 above. */
	    if (MinX > V1[0] && Pt[0] < V1[0]) {
		*HitVertex = TRUE;
		MinX = V1[0];
		MinIdx = i;
	    }
	}
	else if ((V1[1] < Pt[1] && V2[1] > Pt[1]) ||		  /* Case 2. */
		 (V1[1] > Pt[1] && V2[1] < Pt[1])) {		  /* Case 3. */
	    X = ((V2[1] - Pt[1]) * V1[0] + (Pt[1] - V1[1]) * V2[0]) /
						      (V2[1] - V1[1]);
	    if (MinX > X && Pt[0] < X) {
		*HitVertex = FALSE;
		MinX = X;
		MinIdx = i;
	    }

	}
    }

    if (MinIdx < 0)
	return -1;

    /* Now that we have the intersection point - return it. */
    InterPt[0] = MinX;
    InterPt[1] = Pt[1];
    InterPt[2] = 0;

    return MinIdx;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Classify the given trimming curve loops into a hierarchy.                M
*   All curves with even nesting levels are considered outside loops and     M
* are oriented clockwise.  All curves with odd nesting level are considered  M
* islands and are oriented counterclockwise.				     M
*   An island Ci of outside loop Cj will be placed in an "_subTrims"         M
* attribute under Ci.  						             M
*   Assumes all trimming curves are full loops with a single segment.        M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimLoops:    Input loops.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrvsHierarchy2Polys, TrimClassifyTrimCurveOrient                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimClassifyTrimmingLoops                                                M
*****************************************************************************/
int TrimClassifyTrimmingLoops(TrimCrvStruct **TrimLoops)
{
    int i, j, l, MaxIncLvl, NumOfLoops, **LoopInsideArray, *LoopIncLevel;
    TrimCrvStruct *Loop, **LoopVec, *SubLoops;

    /* Handle the trivial case of only one trimming loop. */
    if (*TrimLoops == NULL || (*TrimLoops) -> Pnext == NULL)
	return TRUE;

    NumOfLoops = CagdListLength(*TrimLoops);
    LoopVec = (TrimCrvStruct **)
                             IritMalloc(sizeof(TrimCrvStruct *) * NumOfLoops);
    LoopIncLevel = IritMalloc(sizeof(int) * NumOfLoops);
    LoopInsideArray = (int **) IritMalloc(sizeof(int *) * NumOfLoops);
    for (i = 0, Loop = *TrimLoops; i < NumOfLoops; i++, Loop = Loop -> Pnext) {
        LoopVec[i] = Loop;
	LoopInsideArray[i] = (int *) IritMalloc(sizeof(int) * NumOfLoops);
    }
    for (i = 0; i < NumOfLoops; i++) {
	if (LoopVec[i] -> TrimCrvSegList -> Pnext != NULL) {
	    for (i = 0; i < NumOfLoops; i++)
	        IritFree(LoopInsideArray[i]);
	    IritFree(LoopInsideArray);
	    IritFree(LoopVec);
	    IritFree(LoopIncLevel);
	    TRIM_FATAL_ERROR(TRIM_ERR_INVALID_TRIM_SEG);
	    return FALSE;
	}

        LoopVec[i] -> Pnext = NULL;
    }

    /* Check if Loop j is contained in Loop i. */
    for (i = 0; i < NumOfLoops; i++)
	LoopIncLevel[i] = 0;
    for (i = 0; i < NumOfLoops; i++) {
        for (j = 0; j < NumOfLoops; j++) {
	    CagdRType *R, TMin, TMax;
	    CagdUVType UV;
	    CagdCrvStruct
		*UVCrv = LoopVec[j] -> TrimCrvSegList -> UVCrv;

	    CagdCrvDomain(UVCrv, &TMin, &TMax);
	    R = CagdCrvEval(UVCrv, TMin * TRIM_EVAL_CLASSIFY_BLEND +
			           TMax * (1.0 - TRIM_EVAL_CLASSIFY_BLEND));
	    CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);

	    LoopInsideArray[i][j] =
		i == j ? 0 : TrimIsPointInsideTrimCrvs(LoopVec[i], UV);
	    LoopIncLevel[j] += LoopInsideArray[i][j];
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugClassifyTrimCrvsPrint, FALSE) {
	    for (i = 0; i < NumOfLoops; i++) {
	        IRIT_INFO_MSG_PRINTF(
			"****************** LOOP %d ******************\n", i);
		TrimDbgPrintTrimCurves(LoopVec[i]);
	    }

	    IRIT_INFO_MSG("       ");
	    for (i = 0; i < NumOfLoops; i++)
	        IRIT_INFO_MSG_PRINTF(" %1d ", i);
	    IRIT_INFO_MSG("\n\n");

	    for (i = 0; i < NumOfLoops; i++) {
	        IRIT_INFO_MSG_PRINTF("    %1d) ", i);

		for (j = 0; j < NumOfLoops; j++)
		    IRIT_INFO_MSG_PRINTF(" %1d ", LoopInsideArray[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	    IRIT_INFO_MSG("\nIncLvl");
	    for (j = 0; j < NumOfLoops; j++)
	        IRIT_INFO_MSG_PRINTF(" %2d", LoopIncLevel[j]);
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    /* Construct the hierarchy of trim loops containment as a "forrest" via  */
    /* the use of "_subTrims" attribute.  Each such attribute may hold a     */
    /* list of TrimCrvStruct objects, and so on recursively.                 */
    while (TRUE) {
	/* Find an odd-inclusion-level loop that contains no other loop. */
        for (i = 0; i < NumOfLoops; i++) {
	    if ((LoopIncLevel[i] & 0x01) > 0)
	        break;
	}
	if (i >= NumOfLoops)
	    break;    /* No more odd-inclusion-level loops in the hierarchy. */

	/* Push detected loop into its even-inclusion-level containing loop. */
	LoopIncLevel[i] = -2;        /* Mark as non-odd and out of the game. */
	l = MaxIncLvl = -1;

	for (j = 0; j < NumOfLoops; j++) {
	    if (LoopIncLevel[j] >= 0 && (LoopIncLevel[j] & 0x01) == 0 &&
		LoopInsideArray[j][i] && MaxIncLvl < LoopIncLevel[j]) {
	        MaxIncLvl = LoopIncLevel[j];
		l = j;
	    }
	}
	if (l < 0) {
	    for (i = 0; i < NumOfLoops; i++)
	        IritFree(LoopInsideArray[i]);
	    IritFree(LoopInsideArray);
	    IritFree(LoopVec);
	    IritFree(LoopIncLevel);
	    TRIM_FATAL_ERROR(TRIM_ERR_INVALID_TRIM_SEG);
	    return FALSE;
	}

	if (TrimClassifyTrimCurveOrient(LoopVec[i] -> TrimCrvSegList
								-> UVCrv)) {
	    CagdCrvStruct
	        *Crv = LoopVec[i] -> TrimCrvSegList -> UVCrv;

	    LoopVec[i] -> TrimCrvSegList -> UVCrv = CagdCrvReverse(Crv);
	    CagdCrvFree(Crv);
	}

	/* Place the new sub curve into attribute list of container curve. */
	if ((SubLoops = (TrimCrvStruct *) AttrGetPtrAttrib(LoopVec[l] -> Attr,
						       "_subTrims")) != NULL) {
	    LoopVec[i] -> Pnext = SubLoops;
	}
	AttrSetPtrAttrib(&LoopVec[l] -> Attr, "_subTrims", LoopVec[i]);
    }

    /* Place all remaining positive and even trimming curve in output list. */
    *TrimLoops = NULL;
    for (j = 0; j < NumOfLoops; j++) {
	if (LoopIncLevel[j] >= 0 && (LoopIncLevel[j] & 0x01) == 0) {
	    if (!TrimClassifyTrimCurveOrient(LoopVec[j] -> TrimCrvSegList
					                         -> UVCrv)) {
	        CagdCrvStruct
		    *Crv = LoopVec[j] -> TrimCrvSegList -> UVCrv;

		LoopVec[j] -> TrimCrvSegList -> UVCrv = CagdCrvReverse(Crv);
		CagdCrvFree(Crv);
	    }

	    IRIT_LIST_PUSH(LoopVec[j], *TrimLoops);
	}
    }

    /* Time to release all aux data structures. */
    for (i = 0; i < NumOfLoops; i++)
	IritFree(LoopInsideArray[i]);
    IritFree(LoopInsideArray);
    IritFree(LoopVec);
    IritFree(LoopIncLevel);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a closed, piecewise linear trimming curve, returns TRUE if the     M
* curve is clockwise, FALSE if counter clockwise.                            M
*   Orientation is determined by computing the signed area of the polygon    M
* and examining the sign of the result.				             M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrv:   Trimming curve to examine its orientation.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if the curve is clockwise, FALSE if counter clockwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimClassifyTrimmingLoops                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimClassifyTrimCurveOrient                                              M
*****************************************************************************/
CagdBType TrimClassifyTrimCurveOrient(const CagdCrvStruct *UVCrv)
{
    int i,
	Length = UVCrv -> Length - 1;
    CagdRType Dx, AvgY, Accum,
	* const *Points = UVCrv -> Points;

    Dx = Points[1][0] - Points[1][Length - 1];
    AvgY = Points[2][0] + Points[2][Length - 1]; /* Purged the "div by 2"... */
    Accum = AvgY * Dx;
    for (i = 0; i < Length - 1; i++) {
        Dx = Points[1][i + 1] - Points[1][i];
	AvgY = Points[2][i + 1] + Points[2][i];  /* Purged the "div by 2"... */
	Accum += AvgY * Dx;
    }

    return Accum > 0.0;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints a CagdPolylineStruct.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:  Polyline to print.		                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrimDbgPrintCagdPolyline(CagdPolylineStruct *Poly)
{
    int i;

    printf("[OBJECT PL\n    [POLYLINE %d\n", Poly -> Length);

    for (i = 0; i < Poly -> Length; i++) {
        printf("\t[ %15.12lg  %15.12lg  %15.12lg]\n",
	       Poly -> Polyline[i].Pt[0],
	       Poly -> Polyline[i].Pt[1],
	       Poly -> Polyline[i].Pt[2]);
    }

    printf("    ]\n]\n\n");
}

#endif /* DEBUG */
