/******************************************************************************
* MvarProj.c - Compute the (orthogonal) projection of a curve on a surface.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 2011.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

#define MVAR_ORTH_PROJ_NUMER_TOL	1e-10

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the orthogonal projection of a curve (Ct) on a surface S(u, v). M
*  That is the projection is along the normal lines of the surface.          M
*									     M
*  Computed as the univariate solution to the following two equations:	     M
*									     M
*                    dS		                 			     V
* < C(t) - S(u, v), ----- > = 0,				             V
*                    du				                             V
*									     M
*                    dS		                 			     V
* < C(t) - S(u, v), ----- > = 0.				             V
*                    dv				                             V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          The curve to project on Srf, orthogonally.		     M
*   Srf:          The surface to project Crv on.			     M
*   Tol:          Tolerance of the computation.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:    The projections in UV space of Srf.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVOrthoCrvProjOnSrf, projection                                      M
*****************************************************************************/
MvarPolyStruct *MvarMVOrthoCrvProjOnSrf(const CagdCrvStruct *Crv,
					const CagdSrfStruct *Srf,
					CagdRType Tol)
{
    CagdRType Min, Max;
    MvarMVStruct *MSrf, *MCrv, *MTmp, *DuMSrf, *DvMSrf, *MVs[2];
    MvarPolyStruct *Solution;
    CagdSrfStruct *TSrf, *NewSrf;

    /* Convert the curve and surface to trivariates T(u, v, t). */

    if (CAGD_IS_BEZIER_SRF(Srf)) { /* Ensure building Bspline constraints...*/
        Srf = NewSrf = CagdCnvrtBzr2BspSrf(Srf);
    }
    else
        NewSrf = NULL;

    MTmp = MvarSrfToMV(Srf);
    MSrf = MvarPromoteMVToMV2(MTmp, 3, 0);  
    MvarMVFree(MTmp);

    TSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    MTmp = MvarSrfToMV(TSrf);  
    CagdSrfFree(TSrf);
    DuMSrf = MvarPromoteMVToMV2(MTmp, 3, 0);  
    MvarMVFree(MTmp);

    TSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
    MTmp = MvarSrfToMV(TSrf);  
    CagdSrfFree(TSrf);
    DvMSrf = MvarPromoteMVToMV2(MTmp, 3, 0);  
    MvarMVFree(MTmp);

    /* Build the constraints. */
    if (CAGD_IS_BEZIER_CRV(Crv)) { /* Ensure building Bspline constraints...*/
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv);

	MTmp = MvarCrvToMV(TCrv);
	CagdCrvFree(TCrv);
    }
    else
        MTmp = MvarCrvToMV(Crv);
    MCrv = MvarPromoteMVToMV2(MTmp, 3, 2);
    MvarMVFree(MTmp);

    if (NewSrf != NULL)
	CagdSrfFree(NewSrf);

    /* Make domains the same. */
    MvarMVDomain(MSrf, &Min, &Max, 0);
    MTmp = MvarMVSetDomain(MCrv, Min, Max, 0, FALSE);
    MvarMVFree(MCrv);
    MCrv = MTmp;

    MvarMVDomain(MSrf, &Min, &Max, 1);
    MTmp = MvarMVSetDomain(MCrv, Min, Max, 1, FALSE);
    MvarMVFree(MCrv);
    MCrv = MTmp;

    MvarMVDomain(MCrv, &Min, &Max, 2);
    MTmp = MvarMVSetDomain(MSrf, Min, Max, 2, FALSE);
    MvarMVFree(MSrf);
    MSrf = MTmp;
    MTmp = MvarMVSetDomain(DuMSrf, Min, Max, 2, FALSE);
    MvarMVFree(DuMSrf);
    DuMSrf = MTmp;
    MTmp = MvarMVSetDomain(DvMSrf, Min, Max, 2, FALSE);
    MvarMVFree(DvMSrf);
    DvMSrf = MTmp;

    MTmp = MvarMVSub(MSrf, MCrv);
    MVs[0] = MvarMVDotProd(MTmp, DuMSrf);
    MVs[1] = MvarMVDotProd(MTmp, DvMSrf);
    MvarMVFree(MTmp);

    MvarMVFree(MCrv);
    MvarMVFree(MSrf);
    MvarMVFree(DuMSrf);
    MvarMVFree(DvMSrf);

    Solution = MvarMVUnivarInter(MVs, Tol, Tol, MVAR_ORTH_PROJ_NUMER_TOL);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    return Solution;
}
