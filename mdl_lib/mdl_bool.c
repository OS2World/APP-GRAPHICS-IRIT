/*****************************************************************************
*   Main module for booleans over models.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0, Dec. 1996    *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "mdl_loc.h"

#define MDL_BOOL_CLIP_EPS_EXTENSION 0.01
#define MDL_BOOL_MODEL_PERTURBATION 1e-12

IRIT_STATIC_DATA CagdRType
    GlblMdlBoolSubdivTol = 0.01,
    GlblMdlBoolNumerTol = 1e-8,
    GlblMdlBoolTraceTol = 0.01;
IRIT_STATIC_DATA int
    MdlBoolOutputInterCurve = FALSE,
    MdlBoolOutputInterCurveType = 0;
IRIT_GLOBAL_DATA jmp_buf
    _MdlBoolLongJumpBuf;			 /* Used in fatal Bool err. */
IRIT_GLOBAL_DATA MdlFatalErrorType 
    _MdlBoolFatalErrorNum = MDL_ERR_NO_ERROR;

static void MdlBoolCleanInterCrvs(CagdCrvStruct *Crv,
				  const CagdSrfStruct *Srf);
static MdlTrimSegStruct *MdlBoolFilterIntersections(MdlTrimSrfStruct *TSrf,
						    MdlTrimSegStruct *TSegs,
						    CagdBType First);
static void MdlBoolSplitInterTrimCrv(MdlTrimSegStruct *Seg1,
				     MdlTrimSegStruct *Seg2,
				     CagdBType FirstInSeg1,
				     CagdBType FirstInSeg2,
				     const CagdPtStruct *InterPts,
				     CagdRType Eps);
static void MdlBoolCorrectCrvEndPt(const MdlTrimSegStruct *Seg,
				   CagdBType FirstInSeg,
				   CagdBType BeginningOfSeg,
				   MdlTrimSegStruct *OtherSegs,
				   CagdBType FirstInOSegs,
				   CagdRType Eps);
static MdlTrimSegStruct *MdlBoolGenTrimSegs(const MvarPolyStruct *Inters12,
					    MdlTrimSrfStruct *Srf1,
					    MdlTrimSrfStruct *Srf2);
static void MdlBoolFPE(int Type);
static MdlModelStruct *MdlInterTwoModels(const MdlModelStruct *Model1,
					 const MdlModelStruct *Model2,
					 MdlBooleanType BType);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the tolerances to use in the boolean operations computations.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SubdivTol, NumerTol, TraceTol:    See MvarSrfSrfInter2.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanSetTolerances                                                  M
*****************************************************************************/
void MdlBooleanSetTolerances(CagdRType SubdivTol,
			     CagdRType NumerTol,
			     CagdRType TraceTol)
{
    GlblMdlBoolSubdivTol = SubdivTol;
    GlblMdlBoolNumerTol = NumerTol;
    GlblMdlBoolTraceTol = TraceTol;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make sure the intersection curve is fully consistent with the surface.   *
* Either precisely closed or precisely on the boundary of the surface.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:    Intersection curve to clean up.                                  *
*   Srf:    where Crv is an intersection (trimming) curve in its domain.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolCleanInterCrvs(CagdCrvStruct *Crv,
				  const CagdSrfStruct *Srf)
{
    CagdRType **Pts;
    int Len1 = Crv -> Length - 1;

    /* We assume the intersection curve is an E2 open end curve. */
    assert(Crv -> PType == CAGD_PT_E2_TYPE && BspCrvHasOpenEC(Crv));

    /*   Make sure the new intersection curves are either tightly closed,  */
    /* or tightly on the boundary.				           */
    Pts = Crv -> Points;

    if (IRIT_APX_EQ(Pts[1][0], Pts[1][Len1]) &&
	IRIT_APX_EQ(Pts[2][0], Pts[2][Len1])) {
        /* Make the end points precisely the same. */
        Pts[1][0] = Pts[1][Len1] = (Pts[1][0] + Pts[1][Len1]) * 0.5;
        Pts[2][0] = Pts[2][Len1] = (Pts[2][0] + Pts[2][Len1]) * 0.5;
    }
    else {
        int i;
        CagdRType UMin, UMax, VMin, VMax;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	for (i = 0; i <= Len1; i += Len1) {
	    if (IRIT_APX_EQ(Pts[1][i], UMin))
	        Pts[1][i] = UMin;
	    else if (IRIT_APX_EQ(Pts[1][i], UMax))
	        Pts[1][i] = UMax;

	    if (IRIT_APX_EQ(Pts[2][i], VMin))
	        Pts[2][i] = VMin;
	    else if (IRIT_APX_EQ(Pts[2][i], VMax))
	        Pts[2][i] = VMax;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Intersect the new given set of intersection curves, TSegs, with the      *
* given trimming curves of trimmed surface TSrf.                             *
*   TSegs segments that are outside the trimming zones are purged away while *
* the trimming curves of TSrf are also split at these intersection locations.*
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:    Current trimmed surface in a model.                             *
*   TSegs:   New intersection curves through TSrf. Updated in place.         *
*   First:   TRUE if TSrf is first surface in TSegs, FALSE if second.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSegStruct *:  The resulting TSegs.                                *
*****************************************************************************/
static MdlTrimSegStruct *MdlBoolFilterIntersections(MdlTrimSrfStruct *TSrf,
						    MdlTrimSegStruct *TSegs,
						    CagdBType First)
{
    MdlTrimSegStruct *TSeg;

    /* Make the intersection curves consistent and precise. */
    for (TSeg = TSegs; TSeg != NULL; TSeg = TSeg -> Pnext)
	MdlBoolCleanInterCrvs(First ? TSeg -> UVCrvFirst : TSeg -> UVCrvSecond,
			      TSrf -> Srf);

    /* Split the SSI intersections curves, TSegs, against the existing      */
    /* trimming curves of TSrf at all UV locations they meet.		    */
    for (TSeg = TSegs; TSeg != NULL; TSeg = TSeg -> Pnext) {
        MdlLoopStruct *Loop;

	for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	    MdlTrimSegRefStruct *SegRef;

	    for (SegRef = Loop -> SegRefList;
		 SegRef != NULL;
		 SegRef = SegRef -> Pnext) {
	        MdlTrimSegStruct
		    *Seg = SegRef -> TrimSeg;
		CagdPtStruct
		    *InterPts =
			CagdCrvCrvInter(First ? TSeg -> UVCrvFirst
					      : TSeg -> UVCrvSecond,
					TSrf == Seg -> SrfFirst ?
						    Seg -> UVCrvFirst :
						    Seg -> UVCrvSecond,
					IRIT_MAX(GlblMdlBoolNumerTol * 100,
						 IRIT_UEPS));

#		ifdef DEBUG_MDL_BOOL_DUMP_CCI_PTS
	        {
		    CagdRType t1, t2, r1, r2;
		    CagdPtStruct *Pt;

		    CagdCrvDomain(First ? TSeg -> UVCrvFirst 
				        : TSeg -> UVCrvSecond, &t1, &t2);
		    CagdCrvDomain(TSrf == Seg -> SrfFirst ? Seg -> UVCrvFirst
							  : Seg -> UVCrvSecond,
				  &r1, &r2);
		    fprintf(stderr, "Inter Points for: [%.10f %.10f], [%.10f, %.10f]\n",
			    t1, t2, r1, r2);
		    for (Pt = InterPts; Pt != NULL; Pt = Pt -> Pnext) {
			fprintf(stderr, "Inter Pt = %.15f %.15f\n",
				Pt -> Pt[0], Pt -> Pt[1]);
		    }
	        }
#		endif /* DEBUG_MDL_BOOL_DUMP_CCI_PTS */

		if (InterPts != NULL) {
		    /* Split TSeg and old trimming curve, Seg, in place. */
		    MdlBoolSplitInterTrimCrv(TSeg, Seg,
					     First, TSrf == Seg -> SrfFirst,
					     InterPts, MDL_BOOL_NEAR_UV_EPS);

		    CagdPtFreeList(InterPts);
		}
	    }
	}
    }

    MdlEnsureTSrfTrimCrvsPrecision(TSrf);

    /* Filter out intersection curves that are outside. */
    return MdlFilterOutCrvs(TSegs, TSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Divides the two intersecting curve segments at InterPts.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Seg1, Seg2:   The two intersecting segments.                             *
*   FirstInSeg1, FirstInSeg2: TRUE if relevant curve is first, FALSE second. *
*   InterPts:     List of parameters to split Se1/2 at.  First coordinate    *
*                 holds Seg1 params and second coordinate Seg2 params.       *
*   Eps:          Tolerance to consider parameters as identical or too close *
*                 to the boundary.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolSplitInterTrimCrv(MdlTrimSegStruct *Seg1,
				     MdlTrimSegStruct *Seg2,
				     CagdBType FirstInSeg1,
				     CagdBType FirstInSeg2,
				     const CagdPtStruct *InterPts,
				     CagdRType Eps)
{
    int Proximity1, Proximity2;
    MdlTrimSegStruct
	*Seg1Next = Seg1 -> Pnext,
	*Seg2Next = Seg2 -> Pnext;

    Seg1 -> Pnext = Seg2 -> Pnext = NULL;

    MdlSplitTrimCrv(Seg1, InterPts, 0, Eps, &Proximity1);
    MdlSplitTrimCrv(Seg2, InterPts, 1, Eps, &Proximity2);

    if (Proximity1 != 0) {
        if (Proximity1 & 0x01) {
	    /* First parameter too close to beginning of domain & ignored. */
	    MdlBoolCorrectCrvEndPt(Seg1, FirstInSeg1, TRUE,
				   Seg2, FirstInSeg2, Eps);
	}
        if (Proximity1 & 0x02) {
	    /* Last parameter too close to end of domain & ignored. */
	    MdlBoolCorrectCrvEndPt(CagdListLast(Seg1), FirstInSeg1, FALSE,
				   Seg2, FirstInSeg2, Eps);
	}
        if (Proximity1 & 0x04) {
	    assert(0);         /* Interior locations that are too similar. */
	}
    }

    if (Proximity2 != 0) {
        if (Proximity2 & 0x01) {
	    /* First parameter too close to beginning of domain & ignored. */
	    MdlBoolCorrectCrvEndPt(Seg2, FirstInSeg2, TRUE,
				   Seg1, FirstInSeg1, Eps);
	}
        if (Proximity2 & 0x02) {
	    /* Last parameter too close to end of domain & ignored. */
	    MdlBoolCorrectCrvEndPt(CagdListLast(Seg2), FirstInSeg2,
				   FALSE, Seg1, FirstInSeg1, Eps);
	}
        if (Proximity2 & 0x04) {
	    assert(0);         /* Interior locations that are too similar. */
	}
    }

    ((MdlTrimSegStruct *) CagdListLast(Seg1)) -> Pnext = Seg1Next;
    ((MdlTrimSegStruct *) CagdListLast(Seg2)) -> Pnext = Seg2Next;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Correct the given Seg end point to be on the location found within       *
* OtherSegs.                                                                 *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Seg:            Segments to correct OtherSeg using its designated end pt.*
*   FirstInSeg:     TRUE to handle the first curve in Seg, FALSE for second. *
*   BeginningOfSeg: TRUE to correct based on beginning of Seg, FALSE end.    *
*   OtherSegs:      The other segments to correct the intersection location. *
*   FirstInOSegs:   TRUE to handle the first curve in OtherSegs, FALSE 2nd.  *
*   Eps:            Tolerance of comparison.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolCorrectCrvEndPt(const MdlTrimSegStruct *Seg,
				   CagdBType FirstInSeg,
				   CagdBType BeginningOfSeg,
				   MdlTrimSegStruct *OtherSegs,
				   CagdBType FirstInOSegs,
				   CagdRType Eps)
{
    CagdRType TMin, TMax, *R;
    CagdUVType UV, UVTmp;
    CagdCrvStruct
        *UVCrv = FirstInSeg ? Seg -> UVCrvFirst : Seg -> UVCrvSecond;
    MdlTrimSegStruct *OSeg;

    /* Get the location we want to correct to. */
    CagdCrvDomain(UVCrv, &TMin, &TMax);
    R = CagdCrvEval(UVCrv, BeginningOfSeg ? TMin : TMax);
    CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);

    /* Go over all the others segments, search for the UV neighborhood and */
    /* correct to be precisely it if found such a neighborhood.		   */
    for (OSeg = OtherSegs; OSeg != NULL; OSeg = OSeg -> Pnext) {
        UVCrv = FirstInOSegs ? OSeg -> UVCrvFirst : OSeg -> UVCrvSecond;

	CagdCrvDomain(UVCrv, &TMin, &TMax);

	R = CagdCrvEval(UVCrv, TMin);
	CagdCoerceToE2(UVTmp, &R, -1, UVCrv -> PType);
	if (IRIT_PT_APX_EQ_E2_EPS(UV, UVTmp, Eps)) {
	    /* Correct UVTmp to UV. */
	    assert(UVCrv -> Order == 2); /* Assume a linear curve! */
	    UVCrv -> Points[1][0] = UV[0];
	    UVCrv -> Points[2][0] = UV[1];
	}

	R = CagdCrvEval(UVCrv, TMax);
	CagdCoerceToE2(UVTmp, &R, -1, UVCrv -> PType);
	if (IRIT_PT_APX_EQ_E2_EPS(UV, UVTmp, Eps)) {
	    /* Correct UVTmp to UV. */
	    assert(UVCrv -> Order == 2); /* Assume a linear curve! */
	    UVCrv -> Points[1][UVCrv -> Length - 1] = UV[0];
	    UVCrv -> Points[2][UVCrv -> Length - 1] = UV[1];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the result of two surfaces intersected into MdlTrimSegStructs.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Inters12:     The intersection result as polylines in R^4 (the space of  *
*                 both surfaces' domains).				     *
*   Srf1, Srf2:   The two surfaces participated in this intersection         *
*		  computation.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSegSturct *:   List of model trimming segments.                   *
*****************************************************************************/
static MdlTrimSegStruct *MdlBoolGenTrimSegs(const MvarPolyStruct *Inters12,
					    MdlTrimSrfStruct *Srf1,
					    MdlTrimSrfStruct *Srf2)
{
    const MvarPolyStruct *MVPl;
    MdlTrimSegStruct *TSeg,
        *TSegs = NULL;

    for (MVPl = Inters12; MVPl != NULL; MVPl = MVPl -> Pnext) {
        MvarPtStruct *MVPt,
	    *MVPts = MVPl -> Pl;

	if (MVPts != NULL && MVPts -> Pnext == NULL) {
	    /* A singular case we ignore. */
	}
	else {
	    int i,
	        n = CagdListLength(MVPts);
	    CagdCrvStruct
	        *UVCrv1 = BspCrvNew(n, 2, CAGD_PT_E2_TYPE),
	        *UVCrv2 = BspCrvNew(n, 2, CAGD_PT_E2_TYPE);

	    BspKnotUniformOpen(n, 2, UVCrv1 -> KnotVector);
	    BspKnotUniformOpen(n, 2, UVCrv2 -> KnotVector);

	    for (i = 0, MVPt = MVPts;
		 MVPt != NULL;
		 i++, MVPt = MVPt -> Pnext) {
		UVCrv1 -> Points[1][i] = MVPt -> Pt[0];
		UVCrv1 -> Points[2][i] = MVPt -> Pt[1];

		UVCrv2 -> Points[1][i] = MVPt -> Pt[2];
		UVCrv2 -> Points[2][i] = MVPt -> Pt[3];
	    }
	    assert(i == n);

	    TSeg = MdlTrimSegNew(UVCrv1, UVCrv2, NULL, Srf1, Srf2);
	    IRIT_LIST_PUSH(TSeg, TSegs);
	}
    }

    return TSegs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clean unused trimming curves/surfaces in the model - trimming curves     M
* that are used by no trimming surface.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:   To process and clean unused trimming crvs/srfs in, in place.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Number of trimming curves that were purged.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolCleanUnusedTrimCrvsSrfs                                           M
*****************************************************************************/
int MdlBoolCleanUnusedTrimCrvsSrfs(MdlModelStruct *Model)
{
    int n = 0;
    MdlTrimSegStruct *Seg, *TSeg;
    MdlTrimSrfStruct *TSrf, *TmpTSrf;

    /* Mark all trimming curves as unused. */
    for (Seg = Model -> TrimSegList; Seg != NULL; Seg = Seg -> Pnext)
        MDL_RST_TSEG_USED(Seg);
    
    /* Mark all used trimmed curves from their reference as such: */
    for (TSrf = Model -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
        MdlLoopStruct *Loop, *TmpLoop;

	for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	    MdlTrimSegRefStruct *SegRef;

	    for (SegRef = Loop -> SegRefList;
		 SegRef != NULL;
		 SegRef = SegRef -> Pnext) {
	        MDL_SET_TSEG_USED(SegRef -> TrimSeg);
		AttrFreeOneAttribute(&SegRef -> Attr, "_Inside");
	    }
	}

	/* Remove empty loops, if any. */
	if (TSrf -> LoopList != NULL) {
	    while (TSrf -> LoopList != NULL &&		 /* First is empty. */
		   TSrf -> LoopList -> SegRefList == NULL) {
		TmpLoop = TSrf -> LoopList -> Pnext;
		MdlLoopFree(TSrf -> LoopList);
		TSrf -> LoopList = TmpLoop;
	    }

	    if (TSrf -> LoopList != NULL &&
		TSrf -> LoopList -> Pnext != NULL) {
		for (Loop = TSrf -> LoopList; Loop -> Pnext != NULL; ) {
		    if (Loop -> Pnext -> SegRefList == NULL) {
			TmpLoop = Loop -> Pnext;
			Loop -> Pnext = TmpLoop -> Pnext;
			MdlLoopFree(TmpLoop);
		    }
		    else
			Loop = Loop -> Pnext;
		}
	    }
	}
    }

    /* Remove all the trimming curves that are unused. */
    while (Model -> TrimSegList != NULL &&
	   !MDL_IS_TSEG_USED(Model -> TrimSegList)) {
        Seg = Model -> TrimSegList;
	Model -> TrimSegList = Seg -> Pnext;
	MdlTrimSegFree(Seg);
	n++;
    }

    if (Model -> TrimSegList != NULL) {
        for (Seg = Model -> TrimSegList; Seg -> Pnext != NULL; ) {
	    if (!MDL_IS_TSEG_USED(Seg -> Pnext)) {
	        TSeg = Seg -> Pnext;
		Seg -> Pnext = TSeg -> Pnext;
		MdlTrimSegFree(TSeg);
		n++;
	    }
	    else
	        Seg = Seg -> Pnext;
	}
    }

    /* Remove trim-surfaces with no trimming curves. */
    if (Model -> TrimSrfList != NULL) {
	while (Model -> TrimSrfList != NULL &&		 /* First is empty. */
	       Model -> TrimSrfList -> LoopList == NULL) {
	    TmpTSrf = Model -> TrimSrfList -> Pnext;
	    MdlTrimSrfFree(Model -> TrimSrfList);
	    Model -> TrimSrfList = TmpTSrf;
	}

	if (Model -> TrimSrfList != NULL &&
	    Model -> TrimSrfList -> Pnext != NULL) {
	    for (TSrf = Model -> TrimSrfList; TSrf -> Pnext != NULL; ) {
		if (TSrf -> Pnext -> LoopList == NULL) {
		    TmpTSrf = TSrf -> Pnext;
		    TSrf -> Pnext = TmpTSrf -> Pnext;
		    MdlTrimSrfFree(TmpTSrf);
		}
		else
		    TSrf = TSrf -> Pnext;
	    }
	}
    }

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clip the surfaces in the given model to the domain of the trimming       M
* curves (+ some epsilon).						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:   To clip its surfaces to their trimmed domain, in place.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolClipTSrfs2TrimDomain                                              M
*****************************************************************************/
void MdlBoolClipTSrfs2TrimDomain(MdlModelStruct *Model)
{
    MdlTrimSrfStruct *TSrf;
    
    for (TSrf = Model -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
        CagdRType UMin, UMax, VMin, VMax, Du, Dv;
	CagdBBoxStruct BBox;
	CagdSrfStruct *Srf1, *Srf2;

	CagdSrfDomain(TSrf -> Srf, &UMin, &UMax, &VMin, &VMax);
	Du = (UMax - UMin) * MDL_BOOL_CLIP_EPS_EXTENSION;
	Dv = (VMax - VMin) * MDL_BOOL_CLIP_EPS_EXTENSION;
 
	MdlModelTSrfTCrvsBBox(TSrf, &BBox);

	if (BBox.Min[0] - Du > UMin || BBox.Max[0] + Du < UMax) {
	    /* Can clip the surface in U! */
	    UMin = IRIT_MAX(BBox.Min[0] - Du, UMin);
	    UMax = IRIT_MIN(BBox.Max[0] + Du, UMax);
	    if (CAGD_IS_BEZIER_SRF(TSrf -> Srf)) {
	        CagdSrfStruct
		    *TmpSrf = CagdCnvrtBzr2BspSrf(TSrf -> Srf);

	        Srf1 = CagdSrfRegionFromSrf(TmpSrf, UMin, UMax,
					    CAGD_CONST_U_DIR);
		CagdSrfFree(TmpSrf);
	    }
	    else
	        Srf1 = CagdSrfRegionFromSrf(TSrf -> Srf, UMin, UMax,
					    CAGD_CONST_U_DIR);
	}
	else
	    Srf1 = TSrf -> Srf;

	if (BBox.Min[1] - Dv > VMin || BBox.Max[1] + Dv < VMax) {
	    /* Can clip the surface in V! */
	    VMin = IRIT_MAX(BBox.Min[1] - Dv, VMin);
	    VMax = IRIT_MIN(BBox.Max[1] + Dv, VMax);
	    if (CAGD_IS_BEZIER_SRF(Srf1)) {
	        CagdSrfStruct
		    *TmpSrf = CagdCnvrtBzr2BspSrf(Srf1);

	        Srf2 = CagdSrfRegionFromSrf(TmpSrf, VMin, VMax,
					    CAGD_CONST_V_DIR);
		CagdSrfFree(TmpSrf);
	    }
	    else
	        Srf2 = CagdSrfRegionFromSrf(Srf1, VMin, VMax,
					    CAGD_CONST_V_DIR);
	}
	else
	    Srf2 = Srf1;

	if (Srf1 != TSrf -> Srf && Srf1 != Srf2)
	    CagdSrfFree(Srf1);
	if (Srf2 != TSrf -> Srf)
	    CagdSrfFree(TSrf -> Srf);

	TSrf -> Srf = Srf2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine that is called from the floating point package in case of fatal  *
* floating point error. Print error message, and quit this Boolean module.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Type:       of exception.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolFPE(int Type)
{
    char Line[IRIT_LINE_LEN];

    sprintf(Line, IRIT_EXP_STR("Floating point error %d."), Type);
    IRIT_WARNING_MSG(Line);

    _MdlBoolFatalErrorNum = MDL_ERR_FP_ERROR;

    longjmp(_MdlBoolLongJumpBuf, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the intersection curves between the given two models, if any,    *
* and process according to the desired Boolean operation.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Model1, Model2:    The two models to consider.                           *
*   BType:             The type of Boolean operation involved.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlModelStruct *:    Resulting reconstructed boolean model.              *
*****************************************************************************/
static MdlModelStruct *MdlInterTwoModels(const MdlModelStruct *Model1,
					 const MdlModelStruct *Model2,
					 MdlBooleanType BType)
{
    int MdlsInter = FALSE;
    IrtVecType Translate;
    MdlModelStruct *CpModel2,
        *CpModel1 = MdlModelCopy(Model1),
	*Model = MdlModelNew2(NULL, NULL);
    MdlTrimSegRefStruct *SegRef;
    MdlLoopStruct *Loop;
    MdlTrimSrfStruct *TSrf, *TSrf1, *TSrf2;

    if (setjmp(_MdlBoolLongJumpBuf) != 0) { 	     /* Its a fatal error! */
	MDL_FATAL_ERROR(_MdlBoolFatalErrorNum);
	return NULL;
    }
    _MdlBoolFatalErrorNum = MDL_ERR_NO_ERROR;
    signal(SIGFPE, MdlBoolFPE);      /* Will trap floating point errors. */

    /* Convert "A - B" to "A * -B". */
    if (BType == MDL_BOOL_SUBTRACTION) {
        CpModel2 = MdlModelNegate(Model2);
        BType = MDL_BOOL_INTERSECTION;
    }
    else
        CpModel2 = MdlModelCopy(Model2);

#ifdef DEBUG
    assert(MdlDebugVerify(CpModel1, TRUE) && MdlDebugVerify(CpModel2, TRUE));
#endif /* DEBUG */

    /* Mark all trimmed curves reference as old: */
    for (TSrf1 = CpModel1 -> TrimSrfList;
	 TSrf1 != NULL;
	 TSrf1 = TSrf1 -> Pnext) {
        AttrSetIntAttrib(&TSrf1 -> Attr, "_Input", 1);
	for (Loop = TSrf1 -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	    for (SegRef = Loop -> SegRefList;
		 SegRef != NULL;
		 SegRef = SegRef -> Pnext)
	        MDL_RST_TSEG_NEW(SegRef);
	}
    }

    /* Apply perturbations to second model. */
    Translate[0] = -1.301060 * MDL_BOOL_MODEL_PERTURBATION;
    Translate[1] = -2.301060 * MDL_BOOL_MODEL_PERTURBATION;
    Translate[2] = -3.301060 * MDL_BOOL_MODEL_PERTURBATION;
    MdlModelTransform(CpModel2, Translate, 1.0);

    for (TSrf2 = CpModel2 -> TrimSrfList;
	 TSrf2 != NULL;
	 TSrf2 = TSrf2 -> Pnext) {
        AttrSetIntAttrib(&TSrf2 -> Attr, "_Input", 2);

	for (Loop = TSrf2 -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	    for (SegRef = Loop -> SegRefList;
		 SegRef != NULL;
		 SegRef = SegRef -> Pnext)
	        MDL_RST_TSEG_NEW(SegRef);
	}
    }

    /* Do an O(n^2) pass over all pairs of surfaces and intersect them, */
    for (TSrf1 = CpModel1 -> TrimSrfList;
	 TSrf1 != NULL;
	 TSrf1 = TSrf1 -> Pnext) {
        for (TSrf2 = CpModel2 -> TrimSrfList;
	     TSrf2 != NULL;
	     TSrf2 = TSrf2 -> Pnext) {
	    MvarPolyStruct
	        *Inters12 = MvarSrfSrfInter2(TSrf1 -> Srf, TSrf2 -> Srf,
					     GlblMdlBoolTraceTol,
					     GlblMdlBoolSubdivTol,
					     GlblMdlBoolNumerTol);

	    if (Inters12 != NULL) {
	        MdlTrimSegStruct *TSegs;

		/* Merge results into a new MdlTrimSegStruct. */
		TSegs = MdlBoolGenTrimSegs(Inters12, TSrf1, TSrf2);
		MvarPolyFreeList(Inters12);

		/* Filter the intersection curves based on the current      */
	        /* trimming curves of TSrf1/2 if any, so intersections not  */
		/* inside the defined trimming domain will be purged now.   */
		/*   Also existing trimming curves are split at these       */
		/* intersection locations so we can form new loops.         */
		TSegs = MdlBoolFilterIntersections(TSrf1, TSegs, TRUE);
		TSegs = MdlBoolFilterIntersections(TSrf2, TSegs, FALSE);

		if (TSegs != NULL) {
		    MdlsInter = TRUE;

		    Model -> TrimSegList = 
		                  CagdListAppend(TSegs, Model -> TrimSegList);
		}
	    }
	}
    }

#ifdef DEBUG
    assert(MdlDebugVerify(Model, FALSE));
#endif /* DEBUG */

    if (!MdlsInter) {
        /* Either they are completely disjoint or one is completely         */
        /* contained in the other.  Not handled yet.	   		    */
	MDL_FATAL_ERROR(_MdlBoolFatalErrorNum = MDL_ERR_BOOL_DISJOINT);
	return NULL;
    }
    else {
	MdlTrimSrfStruct *TSrf;

	assert(Model -> TrimSegList != NULL);

	/* Chain all surfaces into one list. */
	Model -> TrimSrfList =
	      CagdListAppend(CpModel1 -> TrimSrfList, CpModel2 -> TrimSrfList);
	CpModel1 -> TrimSrfList = CpModel2 -> TrimSrfList = NULL;

	if (BType == MDL_BOOL_INTER_CRVS) {
	    MdlModelFree(CpModel1);
	    MdlModelFree(CpModel2);

	    return Model;		       /* Only intersection curves. */
	}

	/* collect all segments of a certain surface & add to its ref. list. */
	for (TSrf = Model -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    MdlLoopStruct
	        *Loop = NULL;
	    MdlTrimSegStruct *TSeg;

	    /* Scan the intersection curves - find those with TSrf. */
	    for (TSeg = Model -> TrimSegList;
		 TSeg != NULL;
		 TSeg = TSeg -> Pnext) {
	        if (TSeg -> SrfFirst == TSrf || TSeg -> SrfSecond == TSrf) {
		    MdlTrimSegRefStruct
		        *TSegRef = MdlTrimSegRefNew(TSeg);

		    /* Create reference to this intersection curve in TSrf. */
		    MDL_SET_TSEG_NEW(TSegRef);
		    if (Loop == NULL)
		        Loop = MdlLoopNew(TSegRef);
		    else {
		        IRIT_LIST_PUSH(TSegRef, Loop -> SegRefList);
		    }
		}
	    }

	    if (Loop != NULL)
		IRIT_LIST_PUSH(Loop, TSrf -> LoopList);
	}

        /* Append the existing trimming curve segments. */
	Model -> TrimSegList =
		 CagdListAppend(Model -> TrimSegList, CpModel1 -> TrimSegList);
	Model -> TrimSegList =
		 CagdListAppend(Model -> TrimSegList, CpModel2 -> TrimSegList);
	CpModel1 -> TrimSegList = CpModel2 -> TrimSegList = NULL;

	MdlModelFree(CpModel1);
	MdlModelFree(CpModel2);
    }

#ifdef DEBUG
    assert(MdlDebugVerify(Model, FALSE));
#endif /* DEBUG */

#ifdef MDL_DEBUG_DUMP_BOOL_TEMP1RSLT
    MdlDbgMC(Model, FALSE);
#endif /* MDL_DEBUG_DUMP_BOOL_TEMP1RSLT */

    /* Classify and build the new trimming loops in each trimmed surface.   */
    /* Start with the trimmed surfaces that actually intersect now.         */
    for (TSrf = Model -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
        if (MdlBoolTrimSrfIntersects(TSrf)) {
	    int Inside,
	        Input = AttrGetIntAttrib(TSrf -> Attr, "_Input");

	    switch (BType) {
	        case MDL_BOOL_UNION:
		    Inside = FALSE;
		    break;
	        case MDL_BOOL_INTERSECTION:
		    Inside = TRUE;
		    break;
	        case MDL_BOOL_SUBTRACTION:
		    Inside = TRUE;
		    assert(0);/* Should have been converted to an inter. op.*/
		    break;
	        case MDL_BOOL_CUT:
		    Inside = Input == 1 ? FALSE : TRUE;
		    break;
	        default:
		    Inside = FALSE;
		    assert(0);
		    break;
	    }

	    MdlBoolClassifyTrimSrfLoops(TSrf, GlblMdlBoolNumerTol, Inside);

	    AttrFreeOneAttribute(&TSrf -> Attr, "_Input");
	}
    }

    /* And now propagate the classification to the non-intersecting srfs. */
    MdlBoolClassifyNonInterTrimSrfs(Model);

    MdlBoolCleanUnusedTrimCrvsSrfs(Model);
    MdlBoolClipTSrfs2TrimDomain(Model);
    MdlEnsureMdlTrimCrvsPrecision(Model);

#ifdef DEBUG
    assert(MdlDebugVerify(Model, TRUE));
#endif /* DEBUG */

    return Model;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Boolean union of two models.		                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2:    The two models to consider.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Resulting boolean.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanIntersection                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanUnion                                                          M
*****************************************************************************/
IPObjectStruct *MdlBooleanUnion(const MdlModelStruct *Model1, 
				const MdlModelStruct *Model2)
{
    if (MdlBoolOutputInterCurve)
        return IPGenCRVObject(MdlBooleanInterCrv(Model1, Model2,
						 MdlBoolOutputInterCurveType));
    else
        return IPGenMODELObject(MdlInterTwoModels(Model1, Model2,
						  MDL_BOOL_UNION));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Boolean intersection of two models.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2:    The two models to consider.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Resulting boolean.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanSetTolerances                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanIntersection                                                   M
*****************************************************************************/
IPObjectStruct *MdlBooleanIntersection(const MdlModelStruct *Model1, 
				       const MdlModelStruct *Model2)
{
    
    if (MdlBoolOutputInterCurve)
        return IPGenCRVObject(MdlBooleanInterCrv(Model1, Model2,
						 MdlBoolOutputInterCurveType));
    else
        return IPGenMODELObject(MdlInterTwoModels(Model1, Model2,
						  MDL_BOOL_INTERSECTION));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Boolean subtraction of two models.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2:    The two models to consider.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Resulting boolean.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanSetTolerances                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanSubtraction                                                    M
*****************************************************************************/
IPObjectStruct *MdlBooleanSubtraction(const MdlModelStruct *Model1, 
				      const MdlModelStruct *Model2)
{
    if (MdlBoolOutputInterCurve)
        return IPGenCRVObject(MdlBooleanInterCrv(Model1, Model2,
						 MdlBoolOutputInterCurveType));
    else
        return IPGenMODELObject(MdlInterTwoModels(Model1, Model2,
						  MDL_BOOL_SUBTRACTION));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Boolean subtraction of two models.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2:    The two models to consider.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Resulting boolean.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanSetTolerances                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanCut		                                             M
*****************************************************************************/
IPObjectStruct *MdlBooleanCut(const MdlModelStruct *Model1,
			      const MdlModelStruct *Model2)
{
    if (MdlBoolOutputInterCurve)
        return IPGenCRVObject(MdlBooleanInterCrv(Model1, Model2,
						 MdlBoolOutputInterCurveType));
    else
        return IPGenMODELObject(MdlInterTwoModels(Model1, Model2,
						  MDL_BOOL_CUT));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the merge of two models.  All surfaces and trimming curves are  M
* concatenated together and optionally shared boundaries are stitched	     M
* together.					                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2:    The two models to consider.                           M
*   StitchBndries:     TRUE to also stitch shared boundaries.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Resulting merged mdoel.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlStitchModel	                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanMerge		                                             M
*****************************************************************************/
IPObjectStruct *MdlBooleanMerge(const MdlModelStruct *Model1,
				const MdlModelStruct *Model2,
				CagdBType StitchBndries)
{
    if (MdlBoolOutputInterCurve)
        return IPGenCRVObject(MdlBooleanInterCrv(Model1, Model2,
						 MdlBoolOutputInterCurveType));
    else {
	MdlModelStruct
	    *CpModel1 = MdlModelCopy(Model1),
	    *CpModel2 = MdlModelCopy(Model2),
	    *Model = MdlModelNew2(NULL, NULL);

	Model -> TrimSrfList = CagdListAppend(CpModel1 -> TrimSrfList,
					      CpModel2 -> TrimSrfList);
	CpModel1 -> TrimSrfList = CpModel2 -> TrimSrfList = NULL;

	Model -> TrimSegList = CagdListAppend(CpModel1 -> TrimSegList,
					      CpModel2 -> TrimSegList);
	CpModel1 -> TrimSegList = CpModel2 -> TrimSegList = NULL;

	MdlModelFree(CpModel1);
	MdlModelFree(CpModel2);

	if (StitchBndries)
	    MdlStitchModel(Model, IRIT_EPS);

	return IPGenMODELObject(Model);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve of two models.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Model1, Model2: The two models to consider.                              M
*   InterType:      0 for Euclidean space, 1/2 for inter curves in Model1/2. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Resulting intersection curves.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBooleanInterCrv		                                             M
*****************************************************************************/
CagdCrvStruct *MdlBooleanInterCrv(const MdlModelStruct *Model1,
				  const MdlModelStruct *Model2,
				  int InterType)
{
    MdlModelStruct
        *InterCrvs = MdlInterTwoModels(Model1, Model2, MDL_BOOL_INTER_CRVS);
    MdlTrimSegStruct *TSeg;
    CagdCrvStruct *Crv,
        *Crvs = NULL;

    for (TSeg = InterCrvs -> TrimSegList; TSeg != NULL; TSeg = TSeg -> Pnext) {
        switch (InterType) {
	    default:
	    case 0: /* Euclidean. */
	        if ((Crv = TSeg -> EucCrv) == NULL) {
		    /* Needs to evaluate from UV curves. */
		    assert(TSeg -> SrfFirst != NULL &&
			   TSeg -> UVCrvFirst != NULL);
		    Crv = TrimEvalTrimCrvToEuclid2(TSeg -> SrfFirst -> Srf,
						   TSeg -> UVCrvFirst);
		}
	        break;
	    case 1: /* Model1. */
	        assert(TSeg -> UVCrvFirst != NULL);
		Crv = CagdCrvCopy(TSeg -> UVCrvFirst);
	        break;
	    case 2: /* Model2. */
	        assert(TSeg -> UVCrvSecond != NULL);
	        Crv = TSeg -> UVCrvSecond;
	        break;
	}
	IRIT_LIST_PUSH(Crv, Crvs);
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if intersection curves or full Boolean operation is to be       M
* performed on input models.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   OutputInterCurve:    If TRUE only intersection curves are computed.      M
*                        If FALSE, full blown Boolean is applied.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	Old OutputInterCurve value.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanInterCrv, MdlBoolSetOutputInterCrvType			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolSetOutputInterCrv, Booleans                                       M
*****************************************************************************/
int MdlBoolSetOutputInterCrv(int OutputInterCurve)
{
    int Old = MdlBoolOutputInterCurve;

    MdlBoolOutputInterCurve = OutputInterCurve;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls the type of intersection curves to return.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   OutputInterCurveType:    0 for Euclidean space,			     M
*                            1/2 for inter curves in Model1/2.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	Old OutputInterCurveType value.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanInterCrv, MdlBoolSetOutputInterCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolSetOutputInterCrvType, Booleans                                   M
*****************************************************************************/
int MdlBoolSetOutputInterCrvType(int OutputInterCurveType)
{
    int Old = MdlBoolOutputInterCurveType;

    MdlBoolOutputInterCurveType = OutputInterCurveType;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the inside out (negation of the given model as a new model.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:    The model to negate.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:    Resulting negation.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*			                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelNegate		                                             M
*****************************************************************************/
MdlModelStruct *MdlModelNegate(const MdlModelStruct *Model)
{
    MdlModelStruct
        *CpModel = MdlModelCopy(Model);
    MdlTrimSrfStruct *TSrf;
    MdlTrimSegStruct *TSeg; 

    /* Reverse the orientation of a Model will be done by flipping all UV */
    /* coordinates of surfaces and their trimming curves.		  */

    for (TSrf = CpModel -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
        CagdSrfStruct
	    *Srf = CagdSrfReverse2(TSrf -> Srf);

	CagdSrfFree(TSrf -> Srf);
	TSrf -> Srf = Srf;
    }

    for (TSeg = CpModel -> TrimSegList; TSeg != NULL; TSeg = TSeg -> Pnext) {
        CagdCrvStruct *Crv;

        if (TSeg -> UVCrvFirst != NULL) {
	    Crv = CagdCrvReverseUV(TSeg -> UVCrvFirst);
	    CagdCrvFree(TSeg -> UVCrvFirst);
	    TSeg -> UVCrvFirst = Crv;
	}

        if (TSeg -> UVCrvSecond != NULL) {
	    Crv = CagdCrvReverseUV(TSeg -> UVCrvSecond);
	    CagdCrvFree(TSeg -> UVCrvSecond);
	    TSeg -> UVCrvSecond = Crv;
	}
    }
 
    return CpModel;
}
