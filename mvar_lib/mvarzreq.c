/******************************************************************************
* MvarZrEq.c - Compute zero sets of multivariate expression tree equations.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel and Gershon Elber, Aug. 08.			      *
******************************************************************************/

#include "irit_sm.h"
#include "geom_lib.h"
#include "mvar_loc.h"

IRIT_GLOBAL_DATA MvarMVsZerosSubdivCallBackFunc
    _MVGlblZeroETSubdivCallBackFunc = NULL;
IRIT_GLOBAL_DATA CagdBType
    _MVGlblETUseCommonExpr = TRUE,
    _MVGlblETCnvrtBezier2MVs = TRUE;

/* #define DEBUG_MVAR_PRINT_ET_INFO */
/* #define DEBUG_DUMP_DOMAINS */

#ifdef DEBUG_DUMP_DOMAINS
IRIT_STATIC_DATA FILE
    *GlblDumpDomainsFile = NULL;
#endif /* DEBUG_DUMP_DOMAINS */


#ifdef DEBUG
IRIT_STATIC_DATA int
    _DebugMvarZeroErr = FALSE;
#endif /* DEBUG */

/* Expression trees' utility functions. */
static CagdBType MvarExprTreeEqnsIsHolding(MvarExprTreeEqnsStruct *Eqns);
static CagdBType MvarExprTreeEqnsOnlyMVs(MvarExprTreeEqnsStruct *Eqns);

/* Expression trees' solving routines. */
static MvarPtStruct *MvarExprTreeEqnsZerosAux(MvarExprTreeEqnsStruct *Eqns,
					      CagdRType SubdivTol,
					      CagdRType NumericTol);
static MvarPtStruct *MvarExprTreeEqnsZeroAux2(MvarExprTreeEqnsStruct *Eqns,
					      CagdRType SubdivTol);
static MvarPtStruct *MvarExprTreeEqnsZeroByMVs(MvarExprTreeEqnsStruct *Eqns,
					       CagdRType SubdivTol,
					       CagdRType NumericTol);
static CagdRType MvarExprTreeEqnsEvalErrorL1(const MvarExprTreeEqnsStruct
					                               *Eqns,
					     CagdRType *Params);
static CagdRType MvarMVEvalErrorL1(MvarMVStruct * const *MVs,
				   CagdRType *Params,
				   int NumOfMVs);
static int MvarEqnsPerturbZero(MvarPtStruct *Pt,
			       CagdRType StartRngErr,
			       const MvarExprTreeEqnsStruct *Eqns,
			       MvarMVStruct * const *MVs,
			       int NumEqns,
			       CagdRType *MinDmn,
			       CagdRType *MaxDmn,
			       CagdRType Step);

#if defined(DEBUG_MVAR_PRINT_ET_INFO) || defined(DEBUG_MVAR_REPORT_COMMON_EXPR)
static void MVDbgPrintfStdout(const char *Str);
#endif /* DEBUG_MVAR_PRINT_ET_INFO || DEBUG_MVAR_REPORT_COMMON_EXPR */

#define MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumMVs, x) \
	(Eqns) != NULL ? MvarExprTreeEqnsEvalErrorL1((Eqns), (x)) : \
			 MvarMVEvalErrorL1((MVs), (x), NumMVs)

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the exploitation of common expression extraction in expression      M
* trees.  If TRUE, the ETs are scanned for common expressions that are then  M
* processed once only, during the subdivision process.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   UseCommonExpr:   TRUE to use common expressions, FALSE otherwise.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting of common expressions' use.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreesZeros, MvarExprTreeZerosCnvrtBezier2MVs		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeZerosUseCommonExpr                                           M
*****************************************************************************/
int MvarExprTreeZerosUseCommonExpr(int UseCommonExpr)
{
    int OldVal = _MVGlblETUseCommonExpr;

    _MVGlblETUseCommonExpr = UseCommonExpr;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the way expression trees of Bezier MVs are treated.  If TRUE, the   M
* ET are converted into MVS and the regular MV zero solver is invoked.  If   M
* FALSE, the ET is subdivided all the way to SUbdivTol.			     M
* subdivisions' zero set solver.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Bezier2MVs:   TRUE to convert to MVs, FALSE to subdivide Bezier ETs.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting for bezier conversion setting.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreesZeros, MvarExprTreeZerosUseCommonExpr			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeZerosCnvrtBezier2MVs                                         M
*****************************************************************************/
int MvarExprTreeZerosCnvrtBezier2MVs(int Bezier2MVs)
{
    int OldVal = _MVGlblETCnvrtBezier2MVs;

    _MVGlblETCnvrtBezier2MVs = Bezier2MVs;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the call back function to invoke just before the expression tree    M
* recursion invokes the regular zeros solver (MvarMVsZeros) with the         M
* converted to multivariates.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   SubdivCallBackFunc:   New call back function, NULL to disable call back. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVsZerosSubdivCallBackFunc:   Old call back function.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreesZeros                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeZerosSetCallBackFunc                                         M
*****************************************************************************/
MvarMVsZerosSubdivCallBackFunc MvarExprTreeZerosSetCallBackFunc(
			   MvarMVsZerosSubdivCallBackFunc SubdivCallBackFunc)
{
    MvarMVsZerosSubdivCallBackFunc
	OldVal = _MVGlblZeroETSubdivCallBackFunc;

    _MVGlblZeroETSubdivCallBackFunc = SubdivCallBackFunc;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocate a structure to hold NumEqns equations and at most               M
* MaxNumCommonExprs common expressions.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   NumEqns:            Number of equations we have.                         M
*   MaxNumCommonExprs:  Maximum number of common expression we can initially M
*			hold.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeEqnsStruct *:  Allocated structure.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarExprTreeEqnsFree					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsMalloc                                                   M
*****************************************************************************/
MvarExprTreeEqnsStruct *MvarExprTreeEqnsMalloc(int NumEqns,
					       int MaxNumCommonExprs)
{
    MvarExprTreeEqnsStruct
        *Eqns = (MvarExprTreeEqnsStruct *)
				   IritMalloc(sizeof(MvarExprTreeEqnsStruct));

    /* Allocate the equations' slots. */
    Eqns -> Eqns = (MvarExprTreeStruct **)
		           IritMalloc(sizeof(MvarExprTreeStruct *) * NumEqns);
    IRIT_ZAP_MEM(Eqns -> Eqns, sizeof(MvarExprTreeStruct *) * NumEqns);
    Eqns -> NumEqns = NumEqns;
    Eqns -> NumZeroEqns = Eqns -> NumZeroSubdivEqns = -1;

    /* Allocate the common expressions' slots. */
    if (MaxNumCommonExprs > 0)
        Eqns -> CommonExprs = (MvarExprTreeStruct **)
		 IritMalloc(sizeof(MvarExprTreeStruct *) * MaxNumCommonExprs);
    else
        Eqns -> CommonExprs = NULL;
    Eqns -> NumCommonExprs = 0;
    Eqns -> MaxNumCommonExprs = MaxNumCommonExprs;

    /* Allocate the constraints' types. */
    Eqns -> ConstraintTypes = IritMalloc(sizeof(MvarConstraintType) * NumEqns);

    return Eqns;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free all data allocated in the expression tree equation's structure.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqns:   Data structure to free.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarExprTreeEqnsMalloc				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsFree                                                     M
*****************************************************************************/
void MvarExprTreeEqnsFree(MvarExprTreeEqnsStruct *Eqns)
{
    int i;

    for (i = 0; i < Eqns -> NumCommonExprs; i++) {
        if (Eqns -> CommonExprs[i] != NULL)
	    MvarExprTreeFree(Eqns -> CommonExprs[i], FALSE);
    }

    if (Eqns -> CommonExprs != NULL)
        IritFree(Eqns -> CommonExprs);

    for (i = 0; i < Eqns -> NumEqns; i++)
        MvarExprTreeFree(Eqns -> Eqns[i], FALSE);
    IritFree(Eqns -> Eqns);

    IritFree(Eqns -> ConstraintTypes);

    IritFree(Eqns);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reallocate (increase) the number of common expressions give Eqns can     M
* hold, in place.						             M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqns:      Set of equations to increase, in place, the number of coomon  M
*              expressions it can old.					     M
*   NewSize:   of vector of common expression, zero to double the size.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsReallocCommonExprs                                       M
*****************************************************************************/
void MvarExprTreeEqnsReallocCommonExprs(MvarExprTreeEqnsStruct *Eqns,
					int NewSize)
{
    if (NewSize == 0)
        NewSize = Eqns -> MaxNumCommonExprs * 2;
    else if (NewSize <= Eqns -> MaxNumCommonExprs)
        return;

    Eqns -> CommonExprs =
        IritRealloc(Eqns -> CommonExprs,
		    sizeof(MvarExprTreeStruct *) * Eqns -> MaxNumCommonExprs,
		    sizeof(MvarExprTreeStruct *) * NewSize);

    Eqns -> MaxNumCommonExprs = NewSize;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Traverse an expression tree and update every common expression index to  M
* a real reference to a (common) expression tree.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:         Expression tree to traverse and update common expresssions.  M
*   CommonExprs:      Vector of common expressions to reference from MVET.   M
*   NumOfCommonExpr:  Size of CommonExprs vector.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsUpdateCommonExprIdcs                                     M
*****************************************************************************/
void MvarExprTreeEqnsUpdateCommonExprIdcs(MvarExprTreeStruct *ET,
					  MvarExprTreeStruct **CommonExprs,
					  int NumOfCommonExpr)
{
    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    MvarExprTreeEqnsUpdateCommonExprIdcs(ET -> Right, CommonExprs,
						 NumOfCommonExpr);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    MvarExprTreeEqnsUpdateCommonExprIdcs(ET -> Left, CommonExprs,
						 NumOfCommonExpr);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    assert(ET -> IAux >= 0 && ET -> IAux < NumOfCommonExpr);
	    ET -> Left = CommonExprs[ET -> IAux];
	    break;
	default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a set of equations over some domain, divided them all in Dir at t. M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqns:    The equations to subdivide.                                     M
*   t:       Parameter to subdivide at.                                      M
*   Dir:     Direction of subdivision.                                       M
*   Eqns1:   First set of divided equations.				     M
*   Eqns2:   Second set of divided equations.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsSubdivAtParam	                                     M
*****************************************************************************/
int MvarExprTreeEqnsSubdivAtParam(const MvarExprTreeEqnsStruct *Eqns,
				  CagdRType t,
				  MvarMVDirType Dir,
				  MvarExprTreeEqnsStruct **Eqns1,
				  MvarExprTreeEqnsStruct **Eqns2)
{
    int i;

    *Eqns1 = MvarExprTreeEqnsMalloc(Eqns -> NumEqns,
				    Eqns -> MaxNumCommonExprs);
    *Eqns2 = MvarExprTreeEqnsMalloc(Eqns -> NumEqns,
				    Eqns -> MaxNumCommonExprs);

    /* Subdivide the common expressions first, if any. */
    for (i = 0; i < Eqns -> NumCommonExprs; i++) {
        if (Eqns -> CommonExprs[i] == NULL) {
	    (*Eqns1) -> CommonExprs[i] = (*Eqns2) -> CommonExprs[i] = NULL;
	}
	else {
#ifdef DEBUG
	    int RetVal = 
#endif /* DEBUG */
	        MvarExprTreeSubdivAtParam(Eqns -> CommonExprs[i],
					  t, Dir,
					  &(*Eqns1) -> CommonExprs[i],
					  &(*Eqns2) -> CommonExprs[i]);

	    assert(RetVal);

	    /* No need to go over the trees if no common eqns, recursively. */
	    if (Eqns -> NumCommonExprs > 0) {
	        MvarExprTreeEqnsUpdateCommonExprIdcs(
						 (*Eqns1) -> CommonExprs[i],
						 (*Eqns1) -> CommonExprs,
						 Eqns -> NumCommonExprs);
		MvarExprTreeEqnsUpdateCommonExprIdcs(
						 (*Eqns2) -> CommonExprs[i],
						 (*Eqns2) -> CommonExprs,
						 Eqns -> NumCommonExprs);
	    }
	}
    }

    /* Subdivide the equations themselves now. */
    for (i = 0; i < Eqns -> NumEqns; i++) {
#ifdef DEBUG
	    int RetVal = 
#endif /* DEBUG */
	        MvarExprTreeSubdivAtParam(Eqns -> Eqns[i], t, Dir,
					  &(*Eqns1) -> Eqns[i],
					  &(*Eqns2) -> Eqns[i]);

	assert(RetVal);

        /* No need to go over the trees if no common equations.*/
        if (Eqns -> NumCommonExprs > 0) {
	    MvarExprTreeEqnsUpdateCommonExprIdcs((*Eqns1) -> Eqns[i],
						 (*Eqns1) -> CommonExprs,
						 Eqns -> NumCommonExprs);
	    MvarExprTreeEqnsUpdateCommonExprIdcs((*Eqns2) -> Eqns[i],
						 (*Eqns2) -> CommonExprs,
						 Eqns -> NumCommonExprs);
        }
    }

    (*Eqns1) -> NumCommonExprs =
        (*Eqns2) -> NumCommonExprs = Eqns -> NumCommonExprs;

    IRIT_GEN_COPY((*Eqns1) -> ConstraintTypes, Eqns -> ConstraintTypes,
	     sizeof(MvarConstraintType) * Eqns -> NumEqns);
    IRIT_GEN_COPY((*Eqns2) -> ConstraintTypes, Eqns -> ConstraintTypes,
	     sizeof(MvarConstraintType) * Eqns -> NumEqns);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place all nodes (top down) in one linear vector.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:            Expression tree to place all its node on Vec.	     M
*   Vec:           Where to place all nodes of ET, top down.		     M
*   Idx:           Index into vec where to place the next node.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	   Last index of a placed node in vec.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeToVector		                                     M
*****************************************************************************/
int MvarExprTreeToVector(MvarExprTreeStruct *ET,
			 MvarExprTreeStruct **Vec,
			 int Idx)
{
    if (ET == NULL)
        return Idx;

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    Vec[Idx++] = ET;
	    return Idx;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    Idx = MvarExprTreeToVector(ET -> Left, Vec, Idx);
	    Idx = MvarExprTreeToVector(ET -> Right, Vec, Idx);
	    Vec[Idx++] = ET;  /* We want the smaller (sons) subtrees first. */
	    return Idx;
	case MVAR_ET_NODE_COMMON_EXPR:
	    /* Should not encounter COMMON EXPR nodes at this time. */
	default:
	    assert(0);
	    return 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes all references in vector from position i to all the subtree.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:   Vector to remove all reference (make NULL) from the i'th pos.     M
*   Idx:   Position in vector where to remove the references from.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Index of next item in vector, after all removed items.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeRemoveFromVector                                             M
*****************************************************************************/
int MvarExprTreeRemoveFromVector(MvarExprTreeStruct **Vec, int Idx)
{
    int Idx2;

    if (Vec[Idx] == NULL)
        return Idx + 1;

    switch (Vec[Idx] -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    Idx2 = Idx + 1;
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    Idx2 = MvarExprTreeRemoveFromVector(Vec, Idx + 1);
	    Idx2 = MvarExprTreeRemoveFromVector(Vec, Idx2);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    /* Should not encounter COMMON EXPR nodes at this time. */
	default:
	    Idx2 = Idx;
	    assert(0);
	    break;
    }

    Vec[Idx] = NULL;

    return Idx2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy the given expression trees and process and fetch common expressions M
* out to a separated common expressions' vector, all within the returned     M
* expression tree equations structure.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVETs:      Input mvar expression tree equations.                        M
*   ConstraintTypes:  Type of MVETs constraints.  Same length as MVETs vec.  M
*   NumOfMVETs: Number of input mvar expression tree equations, in MVETs.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeEqnsStruct *:   Build set of equations with common exprs.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsBuild		                                     M
*****************************************************************************/
MvarExprTreeEqnsStruct *MvarExprTreeEqnsBuild(MvarExprTreeStruct **MVETs,
					      const MvarConstraintType
					                   *ConstraintTypes,
					      int NumOfMVETs)
{
    int i, j, k, n, Res,
	MaxNumCommonExprs = _MVGlblETUseCommonExpr ? NumOfMVETs : 0;
    MvarExprTreeStruct *Expr, **Exprs;
    MvarExprTreeEqnsStruct
	*Eqns = MvarExprTreeEqnsMalloc(NumOfMVETs, MaxNumCommonExprs);

    /* Copy the constraints. */
    for (i = n = 0; i < NumOfMVETs; i++) {
        Eqns -> Eqns[i] = MvarExprTreeCopy(MVETs[i], FALSE, TRUE);
	n += MvarExprTreeSize(Eqns -> Eqns[i]);

	/* Make sure we only have Bsp leaves (and no Bzr leaves.) */
        Res = MvarExprTreeCnvrtBzr2BspMV(Eqns -> Eqns[i]);
        assert(Res);
    }
    IRIT_GEN_COPY(Eqns -> ConstraintTypes, ConstraintTypes,
	     sizeof(MvarConstraintType) * NumOfMVETs);

    /* Make sure all zero constraints are first, zero-subdiv are second,    */
    /* and count how many.						    */
    for (i = 0; i < NumOfMVETs; i++) {
        if (Eqns -> ConstraintTypes[i] != MVAR_CNSTRNT_ZERO) {
	    int j;

	    for (j = i + 1; j < NumOfMVETs; j++) {
	        if (Eqns -> ConstraintTypes[j] == MVAR_CNSTRNT_ZERO) {
		    IRIT_SWAP(MvarConstraintType, Eqns -> ConstraintTypes[i],
			                     Eqns -> ConstraintTypes[j]);
		    IRIT_SWAP(MvarExprTreeStruct *, Eqns -> Eqns[i],
                                               Eqns -> Eqns[j]);
		    break;
		}
	    }
	    if (j >= NumOfMVETs)
	        break;
	}
    }
    Eqns -> NumZeroEqns = i;

    for (i = Eqns -> NumZeroEqns; i < NumOfMVETs; i++) {
        if (Eqns -> ConstraintTypes[i] != MVAR_CNSTRNT_ZERO_SUBDIV) {
	    int j;

	    for (j = i + 1; j < NumOfMVETs; j++) {
	        if (Eqns -> ConstraintTypes[j] == MVAR_CNSTRNT_ZERO_SUBDIV) {
		    /* Swap constraints i and j. */
		    IRIT_SWAP(MvarConstraintType, Eqns -> ConstraintTypes[i],
			                     Eqns -> ConstraintTypes[j]);
		    IRIT_SWAP(MvarExprTreeStruct *, Eqns -> Eqns[i],
                                               Eqns -> Eqns[j]);
		    break;
		}
	    }
	    if (j >= NumOfMVETs)
	        break;
	}
    }
    Eqns -> NumZeroSubdivEqns = i;

    if (!_MVGlblETUseCommonExpr)
        return Eqns; 

    /* Build an auxiliary vector of all expressions in these expression     */
    /* trees, top level expressions first, and search in O(n^2) for all     */
    /* expressions that are the same, and mark them as such.	            */
    Exprs = (MvarExprTreeStruct **)
                                 IritMalloc(sizeof(MvarExprTreeStruct *) * n);
    for (i = k = 0; i < NumOfMVETs; i++)
        k = MvarExprTreeToVector(Eqns -> Eqns[i], Exprs, k);
    assert(k == n);

    /* Now search for similar expressions in O(n^2). */
    for (i = 0; i < n; i++) {
        Exprs[i] -> IAux = -1;
	Exprs[i] -> PAux = NULL;
    }

#   ifndef MVAR_NO_COMMON_EXPRESSION/* Define to test with no common exprs. */
    for (i = k = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
	    if (Exprs[j] -> IAux > -1)
	        continue;

	    if (MvarExprTreesSame(Exprs[i], Exprs[j], IRIT_UEPS)) {
	        if (Exprs[i] -> IAux == -1)
		    Exprs[i] -> IAux = k++;

	        Exprs[j] -> IAux = Exprs[i] -> IAux;
	    }
	}
    }
#   endif /* MVAR_NO_COMMON_EXPRESSION */

    if (k > 0) {
        /* Having k common expressions - make sure we can hold them all. */
        MvarExprTreeEqnsReallocCommonExprs(Eqns, k);
	IRIT_ZAP_MEM(Eqns -> CommonExprs, k * sizeof(MvarExprTreeStruct *));

	/*  For every common expressions do:                                */
	/*  + Copy the common expression to the common expressions area.    */
	/*  + Remove expressions from original ETs and substitute with a    */
	/*    MVAR_ET_NODE_COMMON_EXPR NodeType that refers the common expr.*/
	for (i = 0; i < n; i++) {
	    int m;

	    /* skip if not a common expression or already processed. */
	    if (Exprs[i] -> IAux < 0 || Exprs[i] -> PAux != NULL)
	        continue;

	    /* If we are here, then this one is marked a common expression. */
	    j = Exprs[i] -> IAux;
	    assert(j < k);

	    if (Eqns -> CommonExprs[j] == NULL) {
	        /* Copy this expression to the CommonExprs vector - it is   */
	        /* the first time we encounter the expression in this loop. */
	        Eqns -> CommonExprs[j] = MvarExprTreeCopy(Exprs[i],
							  FALSE, TRUE);

		/* Cannot have a common expression as root node here. */
		assert(Eqns -> CommonExprs[j] -> NodeType !=
		                                    MVAR_ET_NODE_COMMON_EXPR);
	    }
	    else {
	        /* Other instances should have been removed by code below!  */
	        assert(Exprs[i] -> NodeType == MVAR_ET_NODE_COMMON_EXPR);
	    }

	    /* Remove all sub-expression of this one from vector, free      */
	    /* them, and make them a common expression reference node.      */
	    for (m = 0; m < n; m++) {
	        if (Exprs[m] -> IAux == j) {
		    Expr = Exprs[m];
		    MvarExprTreeFreeSlots(Expr, TRUE);
		    Expr -> Left = Eqns -> CommonExprs[j];
		    Expr -> Right = NULL;
		    Expr -> NodeType = MVAR_ET_NODE_COMMON_EXPR;
		    Expr -> PAux = Expr -> Left;      /* Mark as processed. */
		}
	    }
	}
	Eqns -> NumCommonExprs = k;

#	ifdef DEBUG
	    for (i = 0; i < k; i++)
	        assert(Eqns -> CommonExprs[i] != NULL);
#	endif /* DEBUG */
    }

    /* Note that not all k entries in CommonExprs will be valid and some can */
    /* be NULL as we could find a common sub expression in a common super    */
    /* expression, and we grab as common only the sper expression.           */

#ifdef DEBUG_MVAR_REPORT_COMMON_EXPR
    for (i = 0; i < k; i++)
        assert(Eqns -> CommonExprs[i] != NULL);

    for (i = 0; i < n; i++) {
        if (Exprs[i] -> NodeType == MVAR_ET_NODE_COMMON_EXPR) {
	    assert(Exprs[i] -> Left != NULL && Exprs[i] -> Right == NULL);
	}
    }

    printf("\nFound %d common expression out of %d expressions in %d equations.\n",
	    k, n, NumOfMVETs);
    
    printf("Common Expressions:\n");
    for (i = 0; i < k; i++) {
        printf("\nCE %d (%08x): ", i, Eqns -> CommonExprs[i]);
	MvarExprTreePrintInfo(Eqns -> CommonExprs[i], TRUE, MVDbgPrintfStdout);
        printf("\nCE %d: ", i);
	MvarExprTreePrintInfo(Eqns -> CommonExprs[i], FALSE, MVDbgPrintfStdout);
    }
    printf("\n\nEquations:\n");
    for (i = 0; i < NumOfMVETs; i++) {
        printf("\nEqn %d: ", i);
	MvarExprTreePrintInfo(Eqns -> Eqns[i], TRUE, MVDbgPrintfStdout);
        printf("\nEqn %d: ", i);
	MvarExprTreePrintInfo(Eqns -> Eqns[i], FALSE, MVDbgPrintfStdout);
    }
    printf("\n");
#endif /* DEBUG_MVAR_REPORT_COMMON_EXPR */

    IritFree(Exprs);

    return Eqns;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy the given expression trees and process and fetch common expressions M
* out to a separated common expressions' vector, all within the returned     M
* expression tree equations structure.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:        Input mvar expression tree equations.                        M
*   ConstraintTypes:  Type of MVETs constraints.  Same length as MVETs vec.  M
*   NumOfMVs:   Number of input mvar expression tree equations, in MVETs.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeEqnsStruct *:   Build set of equations with common exprs.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsBuild2		                                     M
*****************************************************************************/
MvarExprTreeEqnsStruct *MvarExprTreeEqnsBuild2(MvarMVStruct * const *MVs,
					       const MvarConstraintType
					                   *ConstraintTypes,
					       int NumOfMVs)
{
    int i;
    MvarExprTreeStruct
	**MVETs = (MvarExprTreeStruct **)
		         IritMalloc(sizeof(MvarExprTreeStruct *) * NumOfMVs);
    MvarExprTreeEqnsStruct *Eqns;

    for (i = 0; i < NumOfMVs; i++)
        MVETs[i] = MvarExprTreeFromMV2(MVs[i]);

    Eqns = MvarExprTreeEqnsBuild(MVETs, ConstraintTypes, NumOfMVs);

    for (i = 0; i < NumOfMVs; i++)
        MvarExprTreeFree(MVETs[i], FALSE);
    IritFree(MVETs);

    return Eqns;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine the given constraints and return TRUE if the constraints do not  *
* fail.                                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Eqns:        To examine.                                                 *
*   Constraints: Type of constraints in Eqns.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:   FALSE if one (or more) constraints clearly fails.           *
*****************************************************************************/
static CagdBType MvarExprTreeEqnsIsHolding(MvarExprTreeEqnsStruct *Eqns)
{
    int i,
        NumOfMVETs = Eqns -> NumEqns;

    /* Examine the possible values the trees can assume. */
    for (i = 0; i < NumOfMVETs; i++) {
        const MvarBBoxStruct
	    *BBox = MvarExprTreeBBox(Eqns -> Eqns[i]);

	assert(BBox -> Dim == 1);   /* Expects scalar value to be returned. */

	switch (Eqns -> ConstraintTypes[i]) {
	    default:
	        assert(0);
	    case MVAR_CNSTRNT_ZERO:
	        if (BBox -> Min[0] > 0.0 || BBox -> Max[0] < 0.0)
		    return FALSE;
		break;
	    case MVAR_CNSTRNT_POSITIVE:
	        if (BBox -> Max[0] <= 0.0)
		    return FALSE;
		break;
	    case MVAR_CNSTRNT_NEGATIVE:
	        if (BBox -> Min[0] >= 0.0)
		    return FALSE;
		break;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine if the given equations consist of MVs only (i.e. degenerated     *
* expression trees of a single MV node).				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Eqns:  To examine.                                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:                                                               *
*****************************************************************************/
static CagdBType MvarExprTreeEqnsOnlyMVs(MvarExprTreeEqnsStruct *Eqns)
{
    int i,
        NumOfMVETs = Eqns -> NumEqns;

    for (i = 0; i < NumOfMVETs; i++) {
        if (Eqns -> Eqns[i] -> NodeType != MVAR_ET_NODE_LEAF)
            return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the simultaneous solution of the given set of NumOfMVETs        M
* expression tree constraints.  A constraints can be equality or ineqaulity  M
* as prescribed by the Constraints vector. 				     M
*   All multivariates are assumed to be scalar and be in the same parametric M
* domain size and dimension.						     M
*   This procedure performs a preliminary subdivision until the geometry is  M
* found simple enough (I.e. no internal knots or small enough), when it      M
* invokes MvarMVsZeros.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVETs:        Vector of multivariate expression tree constraints.        M
*   Constraints:  Either an equality or an inequality type of constraint.    M
*   NumOfMVETs:   Size of the MVs expresion trees and Constraints vectors.   M
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
*   MvarMVsZeros, MvarExprTreeZerosSetCallBackFunc,			     M
*   MvarExprTreeZerosCnvrtBezier2MVs, MvarExprTreeZerosUseCommonExpr	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreesZeros                                                       M
*****************************************************************************/
MvarPtStruct *MvarExprTreesZeros(MvarExprTreeStruct **MVETs,
				 MvarConstraintType *Constraints,
				 int NumOfMVETs,
				 CagdRType SubdivTol,
				 CagdRType NumericTol)
{
    MvarPtStruct *RetPts;
    MvarExprTreeEqnsStruct *Eqns;

    /* Build the Eqns struct (& extract all common expressions if needed). */
    Eqns = MvarExprTreeEqnsBuild(MVETs, Constraints, NumOfMVETs);

    RetPts = MvarExprTreeEqnsZeros(Eqns, SubdivTol, NumericTol);

    MvarExprTreeEqnsFree(Eqns);

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the simultaneous solution of the given set of NumOfMVETs        M
* expression tree constraints.  A constraints can be equality or ineqaulity  M
* as prescribed by the Constraints vector. 				     M
*   All multivariates are assumed to be scalar and be in the same parametric M
* domain size and dimension.						     M
*   This procedure performs a preliminary subdivision until the geometry is  M
* found simple enough (I.e. no internal knots or small enough), when it      M
* invokes MvarMVsZeros.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqns:         All the equations as expression tree constraints.          M
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
*   MvarMVsZeros, MvarExprTreeZerosSetCallBackFunc,			     M
*   MvarExprTreeZerosCnvrtBezier2MVs, MvarExprTreeZerosUseCommonExpr	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsZeros                                                    M
*****************************************************************************/
MvarPtStruct *MvarExprTreeEqnsZeros(MvarExprTreeEqnsStruct *Eqns,
				    CagdRType SubdivTol,
				    CagdRType NumericTol)
{ 
    MvarPtStruct
        *RetPts = NULL;
    CagdBType
        IsOnlyMVs = FALSE;

    /* Make sure we are reasonable. */
    SubdivTol = IRIT_MAX(SubdivTol, 1e-8);
 
#   ifdef DEBUG_MVAR_PRINT_ET_INFO
    {
        int i;

	for (i = 0; i < Eqns -> NumCommonExprs; i++) {
	    assert(Eqns -> CommonExprs[i] != NULL);
	    printf("MVCE %d: ", i);
	    MvarExprTreePrintInfo(Eqns -> CommonExprs[i], FALSE,
				  MVDbgPrintfStdout);
	    printf("\n");
	}
	printf("\n\n");

	for (i = 0; i < Eqns -> NumEqns; i++) {
	    printf("MVEQ %d: ", i);
	    MvarExprTreePrintInfo(Eqns -> Eqns[i], TRUE, MVDbgPrintfStdout);
	    printf("\nMVEQ %d: ", i);
	    MvarExprTreePrintInfo(Eqns -> Eqns[i], FALSE, MVDbgPrintfStdout);
	    printf("\n\n");
	}
    }
#   endif /* DEBUG_MVAR_PRINT_ET_INFO */

    IsOnlyMVs = MvarExprTreeEqnsOnlyMVs(Eqns);    
        
    /* Compute the zeros. */
    if (IsOnlyMVs) {
        /* Calling  zero function for MV only Eqns. Includes numeric stage. */
        RetPts = MvarExprTreeEqnsZeroByMVs(Eqns, SubdivTol, NumericTol);
    }
    else {
        /* Call subdiv recursive function all the way. */
        RetPts = MvarExprTreeEqnsZerosAux(Eqns, fabs(SubdivTol), NumericTol);

	if (IRIT_FABS(NumericTol) < SubdivTol && !_MVGlblETCnvrtBezier2MVs) {
	    /* Apply a numeric improvement stage now. */
            RetPts = MvarEqnsZeroNumeric(RetPts, Eqns, NULL, 0, NumericTol);
        }
    }

    if (IRIT_FABS(NumericTol) < SubdivTol) {
        /* Filter out points that fail on the inequality constraints or     */
        /* identical points in the zero set.			 	    */
        RetPts = MvarExprTreeEqnsZeroFilterIdenticalSet(RetPts, Eqns,
						       sqrt(fabs(NumericTol)));
    }

    /* Sort the result based on first axis. */
    RetPts = MvarPtSortListAxis(RetPts, 1);

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function - recursive subdivision of Eqns to do the real work.    *
*   Computes the simultaneous solution of the given set of expression tree   *
* constraints.  A constraints can be equality or inequality as prescribed by *
* the Constraints vector.		 				     *
*   All multivariates are assumed to be scalar and be in the same parametric *
* domain size and dimension.						     *
*   This procedure performs a preliminary subdivision until the geometry is  *
* found simple enough (I.e. no internal knots), when it invokes an           *
*									     *
* additional subdivision process, depending on the flow flags.               *
* PARAMETERS:                                                                *
*   Eqns:         Equations/constraints with common expression collected.    *
*   NumOfMVETs:   Size of the MVs expresion trees and Constraints vectors.   *
*   NumOfZeroMVETs: Number of zero constraints in Eqns & constraints vector. *
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        *
*		  measured in the parametric space of the multivariates.     *
*   NumericTol:   Numeric tolerance of the numeric stage.  The numeric stage *
*		  is employed only when _MVGlblETCnvrtBezier2MVs TRUE.       *
*                 In such a case the Eqns are converted to MVs and processed.*
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  *
*		      points will be the same as the dimensions of all MVs.  *
*****************************************************************************/
static MvarPtStruct *MvarExprTreeEqnsZerosAux(MvarExprTreeEqnsStruct *Eqns,
					      CagdRType SubdivTol,
					      CagdRType NumericTol)
{
    int i, Dir;
    CagdRType Knot;
    MvarExprTreeEqnsStruct *Eqns1, *Eqns2;
    MvarPtStruct *MVPts1, *MVPts2, *MVPts;

    /* Should we purge this subdomain? */
    if (!MvarExprTreeEqnsIsHolding(Eqns))
        return NULL;

    /* Subdiv recursively until no knots.*/
    for (i = 0; i < Eqns -> NumEqns; i++) {
        if ((Dir = MvarExprTreeInteriorKnots(Eqns -> Eqns[i], &Knot)) >= 0)
	    break;
    }

    if (i < Eqns -> NumEqns) {
        /* i'th Eqn has an interior knot in direction Dir - subdivide. */
        MvarExprTreeEqnsSubdivAtParam(Eqns, Knot, Dir, &Eqns1, &Eqns2);

	MVPts1 = MvarExprTreeEqnsZerosAux(Eqns1, SubdivTol, NumericTol);
	MVPts2 = MvarExprTreeEqnsZerosAux(Eqns2, SubdivTol, NumericTol);
	MvarExprTreeEqnsFree(Eqns1);
	MvarExprTreeEqnsFree(Eqns2);

	MVPts = CagdListAppend(MVPts1, MVPts2);
    }
    else if (_MVGlblETCnvrtBezier2MVs) {
        /* Call function that converts to MVs and solves the MVs.*/
        MVPts = MvarExprTreeEqnsZeroByMVs(Eqns, SubdivTol, NumericTol);
    }
    else {
        /* Continue subdivision until sufficiently small subdomain.*/
        MVPts = MvarExprTreeEqnsZeroAux2(Eqns, SubdivTol);
    }

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function to continue subdividing Eqns until reaching a	     *
* sufficiently small subdomain.                                              *
*   Computes the simultaneous solution of the given set of NumOfMVETs        *
* expression tree constraints.  A constraints can be equality or inequality  *
* as prescribed by the Constraints vector.				     *
*   All multivariates are assumed to be scalar Bezier and be in the same     *
* parametric domain size and dimension.					     *
*   This procedure performs the subdivision recursively until the geometry   *
* is found small enough enough (I.e. to with SubdivTol), when it invokes     *
* MvarMVsZeros.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Eqns:         The MVETs constraints formated into equations with         *
*                 common expressions.					     *
*   Constraints:  Either an equality or an inequality type of constraint.    *
*   NumOfMVETs:   Size of the MVs expresion trees and Constraints vectors.   *
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        *
*		  measured in the parametric space of the multivariates.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  *
*		      points will be the same as the dimensions of all MVs.  *
*****************************************************************************/
static MvarPtStruct *MvarExprTreeEqnsZeroAux2(MvarExprTreeEqnsStruct *Eqns,
					      CagdRType SubdivTol)
{
    int i, j, Dir;
    CagdRType Min, Max;
    MvarPtStruct *MVPts1, *MVPts2, *MVPts;

    /* Should we purge this subdomain? */
    if (!MvarExprTreeEqnsIsHolding(Eqns))
        return NULL;

    /* Find largest direction. */
    Dir = -1;
    Max = Min = 0.0;
    for (i = 0; i < Eqns -> Eqns[0] -> Dim; i++) {
	CagdBType HasDmn;
	CagdRType MinCurr, MaxCurr;

	for (j = 0; j < Eqns -> NumEqns; j++)
	    if ((HasDmn = MvarExprTreeDomain(Eqns -> Eqns[j],
					     &MinCurr, &MaxCurr, i)) == TRUE)
	        break;
	assert(HasDmn);

        if (MaxCurr - MinCurr > Max - Min) {
	    Dir = i;
	    Min = MinCurr;
	    Max = MaxCurr;
        }
    }

    if (Max - Min < SubdivTol) {
        /* Domain is small enough. */
	MVPts = MvarExprTreeEqnsMidPt(Eqns -> Eqns, Eqns -> NumEqns, FALSE);
    }
    else {
	MvarExprTreeEqnsStruct *Eqns1, *Eqns2;

	/* Perform normal cone test */
	if (!MvarExprTreeConesOverlap(Eqns))
	    return MvarExprTreeEqnsMidPt(Eqns -> Eqns, Eqns -> NumEqns, TRUE);

	/* Subdivide in the largest direction. */
        MvarExprTreeEqnsSubdivAtParam(Eqns, 0.5 * (Max + Min), Dir,
				      &Eqns1, &Eqns2);

	MVPts1 = MvarExprTreeEqnsZeroAux2(Eqns1, SubdivTol);
	MVPts2 = MvarExprTreeEqnsZeroAux2(Eqns2, SubdivTol);

	MvarExprTreeEqnsFree(Eqns1);
	MvarExprTreeEqnsFree(Eqns2);

	MVPts = CagdListAppend(MVPts1, MVPts2);
    }

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the given equations to multivariates and solve using MVs zeros. *
*                                                                            *
* PARAMETERS:                                                                *
*   Eqns:         Equations/constraints with only MVs.			     *
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        *
*		  measured in the parametric space of the multivariates.     *
*   NumericTol:   Numeric tolerance of the numeric stage.  The numeric stage *
*		  is employed only if IRIT_FABS(NumericTol) < SubdivTol.     *
*		  If NumericTol is negative, points that fail to improve     *
*		  numerically are purged away.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  *
*		      points will be the same as the dimensions of all MVs.  *
*****************************************************************************/
static MvarPtStruct *MvarExprTreeEqnsZeroByMVs(MvarExprTreeEqnsStruct *Eqns,
					       CagdRType SubdivTol,
					       CagdRType NumericTol)
{
    int i,
        NumOfMVs = Eqns -> NumEqns;
    MvarPtStruct *MVPts;
    MvarMVStruct
	**MVs = (MvarMVStruct **)
			       IritMalloc(sizeof(MvarMVStruct *) * NumOfMVs);

    /* Convert to a regular MV problem and solve. */
    for (i = 0; i < NumOfMVs; i++)
        MVs[i] = MvarExprTreeToMV(Eqns -> Eqns[i]);
    MvarUpdateConstDegDomains(MVs, NumOfMVs);

    /* Do we have a call back test function?  If so call it. */
    if (_MVGlblZeroETSubdivCallBackFunc != NULL &&
	_MVGlblZeroETSubdivCallBackFunc(&MVs, &Eqns -> ConstraintTypes,
					&NumOfMVs, &NumOfMVs, 0)) {
        for (i = 0; i < NumOfMVs; i++)
	    MvarMVFree(MVs[i]);
	IritFree(MVs);

	return NULL;
    }

    MVPts = MvarMVsZeros(MVs, Eqns -> ConstraintTypes, NumOfMVs,
			 SubdivTol, NumericTol);

    for (i = 0; i < NumOfMVs; i++)
        MvarMVFree(MVs[i]);
    IritFree(MVs);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the L1 error of the current position.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Eqns:        The ETs to evaluate error for.		                     *
*   Params:     The location where the error is to be evaluated.             *
*   NumOfMVs:   Number of multivariates we have.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     Computed error.                                           *
*****************************************************************************/
static CagdRType MvarExprTreeEqnsEvalErrorL1(const MvarExprTreeEqnsStruct
					                                *Eqns,
					     CagdRType *Params)
{
    int i;
    CagdRType
	Err = 0.0;

    for (i = 0; i < Eqns -> NumEqns; i++) {
	CagdRType
	    *R = MvarExprTreeEval(Eqns -> Eqns[i], Params);

	Err += IRIT_FABS(R[1]);
    }

    return Err;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the L1 error of the current position.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:        The multivariates to evaluate error for.                     *
*   Params:     The location where the error is to be evaluated.             *
*   NumOfMVs:   Number of multivariates we have.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     Computed error.                                           *
*****************************************************************************/
static CagdRType MvarMVEvalErrorL1(MvarMVStruct * const *MVs,
				   CagdRType *Params,
				   int NumOfMVs)
{
    int i;
    CagdRType
	Err = 0.0;

    for (i = 0; i < NumOfMVs; i++) {
	CagdRType
	    *R = MvarMVEval(MVs[i], Params);

	assert(!MVAR_IS_RATIONAL_MV(MVs[i]));

	Err += IRIT_FABS(R[1]);
    }

    return Err;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply a numerical improvement stage, as a first order minimization       M
* procedure of gradient computation and marching, in place.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   ZeroSet:	  Set of approximated solution, derived from a subdivision   M
*		  process, to improve in place.				     M
*   Eqns:         The constraints are given as Equations, if not NULL.       M
*   MVs:          Alternatively, the constraints are given as MVS.           M
*   NumMVs:       If MVs is not NULL, this specifies size of the MVs vector. M
*   NumericTol:   Tolerance of the numerical process.  Tolerance is measured M
*		  in the deviation of the scalar multivariates from their    M
*		  equality. Inequalities are ignored here.  If NumericTol is M
*		  negative, points that fail to improve numerically are      M
*		  purged away.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  M
*		      points will be the same as the dimensions of all MVs.  M
*		      Points that failed to improve are purged away.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarExprTreesZeros, MvarExprTreeEqnsZeros                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarEqnsZeroNumeric                                                      M
*****************************************************************************/
MvarPtStruct *MvarEqnsZeroNumeric(MvarPtStruct *ZeroSet,
				  const MvarExprTreeEqnsStruct *Eqns,
				  MvarMVStruct * const *MVs,
				  int NumMVs,
				  CagdRType NumericTol)
{
    MvarPtStruct
	*ImprovedSet = NULL;
    CagdBType
	PurgeFailure = NumericTol < 0.0;
    int i, j, NumEqns, Dim;
    CagdRType *A, *x, *x2, *b, *MinDmn, *MaxDmn;

    NumericTol = IRIT_FABS(NumericTol);

    if (Eqns != NULL) {
	NumEqns = Eqns -> NumZeroEqns,
        Dim = Eqns -> Eqns[0] -> Dim;
    }
    else {
        assert(MVs != NULL);
	NumEqns = NumMVs;
	Dim = MVs[0] -> Dim;
    }

    /* Make sure this is not an over-constrained system of equations. */
    if (Dim < NumEqns || NumEqns == 0)
	return ZeroSet;

    A = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim * NumEqns);
    x = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
    x2 = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
    b = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
    MinDmn = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
    MaxDmn = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);

    /* Get the domain of the constraints. */
    if (Eqns != NULL) {
        for (j = 0; j < Dim; j++) {
	    for (i = 0; i < NumEqns; i++) {
	        if (MvarExprTreeDomain(Eqns -> Eqns[i],
				       &MinDmn[j], &MaxDmn[j], j))
		    break;
	    }
	    assert(i < NumEqns);
	}
    }
    else
        MvarMVDomain(MVs[0], MinDmn, MaxDmn, -1);

    while (ZeroSet != NULL) {
	int Count = 0,
	    GoodMoves = 0;
	MvarPtStruct *Pt;
	CagdRType NewRngErr, NewDmnErr,
	    DmnErr = IRIT_INFNTY,
	    RngErr = IRIT_INFNTY,
	    RangeDiff = 0.0,
	    StepSize = 1.0;

	IRIT_LIST_POP(Pt, ZeroSet);

	do {
#	    ifdef DEBUG
	    {
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugMvarZeroErr) {
		    if (DmnErr == IRIT_INFNTY && RngErr == IRIT_INFNTY)
		        IRIT_INFO_MSG("\n******************************\n");
		    printf("\nRngErr=%.7g, DmnErr=%.7g, StpSz=%.7g at:\n\t",
			   RngErr, DmnErr, StepSize);
		    for (i = 0; i < Dim; i++)
		        printf(" %.7g", Pt -> Pt[i]);
		    printf("\nEquations:\n");
		}
	    }
#	    endif /* DEBUG */

	    /* We now seek a point that is on each tangent hyperplanes.      */
	    /* Hence, we have Dim linear equations and Dim dofs.             */
	    for (i = 0; i < NumEqns; i++) {
		/* Derive tangent hyperplane for multivariate at current pos.*/
		MvarPlaneStruct
		    *Pln = Eqns != NULL ?
			   MvarExprTreeEvalTanPlane(Eqns -> Eqns[i], Pt -> Pt) :
			   MvarMVEvalTanPlane(MVs[i], Pt -> Pt);

		/* Copy the constraint into the matrix form. */
		CAGD_GEN_COPY(&A[i * Dim], Pln -> Pln,
			      sizeof(CagdRType) * Dim);
		b[i] = -Pln -> Pln[Dim + 1];

#	        ifdef DEBUG
	        {
	            IRIT_IF_DEBUG_ON_PARAMETER(_DebugMvarZeroErr) {
		        for (j = 0; j < Dim; j++)
		            printf(" %.7g", Pln -> Pln[j]);
		        printf(" = %.7g\n", b[i]);
		    }
	        }
#	        endif /* DEBUG */

		/* Free the computed plane. */
		MvarPlaneFree(Pln);
	    }

	    /* Solve, possibly under constrained, system of lin. equations. */
	    if (IritQRUnderdetermined(A, NULL, NULL, NumEqns, Dim))
		break;

	    /* Add current position to b vector, seeking relative solution. */
	    for (i = 0; i < NumEqns; i++) {
		int j;
		CagdRType
		    t = 0.0;

		for (j = 0; j < Dim; j++)
		    t += A[i * Dim + j] * Pt -> Pt[j];

		b[i] -= t;
	    }

	    /* Solve for the new multivariate(s) coefficients. */
	    IritQRUnderdetermined(NULL, x, b, NumEqns, Dim);

	    /* And add the current location back, getting absolute result. */
	    for (i = 0; i < Dim; i++)
		x[i] = Pt -> Pt[i] + x[i] * StepSize;

#	    ifdef DEBUG
	    {
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugMvarZeroErr) {
		    printf("\tNew solution: ");
		    for (i = 0; i < Dim; i++)
		        printf(" %f", x[i]);
		    printf("\n");
		}
	    }
#	    endif /* DEBUG */

	    for (i = 0; i < Dim; i++) {
	        /* Make sure we stay within our domain bounds. */
		x[i] = IRIT_BOUND(x[i], MinDmn[i], MaxDmn[i]);
	    }

	    /* Now the million dollar question - is x a better solution!? */
	    NewRngErr = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumEqns, x);

#ifdef MVAR_ZERO_QUAD_SOLVER
	    /* If convergence is slowed down, try a quadratic step. */
	    if (NewRngErr != 0.0 && RngErr / NewRngErr < 2.0) {
	        int NumSols;
	        CagdRType NewRngErr2, NewRngErr3, A, B, C, Max, Sols[2];

		for (i = 0; i < Dim; i++)
		    x2[i] = (x[i] + Pt -> Pt[i]) * 0.5;
		NewRngErr2 = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumEqns, x2);

		/* Build a quadratic fit through Pt(t=0), x2(t=1/2), x(t=1)  */
		/* As "At^2 + Bt + C = 0".				     */
		A = 2 * RngErr - 4 * NewRngErr2 + 2 * NewRngErr;
		B = -3 * RngErr + 4 * NewRngErr2 - NewRngErr;
		C = RngErr;
		Max = IRIT_MAX(IRIT_MAX(IRIT_FABS(A), IRIT_FABS(B)),
			       IRIT_FABS(C));

		if (IRIT_APX_EQ(A / Max, 0) && IRIT_APX_EQ(A / Max, 0)) {
		    /* Examine the constant equation case. */
		    if (IRIT_APX_EQ(B / Max, 0))
		        NumSols = -1;    /* Use the solution found before... */
		    else {
		        /* Actually we have a linear function here... */
		        Sols[0] = -C / B;
			NumSols = 1;
		    }
		}
		else {
		    B /= A;
		    C /= A;

		    NumSols = GMSolveQuadraticEqn(B, C, Sols);
		}

		if (NumSols == 2) {
		    int j;
		    CagdRType Vals[2];

		    for (j = 0; j < 2; j++) {
		        for (i = 0; i < Dim; i++) {
			    x2[i] = Pt -> Pt[i] +
			            (x[i] - Pt -> Pt[i]) * Sols[j];
			    x2[i] = IRIT_BOUND(x2[i], MinDmn[i], MaxDmn[i]);
			}

			Vals[j] = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs,
							 NumEqns, x2);
		    }
		    j = Vals[0] > Vals[1];
		    if (Vals[j] < NewRngErr) {
		        /* We are improving with this quadratic step. */
		        for (i = 0; i < Dim; i++) {
			    x[i] = Pt -> Pt[i] +
			           (x[i] - Pt -> Pt[i]) * Sols[j];
			    x[i] = IRIT_BOUND(x[i], MinDmn[i], MaxDmn[i]);
			}
		        NewRngErr = Vals[j];
		    }
		}
		else {
		    if (NumSols == 0) {
		        /* Find minimum of parabola as the solution. */
		        Sols[0] = -B / 2;
			NumSols = 1;
		    }

		    /* So now we typically have one solution: */
		    if (NumSols == 1) {
		        for (i = 0; i < Dim; i++) {
			    x2[i] = Pt -> Pt[i] +
			           (x[i] - Pt -> Pt[i]) * Sols[0];
			    x2[i] = IRIT_BOUND(x2[i], MinDmn[i], MaxDmn[i]);
			}
		    }

		    /* Compute the new location's error. If middle location */
		    /* has smaller error, use that instead.		    */
		    NewRngErr3 = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumEqns, x2);
		    if (NewRngErr > NewRngErr2 && NewRngErr > NewRngErr3) {
			for (i = 0; i < Dim; i++)
			    x[i] = x2[i];
			NewRngErr = NewRngErr2;
		    }		        
		}
	    }
#endif /* MVAR_ZERO_QUAD_SOLVER */

	    /* Estimate the domain's error as the difference between last   */
	    /* parametric location and this one.			    */
	    NewDmnErr = 0.0;
	    for (i = 0; i < Dim; i++)
	        NewDmnErr += IRIT_FABS(Pt -> Pt[i] - x[i]);

#	    ifdef DEBUG
	    {
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugMvarZeroErr) {
		    IRIT_INFO_MSG_PRINTF(
			    "Rng Err = %.7g (%.7g), Dmn Err = %.7g (%.7g)",
			    NewRngErr, RngErr, NewDmnErr, DmnErr);
		    for (i = 0; i < Dim; i++)
		        printf(" %.7g", x[i]);
		    printf("\n");
		}
	    }
#	    endif /* DEBUG */

	    if (NewRngErr < RngErr || NewRngErr == 0.0 || NewDmnErr == 0.0) {
	        RangeDiff = IRIT_FABS(RngErr - NewRngErr);
		RngErr = NewRngErr;
		DmnErr = NewDmnErr;
		CAGD_GEN_COPY(Pt -> Pt, x, sizeof(CagdRType) * Dim);

		/* Increase step size if we have a sequence of good moves. */
		if (GoodMoves++ > 5) {
		    StepSize = IRIT_MAX(1.0, StepSize * 2.0);
		    GoodMoves = 0;
		}
	    }
	    else {
		if (StepSize < 0.5) {
		    /* Find a nearby point - perturb the solution pt. */
		    if (MvarEqnsPerturbZero(Pt, RngErr, Eqns, MVs, NumMVs,
					    MinDmn, MaxDmn, RngErr * StepSize))
		        StepSize *= 2.0;
		    else
			StepSize *= 0.5;
		}
		else {
		    StepSize *= 0.5;
		    GoodMoves = 0;
		}
	    }

	    /* When to terminate is a difficult question.  We clearly desire */
	    /* that the RngErr will be below the NumericTol but we also      */
	    /* desire low DmnErr (in tangency intersections it might be much */
	    /* larger than the RngErr), so for the 1st few iterations ask    */
	    /* for small DmnErr as well.				     */
	}
	while ((Count++ < 10 && DmnErr > NumericTol) ||
	       (RngErr > NumericTol &&
	        DmnErr > 0.0 &&
	        Count < MVAR_NUMER_ZERO_NUM_STEPS));

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMvarNumerZeroIters, FALSE) {
	        IRIT_INFO_MSG_PRINTF(
		    "\tNumeric step %ssuccessful after %d iters (%.8g %.8g)\n",
		    Count >= MVAR_NUMER_ZERO_NUM_STEPS ? "un" : "",
		    Count, DmnErr, RngErr);
;	    }
	}
#	endif /* DEBUG */

	if (PurgeFailure && RngErr > NumericTol) {
	    MvarPtFree(Pt);
	}
	else {
	    AttrSetRealAttrib(&Pt -> Attr, "RngError", RngErr);
	    IRIT_LIST_PUSH(Pt, ImprovedSet);
	}
    }

    IritFree(A);
    IritFree(x);
    IritFree(x2);
    IritFree(b);
    IritFree(MinDmn);
    IritFree(MaxDmn);

    return ImprovedSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Look for a near by location that improves the solution.  This naive      *
* scheme is employed when the regular NR approach failed.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:           To try and locally improve, in place.                      *
*   StartRngErr:  Current range error of parameteric location Pt.            *
*   Eqns:         The constraints are given as Equations, if not NULL.       *
*   MVs:          Alternatively, the constraints are given as MVS.           *
*   NumEqns:      Size of theEqns or  MVs vector.			     *
*   MinDmn, MaxDmn:  Domain of constraints.				     *
*   Step:         To take in perturbation.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int MvarEqnsPerturbZero(MvarPtStruct *Pt,
			       CagdRType StartRngErr,
			       const MvarExprTreeEqnsStruct *Eqns,
			       MvarMVStruct * const *MVs,
			       int NumEqns,
			       CagdRType *MinDmn,
			       CagdRType *MaxDmn,
			       CagdRType Step)
{
    int i, n,
        Dim = Eqns != NULL ? Dim = Eqns -> Eqns[0] -> Dim : MVs[0] -> Dim;
    CagdRType RngErr,
        x[MVAR_MAX_PT_SIZE];

    CAGD_GEN_COPY(x, Pt -> Pt, sizeof(CagdRType) * Dim);

    for (n = 0; n < 3; n++) {
        Step *= 0.5;

        /* Try +/- perturbations of Step size, in all Dim directions: */
	for (i = 0; i < Dim; i++) {
	    x[i] += Step;
	    if (x[i] > MaxDmn[i])
		x[i] = MaxDmn[i];
	    if ((RngErr = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumEqns, x))
	                                                       > StartRngErr) {
	        x[i] -= Step * 2;
		if (x[i] < MinDmn[i])
		    x[i] = MinDmn[i];
		RngErr = MVAR_EVAL_NUMER_ERR_L1(Eqns, MVs, NumEqns, x);
	    }

	    if (RngErr < StartRngErr) {
		CAGD_GEN_COPY(Pt -> Pt, x, sizeof(CagdRType) * Dim);
		return TRUE;
	    }
	    else
	        x[i] = Pt -> Pt[i];
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a point of the dimension as the given MVET in the middle of    M
* its parametric domain.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVETs:      To construct a point in the middle of their domain.          M
*   NumOfMVETs: Size of MVETs vector.					     M
*   SingleSol:  If TRUE, this point is a single solution in MV domain.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  The construct point in the middle of the MV.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeEqnsZero                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsMidPt		                                     M
*****************************************************************************/
MvarPtStruct *MvarExprTreeEqnsMidPt(MvarExprTreeStruct * const *MVETs,
				    int NumOfMVETs,
				    int SingleSol)
{
    int l, HasDmn;
    MvarPtStruct
        *Pt = MvarPtNew(MVETs[0] -> Dim);

    for (l = 0; l < MVETs[0] -> Dim; l++) {
        int j;
	CagdRType Min, Max;

	for (j = 0; j < NumOfMVETs; j++)
	    if ((HasDmn = MvarExprTreeDomain(MVETs[j], &Min, &Max, l)) == TRUE)
	        break;
	assert(HasDmn);

	Pt -> Pt[l] = (Min + Max) * 0.5;
    }

#ifdef DEBUG_DUMP_DOMAINS
    fprintf(GlblDumpDomainsFile,
	    "[OBJECT [RGB \"255,100,100\"] [Gray 0.5] [Width 0.02] NONE [CTLPT E%d",
	    MVETs[0] -> Dim);
    for (l = 0; l < MVETs[0] -> Dim; l++)
	fprintf(GlblDumpDomainsFile, " %f", Pt -> Pt[l]);
    fprintf(GlblDumpDomainsFile, "]\n]\n");
#endif /* DEBUG_DUMP_DOMAINS */

    AttrSetIntAttrib(&Pt -> Attr, "SingleSol", SingleSol);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Filters out identical points or points that fail the inequality          M
* constraints.                                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   ZeroSet:      The solution points to filter out.                         M
*   Eqns:         Multivariate ET constraints.                               M
*   Tol:          Tolerance to consider to points same in L^1 norm.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Filtered solution points.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreesZeros, MvarExprTreeEqnsZeros                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEqnsZeroFilterIdenticalSet                                   M
*****************************************************************************/
MvarPtStruct *MvarExprTreeEqnsZeroFilterIdenticalSet(
					   MvarPtStruct *ZeroSet,
					   const MvarExprTreeEqnsStruct *Eqns,
					   CagdRType Tol)
{
    int i, l;
    MvarPtStruct *Pt,
	*OutSet = NULL;
    MvarExprTreeStruct
        **ETs = Eqns -> Eqns;
    int NumEqns = Eqns -> NumEqns,
	NumZeroEqns = Eqns -> NumZeroEqns;

    while (ZeroSet != NULL) {
        IRIT_LIST_POP(Pt, ZeroSet);

	if (AttrGetIntAttrib(Pt -> Attr, "Similar") == TRUE) {
	    MvarPtFree(Pt);
	}
	else {
	    MvarPtStruct *Pt2;
	    CagdBType
	        PurgePt = FALSE;

	    /* Lets see if we fail any inequality constraint. */
	    for (i = NumZeroEqns; i < NumEqns && !PurgePt; i++) {
                int NumOfCoord =  ETs[i] -> PtSize;
		CagdRType
                    *R = MvarExprTreeEval(ETs[i], Pt -> Pt);

		switch (Eqns -> ConstraintTypes[i]) {
		    case MVAR_CNSTRNT_POSITIVE:
			if (NumOfCoord > 1) { /* Union implementation. */
			    PurgePt = TRUE;
			    for (l = 1; l <= NumOfCoord; l++) {
			        if (R[l] >= 0.0) {
				    PurgePt = FALSE;
				    break;
				}
			    }
			}
			else {
			    if (R[1] < 0.0)
			        PurgePt = TRUE;
			}
			break;
		    case MVAR_CNSTRNT_NEGATIVE:
		        if (NumOfCoord > 1) { /* Union implementation. */
			    PurgePt = TRUE;
			    for (l = 1; l <= NumOfCoord; l++) {
			        if (R[l] <= 0.0) {
				    PurgePt = FALSE;
				    break;
				}
			    }
			}
			else {
			    if (R[1] > 0.0)
			        PurgePt = TRUE;
			}
			break;
	            default:
		        break;
		}
	    }

	    if (PurgePt) {
	        MvarPtFree(Pt);
	    }
	    else {
	        for (Pt2 = ZeroSet; Pt2 != NULL; Pt2 = Pt2 -> Pnext) {
		    for (i = 0; i < Pt -> Dim; i++) {
		        if (!IRIT_APX_EQ_EPS(Pt -> Pt[i], Pt2 -> Pt[i], Tol))
			    break;
		    }

		    if (i >= Pt -> Dim) {
		        /* Pt and Pt2 are same - mark Pt2 as similar. */
		        AttrSetIntAttrib(&Pt2 -> Attr, "Similar", TRUE);
		    }
		}

		IRIT_LIST_PUSH(Pt, OutSet);
	    }
	}
    }

    return OutSet;
}

#if defined(DEBUG_MVAR_PRINT_ET_INFO) || defined(DEBUG_MVAR_REPORT_COMMON_EXPR)

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Debug function to print a string to stdout.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   const char *:  String to print.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MVDbgPrintfStdout(const char *Str)
{
    printf(Str);
}

#endif /* DEBUG_MVAR_PRINT_ET_INFO || DEBUG_MVAR_REPORT_COMMON_EXPR*/
