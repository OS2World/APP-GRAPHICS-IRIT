/******************************************************************************
* Ray-trap.c - computes solution for rays bouncing for every between curves.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 99.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "mvar_loc.h"

#define MVAR_RAY_TRAP_MAX_CRVS 100
#define MVAR_RAY_TRAP_FILTER_REVERSED   /* Filter rays bouncing into object. */

static MvarPtStruct *MvarComputeRayTrapsET(const CagdCrvStruct *Crvs,
					   int Orient,
					   CagdRType SubdivTol,
					   CagdRType NumerTol);
static MvarPtStruct *MvarComputeRayTrapsMV(const CagdCrvStruct *Crvs,
					   int Orient,
					   CagdRType SubdivTol,
					   CagdRType NumerTol);
static MvarExprTreeStruct *MvarBuildOneRayTrapConstrntET(int i,
							 int n,
							 MvarMVStruct *Mv1,
							 MvarMVStruct *Mv2,
							 MvarMVStruct *Mv3,
							 MvarMVStruct *Mv2Nrml,
							 int Orient,
							 MvarExprTreeStruct
							           **MVOrient);
static MvarMVStruct *MvarBuildOneRayTrapConstrntMV(MvarMVStruct *Mv1,
						   MvarMVStruct *Mv2,
						   MvarMVStruct *Mv3,
						   MvarMVStruct *Mv2Nrml,
						   int Orient,
						   MvarMVStruct **MVOrient);
static MvarPtStruct *MvarPostProcessRayTrapPts(int n,
					       MvarPtStruct *Pts,
					       const CagdCrvStruct *Crvs,
					       const CagdRType *TMin,
					       const CagdRType *TMax);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes solutions to locations on the given curves that would bounce    M
* rays from one curve to the next in a closed loop.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:         List of curves to handle in order (cyclically).            M
*   Orient:       Pick the proper orientation with respect to the normal if  M
*		  TRUE.  May be faster at times.			     M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the curves.			     M
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*   UseExprTree:  TRUE to use expression trees in the computation, FALSE to  M
*		  use regular multivariate expressions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    M
*		values of the different Crvs.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarComputeRayTraps                                                      M
*****************************************************************************/
MvarPtStruct *MvarComputeRayTraps(const CagdCrvStruct *Crvs,
				  int Orient,
				  CagdRType SubdivTol,
				  CagdRType NumerTol,
				  CagdBType UseExprTree)
{
    return UseExprTree ? MvarComputeRayTrapsET(Crvs, Orient,
					       SubdivTol, NumerTol)
		       : MvarComputeRayTrapsMV(Crvs, Orient,
					       SubdivTol, NumerTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes solutions to locations on the given curves that would bounce    *
* rays from one curve to the next in a closed loop, using expression trees.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:         List of curves to handle in order (cyclically).            *
*   Orient:       Pick the proper orientation with respect to the normal if  *
*		  TRUE.  May be faster at times.			     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the curves.			     *
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    *
*		values of the different Crvs.				     *
*****************************************************************************/
static MvarPtStruct *MvarComputeRayTrapsET(const CagdCrvStruct *Crvs,
					   int Orient,
					   CagdRType SubdivTol,
					   CagdRType NumerTol)
{
    int i, j,
	n = CagdListLength(Crvs);
    const CagdCrvStruct *Crv;
    CagdRType TMin[MVAR_RAY_TRAP_MAX_CRVS], TMax[MVAR_RAY_TRAP_MAX_CRVS];
    MvarMVStruct *MVCrvs[MVAR_RAY_TRAP_MAX_CRVS],
		 *MVNrmls[MVAR_RAY_TRAP_MAX_CRVS];
    MvarExprTreeStruct *MVETCnstrns[MVAR_RAY_TRAP_MAX_CRVS];
    MvarConstraintType Constraints[MVAR_RAY_TRAP_MAX_CRVS];
    MvarPtStruct *Pts;

    if (n * 2 > MVAR_RAY_TRAP_MAX_CRVS) {
	MVAR_FATAL_ERROR(MVAR_ERR_TOO_MANY_PARAMS);
	return NULL;
    }

    /* Convert the curves into multivariates and promote each to an          */
    /* n-dimensional variety, spanning dimension i.			     */
    for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++) {
	CagdRType **Points;
	CagdCrvStruct *Crv2, *DCrv;
	MvarMVStruct *MVTmp;

	Crv2 = CagdCrvCopy(Crv);
	CagdCrvDomain(Crv2, &TMin[i], &TMax[i]);
	if (CAGD_IS_BSPLINE_CRV(Crv2))
	    BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				     Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);
	MVTmp = MvarCrvToMV(Crv2);

	MVCrvs[i] = MvarPromoteMVToMV2(MVTmp, n, i);
	MvarMVFree(MVTmp);

	/* Now compute a normal field for the curve. */
	DCrv = CagdCrvDerive(Crv2);
	Points = DCrv -> Points;
	for (j = 0; j < DCrv -> Length; j++) {
	    IRIT_SWAP(CagdRType, Points[1][j], Points[2][j]);
	    Points[1][j] = -Points[1][j];
	}
	MVTmp = MvarCrvToMV(DCrv);
	CagdCrvFree(Crv2);
	CagdCrvFree(DCrv);
	MVNrmls[i] = MvarPromoteMVToMV2(MVTmp, n, i);
	MvarMVFree(MVTmp);
    }

    /* Build the bouncing ray's constraints. */
    for (i = 0; i < n; i++) {
        MVETCnstrns[i] = MvarBuildOneRayTrapConstrntET(i, n,
						       MVCrvs[i],
						       MVCrvs[(i + 1) % n],
						       MVCrvs[(i + 2) % n],
						       MVNrmls[(i + 1) % n],
						       Orient,
						       &MVETCnstrns[i + n]);
	Constraints[i] = MVAR_CNSTRNT_ZERO;
	Constraints[i + n] = MVAR_CNSTRNT_POSITIVE;
    }
    
    Pts = MvarExprTreesZeros(MVETCnstrns, Constraints, n + (Orient ? n : 0),
			     SubdivTol, NumerTol);

    for (i = 0; i < n; i++) {
	MvarMVFree(MVCrvs[i]);
	MvarMVFree(MVNrmls[i]);
        MvarExprTreeFree(MVETCnstrns[i], FALSE);
	if (Orient)
	    MvarExprTreeFree(MVETCnstrns[i + n], FALSE);
    }

    return MvarPostProcessRayTrapPts(n, Pts, Crvs, TMin, TMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Build the ray constraint of a ray coming from Mv1 bouncing of Mv2 to Mv3 *
* as an angular constraint on the coming (to Mv2) and leaving (Mv2) angles   *
* with respect to the normal of Mv2.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   i:             The constraint is for the i'th curve.                     *
*   n:             Number of curves involved.                                *
*   Mv1, Mv2, Mv3: Three consecutive curves to formulate the constraint for. *
*   Mv2Nrml:	   The normal field of Mv2.				     *
*   Orient:        If TRUE, synthesize an orientation setting constraint     *
*		   into MVOrient.					     *
*   ETMVOrient:    Optional orientation constraint (if Orient is TRUE).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarExprTreeStruct *:  A zero set ET constraint for angular equality.    *
*****************************************************************************/
static MvarExprTreeStruct *MvarBuildOneRayTrapConstrntET(int i,
							 int n,
							 MvarMVStruct *Mv1,
							 MvarMVStruct *Mv2,
							 MvarMVStruct *Mv3,
							 MvarMVStruct *Mv2Nrml,
							 int Orient,
							 MvarExprTreeStruct
							          **ETMVOrient)
{
    MvarExprTreeStruct *MVET1, *MVET2a, *MVET2b, *MVET3, *MVET2Na, *MVET2Nb,
        *Dist12, *Dist32, *Ang12, *Ang32, *Ang12Sqr, *Ang32Sqr,
	*Dist12Sqr, *Dist32Sqr, *Mult1, *Mult2, *RetVal;

    Dist12 = MvarExprTreeSub(MVET1 = MvarExprTreeFromMV2(Mv1),
			     MVET2a = MvarExprTreeFromMV2(Mv2));
    Dist32 = MvarExprTreeSub(MVET3 = MvarExprTreeFromMV2(Mv3),
			     MVET2b = MvarExprTreeFromMV2(Mv2));    
    Ang12 = MvarExprTreeDotProd(Dist12,
				MVET2Na = MvarExprTreeFromMV2(Mv2Nrml));
    Ang32 = MvarExprTreeDotProd(Dist32,
				MVET2Nb = MvarExprTreeFromMV2(Mv2Nrml));

#   ifdef DEBUG_MVAR_RAY_TRAP_ADD_INFO
    {
        char Line[IRIT_LINE_LEN];

	sprintf(Line, "C(%d)", i);
	MVET1 -> Info = IritStrdup(Line);

	sprintf(Line, "C(%d)", (i + 1) % n);
	MVET2a -> Info = IritStrdup(Line);
	MVET2b -> Info = IritStrdup(Line);

	sprintf(Line, "C(%d)", (i + 2) % n);
	MVET3 -> Info = IritStrdup(Line);

	sprintf(Line, "N(%d)", (i + 1) % n);
	MVET2Na -> Info = IritStrdup(Line);
	MVET2Nb -> Info = IritStrdup(Line);
    }
#   endif /* DEBUG_MVAR_RAY_TRAP_ADD_INFO */

    Ang12Sqr = MvarExprTreeMult(Ang12, Ang12);
    Ang32Sqr = MvarExprTreeMult(Ang32, Ang32);
    Dist12Sqr = MvarExprTreeDotProd(Dist12, Dist12);
    Dist32Sqr = MvarExprTreeDotProd(Dist32, Dist32);
    Mult1 = MvarExprTreeMult(Ang12Sqr, Dist32Sqr);
    Mult2 = MvarExprTreeMult(Ang32Sqr, Dist12Sqr);
    RetVal = MvarExprTreeSub(MvarExprTreeCopy(Mult1, FALSE, TRUE),
			     MvarExprTreeCopy(Mult2, FALSE, TRUE));

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
*   Computes solutions to locations on the given curves that would bounce    *
* rays from one curve to the next in a closed loop, using multivariates.     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:         List of curves to handle in order (cyclically).            *
*   Orient:       Pick the proper orientation with respect to the normal if  *
*		  TRUE.  May be faster at times.			     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the curves.			     *
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Linked list of solutions, each holding the parameter    *
*		values of the different Crvs.				     *
*****************************************************************************/
static MvarPtStruct *MvarComputeRayTrapsMV(const CagdCrvStruct *Crvs,
					   int Orient,
					   CagdRType SubdivTol,
					   CagdRType NumerTol)
{
    int i, j,
	n = CagdListLength(Crvs);
    const CagdCrvStruct *Crv;
    CagdRType TMin[MVAR_RAY_TRAP_MAX_CRVS], TMax[MVAR_RAY_TRAP_MAX_CRVS];
    MvarMVStruct *MVCrvs[MVAR_RAY_TRAP_MAX_CRVS],
		 *MVNrmls[MVAR_RAY_TRAP_MAX_CRVS],
		 *MVCnstrns[MVAR_RAY_TRAP_MAX_CRVS];
    MvarConstraintType Constraints[MVAR_RAY_TRAP_MAX_CRVS];
    MvarPtStruct *Pts;

    if (n * 2 > MVAR_RAY_TRAP_MAX_CRVS) {
	MVAR_FATAL_ERROR(MVAR_ERR_TOO_MANY_PARAMS);
	return NULL;
    }

    /* Convert the curves into multivariates and promote each to an          */
    /* n-dimensional variety, spanning dimension i.			     */
    for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++) {
	CagdRType **Points;
	CagdCrvStruct *Crv2, *DCrv;
	MvarMVStruct *MVTmp;

	Crv2 = CagdCrvCopy(Crv);
	CagdCrvDomain(Crv2, &TMin[i], &TMax[i]);
	if (CAGD_IS_BSPLINE_CRV(Crv2))
	    BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				     Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);
	MVTmp = MvarCrvToMV(Crv2);

	MVCrvs[i] = MvarPromoteMVToMV2(MVTmp, n, i);
	MvarMVFree(MVTmp);

	/* Now compute a normal field for the curve. */
	DCrv = CagdCrvDerive(Crv2);
	Points = DCrv -> Points;
	for (j = 0; j < DCrv -> Length; j++) {
	    IRIT_SWAP(CagdRType, Points[1][j], Points[2][j]);
	    Points[1][j] = -Points[1][j];
	}
	MVTmp = MvarCrvToMV(DCrv);
	CagdCrvFree(Crv2);
	CagdCrvFree(DCrv);
	MVNrmls[i] = MvarPromoteMVToMV2(MVTmp, n, i);
	MvarMVFree(MVTmp);
    }

    /* Build the bouncing ray's constraints. */
    for (i = 0; i < n; i++) {
	MVCnstrns[i] = MvarBuildOneRayTrapConstrntMV(MVCrvs[i],
						     MVCrvs[(i + 1) % n],
						     MVCrvs[(i + 2) % n],
						     MVNrmls[(i + 1) % n],
						     Orient,
						     &MVCnstrns[i + n]);
	Constraints[i] = MVAR_CNSTRNT_ZERO;
	Constraints[i + n] = MVAR_CNSTRNT_POSITIVE;
    }
    
    Pts = MvarMVsZeros(MVCnstrns, Constraints, n + (Orient ? n : 0),
		       SubdivTol, NumerTol);

    for (i = 0; i < n; i++) {
	MvarMVFree(MVCrvs[i]);
	MvarMVFree(MVNrmls[i]);
        MvarMVFree(MVCnstrns[i]);
	if (Orient)
	    MvarMVFree(MVCnstrns[i + n]);
    }

    return MvarPostProcessRayTrapPts(n, Pts, Crvs, TMin, TMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Build the ray constraint of a ray coming from Mv1 bouncing of Mv2 to Mv3 *
* as an angular constraint on the coming (to Mv2) and leaving (Mv2) angles   *
* with respect to the normal of Mv2.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mv1, Mv2, Mv3: Three consecutive curves to formulate the constraint for. *
*   Mv2Nrml:	   The normal field of Mv2.				     *
*   Orient:        If TRUE, synthesize an orientation setting constraint     *
*		   into MVOrient.					     *
*   MVOrient:      Optional orientation constraint (if Orient is TRUE).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:  A zero set constraint for the angular equality.         *
*****************************************************************************/
static MvarMVStruct *MvarBuildOneRayTrapConstrntMV(MvarMVStruct *Mv1,
						   MvarMVStruct *Mv2,
						   MvarMVStruct *Mv3,
						   MvarMVStruct *Mv2Nrml,
						   int Orient,
						   MvarMVStruct **MVOrient)
{
    MvarMVStruct
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

    if (Orient) {
	*MVOrient = MvarMVMult(Ang12, Ang32);
    }

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
*   Crvs:          Input ray-trap curves.                                    *
*   TMin, TMax:    Vector of size n of all domains of all curves.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Filtered points.                                        *
*****************************************************************************/
static MvarPtStruct *MvarPostProcessRayTrapPts(int n,
					       MvarPtStruct *Pts,
					       const CagdCrvStruct *Crvs,
					       const CagdRType *TMin,
					       const CagdRType *TMax)
{
    int i;
    MvarPtStruct *Pt;

    /* Recover the solution's domain as we transformed to (0, 1) domains. */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	for (i = 0; i < n; i++)
	    Pt -> Pt[i] = Pt -> Pt[i] * (TMax[i] - TMin[i]) + TMin[i];
    }

#ifdef MVAR_RAY_TRAP_FILTER_REVERSED
    {
	MvarPtStruct *PtNext,
	    *Pt = Pts;
	CagdPType Pos[MVAR_RAY_TRAP_MAX_CRVS];
	CagdVType Nrml[MVAR_RAY_TRAP_MAX_CRVS];
	const CagdCrvStruct *Crv;

	Pts = NULL;
	while (Pt != NULL) {
	    PtNext = Pt -> Pnext;

	    Pt -> Pnext = NULL;

	    /* Evaluate each points in case we are in wrong orientation. */
	    for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++) {
		CagdRType
		    *R = CagdCrvEval(Crv, Pt -> Pt[i]);
		CagdVecStruct
		    *V = CagdCrvNormal(Crv, Pt -> Pt[i], FALSE);

		CagdCoerceToE3(Pos[i], &R, -1, Crv -> PType);
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
