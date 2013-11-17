/******************************************************************************
* CagdBbox.c - Handle freeform cuves and surfaces bounding boxes.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jan. 92.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define BBOX_CRV_KNOTS	20
#define BBOX_SRF_KNOTS	20

IRIT_STATIC_DATA CagdBType
    GlblIgnoreNonPosWeightBBox = FALSE,
    GlblTightBBox = FALSE;
IRIT_STATIC_DATA CagdRType
    *GlblNewKVs = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Enforce the computation of a tighter bounding box for a freeform.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TightBBox:   TRUE for tight bbox on freeforms, FALSE for, simpler,       M
*		 looser bbox that is derived using the control poly/mesh.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   old value.                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBBox, CagdSrfBBox, CagdIgnoreNonPosWeightBBox                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdTightBBox, bbox, bounding box                                        M
*****************************************************************************/
CagdBType CagdTightBBox(CagdBType TightBBox)
{
    CagdBType
	OldVal = GlblTightBBox;

    GlblTightBBox = TightBBox;

    if (TightBBox && GlblNewKVs == NULL)
	GlblNewKVs =
	    (CagdRType *) IritMalloc(IRIT_MAX(BBOX_CRV_KNOTS, BBOX_SRF_KNOTS)
				                         * sizeof(CagdRType));

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a freeform curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   IgnoreNonPosWeightBBox:   TRUE to ignore negative and zero weight        M
*		 control points in the bounding box computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   old value.                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBBox, CagdSrfBBox, CagdTightBBox                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIgnoreNonPosWeightBBox, bbox, bounding box                           M
*****************************************************************************/
CagdBType CagdIgnoreNonPosWeightBBox(CagdBType IgnoreNonPosWeightBBox)
{
    CagdBType
	OldVal = GlblIgnoreNonPosWeightBBox;

    GlblIgnoreNonPosWeightBBox = IgnoreNonPosWeightBBox;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a freeform curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvListBBox, CagdSrfBBox, CagdTightBBox, CagdIgnoreNonPosWeightBBox, M
*   CagdPolygonBBox							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvBBox, bbox, bounding box                                          M
*****************************************************************************/
void CagdCrvBBox(const CagdCrvStruct *Crv, CagdBBoxStruct *BBox)
{
    CagdCrvStruct *CpCrv;

    if (GlblTightBBox && Crv -> Order > 2) { /* Refine crv before bbox test. */
	CagdRType TMin, TMax,
	    *KV = Crv -> KnotVector;
	int i,
	    l = 0,
	    KVLen = Crv -> Length + Crv -> Order;

	CagdCrvDomain(Crv, &TMin, &TMax);
	for (i = 0; i < BBOX_CRV_KNOTS; i++) {
	    int j;
	    CagdRType
		t = TMin + i * (TMax - TMin) / (BBOX_CRV_KNOTS - 1);

	    if (CAGD_IS_BSPLINE_CRV(Crv)) {
		j = BspKnotLastIndexLE(KV, KVLen, t);
		if (!IRIT_APX_EQ(KV[j], t))
		    GlblNewKVs[l++] = t;
	    }
	    else {
		GlblNewKVs[l++] = t;
	    }
	}

	Crv = CpCrv = CagdCrvRefineAtParams(Crv, FALSE, GlblNewKVs, l);
    }
    else
        CpCrv = NULL;

    CagdPointsBBox(Crv -> Points, Crv -> Length, 3, BBox -> Min, BBox -> Max);

    if (CpCrv  != NULL)
	CagdCrvFree(CpCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of freeform curves.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:     To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBBox, CagdSrfBBox, CagdTightBBox                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvListBBox, bbox, bounding box                                      M
*****************************************************************************/
void CagdCrvListBBox(const CagdCrvStruct *Crvs, CagdBBoxStruct *BBox)
{
    CAGD_RESET_BBOX(BBox);

    for ( ; Crvs != NULL; Crvs = Crvs -> Pnext) {
	CagdBBoxStruct TmpBBox;

	CagdCrvBBox(Crvs, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a freeform surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBBox, CagdSrfListBBox, CagdTightBBox, CagdIgnoreNonPosWeightBBox, M
*   CagdPolygonBBox							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfBBox, bbox, bounding box                                          M
*****************************************************************************/
void CagdSrfBBox(const CagdSrfStruct *Srf, CagdBBoxStruct *BBox)
{
    CagdSrfStruct *CpSrf;

    if (GlblTightBBox) {         /* Refine the surface before bbox testing. */
	CagdRType UMin, UMax, VMin, VMax,
	    *KV = Srf -> UKnotVector;
	int i, l,
	    KVLen = Srf -> ULength + Srf -> UOrder;
	CagdSrfStruct *AuxSrf;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	for (i = l = 0; i < BBOX_SRF_KNOTS; i++) {
	    int j;
	    CagdRType
		u = UMin + i * (UMax - UMin) / (BBOX_SRF_KNOTS - 1);

	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
		j = BspKnotLastIndexLE(KV, KVLen, u);
		if (!IRIT_APX_EQ(KV[j], u))
		    GlblNewKVs[l++] = u;
	    }
	    else {
		GlblNewKVs[l++] = u;
	    }
	}
	AuxSrf = CagdSrfRefineAtParams(Srf, CAGD_CONST_U_DIR,
				       FALSE, GlblNewKVs, l);

	KV = Srf -> VKnotVector;
	KVLen = Srf -> VLength + Srf -> VOrder;
	for (i = l = 0; i < BBOX_SRF_KNOTS; i++) {
	    int j;
	    CagdRType
		v = VMin + i * (VMax - VMin) / (BBOX_SRF_KNOTS - 1);

	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
		j = BspKnotLastIndexLE(KV, KVLen, v);
		if (!IRIT_APX_EQ(KV[j], v))
		    GlblNewKVs[l++] = v;
	    }
	    else {
		GlblNewKVs[l++] = v;
	    }
	}
	Srf = CpSrf = CagdSrfRefineAtParams(AuxSrf, CAGD_CONST_V_DIR,
					    FALSE, GlblNewKVs, l);

	CagdSrfFree(AuxSrf);
    }
    else
        CpSrf = NULL;

    CagdPointsBBox(Srf -> Points, Srf -> ULength * Srf -> VLength,
		   3, BBox -> Min, BBox -> Max);

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a cagd polygon.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:     To computes its bbox.                                          M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPolygonListBBox, CagdCrvBBox, CagdSrfBBox                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonBBox                                                          M
*****************************************************************************/
void CagdPolygonBBox(const CagdPolygonStruct *Poly, CagdBBoxStruct *BBox)
{
    int i, j, n;

    for (i = 0; i < 3; i++) {
        BBox -> Min[i] = IRIT_INFNTY;
        BBox -> Max[i] = -IRIT_INFNTY;
    }

    switch (Poly -> PolyType) {
	case CAGD_POLYGON_TYPE_TRIANGLE:
	    n = 3;
	    break;
	case CAGD_POLYGON_TYPE_RECTANGLE:
	    n = 4;
	    break;
	case CAGD_POLYGON_TYPE_POLYSTRIP:
        default:
	    n = -1;
	    assert(0);
	    return;

    }

    for (j = 0; j < n; j++) {
        for (i = 0; i < 3; i++) {
	    if (BBox -> Min[i] > Poly -> U.Polygon[j].Pt[i])
		BBox -> Min[i] = Poly -> U.Polygon[j].Pt[i];
	    if (BBox -> Max[i] < Poly -> U.Polygon[j].Pt[i])
		BBox -> Max[i] = Poly -> U.Polygon[j].Pt[i];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of cagd polygons.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Polys:    To computes its bbox.                                          M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPolygonBBox, CagdCrvListBBox, CagdSrfListBBox                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonListBBox                                                      M
*****************************************************************************/
void CagdPolygonListBBox(const CagdPolygonStruct *Polys, CagdBBoxStruct *BBox)
{
    int i, j, n;
    const CagdPolygonStruct *Poly;

    for (i = 0; i < 3; i++) {
        BBox -> Min[i] = IRIT_INFNTY;
        BBox -> Max[i] = -IRIT_INFNTY;
    }

    for (Poly = Polys ; Poly != NULL; Poly = Poly -> Pnext) {
        switch (Poly -> PolyType) {
	    case CAGD_POLYGON_TYPE_TRIANGLE:
	        n = 3;
		break;
	    case CAGD_POLYGON_TYPE_RECTANGLE:
	        n = 4;
		break;
	    case CAGD_POLYGON_TYPE_POLYSTRIP:
            default:
		n = -1;
	        assert(0);
		return;
	}

        for (j = 0; j < n; j++) {
	    for (i = 0; i < 3; i++) {
	        if (BBox -> Min[i] > Poly -> U.Polygon[j].Pt[i])
		    BBox -> Min[i] = Poly -> U.Polygon[j].Pt[i];
		if (BBox -> Max[i] < Poly -> U.Polygon[j].Pt[i])
		    BBox -> Max[i] = Poly -> U.Polygon[j].Pt[i];
	    }
	}
    }
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of freeform surfaces.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:     To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBBox, CagdSrfBBox, CagdPolygonListBBox, CagdTightBBox             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfListBBox, bbox, bounding box                                      M
*****************************************************************************/
void CagdSrfListBBox(const CagdSrfStruct *Srfs, CagdBBoxStruct *BBox)
{
    CAGD_RESET_BBOX(BBox);

    for ( ; Srfs != NULL; Srfs = Srfs -> Pnext) {
	CagdBBoxStruct TmpBBox;

	CagdSrfBBox(Srfs, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a set of control points.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:            To compute bounding box for.                          M
*   Length:            Length of vectors of Points array.                    M
*   Dim:	       Dimensions of points, typically 3 for R^3.	     M
*   BBoxMin, BBoxMax:  Where bounding information is to be saved.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPointsBBox, bbox, bounding box                                       M
*****************************************************************************/
void CagdPointsBBox(CagdRType * const *Points,
		    int Length,
		    int Dim,
		    CagdRType *BBoxMin,
		    CagdRType *BBoxMax)
{
    CagdBType
	MixedWeightsSign = FALSE,
	FirstNegativeWeight = TRUE;
    int i, l;

    for (l = 0; l < Dim; l++) {
        BBoxMin[l] = IRIT_INFNTY;
	BBoxMax[l] = -IRIT_INFNTY;
    }

    if (Points[0] != NULL) {
	for (l = 0; l < Dim; l++) {
	    const CagdRType
		*R = Points[l + 1],
		*WR = Points[0];

	    if (R != NULL) {
		CagdRType
		    Min = BBoxMin[l],
		    Max = BBoxMax[l];

		for (i = 0; i < Length; i++) {
		    CagdRType RTmp;

		    if (*WR <= 0.0) {
			if (FirstNegativeWeight) {
			    /* Have both positive and negative weights? */
			    int j;
			    CagdBType
				Positive = FALSE,
				Negative = FALSE;
			    const CagdRType
				*Weights = Points[0];

			    for (j = 0; j < Length; j++, Weights++) {
				if (*Weights > 0)
				    Positive = TRUE;
				if (*Weights < 0)
				    Negative = TRUE;
			    }
			    MixedWeightsSign = Positive && Negative;
			    FirstNegativeWeight = FALSE;
			}
			
		        if (*WR == 0.0 && *R != 0.0) {
			    if (!GlblIgnoreNonPosWeightBBox) {
			        if (*R > 0.0)
				    Max = IRIT_INFNTY;
				else
				    Min = -IRIT_INFNTY;
			    }
			    WR++;
			    R++;
			    continue;
			}
		        if (*WR == 0.0 && *R == 0.0) {
			    WR++;
			    R++;
			    continue;
			}
			else if (*WR < 0.0) {
			    if (MixedWeightsSign) {
				/* Negative weights can explode! */
			        if (!GlblIgnoreNonPosWeightBBox) {
				    Max = IRIT_LARGE;
				    Min = -Max;
				}
				WR++;
				R++;
				continue;
			    }
			    else {
				/* Continue - all weights are negative! */
			    }
			}
		    }

		    RTmp = *R++ / *WR++;

		    if (Min > RTmp)
			Min = RTmp;
		    if (Max < RTmp)
			Max = RTmp;
		}

		BBoxMin[l] = Min;
		BBoxMax[l] = Max;
	    }
	    else {			   /* Points[l + 1] is not defined. */
		BBoxMin[l] = BBoxMax[l] = 0.0;
	    }
	}
    }
    else {
	for (l = 0; l < Dim; l++) {
	    const CagdRType *R;

	    if ((R = Points[l + 1]) != NULL) {
	        CagdRType Min, Max;
		
		Min = Max = *R++;

		for (i = Length; --i > 0; ) {
		    if (Min > *R)
			Min = *R;
		    if (Max < *R++)
			Max = R[-1];
		}

		BBoxMin[l] = Min;
		BBoxMax[l] = Max;
	    }
	    else {			   /* Points[l + 1] is not defined. */
		BBoxMin[l] = BBoxMax[l] = 0.0;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges (union) two bounding boxes into one, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   DestBBox:    One BBox operand as well as the result.                     M
*   SrcBBox:     Second BBox operand.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeBBox, bbox, bounding box                                        M
*****************************************************************************/
void CagdMergeBBox(CagdBBoxStruct *DestBBox, const CagdBBoxStruct *SrcBBox)
{
    int i;

    for (i = 0; i < 3; i++) {
	if (DestBBox -> Min[i] > SrcBBox -> Min[i])
	    DestBBox -> Min[i] = SrcBBox -> Min[i];
	if (DestBBox -> Max[i] < SrcBBox -> Max[i])
	    DestBBox -> Max[i] = SrcBBox -> Max[i];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a min max bound on a curve in a given axis.			     M
*   The curve is not coerced to anything and the given axis is tested        M
* directly where 0 is the W axis and 1, 2, 3 are the X, Y, Z etc.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To test for minimum/maximum.                                   M
*   Axis:     0 for W, 1 for X, 2 for Y etc.                                 M
*   Min:      Where minimum found value should be place.                     M
*   Max:      Where maximum found value should be place.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvMinMax, bbox, bounding box, minimum, maximum                      M
*****************************************************************************/
void CagdCrvMinMax(const CagdCrvStruct *Crv,
		   int Axis,
		   CagdRType *Min,
		   CagdRType *Max)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Length = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType
	*Pts = Crv -> Points[Axis],
	*WPts = IsNotRational ? NULL : Crv -> Points[0];

    if ((Axis == 0 && IsNotRational) ||	(Axis > MaxCoord))
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_CRV);

    for (i = 0, *Min = IRIT_INFNTY, *Max = -IRIT_INFNTY; i < Length; i++) {
	CagdRType
	    V = WPts ? Pts[i] / WPts[i] : Pts[i];

	if (*Max < V)
	    *Max = V;
	if (*Min > V)
	    *Min = V;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a min max bound on a surface in a given axis.		     M
*   The surface is not coerced to anything and the given axis is tested      M
* directly where 0 is the W axis and 1, 2, 3 are the X, Y, Z etc.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To test for minimum/maximum.                                   M
*   Axis:     0 for W, 1 for X, 2 for Y etc.                                 M
*   Min:      Where minimum found value should be place.                     M
*   Max:      Where maximum found value should be place.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfMinMax, bbox, bounding box, minimum, maximum                      M
*****************************************************************************/
void CagdSrfMinMax(const CagdSrfStruct *Srf,
		   int Axis,
		   CagdRType *Min,
		   CagdRType *Max)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i,
	Length = Srf -> ULength * Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType
	*Pts = Srf -> Points[Axis],
	*WPts = IsNotRational ? NULL : Srf -> Points[0];

    if ((Axis == 0 && IsNotRational) ||	(Axis > MaxCoord))
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SRF);

    for (i = 0, *Min = IRIT_INFNTY, *Max = -IRIT_INFNTY; i < Length; i++) {
	CagdRType
	    V = WPts ? Pts[i] / WPts[i] : Pts[i];

	if (*Max < V)
	    *Max = V;
	if (*Min > V)
	    *Min = V;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if a circle is contained in the given prescribed circle.           M
*   Test is conducted by verifying that all the control points of Crv are    M
* inside the circle.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           Curve to test for containment in the circle.              M
*   Center:        Center of the circle to test against.                     M
*   Radius:        Radius of the circle to test against.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if Crv is indeed inside the circle, FALSE otherwise.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsCrvInsideCH, MvarIsCrvInsideCirc				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsCrvInsideCirc                                                      M
*****************************************************************************/
int CagdIsCrvInsideCirc(const CagdCrvStruct *Crv,
			const CagdRType Center[2],
			CagdRType Radius)
{
    int i;
    CagdRType
	*XPts = Crv -> Points[1],
	*YPts = Crv -> Points[2],
	*WPts = Crv -> Points[0];

    /* Check if all control points are inside the circle. */
    for (i = 0; i < Crv -> Length; i++) {
	CagdRType
	    x = WPts == NULL ? *XPts++ : *XPts++ / *WPts,
	    y = WPts == NULL ? *YPts++ : *YPts++ / *WPts++;

	if (IRIT_SQR(x - Center[0]) + IRIT_SQR(y - Center[1]) >
							     IRIT_SQR(Radius))
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Identifying whether the given curve is inside the convex hull.           M
*   Test is conducted by verifying that all the control points of Crv are    M
* inside the convex hull.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          The input curve.                                           M
*   CHPts:        Points in convex hull in GMR2Struct form.                  M
*   NumCHPts:     Number of Points in the convex hull.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsCrvInsideCirc, GMConvexHull                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsCrvInsideCH                                                        M
*****************************************************************************/
int CagdIsCrvInsideCH(const CagdCrvStruct *Crv,
		      const IrtE2PtStruct *CHPts,
		      int NumCHPts)
{
    int i, l, 
        Length = Crv -> Length;
    CagdRType
	RefSignedArea = 0.0;

    /* Check if all control points are inside the Convex Hull. */
    for (l = 0; l < NumCHPts; l++) {
        CagdRType SignedArea, CrvPt[2];
	CagdRType const
	    *CHPt1 = CHPts[l].Pt,
	    *CHPt2 = CHPts[(l + 1) % NumCHPts].Pt,
	    *XPts = Crv -> Points[1],
	    *YPts = Crv -> Points[2],
	    *WPts = Crv -> Points[0];

	if (l == 0) {
	    /* Initialize the reference sign. */
	    CrvPt[0] = WPts == NULL ? *XPts : *XPts / *WPts;
	    CrvPt[1] = WPts == NULL ? *YPts : *YPts / *WPts;
	    RefSignedArea = GMAreaOfTriangle(CHPt1, CHPt2, CrvPt);
	}

	for (i = 0; i < Length; i++) {
	    CrvPt[0] = WPts == NULL ? *XPts++ : *XPts++ / *WPts;
	    CrvPt[1] = WPts == NULL ? *YPts++ : *YPts++ / *WPts++;
	    SignedArea = GMAreaOfTriangle(CHPt1, CHPt2, CrvPt);

	    if (IRIT_APX_EQ_EPS(RefSignedArea, 0.0, IRIT_UEPS))
		RefSignedArea = SignedArea;
	    else
	        if (RefSignedArea * SignedArea < 0.0)
		    return FALSE;
	}
    }

    return TRUE;
}
