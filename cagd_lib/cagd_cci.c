/******************************************************************************
* Cagd_cci.c - curve curve intersection routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 96.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define X_COORD(i) (WPoints == NULL ? XPoints[i] : XPoints[i] / WPoints[i])
#define Y_COORD(i) (WPoints == NULL ? YPoints[i] : YPoints[i] / WPoints[i])
#define ZERO_IRIT_APX_EQ(x, y)		(IRIT_FABS((x) - (y)) < Eps * 10)
#define CAGD_POLE_SPLIT_EPS		1e-10
#define CAGD_POLE_SPLIT_OUTREACH	1000
IRIT_STATIC_DATA CagdPtStruct
    *GlblInterList = NULL;
IRIT_STATIC_DATA CagdCrvStruct
    *GlblTanCrv1 = NULL,
    *GlblTanCrv2 = NULL;

static void CagdCrvCrvInterAux(const CagdCrvStruct *Crv1,
			       const CagdCrvStruct *Crv2,
			       CagdRType Eps);
static int CagdCrv1OutBoundingWedge2(const CagdCrvStruct *Crv1,
				     const CagdCrvStruct *Crv2,
				     CagdVType ConeDir2,
				     CagdRType AngularSpan2,
				     CagdRType Eps);
static CagdBType CagdCrvCrvInterNumer(const CagdCrvStruct *Crv1,
				      const CagdCrvStruct *Crv2,
				      CagdRType Eps);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a curve, computes the angular span of its tangent field, in XY     M
* plane.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Curve to consider                                          M
*   ConeDir:      General, median, direction of tangent field, in XY plane.  M
*   AngularSpan:  Maximal deviation of tangent field from Dir, in radians.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if angular span of curve is less than 180 degrees,    M
*	    FALSE otherwise.  In the later case, Dir and Angle are invalid.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvCrvInter                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvTanAngularSpan                                                    M
*****************************************************************************/
CagdBType CagdCrvTanAngularSpan(const CagdCrvStruct *Crv,
				CagdVType ConeDir,
				CagdRType *AngularSpan)
{
    int i,
	Length = Crv -> Length;
    CagdRType R, Cr, Sr,
	AngularLSpan = 1.0,
	AngularRSpan = 1.0,
	*WPoints = Crv -> Points[0],
	*XPoints = Crv -> Points[1],
	*YPoints = Crv -> Points[2];
    CagdVType V, Dir;

    Dir[0] = X_COORD(Length - 1) - X_COORD(0);
    Dir[1] = Y_COORD(Length - 1) - Y_COORD(0);

    if ((R = IRIT_PT2D_SQR_LENGTH(Dir)) == 0.0)
        return FALSE;
    R = 1.0 / sqrt(R);
    IRIT_PT2D_SCALE(Dir, R);

    for (i = 1; i < Length; i++) {
	V[0] = X_COORD(i) - X_COORD(i - 1);
	V[1] = Y_COORD(i) - Y_COORD(i - 1);

	if ((R = IRIT_PT2D_SQR_LENGTH(V)) == 0.0)
	    continue;
	R = 1.0 / sqrt(R);
	IRIT_PT2D_SCALE(V, R);

	if ((R = IRIT_DOT_PROD_2D(V, Dir)) < 0.0)
	    return FALSE;

	if (IRIT_CROSS_PROD_2D(V, Dir) > 0.0) {
	    if (AngularLSpan > R)
	        AngularLSpan = R;
	}
	else {
	    if (AngularRSpan > R)
	        AngularRSpan = R;
	}
    }

    AngularLSpan = IRIT_BOUND(AngularLSpan, 0.0, 1.0);
    AngularLSpan = acos(AngularLSpan);

    AngularRSpan = IRIT_BOUND(AngularRSpan, 0.0, 1.0);
    AngularRSpan = acos(AngularRSpan);

    *AngularSpan = (AngularRSpan + AngularLSpan) * 0.5;

    R = (AngularRSpan - AngularLSpan) * 0.5;
    Cr = cos(R);
    Sr = sin(R);

    ConeDir[0] = Dir[0] * Cr - Dir[1] * Sr;
    ConeDir[1] = Dir[0] * Sr + Dir[1] * Cr;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a line in the XY Plane, finds a bound on the minimal     M
* signed distance between the two.  Returns positive/negative minimal        M
* expected distance if curve on either side of the line or zero if might     M
* intersect.								     M
*   Computation is performed by measuring the signed distance between the    M
* line and all control points of the curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Planar curve to compute its signed distance to the line.     M
		Assumed to be a Bezier or a Bspline curve.	             M
*   Line:       Line Equations, in the XY plane.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  Zero if might intersect.  Otherwise, a bound on the minimal  M
8		possible distance, signed.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvLine, SymbLclDistCrvLine					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDistCrvLine, curve line distance                                     M
*****************************************************************************/
CagdRType CagdDistCrvLine(const CagdCrvStruct *Crv, CagdLType Line)
{
    int i,
	Len = Crv -> Length;
    CagdRType
	MinPos = IRIT_INFNTY,
	MinNeg = -IRIT_INFNTY,
	*WPts = Crv -> Points[0],
	*XPts = Crv -> Points[1],
	*YPts = Crv -> Points[2];

    if (WPts != NULL) {
        for (i = 0; i < Len; i++) {
	    CagdRType
		R = Line[0] * *XPts++ + Line[1] * *YPts++ + Line[2] * *WPts++;

	    if (R > 0.0) {
		if (MinNeg != IRIT_INFNTY)
		    return 0.0;
		MinPos = IRIT_MIN(MinPos, R);
	    }
	    else if (R < 0.0) {
		if (MinPos != IRIT_INFNTY)
		    return 0.0;
		MinNeg = IRIT_MAX(MinNeg, R);
	    }
	    else
	        return 0.0;
	}
    }
    else {
        for (i = 0; i < Len; i++) {
	    CagdRType
		R = Line[0] * *XPts++ + Line[1] * *YPts++ + Line[2];

	    if (R > 0.0) {
		if (MinNeg != -IRIT_INFNTY)
		    return 0.0;
		MinPos = IRIT_MIN(MinPos, R);
	    }
	    else if (R < 0.0) {
		if (MinPos != IRIT_INFNTY)
		    return 0.0;
		MinNeg = IRIT_MAX(MinNeg, R);
	    }
	    else
	        return 0.0;
	}
    }

    if (MinNeg != -IRIT_INFNTY)
        return MinNeg;
    else
        return MinPos;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection points, if any, of the two given curves.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curves to compute their intersection points.            M
*   Eps:         Accuracy of computation.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   List of intersection points.  Each points would        M
*	contain (u1, u2, 0.0).						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTanAngularSpan, SymbCrvCrvInter, UserSrfSrfInter,                 M
*   CagdCrvCrvInterArrangment						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCrvInter, cci                                                     M
*****************************************************************************/
CagdPtStruct *CagdCrvCrvInter(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2,
			      CagdRType Eps)
{
    CagdCrvStruct *Crv1All, *Crv2All, *CpCrv1, *CpCrv2;

    if (CAGD_NUM_OF_PT_COORD(Crv1 -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(Crv2 -> PType) < 2) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_PT_TYPE);
	return NULL;
    }

    CagdInsertInterPointInit();  /* Reset the list. */

    if (CAGD_IS_BEZIER_CRV(Crv1))
	Crv1 = CpCrv1 = CagdCnvrtBzr2BspCrv(Crv1);
    else
        CpCrv1 = NULL;

    if (CAGD_IS_BEZIER_CRV(Crv2))
	Crv2 = CpCrv2 = CagdCnvrtBzr2BspCrv(Crv2);
    else
        CpCrv2 = NULL;

    Crv1All = SymbCrvSplitPoleParams(Crv1, CAGD_POLE_SPLIT_EPS,
				     CAGD_POLE_SPLIT_OUTREACH);
    Crv2All = SymbCrvSplitPoleParams(Crv2, CAGD_POLE_SPLIT_EPS,
				     CAGD_POLE_SPLIT_OUTREACH);
    if (CpCrv1 != NULL)
	CagdCrvFree(CpCrv1);
    if (CpCrv2 != NULL)
	CagdCrvFree(CpCrv2);

    for (Crv1 = Crv1All; Crv1 != NULL; Crv1 = Crv1 -> Pnext) {
        for (Crv2 = Crv2All; Crv2 != NULL; Crv2 = Crv2 -> Pnext) {
	    GlblTanCrv1 = CagdCrvDerive(Crv1);
	    GlblTanCrv2 = CagdCrvDerive(Crv2);

	    CagdCrvCrvInterAux(Crv1, Crv2, Eps);

	    CagdCrvFree(GlblTanCrv1);
	    CagdCrvFree(GlblTanCrv2);
	}
    }

    CagdCrvFreeList(Crv1All);
    CagdCrvFreeList(Crv2All);

    return CagdInsertInterPointInit(); /* Return the outcome. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxliary function of CagdCrvCrvInter.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:  Two curves to compute their intersection points.            *
*   Eps:         Accuracy of computation.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:   List of intersection points.  Each points would        *
*	contain (u1, u2, 0.0).						     *
*****************************************************************************/
static void CagdCrvCrvInterAux(const CagdCrvStruct *Crv1,
			       const CagdCrvStruct *Crv2,
			       CagdRType Eps)
{
    CagdBBoxStruct BBox1, BBox2;
    CagdVType Dir1, Dir2;
    CagdRType Angle1, Angle2, TMin1, TMax1, TMin2, TMax2;
    CagdCrvStruct *Crv1a, *Crv1b, *Crv2a, *Crv2b;

    CagdCrvBBox(Crv1, &BBox1);
    CagdCrvBBox(Crv2, &BBox2);

    if (BBox1.Max[0] < BBox2.Min[0] ||
	BBox1.Max[1] < BBox2.Min[1] ||
	BBox2.Max[0] < BBox1.Min[0] ||
	BBox2.Max[1] < BBox1.Min[1]) {
	/* The bounding boxes do not overlap - return. */
	return;
    }

    if (CagdCrvTanAngularSpan(Crv1, Dir1, &Angle1) &&
	CagdCrvTanAngularSpan(Crv2, Dir2, &Angle2)) {
        CagdRType
	    d = IRIT_DOT_PROD_2D(Dir1, Dir2);

	if (acos(IRIT_FABS(d)) > Angle1 + Angle2) {
	    /* Tangent field's cones do not overlap - only one intersection */
	    /* occur between the two curves - try to find it numerically.   */
	    if (CagdCrvCrvInterNumer(Crv1, Crv2, Eps))
		return;
	}

	if (CagdCrv1OutBoundingWedge2(Crv1, Crv2, Dir2, Angle2, Eps) ||
	    CagdCrv1OutBoundingWedge2(Crv2, Crv1, Dir1, Angle1, Eps))
	    return;                       /* No overlap between the curves. */
    }

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    /* Subdivide the two curves and recurse. */
    if (TMax1 - TMin1 < Eps || TMax2 - TMin2 < Eps) {
	/* Failed in the numerical approach - stop the subdivision! */
	CagdInsertInterPoints((TMin1 + TMax1) * 0.5, (TMin2 + TMax2) * 0.5, Eps);
	return;
    }

    Crv1a = CagdCrvSubdivAtParam(Crv1, (TMin1 + TMax1) * 0.5);
    Crv1b = Crv1a -> Pnext;
    Crv1a -> Pnext = NULL;
    Crv2a = CagdCrvSubdivAtParam(Crv2, (TMin2 + TMax2) * 0.5);
    Crv2b = Crv2a -> Pnext;
    Crv2a -> Pnext = NULL;

    CagdCrvCrvInterAux(Crv1a, Crv2a, Eps);
    CagdCrvCrvInterAux(Crv1a, Crv2b, Eps);
    CagdCrvCrvInterAux(Crv1b, Crv2a, Eps);
    CagdCrvCrvInterAux(Crv1b, Crv2b, Eps);

    CagdCrvFree(Crv1a);
    CagdCrvFree(Crv1b);
    CagdCrvFree(Crv2a);
    CagdCrvFree(Crv2b);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if Crv1 is outside the bounding wedge of Crv2.  The bounding wedge  *
* is the domain bounded by the intersection of the two bounding cones placed *
* at the end points of the curve at has the shape of a parallelogram.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1:         First curves to test if out of the bounding wedge of Crv2. *
*   Crv2:         Second curve its bounding wedge we use.		     *
*   ConeDir2:     General, median, direction of tangent field of Crv2, in    *
*		  the XY plane.						     *
*   AngularSpan2: Maximal deviation of tangent field from Dir, in radians.   *
*   Eps:          Accuracy of computation.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   static int:   TRUE if outside the wedge, FALSE if cannot tell.           *
*****************************************************************************/
static int CagdCrv1OutBoundingWedge2(const CagdCrvStruct *Crv1,
				     const CagdCrvStruct *Crv2,
				     CagdVType ConeDir2,
				     CagdRType AngularSpan2,
				     CagdRType Eps)
{
    CagdPType PStart, PEnd;
    CagdVType LDir, RDir;
    CagdRType *R, R1, R2, TMin, TMax,
	Cr = cos(AngularSpan2),
	Sr = sin(AngularSpan2);
    CagdLType Line;

    LDir[0] = ConeDir2[0] * Cr - ConeDir2[1] * Sr;
    LDir[1] = ConeDir2[0] * Sr + ConeDir2[1] * Cr;

    RDir[0] =  ConeDir2[0] * Cr + ConeDir2[1] * Sr;
    RDir[1] = -ConeDir2[0] * Sr + ConeDir2[1] * Cr;

    CagdCrvDomain(Crv2, &TMin, &TMax);

    R = CagdCrvEval(Crv2, TMin);
    CagdCoerceToE2(PStart, &R, -1, Crv2 -> PType);
    R = CagdCrvEval(Crv2, TMax);
    CagdCoerceToE2(PEnd, &R, -1, Crv2 -> PType);

    /* We now have four lines to check against. */
    Line[0] = -LDir[1];
    Line[1] = LDir[0];
    Line[2] = -Line[0] * PStart[0] - Line[1] * PStart[1];

    R2 = IRIT_DOT_PROD_2D(Line, PEnd) + Line[2];    /* Side of this curve, Crv2. */
    R1 = CagdDistCrvLine(Crv1, Line);
    if (R1 * R2 < 0.0 && IRIT_FABS(R1) > Eps)
	return TRUE;

    Line[2] = -Line[0] * PEnd[0] - Line[1] * PEnd[1];

    R2 = IRIT_DOT_PROD_2D(Line, PStart) + Line[2];  /* Side of this curve, Crv2. */
    R1 = CagdDistCrvLine(Crv1, Line);
    if (R1 * R2 < 0.0 && IRIT_FABS(R1) > Eps)
	return TRUE;

    Line[0] = -RDir[1];
    Line[1] = RDir[0];
    Line[2] = -Line[0] * PStart[0] - Line[1] * PStart[1];

    R2 = IRIT_DOT_PROD_2D(Line, PEnd) + Line[2];    /* Side of this curve, Crv2. */
    R1 = CagdDistCrvLine(Crv1, Line);
    if (R1 * R2 < 0.0 && IRIT_FABS(R1) > Eps)
	return TRUE;

    Line[2] = -Line[0] * PEnd[0] - Line[1] * PEnd[1];

    R2 = IRIT_DOT_PROD_2D(Line, PStart) + Line[2];  /* Side of this curve, Crv2. */
    R1 = CagdDistCrvLine(Crv1, Line);
    if (R1 * R2 < 0.0 && IRIT_FABS(R1) > Eps)
	return TRUE;

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Uses Newton raphson to try and converge on an intersection points of     *
* two curves.  The two curves are assumed to have at most one intersection.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:  Two curves to compute their intersection points.            *
*   Eps:         Accuracy of computation.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:   TRUE if found intersection, FALSE otherwise.                *
*****************************************************************************/
static CagdBType CagdCrvCrvInterNumer(const CagdCrvStruct *Crv1,
				      const CagdCrvStruct *Crv2,
				      CagdRType Eps)
{
    CagdRType t1, t2, TMin1, TMax1, TMin2, TMax2,
	CrntDist = IRIT_INFNTY;
    CagdPType Pt1, Pt2;
    CagdVType Tan1, Tan2;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    t1 = (TMin1 + TMax1) * 0.5;
    t2 = (TMin2 + TMax2) * 0.5;

    Pt1[2] = Pt2[2] = Tan1[2] = Tan2[2] = 0.0;

    while (TRUE) {
	CagdRType Dist, *R, Inter1Param, Inter2Param;
	CagdPType Inter1, Inter2;

	R = CagdCrvEval(Crv1, t1);
	CagdCoerceToE2(Pt1, &R, -1, Crv1 -> PType);

	R = CagdCrvEval(GlblTanCrv1, t1);
	CagdCoerceToE2(Tan1, &R, -1, GlblTanCrv1 -> PType);

	R = CagdCrvEval(Crv2, t2);
	CagdCoerceToE2(Pt2, &R, -1, Crv2 -> PType);

	R = CagdCrvEval(GlblTanCrv2, t2);
	CagdCoerceToE2(Tan2, &R, -1, GlblTanCrv2 -> PType);

	if ((Dist = IRIT_PT2D_DIST(Pt1, Pt2)) < Eps) {
	    /* Done - found the intersection points. */
	    CagdInsertInterPoints(t1, t2, Eps);
	    return TRUE;
	}
	else if (Dist * 1.1 > CrntDist) {/* Failed to significantly improve. */
	    return FALSE;
	}
	else {
	    CrntDist = Dist;
	}

	if (!GM2PointsFromLineLine(Pt1, Tan1, Pt2, Tan2,
				   Inter1, &Inter1Param,
				   Inter2, &Inter2Param))
	    return FALSE;

	t1 += Inter1Param;
	t1 = IRIT_BOUND(t1, TMin1, TMax1);

	t2 += Inter2Param;
	t2 = IRIT_BOUND(t2, TMin2, TMax2);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset the list of intersection points to insert & returns the old list.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  Old list of points found on the intersections list.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdInsertInterPoints                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdInsertInterPointInit                                                 M
*****************************************************************************/
CagdPtStruct *CagdInsertInterPointInit(void)
{
    CagdPtStruct
        *InterList = GlblInterList;

    GlblInterList = NULL;

    return InterList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert t1/t2 values into GlblInterList, provided no equal t1/t2 value      M
* exists already in the list. List is in ascending order with respect to t1. M
*                                                                            *
* PARAMETERS:                                                                M
*   t1, t2:     New parameter values to insert to global GlblInterList list. M
*   Eps:        Accuracy of insertion computation.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdInsertInterPointInit                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdInsertInterPoints                                                    M
*****************************************************************************/
void CagdInsertInterPoints(CagdRType t1, CagdRType t2, CagdRType Eps)
{
    CagdPtStruct *PtTmp, *PtLast, *Pt;

    Pt = CagdPtNew();
    Pt -> Pt[0] = t1;
    Pt -> Pt[1] = t2;
    Pt -> Pt[2] = 0.0;

    if (GlblInterList) {
	for (PtTmp = GlblInterList, PtLast = NULL;
	     PtTmp != NULL;
	     PtLast = PtTmp, PtTmp = PtTmp -> Pnext) {
	    if (ZERO_IRIT_APX_EQ(PtTmp -> Pt[0], t1) &&
		ZERO_IRIT_APX_EQ(PtTmp -> Pt[1], t2)) {
	        IritFree(Pt);
		return;
	    }
	    if (PtTmp -> Pt[0] > t1)
	        break;
	}
	if (PtTmp) {
	    /* Insert the new point in the middle of the list. */
	    Pt -> Pnext = PtTmp;
	    if (PtLast)
		PtLast -> Pnext = Pt;
	    else
		GlblInterList = Pt;
	}
	else {
	    /* Insert the new point as the last point in the list. */
	    PtLast -> Pnext = Pt;
	}
    }
    else
        GlblInterList = Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersections in the plane between all given curves and     M
* optionally split the curves at those parameters.  If the curves are not    M
* split, a list of intersection parameters is returned in an attribute       M
* called "InterPts", holding the parameter in X coordinate of pts.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ArngCrvs:   Curves to intersect. 					     M
*   SplitCrvs:  TRUE, to also split the curves at detected intersections.    M
*   Eps:        Tolerance of CCI computations.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  (Sub) curves as splitted due to cci's, or identical    M
*		      curves with "InterPts" attributes.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvCrvInter, SymbCrvCrvInter, CagdCrvsLowerEnvelop                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCrvInterArrangment, cci                                           M
*****************************************************************************/
CagdCrvStruct *CagdCrvCrvInterArrangment(const CagdCrvStruct *ArngCrvs,
					 CagdBType SplitCrvs,
					 CagdRType Eps)
{
    CagdCrvStruct *Crv1, *Crv2,
	*Crvs = CagdCrvCopyList(ArngCrvs);
    CagdPtStruct *IPts, *IPts1, *IPts2;

    /* Intersect all curves against themselves and keep the intersection   */
    /* locations on the curves as attributes.				   */
    for (Crv1 = Crvs; Crv1 != NULL; Crv1 = Crv1 -> Pnext) {
        for (Crv2 = Crv1 -> Pnext; Crv2 != NULL; Crv2 = Crv2 -> Pnext) {
	    if ((IPts1 = CagdCrvCrvInter(Crv1, Crv2, Eps)) != NULL) {
	        /* Make a copy for second curve and swap parameter values. */
	        IPts2 = CagdPtCopyList(IPts1);
		for (IPts = IPts2; IPts != NULL; IPts = IPts -> Pnext)
		    IRIT_SWAP(CagdRType, IPts -> Pt[1], IPts -> Pt[0]);

		/* Append in both intersection lists into the curves. */
		AttrSetRefPtrAttrib(&Crv1 -> Attr, "InterPts",
			CagdListAppend(IPts1,
				       AttrGetRefPtrAttrib(Crv1 -> Attr,
							   "InterPts")));
		AttrSetRefPtrAttrib(&Crv2 -> Attr, "InterPts",
			CagdListAppend(IPts2,
				       AttrGetRefPtrAttrib(Crv2 -> Attr,
							   "InterPts")));
	    }
	}
    }

    if (SplitCrvs) {
        CagdCrvStruct
	    *CrvsSplitted = NULL;

        /* Time to split all curves at the intersection points. */
        while (Crvs != NULL) {
	    IRIT_LIST_POP(Crv1, Crvs);

	    if ((IPts = (CagdPtStruct *) AttrGetRefPtrAttrib(Crv1 -> Attr,
							"InterPts")) != NULL) {
	        int Proximity;

		IPts = CagdPtsSortAxis(IPts, 1);

		CrvsSplitted =
		  CagdListAppend(CagdCrvSubdivAtParams(Crv1, IPts, Eps * 10,
						       &Proximity),
				   CrvsSplitted);

		CagdPtFreeList(IPts);
		CagdCrvFree(Crv1);
	    }
	    else
	        IRIT_LIST_PUSH(Crv1, CrvsSplitted);
	}

	return CrvsSplitted;
    }
    else {
        return Crvs;
    }
}
