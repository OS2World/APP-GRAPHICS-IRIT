/******************************************************************************
* BspCoxDB.c - Bspline evaluation using Cox - de Boor recursive algorithm.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define COX_DB_IRIT_EPS		1e-20

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to a static data, holding the value of the curve at      M
* the prescribed parametric location t.                                      M
*   Uses the recursive Cox de-Boor algorithm, to evaluate the spline, which  M
* is not very efficient if many evaluations of the same curve are necessary  M
*   Use knot insertion when multiple evaluations are to be performed.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To evaluate at the given parametric location t.                M
*   t:        The parameter value at which the curve Crv is to be evaluated. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvEvalCoxDeBoor, evaluation, Bsplines                                M
*****************************************************************************/
CagdRType *BspCrvEvalCoxDeBoor(const CagdCrvStruct *Crv, CagdRType t)
{
    IRIT_STATIC_DATA CagdRType Pt[CAGD_MAX_PT_COORD];
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdRType *pPoints, *pPt, *BasisFunc, *pBasisFunc;
    int i, j, l, IndexFirst,
	k = Crv -> Order,
	Length = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);

    BasisFunc = BspCrvCoxDeBoorBasis(Crv -> KnotVector, k,
				     Length, Crv -> Periodic,
				     t, &IndexFirst);

    /* Clear the point. */
    IRIT_ZAP_MEM(Pt, sizeof(CagdRType) * CAGD_MAX_PT_COORD);
    pPt = IsNotRational ? &Pt[1] : Pt;

    /* And finally multiply the basis functions with the control polygon. */
    if (Crv -> Periodic) {
	for (i = IsNotRational; i <= MaxCoord; i++, pPt++) {
	    pPoints = &Crv -> Points[i][j = IndexFirst];
	    pBasisFunc = BasisFunc;
	    for (l = 0; l++ < k; ) {
	        if (j++ >= Length)
		    pPoints = &Crv -> Points[i][j = 0];
	        *pPt += *pPoints++ * *pBasisFunc++;
	    }
	}
    }
    else {
	for (i = IsNotRational; i <= MaxCoord; i++, pPt++) {
	    pPoints = &Crv -> Points[i][IndexFirst];
	    pBasisFunc = BasisFunc;
	    for (l = 0; l++ < k; )
	        *pPt += *pPoints++ * *pBasisFunc++;
	}
    }

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to a vector of size Order, holding values of the non     M
* zero basis functions of a given curve at given parametric location t.	     M
*   This vector SHOULD NOT BE FREED. Although it is dynamically allocated,   M
* the returned pointer does not point to the beginning of this memory and it M
* it be maintained by this routine (i.e. it might be freed next time this    M
* routine is being called).						     M
*   IndexFirst returns the index of first non zero basis function for the    M
* given parameter value t.						     M
*   Uses the recursive Cox de Boor algorithm, to evaluate the Bspline basis  M
* functions.								     M
*   Algorithm:								     M
* Use the following recursion relation with B(i,0) == 1.                     M
*									     M
*          t     -    t(i)            t(i+k)    -   t                        V
* B(i,k) = --------------- B(i,k-1) + --------------- B(i+1,k-1)             V
*          t(i+k-1) - t(i)            t(i+k) - t(i+1)                        V
*									     M
*   Starting with constant Bspline (k == 0) only one basis function is non   M
* zero and is equal to one. This is the constant Bspline spanning interval   M
* t(i)...t(i+1) such that t(i) <= t < t(i+1). We then raise this constant    M
* Bspline to the prescribed Order and find in this process all the basis     M
* functions that are non zero in t for order Order. Sound simple hah!?	     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To evaluate the Bspline Basis functions for this space.   M
*   Order:         Of the geometry.                                          M
*   Len:           Number of control points in the geometry. The length of   M
*                  KnotVector is equal to Len + Order (+(Order-1) if	     M
*		   periodic).				                     M
*   Periodic:      TRUE if freeform is periodic.			     M
*   t:             At which the Bspline basis functions are to be evaluated. M
*   IndexFirst:    Index of the first Bspline basis function that might be   M
*                  non zero.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   A vector of length Order thats holds the values of the    M
*                  Bspline basis functions for the given t. A Bspline of     M
*                  order Order might have at most Order non zero basis       M
*                  functions that will hence start at IndexFirst and upto    M
*                  (*IndexFirst + Order - 1).                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCoxDeBoorBasis, evaluation, Bsplines                               M
*****************************************************************************/
CagdRType *BspCrvCoxDeBoorBasis(const CagdRType *KnotVector,
				int Order,
				int Len,
				CagdBType Periodic,
				CagdRType t,
				int *IndexFirst)
{
    IRIT_STATIC_DATA int
	BSize = 0;
    IRIT_STATIC_DATA CagdRType
	*B = NULL;
    CagdRType *BasisFunc;
    int i, l, Index, KVLen,
	OrigLen = Len;

    /* If the conditions are of a Bezier, evaluate a Bezier instead. */
    if (Periodic) {
        Len += Order - 1;
    }
    else if (Order == Len && BspKnotHasBezierKV(KnotVector, Len, Order)) {
	*IndexFirst = 0;

	return BzrCrvEvalBasisFuncs(Order, (t - KnotVector[Order - 1]) /
				 (KnotVector[Order] - KnotVector[Order - 1]));
    }

    KVLen = Order + Len;

    if (!BspKnotParamInDomain(KnotVector, Len, Order, FALSE, t))
	CAGD_FATAL_ERROR(CAGD_ERR_T_NOT_IN_CRV);
    CAGD_VALIDATE_MIN_MAX_DOMAIN(t, KnotVector[Order - 1], KnotVector[Len])

    Index = BspKnotLastIndexLE(KnotVector, KVLen, t);

    /* Starting the recursion from constant splines - one spline is non     */
    /* zero and is equal to one. This is the spline that starts at Index.   */
    /* As we are going to reference index-1 we increment the buffer by one  */
    /* and save 0.0 at index-1. We then initialize the constant spline      */
    /* values - all are zero but the one from t(i) to t(i+1).               */
    if (BSize < Order + 1) {
	if (B != NULL)
	    IritFree(B);
	BSize = (Order + 1) * 2;
	B = (CagdRType *) IritMalloc(sizeof(CagdRType) * BSize);
    }
    IRIT_ZAP_MEM(B, (Order + 1) * sizeof(CagdRType));
    BasisFunc = &B[1];

    if (Index >= KVLen - 1) {
	/* We are at the end of the parametric domain and this is open      */
	/* end condition - simply return last point.			    */
        BasisFunc[Order - 1] = 1.0;

	*IndexFirst = Len - Order;
	return BasisFunc;
    }
    else
        BasisFunc[0] = 1.0;

    /* Here is the tricky part. we raise these constant splines to the      */
    /* required order of the curve Crv for the given parameter t. There are */
    /* at most order non zero function at param. value t. These functions   */
    /* start at Index-order+1 up to Index (order functions overwhole).      */
    for (i = 2; i <= Order; i++) {            /* Goes through all orders... */
	/* This code is highly optimized from the commented out code below. */
	/* for (l = i - 1; l >= 0; l--) {				    */
	/*  s1 = (KnotVector[Index + l] - KnotVector[Index + l - i + 1]);   */
	/*  s1 = COX_DB_IRIT_APX_EQ(s1, 0.0)					    */
	/*	? 0.0 : (t - KnotVector[Index + l - i + 1]) / s1;	    */
	/*  s2 = (KnotVector[Index + l + 1]-KnotVector[Index + l - i + 2]); */
	/*  s2 = COX_DB_IRIT_APX_EQ(s2, 0.0)					    */
	/*	? 0.0 : (KnotVector[Index + l + 1] - t) / s2;		    */
	/*  BasisFunc[l] = s1 * BasisFunc[l - 1] + s2 * BasisFunc[l];	    */
	CagdRType const 
	    *KVIndexl = &KnotVector[Index + i - 1],        /* KV[Index + l] */
	    *KVIndexl1 = &KVIndexl[1],		       /* KV[Index + l + 1] */
	    *KVIndexli1 = &KnotVector[Index];      /* KV[Index + l - i + 1] */
	CagdRType s1, s2, s2inv,
	    *BF = &BasisFunc[i - 1];

	if ((s2 = *KVIndexl1 - KVIndexli1[1]) >= COX_DB_IRIT_EPS)
	    s2inv = 1.0 / s2;
	else
	    s2inv = 0.0;

	for (l = i - 1; l >= 0; l--) {  /* And all basis funcs. of order i. */
	    if (s2inv == 0.0) {
	        *BF-- = 0.0;
		KVIndexl1--;
	    }
	    else
	        *BF-- *= (*KVIndexl1-- - t) * s2inv;

	    if ((s1 = *KVIndexl-- - *KVIndexli1--) >= COX_DB_IRIT_EPS) {
	        s2inv = 1.0 / s1;
	        BF[1] += BF[0] * (t - KVIndexli1[1]) * s2inv;
	    }
	    else
	        s2inv = 0.0;
	}
    }

    if ((*IndexFirst = Index - Order + 1) >= OrigLen)
	*IndexFirst -= OrigLen;

    return BasisFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the index of the first non zero basis function as returned by the M
* BspCrvCoxDeBoorBasis function.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To evaluate the Bspline Basis functions for this space.   M
*   Order:         Of the geometry.                                          M
*   Len:           Number of control points in the geometry. The length of   M
*                  KnotVector is equal to Len + Order.                       M
*   t:             At which the Bspline basis functions are to be evaluated. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           The index.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCoxDeBoorIndexFirst, evaluation, Bsplines                          M
*****************************************************************************/
int BspCrvCoxDeBoorIndexFirst(const CagdRType *KnotVector,
			      int Order,
			      int Len,
			      CagdRType t)
{
    int Index,
	KVLen = Order + Len;

    Index = BspKnotLastIndexLE(KnotVector, KVLen, t);

    if (Index >= KVLen - 1)
	return Len - Order;
    else
        return Index - Order + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes multiple evaluations of the given spline space basis functions, M
* as prescribed by KnotVector and Order, at the requested NumOfParams        M
* parameter values, Params.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:   Knot sequence defining the spline space.                   M
*   KVLength:     Length of KnotVector.					     M
*   Order:        Of the spline space.					     M
*   Periodic:     TRUE if space is periodic.				     M
*   Params:       At which to evaluate and compute the spline functions.     M
*   NumOfParams:  Size of Params vector.                                     M
*   EvalType:     Type of evaluation requested:  value (position), 1st       M
*		  derivative, or 2nd derivative.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBspBasisFuncEvalStruct *:   A vector of size NumOfParams of          M
*			evaluation results, each holding the index of the    M
*			first non zero basis function and the (at most)      M
*			Order non zero basis function values.                M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspBasisFuncMultEval                                                     M
*****************************************************************************/
CagdBspBasisFuncEvalStruct *BspBasisFuncMultEval(const CagdRType *KnotVector,
						 int KVLength,
						 int Order,
						 CagdBType Periodic,
						 CagdRType *Params,
						 int NumOfParams,
						 CagdBspBasisFuncMultEvalType
						                      EvalType)
{
    int i, j,
        Degree = Order - 1;
    CagdRType *V;
    CagdBspBasisFuncEvalStruct *LOArray,
	*RetArray = IritMalloc(sizeof(CagdBspBasisFuncEvalStruct) *
			       NumOfParams);

    /* Allocate the data structure. */
    for (i = 0; i < NumOfParams; i++)
      RetArray[i].BasisFuncsVals = IritMalloc(sizeof(CagdRType) * Order);

#   ifdef DEBUG
    {
        CagdRType
	    TMin = KnotVector[Order - 1],
	    TMax = KnotVector[KVLength - Order];

        for (i = 0; i < NumOfParams; i++)
	    assert (Params[i] >= TMin || Params[i] < TMax);
    }
#   endif /* DEBUG */

    switch (EvalType) {
	case CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE:
	    for (i = 0; i < NumOfParams; i++) {
	        V = BspCrvCoxDeBoorBasis(KnotVector, Order, KVLength - Order,
					 Periodic, Params[i],
					 &RetArray[i].FirstBasisFuncIndex);
		CAGD_GEN_COPY(RetArray[i].BasisFuncsVals,
			      V, sizeof(CagdRType) * Order);
	    }
	    break;
	case CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST:
	case CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER2ND:
	case CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER3RD:
	    /* Evalue the basis functions of one lower order, recursively. */
	    LOArray = BspBasisFuncMultEval(&KnotVector[1], KVLength - 2,
					   Order - 1, Periodic, Params,
					   NumOfParams,
					   (CagdBspBasisFuncMultEvalType)
					            ((int) EvalType - 1));

	    /* Use the recursive Bspline derivative formula to compute the */
	    /* derivative values:					   */
	    /*                  Bi,k-1(t)        Bi+1,k-1(t)     	   */
	    /* B'i,k(t) = k ( ------------- - ----------------- )          */
	    /*                t(i+k) - t(i)   t(i+k+1) - t(i+1)		   */
	    for (i = 0; i < NumOfParams; i++) {
	        int Idx = BspCrvCoxDeBoorIndexFirst(KnotVector, Order,
						    KVLength - Order,
						    Params[i]);
		CagdRType
		    *BasisFuncsVals = RetArray[i].BasisFuncsVals;

		RetArray[i].FirstBasisFuncIndex = Idx;

		for (j = Idx; j < Idx + Order; j++, BasisFuncsVals++) {
		    int LOIdx1 = j - LOArray[i].FirstBasisFuncIndex - 1,
			LOIdx2 = LOIdx1 + 1;
		    *BasisFuncsVals = 0.0;

		    if (LOIdx1 >= 0 && LOIdx1 < Degree) {
		        /* Lower order basis function is valid here. */
		        *BasisFuncsVals +=
			    Degree * LOArray[i].BasisFuncsVals[LOIdx1] /
					     (KnotVector[j + Degree]
						       - KnotVector[j]);
		    }
		    if (LOIdx2 >= 0 && LOIdx2 < Degree) {
		        /* Lower order basis function is valid here. */
		        *BasisFuncsVals -=
			    Degree * LOArray[i].BasisFuncsVals[LOIdx2] /
					     (KnotVector[j + Degree + 1]
						  - KnotVector[j + 1]);
		    }
		}
	    }

	    /* Free the lower order basis functions we computed. */
	    BspBasisFuncMultEvalFree(LOArray, NumOfParams);
	    break;
        default:
	    CAGD_FATAL_ERROR(CAGD_ERR_WRONG_DERIV_ORDER);
	    return NULL;
    }

    return RetArray;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prints to stdout the allocated structure for multiple evaluations.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Evals:        Structure to print.                                        M
*   Order:        Of evaluated basis functions.                              M
*   Params: 	  Parameters the basis functions were evaluated at.          M
*   NumOfParams:  Size of Evals/Params - number of parameters/evaluations.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspBasisFuncMultEvalPrint                                                M
*****************************************************************************/
void BspBasisFuncMultEvalPrint(const CagdBspBasisFuncEvalStruct *Evals,
			       int Order,
			       CagdRType *Params,
			       int NumOfParams)
{
    int i, j;

    for (i = 0; i < NumOfParams; i++) {
        printf("%d) t = %8.5lf | Idx1st = %d |",
	       i, Params[i], Evals[i].FirstBasisFuncIndex);

	for (j = 0; j < Order; j++)
	    printf(" %8.5lg%c", Evals[i].BasisFuncsVals[j],
		   j < Order - 1 ? ',' : '\n');
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the allocated structure for multiple evaluations.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Evals:        Structure to free.                                         M
*   NumOfParams:  Size of Evals - number of parameter evaluations we have.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspBasisFuncMultEvalFree                                                 M
*****************************************************************************/
void BspBasisFuncMultEvalFree(CagdBspBasisFuncEvalStruct *Evals,
			      int NumOfParams)
{
    int i;

    for (i = 0; i < NumOfParams; i++)
	IritFree(Evals[i].BasisFuncsVals);

    IritFree(Evals);
}
