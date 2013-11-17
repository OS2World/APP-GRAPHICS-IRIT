/******************************************************************************
* Crv_lenv.c - lower envelop of curve arrangments.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, may. 2005.					      *
******************************************************************************/

#include "cagd_lib.h"
#include "symb_loc.h"

IRIT_STATIC_DATA int
    GlblLowEnvRadial = FALSE;

#if defined(ultrix) && defined(mips)
static int LowerEnvEventSortCmpr(VoidPtr VPt1, VoidPtr VPt2);
#else
static int LowerEnvEventSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to polygons.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two polygons.            *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int LowerEnvEventSortCmpr(VoidPtr VPt1, VoidPtr VPt2)
#else
static int LowerEnvEventSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
	*Pt1 = (CagdRType *) VPt1,
	*Pt2 = (CagdRType *) VPt2;

    if (GlblLowEnvRadial)
        return IRIT_SIGN(Pt1[2] - Pt2[2]);
    else
        return IRIT_SIGN(Pt1[0] - Pt2[0]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Split the given list of curves at the extremum values of each curve, if  M
* any.                                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrvs:  Input list of curves to split at all extremum values the curves  M
*	    assumes.					                     M
*   Axis:   Extremum to consider:			                     M
*	    0 - Radials silhouette as viewed from Pt.                        M
*	    1,2 - Look for extremum (silhouette) in X,Y dir.                 M
*   Pt:     If radial silhouette are sought, use this as Eye location.       M
*   Eps:    Tolerance of computations.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of splitted curves.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSplitCrvsAtExtremums                                                 M
*****************************************************************************/
CagdCrvStruct *SymbSplitCrvsAtExtremums(const CagdCrvStruct *CCrvs,
					int Axis,
					const CagdPType Pt,
					CagdRType Eps)
{
    CagdLType Line;
    CagdCrvStruct *Crv,
	*SplittedCrvs = NULL;
    CagdBBoxStruct BBox;
    CagdCrvStruct
        *Crvs = CagdCrvCopyList(CCrvs);

    if (Axis == 1 || Axis == 2) {
        /* Create a line in the proper orienation and location, so it       */
        /* intersect no curve in the data.				    */

        CagdCrvListBBox(Crvs, &BBox);
	switch (Axis) {
            case 1:
	        Line[0] = 1;
		Line[1] = 0;
		Line[2] = BBox.Min[0] - 1.0;
		break;
            case 2:
	        Line[0] = 0;
		Line[1] = 1;
		Line[2] = BBox.Min[1] - 1.0;
		break;
	}
    }

    while (Crvs) {
        CagdPtStruct *ExtremPts;

	IRIT_LIST_POP(Crv, Crvs);

	/* Compute extreme locations of Crv. */
	switch (Axis) {
	    case 0:
	        ExtremPts = SymbCrvPtTangents(Crv, Pt, Eps);
		break;
	    case 1:
	    case 2:
	        ExtremPts =  SymbLclDistCrvLine(Crv, Line, Eps, FALSE, TRUE);
		break;
	    default:
	        SYMB_FATAL_ERROR(SYMB_ERR_INVALID_AXIS);
		return NULL;
	}

	if (ExtremPts != NULL) {
	    int Proximity;
	    CagdCrvStruct *SplittedCrv;

	    /* Split Crv at the extremums. */
	    SplittedCrv = CagdCrvSubdivAtParams(Crv, ExtremPts, Eps * 10,
						&Proximity);
	    SplittedCrvs = CagdListAppend(SplittedCrv, SplittedCrvs);
	    CagdPtFreeList(ExtremPts);
	    CagdCrvFree(Crv);
	}
	else
	    IRIT_LIST_PUSH(Crv, SplittedCrvs);
    }

    return SplittedCrvs;	
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the lower envelop in the plane of all given curves.  Returned   M
* is a list of (pieces of) curves that forms the lower envelop.              M
*   If Pt is not NULL, radial lower envelop around Pt is computed.           M
* If Pt is NULL, regular, linear, lower envelop is computed seeking minimum  M
* Y values for the X domain that is spanned by the curves.		     M
*   Note the lower envelop might be discontinuous.			     M
*   The lower envelop is computed by splitting the input curves at all       M
* intersetion locations, sorting intersection and end point events and       M
* shooting rays in the middle of these intervals to determine lowest one.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrvs:      Curves to derive the lower envelop for.			     M
*   Pt:         Point defining the center of the radial envelop, or NULL     M
*		for linear, minimum Y, lower envelop.			     M
*   Eps:        Tolerance of computations.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of curves defining the lower envelop.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvCrvInter, CagdCrvCrvInterArrangment		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvsLowerEnvelop, cci, lower envelop                                 M
*****************************************************************************/
CagdCrvStruct *SymbCrvsLowerEnvelop(const CagdCrvStruct *CCrvs,
				    CagdRType *Pt,
				    CagdRType Eps)
{
    int i, j, CmpIdx, NumOfEvents;
    CagdPType *Events;
    CagdCrvStruct *Crv, *Crvs, *TCrvs, **CrvsBuckets,
	*LowEnvCrvs = NULL;

    GlblLowEnvRadial = Pt != NULL;

    /* Subdivide the curves at all intersection and all extremum location. */
    Crvs = CagdCrvCrvInterArrangment(CCrvs, TRUE, Eps);
    TCrvs = SymbSplitCrvsAtExtremums(Crvs, GlblLowEnvRadial ? 0 : 1, Pt, Eps);
    CagdCrvFreeList(Crvs);
    Crvs = TCrvs;

    NumOfEvents = CagdListLength(Crvs) * 2;

    /* Accumulate all end points as events, and sort in order. */
    Events = (CagdPType *) IritMalloc(sizeof(CagdPType) * (NumOfEvents + 2));
    for (i = -1, Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdRType *R, TMin, TMax;

	CagdCrvDomain(Crv, &TMin, &TMax);

	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE2(Events[++i], &R, -1, Crv -> PType);
	if (GlblLowEnvRadial)
	    IRIT_PT2D_SUB(Events[i], Events[i], Pt);
  
	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE2(Events[++i], &R, -1, Crv -> PType);
	if (GlblLowEnvRadial)
	    IRIT_PT2D_SUB(Events[i], Events[i], Pt);
    }

    if (GlblLowEnvRadial) {
        /* Convert all vectors to angles and keep in the Z axis. */
        for (i = 0; i < NumOfEvents; i++) {
	    IRIT_PT2D_NORMALIZE(Events[i]);
	    /* Map the atan2 [-Pi, +Pi] domain to monotone [0, 2Pi]. */
	    if ((Events[i][2] = atan2(Events[i][1], Events[i][0])) < 0.0)
		Events[i][2] += M_PI_MUL_2;
	}

	/* Place minimum and maximum radial events, at 0 and 360 degress. */
	Events[NumOfEvents][0] = 1.0;
	Events[NumOfEvents][1] = 0.0;
	Events[NumOfEvents][2] = 0.0;
	NumOfEvents++;
	Events[NumOfEvents][0] = 1.0;
	Events[NumOfEvents][1] = 0.0;
	Events[NumOfEvents][2] = 360.0;
	NumOfEvents++;
    }
    else {
        for (i = 0; i < NumOfEvents; i++)
	    Events[i][2] = 0.0;
    }

    /* Sort the events and filter out duplicated events. */
    qsort(Events, NumOfEvents, sizeof(CagdPType), LowerEnvEventSortCmpr);
    for (i = j = 0; i < NumOfEvents; i++) {
        if (!IRIT_PT_APX_EQ_EPS(Events[j], Events[i], Eps * 10)) {
	    j++;
	    IRIT_PT_COPY(Events[j], Events[i]);
	}
    }
    NumOfEvents = ++j;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugLowEnvPrintEvents, FALSE) {
	    for (i = 0; i < NumOfEvents; i++) {
	        IRIT_INFO_MSG_PRINTF("Event %3d = %16.14lg %16.14lg %16.14lg\n",
				  i, Events[i][0], Events[i][1], Events[i][2]);
	    }
	}
    }
#   endif /* DEBUG */

    /* Go over events and split all curves further along the event lines.   */
    /* No need to split both at 0 degrees and 360 degrees. Ignore latter.   */
    if (GlblLowEnvRadial)
	NumOfEvents--;
    for (i = 0; i < NumOfEvents; i++) {
	CagdVType Dir;
        CagdLType Line;
	CagdCrvStruct *SplittedCrv, *SplittedCrvs;

	if (GlblLowEnvRadial) { /* Radial line from Pt to curves' end point. */
	    IRIT_PT2D_COPY(Line, Events[i]);
	    IRIT_PT2D_NORMALIZE(Line);
	    IRIT_PT2D_COPY(Dir, Line);
	    IRIT_SWAP(CagdRType, Line[0], Line[1]);
	    Line[1] = -Line[1];
	    Line[2] = -IRIT_DOT_PROD_2D(Line, Pt);
	}
	else {                          /* Vertical line of the form X = X0. */
	    Line[0] = 1;
	    Line[1] = 0;
	    Line[2] = -Events[i][0];
	    IRIT_PT2D_RESET(Dir);
	}

	SplittedCrvs = NULL;
	while (Crvs) {
	    CagdPtStruct *InterPts, *IPts;

	    IRIT_LIST_POP(Crv, Crvs);

	    if ((InterPts = SymbLclDistCrvLine(Crv, Line, Eps, TRUE, FALSE))
								    != NULL) {
	        if (GlblLowEnvRadial) {
		    /* Filter out intersections behind the origin, in -Dir. */
		    for (IPts = InterPts; IPts != NULL; IPts = IPts -> Pnext) {
		        CagdPType PtE2;
		        CagdRType
			    *R = CagdCrvEval(Crv, IPts -> Pt[0]);

			CagdCoerceToE2(PtE2, &R, -1, Crv -> PType);
			IRIT_VEC2D_SUB(PtE2, PtE2, Pt);
			if (IRIT_DOT_PROD_2D(Dir, PtE2) < 0.0) {
			    /* Behind origin of view - mark to purge. */
			    IPts -> Pt[0] = IRIT_INFNTY;
			}
		    }
		    /* And purge all invalidated parameters. */
		    InterPts = CagdPtsSortAxis(InterPts, 1);
		    if (InterPts -> Pt[0] == IRIT_INFNTY) {
		        CagdPtFreeList(InterPts);
			InterPts = NULL;
		    }
		    else {
		        for (IPts = InterPts;
			     IPts -> Pnext != NULL;
			     IPts = IPts -> Pnext) {
			    if (IPts -> Pnext -> Pt[0] == IRIT_INFNTY) {
			        CagdPtFreeList(IPts -> Pnext);
				IPts -> Pnext = NULL;
			    }
			}
		    }
		}

		if (InterPts != NULL) {
		    int Proximity;

		    /* Split Crv at the intersection locations. */
		    SplittedCrv = CagdCrvSubdivAtParams(Crv, InterPts,
							Eps * 10, &Proximity);
		    SplittedCrvs = CagdListAppend(SplittedCrv, SplittedCrvs);
		    CagdPtFreeList(InterPts);
		    CagdCrvFree(Crv);
		}
		else
		    IRIT_LIST_PUSH(Crv, SplittedCrvs);
	    }
	    else
	        IRIT_LIST_PUSH(Crv, SplittedCrvs);
	}
	Crvs = SplittedCrvs;
    }
    /* Restore number of events. */
    if (GlblLowEnvRadial)
	NumOfEvents++;

    /* Place all Crvs in the proper buckets as created by splitting lines. */
    CrvsBuckets = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *)
						               * NumOfEvents);
    IRIT_ZAP_MEM(CrvsBuckets, sizeof(CagdCrvStruct *) * NumOfEvents);

    CmpIdx = GlblLowEnvRadial ? 2 : 0;
    while (Crvs) {
        int MinIdx, MaxIdx, MidIdx;
        CagdRType *R, TMin, TMax;
	CagdPType PtMid;

        IRIT_LIST_POP(Crv, Crvs);

	CagdCrvDomain(Crv, &TMin, &TMax);

	R = CagdCrvEval(Crv, (TMin + TMax) * 0.5);
	CagdCoerceToE2(PtMid, &R, -1, Crv -> PType);
	if (GlblLowEnvRadial) {
	    IRIT_PT2D_SUB(PtMid, PtMid, Pt);
	    IRIT_PT2D_NORMALIZE(PtMid);
	    /* Map the atan2 [-Pi, +Pi] domain to monotone [0, 2Pi]. */
	    if ((PtMid[2] = atan2(PtMid[1], PtMid[0])) < 0.0)
		PtMid[2] += M_PI_MUL_2;
	}

	/* Use bisection search to find the proper backet for this Crv. */
	MinIdx = 0;
	MaxIdx = NumOfEvents;
	do {
	    MidIdx = ((MinIdx + MaxIdx) >> 1);

	    if (Events[MidIdx][CmpIdx] > PtMid[CmpIdx])
	        MaxIdx = MidIdx;
	    else
	        MinIdx = MidIdx;
	}
	while (MaxIdx - MinIdx > 1);

	IRIT_LIST_PUSH(Crv, CrvsBuckets[MinIdx]);
    }

    /* Pick lowest crv from each bucket, if any, and free other segments. */
    for (i = 0; i < NumOfEvents - 1; i++) {
        CagdRType MinDist;
        CagdLType Line;
	CagdCrvStruct
	    *LowEnvCrv = NULL;

	if (CrvsBuckets[i] == NULL)
	    continue;

	if (Pt != NULL) {       /* Radial line from Pt to curves' mid point. */
	    IRIT_PT2D_COPY(Line, Events[i]);
	    IRIT_PT2D_ADD(Line, Line, Events[i + 1]);
	    IRIT_PT2D_NORMALIZE(Line);
	    IRIT_SWAP(CagdRType, Line[0], Line[1]);
	    Line[1] = -Line[1];
	    Line[2] = -IRIT_DOT_PROD_2D(Line, Pt);
	}
	else {                          /* Vertical line of the form X = X0. */
	    Line[0] = 1;
	    Line[1] = 0;
	    Line[2] = -(Events[i][0] + Events[i + 1][0]) * 0.5;
	}

	MinDist = IRIT_INFNTY;
	for (Crv = CrvsBuckets[i]; Crv != NULL; Crv = Crv -> Pnext) {
	    CagdPtStruct *InterPts, *IPt;

	    if ((InterPts = SymbLclDistCrvLine(Crv, Line, Eps, TRUE, FALSE))
								    != NULL) {
		for (IPt = InterPts; IPt != NULL; IPt = IPt -> Pnext) {
		    CagdRType Dist,
		        *R = CagdCrvEval(Crv, IPt -> Pt[0]);
		    CagdPType PtE2;

		    CagdCoerceToE2(PtE2, &R, -1, Crv -> PType);
		    Dist = GlblLowEnvRadial ? IRIT_PT2D_DIST_SQR(Pt, PtE2)
					    : PtE2[1];
		    if (MinDist > Dist) {
		        MinDist = Dist;
		        LowEnvCrv = Crv;
		    }
		}
		CagdPtFreeList(InterPts);
	    }
	}

	if (LowEnvCrv != NULL) {
	    LowEnvCrv = CagdCrvCopy(LowEnvCrv);
	    IRIT_LIST_PUSH(LowEnvCrv, LowEnvCrvs);
	}

	CagdCrvFreeList(CrvsBuckets[i]);
    }

    IritFree(Events);
    IritFree(CrvsBuckets);

    return LowEnvCrvs;
}

