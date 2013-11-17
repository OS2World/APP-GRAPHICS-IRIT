/******************************************************************************
* Cagd2Pl2.c - more support for cagd to polygons (for cagd2ply.c).	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 2001.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define CAGD_A2PPTS_ALLOC_BLOCK		100

#define CAGD_ULIST_PUSH(New, List) { (New) -> UPnext = (List); (List) = (New);}
#define CAGD_ULIST_POP(Head, List) { (Head) = (List); \
				     (List) = (List) -> UPnext; \
				     (Head) -> UPnext = NULL; }

#define CAGD_VLIST_PUSH(New, List) { (New) -> VPnext = (List); (List) = (New);}
#define CAGD_VLIST_POP(Head, List) { (Head) = (List); \
				     (List) = (List) -> VPnext; \
				     (Head) -> VPnext = NULL; }

typedef struct CagdA2PPtStruct {
    struct CagdA2PPtStruct *UPnext, *VPnext;
    int UIndex, VIndex;
    CagdSrfPtStruct SrfPt;
} CagdA2PPtStruct;

typedef struct CagdA2PGridStruct {
    const CagdSrfStruct *Srf;

    CagdBType ClosedInU, ClosedInV;
    CagdRType UMin, UMax, VMin, VMax;

    /* Booleans values prescribing the U / V indices that are C^1 discont. */
    CagdBType DiscontUVals[CAGD2PLY_MAX_SUBDIV_INDEX + 1];
    CagdBType DiscontVVals[CAGD2PLY_MAX_SUBDIV_INDEX + 1];

    /* Association between U / V indices and surface parameter values. */
    CagdRType SubdivUVals[CAGD2PLY_MAX_SUBDIV_INDEX + 1];
    CagdRType SubdivVVals[CAGD2PLY_MAX_SUBDIV_INDEX + 1];

    /* The grid will be expressed as a U vector of pointers to UV linked    */
    /* list values and a V vector of pointers to U linked list UV values.   */
    CagdA2PPtStruct *UGridVec[CAGD2PLY_MAX_SUBDIV_INDEX + 1];
    CagdA2PPtStruct *VGridVec[CAGD2PLY_MAX_SUBDIV_INDEX + 1];
} CagdA2PGridStruct;

IRIT_STATIC_DATA CagdA2PPtStruct
    *GlblA2PPtList = NULL;

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugCagdPrintA2PGrid, FALSE);
#endif /* DEBUG */

#if defined(ultrix) && defined(mips)
static int PtSortCmprU(VoidPtr VPt1, VoidPtr VPt2);
static int PtSortCmprV(VoidPtr VPt1, VoidPtr VPt2);
#else
static int PtSortCmprU(const VoidPtr VPt1, const VoidPtr VPt2);
static int PtSortCmprV(const VoidPtr VPt1, const VoidPtr VPt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes a data structure to efficiently save UV sample locations on  M
* a surface and allow fast fetching of them as well.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to prepare the grid structure point sampling support.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   struct CagdA2PGridStruct *: The structure if successful, NULL otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*****************************************************************************/
struct CagdA2PGridStruct *CagdSrfA2PGridInit(const CagdSrfStruct *Srf)
{
    CagdA2PGridStruct
        *A2PGrid = IritMalloc(sizeof(CagdA2PGridStruct));

    IRIT_ZAP_MEM(A2PGrid, sizeof(CagdA2PGridStruct));

    A2PGrid -> Srf = Srf;

    A2PGrid -> ClosedInU = CagdIsClosedSrf(A2PGrid -> Srf, CAGD_CONST_U_DIR);
    A2PGrid -> ClosedInV = CagdIsClosedSrf(A2PGrid -> Srf, CAGD_CONST_V_DIR);

    CagdSrfDomain(A2PGrid -> Srf,
		  &A2PGrid -> UMin,
		  &A2PGrid -> UMax,
		  &A2PGrid -> VMin,
		  &A2PGrid -> VMax);

    return A2PGrid;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the grid data structure.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   A2PGrid:   The data structure to free.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridFree                                                       M
*****************************************************************************/
void CagdSrfA2PGridFree(struct CagdA2PGridStruct *A2PGrid)
{
    int i;
    CagdA2PPtStruct *Pt;

    /* Push all points we have in this grid on a global cache.  Note that   */
    /* the same points are linked along U and V so free the once only!      */
    for (i = 0; i <= CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
        if (A2PGrid -> UGridVec[i] != NULL) {
	    for (Pt = A2PGrid -> UGridVec[i];
		 Pt -> VPnext != NULL;
		 Pt = Pt -> VPnext);

	    Pt -> VPnext = GlblA2PPtList;
	    GlblA2PPtList = A2PGrid -> UGridVec[i];
	}
    }

    IritFree(A2PGrid);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert one UV location to sample the surfaces, into the data grid.       M
*                                                                            *
* PARAMETERS:                                                                M
*   A2PGrid:         This grid data struction.                               M
*   UIndex, VIndex:  Indices of these U / V parameter values.                M
*   u, v:            The parameter values.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridInsertUV                                                   M
*****************************************************************************/
void CagdSrfA2PGridInsertUV(struct CagdA2PGridStruct *A2PGrid,
			    int UIndex,
			    int VIndex,
			    CagdRType u,
			    CagdRType v)
{
    CagdA2PPtStruct *Pt;

    /* Verify domain of indices. */
    assert(UIndex >= 0 &&
	   UIndex <= CAGD2PLY_MAX_SUBDIV_INDEX &&
	   VIndex >= 0 &&
	   VIndex <= CAGD2PLY_MAX_SUBDIV_INDEX);

    if (A2PGrid -> UGridVec[UIndex] != NULL) {
        /* Verify Pts in same UIndex have save U values. */
        assert(UIndex == A2PGrid -> UGridVec[UIndex] -> UIndex &&
	       IRIT_APX_UEQ(A2PGrid -> UGridVec[UIndex] -> SrfPt.Uv[0], u));

	/* Skip if this point was just inserted last time. */
	if (IRIT_APX_UEQ(A2PGrid -> UGridVec[UIndex] -> SrfPt.Uv[1], v))
	    return;
    }

    A2PGrid -> SubdivUVals[UIndex] = u;
    A2PGrid -> SubdivVVals[VIndex] = v;

    /* Make sure we have allocated Pt structs. */
    if (GlblA2PPtList == NULL) {
	int i;
        CagdA2PPtStruct
	    *Pts = IritMalloc(sizeof(CagdA2PPtStruct) *
			      CAGD_A2PPTS_ALLOC_BLOCK);
	GlblA2PPtList = &Pts[0];

	/* Allocate a block of CAGD_A2PPTS_ALLOC_BLOCK such structures. */
	for (i = 0; i < CAGD_A2PPTS_ALLOC_BLOCK - 1; i++)
	    Pts[i].VPnext = &Pts[i + 1];
	Pts[i].VPnext = NULL;
    }
    CAGD_VLIST_POP(Pt, GlblA2PPtList);

    Pt -> SrfPt.Pnext = NULL;
    Pt -> SrfPt.Attr = NULL;
    Pt -> SrfPt.Uv[0] = u;
    Pt -> SrfPt.Uv[1] = v;
    Pt -> UPnext = Pt -> VPnext = NULL;
    Pt -> UIndex = UIndex;
    Pt -> VIndex = VIndex;

    CAGD_VLIST_PUSH(Pt, A2PGrid -> UGridVec[UIndex]);

    if (A2PGrid -> ClosedInU) {
        /* If uv is on a UMin/UMax boundary - place on other side as well.  */
        A2PGrid -> ClosedInU = FALSE; /* To prevent from recursion forever. */

        if (UIndex == 0) {
	    CagdSrfA2PGridInsertUV(A2PGrid, CAGD2PLY_MAX_SUBDIV_INDEX,
				   VIndex, A2PGrid -> UMax, v);
	}
	else if (UIndex == CAGD2PLY_MAX_SUBDIV_INDEX) {
	    CagdSrfA2PGridInsertUV(A2PGrid, 0, VIndex, A2PGrid -> UMin, v);
	}

        A2PGrid -> ClosedInU = TRUE;
    }

    if (A2PGrid -> ClosedInV) {
        /* If uv is on a VMin/VMax boundary - place on other side as well.  */
        A2PGrid -> ClosedInV = FALSE; /* To prevent from recursion forever. */

        if (VIndex == 0) {
	    CagdSrfA2PGridInsertUV(A2PGrid, UIndex, CAGD2PLY_MAX_SUBDIV_INDEX,
				   u, A2PGrid -> VMax);
	}
	else if (VIndex == CAGD2PLY_MAX_SUBDIV_INDEX) {
	    CagdSrfA2PGridInsertUV(A2PGrid, UIndex, 0, u, A2PGrid -> VMin);
	}

        A2PGrid -> ClosedInV = TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to points.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two points.              *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int PtSortCmprU(VoidPtr VPt1, VoidPtr VPt2)
#else
static int PtSortCmprU(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((CagdA2PPtStruct **) VPt2)) -> SrfPt.Uv[0] -
	       (*((CagdA2PPtStruct **) VPt1)) -> SrfPt.Uv[0];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to points.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two points.              *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int PtSortCmprV(VoidPtr VPt1, VoidPtr VPt2)
#else
static int PtSortCmprV(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((CagdA2PPtStruct **) VPt2)) -> SrfPt.Uv[1] -
	       (*((CagdA2PPtStruct **) VPt1)) -> SrfPt.Uv[1];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Once all surface sampled points are insert, this function is invoked to  M
* process the data for fast fetch.					     M
*   Two vectors, UGridVec and VGridVec are processed and updated to hold     M
* evaluate surface locations. Each entry in UGridVec (respectively in 	     M
* VGridVec) will hold a linked list of surface evaluated locations sort in V M
* for that particular U value.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   A2PGrid:         This grid data structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridProcessUV                                                  M
*****************************************************************************/
void CagdSrfA2PGridProcessUV(struct CagdA2PGridStruct *A2PGrid)
{
    int i, j, n;
    CagdA2PPtStruct *Pt, *Pt2,
        /* 4 times the maximal length due to periodic boundaries! */
        **PtsVec = IritMalloc(sizeof(CagdA2PPtStruct *) * 4 *
			      (CAGD2PLY_MAX_SUBDIV_INDEX + 1));
    const CagdSrfStruct
        *Srf = A2PGrid -> Srf;

    /* For every non empty entry in UGrid, sort points along V from the    */
    /* UGridVec vector, and build a sorted linked list in V.		   */
    for (i = 0; i <= CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
        if (A2PGrid -> UGridVec[i] == NULL) 
	    continue;

	for (Pt = A2PGrid -> UGridVec[i], n = 0;
	     Pt != NULL;
	     Pt = Pt -> VPnext) {
	    PtsVec[n++] = Pt;
	    assert(IRIT_APX_UEQ(PtsVec[0] -> SrfPt.Uv[0], Pt -> SrfPt.Uv[0]) &&
		   n < 4 * (CAGD2PLY_MAX_SUBDIV_INDEX + 1));
	}

	qsort(PtsVec, n, sizeof(CagdA2PPtStruct *), PtSortCmprV);

	/* Keep first point and push on the VGridVec list as well, for    */
	/* the 2nd sort in the other direction.				  */
	Pt = PtsVec[0];
	Pt -> VPnext = NULL;
	CAGD_ULIST_PUSH(Pt, A2PGrid -> VGridVec[Pt -> VIndex]);

	for (j = 1; j < n; j++) {
	    Pt2 = PtsVec[j];

	    /* Data points better be similar in U now. */
	    assert(IRIT_APX_UEQ(Pt -> SrfPt.Uv[0], Pt2 -> SrfPt.Uv[0]));

	    /* Process the sorted data: */
	    if (IRIT_APX_UEQ(Pt -> SrfPt.Uv[1], Pt2 -> SrfPt.Uv[1])) {
	        /* If same v, purge pt. */
	        CAGD_VLIST_PUSH(Pt2, GlblA2PPtList);
	    }
	    else {
	        /* Otherwise push back in order and eval. */	        
	        CAGD_VLIST_PUSH(Pt2, Pt);

		/* And push on the VGridVec list as well, for the 2nd sort. */
	        CAGD_ULIST_PUSH(Pt2, A2PGrid -> VGridVec[Pt2 -> VIndex]);
	    }
	}
	A2PGrid -> UGridVec[i] = Pt;
    }

    /* Make sure we have discontinuity lines marked properly before eval. */
    if (CAGD_IS_BSPLINE_SRF(Srf)) {
        for (i = 1; i < CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
	    CagdRType t;

	    if (A2PGrid -> UGridVec[i] != NULL) {
	        t = A2PGrid -> SubdivUVals[i];

		if (BspKnotFindMult(Srf -> UKnotVector, Srf -> UOrder,
				    Srf -> ULength, t) >= Srf -> UOrder - 1 &&
		    BspSrfIsC1DiscontAt(Srf, CAGD_CONST_U_DIR, t))
		    A2PGrid -> DiscontUVals[i] = TRUE;
	    }
	    if (A2PGrid -> VGridVec[i] != NULL) {
	        t = A2PGrid -> SubdivVVals[i];

		if (BspKnotFindMult(Srf -> VKnotVector, Srf -> VOrder,
				    Srf -> VLength, t) >= Srf -> VOrder - 1 &&
		    BspSrfIsC1DiscontAt(Srf, CAGD_CONST_V_DIR, t))
		    A2PGrid -> DiscontVVals[i] = TRUE;
	    }
	}
    }

    /* Resort the same points, now along U, from the VGridVec.  Also eval.  */
    /* the Euclidean location and normal (if not C^1 discont.) of the pt.   */
    for (i = 0; i <= CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
        if (A2PGrid -> VGridVec[i] == NULL) 
	    continue;

	for (Pt = A2PGrid -> VGridVec[i], n = 0;
	     Pt != NULL;
	     Pt = Pt -> UPnext) {
	    PtsVec[n++] = Pt;
	    assert(IRIT_APX_UEQ(PtsVec[0] -> SrfPt.Uv[1], Pt -> SrfPt.Uv[1]) &&
		   n < 4 * (CAGD2PLY_MAX_SUBDIV_INDEX + 1));
	}

	qsort(PtsVec, n, sizeof(CagdA2PPtStruct *), PtSortCmprU);

	Pt = NULL;
	PtsVec[0] -> UPnext = NULL;
	for (j = 0; j < n; j++) {
	    CagdRType *R;

	    Pt2 = PtsVec[j];

	    /* Data points better be different in U now. */
	    assert(Pt == NULL ||
		   (Pt -> SrfPt.Uv[0] != Pt2 -> SrfPt.Uv[0] &&
		    IRIT_APX_UEQ(Pt -> SrfPt.Uv[1], Pt2 -> SrfPt.Uv[1])));

	    /* Evaluate position and normal .*/
	    R = CagdSrfEval(Srf, Pt2 -> SrfPt.Uv[0], Pt2 -> SrfPt.Uv[1]);
	    CagdCoerceToE3(Pt2 -> SrfPt.Pt, &R, -1, Srf -> PType);

	    if (A2PGrid -> DiscontUVals[Pt2 -> UIndex] ||
		A2PGrid -> DiscontVVals[Pt2 -> VIndex])
	        Pt2 -> SrfPt.Nrml[0] = IRIT_INFNTY;
	    else {
	        CagdVecStruct
		    *N = CagdSrfEffiNrmlEval(Pt2 -> SrfPt.Uv[0],
					     Pt2 -> SrfPt.Uv[1], TRUE);

		if (IRIT_PT_APX_EQ_ZERO_EPS(N -> Vec, IRIT_EPS))
		    Pt2 -> SrfPt.Nrml[0] = IRIT_INFNTY;
		else
		    IRIT_VEC_COPY(Pt2 -> SrfPt.Nrml, N -> Vec);
	    }

	    /* Push back in order and eval. */	        
	    CAGD_ULIST_PUSH(Pt2, Pt);
	}
	A2PGrid -> VGridVec[i] = Pt;
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCagdPrintA2PGrid, FALSE) {
	    for (i = 0; i <= CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
	        if ((Pt = A2PGrid -> UGridVec[i]) != NULL) {
		  fprintf(stderr, "UGrid[%4d], U = %6.3f, V =",
			  i, Pt -> SrfPt.Uv[0]);
		  for ( ; Pt != NULL; Pt = Pt -> VPnext)
		      fprintf(stderr, " %6.3f", Pt -> SrfPt.Uv[1]);
		  fprintf(stderr, "\n");
		}
	    }

	    fprintf(stderr, "\n");

	    for (i = 0; i <= CAGD2PLY_MAX_SUBDIV_INDEX; i++) {
	        if ((Pt = A2PGrid -> VGridVec[i]) != NULL) {
		    fprintf(stderr, "VGrid[%4d], V = %6.3f, U =",
			    i, Pt -> SrfPt.Uv[1]);
		    for ( ; Pt != NULL; Pt = Pt -> UPnext)
		        fprintf(stderr, " %6.3f", Pt -> SrfPt.Uv[0]);
		    fprintf(stderr, "\n");
		}
	    }
	}
    }
#endif /* DEBUG */

    IritFree(PtsVec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetch a all sampled points in the rectangle region defined               M
* from [UIndex1, VIndex1]  to  [UIndex2, VIndex2].			     M
*                                                                            *
* PARAMETERS:                                                                M
*   A2PGrid:               This grid data structure.                         M
*   UIndex1, VIndex1:      Start point of rectangle domain.		     M
*   UIndex2, VIndex2:      End point of rectangle domain.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfPtStruct *:  List of points found around the rectangle,	     M
*		        or NULL if error.			             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridFetchRect                                                  M
*****************************************************************************/
CagdSrfPtStruct *CagdSrfA2PGridFetchRect(struct CagdA2PGridStruct *A2PGrid,
					 int UIndex1,
					 int VIndex1,
					 int UIndex2,
					 int VIndex2)
{
    CagdSrfPtStruct *VPosLast, *UPosLast, *VNegLast, *UNegLast,
        *UPos = CagdSrfA2PGridFetchPts(A2PGrid, CAGD_CONST_U_DIR,
				       UIndex1, UIndex2, VIndex2,
				       &UPosLast, FALSE),
        *UPosNext = UPos == NULL ? NULL : UPos -> Pnext,
        *VNeg = CagdSrfA2PGridFetchPts(A2PGrid, CAGD_CONST_V_DIR,
				       VIndex1, VIndex2, UIndex2,
				       &VNegLast, TRUE),
        *UNeg = CagdSrfA2PGridFetchPts(A2PGrid, CAGD_CONST_U_DIR,
				       UIndex1, UIndex2, VIndex1,
				       &UNegLast, TRUE),
        *VPos = CagdSrfA2PGridFetchPts(A2PGrid, CAGD_CONST_V_DIR,
				       VIndex1, VIndex2, UIndex1,
				       &VPosLast, FALSE);

    if (UPos == NULL ||
	UPosLast != VNeg ||
	VNegLast != UNeg ||
	UNegLast != VPos ||
	VPosLast != UPos)
	return NULL;

    /* The last invocation of VPos above ends at UPos and make it point     */
    /* to a NULL as Pnext.  Fix this and Make the list circular.            */
    UPos -> Pnext = UPosNext;

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCagdPrintA2PGrid, FALSE) {
	    CagdSrfPtStruct
	        *Pt = UPos;

	    fprintf(stderr, "Rect Pt:\n");
	    do {
	        fprintf(stderr, "\t[%6.3f,  %6.3f]\n", Pt -> Uv[0], Pt -> Uv[1]);
		Pt = Pt -> Pnext;
	    }
	    while (Pt != UPos && Pt != NULL);

	    assert(Pt != NULL);
	}
    }
#endif /* DEBUG */

    return UPos;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetch an interval of sampled points along isoparametric direction.       M
*                                                                            *
* PARAMETERS:                                                                M
*   A2PGrid:               This grid data structure.                         M
*   Dir:                   Are we to fetch sampled points along U or V       M
*                          direction?					     M
*   StartIndex, EndIndex:  Limit indices along this direction to fetch.      M
*   OtherDirIndex:         Index along the other direction to fetch points.  M
*   LastPt:		   Will be set to last point in linked list.         M
*   Reversed:              TRUE to fetch the linked list reversed.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfPtStruct *:   Sampled points.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfA2PGridInit                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfA2PGridFetchPts                                                   M
*****************************************************************************/
CagdSrfPtStruct *CagdSrfA2PGridFetchPts(struct CagdA2PGridStruct *A2PGrid,
					CagdSrfDirType Dir,
					int StartIndex,
					int EndIndex,
					int OtherDirIndex,
					CagdSrfPtStruct **LastPt,
					CagdBType Reversed)
{
    int IDir;
    CagdA2PPtStruct *Pt, *PtHead, *PtNext;
    CagdRType StartParam, EndParam;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    PtHead = A2PGrid -> VGridVec[OtherDirIndex];
	    StartParam = A2PGrid -> SubdivUVals[StartIndex] - IRIT_UEPS;
	    EndParam = A2PGrid -> SubdivUVals[EndIndex] - IRIT_UEPS;
	    IDir = 0;
	    break;
	case CAGD_CONST_V_DIR:
	    PtHead = A2PGrid -> UGridVec[OtherDirIndex];
	    StartParam = A2PGrid -> SubdivVVals[StartIndex] - IRIT_UEPS;
	    EndParam = A2PGrid -> SubdivVVals[EndIndex] - IRIT_UEPS;
	    IDir = 1;
	    break;
        default:
	    assert(0);
	    return NULL;
    }

    /* Find starting position. */
    while (PtHead != NULL && PtHead -> SrfPt.Uv[IDir] < StartParam) {
        PtHead = IDir == 0 ? PtHead -> UPnext : PtHead -> VPnext;
    }
    if (PtHead == NULL)
        return NULL;

    /* Chain all points until end position into a linked list. */
    for (Pt = PtHead; Pt != NULL && Pt -> SrfPt.Uv[IDir] < EndParam; ) {
        PtNext = IDir == 0 ? Pt -> UPnext : Pt -> VPnext;
	if (Reversed)
	    PtNext -> SrfPt.Pnext = &Pt -> SrfPt;
	else
	    Pt -> SrfPt.Pnext = &PtNext -> SrfPt;
	Pt = PtNext;
    }

    if (Reversed) {
        PtHead -> SrfPt.Pnext = NULL;

	*LastPt = &PtHead -> SrfPt;
	return &Pt -> SrfPt;
    }
    else {
        Pt -> SrfPt.Pnext = NULL;

	*LastPt = &Pt -> SrfPt;
	return &PtHead -> SrfPt;
    }
}
