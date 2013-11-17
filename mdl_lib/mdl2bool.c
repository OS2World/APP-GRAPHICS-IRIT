/*****************************************************************************
*   Additional module for booleans over models.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0, Dec. 1996    *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "mdl_loc.h"

static int MdlBoolRemoveSegRefFromSegRefList(const MdlTrimSrfStruct *TSrf,
					     MdlTrimSegRefStruct **RefList,
					     MdlTrimSegRefStruct *Ref,
					     CagdBType FreeRef,
					     CagdRType Tol);
static int MdlBoolFindSegRefInSegRefList(const CagdRType *UV,
					 const MdlTrimSrfStruct *TSrf,
					 const MdlTrimSegRefStruct *RefList,
					 const MdlTrimSegRefStruct **Ref,
					 CagdRType Tol);
static int MdlBoolGetOtherRefsAtIntersection(const MdlTrimSrfStruct *TSrf,
					     const MdlTrimSegRefStruct *AllRefs,
					     const MdlTrimSegRefStruct *Ref,
					     CagdBType StartPt,
					     CagdRType Tol,
					     CagdUVType UV,
					     CagdUVType OtherUV,
					     const MdlTrimSegRefStruct **Ref1,
					     int *n1,
					     const MdlTrimSegRefStruct **Ref2,
					     int *n2);
static CagdBType MdlBoolClassifyAtPtAgainstOtherModel(
					    const MdlTrimSrfStruct *TSrf,
					    const MdlTrimSrfStruct *OtherTSrf,
					    const CagdUVType UV,
					    const CagdUVType UV2,
					    const CagdUVType OtherUV);
static CagdBType MdlBoolRefInOtherModel(const MdlTrimSrfStruct *TSrf,
					const MdlTrimSrfStruct *OtherTSrf,
					const MdlTrimSegRefStruct *Ref,
					CagdBType StartPt,
					const CagdUVType UV,
					const CagdUVType OtherUV);
static void MdlBoolPropagateRefInOut(const MdlTrimSrfStruct *TSrf,
				     CagdRType Tol,
				     MdlTrimSegRefStruct **Refs,
				     const MdlTrimSegRefStruct *ThisRef,
				     CagdBType PropThruStartPt,
				     CagdBType IsInside);
static int MdlBoolClassifyNonInterTrimRef(const MdlTrimSrfStruct *TSrf,
					  const MdlTrimSegRefStruct *CRef,
					  const MdlTrimSegRefStruct **Refs,
					  int *Classification);
static int MdlBoolIsCornerPoint(const CagdCrvStruct *Crv, int PtIdx);
static void MdlBoolMakeCrvsEndPtsSame(const MdlTrimSrfStruct *TSrf,
				      MdlTrimSegRefStruct *Ref1,
				      CagdBType Start1,
				      MdlTrimSegRefStruct *Ref2,
				      CagdBType Start2);
static int MdlBoolClassifyAtIntersections(MdlTrimSegRefStruct **Refs,
					  MdlTrimSrfStruct *TSrf,
					  CagdRType Tol,
					  CagdBType InsideOtherModelTSrf);
static MdlLoopStruct *MdlBoolMergeIntoLoops(MdlTrimSegRefStruct **Refs,
					    const MdlTrimSrfStruct *TSrf,
					    CagdRType Tol);
static MdlTrimSegRefStruct *MdlBoolGetOtherRef(
					   const MdlTrimSegRefStruct *SegRef,
					   const MdlTrimSrfStruct *TSrf);
static MdlTrimSegRefStruct *MdlBoolGetOtherRef2(
				     const MdlTrimSegRefStruct *SegRef,
				     const MdlTrimSrfStruct *TSrf,
				     MdlTrimSrfStruct **OtherTSrf,
				     MdlLoopStruct **OtherLoop);
static void MdlBoolSetClassifyNonInterOneTrimSrf(MdlTrimSrfStruct *TSrf,
						 CagdBType Inside);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Delete one Ref from the given RefList.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:      The trimmed surface this reference belongs to.		     *
*   RefList:   List to delete the item pointed by Ref.		             *
*   Ref:       Reference to delete from the list.                            *
*   FreeRef:   TRUE to also free it and its associated reference if exist,   *
*              FALSE to just remove from list.			             *
*   Tol:       Tolerance to search and mark adjacent segment and mark as     *
*              outside, if FreeRef.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if found and deleted, FALSE otherwise.                    *
*****************************************************************************/
static int MdlBoolRemoveSegRefFromSegRefList(const MdlTrimSrfStruct *TSrf,
					     MdlTrimSegRefStruct **RefList,
					     MdlTrimSegRefStruct *Ref,
					     CagdBType FreeRef,
					     CagdRType Tol)
{
    MdlTrimSegRefStruct *R;

#ifdef DEBUG_MDL_BOOL_DEL_SEGREF
    fprintf(stderr, "Deleting Ref %08x, Seg %08x, TSrfs = %08x, %08x\n",
	    Ref, Ref -> TrimSeg,
	    Ref -> TrimSeg -> SrfFirst, Ref -> TrimSeg -> SrfSecond);
#endif /* DEBUG_MDL_BOOL_DEL_SEGREF */

    if (*RefList == Ref) {
        *RefList = (*RefList) -> Pnext;
    }
    else {
        for (R = *RefList;
	     R -> Pnext != NULL && R -> Pnext != Ref;
	     R = R -> Pnext);
	if (R == NULL)
	    return FALSE;

	assert(R -> Pnext == Ref);
	R -> Pnext = Ref -> Pnext;
    }

    if (FreeRef) {
        MdlTrimSrfStruct *OtherTSrf;
	MdlLoopStruct *OtherLoop;
        MdlTrimSegRefStruct
	    *OtherRef = MdlBoolGetOtherRef2(Ref, TSrf, &OtherTSrf, &OtherLoop);

	/* Mark all adjacent segments to Ref as outside. */
	MdlBoolPropagateRefInOut(TSrf, Tol, RefList, Ref, TRUE, FALSE);
	MdlBoolPropagateRefInOut(TSrf, Tol, RefList, Ref, FALSE, FALSE);

	if (OtherRef != NULL) {
	    /* Recursively remove & delete other reference to this segment. */
	    MdlBoolRemoveSegRefFromSegRefList(OtherTSrf,
					      &OtherLoop -> SegRefList,
					      OtherRef, FALSE, Tol);
	    MdlTrimSegRefFree(OtherRef);
	}

	MdlTrimSegRefFree(Ref);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Search for a curve in RefList that belongs to surface TSrf and starts or *
* ends at UV.                                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   UV:        UV location to search in the list.                            *
*   TSrf:      Trimmed surface we are now processing.			     *
*   RefList:   List to search for a curve with given UV end point.           *
*   Ref:       Found reference, if any.                                      *
*   Tol:       Tolerance to match end points of trimming curves.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     0 if not found, 1 if match start pt of Ref, 2 if match end pt.  *
*****************************************************************************/
static int MdlBoolFindSegRefInSegRefList(const CagdRType *UV,
					 const MdlTrimSrfStruct *TSrf,
					 const MdlTrimSegRefStruct *RefList,
					 const MdlTrimSegRefStruct **Ref,
					 CagdRType Tol)
{
    const MdlTrimSegRefStruct *R;

    *Ref = NULL;

    for (R = RefList; R != NULL; R = R -> Pnext) {
        CagdRType TMin, TMax, *t;
	CagdUVType UVTmp;
        const MdlTrimSegStruct
	    *Seg = R -> TrimSeg;
        const CagdCrvStruct *UVCrv;

	if (Seg -> SrfFirst == TSrf)
	    UVCrv = Seg -> UVCrvFirst;
	else if (Seg -> SrfSecond == TSrf)
	    UVCrv = Seg -> UVCrvSecond;
	else
	    continue;

	CagdCrvDomain(UVCrv, &TMin, &TMax);

	t = CagdCrvEval(UVCrv, TMin);
	CagdCoerceToE2(UVTmp, &t, -1, UVCrv -> PType);
	if (IRIT_PT_APX_EQ_E2_EPS(UV, UVTmp, Tol)) {
	    *Ref = R;
	    return 1;
	}

	t = CagdCrvEval(UVCrv, TMax);
	CagdCoerceToE2(UVTmp, &t, -1, UVCrv -> PType);
	if (IRIT_PT_APX_EQ_E2_EPS(UV, UVTmp, Tol)) {
	    *Ref = R;
	    return 2;
	}
    }

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a reference to a trimming curve and a designated end point (start  *
* or end point), find the up to two other trimming curves at that location.  *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:     Trimmed surface we now update its trimming loops.		     *
*   Ref:      Reference to seek its neighbors at the designed end point.     *
*   AllRefs:  All references to search.  A linked list.			     *
*   StartPt:  TRUE to seek neighbors of Ref at start location, FALSE end pt. *
*   Tol:    Tolerance to match end points of trimming curves.	             *
*   UV:       UV value of this surface (TSrf) at this junction.              *
*   OtherUV:  UV value of the other model's surface (!TSrf) at this junction.*
*   Ref1:     First neighbor.                                                *
*   n1:       1 if match start pt of Ref1, 2 if match end pt of Ref1.        *
*   Ref2:     Second neighbor - can be NULL.                                 *
*   n2:       1 if match start pt of Ref2, 2 if match end pt of Ref2.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     Returns the number of neighbors found.                          *
*****************************************************************************/
static int MdlBoolGetOtherRefsAtIntersection(const MdlTrimSrfStruct *TSrf,
					     const MdlTrimSegRefStruct *AllRefs,
					     const MdlTrimSegRefStruct *Ref,
					     CagdBType StartPt,
					     CagdRType Tol,
					     CagdUVType UV,
					     CagdUVType OtherUV,
					     const MdlTrimSegRefStruct **Ref1,
					     int *n1,
					     const MdlTrimSegRefStruct **Ref2,
					     int *n2)
{
    CagdRType TMin, TMax, *t;
    const CagdCrvStruct *UVCrv, *OtherUVCrv;

    if (Ref -> TrimSeg -> SrfFirst == TSrf) {
        UVCrv = Ref -> TrimSeg -> UVCrvFirst;
	OtherUVCrv = Ref -> TrimSeg -> UVCrvSecond;
    }
    else if (Ref -> TrimSeg -> SrfSecond == TSrf) {
        UVCrv = Ref -> TrimSeg -> UVCrvSecond;
	OtherUVCrv = Ref -> TrimSeg -> UVCrvFirst;
    }
    else {
        assert(0);
	_MdlBoolFatalErrorNum = MDL_ERR_BOOL_GET_REF;
	longjmp(_MdlBoolLongJumpBuf, 1);
	return 0;
    }

    CagdCrvDomain(UVCrv, &TMin, &TMax);
    if (StartPt) {
        t = CagdCrvEval(UVCrv, TMin);
	CagdCoerceToE2(UV, &t, -1, UVCrv -> PType);
	if (OtherUVCrv != NULL) {
	    t = CagdCrvEval(OtherUVCrv, TMin);
	    CagdCoerceToE2(OtherUV, &t, -1, OtherUVCrv -> PType);
	}
	else {
	    OtherUV[0] = OtherUV[1] = IRIT_INFNTY;
	}
    }
    else {
        t = CagdCrvEval(UVCrv, TMax);
	CagdCoerceToE2(UV, &t, -1, UVCrv -> PType);
	if (OtherUVCrv != NULL) {
	    t = CagdCrvEval(OtherUVCrv, TMax);
	    CagdCoerceToE2(OtherUV, &t, -1, OtherUVCrv -> PType);
	}
	else {
	    OtherUV[0] = OtherUV[1] = IRIT_INFNTY;
	}
    }

    /* Find the other (upto) two old trimming curves at this end pt. */
    *n1 = MdlBoolFindSegRefInSegRefList(UV, TSrf, AllRefs, Ref1, Tol);
    if (*Ref1 == Ref)
        *n1 = MdlBoolFindSegRefInSegRefList(UV, TSrf, (*Ref1) -> Pnext,
					    Ref1, Tol);

    if (*Ref1 == NULL) {
        *Ref2 = NULL;
	*n1 = *n2 = 0;
        return 0;
    }

    *n2 = MdlBoolFindSegRefInSegRefList(UV, TSrf, (*Ref1) -> Pnext,
					Ref2, Tol);
    if (*Ref2 == Ref)
        *n2 = MdlBoolFindSegRefInSegRefList(UV, TSrf, (*Ref2) -> Pnext,
					    Ref2, Tol);
    else if (*Ref2 == NULL)
        *n2 = 0;

    return *Ref2 == NULL ? 1 : 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Classify point TSrf(UV2) that is near TSrf(UV) == OtherTSrf(OtherUV) as  *
* inside or outside OtherTSrf.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:     A trimmed surface we need to update its trimming loops.        *
*   OtherTSrf: Other trimmed surface intersecting TSrf.			     *
*   UV:       The UV value of the designated end point in TSrf.		     *
*   UV2:      A UV value near UV on TSrf, in the examined area for in/out.   *
*   OtherUV:  The UV value of the designated end point in other surface.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if TSrf(UV2) is inside the other surfaces' model,       *
*		OtherTSrf.  FALSE if outside.                                *
*****************************************************************************/
static CagdBType MdlBoolClassifyAtPtAgainstOtherModel(
					    const MdlTrimSrfStruct *TSrf,
					    const MdlTrimSrfStruct *OtherTSrf,
					    const CagdUVType UV,
					    const CagdUVType UV2,
					    const CagdUVType OtherUV)
{
    CagdRType *R;
    CagdPType Pos, Pos2;
    CagdVType Dir;
    CagdVecStruct *Nrml;

    R = CagdSrfEval(TSrf -> Srf, UV[0], UV[1]);
    CagdCoerceToE3(Pos, &R, -1, TSrf -> Srf -> PType);

#ifdef DEBUG
    /* Verify the end points are the same in E3. */
    R = CagdSrfEval(OtherTSrf -> Srf, OtherUV[0], OtherUV[1]);
    CagdCoerceToE3(Pos2, &R, -1, OtherTSrf -> Srf -> PType);
    assert(IRIT_PT_APX_EQ_EPS(Pos, Pos2, MDL_BOOL_NEAR_UV_EPS));
#endif /*DEBUG */

    Nrml = CagdSrfNormal(OtherTSrf -> Srf, OtherUV[0], OtherUV[1], TRUE);

    R = CagdSrfEval(TSrf -> Srf, UV2[0], UV2[1]);
    CagdCoerceToE3(Pos2, &R, -1, TSrf -> Srf -> PType);

    IRIT_VEC_SUB(Dir, Pos2, Pos);

    return IRIT_DOT_PROD(Nrml -> Vec, Dir) > 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine if the trimming curve segment referenced by Ref, at its 	     *
* designated end point is inside other surface' model (that is not TSrf).    *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:     A trimmed surface we need to update its trimming loops.        *
*   OtherTSrf: Other trimmed surface intersecting TSrf.			     *
*   Ref:      Reference to a trimming curve we had like to examine if inside *
*	      the other trimmed surface's model(that is not TSrf).           *
*   StartPt:  TRUE to consider starting pt of Ref curve, FALSE for end pt.   *
*   UV:       The UV value of the designated end point in TSrf.		     *
*   OtherUV:  The UV value of the designated end point in other surface.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:    TRUE if Ref references a curve that is inside the other    *
*             surfaces' model.  FALSE if outside.                            *
*****************************************************************************/
static CagdBType MdlBoolRefInOtherModel(const MdlTrimSrfStruct *TSrf,
					const MdlTrimSrfStruct *OtherTSrf,
					const MdlTrimSegRefStruct *Ref,
					CagdBType StartPt,
					const CagdUVType UV,
					const CagdUVType OtherUV)
{
    int i, Step, Len;
    CagdUVType UV2, DUV;
    CagdCrvStruct *UVCrv;

    if (Ref -> TrimSeg -> SrfFirst == TSrf) {
	UVCrv = Ref -> TrimSeg -> UVCrvFirst;
    }
    else {
        assert(Ref -> TrimSeg -> SrfSecond == TSrf);
	UVCrv = Ref -> TrimSeg -> UVCrvSecond;
    }
    Len = UVCrv -> Length;

    if (StartPt) {
        assert(IRIT_APX_EQ_EPS(UV[0], UVCrv -> Points[1][0],
			       MDL_BOOL_NEAR_UV_EPS) &&
	       IRIT_APX_EQ_EPS(UV[1], UVCrv -> Points[2][0],
			       MDL_BOOL_NEAR_UV_EPS));

	i = 1;
	Step = 1;
    }
    else {
        assert(IRIT_APX_EQ_EPS(UV[0], UVCrv -> Points[1][UVCrv -> Length - 1],
			       MDL_BOOL_NEAR_UV_EPS) &&
	       IRIT_APX_EQ_EPS(UV[1], UVCrv -> Points[2][UVCrv -> Length - 1],
			       MDL_BOOL_NEAR_UV_EPS));

	i = Len - 2;
	Step = -1;
    }

    /* Search a near by (but not too near) point on the curve to UV. */
    do {
        UV2[0] = UVCrv -> Points[1][i];
	UV2[1] = UVCrv -> Points[2][i];
	IRIT_PT2D_SUB(DUV, UV2, UV);
	i += Step;
    }
    while (IRIT_ABS(DUV[0]) + IRIT_ABS(DUV[1]) < MDL_BOOL_NEAR_UV_EPS &&
	   (i > 0 && i < Len));
    if (IRIT_ABS(DUV[0]) + IRIT_ABS(DUV[1]) < MDL_BOOL_NEAR_UV_EPS) {
        assert(0);
	_MdlBoolFatalErrorNum = MDL_ERR_BOOL_CLASSIFY_FAIL;
	longjmp(_MdlBoolLongJumpBuf, 1);
	return 0;
    }

    /* Make sure the step from UV to UV2 is minute. */
    IRIT_VEC2D_NORMALIZE(DUV);
    IRIT_VEC2D_SCALE(DUV, MDL_BOOL_NEAR_UV_EPS);
    IRIT_VEC2D_ADD(UV2, UV, DUV);

    return MdlBoolClassifyAtPtAgainstOtherModel(TSrf, OtherTSrf,
						UV, UV2, OtherUV);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Propagates inside/outside information from This ref to its designated    *
* neighbor 'PropThruStartPt'.  Only Old trimming curves are considered.      *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:          Trimmed surface we now update its trimming loops.         *
*   Tol:           Tolerance to match end points of trimming curves.         *
*   Refs:          All references to all trimming curves.	             *
*   ThisRef:       This reference we had like to propagate it property thru. *
*   PropThruStartPt: TRUE to propagate thru the starting point, FALSE thru   *
*                  end point.					             *
*   IsInside:      Inside/outside info - TRUE for inside, FALSE outside.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void MdlBoolPropagateRefInOut(const MdlTrimSrfStruct *TSrf,
				     CagdRType Tol,
				     MdlTrimSegRefStruct **Refs,
				     const MdlTrimSegRefStruct *ThisRef,
				     CagdBType PropThruStartPt,
				     CagdBType IsInside)
{
    MdlTrimSegRefStruct *Ref1, *Ref2;
    CagdUVType UV, UV2;
    int n1, n2,
	n = MdlBoolGetOtherRefsAtIntersection(TSrf, *Refs, ThisRef,
				    PropThruStartPt, Tol, UV, UV2,
				    (const MdlTrimSegRefStruct **) &Ref1, &n1,
				    (const MdlTrimSegRefStruct **) &Ref2, &n2);

    if (n > 1)
	return;/* Do not propagate if this joint is a crossing intersection. */

    if (Ref1 != NULL &&
	!MDL_IS_TSEG_NEW(Ref1) &&
	IP_ATTR_IS_BAD_INT(AttrGetIntAttrib(Ref1 -> Attr, "_Inside"))) {
        AttrSetIntAttrib(&Ref1 -> Attr, "_Inside", IsInside);
	MdlBoolPropagateRefInOut(TSrf, Tol, Refs, Ref1,
				 PropThruStartPt, IsInside);
    }
    if (Ref2 != NULL &&
	!MDL_IS_TSEG_NEW(Ref2) &&
	IP_ATTR_IS_BAD_INT(AttrGetIntAttrib(Ref2 -> Attr, "_Inside"))) {
        AttrSetIntAttrib(&Ref2 -> Attr, "_Inside", IsInside);
	MdlBoolPropagateRefInOut(TSrf, Tol, Refs, Ref2,
				 PropThruStartPt, IsInside);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Classify a trimming curve segment (reference) of a loop that does not    *
* intersect with the other model.					     *
*   A point, P, on the intersection curve(s) that is closest to the middle,  *
* M, of CRef is found, in the parametric domain, and the PM ray is examined  *
* near P, against the other intersecting surface.                            *
*   Note there can be no intersection curve in this surface in which case    *
* the classification will fail.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:       Trimmed surface we now update its trimming loops.	     *
*   CRef:	Reference to a trimming curve segment we need to classify    *
*               as in or as out.					     *
*   Refs:	List of references of TSrf.                                  *
*   Classification: Result - TRUE if Ref is inside, FALSE otherwise.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        TRUE if classification was successful, FALSE otherwise.      *
*****************************************************************************/
static int MdlBoolClassifyNonInterTrimRef(const MdlTrimSrfStruct *TSrf,
					  const MdlTrimSegRefStruct *CRef,
					  const MdlTrimSegRefStruct **Refs,
					  int *Classification)
{
    int MinIdx = -1;
    CagdRType t1, t2, *R, DSqr,
        MinDistSqr = IRIT_INFNTY;
    CagdUVType CRefUV, UV, UV2, OtherUV;
    const MdlTrimSegRefStruct *Ref,
        *MinRef = NULL;
    const CagdCrvStruct *Crv, *OtherCrv;
    const MdlTrimSrfStruct *OtherTSrf;

    /* Find a middle point in Ref. */
    if (CRef -> TrimSeg -> SrfFirst == TSrf) {
        Crv = CRef -> TrimSeg -> UVCrvFirst;
    }
    else {
        assert(CRef -> TrimSeg -> SrfSecond == TSrf);
	Crv = CRef -> TrimSeg -> UVCrvSecond;
    }

    CagdCrvDomain(Crv, &t1, &t2);
    R = CagdCrvEval(Crv, (t1 + t2) * 0.5);
    CagdCoerceToE2(CRefUV, &R, -1, Crv -> PType);

    /* Find the closest intersection point to UV. */
    for (Ref = *Refs; Ref != NULL; Ref = Ref -> Pnext) {
	if (MDL_IS_TSEG_NEW(Ref)) {	    /* It is an intersection curve. */
	    int i;
	    CagdRType * const *Points;

	    /* Fetch the intersection curve in TSrf domain. */
	    if (Ref -> TrimSeg -> SrfFirst == TSrf) {
	        Crv = Ref -> TrimSeg -> UVCrvFirst;
	    }
	    else {
		Crv = Ref -> TrimSeg -> UVCrvSecond;
	    }
	    assert(Crv -> Order == 2);  /* Assume linear intersection crvs. */
	    Points = Crv -> Points;

	    for (i = 0; i < Crv -> Length; i++) {
		DSqr = IRIT_SQR(CRefUV[0] - Points[1][i]) +
		       IRIT_SQR(CRefUV[1] - Points[2][i]);

		if (MinDistSqr > DSqr && !MdlBoolIsCornerPoint(Crv, i)) {
		    MinDistSqr = DSqr;
		    MinRef = Ref;
		    MinIdx = i;
		}
	    }
	}
    }

    if (MinRef == NULL)
        return FALSE;

    /* Classify. */
    if (MinRef -> TrimSeg -> SrfFirst == TSrf) {
        Crv = MinRef -> TrimSeg -> UVCrvFirst;
	OtherCrv = MinRef -> TrimSeg -> UVCrvSecond;
	OtherTSrf = MinRef -> TrimSeg -> SrfSecond;
    }
    else {
        Crv = MinRef -> TrimSeg -> UVCrvSecond;
	OtherCrv = MinRef -> TrimSeg -> UVCrvFirst;
	OtherTSrf = MinRef -> TrimSeg -> SrfFirst;
    }

    /* If MinIDx happens to be on a sharp corner of Crv, a small            */
    /* perturbation might result in being outside.  Since MinIdx refers to  */
    /* an extreme point and hence a convex corner, we take the average of   */
    /* the PrevIdx, MinIDx and NextIdx which is going to be inside.         */
    UV[0] = Crv -> Points[1][MinIdx];
    UV[1] = Crv -> Points[2][MinIdx];
    OtherUV[0] = OtherCrv -> Points[1][MinIdx];
    OtherUV[1] = OtherCrv -> Points[2][MinIdx];

    /* Prepare a location near intersection curve UV in TSrf's domain in    */
    /* the direction toward unclassified CRef.				    */
    IRIT_VEC2D_SUB(UV2, CRefUV, UV);
    IRIT_VEC2D_NORMALIZE(UV2);
    IRIT_VEC2D_SCALE(UV2, MDL_BOOL_NEAR_UV_EPS);
    IRIT_VEC2D_ADD(UV2, UV2, UV);

    *Classification = MdlBoolClassifyAtPtAgainstOtherModel(TSrf, OtherTSrf,
							   UV, UV2, OtherUV);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine the given Points[PtIdx] if it is a corner.  A point will be      *
* considered a corner if more than a 45 degs. turn, or is an end point.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      Curve to examine its Points[PtIdx] point if a corner.          *
*   PtIdx:    The index of the points to consider.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if a corner, FALSE otherwise.                              *
*****************************************************************************/
static int MdlBoolIsCornerPoint(const CagdCrvStruct *Crv, int PtIdx)
{
    CagdRType
        * const *Points = Crv -> Points;
    CagdUVType UVPrev, UVNext;

    if (PtIdx == 0 || PtIdx == Crv -> Length - 1)
        return TRUE;           /* Consider the end points as corner points. */

    UVPrev[0] = Points[1][PtIdx] - Points[1][PtIdx - 1];
    UVPrev[1] = Points[2][PtIdx] - Points[2][PtIdx - 1];
    IRIT_PT2D_NORMALIZE(UVPrev);

    UVNext[0] = Points[1][PtIdx + 1] - Points[1][PtIdx];
    UVNext[1] = Points[2][PtIdx + 1] - Points[2][PtIdx];
    IRIT_PT2D_NORMALIZE(UVNext);

    return IRIT_DOT_PROD_2D(UVPrev, UVNext) < 0.707;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Force adjacent ref segments that were identified as such to be precisely *
* the same.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:   Trimemd surface these two adjacent edges belongs to.             *
*   Ref1:   First reference to a trimming curve of TSrf.                     *
*   Start1: TRUE if the shared end point is Start of Ref1, FALSE if end.     *
*   Ref2:   Second reference to a trimming curve of TSrf.                    *
*   Start2: TRUE if the shared end point is Start of Ref2, FALSE if end.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolMakeCrvsEndPtsSame(const MdlTrimSrfStruct *TSrf,
				      MdlTrimSegRefStruct *Ref1,
				      CagdBType Start1,
				      MdlTrimSegRefStruct *Ref2,
				      CagdBType Start2)
{
    int i, i1, i2;
    CagdCrvStruct *UVCrv1, *UVCrv2;

    if (Ref1 -> TrimSeg -> SrfFirst == TSrf)
	UVCrv1 = Ref1 -> TrimSeg -> UVCrvFirst;
    else {
        assert(Ref1 -> TrimSeg -> SrfSecond == TSrf);
        UVCrv1 = Ref1 -> TrimSeg -> UVCrvSecond;
    }

    if (Ref2 -> TrimSeg -> SrfFirst == TSrf)
	UVCrv2 = Ref2 -> TrimSeg -> UVCrvFirst;
    else {
        assert(Ref2 -> TrimSeg -> SrfSecond == TSrf);
        UVCrv2 = Ref2 -> TrimSeg -> UVCrvSecond;
    }

    assert(UVCrv1 != NULL &&
	   UVCrv2 != NULL &&
	   UVCrv1 -> Order == 2 &&
	   UVCrv2 -> Order == 2);

    i1 = Start1 ? 0 : UVCrv1 -> Length - 1;
    i2 = Start2 ? 0 : UVCrv2 -> Length - 1;

    if (!IRIT_APX_EQ_EPS(UVCrv1 -> Points[1][i1],
			 UVCrv2 -> Points[1][i2],
			 MDL_BOOL_NEAR_UV_EPS) ||
	!IRIT_APX_EQ_EPS(UVCrv1 -> Points[2][i1],
			 UVCrv2 -> Points[2][i2],
			 MDL_BOOL_NEAR_UV_EPS)) {
        assert(0);/*Assume close end points to within MDL_BOOL_NEAR_UV_EPS. */
	_MdlBoolFatalErrorNum = MDL_ERR_BOOL_UVMATCH_FAIL;
	longjmp(_MdlBoolLongJumpBuf, 1);
	return;
    }

    for (i = 1; i <= 2; i++) {  /* Make end points same. */
        UVCrv1 -> Points[i][i1] =
	    UVCrv2 -> Points[i][i2] =
	        (UVCrv1 -> Points[i][i1] + UVCrv2 -> Points[i][i2]) * 0.5;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Classify old trimming curves by examining the neighborhood of the        *
* intersection points with new curves, if inside/outside other model.        *
*   Old trimming curves that are found in/outside propagate this info to     *
* their neighbors.							     *
*   Isolated old loops (with no intersection with new curves/model) will not *
* be handled here.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Refs:   Set of references to trim curves segs to classify.  Outside      *
*           segments of Refs are purged on the fly, as a side effect.        *
*   TSrf:   Trimmed surface we now update its trimming loops.		     *
*   Tol:    Tolerance to match end points of trimming curves.	             *
*   InsideOtherModel:  TRUE if we seek the inside of the other model (i.e.   *
*           intersection operations), FALSE for outside (i.e. union).        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if successful, FALSE otherwise.                             *
*****************************************************************************/
static int MdlBoolClassifyAtIntersections(MdlTrimSegRefStruct **Refs,
					  MdlTrimSrfStruct *TSrf,
					  CagdRType Tol,
					  CagdBType InsideOtherModelTSrf)
{
    MdlTrimSegRefStruct *Ref, *LastRef;

#   ifdef DEBUG_MDL_DUMP_TSRF_TRIM_CRVS
    fprintf(stderr, "Refs before classification ******************:\n");
    MdlDebugHandleTSrfRefCrvs(*Refs, TSrf, NULL, 1, FALSE);
#   endif /* DEBUG_MDL_DUMP_TSRF_TRIM_CRVS */

    /* Classify by using intersection locations between original trimming   */
    /* curves and new intersection curves.				    */
    for (Ref = *Refs; Ref != NULL; Ref = Ref -> Pnext) {
        if (MDL_IS_TSEG_NEW(Ref)) {  /* Find the neighbors to its two ends. */
	    CagdBType Inside1, Inside2;
	    int n1, n2;
	    CagdRType
	        AdapTol = Tol;
	    CagdUVType UV, OtherUV;
	    MdlTrimSegRefStruct *Ref1, *Ref2;
	    const MdlTrimSrfStruct *OtherTSrf;

	    /* All new (intersection) curves are inside. */
	    AttrSetIntAttrib(&Ref -> Attr, "_Inside", TRUE);

	    /* All surfaces that have intersection curves are inside. */
	    AttrSetIntAttrib(&TSrf -> Attr, "_Inside", TRUE);

	    do {
	        MdlBoolGetOtherRefsAtIntersection(TSrf, *Refs, Ref, TRUE,
				AdapTol, UV, OtherUV,
				(const MdlTrimSegRefStruct **) &Ref1, &n1,
				(const MdlTrimSegRefStruct **) &Ref2, &n2);
		if (n1 == 0) {
		    /* No adjacent edges could be found at all.  This can */
		    /* happen because this is a closed loop or due to a   */
		    /* larger error to neighboring edges.  Increase Tol   */
		    /* if not a closed loop up to MDL_BOOL_NEAR_UV_EPS.   */
		    int Len1;
		    CagdCrvStruct *UVCrv;

		    if (Ref -> TrimSeg -> SrfFirst == TSrf)
		        UVCrv = Ref -> TrimSeg -> UVCrvFirst;
		    else {
			assert(Ref -> TrimSeg -> SrfSecond == TSrf);
		        UVCrv = Ref -> TrimSeg -> UVCrvSecond;
		    }
		    Len1 = UVCrv -> Length - 1;

		    if (IRIT_APX_EQ_EPS(UVCrv -> Points[1][0],
					UVCrv -> Points[1][Len1],
					MDL_BOOL_NEAR_UV_EPS) &&
			IRIT_APX_EQ_EPS(UVCrv -> Points[2][0],
					UVCrv -> Points[2][Len1],
					MDL_BOOL_NEAR_UV_EPS)) { /* A loop. */
		        MdlBoolMakeCrvsEndPtsSame(TSrf, Ref, TRUE, Ref, FALSE);
			break;
		    }
		    else
		        AdapTol *= 10.0;
		}
	    }
	    while (n1 == 0 && AdapTol < MDL_BOOL_NEAR_UV_EPS);

	    if (Ref1 != NULL)
	        MdlBoolMakeCrvsEndPtsSame(TSrf, Ref, TRUE, Ref1, n1 == 1);
	    if (Ref2 != NULL)
	        MdlBoolMakeCrvsEndPtsSame(TSrf, Ref, TRUE, Ref2, n2 == 1);

	    if (Ref -> TrimSeg -> SrfFirst == TSrf) {
	        OtherTSrf = Ref -> TrimSeg -> SrfSecond;
	    }
	    else {
	        assert(Ref -> TrimSeg -> SrfSecond == TSrf);
	        OtherTSrf = Ref -> TrimSeg -> SrfFirst;
	    }

	    /* Classify. */
	    switch (n1) {
	        default:
		    assert(0);
		    break;
	        case 0:
		    /* An intersection curve that is a closed loop. */
		    break;
	        case 1:
	        case 2:
		    Inside1 = MdlBoolRefInOtherModel(TSrf, OtherTSrf, Ref1,
						     n1 == 1, UV, OtherUV);
		    if (!InsideOtherModelTSrf)
		        Inside1 = !Inside1;
		    /* New intersection curves or in the case of only one   */
		    /* next refs are always inside.			    */
		    if (MDL_IS_TSEG_NEW(Ref1) || n2 == 0) 
		        Inside1 = TRUE;

		    AttrSetIntAttrib(&Ref1 -> Attr, "_Inside", Inside1);
		    MdlBoolPropagateRefInOut(TSrf, MDL_BOOL_NEAR_UV_EPS,
					     Refs, Ref1, n1 == 2, Inside1);
		    break;
	    }

	    switch (n2) {
	        default:
	        case 0:
		    break;                   /* Can have only one neighbor. */
	        case 1:
	        case 2:
		    Inside2 = MdlBoolRefInOtherModel(TSrf, OtherTSrf, Ref2,
						     n2 == 1, UV, OtherUV);
		    if (!InsideOtherModelTSrf)
		        Inside2 = !Inside2;
		    if (MDL_IS_TSEG_NEW(Ref2))   /* New intersection curves */
		        Inside2 = TRUE;	         /* are always inside.      */

		    AttrSetIntAttrib(&Ref2 -> Attr, "_Inside", Inside2);
		    MdlBoolPropagateRefInOut(TSrf, MDL_BOOL_NEAR_UV_EPS,
					     Refs, Ref2, n2 == 2, Inside2);
		    assert(Inside1 != Inside2);
		    break;
	    }
	}
    }

    /* If we have original trimming curves that are not classified now,     */
    /* this is due to original trimming loops that are isolated - the       */
    /* entire loop does not intersect with the other model.		    */
    for (Ref = *Refs; Ref != NULL; Ref = Ref -> Pnext) {
        CagdBType
	    Inside = AttrGetIntAttrib(Ref -> Attr, "_Inside");

	if (IP_ATTR_IS_BAD_INT(Inside) &&
	    MdlBoolClassifyNonInterTrimRef(TSrf, Ref,
					   (const MdlTrimSegRefStruct **) Refs,
					   &Inside)) {
	    if (!InsideOtherModelTSrf)
	        Inside = !Inside;

	    AttrSetIntAttrib(&Ref -> Attr, "_Inside", Inside);
	    MdlBoolPropagateRefInOut(TSrf, MDL_BOOL_NEAR_UV_EPS,
				     Refs, Ref, TRUE, Inside);
	    MdlBoolPropagateRefInOut(TSrf, MDL_BOOL_NEAR_UV_EPS,
				     Refs, Ref, FALSE, Inside);
	}
    }

#   ifdef DEBUG_MDL_DUMP_TSRF_TRIM_CRVS
        fprintf(stderr, "Refs before deletions ******************:\n");
	MdlDebugHandleTSrfRefCrvs(*Refs, TSrf, NULL, 1, FALSE);
#   endif /* DEBUG_MDL_DUMP_TSRF_TRIM_CRVS */

    /* Now purge all refs that are outside. */
    for (LastRef = NULL, Ref = *Refs; Ref != NULL; ) {
        CagdBType
	    Inside = AttrGetIntAttrib(Ref -> Attr, "_Inside");

#	ifdef DEBUG
	    if (IP_ATTR_IS_BAD_INT(Inside)) {
	        /* should not happen at this time. */
	        assert(0);
	    }
	    if (MDL_IS_TSEG_NEW(Ref))
		assert(Inside == TRUE);
#	endif /* DEBUG */

	if (!Inside) {
	    MdlBoolRemoveSegRefFromSegRefList(TSrf, Refs, Ref, TRUE,
					      MDL_BOOL_NEAR_UV_EPS);
	    if (LastRef == NULL)
	        Ref = *Refs;
	    else
	        Ref = LastRef -> Pnext;
	}
	else {
	    LastRef = Ref;
	    Ref = Ref -> Pnext;
	}	        
    }

#   ifdef DEBUG_MDL_DUMP_TSRF_TRIM_CRVS
	fprintf(stderr, "Refs after deletions ********************:\n");
	MdlDebugHandleTSrfRefCrvs(*Refs, TSrf, NULL, 2, FALSE);
#   endif /* DEBUG_MDL_DUMP_TSRF_TRIM_CRVS */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges the given list of trimming segments back into loops, in place.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Refs:   Set of references to trim curves segs to merge, in place.        *
*   TSrf:   Trimmed surface we now update its trimming loops.		     *
*   Tol:    Tolerance to match end points of trimming curves.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlLoopStruct *:  The newly formed loops of the given surface.           *
*****************************************************************************/
static MdlLoopStruct *MdlBoolMergeIntoLoops(MdlTrimSegRefStruct **Refs,
					    const MdlTrimSrfStruct *TSrf,
					    CagdRType Tol)
{
    MdlTrimSegRefStruct *Ref;
    MdlLoopStruct *L,
        *Loops = NULL;

#   ifdef DEBUG_MDL_DUMP_MERGE_TRIM_CRVS
        fprintf(stderr, "Refs before merges ******************:\n");
	MdlDebugHandleTSrfRefCrvs(*Refs, TSrf, NULL, 1, FALSE);
#   endif /* DEBUG_MDL_DUMP_MERGE_TRIM_CRVS */

    while (*Refs != NULL) {
        int EndSide = 1,
	    LoopStart = TRUE;
	CagdUVType UV, UVStart;
	CagdRType TMin, TMax, *R;
	const CagdCrvStruct *UVCrv;
	MdlTrimSegRefStruct *NextRef;

	IRIT_LIST_POP(Ref, *Refs);
	Ref -> Reversed = FALSE;

	L = MdlLoopNew(Ref);

	while (TRUE) {
	    if (Ref -> TrimSeg -> SrfFirst == TSrf)
	        UVCrv = Ref -> TrimSeg -> UVCrvFirst;
	    else {
	        assert(Ref -> TrimSeg -> SrfSecond == TSrf);
		UVCrv = Ref -> TrimSeg -> UVCrvSecond;
	    }

	    CagdCrvDomain(UVCrv, &TMin, &TMax);
	    R = CagdCrvEval(UVCrv, EndSide == 1 ? TMax : TMin);
	    CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);
	    if (LoopStart) {
		R = CagdCrvEval(UVCrv, EndSide == 1 ? TMin : TMax);
		CagdCoerceToE2(UVStart, &R, -1, UVCrv -> PType);
	        LoopStart = FALSE;
	    }
	    else if (IRIT_PT_APX_EQ_E2_EPS(UV, UVStart, Tol)) {
	        /* Done with this loop. */
	        break;
	    }

	    EndSide = MdlBoolFindSegRefInSegRefList(UV, TSrf, *Refs,
				(const MdlTrimSegRefStruct **) &NextRef, Tol);
	    if (NextRef == NULL) {
	        /* A closed loop of a single curve. */
	        break;
	    }
	    else {
	        MdlBoolRemoveSegRefFromSegRefList(TSrf, Refs, NextRef,
						  FALSE, Tol);
		NextRef -> Reversed = EndSide == 2;
		NextRef -> Pnext = NULL;
		Ref -> Pnext = NextRef;
		Ref = NextRef;
	    }
	}

	/* Make end points of adjacent curves in trimming loop identical. */
	if (L -> SegRefList -> Pnext != NULL) {
	    Ref = L -> SegRefList;
	    NextRef = Ref -> Pnext;

	    while (NextRef != NULL) {
	        MdlBoolMakeCrvsEndPtsSame(TSrf, Ref, Ref -> Reversed,
					  NextRef, !NextRef -> Reversed);

		Ref = NextRef;
		NextRef = NextRef -> Pnext;
	    }

	    /* Close the loop. */
	    MdlBoolMakeCrvsEndPtsSame(TSrf, Ref, Ref -> Reversed,
				      L -> SegRefList,
				      !L -> SegRefList -> Reversed);
	}

	IRIT_LIST_PUSH(L, Loops);
    }

#   ifdef DEBUG_MDL_DUMP_MERGE_TRIM_CRVS
        fprintf(stderr, "Refs after mergers ******************:\n");
	MdlDebugHandleTCrvLoops(NULL, Loops, NULL, FALSE);
#   endif /* DEBUG_MDL_DUMP_MERGE_TRIM_CRVS */

    return Loops;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   based on the new trimming curves, recreate the trimming loops of this    M
* trimmed surface.  At this stage we assume the new intersecting curves only M
* exists in the interior domain of trimmed surface (outside intersections    M
* were purged away).  Further, the old existing trimming curves were split   M
* at the locations they intersect with the new intersection curves.          M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:   A trimmed surface we need to update its trimming loops.          M
*   Tol:    Tolerance to match end points of trimming curves.		     M
*   InsideOtherModel:  TRUE if we seek the inside of the other model (i.e.   M
*           intersection operations), FALSE for outside (i.e. subtraction).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolClassifyTrimSrfLoops                                              M
*****************************************************************************/
int MdlBoolClassifyTrimSrfLoops(MdlTrimSrfStruct *TSrf,
				CagdRType Tol,
				CagdBType InsideOtherModel)
{
    MdlLoopStruct
        *Loop = TSrf -> LoopList;
    MdlTrimSegRefStruct
        *AllRefs = NULL;

    /* Collect all references to trimming curves into one large list. */
    for ( ; Loop != NULL; Loop = Loop -> Pnext) {
	AllRefs = CagdListAppend(Loop -> SegRefList, AllRefs);
	Loop -> SegRefList = NULL;
    }
    MdlLoopFreeList(TSrf -> LoopList);
    TSrf -> LoopList = NULL;

    /* Classify all old trimming curves of TSrf as in/outside other model   */
    /* by examining intersections of old trim curves with new ones.         */
    MdlBoolClassifyAtIntersections(&AllRefs, TSrf, Tol, InsideOtherModel);
    TSrf -> LoopList = MdlBoolMergeIntoLoops(&AllRefs, TSrf,
					     MDL_BOOL_NEAR_UV_EPS);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if this trimmed surface intersects the other model in this  M
* Boolean operation.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:   Trimmed surface to examine if it intersects the other model.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if intersects, FALSE otherwise                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolTrimSrfIntersects                                                 M
*****************************************************************************/
int MdlBoolTrimSrfIntersects(const MdlTrimSrfStruct *TSrf)
{
    MdlLoopStruct *Loop;

    for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
        MdlTrimSegRefStruct *SegRef;

	for (SegRef = Loop -> SegRefList;
	     SegRef != NULL;
	     SegRef = SegRef -> Pnext) {
	    if (MDL_IS_TSEG_NEW(SegRef))
	        return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the second reference to the given reference of a trimming curve     *
* if any.                                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   SegRef:        Reference to trimming curve segment we seek the second    *
*		   reference ,if exist.					     *
*   TSrf:          Trimmed surface SegRef belongs to.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSegRefStruct *:    The second reference ot NULL if none.          *
*****************************************************************************/
static MdlTrimSegRefStruct *MdlBoolGetOtherRef(
					   const MdlTrimSegRefStruct *SegRef,
					   const MdlTrimSrfStruct *TSrf)
{
    MdlTrimSrfStruct *OtherTSrf;
    MdlLoopStruct *OtherRefList;

    return MdlBoolGetOtherRef2(SegRef, TSrf, &OtherTSrf, &OtherRefList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the second reference to the given reference of a trimming curve     *
* if any.   Same as MdlBoolGetOtherRef but returns more info.                *
*                                                                            *
* PARAMETERS:                                                                *
*   SegRef:        Reference to trimming curve segment we seek the second    *
*		   reference ,if exist.					     *
*   TSrf:          Trimmed surface SegRef belongs to.			     *
*   OtherTSrf:     The other trimmed surface holding other ref., if any.     *
*   OtherLoop:     The other loop holding other ref., if any.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSegRefStruct *:    The second reference ot NULL if none.          *
*****************************************************************************/
static MdlTrimSegRefStruct *MdlBoolGetOtherRef2(
				     const MdlTrimSegRefStruct *SegRef,
				     const MdlTrimSrfStruct *TSrf,
				     MdlTrimSrfStruct **OtherTSrf,
				     MdlLoopStruct **OtherLoop)
{
    const MdlTrimSegStruct
        *TrimSeg = SegRef -> TrimSeg;

    if (TSrf == NULL)
        return NULL;

    if (TSrf == TrimSeg -> SrfFirst)
        *OtherTSrf = TrimSeg -> SrfSecond;
    else {
        assert(TSrf == TrimSeg -> SrfSecond);
	*OtherTSrf = TrimSeg -> SrfFirst;
    }

    if (*OtherTSrf == NULL)
        return NULL;

    for (*OtherLoop = (*OtherTSrf) -> LoopList;
	 *OtherLoop != NULL;
	 *OtherLoop = (*OtherLoop) -> Pnext) {
        MdlTrimSegRefStruct *OtherSegRef;

	for (OtherSegRef = (*OtherLoop) -> SegRefList;
	     OtherSegRef != NULL;
	     OtherSegRef = OtherSegRef -> Pnext) {
	    if (OtherSegRef -> TrimSeg == TrimSeg)
	        return OtherSegRef;
	}
    }

    /* Model is inconsistent - a reference to a TrimSeg in OtherTSrf, where */
    /* TrimSeg does not know about this TrimSeg.			    */
    assert(0);
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Mark all references of trimming curves of TSrf to have in/outside state  *
* of Inside.						                     *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:     Trimmed surface to update all it trimming curves references    *
*             to the in/outside state of Inside.		             *
*   Inside:   TRUE if TSrf is inside the model, FALSE otherwise.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MdlBoolSetClassifyNonInterOneTrimSrf(MdlTrimSrfStruct *TSrf,
						 CagdBType Inside)
{
    MdlLoopStruct *Loop;

    for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
        MdlTrimSegRefStruct *SegRef;

	for (SegRef = Loop -> SegRefList;
	     SegRef != NULL;
	     SegRef = SegRef -> Pnext) {
	    AttrSetIntAttrib(&SegRef -> Attr, "_Inside", Inside);
	}
    }

    AttrSetIntAttrib(&TSrf -> Attr, "_Inside", Inside);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Classify references of trimming curves in trimmed surfaces that are      M
* not interested in this Boolean operation.                                  M
*   1. Go over all refs and search for a reference of an old intersection    M
*      that is classified on one srf side only and propagate to the second   M
*      srf side.							     M
*   2. Go over all trimming references in the second srf side and propagated M
*      the information.							     M
*   3. While there are unclassified references go to 1.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model: Model in which to cliassify the non-intersecting trimming curves. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlBoolClassifyNonInterTrimSrfs                                          M
*****************************************************************************/
void MdlBoolClassifyNonInterTrimSrfs(MdlModelStruct *Model)
{
    int Iter,
        HasUnclassifiedSegRef = TRUE,
        MaxIters = CagdListLength(Model -> TrimSrfList);
    MdlTrimSrfStruct *TSrf, *OtherTSrf;

    for (Iter = 0; Iter < MaxIters && HasUnclassifiedSegRef; Iter++) {
	/* Go over all classified trimming curves' references in            */
	/* intersecting surfaces and propagate to the matching surfaces     */
	/* that are using these same trimming curves.			    */
	/*  This, until all trimming curves' references are classified.	    */
        HasUnclassifiedSegRef = FALSE;
	for (TSrf = Model -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    MdlLoopStruct *Loop;
	    CagdBType
	        SrfInside = FALSE;

	    for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	        MdlTrimSegRefStruct *SegRef, *OtherSegRef;

		for (SegRef = Loop -> SegRefList;
		     SegRef != NULL;
		     SegRef = SegRef -> Pnext) {
		    CagdBType Inside2,
		        Inside = AttrGetIntAttrib(SegRef -> Attr, "_Inside");

		    if (!IP_ATTR_IS_BAD_INT(Inside)) { /* A classified ref. */
		        if (Inside)
			    SrfInside = TRUE; /* TSrf has one inside ref. */

		        OtherSegRef = MdlBoolGetOtherRef(SegRef, TSrf);
			if (OtherSegRef != NULL) {
			    Inside2 = AttrGetIntAttrib(OtherSegRef -> Attr,
						       "_Inside");

			    if (IP_ATTR_IS_BAD_INT(Inside2)) {
			        HasUnclassifiedSegRef = TRUE;

				if (Inside) {
				    MdlTrimSegStruct
				        *TrimSeg = SegRef -> TrimSeg;

				    if (TSrf == TrimSeg -> SrfFirst)
				        OtherTSrf = TrimSeg -> SrfSecond;
				    else {
				        assert(TSrf == TrimSeg -> SrfSecond);
					OtherTSrf = TrimSeg -> SrfFirst;
				    }

				    /* Update OtherTSrf refs with Inside. */
				    MdlBoolSetClassifyNonInterOneTrimSrf(
							   OtherTSrf, Inside);
				}
			    }
			}
		    }   
		}
	    }

	    if (!SrfInside) {
	        /* Mark TSrf for deletion. */
	        AttrSetIntAttrib(&TSrf -> Attr, "_Inside", FALSE);
	    }
	}
    }

    /* Remove all trimmed surfaces that are not (even partially) inside. */
    while (Model -> TrimSrfList != NULL &&
	   AttrGetIntAttrib(Model -> TrimSrfList -> Attr,
			    "_Inside") == FALSE) {
        OtherTSrf = Model -> TrimSrfList -> Pnext;
	MdlTrimSrfFree(Model -> TrimSrfList);
	Model -> TrimSrfList = OtherTSrf;
    }

    if (Model -> TrimSrfList != NULL) {
	for (TSrf = Model -> TrimSrfList; TSrf -> Pnext != NULL; ) {
	    if (AttrGetIntAttrib(TSrf -> Pnext -> Attr,
				 "_Inside") == FALSE) {
	        OtherTSrf = TSrf -> Pnext;
		TSrf -> Pnext = OtherTSrf -> Pnext;
		MdlTrimSrfFree(OtherTSrf);
	    }
	    else
		TSrf = TSrf -> Pnext;
	}
    }
}
