/******************************************************************************
* Crv_Tans.c - computation of curve and point tangents and relations.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, January 96  				      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"
#include "bool_lib.h"
#include "iritprsr.h"
#include "allocate.h"

#define TAN_TAN_DIAGONAL_EPS 3e-3

static CagdPtStruct *UpdateCircTanPts(CagdPtStruct *Pts,
				      CagdCrvStruct *Crv1,
				      CagdCrvStruct *Crv2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the points on a C1 freeform planar Bspline curve, Crv, that a   M
* line tangent to Crv there goes through point Pt.  That is,		     M
*                                                                            M
*		(C(t) - P) || C'(t),					     V
*                                                                            M
* where || denotes a parallel constraint.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      To compute its tangent lines through Pt.                      M
*   Pt:	       Point of origin, all tangents to Crv goes through.            M
*   Tolerance: Accuracy of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter location on Crv with tangent lines M
*	       through Pt.  Parameters are save in the X coordinate.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCircTanTo2Crvs, SymbTangentToCrvAtTwoPts,	     M
* SymbCrvDiameter						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvPtTangents                                                        M
*****************************************************************************/
CagdPtStruct *SymbCrvPtTangents(const CagdCrvStruct *CCrv,
				const CagdPType Pt,
				CagdRType Tolerance)
{
    CagdPtStruct *RetList;
    CagdCrvStruct *DCrv, *Crv1, *Crv2, *Crv,
	*DCrvW, *DCrvX, *DCrvY, *DCrvZ, *CrvW, *CrvX, *CrvY, *CrvZ;
    CagdRType Trans[3];

    /* Make sure the given curve is open end conditioned curve. */
    if (CAGD_IS_BEZIER_CRV(CCrv))
	Crv = CagdCnvrtBzr2BspCrv(CCrv);
    else if (CAGD_IS_BSPLINE_CRV(CCrv) && !BspCrvHasOpenEC(CCrv))
	Crv = BspCrvOpenEnd(CCrv);
    else
        Crv = CagdCrvCopy(CCrv);

    DCrv = CagdCrvDerive(Crv);

    /* Compute 'C(t) - Pt' into 'C(t)': */
    IRIT_PT_COPY(Trans, Pt);
    IRIT_PT_SCALE(Trans, -1.0);
    CagdCrvTransform(Crv, Trans, 1.0);

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    CagdCrvFree(Crv);
    if (CrvW != NULL)
	CagdCrvFree(CrvW);
    if (CrvZ != NULL)
	CagdCrvFree(CrvZ);
    SymbCrvSplitScalar(DCrv, &DCrvW, &DCrvX, &DCrvY, &DCrvZ);
    CagdCrvFree(DCrv);
    if (DCrvW != NULL)
	CagdCrvFree(DCrvW);
    if (DCrvZ != NULL)
	CagdCrvFree(DCrvZ);

    /* Make sure "C(t) - Pt" is parallel to "C'(t)" by making sure that */
    /* "C(t) - Pt" is orthogonal to a vertical to "C'(t)":		*/
    Crv1 = SymbCrvMult(CrvX, DCrvY);
    CagdCrvFree(CrvX);
    CagdCrvFree(DCrvY);
    Crv2 = SymbCrvMult(CrvY, DCrvX);
    CagdCrvFree(CrvY);
    CagdCrvFree(DCrvX);
    Crv = SymbCrvSub(Crv1, Crv2);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

#ifdef DUMP_PT_CRV_TAN_SYMBOLIC_FUNC
    IPStderrObject(IPGenCRVObject(Crv));        /* Also a memory leak... */
#endif /* DUMP_PT_CRV_TAN_SYMBOLIC_FUNC */

    RetList = SymbCrvZeroSet(Crv, 1, Tolerance, FALSE);
    CagdCrvFree(Crv);

    return RetList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all the lines that are tangent to Crv at two locations.	     M
* Returned is a list of parameter locations' pairs where the tangent is      M
* tangent to the curve.							     M
*   The tangents are computed as two sets of contours of the solution to     M
* the two equations of:							     M
*   1.  [ C(t) - C(r) ] || C'(t)					     V
*   2.  [ C(t) - C(r) ] || C'(r)					     V
* and computing all the intersection points between these two sets of        M
* contours.								     M
*   Note that since equations 1 and 2 are symmetric, one only needs to solve M
* for once and flip the notation of r and t.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      The curve to compute all tangent lines at two locations.      M
*   FineNess:  Of numeric search for the zero set (for surface subdivision). M
*	       A positive value (10 is a good start).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   A list of parameter location on Crv with tangent lines M
*	       through Pt.  Parameters are save in the X & Y coordinate.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPtTangents, SymbCircTanTo2Crvs, SymbCrvCnvxHull, SymbCrvDiameter  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTangentToCrvAtTwoPts                                                 M
*****************************************************************************/
CagdPtStruct *SymbTangentToCrvAtTwoPts(const CagdCrvStruct *CCrv,
				       CagdRType FineNess)
{
    int OldCirc;
    CagdRType TMin, TMax;
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, 0.0 };  /* it is a scalar surface - only X. */
    CagdCrvStruct *DCrv, *Crv;
    CagdSrfStruct *Srf1, *Srf2, *Srf, *DSrf, *SrfT, *SrfR,
	*SrfW, *SrfX, *SrfY, *SrfZ, *DSrfW, *DSrfX, *DSrfY, *DSrfZ;
    IPPolygonStruct *Cntrs1, *Cntrs2, *CntrA, *CntrB;
    CagdPtStruct
	*RetList = NULL;

    /* Make sure the given curve is open end conditioned curve. */
    if (CAGD_IS_BEZIER_CRV(CCrv))
	Crv = CagdCnvrtBzr2BspCrv(CCrv);
    else if (CAGD_IS_BSPLINE_CRV(CCrv) && !BspCrvHasOpenEC(CCrv))
	Crv = BspCrvOpenEnd(CCrv);
    else
        Crv = CagdCrvCopy(CCrv);

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Make sure the domain is zero to one. */
    BspKnotAffineTrans(Crv -> KnotVector,
		       Crv -> Length + Crv -> Order,
		       -TMin,
		       1.0 / (TMax - TMin));

    DCrv = CagdCrvDerive(Crv);
    DSrf = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_U_DIR);
    SrfT = CagdPromoteCrvToSrf(Crv, CAGD_CONST_U_DIR);
    SrfR = CagdPromoteCrvToSrf(Crv, CAGD_CONST_V_DIR);
    CagdCrvFree(Crv);
    CagdCrvFree(DCrv);

    Srf1 = SymbSrfSub(SrfR, SrfT);
    CagdSrfFree(SrfR);
    CagdSrfFree(SrfT);
    SymbSrfSplitScalar(Srf1, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(Srf1);
    if (SrfW != NULL)
	CagdSrfFree(SrfW);
    if (SrfZ != NULL)
	CagdSrfFree(SrfZ);
    SymbSrfSplitScalar(DSrf, &DSrfW, &DSrfX, &DSrfY, &DSrfZ);
    CagdSrfFree(DSrf);
    if (DSrfW != NULL)
	CagdSrfFree(DSrfW);
    if (DSrfZ != NULL)
	CagdSrfFree(DSrfZ);

    /* Compute "parallel" constraint as perpendicular cross product zero. */
    Srf1 = SymbSrfMult(SrfX, DSrfY);
    CagdSrfFree(SrfX);
    CagdSrfFree(DSrfY);
    Srf2 = SymbSrfMult(SrfY, DSrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(DSrfX);
    Srf = SymbSrfSub(Srf1, Srf2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    /* Compute the zero set of the equation as contours. */
    OldCirc = IPSetPolyListCirc(TRUE);
    Cntrs1 = UserCntrSrfWithPlane(Srf, Plane, FineNess);
    IPSetPolyListCirc(OldCirc);

    /* Move the rt parametric domain from YZ to XY. */
    for (CntrA = Cntrs1; CntrA != NULL; CntrA = CntrA -> Pnext) {
	IPVertexStruct *V;

	for (V = CntrA -> PVertex; V != NULL; V = V -> Pnext) {
	    V -> Coord[0] = V -> Coord[1];
	    V -> Coord[1] = V -> Coord[2];
	    V -> Coord[2] = 0.0;
	}
    }

#ifdef DUMP_2TAN_SRF
    IPStderrObject(IPGenSRFObject(Srf));             /* Also a memory leak! */
    IPStderrObject(IPGenPOLYObject(Cntrs1));
#endif /* DUMP_2TAN_SRF */
    CagdSrfFree(Srf);

    /* Create the flipped r <-> t copy of the contours. */
    Cntrs2 = IPCopyPolygonList(Cntrs1);
    for (CntrB = Cntrs2; CntrB != NULL; CntrB = CntrB -> Pnext) {
	IPVertexStruct *V;

	for (V = CntrB -> PVertex; V != NULL; V = V -> Pnext)
	    IRIT_SWAP(CagdRType, V -> Coord[0], V -> Coord[1]);
    }

#ifdef DUMP_2TAN_CRV_CNTRS
    IPStderrObject(IPGenPOLYObject(Cntrs1));         /* Also a memory leak! */
    IPStderrObject(IPGenPOLYObject(Cntrs2));
#endif /* DUMP_2TAN_CRV_CNTRS */

    /* Compute all the intersection points of both sets of contours. */
    for (CntrB = Cntrs2; CntrB != NULL; CntrB = CntrB -> Pnext) {
	for (CntrA = Cntrs1; CntrA != NULL; CntrA = CntrA -> Pnext) {
	    Bool2DInterStruct *Inter,
	        *Inters = Boolean2DComputeInters(CntrA, CntrB, FALSE, FALSE);

	    for (Inter = Inters; Inter != NULL; ) {
		Bool2DInterStruct
		    *NextInter = Inter -> Pnext;

		/* Eliminate the intersections along the diagonal (t == r) */
		/* and due to symmetry - select only one out of each pair. */
		if (!IRIT_APX_EQ_EPS(Inter -> InterPt[0],
				Inter -> InterPt[1],
				TAN_TAN_DIAGONAL_EPS) &&
		    Inter -> InterPt[0] < Inter -> InterPt[1]) {
		    CagdPtStruct
		        *NewInterPt = CagdPtNew();

#ifdef DUMP_2TAN_CRV_INTER_PTS
		    printf("[OBJECT [COLOR 13] [width 0.04] NONE\n    [POINT %f %f 0]\n]\n",
			   Inter -> InterPt[0], Inter -> InterPt[1]);
#endif /* DUMP_2TAN_CRV_INTER_PTS */

		    NewInterPt -> Pnext = RetList;
		    RetList = NewInterPt;
		    NewInterPt -> Pt[0] =
		        TMin + Inter -> InterPt[0] * (TMax - TMin);
		    NewInterPt -> Pt[1] =
		        TMin + Inter -> InterPt[1] * (TMax - TMin);
		    NewInterPt -> Pt[2] = 0.0;
		}

		IritFree(Inter);
		Inter = NextInter;
	    }
	}
    }

    IPFreePolygonList(Cntrs1);
    IPFreePolygonList(Cntrs2);

    return RetList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the given points with are the result of CCI computation to hold   *
* the intersection locations with the parameters as "Params" attribute.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Pts:	 To update.						     *
*   Crv1, Crv2:  Two curves that participated in the CCI computation.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:   Updated list.                                          *
*****************************************************************************/
static CagdPtStruct *UpdateCircTanPts(CagdPtStruct *Pts,
				      CagdCrvStruct *Crv1,
				      CagdCrvStruct *Crv2)
{
    CagdPtStruct *Pt;

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        CagdRType *R;
        CagdPType Pt1, Pt2;

        AttrSetUVAttrib(&Pt -> Attr, "Params", Pt -> Pt[0], Pt -> Pt[1]);

	/* And evaluate the center of the circle as average of the          */
	/* intersection point on the two curves.			    */
	R = CagdCrvEval(Crv1, Pt -> Pt[0]);
        CagdCoerceToE2(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, Pt -> Pt[1]);
        CagdCoerceToE2(Pt2, &R, -1, Crv2 -> PType);

	IRIT_PT2D_BLEND(Pt -> Pt, Pt1, Pt2, 0.5);	
    }

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all circles of prescribed radius that are tangent to given two  M
* curves.								     M
*   Compute the offset of +/-R to the two curves and intersect the pairs.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to find the circle that is tangent to both. M
*   Radius:       Of the circle that is tangent to Crv1/2.		     M
*   Tol:	  Tolerance of approximation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:    List of the centers of bi-tangent circles.  Each such M
*		       point also contains a "Params" attribute with the two M
*		       parameter values of the two curves.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTangentToCrvAtTwoPts                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCircTanTo2Crvs, bi-tangent                                           M
*****************************************************************************/
CagdPtStruct *SymbCircTanTo2Crvs(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2,
				 CagdRType Radius,
				 CagdRType Tol)
{
    CagdCrvStruct
	*Crv1P = SymbCrvSubdivOffset(Crv1,  Radius, Tol, TRUE),
	*Crv1M = SymbCrvSubdivOffset(Crv1, -Radius, Tol, TRUE),
	*Crv2P = SymbCrvSubdivOffset(Crv2,  Radius, Tol, TRUE),
	*Crv2M = SymbCrvSubdivOffset(Crv2, -Radius, Tol, TRUE);
    CagdPtStruct *RetVal,
	*Pts1P2P = CagdCrvCrvInter(Crv1P, Crv2P, Tol),
	*Pts1P2M = CagdCrvCrvInter(Crv1P, Crv2M, Tol),
	*Pts1M2P = CagdCrvCrvInter(Crv1M, Crv2P, Tol),
	*Pts1M2M = CagdCrvCrvInter(Crv1M, Crv2M, Tol);

    RetVal = CagdListAppend(CagdListAppend(UpdateCircTanPts(Pts1P2P,
							    Crv1P, Crv2P),
					   UpdateCircTanPts(Pts1P2M,
							    Crv1P, Crv2M)),
			    CagdListAppend(UpdateCircTanPts(Pts1M2P,
							    Crv1M, Crv2P),
					   UpdateCircTanPts(Pts1M2M,
							    Crv1M, Crv2M)));

    CagdCrvFree(Crv1P);
    CagdCrvFree(Crv1M);
    CagdCrvFree(Crv2P);
    CagdCrvFree(Crv2M);

    return RetVal;
}
