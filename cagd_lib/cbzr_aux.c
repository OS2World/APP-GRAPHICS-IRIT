/******************************************************************************
* CBzr-Aux.c - Bezier curve auxilary routines.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

static CagdVecStruct *BzrCrvTangentAux(const CagdCrvStruct *Crv,
				       CagdRType t,
				       CagdBType Normalize);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply Bezier subdivision to the given curve at parameter value t, and    M
* save the result in data LPoints/RPoints.  Note this function could also be M
* called from a B-spline curve with a Bezier knot sequence.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:            To subdivide at parametr value t.                     M
*   LPoints, RPoints:  Where the results are kept.			     M
*   Length:	       Of this Bezier curve.				     M
*   PType:	       Points types we have here.			     M
*   t:                 Parameter value to subdivide curve at.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvSubdivAtParam, BspCrvSubdivCtlPoly, BzrCrvSubdivCtlPolyStep        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvSubdivCtlPoly                                                      M
*****************************************************************************/
void BzrCrvSubdivCtlPoly(CagdRType * const *Points,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int Length,
			 CagdPointType PType,
			 CagdRType t)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_PT(PType);
    int i, j, l,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType
	t1 = 1.0 - t;

    /* Copy Points into RPoints, so we can apply the recursive algo. to it.  */
    for (j = IsNotRational; j <= MaxCoord; j++)
        IRIT_GEN_COPY(RPoints[j], Points[j], Length * sizeof(CagdRType));

    for (j = IsNotRational; j <= MaxCoord; j++)
	LPoints[j][0] = Points[j][0];

    /* Apply the recursive algorithm to RPoints, and update LPoints with the */
    /* temporary results. Note we updated the first point of LPoints above.  */
    for (i = 1; i < Length; i++) {
	for (l = 0; l < Length - i; l++)
	    for (j = IsNotRational; j <= MaxCoord; j++)
		RPoints[j][l] = RPoints[j][l] * t1 + RPoints[j][l + 1] * t;

	/* Copy temporary result to LPoints: */
	for (j = IsNotRational; j <= MaxCoord; j++)
	    LPoints[j][i] = RPoints[j][0];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply Bezier subdivision to the given data at parameter value t, and     M
* save the result in data LPoints/RPoints.  Note this function could also be M
* called from a B-spline curve with a Bezier knot sequence.		     M
*   This function is used to Bezier subdivide surfaces (See Step size!).     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:            To subdivide at parametr value t.                     M
*   LPoints, RPoints:  Where the results are kept.			     M
*   Length:	       Of this Bezier curve.				     M
*   PType:	       Points types we have here.			     M
*   t:                 Parameter value to subdivide data at.                 M
*   Step:	       Stride along the data, 1 for curves, ULength for a    M
*		       surface subdivision along V.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvSubdivAtParam, BspCrvSubdivCtlPoly, BzrCrvSubdivCtlPoly            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvSubdivCtlPolyStep                                                  M
*****************************************************************************/
void BzrCrvSubdivCtlPolyStep(CagdRType * const *Points,
			     CagdRType **LPoints,
			     CagdRType **RPoints,
			     int Length,
			     CagdPointType PType,
			     CagdRType t,
			     int Step)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_PT(PType);
    int i, j, l,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType
	t1 = 1.0 - t;

    /* Copy Points into RPoints, so we can apply the recursive algo. to it.  */
    for (j = IsNotRational; j <= MaxCoord; j++) {
        CagdRType const
	    *Pts = Points[j];
        CagdRType
            *RPts = RPoints[j];

        for (i = 0; i < Length; i++) {
	    RPts[0] = Pts[0];
	    Pts += Step;
	    RPts += Step;
	}

	LPoints[j][0] = Points[j][0];
    }

    /* Apply the recursive algorithm to RPoints, and update LPoints with the */
    /* temporary results. Note we updated the first point of LPoints above.  */
    for (i = 1; i < Length; i++) {
        int s;

        for (l = s = 0; l < Length - i; l++, s += Step) {
	    for (j = IsNotRational; j <= MaxCoord; j++)
		RPoints[j][s] = RPoints[j][s] * t1 + RPoints[j][s + Step] * t;
	}

	/* Copy temporary result to LPoints: */
	for (j = IsNotRational; j <= MaxCoord; j++)
	    LPoints[j][i * Step] = RPoints[j][0];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier curve - subdivides it into two sub-curves at the given      M
* parametric value.                                                          M
*   Returns pointer to first curve in a list of two subdivided curves.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To subdivide at parametr value t.                             M
*   t:         Parameter value to subdivide Crv at.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of the two subdivided curves.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSubdivCtlPoly, BspCrvSubdivAtParam                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvSubdivAtParam, subdivision, refinement                             M
*****************************************************************************/
CagdCrvStruct *BzrCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    int k = Crv -> Length;
    CagdCrvStruct
	*LCrv = BzrCrvNew(k, Crv -> PType),
	*RCrv = BzrCrvNew(k, Crv -> PType);

    BzrCrvSubdivCtlPoly(Crv -> Points, LCrv -> Points, RCrv -> Points,
			k, Crv -> PType, t);

    LCrv -> Pnext = RCrv;

    CAGD_PROPAGATE_ATTR(LCrv, Crv);
    CAGD_PROPAGATE_ATTR(RCrv, Crv);

    return LCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with order NewOrder.    M
*   Degree raise is computed by multiplying by a constant 1 curve of order   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree to a NewOrder.                           M
*   NewOrder:   NewOrder for Crv.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of order NewOrder representing the same        M
*                     geometry as Crv.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDegreeRaise, PwrCrvDegreeRaiseN	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdCrvStruct *BzrCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder)
{
    int i, j, RaisedOrder,
	Order = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RaisedCrv, *UnitCrv;

    if (NewOrder < Order) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedOrder = NewOrder - Order + 1;

    UnitCrv = BzrCrvNew(RaisedOrder, CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
    for (i = 1; i <= MaxCoord; i++)
	for (j = 0; j < RaisedOrder; j++)
	    UnitCrv -> Points[i][j] = 1.0;

    RaisedCrv = BzrCrvMult(Crv, UnitCrv);

    CagdCrvFree(UnitCrv);

    CAGD_PROPAGATE_ATTR(RaisedCrv, Crv);

    return RaisedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with one degree higher. M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.  Then: M
*		       i	    k-i					     V
* Q(0) = P(0), Q(i) = --- P(i-1) + (---) P(i), Q(k) = P(k-1).		     V
*		       k	     k					     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree by one.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of one order higher representing the same      M
*                     geometry as Crv.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDegreeReduce, BzrCrvDegreeRaiseN, PwrCrvDegreeRaise                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdCrvStruct *BzrCrvDegreeRaise(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	k = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
	*RaisedCrv = BzrCrvNew(k + 1, Crv -> PType);

    for (j = IsNotRational; j <= MaxCoord; j++)			    /* Q(0). */
	RaisedCrv -> Points[j][0] = Crv -> Points[j][0];

    for (i = 1; i < k; i++)					    /* Q(i). */
	for (j = IsNotRational; j <= MaxCoord; j++)
	    RaisedCrv -> Points[j][i] =
		Crv -> Points[j][i-1] * (i / ((CagdRType) k)) +
		Crv -> Points[j][i] * ((k - i) / ((CagdRType) k));

    for (j = IsNotRational; j <= MaxCoord; j++)			    /* Q(k). */
	RaisedCrv -> Points[j][k] = Crv -> Points[j][k-1];

    CAGD_PROPAGATE_ATTR(RaisedCrv, Crv);

    return RaisedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, usuall similar to the original but with one degree    M
* smaller.								     M
* Let old control polygon be P(i), i = 0 to n, and Q(i) be new one.  Then:   M
*									     M
*	   n P(i) - i Q_r(i-1)						     V
* Q_r(i) = ------------------- ,   i = 0, 1, ... , n - 1.		     V
*		  n - i							     V
*									     M
*	     n P(i) - (n - i) Q_l(i)					     V
* Q_l(i-1) = ----------------------- ,   i = n, n - 1, ... , 1.		     V
*		        i						     V
*									     M
* and		   							     M
*		       i 						     V
* 		      __						     V
*	         1    \	    2n					             V
*     g(i) = -------  /    (  )					             V
*             (2n-1)  --    2j					             V
*            2        j=0					             V
*	     								     M
* yielding,  								     M
*	     								     M
*     Q(i) = (1 - g(i)) Q_r(i) + g(i) Q_l(i).				     V
*	     								     M
* See also "Curves and Surfaces for Computer Aided Geometric Design"	     M
* Gerald Farin. Academic Press, Inc. Third Edition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To reduce its degree by one.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of one order lower representing a similar      M
*                     geometry to Crv.  The result is optimal in the         M
*		      infinity norm and will be identical to the given       M
*		      curve if the original curve was degree raised.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDegreeRaise                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDegreeReduce, degree reduction, degree raising.                    M
*****************************************************************************/
CagdCrvStruct *BzrCrvDegreeReduce(const CagdCrvStruct *Crv)
{
    int i, l,
	Order = Crv -> Length,
	n = Order - 1,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct
        *NewCrv = BzrCrvNew(n, Crv -> PType);
    CagdRType R,
	*Qr = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	*Ql = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	*g = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	**NewPoints = NewCrv -> Points;
    CagdRType
	* const *Points = Crv -> Points;

    for (l = IsNotRational; l <= MaxCoord; l++) {
	CagdRType const
	    *Pts = Points[l];
	CagdRType
	    *NewPts = NewPoints[l];

	/* Compute the Q_r coefficients. */
	Qr[0] = Pts[0];
	for (i = 1; i < n; i++)
	    Qr[i] = (n * Pts[i] - i * Qr[i - 1]) / (n - i);

	/* Compute the Q_l coefficients. */
	Ql[n - 1] = Pts[n];
	for (i = n - 1; i > 0; i--)
	    Ql[i - 1] = (n * Pts[i] - (n - i) * Ql[i]) / i;

	/* Compute the g(i) coefficients. */
	g[0] = CagdIChooseK(0, 2 * n);
	for (i = 1; i < n; i++)
	    g[i] = g[i - 1] + CagdIChooseK(2 * i, 2 * n);
	R = pow(2, 2 * n - 1);
	for (i = 0; i < n; i++)
	    g[i] /= R;

	/* Compute the final coefficient vector. */
	for (i = 0; i < n; i++)
	    NewPts[i] = (1.0 - g[i]) * Qr[i] + g[i] * Ql[i];
    }

    IritFree(Qr);
    IritFree(Ql);
    IritFree(g);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the tangent to Crv at parameter value t. M
*   Algorithm: pseudo subdivide Crv at t and using control point of          M
* subdivided curve find the tangent as the difference of the 2 end points.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) tangent.                    M
*   t:         The parameter at which to compute the unit tangent.           M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, returned is an unnormalized vector in the right     M
*	       direction of the tangent.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the tangent       M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvTangent, tangent                                                   M
*****************************************************************************/
CagdVecStruct *BzrCrvTangent(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize)
{
    CagdVecStruct
        *V = BzrCrvTangentAux(Crv, t, Normalize);

    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_SQR(IRIT_UEPS) && Crv -> Order > 1) {
        /* Try to move a little. This location has zero speed. However,     */
        /* do it only once since we can be here forever. The "_tan"         */
        /* attribute guarantee we will try to move IRIT_EPS only once.      */
        V = BzrCrvTangentAux(Crv, t < 0.5 ? t + IRIT_EPS : t - IRIT_EPS,
			     Normalize);
    }

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a (unit) vector, equal to the tangent to Crv at parameter value t. *
*   Algorithm: pseudo subdivide Crv at t and using control point of          *
* subdivided curve find the tangent as the difference of the 2 end points.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       Crv for which to compute a (unit) tangent.                    *
*   t:         The parameter at which to compute the unit tangent.           *
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    *
*              If FALSE, returned is an unnormalized vector in the right     *
*	       direction of the tangent.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdVecStruct *:  A pointer to a static vector holding the tangent       *
*                     information.                                           *
*****************************************************************************/
static CagdVecStruct *BzrCrvTangentAux(const CagdCrvStruct *Crv,
				       CagdRType t,
				       CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct Tan;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, l,
	FlipPts = FALSE,
	k = Crv -> Length,
        MaxCoord = IRIT_MIN(CAGD_NUM_OF_PT_COORD(Crv -> PType), 3);
    CagdPType P0, P1;
    CagdRType Pt0[4], Pt1[4],
	Domain = 1.0;

    /* Can not compute for constant curves. */
    if (k <= 1)
	return NULL;

    if (IRIT_APX_EQ(t, 0.0)) {
	/* Use Crv starting tangent direction. */
	if (IsNotRational) {
	    CagdCoerceToE3(&Pt0[1], Crv -> Points, 0, Crv -> PType);
	    CagdCoerceToE3(&Pt1[1], Crv -> Points, 1, Crv -> PType);
	}
	else {
	    CagdCoerceToP3(Pt0, Crv -> Points, 0, Crv -> PType);
	    CagdCoerceToP3(Pt1, Crv -> Points, 1, Crv -> PType);
	}
    }
    else if (IRIT_APX_EQ(t, 1.0)) {
	/* Use Crv ending tangent direction. */
	if (IsNotRational) {
	    CagdCoerceToE3(&Pt0[1], Crv -> Points, k - 2, Crv -> PType);
	    CagdCoerceToE3(&Pt1[1], Crv -> Points, k - 1, Crv -> PType);
	}
	else {
	    CagdCoerceToP3(Pt0, Crv -> Points, k - 2, Crv -> PType);
	    CagdCoerceToP3(Pt1, Crv -> Points, k - 1, Crv -> PType);
	}
	FlipPts = TRUE;                       /* Pt1 is the point on Crv(t). */
    }
    else {
        IRIT_STATIC_DATA int
	    PtsLen = 0;
        IRIT_STATIC_DATA CagdRType
	    *Pts = NULL;
        CagdRType
	    t1 = 1.0 - t;

	if (PtsLen < k) {
	    if (Pts != NULL)
	        IritFree(Pts);
	    PtsLen = k * 2;
	    Pts = (CagdRType *) IritMalloc(sizeof(CagdRType) * PtsLen);
	}

	/* Apply the recursive algorithm to copies of Crv's points. */
	for (j = IsNotRational; j <= MaxCoord; j++) {
	    CAGD_GEN_COPY(Pts, Crv -> Points[j], k * sizeof(CagdRType));

	    for (i = 1; i < k; i++)
	        for (l = 0; l < k - i; l++)
		    Pts[l] = Pts[l] * t1 + Pts[l + 1] * t;

	    Pt0[j] = Pts[0];
	    Pt1[j] = Pts[1];
	}
	for (j = MaxCoord + 1; j <= 3; j++)
	    Pt0[j] = Pt1[j] = 0.0;

	Domain = t1; /* The bezier speed is actually for a subset of domain. */
    }

    IRIT_PT_COPY(P0, &Pt0[1]);
    IRIT_PT_COPY(P1, &Pt1[1]);
    if (IsNotRational) {
	IRIT_PT_SUB(Tan.Vec, P1, P0);
    }
    else if (Pt0[0] == 0.0 || Pt1[0] == 0.0) {
        IRIT_VEC_RESET(Tan.Vec);
    }
    else {
        IRIT_PT_SCALE(P0, 1.0 / Pt0[0]);
        IRIT_PT_SCALE(P1, 1.0 / Pt1[0]);
	IRIT_PT_SUB(Tan.Vec, P1, P0);
    }

    if (CAGD_SQR_LEN_VECTOR(Tan) > IRIT_SQR(IRIT_UEPS)) {
        if (IsNotRational) {
	    if (Normalize)
	        CAGD_NORMALIZE_VECTOR_MSG_ZERO(Tan)/* Normalize the vector. */
	    else
	        IRIT_VEC_SCALE(Tan.Vec, (k - 1) / Domain);
	}
	else {
	    /* Make P1 hold the derivative.  P0 already holds the position. */
	    if (FlipPts) {
	        for (i = 0; i < 4; i++) {
		    Pt0[i] = (Pt1[i] - Pt0[i]) * (k - 1) / Domain;
		    IRIT_SWAP(CagdRType, Pt1[i], Pt0[i]);
		}
	    }
	    else {
	        for (i = 0; i < 4; i++)
		    Pt1[i] = (Pt1[i] - Pt0[i]) * (k - 1) / Domain;
	    }

	    /* And use to quotient rule to get the exact tangent. */
	    for (i = 1; i <= 3; i++)
	        Tan.Vec[i - 1] = (Pt1[i] * Pt0[0] - Pt1[0] * Pt0[i])
							       / IRIT_SQR(Pt0[0]);

	    if (Normalize)
	        CAGD_NORMALIZE_VECTOR_MSG_ZERO(Tan);/* Normalize the vector. */
	}
    }

    return &Tan;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the binormal to Crv at parameter value t.M
*   Algorithm: insert (order - 1) knots and using 3 consecutive control      M
* points at the refined location (p1, p2, p3), compute to binormal to be the M
* cross product of the two vectors (p1 - p2) and (p2 - p3).		     M
*   Since a curve may have not BiNormal at inflection points or if the 3     M
* points are collinear, NULL will be returned at such cases.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) binormal.                   M
*   t:         The parameter at which to compute the unit binormal.          M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the binormal      M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvBiNormal, binormal                                                 M
*****************************************************************************/
CagdVecStruct *BzrCrvBiNormal(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct P3;
    CagdVecStruct P1, P2;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, l,
	k = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);

    /* Can not compute for linear curves. */
    if (k <= 2)
	return NULL;

    /* For planar curves, B is trivially the Z axis. */
    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) == 2) {
	P3.Vec[0] = P3.Vec[1] = 0.0;
	P3.Vec[2] = 1.0;
	return &P3;
    }

    if (IRIT_APX_EQ(t, 0.0)) {
	/* Use Crv starting tangent direction. */
	CagdCoerceToE3(P1.Vec, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE3(P2.Vec, Crv -> Points, 1, Crv -> PType);
	CagdCoerceToE3(P3.Vec, Crv -> Points, 2, Crv -> PType);
    }
    else if (IRIT_APX_EQ(t, 1.0)) {
	/* Use Crv ending tangent direction. */
	CagdCoerceToE3(P1.Vec, Crv -> Points, k - 3, Crv -> PType);
	CagdCoerceToE3(P2.Vec, Crv -> Points, k - 2, Crv -> PType);
	CagdCoerceToE3(P3.Vec, Crv -> Points, k - 1, Crv -> PType);
    }
    else {
        IRIT_STATIC_DATA int
	    PtsLen = 0;
        IRIT_STATIC_DATA CagdRType
	    *Pts = NULL;
        CagdRType *R, Pt0[CAGD_MAX_PT_SIZE], Pt1[CAGD_MAX_PT_SIZE],
	              Pt2[CAGD_MAX_PT_SIZE],
	    t1 = 1.0 - t;

	if (PtsLen < k) {
	    if (Pts != NULL)
	        IritFree(Pts);
	    PtsLen = k * 2;
	    Pts = (CagdRType *) IritMalloc(sizeof(CagdRType) * PtsLen);
	}

	/* Apply the recursive algorithm to copies of Crv's points. */
	for (j = IsNotRational; j <= MaxCoord; j++) {
	    CAGD_GEN_COPY(Pts, Crv -> Points[j], k * sizeof(CagdRType));

	    for (i = 1; i < k; i++)
	        for (l = 0; l < k - i; l++)
		    Pts[l] = Pts[l] * t1 + Pts[l + 1] * t;

	    Pt0[j] = Pts[0];
	    Pt1[j] = Pts[1];
	    Pt2[j] = Pts[2];
	}

	R = Pt0;
	CagdCoerceToE3(P1.Vec, &R, -1, Crv -> PType);
	R = Pt1;
	CagdCoerceToE3(P2.Vec, &R, -1, Crv -> PType);
	R = Pt2;
	CagdCoerceToE3(P3.Vec, &R, -1, Crv -> PType);
    }

    CAGD_SUB_VECTOR(P1, P2);
    CAGD_SUB_VECTOR(P2, P3);

    IRIT_CROSS_PROD(P3.Vec, P1.Vec, P2.Vec);

    if (Normalize) {
	if ((t = CAGD_LEN_VECTOR(P3)) == 0.0)
	    return NULL;
	else
	    CAGD_DIV_VECTOR(P3, t);		    /* Normalize the vector. */
    }

    return &P3;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a (unit) vector, equal to the normal of Crv at parameter value t.  M
*   Algorithm: returns the cross product of the curve tangent and binormal.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Crv for which to compute a (unit) normal.                     M
*   t:         The parameter at which to compute the unit normal.            M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the normal        M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvNormal, normal                                                     M
*****************************************************************************/
CagdVecStruct *BzrCrvNormal(const CagdCrvStruct *Crv,
			    CagdRType t,
			    CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct N;
    CagdVecStruct *T, *B;

    T = BzrCrvTangent(Crv, t, FALSE);
    B = BzrCrvBiNormal(Crv, t, FALSE);

    if (T == NULL || B == NULL)
	return NULL;

    IRIT_CROSS_PROD(N.Vec, B -> Vec, T -> Vec);

    if (Normalize)
	CAGD_NORMALIZE_VECTOR_MSG_ZERO(N);         /* Normalize the vector. */

    return &N;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then:   M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDerive, BspCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational, M
*   BzrCrvDeriveScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDerive, derivatives                                                M
*****************************************************************************/
CagdCrvStruct *BzrCrvDerive(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	k = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *DerivedCrv;

    if (!GlblDeriveScalar && !IsNotRational)
	return BzrCrvDeriveRational(Crv);

    DerivedCrv = BzrCrvNew(IRIT_MAX(1, k - 1), Crv -> PType);

    if (k >= 2) {
	for (i = 0; i < k - 1; i++)
	    for (j = IsNotRational; j <= MaxCoord; j++)
		DerivedCrv -> Points[j][i] =
		    (k - 1) * (Crv -> Points[j][i+1] - Crv -> Points[j][i]);
    }
    else {
	for (j = IsNotRational; j <= MaxCoord; j++)
	    DerivedCrv -> Points[j][0] = 0.0;
    }

    return DerivedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*   For a Euclidean curve this is the same as CagdCrvDerive but for a        M
* rational curve the returned curve is not the vector field but simply the   M
* derivatives of all the curve's coefficients, including the weights.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, CagdCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational  M
*   BspCrvDerive, BzrCrvDeriveScalar, CagdCrvDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDeriveScalar, derivatives                                          M
*****************************************************************************/
CagdCrvStruct *BzrCrvDeriveScalar(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *TCrv;

    GlblDeriveScalar = TRUE;

    TCrv = BzrCrvDerive(Crv);

    GlblDeriveScalar = FALSE;

    return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier curve, equal to the integral of the given Bezier      M
* crv.		                                                             M
* The given Bezier curve should be nonrational.				     M
*									     V
* 	      n		   n		   n	   n+1			     V
*   /	     /-		   -	  /	   -   P    -			     V
*  |	    | \	    n	   \	 |  n	   \    i   \  n+1		     V
*  | C(t) = | / P  B (t) = / P   | B (t) = / -----  / B   (t) =		     V
* / 	   /  -	 i  i	   -  i /   i	   - n + 1  -  j		     V
*            i=0          i=0             i=0     j=i+1			     V
*									     V
*        n+1 j-1							     V
*         -   -								     V
*     1   \   \	    n+1							     V
* = ----- /   / P  B   (t)						     V
*   n + 1 -   -  i  j							     V
*        j=1 i=0							     V
*									     V
*									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to integrate.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Integrated curve.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvIntegrate, BzrSrfIntegrate, CagdCrvIntegrate                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvIntegrate, integrals                                               M
*****************************************************************************/
CagdCrvStruct *BzrCrvIntegrate(const CagdCrvStruct *Crv)
{
    int i, j, k,
	n = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *IntCrv;

    if (CAGD_IS_RATIONAL_CRV(Crv))
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

    IntCrv = BzrCrvNew(n + 1, Crv -> PType);

    for (k = 1; k <= MaxCoord; k++) {
	CagdRType
	    *Points = Crv -> Points[k],
	    *IntPoints = IntCrv -> Points[k];

	for (j = 0; j < n + 1; j++) {
	    IntPoints[j] = 0.0;
	    for (i = 0; i < j; i++)
	        IntPoints[j] += Points[i];
	    IntPoints[j] /= n;
	}
    }

    return IntCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier curve into Bspline curve by adding an open knot vector.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     A Bezier curve to convert to a Bspline curve.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A Bspline curve representing Bezier curve Crv.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBsp2BzrCrv, CagdCnvrtBzr2PwrCrv, CagdCnvrtPwr2BzrCrv	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBzr2BspCrv, conversion                                          M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtBzr2BspCrv(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *BspCrv;

    if (Crv -> GType != CAGD_CBEZIER_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_CRV);
	return NULL;
    }

    BspCrv = CagdCrvCopy(Crv);

    BspCrv -> Order = BspCrv -> Length;
    BspCrv -> KnotVector = BspKnotUniformOpen(BspCrv -> Length,
					      BspCrv -> Order, NULL);
    BspCrv -> GType = CAGD_CBSPLINE_TYPE;

    CAGD_PROPAGATE_ATTR(BspCrv, Crv);

    return BspCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline curve into a set of Bezier curves by subdividing the    M
* Bspline curve at all its internal knots.				     M
*   Returned is a list of Bezier curves.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:     A Bspline curve to convert to a Bezier curve.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A list of Bezier curves representing the Bspline      M
*                      curve Crv.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBzr2BspCrv, CagdCnvrtBzr2PwrCrv, CagdCnvrtPwr2BzrCrv	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBsp2BzrCrv, conversion                                          M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtBsp2BzrCrv(const CagdCrvStruct *CCrv)
{
    int i, Order, Length;
    CagdRType LastT;
    CagdRType const *KnotVector;
    CagdCrvStruct *Crv,
	*BezierCrvs = NULL;

    if (CCrv -> GType != CAGD_CBSPLINE_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_CRV);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_CRV(CCrv) && !BspCrvHasOpenEC(CCrv))
	Crv = BspCrvOpenEnd(CCrv);
    else
	Crv = CagdCrvCopy(CCrv);

    Order = Crv -> Order,
    Length = Crv -> Length;
    KnotVector = CCrv -> KnotVector;            /* Reference original KV! */

    for (i = Length - 1, LastT = KnotVector[Length]; i >= Order; i--) {
    	CagdRType
    	    t = KnotVector[i];
    	    
	if (!IRIT_APX_EQ(LastT, t)) {
    	    CagdCrvStruct
		*Crvs = BspCrvSubdivAtParam(Crv, t);

	    CagdCrvFree(Crv);

    	    Crvs -> Pnext -> Pnext = BezierCrvs;
    	    BezierCrvs = Crvs -> Pnext;

    	    Crv = Crvs;
    	    Crv -> Pnext = NULL;

	    LastT = t;
    	}
    }

    Crv -> Pnext = BezierCrvs;
    BezierCrvs = Crv;

    for (Crv = BezierCrvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdRType TMin, TMax;

	CagdCrvDomain(Crv, &TMin, &TMax);

    	Crv -> GType = CAGD_CBEZIER_TYPE;
	Crv -> Length = Crv -> Order;
	IritFree(Crv -> KnotVector);
	Crv -> KnotVector = NULL;

	AttrSetRealAttrib(&Crv -> Attr, "BspDomainMin", TMin);
	AttrSetRealAttrib(&Crv -> Attr, "BspDomainMax", TMax);
    }

    return BezierCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply the Moebius transformation to a rational Bezier curve.             M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:       Curve to apply the Moebius transformation to.                M
*   c:          The scaling coefficient - c^n is the ratio between the first M
*	        and last weight of the curve.  				     M
*		If c == 0, the first and last weights are made equal.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The modified curve with the same shape but different  M
*		speed.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvMoebiusTransform, BzrSrfMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvMoebiusTransform                                                   M
*****************************************************************************/
CagdCrvStruct *BzrCrvMoebiusTransform(const CagdCrvStruct *CCrv, CagdRType c)
{
    int i, j,
	Order = CCrv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType);
    CagdRType c0, **Points,
	MaxW = IRIT_UEPS;    
    CagdCrvStruct *Crv;

    if (!CAGD_IS_BEZIER_CRV(CCrv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_CRV);
	return NULL;
    }

    if (!CAGD_IS_RATIONAL_CRV(CCrv)) {
        if (c == 1.0)
	    return CagdCrvCopy(CCrv);
	else
	    Crv = CagdCoerceCrvTo(CCrv, CAGD_MAKE_PT_TYPE(TRUE, MaxCoord), FALSE);
    }
    else
        Crv = CagdCrvCopy(CCrv);
    Points = Crv -> Points;

    if (Points[0][0] == 0 || Points[0][Order - 1] == 0) {
	CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
	return NULL;
    }

    if (c == 0.0) {
        c = Points[0][0] / Points[0][Order - 1];
	c = pow(c, 1.0 / (Order - 1.0));
    }

    for (c0 = c, i = 1; i < Order; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] *= c;
	c *= c0;
    }

    /* Normalize all weights so largest has magnitude of one. */
    for (i = 0; i < Order; i++) {
	if (MaxW < IRIT_FABS(Points[0][i]))
	    MaxW = IRIT_FABS(Points[0][i]);
    }
    for (i = 0; i < Order; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] /= MaxW;
    }

    return Crv;
}
