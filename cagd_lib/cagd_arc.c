/******************************************************************************
* Cagd_Arc.c - Curve representation of arcs and circles			      *
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
#include "cagd_loc.h"

#define UNIT_CIRCLE_ORDER	3	        /* Quadratic rational curve. */
#define UNIT_CIRCLE_LENGTH	9   /* Ctl pts in rational quadratic circle. */
#define UNIT_PCIRCLE_ORDER	4		  /* Cubic polynomial curve. */
#define UNIT_PCIRCLE_LENGTH	10      /* Ctl pts in integral cubic circle. */

#define CAGD_SPIRAL_ORDER	4
#define CAGD_HELIX_ORDER	4
#define CAGD_SINE_ORDER		4

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an arc at the specified position as a rational quadratic Bezier    M
* curve.								     M
*   The arc is assumed to be less than 180 degrees from Start to End in the  M
* shorter path as arc where Center as arc center.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Start:       Point of beginning of arc.                                  M
*   Center:      Point of arc.                                               M
*   End:         Point of end of arc. 	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A rational quadratic Bezier curve representing the arc. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreatePCircle,         M
*   CagdCreateConicCurve, CagdCrvCreateArc, BspCrvCreateUnitPCircle,	     M
*   CagdCrvCreateArcCCW, CagdCrvCreateArcCW				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvCreateArc, circle, arc                                             M
*****************************************************************************/
CagdCrvStruct *BzrCrvCreateArc(const CagdPtStruct *Start,
			       const CagdPtStruct *Center,
			       const CagdPtStruct *End)
{
    int i;
    CagdCrvStruct
	*Arc = BzrCrvNew(3, CAGD_PT_P3_TYPE);
    CagdRType Len, CosAlpha, Radius,
	**Points = Arc -> Points;
    CagdVecStruct V1, V2, V;

    /* Copy first point. */
    Points[X][0] = Start -> Pt[0];
    Points[Y][0] = Start -> Pt[1];
    Points[Z][0] = Start -> Pt[2];
    Points[W][0] = 1.0;

    /* Copy last point. */
    Points[X][2] = End -> Pt[0];
    Points[Y][2] = End -> Pt[1];
    Points[Z][2] = End -> Pt[2];
    Points[W][2] = 1.0;

    /* Compute position of middle point. */
    Len = 0.0;
    for (i = 0; i < 3; i++) {
	V1.Vec[i] = Start -> Pt[i] - Center -> Pt[i];
	V2.Vec[i] = End -> Pt[i] - Center -> Pt[i];
	V.Vec[i] = V1.Vec[i] + V2.Vec[i];
	Len += IRIT_SQR(V.Vec[i]);
    }

    if (IRIT_APX_UEQ(Len, 0.0)) {
	CagdCrvFree(Arc);
	CAGD_FATAL_ERROR(CAGD_ERR_180_ARC);
	return NULL;
    }
    else
	Len = sqrt(Len);

    for (i = 0; i < 3; i++)
	V.Vec[i] /= Len;

    /* Compute cosine alpha (where alpha is the angle between V and V1. */
    Radius = sqrt(IRIT_DOT_PROD(V1.Vec, V1.Vec));
    CosAlpha = IRIT_DOT_PROD(V1.Vec, V.Vec) / Radius;

    CAGD_DIV_VECTOR(V, CosAlpha);
    CAGD_MULT_VECTOR(V, Radius);

    /* And finally fill in the middle point with CosAlpha as the Weight. */
    Points[X][1] = (Center -> Pt[0] + V.Vec[0]) * CosAlpha;
    Points[Y][1] = (Center -> Pt[1] + V.Vec[1]) * CosAlpha;
    Points[Z][1] = (Center -> Pt[2] + V.Vec[2]) * CosAlpha;
    Points[W][1] = CosAlpha;

    CAGD_SET_GEOM_TYPE(Arc, CAGD_GEOM_CIRCULAR);

    return Arc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an arc at the specified position as a rational quadratic Bspline   M
* curve, with upto two Bezier pieces.					     M
*   The arc is defined from StartAngle to EndAngle counter clockwise, and is M
* assumed to be less than 360 degrees from Start to End.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:      Point of arc.                                               M
*   Radius:      Of arc.			                             M
*   StartAngle:  Starting angle of arc, in degrees.                          M
*   EndAngle:    End angle of arc, in degrees.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A rational quadratic Bezier (or Bspline) curve          M
*		representing the arc.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreatePCircle,         M
*   CagdCreateConicCurve, BzrCrvCreateArc, BspCrvCreateUnitPCircle,	     M
*   CagdCrvCreateArcCCW, CagdCrvCreateArcCW				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCreateArc, circle, arc                                            M
*****************************************************************************/
CagdCrvStruct *CagdCrvCreateArc(const CagdPtStruct *Center,
				CagdRType Radius,
				CagdRType StartAngle,
				CagdRType EndAngle)
{
    CagdPtStruct Start, End;

    if (EndAngle < StartAngle)
	EndAngle += 360.0;

    Start.Pt[0] = Center -> Pt[0] + Radius * cos(IRIT_DEG2RAD(StartAngle));
    Start.Pt[1] = Center -> Pt[1] + Radius * sin(IRIT_DEG2RAD(StartAngle));
    Start.Pt[2] = Center -> Pt[2];

    End.Pt[0] = Center -> Pt[0] + Radius * cos(IRIT_DEG2RAD(EndAngle));
    End.Pt[1] = Center -> Pt[1] + Radius * sin(IRIT_DEG2RAD(EndAngle));
    End.Pt[2] = Center -> Pt[2];

    if (EndAngle - StartAngle < 160.0) {   /* Lets not get too extreme here. */
	return BzrCrvCreateArc(&Start, Center, &End);
    }
    else {
	CagdCrvStruct *Arc1, *Arc2, *Arc;
	CagdPtStruct Middle;
	CagdRType
	    MidAngle = (StartAngle + EndAngle) * 0.5;

	Middle.Pt[0] = Center -> Pt[0] + Radius * cos(IRIT_DEG2RAD(MidAngle));
	Middle.Pt[1] = Center -> Pt[1] + Radius * sin(IRIT_DEG2RAD(MidAngle));
	Middle.Pt[2] = Center -> Pt[2];

	Arc1 = BzrCrvCreateArc(&Start, Center, &Middle);
	Arc2 = BzrCrvCreateArc(&Middle, Center, &End);
	Arc = CagdMergeCrvCrv(Arc1, Arc2, FALSE);
	CagdCrvFree(Arc1);
	CagdCrvFree(Arc2);

	CAGD_SET_GEOM_TYPE(Arc, CAGD_GEOM_CIRCULAR);

	return Arc;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a counter clockwise arc at the specified position as a rational    M
* quadratic Bspline curve, with up to two Bezier pieces.		     M
*   The arc is defined from Start to End, and is assumed to be less than or  M
* eual to 360 degrees (full circle, if Start == End).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Start:     Starting position of arc. 	                             M
*   Center:    Center point of arc.                                          M
*   End:       End position of arc.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A rational quadratic Bezier (or Bspline) curve          M
*		representing the arc, or NULL if error.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreatePCircle,         M
*   CagdCreateConicCurve, BzrCrvCreateArc, BspCrvCreateUnitPCircle,	     M
*   CagdCrvCreateArc, CagdCrvCreateArcCW				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCreateArcCCW, circle, arc                                         M
*****************************************************************************/
CagdCrvStruct *CagdCrvCreateArcCCW(const CagdPtStruct *Start,
				   const CagdPtStruct *Center,
				   const CagdPtStruct *End)
{
    CagdRType StartAngle, EndAngle;

    if ((Start -> Pt[0] == Center -> Pt[0] &&
	 Start -> Pt[1] == Center -> Pt[1]) ||
	(End -> Pt[0] == Center -> Pt[0] &&
	 End -> Pt[1] == Center -> Pt[1]))
	 return NULL;

    StartAngle = atan2(Start -> Pt[1] - Center -> Pt[1],
		       Start -> Pt[0] - Center -> Pt[0]),
    EndAngle = atan2(End -> Pt[1] - Center -> Pt[1],
		     End -> Pt[0] - Center -> Pt[0]);

    StartAngle = IRIT_RAD2DEG(StartAngle);
    EndAngle = IRIT_RAD2DEG(EndAngle);
    if (EndAngle <= StartAngle)
	EndAngle += 360.0;

    if (EndAngle - StartAngle < 160.0) {  /* Lets not get too extremes here. */
	return BzrCrvCreateArc(Start, Center, End);
    }
    else {
	CagdCrvStruct *Arc1, *Arc2, *Arc;
	CagdPtStruct Middle;
	CagdRType
	    MidAngle = (StartAngle + EndAngle) * 0.5,
	    Radius = IRIT_PT_PT_DIST(Start -> Pt, Center -> Pt);
    
	Middle.Pt[0] = Center -> Pt[0] + Radius * cos(IRIT_DEG2RAD(MidAngle));
	Middle.Pt[1] = Center -> Pt[1] + Radius * sin(IRIT_DEG2RAD(MidAngle));
	Middle.Pt[2] = Center -> Pt[2];

	Arc1 = CagdCrvCreateArcCCW(Start, Center, &Middle);
	Arc2 = CagdCrvCreateArcCCW(&Middle, Center, End);
	Arc = CagdMergeCrvCrv(Arc1, Arc2, FALSE);
	CagdCrvFree(Arc1);
	CagdCrvFree(Arc2);

	CAGD_SET_GEOM_TYPE(Arc, CAGD_GEOM_CIRCULAR);

	return Arc;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a counter clockwise arc at the specified position as a rational    M
* quadratic Bspline curve, with upto two Bezier pieces.			     M
*   The arc is defined from Start to End, and is assumed to be less than or  M
* eual to 360 degrees (full circle, if Start == End).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Start:     Starting position of arc. 	                             M
*   Center:    Center point of arc.                                          M
*   End:       End position of arc.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A rational quadratic Bezier (or Bspline) curve          M
*		representing the arc, or NULL if error.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreatePCircle,         M
*   CagdCreateConicCurve, BzrCrvCreateArc, BspCrvCreateUnitPCircle,	     M
*   CagdCrvCreateArc, CagdCrvCreateArcCCW				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCreateArcCW, circle, arc                                          M
*****************************************************************************/
CagdCrvStruct *CagdCrvCreateArcCW(const CagdPtStruct *Start,
				  const CagdPtStruct *Center,
				  const CagdPtStruct *End)
{
    CagdCrvStruct *Arc,
	*RevArc = CagdCrvCreateArcCCW(End, Center, Start);

    if (RevArc == NULL)
	return NULL;

    Arc = CagdCrvReverse(RevArc);
    CagdCrvFree(RevArc);

    return Arc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a circle at the specified position as a rational quadratic Bspline M
* curve. 								     M
*   Constructs a unit circle as 4 90 degrees arcs of rational quadratic      M
* Bezier segments using a predefined constants.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A rational quadratic bsplinecurve representing a unit   M
*                    circle.                                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreatePCircle, BspCrvCreateUnitPCircle,        M
*   CagdCreateConicCurve, CagdCrvCreateArc, BzrCrvCreateArc		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateUnitCircle, circle                                           M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateUnitCircle(void)
{    
    IRIT_STATIC_DATA CagdRType
	UnitCircleKnots[UNIT_CIRCLE_ORDER + UNIT_CIRCLE_LENGTH] =
					{ 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4 },
	UnitCircleX[UNIT_CIRCLE_LENGTH] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 },
	UnitCircleY[UNIT_CIRCLE_LENGTH] = { 0, 1, 1, 1, 0, -1, -1, -1, 0 };

    int i;
    CagdRType Weight,
        W45 = sin(M_PI / 4.0);
    CagdCrvStruct
	*Circle = BspCrvNew(UNIT_CIRCLE_LENGTH, UNIT_CIRCLE_ORDER,
							      CAGD_PT_P3_TYPE);
    CagdRType
	**Points = Circle -> Points;

    CAGD_GEN_COPY(Circle -> KnotVector, UnitCircleKnots,
		  sizeof(CagdRType) * (UNIT_CIRCLE_LENGTH + UNIT_CIRCLE_ORDER));

    for (i = 0; i < UNIT_CIRCLE_LENGTH; i++) {
	Weight = Points[W][i] = i % 2 ? W45: 1.0;
	Points[X][i] = UnitCircleX[i] * Weight;
	Points[Y][i] = UnitCircleY[i] * Weight;
	Points[Z][i] = 0.0;
    }

    CAGD_SET_GEOM_TYPE(Circle, CAGD_GEOM_CIRCULAR);

    return Circle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a circle at the specified position as a rational quadratic Bspline M
* curve. Circle is always paralell to the XY plane.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   Of circle to be created.                                       M
*   Radius:   Of circle to be created.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A circle centered at Center and radius Radius that is M
*                      parallel to the XY plane represented as a rational    M
*                      quadratic Bspline curve.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateUnitCircle, BspCrvCreatePCircle, BspCrvCreateUnitPCircle,    M
*   CagdCreateConicCurve, CagdCrvCreateArc, BzrCrvCreateArc		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateCircle, circle                                               M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateCircle(const CagdPtStruct *Center, CagdRType Radius)
{
    CagdCrvStruct
	*Circle = BspCrvCreateUnitCircle();

    /* Do it in two stages: 1. scale, 2. translate */
    CagdCrvTransform(Circle, NULL, Radius);
    CagdCrvTransform(Circle, Center -> Pt, 1.0);

    CAGD_SET_GEOM_TYPE(Circle, CAGD_GEOM_CIRCULAR);

    return Circle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates a unit circle as a cubic polynomial Bspline curve.	     M
*   Construct a circle as four 90 degrees arcs of polynomial cubic Bezier    M
* segments using predefined constants.					     M
*   See Faux & Pratt "Computational Geometry for Design and Manufacturing"   M
* for a polynomial approximation to a circle.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None				                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A cubic polynomial Bspline curve approximating a unit   M
*                    circle                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreatePCircle,         M
*   CagdCreateConicCurve, CagdCrvCreateArc, BzrCrvCreateArc,		     M
*   BspCrvCreateApproxSpiral						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateUnitPCircle, circle                                          M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateUnitPCircle(void)
{
    IRIT_STATIC_DATA CagdRType
	UnitPCircleKnots[UNIT_PCIRCLE_ORDER + UNIT_PCIRCLE_LENGTH] =
				{ 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4 };
    CagdCrvStruct
	*PCircle = BspCrvNew(UNIT_PCIRCLE_LENGTH, UNIT_PCIRCLE_ORDER,
			     CAGD_PT_E3_TYPE);
    CagdRType CosAngle1, SinAngle1, CosAngle2, SinAngle2,
	**Points = PCircle -> Points;

    CAGD_GEN_COPY(PCircle -> KnotVector, UnitPCircleKnots,
		  sizeof(CagdRType) *
		                  (UNIT_PCIRCLE_LENGTH + UNIT_PCIRCLE_ORDER));

    /* The Z components are identical in all circle. */
    IRIT_ZAP_MEM(Points[Z], sizeof(CagdRType) * UNIT_PCIRCLE_LENGTH);

    Points[X][0] = Points[X][UNIT_PCIRCLE_LENGTH - 1] = 1.0;
    Points[Y][0] = Points[Y][UNIT_PCIRCLE_LENGTH - 1] = 0.0;
    
    CosAngle1 = 4.0 * (sqrt(2.0) - 1.0) / 3.0;
    SinAngle1 = 1.0;

    CosAngle2 = SinAngle1;
    SinAngle2 = CosAngle1;

    Points[X][1] =  SinAngle1;
    Points[Y][1] =  CosAngle1;
    Points[X][2] =  SinAngle2;
    Points[Y][2] =  CosAngle2;
    Points[X][3] = -SinAngle2;
    Points[Y][3] =  CosAngle2;
    Points[X][4] = -SinAngle1;
    Points[Y][4] =  CosAngle1;
    Points[X][5] = -SinAngle1;
    Points[Y][5] = -CosAngle1;
    Points[X][6] = -SinAngle2;
    Points[Y][6] = -CosAngle2;
    Points[X][7] =  SinAngle2;
    Points[Y][7] = -CosAngle2;
    Points[X][8] =  SinAngle1;
    Points[Y][8] = -CosAngle1;

    CAGD_SET_GEOM_TYPE(PCircle, CAGD_GEOM_CIRCULAR);

    return PCircle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates a circle as a cubic polynomial Bspline curve at the specified M
* position and radius.							     M
*   Construct the circle as four 90 degrees arcs of polynomial cubic Bezier  M
* segments using predefined constants.					     M
*   See Faux & Pratt "Computational Geometry for Design and Manufacturing"   M
* for a polynomial approximation to a circle.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   Of circle to be created.                                       M
*   Radius:   Of circle to be created.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A circle approximation centered at Center and radius  M
*                      Radius that is parallel to the XY plane represented   M
*                      as a polynomial cubic Bspline curve.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateUnitCircle, BspCrvCreateUnitPCircle,     M
*   CagdCreateConicCurve, CagdCrvCreateArc, BzrCrvCreateArc		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreatePCircle, circle                                              M
*****************************************************************************/
CagdCrvStruct *BspCrvCreatePCircle(const CagdPtStruct *Center,
				   CagdRType Radius)
{
    CagdCrvStruct
	*Circle = BspCrvCreateUnitPCircle();

    /* Do it in two stages: 1. scale, 2. translate */
    CagdCrvTransform(Circle, NULL, Radius);
    CagdCrvTransform(Circle, Center -> Pt, 1.0);

    CAGD_SET_GEOM_TYPE(Circle, CAGD_GEOM_CIRCULAR);

    return Circle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an approximated spiral curve (not rational!)                  M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfLoops:    Number of loops in the spiral - can be fractional.        M
*   Pitch:         Essentially the size of the spiral.  A Pitch of one will  M
*		   construct a roughly size-one spiral curve.		     M
*   Sampling:      Number of samples to compute on the spiral.  Should be    M
*		   several hundreds for a reasonable result.		     M
*   CtlPtsPerLoop: Number of control points to use per loop.  Use at least 5 M
*                  for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A spiral Bspline curve approximation.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateApproxSine, BspCrvCreateApproxHelix      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateApproxSpiral                                                 M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateApproxSpiral(CagdRType NumOfLoops,
					CagdRType Pitch,
					int Sampling,
					int CtlPtsPerLoop)
{
    CagdRType t,
	TotalAngle = 2.0 * M_PI * NumOfLoops,
	Step = TotalAngle / Sampling,
	Rad = Pitch / (2.0 * M_PI * (1.0 + NumOfLoops));
    CagdCtlPtStruct *CtlPt,
	*CtlPtList = NULL;
    CagdCrvStruct *Crv;

    for (t = 0.0; t <= TotalAngle + Step * 0.5; t += Step) {
	CtlPt = CagdCtlPtNew(CAGD_PT_E2_TYPE);

	CtlPt -> Coords[1] = cos(t) * t * Rad;
	CtlPt -> Coords[2] = sin(t) * t * Rad;

	IRIT_LIST_PUSH(CtlPt, CtlPtList);
    }
    CtlPtList = CagdListReverse(CtlPtList);

    Crv = BspCrvInterpPts2(CtlPtList, CAGD_SPIRAL_ORDER,
			   CAGD_SPIRAL_ORDER +
			       ((int) (CtlPtsPerLoop * NumOfLoops)),
			   CAGD_UNIFORM_PARAM, FALSE);

    CagdCtlPtFreeList(CtlPtList);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an approximated polynomial helix curve, along the +Z axis.    M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfLoops:    Number of loops in the helix - can be fractional.         M
*   Pitch:         Essentially the size of the helix.  A Pitch of one will   M
*		   step one unit in Z for one full circle.		     M
*   Radius:        Radius of helix.  If radius is negative, the radius will  M
*		   change monotonically from zero to abs(Radius) at the end. M
*   Sampling:      Number of samples to compute on the helix.  Should be     M
*		   several hundreds for a reasonable result.		     M
*   CtlPtsPerLoop: Number of control points to use per loop.  Use at least 5 M
*                  for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A helix Bspline curve approximation.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateApproxSine, BspCrvCreateApproxSpiral     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateApproxHelix                                                  M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateApproxHelix(CagdRType NumOfLoops,
				       CagdRType Pitch,
				       CagdRType Radius,
				       int Sampling,
				       int CtlPtsPerLoop)
{
    CagdRType t,
	TotalAngle = 2.0 * M_PI * NumOfLoops,
	Step = TotalAngle / Sampling,
	Rad = IRIT_FABS(Radius);
    CagdCtlPtStruct *CtlPt,
	*CtlPtList = NULL;
    CagdCrvStruct *Crv;

    for (t = 0.0; t <= TotalAngle + Step * 0.5; t += Step) {
	CtlPt = CagdCtlPtNew(CAGD_PT_E3_TYPE);

	Rad = Radius > 0 ? Radius : -Radius * t / TotalAngle;

	CtlPt -> Coords[1] = cos(t) * Rad;
	CtlPt -> Coords[2] = sin(t) * Rad;
	CtlPt -> Coords[3] = Pitch * t / 2.0 * M_PI;

	IRIT_LIST_PUSH(CtlPt, CtlPtList);
    }
    CtlPtList = CagdListReverse(CtlPtList);

    Crv = BspCrvInterpPts2(CtlPtList, CAGD_HELIX_ORDER,
			   CAGD_HELIX_ORDER +
			       ((int) (CtlPtsPerLoop * NumOfLoops)),
			   CAGD_UNIFORM_PARAM, FALSE);

    CagdCtlPtFreeList(CtlPtList);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an approximated polynomial sine curve.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfCycles:    Number of cycles in the sine - can be fractional.        M
*   Sampling:       Number of samples to compute on the sine.  Should be     M
*		    several hundreds for a reasonable result.		     M
*   CtlPtsPerCycle: Number of control points to use per cycle.  Use at least M
*                   5 for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A sine wave Bspline curve approximation.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateCircle, BspCrvCreateApproxSine, BspCrvCreateApproxHelix      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvCreateApproxSine                                                   M
*****************************************************************************/
CagdCrvStruct *BspCrvCreateApproxSine(CagdRType NumOfCycles,
				      int Sampling,
				      int CtlPtsPerCycle)
{
    CagdRType t,
	TotalAngle = 2.0 * M_PI * NumOfCycles,
	Step = TotalAngle / Sampling;
    CagdCtlPtStruct *CtlPt,
	*CtlPtList = NULL;
    CagdCrvStruct *Crv;

    for (t = 0.0; t <= TotalAngle + Step * 0.5; t += Step) {
	CtlPt = CagdCtlPtNew(CAGD_PT_E2_TYPE);

	CtlPt -> Coords[1] = t;
	CtlPt -> Coords[2] = sin(t);

	IRIT_LIST_PUSH(CtlPt, CtlPtList);
    }
    CtlPtList = CagdListReverse(CtlPtList);

    Crv = BspCrvInterpPts2(CtlPtList, CAGD_SINE_ORDER,
			   CAGD_SINE_ORDER +
			       ((int) (CtlPtsPerCycle * NumOfCycles)),
			   CAGD_UNIFORM_PARAM, FALSE);

    CagdCtlPtFreeList(CtlPtList);

    return Crv;
}
