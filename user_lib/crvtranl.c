/******************************************************************************
* CrvtrAnl.c - curvature analysis computation.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 06.					      *
******************************************************************************/

#include "iritprsr.h"
#include "allocate.h"
#include "user_loc.h"
#include "triv_lib.h"
#include "mrchcube.h"
#include "mvar_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a parametric curve, Crv, and a control point index CtlPtIdx,       M
* compute the curvature sign field of the curve as function of the Euclidean M
* locations of control point index CtlPtIdx. 				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute its curvature behaviour (convex vs. concave) as a  M
*	       function of the curve parameter and the Euclidean coordiate   M
*	       of the CtlPtIdx's control point.				     M
*   CtlPtIdx:  Index of control point to make a parameter for the curvature. M
*   Min, Max:  Domain each coordinate of CtlPtIdx point should vary.         M
*   SubdivTol, NumerTol:  Tolerance for the silhouette solving, if any.      M
*   Operation: 1. Returned is a multivariate of dimension "1 + Dim(Crv)",    M
*		  where Dim(Crv) is the dimension of curve (E2, E3, etc.).   M
*	       2. Extract the zero set of 1. using marching cubes.	     M
*              3. Computes the t's silhouette of the 1. by simultaneously    M
*		  solving for 1 and its derivative with respect to t.        M
*              4. Same as 3 but evaluate the result into Euclidean space.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either the multivariate (1 above) or its t's	     M
*			silhouette (2 above).				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvtrByOneCtlPt, SymbCrv2DCurvatureSign, MvarCrvMakeCtlPtParam    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvCrvtrByOneCtlPt, curvature                                        M
*****************************************************************************/
IPObjectStruct *UserCrvCrvtrByOneCtlPt(const CagdCrvStruct *Crv,
				       int CtlPtIdx,
				       CagdRType Min,
				       CagdRType Max,
				       CagdRType SubdivTol,
				       CagdRType NumerTol,
				       int Operation)
{
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    IrtPtType MCCubeDim;
    MvarConstraintType Constraints[2];
    IPObjectStruct *PObj;
    TrivTVStruct *TV;
    MvarPtStruct *MVPts;
    MvarMVStruct *DMV, *MVs[2],
        *MV = MvarCrvCrvtrByOneCtlPt(Crv, CtlPtIdx, Min, Max);

    switch (Operation) {
        default:
	case 1:
	    PObj = IPGenMULTIVARObject(MV);
	    break;
	case 2:
	    TV = MvarMVToTV(MV);
	    MCCubeDim[0] = MCCubeDim[1] = MCCubeDim[2] = 1.0 / SubdivTol;
	    PObj = MCExtractIsoSurface2(TV, 1, TRUE, MCCubeDim,
					1, 1.0 / SubdivTol, 0.0);
	    TrivTVFree(TV);
	    MvarMVFree(MV);
	    break;
	case 3:
	case 4:
	    DMV = MvarMVDerive(MV, 0);

	    /* Compute the zeros. */
	    MVs[0] = MV;
	    MVs[1] = DMV;
	    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
	    MVPts = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumerTol);
	    MvarMVFree(MV);
	    MvarMVFree(DMV);

	    /* Evaluated into Euclidean space if so desired. */
	    if (Operation == 3) {
	        MvarPtStruct *MVPt;

		for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
		    for (i = 1; i <= MaxAxis; i++) {
		        MVPt -> Pt[i] = MVPt -> Pt[i] * (Max - Min) + Min;
		    }
		}
	    }

	    /* Merge into a polyline. */
	    PObj = MvarCnvrtMVPtsToCtlPts(MVPts, SubdivTol * 10.0);
	    MvarPtFreeList(MVPts);
	    break;
    }

    return PObj;
}
