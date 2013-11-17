/******************************************************************************
* Trng_Iso.c - Computes iso parametric curves of triangular surfaces.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single triangular surface to NumOfIsolines          M
* polylines in each parametric direction with SamplesPerCurve in each        M
* isoparametric curve. 							     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. Attempt is made to extract isolines along C1           M
* discontinuities first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:        To extract isoparametric curves from.                     M
*   NumOfIsocurves: In each (U or V or W) direction.                         M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *: List of polylines representing a piecewise linear  M
*                         approximation of the extracted isoparamteric       M
*                         curves or NULL is case of an error.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrf2Polylines, isoparametric curves                               M
*****************************************************************************/
CagdPolylineStruct *TrngTriSrf2Polylines(const TrngTriangSrfStruct *TriSrf,
					 int NumOfIsocurves[3],
					 CagdRType TolSamples,
					 SymbCrvApproxMethodType Method)
{
    int i, j,
    	SamplesPerCurve = (int) TolSamples;
    CagdPolylineStruct
	*PolylineList = NULL;

    for (i = 0; i < 3; i++)
	if (NumOfIsocurves[i] < 2)
	    NumOfIsocurves[i] = 2;
    if (SamplesPerCurve < 2)
	SamplesPerCurve = 2;
 
    if (TRNG_IS_BEZIER_TRISRF(TriSrf)) {
	for (i = 0; i < NumOfIsocurves[0]; i++) {   /* Constant u isolines. */
	    CagdRType
		u = i / ((CagdRType) (NumOfIsocurves[0] - 1)),
		u1 = 1.0 - u;
	    CagdPolylineStruct
	        *Polyline = CagdPolylineNew(SamplesPerCurve);
	    CagdPolylnStruct
		*Pts = Polyline -> Polyline;

	    for (j = 0; j < SamplesPerCurve; j++) {
		CagdRType
		    v = u1 * j / ((CagdRType) (SamplesPerCurve - 1)),
		    *R = TrngTriSrfEval2(TriSrf, u, v);

		CagdCoerceToE3(Pts[j].Pt, &R, -1, TriSrf -> PType);
	    }

	    IRIT_LIST_PUSH(Polyline, PolylineList);
	}
	for (i = 0; i < NumOfIsocurves[1]; i++) {   /* Constant v isolines. */
	    CagdRType
		v = i / ((CagdRType) (NumOfIsocurves[1] - 1)),
		v1 = 1.0 - v;
	    CagdPolylineStruct
	        *Polyline = CagdPolylineNew(SamplesPerCurve);
	    CagdPolylnStruct
		*Pts = Polyline -> Polyline;

	    for (j = 0; j < SamplesPerCurve; j++) {
		CagdRType
		    u = v1 * j / ((CagdRType) (SamplesPerCurve - 1)),
		    *R = TrngTriSrfEval2(TriSrf, u, v);

		CagdCoerceToE3(Pts[j].Pt, &R, -1, TriSrf -> PType);
	    }

	    IRIT_LIST_PUSH(Polyline, PolylineList);
	}
	for (i = 0; i < NumOfIsocurves[2]; i++) {   /* Constant w isolines. */
	    CagdRType
		w = i / ((CagdRType) (NumOfIsocurves[2] - 1)),
		w1 = 1.0 - w;
	    CagdPolylineStruct
	        *Polyline = CagdPolylineNew(SamplesPerCurve);
	    CagdPolylnStruct
		*Pts = Polyline -> Polyline;

	    for (j = 0; j < SamplesPerCurve; j++) {
		CagdRType
		    v = w1 * j / ((CagdRType) (SamplesPerCurve - 1)),
		    *R = TrngTriSrfEval2(TriSrf, 1.0 - v - w, v);

		CagdCoerceToE3(Pts[j].Pt, &R, -1, TriSrf -> PType);
	    }

	    IRIT_LIST_PUSH(Polyline, PolylineList);
	}
	return PolylineList;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a triangular surface NumOfIsoline isocurve list  M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   If, however, oout of the three NumOfIsocurves values, two are zero and   M
* one NumOfIsocurves values equals to one, extarct one isocurve at that      M
* direction, at value Val.						     M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
*									     M
* Consider isoparametric curve of Bezier triangular surface at fixed u = u0: M
*	__								     V
*	\          n!             i   j              n-i-j		     V
*	/  -------------------  u0   v   (1 - u0 - v)	   b    =	     V
*	--  i! j! (n - i - j)!				    ijk		     V
*									     V
*  __	          __							     V
*  \     n!     i \      (n - i)!       j             n-i-j                  V
*  /  ------- u0  /  ----------------  v  (1 - u0 - v)      b    =           V
*  -- i!(n-i)!    --  j! (n - i - j)!			     ijk             V
*									     V
* __	                   __						     V
* \     n!     i       n-i \     (n - i)!       v   j       v   n-i-j        V
* /  ------- u0  (1-u0)    /  --------------- (----)  (1 - ----)      b      V
* -- i!(n-i)!              -- j! (n - i - j)!  1-u0	   1-u0	       ijk   V
*									     M
*   Hence, the isoparametric curve of u = u0 is a weighted sum of a sequence M
* of Bezier curves of degree 1 to n-1, each defined over a row of the        M
* triangular mesh, over the domain of v = [0, 1-u0].			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:          To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In each (U or V or W) direction.                        M
*   Val:	     If only one isocurve to extract - do so at value Val.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrngTriSrf2Curves, TrngCrvFromTriSrf                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriBzrSrf2Curves, curves, isoparametric curves                       M
*****************************************************************************/
CagdCrvStruct *TrngTriBzrSrf2Curves(const TrngTriangSrfStruct *TriSrf,
				    int NumOfIsocurves[3],
				    IrtRType Val)
{
    CagdPointType
	PType = TriSrf -> PType;
    int d,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType),
	Length = TriSrf -> Length;
    CagdBType
	SingleIsoCurve = FALSE,
	IsNotRational = !TRNG_IS_RATIONAL_TRISRF(TriSrf);
    CagdRType
	* const *TriSrfPoints = TriSrf -> Points;
    CagdCrvStruct
	*CrvList = NULL;

    if (NumOfIsocurves[0] + NumOfIsocurves[1] + NumOfIsocurves[2] == 1) {
	int i;

	/* Needs to extract a single isocurve at value Val. */
	SingleIsoCurve = TRUE;

	/* We need at least two isolines to generate something below. */
	for (i = 0; i < 3; i++)
	    NumOfIsocurves[i] *= 2;

	if (Val < 0.0 || Val > 1.0) {
	    TRNG_FATAL_ERROR(TRNG_ERR_WRONG_DOMAIN);
	    return FALSE;
	}
    }

    if (TRNG_IS_BEZIER_TRISRF(TriSrf)) {
	for (d = 0; d < 3; d++) {     /* For 3 different isoparametric dirs. */
	    int i, j, l;

	    /* The following is written for U isoparametric curves basically */
	    /* but we rotate the ijk indices in accessing the triangular     */
	    /* mesh, extracting isocurves in all three directions.	     */
	    for (l = 0; l < NumOfIsocurves[d] - 1; l++) {
		CagdRType uu0, uu1,
		    u0 = SingleIsoCurve ? Val
					: l / ((CagdRType) (NumOfIsocurves[d] - 1)),
		    u1 = 1.0 - u0;
		CagdCrvStruct
		    *Crv = BzrCrvNew(Length, PType);
		CagdRType
		    **Points = Crv -> Points;

		for (i = IsNotRational; i <= MaxCoord; i++) { /* Clr isocrv. */
		    CagdRType
			*R = Points[i];

		    for (j = 0; j < Length; j++)
			*R++ = 0.0;
		}

		for (i = 1, uu1 = 1; i < Length; i++, uu1 *= u1); /*(1-u0)^n.*/

		for (i = 0, uu0 = 1; i < Length; i++) {
		    CagdRType
			Wgt = TrngIJChooseN(i, 0, Length - 1) * uu0 * uu1;
		    CagdCrvStruct *TCrv2,
			*TCrv = BzrCrvNew(Length - i, PType);
		    CagdRType
			**TPoints = TCrv -> Points;

		    for (j = 0; j < Length - i; j++) {
			int m, Index,
			    k = Length - i - j - 1;

			switch (d) {
			    default:
			    case 0:
				Index = TRNG_MESH_IJK(TriSrf, i, j, k);
				break;
			    case 1:
				Index = TRNG_MESH_IJK(TriSrf, j, k, i);
				break;
			    case 2:
				Index = TRNG_MESH_IJK(TriSrf, k, i, j);
				break;
			}

			for (m = IsNotRational; m <= MaxCoord; m++)
			    TPoints[m][j] = TriSrfPoints[m][Index] * Wgt;
		    }

		    TCrv2 = SymbCrvAdd(Crv, TCrv);     /* Also degree raise! */
		    CagdCrvFree(Crv);
		    CagdCrvFree(TCrv);
		    Crv = TCrv2;
		    uu0 *= u0;
		    uu1 /= (u1 == 0.0 ? IRIT_UEPS : u1);
		}

		if (SingleIsoCurve)
		    return Crv;
		else
		    IRIT_LIST_PUSH(Crv, CrvList);
	    }
	}

	return CrvList;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts an isoparametric curve from the triangular surface TriSrf in      M
* direction Dir at the parameter value of t.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:    To extract an isoparametric curve from.                       M
*   t:         Parameter value of extracted isoparametric curve.             M
*   Dir:       Direction of extracted isocurve. Either U or V or W.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An isoparametric curve of TriSrf. This curve inherit  M
*                      the order and continuity of TriSrf in direction Dir.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfCrvFromSrf, BspSrfCrvFromSrf, CagdCrvFromMesh, BzrSrfCrvFromMesh,  M
*   BspSrfCrvFromMesh, CagdCrvFromSrf, TrngTriBzrSrf2Curves                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngCrvFromTriSrf, isoparametric curves, curve from surface              M
*****************************************************************************/
CagdCrvStruct *TrngCrvFromTriSrf(const TrngTriangSrfStruct *TriSrf,
				 CagdRType t,
				 TrngTriSrfDirType Dir)
{
    int i, NumOfIsocurves[3];

    for (i = 0; i < 3; i++)
	NumOfIsocurves[i] = 0;

    switch (Dir) {
	case TRNG_CONST_U_DIR:
	    NumOfIsocurves[0] = 1;
	    break;
	case TRNG_CONST_V_DIR:
	    NumOfIsocurves[1] = 1;
	    break;
	case TRNG_CONST_W_DIR:
	    NumOfIsocurves[2] = 1;
	    break;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_DIR_NOT_VALID);
	    break;
    }

    return TrngTriBzrSrf2Curves(TriSrf, NumOfIsocurves, t);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a triangular surface NumOfIsoline isocurve list  M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:          To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In each (U or V or W) direction.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrngTriBzrSrf2Curves, TrngCrvFromTriSrf                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrf2Curves, curves, isoparametric curves                          M
*****************************************************************************/
CagdCrvStruct *TrngTriSrf2Curves(const TrngTriangSrfStruct *TriSrf,
				 int NumOfIsocurves[3])
{
    int d;

    for (d = 0; d < 3; d++)
        if (NumOfIsocurves[d] < 2)
	    NumOfIsocurves[d] = 2;

    if (TRNG_IS_BEZIER_TRISRF(TriSrf)) {
	return TrngTriBzrSrf2Curves(TriSrf, NumOfIsocurves, 0.0);
    }
    else
	return NULL;
}
