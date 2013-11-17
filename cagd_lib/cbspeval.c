/******************************************************************************
* CBspEval.c - Bezier curves handling routines - evaluation routines.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Assumes Vec holds control points for scalar Bspline curve of order Order   M
* length Len and knot vector KnotVector.				     M
*   Evaluates and returns that curve value at parameter value t.	     M
*   Vec is incremented by VecInc (usually by 1) after each iteration.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:            Coefficents of a scalar Bspline univariate function.     M
*   VecInc:         Step to move along Vec.                                  M
*   KnotVector:     Knot vector of associated geoemtry.                      M
*   Order:          Order of associated geometry.                            M
*   Len:            Length of control vector.                                M
*   Periodic:       If this geometry is Periodic.                            M
*   t:              Parameter value where to evaluate the curve.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:      Geometry's value at parameter value t.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvEvalVecAtParam, evaluation                                         M
*****************************************************************************/
CagdRType BspCrvEvalVecAtParam(const CagdRType *Vec,
			       int VecInc,
			       const CagdRType *KnotVector,
			       int Order,
			       int Len,
			       CagdBType Periodic,
			       CagdRType t)
{
    int i, IndexFirst;
    CagdRType
	R = 0.0,
	*BasisFunc = BspCrvCoxDeBoorBasis(KnotVector, Order, Len, Periodic,
					  t, &IndexFirst);

    if (VecInc == 1) {
	for (i = 0; i < Order; i++)
	    R += BasisFunc[i] * Vec[IndexFirst++ % Len];
    }
    else {
	int IndexFirstInc = IndexFirst;

	IndexFirstInc *= VecInc;
	for (i = 0; i < Order; i++) {
	    R += BasisFunc[i] * Vec[IndexFirstInc];
	    IndexFirstInc += VecInc;
	    if (++IndexFirst >= Len) {
	        IndexFirst -= Len;
		IndexFirstInc -= Len * VecInc;
	    }
	}
    }

    return R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to a static data, holding the value of the curve at      M
* given parametric location t. The curve is assumed to be Bspline.	     M
*   Uses the Cox de Boor recursive algorithm.                                M
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
*   CagdCrvEval, BzrCrvEvalAtParam, BzrCrvEvalVecAtParam, 	             M
*   BspCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvEvalAtParam, evaluation                                            M
*****************************************************************************/
CagdRType *BspCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    return BspCrvEvalCoxDeBoor(Crv, t);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Samples the curve at FineNess location equally spaced in the curve's       M
* parametric domain.							     M
*   Computes a refinement alpha matrix (If FineNess > 0), refines the curve  M
* and uses refined control polygon as the approximation to the curve.	     M
*   If FineNess == 0, Alpha matrix A is used instead.			     M
*   Returns the actual number of points in polyline (<= FineNess).	     M
* Note this routine may be invoked with Bezier curves as well as Bspline.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         To approximate as a polyline.                               M
*   FineNess:    Control on number of samples.                               M
*   Points:      Where to put the resulting polyline.                        M
*   A:           Optional alpha matrix for refinement.                       M
*   OptiLin:     If TRUE, optimize linear curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         The actual number of samples placed in Points. Always       M
*                less than or eaul to FineNess.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvEvalToPolyline, AfdBzrCrvEvalToPolyline, CagdCrvEval               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvEvalToPolyline, conversion, refinement, evaluation                M
*****************************************************************************/
int CagdCrvEvalToPolyline(const CagdCrvStruct *Crv,
			  int FineNess,
			  CagdRType *Points[],
			  BspKnotAlphaCoeffStruct *A,
			  CagdBType OptiLin)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, Count, PeriodicLen,
	Len = Crv -> Length,
    	n = FineNess == 0 ? A -> RefLength : FineNess,
	Order = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);

    if (Crv -> Order == 2 && OptiLin) {	 /* Simply copy the control polygon. */
	CagdRType
	    * const *CrvPoints = Crv -> Points;
	int Size;

	Count = IRIT_MIN(n, Len);
	Size = Count * sizeof(CagdRType);
	
	for (i = IsNotRational; i <= MaxCoord; i++)
	    CAGD_GEN_COPY(Points[i], CrvPoints[i], Size);

	if (CAGD_IS_PERIODIC_CRV(Crv) &&
	    CAGD_IS_BSPLINE_CRV(Crv) &&
	    Count < n) {
	    for (i = IsNotRational; i <= MaxCoord; i++)
	        Points[i][Count] = CrvPoints[i][0];
	    Count++;
	}

	return Count;
    }

    PeriodicLen = CAGD_CRV_PT_LST_LEN(Crv);

    if (FineNess > 0 && n > PeriodicLen) {     /* Compute the A matrix here. */
	CagdRType *RefKV, Tmin, Tmax;

	CagdCrvDomain(Crv, &Tmin, &Tmax);

	RefKV = BspKnotPrepEquallySpaced(n - PeriodicLen, Tmin, Tmax);

	if (CAGD_IS_BEZIER_CRV(Crv)) {
	    CagdRType
		*KV = BspKnotUniformOpen(Crv -> Length, Crv -> Order, NULL);

	    A = BspKnotEvalAlphaCoefMerge(Order, KV, Len, RefKV,
					  n - Len, FALSE);

	    IritFree(KV);
	}
	else {
	    A = BspKnotEvalAlphaCoefMerge(Order, Crv -> KnotVector,
					  PeriodicLen, RefKV, n - PeriodicLen,
					  FALSE);
	}

	IritFree(RefKV);
    }

    if (n > PeriodicLen) {
	/* Compute refined control polygon straight to destination Points. */
	for (j = IsNotRational; j <= MaxCoord; j++) {
	    CagdRType
	        *ROnePts = Points[j],
	        *OnePts = Crv -> Points[j];

	    if (CAGD_IS_PERIODIC_CRV(Crv)) {
	        BspKnotAlphaLoopBlendPeriodic(A, 0, n, OnePts, Crv -> Length,
					      ROnePts);
	    }
	    else {
	        BspKnotAlphaLoopBlendNotPeriodic(A, 0, n, OnePts, ROnePts);
	    }
	}

	if (FineNess > 0)
	    BspKnotFreeAlphaCoef(A);
    }
    else if (n == PeriodicLen) {
        /* Copy the control polygons instead. */
        for (j = IsNotRational; j <= MaxCoord; j++) {
	    CAGD_GEN_COPY(Points[j], Crv -> Points[j], sizeof(CagdRType) * n);
	}
    }
    else {
        CAGD_FATAL_ERROR(CAGD_ERR_REF_LESS_ORIG);
    }

    return n;
}
