/******************************************************************************
* TrngEval.c - triangular surface evaluation routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include <string.h>
#include "trng_loc.h"

#define MAX_MULTS_VAL	20  /* Maximal order of triangular Bezier supported. */

/*****************************************************************************
* DESCRIPTION:                                                               *
* Evaluates the following (in floating point arithmetic):		     *
*			  N              N!				     *
*			(   ) = ----------------------			     *
*			 i j    i! * j! * (N - i - j)!			     *
*                                                                            *
* PARAMETERS:                                                                *
*   i, j, N:   Coefficients of combinatorial expression.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Result of ij choose N, in floating point, to prevent from   *
*                overflows.                                                  *
*                                                                            *
* KEYWORDS:                                                                  *
*   TrngIJChooseN, evaluation, combinatorics                                 *
*****************************************************************************/
CagdRType TrngIJChooseN(int i, int j, int N)
{
    IRIT_STATIC_DATA CagdRType
	Facts[MAX_MULTS_VAL] = { -1.0 };

    if (Facts[0] < 0.0) {
	int l = 0;

	Facts[0] = 1;
	for (l = 1; l < MAX_MULTS_VAL; l++)
	    Facts[l] = l * Facts[l - 1];
    }
    if (N >= MAX_MULTS_VAL) {
	IRIT_WARNING_MSG("TrngLib: Fatal: Order of triangular Bezier too large - increase MAX_MULTS_VAL\n");
	return 1.0;
    }

    return Facts[N] / (Facts[i] * Facts[j] * Facts[N - i - j]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given triangular surface at a given point.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To evaluate at given (u, v, w) parametric location.            M
*   u, v, w:  Parametric location to evaluate TriSrf at.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the triangualr surface's point type. If for example     M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfEval, evaluation, triangular surfaces                          M
*****************************************************************************/
CagdRType *TrngTriSrfEval(const TrngTriangSrfStruct *TriSrf,
			  CagdRType u,
			  CagdRType v,
			  CagdRType w)
{
    IRIT_STATIC_DATA CagdRType Pt[CAGD_MAX_PT_COORD];
    int i, j,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TriSrf -> PType),
	Length = TriSrf -> Length;
    CagdBType
	IsNotRational = !TRNG_IS_RATIONAL_TRISRF(TriSrf);
    CagdRType B, uu, vv, ww,
	* const *Points = TriSrf -> Points;

    for (j = IsNotRational; j <= MaxCoord; j++)
	Pt[j] = 0.0;

    if (TRNG_IS_BEZIER_TRISRF(TriSrf)) {
	for (i = 0, uu = 1.0; i < Length; i++, uu *= u) {	    
	    for (j = 0, vv = 1.0; j < Length - i; j++, vv *= v) {
		int l, kk,
		    k = Length - i - j - 1,
		    Index = TRNG_MESH_IJK(TriSrf, i, j, k);

		for (kk = 0, ww = 1.0; kk < k; kk++)
		    ww *= w;

		B = TrngIJChooseN(i, j, Length - 1) * uu * vv * ww;

		for (l = IsNotRational; l <= MaxCoord; l++)
		    Pt[l] += B * Points[l][Index];
	    }
	}
    }
    else if (TRNG_IS_BSPLINE_TRISRF(TriSrf)) {
	TRNG_FATAL_ERROR(TRNG_ERR_BSPLINE_NO_SUPPORT);
	return NULL;
    }

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given triangular surface at a given point. Same as function  M
* TrngTriSrfEval with w computed using 'w = 1 - u - v' for Bezier triangular M
* surfaces.	                                                             M
*									     *
* PARAMETERS:                                                                M
*   TriSrf:   To evaluate at given (u, v) parametric location.               M
*   u, v:     Parametric location to evaluate TriSrf at.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the triangualr surface's point type. If for example     M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfEval2, evaluation, triangular surfaces                         M
*****************************************************************************/
CagdRType *TrngTriSrfEval2(const TrngTriangSrfStruct *TriSrf,
			   CagdRType u,
			   CagdRType v)
{
    if (TRNG_IS_BEZIER_TRISRF(TriSrf)) {
	return TrngTriSrfEval(TriSrf, u, v, 1.0 - u - v);
    }
    else if (TRNG_IS_BSPLINE_TRISRF(TriSrf)) {
	TRNG_FATAL_ERROR(TRNG_ERR_BSPLINE_NO_SUPPORT);
	return NULL;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the normal of the given triangular surface at a given point.     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To evaluate at given (u, v, w) parametric location.            M
*   u, v:     Parametric location to evaluate normal of TriSrf at.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit normal   M
*                     information.                                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BzrSrfNormal, BspSrfNormal, SymbSrfNormalSrf		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfNrml, evaluation, triangular surfaces                          M
*****************************************************************************/
CagdVecStruct *TrngTriSrfNrml(const TrngTriangSrfStruct *TriSrf,
			      CagdRType u,
			      CagdRType v)
{
    IRIT_STATIC_DATA CagdVecStruct Normal;
    CagdVType Vec1, Vec2;
    CagdPType Pt1, Pt2;
    CagdRType *R, UMin, UMax, VMin, VMax, WMin, WMax, Du, Dv;

    TrngTriSrfDomain(TriSrf, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    R = TrngTriSrfEval2(TriSrf, u, v);
    CagdCoerceToE3(Pt1, &R, -1, TriSrf -> PType);

    Du = UMax - u > u - UMin ? u + IRIT_EPS : u - IRIT_EPS;
    R = TrngTriSrfEval2(TriSrf, Du, v);
    CagdCoerceToE3(Pt2, &R, -1, TriSrf -> PType);
    IRIT_PT_SUB(Vec1, Pt1, Pt2);
    IRIT_PT_SCALE(Vec1, 1 / (Du - u));

    Dv = VMax - v > v - VMin ? v + IRIT_EPS : v - IRIT_EPS;
    R = TrngTriSrfEval2(TriSrf, u, Dv);
    CagdCoerceToE3(Pt2, &R, -1, TriSrf -> PType);
    IRIT_PT_SUB(Vec2, Pt1, Pt2);
    IRIT_PT_SCALE(Vec2, 1 / (Dv - v));

    IRIT_CROSS_PROD(Normal.Vec, Vec1, Vec2);
    IRIT_PT_NORMALIZE(Normal.Vec);

    return &Normal;
}
