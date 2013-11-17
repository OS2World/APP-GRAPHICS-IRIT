/******************************************************************************
* Distance.c - Distance computations.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 93.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "symb_loc.h"

#define SELF_INTER_REL_EPS 100

IRIT_STATIC_DATA CagdPtStruct
    *GlblSrfDistPtsList = NULL;

static int SymbCrvPointInclusionAux(const CagdCrvStruct *Crv,
				    const CagdPType Pt,
				    CagdRType Epsilon,
				    CagdRType *AngleAccum);
static void SymbSrfDistFindPointsAux(const CagdSrfStruct *Srf,
				     CagdRType Epsilon,
				     CagdBType SelfInter);
static void SrfDistAddZeroPoint(CagdRType u, CagdRType v, CagdRType Epsilon);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a point, finds the nearest point (if MinDist) or the     M
* farthest location (if MinDist FALSE) from the curve to the given point.    M
*   Returned is the parameter value of the curve.  Both internal as well as  M
* boundary extrema are considered.					     M
*   Computes the zero set of (Crv(t) - Pt) . Crv'(t), and also look at the   M
* curves' end points.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        The curve to find its nearest (farthest) point to Pt.        M
*   Pt:         The point to find the nearest (farthest) point on Crv to it. M
*   MinDist:    If TRUE nearest points is needed, if FALSE farthest.         M
*   Epsilon:    Accuracy of computation.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Parameter value in the parameter space of Crv of the        M
*                nearest (farthest) point to point Pt.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbLclDistCrvPoint, MvarDistSrfPoint				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbDistCrvPoint, curve point distance                                   M
*****************************************************************************/
CagdRType SymbDistCrvPoint(const CagdCrvStruct *Crv,
			   const CagdPType Pt,
			   CagdBType MinDist,
			   CagdRType Epsilon)
{
    CagdRType TMin, TMax, t,
	ExtremeDistSqr = MinDist ? IRIT_INFNTY : -IRIT_INFNTY;
    CagdPtStruct *TPt,
	*Pts = SymbLclDistCrvPoint(Crv, Pt, Epsilon);

    /* Add the two end points of the domain. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    TPt = CagdPtNew();
    TPt -> Pt[0] = TMin;
    IRIT_LIST_PUSH(TPt, Pts);
    TPt = CagdPtNew();
    TPt -> Pt[0] = TMax;
    IRIT_LIST_PUSH(TPt, Pts);

    for (TPt = Pts, t = TMin; TPt != NULL; TPt = TPt -> Pnext) {
	CagdPType EPt;
	CagdRType DistSqr,
	    *R = CagdCrvEval(Crv, TPt -> Pt[0]);

	CagdCoerceToE3(EPt, &R, - 1, Crv -> PType);
	DistSqr = IRIT_PT_PT_DIST_SQR(EPt, Pt);

	if (MinDist) {
	    if (DistSqr < ExtremeDistSqr) {
		t = TPt -> Pt[0];
		ExtremeDistSqr = DistSqr;
	    }
	}
	else {
	    if (DistSqr > ExtremeDistSqr) {
		t = TPt -> Pt[0];
		ExtremeDistSqr = DistSqr;
	    }
	}
    }
    CagdPtFreeList(Pts);

    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a point, find the local extremum distance points on the  M
* curve to the given point.   Only interior extrema are considered.	     M
*   Returned is a list of parameter value with local extremum.		     M
*   Computes the zero set of (Crv(t) - Pt) . Crv'(t).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:       The curve to find its extreme distance locations to Pt.      M
*   Pt:         The point to find the extreme distance locations from Crv.   M
*   Epsilon:    Accuracy of computation.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter values of extreme distance         M
*		      locations.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvPoint, MvarDistSrfPoint					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbLclDistCrvPoint, curve point distance                                M
*****************************************************************************/
CagdPtStruct *SymbLclDistCrvPoint(const CagdCrvStruct *CCrv,
				  const CagdPType Pt,
				  CagdRType Epsilon)
{
    int i;
    CagdCrvStruct *ZCrv,
        *Crv = CagdCrvCopy(CCrv),
	*DCrv = CagdCrvDerive(Crv);
    CagdPType MinusPt;
    CagdPtStruct *ZeroSet;

    for (i = 0; i < 3; i++)
	MinusPt[i] = -Pt[i];

    CagdCrvTransform(Crv, MinusPt, 1.0);

    ZCrv = SymbCrvDotProd(Crv, DCrv);
    CagdCrvFree(Crv);
    CagdCrvFree(DCrv);

    ZeroSet = SymbCrvZeroSet(ZCrv, 1, Epsilon, FALSE);
    CagdCrvFree(ZCrv);

    return ZeroSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a line, finds the nearest point (if MinDist) or the      M
* farthest location (if MinDist FALSE) from the curve to the given line.     M
*   Returned is the parameter value of the curve.  Both internal as well as  M
* boundary extrema are considered.					     M
*   Let Crv be (x(t), y(t)). By substituting x(t) and y(t) into the line     M
* equation, we derive the distance function.				     M
*   Its zero set, combined with the zero set of its derivative provide the   M
* needed extreme distances.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        The curve to find its nearest (farthest) point to Line.      M
*   Line:       The line to find the nearest (farthest) point on Crv to it.  M
*   MinDist:    If TRUE nearest points is needed, if FALSE farthest.         M
*   Epsilon:    Accuracy of computation.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Parameter value in the parameter space of Crv of the        M
*                nearest (farthest) point to line Line.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbLclDistCrvLine, MvarDistSrfLine, SymbCrvRayInter		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbDistCrvLine, curve line distance                                     M
*****************************************************************************/
CagdRType SymbDistCrvLine(const CagdCrvStruct *Crv,
			  const CagdLType Line,
			  CagdBType MinDist,
			  CagdRType Epsilon)
{
    CagdRType TMin, TMax, t,
	ExtremeDist = MinDist ? IRIT_INFNTY : -IRIT_INFNTY;
    CagdPtStruct *TPt,
	*Pts = SymbLclDistCrvLine(Crv, Line, Epsilon, TRUE, TRUE);

    /* Add the two end point of the domain. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    TPt = CagdPtNew();
    TPt -> Pt[0] = TMin;
    IRIT_LIST_PUSH(TPt, Pts);
    TPt = CagdPtNew();
    TPt -> Pt[0] = TMax;
    IRIT_LIST_PUSH(TPt, Pts);

    for (TPt = Pts, t = TMin; TPt != NULL; TPt = TPt -> Pnext) {
	CagdPType EPt;
	CagdRType
	    Dist = 0.0,
	    *R = CagdCrvEval(Crv, TPt -> Pt[0]);

	CagdCoerceToE2(EPt, &R, - 1, Crv -> PType);

	Dist = EPt[0] * Line[0] + EPt[1] * Line[1] + Line[2];
	Dist = IRIT_FABS(Dist);

	if (MinDist) {
	    if (Dist < ExtremeDist) {
		t = TPt -> Pt[0];
		ExtremeDist = Dist;
	    }
	}
	else {
	    if (Dist > ExtremeDist) {
		t = TPt -> Pt[0];
		ExtremeDist = Dist;
	    }
	}
    }
    CagdPtFreeList(Pts);

    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a line, finds the local extreme distance points on the   M
* curve to the given line.  Only interior extrema are considered.	     M
*   Returned is a list of parameter value with local extreme distances.      M
*   Let Crv be (x(t), y(t)). By substituting x(t) and y(t) into the line     M
* equation, we derive the distance function.				     M
*   Its zero set, possibly combined with the zero set of its derivative      M
* provide the needed extreme distances.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        The curve to find its nearest (farthest) point to Line.      M
*   Line:       The line to find the nearest (farthest) point on Crv to it.  M
*   Epsilon:    Accuracy of computation.                                     M
*   InterPos:   Do we want the intersection locations?			     M
*   ExtremPos:  Do we want the extremum distance locations?		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter values of extreme distance         M
*		      locations.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvLine, MvarDistSrfLine, SymbCrvRayInter			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbLclDistCrvLine, curve line distance                                  M
*****************************************************************************/
CagdPtStruct *SymbLclDistCrvLine(const CagdCrvStruct *Crv,
				 const CagdLType Line,
				 CagdRType Epsilon,
				 CagdBType InterPos,
				 CagdBType ExtremPos)
{
    CagdCrvStruct *TCrv1, *CrvX, *CrvY, *CrvZ, *CrvW, *DistCrv, *DerivDistCrv;
    CagdPtStruct *ZeroSet1, *ZeroSet2, *TPt;

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvZ)
	CagdCrvFree(CrvZ);

    CagdCrvTransform(CrvX, NULL, Line[0]);
    CagdCrvTransform(CrvY, NULL, Line[1]);
    TCrv1 = SymbCrvAdd(CrvX, CrvY);
    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    if (CrvW) {
	CagdCrvTransform(CrvW, NULL, Line[2]);
	DistCrv = SymbCrvAdd(TCrv1, CrvW);
	CagdCrvFree(CrvW);
	CagdCrvFree(TCrv1);
    }
    else {
	CagdCrvTransform(TCrv1, &Line[2], 1.0);
	DistCrv = TCrv1;
    }

    if (InterPos)
	ZeroSet1 = SymbCrvZeroSet(DistCrv, 1, Epsilon, FALSE);
    else
	ZeroSet1 = NULL;

    if (ExtremPos) {
	DerivDistCrv = CagdCrvDerive(DistCrv);

	ZeroSet2 = SymbCrvZeroSet(DerivDistCrv, 1, Epsilon, FALSE);
	CagdCrvFree(DerivDistCrv);
    }
    else
	ZeroSet2 = NULL;

    CagdCrvFree(DistCrv);

    if (ZeroSet1 == NULL)
	return ZeroSet2;
    else if (ZeroSet2 == NULL)
	return ZeroSet1;
    else {
	for (TPt = ZeroSet1; TPt -> Pnext != NULL; TPt = TPt -> Pnext);

	TPt -> Pnext = ZeroSet2;

	return ZeroSet1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a closed planar curve and a point, finds if point is inside curve. M
*   Uses winding number accumulation in the computation.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:     Closed planar curve to examine for the inclusion of Pt.        M
*   Pt:       Point tp test for inclusion in Crv.			     M
*   Epsilon:  Accuracy of computation.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       1 if Pt inside Crv, -1 otherwise, 0 if on boundary to within  M
*	       Epsilon.						             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPointInclusion, SymbDistCrvLine, SymbCrvRayInter		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvPointInclusion, curve point inclusion                             M
*****************************************************************************/
int SymbCrvPointInclusion(const CagdCrvStruct *CCrv,
			  const CagdPType Pt,
			  CagdRType Epsilon)
{
    int RetVal;
    CagdRType
        Theta = 0.0;
    CagdCrvStruct
	*Crv = CagdCnvrtBsp2OpenCrv(CCrv);

    RetVal = SymbCrvPointInclusionAux(Crv, Pt, Epsilon, &Theta);
    CagdCrvFree(Crv);

    if (!RetVal)
	return 0;
    else
        return IRIT_FABS(Theta) > M_PI ? 1 : -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a closed planar curve and a point, finds if point is inside curve. *
*   Uses winding number accumulation in the computation, over bbox's that do *
* not include the queried point Pt.		                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         Closed planar curve to examine for the inclusion of Pt.     *
*   Pt:          Point tp test for inclusion in Crv.			     *
*   Epsilon:     Accuracy of computation.                                    *
*   AngleAccum:  To accumulate the computed angles here.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE if (almost) on boundary.            *
*****************************************************************************/
static int SymbCrvPointInclusionAux(const CagdCrvStruct *Crv,
				    const CagdPType Pt,
				    CagdRType Epsilon,
				    CagdRType *AngleAccum)
{
    CagdBBoxStruct BBox;

    CagdCrvBBox(Crv, &BBox);

    if (Pt[0] < BBox.Min[0] ||
	Pt[0] > BBox.Max[0] ||
	Pt[1] < BBox.Min[1] ||
	Pt[1] > BBox.Max[1]) {
        /* Point outside the curve's bbox - accumulate end pts angles. */
        CagdPType Pt1, Pt2;
	CagdVType Dir1, Dir2;

	CagdCoerceToE2(Pt1, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE2(Pt2, Crv -> Points, Crv -> Length - 1, Crv -> PType);
	IRIT_VEC2D_SET(Dir1, Pt1[0] - Pt[0], Pt1[1] - Pt[1]);
	IRIT_VEC2D_SET(Dir2, Pt2[0] - Pt[0], Pt2[1] - Pt[1]);

	*AngleAccum += atan2(IRIT_CROSS_PROD_2D(Dir1, Dir2),
			     IRIT_DOT_PROD_2D(Dir1, Dir2));
	return TRUE;
    }
    else {				             /* Divide and conquer. */
        int RetVal;
	CagdRType TMin, TMax;
        CagdCrvStruct *Crv1;

	CagdCrvDomain(Crv, &TMin, &TMax);
	if (TMax - TMin < Epsilon)
	    return FALSE;
	Crv1 = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5);
	RetVal = SymbCrvPointInclusionAux(Crv1, Pt, Epsilon, AngleAccum) &&
	         SymbCrvPointInclusionAux(Crv1 -> Pnext, Pt, Epsilon,
					  AngleAccum);
	CagdCrvFreeList(Crv1);
	return RetVal;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a ray, finds the intersection points on the curve where  M
* the ray pierces the curve.  						     M
*   Returned is a list of parameter value with local extreme distances.      M
*   Let Crv be (x(t), y(t)). By substituting x(t) and y(t) into the line     M
* equation of the ray, we derive the distance function whose zero set	     M
* captures all curve-line intersections.  They are then filtered to those    M
* on the half-line ray.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:            The curve to find its intersections with the ray.	     M
*   RayPt, RayDir:  The ray prescription.				     M
*   Epsilon:        Accuracy of computation.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter values of the ray-curve            M
*		      intersections.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvLine, MvarDistSrfLine, SymbLclDistCrvLine		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvRayInter, curve ray intersection                                  M
*****************************************************************************/
CagdPtStruct *SymbCrvRayInter(const CagdCrvStruct *Crv,
			      const CagdPType RayPt,
			      const CagdVType RayDir,
			      CagdRType Epsilon)
{
    CagdLType Line;
    CagdPtStruct *Pts,
	*RetPts = NULL;

    /* Build the line equation. */
    Line[0] = RayDir[1];
    Line[1] = -RayDir[0];
    Line[2] = -IRIT_DOT_PROD_2D(RayPt, Line);

    /* Find all line-curve intersections. */
    Pts = SymbLclDistCrvLine(Crv, Line, Epsilon, TRUE, FALSE);

    /* Filter out those points that are not in the ray, half-line, domain. */
    while (Pts != NULL) {
        CagdRType *R;
	CagdPType PtE2;
	CagdPtStruct *Pt;

	IRIT_LIST_POP(Pt, Pts);

	R = CagdCrvEval(Crv, Pt -> Pt[0]);
	CagdCoerceToE2(PtE2, &R, -1, Crv -> PType);

	IRIT_PT2D_SUB(PtE2, PtE2, RayPt);
	if (IRIT_DOT_PROD_2D(PtE2, RayDir) > Epsilon * 10)
	    IRIT_LIST_PUSH(Pt, RetPts)
	else
	    CagdPtFree(Pt);	        
    }

    return CagdListReverse(RetPts);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves, creates a bivariate scalar surface representing the      M
* distance function square, between them.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves, Crv1(u) and Crv2(v), to form their distance M
*                function square between them as a bivariate function.       M
*   DistType:    0 for distance vector function,			     M
*		 1 for distance square scalar function,			     M
*		 2 for distance vector projected on the normal of Crv1,	     M
*		 3 for distance vector projected on the normal of Crv2.	     M
*		 4 for distance vector projected on the tangent of Crv1.     M
*		 5 for distance vector projected on the tangent of Crv2.     M
*		 In cases 2 to 5 the vector field is not normalized.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The distance function square d2(u, v) of the distance M
*                      from Crv1(u) to Crv2(v).                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCrvInter, SymbSrfDistFindPoints				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDistCrvCrv, curve curve distance                                  M
*****************************************************************************/
CagdSrfStruct *SymbSrfDistCrvCrv(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2,
				 int DistType)
{
    CagdCrvStruct *CNrml, *CTan;
    CagdSrfStruct *TSrf, *DiffSrf, *RetSrf, *SNrml, *STan,
	*Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR),
	*Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);

    if (CAGD_IS_BSPLINE_SRF(Srf1) || CAGD_IS_BSPLINE_SRF(Srf2)) {
	CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;

	if (CAGD_IS_BEZIER_SRF(Srf1)) {
	    TSrf = CagdCnvrtBzr2BspSrf(Srf1);
	    CagdSrfFree(Srf1);
	    Srf1 = TSrf;
	}
	if (CAGD_IS_BEZIER_SRF(Srf2)) {
	    TSrf = CagdCnvrtBzr2BspSrf(Srf2);
	    CagdSrfFree(Srf2);
	    Srf2 = TSrf;
	}

	CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
	CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
	BspKnotAffineTrans(Srf1 -> VKnotVector,
			   Srf1 -> VLength + Srf1 -> VOrder,
			   VMin2 - VMin1, (VMax2 - VMin2) / (VMax1 - VMin1));
	BspKnotAffineTrans(Srf2 -> UKnotVector,
			   Srf2 -> ULength + Srf1 -> VOrder,
			   UMin1 - UMin2, (UMax1 - UMin1) / (UMax2 - UMin2));
    }

    DiffSrf = SymbSrfSub(Srf1, Srf2);
    switch (DistType) {
	case 0:
	    RetSrf = DiffSrf;
    	    break;
	case 1:
	default:
	    RetSrf = SymbSrfDotProd(DiffSrf, DiffSrf);
	    CagdSrfFree(DiffSrf);
	    break;
	case 2:
	    CNrml = SymbCrv2DUnnormNormal(Crv1);
	    SNrml = CagdPromoteCrvToSrf(CNrml, CAGD_CONST_U_DIR);
	    CagdCrvFree(CNrml);
	    RetSrf = SymbSrfDotProd(DiffSrf, SNrml);
	    CagdSrfFree(DiffSrf);
	    CagdSrfFree(SNrml);
	    break;
	case 3:
	    CNrml = SymbCrv2DUnnormNormal(Crv2);
	    SNrml = CagdPromoteCrvToSrf(CNrml, CAGD_CONST_V_DIR);
	    CagdCrvFree(CNrml);
	    RetSrf = SymbSrfDotProd(DiffSrf, SNrml);
	    CagdSrfFree(DiffSrf);
	    CagdSrfFree(SNrml);
	    break;
	case 4:
	    CTan = CagdCrvDerive(Crv1);
	    STan = CagdPromoteCrvToSrf(CTan, CAGD_CONST_U_DIR);
	    CagdCrvFree(CTan);
	    RetSrf = SymbSrfDotProd(DiffSrf, STan);
	    CagdSrfFree(DiffSrf);
	    CagdSrfFree(STan);
	    break;
	case 5:
	    CTan = CagdCrvDerive(Crv2);
	    STan = CagdPromoteCrvToSrf(CTan, CAGD_CONST_V_DIR);
	    CagdCrvFree(CTan);
	    RetSrf = SymbSrfDotProd(DiffSrf, STan);
	    CagdSrfFree(DiffSrf);
	    CagdSrfFree(STan);
	    break;
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return RetSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar surface representing the distance function square between   M
* two curves, finds the zero set of the distance surface, if any, and        M
* returns it.								     M
*   The given surface is a non negative surface and zero set is its minima.  M
*   The returned points will contain the two parameter values of the two     M
* curves that intersect in the detected zero set points.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:        A bivariate function that represent the distance square     M
*                function between two curves.				     M
*   Epsilon:     Accuracy control.                                           M
*   SelfInter:   Should we consider self intersection? That is, is Srf       M
*                computed from a curve to itself!?			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  A list of parameter values of both curves, at all       M
*                    detected intersection locations.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDistCrvCrv, SymvCrvCrvInter					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDistFindPoints, curve curve distance, curve curve intersection    M
*****************************************************************************/
CagdPtStruct *SymbSrfDistFindPoints(const CagdSrfStruct *CSrf,
				    CagdRType Epsilon,
				    CagdBType SelfInter)
{
    CagdSrfStruct *Srf;

    GlblSrfDistPtsList = NULL;

    if (CSrf -> GType == CAGD_SBEZIER_TYPE)
	Srf = CagdCnvrtBzr2BspSrf(CSrf);/* To keep track on the domains. */
    else
	Srf = CagdSrfCopy(CSrf);

    SymbSrfDistFindPointsAux(Srf, Epsilon, SelfInter);

    CagdSrfFree(Srf);

    return GlblSrfDistPtsList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection points of two planar curves, in the XY plane   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to intersect.				     M
*   CCIEpsilon:   Tolerance of computation.                                  M
*   SelfInter:    If TRUE, needs to handle a curve against itself detecting  M
*	self intersections in Crv1 (== Crv2).				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   List of intersection points.  Each point holds the     M
*	intersection location in Crv1 as first coefficient and the           M
*	intersection location in Crv2 as second coefficient.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDistCrvCrv, SymbSrfDistFindPoints, CagdCrvCrvInter		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvInter                                                          M
*****************************************************************************/
CagdPtStruct *SymbCrvCrvInter(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2,
			      CagdRType CCIEpsilon,
			      CagdBType SelfInter)

{
    CagdSrfStruct
	*DistSrf = SymbSrfDistCrvCrv(Crv1, Crv2, 1);
    CagdPtStruct
        *IPts = SymbSrfDistFindPoints(DistSrf, CCIEpsilon, SelfInter);

    CagdSrfFree(DistSrf);
    return IPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function of SymbSrfDistFindPoints - does the hard work.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:         A bivariate function that represent the distance square     *
*                function between two curves.				     *
*   Epsilon:     Accuracy control.                                           *
*   SelfInter:   Should be consider self intersection? That is, is Srf       *
*                between a curve to itself!?				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None	                                                             *
*****************************************************************************/
static void SymbSrfDistFindPointsAux(const CagdSrfStruct *Srf,
				     CagdRType Epsilon,
				     CagdBType SelfInter)
{
    int i, j,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength;
    CagdRType
	*XPts = Srf -> Points[1];

    for (i = 0; i < ULength; i++) {
	for (j = 0; j < VLength; j++) {
	    if (*XPts++ <= 0.0) {
		CagdRType UMin, UMax, VMin, VMax;

		CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

		if (SelfInter) {
		    CagdRType
			SelfInterEps = Epsilon * SELF_INTER_REL_EPS;

		    if (UMax - UMin < SelfInterEps &&
			VMax - VMin < SelfInterEps &&
			UMax - VMin < SelfInterEps &&
			VMax - UMin < SelfInterEps)
		        return;
		}

		/* The surface may have a zero here. Test the domain size   */
		/* to make sure it makes sense to subdivide.		    */
		if (UMax - UMin < Epsilon && VMax - VMin < Epsilon) {
		    SrfDistAddZeroPoint((UMin + UMax) * 0.5,
					(VMin + VMax) * 0.5, Epsilon);
		}
		else {
		    /* Subdivide the surface and invoke recursively. */
		    CagdSrfStruct *Srf1, *Srf2;
		    CagdRType t;
		    CagdSrfDirType Dir;

		    if (UMax - UMin > VMax - VMin) {
			t = (UMin + UMax) * 0.5;
			Dir = CAGD_CONST_U_DIR;
		    }
		    else {
			t = (VMin + VMax) * 0.5;
			Dir = CAGD_CONST_V_DIR;
		    }
		    Srf1 = CagdSrfSubdivAtParam(Srf, t, Dir);
		    Srf2 = Srf1 -> Pnext;
		    SymbSrfDistFindPointsAux(Srf1, Epsilon, SelfInter);
		    SymbSrfDistFindPointsAux(Srf2, Epsilon, SelfInter);
		    CagdSrfFree(Srf1);
		    CagdSrfFree(Srf2);
		}
		return;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function of SymbSrfDistFindPoints - part of the hard work.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   u, v:   A zero set location found to be added to global parameter list.  *
*   Epsilon:    To match against existing zero set point, for simiarity,     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfDistAddZeroPoint(CagdRType u, CagdRType v, CagdRType Epsilon)
{
    CagdPtStruct *Pt;

    for (Pt = GlblSrfDistPtsList; Pt != NULL; Pt = Pt -> Pnext) {
	if (IRIT_FABS(Pt -> Pt[0] - u) < Epsilon * 10 &&
	    IRIT_FABS(Pt -> Pt[1] - v) < Epsilon * 10)
	    return;			         /* Point is already there. */
    }

    Pt = CagdPtNew();

    Pt -> Pt[0] = u;
    Pt -> Pt[1] = v;
    Pt -> Pnext = GlblSrfDistPtsList;
    GlblSrfDistPtsList = Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the surface of equadistance to two univariate (curve) entities. M
* The zero set of the computed surface equal to the locus of points whose    M
* sum or different of distances to Crv1 and Crv2 equal Dist.		     M
*   Let d1 and d2 be the two distances of a point on the conic to Crv1 and   M
* Crv2, respectively:							     M
*				d1 +/- d2 = Dist			     V
*									     M
*   The distance to a curve is a local minimal distance to it occurs along   M
* the normal of the curves.  Hence we first seek the solution to the         M
* following two equations in X and Y and two unknowns, A and B:		     M
*									     M
*   C1(t) + a N1(t) = C2(r) + b N2(r),	    a, b > 0			     V
*									     M
* where Ni is the unnormalized normal of Ci.  Given a and b (rationals), d1  M
* and d2 equals:							     M
*									     M
*	d1 = a || N1(t) ||) = a sqrt( <N1(t), N1(t)> ),			     M
*       d2 = b || N2(s) ||) = b sqrt( <N2(s), N2(s)> ),			     M
*									     M
* and we need to solve for,						     M
*									     M
*	a sqrt( <N1(t), N1(t)> ) +/- b sqrt( <N2(s), N2(s)> ) = Dist	     V
*									     M
* Because we are unable to represent square roots as rationals, we square:   M
*									     M
*	a^2 <N1(t), N1(t)> + b^2 <N2(s), N2(s)> - Dist^2 =		     V
*		= -/+ 2 a b sqrt( <N1(t), N1(t)> ) sqrt( <N2(s), N2(s)> )    V
* or									     M
*	( a^2 <N1(t), N1(t)> + b^2 <N2(s), N2(s)> - Dist^2 )^2 -	     V
*			    - 4 (ab)^2 <N1(t), N1(t)> <N2(s), N2(s)> = 0     V
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2:  Two curves to compute elliptic/hyperbolic distance        M
*		function to.  Assumes E2 curves.			     M
*   Dist:       Distance to the two entities.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar surface whose zero set provides the conic    M
*		distance (both elliptic sum and hyperbolc difference).	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConicDistCrvCrv                                                      M
*****************************************************************************/
CagdSrfStruct *SymbConicDistCrvCrv(const CagdCrvStruct *CCrv1,
				   const CagdCrvStruct *CCrv2,
				   CagdRType Dist)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *DCrv1, *DCrv2, *Crv1, *Crv2;
    CagdSrfStruct *Res, *Srf1X, *Srf1Y, *Srf2X, *Srf2Y, *NSrf1X, *NSrf1Y,
	*NSrf2X, *NSrf2Y, *AlphaSrf, *BetaSrf, *Denom,
        *TSrf1, *TSrf2, *TSrf3, *TSrf4, *Srf1, *Srf2, *DSrf1, *DSrf2;

    Crv1 = CagdCoerceCrvTo(CCrv1, CAGD_PT_E2_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(CCrv2, CAGD_PT_E2_TYPE, FALSE);

    if (CAGD_IS_BEZIER_CRV(Crv1)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
    if (CAGD_IS_BEZIER_CRV(Crv2)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv2);

	CagdCrvFree(Crv2);
	Crv2 = TCrv;
    }

    DCrv1 = CagdCrvDerive(Crv1);
    DCrv2 = CagdCrvDerive(Crv2);

    Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
    Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    BspKnotAffineTrans2(Srf1 -> VKnotVector, Srf1 -> VLength + Srf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(Srf2 -> UKnotVector, Srf2 -> ULength + Srf2 -> UOrder,
			UMin1, UMax1);

    DSrf1 = CagdPromoteCrvToSrf(DCrv1, CAGD_CONST_U_DIR);
    DSrf2 = CagdPromoteCrvToSrf(DCrv2, CAGD_CONST_V_DIR);

    BspKnotAffineTrans2(DSrf1 -> VKnotVector,
			DSrf1 -> VLength + DSrf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(DSrf2 -> UKnotVector,
			DSrf2 -> ULength + DSrf2 -> UOrder,
			UMin1, UMax1);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(DCrv1);
    CagdCrvFree(DCrv2);

    SymbSrfSplitScalar(Srf1, &TSrf1, &Srf1X, &Srf1Y, &TSrf1);
    SymbSrfSplitScalar(Srf2, &TSrf1, &Srf2X, &Srf2Y, &TSrf1);

    /* Extract normal vectors from tangent fields as (Nx, Ny) = (Ty, -Tx): */
    SymbSrfSplitScalar(DSrf1, &TSrf1, &NSrf1Y, &TSrf2, &TSrf1);
    NSrf1X = SymbSrfScalarScale(TSrf2, -1);
    CagdSrfFree(TSrf2);
    SymbSrfSplitScalar(DSrf2, &TSrf1, &NSrf2Y, &TSrf2, &TSrf1);
    NSrf2X = SymbSrfScalarScale(TSrf2, -1);
    CagdSrfFree(TSrf2);

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    /* Set up two equations of C1(t) + a N1(t) == C2(r) + b N2(r), as:       */
    /* a N1(t) - b N2(r) = C2(r) - C1(t), in X and Y, and solve for a and b. */

    TSrf1 = SymbSrfSub(Srf2X, Srf1X);
    TSrf2 = SymbSrfSub(Srf2Y, Srf1Y);

    TSrf3 = SymbSrfMult(NSrf1X, NSrf2Y);
    TSrf4 = SymbSrfMult(NSrf1Y, NSrf2X);
    Denom = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    TSrf3 = SymbSrfMult(TSrf1, NSrf2Y);
    TSrf4 = SymbSrfMult(TSrf2, NSrf2X);
    AlphaSrf = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    TSrf3 = SymbSrfMult(NSrf1X, TSrf2);
    TSrf4 = SymbSrfMult(NSrf1Y, TSrf1);
    BetaSrf = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    /* Compute "Alpha^2 <N1(t), N1(t)>" and "Beta^2 <N2(r), N2(r)>": */
    TSrf1 = SymbSrfDotProd(DSrf1, DSrf1);
    CagdSrfFree(DSrf1);
    TSrf2 = SymbSrfMult(AlphaSrf, AlphaSrf);
    CagdSrfFree(AlphaSrf);
    AlphaSrf = SymbSrfMult(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    TSrf1 = SymbSrfDotProd(DSrf2, DSrf2);
    CagdSrfFree(DSrf2);
    TSrf2 = SymbSrfMult(BetaSrf, BetaSrf);
    CagdSrfFree(BetaSrf);
    BetaSrf = SymbSrfMult(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    TSrf1 = SymbSrfScalarScale(Denom, Dist);
    TSrf2 = SymbSrfMult(TSrf1, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(Denom);
    Denom = TSrf2;

    /* Eval "(Alpha^2 <N1(t), N1(t)> + Beta^2 <N2(r), N2(r)> - Dist^2)^2": */
    TSrf1 = SymbSrfAdd(AlphaSrf, BetaSrf);
    TSrf2 = SymbSrfSub(TSrf1, Denom);
    CagdSrfFree(Denom);
    CagdSrfFree(TSrf1);
    Res = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf2);

    /* Subtract "4 (ab)^2 <N1(t), N1(t)> <N2(r), N2(r)>": */
    TSrf1 = SymbSrfMult(AlphaSrf, BetaSrf);
    TSrf2 = SymbSrfScalarScale(TSrf1, 4.0);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfSub(Res, TSrf2);
    CagdSrfFree(Res);
    CagdSrfFree(TSrf2);
    Res = TSrf3;

    CagdSrfFree(AlphaSrf);
    CagdSrfFree(BetaSrf);
    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(NSrf1X);
    CagdSrfFree(NSrf1Y);
    CagdSrfFree(NSrf2X);
    CagdSrfFree(NSrf2Y);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute a discrete distance map to a freeform curve by sampling the      M
* distance on a regular grid.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          The curve to approximate a discrete distance map for.      M
*   Tolerance:	  Tolerance of distance computation.			     M
*   XDomain:      X domain to sample R2 for distances.			     M
*   YDomain:      Y domain to sample R2 for distances.			     M
*   DiscMap:      Where output is saved as a real distance value.	     M
*   DiscMapXSize: Horizontal resolution, 0 will be mapped to XDomain[0],     M
*				         (DiscMapXSize-1) to XDomain[0].     M
*   DiscMapYSize: Vertical resolution, 0 will be mapped to YDomain[0],	     M
*				       (DiscMapYSize-1) to YDomain[0].	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  Maximal distance of points in prescribed domain to crv Crv.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvPoint                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbDistBuildMapToCrv                                                    M
*****************************************************************************/
CagdRType SymbDistBuildMapToCrv(const CagdCrvStruct *Crv,
				CagdRType Tolerance,
				CagdRType *XDomain,
				CagdRType *YDomain,
				CagdRType **DiscMap,
				CagdRType DiscMapXSize,
				CagdRType DiscMapYSize)
{
    int i, j;
    CagdRType
	MaxDist = 0.0,
	Dx = (XDomain[1] - XDomain[0]) / (DiscMapXSize - 1),
	Dy = (YDomain[1] - YDomain[0]) / (DiscMapYSize - 1);
    CagdPType Pos;

    Pos[1] = YDomain[0];

    for (j = 0; j < DiscMapYSize; j++, Pos[1] += Dy) {
	Pos[0] = XDomain[0];
#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDistMapState, FALSE)
	        IRIT_INFO_MSG_PRINTF("Processing line %3d\r", j);
	}
#	endif /* DEBUG */

	for (i = 0; i < DiscMapXSize; i++, Pos[0] += Dx) {
	    IrtRType *R,
		t = SymbDistCrvPoint(Crv, Pos, TRUE, Tolerance);
	    CagdPType E2Pt;

	    R = CagdCrvEval(Crv, t);
	    CagdCoerceToE2(E2Pt, &R, - 1, Crv -> PType);

	    t = IRIT_PT2D_DIST(E2Pt, Pos);

	    DiscMap[i][j] = t;
	    MaxDist = IRIT_MAX(MaxDist, t);
	}
    }

    return MaxDist;
}
