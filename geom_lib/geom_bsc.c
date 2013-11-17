/******************************************************************************
* Geom_Bsc.c - basic geometry.						      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"

#define GM_COLLINEAR_EPS		1e-10
#define GM_QUARTIC_SOLVE_EPS		1e-10
#define GM_SOLVER_MAX_NEWTON_ITER	20

IRIT_STATIC_DATA IrtRType
    GMBasicEps = IRIT_UEPS,
    GMBasicEpsSqr = IRIT_SQR(IRIT_UEPS),
    GMBasicColinEps = GM_COLLINEAR_EPS;

#ifdef DEBUG
/* Print information on entry and exit of routines. */
IRIT_SET_DEBUG_PARAMETER(_DebugGMEntryExit, FALSE);
#endif /* DEBUG */

#define GM_QUART(x)  (IRIT_SQR(IRIT_SQR((IrtRType) (x))))
#define GM_SQRT3(x)  (pow(IRIT_FABS((IrtRType) (x)), 1.0 / 3.0) * IRIT_SIGN(x))

static int GMPointRayRelation(const IrtPtType Pt,
			      const IrtPtType PtRay,
			      int Axes);
static IrtRType GMSolveCubicApplyNewton(IrtRType Root,
					IrtRType A,
					IrtRType B,
					IrtRType C);
static IrtRType GMSolveQuarticApplyNewton(IrtRType Root,
					  IrtRType A,
					  IrtRType B,
					  IrtRType C,
					  IrtRType D);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the epsilon to use in basic geometry processing.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Eps:   New epsilon to use.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Old epsilon value.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBasicSetEps                                                            M
*****************************************************************************/
IrtRType GMBasicSetEps(IrtRType Eps)
{
    IrtRType
        OldVal = GMBasicEps;

    GMBasicColinEps = GM_COLLINEAR_EPS * Eps / IRIT_UEPS;
    GMBasicEps = Eps;
    GMBasicEpsSqr = IRIT_SQR(GMBasicEps);

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to copy one vector to another:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vdst:      Destination vector.                                           M
*   Vsrc:      Source vector.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecCopy, copy                                                          M
*****************************************************************************/
void GMVecCopy(IrtVecType Vdst, const IrtVecType Vsrc)
{
    IRIT_GEN_COPY(Vdst, Vsrc, sizeof(IrtRType) * 3);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to normalize the vector length to a unit size.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   V:        To normalize.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecNormalize, normalize                                                M
*****************************************************************************/
void GMVecNormalize(IrtVecType V)
{
    IrtRType Len;

    Len = IRIT_VEC_LENGTH(V);

    if (Len > 0.0) {
        Len = 1.0 / Len;
	IRIT_VEC_SCALE(V, Len);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to compute the magnitude (length) of a given 3D vector:           M
*                                                                            *
* PARAMETERS:                                                                M
*   V:        To compute its magnitude.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Magnitude of V.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecLength, magnitude                                                   M
*****************************************************************************/
IrtRType GMVecLength(const IrtVecType V)
{
    return sqrt(IRIT_SQR(V[0]) + IRIT_SQR(V[1]) + IRIT_SQR(V[2]));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to compute the cross product of two vectors.                      M
* Note Vres may be the same as V1 or V2.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vres:    Result of cross product                                         M
*   V1, V2:  Two vectors of the cross product.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecCrossProd, cross prod                                               M
*****************************************************************************/
void GMVecCrossProd(IrtVecType Vres,
		    const IrtVecType V1,
		    const IrtVecType V2)
{
    IrtVecType Vtemp;

    Vtemp[0] = V1[1] * V2[2] - V2[1] * V1[2];
    Vtemp[1] = V1[2] * V2[0] - V2[2] * V1[0];
    Vtemp[2] = V1[0] * V2[1] - V2[0] * V1[1];

    GMVecCopy(Vres, Vtemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the angle between two planar vectors V1 and V2.  Angle is zero   M
* if V2 is in the exact same direction as V1, negative if V2 turns right and M
* positive if V2 turns left.  Angle is returned in degrees in the domain of  M
* (-180, +180].  Only the XY coefficients of V1 and V2 are considered.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2:    Planar vectors to compute their relative angle, in degrees.   M
*   Normalize: TRUE if vectors need normalization first, FALSE if unit size. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Angle between V1 and V2, in degree.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlanarVecVecAngle                                                      M
*****************************************************************************/
IrtRType GMPlanarVecVecAngle(const IrtVecType V1,
			     const IrtVecType V2,
			     int Normalize)
{
    IrtRType DProd, CProd;
    const IrtRType *VP1, *VP2;
    IrtVecType V1Tmp, V2Tmp;

    if (Normalize) {
	IRIT_VEC2D_COPY(V1Tmp, V1);
	IRIT_VEC2D_NORMALIZE(V1Tmp);
	VP1 = V1Tmp;
	IRIT_VEC2D_COPY(V2Tmp, V2);
	IRIT_VEC2D_NORMALIZE(V2Tmp);
	VP2 = V2Tmp;
    }
    else {
	VP1 = V1;
	VP2 = V2;
    }

    DProd = IRIT_DOT_PROD_2D(VP1, VP2);
    CProd = IRIT_CROSS_PROD_2D(VP1, VP2);

    if (CProd == 0)
        return DProd > 0 ? 0.0 : 180.0;
    else if (CProd > 0.0)
	return IRIT_RAD2DEG(acos(IRIT_BOUND(DProd, -1.0, 1.0)));
    else
        return -IRIT_RAD2DEG(acos(IRIT_BOUND(DProd, -1.0, 1.0)));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the angle between two space vectors.		             M
*   Angle is returned in radians in the domain of [-Pi, +Pi].                M
*									     *
* PARAMETERS:                                                                M
*   V1, V2:    Vectors to compute their relative angle, in radians.          M
*   Normalize: TRUE if vectors need normalization first, FALSE if unit size. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  Angle.			                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAreaSphericalTriangle                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecVecAngle	                                                     M
*****************************************************************************/
IrtRType GMVecVecAngle(const IrtVecType V1, const IrtVecType V2, int Normalize)
{
    IrtRType CosAngle;

    if (Normalize) {
        IrtVecType D1, D2;

        IRIT_VEC_COPY(D1, V1);
        IRIT_VEC_COPY(D2, V2);
	IRIT_VEC_NORMALIZE(D1);
	IRIT_VEC_NORMALIZE(D2);
	CosAngle = IRIT_DOT_PROD(D1, D2);
    }
    else
	CosAngle = IRIT_DOT_PROD(V1, V2);

    return acos(IRIT_BOUND(CosAngle, -1.0, 1.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Verifies the collinearity of three points.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3: Three points to verify for collinearity.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if collinear, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCollinear3PtsInside                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCollinear3Pts, collinearity                                            M
*****************************************************************************/
int GMCollinear3Pts(const IrtPtType Pt1,
		    const IrtPtType Pt2,
		    const IrtPtType Pt3)
{
    IrtVecType V1, V2, V3;
    IrtRType L1Sqr, L2Sqr;

    IRIT_PT_SUB(V1, Pt1, Pt2);
    IRIT_PT_SUB(V2, Pt2, Pt3);

    L1Sqr = IRIT_PT_SQR_LENGTH(V1);
    L2Sqr = IRIT_PT_SQR_LENGTH(V2);
    if (L1Sqr < IRIT_SQR(GMBasicColinEps) || L2Sqr < IRIT_SQR(GMBasicColinEps))
	return TRUE;

    IRIT_CROSS_PROD(V3, V1, V2);

    return IRIT_PT_SQR_LENGTH(V3) < (L1Sqr * L2Sqr) * IRIT_SQR(GMBasicColinEps);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an orthogonal vector in R^3 to the given vector.                M
*                                                                            *
* PARAMETERS:                                                                M
*   V:  Input vector to find an orthogonal vector for.                       M
*   OV: Output newly computed orthogonal (possibly unit) vector to V, in R^3.M
*   UnitLen:  If TRUE, normalize the outpt vector.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMOrthogonalVector                                                       M
*****************************************************************************/
int GMOrthogonalVector(const IrtVecType V, IrtVecType OV, int UnitLen)
{
    IrtVecType U;

    IRIT_VEC_RESET(U);

    /* Zeros the maximal coefficient in V. */
    if (fabs(V[0]) <= fabs(V[1]) && fabs(V[0]) <= fabs(V[2])) {
        U[0] = 1.0;
    }
    else if (fabs(V[1]) <= fabs(V[0]) && fabs(V[1]) <= fabs(V[2])) {
        U[1] = 1.0;
    }
    else {
        U[2] = 1.0;
    }

    IRIT_CROSS_PROD(OV, V, U);
    
    if (IRIT_PT_EQ_ZERO(OV))
        return FALSE;

    if (UnitLen)
	IRIT_VEC_NORMALIZE(OV);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Verifies the collinearity of three points and that Pt2 is inside (up to   M
* GMBasicColinEps) the line segment (Pt1, Pt3).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3: Three points to verify for collinearity.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if collinear and inside segment, FALSE otherwise          M
*             (including the case of Pt1 == Pt2 or Pt3 == Pt2).              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCollinear3Pts                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCollinear3PtsInside, collinearity                                      M
*****************************************************************************/
int GMCollinear3PtsInside(const IrtPtType Pt1,
			  const IrtPtType Pt2,
			  const IrtPtType Pt3)
{
    IrtVecType V1, V2, V3;
    IrtRType L1Sqr, L2Sqr;

    IRIT_PT_SUB(V1, Pt1, Pt2);
    IRIT_PT_SUB(V2, Pt2, Pt3);

    L1Sqr = IRIT_PT_SQR_LENGTH(V1);
    L2Sqr = IRIT_PT_SQR_LENGTH(V2);
    if (L1Sqr < IRIT_SQR(GMBasicColinEps) || L2Sqr < IRIT_SQR(GMBasicColinEps))
	return FALSE;

    if (IRIT_DOT_PROD(V1, V2) < 0.0)
	return FALSE;

    IRIT_CROSS_PROD(V3, V1, V2);

    return IRIT_PT_SQR_LENGTH(V3) < (L1Sqr * L2Sqr) * IRIT_SQR(GMBasicColinEps);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Verifies the coplanarity of four points.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3, Pt4: Four points to verify for coplanarity.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if coplanar, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCollinear3Pts                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCoplanar4Pts, coplanarity                                              M
*****************************************************************************/
int GMCoplanar4Pts(const IrtPtType Pt1,
		   const IrtPtType Pt2,
		   const IrtPtType Pt3,
		   const IrtPtType Pt4)
{
    IrtVecType V1, V2, V3, V4;
    IrtRType L1Sqr, L2Sqr, L3Sqr, t;

    IRIT_PT_SUB(V1, Pt1, Pt2);
    IRIT_PT_SUB(V2, Pt1, Pt3);
    IRIT_PT_SUB(V3, Pt1, Pt4);

    L1Sqr = IRIT_PT_SQR_LENGTH(V1);
    L2Sqr = IRIT_PT_SQR_LENGTH(V2);
    L3Sqr = IRIT_PT_SQR_LENGTH(V3);
    if (L1Sqr < IRIT_SQR(GMBasicColinEps) ||
	L2Sqr < IRIT_SQR(GMBasicColinEps) ||
	L3Sqr < IRIT_SQR(GMBasicColinEps))
	return TRUE;

    IRIT_CROSS_PROD(V4, V1, V2);
    t = IRIT_FABS(IRIT_DOT_PROD(V3, V4));

    return IRIT_SQR(t) < (L1Sqr * L2Sqr * L3Sqr) * IRIT_SQR(GMBasicColinEps);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to compute the dot product of two vectors.                        M
*                                                                            M
* PARAMETERS:                                                                M
*   V1, V2:   Two vector to compute dot product of.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Resulting dot product.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMVecDotProd, dot product                                                M
*****************************************************************************/
IrtRType GMVecDotProd(const IrtVecType V1, const IrtVecType V2)
{
    return  V1[0] * V2[0] + V1[1] * V2[1] + V1[2] * V2[2];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the distance between two 3d points.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   P1, P2:   Two points to compute the distance between.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Computed distance.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistPointPoint, point point distance                                   M
*****************************************************************************/
IrtRType GMDistPointPoint(const IrtPtType P1, const IrtPtType P2)
{
    IrtRType t;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointPoint entered\n");
#endif /* DEBUG */

    t = sqrt(IRIT_SQR(P2[0] - P1[0]) +
	     IRIT_SQR(P2[1] - P1[1]) +
	     IRIT_SQR(P2[2] - P1[2]));

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointPoint exit\n");
#endif /* DEBUG */

     return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the linear combination of V1 and V2 that yields V.  It is       M
* assumed that the three vectors are coplanar.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2:   The two vectors that span the plane containing V.		     M
*   V:        To compute lin. comb. "w[0] * V1 + w[1] * V2" for.	     M
*   w:        The two scalar coefficients to be computed.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindLinComb2Vecs                                                       M
*****************************************************************************/
int GMFindLinComb2Vecs(const IrtVecType V1,
		       const IrtVecType V2,
		       const IrtVecType V,
		       IrtRType w[2])
{
    int Axis1, Axis2;
    IrtRType Desc;
    IrtVecType Vec;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMFindLinComb2Vecs entered\n");
#endif /* DEBUG */

    /* Find out the two (out of the three) axes that affects the most. */
    IRIT_CROSS_PROD(Vec, V1, V2);
    if (Vec[0] < Vec[1] && Vec[0] < Vec[2]) {
	Axis1 = 1;
	Axis2 = 2;
    }
    else if (Vec[1] < Vec[0] && Vec[1] < Vec[2]) {
	Axis1 = 0;
	Axis2 = 2;
    }
    else {
	Axis1 = 0;
	Axis2 = 1;
    }

    /* Solve the 2x2 linear system. */
    Desc = V1[Axis1] * V2[Axis2] - V1[Axis2] * V2[Axis1];
    if (IRIT_FABS(Desc) < GMBasicEps)
        return FALSE;

    w[0] = (V[Axis1] * V2[Axis2] - V[Axis2] * V2[Axis1]) / Desc;
    w[1] = (V1[Axis1] * V[Axis2] - V1[Axis2] * V[Axis1]) / Desc;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMFindLinComb2Vecs exit\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to construct a line through 2 points, in the plane. If the       M
* points are the same it returns FALSE, otherwise (successful), TRUE.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:      To compute as Ax + By + C, such that A^2 + B^2 = 1.           M
*   Pt1, Pt2:  Two points to fit a line through. Only XY coordinates are     M
*	       considered.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMLineFrom2Points, line                                                  M
*****************************************************************************/
int GMLineFrom2Points(IrtLnType Line, const IrtPtType Pt1, const IrtPtType Pt2)
{
    IrtRType Size;

    Line[0] = Pt2[1] - Pt1[1];
    Line[1] = Pt1[0] - Pt2[0];
    Line[2] = -Line[0] * Pt1[0] - Line[1] * Pt1[1];

    if ((Size = sqrt(IRIT_SQR(Line[0]) + IRIT_SQR(Line[1]))) < GMBasicEps)
	return FALSE;
    else {
	Size = 1.0 / Size;
        Line[0] *= Size;
        Line[1] *= Size;
        Line[2] *= Size;

	return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a point on and the direction of the given line                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:   To extract and point and a direction for.                        M
*   Pt:     A point on line Line.			                     M
*   Dir:    The direction of line Line.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointVecFromLine                                                       M
*****************************************************************************/
void GMPointVecFromLine(const IrtLnType Line, IrtPtType Pt, IrtVecType Dir)
{
    Pt[2] = Dir[2] = 0.0;

    if (IRIT_FABS(Line[0]) > IRIT_FABS(Line[1])) {
	Pt[0] = -Line[2] / Line[0];
	Pt[1] = 0;
    }
    else {
	Pt[0] = 0;
	Pt[1] = -Line[2] / Line[1];
    }

    Dir[0] = Line[1];
    Dir[1] = -Line[0];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to construct the plane from given 3 points. If two of the points M
* are the same or the three points are collinear it returns FALSE, otherwise M
* (successful), it returns TRUE.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:          To compute.                                              M
*   Pt1, Pt2, Pt3:  Three points to fit a plane through.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlaneFrom3Points, plane                                                M
*****************************************************************************/
int GMPlaneFrom3Points(IrtPlnType Plane,
		       const IrtPtType Pt1,
		       const IrtPtType Pt2,
		       const IrtPtType Pt3)
{
    IrtVecType V1, V2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPlaneFrom3Points entered\n");
#endif /* DEBUG */

    if (GMCollinear3Pts(Pt1, Pt2, Pt3))
	return FALSE;

    IRIT_VEC_SUB(V1, Pt2, Pt1);
    IRIT_VEC_SUB(V2, Pt3, Pt2);
    IRIT_CROSS_PROD(Plane, V1, V2);
    IRIT_VEC_NORMALIZE(Plane);

    Plane[3] = -IRIT_DOT_PROD(Plane, Pt1);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPlaneFrom3Points exit\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the closest point on a given 3d line to a given 3d    M
* point. The line is prescribed using a point on it (Pl) and vector (Vl).    M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the closest to on the line.                       M
*   Pl, Vl:        Position and direction that defines the line. Vl need not M
*		   be a unit length vector.		                     M
*   ClosestPoint:  Where closest point found on the line is to be saved.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPointFromPointLine     		                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointFromPointLine, point line distance                                M
*****************************************************************************/
void GMPointFromPointLine(const IrtPtType Point,
			  const IrtPtType Pl,
			  const IrtPtType Vl,
			  IrtPtType ClosestPoint)
{
    IrtPtType V1;
    IrtRType CosAlpha, l;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromPointLine entered\n");
#endif /* DEBUG */

    IRIT_VEC_SUB(V1, Point, Pl);

    /* Compute the magnitude square of Vl. */
    if ((l = IRIT_VEC_SQR_LENGTH(Vl)) < IRIT_SQR(IRIT_EPS))
        l = IRIT_SQR(IRIT_EPS);

    /* Find angle between the two vectors, including magnitudes of V1 & Vl. */
    CosAlpha = IRIT_DOT_PROD(V1, Vl) / l;

    /* Find P1 - the closest point to Point on the line: */
    IRIT_PT_SCALE_AND_ADD(ClosestPoint, Pl, Vl, CosAlpha);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromPointLine exit\n");
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the disstance between a 3d point and a 3d line.         M
*   The line is prescribed using a point on it (Pl) and vector (Vl).         M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the distance to on the line.                      M
*   Pl, Vl:        Position and direction that defines the line.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:      The computed distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarDistPointLine     		                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistPointLine, point line distance                                     M
*****************************************************************************/
IrtRType GMDistPointLine(const IrtPtType Point,
			 const IrtPtType Pl,
			 const IrtPtType Vl)
{
    IrtRType t;
    IrtPtType Ptemp;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointLine entered\n");
#endif /* DEBUG */

    GMPointFromPointLine(Point, Pl, Vl, Ptemp);/* Find closest point on line.*/
    t = IRIT_PT_PT_DIST(Point, Ptemp);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointLine exit\n");
#endif /* DEBUG */

    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the distance between a Point and a Plane. The Plane   M
* is prescribed using its four coefficients : Ax + By + Cz + D = 0 given as  M
* four elements vector.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the distance to on the plane.                     M
*   Plane:         To find the distance to on the point.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:      The computed distance.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistPointPlane, point plane distance                                   M
*****************************************************************************/
IrtRType GMDistPointPlane(const IrtPtType Point, const IrtPlnType Plane)
{
    IrtRType t;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointPlane entered\n");
#endif /* DEBUG */

    t = (IRIT_DOT_PROD(Plane, Point) + Plane[3]) / IRIT_VEC_LENGTH(Plane);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistPointPlane exit\n");
#endif /* DEBUG */

    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the closest point on a given plane to a given 3d      M
* point. 								     M
*   The Plane is prescribed using four coefficients : Ax + By + Cz + D = 0   M
* given as four elements vector.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:            Point to find closest point on Plane to.                  M
*   Plane:         To find the closest point on to Pt.                       M
*   ClosestPoint:  Where the closest point on Plane to Pt is.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE, if successful.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointFromPointPlane, point plane distance                              M
*****************************************************************************/
int GMPointFromPointPlane(const IrtPtType Pt,
			  const IrtPlnType Plane,
			  IrtPtType ClosestPoint)
{
    IrtRType t, DotProd;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromPointPlane entered\n");
#endif /* DEBUG */

    DotProd = IRIT_DOT_PROD(Plane, Plane);
    if (IRIT_FABS(DotProd) < GMBasicEps)
	return FALSE;				 /* Vl is coplanar to Plane. */

    /* Else find t in line such that the plane equation plane is satisfied: */
    t = (-Plane[3] - IRIT_DOT_PROD(Plane, Pt)) / DotProd;

    /* And so find the intersection point which is at that t: */
    IRIT_PT_SCALE_AND_ADD(ClosestPoint, Pt, Plane, t);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromPointPlane exit\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to find the intersection point of a line and a plane (if any).     M
*   The Plane is prescribed using four coefficients : Ax + By + Cz + D = 0   M
* given as four elements vector. The line is define via a point on it Pl and M
* a direction vector Vl. Returns TRUE only if such point exists.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl, Vl:        Position and direction that defines the line.             M
*   Plane:         To find the intersection with the line.                   M
*   InterPoint:    Where the intersection occured.                           M
*   t:             Parameter along the line of the intersection location     M
*                  (as Pl + Vl * t).                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE, if successful.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointFromLinePlane, line plane intersection                            M
*****************************************************************************/
int GMPointFromLinePlane(const IrtPtType Pl,
			 const IrtPtType Vl,
			 const IrtPlnType Plane,
			 IrtPtType InterPoint,
			 IrtRType *t)
{
    IrtRType DotProd;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromLinePlane entered\n");
#endif /* DEBUG */

    /* Check to see if they are vertical - no intersection at all! */
    DotProd = IRIT_DOT_PROD(Vl, Plane);
    if (IRIT_FABS(DotProd) < GMBasicEps)
	return FALSE;				 /* Vl is coplanar to Plane. */

    /* Else find t in line such that the plane equation plane is satisfied: */
    *t = (-Plane[3] - IRIT_DOT_PROD(Plane, Pl)) / DotProd;

    /* And so find the intersection point which is at that t: */
    IRIT_PT_SCALE_AND_ADD(InterPoint, Pl, Vl, *t);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromLinePlane exit\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to find the intersection point of a line and a plane (if any).     M
*   The Plane is prescribed using four coefficients : Ax + By + Cz + D = 0   M
* given as four elements vector. The line is define via a point on it Pl and M
* a direction vector Vl. Returns TRUE only if such point exists.             M
*   this routine accepts solutions only for t between zero and one.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl, Vl:        Position and direction that defines the line.             M
*   Plane:         To find the intersection with the line.                   M
*   InterPoint:    Where the intersection occured.                           M
*   t:             Parameter along the line of the intersection location     M
*                  (as Pl + Vl * t).                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE, if successful and t between zero and one.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointFromLinePlane01, line plane intersection                          M
*****************************************************************************/
int GMPointFromLinePlane01(const IrtPtType Pl,
			   const IrtPtType Vl,
			   const IrtPlnType Plane,
			   IrtPtType InterPoint,
			   IrtRType *t)
{
    IrtRType DotProd;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromLinePlane01 entered\n");
#endif /* DEBUG */

    /* Check to see if they are vertical - no intersection at all! */
    DotProd = IRIT_DOT_PROD(Vl, Plane);
    if (IRIT_FABS(DotProd) < GMBasicEps)
	return FALSE;

    /* Else find t in line such that the plane equation plane is satisfied: */
    *t = (-Plane[3] - IRIT_DOT_PROD(Plane, Pl)) / DotProd;

    if ((*t < 0.0 && !IRIT_APX_UEQ(*t, 0.0)) ||	 /* Not in parameter range. */
	(*t > 1.0 && !IRIT_APX_UEQ(*t, 1.0)))
	return FALSE;

    /* And so find the intersection point which is at that t: */
    IRIT_PT_SCALE_AND_ADD(InterPoint, Pl, Vl, *t);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMPointFromLinePlane01 exit\n");
#endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to find the two points Pti on the lines (Pli, Vli) ,   i = 1, 2  M
* with the minimal Euclidian distance between them. In other words, the      M
* distance between Pt1 and Pt2 is defined as distance between the two lines. M
*   The two points are calculated using the fact that if V = (Vl1 cross Vl2) M
* then these two points are the intersection point between the following:    M
* Point 1 - a plane (defined by V and line1) and the line line2.             M
* Point 2 - a plane (defined by V and line2) and the line line1.             M
*   This function returns TRUE iff the two lines are not parallel!           M
*   This function is also valid for the case of coplanar lines.		     M
*                                                                            M
* PARAMETERS:                                                                M
*   Pl1, Vl1:  Position and direction defining the first line.               M
*   Pl2, Vl2:  Position and direction defining the second line.              M
*   Pt1:       Point on Pt1 that is closest to line 2.                       M
*   t1:        Parameter value of Pt1 as (Pl1 + Vl1 * t1).                   M
*   Pt2:       Point on Pt2 that is closest to line 1.                       M
*   t2:        Parameter value of Pt2 as (Pl2 + Vl2 * t2).                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE, if successful.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromCircCirc                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM2PointsFromLineLine, line line distance                                M
*****************************************************************************/
int GM2PointsFromLineLine(const IrtPtType Pl1,
			  const IrtPtType Vl1,
			  const IrtPtType Pl2,
			  const IrtPtType Vl2,
			  IrtPtType Pt1,
			  IrtRType *t1,
			  IrtPtType Pt2,
			  IrtRType *t2)
{
    int i;
    IrtPtType Vtemp;
    IrtPlnType Plane1, Plane2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GM2PointsFromLineLine entered\n");
#endif /* DEBUG */

    IRIT_CROSS_PROD(Vtemp, Vl1, Vl2);  /* Check to see if they are parallel. */
    if (IRIT_VEC_SQR_LENGTH(Vtemp) < GMBasicEpsSqr) {
	*t1 = *t2 = IRIT_INFNTY;
	IRIT_PT_COPY(Pt1, Pl1);		     /* Pick point on line1 and find */
	GMPointFromPointLine(Pl1, Pl2, Vl2, Pt2); /* closest point on line2. */
        return FALSE;
    }

    /* Define the two planes: 1) Vl1, Pl1, Vtemp and 2) Vl2, Pl2, Vtemp	     */
    /* Note this sets the first 3 elements A, B, C out of the 4...	     */
    IRIT_CROSS_PROD(Plane1, Vl1, Vtemp);	/* Find the A, B, C coef.'s. */
    IRIT_VEC_SAFE_NORMALIZE(Plane1);
    IRIT_CROSS_PROD(Plane2, Vl2, Vtemp);	/* Find the A, B, C coef.'s. */
    IRIT_VEC_SAFE_NORMALIZE(Plane2);

    /* and now use a point on the plane to find the 4th coef. D: */
    Plane1[3] = -IRIT_DOT_PROD(Plane1, Pl1);/* IRIT_DOT_PROD uses only first */
    Plane2[3] = -IRIT_DOT_PROD(Plane2, Pl2);  /* three elements in vec.      */

    /* Thats it! now we should solve for the intersection point between a    */
    /* line and a plane but we are already familiar with this problem...     */
    i = GMPointFromLinePlane(Pl1, Vl1, Plane2, Pt1, t1) &&
	GMPointFromLinePlane(Pl2, Vl2, Plane1, Pt2, t2);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GM2PointsFromLineLine exit\n");
#endif /* DEBUG */

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Find the intersection point (if exists) of three planes.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2, Pl3:   Three planes to consider.				     M
*   Pt:              Intersection point found (if any).		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if exists an intersection point, FALSE otherwise.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointFrom3Planes                                                       M
*****************************************************************************/
int GMPointFrom3Planes(const IrtPlnType Pl1,
		       const IrtPlnType Pl2,
		       const IrtPlnType Pl3,
		       IrtPtType Pt)
{
    IrtHmgnMatType Mat, InvMat;
    IrtVecType V;

    MatGenUnitMat(Mat);

    Mat[0][0] = Pl1[0]; Mat[1][0] = Pl1[1]; Mat[2][0] = Pl1[2];
    Mat[0][1] = Pl2[0]; Mat[1][1] = Pl2[1]; Mat[2][1] = Pl2[2];
    Mat[0][2] = Pl3[0]; Mat[1][2] = Pl3[1]; Mat[2][2] = Pl3[2];

    if (!MatInverseMatrix(Mat, InvMat))
        return FALSE;

    V[0] = -Pl1[3];
    V[1] = -Pl2[3];
    V[2] = -Pl3[3];
    MatMultVecby4by4(Pt, V, InvMat);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to find the distance between two lines (Pli, Vli) ,  i = 1, 2.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Vl1:  Position and direction defining the first line.               M
*   Pl2, Vl2:  Position and direction defining the second line.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Distance between the two lines.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistLineLine, line line distance                                       M
*****************************************************************************/
IrtRType GMDistLineLine(const IrtPtType Pl1,
			const IrtPtType Vl1,
			const IrtPtType Pl2,
			const IrtPtType Vl2)
{
    IrtRType t1, t2;
    IrtPtType Ptemp1, Ptemp2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistLineLine entered\n");
#endif /* DEBUG */

    GM2PointsFromLineLine(Pl1, Vl1, Pl2, Vl2, Ptemp1, &t1, Ptemp2, &t2);
    t1 = IRIT_PT_PT_DIST(Ptemp1, Ptemp2);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMDistLineLine exit\n");
#endif /* DEBUG */

    return t1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the closest vertices of Pl1 and Pl2.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2:  Two polys we seek the closest vertices of the two.            M
*   V1, V2:    Two vertices on Pl1 and Pl2, respectively that are closest.   M
*                                                                            *
* RETURN VALUE:								     M
*   IrtRType:   Distance between V1 and V2.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistPolyPoly                                                           M
*****************************************************************************/
IrtRType GMDistPolyPoly(const IPPolygonStruct *Pl1,
			const IPPolygonStruct *Pl2,
			IPVertexStruct **V1,
			IPVertexStruct **V2)
{
    IrtRType DistSqr,
        MinDistSqr = IRIT_INFNTY;
    IPVertexStruct
        *Vrtx1 = Pl1 -> PVertex;

    do {
	IPVertexStruct
	    *Vrtx2 = Pl2 -> PVertex;

        do {
	    DistSqr = IRIT_PT_PT_DIST_SQR(Vrtx1 -> Coord, Vrtx2 -> Coord);
	    if (DistSqr < MinDistSqr) {
	        *V1 = Vrtx1;
	        *V2 = Vrtx2;
		MinDistSqr = DistSqr;
	    }

	    Vrtx2 = Vrtx2 -> Pnext;
	}
	while (Vrtx2 != NULL && Vrtx2 != Pl2 -> PVertex);

        Vrtx1 = Vrtx1 -> Pnext;
    }
    while (Vrtx1 != NULL && Vrtx1 != Pl1 -> PVertex);

    return sqrt(MinDistSqr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if the given polygon intersects the given plane.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:      Polygon to test if interestes plane Pln.                        M
*   Pln:     Plane to examine if polygon Pl intersects.  Assumed normalized  M
*	     normal vector in Pln.			                     M
*   MinDist: Returns closest vertex in Pl to Pln.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if intersects, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonRayInter, GMSplitPolygonAtPlane                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonPlaneInter                                                      M
*****************************************************************************/
int GMPolygonPlaneInter(const IPPolygonStruct *Pl,
			const IrtPlnType Pln,
			IrtRType *MinDist)
{
    IPVertexStruct
	*VHead = Pl -> PVertex,
	*V = VHead;
    IrtRType
	MaxNegDist = 0.0,
	MaxPosDist = 0.0;

    *MinDist = IRIT_INFNTY;

    do {
	IrtRType
	    Dist = IRIT_DOT_PROD(V -> Coord, Pln) + Pln[3];

	if (Dist > 0.0) {
	    if (*MinDist > Dist)
	        *MinDist = Dist;

	    MaxPosDist = IRIT_MAX(Dist, MaxPosDist);
	}
	else {
	    Dist = -Dist;

	    if (*MinDist > Dist)
	        *MinDist = Dist;

	    MaxNegDist = IRIT_MAX(Dist, MaxNegDist);

	}

        V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    return MaxNegDist > 0.0 && MaxPosDist > 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Split the given convex polygon where it intersects the given plane.      M
* Pl is updated to in inside potion (where the normal is point into) and     M
* Pl -> Pnext will hold the second half.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:      Polygon to split if interestes plane Pln.                       M
*   Pln:     Plane to split polygon Pl at.  Assumed normalized		     M
*	     normal vector in Pln.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if intersects, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonRayInter, GMPolygonPlaneInter                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSplitPolygonAtPlane                                                    M
*****************************************************************************/
int GMSplitPolygonAtPlane(IPPolygonStruct *Pl, const IrtPlnType Pln)
{
    int i,
	NumInters = 0;
    IrtRType D1, D2;
    IPVertexStruct *InterV[2], *PrevV[2],
	*VHead = Pl -> PVertex,
	*V = VHead;

    do {
        IrtRType t;
	IrtPtType InterPoint;
        IrtVecType Vl;
	IPVertexStruct
	    *VNext = V -> Pnext == NULL ? VHead : V -> Pnext;

	IRIT_VEC_SUB(Vl, VNext -> Coord, V -> Coord);

	if (GMPointFromLinePlane(V -> Coord, Vl, Pln, InterPoint, &t) &&
	    t > 0.0 && t <= 1.0) {
	    PrevV[NumInters] = V;

	    InterV[NumInters] = IPAllocVertex2(V -> Pnext);
	    IRIT_PT_COPY(InterV[NumInters] -> Coord, InterPoint);

	    GMUpdateVertexByInterp(InterV[NumInters], V, VNext,
				   TRUE, TRUE, TRUE);

	    if (++NumInters >= 2)
	        break;
	}

        V = VNext;
    }
    while (V != VHead);

    if (NumInters < 2) {
	for (i = 0; i < NumInters; i++)
	    IPFreeVertex(InterV[i]);
	return FALSE;
    }

    /* We have two intersection locations - split the polygon. */
    PrevV[0] -> Pnext = InterV[0];
    PrevV[1] -> Pnext = InterV[1];
    if (GMSplitPolyInPlaceAt2Vertices(Pl, InterV[0], InterV[1])) {
        D1 = GMPolyPlaneClassify(Pl, Pln);
	D2 = GMPolyPlaneClassify(Pl -> Pnext, Pln);

	if (D1 > 0 && D2 < 0)
	    return TRUE;
	else if (D1 < 0 && D2 > 0) {
	    IRIT_SWAP(IPVertexStruct *, Pl -> PVertex, Pl -> Pnext -> PVertex);
	    return TRUE;
	}
	else {
	    GEOM_FATAL_ERROR(GEOM_ERR_INVALID_POLYGON);
	    return FALSE;
	}
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Classify a plane as (mostly) on the positive side of the plane,          M
* returning a positive value, or (mostly) on the negative side of the plane, M
* returning a negative value.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:     Polygon to consider and classify.                                M
*   Pln:    Plane to claassify against.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Positive or negative, classifying the polygon's side.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyPlaneClassify                                                      M
*****************************************************************************/
IrtRType GMPolyPlaneClassify(const IPPolygonStruct *Pl, const IrtPlnType Pln)
{
    IrtRType
	D = 0.0;
    IPVertexStruct
	*VHead = Pl -> PVertex,
	*V = VHead;

    do {
	D += IRIT_DOT_PROD(Pln, V -> Coord) + Pln[3];

        V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    return D;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to check if a point is inside a triangle, in the XY plane.       M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2, V3: Triangle to test if Pt is in it.			     M
*   Pt:         Point to test for inclusion in triangle.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if Pt inside triangle, FALSE otherwise.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonPointInclusion, GMPolygonPlaneInter, GMPolygonRayInter          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTrianglePointInclusion, point inclusion				     M
*****************************************************************************/
int GMTrianglePointInclusion(const IrtRType *V1,
			     const IrtRType *V2,
			     const IrtRType *V3,
			     const IrtPtType Pt)
{
    IrtRType c1, c2, c3;
    IrtVecType V_V, V_Pt;

    IRIT_VEC2D_SUB(V_Pt, V1, Pt);
    IRIT_VEC2D_SUB(V_V, V1, V2);
    c1 = IRIT_CROSS_PROD_2D(V_Pt, V_V);

    IRIT_VEC2D_SUB(V_Pt, V2, Pt);
    IRIT_VEC2D_SUB(V_V, V2, V3);
    c2 = IRIT_CROSS_PROD_2D(V_Pt, V_V);

    IRIT_VEC2D_SUB(V_Pt, V3, Pt);
    IRIT_VEC2D_SUB(V_V, V3, V1);
    c3 = IRIT_CROSS_PROD_2D(V_Pt, V_V);

    return c1 * c2 > -GMBasicEps && c2 * c3 > -GMBasicEps;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to check if a point is inside a polygon, in the XY plane.        M
*   Uses winding number accumulation in the computation.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        Polygon to test if Pt is in it.				     M
*   Pt:        Point to test for inclusion in Pl.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if Pt inside Pl, FALSE otherwise.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonPointInclusion3D, GMPolygonPlaneInter, GMPolygonRayInter        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonPointInclusion, ray polygon intersection, point inclusion       M
*****************************************************************************/
int GMPolygonPointInclusion(const IPPolygonStruct *Pl, const IrtPtType Pt)
{
    IrtRType
        Theta = 0.0;
    IrtPtType FirstDir, PrevDir, Dir;
    IPVertexStruct
	*VHead = Pl -> PVertex,
	*V = VHead;

    IRIT_VEC2D_SET(PrevDir, V -> Coord[0] - Pt[0], V -> Coord[1] - Pt[1]);
    IRIT_VEC2D_NORMALIZE(PrevDir);
    IRIT_VEC2D_COPY(FirstDir, PrevDir);
    V = V -> Pnext;
    do {
        IRIT_VEC2D_SET(Dir, V -> Coord[0] - Pt[0], V -> Coord[1] - Pt[1]);
	IRIT_VEC2D_NORMALIZE(Dir);

	/* Computes angle between Pt and the two end point of segment. */
	Theta += atan2(IRIT_CROSS_PROD_2D(PrevDir, Dir),
		       IRIT_DOT_PROD_2D(PrevDir, Dir));

	IRIT_VEC2D_COPY(PrevDir, Dir);

        V = V -> Pnext;
    }
    while (V != NULL && V != VHead);
    Theta += atan2(IRIT_CROSS_PROD_2D(PrevDir, FirstDir),
		   IRIT_DOT_PROD_2D(PrevDir, FirstDir));

    return IRIT_FABS(Theta) > M_PI;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the area of a spherical triangle over the unit sphere with      M
* given three (unit vector) vertices, Dir1, Dir2, Dir3.                      M
*   Area is equal to (Alpha1 + Alpha2 + Alpha3 - Pi) where Alphai is the     M
* angle at vertex Diri with the other two vertices.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir1, Dir2, Dir3:  Vertices of the spherical triangle.  unit vectors.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Computed area.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAngleSphericalTriangle                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAreaSphericalTriangle                                                  M
*****************************************************************************/
IrtRType GMAreaSphericalTriangle(const IrtVecType Dir1,
				 const IrtVecType Dir2,
				 const IrtVecType Dir3)
{
    IrtVecType V;
    IrtRType
	x1 = GMAngleSphericalTriangle(Dir1, Dir2, Dir3),
        x2 = GMAngleSphericalTriangle(Dir2, Dir3, Dir1),
        x3 = GMAngleSphericalTriangle(Dir3, Dir1, Dir2);

    assert(x1 + x2 + x3 - M_PI > 0.0);

    /* Decide on the sign. */
    IRIT_CROSS_PROD(V, Dir1, Dir2);
    return IRIT_DOT_PROD(V, Dir3) > 0.0 ? x1 + x2 + x3 - M_PI
				        : M_PI - x1 - x2 - x3;

}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the angle at Dir, with respect to ODir1 and ODir2.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir:      Spherical vertex to compute its angle with respect to ODir1/2. M
*   ODir1, ODir2:  Other two vertices of spherical triangle.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  Spherical angle at Dir.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAreaSphericalTriangle                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAngleSphericalTriangle                                                 M
*****************************************************************************/
IrtRType GMAngleSphericalTriangle(const IrtVecType Dir,
				  const IrtVecType ODir1,
				  const IrtVecType ODir2)
{
    IrtVecType D1Aux, D2Aux, D1, D2;

    IRIT_VEC_SUB(D1Aux, ODir1, Dir);
    IRIT_VEC_SUB(D2Aux, ODir2, Dir);

    /* Make sure the vectors are orthogonal to Dir. */
    IRIT_CROSS_PROD(D1, D1Aux, Dir);
    IRIT_CROSS_PROD(D2, D2Aux, Dir);

    return GMVecVecAngle(D1, D2, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to check if a point is inside a polygon.                         M
*   Computes the accumulated Gaussian sphere area deviation of Pt-vertices   M
* over all triangles in the given Pl model, with respect to given point Pt.  M
*   Each angular deviation of each triangle is measured as area of spherical M
* triangle over the Gaussian sphere, formed from its three Pt-vertices dirs. M
*   A point is inside a simple closed polyhedra if the sum is +/-4 Pi, zero  M
* if the point is outside.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        Polyhedra in 3D to test if Pt is in it.			     M
*   Pt:        Point to test for inclusion in Pl.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if Pt inside Pl, FALSE otherwise.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonPointInclusion, GMPolygonPlaneInter, GMPolygonRayInter          M
*   GMAreaSphericalTriangle						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonPointInclusion3D, ray polygon intersection, point inclusion     M
*****************************************************************************/
int GMPolygonPointInclusion3D(const IPPolygonStruct *Pl, const IrtPtType Pt)
{
    IrtRType
        Area = 0.0;

    for ( ; Pl != NULL; Pl = Pl -> Pnext) {
        IrtVecType Dir1, Dir2, Dir3;
        IPVertexStruct
	    *VHead = Pl -> PVertex,
	    *V = VHead -> Pnext;

	IRIT_VEC_SUB(Dir1, VHead -> Coord, Pt);
	IRIT_VEC_SUB(Dir2, V -> Coord, Pt);
	V = V -> Pnext;
	do {
	    IRIT_VEC_SUB(Dir3, V -> Coord, Pt);

	    /* Accumulate the spherical (signed) area. */
	    Area += GMAreaSphericalTriangle(Dir1, Dir2, Dir3);

	    IRIT_VEC_COPY(Dir2, Dir3);

	    V = V -> Pnext;
	}
	while (V != NULL && V != VHead);
    }

    return IRIT_FABS(Area) > M_PI * 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine that implements "Jordan Theorem".  Same as GMPolygonRayInter2 but  M
* does not return the first intersection info.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        To compute "Jordan Theorem" for the given ray.                M
*	       Can be either a polygon or a closed polyline (first and last  M
*	       points of polyline are equal).				     M
*   PtRay:     Origin of ray.                                                M
*   RayAxes:   Direction of ray. 0 for X, 1 for Y, etc.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Number of intersections of ray with the polygon.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonPlaneInter, GMPolygonPointInclusion, GMSplitPolygonAtPlane      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonRayInter, ray polygon intersection, Jordan theorem              M
*****************************************************************************/
int GMPolygonRayInter(const IPPolygonStruct *Pl,
		      const IrtPtType PtRay,
		      int RayAxes)
{
    IPVertexStruct *FirstInterV;
    IrtRType FirstInterP;

    return GMPolygonRayInter2(Pl, PtRay, RayAxes, &FirstInterV, &FirstInterP);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine that implements "Jordan Theorem":                                  M
*   Fire a ray from a given point and find the number of intersections of a  M
* ray with the polygon, excluding the given point Pt (start of ray) itself,  M
* if on polygon boundary. The ray is fired in +X (Axes == 0) or +Y if        M
* (Axes == 1).                                                               M
*   Only the X/Y coordinates of the polygon are taken into account, i.e. the M
* orthogonal projection of the polygon on an X/Y parallel plane (equal to    M
* polygon itself if on X/Y parallel plane...).				     M
*   Note that if the point is on polygon boundary, the ray should not be in  M
* its edge direction.							     M
*									     M
* Algorithm:								     M
* 1. 1.1. Set NumOfIntersection = 0;					     M
*    1.2. Find vertex V not on Ray level and set AlgState to its level       M
*         (below or above the ray level). If none goto 3;		     M
*    1.3. Mark VStart = V;						     M
* 2. Do									     M
*    2.1. While State(V) == AlgState do					     M
*	    2.1.1. V = V -> Pnext;					     M
*	    2.1.2. If V == VStart goto 3;				     M
*    2.2. IntersectionMinX = IRIT_INFNTY;				     M
*    2.3. While State(V) == ON_RAY do					     M
*	    2.3.1. IntersectionMin = IRIT_MIN(IntersectionMin,               M
*                                             V -> Coord[Axes]);             M
*	    2.3.2. V = V -> Pnext;					     M
*    2.4. If State(V) != AlgState do					     M
*	    2.4.1. Find the intersection point between polygon edge	     M
*		   VLast, V and the Ray and update IntersectionMin if	     M
*		   lower than it.					     M
*	    2.4.2. If IntersectionMin is greater than Pt[Axes] increase	     M
*		   the NumOfIntersection counter by 1.		  	     M
*    2.5. AlgState = State(V);						     M
*    2.6. goto 2.2.							     M
* 3. Return NumOfIntersection;						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:           To compute "Jordan Theorem" for the given ray.             M
*	          Can be either a polygon or a closed polyline (first and    M
*	          last points of polyline are equal).			     M
*   PtRay:        Origin of ray.                                             M
*   RayAxes:      Direction of ray. 0 for X, 1 for Y, etc.                   M
*   FirstInterV:  OUT - First intersection edge, between V and VNext.        M
*   FirstInterP:  OUT - First intersection location, blending V and VNext.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Number of intersections of ray with the polygon.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolygonPlaneInter, GMSplitPolygonAtPlane                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonRayInter2, ray polygon intersection, Jordan theorem             M
*****************************************************************************/
int GMPolygonRayInter2(const IPPolygonStruct *Pl,
		       const IrtPtType PtRay,
		       int RayAxes,
		       IPVertexStruct **FirstInterV,
		       IrtRType *FirstInterP)
{
    int NewState, AlgState, RayOtherAxes,
	Quit = FALSE,
	NumOfInter = 0;
    IrtRType InterMin, Inter, t,
	FirstInterDist = IRIT_INFNTY;
    const IPVertexStruct *V, *VStart,
	*VLast = NULL;

    RayOtherAxes = (RayAxes == 1 ? 0 : 1);     /* Other dir: X -> Y, Y -> X. */

    /* Stage 1 - find a vertex below the ray level: */
    V = VStart = Pl -> PVertex;
    do {
	if ((AlgState = GMPointRayRelation(V -> Coord, PtRay, RayOtherAxes))
							         != GM_ON_RAY)
	    break;
	V = V -> Pnext;
    }
    while (V != VStart && V != NULL);
    if (AlgState == GM_ON_RAY)
	return 0;
    VStart = V; /* Vertex Below Ray level */

    /* Stage 2 - scan the vertices and count number of intersections. */
    while (!Quit) {
	/* Stage 2.1. : */
	while (GMPointRayRelation(V -> Coord, PtRay,
				  RayOtherAxes) == AlgState) {
	    VLast = V;
	    V = V -> Pnext;
	    if (V == VStart) {
		Quit = TRUE;
		break;
	    }
	    else if (V == NULL)
		return NumOfInter;
	}
	InterMin = IRIT_INFNTY;

	/* Stage 2.2. : */
	while (GMPointRayRelation(V -> Coord, PtRay,
				  RayOtherAxes) == GM_ON_RAY) {
	    InterMin = IRIT_MIN(InterMin, V -> Coord[RayAxes]);
	    VLast = V;
	    V = V -> Pnext;
	    if (V == VStart)
		Quit = TRUE;
	    else if (V == NULL)
		return NumOfInter;
	}

	/* Stage 2.3. : */
	if ((NewState = GMPointRayRelation(V -> Coord, PtRay, RayOtherAxes))
								!= AlgState) {
	    /* Stage 2.3.1 Intersection with ray is in middle of edge: */
	    t = (PtRay[RayOtherAxes] - V -> Coord[RayOtherAxes]) /
		(VLast -> Coord[RayOtherAxes] - V -> Coord[RayOtherAxes]);
	    Inter = VLast -> Coord[RayAxes] * t +
		    V -> Coord[RayAxes] * (1.0 - t);
	    InterMin = IRIT_MIN(InterMin, Inter);

	    /* Stage 2.3.2. comp. with ray base and inc. # of inter if above.*/
	    if (InterMin > PtRay[RayAxes] &&
		!IRIT_APX_UEQ(InterMin, PtRay[RayAxes])) {
		NumOfInter++;

		if (FirstInterDist > IRIT_FABS(Inter - PtRay[RayAxes])) {
		    FirstInterDist = IRIT_FABS(Inter - PtRay[RayAxes]);
		    *FirstInterV = (IPVertexStruct *) VLast;
		    *FirstInterP = t;
		}
	    }
	}

	AlgState = NewState;
    }

    return NumOfInter;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to returns the relation between the ray level and a given point, *
* to be used in the GMPolygonRayInter routine above.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:       Given point.                                                   *
*   PtRay:    Given ray.                                                     *
*   Axes:     Given axes.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Pt is either above below or on the ray.                        *
*****************************************************************************/
static int GMPointRayRelation(const IrtPtType Pt,
			      const IrtPtType PtRay,
			      int Axes)
{
    if (IRIT_APX_UEQ(PtRay[Axes], Pt[Axes]))
        return GM_ON_RAY;
    else if (PtRay[Axes] < Pt[Axes])
        return GM_ABOVE_RAY;
    else
	return GM_BELOW_RAY;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as GMPolygonRayInter but for arbitrary oriented polygon.		     M
*   The polygon is transformed into the XY plane and then GMPolygonRayInter  M
* is invoked on it.                                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        To compute "Jordan Theorem" for the given ray.                M
*   PtRay:     Origin of ray.                                                M
*   RayAxes:   Direction of ray. 0 for X, 1 for Y, etc.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Number of intersections of ray with the polygon.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonRayInter3D, ray polygon intersection, Jordan theorem            M
*****************************************************************************/
int GMPolygonRayInter3D(const IPPolygonStruct *Pl,
			const IrtPtType PtRay,
			int RayAxes)
{
    int i;
    IrtHmgnMatType RotMat;
    IPVertexStruct *V, *VHead;
    IPPolygonStruct *RotPl;
    IrtPtType RotPt;

    /* Make a copy of original to work on. */
    RotPl = IPAllocPolygon(Pl -> Tags, IPCopyVertexList(Pl -> PVertex), NULL);

    /* Make sure list is circular. */
    V = IPGetLastVrtx(RotPl -> PVertex);
    if (V -> Pnext == NULL)
	V -> Pnext = RotPl -> PVertex;

    /* Bring the polygon to a XY parallel plane by rotation. */
    GMGenRotateMatrix(RotMat, Pl -> Plane);
    V = VHead = RotPl -> PVertex;
    do {				    /* Transform the polygon itself. */
	MatMultPtby4by4(V -> Coord, V -> Coord, RotMat);
	V = V -> Pnext;
    }
    while (V != VHead);
    MatMultPtby4by4(RotPt, PtRay, RotMat);

    i = GMPolygonRayInter(RotPl, RotPt, RayAxes);

    IPFreePolygonList(RotPl);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a root polygons with islands into a closed, simple, polygon.    M
*   Interior islands are all connected into the root, outer, polygon.        M
*   The outer, root, loop is assumed to be oriented in opposite              M
* direction to the islands.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:      The top most, outer polygon.                                  M
*   Islands:   Interior islands to connected with root into one simply poly. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:    Merged poly.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyHierarchy2SimplePoly, polygonization, islands, holes               M
*****************************************************************************/
IPPolygonStruct *GMPolyHierarchy2SimplePoly(IPPolygonStruct *Root,
					    IPPolygonStruct *Islands)
{
    int i;
    IrtRType m;
    IPVertexStruct *V;
    IPPolygonStruct *Island, *MaxIsland;

    Root = IPCopyPolygon(Root);
    Islands = IPCopyPolygonList(Islands);

    /* Make sure lists are circular. */
    V = IPGetLastVrtx(Root -> PVertex);
    if (V -> Pnext == NULL)
	V -> Pnext = Root -> PVertex;
    for (Island = Islands; Island != NULL; Island = Island -> Pnext) {
        V = IPGetLastVrtx(Island -> PVertex);
	if (V -> Pnext == NULL)
	    V -> Pnext = Island -> PVertex;
    }

    for (Island = Islands; Island != NULL; Island = Island -> Pnext) {
        /* Compute the maximum X value for each interior islands. */
        V = Island -> PVertex;
	m = -IRIT_INFNTY;
        do {
	    if (m < V -> Coord[0])
	        m = V -> Coord[0];
	    V = V -> Pnext;
	}
	while (V != Island -> PVertex);
	Island -> BBox[1][0] = m;
    }

    while (Islands != NULL) {
        IPVertexStruct *FirstInterV, *VTmp1, *VTmp2, *MaxV2,
	    *MaxV = NULL;
	IrtRType FirstInterP;

        /* Fetch the island with the maximum X so far. */
        m = -IRIT_INFNTY;
	MaxIsland = NULL;
        for (Island = Islands; Island != NULL; Island = Island -> Pnext) {
	    if (m < Island -> BBox[1][0]) {
	        m = Island -> BBox[1][0];
		MaxIsland = Island;
	    }
	}
	assert(MaxIsland != NULL);

	if (MaxIsland == Islands) {
	    Islands = Islands -> Pnext;
	}
	else {
	    for (Island = Islands; Island != NULL; Island = Island -> Pnext) {
	        if (Island -> Pnext == MaxIsland)
		    break;
	    }
	    assert (Island != NULL);
	    Island -> Pnext = MaxIsland -> Pnext;
	}
	MaxIsland -> Pnext = NULL;

	/* Connect this max-X island to the outer root loop, at the maximal */
	/* X locations.							    */
	V = MaxIsland -> PVertex;
	m = -IRIT_INFNTY;
        do {
	    if (m < V -> Coord[0]) {
	        m = V -> Coord[0];
		MaxV = V;
	    }
	    V = V -> Pnext;
	}
	while (V != MaxIsland -> PVertex);

	if (GMPolygonRayInter2(Root, MaxV -> Coord, 0,
			       &FirstInterV, &FirstInterP) == 0) {
	    IPFreePolygon(Root);
	    IPFreePolygon(MaxIsland);
	    IPFreePolygonList(Islands);
	    GEOM_FATAL_ERROR(GEOM_ERR_INVALID_POLYGON);
	    return NULL;
	}

	/* Merge Root and Island into one large simple polygon.             */
	/* Create a double point at the ray intersection.  Connect VTmp1 to */
	/* Island and connect Island back to VTmp2.			    */
	VTmp1 = IPCopyVertex(FirstInterV);
	IRIT_PT_BLEND(VTmp1 -> Coord, FirstInterV -> Coord,
		      FirstInterV -> Pnext -> Coord, FirstInterP);
	VTmp1 -> Pnext = FirstInterV -> Pnext;
	FirstInterV -> Pnext = VTmp1;

	VTmp2 = IPCopyVertex(VTmp1);
	VTmp2 -> Pnext = VTmp1 -> Pnext;
	VTmp1 -> Pnext = VTmp2;

	MaxV2 = IPCopyVertex(MaxV); /* Make a double point in Island Max-X. */
	MaxV2 -> Pnext = MaxV -> Pnext;
	MaxV -> Pnext = MaxV2;

	/* COnnect Root and island. */
	VTmp1 -> Pnext = MaxV2;
	IP_SET_INTERNAL_VRTX(VTmp1);
	MaxV -> Pnext = VTmp2;
	IP_SET_INTERNAL_VRTX(MaxV);
    }

    /* If output should not be circular, make sure it is not. */
    i = IPSetPolyListCirc(FALSE);
    IPSetPolyListCirc(i);
    if (!i)
        IPGetLastVrtx(Root -> PVertex) -> Pnext = NULL;

    return Root;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Barycentric coordinates of given point Pt with respect to   M
* given Triangle Pt1 Pt2 Pt3. All points are assumed to be in the XY plane.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:   Three points forming a triangular in general position.  M
*   Pt:		     A point for which the barycentric coordinates are to be M
*		     computed.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType *: A pointer to a static space holding the three Barycentric    M
*	coefficients, or NULL if point Pt is outside the triangle	     M
*	Pt1 Pt2 Pt3.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBaryCentric3Pts                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBaryCentric3Pts2D				                             M
*****************************************************************************/
IrtRType *GMBaryCentric3Pts2D(const IrtPtType Pt1,
			      const IrtPtType Pt2,
			      const IrtPtType Pt3,
			      const IrtPtType Pt)
{
    IRIT_STATIC_DATA IrtVecType RetVal;
    IrtVecType PtV1, PtV2, PtV3;
    IrtRType R, W1, W2, W3;

    IRIT_PT2D_SUB(PtV1, Pt, Pt1);
    IRIT_PT2D_SUB(PtV2, Pt, Pt2);
    IRIT_PT2D_SUB(PtV3, Pt, Pt3);

    W1 = IRIT_CROSS_PROD_2D(PtV2, PtV3);
    W2 = IRIT_CROSS_PROD_2D(PtV3, PtV1);
    W3 = IRIT_CROSS_PROD_2D(PtV1, PtV2);

    if (W1 * W2 < -GMBasicEps ||
	W2 * W3 < -GMBasicEps ||
	W3 * W1 < -GMBasicEps)
	return NULL;		   /* Pt is out of the triangle Pt1 Pt2 Pt3. */

    RetVal[0] = IRIT_FABS(W1);
    RetVal[1] = IRIT_FABS(W2);
    RetVal[2] = IRIT_FABS(W3);

    if ((R = RetVal[0] + RetVal[1] + RetVal[2]) > 0.0) {
	R = 1.0 / R;
	IRIT_PT_SCALE(RetVal, R);
    }
    else {
        assert(0);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Barycentric coordinates of given point Pt with respect to   M
* given Triangle Pt1 Pt2 Pt3. All points are assumed to be coplanar.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:   Three points forming a triangular in general position.  M
*   Pt:		     A point for which the barycentric coordinates are to be M
*		     computed.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType *: A pointer to a static space holding the three Barycentric    M
*	coefficients, or NULL if point Pt is outside the triangle	     M
*	Pt1 Pt2 Pt3.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBaryCentric3Pts2D                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBaryCentric3Pts				                             M
*****************************************************************************/
IrtRType *GMBaryCentric3Pts(const IrtPtType Pt1,
			    const IrtPtType Pt2,
			    const IrtPtType Pt3,
			    const IrtPtType Pt)
{
    IRIT_STATIC_DATA IrtVecType RetVal;
    IrtVecType PtV1, PtV2, PtV3, W1, W2, W3;
    IrtRType R;

    IRIT_PT_SUB(PtV1, Pt, Pt1);
    IRIT_PT_SUB(PtV2, Pt, Pt2);
    IRIT_PT_SUB(PtV3, Pt, Pt3);

    IRIT_CROSS_PROD(W1, PtV2, PtV3);
    IRIT_CROSS_PROD(W2, PtV3, PtV1);
    IRIT_CROSS_PROD(W3, PtV1, PtV2);

    if (IRIT_DOT_PROD(W1, W2) < -GMBasicEps ||
	IRIT_DOT_PROD(W2, W3) < -GMBasicEps ||
	IRIT_DOT_PROD(W3, W1) < -GMBasicEps)
	return NULL;		   /* Pt is out of the triangle Pt1 Pt2 Pt3. */

    RetVal[0] = sqrt(IRIT_DOT_PROD(W1, W1));
    RetVal[1] = sqrt(IRIT_DOT_PROD(W2, W2));
    RetVal[2] = sqrt(IRIT_DOT_PROD(W3, W3));

    if ((R = RetVal[0] + RetVal[1] + RetVal[2]) > 0.0) {
	R = 1.0 / R;
	IRIT_PT_SCALE(RetVal, R);
    }
    else {
        assert(0);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds the two intersection points of the given two planar circles.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center1, Radius1:  Geometry of first circle.                             M
*   Center2, Radius2:  Geometry of second circle.                            M
*   Inter1, Inter2:    Where the two intersection locations will be placed.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE for successful computation, FALSE for failure.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc3D, GMCircleFrom3Points,     M
*   GMCircleFrom2Pts2Tans, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM2PointsFromCircCirc, circle circle intersection                        M
*****************************************************************************/
int GM2PointsFromCircCirc(const IrtPtType Center1,
			  IrtRType Radius1,
			  const IrtPtType Center2,
			  IrtRType Radius2,
			  IrtPtType Inter1,
			  IrtPtType Inter2)
{
    int RetVal = TRUE;
    IrtRType A, B, C, Delta,
	a = Center2[0] - Center1[0],
	b = Center2[1] - Center1[1],
	c = (IRIT_SQR(Radius1) - IRIT_SQR(Radius2) +
	     IRIT_SQR(Center2[0]) - IRIT_SQR(Center1[0]) +
	     IRIT_SQR(Center2[1]) - IRIT_SQR(Center1[1])) * 0.5;

    if (IRIT_PT_APX_EQ(Center1, Center2)) {
	Inter1[0] = Inter2[0] = Radius1;
	Inter1[1] = Inter2[1] = 0.0;
    }
    else if (IRIT_FABS(a) > IRIT_FABS(b)) {
	/* Solve for Y first. */
	A = IRIT_SQR(b) / IRIT_SQR(a) + 1;
	B = 2 * (b * Center1[0] / a - b * c / IRIT_SQR(a) - Center1[1]);
	C = IRIT_SQR(c / a) + IRIT_SQR(Center1[0]) + IRIT_SQR(Center1[1])
				-2 * c * Center1[0] / a - IRIT_SQR(Radius1);
	Delta = IRIT_SQR(B) - 4 * A * C;
	if (Delta < 0) { /* If no solution, do something almost reasonable. */
	    RetVal = FALSE;
	    Delta = 0;
	}
	Inter1[1] = (-B + sqrt(Delta)) / (2 * A);
	Inter2[1] = (-B - sqrt(Delta)) / (2 * A);

	Inter1[0] = (c - b * Inter1[1]) / a;
	Inter2[0] = (c - b * Inter2[1]) / a;
    }
    else {
	/* Solve for X first. */
	A = IRIT_SQR(a) / IRIT_SQR(b) + 1;
	B = 2 * (a * Center1[1] / b - a * c / IRIT_SQR(b) - Center1[0]);
	C = IRIT_SQR(c / b) + IRIT_SQR(Center1[0]) + IRIT_SQR(Center1[1])
				-2 * c * Center1[1] / b - IRIT_SQR(Radius1);
	Delta = IRIT_SQR(B) - 4 * A * C;
	if (Delta < 0) { /* If no solution, do something almost reasonable. */
	    RetVal = FALSE;
	    Delta = 0;
	}
	Inter1[0] = (-B + sqrt(Delta)) / (2 * A);
	Inter2[0] = (-B - sqrt(Delta)) / (2 * A);

	Inter1[1] = (c - a * Inter1[0]) / b;
	Inter2[1] = (c - a * Inter2[0]) / b;
    }

    Inter1[2] = Inter2[2] = 0.0;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the intersection of two circles in general position in R^3.      M
* The circles are centered at Cntr1/2 in a plane normal to Nrml1/2 and have  M
* a radius of Rad1/2.  The upto two intersections are returned in Inter1/2.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Cntr1, Nrml1, Rad1:    Center, normal and radius of first circle.        M
*   Cntr2, Nrml2, Rad2:    Center, normal and radius of second circle.       M
*   Inter1:          First intersection location in E3.                      M
*   Inter2:          Second intersection location in E3.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         Number of intersections found - 0, 1, or 2.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc, GMCircleFrom3Points,       M
*   GMCircleFrom2Pts2Tans, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM2PointsFromCircCirc3D                                                  M
*****************************************************************************/
int GM2PointsFromCircCirc3D(const IrtPtType Cntr1,
			    const IrtVecType Nrml1,
			    IrtRType Rad1,
			    const IrtPtType Cntr2,
			    const IrtVecType Nrml2,
			    IrtRType Rad2,
			    IrtPtType Inter1,
			    IrtPtType Inter2)
{
    IrtHmgnMatType Mat, InvMat;
    IrtPtType Cntr1t, Cntr2t;
    IrtVecType Nrml1t, Nrml2t;

    /* Bring Nrml1 to be the Z axis. */
    GMGenMatrixZ2Dir(Mat, Nrml1);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    MatMultPtby4by4(Cntr1t, Cntr1, InvMat);
    MatMultPtby4by4(Cntr2t, Cntr2, InvMat);
    MatMultVecby4by4(Nrml1t, Nrml1, InvMat);
    MatMultVecby4by4(Nrml2t, Nrml2, InvMat);

    if (IRIT_PT_APX_EQ(Nrml1, Nrml2)) {
	/* It is a 2D problem. Use GM2PointsFromCircCirc. */
	if (GM2PointsFromCircCirc(Cntr1, Rad1, Cntr2, Rad2, Inter1, Inter2))
	    return 2;
	else
	    return 0;
    }
    else {
        IrtRType a, h;
        IrtPtType P1, ClosestPoint;
        IrtVecType V1, V2;
        IrtPlnType Plane2;

	/* It is a 3D problem. Circle one is XY parallel at level Cntr1t[2]. */

        /* Find the intersection of the Plane 'Z = Cntr1t[2]' with second    */
	/* circle.							     */
        IRIT_VEC_COPY(Plane2, Nrml2t);
	Plane2[3] = -IRIT_DOT_PROD(Cntr2t, Nrml2t);

	/* Substituting 'Z = Cntr1t[2]' into the plane euqation, we can      */
	/* derive the vector of the line via the perpendicular to the vector */
	/* and then find a point on that line.				     */
	V1[0] = Plane2[0];			/* The line's normal vector. */
	V1[1] = Plane2[1];
	V1[2] = 0.0;

	V2[0] = V1[1];				       /* The line's vector. */
	V2[1] = -V1[0];
	V2[2] = 0.0;

	if (IRIT_FABS(Plane2[0]) > IRIT_FABS(Plane2[1])) {
	    /* Assume Y is zero and find X as point on line: */
	    P1[0] = -(Plane2[2] * Cntr1t[2] + Plane2[3]) / Plane2[0];
	    P1[1] = 0.0;
	    P1[2] = Cntr1t[2];
	}
	else {
	    /* Assume X is zero and find Y as point on line: */
	    P1[0] = 0.0;
	    P1[1] = -(Plane2[2] * Cntr1t[2] + Plane2[3]) / Plane2[1];
	    P1[2] = Cntr1t[2];
	}

	/* Find the closest point to Cntr1t on Line "P1 + V2 t" and apply    */
	/* some pythagorian relations to derive the final solution.	     */
	GMPointFromPointLine(Cntr1t, P1, V2, ClosestPoint);
	a = IRIT_PT_PT_DIST(Cntr1t, ClosestPoint);
	if (IRIT_FABS(Rad1) < IRIT_FABS(a))
	    return 0;
	h = sqrt(IRIT_SQR(Rad1) - IRIT_SQR(a));

	/* Compute the solutions as "ClosestPoint +/- h V2": */
	IRIT_VEC_NORMALIZE(V2);
	IRIT_VEC_SCALE(V2, h);
	IRIT_PT_ADD(Inter1, ClosestPoint, V2);
	IRIT_PT_SUB(Inter2, ClosestPoint, V2);
    }

    /* Map the solutions back to their original position. */

    MatMultPtby4by4(Inter1, Inter1, Mat);
    MatMultPtby4by4(Inter2, Inter2, Mat);

    return IRIT_PT_APX_EQ(Inter1, Inter2) ? 1 : 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to construct a circle through given 3 points. If two of the      M
* points are the same or the three points are collinear it returns FALSE,    M
* otherwise (successful), it returns TRUE.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:         Of computed circle.                                      M
*   Radius:	    Of computed circle.                                      M
*   Pt1, Pt2, Pt3:  Three points to fit a circle through.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromCircCirc3D, GM2PointsFromLineLine, GM2PointsFromCircCirc,   M
*   GMCircleFrom2Pts2Tans, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCircleFrom3Points, circle                                              M
*****************************************************************************/
int GMCircleFrom3Points(IrtPtType Center,
			IrtRType *Radius,
			const IrtPtType Pt1,
			const IrtPtType Pt2,
			const IrtPtType Pt3)
{
    int RetVal;
    IrtRType t1, t2;
    IrtPtType Pl1, Pl2, PInter1, PInter2;
    IrtVecType Vl1, Vl2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMCircleFrom3Points entered\n");
#endif /* DEBUG */

    if (GMCollinear3Pts(Pt1, Pt2, Pt3))
	return FALSE;

    IRIT_PT_SUB(Vl1, Pt2, Pt1);
    IRIT_SWAP(IrtRType, Vl1[0], Vl1[1]);
    Vl1[1] = -Vl1[1];
    IRIT_PT_SUB(Vl2, Pt3, Pt2);
    IRIT_SWAP(IrtRType, Vl2[0], Vl2[1]);
    Vl2[1] = -Vl2[1];

    IRIT_PT_BLEND(Pl1, Pt1, Pt2, 0.5);
    IRIT_PT_BLEND(Pl2, Pt2, Pt3, 0.5);

    RetVal = GM2PointsFromLineLine(Pl1, Vl1, Pl2, Vl2,
				   PInter1, &t1, PInter2, &t2);
    IRIT_PT_BLEND(Center, PInter1, PInter2, 0.5);
    *Radius = sqrt(IRIT_SQR(Center[0] - Pt1[0]) +
		   IRIT_SQR(Center[1] - Pt1[1]));

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMCircleFrom3Points exit\n");
#endif /* DEBUG */

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to construct a circle through given 3 points. If two of the      M
* points are the same or the three points are collinear it returns FALSE,    M
* otherwise (successful), it returns TRUE.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:      Of computed circle.                                         M
*   Radius:	 Of computed circle.                                         M
*   Pt1, Pt2:    Two points to fit a circle through.                         M
*   Tan1, Tan2:  Two tangents to the circle at Pt1, Pt2.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromCircCirc3D, GM2PointsFromLineLine, GM2PointsFromCircCirc,   M
*   GMCircleFrom3Points, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,     M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCircleFrom2Pts2Tans, circle                                            M
*****************************************************************************/
int GMCircleFrom2Pts2Tans(IrtPtType Center,
			  IrtRType *Radius,
			  const IrtPtType Pt1,
			  const IrtPtType Pt2,
			  const IrtVecType Tan1,
			  const IrtVecType Tan2)
{
    int RetVal;
    IrtRType t1, t2;
    IrtPtType PInter1, PInter2;
    IrtVecType Nrml1, Nrml2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMCircleFrom2Pts2Tans entered\n");
#endif /* DEBUG */

    if (IRIT_PT_APX_EQ(Pt1, Pt2))
	return FALSE;

    Nrml1[0] = Tan1[1];
    Nrml1[1] = -Tan1[0];
    Nrml1[2] = 0.0;
    Nrml2[0] = Tan2[1];
    Nrml2[1] = -Tan2[0];
    Nrml2[2] = 0.0;

    RetVal = GM2PointsFromLineLine(Pt1, Nrml1, Pt2, Nrml2,
				   PInter1, &t1, PInter2, &t2);
    IRIT_PT_BLEND(Center, PInter1, PInter2, 0.5);
    *Radius = sqrt(IRIT_SQR(Center[0] - Pt1[0]) +
		   IRIT_SQR(Center[1] - Pt1[1]));

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugGMEntryExit)
        IRIT_INFO_MSG("GMCircleFrom2Pts2Tans exit\n");
#endif /* DEBUG */

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute a least-sqaures fitted circle to a set of points, in the XY      M
* plane.  Returns the center and radius of the computed circle.              M
*   Solving the problem in a new (U,V) apace by minimizing P^2,              M
* P=(U-Uc)^2+(V-Vc)^2-R^2, where (Uc,Vc) the circle center.		     M
*   For the full solution, see						     M
* http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:    The circle's center is returned in this param.		     M
*   Radius:    The circle's radius is returned in this param.		     M
*   Pts:       Vector of points to fit a circle to, least squares.           M
*   PtsSize:   Size of the PTs vector.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCircleFromLstSqrPts                                                    M
*****************************************************************************/
int GMCircleFromLstSqrPts(CagdPType Center,
			  IrtRType *Radius,
			  const CagdPType *Pts,
			  int PtsSize)
{
    int i;
    IrtRType 
	Xh = 0.0,
        Yh = 0.0,
        Suu = 0.0,
        Svv = 0.0, 
        Suv = 0.0,
        Suuu = 0.0, 
	Svvv = 0.0,
        Suvv = 0.0,
        Svuu = 0.0;
    CagdPType *TPts;
    
    if (PtsSize < 4)
	return FALSE;

    TPts = IritMalloc(sizeof(CagdPType) * PtsSize);

    for (i = 0; i < PtsSize; i++) {
	Xh += Pts[i][0];
	Yh += Pts[i][1];
    }
	
    Xh /= PtsSize;
    Yh /= PtsSize;
	
    /* Converting the points to (U,V) coords,                               */
    /* Ui = Xi - (average of X cords), and Vi = Yi - (average of Y cords).  */
    for (i = 0; i < PtsSize; i++) {
	TPts[i][0] = Pts[i][0] - Xh;
	TPts[i][1] = Pts[i][1] - Yh;
	TPts[i][2] = 0.0;
    }
	
    /* Su is sum of Ui, Suu is sum of Ui^2, Suv is sum of Ui*Vi and etc...  */
    for (i = 0; i < PtsSize; i++) {
	Suu += IRIT_SQR(TPts[i][0]);
	Svv += IRIT_SQR(TPts[i][1]);
	Suv += TPts[i][0] * TPts[i][1];
	Suuu += IRIT_CUBE(TPts[i][0]);
	Svvv += IRIT_CUBE(TPts[i][1]);
	Suvv += TPts[i][0] * IRIT_SQR(TPts[i][1]);
	Svuu += TPts[i][1] * IRIT_SQR(TPts[i][0]);	
    }
    
    /*   The center equation is the result of solving for zero of two eqns, */
    /* equation 1-differentiation by Uc, equation 2-differentiation by Vc,  */
    /* of the equation P=(U-Uc)^2+(V-Vc)^2-R^2.				    */
    if (IRIT_FABS(Suu) > IRIT_UEPS) {
	Center[1] = 0.5 * ((Svvv + Svuu) - (Suv / Suu) * (Suuu + Suvv)) / 
		                                  (Svv - IRIT_SQR(Suv) / Suu);
	Center[0] = (0.5 * (Suuu + Suvv) - Center[1] * Suv) / Suu;
    }
    else {
	assert(Suv != 0);
	Center[1] = 0.5 * (Suuu + Suvv) / Suv;
	Center[0] = (0.5 * (Svvv + Svuu) - 0.5 * 
		                             (Suuu + Suvv) * Svv / Suv) / Suv;
    }
    Center[2] = 0.0;
    *Radius = sqrt(IRIT_SQR(Center[0]) + IRIT_SQR(Center[1]) + 
				                       (Suu + Svv) / PtsSize);

    /* Return back the circle center to (X,Y) coordinates. */
    IRIT_PT_SET(Center, Center[0] + Xh, Center[1] + Yh, 0.0);

    IritFree(TPts);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds the two pairs of tangent points of the given two planar circles.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center1, Radius1:  Geometry of first circle.                             M
*   Center2, Radius2:  Geometry of second circle.                            M
*   OuterTans:	       TRUE for outer two tangents, FALSE for inner two.     M
*   TanPts:            The two tangents designated by the end points of the  M
*		       Segments.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE for successful computation, FALSE for failure or no such     M
*	   bitangents exist for the current configuration.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc3D, GMCircleFrom3Points,     M
*   GMCircleFrom2Pts2Tans, GM2PointsFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM2BiTansFromCircCirc, circle circle tangenties                          M
*****************************************************************************/
int GM2BiTansFromCircCirc(const IrtPtType Center1,
			  IrtRType Radius1,
			  const IrtPtType Center2,
			  IrtRType Radius2,
			  int OuterTans,
			  IrtPtType TanPts[2][2])
{
    int i;
    IrtLnType TanLines[2];
    IrtPtType LnPt;
    IrtVecType LnDir;

    if (!GM2TanLinesFromCircCirc(Center1, Radius1, Center2, Radius2,
				 OuterTans, TanLines))
	return FALSE;

    /* Find tangency locations of lines and the two circles. */
    for (i = 0; i < 2; i++) {
        GMPointVecFromLine(TanLines[i], LnPt, LnDir);

	GMPointFromPointLine(Center1, LnPt, LnDir, TanPts[i][0]);
	GMPointFromPointLine(Center2, LnPt, LnDir, TanPts[i][1]);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds the two tangent lines to the given two planar circles.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Center1, Radius1:  Geometry of first circle.                             M
*   Center2, Radius2:  Geometry of second circle.                            M
*   OuterTans:	       TRUE for outer two tangents, FALSE for inner two.     M
*   Tans:              The two tangent lines designated by line equatios.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE for successful computation, FALSE for failure or no such     M
*	   bitangents exist for the current configuration.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc3D, GMCircleFrom3Points,     M
*   GMCircleFrom2Pts2Tans, GM2PointsFromCircCirc, GM2BiTansFromCircCirc,     M
*   GM2IsPtInsideCirc							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM2TanLinesFromCircCirc, circle circle tangenties                        M
*****************************************************************************/
int GM2TanLinesFromCircCirc(const IrtPtType Center1,
			    IrtRType Radius1,
			    const IrtPtType Center2,
			    IrtRType Radius2,
			    int OuterTans,
			    IrtLnType Tans[2])
{
    int i, n, SwapXY;
    IrtRType A, B, C, Sols[2];
    IrtPtType Cntr1, Cntr2;

    IRIT_PT_COPY(Cntr1, Center1);
    IRIT_PT_COPY(Cntr2, Center2);

    if (IRIT_PT_PT_DIST(Cntr1, Cntr2) < IRIT_FABS(Radius1 - Radius2))
	return FALSE;				 /* No bitangentices. */

    if (OuterTans) {
	Radius1 = IRIT_FABS(Radius1);  /* Both should be positive. */
	Radius2 = IRIT_FABS(Radius2);
    }
    else {
	Radius1 = -IRIT_FABS(Radius1);  /* Different signs! */
	Radius2 = IRIT_FABS(Radius2);
    }

    if (IRIT_FABS(Cntr1[0] - Cntr2[0]) < IRIT_FABS(Cntr1[1] - Cntr2[1])) {
	/* Exchange the role of X and Y as we divide by Dx. */
	IRIT_SWAP(IrtRType, Cntr1[0], Cntr1[1]);
	IRIT_SWAP(IrtRType, Cntr2[0], Cntr2[1]);
	SwapXY = TRUE;
    }
    else
	SwapXY = FALSE;

    /* Build the coefficients of the quadratic equation and solve. */
    A = IRIT_SQR(Cntr1[0] - Cntr2[0]) + IRIT_SQR(Cntr1[1] - Cntr2[1]);
    B = 2 * (Radius1 * (Cntr2[1] - Cntr1[1]) -
	     Radius2 * (Cntr2[1] - Cntr1[1]));
    C = IRIT_SQR(Radius1 - Radius2) - IRIT_SQR(Cntr1[0] - Cntr2[0]);

    if (A != 0)
        n = GMSolveQuadraticEqn(B / A, C / A, Sols);
    else
	n = 0;

    if (n < 2)
	return FALSE;

    for (i = 0; i < 2; i++) {
       Tans[i][0] = (Sols[i] * (Cntr2[1] - Cntr1[1]) +
		      (Radius1 - Radius2)) / (Cntr1[0] - Cntr2[0]);
       Tans[i][1] = Sols[i];
       Tans[i][2] = (Cntr2[0] * (Sols[i] * Cntr1[1] - Radius1) -
		     Cntr1[0] * (Sols[i] * Cntr2[1] - Radius2))
			                     / (Cntr1[0] - Cntr2[0]);
    }

    if (SwapXY) {
	/* Exchange the role of X and Y back. */
	IRIT_SWAP(IrtRType, Tans[0][0], Tans[0][1]);
	IRIT_SWAP(IrtRType, Tans[1][0], Tans[1][1]);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if a point is contained in the given prescribed circle in XY	     M
* plane.  Points on the circle are not considered inside (open domain).	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         Point to test for containment in the circle in XY plane.  M
*   Center:        Center of the circle to test against.                     M
*   Radius:        Radius of the circle to test against.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if Point is indeed inside the circle, FALSE otherwise.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc3D, GMCircleFrom3Points,     M
*   GMCircleFrom2Pts2Tans, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2PointsFromCircCirc, GMIsPtOnCirc					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsPtInsideCirc                                                         M
*****************************************************************************/
int GMIsPtInsideCirc(const IrtRType *Point,
		     const IrtRType *Center,
		     IrtRType Radius)
{
    return IRIT_PT2D_DIST_SQR(Point, Center) - IRIT_SQR(Radius) <
                                                               -GMBasicEpsSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if a point is on the given prescribed circle's boundary, in the    N
* XY plane.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:      Point to test for inclusing in circle boundary in XY plane.  M
*   Center:     Center of the circle to test against.                        M
*   Radius:     Radius of the circle to test against.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if Point is indeed on the circle, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM2PointsFromLineLine, GM2PointsFromCircCirc3D, GMCircleFrom3Points,     M
*   GMCircleFrom2Pts2Tans, GM2BiTansFromCircCirc, GM2TanLinesFromCircCirc,   M
*   GM2PointsFromCircCirc, GMIsPtInsideCirc				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsPtOnCirc                                                             M
*****************************************************************************/
int GMIsPtOnCirc(const IrtRType *Point,
		 const IrtRType *Center,
		 IrtRType Radius)
{
    return IRIT_FABS(IRIT_PT2D_DIST_SQR(Point, Center) - IRIT_SQR(Radius)) <=
								GMBasicEpsSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computing the Area of the triangle formed by points Pt1, Pt2 and Pt3.    M
*   This can be used as a test to identify whether point Pt3 lies to the     M
* left, right, or on the line (vector) formed by Pt1 and Pt2 using the 	     M
* this signed area of the triangle's computation.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3: Points to compute the area of a triangle formed by Pt1,   M
*                  Pt2, and Pt3, in the XY plane.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:      Resulting area.		                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAreaOfTriangle                                                         M
*****************************************************************************/
IrtRType GMAreaOfTriangle(const IrtRType *Pt1,
			  const IrtRType *Pt2,
			  const IrtRType *Pt3)
{
    return ((Pt2[0] - Pt1[0]) * (Pt3[1] - Pt1[1]) - 
	    (Pt3[0] - Pt1[0]) * (Pt2[1] - Pt1[1]));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the solutions, if any, of the given quadratic equation.  Only   M
* real solutions are considered.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B:    The equation's coefficients as x^2 + A x + B = 0.               M
*   Sols:    Where to place the solutions.  At most two.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Number of real solutions.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSolveCubicEqn, GMSolveCubicEqn2, GMSolveQuadraticEqn2                  M
*   GMSolveQuarticEqn							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSolveQuadraticEqn                                                      M
*****************************************************************************/
int GMSolveQuadraticEqn(IrtRType A, IrtRType B, IrtRType *Sols)
{
    IrtRType
        d = IRIT_SQR(A) - 4.0 * B;

    if (d < 0.0)
	return 0;
    else if (d == 0.0) {
	Sols[0] = -0.5 * A;
	return 1;
    }
    else {
	d = sqrt(d);

	Sols[0] = (-A - d) * 0.5;
	Sols[1] = (-A + d) * 0.5;
	return 2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the two square roots of the quadratic equation:               M
*     x^2 + Bx + C = 0                                      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   B, C:         The coefficients of the quadratic polynomial.		     M
*   RSols, ISols: Solutions such that each pair RSols[i], ISols[i] is the    M
*		  complex root(i).					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        The number of REAL solutions of the polynomial.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSolveCubicEqn, GMSolveCubicEqn2, GMSolveQuadraticEqn                   M
*   GMSolveQuarticEqn							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSolveQuadraticEqn2                                                     M
*****************************************************************************/
int GMSolveQuadraticEqn2(IrtRType B,
			 IrtRType C,
			 IrtRType *RSols,
			 IrtRType *ISols)
{
    int n;
    IrtRType
	D = IRIT_SQR(B) - 4 * C;

    if (D >= 0) {	/* 2 real solutions. */
	D = sqrt(D);
	RSols[0] = (-B + D) * 0.5;
	RSols[1] = (-B - D) * 0.5;

	ISols[0] = ISols[1] = 0.0;

	n = 2;
    }
    else {		/* Two conjugate complex solutions. */
	D = sqrt(-D);
	RSols[0] = -B * 0.5;
	ISols[0] = -D * 0.5;

	RSols[1] = -B * 0.5;
	ISols[1] = +D * 0.5;

	n = 0;
    }

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function for the function GMSolveCubicEqn.		     *
*   Given a real solution found analytically but not precise: F(x) != 0,     *
* this function enhances the error of F(x) using Newton's method.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Root:        The real (analytic) solution of the cubic polynomial.       *
*   A, B, C:     The coefficients of the cubic polynomial.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   The most accurate rounded solution function could derive.    *
*****************************************************************************/
static IrtRType GMSolveCubicApplyNewton(IrtRType Root,
					IrtRType A,
					IrtRType B,
					IrtRType C)
{
    int i;
    IrtRType
	EvalRoot,	/* f(Root) - Original value to be improved. */
        EvalX,		/* f(x) = x^3 + Ax^2 + Bx^ + C */
        Evaldx,		/* f'(x)		*/
	EvalF,		/* F = f^2(x)		*/
        EvalFdx,	/* F'(x)		*/
        EvalFd2x,	/* F''(x)		*/
        x, NewX;

    /* Init loop, and don't loop too much because cucles are expected. */
    NewX = Root;
    i = 0;

    EvalX = EvalRoot = C + Root * (B + Root * (A + Root));
    if (IRIT_FABS(EvalX) < GM_QUARTIC_SOLVE_EPS)/* Avoid precision problems. */
        return Root;

    do {
	x = NewX;
	Evaldx = B + x * (2 * A + 3 * x);

	EvalF = IRIT_SQR(EvalX);
	EvalFdx = 2 * EvalX * Evaldx; /* F'(x) = 2f(x)f'(x) */
	EvalFd2x = 2 * (Evaldx * Evaldx + EvalX * (2 * A + 6 * x));

	/* Newton: NewX = x - EvalF/EvalFdx + second order term; */
	NewX = x - (EvalF / EvalFdx) * (1 + (EvalF * EvalFd2x) /
					    (2 * IRIT_SQR(EvalFdx)));
	EvalX = C + NewX * (B + NewX * (A + NewX));
	if (IRIT_FABS(EvalX) < GM_QUARTIC_SOLVE_EPS)
	    break;

	i++;
    }
    while (NewX != x &&
	   i <= GM_SOLVER_MAX_NEWTON_ITER &&
	   IRIT_FABS(EvalX) < IRIT_FABS(EvalRoot));

    return IRIT_FABS(EvalX) >= IRIT_FABS(EvalRoot) ? Root : NewX;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the solutions, if any, of the given cubic equation.  Only real  M
* solutions are considered.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C:   The equation's coefficients as x^3 + A x^2 + B x + C = 0.     M
*   Sols:      Where to place the solutions.  At most three.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Number of real solutions.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSolveQuadraticEqn, GMSolveQuadraticEqn2, GMSolveCubicEqn2,             M
*   GMSolveQuarticEqn							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSolveCubicEqn                                                          M
*****************************************************************************/
int GMSolveCubicEqn(IrtRType A, IrtRType B, IrtRType C, IrtRType *Sols)
{
    int i, n;
    IrtRType D,
	Q = (3.0 * B - IRIT_SQR(A)) / 9.0,
	R = (9.0 * A * B - 27.0 * C - 2.0 * IRIT_CUBE(A)) / 54.0,
	Q3 = IRIT_CUBE(Q),
        R2 = IRIT_SQR(R);

    if (IRIT_APX_EQ_EPS(Q3, -R2, IRIT_UEPS))
        D = 0.0;
    else
        D = Q3 + R2;

    if (D == 0.0 && Q == 0.0 && R == 0.0) {       /* 3 equal real solution. */
	Sols[0] = Sols[1] = Sols[2] = GM_SQRT3(-C);

	n = 3;
    }
    else if (D <= 0.0) {		       /* We have 3 real solutions. */
        IrtRType
	    q = Q < 0.0 ? sqrt(-Q) : 0.0,
	    R1 = 2.0 * q,
	    R2 = A / 3.0,
	    t = R / IRIT_CUBE(q),
	    Theta = acos(IRIT_BOUND(t, -1.0, 1.0));

	Sols[0] = R1 * cos(Theta / 3.0) - R2;
	Sols[1] = R1 * cos((Theta + M_PI_MUL_2) / 3.0) - R2;
	Sols[2] = R1 * cos((Theta + 2.0 * M_PI_MUL_2) / 3.0) - R2;

        n = 3;
    }
    else {				       /* We have 1 real solutions. */
	IrtRType RD1, RD2;

	D = sqrt(D);
        RD1 = R - D;
        RD2 = R + D;

	Sols[0] = pow(IRIT_FABS(RD1), 1.0 / 3.0) * IRIT_SIGN(RD1) +
	          pow(IRIT_FABS(RD2), 1.0 / 3.0) * IRIT_SIGN(RD2) -
		  A / 3.0;

        n = 1;
    }

    /* Improve the solution.s */
    for (i = 0; i < n; i++)
        Sols[i] = GMSolveCubicApplyNewton(Sols[i], A, B, C);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugGMCubicSol, FALSE) {
	    int i;

	    for (i = 0; i < n; i++) {
	        IrtRType
		    x = Sols[i];

		IRIT_INFO_MSG_PRINTF("Sol = %f, (%d)\n",
				     x, (((x + A) * x) + B) * x + C);
	    }
	}
    }
#   endif /* DEBUG */

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the three roots (complex & real) of the cubic equation        M
*     x^3 + Ax^2 + Bx + C = 0						     M
*   Note: Cubic equations have at least one real root; this function always  M
* calculates the real root first, and the other two (possibly complex) later.M
* This order of filling RSols & ISols is CRUCIAL for GMSolveQuarticEqn()     M
*                                                                            *
* PARAMETERS:                                                                M
*   A, B, C:        The coefficients of the cubic polynomial.		     M
*   RSols, ISols:   Each pair (RSols[i], ISols[i]) is the complex root(i)    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     The number of REAL solutions of the cubic polynomial.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSolveCubicEqn, GMSolveQuadraticEqn, GMSolveQuadraticEqn2,              M
*   GMSolveQuarticEqn							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSolveCubicEqn2                                                         M
*****************************************************************************/
int GMSolveCubicEqn2(IrtRType A,
		     IrtRType B,
		     IrtRType C,
		     IrtRType *RSols,
		     IrtRType *ISols)
{
    int i, n;
    IrtRType D,
        Q = (3.0 * B - IRIT_SQR(A)) / 9.0,
        R = (9.0 * A * B - 27.0 * C - 2.0 * IRIT_CUBE(A)) / 54.0,
	Q3 = IRIT_CUBE(Q),
        R2 = IRIT_SQR(R);

    if (IRIT_APX_EQ_EPS(Q3, -R2, IRIT_UEPS))
        D = 0.0;
    else
        D = Q3 + R2;

    if (D == 0.0 && Q == 0.0 && R == 0.0) {       /* 3 equal real solution. */
	RSols[0] = RSols[1] = RSols[2] = GM_SQRT3(-C);
	ISols[0] = ISols[1] = ISols[2] = 0.0;

	n = 3;
    }
    else if (D <= 0.0) {	      /* We have 3 distinct real solutions. */
	IrtRType
	    q = Q < 0.0 ? sqrt(-Q) : 0.0,
	    R1 = 2.0 * q,
	    R2 = A / 3.0,
	    t = R / IRIT_CUBE(q),
	    Theta = acos(IRIT_BOUND(t, -1.0, 1.0));

	RSols[0] = R1 * cos(Theta / 3.0) - R2;
	RSols[1] = R1 * cos((Theta + M_PI_MUL_2) / 3.0) - R2;
	RSols[2] = R1 * cos((Theta + 2.0 * M_PI_MUL_2) / 3.0) - R2;

	ISols[0] = ISols[1] = ISols[2] = 0.0;

        n = 3;
    }
    else {		/* We have 1 real solution. */
	IrtRType RD1, RD2;

	D = sqrt(D);
	RD1 = R - D;
	RD2 = R + D;

	RSols[0] = GM_SQRT3(RD1) + GM_SQRT3(RD2) - A/3.0;
	ISols[0] = 0.0;

	/* Continue solving via dividing the cubic polynomial */ 
	n = GMSolveQuadraticEqn2(A + RSols[0], B + RSols[0] * (A + RSols[0]),  
				 RSols + 1, ISols + 1); 
	n += 1 ; /* counting RSols[0] */
    }

    /* Improve the real solutions. */
    for (i = 0; i < n; i++) {
        if (IRIT_APX_EQ_EPS(ISols[i], 0.0, GM_QUARTIC_SOLVE_EPS))
	    RSols[i] = GMSolveCubicApplyNewton(RSols[i], A, B, C);
    }

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function for the function GMSolveQuarticEqn.		     *
*   Given a real solution found analytically but not precise: F(x) != 0,     *
* this function enhances the error of F(x) using Newton's method.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Root:        The real (analytic) solution of the quartic polynomial.     *
*   A, B, C, D:  The coefficients of the quartic polynomial.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   The most accurate rounded solution function could derive.    *
*****************************************************************************/
static IrtRType GMSolveQuarticApplyNewton(IrtRType Root,
					  IrtRType A,
					  IrtRType B,
					  IrtRType C,
					  IrtRType D)
{
    int i;
    IrtRType
	EvalRoot,	/* f(Root) - Original value to be improved. */
        EvalX,		/* f(x) = x^4 + Ax^3 + Bx^2 + Cx + D */
        Evaldx,		/* f'(x)		*/
	EvalF,		/* F = f^2(x)		*/
        EvalFdx,	/* F'(x)		*/
        EvalFd2x,	/* F''(x)		*/
        x, NewX;

    /* Init loop, and don't loop too much because cycles are expected. */
    NewX = Root;
    i = 0;

    EvalX = EvalRoot = D + Root * (C + Root * (B + Root * (A + Root)));
    if (IRIT_FABS(EvalX) < GM_QUARTIC_SOLVE_EPS)/* Avoid precision problems. */
        return Root;

    do {
	x = NewX;
	Evaldx = C + x * (2 * B + x * (3 * A + 4 * x));

	EvalF = IRIT_SQR(EvalX);
	EvalFdx = 2 * EvalX * Evaldx; /* F'(x) = 2f(x)f'(x) */
	EvalFd2x = 2 * (Evaldx * Evaldx +
			EvalX * (2 * B + x * (6 * A + 12 * x)));

	/* Newton: NewX = x - EvalF/EvalFdx + second order term; */
	NewX = x - (EvalF / EvalFdx) * (1 + (EvalF * EvalFd2x) /
					    (2 * IRIT_SQR(EvalFdx)));
	EvalX = D + NewX * (C + NewX * (B + NewX * (A + NewX)));
	if (IRIT_FABS(EvalX) < GM_QUARTIC_SOLVE_EPS)
	    break;

	i++;
    }
    while (NewX != x &&
	   i <= GM_SOLVER_MAX_NEWTON_ITER &&
	   IRIT_FABS(EvalX) < IRIT_FABS(EvalRoot));

    return IRIT_FABS(EvalX) >= IRIT_FABS(EvalRoot) ? Root : NewX;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the (upto) four real roots of the quartic equation              M
*     x^4 + Ax^3 + Bx^2 + Cx + D = 0					     V
*									     M
* Note 1								     M
* ------								     M
* In order to avoid building a library for complex numbers arithmetics, two  M
* arrays are used ISols[] and RSols[], where each RSols[i] and ISols[i],     M
* represent a complex number, and so calculation where made on the fly;      M
*   Anyway, some of these calculations where performed in a specific way to  M
* reduce errors of double-precision nature. (especially calculating square   M
* roots of complex numbers).					             M
*									     M
* Note 2.						    		     M
* ----------								     M
* In the case of Cubic and Quadratic equations, the number of real solutions M
* is determined via the value of D (the descrimenant); As such, the number   M
* of real solutions is easier to depict.				     M
*   However, Euler's solution for quartic equations manipulates all the	     M
* three solutions of the cubic (real and complex), hoping to find some real  M
* roots by eliminating the imaginary part of the complex solutions.          M
*   This, however, is a weakness of dependency upon the accuracy of the	     M
* numbers' representation as a double-precision floating point.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   a, b, c, d:   The coefficients of the quartic polynomial.		     M
*   Sols:  	  The real roots of the polynomial.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         The number of REAL solutions of the polynomial.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSolveCubicEqn, GMSolveCubicEqn2,                                       M
*   GMSolveQuadraticEqn, GMSolveQuadraticEqn2			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSolveQuarticEqn                                                        M
*****************************************************************************/
int GMSolveQuarticEqn(IrtRType a,
		      IrtRType b,
		      IrtRType c,
		      IrtRType d,
		      IrtRType *Sols)
{
    int n = 0;
    IrtRType e, f, g,
	RSols[4] = { 0 },
	ISols[4] = { 0 };

    IRIT_ZAP_MEM(Sols, sizeof(IrtRType) * 4);

    /* Get a depressed quartic equation - no coefficient for x^3, as        */
    /* x^4 + e x^2 + f x + g = 0.					    */
    e = (b - 3.0 * IRIT_SQR(a) / 8.0);
    f = (c + IRIT_CUBE(a) / 8.0 - (a * b / 2.0));
    g = (d - (3.0 * GM_QUART(a) / 256.0) +
	                            (IRIT_SQR(a) * b / 16.0) - (a * c / 4.0));

    /* Zero is a solution. Solving a Cubic form (and subtracting a/4 from   */
    /* solutions.							    */
    if (g == 0) {                                         /* Includes d==0. */
        if (d == 0 || (f == 0.0 && e == 0.0)) {
	    Sols[0] = Sols[1] = Sols[2] = Sols[3] = -a * 0.25;
	    n = 4;
	}
	else {
	    int i = 0;

	    /* No coef for x^3. */
	    n = GMSolveCubicEqn2(0.0, e, f, RSols, ISols);

	    for (i = 0; i < n; i++)     /* Real roots are returned first... */
	        Sols[i] = RSols[i] - a * 0.25;

	    Sols[n] = -a * 0.25; /* Zero in the depressed quartic equation. */

	    n = n + 1;
	}
    }
    else if (f == 0) {               /* It's a quadratic equation Y^2 -> Y. */
	int i = GMSolveQuadraticEqn2(e, g, RSols, ISols);

	if (i > 0) {
	    IrtRType x;

	    n = 0;
	    if (RSols[0] >= 0) {
		x = sqrt(RSols[0]);
		Sols[0] =  x - a * 0.25;
		Sols[1] = -x - a * 0.25;
		n += 2;
	    }
	    if (RSols[1] >= 0) {
		x = sqrt(RSols[1]);
		Sols[n]      = x - a * 0.25;
		Sols[n + 1] = -x - a * 0.25;
		n += 2;
	    }
	}
    }
    else {						     /* d,f,g != 0. */
        int i;
	IrtRType p, q, r, ip, iq, ir, t, it;

	n = GMSolveCubicEqn2(e / 2.0, (IRIT_SQR(e) - (4.0 * g)) / 16.0,
			     -IRIT_SQR(f) / 64.0, RSols, ISols);

	/* Square roots of three real numbers (may be negative). */
	if (n == 3) {
	    /* p = sqrt(r[1]). */
	    if (RSols[1] >= 0) {
		p = sqrt(RSols[1]);
		ip = 0.0;
	    }
	    else {
		ip = sqrt(-RSols[1]);
		p = 0.0;
	    }

	    /* q = sqrt(r[2]). */
	    if (RSols[2] >= 0) {
		q = sqrt(RSols[2]);
		iq = 0.0;
	    }
	    else {
		iq = sqrt(-RSols[2]);
		q = 0.0;
	    }
	}
	else {       /* n = 1; performing complex square-root calculations, */
	    /* over the two complex roots.  Almost the same for q and p.    */
	    GMComplexRoot(RSols[1], ISols[1], &p, &ip);
	    GMComplexRoot(RSols[2], ISols[2], &q, &iq);
	}
	/* Compute r = -f / (8 p q). */
	t = p * q - ip * iq;
	it = p * iq + q * ip;
	r = -f * t / (8.0 * (IRIT_SQR(t) - IRIT_SQR(it)));
	ir = f * it / (8.0 * (IRIT_SQR(t) - IRIT_SQR(it)));

	/* Euler's solution for quartic equations. */
	RSols[0] =  p + q + r - a * 0.25;	ISols[0] =  ip + iq + ir;
	RSols[1] =  p - q - r - a * 0.25;	ISols[1] =  ip - iq - ir;
	RSols[2] = -p + q - r - a * 0.25;	ISols[2] = -ip + iq - ir;
	RSols[3] = -p - q + r - a * 0.25;	ISols[3] = -ip - iq + ir;
	
	/* Consider real solution only. */
	n = 0;
	for (i = 0; i < 4; i++) {
	    if (IRIT_APX_EQ_EPS(ISols[i], 0.0, GM_QUARTIC_SOLVE_EPS))
		Sols[n++] = GMSolveQuarticApplyNewton(RSols[i], a, b, c, d);
	}
    } /* End else (d != 0; f != 0, g != 0). */

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes one root of an imagniary number.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   RealVal, ImageVal:     The number to compute the root for.               M
*   RealRoot, ImageRoot:   The computed root.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMComplexRoot                                                            M
*****************************************************************************/
void GMComplexRoot(IrtRType RealVal,
		   IrtRType ImageVal,
		   IrtRType *RealRoot,
		   IrtRType *ImageRoot)
{
    if (IRIT_APX_EQ_EPS(ImageVal, 0.0, GM_QUARTIC_SOLVE_EPS)) {
        *RealRoot = sqrt(RealVal);
        *ImageRoot = 0.0;
    }
    else {
        IrtRType
	    sqrR = IRIT_SQR(RealVal),
	    sqrI = IRIT_SQR(ImageVal),
	    Len = sqrt(sqrR + sqrI),
	    RootLen = sqrt(Len),
	    Angle = atan2(ImageVal, RealVal);

        if (Angle < 0.0)
	    Angle = M_PI_MUL_2 + Angle;
	*RealRoot = RootLen * cos(Angle * 0.5);
	*ImageRoot = RootLen * sin(Angle * 0.5);
    }

#ifdef DEBUG_TEST_COMPLEX_ROOT
    {
        IrtRType
	    x = IRIT_SQR(*RealRoot) - IRIT_SQR(*ImageRoot),
	    y = *RealRoot * *ImageRoot * 2.0;

	if (!IRIT_APX_EQ_EPS(x, RealVal, GM_QUARTIC_SOLVE_EPS) ||
	    !IRIT_APX_EQ_EPS(y, ImageVal, GM_QUARTIC_SOLVE_EPS))
	    fprintf(stderr, "COMPLEX REAL ROOT FAILED\n");
    }
#endif /* DEBUG_TEST_COMPLEX_ROOT */
}

#ifdef DEBUG_TEST_POLYNOMIAL_EQN_SOLVERS

#define SOL_EPS	GM_QUARTIC_SOLVE_EPS

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test routines for the polynomial equation solvers.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
void main(void)
{
    int i, j, n;
    IrtRType Coefs[4], Sols[4], ISols[4], Val;

    for (i = 0; i < 10000000; i++) {
        switch (i) {
	    case 0:
	        Coefs[0] = Coefs[1] = Coefs[2] = Coefs[3] = 0.0;
	        break;
	    case 1:
	    case 2:
	        Coefs[0] = Coefs[2] = Coefs[3] = 0.0;
	        Coefs[1] = i == 1 ? 1.0 : -1.0;
	        break;
	    case 3:
	    case 4:
	        Coefs[0] = Coefs[1] = Coefs[3] = 0.0;
	        Coefs[2] = i == 3 ? 1.0 : -1.0;
	        break;
	    case 5:
	    case 6:
	        Coefs[0] = Coefs[1] = Coefs[2] = 0.0;
	        Coefs[3] = i == 5 ? 1.0 : -1.0;
	        break;
	    default:
	        for (j = 0; j < 4; j++ ) {
		    if (IRIT_FABS(Coefs[j] = IritRandom(-1, 1)) < 0.1)
		        Coefs[j] = 0.0;
		}
	}

	n = GMSolveQuadraticEqn(Coefs[0], Coefs[1], Sols);
	for (j = 0; j < n; j++) {
	    Val = IRIT_SQR(Sols[j]) + Coefs[0] * Sols[j] + Coefs[1];

	    if (IRIT_FABS(Val) > SOL_EPS)
	        printf("Error in quadratic equation solving\nX^2 + %.12f X + %.12f = %.12f, (X = %.12f)\n",
		       Coefs[0], Coefs[1], Val, Sols[j]);
	}

	n = GMSolveQuadraticEqn2(Coefs[0], Coefs[1], Sols, ISols);
	for (j = 0; j < n; j++) {
	    Val = IRIT_SQR(Sols[j]) + Coefs[0] * Sols[j] + Coefs[1];

	    if (IRIT_FABS(Val) > SOL_EPS)
	        printf("Error in quadratic equation 2 solving\nX^2 + %.12f X + %.12f = %.12f, (X = %.12f)\n",
		       Coefs[0], Coefs[1], Val, Sols[j]);
	}

	n = GMSolveCubicEqn(Coefs[0], Coefs[1], Coefs[2], Sols);
	for (j = 0; j < n; j++) {
	    Val = IRIT_CUBE(Sols[j]) + Coefs[0] * IRIT_SQR(Sols[j]) +
	          Coefs[1] * Sols[j] + Coefs[2];

	    if (IRIT_FABS(Val) > SOL_EPS)
	        printf("Error in cubic equation solving\nX^3 + %.12f X^2 + %.12f X + %.12f = %.12f, (X = %.12f)\n",
		       Coefs[0], Coefs[1], Coefs[2], Val, Sols[j]);
	}

	n = GMSolveCubicEqn2(Coefs[0], Coefs[1], Coefs[2], Sols, ISols);
	for (j = 0; j < n; j++) {
	    if (ISols[j] == 0.0) {
	        Val = IRIT_CUBE(Sols[j]) + Coefs[0] * IRIT_SQR(Sols[j]) +
	              Coefs[1] * Sols[j] + Coefs[2];

		if (IRIT_FABS(Val) > SOL_EPS)
		    printf("Error in cubic equation 2 solving\nX^3 + %.12f X^2 + %.12f X + %.12f = %.12f, (X = %.12f)\n",
			   Coefs[0], Coefs[1], Coefs[2], Val, Sols[j]);
	    }
	}

	n = GMSolveQuarticEqn(Coefs[0], Coefs[1], Coefs[2], Coefs[3], Sols);
	for (j = 0; j < n; j++) {
	    if (ISols[j] == 0.0) {
	        Val = GM_QUART(Sols[j]) + Coefs[0] * IRIT_CUBE(Sols[j]) +
		      Coefs[1] * IRIT_SQR(Sols[j]) + Coefs[2] * Sols[j] +
		      Coefs[3];

		if (IRIT_FABS(Val) > SOL_EPS)
		    printf("Error in quartic equation solving\nX^4 + %.12f X^3 + %.12f X^2 + %.12f X + %.12f = %.12f, (X = %.12f)\n",
			   Coefs[0], Coefs[1], Coefs[2], Coefs[3],
			   Val, Sols[j]);
	    }
	}
    }
}

#endif /* DEBUG_TEST_POLYNOMIAL_EQN_SOLVERS */
