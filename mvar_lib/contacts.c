/******************************************************************************
* Contacts.c - computes tangential contacts between freeforms.	              *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 2010.					      *
******************************************************************************/

#include <assert.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "mvar_loc.h"

#define MVAR_CRV_CONTACT_SUBDIV_TOL              1e-3
#define MVAR_CRV_CONTACT_OFFSET_TOL              1e-4

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tangential contact points of the given two curves by solving    M
* for Ci(t) = (xi(t), yi(t)), i = 1,2:					     M
*     x1(t) - x2(r) = 0,                                                     V
*     y1(t) - y2(r) = 0,                                        	     V
*     < C1(t) - C2O(r), C1'(t) > = 0,				             V
*     < C1(t) - C2O(r), C2'(r) > = 0,				             V
* where C2O is an offset curve of C2 by amount larger than subdivision tol.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves to solve for their antipodal locations.      M
*   Epsilon:     Tolerance of computation.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of pairs of parameters in the r & t coefficients. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCntctTangentialCrvCrvC1                                              M
*****************************************************************************/
MvarPtStruct *MvarCntctTangentialCrvCrvC1(const CagdCrvStruct *Crv1,
					  const CagdCrvStruct *Crv2,
					  CagdRType Epsilon)
{
    CagdBType BspGeom, OffsetContstraints;
    int MaxDim1 = CAGD_NUM_OF_PT_COORD(Crv1 -> PType),
        MaxDim2 = CAGD_NUM_OF_PT_COORD(Crv2 -> PType);
    CagdRType t, TMin1, TMax1, TMin2, TMax2;
    MvarPtStruct *ContactPoints;
    CagdCrvStruct *Crv1Off, *Crv2Off, *TCrv1, *TCrv2;
    MvarMVStruct *MVCrv1, *MVDCrv1, *MVCrv2, *MVDCrv2, *MVTemp, *MVCrvDiff,
      **MVSplit, *MVs[4];
    MvarConstraintType Constrs[4];

    if (CAGD_IS_RATIONAL_CRV(Crv1) || CAGD_IS_RATIONAL_CRV(Crv2)) {
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    /* Make sure curves have open end condition and are compatible. */
    if (CAGD_IS_BEZIER_CRV(Crv1))
	TCrv1 = CagdCnvrtBzr2BspCrv(Crv1);
    else
        TCrv1 = CagdCnvrtBsp2OpenCrv(Crv1);

    if (CAGD_IS_BEZIER_CRV(Crv2))
	TCrv2 = CagdCnvrtBzr2BspCrv(Crv2);
    else
	TCrv2 = CagdCnvrtBsp2OpenCrv(Crv2);

    CagdCrvDomain(TCrv1, &TMin1, &TMax1);
    CagdCrvDomain(TCrv2, &TMin2, &TMax2);

    BspGeom = CAGD_IS_BSPLINE_CRV(TCrv1);

    /* Compute the first two equality in X and Y constraints. */
    MVTemp = MvarCrvToMV(TCrv1);

    MVCrv1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    if (BspGeom)
        BspKnotAffineTransOrder2(MVCrv1 -> KnotVectors[1], MVCrv1 -> Orders[1],
				 MVCrv1 -> Lengths[1] + MVCrv1 -> Orders[1],
				 TMin2, TMax2);    
    MvarMVFree(MVTemp);
    MVTemp = MvarCrvToMV(TCrv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    if (BspGeom)
        BspKnotAffineTransOrder2(MVCrv2 -> KnotVectors[0], MVCrv2 -> Orders[0],
				 MVCrv2 -> Lengths[0] + MVCrv2 -> Orders[0],
				 TMin1, TMax1);  
    MvarMVFree(MVTemp);

    MVCrvDiff = MvarMVSub(MVCrv1, MVCrv2);

    MVSplit = MvarMVSplitScalar(MVCrvDiff);
    MVs[0] = MVSplit[1];
    MVs[1] = MVSplit[2];
    if (MVSplit[3] != NULL)
        MvarMVFree(MVSplit[3]);

    MvarMVFree(MVCrvDiff);

    OffsetContstraints = TRUE;
    /* Verify that the normal field will be continuous. */
    if ((MaxDim1 == 2 && !BspCrvKnotC1Discont(TCrv1, &t)) ||
	(MaxDim1 == 3 && TCrv1 -> Order > 2 && !BspCrvKnotC2Discont(TCrv1, &t))) {
        /* Compute Offset(Crv1(t)) - Crv2(r). */
        Crv1Off = SymbCrvSubdivOffset(TCrv1, MVAR_CRV_CONTACT_SUBDIV_TOL * 10,
				      MVAR_CRV_CONTACT_OFFSET_TOL, FALSE);
	MVTemp = MvarCrvToMV(Crv1Off);
	MvarMVFree(MVCrv1);
	CagdCrvFree(Crv1Off);
	MVCrv1 = MvarPromoteMVToMV2(MVTemp, 2, 0);

	if (BspGeom)
	    BspKnotAffineTransOrder2(MVCrv1 -> KnotVectors[1],
				     MVCrv1 -> Orders[1],
				     MVCrv1 -> Lengths[1] +
				                          MVCrv1 -> Orders[1],
				     TMin2, TMax2); 

	MvarMVFree(MVTemp);
    }
    else if ((MaxDim2 == 2 && !BspCrvKnotC1Discont(TCrv2, &t)) ||
	     (MaxDim2 == 3 && TCrv2 -> Order > 2 && !BspCrvKnotC2Discont(TCrv2, &t))) {
        /* Compute Crv1(t) - Offset(Crv2(r)). */
        Crv2Off = SymbCrvSubdivOffset(TCrv2, MVAR_CRV_CONTACT_SUBDIV_TOL * 10,
				      MVAR_CRV_CONTACT_OFFSET_TOL, FALSE);
	MVTemp = MvarCrvToMV(Crv2Off);
	MvarMVFree(MVCrv2);
	CagdCrvFree(Crv2Off);
	MVCrv2 = MvarPromoteMVToMV2(MVTemp, 2, 1);

	if (BspGeom)
	    BspKnotAffineTransOrder2(MVCrv2 -> KnotVectors[0],
				     MVCrv2 -> Orders[0],
				     MVCrv2 -> Lengths[0] +
				                          MVCrv2 -> Orders[0],
				     TMin1, TMax1);  
	MvarMVFree(MVTemp);
    }
    else
        OffsetContstraints = FALSE;

    if (OffsetContstraints) {
        MVCrvDiff = MvarMVSub(MVCrv1, MVCrv2);

	/* Compute derivatives of Crv1, DCrv2. */
	MVDCrv1 = MvarMVDerive(MVCrv1, 0);
	MVDCrv2 = MvarMVDerive(MVCrv2, 1);

	/* < C1(t) - C2(r), C1'(t) >, but one curve is offset! */
	MVs[2] = MvarMVDotProd(MVCrvDiff, MVDCrv1);

	/* < C1(t) - C2(r), C2'(r) >, but one curve is offset! */
	MVs[3] = MvarMVDotProd(MVCrvDiff, MVDCrv2);

	Constrs[0] = Constrs[1] = Constrs[2] = Constrs[3] = MVAR_CNSTRNT_ZERO;

	ContactPoints = MvarMVsZeros(MVs, Constrs, 4,
				     MVAR_CRV_CONTACT_SUBDIV_TOL, -Epsilon);

        MvarMVFree(MVs[2]);
	MvarMVFree(MVs[3]);
	MvarMVFree(MVDCrv1);
	MvarMVFree(MVDCrv2);
	MvarMVFree(MVCrvDiff);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_CURVATURE_CONT);
        ContactPoints = NULL;
    }

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);

    return ContactPoints;
}
