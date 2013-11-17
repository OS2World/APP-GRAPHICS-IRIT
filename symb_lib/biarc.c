/******************************************************************************
* Bi_arc.c - piecewise biarc approximation of freeform curves.  	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 03	  				      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"
#include "iritprsr.h"
#include "allocate.h"

#define SYMB_BIARC_COLIN_VEC		0.9999999
#define SYMB_BIARC_INFLECTION_TOL	1e-6

IRIT_STATIC_DATA CagdCrvStruct
    *GlblDCrv = NULL;

static SymbArcStruct *SymbCrvBiArcApproxAux(CagdCrvStruct *Crv,
					    CagdRType Tolerance);
static SymbArcStruct *SymbCrvBiArcApproxAux2(CagdCrvStruct *Crv,
					     CagdRType Tolerance);
static int EstimateDistCrvCenter(CagdCrvStruct *Crv,
				 CagdPType Center,
				 CagdRType Radius,
				 CagdRType Tolerance);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a piecewise biarc approximation to given freeform planar curve. M
*   The following steps are performed during this approximation process:     M
* 1. The curve is split at inflection points, if any.			     M
* 2. Each convex/concave curve region:					     M
*    a. Fit the curve region with a G^1 continuous biarc that is tangent to  M
*       the curve's region at the region's end points.			     M
*    b. If fit is good enough, we stop.  Otherwise subdivide region into two M
*	and recursively invoke step 2 on both halfs.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:       2D Curve to approximate using piecewise biarcs.              M
*   Tolerance:  Of approximation.					     M
*   MaxAngle:   Of an arc in the output set.  In no way it will be more than M
*		or equal to 180 degrees.  In Degrees.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbArcStruct *:  List of arcs approximating Crv to within Tolerance.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2DInflectionPts                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvBiArcApprox                                                       M
*****************************************************************************/
SymbArcStruct *SymbCrvBiArcApprox(const CagdCrvStruct *CCrv,
				  CagdRType Tolerance,
				  CagdRType MaxAngle)
{
    CagdBType
	OldCagdInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdPtStruct *Pt,
	*Pts = CCrv -> Order > 3 ? SymbCrv2DInflectionPts(CCrv,
						   SYMB_BIARC_INFLECTION_TOL)
			        : NULL;
    SymbArcStruct *Arcs, *Arc,
	*AllArcs = NULL;
    CagdCrvStruct *Crv;

    GlblDCrv = CagdCrvDerive(CCrv);

    if (CAGD_IS_BSPLINE_CRV(CCrv))
	Crv = CagdCrvCopy(CCrv);
    else
        Crv = CagdCnvrtBzr2BspCrv(CCrv);       /* Keep parameter values. */

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	CagdRType
	    *KV = Crv -> KnotVector;
	int Len = Crv -> Order + Crv -> Length,
	    i1 = BspKnotLastIndexLE(KV, Len, Pt -> Pt[0]),
	    i2 = BspKnotFirstIndexG(KV, Len, Pt -> Pt[0]);
	CagdCrvStruct *Crvs;

	if (IRIT_APX_EQ(Pt -> Pt[0], KV[i1]))
	    Pt -> Pt[0] = KV[i1];
	if (IRIT_APX_EQ(Pt -> Pt[0], KV[i2]))
	    Pt -> Pt[0] = KV[i2];

	Crvs = CagdCrvSubdivAtParam(Crv, Pt -> Pt[0]);

	Arcs = SymbCrvBiArcApproxAux(Crvs, Tolerance);
	
	AllArcs = (SymbArcStruct *) CagdListAppend(AllArcs, Arcs);

	CagdCrvFree(Crv);
	Crv = Crvs -> Pnext;
	CagdCrvFree(Crvs);
    }
    CagdPtFreeList(Pts);

    Arcs = SymbCrvBiArcApproxAux(Crv, Tolerance);
	
    AllArcs = (SymbArcStruct *) CagdListAppend(AllArcs, Arcs);

    CagdCrvFree(Crv);

    BspMultComputationMethod(OldCagdInterpFlag);

    CagdCrvFree(GlblDCrv);
    GlblDCrv = NULL;

    /* Make sure all arcs span less than MaxAngle degrees. */
    if (MaxAngle >= 180)
	MaxAngle = 179;
    MaxAngle = cos(IRIT_DEG2RAD(MaxAngle));

    for (Arc = AllArcs; Arc != NULL; ) {
	if (Arc -> Arc) {
	    CagdRType Rad1, Rad;
	    CagdVType V1, V2;
	    CagdPType PMid;

	    IRIT_VEC_SUB(V1, Arc -> Pt1, Arc -> Cntr);
	    IRIT_VEC_SUB(V2, Arc -> Pt2, Arc -> Cntr);
	    Rad = IRIT_VEC_LENGTH(V1);
	    Rad1 = 1.0 / Rad;
	    IRIT_VEC_SCALE(V1, Rad1);
	    IRIT_VEC_SCALE(V2, Rad1);

	    if (IRIT_DOT_PROD(V1, V2) < MaxAngle) {
		SymbArcStruct
		    *Arc2 = SymbArcCopy(Arc);

		/* Split this arc into two. */
		IRIT_VEC_ADD(PMid, V1, V2);
		IRIT_VEC_NORMALIZE(PMid);
		IRIT_VEC_SCALE(PMid, Rad);
		IRIT_PT_ADD(Arc -> Pt2, Arc -> Cntr, PMid);
		IRIT_PT_ADD(Arc2 -> Pt1, Arc -> Cntr, PMid);

	        Arc2 -> Pnext = Arc -> Pnext;
		Arc -> Pnext = Arc2;
	    }
	    else
	        Arc = Arc -> Pnext;
	}
	else
	    Arc = Arc -> Pnext;
    }

    return AllArcs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split the curve at all its potential C^1 discontinuities as a 1st        *
* step.                                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         Inflection free curve to fit biarcs to.                     *
*   Tolerance:   Of approximation.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   SymbArcStruct *:  List of arcs approximating Crv to within Tolerance.    *
*****************************************************************************/
static SymbArcStruct *SymbCrvBiArcApproxAux(CagdCrvStruct *Crv,
					    CagdRType Tolerance)
{
    int i, n;
    CagdRType
	t = -IRIT_INFNTY,
	*KVC1DiscPotential = BspKnotAllC1Discont(Crv -> KnotVector,
						 Crv -> Order,
						 Crv -> Length, &n);

    for (i = 0; i < n; i++) {
        int Idx = BspKnotLastIndexL(Crv -> KnotVector,
				    Crv -> Length + Crv -> Order,
				    KVC1DiscPotential[i]) + 1;

	if (BspCrvMeshC1Continuous(Crv, Idx))
	    break;
    }

    if (i < n) {
	/* Found a real C1 discontinuity - break the curve there. */
        t = KVC1DiscPotential[i];
    }
    IritFree(KVC1DiscPotential);

    if (i < n) {
        CagdCrvStruct
	    *Crvs = CagdCrvSubdivAtParam(Crv, t);
	SymbArcStruct *RetVal;

	RetVal = (SymbArcStruct *)
	    CagdListAppend(SymbCrvBiArcApproxAux(Crvs, Tolerance),
			   SymbCrvBiArcApproxAux(Crvs -> Pnext, Tolerance));

	CagdCrvFreeList(Crvs);

	return RetVal;	
    }
    else {
        if (CagdEstimateCrvCollinearity(Crv) < IRIT_EPS) {
	    CagdRType *R, TMin, TMax;
	    SymbArcStruct *Arc;

	    /* This curve is a line - return a line. */
	    Arc = SymbArcNew(FALSE);

	    CagdCrvDomain(Crv, &TMin, &TMax);
	    R = CagdCrvEval(Crv, TMin);
	    CagdCoerceToE3(Arc -> Pt1, &R, -1, Crv -> PType);
	    R = CagdCrvEval(Crv, TMax);
	    CagdCoerceToE3(Arc -> Pt2, &R, -1, Crv -> PType);

	    IRIT_PT_RESET(Arc -> Cntr);
	    return Arc;
	}
	else
	    return SymbCrvBiArcApproxAux2(Crv, Tolerance);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Recursively compute a biarc fit to the given inflection-free curve.      *
*   Let:								     *
* + curve end points' be P1 and P3, with (nonnormalized) tangents V1 and V3. *
* + the first arc end points be P1 and P2 and its middle point P12.	     *
* + the second arc end points be P2 and P3 and its middle point P23.	     *
*                                                                            *
*   Then we have the following condition to ensure G^1 continuity between    *
* the two biarcs - we are going to have two control polygons with 3 points   *
* each.  The first one is from P1 through P12 to P2 and the second control   *
* polygon is from P2 through P23 to P3:					     *
*                                                                            *
* ||(P1 + V1 a) - (P3 - V3 a)|| = ||V1 a|| + ||V3 a||, for some scalar a.    *
*                                                                            *
* or                                                                         *
*                                                                            *
* ||(P1 - P3) + (V1 + V3) a|| = (||V1|| + ||V3||) a                          *
*                                                                            *
* or                                                                         *
*                                                                            *
* ||(P1 - P3)||^2 + ||(V1 + V3)||^2 a^2 +                                    *
*   2 ||(P1 - P3)|| ||(V1 + V3)|| cos(beta) a = (||V1|| + ||V3||)^2 a^2,     *
* where beta is the angle between vectors (P1 - P3) and (V1 + V3).	     *
*									     *
*   The curve is then split in the middle and the distances of the two       *
* pieces to the two centers of the two arcs is computed and compared against *
* the radii of the two arcs.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         Inflection/C^1 discontinuities free curve to fit biarcs to. *
*   Tolerance:   Of approximation.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   SymbArcStruct *:  List of arcs approximating Crv to within Tolerance.    *
*****************************************************************************/
static SymbArcStruct *SymbCrvBiArcApproxAux2(CagdCrvStruct *Crv,
					     CagdRType Tolerance)
{
    int FirstIsLine, SecondIsLine,
	Split = FALSE;
    CagdRType a, *R, TMin, TMax, LenV13, LenT13, CosBeta, A, B, C, Disc,
	LenTan1, LenTan3, t, Rad1, Rad2;
    CagdPType Pt1, Pt2, Pt3, Pt12, Pt23, Cntr1, Cntr2;
    CagdVType Tan1, Tan3, Tan2, V13, T13;
    CagdCrvStruct *Crvs;
    SymbArcStruct *Arcs1, *Arcs2;

    CagdCrvDomain(Crv, &TMin, &TMax);
    R = CagdCrvEval(Crv, TMin);
    CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);
    R = CagdCrvEval(Crv, TMax);
    CagdCoerceToE3(Pt3, &R, -1, Crv -> PType);

    R = CagdCrvEval(GlblDCrv, TMin);
    CagdCoerceToE3(Tan1, &R, -1, GlblDCrv -> PType);
    R = CagdCrvEval(GlblDCrv, TMax);
    CagdCoerceToE3(Tan3, &R, -1, GlblDCrv -> PType);

    IRIT_PT_SUB(V13, Pt1, Pt3);

    LenTan1 = IRIT_VEC_LENGTH(Tan1);
    LenTan3 = IRIT_VEC_LENGTH(Tan3);
    LenV13 = IRIT_VEC_LENGTH(V13);

    if (IRIT_DOT_PROD(Tan1, Tan3) > SYMB_BIARC_COLIN_VEC * LenTan1 * LenTan3 &&
	IRIT_DOT_PROD(Tan1, V13) > SYMB_BIARC_COLIN_VEC * LenTan1 * LenV13) {
	if (CagdEstimateCrvCollinearity(Crv) < IRIT_EPS) {
	    /* This segment is a line.  Shouldnt get here... */
	    SYMB_FATAL_ERROR(SYMB_ERR_BIARC_FIT_FAIL);
	    return NULL;
	}
	else
	    Split = TRUE;
    }

    IRIT_VEC_ADD(T13, Tan1, Tan3);

    LenT13 = IRIT_VEC_LENGTH(T13);

    CosBeta = IRIT_DOT_PROD(V13, T13) / (LenV13 * LenT13);

    /* Build the quadratic equation "A a^2 + B a + C = 0" and solve for a. */
    A = IRIT_DOT_PROD(T13, T13) - IRIT_SQR(LenTan1 + LenTan3);
    B = 2.0 * CosBeta * LenV13 * LenT13;
    C = IRIT_SQR(LenV13);

    if (IRIT_APX_EQ_EPS(A, 0.0, IRIT_UEPS)) {
        a = -C / B;
    }
    else {
	CagdRType a1, a2;

        if ((Disc = B * B - 4 * A * C) < 0.0) {
	    Split = TRUE;
	    Disc = 0.0;
	}
	else
	    Disc = sqrt(Disc);

	/* Take the smallest positive solution. */
	a1 = (-B + Disc) / (2.0 * A);
	a2 = (-B - Disc) / (2.0 * A);
	if (a1 > 0 && a2 > 0)
	    a = IRIT_MIN(a1, a2);
	else if (a1 > 0)
	    a = a1;
	else if (a2 > 0)
	    a = a2;
	else
	    a = IRIT_UEPS;
    }

    /* Build the arcs' coefficients. */
    IRIT_VEC_SCALE(Tan1, a);
    IRIT_VEC_SCALE(Tan3, a);
    IRIT_PT_ADD(Pt12, Pt1, Tan1);
    IRIT_PT_SUB(Pt23, Pt3, Tan3);
    t = LenTan3 / (LenTan1 + LenTan3);
    IRIT_PT_BLEND(Pt2, Pt12, Pt23, t);

    if (!IRIT_APX_EQ(IRIT_PT_PT_DIST(Pt1, Pt12), IRIT_PT_PT_DIST(Pt12, Pt2)) ||
	!IRIT_APX_EQ(IRIT_PT_PT_DIST(Pt2, Pt23), IRIT_PT_PT_DIST(Pt23, Pt3))) {
#	ifdef DEBUG
        {
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugBiarcQuadSolver, FALSE) {
	        IRIT_INFO_MSG_PRINTF("Split1: %f %f %f %f %f %f\n",
			IRIT_PT_PT_DIST(Pt1, Pt12), IRIT_PT_PT_DIST(Pt12, Pt2),
			IRIT_PT_PT_DIST(Pt2, Pt23), IRIT_PT_PT_DIST(Pt23, Pt3),
			IRIT_PT_PT_DIST(Pt12, Pt23),
			IRIT_PT_PT_DIST(Pt1, Pt12) + IRIT_PT_PT_DIST(Pt23, Pt3));
	    }
	}
#	endif /* DEBUG */
	Split = TRUE;
    }

    IRIT_PT_SUB(Tan2, Pt23, Pt12);
    IRIT_VEC_NORMALIZE(Tan1);
    IRIT_VEC_NORMALIZE(Tan2);
    IRIT_VEC_NORMALIZE(Tan3);

    FirstIsLine = FALSE;
    if (!GMCircleFrom2Pts2Tans(Cntr1, &Rad1, Pt1, Pt2, Tan1, Tan2)) {
        if (IRIT_PT_APX_EQ(Tan1, Tan2)) {
	    /* This half of the biarc is actually suggested as a line. */
	    FirstIsLine = TRUE;
	}
	else
	    Split = TRUE;
    }

    SecondIsLine = FALSE;
    if (!GMCircleFrom2Pts2Tans(Cntr2, &Rad2, Pt2, Pt3, Tan2, Tan3)) {
        if (IRIT_PT_APX_EQ(Tan2, Tan3)) {
	    /* This half of the biarc is actually suggested as a line. */
	    SecondIsLine = TRUE;
	}
	else
	    Split = TRUE;
    }

    /* Split the curve and estimate distances to centers. */
    if (Crv -> Length > Crv -> Order)
        Crvs = CagdCrvSubdivAtParam(Crv,
				    Crv -> KnotVector[(Crv -> Length +
						       Crv -> Order) >> 1]);
    else
        Crvs = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5);

    if (Split ||
	(!FirstIsLine &&
	        IRIT_FABS(IRIT_PT_PT_DIST(Pt1, Cntr1) - Rad1) > Tolerance) ||
	(!FirstIsLine &&
		IRIT_FABS(IRIT_PT_PT_DIST(Pt2, Cntr1) - Rad1) > Tolerance) ||
	(!SecondIsLine &&
		IRIT_FABS(IRIT_PT_PT_DIST(Pt2, Cntr2) - Rad2) > Tolerance) ||
	(!SecondIsLine &&
		IRIT_FABS(IRIT_PT_PT_DIST(Pt3, Cntr2) - Rad2) > Tolerance) ||
	!EstimateDistCrvCenter(Crvs, FirstIsLine ? NULL : Cntr1,
			       Rad1, Tolerance) ||
	!EstimateDistCrvCenter(Crvs -> Pnext, SecondIsLine ? NULL : Cntr2,
			       Rad2, Tolerance)) {
        Arcs1 = SymbCrvBiArcApproxAux2(Crvs, Tolerance);
	Arcs2 = SymbCrvBiArcApproxAux2(Crvs -> Pnext, Tolerance);
    }
    else {
        /* Approximation good enough - update the arcs and return. */
        Arcs1 = SymbArcNew(!FirstIsLine);
	IRIT_PT_COPY(Arcs1 -> Pt1, Pt1);
	IRIT_PT_COPY(Arcs1 -> Cntr, Cntr1);
	IRIT_PT_COPY(Arcs1 -> Pt2, Pt2);

	Arcs2 = SymbArcNew(!SecondIsLine);
	IRIT_PT_COPY(Arcs2 -> Pt1, Pt2);
	IRIT_PT_COPY(Arcs2 -> Cntr, Cntr2);
	IRIT_PT_COPY(Arcs2 -> Pt2, Pt3);
    }
    CagdCrvFreeList(Crvs);
    return (SymbArcStruct *) CagdListAppend(Arcs1, Arcs2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the distance function from the Crv to Center and compare against *
* Radius for a deviation of more than the allowed Tolerance.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       To compute its distance to Center                             *
*   Center:    Center of arc to compute distance against, NULL if suppose to *
*	       be a line.				                     *
*   Radius:    Radius of arc to compute distance against.                    *
*   Tolerance: Valid tolerance of computed approximation.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if within prescribed Tolerance, FALSE otherwise.            *
*****************************************************************************/
static int EstimateDistCrvCenter(CagdCrvStruct *Crv,
				 CagdPType Center,
				 CagdRType Radius,
				 CagdRType Tolerance)
{
    int i;
    CagdRType *Pts, *WPts, *R, TMin, TMax, t,
	RadSqr = IRIT_SQR(Radius),
	TolSqr = IRIT_SQR(Tolerance);
    CagdPType Translate, Pt;
    CagdCrvStruct *CrvSqrLen;

    if (Center == NULL) {
	/* The curve to be examined as a line. */
	return CagdEstimateCrvCollinearity(Crv) < IRIT_EPS;        
    }

    /* Do a quick test to purge inaccuracies fast. */
    CagdCrvDomain(Crv, &TMin, &TMax);

    R = CagdCrvEval(Crv, (TMin + TMax) * 0.5);
    CagdCoerceToE3(Pt, &R, -1, Crv -> PType);
    t = IRIT_PT_PT_DIST_SQR(Pt, Center) - RadSqr;
    if (IRIT_FABS(t) > TolSqr)
	return FALSE;

    /* Do the real test. */
    Crv = CagdCrvCopy(Crv);
    IRIT_PT_COPY(Translate, Center);
    IRIT_PT_SCALE(Translate, -1.0);
    CagdCrvTransform(Crv, Translate, 1.0);
    CrvSqrLen = SymbCrvDotProd(Crv, Crv);
    CagdCrvFree(Crv);

    Pts = CrvSqrLen -> Points[1];
    if (CAGD_IS_RATIONAL_CRV(CrvSqrLen)) {
	WPts = CrvSqrLen -> Points[0];
	for (i = CrvSqrLen -> Length; i > 0; i--, Pts++, WPts++)
	    if (IRIT_FABS(*Pts / *WPts - RadSqr) > TolSqr)
		break;
    }
    else {
	for (i = CrvSqrLen -> Length; i > 0; i--, Pts++)
	    if (IRIT_FABS(*Pts - RadSqr) > TolSqr)
		break;
    }

    CagdCrvFree(CrvSqrLen);

    return i == 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a list of arcs (and lines) into curves geometry.                M
*                                                                            *
* PARAMETERS:                                                                M
*   Arcs:  To convert to real curves' geometry.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Generated curves.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcs2Crvs                                                            M
*****************************************************************************/
CagdCrvStruct *SymbArcs2Crvs(const SymbArcStruct *Arcs)
{
    const SymbArcStruct *Arc;
    CagdCrvStruct
	*Crvs = NULL;

    for (Arc = Arcs; Arc != NULL; Arc = Arc -> Pnext) {
        CagdCrvStruct *Crv;
	
	if (Arc -> Arc) {
	    CagdPtStruct Start, Center, End;

	    /* It is an arc. */

	    IRIT_PT2D_COPY(Start.Pt, Arc -> Pt1);
	    IRIT_PT2D_COPY(Center.Pt, Arc -> Cntr);
	    IRIT_PT2D_COPY(End.Pt, Arc -> Pt2);
	    Start.Pt[2] = Center.Pt[2] = End.Pt[2] = 0.0;

	    Crv = BzrCrvCreateArc(&Start, &Center, &End);

	    AttrSetObjAttrib(&Crv -> Attr, "center",
			     IPGenPTObject(&Arc -> Cntr[0],
					   &Arc -> Cntr[1],
					   &Arc -> Cntr[2]), FALSE);
	    IRIT_LIST_PUSH(Crv, Crvs);
	}
	else {
	    CagdPtStruct Pt1, Pt2;

	    /* It is a line segment. */

	    IRIT_PT2D_COPY(Pt1.Pt, Arc -> Pt1);
	    IRIT_PT2D_COPY(Pt2.Pt, Arc -> Pt2);
	    Pt1.Pt[2] = Pt2.Pt[2] = 0.0;
	    Crv = CagdMergePtPt(&Pt1, &Pt2);

	    IRIT_LIST_PUSH(Crv, Crvs);
	}
    }

    return CagdListReverse(Crvs);
}
