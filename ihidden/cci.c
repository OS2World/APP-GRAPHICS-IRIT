/*****************************************************************************
*   Computes all curve curve intersections in the plane and split as needed. *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan. 2000   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"

#define IHID_SIL_MAX_SHARP_ANGLE	0.707		  /* ~45 degrees. */
#define IHID_CCI_SAME_EPS 1e-3
#define IHID_COPY_ORIG_ATTR(Dest, Src) { \
    (Dest) -> Attr = IP_ATTR_COPY_ATTRS((Src) -> Attr); \
    AttrSetPtrAttrib(&(Dest) -> Attr, "_OrigObj", \
		     AttrGetPtrAttrib((Src) -> Attr, "_OrigObj")); \
    AttrSetPtrAttrib(&(Dest) -> Attr, "_Uv", \
		     AttrGetPtrAttrib((Src) -> Attr, "_Uv")); \
}
		      
static CagdCrvStruct *SplitSilAtCusps(CagdCrvStruct *Sil);
static CagdCrvStruct *CCIOneAgainstActive(CagdCrvStruct *Crv,
					  CagdCrvStruct *ActiveCurves,
					  CagdRType CCITol,
					  int CrvActive);
static void InsertInterPoints(CagdRType t, CagdPtStruct **SplitLst);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all intersections in the plane between the given curves.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:    Curves to intersect.  The curves will have "ctype" attributes   M
*	     that defines the curve type (one of 'iso', 'bndry', 'sil', and  M
*	     'discont', an attribute that will be propagated further.        M
*   CCITol:  Tolerance of CCI computations.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Sub curves as splitted due to cci's.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCrvIntersections                                                      M
*****************************************************************************/
CagdCrvStruct *CrvCrvIntersections(CagdCrvStruct *Crvs, CagdRType CCITol)
{
    int i;
    CagdCrvStruct *Crv, *PCrv, *ACrv, *TmpCrvs,
	*ActiveCurves = NULL,
        *PassiveCurves = NULL;

    /* Decompose the curves' list into passive and active curves. */ 
    while (Crvs != NULL) {
        int CType = AttrGetIntAttrib(Crvs -> Attr, "ctype");

	IRIT_LIST_POP(Crv, Crvs);

	switch (CType) {
	    case IHID_CURVE_BOUNDARY:
	    case IHID_CURVE_SILHOUETTE:
		IRIT_LIST_PUSH(Crv, ActiveCurves);
		break;
	    case IHID_CURVE_DISCONT:
	    case IHID_CURVE_ISOPARAM:
		IRIT_LIST_PUSH(Crv, PassiveCurves);
		break;
	    default:
		IRIT_WARNING_MSG_PRINTF("Undefined curve type %d ignored\n",
				        CType);
		break;
	}
    }

    /* Intersect all passive curves against the active curves. */
    TmpCrvs = NULL;
    if (!GlblQuiet)
        IRIT_INFO_MSG("Passive:      \b");
    i = 1;
    while (PassiveCurves != NULL) {
	IRIT_LIST_POP(PCrv, PassiveCurves);

	if (!GlblQuiet)
	    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", i++);

	TmpCrvs = CagdListAppend(CCIOneAgainstActive(PCrv, ActiveCurves,
						     CCITol, FALSE),
				 TmpCrvs);
    }
    PassiveCurves = TmpCrvs;

    /* Split active silhouette curves at cusps in screen space. */
    TmpCrvs = NULL;
    while (ActiveCurves != NULL) {
	IRIT_LIST_POP(ACrv, ActiveCurves);

	if (AttrGetIntAttrib(ACrv -> Attr, "ctype") == IHID_CURVE_SILHOUETTE) {
	    TmpCrvs = CagdListAppend(SplitSilAtCusps(ACrv), TmpCrvs);
	}
	else {
	    IRIT_LIST_PUSH(ACrv, TmpCrvs);
	}
    }
    ActiveCurves = TmpCrvs;

    /* Intersect all active curves against themselves. */
    TmpCrvs = NULL;
    if (!GlblQuiet)
        IRIT_INFO_MSG(", Active:      \b");
    i = 1;
    while (ActiveCurves != NULL) {
        CagdPtStruct *ACrvSplitLst;
	CagdCrvStruct
	    *ACrvLst = NULL;

	IRIT_LIST_POP(ACrv, ActiveCurves);

	if (!GlblQuiet)
	    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", i++);

	/* Do we need to split this active curve first? */
	if ((ACrvSplitLst =
		     (CagdPtStruct *) AttrGetPtrAttrib(ACrv -> Attr,
						       "_SplitPts")) != NULL) {
	    CagdCrvStruct *ACrv1, *ACrv2;
	    CagdPtStruct *Pt;

	    /* Split ACrv at all locations specified in the split list. */
	    for (Pt = ACrvSplitLst; Pt != NULL; Pt = Pt -> Pnext) {
	        if (CCIUpdateSubdivCrvs(ACrv, Pt -> Pt[0], &ACrv1, &ACrv2)) {
		    CagdCrvFree(ACrv);
		    IRIT_LIST_PUSH(ACrv1, ACrvLst);
		    ACrv = ACrv2;
		}
	    }
	    IRIT_LIST_PUSH(ACrv, ACrvLst);
	    CagdPtFreeList(ACrvSplitLst);
	}
	else
	    ACrvLst = ACrv;

	while (ACrvLst != NULL) {
	    IRIT_LIST_POP(ACrv, ACrvLst);

	    TmpCrvs = CagdListAppend(CCIOneAgainstActive(ACrv, ActiveCurves,
							 CCITol, TRUE),
				     TmpCrvs);
	}
    }
    ActiveCurves = TmpCrvs;

    return CagdListAppend(ActiveCurves, PassiveCurves);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split the given silhouette curve at its cusps location in screen space.  *
* This split is performed as in these cusps location, the silhouette might   *
* change its visibility.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Sil:      Silhouette curve to split at screen cusps, in place.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:    Splitted curve (or original if no cusps).            *
*****************************************************************************/
static CagdCrvStruct *SplitSilAtCusps(CagdCrvStruct *Sil)
{
    int i;
    CagdRType TMin, TMax, Dt,
	**Points = Sil -> Points;
    CagdVType V1, V2;
    CagdCrvStruct
	*SplitSils = NULL;

    CagdCrvDomain(Sil, &TMin, &TMax);
    Dt = TMax - TMin;

    V2[0] = Points[1][1] - Points[1][0];
    V2[1] = Points[2][1] - Points[2][0];
    IRIT_VEC2D_SAFE_NORMALIZE(V2);

    for (i = 2; i < Sil -> Length; i++) {
        IRIT_PT2D_COPY(V1, V2);
	V2[0] = Points[1][i] - Points[1][i - 1];
	V2[1] = Points[2][i] - Points[2][i - 1];
	IRIT_VEC2D_SAFE_NORMALIZE(V2);

	if (IRIT_DOT_PROD_2D(V1, V2) < IHID_SIL_MAX_SHARP_ANGLE) {
	    CagdCrvStruct *Sil1, *Sil2;

	    /* Split silhouette at sharp corners. */
	    if (CCIUpdateSubdivCrvs(Sil,
				    TMin + Dt * (i - 1.0) / Sil -> Length,
				    &Sil1, &Sil2)) {
	        CagdCrvFree(Sil);
	        IRIT_LIST_PUSH(Sil1, SplitSils);
	        Sil = Sil2;

	        Points = Sil -> Points;
	        CagdCrvDomain(Sil, &TMin, &TMax);
	        Dt = TMax - TMin;
	        i = IRIT_MIN(2, Sil -> Length);
	    }
	}
    }

    IRIT_LIST_PUSH(Sil, SplitSils);

    return SplitSils;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Breaks the given curve Crv at all locations it intersects in the XY      *
* plane with a curve in the ActiveList and ActiveList curve is closer.       *
*   If Crv is an active curve (CrvActive TRUE), ActiveCurves it intersects   *
* with might be splitted as well as a side effect.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:           To intersect and break against the active list.           *
*   ActiveCurves:  List of active curves in the scene.			     *
*   CCITol:        Tolerance of CCI computations.			     *
*   CrvActive:     TRUE if Crv an active curve, FALSE otherwise.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  List of sub curves of Crv as it was splitted.          *
*****************************************************************************/
static CagdCrvStruct *CCIOneAgainstActive(CagdCrvStruct *Crv,
					  CagdCrvStruct *ActiveCurves,
					  CagdRType CCITol,
					  int CrvActive)
{
    CagdCrvStruct *ACrv,
	*CrvSplits = NULL;
    int MdlID1 = AttrGetIntAttrib(Crv -> Attr, "MdlID");

    for (ACrv = ActiveCurves; ACrv != NULL; ACrv = ACrv -> Pnext) {
	CagdPtStruct *IPts, *InterPts;

	if (!IP_ATTR_IS_BAD_INT(MdlID1)) {
	    int MdlID2 = AttrGetIntAttrib(ACrv -> Attr, "MdlID");

	    /* These are trimming curves that share an edge - skip. */
	    if (!IP_ATTR_IS_BAD_INT(MdlID1) && MdlID1 == MdlID2)
	        continue;
	}

        MdlID1 = AttrGetIntAttrib(Crv -> Attr, "MdlID");

	InterPts = CagdCrvCrvInter(Crv, ACrv, CCITol);

	if (InterPts != NULL) {
	    CagdRType TMin1, TMax1, TMin2, TMax2;
	    CagdPtStruct
		*ACrvSplitLst = (CagdPtStruct *) AttrGetPtrAttrib(ACrv -> Attr,
								  "_SplitPts");
	    CagdCrvDomain(Crv, &TMin1, &TMax1);
	    CagdCrvDomain(ACrv, &TMin2, &TMax2);
	    
	    for (IPts = InterPts; IPts != NULL; IPts = IPts -> Pnext) {
		CagdRType *R, Z1, Z2;
		CagdPType PtE3;

		if (IPts -> Pt[0] < TMin1 || IPts -> Pt[0] > TMax1)
		    continue;

		R = CagdCrvEval(Crv, IPts -> Pt[0]);
		CagdCoerceToE3(PtE3, &R, -1, Crv -> PType);
		Z1 = PtE3[2];

		R = CagdCrvEval(ACrv, IPts -> Pt[1]);
		CagdCoerceToE3(PtE3, &R, -1, ACrv -> PType);
		Z2 = PtE3[2];

		/* Split Crv if active curve is closer and intersection      */
		/* location is not too close to the domain boundary.         */
	        if (Z1 < Z2 + IHID_CCI_SAME_EPS &&
		    !IRIT_APX_EQ_EPS(TMin1, IPts -> Pt[0], IHID_CCI_SAME_EPS) &&
		    !IRIT_APX_EQ_EPS(TMax1, IPts -> Pt[0], IHID_CCI_SAME_EPS)) {
		    CagdCrvStruct *Crv1, *Crv2;

		    /* Split Crv here. */
		    if (CCIUpdateSubdivCrvs(Crv, IPts -> Pt[0],
					    &Crv1, &Crv2)) {
		        IRIT_LIST_PUSH(Crv1, CrvSplits);
			CagdCrvFree(Crv);
			Crv = Crv2;
			CagdCrvDomain(Crv, &TMin1, &TMax1);
		    }
		}

		/* Mark ACrv to be splitted if Crv is an active curve that   */
		/* is closer and intersection location is not too close to   */
		/* the boundary of the domain.				     */
		if (CrvActive &&
		    Z1 + IHID_CCI_SAME_EPS > Z2 &&
		    !IRIT_APX_EQ_EPS(TMin2, IPts -> Pt[1], IHID_CCI_SAME_EPS) &&
		    !IRIT_APX_EQ_EPS(TMax2, IPts -> Pt[1], IHID_CCI_SAME_EPS)) {
		    InsertInterPoints(IPts -> Pt[1], &ACrvSplitLst);
		}
	    }

	    /* Update the point split list of this active curve. */
	    AttrSetPtrAttrib(&ACrv -> Attr, "_SplitPts", ACrvSplitLst);

	    if (CrvSplits != NULL) {
	        CagdCrvStruct
		    *CrvRes = NULL;

		/* Crv was subdivided - invoke recursively on all pieces     */
		/* against the rest of the active list.		             */
		IRIT_LIST_PUSH(Crv, CrvSplits);

	        while (CrvSplits != NULL) {
		    CagdCrvStruct *TCrvs;

		    IRIT_LIST_POP(Crv, CrvSplits);

		    TCrvs = CCIOneAgainstActive(Crv,
						ACrv -> Pnext,
						CCITol,
						CrvActive);

		    CrvRes = CagdListAppend(TCrvs, CrvRes);
		}

		return CrvRes;
	    }

	    CagdPtFreeList(InterPts);
	}
    }

    return Crv;					       /* CrvSplits == NULL. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Insert t1/t2 values into GlblInterList, provided no equal t1/t2 value      *
* exists already in the list. List is in ascending order with respect to t1. *
*                                                                            *
* PARAMETERS:                                                                *
*   t:  	 New parameter values to insert to split list.		     *
*   SplitLst:    List of points the curve in questions is to be splitted at. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertInterPoints(CagdRType t, CagdPtStruct **SplitLst)
{
    CagdPtStruct *PtTmp, *PtLast, *Pt;

    Pt = CagdPtNew();
    Pt -> Pt[0] = t;
    Pt -> Pt[1] = 0.0;
    Pt -> Pt[2] = 0.0;

    if (*SplitLst != NULL) {
	for (PtTmp = *SplitLst, PtLast = NULL;
	     PtTmp != NULL;
	     PtLast = PtTmp, PtTmp = PtTmp -> Pnext) {
	    if (IRIT_APX_EQ_EPS(PtTmp -> Pt[0], t, IHID_CCI_SAME_EPS)) {
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
		*SplitLst = Pt;
	}
	else {
	    /* Insert the new point as the last point in the list. */
	    PtLast -> Pnext = Pt;
	}
    }
    else
        *SplitLst = Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the subdivided curves out of the given curve, with all attributes.M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to subdivide.                                          M
*   t:          Subdivision parameter value.                                 M
*   Crv1, Crv2: The two subdivided segments, updated with all attributes.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if split occured, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCIUpdateSubdivCrvs                                                      M
*****************************************************************************/
int CCIUpdateSubdivCrvs(CagdCrvStruct *Crv,
			CagdRType t,
			CagdCrvStruct **Crv1,
			CagdCrvStruct **Crv2)
{
    CagdRType TMin, TMax, TMid, t1, t2;
    CagdCrvStruct *Crv2a,
	*UVCrv = AttrGetPtrAttrib(Crv -> Attr, "_Uv");

    CagdCrvDomain(Crv, &TMin, &TMax);
    TMid = (TMin + TMax) * 0.5;
    if (t > TMid) {
        t1 = t - GlblIHidTolerance;
	t2 = t;
    }
    else {
        t1 = t;
	t2 = t + GlblIHidTolerance;
    }

    if (t1 <= TMin + GlblIHidTolerance || t2 >= TMax - GlblIHidTolerance)
        return FALSE;

    *Crv1 = CagdCrvSubdivAtParam(Crv, t1);
    CagdCrvFree((*Crv1) -> Pnext);
    (*Crv1) -> Pnext = NULL;
	      
    Crv2a = CagdCrvSubdivAtParam(Crv, t2);
    *Crv2 = Crv2a -> Pnext;
    CagdCrvFree(Crv2a);

    IHID_COPY_ORIG_ATTR(*Crv1, Crv);
    IHID_COPY_ORIG_ATTR(*Crv2, Crv);

    if (UVCrv != NULL) {
	CagdCrvStruct
	    *UVCrv1 = CagdCrvSubdivAtParam(UVCrv, t),
	    *UVCrv2 = UVCrv1 -> Pnext;

	AttrSetPtrAttrib(&(*Crv1) -> Attr, "_Uv", UVCrv1);
	AttrSetPtrAttrib(&(*Crv2) -> Attr, "_Uv", UVCrv2);
	
	CagdCrvFree(UVCrv);
    }

    return TRUE;
}
