/******************************************************************************
* Cagd_Cnc.c - Curve representation of arcs and circles			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jun. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "irit_sm.h"
#include "extra_fn.h"
#include "geom_lib.h"
#include "cagd_loc.h"

#define CONIC_MAX_VAL		-10
#define CONIC_MIN_VAL		10
#define CONIC_DELTA_VAL		(CONIC_MAX_VAL - CONIC_MIN_VAL)

#define CAGD_QUADRIC_EXTENT	(CONIC_MAX_VAL)

#define QUADRIC_INVERT(x)	((x) == 0 ? IRIT_INFNTY : 1.0 / sqrt(x))

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct rational quadratic curve out of the 6 coefficients of the      M
* conic section: A x^2 + B xy + C y^2 + D x + E y + F = 0.		     M
*   Based on:								     M
* Bezier Curves and Surface Patches on Quadrics, by Josef Hoschek,           M
* Mathematical methods in Computer aided Geometric Design II, Tom Lyche and  M
* Larry L. Schumaker (eds.), pp 331-342, 1992.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F:   The six coefficients of the conic curve.             M
*   ZLevel:		Sets the Z level of this XY parallel conic curve.    M
*   RationalEllipses:   TRUE for ration ellipses,			     M
*                       FALSE for a polynomial approximation.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A quadratic curve representing the conic.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, CagdCrvCreateArc,	     M
*   BzrCrvCreateArc, CagdCreateConicCurve2, CagdCreateQuadricSrf	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCreateConicCurve                                                     M
*****************************************************************************/
CagdCrvStruct *CagdCreateConicCurve(CagdRType A,
				    CagdRType B,
				    CagdRType C,
				    CagdRType D,
				    CagdRType E,
				    CagdRType F,
				    CagdRType ZLevel,
				    CagdBType RationalEllipses)
{
    IrtVecType Trans;
    CagdRType **Points, 
	RotAngle = IRIT_APX_EQ(B, 0.0) ? 0.0 : atan2(B, A - C) * 0.5,
	A1 = ((A + C) + B * sin(2 * RotAngle) +
	      (A - C) * cos(2 * RotAngle)) * 0.5,
	B1 = B * cos(2 * RotAngle) - (A - C) * sin(2 * RotAngle),
	C1 = ((A + C) - B * sin(2 * RotAngle) -
	      (A - C) * cos(2 * RotAngle)) * 0.5,
	D1 = D * cos(RotAngle) + E * sin(RotAngle),
	E1 = -D * sin(RotAngle) + E * cos(RotAngle),
	F1 = F;
    CagdMType Mat;
    CagdCrvStruct *PwrCrv, *TCrv,
	*Crv = NULL;

    if (!IRIT_APX_EQ(B1, 0.0) ||
	(IRIT_APX_EQ_EPS(A1, 0.0, IRIT_UEPS) &&
	 IRIT_APX_EQ_EPS(C1, 0.0, IRIT_UEPS))) {
	CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	return NULL;
    }
    if (IRIT_APX_EQ_EPS(A1 * C1, 0.0, IRIT_UEPS)) {
	/* It is a parabola. */
        return CagdCreateConicCurve2(A, B, C, D, E, F, ZLevel, NULL, NULL,
				     RationalEllipses);
    }

    /* We now have conic: A1 x^2 + C1 y^2 + D1 x + E1 y + F1 = 0.            */
    /* Compute the translation factors by converting to,		     */
    /* A1(x + D1/(2A1))^2 + C1(y + E1/(2C1))^2 +			     */
    /*                                        F1 - D1^2/4A1 - E1^2/4C1 = 0.  */
    Trans[0] = -D1 / (2.0 * A1);
    Trans[1] = -E1 / (2.0 * C1);
    Trans[2] = ZLevel;

    F1 -= (C1 * IRIT_SQR(D1) + A1 * IRIT_SQR(E1)) / (4 * A1 * C1);

    if (A1 < 0.0) {			        /* Make sure A1 is positive. */
	A1 = -A1;
	C1 = -C1;
	F1 = -F1;
    }
    if (A1 * C1 > 0.0) {
        /* Conic is an ellipse A1 x^2 + C1 y^2 = -F1,  A1 > 0, C1 > 0. */
        if (F1 <= 0) {
	    CagdRType R[2];

	    Crv = RationalEllipses ? BspCrvCreateUnitCircle()
				   : BspCrvCreateUnitPCircle();

	    R[0] = sqrt(-F1 / A1);
	    R[1] = sqrt(-F1 / C1);

	    /* Scale the circular shape into an ellipse. */
	    MatGenMatScale(R[0], R[1], 1.0, Mat);
	    TCrv = CagdCrvMatTransform(Crv, Mat);
	    CagdCrvFree(Crv);
	    Crv = TCrv;

	    AttrSetStrAttrib(&Crv -> Attr, "conic", "Ellipse");
	}
	else {
	    CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	    return NULL;
	}
    }
    else {
        CagdBType SwapXY;

	if (F1 > 0.0) {
	    IRIT_SWAP(CagdRType, A1, C1);
	    A1 = -A1;
	    C1 = -C1;
	    F1 = -F1;
	    SwapXY = TRUE;
	}
	else
	    SwapXY = FALSE;

	C1 = -C1;
        /* Conic is now A1 x^2 - C1 y^2 = -F1,    A1 > 0,  C1 > 0,  -F1 > 0. */
	/* The above hyperbola can be written as a parametrization of,	     */
	/*              x = (1/w) * (a^2 + b^2) / sqrt(A1),		     */
	/*              y = (1/w) * 2ab / sqrt(C1),			     */
	/* and w equals w = (a^2 - b^2) / sqrt(-F1).			     */
	/* The only (large) remaining question is how to parametrize a, b.   */
	/* For no better selection, we select a = t, b = 1 - t, or	     */
	/*              x = (1/w) * (1 - 2 * t + 2 * t^2) / sqrt(A1),	     */
	/*              y = (1/w) * (2 * t - 2 * t^2) / sqrt(C1),	     */
	/* and w equals w = (-1 + 2 * t) / sqrt(-F1).			     */
	A1 = 1.0 / sqrt(A1);
	C1 = 1.0 / sqrt(C1);
	F1 = F1 == 0.0 ? IRIT_INFNTY : 1.0 / sqrt(-F1);  

	PwrCrv = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_P2_TYPE, 3);
	Points = PwrCrv -> Points;
    
	Points[0][0] = -F1;				      /* The w term. */
	Points[0][1] = 2.0 * F1;
	Points[0][2] = 0.0;
	Points[1][0] = A1;				      /* The x term. */
	Points[1][1] = -2.0 * A1;
	Points[1][2] = 2.0 * A1;
	Points[2][0] = 0.0;				      /* The y term. */
	Points[2][1] = 2.0 * C1;
	Points[2][2] = -2.0 * C1;

	if (SwapXY) {
	    IRIT_SWAP(CagdRType, Points[1][0], Points[2][0]);
	    IRIT_SWAP(CagdRType, Points[1][1], Points[2][1]);
	    IRIT_SWAP(CagdRType, Points[1][2], Points[2][2]);
	}
	
	Crv = CagdCnvrtPwr2BzrCrv(PwrCrv);
	CagdCrvFree(PwrCrv);

	AttrSetStrAttrib(&Crv -> Attr, "conic", "Hyper");
    }

    CagdCrvTransform(Crv, Trans, 1.0);
    MatGenMatRotZ1(RotAngle, Mat);
    TCrv = CagdCrvMatTransform(Crv, Mat);
    CagdCrvFree(Crv);
    Crv = TCrv;

    CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_CONIC_SEC);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct rational quadratic curve out of the 6 coefficients of the      M
* conic section: A x^2 + B xy + C y^2 + D x + E y + F = 0.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F:   The six coefficients of the conic curve.             M
*   ZLevel:		Sets the Z level of this XY parallel conic curve.    M
*   PStartXY, PEndXY:   Domain of conic section - starting/end points, in    M
*			the XY plane. If NULL, the most complete conic       M
*			possible is created.				     M
*   RationalEllipses:   TRUE for rational ellipses (if full ellipse),        M
*                       FALSE for a polynomial approximation.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A quadratic curve representing the conic.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, CagdCrvCreateArc,	     M
*   BzrCrvCreateArc, CagdCreateConicCurve, CagdCreateQuadricSrf		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCreateConicCurve2                                                    M
*****************************************************************************/
CagdCrvStruct *CagdCreateConicCurve2(CagdRType A,
				     CagdRType B,
				     CagdRType C,
				     CagdRType D,
				     CagdRType E,
				     CagdRType F,
				     CagdRType ZLevel,
				     const CagdRType *PStartXY,
				     const CagdRType *PEndXY,
				     CagdBType RationalEllipses)
{
    IrtVecType Trans;
    CagdRType Desc, x, x1, x2, dx, y1, y2, dy,
	RotAngle = IRIT_APX_EQ(B, 0.0) ? 0.0 : atan2(B, A - C) * 0.5,
	A1 = ((A + C) + B * sin(2 * RotAngle) +
	      (A - C) * cos(2 * RotAngle)) * 0.5,
	B1 = B * cos(2 * RotAngle) - (A - C) * sin(2 * RotAngle),
	C1 = ((A + C) - B * sin(2 * RotAngle) -
	      (A - C) * cos(2 * RotAngle)) * 0.5,
	D1 = D * cos(RotAngle) + E * sin(RotAngle),
	E1 = -D * sin(RotAngle) + E * cos(RotAngle),
	F1 = F;
    IrtHmgnMatType Mat;
    CagdCrvStruct *TCrv,
	*Crv = NULL;

    if (!IRIT_APX_EQ(B1, 0.0) ||
	(IRIT_APX_EQ_EPS(A1, 0.0, IRIT_UEPS) &&
	 IRIT_APX_EQ_EPS(C1, 0.0, IRIT_UEPS))) {
	CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	return NULL;
    }

    /* Computes the translation factors. */
    Trans[0] = Trans[1] = 0.0;
    Trans[2] = ZLevel;

    /* We now have conic: A1 x^2 + C1 y^2 + D1 x + E1 y + F1 = 0. */

    Desc = A1 * C1;

    if (IRIT_APX_EQ_EPS(Desc, 0.0, IRIT_UEPS)) {
	CagdCrvStruct
	    *PwrCrv = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E3_TYPE, 3);
	 CagdRType
	     **Points= PwrCrv -> Points;

	/* Its a parabola. */

	PwrCrv -> Order = PwrCrv -> Length = 3;
	IRIT_ZAP_MEM(PwrCrv -> Points[1], 3 * sizeof(CagdRType));
	IRIT_ZAP_MEM(PwrCrv -> Points[2], 3 * sizeof(CagdRType));
	IRIT_ZAP_MEM(PwrCrv -> Points[3], 3 * sizeof(CagdRType));

	if (!IRIT_APX_EQ(A1, 0.0) && !IRIT_APX_EQ(E1, 0.0)) {
	    /* Update the translation factors. */
	    Trans[0] = -D1 / (2.0 * A1);
	    Trans[1] = -(F1 - IRIT_SQR(D1) / (4 * A1)) / E1;

	    /* Update the control points. */
	    if (PStartXY != NULL && PEndXY != NULL) {
		x1 = PStartXY[0] - Trans[0];
		x2 = PEndXY[0]   - Trans[0];
		y1 = PStartXY[1] - Trans[1];
		y2 = PEndXY[1]   - Trans[1];
		dx = x2 - x1;
		dy = y2 - y1;
	    }
	    else {
		x1 = CONIC_MIN_VAL - Trans[0];
		x2 = CONIC_MAX_VAL - Trans[0];
		y1 = CONIC_MIN_VAL - Trans[1];
		y2 = CONIC_MAX_VAL - Trans[1];
		dx = CONIC_DELTA_VAL;
		dy = CONIC_DELTA_VAL;
	    }

	    x  = x1 * cos(RotAngle) - y1 * sin(RotAngle);
	    y1 = x1 * sin(RotAngle) + y1 * cos(RotAngle);
	    x1 = x;

	    Points[1][0] = x1;
	    Points[1][1] = dx;
	    Points[2][0] = IRIT_SQR(x1) * (- A1 / E1);
	    Points[2][1] = 2 * dx * x1 * (- A1 / E1);
	    Points[2][2] = IRIT_SQR(dx) * (- A1 / E1);
	}
	else if (!IRIT_APX_EQ(C1, 0.0) && !IRIT_APX_EQ(D1, 0.0)) {
	    /* Update the translation factors. */
	    Trans[0] = -(F1 - IRIT_SQR(E1) / (4 * C1)) / D1;
	    Trans[1] = -E1 / (2.0 * C1);

	    /* Update the control points. */
	    if (PStartXY != NULL && PEndXY != NULL) {
		x1 = PStartXY[0] - Trans[0];
		x2 = PEndXY[0]   - Trans[0];
		y1 = PStartXY[1] - Trans[1];
		y2 = PEndXY[1]   - Trans[1];
		dx = x2 - x1;
		dy = y2 - y1;
	    }
	    else {
		x1 = CONIC_MIN_VAL - Trans[0];
		x2 = CONIC_MAX_VAL - Trans[0];
		y1 = CONIC_MIN_VAL - Trans[1];
		y2 = CONIC_MAX_VAL - Trans[1];
		dx = CONIC_DELTA_VAL;
		dy = CONIC_DELTA_VAL;
	    }

	    x  = x1 * cos(RotAngle) - y1 * sin(RotAngle);
	    y1 = x1 * sin(RotAngle) + y1 * cos(RotAngle);
	    x1 = x;

	    Points[1][0] = IRIT_SQR(y1) * (- C1 / D1);
	    Points[1][1] = 2 * dy * y1 * (- C1 / D1);
	    Points[1][2] = IRIT_SQR(dy) * (- C1 / D1);
	    Points[2][0] = y1;
	    Points[2][1] = dy;
	}
	else {
	    CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	    return NULL;
	}
	Crv = CagdCnvrtPwr2BzrCrv(PwrCrv);
	AttrSetStrAttrib(&Crv -> Attr, "conic", "Parab");

	CagdCrvFree(PwrCrv);
    }
    else {
	/* Update the translation factors. */
	Trans[0] = -D1 / (2.0 * A1);
	Trans[1] = -E1 / (2.0 * C1);

	if (PStartXY != NULL && PEndXY != NULL) {
	    x1 = PStartXY[0] - Trans[0];
	    x2 = PEndXY[0]   - Trans[0];
	    y1 = PStartXY[1] - Trans[1];
	    y2 = PEndXY[1]   - Trans[1];
	    x  = x1 * cos(RotAngle) - y1 * sin(RotAngle);
	    y1 = x1 * sin(RotAngle) + y1 * cos(RotAngle);
	    x1 = x;
	    dx = x2 - x1;
	    dy = y2 - y1;
	}
	else
	    x1 = y1 = x2 = y2 = -IRIT_INFNTY;

	F1 -= (C1 * IRIT_SQR(D1) + A1 * IRIT_SQR(E1)) / (4 * A1 * C1);

	/* We now have conic in canonic form of: A1 x^2 + C1 y^2 + F1 = 0. */
	if (Desc > 0) {
	    CagdRType R[2];

	    /* Its an ellipse. */
	    if (F1 / A1 < 0 && F1 / C1 < 0) {
		IRIT_STATIC_DATA CagdPtStruct
		    POrigin = { NULL, NULL, { 0.0, 0.0, 0.0 } };

		R[0] = sqrt(-F1 / A1);
		R[1] = sqrt(-F1 / C1);

		if (PStartXY != NULL && PEndXY != NULL) {
		    CagdRType StartAngle, EndAngle;

		    /* Create an ellipse from PStartXY to PEndXY! */
		    StartAngle = atan2(y1 / R[1], x1 / R[0]);
		    if (StartAngle < 0.0)
			StartAngle = IRIT_RAD2DEG(StartAngle) + 360.0;
		    else
			StartAngle = IRIT_RAD2DEG(StartAngle);
		    EndAngle = atan2(y2 / R[1], x2 / R[0]);
		    if (EndAngle < 0.0)
			EndAngle = IRIT_RAD2DEG(EndAngle) + 360.0;
		    else
			EndAngle = IRIT_RAD2DEG(EndAngle);

		    Crv = CagdCrvCreateArc(&POrigin, 1.0,
					   StartAngle, EndAngle);
		}
		else {
		    /* Create a full ellipse! */
		    Crv =  RationalEllipses ? BspCrvCreateUnitCircle()
					    : BspCrvCreateUnitPCircle();
		}
	    }
	    else {
		CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
		return NULL;
	    }

	    /* Scale the circular shape into an ellipse. */
	    MatGenMatScale(R[0], R[1], 1.0, Mat);
	    TCrv = CagdCrvMatTransform(Crv, Mat);
	    CagdCrvFree(Crv);
	    Crv = TCrv;
	    AttrSetStrAttrib(&Crv -> Attr, "conic", "Ellipse");
	}
	else { /* t < 0 */
	    /* Its an hyperbola. */
	    CAGD_FATAL_ERROR(CAGD_ERR_HYPERBOLA_NO_SUPPORT);
	    return NULL;
	}
    }

    CagdCrvTransform(Crv, Trans, 1.0);
    MatGenMatRotZ1(RotAngle, Mat);
    TCrv = CagdCrvMatTransform(Crv, Mat);
    CagdCrvFree(Crv);
    Crv = TCrv;

    CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_CONIC_SEC);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handles construction of singular conics, when the conic degenerates      M
* into line/2lines out of the 6 coefficients:				     M
*	 A x^2 + B xy + C y^2 + D x + E y + F = 0.			     M
*   The conic is singular if the following 3x3 determinant vanishes: 	     M
*									     M
*	     |	 A      0.5*B   0.5*D  |				     V
*	     |  0.5*B     C     0.5*E  |				     V
*	     |  0.5*D   0.5*E    F     |				     V
*									     M
* See also http://mathworld.wolfram.com/QuadraticCurve.html.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F:   The six coefficients of the singular conic curve.    M
*   ZLevel:		Sets the Z level of this XY parallel conic curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A line/list of lines representing the singular conic.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCreateConicCurve						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCreateConicCurveSingular                                             M
*****************************************************************************/
CagdCrvStruct *CagdCreateConicCurveSingular(CagdRType A, 
					    CagdRType B,
					    CagdRType C,
					    CagdRType D,
					    CagdRType E,
					    CagdRType F,
					    CagdRType ZLevel)
{
    CagdCrvStruct *Line2, 
	*Line1 = NULL;
    IrtPtType P1, P2, V, S1, S2;
    IrtVecType Vec1, Vec2, TVec1, TVec2;
    IrtGnrlMatType MQ;
    CagdRType Det, DetX, DetY, DiscDelta, DiscJ, DiscK;
  
    DiscDelta = Mat3x3Determinant(A, 0.5*B, 0.5*D,
				  0.5*B, C, 0.5*E,
				  0.5*D, 0.5*E, F);
    DiscJ = Mat2x2Determinant(A, 0.5*B, 0.5*B, C);
    DiscK = Mat2x2Determinant(A, 0.5*D, 0.5*D, F) +
            Mat2x2Determinant(C, 0.5*E, 0.5*E, F);

    /* Vec1 and Vec2 are the points on the conic in the infinity.           */
    /* Homogenous coordinate is the last one!				    */
    Vec1[2] = 0.0;
    Vec2[2] = 0.0;

    /* Conic is assumed to be singular. */
    assert(IRIT_APX_EQ(DiscDelta, 0.0));

    /* Conic is singular, but form is still quadratic. */
    if (!(IRIT_APX_EQ(A, 0.0) &&
	  IRIT_APX_EQ(B, 0.0) &&
	  IRIT_APX_EQ(C, 0.0))) { 
	/* Two intersecting real lines. */
	if ((DiscJ < 0) && !IRIT_APX_EQ(DiscJ, 0.0)) {
	    /* One line is horizontal!!! */
	    if (IRIT_APX_EQ(A,0.0)) {
		Vec1[0] = 1.0;
		Vec1[1] = 0.0;

		Vec2[0] = C;
		Vec2[1] = -B;
	    }
	    else {
		Vec1[0] = (-B + 2 * sqrt(-DiscJ))/ (2*A);
		Vec1[1] = 1.0;

		Vec2[0] = (-B - 2 * sqrt(-DiscJ))/ (2*A);
		Vec2[1] = 1.0;
	    }

	    /* The matrix of bilinear form; its zero set is the conic      */
	    /* section.						           */
	    MQ = (IrtGnrlMatType) IritMalloc(sizeof(IrtRType) * 9);

	    MQ[0] = A;
	    MQ[1] = B * 0.5;
	    MQ[2] = D * 0.5;

	    MQ[3] = B * 0.5;
	    MQ[4] = C;
	    MQ[5] = E * 0.5;

	    MQ[6] = D * 0.5;
	    MQ[7] = E * 0.5;
	    MQ[8] = F;

	    /* Vec1 * Q = 1st line, Vec2 * Q = 2nd line, their             */
	    /* intersection is V = point conjugated with all pts of plane. */
	    MatGnrlMultVecbyMat(TVec1, MQ, Vec1, 3);
	    MatGnrlMultVecbyMat(TVec2, MQ, Vec2, 3);

	    IritFree(MQ);

	    /* Intersection point by Cramer rule. */
	    Det = Mat2x2Determinant(TVec1[0], TVec2[0], TVec1[1], TVec2[1]);
	    DetX = Mat2x2Determinant(-TVec1[2], -TVec2[2], TVec1[1], TVec2[1]); 
	    DetY = Mat2x2Determinant(TVec1[0], TVec2[0], -TVec1[2], -TVec2[2]); 

	    V[0] = DetX / Det;
	    V[1] = DetY / Det;
	    V[2] = ZLevel; 

	    GMVecNormalize(Vec1);
	    GMVecNormalize(Vec2); 
	    IRIT_PT_ADD(P1, V, Vec1);
	    IRIT_PT_ADD(P2, V, Vec2);
	    Line1 = CagdMergePtPt2(V, P1);
	    Line2 = CagdMergePtPt2(V, P2);

	    Line1 -> Pnext = Line2;
	}
	else if (IRIT_APX_EQ(DiscJ,0.0)) {     	          /* Parallel lines. */
	    /* Two real. */
	    if (DiscK < 0.0) {		
		/* Lines are horizontal. */
		if (IRIT_APX_EQ(A,0.0)) {
		    Vec1[0] = 1.0;
		    Vec1[1] = 0.0;

		    /* Quadric intersected y-axis gives Cy^2 + Ey + F = 0. */
		    S1[0] = S2[0] = 0;
		    S1[1] = (-E + sqrt(E*E - 4*C*F))/(2*C);
		    S2[1] = (-E - sqrt(E*E - 4*C*F))/(2*C);
		    S1[2] = S2[2] = ZLevel; 		
		}
		else {
		    Vec1[0] = -B / (2*A);
		    Vec1[1] = 1.0;

		    /* Quadric intersected x-axis gives Ax^2 + Dx + F = 0. */
		    S1[0] = (-D + sqrt(D*D - 4*A*F))/(2*A);
		    S2[0] = (-D - sqrt(D*D - 4*A*F))/(2*A);	   
		    S1[1] = S2[1] = 0;
		    S1[2] = S2[2] = ZLevel; 
		}

		GMVecNormalize(Vec1);
		IRIT_PT_ADD(P1, S1, Vec1);
		IRIT_PT_ADD(P2, S2, Vec1);
		Line1 = CagdMergePtPt2(S1, P1);
		Line2 = CagdMergePtPt2(S2, P2);

		Line1 -> Pnext = Line2;
	    }
	    else if (IRIT_APX_EQ(DiscK,0.0)) {         /* Coincident lines. */
		/* Lines are horizontal. */
		if (IRIT_APX_EQ(A,0.0)) {
		    Vec1[0] = 1.0;
		    Vec1[1] = 0.0;

		    /* Quadric intersected y-axis gives Cy^2 + Ey + F = 0. */
		    S1[0] = 0;
		    S1[1] = -E /(2*C);
		    S1[2] = ZLevel; 		
		}
		else {
		    Vec1[0] = -B/(2*A);
		    Vec1[1] = 1.0;

		    /* Quadric intersected x-axis gives Ax^2 + Dx + F = 0. */
		    S1[0] = -D/(2*A);
		    S1[1] = 0;
		    S1[2] = ZLevel; 
		}
		GMVecNormalize(Vec1);
		IRIT_PT_ADD(P1, S1, Vec1);
		Line1 = CagdMergePtPt2(S1, P1);  	    
	    }
	}
    }
    /* Linear Dx + Ey + F = 0. */
    else {
	if (IRIT_APX_EQ(D,0.0)) {
	    if (!IRIT_APX_EQ(E,0.0)) {
		Vec1[0] = 1.0;
		Vec1[1] = 0.0;

		S1[0] = 0;
		S1[1] = -F /E;
		S1[2] = ZLevel;		
	    }
	    else {
		return NULL;
	    }	    
	}
	else {
	    if (!IRIT_APX_EQ(E,0.0)) {
		Vec1[0] = -E;
		Vec1[1] = D;

		S1[0] = 0;
		S1[1] = -F /E;
		S1[2] = ZLevel;	
	    }
	    else {
		Vec1[0] = 0.0;
		Vec1[1] = 1.0;

		S1[0] = -F/D;
		S1[1] = 0;
		S1[2] = ZLevel;	
		
	    }		   
	}
	GMVecNormalize(Vec1);
	IRIT_PT_ADD(P1, S1, Vec1);
	Line1 = CagdMergePtPt2(S1, P1);
    }
    return Line1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an ellipse in the XY plane through the given 3 points of      M
* minimal area.  The A,B,C,D,E,F coefficients of the bounding ellipse as in  M
* A x^2 + B xy + C y^2 + D x + E y + F = 0.				     M
* are returned.                                                              M
*   Algorithm:								     V
* 1. Compute center,  C := (Pt1 + Pt2 + Pt3) / 3			     V
*									     V
*                                    3					     V
* 2. Computer a 2x2 matrix  N = 1/3 Sum (Pti - C) (Pti - C)^T		     V
*                                   i=1					     V
*									     V
* 3. M = N^{-1}								     V
*									     V
* 4. The ellipse E: (P - C)^T M (P - C) - Z = 0,    Z constant, P = (x, y).  V
*									     M
* See also:  "Exact Primitives for Smallest Enclosing Ellipses",	     M
* by Bernd Gartner and Sven Schonherr, Proceedings of the 13th annual 	     M
* symposium on Computational geometry, 1997.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:    The 3 input points.  Assumed non-colnear.		     M
*   A, B, C, D, E, F: Coefficients of the computed bounding ellipse.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if succesful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdEllipseOffset, CagdCreateConicCurve, CagdCreateConicCurve2,          M
*   CagdEllipse4Points							     M
*									     *
* KEYWORDS:                                                                  M
*   CagdEllipse3Points, ellipse                                              M
*****************************************************************************/
int CagdEllipse3Points(CagdPType Pt1,
		       CagdPType Pt2,
		       CagdPType Pt3,
		       CagdRType *A,
		       CagdRType *B,
		       CagdRType *C,
		       CagdRType *D,
		       CagdRType *E,
		       CagdRType *F)
{
    CagdPType Center, DPt1, DPt2, DPt3;
    CagdRType Det, Nrml, N[2][2], M[2][2];

    /* Compute the average center to points. */
    IRIT_PT_COPY(Center, Pt1);
    IRIT_PT_ADD(Center, Center, Pt2);
    IRIT_PT_ADD(Center, Center, Pt3);
    IRIT_PT_SCALE(Center, 1.0 / 3.0);

    IRIT_PT_SUB(DPt1, Pt1, Center);
    IRIT_PT_SUB(DPt2, Pt2, Center);
    IRIT_PT_SUB(DPt3, Pt3, Center);

    N[0][0] = (IRIT_SQR(DPt1[0]) + IRIT_SQR(DPt2[0]) + IRIT_SQR(DPt3[0])) / 3;
    N[1][0] = N[0][1] = (DPt1[0] * DPt1[1] +
			 DPt2[0] * DPt2[1] +
			 DPt3[0] * DPt3[1]) / 3;
    N[1][1] = (IRIT_SQR(DPt1[1]) + IRIT_SQR(DPt2[1]) + IRIT_SQR(DPt3[1])) / 3;

    Det = N[0][0] * N[1][1] - N[0][1] * N[1][0];
    if (IRIT_APX_EQ_EPS(Det, 0.0, IRIT_UEPS))
        return FALSE;

    M[0][0] = N[1][1] / Det;
    M[1][1] = N[0][0] / Det;
    M[0][1] = M[1][0] = -N[0][1] / Det;

    *A = M[0][0];
    *B = M[0][1] + M[1][0];
    *C = M[1][1];
    Nrml = 1.0 / IRIT_MAX(IRIT_MAX(IRIT_FABS(*A), IRIT_FABS(*B)),
			  IRIT_FABS(*C));
    if (IRIT_APX_EQ_EPS(Nrml, 0.0, IRIT_UEPS))
        return FALSE;
    *A *= Nrml;
    *B *= Nrml;
    *C *= Nrml;

    *D = (-2 * M[0][0] * Center[0] -
	  Center[1] * (M[1][0] + M[0][1])) * Nrml;
    *E = (-2 * M[1][1] * Center[1] -
	  Center[0] * (M[1][0] + M[0][1])) * Nrml;

    *F = -(*A * IRIT_SQR(Pt1[0]) +
	   *B * Pt1[0] * Pt1[1] +
	   *C * IRIT_SQR(Pt1[1]) +
	   *D * Pt1[0] +
	   *E * Pt1[1]);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an ellipse in the XY plane through the given 4 points of      M
* minimal area.  The A,B,C,D,E,F coefficients of the bounding ellipse as in  M
* A x^2 + B xy + C y^2 + D x + E y + F = 0.				     M
* are returned.                                                              M
*   Algorithm:								     V
*									     M
* 1. Using the four points (x1,y1)..(x4,y4), the following matrices:	     M
*									     M
*		| x1^2  y1^2  2*x1*y1  2*x1  2*y1  1 |			     M
*   ACBDEF   =	| x2^2  y2^2  2*x2*y2  2*x2  2*y2  1 |			     M
*		| x3^2  y3^2  2*x3*y3  2*x3  2*y3  1 |			     M
*		| x4^2  y4^2  2*x4*y4  2*x4  2*y4  1 |			     M
*									     M
*		| 2*x1*y1  2*x1  2*y1  1 |				     M
*   matBDEF  =	| 2*x2*y2  2*x2  2*y2  1 |				     M
*		| 2*x3*y3  2*x3  2*y3  1 |				     M
*		| 2*x4*y4  2*x4  2*y4  1 |				     M
*									     M
*  and the following vectors:						     M
*   									     M
*  x = | A  C  B  D  E  F |						     M
*									     M
*  Zeros = | 0  0  0  0 |						     M
*									     M
*  are defined, and the 4 point interpolation constraints can be written as: M
*									     M
*  ACBDEF * x = Zeros							     M
*									     M
* 2. Using this, the following can be written:				     M
*									     M
*  BDEFfromAC =	inv(matBDEF)*ACBDEF					     M
*									     M
*  which, by construction, is a matrix in the form:			     M
*									     M
*		   | AB  CB  1  0  0  0 |				     M
*     BDEFfromAC = | AD  CD  0  1  0  0 |				     M
*		   | AE  CE  0  0  1  0 |				     M
*		   | AF  CF  0  0  0  1 |				     M
*									     M
*  Where AB, AD, AE, AF, CB, CD, CE, CF are functions of (x1,y1)..(x4,y4).   M
*									     M
*  Using the previous equations, it is possible to write:		     M
*									     M
*    BDEFfromAC * x = Zeros						     M
*									     M
*  And conclude the following equations:				     M
*									     M
*    AB * A + CB * C + B = 0						     M
*    AD * A + CD * C + D = 0						     M
*    AE * A + CE * C + E = 0						     M
*    AF * A + CF * C + F = 0						     M
*									     M
* 3. The A..F ellipse coefficients can be scaled so that C = 1 - A, allowing M
*  the following equations to be written, using the previous step:	     M
*									     M
*   C(A) =	(-1) * A +  1						     M
*   B(A) = (CB - AB) * A - CB						     M
*   D(A) = (CD - AD) * A - CD						     M
*   E(A) = (CE - AE) * A - CE						     M
*   F(A) = (CF - AF) * A - CF						     M
*									     M
*									     M
* 4. This allows writing the ellipse area function:			     M
*									     M
*   area(A) = f(A)^3 / g(A)^2						     M
*									     M
*  where:								     M
*									     M
*  f(A) = A*C(A)-B(A)^2							     M
*  g(A) = -d(A)^2*C(A)+2*d(A)*E(A)*B(A)-E(A)^2*A+F(A)*A*C(A)-F(A)*B(A)^2     M
*									     M
*  as a function of A alone. The extreme (minimum) area values can be found  M
*  by locating where its derivative with respect to A equals zero, by 	     M
*  solving the cubic polynomial:					     M
*									     M
*    (-3*diff(f(A),A)*g(A) + 2*f(A)*diff(g(A),A))			     M
*									     M
*  which coefficients are defined using AB, AD, AE, AF, CB, CD, CE, CF 	     M
*  mentioned above.							     M
*									     M
* If solutions for A exist, they define the other ellipse coefficients	     M
* (B, C, D, E, F) as mentioned above. If these coefficients define an	     M
* ellipse, they are returned to the user. Otherwise, the function returns    M
* false.								     M
*									     M
* See also:  "Exact Primitives for Smallest Enclosing Ellipses",	     M
* by Bernd Gartner and Sven Schonherr, Proceedings of the 13th annual 	     M
* symposium on Computational geometry, 1997.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3, Pt4:    The 4 input points.  Assumed in general position. M
*   A, B, C, D, E, F: Coefficients of the computed bounding ellipse.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdEllipseOffset, CagdCreateConicCurve, CagdCreateConicCurve2           M
*									     *
* KEYWORDS:                                                                  M
*   CagdEllipse4Points, ellipse                                              M
*****************************************************************************/
int CagdEllipse4Points(CagdPType Pt1,
		       CagdPType Pt2,
		       CagdPType Pt3,
		       CagdPType Pt4,
		       CagdRType *A,
		       CagdRType *B,
		       CagdRType *C,
		       CagdRType *D,
		       CagdRType *E,
		       CagdRType *F)
{
    IrtRType *Pts[4], matBDEF[16], BDEFfromAC[8],invmatBDEF[16], aCoeffs[4],
	   aSolutions[3], b0, b1, d0, d1, e0, e1, f0, f1;
    int solCount, pIndex, bdefIndex;

    Pts[0] = Pt1;
    Pts[1] = Pt2;
    Pts[2] = Pt3;
    Pts[3] = Pt4;

    /* Create the matBDEF mentioned in the algorithm, using the input points */
    for (pIndex = 0; pIndex < 4; pIndex++) {
	matBDEF[4 * pIndex + 0] = 2.0 * Pts[pIndex][0] *
	                                Pts[pIndex][1];
	matBDEF[4 * pIndex + 1] = 2.0 * Pts[pIndex][0];
	matBDEF[4 * pIndex + 2] = 2.0 * Pts[pIndex][1];
	matBDEF[4 * pIndex + 3] = 1.0;
    }

    /* finding inv(matBDEF). Fails of points are not in general position. */
    if (!MatGnrlInverseMatrix(matBDEF, invmatBDEF, 4))
	return FALSE;

    /* calculating the necessary part of BDEFfromAC matrix by multiplying */
    /* the two left columns of ACBDEF by inv(matBDEF)			  */
    for (bdefIndex = 0; bdefIndex < 4; bdefIndex++) {
	BDEFfromAC[2 * bdefIndex] = 0.0;
	BDEFfromAC[2 * bdefIndex + 1] = 0.0;

	for (pIndex = 0; pIndex < 4; pIndex++) {
	    BDEFfromAC[2 * bdefIndex] +=
	        invmatBDEF[4 * bdefIndex + pIndex] * 
	        Pts[pIndex][0] * Pts[pIndex][0];

	    BDEFfromAC[2 * bdefIndex + 1] +=
		invmatBDEF[4 * bdefIndex + pIndex] * 
		Pts[pIndex][1] * Pts[pIndex][1];
	}
    }

    /* Using BDEFfromAC matrix, and the fact that C = 1 - A, the following   */
    /* describes B, D, E, F ellipse coefficients using A alone, in the form: */
    /* B = b0 * A + b1	    (example for B)				     */
    b0 =  BDEFfromAC[2 * 0 + 1] - BDEFfromAC[2 * 0 + 0];
    b1 = -BDEFfromAC[2 * 0 + 1];
    d0 =  BDEFfromAC[2 * 1 + 1] - BDEFfromAC[2 * 1 + 0];
    d1 = -BDEFfromAC[2 * 1 + 1];
    e0 =  BDEFfromAC[2 * 2 + 1] - BDEFfromAC[2 * 2 + 0];
    e1 = -BDEFfromAC[2 * 2 + 1];
    f0 =  BDEFfromAC[2 * 3 + 1] - BDEFfromAC[2 * 3 + 0];
    f1 = -BDEFfromAC[2 * 3 + 1];

    /* This part was solved in maple by deriving the area function by A, and */
    /* collecting the resulting cubic polynomial coefficients.               */
    /* The full maple algorithm is attached below. The cubic polynomial      */
    /* equation that defines the derivation of the area function is plotted  */
    /* as variable "areaExtremeEq1"					     */

    /*  Maple code:
	with(LinearAlgebra):
	
	M_ACBDEF := Matrix(4,6):
	ACBDEF := [x^2,y^2, 2*x*y,2*x,2*y,1]:
	for i in [1,2,3,4] do
	  M_ACBDEF[i,1..6] := Matrix(1,6,subs({x = x[i], y = y[i]}, ACBDEF)):
	end do:
	
	M_BDEF := M_ACBDEF[[1..4],[3..6]]:
	M_invBDEF := (MatrixInverse(M_BDEF)):
	M_invACBDEF := simplify(MatrixMatrixMultiply(M_invBDEF, M_ACBDEF)):
	
	substitutions1 := {f(A) = A*C(A)-B(A)^2,
                           g(A) = -d(A)^2*C(A)+2*d(A)*E(A)*B(A)-E(A)^2*A+F(A)*A*C(A)-F(A)*B(A)^2}:

	areaExtreme1 := f(A)^3/g(A)^2:
	areaExtreme2 := simplify(diff(areaExtreme1, A)):
	areaExtreme3 := (-3*diff(f(A),A)*g(A)+2*f(A)*diff(g(A),A)):
	areaExtreme4 := subs(substitutions1, areaExtreme3):
	areaExtremeEq1 := collect(simplify(eval(subs({B(A) = b0*A + b1, C(A) = -A + 1, d(A) = d0*A +d1, E(A) = e0*A +e1, F(A) = f0*A +f1}, areaExtreme4))), A);
	
	substitutions2 := {
	    b0 = -M_invACBDEF[1,1] + M_invACBDEF[1,2],
	    b1 = -M_invACBDEF[1,2],
	    d0 = -M_invACBDEF[2,1] + M_invACBDEF[2,2],
	    d1 = -M_invACBDEF[2,2],
	    e0 = -M_invACBDEF[3,1] + M_invACBDEF[3,2],
	    e1 = -M_invACBDEF[3,2],
	    f0 = -M_invACBDEF[4,1] + M_invACBDEF[4,2],
	    f1 = -M_invACBDEF[4,2]
	}:
	
	areaExtremeEq2 := subs(substitutions2, areaExtremeEq1):
	
	pointsSubs := {
	    x[1] = -1, y[1] = 1,
	    x[2] = 1, y[2] = 1,
	    x[3] = -1, y[3] = -1,
	    x[4] = 1, y[4] = -3/2
        }:
	
	FinalEq := eval(subs(pointsSubs, areaExtremeEq2)):
	solve(FinalEq):
    */

    aCoeffs[3] = (-IRIT_SQR(b0)*f0+4*IRIT_CUBE(b0)*d0*e1+4*d1*e0*b0+6*b1*b0*IRIT_SQR(e0)-4*IRIT_SQR(b0)*e1*e0+IRIT_SQR(d0)+4*IRIT_CUBE(b0)*d1*e0-2*f1+2*b1*IRIT_CUBE(b0)*f0-6*b1*b0*IRIT_SQR(d0)+2*b1*b0*f0+4*d1*d0-f0-3*IRIT_SQR(e0)-2*pow(b0,4)*f1-2*IRIT_SQR(b0)*IRIT_SQR(d0)-4*e1*e0-8*b1*IRIT_SQR(b0)*d0*e0-4*IRIT_SQR(b0)*f1+4*d0*e1*b0+4*d0*e0*b1+6*d0*e0*b0+4*IRIT_SQR(b0)*d1*d0);

    aCoeffs[2] = ((f0+3*f1-8*IRIT_SQR(b0)*d1*d0+2*b1*b0*IRIT_SQR(d0)-6*b1*IRIT_CUBE(b0)*f1+6*IRIT_SQR(b1)*IRIT_SQR(b0)*f0+3*IRIT_SQR(b0)*f1+2*f0*IRIT_SQR(b1)-4*b1*b0*f0-6*b1*b0*f1+2*d0*e0*b1-16*IRIT_SQR(b1)*b0*d0*e0+4*b1*IRIT_SQR(b0)*d0*e1-6*d1*d0+2*d1*e0*b0+8*d1*e0*b1-4*IRIT_SQR(e1)+4*IRIT_SQR(d1)+2*d0*e1*b0+8*d0*e1*b1+8*d1*e1*b0-IRIT_SQR(d0)+6*IRIT_SQR(b1)*IRIT_SQR(e0)-2*e1*e0-4*b1*b0*d1*d0+4*IRIT_SQR(b0)*IRIT_SQR(d1)+4*IRIT_SQR(b0)*d1*e0*b1+8*IRIT_CUBE(b0)*d1*e1-6*IRIT_SQR(b1)*IRIT_SQR(d0)-4*IRIT_SQR(b0)*IRIT_SQR(e1)+4*b1*b0*e1*e0));

    aCoeffs[1] = ((-4*IRIT_SQR(b1)*b0*d1*e0+4*IRIT_SQR(b1)*IRIT_SQR(d0)-4*IRIT_SQR(b1)*b0*d0*e1+6*IRIT_CUBE(b1)*b0*f0-6*IRIT_SQR(b1)*IRIT_SQR(b0)*f1-7*IRIT_SQR(d1)+2*b1*b0*IRIT_SQR(d1)-4*b1*b0*d1*d0-3*f0*IRIT_SQR(b1)+4*b1*b0*f1-8*IRIT_SQR(b1)*d1*d0-f1+12*d1*e1*b1-2*d1*e1*b0+2*d1*d0-2*d1*e0*b1-2*d0*e1*b1+8*IRIT_SQR(b1)*e1*e0-6*IRIT_SQR(b0)*IRIT_SQR(d1)-2*b1*b0*IRIT_SQR(e1)-2*f1*IRIT_SQR(b1)-8*IRIT_CUBE(b1)*d0*e0+IRIT_SQR(e1)+16*IRIT_SQR(b0)*d1*e1*b1));

    aCoeffs[0] = (-2*IRIT_SQR(b1)*IRIT_SQR(d1)-4*IRIT_CUBE(b1)*d0*e1-6*b1*b0*IRIT_SQR(d1)+4*IRIT_SQR(b1)*d1*d0-2*IRIT_CUBE(b1)*b0*f1+8*IRIT_SQR(b1)*b0*d1*e1+2*pow(b1,4)*f0-4*IRIT_CUBE(b1)*d1*e0+2*IRIT_SQR(b1)*IRIT_SQR(e1)+f1*IRIT_SQR(b1)-6*d1*e1*b1+3*IRIT_SQR(d1));

    if (fabs(aCoeffs[3] > IRIT_EPS))
        solCount = GMSolveCubicEqn(aCoeffs[2] / aCoeffs[3],
				   aCoeffs[1] / aCoeffs[3], 
				   aCoeffs[0] / aCoeffs[3], aSolutions);
    else
	solCount = 0;

    if (solCount == 3) {
        /* For three solutions to this cubic, the middle one is taken, as   */
	/* the two extreme ones describe non-ellipse conics.                */
	if ((aSolutions[0] > aSolutions[1]) ^ (aSolutions[0] > aSolutions[2]))
	    *A = aSolutions[0];
	else if ((aSolutions[1] > aSolutions[0]) ^ (aSolutions[1] > aSolutions[2]))
	    *A = aSolutions[1];
	else
	    *A = aSolutions[2];
    }
    else {
	if (solCount == 1)
	    *A = aSolutions[0];
	else
	    return FALSE;
    }

    *B = (b0 * *A + b1) * 2.0;
    *C = 1.0 - *A;
    *D = (d0 * *A + d1) * 2.0;
    *E = (e0 * *A + e1) * 2.0;
    *F = f0 * *A + f1;

    return *A * *C - IRIT_SQR(*B) > 0;		      /* Ellipse condition. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the implicit form of the given ellipse with some offset Offset.   M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F:   The six coefficients of the ellipse.	             M
*   Offset:		Offset amount.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdEllipse3Points, CagdEllipse4Points, CagdEllipse4Points, 	     M
*   CagdCreateConicCurve, CagdCreateConicCurve2			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEllipseOffset, ellipse                                               M
*****************************************************************************/
int CagdEllipseOffset(CagdRType *A,
		      CagdRType *B,
		      CagdRType *C,
		      CagdRType *D,
		      CagdRType *E,
		      CagdRType *F,
		      CagdRType Offset)
{
    IrtVecType Trans;
    CagdRType Desc,
	RotAngle = IRIT_APX_EQ(*B, 0.0) ? 0.0 : atan2(*B, *A - *C) * 0.5,
	A1 = ((*A + *C) + *B * sin(2 * RotAngle) +
	      (*A - *C) * cos(2 * RotAngle)) * 0.5,
	B1 = *B * cos(2 * RotAngle) - (*A - *C) * sin(2 * RotAngle),
	C1 = ((*A + *C) - *B * sin(2 * RotAngle) -
	      (*A - *C) * cos(2 * RotAngle)) * 0.5,
	D1 = *D * cos(RotAngle) + *E * sin(RotAngle),
	E1 = -*D * sin(RotAngle) + *E * cos(RotAngle),
	F1 = *F;

    if (!IRIT_APX_EQ(B1, 0.0) ||
	(IRIT_APX_EQ(A1, 0.0) && IRIT_APX_EQ(C1, 0.0))) {
	CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	return FALSE;
    }

    /* We now have conic: A1 x^2 + C1 y^2 + D1 x + E1 y + F1 = 0. */

    if ((Desc = A1 * C1) <= 0.0) {
	CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
        return FALSE;
    }

    /* Update the translation factors. */
    Trans[0] = -D1 / (2.0 * A1);
    Trans[1] = -E1 / (2.0 * C1);

    F1 -= (C1 * IRIT_SQR(D1) + A1 * IRIT_SQR(E1)) / (4 * A1 * C1);

    /* We now have conic in canonic form of: A1 x^2 + C1 y^2 + F1 = 0. */
    /* Update the major and minor axes with the desired offset.        */
    A1 = -F1 / IRIT_SQR(sqrt(-F1 / A1) + Offset);
    C1 = -F1 / IRIT_SQR(sqrt(-F1 / C1) + Offset);

    D1 = -2.0 * A1 * Trans[0];
    E1 = -2.0 * C1 * Trans[1];

    F1 += (C1 * IRIT_SQR(D1) + A1 * IRIT_SQR(E1)) / (4 * A1 * C1);

    /* And rotate back. */
    RotAngle = -RotAngle;
    *A = ((A1 + C1) + B1 * sin(2 * RotAngle) +
	  (A1 - C1) * cos(2 * RotAngle)) * 0.5;
    *B = B1 * cos(2 * RotAngle) - (A1 - C1) * sin(2 * RotAngle);
    *C = ((A1 + C1) - B1 * sin(2 * RotAngle) -
	  (A1 - C1) * cos(2 * RotAngle)) * 0.5;
    *D = D1 * cos(RotAngle) + E1 * sin(RotAngle);
    *E = -D1 * sin(RotAngle) + E1 * cos(RotAngle);
    *F = F1;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Transform given conic form A x^2 + B xy + C y^2 + D x + E y + F = 0,     M
* using Mat, in the XY plane.  Algorithm:				     M
*									     M
* 1. Convert the implicit conic to a matrix form as:			     V
*                 [ A  B/2  0  D/2] [x]					     V
*									     V
*    [x, y, 0, 1] [B/2  C   0  E/2] [y] = P^T M P = 0			     V
*									     V
*                 [ 0   0   1   0 ] [0]					     V
*									     V
*                 [D/2 E/2  0   F ] [1]					     V
*									     V
* 2. Compute N = Mat^{-1} the inverse of the desired transformation.	     V
*									     V
* 3. Compute K = N M N^T and decompose K back to A-F coefficients.	     V
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F:   The six coefficients of the conic. Updated in place. M
*   Mat:		Transformation matrix in the XY plane.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdQuadricMatTransform, CagdSrfTransfrom, CagdCrvTransform	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdConicMatTransform, implicit                                          M
*****************************************************************************/
int CagdConicMatTransform(CagdRType *A,
			  CagdRType *B,
			  CagdRType *C,
			  CagdRType *D,
			  CagdRType *E,
			  CagdRType *F,
			  CagdMType Mat)
{
    IrtHmgnMatType M, K, InvMat, TransposeMat;

    /* Construct the M matrix. */
    MatGenUnitMat(M);
    M[0][0] = *A;
    M[0][1] = M[1][0] = *B * 0.5;
    M[1][1] = *C;

    M[0][3] = M[3][0] = *D * 0.5;
    M[1][3] = M[3][1] = *E * 0.5;
    M[3][3] = *F;

    MatInverseMatrix(Mat, InvMat);

    /* Compute N M N^T. */
    MatMultTwo4by4(K, InvMat, M);
    MatTranspMatrix(InvMat, TransposeMat);
    MatMultTwo4by4(K, K, TransposeMat);

    *A = K[0][0];
    *B = K[0][1] * 2.0;
    *C = K[1][1];

    *D = K[0][3] * 2.0;
    *E = K[1][3] * 2.0;
    *F = K[3][3];

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Transform given quadric form                                             M
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0,      M
* using Mat.  Algorithm:						     M
*									     M
* 1. Convert the implicit quadric to a matrix form as:			     V
*                 [ A  D/2 E/2 G/2] [x]					     V
*									     V
*    [x, y, z, 1] [D/2  B  F/2 H/2] [y] = P^T M P = 0			     V
*									     V
*                 [E/2 F/2  C  I/2] [z]					     V
*									     V
*                 [G/2 H/2 I/2  J ] [1]					     V
*									     V
* 2. Compute N = Mat^{-1} the inverse of the desired transformation.	     V
*									     V
* 3. Compute K = N M N^T and decompose K back to A-J coefficients.	     V
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F, G, H, I, J:   The ten coefficients of the quadric.     M
8			Updated in place.			             M
*   Mat:		Transformation matrix.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdConicMatTransform, CagdSrfTransfrom, CagdCrvTransform	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdQuadricMatTransform, implicit                                        M
*****************************************************************************/
int CagdQuadricMatTransform(CagdRType *A,
			    CagdRType *B,
			    CagdRType *C,
			    CagdRType *D,
			    CagdRType *E,
			    CagdRType *F,
			    CagdRType *G,
			    CagdRType *H,
			    CagdRType *I,
			    CagdRType *J,
			    CagdMType Mat)
{
    IrtHmgnMatType M, K, InvMat, TransposeMat;

    /* Construct the M matrix. */
    M[0][0] = *A;
    M[1][1] = *B;
    M[2][2] = *C;

    M[0][1] = M[1][0] = *D * 0.5;
    M[0][2] = M[2][0] = *E * 0.5;
    M[1][2] = M[2][1] = *F * 0.5;

    M[0][3] = M[3][0] = *G * 0.5;
    M[1][3] = M[3][1] = *H * 0.5;
    M[2][3] = M[3][2] = *I * 0.5;

    M[3][3] = *J;

    MatInverseMatrix(Mat, InvMat);

    /* Compute N M N^T. */
    MatMultTwo4by4(K, InvMat, M);
    MatTranspMatrix(InvMat, TransposeMat);
    MatMultTwo4by4(K, K, TransposeMat);

    *A = K[0][0];
    *B = K[1][1];
    *C = K[2][2];

    *D = K[0][1] * 2.0;
    *E = K[0][2] * 2.0;
    *F = K[1][2] * 2.0;

    *G = K[0][3] * 2.0;
    *H = K[1][3] * 2.0;
    *I = K[2][3] * 2.0;

    *J = K[3][3];

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct rational quadric surface above (and below) the given conic     M
* in the XY plane of Z height.  The conic is given in the A-F coefficients   M
* as A x^2 + B xy + C y^2 + D x + E y + F = 0 and the quadric is returned    M
* in the A-J coefficients as:					             M
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0.      M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F, G, H, I, J:   Input - in A-F the conic and in J the    M
*	  Z height.  Output - the new 10 coefficients of the quadric.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCreateQuadricSrf, CagdEllipse3Points, CagdEllipse4Points,	     M
*   CagdEllipseOffset							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdConic2Quadric		                                             M
*****************************************************************************/
int CagdConic2Quadric(CagdRType *A,
		      CagdRType *B,
		      CagdRType *C,
		      CagdRType *D,
		      CagdRType *E,
		      CagdRType *F,
		      CagdRType *G,
		      CagdRType *H,
		      CagdRType *I,
		      CagdRType *J)
{
    CagdRType
	RotAngle = IRIT_APX_EQ(*B, 0.0) ? 0.0 : atan2(*B, *A - *C) * 0.5,
	A1 = ((*A + *C) + *B * sin(2 * RotAngle) +
	      (*A - *C) * cos(2 * RotAngle)) * 0.5,
	B1 = *B * cos(2 * RotAngle) - (*A - *C) * sin(2 * RotAngle),
	C1 = ((*A + *C) - *B * sin(2 * RotAngle) -
	      (*A - *C) * cos(2 * RotAngle)) * 0.5,
	D1 = *D * cos(RotAngle) + *E * sin(RotAngle),
	E1 = -*D * sin(RotAngle) + *E * cos(RotAngle),
	F1 = *F,
	ZHeight = *J;

    if (!IRIT_APX_EQ(B1, 0.0) ||
	(IRIT_APX_EQ(A1, 0.0) && IRIT_APX_EQ(C1, 0.0))) {
	CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	return FALSE;
    }

    F1 -= (C1 * IRIT_SQR(D1) + A1 * IRIT_SQR(E1)) / (4 * A1 * C1);

    /* Update the modified coefficients. */
    *J = *F;
    *I = 0;
    *H = *E;
    *G = *D;
    *E = *F = 0;
    *D = *B;
    *B = *C;
    *C = -F1 / IRIT_SQR(ZHeight);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct rational quadric surface out of the 9 coefficients of:         M
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0.      M
*   Based on:								     M
* Bezier Curves and Surface Patches on Quadrics, by Josef Hoschek,           M
* Mathematical methods in Computer aided Geometric Design II, Tom Lyche and  M
* Larry L. Schumaker (eds.), pp 331-342, 1992.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C, D, E, F, G, H, I, J:   The ten coefficients of the quadric.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A quadric surface representing the given form.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, CagdCrvCreateArc,	     M
*   BzrCrvCreateArc, CagdCreateConicCurve, CagdCreateConicCurve2,	     M
*   CagdConic2Quadric							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCreateQuadricSrf                                                     M
*****************************************************************************/
CagdSrfStruct *CagdCreateQuadricSrf(CagdRType A,
				    CagdRType B,
				    CagdRType C,
				    CagdRType D,
				    CagdRType E,
				    CagdRType F,
				    CagdRType G,
				    CagdRType H,
				    CagdRType I,
				    CagdRType J)
{
    CagdBType
	SwitchXY = FALSE,
	SwitchXZ = FALSE,
	SwitchYZ = FALSE;
    int i;
    CagdRType **Points, N[9], b[3];
    IrtVecType Trans;
    IrtHmgnMatType Mat, Mat2, M, U, d, V;
    CagdSrfStruct *PwrSrf, *TSrf,
	*BzrSrf = NULL;

    /* Computes the translation factors.  Substituting (x-X0) for x (and     */
    /* same for y and z), one gets the following constraints for the linear  */
    /* terms to vanish (G, H, I in the original general quadric):	     */
    /* [2A   D   E] [X0] = [G]						     */
    /* [ D  2B   F] [Y0] = [H]						     */
    /* [ E   F  2C] [Z0] = [I]						     */
    /* The solution for X0, Y0, Z0 zeros these linear terms and yields new   */
    /* constant J term: J + A X0^2 + B Y0^2 + C Z0^2 +			     */
    /*			D X0 Y0 + E X0 Z0 + F Y0 Z0 - G X0 - H Y0 - I Z0.    */
    N[0] = 2 * A;     N[3] = D;      	N[6] = E;
    N[1] = D;	      N[4] = 2 * B;	N[7] = F; 
    N[2] = E;	      N[5] = F;		N[8] = 2 * C;
    b[0] = G;
    b[1] = H;
    b[2] = I;
    IritQRUnderdetermined(N, NULL, NULL, 3, 3);
    IritQRUnderdetermined(NULL, Trans, b, 3, 3);

    /* Find the translational factors. */
    J += A * IRIT_SQR(Trans[0]) +
	 B * IRIT_SQR(Trans[1]) +
	 C * IRIT_SQR(Trans[2]) +
	 D * Trans[0] * Trans[1] +
	 E * Trans[0] * Trans[2] +
	 F * Trans[1] * Trans[2] -
	 G * Trans[0] -
	 H * Trans[1] -
	 I * Trans[2];

    G = H = I = 0.0;

    /* Compute the rotation factors.  Now we have a quadrics of the form:    */
    /* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + J = 0.		     */
    /* Writing the quadric form as:					     */
    /* [x y z 1] [A    D/2  E/2  G/2] [x] = [x y z 1] M [x] = 0		     */
    /*		 [D/2  B    F/2  H/2] [y]		[y]		     */
    /*		 [E/2  F/2  C    I/2] [z]		[z]		     */
    /*		 [G/2  H/2  I/2  J  ] [1]		[1]		     */
    /* we seek a rotation of M that make it diagonal.  Toward this end we    */
    /* employ SVD to decompose M into U N V, N diagonal and U and V are two  */
    /* rotational matrices that are also transpose (==inverse) of each other.*/
    M[0][0] = A;
    M[1][1] = B;
    M[2][2] = C;
    M[3][3] = J;

    M[0][1] = M[1][0] = D * 0.5;
    M[0][2] = M[2][0] = E * 0.5;
    M[1][2] = M[2][1] = F * 0.5;

    M[0][3] = M[3][0] = G * 0.5;
    M[1][3] = M[3][1] = H * 0.5;
    M[2][3] = M[3][2] = I * 0.5;

    JacobiMatrixDiag4x4(M, U, d, V);

    A = d[0][0];
    B = d[1][1];
    C = d[2][2];
    J = d[3][3];

    /* We now have quadric: A x^2 + B y^2 + C z^2 + J = 0. */
    if (J == 0.0) {
        if (A < 0.0) {
	    A = -A;
	    B = -B;
	    C = -C;
	}
    }
    else {
        if (J > 0.0) {			    /* Make sure J is not positive. */
	    A = -A;
	    B = -B;
	    C = -C;
	    J = -J;
	}

        if (A * J >= 0.0) {
	    if (B * J >= 0.0) {
	        if (C * J >= 0.0) {
		    CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
		    return NULL;
		}
		else {
		    SwitchXZ = TRUE;
		    IRIT_SWAP(CagdRType, A, C);
		}
	    }
	    else {
	        SwitchXY = TRUE;
		IRIT_SWAP(CagdRType, A, B);
	    }
	}
    }

    /* Now J is not positive and A is not negative. */

    if (B >= 0.0 && C >= 0.0) {
        /* Quadrics is ellipsoid A x^2 + B y^2 + C z^2 + J w^2 = 0,          */
	/*				    A >= 0, B >= 0, C >= 0, J <= 0.  */
        IRIT_STATIC_DATA CagdVType
	    Center = { 0.0, 0.0, 0.0 };
	CagdPType R;

	BzrSrf = CagdPrimSphereSrf(Center, 1.0, TRUE);

	R[0] = sqrt(-J / A);
	R[1] = sqrt(-J / B);
	R[2] = sqrt(-J / C);

	/* Scale the circular shape into an ellipse. */
	MatGenMatScale(R[0], R[1], R[2], Mat);
	TSrf = CagdSrfMatTransform(BzrSrf, Mat);
	CagdSrfFree(BzrSrf);
	BzrSrf = TSrf;
    }
    else if (B <= 0.0 && C <= 0.0) {
        /* Quadrics is now A x^2 + B y^2 + C z^2 + J w^2 = 0,	             */
	/*				   A >= 0, B <= 0, C <= 0, J <= 0.   */
	/* The above hyperboloid of two sheets can be written as,	     */
	/*              x = (1/w) * (a^2 + b^2 + c^2) / sqrt(A),      	     */
	/*              y = (1/w) * 2ab / sqrt(B),			     */
	/*              z = (1/w) * 2ac / sqrt(C),			     */
	/* and w equals w = (a^2 - b^2 - c^2) / -sqrt(-J).     		     */
	/* The only (large) remaining question is how to parametrize a, b, c.*/
	/* For no better selection, we select a = u, b = 1 - v, c = v, or    */
	/*              x = (1/w) * (u^2 + 1 - 2 * v + 2 * v^2) / sqrt(A),   */
	/*              y = (1/w) * (2 * u - 2 * uv) / sqrt(B),		     */
	/*              z = (1/w) * (2 * u * v) / sqrt(C),		     */
	/* and w equals w = (u^2 - 1 + 2 * v - 2 * v^2) / sqrt(-J)           */

	A = QUADRIC_INVERT(A);
	B = QUADRIC_INVERT(-B);
	C = QUADRIC_INVERT(-C);
	J = -QUADRIC_INVERT(-J);

	PwrSrf = CagdSrfNew(CAGD_SPOWER_TYPE, CAGD_PT_P3_TYPE, 3, 3);
	Points = PwrSrf -> Points;

	for (i = 0; i <= 3; i++)
	    IRIT_ZAP_MEM(Points[i], 9 * sizeof(CagdRType));

	/* Coefficients of the biquadratic surface are orders as follows     */
	/*    0) 1          1)  u         2) u^2			     */
	/*    3) v          4)  u * v     5) u^2 * v			     */
	/*    6) v^2        7)  u * v^2   8) u^2 * v^2			     */
	Points[0][0] = -J;				      /* The w term. */
	Points[0][2] = J;
	Points[0][3] = 2.0 * J;
	Points[0][6] = -2.0 * J;
	Points[1][0] = A;				      /* The x term. */
	Points[1][2] = A;
	Points[1][3] = -2.0 * A;
	Points[1][6] = 2.0 * A;
	Points[2][1] = 2.0 * B;				      /* The y term. */
	Points[2][4] = -2.0 * B;
	Points[3][4] = 2.0 * C;				      /* The z term. */

	BzrSrf = CagdCnvrtPwr2BzrSrf(PwrSrf);
	CagdSrfFree(PwrSrf);
    }
    else if ((B >= 0.0 && C <= 0.0) || (B <= 0.0 && C >= 0.0)) {
        if (B <= 0.0 && C >= 0.0) {
	    /* Exchange the role of the B and C coefficients. */
	    IRIT_SWAP(CagdRType, B, C);
	    SwitchYZ = TRUE;
	}

	/* Quadrics is now A x^2 + B y^2 + C z^2 + J w^2 = 0,	             */
	/*				    A >= 0, B >= 0, C <= 0, J <= 0.  */
	/* The above hyperboloid of one sheet can be written as,	     */
	/*              x = (1/w) * (a^2 - b^2 + c^2) / sqrt(A),      	     */
	/*              y = (1/w) * 2ab / sqrt(B),			     */
	/*              z = (1/w) * 2ac / -sqrt(-C),			     */
	/* and w equals w = (a^2 + b^2 - c^2) / -sqrt(-J).     		     */
	/* The only (large) remaining question is how to parametrize a, b, c.*/
	/* For no better selection, we select a = u, b = 1 - v, c = v, or    */
	/*              x = (1/w) * (u^2 - 1 + 2 * v) / sqrt(A),	     */
	/*              y = (1/w) * (2 * u - 2 * uv) / sqrt(B),  	     */
	/*              z = (1/w) * (2 * u * v) / -sqrt(C),		     */
	/* and w equals w = (1 + u^2 - 2 * v) / -sqrt(-J).	             */

	A = QUADRIC_INVERT(A);
	B = QUADRIC_INVERT(B);
	C = -QUADRIC_INVERT(-C);
	J = -QUADRIC_INVERT(-J);

	PwrSrf = CagdSrfNew(CAGD_SPOWER_TYPE, CAGD_PT_P3_TYPE, 3, 3);
	Points = PwrSrf -> Points;

	for (i = 0; i <= 3; i++)
	    IRIT_ZAP_MEM(Points[i], 9 * sizeof(CagdRType));

	/* Coefficients of the biquadratic surface are orders as follows     */
	/*    0) 1          1)  u         2) u^2			     */
	/*    3) v          4)  u * v     5) u^2 * v			     */
	/*    6) v^2        7)  u * v^2   8) u^2 * v^2			     */
	Points[0][0] = J;				      /* The w term. */
	Points[0][2] = J;
	Points[0][3] = -2.0 * J;
	Points[1][0] = -A;				      /* The x term. */
	Points[1][2] = A;
	Points[1][3] = 2.0 * A;
	Points[2][1] = 2.0 * B;				      /* The y term. */
	Points[2][4] = -2.0 * B;
	Points[3][4] = 2.0 * C;				      /* The z term. */
    
	BzrSrf = CagdCnvrtPwr2BzrSrf(PwrSrf);
	CagdSrfFree(PwrSrf);
    }
    else {
        CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CONIC_COEF);
	return NULL;
    }

    if (SwitchYZ) {
	MatGenMatRotX1(M_PI * 0.5, Mat);
	MatGenMatScale(1, 1, -1, Mat2);
	MatMultTwo4by4(Mat, Mat2, Mat);

	TSrf = CagdSrfMatTransform(BzrSrf, Mat);
	CagdSrfFree(BzrSrf);
	BzrSrf = TSrf;
    }
    if (SwitchXZ) {
	MatGenMatRotY1(-M_PI * 0.5, Mat);
	MatGenMatScale(1, 1, -1, Mat2);
	MatMultTwo4by4(Mat, Mat2, Mat);

	TSrf = CagdSrfMatTransform(BzrSrf, Mat);
	CagdSrfFree(BzrSrf);
	BzrSrf = TSrf;
    }
    if (SwitchXY) {
	MatGenMatRotZ1(M_PI * 0.5, Mat);
	MatGenMatScale(1, -1, 1, Mat2);
	MatMultTwo4by4(Mat, Mat2, Mat);

	TSrf = CagdSrfMatTransform(BzrSrf, Mat);
	CagdSrfFree(BzrSrf);
	BzrSrf = TSrf;
    }

    if (BzrSrf != NULL) {
	TSrf = CagdSrfMatTransform(BzrSrf, V);
	CagdSrfFree(BzrSrf);
	BzrSrf = TSrf;

        IRIT_PT_SCALE(Trans, -1.0);
	CagdSrfTransform(BzrSrf, Trans, 1.0);

	CAGD_SET_GEOM_TYPE(BzrSrf, CAGD_GEOM_CONIC_SEC);
    }

    return BzrSrf;
}
