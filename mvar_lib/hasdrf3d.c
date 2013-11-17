/******************************************************************************
* Hasdrf3d.c - computes Hausdorff distance between freeforms in 3D.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 2006.					      *
******************************************************************************/

#include <assert.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "mvar_loc.h"

#define MVAR_HDIST_SUBDIV_TOL	0.01
#define MVAR_HDIST_NUMERIC_TOL	1e-10

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Hausdorff distance between a point and a C^1 cont. surface. M
*   The surface and point are assumed to be either in R3.		     M
*   The extreme distance between a point and a surface could happen either   M
* at the corner points of surface Srf, the boundary curves of Srf, or when   M
*    <S(u, v) - P> dS'(u, v)/du = 0,					     V
*    <S(u, v) - P> dS'(u, v)/dv = 0.					     V
*                                                                            *
* PARAMETERS:                                                                M
*   P:        Point to measure its Hausdorff distance to surface Srf.	     M
*   Srf:      Srf to measure its hausdorff distance to point Pt.	     M
*   Param:    Where to return the parameter value with the maximal distance. M
*   MinDist:  TRUE for minimal distance, FALSE for maximal.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistSrfPoint                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistPointSrfC1                                                     M
*****************************************************************************/
CagdRType MvarHFDistPointSrfC1(const CagdPType P,
			       const CagdSrfStruct *Srf,
			       MvarHFDistParamStruct *Param,
			       CagdBType MinDist)
{
    CagdRType *R;
    CagdPType Pt;

    R = MvarDistSrfPoint(Srf, P, MinDist, 
			 MVAR_HDIST_SUBDIV_TOL,
			 MVAR_HDIST_NUMERIC_TOL);
    Param -> UV[0][0] = R[0];
    Param -> UV[0][1] = R[1];

    Param -> NumOfParams = 1;
    Param -> ManifoldDim = 2;

    R = CagdSrfEval(Srf, R[0], R[1]);
    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

    return IRIT_PT_PT_DIST(P, Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the local extreme distance between a point P of Srf1 and Srf2.  M
* At the local extreme distance location on Srf2, denoted Q, verify that Q   M
* is closest to P than to any other location on Srf1. 			     M
*   Returns maximal local extreme distance found, 0.0 if none.   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   P:        Point on Srf1 to measure its extreme distance to curve Srf2.   M
*   Srf1:     First curve that contains P.				     M
*   Srf2:     Second curve to measure extreme distance to from P.	     M
*   Param2:   Where to return the parameter value of Srf2 is returned.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The local extreme distance found, 0.0 if none.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbLclDistSrfPoint                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFExtremeLclDistPointSrfC1                                           M
*****************************************************************************/
CagdRType MvarHFExtremeLclDistPointSrfC1(const CagdPType P,
					 const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 MvarHFDistParamStruct *Param2)
{
    CagdRType *R, Dist, MinDist,
	MaxDist = 0.0;
    CagdPType Q;
    MvarPtStruct *Pt,
	*Pts = MvarLclDistSrfPoint(Srf2, P, 
				   MVAR_HDIST_SUBDIV_TOL,
				   MVAR_HDIST_NUMERIC_TOL);
    MvarHFDistParamStruct Param1;

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        R = CagdSrfEval(Srf2, Pt -> Pt[0], Pt -> Pt[1]);
	CagdCoerceToE3(Q, &R, - 1, Srf2 -> PType);
	Dist = IRIT_PT_PT_DIST(P, Q);

	if (Dist > MaxDist) {
	    MinDist = MvarHFDistPointSrfC1(Q, Srf1, &Param1, TRUE);

	    if (IRIT_APX_EQ_EPS(MinDist, Dist, MVAR_HDIST_NUMERIC_TOL * 10)) {
	        /* Consider this solution which is a valid maximum! */
	        MaxDist = IRIT_MAX(MaxDist, Dist);
		Param2 -> UV[0][0] = Pt -> Pt[0];
		Param2 -> UV[0][1] = Pt -> Pt[1];
		Param2 -> NumOfParams = 1;
		Param2 -> ManifoldDim = 2;
	    }
	}
    }

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the antipodal points of the given surface and curve by solving: M
*     < C2(t) - S1(u, v), dS1(u, v) / du > = 0,			             V
*     < C2(t) - S1(u, v), dS1(u, v) / dv > = 0,			             V
*     < C2(t) - S1(u, v), dC2(t) / dt > = 0.			             V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Crv2:  A surface and a curve to solve for their antipodal	     M
*		 locations.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *: List of pairs of parameters in the uv & t coefficients.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfAntipodalPoints                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistAntipodalCrvSrfC1                                              M
*****************************************************************************/
MvarPtStruct *MvarHFDistAntipodalCrvSrfC1(const CagdSrfStruct *Srf1,
					  const CagdCrvStruct *Crv2)
{
    CagdBType
	NewSrf1 = FALSE,
	NewCrv2 = FALSE;
    CagdRType UMin1, UMax1, VMin1, VMax1, TMin2, TMax2;
    CagdCrvStruct *TCrv,
	*TCrv2 = NULL;
    CagdSrfStruct *TSrf,
	*TSrf1 = NULL;
    MvarPtStruct *AntiPodalPoints, *MVPt;
    MvarConstraintType Constrs[3];
    MvarMVStruct *MVSrf1, *MVCrv2, *MVDiff, *MVTemp1, *MVTemp2, *MVs[3];

    if (CAGD_IS_RATIONAL_SRF(Srf1) || CAGD_IS_RATIONAL_CRV(Crv2))
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);

    if (CAGD_IS_BSPLINE_SRF(Srf1) && !BspSrfHasOpenEC(Srf1)) {
        Srf1 = TSrf1 = BspSrfOpenEnd(Srf1);
	NewSrf1 = TRUE;
    }
    if (CAGD_IS_BSPLINE_CRV(Crv2) && !BspCrvHasOpenEC(Crv2)) {
        Crv2 = TCrv2 = BspCrvOpenEnd(Crv2);
	NewCrv2 = TRUE;
    }

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    if (UMin1 != 0.0 || UMax1 != 1.0 || VMin1 != 0.0 || VMax1 != 1.0) {
	if (!NewSrf1)
	    Srf1 = TSrf1 = CagdSrfCopy(Srf1);
	NewSrf1 = TRUE;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Srf1 -> UKnotVector, Srf1 -> UOrder,
				 Srf1 -> ULength + Srf1 -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(Srf1 -> VKnotVector, Srf1 -> VOrder,
				 Srf1 -> VLength + Srf1 -> VOrder,
				 0.0, 1.0);
    }
    if (TMin2 != 0.0 || TMax2 != 1.0) {
	if (!NewCrv2)
	    Crv2 = TCrv2 = CagdCrvCopy(Crv2);
	NewCrv2 = TRUE;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order,
				 0.0, 1.0);
   }

    /* Srf(u, v) - Srf(s, t). */
    MVTemp1 = MvarSrfToMV(Srf1);
    MVTemp2 = MvarCrvToMV(Crv2);
    MVSrf1 = MvarPromoteMVToMV2(MVTemp1, 3, 0);
    MVCrv2 = MvarPromoteMVToMV2(MVTemp2, 3, 2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    MVDiff = MvarMVSub(MVSrf1, MVCrv2);
    MvarMVFree(MVSrf1);
    MvarMVFree(MVCrv2);

    /* < Srf(u,v) - Srf(s,t), DuSrf >. */
    TSrf = CagdSrfDerive(Srf1, CAGD_CONST_U_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 3, 0);
    MVs[0] = MvarMVDotProd(MVDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DvSrf >. */
    TSrf = CagdSrfDerive(Srf1, CAGD_CONST_V_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 3, 0);
    MVs[1] = MvarMVDotProd(MVDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DtSrf >. */
    TCrv = CagdCrvDerive(Crv2);
    MVTemp1 = MvarCrvToMV(TCrv);
    CagdCrvFree(TCrv);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 3, 2);
    MVs[2] = MvarMVDotProd(MVDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    MvarMVFree(MVDiff);

    Constrs[0] = Constrs[1] = Constrs[2] = MVAR_CNSTRNT_ZERO;

    AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 3, 
				   MVAR_HDIST_SUBDIV_TOL,
				   MVAR_HDIST_NUMERIC_TOL);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);

    /* Map coordinates back. */
    if (UMin1 != 0.0 || UMax1 != 1.0 || VMin1 != 0.0 || VMax1 != 1.0) {
        for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
	    MVPt -> Pt[0] = MVPt -> Pt[0] * (UMax1 - UMin1) + UMin1;
	    MVPt -> Pt[1] = MVPt -> Pt[1] * (VMax1 - VMin1) + VMin1;
	}
    }
    if (TMin2 != 0.0 || TMax2 != 1.0) {
        for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
	    MVPt -> Pt[2] = MVPt -> Pt[2] * (TMax2 - TMin2) + TMin2;
	}        
    }

    if (NewSrf1)
        CagdSrfFree(TSrf1);
    if (NewCrv2)
        CagdCrvFree(TCrv2);

    return AntiPodalPoints;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the one sided Hausdorff distance between a C^1 cont. curve      M
* and a C^1 cont. surface, from C1 to S2.				     M
*   The shapes are assumed to be in R3 and non intersecting.		     M
*   The one sided extreme distance between the two shapes could happen at:   M
* + The corner/end points of curve C1 or surface S2.			     M
* + Antipodal locations between the two shapes.				     M
* + Locations where C1 crosses the self bisector of S2 that are also local   M
*   distance minima from C1 to any point on S2.				     M
*									     M
* PARAMETERS:                                                                M
*   Crv1:     First crv to measure its hausdorff distance to Srf2.	     M
*   Srf2:     Second srf to measure its hausdorff distance from Crv1.	     M
*   Param1:   Where to return the parameter value(s) of Crv1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Srf2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarHFDistFromSrfToSrfC1, MvarHFDistFromSrfToCrvC1                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistFromCrvToSrfC1                                                 M
*****************************************************************************/
CagdRType MvarHFDistFromCrvToSrfC1(const CagdCrvStruct *Crv1,
				   const CagdSrfStruct *Srf2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2)
{
    fprintf(stderr, IRIT_EXP_STR("MvarHFDistFromCrvToSrfC1 Not implemented yet.\n"));

    return -1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the one sided Hausdorff distance between a C^1 cont. surface    M
* and a C^1 cont. curve, from S1 to C2					     M
*   The shapes are assumed to be in R3 and non intersecting.		     M
*   The one sided extreme distance between the two shapes could happen at:   M
* + The corner/end points of surface S1 or curve C2.			     M
* + Antipodal locations between the two shapes.				     M
* + Locations where S1 crosses the self bisector of C2 that are also local   M
*   distance minima from S1 to any point on C2.				     M
*									     M
* PARAMETERS:                                                                M
*   Srf1:     First srf to measure its hausdorff distance to Crv2.	     M
*   Crv2:      Second crv to measure its hausdorff distance from Srf1.	     M
*   Param1:   Where to return the parameter value(s) of Srf1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Crv2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarHFDistFromSrfToSrfC1                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistFromSrfToCrvC1                                                 M
*****************************************************************************/
CagdRType MvarHFDistFromSrfToCrvC1(const CagdSrfStruct *Srf1,
				   const CagdCrvStruct *Crv2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2)
{
    fprintf(stderr, IRIT_EXP_STR("MvarHFDistFromSrfToCrvC1 Not implemented yet.\n"));

    return -1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes Hausdorff distance between a C^1 cont. surface and a C^1 cont.  M
* curve, S1 and C2.							     M
*   The shapes are assumed to be in R3 and non intersecting.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:     First srf to measure its hausdorff distance to Crv2.	     M
*   Crv2:     Second crv to measure its hausdorff distance to Srf1.	     M
*   Param1:   Where to return the parameter value(s) of Srf1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Crv2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarHFDistSrfSrfC1                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistSrfCrvC1                                                       M
*****************************************************************************/
CagdRType MvarHFDistSrfCrvC1(const CagdSrfStruct *Srf1,
			     const CagdCrvStruct *Crv2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2)
{
    MvarHFDistParamStruct ParamA1, ParamA2, ParamB1, ParamB2;
    CagdRType
	Dist1 = MvarHFDistFromSrfToCrvC1(Srf1, Crv2, &ParamA1, &ParamA2),
        Dist2 = MvarHFDistFromCrvToSrfC1(Crv2, Srf1, &ParamB2, &ParamB1);

    if (Dist1 > Dist2) {
	*Param1 = ParamA1;
	*Param2 = ParamA2;
	return Dist1;
    }
    else {
	*Param1 = ParamB1;
	*Param2 = ParamB2;
	return Dist2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the antipodal points of the given two surfaces by solving:      M
*     < S2(r, t) - S1(u, v), dS1(u, v) / du > = 0,		             V
*     < S2(r, t) - S1(u, v), dS1(u, v) / dv > = 0,		             V
*     < S2(r, t) - S1(u, v), dS2(s, t) / ds > = 0,		             V
*     < S2(r, t) - S1(u, v), dS2(s, t) / ds > = 0.		             V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  The two surfaces to solve for their antipodal locations.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *: List of pairs of parameters in the uv & st coefficients. M
*                                                                            *
* SEE ALSO:                                                                  M
*    MvarSrfAntipodalPoints                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistAntipodalSrfSrfC1                                              M
*****************************************************************************/
MvarPtStruct *MvarHFDistAntipodalSrfSrfC1(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2)
{
    CagdBType
	NewSrf1 = FALSE,
	NewSrf2 = FALSE;
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdSrfStruct *TSrf,
	*TSrf1 = NULL,
	*TSrf2 = NULL;
    MvarPtStruct *AntiPodalPoints, *MVPt;
    MvarConstraintType Constrs[4];
    MvarMVStruct *MVSrf1, *MVSrf2, *MVSrfDiff, *MVTemp1, *MVTemp2, *MVs[4];

    if (CAGD_IS_RATIONAL_SRF(Srf1) || CAGD_IS_RATIONAL_SRF(Srf2))
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);

    if (CAGD_IS_BSPLINE_SRF(Srf1) && !BspSrfHasOpenEC(Srf1)) {
        Srf1 = TSrf1 = BspSrfOpenEnd(Srf1);
	NewSrf1 = TRUE;
    }
    if (CAGD_IS_BSPLINE_SRF(Srf2) && !BspSrfHasOpenEC(Srf2)) {
        Srf2 = TSrf2 = BspSrfOpenEnd(Srf2);
	NewSrf2 = TRUE;
    }

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);

    if (UMin1 != 0.0 || UMax1 != 1.0 || VMin1 != 0.0 || VMax1 != 1.0) {
	if (!NewSrf1)
	    Srf1 = TSrf1 = CagdSrfCopy(Srf1);
	NewSrf1 = TRUE;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Srf1 -> UKnotVector, Srf1 -> UOrder,
				 Srf1 -> ULength + Srf1 -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(Srf1 -> VKnotVector, Srf1 -> VOrder,
				 Srf1 -> VLength + Srf1 -> VOrder,
				 0.0, 1.0);
    }
    if (UMin2 != 0.0 || UMax2 != 1.0 || VMin2 != 0.0 || VMax2 != 1.0) {
	if (!NewSrf2)
	    Srf2 = TSrf2 = CagdSrfCopy(Srf2);
	NewSrf2 = TRUE;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Srf2 -> UKnotVector, Srf2 -> UOrder,
				 Srf2 -> ULength + Srf2 -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(Srf2 -> VKnotVector, Srf2 -> VOrder,
				 Srf2 -> VLength + Srf2 -> VOrder,
				 0.0, 1.0);
    }

    /* Srf(u, v) - Srf(s, t). */
    MVTemp1 = MvarSrfToMV(Srf1);
    MVTemp2 = MvarSrfToMV(Srf2);
    MVSrf1 = MvarPromoteMVToMV2(MVTemp1, 4, 0);
    MVSrf2 = MvarPromoteMVToMV2(MVTemp2, 4, 2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    MVSrfDiff = MvarMVSub(MVSrf1, MVSrf2);
    MvarMVFree(MVSrf1);
    MvarMVFree(MVSrf2);

    /* < Srf(u,v) - Srf(s,t), DuSrf >. */
    TSrf = CagdSrfDerive(Srf1, CAGD_CONST_U_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 4, 0);
    MVs[0] = MvarMVDotProd(MVSrfDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DvSrf >. */
    TSrf = CagdSrfDerive(Srf1, CAGD_CONST_V_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 4, 0);
    MVs[1] = MvarMVDotProd(MVSrfDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DsSrf >. */
    TSrf = CagdSrfDerive(Srf2, CAGD_CONST_U_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 4, 2);
    MVs[2] = MvarMVDotProd(MVSrfDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DtSrf >. */
    TSrf = CagdSrfDerive(Srf2, CAGD_CONST_V_DIR);
    MVTemp1 = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 4, 2);
    MVs[3] = MvarMVDotProd(MVSrfDiff, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    MvarMVFree(MVSrfDiff);

    Constrs[0] = Constrs[1] = Constrs[2] = Constrs[3] = MVAR_CNSTRNT_ZERO;

    AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 4, 
				   MVAR_HDIST_SUBDIV_TOL,
				   MVAR_HDIST_NUMERIC_TOL);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);
    MvarMVFree(MVs[3]);

    /* Map coordinates back. */
    if (UMin1 != 0.0 || UMax1 != 1.0 || VMin1 != 0.0 || VMax1 != 1.0) {
        for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
	    MVPt -> Pt[0] = MVPt -> Pt[0] * (UMax1 - UMin1) + UMin1;
	    MVPt -> Pt[1] = MVPt -> Pt[1] * (VMax1 - VMin1) + VMin1;
	}
    }
    if (UMin2 != 0.0 || UMax2 != 1.0 || VMin2 != 0.0 || VMax2 != 1.0) {
        for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
	    MVPt -> Pt[2] = MVPt -> Pt[2] * (UMax2 - UMin2) + UMin2;
	    MVPt -> Pt[3] = MVPt -> Pt[3] * (VMax2 - VMin2) + VMin2;
	}
    }

    if (NewSrf1)
        CagdSrfFree(TSrf1);
    if (NewSrf2)
        CagdSrfFree(TSrf2);

    return AntiPodalPoints;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the intersection locations of (C^1 cont) Srf2 with the self	     M
* bisectors of (C^1 cont.) Srf1, if any.  The solution is computed by        M
* solving the following set of contraints:				     M
*									     M
*     || S2(s, t) - S1(a, b) ||^2 == || S2(s, t) - S1(c, d) ||^2,            V
*     < S2((s, t) - S1(a, b), dS1(a, b)/da > = 0,		             V
*     < S2((s, t) - S1(a, b), dS1(a, b)/db > = 0,		             V
*     < S2((s, t) - S1(c, d), dS1(c, d)/dc > = 0.		             V
*     < S2((s, t) - S1(c, d), dS1(c, d)/dd > = 0.		             V
*									     M
* augumented with							     M
*									     M
*     < (S1(a, b) - S(c, d)) x (S1(a, b) + S1(c, d) - 2S2(s, t)),	     V
*       NS2(s, t) > = 0,						     V
*									     M
* where NS2 is a (non normalized) normnal field of S2.			     M
*									     M
*    The first equation above (equal distance to two different locations in  M
* C1) could be rewritten as:						     M
*     < S1(a, b) + S1(c, d) - 2S2(s, t), S1(a, b) - S1(c, d) > = 0,          V
* which hints to the fact that this equation vanish for ((a, b) == (c, d)).  M
* Hence, in the solution process, we eliminate the ((a, b) == (c, d))        M
* factors from it.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:     First surface to intersect its self-bisector with Srf2.	     M
*   Srf2:     Second surface to intersect against the self bisector of Srf1. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarHFDistPairParamStruct *:  Linked list of all detected intersections. M
*	     Note each detected intersection holds two parameter locations   M
*	     of Srf1.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistBisectSrfSrfC1	                                             M
*****************************************************************************/
MvarHFDistPairParamStruct *MvarHFDistBisectSrfSrfC1(const CagdSrfStruct *Srf1,
						    const CagdSrfStruct *Srf2)
{
    fprintf(stderr, IRIT_EXP_STR("MvarHFDistBisectSrfSrfC1 Not implemented yet.\n"));

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the intersection locations of (C^1 cont) Srf2 with the self	     M
* bisectors of (C^1 cont.) Srf1, if any.  The solution is computed by        M
* solving the following set of contraints:				     M
* 1. The surface-surface self bisector of Srf1, intersected with Srf2	     M
* 2. boundary-curve self bisectors of Srf1, intersected with Srf2            M
* 3. corner-point self bisectors of Srf1, intersected with Srf2	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:    First curve to intersect its self-bisector with Srf2.	     M
*   Srf2:    Second curve to intersect against the self bisector of Srf1.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarHFDistPairParamStruct *:  Linked list of all detected intersections. M
*	     Note each detected intersection holds two parameters locations  M
*            of Srf1.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistInterBisectSrfSrfC1                                            M
*****************************************************************************/
MvarHFDistPairParamStruct *MvarHFDistInterBisectSrfSrfC1(
						    const CagdSrfStruct *Srf1,
						    const CagdSrfStruct *Srf2)
{
    fprintf(stderr, IRIT_EXP_STR("MvarHFDistInterBisectSrfSrfC1 Not implemented yet.\n"));

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the one sided Hausdorff distance between two C^1 cont. surfaces M
* from S1 to S2.							     M
*   The surfaces are assumed to be in R3 and non intersecting.		     M
*   The one sided extreme distance between two surfaces could happen at:     M
* + The corner points of surface S1 or surface S2.			     M
* + Antipodal locations between the two surfaces.
* + Locations where S1 crosses the self bisector of S2 that are also local   M
*   distance minima from S1 to any point on S2.				     M
*									     M
* PARAMETERS:                                                                M
*   CSrf1:    First Srf to measure its hausdorff distance to Srf2.	     M
*   CSrf2:    Second Srf to measure its hausdorff distance from Srf1.	     M
*   Param1:   Where to return the parameter value(s) of Srf1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Srf2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistFromSrfToSrfC1                                                 M
*****************************************************************************/
CagdRType MvarHFDistFromSrfToSrfC1(const CagdSrfStruct *CSrf1,
				   const CagdSrfStruct *CSrf2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2)
{
    int i, j;
    CagdBType BspGeom;
    CagdRType *R, UDmn[2], VDmn[2], Dist,
	MaxDist = 0.0;
    CagdPType P, Pt1, Pt2;
    MvarPtStruct *AntiPodalPoints, *MVPt;
    MvarHFDistPairParamStruct *PPList, *PP;
    MvarHFDistParamStruct Param;
    CagdSrfStruct *Srf1, *Srf2;

    Param1 -> NumOfParams = 1;
    Param2 -> NumOfParams = 1;

    /* Make sure surfaces have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_SRF(CSrf1)) {
        if (CAGD_IS_BSPLINE_SRF(CSrf2))
	    Srf1 = CagdCnvrtBzr2BspSrf(CSrf1);
	else
	    Srf1 = CagdSrfCopy(CSrf1);
    }
    else
        Srf1 = CagdCnvrtBsp2OpenSrf(CSrf1);

    if (CAGD_IS_BEZIER_SRF(CSrf2)) {
        if (CAGD_IS_BSPLINE_SRF(CSrf1))
	    Srf2 = CagdCnvrtBzr2BspSrf(CSrf2);
	else
	    Srf2 = CagdSrfCopy(CSrf2);
    }
    else
	Srf2 = CagdCnvrtBsp2OpenSrf(CSrf2);

    BspGeom = CAGD_IS_BSPLINE_SRF(Srf1);

    /* Stage 1. Examine corner points of Srf1 vs. surface Srf2. */
    CagdSrfDomain(Srf1, &UDmn[0], &UDmn[1], &VDmn[0], &VDmn[1]);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    R = CagdSrfEval(Srf1, UDmn[i], VDmn[j]);
	    CagdCoerceToE3(P, &R, -1, Srf1 -> PType);
	    if (MaxDist < (Dist = MvarHFDistPointSrfC1(P, Srf2,
						       &Param, TRUE))) {
	        MaxDist = Dist;
		*Param2 = Param;
		Param1 -> UV[0][0] = UDmn[i];
		Param1 -> UV[0][1] = VDmn[j];
		Param1 -> NumOfParams = 1;
		Param1 -> ManifoldDim = 2;
	    }
	}
    }

    /* Stage 1. Examine end points of Srf2 vs. surface Srf1. */
    CagdSrfDomain(Srf2, &UDmn[0], &UDmn[1], &VDmn[0], &VDmn[1]);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    R = CagdSrfEval(Srf2, UDmn[i], VDmn[j]);
	    CagdCoerceToE3(P, &R, -1, Srf2 -> PType);
	    if (MaxDist < (Dist = MvarHFExtremeLclDistPointSrfC1(P, Srf2, Srf1,
								 &Param))) {
	        MaxDist = Dist;
		*Param1 = Param;
		Param1 -> UV[0][0] = UDmn[i];
		Param1 -> UV[0][1] = VDmn[j];
		Param2 -> NumOfParams = 1;
		Param2 -> ManifoldDim = 2;
	    }
	}
    }

    /* Stage 2. Solve for the antipodal locations. */
    AntiPodalPoints = MvarHFDistAntipodalSrfSrfC1(Srf1, Srf2);

    /* Examine all antipodal points we got. */
    for (MVPt = AntiPodalPoints; MVPt != NULL; MVPt = MVPt -> Pnext) {
        R = CagdSrfEval(Srf1, MVPt -> Pt[0], MVPt -> Pt[1]);
	CagdCoerceToE3(Pt1, &R, -1, Srf1 -> PType);
        R = CagdSrfEval(Srf2, MVPt -> Pt[2], MVPt -> Pt[3]);
	CagdCoerceToE3(Pt2, &R, -1, Srf1 -> PType);

	if (MaxDist < (Dist = IRIT_PT_PT_DIST(Pt1, Pt2))) {
	    MvarHFDistParamStruct TParam;
	    CagdRType
	        MinDist = MvarHFDistPointSrfC1(Pt1, Srf2, &TParam, TRUE);

	    /* Accept this solution only if a local minima. */
	    if (IRIT_APX_EQ_EPS(MinDist, Dist, MVAR_HDIST_NUMERIC_TOL * 10)) {
	        MaxDist = Dist;
		Param1 -> UV[0][0] = MVPt -> Pt[0];
		Param1 -> UV[0][1] = MVPt -> Pt[1];
		Param2 -> UV[0][0] = MVPt -> Pt[2];
		Param2 -> UV[0][1] = MVPt -> Pt[3];
		Param1 -> ManifoldDim = Param2 -> ManifoldDim = 2;
		Param1 -> NumOfParams =	Param2 -> NumOfParams = 1;
	    }
	}
    }
    MvarPtFreeList(AntiPodalPoints);

    /* Stage 3. Compute the intersections of S1 with self bisector of S2. */
    PPList = MvarHFDistInterBisectSrfSrfC1(Srf2, Srf1);

    /* Examine all S1 -- self-bisectors-of-S2 intersection locations.     */
    /* Note Srf1 and Srf2 are reversed here for proper function call.     */
    while (PPList) {
        IRIT_LIST_POP(PP, PPList);

        R = CagdSrfEval(Srf1, PP -> Param2.UV[0][0], PP -> Param2.UV[0][1]);
	CagdCoerceToE3(Pt1, &R, -1, Srf1 -> PType);
        R = CagdSrfEval(Srf2, PP -> Param1.UV[0][0], PP -> Param1.UV[0][1]);
	CagdCoerceToE3(Pt2, &R, -1, Srf1 -> PType);
        if (MaxDist < (Dist = IRIT_PT_PT_DIST(Pt1, Pt2))) {
	    MvarHFDistParamStruct TParam;
	    CagdRType
	        MinDist = MvarHFDistPointSrfC1(Pt1, Srf2, &TParam, TRUE);

	    /* Accept this solution only if a local minima. */
	    if (IRIT_APX_EQ_EPS(MinDist, Dist, MVAR_HDIST_NUMERIC_TOL * 10)) {
	        MaxDist = Dist;
		IRIT_GEN_COPY(Param1 -> UV[0], PP -> Param2.UV[0],
			 sizeof(CagdRType) * 2);
		IRIT_GEN_COPY(Param2 -> UV[0], PP -> Param1.UV[0],
			 sizeof(CagdRType) * 2);
		IRIT_GEN_COPY(Param2 -> UV[1], PP -> Param1.UV[1],
			 sizeof(CagdRType) * 2);
		Param1 -> NumOfParams = 1;
		Param2 -> NumOfParams = 2;
		Param1 -> ManifoldDim = Param2 -> ManifoldDim = 2;
	    }
	}

	IritFree(PP);
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes Hausdorff distance between two C^1 cont. surfaces, S1 and S2.   M
*   The surfaces are assumed to be in R3 and non intersecting.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:     First Srf to measure its hausdorff distance to Srf2.	     M
*   Srf2:     Second Srf to measure its hausdorff distance to Srf1.	     M
*   Param1:   Where to return the parameter value(s) of Srf1 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*   Param2:   Where to return the parameter value(s) of Srf2 with the	     M
*	      maximal Hausdorff distance.  Can be more than one location!    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    The Hausdorff distance.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarHFDistSrfSrfC1                                                       M
*****************************************************************************/
CagdRType MvarHFDistSrfSrfC1(const CagdSrfStruct *Srf1,
			     const CagdSrfStruct *Srf2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2)
{
    MvarHFDistParamStruct ParamA1, ParamA2, ParamB1, ParamB2;
    CagdRType
	Dist1 = MvarHFDistFromSrfToSrfC1(Srf1, Srf2, &ParamA1, &ParamA2),
	Dist2 = MvarHFDistFromSrfToSrfC1(Srf2, Srf1, &ParamB2, &ParamB1);

    if (Dist1 > Dist2) {
	*Param1 = ParamA1;
	*Param2 = ParamA2;
	return Dist1;
    }
    else {
	*Param1 = ParamB1;
	*Param2 = ParamB2;
	return Dist2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal distance between a given C1 curve and a C1 surface. M
*   Shapes are assumed to not intersect.				     M
*   This minimal distance can occur:					     M
*	1. At end points vs. end points.				     M
*	2. At the end points vs interior locations.			     M
*       3. At a boundary curve of the surface vs the other curves.	     M
*	4. At antipodal interior locations.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:         To detect its minimal distance to Crv2.                    M
*   Crv2:         To detect its minimal distance to Srf1.                    M
*   MinDist:	  Upon return, is set to the minimal distance detected.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Pairs of parameters at the minimal distance.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfSrfAntipodalPoints, SymbDistSrfPoint, MvarSrfSrfMinimalDist       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvSrfMinimalDist                                                    M
*****************************************************************************/
MvarPtStruct *MvarCrvSrfMinimalDist(const CagdSrfStruct *Srf1,
				    const CagdCrvStruct *Crv2,
				    CagdRType *MinDist)
{
    int i, j;
    CagdRType *R, UDmn1[2], VDmn1[2], TDmn2[2], UMin, UMax, VMin, VMax;
    CagdPType Pt;
    CagdCrvStruct **Bndry;
    MvarPtStruct *MPts, *MPt, *RetVal;

    MPts = NULL;
    CagdSrfDomain(Srf1, &UDmn1[0], &UDmn1[1], &VDmn1[0], &VDmn1[1]);
    CagdCrvDomain(Crv2, &TDmn2[0], &TDmn2[1]);

    /* 1. Set to check End points vs. End Points. */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    int k;

	    for (k = 0; k < 2; k++) {
	        MPt = MvarPtNew(3);
		MPt -> Pt[0] = UDmn1[i];
		MPt -> Pt[1] = UDmn1[j];
		MPt -> Pt[2] = TDmn2[k];
		IRIT_LIST_PUSH(MPt, MPts);
	    }
	}
    }

    /* 2. Set to check End points vs. Interior points. */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    R = CagdSrfEval(Srf1, UDmn1[i], VDmn1[j]);
	    CagdCoerceToE3(Pt, &R, -1, Srf1 -> PType);
	    MPt = MvarPtNew(3);
	    MPt -> Pt[0] = UDmn1[i];
	    MPt -> Pt[1] = VDmn1[j];
	    MPt -> Pt[2] = SymbDistCrvPoint(Crv2, Pt, TRUE,
					    MVAR_HDIST_NUMERIC_TOL);
	    IRIT_LIST_PUSH(MPt, MPts);
	}
    }

    for (i = 0; i < 2; i++) {
        R = CagdCrvEval(Crv2, TDmn2[i]);
	CagdCoerceToE3(Pt, &R, -1, Crv2 -> PType);
	R = MvarDistSrfPoint(Srf1, Pt, TRUE, 
			     MVAR_HDIST_SUBDIV_TOL,
			     MVAR_HDIST_NUMERIC_TOL);
	MPt = MvarPtNew(3);
	MPt -> Pt[0] = R[0];
	MPt -> Pt[1] = R[1];
	MPt -> Pt[2] = TDmn2[i];
	IRIT_LIST_PUSH(MPt, MPts);
    }

    /* 3. Examine the boundary curves of Srf1 vs. Crv2. */
    Bndry = CagdBndryCrvsFromSrf(Srf1);
    CagdSrfDomain(Srf1, &UMin, &UMax, &VMin, &VMax);
    for (i = 0; i < 4; i++) {
        MvarPtStruct *MBndryPts, *MBndryPt;

        MBndryPts = MvarCrvCrvMinimalDist(Bndry[i], Crv2, MinDist, TRUE,
					  MVAR_HDIST_NUMERIC_TOL);

	for (MBndryPt = MBndryPts;
	     MBndryPt != NULL;
	     MBndryPt = MBndryPt -> Pnext) {
	    /* Convert to surface parameter values. */
	    MPt = MvarPtNew(3);
	    switch (i) {
	        case 0:  /* UMin. */
		    MPt -> Pt[0] = UMin;
		    MPt -> Pt[1] = MBndryPt -> Pt[0];
		    MPt -> Pt[2] = MBndryPt -> Pt[1];
		    break;
	        case 1:  /* UMax. */
		    MPt -> Pt[0] = UMax;
		    MPt -> Pt[1] = MBndryPt -> Pt[0];
		    MPt -> Pt[2] = MBndryPt -> Pt[1];
	            break;
	        case 2:  /* VMin. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = VMin;
		    MPt -> Pt[2] = MBndryPt -> Pt[1];
		    break;
	        case 3:  /* VMax. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = VMax;
		    MPt -> Pt[2] = MBndryPt -> Pt[1];
		    break;
	        default:
		    assert(0);
		    break;
	    }
	    IRIT_LIST_PUSH(MPt, MPts);
	}

	MvarPtFreeList(MBndryPts);
	CagdCrvFree(Bndry[i]);
    }

    /* 4. Add the antipodal points. */
    MPt = MvarHFDistAntipodalCrvSrfC1(Srf1, Crv2);
    MPts = CagdListAppend(MPt, MPts);

    /* Time to examine all the events we collected.  Find the minimum       */
    /* distance and return it as the minimal distance between this pair.    */
    RetVal = MvarPtNew(3);
    *MinDist = IRIT_INFNTY;
    for (MPt = MPts; MPt != NULL; MPt = MPt -> Pnext) {
        CagdRType d;
        CagdPType Pt1, Pt2;

        R = CagdSrfEval(Srf1, MPt -> Pt[0], MPt -> Pt[1]);
	CagdCoerceToE3(Pt1, &R, -1, Srf1 -> PType);
        R = CagdCrvEval(Crv2, MPt -> Pt[2]);
	CagdCoerceToE3(Pt2, &R, -1, Srf1 -> PType);

	d = IRIT_PT_PT_DIST(Pt1, Pt2);

	if (*MinDist > d) {
	    *MinDist = d;
	    IRIT_GEN_COPY(RetVal -> Pt, MPt -> Pt, sizeof(CagdRType) * 3);
	}
    }
    MvarPtFreeList(MPts);

    return RetVal;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal distance between two given C1 surfaces.	     M
*   Surfaces are assumed to not intersect.				     M
*   This minimal distance can occur:					     M
*	1. At end points vs. end points.				     M
*	2. At the end points vs interior locations.			     M
*       3. At a boundary curve vs the other surface.			     M
*	4. At antipodal interior locations.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:         To detect its minimal distance to Srf2.                    M
*   Srf2:         To detect its minimal distance to Srf1.                    M
*   MinDist:	  Upon return, is set to the minimal distance detected.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Pairs of parameters at the minimal distance.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfSrfAntipodalPoints, SymbDistSrfPoint, MvarCrvSrfMinimalDist       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfMinimalDist                                                    M
*****************************************************************************/
MvarPtStruct *MvarSrfSrfMinimalDist(const CagdSrfStruct *Srf1,
				    const CagdSrfStruct *Srf2,
				    CagdRType *MinDist)
{
    int i, j;
    CagdRType *R, UDmn1[2], VDmn1[2], UDmn2[2], VDmn2[2],
	UMin, UMax, VMin, VMax;
    CagdPType Pt;
    CagdCrvStruct *Bndry[4];
    MvarPtStruct *MPts, *MPt, *RetVal;

    MPts = NULL;
    CagdSrfDomain(Srf1, &UDmn1[0], &UDmn1[1], &VDmn1[0], &VDmn1[1]);
    CagdSrfDomain(Srf2, &UDmn2[0], &UDmn2[1], &VDmn2[0], &VDmn2[1]);

    /* 1. Set to check End points vs. End Points. */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    int k, l;

	    for (k = 0; k < 2; k++) {
	        for (l = 0; l < 2; l++) {
		    MPt = MvarPtNew(4);
		    MPt -> Pt[0] = UDmn1[i];
		    MPt -> Pt[1] = VDmn1[j];
		    MPt -> Pt[2] = UDmn2[k];
		    MPt -> Pt[3] = VDmn2[l];
		    IRIT_LIST_PUSH(MPt, MPts);
		}
	    }
	}
    }

    /* 2. Set to check End points vs. Interior points. */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    R = CagdSrfEval(Srf1, UDmn1[i], VDmn1[j]);
	    CagdCoerceToE3(Pt, &R, -1, Srf1 -> PType);
	    R = MvarDistSrfPoint(Srf2, Pt, TRUE, 
				 MVAR_HDIST_SUBDIV_TOL,
				 MVAR_HDIST_NUMERIC_TOL);
	    MPt = MvarPtNew(4);
	    MPt -> Pt[0] = UDmn1[i];
	    MPt -> Pt[1] = VDmn1[j];
	    MPt -> Pt[2] = R[0];
	    MPt -> Pt[3] = R[1];
	    IRIT_LIST_PUSH(MPt, MPts);
	}
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    R = CagdSrfEval(Srf2, UDmn2[i], VDmn2[j]);
	    CagdCoerceToE3(Pt, &R, -1, Srf2 -> PType);
	    R = MvarDistSrfPoint(Srf1, Pt, TRUE, 
				 MVAR_HDIST_SUBDIV_TOL,
				 MVAR_HDIST_NUMERIC_TOL);
	    MPt = MvarPtNew(4);
	    MPt -> Pt[0] = R[0];
	    MPt -> Pt[1] = R[1];
	    MPt -> Pt[2] = UDmn2[i];
	    MPt -> Pt[3] = VDmn2[j];
	    IRIT_LIST_PUSH(MPt, MPts);
	}
    }

    /* 3. Examine the boundary curves of Srf1 vs. Srf2 and vice versa. */
    IRIT_GEN_COPY(Bndry, CagdBndryCrvsFromSrf(Srf1),
		  sizeof(CagdCrvStruct *) * 4);
    CagdSrfDomain(Srf1, &UMin, &UMax, &VMin, &VMax);
    for (i = 0; i < 4; i++) {
        MvarPtStruct *MBndryPts, *MBndryPt;

        MBndryPts = MvarCrvSrfMinimalDist(Srf2, Bndry[i], MinDist);

	for (MBndryPt = MBndryPts;
	     MBndryPt != NULL;
	     MBndryPt = MBndryPt -> Pnext) {
	    /* Convert to surface parameter values. */
	    MPt = MvarPtNew(4);
	    switch (i) {
	        case 0:  /* UMin. */
		    MPt -> Pt[0] = UMin;
		    MPt -> Pt[1] = MBndryPt -> Pt[2];
		    MPt -> Pt[2] = MBndryPt -> Pt[0];
		    MPt -> Pt[3] = MBndryPt -> Pt[1];
		    break;
	        case 1:  /* UMax. */
		    MPt -> Pt[0] = UMax;
		    MPt -> Pt[1] = MBndryPt -> Pt[2];
		    MPt -> Pt[2] = MBndryPt -> Pt[0];
		    MPt -> Pt[3] = MBndryPt -> Pt[1];
	            break;
	        case 2:  /* VMin. */
		    MPt -> Pt[0] = MBndryPt -> Pt[2];
		    MPt -> Pt[1] = VMin;
		    MPt -> Pt[2] = MBndryPt -> Pt[0];
		    MPt -> Pt[3] = MBndryPt -> Pt[1];
		    break;
	        case 3:  /* VMax. */
		    MPt -> Pt[0] = MBndryPt -> Pt[2];
		    MPt -> Pt[1] = VMax;
		    MPt -> Pt[2] = MBndryPt -> Pt[0];
		    MPt -> Pt[3] = MBndryPt -> Pt[1];
		    break;
	        default:
		    assert(0);
		    break;
	    }
	    IRIT_LIST_PUSH(MPt, MPts);
	}

	MvarPtFreeList(MBndryPts);
	CagdCrvFree(Bndry[i]);
    }

    IRIT_GEN_COPY(Bndry, CagdBndryCrvsFromSrf(Srf2),
		  sizeof(CagdCrvStruct *) * 4);
    CagdSrfDomain(Srf2, &UMin, &UMax, &VMin, &VMax);
    for (i = 0; i < 4; i++) {
        MvarPtStruct *MBndryPts, *MBndryPt;

        MBndryPts = MvarCrvSrfMinimalDist(Srf1, Bndry[i], MinDist);

	for (MBndryPt = MBndryPts;
	     MBndryPt != NULL;
	     MBndryPt = MBndryPt -> Pnext) {
	    /* Convert to surface parameter values. */
	    MPt = MvarPtNew(4);
	    switch (i) {
	        case 0:  /* UMin. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = MBndryPt -> Pt[1];
		    MPt -> Pt[2] = UMin;
		    MPt -> Pt[3] = MBndryPt -> Pt[2];
		    break;
	        case 1:  /* UMax. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = MBndryPt -> Pt[1];
		    MPt -> Pt[2] = UMax;
		    MPt -> Pt[3] = MBndryPt -> Pt[2];
	            break;
	        case 2:  /* VMin. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = MBndryPt -> Pt[1];
		    MPt -> Pt[2] = MBndryPt -> Pt[2];
		    MPt -> Pt[3] = VMin;
		    break;
	        case 3:  /* VMax. */
		    MPt -> Pt[0] = MBndryPt -> Pt[0];
		    MPt -> Pt[1] = MBndryPt -> Pt[1];
		    MPt -> Pt[2] = MBndryPt -> Pt[2];
		    MPt -> Pt[3] = VMax;
		    break;
	        default:
		    assert(0);
		    break;
	    }
	    IRIT_LIST_PUSH(MPt, MPts);
	}

	MvarPtFreeList(MBndryPts);
	CagdCrvFree(Bndry[i]);
    }


    /* 4. Add the antipodal points. */
    MPt = MvarHFDistAntipodalSrfSrfC1(Srf1, Srf2);
    MPts = CagdListAppend(MPt, MPts);

    /* Time to examine all the events we collected.  Find the minimum       */
    /* distance and return it as the minimal distance between this pair.    */
    RetVal = MvarPtNew(4);
    *MinDist = IRIT_INFNTY;
    for (MPt = MPts; MPt != NULL; MPt = MPt -> Pnext) {
        CagdRType d;
        CagdPType Pt1, Pt2;

        R = CagdSrfEval(Srf1, MPt -> Pt[0], MPt -> Pt[1]);
	CagdCoerceToE3(Pt1, &R, -1, Srf1 -> PType);
        R = CagdSrfEval(Srf2, MPt -> Pt[2], MPt -> Pt[3]);
	CagdCoerceToE3(Pt2, &R, -1, Srf1 -> PType);

	d = IRIT_PT_PT_DIST(Pt1, Pt2);

	if (*MinDist > d) {
	    *MinDist = d;
	    IRIT_GEN_COPY(RetVal -> Pt, MPt -> Pt, sizeof(CagdRType) * 4);
	}
    }
    MvarPtFreeList(MPts);

    return RetVal;
}
