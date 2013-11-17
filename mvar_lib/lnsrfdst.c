/******************************************************************************
* LnSrfDst.c - computes maximal distance between a line and surface.          *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb. 2011.					      *
******************************************************************************/

#include <assert.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "mvar_loc.h"

#define MVAR_LN_SRF_SUBDIV_DIST	1e-4

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the maximal XY distance location of given curve from origin.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To examine for its maximal XY distance for the origin.         M
*   Epsilon:  Tolerance of computation.	               		             M
*   Param: Will be set with the parameter location where this maximum occur. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:     The distance.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvMaxXYOriginDistance                                               M
*****************************************************************************/
CagdRType MvarCrvMaxXYOriginDistance(const CagdCrvStruct *Crv,
				     CagdRType Epsilon,
				     CagdRType *Param)
{
    IRIT_STATIC_DATA const CagdRType
	ZeroZScale[3] = { 1.0, 1.0, 0.0 };		/* Zero the Z axis. */
    BspMultComputationmethodType
        OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
        OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType *R, TMin, TMax, t,
        MaxDistSqr = 0.0;
    CagdPType P;
    CagdCrvStruct *DCrv, *TCrv;
    CagdPtStruct *Pts, *Pt;

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Examine the end locations. */
    R = CagdCrvEval(Crv, TMin);
    CagdCoerceToE2(P, &R, -1, Crv -> PType);
    MaxDistSqr = IRIT_SQR(P[0]) + IRIT_SQR(P[1]);

    R = CagdCrvEval(Crv, TMax);
    CagdCoerceToE2(P, &R, -1, Crv -> PType);
    t = IRIT_SQR(P[0]) + IRIT_SQR(P[1]);
    MaxDistSqr = IRIT_MAX(MaxDistSqr, t);

    /* Examine the extremas which are the solutions to <Crv, Crv'> = 0. */
    DCrv = CagdCrvDerive(Crv);
    CagdCrvScale(DCrv, ZeroZScale);
    TCrv = SymbCrvDotProd(Crv, DCrv);
    CagdCrvFree(DCrv);
    Pts = SymbCrvZeroSet(TCrv, 1, Epsilon, TRUE);
    CagdCrvFree(TCrv);

    *Param = IRIT_INFNTY;
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        /* Examine the location. */
        R = CagdCrvEval(Crv, Pt -> Pt[0]);
	CagdCoerceToE2(P, &R, -1, Crv -> PType);
	t = IRIT_SQR(P[0]) + IRIT_SQR(P[1]);
	if (t > MaxDistSqr) {
	    MaxDistSqr = t;
	    *Param = Pt -> Pt[0];
	}
    }
    CagdPtFreeList(Pts);

    MvarBspMultComputationMethod(OldMvarInterpFlag);
    BspMultComputationMethod(OldInterpFlag);

    return sqrt(MaxDistSqr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the maximal distance between a given line segment to a given    M
* surface.  This is the one sided Hausdorff distance from the line to Srf.   M
*   The line segment is assumed to be on the surface and is prescribed by    M
* two surface locations.				                     M
*   The distance is bounded as follow.  The composition of Srf(UV1UV2) is    M
* computed and the maximal distance to line is used as an upper bound, UB.   M
*   Then, assuming the line is the Z axis, the farthest solution to:	     M
*        Nz(u, v) = 0,							     V 
*        Nx(u, v) Y(u, v) - Ny(u, v) X(u, v) = 0,			     V 
* where Srf = (X(u, v), Y(u, v), Z(u, v)), and				     V
* (Nx, Ny, Nz) is its normal field, that is smaller than UB is selected.     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Surface to compute the line distance to.                    M
*   UV1, UV2:    The two points on Srf the prescribes the line.		     M
*   ClosedDir:	 If a valid direction the surface is to be treated as closed M
*		 in that direction when examining the line segment.	     M
*   Epsilon:     Tolerance of computation.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The maximal distance from a point on the line to nearest    M
*                location on the surfaces.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvMaxXYOriginDistance                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfLineOneSidedMaxDist                                               M
*****************************************************************************/
CagdRType MvarSrfLineOneSidedMaxDist(const CagdSrfStruct *Srf,
				     const CagdUVType UV1,
				     const CagdUVType UV2,
				     CagdSrfDirType ClosedDir,
				     CagdRType Epsilon)
{
    int i;
    BspMultComputationmethodType
        OldMvarInterpFlag = MvarBspMultComputationMethod(BSP_MULT_BEZ_DECOMP),
        OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType *R, UpperBoundSqr, t, Param,
        BoundSqr = 0.0;
    CagdPType Pt1, Pt2, Pt;
    IrtVecType Dir;
    IrtHmgnMatType Mat1, Mat2;
    CagdPtStruct Pt1UV, Pt2UV;
    CagdCrvStruct *TCrv1, *TCrv2;
    CagdSrfStruct *TSrf, *NSrf;
    MvarPtStruct *MVPts, *MVPt;
    MvarMVStruct *MVSrf1, *MVSrf2, *MVs[2], *MVSrf, *MVNSrf,
        *MVSrfSplit[MVAR_MAX_PT_SIZE], *MVNSrfSplit[MVAR_MAX_PT_SIZE];
    MvarConstraintType Constraints[2];

    R = CagdSrfEval(Srf, UV1[0], UV1[1]);
    CagdCoerceToE3(Pt1, &R, -1, Srf -> PType);

    R = CagdSrfEval(Srf, UV2[0], UV2[1]);
    CagdCoerceToE3(Pt2, &R, -1, Srf -> PType);

    /* Transform the surfaces so Pt1 is at the origin and Pt2 is above it so */
    /* the line is the +Z axis.						     */
    IRIT_VEC_SUB(Dir, Pt2, Pt1);
    GMGenMatrixZ2Dir(Mat2, Dir);
    MatInverseMatrix(Mat2, Mat1);			   /* Need Dir to Z. */
    MatGenMatTrans(-Pt1[0], -Pt1[1], -Pt1[2], Mat2);
    MatMultTwo4by4(Mat1, Mat2, Mat1);
    TSrf = CagdSrfMatTransform(Srf, Mat1);

    /* Compute the composition of TSrf(UV1UV2). */
    IRIT_UV_COPY(Pt1UV.Pt, UV1);
    IRIT_UV_COPY(Pt2UV.Pt, UV2);
    Pt1UV.Pt[2] = Pt2UV.Pt[2] = 0.0;
    TCrv1 = CagdMergePtPt(&Pt1UV, &Pt2UV);
    if (CAGD_NUM_OF_PT_COORD(TCrv1 -> PType) < 2) {
        TCrv2 = CagdCoerceCrvsTo(TCrv1, CAGD_PT_E2_TYPE, FALSE);
	CagdCrvFree(TCrv1);
	TCrv1 = TCrv2;
    }
    TCrv2 = SymbComposeSrfCrv(TSrf, TCrv1);

    t = MvarCrvMaxXYOriginDistance(TCrv2, Epsilon, &Param);

#   ifdef DEBUG_DUMP_UPPER_BOUND_INFO
    CagdDbg(Srf);
    CagdDbg(TSrf);
    CagdDbg(TCrv2);
    fprintf(stderr, "t = %f\n", Param);
#endif /* DEBUG_DUMP_UPPER_BOUND_INFO */

    UpperBoundSqr = IRIT_SQR(t);
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    if (UpperBoundSqr < IRIT_UEPS) {
        MvarBspMultComputationMethod(OldMvarInterpFlag);
	BspMultComputationMethod(OldInterpFlag);
	CagdSrfFree(TSrf);

        return sqrt(UpperBoundSqr);
    }

    /* Solve for Nz(u, v) = 0,					         */
    /*           Nx(u, v) Y(u, v) - Ny(u, v) X(u, v) = 0,	         */
    MVSrf = MvarSrfToMV(TSrf);
    NSrf = SymbSrfNormalSrf(TSrf);
    MVNSrf = MvarSrfToMV(NSrf);
    CagdSrfFree(NSrf);

    IRIT_GEN_COPY(MVSrfSplit, MvarMVSplitScalar(MVSrf),
		  sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    IRIT_GEN_COPY(MVNSrfSplit, MvarMVSplitScalar(MVNSrf),
		  sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);

    MVs[0] = MVNSrfSplit[3];
    MVNSrfSplit[3] = NULL;
    MVSrf1 = MvarMVMult(MVNSrfSplit[2], MVSrfSplit[1]);
    MVSrf2 = MvarMVMult(MVNSrfSplit[1], MVSrfSplit[2]);
    MVs[1] = MvarMVSub(MVSrf1, MVSrf2);
    MvarMVFree(MVSrf1);
    MvarMVFree(MVSrf2);

    for (i = 0; i <= 3; i++) {
        if (MVSrfSplit[i] != NULL)
	    MvarMVFree(MVSrfSplit[i]);
        if (MVNSrfSplit[i] != NULL)
	    MvarMVFree(MVNSrfSplit[i]);
    }
    MvarMVFree(MVSrf);
    MvarMVFree(MVNSrf);

    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 2,
			 MVAR_LN_SRF_SUBDIV_DIST, Epsilon);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    /* Examine the identified locations. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        R = CagdSrfEval(TSrf, MVPt -> Pt[0], MVPt -> Pt[1]);
	CagdCoerceToE3(Pt, &R, -1, TSrf -> PType);
	
	if ((t = IRIT_SQR(Pt[0]) + IRIT_SQR(Pt[1])) <= UpperBoundSqr ||
	    IRIT_APX_EQ(t, UpperBoundSqr))
	    BoundSqr = IRIT_MAX(BoundSqr, t);
    }

    CagdSrfFree(TSrf);
    MvarPtFreeList(MVPts);

    /* If we should consider the surface as closed, try mod domain size     */
    /* and select the minimum.						    */
    if (ClosedDir != CAGD_NO_DIR) {
        CagdRType Bound2, UMin, UMax, VMin, VMax;
	CagdUVType UV;
	CagdSrfStruct
	    *Srf2 = CagdMergeSrfSrf(Srf, Srf, ClosedDir, TRUE, FALSE);

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	IRIT_UV_COPY(UV, UV1);
	switch (ClosedDir) {
	    case CAGD_CONST_U_DIR:
		UV[0] += UMax - UMin;
		break;
	    case CAGD_CONST_V_DIR:
		UV[1] += VMax - VMin;
		break;
	    default:
		assert(0);
		break;
	}
	Bound2 = MvarSrfLineOneSidedMaxDist(Srf2, UV, UV2, CAGD_NO_DIR,
					    Epsilon);
	CagdSrfFree(Srf2);

	BoundSqr = IRIT_MIN(BoundSqr, IRIT_SQR(Bound2));
    }

    MvarBspMultComputationMethod(OldMvarInterpFlag);
    BspMultComputationMethod(OldInterpFlag);

    return sqrt(BoundSqr);
}
