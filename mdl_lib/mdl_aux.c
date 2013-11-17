/******************************************************************************
* Mdl_aux.c - auxiliary routines to interface and maintain models.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber					 Feb 2011     *
******************************************************************************/

#include "irit_sm.h"
#include "mdl_loc.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"

static int MdlUpdateSrfSegRefs(MdlTrimSrfStruct *Srf,
			       MdlTrimSegStruct *Seg,
			       MdlTrimSegStruct *NextSeg);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examines the given list of curve segments in TSrf domain if inside or    M
* not. Outside segments are purged while inside curve segments are returned. M
*                                                                            *
* PARAMETERS:                                                                M
*   TSegs:    Curve segments in TSrf domain to examine if inside the trimmed M
*             domain of TSrf.						     M
*   TSrf:     Trimmed surface to examine containment of TSegs in.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegStruct *:  Filtered (inside) list of TSegs (in TSrf).          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlFilterOutCrvs                                                         M
*****************************************************************************/
MdlTrimSegStruct *MdlFilterOutCrvs(MdlTrimSegStruct *TSegs,
				   const MdlTrimSrfStruct *TSrf)
{
    MdlTrimSegStruct *TSeg,
        *InsideTSegs = NULL;

    while (TSegs != NULL) {
        CagdRType TMin, TMax, *R;
	CagdPType UV;
	CagdCrvStruct *UVCrv;

        IRIT_LIST_POP(TSeg, TSegs);

  	assert(TSeg -> SrfFirst == TSrf ||
	       TSeg -> SrfSecond == TSrf);
	UVCrv = TSeg -> SrfFirst == TSrf ? TSeg -> UVCrvFirst
					 : TSeg -> UVCrvSecond;
	assert(UVCrv != NULL);

	CagdCrvDomain(UVCrv, &TMin, &TMax);

	R = CagdCrvEval(UVCrv, (TMin + TMax) * 0.5);
	CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);

	if (MdlIsPointInsideTrimSrf(TSrf, UV)) {
	    IRIT_LIST_PUSH(TSeg, InsideTSegs);
	}
	else {
	    MdlTrimSegFree(TSeg);
	}
    }

    return CagdListReverse(InsideTSegs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Seg was divided in place into Seg and RestSegs.  Update references in    *
* Srf to Seg to now point to both Seg and all pieces in RestSegs.            *
*   Note that the order can be reversed if the segments are referenced in    *
* reverse.						                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:     Surface that needs an update to its trimming curves refs.       *
*   Seg:     Original segment referenced in Srf.                             *
*   RestSegs: New segments, after Seg, due to division, to update Srf with.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if segment found and updated, FALSE otherwise.                *
*****************************************************************************/
static int MdlUpdateSrfSegRefs(MdlTrimSrfStruct *Srf,
			       MdlTrimSegStruct *Seg,
			       MdlTrimSegStruct *RestSegs)
{
    MdlLoopStruct *Loop;

    if (RestSegs == NULL) {
	return FALSE;
    }

    for (Loop = Srf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
        MdlTrimSegRefStruct *SegRef;

	for (SegRef = Loop -> SegRefList;
	     SegRef != NULL;
	     SegRef = SegRef -> Pnext) {
	    if (SegRef -> TrimSeg == Seg) {          /* Found the segment. */
	        MdlTrimSegRefStruct *LastSRef, *SRef,
		    *RestRefSegs = NULL;
		MdlTrimSegStruct *S;

		for (S = RestSegs; S != NULL; S = S -> Pnext) {
		    SRef = MdlTrimSegRefNew(S);
		    SRef -> Reversed = SegRef -> Reversed;
		    SRef -> Tags = SegRef -> Tags;
		    SRef -> Attr = AttrCopyAttributes(SegRef -> Attr);

		    IRIT_LIST_PUSH(SRef, RestRefSegs);
		}

		/* We now have in SegRef the 1st piece and in RestRefSegs */
		/* the rest in reverse as (Last, ..., 3rd, 2nd) pieces.	  */
	        if (SegRef -> Reversed) {
		    /* Swap so Seg will hold the Last element and         */
		    /* RestSegs will have (1st, ..., 3rd, 2nd).		  */
		    IRIT_SWAP(MdlTrimSegStruct *, RestRefSegs -> TrimSeg,
			                          SegRef -> TrimSeg);
 
		    /* And move the first item in RestSegs to be the last.*/
		    if (RestRefSegs -> Pnext != NULL) {
		        LastSRef = CagdListLast(RestRefSegs);
		        SRef = RestRefSegs;
			RestRefSegs = RestRefSegs -> Pnext;
			SRef -> Pnext = NULL;
			LastSRef -> Pnext = SRef;
		    }
	        }
		else {
		    /* Reference Seg and RestRefSegs, in place, in order. */
		    RestRefSegs = CagdListReverse(RestRefSegs);
		}

		/* Chain rest of the pieces, in RestRefSegs, after SegRef. */
		LastSRef = CagdListLast(RestRefSegs);
		LastSRef -> Pnext = SegRef -> Pnext;
		SegRef -> Pnext = RestRefSegs;

		return TRUE;
	    }
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivides the given segment at the specified parameter values.          M
*   This amounts to:							     M
* 1. Dividing all curves in Seg and chaining the pieces after Seg.	     M
* 2. Updating the references in SrfFirst and SrfNext that points to Seg, to  M
*    now point to all new pieces, in the right order, as can be reversed.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:   Trimming segment to split, in place.	        		     M
*   Pts:   Parameters at which to split.		       		     M
*   Idx:   Index of parameter in Pts points: 0 for X, 1 for Y, etc.          M
*   Eps:   parameter closer than Eps to boundary or other parameters are     M
*	   ignored.							     M
*   Proximity: Proximity bit set to end points - see CagdCrvSubdivAtParams2. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlSplitTrimCrv, allocation                   	                     M
*****************************************************************************/
int MdlSplitTrimCrv(MdlTrimSegStruct *Seg,
		    const CagdPtStruct *Pts,
		    int Idx,
		    CagdRType Eps,
		    int *Proximity)
{
    CagdRType TMin, TMax;
    MdlTrimSegStruct *RestSegs;

    assert(Seg -> UVCrvFirst != NULL);
    CagdCrvDomain(Seg -> UVCrvFirst, &TMin, &TMax);

#ifdef DEBUG
    {
        CagdRType t1, t2;

	if (Seg -> UVCrvSecond != NULL) {
	    CagdCrvDomain(Seg -> UVCrvSecond, &t1, &t2);
	    assert(IRIT_APX_EQ_EPS(TMin, t1, IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(TMax, t2, IRIT_UEPS));
	}
    }
#endif /* DEBUG */

    RestSegs = MdlDivideTrimCrv(Seg, Pts, Idx, Eps, Proximity);

    /* Update the up to two surfaces that reference this trimming curve. */
    if (Seg -> SrfFirst != NULL)
        MdlUpdateSrfSegRefs(Seg -> SrfFirst, Seg, RestSegs);

    if (Seg -> SrfSecond != NULL)
        MdlUpdateSrfSegRefs(Seg -> SrfSecond, Seg, RestSegs);

    if (RestSegs != NULL) {
        ((MdlTrimSegStruct *) CagdListLast(RestSegs)) -> Pnext = Seg -> Pnext;
	Seg -> Pnext = RestSegs;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivides the given segment at the specified parameters values.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:   Trimming segment to divide.  Seg will return, in place, the       M
*          first curve in the divided set.		       		     M
*   Pts:   Parameters at which to split.		       		     M
*   Idx:   Index of parameter in Pts points: 0 for X, 1 for Y, etc.          M
*   Eps:   parameter closer than Eps to boundary or other parameters are     M
*	   ignored.							     M
*   Proximity: Proximity bit set to end points - see CagdCrvSubdivAtParams2. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegStruct *:  The rest of the divided segments.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvSubdivAtParams2						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDivideTrimCrv, allocation                 	                     M
*****************************************************************************/
MdlTrimSegStruct *MdlDivideTrimCrv(MdlTrimSegStruct *Seg,
				   const CagdPtStruct *Pts,
				   int Idx,
				   CagdRType Eps,
				   int *Proximity)
{
    CagdCrvStruct *UVCrvsFirst, *UVCrvsSecond, *UVCFirst, *UVCSecond;
    MdlTrimSegStruct *NewSeg,
        *RestSegs = NULL;

    assert(Seg -> UVCrvFirst != NULL);
    UVCrvsFirst = CagdCrvSubdivAtParams2(Seg -> UVCrvFirst, Pts, Idx, Eps,
					 Proximity);

    if (Seg -> UVCrvSecond != NULL)
        UVCrvsSecond = CagdCrvSubdivAtParams2(Seg -> UVCrvSecond, Pts, Idx,
					      Eps, Proximity);
    else
        UVCrvsSecond = NULL;

    /* Substitute the first piece into the old Seg, in place. */
    CagdCrvFree(Seg -> UVCrvFirst);
    Seg -> UVCrvFirst = UVCrvsFirst;
    UVCrvsFirst = UVCrvsFirst -> Pnext;
    Seg -> UVCrvFirst -> Pnext = NULL;

    if (Seg -> UVCrvSecond != NULL) {
        CagdCrvFree(Seg -> UVCrvSecond);
	Seg -> UVCrvSecond = UVCrvsSecond;
	UVCrvsSecond = UVCrvsSecond -> Pnext;
	Seg -> UVCrvSecond -> Pnext = NULL;
    }

    if (Seg -> EucCrv != NULL) {
        /* Will recreate the Euclidean curve on next use. */
	CagdCrvFree(Seg -> EucCrv);
	Seg -> EucCrv = NULL;
    }

    /* Build next Segs for all the other pieces, after Seg. */
    while (UVCrvsFirst != NULL) {
        NewSeg = MdlTrimSegNew(NULL, NULL, NULL,
			       Seg -> SrfFirst, Seg -> SrfSecond);

        IRIT_LIST_POP(UVCFirst, UVCrvsFirst);
	NewSeg -> UVCrvFirst = UVCFirst;

	if (Seg -> UVCrvSecond != NULL) {
	    assert(UVCrvsSecond != NULL);
	    IRIT_LIST_POP(UVCSecond, UVCrvsSecond);
	    NewSeg -> UVCrvSecond = UVCSecond;
	}

	IRIT_LIST_PUSH(NewSeg, RestSegs);
    }

    return CagdListReverse(RestSegs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the given UV value is inside the domain prescribed by    M
* the trimming curves of TSrf.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:           Trimming curves to consider.                             M
*   UV:             Parametric location.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if inside, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimIsPointInsideTrimUVCrv, TrimIsPointInsideTrimSrf,		     M
*   TrimIsPointInsideTrimCrvs				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlIsPointInsideTrimSrf, inclusion    	                             M
*****************************************************************************/
CagdBType MdlIsPointInsideTrimSrf(const MdlTrimSrfStruct *TSrf,
				  CagdUVType UV)
{
    int NumInters = 0;
    const MdlLoopStruct *Loop;

    for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
        const MdlTrimSegRefStruct *SegRef;

	for (SegRef = Loop -> SegRefList;
	     SegRef != NULL;
	     SegRef = SegRef -> Pnext) {
	    const CagdCrvStruct *UVCrv;

	    if (SegRef -> TrimSeg -> SrfFirst == TSrf)
	        UVCrv = SegRef -> TrimSeg -> UVCrvFirst;
	    else if (SegRef -> TrimSeg -> SrfSecond == TSrf)
	        UVCrv = SegRef -> TrimSeg -> UVCrvSecond;
	    else
	        UVCrv = NULL;

	    if (UVCrv != NULL)
	        NumInters += TrimIsPointInsideTrimUVCrv(UVCrv, UV);
	}
    }

    return NumInters & 0x01;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Ensure adjacent trimming curves have the precise same end points.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Mdl:  To ensure the precision of its trimming curves.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlEnsureTSrfTrimCrvsPrecision			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlEnsureMdlTrimCrvsPrecision                                            M
*****************************************************************************/
void MdlEnsureMdlTrimCrvsPrecision(MdlModelStruct *Mdl)
{
    MdlTrimSrfStruct *MdlTrimSrf;

    for (MdlTrimSrf = Mdl -> TrimSrfList;
	 MdlTrimSrf != NULL;
	 MdlTrimSrf = MdlTrimSrf -> Pnext) {
        MdlEnsureTSrfTrimCrvsPrecision(MdlTrimSrf);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Ensure adjacent trimming curves have the precise same end points.        M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSrf:  To ensure the precision of its trimming curves.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlEnsureMdlTrimCrvsPrecision			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlEnsureTSrfTrimCrvsPrecision                                           M
*****************************************************************************/
void MdlEnsureTSrfTrimCrvsPrecision(MdlTrimSrfStruct *MdlTrimSrf)
{
    MdlLoopStruct
        *Loops = MdlTrimSrf -> LoopList;

    for ( ; Loops != NULL; Loops = Loops -> Pnext) {
        int Idx,  PrevIdx;
	CagdRType
	    **PtsPrev, **Pts;
	CagdCrvStruct
	    *UVCrv, *UVCrvPrev;
	MdlTrimSegRefStruct
	    *TRef = CagdListLast(Loops -> SegRefList);
	MdlTrimSegStruct
	    *TSeg = TRef -> TrimSeg;

	if (TSeg -> SrfFirst == MdlTrimSrf)
	    UVCrvPrev = TSeg -> UVCrvFirst;
	else {
	    assert(TSeg -> SrfSecond == MdlTrimSrf);
	    UVCrvPrev = TSeg -> UVCrvSecond;
	}
	PrevIdx = TRef -> Reversed ? 0 : UVCrvPrev -> Length - 1;

	/* Make all end points in the loop the same. */
	for (TRef = Loops -> SegRefList;
	     TRef != NULL;
	     TRef = TRef -> Pnext) {
	    TSeg = TRef -> TrimSeg;

	    if (TSeg -> SrfFirst == MdlTrimSrf)
	        UVCrv = TSeg -> UVCrvFirst;
	    else {
	        assert(TSeg -> SrfSecond == MdlTrimSrf);
		UVCrv = TSeg -> UVCrvSecond;
	    }

	    /* Make end pt of UVCrvPrev and staring pt of UVCrv same. */
	    assert(UVCrv -> Order == 2);
	    Idx = TRef -> Reversed ? UVCrv -> Length - 1 : 0;
	    PtsPrev = UVCrvPrev -> Points;
	    Pts = UVCrv -> Points;

#	    ifdef DEBUG
	    if (!IRIT_APX_EQ_EPS(PtsPrev[1][PrevIdx], Pts[1][Idx],
				 MDL_BOOL_NEAR_UV_EPS) ||
		!IRIT_APX_EQ_EPS(PtsPrev[2][PrevIdx], Pts[2][Idx],
				 MDL_BOOL_NEAR_UV_EPS))
	        assert(0);
#	    endif /* DEBUG */

	    PtsPrev[1][PrevIdx] = Pts[1][Idx] =
	                            (PtsPrev[1][PrevIdx] + Pts[1][Idx]) * 0.5;
	    PtsPrev[2][PrevIdx] = Pts[2][Idx] =
	                            (PtsPrev[2][PrevIdx] + Pts[2][Idx]) * 0.5;

	    UVCrvPrev = UVCrv;
	    PrevIdx = TRef -> Reversed ? 0 : UVCrv -> Length - 1;
	}
    }
}
