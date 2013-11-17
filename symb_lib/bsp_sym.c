/******************************************************************************
* SBsp_Sym.c - Bspline surface symbolic computation.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 92.					      *
******************************************************************************/

#include "symb_loc.h"

/* Use multisets in bsp mult blossom computations - far more efficient. */
#define BSP_MULT_BLOSSOM_MULTISET	1

#define NODE_EQUAL_SHIFT 0.8

typedef void (*SymbBlossomMultFuncType)(IrtRType, void *);

typedef struct SymbCrvBlossomMultInfoStruct {
    /* Two input curves' info. */
    const CagdCrvStruct *Crv1, *Crv2, *ProdCrv;
    CagdPointType PType;
    int Degree1, Degree2;
    /* Blossoming values to use, for regular or multiset cases. */
    CagdRType *VBlsmVals, *BlsmMultiSetValues;
    int BlsmMultiSetLen, *BlsmMultiSetMultplcty;
    int *PermVector;
    CagdRType *VBlsmVals1, *VBlsmVals2;
    CagdRType Result[CAGD_MAX_PT_COORD];
} SymbCrvBlossomMultInfoStruct;

typedef struct SymbSrfBlossomMultInfoStruct {
    /* Two input curves' info. */
    const CagdSrfStruct *Srf1, *Srf2, *ProdSrf;
    CagdCrvStruct *Crv1, *Crv2;
    CagdPointType PType;
    int UDegree1, VDegree1, UDegree2, VDegree2;
    /* Blossoming values to use, for regular or multiset cases. */
    CagdRType *UBlsmVals, *VBlsmVals;
    CagdRType *UBlsmMultiSetValues, *VBlsmMultiSetValues;
    int UBlsmMultiSetLen, VBlsmMultiSetLen;
    int *UBlsmMultiSetMultplcty, *VBlsmMultiSetMultplcty;
    int *UPermVector, *VPermVector;
    CagdRType *UBlsmVals1, *VBlsmVals1, *UBlsmVals2, *VBlsmVals2;
    CagdBType UBlsmValsUpdated;
    CagdRType Result[CAGD_MAX_PT_COORD];
} SymbSrfBlossomMultInfoStruct;

IRIT_STATIC_DATA int
    BspMultComputeMethod = BSP_MULT_BLOSSOMING;
IRIT_STATIC_DATA CagdCrvStruct
    *GlblInnerProdCrv1 = NULL,
    *GlblInnerProdCrv2 = NULL;
IRIT_STATIC_DATA SymbCrvBlossomMultInfoStruct
   GlblCrvBlossomMultInfo;
IRIT_STATIC_DATA SymbSrfBlossomMultInfoStruct
   GlblSrfBlossomMultInfo;

static void SymbCrvBlossomMultPermute(IrtRType Weight, void *Data);
static void SymbCrvBlossomMultiSetPerm(int *Vector,
				       int VecLength,
				       CagdRType *MultiSetValues,
				       int *MultiSetMultplcty,
				       int MultiSetLen,
				       int PermToSelect,
				       CagdRType Weight,
				       void *CallBackData,
				       SymbBlossomMultFuncType CallBackFunc);
#ifndef BSP_MULT_BLOSSOM_MULTISET
static void SymbBlossomPermute(int *Vector,
			       int VecLength,
			       int OnesToPlace,
			       SymbBlossomMultFuncType CallBackFunc);
#endif /* BSP_MULT_BLOSSOM_MULTISET */

static CagdCrvStruct *BspCrvMultAux(CagdCrvStruct *Crv1, CagdCrvStruct *Crv2);
static CagdSrfStruct *BspSrfMultAux(CagdSrfStruct *Srf1, CagdSrfStruct *Srf2);
static void SymbSrfBlossomMultiSetPerm(int *UVector,
				       int UVecLength,
				       CagdRType *UMultiSetValues,
				       int *UMultiSetMultplcty,
				       int UMultiSetLen,
				       int UPermToSelect,
				       SymbSrfBlossomMultInfoStruct
							*SrfBlossomMultInfo,
				       CagdRType Weight,
				       SymbBlossomMultFuncType CallBackFunc);
static void SymbSrfBlossomMultPermute(IrtRType Weight, void *Data);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets method of Bspline product computation.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   BspMultUsingInter:  If 1, Bspline product is computed by setting an      M
*                       interpolation problem. For 2, use Blossoming.        M
*			Otherwise, by decomposing the Bspline geometry to    M
*			Bezier geometry.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         Previous setting.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspMultComputationMethod, product	                                     M
*****************************************************************************/
int BspMultComputationMethod(int BspMultUsingInter)
{
    int i = BspMultComputeMethod;

    BspMultComputeMethod = BspMultUsingInter;
    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bspline curves - multiply them coordinatewise.		     M
*   The two curves are promoted to same point type before the multiplication M
* can take place.  The two curves are assumed to hold the same domain.       M
*   See also BspMultComputationMethod.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2:   The two curves to multiply.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The product Crv1 * Crv2 coordinatewise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvMult, product                                                      M
*****************************************************************************/
CagdCrvStruct *BspCrvMult(const CagdCrvStruct *CCrv1,
			  const CagdCrvStruct *CCrv2)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *ProdCrv, *TCrv, *Crv1, *Crv2;

    if (CAGD_IS_BEZIER_CRV(CCrv1))
	Crv1 = CagdCnvrtBzr2BspCrv(CCrv1);
    else if (!BspCrvHasOpenEC(CCrv1)) {
	CagdCrvDomain(CCrv1, &TMin, &TMax);
	Crv1 = CagdCrvRegionFromCrv(CCrv1, TMin, TMax);
    }
    else
        Crv1 = CagdCrvCopy(CCrv1);

    if (CAGD_IS_BEZIER_CRV(CCrv2))
	Crv2 = CagdCnvrtBzr2BspCrv(CCrv2);
    else if (!BspCrvHasOpenEC(CCrv2)) {
	CagdCrvDomain(CCrv2, &TMin, &TMax);
	Crv2 = CagdCrvRegionFromCrv(CCrv2, TMin, TMax);
    }
    else
        Crv2 = CagdCrvCopy(CCrv2);

    if (!CagdMakeCrvsCompatible(&Crv1, &Crv2, FALSE, FALSE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	return NULL;
    }

    /* Use interpolation to compute the product only if so requested and at  */
    /* least one of the two curves is non constant.			     */
    if (BspMultComputeMethod == BSP_MULT_INTERPOL &&
	(Crv1 -> Order > 1 || Crv2 -> Order > 1)) {
	CagdPointType
	    PType = Crv1 -> PType;
	CagdBType
	    IsRational = CAGD_IS_RATIONAL_PT(PType);
	int i, j, KVLen, ResLength,
	    NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	    ResOrder = Crv1 -> Order + Crv2 -> Order - 1;
	CagdRType *R,
	    *KV = BspKnotContinuityMergeTwo(Crv1 -> KnotVector,
					    Crv1 -> Length + Crv1 -> Order,
					    Crv1 -> Order,
					    Crv2 -> KnotVector,
					    Crv2 -> Length + Crv2 -> Order,
					    Crv2 -> Order,
					    ResOrder, &KVLen),
	    *KVNodes = BspKnotNodes(KV, KVLen, ResOrder);
	CagdCtlPtStruct
	    *CtlPt = NULL,
	    *CtlPtList = NULL;

	ResLength = KVLen - ResOrder;

	/* Verify that all nodes are distinct. */
	for (i = 0, R = KVNodes; i < ResLength - 1; i++, R++) {
	    if (IRIT_APX_EQ_EPS(R[0], R[1], IRIT_UEPS)) {
		if (i > 0)
		    R[0] = R[0] * NODE_EQUAL_SHIFT +
			   R[-1] * (1 - NODE_EQUAL_SHIFT);
	    }
	}

	/* Evaluate the multiplication at the node values. */
	for (i = 0, R = KVNodes; i < ResLength; i++, R++) {
	    CagdRType *Evl;

	    if (CtlPt == NULL)
		CtlPt = CtlPtList = CagdCtlPtNew(PType);
	    else {
		CtlPt -> Pnext = CagdCtlPtNew(PType);
		CtlPt = CtlPt -> Pnext;
	    }

	    Evl = CagdCrvEval(Crv1, *R);
	    SYMB_GEN_COPY(CtlPt -> Coords, Evl,
			  CAGD_MAX_PT_SIZE * sizeof(CagdRType));
	    Evl = CagdCrvEval(Crv2, *R);
	    for (j = !IsRational; j <= NumCoords; j++)
	        CtlPt -> Coords[j] *= Evl[j];

	}

	if ((ProdCrv = BspCrvInterpolate(CtlPtList, KVNodes, KV,
					ResLength, ResOrder, FALSE)) == NULL) {
	    SYMB_FATAL_ERROR(SYMB_ERR_SPL_PROD_FAILED);
	}

	IritFree(KVNodes);
	IritFree(KV);
	CagdCtlPtFreeList(CtlPtList);
    }
    else if (BspMultComputeMethod == BSP_MULT_BLOSSOMING &&
	     (Crv1 -> Order > 1 || Crv2 -> Order > 1)) {
        ProdCrv = BspCrvBlossomMult(Crv1, Crv2);
    }
    else {
	TCrv = BspCrvMultAux(Crv1, Crv2);

	if (CAGD_IS_BEZIER_CRV(TCrv)) {
	    ProdCrv = CagdCnvrtBzr2BspCrv(TCrv);
	    CagdCrvFree(TCrv);
	}
	else
	    ProdCrv = TCrv;
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return ProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bspline curves - multiply them coordinatewise, using Blossoming. M
*   The two curves are assumed compatible (same point type and domain).	     M
*   See also:								     M
*									     M
* "Multiplication as a General Operation for Splines", by Kenji Ueda, in     M
* curves and Surfaces in Geometric Design, Laurent, Mehaute and Shumaker     M
* (eds.), pp 475-482, A. K. Peters 1994.				     M
*									     M
* "Sliding Windows Algorithm for B-spline Multiplication", by X. Chen,	     M
* R. F. Riesenfeld, and E. Cohen, Solid and Physical Modeling 2007.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to multiply.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The product Crv1 * Crv2 coordinatewise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvMult						                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvBlossomMult, product                                               M
*****************************************************************************/
CagdCrvStruct *BspCrvBlossomMult(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2)
{
    CagdRType t;
    CagdCrvStruct *ProdCrv;

    /* Cannot blossom over discontinuities so recursively split there first. */
    if (BspCrvKnotC0Discont(Crv1, &t) || BspCrvKnotC0Discont(Crv2, &t)) {
        CagdCrvStruct
	    *Crv1a = CagdCrvSubdivAtParam(Crv1, t),
	    *Crv1b = Crv1a -> Pnext,
	    *Crv2a = CagdCrvSubdivAtParam(Crv2, t),
	    *Crv2b = Crv2a -> Pnext,
	    *ProdCrva = BspCrvBlossomMult(Crv1a, Crv2a),
	    *ProdCrvb = BspCrvBlossomMult(Crv1b, Crv2b);

	CagdCrvFreeList(Crv1a);
	CagdCrvFreeList(Crv2a);

	ProdCrv = CagdMergeCrvCrv(ProdCrva, ProdCrvb, FALSE);

	CagdCrvFree(ProdCrva);
	CagdCrvFree(ProdCrvb);

	return ProdCrv;
    }
    else {
	CagdPointType
	    PType = Crv1 -> PType;
	CagdBType
	    IsRational = CAGD_IS_RATIONAL_PT(PType);
	int i, k, ProdKVLen, ProdLength,
	    Order1 = Crv1 -> Order,
	    Order2 = Crv2 -> Order,
	    NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	    ProdOrder = Order1 + Order2 - 1;
	CagdRType **ProdPts, Divisor,
	    *ProdKV = BspKnotContinuityMergeTwo(Crv1 -> KnotVector,
						Crv1 -> Length + Order1,
						Order1,
						Crv2 -> KnotVector,
						Crv2 -> Length + Order2,
						Order2,
						ProdOrder, &ProdKVLen);

	ProdCrv = BspCrvNew(ProdLength = ProdKVLen - ProdOrder, ProdOrder,
			    PType);

	IRIT_GEN_COPY(ProdCrv -> KnotVector, ProdKV,
		      sizeof(CagdRType) * ProdKVLen);
	IritFree(ProdKV);
	ProdKV = ProdCrv -> KnotVector;
	ProdPts = ProdCrv -> Points;

	/* Compute the products' coefficients using blossoming. See Eqn (22) */
	/* in Kenji Ueda above cited paper.				     */
	Divisor = CagdIChooseK(Order1 - 1, ProdOrder - 1);

	GlblCrvBlossomMultInfo.Crv1 = Crv1;
	GlblCrvBlossomMultInfo.Crv2 = Crv2;
	GlblCrvBlossomMultInfo.ProdCrv = ProdCrv;
	GlblCrvBlossomMultInfo.PType = PType;
	GlblCrvBlossomMultInfo.Degree1 = Order1 - 1;
	GlblCrvBlossomMultInfo.Degree2 = Order2 - 1;
	GlblCrvBlossomMultInfo.PermVector = (int *) 
				          IritMalloc(sizeof(int) * ProdOrder);
	GlblCrvBlossomMultInfo.BlsmMultiSetMultplcty = (int *) 
				          IritMalloc(sizeof(int) * ProdOrder);
	GlblCrvBlossomMultInfo.BlsmMultiSetValues = (CagdRType *) 
				    IritMalloc(sizeof(CagdRType) * ProdOrder);
	GlblCrvBlossomMultInfo.VBlsmVals1 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * Order1);
	GlblCrvBlossomMultInfo.VBlsmVals2 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * Order2);

	for (i = 0; i < ProdLength; i++) {
	    IRIT_ZAP_MEM(GlblCrvBlossomMultInfo.Result,
			 sizeof(CagdRType) * CAGD_MAX_PT_COORD);

#	    ifdef BSP_MULT_BLOSSOM_MULTISET
		/* Build the multiset. */
	        GlblCrvBlossomMultInfo.BlsmMultiSetLen =
		    BspKnotsMultiplicityVector(
			&ProdCrv -> KnotVector[i + 1],
			ProdOrder - 1,
			GlblCrvBlossomMultInfo.BlsmMultiSetValues,
			GlblCrvBlossomMultInfo.BlsmMultiSetMultplcty);
		/* Compute all permutations. */
		SymbCrvBlossomMultiSetPerm(
			      GlblCrvBlossomMultInfo.PermVector,
			      ProdOrder - 1,
			      GlblCrvBlossomMultInfo.BlsmMultiSetValues,
			      GlblCrvBlossomMultInfo.BlsmMultiSetMultplcty,
			      GlblCrvBlossomMultInfo.BlsmMultiSetLen,
			      Order2 - 1, 1.0, &GlblCrvBlossomMultInfo,
			      SymbCrvBlossomMultPermute);
#	    else
	        GlblCrvBlossomMultInfo.VBlsmVals = &ProdCrv -> KnotVector[i + 1];
		SymbBlossomPermute(GlblCrvBlossomMultInfo.PermVector,
				   ProdOrder - 1, Order2 - 1,
				   SymbCrvBlossomMultPermute);
#	    endif /* BSP_MULT_BLOSSOM_MULTISET */

	    for (k = !IsRational; k <= NumCoords; k++)
	        ProdPts[k][i] = GlblCrvBlossomMultInfo.Result[k] / Divisor;
	}

	IritFree(GlblCrvBlossomMultInfo.PermVector);
	IritFree(GlblCrvBlossomMultInfo.BlsmMultiSetMultplcty);
	IritFree(GlblCrvBlossomMultInfo.BlsmMultiSetValues);
	IritFree(GlblCrvBlossomMultInfo.VBlsmVals1);
	IritFree(GlblCrvBlossomMultInfo.VBlsmVals2);

#	ifdef DEBUG_TEST_CRV_PROD
	{
	    int OldBspMultComputeMethod = BspMultComputeMethod;
	    CagdCrvStruct *ProdCrv2;

	    BspMultComputeMethod = BSP_MULT_INTERPOL;
	    ProdCrv2 = BspCrvMult(Crv1, Crv2);
	    BspMultComputeMethod = OldBspMultComputeMethod;

	    if (!CagdCrvsSame(ProdCrv, ProdCrv2, IRIT_EPS))
	        fprintf(stderr, "Crv product turned to be wrong\n");

	    IritFree(ProdCrv2);
	}
#	endif /* DEBUG_TEST_CRV_PROD */

	return ProdCrv;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function that is invoked from the permutations function.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Weight:  Multiplicative Weight to apply to this blossom.		     *
*   Data:    Actually the curve blossoming info.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None	                                                             *
*****************************************************************************/
static void SymbCrvBlossomMultPermute(IrtRType Weight, void *Data)
{
    CagdRType Eval1[CAGD_MAX_PT_COORD], *Eval2;
    SymbCrvBlossomMultInfoStruct
	*CrvBlsmMInfo = (SymbCrvBlossomMultInfoStruct *) Data;
    int i, j, k,
        *Perms = CrvBlsmMInfo -> PermVector;

#ifdef BSP_MULT_BLOSSOM_MULTISET
    for (i = j = k = 0; i < CrvBlsmMInfo -> BlsmMultiSetLen; i++) {
        int m;

        for (m = 0; m < Perms[i]; m++)
	    CrvBlsmMInfo -> VBlsmVals2[k++] =
	        CrvBlsmMInfo -> BlsmMultiSetValues[i];

	for (m = Perms[i];
	     m < CrvBlsmMInfo -> BlsmMultiSetMultplcty[i];
	     m++)
	    CrvBlsmMInfo -> VBlsmVals1[j++] =
	        CrvBlsmMInfo -> BlsmMultiSetValues[i];
    }
#else
    for (i = j = k = 0; i < CrvBlsmMInfo -> ProdCrv -> Order - 1; i++) {
        if (*Perms++) {
	    CrvBlsmMInfo -> VBlsmVals2[k++] =
	        CrvBlsmMInfo -> VBlsmVals[i];
	}
	else {
	    CrvBlsmMInfo -> VBlsmVals1[j++] =
	        CrvBlsmMInfo -> VBlsmVals[i];
	}
    }
#endif /* BSP_MULT_BLOSSOM_MULTISET */

    assert(j == CrvBlsmMInfo -> Degree1 &&
	   k == CrvBlsmMInfo -> Degree2);

    /* Evaluate the blossoms (Copy first as it is statically allocated). */
    Eval2 = CagdCrvBlossomEval(CrvBlsmMInfo -> Crv1,
			       CrvBlsmMInfo -> VBlsmVals1,
			       CrvBlsmMInfo -> Degree1);
    IRIT_GEN_COPY(Eval1, Eval2, sizeof(CagdRType) * CAGD_MAX_PT_COORD);
    Eval2 = CagdCrvBlossomEval(CrvBlsmMInfo -> Crv2,
			       CrvBlsmMInfo -> VBlsmVals2,
			       CrvBlsmMInfo -> Degree2);
    for (k = !CAGD_IS_RATIONAL_PT(CrvBlsmMInfo -> PType);
	 k <= CAGD_NUM_OF_PT_COORD(CrvBlsmMInfo -> PType);
	 k++)
        CrvBlsmMInfo -> Result[k] += Weight * Eval1[k] * Eval2[k];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes, recursively, all the permutations of placing PermToSelect more *
* into vector of length vecLength.  					     *
*   If done selecting, invoke the call back function.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   Vector:         A vector of length vecLength to place PermToSelect ones. *
*   VecLength:      Length of vector.                                        *
*   MultiSetValues: A vector of length MultiSetLen of unique values.	     *
*   MultiSetMultplcty:  Multiplicities of the unique values.                 *
*   MultiSetLen:    Length of multiset.                                      *
*   PermToSelect:   How many more values we need to select.                  *
*   Weight:	    of this blossom factor.				     *
*   CallBackData:   Data to pass to the call back function.		     *
*   CallBackFunc:   To be invoked if all values were selected.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void SymbCrvBlossomMultiSetPerm(int *Vector,
				       int VecLength,
				       CagdRType *MultiSetValues,
				       int *MultiSetMultplcty,
				       int MultiSetLen,
				       int PermToSelect,
				       CagdRType Weight,
				       void *CallBackData,
				       SymbBlossomMultFuncType CallBackFunc)
{
    int i;

    if (PermToSelect <= 0) {
        for (i = 0; i < MultiSetLen; i++)
	    Vector[i] = 0;
        CallBackFunc(Weight, CallBackData);
    }
    else {
        int m;

        /* Find out how many item we have in the multiset, excluding first. */
        for (i = 1, m = 0; i < MultiSetLen; i++)
	    m += MultiSetMultplcty[i];

	/* Select i items from first value and (PermToSelect - i) from rest. */
	for (i = IRIT_MAX(0, PermToSelect - m);
	     i <= IRIT_MIN(MultiSetMultplcty[0], PermToSelect);
	     i++) {
	    Vector[0] = i;
	    SymbCrvBlossomMultiSetPerm(&Vector[1], VecLength - 1,
				       &MultiSetValues[1],
				       &MultiSetMultplcty[1],
				       MultiSetLen - 1, PermToSelect - i,
				       Weight *
				         CagdIChooseK(i, MultiSetMultplcty[0]),
				       CallBackData, CallBackFunc);
	}
    }
}

#ifndef BSP_MULT_BLOSSOM_MULTISET

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes, recursively, all the permutations of placing  PermToSelect 1's *
* in a vector of length vecLength.  Places 0's in other locations.           *
*   If not more ones to be placed, invoke the call back function.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Vector:        A vector of length vecLength to place PermToSelect ones.  *
*   VecLength:     Length of vector.                                         *
*   PermToSelect:   How many ones we need to place.	                     *
*   CallBackFunc:  To be invoked if all ones were placed.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void SymbBlossomPermute(int *Vector,
			       int VecLength,
			       int PermToSelect,
			       SymbBlossomMultFuncType CallBackFunc)
{
    if (PermToSelect <= 0) {
        int i;

        for (i = 0; i < VecLength; i++)
	    Vector[i] = 0;
        CallBackFunc(1.0);
    }
    else {
        if (PermToSelect < VecLength) {
	    Vector[0] = 0;
	    SymbBlossomPermute(&Vector[1], VecLength - 1, PermToSelect,
			       CallBackFunc);
	}
        if (PermToSelect <= VecLength) {
	    Vector[0] = 1;
	    SymbBlossomPermute(&Vector[1], VecLength - 1, PermToSelect - 1,
			       CallBackFunc);
	}
    }
}

#endif /* BSP_MULT_BLOSSOM_MULTISET */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a matrix of size (Len x (Len - (Order1 - Order2)) of inner	     M
* products, SymbBspBasisInnerProd style.  matrix is allocated dynamically.   M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:       Knot vector of the basis functions (of Order).                 M
*   Len:      Length of knot vector KV.					     M
*   Order1:   Order of first basis function.                                 M
*   Order2:   Order of second basis function, <= Order1.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType **:  The allocated matrix and values of inner products.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProd, SymbBspBasisInnerProdPrep                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBspBasisInnerProdMat                                                 M
*****************************************************************************/
CagdRType **SymbBspBasisInnerProdMat(const CagdRType *KV,
				     int Len,
				     int Order1,
				     int Order2)
{
    int i, j,
	PtsLen = Len - Order1,
	DOrder = Order1 - Order2;
    CagdRType **M;

    M = (CagdRType **) IritMalloc(sizeof(CagdRType *) * PtsLen);
    for (i = 0; i < PtsLen; i++)
	M[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * (PtsLen - DOrder));

    SymbBspBasisInnerProdPrep(KV, Len, Order1, Order2);

    for (i = 0; i < PtsLen; i++)
        for (j = 0; j < PtsLen - DOrder; j++)
	    M[i][j] = SymbBspBasisInnerProd(i, j);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintInnerProdMat, FALSE) {
	    IRIT_INFO_MSG_PRINTF("Inner product matrix of size %d x %d\n",
				    PtsLen, PtsLen - DOrder);
	    for (i = 0; i < PtsLen; i++) {
	        for (j = 0; j < PtsLen - DOrder; j++)
		    IRIT_INFO_MSG_PRINTF("%5.3f ", M[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#endif /* DEBUG */

    return M;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares for the computation of the inner product of pair of basis       M
* functions over a similar function space.                                   M
*   The inner product is defined as "int( B1(t) * B2(t) )" where "int ( . )" M
* denotes the integral of the function over all the domain.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:       Knot vector of the basis functions (of Order).                 M
*   Len:      Length of knot vector KV.					     M
*   Order1:   Order of first basis function.                                 M
*   Order2:   Order of second basis function, <= Order1.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProd, SymbBspBasisInnerProdPrep2                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBspBasisInnerProdPrep                                                M
*****************************************************************************/
void SymbBspBasisInnerProdPrep(const CagdRType *KV,
			       int Len,
			       int Order1,
			       int Order2)
{
    if (GlblInnerProdCrv1 != NULL)
        CagdCrvFree(GlblInnerProdCrv1);
    if (GlblInnerProdCrv2 != NULL)
        CagdCrvFree(GlblInnerProdCrv2);

    /* Construct two zero curves with the proper orders and knot vectors. */
    GlblInnerProdCrv1 = BspCrvNew(Len - Order1, Order1, CAGD_PT_E1_TYPE);
    IRIT_ZAP_MEM(GlblInnerProdCrv1 -> Points[1],
	    sizeof(CagdRType) * GlblInnerProdCrv1 -> Length);
    CAGD_GEN_COPY(GlblInnerProdCrv1 -> KnotVector, KV,
		  sizeof(CagdRType) * Len);

    GlblInnerProdCrv2 = BspCrvNew(Len - Order2 - (Order1 - Order2) * 2,
				  Order2, CAGD_PT_E1_TYPE);
    IRIT_ZAP_MEM(GlblInnerProdCrv2 -> Points[1],
	    sizeof(CagdRType) * GlblInnerProdCrv2 -> Length);
    CAGD_GEN_COPY(GlblInnerProdCrv2 -> KnotVector, &KV[Order1 - Order2],
		  sizeof(CagdRType) * (Len - (Order1 - Order2) * 2));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares for the computation of the inner product of pair of basis       M
* functions over a similar function space.                                   M
*   The inner product is defined as "int( B1(t) * B2(t) )" where "int ( . )" M
* denotes the integral of the function over all the domain.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   KV1:      Knot vector of the first basis functions (of Order1).          M
*   KV2:      Knot vector of the second basis functions (of Order2).         M
*   Len1:     Length of knot vector KV1.				     M
*   Len2:     Length of knot vector KV2.				     M
*   Order1:   Order of first basis function.                                 M
*   Order2:   Order of second basis function, <= Order1.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProd2, SymbBspBasisInnerProdPrep                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBspBasisInnerProdPrep2                                               M
*****************************************************************************/
void SymbBspBasisInnerProdPrep2(const CagdRType *KV1,
				const CagdRType *KV2,
				int Len1,
				int Len2,
				int Order1,
				int Order2)
{
    if (GlblInnerProdCrv1 != NULL)
        CagdCrvFree(GlblInnerProdCrv1);
    if (GlblInnerProdCrv2 != NULL)
        CagdCrvFree(GlblInnerProdCrv2);

    /* Construct two zero curves with the proper orders and knot vectors. */
    GlblInnerProdCrv1 = BspCrvNew(Len1 - Order1, Order1, CAGD_PT_E1_TYPE);
    IRIT_ZAP_MEM(GlblInnerProdCrv1 -> Points[1],
	    sizeof(CagdRType) * GlblInnerProdCrv1 -> Length);
    CAGD_GEN_COPY(GlblInnerProdCrv1 -> KnotVector, KV1,
		  sizeof(CagdRType) * Len1);

    GlblInnerProdCrv2 = BspCrvNew(Len2 - Order2, Order2, CAGD_PT_E1_TYPE);
    IRIT_ZAP_MEM(GlblInnerProdCrv2 -> Points[1],
	    sizeof(CagdRType) * GlblInnerProdCrv2 -> Length);
    CAGD_GEN_COPY(GlblInnerProdCrv2 -> KnotVector, KV2,
		  sizeof(CagdRType) * Len2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the inner product of two basis functions over a similar         M
* function space, as created via SymbBspBasisInnerProdPrep.		     M
*   The inner product is defined as "int( B1(t) * B2(t) )" where "int ( . )" M
* denotes the integral of the function over all the domain.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Index1:   Index of first basis function.			             M
*   Index2:   Index of second basis function.  			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The value of the inner product.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProdPrep, SymbBspBasisInnerProdMat                      M
*   SymbBspBasisInnerProd2						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBspBasisInnerProd                                                    M
*****************************************************************************/
CagdRType SymbBspBasisInnerProd(int Index1, int Index2)
{
    int Order1, Order2;
    CagdRType *R, *KV1, *KV2, FirstVal, LastVal, MinVal, MaxVal;
    CagdCrvStruct *Crv1, *Crv2, *TCrv, *ICrv;
    
    if (GlblInnerProdCrv1 == NULL || GlblInnerProdCrv2 == NULL)
	return 0.0;
    KV1 = GlblInnerProdCrv1 -> KnotVector;
    KV2 = GlblInnerProdCrv2 -> KnotVector;
    Order1 = GlblInnerProdCrv1 -> Order;
    Order2 = GlblInnerProdCrv2 -> Order;
    CagdCrvDomain(GlblInnerProdCrv1, &MinVal, &MaxVal);

    if (Index1 < 0 || Index1 >= GlblInnerProdCrv1 -> Length ||
	Index2 < 0 || Index2 >= GlblInnerProdCrv2 -> Length ||
	KV1[Index1 + Order1] <= KV2[Index2] ||
	KV2[Index2 + Order2] <= KV1[Index1])
	return 0.0;		    /* No common domain to basis functions. */

    GlblInnerProdCrv1 -> Points[1][Index1] = 1.0;
    GlblInnerProdCrv2 -> Points[1][Index2] = 1.0;

    /* Compute indices. */
    FirstVal = IRIT_MAX(KV1[Index1], KV2[Index2]);
    FirstVal = IRIT_MAX(FirstVal, MinVal);

    LastVal = IRIT_MIN(KV1[Index1 + Order1], KV2[Index2 + Order2]);
    LastVal = IRIT_MIN(LastVal, MaxVal);

    /* Construct the two curves with minimal supporting domain. */
    Crv1 = CagdCrvRegionFromCrv(GlblInnerProdCrv1, FirstVal, LastVal);
    Crv2 = CagdCrvRegionFromCrv(GlblInnerProdCrv2, FirstVal, LastVal);
    TCrv = BspCrvMultAux(Crv1, Crv2);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    ICrv = BspCrvIntegrate(TCrv);
    CagdCrvFree(TCrv);

    R = CagdCrvEval(ICrv, LastVal);
    CagdCrvFree(ICrv);

    GlblInnerProdCrv1 -> Points[1][Index1] = 0.0;
    GlblInnerProdCrv2 -> Points[1][Index2] = 0.0;

    return R[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary routine. Subdivides the curves into Bezier curves, multiply      *
* the Bezier curves and merge them back. All is done simultaneously.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:   The two curves to multiply.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  The product Crv1 * Crv2 coordinatewise.                *
*****************************************************************************/
static CagdCrvStruct *BspCrvMultAux(CagdCrvStruct *Crv1, CagdCrvStruct *Crv2)
{
    CagdCrvStruct *Crv1a, *Crv1b, *Crv2a, *Crv2b, *CrvA, *CrvB, *ProdCrv;

    if (Crv1 -> Order != Crv1 -> Length ||
	Crv2 -> Order != Crv2 -> Length) {
	CagdRType
	    SubdivVal = Crv1 -> Order != Crv1 -> Length ?
				Crv1 -> KnotVector[(Crv1 -> Length +
						    Crv1 -> Order) >> 1] :
				Crv2 -> KnotVector[(Crv2 -> Length +
						    Crv2 -> Order) >> 1];

	/* Subdivide. */
	Crv1a = BspCrvSubdivAtParam(Crv1, SubdivVal);
	Crv1b = Crv1a -> Pnext;
	Crv1a -> Pnext = NULL;

	Crv2a = BspCrvSubdivAtParam(Crv2, SubdivVal);
	Crv2b = Crv2a -> Pnext;
	Crv2a -> Pnext = NULL;

	CrvA = BspCrvMultAux(Crv1a, Crv2a);
	CrvB = BspCrvMultAux(Crv1b, Crv2b);
	CagdCrvFree(Crv1a);
	CagdCrvFree(Crv1b);
	CagdCrvFree(Crv2a);
	CagdCrvFree(Crv2b);

	ProdCrv = CagdMergeCrvCrv(CrvA, CrvB, FALSE);
	CagdCrvFree(CrvA);
	CagdCrvFree(CrvB);
    }
    else {
	int i;
	CagdRType TMin, TMax;
	CagdCrvStruct
	    *Crv1Bzr = CagdCnvrtBsp2BzrCrv(Crv1),
	    *Crv2Bzr = CagdCnvrtBsp2BzrCrv(Crv2),
	    *ProdCrvAux = BzrCrvMult(Crv1Bzr, Crv2Bzr);

	CagdCrvDomain(Crv1, &TMin, &TMax);
	ProdCrv = CagdCnvrtBzr2BspCrv(ProdCrvAux);
	for (i = 0; i < ProdCrv -> Order; i++) {
	    ProdCrv -> KnotVector[i] = TMin;
	    ProdCrv -> KnotVector[i + ProdCrv -> Order] = TMax;
	}

	CagdCrvFree(Crv1Bzr);
	CagdCrvFree(Crv2Bzr);
	CagdCrvFree(ProdCrvAux);
    }

    return ProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bspline surfaces - multiply them coordinatewise.		     M
*   The two surfaces are promoted to same point type before multiplication   M
* can take place.  The two surfaces are assumed to hold the same domain.     M
*   See also BspMultComputationMethod.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf1, CSrf2:   The two surfaces to multiply.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The product Srf1 * Srf2 coordinatewise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMult, product                                                      M
*****************************************************************************/
CagdSrfStruct *BspSrfMult(const CagdSrfStruct *CSrf1,
			  const CagdSrfStruct *CSrf2)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *ProdSrf, *TSrf, *Srf1, *Srf2;

    if (CAGD_IS_BEZIER_SRF(CSrf1))
	Srf1 = CagdCnvrtBzr2BspSrf(CSrf1);
    else if (!BspSrfHasOpenEC(CSrf1)) {
	CagdSrfDomain(CSrf1, &UMin, &UMax, &VMin, &VMax);
	TSrf = CagdSrfRegionFromSrf(CSrf1, UMin, UMax, CAGD_CONST_U_DIR);
	Srf1 = CagdSrfRegionFromSrf(TSrf, VMin, VMax, CAGD_CONST_V_DIR);
	CagdSrfFree(TSrf);
    }
    else
        Srf1 = CagdSrfCopy(CSrf1);

    if (CAGD_IS_BEZIER_SRF(CSrf2))
	Srf2 = CagdCnvrtBzr2BspSrf(CSrf2);
    else if (!BspSrfHasOpenEC(CSrf2)) {
	CagdSrfDomain(CSrf2, &UMin, &UMax, &VMin, &VMax);
	TSrf = CagdSrfRegionFromSrf(CSrf2, UMin, UMax, CAGD_CONST_U_DIR);
	Srf2 = CagdSrfRegionFromSrf(TSrf, VMin, VMax, CAGD_CONST_V_DIR);
	CagdSrfFree(TSrf);
    }
    else
        Srf2 = CagdSrfCopy(CSrf2);

    if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, FALSE, FALSE, FALSE, FALSE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	return NULL;
    }

    if (BspMultComputeMethod == BSP_MULT_BLOSSOMING &&
	(Srf1 -> UOrder > 1 || Srf2 -> UOrder > 1) &&
	(Srf1 -> VOrder > 1 || Srf2 -> VOrder > 1)) {
        ProdSrf = BspSrfBlossomMult(Srf1, Srf2);
    }
    else {
        TSrf = BspSrfMultAux(Srf1, Srf2);

	if (CAGD_IS_BEZIER_SRF(TSrf)) {
	    ProdSrf = CagdCnvrtBzr2BspSrf(TSrf);
	    CagdSrfFree(TSrf);
	}
	else
	    ProdSrf = TSrf;
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return ProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary routine. Subdivides the surfaces into Bezier surfaces, multiply  *
* the Bezier surfaces and merge them back. All is done simultaneously.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:   The two surfaces to multiply.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:  The product Srf1 * Srf2 coordinatewise.                *
*****************************************************************************/
static CagdSrfStruct *BspSrfMultAux(CagdSrfStruct *Srf1, CagdSrfStruct *Srf2)
{
    CagdSrfStruct *Srf1a, *Srf1b, *Srf2a, *Srf2b, *SrfA, *SrfB, *ProdSrf;

    if (Srf1 -> UOrder != Srf1 -> ULength ||
	Srf2 -> UOrder != Srf2 -> ULength) {
	CagdRType
	    SubdivVal = Srf1 -> UOrder != Srf1 -> ULength ?
				Srf1 -> UKnotVector[(Srf1 -> ULength +
						     Srf1 -> UOrder) >> 1] :
				Srf2 -> UKnotVector[(Srf2 -> ULength +
						     Srf2 -> UOrder) >> 1];
	    int Mult1 = BspKnotFindMult(Srf1 -> UKnotVector, Srf1 -> UOrder,
					Srf1 -> ULength, SubdivVal),
		Mult2 = BspKnotFindMult(Srf2 -> UKnotVector, Srf2 -> UOrder,
					Srf2 -> ULength, SubdivVal),
	        C0Discont = Mult1 >= Srf1 -> UOrder || Mult2 >= Srf2 -> UOrder;

	/* Subdivide along U. */
	Srf1a = BspSrfSubdivAtParam(Srf1, SubdivVal, CAGD_CONST_U_DIR);
	Srf1b = Srf1a -> Pnext;
	Srf1a -> Pnext = NULL;

	Srf2a = BspSrfSubdivAtParam(Srf2, SubdivVal, CAGD_CONST_U_DIR);
	Srf2b = Srf2a -> Pnext;
	Srf2a -> Pnext = NULL;

	SrfA = BspSrfMultAux(Srf1a, Srf2a);
	SrfB = BspSrfMultAux(Srf1b, Srf2b);
	CagdSrfFree(Srf1a);
	CagdSrfFree(Srf1b);
	CagdSrfFree(Srf2a);
	CagdSrfFree(Srf2b);

	ProdSrf = CagdMergeSrfSrf(SrfA, SrfB, CAGD_CONST_U_DIR,
				  !C0Discont, FALSE);
	CagdSrfFree(SrfA);
	CagdSrfFree(SrfB);
    }
    else if (Srf1 -> VOrder != Srf1 -> VLength ||
	     Srf2 -> VOrder != Srf2 -> VLength) {
	CagdRType
	    SubdivVal = Srf1 -> VOrder != Srf1 -> VLength ?
				Srf1 -> VKnotVector[(Srf1 -> VLength +
						     Srf1 -> VOrder) >> 1] :
				Srf2 -> VKnotVector[(Srf2 -> VLength +
						     Srf2 -> VOrder) >> 1];
	    int Mult1 = BspKnotFindMult(Srf1 -> VKnotVector, Srf1 -> VOrder,
					Srf1 -> VLength, SubdivVal),
		Mult2 = BspKnotFindMult(Srf2 -> VKnotVector, Srf2 -> VOrder,
					Srf2 -> VLength, SubdivVal),
	        C0Discont = Mult1 >= Srf1 -> VOrder || Mult2 >= Srf2 -> VOrder;

	/* Subdivide along V. */
	Srf1a = BspSrfSubdivAtParam(Srf1, SubdivVal, CAGD_CONST_V_DIR);
	Srf1b = Srf1a -> Pnext;
	Srf1a -> Pnext = NULL;

	Srf2a = BspSrfSubdivAtParam(Srf2, SubdivVal, CAGD_CONST_V_DIR);
	Srf2b = Srf2a -> Pnext;
	Srf2a -> Pnext = NULL;

	SrfA = BspSrfMultAux(Srf1a, Srf2a);
	SrfB = BspSrfMultAux(Srf1b, Srf2b);
	CagdSrfFree(Srf1a);
	CagdSrfFree(Srf1b);
	CagdSrfFree(Srf2a);
	CagdSrfFree(Srf2b);

	ProdSrf = CagdMergeSrfSrf(SrfA, SrfB, CAGD_CONST_V_DIR,
				  !C0Discont, FALSE);
	CagdSrfFree(SrfA);
	CagdSrfFree(SrfB);
    }
    else {
	int i;
	CagdRType UMin, UMax, VMin, VMax;
	CagdSrfStruct
	    *Srf1Bzr = CagdCnvrtBsp2BzrSrf(Srf1),
	    *Srf2Bzr = CagdCnvrtBsp2BzrSrf(Srf2),
	    *ProdSrfAux = BzrSrfMult(Srf1Bzr, Srf2Bzr);

	CagdSrfDomain(Srf1, &UMin, &UMax, &VMin, &VMax);
	ProdSrf = CagdCnvrtBzr2BspSrf(ProdSrfAux);
	for (i = 0; i < ProdSrf -> UOrder; i++) {
	    ProdSrf -> UKnotVector[i] = UMin;
	    ProdSrf -> UKnotVector[i + ProdSrf -> UOrder] = UMax;
	}
	for (i = 0; i < ProdSrf -> VOrder; i++) {
	    ProdSrf -> VKnotVector[i] = VMin;
	    ProdSrf -> VKnotVector[i + ProdSrf -> VOrder] = VMax;
	}

	CagdSrfFree(Srf1Bzr);
	CagdSrfFree(Srf2Bzr);
	CagdSrfFree(ProdSrfAux);
    }

    return ProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two Bspline surfaces - multiply them coordinatewise, using	     M
* Blossoming.								     M
*   The two surfaces are assumed compatible (same point type & domain).	     M
*   See also:								     M
*									     M
* "Multiplication as a General Operation for Splines", by Kenji Ueda, in     M
* surfaces and Surfaces in Geometric Design, Laurent, Mehaute and Shumaker   M
* (eds.), pp 475-482, A. K. Peters 1994.				     M
*									     M
* "Sliding Windows Algorithm for B-spline Multiplication", by X. Chen,	     M
* R. F. Riesenfeld, and E. Cohen, Solid and Physical Modeling 2007.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:   The two surfaces to multiply.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The product Srf1 * Srf2 coordinatewise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfMult						                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfBlossomMult, product                                               M
*****************************************************************************/
CagdSrfStruct *BspSrfBlossomMult(const CagdSrfStruct *Srf1,
				 const CagdSrfStruct *Srf2)
{
    CagdRType t;
    CagdSrfStruct *ProdSrf;

    /* Cannot blossom over discontinuities so recursively split there first. */
    if (BspSrfKnotC0Discont(Srf1, CAGD_CONST_U_DIR, &t) ||
	BspSrfKnotC0Discont(Srf2, CAGD_CONST_U_DIR, &t)) {
        CagdSrfStruct
	    *Srf1a = CagdSrfSubdivAtParam(Srf1, t, CAGD_CONST_U_DIR),
	    *Srf1b = Srf1a -> Pnext,
	    *Srf2a = CagdSrfSubdivAtParam(Srf2, t, CAGD_CONST_U_DIR),
	    *Srf2b = Srf2a -> Pnext,
	    *ProdSrfa = BspSrfBlossomMult(Srf1a, Srf2a),
	    *ProdSrfb = BspSrfBlossomMult(Srf1b, Srf2b);

	CagdSrfFreeList(Srf1a);
	CagdSrfFreeList(Srf2a);

	ProdSrf = CagdMergeSrfSrf(ProdSrfa, ProdSrfb, CAGD_CONST_U_DIR,
				  FALSE, FALSE);

	CagdSrfFree(ProdSrfa);
	CagdSrfFree(ProdSrfb);

	return ProdSrf;
    }
    else if (BspSrfKnotC0Discont(Srf1, CAGD_CONST_V_DIR, &t) ||
	     BspSrfKnotC0Discont(Srf2, CAGD_CONST_V_DIR, &t)) {
        CagdSrfStruct
	    *Srf1a = CagdSrfSubdivAtParam(Srf1, t, CAGD_CONST_V_DIR),
	    *Srf1b = Srf1a -> Pnext,
	    *Srf2a = CagdSrfSubdivAtParam(Srf2, t, CAGD_CONST_V_DIR),
	    *Srf2b = Srf2a -> Pnext,
	    *ProdSrfa = BspSrfBlossomMult(Srf1a, Srf2a),
	    *ProdSrfb = BspSrfBlossomMult(Srf1b, Srf2b);

	CagdSrfFreeList(Srf1a);
	CagdSrfFreeList(Srf2a);

	ProdSrf = CagdMergeSrfSrf(ProdSrfa, ProdSrfb, CAGD_CONST_V_DIR,
				  FALSE, FALSE);

	CagdSrfFree(ProdSrfa);
	CagdSrfFree(ProdSrfb);

	return ProdSrf;
    }
    else {
	CagdPointType
	    PType = Srf1 -> PType;
	CagdBType
	    IsRational = CAGD_IS_RATIONAL_PT(PType);
	int i, j, k, ProdUKVLen, ProdVKVLen, ProdULength, ProdVLength,
	    UOrder1 = Srf1 -> UOrder,
	    VOrder1 = Srf1 -> VOrder,
	    UOrder2 = Srf2 -> UOrder,
	    VOrder2 = Srf2 -> VOrder,
	    NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	    ProdUOrder = UOrder1 + UOrder2 - 1,
	    ProdVOrder = VOrder1 + VOrder2 - 1;
	CagdRType **ProdPts, Divisor,
	    *ProdUKV = BspKnotContinuityMergeTwo(Srf1 -> UKnotVector,
						 Srf1 -> ULength + UOrder1,
						 UOrder1,
						 Srf2 -> UKnotVector,
						 Srf2 -> ULength + UOrder2,
						 UOrder2,
						 ProdUOrder, &ProdUKVLen),
	    *ProdVKV = BspKnotContinuityMergeTwo(Srf1 -> VKnotVector,
						 Srf1 -> VLength + VOrder1,
						 VOrder1,
						 Srf2 -> VKnotVector,
						 Srf2 -> VLength + VOrder2,
						 VOrder2,
						 ProdVOrder, &ProdVKVLen);

	ProdSrf = BspSrfNew(ProdULength = ProdUKVLen - ProdUOrder,
			    ProdVLength = ProdVKVLen - ProdVOrder,
			    ProdUOrder, ProdVOrder, PType);

	IRIT_GEN_COPY(ProdSrf -> UKnotVector, ProdUKV,
		      sizeof(CagdRType) * ProdUKVLen);
	IRIT_GEN_COPY(ProdSrf -> VKnotVector, ProdVKV,
		      sizeof(CagdRType) * ProdVKVLen);
	IritFree(ProdUKV);
	IritFree(ProdVKV);
	ProdUKV = ProdSrf -> UKnotVector;
	ProdVKV = ProdSrf -> VKnotVector;
	ProdPts = ProdSrf -> Points;

	/* Compute the products' coefficients using blossoming. See Eqn (29) */
	/* in Kenji Ueda above cited paper.				     */
	Divisor = CagdIChooseK(UOrder1 - 1, ProdUOrder - 1) *
		  CagdIChooseK(VOrder1 - 1, ProdVOrder - 1);

	GlblSrfBlossomMultInfo.Srf1 = Srf1;
	GlblSrfBlossomMultInfo.Srf2 = Srf2;
	GlblSrfBlossomMultInfo.ProdSrf = ProdSrf;
	GlblSrfBlossomMultInfo.Crv1 = NULL;
	GlblSrfBlossomMultInfo.Crv2 = NULL;
	GlblSrfBlossomMultInfo.PType = PType;
	GlblSrfBlossomMultInfo.UDegree1 = UOrder1 - 1;
	GlblSrfBlossomMultInfo.VDegree1 = VOrder1 - 1;
	GlblSrfBlossomMultInfo.UDegree2 = UOrder2 - 1;
	GlblSrfBlossomMultInfo.VDegree2 = VOrder2 - 1;

	GlblSrfBlossomMultInfo.UPermVector = (int *) 
				          IritMalloc(sizeof(int) * ProdUOrder);
	GlblSrfBlossomMultInfo.VPermVector = (int *) 
				          IritMalloc(sizeof(int) * ProdVOrder);

	GlblSrfBlossomMultInfo.UBlsmMultiSetMultplcty = (int *) 
				          IritMalloc(sizeof(int) * ProdUOrder);
	GlblSrfBlossomMultInfo.VBlsmMultiSetMultplcty = (int *) 
				          IritMalloc(sizeof(int) * ProdVOrder);

	GlblSrfBlossomMultInfo.UBlsmMultiSetValues = (CagdRType *) 
				    IritMalloc(sizeof(CagdRType) * ProdUOrder);
	GlblSrfBlossomMultInfo.VBlsmMultiSetValues = (CagdRType *) 
				    IritMalloc(sizeof(CagdRType) * ProdVOrder);

	GlblSrfBlossomMultInfo.UBlsmVals1 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * UOrder1);
	GlblSrfBlossomMultInfo.VBlsmVals1 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * VOrder1);

	GlblSrfBlossomMultInfo.UBlsmVals2 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * UOrder2);
	GlblSrfBlossomMultInfo.VBlsmVals2 = (CagdRType *)
				       IritMalloc(sizeof(CagdRType) * VOrder2);

	for (i = 0; i < ProdULength; i++) {
	    /* Build the U multiset. */
	    GlblSrfBlossomMultInfo.UBlsmMultiSetLen =
	        BspKnotsMultiplicityVector(
			    &ProdSrf -> UKnotVector[i + 1], ProdUOrder - 1,
			    GlblSrfBlossomMultInfo.UBlsmMultiSetValues,
			    GlblSrfBlossomMultInfo.UBlsmMultiSetMultplcty);
	    for (j = 0; j < ProdVLength; j++) {
	        /* Build the V multiset. */
	        GlblSrfBlossomMultInfo.VBlsmMultiSetLen =
		    BspKnotsMultiplicityVector(
			    &ProdSrf -> VKnotVector[j + 1], ProdVOrder - 1,
			    GlblSrfBlossomMultInfo.VBlsmMultiSetValues,
			    GlblSrfBlossomMultInfo.VBlsmMultiSetMultplcty);

		IRIT_ZAP_MEM(GlblSrfBlossomMultInfo.Result,
			sizeof(CagdRType) * CAGD_MAX_PT_COORD);

		/* Compute all permutations. */
		SymbSrfBlossomMultiSetPerm(
			      GlblSrfBlossomMultInfo.UPermVector,
			      ProdUOrder - 1,
			      GlblSrfBlossomMultInfo.UBlsmMultiSetValues,
			      GlblSrfBlossomMultInfo.UBlsmMultiSetMultplcty,
			      GlblSrfBlossomMultInfo.UBlsmMultiSetLen,
			      UOrder2 - 1,
			      &GlblSrfBlossomMultInfo,
			      1.0, SymbSrfBlossomMultPermute);

		for (k = !IsRational; k <= NumCoords; k++) {
		    ProdPts[k][CAGD_MESH_UV(ProdSrf, i, j)] =
		        GlblSrfBlossomMultInfo.Result[k] / Divisor;
		}
	    }
	}

	IritFree(GlblSrfBlossomMultInfo.UPermVector);
	IritFree(GlblSrfBlossomMultInfo.VPermVector);
	IritFree(GlblSrfBlossomMultInfo.UBlsmMultiSetMultplcty);
	IritFree(GlblSrfBlossomMultInfo.VBlsmMultiSetMultplcty);
	IritFree(GlblSrfBlossomMultInfo.UBlsmMultiSetValues);
	IritFree(GlblSrfBlossomMultInfo.VBlsmMultiSetValues);
	IritFree(GlblSrfBlossomMultInfo.UBlsmVals1);
	IritFree(GlblSrfBlossomMultInfo.VBlsmVals1);
	IritFree(GlblSrfBlossomMultInfo.UBlsmVals2);
	IritFree(GlblSrfBlossomMultInfo.VBlsmVals2);

	if (GlblSrfBlossomMultInfo.Crv1 != NULL)
	    CagdCrvFree(GlblSrfBlossomMultInfo.Crv1);
	GlblSrfBlossomMultInfo.Crv1 = NULL;
	if (GlblSrfBlossomMultInfo.Crv2 != NULL)
	    CagdCrvFree(GlblSrfBlossomMultInfo.Crv2);
	GlblSrfBlossomMultInfo.Crv2 = NULL;

#	ifdef DEBUG_TEST_SRF_PROD
	{
	    int OldBspMultComputeMethod = BspMultComputeMethod;
	    CagdSrfStruct *ProdSrf2,
	        *ProdSrfCp = CagdSrfCopy(ProdSrf);;

	    BspMultComputeMethod = BSP_MULT_BEZ_DECOMP;
	    ProdSrf2 = BspSrfMult(Srf1, Srf2);
	    BspMultComputeMethod = OldBspMultComputeMethod;

	    CagdMakeSrfsCompatible(&ProdSrfCp, &ProdSrf2, TRUE, TRUE,
				   TRUE, TRUE);
	    if (!CagdSrfsSame(ProdSrfCp, ProdSrf2, IRIT_EPS)) {
	        fprintf(stderr, "************ Srf blossom product is wrong\n");
	        CagdDbg(Srf1);
	        CagdDbg(Srf2);
	        fprintf(stderr, "***************** Products:\n");
	        CagdDbg(ProdSrf);
	        CagdDbg(ProdSrfCp);
		CagdDbg(ProdSrf2);
		assert(0);
	    }
	    IritFree(ProdSrfCp);
	    IritFree(ProdSrf2);
	}
#	endif /* DEBUG_TEST_SRF_PROD */

	return ProdSrf;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes, recursively, all the permutations of placing PermToSelect more *
* into vector of length vecLength.  					     *
*   If done selecting, invoke the call back function.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   UVector:          Vector of length vecLength to place PermToSelect ones. *
*   UVecLength:       Length of UVector.                                     *
*   UMultiSetValues:  Vector of length UMultiSetLen of unique values.	     *
*   UMultiSetMultplcty: U multiplicities of the unique values.               *
*   UMultiSetLen:     Length of U multiset.                                  *
*   UPermToSelect:    How many more values we need to select in U.           *
*   SrfBlossomMultInfo:   All surface blossing information.		     *
*   Weight:	      of this blossom factor.				     *
*   CallBackFunc:     To be invoked if all values were selected.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void SymbSrfBlossomMultiSetPerm(int *UVector,
				       int UVecLength,
				       CagdRType *UMultiSetValues,
				       int *UMultiSetMultplcty,
				       int UMultiSetLen,
				       int UPermToSelect,
				       SymbSrfBlossomMultInfoStruct
							  *SrfBlossomMultInfo,
				       CagdRType Weight,
				       SymbBlossomMultFuncType CallBackFunc)
{
    int i;

    /* Handle the U permutations here in this recursive function. */
    if (UPermToSelect <= 0) {
        /* Done with U - time to handle the second, V, direction. */
        for (i = 0; i < UMultiSetLen; i++)
	    UVector[i] = 0;
	SrfBlossomMultInfo -> UBlsmValsUpdated = TRUE;
	SymbCrvBlossomMultiSetPerm(SrfBlossomMultInfo -> VPermVector,
				   SrfBlossomMultInfo -> ProdSrf -> VOrder - 1,
				   SrfBlossomMultInfo -> VBlsmMultiSetValues,
				   SrfBlossomMultInfo -> VBlsmMultiSetMultplcty,
				   SrfBlossomMultInfo -> VBlsmMultiSetLen,
				   SrfBlossomMultInfo -> VDegree2,
				   Weight, SrfBlossomMultInfo, CallBackFunc);
    }
    else {
        int m;

        /* Find out how many item we have in U multiset, excluding first.   */
	for (i = 1, m = 0; i < UMultiSetLen; i++)
	    m += UMultiSetMultplcty[i];

	/* Select i items from 1st value and (UPermToSelect - i) from rest. */
	for (i = IRIT_MAX(0, UPermToSelect - m);
	     i <= IRIT_MIN(UMultiSetMultplcty[0], UPermToSelect);
	     i++) {
	    UVector[0] = i;
	    SymbSrfBlossomMultiSetPerm(&UVector[1], UVecLength - 1,
				       &UMultiSetValues[1],
				       &UMultiSetMultplcty[1],
				       UMultiSetLen - 1,
				       UPermToSelect - i,
				       SrfBlossomMultInfo,
				       Weight *
				           CagdIChooseK(i,
						        UMultiSetMultplcty[0]),
				       CallBackFunc);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function that is invoked from the permutations function.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Weight:  Multiplicative Weight to apply to this blossom.		     *
*   Data:    Actually the surface blossoming info.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None	                                                             *
*****************************************************************************/
static void SymbSrfBlossomMultPermute(IrtRType Weight, void *Data)
{
    int i, j, k, *Perms;
    CagdRType Eval1[CAGD_MAX_PT_COORD], *Eval2;
    SymbSrfBlossomMultInfoStruct
	*SrfBlsmMInfo = (SymbSrfBlossomMultInfoStruct *) Data;

    if (SrfBlsmMInfo -> UBlsmValsUpdated) {		       /* Handle U. */
        Perms = SrfBlsmMInfo -> UPermVector;
	for (i = j = k = 0; i < SrfBlsmMInfo -> UBlsmMultiSetLen; i++) {
	    int m;

	    for (m = 0; m < Perms[i]; m++)
	        SrfBlsmMInfo -> UBlsmVals2[k++] =
		    SrfBlsmMInfo -> UBlsmMultiSetValues[i];

	    for (m = Perms[i];
		 m < SrfBlsmMInfo -> UBlsmMultiSetMultplcty[i];
		 m++)
	        SrfBlsmMInfo -> UBlsmVals1[j++] =
		    SrfBlsmMInfo -> UBlsmMultiSetValues[i];
	}

	assert(j == SrfBlsmMInfo -> UDegree1 &&
	       k == SrfBlsmMInfo -> UDegree2);

	if (SrfBlsmMInfo -> Crv1 != NULL)
	    CagdCrvFree(SrfBlsmMInfo -> Crv1);
	if (SrfBlsmMInfo -> Crv2 != NULL)
	    CagdCrvFree(SrfBlsmMInfo -> Crv2);
	SrfBlsmMInfo -> Crv1 =
	    CagdSrfBlossomEvalU(SrfBlsmMInfo -> Srf1,
				SrfBlsmMInfo -> UBlsmVals1,
				SrfBlsmMInfo -> UDegree1);
	SrfBlsmMInfo -> Crv2 =
	    CagdSrfBlossomEvalU(SrfBlsmMInfo -> Srf2,
				SrfBlsmMInfo -> UBlsmVals2,
				SrfBlsmMInfo -> UDegree2);

	SrfBlsmMInfo -> UBlsmValsUpdated = FALSE;
    }

    /* Handle V. */
    Perms = SrfBlsmMInfo -> VPermVector;

    for (i = j = k = 0; i < SrfBlsmMInfo -> VBlsmMultiSetLen; i++) {
        int m;

        for (m = 0; m < Perms[i]; m++)
	    SrfBlsmMInfo -> VBlsmVals2[k++] =
	        SrfBlsmMInfo -> VBlsmMultiSetValues[i];

	for (m = Perms[i];
	     m < SrfBlsmMInfo -> VBlsmMultiSetMultplcty[i];
	     m++)
	    SrfBlsmMInfo -> VBlsmVals1[j++] =
	        SrfBlsmMInfo -> VBlsmMultiSetValues[i];
    }

    assert(j == SrfBlsmMInfo -> VDegree1 &&
	   k == SrfBlsmMInfo -> VDegree2);

    /* Evaluate the blossoms (Copy first as it is statically allocated). */
    Eval2 = CagdCrvBlossomEval(SrfBlsmMInfo -> Crv1,
			       SrfBlsmMInfo -> VBlsmVals1,
			       SrfBlsmMInfo -> VDegree1);
    IRIT_GEN_COPY(Eval1, Eval2, sizeof(CagdRType) * CAGD_MAX_PT_COORD);
    Eval2 = CagdCrvBlossomEval(SrfBlsmMInfo -> Crv2,
			       SrfBlsmMInfo -> VBlsmVals2,
			       SrfBlsmMInfo -> VDegree2);
    for (k = !CAGD_IS_RATIONAL_PT(SrfBlsmMInfo -> PType);
	 k <= CAGD_NUM_OF_PT_COORD(SrfBlsmMInfo -> PType);
	 k++)
        SrfBlsmMInfo -> Result[k] += Weight * Eval1[k] * Eval2[k];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Factors out a given bilinear term from a scalar surface, assuming it has M
* this term.								     M
* S(u,v) / {A[0] (1-u)(1-v) + A[1] u(1-v) + A[2] (1-u)v + A[3] uv}	     V
*   Note that typically a Bspline surface will not have this bilinear in all M
* its patches so use this function with care - this function does not verify M
* this existence. 							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To factor out a bilinear factor from.                           M
*   A:       Four coeficients of the scalar bilinear. 		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Srf / {A[0] (1-u)(1-v) + A[1] u(1-v) +		     M
*			      A[2] (1-u)v +     A[3] uv}.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfFactorUMinusV, BzrSrfFactorBilinear, MvarMVFactorUMinV             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfFactorBilinear                                                     M
*****************************************************************************/
CagdSrfStruct *BspSrfFactorBilinear(const CagdSrfStruct *Srf,
				    const CagdRType *A)
{
    CagdSrfStruct *Srf1 , *Srf2, *Srf1f, *Srf2f, *FactorSrf;

    if (Srf -> UOrder != Srf -> ULength) {
	CagdRType
	    SubdivVal = Srf -> UKnotVector[(Srf -> ULength +
					    Srf -> UOrder) >> 1];
	    int Mult = BspKnotFindMult(Srf -> UKnotVector, Srf -> UOrder,
				       Srf -> ULength, SubdivVal),
	        C0Discont = Mult >= Srf -> UOrder;

	/* Subdivide along U. */
	Srf1 = BspSrfSubdivAtParam(Srf, SubdivVal, CAGD_CONST_U_DIR);
	Srf2 = Srf1 -> Pnext;
	Srf1 -> Pnext = NULL;

	Srf1f = BspSrfFactorUMinusV(Srf1);
	Srf2f = BspSrfFactorUMinusV(Srf2);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	FactorSrf = CagdMergeSrfSrf(Srf1f, Srf2f, CAGD_CONST_U_DIR,
				    !C0Discont, FALSE);
	CagdSrfFree(Srf1f);
	CagdSrfFree(Srf2f);
    }
    else if (Srf -> VOrder != Srf -> VLength) {
	CagdRType
	    SubdivVal = Srf -> VKnotVector[(Srf -> VLength +
					    Srf -> VOrder) >> 1];
	    int Mult = BspKnotFindMult(Srf -> VKnotVector, Srf -> VOrder,
				       Srf -> VLength, SubdivVal),
	        C0Discont = Mult >= Srf -> VOrder;

	/* Subdivide along V. */
	Srf1 = BspSrfSubdivAtParam(Srf, SubdivVal, CAGD_CONST_V_DIR);
	Srf2 = Srf1 -> Pnext;
	Srf1 -> Pnext = NULL;

	Srf1f = BspSrfFactorUMinusV(Srf1);
	Srf2f = BspSrfFactorUMinusV(Srf2);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	FactorSrf = CagdMergeSrfSrf(Srf1f, Srf2f, CAGD_CONST_V_DIR,
				    !C0Discont, FALSE);
	CagdSrfFree(Srf1f);
	CagdSrfFree(Srf2f);
    }
    else {
	int i;
	CagdRType UMin, UMax, VMin, VMax, A1[4], A2[4];
	CagdSrfStruct *FactorSrfBzr,
	    *SrfBzr = CagdCnvrtBsp2BzrSrf(Srf);

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

        A1[0] = A[0] + (A[1] - A[0]) * UMin;
	A1[1] = A[0] + (A[1] - A[0]) * UMax;
	A1[2] = A[2] + (A[3] - A[2]) * UMin;
	A1[3] = A[2] + (A[3] - A[2]) * UMax;

        A2[0] = A1[0] + (A1[2] - A1[0]) * VMin;
	A2[1] = A1[0] + (A1[2] - A1[0]) * VMax;
	A2[2] = A1[1] + (A1[3] - A1[1]) * VMin;
	A2[3] = A1[1] + (A1[3] - A1[1]) * VMax;

	FactorSrfBzr = BzrSrfFactorBilinear(SrfBzr, A2);

	FactorSrf = CagdCnvrtBzr2BspSrf(FactorSrfBzr);

	for (i = 0; i < FactorSrf -> UOrder; i++) {
	    FactorSrf -> UKnotVector[i] = UMin;
	    FactorSrf -> UKnotVector[i + FactorSrf -> UOrder] = UMax;
	}
	for (i = 0; i < FactorSrf -> VOrder; i++) {
	    FactorSrf -> VKnotVector[i] = VMin;
	    FactorSrf -> VKnotVector[i + FactorSrf -> VOrder] = VMax;
	}

	CagdSrfFree(SrfBzr);
	CagdSrfFree(FactorSrfBzr);

#       ifdef DEBUG
	{
	    CagdSrfStruct *UMinusV, *MultSrf;

	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestUMinVResult, FALSE) {
	        /* Lets see if product with (u - v) recovers the original. */
		UMinusV = BspSrfNew(2, 2, 2, 2, CAGD_PT_E1_TYPE);
		BspKnotUniformOpen(2, 2, UMinusV -> UKnotVector);
		BspKnotAffineTransOrder2(UMinusV -> UKnotVector,
					 UMinusV -> UOrder,
					 UMinusV -> UOrder +
					     UMinusV -> ULength,
					 UMin, UMax);

		BspKnotUniformOpen(2, 2, UMinusV -> VKnotVector);
		BspKnotAffineTransOrder2(UMinusV -> VKnotVector,
					 UMinusV -> VOrder,
					 UMinusV -> VOrder +
					     UMinusV -> VLength,
					 VMin, VMax);

		UMinusV -> Points[1][0] =  UMin - VMin;
		UMinusV -> Points[1][1] =  UMax - VMin;
		UMinusV -> Points[1][2] =  UMin - VMax;
		UMinusV -> Points[1][3] =  UMax - VMax;

		MultSrf = SymbSrfMult(FactorSrf, UMinusV);
		fprintf(stderr, "Bsp Srf UMinusV factor is %s\n",
			CagdSrfsSame(Srf, MultSrf, IRIT_EPS) ? "Same"
							     : "Different");
		CagdSrfFree(UMinusV);
		CagdSrfFree(MultSrf);
	    }
	}
#       endif /* DEBUG */
    }

    return FactorSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Factors out a (u - v) term from a surface, assuming it has one.          M
*   Note that typically a Bspline surface will not have (u, v) in all its    M
* patches so use this function with care - this function does not verify     M
* this existence.  It is more common to have (u, v) only along symmetric     M
* diagonal patches of the Bspline surface, after symbolic operations like    M
* C1(u) - C2(v).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To factor out a (u - v) term from.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Srf / (u - v).                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfFactorUMinusV, BzrSrfFactorBilinear, MvarMVFactorUMinV             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfFactorUMinusV                                                      M
*****************************************************************************/
CagdSrfStruct *BspSrfFactorUMinusV(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *Srf1 , *Srf2, *Srf1f, *Srf2f, *FactorSrf;

    if (Srf -> UOrder != Srf -> ULength) {
	CagdRType
	    SubdivVal = Srf -> UKnotVector[(Srf -> ULength +
					    Srf -> UOrder) >> 1];

	/* Subdivide along U. */
	Srf1 = BspSrfSubdivAtParam(Srf, SubdivVal, CAGD_CONST_U_DIR);
	Srf2 = Srf1 -> Pnext;
	Srf1 -> Pnext = NULL;

	Srf1f = BspSrfFactorUMinusV(Srf1);
	Srf2f = BspSrfFactorUMinusV(Srf2);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	FactorSrf = CagdMergeSrfSrf(Srf1f, Srf2f, CAGD_CONST_U_DIR,
				    FALSE, FALSE);
	CagdSrfFree(Srf1f);
	CagdSrfFree(Srf2f);
    }
    else if (Srf -> VOrder != Srf -> VLength) {
	CagdRType
	    SubdivVal = Srf -> VKnotVector[(Srf -> VLength +
					    Srf -> VOrder) >> 1];

	/* Subdivide along V. */
	Srf1 = BspSrfSubdivAtParam(Srf, SubdivVal, CAGD_CONST_V_DIR);
	Srf2 = Srf1 -> Pnext;
	Srf1 -> Pnext = NULL;

	Srf1f = BspSrfFactorUMinusV(Srf1);
	Srf2f = BspSrfFactorUMinusV(Srf2);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	FactorSrf = CagdMergeSrfSrf(Srf1f, Srf2f, CAGD_CONST_V_DIR,
				    FALSE, FALSE);
	CagdSrfFree(Srf1f);
	CagdSrfFree(Srf2f);
    }
    else {
	CagdRType UMin, UMax, VMin, VMax, A[4];

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	if (!IRIT_APX_EQ(UMin, VMin) || !IRIT_APX_EQ(UMax, VMax)) {
	    /* Not a diagonal. No need to reduce this one. */
	    FactorSrf = CagdSrfCopy(Srf);
	}
	else {
	    int i;
	    CagdSrfStruct *FactorSrfBzr, *SrfBzr;

	    SrfBzr = CagdCnvrtBsp2BzrSrf(Srf);

	    /*****************************************************************/
	    /* Srf[u,v]/(u-v) =						     */
	    /*	Srf[s,t] / { (u0-v0)(1-s)(1-t) + (u1-v0)s(1-t)               */
	    /*		        (u0-v1)(1-s)t + (u1-v1)st }		     */
	    /*****************************************************************/

	    A[0] = UMin - VMin;
	    A[1] = UMax - VMin;
	    A[2] = UMin - VMax;
	    A[3] = UMax - VMax;

	    FactorSrfBzr = BzrSrfFactorBilinear(SrfBzr, A);

	    FactorSrf = CagdCnvrtBzr2BspSrf(FactorSrfBzr);

	    for (i = 0; i < FactorSrf -> UOrder; i++) {
	        FactorSrf -> UKnotVector[i] = UMin;
		FactorSrf -> UKnotVector[i + FactorSrf -> UOrder] = UMax;
	    }
	    for (i = 0; i < FactorSrf -> VOrder; i++) {
	        FactorSrf -> VKnotVector[i] = VMin;
		FactorSrf -> VKnotVector[i + FactorSrf -> VOrder] = VMax;
	    }

	    CagdSrfFree(SrfBzr);
	    CagdSrfFree(FactorSrfBzr);

#	    ifdef DEBUG
	    {
	        CagdSrfStruct *UMinusV, *MultSrf;
		CagdPointType
		    PType = Srf -> PType;
		CagdBType
		    IsRational = CAGD_IS_RATIONAL_PT(PType);
		int i,
		    NumCoords = CAGD_NUM_OF_PT_COORD(PType);

		IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestUMinVResult, FALSE) {
		    /* Lets see if product with (u - v) recovers original. */
		    UMinusV = BspSrfNew(2, 2, 2, 2, PType);
		    BspKnotUniformOpen(2, 2, UMinusV -> UKnotVector);
		    BspKnotAffineTransOrder2(UMinusV -> UKnotVector,
					     UMinusV -> UOrder,
					     UMinusV -> UOrder +
					         UMinusV -> ULength,
					     UMin, UMax);

		    BspKnotUniformOpen(2, 2, UMinusV -> VKnotVector);
		    BspKnotAffineTransOrder2(UMinusV -> VKnotVector,
					     UMinusV -> VOrder,
					     UMinusV -> VOrder +
					         UMinusV -> VLength,
					     VMin, VMax);

		    for (i = !IsRational; i <= NumCoords; i++) {
		        UMinusV -> Points[i][0] =  UMin - VMin;
			UMinusV -> Points[i][1] =  UMax - VMin;
			UMinusV -> Points[i][2] =  UMin - VMax;
			UMinusV -> Points[i][3] =  UMax - VMax;
		    }

		    MultSrf = SymbSrfMult(FactorSrf, UMinusV);
		    fprintf(stderr, "Bsp Srf UMinusV factor is %s\n",
			    CagdSrfsSame(Srf, MultSrf, IRIT_EPS) ? "Same"
			                                       : "Different");
		    CagdSrfFree(UMinusV);
		    CagdSrfFree(MultSrf);
		}
	    }
#	    endif /* DEBUG */
	}
    }

    return FactorSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bspline curve - computes its derivative curve (Hodograph) M
* using the quotient rule for differentiation.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Bspline curve to differentiate.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Differentiated rational Bspline curve.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, BspCrvDerive, BzrCrvDeriveRational, CagdCrvDerive          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDeriveRational, derivatives                                        M
*****************************************************************************/
CagdCrvStruct *BspCrvDeriveRational(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *TCrv1, *TCrv2, *DeriveCrv,
        *DCrvW = NULL,
	*DCrvX = NULL,
	*DCrvY = NULL,
	*DCrvZ = NULL;

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvW)
	DCrvW = BspCrvDerive(CrvW);
    else {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_EXPECTED);
	return NULL;
    }

    if (CrvX) {
	DCrvX = BspCrvDerive(CrvX);

	TCrv1 = BspCrvMult(DCrvX, CrvW);
	TCrv2 = BspCrvMult(CrvX, DCrvW);

	CagdCrvFree(CrvX);
	CrvX = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }
    if (CrvY) {
	DCrvY = BspCrvDerive(CrvY);

	TCrv1 = BspCrvMult(DCrvY, CrvW);
	TCrv2 = BspCrvMult(CrvY, DCrvW);

	CagdCrvFree(CrvY);
	CrvY = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }
    if (CrvZ) {
	DCrvZ = BspCrvDerive(CrvZ);

	TCrv1 = BspCrvMult(DCrvZ, CrvW);
	TCrv2 = BspCrvMult(CrvZ, DCrvW);

	CagdCrvFree(CrvZ);
	CrvZ = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }
    
    TCrv1 = BspCrvMult(CrvW, CrvW);
    CagdCrvFree(CrvW);
    CrvW = TCrv1;

    if (!CagdMakeCrvsCompatible(&CrvW, &CrvX, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvZ, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	return NULL;
    }
    /* Note CrvX/Y/Z might be different due to possible C0 discontinuities. */
    if (!CagdMakeCrvsCompatible(&CrvX, &CrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvX, &CrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvY, &CrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvX, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvZ, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	return NULL;
    }

    DeriveCrv = SymbCrvMergeScalar(CrvW, CrvX, CrvY, CrvZ);

    if (CrvX) {
	CagdCrvFree(CrvX);
	CagdCrvFree(DCrvX);
    }
    if (CrvY) {
	CagdCrvFree(CrvY);
	CagdCrvFree(DCrvY);
    }
    if (CrvZ) {
	CagdCrvFree(CrvZ);
	CagdCrvFree(DCrvZ);
    }
    if (CrvW) {
	CagdCrvFree(CrvW);
	CagdCrvFree(DCrvW);
    }

    return DeriveCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bspline surface - computes its derivative surface in      M
* direction Dir, using the quotient rule for differentiation.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Bspline surface to differentiate.                           M
*   Dir:         Direction of Differentiation. Either U or V.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Differentiated rational Bspline surface.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDerive, BzrSrfDerive, BspSrfDerive, BzrSrfDeriveRational          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDeriveRational, derivatives                                        M
*****************************************************************************/
CagdSrfStruct *BspSrfDeriveRational(const CagdSrfStruct *Srf,
				    CagdSrfDirType Dir)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *TSrf1, *TSrf2, *DeriveSrf,
        *DSrfW = NULL,
	*DSrfX = NULL,
	*DSrfY = NULL,
	*DSrfZ = NULL;

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    if (SrfW)
	DSrfW = BspSrfDerive(SrfW, Dir);
    else {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_EXPECTED);
	return NULL;
    }

    if (SrfX) {
	DSrfX = BspSrfDerive(SrfX, Dir);

	TSrf1 = BspSrfMult(DSrfX, SrfW);
	TSrf2 = BspSrfMult(SrfX, DSrfW);

	CagdSrfFree(SrfX);
	SrfX = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    if (SrfY) {
	DSrfY = BspSrfDerive(SrfY, Dir);

	TSrf1 = BspSrfMult(DSrfY, SrfW);
	TSrf2 = BspSrfMult(SrfY, DSrfW);

	CagdSrfFree(SrfY);
	SrfY = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    if (SrfZ) {
	DSrfZ = BspSrfDerive(SrfZ, Dir);

	TSrf1 = BspSrfMult(DSrfZ, SrfW);
	TSrf2 = BspSrfMult(SrfZ, DSrfW);

	CagdSrfFree(SrfZ);
	SrfZ = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    
    TSrf1 = BspSrfMult(SrfW, SrfW);
    CagdSrfFree(SrfW);
    SrfW = TSrf1;

    if (!CagdMakeSrfsCompatible(&SrfW, &SrfX, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfZ, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	return NULL;
    }

    /* Note SrfX/Y/Z might be different due to possible C0 discontinuities. */
    if (!CagdMakeSrfsCompatible(&SrfX, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfX, &SrfZ, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfY, &SrfZ, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfX, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfZ, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	return NULL;
    }

    DeriveSrf = SymbSrfMergeScalar(SrfW, SrfX, SrfY, SrfZ);

    if (SrfX) {
	CagdSrfFree(SrfX);
	CagdSrfFree(DSrfX);
    }
    if (SrfY) {
	CagdSrfFree(SrfY);
	CagdSrfFree(DSrfY);
    }
    if (SrfZ) {
	CagdSrfFree(SrfZ);
	CagdSrfFree(DSrfZ);
    }
    if (SrfW) {
	CagdSrfFree(SrfW);
	CagdSrfFree(DSrfW);
    }

    return DeriveSrf;
}

