/******************************************************************************
* MVar_Ref.c - Refinements for Multi-variate Bsplines (and Bezier).	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 97.					      *
******************************************************************************/

#include <string.h>
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, refines it at the given n knots as defined by the   M
* vector t.								     M
*   If Replace is TRUE, the values replace the current knot vector.	     M
*   Returns pointer to refined MV (Note a Bezier multi-variate will be       M
* converted into a Bspline multi-variate).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-variate to refine according to t in direction Dir.       M
*   Dir:      Direction of refinement. Either U or V or W.                   M
*   Replace:  If TRUE t is a knot vector exaclt in the length of the knot    M
*             vector in direction Dir in MV and t simply replaces than knot  M
*	      vector. If FALSE, the knot vector in direction Dir in MV is    M
*	      refined by adding all the knots in t.			     M
*   t:        Knot vector to refine/replace the knot vector of MV in         M
*	      direction Dir.						     M
*   n:        Length of vector t.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    The refined multi-variate. Always a Bspline.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVRefineAtParams, multi-variates                                     M
*****************************************************************************/
MvarMVStruct *MvarMVRefineAtParams(const MvarMVStruct *MV,
				   MvarMVDirType Dir,
				   CagdBType Replace,
				   CagdRType *t,
				   int n)
{
    MvarMVStruct *BspMV, *TMV;

    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
    	    BspMV = MvarCnvrtBzr2BspMV(MV);
	    TMV = MvarBspMVKnotInsertNDiff(BspMV, Dir, Replace, t, n);
	    MvarMVFree(BspMV);
	    return TMV;
	case MVAR_BSPLINE_TYPE:
	    return MvarBspMVKnotInsertNDiff(MV, Dir, Replace, t, n);
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline multi-variate, inserts n knots with different values as    M
* defined by t.								     M
*   If, however, Replace is TRUE, the knot are simply replacing the current  M
* knot vector in the prescribed direction.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-variate to refine according to t in direction Dir.       M
*   Dir:      Direction of refinement. Either U or V or W.                   M
*   Replace:  If TRUE t is a knot vector exactly in the length of the knot   M
*             vector in direction Dir in MV and t simply replaces that knot  M
*	      vector. If FALSE, the knot vector in direction Dir in MV is    M
*	      refined by adding all the knots in t.			     M
*   t:        Knot vector to refine/replace the knot vector of MV in         M
*	      direction Dir.						     M
*   n:        Length of vector t.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    The refined multi-variate.  A Bspline multi-variate.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVKnotInsertNDiff, multi-variates                                 M
*****************************************************************************/
MvarMVStruct *MvarBspMVKnotInsertNDiff(const MvarMVStruct *MV,
				       MvarMVDirType Dir,
				       int Replace,
				       CagdRType *t,
				       int n)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i,
	Length = MV -> Lengths[Dir],
	Order = MV -> Orders[Dir],
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *CpMV,
	*RefMV = NULL;

    if (Dir < 0 || Dir >= MV -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_DIR_NOT_VALID);
	return NULL;
    }

    if (Replace) {
	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1]) {
		MVAR_FATAL_ERROR(MVAR_ERR_KNOT_NOT_ORDERED);
		return NULL;
	    }

	if (Order + Length != n) {
	    MVAR_FATAL_ERROR(MVAR_ERR_NUM_KNOT_MISMATCH);
	    return NULL;
	}

	RefMV = MvarMVCopy(MV);
	for (i = 0; i < n; i++)
	    RefMV -> KnotVectors[Dir][i] = *t++;
    }
    else if (n == 0) {
	RefMV = MvarMVCopy(MV);
    }
    else {
        int j, LengthKVt, RLength, RIndex,
	    *Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
	BspKnotAlphaCoeffStruct *A;
	CagdRType *MergedKVt,
	    *KnotVector = MV -> KnotVectors[Dir];

	if (MvarBspMVIsPeriodic(MV)) {
	    MV = CpMV = MvarCnvrtPeriodic2FloatMV(MV);
	    Length = MV -> Lengths[Dir];
	    KnotVector = MV -> KnotVectors[Dir];
	}
	else
	    CpMV = NULL;

	MV -> Lengths[Dir] += n;
	RefMV = MvarBspMVNew(MV -> Dim, MV -> Lengths, MV -> Orders,
			     MV -> PType);
	MV -> Lengths[Dir] -= n;

	/* Compute the Alpha refinement matrix. */
	MergedKVt = BspKnotMergeTwo(KnotVector, Length + Order,
				    t, n, 0, &LengthKVt);
	A = BspKnotEvalAlphaCoef(Order, KnotVector, Length,
				 MergedKVt, LengthKVt - Order,
				 MV -> Periodic[Dir]);

	for (i = 0; i < MV -> Dim; i++) {
	    if (i == Dir) {
		IritFree(RefMV -> KnotVectors[i]);
		RefMV -> KnotVectors[i] = MergedKVt;
	    }
	    else {
		CAGD_GEN_COPY(RefMV -> KnotVectors[i], MV -> KnotVectors[i],
		     sizeof(CagdRType) * (MV -> Lengths[i] + MV -> Orders[i]));
	    }
	}
	RLength = RefMV -> Lengths[Dir];

	/* Update the control mesh. */
	IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
	RIndex = 0;
	do {
	    int Index = MvarGetPointsMeshIndices(MV, Indices);

	    for (j = IsNotRational; j <= MaxCoord; j++) {
	        int RStep = MVAR_NEXT_DIM(RefMV, Dir),
		    Step = MVAR_NEXT_DIM(MV, Dir);
		CagdRType
		    *RPts = &RefMV -> Points[j][RIndex],
		    *Pts = &MV -> Points[j][Index];

		BspKnotAlphaLoopBlendStep(A, 0, RLength, Pts, Step,
					  -1, RPts, RStep);
	    }
	}
	while (MVAR_INC_SKIP_MESH_INDICES(RefMV, Indices, Dir, RIndex));

	if (CpMV != NULL)
	    MvarMVFree(CpMV);

	IritFree(Indices);

	BspKnotFreeAlphaCoef(A);
    }

    for (i = 0; i < RefMV -> Dim; i++) {
	BspKnotMakeRobustKV(RefMV -> KnotVectors[i],
			    RefMV -> Orders[i] + RefMV -> Lengths[i]);
    }

    return RefMV;
}
