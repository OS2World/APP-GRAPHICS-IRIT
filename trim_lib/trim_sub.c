/******************************************************************************
* Trim_sub.c - subdivision of trimmed surfaces.                               *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 95.					      *
******************************************************************************/

#include "trim_loc.h"

#define MAX_NUM_OF_INTERS		100
#define SUBDIV_TCRV_EPS_HIGHORDER	1e-4
#define SUBDIV_TCRV_EPS_LINEAR		1e-6
#define SUBDIV_PERTURB_PARAM		3.01060e-10

IRIT_STATIC_DATA CagdPtStruct
    *GlblPtList = NULL;

#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2);
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2);
#endif /* ultrix && mips (no const support) */

static int ClassifyTrimCurve(CagdCrvStruct *UVCrv, IrtRType t, int Axis);
static CagdCrvStruct *GenSplitBndrySegs(CagdCrvStruct *Crvs,
					int Axis,
					CagdRType t);
static CagdPtStruct *InterCrvLine(CagdCrvStruct *Crv, CagdLType Line);
static void InsertNewParam(CagdRType t);
static CagdCrvStruct *FreeTooSmallCollinearCurves(CagdCrvStruct **Crvs,
						  int Axis,
						  CagdRType t);
static void FiltersZeroAreaTrims(TrimCrvStruct **TrimCrvs);
static void TrimSubAddInterLoc(CagdRType *Inters,
			       int *NumOfInters,
			       CagdRType r);
static void SplitTrimmingCurve(CagdCrvStruct *Crv,
			       CagdPtStruct *Pts,
			       int Axis,
			       CagdRType t,
			       CagdCrvStruct **UVCrv1,
			       CagdCrvStruct **UVCrv2,
			       int *NumInters,
			       CagdRType *Inters);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trimmed surface - subdivides it into two sub-surfaces at given     M
* parametric value t in the given direction Dir.                             M
*    Returns pointer to a list of two trimmed surfaces, at most. It can very M
* well may happen that the subdivided surface is completely trimmed out and  M
* hence nothing is returned for it.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:  To subdivide at the prescibed parameter value t.               M
*   t:        The parameter to subdivide the curve Crv at.                   M
*   Dir:      Direction of subdivision. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  The subdivided surfaces. Usually two, but can have     M
*		      only one, if other is totally trimmed away.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfSubdivTrimmingCrvs                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfSubdivAtParam, subdivision                                        M
*****************************************************************************/
TrimSrfStruct *TrimSrfSubdivAtParam(TrimSrfStruct *TrimSrf,
				    CagdRType t,
				    CagdSrfDirType Dir)
{
    int i, IsBezier;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *DividedSrf,
	*Srf = TrimSrf -> Srf;
    TrimCrvStruct *TrimCrv1, *TrimCrv2;
    TrimSrfStruct *TSrf1, *TSrf2;

    /* Optionally, perturb the subdivision parameter a bit. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    IsBezier = CAGD_IS_BEZIER_SRF(Srf);
    switch (Dir) {
        case CAGD_CONST_U_DIR:
	    i = IsBezier ? -1
			 : BspKnotLastIndexLE(Srf -> UKnotVector,
					      Srf -> ULength + Srf -> UOrder,
					      t);

	    if (!IRIT_APX_EQ(t, UMin) &&
		!IRIT_APX_EQ(t, UMax) &&
	        (IsBezier || !IRIT_APX_EQ(t, Srf -> UKnotVector[i])))
	        t += SUBDIV_PERTURB_PARAM;
	    break;
        case CAGD_CONST_V_DIR:
	    i = IsBezier ? -1
			 : BspKnotLastIndexLE(Srf -> VKnotVector,
					      Srf -> VLength + Srf -> VOrder,
					      t);

	    if (!IRIT_APX_EQ(t, VMin) &&
		!IRIT_APX_EQ(t, VMax) &&
	        (IsBezier || !IRIT_APX_EQ(t, Srf -> VKnotVector[i])))
	        t += SUBDIV_PERTURB_PARAM;
	    break;
        default:
	    assert(0);
    }


    TrimPiecewiseLinearTrimmingCurves(TrimSrf, FALSE);

    /* Coerce Bezier surfaces to Bspline so we do not have to scale the */
    /* trimming curves to the unit domain all the time.		        */
    Srf = TrimSrf -> Srf;
    if (CAGD_IS_BEZIER_SRF(Srf))
	Srf = CagdCnvrtBzr2BspSrf(Srf);
    DividedSrf = BspSrfSubdivAtParam(Srf, t, Dir);
    if (Srf != TrimSrf -> Srf)
	CagdSrfFree(Srf);

    TrimSrfSubdivTrimmingCrvs(TrimSrf -> TrimCrvList, t, Dir,
			      &TrimCrv1, &TrimCrv2);

    /* Construct the (upto) two new trimmed surfaces and return them. */
    if (TrimCrv1 == NULL) {
	TSrf1 = TrimSrfNew(DividedSrf -> Pnext, TrimCrv2, TRUE);
	CagdSrfFree(DividedSrf);
    }
    else if (TrimCrv2 == NULL) {
	TSrf1 = TrimSrfNew(DividedSrf, TrimCrv1, TRUE);
	CagdSrfFree(DividedSrf -> Pnext);
	DividedSrf -> Pnext = NULL;
    }
    else {
	TSrf1 = TrimSrfNew(DividedSrf, TrimCrv1, TRUE);
	TSrf2 = TrimSrfNew(DividedSrf -> Pnext, TrimCrv2, TRUE);
	DividedSrf -> Pnext = NULL;
	TSrf1 -> Pnext = TSrf2;
    }

    return TSrf1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of trimming curves - subdivides them into two groups below and M
* above the subdividng line in direction Dir at parameter t.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvs:  To subdivide at the prescibed parameter value t.              M
*   t:         The parameter to subdivide the curve Crv at.                  M
*   Dir:       Direction of subdivision. Either U or V.                      M
*   TrimCrvs1: Returned first half of trimming curves, < t.  Could be NULL.  M
*   TrimCrvs2: Returned second half of trimming curves, > t.  Could be NULL. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful and have two halfs.  FALSE if failed or have   M
*	   only one half.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfSubdivAtParam	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfSubdivTrimmingCrvs, subdivision                                   M
*****************************************************************************/
int TrimSrfSubdivTrimmingCrvs(TrimCrvStruct *TrimCrvs,
			      CagdRType t,
			      CagdSrfDirType Dir,
			      TrimCrvStruct **TrimCrvs1,
			      TrimCrvStruct **TrimCrvs2)
{
    int Axis,
	NumOfInters = 0;
    CagdRType *Inters;
    CagdCrvStruct *Crv,
	*UVCrv1All = NULL,
	*UVCrv2All = NULL;
    TrimCrvStruct *TrimCrvTmp, *TrimCrv;
    CagdLType Line;

    *TrimCrvs1 = *TrimCrvs2 = NULL;

    TrimCrvs = TrimChainTrimmingCurves2Loops(TrimCrvs);

    /* Compute a line to subdivide along and represent it as a curve. */
    Inters = (CagdRType *) IritMalloc(sizeof(CagdRType) * MAX_NUM_OF_INTERS);
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Axis = 0;
	    Line[0] = 1;
	    Line[1] = 0;
	    Line[2] = -t;
	    break;
	case CAGD_CONST_V_DIR:
	    Axis = 1;
	    Line[0] = 0;
	    Line[1] = 1;
	    Line[2] = -t;
	    break;
	default:
	    TRIM_FATAL_ERROR(TRIM_ERR_DIR_NOT_CONST_UV);
	    return FALSE;
    }


    /* If t is very close to a control point value - make the control point  */
    /* identical to t.							     */
    for (TrimCrv = TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    int i;
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;
	    IrtRType
		**Points = UVCrv -> Points;

	    for (i = 0; i < UVCrv -> Length; i++)
	        if (IRIT_APX_EQ_EPS(Points[Axis + 1][i], t, CAGD_MAX_DOMAIN_EPS))
		    Points[Axis + 1][i] = t;
	}
    }

    /* Compute the hard part - subdividing the trimming curves into two. */
    for (TrimCrv = TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;
	    CagdPtStruct
		*Pts = InterCrvLine(UVCrv, Line);

	    if (Pts == NULL) {/* No intersection - classify to one of halfs. */
	        if (ClassifyTrimCurve(UVCrv, t, Axis)) {
		    /* Goes to second half. */
		    TrimCrvTmp = TrimCrvNew(TrimCrvSegCopy(TrimCrvSeg));
		    TrimCrvTmp -> Pnext = *TrimCrvs2;
		    *TrimCrvs2 = TrimCrvTmp;
		}
		else {
		    /* Goes to first half. */
		    TrimCrvTmp = TrimCrvNew(TrimCrvSegCopy(TrimCrvSeg));
		    TrimCrvTmp -> Pnext = *TrimCrvs1;
		    *TrimCrvs1 = TrimCrvTmp;
		}
	    }
	    else { /* Needs to split the curve at the intersections. */
		CagdCrvStruct *UVCrv1, *UVCrv2;

		SplitTrimmingCurve(UVCrv, Pts, Axis, t, &UVCrv1, &UVCrv2,
				   &NumOfInters, Inters);

#		ifdef DEBUG
		{
		    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpSplitTrimCrvs1,
						   FALSE) {
		        IRIT_INFO_MSG_PRINTF(
				"Input UVCrv to be split at axis %d, t = %f\n",
				Axis, t);
			CagdDbg(UVCrv);
			IRIT_INFO_MSG("\nFirst half:\n");
			CagdDbg(UVCrv1);
			IRIT_INFO_MSG("\nSecond half:\n");
			CagdDbg(UVCrv2);
		    }
		}
#		endif /* DEBUG */

		if (UVCrv1 != NULL && FreeTooSmallCollinearCurves(&UVCrv1,
								  Axis, t))
		    UVCrv1All = CagdListAppend(UVCrv1, UVCrv1All);

		if (UVCrv2 != NULL && FreeTooSmallCollinearCurves(&UVCrv2,
								  Axis, t))
		    UVCrv2All = CagdListAppend(UVCrv2, UVCrv2All);

		CagdPtFreeList(Pts);
	    }
	}
    }
    TrimCrvFreeList(TrimCrvs);

    IritFree(Inters);

    UVCrv1All = CagdListAppend(GenSplitBndrySegs(UVCrv1All, Axis, t),
			       UVCrv1All);
    UVCrv2All = CagdListAppend(GenSplitBndrySegs(UVCrv2All, Axis, t),
			       UVCrv2All);

#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpSplitTrimCrvs2, FALSE) {
	    IRIT_INFO_MSG("\nFirst half with closing segments:\n");
	    CagdDbg(UVCrv1All);
	    IRIT_INFO_MSG("\nSecond half with closing segments:\n");
	    CagdDbg(UVCrv2All);
	}
    }
#   endif /* DEBUG */

    for (Crv = UVCrv1All; Crv != NULL; ) {
	CagdCrvStruct
	    *NextCrv = Crv -> Pnext;

	TrimCrvTmp = TrimCrvNew(TrimCrvSegNew(Crv, NULL));
	Crv -> Pnext = NULL;

	TrimCrvTmp -> Pnext = *TrimCrvs1;
	*TrimCrvs1 = TrimCrvTmp;

	Crv = NextCrv;
    }
    for (Crv = UVCrv2All; Crv != NULL; ) {
	CagdCrvStruct
	    *NextCrv = Crv -> Pnext;

	TrimCrvTmp = TrimCrvNew(TrimCrvSegNew(Crv, NULL));
	Crv -> Pnext = NULL;

	TrimCrvTmp -> Pnext = *TrimCrvs2;
	*TrimCrvs2 = TrimCrvTmp;

	Crv = NextCrv;
    }

    /* A test for a zero area trimming curves - to be purged away. */
    FiltersZeroAreaTrims(TrimCrvs1);
    FiltersZeroAreaTrims(TrimCrvs2);

    return (*TrimCrvs1) != NULL && (*TrimCrvs2) != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two real numbers for sorting purposes.                *
*                                                                            *
* PARAMETERS:                                                                *
*   PReal1, PReal2:  Two pointers to real numbers.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two reals.               *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2)
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
	Diff = (*((CagdRType *) PReal1)) - (*((CagdRType *) PReal2));

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the segments along the splitting subdivision line that needs   *
* to be added to Crvs.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      Curves on one side of the axis.                               *
*   Axis:      The axis of the splitting line.  0 for U, 1 for V.            *
*   t:         The split value along axis Axis.	             		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCRvStruct *:                                                         *
*****************************************************************************/
static CagdCrvStruct *GenSplitBndrySegs(CagdCrvStruct *TrimCrvs,
					int Axis,
					CagdRType t)
{
    int i,
	NumOfInters = 0,
	OAxis = 1 - Axis;
    CagdRType *Inters;
    CagdCrvStruct *Crv, *TCrv,
	*Crvs = NULL;

    if (TrimCrvs == NULL)
	return NULL;

    Inters = (CagdRType *) IritMalloc(sizeof(CagdRType) * 2 *
						   CagdListLength(TrimCrvs));

    /* Build a sorted list of the end points on the split axis. */
    for (Crv = TrimCrvs; Crv != NULL; Crv = Crv -> Pnext) {
	/* Get the two end points and insert into Inter if on axis. */
        int Len1 = Crv -> Length - 1;
        CagdRType
	    **Points = Crv -> Points;

	if (Points[Axis + 1][0] == t)
	    Inters[NumOfInters++] = Points[2 - Axis][0];
	if (Points[Axis + 1][Len1] == t)
	    Inters[NumOfInters++] = Points[2 - Axis][Len1];
    }

    /* Sort the intersections into order. */
    qsort(Inters, NumOfInters, sizeof(CagdRType), CompareReal);

    if (NumOfInters & 0x01)
	TRIM_FATAL_ERROR(TRIM_ERR_ODD_NUM_OF_INTER);

    for (i = 0; i < NumOfInters; i += 2) {
	if (!IRIT_APX_EQ_EPS(Inters[i], Inters[i + 1], IRIT_UEPS)) {
	    CagdCtlPtStruct Pt1, Pt2;

	    Pt1.PtType = Pt2.PtType = CAGD_PT_E2_TYPE;
	    Pt1.Coords[Axis + 1] = Pt2.Coords[Axis + 1] = t;
	    Pt1.Coords[OAxis + 1] = Inters[i];
	    Pt2.Coords[OAxis + 1] = Inters[i + 1];

	    TCrv = CagdMergeCtlPtCtlPt(&Pt1, &Pt2, 2);
	    Crv = CagdCnvrtBzr2BspCrv(TCrv);
	    CagdCrvFree(TCrv);

	    IRIT_LIST_PUSH(Crv, Crvs);
	}
    }

    IritFree(Inters);

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Classifies the given curve into one of the two subdivided domains.       *
*                                                                            *
* PARAMETERS:                                                                *
*   UVCrv:    UV curve to classify.                                          *
*   t:        UV parameter in direction Axis to use as seperator.            *
*   Axis:     Direction of subdivision - 0 for U, 1 for V.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE for above t, FALSE for below it.                          *
*****************************************************************************/
static int ClassifyTrimCurve(CagdCrvStruct *UVCrv, IrtRType t, int Axis)
{
    int i,
	Len = UVCrv -> Length;
    CagdUVType UVSum, UV;

    UVSum[0] = UVSum[1] = 0;

    for (i = 0; i < Len; i++) {
	CagdCoerceToE2(UV, UVCrv -> Points, i, UVCrv -> PType);
	UVSum[Axis] += UV[Axis];
    }

    return UVSum[Axis] / Len > t;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the parameters on the curve of the intersection points between  *
* Crv and Line                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       To check for intersection(s) with the given line in XY plane. *
*	       Curve is assumed non rational, if linear.		     *
*   Line:      A horizontal or a vertical line in XY plane.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:    List of intersection points, NULL if none.            *
*****************************************************************************/
static CagdPtStruct *InterCrvLine(CagdCrvStruct *Crv, CagdLType Line)
{
    if (Crv -> Order == 2) {
	int i,
	    Len = Crv -> Length;
	CagdRType Inter, t,
	    *KV = Crv -> KnotVector,
	    *Points = IRIT_APX_EQ_EPS(Line[0], 1.0, IRIT_UEPS) ? Crv -> Points[1]
					                  : Crv -> Points[2],
	    PSecond = Points[1];

	t = -Line[2];

	GlblPtList = NULL;

	for (i = 1, Points++; i < Len; i++, Points++) {
	    if ((Points[-1] < t && Points[0] > t) ||
		(Points[-1] < t && Points[0] == t &&
		 (i < Len - 1 ? Points[1] : PSecond) > t)) {
		Inter = (t - Points[-1]) / (Points[0] - Points[-1]);
		Inter = Inter * KV[i + 1] + (1.0 - Inter) * KV[i];
	    }
	    else if ((Points[-1] > t && Points[0] < t) ||
		     (Points[-1] > t && Points[0] == t &&
		      (i < Len - 1 ? Points[1] : PSecond) < t)) {
		Inter = (t - Points[0]) / (Points[-1] - Points[0]);
		Inter = Inter * KV[i] + (1.0 - Inter) * KV[i + 1];
	    }
	    else if (Points[-1] == t && Points[0] == t) {
		/* Insert both end points of the segment to break at. */
		InsertNewParam(KV[i + 1]);
		Inter = KV[i];
	    }
	    else
	        continue;

	    InsertNewParam(Inter);
	}

	return GlblPtList;
    }
    else {
	int Axis;
	CagdRType t, TMin, TMax;
        CagdUVType UV1, UV2;
        CagdPtStruct *Pt, *PtLast,
	    *Pts = SymbLclDistCrvLine(Crv, Line, SUBDIV_TCRV_EPS_HIGHORDER,
				      TRUE, FALSE);

	CagdCrvDomain(Crv, &TMin, &TMax);

	/* Check end points. */
	CagdCoerceToE2(UV1, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE2(UV2, Crv -> Points, Crv -> Length - 1, Crv -> PType);

	Axis = IRIT_APX_EQ_EPS(Line[1], 1.0, IRIT_UEPS);
	t = -UV1[Axis];
	if (IRIT_APX_EQ_EPS(t, Line[2], IRIT_UEPS)) {
	    Pt = CagdPtNew();
	    Pt -> Pt[0] = TMin;
	    Pt -> Pnext = Pts;
	    Pts = Pt;
	}

	t = -UV2[Axis];
	if (IRIT_APX_EQ_EPS(t, Line[2], IRIT_UEPS) &&
	    !IRIT_APX_EQ_EPS(UV1[!Axis], UV2[!Axis], IRIT_UEPS)) {
	    Pt = CagdPtNew();
	    Pt -> Pt[0] = TMax;
	    if (Pts == NULL)
		Pts = Pt;
	    else {
	        PtLast = CagdListLast(Pts);
	        PtLast -> Pnext = Pt;
	    }
	}

	return Pts;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Insert a single t value into existing GlblPtList, provided no equal t      *
* value exists already in the list. List is ordered incrementally.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   t:         New value to insert to global GlblPtList list.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertNewParam(CagdRType t)
{
    CagdPtStruct *PtTmp, *PtLast, *Pt;

    Pt = CagdPtNew();
    Pt -> Pt[0] = t;

    if (GlblPtList) {
	for (PtTmp = GlblPtList, PtLast = NULL;
	     PtTmp != NULL;
	     PtLast = PtTmp, PtTmp = PtTmp -> Pnext) {
	    if (PtTmp -> Pt[0] == t) {
	        IritFree(Pt);
		return;
	    }
	    if (PtTmp -> Pt[0] > t)
	        break;
	}
	if (PtTmp) {
	    /* Insert the new point in the middle of the list. */
	    Pt -> Pnext = PtTmp;
	    if (PtLast)
		PtLast -> Pnext = Pt;
	    else
		GlblPtList = Pt;
	}
	else {
	    /* Insert the new point as the last point in the list. */
	    PtLast -> Pnext = Pt;
	}
    }
    else
        GlblPtList = Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans the given list of curves and purge away too small curves, or       *
* curves that are on the split axis, Axis.  Purge in place.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:   List of curves to scan and purge too small ones, in place.       *
*   Axis:   Axis that is considered (either 0 for U or 1 for V).             *
*   t:      Parameter value of surface at which the trimming curve is split. *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Result.  Identical to the returned list of valid       *
*		curves, or NULL if no curves returned.			     *
*****************************************************************************/
static CagdCrvStruct *FreeTooSmallCollinearCurves(CagdCrvStruct **Crvs,
						  int Axis,
						  CagdRType t)
{
    CagdCrvStruct
	*RetCrvs = NULL;

    while (*Crvs) {
	CagdCrvStruct
	    *Crv = *Crvs;

	*Crvs = (*Crvs) -> Pnext;

	if (CagdCrvArcLenPoly(Crv) < IRIT_UEPS) {
	    CagdCrvFree(Crv);
	}
	else {
	    int i;
	    CagdRType
		*Pts = Crv -> Points[Axis + 1];

	    for (i = 0; i < Crv -> Length; i++, Pts++) {
	        if (!IRIT_APX_EQ_EPS(*Pts, t, IRIT_UEPS))
		    break;
	    }

	    if (i < Crv -> Length)
	        IRIT_LIST_PUSH(Crv, RetCrvs)
	    else
	        CagdCrvFree(Crv);
	}
    }

    *Crvs = RetCrvs;

    return RetCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes but not delete the given trimming crv segment from the list of   M
* trimming curves point by TrimCrvs.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSeg:     Segment to delete.                                       M
*   TrimCrvs:       List of trimming curves to delete TrimCrvSeg from.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if found and removed, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimRemoveCrvSegTrimCrvSegs                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimRemoveCrvSegTrimCrvs                                                 M
*****************************************************************************/
int TrimRemoveCrvSegTrimCrvs(TrimCrvSegStruct *TrimCrvSeg,
			     TrimCrvStruct **TrimCrvs)
{
    TrimCrvStruct *TrimCrv, *TrimCrvAux;

    if (*TrimCrvs == NULL)
	return FALSE;

    if (TrimRemoveCrvSegTrimCrvSegs(TrimCrvSeg,
				    &(*TrimCrvs) -> TrimCrvSegList) &&
	(*TrimCrvs) -> TrimCrvSegList == NULL) {
	/* Remove the TrimCrvStruct as well. */
	TrimCrvAux = *TrimCrvs;
	*TrimCrvs = (*TrimCrvs) -> Pnext;
	TrimCrvFree(TrimCrvAux);
	return TRUE;
    }

    for (TrimCrv = *TrimCrvs;
	 TrimCrv -> Pnext != NULL;
	 TrimCrv = TrimCrv -> Pnext) {
	if (TrimRemoveCrvSegTrimCrvSegs(TrimCrvSeg,
					&TrimCrv -> Pnext -> TrimCrvSegList) &&
	    TrimCrv -> Pnext -> TrimCrvSegList == NULL) {
	    /* Remove the TrimCrvStruct as well. */
	    TrimCrvAux = TrimCrv -> Pnext;
	    TrimCrv -> Pnext = TrimCrv -> Pnext -> Pnext;
	    TrimCrvFree(TrimCrvAux);
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes but not delete the given trimming crv segment from the list of   M
* trimming curve segments pointed by TrimCrvSegs.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrvSeg:   Segment to delete.                                         M
*   TrimCrvSegs:  List of trimming curve segments to delete TrimCrvSeg from. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if found and removed, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimRemoveCrvSegTrimCrvs                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimRemoveCrvSegTrimCrvSegs                                              M
*****************************************************************************/
int TrimRemoveCrvSegTrimCrvSegs(TrimCrvSegStruct *TrimCrvSeg,
				TrimCrvSegStruct **TrimCrvSegs)
{
    if (*TrimCrvSegs == TrimCrvSeg) {
	*TrimCrvSegs = (*TrimCrvSegs) -> Pnext;
	return TRUE;
    }
    else {
	TrimCrvSegStruct *Segs;

	for (Segs = *TrimCrvSegs; Segs -> Pnext != NULL; Segs = Segs -> Pnext) {
	    if (Segs -> Pnext == TrimCrvSeg) {
		Segs -> Pnext = TrimCrvSeg -> Pnext;
		return TRUE;
	    }
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Completely delete TrimCrvs if recognize it as a zero area.               *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimCrvs:   To examine for a zero area trimming curve.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FiltersZeroAreaTrims(TrimCrvStruct **TrimCrvs)
{
    CagdCrvStruct *UVCrv;
    CagdUVType UVRef;
    CagdBType
	SameU = TRUE,
	SameV = TRUE;
    TrimCrvStruct *TrimCrv;

    if (*TrimCrvs == NULL)
	return;

    UVCrv = (*TrimCrvs) -> TrimCrvSegList -> UVCrv;
    CagdCoerceToE2(UVRef, UVCrv -> Points, 0, UVCrv -> PType);

    for (TrimCrv = *TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    CagdUVType UV;
	    int i;

	    UVCrv = TrimCrvSeg -> UVCrv;

	    /* If not the same U or same V value - mark it. */
	    for (i = 0; i < UVCrv -> Length; i++) {
		CagdCoerceToE2(UV, UVCrv -> Points, i, UVCrv -> PType);
		SameU &= IRIT_APX_EQ_EPS(UVRef[0], UV[0], IRIT_UEPS);
		SameV &= IRIT_APX_EQ_EPS(UVRef[1], UV[1], IRIT_UEPS);
	    }
	}
    }

    if (SameU || SameV) {
	TrimCrvFreeList(*TrimCrvs);
	*TrimCrvs = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts a new intersection location into intersections array.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Inters:       Array of intersection locations.                           *
*   NumOfInters:  Number of current intersections in Inters.                 *
*   r:            New intersection location to insert.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrimSubAddInterLoc(CagdRType *Inters,
			       int *NumOfInters,
			       CagdRType r)
{
    Inters[(*NumOfInters)++] = r;
    if (*NumOfInters >= MAX_NUM_OF_INTERS)
	TRIM_FATAL_ERROR(TRIM_ERR_TRIM_TOO_COMPLEX);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits one trimming curve at the t value in axis Axis. The split pieces  *
* are placed in either UVCrv1 (< t) or UVCrv2 (>t) while being merged into   *
* new loops.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   UVCrv:   Curve to split at t in axis Axis.                               *
*   Pts:     Parameter values at which to split the UVCrv curve.             *
*   Axis:    Axis that is considered (either 0 for U or 1 for V).            *
*   t:       Parameter value of surface at which the trimming curve is split.*
*   UVCrv1:  To hold the <t side of the trimming curve.                      *
*   UVCrv2:  To hold the >t side of the trimming curve.                      *
*   NumOfInters:  Number of intersections.  Typically even at the end.       *
*   Inters:  Intersection values along the splitting line in axis Axis       *
*	     of value t.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SplitTrimmingCurve(CagdCrvStruct *UVCrv,
			       CagdPtStruct *Pts,
			       int Axis,
			       CagdRType t,
			       CagdCrvStruct **UVCrv1,
			       CagdCrvStruct **UVCrv2,
			       int *NumOfInters,
			       CagdRType *Inters)
{
    int Axis1 = Axis + 1,
	OAxis = 1 - Axis;
    CagdRType r;
    CagdPtStruct *Pt;

    *UVCrv1 = *UVCrv2 = NULL;
    UVCrv = CagdCrvCopy(UVCrv);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	CagdRType TMin, TMax;
	CagdUVType UV;
	CagdCrvStruct *Crv1, *Crv2;

	CagdCrvDomain(UVCrv, &TMin, &TMax);
	if (TMin == Pt -> Pt[0]) {
	    CagdCoerceToE2(UV, UVCrv -> Points, 0, UVCrv -> PType);
	    TrimSubAddInterLoc(Inters, NumOfInters, UV[OAxis]);
	    continue;
	}
	else if (TMax == Pt -> Pt[0]) {
	    CagdCoerceToE2(UV, UVCrv -> Points, UVCrv -> Length - 1,
			   UVCrv -> PType);
	    TrimSubAddInterLoc(Inters, NumOfInters, UV[OAxis]);
	    continue;
	}

	Crv1 = CagdCrvSubdivAtParam(UVCrv, Pt -> Pt[0]);
	Crv2 = Crv1 -> Pnext;
	Crv1 -> Pnext = NULL;

	/* Make sure intersection point is exactly on the line. */
	if (CAGD_IS_RATIONAL_CRV(UVCrv)) {
	    Crv1 -> Points[Axis1][Crv1 -> Length - 1] =
				    t * Crv1 -> Points[0][Crv1 -> Length - 1];
	    Crv2 -> Points[Axis1][0] = t * Crv2 -> Points[0][0];
	    r = Crv2 -> Points[OAxis + 1][0] / Crv2 -> Points[0][0];
	}
	else {
	    Crv1 -> Points[Axis1][Crv1 -> Length - 1] = t;
	    Crv2 -> Points[Axis1][0] = t;
	    r = Crv2 -> Points[OAxis + 1][0];
	}
	TrimSubAddInterLoc(Inters, NumOfInters, r);

	/* Classify the segment. */
	if (ClassifyTrimCurve(Crv1, t, Axis)) {
	    Crv1 -> Pnext = *UVCrv2;
	    *UVCrv2 = Crv1;
	}
	else {
	    Crv1 -> Pnext = *UVCrv1;
	    *UVCrv1 = Crv1;
	}

	CagdCrvFree(UVCrv);
	UVCrv = Crv2;
    }

    /* Classify the last segment. */
    if (ClassifyTrimCurve(UVCrv, t, Axis)) {
        UVCrv -> Pnext = *UVCrv2;
	*UVCrv2 = UVCrv;
    }
    else {
        UVCrv -> Pnext = *UVCrv1;
	*UVCrv1 = UVCrv;
    }

    if (CAGD_IS_BSPLINE_CRV(UVCrv) && UVCrv -> Order == 2) {
        CagdCrvStruct *Crv;

	for (Crv = *UVCrv1; Crv != NULL; Crv = Crv -> Pnext)
	    BspKnotUniformOpen(Crv -> Length, Crv -> Order,
			       Crv -> KnotVector);
	for (Crv = *UVCrv2; Crv != NULL; Crv = Crv -> Pnext)
	    BspKnotUniformOpen(Crv -> Length, Crv -> Order,
			       Crv -> KnotVector);
    }
}
