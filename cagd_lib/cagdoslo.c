/******************************************************************************
* CagdOslo.c - an implementation of the oslo algorithm (1).		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 92.					      *
******************************************************************************/

#include "cagd_loc.h"

#define CAGD_OSLO_STRUCT_ALLOC_ONCE	1
#define CAGD_OSLO_OPTIMIZED_ZEROS	TRUE

#define OSLO_IRIT_EPS		1e-20
#define OSLO_APX_ZERO(x)	(IRIT_FABS(x) < OSLO_IRIT_EPS)

#ifdef DEBUG
void CagdDbgPrintAlphaMat(BspKnotAlphaCoeffStruct *A);
#endif /* DEBUG */

IRIT_STATIC_DATA BspKnotAlphaCoeffStruct
    *GlblCachedA = NULL;

static void BspKnotAlphaCoefFree(BspKnotAlphaCoeffStruct *A);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the values of the alpha coefficients, Ai,k(j) of order k:	     M
*                                                                            M
*         n              n    m                   m    n		     V
*         _              _    _                   _    _		     V
*  C(t) = \ Pi Bi,k(t) = \ Pi \ Ai,k(j) Nj,k(t) = \  ( \ Pi Ai,k(j) Nj,k(t)  V
*         /              /    /                   /    /		     V
*         -              -    -                   -    -		     V
*        i=0            i=0  j=0                 j=0  i=0		     V
*                                                                            M
* Let T be the original knot vector and let t be the refined one, i.e. T is  M
* a subset of t. 							     M
*   The Ai,k(j) are computed from the following recursive definition:        M
*                                                                            M
*             1, T(i) <= t(i) < T(i+1)                                       V
*           /                                                                V
* Ai,1(j) =                                                                  V
*           \                                                                V
*             0, otherwise.                                                  V
*                                                                            V
*                                                                            V
*           T(j+k-1) - T(i)             T(i+k) - T(j+k-1)                    V
* Ai,k(j) = --------------- Ai,k-1(j) + --------------- Ai+1,k-1(j)          V
*           T(i+k-1) - T(i)             T(i+k) - T(i+1)                      V
*                                                                            M
* LengthKVT + k is the length of KVT and similarly LengthKVt + k is the      M
* length of KVt. In other words, LengthKVT and LengthKVt are the control     M
* points len...								     M
*                                                                            M
* The output matrix has LengthKVT rows and LengthKVt columns (#cols > #rows) M
* ColIndex/Length hold LengthKVt pairs of first non zero scalar and length ofM
* non zero values in that column, so not all LengthKVT scalars are blended.  M
*                                                                            *
* PARAMETERS:                                                                M
*   k:           Order of geometry.                                          M
*   KVT:         Original knot vector.                                       M
*   LengthKVT:   Length of original control polygon with KVT knot vector.    M
*   KVt:         Refined knot vector. Must contain all knots of KVT.         M
*   LengthKVt:   Length of refined control polygon with KVt knot vector.     M
*   Periodic:    If the refinement is for a periodic entity.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   BspKnotAlphaCoeffStruct *: A matrix to multiply the coefficients of the  M
*                              geometry using KVT, in order to get the       M
*                              coefficients under the space defined using    M
*                              KVt that represent the same geometry.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotFreeAlphaCoef, BspKnotEvalAlphaCoefMerge, BspCrvKnotInsert,       M
*   BspSrfKnotInsert, BspKnotAlphaLoopBlendPeriodic,			     M
*   BspKnotAlphaLoopBlendNotPeriodic, BspKnotAlphaLoopBlendStep		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotEvalAlphaCoef, alpha matrix, refinement                           M
*****************************************************************************/
BspKnotAlphaCoeffStruct *BspKnotEvalAlphaCoef(int k,
					      CagdRType *KVT,
					      int LengthKVT,
					      CagdRType *KVt,
					      int LengthKVt,
					      int Periodic)
{
    IRIT_STATIC_DATA int
	NoZeroKVtLen = 0,
	*NoZeroKVtMin = NULL,
	*NoZeroKVtMax = NULL;
    int Size, i, j, o, NextStart, *ColLen, *ColIdx;
    CagdRType *m, **r, **Rows, **RowsTransp;
    BspKnotAlphaCoeffStruct *A;

    /* Make sure cache of zeros in blending (if CAGD_OSLO_OPTIMIZED_ZEROS)  */
    /* of the Alpha matrix coefficients is of valid length.		    */
    if (NoZeroKVtLen < LengthKVT + 2) {
	if (NoZeroKVtMin != NULL) {
	    IritFree(NoZeroKVtMin);
	    IritFree(NoZeroKVtMax);
	}

	NoZeroKVtLen = LengthKVT * 2 + 2;

	NoZeroKVtMin = (int *) IritMalloc(sizeof(int) * NoZeroKVtLen);
	NoZeroKVtMax = (int *) IritMalloc(sizeof(int) * NoZeroKVtLen);
    }

    /* Verify the robustness (monotonicity) of the knot sequences. */
#ifdef DEBUG
    if (BspKnotMakeRobustKV(KVT, LengthKVT + k) ||
	BspKnotMakeRobustKV(KVt, LengthKVt + k)) {
        CAGD_FATAL_ERROR(CAGD_ERR_KNOT_NOT_ORDERED);
    }
#endif /* DEBUG */

    Size = (LengthKVT + 1) * (LengthKVt + 1);

    if (GlblCachedA != NULL &&
	GlblCachedA -> RefLength == LengthKVt &&
	GlblCachedA -> Length == LengthKVT &&
	GlblCachedA -> Order == k &&
	GlblCachedA -> Periodic == Periodic) {
	A = GlblCachedA;
	GlblCachedA = NULL;

        if (IRIT_GEN_CMP(KVt, A -> _CacheKVt,
		    sizeof(CagdRType) * (LengthKVt + k)) == 0 &&
	    IRIT_GEN_CMP(KVT, A -> _CacheKVT,
		    sizeof(CagdRType) * (LengthKVT + k)) == 0) {
	    /* We found exact match - return cached matrix. */
	    return A;
	}
	else {
	    Rows = A -> Rows;
	    RowsTransp = A -> RowsTransp;
	    ColIdx = A -> ColIndex;
	    ColLen = A -> ColLength;

	    IRIT_ZAP_MEM(A -> Matrix, sizeof(CagdRType) * Size); 
	    IRIT_ZAP_MEM(A -> MatrixTransp, sizeof(CagdRType) * Size); 
	}
    }
    else {
#ifdef CAGD_OSLO_STRUCT_ALLOC_ONCE
	A = (BspKnotAlphaCoeffStruct *)
	    IritMalloc(sizeof(BspKnotAlphaCoeffStruct)
		+ 8 + sizeof(CagdRType) * Size		           /* Matrix */
		+ 8 + sizeof(CagdRType) * Size		     /* MatrixTransp */
		+ 8 + sizeof(CagdRType *) * (LengthKVT + 1)          /* Rows */
		+ 8 + sizeof(CagdRType *) * (LengthKVt + 1)    /* RowsTransp */
		+ 8 + sizeof(int) * LengthKVt			 /* ColIndex */
		+ 8 + sizeof(int) * LengthKVt		        /* ColLength */
		+ 8 + sizeof(CagdRType) * (LengthKVT + k + 1)    /* CacheKVT */
		+ 8 + sizeof(CagdRType) * (LengthKVt + k + 1));  /* CacheKVt */

        /* Align it to 8 bytes. */
        A -> Matrix = (CagdRType *)
	                        ((((IritIntPtrSizeType) &A[1]) + 7) & ~0x07);
        A -> MatrixTransp = &A -> Matrix[Size];
	Rows = A -> Rows = (CagdRType **) &A -> MatrixTransp[Size];
	RowsTransp = A -> RowsTransp = (CagdRType **) &Rows[LengthKVT + 1];

        ColIdx = A -> ColIndex = (int *) &RowsTransp[LengthKVt + 1];
	ColLen = A -> ColLength = (int *) &A -> ColIndex[LengthKVt + 1];

        A -> _CacheKVT = (CagdRType *) &A -> ColLength[LengthKVt + 1];
	A -> _CacheKVt = &A -> _CacheKVT[LengthKVT + k];

	IRIT_ZAP_MEM(A -> Matrix,
		sizeof(CagdRType) * Size * 2);  /* Clears also MatrixTransp. */
#else
	A = (BspKnotAlphaCoeffStruct *)
	    IritMalloc(sizeof(BspKnotAlphaCoeffStruct));
	A -> Matrix = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size);
	A -> MatrixTransp = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size);
	Rows = A -> Rows = (CagdRType **)
	    IritMalloc(sizeof(CagdRType *) * (LengthKVT + 1));
	RowsTransp = A -> RowsTransp = (CagdRType **)
	    IritMalloc(sizeof(CagdRType *) * (LengthKVt + 1));
	ColIdx = A -> ColIndex = (int *) IritMalloc(sizeof(int) * LengthKVt);
	ColLen = A -> ColLength = (int *) IritMalloc(sizeof(int) * LengthKVt);

        A -> _CacheKVT = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * (LengthKVT + k));
	A -> _CacheKVt = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * (LengthKVt + k));

	IRIT_ZAP_MEM(A -> Matrix, sizeof(CagdRType) * Size); 
	IRIT_ZAP_MEM(A -> MatrixTransp, sizeof(CagdRType) * Size); 
#endif /* CAGD_OSLO_STRUCT_ALLOC_ONCE */
	A -> Order = k;
	A -> Length = LengthKVT;
	A -> RefLength = LengthKVt;
	A -> Periodic = Periodic;

	/* Update the row pointers to point onto the matrix rows. */
	for (i = 0, r = Rows, m = A -> Matrix;
	     i++ <= LengthKVT;
	     m += LengthKVt)
	    *r++ = m;
	for (i = 0, r = RowsTransp, m = A -> MatrixTransp;
	     i++ <= LengthKVt;
	     m += LengthKVT)
	    *r++ = m;
    }

    /* Update the cache. */
    IRIT_GEN_COPY(A -> _CacheKVT, KVT, sizeof(CagdRType) * (LengthKVT + k));
    IRIT_GEN_COPY(A -> _CacheKVt, KVt, sizeof(CagdRType) * (LengthKVt + k));

    /* Initialize the matrix with according to order 1: */
    for (i = NextStart = 0; i < LengthKVT; i++) {
	int Last = -1,
	    First = -1;
	CagdRType
	    *RowI = &Rows[i][NextStart],
	    *KVtp = &KVt[NextStart],
	    KVTI = KVT[i],
	    KVTI1 = KVT[i + 1];

	for (j = NextStart - 1; ++j < LengthKVt; RowI++, KVtp++) {
	    if (*KVtp >= KVTI1) {
	        if (First >= 0)
		    Last = j - 1;
	        break;
	    }
	    else if (KVTI <= *KVtp) {
		*RowI = 1.0;
		if (First < 0) {
		    First = j;
		    if (NextStart < First)
		        NextStart = First;
		}
	    }
	}
	if (j >= LengthKVt)
	    Last = LengthKVt - 1; 

	NoZeroKVtMin[i] = First < 0 ? LengthKVt + 2 : First;
	NoZeroKVtMax[i] = Last;
    }
    NoZeroKVtMin[LengthKVT] = LengthKVt + 2;
    NoZeroKVtMax[LengthKVT] = -1;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugOsloSteps, FALSE) {
	    IRIT_INFO_MSG("INITIALIZATION\n        ");
	    for (i = 0; i <= A -> Length; i++)
	        IRIT_INFO_MSG_PRINTF(" %7d", NoZeroKVtMin[i]);
	    IRIT_INFO_MSG("\n        ");
	    for (i = 0; i <= A -> Length; i++)
	        IRIT_INFO_MSG_PRINTF(" %7d", NoZeroKVtMax[i]);
	    IRIT_INFO_MSG_PRINTF("\n");

	    for (j = 0; j < A -> RefLength; j++) {
	        IRIT_INFO_MSG_PRINTF("%3d)    ", j);
		for (i = 0; i < A -> Length; i++)
		    IRIT_INFO_MSG_PRINTF(" %7.4lg", A -> Rows[i][j]);
		IRIT_INFO_MSG_PRINTF("\n");
	    }
	}
    }
#   endif /* DEBUG */

    /* Iterate upto order k: */
    for (o = 2; o <= k; o++) {
#	ifdef CAGD_OSLO_OPTIMIZED_ZEROS
	for (i = 0; i < LengthKVT; i++) {
	    int JMin = NoZeroKVtMin[i] = IRIT_MIN(NoZeroKVtMin[i],
					     NoZeroKVtMin[i + 1]),
		JMax = NoZeroKVtMax[i] = IRIT_MAX(NoZeroKVtMax[i],
					     NoZeroKVtMax[i + 1]);

	    if (JMax >= JMin) {
		CagdRType
		    *RowI = &Rows[i][JMin],
		    *RowI1 = &Rows[i + 1][JMin],
		    KVTI = KVT[i],
		    KVTIO = KVT[i + o],
		    *KVtp = &KVt[JMin + o - 1],
		    t1 = KVT[i + o - 1] - KVTI,
		    t2 = KVTIO - KVT[i + 1];

		/* If ti == 0, the whole term should be ignored. */
		t1 = t1 < OSLO_IRIT_EPS ? 0.0 : 1.0 / t1;
		t2 = t2 < OSLO_IRIT_EPS ? 0.0 : 1.0 / t2;

		for (j = JMin; j <= JMax; j++, RowI++, RowI1++, KVtp++) {
		    *RowI = *RowI * (*KVtp - KVTI) * t1 +
			    *RowI1 * (KVTIO - *KVtp) * t2;
		}
	    }
	}
#	else
	for (i = 0; i < LengthKVT; i++) {
	    CagdRType
		*RowI = Rows[i],
		*RowI1 = Rows[i + 1],
		KVTI = KVT[i],
		KVTIO = KVT[i + o],
		*KVtp = &KVt[o - 1],
		t1 = KVT[i + o - 1] - KVTI,
		t2 = KVTIO - KVT[i + 1];

	    /* If ti == 0, the whole term should be ignored. */
	    t1 = t1 < OSLO_IRIT_EPS ? 0.0 : 1.0 / t1;
	    t2 = t2 < OSLO_IRIT_EPS ? 0.0 : 1.0 / t2;

	    for (j = 0; j < LengthKVt - 1; j++, RowI++, RowI1++, KVtp++) {
		*RowI = *RowI * (*KVtp - KVTI) * t1 +
			*RowI1 * (KVTIO - *KVtp) * t2;
	    }
	}
#	endif /* CAGD_OSLO_OPTIMIZED_ZEROS */


#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugOsloSteps, FALSE) {
	        IRIT_INFO_MSG_PRINTF("STEP %d\n        ", o);
		for (i = 0; i <= A -> Length; i++)
		    IRIT_INFO_MSG_PRINTF(" %7d", NoZeroKVtMin[i]);
		IRIT_INFO_MSG("\n        ");
		for (i = 0; i <= A -> Length; i++)
		    IRIT_INFO_MSG_PRINTF(" %7d", NoZeroKVtMax[i]);
		IRIT_INFO_MSG("\n");
		for (j = 0; j < A -> RefLength; j++) {
		    IRIT_INFO_MSG_PRINTF("%3d)    ", j);
		    for (i = 0; i < A -> Length; i++)
		        IRIT_INFO_MSG_PRINTF(" %7.4lg", A -> Rows[i][j]);
		    IRIT_INFO_MSG("\n");
		}
	    }
	}
#	endif /* DEBUG */
    }

    if (Periodic) {
	int Len = LengthKVT - (k - 1);

	/* Update the row non zero indices. */
	for (i = LengthKVt - 1; i >= 0; i--) {
	    for (j = 0; j < LengthKVT && OSLO_APX_ZERO(Rows[j][i]); j++);
	    ColIdx[i] = j;
	    for (j = LengthKVT - 1; j >= 0 && OSLO_APX_ZERO(Rows[j][i]); j--);
	    if ((ColLen[i] = j + 1 - ColIdx[i]) <= 0)
	        CAGD_FATAL_ERROR(CAGD_ERR_DEGEN_ALPHA);
	}

	/* Move the refinement area to the beginning if in the end. */
	for (i = LengthKVt - 2 * k + 1; i < LengthKVt; i++) {
	    if (ColIdx[i] + ColLen[i] > Len) {
		int l, m;

		/* We have entities refined in the end. */
		for (j = Len; j < LengthKVT; j++) {
		    if (!IRIT_APX_EQ(Rows[j][i], 0.0))
		        IRIT_SWAP(CagdRType, Rows[j][i], Rows[j - Len][i]);
		}

		/* Update the index and length of this column. */
		for (l = 0;
		     l < LengthKVT && OSLO_APX_ZERO(Rows[l][i]);
		     l++);
		for (m = LengthKVT - 1;
		     m >= 0 && OSLO_APX_ZERO(Rows[m][i]);
		     m--);
		ColLen[i] = m - l + 1;
		ColIdx[i] = l;

	    }
	}

	for (i = LengthKVt - k + 1; i < LengthKVt; i++) {
	    int ii = i - (LengthKVt - k + 1);

	    if (ColLen[i] > 1) {
		for (j = ColIdx[i]; j <= ColIdx[i] + ColLen[i] - 1; j++) {
		    IRIT_SWAP(CagdRType, Rows[j][i], Rows[j][ii]);
		}
		IRIT_SWAP(int, ColLen[i], ColLen[ii]);
		IRIT_SWAP(int, ColIdx[i], ColIdx[ii]);
	    }
	}

	/* Update the transposed matrix and ColIdx/Len arrays. */
	for (i = LengthKVt; --i >= 0; ) {
	    int First = -1,
	        Last = -1;

	    for (j = LengthKVT; --j >= 0; ) {
	        if ((RowsTransp[i][j] = Rows[j][i]) > OSLO_IRIT_EPS) {
		    First = Last < 0 ? Last = j : j;
		}
	    }
	    assert(Last >= 0);
	    ColIdx[i] = First;
	    ColLen[i] = Last - First + 1;
	}
    }
    else {          /* Not periodic - all non zeros are one connected group. */
        /* Update the transposed matrix and ColIdx/Len arrays. */
        NextStart = LengthKVT - 1;
        for (i = LengthKVt; --i >= 0; ) {
	    int First = 0,
	        Last = -1;

	    for (j = NextStart + 1; --j >= 0; ) {
	        if ((RowsTransp[i][j] = Rows[j][i]) > OSLO_IRIT_EPS) {
		    First = Last < 0 ? Last = j : j;
		}
		else if (Last >= 0) 
		    break;
	    }
	    ColIdx[i] = First;
	    ColLen[i] = Last - First + 1;
	    if (NextStart > Last && Last > -1)
		NextStart = Last;
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugOsloSteps2, FALSE) {
	    CagdDbgPrintAlphaMat(A);
	}
    }
#   endif /* DEBUG */

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copies the BspKnotAlphaCoeffStruct data structrure.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   A:      Alpha matrix to copy.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   BspKnotAlphaCoeffStruct *:  Copied matrix.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotEvalAlphaCoef, BspKnotEvalAlphaCoefMerge, BspKnotFreeAlphaCoef,   M
*   BspCrvKnotInsert, BspSrfKnotInsert	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotCopyAlphaCoef, alpha matrix, refinement                           M
*****************************************************************************/
BspKnotAlphaCoeffStruct *BspKnotCopyAlphaCoef(const BspKnotAlphaCoeffStruct *A)
{
    int i, j;
    CagdRType **Rows, **RowsTransp;
    BspKnotAlphaCoeffStruct *NewA;

#   ifdef CAGD_OSLO_STRUCT_ALLOC_ONCE
	NewA = (BspKnotAlphaCoeffStruct *)
	    IritMalloc(sizeof(BspKnotAlphaCoeffStruct) +
		+ 8 + sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength
		+ 8 + sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength
		+ 8 + sizeof(CagdRType *) * (A -> Length + 1)
		+ 8 + sizeof(CagdRType *) * (A -> RefLength + 1)
		+ 8 + sizeof(int) * A -> RefLength
		+ 8 + sizeof(int) * A -> RefLength
		+ 8 + sizeof(CagdRType) * (A -> Length + A -> Order + 1)
		+ 8 + sizeof(CagdRType) * (A -> RefLength + A -> Order + 1));

        /* Align it to 8 bytes. */
        NewA -> Matrix = (CagdRType *)
	                      ((((IritIntPtrSizeType) &NewA[1]) + 7) & ~0x07);
        NewA -> MatrixTransp =
		 &NewA -> Matrix[(A -> Length + 1) * A -> RefLength];
	Rows = NewA -> Rows = (CagdRType **)
		 &NewA -> MatrixTransp[(A -> Length + 1) * A -> RefLength];
	RowsTransp = NewA -> RowsTransp = (CagdRType **) &Rows[A -> Length + 1];

        NewA -> ColIndex = (int *) &RowsTransp[A -> RefLength + 1];
	NewA -> ColLength = (int *) &NewA -> ColIndex[A -> RefLength + 1];

        NewA -> _CacheKVT = (CagdRType *) &NewA -> ColLength[A -> RefLength + 1];
	NewA -> _CacheKVt = &NewA -> _CacheKVT[A -> Length + A -> Order];

#   else
	NewA = (BspKnotAlphaCoeffStruct *)
	    IritMalloc(sizeof(BspKnotAlphaCoeffStruct));
	NewA -> Matrix = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength);
	NewA -> MatrixTransp = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength);
	Rows = NewA -> Rows = (CagdRType **) IritMalloc(sizeof(CagdRType *) *
							   (A -> Length + 1));
	RowsTransp = NewA -> RowsTransp = (CagdRType **)
	    IritMalloc(sizeof(CagdRType *) * (A -> RefLength + 1));
	NewA -> ColIndex = (int *) IritMalloc(sizeof(int) * A -> RefLength);
	NewA -> ColLength = (int *) IritMalloc(sizeof(int) * A -> RefLength);

        NewA -> _CacheKVT = (CagdRType *)
		IritMalloc(sizeof(CagdRType) * (A -> Length + A -> Order));
	NewA -> _CacheKVt = (CagdRType *)
		IritMalloc(sizeof(CagdRType) * (A -> RefLength + A -> Order));
#   endif /* CAGD_OSLO_STRUCT_ALLOC_ONCE */
    NewA -> Order = A -> Order;
    NewA -> Length = A -> Length;
    NewA -> RefLength = A -> RefLength;
    NewA -> Periodic = A -> Periodic;

    IRIT_GEN_COPY(NewA -> Matrix, A -> Matrix,
	     sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength);
    IRIT_GEN_COPY(NewA -> MatrixTransp, A -> MatrixTransp,
	     sizeof(CagdRType) * (A -> Length + 1) * A -> RefLength);

    /* Update the cache. */
    IRIT_GEN_COPY(NewA -> _CacheKVT, A -> _CacheKVT,
	     sizeof(CagdRType) * (A -> Length + A -> Order));
    IRIT_GEN_COPY(NewA -> _CacheKVt, A -> _CacheKVt,
	     sizeof(CagdRType) * (A -> RefLength + A -> Order));

    /* Update the row pointers to point onto the matrix rows. */
    for (i = 0, j = 0; i <= A -> Length; i++, j += A -> RefLength)
	Rows[i] = &A -> Matrix[j];
    for (i = 0, j = 0; i <= A -> RefLength; i++, j += A -> Length)
        RowsTransp[i] = &A -> MatrixTransp[j];

    IRIT_GEN_COPY(NewA -> ColIndex, A -> ColIndex,
		  sizeof(int) * A -> RefLength);
    IRIT_GEN_COPY(NewA -> ColLength, A -> ColLength,
		  sizeof(int) * A -> RefLength);

    return NewA;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees the BspKnotAlphaCoeffStruct data structure.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   A:      Alpha matrix to free.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotEvalAlphaCoef, BspKnotEvalAlphaCoefMerge, BspKnotCopyAlphaCoef,   M
*   BspCrvKnotInsert, BspSrfKnotInsert	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotFreeAlphaCoef, alpha matrix, refinement                           M
*****************************************************************************/
void BspKnotFreeAlphaCoef(BspKnotAlphaCoeffStruct *A)
{
    if (GlblCachedA != NULL)
        BspKnotAlphaCoefFree(GlblCachedA);

    GlblCachedA = A;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Frees the BspKnotAlphaCoeffStruct data structrure.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   A:      Alpha matrix to free.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BspKnotAlphaCoefFree(BspKnotAlphaCoeffStruct *A)
{
#ifndef CAGD_OSLO_STRUCT_ALLOC_ONCE
    IritFree(A -> MatrixTransp);
    IritFree(A -> Matrix);
    IritFree(A -> RowsTransp);
    IritFree(A -> Rows);
    IritFree(A -> ColIndex);
    IritFree(A -> ColLength);
    IritFree(A -> _CacheKVT);
    IritFree(A -> _CacheKVt);
#endif /* CAGD_OSLO_STRUCT_ALLOC_ONCE */
    IritFree(A);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as EvalAlphaCoef but the new knot set NewKV is merged with KVT to     M
* form the new knot vector KVt.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   k:           Order of geometry.                                          M
*   KVT:         Original knot vector.                                       M
*   LengthKVT:   Length of original knot vector.                             M
*   NewKV:       A sequence of new knots to introduce into KVT.              M
*   LengthNewKV: Length of new knot sequence.                                M
*   Periodic:    If the refinement is for a periodic entity.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   BspKnotAlphaCoeffStruct *: A matrix to multiply the coefficients of the  M
*                              geometry using KVT, in order to get the       M
*                              coefficients under the space defined using    M
*                              KVt that represent the same geometry.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotFreeAlphaCoef, BspKnotEvalAlphaCoef, BspCrvKnotInsert,            M
*   BspSrfKnotInsert			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotEvalAlphaCoefMerge, alpha matrix, refinement                      M
*****************************************************************************/
BspKnotAlphaCoeffStruct *BspKnotEvalAlphaCoefMerge(int k,
						   CagdRType *KVT,
						   int LengthKVT,
						   CagdRType *NewKV,
						   int LengthNewKV,
						   int Periodic)
{
    BspKnotAlphaCoeffStruct *A;

    if (NewKV == NULL || LengthNewKV == 0) {
	A = BspKnotEvalAlphaCoef(k, KVT, LengthKVT, KVT, LengthKVT, Periodic);
    }
    else {
	int LengthKVt;
	CagdRType
	    *KVt = BspKnotMergeTwo(KVT, LengthKVT + k,
				   NewKV, LengthNewKV, 0, &LengthKVt);

	A = BspKnotEvalAlphaCoef(k, KVT, LengthKVT, KVt, LengthKVt - k,
				 Periodic);

	IritFree(KVt);
    }

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prepares a refinement vector for the given knot vector domain with n	     M
* inserted knots equally spaced.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   n:       Number of knots to introduce.                                   M
*   Tmin:    Minimum domain to introduce knots.                              M
*   Tmax:    Maximum domain to introduce knots.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: A vector of n knots uniformly spaced between TMin and TMax. M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotPrepEquallySpaced, refinement                                     M
*****************************************************************************/
CagdRType *BspKnotPrepEquallySpaced(int n, CagdRType Tmin, CagdRType Tmax)
{
    int i;
    CagdRType dt, t, *RefKV;

    if (n <= 0) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_INDEX);
	return NULL;
    }

    dt = (Tmax - Tmin) / (n + 1),
    t = Tmin + dt,
    RefKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * n);

    for (i = 0; i < n; i++, t += dt) {
	RefKV[i] = t;
    }
    return RefKV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Blend the input control points using the given Alpha matrix.  A non      M
* periodic case is assumed.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   A:          Alpha matrix to use.	 	                             M
*   IMin, IMax: Domain of refined controls points to blend.                  M
*   OrigPts:    original coefficients.       		                     M
*   RefPts:     Refined (returned) coefficients.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotEvalAlphaCoef, BspKnotAlphaLoopBlendPeriodic                      M
*   BspKnotAlphaLoopBlendStep						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAlphaLoopBlendNotPeriodic                                         M
*****************************************************************************/
void BspKnotAlphaLoopBlendNotPeriodic(const BspKnotAlphaCoeffStruct *A,
				      int IMin,
				      int IMax,
				      const CagdRType *OrigPts,
				      CagdRType *RefPts)
{
    int i,
	*ColLength = &A -> ColLength[IMin],
	*ColIndex = &A -> ColIndex[IMin];
    CagdRType *r;
    CagdRType const *p;

    for (i = IMin; i < IMax; i++) {
	switch (*ColLength++) {
 	    case 1:
		*RefPts++ = OrigPts[*ColIndex++];
		break;
 	    case 2:
		p = &OrigPts[*ColIndex];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts++ = p[0] * r[0] +
		            p[1] * r[1];
		break;
 	    case 3:
		p = &OrigPts[*ColIndex];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts++ = p[0] * r[0] +
		            p[1] * r[1] +
		            p[2] * r[2];
		break;
 	    case 4:
		p = &OrigPts[*ColIndex];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts++ = p[0] * r[0] +
		            p[1] * r[1] +
		            p[2] * r[2] +
		            p[3] * r[3];
		break;
	    default:
		{
		    int Len = ColLength[-1];

		    p = &OrigPts[*ColIndex];
		    r = &A -> RowsTransp[i][*ColIndex++];

		    for (*RefPts = *p++ * *r++; --Len > 0; ) {
			*RefPts += *p++ * *r++;
		    }

		    RefPts++;
	        }
		break;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Blend the input control points using the given Alpha matrix.  A non      M
* periodic case is assumed.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   A:          Alpha matrix to use.	 	                             M
*   IMin, IMax: Domain of refined controls points to blend.                  M
*   OrigPts:    original coefficients.       		                     M
*   OrigLen:    Original length of OrigPts.  		                     M
*   RefPts:     Refined (returned) coefficients.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotEvalAlphaCoef, BspKnotAlphaLoopBlendNotPeriodic,                  M
*   BspKnotAlphaLoopBlendStep						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAlphaLoopBlendPeriodic                                            M
*****************************************************************************/
void BspKnotAlphaLoopBlendPeriodic(const BspKnotAlphaCoeffStruct *A,
				   int IMin,
				   int IMax,
				   const CagdRType *OrigPts,
				   int OrigLen,  
				   CagdRType *RefPts)
{
    int i,
	*ColLength = &A -> ColLength[IMin],
	*ColIndex = &A -> ColIndex[IMin];
    CagdRType *r;
    CagdRType const *p;

    for (i = IMin; i < IMax; i++) {
        int Len = *ColLength++,
	    Idx = *ColIndex++;

	if (Idx + Len <= OrigLen) {
	    switch (Len) {
 	        case 1:
		    *RefPts++ = OrigPts[Idx];
		    break;
 	        case 2:
		    p = &OrigPts[Idx];
		    r = &A -> RowsTransp[i][Idx];
		    *RefPts++ = p[0] * r[0] +
		                p[1] * r[1];
		    break;
 	        case 3:
		    p = &OrigPts[Idx];
		    r = &A -> RowsTransp[i][Idx];
		    *RefPts++ = p[0] * r[0] +
		                p[1] * r[1] +
		                p[2] * r[2];
		    break;
 	        case 4:
		    p = &OrigPts[Idx];
		    r = &A -> RowsTransp[i][Idx];
		    *RefPts++ = p[0] * r[0] +
		                p[1] * r[1] +
		                p[2] * r[2] +
		                p[3] * r[3];
		    break;
	        default:
	  	    {
		        if (Len == 1) {
			    *RefPts++ = OrigPts[Idx];
			}
			else {
			    p = &OrigPts[Idx];
			    r = &A -> RowsTransp[i][Idx];

			    for (*RefPts = *p++ * *r++; --Len > 0; ) {
				*RefPts += *p++ * *r++;
			    }

			    RefPts++;
			}
		    }
		    break;
	    }
	}
	else {
	    if (Len == 1) {
		*RefPts++ = OrigPts[Idx >= OrigLen ? Idx - OrigLen : Idx];
	    }
	    else {
		p = &OrigPts[Idx];
		r = A -> RowsTransp[i];

		for (*RefPts = 0.0 ; Len-- > 0; Idx++) {
		    *RefPts += *p++ * r[Idx >= OrigLen ? Idx - OrigLen : Idx];
		}

		RefPts++;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Blend the input control points using the given Alpha matrix.  A non      M
* periodic case is assumed.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   A:          Alpha matrix to use.	 	                             M
*   IMin, IMax: Domain of refined controls points to blend.                  M
*   OrigPts:    original coefficients.       		                     M
*   OrigPtsStep: Steps between adjacent coefficients, in multi-dim. arrays.  M
*   OrigLen:    Original length of OrigPts.  		                     M
*   RefPts:     Refined (returned) coefficients.                             M
*   RefPtsStep: Steps between adjacent refined coefficients, in multi-dim.   M
*		arrays.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotEvalAlphaCoef, BspKnotAlphaLoopBlendNotPeriodic,                  M
*   BspKnotAlphaLoopBlendPeriodic					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAlphaLoopBlendStep                                                M
*****************************************************************************/
void BspKnotAlphaLoopBlendStep(const BspKnotAlphaCoeffStruct *A,
			       int IMin,
			       int IMax,
			       const CagdRType *OrigPts,
			       int OrigPtsStep,
			       int OrigLen,  
			       CagdRType *RefPts,
			       int RefPtsStep)
{
    int i,
	*ColLength = &A -> ColLength[IMin],
	*ColIndex = &A -> ColIndex[IMin];
    CagdRType *r;
    CagdRType const *p;

    for (i = IMin; i < IMax; i++) {
	switch (*ColLength++) {
 	    case 1:
		*RefPts = OrigPts[*ColIndex++ * OrigPtsStep];
		break;
 	    case 2:
		p = &OrigPts[*ColIndex * OrigPtsStep];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts = p[0] * r[0] +
		          p[OrigPtsStep] * r[1];
		break;
 	    case 3:
		p = &OrigPts[*ColIndex * OrigPtsStep];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts = p[0] * r[0] +
		          p[OrigPtsStep] * r[1] +
		          p[OrigPtsStep * 2] * r[2];
		break;
 	    case 4:
		p = &OrigPts[*ColIndex * OrigPtsStep];
		r = &A -> RowsTransp[i][*ColIndex++];
		*RefPts = p[0] * r[0] +
		          p[OrigPtsStep] * r[1] +
		          p[OrigPtsStep * 2] * r[2] +
		          p[OrigPtsStep * 3] * r[3];
		break;
	    default:
		{
		    int Len = ColLength[-1];

		    p = &OrigPts[*ColIndex * OrigPtsStep];
		    r = &A -> RowsTransp[i][*ColIndex++];

		    for (*RefPts = 0.0; Len-- > 0; p += OrigPtsStep) {
			*RefPts += *p * *r++;
		    }
	        }
		break;
	}

	RefPts += RefPtsStep;
    }
}

#ifdef DEBUG
/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the content of the alpha matrix.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   A:        Alpha matrix to print.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void CagdDbgPrintAlphaMat(BspKnotAlphaCoeffStruct *A)
{
    int i, j;

    IRIT_INFO_MSG_PRINTF("Order = %d, Length = %d\n",
	    A -> Order, A -> Length);

    IRIT_INFO_MSG("A matrix:\n");
    for (j = 0; j < A -> RefLength; j++) {
        IRIT_INFO_MSG_PRINTF("%3d) [%2d, %2d] ",
	       j, A -> ColIndex[j], A -> ColLength[j]);
	for (i = 0; i < A -> Length; i++)
	    IRIT_INFO_MSG_PRINTF(" %7.4lg", A -> Rows[i][j]);
	IRIT_INFO_MSG("\n");
    }
    IRIT_INFO_MSG("A transpose matrix:\n");
    for (j = 0; j < A -> RefLength; j++) {
        IRIT_INFO_MSG_PRINTF("%3d) [%2d, %2d] ",
	       j, A -> ColIndex[j], A -> ColLength[j]);
	for (i = 0; i < A -> Length; i++)
	    IRIT_INFO_MSG_PRINTF(" %7.4lg", A -> RowsTransp[j][i]);
	IRIT_INFO_MSG("\n");
    }

    IRIT_INFO_MSG("KVT:\n");
    for (i = 0; i < A -> Length + A -> Order; i++)
        IRIT_INFO_MSG_PRINTF(" %7.4lg ", A -> _CacheKVT[i]);

    IRIT_INFO_MSG("\nKVt:\n");
    for (i = 0; i < A -> RefLength + A -> Order; i++)
        IRIT_INFO_MSG_PRINTF(" %7.4lg ", A -> _CacheKVt[i]);
    IRIT_INFO_MSG( "\n");

    fflush(stdout);
}
#endif /* DEBUG */
