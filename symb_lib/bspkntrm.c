/******************************************************************************
* BspKntRm.c - Knot removal for Bspline curves.	   	                      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by: Gershon Elber			 	  Vec 0.2, Sep 2005.  *
******************************************************************************/

#include "cagd_lib.h"
#include "extra_fn.h"
#include "symb_loc.h"

#define SYMB_KNOT_RM_CLEAN_TOL	1e-10

static CagdCrvStruct *SymbRmKntBspCrvTestKnot(const CagdCrvStruct *Crv,
					      CagdRType *Error,
					      int KnotIndex);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test the error that is introduced by removing the KnotIndex knot from    *
* the given curve.	                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         To try and see what error will be introduced by removing    *
*		 the KnotIndex knot.					     *
*   Error:       The error introduced by this removal, in L-infinity terms.  *
*   KnotIndex:   The index of the knot to try and remove.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct:   The reduced curve with one less knot.                   *
*****************************************************************************/
static CagdCrvStruct *SymbRmKntBspCrvTestKnot(const CagdCrvStruct *Crv,
					      CagdRType *Error,
					      int KnotIndex)
{
    CagdBType SolveFor;
    CagdPointType
	PType = Crv -> PType;
    int i, j, MinIndex, MaxIndex, MinRedIndex, MaxRedIndex,
	n = -1,
	m = -1,
	Length = Crv -> Length,
	Order = Crv -> Order;
    CagdRType *r,
	*M = NULL;
    CagdCrvStruct *ReducedCrv, *RefCrv;
    BspKnotAlphaCoeffStruct *A;

    *Error = 0.0;

    if (KnotIndex < Order || KnotIndex >= Length) {
        SYMB_FATAL_ERROR(SYMB_ERR_WRONG_KNOT_INDEX);
        return NULL;
    }

    /* Create the reduced knot sequence. */
    ReducedCrv = BspCrvNew(Length - 1, Order, Crv -> PType);
    CAGD_GEN_COPY(ReducedCrv -> KnotVector, Crv -> KnotVector,
		  sizeof(CagdRType) * KnotIndex);
    CAGD_GEN_COPY(&ReducedCrv -> KnotVector[KnotIndex],
		  &Crv -> KnotVector[KnotIndex + 1],
		  sizeof(CagdRType) * (Order + Length - KnotIndex - 1));
    
    /* Compute the Alpha refinement matrix. */
    A = BspKnotEvalAlphaCoef(Order, ReducedCrv -> KnotVector, Length - 1,
			     Crv -> KnotVector, Length, Crv -> Periodic);

    /* Create a least-squares problem to solve for the "best" coefficients */
    /* in the new low dimensional space.			           */
    MinIndex = MinRedIndex = Length + 1;     /* Find affected coefficients. */
    MaxIndex = MaxRedIndex = 0;
    for (i = 0; i < A -> RefLength; i++) {
        if (A -> ColLength[i] != 1) {
	    if (MinIndex > i)
		MinIndex = i;
	    if (MaxIndex < i)
		MaxIndex = i;

	    if (MinRedIndex > A -> ColIndex[i])
	        MinRedIndex = A -> ColIndex[i];
	    if (MaxRedIndex < A -> ColIndex[i] + A -> ColLength[i] - 1)
	        MaxRedIndex = A -> ColIndex[i] + A -> ColLength[i] - 1;
	}
    }

    /* Check if this knot is of valid continuity and if not (C-1 discont.)  */
    /* Simply copy the curve without it.                                    */
    if (MinIndex > MaxIndex) {
        for (i = 1; i < A -> RefLength; i++) {
	    if (A -> ColIndex[i] == A -> ColIndex[i - 1]) {
	        /* We have a C-1 discontinuity here. */
	        MinIndex = MaxIndex = MinRedIndex = i - 1;
		MaxRedIndex = MinRedIndex - 1;
		break;
	    }
	}

	if (MinIndex > MaxIndex) {
	    SYMB_FATAL_ERROR(SYMB_ERR_WRONG_KNOT_INDEX);
	    return NULL;
	}

	SolveFor = FALSE;
    }
    else {
        /* We need to solve for (MaxIndex - MinIndex + 1) new coefficients. */
        MinIndex--;
	MaxIndex++;
	n = MaxIndex - MinIndex + 1;
	m = MaxRedIndex - MinRedIndex + 1;
	M = (CagdRType *) IritMalloc(sizeof(CagdRType) * n * m);
	for (j = 0, r = M; j < n; j++) {
	    for (i = 0; i < m; i++) {
	        *r++ = A -> RowsTransp[MinIndex + j][MinRedIndex + i];
	    }
	}
	BspKnotFreeAlphaCoef(A);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintReduceMat, FALSE) {
	        printf("*------------- M ------------*\n");
		for (j = 0, r = M; j < n; j++) {
		    for (i = 0; i < m; i++)
		        printf("%9.6f ", *r++);
		    printf("\n");
		}
	    }
	}
#	endif /* DEBUG */

	if (IRIT_FABS(SvdLeastSqr(M, NULL, NULL, n, m)) < IRIT_UEPS) {
	    IritFree(M);
	    CagdCrvFree(ReducedCrv);
	    return NULL;
	}
	SolveFor = TRUE;
    }

    /* Time to update the control polygon. */
    for (i = !CAGD_IS_RATIONAL_PT(PType);
	 i <= CAGD_NUM_OF_PT_COORD(PType);
	 i++) {
        if (MinIndex > 0) {
	    /* Copy the first half */
	    CAGD_GEN_COPY(&ReducedCrv -> Points[i][0],
			  &Crv -> Points[i][0],
			  sizeof(CagdRType) * MinIndex);
	}
 
	/* Get least sqaures middle solution directly into the control poly. */
	if (SolveFor)
	    SvdLeastSqr(NULL, &ReducedCrv -> Points[i][MinRedIndex],
			&Crv -> Points[i][MinIndex], n, m);

        if (Length - MaxIndex - 1 > 0) {
	    /* Copy the last half */
	    CAGD_GEN_COPY(&ReducedCrv -> Points[i][MaxRedIndex + 1],
			  &Crv -> Points[i][MaxIndex + 1],
			  sizeof(CagdRType) * (Length - MaxIndex - 1));
	}   
    }

    if (SolveFor)
        IritFree(M);

    /* Time to refine the reduced curve and measure the error. */
    RefCrv = CagdCrvRefineAtParams(ReducedCrv, FALSE,
				   &Crv -> KnotVector[KnotIndex], 1);
    for (i = MinRedIndex, *Error = 0.0; i <= MaxRedIndex; i++) {
        for (j = !CAGD_IS_RATIONAL_PT(PType);
	     j <= CAGD_NUM_OF_PT_COORD(PType);
	     j++) {
	    CagdRType
		E = IRIT_FABS(Crv -> Points[j][i] - RefCrv -> Points[j][i]);

	    if (*Error < E)
	        *Error = E;
	}
    }

    CagdCrvFree(RefCrv);

    return ReducedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Remove knots while Tolerance is kept.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         Curve to remove knots from.                                M
*   Tolerance:    Desired accuracy to be kept, in L-infinity norm.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The new curve after removal.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRmKntBspCrvCleanKnots					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRmKntBspCrvRemoveKnots                                               M
*****************************************************************************/
CagdCrvStruct *SymbRmKntBspCrvRemoveKnots(const CagdCrvStruct *CCrv,
					  CagdRType Tolerance)
{
    int i;
    CagdRType Error;
    CagdCrvStruct *TCrv, *Crv;

    if (!CAGD_IS_BSPLINE_CRV(CCrv)) {
	SYMB_FATAL_ERROR(SYMB_ERR_BSP_CRV_EXPECT);
	return FALSE;
    }

    Crv = CagdCrvCopy(CCrv);

    for (i = Crv -> Order; i < Crv -> Length; ) {
        TCrv = SymbRmKntBspCrvTestKnot(Crv, &Error, i);

	if (Error < Tolerance) {
	    CagdCrvFree(Crv);
	    Crv = TCrv;
	}
	else {
	    CagdCrvFree(TCrv);
	    i++;
	}
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Remove only knots which do not change the given curve.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  Curve to remove knot from.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The new curve after removal.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRmKntBspCrvRemoveKnots, SymbRmKntBspCrvRemoveKnotsError              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRmKntBspCrvCleanKnots                                                M
*****************************************************************************/
CagdCrvStruct *SymbRmKntBspCrvCleanKnots(const CagdCrvStruct *Crv)
{
    return SymbRmKntBspCrvRemoveKnots(Crv, SYMB_KNOT_RM_CLEAN_TOL);
}
