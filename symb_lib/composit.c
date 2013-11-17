/******************************************************************************
* Composit.c - Composition computation (symbolic).			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Apr. 93.					      *
******************************************************************************/

#include <string.h>
#include "symb_loc.h"

#define SYMB_ZERO_SET_EPS	IRIT_UEPS
#define SYMB_COMP_SUBDIV_EPS	(IRIT_UEPS * 100)
#define SYMB_CC_COMPOSE_EPS	IRIT_UEPS
#define SYMB_PER_COMP_EPS	IRIT_UEPS

#define SYMB_VERIFY_E2_CRV(Crv) \
    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2) { \
	CagdCrvStruct \
	  *CTmp = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);	\
	CagdCrvFree(Crv); \
	Crv = CTmp; \
    }

IRIT_STATIC_DATA int
    GlblComposeSrfCrvCache = FALSE;

static CagdCrvStruct *SymbComposeCrvCrvAux(const CagdCrvStruct *Crv1,
					   const CagdCrvStruct *Crv2);
static CagdCrvStruct *SymbComposeCrvCrvAux2(const CagdCrvStruct *Crv1,
					    const CagdCrvStruct *Crv2);
static CagdCrvStruct *FakedBzrComposeCrvCrv(const CagdCrvStruct *Crv1,
					    const CagdCrvStruct *Crv2);
static CagdCrvStruct *SymbComposeSrfCrvAux(const CagdSrfStruct *Srf,
					   const CagdCrvStruct *Crv);
static void SymbComposeSrfClrCacheAux(CagdSrfStruct *Srf);

static CagdCrvStruct *SymbComposeSrfCrvAux(const CagdSrfStruct *Srf,
					   const CagdCrvStruct *Crv);
static CagdCrvStruct *SymbComposeSrfCrvAux2(const CagdSrfStruct *Srf,
					    const CagdCrvStruct *Crv);
static int CompIntersectCrvWithLine(const CagdCrvStruct *OrigCrv,
				    int Axis,
				    CagdRType Val,
				    CagdCrvStruct **CrvsBelow,
				    CagdCrvStruct **CrvsAbove);
static CagdCrvStruct *CompMergeCrvListIntoOne(CagdCrvStruct *CrvSegs);
static CagdCrvStruct **SymbComputeCurvePowers(const CagdCrvStruct *Crv,
					      int Order);
static void SymbComputeCurvePowersMaple(const CagdCrvStruct *Crv,
					int Order,
					CagdCrvStruct ***CrvFactorsCache,
					int *CachedOrder);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to examine if the given curve has a monotone control polygon  M
* in the prescribed axis.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:    Curve to examine the the monotonicity in axis Axis.              M
*   Axis:   The axis of Crv to examine the monotonicity for.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    0 if not monotone, +1/-1 for increasing/decreasing monotone.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPosNegWeights                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMonotoneCtlPt                                                     M
*****************************************************************************/
int SymbCrvMonotoneCtlPt(const CagdCrvStruct *Crv, int Axis)
{
    int i,
	Len = Crv -> Length,
        Sign = 0;
     CagdRType
	*ScalarPts = Crv -> Points[1],
	*ScalarWPts = Crv -> Points[0];

    if (SymbCrvPosNegWeights(Crv)) {
	return 0;
    }
    else {
	CagdRType
	    LastVal = (ScalarWPts ? ScalarPts[0] / ScalarWPts[0]
		                  : ScalarPts[0]);
	for (i = 1; i < Len; i++) {
	    CagdRType
		NewVal = (ScalarWPts ? ScalarPts[i] / ScalarWPts[i]
                                     : ScalarPts[i]),
		NewDiff = IRIT_FABS(NewVal - LastVal) > SYMB_PER_COMP_EPS * 10 ?
						      NewVal - LastVal : 0.0;
	    int NewSign = IRIT_SIGN(NewDiff);

	    LastVal = NewVal;
	    if (Sign * NewSign < 0)
		return 0;
	    else if (NewSign)
		Sign = NewSign;
	}
    }

    return Sign;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves, Crv1 and Crv2, computes the composition Crv1(Crv2(t)).   M
*   Crv2 must be a scalar curve completely contained in Crv1's parametric    M
* domain.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to compose together.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The composed curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrComposeCrvCrv, SymbDecomposeCrvCrv, SymbComposePeriodicCrvCrv         M
*   SymbComposeSrfCrv							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposeCrvCrv, composition                                           M
*****************************************************************************/
CagdCrvStruct *SymbComposeCrvCrv(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2)
{
    CagdCrvStruct *CmpsCrv;
    CagdCrvStruct
	*NewCrv1 = NULL,
	*NewCrv2 = NULL;
    CagdRType TMax, TMin, CTMax, CTMin;
    CagdBType
	DoBezier = Crv1 -> GType == CAGD_CBEZIER_TYPE &&
	           Crv2 -> GType == CAGD_CBEZIER_TYPE;

    switch (Crv1 -> GType) {
	case CAGD_CBEZIER_TYPE:
	    if (!DoBezier)
		Crv1 = NewCrv1 = CagdCnvrtBzr2BspCrv(Crv1);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    break;
    }

    switch (Crv2 -> GType) {
	case CAGD_CBEZIER_TYPE:
	    if (!DoBezier)
		Crv2 = NewCrv2 = CagdCnvrtBzr2BspCrv(Crv2);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    break;
    }

    if (Crv1-> Order < 2 || Crv2 -> Order < 2)
	SYMB_FATAL_ERROR(SYMB_ERR_MINIMUM_LINEAR);

    CmpsCrv = SymbComposeCrvCrvAux(Crv1, Crv2);

    /* Now map the domain of the curve to be crv2 domain. */
    if (!DoBezier) {
	CagdCrvDomain(Crv2, &TMin, &TMax);
	CagdCrvDomain(CmpsCrv, &CTMin, &CTMax);
	if (CmpsCrv -> GType == CAGD_CBEZIER_TYPE) {
	    CagdCrvStruct
		*CTmp = CagdCnvrtBzr2BspCrv(CmpsCrv);

	    CagdCrvFree(CmpsCrv);
	    CmpsCrv = CTmp;
	}
	BspKnotAffineTrans(CmpsCrv -> KnotVector,
			   CmpsCrv -> Length + CmpsCrv -> Order,
			   TMin - CTMin,
			   (TMax - TMin) / (CTMax - CTMin));
    }

    if (NewCrv1 != NULL)
        CagdCrvFree(NewCrv1);
    if (NewCrv2 != NULL)
	CagdCrvFree(NewCrv2);

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Subdivides Crv2 until it is a Bezier curve, invokes the Bezier composition *
* code on each, and merges them back to a complete curve.		     *
*   At this point, curves can be either Bezier or Bspline only.		     *
*   Curves are both assumed to have open end condition.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:   The two curve to compose together.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The composed curve.                                   *
*****************************************************************************/
static CagdCrvStruct *SymbComposeCrvCrvAux(const CagdCrvStruct *Crv1,
					   const CagdCrvStruct *Crv2)
{
    CagdBType
	Monotone = SymbCrvMonotoneCtlPt(Crv2, 1);
    CagdRType *R,
	t = IRIT_INFNTY;
    CagdPtStruct *ExtremeSet, *Pt;
    CagdCrvStruct *TCrv, *Crv1a, *Crv1b, *Crv2a, *Crv2b,
	*CmpsCrv, *CmpsCrvA, *CmpsCrvB;

    if (!Monotone) {
	TCrv = CagdCrvDerive(Crv2);
	ExtremeSet = SymbCrvConstSet(TCrv, 1, SYMB_CC_COMPOSE_EPS, 0.0, TRUE);
	CagdCrvFree(TCrv);
	if (ExtremeSet == NULL) {
	    int l = Crv2 -> Length;
	    CagdRType
		*Pts = Crv2 -> Points[1],
		*WPts = Crv2 -> Points[0];

	    /* Curve is actually monotone (while its ctl poly is not). */
	    if (WPts == NULL)
		Monotone = Pts[l - 1] > Pts[0] ? 1 : -1;
	    else
		Monotone = Pts[l - 1] / WPts[l - 1] > Pts[0] / WPts[0] ? 1 : -1;
	}
	if (ExtremeSet != NULL) {
	    CagdRType TMin, TMax;

	    CagdCrvDomain(Crv2, &TMin, &TMax);

	    /* Find a middle point to split at. */
	    for (Pt = ExtremeSet; Pt != NULL; Pt = Pt -> Pnext) {
	        if (!IRIT_APX_EQ_EPS(Pt -> Pt[0], TMin, SYMB_CC_COMPOSE_EPS * 10) &&
		    !IRIT_APX_EQ_EPS(Pt -> Pt[0], TMax, SYMB_CC_COMPOSE_EPS * 10)) {
		    t = Pt -> Pt[0];
		    break;
		}
	    }
	    CagdPtFreeList(ExtremeSet);
	}
    }

    if (t != IRIT_INFNTY) {
	/* Crv2 is not a monotone segment. Subdivide, compute for each       */
	/* segment and merge back.					     */
	Crv2a = CagdCrvSubdivAtParam(Crv2, t);
	Crv2b = Crv2a -> Pnext;
	Crv2a -> Pnext = NULL;

	CmpsCrvA = SymbComposeCrvCrvAux(Crv1, Crv2a);
	CmpsCrvB = SymbComposeCrvCrvAux(Crv1, Crv2b);
	CagdCrvFree(Crv2a);
	CagdCrvFree(Crv2b);

	CmpsCrv = CagdMergeCrvCrv(CmpsCrvA, CmpsCrvB, FALSE);
	CagdCrvFree(CmpsCrvA);
	CagdCrvFree(CmpsCrvB);
    }
    else if (Crv2 -> Length > Crv2 -> Order) {
	/* Crv2 is not a Bezier segment. Subdivide, compute for each segment */
	/* and merge back.						     */

	t = Crv2 -> KnotVector[(Crv2 -> Order + Crv2 -> Length) >> 1];

	Crv2a = CagdCrvSubdivAtParam(Crv2, t);
	Crv2b = Crv2a -> Pnext;
	Crv2a -> Pnext = NULL;

	R = CagdCrvEval(Crv2, t);
	Crv1a = CagdCrvSubdivAtParam(Crv1, R[1]);
	Crv1b = Crv1a -> Pnext;
	Crv1a -> Pnext = NULL;

	/* Monotone can be +1 or -1 based on Crv2 being inc/decreasing. */
	CmpsCrvA = SymbComposeCrvCrvAux(Monotone == 1 ? Crv1a : Crv1b, Crv2a);
	CmpsCrvB = SymbComposeCrvCrvAux(Monotone == 1 ? Crv1b : Crv1a, Crv2b);
	CagdCrvFree(Crv1a);
	CagdCrvFree(Crv1b);
	CagdCrvFree(Crv2a);
	CagdCrvFree(Crv2b);

	CmpsCrv = CagdMergeCrvCrv(CmpsCrvA, CmpsCrvB, FALSE);
	CagdCrvFree(CmpsCrvA);
	CagdCrvFree(CmpsCrvB);
    }
    else {
	CagdBBoxStruct BBox;
	CagdRType TMin1, TMax1;

	CagdCrvDomain(Crv1, &TMin1, &TMax1);

    	/* Crv2 is a Bezier segment - compute its composition. */
	CagdCrvBBox(Crv2, &BBox);
	TCrv = CagdCrvRegionFromCrv(Crv1, IRIT_MAX(BBox.Min[0], TMin1),
				          IRIT_MIN(BBox.Max[0], TMax1));

	CmpsCrv = SymbComposeCrvCrvAux2(TCrv, Crv2);
	CagdCrvFree(TCrv);
    }

    return CmpsCrv;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* See SymbComposeCrvCrvAux.                                                  *
*****************************************************************************/
static CagdCrvStruct *SymbComposeCrvCrvAux2(const CagdCrvStruct *Crv1,
					    const CagdCrvStruct *Crv2)
{
    CagdRType t, TMin, TMax;
    CagdCrvStruct *CmpsCrv;

    if (Crv1 -> Length > Crv1 -> Order) {
	/* Needs to compose in pieces after subdividing Crv2 at the interior */
	/* knots of Crv1, by finding where Crv2(r) = t, the interior knot.   */
	CagdCrvStruct *Crv1a, *Crv1b, *Crv2a, *Crv2b, *CmpsCrvA, *CmpsCrvB;
	CagdPtStruct *Pts;

	t = Crv1 -> KnotVector[(Crv1 -> Order + Crv1 -> Length) >> 1];
	Crv1a = CagdCrvSubdivAtParam(Crv1, t);
	Crv1b = Crv1a -> Pnext;
	Crv1a -> Pnext = NULL;

	Pts = SymbCrvConstSet(Crv2, 1, SYMB_ZERO_SET_EPS, t, TRUE);
	if (Pts) {
	    if (Pts -> Pnext != NULL)
		SYMB_FATAL_ERROR(SYMB_ERR_REPARAM_NOT_MONOTONE);

	    t = Pts -> Pt[0];
	    CagdPtFreeList(Pts);
	    Crv2a = CagdCrvSubdivAtParam(Crv2, t);
	    Crv2b = Crv2a -> Pnext;
	    Crv2a -> Pnext = NULL;
	}
	else
	    Crv2a = Crv2b = NULL;

	CagdCrvDomain(Crv1, &TMin, &TMax);
	if (IRIT_APX_EQ(TMin, t))
	    CmpsCrvA = NULL;
	else
	    CmpsCrvA = SymbComposeCrvCrvAux2(Crv1a, Crv2a ? Crv2a : Crv2);
	if (IRIT_APX_EQ(TMax, t))
	    CmpsCrvB = NULL;
	else
	    CmpsCrvB = SymbComposeCrvCrvAux2(Crv1b, Crv2b ? Crv2b : Crv2);
	CagdCrvFree(Crv1a);
	CagdCrvFree(Crv1b);
	if (Crv2a)
	    CagdCrvFree(Crv2a);
	if (Crv2b)
	    CagdCrvFree(Crv2b);

	if (CmpsCrvA == NULL)
	    CmpsCrv = CmpsCrvB;
	else if (CmpsCrvB == NULL)
	    CmpsCrv = CmpsCrvA;
	else {
	    CmpsCrv = CagdMergeCrvCrv(CmpsCrvA, CmpsCrvB, FALSE);
	    CagdCrvFree(CmpsCrvA);
	    CagdCrvFree(CmpsCrvB);
	}
    }
    else {
	/* We can compose the curves, but first make sure the domain of */
	/* Crv1 is [0, 1] which is also the range of Crv2.		*/
	CagdCrvDomain(Crv1, &TMin, &TMax);

	if (!IRIT_APX_EQ(TMin, 0.0) || !IRIT_APX_EQ(TMax, 1.0)) {
	    CagdRType
		Translate = -TMin;
	    CagdCrvStruct
	        *Crv2Tmp = CagdCrvCopy(Crv2);

	    CagdCrvTransform(Crv2Tmp, &Translate, 1.0 / (TMax - TMin));
	    CmpsCrv = FakedBzrComposeCrvCrv(Crv1, Crv2Tmp);
	    CagdCrvFree(Crv2Tmp);
	}
	else
	    CmpsCrv = FakedBzrComposeCrvCrv(Crv1, Crv2);

	/* Now map the domain of the curve to be domain of Crv2. */
	CagdCrvDomain(Crv2, &TMin, &TMax);

	if (CmpsCrv -> GType == CAGD_CBEZIER_TYPE) {
	    CagdCrvStruct
		*CTmp = CagdCnvrtBzr2BspCrv(CmpsCrv);

	    CagdCrvFree(CmpsCrv);
	    CmpsCrv = CTmp;
	}
	BspKnotAffineTrans2(CmpsCrv -> KnotVector,
			    CmpsCrv -> Length + CmpsCrv -> Order,
			    TMin, TMax);
    }

    return CmpsCrv;
}

/*****************************************************************************
* AUXILIARY:								     *
*   Auxiliary function to fake the given curves as Bezier curves.	     *
*****************************************************************************/
static CagdCrvStruct *FakedBzrComposeCrvCrv(const CagdCrvStruct *Crv1,
					    const CagdCrvStruct *Crv2)
{
    CagdCrvStruct Crv1Bzr, Crv2Bzr;

    CAGD_GEN_COPY(&Crv1Bzr, Crv1, sizeof(CagdCrvStruct));
    CAGD_GEN_COPY(&Crv2Bzr, Crv2, sizeof(CagdCrvStruct));

    Crv1Bzr.GType = CAGD_CBEZIER_TYPE;
    Crv2Bzr.GType = CAGD_CBEZIER_TYPE;
    Crv1Bzr.KnotVector = Crv2Bzr.KnotVector = NULL;

    return BzrComposeCrvCrv(&Crv1Bzr, &Crv2Bzr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier curves, Crv1 and Crv2, computes their composition         M
* Crv1(Crv2(t)).							     M
*   Crv2 must be a scalar curve completely contained in Crv1's parametric    M
* domain.								     M
*   See: "Freeform surfcae analysis using a hybrid of symbolic and numeric   M
* computation" by Gershon Elber, PhD thesis, University of Utah, 1992.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curve to compose together.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The composed curve.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDecomposeCrvCrv, BzrComposeCrvCrv                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrComposeCrvCrv, composition                                            M
*****************************************************************************/
CagdCrvStruct *BzrComposeCrvCrv(const CagdCrvStruct *Crv1,
				const CagdCrvStruct *Crv2)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv1);
    int i, j, k, CmpsOrder,
	Order = Crv1 -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv1 -> PType);
    CagdRType
	Translate = 0.0;
    CagdCrvStruct **Crv2Factors,
    	*CmpsCrv = NULL;

    Crv2Factors = SymbComputeCurvePowers(Crv2, Order);
    CmpsCrv = BzrCrvNew(Crv2Factors[0] -> Length, Crv1 -> PType);
    CmpsOrder = CmpsCrv -> Order;

    for (j = !IsRational; j <= MaxCoord; j++) {
    	CagdRType
	    *CmpsPoints = CmpsCrv -> Points[j],
	    *Crv1Points = Crv1 -> Points[j];

	for (i = 0; i < Order; i++) {
	    CagdCrvStruct
		*CTmp = CagdCrvCopy(Crv2Factors[i]);
	    CagdRType
	        *CTmpPoints = CTmp -> Points[1];

	    CagdCrvTransform(CTmp, &Translate, *Crv1Points++);
	    if (i == 0) {
       		SYMB_GEN_COPY(CmpsPoints, CTmpPoints,
			      CmpsOrder * sizeof(CagdRType));
	    }
	    else {
	    	for (k = 0; k < CmpsOrder; k++)
	    	    CmpsPoints[k] += CTmpPoints[k];
	    }

	    CagdCrvFree(CTmp);
	}
    }

    for (i = 0; i < Order; i++)
	CagdCrvFree(Crv2Factors[i]);

    if (CAGD_IS_RATIONAL_CRV(Crv2)) {
	CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *CTmp;

	SymbCrvSplitScalar(CmpsCrv, &CrvW, &CrvX, &CrvY, &CrvZ);
	CTmp = SymbCrvMergeScalar(Crv2Factors[Order], CrvX, CrvY, CrvZ);
	CagdCrvFree(CmpsCrv);
	CmpsCrv = CTmp;

	if (CrvX)
	    CagdCrvFree(CrvX);
	if (CrvY)
	    CagdCrvFree(CrvY);
	if (CrvZ)
	    CagdCrvFree(CrvZ);
	
	CagdCrvFree(Crv2Factors[Order]);
    }

    IritFree(Crv2Factors);

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves, Crv1 and Crv2, computes the composition Crv1(Crv2(t)).   M
*   Crv2 must be a scalar curve that is clipped and tiled modulus the domain M
* of Crv1.  As an example, if Crv1's domain is [1, 5), and Crv2's range is   M
* [-3, 11), then Crv2 will be clipped to the following pieces:               M
* 1. [-3, 1) that is remapped to [1, 5) and composed.			     V
* 2. [ 1, 5) that is composed as is.					     V
* 3. [ 5, 9) that is remapped to [1, 5) and composed.			     V
* 4. [ 9, 11) that is remapped to [1, 3) and composed.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to compose together.                        M
*   Epsilon:      Of subdivision at boundary crossing locations.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The composed curve.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeCrvCrv                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposePeriodicCrvCrv                                                M
*****************************************************************************/
CagdCrvStruct *SymbComposePeriodicCrvCrv(const CagdCrvStruct *Crv1,
					 const CagdCrvStruct *Crv2,
					 CagdRType Epsilon)
{
    CagdRType TMin1, TMax1, TMin2, TMax2;
    CagdPtStruct
	*BndryPts = NULL;
    CagdCrvStruct *Crv21, *Crv22, *Crv21c, *Crv22c, *RetCrv;
    CagdBBoxStruct BBox;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);
    CagdCrvBBox(Crv2, &BBox);

    if (BBox.Min[0] < TMin1)
	BndryPts = SymbCrvConstSet(Crv2, 1, SYMB_PER_COMP_EPS, TMin1, TRUE);

    if (BndryPts == NULL && BBox.Max[0] > TMax1)
        BndryPts = SymbCrvConstSet(Crv2, 1, SYMB_PER_COMP_EPS, TMax1, TRUE);

    if (BndryPts != NULL &&
	!IRIT_APX_EQ_EPS(BndryPts -> Pt[0], TMin2, SYMB_PER_COMP_EPS * 10) &&
	!IRIT_APX_EQ_EPS(BndryPts -> Pt[0], TMax2, SYMB_PER_COMP_EPS * 10)) {
	/* A valid point to split Crv2 at. */
	Crv21 = CagdCrvSubdivAtParam(Crv2, BndryPts -> Pt[0]);
	Crv22 = Crv21 -> Pnext;
	Crv21 -> Pnext = NULL;
	CagdPtFreeList(BndryPts);

        Crv21c = SymbComposePeriodicCrvCrv(Crv1, Crv21, Epsilon);
        Crv22c = SymbComposePeriodicCrvCrv(Crv1, Crv22, Epsilon);
	CagdCrvFree(Crv21);
	CagdCrvFree(Crv22);

	RetCrv = CagdMergeCrvCrv(Crv21c, Crv22c, TRUE);
	CagdCrvFree(Crv21c);
	CagdCrvFree(Crv22c);
    }
    else {
	/* No intersection with domain of Crv1.  Either we are fine or we    */
	/* are completely outside.  Find out by evaluation middle point.     */
	CagdRType t, Trans[3],
	    *R = CagdCrvEval(Crv2, (TMin2 + TMax2) * 0.5);

	if (BndryPts != NULL)
	    CagdPtFreeList(BndryPts);

	t = CAGD_IS_RATIONAL_CRV(Crv2) ? R[1] / R[0] : R[1];
	if (t < TMin1 || t > TMax1) {
	    Crv21 = CagdCrvCopy(Crv2);
	    Trans[0] = t < TMin1 ? TMax1 - TMin1 : TMin1 - TMax1;
	    CagdCrvTransform(Crv21, Trans, 1.0);

	    RetCrv = SymbComposePeriodicCrvCrv(Crv1, Crv21, Epsilon);
	    CagdCrvFree(Crv21);
	}
	else
	    RetCrv = SymbComposeCrvCrv(Crv1, Crv2);
    }

    return RetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Activate caching during surface-curve composition operations.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Cache:   TRUE to activate the cache, FALSE to disable it.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Old state of caching.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeSrfCrv, SymbComposeSrfClrCache                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposeSrfSetCache, composition                                      M
*****************************************************************************/
int SymbComposeSrfSetCache(int Cache)
{
   int OldCache = Cache;

   GlblComposeSrfCrvCache = Cache;

   return OldCache;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a surface Srf that was used in a composition operation with some   M
* curve via SymbComposeSrfCrv while SymbComposeSrfSetCache was on, cleans    M
* all cached informaation on the surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to clean all its cached data for SymbComposeSrfCrv.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeSrfCrv, SymbComposeSrfClrCache                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposeSrfClrCache, composition                                      M
*****************************************************************************/
void SymbComposeSrfClrCache(const CagdSrfStruct *Srf)
{
    CagdSrfStruct
	*CacheSrfs = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, "_csc");

    if (CacheSrfs) {
	SymbComposeSrfClrCacheAux(CacheSrfs -> Pnext);
	SymbComposeSrfClrCacheAux(CacheSrfs);
	AttrFreeOneAttribute(&((CagdSrfStruct *) Srf) -> Attr, "_csc");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. function of SymbComposeSrfClrCache that traverses the recursive       *
* subdivision tree and frees everything.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   The surface subtree to completely free.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void SymbComposeSrfClrCacheAux(CagdSrfStruct *Srf)
{
    CagdSrfStruct
	*CacheSrfs = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, "_csc");

    if (CacheSrfs) {
	SymbComposeSrfClrCacheAux(CacheSrfs -> Pnext);
	SymbComposeSrfClrCacheAux(CacheSrfs);
    }

    CagdSrfFree(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and surface Srf, computes the composition Srf(Crv(t)).   M
*   Crv must be a two dimensional curve completely contained in the          M
* parametric domain of Srf.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf, Crv:   The curve and surface to compose.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The resulting composition.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDecomposeCrvCrv, SymbComposeSrfSetCache, SymbComposePeriodicSrfCrv   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposeSrfCrv, composition                                           M
*****************************************************************************/
CagdCrvStruct *SymbComposeSrfCrv(const CagdSrfStruct *Srf,
				 const CagdCrvStruct *Crv)
{
    CagdBType
	BezCrv = FALSE,
	BezSrf = FALSE;
    CagdCrvStruct *CmpsCrv,
	*NewCrv = NULL;
    CagdSrfStruct
	*NewSrf = NULL;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = NewSrf = CagdCnvrtBzr2BspSrf(Srf);
	    BezCrv = TRUE;
	    break;
	case CAGD_SBSPLINE_TYPE:
	    break;
	case CAGD_SPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
	    break;
    }

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = NewCrv = CagdCnvrtBzr2BspCrv(Crv);
	    BezSrf = TRUE;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    if (CAGD_IS_PERIODIC_CRV(Crv))
		Crv = NewCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
	    if (!BspCrvHasOpenEC(Crv)) {
	        CagdCrvStruct *NewCrv2;

		Crv = NewCrv2 = CagdCnvrtFloat2OpenCrv(Crv);
		if (NewCrv != NULL)
		    CagdCrvFree(NewCrv);
		NewCrv = NewCrv2;
	    }
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    break;
    }

    CmpsCrv = SymbComposeSrfCrvAux(Srf, Crv);

    if (BezCrv && BezSrf) {		    /* Both Crv and Srf are Bezier. */
	CagdCrvStruct
	    *TCrv = CagdCnvrtBsp2BzrCrv(CmpsCrv);

	CagdCrvFree(CmpsCrv);
	CmpsCrv = TCrv;
    }

    if (NewCrv != NULL)
        CagdCrvFree(NewCrv);
    if (NewSrf != NULL)
        CagdSrfFree(NewSrf);

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. function. Subdivides Crv until it is a Bezier curve, invokes the      *
* Bezier composition code on each, and merges them back to a complete curve. *
*   At this point, the curve can be either Bezier or Bspline only.           *
*   Curve is assumed to have open end condition.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf, Crv:   The curve and surface to compose together.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  The composed curve.                                    *
*****************************************************************************/
static CagdCrvStruct *SymbComposeSrfCrvAux(const CagdSrfStruct *Srf,
					   const CagdCrvStruct *Crv)
{
    CagdRType t;
    CagdCrvStruct *CmpsCrv;

    if (Crv -> Length > Crv -> Order) {
	/* Crv is not a Bezier segment. Subdivide, compute for each segment  */
	/* and merge back.						     */
	CagdCrvStruct *CrvA, *CrvB, *CmpsCrvA, *CmpsCrvB;

	t = Crv -> KnotVector[(Crv -> Order + Crv -> Length) >> 1];

	CrvA = CagdCrvSubdivAtParam(Crv, t);
	CrvB = CrvA -> Pnext;
	CrvA -> Pnext = NULL;

	CmpsCrvA = SymbComposeSrfCrvAux(Srf, CrvA);
	CmpsCrvB = SymbComposeSrfCrvAux(Srf, CrvB);
	CagdCrvFree(CrvA);
	CagdCrvFree(CrvB);

	CmpsCrv = CagdMergeCrvCrv(CmpsCrvA, CmpsCrvB, FALSE);
	CagdCrvFree(CmpsCrvA);
	CagdCrvFree(CmpsCrvB);
    }
    else {
    	/* Crv is a Bezier curve segment - compute its composition. */
    	CmpsCrv = SymbComposeSrfCrvAux2(Srf, Crv);
    }

    return CmpsCrv;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* See SymbComposeSrfCrvAux.  At this time, Crv is a Bezier curve.            *
*****************************************************************************/
static CagdCrvStruct *SymbComposeSrfCrvAux2(const CagdSrfStruct *Srf,
					    const CagdCrvStruct *Crv)
{
    CagdSrfStruct *Srf1, *Srf2;
    CagdCrvStruct *CrvsBelow, *CrvsAbove, *CmpsCrvsBelow, *CmpsCrvsAbove,
	*CmpsCrv, *TCrv, *CmpsTCrv;

    if (Srf -> UOrder != Srf -> ULength || Srf -> VOrder != Srf -> VLength) {
        CagdSrfDirType Dir;
	CagdRType SubdivVal;

	if (Srf -> UOrder != Srf -> ULength) {
	    SubdivVal = Srf -> UKnotVector[(Srf -> ULength +
					    Srf -> UOrder) >> 1];
	    Dir = CAGD_CONST_U_DIR;
	}
	else {
	    SubdivVal = Srf -> VKnotVector[(Srf -> VLength +
					    Srf -> VOrder) >> 1];
	    Dir = CAGD_CONST_V_DIR;
	}

	if (GlblComposeSrfCrvCache &&
	    (Srf1 = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						       "_csc")) != NULL) {
	    Srf2 = Srf1 -> Pnext;
	}
	else {
	    /* Subdivide. */
	    Srf1 = BspSrfSubdivAtParam(Srf, SubdivVal, Dir);
	    Srf2 = Srf1 -> Pnext;

	    if (GlblComposeSrfCrvCache)
	      AttrSetPtrAttrib(&((CagdSrfStruct *) Srf) -> Attr,
			       "_csc", Srf1);
	}

	/* Split the curve at this U parameter. */
	CompIntersectCrvWithLine(Crv, Dir == CAGD_CONST_U_DIR ? 0 : 1,
				 SubdivVal, &CrvsBelow, &CrvsAbove);

	CmpsCrvsBelow = CmpsCrvsAbove = NULL;
	for (TCrv = CrvsBelow; TCrv != NULL; TCrv = TCrv -> Pnext) {
	    CmpsTCrv = SymbComposeSrfCrvAux2(Srf1, TCrv);
	    IRIT_LIST_PUSH(CmpsTCrv, CmpsCrvsBelow);
	}
	for (TCrv = CrvsAbove; TCrv != NULL; TCrv = TCrv -> Pnext) {
	    CmpsTCrv = SymbComposeSrfCrvAux2(Srf2, TCrv);
	    IRIT_LIST_PUSH(CmpsTCrv, CmpsCrvsAbove);
	}
	CagdCrvFreeList(CrvsBelow);
	CagdCrvFreeList(CrvsAbove);

	if (!GlblComposeSrfCrvCache) {
	    CagdSrfFree(Srf1);
	    CagdSrfFree(Srf2);
	}

	CmpsCrv = CompMergeCrvListIntoOne((CagdCrvStruct *)
					  CagdListAppend(CmpsCrvsBelow,
							 CmpsCrvsAbove));
    }
    else {
        IRIT_STATIC_DATA CagdCrvStruct
	    *LclCrv = NULL;
	IRIT_STATIC_DATA int
	    LclCrvOrder = 0;
	int i,
	    Length = Crv -> Length;
	CagdRType **LPoints, TMin, TMax, UMin, UMax, VMin, VMax, Du1, Dv1;
	CagdRType * const *Points;

	/* Bring the curve to cover the Bspline domain of Srf. */
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	Du1 = 1.0 / (UMax - UMin);
	Dv1 = 1.0 / (VMax - VMin);
	CagdCrvDomain(Crv, &TMin, &TMax);

	if (LclCrv == NULL || LclCrvOrder < Crv -> Order) {
	    if (LclCrv != NULL) {
	        LclCrv -> PType = CAGD_PT_P2_TYPE;
		CagdCrvFree(LclCrv);
	    }
	    LclCrv = BzrCrvNew(LclCrvOrder = Crv -> Order, CAGD_PT_P2_TYPE);
	}
	Points = Crv -> Points;
	LPoints = LclCrv -> Points;
	LclCrv -> Order = Crv -> Order;
	LclCrv -> Length = Crv -> Length;
	LclCrv -> PType = Crv -> PType;
	if (CAGD_IS_RATIONAL_CRV(Crv)) {
	    LclCrv -> PType = CAGD_PT_P2_TYPE;
	    CAGD_GEN_COPY(LPoints[0], Points[0], sizeof(CagdRType) * Length);
	    for (i = 0; i < Length; i++) {
		LPoints[1][i] = (Points[1][i] / Points[0][i] - UMin)
							* Du1 * Points[0][i];
		LPoints[2][i] = (Points[2][i] / Points[0][i] - VMin)
							* Dv1 * Points[0][i];
	    }
	}
	else {
	    LclCrv -> PType = CAGD_PT_E2_TYPE;
	    for (i = 0; i < Length; i++) {
		LPoints[1][i] = (Points[1][i] - UMin) * Du1;
		LPoints[2][i] = (Points[2][i] - VMin) * Dv1;
	    }
	}

    	/* Srf is a Bezier surface - compute its composition.        */
	/* Fake both curves as Bezier for a minute and then recover. */
	{
	    CagdSrfStruct SrfBzr;

	    CAGD_GEN_COPY(&SrfBzr, Srf, sizeof(CagdSrfStruct));

	    SrfBzr.GType = CAGD_SBEZIER_TYPE;
	    SrfBzr.UKnotVector = SrfBzr.VKnotVector = NULL;

	    CmpsCrv = BzrComposeSrfCrvInterp(&SrfBzr, LclCrv);
	}
	
	/* Now map the domain of the composed curve to be crv's domain. */
	if (CmpsCrv -> GType == CAGD_CBEZIER_TYPE) {
	    /* Convert to a Bspline form. */
	    CmpsCrv -> Order = CmpsCrv -> Length;
	    CmpsCrv -> KnotVector = BspKnotUniformOpen(CmpsCrv -> Length,
						       CmpsCrv -> Order, NULL);
	    CmpsCrv -> GType = CAGD_CBSPLINE_TYPE;
	}
	BspKnotAffineTransOrder2(CmpsCrv -> KnotVector,
				 CmpsCrv -> Order,
				 CmpsCrv -> Length + CmpsCrv -> Order,
				 TMin, TMax);
    }

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and surface Srf, computes the composition Srf(Crv(t)).   M
*   Crv must be a two dimensional curve that is clipped and tiled modulus    M
* the domain of Srf.  As an example, if Surface V domain is [1, 5), and Crv  M
* Y range [-3, 11), then Crv will be V clipped to the following pieces:      M
* 1. [-3, 1) that is remapped to [1, 5) and composed.			     V
* 2. [ 1, 5) that is composed as is.					     V
* 3. [ 5, 9) that is remapped to [1, 5) and composed.			     V
* 4. [ 9, 11) that is remapped to [1, 3) and composed.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf, Crv:     The curve and periodic surface to compose together.        M
*   Epsilon:      Of subdivision at boundary crossing locations.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The composed curve.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeSrfCrv                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposePeriodicSrfCrv, composition                                   M
*****************************************************************************/
CagdCrvStruct *SymbComposePeriodicSrfCrv(const CagdSrfStruct *Srf,
					 const CagdCrvStruct *Crv,
					 CagdRType Epsilon)
{
    CagdRType TMin, TMax, UMin, UMax, VMin, VMax;
    CagdPtStruct
	*BndryPts = NULL;
    CagdCrvStruct *Crv1, *Crv2, *RetCrv;
    CagdBBoxStruct BBox;

    CagdCrvDomain(Crv, &TMin, &TMax);
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    CagdCrvBBox(Crv, &BBox);

    if (BBox.Min[0] < UMin)
	BndryPts = SymbCrvConstSet(Crv, 1, SYMB_PER_COMP_EPS, UMin, TRUE);

    if (BndryPts == NULL && BBox.Max[0] > UMax)
        BndryPts = SymbCrvConstSet(Crv, 1, SYMB_PER_COMP_EPS, UMax, TRUE);

    if (BndryPts == NULL && BBox.Min[1] < VMin)
	BndryPts = SymbCrvConstSet(Crv, 2, SYMB_PER_COMP_EPS, VMin, TRUE);

    if (BndryPts == NULL && BBox.Max[1] > VMax)
        BndryPts = SymbCrvConstSet(Crv, 2, SYMB_PER_COMP_EPS, VMax, TRUE);

    if (BndryPts != NULL &&
	!IRIT_APX_EQ(BndryPts -> Pt[0], TMin) &&
	!IRIT_APX_EQ(BndryPts -> Pt[0], TMax)) {
	CagdCrvStruct *Crv1c, *Crv2c;

	/* A valid point to split Crv2 at. */
	Crv1 = CagdCrvSubdivAtParam(Crv, BndryPts -> Pt[0]);
	Crv2 = Crv1 -> Pnext;
	Crv1 -> Pnext = NULL;

        Crv1c = SymbComposePeriodicSrfCrv(Srf, Crv1, Epsilon);
        Crv2c = SymbComposePeriodicSrfCrv(Srf, Crv2, Epsilon);
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

	RetCrv = CagdMergeCrvCrv(Crv1c, Crv2c, TRUE);
	CagdCrvFree(Crv1c);
	CagdCrvFree(Crv2c);
    }
    else {
	/* No intersection with domain of Srf.  Either we are fine or we    */
	/* are completely outside.  Find out by evaluation middle point.    */
        int Inside = TRUE;
        CagdUVType UV;
	CagdRType Trans[3],
	    *R = CagdCrvEval(Crv, (TMin + TMax) * 0.5);

	CagdCoerceToE2(UV, &R, -1, Crv -> PType);
	if (UV[0] < UMin || UV[0] > UMax) {
	    Inside = FALSE;
	    Trans[0] = UV[0] < UMin ? UMax - UMin : UMin - UMax;
	    Trans[1] = 0.0;
	}
	else if (UV[1] < VMin || UV[1] > VMax) {
	    Inside = FALSE;
	    Trans[0] = 0.0;
	    Trans[1] = UV[1] < VMin ? VMax - VMin : VMin - VMax;
	}

	if (Inside) {
	    RetCrv = SymbComposeSrfCrv(Srf, Crv);
	}
	else {
	    CagdCrvStruct
	        *TrCrv = CagdCrvCopy(Crv);

	    CagdCrvTransform(TrCrv, Trans, 1.0);

	    RetCrv = SymbComposePeriodicSrfCrv(Srf, TrCrv, Epsilon);
	    CagdCrvFree(TrCrv);
	}
    }

    CagdPtFreeList(BndryPts);

    return RetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Intersects the given curve with a line along the given direction Axis    *
* and splits the curve at all such intersections.  The segments are placed   *
* into two lists, of curves below that line and curves above that line.      *
*                                                                            *
* PARAMETERS:                                                                *
*   OrigCrv:   Curve to intersect and split with a line.                     *
*   Axis:      Line axis, 0 for X, 1 for Y, etc.                             *
*   Val:       Elevation along that axis.                                    *
*   CrvsBelow: All curve segments found to be below the prescribed line.     *
*   CrvsAbove: All curve segments found to be above the prescribed line.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if succesful, FALSE otherwise                             *
*****************************************************************************/
static int CompIntersectCrvWithLine(const CagdCrvStruct *OrigCrv,
				    int Axis,
				    CagdRType Val,
				    CagdCrvStruct **CrvsBelow,
				    CagdCrvStruct **CrvsAbove)
{
    int i;
    CagdRType Sum, *Points, *WPoints;
    CagdPtStruct *Pt,
	*Pts = SymbCrvConstSet(OrigCrv, Axis + 1, SYMB_COMP_SUBDIV_EPS,
			       Val, TRUE);
    CagdCrvStruct
        *Crv = CagdCrvCopy(OrigCrv);

    *CrvsBelow = *CrvsAbove = NULL;

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	CagdCrvStruct
	    *Crvs = CagdCrvSubdivAtParam(Crv, Pt -> Pt[0]);

	if (Crvs == NULL || Crvs -> Pnext == NULL)
	    continue;

	CagdCrvFree(Crv);
	Crv = Crvs -> Pnext;

	/* Classify the curve segment by computing average elevation. */
	Points = Crvs -> Points[Axis + 1];
	for (i = 0, Sum = 0.0; i < Crvs -> Length; i++)
	    Sum += *Points++;
	Sum /= Crvs -> Length;
	if (Sum > Val)
	    IRIT_LIST_PUSH(Crvs, *CrvsAbove)
	else
	    IRIT_LIST_PUSH(Crvs, *CrvsBelow);
    }
    CagdPtFreeList(Pts);

    /* Do last segment. */

    /* Classify the curve segment by computing average elevation. */
    Points = Crv -> Points[Axis + 1];
    WPoints = Crv -> Points[0];
    for (i = 0, Sum = 0.0; i < Crv -> Length; i++) {
	if (WPoints != NULL)
	    Sum += *Points++ / *WPoints++;
	else
	    Sum += *Points++;
    }
    Sum /= Crv -> Length;
    if (Sum > Val)
        IRIT_LIST_PUSH(Crv, *CrvsAbove)
    else
        IRIT_LIST_PUSH(Crv, *CrvsBelow);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges all curve segments into a single composed curve.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvSegs:    Curve segments to merge. Curves are freed once used.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The merged curve.                                     *
*****************************************************************************/
static CagdCrvStruct *CompMergeCrvListIntoOne(CagdCrvStruct *CrvSegs)
{
    CagdCrvStruct *Crv;

    IRIT_LIST_POP(Crv, CrvSegs);
    while (CrvSegs != NULL) {
	CagdRType Min, Max;
	CagdCrvStruct *PrevCrv, *TCrv, *MCrv;

	CagdCrvDomain(Crv, &Min, &Max);

	for (PrevCrv = NULL, TCrv = CrvSegs;
	     TCrv != NULL;
	     PrevCrv = TCrv, TCrv = TCrv -> Pnext) {
	    CagdRType TMin, TMax;

	    CagdCrvDomain(TCrv, &TMin, &TMax);
	    if (IRIT_APX_EQ(Max, TMin) || IRIT_APX_EQ(TMax, Min)) {
		/* Remove TCrv from curve segments list. */
		if (PrevCrv == NULL)
		    CrvSegs = CrvSegs -> Pnext;
		else
		    PrevCrv -> Pnext = TCrv -> Pnext;

		/* And append the two curves into one. */
		if (IRIT_APX_EQ(Max, TMin))
		    MCrv = CagdMergeCrvCrv(Crv, TCrv, FALSE);
		else
		    MCrv = CagdMergeCrvCrv(TCrv, Crv, FALSE);
		CagdCrvFree(Crv);
		CagdCrvFree(TCrv);
		Crv = MCrv;
		break;
	    }
	}
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier curve Crv and a Bezier surface Srf, computes their          M
* coomposition Srf(Crv(t)).						     M
*   Crv must be a two dimensional curve completely contained in the          M
* parametric domain of Srf.                                                  M
*   Composition is solved by deriving the degree d of the resulting curve    M
* and solving a Bezier interpolation problem.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf, Crv:     The curve and surface to compose.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The resulting composition.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrComposeSrfCrvInterp, composition                                      M
*****************************************************************************/
CagdCrvStruct *BzrComposeSrfCrvInterp(const CagdSrfStruct *Srf,
				      const CagdCrvStruct *Crv)
{
    IRIT_STATIC_DATA int
	InputVecLen = 0;
    IRIT_STATIC_DATA CagdRType
	*Input[CAGD_MAX_PT_SIZE] = { NULL };
    int i, j,
	Deg = ((Srf -> UOrder - 1) + (Srf -> VOrder - 1)) * (Crv -> Order - 1);
    CagdCrvStruct *CmpsCrv;

    if (CAGD_IS_RATIONAL_CRV(Crv))
	return BzrComposeSrfCrv(Srf, Crv);

    /* Expects UV in the curve and at most XYZ in surface. */
    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2)
	SYMB_FATAL_ERROR(SYMB_ERR_WRONG_PT_TYPE);

    CmpsCrv = BzrCrvNew(Deg + 1, Srf -> PType);

    /* Make sure Input is big enough. */
    if (InputVecLen < Deg + 2) {
        if (Input[0] != NULL) {
	    for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	        IritFree(Input[i]);
	}
	InputVecLen = Deg * 2 + 2;
	for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	    Input[i] = IritMalloc(sizeof(CagdRType) * InputVecLen);
    }

    /* Evaluate the composition points. */
    for (i = 0; i <= Deg; i++) {
        CagdRType
	    *R = CagdCrvEval(Crv, ((CagdRType) i) / Deg);

	R = CagdSrfEval(Srf, R[1], R[2]);
	for (j = !CAGD_IS_RATIONAL_SRF(Srf);
	     j <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	     j++) {
	    Input[j][i] = R[j];
	}
    }

    /* Solve the interpolation problem. */
    for (j = !CAGD_IS_RATIONAL_SRF(Srf);
	 j <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	 j++) {
        if (!BzrCrvInterp2(CmpsCrv -> Points[j], Input[j], Deg + 1)) {
	    CagdCrvFree(CmpsCrv);
	    return BzrComposeSrfCrv(Srf, Crv);
	}
    }

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier curve Crv and a Bezier surface Srf, computes their          M
* coomposition Srf(Crv(t)).						     M
*   Crv must be a two dimensional curve completely contained in the          M
* parametric domain of Srf.                                                  M
*   See: "Freeform surfcae analysis using a hybrid of symbolic and numeric   M
* computation" by Gershon Elber, PhD thesis, University of Utah, 1992.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf, Crv:     The curve and surface to compose.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The resulting composition.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrComposeSrfCrv, composition                                            M
*****************************************************************************/
CagdCrvStruct *BzrComposeSrfCrv(const CagdSrfStruct *Srf,
				const CagdCrvStruct *Crv)
{
    IRIT_STATIC_DATA int
 	CrvCacheFactor = TRUE,
	CrvCacheUOrder = 0,
	CrvCacheVOrder = 0;
    IRIT_STATIC_DATA CagdCrvStruct
	**CrvUFactors = NULL,
	**CrvVFactors = NULL;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, k, l, CmpsOrder,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CTmpPoints,
        * const *SPoints = Srf -> Points;
    CagdCrvStruct *CrvUV, *CrvW, *CrvX, *CrvY, *CrvZ,
    	*CmpsCrv = NULL;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2)
	SYMB_FATAL_ERROR(SYMB_ERR_WRONG_PT_TYPE);

    CrvUV = CagdCoerceCrvTo(Crv, CAGD_IS_RATIONAL_CRV(Crv) ? CAGD_PT_P1_TYPE
							   : CAGD_PT_E1_TYPE,
			    FALSE);

    if (UOrder <= 4 &&
	VOrder <= 4 &&
	CrvUV -> Order <= 4 &&
	!CAGD_IS_RATIONAL_SRF(Srf) &&
	!CAGD_IS_RATIONAL_CRV(Crv)) {
        SymbComputeCurvePowersMaple(CrvUV, UOrder,
				    &CrvUFactors, &CrvCacheUOrder);

	CAGD_GEN_COPY(CrvUV -> Points[1], Crv -> Points[2],
		      sizeof(CagdRType) * Crv -> Length);

	SymbComputeCurvePowersMaple(CrvUV, VOrder,
				    &CrvVFactors, &CrvCacheVOrder);
	CrvCacheFactor = TRUE;
    }
    else {
        if (CrvUFactors != NULL) {
	    for (i = 0; CrvUFactors[i] != NULL; i++)
	        CagdCrvFree(CrvUFactors[i]);
	    IritFree(CrvUFactors);
	}

        CrvUFactors = SymbComputeCurvePowers(CrvUV, UOrder);

	CAGD_GEN_COPY(CrvUV -> Points[1], Crv -> Points[2],
		      sizeof(CagdRType) * Crv -> Length);

        if (CrvVFactors != NULL) {
	    for (i = 0; CrvVFactors[i] != NULL; i++)
	        CagdCrvFree(CrvVFactors[i]);
	    IritFree(CrvVFactors);
	}

	CrvVFactors = SymbComputeCurvePowers(CrvUV, VOrder);
	CrvCacheFactor = FALSE;
    }

    CagdCrvFree(CrvUV);

    CmpsCrv = BzrCrvNew(CrvUFactors[0] -> Length + 
			CrvVFactors[0] -> Length - 1, Srf -> PType);
    CmpsOrder = CmpsCrv -> Order;

    CTmpPoints = (CagdRType *) IritMalloc(CmpsOrder * sizeof(CagdRType));

    for (j = 0; j < VOrder; j++) {
        for (i = 0; i < UOrder; i++) {
	    BzrCrvMultPtsVecs(CrvUFactors[i] -> Points[1],
			      CrvUFactors[i] -> Order,
			      CrvVFactors[j] -> Points[1],
			      CrvVFactors[j] -> Order,
			      CTmpPoints);

	    for (k = !IsRational; k <= MaxCoord; k++) {
	        CagdRType
		    *CmpsPoints = CmpsCrv -> Points[k],
		    SPt = SPoints[k][CAGD_MESH_UV(Srf, i, j)];

		if (i == 0 && j == 0) {
		    for (l = 0; l < CmpsOrder; l++)
		        CmpsPoints[l] = CTmpPoints[l] * SPt;
		}
		else {
		    for (l = 0; l < CmpsOrder; l++)
		        CmpsPoints[l] += CTmpPoints[l] * SPt;
		}
	    }
	}
    }

    IritFree(CTmpPoints);

    if (CAGD_IS_RATIONAL_CRV(Crv)) {
	CagdCrvStruct *CTmp,
	    *NewCrvW = SymbCrvMult(CrvUFactors[UOrder], CrvVFactors[VOrder]);

	SymbCrvSplitScalar(CmpsCrv, &CrvW, &CrvX, &CrvY, &CrvZ);
	CTmp = SymbCrvMergeScalar(NewCrvW, CrvX, CrvY, CrvZ);
	CagdCrvFree(NewCrvW);
	CagdCrvFree(CmpsCrv);
	CmpsCrv = CTmp;

	if (CrvX)
	    CagdCrvFree(CrvX);
	if (CrvY)
	    CagdCrvFree(CrvY);
	if (CrvZ)
	    CagdCrvFree(CrvZ);
    }

    if (!CrvCacheFactor) {
	for (i = 0; CrvUFactors[i] != NULL; i++)
	    CagdCrvFree(CrvUFactors[i]);
	for (i = 0; CrvVFactors[i] != NULL; i++)
	    CagdCrvFree(CrvVFactors[i]);

	IritFree(CrvUFactors);
	CrvUFactors = NULL;
	IritFree(CrvVFactors);
	CrvVFactors = NULL;
    }

    return CmpsCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the factors of the Bernstein polynomials where crv is a scalar    *
* curve.								     *
*									     *
*   n            n              n-i         i				     *
*  B (crv(t)) = ( ) (1 - crv(t))    (crv(t))				     *
*   i            i							     *
*									     *
*   The curve crv(t) is a scalar, possibly rational curve.		     *
*   The constant 1 is equal to wcrv(t) if crv(t) is rational.		     *
*   If rational, the returned vector, index Order will contain               *
* wcrv(t)^(Order-1). 							     *
* See: "Freeform surface analysis using a hybrid of symbolic and numeric     *
* computation" by Gershon Elber, PhD thesis, University of Utah, 1992.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      Scalar curve to compute factor.                                *
*   Order:    Order is n + 1.	                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct **:   A vector of all possible factors for i equal to      *
*                       zero to i equal to n.                                *
*****************************************************************************/
static CagdCrvStruct **SymbComputeCurvePowers(const CagdCrvStruct *Crv, 
					      int Order)
{
    IRIT_STATIC_DATA int
	LastOrderStaticAlloc = -1;
    IRIT_STATIC_DATA CagdCrvStruct
	**CrvFactors1_Crv = NULL,
	**CrvFactorsCrv = NULL;
    int i;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct *Crv_1,
	**CrvFactors = (CagdCrvStruct **)
			    IritMalloc((Order + 2) * sizeof(CagdCrvStruct *));
    CagdRType *Points,
	Translate = 0.0;

    if (LastOrderStaticAlloc < Order) {
	LastOrderStaticAlloc = Order * 2 + 1;

	if (CrvFactors1_Crv != NULL) {
	    IritFree(CrvFactors1_Crv);
	    IritFree(CrvFactorsCrv);
	}

	CrvFactors1_Crv = (CagdCrvStruct **)
		  IritMalloc(LastOrderStaticAlloc * sizeof(CagdCrvStruct *));
	CrvFactorsCrv = (CagdCrvStruct **)
		  IritMalloc(LastOrderStaticAlloc * sizeof(CagdCrvStruct *));
    }

    if (IsRational) {
	CagdCrvStruct *CrvX, *CrvY, *CrvZ, *CrvW, *CTmp;

	SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
	Crv_1 = SymbCrvSub(CrvW, CrvX);

	/* Set CrvFactors[Order] to CrvW(t)^(Order-1) if curve is rational. */
	CrvFactors[Order] = CagdCrvCopy(CrvW);
	for (i = 1; i < Order - 1; i++) {
	    CTmp = BzrCrvMult(CrvFactors[Order], CrvW);
	    CagdCrvFree(CrvFactors[Order]);
	    CrvFactors[Order] = CTmp;
	}
	CrvFactors[Order + 1] = NULL;

	CagdCrvFree(CrvW);
	CagdCrvFree(CrvX);
    }
    else {
    	Crv_1 = CagdCrvCopy(Crv);
	Points = Crv_1 -> Points[1];
    	for (i = 0; i < Crv -> Order; i++, Points++)
    	    *Points = 1.0 - *Points;

	CrvFactors[Order] = NULL;
    }

    for (i = 0; i < Order; i++) {
    	if (i == 0) {
    	    CrvFactors1_Crv[0] = NULL;
    	    CrvFactorsCrv[0] = NULL;
	}
    	else if (i == 1) {
    	    CrvFactors1_Crv[1] = Crv_1;
    	    CrvFactorsCrv[1] = CagdCrvCopy(Crv);
    	}
    	else {
    	    CrvFactors1_Crv[i] = BzrCrvMult(CrvFactors1_Crv[i - 1], Crv_1);
    	    CrvFactorsCrv[i] = BzrCrvMult(CrvFactorsCrv[i - 1], Crv);
    	}
    }

    for (i = 0; i < Order; i++) {
	if (i == 0) {
	    CrvFactors[i] = CrvFactors1_Crv[Order - 1];
	}
	else if (i == Order - 1) {
	    CrvFactors[i] = CrvFactorsCrv[Order - 1];
	}
	else {
	    CrvFactors[i] = BzrCrvMult(CrvFactors1_Crv[Order - 1 - i],
				       CrvFactorsCrv[i]);
	}
    }

    for (i = 0; i < Order; i++) {
	CagdCrvTransform(CrvFactors[i],
			 &Translate, CagdIChooseK(i, Order - 1));
    }

    for (i = 1; i < Order - 1; i++) {
    	CagdCrvFree(CrvFactorsCrv[i]);
    	CagdCrvFree(CrvFactors1_Crv[i]);
    }

    return CrvFactors;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the factors of the Bernstein polynomials where crv is a scalar    *
* polynomial curve.							     *
*									     *
*   n            n              n-i         i				     *
*  B (crv(t)) = ( ) (1 - crv(t))    (crv(t))				     *
*   i            i							     *
*									     *
*   The curve crv(t) is a scalar, possibly rational curve.		     *
*   The constant 1 is equal to wcrv(t) if crv(t) is rational.		     *
*   If rational, the returned vector, index Order will contain               *
* wcrv(t)^(Order-1). 							     *
* See: "Freeform surface analysis using a hybrid of symbolic and numeric     *
* computation" by Gershon Elber, PhD thesis, University of Utah, 1992.	     *
*   This function precomputed (using Maple) the linear/quadratic/cubic cases *
* of the order of crv and Order n.					     *
*   This function cache the CrvFactors and realloc only if larger orders are *
* requested.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      Scalar polynomial curve of order 2, 3, 4 to compute factor.    *
*   Order:    Order is n.  Can be 2, 3 or 4.	                             *
*   CrvFactorsCache:  The curve factors to evaluate in a cached memory.      *
*   CachedOrder:      The order of the cached vector of curves.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct **:   A vector of all possible factors for i equal to      *
*                       zero to i equal to n.                                *
*****************************************************************************/
static void SymbComputeCurvePowersMaple(const CagdCrvStruct *Crv,
					int Order,
					CagdCrvStruct ***CrvFactorsCache,
					int *CachedOrder)
{
    int i,
	COrder = Crv -> Order;
    CagdRType *Pts, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10,
	t11, t12, t13, t14, t15, t16, t17, t18, t19, t20, t21, t22, t27,
	t24,t30, t36, t25, t26, t28, t29, t31, t38, t47, t77, t23, t40, t44,
	t53, t55, t71, t80, t32, t41,
	*CPts = Crv -> Points[1];
    CagdCrvStruct
	**CrvFactors = *CrvFactorsCache;

    /* Realloc the cache if necessary and set the proper order. */
    if (CrvFactors != NULL && (COrder - 1) * (Order - 1) + 1 > *CachedOrder) {
	for (i = 0; CrvFactors[i] != NULL; i++)
	    CagdCrvFree(CrvFactors[i]);
	IritFree(CrvFactors);
	*CrvFactorsCache = NULL;
    }
    if (*CrvFactorsCache == NULL) {
	*CachedOrder = 2 * ((COrder - 1) * (Order - 1) + 1);

	CrvFactors = *CrvFactorsCache = (CagdCrvStruct **)
	    IritMalloc((*CachedOrder + 2) * sizeof(CagdCrvStruct *));

	for (i = 0; i <= *CachedOrder; i++)
	    CrvFactors[i] = BzrCrvNew(*CachedOrder,  CAGD_PT_E1_TYPE);
	CrvFactors[i] = NULL;
    }
    for (i = 0; i <= *CachedOrder; i++)
	CrvFactors[i] -> Order = 
	    CrvFactors[i] -> Length = (COrder - 1) * (Order - 1) + 1;

    switch (Crv -> Order) {
	case 2:
	    switch (Order) {
		case 2:
		    Pts = CrvFactors[0] -> Points[1];
		    Pts[1] = 1.0-CPts[1];
		    Pts[0] = 1.0-CPts[0];

		    Pts = CrvFactors[1] -> Points[1];
		    Pts[1] = CPts[1];
		    Pts[0] = CPts[0];
		    break;
		case 3:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-2.0*CPts[0]+t2;
		    t4 = CPts[1]*CPts[1];
		    Pts[2] = 1.0-2.0*CPts[1]+t4;
		    Pts[1] = 1.0-CPts[0]-CPts[1]+CPts[0]*CPts[1];

		    Pts = CrvFactors[1] -> Points[1];
		    t1 = CPts[1]*CPts[1];
		    Pts[2] = 2.0*CPts[1]-2.0*t1;
		    t3 = CPts[0]*CPts[0];
		    Pts[0] = 2.0*CPts[0]-2.0*t3;
		    Pts[1] = CPts[0]+CPts[1]-2.0*CPts[0]*CPts[1];

		    Pts = CrvFactors[2] -> Points[1];
		    Pts[2] = CPts[1]*CPts[1];
		    Pts[0] = CPts[0]*CPts[0];
		    Pts[1] = CPts[0]*CPts[1];
		   break;
		case 4:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[1]*CPts[1];
		    Pts[3] = 1.0-3.0*CPts[1]+3.0*t2-t2*CPts[1];
		    t6 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-3.0*CPts[0]+3.0*t6-t6*CPts[0];
		    t12 = 2.0*CPts[0]*CPts[1];
		    Pts[2] = 1.0-CPts[0]-CPts[0]*t2-2.0*CPts[1]+t2+t12;
		    Pts[1] = 1.0-2.0*CPts[0]+t6+t12-CPts[1]-t6*CPts[1];

		    Pts = CrvFactors[1] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 3.0*CPts[0]-6.0*t2+3.0*CPts[0]*t2;
		    t7 = CPts[1]*CPts[1];
		    Pts[3] = 3.0*CPts[1]-6.0*t7+3.0*t7*CPts[1];
		    t16 = 4.0*CPts[0]*CPts[1];
		    Pts[2] = CPts[0]+3.0*CPts[0]*t7+2.0*CPts[1]-2.0*t7-t16;
		    Pts[1] = 2.0*CPts[0]-2.0*t2-t16+CPts[1]+3.0*t2*CPts[1];

		    Pts = CrvFactors[2] -> Points[1];
		    t1 = CPts[0]*CPts[0];
		    Pts[0] = 3.0*t1-3.0*t1*CPts[0];
		    t4 = CPts[1]*CPts[1];
		    Pts[3] = 3.0*t4-3.0*t4*CPts[1];
		    t8 = 2.0*CPts[0]*CPts[1];
		    Pts[2] = t4+t8-3.0*CPts[0]*t4;
		    Pts[1] = t1-3.0*t1*CPts[1]+t8;

		    Pts = CrvFactors[3] -> Points[1];
		    t1 = CPts[1]*CPts[1];
		    Pts[3] = t1*CPts[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = CPts[0]*t2;
		    Pts[2] = CPts[0]*t1;
		    Pts[1] = t2*CPts[1];
		    break;
	    }
            break;
	case 3:
	    switch (Order) {
	        case 2:
	            Pts = CrvFactors[0] -> Points[1];
		    Pts[1] = 1.0-CPts[1];
		    Pts[0] = 1.0-CPts[0];
		    Pts[2] = 1.0-CPts[2];

		    Pts = CrvFactors[1] -> Points[1];
		    Pts[2] = CPts[2];
		    Pts[1] = CPts[1];
		    Pts[0] = CPts[0];
		    break;
		case 3:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-2.0*CPts[0]+t2;
		    Pts[1] = 1.0-CPts[0]-CPts[1]+CPts[0]*CPts[1];
		    t5 = CPts[2]*CPts[2];
		    Pts[4] = 1.0-2.0*CPts[2]+t5;
		    Pts[3] = 1.0-CPts[2]-CPts[1]+CPts[1]*CPts[2];
		    t12 = CPts[1]*CPts[1];
		    Pts[2] = 1.0-CPts[0]/3.0+CPts[0]*CPts[2]/3.0-CPts[2]/3.0-4.0/3.0*CPts[1]+2.0/3.0*t12;

		    Pts = CrvFactors[1] -> Points[1];
		    t1 = CPts[0]*CPts[0];
		    Pts[0] = 2.0*CPts[0]-2.0*t1;
		    Pts[1] = CPts[0]+CPts[1]-2.0*CPts[0]*CPts[1];
		    t5 = CPts[2]*CPts[2];
		    Pts[4] = 2.0*CPts[2]-2.0*t5;
		    Pts[3] = CPts[2]+CPts[1]-2.0*CPts[1]*CPts[2];
		    t14 = CPts[1]*CPts[1];
		    Pts[2] = CPts[0]/3.0-2.0/3.0*CPts[0]*CPts[2]+CPts[2]/3.0+4.0/3.0*CPts[1]-4.0/3.0*t14;

		    Pts = CrvFactors[2] -> Points[1];
		    Pts[0] = CPts[0]*CPts[0];
		    Pts[1] = CPts[0]*CPts[1];
		    Pts[4] = CPts[2]*CPts[2];
		    t3 = CPts[1]*CPts[1];
		    Pts[2] = CPts[0]*CPts[2]/3.0+2.0/3.0*t3;
		    Pts[3] = CPts[1]*CPts[2];
		    break;
		case 4:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-3.0*CPts[0]+3.0*t2-t2*CPts[0];
		    t6 = CPts[2]*CPts[2];
		    Pts[6] = 1.0-3.0*CPts[2]+3.0*t6-t6*CPts[2];
		    t11 = 8.0/5.0*CPts[1];
		    t12 = CPts[1]*CPts[2];
		    t15 = CPts[1]*CPts[1];
		    t20 = 4.0/5.0*t15;
		    t21 = CPts[0]*CPts[2];
		    t22 = 2.0/5.0*t21;
		    Pts[4] = 1.0+t6/5.0-6.0/5.0*CPts[2]-t11+8.0/5.0*t12-CPts[0]/5.0-4.0/5.0*t15*CPts[2]-CPts[0]*t6/
		      5.0+t20+t22;
		    Pts[5] = 1.0-2.0*CPts[2]-CPts[1]+t6-CPts[1]*t6+2.0*t12;
		    t27 = CPts[0]*CPts[1];
		    Pts[1] = 1.0-2.0*CPts[0]-CPts[1]+t2+2.0*t27-t2*CPts[1];
		    Pts[2] = 1.0-6.0/5.0*CPts[0]-t11+t2/5.0+8.0/5.0*t27+t20-t2*CPts[2]/5.0-4.0/5.0*CPts[0]*t15-
		      CPts[2]/5.0+t22;
		    Pts[3] = 1.0-3.0/5.0*CPts[0]-9.0/5.0*CPts[1]+3.0/5.0*t27+6.0/5.0*t15-2.0/5.0*t15*CPts[1]-3.0
		      /5.0*t27*CPts[2]-3.0/5.0*CPts[2]+3.0/5.0*t21+3.0/5.0*t12;

		    Pts = CrvFactors[1] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 3.0*CPts[0]-6.0*t2+3.0*t2*CPts[0];
		    t7 = CPts[2]*CPts[2];
		    Pts[6] = 3.0*CPts[2]-6.0*t7+3.0*t7*CPts[2];
		    t13 = CPts[0]*CPts[1];
		    t15 = CPts[1]*CPts[1];
		    t22 = CPts[0]*CPts[2];
		    t24 = CPts[1]*CPts[2];
		    Pts[3] = 3.0/5.0*CPts[0]+9.0/5.0*CPts[1]-6.0/5.0*t13-12.0/5.0*t15+6.0/5.0*t15*CPts[1]+9.0/
		      5.0*t13*CPts[2]+3.0/5.0*CPts[2]-6.0/5.0*t22-6.0/5.0*t24;
		    t27 = 8.0/5.0*CPts[1];
		    t30 = 8.0/5.0*t15;
		    t36 = 4.0/5.0*t22;
		    Pts[2] = 6.0/5.0*CPts[0]+t27-2.0/5.0*t2-16.0/5.0*t13-t30+3.0/5.0*t2*CPts[2]+12.0/5.0*CPts[0]
		      *t15+CPts[2]/5.0-t36;
		    Pts[1] = 2.0*CPts[0]+CPts[1]-2.0*t2-4.0*t13+3.0*t2*CPts[1];
		    Pts[5] = 2.0*CPts[2]+CPts[1]-2.0*t7+3.0*CPts[1]*t7-4.0*t24;
		    Pts[4] = -2.0/5.0*t7+6.0/5.0*CPts[2]+t27-16.0/5.0*t24+CPts[0]/5.0+12.0/5.0*t15*CPts[2]+3.0/
		      5.0*CPts[0]*t7-t30-t36;

		    Pts = CrvFactors[2] -> Points[1];
		    t1 = CPts[0]*CPts[0];
		    Pts[0] = 3.0*t1-3.0*t1*CPts[0];
		    t4 = CPts[2]*CPts[2];
		    t7 = CPts[1]*CPts[2];
		    Pts[5] = t4-3.0*CPts[1]*t4+2.0*t7;
		    t13 = CPts[1]*CPts[1];
		    t14 = 4.0/5.0*t13;
		    t17 = CPts[0]*CPts[2];
		    t18 = 2.0/5.0*t17;
		    Pts[4] = -3.0/5.0*CPts[0]*t4+t4/5.0+8.0/5.0*t7+t14-12.0/5.0*t13*CPts[2]+t18;
		    t24 = CPts[0]*CPts[1];
		    Pts[3] = 3.0/5.0*t7+6.0/5.0*t13+3.0/5.0*t17-6.0/5.0*t13*CPts[1]+3.0/5.0*t24-9.0/
		      5.0*t24*CPts[2];
		    Pts[2] = t1/5.0+8.0/5.0*t24+t14-3.0/5.0*t1*CPts[2]-12.0/5.0*CPts[0]*t13+t18;
		    Pts[1] = t1+2.0*t24-3.0*t1*CPts[1];
		    Pts[6] = 3.0*t4-3.0*t4*CPts[2];

		    Pts = CrvFactors[3] -> Points[1];
		    t1 = CPts[2]*CPts[2];
		    Pts[5] = t1*CPts[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = t2*CPts[0];
		    Pts[6] = t1*CPts[2];
		    t5 = CPts[1]*CPts[1];
		    Pts[4] = t1*CPts[0]/5.0+4.0/5.0*t5*CPts[2];
		    Pts[3] = 3.0/5.0*CPts[0]*CPts[1]*CPts[2]+2.0/5.0*t5*CPts[1];
		    Pts[2] = t2*CPts[2]/5.0+4.0/5.0*CPts[0]*t5;
		    Pts[1] = t2*CPts[1];
		    break;
	    }
            break;
	case 4:
	    switch (Order) {
	        case 2:
	            Pts = CrvFactors[0] -> Points[1];
		    Pts[3] = 1.0-CPts[3];
		    Pts[0] = 1.0-CPts[0];
		    Pts[1] = 1.0-CPts[1];
		    Pts[2] = 1.0-CPts[2];

		    Pts = CrvFactors[1] -> Points[1];
		    Pts[3] = CPts[3];
		    Pts[0] = CPts[0];
		    Pts[1] = CPts[1];
		    Pts[2] = CPts[2];
		    break;
		case 3:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-2.0*CPts[0]+t2;
		    Pts[1] = 1.0-CPts[0]-CPts[1]+CPts[0]*CPts[1];
		    t5 = CPts[3]*CPts[3];
		    Pts[6] = 1.0-2.0*CPts[3]+t5;
		    Pts[5] = 1.0-CPts[2]-CPts[3]+CPts[2]*CPts[3];
		    t10 = CPts[2]*CPts[2];
		    Pts[4] = 1.0-6.0/5.0*CPts[2]-2.0/5.0*CPts[3]-2.0/5.0*CPts[1]+3.0/5.0*t10+2.0/5.0*CPts[1]*CPts[3];
		    Pts[3] = 1.0-9.0/10.0*CPts[1]-9.0/10.0*CPts[2]-CPts[3]/10.0-CPts[0]/10.0+9.0/10.0*CPts[1]*CPts[2]+CPts[0]*CPts[3]/
		      10.0;
		    t24 = CPts[1]*CPts[1];
		    Pts[2] = 1.0-2.0/5.0*CPts[0]-6.0/5.0*CPts[1]+3.0/5.0*t24-2.0/5.0*CPts[2]+2.0/5.0*CPts[0]*CPts[2];

		    Pts = CrvFactors[1] -> Points[1];
		    t1 = CPts[3]*CPts[3];
		    Pts[6] = 2.0*CPts[3]-2.0*t1;
		    t3 = CPts[0]*CPts[0];
		    Pts[0] = 2.0*CPts[0]-2.0*t3;
		    t10 = CPts[2]*CPts[2];
		    Pts[4] = 6.0/5.0*CPts[2]+2.0/5.0*CPts[3]+2.0/5.0*CPts[1]-4.0/5.0*CPts[1]*CPts[3]-6.0/5.0*t10;
		    Pts[5] = CPts[2]+CPts[3]-2.0*CPts[2]*CPts[3];
		    Pts[1] = CPts[0]+CPts[1]-2.0*CPts[0]*CPts[1];
		    t18 = CPts[1]*CPts[1];
		    Pts[2] = 2.0/5.0*CPts[0]+6.0/5.0*CPts[1]-6.0/5.0*t18+2.0/5.0*CPts[2]-4.0/5.0*CPts[0]*CPts[2];
		    Pts[3] = 9.0/10.0*CPts[1]+9.0/10.0*CPts[2]+CPts[3]/10.0+CPts[0]/10.0-9.0/5.0*CPts[1]*CPts[2]-CPts[0]*CPts[3]/5.0;

		    Pts = CrvFactors[2] -> Points[1];
		    Pts[6] = CPts[3]*CPts[3];
		    Pts[0] = CPts[0]*CPts[0];
		    Pts[1] = CPts[0]*CPts[1];
		    Pts[5] = CPts[2]*CPts[3];
		    t3 = CPts[2]*CPts[2];
		    Pts[4] = 2.0/5.0*CPts[1]*CPts[3]+3.0/5.0*t3;
		    Pts[3] = 9.0/10.0*CPts[1]*CPts[2]+CPts[0]*CPts[3]/10.0;
		    t11 = CPts[1]*CPts[1];
		    Pts[2] = 2.0/5.0*CPts[0]*CPts[2]+3.0/5.0*t11;
		    break;
		case 4:
		    Pts = CrvFactors[0] -> Points[1];
		    t2 = CPts[3]*CPts[3];
		    Pts[9] = 1.0-3.0*CPts[3]+3.0*t2-t2*CPts[3];
		    t6 = CPts[0]*CPts[0];
		    Pts[0] = 1.0-3.0*CPts[0]+3.0*t6-t6*CPts[0];
		    t11 = CPts[2]*CPts[2];
		    t13 = CPts[0]*CPts[1];
		    t15 = CPts[1]*CPts[1];
		    t17 = CPts[0]*CPts[3];
		    t18 = t17/7.0;
		    t26 = CPts[0]*CPts[2];
		    t28 = CPts[1]*CPts[2];
		    t29 = 9.0/7.0*t28;
		    t31 = CPts[1]*CPts[3];
		    Pts[4] = 1.0-5.0/14.0*CPts[0]-10.0/7.0*CPts[1]+3.0/14.0*t11+t13/7.0+9.0/14.0*t15+t18-
		      t13*CPts[3]/7.0-3.0/14.0*CPts[0]*t11-9.0/14.0*t15*CPts[2]-15.0/14.0*CPts[2]+3.0/7.0*t26+t29-CPts[3]/7.0+
			t31/7.0;
		    t38 = t17/14.0;
		    t47 = 9.0/14.0*t28;
		    Pts[3] = 1.0-5.0/7.0*CPts[0]-45.0/28.0*CPts[1]+t6/28.0+9.0/14.0*t13+27.0/28.0*t15+t38
		      -9.0/14.0*t13*CPts[2]-9.0/28.0*t15*CPts[1]-t6*CPts[3]/28.0-9.0/14.0*CPts[2]+9.0/14.0*t26+t47-CPts[3]/
			28.0;
		    Pts[2] = 1.0-5.0/4.0*CPts[0]-3.0/2.0*CPts[1]+t6/4.0+3.0/2.0*t13+3.0/4.0*t15-3.0/4.0*CPts[0]*
		      t15-t6*CPts[2]/4.0-CPts[2]/4.0+t26/2.0;
		    Pts[1] = 1.0-2.0*CPts[0]-CPts[1]+t6+2.0*t13-t6*CPts[1];
		    t77 = CPts[2]*CPts[3];
		    Pts[5] = 1.0-CPts[0]/7.0-15.0/14.0*CPts[1]+9.0/14.0*t11+3.0/14.0*t15+t18-t26*CPts[3]/7.0-9.0
		      /14.0*CPts[1]*t11-3.0/14.0*t15*CPts[3]-10.0/7.0*CPts[2]+t26/7.0+t29-5.0/14.0*CPts[3]+3.0/7.0*t31+
			t77/7.0;
		    Pts[7] = 1.0-CPts[1]*t2/4.0+3.0/2.0*t77-3.0/2.0*CPts[2]-5.0/4.0*CPts[3]+t2/4.0-CPts[1]/4.0+3.0/
		      4.0*t11-3.0/4.0*t11*CPts[3]+t31/2.0;
		    Pts[6] = 1.0-CPts[0]/28.0-9.0/14.0*CPts[1]+27.0/28.0*t11+t38-9.0/14.0*t28*CPts[3]-9.0/28.0*
		      t11*CPts[2]-CPts[0]*t2/28.0-45.0/28.0*CPts[2]+t47-5.0/7.0*CPts[3]+9.0/14.0*t31+9.0/14.0*t77+t2/28.0
			;
		    Pts[8] = 1.0+2.0*t77-CPts[2]*t2-CPts[2]-2.0*CPts[3]+t2;

		    Pts = CrvFactors[1] -> Points[1];
		    t1 = CPts[0]*CPts[1];
		    t4 = CPts[0]*CPts[0];
		    Pts[1] = -4.0*t1+2.0*CPts[0]+CPts[1]-2.0*t4+3.0*t4*CPts[1];
		    t12 = CPts[1]*CPts[1];
		    t14 = CPts[0]*CPts[3];
		    t15 = t14/7.0;
		    t23 = CPts[0]*CPts[2];
		    t25 = CPts[1]*CPts[2];
		    t26 = 9.0/7.0*t25;
		    Pts[3] = 5.0/7.0*CPts[0]+45.0/28.0*CPts[1]-t4/14.0-9.0/7.0*t1-27.0/14.0*t12-t15+27.0/
		      14.0*t1*CPts[2]+27.0/28.0*t12*CPts[1]+3.0/28.0*t4*CPts[3]+9.0/14.0*CPts[2]-9.0/7.0*t23-t26+CPts[3]/28.0;
		    Pts[2] = -3.0*t1+5.0/4.0*CPts[0]+3.0/2.0*CPts[1]-t4/2.0-3.0/2.0*t12+9.0/4.0*CPts[0]*t12+3.0/
		      4.0*t4*CPts[2]+CPts[2]/4.0-t23;
		    t40 = CPts[2]*CPts[2];
		    t44 = 2.0/7.0*t14;
		    t53 = 18.0/7.0*t25;
		    t55 = CPts[1]*CPts[3];
		    Pts[4] = 5.0/14.0*CPts[0]+10.0/7.0*CPts[1]-3.0/7.0*t40-2.0/7.0*t1-9.0/7.0*t12-t44+3.0/
		      7.0*t1*CPts[3]+9.0/14.0*CPts[0]*t40+27.0/14.0*t12*CPts[2]+15.0/14.0*CPts[2]-6.0/7.0*t23-t53+CPts[3]/7.0
			-2.0/7.0*t55;
		    t71 = CPts[2]*CPts[3];
		    Pts[5] = CPts[0]/7.0+15.0/14.0*CPts[1]-9.0/7.0*t40-3.0/7.0*t12-t44+3.0/7.0*t23*CPts[3]+27.0/
		      14.0*CPts[1]*t40+9.0/14.0*t12*CPts[3]+10.0/7.0*CPts[2]-2.0/7.0*t23-t53+5.0/14.0*CPts[3]-6.0/7.0*t55
			-2.0/7.0*t71;
		    t80 = CPts[3]*CPts[3];
		    Pts[6] = CPts[0]/28.0+9.0/14.0*CPts[1]-27.0/14.0*t40-t15+27.0/14.0*t25*CPts[3]+27.0/28.0*t40
		      *CPts[2]+3.0/28.0*CPts[0]*t80+45.0/28.0*CPts[2]-t26+5.0/7.0*CPts[3]-9.0/7.0*t55-9.0/7.0*t71-t80/
			14.0;
		    Pts[7] = CPts[1]/4.0-3.0/2.0*t40+3.0/4.0*CPts[1]*t80+9.0/4.0*t40*CPts[3]+3.0/2.0*CPts[2]+5.0/4.0*
		      CPts[3]-t55-3.0*t71-t80/2.0;
		    Pts[9] = 3.0*CPts[3]-6.0*t80+3.0*t80*CPts[3];
		    Pts[8] = 3.0*CPts[2]*t80+CPts[2]+2.0*CPts[3]-4.0*t71-2.0*t80;
		    Pts[0] = 3.0*CPts[0]-6.0*t4+3.0*t4*CPts[0];

		    Pts = CrvFactors[2] -> Points[1];
		    t1 = CPts[3]*CPts[3];
		    Pts[9] = 3.0*t1-3.0*t1*CPts[3];
		    t4 = CPts[0]*CPts[0];
		    Pts[0] = 3.0*t4-3.0*t4*CPts[0];
		    t9 = CPts[2]*CPts[3];
		    Pts[8] = -3.0*t1*CPts[2]+2.0*t9+t1;
		    t12 = CPts[0]*CPts[1];
		    t14 = CPts[1]*CPts[1];
		    t16 = CPts[0]*CPts[2];
		    t18 = CPts[0]*CPts[3];
		    t19 = t18/14.0;
		    t26 = CPts[1]*CPts[2];
		    t27 = 9.0/14.0*t26;
		    Pts[3] = t4/28.0+9.0/14.0*t12+27.0/28.0*t14+9.0/14.0*t16+t19-27.0/14.0*t12*CPts[2]
		      -27.0/28.0*t14*CPts[1]-3.0/28.0*t4*CPts[3]+t27;
		    t28 = CPts[2]*CPts[2];
		    t32 = t18/7.0;
		    t40 = 9.0/7.0*t26;
		    t41 = CPts[1]*CPts[3];
		    Pts[4] = 3.0/14.0*t28+t12/7.0+9.0/14.0*t14+t32-3.0/7.0*t12*CPts[3]-9.0/14.0*CPts[0]*t28
		      -27.0/14.0*t14*CPts[2]+3.0/7.0*t16+t40+t41/7.0;
		    Pts[5] = 9.0/14.0*t28+3.0/14.0*t14+t32-3.0/7.0*t16*CPts[3]-27.0/14.0*CPts[1]*t28-9.0/
		      14.0*t14*CPts[3]+t16/7.0+t40+3.0/7.0*t41+t9/7.0;
		    Pts[6] = 27.0/28.0*t28+t19-27.0/14.0*t26*CPts[3]-27.0/28.0*t28*CPts[2]-3.0/28.0*CPts[0]*t1+
		      t27+9.0/14.0*t41+9.0/14.0*t9+t1/28.0;
		    Pts[7] = 3.0/4.0*t28-3.0/4.0*CPts[1]*t1-9.0/4.0*t28*CPts[3]+t41/2.0+3.0/2.0*t9+t1/4.0;
		    Pts[1] = t4+2.0*t12-3.0*t4*CPts[1];
		    Pts[2] = t4/4.0+3.0/2.0*t12+3.0/4.0*t14-9.0/4.0*CPts[0]*t14-3.0/4.0*t4*CPts[2]+t16/2.0;

		    Pts = CrvFactors[3] -> Points[1];
		    t1 = CPts[3]*CPts[3];
		    Pts[9] = t1*CPts[3];
		    t2 = CPts[0]*CPts[0];
		    Pts[0] = t2*CPts[0];
		    t5 = CPts[2]*CPts[2];
		    Pts[7] = CPts[1]*t1/4.0+3.0/4.0*t5*CPts[3];
		    Pts[8] = t1*CPts[2];
		    Pts[1] = t2*CPts[1];
		    t8 = CPts[1]*CPts[1];
		    Pts[2] = 3.0/4.0*CPts[0]*t8+CPts[2]*t2/4.0;
		    t13 = CPts[0]*CPts[1];
		    Pts[3] = 9.0/14.0*t13*CPts[2]+9.0/28.0*t8*CPts[1]+t2*CPts[3]/28.0;
		    Pts[4] = t13*CPts[3]/7.0+3.0/14.0*CPts[0]*t5+9.0/14.0*t8*CPts[2];
		    Pts[5] = CPts[0]*CPts[2]*CPts[3]/7.0+9.0/14.0*CPts[1]*t5+3.0/14.0*t8*CPts[3];
		    Pts[6] = 9.0/14.0*CPts[1]*CPts[2]*CPts[3]+9.0/28.0*t5*CPts[2]+CPts[0]*t1/28.0;
		    break;
	    }
            break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a surface patch that is a general rectangle in the domain of    M
* given surface.                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     In which the four UVij corners resides.                         M
*   UV00, UV01, UV10, UV11:  The four corners of the patch to extract.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A patch inside Srf that is a rectangle through UVij   M
*                      in the domain of Srf.  Only the four boundaries will  M
*                      be precisely reconstructed.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeSrfCrv                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbComposeSrfPatch                                                      M
*****************************************************************************/
CagdSrfStruct *SymbComposeSrfPatch(const CagdSrfStruct *Srf,
				   const CagdUVType UV00,
				   const CagdUVType UV01,
				   const CagdUVType UV10,
				   const CagdUVType UV11)
{
    CagdCrvStruct *Crv1E3, *Crv2E3, *Crv3E3, *Crv4E3,
        *Crv1UV = CagdMergeUvUv(UV00, UV01),             /* Left. */
        *Crv2UV = CagdMergeUvUv(UV10, UV11),             /* Right. */
        *Crv3UV = CagdMergeUvUv(UV00, UV10),             /* Top reversed. */
	*Crv4UV = CagdMergeUvUv(UV01, UV11);             /* Bottom. */
    CagdSrfStruct *BSumSrf;

    /* If both V values are zero, will create a E1 curve. Remedy that: */
    SYMB_VERIFY_E2_CRV(Crv1UV);
    SYMB_VERIFY_E2_CRV(Crv2UV);
    SYMB_VERIFY_E2_CRV(Crv3UV);
    SYMB_VERIFY_E2_CRV(Crv4UV);

    Crv1E3 = SymbComposeSrfCrv(Srf, Crv1UV),
    Crv2E3 = SymbComposeSrfCrv(Srf, Crv2UV),
    Crv3E3 = SymbComposeSrfCrv(Srf, Crv3UV),
    Crv4E3 = SymbComposeSrfCrv(Srf, Crv4UV);

    CagdCrvFree(Crv1UV);
    CagdCrvFree(Crv2UV);
    CagdCrvFree(Crv3UV);
    CagdCrvFree(Crv4UV);

    BSumSrf = CagdBoolSumSrf(Crv1E3, Crv2E3, Crv3E3, Crv4E3);

    CagdCrvFree(Crv1E3);
    CagdCrvFree(Crv2E3);
    CagdCrvFree(Crv3E3);
    CagdCrvFree(Crv4E3);

    return BSumSrf;
}
