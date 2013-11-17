/*****************************************************************************
*  Generic tools of interactive curve editing.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, Mar. 1998.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "cnstcrvs.h"

#define CRV_EDIT_UNDO_STACK_SIZE	50
#define CRV_EDIT_NUM_CIRC_SAMPLES	64
#define CRV_EDIT_TRIM_SRF_FINENESS	0.001
#define CRV_MR_UNCONSTRANT_SCALE	2.0
#define CRV_MR_REGION_WIDTH		3                      /* In pixels. */
#define CRV_EDIT_PLLN_FINENESS		0.001
#define CRV_CMP_DIST_SQR(Pt1, Pt2, i) { \
		IrtRType d = IRIT_PT_PT_DIST_SQR(Pt1, Pt2); \
		if (d < DistSqr) { \
		    DistSqr = d; \
		    Index = i; \
	        } }
#define CRV_EDIT_GET_SKTCH_SRF(Obj)	(IP_IS_SRF_OBJ(Obj) \
					         ? (Obj) -> U.Srfs \
						 : (Obj) -> U.TrimSrfs -> Srf)


typedef struct CrvEditUndoStackStruct {
    CagdCrvStruct *Crv, *UVCrv;
    IGCrvEditParamStruct CrvEditParam;
} CrvEditUndoStackStruct;

IRIT_STATIC_DATA int
    CEditUndoStackPushPtr = 0,  /* Also tells us the number of pushed items. */
    CEditUndoStackPopPtr = -1,  /* Shifts back and forth by the undo/redo's. */
    GlblSketchPtSize = 0,    /* Number of points in the list of stroked pts. */
    CEditPickedPointIndex = 0;  /* Number of picked points when n are neeed. */
IRIT_STATIC_DATA CagdRType
    CEditLastMRParameter = -IRIT_INFNTY;
IRIT_STATIC_DATA CagdPtStruct
    *GlblSketchPtList = NULL,
    *GlblSketchUVList = NULL;
IRIT_STATIC_DATA CrvEditUndoStackStruct
    CEditUndoStack[CRV_EDIT_UNDO_STACK_SIZE];
IRIT_STATIC_DATA CEditMultiResKvsStruct
    *CEditMultiResKvs = NULL;
IRIT_STATIC_DATA IPObjectStruct
    *CEditSrfSktchObj = NULL;
IRIT_STATIC_DATA VoidPtr
    *CEditSrfSktchRayPrep = NULL;

IRIT_GLOBAL_DATA IGCrvEditParamStruct
    IGCrvEditParam = {
        4,                                                   /* Curve order. */
        CAGD_CBSPLINE_TYPE,         /* Type of curve (Bspline, Bezier etc.). */
        CAGD_END_COND_OPEN, /* End conditions of the curve (Float, Open etc. */
        CAGD_UNIFORM_PARAM,                 /* KV spacing (parametrization). */

        FALSE,              /* Curve is rational (TRUE), polynomial (FALSE). */

        IG_CRV_EDIT_STATE_DETACH,                 /* State of curve editing. */

        IG_CRV_EDIT_LST_SQR_DEF_PERCENT,  /* Least squares ctlpt percentage. */

        FALSE,			/* No support for constraints to begin with. */
        FALSE,		 /* No abort if we fails to satisfy all constraints. */
	FALSE, FALSE, FALSE,                /* No X/Y/C symmetry constraint. */
	FALSE,			                /* No fixed area constraint. */
	100.0			  /* Maximum allowed solution's coefficient. */
    };

IRIT_GLOBAL_DATA char
    *IGCrvEditStateEntries[] = {
	"Primitive Curve",
	"Attach to Old Curve",
	"Clone Old Curve",
	"New Curve By New Ctl Pts",
	"New Curve Via a Sketch",
	"New Curve Via Sketch on Srf",
	"Manipulate/Edit Curve",
	"Detach Current Curve",
	NULL
    },
    *IGCrvEditSubdivEntries[] = {
	"Subdivide Pick First",
	"Subdivide Pick Second",
	"Insert C0 Continuity",
	"Insert C1 Continuity",
	NULL
    },
    *IGCrvEditRefineEntries[] = {
	"Refine One",
	"Refine All",
	"Refine Region",
	NULL
    },
    *IGCrvEditEndCondEntries[] = {
	"Open End Condition",
	"Float End Condition",
	"Periodic End Condition",
	NULL
    },
    *IGCrvEditParamEntries[] = {
	"General Parameterization",
	"Uniform Parameterization",
	"Centripetal Parameterization",
	"Chord Len Parameterization",
	NULL
    },
    *IGCrvEditEvalEntityEntries[] = {
	"Eval Parameter",
	"Eval Position",
	"Eval Tangent",
	"Eval Normal",
	NULL
    },
    *IGCrvEditPrimitivesEntries[] = {
	"Unit Circle",
	"Unit PCircle",
	NULL
    };
IRIT_GLOBAL_DATA int
    IGCrvEditActive = FALSE,
    IGCrvDrawOriginal = TRUE,
    IGCrvEditDrawMesh = TRUE,
    IGCrvEvalEntity = -1,
    IGCrvEditGrabMouse = FALSE;
IRIT_GLOBAL_DATA IrtRType
    IGCrvEditMRLevel = 0.0;
IRIT_GLOBAL_DATA IGCrvEditEventType
    IGCEditOperation = IG_CRV_EDIT_EVENT_STATE;
IRIT_GLOBAL_DATA CagdCrvStruct
    *IGCrvOriginalCurve = NULL,
    *IGCrvEditCurrentCrv = NULL,
    *IGCrvEditCurrentUVCrv = NULL;
IRIT_GLOBAL_DATA CagdSrfStruct
    *IGCrvEditSktchSrf = NULL;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGCrvEditPreloadEditCurveObj = NULL,
    *IGCrvEditCurrentObj = NULL;

static int CEditDelPointAux(CagdCrvStruct **Crv, int Index);
static CagdCrvStruct *EvaluateEucCrvFromUVCrv(CagdCrvStruct *UVCrv);
static IPPolygonStruct *EvaluateEucPolyFromUVCrv(CagdCrvStruct *Crv);
static void CEditUpdateKnotVectorAux(CagdCrvStruct *Crv,
				     CagdEndConditionType EndCond,
				     CagdParametrizationType Param);
static CagdRType **CEditBasisInnerProdMat(CagdRType *KV,
					  int Len,
					  int Order,
					  int Periodic);
static IPObjectStruct *CEditMultiResViewRegion(CagdCrvStruct *Crv,
					       CEditMultiResKvsStruct *MultiKvs);
static void CEditMultiResUpdateOneCrv(CagdCrvStruct *Crv,
				      CEditMultiResOneKvStruct *Kvs,
				      CagdRType t,
				      IrtVecType TransDir);
static void CEditUpdateMultResCrvDecomp(void);
static void CEditHandleMouseStateEdit(int x, int y, int Event);
static void CEditConstrainedMoveCtlPt(CagdCrvStruct *Crv,
				      int CtlPtIndex,
				      IrtPtType OldPos,
				      IrtPtType NewPos);
static void CEditSetState(CagdCrvStruct *Crv, IGCrvEditParamStruct *State);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pushes the current curve editing state on the Undo/Redo Stack.           M
* The implementation below is geared for simplicity, and not efficiency that M
* is not an issue here!							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:            Current editted curve.                                   M
*   UVCrv:	    The UV curve on the sketched surface, if any.	     M
*   CrvEditParam:   Current Curve editing parameters.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if successful.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditUndoState, CEditRedoState, CEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditPushState                                                           M
*****************************************************************************/
int CEditPushState(CagdCrvStruct *Crv,
		   CagdCrvStruct *UVCrv,
		   IGCrvEditParamStruct *CrvEditParam)
{
    if (Crv == NULL)
	return FALSE;

    /* Make sure new pushed result is different than last time. */
    if (CEditUndoStackPushPtr > 0 &&
	CagdCrvsSame(CEditUndoStack[CEditUndoStackPushPtr - 1].Crv, Crv,
		     IRIT_EPS) &&
	(UVCrv == NULL ||
	 CEditUndoStack[CEditUndoStackPushPtr - 1].UVCrv == NULL ||
	 CagdCrvsSame(CEditUndoStack[CEditUndoStackPushPtr - 1].UVCrv,
		      UVCrv, IRIT_EPS)) &&
	IRIT_GEN_CMP(&CEditUndoStack[CEditUndoStackPushPtr - 1].CrvEditParam,
		     CrvEditParam, sizeof(IGCrvEditParamStruct)) == 0)
        return FALSE;

    /* Make sure we have place in the undo stack. */
    if (CEditUndoStackPushPtr >= CRV_EDIT_UNDO_STACK_SIZE) {
	int i;

	CEditUndoStackPushPtr = CRV_EDIT_UNDO_STACK_SIZE - 1;

	/* Shift all stack one element back. */
	CagdCrvFree(CEditUndoStack[0].Crv);
	for (i = 1; i < CRV_EDIT_UNDO_STACK_SIZE; i++) {
	    CEditUndoStack[i - 1] = CEditUndoStack[i];
	}
    }
    
    /* And push the new state. */
    CEditUndoStack[CEditUndoStackPushPtr].Crv = CagdCrvCopy(Crv);
    CEditUndoStack[CEditUndoStackPushPtr].UVCrv =
				UVCrv == NULL ? NULL : CagdCrvCopy(UVCrv);
    CEditUndoStack[CEditUndoStackPushPtr++].CrvEditParam = *CrvEditParam;

    /* Update the undo/redo pointer to the end of the stack. */
    CEditUndoStackPopPtr = CEditUndoStackPushPtr;
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Undo one step in the undo stack, updating CrvEditParam and returning a   M
* pointer to the old curve.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvEditParam:  To update with the old state of the curve editing params. M
*   UVCrv:	   The UV curve on the sketched surface, if any.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The old curve (the object must be copied before use.) M
*	      NULL is returned if no undo to use any more (bottom of stack). M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditPushState, CEditRedoState, CEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditUndoState                                                           M
*****************************************************************************/
CagdCrvStruct *CEditUndoState(IGCrvEditParamStruct *CrvEditParam,
			      CagdCrvStruct **UVCrv)
{
    if (CEditUndoStackPopPtr > 0) {
	*CrvEditParam = CEditUndoStack[--CEditUndoStackPopPtr].CrvEditParam;
	*UVCrv = CEditUndoStack[CEditUndoStackPopPtr].UVCrv;
	return CEditUndoStack[CEditUndoStackPopPtr].Crv;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redo one step in the undo stack, updating CrvEditParam and returning a   M
* pointer to the old curve.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvEditParam:  To update with the old state of the curve editing params. M
*   UVCrv:	   The UV curve on the sketched surface, if any.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The old curve (the object must be copied before use.) M
*	      NULL is returned if no redo to use any more (bottom of stack). M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditPushState, CEditRedoState, CEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditRedoState                                                           M
*****************************************************************************/
CagdCrvStruct *CEditRedoState(IGCrvEditParamStruct *CrvEditParam,
			      CagdCrvStruct **UVCrv)
{
    if (CEditUndoStackPopPtr < CEditUndoStackPushPtr) {
	*CrvEditParam = CEditUndoStack[CEditUndoStackPopPtr].CrvEditParam;
	*UVCrv = CEditUndoStack[CEditUndoStackPopPtr].UVCrv;
	return CEditUndoStack[CEditUndoStackPopPtr++].Crv;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees and reset the undo/redo stack.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditUndoState, CEditRedoState, CEditPushState                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditFreeStateStack                                                      M
*****************************************************************************/
void CEditFreeStateStack(void)
{
    int i;

    for (i = 0; i < CEditUndoStackPushPtr; i++) {
	CagdCrvFree(CEditUndoStack[i].Crv);
	if (CEditUndoStack[i].UVCrv != NULL)
	    CagdCrvFree(CEditUndoStack[i].UVCrv);
    }

    CEditUndoStackPushPtr = 0;
    CEditUndoStackPopPtr = -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds another point at the given index Index, in place.		     M
* A new knot vector is created to following IGCrvEditParam guidlines.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and move one of its control points.                 M
*   Index:      Index of control point to insert.  Negative index denotes    M
*               before the first and index larger than length of curve would M
*               insert the new point as last.				     M
*   NewPos:     Position of new control point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditRefineCrv, CEditDomainFromCrv, CEditSubdivCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditAddPoint                                                            M
*****************************************************************************/
int CEditAddPoint(CagdCrvStruct **Crv, int Index, CagdPType NewPos)
{
    int i, PtSize;
    CagdCrvStruct
	*OrigCrv = *Crv;
    CagdRType **Points,
	**OrigPoints = OrigCrv -> Points;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(OrigCrv);

    *Crv = CagdPeriodicCrvNew(OrigCrv -> GType, OrigCrv -> PType,
			      OrigCrv -> Length + 1,
			      OrigCrv -> GType == CAGD_CBSPLINE_TYPE &&
			           OrigCrv -> Periodic);
    (*Crv) -> Order = OrigCrv -> Order;
    Points = (*Crv) -> Points;

    PtSize = IRIT_MIN(CAGD_NUM_OF_PT_COORD(OrigCrv -> PType), 3);

    /* Keep the bounds tight. */
    if (Index < -1)
	Index = -1;
    else if (Index > OrigCrv -> Length)
	Index = OrigCrv -> Length;

    /* Copy the new point into index (Index + 1). */
    if (IsRational)
	Points[0][IRIT_MAX(0, Index)] = 1.0;      /* Place a neutral weight. */
    for (i = 0; i < PtSize; i++)
	Points[i + 1][IRIT_MAX(0, Index)] = NewPos[i];

    if (Index < 0) {
	/* Copy the rest of the points (new point first). */
	for (i = !IsRational; i < PtSize; i++) {
	    CAGD_GEN_COPY(&Points[i][1],
			  &OrigPoints[i][0],
			  sizeof(CagdRType) * OrigCrv -> Length);
	}
    }
    else if (Index >= OrigCrv -> Length) {
	/* Copy the rest of the points (new point last). */
	for (i = !IsRational; i <= PtSize; i++) {
	    CAGD_GEN_COPY(&Points[i][0],
			  &OrigPoints[i][0],
			  sizeof(CagdRType) * OrigCrv -> Length);
	}
    }
    else {
	Index++;             /* Go from [-1,..,OrigLength] to [0,..,Length]. */

	/* Copy the rest of the points (new point in the middle). */
	for (i = !IsRational; i <= PtSize; i++) {
	    CAGD_GEN_COPY(&Points[i][0],
			  &OrigPoints[i][0],
			  sizeof(CagdRType) * Index);
	    CAGD_GEN_COPY(&Points[i][Index + 1],
			  &OrigPoints[i][Index],
			  sizeof(CagdRType) * (OrigCrv -> Length - Index));
	}
    }

    /* Update the knot vector following guidelines of curve edit parameters. */
    if (CAGD_IS_BSPLINE_CRV(*Crv)) {
	if (IGCrvEditParam.ParamType == CAGD_GENERAL_PARAM)
	    IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;

	CEditUpdateKnotVector(*Crv, IGCrvEditParam.EndCond,
			      IGCrvEditParam.ParamType);
    }

    CagdCrvFree(OrigCrv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deletes one control point at the given index Index, in place.	     M
* A new knot vector is created to following IGCrvEditParam guidlines.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and delete one of its control points.               M
*   Index:      Index of control point to delete.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditRefineCrv, CEditDomainFromCrv, CEditSubdivCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditDelPoint                                                            M
*****************************************************************************/
int CEditDelPoint(CagdCrvStruct **Crv, int Index)
{
    if (IGCrvEditCurrentUVCrv != NULL) {
	return CEditDelPointAux(Crv, Index) &&
	       CEditDelPointAux(&IGCrvEditCurrentUVCrv, Index);
    }
    else
        return CEditDelPointAux(Crv, Index);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function CEditDelPoint                               *
*****************************************************************************/
static int CEditDelPointAux(CagdCrvStruct **Crv, int Index)
{
    int i, PtSize;
    CagdCrvStruct
	*OrigCrv = *Crv;
    CagdRType **Points,
	**OrigPoints = OrigCrv -> Points;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(OrigCrv);

    if (Index < 0 || Index >= (*Crv) -> Length)
        return FALSE;

    *Crv = CagdPeriodicCrvNew(OrigCrv -> GType, OrigCrv -> PType,
			      OrigCrv -> Length - 1,
			      OrigCrv -> GType == CAGD_CBSPLINE_TYPE &&
			           OrigCrv -> Periodic);
    (*Crv) -> Order = OrigCrv -> Order;
    Points = (*Crv) -> Points;

    PtSize = IRIT_MIN(CAGD_NUM_OF_PT_COORD(OrigCrv -> PType), 3);

    /* Copy the points, ignoring point index Index. */
    for (i = !IsRational; i <= PtSize; i++) {
        if (Index > 0)
	    CAGD_GEN_COPY(&Points[i][0], &OrigPoints[i][0],
			  sizeof(CagdRType) * Index);
        if (Index < OrigCrv -> Length - 1)
	    CAGD_GEN_COPY(&Points[i][Index], &OrigPoints[i][Index + 1],
			  sizeof(CagdRType) * (OrigCrv -> Length - Index - 1));
    }

    if ((*Crv) -> Length < (*Crv) -> Order) {
        IGCrvEditParam.Order = (*Crv) -> Order = (*Crv) -> Length;

	/* Update knot vector following guidelines of curve edit parameters. */
	if (CAGD_IS_BSPLINE_CRV(*Crv)) {
	    if (IGCrvEditParam.ParamType == CAGD_GENERAL_PARAM)
		IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;

	    CEditUpdateKnotVector(*Crv, IGCrvEditParam.EndCond,
				  IGCrvEditParam.ParamType);
	}

	IGCrvEditParamUpdateWidget();
    }
    else {
	/* Update knot vector following guidelines of curve edit parameters. */
	if (CAGD_IS_BSPLINE_CRV(*Crv)) {
	    if (IGCrvEditParam.ParamType == CAGD_GENERAL_PARAM)
		IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;

	    CEditUpdateKnotVector(*Crv, IGCrvEditParam.EndCond,
				  IGCrvEditParam.ParamType);
	}
    }

    CagdCrvFree(OrigCrv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Move/drag one control point at the given index Index, in place.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and move one of its control points.                 M
*   Index:      Index of control point to drag.				     M
*   ModifyWeight:  If TRUE, modify the weight of the control point.          M
*   MousePt, MouseDir: Point and direction of current mouse line to move to. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditRefineCrv, CEditDomainFromCrv, CEditSubdivCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditMovePoint                                                           M
*****************************************************************************/
int CEditMovePoint(CagdCrvStruct *Crv,
		   int Index,
		   int ModifyWeight,
		   IrtPtType MousePt,
		   IrtVecType MouseDir)
{
    int i,
	CrvPtSize = IRIT_BOUND(CAGD_NUM_OF_PT_COORD(IGCrvEditCurrentCrv -> PType),
			       2, 3);
    CagdUVType UV;
    IrtPtType CtlPt, ClosestPt;
    CagdRType
	**Points = Crv -> Points;

    CtlPt[2] = 0.0;

    if (CEditSrfSktchRayPrep != NULL) {      /* It is a curve on a surface. */
	CagdRType
	    **UVPoints = IGCrvEditCurrentUVCrv -> Points;
	int Len = Crv -> Length;

	if (ModifyWeight) {
	    IGIritError("CEditMovePoint Error: No support for rational curves on surfaces.");
	}
	else {
	    CagdCrvStruct *TCrv;

	    if (!IntrSrfHierarchyTestRay(CEditSrfSktchRayPrep,
					 MousePt, MouseDir, UV))
	        return FALSE;

	    if (IGCrvEditParam.SupportConstraints) {

		for (i = 1; i <= 2; i++)              /* E2 - UV coordinate. */
		    CtlPt[i - 1] = UVPoints[i][Index];

		IRIT_UV_COPY(ClosestPt, UV);
		ClosestPt[2] = 0.0;

		TCrv = CagdCrvCopy(IGCrvEditCurrentUVCrv);  /* Keep a copy. */
		CEditConstrainedMoveCtlPt(IGCrvEditCurrentUVCrv, Index,
					  CtlPt, ClosestPt);
		UVPoints = IGCrvEditCurrentUVCrv -> Points;
		for (i = 0; i < IGCrvEditCurrentUVCrv -> Length; i++) {
		    if (UVPoints[1][i] < 0.0 || UVPoints[1][i] > 1.0 ||
			UVPoints[2][i] < 0.0 || UVPoints[2][i] > 1.0)
		        break;
		}

		/* If constraint's solution is off srf domain, ignore it. */
		if (IGCrvEditParam.AbortIfCnstFailed &&
		    i < IGCrvEditCurrentUVCrv -> Length) {
		    CagdCrvFree(IGCrvEditCurrentUVCrv);
		    IGCrvEditCurrentUVCrv = TCrv;
		}
		else
		    CagdCrvFree(TCrv);
	    }
	    else {
		for (i = 1; i <= 2; i++)
		    UVPoints[i][Index] = UV[i - 1];
	    }

	    /* Evaluate the Euclidean curve out of the UV curve. */
	    TCrv = EvaluateEucCrvFromUVCrv(IGCrvEditCurrentUVCrv);

	    /* Copy the points to the given curve. */
	    if (Len != TCrv -> Length)
	        IGIritError("CEditMovePoint Error: Curve length mismatch");
	    for (i = 1; i <= CrvPtSize; i++)
	        IRIT_GEN_COPY(Points[i], TCrv -> Points[i],
			 sizeof(CagdRType) * Len);

	    CagdCrvFree(TCrv);
	}
    }
    else {
	if (ModifyWeight) {
	    /* Need to update the weight of point. */
	    for (i = 1; i <= CrvPtSize; i++) {
		Points[i][Index] /= Points[0][Index];

		CtlPt[i - 1] = Points[i][Index];
	    }

	    Points[0][Index] = GMDistPointLine(CtlPt, MousePt, MouseDir) /
						   IG_CRV_RATIONAL_CIRC_LENGTH;

	    for (i = 1; i <= CrvPtSize; i++)
	        Points[i][Index] *= Points[0][Index];
	}
	else {
	    if (CAGD_IS_RATIONAL_CRV(Crv)) {
		for (i = 1; i <= CrvPtSize; i++)
		    CtlPt[i - 1] =
		        Points[i][Index] / Points[0][Index];

		GMPointFromPointLine(CtlPt, MousePt, MouseDir, ClosestPt);
				    
		for (i = 1; i <= CrvPtSize; i++)
		    Points[i][Index] = Points[0][Index] * ClosestPt[i - 1];
	    }
	    else {
		for (i = 1; i <= CrvPtSize; i++)
		    CtlPt[i - 1] = Points[i][Index];

		GMPointFromPointLine(CtlPt, MousePt, MouseDir, ClosestPt);

		if (IGCrvEditParam.SupportConstraints) {
		    CEditConstrainedMoveCtlPt(Crv, Index, CtlPt, ClosestPt);
		}
		else {
		    for (i = 1; i <= CrvPtSize; i++)
		        Points[i][Index] = ClosestPt[i - 1];
		}
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Move/drag one control point at the given index Index, in place.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:                        To treat and move one of its control points. M
*   CurveParam:                   Parameter value of dragged point on curve. M
*   MouseStartPt, MouseStartDir:  Starting drag point/direction in R3.       M
*   MousePt, MouseDir: Point and direction of current mouse line to move to. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditRefineCrv, CEditDomainFromCrv, CEditSubdivCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditModifyCurve                                                         M
*****************************************************************************/
int CEditModifyCurve(CagdCrvStruct *Crv,
		     IrtRType CurveParam,
		     IrtPtType MouseStartPt,
		     IrtVecType MouseStartDir,
		     IrtPtType MousePt,
		     IrtVecType MouseDir)
{
    if (CEditSrfSktchRayPrep != NULL) {      /* It is a curve on a surface. */
	CagdCrvStruct *TCrv;
	CagdUVType UVStart, UV;
	IrtVecType TransDir;

	if (!IntrSrfHierarchyTestRay(CEditSrfSktchRayPrep,
				     MouseStartPt, MouseStartDir, UVStart) ||
	    !IntrSrfHierarchyTestRay(CEditSrfSktchRayPrep,
				     MousePt, MouseDir, UV))
	    return FALSE;
	TransDir[0] = UV[0] - UVStart[0];
	TransDir[1] = UV[1] - UVStart[1];
	TransDir[2] = 0.0;
	if (IRIT_PT_APX_EQ_ZERO_EPS(TransDir, IRIT_EPS))
	    return TRUE;
	IRIT_PT_COPY(MouseStartPt, MousePt);
	IRIT_VEC_COPY(MouseStartDir, MouseDir);

	TCrv = CEditMultiResModify(IGCrvEditCurrentUVCrv,
				   IGCrvEditCurrentUVCrv -> Order,
				   CEditMultiResKvs,
				   IGCrvEditMRLevel *
				   (CEditMultiResKvs -> NumKvs - 1),
				   CurveParam, TransDir);
	CagdCrvFree(IGCrvEditCurrentUVCrv);
	IGCrvEditCurrentUVCrv = TCrv;

	/* Evaluate the Euclidean curve out of the UV curve. */
	IGCrvEditCurrentCrv = EvaluateEucCrvFromUVCrv(IGCrvEditCurrentUVCrv);
    }
    else {
	IrtPtType ClosestPt;
	IrtVecType TransDir;
	CagdCrvStruct *TCrv;

	GMPointFromPointLine(MouseStartPt, MousePt, MouseDir, ClosestPt);
	IRIT_PT_SUB(TransDir, ClosestPt, MouseStartPt);

	if (CAGD_NUM_OF_PT_COORD(IGCrvEditCurrentCrv -> PType) == 2)
	    TransDir[2] = 0.0;
	if (IRIT_PT_APX_EQ_ZERO_EPS(TransDir, IRIT_EPS))
	    return TRUE;
	IRIT_PT_COPY(MouseStartPt, ClosestPt);

	TCrv = CEditMultiResModify(IGCrvEditCurrentCrv,
				   IGCrvEditCurrentCrv -> Order,
				   CEditMultiResKvs,
				   IGCrvEditMRLevel *
				   (CEditMultiResKvs -> NumKvs - 1),
				   CurveParam, TransDir);
	CagdCrvFree(IGCrvEditCurrentCrv);
	IGCrvEditCurrentCrv = TCrv;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Revaluate the Euclidean curve on surface from its parametric domain crv. *
*                                                                            *
* PARAMETERS:                                                                *
*   UVCrv:	Curve in parametric domain of surface we sketched on.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct:  The evaluated curve in Euclidean space.                  *
*****************************************************************************/
static CagdCrvStruct *EvaluateEucCrvFromUVCrv(CagdCrvStruct *UVCrv)
{
    int i;
    CagdRType **Points, **UVPoints;
    CagdCrvStruct *Crv;

    if (UVCrv == NULL)
        return NULL;

    Crv = CagdCoerceCrvTo(UVCrv, CAGD_PT_E3_TYPE, FALSE);

    Points = Crv -> Points;
    UVPoints = UVCrv -> Points;
    for (i = 0; i < Crv -> Length; i++) {
	CagdPType PtE3;
	CagdRType *R;

	UVPoints[1][i] = IRIT_BOUND(UVPoints[1][i], 0.0, 1.0);
	UVPoints[2][i] = IRIT_BOUND(UVPoints[2][i], 0.0, 1.0);

	R = CagdSrfEval(IGCrvEditSktchSrf, UVPoints[1][i], UVPoints[2][i]);

	CagdCoerceToE3(PtE3, &R, -1, IGCrvEditSktchSrf -> PType);
	Points[1][i] = PtE3[0];
	Points[2][i] = PtE3[1];
	Points[3][i] = PtE3[2];
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Revaluate the Euclidean polyline on surface from its parametric domain   *
* curve if a curve on surface, or directly from the E3 curve otherwise.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:	Curve in parametric domain of surface we sketched on, or an  *
*		E3 curve in Euclidean space.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct:  The evaluated polyline in Euclidean space.	     *
*****************************************************************************/
static IPPolygonStruct *EvaluateEucPolyFromUVCrv(CagdCrvStruct *Crv)
{
    IPPolygonStruct *Pl;

    if (IGCrvEditCurrentUVCrv != NULL) {
	IPVertexStruct *V;

	Pl = IPCurve2Polylines(Crv, IGGlblPllnFineness,
			       IGGlblPolylineOptiApprox);
	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    CagdRType *R;

	    V -> Coord[0] = IRIT_BOUND(V -> Coord[0], 0.0, 1.0);
	    V -> Coord[1] = IRIT_BOUND(V -> Coord[1], 0.0, 1.0);

	    R = CagdSrfEval(IGCrvEditSktchSrf, V -> Coord[0], V -> Coord[1]);

	    CagdCoerceToE3(V -> Coord, &R, -1, IGCrvEditSktchSrf -> PType);
	}
    }
    else {
	Pl = IPCurve2Polylines(Crv, IGGlblPllnFineness,
			       IGGlblPolylineOptiApprox);
    }

    return Pl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the knot vector of the given curve to follow the EndCond end      M
* condition and Param parametrization prescription.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To update.  The knot vector of the curve my be NULL and a     M
*	       new knot vector will be allocated for it in that case.        M
*   EndCond:   End condition to employ.                                      M
*   Param:     Parametrization to use.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditUpdateKnotVector                                                    M
*****************************************************************************/
void CEditUpdateKnotVector(CagdCrvStruct *Crv,
			   CagdEndConditionType EndCond,
			   CagdParametrizationType Param)
{
    CEditUpdateKnotVectorAux(Crv, EndCond, Param);

    if (IGCrvEditCurrentUVCrv != NULL)
        CEditUpdateKnotVectorAux(IGCrvEditCurrentUVCrv, EndCond, Param);
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function to function CEditUpdateKnotVector.			     *
*****************************************************************************/
static void CEditUpdateKnotVectorAux(CagdCrvStruct *Crv,
				     CagdEndConditionType EndCond,
				     CagdParametrizationType Param)
{
    int i,
    	Order = Crv -> Order,
	Length = Crv -> Length;
    CagdRType *Lens, Sum, *Kv, TMin, TMax,
	**Points = Crv -> Points;
    CagdPType PrevPt, Pt;
    CagdPointType
	PType = Crv -> PType;

    if (Crv -> GType != CAGD_CBSPLINE_TYPE)
	return;

    if (Crv -> KnotVector == NULL) {
        Crv -> KnotVector = BspKnotUniformOpen(Crv -> Length, Crv -> Order,
					       NULL);
    }

    CagdCrvDomain(Crv, &TMin, &TMax);

    if (Param != CAGD_GENERAL_PARAM) {
	if (Crv -> KnotVector != NULL) {
	    IritFree(Crv -> KnotVector);
	    Crv -> KnotVector = NULL;
	}

        switch (EndCond) {
	    default:
	    case CAGD_END_COND_OPEN:
		Crv -> KnotVector = BspKnotUniformOpen(Crv -> Length,
						       Crv -> Order, NULL);
		break;
	    case CAGD_END_COND_FLOAT:
		Crv -> KnotVector = BspKnotUniformFloat(Crv -> Length,
							Crv -> Order, NULL);
		break;
	    case CAGD_END_COND_PERIODIC:
		Crv -> KnotVector = BspKnotUniformPeriodic(Crv -> Length,
							   Crv -> Order, NULL);
		break;
	}
    }
    Kv = Crv -> KnotVector;

    switch (Param) {
	default:
	case CAGD_GENERAL_PARAM:
	case CAGD_UNIFORM_PARAM:
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	case CAGD_CHORD_LEN_PARAM:
	    /* Compute the lengths of all the segments. */
	    Lens = IritMalloc(sizeof(CagdRType) * Length);
	    CagdCoerceToE3(PrevPt, Points, 0, PType);
	    for (i = 1; i < Length; i++) {
		CagdCoerceToE3(Pt, Points, i, PType);
		Lens[i - 1] = IRIT_PT_PT_DIST(PrevPt, Pt);
		IRIT_PT_COPY(PrevPt, Pt);
	    }
	    Lens[i - 1] = 0.0;

	    /* Update the knots based on these lengths. */
	    for (i = 0, Sum = 0.0; i < Order - 1; i++)
		Sum += Lens[i];
	    for (i = Order; i <= Length; i++) {
		Kv[i] = Kv[i - 1] +
			  ((Param == CAGD_CHORD_LEN_PARAM) ? Sum : sqrt(Sum));
		Sum += Lens[i - 1] - Lens[i - Order];
	    }

	    switch (EndCond) {
	        default:
	        case CAGD_END_COND_OPEN:
		    for (i = Length + 1; i < Length + Order; i++)
			Kv[i] = Kv[Length];
		    break;
		case CAGD_END_COND_FLOAT:
		    for (i = Length + 1; i < Length + Order; i++)
			Kv[i] = Kv[Length] + i - Length;
		    break;
		case CAGD_END_COND_PERIODIC:
		    for (i = Length + 1;
			 i < Length + Order + Order - 1;
			 i++)
			Kv[i] = Kv[i - 1] + (Kv[i - Length] -
					     Kv[i - Length - 1]);
		    break;
	    }
	    break;
    }

    Crv -> Periodic = EndCond == CAGD_END_COND_PERIODIC;

#ifdef CRV_DO_ZERO_ONE_DOMAIN
    BspKnotAffineTransOrder2(Crv -> KnotVector,
			     Crv -> Order,
			     CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order,
			     0.0, 1.0);
#else
    BspKnotAffineTransOrder2(Crv -> KnotVector,
			     Crv -> Order,
			     CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order,
			     TMin, TMax);
#endif /* CRV_DO_ZERO_ONE_DOMAIN */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset and initialize a new sketch of a new curve.  A sketch is a         M
* sequence of XY locations generated from the input device such as a mouse.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditSketchGetStrokedPoly, CEditSketchGetCurve, CEditSketchAddPoint      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSketchCrvReset                                                      M
*****************************************************************************/
void CEditSketchCrvReset(void)
{
    if (GlblSketchPtList != NULL)
	CagdPtFreeList(GlblSketchPtList);
    if (GlblSketchUVList != NULL)
	CagdPtFreeList(GlblSketchUVList);
    GlblSketchPtList = NULL;
    GlblSketchUVList = NULL;
    GlblSketchPtSize = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new location at the end of the currently stroked curve.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       Location of new point of stroked curve.                        M
*   UV:	      If sketch on a surface, holds the UV location, otherwise NULL. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditSketchGetStrokedPoly, CEditSketchGetCurve, CEditSketchCrvReset      M
*   CEditMergeCurves							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSketchAddPoint                                                      M
*****************************************************************************/
void CEditSketchAddPoint(CagdPType Pt, CagdRType *UV)
{
    CagdPtStruct
	*P = CagdPtNew();

    IRIT_PT_COPY(P -> Pt, Pt);
    IRIT_LIST_PUSH(P, GlblSketchPtList);
    GlblSketchPtSize++;

    if (UV != NULL) {
        CagdPtStruct
	    *PUV = CagdPtNew();

	IRIT_UV_COPY(PUV -> Pt, UV);
	PUV -> Pt[2] = 0.0;
	IRIT_LIST_PUSH(PUV, GlblSketchUVList);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the curve computed using least squares over the stroked points.     M
* Note this curve is parameterized in reverse from the end to the beginning. M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The curve computed using least squares.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditSketchGetStrokedPoly, CEditSketchAddPoint, CEditSketchCrvReset      M
*   CEditSketchGetUVCurve						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSketchGetCurve                                                      M
*****************************************************************************/
CagdCrvStruct *CEditSketchGetCurve(void)
{
    CagdCrvStruct *Crv;

    if (GlblSketchPtSize < IGCrvEditParam.Order)
	return NULL;

    if (GlblSketchUVList != NULL) {
	CagdCrvStruct
	    *UVCrv = CEditSketchGetUVCurve();

	Crv = EvaluateEucCrvFromUVCrv(UVCrv);
	CagdCrvFree(UVCrv);
    }
    else {
	Crv = BspCrvInterpPts(GlblSketchPtList,
			      IGCrvEditParam.Order,
			      IRIT_MAX(IGCrvEditParam.Order,
				  (int) (GlblSketchPtSize *
				     IGCrvEditParam.LstSqrPercent / 100.0)),
			      IGCrvEditParam.ParamType,
			      IGCrvEditParam.EndCond == CAGD_END_COND_PERIODIC);

	if (IGCrvEditParam.Rational) {
	    int i;
 
	    Crv -> Points[0] = (CagdRType *)
	        IritMalloc(Crv -> Length * sizeof(CagdRType));
	    for (i = 0; i < Crv -> Length; i++)
	        Crv -> Points[0][i] = 1.0;
	    Crv -> PType = CAGD_MAKE_PT_TYPE(1,
					 CAGD_NUM_OF_PT_COORD(Crv -> PType));
	}
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the curve computed using least squares over the stroked points.     M
* Note this curve is parameterized in reverse from the end to the beginning. M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The curve computed using least squares.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditSketchGetStrokedPoly, CEditSketchAddPoint, CEditSketchCrvReset      M
*   CEditSketchGetCurve							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSketchGetUVCurve                                                    M
*****************************************************************************/
CagdCrvStruct *CEditSketchGetUVCurve(void)
{
    CagdCrvStruct *Crv;

    if (GlblSketchPtSize < IGCrvEditParam.Order)
	return NULL;

    Crv = BspCrvInterpPts(GlblSketchUVList,
			  IGCrvEditParam.Order,
			  IRIT_MAX(IGCrvEditParam.Order,
			      (int) (GlblSketchPtSize *
				     IGCrvEditParam.LstSqrPercent / 100.0)),
			  IGCrvEditParam.ParamType,
			  IGCrvEditParam.EndCond == CAGD_END_COND_PERIODIC);

    if (IGCrvEditParam.Rational) {
	int i;
 
	Crv -> Points[0] = (CagdRType *)
	    IritMalloc(Crv -> Length * sizeof(CagdRType));
	for (i = 0; i < Crv -> Length; i++)
	    Crv -> Points[0][i] = 1.0;
	Crv -> PType = CAGD_MAKE_PT_TYPE(1,
					 CAGD_NUM_OF_PT_COORD(Crv -> PType));
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the control polygon as derived by the stroked points.               M
* This polygon is parameterized from the beginning to the end.               M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   The polygon computed using least squares.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditSketchGetCurve, CEditSketchAddPoint, CEditSketchCrvReset            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSketchGetStrokedPoly                                                M
*****************************************************************************/
IPPolygonStruct *CEditSketchGetStrokedPoly(void)
{
    IPVertexStruct
        *VHead = NULL;
    CagdPtStruct *P;

    for (P = GlblSketchPtList; P != NULL; P = P -> Pnext) {
	VHead = IPAllocVertex2(VHead);
	IRIT_PT_COPY(VHead -> Coord, P -> Pt);
    }

    return IPAllocPolygon(0, VHead, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Refine the curve at the given t value, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and refine at t.		                     M
*   t:          The parameter(s) at which to insert the new knot(s).         M
*   n:          Size of vector t - number of new knots.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditAddPoint, CEditDomainFromCrv, CEditSubdivCrv, CEditMergeCurves	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditRefineCrv                                                           M
*****************************************************************************/
int CEditRefineCrv(CagdCrvStruct **Crv, CagdRType *t, int n)
{
    int i;
    CagdRType TMin, TMax;
    CagdCrvStruct *TCrv,
	*OrigCrv = *Crv;
    
    CagdCrvDomain(OrigCrv, &TMin, &TMax);
    for (i = 0; i < n; i++)
        t[i] = IRIT_BOUND(t[i], TMin, TMax);

    if (IGCrvEditCurrentUVCrv != NULL) {
	TCrv = CagdCrvRefineAtParams(IGCrvEditCurrentUVCrv, FALSE, t, n);
	CagdCrvFree(IGCrvEditCurrentUVCrv);
	IGCrvEditCurrentUVCrv = TCrv;

	*Crv = EvaluateEucCrvFromUVCrv(IGCrvEditCurrentUVCrv);
    }
    else {
	*Crv = CagdCrvRefineAtParams(OrigCrv, FALSE, t, n);
    }
    CagdCrvFree(OrigCrv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivide the curve at the given t value, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and subdivide.  Returned is a list of two curves    M
*		that are the result of the subdivision.			     M
*   t:          The parameter at which to subdivide.		             M
*   Continuity: Continuity of subdivided point.  Note that if continuity is  M
*               non negative, the curves are not split.	 If -1 it is a hint  M
*		that first portion is to be extracted, if -2, the second.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditAddPoint, CEditRefineCrv, CEditDomainFromCrv, CEditMergeCurves      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditSubdivCrv                                                           M
*****************************************************************************/
int CEditSubdivCrv(CagdCrvStruct **Crv, CagdRType t, int Continuity)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *TCrv,
	*OrigCrv = *Crv;
    
    CagdCrvDomain(OrigCrv, &TMin, &TMax);
    if (t <= TMin || t >= TMax)
	return FALSE;

    if (Continuity < 0) {
	if (IGCrvEditCurrentUVCrv != NULL) {
	    TCrv = CagdCrvSubdivAtParam(IGCrvEditCurrentUVCrv, t);
	    CagdCrvFree(IGCrvEditCurrentUVCrv);
	    if (Continuity == -1) {
		IGCrvEditCurrentUVCrv = TCrv;
		CagdCrvFree(TCrv -> Pnext);
	    }
	    else {
		IGCrvEditCurrentUVCrv = TCrv -> Pnext;
		CagdCrvFree(TCrv);
	    }
	    IGCrvEditCurrentUVCrv -> Pnext = NULL;

	    CagdCrvFree(*Crv);
	    *Crv = EvaluateEucCrvFromUVCrv(IGCrvEditCurrentUVCrv);
	}
	else {
	    *Crv = CagdCrvSubdivAtParam(OrigCrv, t);
	    CagdCrvFree(OrigCrv);
	}
    }
    else {
	int i,
	    Mult = (*Crv) -> Order - Continuity - 1;
	CagdRType *TVec;

	if (Mult > 0) {
	    TVec = (CagdRType *) IritMalloc(Mult * sizeof(CagdRType));

	    for (i = 0; i < Mult; i++)
	        TVec[i] = t;

	    CEditRefineCrv(Crv, TVec, Mult);

	    IritFree(TVec);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merge Crv2 to Crv1 and update Crv1 in place.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:      First curve of the merge.  Result is updated here in place.   M
*   Crv2:      Second curve of the merge.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditAddPoint, CEditRefineCrv, CEditDomainFromCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditMergeCrvs                                                           M
*****************************************************************************/
int CEditMergeCrvs(CagdCrvStruct **Crv1, CagdCrvStruct *Crv2)
{
    int Index = 0;
    CagdRType *R, TMin, TMax, DistSqr;
    CagdPType P1Start, P1End, P2Start, P2End;
    CagdCrvStruct *TCrv1, *TCrv2;

    CagdCrvDomain(*Crv1, &TMin, &TMax);
    R = CagdCrvEval(*Crv1, TMin);
    CagdCoerceToE3(P1Start, &R, -1, (*Crv1) -> PType);
    R = CagdCrvEval(*Crv1, TMax);
    CagdCoerceToE3(P1End, &R, -1, (*Crv1) -> PType);

    CagdCrvDomain(Crv2, &TMin, &TMax);
    R = CagdCrvEval(Crv2, TMin);
    CagdCoerceToE3(P2Start, &R, -1, Crv2 -> PType);
    R = CagdCrvEval(Crv2, TMax);
    CagdCoerceToE3(P2End, &R, -1, Crv2 -> PType);

    DistSqr = IRIT_INFNTY;
    CRV_CMP_DIST_SQR(P1Start, P2Start, 0);
    CRV_CMP_DIST_SQR(P1Start, P2End,   1);
    CRV_CMP_DIST_SQR(P1End,   P2Start, 2);
    CRV_CMP_DIST_SQR(P1End,   P2End,   3);

    switch (Index) {
	default:
	case 0: /* P1Start to P2Start. */
	    TCrv1 = CagdCrvReverse(*Crv1);
	    CagdCrvFree(*Crv1);
	    *Crv1 = CagdMergeCrvCrv(TCrv1, Crv2, TRUE);
	    CagdCrvFree(TCrv1);
	    break;
	case 1: /* P1Start to P2End. */
	    TCrv1 = CagdCrvReverse(*Crv1);
	    TCrv2 = CagdCrvReverse(Crv2);
	    CagdCrvFree(*Crv1);
	    *Crv1 = CagdMergeCrvCrv(TCrv1, TCrv2, TRUE);
	    CagdCrvFree(TCrv1);
	    CagdCrvFree(TCrv2);
	    break;
	case 2: /* P1End to P2Start. */
	    TCrv1 = CagdMergeCrvCrv(*Crv1, Crv2, TRUE);
	    CagdCrvFree(*Crv1);
	    *Crv1 = TCrv1;
	    break;
	case 3: /* P1End to P2End. */
	    TCrv2 = CagdCrvReverse(Crv2);
	    TCrv1 = CagdMergeCrvCrv(*Crv1, TCrv2, TRUE);
	    CagdCrvFree(*Crv1);
	    *Crv1 = TCrv1;
	    CagdCrvFree(TCrv2);
	    break;
    }

    if (IGCrvEditCurrentUVCrv != NULL) {
	CagdCrvFree(IGCrvEditCurrentUVCrv);
	IGCrvEditCurrentUVCrv = NULL;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivide the curve at the given t value, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To treat and subdivide.  Returned is the curve restricted to M
*		the domain from t1 to t2.                 		     M
*   t1, t2:     The parameters of the domain to extract.	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditAddPoint, CEditRefineCrv, CEditSubdivCrv			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditDomainFromCrv                                                       M
*****************************************************************************/
int CEditDomainFromCrv(CagdCrvStruct **Crv, CagdRType t1, CagdRType t2)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *TCrv,
	*OrigCrv = *Crv;
    
    CagdCrvDomain(OrigCrv, &TMin, &TMax);
    if (t1 <= TMin || t1 >= TMax || t2 <= TMin || t2 >= TMax || IRIT_APX_EQ(t1, t2))
	return FALSE;

    *Crv = CagdCrvRegionFromCrv(OrigCrv, t1, t2);
    CagdCrvFree(OrigCrv);

    if (IGCrvEditCurrentUVCrv != NULL) {
	TCrv = CagdCrvRegionFromCrv(IGCrvEditCurrentUVCrv, t1, t2);
	CagdCrvFree(IGCrvEditCurrentUVCrv);
	IGCrvEditCurrentUVCrv = TCrv;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the t parameter value of the closest point on Crv to Pt.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To find the parameter value of the closest point to Pt         M
*   MousePt:  To find the closest point on Crv to - a point on ray.          M
*   MouseDir: To find the closest point on Crv to - direction of ray.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Parameter value of closest point on Crv to Pt,             M
*		  or -IRIT_INFNTY if none found.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditFindClosestParameter                                                M
*****************************************************************************/
CagdRType CEditFindClosestParameter(CagdCrvStruct *Crv,
				    CagdPType MousePt,
				    CagdPType MouseDir)
{
    IPPolygonStruct *MinPl, *Poly;
    IrtRType RetVal;
    IrtPtType MinPt;

    AttrSetIntAttrib(&Crv -> Attr, "SaveParamVals", TRUE);
    Poly = IPCurve2Polylines(Crv,
			     IGGlblPllnFineness < 0.01 ? IGGlblPllnFineness
			                               : CRV_EDIT_PLLN_FINENESS,
			     SYMB_CRV_APPROX_TOLERANCE);
    AttrFreeOneAttribute(&Crv -> Attr, "SaveParamVals");

    if (UserMinDistLinePolylineList(MousePt, MouseDir, Poly, FALSE, &MinPl,
				    MinPt, &RetVal)
		      > IGGlblMinPickDist / MatScaleFactorMatrix(IPViewMat))
	RetVal = -IRIT_INFNTY;
    
    IPFreePolygon(Poly);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the index of the closest control point on Crv to Pt.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To find the index of the control closest point to Pt           M
*   MousePt:  To find closest control point of Crv - a point on mouse ray.   M
*   MouseDir: To find closest control point of Crv - direction of mouse ray. M
*   ModifyWeight:  Found a circle (of weight of a ctlpt) in that index.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Index of closest control point on Crv to Pt,		     M
*	     or -1 if nothing was picked.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditFindClosestControlPoint                                             M
*****************************************************************************/
int CEditFindClosestControlPoint(CagdCrvStruct *Crv,
				 CagdPType MousePt,
				 CagdPType MouseDir,
				 int *ModifyWeight)
{
    int i, j,
        Index = -1,
        WeightIndex = -1,
	CrvPtSize = IRIT_BOUND(CAGD_NUM_OF_PT_COORD(Crv -> PType), 2, 3);
    CagdRType Dist, DistWeight,
	MinDist = IGGlblMinPickDist / MatScaleFactorMatrix(IPViewMat),
	MinWeight = MinDist,
        **Points = Crv -> Points;

    *ModifyWeight = FALSE;

    Crv = CagdCrvMatTransform(Crv, IPViewMat);

    for (i = 0; i < Crv -> Length; i++) {
	CagdPType CrvCtlPt;

	if (CAGD_IS_RATIONAL_CRV(Crv)) {
	    for (j = 0; j < CrvPtSize; j++)
		CrvCtlPt[j] = Points[j + 1][i] / Points[0][i];
	    if (CrvPtSize == 2)
		CrvCtlPt[2] = 0.0;
	}
	else {
	    for (j = 0; j < CrvPtSize; j++)
		CrvCtlPt[j] = Points[j + 1][i];
	    if (CrvPtSize == 2)
		CrvCtlPt[2] = 0.0;
	}
	
	Dist = GMDistPointLine(CrvCtlPt, MousePt, MouseDir);
	if (MinDist > Dist) {
	    MinDist = Dist;
	    Index = i;
	}

	if (CAGD_IS_RATIONAL_CRV(Crv)) {
	    DistWeight = IRIT_FABS(Dist - Points[0][i] *
						 IG_CRV_RATIONAL_CIRC_LENGTH);
	    if (MinWeight > DistWeight) {
		MinWeight = DistWeight;
		WeightIndex = i;
	    }
	}
    }

    CagdCrvFree(Crv);

    if (WeightIndex >= 0 && MinWeight < MinDist) {
	*ModifyWeight = TRUE;
	return WeightIndex;
    }
    else
	return Index;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the current constructed curve.  Invoked when screen is redrawn.     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstRedrawConstraints                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditRedrawCrv                                                           M
*****************************************************************************/
void CEditRedrawCrv(void)
{
    IPObjectStruct *PObj;
    CagdCrvStruct *Crv;
    IPPolygonStruct *Pl;
    IrtHmgnMatType IritView;

    switch (IGGlblViewMode) {
        case IG_VIEW_ORTHOGRAPHIC:
            IRIT_GEN_COPY(IritView, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IritView, IPViewMat, IPPrspMat);
	    break;
    }

    if (IGCrvEditCurrentCrv != NULL) {
	if (IGCrvEditCurrentCrv -> Length >= IGCrvEditCurrentCrv -> Order) {
	    PObj = IPAllocObject("", IP_OBJ_POLY, NULL);
	    IP_SET_POLYLINE_OBJ(PObj);
	    PObj -> U.Pl = EvaluateEucPolyFromUVCrv(IGCrvEditCurrentUVCrv ?
						    IGCrvEditCurrentUVCrv :
						    IGCrvEditCurrentCrv);
	    AttrSetObjectRGBColor(PObj,
				  IGGlblHighlight1Color[0],
				  IGGlblHighlight1Color[1],
				  IGGlblHighlight1Color[2]);
	    IGViewObject(PObj, IritView);
	    IPFreeObject(PObj);

	    if (IGCrvEditDrawMesh && IGCrvEditParam.Rational) {
		int i;
		IrtRType
		    **Points = IGCrvEditCurrentCrv -> Points,
		    Rad = IG_CRV_RATIONAL_CIRC_LENGTH,
		    *w = Points[0],
		    *x = Points[1],
		    *y = Points[2],
		    *z = Points[3];
		IrtHmgnMatType RotMat, RotInvMat;

		/* Extract only the rotation matrix and prepare its inverse: */
		MatRotateFactorMatrix(IPViewMat, RotMat);
		MatTranspMatrix(RotMat, RotInvMat);  /* Compute the inverse. */

		/* Draw all rational values as circles around the ctl pts. */
		for (i = 0; i < IGCrvEditCurrentCrv -> Length; i++, w++) {
		    IrtHmgnMatType TransMat, ScaleMat;

		    /* Move the circle to the control point's location. */
		    MatGenMatTrans(*x++ / *w,
				   *y++ / *w,
				   z ? *z++ / *w : 0.0, TransMat);
		    MatMultTwo4by4(TransMat, TransMat, IritView);

		    /* Apply the proper scale, following the weight's size. */
		    MatGenMatUnifScale(Rad * *w, ScaleMat);
		    MatMultTwo4by4(TransMat, ScaleMat, TransMat);
		
		    /* Apply the inverse rotation to the transformation. */
		    MatMultTwo4by4(TransMat, RotInvMat, TransMat);

		    IGViewObject(CEditGetUnitCircle(), TransMat);
		}
	    }
	}

	if (IGGlblDrawSurfaceMesh || IGCrvEditDrawMesh) {
	    PObj = IPGenPOLYObject(IPCurve2CtlPoly(IGCrvEditCurrentCrv));
	    IP_SET_POLYLINE_OBJ(PObj);
	    AttrSetObjectRGBColor(PObj,
				  IGGlblHighlight1Color[0],
				  IGGlblHighlight1Color[1],
				  IGGlblHighlight1Color[2]);
	    IGViewObject(PObj, IritView);
	    IPFreeObject(PObj);
	}
    }

    if ((Crv = CEditSketchGetCurve()) != NULL) {
	PObj = IPGenCRVObject(Crv);
	AttrSetObjectRGBColor(PObj,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
        IGViewObject(PObj, IritView);
	IPFreeObject(PObj);
    }

    if ((Pl = CEditSketchGetStrokedPoly()) != NULL) {
	PObj = IPGenPOLYObject(Pl);
	IP_SET_POLYLINE_OBJ(PObj);
	AttrSetObjectRGBColor(PObj,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
        IGViewObject(PObj, IritView);
	IPFreeObject(PObj);
    }

    /* Draw the multiresolution region, if necessary. */
    if ((PObj = CEditMultiResViewRegion(IGCrvEditCurrentCrv,
					CEditMultiResKvs)) != NULL) {
	AttrSetObjectRealAttrib(PObj, "dwidth", CRV_MR_REGION_WIDTH);
	AttrSetObjectRGBColor(PObj,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
        IGViewObject(PObj, IritView);
	IPFreeObject(PObj);
    }

    /* Draw the constraints, if we have them. */
    CCnstRedrawConstraints();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Decomposes the knot vector to a hierarchy of multi resolution levels.    M
* Updates the global data structures of CEditMRKvs			     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigKv:    Original/base knot vector to decomposed.		             M
*   OrigKVLen: Length of original knot vector.			             M
*   Order:     Order of freeform using this knot vector.	             M
*   Discont:   If TRUE, preserves the discontinuities.		             M
*   Periodic:  If TRUE, Performs a periodic decomposition.	             M
*   InnerProd: IF TRUE, compute inner products as well.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CEditMultiResKvsStruct *: Construct hierarchy of knot vectors.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditMultiResPrepKVs                                                     M
*****************************************************************************/
CEditMultiResKvsStruct *CEditMultiResPrepKVs(CagdRType *OrigKv,
					     int OrigKVLen,
					     int Order,
					     int Discont,
					     int Periodic,
					     int InnerProd)
{
    int i, j, l, PrevLen;
    CagdRType *KVPrev, *KVPtr;
    CEditMultiResKvsStruct *MultiResKvs;
    CEditMultiResOneKvStruct *Kvs;

    /* Compute the number of knot vectors in the decomposition and allocate. */
    for (i = 1, j = 1; i < OrigKVLen; i += i, j++);

    MultiResKvs = (CEditMultiResKvsStruct *)
	IritMalloc(sizeof(CEditMultiResKvsStruct));
    MultiResKvs -> Kvs = Kvs = (CEditMultiResOneKvStruct *)
	IritMalloc(sizeof(CEditMultiResOneKvStruct) * j);
    MultiResKvs -> Order = Order;
    Kvs[0].Kv = BspKnotCopy(NULL, OrigKv, OrigKVLen + Order);
    Kvs[0].KvLen = OrigKVLen + Order;
    if (InnerProd)
        Kvs[0].InnerProds = CEditBasisInnerProdMat(Kvs[0].Kv, Kvs[0].KvLen,
						   Order, Periodic);
    else
        Kvs[0].InnerProds = NULL;

#   ifdef COMP_AREA_FROM_INNER_PRODS
	if (IGCrvEditCurrentCrv != NULL) {
	    int i, j,
		Len = IGCrvEditCurrentCrv -> Length;
	    CagdRType
		Area = 0.0,
		**Points = IGCrvEditCurrentCrv -> Points;

	    for (i = 0; i < OrigKVLen; i++) {
		for (j = 0; j < OrigKVLen; j++) {
		    Area += Points[1][i % Len] *
			    Kvs[0].InnerProds[i][j] *
			    Points[2][j % Len];
		}
	    }

	    IRIT_INFO_MSG_PRINTF("Area, following inner prod coefs = %f\n",
		                  Area * 0.5);
	}
#   endif /* COMP_AREA_FROM_INNER_PRODS */

    for (KVPrev = OrigKv, PrevLen = OrigKVLen, i = 1; i < j; i++) {
	KVPtr = Kvs[i].Kv = (CagdRType *)
		           IritMalloc(sizeof(CagdRType) * (OrigKVLen + Order));

	/* Copy first Order knots verbatim. */
	Kvs[i].KvLen = 2 * Order;         /* End conditions copied verbatim. */
	for (l = 0; l < Order; l++)
	    *KVPtr++ = *KVPrev++;

	/* Skip every second interior knot. */
	for ( ; l < PrevLen; l++, KVPrev++) {
	    if ((l & 0x01) == 1 ||
		(Discont &&
		 (IRIT_APX_EQ(KVPrev[-1], KVPrev[0]) ||
		  IRIT_APX_EQ(KVPrev[0], KVPrev[1])))) {
		*KVPtr++ = *KVPrev;
		Kvs[i].KvLen++;
	    }
	}

	/* Copy last Order knots verbatim. */
	for (l = 0; l < Order; l++)
	    *KVPtr++ = *KVPrev++;

	KVPrev = Kvs[i].Kv;
	PrevLen = Kvs[i].KvLen - Order;

        if (InnerProd)
	    Kvs[i].InnerProds = CEditBasisInnerProdMat(Kvs[i].Kv, Kvs[i].KvLen,
						       Order, Periodic);
	else
	    Kvs[i].InnerProds = NULL;

	/* Make sure we did not exhaust all the interior knots already, or  */
	/* alternatively all interior knots maintains discontonuities.      */
	if (PrevLen <= Order + (Periodic ? Order - 1 : 0)) {
	    if (PrevLen == Order + (Periodic ? Order - 1 : 0))
		i++;
	    break;
	}
    }
    MultiResKvs -> NumKvs = i;             /* Actual number of knot vectors. */

    if (Periodic) {
	IrtRType
	    MultFactor = 2.0;

	/* Make sure spaces are consistent at the ends. */
	for (i = 1; i < MultiResKvs -> NumKvs; i++) {
	    int Len = Kvs[i].KvLen - Order;
	    CagdRType
		*Kv = Kvs[i].Kv;

	    for (j = Len + 1; j < Len + Order; j++)
		Kv[j] = Kv[j - 1] + (Kv[j - Len] - Kv[j - (Len + 1)])
								* MultFactor;

	    for (j = Order - 2; j >= 0 ; j--)
		Kv[j] = Kv[j + 1] - (Kv[j + Len] - Kv[j + Len - 1]);

	    MultFactor += MultFactor;
	}
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintKVs, FALSE) {
	    for (i = 0; i < MultiResKvs -> NumKvs; i++) {
	        IRIT_INFO_MSG_PRINTF("KV - LEVEL %d (len = %d)\n",
			             i, Kvs[i].KvLen);

		for (j = 0; j < Kvs[i].KvLen; j++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ", Kvs[i].Kv[j]);
		IRIT_INFO_MSG("\n\n");
	    }
	}
    }
#endif /* DEBUG */

    return MultiResKvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes all the inner products between basis functions and their        *
* derivatives.  If the function is Periodic, we compensate as well at the    *
* end points.								     *
*									     *
* PARAMETERS:                                                                *
*   KV:        Knot vector of the space                                      *
*   Len:       Length of knot vector KV.                                     *
*   Order:     Order of function space.		                             *
*   Periodic:  TRUE if the space is to be converted to a periodic one.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType **:  The inner products as a 2D matrix.                        *
*****************************************************************************/
static CagdRType **CEditBasisInnerProdMat(CagdRType *KV,
					  int Len,
					  int Order,
					  int Periodic)
{
    int i, j,
	Order1 = Order - 1,
	PtsLen = Len - Order;
    CagdRType
	**IProds = SymbBspBasisInnerProdMat(KV, Len, Order, Order - 1),
	**MProds = (CagdRType **) IritMalloc(sizeof(CagdRType *) * PtsLen);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugInnerProdPreproc, FALSE) {
	    IRIT_INFO_MSG_PRINTF("Inner product matrix of size %d x %d\n",
		                 PtsLen, PtsLen - 1);
	    for (i = 0; i < PtsLen; i++) {
	        for (j = 0; j < PtsLen - 1; j++)
		    IRIT_INFO_MSG_PRINTF("%8.5f ", IProds[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    /* Compute "Int(Bi(t) Bj'(t) + Bi'(t) Bj(t)) dt" from the inner          */
    /* products computed in IProds.					     */
    for (i = 0; i < PtsLen; i++) {
        MProds[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * PtsLen);
	for (j = 0; j < PtsLen; j++) {
	    CagdRType
	        KVD1 = KV[i + Order1] - KV[i],
	        KVD2 = KV[i + Order]  - KV[i + 1],
	        KVD3 = KV[j + Order1] - KV[j],
	        KVD4 = KV[j + Order]  - KV[j + 1];

	    MProds[i][j] = Order1 *
	        ((KVD1 > IRIT_UEPS && i > 0 ? -IProds[j][i - 1] / KVD1 : 0.0) +
		 (KVD2 > IRIT_UEPS && i < PtsLen - 1 ? IProds[j][i] / KVD2 : 0.0) +
		 (KVD3 > IRIT_UEPS && j > 0 ? IProds[i][j - 1] / KVD3 : 0.0) -
		 (KVD4 > IRIT_UEPS && j < PtsLen - 1 ? IProds[i][j] / KVD4 : 0.0));
	}
    }

    for (i = 0; i < PtsLen; i++)
	IritFree(IProds[i]);
    IritFree(IProds);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugInnerProdPreproc, FALSE) {
	    IRIT_INFO_MSG_PRINTF("Area Inner product matrix of size %d x %d\n",
		                 PtsLen, PtsLen);
	    for (i = 0; i < PtsLen; i++) {
	        for (j = 0; j < PtsLen; j++)
		    IRIT_INFO_MSG_PRINTF("%8.5f ", MProds[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    return MProds;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Display the region of the edited curve representing the active MR region *
* using the current MRLevel in IGCrvEditMRLevel and parameter value t saved  *
* in CEditLastMRParameter.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       Curve we are working on.                                      *
*   MultiKvs:  Hierachy of multiresolution knot vectors.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  The active multiresolution region as a curve object.  *
*                                                                            *
* KEYWORDS:                                                                  *
*   CEditMultiResViewRegion                                                  *
*****************************************************************************/
static IPObjectStruct *CEditMultiResViewRegion(CagdCrvStruct *Crv,
					       CEditMultiResKvsStruct *MultiKvs)
{
    int IMRLevel, IMRLevel1, Kv1Len, Kv2Len, Idx1, Idx2, Order;
    CagdRType t, TMin, TMax, MRLevelFrac, t1, t2, t1aux, t2aux,
	*Kv1, *Kv2, MRLevel;
    CagdCrvStruct *MRCrv;
    CEditMultiResOneKvStruct *Kvs;
    IPObjectStruct *PObj;

    if (MultiKvs == NULL ||
	IGCrvEditParam.CrvState != IG_CRV_EDIT_STATE_EDIT ||
	IGCEditOperation != IG_CRV_EDIT_EVENT_MODIFY_CURVE ||
	CEditLastMRParameter == -IRIT_INFNTY)
	return NULL;

    MRLevel = IGCrvEditMRLevel * (MultiKvs -> NumKvs - 1);
    Kvs = MultiKvs -> Kvs;

    Order = Crv -> Order;
    CagdCrvDomain(Crv, &TMin, &TMax); 
    MRLevel = IRIT_BOUND(MRLevel, 0, MultiKvs -> NumKvs - 1);
    t = IRIT_BOUND(CEditLastMRParameter, TMin, TMax);
    IMRLevel = (int) MRLevel;
    MRLevelFrac = MRLevel - IMRLevel;
    IMRLevel1 = IMRLevel + 1;
    IMRLevel1 = IRIT_BOUND(IMRLevel1, 0, MultiKvs -> NumKvs - 1);

    Kv1 = Kvs[IMRLevel].Kv;
    Kv2 = Kvs[IMRLevel1].Kv;
    Kv1Len = Kvs[IMRLevel].KvLen;
    Kv2Len = Kvs[IMRLevel1].KvLen;

    /* Compute starting parameter value: */
    Idx1 = BspKnotLastIndexLE(Kv1, Kv1Len, t) - Order + 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len - 1);
    Idx2 = BspKnotLastIndexLE(Kv2, Kv2Len, t) - Order + 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv2Len - 1);
    t1aux = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t1 = IRIT_BOUND(t1aux, TMin, TMax);

    /* Compute ending parameter value: */
    Idx1 = BspKnotFirstIndexG(Kv1, Kv1Len, t) + Order - 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len - 1);
    Idx2 = BspKnotFirstIndexG(Kv2, Kv2Len, t) + Order - 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv2Len - 1);
    t2aux = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t2 = IRIT_BOUND(t2aux, TMin, TMax);

    /* Make sure IGCrvEditCurrentUVCrv, if exists, is in surface bounds. */
    if (IGCrvEditCurrentUVCrv != NULL) {
	int i;
	CagdRType
	    **UVPoints = IGCrvEditCurrentUVCrv -> Points;

	for (i = 0; i < IGCrvEditCurrentUVCrv -> Length; i++) {
	    UVPoints[1][i] = IRIT_BOUND(UVPoints[1][i], 0.0, 1.0);
	    UVPoints[2][i] = IRIT_BOUND(UVPoints[2][i], 0.0, 1.0);
	}
    }

    /* Convert the curve geometry into polykline(s). */
    PObj = IPAllocObject("", IP_OBJ_POLY, NULL);
    IP_SET_POLYLINE_OBJ(PObj);
    MRCrv = CagdCrvRegionFromCrv(IGCrvEditCurrentUVCrv != NULL ?
				 IGCrvEditCurrentUVCrv :
				 Crv, t1, t2);
    PObj -> U.Pl = EvaluateEucPolyFromUVCrv(MRCrv);
    CagdCrvFree(MRCrv);

    if (Crv -> Periodic) {
	if (t1 != t1aux && TMax - (t1 - t1aux) >= TMin) {
	    /* Needs to jump to end and bring a porion of curve from there. */
	    MRCrv = CagdCrvRegionFromCrv(IGCrvEditCurrentUVCrv != NULL ?
					 IGCrvEditCurrentUVCrv :
					 Crv, TMax - (t1 - t1aux), TMax);
	    PObj -> U.Pl -> Pnext = EvaluateEucPolyFromUVCrv(MRCrv);
	    CagdCrvFree(MRCrv);
	}
	else if (t2 != t2aux && TMin + (t2aux - t2) <= TMax) {
	    /* Needs to jump to begining and bring a porion from there. */
	    MRCrv = CagdCrvRegionFromCrv(IGCrvEditCurrentUVCrv != NULL ?
					 IGCrvEditCurrentUVCrv :
					 Crv, TMin, TMin + (t2aux - t2));
	    PObj -> U.Pl -> Pnext = EvaluateEucPolyFromUVCrv(MRCrv);
	    CagdCrvFree(MRCrv);
	}
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an additive term at multi resolution level MRLevel at           M
* parametric location t.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         The currently edited curve.  Could be NULL!		     M
*   Order:       Order of Crv.  Redundent if Crv is provided.		     M
*   MultiKvs:    The knot vectors' hierarchy.				     M
*   MRLevel:     Multi resolution level.                                     M
*   t:           Parameter value of curve at which a user change occur.      M
*   TransDir:    Vectoric change at that location.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The additive vector field.  If Crv is provided, the     M
*                additive term is added to Crv and the sum is returned.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditMultiResModify                                                      M
*****************************************************************************/
CagdCrvStruct *CEditMultiResModify(CagdCrvStruct *Crv,
				   int Order,
				   CEditMultiResKvsStruct *MultiKvs,
				   IrtRType MRLevel,
				   CagdRType t,
				   IrtVecType TransDir)
{
    CagdBType
	Periodic = IGCrvEditParam.EndCond == CAGD_END_COND_PERIODIC;
    int IMRLevel, IMRLevel1;
    CagdRType MRLevelFrac;
    CagdCrvStruct *Crv1, *Crv2, *TCrv;
    CEditMultiResOneKvStruct
	*Kvs = MultiKvs -> Kvs;
    CagdPointType PType;

    if (Crv != NULL) {
	CagdRType TMin, TMax;

	PType = Crv -> PType;

	CagdCrvDomain(Crv, &TMin, &TMax); 
	t = IRIT_BOUND(t, TMin, TMax);
    }
    else
	PType = CAGD_PT_E3_TYPE;

    MRLevel = IRIT_BOUND(MRLevel, 0, MultiKvs -> NumKvs - 1);
    IMRLevel = (int) MRLevel;
    MRLevelFrac = MRLevel - IMRLevel;
    IMRLevel1 = IMRLevel + 1;
    IMRLevel1 = IRIT_BOUND(IMRLevel1, 0, MultiKvs -> NumKvs - 1);

    /* Create two curves at the two discrete MR levels that bound MRLevel. */
    Crv1 = CagdPeriodicCrvNew(IGCrvEditParam.Type, PType,
			      Kvs[IMRLevel].KvLen - Order -
						  (Periodic ? Order - 1 : 0),
			      Periodic);
    Crv1 -> Order = Order;
    Crv1 -> KnotVector = BspKnotCopy(NULL, Kvs[IMRLevel].Kv,
				     Kvs[IMRLevel].KvLen); 
    CEditMultiResUpdateOneCrv(Crv1, &Kvs[IMRLevel], t, TransDir);

    if (IRIT_APX_EQ(MRLevelFrac, 0.0)) {
	TCrv = Crv1;
    }
    else {
	/* Create second curve... */
	Crv2 = CagdPeriodicCrvNew(IGCrvEditParam.Type, PType,
				  Kvs[IMRLevel1].KvLen - Order -
						  (Periodic ? Order - 1 : 0),
				  Periodic);
	Crv2 -> Order = Order;
	Crv2 -> KnotVector = BspKnotCopy(NULL, Kvs[IMRLevel1].Kv,
					 Kvs[IMRLevel1].KvLen); 
	CEditMultiResUpdateOneCrv(Crv2, &Kvs[IMRLevel1], t, TransDir);
    
	TCrv = SymbCrvScalarScale(Crv1, 1.0 - MRLevelFrac);
	CagdCrvFree(Crv1);
	Crv1 = TCrv;

	TCrv = SymbCrvScalarScale(Crv2, MRLevelFrac);
	CagdCrvFree(Crv2);
	Crv2 = TCrv;

	TCrv = SymbCrvAdd(Crv1, Crv2);
	CagdCrvFree(Crv1); 
	CagdCrvFree(Crv2);
    }

    if (Crv != NULL) {
	Crv1 = SymbCrvAdd(Crv, TCrv);
	CagdCrvFree(TCrv); 

	return Crv1;
    }
    else
	return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the given MR curve according on current motion and possibly       *
* constraints.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         The currently edited curve.				     *
*   Kvs:	 Current knot vector space information.			     *
*   t:           Parameter value of curve at which a user change occur.      *
*   TransDir:    Vectoric change at that location.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void CEditMultiResUpdateOneCrv(CagdCrvStruct *Crv,
				      CEditMultiResOneKvStruct *Kvs,
				      CagdRType t,
				      IrtVecType TransDir)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int SatisfyAll, i, j,
    	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType),
	Order = Crv -> Order;
    CagdRType
	**Points = Crv -> Points;

    if (IGCrvEditParam.SupportConstraints) {
	CCnstSolutionWeightsStruct Weights;

	if (IGCrvEditCurrentCrv -> Length == Crv -> Length) {
	    for (i = !IsRational; i <= MaxAxis; i++)
		CAGD_GEN_COPY(Points[i], IGCrvEditCurrentCrv -> Points[i],
			      sizeof(CagdRType) * Crv -> Length);
	}
	else {
	    for (i = !IsRational; i <= MaxAxis; i++)
		IRIT_ZAP_MEM(Points[i], sizeof(CagdRType) * Crv -> Length);
	}

	/* Solve for weights that do not affect the constraints. */
	Weights = *CCnstSolveConstraints(TransDir, Crv,
					 Kvs, t, 0, IG_CRV_CNST_POSITION,
					 &SatisfyAll);
	for (i = !IsRational; i <= MaxAxis; i++)
	    IRIT_ZAP_MEM(Points[i], sizeof(CagdRType) * Crv -> Length);
	if (!SatisfyAll && IGCrvEditParam.AbortIfCnstFailed)
	    return;

	for (i = Weights.IndexFirst, j = 0;
	     i < Weights.IndexFirst + Weights.DataSize;
	     i++, j++) {
	    int k = i % Crv -> Length;

	    switch (Crv -> PType) {
	        case CAGD_PT_E3_TYPE:
		    Points[3][k] += Weights.W[2][j];
		case CAGD_PT_E2_TYPE:
		    Points[1][k] += Weights.W[0][j];
		    Points[2][k] += Weights.W[1][j];
		    break;
		case CAGD_PT_P3_TYPE:
		    Points[3][k] += Points[0][j] * Weights.W[2][j];
		case CAGD_PT_P2_TYPE:
		    Points[1][k] += Points[0][j] * Weights.W[0][j];
		    Points[2][k] += Points[0][j] * Weights.W[1][j];
		    break;
		default:
		    IGIritError("CEditMultiResModify Error: Invalid point type");
		    break;
	    }
	}
    }
    else {
	/* Weight factors are set to be evaluation of the Bspline functions. */
	int IndexFirst;
	CagdRType
	    *WFactors = BspCrvCoxDeBoorBasis(Crv -> KnotVector, Order,
					     Crv -> Length, Crv -> Periodic,
					     t, &IndexFirst);

	for (i = !IsRational; i <= MaxAxis; i++)
	    IRIT_ZAP_MEM(Points[i], sizeof(CagdRType) * Crv -> Length);

	for (i = IndexFirst; i < IndexFirst + Order; i++, WFactors++) {
	    int j = i % Crv -> Length;

	    *WFactors *= CRV_MR_UNCONSTRANT_SCALE;
	    
	    switch (Crv -> PType) {
	        case CAGD_PT_E3_TYPE:
		    Points[3][j] += *WFactors * TransDir[2];
		case CAGD_PT_E2_TYPE:
		    Points[1][j] += *WFactors * TransDir[0];
		    Points[2][j] += *WFactors * TransDir[1];
		    break;
		case CAGD_PT_P3_TYPE:
		    Points[3][j] += Points[0][j] * *WFactors * TransDir[2];
		case CAGD_PT_P2_TYPE:
		    Points[1][j] += Points[0][j] * *WFactors * TransDir[0];
		    Points[2][j] += Points[0][j] * *WFactors * TransDir[1];
		    break;
		default:
		    IGIritError("CEditMultiResUpdateOneCrv Error: Invalid point type");
		    break;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the global multiresolution decomposition hierarchy.              *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CEditUpdateMultResCrvDecomp(void)
{
    IRIT_STATIC_DATA int
	LastOrder = -1,
	LastKvLen = -1;
    IRIT_STATIC_DATA CagdRType
	*LastKV = NULL;

    if (IGCrvEditParam.Type != CAGD_CBSPLINE_TYPE)
	return;

    if (CEditMultiResKvs != NULL) {
	int i, j,
	    MROrder = CEditMultiResKvs -> Order,
	    CrvOrder = IGCrvEditCurrentCrv -> Order;

	/* Lets see if the old one is same as current curve. */
	if (LastOrder == CrvOrder &&
	    LastKvLen == CAGD_CRV_PT_LST_LEN(IGCrvEditCurrentCrv) + CrvOrder &&
	    BspKnotVectorsSame(LastKV, IGCrvEditCurrentCrv -> KnotVector,
			       LastKvLen, IRIT_EPS))
	    return;

	/* Different KVs - free old data structure. */
	for (i = 0; i < CEditMultiResKvs -> NumKvs; i++) {
	    IritFree(CEditMultiResKvs -> Kvs[i].Kv);
	    if (CEditMultiResKvs -> Kvs[i].InnerProds) {
		for (j = 0; j < CEditMultiResKvs -> Kvs[i].KvLen - MROrder; j++)
		    IritFree(CEditMultiResKvs -> Kvs[i].InnerProds[j]);
		IritFree(CEditMultiResKvs -> Kvs[i].InnerProds);
	    }
	}
	IritFree(CEditMultiResKvs -> Kvs);
	IritFree(CEditMultiResKvs);
    }

    LastKvLen = CAGD_CRV_PT_LST_LEN(IGCrvEditCurrentCrv) +
						IGCrvEditCurrentCrv -> Order;
    if (LastKV != NULL)
	IritFree(LastKV);
    LastKV = BspKnotCopy(NULL, IGCrvEditCurrentCrv -> KnotVector, LastKvLen);
    LastOrder = IGCrvEditCurrentCrv -> Order;

    /* Create new KVs multi resolution decomposition data structure. */
    CEditMultiResKvs =
	CEditMultiResPrepKVs(IGCrvEditCurrentCrv -> KnotVector,
			     CAGD_CRV_PT_LST_LEN(IGCrvEditCurrentCrv),
			     IGCrvEditCurrentCrv -> Order,
			     FALSE,
			     IGCrvEditParam.EndCond == CAGD_END_COND_PERIODIC,
			     TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attach to a curve object for further editing.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:     Curve object to attach to.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditAttachOldDirectly                                                   M
*****************************************************************************/
void CEditAttachOldDirectly(IPObjectStruct *CrvObj)
{
    const char *Str;
    IPObjectStruct *UVCrvObj;

    IGCrvEditCurrentObj = CrvObj;
    IGCrvOriginalCurve = CagdCrvCopy(IGCrvEditCurrentObj -> U.Crvs);

    if ((Str = AttrGetObjectStrAttrib(IGCrvEditCurrentObj, "ECConstraints"))
								      != NULL)
        CCnstStr2Constraints(Str);

    if (((Str = AttrGetObjectStrAttrib(IGCrvEditCurrentObj,
				       "_uvsrf")) != NULL ||
	 (Str = AttrGetObjectStrAttrib(IGCrvEditCurrentObj,
				       "ECUvSrf")) != NULL) &&
	((UVCrvObj = AttrGetObjectObjAttrib(IGCrvEditCurrentObj,
					    "_uvcrv")) != NULL ||
	 (UVCrvObj = AttrGetObjectObjAttrib(IGCrvEditCurrentObj,
					    "ECUvCrv")) != NULL) &&
	(CEditSrfSktchObj = IGFindObjectByName(Str)) != NULL &&
	(IP_IS_SRF_OBJ(CEditSrfSktchObj) ||
	 IP_IS_TRIMSRF_OBJ(CEditSrfSktchObj))) {
	IGCrvEditCurrentUVCrv = CagdCrvCopy(UVCrvObj -> U.Crvs);

	IGCrvEditSktchSrf = CagdSrfCopy(
				    CRV_EDIT_GET_SKTCH_SRF(CEditSrfSktchObj));
	BspKnotAffineTransOrder2(IGCrvEditSktchSrf -> UKnotVector,
				 IGCrvEditSktchSrf -> UOrder,
				 CAGD_SRF_UPT_LST_LEN(IGCrvEditSktchSrf)
					    + IGCrvEditSktchSrf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(IGCrvEditSktchSrf -> VKnotVector,
				 IGCrvEditSktchSrf -> VOrder,
				 CAGD_SRF_VPT_LST_LEN(IGCrvEditSktchSrf)
					    + IGCrvEditSktchSrf -> VOrder,
				 0.0, 1.0);
	CEditSrfSktchRayPrep = IntrSrfHierarchyPreprocessSrf(IGCrvEditSktchSrf,
						   CRV_EDIT_TRIM_SRF_FINENESS);
    }

    IG_RST_HIGHLIGHT1_OBJ(IGCrvEditCurrentObj);
    IGCrvEditCurrentCrv = CagdCrvCopy(IGCrvOriginalCurve);
    if (!IGCrvDrawOriginal) {
	IGCrvEditCurrentObj -> U.Crvs = NULL;
	IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
				     FALSE, TRUE, FALSE, TRUE);
    }

    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
    IGCrvEditPlaceMessage("Waiting for curve editing");
    CEditSetState(IGCrvEditCurrentCrv, &IGCrvEditParam);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle mouse events while the curve editing grabs the mouse.             M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:    Coordinates of the mouse event.                                 M
*   Event:   Type of event (mouse move, button up etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditHandleMouse                                                         M
*****************************************************************************/
void CEditHandleMouse(int x, int y, int Event)
{
    IRIT_STATIC_DATA int
	CEditButtonDown = FALSE;
    char Line[IRIT_LINE_LEN_LONG];
    CagdUVType UV;
    CagdRType *R;
    IrtPtType Pt;
    IrtVecType Dir;

    switch (IGCrvEditParam.CrvState) {
	case IG_CRV_EDIT_STATE_ATTACH_OLD:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if ((IGCrvEditCurrentObj =
		        IGHandlePickEvent(x, y, IG_PICK_CURVE)) != NULL &&
		    IGCrvEditCurrentObj -> U.Crvs != NULL) {
		    IGReleasePickedObject();
		    CEditAttachOldDirectly(IGCrvEditCurrentObj);
		}
		else {
		    IGCrvEditPlaceMessage("Failed to select a curve - try again");
		}
	    }
	    break;
	case IG_CRV_EDIT_STATE_CLONE_OLD:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if ((IGCrvEditCurrentObj =
		        IGHandlePickEvent(x, y, IG_PICK_CURVE)) != NULL &&
		    IGCrvEditCurrentObj -> U.Crvs != NULL) {
		    IGReleasePickedObject();
		    IG_RST_HIGHLIGHT1_OBJ(IGCrvEditCurrentObj);
		    IGCrvEditCurrentObj = IPCopyObject(NULL,
						       IGCrvEditCurrentObj,
						       TRUE);
		    IP_CAT_OBJ_NAME(IGCrvEditCurrentObj, "Clone");
		    IRIT_LIST_PUSH(IGCrvEditCurrentObj, IGGlblDisplayList);
		    CEditAttachOldDirectly(IGCrvEditCurrentObj);
		}
		else {
		    IGCrvEditPlaceMessage("Failed to select a curve - try again");
		}
	    }
	    break;
	case IG_CRV_EDIT_STATE_START_CTLPT:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if (IGCrvEditCurrentCrv == NULL) {
		    IGCrvEditCurrentCrv =
			CagdPeriodicCrvNew(IGCrvEditParam.Type,
					   IGCrvEditParam.Rational ?
					       CAGD_PT_P2_TYPE :
					       CAGD_PT_E2_TYPE,
					   IGCrvEditParam.Order,
					   IGCrvEditParam.Type ==
					       CAGD_CBSPLINE_TYPE &&
					   IGCrvEditParam.EndCond ==
					       CAGD_END_COND_PERIODIC);
		    IGCrvEditCurrentCrv -> Order = IGCrvEditParam.Order;
		    IGCrvEditCurrentCrv -> Length = 0;      /* No points now.*/
		}

		IGCrvEditActive = TRUE;

		/* Convert the points to object space and place in curve. */
		IGScreenToObject(x, y, Pt, Dir);
		if (IGCrvEditCurrentCrv -> Length <
		    IGCrvEditCurrentCrv -> Order) {
		    CagdRType
			**Points = IGCrvEditCurrentCrv -> Points;

		    Points[1][IGCrvEditCurrentCrv -> Length] = Pt[0];
		    Points[2][IGCrvEditCurrentCrv -> Length] = Pt[1];
		    if (Points[3] != NULL)
			Points[3][IGCrvEditCurrentCrv -> Length] = Pt[2];
		    if (CAGD_IS_RATIONAL_CRV(IGCrvEditCurrentCrv))
			Points[0][IGCrvEditCurrentCrv -> Length] = 1.0;

		    if (++IGCrvEditCurrentCrv -> Length ==
			IGCrvEditCurrentCrv -> Order) {
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			IGCrvOriginalCurve = CagdCrvCopy(IGCrvEditCurrentCrv);
			IGCrvEditCurrentObj =
			    IPGenCRVObject(IGCrvDrawOriginal ?
					   IGCrvOriginalCurve : NULL);
			IRIT_LIST_PUSH(IGCrvEditCurrentObj, IGGlblDisplayList);

			if (IGCrvEditParam.Type == CAGD_CBEZIER_TYPE) {
			    /* Done insertion of control points. */
			    IGRedrawViewWindow();
			    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
			    IGCrvEditPlaceMessage("Waiting for curve editing");
			    IGCrvEditParamUpdateWidget();
			    IGCrvEditGrabMouse = FALSE;
			    CEditButtonDown = FALSE;
			}
			CEditLastMRParameter = -IRIT_INFNTY;
			CEditFreeStateStack();
		    }
		}
		else {
		    CEditAddPoint(&IGCrvEditCurrentCrv,
				  IGCrvEditCurrentCrv -> Length, Pt);

		    CagdCrvFree(IGCrvOriginalCurve);
		    IGCrvOriginalCurve = CagdCrvCopy(IGCrvEditCurrentCrv);
		    if (IGCrvDrawOriginal) {
		        IGCrvEditCurrentObj -> U.Crvs = IGCrvOriginalCurve;
			IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
						     FALSE, TRUE, FALSE, TRUE);
		    }
		}
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_STATE_START_SKETCH:
	    if (IGCrvEditParam.Type == CAGD_CBEZIER_TYPE) {
		IGCrvEditPlaceMessage("Can sketch Bspline curves only");
		break;
	    }
	    IGScreenToObject(x, y, Pt, Dir);
	    switch (Event) {
		case IG_CRV_EDIT_BUTTONDOWN:
		    CEditSketchCrvReset();
		    CEditSketchAddPoint(Pt, NULL);
		    IGCrvEditActive = TRUE;
		    CEditButtonDown = TRUE;
		    break;
		case IG_CRV_EDIT_MOTION:
		    if (CEditButtonDown) {
			CEditSketchAddPoint(Pt, NULL);
			IGRedrawViewWindow();
		    }
		    break;
		case IG_CRV_EDIT_BUTTONUP:
		    if (CEditButtonDown) {
		        if ((IGCrvOriginalCurve = CEditSketchGetCurve())
								    != NULL) {
			    IGCrvEditCurrentCrv =
			                       CagdCrvCopy(IGCrvOriginalCurve);

			    IGCrvEditCurrentObj =
			        IPGenCRVObject(IGCrvDrawOriginal ?
					       CagdCrvCopy(IGCrvEditCurrentCrv) :
					       NULL);
			    IRIT_LIST_PUSH(IGCrvEditCurrentObj, IGGlblDisplayList);

			    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
			    IGCrvEditPlaceMessage("Waiting for curve editing");
			    CEditLastMRParameter = -IRIT_INFNTY;
			    CEditFreeStateStack();
			}
			else {
			    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_DETACH;
			    IGCrvEditPlaceMessage("Sketch failed");
			}
			CEditSketchCrvReset();
			IGCrvEditParamUpdateWidget();
			IGRedrawViewWindow();
			IGCrvEditGrabMouse = FALSE;
			CEditButtonDown = FALSE;
		    }
		    break;
	    }
	    break;
	case IG_CRV_EDIT_STATE_START_SKETCH_ON_SRF:
	    if (IGCrvEditParam.Type == CAGD_CBEZIER_TYPE) {
		IGCrvEditPlaceMessage("Can sketch Bspline curves only");
		break;
	    }
	    if (CEditSrfSktchObj == NULL) {
		if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		    if ((CEditSrfSktchObj =
			   IGHandlePickEvent(x, y,
				 IG_PICK_SURFACE | IG_PICK_TRIMSRF)) != NULL) {
		        IGReleasePickedObject();
		        sprintf(Line, "Start sketching on \"%s\"...",
				IP_GET_OBJ_NAME(CEditSrfSktchObj));
			IGCrvEditPlaceMessage(Line);

			if (IGCrvEditSktchSrf != NULL)
			    CagdSrfFree(IGCrvEditSktchSrf);
			IGCrvEditSktchSrf =
				      CRV_EDIT_GET_SKTCH_SRF(CEditSrfSktchObj);
			if (CAGD_IS_BEZIER_SRF(IGCrvEditSktchSrf))
			    IGCrvEditSktchSrf = CagdCnvrtBzr2BspSrf(IGCrvEditSktchSrf);
			else
			    IGCrvEditSktchSrf = CagdSrfCopy(IGCrvEditSktchSrf);

			BspKnotAffineTransOrder2(
					IGCrvEditSktchSrf -> UKnotVector,
					IGCrvEditSktchSrf -> UOrder,
					CAGD_SRF_UPT_LST_LEN(IGCrvEditSktchSrf)
					    + IGCrvEditSktchSrf -> UOrder,
					0.0, 1.0);
			BspKnotAffineTransOrder2(
					IGCrvEditSktchSrf -> VKnotVector,
					IGCrvEditSktchSrf -> VOrder,
					CAGD_SRF_VPT_LST_LEN(IGCrvEditSktchSrf)
					    + IGCrvEditSktchSrf -> VOrder,
					0.0, 1.0);
			CEditSrfSktchRayPrep =
			    IntrSrfHierarchyPreprocessSrf(IGCrvEditSktchSrf,
						   CRV_EDIT_TRIM_SRF_FINENESS);
		    }
		    else {
		        IGCrvEditPlaceMessage("Failed to select surface to sketch on - try again");
		    }
		}
		break;
	    }

	    IGScreenToObject(x, y, Pt, Dir);
	    switch (Event) {
		case IG_CRV_EDIT_BUTTONDOWN:
		    CEditSketchCrvReset();
		    if (IntrSrfHierarchyTestRay(CEditSrfSktchRayPrep, Pt,
						Dir, UV)) {
			R = CagdSrfEval(IGCrvEditSktchSrf, UV[0], UV[1]);
			CagdCoerceToE3(Pt, &R, -1, IGCrvEditSktchSrf -> PType);
		        CEditSketchAddPoint(Pt, UV);
		    }
		    IGCrvEditActive = TRUE;
		    CEditButtonDown = TRUE;
		    break;
		case IG_CRV_EDIT_MOTION:
		    if (CEditButtonDown &&
		        IntrSrfHierarchyTestRay(CEditSrfSktchRayPrep, Pt,
						Dir, UV)) {
		        R = CagdSrfEval(IGCrvEditSktchSrf, UV[0], UV[1]);
			CagdCoerceToE3(Pt, &R, -1, IGCrvEditSktchSrf -> PType);
			CEditSketchAddPoint(Pt, UV);
			IGRedrawViewWindow();
		    }
		    break;
		case IG_CRV_EDIT_BUTTONUP:
		    if (CEditButtonDown) {
			CEditButtonDown = FALSE;
			IGCrvEditGrabMouse = FALSE;

		        if ((IGCrvOriginalCurve = CEditSketchGetCurve())
								    != NULL) {
			    IGCrvEditCurrentCrv =
			                       CagdCrvCopy(IGCrvOriginalCurve);

			    IGCrvEditCurrentObj =
			        IPGenCRVObject(IGCrvDrawOriginal ?
					     CagdCrvCopy(IGCrvEditCurrentCrv) :
					     NULL);
			    IRIT_LIST_PUSH(IGCrvEditCurrentObj, IGGlblDisplayList);

			    IGCrvEditCurrentUVCrv = CEditSketchGetUVCurve();

			    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
			    IGCrvEditPlaceMessage("Waiting for curve editing");
			    CEditLastMRParameter = -IRIT_INFNTY;
			    CEditFreeStateStack();
			}
			else {
			    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_DETACH;
			    IGCrvEditPlaceMessage("Sketch failed");
			}

			CEditSketchCrvReset();
			IGCrvEditParamUpdateWidget();
			IG_RST_HIGHLIGHT1_OBJ(CEditSrfSktchObj);
			IGRedrawViewWindow();
		    }
		    break;
	    }
	    break;
	case IG_CRV_EDIT_STATE_EDIT:
	    CEditHandleMouseStateEdit(x, y, Event);
	    break;
	case IG_CRV_EDIT_STATE_DETACH:
	    break;
	default:
	    IGIritError("CEditHandleMouse Error: Invalid state type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle mouse events while the curve editing grabs the mouse.             *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:    Coordinates of the mouse event.                                 *
*   Event:   Type of event (mouse move, button up etc.).                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   CEditHandleMouse                                                         *
*****************************************************************************/
static void CEditHandleMouseStateEdit(int x, int y, int Event)
{
    IRIT_STATIC_DATA int
	ModifyWeight = FALSE,
	CtlPtIndex = -1,
	CEditButtonDown = FALSE;
    IRIT_STATIC_DATA CagdRType
	CurveParam = -IRIT_INFNTY,
	FirstParam = 0.0;
    IRIT_STATIC_DATA IrtPtType
	StartPt = { 0.0, 0.0, 0.0 };
    IRIT_STATIC_DATA IrtVecType
	StartDir = { 1.0, 0.0, 0.0 };
    char Line[IRIT_LINE_LEN_LONG];
    CagdRType t, *R;
    CagdVecStruct *Vec;
    IrtPtType Pt;
    IrtVecType Dir;
    IPObjectStruct *PTmp;

    switch (IGCEditOperation) {
        case IG_CRV_EDIT_EVENT_REGION:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if (CEditPickedPointIndex == 0) {
		    IGScreenToObject(x, y, Pt, Dir);
		    t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						  Pt, Dir);
		    if (t != -IRIT_INFNTY) {
		        FirstParam = t;
			CEditPickedPointIndex++;
			IGCrvEditPlaceMessage("Pick second point on region from curve");
		    }
		    else {
		        IGCrvEditPlaceMessage("No region was extracted");
			IGCrvEditGrabMouse = FALSE;
		    }
		}
		else { /* CEditPickedPointIndex > 0 */
		    IGScreenToObject(x, y, Pt, Dir);
		    t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						  Pt, Dir);
		    IGCrvEditGrabMouse = FALSE;
		    if (t != -IRIT_INFNTY) {
		        CEditDomainFromCrv(&IGCrvEditCurrentCrv,
					   IRIT_MIN(FirstParam, t),
					   IRIT_MAX(FirstParam, t));
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			CCnstDisableConstraints();
			IGRedrawViewWindow();
		    }
		    else
		        IGCrvEditPlaceMessage("No region was extracted");
		}
	    }
	    break;
	case IG_CRV_EDIT_EVENT_REFINE_ONE:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        IGScreenToObject(x, y, Pt, Dir);
		t = CEditFindClosestParameter(IGCrvEditCurrentCrv, Pt, Dir);
		if (t != -IRIT_INFNTY) {
		    CEditRefineCrv(&IGCrvEditCurrentCrv, &t, 1);
		    CCnstDisableConstraints();
		    IGRedrawViewWindow();
		}
		else
		    IGCrvEditPlaceMessage("No refinement took place");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_REFINE_ALL:
	    IGIritError("CEditHandleMouse Error: Invalid refine all event");
	    break;
	case IG_CRV_EDIT_EVENT_REFINE_REGION:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        if (CEditPickedPointIndex == 0) {
		    IGScreenToObject(x, y, Pt, Dir);
		    t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						  Pt, Dir);
		    if (t != -IRIT_INFNTY) {
		        FirstParam = t;
			CEditPickedPointIndex++;
			IGCrvEditPlaceMessage("Pick second point on region from curve");
		    }
		    else {
		        IGCrvEditPlaceMessage("No region was extracted");
			IGCrvEditGrabMouse = FALSE;
		    }
		}
		else { /* CEditPickedPointIndex > 0 */
		    IGScreenToObject(x, y, Pt, Dir);
		    t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						  Pt, Dir);
		    IGCrvEditGrabMouse = FALSE;
		    if (t != -IRIT_INFNTY) {
		        int IdxFirst, IdxLast,
			    Order = IGCrvEditCurrentCrv -> Order,
			    Length = IGCrvEditCurrentCrv -> Length,
			    Len = Order + Length;
			CagdRType *RefKV,
			    *KV = IGCrvEditCurrentCrv -> KnotVector,
			    t1 = IRIT_MIN(FirstParam, t),
			    t2 = IRIT_MAX(FirstParam, t);

			IdxFirst = BspKnotLastIndexL(KV, Len, t1);
			IdxFirst = IRIT_BOUND(IdxFirst, Order - 1, Length - 1);

			IdxLast = BspKnotFirstIndexG(KV, Len, t2);
			IdxLast = IRIT_BOUND(IdxLast, IdxFirst, Length);

			if (IdxFirst <= IdxLast) {
			    Len = IdxLast - IdxFirst + 2 * Order - 1;
			    IdxFirst -= Order - 1;
			    RefKV = BspKnotDoubleKnots(&KV[IdxFirst],
						       &Len, Order);
			    CEditRefineCrv(&IGCrvEditCurrentCrv, RefKV, Len);
			    IritFree(RefKV);
			    CCnstDisableConstraints();
			    IGRedrawViewWindow();
			    IGCrvEditPlaceMessage("Double the knots in region");
			}
		    }
		    else
		        IGCrvEditPlaceMessage("No region was extracted");
		}
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV1:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        IGScreenToObject(x, y, Pt, Dir);
		IGCrvEditGrabMouse = FALSE;
		t = CEditFindClosestParameter(IGCrvEditCurrentCrv, Pt, Dir);
		if (t != -IRIT_INFNTY) {
		    CEditSubdivCrv(&IGCrvEditCurrentCrv, t, -2);
		    if (IGCrvEditCurrentCrv -> Pnext) {
		        CagdCrvStruct
			    *Crv = IGCrvEditCurrentCrv -> Pnext;

			CagdCrvFree(IGCrvEditCurrentCrv);
			IGCrvEditCurrentCrv = Crv;
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			CCnstDisableConstraints();
			IGRedrawViewWindow();
		    }
		}
		else
		    IGCrvEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV2:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        IGScreenToObject(x, y, Pt, Dir);
		IGCrvEditGrabMouse = FALSE;
		t = CEditFindClosestParameter(IGCrvEditCurrentCrv, Pt, Dir);
		if (t != -IRIT_INFNTY) {
		    CEditSubdivCrv(&IGCrvEditCurrentCrv, t, -1);
		    if (IGCrvEditCurrentCrv -> Pnext) {
		        CagdCrvFree(IGCrvEditCurrentCrv -> Pnext);
			IGCrvEditCurrentCrv -> Pnext = NULL;
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			CCnstDisableConstraints();
			IGRedrawViewWindow();
		    }
		}
		else
		    IGCrvEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV_C0_CONT:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        IGScreenToObject(x, y, Pt, Dir);
		IGCrvEditGrabMouse = FALSE;
		t = CEditFindClosestParameter(IGCrvEditCurrentCrv, Pt, Dir);
		if (t != -IRIT_INFNTY) {
		    CEditSubdivCrv(&IGCrvEditCurrentCrv, t, 0);
		    CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					  IGCrvEditParam.EndCond,
					  IGCrvEditParam.ParamType);
		    CCnstDisableConstraints();
		    IGRedrawViewWindow();
		}
		else
		    IGCrvEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV_C1_CONT:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
	        IGScreenToObject(x, y, Pt, Dir);
		IGCrvEditGrabMouse = FALSE;
		t = CEditFindClosestParameter(IGCrvEditCurrentCrv, Pt, Dir);
		if (t != -IRIT_INFNTY) {
		    CEditSubdivCrv(&IGCrvEditCurrentCrv, t, 1);
		    CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					  IGCrvEditParam.EndCond,
					  IGCrvEditParam.ParamType);
		    CCnstDisableConstraints();
		    IGRedrawViewWindow();
		}
		else
		    IGCrvEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_MERGE_CURVES:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if ((PTmp = IGHandlePickEvent(x, y, IG_PICK_CURVE)) != NULL) {
		    IGReleasePickedObject();
		    if (PTmp == IGCrvEditCurrentObj)
		      IGCrvEditPlaceMessage("Cannot merge a curve to itself");
		    else {
			CEditMergeCrvs(&IGCrvEditCurrentCrv, PTmp -> U.Crvs);
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			CCnstDisableConstraints();
			IGRedrawViewWindow();

			IGDeleteOneObject(PTmp); /* Rm from display. */
		    }
		}
		else {
		    IGCrvEditPlaceMessage("Failed to select a second curve");
		}
		IGCrvEditGrabMouse = FALSE;
	    }
	    break;
	case IG_CRV_EDIT_EVENT_MOVE_CTLPTS:
	    switch (Event) {
		case IG_CRV_EDIT_BUTTONDOWN:
		    IGScreenToObject(x, y, Pt, Dir);
		    if ((CtlPtIndex = CEditFindClosestControlPoint(
						IGCrvEditCurrentCrv, Pt, Dir,
						&ModifyWeight)) >= 0) {
			CEditButtonDown = TRUE;
			CEditUpdateMultResCrvDecomp();
			if (ModifyWeight)
			    IGCrvEditPlaceMessage("Drag the control point's weight");
			else
			    IGCrvEditPlaceMessage("Drag the control point");
		    }
		    else {
			IGCrvEditPlaceMessage("No control point selected");
			break;
		    }

		    /* And redraw the curve in the new position. */
		case IG_CRV_EDIT_MOTION:
		    if (CEditButtonDown && CtlPtIndex >= 0) {
		        IGScreenToObject(x, y, Pt, Dir);
			CEditMovePoint(IGCrvEditCurrentCrv, CtlPtIndex,
				       ModifyWeight, Pt, Dir);
			IGRedrawViewWindow();
		    }
		    break;
		case IG_CRV_EDIT_BUTTONUP:
		    IGCrvEditPlaceMessage("Select a control point");
		    CEditButtonDown = FALSE;
		    CtlPtIndex = -1;
		    break;
		}
	    break;		    
	case IG_CRV_EDIT_EVENT_DELETE_CTLPTS:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		if (IGCrvEditCurrentCrv -> Length <= 2) {
		    IGCrvEditPlaceMessage("Number of control points too small.");
		    break;
		}
		IGScreenToObject(x, y, Pt, Dir);
		if ((CtlPtIndex = CEditFindClosestControlPoint(IGCrvEditCurrentCrv,
						       Pt, Dir,
						       &ModifyWeight)) >= 0) {
		    CEditDelPoint(&IGCrvEditCurrentCrv, CtlPtIndex);
		    CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					  IGCrvEditParam.EndCond,
					  IGCrvEditParam.ParamType);
		    CCnstDisableConstraints();
		    IGRedrawViewWindow();
		}
	    }
	    break;		    
	case IG_CRV_EDIT_EVENT_MODIFY_CURVE:
	    if (IGCrvEditParam.Type == CAGD_CBEZIER_TYPE) {
		IGCrvEditPlaceMessage("Can MR modify Bspline curves only");
		break;
	    }
	    if (IGCrvEditParam.Rational) {
		IGCrvEditPlaceMessage("Can MR modify integral curves only");
		break;
	    }
	    switch (Event) {
	        case IG_CRV_EDIT_BUTTONDOWN:
		IGScreenToObject(x, y, StartPt, StartDir);
		CurveParam = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						       StartPt, StartDir);
		if (CurveParam != -IRIT_INFNTY) {
		    CEditButtonDown = TRUE;
		    CEditUpdateMultResCrvDecomp();
		    IGCrvEditPlaceMessage("Drag the curve");
		}
		else {
		    IGCrvEditPlaceMessage("No point selected");
		    break;
		}
		CEditLastMRParameter = CurveParam;

		/* And redraw the curve in the new position. */
	    case IG_CRV_EDIT_MOTION:
		if (CEditButtonDown && CurveParam != -IRIT_INFNTY) {
		    IGScreenToObject(x, y, Pt, Dir);
		    CEditModifyCurve(IGCrvEditCurrentCrv, CurveParam,
				     StartPt, StartDir, Pt, Dir);
		    IGRedrawViewWindow();
		}
		break;
	    case IG_CRV_EDIT_BUTTONUP:
		IGCrvEditPlaceMessage("Pick a point on curve");
		CEditButtonDown = FALSE;
		CurveParam = -IRIT_INFNTY;
		break;
	    default:
		IGIritError("CEditHandleMouse Error: Invalid mouse event type");
		break;
	    }
	    break;
	case IG_CRV_EDIT_EVENT_EVALUATE:
	    IGScreenToObject(x, y, StartPt, Dir);
	    t = CEditFindClosestParameter(IGCrvEditCurrentCrv, StartPt, Dir);
	    if (t != -IRIT_INFNTY) {
		switch (IGCrvEvalEntity) {
		    case 0:		/* Eval param values. */
		        sprintf(Line, "Param = %f\n", t);
			IGCrvEditPlaceMessage(Line);
			break;
		    case 1:		/* Eval Position. */
			R = CagdCrvEval(IGCrvEditCurrentCrv, t);
			CagdCoerceToE3(Pt, &R, -1,
				       IGCrvEditCurrentCrv -> PType);
			sprintf(Line, "Position = (%f, %f, %f)\n",
				Pt[0], Pt[1], Pt[2]);
			IGCrvEditPlaceMessage(Line);
			break;
		    case 2:		/* Eval Tangent. */
			Vec = CagdCrvTangent(IGCrvEditCurrentCrv, t, TRUE);
			sprintf(Line, "Tangent = (%f, %f, %f)\n",
				Vec -> Vec[0],
				Vec -> Vec[1],
				Vec -> Vec[2]);
			IGCrvEditPlaceMessage(Line);
			break;
		    case 3:		/* Eval Normal. */
			Vec = CagdCrvNormal(IGCrvEditCurrentCrv, t, TRUE);
			sprintf(Line, "Normal = (%f, %f, %f)\n",
				Vec -> Vec[0],
				Vec -> Vec[1],
				Vec -> Vec[2]);
			IGCrvEditPlaceMessage(Line);
			break;
		}
	    }
	    else
	        IGCrvEditPlaceMessage("No point selected");
	    break;
	case IG_CRV_EDIT_EVENT_CONSTRAINTS:
	    CCnstHandleMouse(x, y, Event);
	    break;
	default:
	    IGIritError("CEditHandleMouse Error: Invalid event type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the movement of one control point index CtlPtIndex from current   *
* OldPos to the given NewPos so that all constraints are satisfied, if       *
* possible.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   CtlPtIndex:   Index of control point to move.                            *
*   OldPos:       Old position of control point.                             *
*   NewPos:       New position of control point.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CEditConstrainedMoveCtlPt(CagdCrvStruct *Crv,
				      int CtlPtIndex,
				      IrtPtType OldPos,
				      IrtPtType NewPos)
{
    int i, j, SatisfyAll;
    IrtPtType Pt;
    CagdRType
	**Points = Crv -> Points;
    CCnstSolutionWeightsStruct Weights;

    if (Crv == NULL) {
	IGCrvEditPlaceMessage("No curve under editing");
	return;
    }

    /* Solve for weights that do not affect the constraints. */
    switch (Crv -> PType) {
        case CAGD_PT_E3_TYPE:
	    Pt[2] = NewPos[2] - OldPos[2];
        case CAGD_PT_E2_TYPE:
	    Pt[0] = NewPos[0] - OldPos[0];
	    Pt[1] = NewPos[1] - OldPos[1];
	    break;
        default:
	    assert(0);
    }

    Weights = *CCnstSolveConstraints(Pt, Crv,
				     &CEditMultiResKvs -> Kvs[0],
				     0.0, CtlPtIndex, IG_CRV_CNST_CTL_PT,
				     &SatisfyAll);
    if (!SatisfyAll && IGCrvEditParam.AbortIfCnstFailed)
	return;

    for (i = Weights.IndexFirst, j = 0;
	 i < Weights.IndexFirst + Weights.DataSize;
	 i++, j++) {
        int k = i % Crv -> Length;

	switch (Crv -> PType) {
	    case CAGD_PT_E3_TYPE:
	        Points[3][k] += Weights.W[2][j];
	    case CAGD_PT_E2_TYPE:
	        Points[1][k] += Weights.W[0][j];
		Points[2][k] += Weights.W[1][j];
		break;
            default:
	        assert(0);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reevaluate all the current constraints and update the curve.             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditConstrainedReevalAll                                                M
*****************************************************************************/
void CEditConstrainedReevalAll(void)
{
    IRIT_STATIC_DATA IrtVecType
	TransDir = { 1.0, 1.0, 1.0 };
    int i, j, SatisfyAll;
    CagdCrvStruct
	*Crv = IGCrvEditCurrentCrv;
    CagdRType
	**Points = Crv -> Points;
    CCnstSolutionWeightsStruct Weights;

    if (IGCrvEditCurrentCrv == NULL) {
	IGCrvEditPlaceMessage("No curve under editing");
	return;
    }

    CEditUpdateMultResCrvDecomp();
    Weights = *CCnstSolveConstraints(TransDir, Crv,
				     &CEditMultiResKvs -> Kvs[0],
				     0.0, -1, IG_CRV_CNST_NONE,
				     &SatisfyAll);
    if (!SatisfyAll && IGCrvEditParam.AbortIfCnstFailed)
        return;

    for (i = Weights.IndexFirst, j = 0;
	 i < Weights.IndexFirst + Weights.DataSize;
	 i++, j++) {
        int k = i % Crv -> Length;

	switch (IGCrvEditCurrentCrv -> PType) {
	    case CAGD_PT_E3_TYPE:
	        Points[3][k] += Weights.W[2][j];
	    case CAGD_PT_E2_TYPE:
	        Points[1][k] += Weights.W[0][j];
		Points[2][k] += Weights.W[1][j];
		break;
            default:
	        assert(0);
	}
    }

    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets up the state of the new curve under editing.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      The new curve we are about to edit.                            *
*   State:    The state to update.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CEditSetState(CagdCrvStruct *Crv, IGCrvEditParamStruct *State)
{
    State -> Order = Crv -> Order;
    State -> Type = Crv -> GType;
    State -> EndCond =
	CAGD_IS_BSPLINE_CRV(Crv) && CAGD_IS_PERIODIC_CRV(Crv) ?
	    CAGD_END_COND_PERIODIC : (CAGD_IS_BEZIER_CRV(Crv) ||
				      BspCrvHasOpenEC(Crv) ?
				      CAGD_END_COND_OPEN :
				      CAGD_END_COND_FLOAT);
    State -> Rational = CAGD_IS_RATIONAL_CRV(Crv);

    if (CAGD_IS_BSPLINE_CRV(Crv)) {
        State -> ParamType =
	    BspIsKnotUniform(Crv -> Length, Crv -> Order, Crv -> KnotVector) ==
	       CAGD_END_COND_GENERAL ? CAGD_GENERAL_PARAM : CAGD_UNIFORM_PARAM;

	CEditUpdateKnotVector(Crv, State -> EndCond, State -> ParamType);
    }
    else
        State -> ParamType = CAGD_UNIFORM_PARAM;

    /* And prepare the global state. */
    IGCrvEditParamUpdateWidget();
    IGCrvEditActive = TRUE;
    IGCrvEditGrabMouse = FALSE;
    CEditLastMRParameter = -IRIT_INFNTY;
    CEditFreeStateStack();
    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle non mouse events while the curve editing.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:       Type of event (clear, order change, etc.).                  M
*   MenuIndex:   Index of last pop up menu event, if any.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditHandleNonMouseEvents                                                M
*****************************************************************************/
void CEditHandleNonMouseEvents(IGCrvEditEventType Event, int MenuIndex)
{
    IRIT_STATIC_DATA int
	CEditDisableUndo = FALSE;
    int l;
    char Line[IRIT_LINE_LEN_LONG];
    CagdCrvStruct *Crv, *UVCrv;
    IPObjectStruct *PObj;

    if (Event != IG_CRV_EDIT_EVENT_STATE &&
	Event != IG_CRV_EDIT_EVENT_PRIMITIVES &&
	Event != IG_CRV_EDIT_EVENT_ORDER &&
	Event != IG_CRV_EDIT_EVENT_END_COND &&
	Event != IG_CRV_EDIT_EVENT_RATIONAL &&
	Event != IG_CRV_EDIT_EVENT_CURVE_TYPE &&
	Event != IG_CRV_EDIT_EVENT_PARAM_TYPE &&
	IGCrvEditCurrentCrv == NULL) {
	IGCrvEditPlaceMessage("Pick a curve first");
	return;
    }
    switch (Event) {
	case IG_CRV_EDIT_EVENT_CONSTRAINTS:
	case IG_CRV_EDIT_EVENT_CLEAR:
	case IG_CRV_EDIT_EVENT_DISMISS:
        case IG_CRV_EDIT_EVENT_PRIMITIVES:
	case IG_CRV_EDIT_EVENT_STATE:
	case IG_CRV_EDIT_EVENT_UNDO:
	case IG_CRV_EDIT_EVENT_REDO:
	    break;
	default:
	    if (!CEditDisableUndo)
		CEditPushState(IGCrvEditCurrentCrv, IGCrvEditCurrentUVCrv,
			       &IGCrvEditParam);
	    break;
    }

    IGCrvEditGrabMouse = FALSE;
    IGCEditOperation = Event;
    CEditLastMRParameter = -IRIT_INFNTY;

    switch (Event) {
	case IG_CRV_EDIT_EVENT_CONSTRAINTS:
	    break;
	case IG_CRV_EDIT_EVENT_CLEAR:
	    IGCrvEditPlaceMessage("Clear curve editing operation");
	    IGCEditOperation = IG_CRV_EDIT_EVENT_NONE;
	    break;
	case IG_CRV_EDIT_EVENT_DISMISS:
	    CEditDetachCurve();
	    break;
	case IG_CRV_EDIT_EVENT_PRIMITIVES:
	    switch (MenuIndex) {
		case 0: /* Unit circle */
	            IGCrvEditCurrentObj =
			IPGenCRVObject(BspCrvCreateUnitCircle());
		    IP_SET_OBJ_NAME2(IGCrvEditCurrentObj, "PrimCircle");
		    break;
		case 1: /* Unit polynomial approximation circle */
	            IGCrvEditCurrentObj =
			IPGenCRVObject(BspCrvCreateUnitPCircle());
		    IP_SET_OBJ_NAME2(IGCrvEditCurrentObj, "PrimPCircle");
		    break;
		default:
		    IGCrvEditCurrentObj = NULL;
	    }

	    if (IGCrvEditCurrentObj != NULL) {
		IRIT_LIST_PUSH(IGCrvEditCurrentObj, IGGlblDisplayList);
	        IGCrvOriginalCurve =
		    CagdCrvCopy(IGCrvEditCurrentObj -> U.Crvs);
		IGCrvEditCurrentCrv = CagdCrvCopy(IGCrvOriginalCurve);
		if (!IGCrvDrawOriginal) {
		    IGCrvEditCurrentObj -> U.Crvs = NULL;
		    IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
						 FALSE, TRUE, FALSE, TRUE);
		}

		IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
		IGCrvEditPlaceMessage("Waiting for curve editing");
		CEditSetState(IGCrvEditCurrentCrv, &IGCrvEditParam);
		IGRedrawViewWindow();
	    }
	    else {
		IGCrvEditPlaceMessage("Failed to create primitive!");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_STATE:
	    switch (MenuIndex) {
		case IG_CRV_EDIT_STATE_ATTACH_OLD:
		case IG_CRV_EDIT_STATE_CLONE_OLD:
		    if (IGCrvEditCurrentCrv != NULL) {
			IGCrvEditPlaceMessage("Detach from current curve first");
			break;
		    }

		    IGCrvEditPlaceMessage("Pick a curve first");
		    IGCrvEditGrabMouse = TRUE;
		    IGCrvEditParam.CrvState = (IGCrvEditStateType) MenuIndex;
		    break;
		case IG_CRV_EDIT_STATE_START_CTLPT:
		    if (IGCrvEditCurrentCrv != NULL) {
			IGCrvEditPlaceMessage("Detach from current curve first");
			break;
		    }

		    IGCrvEditPlaceMessage("Please select control points");
		    IGCrvEditGrabMouse = TRUE;
		    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_START_CTLPT;
		    break;
		case IG_CRV_EDIT_STATE_START_SKETCH:
		    if (IGCrvEditCurrentCrv != NULL) {
			IGCrvEditPlaceMessage("Detach from current curve first");
			break;
		    }

		    IGCrvEditPlaceMessage("Please sketch the curve");
		    IGCrvEditGrabMouse = TRUE;
		    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_START_SKETCH;
		    break;
		case IG_CRV_EDIT_STATE_START_SKETCH_ON_SRF:
		    if (IGCrvEditCurrentCrv != NULL) {
			IGCrvEditPlaceMessage("Detach from current curve first");
			break;
		    }

		    IGCrvEditPlaceMessage("Please select surface to sketch on");
		    CEditSrfSktchObj = NULL;
		    IGCrvEditGrabMouse = TRUE;
		    IGCrvEditParam.Rational = FALSE;
		    IGCrvEditParam.CrvState =
		        IG_CRV_EDIT_STATE_START_SKETCH_ON_SRF;
		    break;
		case IG_CRV_EDIT_STATE_EDIT:
		    if (IGCrvEditCurrentCrv == NULL) {
			IGCrvEditPlaceMessage("No curve under editing");
			break;
		    }

		    IGCrvEditPlaceMessage("Waiting for curve editing");
		    IGCrvEditGrabMouse = FALSE;
		    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_EDIT;
		    break;
		case IG_CRV_EDIT_STATE_DETACH:
		    if (IGCrvEditCurrentCrv == NULL) {
			IGCrvEditPlaceMessage("No curve to detach from");
			break;
		    }

		    CEditDetachCurve();
		    IGRedrawViewWindow();
		    break;
		default:
		    break;
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV:
	    switch (MenuIndex) {
	        case 0:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_SUBDIV1;
		    break;
		case 1:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_SUBDIV2;
		    break;
		case 2:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_SUBDIV_C0_CONT;
		    break;
		case 3:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_SUBDIV_C1_CONT;
		    break;
	    }
	    IGCrvEditPlaceMessage("Pick a point on curve to subdivide");
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_EDIT_EVENT_DRAISE:
	    if (IGCrvEditParam.EndCond == CAGD_END_COND_PERIODIC)
	        IGCrvEditPlaceMessage("Cannot degree raise periodics");
	    else {
	        Crv = CagdCrvBlossomDegreeRaise(IGCrvEditCurrentCrv);
		CagdCrvFree(IGCrvEditCurrentCrv);
		IGCrvEditCurrentCrv = Crv;
		IGCrvEditParam.Order = IGCrvEditCurrentCrv -> Order;
		sprintf(Line, "Crv raised to %d", IGCrvEditParam.Order);
	        IGCrvEditPlaceMessage(Line);
		CCnstDisableConstraints();
		IGCrvEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_MERGE_CURVES:
	    IGCrvEditPlaceMessage("Pick a second curve to merge with");
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_EDIT_EVENT_REGION:
	    IGCrvEditPlaceMessage("Pick first point on region from curve");
	    CEditPickedPointIndex = 0;
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_EDIT_EVENT_REFINE:
	    if (IGCrvEditCurrentCrv -> GType != CAGD_CBSPLINE_TYPE) {
		IGCrvEditPlaceMessage("Refine Bspline curves only");
		break;
	    }

	    switch (MenuIndex) {
	        case 0:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_REFINE_ONE;
		    IGCrvEditPlaceMessage("Pick a point on curve to refine");
		    IGCrvEditGrabMouse = TRUE;
		    break;
		case 1:
		    {
		        CagdRType *t;
			int Len = IGCrvEditCurrentCrv -> Order + 
				  IGCrvEditCurrentCrv -> Length;

		        /* Refine at all interval locations along knot vec. */
		        IGCrvEditPlaceMessage("Double the number of knots along curve");
			t = BspKnotDoubleKnots(IGCrvEditCurrentCrv -> KnotVector,
					       &Len,
					       IGCrvEditCurrentCrv -> Order);
			CEditRefineCrv(&IGCrvEditCurrentCrv, t, Len);
			CCnstDisableConstraints();
			IritFree(t);
			IGRedrawViewWindow();
		    }
		    break;
		case 2:
		    IGCEditOperation = IG_CRV_EDIT_EVENT_REFINE_REGION;
		    IGCrvEditPlaceMessage("Pick first point on region from curve");
		    CEditPickedPointIndex = 0;
		    IGCrvEditGrabMouse = TRUE;
		    break;
	    }
	    break;
	case IG_CRV_EDIT_EVENT_ORDER:
	    if (IGCrvEditCurrentCrv != NULL &&
		IGCrvEditCurrentCrv -> GType == CAGD_CBSPLINE_TYPE &&
		IGCrvEditCurrentCrv -> Length >= IGCrvEditParam.Order &&
		IGCrvEditCurrentCrv -> Order != IGCrvEditParam.Order) {
		IGCrvEditCurrentCrv -> Order = IGCrvEditParam.Order;
		IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;
		CEditUpdateKnotVector(IGCrvEditCurrentCrv,
				      IGCrvEditParam.EndCond,
				      IGCrvEditParam.ParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_END_COND:
	    switch (MenuIndex) {
		case 0:
		    IGCrvEditParam.EndCond = CAGD_END_COND_OPEN;
		    break;
		case 1:
		    IGCrvEditParam.EndCond = CAGD_END_COND_FLOAT;
		    break;
		case 2:
		    IGCrvEditParam.EndCond = CAGD_END_COND_PERIODIC;
		    break;
	    };
	    if (IGCrvEditCurrentCrv != NULL &&
		IGCrvEditCurrentCrv -> GType == CAGD_CBSPLINE_TYPE) {
		if (IGCrvEditParam.ParamType == CAGD_GENERAL_PARAM)
		    IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;

		CEditUpdateKnotVector(IGCrvEditCurrentCrv,
				      IGCrvEditParam.EndCond,
				      IGCrvEditParam.ParamType);
		IGRedrawViewWindow();
		IGCrvEditParamUpdateWidget();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_RATIONAL:
	    if (!IGCrvEditParam.Rational && IGCrvEditCurrentUVCrv != NULL) {
		IGCrvEditPlaceMessage("Curve on surface cannot be rational");
		break;
	    }
	    IGCrvEditParam.Rational = !IGCrvEditParam.Rational;
	    if (IGCrvEditCurrentCrv != NULL) {
		l = CAGD_NUM_OF_PT_COORD(IGCrvEditCurrentCrv -> PType);
		if (IGCrvEditParam.Rational) {
		    int i, j;
		    IrtRType
			**Points = IGCrvEditCurrentCrv -> Points;

		    CCnstDisableConstraints();
    
		    IGCrvEditCurrentCrv -> PType = CAGD_MAKE_PT_TYPE(1, l);
		    if (Points[0] == NULL) {
			/* Allocate vec of rational terms and place in crv. */
			Points[0] = (CagdRType *)
			    IritMalloc(IGCrvEditCurrentCrv -> Length *
							    sizeof(CagdRType));
			for (i = 0; i < IGCrvEditCurrentCrv -> Length; i++)
			    Points[0][i] = 1.0;
		    }
		    else {
			/* Multiply the control points by the weights. */
			for (i = 0; i < IGCrvEditCurrentCrv -> Length; i++)
			    for (j = 1; j <= l; j++)
				Points[j][i] *= Points[0][i];
		    }
		}
		else {
		    int i, j;
		    IrtRType
			**Points = IGCrvEditCurrentCrv -> Points;

		    IGCrvEditCurrentCrv -> PType = CAGD_MAKE_PT_TYPE(0, l);
		    /* Divide the control points by the weights. */
		    for (i = 0; i < IGCrvEditCurrentCrv -> Length; i++)
			for (j = 1; j <= l; j++)
			    Points[j][i] /= Points[0][i];
		}
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_MOVE_CTLPTS:
	    IGCrvEditPlaceMessage("Select a control point");
	    IGCrvEditGrabMouse = TRUE;
	    if (!IGCrvEditDrawMesh) {
		IGCrvEditDrawMesh = TRUE;
		IGCrvEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_DELETE_CTLPTS:
	    IGCrvEditPlaceMessage("Select a control point");
	    IGCrvEditGrabMouse = TRUE;
	    if (!IGCrvEditDrawMesh) {
		IGCrvEditDrawMesh = TRUE;
		IGCrvEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_MODIFY_CURVE:
	    IGCrvEditPlaceMessage("Select a point on curve");
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_EDIT_EVENT_CURVE_TYPE:
	    IGCrvEditParam.Type =
		IGCrvEditParam.Type == CAGD_CBEZIER_TYPE
		    ? CAGD_CBSPLINE_TYPE : CAGD_CBEZIER_TYPE;
	    if (IGCrvEditCurrentCrv != NULL) {
		switch (IGCrvEditParam.Type) {
	            case CAGD_CBEZIER_TYPE:
		        IGCrvEditCurrentCrv -> GType = CAGD_CBEZIER_TYPE;
			if (IGCrvEditCurrentCrv -> KnotVector)
			    IritFree(IGCrvEditCurrentCrv -> KnotVector);
			IGCrvEditCurrentCrv -> KnotVector = NULL;
			IGCrvEditCurrentCrv -> Periodic = FALSE;
			IGCrvEditCurrentCrv -> Order =
			    IGCrvEditParam.Order =
				IGCrvEditCurrentCrv -> Length;
			CCnstDisableConstraints();
			IGCrvEditParamUpdateWidget();
			break;
		    case CAGD_CBSPLINE_TYPE:
			IGCrvEditCurrentCrv -> GType = CAGD_CBSPLINE_TYPE;
			if (IGCrvEditCurrentCrv -> KnotVector)
			    IritFree(IGCrvEditCurrentCrv -> KnotVector);
			IGCrvEditCurrentCrv -> KnotVector = NULL;
			IGCrvEditCurrentCrv -> Order = IGCrvEditParam.Order;
			CEditUpdateKnotVector(IGCrvEditCurrentCrv,
					      IGCrvEditParam.EndCond,
					      IGCrvEditParam.ParamType);
			break;
		    default:
			IGIritError("CEditHandleNonMouseEvents Error: Invalid curve type");
			break;
		}
		IGRedrawViewWindow();
	    }			
	    break;
	case IG_CRV_EDIT_EVENT_LSTSQR_PERCENT:
	    break;
	case IG_CRV_EDIT_EVENT_PARAM_TYPE:
	    switch (MenuIndex) {
	        case 0:
		    IGCrvEditParam.ParamType = CAGD_GENERAL_PARAM;
		    break;
	        case 1:
		    IGCrvEditParam.ParamType = CAGD_UNIFORM_PARAM;
		    break;
		case 2:
		    IGCrvEditParam.ParamType = CAGD_CENTRIPETAL_PARAM;
		    break;
		case 3:
		    IGCrvEditParam.ParamType = CAGD_CHORD_LEN_PARAM;
		    break;
	    }
	    if (IGCrvEditCurrentCrv != NULL) {
		CEditUpdateKnotVector(IGCrvEditCurrentCrv,
				      IGCrvEditParam.EndCond,
				      IGCrvEditParam.ParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_CRV_EDIT_EVENT_EVALUATE:
	    IGCrvEditPlaceMessage("Select a point on curve");
	    IGCrvEditGrabMouse = TRUE;
	    IGCrvEvalEntity = MenuIndex;
	    break;
	case IG_CRV_EDIT_EVENT_REVERSE:
	    if ((Crv = CagdCrvReverse(IGCrvEditCurrentCrv)) != NULL) {
	        CagdCrvFree(IGCrvEditCurrentCrv);
		IGCrvEditCurrentCrv = Crv;
		CCnstDisableConstraints();
		IGCrvEditPlaceMessage("Reverse succeeded");
		IGCrvEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    else
		IGCrvEditPlaceMessage("Reverse failed");
	    break;
	case IG_CRV_EDIT_EVENT_UNDO:
	    if ((Crv = CEditUndoState(&IGCrvEditParam, &UVCrv)) != NULL) {
		CEditDisableUndo = TRUE;

		CagdCrvFree(IGCrvEditCurrentCrv);
		IGCrvEditCurrentCrv = CagdCrvCopy(Crv);

		if (IGCrvEditCurrentUVCrv != NULL)
		    CagdCrvFree(IGCrvEditCurrentUVCrv);
		IGCrvEditCurrentUVCrv = UVCrv == NULL ? NULL : CagdCrvCopy(UVCrv);

		IGCrvEditParamUpdateWidget();
		IGCrvEditPlaceMessage("Undo succeeded");
		IGRedrawViewWindow();
		CEditDisableUndo = FALSE;
	    }
	    else
		IGCrvEditPlaceMessage("Undo failed");
	    break;
	case IG_CRV_EDIT_EVENT_REDO:
	    if ((Crv = CEditRedoState(&IGCrvEditParam, &UVCrv)) != NULL) {
		CEditDisableUndo = TRUE;

		CagdCrvFree(IGCrvEditCurrentCrv);
		IGCrvEditCurrentCrv = CagdCrvCopy(Crv);

		if (IGCrvEditCurrentUVCrv != NULL)
		    CagdCrvFree(IGCrvEditCurrentUVCrv);
		IGCrvEditCurrentUVCrv = UVCrv == NULL ? NULL : CagdCrvCopy(UVCrv);

		IGCrvEditParamUpdateWidget();
		IGCrvEditPlaceMessage("Redo succeeded");
		IGRedrawViewWindow();
		CEditDisableUndo = FALSE;
	    }
	    else
		IGCrvEditPlaceMessage("Redo failed");
	    break;
	case IG_CRV_EDIT_EVENT_SAVE_CURVE:
	    {
		int Handler;
		char Line[IRIT_LINE_LEN_LONG],
		    *FileName = (char *) MenuIndex;

		if (!IRT_STR_NULL_ZERO_LEN(FileName)) {
		    char *Str;

		    PObj = IPGenCRVObject(CagdCrvCopy(IGCrvEditCurrentCrv));
		    IP_SET_OBJ_NAME2(PObj, IP_GET_OBJ_NAME(IGCrvEditCurrentObj));

		    /* If has constraints - save as attributes. */
		    if ((Str = CCnstConstraints2Str()) != NULL) {
			AttrSetObjectStrAttrib(PObj, "ECConstraints", Str);
			IritFree(Str);
		    }

		    if (CEditSrfSktchRayPrep != NULL) {
		        AttrSetObjectStrAttrib(PObj, "ECUvSrf",
					       IP_GET_OBJ_NAME(CEditSrfSktchObj));
		        AttrSetObjectObjAttrib(PObj, "ECUvCrv",
			    IPGenCRVObject(CagdCrvCopy(IGCrvEditCurrentUVCrv)),
			    FALSE);
		    }

		    if ((Handler = IPOpenDataFile(FileName, FALSE,
						  FALSE)) >= 0) {
		        IPPutObjectToHandler(Handler, PObj);
			IPCloseStream(Handler, TRUE);

			sprintf(Line, "Curve saved under \"%s\"", FileName);
			IGCrvEditPlaceMessage(Line);	
		    }
		    else {
		        sprintf(Line, "Failed to open file \"%s\"", FileName);
		        IGIritError(Line);
		    }

		    IPFreeObject(PObj);
		}
		else
		    IGCrvEditPlaceMessage("Failed to pick curve's name");
	    }
	    break;
	case IG_CRV_EDIT_EVENT_SUBMIT_CURVE:
	    if (IGCrvEditCurrentCrv == NULL) {
		IGCrvEditPlaceMessage("No curve under editing to submit");
		break;
	    }
	    if (IGGlblStandAlone) {
		IGCrvEditPlaceMessage("No submissions in stand alone mode");
		break;
	    }
	    if (IGCrvEditCurrentObj == NULL ||
		!IP_VALID_OBJ_NAME(IGCrvEditCurrentObj)) {		
		IGCrvEditPlaceMessage("Object must have a name before submission");
		break;
	    }
	    PObj = IPGenCrvObject("_SubmitObject_",
				  CagdCrvCopy(IGCrvEditCurrentCrv), NULL);
	    AttrSetObjectStrAttrib(PObj, "ObjName",
				   IP_GET_OBJ_NAME(IGCrvEditCurrentObj));

	    if (CEditSrfSktchRayPrep != NULL) {
	        AttrSetObjectStrAttrib(PObj, "ECUvSrf",
				       IP_GET_OBJ_NAME(CEditSrfSktchObj));
	        AttrSetObjectObjAttrib(PObj, "ECUvCrv",
		    IPGenCRVObject(CagdCrvCopy(IGCrvEditCurrentUVCrv)),
		    FALSE);
	    }

	    IPSocWriteOneObject(IGGlblIOHandle, PObj);
	    IPFreeObject(PObj);
	    break;
        default:
	    IGIritError("CEditHandleNonMouseEvents Error: Invalid event type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently editted curve.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditDetachCurve                                                         M
*****************************************************************************/
void CEditDetachCurve(void)
{
    GMBBBboxStruct *BBox;
    char *Str;

    IGCrvEditGrabMouse = FALSE;
    IGCrvEditActive = FALSE;
    if ((Str = CCnstConstraints2Str()) != NULL) {
	AttrSetObjectStrAttrib(IGCrvEditCurrentObj, "ECConstraints", Str);
	IritFree(Str);
    }
    CCnstDetachConstraints();
    CEditFreeStateStack();
    CEditSketchCrvReset();
    IGCrvEditParam.CrvState = IG_CRV_EDIT_STATE_DETACH;
    IGCrvEditParamUpdateWidget();

    if (IGCrvEditCurrentCrv == NULL || IGCrvEditCurrentObj == NULL) {
        if (IGCrvEditCurrentCrv != NULL) {
	    CagdCrvFree(IGCrvEditCurrentCrv);
	    IGCrvEditCurrentCrv = NULL;
	}
	return;
    }

    if (!IGCrvEditParam.Rational && IGCrvEditCurrentCrv -> Points[0] != NULL) {
	IritFree(IGCrvEditCurrentCrv -> Points[0]);
	IGCrvEditCurrentCrv -> Points[0] = NULL;
    }

    if (CEditSrfSktchRayPrep != NULL) {
	AttrSetObjectStrAttrib(IGCrvEditCurrentObj, "_uvsrf",
			       IP_GET_OBJ_NAME(CEditSrfSktchObj));
	AttrSetObjectObjAttrib(IGCrvEditCurrentObj, "_uvcrv",
			       IPGenCRVObject(IGCrvEditCurrentUVCrv), FALSE);

	IntrSrfHierarchyFreePreprocess(CEditSrfSktchRayPrep);
	CEditSrfSktchRayPrep = NULL;
	CEditSrfSktchObj = NULL;
	CagdSrfFree(IGCrvEditSktchSrf);
	IGCrvEditSktchSrf = NULL;
	IGCrvEditCurrentUVCrv = NULL;
    }

    IGCrvEditCurrentObj -> U.Crvs = IGCrvEditCurrentCrv;
    CagdCrvFree(IGCrvOriginalCurve);

    IG_RST_HIGHLIGHT1_OBJ(IGCrvEditCurrentObj);

    IGCrvEditCurrentCrv = NULL;
    IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
				 FALSE, TRUE, FALSE, TRUE);
    BBox = GMBBComputeBboxObject(IGCrvEditCurrentObj);
    IRIT_PT_COPY(IGCrvEditCurrentObj -> BBox[0], BBox -> Min);
    IRIT_PT_COPY(IGCrvEditCurrentObj -> BBox[1], BBox -> Max);

    IGCrvEditCurrentObj = NULL;

    IGGlblPickedObj = NULL;
    if (IGGlblPickedPolyObj != NULL) {
	IPFreeObject(IGGlblPickedPolyObj);
	IGGlblPickedPolyObj = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a unit circle in the XY plane.  This circle is used to draw      M
* auxiliary markers such rational points' circles.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A unit circle object.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditGetUnitCircle                                                       M
*****************************************************************************/
IPObjectStruct *CEditGetUnitCircle(void)
{
    IRIT_STATIC_DATA IPObjectStruct
	*PCirc = NULL;
    int i;
    IPVertexStruct *V;
    IrtRType
	CosStep = cos(M_PI_MUL_2 / (CRV_EDIT_NUM_CIRC_SAMPLES - 1)),
	SinStep = sin(M_PI_MUL_2 / (CRV_EDIT_NUM_CIRC_SAMPLES - 1));

    if (PCirc == NULL) {
	PCirc = IPAllocObject("", IP_OBJ_POLY, NULL);
	IP_SET_POLYLINE_OBJ(PCirc);
	PCirc -> U.Pl = IPAllocPolygon(0, NULL, NULL);
	for (i = 0; i < CRV_EDIT_NUM_CIRC_SAMPLES; i++) {
	    PCirc -> U.Pl -> PVertex =
		IPAllocVertex2(PCirc -> U.Pl -> PVertex);
	}
	AttrSetObjectRGBColor(PCirc,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
	V = PCirc -> U.Pl -> PVertex;

	/* Build the circle around the origin. */
	V -> Coord[0] = 1.0;
	V -> Coord[1] = 0.0;
	V -> Coord[2] = 0.0;
	for ( ; V -> Pnext != NULL; V = V -> Pnext) {
	    V -> Pnext -> Coord[0] =  V -> Coord[0] * CosStep +
		                      V -> Coord[1] * SinStep;
	    V -> Pnext -> Coord[1] = -V -> Coord[0] * SinStep +
			              V -> Coord[1] * CosStep;
	    V -> Pnext -> Coord[2] = 0.0;
	}
    }

    return PCirc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a unit diamond in the XY plane.  This diamond is used to draw    M
* auxiliary markers such as tangent constraints.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A diamond object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditGetUnitDiamond                                                      M
*****************************************************************************/
IPObjectStruct *CEditGetUnitDiamond(void)
{
    IRIT_STATIC_DATA IPObjectStruct
	*PDiam = NULL;
    int i;
    IPVertexStruct *V;

    if (PDiam == NULL) {
	PDiam = IPAllocObject("", IP_OBJ_POLY, NULL);
	IP_SET_POLYLINE_OBJ(PDiam);
	PDiam -> U.Pl = IPAllocPolygon(0, NULL, NULL);
	for (i = 0; i < 5; i++) {
	    PDiam -> U.Pl -> PVertex =
		IPAllocVertex2(PDiam -> U.Pl -> PVertex);
	}
	AttrSetObjectRGBColor(PDiam,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
	V = PDiam -> U.Pl -> PVertex;

	/* Build the diamond around the origin. */
	V -> Coord[0] = 1.0;
	V -> Coord[1] = 0.0;
	V -> Coord[2] = 0.0;
	for ( ; V -> Pnext != NULL; V = V -> Pnext) {
	    V -> Pnext -> Coord[0] =  V -> Coord[1];
	    V -> Pnext -> Coord[1] = -V -> Coord[0];
	    V -> Pnext -> Coord[2] = 0.0;
	}
    }

    return PDiam;
}
