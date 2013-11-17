/******************************************************************************
* CBzrEval.c - Bezier curves handling routines - evaluation routines.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

IRIT_STATIC_DATA int
    BezierCacheEnabled = FALSE,
    GlblCacheFineNess = 0;
IRIT_STATIC_DATA CagdRType *BezierCache[CAGD_MAX_BEZIER_CACHE_ORDER + 1]
			               [CAGD_MAX_BEZIER_CACHE_ORDER + 1];

static CagdRType IntPow(CagdRType x, int i);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the bezier sampling cache - if enabled, a Bezier can be evaluated     M
* directly from presampled basis function.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FineNess:       Number of samples to support.                            M
*   EnableCache:    Are we really planning on using this thing?              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvSetCache, evaluation, caching                                      M
*****************************************************************************/
void BzrCrvSetCache(int FineNess, CagdBType EnableCache)
{
    int i, j, k;

    if ((BezierCacheEnabled == EnableCache && FineNess == GlblCacheFineNess) ||
	FineNess < 2)
	return;

    if (BezierCacheEnabled) {
	/* Free old cache if was one. */
	for (k = 2; k <= CAGD_MAX_BEZIER_CACHE_ORDER; k++)
	    for (i = 0; i < k; i++)
		if (BezierCache[k][i] != NULL) {
		    IritFree(BezierCache[k][i]);
		    BezierCache[k][i] = NULL;
		}
    }

    if ((BezierCacheEnabled = EnableCache) != FALSE) {
	/* Allocate the new cache and initalize it: */
	if (FineNess < 2)
	    FineNess = 2;
	if (FineNess > CAGD_MAX_BEZIER_CACHE_FINENESS)
	    FineNess = CAGD_MAX_BEZIER_CACHE_FINENESS;
	GlblCacheFineNess = FineNess;

	for (k = 2; k <= CAGD_MAX_BEZIER_CACHE_ORDER; k++)/* For all orders. */
	    for (i = 0; i < k; i++) { 	 /* Allocate and set all basis func. */
		BezierCache[k][i] = (CagdRType *)
			IritMalloc(sizeof(CagdRType) * GlblCacheFineNess);
		for (j = 0; j < GlblCacheFineNess; j++)
		    BezierCache[k][i][j] = BzrCrvEvalBasisFunc(i, k,
				((CagdRType) j) / (GlblCacheFineNess - 1));
	    }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Assumes Vec holds control points for scalar bezier curve of order Order,   M
* and evaluates and returns that curve value at parameter value t.	     M
*   Vec is incremented by VecInc (usually by 1) after each iteration.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:            Coefficents of a scalar Bspline univariate function.     M
*   VecInc:         Step to move along Vec.                                  M
*   Order:          Order of associated geometry.                            M
*   t:              Parameter value where to evaluate the curve.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:      Geometry's value at parameter value t.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspCrvEvalAtParam, BzrCrvEvalAtParam,                       M
*   BspCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalVecAtParam, evaluation                                         M
*****************************************************************************/
CagdRType BzrCrvEvalVecAtParam(const CagdRType *Vec,
			       int VecInc,
			       int Order,
			       CagdRType t)
{
    int i;
    CagdRType
        *BasisFuncs = BzrCrvEvalBasisFuncs(Order, t),
	R = 0.0;

    if (VecInc == 1)
	for (i = 0; i < Order; i++)
	    R += *BasisFuncs++ * *Vec++;
    else
	for (i = 0; i < Order; i++) {
	    R += *BasisFuncs++ * *Vec;
	    Vec += VecInc;
	}

    return R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to a static data, holding the value of the curve at      M
* given parametric location t. The curve is assumed to be Bezier.	     M
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
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspCrvEvalAtParam, BspCrvEvalVecAtParam,                    M
*   BzrCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalAtParam, evaluation                                            M
*****************************************************************************/
CagdRType *BzrCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    IRIT_STATIC_DATA CagdRType Pt[CAGD_MAX_PT_COORD];
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	k = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType B,
        *BasisFuncs = BzrCrvEvalBasisFuncs(k, t);
    CagdRType
	* const *Points = Crv -> Points;

    for (j = IsNotRational; j <= MaxCoord; j++)
	Pt[j] = 0.0;

    for (i = 0; i < k; i++) {
	B = BasisFuncs[i];
	for (j = IsNotRational; j <= MaxCoord; j++)
	    Pt[j] += B * Points[j][i];
    }

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to a static data, holding the value of the curve at      M
* given parametric location t. The curve is assumed to be Bezier.	     M
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
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspCrvEvalAtParam, BspCrvEvalVecAtParam,                    M
*   BzrCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalAtParam2, evaluation                                           M
*****************************************************************************/
CagdRType *BzrCrvEvalAtParam2(CagdCrvStruct *Crv, CagdRType t)
{
    IRIT_STATIC_DATA int
	BufSize = 0;
    static CagdRType (*Buf)[CAGD_MAX_PT_COORD] = NULL;
    CagdBType
        IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, n,
        k = Crv -> Order,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType t1 = 1.0 - t;

    if (BufSize < k) {
        if (Buf != NULL)
	    IritFree(Buf);
        Buf = (CagdRType (*)[CAGD_MAX_PT_COORD])
            IritMalloc(sizeof(CagdRType [CAGD_MAX_PT_COORD]) * k);
        BufSize = k;
    }

    for (i = 0; i < k; i++)
        for (j = IsNotRational; j <= MaxCoord; j++)
	    Buf[i][j] = Crv -> Points[j][i];

    for (n = 1; n < k; n++)
	for (i = 0; i < k - n; i++)
	    for (j = IsNotRational; j <= MaxCoord; j++)
		Buf[i][j] = t1 * Buf[i][j] + t * Buf[i+1][j];

    return Buf[0];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Samples the curve at FineNess location equally spaced in the curve's       M
* parametric domain.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         To approximate as a polyline.                               M
*   FineNess:    Control on number of samples.                               M
*   Points:      Where to put the resulting polyline.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspCrvEvalAtParam, BzrCrvEvalVecAtParam,                    M
*   BspCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalToPolyline, conversion, refinement, evaluation                 M
*****************************************************************************/
void BzrCrvEvalToPolyline(const CagdCrvStruct *Crv,
			  int FineNess,
			  CagdRType *Points[])
{
    CagdBType
	UseCache = BezierCacheEnabled &&
		   FineNess == GlblCacheFineNess &&
		   Crv -> Order <= CAGD_MAX_BEZIER_CACHE_ORDER,
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, Count,
	k = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType B;

    if (UseCache) {
        for (j = IsNotRational; j <= MaxCoord; j++)
	    IRIT_ZAP_MEM(Points[j], sizeof(CagdRType) * FineNess);

	for (Count = 0; Count < GlblCacheFineNess; Count++) {
	    for (i = 0; i < k; i++) {
		B = BezierCache[k][i][Count];
		for (j = IsNotRational; j <= MaxCoord; j++)
		    Points[j][Count] += B * Crv -> Points[j][i];
	    }
	}
    }
    else {
        for (j = IsNotRational; j <= MaxCoord; j++)
	    IRIT_ZAP_MEM(Points[j], sizeof(CagdRType) * FineNess);

	for (Count = 0; Count < FineNess; Count++) {
	    CagdRType
	        *BasisFuncs = BzrCrvEvalBasisFuncs(k,
					((CagdRType) Count) / (FineNess - 1));

	    for (i = 0; i < k; i++) {
	        B = BasisFuncs[i];
		for (j = IsNotRational; j <= MaxCoord; j++)
		    Points[j][Count] += Crv -> Points[j][i] * B;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the i'th Bezier basis function of order k, at parametric value t M
* (t in [0..1]).							     M
*   The functions is:		     i	   i          k - i - 1		     V
*			Bi,k-1(t) = ( ) * t  * (1 - t)			     V
*				    k-1					     *
* PARAMETERS:                                                                M
*   i:   I'th basis function.                                                M
*   k:   Order of the basis function.                                        M
*   t:   Parameter value at which to evaluate the Bezier basis function.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  Value of basis function.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvEvalBasisFuncs                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalBasisFunc                                                      M
*****************************************************************************/
CagdRType BzrCrvEvalBasisFunc(int i, int k, CagdRType t)
{
    if (IRIT_APX_EQ(t, 0.0))
	return (CagdRType) (i == 0);
    else if (IRIT_APX_EQ(t, 1.0))
	return (CagdRType) (i == k - 1);
    else
	return (k >= CAGD_MAX_BEZIER_CACHE_ORDER ?
		    CagdIChooseK(i, k - 1) : CagdIChooseKTable[k - 1][i]) *
	       IntPow(1.0 - t, k - i - 1) * IntPow(t, i);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the vector of Bezier basis functions of order k, at parametric   M
* value t (t in [0..1]).						     M
*   The functions are:		     i	   i          k - i - 1		     V
*			Bi,k-1(t) = ( ) * t  * (1 - t)			     V
*				    k-1					     *
* PARAMETERS:                                                                M
*   k:   Order of the basis function.                                       M
*   t:   Parameter value at which to evaluate the Bezier basis function.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  Value of basis function's vector (allocated statically).   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvEvalBasisFunc                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvEvalBasisFuncs                                                     M
*****************************************************************************/
CagdRType *BzrCrvEvalBasisFuncs(int k, CagdRType t)
{
    IRIT_STATIC_DATA int
	AllocOrder = 0;
    IRIT_STATIC_DATA CagdRType
	*Vec = NULL;
    int i;
    CagdRType r,
	t1 = 1.0 - t;

    /* Make sure we have a vector of proper length. */
    if (AllocOrder < k) {
	AllocOrder = 2 * k;

	if (Vec != NULL)
	    IritFree(Vec);
	Vec = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocOrder);
    }

    if (k >= CAGD_MAX_BEZIER_CACHE_ORDER) {
        /* Initialize with the combinatorial term and the t^i term. */
        Vec[0] = CagdIChooseK(0, k - 1);

	for (i = 0, r = t; ++i < k; r *= t)
	    Vec[i] = r * CagdIChooseK(i, k - 1);
    }
    else {
        /* Initialize with the combinatorial term and the t^i term. */
        Vec[0] = CagdIChooseKTable[k - 1][0];

	for (i = 0, r = t; ++i < k; r *= t)
	    Vec[i] = r * CagdIChooseKTable[k - 1][i];
    }

    /* Do the (1-t)^(k-i-1) term. */
    for (i = k - 1, r = t1; --i >= 0; r *= t1)
	Vec[i] *= r;

    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes x to the power of i, i integer.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   x, i: Description says it all.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  Integer power of x, computed using pwoer of 2's.             *
*****************************************************************************/
static CagdRType IntPow(CagdRType x, int i)
{
    CagdRType Power, RetVal;

    for (Power = x, RetVal = 1.0; i != 0; i >>= 1) {
	if (i & 0x01)
	   RetVal *= Power;
	
	Power = IRIT_SQR(Power);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the following (in floating point arithmetic):		     M
*			 k         k!					     V
*			( ) = -------------				     V
*			 i    i! * (k - i)!				     V
*                                                                            *
* PARAMETERS:                                                                M
*   i, k:   Coefficients of i choose k.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Result of i choose k, in floating point, to prevent from    M
*                overflows.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIChooseK, evaluation, combinatorics                                  M
*****************************************************************************/
CagdRType CagdIChooseK(int i, int k)
{
    int j;
    CagdRType c;

    if (k < CAGD_MAX_BEZIER_CACHE_ORDER)
	return CagdIChooseKTable[k][i];

    c = 1.0;

    if ((k >> 1) > i) {				/* i is less than half of k: */
	for (j = k - i + 1; j <= k; j++)
	    c *= j;
	for (j = 2; j <= i; j++)
	    c /= j;
    }
    else {
	for (j = i + 1; j <= k; j++)
	    c *= j;
	for (j = 2; j <= k - i; j++)
	    c /= j;
    }

    return c;
}
