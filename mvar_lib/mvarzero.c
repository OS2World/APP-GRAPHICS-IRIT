/******************************************************************************
* MvarZero.c - tools to compute zero sets of multivariates.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 97.					      *
******************************************************************************/

#include "mvar_loc.h"
#include "misc_lib.h"
#include "geom_lib.h"

typedef struct MvarMVParamDomainStruct {
    CagdRType Min, Max;					      /* The domain. */
} MvarMVParamDomainStruct;

IRIT_GLOBAL_DATA int
    _MVGlblSameSpace = FALSE,
    _MVGlblZeroApplyDomainReduction = TRUE,
    _MVGlblZeroApplyGradPreconditioning = FALSE,
    _MVGlblZeroApplyParallelHyperPlaneTest = FALSE,
    _MVGlblZeroApplyNormalConeTest = TRUE;
IRIT_GLOBAL_DATA CagdRType
    _MVGlblZeroParamPerturb = 0.0;
IRIT_STATIC_DATA MvarMVsZerosSubdivCallBackFunc
    _MVGlblZeroSubdivCallBackFunc = NULL;

#ifdef DEBUG_DUMP_DOMAINS
IRIT_STATIC_DATA FILE
    *GlblDumpDomainsFile = NULL;
#endif /* DEBUG_DUMP_DOMAINS */

static void MvarGetSubdivParamDomains(const MvarMVStruct *MV,
				      CagdRType *Min,
				      CagdRType *Max,
				      int Dir);
static int MvarMVsReduceMvsDomains(MvarMVStruct **MVs,
				   int NumOfMVs,
				   int Dir,
				   CagdRType SubdivTol,
				   CagdRType *TMin,
				   CagdRType *TMax);
static MvarMVStruct **MvarMVsOrthogonalizeGrads(MvarMVStruct **MVs,
						int NumOfMVs);
static void MVarMVHyperPlanesBound(const MvarMVStruct *MV,
				   CagdRType *Coeffs,
				   CagdRType *Coeff0Max,
				   CagdRType *Coeff0Min);
static int MVarMVHyperPlanesTestForSol(MvarMVStruct * const *MVs,
				       int NumOfZeroMVs);

/* #define MVAR_DEBUG_DEPTH */
#ifdef MVAR_DEBUG_DEPTH
#define MVAR_DEBUG_MAX_DEPTH	200
IRIT_STATIC_DATA int
    MvarSubdivLevel[MVAR_DEBUG_MAX_DEPTH] = { 0 };
#endif /* MVAR_DEBUG_DEPTH */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of the normal cone tests inside the multivariate   M
* subdivisions' zero set solver.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalConeTest:   New setting for normal cone testing usage.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting for normal cone testing usage.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosDomainReduction,				     M
*   MvarMVsZerosGradPreconditioning, MvarMVsZerosSetCallBackFunc	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosNormalConeTest                                               M
*****************************************************************************/
int MvarMVsZerosNormalConeTest(int NormalConeTest)
{
    int OldVal = _MVGlblZeroApplyNormalConeTest;

    _MVGlblZeroApplyNormalConeTest = NormalConeTest;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of gradient preconditiong option - application of  M
* and orthogonalization process over the gradients in multivariate	     M
* subdivisions' zero set solver.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   GradPreconditioning:   New setting for the gradient orthogonalization.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting of gradient orthogonalization.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosNormalConeTest, MvarMVsZerosDomainReduction,   M
*   MvarMVsZerosSetCallBackFunc						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosGradPreconditioning                                          M
*****************************************************************************/
int MvarMVsZerosGradPreconditioning(int GradPreconditioning)
{
    int OldVal = _MVGlblZeroApplyGradPreconditioning;

    _MVGlblZeroApplyGradPreconditioning = GradPreconditioning;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of the domain reduction option - Bezier (and       M
* (B-spline) clipping in multivariate subdivisions' zero set solver.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   DomainReduction:   New setting for the domain reduction option.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting for normal cone testing usage.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosNormalConeTest,				     M
*   MvarMVsZerosGradPreconditioning, MvarMVsZerosSetCallBackFunc             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosDomainReduction                                              M
*****************************************************************************/
int MvarMVsZerosDomainReduction(int DomainReduction)
{
    int OldVal = _MVGlblZeroApplyDomainReduction;

    _MVGlblZeroApplyDomainReduction = DomainReduction;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of the parallel plane termination criteria         M
* in multivariate subdivisions' zero set solver.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   ParallelHPlaneTest:   New setting for the domain reduction option.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting for normal cone testing usage.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosNormalConeTest,				     M
*   MvarMVsZerosGradPreconditioning, MvarMVsZerosDomainReduction,            M
*   MvarMVsZerosSetCallBackFunc						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosParallelHyperPlaneTest                                       M
*****************************************************************************/
int MvarMVsZerosParallelHyperPlaneTest(int ParallelHPlaneTest)
{
    int OldVal = _MVGlblZeroApplyParallelHyperPlaneTest;

    _MVGlblZeroApplyParallelHyperPlaneTest = ParallelHPlaneTest;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of a call back function that is invoked at every   M
* node of the subdivision tree process.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   SubdivCallBackFunc:   Call back function to use in the MV zeros'	     M
*			  subdivision stage.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVsZerosSubdivCallBackFunc:       Old setting.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosDomainReduction,				     M
*   MvarMVsZerosGradPreconditioning, MvarMVsZerosNormalConeTest		     M
*   MvarMVsZerosParallelHyperPlaneTest		                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosSetCallBackFunc                                              M
*****************************************************************************/
MvarMVsZerosSubdivCallBackFunc MvarMVsZerosSetCallBackFunc(
			    MvarMVsZerosSubdivCallBackFunc SubdivCallBackFunc)
{
    MvarMVsZerosSubdivCallBackFunc
	OldVal = _MVGlblZeroSubdivCallBackFunc;

    _MVGlblZeroSubdivCallBackFunc = SubdivCallBackFunc;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Make sure all given MVs are in the same function space.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:          Vector of multivariate constraints.                        M
*   NumOfMVs:     Size of the MVs vector.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if in same function space, FALSE otherwise.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsSameSpace                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosSameSpace                                                    M
*****************************************************************************/
CagdBType MvarMVsZerosSameSpace(MvarMVStruct **MVs, int NumOfMVs)
{
    int l;

    for (l = 1; l < NumOfMVs; l++) {
        if (!MvarMVsSameSpace(MVs[0], MVs[l], IRIT_EPS))
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the simultaneous solution of the given set of NumOfMVs          M
* constraints.  A constraints can be equality or ineqaulity as prescribed    M
* by the Constraints vector.  Only equality constraints are employed in the  M
* numerical improvement stage.						     M
*   All multivariates are assumed to be scalar and be in the same parametric M
* domain size and dimension.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:          Vector of multivariate constraints.                        M
*   Constraints:  Either an equality or an inequality type of constraint.    M
*   NumOfMVs:     Size of the MVs and Constraints vector.                    M
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        M
*		  measured in the parametric space of the multivariates.     M
*   NumericTol:   Numeric tolerance of the numeric stage.  The numeric stage M
*		  is employed only if IRIT_FABS(NumericTol) < SubdivTol.     M
*		  If NumericTol is negative, points that fail to improve     M
*		  numerically are purged away.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  M
*		      points will be the same as the dimensions of all MVs.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZerosNormalConeTest, MvarMVsZerosDomainReduction,                 M
*   MvarMVsZerosVerifier						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZeros                                                             M
*****************************************************************************/
MvarPtStruct *MvarMVsZeros(MvarMVStruct **MVs,
			   MvarConstraintType *Constraints,
			   int NumOfMVs,
			   CagdRType SubdivTol,
			   CagdRType NumericTol)
{
    CagdBType
	HasBezConst = FALSE,
	HasBspConst = FALSE;
    int i, l, NumOfZeroMVs, NumOfZeroSubdivMVs, CanApplyNormalConeTest;
    MvarPtStruct *ZeroSet;
    MvarMVStruct **LclMVs;
    MvarMVGradientStruct
	**Grads = NULL;

    /* Make sure we are reasonable. */
    SubdivTol = IRIT_MAX(SubdivTol, 1e-8);

#   ifdef MVAR_DEBUG_DUMP_INPUT
    for (l = 0; l < NumOfMVs; l++) {
        printf("MVAR SOLVER INPUT *******************\nMV %d: %s type, ", l,
		MVAR_IS_BEZIER_MV(MVs[l]) ? "Bzr" : (MVAR_IS_BSPLINE_MV(MVs[l]) ? "Bsp" : "Other"));
	printf("orders: ");
	for (i = 0; i < MVs[l] -> Dim; i++)
	    printf("%d, ", MVs[l] -> Orders[i]);
	if (MVAR_IS_BSPLINE_MV(MVs[l])) {
	    printf("lengths: ");
	    for (i = 0; i < MVs[l] -> Dim; i++)
	        printf("%d, ", MVs[l] -> Lengths[i]);
	}
	printf("\n");
    }

    for (l = 0; l < NumOfMVs; l++) {
        fprintf(stderr, "MV %d *******************************\n", l);
	MvarDbg(MVs[l]);
    }
#   endif /* MVAR_DEBUG_DUMP_INPUT */

    /* Make sure the parametric domain is identical at all multivariates. */
    for (l = 0; l < MVs[0] -> Dim; l++) {
	CagdRType Min, Max;

	MvarMVDomain(MVs[0], &Min, &Max, l);

	for (i = 1; i < NumOfMVs; i++) {
	    CagdRType Min2, Max2;

	    MvarMVDomain(MVs[i], &Min2, &Max2, l);
	    if (!IRIT_APX_EQ(Min, Min2) || !IRIT_APX_EQ(Max, Max2)) {
		MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
		return NULL;
	    }
	}
    }

    /* Make sure we have domains (KVs) on all multivariates and all         */
    /* multivariates are polynomials,  either all Bezier or all B-spline.   */
    LclMVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct) * NumOfMVs);
    for (i = 0; i < NumOfMVs; i++) {
	LclMVs[i] = MvarMVCopy(MVs[i]);
	if (MVAR_IS_BEZIER_MV(LclMVs[i])) {
	    MvarMVAuxDomainSlotReset(LclMVs[i]);
	    HasBezConst = TRUE;
	} 
	else if (MVAR_IS_BSPLINE_MV(MVs[i])) {
	    HasBspConst = TRUE;
	}
 
	if (MVAR_IS_RATIONAL_MV(LclMVs[i])) {
	    /* Convert P1 point type to E1, in local copy. */
#ifndef MVAR_MALLOC_STRUCT_ONCE
	    IritFree(LclMVs[i] -> Points[0]);
#endif /* MVAR_MALLOC_STRUCT_ONCE */
	    LclMVs[i] -> Points[0] = NULL;
	    LclMVs[i] -> PType = MVAR_PT_E1_TYPE;
	}
    }

    if (HasBspConst && HasBezConst) {
        MVAR_FATAL_ERROR(MVAR_ERR_CANNT_MIX_BSP_BEZ);
        return NULL;
    }

    /* Preconditionally scale all constraints to a 'reasonable' height. Note */
    /* such scale should not affect the zeros of the set of constraints.     */
    for (i = 0; i < NumOfMVs; i++) {
	int Length = MVAR_CTL_MESH_LENGTH(LclMVs[i]);
	CagdRType Scale, *R,
	    *Pts = LclMVs[i] -> Points[1],
	    Max = 0.0;

	for (R = &Pts[Length]; R-- != Pts; )
	    Max = IRIT_MAX(Max, IRIT_FABS(*R));

	/* For the unlikely case of a constraint that is identically zero. */
	if (Max == 0.0) {
	    /* Purge away this identically zero constraint. */
	    MvarMVFree(LclMVs[i]);
	    LclMVs[i] = LclMVs[NumOfMVs - 1];
	    Constraints[i] = Constraints[NumOfMVs - 1];
	    NumOfMVs--;
	    continue;
	}

	Scale = MVAR_ZERO_PRECOND_SCALE / Max;

	for (R = &Pts[Length]; R-- != Pts; )
	    *R *= Scale;
    }

    /* For the unlikely case of all constraints identically zero. */
    if (NumOfMVs == 0)
	return NULL;

    /* Make sure all zero constraints are first, zero-subdiv are second,     */
    /* and count how many.						     */
    for (i = 0; i < NumOfMVs; i++) {
        if (Constraints[i] != MVAR_CNSTRNT_ZERO) {
	    int j;

	    for (j = i + 1; j < NumOfMVs; j++) {
	        if (Constraints[j] == MVAR_CNSTRNT_ZERO) {
		    /* Swap constraints i and j. */
		    IRIT_SWAP(MvarConstraintType, Constraints[i],
					          Constraints[j]);
		    IRIT_SWAP(MvarMVStruct *, LclMVs[i], LclMVs[j]);
		    break;
		}
	    }
	    if (j >= NumOfMVs)
	        break;
	}
	else if (MVAR_NUM_OF_MV_COORD(LclMVs[i]) != 1) {
	    /* Make sure all zero MV constraints are scalar fields. */
	    MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
	    return NULL;
	}
    }
    NumOfZeroMVs = i;

    for (i = NumOfZeroMVs; i < NumOfMVs; i++) {
        if (Constraints[i] != MVAR_CNSTRNT_ZERO_SUBDIV) {
	    int j;

	    for (j = i + 1; j < NumOfMVs; j++) {
	        if (Constraints[j] == MVAR_CNSTRNT_ZERO_SUBDIV) {
		    /* Swap constraints i and j. */
		    IRIT_SWAP(MvarConstraintType, Constraints[i],
						  Constraints[j]);
		    IRIT_SWAP(MvarMVStruct *, LclMVs[i], LclMVs[j]);
		    break;
		}
	    }
	    if (j >= NumOfMVs)
	        break;
	}
	else if (MVAR_NUM_OF_MV_COORD(LclMVs[i]) != 1) {
	    /* Make sure all zero MV constraints are scalar fields. */
	    MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
	    return NULL;
	}
    }
    NumOfZeroSubdivMVs = i;

    /* Should we employ the cone test for single solutions? */
    CanApplyNormalConeTest = _MVGlblZeroApplyNormalConeTest &&
			     NumOfZeroMVs == LclMVs[0] -> Dim;

    /* See if gradients are to be stored along with the functions. A scalar */
    /* function f of dimension d will be converted to a 1+d vector function,*/
    /* holding original function f and the d coefficients of the gradient.  */
    /*   This 1+d storage will be used for normal cone tests, etc.          */
    if (_MVGlblZeroApplyParallelHyperPlaneTest) {
	Grads = (MvarMVGradientStruct **)
	    IritMalloc(NumOfMVs * sizeof(MvarMVGradientStruct *));

        for (i = 0; i < NumOfMVs; i++) {
	    int Size = sizeof(CagdRType) * MVAR_CTL_MESH_LENGTH(LclMVs[i]);
	    MvarMVGradientStruct
	        *Grad = MvarMVPrepGradient(LclMVs[i], TRUE);

	    assert(!MVAR_IS_RATIONAL_MV(LclMVs[i]));

	    /*   Move the gradient vector to this LclMVs[i], in place.      */
	    /*   The LclMVs[i] was a scalar E1 function and now it will     */
	    /* become E(1+Dim).					            */
	    if (LclMVs[i] -> Dim > MVAR_MAX_PT_COORD) {
	        MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);   
		return NULL;
	    }

	    for (l = 0; l < LclMVs[i] -> Dim; l++) {
	        LclMVs[i] -> Points[l + 2] = (CagdRType *) IritMalloc(Size);
		IRIT_GEN_COPY(LclMVs[i] -> Points[l + 2],
			 Grad -> MVGrad -> Points[l + 1], Size);

		LclMVs[i] -> PType = MVAR_MAKE_PT_TYPE(FALSE,
						       LclMVs[i] -> Dim + 1);
	    }

	    /* Keep for the numeric marching step. */
	    Grads[i] = Grad;
	}
    }

#   ifdef MVAR_DEBUG_DEPTH
        IRIT_ZAP_MEM(MvarSubdivLevel, sizeof(int) * MVAR_DEBUG_MAX_DEPTH);
#   endif /* MVAR_DEBUG_DEPTH */

    _MVGlblZeroParamPerturb = SubdivTol * MVAR_ZERO_PARAM_REL_PERTURB;
    _MVGlblSameSpace = MvarMVsZerosSameSpace(LclMVs, NumOfMVs);

    /* Do the subdivision stage up to SubdivTol tolerance. */
#   ifdef DEBUG_DUMP_DOMAINS
        GlblDumpDomainsFile = fopen("MvarZeroDomains.itd", "w");
#   endif /* DEBUG_DUMP_DOMAINS */

    ZeroSet = MvarZeroMVsSubdiv(LclMVs, Constraints, NumOfMVs,
				NumOfZeroSubdivMVs,
				CanApplyNormalConeTest, SubdivTol, 0);

#   ifdef DEBUG_DUMP_DOMAINS
        fclose(GlblDumpDomainsFile);
#   endif /* DEBUG_DUMP_DOMAINS */

#   ifdef MVAR_DEBUG_DEPTH
    {
        int i,
	    Sum = 0;

	for (i = 0; i < MVAR_DEBUG_MAX_DEPTH; i++) {
	    printf("At level %d :: %d calls\n", i, MvarSubdivLevel[i]);
	    Sum += MvarSubdivLevel[i];
	    if (MvarSubdivLevel[i] == 0)
	        break;
	}
	printf("Total subdivisions: %d\n", Sum);
    }
#   endif /* MVAR_DEBUG_DEPTH */

    if (_MVGlblZeroApplyParallelHyperPlaneTest) {
        for (i = 0; i < NumOfMVs; i++) {
	    int NumOfCoord = MVAR_NUM_OF_MV_COORD(LclMVs[i]);

	    /* Restore scalar multivariates, dropping gradient. */
	    for (l = 2; l <= NumOfCoord; l++) {
	        IritFree(LclMVs[i] -> Points[l]);
		LclMVs[i] -> Points[l] = NULL;
		LclMVs[i] -> PType = MVAR_PT_E1_TYPE;
	    }
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMvarSubdivZeroSet, FALSE) {
	    MvarPtStruct *Pt;

	    printf("    [POINTLIST %d\n", CagdListLength(ZeroSet));
	    for (Pt = ZeroSet; Pt != NULL; Pt = Pt -> Pnext) {
	        printf("\t[[Subdivided %d] ",
		       AttrGetIntAttrib(Pt -> Attr, "SingleSol"));
		for (i = 0; i < Pt -> Dim; i++)
		    printf("%f  ", Pt -> Pt[i]);
		printf("]\n");
	    }
	    printf("    ]\n");
	}
    }
#   endif /* DEBUG */

    /* Do the numeric improvement stage up to NumericTol(Scl) tolerance. */
    if (IRIT_FABS(NumericTol) < SubdivTol) {
        /* Prepare the gradients for faster evaluation. */
        for (i = 0; i < NumOfZeroMVs; i++) {
	    AttrSetPtrAttrib(&LclMVs[i] -> Attr, "Gradient",
			     Grads != NULL ? Grads[i]
			                   : MvarMVPrepGradient(LclMVs[i],
								TRUE));
	}

	ZeroSet = MvarEqnsZeroNumeric(ZeroSet, NULL, LclMVs,
				      NumOfZeroMVs, NumericTol);
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMvarFilteringZeroSet, FALSE) {
	    MvarPtStruct *Pt;

	    for (Pt = ZeroSet; Pt != NULL; Pt = Pt -> Pnext) {
		IRIT_INFO_MSG_PRINTF("\n\tUnfiltered (Err = %.15f) = ",
				     AttrGetRealAttrib(Pt -> Attr, "RngError"));
		for (i = 0; i < Pt -> Dim; i++)
		    IRIT_INFO_MSG_PRINTF("%.14f  ", Pt -> Pt[i]);
	    }
	    IRIT_INFO_MSG("\nDone\n"); 
	}
    }
#   endif /* DEBUG */

    /* Filter out points that fail on the inequality constraints or          */
    /* identical points in the zero set.			 	     */
    if (IRIT_FABS(NumericTol) < SubdivTol) {
	/* Free the gradient functions' cache. */
	for (i = 0; i < NumOfZeroMVs; i++) {
	    MvarMVFreeGradient(AttrGetPtrAttrib(LclMVs[i] -> Attr,
						"Gradient"));
	    AttrFreeOneAttribute(&LclMVs[i] -> Attr, "Gradient");
	}

        ZeroSet = MvarZeroFilterIdenticalSet(ZeroSet, LclMVs, Constraints,
					     NumOfMVs, NumOfZeroMVs,
					     sqrt(fabs(NumericTol)) * 10.0);
    }

    if (Grads != NULL)
        IritFree(Grads);

    for (i = 0; i < NumOfMVs; i++)
	MvarMVFree(LclMVs[i]);

    IritFree(LclMVs);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMvarFilteringZeroSet, FALSE) {
	    MvarPtStruct *Pt;

	    for (Pt = ZeroSet; Pt != NULL; Pt = Pt -> Pnext) {
	        IRIT_INFO_MSG("\n\tFiltered = ");
		for (i = 0; i < Pt -> Dim; i++)
		    IRIT_INFO_MSG_PRINTF("%.14f  ", Pt -> Pt[i]);
	    }
	}
    }
#   endif /* DEBUG */

    /* Sort the result based on first axis. */
    ZeroSet = MvarPtSortListAxis(ZeroSet, 1);

    return ZeroSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Filters out identical points or points that fail the inequality          M
* constraints.                                                               M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ZeroSet:      The solution points to filter out.                         M
*   MVs:          Vector of multivariate constraints.                        M
*   Constraints:  Either an equality or an inequality type of constraint.    M
*   NumOfMVs:     Size of the MVs and Constraints vector.                    M
*   NumOfZeroMVs: Number of zero set constraints (first in above vectors).   M
*   Tol:          Tolerance to consider to points same in L^1 norm.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Filtered solution points.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZeroFilterIdenticalSet                                               M
*****************************************************************************/
MvarPtStruct *MvarZeroFilterIdenticalSet(MvarPtStruct *ZeroSet,
					 MvarMVStruct * const *MVs,
					 const MvarConstraintType *Constraints,
					 int NumOfMVs,
					 int NumOfZeroMVs,
					 CagdRType Tol)
{
    MvarExprTreeEqnsStruct
	*Eqns = MvarExprTreeEqnsBuild2(MVs, Constraints, NumOfMVs);

    ZeroSet = MvarExprTreeEqnsZeroFilterIdenticalSet(ZeroSet, Eqns, Tol);

    MvarExprTreeEqnsFree(Eqns);

    return ZeroSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extracts the subdivision parametric domain from a multivariate.          *
*                                                                            *
* PARAMETERS:                                                                *
*   MV:         Multivariate to extract doamin from.			     *
*   Min, Max:   Subdivision parametric domain to extract.		     *
*   Dir:        Direction of sought domain.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarGetSubdivParamDomains(const MvarMVStruct *MV,
				      CagdRType *Min,
				      CagdRType *Max,
				      int Dir)
{
    if (MVAR_IS_BEZIER_MV(MV))
        MvarMVAuxDomainSlotGet(MV, Min, Max, Dir);
    else
	MvarMVDomain(MV, Min, Max, Dir);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a point of the dimension as the given MV in the middle of its  M
* parametric domain.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To construct a point in the middle of its domain.            M
*   SingleSol:  If TRUE, this point is a single solution it MV domain.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  The construct point in the middle of MV.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZeroGenPtMidMvar                                                     M
*****************************************************************************/
MvarPtStruct *MvarZeroGenPtMidMvar(const MvarMVStruct *MV, int SingleSol)
{
    int l;
    MvarPtStruct
        *Pt = MvarPtNew(MV -> Dim);

    for (l = 0; l < MV -> Dim; l++) {
	CagdRType Min, Max;

	MvarGetSubdivParamDomains(MV, &Min, &Max, l);
	Pt -> Pt[l] = (Min + Max) * 0.5;
    }

#   ifdef DEBUG_DUMP_DOMAINS
    fprintf(GlblDumpDomainsFile, "[OBJECT [RGB \"255,100,100\"] [Gray 0.5] [Width 0.02] NONE [CTLPT E%d", MV -> Dim);
    for (l = 0; l < MV -> Dim; l++)
	fprintf(GlblDumpDomainsFile, " %f", Pt -> Pt[l]);
    fprintf(GlblDumpDomainsFile, "]\n]\n");
#   endif /* DEBUG_DUMP_DOMAINS */

    AttrSetIntAttrib(&Pt -> Attr, "SingleSol", SingleSol);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Using the ideas of "Bezier-clipping" for both Bezier and/or B-spline,    *
* computes the possible reduced domain at both the minimum and the maximum   *
* of the domain, that is known to hold no zeros.			     *
*   Assumes all multivariates have the same domains and dimensions.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:        A vector of the multivariate constraints.                    *
*   NumOfMVs:   Size of the above multivariate constraints vector.           *
*   Dir:        Direction to compute domain reduction.			     *
*   SubdivTol:  Tolerance of the subdivision process.  Tolerance is          *
*	        measured in the parametric space of the multivariates.       *
*   TMin, TMax: Computed reduced domain.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if there might be some zeros, FALSE if no zero could exist *
*            in this case (i.e. all coefficients of the same sign, etc.)     *
*****************************************************************************/
static int MvarMVsReduceMvsDomains(MvarMVStruct **MVs,
				   int NumOfMVs,
				   int Dir,
				   CagdRType SubdivTol,
				   CagdRType *TMin,
				   CagdRType *TMax)
{
    int i,
	Dim = MVs[0] -> Dim,
	*Indices = (int *) IritMalloc(sizeof(int) * Dim);
    CagdRType
        OrigTMin = *TMin,
        OrigTMax = *TMax;

    if (_MVGlblSameSpace && MVs[0] -> Dim == NumOfMVs) {
        MvarMVStruct **NewMVs;

        /* Orthogonalize the (gradients of the) constraints at the center   */
	/* of the domain. 						    */
	if ((NewMVs = MvarMVsOrthogonalizeGrads(MVs, NumOfMVs)) != NULL) {
	    for (i = 0; i < NumOfMVs; i++) {
		MvarMVFree(MVs[i]);
		MVs[i] = NewMVs[i];
	    }
	}
    }

    for (i = 0; i < NumOfMVs; i++) {
	const MvarMVStruct
	    *MV = MVs[i];
	int NumOfSols, Index, j, k,
	    Length = MV -> Lengths[Dir],
	    Order = MV -> Orders[Dir],
	    Step = MVAR_NEXT_DIM(MV, Dir);
	CagdRType *Nodes, Sols[4], t;
	IPVertexStruct *V, *VEnv, *VNext,
	    *VUpperEnv = NULL,
	    *VLowerEnv = NULL;

	if (Length <= 1)
	    continue;

	if (MVAR_IS_BSPLINE_MV(MV)) {
	    Nodes = BspKnotNodes(MV -> KnotVectors[Dir],
				 Length + Order, Order);
	}
	else { /* MVAR_IS_BEZIER_MV(MV) */
	    Nodes = IritMalloc(sizeof(CagdRType) * Length);
	    for (j = 0; j < Length; j++)
	        Nodes[j] = j / ((CagdRType) (Length - 1));
	}

	IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);

	/* Compute the convex hull of the control mesh.  For each node value */
	/* we need only take minimum and maximum so 2 coefficients per node. */
	for (k = j = 0; k < Length; k++) {
	    CagdRType
	        *Pts = MV -> Points[1],
	        Min = IRIT_INFNTY,
	        Max = -IRIT_INFNTY;

	    Indices[Dir] = k;
	    Index = Step * k; 
	    do {
	        if (Min > Pts[Index])
		    Min = Pts[Index];
	        if (Max < Pts[Index])
		    Max = Pts[Index];
	    }
	    while (MVAR_INC_SKIP_MESH_INDICES(MV, Indices, Dir, Index));

	    if (VUpperEnv != NULL &&
		IRIT_APX_EQ_EPS(VUpperEnv -> Coord[0], Nodes[k], IRIT_UEPS)) {
	        /* Duplicated node values can happen if Order identical     */
	        /* knots are presented in the interior of the doamin.       */
	        VUpperEnv -> Coord[1] = IRIT_MAX(VUpperEnv -> Coord[1], Max);
	        VLowerEnv -> Coord[1] = IRIT_MIN(VLowerEnv -> Coord[1], Min);
	    }
	    else {
	        VUpperEnv = IPAllocVertex2(VUpperEnv);
		VLowerEnv = IPAllocVertex2(VLowerEnv);
		VUpperEnv -> Coord[0] = VLowerEnv -> Coord[0] = Nodes[k];
		VUpperEnv -> Coord[1] = IRIT_FABS(Max) < IRIT_UEPS ? 0.0 : Max;
		VLowerEnv -> Coord[1] = IRIT_FABS(Min) < IRIT_UEPS ? 0.0 : Min;
		VUpperEnv -> Coord[2] = VLowerEnv -> Coord[2] = 0.0;
	    }
	}

	VUpperEnv = IPReverseVrtxList2(VUpperEnv);
	VLowerEnv = IPReverseVrtxList2(VLowerEnv);
	GMMonotonePolyConvex(VUpperEnv, TRUE);
	GMMonotonePolyConvex(VLowerEnv, FALSE);

	/* Chain the two envelopes into one circular list (convex hull). */
	VEnv = IPReverseVrtxList2(VUpperEnv);
	((IPVertexStruct *) IPGetLastVrtx(VEnv)) -> Pnext = VLowerEnv;
	((IPVertexStruct *) IPGetLastVrtx(VLowerEnv)) -> Pnext = VEnv;

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCHOutputPrint, FALSE) {
	        IPVertexDbg(VEnv);
	    }
	}
#	endif /* DEBUG */

	V = VEnv;
	NumOfSols = 0;
	do {
	    VNext = V -> Pnext;

	    if (!IRIT_PT_APX_EQ_E2_EPS(VNext -> Coord,
				       V -> Coord, IRIT_UEPS)) {
	        CagdRType
		    Sol = -IRIT_INFNTY;

		if (V -> Coord[1] == 0.0) {
		    Sol = V -> Coord[0];
		}
		else if (V -> Coord[1] * VNext -> Coord[1] < 0.0) {
		    t = VNext -> Coord[1] / (VNext -> Coord[1] -
					                    V -> Coord[1]);
		    Sol = V -> Coord[0] * t + VNext -> Coord[0] * (1.0 - t);
		}

		if (Sol > -IRIT_INFNTY) {
		    for (j = 0; j < NumOfSols; j++)
		        if (IRIT_APX_EQ_EPS(Sols[j],  Sol, IRIT_UEPS))
			    break;

		    if (j >= NumOfSols)
		        Sols[NumOfSols++] = Sol;
		}
	    }

	    V = VNext;
	}
	while (V != VEnv);

	IPFreeVertexList(VEnv);

	switch (NumOfSols) {
	    case 0:
	        /* CH is above or below the zero - no intersection. */
	        IritFree(Indices);
		IritFree(Nodes);

		*TMin = OrigTMin;
		*TMax = OrigTMax;

		return FALSE;
	    case 1:
	        /* Select a very narrow domain... */
	        IritFree(Indices);
		IritFree(Nodes);
	        *TMin = Sols[0] - IRIT_MAX(SubdivTol, 10.0 * IRIT_EPS);
		*TMax = Sols[0] + IRIT_MAX(SubdivTol, 10.0 * IRIT_EPS);
		*TMin = IRIT_MAX(OrigTMin, *TMin);
		*TMax = IRIT_MIN(OrigTMax, *TMax);

		return TRUE;
	    case 2:
	        if (Sols[0] > Sols[1]) {
		    IRIT_SWAP(CagdRType, Sols[0], Sols[1]);
		}

	        /* Make sure we miss no root due to numeric errors. */
		Sols[0] = IRIT_MAX(OrigTMin, Sols[0] - 10.0 * IRIT_UEPS);
		Sols[1] = IRIT_MIN(OrigTMax, Sols[1] + 10.0 * IRIT_UEPS);
		break;
	    default: /* NumOfSols > 2. */
	        /* Something is wrong in our solutions' computations. */
	        IritFree(Indices);
		IritFree(Nodes);
	        *TMin = OrigTMin;
		*TMax = OrigTMax;
#		ifdef DEBUG_NUM_OF_SOLS
		    MvarMVsReduceMvsDomains(MVs, NumOfMVs, Dir,
					    SubdivTol, TMin, TMax);
		    MVAR_FATAL_ERROR(MVAR_ERR_INCONS_DOMAIN);
#		endif /* DEBUG_NUM_OF_SOLS */

		return TRUE;
	}

#	ifdef DEBUG
        {
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintReducedDomain, FALSE) {
	        int k;
	        MvarMVStruct *MVMin, *MVMax;
		CagdRType *R, *PtsMin, *PtsMax,
		    Min = Nodes == NULL ? 0.0 : Nodes[0],
		    Max = Nodes == NULL ? 1.0 : Nodes[Length - 1];

		if (Sols[0] > Min &&
		    !IRIT_APX_EQ_EPS(Sols[0], Min, IRIT_EPS)) {
		    MVMin = MvarMVRegionFromMV(MV, Min, Sols[0], Dir);
		    PtsMin = MVMin -> Points[1];

		    for (R = &PtsMin[1], k = MVAR_CTL_MESH_LENGTH(MVMin);
			 k-- > 1; ) {
		        if (*R++ * *PtsMin < -IRIT_EPS) {
			    printf("Error min in domain reduction computation\n");
			    MvarMVsReduceMvsDomains(MVs, NumOfMVs,
						    Dir, SubdivTol,
						    TMin, TMax);
			}
		    }

		    MvarMVFree(MVMin);
		}

		if (Sols[1] < Max &&
		    !IRIT_APX_EQ_EPS(Sols[1], Max, IRIT_EPS)) {
		    MVMax = MvarMVRegionFromMV(MV, Sols[1], Max, Dir);
		    PtsMax = MVMax -> Points[1];
		
		    for (R = &PtsMax[1], k = MVAR_CTL_MESH_LENGTH(MVMax);
			 k-- > 1; ) {
		        if (*R++ * *PtsMax < -IRIT_EPS) {
			    printf("Error max in domain reduction computation\n");
			    MvarMVsReduceMvsDomains(MVs, NumOfMVs,
						    Dir, SubdivTol,
						    TMin, TMax);
			}
		    }
		}
	    }
	}
#       endif /* DEBUG */

	IritFree(Nodes);

	/* The MV that gives the best (smallest) bound should be selected.   */
	*TMin = IRIT_MAX(*TMin, Sols[0]);
	*TMax = IRIT_MIN(*TMax, Sols[1]);
	if (*TMin > *TMax) {
	    /* The domains clipped from different constraints are disjoint.  */
	    /* No solution can exist here.				     */
	    IritFree(Indices);

	    return FALSE;
	}
    }

    IritFree(Indices);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given NumOfMVs multivariates, linearly blend them so that their	     *
* gradients are orthogonal at the center of their domain.		     *
*   This preconditioning of matrices is not only stabilizing the problem but *
* also speeds up the process by improving the domain-clipping and single     *
* solution test progress and chances of success.			     *
*   Assumes the number of multivariates equals to their dimension, and all   *
* MVs share the same function space (Orders, Lengths, and KVs.).	     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:          Multivariate to manipulate, so their gradients are         *
*                 orthogonal at the center of the domain.		     *
*   NumOfZeroMVs: Number of multivariates.  Must be equal to the dimension   *
*		  of the MVs.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct **:  A locally allocated vector of NumOfMVs                 *
*		  multivariates holding NumOfMVs orthogonalized new          *
*		  constraints, following by NumOfMVs references to the old   *
*		  constraints.						     *
*****************************************************************************/
static MvarMVStruct **MvarMVsOrthogonalizeGrads(MvarMVStruct **MVs,
						int NumOfMVs)
{
    IRIT_STATIC_DATA int
	AllocDim = 0;
    IRIT_STATIC_DATA IrtRType
        *Params = NULL;
    IRIT_STATIC_DATA IrtGnrlMatType
	A = NULL,
	InvA = NULL;
    IRIT_STATIC_DATA MvarMVStruct
	**NewMVs = NULL;
    int i, j, k, l,
	Len = MVAR_CTL_MESH_LENGTH(MVs[0]),
	PtSize = MVAR_NUM_OF_MV_COORD(MVs[0]),
	Dim = MVs[0] -> Dim;
    CagdRType *R, Min, Max, *APtr, W, *Pts, *NewPts;

    assert(Dim == NumOfMVs);

    /* Verify the size of the cache we will use here. */
    if (AllocDim <= Dim) {
        if (Params != NULL) {
	    IritFree(A);
	    IritFree(InvA);
	    IritFree(Params);
	    IritFree(NewMVs);
	}

	AllocDim = Dim * 2;
	A = (IrtGnrlMatType) IritMalloc(sizeof(IrtRType) * IRIT_SQR(AllocDim));
	InvA = (IrtGnrlMatType) IritMalloc(sizeof(IrtRType) *
							   IRIT_SQR(AllocDim));
	Params = (CagdRType *) IritMalloc(sizeof(IrtRType) * AllocDim);
	NewMVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *)
							           * AllocDim);
    }

    /* Compute the center of the domain, from MVs[0]. */
    for (i = 0; i < Dim; i++) {
	MvarGetSubdivParamDomains(MVs[0], &Min, &Max, i);
	Params[i] = (Min + Max) * 0.5;
    }

    /* Place the gradients as rows into matrix A. */
    for (i = 0; i < Dim; i++) {
	R = MvarMVEvalGradient2(MVs[i], Params, NULL);
	CAGD_GEN_COPY(&A[i * Dim], R, sizeof(CagdRType) * Dim);
    }

    /* Invert A. If failed, we have collinear gradients - abort. */
    if (!MatGnrlInverseMatrix(A, InvA, Dim))
        return NULL;

#ifdef DEBUG_TEST_GRAD_MATS
    printf("Gradient Mat:\n");
    MatGnrlPrintMatrix(A, Dim, stdout);
    printf("Gradient Inverse Mat:\n");
    MatGnrlPrintMatrix(InvA, Dim, stdout);
#endif /* DEBUG_TEST_GRAD_MATS */

    /* Copy the Dim MVs to the returned vector's Dim slots.*/
    for (i = 0; i < Dim; i++)
        NewMVs[i] = MvarMVCopy(MVs[i]);

    /* Blend the MVs using the blending dictated by InvA. Note we can either */
    /* have scalar fields, or vector fields with points of dimension Dim+1,  */
    /* having the scalar field and the gradients merged together.	     */
    if (PtSize == 1) {
        for (i = 0; i < Dim; i++) {
	    APtr = &InvA[i * Dim];
	    W = APtr[0];
	    Pts = MVs[0] -> Points[1];

	    for (j = 0, NewPts = NewMVs[i] -> Points[1]; j < Len; j++)
	        *NewPts++ = W * *Pts++;

	    for (k = 1; k < Dim; k++) {
	        W = APtr[k];
		Pts = MVs[k] -> Points[1];

		for (j = 0, NewPts = NewMVs[i] -> Points[1]; j < Len; j++)
		    *NewPts++ += W * *Pts++;
	    }
	}
    }
    else {
        for (l = 1; l <= PtSize; l++) {
	    for (i = 0; i < Dim; i++) {
		APtr = &InvA[i * Dim];
		W = APtr[0];
		Pts = MVs[Dim] -> Points[l];

		for (j = 0, NewPts = NewMVs[i] -> Points[l]; j < Len; j++)
		    *NewPts++ = W * *Pts++;

		for (k = 1; k < Dim; k++) {
		    W = APtr[k];
		    Pts = MVs[Dim + k] -> Points[1];

		    for (j = 0, NewPts = MVs[i] -> Points[l]; j < Len; j++)
		        *NewPts++ += W * *Pts++;
		}
	    }
	}
    }

#ifdef DEBUG_TEST_GRADS_ORTHO
    for (i = 0; i < Dim; i++) {
	R = MvarMVEvalGradient2(NewMVs[i], Params, NULL);
	CAGD_GEN_COPY(&A[i * Dim], R, sizeof(CagdRType) * Dim);
	printf("Gradient %d :: [", i);
	for (j = 0; j < Dim; j++)
	    printf("%f  ", R[j]);
	printf("]\n");
    }
    for (i = 0; i < Dim; i++) {
        for (j = i + 1; j < Dim; j++) {
	    IrtRType
	        R = 0.0;

	    for (k = 0; k < Dim; k++)
	        R += A[i * Dim + k] * A[j * Dim + k];

	    if (!IRIT_APX_EQ(R, 0.0))
	        printf("None orthogonal gradients detected.\n");
	}
    }
#endif /* DEBUG_TEST_GRADS_ORTHO */

    return NewMVs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a pair of parallel bounding hyperplanes of a multivariate       *
* constraint.								     *
*   The algorithm:                                                           *
* 1. Evaluate the gradient N0 at the midpoint of the domain of MV (with an   * 
*    additional (d+1)'th coefficient equal to -1), and normalize it.         *
* 2. For each of the control (d+1)-dim points Pi of MV (i.e., MV             *
*    coefficients coerced to R^d+1):                                         *
*    a. Project Pi onto N0 (i.e., inner product with N0), computing	     *
*       min <Pi, N0> and max <Pi, N0> of that projection.                    *
*    b. The bounding (d+1)-dim hyperplanes are:				     *
*         <N0, P - Pmin> = 0 and <N0, P - Pmax> = 0.			     *
* 3. Therefore, the d-hyperplanes (after intersecting with plane x_{d+1}=0)  *
*    are:  <N0_clip, P> = min <Pi, N0> and <N0_clip, P> = max <Pi, N0>,      *
*          where N0_clip is N0 without the (d+1)-coordinate.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   MV:        Multivariate to compute two parallel bounding hyperplanes.    *
*   Coeffs:    The first d-1 coefficients of the d-dimensional bounding	     *
*	       hyperplanes.						     *
*   Coeff0Max: The unit coefficient of first bounding hyperplanes.	     *
*   Coeff0Min: The unit coefficient of second bounding hyperplane.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None								     *
*****************************************************************************/
static void MVarMVHyperPlanesBound(const MvarMVStruct *MV,
				   CagdRType *Coeffs,
				   CagdRType *Coeff0Max,
				   CagdRType *Coeff0Min)
{
    int i, j, *IndicesVector,
	TotalLength = MVAR_CTL_MESH_LENGTH(MV),
	Dim = MV -> Dim,
	Index = 0;
    MvarVecStruct
        *UnitNormal = MvarVecNew(Dim + 1); 
    MvarVecStruct
        *D1Vec = MvarVecNew(Dim + 1); 
    MvarMVGradientStruct
	*Grad = NULL; 
    CagdRType *R, Min, Max, MinDotProd, MaxDotProd, DotProd,
	*Params = NULL,
	**NodeVectors = NULL;

    /* 1. Evaluate gradient at midpoint (& additional (d+1)-coordinate -1). */
    Params = (CagdRType *) IritMalloc(sizeof(IrtRType) * Dim);
    for (i = 0; i < Dim; i++) {
	MvarGetSubdivParamDomains(MV, &Min, &Max, i);
	Params[i] = (Min + Max) * 0.5;
    }

    if (MVAR_NUM_OF_MV_COORD(MV) == 1) {         /* No precomputed gradient. */
	Grad = MvarMVPrepGradient(MV, FALSE);

        /* Evaluate gradient at midpoint. */
	R = MvarMVEvalGradient(Grad, Params, 0);
        CAGD_GEN_COPY(UnitNormal -> Vec, R, sizeof(CagdRType) * Dim);
    }
    else if (MV -> Dim == MVAR_NUM_OF_MV_COORD(MV) - 1) {
        /* Gradient is embedded in Points[2] to Points[Dim + 1]. */
        
        /* Gradients are saved after the scalar value, in (1+Dim) vector. */
	R = MvarMVEval(MV, Params);
	CAGD_GEN_COPY(UnitNormal -> Vec, &R[2], sizeof(CagdRType) * Dim);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);   
	return;
    }
    UnitNormal -> Vec[Dim] = -1.0;
    MvarVecNormalize(UnitNormal);

    IritFree(Params);

    /* Project all control points of the mesh onto the normalized gradient.  */
    /*   The control (d+1)-points of the mesh are (i/k1, j/k2, ..., Pij) for */
    /* Bezier, scaled to the domain of MV.				     */
    /*   For BSpline, they are computed using the BspKnotNodes function.     */
    NodeVectors = (CagdRType **) IritMalloc(sizeof(CagdRType*) * MV -> Dim);

    /* Construct a nodes' vector for each dimension. */
    if (MVAR_IS_BEZIER_MV(MV)) {
        for (j = 0; j < MV -> Dim; j++) {
	    CagdRType
		*NodeVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
						       MV -> Orders[j]);

	    /* Domain needed for scaling. */
	    MvarGetSubdivParamDomains(MV, &Min, &Max, j);
            for (i = 0; i < MV -> Orders[j]; i++) {
	        NodeVector[i] = (i / (MV -> Orders[j] - 1.0))
							   * (Max - Min) + Min;
            }
            NodeVectors[j] = NodeVector;
        }
    }
    else {
        assert(MVAR_IS_BSPLINE_MV(MV));			    /* Sanity check. */
        for (j = 0; j < MV -> Dim; j++) {
            NodeVectors[j] = BspKnotNodes(MV -> KnotVectors[j],
					  MV -> Lengths[j] + MV -> Orders[j],
					  MV -> Orders[j]);
        }
    }

    /* 2a. Go over all control (d+1)-points, projecting and finding min and  */
    /*     max over N0 and find the extremum.				     */
    IndicesVector = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    IRIT_ZAP_MEM(IndicesVector, sizeof(int) * MV -> Dim);

    /* IndicesVector holds the indices of the current iteration and is	     */
    /* incremented inside the loop.  For example if we're at control point   */
    /* P312 (of a trivariate), the x-coordinate will be NodeVectors[1][3]    */
    /* (for Bezier this will be 3/OrderX), the y-coordinate will be          */
    /* NodesVector[2][1] (for Bezier, 1/OrderY), and the z-coordinate will   */
    /* be NodesVector[3][2] (for Bezier, 2/OrderZ).			     */

    /* Initializing MinDotProd and MaxDotProd with the values corresponding  */
    /* to Index == 0.							     */
    for (i = 0; i < Dim; i++) {
	D1Vec -> Vec[i] = NodeVectors[i][IndicesVector[i]]; 
    }
    /* The d+1 coefficient is the scalar control point. */
    D1Vec -> Vec[Dim] = MV -> Points[1][Index];

    /* We have in D1Vec, the control (d+1)-point P00..0, project it on N0. */
    MinDotProd = MaxDotProd = MvarVecDotProd(D1Vec, UnitNormal);

    /* Loop iterating over all control points of multivariate. */
    for (Index = 1; Index < TotalLength; Index++) {
        /* Getting the IndicesVector. */
        if (!MvarMeshIndicesFromIndex(Index, MV, IndicesVector)) {
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);   
	    return;
        }

        /* Coerce the scalar control point to a (d+1)-point.*/
        for (i = 0; i < Dim; i++) {
	    D1Vec -> Vec[i] = NodeVectors[i][IndicesVector[i]]; 
        }
        D1Vec -> Vec[Dim] = MV -> Points[1][Index];     /* The scalar ctlpt. */

        DotProd = MvarVecDotProd(D1Vec, UnitNormal);       /* project on N0. */

        if (DotProd < MinDotProd)
            MinDotProd = DotProd;
        if (DotProd > MaxDotProd)
            MaxDotProd = DotProd;
    }

    *Coeff0Min = MinDotProd; 
    *Coeff0Max = MaxDotProd; 

    /* Copy only first d coordinates of UnitNormal. */ 
    CAGD_GEN_COPY(Coeffs, UnitNormal -> Vec, Dim * sizeof(CagdRType));

    /* Free allocations. */
    if (Grad != NULL)
	MvarMVFreeGradient(Grad);

    for (j = 0; j < MV -> Dim; j++) {
        IritFree(NodeVectors[j]);
    }
    IritFree(NodeVectors);
    IritFree(IndicesVector);
    MvarVecFree(UnitNormal);
    MvarVecFree(D1Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks whether a solution of a set of multivariate constraints, might    *
* exist inside the domain, using pairs of parallel hyperplanes.		     *
*   Algorithm:								     *
* 1. Construct bounding hyperplanes, for all MVs.			     *
* 2. Solve for intersections of 2^d vertices of polytope.		     *
* 3. Compare all intersections with every bounding hyperplane (halfspace)    *
*    on the boundary of the domain.				 	     *
* 4. Returns FALSE if all intersections are on one side of domain boundary.  *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:            Multivariates to check their solutions.		     *
*   NumOfZeroMVs:   Size of the vector MVs.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if solution is possible in the domain, FALSE if no      *
*               solution is possible in the domain.                          *
*****************************************************************************/
static int MVarMVHyperPlanesTestForSol(MvarMVStruct *const *MVs,
				       int NumOfZeroMVs)
{
    IRIT_STATIC_DATA int
	AllocDim = 0;
    IRIT_STATIC_DATA CagdRType *Solutions,
	*A = NULL,
	*x = NULL,
	*bMin = NULL,
	*bMax = NULL,
	*bCopy = NULL;
    int i = 0,
	j = 0,
	k = 0,
	Dim = MVs[0] -> Dim,
	PowerTwoDim = (int) pow(2, Dim);

    /* In order to save alloc/free run-time we use static variables. */
    if (AllocDim < Dim) {
        if (AllocDim > 0) {
            IritFree(A);
            IritFree(x);
            IritFree(bMin);
            IritFree(bMax);
            IritFree(bCopy);
            IritFree(Solutions);
        }

	AllocDim = Dim * 2;
        A = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim * AllocDim);
        x = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        bMin = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        bMax = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        bCopy = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        Solutions = (CagdRType *) IritMalloc(sizeof(CagdRType) * PowerTwoDim
					     * PowerTwoDim * AllocDim);
    }

    /* Construct the bounding hyperplanes of each MV, keeping them as rows   */
    /* A while the scalar coefficients of the pair are kept in bMin/bMax.    */
    for (i = 0; i < NumOfZeroMVs; i++) {
        MVarMVHyperPlanesBound(MVs[i], &A[i * Dim], &bMax[i], &bMin[i]);
    }

    /* Compute QR decomposition of matrix A. */
    if (IritQRUnderdetermined(A, NULL, NULL, Dim, Dim)) {
	return TRUE;    /* Something went wrong - return TRUE (cannot tell). */
    }

    /* Loop over 2^(d) combinations of b vector (000 -> ---, 111 -> +++).    */
    /*   Store the points to check in the end if they are all on one side of */
    /* the box.								     */

    for (i = 0; i < PowerTwoDim; i++) {
        /* Construct relevant copy of b (bMin/bMAx defined by binary rep.).  */
        k = i;
        for (j = 0; j < Dim; ++j) {
	    bCopy[j] = k & 1 ? bMax[j] : bMin[j];
            k >>= 1;
        }

        IritQRUnderdetermined(NULL, &Solutions[i * Dim], bCopy, Dim, Dim);
    }

    for (j = 0; j < Dim; ++j) {
        int CompToBox;
        CagdRType jMin, jMax;

        MvarGetSubdivParamDomains(MVs[0], &jMin, &jMax, j);
        CompToBox = (Solutions[j] > jMax)
				       ? 1 : ((Solutions[j] < jMin) ? -1 : 0);
        /* -1 for left of Min, 1 for right of Max, 0 inbetween Min and Max. */
        if (CompToBox != 0) {
            for (i = 0; i < PowerTwoDim; i++) {
                int CompToBoxTmp = (Solutions[i * Dim + j] > jMax)
			     ? 1 : ((Solutions[i * Dim + j] < jMin) ? -1 : 0);

                if (CompToBox != CompToBoxTmp)
                    break;                
            }

            if (i == PowerTwoDim) {
                /* All intersections are on one side of Box - no solution.  */
		/* is possible inside the domain.			    */
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Approximate a solution to the set of constraints, if any, using the      M
* subdivision of the parametric domains of the MVs.  Stops when the          M
* parametric domain is smaller than SubdivTol in all dimensions and returns  M
* a central point to that small multivariate patch.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:          Vector of multivariate constraints.                        M
*   Constraints:  Either an equality or an inequality type of constraint.    M
*   NumOfMVs:     Size of the MVs and Constraints vector.                    M
*   NumOfZeroMVs: Number of zero or equality constraints.                    M
*   ApplyNormalConeTest:  TRUE to apply normal cones' single intersection    M
*		  tests.						     M
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        M
*		  measured in the parametric space of the multivariates.     M
*   Depth:        Of subdivision recursion.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  M
*		      points will be the same as the dimensions of all MVs.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZeroMVsSubdiv                                                        M
*****************************************************************************/
MvarPtStruct *MvarZeroMVsSubdiv(MvarMVStruct **MVs,
				MvarConstraintType *Constraints,
				int NumOfMVs,
				int NumOfZeroMVs,
				int ApplyNormalConeTest,
				CagdRType SubdivTol,
				int Depth)
{
    int i, j, l, HasInternalKnot,
	Dim = MVs[0] -> Dim;
    CagdRType Min, Max;

#   ifdef MVAR_DEBUG_DEPTH
    if (Depth < MVAR_DEBUG_MAX_DEPTH)
        MvarSubdivLevel[Depth]++;
#   endif /* MVAR_DEBUG_DEPTH */

#   ifdef DEBUG_ONE_SOLUTION
    if (NumOfZeroMVs == 2 || NumOfZeroMVs == 3) {
        static int
	    Count = 1;
        CagdRType UMin, UMax, VMin, VMax,
	    WMin = 0.0,
	    WMax = 0.0,
	    u = 0.02223875187684,
	    v = 0.87209881542057,
	    w = 0.43073326427685;

        MvarGetSubdivParamDomains(MVs[0], &UMin, &UMax, 0);
        MvarGetSubdivParamDomains(MVs[0], &VMin, &VMax, 1);
	if (NumOfZeroMVs == 3)
	    MvarGetSubdivParamDomains(MVs[0], &WMin, &WMax, 2);

	if (UMin <= u && u <= UMax &&
	    VMin <= v && v <= VMax &&
	    (NumOfZeroMVs < 3 || (WMin <= w && w <= WMax)))
	    IRIT_INFO_MSG_PRINTF("In domain (%d) [%f %f %f] \t[%f %f %f]\n",
				  Count++, UMin, VMin, WMin, UMax, VMax, WMax);
    }
#   endif /* DEBUG_ONE_SOLUTION */

#   ifdef DEBUG_DUMP_DOMAINS
    {
        CagdRType UMin, UMax, VMin, VMax;

        MvarGetSubdivParamDomains(MVs[0], &UMin, &UMax, 0);
        MvarGetSubdivParamDomains(MVs[0], &VMin, &VMax, 1);

	fprintf(GlblDumpDomainsFile,
		"[OBJECT [RGB \"100,255,100\"] NONE    [POLYLINE 5\n\t[%f %f 0]\n\t[%f %f 0]\n\t[%f %f 0]\n\t[%f %f 0]\n\t[%f %f 0]\n    ]\n]\n",
		UMin, VMin, UMin, VMax, UMax, VMax, UMax, VMin, UMin, VMin);
    }
#   endif /* DEBUG_DUMP_DOMAINS */

    /* Do we have a call back test function?  If so call it. */
    if (_MVGlblZeroSubdivCallBackFunc != NULL &&
	_MVGlblZeroSubdivCallBackFunc(&MVs, &Constraints, &NumOfMVs,
				      &NumOfZeroMVs, Depth))
        return NULL;

    /* Test if the multivariate may satisfy their constraints.  Examine the  */
    /* positivity/negativity of the set of values that multivariate assume   */
    /* and check against the prescribed constraint to that multivariate.     */
    for (i = 0; i < NumOfMVs; i++) {
	int Length = MVAR_CTL_MESH_LENGTH(MVs[i]),
	    Pos = FALSE,
	    Neg = FALSE;
        CagdRType
	    *R = MVs[i] -> Points[1];

	switch (Constraints[i]) {
	    case MVAR_CNSTRNT_ZERO:
	    case MVAR_CNSTRNT_ZERO_SUBDIV:
	        while (--Length >= 0) {
		    Pos |= (*R >= 0.0);
		    if ((Neg |= (*R++ <= 0.0)) && Pos)
		        break;
		}
		break;
	    case MVAR_CNSTRNT_POSITIVE:
	        while (--Length >= 0) {
		    if (*R++ >= 0.0)
		        break;
		}
		/* In inequality constraints, we do allow vector functions   */
		/* with the semantics that one positive scalar function is   */
		/* sufficient.  In other words, OR between scalar values.    */
		if (Length < 0 && MVAR_NUM_OF_MV_COORD(MVs[i]) > 1) {
		    for (l = 2; l <= MVAR_NUM_OF_MV_COORD(MVs[i]); l++) {
		        R = MVs[i] -> Points[l];
			Length = MVAR_CTL_MESH_LENGTH(MVs[i]);

			while (--Length >= 0) {
			    if (*R++ >= 0.0)
			        break;
			}
			if (Length >= 0)
			    break;	      /* Found positive coefficient. */
		    }
		}
		break;
	    case MVAR_CNSTRNT_NEGATIVE:
	        while (--Length >= 0) {
		    if (*R++ <= 0.0)
		        break;
		}
		/* In inequality constraints, we do allow vector functions   */
		/* with the semantics that one negative scalar function is   */
		/* sufficient.  In other words, OR between scalar values.    */
		if (Length < 0 && MVAR_NUM_OF_MV_COORD(MVs[i]) > 1) {
		    for (l = 2; l <= MVAR_NUM_OF_MV_COORD(MVs[i]); l++) {
		        R = MVs[i] -> Points[l];
			Length = MVAR_CTL_MESH_LENGTH(MVs[i]);

			while (--Length >= 0) {
			    if (*R++ <= 0.0)
			        break;
			}
			if (Length >= 0)
			    break;	      /* Found negative coefficient. */
		    }
		}
		break;
	}

	if (Length < 0)
	    return NULL;
    }

    /* If no solution is possible, by applying the pairs of hyperplanes      */
    /* bounding test, quit right here.					     */
    if (_MVGlblZeroApplyParallelHyperPlaneTest &&
	Dim == NumOfZeroMVs &&
	!MVarMVHyperPlanesTestForSol(MVs, NumOfZeroMVs))
        return NULL;

    /* Check the normal cone overlapping criteria.			     */
    if (ApplyNormalConeTest && !MvarMVConesOverlap(MVs, NumOfZeroMVs)) {
#	ifdef DEBUG
        {
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintZeroPts, FALSE) {
	        CagdRType Min, Max;

		IRIT_INFO_MSG("Zero by cones at");
		for (i = 0; i < Dim; i++) {
		    MvarGetSubdivParamDomains(MVs[0], &Min, &Max, i);
		    IRIT_INFO_MSG_PRINTF(" %f [%f %f]",
					 (Min + Max) * 0.5, Min, Max);
		}
		IRIT_INFO_MSG("\n");
	    }
	}
#       endif /* DEBUG */

        return MvarZeroGenPtMidMvar(MVs[0], TRUE);
    }

    /* If we got here then these patches may satisfy all constraints.        */
    /* Subdivide them along their maximal parametric length dimension.	     */
    /*   There is one exception to this rule and that is to subdivide first  */
    /* in internal knots so if the maximal parametric length dimension holds */
    /* no knots while another dir do has knots, the other dir will be used.  */
    l = -1;
    HasInternalKnot = FALSE;
    Max = Min = 0.0;
    for (i = 0; i < NumOfMVs && l < 0; i++) {
        for (j = 0; j < Dim && l < 0; j++) {
	    if (MVs[i] -> Lengths[j] != MVs[i] -> Orders[j]) {
	        CagdRType TMin, TMax;

	        /* Found a direction with internal knots. Save in l only if */
		/* this one is a larger domain than what was found so far.  */
	        MvarGetSubdivParamDomains(MVs[0], &TMin, &TMax, j);
		if (TMax - TMin > Max - Min) {
		    Max = TMax;
		    Min = TMin;
		    l = j;
		    HasInternalKnot = TRUE;
		}
	    }
	}
    }

    /* If all domains are Bezier-like, and we use domain reduction with     */
    /* gradient preconditioning, then make sure all functions share space.  */
    if (_MVGlblZeroApplyDomainReduction && 
	_MVGlblZeroApplyGradPreconditioning &&
	!_MVGlblSameSpace) { /* Promote all MVs to the same function space. */
        /* First pass will leave MVs[0] as most common denominator. */
        for (i = 1; i < NumOfMVs; i++)
	    MvarMakeMVsCompatible(&MVs[0], &MVs[i], TRUE, TRUE);

	/* Second pass will elevate all MVs[i] to MVs[0] space. */
	for (i = 1; i < NumOfMVs; i++)
	    MvarMakeMVsCompatible(&MVs[0], &MVs[i], TRUE, TRUE);
    }

    for (i = 0; i < Dim; i++) {
	CagdRType MinCurr, MaxCurr;

	MvarGetSubdivParamDomains(MVs[0], &MinCurr, &MaxCurr, i);
        if (MaxCurr - MinCurr > Max - Min) {
	    int j;

	    /* If we got internal knots with a domain larger than SubdivTol */
	    /* make sure this direction is a direction with knots.          */
	    if (HasInternalKnot && Max - Min > SubdivTol) {
	        for (j = 0; j < NumOfMVs; j++) {
		    if (MVs[j] -> Lengths[i] != MVs[j] -> Orders[i])
		        break;
		}
	    }
	    else
	        j = 0;

	    if (j < NumOfMVs) {
	        l = i;
		Min = MinCurr;
		Max = MaxCurr;
	    }
        }
    }
    assert(l >= 0);

    if (Max - Min > SubdivTol) {
        CagdBType
	    WasReduction = FALSE;
	CagdRType t,
	    TMin = MVAR_IS_BEZIER_MV(MVs[0]) ? 0.0 : Min,
	    TMax = MVAR_IS_BEZIER_MV(MVs[0]) ? 1.0 : Max;
	MvarPtStruct *PtList1, *PtList2;
	MvarMVStruct **MVs1, **MVs2;

	/* Apply the kantorovich test if so desired. */
	PtList1 = MvarZeroGetRootsByKantorovich(MVs,Constraints, NumOfMVs, 
	                                        NumOfZeroMVs, 
						ApplyNormalConeTest, 
						SubdivTol, Depth);
	if (PtList1 != NULL)
	    return PtList1;

	/* MvarMVsReduceMvsDomains returns TMin/TMax in [0, 1] for Bezier   */
	/* and returns correct TMin/TMax for B-spline MVs.		    */
	/*   Also force subdivisions from time to time as domain reduction  */
	/* can fail to converge if applied by itself.			    */
	if (_MVGlblZeroApplyDomainReduction && (Depth & 0x03) != 0) {
	    CagdRType
	        OrigTMin = TMin,
	        OrigTMax = TMax;

	    if (!MvarMVsReduceMvsDomains(MVs, NumOfZeroMVs,
					 l, SubdivTol, &TMin, &TMax))
		return NULL;    /* If the domain reduction ended up empty. */

	    WasReduction = !IRIT_APX_EQ(OrigTMin, TMin) ||
			   !IRIT_APX_EQ(OrigTMax, TMax);
	}

	if (WasReduction) {
	    MVs1 = (MvarMVStruct **) IritMalloc(NumOfMVs *
						      sizeof(MvarMVStruct *));

	    for (i = 0; i < NumOfMVs; i++)
	        MVs1[i] = MvarMVRegionFromMV(MVs[i], TMin, TMax, l);

	    PtList1 = MvarZeroMVsSubdiv(MVs1, Constraints, NumOfMVs,
					NumOfZeroMVs, ApplyNormalConeTest,
					SubdivTol, Depth + 1);

	    for (i = 0; i < NumOfMVs; i++)
	        MvarMVFree(MVs1[i]);

	    IritFree(MVs1);

	    return PtList1;
	}
	else {
	    if (_MVGlblZeroParamPerturb < (TMax - TMin) / 10.0)
	        t = (TMin + TMax) * 0.5 + _MVGlblZeroParamPerturb;
	    else
	        t = (TMin + TMax) * 0.5;

	    /* Lets see if we have a B-spline multivariate with interior    */
	    /* knot in this direction.  If so pick up interior knot instead.*/
	    if (MVAR_IS_BSPLINE_MV(MVs[0])) {
	        for (i = 0; i < NumOfMVs; i++) {
		    if (MVs[i] -> Lengths[l] != MVs[i] -> Orders[l]) {
		        CagdRType
			    r = MVs[i] -> KnotVectors[l][
						   (MVs[i] -> Lengths[l] +
						    MVs[i] -> Orders[l]) >> 1];

			if (r > TMin + IRIT_EPS && r < TMax - IRIT_EPS) {
			    t = r;
			    break;
			}
		    }
		}
	    }

	    /* Ensure we have t within domain, so subdivision can be used. */
	    assert(t >= TMin && t <= TMax);

	    MVs1 = (MvarMVStruct **) IritMalloc(NumOfMVs *
						      sizeof(MvarMVStruct *));
	    MVs2 = (MvarMVStruct **) IritMalloc(NumOfMVs *
						      sizeof(MvarMVStruct *));

	    for (i = 0; i < NumOfMVs; i++) {
	        MVs1[i] = MvarMVSubdivAtParam(MVs[i], t, l);
		MVs2[i] = MVs1[i] -> Pnext;
		MVs1[i] -> Pnext = NULL;
	    }
	    PtList1 = MvarZeroMVsSubdiv(MVs1, Constraints, NumOfMVs,
					NumOfZeroMVs, ApplyNormalConeTest,
					SubdivTol, Depth + 1);
	    PtList2 = MvarZeroMVsSubdiv(MVs2, Constraints, NumOfMVs,
					NumOfZeroMVs, ApplyNormalConeTest,
					SubdivTol, Depth + 1);

	    for (i = 0; i < NumOfMVs; i++) {
	        MvarMVFree(MVs1[i]);
		MvarMVFree(MVs2[i]);
	    }
	    IritFree(MVs1);
	    IritFree(MVs2);

	    return (MvarPtStruct *) CagdListAppend(PtList1, PtList2);
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintZeroPts, FALSE) {
	    CagdRType Min, Max;

	    IRIT_INFO_MSG("Zero by subdivision level at");
	    for (i = 0; i < Dim; i++) {
	        MvarGetSubdivParamDomains(MVs[0], &Min, &Max, i);
		IRIT_INFO_MSG_PRINTF(" %f [%f %f]",
				     (Min + Max) * 0.5, Min, Max);
	    }
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    /* If we got here then in all parametric directions, the domain is       */
    /* smaller than the SubdivTol.  Return central location of this patch.   */
    return MvarZeroGenPtMidMvar(MVs[0], FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A verification function to test the correctness of the solutions.        M
*   For development/debugging purposes.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:           Input constraints.                                        M
*   NumOfZeroMVs:  Number of (zero only) constraints.                        M
*   Sols:          Linked lists of solutions found.                          M
*   NumerEps:      Numeric tolerance used in the solution.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosVerifier                                                     M
*****************************************************************************/
void MvarMVsZerosVerifier(MvarMVStruct * const  *MVs,
			  int NumOfZeroMVs,
			  MvarPtStruct *Sols,
			  CagdRType NumerEps)
{
    int i, j;
    MvarPtStruct *Sol;

    NumerEps = IRIT_FABS(NumerEps);

    for (Sol = Sols; Sol != NULL; Sol = Sol -> Pnext) {
	for (i = 0; i < NumOfZeroMVs; i++) {
	    CagdRType
	        *R = MvarMVEval(MVs[i], Sol -> Pt);

	    if (!IRIT_APX_EQ_EPS(R[1], 0.0, NumerEps)) {
	        printf("Invalid solution! [");
		for (j = 0; j < Sol -> Dim; j++)
		    printf("%f%s",
			   Sol -> Pt[j], j == Sol -> Dim - 1 ? "]" : ", ");
		printf("MV[%d] = %g\n", i, R[1]);
	    }
	}
    }
}
