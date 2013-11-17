/******************************************************************************
* SymbZero.c - computes the zeros and extremes of a given object.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 93.					      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"

#define SET_DEFAULT_IRIT_EPS	1e-3
#define SYMB_ZERO_DOUBLE_ACCURACY 1e-14    /* Double float point accuracy. */
#define ZERO_IRIT_APX_EQ(x, y)	(IRIT_FABS((x) - (y)) < GlblSetEpsilon * 10)
#define SYMB_ZERO_MID_RATIO	0.5000301060
#define SYMB_ZERO_NR_MAX_ITER	10
#define SYMB_DOUBLE_ROOT_SLOPE	1e-3

IRIT_STATIC_DATA CagdBType
    GlblNoSolsOnEndPts = TRUE;
IRIT_STATIC_DATA CagdPtStruct
    *GlblPtList = NULL;
IRIT_STATIC_DATA CagdRType CrvTMin, CrvTMax,
    GlblSetEpsilon = SET_DEFAULT_IRIT_EPS;

static void SymbScalarCrvLowDegZeroSet(CagdCrvStruct *Crv);
static void SymbScalarCrvZeroSet(CagdCrvStruct *Crv);
static CagdRType SymbScalarCrvNewtonRaphson(CagdCrvStruct *Crv,
					    CagdRType t,
					    CagdRType TMin,
					    CagdRType TMax);
#ifdef DEBUG_TEST_COMPARE_ZERO_SET_SOLS
static void SymbScalarCrvZeroSetIN(CagdCrvStruct *Crv);
#endif /* DEBUG_TEST_COMPARE_ZERO_SET_SOLS */
static void SymbScalarCrvZeroSetINAux(CagdCrvStruct *Crv,
				      CagdCrvStruct *OrigCrv);
static void SymbScalarCrvZeroSetINRecurseOnInterval(CagdCrvStruct *Crv,
						    CagdCrvStruct *OrigCrv,
						    CagdRType TMin,
						    CagdRType TMax);
static void SymbScalarCrvExtremSet(CagdCrvStruct *Crv);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the zero set of a given curve, in given axis (0/1-3 for W/X-Z).   M
*   Returned is a list of the zero set points holding the parameter values   M
* at Pt[0] of each point.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute its zero set.                                     M
*   Axis:       The axis of Crv to compute zero set for, W = 0, X = 1, etc.  M
*   Epsilon:    Tolerance control.                                           M
*   NoSolsOnEndPts: If TRUE, solutions at the end of the domain are purged.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   List of parameter values form which Crv is zero in     M
*                     axis Axis.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvZeroSet, zero set, symbolic computation                           M
*****************************************************************************/
CagdPtStruct *SymbCrvZeroSet(const CagdCrvStruct *Crv,
			     int Axis,
			     CagdRType Epsilon,
			     CagdBType NoSolsOnEndPts)
{
    return SymbCrvConstSet(Crv, Axis, Epsilon, 0.0, NoSolsOnEndPts);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the extrema set of a given curve, in given axis (0/1-3 for      M
* W/X-Z).								     M
*   Returned is a list of the extreme set points holding the parameter       M
* values at Pt[0] of each point.					     M
*   One could compute the derivative of the curve and find its zero set.     M
*   However, for rational curves, this will double the degree and slow down  M
* the computation considerably.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute its extrema set.                                    M
*   Axis:     The axis of Crv to compute extrema set for, W = 0, X = 1, etc. M
*   Epsilon:  Tolerance control.                                             M
*   NoSolsOnEndPts: If TRUE, solutions at the end of the domain are purged.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   List of parameter values form which Crv has an         M
*                     extrema value in axis Axis.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvExtremSet, extrema set, symbolic computation                      M
*****************************************************************************/
CagdPtStruct *SymbCrvExtremSet(const CagdCrvStruct *Crv,
			       int Axis,
			       CagdRType Epsilon,
			       CagdBType NoSolsOnEndPts)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *TCrv, *NewCrv,
	*ScalarCrv = NULL;

    GlblSetEpsilon = Epsilon;
    GlblNoSolsOnEndPts = NoSolsOnEndPts;

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);

    switch (Axis) {
	case 0:
	    if (CrvW)
		ScalarCrv = CrvW;
	    else
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    break;
	case 1:
	    if (CrvX)
		ScalarCrv = CrvX;
	    else
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    break;
	case 2:
	    if (CrvY)
		ScalarCrv = CrvY;
	    else
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    break;
	case 3:
	    if (CrvZ)
		ScalarCrv = CrvZ;
	    else
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
    }

    if (Axis > 0) {
        NewCrv = SymbCrvMergeScalar(CrvW, ScalarCrv, NULL, NULL);
	if (CrvW)
	    CagdCrvFree(CrvW);
    }
    else
        NewCrv = ScalarCrv;

    if (CrvX)
	CagdCrvFree(CrvX);
    if (CrvY)
	CagdCrvFree(CrvY);
    if (CrvZ)
	CagdCrvFree(CrvZ);
    
    GlblPtList = NULL;
    if (CAGD_IS_BEZIER_CRV(Crv)) {
	/* We need to save domains. */
	TCrv = CagdCnvrtBzr2BspCrv(NewCrv);
	CagdCrvFree(NewCrv);
	NewCrv = TCrv;
    }

    CagdCrvDomain(NewCrv, &CrvTMin, &CrvTMax);
    SymbScalarCrvExtremSet(NewCrv);
    CagdCrvFree(NewCrv);

    return GlblPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the constant set of a given curve, in the given axis (0/1-3 for M
* W/X-Z).							 	     M
*   Returned is a list of the constant set points holding the parameter	     M
* values at Pt[0] of each point.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:     To compute its constant set.                                   M
*   Axis:     The axis of Crv to compute constant set for, W = 0, X = 1, etc.M
*   Epsilon:  Tolerance control.                                             M
*   ConstVal:   The value at which to compute the constant set.		     M
*   NoSolsOnEndPts: If TRUE, solutions at the end of the domain are purged.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   List of parameter values form which Crv has an         M
*                     value of ConstVal in axis Axis.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvConstSet, constant set, zero set, symbolic computation            M
*****************************************************************************/
CagdPtStruct *SymbCrvConstSet(const CagdCrvStruct *CCrv,
			      int Axis,
			      CagdRType Epsilon,
			      CagdRType ConstVal,
			      CagdBType NoSolsOnEndPts)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *TCrv,
	*Crv = NULL;

    GlblSetEpsilon = Epsilon;
    GlblNoSolsOnEndPts = NoSolsOnEndPts;

    SymbCrvSplitScalar(CCrv, &CrvW, &CrvX, &CrvY, &CrvZ);

    switch (Axis) {
	case 0:
	    if (CrvW)
		IRIT_SWAP(CagdCrvStruct *, Crv, CrvW)
	    break;
	case 1:
	    if (CrvX)
		IRIT_SWAP(CagdCrvStruct *, Crv, CrvX)
	    break;
	case 2:
	    if (CrvY)
		IRIT_SWAP(CagdCrvStruct *, Crv, CrvY)
	    break;
	case 3:
	    if (CrvZ)
		IRIT_SWAP(CagdCrvStruct *, Crv, CrvZ)
	    break;
	default:
	    break;
    }

    if (Crv == NULL)
        SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);

    if (CrvW) {
        if (ConstVal != 0.0) {
	    int i;
            CagdRType
	        *SPts = Crv -> Points[1],
	        *WPts = CrvW -> Points[1];

	    for (i = 0; i < Crv -> Length; i++)
	        *SPts++ -= ConstVal * *WPts++;
	}
	CagdCrvFree(CrvW);
    }
    else {
        if (ConstVal != 0.0) {
	    int i;
	    CagdRType
	        *SPts = Crv -> Points[1];

	      for (i = 0; i < Crv -> Length; i++)
		  *SPts++ -= ConstVal;
	}
    }

    if (CrvX)
	CagdCrvFree(CrvX);
    if (CrvY)
	CagdCrvFree(CrvY);
    if (CrvZ)
	CagdCrvFree(CrvZ);

    GlblPtList = NULL;
    if (CAGD_IS_BEZIER_CRV(Crv)) {
	/* We need to save domains. */
	TCrv = CagdCnvrtBzr2BspCrv(Crv);
	CagdCrvFree(Crv);
	Crv = TCrv;
    }

    CagdCrvDomain(Crv, &CrvTMin, &CrvTMax);

    SymbScalarCrvZeroSet(Crv);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugSymbZeroPrintSol, FALSE) {
	    CagdPtStruct *Pt = GlblPtList;

	    for (Pt = GlblPtList; Pt != NULL; Pt = Pt -> Pnext)
	        IRIT_INFO_MSG_PRINTF("Pt = %f\n", Pt -> Pt[0]);
	}
    }
#endif /* DEBUG */

#ifdef DEBUG_TEST_COMPARE_ZERO_SET_SOLS
    {
	CagdPtStruct *Pt1, *Pt2,
	    *FirstPtList = GlblPtList;

	GlblPtList = NULL;

	SymbScalarCrvZeroSetIN(Crv);
	for (Pt1 = FirstPtList, Pt2 = GlblPtList;
	     Pt1 != NULL && Pt2 != NULL;
	     Pt1 = Pt1 -> Pnext, Pt2 = Pt2 -> Pnext) {
	    if (!IRIT_APX_EQ(Pt1 -> Pt[0], Pt2 -> Pt[0])) {
	        IRIT_INFO_MSG_PRINTF("DIFFER:\n");
		CagdDbg(Crv);
		for (Pt1 = FirstPtList, Pt2 = GlblPtList;
		     Pt1 != NULL && Pt1 != NULL;
		     Pt1 = Pt1 -> Pnext, Pt2 = Pt2 -> Pnext) {
		    IRIT_INFO_MSG_PRINTF("Sols Pt1 = %f  Pt2 = %f\n",
					 Pt1 -> Pt[0], Pt2 -> Pt[0]);
		}
		abort();
	    }
	}
	CagdPtFreeList(FirstPtList);
    }
#endif /* DEBUG_TEST_COMPARE_ZERO_SET_SOLS */

    CagdCrvFree(Crv);

    return GlblPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the zeros of low degree polynomial, analytically.               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   Low degree polynomial to derive its roots analytically.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SymbScalarCrvLowDegZeroSet(CagdCrvStruct *Crv)
{
    int j = 0;
    CagdRType TMin, TMax;
    CagdCrvStruct
        *BzrCrv = CAGD_IS_BEZIER_CRV(Crv) ? Crv
					  : CagdCnvrtBsp2BzrCrv(Crv),
        *PwrCrv = CagdCnvrtBzr2PwrCrv(BzrCrv);
    CagdRType Sols[4],
        *Pts = PwrCrv -> Points[1];

    CagdCrvDomain(Crv, &TMin, &TMax);

    switch (Crv -> Order) {
        case 5:
	    if (IRIT_FABS(Pts[4]) > IRIT_EPS) {
	        j = GMSolveQuarticEqn(Pts[3] / Pts[4],
				      Pts[2] / Pts[4],
				      Pts[1] / Pts[4],
				      Pts[0] / Pts[4],
				      Sols);
		break;
	    }
        case 4:
	    if (IRIT_FABS(Pts[3]) > IRIT_EPS) {
	        j = GMSolveCubicEqn(Pts[2] / Pts[3],
				    Pts[1] / Pts[3],
				    Pts[0] / Pts[3],
				    Sols);
		break;
	    }
        case 3:
	    if (IRIT_FABS(Pts[2]) > IRIT_EPS) {
	        j = GMSolveQuadraticEqn(Pts[1] / Pts[2],
					Pts[0] / Pts[2],
					Sols);
		break;
	    }
        case 2:
	    if (IRIT_FABS(Pts[1]) > IRIT_EPS) {
	        j = 1;
		Sols[0] = -Pts[0] / Pts[1];
	    }
	    break;
    }
    CagdCrvFree(PwrCrv);

    while (--j >= 0) {
        if (Sols[j] > -GlblSetEpsilon && Sols[j] <= 1.0 + GlblSetEpsilon) {
	    Sols[j] = TMin + (TMax - TMin) * Sols[j];
	    if (CagdCrvEval(Crv, Sols[j])[1] > GlblSetEpsilon)
	        Sols[j] = SymbScalarCrvNewtonRaphson(Crv, Sols[j],
				IRIT_MAX(Sols[j] - SET_DEFAULT_IRIT_EPS, TMin),
			 	IRIT_MIN(Sols[j] + SET_DEFAULT_IRIT_EPS, TMax));
	    SymbInsertNewParam(Sols[j]);
	}
    }

    if (BzrCrv != Crv)
        CagdCrvFree(BzrCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the zero set of a scalar curve. Curve must be a (piecewise)     *
* polynomial represented as a Bspline.					     *
*   Assumes open end condition on the curve parametric domain.		     *
*   Zero set solution points are append to the GlblPtList point list.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To compute its zero set. 	                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SymbScalarCrvZeroSet(CagdCrvStruct *Crv)
{
    int i,
	Len = Crv -> Length,
	Pos = FALSE,
	Neg = FALSE;
    CagdRType *R,
	*ScalarPts = Crv -> Points[1];

    for (i = Len, R = ScalarPts; --i >= 0; ) {
        Pos |= (*R >= 0.0);
	if ((Neg |= (*R++ <= 0.0)) && Pos)
	    break;
    }

    if (Pos && Neg) {
	CagdRType TMin, TMax, TMid;

	/* Control polygon is both positive and negative. */
	CagdCrvDomain(Crv, &TMin, &TMax);
	if (Crv -> Length > Crv -> Order)
	    TMid = Crv -> KnotVector[(Crv -> Length + Crv -> Order) >> 1];
	else
	    TMid = TMin * SYMB_ZERO_MID_RATIO +
		   TMax * (1.0 - SYMB_ZERO_MID_RATIO);

	if (TMax - TMin < GlblSetEpsilon * 0.2) {    /* Small enough - stop. */
	    SymbInsertNewParam((TMax + TMin) * 0.5);
	}
	else if (Crv -> Length == Crv -> Order && Crv -> Order < 6) {
	    /* A low degree polynomial Bezier curve - solve analytically. */
	    SymbScalarCrvLowDegZeroSet(Crv);
	}
	else {
	    int Sign;
	    CagdRType PrevV;
	    CagdCrvStruct *Crv1, *Crv2;

	    /* Check if control polygon is monotone - if so one solution. */
	    ScalarPts = Crv -> Points[1];
	    PrevV = ScalarPts[0];

	    for (i = Sign = 0; i < Len; i++) {
	        CagdRType
		    V = ScalarPts[i],
		    DV = V - PrevV;
		int NewSign = IRIT_SIGN(DV);

		if (Sign * NewSign < 0)
		    break;
		else if (NewSign)
		    Sign = NewSign;

		PrevV = V;
	    }

	    if (i == Len) {
	        /* Try to find this single solution using numeric marching. */
	        CagdRType
		    t = SymbScalarCrvNewtonRaphson(Crv, TMid, TMin, TMax);

		if (t >= TMin && t < TMax)
		    SymbInsertNewParam(t);
		else
		    i = 0;
	    }

	    /* Control poly is both increasing and decreasing and/or numeric */
	    /* marching has failed - subdivide.			 	     */
	    if (i < Len) {
	        CagdRType t1a, t1b, t2a, t2b;

	        Crv1 = CagdCrvSubdivAtParam(Crv, TMid);
		Crv2 = Crv1 -> Pnext;

		CagdCrvDomain(Crv1, &t1a, &t1b);
		CagdCrvDomain(Crv2, &t2a, &t2b);
		if (t1a == t1b || t2a == t2b) {
		    /* Subdivision fails - return the middle of this domain */
		    /* as as zero.					    */
		    SymbInsertNewParam((TMax + TMin) * 0.5);
		}
		else {
		    SymbScalarCrvZeroSet(Crv1);
		    SymbScalarCrvZeroSet(Crv2);
		}

		CagdCrvFree(Crv1);
		CagdCrvFree(Crv2);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Assuming Crv = 0 has a single solution in t in [TMin, TMax], seek it     *
* using Newton Raphson numeric iterations.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         Curve to seek its intersectiion with Y = ConstVal.          *
*   t:           Starting parameter.					     *
*   TMin, TMax:  Domain of Crv.					 	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Solution if found, or t outside [TMin, TMax] if failed.     *
*****************************************************************************/
static CagdRType SymbScalarCrvNewtonRaphson(CagdCrvStruct *Crv,
					    CagdRType t,
					    CagdRType TMin,
					    CagdRType TMax)
{
    int i;
    CagdCrvStruct
	*DCrv = CagdCrvDeriveScalar(Crv);

    for (i = 0; i < SYMB_ZERO_NR_MAX_ITER; i++) {
        CagdRType *R, v, Dv;

#	ifdef SYMBZERO_SUPPORT_NR_RATIONAL
        if (IsRational) {
	    CagdRType Pos[2];

	    R = CagdCrvEval(Crv, t);
	    v = R[1] / R[0];
	    IRIT_PT2D_COPY(Pos, R);

	    R = CagdCrvEval(DCrv, t);
	    Dv = (R[1] * Pos[0] - Pos[1] * R[0]) / IRIT_SQR(Pos[0]);
	}
	else
#	endif /* SYMBZERO_SUPPORT_NR_RATIONAL */
	{
	    R = CagdCrvEval(Crv, t);
	    v = R[1];

	    R = CagdCrvEval(DCrv, t);
	    Dv = R[1];
	}

	/* Are we accurate enough!? */
	if (IRIT_FABS(v) < IRIT_SQR(GlblSetEpsilon) ||
	    IRIT_FABS(v / Dv) < GlblSetEpsilon)
	    break;

	if (IRIT_FABS(Dv) < SYMB_DOUBLE_ROOT_SLOPE)
	    t = t - 2 * v / Dv;
	else
	    t = t - v / Dv;
	t = IRIT_BOUND(t, TMin, TMax);
    }

    CagdCrvFree(DCrv);

    if (i >= SYMB_ZERO_NR_MAX_ITER)
        return TMin - 1.0; /* Failed. */
    else
        return t;
}

#ifdef DEBUG_TEST_COMPARE_ZERO_SET_SOLS

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the zero set of a scalar curve, using Interval Newton method.   *
* Curve must be a scalar (piecewise) polynomial.			     *
*   Assumes open end condition on the curve parametric domain.		     *
*   Constant set points are append to the GlblPtList point list.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      A scalar piecewise polynomial to compute its zero set.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SymbScalarCrvZeroSetIN(CagdCrvStruct *Crv)
{
    SymbScalarCrvZeroSetINAux(Crv, Crv);
}

#endif /* DEBUG_TEST_COMPARE_ZERO_SET_SOLS */

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of SymbScalarCrvZeroSetIN.	  Note DCrv could be NULL in *
* which case the derivative function is the second coordinate in Crv.	     *
*****************************************************************************/
static void SymbScalarCrvZeroSetINAux(CagdCrvStruct *Crv,
				      CagdCrvStruct *OrigCrv)
{
    int i,
	Pos = FALSE,
	Neg = FALSE,
	Order = Crv -> Order,
	Len = Crv -> Length;
    CagdRType Sign, Pt1, Pt2, *R, TMin, TMax, TNewMin, TNewMax, t, Pt,
	MinSlope, MaxSlope,
	*ScalarPts = Crv -> Points[1];

    /* Examine if all coefficients are positive or all negative. */
    for (i = Len, R = ScalarPts; --i >= 0; ) {
        Pos |= (*R >= 0.0);
	if ((Neg |= (*R++ <= 0.0)) && Pos)
	    break;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Handle end point solutions - insert them explicitely. */
    if (IRIT_APX_EQ_EPS(ScalarPts[0], 0.0, GlblSetEpsilon)) {
	CagdCrvDomain(OrigCrv, &TNewMin, &TNewMax);
	t = SymbScalarCrvNewtonRaphson(OrigCrv, TMin, TNewMin, TNewMax);
	if (t >= TNewMin && t < TNewMax)
	    SymbInsertNewParam(t);
    }
    else if (IRIT_APX_EQ_EPS(ScalarPts[Len - 1], 0.0, GlblSetEpsilon)) {
	CagdCrvDomain(OrigCrv, &TNewMin, &TNewMax);
        t = SymbScalarCrvNewtonRaphson(OrigCrv, TMax, TNewMin, TNewMax);
	if (t >= TNewMin && t < TNewMax)
	    SymbInsertNewParam(t);
    }


    if (Pos && Neg) {
	if (TMax - TMin < GlblSetEpsilon * 0.2) {    /* Small enough - stop. */
	    SymbInsertNewParam((TMax + TMin) * 0.5);
	    return;
	}

        if (Len == Crv -> Order && Crv -> Order < 6) {
	    /* A low degree polynomial Bezier curve - solve analytically. */
	    SymbScalarCrvLowDegZeroSet(Crv);
	    return;
	}

	/* Check if control polygon is monotone - if so one solution. */
	ScalarPts = Crv -> Points[1];
	for (i = 1, Sign = 0.0; i < Len; i++) {
	    CagdRType
	        NewSign = ScalarPts[i] - ScalarPts[i - 1];

	    if (Sign * NewSign < 0)
	        break;
	    else if (NewSign)
	        Sign = NewSign;
	}

	if (i >= Len) {
	    /* Monotone: Try to find the single solution using NR. */
	    t = SymbScalarCrvNewtonRaphson(Crv, (TMin + TMax) * 0.5,
					   TMin, TMax);

	    if (t >= TMin && t < TMax) {
	        SymbInsertNewParam(t);
		return;
	    }

	    Pt1 = ScalarPts[0];
	    Pt2 = ScalarPts[Crv -> Length - 1];
	    if (Pt1 == 0)
	        t = TMin;
	    else if (Pt2 == 0)
	        t = TMax;
	    else
	        t = TMin + (TMax - TMin) * Pt1 / (Pt1 - Pt2);
	}
	else
	    t = TMin * (1.0 - SYMB_ZERO_MID_RATIO) +
	        TMax * SYMB_ZERO_MID_RATIO;

	Pt = CagdCrvEval(Crv, t)[1];
	CagdCrvScalarCrvSlopeBounds(Crv, &MinSlope, &MaxSlope);

	/* Handle the singular cases of solution on the boundary. */
	if (MinSlope == 0 && MaxSlope == 0) {
	    if (Pos != 0)
	        return; /* No solution - horizontal curve not at level zero. */
	    else {
	        /* Infinitely many solutions - identically zero curve. */
	        SYMB_FATAL_ERROR(SYMB_ERR_IDENTICAL_ZERO_DATA);
		return;
	    }
	}

	if (MinSlope == 0)
	    MinSlope = MaxSlope * -IRIT_UEPS;
	else if (MaxSlope == 0)
	    MaxSlope = MinSlope * -IRIT_UEPS;

	/* Evaluate the new interval's parameter values. */
	TNewMin = t - Pt / MinSlope;
	TNewMax = t - Pt / MaxSlope;
	if (TNewMin > TNewMax)
	    IRIT_SWAP(CagdRType, TNewMin, TNewMax);

	if (MinSlope * MaxSlope > 0) {
	    /* We have one interval only. */
	    if (Order == 2) {
	        /* In linear curves, solutions might end up are on the      */
	        /* boundaries - move the domain a bit to ensure we capture  */
	        /* all the solutions.					    */
	        TMin = IRIT_MAX(TMin, TNewMin - IRIT_EPS);
		TMax = IRIT_MIN(TMax, TNewMax + IRIT_EPS);
	    }
	    else {
	        TMin = IRIT_MAX(TMin, TNewMin);
		TMax = IRIT_MIN(TMax, TNewMax);
	    }

	    if (TMax >= TMin)
	        SymbScalarCrvZeroSetINRecurseOnInterval(Crv, OrigCrv,
							TMin, TMax);
	}
	else {
	    /* We have two intervals.  Start with { -Infinity, TNewMin }.  */
	    if (Order == 2) {
	        t = IRIT_MIN(TMax, TNewMin + IRIT_EPS);
	    }
	    else {
	        t = IRIT_MIN(TMax, TNewMin);
	    }

	    if (t >= TMin)
	        SymbScalarCrvZeroSetINRecurseOnInterval(Crv, OrigCrv,
							TMin, t);

	    /* The second interval, { TNewMax, +Infinity }.  */
	    if (Order == 2) {
	        t = IRIT_MAX(TMin, TNewMax - IRIT_EPS);
	    }
	    else {
	        t = IRIT_MAX(TMin, TNewMax);
	    }

	    if (TMax >= t)
	        SymbScalarCrvZeroSetINRecurseOnInterval(Crv, OrigCrv,
							t, TMax);
	}
    }
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of SymbScalarCrvZeroSetIN.				     *
*****************************************************************************/
static void SymbScalarCrvZeroSetINRecurseOnInterval(CagdCrvStruct *Crv,
						    CagdCrvStruct *OrigCrv,
						    CagdRType TMin,
						    CagdRType TMax)
{
    CagdRType t1, t2, t;
    CagdCrvStruct *Crvs, *TCrv;

    if (IRIT_APX_EQ_EPS(TMin, TMax, CAGD_EPS_ROUND_KNOT * 2)) {
        /* Domain too small numerically to continue and divide. */
        SymbInsertNewParam((TMin + TMax) * 0.5);
	return;
    }
    else {
        CagdCrvDomain(Crv, &t1, &t2);
        if (IRIT_APX_EQ_EPS(TMin, t1, CAGD_EPS_ROUND_KNOT) &&
	    IRIT_APX_EQ_EPS(TMax, t2, CAGD_EPS_ROUND_KNOT)) {
	    /* New domain too similar to input domain - divide in middle. */
	    if (Crv -> Length > Crv -> Order)
	        t = Crv -> KnotVector[(Crv -> Length + Crv -> Order) >> 1];
	    else
	        t = SYMB_ZERO_MID_RATIO * t1 + (1.0 - SYMB_ZERO_MID_RATIO) * t2;
	    Crvs = CagdCrvSubdivAtParam(Crv, t);

	    /* Recurse on the two halves. */
	    SymbScalarCrvZeroSetINAux(Crvs, OrigCrv);
	    SymbScalarCrvZeroSetINAux(Crvs -> Pnext, OrigCrv);
	    CagdCrvFreeList(Crvs);
	}
	else {
	    /* Recurse on the new interval. */
	    TCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);
	    SymbScalarCrvZeroSetINAux(TCrv, OrigCrv);
	    CagdCrvFree(TCrv);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the extrem set of a scalar curve. Curve might be rational.	     *
*   Assumes open end condition on the curve parametric domain.		     *
*   Extreme set points are append to the GlblPtList point list.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To compute its extremum set.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SymbScalarCrvExtremSet(CagdCrvStruct *Crv)
{
    int i,
	Len = Crv -> Length,
        Sign = 0;
    CagdRType
	*ScalarPts = Crv -> Points[1],
	*ScalarWPts = Crv -> Points[0];

    if (SymbCrvPosNegWeights(Crv)) {
	i = 0; 				 /* Force subdivision of the curve. */
    }
    else {
	CagdRType
	    PrevV = ScalarWPts ? ScalarPts[0] / ScalarWPts[0] : ScalarPts[0];

	for (i = 0; i < Len; i++) {
	    CagdRType
		V = (ScalarWPts ? ScalarPts[i] / ScalarWPts[i] : ScalarPts[i]),
		DV = V - PrevV;
	    int NewSign = IRIT_SIGN(DV);

	    if (Sign * NewSign < 0)
		break;
	    else if (NewSign)
		Sign = NewSign;

	    PrevV = V;
	}
    }

    if (i < Len) {
	CagdRType TMin, TMax, TMid;

	/* Control polygon is both increasing and decreasing. */
	CagdCrvDomain(Crv, &TMin, &TMax);
	TMid = TMin * SYMB_ZERO_MID_RATIO + TMax * (1.0 - SYMB_ZERO_MID_RATIO);

	if (TMax - TMin < GlblSetEpsilon * 0.2) {    /* Small enough - stop. */
	    SymbInsertNewParam((TMax + TMin) * 0.5);
	}
	else { 						 /* Needs to subdiv. */
	    CagdCrvStruct
	        *Crv1 = CagdCrvSubdivAtParam(Crv, TMid),
		*Crv2 = Crv1 -> Pnext;

	    SymbScalarCrvExtremSet(Crv1);
	    SymbScalarCrvExtremSet(Crv2);

	    CagdCrvFree(Crv1);
	    CagdCrvFree(Crv2);
	}
    }
    else {
	CagdRType V1, V2, TMin, TMax;

	CagdCrvDomain(Crv, &TMin, &TMax);

	/* This segment is monotone. Test if end points are extremes. */
	V1 = ScalarWPts ? ScalarPts[0] / ScalarWPts[0] : ScalarPts[0];
	V2 = ScalarWPts ? ScalarPts[1] / ScalarWPts[1] : ScalarPts[1];
	if (IRIT_APX_EQ(V1, V2))
	    SymbInsertNewParam(TMin);

	V1 = ScalarWPts ? ScalarPts[Len - 2] / ScalarWPts[Len - 2]
			: ScalarPts[Len - 2];
	V2 = ScalarWPts ? ScalarPts[Len - 1] / ScalarWPts[Len - 1]
			: ScalarPts[Len - 1];
	if (IRIT_APX_EQ(V1, V2))
	    SymbInsertNewParam(TMax);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the Crv is not rational or rational with weights that are M
* entirely positive or entirely negative.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To examine for same sign weights, if any.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if no weights or all of same sign.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvPosNegWeights, symbolic computation                               M
*****************************************************************************/
CagdBType SymbCrvPosNegWeights(const CagdCrvStruct *Crv)
{
    int i;
    CagdBType HasNeg, HasPos;
    CagdRType
	*Weights = Crv -> Points[0];

    if (Weights == NULL)
	return FALSE;				   /* Curve is not rational. */

    for (HasNeg = HasPos = FALSE, i = Crv -> Length - 1; i >= 0; i--) {
	HasNeg |= *Weights < 0.0;
	HasPos |= *Weights++ > 0.0;
    }

    return HasNeg && HasPos;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Splits the given rational curves at all their poles.  Returned is a      M
* list of curves each of which has weights of the same (positive) sign.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:      Rational curves to split at all its poles.		     M
*   Eps:       Tolerance of computation.                                     M
*   OutReach:  Clip end points of curves that goes to infinity at distance   M
*	       that is about OutReach from the origin.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of splitted curves.			             M
*                                                                            *
* SEE ALSO:                                                                  M
*    CagdPointsHasPoles, SymbCrvSplitPoleParams                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvsSplitPoleParams                                                  M
*****************************************************************************/
CagdCrvStruct *SymbCrvsSplitPoleParams(const CagdCrvStruct *Crvs,
				       CagdRType Eps,
				       CagdRType OutReach)
{
    const CagdCrvStruct *Crv;
    CagdCrvStruct
	*NoPolesCrvs = NULL;

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdCrvStruct
	    *TCrvs = SymbCrvSplitPoleParams(Crv, Eps, OutReach);

	NoPolesCrvs = CagdListAppend(TCrvs, NoPolesCrvs);
    }

    return NoPolesCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Splits the given rational curve at all its poles.  Returned is a list of M
* curves each of which has weights of the same (positive) sign.              M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      Rational curve to split at all its poles.		     M
*   Eps:       Tolerance of computation.                                     M
*   OutReach:  Clip end points of curves that goes to infinity at distance   M
*	       that is about OutReach from the origin.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of splitted curves.			             M
*                                                                            *
* SEE ALSO:                                                                  M
*    CagdPointsHasPoles, SymbCrvsSplitPoleParams                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSplitPoleParams                                                   M
*****************************************************************************/
CagdCrvStruct *SymbCrvSplitPoleParams(const CagdCrvStruct *CCrv,
				      CagdRType Eps,
				      CagdRType OutReach)
{
    int i, k, Proximity,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType);
    CagdRType TMin, TMax, *R, CMin, CMax, **Points, *WPts;
    CagdPtStruct *Pts;
    CagdCrvStruct *Crvs, *TCrv,
	*OutCrvs = NULL,
	*Crv = CagdCrvCopy(CCrv);

    if (!CAGD_IS_RATIONAL_CRV(Crv))
	return Crv;

    Points = Crv -> Points;
    WPts = Points[0];
    for (i = 0; i < Crv -> Length; i++) {
	if (IRIT_APX_EQ_EPS(WPts[i], 0.0, Eps)) {
	    for (k = 0; k <= MaxCoord; k++)
		Points[k][i] = 0.0;
	}
    }

    /* If the curve's end points have poles - clip the curve before we       */
    /* attempt to do anything.						     */
    CagdCrvDomain(Crv, &TMin, &TMax);
    R = CagdCrvEval(Crv, TMin);
    CMin = IRIT_APX_EQ_EPS(R[0], 0.0, Eps) ? TMin + Eps : TMin;
    R = CagdCrvEval(Crv, TMax);
    CMax = IRIT_APX_EQ_EPS(R[0], 0.0, Eps) ? TMax - Eps : TMax;
    if (CMin != TMin || CMax != TMax) {
        TCrv = CagdCrvRegionFromCrv(Crv, CMin, CMax);
	CagdCrvFree(Crv);
	Crv = TCrv;
    }

    /* Possible to have poles - do we have both positive & negative weights? */
    if (!CagdPointsHasPoles(Crv -> Points, Crv -> Length) ||
	(Pts = SymbCrvZeroSet(Crv, 0, Eps, TRUE)) == NULL)
        return Crv;

    Crvs = CagdCrvSubdivAtParams(Crv, Pts, Eps, &Proximity);
    CagdPtFreeList(Pts);
    CagdCrvFree(Crv);

    while (Crvs != NULL) {
        int i;
        CagdRType t, dt, X, *WPts;

	IRIT_LIST_POP(Crv, Crvs);
	WPts = Crv -> Points[0];

        /* Make sure all the weights are positive. */
        for (i = 0, t = 0.0; i < Crv -> Length; i++)
	    if (IRIT_FABS(t) < IRIT_FABS(WPts[i]))
	        t = WPts[i];

	if (t < 0.0) {
	    int j,
		MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
	    CagdRType
		**Points = Crv -> Points;

	    /* We are flipping all signs of all coefficients... */
	    for (i = 0; i < Crv -> Length; i++) {
		for (j = 1; j <= MaxAxis; j++)
		    Points[j][i] = -Points[j][i];
	    }
	}

	for (i = 0; i < Crv -> Length; i++)
	    WPts[i] = IRIT_FABS(WPts[i]);

	CagdCrvDomain(Crv, &TMin, &TMax);

	/* Clip starting location, if necessary. */
	t = TMin;
	dt = Eps;
	do {
	    R = CagdCrvEval(Crv, t);
	    X = R[0] == 0 ? IRIT_INFNTY : IRIT_MAX(IRIT_FABS(R[2] / R[0]), 
					           IRIT_FABS(R[1] / R[0]));
	    t += dt;
	    dt *= 3;
	}
	while (X > OutReach);
	TMin = IRIT_MAX(TMin, t);

	/* Clip end location, if necessary. */
	t = TMax;
	dt = Eps;
	do {
	    R = CagdCrvEval(Crv, t);
	    X = R[0] == 0 ? IRIT_INFNTY : IRIT_MAX(IRIT_FABS(R[2] / R[0]), 
					           IRIT_FABS(R[1] / R[0]));
	    t -= dt;
	    dt *= 3;
	}
	while (X > OutReach);
	TMax = IRIT_MIN(TMax, t);

	if (TMin > TMax) {
	    CagdCrvFree(Crv);
	}
	else {
	    TCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);
	    CagdCrvFree(Crv);
	    IRIT_LIST_PUSH(TCrv, OutCrvs);
	}
    }

    return OutCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Clear the global sorted list of points and return what was on that list    M
* before.  This sorted list is built via SymbInsertNewParam		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TMin, TMax:	  Domain to insert points in between.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:	The old point list.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbInsertNewParam                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbGetParamListAndReset		                                     M
*****************************************************************************/
CagdPtStruct *SymbGetParamListAndReset(CagdRType TMin, CagdRType TMax)
{
    CagdPtStruct
	*PtList = GlblPtList;

    CrvTMin = TMin;
    CrvTMax = TMax;
    GlblPtList = NULL;

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert a single t value into existing GlblPtList, provided no equal t      M
* value exists already in the list. List is ordered incrementally.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   t:         New value to insert to global GlblPtList list.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbGetParamListAndReset, SymbInsertNewParam2                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbInsertNewParam    		                                     M
*****************************************************************************/
void SymbInsertNewParam(CagdRType t)
{
    if (t > CrvTMax)
	t = CrvTMax;
    if (t < CrvTMin)
	t = CrvTMin;

    if (GlblNoSolsOnEndPts &&
	(IRIT_APX_EQ_EPS(t, CrvTMin, GlblSetEpsilon) ||
	 IRIT_APX_EQ_EPS(t, CrvTMax, GlblSetEpsilon)))
	return;

    GlblPtList = SymbInsertNewParam2(GlblPtList, t);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert a single t value into given PtList, in place, provided no equal t   M
* value exists already in the list. List is ordered incrementally.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:    List to insert a new value t into.                            M
*   t:         New value to insert to PtList list.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  Updated list, in place.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbGetParamListAndReset, SymbInsertNewParam                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbInsertNewParam2    		                                     M
*****************************************************************************/
CagdPtStruct *SymbInsertNewParam2(CagdPtStruct *PtList, CagdRType t)
{
    CagdPtStruct *PtTmp, *PtLast, *Pt;

    Pt = CagdPtNew();
    Pt -> Pt[0] = t;

    if (PtList) {
	for (PtTmp = PtList, PtLast = NULL;
	     PtTmp != NULL;
	     PtLast = PtTmp, PtTmp = PtTmp -> Pnext) {
	    if (ZERO_IRIT_APX_EQ(PtTmp -> Pt[0], t)) {
	        IritFree(Pt);
		return PtList;
	    }
	    if (PtTmp -> Pt[0] > t)
	        break;
	}
	if (PtTmp) {
	    /* Insert the new point in the middle of the list. */
	    Pt -> Pnext = PtTmp;
	    if (PtLast)
		PtLast -> Pnext = Pt;
	    else
		PtList = Pt;
	}
	else {
	    /* Insert the new point as the last point in the list. */
	    PtLast -> Pnext = Pt;
	}
    }
    else
        PtList = Pt;

    return PtList;
}
