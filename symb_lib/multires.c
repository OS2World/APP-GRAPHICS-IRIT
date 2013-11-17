/*****************************************************************************
*   A decomposition of a Bspline curve into multi resolution hierarchy.      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_loc.h"
#include "extra_fn.h"

#define MAX_AFFECTED_DIST 4.0
#define END_COND_EPS	  1e-8

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a hierarch of knot sequence for the given Bspline curve and   M
* until no interior knot exists in the knot sequence.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to construct a hierarchy of knot sequences.	     M
*   Discont:     Should we preserve discontinuities as much as we can?	     M
*   KVList:      A vector of pointers to knot sequences in the hierarcy	     M
*   KVListSizes: Length of each knot sequence in vector KVList.		     M
*   KVListSize:  Size of KVList - number of knot sequences in hierarch.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvMultiResDecomp, SymbCrvMultiResDecomp2                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResKVBuild                                                   M
*****************************************************************************/
int SymbCrvMultiResKVBuild(const CagdCrvStruct *Crv,
			   int Discont,
			   CagdRType ***KVList,
			   int **KVListSizes,
			   int *KVListSize)
{
    CagdBType
	Periodic = CAGD_IS_PERIODIC_CRV(Crv);
    int i, j,
	PrevLen = CAGD_CRV_PT_LST_LEN(Crv),
	Order = Crv -> Order,
	KVMaxSize = CAGD_CRV_PT_LST_LEN(Crv) + Order;
    CagdRType *KVPtr,
	*KVPrev = Crv -> KnotVector;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	SYMB_FATAL_ERROR(SYMB_ERR_BSP_CRV_EXPECT);
	return FALSE;
    }

    /* Compute the hierarchy size and allocate its KnotVectors. */
    for (*KVListSize = 0;
	 (1 << *KVListSize) < PrevLen - Order;
	 (*KVListSize)++);
    *KVList = (CagdRType **) IritMalloc(sizeof(CagdRType *) * ++(*KVListSize));
    *KVListSizes = (int *) IritMalloc(sizeof(int) * *KVListSize);

    (*KVList)[0] = (CagdRType *) IritMalloc(sizeof(CagdRType) * KVMaxSize);
    (*KVListSizes)[0] = KVMaxSize;
    SYMB_GEN_COPY((*KVList)[0], KVPrev, sizeof(CagdRType) * KVMaxSize);
    
    for (i = 1; i < *KVListSize; i++) {
	KVPtr = (*KVList)[i] = (CagdRType *)
				IritMalloc(sizeof(CagdRType) * KVMaxSize);

	/* Copy first Order knots verbatim. */
	(*KVListSizes)[i] = 2 * Order;/* End conditions are copied verbatim. */
	for (j = 0; j < Order; j++)
	    *KVPtr++ = *KVPrev++;

	/* Skip every second interior knot. */
	for (; j < PrevLen; j++, KVPrev++) {
	    if (Discont) {
		if ((j & 0x01) == 0 ||
		    IRIT_APX_EQ(KVPrev[-1], KVPrev[0]) ||
		    IRIT_APX_EQ(KVPrev[0], KVPrev[1])) {
		    *KVPtr++ = *KVPrev;
		    (*KVListSizes)[i]++;
		}
	    }
	    else {
		if ((j & 0x01) == 0) {
		    *KVPtr++ = *KVPrev;
		    (*KVListSizes)[i]++;
		}
	    }
	}

	/* Copy last Order knots verbatim. */
	for (j = 0; j < Order; j++)
	    *KVPtr++ = *KVPrev++;

	KVPrev = (*KVList)[i];
	PrevLen = (*KVListSizes)[i] - Order;

	/* Make sure we did not exhaust all the interior knots already, or  */
	/* alternatively all interior knots maintains discontonuities.      */
	if (Periodic ? PrevLen <= Order + Order - 1 : PrevLen <= Order) {
	    *KVListSize -= (*KVListSize - i) - 1;
	    if (Periodic ? PrevLen < Order + Order - 1 : PrevLen < Order) {
		IritFree(KVList[i]);
		(*KVListSize)--;
	    }
	    break;
	}
	else if ((*KVListSizes)[i] == (*KVListSizes)[i-1]) {
	    *KVListSize -= *KVListSize - i;
	    IritFree((*KVList)[i]);
	    break;
	}
    }

    if (Periodic) {
	/* Make sure spaces are consistent at the ends. */
	for (i = 0; i < *KVListSize; i++) {
	    int Len = (*KVListSizes)[i] - Order;
	    CagdRType
		*KV = (*KVList)[i];

	    for (j = 0; j < Order - 1; j++)
		KV[j] = KV[Order - 1] + KV[Len + j - (Order - 1)]
				      - KV[Len];
	    for (j = Len + 1; j < Len + Order; j++)
		KV[j] = KV[Len] + KV[j - Len + Order - 1] - KV[Order - 1];
	}
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintKvs, FALSE) {
	    for (i = 0; i < *KVListSize; i++) {
	        IRIT_INFO_MSG_PRINTF(
			"KV - LEVEL %d (len = %d) ***********************\n",
			i, (*KVListSizes)[i]);

		for (j = 0; j < (*KVListSizes)[i]; j++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ", (*KVList)[i][j]);
		IRIT_INFO_MSG("\n\n");
	    }
	}
    }
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline curve, computes a hierarch of Bspline curves, each being   M
* represented using a subspace of the previous, upto a curve with no         M
* interior knots (i.e. a polynomial Bezier).				     M
*   However, if Discont == TRUE, then C1 discontinuities are preserved	     M
* through out the hierarchy decomposition.				     M
*   Each level in hierarchy has approximately half the number of control     M
* points of the previous one.						     M
*   Least square curve fitting is used to build the hierarchy.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute a least square multi resolution decomposition for. M
*   Discont:   Do we want to preserve discontinuities?                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbMultiResCrvStruct *:   A multi resolution curve structure hold the   M
*                              multi resolution decomposition of Crv.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvMultiResDecomp2			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResDecomp, multi resolution, least square decomposition      M
*****************************************************************************/
SymbMultiResCrvStruct *SymbCrvMultiResDecomp(const CagdCrvStruct *Crv,
					     int Discont)
{
    CagdBType
	Periodic = CAGD_IS_PERIODIC_CRV(Crv);
    int KVListSize, *KVListSizes, i, j,
	Order = Crv -> Order;
    CagdRType **KVList, *Params;
    SymbMultiResCrvStruct *MRCrv;
    CagdCrvStruct *OCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	SYMB_FATAL_ERROR(SYMB_ERR_BSP_CRV_EXPECT);
	return FALSE;
    }

    if (!BspCrvHasOpenEC(Crv)) {/* We need an open end version of the curve. */
	if (Periodic) {
	    CagdCrvStruct
		*TCrv = CagdCnvrtPeriodic2FloatCrv(Crv);

	    OCrv = CagdCnvrtFloat2OpenCrv(TCrv);
	    CagdCrvFree(TCrv);
	}
	else {
	    OCrv = CagdCnvrtFloat2OpenCrv(Crv);
	}
    }
    else
	OCrv = CagdCrvCopy(Crv);

    /* Compute the hierarchy size and allocate its KnotVectors. */
    if (!SymbCrvMultiResKVBuild(Crv, Discont,
				&KVList, &KVListSizes, &KVListSize))
	return NULL;

    /* Now compute the curves in reverse, from the smallest subspace to the */
    /* largest and accumulate the representation upto the original space.   */
    Params = CagdCrvNodes(Crv);
    MRCrv = SymbCrvMultiResNew(KVListSize, Periodic);

    for (i = KVListSize - 1; i >= 0; i--) {
	CagdCrvStruct *InterpCrv, *DiffCrv;
	CagdCtlPtStruct
	    *PtList = NULL;

	/* Sample the current curve at its node values to guarantee a       */
	/* unique interpolatory result - identical to the original curve.   */
	for (j = CAGD_CRV_PT_LST_LEN(OCrv) - 1; j >= 0; j--) {
	    CagdCtlPtStruct
		*Pt = CagdCtlPtNew(OCrv -> PType);
	    CagdRType
		*R = BspCrvEvalAtParam(OCrv, Params[j]);

	    SYMB_GEN_COPY(Pt -> Coords, R,
			  sizeof(CagdRType) * CAGD_MAX_PT_SIZE);

	    Pt -> Pnext = PtList;
	    PtList = Pt;
	}

	InterpCrv = BspCrvInterpolate(PtList, Params, KVList[i],
				      KVListSizes[i] - Order -
				          (Periodic ? Order - 1 : 0),
				      Order, Periodic);
	CagdCtlPtFreeList(PtList);

	if (!BspCrvHasOpenEC(InterpCrv)) {
	    CagdCrvStruct *OInterpCrv;

	    if (Periodic) {
		CagdCrvStruct
		    *TCrv = CagdCnvrtPeriodic2FloatCrv(InterpCrv);

		OInterpCrv = CagdCnvrtFloat2OpenCrv(TCrv);
		CagdCrvFree(TCrv);
	    }
	    else {
		OInterpCrv = CagdCnvrtFloat2OpenCrv(InterpCrv);
	    }

	    DiffCrv = SymbCrvSub(OCrv, OInterpCrv);
	    MRCrv -> HieCrv[KVListSize - 1 - i] = OInterpCrv;
	    CagdCrvFree(InterpCrv);
	}
	else {
	    DiffCrv = SymbCrvSub(OCrv, InterpCrv);
	    MRCrv -> HieCrv[KVListSize - 1 - i] = InterpCrv;
	}

	CagdCrvFree(OCrv);
	OCrv = DiffCrv;
    }

    for (i = 0; i < KVListSize; i++)
	IritFree(KVList[i]);
    IritFree(KVList);
    IritFree(KVListSizes);

    CagdCrvFree(OCrv);
    IritFree(Params);

    return MRCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline curve, computes a hierarch of Bspline curves, each being   M
* represented using a subspace of the previous, upto a curve with no         M
* interior knots (i.e. a polynomial Bezier).				     M
*   However, if Discont == TRUE, then C1 discontinuities are preserved	     M
* through out the hierarchy decomposition.				     M
*   Each level in hierarchy has approximately half the number of control     M
* points of the previous one.						     M
*   B-Wavelet decomposition is used to build the hierarchy.		     M
*   See R. Kazinnik and G. Elber, "Orthogonal Decomposition of Non Uniform   M
* Bspline Spaces using Wavelets", Eurographics 1997.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute a B-Wavelet decomposition for.		     M
*   Discont:   Do we want to preserve discontinuities?                       M
*   SameSpace: If this curve is in the same space as last curve, exploit     M
*	       this in optimizing the computation cost.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbMultiResCrvStruct *:   A B-Wavelet decomposition structure hold the  M
*                              multi resolution decomposition of Crv.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvMultiResDecomp			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResDecomp2, multi resolution, B-Wavelet decomposition        M
*****************************************************************************/
SymbMultiResCrvStruct *SymbCrvMultiResDecomp2(const CagdCrvStruct *Crv,
					      int Discont,
					      int SameSpace)
{
    static int
	KVListSize = 0,
	*KVListSizes = NULL;
    static IrtRType
	**ProdMatAList = NULL,
	**ProdMatbList = NULL,
	**KVList = NULL;
    CagdPointType
	PType = Crv -> PType;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType),
	Periodic = CAGD_IS_PERIODIC_CRV(Crv);
    int k,
	Length = Crv -> Length,
	NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	Order = Crv -> Order;
    SymbMultiResCrvStruct *MRCrv;
    CagdCrvStruct *OCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv)) {
	SYMB_FATAL_ERROR(SYMB_ERR_BSP_CRV_EXPECT);
	return FALSE;
    }

    if (!BspCrvHasOpenEC(Crv)) {      /* We need open end version of curve. */
	if (Periodic) {
	    CagdCrvStruct
	        *TCrv = CagdCnvrtPeriodic2FloatCrv(Crv);

	    OCrv = CagdCnvrtFloat2OpenCrv(TCrv);
	    CagdCrvFree(TCrv);
	}
	else {
	    OCrv = CagdCnvrtFloat2OpenCrv(Crv);
	}
    }
    else
	OCrv = CagdCrvCopy(Crv);

    if (!SameSpace) {
	/* Free old data structures not needed any more, if any. */
	if (KVListSize > 0) {
	    for (k = 0; k < KVListSize; k++)
	        IritFree(KVList[k]);
	    IritFree(KVList);
	    for (k = 1; k < KVListSize; k++) {
		IritFree(ProdMatAList[k]);
		IritFree(ProdMatbList[k]);
	    }
	    IritFree(ProdMatAList);
	    IritFree(ProdMatbList);
	    IritFree(KVListSizes);
	}

        /* And compute the hierarchy size and allocate new data structures. */
	if (!SymbCrvMultiResKVBuild(Crv, Discont,
				    &KVList, &KVListSizes, &KVListSize))
	    return NULL;
	ProdMatAList = (CagdRType **) IritMalloc(KVListSize
							* sizeof(CagdRType *));
	ProdMatbList = (CagdRType **) IritMalloc(KVListSize
							* sizeof(CagdRType *));
	IRIT_ZAP_MEM(ProdMatAList, KVListSize * sizeof(CagdRType *));
	IRIT_ZAP_MEM(ProdMatbList, KVListSize * sizeof(CagdRType *));
    }

    /* Now compute the decomposition. */
    MRCrv = SymbCrvMultiResNew(KVListSize, Periodic);
    MRCrv -> HieCrv[0] = CagdCrvCopy(OCrv);

    for (k = 1; k < KVListSize; k++) {
        int i, j, l,
	    Len = KVListSizes[k] - Order - (Periodic ? Order - 1 : 0);
        CagdRType *Matb, *b, *m, *r;
	CagdCrvStruct *ProjCrv;

	if (!SameSpace) {
	    /*   Construct the linear system Ax = b to solve for the         */
	    /* projection where x are the unknown coefficients.		     */
	    /*   A is a square matrix of size (Len x Len). 	             */
	    ProdMatAList[k] = 
		m = (CagdRType *) IritMalloc(Len * Len * sizeof(CagdRType));

	    SymbBspBasisInnerProdPrep2(KVList[k], KVList[k],
				       KVListSizes[k], KVListSizes[k],
				       Order, Order);

	    /* Use the symmetry of this matrix to half the cost. */
	    for (i = 0; i < Len; i++) {
	        for (j = 0; j <= i; j++) {
		    m[j * Len + i] = SymbBspBasisInnerProd(i, j);
		    if (j != i)
		        m[i * Len + j] = m[j * Len + i];
		}
	    }

#	    ifdef DEBUG
	    {
	        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBWaveletMat, FALSE) {
		    IRIT_INFO_MSG_PRINTF(
			    "B_WAVELET_MAT (len = %d) *********************\n",
			    Len);
		    for (i = 0; i < Len; i++) {
		        for (j = 0; j < Len; j++)
			    IRIT_INFO_MSG_PRINTF("%0.7lf ", m[i * Len + j]);
			IRIT_INFO_MSG("\n");
		    }
		}
	    }
#	    endif /* DEBUG */

	    /* Compute the product matrix for the b vector in Ax = b. */
	    ProdMatbList[k] = (CagdRType *) IritMalloc(Length * Len
							* sizeof(CagdRType));

	    SymbBspBasisInnerProdPrep2(KVList[k - 1], KVList[k],
				       KVListSizes[k - 1], KVListSizes[k],
				       Order, Order);
	    for (i = 0, m = ProdMatbList[k]; i < Len; i++) {
	        for (j = 0; j < Length; j++)
		    *m++ = SymbBspBasisInnerProd(j, i);
	    }
	}

	/* Compute the SVD of A in Ax = b. */
	if (IRIT_FABS(SvdLeastSqr(ProdMatAList[k],
			     NULL, NULL, Len, Len)) < IRIT_UEPS) {
	    SymbCrvMultiResFree(MRCrv);
	    return NULL;
	}

	Matb = ProdMatbList[k];

	b = (CagdRType *) IritMalloc(Len * sizeof(CagdRType));

	ProjCrv = BspPeriodicCrvNew(Len, Order, Periodic, PType);
	SYMB_GEN_COPY(ProjCrv -> KnotVector, KVList[k],
		      KVListSizes[k] * sizeof(CagdRType));

	/* Solve for the different axes (different x's in Ax = b). */
	for (l = !IsRational; l <= NumCoords; l++) {
	    for (i = 0, r = b, m = Matb; i < Len; i++, r++) {
		CagdRType
		    *Pts = OCrv -> Points[l];

		*r = 0;
		for (j = 0; j < Length; j++)
		    *r += *m++ * *Pts++;
	    }
#	    ifdef DEBUG
	    {
	        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBWaveletMat, FALSE) {
		    IRIT_INFO_MSG_PRINTF(
			    "B_WAVELET_B (len = %d) ***********************\n",
			    Len);
		    for (i = 0; i < Len; i++)
		        IRIT_INFO_MSG_PRINTF("%0.7lf\n", b[i]);
		}
	    }
#	    endif /* DEBUG */

	    SvdLeastSqr(NULL, ProjCrv -> Points[l], b, Len, Len);
	}

	IritFree(b);

	MRCrv -> HieCrv[k] = ProjCrv;

	CagdCrvFree(OCrv);
	OCrv = CagdCrvCopy(ProjCrv);
	Length = Len;
    }

    CagdCrvFree(OCrv);

    return MRCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi resolution decomposition of a Bspline curve, computes the    M
* regular Bspline curve out of it.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrv:    A multi resolution decomposition of a curve.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve that adds up all components of the multi       M
*                     resolution decomposition MRCrv.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResCompos, multi resolution, least square decomposition      M
*****************************************************************************/
CagdCrvStruct *SymbCrvMultiResCompos(const SymbMultiResCrvStruct *MRCrv)
{
    return SymbCrvMultiResComposAtT(MRCrv,
				    MRCrv -> Levels - 1 +
					(MRCrv -> RefineLevel != FALSE));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multiresolution decomposition of a Bspline curve, computes a       M
* regular Bspline curve out of it representing the decomposed curve at       M
* the multi resolution hierarchy level of T.				     M
*   Although decomposition is discrete, T can be any real number between     M
* these discrete levels and a linear interpolation of adjacent levels is     M
* exploited.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrv:     A multi resolution decomposition of a curve.                  M
*   T:         A mult resolution hierarcy level to compute curve for.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve that adds up all components of the multi       M
*                     resolution decomposition MRCrv up to and including     M
*                     level T.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResComposAtT, multi resolution, least square decomposition   M
*****************************************************************************/
CagdCrvStruct *SymbCrvMultiResComposAtT(const SymbMultiResCrvStruct *MRCrv,
					CagdRType T)
{
    int i,
	It = (int) T;
    CagdCrvStruct
	*SumCrv = CagdCrvCopy(MRCrv -> HieCrv[0]);

    for (i = 1;
	 i <= It && i < MRCrv -> Levels + (MRCrv -> RefineLevel != FALSE);
	 i++) {
	CagdCrvStruct
	    *TCrv = SymbCrvAdd(SumCrv, MRCrv -> HieCrv[i]);

	CagdCrvFree(SumCrv);
	SumCrv = TCrv;
    }

    if (It != T) {
	CagdCrvStruct
	    *TCrv1 = SymbCrvScalarScale(MRCrv -> HieCrv[It + 1], T - It),
	    *TCrv2 = SymbCrvAdd(SumCrv, TCrv1);

	CagdCrvFree(TCrv1);
	CagdCrvFree(SumCrv);
	SumCrv = TCrv2;
    }

    return SumCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi resolution decomposition of a Bspline curve, edit it by      M
* modifying its Level'th Level according to the TransDir of Position at      M
* parametr t. 								     M
*   Level can be a fraction number between the discrete levels of the        M
* decomposition denoting a linear blend of two neighboring discrete levels.  M
*   Editing is performed in place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrv:       A multi resolution decomposition of a curve to edit it in   M
*                place.							     M
*   t:           Parameter value at which to modify MRCrv.                   M
*   TransDir:    Directional tranlation transformation to apply.             M
*   Level:       Of multi resolution hierarchy to edit.                      M
*   FracLevel:   The fraction level to edit - will blend two neighboring     M
*		 levels.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResEdit, multi resolution, least square decomposition        M
*****************************************************************************/
void SymbCrvMultiResEdit(const SymbMultiResCrvStruct *MRCrv,
			 CagdRType t,
			 const CagdVType TransDir,
			 CagdRType Level,
			 CagdRType FracLevel)
{
    int ILevel = (int) Level;

    if (Level == ILevel) {
	CagdCrvStruct *Crv, *OrigCrv, *DiffCrv;
	CagdRType *BasisFuncs, **Points;
	int i, Length, Order, IndexFirst;

	if (ILevel < 0 ||
	    ILevel >= MRCrv -> Levels + (MRCrv -> RefineLevel != FALSE)) {
	    SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    return;
	}

	/* Prepare the Level'th curve in the hierarchy. */
	OrigCrv = CagdCrvCopy(MRCrv -> HieCrv[0]);
	for (i = 1; i <= ILevel; i++) {
	    CagdCrvStruct
		*TCrv = SymbCrvAdd(OrigCrv, MRCrv -> HieCrv[i]);

	    CagdCrvFree(OrigCrv);
	    OrigCrv = TCrv;
	}
	Crv = CagdCrvCopy(OrigCrv);

	Points = Crv -> Points;
	Length = Crv -> Length;
	Order = Crv -> Order;

	BasisFuncs = BspCrvCoxDeBoorBasis(Crv -> KnotVector, Order,
					  Length, Crv -> Periodic,
					  t, &IndexFirst);

	for (i = IndexFirst; i < IndexFirst + Order; i++) {
	    CagdRType
		MultFactor = BasisFuncs[i - IndexFirst];

	    switch (Crv -> PType) {
	        case CAGD_PT_E3_TYPE:
	            Points[3][i] += TransDir[2] * MultFactor;
		case CAGD_PT_E2_TYPE:
	            Points[2][i] += TransDir[1] * MultFactor;
	            Points[1][i] += TransDir[0] * MultFactor;
		    break;
		case CAGD_PT_P3_TYPE:
		case CAGD_PT_P2_TYPE:
		    IRIT_WARNING_MSG_PRINTF(IRIT_EXP_STR("RATIONALS NOT SUPPORTED\n"));
		    SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
		    break;
		default:
		    SYMB_FATAL_ERROR(SYMB_ERR_UNSUPPORT_PT);
		    break;
	    }
	}

	/* Construct the new hierarchy. */
	DiffCrv = SymbCrvSub(Crv, OrigCrv);
	CagdCrvFree(OrigCrv);
	CagdCrvFree(Crv);

	/* Apply the fraction ratio, if necessary. */
	if (!IRIT_APX_EQ(FracLevel, 1.0)) {
	    Crv = SymbCrvScalarScale(DiffCrv, FracLevel);
	    CagdCrvFree(DiffCrv);
	    DiffCrv = Crv;
	}

	Crv = SymbCrvAdd(MRCrv -> HieCrv[ILevel], DiffCrv);
	CagdCrvFree(MRCrv -> HieCrv[ILevel]);
	MRCrv -> HieCrv[ILevel] = Crv;

	CagdCrvFree(DiffCrv);
    }
    else {
	int Level1 = ILevel,
	    Level2 = Level1 + 1;

	FracLevel = Level - Level1;

	SymbCrvMultiResEdit(MRCrv, t, TransDir, Level1,
			    1.0 - FracLevel),
	SymbCrvMultiResEdit(MRCrv, t, TransDir, Level2, FracLevel);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi resolution decomposition of a Bspline curve, refine it at    M
* neighborhood of parameter value t, in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrv:       A multi resolution decomposition of a curve, to refine in   M
*                place.							     M
*   T:           Parameter value at which to refine MRCrv.                   M
*   SpanDiscont: Do we want to refine beyond discontinuities?                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A pointer to an array of two real numbers holding the      M
*                 domain in MRCrv that was refined.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResRefineLevel, multi resolution, least square decomposition M
*****************************************************************************/
CagdRType *SymbCrvMultiResRefineLevel(SymbMultiResCrvStruct *MRCrv,
				      CagdRType T,
				      int SpanDiscont)
{
    IRIT_STATIC_DATA CagdRType Domain[2];
    CagdCrvStruct *Crv, *RefCrv;
    CagdRType TMin, TMax, **Points, *KV, *NewKV;
    int i, j, Order, Length, LastLIndex, FirstGIndex, StartIndex,
	NewKVIndex = 0;

    if (!MRCrv -> RefineLevel) {
	/* Do not have a refining level yet - add a zero valued curve now. */
	Crv = MRCrv -> HieCrv[MRCrv -> Levels] =
	    CagdCrvCopy(MRCrv -> HieCrv[MRCrv -> Levels - 1]);
	Points = Crv -> Points;
	for (j = 0; j < Crv -> Length; j++)
	    for (i = 1; i <= CAGD_NUM_OF_PT_COORD(Crv -> PType); i++)
		Points[i][j] = 0.0;
	MRCrv -> RefineLevel = TRUE;
    }
    else {
	Crv = MRCrv -> HieCrv[MRCrv -> Levels];
    }
    Length = Crv -> Length;
    Order = Crv -> Order;

    /* Figure out the knots to insert to the refining curve. */
    KV = Crv -> KnotVector;
    NewKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * (Order + 1) * 2);
    CagdCrvDomain(Crv, &TMin, &TMax);
    LastLIndex = BspKnotLastIndexL(KV, Length + Order, T);
    FirstGIndex = BspKnotFirstIndexG(KV, Length + Order, T);

    /* Compute the new knots to the left. */
    for (StartIndex = 0, i = IRIT_MAX(0, LastLIndex - Order);
	 i <= LastLIndex;
	 i++) {
	if (!IRIT_APX_EQ(KV[i], KV[i + 1]))
	    NewKV[NewKVIndex++] = (KV[i] + KV[i + 1]) * 0.5;
	else if (SpanDiscont)
	    StartIndex = NewKVIndex;
    }
    /* Compute the new knots to the right. */
    for (i = FirstGIndex;
	 i < IRIT_MIN(FirstGIndex + Order, Length + Order);
	 i++) {
	if (!IRIT_APX_EQ(KV[i], KV[i + 1]))
	    NewKV[NewKVIndex++] = (KV[i] + KV[i + 1]) * 0.5;
	else if (SpanDiscont)
	    break;
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugRefineKnots, FALSE) {
	    IRIT_INFO_MSG("\nOld knot vector ********************\n");
	    for (i = 0; i < Length + Order; i++)
	        IRIT_INFO_MSG_PRINTF("%8lg  ", KV[i]);
	    IRIT_INFO_MSG("\nNew knots **************************\n");
	    for (i = StartIndex; i < NewKVIndex; i++)
	        IRIT_INFO_MSG_PRINTF("%8lg  ", NewKV[i]);
	}
    }
#endif /* DEBUG */

    Domain[0] = NewKV[StartIndex];
    Domain[1] = NewKV[NewKVIndex - 1];
    RefCrv = CagdCrvRefineAtParams(Crv, FALSE, &NewKV[StartIndex],
				   NewKVIndex - StartIndex);
    IritFree(NewKV);

    CagdCrvFree(MRCrv -> HieCrv[MRCrv -> Levels]);
    MRCrv -> HieCrv[MRCrv -> Levels] = RefCrv;

    return Domain;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs the B-Wavelet of knot KV[KnotIndex] for the given knot        M
* sequence KV and order Order.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:        Knot sequence of the space.                                   M
*   Order:     Order of space.                                               M
*   Len:       Length of knot sequence.                                      M
*   KnotIndex: Index of knot to compute the wavelet for.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A scalar curve representing the wavelet.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProd                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResBWavelet                                                  M
*****************************************************************************/
CagdCrvStruct *SymbCrvMultiResBWavelet(CagdRType *KV,
				       int Order,
				       int Len,
				       int KnotIndex)
{
    CagdBType
	HasNewKV = FALSE;
    int i, j, Step, NumConst,
	KnotExtent = 0,
	IndexMin = KnotIndex - Order * 2 + 1,
	IndexMinBound = IRIT_MAX(IndexMin, Order),
	IndexMax = KnotIndex + Order * 2 - 1,
	IndexMaxBound = IRIT_MIN(IndexMax, Len - Order),
	WLen = IndexMax - IndexMin + 1,
	WLen2 = (WLen >> 1) + 1;
    CagdRType *KV1, *KV2, *Mat, *b, *m, t1, t2,
	TMin = KV[Order - 1],
	TMax = KV[Len - Order];
    CagdCrvStruct *BWCrv;

    if (KnotIndex < Order || KnotIndex >= Len - Order) {
	SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
        return NULL;
    }

    /* Extend the KV knot sequence if close to one of the two the ends. */
    if (IndexMin != IndexMinBound || IndexMax != IndexMaxBound) {
	KV1 = (CagdRType *) IritMalloc((Len + Order * 4) * sizeof(CagdRType));
	CAGD_GEN_COPY(&KV1[2 * Order], KV, Len * sizeof(CagdRType));
	for (i = 2 * Order; i >= 0; i--) 	      /* Add end conditions. */
	    KV1[i] = KV1[i + 1] - 1.0;
	for (i = Len + 2 * Order - 1; i < Len + 4 * Order; i++)
	    KV1[i] = KV1[i - 1] + 1.0;
	KV = KV1;
	HasNewKV = TRUE;

	KnotExtent = Order * 2;
	KnotIndex += KnotExtent;  /* Shift prescribed index to new location. */
	IndexMin += KnotExtent;
	IndexMax += KnotExtent;
	
#       ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBInputWaveletKvs, FALSE) {
	        IRIT_INFO_MSG_PRINTF(
			"INPUT B_WAVELET_KVS (len = %d) ****************\nKV:",
			Len + Order * 4);
		for (i = 0; i < Len + Order * 4; i++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ", KV[i]);
		IRIT_INFO_MSG("\n");
	    }
	}
#       endif /* DEBUG */
    }

    /* Prepare knot sequence that will represent/support the B-Wavelet knot. */
    KV1 = (CagdRType *) IritMalloc((WLen + Order * 2) * sizeof(CagdRType));
    CAGD_GEN_COPY(&KV1[Order - 1], &KV[IndexMin], WLen * sizeof(CagdRType));
    for (i = 0; i < Order - 1; i++) {       /* Duplicate the end conditions. */
        KV1[i] = KV1[Order - 1];
	KV1[WLen + Order + i - 1] = KV1[WLen + Order - 2];
    }    /* Make sure no more than order knots at the end. */
    for (i = WLen + Order - 2; KV1[i] <= KV1[i - 1] + END_COND_EPS; i--)
        KV1[i - 1] -= END_COND_EPS;
    for (i = Order; KV1[i] <= KV1[i - 1] + END_COND_EPS; i++)
        KV1[i] += END_COND_EPS;

    /* Prepare a knot sequence, doubly spaced,  without the B-Wavelet knot. */
    KV2 = (CagdRType *) IritMalloc((WLen2 + Order * 2 - 2) * sizeof(CagdRType));
    for (Step = 2, i = KnotIndex - 1, j = Order - 1;
	 i >= 0 && j >= 0;
	 i -= Step, j--) {
	KV2[j + Order - 1] = KV[i];
	if (i < Order)
	    Step = 1;	      /* No skipping of knots at the end conditions. */
    }
    for (Step = 2, i = KnotIndex + 1, j = Order;
	 i < Len + 2 * KnotExtent && j < WLen2;
	 i += Step, j++) {
	KV2[j + Order - 1] = KV[i];
	if (i > Len + 2 * KnotExtent - Order)
	    Step = 1;	      /* No skipping of knots at the end conditions. */
    }
    for (i = 0; i < Order - 1; i++) {       /* Duplicate the end conditions. */
        KV2[i] = KV2[Order - 1];
	KV2[WLen2 + Order + i - 1] = KV2[WLen2 + Order - 2];
    }
    /* Make sure no more than order knots at the end. */
    for (i = WLen2 + Order - 2; KV2[i] <= KV2[i - 1] + END_COND_EPS; i--)
        KV2[i - 1] -= END_COND_EPS;
    for (i = Order; KV2[i] <= KV2[i - 1] + END_COND_EPS; i++)
        KV2[i] += END_COND_EPS;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBInputWaveletKvs, FALSE) {
	    IRIT_INFO_MSG_PRINTF(
		    "B_WAVELET_KVS - KV1: (len = %d) **********************\n",
		    WLen + Order * 2 - 2);
	    for (i = 0; i < WLen + Order * 2 - 2; i++)
	        IRIT_INFO_MSG_PRINTF("%0.7lf ", KV1[i]);
	    IRIT_INFO_MSG("\n");

	    IRIT_INFO_MSG_PRINTF(
		    "B_WAVELET_KVS - KV2: (len = %d) **********************\n",
		    WLen2 + Order * 2 - 2);
	    for (i = 0; i < WLen2 + Order * 2 - 2; i++)
	        IRIT_INFO_MSG_PRINTF("%0.7lf ", KV2[i]);
	    IRIT_INFO_MSG("\n");

	    IRIT_INFO_MSG_PRINTF("WAVELET KNOT = %f\n", KV[KnotIndex]);
	}
    }
#   endif /* DEBUG */

    /* Evaluate the inner products into the matrix A in Ax = b and add      */
    /* all the homogeneous orthogonality constraints.			    */
    SymbBspBasisInnerProdPrep2(KV1, KV2, WLen + Order * 2 - 2,
			       WLen2 + Order * 2 - 2, Order, Order);
    IritFree(KV2);

    /* Number of degrees of freedom is equal the number of constraints and  */
    /* also equal number of non zero coefficients in the created wavelet.   */
    /* (The wavelet function is constructed with (Order - 1) addition       */
    /* zero coefficients in each side).				            */
    NumConst = WLen2 + Order - 1;

    Mat = (CagdRType *) IritMalloc(NumConst * NumConst * sizeof(CagdRType));
    for (j = 0; j < NumConst - 1; j++) {
	m = &Mat[j * NumConst];
        for (i = 0; i < NumConst; i++)
	    m[i] = SymbBspBasisInnerProd(i + Order - 1, j);
    }

    /* Add the magnitude constaint, the only non-homogeneous constraint. */
    m = &Mat[(NumConst - 1) * NumConst];
    IRIT_ZAP_MEM(m, NumConst * sizeof(CagdRType));
    m[NumConst >> 1] = 1.0;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintBWaveletMat, FALSE) {
	    IRIT_INFO_MSG_PRINTF(
		    "B_WAVELET_MAT (%d x %d) ***********************\n",
		    NumConst, NumConst);
	    for (j = 0; j < NumConst; j++) {
	        for (i = 0; i < NumConst; i++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ", Mat[j * NumConst + i]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumConst, NumConst)) <
							IRIT_SQR(IRIT_UEPS)) {
        IritFree(Mat);
	IritFree(KV1);
	if (HasNewKV)
	    IritFree(KV);
        return NULL;
    }
    IritFree(Mat);

    /* Prepare the orthogonality and normalization vector b in Ax = b. */
    b = (CagdRType *) IritMalloc(NumConst * sizeof(CagdRType));
    IRIT_ZAP_MEM(b, (NumConst - 1) * sizeof(CagdRType));
    b[NumConst - 1] = 1.0;

    BWCrv = BspCrvNew(NumConst + (Order - 1) * 2, Order, CAGD_PT_E1_TYPE);
    CAGD_GEN_COPY(BWCrv -> KnotVector, KV1,
		  (BWCrv -> Length + Order) * sizeof(CagdRType));

    IRIT_ZAP_MEM(&BWCrv -> Points[1][0], BWCrv -> Length * sizeof(CagdRType));
    SvdLeastSqr(NULL, &BWCrv -> Points[1][Order - 1], b, NumConst, NumConst);

    /* Clip the new Wavelet to the original domain. */
    CagdCrvDomain(BWCrv, &t1, &t2);
    if (t1 < TMin || t2 > TMax) {
	CagdCrvStruct
	    *TCrv = CagdCrvRegionFromCrv(BWCrv, IRIT_MAX(t1, TMin),
						IRIT_MIN(t2,TMax));

	CagdCrvFree(BWCrv);
	BWCrv = TCrv;
    }

    IritFree(b);
    IritFree(KV1);
    if (HasNewKV)
	IritFree(KV);
        
    return CagdCrvUnitMaxCoef(BWCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi resolution decomposition of a Bspline curve, free it.        M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrv:       A multi resolution decomposition of a curve to free.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResFree, multi resolution, least square decomposition	     M
*****************************************************************************/
void SymbCrvMultiResFree(SymbMultiResCrvStruct *MRCrv)
{
    int i;

    if (MRCrv == NULL)
	return;

    for (i = 0; i < MRCrv -> Levels + (MRCrv -> RefineLevel != FALSE); i++)
	CagdCrvFree(MRCrv -> HieCrv[i]);

    IritFree((MRCrv -> HieCrv));
    IritFree(MRCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates a data structure for multi resolution decomposition of a Bspline M
* curve of Levels levels and possiblt periodic.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Levels:      Number of levels to expect in the decomposition.            M
*   Periodic:    Is the curve periodic?                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbMultiResCrvStruct *:   A structure to hold a multi resolution        M
*                              decomposition of a curve of Levels levels.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResNew, multi resolution, least square decomposition	     M
*****************************************************************************/
SymbMultiResCrvStruct *SymbCrvMultiResNew(int Levels, CagdBType Periodic)
{
    SymbMultiResCrvStruct
	*MRCrv = (SymbMultiResCrvStruct *)
				     IritMalloc(sizeof(SymbMultiResCrvStruct));

    MRCrv -> Levels = Levels;

    /* Keep one more level for the refining level to come. */
    MRCrv -> HieCrv = (CagdCrvStruct **)
	IritMalloc(sizeof(CagdCrvStruct *) * (Levels + 1));

    MRCrv -> RefineLevel = FALSE;
    MRCrv -> Periodic = Periodic;
    MRCrv -> Pnext = NULL;

    return MRCrv;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi resolution decomposition of a Bspline curve, copy it.        M
*                                                                            *
* PARAMETERS:                                                                M
*   MRCrvOrig:    A multi resolution decomposition of a curve to copy.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbMultiResCrvStruct *:   A duplicated structure of MRCrv.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultiResCopy, multi resolution, least square decomposition	     M
*****************************************************************************/
SymbMultiResCrvStruct *SymbCrvMultiResCopy(const SymbMultiResCrvStruct 
					                           *MRCrvOrig)
{
    int i;
    SymbMultiResCrvStruct
	*MRCrv = (SymbMultiResCrvStruct *)
				     IritMalloc(sizeof(SymbMultiResCrvStruct));

    MRCrv -> Levels = MRCrvOrig -> Levels;
    MRCrv -> RefineLevel = MRCrvOrig -> RefineLevel;
    MRCrv -> Pnext = NULL;
    MRCrv -> HieCrv = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *) *
						    (MRCrvOrig -> Levels + 1));

    for (i = 0; i < MRCrv -> Levels + (MRCrv -> RefineLevel != FALSE); i++)
	MRCrv -> HieCrv[i] = CagdCrvCopy(MRCrvOrig -> HieCrv[i]);


    return MRCrv;
}
