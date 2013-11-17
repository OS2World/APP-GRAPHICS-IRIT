/******************************************************************************
 * Blossom.c - Polar forms and Blossoming related functions.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb. 99.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define CAGD_BLSM_STRUCT_ALLOC_ONCE	1

typedef struct CagdBlsmSymbStruct {
    CagdRType *Coefs;
    int Min, Max;		 /* Domain of no zeros in the vector Coefs. */
} CagdBlsmSymbStruct;

#ifdef DEBUG
void CagdDbgBlsmPrintAlphaMat(CagdBlsmAlphaCoeffStruct *A);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Same as CagdBlossomEval, but computes the result symbolically.  That is, M
* get the contribution of each input coefficients to this blossom.	     M
*   This function assumes no Order multiplicity of knots in interior of KV.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Order:       Order of the freeform geometry,			     M
*   Knots:       Knots of the freeform geometry.  If NULL assumed Bezier.    M
*   KnotsLen:    Length of Knots knot vectors.				     M
*   BlsmVals:    Blossoming values to consider.                              M
*   BlsmLen:     Length of BlsmVals vector.                                  M
*   RetIdxFirst: Index of first input coefficient to blend returned vector   M
*		 with.							     M
*   RetLength:   Length of returned blend vector.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    Vector of blending values of the input coefficients for  M
*		  this blossom evaluation.				     M
*		    This vector is maintained by this function and should    M
*		  not be freed by the caller of this function.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdBlossomEval, CagdBlsmAddRowAlphaCoef      M
*   CagdBlsmAllocAlphaCoef, CagdBlsmCopyAlphaCoef,			     M
*   CagdBlsmFreeAlphaCoef, CagdBlsmScaleAlphaCoef,			     M
*   CagdBlsmSetDomainAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmEvalSymb                                                         M
*****************************************************************************/
CagdRType *CagdBlsmEvalSymb(int Order,
			    const CagdRType *Knots,
			    int KnotsLen,
			    const CagdRType *BlsmVals,
			    int BlsmLen,
			    int *RetIdxFirst,
			    int *RetLength)
{
    IRIT_STATIC_DATA int
	LclSymbVecLen = 0,
        CacheIdxFirst = 0,
        CacheIdxLength = 0,
        CacheVecLen = 0,
	CacheOrder = 0,
	CacheKnotsLen = 0,
	CacheBlsmLen = 0;
    IRIT_STATIC_DATA CagdRType
        *CacheCoefs = NULL,
        *CacheBlsmVals = NULL,
        *CacheKnots = NULL;
    IRIT_STATIC_DATA CagdBlsmSymbStruct
	*LclSymbVec = NULL;
    int k, i, m, BlossomValsNotInOrder,
	PtLen = KnotsLen - Order,
	J = 0,
        LastJ = -1;
    CagdRType
	TMin = Knots == NULL ? 0.0 : Knots[0],
	TMax = Knots == NULL ? 1.0 : Knots[KnotsLen - 1];

    /* Compare to cached data and if different, re-cache new data. */
    if (CacheOrder == Order &&
	CacheKnotsLen == KnotsLen &&
	CacheBlsmLen == BlsmLen &&
	IRIT_GEN_CMP(CacheBlsmVals, BlsmVals,
		sizeof(CagdRType) * CacheBlsmLen) == 0 &&
	((Knots != NULL &&
	  CacheKnots != NULL &&
	  IRIT_GEN_CMP(CacheKnots, Knots,
		       sizeof(CagdRType) * CacheKnotsLen) == 0) ||
	 (Knots == NULL && CacheKnots == NULL))) {
        /* Cached data identical to prev. call - return prev. result. */
        *RetIdxFirst = CacheIdxFirst;
	*RetLength = CacheIdxLength;
	return CacheCoefs;
    }
    CacheOrder = Order;
    CacheKnotsLen = KnotsLen;
    CacheBlsmLen = BlsmLen;
    if (CacheVecLen < BlsmLen || CacheVecLen < KnotsLen) {
        IritFree(CacheBlsmVals);
        IritFree(CacheKnots);

	CacheVecLen = IRIT_MAX(BlsmLen, KnotsLen) * 2;
	CacheKnots = Knots == NULL ? NULL
				 : IritMalloc(sizeof(CagdRType) * CacheVecLen);
	CacheBlsmVals = IritMalloc(sizeof(CagdRType) * CacheVecLen);
	CacheCoefs = IritMalloc(sizeof(CagdRType) * CacheVecLen);
    }
    IRIT_GEN_COPY(CacheBlsmVals, BlsmVals, sizeof(CagdRType) * CacheBlsmLen);
    if (Knots != NULL) {
        if (CacheKnots == NULL)
	    CacheKnots = IritMalloc(sizeof(CagdRType) * CacheVecLen);
        IRIT_GEN_COPY(CacheKnots, Knots, sizeof(CagdRType) * CacheKnotsLen);
    }

    /* End of cache maintenance. Have to do the computation... */
#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmEvlSymb, FALSE) {
	    IRIT_INFO_MSG("Blossom Vals: ");
	    for (i = 0; i < BlsmLen; i++)
		IRIT_INFO_MSG_PRINTF(" %f", BlsmVals[i]);
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    /* We are about to blend control points, in place - make a copy: */
    if (LclSymbVecLen < KnotsLen + 1) {
        CagdRType *p;
        if (LclSymbVec != NULL)
	    IritFree(LclSymbVec);

	/* In order to prevent from doing this very often. */
	LclSymbVecLen = (KnotsLen + 1) * 2;

	/* Allocate the CagdBlsmSymbStruct vector and its inner vectors      */
	/* at once and then loop and assign the inner vectors in.            */
	LclSymbVec = (CagdBlsmSymbStruct *)
	    IritMalloc(sizeof(CagdBlsmSymbStruct) * LclSymbVecLen +
		       sizeof(CagdRType) * IRIT_SQR(LclSymbVecLen));
	p = (CagdRType *) &LclSymbVec[LclSymbVecLen];
	for (i = 0; i < LclSymbVecLen; i++)
	    LclSymbVec[i].Coefs = &p[LclSymbVecLen * i];
    }

    if (Order <= BlsmLen)
	CAGD_FATAL_ERROR(CAGD_ERR_NUM_KNOT_MISMATCH);

    /* Initialize the symbolic vector so only LclSymbVec[i][i] = 1.0. */
    for (i = 0; i < KnotsLen; i++) {
        IRIT_ZAP_MEM(LclSymbVec[i].Coefs, sizeof(CagdRType) * KnotsLen);
	LclSymbVec[i].Coefs[i] = 1.0;
	LclSymbVec[i].Min = LclSymbVec[i].Max = i;
    }

    /* Examine if values are in order. */
    for (k = 1; k < BlsmLen; k++)
        if (BlsmVals[k -1] > BlsmVals[k])
	    break;
    BlossomValsNotInOrder = k < BlsmLen;

    for (k = 0; k < BlsmLen; k++) {
	const CagdRType *K;
	CagdRType u,
	    BlsmVal = BlsmVals[k];
	int OrderK = Order - 1 - k;
	CagdBlsmSymbStruct *S;

	if (BlsmVal < TMin - IRIT_UEPS || BlsmVal > TMax + IRIT_UEPS)
	    CAGD_FATAL_ERROR(CAGD_ERR_WRONG_DOMAIN);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymbMatrix, FALSE) {
	        IRIT_INFO_MSG("S Vector:\n");
		for (i = 0; i < PtLen; i++) {
		    for (m = 0; m < PtLen; m++)
		        IRIT_INFO_MSG_PRINTF(" %.7lg", LclSymbVec[i].Coefs[m]);
		    IRIT_INFO_MSG("\n");
		}
	    }
	}
#       endif /* DEBUG */

	if (Knots == NULL) {					  /* Bezier. */
	    u = BlsmVal;
	    J = Order - 1;

	    for (i = J, S = &LclSymbVec[J]; i > J - OrderK && i > 0; i--, S--) {
	        CagdRType
		    *S0Coefs = S[0].Coefs,
		    *S1Coefs = S[-1].Coefs;
		
		for (m = PtLen; m-- > 0; S0Coefs++)
		    *S0Coefs = u * *S0Coefs + (1.0 - u) * *S1Coefs++;
	    }
	}
	else {							 /* Bspline. */
	    CagdRType
		ClippedBlsmVal = BlsmVal;

	    /* Set J to be the last knot LE to ClippedBlsmVal. */
	    if (k == 0 || BlossomValsNotInOrder) {
	        CAGD_VALIDATE_MIN_MAX_DOMAIN(ClippedBlsmVal, TMin, TMax);
		J = BspKnotLastIndexLE(Knots, KnotsLen, ClippedBlsmVal);
	    }
	    else {
	        for (J = LastJ;
		     J < KnotsLen &&
			 (Knots[J] <= ClippedBlsmVal ||
			  IRIT_APX_EQ_EPS(Knots[J], ClippedBlsmVal, IRIT_UEPS));
		     J++);
		J--;
	    }

	    if (J - LastJ > 1) {
		if (k == 0)
		    LastJ = J;
		else
		    J = LastJ + 1;
	    }
	    LastJ = J;

	    for (i = J, S = &LclSymbVec[J], K = &Knots[i];
		 i > J - OrderK && i > 0;
		 i--, S--, K--) {
	        int MinIdx = IRIT_MIN(S[0].Min, S[-1].Min),
		    MaxIdx = IRIT_MAX(S[0].Max, S[-1].Max);
	        CagdRType
		    *S0Coefs = &S[0].Coefs[MinIdx],
		    *S1Coefs = &S[-1].Coefs[MinIdx],
		    u = K[OrderK] - *K > 0 ? (BlsmVal - *K) / (K[OrderK] - *K)
		                           : 0.0,
		    u1 = 1.0 - u;

		for (m = MaxIdx - MinIdx; m-- >= 0; S0Coefs++)
		    *S0Coefs = u * *S0Coefs + u1 * *S1Coefs++;

		S[0].Min = MinIdx;
		S[0].Max = MaxIdx;
	    }
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmEvlSymb, FALSE) {
	    IRIT_INFO_MSG("Blossom result: ");
	    for (i = 0; i < PtLen; i++)
		IRIT_INFO_MSG_PRINTF(" %f", LclSymbVec[J].Coefs[i]);
	    IRIT_INFO_MSG("\n");
	}
    }
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymbMatrix, FALSE) {
	    IRIT_INFO_MSG("S Vector:\n");
	    for (i = 0; i < PtLen; i++) {
	        for (m = 0; m < PtLen; m++)
		    IRIT_INFO_MSG_PRINTF(" %.7lg", LclSymbVec[i].Coefs[m]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    for (i = 0; i < PtLen && LclSymbVec[J].Coefs[i] == 0; i++);
    *RetIdxFirst = i;
    for ( ; i < PtLen && LclSymbVec[J].Coefs[i] != 0; i++);
    *RetLength = i - *RetIdxFirst;

    IRIT_GEN_COPY(CacheCoefs, LclSymbVec[J].Coefs,
		  sizeof(CagdRType) * KnotsLen);
    CacheIdxFirst = *RetIdxFirst;
    CacheIdxLength = *RetLength;

    return LclSymbVec[J].Coefs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Blossom over the given points, Pts, with knot sequence      M
* Knots, and Blossoming factors BlsmVals.  Evaluation is conducted via the   M
* Cox - De Boor algorithm with a possibly different parameter at each        M
* iteration as prescribed via the Blossoming factors.  Note that the Bezier  M
* case is supported via the case for which the Knots are NULL.		     M
*   This function assumes no Order multiplicity of knots in interior of KV.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:        Coefficeints or scalar control points to blossom.            M
*   PtsStep:    Step size between coefficients, typically one.		     M
*   Order:      Order of the freeform geometry,				     M
*   Knots:      Knots of the freeform geometry.  If NULL assumed Bezier.     M
*   KnotsLen:   Length of Knots knot vectors.				     M
*   BlsmVals:   Blossoming values to consider.                               M
*   BlsmLen:    Length of BlsmVals vector.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Evaluated Blossom                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdBlossomEvalSymb                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlossomEval                                                          M
*****************************************************************************/
CagdRType CagdBlossomEval(const CagdRType *Pts,
			  int PtsStep,
			  int Order,
			  const CagdRType *Knots,
			  int KnotsLen,
			  const CagdRType *BlsmVals,
			  int BlsmLen)
{
    IRIT_STATIC_DATA int
	LclVecLen = 0;
    IRIT_STATIC_DATA CagdRType
	*LclVec = NULL;
    int k, i, BlossomValsNotInOrder,
	J = 0,
        PtsLen = KnotsLen - Order,
        LastJ = -1;
    CagdRType
	TMin = Knots == NULL ? 0.0 : Knots[0],
	TMax = Knots == NULL ? 1.0 : Knots[KnotsLen - 1];

    /* We are about to blend control points, in place - make a copy: */
    if (LclVecLen < KnotsLen + 1) {
	if (LclVec != NULL)
	    IritFree(LclVec);

	/* In order to prevent from doing this very often. */
	LclVecLen = (KnotsLen + 1) * 2;
	LclVec = (CagdRType *) IritMalloc(sizeof(CagdRType) * LclVecLen);
    }
    if (PtsStep == 1)
        IRIT_GEN_COPY(LclVec, Pts, sizeof(CagdRType) * PtsLen);
    else {
        for (i = 0; i < PtsLen; i++) {
	    LclVec[i] = *Pts;
	    Pts += PtsStep;
	}
    }
    IRIT_ZAP_MEM(&LclVec[PtsLen], sizeof(CagdRType) * Order);

    if (Order <= BlsmLen)
	CAGD_FATAL_ERROR(CAGD_ERR_NUM_KNOT_MISMATCH);

    /* Examine if values are in order. */
    for (k = 1; k < BlsmLen; k++)
        if (BlsmVals[k -1] > BlsmVals[k])
	    break;
    BlossomValsNotInOrder = k < BlsmLen;

    for (k = 0; k < BlsmLen; k++) {
	const CagdRType *K;
	CagdRType u, *R,
	    BlsmVal = BlsmVals[k];
	int OrderK = Order - 1 - k;

	if (BlsmVal < TMin || BlsmVal > TMax)
	    CAGD_FATAL_ERROR(CAGD_ERR_WRONG_DOMAIN);

	if (Knots == NULL) {					  /* Bezier. */
	    u = BlsmVal;
	    J = Order - 1;

	    for (i = J, R = &LclVec[J]; i > J - OrderK && i > 0; i--, R--)
		*R = u * *R + (1.0 - u) * R[-1];
	}
	else {							 /* Bspline. */
	    CagdRType
		ClippedBlsmVal = BlsmVal;

	    /* Set J to be the last knot LE to ClippedBlsmVal. */
	    if (k == 0 || BlossomValsNotInOrder) {
	        CAGD_VALIDATE_MIN_MAX_DOMAIN(ClippedBlsmVal, TMin, TMax);
		J = BspKnotLastIndexLE(Knots, KnotsLen, ClippedBlsmVal);
	    }
	    else {
	        for (J = LastJ;
		     J < KnotsLen &&
			 (Knots[J] <= ClippedBlsmVal ||
			  IRIT_APX_EQ_EPS(Knots[J], ClippedBlsmVal, IRIT_UEPS));
		     J++);
		J--;
	    }

	    if (J - LastJ > 1) {
		if (k == 0)
		    LastJ = J;
		else
		    J = LastJ + 1;
	    }
	    LastJ = J;

	    for (i = J, R = &LclVec[J], K = &Knots[i];
		 i > J - OrderK && i > 0;
		 i--, R--, K--) {
		u = K[OrderK] - *K > 0 ? (BlsmVal - *K) / (K[OrderK] - *K)
		                       : 0.0;

		*R = u * *R + (1.0 - u) * R[-1];
	    }
	}
    }

#   ifdef DEBUG_TEST_BLSM_EVAL_SYMB
    {
        int ii, jj, ll;
	CagdRType
	    RR = 0,
	    *R = CagdBlsmEvalSymb(Order, Knots, KnotsLen,
				  BlsmVals, BlsmLen, &ii, &ll);

	for (jj = ii; jj < ii + ll; jj++)
	    RR += OrigPts[jj * PtsStep] * R[jj];

	if (!IRIT_APX_EQ(RR, LclVec[J])) {
	    IRIT_WARNING_MSG_PRINTF("************** Error in symb blossoming ***************");
	    exit(0);
	}
    }
#   endif /* DEBUG_TEST_BLSM_EVAL_SYMB */

    return LclVec[J];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Blossom over the given curve, Crv, and Blossoming factors   M
* BlsmVals.          					                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to blossom.				             M
*   BlsmVals:   Blossoming values to consider.                               M
*   BlsmLen:    Length of BlsmVals vector; assumed less than curve order!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    Evaluated Blossom.	                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfBlossomEval, CagdSrfBlossomEvalU				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvBlossomEval                                                       M
*****************************************************************************/
CagdRType *CagdCrvBlossomEval(const CagdCrvStruct *Crv,
			      const CagdRType *BlsmVals,
			      int BlsmLen)
{
    IRIT_STATIC_DATA CagdRType RetPt[CAGD_MAX_PT_COORD];
    int i, j, l, IdxFirst, Length;
    CagdRType
        *BlsmBlends = CagdBlsmEvalSymb(Crv -> Order,
				       CAGD_IS_BEZIER_CRV(Crv) ? NULL
						           : Crv -> KnotVector,
				       Crv -> Order + Crv -> Length,
				       BlsmVals, BlsmLen,
				       &IdxFirst, &Length);

    for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType);
	 i <= CAGD_NUM_OF_PT_COORD(Crv -> PType);
	 i++) {
        CagdRType
	    *Pts = Crv -> Points[i];

        RetPt[i] = 0.0;
        for (j = IdxFirst, l = Length; l-- > 0; j++)
	    RetPt[i] += Pts[j] * BlsmBlends[j];
    }

    return RetPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Blossom over the given surface, Srf, and Blossoming factors M
* BlsmU/VVals.         					                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to blossom.				             M
*   BlsmUVals:  U Blossoming values to consider.                             M
*   BlsmULen:   Length of BlsmUVals vector; assumed less than Srf U order!   M
*   BlsmVVals:  V Blossoming values to consider.                             M
*   BlsmVLen:   Length of BlsmVVals vector; assumed less than Srf V order!   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    Evaluated Blossom.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfBlossomEvalU, CagdCrvBlossomEval				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfBlossomEval                                                       M
*****************************************************************************/
CagdRType *CagdSrfBlossomEval(const CagdSrfStruct *Srf,
			      const CagdRType *BlsmUVals,
			      int BlsmULen,
			      const CagdRType *BlsmVVals,
			      int BlsmVLen)
{
    IRIT_STATIC_DATA CagdRType RetPt[CAGD_MAX_PT_COORD];
    IRIT_STATIC_DATA CagdCrvStruct
        *TempCrv = NULL;
    int i, j, k, l, UBlendIdxFirst, UBlendLen, VBlendIdxFirst, VBlendLen;
    CagdRType
	* const *SrfPts = Srf -> Points;
    CagdRType **Pts, *Pt, *VBlends,
        *UBlends = CagdBlsmEvalSymb(Srf -> UOrder,
				    CAGD_IS_BEZIER_SRF(Srf) ? NULL
						          : Srf -> UKnotVector,
				    Srf -> UOrder + Srf -> ULength,
				    BlsmUVals, BlsmULen,
				    &UBlendIdxFirst, &UBlendLen);

    /* Allocate a temporary curve to hold control points of temporary evals. */
    if (TempCrv == NULL || TempCrv -> Length < Srf -> VLength) {
        if (TempCrv != NULL)
	    CagdCrvFree(TempCrv);
        TempCrv = BzrCrvNew(Srf -> VLength  * 2, CAGD_PT_P9_TYPE);
    }
    Pts = TempCrv -> Points;
    TempCrv -> Order = Srf -> VOrder;

    for (k = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	 k <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	 k++) {
	IRIT_ZAP_MEM(Pts[k], sizeof(CagdRType) * Srf -> VLength);

        for (j = 0, Pt = Pts[k]; j < Srf -> VLength; j++, Pt++) {
	    const CagdRType
	        *SrfPt = &SrfPts[k][CAGD_MESH_UV(Srf, UBlendIdxFirst, j)];

	    for (i = UBlendIdxFirst, l = UBlendLen; l-- > 0; )
	        *Pt += *SrfPt++ * UBlends[i++];
	}
    }

    VBlends = CagdBlsmEvalSymb(Srf -> VOrder,
			       CAGD_IS_BEZIER_SRF(Srf) ? NULL
						       : Srf -> VKnotVector,
			       Srf -> VOrder + Srf -> VLength,
			       BlsmVVals, BlsmVLen,
			       &VBlendIdxFirst, &VBlendLen);
    for (k = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	 k <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	 k++) {
        CagdRType
	    *Pts = &TempCrv -> Points[k][VBlendIdxFirst];

        RetPt[k] = 0.0;
        for (i = VBlendIdxFirst, l = VBlendLen; l-- > 0; )
	    RetPt[k] += *Pts++ * VBlends[i++];
    }

    return RetPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Blossom over the given surface, Srf, and Blossoming factors M
* BlsmUVals, in the U direction only.  Returned is a curve in V.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to blossom.				             M
*   BlsmUVals:  U Blossoming values to consider.                             M
*   BlsmULen:   Length of BlsmUVals vector; assumed less than Srf U order!   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Evaluated Blossom in U as a curve in V.  This curve  M
*		        holds as many control points as Srf has in the V     M
*			direction.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfBlossomEval, CagdCrvBlossomEval				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfBlossomEvalU                                                      M
*****************************************************************************/
CagdCrvStruct *CagdSrfBlossomEvalU(const CagdSrfStruct *Srf,
				   const CagdRType *BlsmUVals,
				   int BlsmULen)
{
    CagdCrvStruct *Crv;
    int i, j, k, l, UBlendIdxFirst, UBlendLen;
    CagdRType
	* const *SrfPts = Srf -> Points;
    CagdRType **Pts, *Pt,
        *UBlends = CagdBlsmEvalSymb(Srf -> UOrder,
				    CAGD_IS_BEZIER_SRF(Srf) ? NULL
						          : Srf -> UKnotVector,
				    Srf -> UOrder + Srf -> ULength,
				    BlsmUVals, BlsmULen,
				    &UBlendIdxFirst, &UBlendLen);

    /* Allocate a curve to hold control points of the U blossom evals. */
    if (CAGD_IS_BEZIER_SRF(Srf)) {
        Crv = BzrCrvNew(Srf -> VOrder, Srf -> PType);
    }
    else {
        Crv = BspCrvNew(Srf -> VLength, Srf -> VOrder, Srf -> PType);
	IRIT_GEN_COPY(Crv -> KnotVector, Srf -> VKnotVector,
		 sizeof(CagdRType) * (Srf -> VLength + Srf -> VOrder));
    }
    Pts = Crv -> Points;

    for (k = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	 k <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	 k++) {
	IRIT_ZAP_MEM(Pts[k], sizeof(CagdRType) * Srf -> VLength);

        for (j = 0, Pt = Pts[k]; j < Srf -> VLength; j++, Pt++) {
	    const CagdRType
	        *SrfPt = &SrfPts[k][CAGD_MESH_UV(Srf, UBlendIdxFirst, j)];

	    for (i = UBlendIdxFirst, l = UBlendLen; l-- > 0; )
	        *Pt += *SrfPt++ * UBlends[i++];
	}
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the product of two adjacent degree raising matrics.  Matrics    M
* are adjacent if A1 raises from Order to Order+k and A2 from Order+k to     M
* Order+n. Returned matrix is a degree raising matrix from Order to Order+n. M
*                                                                            *
* PARAMETERS:                                                                M
*   A1, A2:  matrices to multiply.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBlsmAlphaCoeffStruct *:    Resulting product matrix.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBlossomDegreeRaiseMat, CagdCrvDegreeRaise			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDegreeRaiseMatProd                                                   M
*****************************************************************************/
CagdBlsmAlphaCoeffStruct *CagdDegreeRaiseMatProd(CagdBlsmAlphaCoeffStruct *A1,
						 CagdBlsmAlphaCoeffStruct *A2)
{
    int i, j, k;
    CagdBlsmAlphaCoeffStruct
	*A = CagdBlsmCopyAlphaCoef(A2);    /* A2 could also hold A1 * A2... */

    assert(A1 -> NewOrder == A2 -> Order);
    assert(A1 -> NewLength == A2 -> Length);

    /* Multiply the matrices. */
    for (i = 0; i < A1 -> Length; i++) {
        for (j = 0; j < A2 -> NewLength; j++) {
	    CagdRType
	        R = 0.0;

	    for (k = 0; k < A1 -> NewLength; k++)
	        R += A1 -> Rows[k][i] * A2 -> Rows[j][k];

	    A -> Rows[j][i] = R;
	}
    }

    /* And copy the rest, simple staff. */
    A -> Order = A1 -> Order;
    A -> Length = A1 -> Length;
    IRIT_GEN_COPY(A -> KV, A1 -> KV,
	     sizeof(CagdRType) * (A1 -> Length + A1 -> Order));

    CagdBlsmSetDomainAlphaCoef(A);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymb, FALSE) {
	    IRIT_INFO_MSG("Matrix A1:\n");
	    CagdDbgBlsmPrintAlphaMat(A1);
	    IRIT_INFO_MSG("Matrix A2:\n");
	    CagdDbgBlsmPrintAlphaMat(A2);
	    IRIT_INFO_MSG("Matrix A:\n");
	    CagdDbgBlsmPrintAlphaMat(A);
	}
    }
#   endif /* DEBUG */

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new degree raising matrix to degree raise once the following  M
* function space, defined using the Order, and knot vector KV of length	     M
* Len + Order (Len is the length of the control poly/mesh using KV).         M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:           Of space to degree raise.			             M
*   Order:        Of space to degree raise.			             M
*   Len:          Of control poly/mesh.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBlsmAlphaCoeffStruct *:    Degree raising matrix.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdSrfBlossomDegreeRaise, CagdCrvDegreeRaise M
*   CagdBlossomDegreeRaiseNMat, CagdDegreeRaiseMatProd			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlossomDegreeRaiseMat                                                M
*****************************************************************************/
CagdBlsmAlphaCoeffStruct *CagdBlossomDegreeRaiseMat(const CagdRType *KV,
						    int Order,
						    int Len)
{
    int j, l, NewLen;
    CagdRType t, *NewKV, *BlossomValues;
    CagdBlsmAlphaCoeffStruct *A;

    assert(Order > 0 && Len >= Order);

    if (BspKnotC0Discont(KV, Order, Len, &t))
        CAGD_FATAL_ERROR(CAGD_ERR_C0_KV_DETECTED);

    /* Allocate a vector for the new knot vector that will be sufficient and */
    /* populate it:							     */
    NewKV = BspKnotDegreeRaisedKV(KV, Len, Order, Order + 1, &NewLen);

    NewLen -= Order + 1; /* Get the number of control points we should have. */

    /* Create the blossom alpha matrix. */
    A = CagdBlsmAllocAlphaCoef(Order, Len, Order + 1, NewLen, FALSE);

    if (Order > 1) {
        BlossomValues = (CagdRType *) IritMalloc(sizeof(CagdRType) * Order);

	for (l = 0; l < NewLen; l++) {
	    int Index, Length;
	    CagdRType *R;

	    IRIT_GEN_COPY(BlossomValues, &NewKV[l + 2],
			  sizeof(CagdRType) * Order);

	    for (j = 0; j < Order; j++) {
	        R = CagdBlsmEvalSymb(Order, KV, Len + Order, BlossomValues,
				     Order - 1, &Index, &Length);

		CagdBlsmAddRowAlphaCoef(A, R, l, Index, Length);

		/* Only one value is changing in each step! */
		BlossomValues[j] = NewKV[l + 1 + j];
	    }
	}
	CagdBlsmScaleAlphaCoef(A, 1.0 / Order);

	IritFree(BlossomValues);
    }
    else {
        /* Special treatment of the constant degree case. */
        for (l = 0; l < NewLen; l++)
	    A -> Rows[l][BspKnotLastIndexLE(KV, Len + Order, NewKV[l])] = 1.0;
    }
    CagdBlsmSetDomainAlphaCoef(A);

    BspKnotCopy(A -> KV, KV, Len + Order);
    BspKnotCopy(A -> NewKV, NewKV, NewLen + Order + 1);

    IritFree(NewKV);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymb, FALSE) {
	    CagdDbgBlsmPrintAlphaMat(A);
	}
    }
#   endif /* DEBUG */

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a degree raising matrix to degree raise the following function  M
* space to order NewOrder.  The sapce is defined using the Order, and knot   M
*vector KV of length Len + Order (Len is the length of the control poly/mesh M
* using KV).							             M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:           Of space to degree raise.			             M
*   Order:        Of space to degree raise.			             M
*   NewOrder:     Destination order to raise to.		             M
*   Len:          Of control poly/mesh.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBlsmAlphaCoeffStruct *:    Degree raising matrix.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdSrfBlossomDegreeRaise, CagdCrvDegreeRaise M
*   CagdBlossomDegreeRaiseMat, CagdDegreeRaiseMatProd			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlossomDegreeRaiseNMat                                               M
*****************************************************************************/
CagdBlsmAlphaCoeffStruct *CagdBlossomDegreeRaiseNMat(const CagdRType *KV,
						     int Order,
						     int NewOrder,
						     int Len)
{
    CagdBlsmAlphaCoeffStruct *A2,
	*A1 = NULL,
	*A = NULL;

    while (Order < NewOrder) {
        if (A == NULL) {
	    A = A1 = CagdBlossomDegreeRaiseMat(KV, Order, Len);
	}
	else {
	    A2 = CagdBlossomDegreeRaiseMat(A1 -> NewKV, A1 -> NewOrder,
					   A1 -> NewLength);
	    A = CagdDegreeRaiseMatProd(A1, A2);
	    CagdBlsmFreeAlphaCoef(A1);
	    CagdBlsmFreeAlphaCoef(A2);
	    A1 = A;
	}

	Order++;
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymb, FALSE) {
	    CagdDbgBlsmPrintAlphaMat(A);
	}
    }
#   endif /* DEBUG */

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new curve with its degree raised to NewOrder, given curve     M
* Crv, using blossoming.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to degree raise.				             M
*   NewOrder:   New desired order of curve Crv.		    		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Degree raised curve, or NULL if error.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, BspCrvDegreeRaise, BzrCrvDegreeRaise	     M
*   CagdCrvDegreeRaise, CagdSrfBlossomDegreeRaiseN		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvBlossomDegreeRaiseN                                               M
*****************************************************************************/
CagdCrvStruct *CagdCrvBlossomDegreeRaiseN(const CagdCrvStruct *Crv,
					  int NewOrder)
{
    CagdBType
        IsBezier = FALSE;
    CagdPointType
	PType = Crv -> PType;
    int i, j, l, NewLen;
    CagdRType t;
    CagdCrvStruct *RCrv, *TCrv, *CpCrv;
    CagdBlsmAlphaCoeffStruct *A;

    if (CAGD_IS_BEZIER_CRV(Crv)) {           /* Convert to a Bspline curve. */
	IsBezier = TRUE;
	Crv = CpCrv = CagdCnvrtBzr2BspCrv(Crv);
    }
    else if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (!BspCrvHasOpenEC(Crv)) {
	TCrv = CagdCnvrtFloat2OpenCrv(Crv);
	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = TCrv;
    }

    if (BspKnotC0Discont(Crv -> KnotVector, Crv -> Order,
			 Crv -> Length, &t)) {
        CagdCrvStruct
	    *Crvs = CagdCrvSubdivAtParam(Crv, t),
	    *RCrv1 = CagdCrvBlossomDegreeRaiseN(Crvs, NewOrder),
	    *RCrv2 = CagdCrvBlossomDegreeRaiseN(Crvs -> Pnext, NewOrder);

	CagdCrvFreeList(Crvs);
	RCrv = CagdMergeCrvCrv(RCrv1, RCrv2, FALSE);
	CagdCrvFree(RCrv1);
	CagdCrvFree(RCrv2);
	return RCrv;	
    }

    /* We now have an open end Bspline curve to deal with - compute degree  */
    /* raising matrix for this setup.					    */
    A = CagdBlossomDegreeRaiseNMat(Crv -> KnotVector, Crv -> Order,
				   NewOrder, Crv -> Length);
    NewLen = A -> NewLength;

    /* Allocate space for the new raised curve. */
    RCrv = BspCrvNew(NewLen, NewOrder, PType);
    IRIT_GEN_COPY(RCrv -> KnotVector, A -> NewKV,
	     sizeof(CagdRType) * (NewLen + NewOrder));

    /* And apply the blossom alpha matrix to the control points. */
    for (i = !CAGD_IS_RATIONAL_PT(PType);
	 i <= CAGD_NUM_OF_PT_COORD(PType);
	 i++) {
	CagdRType
	    *Points = Crv -> Points[i],
	    *NewPoints = RCrv -> Points[i];

	for (l = 0; l < NewLen; l++, NewPoints++) {
	    CagdRType
	        *BlendVals = &A -> Rows[l][A -> ColIndex[l]],
		*p = &Points[A -> ColIndex[l]];

	    *NewPoints = 0.0;
	    for (j = 0; j < A -> ColLength[l]; j++)
	        *NewPoints += *p++ * *BlendVals++;
	}
    }

    CagdBlsmFreeAlphaCoef(A);

    if (IsBezier) {
	TCrv = CagdCnvrtBsp2BzrCrv(RCrv);
	CagdCrvFree(RCrv);
	RCrv = TCrv;
    }

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return RCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new curve with its degree raised once, given curve Crv, using M
* blossoming.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to degree raise.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Degree raised curve, or NULL if error.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaiseN, BspCrvDegreeRaise, BzrCrvDegreeRaise	     M
*   CagdCrvDegreeRaise, CagdSrfBlossomDegreeRaise                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvBlossomDegreeRaise                                                M
*****************************************************************************/
CagdCrvStruct *CagdCrvBlossomDegreeRaise(const CagdCrvStruct *Crv)
{
    return CagdCrvBlossomDegreeRaiseN(Crv, Crv -> Order + 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new surface with its degree raised to NewOrder, given surface M
* Srf, using blossoming.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to degree raise.				     M
*   NewUOrder:  New U order of Srf.					     M
*   NewVOrder:  New V order of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Degree raised surface, or NULL if error.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfBlossomDegreeRaise, BspSrfDegreeRaise, BzrSrfDegreeRaise	     M
*   CagdSrfDegreeRaise, CagdCrvBlossomDegreeRaiseN	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfBlossomDegreeRaiseN                                               M
*****************************************************************************/
CagdSrfStruct *CagdSrfBlossomDegreeRaiseN(const CagdSrfStruct *Srf,
					  int NewUOrder,
					  int NewVOrder)
{
    CagdBType
	IsBezier = FALSE;
    CagdPointType
	PType = Srf -> PType;
    int i, j, l, m, NewLen, StepSize;
    CagdSrfStruct *TSrf, *CpSrf,
        *RSrf = NULL;
    CagdBlsmAlphaCoeffStruct *A;

    if (CAGD_IS_BEZIER_SRF(Srf)) {          /* Convert to a Bspline surface. */
	IsBezier = TRUE;
	Srf = CpSrf = CagdCnvrtBzr2BspSrf(Srf);
    }
    else if (CAGD_IS_PERIODIC_SRF(Srf))
	Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    if (!BspSrfHasOpenEC(Srf)) {
	TSrf = CagdCnvrtFloat2OpenSrf(Srf);
	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);
	Srf = CpSrf = TSrf;
    }

    /* We now have an open end Bspline surface to deal with - compute degree */
    /* raising matrices for this setup and apply them - U then V.	     */

    if (NewUOrder > Srf -> UOrder) {
        A = CagdBlossomDegreeRaiseNMat(Srf -> UKnotVector, Srf -> UOrder,
				       NewUOrder, Srf -> ULength);
	NewLen = A -> NewLength;

	/* Allocate space for the new raised surface. */
	RSrf = BspSrfNew(NewLen, Srf -> VLength,
			 NewUOrder, Srf -> VOrder, PType);
	IRIT_GEN_COPY(RSrf -> UKnotVector, A -> NewKV,
		 sizeof(CagdRType) * (NewLen + RSrf -> UOrder));
	IRIT_GEN_COPY(RSrf -> VKnotVector, Srf -> VKnotVector,
		 sizeof(CagdRType) * (Srf -> VLength + Srf -> VOrder));

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    CagdRType
	        *Points = Srf -> Points[i],
	        *NewPoints = RSrf -> Points[i];

	    for (m = 0; m < Srf -> VLength; m++) {
	        for (l = 0; l < NewLen; l++, NewPoints++) {
		    CagdRType
		        *BlendVals = &A -> Rows[l][A -> ColIndex[l]],
		        *p = &Points[A -> ColIndex[l]];

		    *NewPoints = 0.0;
		    for (j = 0; j < A -> ColLength[l]; j++)
		        *NewPoints += *p++ * *BlendVals++;
		}
		Points += Srf -> ULength;
	    }
	}

	CagdBlsmFreeAlphaCoef(A);

	if (CpSrf) {
	    CagdSrfFree(CpSrf);
	    CpSrf = NULL;
	}
	Srf = RSrf;
    }

    if (NewVOrder > Srf -> VOrder) {
        A = CagdBlossomDegreeRaiseNMat(Srf -> VKnotVector, Srf -> VOrder,
				       NewVOrder, Srf -> VLength);
	NewLen = A -> NewLength;

	/* Allocate space for the new raised surface. */
	RSrf = BspSrfNew(Srf -> ULength, NewLen,
			 Srf -> UOrder, NewVOrder, PType);
	IRIT_GEN_COPY(RSrf -> UKnotVector, Srf -> UKnotVector,
		 sizeof(CagdRType) * (Srf -> ULength + Srf -> UOrder));
	IRIT_GEN_COPY(RSrf -> VKnotVector, A -> NewKV,
		 sizeof(CagdRType) * (NewLen + RSrf -> VOrder));
	StepSize = CAGD_NEXT_V(Srf);

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    CagdRType
	        *Points = Srf -> Points[i],
	        *NewPoints = RSrf -> Points[i];
		    
	    for (m = 0; m < Srf -> ULength; m++) {
	        CagdRType
		    *np = NewPoints++;

	        for (l = 0; l < NewLen; l++, np += StepSize) {
		    CagdRType
		        *BlendVals = &A -> Rows[l][A -> ColIndex[l]],
		        *p = &Points[A -> ColIndex[l] * StepSize];

		    *np = 0.0;
		    for (j = 0; j < A -> ColLength[l]; j++, p += StepSize)
		        *np += *p * *BlendVals++;
		}
		Points++;
	    }
	}

	CagdBlsmFreeAlphaCoef(A);

	if (CpSrf != NULL) {
	    CagdSrfFree(CpSrf);
	    CpSrf = NULL;
	}
    }

    if (CpSrf != NULL)
        CagdSrfFree(CpSrf);

    if (RSrf == NULL) {
        /* New orders are not higher than old orders. */
        return NULL;
    }

    if (IsBezier) {
	TSrf = CagdCnvrtBsp2BzrSrf(RSrf);
	CagdSrfFree(RSrf);
	RSrf = TSrf;
    }

    return RSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new surface with its degree raised once, given surface Srf,   M
* using blossoming.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to degree raise.			             M
*   Dir:       Direction of degree raising. Either U or V.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Degree raised surface, or NULL if error.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfBlossomDegreeRaiseN, BspSrfDegreeRaise, BzrSrfDegreeRaise	     M
*   CagdSrfDegreeRaise, CagdCrvBlossomDegreeRaise                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfBlossomDegreeRaise                                                M
*****************************************************************************/
CagdSrfStruct *CagdSrfBlossomDegreeRaise(const CagdSrfStruct *Srf,
					 CagdSrfDirType Dir)
{
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    return CagdSrfBlossomDegreeRaiseN(Srf,
					      Srf -> UOrder + 1,
					      Srf -> VOrder);
	    break;
        case CAGD_CONST_V_DIR:
	    return CagdSrfBlossomDegreeRaiseN(Srf,
					      Srf -> UOrder,
					      Srf -> VOrder + 1);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the CagdBlsmAlphaCoeffStruct data structrure.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Order, Length:  Current Order and Length of current function space.      M
*   NewOrder, NewLength:  New function space, after the blossom.	     M
*   Periodic:       TRUE, if periodic.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBlsmAlphaCoeffStruct *:    Allocated blossom Alpha matrix.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdBlossomEvalSymb, CagdBlsmAddRowAlphaCoef  M
*   CagdBlsmCopyAlphaCoef, CagdBlsmFreeAlphaCoef, CagdBlsmScaleAlphaCoef,    M
*   CagdBlsmSetDomainAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmAllocAlphaCoef, alpha matrix, blossom                            M
*****************************************************************************/
CagdBlsmAlphaCoeffStruct *CagdBlsmAllocAlphaCoef(int Order,
						 int Length,
						 int NewOrder,
						 int NewLength,
						 int Periodic)
{
    int i, j, Size;
    CagdRType **Rows;
    CagdBlsmAlphaCoeffStruct *NewA;

#   ifdef CAGD_BLSM_STRUCT_ALLOC_ONCE
	NewA = (CagdBlsmAlphaCoeffStruct *)
	    IritMalloc(Size = sizeof(CagdBlsmAlphaCoeffStruct)
		+ 8 + sizeof(CagdRType) * (Length + 1) * (NewLength + 1)/*Mat*/
		+ 8 + sizeof(CagdRType *) * (NewLength + 1)         /* Rows. */
		+ 8 + sizeof(int) * NewLength                   /* ColIndex. */
		+ 8 + sizeof(int) * NewLength               /* ColLengthgth. */
	        + 8 + sizeof(CagdRType) * (Length + Order + 1)  /* KV/NewKV. */
		+ 8 + sizeof(CagdRType) * (NewLength + NewOrder + 1));

	IRIT_ZAP_MEM(NewA, Size);

        /* Align it to 8 bytes. */
        NewA -> Matrix = (CagdRType *)
	                      ((((IritIntPtrSizeType) &NewA[1]) + 7) & ~0x07);
	Rows = NewA -> Rows = (CagdRType **)
		 &NewA -> Matrix[(Length + 1) * (NewLength + 1)];

        NewA -> ColIndex = (int *) &Rows[NewLength + 1];
	NewA -> ColLength = (int *) &NewA -> ColIndex[NewLength + 1];

        NewA -> KV = (CagdRType *)
	      ((((IritIntPtrSizeType) &NewA -> ColLength[NewLength + 1]) + 7)
								     & ~0x07);
	NewA -> NewKV = &NewA -> KV[Length + Order];
#   else
	NewA = (CagdBlsmAlphaCoeffStruct *)
	    IritMalloc(sizeof(CagdBlsmAlphaCoeffStruct));
	NewA -> Matrix = (CagdRType *)
	    IritMalloc(Size = sizeof(CagdRType) * (Length + 1)
						* (NewLength + 1));
	IRIT_ZAP_MEM(NewA -> Matrix, Size);
	Rows = NewA -> Rows = (CagdRType **)
	    IritMalloc(sizeof(CagdRType *) * (NewLength + 1));

	NewA -> ColIndex = (int *) IritMalloc(Size = sizeof(int) * NewLength);
	NewA -> ColLength = (int *) IritMalloc(sizeof(int) * NewLength);
	IRIT_ZAP_MEM(NewA -> ColIndex, Size);
	IRIT_ZAP_MEM(NewA -> ColLength, Size);

        NewA -> KV = (CagdRType *)
	    IritMalloc(Size = sizeof(CagdRType) * (Length + Order));
	IRIT_ZAP_MEM(NewA -> Matrix, Size);
	NewA -> NewKV = (CagdRType *)
	    IritMalloc(Size = sizeof(CagdRType) * (NewLength + Order));
	IRIT_ZAP_MEM(NewA -> Matrix, Size);
#   endif /* CAGD_BLSM_STRUCT_ALLOC_ONCE */
    NewA -> Order = Order;
    NewA -> Length = Length;
    NewA -> NewOrder = NewOrder;
    NewA -> NewLength = NewLength;
    NewA -> Periodic = Periodic;

    /* Update the row pointers to point onto the matrix rows. */
    for (i = 0, j = 0; i <= NewLength; i++, j += Length)
	Rows[i] = &NewA -> Matrix[j];

    return NewA;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copies the CagdBlsmAlphaCoeffStruct data structrure.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   A:      Blossom alpha matrix to copy.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBlsmAlphaCoeffStruct *:  Copied matrix.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdBlossomEvalSymb, CagdBlsmAddRowAlphaCoef  M
*   CagdBlsmAllocAlphaCoef, CagdBlsmFreeAlphaCoef, CagdBlsmScaleAlphaCoef,   M
*   CagdBlsmSetDomainAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmCopyAlphaCoef, alpha matrix, refinement                          M
*****************************************************************************/
CagdBlsmAlphaCoeffStruct *CagdBlsmCopyAlphaCoef(const CagdBlsmAlphaCoeffStruct
						                           *A)
{
    int i, j,
	Len = A -> Length,
	NewLen = A  -> NewLength;
    CagdRType **Rows;
    CagdBlsmAlphaCoeffStruct *NewA;

#   ifdef CAGD_BLSM_STRUCT_ALLOC_ONCE
	NewA = (CagdBlsmAlphaCoeffStruct *)
	    IritMalloc(sizeof(CagdBlsmAlphaCoeffStruct)
		+ 8 + sizeof(CagdRType) * (Len + 1) * (NewLen + 1)   /* Mat. */
		+ 8 + sizeof(CagdRType *) * (NewLen + 1)            /* Rows. */
		+ 8 + sizeof(int) * NewLen                      /* ColIndex. */
		+ 8 + sizeof(int) * NewLen                     /* ColLength. */
	        + 8 + sizeof(CagdRType) * (Len + A -> Order + 1)/* KV/NewKV. */
		+ 8 + sizeof(CagdRType) * (NewLen + A -> NewOrder + 1));

        /* Align it to 8 bytes. */
        NewA -> Matrix = (CagdRType *)
	                      ((((IritIntPtrSizeType) &NewA[1]) + 7) & ~0x07);
	Rows = NewA -> Rows = (CagdRType **)
		            &NewA -> Matrix[(Len + 1) * (NewLen + 1)];

        NewA -> ColIndex = (int *) &Rows[NewLen + 1];
	NewA -> ColLength = (int *) &NewA -> ColIndex[NewLen + 1];

        NewA -> KV = (CagdRType *) 
	      ((((IritIntPtrSizeType) &NewA -> ColLength[NewLen + 1]) + 7)
								     & ~0x07);
	NewA -> NewKV = &NewA -> KV[Len + A -> Order];
#   else
	NewA = (CagdBlsmAlphaCoeffStruct *)
	    IritMalloc(sizeof(CagdBlsmAlphaCoeffStruct));
	NewA -> Matrix = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * (Len + 1) * (NewLen + 1));
	Rows = NewA -> Rows = (CagdRType **) IritMalloc(sizeof(CagdRType *) *
							        (NewLen + 1));
	NewA -> ColIndex = (int *) IritMalloc(sizeof(int) * NewLen);
	NewA -> ColLength = (int *) IritMalloc(sizeof(int) * NewLen);

        NewA -> KV = (CagdRType *)
		IritMalloc(sizeof(CagdRType) * (Len + A -> Order));
	NewA -> NewKV = (CagdRType *)
		IritMalloc(sizeof(CagdRType) * (NewLen + A -> NewOrder));
#   endif /* CAGD_BLSM_STRUCT_ALLOC_ONCE */
    NewA -> Order = A -> Order;
    NewA -> Length = Len;
    NewA -> NewOrder = A -> NewOrder;
    NewA -> NewLength = NewLen;
    NewA -> Periodic = A -> Periodic;

    IRIT_GEN_COPY(NewA -> Matrix, A -> Matrix,
	     sizeof(CagdRType) * (Len + 1) * (NewLen + 1));

    /* Update the knot sequences. */
    IRIT_GEN_COPY(NewA -> KV, A -> KV, sizeof(CagdRType) * (Len + A -> Order));
    IRIT_GEN_COPY(NewA -> NewKV, A -> NewKV,
	     sizeof(CagdRType) * (NewLen + A -> NewOrder));

    /* Update the row pointers to point onto the matrix rows. */
    for (i = 0, j = 0; i <= NewLen; i++, j += Len)
	Rows[i] = &NewA -> Matrix[j];

    IRIT_GEN_COPY(NewA -> ColIndex, A -> ColIndex, sizeof(int) * NewLen);
    IRIT_GEN_COPY(NewA -> ColLength, A -> ColLength, sizeof(int) * NewLen);

    return NewA;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees the CagdBlsmAlphaCoeffStruct data structrure.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   A:      Blossom Alpha matrix to free.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomDegreeRaise, CagdBlossomEvalSymb, CagdBlsmAddRowAlphaCoef  M
*   CagdBlsmAllocAlphaCoef, CagdBlsmCopyAlphaCoef, CagdBlsmScaleAlphaCoef,   M
*   CagdBlsmSetDomainAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmFreeAlphaCoef, alpha matrix, blossom                             M
*****************************************************************************/
void CagdBlsmFreeAlphaCoef(CagdBlsmAlphaCoeffStruct *A)
{
#ifndef CAGD_BLSM_STRUCT_ALLOC_ONCE
    IritFree(A -> Matrix);
    IritFree(A -> Rows);
    IritFree(A -> ColIndex);
    IritFree(A -> ColLength);
    IritFree(A -> KV);
    IritFree(A -> NewKV);
#endif /* CAGD_BLSM_STRUCT_ALLOC_ONCE */
    IritFree(A);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates one row, ARow, in the blossom alpha matrix.			     M
*   New coefficients are being added to the current values from ColIndex to  M
* ColIndex+ColLength-1.							     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   A:         The current blossome alpha matrix to update.                  M
*   Coefs:     The coefficients to add to the alpha matrix in row ARow.      M
*   ARow:      The row in A to update.					     M
*   ColIndex:  Starting index in column Col to update.			     M
*   ColLength: Number of coefficients to update in column Col.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBlsmEvalSymb, CagdBlsmAllocAlphaCoef, CagdBlsmCopyAlphaCoef,	     M
*   CagdBlsmFreeAlphaCoef, CagdBlsmScaleAlphaCoef,			     M
*   CagdBlsmSetDomainAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmAddRowAlphaCoef                                                  M
*****************************************************************************/
void CagdBlsmAddRowAlphaCoef(CagdBlsmAlphaCoeffStruct *A,
			     CagdRType *Coefs,
			     int ARow,
			     int ColIndex,
			     int ColLength)
{
    CagdRType
	*ColPtr = &A -> Rows[ARow][ColIndex];

    assert(ARow >= 0 &&
	   ARow < A -> NewLength  &&
	   ColIndex >= 0 &&
	   ColLength >= 1 &&
	   ColIndex + ColLength <= A -> Length);

    Coefs = &Coefs[ColIndex];
    while (ColLength--)
        *ColPtr++ += *Coefs++;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBlsmSymb, FALSE) {
	    CagdDbgBlsmPrintAlphaMat(A);
	}
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scale all the coefficients in the given blossom alpha matrix by Scl.     M
*                                                                            *
* PARAMETERS:                                                                M
*   A:     Blossom alpha matrix to scale all it coefficients.                M
*   Scl:   Scaling factor.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBlsmEvalSymb, CagdBlsmAllocAlphaCoef, CagdBlsmCopyAlphaCoef,	     M
*   CagdBlsmFreeAlphaCoef, CagdBlsmSetDomainAlphaCoef,			     M
*   CagdBlsmAddRowAlphaCoef						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmScaleAlphaCoef                                                   M
*****************************************************************************/
void CagdBlsmScaleAlphaCoef(CagdBlsmAlphaCoeffStruct *A, CagdRType Scl)
{
    int i, j;

    for (j = 0; j < A -> NewLength; j++)
	for (i = 0; i < A -> Length; i++)
	    A -> Rows[j][i] *= Scl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update domain bounds ColIndex/ColLength in the blossom alpha matrix A.   M
*                                                                            *
* PARAMETERS:                                                                M
*   A:   Blossom alpha matrix to update its ColIndex/ColLength settings,     M
*	 in place.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBlsmEvalSymb, CagdBlsmAllocAlphaCoef, CagdBlsmCopyAlphaCoef,	     M
*   CagdBlsmFreeAlphaCoef, CagdBlsmScaleAlphaCoef, CagdBlsmAddRowAlphaCoef   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBlsmSetDomainAlphaCoef                                               M
*****************************************************************************/
void CagdBlsmSetDomainAlphaCoef(CagdBlsmAlphaCoeffStruct *A)
{
    int i, j;

    for (j = 0; j < A -> NewLength; j++) {
        for (i = 0; i < A -> Length; i++) {
	    if (A -> Rows[j][i] != 0.0)
	        break;
	}
	A -> ColIndex[j] = i;

        for (i = A -> Length - 1; i >= 0; i--) {
	    if (A -> Rows[j][i] != 0.0)
	        break;
	}
	A -> ColLength[j] = i - A -> ColIndex[j] + 1;
    }
}

#ifdef DEBUG
/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the content of the blossom alpha matrix.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   A:        Alpha matrix to print.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void CagdDbgBlsmPrintAlphaMat(CagdBlsmAlphaCoeffStruct *A)
{
    int i, j;

    printf("Order = %d, Length = %d, NewOrder = %d, NewLength = %d\n",
	   A -> Order, A -> Length, A -> NewOrder, A -> NewLength);

    printf("A matrix:\n");
    for (j = 0; j < A -> NewLength; j++) {
        printf("%3d) [%2d, %2d] ",
	       j, A -> ColIndex[j], A -> ColLength[j]);
	for (i = 0; i < A -> Length; i++)
	    printf(" %7.4lg", A -> Rows[j][i]);
	printf("\n");
    }

    printf("KVT:\n");
    for (i = 0; i < A -> Length + A -> Order; i++)
        printf(" %7.4lg ", A -> KV[i]);

    printf("\nKVt:\n");
    for (i = 0; i < A -> NewLength + A -> Order + 1; i++)
        printf(" %7.4lg ", A -> NewKV[i]);
    printf( "\n");

    fflush(stdout);
}
#endif /* DEBUG */
