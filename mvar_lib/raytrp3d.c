/******************************************************************************
* Raytrp3d.c - computes solution for rays bouncing for every between surfaces.*
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June. 08.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "mvar_loc.h"

#define MVAR_RAY_TRAP_MAX_SRFS 100
#define MVAR_RAY_TRAP_FILTER_REVERSED   /* Filter rays bouncing into object. */

static MvarPtStruct *MvarComputeRayTraps3DET(const CagdSrfStruct *Srfs,
					     int Orient,
					     CagdRType SubdivTol,
					     CagdRType NumerTol);
static MvarPtStruct *MvarComputeRayTraps3DMV(const CagdSrfStruct *Srfs,
					   int Orient,
					   CagdRType SubdivTol,
					   CagdRType NumerTol);
static MvarExprTreeStruct *MvarBuildOneRayTrap3DConstrntET(
					     int i,
					     int n,
					     MvarMVStruct *Mv1,
					     MvarMVStruct *Mv2,
					     MvarMVStruct *Mv3,
					     MvarMVStruct *Mv2Nrml,
					     int Orient,
					     MvarExprTreeStruct **ETMVCoplanar,
					     MvarExprTreeStruct **ETMVOrient);
static MvarMVStruct *MvarBuildOneRayTrap3DConstrntMV(MvarMVStruct *Mv1,
						     MvarMVStruct *Mv2,
						     MvarMVStruct *Mv3,
						     MvarMVStruct *Mv2Nrml,
						     int Orient,
						     MvarMVStruct **MVCoplanar,
						     MvarMVStruct **MVOrient);
static MvarPtStruct *MvarPostProcessRayTrap3DPts(int n,
						 MvarPtStruct *Pts,
						 const CagdSrfStruct *Srfs,
						 const CagdRType *UMin,
						 const CagdRType *UMax,
						 const CagdRType *VMin,
						 const CagdRType *VMax);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes solutions to locations on the given surfaces that would bounce  M
* rays from one surface to the next in a closed loop.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:         List of surfaces to handle in order (cyclically).          M
*   Orient:       Pick the proper orientation with respect to the normal if  M
*		  TRUE.  May be faster at times.			     M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*   UseExprTree:  TRUE to use expression trees in the computation, FALSE to  M
*		  use regular multivariate expressions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    M
*		values of the different Srfs.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarComputeRayTraps3D                                                    M
*****************************************************************************/
MvarPtStruct *MvarComputeRayTraps3D(const CagdSrfStruct *Srfs,
				    int Orient,
				    CagdRType SubdivTol,
				    CagdRType NumerTol,
				    CagdBType UseExprTree)
{
    return UseExprTree ? MvarComputeRayTraps3DET(Srfs, Orient,
					         SubdivTol, NumerTol)
		       : MvarComputeRayTraps3DMV(Srfs, Orient,
					         SubdivTol, NumerTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes solutions to locations on the given surfaces that would bounce  *
* rays from one srf to the next in a closed loop, using expression trees.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srfs:         List of surfaces to handle in order (cyclically).          *
*   Orient:       Pick the proper orientation with respect to the normal if  *
*		  TRUE.  May be faster at times.			     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    *
*		values of the different Srfs.				     *
*****************************************************************************/
static MvarPtStruct *MvarComputeRayTraps3DET(const CagdSrfStruct *Srfs,
					     int Orient,
					     CagdRType SubdivTol,
					     CagdRType NumerTol)
{
    int i, OldCnvrt,
        OldBzr2MVs = MvarExprTreeZerosCnvrtBezier2MVs(FALSE),
	n = CagdListLength(Srfs);
    const CagdSrfStruct *Srf;
    CagdRType UMin[MVAR_RAY_TRAP_MAX_SRFS], UMax[MVAR_RAY_TRAP_MAX_SRFS],
              VMin[MVAR_RAY_TRAP_MAX_SRFS], VMax[MVAR_RAY_TRAP_MAX_SRFS];
    CagdSrfStruct *STmp;
    MvarMVStruct *MVSrfs[MVAR_RAY_TRAP_MAX_SRFS],
		 *MVNrmls[MVAR_RAY_TRAP_MAX_SRFS];
    MvarExprTreeStruct *MVETCnstrns[MVAR_RAY_TRAP_MAX_SRFS];
    MvarConstraintType Constraints[MVAR_RAY_TRAP_MAX_SRFS];
    MvarPtStruct *Pts;

    if (n * 3 > MVAR_RAY_TRAP_MAX_SRFS) {
	MVAR_FATAL_ERROR(MVAR_ERR_TOO_MANY_PARAMS);
        MvarExprTreeZerosCnvrtBezier2MVs(OldBzr2MVs);
	return NULL;
    }

    /* Convert the surfaces into multivariates and promote each to an       */
    /* n-dimensional variety, spanning dimension i.			    */
    for (Srf = Srfs, i = 0; Srf != NULL; Srf = Srf -> Pnext, i++) {
	CagdSrfStruct *Srf2;
	MvarMVStruct *MVTmp;

	Srf2 = CagdSrfCopy(Srf);
	CagdSrfDomain(Srf2, &UMin[i], &UMax[i], &VMin[i], &VMax[i]);
	if (CAGD_IS_BSPLINE_SRF(Srf2)) {
	    BspKnotAffineTransOrder2(Srf2 -> UKnotVector, Srf2 -> UOrder,
				     Srf2 -> ULength + Srf2 -> UOrder,
				     0.0, 1.0);
	    BspKnotAffineTransOrder2(Srf2 -> VKnotVector, Srf2 -> VOrder,
				     Srf2 -> VLength + Srf2 -> VOrder,
				     0.0, 1.0);
	}
	MVTmp = MvarSrfToMV(Srf2);

	MVSrfs[i] = MvarPromoteMVToMV2(MVTmp, 2 * n, 2 * i);
	MvarMVFree(MVTmp);

	/* Now compute a normal field for the surface. */
	STmp = SymbSrfNormalSrf(Srf2);
	MVTmp = MvarSrfToMV(STmp);
	CagdSrfFree(Srf2);
	CagdSrfFree(STmp);
	MVNrmls[i] = MvarPromoteMVToMV2(MVTmp, 2 * n, 2 * i);
	MvarMVFree(MVTmp);
    }

    /* Build the bouncing ray's constraints. */
    for (i = 0; i < n; i++) {
        MVETCnstrns[i] = MvarBuildOneRayTrap3DConstrntET(
						    i, n,
						    MVSrfs[i],
						    MVSrfs[(i + 1) % n],
						    MVSrfs[(i + 2) % n],
						    MVNrmls[(i + 1) % n],
						    Orient,
						    &MVETCnstrns[i + n],
						    &MVETCnstrns[i + 2 * n]);
	Constraints[i] = MVAR_CNSTRNT_ZERO;
	Constraints[i + n] = MVAR_CNSTRNT_ZERO;
	Constraints[i + 2 * n] = MVAR_CNSTRNT_POSITIVE;
    }

    /* Solve using expression trees but convert Beziers to MVs. */
    OldCnvrt = MvarExprTreeZerosCnvrtBezier2MVs(TRUE);
    Pts = MvarExprTreesZeros(MVETCnstrns, Constraints,
			     2 * n + (Orient ? n : 0),
			     SubdivTol, NumerTol);
    MvarExprTreeZerosCnvrtBezier2MVs(OldCnvrt);

    for (i = 0; i < n; i++) {
	MvarMVFree(MVSrfs[i]);
	MvarMVFree(MVNrmls[i]);
        MvarExprTreeFree(MVETCnstrns[i], FALSE);
        MvarExprTreeFree(MVETCnstrns[i + n], FALSE);
	if (Orient)
	    MvarExprTreeFree(MVETCnstrns[i + 2 * n], FALSE);
    }

    MvarExprTreeZerosCnvrtBezier2MVs(OldBzr2MVs);

    return MvarPostProcessRayTrap3DPts(n, Pts, Srfs, UMin, UMax, VMin, VMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Build the ray constraint of a ray coming from Mv1 bouncing of Mv2 to Mv3 *
* as an angular constraint on the coming (to Mv2) and leaving (Mv2) angles   *
* with respect to the normal of Mv2.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   i:             The constraint is for the i'th surface.                   *
*   n:             Number of surfaces involved.                              *
*   Mv1, Mv2, Mv3: Three consecutive surfaces to formulate constraint for.   *
*   Mv2Nrml:	   The normal field of Mv2.				     *
*   Orient:        If TRUE, synthesize an orientation setting constraint     *
*		   into MVOrient.					     *
*   ETMVCoplanar:  Coplanarity constraint of incoming ray, outgoing ray and  *
*                  the normal.						     *
*   ETMVOrient:    Optional orientation constraint (if Orient is TRUE).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarExprTreeStruct *:  A zero set ET constraint for angular equality.    *
*****************************************************************************/
static MvarExprTreeStruct *MvarBuildOneRayTrap3DConstrntET(
					     int i,
					     int n,
					     MvarMVStruct *Mv1,
					     MvarMVStruct *Mv2,
					     MvarMVStruct *Mv3,
					     MvarMVStruct *Mv2Nrml,
					     int Orient,
					     MvarExprTreeStruct **ETMVCoplanar,
					     MvarExprTreeStruct **ETMVOrient)
{
    MvarExprTreeStruct *Dist12, *Dist32, *Ang12, *Ang32, *Ang12Sqr, *Ang32Sqr,
	*Dist12Sqr, *Dist32Sqr, *Mult1, *Mult2, *RetVal;

    Dist12 = MvarExprTreeSub(MvarExprTreeFromMV2(Mv1),
			     MvarExprTreeFromMV2(Mv2));
    Dist32 = MvarExprTreeSub(MvarExprTreeFromMV2(Mv3),
			     MvarExprTreeFromMV2(Mv2));    
    Ang12 = MvarExprTreeDotProd(Dist12,	MvarExprTreeFromMV2(Mv2Nrml));
    Ang32 = MvarExprTreeDotProd(Dist32,	MvarExprTreeFromMV2(Mv2Nrml));

    Ang12Sqr = MvarExprTreeMult(Ang12, Ang12);
    Ang32Sqr = MvarExprTreeMult(Ang32, Ang32);
    Dist12Sqr = MvarExprTreeDotProd(Dist12, Dist12);
    Dist32Sqr = MvarExprTreeDotProd(Dist32, Dist32);
    Mult1 = MvarExprTreeMult(Ang12Sqr, Dist32Sqr);
    Mult2 = MvarExprTreeMult(Ang32Sqr, Dist12Sqr);
    RetVal = MvarExprTreeSub(MvarExprTreeCopy(Mult1, FALSE, TRUE),
			     MvarExprTreeCopy(Mult2, FALSE, TRUE));

    /* Build the coplanarity constraint. */
    *ETMVCoplanar = MvarExprTreeDotProd(
	        MvarExprTreeCrossProd(MvarExprTreeCopy(Dist12, FALSE, TRUE),
				      MvarExprTreeCopy(Dist32, FALSE, TRUE)),
		MvarExprTreeFromMV2(Mv2Nrml));

    if (Orient) {           /* Build the orientation inequality constraint. */
	*ETMVOrient = MvarExprTreeMult(MvarExprTreeCopy(Ang12, FALSE, TRUE),
				       MvarExprTreeCopy(Ang32, FALSE, TRUE));
    }

    MvarExprTreeFree(Ang12, FALSE); /* Also deletes Dist12. */
    MvarExprTreeFree(Ang32, FALSE); /* Also deletes Dist32. */
    MvarExprTreeFree(Ang12Sqr, TRUE);
    MvarExprTreeFree(Ang32Sqr, TRUE);
    MvarExprTreeFree(Dist12Sqr, TRUE);
    MvarExprTreeFree(Dist32Sqr, TRUE);
    MvarExprTreeFree(Mult1, TRUE);
    MvarExprTreeFree(Mult2, TRUE);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes solutions to locations on the given surfaces that would bounce  *
* rays from one surface to the next in a closed loop, using multivariates.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Srfs:         List of surfaces to handle in order (cyclically).          *
*   Orient:       Pick the proper orientation with respect to the normal if  *
*		  TRUE.  May be faster at times.			     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    *
*		values of the different Srfs.				     *
*****************************************************************************/
static MvarPtStruct *MvarComputeRayTraps3DMV(const CagdSrfStruct *Srfs,
					     int Orient,
					     CagdRType SubdivTol,
					     CagdRType NumerTol)
{
    int i,
	n = CagdListLength(Srfs);
    const CagdSrfStruct *Srf;
    CagdRType UMin[MVAR_RAY_TRAP_MAX_SRFS], UMax[MVAR_RAY_TRAP_MAX_SRFS],
              VMin[MVAR_RAY_TRAP_MAX_SRFS], VMax[MVAR_RAY_TRAP_MAX_SRFS];
    CagdSrfStruct *STmp;
    MvarMVStruct *MVSrfs[MVAR_RAY_TRAP_MAX_SRFS],
		 *MVNrmls[MVAR_RAY_TRAP_MAX_SRFS],
		 *MVCnstrns[MVAR_RAY_TRAP_MAX_SRFS];
    MvarConstraintType Constraints[MVAR_RAY_TRAP_MAX_SRFS];
    MvarPtStruct *Pts;

    if (n * 3 > MVAR_RAY_TRAP_MAX_SRFS) {
	MVAR_FATAL_ERROR(MVAR_ERR_TOO_MANY_PARAMS);
	return NULL;
    }

    /* Convert the surfaces into multivariates and promote each to an       */
    /* n-dimensional variety, spanning dimension i.			    */
    for (Srf = Srfs, i = 0; Srf != NULL; Srf = Srf -> Pnext, i++) {
	CagdSrfStruct *Srf2;
	MvarMVStruct *MVTmp;

	Srf2 = CagdSrfCopy(Srf);
	CagdSrfDomain(Srf2, &UMin[i], &UMax[i], &VMin[i], &VMax[i]);
	if (CAGD_IS_BSPLINE_SRF(Srf2)) {
	    BspKnotAffineTransOrder2(Srf2 -> UKnotVector, Srf2 -> UOrder,
				     Srf2 -> ULength + Srf2 -> UOrder,
				     0.0, 1.0);
	    BspKnotAffineTransOrder2(Srf2 -> VKnotVector, Srf2 -> VOrder,
				     Srf2 -> VLength + Srf2 -> VOrder,
				     0.0, 1.0);
	}
	MVTmp = MvarSrfToMV(Srf2);

	MVSrfs[i] = MvarPromoteMVToMV2(MVTmp, 2 * n, 2 * i);
	MvarMVFree(MVTmp);

	/* Now compute a normal field for the surface. */
	STmp = SymbSrfNormalSrf(Srf2);
	MVTmp = MvarSrfToMV(STmp);
	CagdSrfFree(Srf2);
	CagdSrfFree(STmp);
	MVNrmls[i] = MvarPromoteMVToMV2(MVTmp, 2 * n, 2 * i);
	MvarMVFree(MVTmp);
    }

    /* Build the bouncing ray's constraints. */
    for (i = 0; i < n; i++) {
	MVCnstrns[i] = MvarBuildOneRayTrap3DConstrntMV(
						  MVSrfs[i],
						  MVSrfs[(i + 1) % n],
						  MVSrfs[(i + 2) % n],
						  MVNrmls[(i + 1) % n],
						  Orient,
						  &MVCnstrns[i + n],
						  &MVCnstrns[i + 2 * n]);
	Constraints[i] = MVAR_CNSTRNT_ZERO;
	Constraints[i + n] = MVAR_CNSTRNT_ZERO;
	Constraints[i + 2 * n] = MVAR_CNSTRNT_POSITIVE;
    }
    
    Pts = MvarMVsZeros(MVCnstrns, Constraints, 2 * n + (Orient ? n : 0),
		       SubdivTol, NumerTol);

    for (i = 0; i < n; i++) {
	MvarMVFree(MVSrfs[i]);
	MvarMVFree(MVNrmls[i]);
        MvarMVFree(MVCnstrns[i]);
        MvarMVFree(MVCnstrns[i + n]);
	if (Orient)
	    MvarMVFree(MVCnstrns[i + 2 * n]);
    }

    return MvarPostProcessRayTrap3DPts(n, Pts, Srfs, UMin, UMax, VMin, VMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Build the ray constraint of a ray coming from Mv1 bouncing of Mv2 to Mv3 *
* as an angular constraint on the coming (to Mv2) and leaving (Mv2) angles   *
* with respect to the normal of Mv2.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mv1, Mv2, Mv3: Three consecutive surfaces to formulate constraint for.   *
*   Mv2Nrml:	   The normal field of Mv2.				     *
*   Orient:        If TRUE, synthesize an orientation setting constraint     *
*		   into MVOrient.					     *
*   MVOrient:      Optional orientation constraint (if Orient is TRUE).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:  A zero set constraint for the angular equality.         *
*****************************************************************************/
static MvarMVStruct *MvarBuildOneRayTrap3DConstrntMV(MvarMVStruct *Mv1,
						     MvarMVStruct *Mv2,
						     MvarMVStruct *Mv3,
						     MvarMVStruct *Mv2Nrml,
						     int Orient,
						     MvarMVStruct **MVCoplanar,
						     MvarMVStruct **MVOrient)
{
    MvarMVStruct *MvCross,
	*Dist12 = MvarMVSub(Mv1, Mv2),
	*Dist32 = MvarMVSub(Mv3, Mv2),
	*Ang12 = MvarMVDotProd(Dist12, Mv2Nrml),
	*Ang32 = MvarMVDotProd(Dist32, Mv2Nrml),
	*Ang12Sqr = MvarMVMult(Ang12, Ang12),
	*Ang32Sqr = MvarMVMult(Ang32, Ang32),
	*Dist12Sqr = MvarMVDotProd(Dist12, Dist12),
	*Dist32Sqr = MvarMVDotProd(Dist32, Dist32),
	*Mult1 = MvarMVMult(Ang12Sqr, Dist32Sqr),
	*Mult2 = MvarMVMult(Ang32Sqr, Dist12Sqr),
	*RetVal = MvarMVSub(Mult1, Mult2);

    /* Build the coplanarity constraint. */
    *MVCoplanar = MvarMVDotProd(MvCross = MvarMVCrossProd(Dist12, Dist32),
				Mv2Nrml);

    if (Orient) {	   /* Build the orientation inequality constraint. */
	*MVOrient = MvarMVMult(Ang12, Ang32);
    }

    MvarMVFree(MvCross);
    MvarMVFree(Dist12);
    MvarMVFree(Dist32);
    MvarMVFree(Ang12);
    MvarMVFree(Ang32);
    MvarMVFree(Ang12Sqr);
    MvarMVFree(Ang32Sqr);
    MvarMVFree(Dist12Sqr);
    MvarMVFree(Dist32Sqr);
    MvarMVFree(Mult1);
    MvarMVFree(Mult2);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Post-process the computed ray-trap points - map to original domains and  *
* possibly filter our wrongly oriented solutions.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   n:             Number of surfaces                                        *
*   Pts:           Solution points to filter.                                *
*   Srfs:          Input ray-trap surfaces.                                  *
*   UMin, UMax:    Vector of size n of all U domains of all surfaces.        *
*   VMin, VMax:    Vector of size n of all V domains of all surfaces.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Filtered points.                                        *
*****************************************************************************/
static MvarPtStruct *MvarPostProcessRayTrap3DPts(int n,
						 MvarPtStruct *Pts,
						 const CagdSrfStruct *Srfs,
						 const CagdRType *UMin,
						 const CagdRType *UMax,
						 const CagdRType *VMin,
						 const CagdRType *VMax)
{
    int i;
    MvarPtStruct *Pt;

    /* Recover the solution's domain as we transformed to (0, 1) domains. */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        for (i = 0; i < n; i++) {
	    Pt -> Pt[i * 2] = Pt -> Pt[i * 2] * (UMax[i] - UMin[i]) + UMin[i];
	    Pt -> Pt[i * 2 + 1] = Pt -> Pt[i * 2 + 1] * (VMax[i] - VMin[i])
	                                                            + VMin[i];
	}
    }

#ifdef MVAR_RAY_TRAP_FILTER_REVERSED
    {
        MvarPtStruct *PtNext;
	CagdPType Pos[MVAR_RAY_TRAP_MAX_SRFS];
	CagdVType Nrml[MVAR_RAY_TRAP_MAX_SRFS];
	const CagdSrfStruct *Srf;

	Pt = Pts;
	Pts = NULL;
	while (Pt != NULL) {
	    PtNext = Pt -> Pnext;

	    Pt -> Pnext = NULL;

	    /* Evaluate each points in case we are in wrong orientation. */
	    for (Srf = Srfs, i = 0; Srf != NULL; Srf = Srf -> Pnext, i++) {
		CagdRType
		    *R = CagdSrfEval(Srf, Pt -> Pt[i * 2],
				          Pt -> Pt[i * 2 + 1]);
		CagdVecStruct
		    *V = CagdSrfNormal(Srf, Pt -> Pt[i * 2],
				            Pt -> Pt[i * 2 + 1], FALSE);

		CagdCoerceToE3(Pos[i], &R, -1, Srf -> PType);
		IRIT_VEC_COPY(Nrml[i], V -> Vec);
	    }

	    /* Now test orientation of the constraint for a different sign. */
	    for (i = 0; i < n; i++) {
		CagdVType V1, V2, V1Tmp, V2Tmp;
		CagdRType
		    *N = Nrml[(i + 1) % n];

		IRIT_VEC_SUB(V1, Pos[(i + 1) % n], Pos[i]);
		IRIT_VEC_SUB(V2, Pos[(i + 1) % n], Pos[(i + 2) % n]);

		/* Test that V1/V2 point in the same normal direction. */
		if (IRIT_DOT_PROD(V1, N) *
		    IRIT_DOT_PROD(V2, N) < 0.0)
		    break;
		/* Test that V1 and V2 are in opposite sides of the normal. */
		IRIT_CROSS_PROD(V1Tmp, V1, N);
		IRIT_CROSS_PROD(V2Tmp, N, V2);
		if (IRIT_DOT_PROD(V1Tmp, V2Tmp) < 0.0)
		    break;
	    }
	    if (i < n) {
		/* Failed - purge this location. */
		MvarPtFree(Pt);
	    }
	    else {
		IRIT_LIST_PUSH(Pt, Pts);
	    }

	    Pt = PtNext;
	}
    }
    
#endif /* MVAR_RAY_TRAP_FILTER_REVERSED */

    return Pts;
}
