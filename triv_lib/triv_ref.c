/******************************************************************************
* Triv_Ref.c - Refinements for Tri-Variate Bsplines (and Bezier).	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trivariate, refines it at the given n knots as defined by the      M
* vector t.								     M
*   If Replace is TRUE, the values replace the current knot vector.	     M
*   Returns pointer to refined TV (Note a Bezier trivariate will be          M
* converted into a Bspline trivariate).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to refine according to t in direction Dir.          M
*   Dir:      Direction of refinement. Either U or V or W.                   M
*   Replace:  If TRUE t is a knot vector exactly in the length of the knot   M
*             vector in direction Dir in TV and t simply replaces that knot  M
*	      vector. If FALSE, the knot vector in direction Dir in TV is    M
*	      refined by adding all the knots in t.			     M
*   t:        Knot vector to refine/replace the knot vector of TV in         M
*	      direction Dir.						     M
*   n:        Length of vector t.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    The refined trivariate. Always a Bspline trivariate.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVRefineAtParams, trivariates                                        M
*****************************************************************************/
TrivTVStruct *TrivTVRefineAtParams(const TrivTVStruct *TV,
				   TrivTVDirType Dir,
				   CagdBType Replace,
				   CagdRType *t,
				   int n)
{
    TrivTVStruct *BspTV, *TTV;

    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
    	    BspTV = TrivCnvrtBzr2BspTV(TV);
	    TTV = TrivBspTVKnotInsertNDiff(BspTV, Dir, Replace, t, n);
	    TrivTVFree(BspTV);
	    return TTV;
	case TRIV_TVBSPLINE_TYPE:
	    return TrivBspTVKnotInsertNDiff(TV, Dir, Replace, t, n);
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline trivariate, inserts n knots with different values as       M
* defined by t.								     M
*   If, however, Replace is TRUE, the knot are simply replacing the current  M
* knot vector in the prescribed direction.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to refine according to t in direction Dir.          M
*   Dir:      Direction of refinement. Either U or V or W.                   M
*   Replace:  If TRUE t is a knot vector exaclt in the length of the knot    M
*             vector in direction Dir in TV and t simply replaces than knot  M
*	      vector. If FALSE, the knot vector in direction Dir in TV is    M
*	      refined by adding all the knots in t.			     M
*   t:        Knot vector to refine/replace the knot vector of TV in         M
*	      direction Dir.						     M
*   n:        Length of vector t.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    The refined trivariate.  A Bspline trivariate.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVKnotInsertNDiff, trivariates                                    M
*****************************************************************************/
TrivTVStruct *TrivBspTVKnotInsertNDiff(const TrivTVStruct *TV,
				       TrivTVDirType Dir,
				       int Replace,
				       const CagdRType *t,
				       int n)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, ODim,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	UOrder = TV -> UOrder,
	VOrder = TV -> VOrder,
	WOrder = TV -> WOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
	*RefTV = NULL;

    if (Replace) {
	for (i = 1; i < n; i++)
	    if (t[i] < t[i - 1])
		TRIV_FATAL_ERROR(TRIV_ERR_KNOT_NOT_ORDERED);

    	switch (Dir) {
	    case TRIV_CONST_U_DIR:
		if (TV -> UOrder + TV -> ULength != n)
		    TRIV_FATAL_ERROR(TRIV_ERR_NUM_KNOT_MISMATCH);

		RefTV = TrivTVCopy(TV);
		for (i = 0; i < n; i++)
		    RefTV -> UKnotVector[i] = *t++;
		break;
	    case TRIV_CONST_V_DIR:
		if (TV -> VOrder + TV -> VLength != n)
		    TRIV_FATAL_ERROR(TRIV_ERR_NUM_KNOT_MISMATCH);

		RefTV = TrivTVCopy(TV);
		for (i = 0; i < n; i++)
		    RefTV -> VKnotVector[i] = *t++;
		break;
	    case TRIV_CONST_W_DIR:
		if (TV -> WOrder + TV -> WLength != n)
		    TRIV_FATAL_ERROR(TRIV_ERR_NUM_KNOT_MISMATCH);

		RefTV = TrivTVCopy(TV);
		for (i = 0; i < n; i++)
		    RefTV -> WKnotVector[i] = *t++;
		break;
	    default:
		TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
		break;
	}
    }
    else if (n == 0) {
	RefTV = TrivTVCopy(TV);
    }
    else {
	int j, LengthKVt, RULength, RVLength, RWLength;
	BspKnotAlphaCoeffStruct *A;
	CagdRType *MergedKVt,
	    *UKnotVector = TV -> UKnotVector,
	    *VKnotVector = TV -> VKnotVector,
	    *WKnotVector = TV -> WKnotVector;

	switch (Dir) {
	    case TRIV_CONST_U_DIR:
		/* Compute the Alpha refinement matrix. */
		MergedKVt = BspKnotMergeTwo(UKnotVector, ULength + UOrder,
					    t, n, 0, &LengthKVt);
		A = BspKnotEvalAlphaCoef(UOrder, UKnotVector, ULength,
					 MergedKVt, LengthKVt - UOrder,
					 TV -> UPeriodic);

	        RefTV = TrivBspTVNew(ULength + n, VLength, WLength,
				     UOrder, VOrder, WOrder, TV -> PType);
		IritFree(RefTV -> UKnotVector);
		RefTV -> UKnotVector = MergedKVt;
		BspKnotCopy(RefTV -> VKnotVector, TV -> VKnotVector,
			    TV -> VLength + TV -> VOrder);
		BspKnotCopy(RefTV -> WKnotVector, TV -> WKnotVector,
			    TV -> WLength + TV -> WOrder);

		RULength = RefTV -> ULength;

		/* Update the control mesh */
		for (ODim = 0; ODim < VLength * WLength; ODim++) {
		    int OIndex = ODim * TRIV_NEXT_V(TV),
			ROIndex = ODim * TRIV_NEXT_V(RefTV);

		    for (j = IsNotRational; j <= MaxCoord; j++) {
			CagdRType
			    *ROnePts = &RefTV -> Points[j][ROIndex],
			    *OnePts = &TV -> Points[j][OIndex];

			BspKnotAlphaLoopBlendStep(A, 0, RULength,
						  OnePts, TRIV_NEXT_U(TV), -1,
						  ROnePts, TRIV_NEXT_U(RefTV));
		    }
		}

		BspKnotFreeAlphaCoef(A);
		break;
	    case TRIV_CONST_V_DIR:
		/* Compute the Alpha refinement matrix. */
		MergedKVt = BspKnotMergeTwo(VKnotVector, VLength + VOrder,
					    t, n, 0, &LengthKVt);
		A = BspKnotEvalAlphaCoef(VOrder, VKnotVector, VLength,
					 MergedKVt, LengthKVt - VOrder,
					 TV -> VPeriodic);

	        RefTV = TrivBspTVNew(ULength, VLength + n, WLength,
				     UOrder, VOrder, WOrder, TV -> PType);
		BspKnotCopy(RefTV -> UKnotVector, TV -> UKnotVector,
			    TV -> ULength + TV -> UOrder);
		IritFree(RefTV -> VKnotVector);
		RefTV -> VKnotVector = MergedKVt;
		BspKnotCopy(RefTV -> WKnotVector, TV -> WKnotVector,
			    TV -> WLength + TV -> WOrder);

		RULength = RefTV -> ULength;
		RVLength = RefTV -> VLength;

		/* Update the control mesh */
		for (ODim = 0; ODim < ULength * WLength; ODim++) {
		    int OIndex = (ODim / ULength) * TRIV_NEXT_W(TV) +
				 (ODim % ULength) * TRIV_NEXT_U(TV),
		        ROIndex = (ODim / RULength) * TRIV_NEXT_W(RefTV) +
				  (ODim % RULength) * TRIV_NEXT_U(RefTV);

		    for (j = IsNotRational; j <= MaxCoord; j++) {
			CagdRType
			    *ROnePts = &RefTV -> Points[j][ROIndex],
			    *OnePts = &TV -> Points[j][OIndex];

			BspKnotAlphaLoopBlendStep(A, 0, RVLength,
						  OnePts, TRIV_NEXT_V(TV), -1,
						  ROnePts, TRIV_NEXT_V(RefTV));
		    }
		}

		BspKnotFreeAlphaCoef(A);
		break;
	    case TRIV_CONST_W_DIR:
		/* Compute the Alpha refinement matrix. */
		MergedKVt = BspKnotMergeTwo(WKnotVector, WLength + WOrder,
					    t, n, 0, &LengthKVt);
		A = BspKnotEvalAlphaCoef(WOrder, WKnotVector, WLength,
					 MergedKVt, LengthKVt - WOrder,
					 TV -> WPeriodic);

	        RefTV = TrivBspTVNew(ULength, VLength, WLength + n,
				     UOrder, VOrder, WOrder, TV -> PType);
		BspKnotCopy(RefTV -> UKnotVector, TV -> UKnotVector,
			    TV -> ULength + TV -> UOrder);
		BspKnotCopy(RefTV -> VKnotVector, TV -> VKnotVector,
			    TV -> VLength + TV -> VOrder);
		IritFree(RefTV -> WKnotVector);
		RefTV -> WKnotVector = MergedKVt;

		RWLength = RefTV -> WLength;

		/* Update the control mesh */
		for (ODim = 0; ODim < ULength * VLength; ODim++) {
		    int OIndex = ODim * TRIV_NEXT_U(TV),
			ROIndex = ODim * TRIV_NEXT_U(RefTV);

		    for (j = IsNotRational; j <= MaxCoord; j++) {
			CagdRType
			    *ROnePts = &RefTV -> Points[j][ROIndex],
			    *OnePts = &TV -> Points[j][OIndex];

			BspKnotAlphaLoopBlendStep(A, 0, RWLength,
						  OnePts, TRIV_NEXT_W(TV), -1,
						  ROnePts, TRIV_NEXT_W(RefTV));
		    }
		}

		BspKnotFreeAlphaCoef(A);
		break;
	    default:
		TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
		break;
	}
    }

    BspKnotMakeRobustKV(RefTV -> UKnotVector,
			RefTV -> UOrder + RefTV -> ULength);
    BspKnotMakeRobustKV(RefTV -> VKnotVector,
			RefTV -> VOrder + RefTV -> VLength);
    BspKnotMakeRobustKV(RefTV -> WKnotVector,
			RefTV -> WOrder + RefTV -> WLength);

    return RefTV;
}
