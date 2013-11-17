/******************************************************************************
* Crv_Skel.c - computation of curve/surface skeleton approximation.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, August 96    				      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"

#define CONTOUR_EPS	   1e-8     /* Level above zero to actually contour. */
#define DIAG_POINT_EPS     1e-3   /* Off diagonal to be considered diagonal. */
#define MAX_DOMAIN	   10	               /* 10 times the bounding box. */
#define MAX_NUMER_ITER     3		  /* Number of numerical iterations. */
#define MAX_ERROR_ALLOWED  1e-6		        /* In mid point computation. */
#define SRF_SCALE_FACTOR   1	    /* Scaling factor for better contouring. */

#define MULT_AND_REPLACE_FIRST(Srf1, Srf2) { \
					CagdSrfStruct \
					    *TSrf = Srf1; \
\
				        Srf1 = SymbSrfMult(Srf1, Srf2); \
					CagdSrfFree(TSrf); \
				    }

#define DUPLICATE_E1_TO_E3_CRV(CrvE3, CrvE1) { \
		CrvE3 = CagdCoerceCrvTo(CrvE1, CAGD_PT_E3_TYPE, FALSE); \
		CAGD_GEN_COPY(CrvE3 -> Points[2], CrvE3 -> Points[1], \
			      sizeof(CagdRType) * CrvE3 -> Length); \
		CAGD_GEN_COPY(CrvE3 -> Points[3], CrvE3 -> Points[1], \
			      sizeof(CagdRType) * CrvE3 -> Length); \
	    }

#define DUPLICATE_E1_TO_E3_SRF(SrfE3, SrfE1) { \
        SrfE3 = CagdCoerceSrfTo(SrfE1, CAGD_PT_E3_TYPE, FALSE); \
	CAGD_GEN_COPY(SrfE3 -> Points[2], SrfE3 -> Points[1], \
		      sizeof(CagdRType) * SrfE3 -> ULength * SrfE3 -> VLength); \
	CAGD_GEN_COPY(SrfE3 -> Points[3], SrfE3 -> Points[1], \
		      sizeof(CagdRType) * SrfE3 -> ULength * SrfE3 -> VLength); \
    }

static int NumerMarchMidPoint(const CagdCrvStruct *Crv1,
			      CagdRType *t1,
			      const CagdCrvStruct *Crv2,
			      CagdRType *t2);


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the skeleton curves (bisectors) of a given curve or two.        M
* If Crv contains a list of two curves the bisector between the two curves   M
* is computed.  Otherwise, Crv self bisectors are computed.		     M
*   Employs the F1/F2/F34 functions from the paper:			     M
* Gershon Elber and Myung Soo Kim.  ``Bisector Curves of Planar Rational     M
* Curves.''  CAD, Vol 30, No 14, pp 1089-1096, December 1998.                M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Either one or two curves to compute bisectors for.	     M
*		  Assumes E2 curves.					     M
*   BisectFunc:   If 1, normal fields are used to compute bisector surface.  M
*		  If 2, tangent fields instead of normals are used to        M
*                 compute the bisector surface function.		     M
*		  If 3, using solution of normal intersection pt.	     M 
*   Tolerance:    Accuracy of computation.				     M
*   NumerImprove: If TRUE, a numerical improvment stage is applied.          M
*   SameNormal:	  If TRUE, the bisector should be oriented for inner or      M
*		  outer side of the curves, with respect to their normals.   M
*   SupportPrms:  If TRUE, return curve is of type E4 instead of E2 and the  M
*		  third and fourth coefficients holds the support parameters M
*		  of the first and second curves, respectively.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of piecewise linear curves approximating the    M
*	       bisectors of Crv.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectorsSrf                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvBisectors, bisectors, skeleton                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrvBisectors(const CagdCrvStruct *Crv,
				int BisectFunc,
				CagdRType Tolerance,
				CagdBType NumerImprove,
				CagdBType SameNormal,
				CagdBType SupportPrms)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };    /* A scalar srf - only X. */
    CagdBType
	SelfBisect = Crv -> Pnext == NULL;
    CagdRType TMin1, TMax1, TMin2, TMax2, XMax, YMax;
    CagdSrfStruct
        *BisectSrf = SymbCrvBisectorsSrf(Crv, BisectFunc);
    IPPolygonStruct *Cntrs, *Cntr;
    CagdCrvStruct
	*PLCrvList = NULL;
    const CagdCrvStruct
	*Crv1 = Crv,
	*Crv2 = Crv -> Pnext ? Crv -> Pnext : Crv;
    CagdBBoxStruct BBox;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    CagdSrfTransform(BisectSrf, NULL, SRF_SCALE_FACTOR);

    /* Computes the zero set of the equation as contours. */
    Cntrs = UserCntrSrfWithPlane(BisectSrf, Plane, Tolerance);
    CagdSrfFree(BisectSrf);

    CagdCrvBBox(Crv, &BBox);
    XMax = MAX_DOMAIN * IRIT_MAX(IRIT_FABS(BBox.Min[0]),
				 IRIT_FABS(BBox.Max[0]));
    YMax = MAX_DOMAIN * IRIT_MAX(IRIT_FABS(BBox.Min[1]),
				 IRIT_FABS(BBox.Max[1]));

    /* Get the st parameters and convert to the bisector's position. */
    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V,
	    *VPrev = NULL;
	int i, Len;
	CagdCrvStruct *PLCrv;
	CagdRType **Points;

	/* Filters out the data - no diagonals or below diagonals. */
	for (V = Cntr -> PVertex; V != NULL; ) {
	    CagdRType *R, Err;
	    CagdPType Pt1, Pt2, V1, V2, Nrml1, Nrml2;
	    CagdVecStruct *Vec;

	    V -> Coord[1] = IRIT_BOUND(V -> Coord[1], TMin1, TMax1);
	    V -> Coord[2] = IRIT_BOUND(V -> Coord[2], TMin2, TMax2);

	    /* Numerically improve the data. */
	    if (NumerImprove) {
		NumerMarchMidPoint(Crv1, &V -> Coord[1], Crv2, &V -> Coord[2]);
		NumerMarchMidPoint(Crv2, &V -> Coord[2], Crv1, &V -> Coord[1]);
	    }

	    Vec = CagdCrvNormalXY(Crv1, V -> Coord[1], TRUE);
	    IRIT_PT_COPY(Nrml1, Vec -> Vec);
	    Vec = CagdCrvNormalXY(Crv2, V -> Coord[2], TRUE);
	    IRIT_PT_COPY(Nrml2, Vec -> Vec);

	    R = CagdCrvEval(Crv1, V -> Coord[1]);
	    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	    R = CagdCrvEval(Crv2, V -> Coord[2]);
	    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

	    R = SymbBsctComputeInterMidPoint(Crv1, V -> Coord[1], Crv2, V -> Coord[2]);
		
	    IRIT_PT_SUB(V1, Pt1, R);
	    IRIT_PT_SUB(V2, Pt2, R);
	    Err = IRIT_FABS(IRIT_PT_LENGTH(V1) - IRIT_PT_LENGTH(V2));

	    if ((NumerImprove && Err > MAX_ERROR_ALLOWED) ||
		IRIT_FABS(R[0]) > XMax ||
		IRIT_FABS(R[1]) > YMax ||
		(SameNormal && IRIT_DOT_PROD(Nrml1, Nrml2) > 0.0) ||
		(VPrev != NULL && IRIT_PT_APX_EQ(V -> Coord, VPrev -> Coord)) ||
		(SelfBisect &&
		 (IRIT_APX_EQ_EPS(V -> Coord[1], V -> Coord[2], DIAG_POINT_EPS) ||
		  V -> Coord[1] < V -> Coord[2]))) {
		/* Remove this vertex. */
		if (VPrev != NULL) {
		    VPrev -> Pnext = V -> Pnext;
		    IPFreeVertex(V);
		    V = VPrev -> Pnext;
		}
		else { /* First vertex in contour. */
		    Cntr -> PVertex = V -> Pnext;
		    IPFreeVertex(V);
		    V = Cntr -> PVertex;
		}
	    }
	    else {
		VPrev = V;
		V = V -> Pnext;
	    }		    
	}

	Len = IPVrtxListLen(Cntr -> PVertex);
	if (Len < 2)
	    continue;
 
	PLCrv = BspCrvNew(Len, 2, SupportPrms ? CAGD_PT_E4_TYPE
			  		      : CAGD_PT_E2_TYPE);
	Points = PLCrv -> Points;
	BspKnotUniformOpen(Len, 2, PLCrv -> KnotVector);

	for (V = Cntr -> PVertex, i = 0; V != NULL; V = V -> Pnext, i++) {
	    CagdRType
	        *R = SymbBsctComputeInterMidPoint(Crv1, V -> Coord[1],
					  Crv2, V -> Coord[2]);

	    Points[1][i] = R[0];
	    Points[2][i] = R[1];
		
	    if (SupportPrms) {
		Points[3][i] = V -> Coord[1];
		Points[4][i] = V -> Coord[2];
	    }
	}

	if (PLCrv != NULL)
	    IRIT_LIST_PUSH(PLCrv, PLCrvList);
    }

    IPFreePolygonList(Cntrs);

    return PLCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Numerically march along curve 1 in order to improve the result of the    *
* mid point computation, using Newton Raphson.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1:      First curve of the matching mid point.                        *
*   t1:        Parameter value of first curve's mid point.                   *
*   Crv2       Second curve of the matching mid point.                       *
*   t2:        Parameter value of second curve's mid point.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void       TRUE if succesfully improved the mid point, FALSE otherwise.  *
*****************************************************************************/
static int NumerMarchMidPoint(const CagdCrvStruct *Crv1,
			      CagdRType *t1,
			      const CagdCrvStruct *Crv2,
			      CagdRType *t2)
{
    int i = 0,
	Improve = FALSE;
    CagdPType Pt1, Pt2;
    CagdRType Error, *R, TMin1, TMax1, NewError, DErrDt1, t1n;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);

    R = CagdCrvEval(Crv1, *t1);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, *t2);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    R = SymbBsctComputeInterMidPoint(Crv1, *t1, Crv2, *t2);
    Error = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
		      IRIT_PT_PT_DIST_SQR(Pt2, R));

    do {
	t1n = *t1 + IRIT_EPS;
	if (t1n < TMin1 || t1n > TMax1)
	    break;

	R = CagdCrvEval(Crv1, t1n);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = SymbBsctComputeInterMidPoint(Crv1, t1n, Crv2, *t2),
	NewError = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
			     IRIT_PT_PT_DIST_SQR(Pt2, R));

	/* Compute the change in error as a function of parameter change. */
	DErrDt1 = (Error - NewError) / IRIT_EPS;
	if (DErrDt1 == 0)
	    break;

	t1n = *t1 + Error / DErrDt1;
	if (t1n < TMin1 || t1n > TMax1)
	    break;

	R = CagdCrvEval(Crv1, t1n);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = SymbBsctComputeInterMidPoint(Crv1, t1n, Crv2, *t2);
	NewError = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
			     IRIT_PT_PT_DIST_SQR(Pt2, R));

	if (NewError < Error) {
	    Error = NewError;
	    Improve = TRUE;
	    *t1 = t1n;
	}
    }
    while (i++ < MAX_NUMER_ITER && Improve);

    return Improve;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the intersection point of the normals of the given two points   *
* on the given two curves.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1:      First curve of the matching mid point.                        *
*   t1:        Parameter value of first curve's mid point.                   *
*   Crv2       Second curve of the matching mid point.                       *
*   t2:        Parameter value of second curve's mid point.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   Point of intersection, statically allocated.              *
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors,			     M
*   SymbCrvBisectorsSrf2, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBsctComputeInterMidPoint, bisectors, skeleton                        M
*****************************************************************************/
CagdRType *SymbBsctComputeInterMidPoint(const CagdCrvStruct *Crv1,
					CagdRType t1,
					const CagdCrvStruct *Crv2,
					CagdRType t2)
{
    IRIT_STATIC_DATA CagdPType Inter1;
    CagdPType Pt1, Pt2, Nrml1, Nrml2, Inter2;
    CagdRType *R;
    CagdVecStruct *Vec;

    R = CagdCrvEval(Crv1, t1);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, t2);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    Vec = CagdCrvNormalXY(Crv1, t1, TRUE);
    IRIT_PT_COPY(Nrml1, Vec -> Vec);
    Vec = CagdCrvNormalXY(Crv2, t2, TRUE);
    IRIT_PT_COPY(Nrml2, Vec -> Vec);

    GM2PointsFromLineLine(Pt1, Nrml1, Pt2, Nrml2, Inter1, &t1, Inter2, &t2);
    return Inter1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface definition of a given curve or two.        M
* If Crv contains a list of two curves the bisector between the two curves   M
* is computed.  Otherwise, Crv self--bisectors are sought.  The result is a  M
* scalar surface whose zero set is the set of bisector(s) of the curves.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Either one or two curves to compute bisectors for.  Assumes  M
*		E2 curves.						     M
*   BisectFunc: If 1, normal fields are used to compute bisector surface.    M
*		If 2, tangent fields instead of tangents are used to compute M
*               the bisector surface function. 		                     M
*		If 3, using solution of normal intersection pt.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar surface whose zero set provides matching     M
*		bisecting points on Crv, if BisectFunc > 0.                  M 
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors,			     M
*   SymbCrvBisectorsSrf2, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvBisectorsSrf, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbCrvBisectorsSrf(const CagdCrvStruct *Crv, int BisectFunc)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2;
    CagdSrfStruct *Res, *Srf1X, *Srf1Y, *Srf2X, *Srf2Y, *DSrf1X, *DSrf1Y,
	*DSrf2X, *DSrf2Y, *AlphaSrf, *BetaSrf, *TSrf1, *TSrf2, *TSrf3, *TSrf4,
	*Srf1, *Srf2, *DSrf1, *DSrf2;

    if (BisectFunc == 3)
	return SymbCrvBisectorsSrf3(Crv);

    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(Crv -> Pnext ? Crv -> Pnext : Crv,
			   CAGD_PT_E2_TYPE, FALSE);
    
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
    SymbSrfSplitScalar(DSrf1, &TSrf1, &DSrf1X, &DSrf1Y, &TSrf1);
    SymbSrfSplitScalar(DSrf2, &TSrf1, &DSrf2X, &DSrf2Y, &TSrf1);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    TSrf1 = SymbSrfSub(Srf2X, Srf1X);
    TSrf2 = SymbSrfSub(Srf2Y, Srf1Y);
    
    switch (BisectFunc) {
	default:
	case 2:  /* Use tangents to compute angular bound. */
	    TSrf3 = SymbSrfMult(TSrf1, DSrf2X);
	    TSrf4 = SymbSrfMult(TSrf2, DSrf2Y);
	    AlphaSrf = SymbSrfAdd(TSrf3, TSrf4);
	    CagdSrfFree(TSrf3);
	    CagdSrfFree(TSrf4);

	    TSrf3 = SymbSrfMult(DSrf1Y, TSrf2);
	    TSrf4 = SymbSrfMult(DSrf1X, TSrf1);
	    BetaSrf = SymbSrfAdd(TSrf3, TSrf4);
	    CagdSrfFree(TSrf3);
	    CagdSrfFree(TSrf4);

	    CagdSrfFree(TSrf1);
	    CagdSrfFree(TSrf2);
	    break;
	case 1:  /* Use normal to compute angular bound. */
	    TSrf3 = SymbSrfMult(TSrf1, DSrf2Y);
	    TSrf4 = SymbSrfMult(TSrf2, DSrf2X);
	    AlphaSrf = SymbSrfSub(TSrf3, TSrf4);
	    CagdSrfFree(TSrf3);
	    CagdSrfFree(TSrf4);

	    TSrf3 = SymbSrfMult(DSrf1X, TSrf2);
	    TSrf4 = SymbSrfMult(DSrf1Y, TSrf1);
	    BetaSrf = SymbSrfSub(TSrf3, TSrf4);
	    CagdSrfFree(TSrf3);
	    CagdSrfFree(TSrf4);

	    CagdSrfFree(TSrf1);
	    CagdSrfFree(TSrf2);
	    break;
    }

    /* Adds up the components of "|| Alpha N(t) ||^2 - || Beta N(s) ||^2" */
    TSrf1 = SymbSrfMult(AlphaSrf, DSrf1X);
    Res = SymbSrfMult(TSrf1, TSrf1);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(AlphaSrf, DSrf1Y);
    TSrf2 = SymbSrfMult(TSrf1, TSrf1);
    TSrf3 = SymbSrfAdd(Res, TSrf2);
    CagdSrfFree(Res);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    Res = TSrf3;

    TSrf1 = SymbSrfMult(BetaSrf, DSrf2X);
    TSrf2 = SymbSrfMult(TSrf1, TSrf1);
    TSrf3 = SymbSrfSub(Res, TSrf2);
    CagdSrfFree(Res);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    Res = TSrf3;

    TSrf1 = SymbSrfMult(BetaSrf, DSrf2Y);
    TSrf2 = SymbSrfMult(TSrf1, TSrf1);
    TSrf3 = SymbSrfSub(Res, TSrf2);
    CagdSrfFree(Res);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    Res = TSrf3;

    CagdSrfFree(AlphaSrf);
    CagdSrfFree(BetaSrf);
    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Res -> Points, Res -> PType,
			   Res -> ULength * Res -> VLength, TRUE);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface definition of a given curve or two.        M
* If Crv contains a list of two curves the bisector between the two curves   M
* is computed.  Otherwise, Crv self--bisectors are sought.  The result is a  M
* scalar surface whose zero set is the set of bisector(s) of the curves.     M
*   Solve for the normal intersection surface in the plane and then elevate  M
* in Z using the rational function of					     M
*				    ||P - C1(s)||^2 - ||P - C2(t)||^2.       V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Either one or two curves to compute bisectors for.  Assumes  M
*		E3 curves.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    The real bisector surface for three space curves,    M
*                       if BisectFunc = 4.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors,			     M
*   SymbCrvBisectorsSrf, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D,   M
*   SymbCrvBisectorsSrf3						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvBisectorsSrf2, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbCrvBisectorsSrf2(const CagdCrvStruct *Crv)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2,
        *Res, *Denom, *NumerX, *NumerY, *NumerZ,
        *Srf1X, *Srf1Y, *Srf2X, *Srf2Y, *DSrf1X, *DSrf1Y, *DSrf2X, *DSrf2Y,
	*TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5, *TSrf6;

    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(Crv -> Pnext ? Crv -> Pnext : Crv,
			   CAGD_PT_E2_TYPE, FALSE);

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
    SymbSrfSplitScalar(DSrf1, &TSrf1, &DSrf1X, &DSrf1Y, &TSrf1);
    SymbSrfSplitScalar(DSrf2, &TSrf1, &DSrf2X, &DSrf2Y, &TSrf1);

    Denom = SymbSrfDeterminant2(DSrf1X, DSrf1Y, DSrf2X, DSrf2Y);
    TSrf1 = SymbSrfDotProd(Srf1, DSrf1);
    TSrf2 = SymbSrfDotProd(Srf2, DSrf2);
    NumerX = SymbSrfDeterminant2(TSrf1, DSrf1Y, TSrf2, DSrf2Y);
    NumerY = SymbSrfDeterminant2(DSrf1X, TSrf1, DSrf2X, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);

    /* Compute || P - C1(s) ||^2 - || P - C2(t) ||^2 */
    TSrf1 = SymbSrfMult(Srf1X, Denom);
    TSrf2 = SymbSrfSub(TSrf1, NumerX);
    TSrf3 = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(Srf1Y, Denom);
    TSrf2 = SymbSrfSub(TSrf1, NumerY);
    TSrf4 = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf5 = SymbSrfAdd(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    TSrf1 = SymbSrfMult(Srf2X, Denom);
    TSrf2 = SymbSrfSub(TSrf1, NumerX);
    TSrf3 = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(Srf2Y, Denom);
    TSrf2 = SymbSrfSub(TSrf1, NumerY);
    TSrf4 = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf6 = SymbSrfAdd(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    NumerZ = SymbSrfSub(TSrf5, TSrf6);
    CagdSrfFree(TSrf5);
    CagdSrfFree(TSrf6);

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);

    CagdMakeSrfsCompatible(&NumerZ, &NumerX, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&NumerZ, &NumerY, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&NumerZ, &Denom,  TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&NumerY, &NumerX, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&NumerY, &Denom,  TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&NumerX, &Denom,  TRUE, TRUE, TRUE, TRUE);

    Res = SymbSrfMergeScalar(Denom, NumerX, NumerY, NumerZ);

    CagdSrfFree(NumerX);
    CagdSrfFree(NumerY);
    CagdSrfFree(NumerZ);
    CagdSrfFree(Denom);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Res -> Points, Res -> PType,
			   Res -> ULength * Res -> VLength, TRUE);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface definition of a given curve or two.        M
* If Crv contains a list of two curves the bisector between the two curves   M
* is computed.  Otherwise, Crv self--bisectors are sought.  The result is a  M
* scalar surface whose zero set is the set of bisector(s) of the curves.     M
*   Solve for the normal intersection surface in the plane and then	     M
* substitute into (the bisector's correspondance is the zero set then).      M
*	      C1(s) + C2(t)	                                             V
*	< P - -------------, C1(t) - C2(s) > = 0.                            V
*		    2		                                             V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Either one or two curves to compute bisectors for.  Assumes  M
*		E2 curves.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar surface whose zero set provides matching     M
*		bisecting points on Crv, if BisectFunc = 3.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors,SymbCrvBisectorsSrf2  M
*   SymbCrvBisectorsSrf, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvBisectorsSrf3, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbCrvBisectorsSrf3(const CagdCrvStruct *Crv)
{
    IRIT_STATIC_DATA CagdVType
	Scale = { 0.5, 0.5, 0.5 };
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2,
        *Res, *Denom, *NumerX, *NumerY,
        *Srf1X, *Srf1Y, *Srf2X, *Srf2Y, *DSrf1X, *DSrf1Y, *DSrf2X, *DSrf2Y,
	*TSrf1, *TSrf2, *TSrf3;

    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(Crv -> Pnext ? Crv -> Pnext : Crv,
			   CAGD_PT_E2_TYPE, FALSE);

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
    SymbSrfSplitScalar(DSrf1, &TSrf1, &DSrf1X, &DSrf1Y, &TSrf1);
    SymbSrfSplitScalar(DSrf2, &TSrf1, &DSrf2X, &DSrf2Y, &TSrf1);

    Denom = SymbSrfDeterminant2(DSrf1X, DSrf1Y, DSrf2X, DSrf2Y);
    TSrf1 = SymbSrfDotProd(Srf1, DSrf1);
    TSrf2 = SymbSrfDotProd(Srf2, DSrf2);
    NumerX = SymbSrfDeterminant2(TSrf1, DSrf1Y, TSrf2, DSrf2Y);
    NumerY = SymbSrfDeterminant2(DSrf1X, TSrf1, DSrf2X, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);

    /* Compute:								     */
    /*									     */
    /*	      C1(s) + C2(t)	                                             */
    /*	< P - -------------, C1(t) - C2(s) > = 0.                            */
    /*		    2		                                             */
    
    TSrf1 = SymbSrfAdd(Srf1, Srf2);
    CagdSrfScale(TSrf1, Scale);
    TSrf2 = SymbSrfMultScalar(TSrf1, Denom);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfMergeScalar(NULL, NumerX, NumerY, NULL);
    TSrf1 = SymbSrfSub(TSrf3, TSrf2);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);

    TSrf2 = SymbSrfSub(Srf1, Srf2);
    Res = SymbSrfDotProd(TSrf1, TSrf2);

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);

    CagdSrfFree(NumerX);
    CagdSrfFree(NumerY);
    CagdSrfFree(Denom);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Res -> Points, Res -> PType,
			   Res -> ULength * Res -> VLength, TRUE);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface of two curve in arbitrary general three    M
* space position.    				                             M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2: Two three space curves to compute their bisector surface.  M
*   Alpha:        Alpha-sector ratio (0.5 for a bisector).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The bisector surface.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter, SymbCrvCnvxHull, SymbCrvBisectorsSrf,                   M
*   SymbCrvPtBisectorSrf3D						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvBisectorSrf3D, bisectors, skeleton                             M
*****************************************************************************/
CagdSrfStruct *SymbCrvCrvBisectorSrf3D(const CagdCrvStruct *CCrv1,
				       const CagdCrvStruct *CCrv2,
				       CagdRType Alpha)
{
    CagdSrfStruct *BisectSrf;
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *DCrv1, *DCrv2, *Crv1, *Crv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2;
    CagdBType
        IsRational1 = CAGD_IS_RATIONAL_PT(CCrv1 -> PType),
        IsRational2 = CCrv2 ? CAGD_IS_RATIONAL_PT(CCrv2 -> PType) 
			    : IsRational1,
	IsRational = IsRational1 || IsRational2;
    
    Crv1 = CagdCoerceCrvTo(CCrv1, IsRational ? CAGD_PT_P3_TYPE
			                     : CAGD_PT_E3_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(CCrv2 ? CCrv2 : CCrv2,
			   IsRational ? CAGD_PT_P3_TYPE
			              : CAGD_PT_E3_TYPE, FALSE);
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
    
    BspKnotAffineTrans2(Srf1 -> VKnotVector,
			Srf1 -> VLength + Srf1 -> VOrder, VMin2, VMax2);
    BspKnotAffineTrans2(Srf2 -> UKnotVector,
			Srf2 -> ULength + Srf2 -> UOrder, UMin1, UMax1);
    
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

    /* Note this function free the input surfaces. */
    BisectSrf = SymbCrvBisectorsSrf3D(Srf1, Srf2, DSrf1, DSrf2, Alpha);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the expression of a 3 by 3 determinants.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf11, Srf12, Srf13:  The nine factors of the determinant.               M
*   Srf21, Srf22, Srf23:                 "				     M
*   Srf31, Srf32, Srf33:                 "				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: A scalar field representing the determinant computation.M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfGaussCurvature, SymbSrfMeanEvolute,	     M
*   SymbSrfMeanCurvatureSqr, SymbSrfIsoFocalSrf, SymbSrfCurvatureUpperBound, M
*   SymbSrfIsoDirNormalCurvatureBound, SymbSrfDeterminant2,		     M
*   SymbCrvDeterminant3							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDeterminant3, determinant                                         M
*****************************************************************************/
CagdSrfStruct *SymbSrfDeterminant3(const CagdSrfStruct *Srf11,
				   const CagdSrfStruct *Srf12,
				   const CagdSrfStruct *Srf13,
				   const CagdSrfStruct *Srf21,
				   const CagdSrfStruct *Srf22,
				   const CagdSrfStruct *Srf23,
				   const CagdSrfStruct *Srf31,
				   const CagdSrfStruct *Srf32,
				   const CagdSrfStruct *Srf33)
{
    CagdSrfStruct
	*Prod1 = SymbSrfDeterminant2(Srf22, Srf23, Srf32, Srf33),
        *Prod1a = SymbSrfMult(Srf11, Prod1),
	*Prod2 = SymbSrfDeterminant2(Srf21, Srf23, Srf31, Srf33),
        *Prod2a = SymbSrfMult(Srf12, Prod2),
	*Prod3 = SymbSrfDeterminant2(Srf21, Srf22, Srf31, Srf32),
        *Prod3a = SymbSrfMult(Srf13, Prod3),
	*Sub12 = SymbSrfSub(Prod1a, Prod2a),
	*Add123 = SymbSrfAdd(Sub12, Prod3a);

    CagdSrfFree(Prod1);
    CagdSrfFree(Prod1a);
    CagdSrfFree(Prod2);
    CagdSrfFree(Prod2a);
    CagdSrfFree(Prod3);
    CagdSrfFree(Prod3a);
    CagdSrfFree(Sub12);

    return Add123;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the bisector surface of two curves in arbitrary general three   *
* space position.  Given are the curves and their derivatives as curves      *
* already promoted to surfaces.	Input surfaces are freed!		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1:    The first curve promoted to a surface.                          *
*   Srf2:    The second curve promoted to a surface.                         *
*   DSrf1:   The derivative of the first curve, promoted to a surface.       *
*   DSrf2:   The derivative of the second curve, promoted to a surface.      *
*   Alpha:   Alpha-sector ratio.	                		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:   The bisector surface between these two curves.        *
*****************************************************************************/
CagdSrfStruct *SymbCrvBisectorsSrf3D(CagdSrfStruct *Srf1,
				     CagdSrfStruct *Srf2,
				     CagdSrfStruct *DSrf1,
				     CagdSrfStruct *DSrf2,
				     CagdRType Alpha)
{
    CagdSrfStruct *Ret, *TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5,
	*Srf1W, *Srf1X, *Srf1Y, *Srf1Z, *Srf2W, *Srf2X, *Srf2Y, *Srf2Z,
	*DSrf1W, *DSrf1X, *DSrf1Y, *DSrf1Z, *DSrf2W, *DSrf2X, *DSrf2Y, *DSrf2Z,
	*D1Srf, *D2Srf, *E12X, *E12Y, *E12Z, *M12Srf;

    SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
    SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);
    SymbSrfSplitScalar(DSrf1, &DSrf1W, &DSrf1X, &DSrf1Y, &DSrf1Z);
    SymbSrfSplitScalar(DSrf2, &DSrf2W, &DSrf2X, &DSrf2Y, &DSrf2Z);
    if (DSrf1W)
	CagdSrfFree(DSrf1W);
    if (DSrf2W)
	CagdSrfFree(DSrf2W);

    if (Srf1W != NULL && Srf2W != NULL) {
	CagdSrfStruct *Srf1E3, *Srf2E3, *DSrf1E3, *DSrf2E3;

	Srf1E3 = SymbSrfMergeScalar(NULL, Srf1X, Srf1Y, Srf1Z);
	Srf2E3 = SymbSrfMergeScalar(NULL, Srf2X, Srf2Y, Srf2Z);
	DSrf1E3 = SymbSrfMergeScalar(NULL, DSrf1X, DSrf1Y, DSrf1Z);
	DSrf2E3 = SymbSrfMergeScalar(NULL, DSrf2X, DSrf2Y, DSrf2Z);

	D1Srf = SymbSrfDotProd(Srf1E3, DSrf1E3);
	D2Srf = SymbSrfDotProd(Srf2E3, DSrf2E3);
	CagdSrfFree(Srf1E3);
	CagdSrfFree(Srf2E3);
	CagdSrfFree(DSrf1E3);
	CagdSrfFree(DSrf2E3);

	/* Place the weights where there are affecting the equations. */
	MULT_AND_REPLACE_FIRST(DSrf1X, Srf1W);
	MULT_AND_REPLACE_FIRST(DSrf1Y, Srf1W);
	MULT_AND_REPLACE_FIRST(DSrf1Z, Srf1W);
	MULT_AND_REPLACE_FIRST(DSrf2X, Srf2W);
	MULT_AND_REPLACE_FIRST(DSrf2Y, Srf2W);
	MULT_AND_REPLACE_FIRST(DSrf2Z, Srf2W);

	MULT_AND_REPLACE_FIRST(Srf1X, Srf2W);
	MULT_AND_REPLACE_FIRST(Srf1Y, Srf2W);
	MULT_AND_REPLACE_FIRST(Srf1Z, Srf2W);
	MULT_AND_REPLACE_FIRST(Srf2X, Srf1W);
	MULT_AND_REPLACE_FIRST(Srf2Y, Srf1W);
	MULT_AND_REPLACE_FIRST(Srf2Z, Srf1W);

	/* Compute C1 w1 w2^2 - C2 w2 w1^2. */
	TSrf1 = SymbSrfMult(Srf1W, Srf2W);

	TSrf3 = SymbSrfMult(Srf1X, TSrf1);
	TSrf4 = SymbSrfMult(Srf2X, TSrf1);
	E12X = SymbSrfSub(TSrf3, TSrf4);
	CagdSrfFree(TSrf3);
	CagdSrfFree(TSrf4);

	TSrf3 = SymbSrfMult(Srf1Y, TSrf1);
	TSrf4 = SymbSrfMult(Srf2Y, TSrf1);
	E12Y = SymbSrfSub(TSrf3, TSrf4);
	CagdSrfFree(TSrf3);
	CagdSrfFree(TSrf4);

	TSrf3 = SymbSrfMult(Srf1Z, TSrf1);
	TSrf4 = SymbSrfMult(Srf2Z, TSrf1);
	E12Z = SymbSrfSub(TSrf3, TSrf4);
	CagdSrfFree(TSrf3);
	CagdSrfFree(TSrf4);

	CagdSrfFree(TSrf1);

	/* Compute <((1.0 - Alpha) w2 C1 + Alpha w1 C2), (w2 C1 - w1 C2)>. */
	TSrf1 = SymbSrfMergeScalar(NULL, Srf1X, Srf1Y, Srf1Z);
	TSrf2 = SymbSrfMergeScalar(NULL, Srf2X, Srf2Y, Srf2Z);
	TSrf4 = SymbSrfSub(TSrf1, TSrf2);

	CagdSrfTransform(TSrf1, NULL, 1.0 - Alpha);
	CagdSrfTransform(TSrf2, NULL, Alpha);
	TSrf5 = SymbSrfAdd(TSrf1, TSrf2);

	M12Srf = SymbSrfDotProd(TSrf4, TSrf5);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf4);
	CagdSrfFree(TSrf5);
    }
    else {
	D1Srf = SymbSrfDotProd(Srf1, DSrf1);
	D2Srf = SymbSrfDotProd(Srf2, DSrf2);

	TSrf4 = SymbSrfSub(Srf2, Srf1);
	SymbSrfSplitScalar(TSrf4, &TSrf1, &E12X, &E12Y, &E12Z);

	CagdSrfTransform(Srf1, NULL, 1.0 - Alpha); /* Modifies input Srf1/2! */
	CagdSrfTransform(Srf2, NULL, Alpha);
	TSrf5 = SymbSrfAdd(Srf1, Srf2);

	M12Srf = SymbSrfDotProd(TSrf4, TSrf5);

	CagdSrfFree(TSrf4);
	CagdSrfFree(TSrf5);
    }

    TSrf1 = SymbSrfDeterminant3(DSrf1X, DSrf1Y, DSrf1Z,
				DSrf2X, DSrf2Y, DSrf2Z,
				E12X,   E12Y,   E12Z);
    TSrf2 = SymbSrfDeterminant3(D1Srf,  DSrf1Y, DSrf1Z,
				D2Srf,  DSrf2Y, DSrf2Z,
				M12Srf, E12Y,   E12Z);
    TSrf3 = SymbSrfDeterminant3(DSrf1X, D1Srf,  DSrf1Z,
				DSrf2X, D2Srf,  DSrf2Z,
				E12X,   M12Srf, E12Z);
    TSrf4 = SymbSrfDeterminant3(DSrf1X, DSrf1Y, D1Srf,
				DSrf2X, DSrf2Y, D2Srf,
				E12X,   E12Y,   M12Srf);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf4, TRUE, TRUE, TRUE, TRUE);

    Ret = SymbSrfMergeScalar(TSrf1, TSrf2, TSrf3, TSrf4);

    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    CagdSrfFree(D1Srf);
    CagdSrfFree(D2Srf);
    CagdSrfFree(M12Srf);
    CagdSrfFree(E12X);
    CagdSrfFree(E12Y);
    CagdSrfFree(E12Z);

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf1Z);
    if (Srf1W)
	CagdSrfFree(Srf1W);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(Srf2Z);
    if (Srf2W)
	CagdSrfFree(Srf2W);

    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf1Z);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);
    CagdSrfFree(DSrf2Z);

    CagdSrfFree(Srf1);				     /* Free given surfaces! */
    CagdSrfFree(Srf2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the alpha-/bi-sector curve of a planar curve a point, all in    M
* the XY plane.	 The result is the solution to the following two linear      M
* equations in alpha-/bi-sector's two unknowns, the x and y coefficients:    M
*									     M
*	<C'(t),     B(t)> = <C'(t), C(t)>				     V
*	<C(t) - Pt, B(t)> = <C(t) - Pt, a Pt + (1 - a) C(t)>		     V
*									     M
* where a is the Alpha of the alpha-sector, 0.5 for a bisector, Pt is the    M
* point entity, C(t) is the curve entity and B(t) is the sought bisector.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         Planar curve to compute its bisector curve with Pt.        M
*   Pt:           A point in the plane to compute its bisector with Crv.     M
*   Alpha:        Alpha-sector ratio (0.5 for a bisector).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The bisector curve, in the XY plane.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter, SymbCrvCnvxHull, SymbCrvBisectorsSrf,                   M
*   SymbCrvCrvBisectorSrf3D, SymbSrfPtBisectorSrf3D, SymbCrvPtBisectorSrf3D  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvPtBisectorCrv2D, bisector                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrvPtBisectorCrv2D(const CagdCrvStruct *CCrv,
				      const CagdPType Pt,
				      CagdRType Alpha)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(CCrv -> PType);
    CagdPType Trans;
    CagdCrvStruct *Crv, *DCrv, *CrvE3, *DCrvE3, *TCrv1, *TCrv2, *TCrv3, *TCrv4,
	*A11, *A12, *A21, *A22, *B1, *B2, *CrvW, *Ret;

    Crv = CagdCoerceCrvTo(CCrv, IsRational ? CAGD_PT_P2_TYPE
					   : CAGD_PT_E2_TYPE, FALSE);
    DCrv = CagdCrvDerive(Crv);
    if (IsRational) {
        /* Constructs an E2/E3 version of these two rational curves. */
	SymbCrvSplitScalar(DCrv, &CrvW, &TCrv1, &TCrv2, &TCrv3);
	CagdCrvFree(CrvW);
	DCrvE3 = SymbCrvMergeScalar(NULL, TCrv1, TCrv2, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);
	SymbCrvSplitScalar(Crv, &CrvW, &TCrv1, &TCrv2, &TCrv3);
	CrvE3 = SymbCrvMergeScalar(NULL, TCrv1, TCrv2, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);

        /* Prepare the 2x2 determinant coefficients. */
	TCrv1 = SymbCrvMultScalar(DCrvE3, CrvW);
	SymbCrvSplitScalar(TCrv1, &TCrv2, &A11, &A12, &TCrv3);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);

	DUPLICATE_E1_TO_E3_CRV(TCrv4, CrvW);
	CagdCrvScale(TCrv4, Pt);
	TCrv3 = SymbCrvSub(CrvE3, TCrv4);
	TCrv2 = CagdCrvCopy(TCrv3);
	TCrv1 = SymbCrvMultScalar(TCrv2, CrvW);
	CagdCrvFree(TCrv2);
	CagdCrvFree(CrvW);
	SymbCrvSplitScalar(TCrv1, &CrvW, &A21, &A22, &TCrv2);
	CagdCrvFree(TCrv1);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	/* And the B coefficients in the "Ax = B" system. */
	B1 = SymbCrvDotProd(DCrvE3, CrvE3);

	CagdCrvTransform(TCrv4, NULL, Alpha);
	TCrv2 = CagdCrvCopy(CrvE3);
	CagdCrvTransform(TCrv2, NULL, 1.0 - Alpha);
	TCrv1 = SymbCrvAdd(TCrv2, TCrv4);
	B2 = SymbCrvDotProd(TCrv1, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	CagdCrvFree(TCrv3);
	CagdCrvFree(TCrv4);

	CagdCrvFree(CrvE3);
	CagdCrvFree(DCrvE3);
    }
    else {
        /* Prepare the 2x2 determinant coefficients. */
	SymbCrvSplitScalar(DCrv, &CrvW, &A11, &A12, &TCrv2);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	Trans[0] = -Pt[0];
	Trans[1] = -Pt[1];
	Trans[2] = 0.0;
	TCrv3 = CagdCrvCopy(Crv);
	CagdCrvTransform(TCrv3, Trans, 1.0);
	SymbCrvSplitScalar(TCrv3, &CrvW, &A21, &A22, &TCrv2);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	/* And the B coefficients in the "Ax = B" system. */
        B1 = SymbCrvDotProd(DCrv, Crv);

	TCrv1 = CagdCrvCopy(Crv);
	CagdCrvTransform(TCrv1, NULL, 1.0 - Alpha);
	Trans[0] = Alpha * Pt[0];
	Trans[1] = Alpha * Pt[1];
	Trans[2] = 0.0;
	CagdCrvTransform(TCrv1, Trans, 1.0);
	B2 = SymbCrvDotProd(TCrv1, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv3);
    }

    TCrv1 = SymbCrvDeterminant2(A11, A12,
				A21, A22);
    TCrv2 = SymbCrvDeterminant2(B1, A12,
				B2, A22);
    TCrv3 = SymbCrvDeterminant2(A11, B1,
				A21, B2);
    CagdCrvFree(A11);
    CagdCrvFree(A12);
    CagdCrvFree(A21);
    CagdCrvFree(A22);
    CagdCrvFree(B1);
    CagdCrvFree(B2);

    CagdMakeCrvsCompatible(&TCrv1, &TCrv2, TRUE, TRUE);
    CagdMakeCrvsCompatible(&TCrv1, &TCrv3, TRUE, TRUE);

    Ret = SymbCrvMergeScalar(TCrv1, TCrv2, TCrv3, NULL);

    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);
    CagdCrvFree(TCrv3);

    CagdCrvFree(DCrv);
    CagdCrvFree(Crv);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Ret -> Points, Ret -> PType, Ret -> Length, TRUE);

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface of a curve in arbitrary general three      M
* space position and a point in three space.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         Three space curve to compute its bisector surface with Pt. M
*   Pt:           A point in three space to compute its bisector with Crv.   M
*   RulingScale:  The scaling factor for the ruling direction.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The bisector surface.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter, SymbCrvCnvxHull, SymbCrvBisectorsSrf,                   M
*   SymbCrvCrvBisectorSrf3D, SymbSrfPtBisectorSrf3D, SymbCrvPtBisectorCrv2D  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvPtBisectorSrf3D, bisector                                         M
*****************************************************************************/
CagdSrfStruct *SymbCrvPtBisectorSrf3D(const CagdCrvStruct *CCrv,
				      const CagdPType Pt,
				      CagdRType RulingScale)
{
    int i;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(CCrv -> PType);
    CagdPType Trans;
    CagdPtStruct *Pt1, *Pt2;
    CagdCrvStruct *Crv, *DCrv, *ECrv, *FCrv, *GCrv, *DCrvE3, *CrvE3,
	*TCrv1, *TCrv2, *TCrv3, *TCrv4, *TCrv5, *TCrv6, *FCrvW, *FCrvX,
	*DCrvW, *DCrvX, *DCrvY, *DCrvZ, *ECrvW, *ECrvX, *ECrvY, *ECrvZ,
	*GCrvW, *GCrvX, *GCrvY, *GCrvZ, *CrvW, *CrvX, *CrvY, *CrvZ;
    CagdSrfStruct *Ret, *TSrf1, *TSrf2, *TSrf3, *TSrf4;

    Crv = CagdCoerceCrvTo(CCrv, IsRational ? CAGD_PT_P3_TYPE
					   : CAGD_PT_E3_TYPE, FALSE);
    DCrv = CagdCrvDerive(Crv);
    if (IsRational) {
	SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
	SymbCrvSplitScalar(DCrv, &DCrvW, &DCrvX, &DCrvY, &DCrvZ);
	CrvE3 = SymbCrvMergeScalar(NULL, CrvX, CrvY, CrvZ);
	DCrvE3 = SymbCrvMergeScalar(NULL, DCrvX, DCrvY, DCrvZ);

	TCrv4 = SymbCrvMult(CrvW, CrvW);
	DUPLICATE_E1_TO_E3_CRV(TCrv1, TCrv4);
	CagdCrvScale(TCrv1, Pt);
	DUPLICATE_E1_TO_E3_CRV(TCrv5, CrvW);
	TCrv2 = SymbCrvMult(CrvE3, TCrv5);
	GCrv = SymbCrvSub(TCrv2, TCrv1);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);

	IRIT_PT_RESET(Trans);
	Trans[0] = IRIT_DOT_PROD(Pt, Pt);
	CagdCrvScale(TCrv4, Trans);
	TCrv1 = SymbCrvDotProd(CrvE3, CrvE3);
	FCrv = SymbCrvSub(TCrv1, TCrv4);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv4);
	CagdCrvTransform(FCrv, NULL, 0.5);

	CagdCrvFree(DCrv);
	DCrv = SymbCrvMult(DCrvE3, TCrv5);

	CagdCrvScale(TCrv5, Pt);
	TCrv2 = SymbCrvSub(CrvE3, TCrv5);
	ECrv = SymbCrvCrossProd(TCrv2, DCrvE3);
	CagdCrvFree(TCrv2);
	CagdCrvFree(TCrv5);

	TCrv1 = SymbCrvDotProd(CrvE3, DCrv);
	TCrv2 = SymbCrvDotProd(CrvE3, ECrv);

	CagdCrvFree(CrvW);
	CagdCrvFree(CrvX);
	CagdCrvFree(CrvY);
	CagdCrvFree(CrvZ);
	CagdCrvFree(DCrvW);
	CagdCrvFree(DCrvX);
	CagdCrvFree(DCrvY);
	CagdCrvFree(DCrvZ);
	CagdCrvFree(CrvE3);
	CagdCrvFree(DCrvE3);
    }
    else {
	GCrv = CagdCrvCopy(Crv);
	IRIT_PT_SCALE2(Trans, Pt, -1);
	CagdCrvTransform(GCrv, Trans, 1.0);
	ECrv = SymbCrvCrossProd(GCrv, DCrv);

	FCrv = SymbCrvDotProd(Crv, Crv);
	CagdCrvTransform(FCrv, NULL, 0.5);
	Trans[0] = -IRIT_DOT_PROD(Pt, Pt) * 0.5;
	CagdCrvTransform(FCrv, Trans, 1.0);

	TCrv1 = SymbCrvDotProd(Crv, DCrv);
	TCrv2 = SymbCrvDotProd(Crv, ECrv);
    }

    SymbCrvSplitScalar(FCrv, &FCrvW, &FCrvX, &TCrv3, &TCrv3);
    SymbCrvSplitScalar(DCrv, &DCrvW, &DCrvX, &DCrvY, &DCrvZ);
    SymbCrvSplitScalar(ECrv, &ECrvW, &ECrvX, &ECrvY, &ECrvZ);
    SymbCrvSplitScalar(GCrv, &GCrvW, &GCrvX, &GCrvY, &GCrvZ);

    TCrv3 = SymbCrvDeterminant3(GCrvX, GCrvY, GCrvZ,
				DCrvX, DCrvY, DCrvZ,
				ECrvX, ECrvY, ECrvZ);
    TCrv4 = SymbCrvDeterminant3(FCrv,  GCrvY, GCrvZ,
				TCrv1, DCrvY, DCrvZ,
				TCrv2, ECrvY, ECrvZ);
    TCrv5 = SymbCrvDeterminant3(GCrvX, FCrv,  GCrvZ,
				DCrvX, TCrv1, DCrvZ,
				ECrvX, TCrv2, ECrvZ);
    TCrv6 = SymbCrvDeterminant3(GCrvX, GCrvY, FCrv,
				DCrvX, DCrvY, TCrv1,
				ECrvX, ECrvY, TCrv2);
    CagdMakeCrvsCompatible(&TCrv3, &TCrv4, TRUE, TRUE);
    CagdMakeCrvsCompatible(&TCrv3, &TCrv5, TRUE, TRUE);
    CagdMakeCrvsCompatible(&TCrv3, &TCrv6, TRUE, TRUE);

    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    TCrv1 = SymbCrvMergeScalar(TCrv3, TCrv4, TCrv5, TCrv6);
    CagdCrvFree(TCrv3);
    CagdCrvFree(TCrv4);
    CagdCrvFree(TCrv5);
    CagdCrvFree(TCrv6);

    CagdCrvFree(Crv);
    CagdCrvFree(DCrv);
    CagdCrvFree(FCrv);
    CagdCrvFree(GCrv);

    CagdCrvFree(DCrvX);
    CagdCrvFree(DCrvY);
    CagdCrvFree(DCrvZ);
    CagdCrvFree(ECrvX);
    CagdCrvFree(ECrvY);
    CagdCrvFree(ECrvZ);
    CagdCrvFree(FCrvX);
    CagdCrvFree(GCrvX);
    CagdCrvFree(GCrvY);
    CagdCrvFree(GCrvZ);

    /* Convert the ruled surface directrix (TSrf1) into a surface in the U   */
    /* direction and add the ruling direction as V direction.		     */
    TSrf1 = CagdPromoteCrvToSrf(TCrv1, CAGD_CONST_U_DIR);
    CagdCrvFree(TCrv1);

    TSrf2 = CagdPromoteCrvToSrf(ECrv, CAGD_CONST_U_DIR);
    CagdCrvFree(ECrv);

    Pt1 = CagdPtNew();
    Pt2 = CagdPtNew();
    for (i = 0; i < 3; i++) {
	Pt1 -> Pt[i] = -RulingScale;
	Pt2 -> Pt[i] = RulingScale;
    }
    TCrv2 = CagdMergePtPt(Pt1, Pt2);
    CagdPtFree(Pt1);
    CagdPtFree(Pt2);

    TSrf3 = CagdPromoteCrvToSrf(TCrv2, CAGD_CONST_V_DIR);
    CagdCrvFree(TCrv2);

    if (CAGD_IS_BSPLINE_SRF(TSrf2)) {
	CagdRType UMin, UMax, VMin, VMax;

	TSrf4 = CagdCnvrtBzr2BspSrf(TSrf3);
	CagdSrfFree(TSrf3);

	CagdSrfDomain(TSrf2, &UMin, &UMax, &VMin, &VMax);
	BspKnotAffineTrans2(TSrf4 -> UKnotVector,
			    TSrf4 -> ULength + TSrf4 -> UOrder,
			    UMin, UMax);
	TSrf3 = TSrf4;
    }

    TSrf4 = SymbSrfMult(TSrf2, TSrf3);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);

    Ret = SymbSrfAdd(TSrf1, TSrf4);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf4);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Ret -> Points, Ret -> PType,
			   Ret -> ULength * Ret -> VLength, TRUE);

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface of a surface in arbitrary general three    M
* space position and a point in three space.                                 M
*   Solution bisector surface R is derived by solving the three linear       M
* equations of (S for Srf, P for Pt):					     M
*	< dS/du, R > = < dS/du, S >					     V
*	< dS/dv, R > = < dS/dv, S >					     V
*	< S - P, R > = (< S, S > - < P, P >) / 2			     V
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:      Three space surface too compute its bisector surface with Pt. M
*   Pt:        A point in three space to compute its bisector with Srf.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The bisector surface.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter, SymbCrvCnvxHull, SymbCrvBisectorsSrf,                   M
*   SymbCrvCrvBisectorSrf3D				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfPtBisectorSrf3D, bisector                                         M
*****************************************************************************/
CagdSrfStruct *SymbSrfPtBisectorSrf3D(const CagdSrfStruct *CSrf,
				      const CagdPType Pt)
{
   CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(CSrf -> PType);
    CagdPType Trans;
    CagdSrfStruct *Srf, *DuSrfW, *DuSrfX, *DuSrfY, *DuSrfZ,
	*DvSrfW, *DvSrfX, *DvSrfY, *DvSrfZ,
	*DuSrf, *DvSrf, *B1Srf, *B2Srf, *B3Srf, *SrfXPt, *SrfYPt, *SrfZPt,
	*TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5, *TSrf6;

    Srf = CagdCoerceSrfTo(CSrf, IsRational ? CAGD_PT_P3_TYPE 
					   : CAGD_PT_E3_TYPE, FALSE);

    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

    /* Forming the Ax = B system. */
    if (IsRational) {
	CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *SrfE3, *DuSrfE3, *DvSrfE3;

	SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
	SymbSrfSplitScalar(DuSrf, &DuSrfW, &DuSrfX, &DuSrfY, &DuSrfZ);
	SymbSrfSplitScalar(DvSrf, &DvSrfW, &DvSrfX, &DvSrfY, &DvSrfZ);

	SrfE3 = SymbSrfMergeScalar(NULL, SrfX, SrfY, SrfZ);
	DuSrfE3 = SymbSrfMergeScalar(NULL, DuSrfX, DuSrfY, DuSrfZ);
	DvSrfE3 = SymbSrfMergeScalar(NULL, DvSrfX, DvSrfY, DvSrfZ);
	CagdSrfFree(DuSrfX);
	CagdSrfFree(DuSrfY);
	CagdSrfFree(DuSrfZ);
	CagdSrfFree(DuSrfW);
	CagdSrfFree(DvSrfX);
	CagdSrfFree(DvSrfY);
	CagdSrfFree(DvSrfZ);
	CagdSrfFree(DvSrfW);
	CagdSrfFree(SrfX);
	CagdSrfFree(SrfY);
	CagdSrfFree(SrfZ);

	B1Srf = SymbSrfDotProd(SrfE3, DuSrfE3);
	B2Srf = SymbSrfDotProd(SrfE3, DvSrfE3);

	TSrf1 = SymbSrfMult(SrfW, SrfW);
	CagdSrfTransform(TSrf1, NULL, IRIT_DOT_PROD(Pt, Pt));
	TSrf2 = SymbSrfDotProd(SrfE3, SrfE3);
	B3Srf = SymbSrfSub(TSrf2, TSrf1);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
	CagdSrfTransform(B3Srf, NULL, 0.5);

	/* Now the A matrix' coefficients: */
	DUPLICATE_E1_TO_E3_SRF(TSrf6, SrfW);

	CagdSrfFree(DuSrf);
	CagdSrfFree(DvSrf);

	DuSrf = SymbSrfMult(DuSrfE3, TSrf6);
	DvSrf = SymbSrfMult(DvSrfE3, TSrf6);

	IRIT_PT_COPY(Trans, Pt);
	IRIT_PT_SCALE(Trans, -1.0);
	TSrf1 = SymbSrfMult(SrfW, SrfW);
	DUPLICATE_E1_TO_E3_SRF(TSrf3, TSrf1);
	CagdSrfFree(TSrf1);
	CagdSrfScale(TSrf3, Pt);

	TSrf2 = SymbSrfMult(SrfE3, TSrf6);
	CagdSrfFree(TSrf6);
	TSrf1 = SymbSrfSub(TSrf2, TSrf3);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf3);

	CagdSrfFree(Srf);
	Srf = SrfE3;

	CagdSrfFree(SrfW);
	CagdSrfFree(DuSrfE3);
	CagdSrfFree(DvSrfE3);
    }
    else {
	B1Srf = SymbSrfDotProd(Srf, DuSrf);
	B2Srf = SymbSrfDotProd(Srf, DvSrf);
	B3Srf = SymbSrfDotProd(Srf, Srf);
	IRIT_PT_RESET(Trans);
	Trans[0] = -IRIT_DOT_PROD(Pt, Pt);
	CagdSrfTransform(B3Srf, Trans, 0.5);

	TSrf1 = CagdSrfCopy(Srf);
	IRIT_PT_COPY(Trans, Pt);
	IRIT_PT_SCALE(Trans, -1.0);
	CagdSrfTransform(TSrf1, Trans, 1.0);
    }

    SymbSrfSplitScalar(DuSrf, &TSrf2, &DuSrfX, &DuSrfY, &DuSrfZ);
    SymbSrfSplitScalar(DvSrf, &TSrf2, &DvSrfX, &DvSrfY, &DvSrfZ);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &SrfXPt, &SrfYPt, &SrfZPt);
    CagdSrfFree(TSrf1);

    /* And solving the linear system. */
    TSrf3 = SymbSrfDeterminant3(DuSrfX, DuSrfY, DuSrfZ,
				DvSrfX, DvSrfY, DvSrfZ,
				SrfXPt, SrfYPt, SrfZPt);
    TSrf4 = SymbSrfDeterminant3(B1Srf,  DuSrfY, DuSrfZ,
				B2Srf,  DvSrfY, DvSrfZ,
				B3Srf,  SrfYPt, SrfZPt);
    TSrf5 = SymbSrfDeterminant3(DuSrfX, B1Srf,  DuSrfZ,
				DvSrfX, B2Srf,  DvSrfZ,
				SrfXPt, B3Srf,  SrfZPt);
    TSrf6 = SymbSrfDeterminant3(DuSrfX, DuSrfY, B1Srf,
				DvSrfX, DvSrfY, B2Srf,
				SrfXPt, SrfYPt, B3Srf);
    CagdSrfFree(DuSrfX);
    CagdSrfFree(DuSrfY);
    CagdSrfFree(DuSrfZ);
    CagdSrfFree(DvSrfX);
    CagdSrfFree(DvSrfY);
    CagdSrfFree(DvSrfZ);
    CagdSrfFree(SrfXPt);
    CagdSrfFree(SrfYPt);
    CagdSrfFree(SrfZPt);
    CagdSrfFree(B1Srf);
    CagdSrfFree(B2Srf);
    CagdSrfFree(B3Srf);

    CagdMakeSrfsCompatible(&TSrf3, &TSrf4, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf3, &TSrf5, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf3, &TSrf6, TRUE, TRUE, TRUE, TRUE);

    TSrf1 = SymbSrfMergeScalar(TSrf3, TSrf4, TSrf5, TSrf6);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);
    CagdSrfFree(TSrf5);
    CagdSrfFree(TSrf6);

    CagdSrfFree(Srf);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(TSrf1 -> Points, TSrf1 -> PType,
			   TSrf1 -> ULength * TSrf1 -> VLength, TRUE);

    return TSrf1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the expression of a 3 by 3 determinants.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv11, Crv12, Crv13:  The nine factors of the determinant.		     M
*   Crv21, Crv22, Crv23:                  "				     M
*   Crv31, Crv32, Crv33:                  "				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A scalar field representing the determinant computation.M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDeterminant2, SymbSrfDeterminant3                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDeterminant3, determinant                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrvDeterminant3(const CagdCrvStruct *Crv11,
				   const CagdCrvStruct *Crv12,
				   const CagdCrvStruct *Crv13,
				   const CagdCrvStruct *Crv21,
				   const CagdCrvStruct *Crv22,
				   const CagdCrvStruct *Crv23,
				   const CagdCrvStruct *Crv31,
				   const CagdCrvStruct *Crv32,
				   const CagdCrvStruct *Crv33)
{
    CagdCrvStruct
	*Prod1 = SymbCrvDeterminant2(Crv22, Crv23, Crv32, Crv33),
        *Prod1a = SymbCrvMult(Crv11, Prod1),
	*Prod2 = SymbCrvDeterminant2(Crv21, Crv23, Crv31, Crv33),
        *Prod2a = SymbCrvMult(Crv12, Prod2),
	*Prod3 = SymbCrvDeterminant2(Crv21, Crv22, Crv31, Crv32),
        *Prod3a = SymbCrvMult(Crv13, Prod3),
	*Sub12 = SymbCrvSub(Prod1a, Prod2a),
	*Add123 = SymbCrvAdd(Sub12, Prod3a);

    CagdCrvFree(Prod1);
    CagdCrvFree(Prod1a);
    CagdCrvFree(Prod2);
    CagdCrvFree(Prod2a);
    CagdCrvFree(Prod3);
    CagdCrvFree(Prod3a);
    CagdCrvFree(Sub12);

    return Add123;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the expression of Crv11 * Crv22 - Crv12 * Crv21, which is a	     M
* determinant of a 2 by 2 matrix.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv11, Crv12, Crv21, Crv22:  The four factors of the determinant.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A scalar field representing the determinant computation.M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDeterminant3, SymbSrfDeterminant2				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDeterminant2, determinant                                         M
*****************************************************************************/
CagdCrvStruct *SymbCrvDeterminant2(const CagdCrvStruct *Crv11,
				   const CagdCrvStruct *Crv12,
				   const CagdCrvStruct *Crv21,
				   const CagdCrvStruct *Crv22)
{
    CagdCrvStruct
	*Prod1 = SymbCrvMult(Crv11, Crv22),
	*Prod2 = SymbCrvMult(Crv21, Crv12),
	*Add12 = SymbCrvSub(Prod1, Prod2);

    CagdCrvFree(Prod1);
    CagdCrvFree(Prod2);
    return Add12;
}
