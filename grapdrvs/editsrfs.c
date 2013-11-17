/*****************************************************************************
*  Generic tools of interactive surface editing.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, May 1999.   *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "trim_lib.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "miscattr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "editsrfs.h"

#define SRF_EDIT_UNDO_STACK_SIZE	50
#define SRF_EDIT_NUM_CIRC_SAMPLES	64
#define SRF_EDIT_TRIM_CRV_EPS		1e-2
#define SRF_MR_MOTION_SCALE		2.0
#define SRF_MR_REGION_WIDTH		3                     /* In pixels. */

#define SRF_CMP_DIST_SQR(Pt1, Pt2, i, j) { \
		IrtRType d = IRIT_PT_PT_DIST_SQR(Pt1, Pt2); \
		if (d < DistSqr) { \
		    DistSqr = d; \
		    Index1 = i; \
		    Index2 = j; \
	        } }

typedef struct SrfEditUndoStackStruct {
    CagdSrfStruct *Srf;
    TrimSrfStruct *TSrfSkel;
    IGSrfEditParamStruct SrfEditParam;
} SrfEditUndoStackStruct;

IRIT_STATIC_DATA int
    SEditUndoStackPushPtr = 0, /* Also tells us the number of pushed items. */
    SEditUndoStackPopPtr = -1, /* Shifts back and forth by the undo/redo's. */
    SEditPickedPointIndex = 0; /* Number of picked points when n are neeed. */
IRIT_STATIC_DATA CagdRType
    SEditUMin, SEditUMax, SEditVMin, SEditVMax,
    SEditLastMRUParameter = -IRIT_INFNTY,
    SEditLastMRVParameter = -IRIT_INFNTY;
IRIT_STATIC_DATA TrimSrfStruct
    *SEditTrimSrfSkel = NULL; /* Valid if surface is part of a trimmed srf. */

IRIT_STATIC_DATA SrfEditUndoStackStruct
    SEditUndoStack[SRF_EDIT_UNDO_STACK_SIZE];
IRIT_STATIC_DATA SEditMultiResKvsStruct
    *SEditMultiResKvs = NULL;

IRIT_GLOBAL_DATA IGSrfEditParamStruct
    IGSrfEditParam = {
        4, 4,			                          /* Surface order. */
	CAGD_SBSPLINE_TYPE,	 /* Type of surface (Bspline, Bezier etc.). */
	CAGD_END_COND_OPEN, CAGD_END_COND_OPEN,   /* End conditions of srf. */
	CAGD_UNIFORM_PARAM, CAGD_UNIFORM_PARAM,     /* KV spacing (param.). */

	FALSE,		 /* Surface is rational (TRUE), polynomial (FALSE). */

	IG_SRF_EDIT_STATE_DETACH,	       /* State of surface editing. */
    };

IRIT_GLOBAL_DATA char
    *IGSrfEditStateEntries[] = {
	"Primitive Surface",
	"Attach to Old Surface",
	"Clone Old Surface",
	"Manipulate/Edit Surface",
	"Detach Current Surface",
	NULL
    },
    *IGSrfEditSubdivEntries[] = {
	"Subdivide Pick First",
	"Subdivide Pick Second",
	"Insert C0 Continuity",
	"Insert C1 Continuity",
	NULL
    },
    *IGSrfEditTrimSrfEntries[] = {
	"Trim Pick First",
	"Trim Pick Second",
	NULL
    },
    *IGSrfEditRefineEntries[] = {
	"Refine One",
	"Refine All",
	"Refine Region",
	NULL
    },
    *IGSrfEditEndCondEntries[] = {
	"Open End Condition",
	"Float End Condition",
	"Periodic End Condition",
	NULL
    },
    *IGSrfEditParamEntries[] = {
	"Uniform Parameterization",
	"Centripetal Parameterization",
	"Chord Len Parameterization",
	NULL
    },
    *IGSrfEditEvalEntityEntries[] = {
	"Eval Parameter",
	"Eval Position",
	"Eval U Tangent",
	"Eval V Tangent",
	"Eval Normal",
	NULL
    },
    *IGSrfEditPrimitivesEntries[] = {
	"Unit Sphere",
	"Unit XY Plane", 
	"Unit XZ Plane", 
	"Unit YZ Plane", 
	"Cylinder",
	"Trunc. Cone",
	"Cone",
	"Torus",
	NULL
    },
    *IGSrfEditReverseSrfEntries[] = {
	"Flip U <-> V",
	"Reverse U",
	"Reverse V",
	NULL
    };
IRIT_GLOBAL_DATA int
    IGSrfEditActive = FALSE,
    IGSrfDrawOriginal = TRUE,
    IGSrfEditDrawMesh = TRUE,
    IGSrfEvalEntity = -1,
    IGSrfEditNormalDir = TRUE,
    IGSrfEditGrabMouse = FALSE;
IRIT_GLOBAL_DATA IrtRType
    IGSrfEditMRULevel = 0.0,
    IGSrfEditMRVLevel = 0.0;
IRIT_GLOBAL_DATA IGSrfEditEventType
    IGSEditOperation = IG_SRF_EDIT_EVENT_STATE;
IRIT_GLOBAL_DATA CagdSrfStruct
    *IGSrfOriginalSurface = NULL,
    *IGSrfEditCurrentSrf = NULL;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGSrfEditPreloadEditSurfaceObj = NULL,
    *IGSrfEditCurrentObj = NULL;

static void UpdateCrntObjType(IPObjectStruct *Obj,
			      CagdSrfStruct *Srf,
			      TrimSrfStruct *TSrfSkel,
			      IPObjStructType NewObjType);
static IPObjectStruct *SEditMultiResViewRegion(CagdSrfStruct *Srf,
					       SEditMultiResKvsStruct *MultiKvs);
static void UpdateMultResSrfDecomp(void);
static void SEditHandleMouseStateEdit(int x, int y, int Event);
static void IGSrfEditSetState(CagdSrfStruct *Srf, IGSrfEditParamStruct *State);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pushes the current surface editing state on the Undo/Redo Stack.         M
* The implementation below is geared for simplicity, and not efficiency that M
* is not an issue here!							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:            Current editted surface.                                 M
*   TSrfSkel:       Current editted trimmed surface skeleton.		     M
*   SrfEditParam:   Current Surface editing parameters.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if successful.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditUndoState, SEditRedoState, SEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditPushState                                                           M
*****************************************************************************/
int SEditPushState(CagdSrfStruct *Srf,
		   TrimSrfStruct *TSrfSkel,
		   IGSrfEditParamStruct *SrfEditParam)
{
    int Index = SEditUndoStackPushPtr - 1;

    if (Srf == NULL)
	return FALSE;

    /* Make sure new pushed result is different than last time. */
    if (SEditUndoStackPushPtr > 0 &&
	((TSrfSkel == NULL && SEditUndoStack[Index].TSrfSkel == NULL) ||
	 (TSrfSkel != NULL && SEditUndoStack[Index].TSrfSkel != NULL &&
	  TrimSrfsSame(SEditUndoStack[Index].TSrfSkel, TSrfSkel,
		       IRIT_EPS))) &&
	CagdSrfsSame(SEditUndoStack[Index].Srf, Srf, IRIT_EPS) &&
	IRIT_GEN_CMP(&SEditUndoStack[Index].SrfEditParam,
		     SrfEditParam, sizeof(IGSrfEditParamStruct)) == 0)
        return FALSE;

    /* Make sure we have place in the undo stack. */
    if (SEditUndoStackPushPtr >= SRF_EDIT_UNDO_STACK_SIZE) {
	int i;

	SEditUndoStackPushPtr = SRF_EDIT_UNDO_STACK_SIZE - 1;

	/* Shift all stack one element back. */
	CagdSrfFree(SEditUndoStack[0].Srf);
	if (SEditUndoStack[0].TSrfSkel != NULL)
	    TrimSrfFree(SEditUndoStack[0].TSrfSkel);
	for (i = 1; i < SRF_EDIT_UNDO_STACK_SIZE; i++) {
	    SEditUndoStack[i - 1] = SEditUndoStack[i];
	}
    }
    
    /* And push the new state. */
    SEditUndoStack[SEditUndoStackPushPtr].Srf = CagdSrfCopy(Srf);
    SEditUndoStack[SEditUndoStackPushPtr].TSrfSkel =
			      TSrfSkel != NULL ? TrimSrfCopy(TSrfSkel) : NULL;
    SEditUndoStack[SEditUndoStackPushPtr++].SrfEditParam = *SrfEditParam;

    /* Update the undo/redo pointer to the end of the stack. */
    SEditUndoStackPopPtr = SEditUndoStackPushPtr;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Undo one step in the undo stack, updating SrfEditParam and returning a   M
* pointer to the old surface.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfEditParam:  To update with the old state of surface editing params.   M
*   TSrfSkel:      Trimmed surface skeleton to update with old state.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The old surface (the object must be copied before use.) M
*	      NULL is returned if no undo to use any more (bottom of stack). M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditPushState, SEditRedoState, SEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditUndoState                                                           M
*****************************************************************************/
CagdSrfStruct *SEditUndoState(IGSrfEditParamStruct *SrfEditParam,
			      TrimSrfStruct **TSrfSkel)
{
    CagdSrfStruct *Srf;

    if (SEditUndoStackPopPtr > 0) {
	*SrfEditParam = SEditUndoStack[--SEditUndoStackPopPtr].SrfEditParam;
	*TSrfSkel = SEditUndoStack[SEditUndoStackPopPtr].TSrfSkel;
	Srf = SEditUndoStack[SEditUndoStackPopPtr].Srf;
	UpdateCrntObjType(IGSrfEditCurrentObj, Srf, *TSrfSkel,
			  *TSrfSkel == NULL ? IP_OBJ_SURFACE : IP_OBJ_TRIMSRF);
	return Srf;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redo one step in the undo stack, updating SrfEditParam and returning a   M
* pointer to the old surface.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfEditParam: To update with the old state of the surface editing params.M
*   TSrfSkel:      Trimmed surface skeleton to update with old state.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The old surface (the object must be copied before use.) M
*	      NULL is returned if no redo to use any more (bottom of stack). M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditPushState, SEditRedoState, SEditFreeStateStack                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditRedoState                                                           M
*****************************************************************************/
CagdSrfStruct *SEditRedoState(IGSrfEditParamStruct *SrfEditParam,
			      TrimSrfStruct **TSrfSkel)
{
    CagdSrfStruct *Srf;

    if (SEditUndoStackPopPtr < SEditUndoStackPushPtr) {
	*SrfEditParam = SEditUndoStack[SEditUndoStackPopPtr].SrfEditParam;
	*TSrfSkel = SEditUndoStack[SEditUndoStackPopPtr].TSrfSkel;
	Srf = SEditUndoStack[SEditUndoStackPopPtr++].Srf;
	UpdateCrntObjType(IGSrfEditCurrentObj, Srf, *TSrfSkel,
			  *TSrfSkel == NULL ? IP_OBJ_SURFACE : IP_OBJ_TRIMSRF);
	return Srf;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert between trimmed and untrimmed surface objects.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Obj:           Object to convert its type.                               *
*   Srf:	   surface tp update into old object.                        *
*   TSrfSkel:      Trimmed surface skeleton to update into old object.	     *
*   NewObjType:    New type to assign to object.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void UpdateCrntObjType(IPObjectStruct *Obj,
			      CagdSrfStruct *Srf,
			      TrimSrfStruct *TSrfSkel,
			      IPObjStructType NewObjType)
{
    if (Obj -> ObjType == NewObjType)
        return;

    /* Assuming object types of surface and trimmed surface only. */
    switch (Obj -> ObjType) {
	case IP_OBJ_SURFACE:
	    CagdSrfFreeList(Obj -> U.Srfs);
	    Obj -> U.TrimSrfs =
	        TrimSrfNew(CagdSrfCopy(Srf),
			   TrimCrvCopyList(TSrfSkel -> TrimCrvList), FALSE);
	    break;
	case IP_OBJ_TRIMSRF:
	    TrimSrfFreeList(Obj -> U.TrimSrfs);
	    Obj -> U.Srfs = CagdSrfCopy(Srf);
	    break;
	default:
	    IGIritError("UpdateCrntObjType Error: Undefine surface type");
	    break;
    }
    Obj -> ObjType = NewObjType;

    /* Free all polyline/gon approximated geometry of this surface. */
    IGActiveFreePolyIsoAttribute(Obj, TRUE, TRUE, TRUE, TRUE);
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
*   SEditUndoState, SEditRedoState, SEditPushState                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditFreeStateStack                                                      M
*****************************************************************************/
void SEditFreeStateStack(void)
{
    int i;

    for (i = 0; i < SEditUndoStackPushPtr; i++) {
	CagdSrfFree(SEditUndoStack[i].Srf);
	if (SEditUndoStack[i].TSrfSkel != NULL)
	    TrimSrfFree(SEditUndoStack[i].TSrfSkel);
    }

    SEditUndoStackPushPtr = 0;
    SEditUndoStackPopPtr = -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the knot vector of the given surface to follow the EndCond end    M
* condition and Param parametrization prescription.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To update.  The knot vector of the surface my be NULL and a   M
*	       new knot vector will be allocated for it in that case.        M
*   EndCond:   End condition to employ.                                      M
*   Dir:       Either U or V.		                                     M
*   Param:     Parametrization to use.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditUpdateKnotVector                                                    M
*****************************************************************************/
void SEditUpdateKnotVector(CagdSrfStruct *Srf,
			   CagdEndConditionType EndCond,
			   CagdSrfDirType Dir,
			   CagdParametrizationType Param)
{
    int i,
	PtStep = Dir == CAGD_CONST_U_DIR ? CAGD_NEXT_U(Srf) : CAGD_NEXT_V(Srf),
    	Order = Dir == CAGD_CONST_U_DIR ? Srf -> UOrder : Srf -> VOrder,
	Length = Dir == CAGD_CONST_U_DIR ? Srf -> ULength : Srf -> VLength;
    CagdRType *Lens, Sum, *KV, UMin, UMax, VMin, VMax,
	**KVP = Dir == CAGD_CONST_U_DIR ? &Srf -> UKnotVector
                                        : &Srf -> VKnotVector,
	**Points = Srf -> Points;
    CagdPType PrevPt, Pt;
    CagdPointType
	PType = Srf -> PType;

    if (Srf -> GType != CAGD_SBSPLINE_TYPE)
	return;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    if (*KVP != NULL) {
	IritFree(*KVP);
	*KVP = NULL;
    }

    switch (EndCond) {
	default:
	case CAGD_END_COND_OPEN:
	    *KVP = BspKnotUniformOpen(Length, Order, NULL);
	    break;
	case CAGD_END_COND_FLOAT:
	    *KVP = BspKnotUniformFloat(Length, Order, NULL);
	    break;
	case CAGD_END_COND_PERIODIC:
	    *KVP = BspKnotUniformPeriodic(Length, Order, NULL);
	    break;
    }
    KV = *KVP;

    switch (Param) {
	default:
	case CAGD_UNIFORM_PARAM:
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	case CAGD_CHORD_LEN_PARAM:
	    /* Compute the lengths of first row/column. */
	    Lens = IritMalloc(sizeof(CagdRType) * Length);
	    CagdCoerceToE3(PrevPt, Points, 0, PType);
	    for (i = 1; i < Length; i++) {
		CagdCoerceToE3(Pt, Points, i * PtStep, PType);
		Lens[i - 1] = IRIT_PT_PT_DIST(PrevPt, Pt);
		IRIT_PT_COPY(PrevPt, Pt);
	    }
	    Lens[i - 1] = 0.0;

	    /* Update the knots based on these lengths. */
	    for (i = 0, Sum = 0.0; i < Order - 1; i++)
		Sum += Lens[i];
	    for (i = Order; i <= Length; i++) {
		KV[i] = KV[i - 1] +
			  ((Param == CAGD_CHORD_LEN_PARAM) ? Sum : sqrt(Sum));
		Sum += Lens[i - 1] - Lens[i - Order];
	    }
	    
	    switch (EndCond) {
	        default:
	        case CAGD_END_COND_OPEN:
		    for (i = Length + 1; i < Length + Order; i++)
			KV[i] = KV[Length];
		    break;
		case CAGD_END_COND_FLOAT:
		    for (i = Length + 1; i < Length + Order; i++)
			KV[i] = KV[Length] + i - Length;
		    break;
		case CAGD_END_COND_PERIODIC:
		    for (i = Length + 1;
			 i < Length + Order + Order - 1;
			 i++)
			KV[i] = KV[i - 1] + (KV[i - Length] -
					     KV[i - Length - 1]);
		    break;
	    }
	    break;
    }

#ifdef SRF_DO_ZERO_ONE_DOMAIN
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				CAGD_SRF_UPT_LST_LEN(Srf) + Srf -> UOrder,
				0.0, 1.0);
	    break;
	case CAGD_CONST_V_DIR:
	    BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder,
				0.0, 1.0);
	    break;
	default:
	    break;
    }
#else
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				CAGD_SRF_UPT_LST_LEN(Srf) + Srf -> UOrder,
				UMin, UMax);
	    break;
	case CAGD_CONST_V_DIR:
	    BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder,
				VMin, VMax);
	    break;
	default:
	    break;
    }
#endif /* SRF_DO_ZERO_ONE_DOMAIN */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Refine the surface at the given t value, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To treat and refine at t.		                     M
*   t:          The parameter(s) at which to insert the new knot(s).         M
*   n:          Size of vector t - number of new knots.			     M
*   Dir:        Either U or V.		                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditDomainFromSrf, SEditSubdivSrf, SEditMergeSrfs, SEditTrimSrf	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditRefineSrf                                                           M
*****************************************************************************/
int SEditRefineSrf(CagdSrfStruct **Srf,
		   CagdRType *t,
		   int n,
		   CagdSrfDirType Dir)
{
    int i;
    CagdSrfStruct
	*OrigSrf = *Srf;
    
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    for (i = 0; i < n; i++)
	        t[i] = IRIT_BOUND(t[i], 0.0, 1.0);
	    break;
	case CAGD_CONST_V_DIR:
	    for (i = 0; i < n; i++)
	        t[i] = IRIT_BOUND(t[i], 0.0, 1.0);
	    break;
	default:
	    return FALSE;
    }

    *Srf = CagdSrfRefineAtParams(OrigSrf, Dir, FALSE, t, n);

    CagdSrfFree(OrigSrf);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivide the surface at the given t value, in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To treat and subdivide.  Returned is a list of two surfaces  M
*		that are the result of the subdivision.			     M
*   t:          The parameter at which to subdivide.		             M
*   Continuity: Continuity of subdivided point.  Note that if continuity is  M
*               non negative, the surfaces are not split.		     M
*   Dir:        Either U or V.		                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditRefineSrf, SEditDomainFromSrf, SEditMergeSrfs, SEditTrimSrf	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditSubdivSrf                                                           M
*****************************************************************************/
int SEditSubdivSrf(CagdSrfStruct **Srf,
		   CagdRType t,
		   int Continuity,
		   CagdSrfDirType Dir)
{
    int Mult;
    CagdSrfStruct *TSrf,
	*OrigSrf = *Srf;
    
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    if (t <= 0.0 || t >= 1.0)
	        return FALSE;
	    Mult = (*Srf) -> UOrder - Continuity - 1;
	    break;
	case CAGD_CONST_V_DIR:
	    if (t <= 0.0 || t >= 1.0)
	        return FALSE;
	    Mult = (*Srf) -> VOrder - Continuity - 1;
	    break;
	default:
	    return FALSE;
    }

    if (Continuity < 0) {
	*Srf = CagdSrfSubdivAtParam(OrigSrf, t, Dir);
	CagdSrfFree(OrigSrf);
    }
    else {
	int i;
	CagdRType *TVec;

	if (Mult > 0) {
	    TVec = (CagdRType *) IritMalloc(Mult * sizeof(CagdRType));

	    for (i = 0; i < Mult; i++)
	        TVec[i] = t;

	    *Srf = CagdSrfRefineAtParams(OrigSrf, Dir, FALSE, TVec, Mult);

	    CagdSrfFree(OrigSrf);
	    IritFree(TVec);
	}
    }

    for (TSrf = *Srf; TSrf != NULL; TSrf = TSrf -> Pnext) {
        switch (Dir) {
	    case CAGD_CONST_U_DIR:
	        BspKnotAffineTransOrder2(TSrf -> UKnotVector, TSrf -> UOrder,
				CAGD_SRF_UPT_LST_LEN(TSrf) + TSrf -> UOrder,
				0.0, 1.0);
		break;
	    case CAGD_CONST_V_DIR:
		BspKnotAffineTransOrder2(TSrf -> VKnotVector, TSrf -> VOrder,
				CAGD_SRF_VPT_LST_LEN(TSrf) + TSrf -> VOrder,
				0.0, 1.0);
		break;
	    default:
		return FALSE;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merge Srf2 to Srf1 and update Srf1 in place.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1:      First surface of the merge.  Result is updated here in place. M
*   Srf2:      Second surface of the merge.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditAddPoint, SEditRefineSrf, SEditSubdivSrf, SEditTrimSrf		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditMergeSrfs                                                           M
*****************************************************************************/
int SEditMergeSrfs(CagdSrfStruct **Srf1, CagdSrfStruct *Srf2)
{
    int i, j,
	Index1 = 0,
	Index2 = 0;
    CagdRType *R, UMin, UMax, VMin, VMax, DistSqr;
    CagdPType P1[4], P2[4];			 /* For the four boundaries. */
    CagdSrfDirType Dir;
    CagdSrfStruct *TSrf1, *TSrf2;

    CagdSrfDomain(*Srf1, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(*Srf1, UMin, (VMin + VMax) * 0.5); /* UMin */
    CagdCoerceToE3(P1[0], &R, -1, (*Srf1) -> PType);
    R = CagdSrfEval(*Srf1, UMax, (VMin + VMax) * 0.5); /* UMax */
    CagdCoerceToE3(P1[1], &R, -1, (*Srf1) -> PType);
    R = CagdSrfEval(*Srf1, (UMin + UMax) * 0.5, VMin); /* VMin */
    CagdCoerceToE3(P1[2], &R, -1, (*Srf1) -> PType);
    R = CagdSrfEval(*Srf1, (UMin + UMax) * 0.5, VMax); /* VMax */
    CagdCoerceToE3(P1[3], &R, -1, (*Srf1) -> PType);

    CagdSrfDomain(Srf2, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(Srf2, UMin, (VMin + VMax) * 0.5);  /* UMin */
    CagdCoerceToE3(P2[0], &R, -1, Srf2 -> PType);
    R = CagdSrfEval(Srf2, UMax, (VMin + VMax) * 0.5);  /* UMax */
    CagdCoerceToE3(P2[1], &R, -1, Srf2 -> PType);
    R = CagdSrfEval(Srf2, (UMin + UMax) * 0.5, VMin);  /* VMin */
    CagdCoerceToE3(P2[2], &R, -1, Srf2 -> PType);
    R = CagdSrfEval(Srf2, (UMin + UMax) * 0.5, VMax);  /* VMax */
    CagdCoerceToE3(P2[3], &R, -1, Srf2 -> PType);

    DistSqr = IRIT_INFNTY;
    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	    SRF_CMP_DIST_SQR(P1[i], P2[j], i, j);

    if ((Index1 < 2) != (Index2 < 2)) {
        /* Attempt to merge U boundary of one to a V boundary of another. */
        Srf2 = CagdSrfReverse2(Srf2);		 /* Flip U and V in Srf2. */
	i = SEditMergeSrfs(Srf1, Srf2);
	CagdSrfFree(Srf2);
	return i;
    }

    CagdSrfDomain(*Srf1, &UMin, &UMax, &VMin, &VMax);
    switch (Index1) {
	default:
	case 0: /* UMin. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(*Srf1, UMin, VMin);
	    CagdCoerceToE3(P1[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(*Srf1, UMin, VMax);
	    CagdCoerceToE3(P1[1], &R, -1, (*Srf1) -> PType);

	    Dir = CAGD_CONST_U_DIR;
            TSrf1 = CagdSrfReverseDir(*Srf1, CAGD_CONST_U_DIR);
	    CagdSrfFree(*Srf1);
	    break;
	case 1: /* UMax. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(*Srf1, UMax, VMin);
	    CagdCoerceToE3(P1[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(*Srf1, UMax, VMax);
	    CagdCoerceToE3(P1[1], &R, -1, (*Srf1) -> PType);

	    Dir = CAGD_CONST_U_DIR;
	    TSrf1 = *Srf1;
	    break;
	case 2: /* VMin. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(*Srf1, UMin, VMin);
	    CagdCoerceToE3(P1[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(*Srf1, UMax, VMin);
	    CagdCoerceToE3(P1[1], &R, -1, (*Srf1) -> PType);

	    Dir = CAGD_CONST_V_DIR;
            TSrf1 = CagdSrfReverseDir(*Srf1, CAGD_CONST_V_DIR);
	    CagdSrfFree(*Srf1);
	    break;
	case 3: /* VMax. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(*Srf1, UMin, VMax);
	    CagdCoerceToE3(P1[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(*Srf1, UMax, VMax);
	    CagdCoerceToE3(P1[1], &R, -1, (*Srf1) -> PType);

	    Dir = CAGD_CONST_V_DIR;
	    TSrf1 = *Srf1;
	    break;
    }

    CagdSrfDomain(Srf2, &UMin, &UMax, &VMin, &VMax);
    switch (Index2) {
	default:
	case 0: /* UMin. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(Srf2, UMin, VMin);
	    CagdCoerceToE3(P2[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(Srf2, UMin, VMax);
	    CagdCoerceToE3(P2[1], &R, -1, (*Srf1) -> PType);

            TSrf2 = CagdSrfCopy(Srf2);
	    break;
	case 1: /* UMax. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(Srf2, UMax, VMin);
	    CagdCoerceToE3(P2[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(Srf2, UMax, VMax);
	    CagdCoerceToE3(P2[1], &R, -1, (*Srf1) -> PType);

	    TSrf2 = CagdSrfReverseDir(Srf2, CAGD_CONST_U_DIR);
	    break;
	case 2: /* VMin. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(Srf2, UMin, VMin);
	    CagdCoerceToE3(P2[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(Srf2, UMax, VMin);
	    CagdCoerceToE3(P2[1], &R, -1, (*Srf1) -> PType);

            TSrf2 = CagdSrfCopy(Srf2);
	    break;
	case 3: /* VMax. */
	    /* Compute the end points of the boundary (boundary flipped!?). */
	    R = CagdSrfEval(Srf2, UMin, VMax);
	    CagdCoerceToE3(P2[0], &R, -1, (*Srf1) -> PType);
	    R = CagdSrfEval(Srf2, UMax, VMax);
	    CagdCoerceToE3(P2[1], &R, -1, (*Srf1) -> PType);

	    TSrf2 = CagdSrfReverseDir(Srf2, CAGD_CONST_V_DIR);
	    break;
    }

    /* If boundary is flipped - flip Srf2. */
    if (IRIT_PT_PT_DIST_SQR(P1[0], P2[0]) + IRIT_PT_PT_DIST_SQR(P1[1], P2[1]) >
	IRIT_PT_PT_DIST_SQR(P1[0], P2[1]) + IRIT_PT_PT_DIST_SQR(P1[1], P2[0])) {
        CagdSrfStruct
	    *TSrf = CagdSrfReverseDir(TSrf2, CAGD_OTHER_DIR(Dir));

	CagdSrfFree(TSrf2);
	TSrf2 = TSrf;
    }

    *Srf1 = CagdMergeSrfSrf(TSrf1, TSrf2, Dir, IRIT_APX_EQ(DistSqr, 0.0), TRUE);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivide the surface at the given t value, in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To treat and subdivide.  Returned is the surface restricted  M
*		the domain from t1 to t2, in direction Dir.   		     M
*   t1, t2:     The parameters of the domain to extract.	             M
*   Dir:        Either U or V.		                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditRefineSrf, SEditSubdivSrf, SEditTrimSrf			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditDomainFromSrf                                                       M
*****************************************************************************/
int SEditDomainFromSrf(CagdSrfStruct **Srf,
		       CagdRType t1,
		       CagdRType t2,
		       CagdSrfDirType Dir)
{
    CagdSrfStruct
	*OrigSrf = *Srf;
    
    if (t1 <= 0.0 || t1 >= 1.0 || t2 <= 0.0 || t2 >= 1.0)
        return FALSE;
    
    *Srf = CagdSrfRegionFromSrf(OrigSrf, t1, t2, Dir);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    BspKnotAffineTransOrder2((*Srf) -> UKnotVector, (*Srf) -> UOrder,
				CAGD_SRF_UPT_LST_LEN(*Srf) + (*Srf) -> UOrder,
				0.0, 1.0);
	    break;
	case CAGD_CONST_V_DIR:
	    BspKnotAffineTransOrder2((*Srf) -> VKnotVector, (*Srf) -> VOrder,
				CAGD_SRF_VPT_LST_LEN(*Srf) + (*Srf) -> VOrder,
				0.0, 1.0);
	    break;
	default:
	    return FALSE;
    }

    CagdSrfFree(OrigSrf);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Refine the surface at the given t value, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Object to trim.  Either a regular or a trimmed surface.      M
*   UVCrv:      New curve in parameteric space to trim with.                 M
*   First:	If two resulting srfs, pick first if TRUE, second otherwise. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SEditDomainFromSrf, SEditSubdivSrf, SEditMergeSrfs			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditTrimSrf                                                             M
*****************************************************************************/
int SEditTrimSrf(IPObjectStruct *SrfObj, CagdCrvStruct *UVCrv, CagdBType First)
{
    CagdRType *R, TMin, TMax, UMin, UMax, VMin, VMax;
    CagdUVType UVMin, UVMax;
    CagdSrfStruct
	*Srf = SRF_EDIT_GET_SRF_OBJ(SrfObj);

    CagdCrvDomain(UVCrv, &TMin, &TMax);
    R = CagdCrvEval(UVCrv, TMin);
    CagdCoerceToE2(UVMin, &R, -1, UVCrv -> PType);
    R = CagdCrvEval(UVCrv, TMax);
    CagdCoerceToE2(UVMax, &R, -1, UVCrv -> PType);

    if (!CAGD_IS_PERIODIC_CRV(UVCrv) &&
	!((IRIT_APX_EQ_EPS(UVMin[0], 0.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMin[0], 1.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMin[1], 0.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMin[1], 1.0, SRF_EDIT_TRIM_CRV_EPS)) &&
	  (IRIT_APX_EQ_EPS(UVMax[0], 0.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMax[0], 1.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMax[1], 0.0, SRF_EDIT_TRIM_CRV_EPS) ||
	   IRIT_APX_EQ_EPS(UVMax[1], 1.0, SRF_EDIT_TRIM_CRV_EPS)))) {
	IGSrfEditPlaceMessage("Curve not a proper trimming curve");
	return FALSE;
    }

    /* Keep the domain zero to one for the short detachment period. */
    UMin = SEditUMin;
    UMax = SEditUMax;
    VMin = SEditVMin;
    VMax = SEditVMax;
    SEditUMin = SEditVMin = 0.0;
    SEditUMax = SEditVMax = 1.0;

    if (IP_IS_SRF_OBJ(SrfObj)) {
	/* This is a regular surface - convert into a trimmed srf. */
	if (CAGD_IS_PERIODIC_CRV(UVCrv)) {
	    /* Reclaim the object into a trimmed surface object. */
	    SrfObj -> ObjType = IP_OBJ_TRIMSRF;
	    SrfObj -> U.TrimSrfs = TrimSrfNew2(CagdSrfCopy(Srf),
					       CagdCrvCopy(UVCrv), First);
	}
	else {
	    IPPolygonStruct
		*Pl = IPCurve2Polylines(UVCrv, IGGlblPllnFineness,
					IGGlblPolylineOptiApprox);
	    IPVertexStruct
		*V1 = Pl -> PVertex,
		*V2 = IPGetLastVrtx(V1);
	    TrimSrfStruct *TSrfs;

	    if (IRIT_APX_EQ_EPS(V1 -> Coord[0], 0.0, SRF_EDIT_TRIM_CRV_EPS))
		V1 -> Coord[0] = 0.0;
	    else if (IRIT_APX_EQ_EPS(V1 -> Coord[0], 1.0, SRF_EDIT_TRIM_CRV_EPS))
		V1 -> Coord[0] = 1.0;
	    else if (IRIT_APX_EQ_EPS(V1 -> Coord[1], 0.0, SRF_EDIT_TRIM_CRV_EPS))
		V1 -> Coord[1] = 0.0;
	    else if (IRIT_APX_EQ_EPS(V1 -> Coord[1], 1.0, SRF_EDIT_TRIM_CRV_EPS))
		V1 -> Coord[1] = 1.0;

	    if (IRIT_APX_EQ_EPS(V2 -> Coord[0], 0.0, SRF_EDIT_TRIM_CRV_EPS))
		V2 -> Coord[0] = 0.0;
	    else if (IRIT_APX_EQ_EPS(V2 -> Coord[0], 1.0, SRF_EDIT_TRIM_CRV_EPS))
		V2 -> Coord[0] = 1.0;
	    else if (IRIT_APX_EQ_EPS(V2 -> Coord[1], 0.0, SRF_EDIT_TRIM_CRV_EPS))
		V2 -> Coord[1] = 0.0;
	    else if (IRIT_APX_EQ_EPS(V2 -> Coord[1], 1.0, SRF_EDIT_TRIM_CRV_EPS))
		V2 -> Coord[1] = 1.0;

	    TSrfs = TrimSrfsFromContours(Srf, Pl);

	    if (TSrfs != NULL) {
		/* Reclaim the object into a trimmed surface object. */

		if (TSrfs -> Pnext != NULL && !First) {
		    /* Pick second trimmed surface. */
		    SrfObj -> U.TrimSrfs = TSrfs -> Pnext;
		    TrimSrfFree(TSrfs);
		}
		else {
		    /* Pick first trimmed surface. */
		    SrfObj -> U.TrimSrfs = TSrfs;
		    TrimSrfFree(TSrfs -> Pnext);
		    TSrfs -> Pnext = NULL;
		}

		SrfObj -> ObjType = IP_OBJ_TRIMSRF;
		IGRedrawViewWindow();
	    }
	}
    }
    else {
	/* It is a trimmed surface already. */
	TrimCrvStruct *TrimCrv;
	CagdCrvStruct *Crv,
	    *TCrvs = TrimGetTrimmingCurves(SrfObj -> U.TrimSrfs, TRUE, FALSE);

	/* Examine if we intersect existing trimming curves. */
	for (Crv = TCrvs; Crv != NULL; Crv = Crv -> Pnext) {
	    CagdPtStruct
		*Pts = CagdCrvCrvInter(Crv, UVCrv, IRIT_EPS);

	    if (Pts != NULL) {
		CagdPtFreeList(Pts);
		CagdCrvFreeList(TCrvs);
		IGSrfEditPlaceMessage("New trimming crv intersects old trimming crvs");
		return FALSE;
	    }
	}
	CagdCrvFreeList(TCrvs);

	if (CAGD_IS_PERIODIC_CRV(UVCrv)) {
	    /* Add the UV curve as a new trimming curve of the surface. */
	    TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(UVCrv), NULL));
	    TrimCrv -> Pnext = SrfObj -> U.TrimSrfs -> TrimCrvList;
	    SrfObj -> U.TrimSrfs -> TrimCrvList = TrimCrv;
	}
	else {
	    IGSrfEditPlaceMessage("Boundary trimming is support at first only");
	}
    }

    /* Free all polyline/gon approximated geometry of this surface. */
    IGActiveFreePolyIsoAttribute(SrfObj, TRUE, TRUE, TRUE, TRUE);

    /* Update the trimming information on the editted surface. */
    if (SEditTrimSrfSkel != NULL)
	TrimSrfFree(SEditTrimSrfSkel);
    SEditTrimSrfSkel = TrimSrfCopy(SrfObj -> U.TrimSrfs);
    CagdSrfFree(SEditTrimSrfSkel -> Srf);
    SEditTrimSrfSkel -> Srf = NULL;

    /* Resore the domain... */
    SEditUMin = UMin;
    SEditUMax = UMax;
    SEditVMin = VMin;
    SEditVMax = VMax;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the UV parameter value of the closest point on Srf to Pt.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To find the parameter value of the closest point to Pt         M
*   MousePt:  To find the closest point on Srf to - a point on ray.          M
*   MouseDir: To find the closest point on Srf to - direction of ray.        M
*   Dir:      Will be set with the direction of detected isocurve, U or V.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Statically allocated UV Parameter values of closest point M
*	 	   on Srf to Pt, or NULL if none found.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditFindClosestParameter                                                M
*****************************************************************************/
CagdRType *SEditFindClosestParameter(CagdSrfStruct *Srf,
				     CagdPType MousePt,
				     CagdVType MouseDir,
				     CagdSrfDirType *Dir)
{
    IRIT_STATIC_DATA CagdUVType RetUV;
    CagdRType MinDist, IndexFrac, HitDepth;
    CagdPType MinPt;
    IPPolygonStruct *Pls, *MinPl;

    Pls = IPSurface2Polygons(Srf, IGGlblFourPerFlat,
			     IGGlblPlgnFineness, TRUE, FALSE,
			     IGGlblPolygonOptiApprox);

    MinDist = UserMinDistLinePolygonList(MousePt, MouseDir, Pls, &MinPl,
					 MinPt, &HitDepth, &IndexFrac);

    /* See if we can recover UV parameter values. */
    RetUV[0] = RetUV[1] = -999;
    if (MinPl != NULL) {
        IPVertexStruct
	    *V = IPAllocVertex2(NULL);
	float *UV;

	IRIT_PT_COPY(V -> Coord, MinPt);
	if (GMInterpVrtxUVFromPl(V, MinPl) &&
	    (UV = AttrGetUVAttrib(V -> Attr, "uvvals")) != NULL) {
	    RetUV[0] = UV[0];
	    RetUV[1] = UV[1];
	}
    }

    IPFreePolygonList(Pls);

    return RetUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the indices of the closest control point on Srf to MousePt/Dir. M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To find the index of the control closest point to MousePt/Dir. M
*   MousePt:  To find closest control point of Srf - a point on mouse ray.   M
*   MouseDir: To find closest control point of Srf - direction of mouse ray. M
*   FoundRational:  Found a rational circle that index.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Index of closest control point on Srf to MousePt/Dir	     M
*	     or -1 if nothing was picked.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditFindClosestControlPoint                                             M
*****************************************************************************/
int SEditFindClosestControlPoint(CagdSrfStruct *Srf,
				 CagdPType MousePt,
				 CagdPType MouseDir,
				 int *FoundRational)
{
    int i,
        Index = -1,
        WeightIndex = -1;
    CagdRType Dist, DistWeight,
	MinDist = IGGlblMinPickDist / MatScaleFactorMatrix(IPViewMat),
        MinWeight = MinDist,
        **Points = Srf -> Points;

    *FoundRational = FALSE;

    Srf = CagdSrfMatTransform(Srf, IPViewMat);

    for (i = 0; i < Srf -> ULength * Srf -> VLength; i++) {
	CagdPType SrfCtlPt;

	if (CAGD_IS_RATIONAL_SRF(Srf)) {
	    SrfCtlPt[0] = Points[1][i] / Points[0][i];
	    SrfCtlPt[1] = Points[2][i] / Points[0][i];
	    SrfCtlPt[2] = Points[3][i] / Points[0][i];
	}
	else {
	    SrfCtlPt[0] = Points[1][i];
	    SrfCtlPt[1] = Points[2][i];
	    SrfCtlPt[2] = Points[3][i];
	}
	
	Dist = GMDistPointLine(SrfCtlPt, MousePt, MouseDir);
	if (MinDist > Dist) {
	    MinDist = Dist;
	    Index = i;
	}

	if (CAGD_IS_RATIONAL_SRF(Srf)) {
	    DistWeight = IRIT_FABS(Dist - Points[0][i] *
						 IG_CRV_RATIONAL_CIRC_LENGTH);
	    if (MinWeight > DistWeight) {
		MinWeight = DistWeight;
		WeightIndex = i;
	    }
	}	
    }

    CagdSrfFree(Srf);

    if (WeightIndex >= 0 && MinWeight < MinDist) {
	*FoundRational = TRUE;
	return WeightIndex;
    }
    else
	return Index;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the current constructed surface.  Invoked when screen is redrawn.   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditRedrawSrf                                                           M
*****************************************************************************/
void SEditRedrawSrf(void)
{
    IPObjectStruct *PObj;
    IrtHmgnMatType IritView;

    switch (IGGlblViewMode) {
        case IG_VIEW_ORTHOGRAPHIC:
            IRIT_GEN_COPY(IritView, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IritView, IPViewMat, IPPrspMat);
	    break;
    }

    if (IGSrfEditCurrentSrf != NULL) {
        CagdSrfStruct
	    *TSrf = CagdSrfCopy(IGSrfEditCurrentSrf);

	/* Make a copy and replace to eliminate all local/internal caches... */
	CagdSrfFree(IGSrfEditCurrentSrf);
	IGSrfEditCurrentSrf = TSrf;

        if (IGGlblDrawSurfacePoly) {
	    PObj = IPAllocObject("", IP_OBJ_POLY, NULL);
	    IP_SET_POLYGON_OBJ(PObj);
	    if (SEditTrimSrfSkel != NULL) {
	        SEditTrimSrfSkel -> Srf = IGSrfEditCurrentSrf;
	        PObj -> U.Pl = IPTrimSrf2Polygons(SEditTrimSrfSkel,
						  IGGlblFourPerFlat,
						  IGGlblPlgnFineness,
						  FALSE, TRUE,
						  IGGlblPolygonOptiApprox);
	        SEditTrimSrfSkel -> Srf = NULL;
	    }
	    else
	        PObj -> U.Pl = IPSurface2Polygons(IGSrfEditCurrentSrf,
						  IGGlblFourPerFlat,
						  IGGlblPlgnFineness,
						  FALSE, TRUE,
						  IGGlblPolygonOptiApprox);

	    AttrSetObjectRGBColor(PObj,
				  IGGlblHighlight1Color[0],
				  IGGlblHighlight1Color[1],
				  IGGlblHighlight1Color[2]);
	    IGViewObject(PObj, IritView);
	    IPFreeObject(PObj);
	}

	if (IGGlblDrawSurfaceWire) {
	    int NumOfIso[2];

	    NumOfIso[0] = -IGGlblNumOfIsolines;
	    NumOfIso[1] = -IGGlblNumOfIsolines;

	    PObj = IPAllocObject("", IP_OBJ_POLY, NULL);
	    IP_SET_POLYLINE_OBJ(PObj);
	    if (SEditTrimSrfSkel != NULL) {
	        SEditTrimSrfSkel -> Srf = IGSrfEditCurrentSrf;
	        PObj -> U.Pl = IPTrimSrf2Polylines(SEditTrimSrfSkel,
						   NumOfIso,
						   IGGlblPllnFineness,
						   IGGlblPolylineOptiApprox,
						   TRUE, TRUE);
	        SEditTrimSrfSkel -> Srf = NULL;
	    }
	    else
	        PObj -> U.Pl = IPSurface2Polylines(IGSrfEditCurrentSrf,
						   NumOfIso,
						   IGGlblPllnFineness,
						   IGGlblPolylineOptiApprox);

	    AttrSetObjectRGBColor(PObj,
				  IGGlblHighlight1Color[0],
				  IGGlblHighlight1Color[1],
				  IGGlblHighlight1Color[2]);
	    IGViewObject(PObj, IritView);
	    IPFreeObject(PObj);
	}

	if (IGSrfEditDrawMesh && IGSrfEditParam.Rational) {
	    int i;
	    IrtRType
	        **Points = IGSrfEditCurrentSrf -> Points,
		Rad = IG_CRV_RATIONAL_CIRC_LENGTH,
		*w = Points[0],
		*x = Points[1],
		*y = Points[2],
		*z = Points[3];
	    IrtHmgnMatType RotMat, RotInvMat;

	    /* Extract only the rotation matrix and prepare its inverse: */
	    MatRotateFactorMatrix(IPViewMat, RotMat);
	    MatTranspMatrix(RotMat, RotInvMat);	     /* Compute the inverse. */

	    /* Draw all rational values as circles around the ctl pts. */
	    for (i = 0;
		 i < IGSrfEditCurrentSrf -> ULength *
		     IGSrfEditCurrentSrf -> VLength;
		 i++, w++) {
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

	if (IGGlblDrawSurfaceMesh || IGSrfEditDrawMesh) {
	    PObj = IPGenPOLYObject(IPSurface2CtlMesh(IGSrfEditCurrentSrf));
	    IP_SET_POLYLINE_OBJ(PObj);
	    AttrSetObjectRGBColor(PObj,
				  IGGlblHighlight1Color[0],
				  IGGlblHighlight1Color[1],
				  IGGlblHighlight1Color[2]);
	    IGViewObject(PObj, IritView);
	    IPFreeObject(PObj);
	}
    }

    /* Draw the multiresolution region, if necessary. */
    if ((PObj = SEditMultiResViewRegion(IGSrfEditCurrentSrf,
					SEditMultiResKvs)) != NULL) {
	AttrSetObjectRealAttrib(PObj, "dwidth", SRF_MR_REGION_WIDTH);
	AttrSetObjectRGBColor(PObj,
			      IGGlblHighlight1Color[0],
			      IGGlblHighlight1Color[1],
			      IGGlblHighlight1Color[2]);
        IGViewObject(PObj, IritView);
	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Decomposes the knot vectors of the given surface to a hierarchy of multi M
* resolution levels.							     M
*   Updates the global data structures of SEditMRKvs			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:                   Whose knot vector is to be decomposed.	     M
*   Discont:               If TRUE, preserves the discontinuities.	     M
*   UPeriodic, VPeriodic:  If TRUE, Performs a periodic decomposition.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   SEditMultiResKvsStruct *:  Hierarchies in both U and V.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditMultiResPrepKVs                                                     M
*****************************************************************************/
SEditMultiResKvsStruct *SEditMultiResPrepKVs(CagdSrfStruct *Srf,
					     int Discont,
					     int UPeriodic,
					     int VPeriodic)
{
    SEditMultiResKvsStruct
	*MultiResKvs = (SEditMultiResKvsStruct *)
	    IritMalloc(sizeof(SEditMultiResKvsStruct));
    CEditMultiResKvsStruct *CEditMultiResKvs;

    /* Do the U decomposition. */
    CEditMultiResKvs = CEditMultiResPrepKVs(Srf -> UKnotVector,
					    CAGD_SRF_UPT_LST_LEN(Srf),
					    Srf -> UOrder,
					    Discont, UPeriodic, FALSE);
    MultiResKvs -> UKvs = CEditMultiResKvs -> Kvs;
    MultiResKvs -> NumUKvs = CEditMultiResKvs -> NumKvs;
    IritFree(CEditMultiResKvs);

    /* Do the V decomposition. */
    CEditMultiResKvs = CEditMultiResPrepKVs(Srf -> VKnotVector,
					    CAGD_SRF_VPT_LST_LEN(Srf),
					    Srf -> VOrder,
					    Discont, VPeriodic, FALSE);
    MultiResKvs -> VKvs = CEditMultiResKvs -> Kvs;
    MultiResKvs -> NumVKvs = CEditMultiResKvs -> NumKvs;
    IritFree(CEditMultiResKvs);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintKVs, FALSE) {
	    int i, j;

	    for (i = 0; i < MultiResKvs -> NumUKvs; i++) {
	        IRIT_INFO_MSG_PRINTF("UKV - LEVEL %d (len = %d)\n",
			             i, MultiResKvs -> UKvs[i].KvLen);

		for (j = 0; j < MultiResKvs -> UKvs[i].KvLen; j++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ",
			                 MultiResKvs -> UKvs[i].Kv[j]);
		IRIT_INFO_MSG("\n\n");
	    }
	    for (i = 0; i < MultiResKvs -> NumVKvs; i++) {
	        IRIT_INFO_MSG_PRINTF("VKV - LEVEL %d (len = %d)\n",
			             i, MultiResKvs -> VKvs[i].KvLen);
	    
		for (j = 0; j < MultiResKvs -> VKvs[i].KvLen; j++)
		    IRIT_INFO_MSG_PRINTF("%0.7lf ",
			                 MultiResKvs -> VKvs[i].Kv[j]);
		IRIT_INFO_MSG("\n\n");
	    }
	}
    }
#endif /* DEBUG */

    return MultiResKvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Display the region of the edited surface representing the active MR      *
* region using the current MRLevel in IGSrfEditMRU/VLevel and parameter      *
* value t saved in SEditLastMRU/VParameter.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   None	                                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  The surface as an object.                             *
*                                                                            *
* KEYWORDS:                                                                  *
*   SEditMultiResViewRegion                                                  *
*****************************************************************************/
static IPObjectStruct *SEditMultiResViewRegion(CagdSrfStruct *Srf,
					       SEditMultiResKvsStruct *MultiKvs)
{
    int IMRLevel, IMRLevel1, Kv1Len, Kv2Len, Idx1, Idx2, UOrder, VOrder;
    CagdRType t, MRLevelFrac, t1, t2, *Kv1, *Kv2,
	MRULevel, MRVLevel;
    CagdSrfStruct *MRSrf, *TSrf;
    CEditMultiResOneKvStruct *UKvs, *VKvs;

    if (MultiKvs == NULL ||
	IGSrfEditParam.SrfState != IG_SRF_EDIT_STATE_EDIT ||
	IGSEditOperation != IG_SRF_EDIT_EVENT_MODIFY_SRF ||
	SEditLastMRUParameter == -IRIT_INFNTY ||
	SEditLastMRVParameter == -IRIT_INFNTY)
	return NULL;

    MRULevel = IGSrfEditMRULevel * (MultiKvs -> NumUKvs - 1),
    MRVLevel = IGSrfEditMRVLevel * (MultiKvs -> NumVKvs - 1);
    UKvs = MultiKvs -> UKvs;
    VKvs = MultiKvs -> VKvs;

    UOrder = Srf -> UOrder;
    VOrder = Srf -> VOrder;

    /* Extract the U Region: */
    MRULevel = IRIT_BOUND(MRULevel, 0, MultiKvs -> NumUKvs - 1);
    t = IRIT_BOUND(SEditLastMRUParameter, 0.0, 1.0);
    IMRLevel = (int) MRULevel;
    MRLevelFrac = MRULevel - IMRLevel;
    IMRLevel1 = IMRLevel + 1;
    IMRLevel1 = IRIT_BOUND(IMRLevel1, 0, MultiKvs -> NumUKvs - 1);

    Kv1 = UKvs[IMRLevel].Kv;
    Kv2 = UKvs[IMRLevel1].Kv;
    Kv1Len = UKvs[IMRLevel].KvLen;
    Kv2Len = UKvs[IMRLevel1].KvLen;

    /* Compute starting parameter value: */
    Idx1 = BspKnotLastIndexLE(Kv1, Kv1Len, t) - UOrder + 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len);
    Idx2 = BspKnotLastIndexLE(Kv2, Kv2Len, t) - UOrder + 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv1Len);
    t1 = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t1 = IRIT_BOUND(t1, 0.0, 1.0);
    
    /* Compute ending parameter value: */
    Idx1 = BspKnotLastIndexLE(Kv1, Kv1Len, t) + UOrder - 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len);
    Idx2 = BspKnotLastIndexLE(Kv2, Kv2Len, t) + UOrder - 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv2Len);
    t2 = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t2 = IRIT_BOUND(t2, 0.0, 1.0);

    TSrf = CagdSrfRegionFromSrf(Srf, t1, t2, CAGD_CONST_U_DIR);

    /* Extract the V Region: */
    MRVLevel = IRIT_BOUND(MRVLevel, 0, MultiKvs -> NumVKvs - 1);
    t = IRIT_BOUND(SEditLastMRVParameter, 0.0, 1.0);
    IMRLevel = (int) MRVLevel;
    MRLevelFrac = MRVLevel - IMRLevel;
    IMRLevel1 = IMRLevel + 1;
    IMRLevel1 = IRIT_BOUND(IMRLevel1, 0, MultiKvs -> NumVKvs - 1);

    Kv1 = VKvs[IMRLevel].Kv;
    Kv2 = VKvs[IMRLevel1].Kv;
    Kv1Len = VKvs[IMRLevel].KvLen;
    Kv2Len = VKvs[IMRLevel1].KvLen;

    /* Compute starting parameter value: */
    Idx1 = BspKnotLastIndexLE(Kv1, Kv1Len, t) - VOrder + 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len);
    Idx2 = BspKnotLastIndexLE(Kv2, Kv2Len, t) - VOrder + 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv1Len);
    t1 = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t1 = IRIT_BOUND(t1, 0.0, 1.0);
    
    /* Compute ending parameter value: */
    Idx1 = BspKnotLastIndexLE(Kv1, Kv1Len, t) + VOrder - 1;
    Idx1 = IRIT_BOUND(Idx1, 0, Kv1Len);
    Idx2 = BspKnotLastIndexLE(Kv2, Kv2Len, t) + VOrder - 1;
    Idx2 = IRIT_BOUND(Idx2, 0, Kv2Len);
    t2 = Kv1[Idx1] * (1.0 - MRLevelFrac) + Kv2[Idx2] * MRLevelFrac;
    t2 = IRIT_BOUND(t2, 0.0, 1.0);
    
    MRSrf = CagdSrfRegionFromSrf(TSrf, t1, t2, CAGD_CONST_V_DIR);
    CagdSrfFree(TSrf);

    return IPGenSRFObject(MRSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an additive term at multi resolution level MRLevel at           M
* parametric location u, v.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         The currently edited surface.				     M
*   SMultiKvs:   A structure holding surface MR knot sequences.		     M
*   UMRLevel:    U Multi resolution level.                                   M
*   VMRLevel:    V Multi resolution level.                                   M
*   u, v:        Parameter values of surface.                                M
*   TransDir:    Vectoric change at that location.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The additive vector field.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditMultiResModify                                                      M
*****************************************************************************/
CagdSrfStruct *SEditMultiResModify(CagdSrfStruct *Srf,
				   SEditMultiResKvsStruct *SMultiKvs,
				   IrtRType UMRLevel,
				   IrtRType VMRLevel,
				   CagdRType u,
				   CagdRType v,
				   IrtVecType TransDir)
{
    CagdBType
	UPeriodic = IGSrfEditParam.UEndCond == CAGD_END_COND_PERIODIC,
	VPeriodic = IGSrfEditParam.VEndCond == CAGD_END_COND_PERIODIC;
    int i, j, Len,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder;
    CagdRType t, **UPoints, **VPoints, **Points;
    CagdCrvStruct *UCrv, *VCrv;
    CagdSrfStruct *MRSrf;
    CEditMultiResKvsStruct CMultiKvs;

    /* As the U TransDir will be multiplied by the V TransDir. */
    if ((t = sqrt(IRIT_VEC_LENGTH(TransDir))) > 0.0) {
	t = 1.0 / sqrt(t);
	IRIT_VEC_SCALE(TransDir, t);
    }

    CMultiKvs.Kvs = SMultiKvs -> UKvs;
    CMultiKvs.NumKvs = SMultiKvs -> NumUKvs;
    UCrv = CEditMultiResModify(NULL, Srf -> UOrder, &CMultiKvs,
			       UMRLevel, u, TransDir);
    UPoints = UCrv -> Points;

    CMultiKvs.Kvs = SMultiKvs -> VKvs;
    CMultiKvs.NumKvs = SMultiKvs -> NumVKvs;
    VCrv = CEditMultiResModify(NULL, Srf -> VOrder, &CMultiKvs,
			       VMRLevel, v, TransDir);
    VPoints = VCrv -> Points;

    /* Create a surface as the outer product of UCrv and VCrv. Note that     */
    /* U/VCrv are never periodic and holds Order-1 more pts if Srf periodic. */
    MRSrf = CagdPeriodicSrfNew(IGSrfEditParam.Type,
			       IGSrfEditParam.Rational ? CAGD_PT_P3_TYPE
						       : CAGD_PT_E3_TYPE,
			       UPeriodic ? UCrv -> Length - UOrder + 1
					 : UCrv -> Length,
			       VPeriodic ? VCrv -> Length - VOrder + 1
					 : VCrv -> Length,
			       UPeriodic, VPeriodic);
    MRSrf -> UOrder = UOrder;
    MRSrf -> VOrder = VOrder;

    MRSrf -> UKnotVector = BspKnotCopy(NULL, UCrv -> KnotVector,
				       UCrv -> Order + UCrv -> Length); 
    MRSrf -> VKnotVector = BspKnotCopy(NULL, VCrv -> KnotVector,
				       VCrv -> Order + VCrv -> Length); 
    Points = MRSrf -> Points;
    Len = MRSrf -> ULength * MRSrf -> VLength;
    for (i = 1; i <= 3; i++)
        IRIT_ZAP_MEM(Points[i], sizeof(CagdRType) * Len);
    if (MRSrf -> PType == CAGD_PT_P3_TYPE)
        IRIT_ZAP_MEM(Points[0], sizeof(CagdRType) * Len);

    for (j = 0; j < MRSrf -> VLength; j++) {
	if (IRIT_FABS(VPoints[1][j]) > IRIT_UEPS ||
	    IRIT_FABS(VPoints[2][j]) > IRIT_UEPS ||
	    IRIT_FABS(VPoints[3][j]) > IRIT_UEPS) {
	    for (i = 0; i < MRSrf -> ULength; i++) {
		if (IRIT_FABS(UPoints[1][i]) > IRIT_UEPS ||
		    IRIT_FABS(UPoints[2][i]) > IRIT_UEPS ||
		    IRIT_FABS(UPoints[3][i]) > IRIT_UEPS) {
		    int k = CAGD_MESH_UV(MRSrf, i, j);
		    IrtRType 
			w = IRIT_SQR(SRF_MR_MOTION_SCALE),
			XMultFactor = UPoints[1][i] * VPoints[1][j] * w,
			YMultFactor = UPoints[2][i] * VPoints[2][j] * w,
			ZMultFactor = UPoints[3][i] * VPoints[3][j] * w;

		    switch (MRSrf -> PType) {
		        case CAGD_PT_E3_TYPE:
			    Points[1][k] += TransDir[0] * XMultFactor;
			    Points[2][k] += TransDir[1] * YMultFactor;
			    Points[3][k] += TransDir[2] * ZMultFactor;
			    break;
			case CAGD_PT_P3_TYPE:
			    w = Points[0][k];
			    Points[1][k] += w * TransDir[0] * XMultFactor;
			    Points[2][k] += w * TransDir[1] * YMultFactor;
			    Points[3][k] += w * TransDir[2] * ZMultFactor;
			    break;
			default:
			    IGIritError("SEditMultiResModify Error: Invalid point type");
			    break;
		    }
		}
	    }
	}
    }

    CagdCrvFree(UCrv);
    CagdCrvFree(VCrv);

    Srf = SymbSrfAdd(MRSrf, Srf);
    CagdSrfFree(MRSrf);

    return Srf;
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
static void UpdateMultResSrfDecomp(void)
{
    if (SEditMultiResKvs != NULL) {
	int i;

	for (i = 0; i < SEditMultiResKvs -> NumUKvs; i++)
	    IritFree(SEditMultiResKvs -> UKvs[i].Kv);
	IritFree(SEditMultiResKvs -> UKvs);

	for (i = 0; i < SEditMultiResKvs -> NumVKvs; i++)
	    IritFree(SEditMultiResKvs -> VKvs[i].Kv);
	IritFree(SEditMultiResKvs -> VKvs);

	IritFree(SEditMultiResKvs);
    }

    SEditMultiResKvs =
	SEditMultiResPrepKVs(IGSrfEditCurrentSrf,
			     FALSE,
			     IGSrfEditParam.UEndCond == CAGD_END_COND_PERIODIC,
			     IGSrfEditParam.VEndCond == CAGD_END_COND_PERIODIC);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attach to a surface object for further editing.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface object to attach to.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditAttachOldDirectly                                                   M
*****************************************************************************/
void SEditAttachOldDirectly(IPObjectStruct *SrfObj)
{
    IGSrfEditCurrentObj = SrfObj;
    IGSrfOriginalSurface = SRF_EDIT_GET_SRF_OBJ(IGSrfEditCurrentObj);

    IG_RST_HIGHLIGHT1_OBJ(IGSrfEditCurrentObj);

    if (IP_IS_TRIMSRF_OBJ(IGSrfEditCurrentObj)) {
	TrimSrfDomain(IGSrfEditCurrentObj -> U.TrimSrfs,
		      &SEditUMin, &SEditUMax, &SEditVMin, &SEditVMax);
	TrimAffineTransTrimCurves(IGSrfEditCurrentObj -> U.TrimSrfs
								-> TrimCrvList,
				  SEditUMin, SEditUMax,
				  SEditVMin, SEditVMax,
				  0.0, 1.0, 0.0, 1.0);

	SEditTrimSrfSkel = TrimSrfCopy(IGSrfEditCurrentObj -> U.TrimSrfs);
	CagdSrfFree(SEditTrimSrfSkel -> Srf);
	SEditTrimSrfSkel -> Srf = NULL;
    }
    else {
	CagdSrfDomain(IGSrfOriginalSurface,
		      &SEditUMin, &SEditUMax, &SEditVMin, &SEditVMax);
	SEditTrimSrfSkel = NULL;
    }

    if (CAGD_IS_BSPLINE_SRF(IGSrfOriginalSurface)) {
	BspKnotAffineTransOrder2(IGSrfOriginalSurface -> UKnotVector,
				 IGSrfOriginalSurface -> UOrder,
				 CAGD_SRF_UPT_LST_LEN(IGSrfOriginalSurface) +
			             IGSrfOriginalSurface -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(IGSrfOriginalSurface -> VKnotVector,
				 IGSrfOriginalSurface -> VOrder,
				 CAGD_SRF_VPT_LST_LEN(IGSrfOriginalSurface) +
			             IGSrfOriginalSurface -> VOrder,
				 0.0, 1.0);
    }

    IGSrfEditCurrentSrf = CagdSrfCopy(IGSrfOriginalSurface);

    if (!IGSrfDrawOriginal) {
        SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, NULL);
	IGActiveFreePolyIsoAttribute(IGSrfEditCurrentObj,
				     TRUE, TRUE, TRUE, TRUE);
    }

    IGSrfEditParam.SrfState = IG_SRF_EDIT_STATE_EDIT;
    IGSrfEditPlaceMessage("Waiting for surface editing");
    IGSrfEditSetState(IGSrfEditCurrentSrf, &IGSrfEditParam);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle mouse events while the surface editing grabs the mouse.           M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:    Coordinates of the mouse event.                                 M
*   Event:   Type of event (mouse move, button up etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditHandleMouse                                                         M
*****************************************************************************/
void SEditHandleMouse(int x, int y, int Event)
{
    switch (IGSrfEditParam.SrfState) {
	case IG_SRF_EDIT_STATE_ATTACH_OLD:
            if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if ((IGSrfEditCurrentObj = IGHandlePickEvent(x, y,
				IG_PICK_SURFACE | IG_PICK_TRIMSRF)) != NULL) {
		    IGReleasePickedObject();
		    SEditAttachOldDirectly(IGSrfEditCurrentObj);
		}
		else {
		    IGSrfEditPlaceMessage("Failed to select a surface - try again");
		}
	    }
	    break;
	case IG_SRF_EDIT_STATE_CLONE_OLD:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if ((IGSrfEditCurrentObj = IGHandlePickEvent(x, y,
				IG_PICK_SURFACE | IG_PICK_TRIMSRF)) != NULL) {
		    IGReleasePickedObject();
		    IG_RST_HIGHLIGHT1_OBJ(IGSrfEditCurrentObj);
		    IGSrfEditCurrentObj = IPCopyObject(NULL,
						       IGSrfEditCurrentObj,
						       TRUE);
		    IP_CAT_OBJ_NAME(IGSrfEditCurrentObj, "Clone");
		    IRIT_LIST_PUSH(IGSrfEditCurrentObj, IGGlblDisplayList);

		    SEditAttachOldDirectly(IGSrfEditCurrentObj);
		}
		else {
		    IGSrfEditPlaceMessage("Failed to select a surface - try again");
		}
	    }
	    break;
	case IG_SRF_EDIT_STATE_EDIT:
	    SEditHandleMouseStateEdit(x, y, Event);
	    break;
	default:
	    IGIritError("SEditHandleMouse Error: Invalid state type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle mouse events while the surface editing grabs the mouse.           *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:    Coordinates of the mouse event.                                 *
*   Event:   Type of event (mouse move, button up etc.).                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SEditHandleMouseStateEdit(int x, int y, int Event)
{
    IRIT_STATIC_DATA int
	FoundRational = FALSE,
        CtlPtIndex = -1,
	SEditButtonDown = FALSE;
    IRIT_STATIC_DATA CagdRType
	SurfaceUParam = -IRIT_INFNTY,
	SurfaceVParam = -IRIT_INFNTY,
	FirstUParam = 0.0,
	FirstVParam = 0.0;
    IRIT_STATIC_DATA CagdSrfDirType FirstDir;
    IRIT_STATIC_DATA IrtPtType
	StartPt = { 0.0, 0.0, 0.0 };
    IRIT_STATIC_DATA CagdSrfStruct
	*MRSrf = NULL;
    const char *Str;
    char Line[IRIT_LINE_LEN_LONG];
    int i;
    CagdRType *R, u, v, **Points;
    IrtPtType Pt;
    IrtVecType VDir;
    CagdVecStruct *Vec;
    CagdSrfDirType Dir;
    IPObjectStruct *PTmp;

    switch (IGSEditOperation) {
        case IG_SRF_EDIT_EVENT_REGION:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if (SEditPickedPointIndex == 0) {
		    IGScreenToObject(x, y, Pt, VDir);
		    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
						  Pt, VDir, &FirstDir);
		    if (R != NULL) {
			FirstUParam = R[0];
			FirstVParam = R[1];
			SEditPickedPointIndex++;
			IGSrfEditPlaceMessage("Pick second point on region from surface");
		    }
		    else {
			IGSrfEditPlaceMessage("No region was extracted");
			IGSrfEditGrabMouse = FALSE;
		    }
		}
		else {		/* SEditPickedPointIndex > 0 */
		    IGScreenToObject(x, y, Pt, VDir);
		    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
						  Pt, VDir, &Dir);
		    IGSrfEditGrabMouse = FALSE;
		    if (R != NULL && FirstDir == Dir) {
			CagdRType t1, t2;

			if (Dir == CAGD_CONST_V_DIR) {
			    t1 = IRIT_MIN(FirstVParam, R[1]);
			    t2 = IRIT_MAX(FirstVParam, R[1]);
			}
			else {
			    t1 = IRIT_MIN(FirstUParam, R[0]);
			    t2 = IRIT_MAX(FirstUParam, R[0]);
			}
			if (t1 < t2)
			    SEditDomainFromSrf(&IGSrfEditCurrentSrf,
					       t1, t2, Dir);
		    }
		    else
		        IGSrfEditPlaceMessage("No region was extracted");
		}
	    }
	    break;
	case IG_SRF_EDIT_EVENT_REFINE_ONE:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, VDir);
		R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					      Pt, VDir, &Dir);
		if (R != NULL) {
		    SEditRefineSrf(&IGSrfEditCurrentSrf,
				   &R[Dir == CAGD_CONST_U_DIR ? 0 : 1],
				   1, Dir);
		    IGRedrawViewWindow();
		}
		else
		    IGSrfEditPlaceMessage("No refinement took place");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_REFINE_ALL:
	    IGIritError("SEditHandleMouse Error: Invalid refine all event");
	    break;
	case IG_SRF_EDIT_EVENT_REFINE_REGION:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if (SEditPickedPointIndex == 0) {
		    IGScreenToObject(x, y, Pt, VDir);
		    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
						  Pt, VDir, &FirstDir);
		    if (R != NULL) {
			FirstUParam = R[0];
			FirstVParam = R[1];
			SEditPickedPointIndex++;
			IGSrfEditPlaceMessage("Pick second point on region from surface");
		    }
		    else {
			IGSrfEditPlaceMessage("No region to refine");
			IGSrfEditGrabMouse = FALSE;
		    }
		}
		else {		/* SEditPickedPointIndex > 0 */
		    IGScreenToObject(x, y, Pt, VDir);
		    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
						  Pt, VDir, &Dir);
		    IGSrfEditGrabMouse = FALSE;
		    if (R != NULL) {
			int IdxFirst, IdxLast, Length, Order, Len;
			CagdRType *KV, *RefKV,
			u = R[0],
			v = R[1];

			if (FirstUParam > u)
			    IRIT_SWAP(CagdRType, FirstUParam, u);
			if (FirstVParam > v)
			    IRIT_SWAP(CagdRType, FirstVParam, v);

			/* Do the U refine. */
			KV = IGSrfEditCurrentSrf -> UKnotVector;
			Order = IGSrfEditCurrentSrf -> UOrder;
			Length = IGSrfEditCurrentSrf -> ULength;
			Len = Order + Length;

			IdxFirst = BspKnotLastIndexL(KV, Len, FirstUParam);
			IdxFirst = IRIT_BOUND(IdxFirst, Order - 1, Length - 1);

			IdxLast = BspKnotFirstIndexG(KV, Len, u);
			IdxLast = IRIT_BOUND(IdxLast, IdxFirst, Length);

			if (IdxFirst <= IdxLast) {
			    Len = IdxLast - IdxFirst + 2 * Order - 1;
			    IdxFirst -= Order - 1;
			    RefKV = BspKnotDoubleKnots(&KV[IdxFirst],
						       &Len, Order);
			    SEditRefineSrf(&IGSrfEditCurrentSrf,
					   RefKV, Len, CAGD_CONST_U_DIR);
			    IritFree(RefKV);
			    IGRedrawViewWindow();
			    IGSrfEditPlaceMessage("Double the knots in region");
			}

			/* Do the V refine. */
			KV = IGSrfEditCurrentSrf -> VKnotVector;
			Order = IGSrfEditCurrentSrf -> VOrder;
			Length = IGSrfEditCurrentSrf -> VLength;
			Len = Order + Length;

			IdxFirst = BspKnotLastIndexL(KV, Len, FirstVParam);
			IdxFirst = IRIT_BOUND(IdxFirst, Order - 1, Length - 1);

			IdxLast = BspKnotFirstIndexG(KV, Len, v);
			IdxLast = IRIT_BOUND(IdxLast, IdxFirst, Length);

			if (IdxFirst <= IdxLast) {
			    Len = IdxLast - IdxFirst + 2 * Order - 1;
			    IdxFirst -= Order - 1;
			    RefKV = BspKnotDoubleKnots(&KV[IdxFirst],
						       &Len, Order);
			    SEditRefineSrf(&IGSrfEditCurrentSrf,
					   RefKV, Len, CAGD_CONST_V_DIR);
			    IritFree(RefKV);
			    IGRedrawViewWindow();
			    IGSrfEditPlaceMessage("Double the knots in region");
			}
		    }
		    else
		        IGSrfEditPlaceMessage("No region to refine");
		}
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV1:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, VDir);
		IGSrfEditGrabMouse = FALSE;
		R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					      Pt, VDir, &Dir);
		if (R != NULL) {
		    SEditSubdivSrf(&IGSrfEditCurrentSrf,
				   R[Dir == CAGD_CONST_U_DIR ? 0 : 1],
				   -1, Dir);
		    if (IGSrfEditCurrentSrf -> Pnext) {
			CagdSrfStruct
			  *Srf = IGSrfEditCurrentSrf -> Pnext;

			CagdSrfFree(IGSrfEditCurrentSrf);
			IGSrfEditCurrentSrf = Srf;
		    }
		}
		else
		    IGSrfEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV2:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, VDir);
		IGSrfEditGrabMouse = FALSE;
		R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					      Pt, VDir, &Dir);
		if (R != NULL) {
		    SEditSubdivSrf(&IGSrfEditCurrentSrf,
				   R[Dir == CAGD_CONST_U_DIR ? 0 : 1],
				   -1, Dir);
		    if (IGSrfEditCurrentSrf -> Pnext) {
			CagdSrfFree(IGSrfEditCurrentSrf -> Pnext);
			IGSrfEditCurrentSrf -> Pnext = NULL;
		    }
		}
		else
		    IGSrfEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV_C0_CONT:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, VDir);
		IGSrfEditGrabMouse = FALSE;
		R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					      Pt, VDir, &Dir);
		if (R != NULL) {
		    SEditSubdivSrf(&IGSrfEditCurrentSrf,
				   R[Dir == CAGD_CONST_U_DIR ? 0 : 1],
				   0, Dir);
		}
		else
		    IGSrfEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV_C1_CONT:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, VDir);
		IGSrfEditGrabMouse = FALSE;
		R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					      Pt, VDir, &Dir);
		if (R != NULL) {
		    SEditSubdivSrf(&IGSrfEditCurrentSrf,
				   R[Dir == CAGD_CONST_U_DIR ? 0 : 1],
				   1, Dir);
		}
		else
		    IGSrfEditPlaceMessage("No subdivision took place");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_MERGE_SRFS:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if (!IP_IS_SRF_OBJ(IGSrfEditCurrentObj)) {
		    IGSrfEditPlaceMessage("Can merge only regular surface");
		    break;
		}

		if ((PTmp = IGHandlePickEvent(x, y, IG_PICK_SURFACE))
								    != NULL) {
		    IGReleasePickedObject();
		    if (PTmp == IGSrfEditCurrentObj)
		        IGSrfEditPlaceMessage("Cannot merge a surface to itself");
		    else {
			if (SEditMergeSrfs(&IGSrfEditCurrentSrf,
					   PTmp -> U.Srfs))
			    IGDeleteOneObject(PTmp);   /* Del from display. */
			IGRedrawViewWindow();
		    }
		}
		else {
		    IGSrfEditPlaceMessage("Failed to select a second surface");
		}
		IGSrfEditGrabMouse = FALSE;
	    }
	    break;
	case IG_SRF_EDIT_EVENT_MOVE_CTLPTS:
	    switch (Event) {
	        case IG_SRF_EDIT_BUTTONDOWN:
		    IGScreenToObject(x, y, Pt, VDir);
		    if ((CtlPtIndex = SEditFindClosestControlPoint(
						IGSrfEditCurrentSrf, Pt, VDir,
						&FoundRational)) != -1) {
			SEditButtonDown = TRUE;
			if (FoundRational)
			    IGSrfEditPlaceMessage("Drag the control point's weight");
			else
			    IGSrfEditPlaceMessage("Drag the control point");
		    }
		    else {
			IGSrfEditPlaceMessage("No control point selected");
			break;
		    }

		/* And redraw the surface in the new position. */
		case IG_SRF_EDIT_MOTION:
		    if (SEditButtonDown && CtlPtIndex >= 0) {
			IrtPtType CtlPt, ClosestPt;

			IGScreenToObject(x, y, Pt, VDir);
			Points = IGSrfEditCurrentSrf -> Points;
			if (FoundRational) {
			    /* Need to update the weight of point. */
			    for (i = 1; i <= 3; i++) {
				Points[i][CtlPtIndex] /= Points[0][CtlPtIndex];
				CtlPt[i - 1] = Points[i][CtlPtIndex];
			    }
				    
			    Points[0][CtlPtIndex] =
			        GMDistPointLine(CtlPt, Pt, VDir) /
				    IG_CRV_RATIONAL_CIRC_LENGTH;

			    for (i = 1; i <= 3; i++)
			        Points[i][CtlPtIndex] *= Points[0][CtlPtIndex];
			}
			else {
			    if (CAGD_IS_RATIONAL_SRF(IGSrfEditCurrentSrf)) {
				for (i = 1; i <= 3; i++)
				    CtlPt[i - 1] =
				        Points[i][CtlPtIndex] /
					    Points[0][CtlPtIndex];

				GMPointFromPointLine(CtlPt, Pt, VDir,
						     ClosestPt);

				for (i = 1; i <= 3; i++)
				  Points[i][CtlPtIndex] =
				    Points[0][CtlPtIndex] *
				      ClosestPt[i - 1];
			    }
			    else {
				for (i = 1; i <= 3; i++)
				    CtlPt[i - 1] = Points[i][CtlPtIndex];

				GMPointFromPointLine(CtlPt, Pt, VDir,
						     ClosestPt);

				for (i = 1; i <= 3; i++)
				    Points[i][CtlPtIndex] = ClosestPt[i - 1];
			    }
			}
			IGRedrawViewWindow();
		    }
		    break;
		case IG_SRF_EDIT_BUTTONUP:
		    IGSrfEditPlaceMessage("Select a control point");
		    SEditButtonDown = FALSE;
		    CtlPtIndex = -1;
		    break;
	    }
	    break;		    
	case IG_SRF_EDIT_EVENT_MODIFY_SRF:
	    if (IGSrfEditParam.Type == CAGD_SBEZIER_TYPE) {
		IGSrfEditPlaceMessage("Can MR modify Bspline surfaces only");
		break;
	    }
	    if (IGSrfEditParam.Rational) {
		IGSrfEditPlaceMessage("Can MR modify integral surfaces only");
		break;
	    }
	    switch (Event) {
	        case IG_SRF_EDIT_BUTTONDOWN:
		    IGScreenToObject(x, y, StartPt, VDir);
		    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
						  StartPt, VDir, &Dir);
		    if (R != NULL) {
			SurfaceUParam = R[0];
			SurfaceVParam = R[1];
			SEditButtonDown = TRUE;
			UpdateMultResSrfDecomp();
			MRSrf = CagdSrfCopy(IGSrfEditCurrentSrf);
			IGSrfEditPlaceMessage("Drag the surface");
		    }
		    else {
			IGSrfEditPlaceMessage("No point selected");
			break;
		    }
		    SEditLastMRUParameter = SurfaceUParam;
		    SEditLastMRVParameter = SurfaceVParam;

		    /* And redraw the surface in the new position. */
		case IG_SRF_EDIT_MOTION:
		    if (SEditButtonDown &&
			SurfaceUParam != -IRIT_INFNTY &&
			SurfaceVParam != -IRIT_INFNTY) {
			IrtPtType ClosestPt;
			IrtVecType TransDir;

			IGScreenToObject(x, y, Pt, VDir);
			GMPointFromPointLine(StartPt, Pt, VDir, ClosestPt);

			IRIT_PT_SUB(TransDir, ClosestPt, StartPt);
			if (IRIT_PT_APX_EQ_ZERO_EPS(TransDir, IRIT_EPS))
			    break;
			if (IGSrfEditNormalDir) {
			    CagdVecStruct
			        *Nrml = CagdSrfNormal(MRSrf, SurfaceUParam,
						      SurfaceVParam, TRUE);
			    IrtRType
			        Size = IRIT_DOT_PROD(TransDir, Nrml -> Vec);

			    IRIT_VEC_COPY(TransDir, Nrml -> Vec);
			    IRIT_VEC_SCALE(TransDir, Size);
			}

			CagdSrfFree(IGSrfEditCurrentSrf);
			IGSrfEditCurrentSrf =
			    SEditMultiResModify(MRSrf,
					    SEditMultiResKvs,
					    IGSrfEditMRULevel *
					    (SEditMultiResKvs -> NumUKvs - 1),
					    IGSrfEditMRVLevel *
					    (SEditMultiResKvs -> NumVKvs - 1),
					    SurfaceUParam, SurfaceVParam,
					    TransDir);
			IGRedrawViewWindow();
		    }
		    break;
		case IG_SRF_EDIT_BUTTONUP:
		    CagdSrfFree(MRSrf);
		    MRSrf = NULL;
		    IGSrfEditPlaceMessage("Pick a point on surface");
		    SEditButtonDown = FALSE;
		    SurfaceUParam = -IRIT_INFNTY;
		    SurfaceVParam = -IRIT_INFNTY;
		    break;
		default:
		    IGIritError("SEditHandleMouse Error: Invalid mouse event type");
		    break;
	    }
	    break;
	case IG_SRF_EDIT_EVENT_TRIM1:
	case IG_SRF_EDIT_EVENT_TRIM2:
	    if (Event == IG_SRF_EDIT_BUTTONDOWN) {
		if ((PTmp = IGHandlePickEvent(x, y, IG_PICK_CURVE)) != NULL &&
		    (Str = AttrGetObjectStrAttrib(PTmp, "_uvsrf")) != NULL &&
		    strcmp(Str, IP_GET_OBJ_NAME(IGSrfEditCurrentObj)) == 0 &&
		    (PTmp = AttrGetObjectObjAttrib(PTmp, "_uvcrv")) != NULL) {
		    IGReleasePickedObject();
		    SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj,
					 IGSrfEditCurrentSrf);
		    SEditTrimSrf(IGSrfEditCurrentObj, PTmp -> U.Crvs,
				 IGSEditOperation == IG_SRF_EDIT_EVENT_TRIM1);
		}
		else {
		    IGSrfEditPlaceMessage("Failed to select a trimming curve");
		}
		IGSrfEditGrabMouse = FALSE;
	    }
	    break;
	case IG_SRF_EDIT_EVENT_EVALUATE:
	    IGScreenToObject(x, y, StartPt, VDir);
	    R = SEditFindClosestParameter(IGSrfEditCurrentSrf,
					  StartPt, VDir, &Dir);
	    if (R != NULL) {
		u = R[0];
		v = R[1];
		switch (IGSrfEvalEntity) {
		    case 0:		/* Eval param values. */
		        sprintf(Line, "Params = (%f, %f)\n",
				u * (SEditUMax - SEditUMin) + SEditUMin,
				v * (SEditVMax - SEditVMin) + SEditVMin);
			IGSrfEditPlaceMessage(Line);
			break;
		    case 1:		/* Eval Position. */
			R = CagdSrfEval(IGSrfEditCurrentSrf, u, v);
			CagdCoerceToE3(Pt, &R, -1,
				       IGSrfEditCurrentSrf -> PType);
			sprintf(Line, "Position = (%f, %f, %f)\n",
				Pt[0], Pt[1], Pt[2]);
			IGSrfEditPlaceMessage(Line);
			break;
		    case 2:		/* Eval U Tangent. */
			Vec = CagdSrfTangent(IGSrfEditCurrentSrf,
					     u, v, CAGD_CONST_U_DIR,
					     TRUE);
			sprintf(Line, "U Tangent = (%f, %f, %f)\n",
				Vec -> Vec[0],
				Vec -> Vec[1],
				Vec -> Vec[2]);
			IGSrfEditPlaceMessage(Line);
			break;
		    case 3:		/* Eval V Tangent. */
			Vec = CagdSrfTangent(IGSrfEditCurrentSrf,
					     u, v, CAGD_CONST_V_DIR,
					     TRUE);
			sprintf(Line, "V Tangent = (%f, %f, %f)\n",
				Vec -> Vec[0],
				Vec -> Vec[1],
				Vec -> Vec[2]);
			IGSrfEditPlaceMessage(Line);
			break;
		    case 4:		/* Eval Normal. */
			Vec = CagdSrfNormal(IGSrfEditCurrentSrf,
					    u, v, TRUE);
			sprintf(Line, "Normal = (%f, %f, %f)\n",
				Vec -> Vec[0],
				Vec -> Vec[1],
				Vec -> Vec[2]);
			IGSrfEditPlaceMessage(Line);
			break;
		}
	    }
	    else
	        IGSrfEditPlaceMessage("No point selected");
	    break;
	default:
	    IGIritError("SEditHandleMouse Error: Invalid event type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets up the state of the new surface under editting.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:      The new surface we are about to edit.                          *
*   State:    The state to update.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGSrfEditSetState(CagdSrfStruct *Srf, IGSrfEditParamStruct *State)
{
    State -> UOrder = Srf -> UOrder;
    State -> VOrder = Srf -> VOrder;
    State -> Type = Srf -> GType;
    State -> UEndCond =
        CAGD_IS_BSPLINE_SRF(Srf) && CAGD_IS_UPERIODIC_SRF(Srf) ?
	    CAGD_END_COND_PERIODIC : (CAGD_IS_BEZIER_SRF(Srf) ||
				      BspSrfHasOpenECDir(Srf,
							 CAGD_CONST_U_DIR) ?
				      CAGD_END_COND_OPEN :
				      CAGD_END_COND_FLOAT);
    State -> VEndCond =
        CAGD_IS_BSPLINE_SRF(Srf) && CAGD_IS_VPERIODIC_SRF(Srf) ?
	    CAGD_END_COND_PERIODIC : (CAGD_IS_BEZIER_SRF(Srf) ||
				      BspSrfHasOpenECDir(Srf,
							 CAGD_CONST_V_DIR) ?
				      CAGD_END_COND_OPEN :
				      CAGD_END_COND_FLOAT);
    State -> Rational = CAGD_IS_RATIONAL_SRF(Srf);

    /* And prepare the global state. */
    IGSrfEditParamUpdateWidget();
    IGSrfEditActive = TRUE;
    IGSrfEditGrabMouse = FALSE;
    SEditLastMRUParameter = -IRIT_INFNTY;
    SEditLastMRVParameter = -IRIT_INFNTY;
    SEditFreeStateStack();
    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle non mouse events while the surface editing.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:     Type of event (clear, order change, etc.).                    M
*   MenuIndex: Menu index of sub-pop-up menu.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditHandleNonMouseEvents                                                M
*****************************************************************************/
void SEditHandleNonMouseEvents(IGSrfEditEventType Event, int MenuIndex)
{
    IRIT_STATIC_DATA int
	SEditDisableUndo = FALSE;
    IRIT_STATIC_DATA CagdVType
	Origin = { 0.0, 0.0, 0.0 };
    int l, Len;
    char Line[IRIT_LINE_LEN_LONG];
    IrtHmgnMatType Mat;
    CagdSrfStruct *Srf, *TSrf;
    TrimSrfStruct *TSrfSkel;
    IPObjectStruct *PObj;

    if (Event != IG_SRF_EDIT_EVENT_STATE &&
	Event != IG_SRF_EDIT_EVENT_PRIMITIVES &&
	Event != IG_SRF_EDIT_EVENT_U_ORDER &&
	Event != IG_SRF_EDIT_EVENT_V_ORDER &&
	Event != IG_SRF_EDIT_EVENT_U_END_COND &&
	Event != IG_SRF_EDIT_EVENT_V_END_COND &&
	Event != IG_SRF_EDIT_EVENT_RATIONAL &&
	Event != IG_SRF_EDIT_EVENT_SRF_TYPE &&
	Event != IG_SRF_EDIT_EVENT_U_PARAM_TYPE &&
	Event != IG_SRF_EDIT_EVENT_V_PARAM_TYPE &&
	IGSrfEditCurrentSrf == NULL) {
	IGSrfEditPlaceMessage("Pick a surface first");
	return;
    }

    switch (Event) {
	case IG_SRF_EDIT_EVENT_CLEAR:
	case IG_SRF_EDIT_EVENT_DISMISS:
        case IG_SRF_EDIT_EVENT_PRIMITIVES:
	case IG_SRF_EDIT_EVENT_STATE:
	case IG_SRF_EDIT_EVENT_UNDO:
	case IG_SRF_EDIT_EVENT_REDO:
	    break;
	default:
	    if (!SEditDisableUndo)
		SEditPushState(IGSrfEditCurrentSrf, SEditTrimSrfSkel,
			       &IGSrfEditParam);
	    break;
    }

    IGSrfEditGrabMouse = FALSE;
    IGSEditOperation = Event;
    SEditLastMRUParameter = -IRIT_INFNTY;
    SEditLastMRVParameter = -IRIT_INFNTY;

    switch (Event) {
	case IG_SRF_EDIT_EVENT_CLEAR:
	    IGSrfEditPlaceMessage("Clear surface editing operation");
	    IGSEditOperation = IG_SRF_EDIT_EVENT_NONE;
	    break;
	case IG_SRF_EDIT_EVENT_DISMISS:
	    SEditDetachSurface();
	    break;
	case IG_SRF_EDIT_EVENT_PRIMITIVES:
	    switch (MenuIndex) {
		case 0: /* Unit sphere */
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdPrimSphereSrf(Origin, 1.0, TRUE));
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimSphere");
		    break;
		case 1: /* Unit XY Plane */
		    Srf = CagdPrimPlaneSrf(-1, -1, 1, 1, 0);
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE,
						       FALSE));
		    CagdSrfFree(Srf);
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimXYPlane");
		    break;
		case 2: /* Unit XZ Plane */
		    Srf = CagdPrimPlaneSrf(-1, -1, 1, 1, 0);
		    MatGenMatRotX1(IRIT_DEG2RAD(90.0), Mat);
		    TSrf = CagdSrfMatTransform(Srf, Mat);
		    CagdSrfFree(Srf);
		    Srf = TSrf;

	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE,
						       FALSE));
		    CagdSrfFree(Srf);
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimXZPlane");
		    break;
		case 3: /* Unit YZ Plane */
		    Srf = CagdPrimPlaneSrf(-1, -1, 1, 1, 0);
	            MatGenMatRotY1(IRIT_DEG2RAD(90.0), Mat);
		    TSrf = CagdSrfMatTransform(Srf, Mat);
		    CagdSrfFree(Srf);
		    Srf = TSrf;

	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE,
						       FALSE));
		    CagdSrfFree(Srf);
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimYZPlane");
		    break;
		case 4: /* Cylinder */
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdPrimCylinderSrf(Origin, 0.5, 1.0,
							   TRUE,
							   CAGD_PRIM_CAPS_BOTH));
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimCylinder");
		    break;
		case 5: /* Truncated Cone */
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdPrimCone2Srf(Origin, 0.5, 0.25, 1.0,
							TRUE,
							CAGD_PRIM_CAPS_BOTH));
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimTruncCone");
		    break;
		case 6: /* Cone */
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdPrimConeSrf(Origin, 0.5, 1.0, TRUE,
						       CAGD_PRIM_CAPS_BOTH));
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimCone");
		    break;
		case 7: /* Torus */
	            IGSrfEditCurrentObj =
			IPGenSRFObject(CagdPrimTorusSrf(Origin, 1.0, 0.25,
							TRUE));
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, "PrimTorus");
		    break;
		default:
		    IGSrfEditCurrentObj = NULL;
	    }

	    if (IGSrfEditCurrentObj != NULL) {
		IRIT_LIST_PUSH(IGSrfEditCurrentObj, IGGlblDisplayList);
		SEditAttachOldDirectly(IGSrfEditCurrentObj);
		IGRedrawViewWindow();
	    }
	    else {
		IGSrfEditPlaceMessage("Failed to create primitive!");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_STATE:
	    switch (MenuIndex) {
		case IG_SRF_EDIT_STATE_ATTACH_OLD:
		case IG_SRF_EDIT_STATE_CLONE_OLD:
		    if (IGSrfEditCurrentSrf != NULL) {
			IGSrfEditPlaceMessage("Detach from current surface first");
			break;
		    }

		    IGSrfEditPlaceMessage("Please select a surface");
		    IGSrfEditGrabMouse = TRUE;
		    IGSrfEditParam.SrfState = (IGSrfEditStateType) MenuIndex;
		    break;
		case IG_SRF_EDIT_STATE_DETACH:
		    if (IGSrfEditCurrentSrf == NULL) {
			IGSrfEditPlaceMessage("No surface to detach from");
			break;
		    }

		    SEditDetachSurface();
		    IGRedrawViewWindow();
		    break;
		default:
		    break;
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV:
	    switch (MenuIndex) {
	        case 0:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_SUBDIV1;
		    break;
		case 1:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_SUBDIV2;
		    break;
		case 2:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_SUBDIV_C0_CONT;
		    break;
		case 3:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_SUBDIV_C1_CONT;
		    break;
	    }
	    IGSrfEditPlaceMessage("Pick a point on surface to subdivide");
	    IGSrfEditGrabMouse = TRUE;
	    break;
	case IG_SRF_EDIT_EVENT_U_DRAISE:
	case IG_SRF_EDIT_EVENT_V_DRAISE:
	    if ((Event == IG_SRF_EDIT_EVENT_U_DRAISE &&
		 IGSrfEditParam.UEndCond == CAGD_END_COND_PERIODIC) ||
		(Event == IG_SRF_EDIT_EVENT_V_DRAISE &&
		 IGSrfEditParam.VEndCond == CAGD_END_COND_PERIODIC))
	        IGSrfEditPlaceMessage("Cannot degree raise periodics");
	    else {
	        Srf = CagdSrfBlossomDegreeRaise(IGSrfEditCurrentSrf,
		       Event == IG_SRF_EDIT_EVENT_U_DRAISE ? CAGD_CONST_U_DIR
							   : CAGD_CONST_V_DIR);
		CagdSrfFree(IGSrfEditCurrentSrf);
		IGSrfEditCurrentSrf = Srf;
		IGSrfEditParam.UOrder = IGSrfEditCurrentSrf -> UOrder;
		IGSrfEditParam.VOrder = IGSrfEditCurrentSrf -> VOrder;
		sprintf(Line, "Srf raised to (%d, %d)",
			IGSrfEditParam.UOrder, IGSrfEditParam.VOrder);
	        IGSrfEditPlaceMessage(Line);
		IGSrfEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_MERGE_SRFS:
	    IGSrfEditPlaceMessage("Pick a second surface to merge with");
	    IGSrfEditGrabMouse = TRUE;
	    break;
	case IG_SRF_EDIT_EVENT_REGION:
	    IGSrfEditPlaceMessage("Pick first point on region from surface");
	    SEditPickedPointIndex = 0;
	    IGSrfEditGrabMouse = TRUE;
	    break;
	case IG_SRF_EDIT_EVENT_REFINE:
	    if (IGSrfEditCurrentSrf -> GType != CAGD_SBSPLINE_TYPE) {
		IGSrfEditPlaceMessage("Refine Bspline surfaces only");
		break;
	    }

	    switch (MenuIndex) {
	        case 0:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_REFINE_ONE;
		    IGSrfEditPlaceMessage("Pick a point on surface to refine");
		    IGSrfEditGrabMouse = TRUE;
		    break;
		case 1:
		    {
		        CagdRType *t;
			int Len = IGSrfEditCurrentSrf -> UOrder + 
				  IGSrfEditCurrentSrf -> ULength;

		        IGSrfEditPlaceMessage("Double the number of knots along surface");
		        /* Refine at all interval locations along U KV. */
			t = BspKnotDoubleKnots(IGSrfEditCurrentSrf -> UKnotVector,
					       &Len,
					       IGSrfEditCurrentSrf -> UOrder);
			SEditRefineSrf(&IGSrfEditCurrentSrf, t, Len,
				       CAGD_CONST_U_DIR);
			IritFree(t);

		        /* Refine at all interval locations along U KV. */
			Len = IGSrfEditCurrentSrf -> VOrder + 
			      IGSrfEditCurrentSrf -> VLength;

		        /* Refine at all interval locations along knot vec. */
			t = BspKnotDoubleKnots(IGSrfEditCurrentSrf -> VKnotVector,
					       &Len,
					       IGSrfEditCurrentSrf -> VOrder);
			SEditRefineSrf(&IGSrfEditCurrentSrf, t, Len,
				       CAGD_CONST_V_DIR);
			IritFree(t);

			IGRedrawViewWindow();
		    }
		    break;
		case 2:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_REFINE_REGION;
		    IGSrfEditPlaceMessage("Pick first point on region from surface");
		    SEditPickedPointIndex = 0;
		    IGSrfEditGrabMouse = TRUE;
		    break;
	    }
	    break;
	case IG_SRF_EDIT_EVENT_U_ORDER:
	    if (IGSrfEditCurrentSrf != NULL &&
		IGSrfEditCurrentSrf -> GType == CAGD_SBSPLINE_TYPE &&
		IGSrfEditCurrentSrf -> ULength >= IGSrfEditParam.UOrder &&
		IGSrfEditCurrentSrf -> UOrder != IGSrfEditParam.UOrder) {
		IGSrfEditCurrentSrf -> UOrder = IGSrfEditParam.UOrder;
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.UEndCond,
				      CAGD_CONST_U_DIR,
				      IGSrfEditParam.UParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_V_ORDER:
	    if (IGSrfEditCurrentSrf != NULL &&
		IGSrfEditCurrentSrf -> GType == CAGD_SBSPLINE_TYPE &&
		IGSrfEditCurrentSrf -> VLength >= IGSrfEditParam.VOrder &&
		IGSrfEditCurrentSrf -> VOrder != IGSrfEditParam.VOrder) {
		IGSrfEditCurrentSrf -> VOrder = IGSrfEditParam.VOrder;
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.VEndCond,
				      CAGD_CONST_V_DIR,
				      IGSrfEditParam.VParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_U_END_COND:
	    switch (MenuIndex) {
		case 0:
		    IGSrfEditParam.UEndCond = CAGD_END_COND_OPEN;
		    break;
		case 1:
		    IGSrfEditParam.UEndCond = CAGD_END_COND_FLOAT;
		    break;
		case 2:
		    IGSrfEditParam.UEndCond = CAGD_END_COND_PERIODIC;
		    break;
	    };
	    if (IGSrfEditCurrentSrf != NULL &&
		IGSrfEditCurrentSrf -> GType == CAGD_SBSPLINE_TYPE) {
		IGSrfEditCurrentSrf -> UPeriodic =
		    IGSrfEditParam.UEndCond == CAGD_END_COND_PERIODIC;
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.UEndCond,
				      CAGD_CONST_U_DIR,
				      IGSrfEditParam.UParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_V_END_COND:
	    switch (MenuIndex) {
		case 0:
		    IGSrfEditParam.VEndCond = CAGD_END_COND_OPEN;
		    break;
		case 1:
		    IGSrfEditParam.VEndCond = CAGD_END_COND_FLOAT;
		    break;
		case 2:
		    IGSrfEditParam.VEndCond = CAGD_END_COND_PERIODIC;
		    break;
	    };
	    if (IGSrfEditCurrentSrf != NULL &&
		IGSrfEditCurrentSrf -> GType == CAGD_SBSPLINE_TYPE) {
		IGSrfEditCurrentSrf -> VPeriodic =
		    IGSrfEditParam.VEndCond == CAGD_END_COND_PERIODIC;
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.VEndCond,
				      CAGD_CONST_V_DIR,
				      IGSrfEditParam.VParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_RATIONAL:
	    IGSrfEditParam.Rational = !IGSrfEditParam.Rational;
	    if (IGSrfEditCurrentSrf != NULL) {
		l = CAGD_NUM_OF_PT_COORD(IGSrfEditCurrentSrf -> PType);
		Len = IGSrfEditCurrentSrf -> ULength *
		                               IGSrfEditCurrentSrf -> VLength;

		if (IGSrfEditParam.Rational) {
		    int i, j;
		    IrtRType
			**Points = IGSrfEditCurrentSrf -> Points;

		    IGSrfEditCurrentSrf -> PType = CAGD_MAKE_PT_TYPE(1, l);
		    if (Points[0] == NULL) {
			/* Allocate vec of rational terms and place in srf. */
			Points[0] = (CagdRType *)
			    IritMalloc(Len * sizeof(CagdRType));
			for (i = 0; i < Len; i++)
			    Points[0][i] = 1.0;
		    }
		    else {
			/* Multiply the control points by the weights. */
			for (i = 0; i < Len; i++)
			    for (j = 1; j <= l; j++)
				Points[j][i] *= Points[0][i];
		    }
		}
		else {
		    int i, j;
		    IrtRType
			**Points = IGSrfEditCurrentSrf -> Points;

		    IGSrfEditCurrentSrf -> PType = CAGD_MAKE_PT_TYPE(0, l);
		    /* Divide the control points by the weights. */
		    for (i = 0; i < Len; i++)
			for (j = 1; j <= l; j++)
			    Points[j][i] /= Points[0][i];
		}
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_MOVE_CTLPTS:
	    IGSrfEditPlaceMessage("Select a control point");
	    IGSrfEditGrabMouse = TRUE;
	    if (!IGSrfEditDrawMesh) {
		IGSrfEditDrawMesh = TRUE;
		IGSrfEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_MODIFY_SRF:
	    IGSrfEditPlaceMessage("Select a point on surface");
	    IGSrfEditGrabMouse = TRUE;
	    break;
	case IG_SRF_EDIT_EVENT_SRF_TYPE:
	    IGSrfEditParam.Type =
		IGSrfEditParam.Type == CAGD_SBEZIER_TYPE ? CAGD_SBSPLINE_TYPE
						         : CAGD_SBEZIER_TYPE;
	    if (IGSrfEditCurrentSrf != NULL) {
		switch (IGSrfEditParam.Type) {
	            case CAGD_SBEZIER_TYPE:
			IGSrfEditCurrentSrf -> GType = CAGD_SBEZIER_TYPE;
			if (IGSrfEditCurrentSrf -> UKnotVector)
			    IritFree(IGSrfEditCurrentSrf -> UKnotVector);
			if (IGSrfEditCurrentSrf -> VKnotVector)
			    IritFree(IGSrfEditCurrentSrf -> VKnotVector);
			IGSrfEditCurrentSrf -> UKnotVector =
			    IGSrfEditCurrentSrf -> VKnotVector = NULL;
			IGSrfEditCurrentSrf -> UPeriodic =
			    IGSrfEditCurrentSrf -> VPeriodic = FALSE;
			IGSrfEditCurrentSrf -> UOrder =
			    IGSrfEditParam.UOrder =
				IGSrfEditCurrentSrf -> ULength;
			IGSrfEditCurrentSrf -> VOrder =
			    IGSrfEditParam.VOrder =
				IGSrfEditCurrentSrf -> VLength;
			IGSrfEditParamUpdateWidget();
			break;
		    case CAGD_SBSPLINE_TYPE:
			IGSrfEditCurrentSrf -> GType = CAGD_SBSPLINE_TYPE;
			if (IGSrfEditCurrentSrf -> UKnotVector)
			    IritFree(IGSrfEditCurrentSrf -> UKnotVector);
			if (IGSrfEditCurrentSrf -> VKnotVector)
			    IritFree(IGSrfEditCurrentSrf -> VKnotVector);
			IGSrfEditCurrentSrf -> UKnotVector = NULL;
			IGSrfEditCurrentSrf -> VKnotVector = NULL;
			IGSrfEditCurrentSrf -> UPeriodic =
			    IGSrfEditParam.UEndCond == CAGD_END_COND_PERIODIC;
			IGSrfEditCurrentSrf -> VPeriodic =
			    IGSrfEditParam.VEndCond == CAGD_END_COND_PERIODIC;
			IGSrfEditCurrentSrf -> UOrder = IGSrfEditParam.UOrder;
			IGSrfEditCurrentSrf -> VOrder = IGSrfEditParam.VOrder;
			SEditUpdateKnotVector(IGSrfEditCurrentSrf,
					      IGSrfEditParam.UEndCond,
					      CAGD_CONST_U_DIR,
					      IGSrfEditParam.UParamType);
			SEditUpdateKnotVector(IGSrfEditCurrentSrf,
					      IGSrfEditParam.VEndCond,
					      CAGD_CONST_V_DIR,
					      IGSrfEditParam.VParamType);
			break;
		    default:
			IGIritError("SEditHandleNonMouseEvents Error: Invalid surface type");
			break;
		}
		IGRedrawViewWindow();
	    }			
	    break;
	case IG_SRF_EDIT_EVENT_U_PARAM_TYPE:
	    switch (MenuIndex) {
	        case 0:
		    IGSrfEditParam.UParamType = CAGD_UNIFORM_PARAM;
		    break;
	        case 1:
		    IGSrfEditParam.UParamType = CAGD_CENTRIPETAL_PARAM;
		    break;
	        case 2:
		    IGSrfEditParam.UParamType = CAGD_CHORD_LEN_PARAM;
		    break;
	    }
	    if (IGSrfEditCurrentSrf != NULL) {
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.UEndCond,
				      CAGD_CONST_U_DIR,
				      IGSrfEditParam.UParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_V_PARAM_TYPE:
	    switch (MenuIndex) {
	        case 0:
		    IGSrfEditParam.VParamType = CAGD_UNIFORM_PARAM;
		    break;
	        case 1:
		    IGSrfEditParam.VParamType = CAGD_CENTRIPETAL_PARAM;
		    break;
	        case 2:
		    IGSrfEditParam.VParamType = CAGD_CHORD_LEN_PARAM;
		    break;
	    }
	    if (IGSrfEditCurrentSrf != NULL) {
		SEditUpdateKnotVector(IGSrfEditCurrentSrf,
				      IGSrfEditParam.VEndCond,
				      CAGD_CONST_V_DIR,
				      IGSrfEditParam.VParamType);
		IGRedrawViewWindow();
	    }
	    break;
	case IG_SRF_EDIT_EVENT_EVALUATE:
	    IGSrfEditPlaceMessage("Select a point on surface");
	    IGSrfEditGrabMouse = TRUE;
	    IGSrfEvalEntity = MenuIndex;
	    break;
	case IG_SRF_EDIT_EVENT_TRIM:
	    IGSrfEditPlaceMessage("Pick a curve on surface to trim with");
	    switch (MenuIndex) {
	        case 0:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_TRIM1;
		    break;
		case 1:
		    IGSEditOperation = IG_SRF_EDIT_EVENT_TRIM2;
		    break;
	    }
	    IGSrfEditGrabMouse = TRUE;
	    break;
	case IG_SRF_EDIT_EVENT_REVERSE:
	    Srf = NULL;
	    switch (MenuIndex) {
		case 0: /* Flip <-> V. */
	            Srf = CagdSrfReverse2(IGSrfEditCurrentSrf);
	            break;
		case 1: /* U Reverse. */
	            Srf = CagdSrfReverseDir(IGSrfEditCurrentSrf,
					    CAGD_CONST_U_DIR);
	            break;
		case 2: /* V Reverse. */
	            Srf = CagdSrfReverseDir(IGSrfEditCurrentSrf,
					    CAGD_CONST_V_DIR);
	            break;
	    }
	    if (Srf != NULL) {
	        CagdSrfFree(IGSrfEditCurrentSrf);
		IGSrfEditCurrentSrf = Srf;
		IGActiveFreePolyIsoAttribute(IGSrfEditCurrentObj,
					     TRUE, TRUE, TRUE, TRUE);
		IGSrfEditPlaceMessage("Reverse succeeded");
		IGSrfEditParamUpdateWidget();
		IGRedrawViewWindow();
	    }
	    else
	        IGSrfEditPlaceMessage("Failed to reverse surface");
	    break;
	case IG_SRF_EDIT_EVENT_UNDO:
	    if ((Srf = SEditUndoState(&IGSrfEditParam, &TSrfSkel)) != NULL) {
		SEditDisableUndo = TRUE;

		CagdSrfFree(IGSrfEditCurrentSrf);
		IGSrfEditCurrentSrf = CagdSrfCopy(Srf);

		if (SEditTrimSrfSkel != NULL)
		    TrimSrfFree(SEditTrimSrfSkel);
		SEditTrimSrfSkel = TSrfSkel != NULL ? TrimSrfCopy(TSrfSkel)
						    : NULL;

		IGSrfEditParamUpdateWidget();
		IGSrfEditPlaceMessage("Undo succeeded");
		IGRedrawViewWindow();
		SEditDisableUndo = FALSE;
	    }
	    else
		IGSrfEditPlaceMessage("Undo failed");
	    break;
	case IG_SRF_EDIT_EVENT_REDO:
	    if ((Srf = SEditRedoState(&IGSrfEditParam, &TSrfSkel)) != NULL) {
		SEditDisableUndo = TRUE;

		CagdSrfFree(IGSrfEditCurrentSrf);
		IGSrfEditCurrentSrf = CagdSrfCopy(Srf);

		if (SEditTrimSrfSkel != NULL)
		    TrimSrfFree(SEditTrimSrfSkel);
		SEditTrimSrfSkel = TSrfSkel != NULL ? TrimSrfCopy(TSrfSkel)
						    : NULL;

		IGSrfEditParamUpdateWidget();
		IGSrfEditPlaceMessage("Redo succeeded");
		IGRedrawViewWindow();
		SEditDisableUndo = FALSE;
	    }
	    else
		IGSrfEditPlaceMessage("Redo failed");
	    break;
	case IG_SRF_EDIT_EVENT_SAVE_SRF:
	    {
		int Handler;
		char Line[IRIT_LINE_LEN_LONG],
		    *FileName = (char *) MenuIndex;
		CagdSrfStruct *Srf;

		if (!IRT_STR_NULL_ZERO_LEN(FileName)) {
		    PObj = IPCopyObject(NULL, IGSrfEditCurrentObj, TRUE);
		    if (SRF_EDIT_GET_SRF_OBJ(PObj) != NULL) {
		        Srf = SRF_EDIT_GET_SRF_OBJ(PObj);
		        CagdSrfFree(Srf);
		    }
		    Srf = CagdSrfCopy(IGSrfEditCurrentSrf);
		    SRF_EDIT_SET_SRF_OBJ(PObj, Srf);
		    if (CAGD_IS_BSPLINE_SRF(Srf)) {
		        BspKnotAffineTransOrder2(Srf -> UKnotVector,
						 Srf -> UOrder,
						 CAGD_SRF_UPT_LST_LEN(Srf)
						     + Srf -> UOrder,
						 SEditUMin, SEditUMax);
			BspKnotAffineTransOrder2(Srf -> VKnotVector,
						 Srf -> VOrder,
						 CAGD_SRF_VPT_LST_LEN(Srf)
						     + Srf -> VOrder,
						 SEditVMin, SEditVMax);
		    }
		    if (IP_IS_TRIMSRF_OBJ(PObj)) {
		    	TrimAffineTransTrimCurves(PObj -> U.TrimSrfs
								-> TrimCrvList,
						  0.0, 1.0, 0.0, 1.0,
						  SEditUMin, SEditUMax,
						  SEditVMin, SEditVMax);
		    }
		    if ((Handler = IPOpenDataFile(FileName, FALSE,
						  FALSE)) >= 0) {
		        IPPutObjectToHandler(Handler, PObj);
			IPCloseStream(Handler, TRUE);

			sprintf(Line, "Surface saved under \"%s\"", FileName);
			IGSrfEditPlaceMessage(Line);
		    }
		    else {
		        sprintf(Line, "Failed to open file \"%s\"", FileName);
		        IGIritError(Line);
		    }

		    IPFreeObject(PObj);
		}
		else
		    IGSrfEditPlaceMessage("Failed to pick surfaces's name");
	    }
	    break;
	case IG_SRF_EDIT_EVENT_SUBMIT_SRF:
	    if (IGSrfEditCurrentSrf == NULL) {
		IGSrfEditPlaceMessage("No surface under editing to submit");
		break;
	    }
	    if (IGGlblStandAlone) {
		IGSrfEditPlaceMessage("No submissions in stand alone mode");
		break;
	    }
	    if (IGSrfEditCurrentObj == NULL ||
		!IP_VALID_OBJ_NAME(IGSrfEditCurrentObj)) {		
		IGSrfEditPlaceMessage("Object must have a name before submission");
		break;
	    }
	    PObj = IPGenSrfObject("_SubmitObject_",
				  CagdSrfCopy(IGSrfEditCurrentSrf), NULL);
	    AttrSetObjectStrAttrib(PObj, "ObjName",
				   IP_GET_OBJ_NAME(IGSrfEditCurrentObj));
	    IPSocWriteOneObject(IGGlblIOHandle, PObj);
	    IPFreeObject(PObj);
	    break;
        default:
	    IGIritError("SEditHandleNonMouseEvents Error: Invalid event type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently editted surface.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditDetachSurface                                                       M
*****************************************************************************/
void SEditDetachSurface(void)
{
    GMBBBboxStruct *BBox;

    IGSrfEditGrabMouse = FALSE;
    IGSrfEditActive = FALSE;
    SEditFreeStateStack();
    IGSrfEditParam.SrfState = IG_SRF_EDIT_STATE_DETACH;
    IGSrfEditParamUpdateWidget();

    if (IGSrfEditCurrentSrf == NULL || IGSrfEditCurrentObj == NULL) {
        if (IGSrfEditCurrentSrf != NULL) {
	    CagdSrfFree(IGSrfEditCurrentSrf);
	    IGSrfEditCurrentSrf = NULL;
	}
        return;
    }

    if (!IGSrfEditParam.Rational && IGSrfEditCurrentSrf -> Points[0] != NULL) {
	IritFree(IGSrfEditCurrentSrf -> Points[0]);
	IGSrfEditCurrentSrf -> Points[0] = NULL;
    }

    if (IP_IS_TRIMSRF_OBJ(IGSrfEditCurrentObj)) {
	SEditTrimSrfSkel -> Srf = NULL;
	TrimSrfFree(SEditTrimSrfSkel);

	TrimAffineTransTrimCurves(IGSrfEditCurrentObj -> U.TrimSrfs -> TrimCrvList,
				  0.0, 1.0, 0.0, 1.0,
				  SEditUMin, SEditUMax,
				  SEditVMin, SEditVMax);
    }

    if (IGSrfEditCurrentSrf -> UKnotVector != NULL)
	BspKnotAffineTransOrder2(IGSrfEditCurrentSrf -> UKnotVector,
				 IGSrfEditCurrentSrf -> UOrder,
				 CAGD_SRF_UPT_LST_LEN(IGSrfEditCurrentSrf) +
			             IGSrfEditCurrentSrf -> UOrder,
				 SEditUMin, SEditUMax);
    if (IGSrfEditCurrentSrf -> VKnotVector != NULL)
	BspKnotAffineTransOrder2(IGSrfEditCurrentSrf -> VKnotVector,
				 IGSrfEditCurrentSrf -> VOrder,
				 CAGD_SRF_VPT_LST_LEN(IGSrfEditCurrentSrf) +
			             IGSrfEditCurrentSrf -> VOrder,
				 SEditVMin, SEditVMax);

    SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, IGSrfEditCurrentSrf);
    CagdSrfFree(IGSrfOriginalSurface);

    IG_RST_HIGHLIGHT1_OBJ(IGSrfEditCurrentObj);

    IGSrfEditCurrentSrf = NULL;
    IGActiveFreePolyIsoAttribute(IGSrfEditCurrentObj, TRUE, TRUE, TRUE, TRUE);
    BBox = GMBBComputeBboxObject(IGSrfEditCurrentObj);
    IRIT_PT_COPY(IGSrfEditCurrentObj -> BBox[0], BBox -> Min);
    IRIT_PT_COPY(IGSrfEditCurrentObj -> BBox[1], BBox -> Max);

    IGSrfEditCurrentObj = NULL;

    IGGlblPickedObj = NULL;
    if (IGGlblPickedPolyObj != NULL) {
	IPFreeObject(IGGlblPickedPolyObj);
	IGGlblPickedPolyObj = NULL;
    }
}
