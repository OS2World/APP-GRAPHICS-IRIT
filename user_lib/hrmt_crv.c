/******************************************************************************
* Hrmt_crv.c - user inteface to construct piecewise planar cubic hermit crvs. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber,					 Dec. 2005.   *
******************************************************************************/

#include "irit_sm.h"
#include "user_loc.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "geom_lib.h"

#define USER_HC_IRIT_VEC_SCALE	1
#define USER_HC_NUM_INIT_CRVS	10
#define USER_HC_SAMPLES_PER_CRV	50

/* #define DEBUG_HC_EDIT */

typedef enum {
    USER_HC_MODE_INIT,
    USER_HC_MODE_INSERT_CTLPTS,
    USER_HC_MODE_EDIT_CTLPTS
} UserHCModeType;

typedef struct UserHCStruct {
    CagdPType StartPos, EndPos;
    CagdVType StartTan, EndTan;
    CagdRType TMin, TMax;
    int EndPosCont;
    int EndPosIdx, StartPosIdx;
    CagdCrvStruct *Crv;
    CagdPolylineStruct *Pl;
} UserHCStruct;

typedef struct UserHCEditStruct {
    int IsPeriodic;
    int MaxNumOfHCrvs;
    int NumOfHCrvs;
    UserHCEditDrawCtlPtFuncType _CtlPtDrawFunc;
    UserHCModeType Mode;
    UserHCStruct *HCrvs;
} UserHCEditStruct;

IRIT_STATIC_DATA int
    UserHCEditPointUniqueIndex = 1;

static void UserHCEditReallocCrvs(VoidPtr HC);
static void UserHCEditCopyCrv(UserHCStruct *Dst, UserHCStruct *Src);
static void UserHCEditMakeTangentCont(VoidPtr HC, int Index);
static void UserHCEditUpdateOneHermite(VoidPtr HC, int Index);
static void UserHCEditUpdateFromCrv(UserHCStruct *HCrv, CagdCrvStruct *Crv);
static void UserHCEditRedirectTangent(IrtVecType Tan, IrtVecType PrescribedTan);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes the interface for editing planar piecewise cubic Hermite     M
* curves.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   StartX, StartY:   Starting control point of curve.                       M
*   Periodic:         TRUE if curve is to be closed.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:      A Handle on the created HC to be edited.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditFromCurve, UserHCEditIsPeriodic, UserHCEditSetPeriodic,	     M
*   UserHCEditSetCtlPtCont, UserHCEditSetDrawCtlptFunc, UserHCEditDelete,    M
*   UserHCEditTranslate, UserHCEditCreateAppendCtlpt, UserHCEditCreateDone,  M
*   UserHCEditInsertCtlpt, UserHCEditDeleteCtlpt, UserHCEditMoveCtl,	     M
*   UserHCEditMoveCtlPt, UserHCEditMoveCtlTan, UserHCEditIsNearCrv,	     M
*   UserHCEditIsNearCtlPt, UserHCEditIsNearCtlTan,			     M
*   UserHCEditGetCrvRepresentation, UserHCEditDrawCtlpts,		     M
*   UserHCEditTransform, UserHCEditRelativeTranslate, UserHCEditEvalDefTans  M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditInit                                                           M
*****************************************************************************/
VoidPtr UserHCEditInit(CagdRType StartX, CagdRType StartY, CagdBType Periodic)
{
    UserHCEditStruct 
	*HCE = (UserHCEditStruct *) IritMalloc(sizeof(UserHCEditStruct));

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditInit (%f %f), Periodic = %d\n",
	   StartX, StartY, Periodic);
#endif /* DEBUG_HC_EDIT */

    /* Init new data structure. */
    HCE -> Mode = USER_HC_MODE_INIT;
    HCE -> NumOfHCrvs = 0;
    HCE -> MaxNumOfHCrvs = USER_HC_NUM_INIT_CRVS;
    HCE -> HCrvs = (UserHCStruct *)
	IritMalloc(HCE -> MaxNumOfHCrvs * sizeof(UserHCStruct));

    HCE -> IsPeriodic = Periodic;
    HCE -> _CtlPtDrawFunc = NULL;

    return HCE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Create a Hermite curve from the given regular curve.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Regular curve to convert to an Hermite curve.                     M
*   Tol:   Tolerance of approximation if the input curve is higher degree    M
*	   than cubic.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:      A Handle on the created HC to be edited.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvAsBzrCubics, UserHCEditInit                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditFromCurve                                                      M
*****************************************************************************/
VoidPtr UserHCEditFromCurve(const CagdCrvStruct *Crv, CagdRType Tol)
{
    CagdCrvStruct *TCrv,
	*CubicCrvs = SymbApproxCrvAsBzrCubics(Crv, Tol, IRIT_INFNTY);
    UserHCEditStruct 
	*HCE = (UserHCEditStruct *) IritMalloc(sizeof(UserHCEditStruct));
    UserHCStruct *HCrvs;
    int i;

    HCE -> Mode = USER_HC_MODE_EDIT_CTLPTS;
    HCE -> NumOfHCrvs = CagdListLength(CubicCrvs);
    HCE -> MaxNumOfHCrvs = HCE -> NumOfHCrvs + USER_HC_NUM_INIT_CRVS;
    HCE -> HCrvs = HCrvs = (UserHCStruct *)
	IritMalloc(HCE -> MaxNumOfHCrvs * sizeof(UserHCStruct));

    HCE -> IsPeriodic = Crv -> Periodic;
    HCE -> _CtlPtDrawFunc = NULL;

    /* Initialize all the curves' segments. */
    for (TCrv = CubicCrvs, i = 0; TCrv != NULL; TCrv = TCrv -> Pnext, i++) {
	assert(TCrv -> Order == 4 && TCrv -> Length == 4);
	HCrvs[i].TMin = i;
	HCrvs[i].TMax = i + 1.0;
	HCrvs[i].StartPosIdx = UserHCEditPointUniqueIndex++;
	HCrvs[i].EndPosIdx = UserHCEditPointUniqueIndex;
	HCrvs[i].Crv = NULL;
	HCrvs[i].Pl = NULL;
	HCrvs[i].EndPosCont = TRUE;
	
	UserHCEditUpdateFromCrv(&HCrvs[i], TCrv);
	UserHCEditUpdateOneHermite(HCE, i);
    }

    UserHCEditPointUniqueIndex++;

    CagdCrvFreeList(CubicCrvs);

    return HCE;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the periodic conditions of the given HC curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:		A handle on the Hermite curve to be examined.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if given HC curve is periodic, FALSE otherwise.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditIsPeriodic                                                     M
*****************************************************************************/
int UserHCEditIsPeriodic(VoidPtr HC)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

    return HCE -> IsPeriodic;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the periodic conditions of the given HC curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:		A handle on Hermite curve to be set periodic (or not).       M
*   Periodic:   TRUE if curve is to be closed (periodic).                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void					                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditSetPeriodic                                                    M
*****************************************************************************/
void UserHCEditSetPeriodic(VoidPtr HC, CagdBType Periodic)
{
    IrtPtType Pt;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    if ((HCE -> IsPeriodic = Periodic) != FALSE) {
	/* Make last point be the same as first. */
	IRIT_PT_ADD(Pt, HCrvs[HCE -> NumOfHCrvs - 1].EndPos, HCrvs[0].StartPos);
	IRIT_PT_SCALE(Pt, 0.5);
	IRIT_PT_COPY(HCrvs[HCE -> NumOfHCrvs - 1].EndPos, Pt);
	IRIT_PT_COPY(HCrvs[0].StartPos, Pt);

        /* Make last tangent be the same as first and update their Crv/Pl. */
        UserHCEditMakeTangentCont(HC, HCE -> NumOfHCrvs - 1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the continuity of the Index control point in the given curve.       M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	     A handle on edited Hermite curve to update its Index segment.   M
*   Index:   Of control point to get its C^1 continuity state.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE for C^1 continuity, FALSE for C^0.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditGetCtlPtCont                                                   M
*****************************************************************************/
CagdBType UserHCEditGetCtlPtCont(VoidPtr HC, int Index)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    if (Index == 0) {
        if (HCE -> IsPeriodic)
	    return HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont;
	else
	    return FALSE;
    }
    else
        return HCrvs[Index - 1].EndPosCont;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the continuity of the Index control point in the given curve.       M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	     A handle on edited Hermite curve to update its Index segment.   M
*   Index:   Of control point to change its C^1 continuity.                  M
*   Cont:    TRUE to make C^1 continuous, FALSE for only C^0.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditSetCtlPtCont                                                   M
*****************************************************************************/
void UserHCEditSetCtlPtCont(VoidPtr HC, int Index, CagdBType Cont)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    if (Index == 0 && HCE -> IsPeriodic)
	HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont = Cont;
    if (Index > 0)
        HCrvs[Index - 1].EndPosCont = Cont;

    if (Cont)
        UserHCEditMakeTangentCont(HC, Index == 0 ? HCE -> NumOfHCrvs - 1
						 : Index - 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the function to invoke when the control points/tangents are to be   M
* drawn.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	    A handle on edited Hermite curve to update its Index segment.    M
*   CtlPtDrawFunc:   Function to use to draw control points/tangent.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditSetDrawCtlptFunc                                               M
*****************************************************************************/
void UserHCEditSetDrawCtlptFunc(VoidPtr HC,
				UserHCEditDrawCtlPtFuncType CtlPtDrawFunc)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

    HCE -> _CtlPtDrawFunc = CtlPtDrawFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Delete the data structures for editing planar piecewise cubic Hermite    M
* curves.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:		A handle on the edited Hermite curve to be deleted.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditDelete                                                         M
*****************************************************************************/
void UserHCEditDelete(VoidPtr HC)
{
    int i;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditDelete\n");
#endif /* DEBUG_HC_EDIT */

    if (HCE -> MaxNumOfHCrvs > 0) {
        for (i = 0; i < HCE -> NumOfHCrvs; i++) {
	    if (HCE -> HCrvs[i].Crv != NULL)
	        CagdCrvFree(HCE -> HCrvs[i].Crv);
	    if (HCE -> HCrvs[i].Pl != NULL)
	        CagdPolylineFree(HCE -> HCrvs[i].Pl);
	}
	IritFree(HCE -> HCrvs);
    }
    IritFree(HCE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Duplicates the data structures for editing planar piecewise cubic        M
* Hermite curves.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:		A handle on the edited Hermite curve to be duplicated.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  Duplicated HC.                                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditCopy                                                           M
*****************************************************************************/
VoidPtr UserHCEditCopy(VoidPtr HC)
{
    int i;
    UserHCEditStruct
	*NewHCE = (UserHCEditStruct *) IritMalloc(sizeof(UserHCEditStruct)),
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct *NewHCrvs,
        *HCrvs = HCE -> HCrvs;

    /* Copy all slots. */
    *NewHCE = *HCE;
    NewHCrvs = NewHCE -> HCrvs = (UserHCStruct *)
	IritMalloc(HCE -> MaxNumOfHCrvs * sizeof(UserHCStruct));

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditCopy\n");
#endif /* DEBUG_HC_EDIT */

    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        NewHCrvs[i] = HCrvs[i];
	NewHCrvs[i].Crv = HCrvs[i].Crv ? CagdCrvCopy(HCrvs[i].Crv) : NULL;
	NewHCrvs[i].Pl = HCrvs[i].Pl ? CagdPolylineCopy(HCrvs[i].Pl) : NULL;
    }

    return NewHCE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Append control point at the end of the current curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	       A handle on edited Hermite curve to append control point to.  M
*   x, y:      The coordinate of the control point.			     M
*   MouseMode: 0 for mouse down, 1 for mouse move, 2 for mouse up (done).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditCreateAppendCtlpt                                              M
*****************************************************************************/
int UserHCEditCreateAppendCtlpt(VoidPtr HC,
				CagdRType x,
				CagdRType y,
				int MouseMode)
{
    IRIT_STATIC_DATA IrtRType
	*UpdatePos = NULL;
    int i;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditAppendCtlpt (%f %f), Mouse = %d\n", x, y, MouseMode);
#endif /* DEBUG_HC_EDIT */

    if (MouseMode == 0) { /* Mouse down. */
        if (HCE -> NumOfHCrvs == 0) {
	    if (HCE -> Mode == USER_HC_MODE_INSERT_CTLPTS) {
	        UpdatePos = HCrvs[0].EndPos;
		HCrvs[0].EndPosIdx = UserHCEditPointUniqueIndex++;
		HCrvs[0].TMin = 0.0;
		HCrvs[0].TMax = 1.0;
		HCrvs[0].Crv = NULL;
		HCrvs[0].Pl = NULL;
		HCrvs[0].EndPosCont = TRUE;
		HCE -> NumOfHCrvs = 1;/* We have 2 pts in - 1st curve valid! */
	    }
	    else {
	        UpdatePos = HCrvs[0].StartPos;
		HCrvs[0].StartPosIdx = UserHCEditPointUniqueIndex++;
		IRIT_VEC_RESET(HCrvs[0].StartTan);
	        HCE -> Mode = USER_HC_MODE_INSERT_CTLPTS;
	    }
	}
	else {				          /* Already have curves in. */
	    i = HCE -> NumOfHCrvs++;
	    UserHCEditReallocCrvs(HC);    /* Make sure we have enough space. */
	    HCrvs = HCE -> HCrvs; /* In case it was modified by the realloc. */
	    HCrvs[i].TMin = i;
	    HCrvs[i].TMax = i + 1.0;
	    HCrvs[i].Crv = NULL;
	    HCrvs[i].Pl = NULL;
	    HCrvs[i].EndPosCont = TRUE;

	    IRIT_PT_COPY(HCrvs[i].StartPos, HCrvs[i - 1].EndPos);
	    HCrvs[i].StartPosIdx = HCrvs[i - 1].EndPosIdx;

	    UpdatePos = HCrvs[i].EndPos;
	    HCrvs[i].EndPosIdx = UserHCEditPointUniqueIndex++;
	}
    }
    else if (MouseMode == 2) { /* Mouse up. */
        UpdatePos = NULL;
    }

    if (UpdatePos != NULL) {
        /* Update position. */
        UpdatePos[0] = x;
	UpdatePos[1] = y;
	UpdatePos[2] = 0.0;

	/* Update tangents. */
	if (HCE -> NumOfHCrvs > 0) {
	    i = HCE -> NumOfHCrvs - 1;
	    IRIT_VEC_SUB(HCrvs[i].EndTan, HCrvs[i].EndPos, HCrvs[i].StartPos);
	    IRIT_VEC_SCALE(HCrvs[i].EndTan, USER_HC_IRIT_VEC_SCALE);
	    if (i > 0) {
	        IRIT_VEC_SUB(HCrvs[i].StartTan,
			HCrvs[i].EndPos, HCrvs[i - 1].StartPos);
		IRIT_VEC_SCALE(HCrvs[i].StartTan, USER_HC_IRIT_VEC_SCALE);
	        IRIT_VEC_COPY(HCrvs[i - 1].EndTan, HCrvs[i].StartTan);
	        UserHCEditUpdateOneHermite(HC, i - 1);
	    }
	    else
	        IRIT_VEC_COPY(HCrvs[i].StartTan, HCrvs[i].EndTan);

	    UserHCEditUpdateOneHermite(HC, i);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes the interface for editing planar piecewie cubic Hermite	     M
* curves.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	   A handle on the edited Hermite curve to insert control point to.  M
*   LastX, LastY:   Last control point of curve, during creation stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditCreateDone                                                     M
*****************************************************************************/
int UserHCEditCreateDone(VoidPtr HC, CagdRType LastX, CagdRType LastY)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditCreateDone (%f %f)\n", LastX, LastY);
#endif /* DEBUG_HC_EDIT */

    HCE -> Mode = USER_HC_MODE_EDIT_CTLPTS;    /* 2nd mode - ctlpts editing. */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert new control point at the given parameter value of the curve.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	   A handle on the edited Hermite curve to insert control point to.  M
*   x, y:  The coordinate of the point.					     M
*   t:     The parameter location to insert a new control point.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditInsertCtlpt                                                    M
*****************************************************************************/
int UserHCEditInsertCtlpt(VoidPtr HC, CagdRType x, CagdRType y, CagdRType t)
{
    int i, CrvIndex = (int) t;
    IrtRType
        CrvParam = t - CrvIndex;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct *HCrvs;
    CagdCrvStruct *Crvs;

    assert(CrvIndex >= 0 && CrvIndex < HCE -> NumOfHCrvs);

    if (CrvParam < IRIT_EPS || CrvParam > 1.0 - IRIT_EPS)
	return FALSE;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditInsertCtlpt (%f %f) t = %f\n",
	   x, y, t);
#endif /* DEBUG_HC_EDIT */

    HCE -> NumOfHCrvs++;
    UserHCEditReallocCrvs(HC);		  /* Make sure we have enough space. */
    HCrvs = HCE -> HCrvs;	  /* In case it was modified by the realloc. */

    /* Shift all curves after the splitted one, by one. */
    for (i = HCE -> NumOfHCrvs - 1; i > CrvIndex; i--) {
        UserHCEditCopyCrv(&HCrvs[i], &HCrvs[i - 1]);
    }
    /* We duplicated points also, keep only one reference here. */
    HCrvs[CrvIndex + 1].Crv = NULL;
    HCrvs[CrvIndex + 1].Pl = NULL;

    /* Subdivide the CrvIndex segment at the CrvParam value and update the   */
    /* two new curves we have instead of it.				     */
    Crvs = CagdCrvSubdivAtParam(HCrvs[CrvIndex].Crv, CrvParam);
    UserHCEditUpdateFromCrv(&HCrvs[CrvIndex], Crvs);
    UserHCEditUpdateFromCrv(&HCrvs[CrvIndex + 1], Crvs -> Pnext);
    CagdCrvFreeList(Crvs);

    /* Update the new point's index. */
    HCrvs[CrvIndex].EndPosIdx =
        HCrvs[CrvIndex + 1].StartPosIdx = UserHCEditPointUniqueIndex++;

    /* Update the new point's smoothness to be continuous. After all it was */
    /* C^infinity before we broke the curve here...			    */
    HCrvs[CrvIndex].EndPosCont = TRUE;

    /* Update the two new hcurves' polylines and curves. */
    UserHCEditUpdateOneHermite(HC, CrvIndex);
    UserHCEditUpdateOneHermite(HC, CrvIndex + 1);

    /* Update the (uniform) parameter domains of all the curve. */
    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        HCrvs[i].TMin = i;
	HCrvs[i].TMax = i + 1.0;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Delete the control point at the given location on the current curve.     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	   A handle on edited Hermite curve to delete control point from.    M
*   x, y:  The coordinate of the point.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditDeleteCtlpt                                                    M
*****************************************************************************/
int UserHCEditDeleteCtlpt(VoidPtr HC, CagdRType x, CagdRType y)
{
    int i, Index, ID;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditDeleteCtlpt (%f %f)\n", x, y);
#endif /* DEBUG_HC_EDIT */

    /* Assume we are near a control point so just find the closest one. */
    if (!UserHCEditIsNearCtlPt(HC, &x, &y, &Index, &ID, IRIT_INFNTY))
        return FALSE;

    assert(Index >= 0 && Index <= HCE -> NumOfHCrvs);

    /* Make sure we have at least one curve. */
    if (HCE -> NumOfHCrvs <= 1)
	return FALSE;

    /* Duplicate the control points of the deleted curve to its neighbor. */
    if (Index == 0) {
	if (HCE -> IsPeriodic) {
	    /* Update last segment to now connect to the second. */
	    IRIT_PT_COPY(HCrvs[HCE -> NumOfHCrvs - 1].EndPos, HCrvs[0].EndPos);
	    IRIT_VEC_COPY(HCrvs[HCE -> NumOfHCrvs - 1].EndTan, HCrvs[0].EndTan);
	    HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont = HCrvs[0].EndPosCont;
	    HCrvs[HCE -> NumOfHCrvs - 1].EndPosIdx = HCrvs[0].EndPosIdx;
	}
    }
    else if (Index < HCE -> NumOfHCrvs) {
        IRIT_PT_COPY(HCrvs[Index - 1].EndPos, HCrvs[Index].EndPos);
	IRIT_VEC_COPY(HCrvs[Index - 1].EndTan, HCrvs[Index].EndTan);
	HCrvs[Index - 1].EndPosCont = HCrvs[Index].EndPosCont;
	HCrvs[Index - 1].EndPosIdx = HCrvs[Index].EndPosIdx;
    }

    /* Shift all curves back, removing the Index curve. */
    if (Index < HCE -> NumOfHCrvs) {
        if (HCrvs[Index].Crv != NULL)
	    CagdCrvFree(HCrvs[Index].Crv);
	if (HCrvs[Index].Pl != NULL)
	    CagdPolylineFree(HCrvs[Index].Pl);
    }
    for (i = Index; i < HCE -> NumOfHCrvs - 1; i++) {
        UserHCEditCopyCrv(&HCrvs[i], &HCrvs[i + 1]);
    }

    HCE -> NumOfHCrvs--;

    /* Update the modified hcurve's polylines and curve. */
    UserHCEditUpdateOneHermite(HC, Index == 0 ? HCE -> NumOfHCrvs - 1
					      : Index - 1);

    /* Update the (uniform) parameter domains of all the curve. */
    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        HCrvs[i].TMin = i;
	HCrvs[i].TMax = i + 1.0;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Uodate a control point or tangent of the current curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	         A handle on edited Hermite curve to move control pt/tan in. M
*   CtlIndex:    The index of the control pt/tan to update		     M
*   IsPosition:  TRUE if we aim at updating the position of control pt/tan.  M
*		 If FALSE, positive CtlIndex denotes forward tangent whereas M
*		 negative CtlIndex denotes a backward tangent.		     M
*   NewX, NewY:  New position or tangent vector to update with.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditUpdateCtl                                                      M
*****************************************************************************/
int UserHCEditUpdateCtl(VoidPtr HC,
			int CtlIndex,
			CagdBType IsPosition,
			CagdRType NewX,
			CagdRType NewY)
{
    int CtlIndex1;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    if (IsPosition) {
        if (CtlIndex < 0 || CtlIndex > HCE -> NumOfHCrvs)
	    return FALSE;
	CtlIndex1 = CtlIndex > 0 ? CtlIndex - 1 : HCE -> NumOfHCrvs - 1;

	if (CtlIndex < HCE -> NumOfHCrvs) {
	    HCrvs[CtlIndex].StartPos[0] = NewX;
	    HCrvs[CtlIndex].StartPos[1] = NewY;
	    UserHCEditUpdateOneHermite(HC, CtlIndex);
	}
	if (CtlIndex > 0 || HCE -> IsPeriodic) {
	    HCrvs[CtlIndex1].EndPos[0] = NewX;
	    HCrvs[CtlIndex1].EndPos[1] = NewY;
	    UserHCEditUpdateOneHermite(HC, CtlIndex1);
	}

	return TRUE;
    }
    else { /* Tangent. */
        int Foreward = CtlIndex >= 0;

	CtlIndex = IRIT_ABS(CtlIndex);
        if (CtlIndex < 0 || CtlIndex > HCE -> NumOfHCrvs)
	    return FALSE;
	CtlIndex1 = CtlIndex > 0 ? CtlIndex - 1 : HCE -> NumOfHCrvs;

	if (Foreward) {
	    /* Update StartTan of CtlIndex curve. */
	    HCrvs[CtlIndex].StartTan[0] =
	        (NewX - HCrvs[CtlIndex].StartPos[0]) / USER_HC_VEC_DRAW_SCALE;
	    HCrvs[CtlIndex].StartTan[1] =
	        (NewY - HCrvs[CtlIndex].StartPos[1]) / USER_HC_VEC_DRAW_SCALE;
	}
	else {
	    /* Udpate EndTan of CtlIndex1. */
	    HCrvs[CtlIndex1].EndTan[0] =
	        (HCrvs[CtlIndex1].EndPos[0] - NewX) / USER_HC_VEC_DRAW_SCALE;
	    HCrvs[CtlIndex1].EndTan[1] =
	        (HCrvs[CtlIndex1].EndPos[1] - NewY) / USER_HC_VEC_DRAW_SCALE;
	}

	if ((CtlIndex == 0 &&
	     HCE -> IsPeriodic &&
	     HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont) ||
	    (CtlIndex > 0 && HCrvs[CtlIndex1].EndPosCont)) {
	    /* This is a C1 continuous connection.  Update other tangent. */
	    if (Foreward) {
	        IrtRType
		    R = IRIT_PT2D_LENGTH(HCrvs[CtlIndex1].EndTan);

	        HCrvs[CtlIndex1].EndTan[0] = HCrvs[CtlIndex].StartTan[0];
	        HCrvs[CtlIndex1].EndTan[1] = HCrvs[CtlIndex].StartTan[1];
		IRIT_PT2D_NORMALIZE(HCrvs[CtlIndex1].EndTan);
		IRIT_PT2D_SCALE(HCrvs[CtlIndex1].EndTan, R);
	    }
	    else {
	        IrtRType
		    R = IRIT_PT2D_LENGTH(HCrvs[CtlIndex1].StartTan);

	        HCrvs[CtlIndex].StartTan[0] = HCrvs[CtlIndex1].EndTan[0];
		HCrvs[CtlIndex].StartTan[1] = HCrvs[CtlIndex1].EndTan[1];
		IRIT_PT2D_NORMALIZE(HCrvs[CtlIndex].StartTan);
		IRIT_PT2D_SCALE(HCrvs[CtlIndex].StartTan, R);
	    }
	}

	UserHCEditMakeTangentCont(HC, CtlIndex > 0 ? CtlIndex - 1
						   : HCE -> NumOfHCrvs - 1);

	return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Move a control point or tangent (the closest detected) of the current    M
* curve.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	         A handle on edited Hermite curve to move control pt/tan in. M
*   OldX, OldY:  The original coordinate of the control pt/tan.		     M
*   NewX, NewY:  The original coordinate of the control pt/tan.		     M
*   MouseMode:   0 for mouse down, 1 for mouse move, 2 for mouse up (done),  M
*                3 for reset/clear state.				     M
*   MinDist:     If not NULL and MouseMode is 0, returns the minimal	     M
*                distance found.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditMoveCtl                                                        M
*****************************************************************************/
int UserHCEditMoveCtl(VoidPtr HC,
		      CagdRType OldX,
		      CagdRType OldY,
		      CagdRType NewX,
		      CagdRType NewY,
		      int MouseMode,
		      CagdRType *MinDist)
{
    IRIT_STATIC_DATA int
	IsActive = FALSE,
	IsTangent = FALSE;
    int Index, ID, RetVal;
    CagdBType Forward;
    CagdRType OldPos[2], ClosestTan[2], ClosestPos[2];

    switch (MouseMode) {
        case 0:
            /* See if we are closer to control point or control tangent. */
            OldPos[0] = ClosestTan[0] = ClosestPos[0] = OldX;
	    OldPos[1] = ClosestTan[1] = ClosestPos[1] = OldY;

	    UserHCEditIsNearCtlPt(HC, &ClosestPos[0], &ClosestPos[1],
				  &Index, &ID, IRIT_INFNTY);

	    UserHCEditIsNearCtlTan(HC, &ClosestTan[0], &ClosestTan[1],
				   &Index, &ID, &Forward, IRIT_INFNTY);

	    IsTangent = IRIT_PT2D_DIST_SQR(OldPos, ClosestTan) <=
			IRIT_PT2D_DIST_SQR(OldPos, ClosestPos);
	    if (MinDist != NULL) {
		*MinDist = IRIT_MIN(IRIT_PT2D_DIST_SQR(OldPos, ClosestTan),
			       IRIT_PT2D_DIST_SQR(OldPos, ClosestPos));
		*MinDist = sqrt(*MinDist);
	    }
	    IsActive = TRUE;
	case 1:
	case 2:
	    RetVal = FALSE;
	    if (IsActive) {
	        if (IsTangent) {
		    RetVal = UserHCEditMoveCtlTan(HC, OldX, OldY, NewX, NewY,
						  MouseMode);
		}
		else {
		    RetVal = UserHCEditMoveCtlPt(HC, OldX, OldY, NewX, NewY,
						 MouseMode);
		}
	    }
	    if (MouseMode == 2)
	        IsActive = FALSE;
	    return RetVal;
	case 3:
	    IsActive = FALSE;
	    return TRUE;
	default:
	    IsActive = FALSE;
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Move a control point of the current curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	         A handle on edited Hermite curve to move control point in.  M
*   OldX, OldY:  The original coordinate of the point.			     M
*   NewX, NewY:  The new coordinate of the point.			     M
*   MouseMode:   0 for mouse down, 1 for mouse move, 2 for mouse up (done).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditMoveCtlPt                                                      M
*****************************************************************************/
int UserHCEditMoveCtlPt(VoidPtr HC,
			CagdRType OldX,
			CagdRType OldY,
			CagdRType NewX,
			CagdRType NewY,
			int MouseMode)
{
    IRIT_STATIC_DATA int
        CtlPtIndexOnMouseDown = -1;
    int ID;
    CagdRType x, y;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditMoveCtlPt (%f %f) (%f %f), Mouse = %d\n",
	   OldX, OldY, NewX, NewY, MouseMode);
#endif /* DEBUG_HC_EDIT */

    switch (MouseMode) {
	case 0:		  /* Find the relevant control point, on mouse-down. */
	    x = OldX;
	    y = OldY;
	    return UserHCEditIsNearCtlPt(HC, &x, &y,
				 &CtlPtIndexOnMouseDown, &ID, IRIT_INFNTY);
	case 1:    /* Update relevant control with a new pos, on mouse-move. */
	    if (CtlPtIndexOnMouseDown == 0) {
		HCrvs[0].StartPos[0] = NewX;
		HCrvs[0].StartPos[1] = NewY;

		if (HCE -> IsPeriodic) {
		    HCrvs[HCE -> NumOfHCrvs - 1].EndPos[0] = NewX;
		    HCrvs[HCE -> NumOfHCrvs - 1].EndPos[1] = NewY;
		}
		
		UserHCEditMakeTangentCont(HC, HCE -> NumOfHCrvs - 1);
	    }
	    else if (CtlPtIndexOnMouseDown <= HCE -> NumOfHCrvs) {
		HCrvs[CtlPtIndexOnMouseDown - 1].EndPos[0] = NewX;
		HCrvs[CtlPtIndexOnMouseDown - 1].EndPos[1] = NewY;

		if (CtlPtIndexOnMouseDown < HCE -> NumOfHCrvs) {
		    HCrvs[CtlPtIndexOnMouseDown].StartPos[0] = NewX;
		    HCrvs[CtlPtIndexOnMouseDown].StartPos[1] = NewY;
		}

		UserHCEditMakeTangentCont(HC, CtlPtIndexOnMouseDown - 1);
	    }
	    else
		return FALSE;
	    return TRUE;
	case 2:					   /* Clear up, on mouse-up. */
	default:
	    CtlPtIndexOnMouseDown = -1;
	    return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Move a control tangent of the current curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	         A handle on edited Hermite curve to move control tangent in.M
*   OldX, OldY:  The original coordinate of the tangent.		     M
*   NewX, NewY:  The new coordinate of the tangent.			     M
*   MouseMode:   0 for mouse down, 1 for mouse move, 2 for mouse up (done).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditMoveCtlTan                                                     M
*****************************************************************************/
int UserHCEditMoveCtlTan(VoidPtr HC,
			 CagdRType OldX,
			 CagdRType OldY,
			 CagdRType NewX,
			 CagdRType NewY,
			 int MouseMode)
{
    IRIT_STATIC_DATA int
        CtlTanIndexOnMouseDown = -1,
	CtlTanForwardOnMouseDown = FALSE;
    IRIT_STATIC_DATA IrtVecType
	CtlTanOrig;
    int RetVal, ID;
    CagdRType x, y;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditMoveCtlTan (%f %f) (%f %f), Mouse = %d\n",
	   OldX, OldY, NewX, NewY, MouseMode);
#endif /* DEBUG_HC_EDIT */

    switch (MouseMode) {
	case 0:		  /* Find the relevant control point, on mouse-down. */
	    x = OldX;
	    y = OldY;
	    if ((RetVal = UserHCEditIsNearCtlTan(HC, &x, &y,
						 &CtlTanIndexOnMouseDown, &ID,
						 &CtlTanForwardOnMouseDown,
						 IRIT_INFNTY)) != FALSE) {
	        assert(CtlTanIndexOnMouseDown >= 0 &&
		       CtlTanIndexOnMouseDown <= HCE -> NumOfHCrvs);

	        if (CtlTanForwardOnMouseDown) {
		    IRIT_VEC2D_COPY(CtlTanOrig,
			       HCrvs[CtlTanIndexOnMouseDown].StartTan);
		}
		else {
		    IRIT_VEC2D_COPY(CtlTanOrig,
			       HCrvs[CtlTanIndexOnMouseDown - 1].EndTan);
		}
	    }
	    return RetVal;
	case 1:    /* Update relevant control with a new tan, on mouse-move. */
	    assert(CtlTanIndexOnMouseDown >= 0 &&
		       CtlTanIndexOnMouseDown <= HCE -> NumOfHCrvs);

	    if (CtlTanIndexOnMouseDown == 0) {
	        assert(CtlTanForwardOnMouseDown); /* Only forward for Idx 0. */

		HCrvs[0].StartTan[0] = NewX - HCrvs[0].StartPos[0];
		HCrvs[0].StartTan[1] = NewY - HCrvs[0].StartPos[1];
		IRIT_VEC_SCALE(HCrvs[0].StartTan, 1.0 / USER_HC_VEC_DRAW_SCALE);
		UserHCEditUpdateOneHermite(HC, 0);

		if (HCE -> IsPeriodic &&
		    HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont) {
		    UserHCEditRedirectTangent(
					HCrvs[HCE -> NumOfHCrvs - 1].EndTan,
					HCrvs[0].StartTan);
		    UserHCEditUpdateOneHermite(HC, HCE -> NumOfHCrvs - 1);
		}
	    }
	    else if (CtlTanIndexOnMouseDown == HCE -> NumOfHCrvs) {
	        assert(!CtlTanForwardOnMouseDown);/* Only back for last Idx. */

		HCrvs[HCE -> NumOfHCrvs - 1].EndTan[0] =
				HCrvs[HCE -> NumOfHCrvs - 1].EndPos[0] - NewX;
		HCrvs[HCE -> NumOfHCrvs - 1].EndTan[1] =
				HCrvs[HCE -> NumOfHCrvs - 1].EndPos[1] - NewY;
		IRIT_VEC_SCALE(HCrvs[HCE -> NumOfHCrvs - 1].EndTan,
			  1.0 / USER_HC_VEC_DRAW_SCALE);
		UserHCEditUpdateOneHermite(HC, HCE -> NumOfHCrvs - 1);

		if (HCE -> IsPeriodic &&
		    HCrvs[HCE -> NumOfHCrvs - 1].EndPosCont) {
		    UserHCEditRedirectTangent(
					HCrvs[0].StartTan,
					HCrvs[HCE -> NumOfHCrvs - 1].EndTan);
		    UserHCEditUpdateOneHermite(HC, 0);
		}
	    }
	    else {
	        if (CtlTanForwardOnMouseDown) {
		    HCrvs[CtlTanIndexOnMouseDown].StartTan[0] =
			      NewX - HCrvs[CtlTanIndexOnMouseDown].StartPos[0];
		    HCrvs[CtlTanIndexOnMouseDown].StartTan[1] =
			      NewY - HCrvs[CtlTanIndexOnMouseDown].StartPos[1];
		    IRIT_VEC_SCALE(HCrvs[CtlTanIndexOnMouseDown].StartTan,
			      1.0 / USER_HC_VEC_DRAW_SCALE);
		    UserHCEditUpdateOneHermite(HC, CtlTanIndexOnMouseDown);

		    if (HCrvs[CtlTanIndexOnMouseDown - 1].EndPosCont) {
		        UserHCEditRedirectTangent(
				     HCrvs[CtlTanIndexOnMouseDown - 1].EndTan,
				     HCrvs[CtlTanIndexOnMouseDown].StartTan);
			UserHCEditUpdateOneHermite(HC,
						   CtlTanIndexOnMouseDown - 1);
		    }
		}
		else {
		    HCrvs[CtlTanIndexOnMouseDown - 1].EndTan[0] =
			    HCrvs[CtlTanIndexOnMouseDown - 1].EndPos[0] - NewX;
		    HCrvs[CtlTanIndexOnMouseDown - 1].EndTan[1] =
			    HCrvs[CtlTanIndexOnMouseDown - 1].EndPos[1] - NewY;
		    IRIT_VEC_SCALE(HCrvs[CtlTanIndexOnMouseDown - 1].EndTan,
			      1.0 / USER_HC_VEC_DRAW_SCALE);
		    UserHCEditUpdateOneHermite(HC, CtlTanIndexOnMouseDown - 1);
		    
		    if (HCrvs[CtlTanIndexOnMouseDown - 1].EndPosCont) {
		        UserHCEditRedirectTangent(
				     HCrvs[CtlTanIndexOnMouseDown].StartTan,
				     HCrvs[CtlTanIndexOnMouseDown - 1].EndTan);
			UserHCEditUpdateOneHermite(HC, CtlTanIndexOnMouseDown);
		    }
		}
	    }
	    return TRUE;
	case 2:					   /* Clear up, on mouse-up. */
	default:
	    CtlTanIndexOnMouseDown = -1;
	    return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to examine if given location is near HC to within Eps.           M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	   A handle on edited hermite curve to see if we are close.          M
*   x, y:  The coordinate of the given location.			     M
*   t:     The closest parameter location detected near the mouse.	     M
*   Eps:   Distance to consider it near.				     M
*   NormalizeZeroOne:  If TRUE detected parameter is normalized into zero to M
*	   one for beginning to end.  Otherwise, if FALSE, the returned	     M
*	   parameter is 'x.y' where x is the Hermite segment number and y    M
*	   is the relative location within the segment.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if given location is indeed near curve, FALSE otherwise.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditIsNearCrv		                                             M
*****************************************************************************/
int UserHCEditIsNearCrv(VoidPtr HC,
			CagdRType x,
			CagdRType y,
			CagdRType *t,
			CagdRType Eps,
			int NormalizeZeroOne)
{
    int i, j;
    IrtRType d, R,
	EpsSqr = IRIT_SQR(Eps),
	MinDistSqr = EpsSqr;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
	*HCrvs = HCE -> HCrvs;

    if (HCE -> NumOfHCrvs == 0)
	return FALSE;

    *t = 0.0;

    for (j = 0; j < HCE -> NumOfHCrvs; j++) {
        IrtVecType Vec, Vec1, Vec2;
        IrtPtType Pt, ClosestPt;
	CagdPolylineStruct
	    *Pl = HCrvs[j].Pl;
	CagdPolylnStruct *Plln;

	assert(Pl != NULL);
	Plln = Pl -> Polyline;
	Pt[0] = x;
	Pt[1] = y;
	Pt[2] = 0.0;
	Vec[2] = 0.0;

	/* Go over the polyline and find the closest location. */
	for (i = 1; i < Pl -> Length; i++) {
	    IRIT_PT2D_SUB(Vec, Plln[i].Pt, Plln[i - 1].Pt);
	    GMPointFromPointLine(Pt, Plln[i - 1].Pt, Vec, ClosestPt);
	    if ((d = IRIT_PT2D_DIST_SQR(Pt, ClosestPt)) < MinDistSqr) {
	        /* Verify ClosestPt is between Plln[i].Pt & Plln[i - 1].Pt. */
	        IRIT_PT2D_SUB(Vec1, ClosestPt, Plln[i - 1].Pt);
	        IRIT_PT2D_SUB(Vec2, ClosestPt, Plln[i].Pt);

		if (IRIT_DOT_PROD_2D(Vec1, Vec2) <= 0.0) {
		    /* Projection is indeed inside the line segment. */
		    MinDistSqr = d;
		    /* Compute the parameter inside the curve over [0..1]. */
		    R = IRIT_VEC2D_LENGTH(Vec1);
		    *t = (R / (R + IRIT_VEC2D_LENGTH(Vec2)) + i - 1.0) /
							(Pl -> Length - 1.0);

		    /* Map this parameter to the real domain of this curve. */
		    *t = HCrvs[j].TMin + (HCrvs[j].TMax - HCrvs[j].TMin) * *t;
		}
	    }

	    if ((d = IRIT_PT2D_DIST_SQR(Pt, Plln[i].Pt)) < MinDistSqr) {
	        /* Distance to sampled points is the smallest. */
	        MinDistSqr = d;
		/* Compute the parameter inside the curve over [0..1]. */
		*t = i / (Pl -> Length - 1.0);

		/* Map this parameter to the real domain of this curve. */
		*t = HCrvs[j].TMin + (HCrvs[j].TMax - HCrvs[j].TMin) * *t;
	    }
	}
    }

    if (NormalizeZeroOne) {
        /* Normalize into zero to one. */
        *t /= (HCrvs[HCE -> NumOfHCrvs - 1].TMax + IRIT_UEPS);
    }

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditIsNearCrv=%d (%f %f), Eps = %f (t = %f)\n",
	   MinDistSqr < EpsSqr, x, y, Eps, 
	   MinDistSqr < EpsSqr ? *t : -1.0);
#endif /* DEBUG_HC_EDIT */

    return MinDistSqr < EpsSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to examine if given location is near a control point of HC to    M
* within Eps.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	   A handle on edited Hermite curve to see if we are close.          M
*   x, y:  The coordinate of the given location.  This coordinated will be   M
*	   updated with the precise control point location, if indeed near.  M
*   Index: Of closest control point, if found any below distance Eps, in     M
*	   the curve, first points is indexed 0.			     M
*   UniqueID:  The unique integer ID that is assigned to this control point. M
*   Eps:   Distance to consider it near.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if given location is indeed near a control point, 	     M
*          FALSE otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditIsNearCtlPt		                                     M
*****************************************************************************/
int UserHCEditIsNearCtlPt(VoidPtr HC,
			  CagdRType *x,
			  CagdRType *y,
			  int *Index,
			  int *UniqueID,
			  CagdRType Eps)
{
    int i,
	RetVal = FALSE;
    CagdRType dSqr, MinDistSqr;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    /* Examine first point in the curve. */
    MinDistSqr = IRIT_SQR(HCrvs[0].StartPos[0] - *x) +
		 IRIT_SQR(HCrvs[0].StartPos[1] - *y);
    *Index = (MinDistSqr < IRIT_SQR(Eps)) ? 0 : -1;
    *UniqueID = HCrvs[0].StartPosIdx;

    /* Find the closest control point to the given location. */
    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        dSqr = IRIT_SQR(HCrvs[i].EndPos[0] - *x) +
	       IRIT_SQR(HCrvs[i].EndPos[1] - *y);
	if (dSqr < MinDistSqr) {
	    MinDistSqr = dSqr;
	    *Index = i + 1;
	    *UniqueID = HCrvs[i].EndPosIdx;
	}
    }

    if (HCE -> IsPeriodic && *Index >= HCE -> NumOfHCrvs) {
        *Index = 0;
	*UniqueID = HCrvs[0].StartPosIdx;
    }

    if (MinDistSqr < IRIT_SQR(Eps)) {
        if (*Index == 0) {
	    *x = HCrvs[0].StartPos[0];
	    *y = HCrvs[0].StartPos[1];
	}
	else {
	    *x = HCrvs[*Index - 1].EndPos[0];
	    *y = HCrvs[*Index - 1].EndPos[1];
	}
	RetVal = TRUE;
    }
    else {
	RetVal = FALSE;
    }

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditIsNearCtlPt=%d (%f %f), Index = %d, UniqueID = %d\n",
	   RetVal, *x, *y, *Index, *UniqueID);
#endif /* DEBUG_HC_EDIT */

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to examine if given location is near a control tangent of HC, to M
* within Eps.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	      A handle on edited Hermite curve to see if we are close.       M
*   x, y:     The coordinate of the given location.  This coordinated will   M
*	      be updated with the precise control tangent location, if       M
*	      indeed near.						     M
*   Index:    Of closest control tangent, if found any below distance Eps.   M
*   UniqueID: The unique integer ID that is assigned to this control point.  M
*   Forward:  TRUE for forward tangent, FALSE for backward tangent.	     M
*   Eps:      Distance to consider it near.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if given location is indeed near a control tangent, 	     M
*          FALSE otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditIsNearCtlTan		                                     M
*****************************************************************************/
int UserHCEditIsNearCtlTan(VoidPtr HC,
			   CagdRType *x,
			   CagdRType *y,
			   int *Index,
			   int *UniqueID,
			   CagdBType *Forward,
			   CagdRType Eps)
{
    int i,
	RetVal = FALSE;
    CagdRType dSqr,
	NewX = -1,
	NewY = -1,
	MinDistSqr = IRIT_SQR(Eps);
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    /* Find the closest control tangent to the given location. */
    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        dSqr = IRIT_SQR(HCrvs[i].StartPos[0] +
		   HCrvs[i].StartTan[0] * USER_HC_VEC_DRAW_SCALE - *x) +
	       IRIT_SQR(HCrvs[i].StartPos[1] +
		   HCrvs[i].StartTan[1] * USER_HC_VEC_DRAW_SCALE - *y);
	if (dSqr < MinDistSqr) {
	    MinDistSqr = dSqr;
	    NewX = HCrvs[i].StartPos[0] +
	           HCrvs[i].StartTan[0] * USER_HC_VEC_DRAW_SCALE;
	    NewY = HCrvs[i].StartPos[1] +
	           HCrvs[i].StartTan[1] * USER_HC_VEC_DRAW_SCALE;
	    *Index = i;
	    *UniqueID = HCrvs[i].StartPosIdx;
	    *Forward = TRUE;
	}

        dSqr = IRIT_SQR(HCrvs[i].EndPos[0] -
		   HCrvs[i].EndTan[0] * USER_HC_VEC_DRAW_SCALE - *x) +
	       IRIT_SQR(HCrvs[i].EndPos[1] -
		   HCrvs[i].EndTan[1] * USER_HC_VEC_DRAW_SCALE - *y);
	if (dSqr < MinDistSqr) {
	    MinDistSqr = dSqr;
	    NewX = HCrvs[i].EndPos[0] -
	           HCrvs[i].EndTan[0] * USER_HC_VEC_DRAW_SCALE;
	    NewY = HCrvs[i].EndPos[1] -
	           HCrvs[i].EndTan[1] * USER_HC_VEC_DRAW_SCALE;
	    *Index = i + 1;
	    *UniqueID = HCrvs[i].EndPosIdx;
	    *Forward = FALSE;
	}
    }

    if ((RetVal = (MinDistSqr < IRIT_SQR(Eps))) != FALSE) {
        *x = NewX;
        *y = NewY;
	RetVal = TRUE;
    }

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditIsNearCtlTan=%d (%f %f), Index = %d, UniqueID = %d, Forward = %d\n",
	   RetVal, *x, *y, *Index, *UniqueID, *Forward);
#endif /* DEBUG_HC_EDIT */

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs one (Bspline) curve representation for the given HC.          M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	     A handle on edited Hermite curve to convert to Bspline.         M
*   ArcLen:  TRUE to approximately parametrize the geometry to follow arc    M
*	     length parametrization, FALSE to apply uniform parametrization. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A Bspline curve representing the given HC.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditGetCrvRepresentation                                           M
*****************************************************************************/
CagdCrvStruct *UserHCEditGetCrvRepresentation(VoidPtr HC, int ArcLen)
{
    int i, j, m, BspCrvLen;
    IrtRType **BzrPts, **BspPts, *KV, Dt;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;
    CagdCrvStruct *BspCrv, *BzrCrv;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditGetCrvRepresentation\n");
#endif /* DEBUG_HC_EDIT */

    if (HCE -> NumOfHCrvs == 0)
	return NULL;

    BspCrvLen = HCE -> NumOfHCrvs * 3 + 1;
    BspCrv = BspCrvNew(BspCrvLen, 4, CAGD_PT_E2_TYPE);
    BspPts = BspCrv -> Points;
    KV = BspCrv -> KnotVector;

    /* Update the control points */
    for (i = m = 0; i < HCE -> NumOfHCrvs; i++) {
        BzrCrv = HCrvs[i].Crv;
	assert(BzrCrv != NULL);
	BzrPts = BzrCrv -> Points;

	for (j = i != 0; j < 4; j++, m++) {
	    BspPts[1][m] = BzrPts[1][j];
	    BspPts[2][m] = BzrPts[2][j];
	}
    }

    /* Update the knot sequence. */
    for (m = 0; m < 4; m++)
	KV[m] = HCrvs[0].TMin;
    for (i = 0; i < HCE -> NumOfHCrvs; i++, m += 3) {
        /* Approximate arc length, as control polygon's arc length? */
        if (ArcLen) {
	    if ((Dt = CagdCrvArcLenPoly(HCrvs[i].Crv)) == 0.0)
	        Dt = IRIT_EPS;
	}
	else
	    Dt = 1.0;

	KV[m] = KV[m + 1] = KV[m + 2] = Dt + KV[m - 1];
    }
    KV[m] = KV[m - 1];

    /* Make domain zero to one. */
    BspKnotAffineTransOrder2(BspCrv -> KnotVector, BspCrv -> Order,
			     BspCrv -> Length + BspCrv -> Order, 0.0, 1.0);

    return BspCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns number of HC control points this HC curve has.		     M
* These points are end points of Hermite intervals.  For example, a curve    M
* with three Hermite intervals will have 4 HC control points.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	      A handle on edited Hermite curve to get its size.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Number of HC control points.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditGetNumCtlPt                                                    M
*****************************************************************************/
int UserHCEditGetNumCtlPt(VoidPtr HC)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

    return HCE -> NumOfHCrvs + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the position and tangent of the Index control point of the HC    M
* curve.  First point is index zero.  Tan is always pointing along the       M
* forward moving direction of the curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	      A handle on edited Hermite curve to feicth one of its HC       M
*	      control points and tangent.				     M
*   Index:    Of control point and tangent to fetch.  If Index is negative   M
*	      the backward tangent is fecthed.				     M
*   Pos:      Position to fetch.					     M
*   Tan:      Tangent to fetch.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditGetCtlPtTan                                                    M
*****************************************************************************/
int UserHCEditGetCtlPtTan(VoidPtr HC, int Index, CagdPType Pos, CagdPType Tan)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    Pos[2] = Tan[2] = 0.0; /* A 2D curve! */

    if (Index >= 0 && Index < HCE -> NumOfHCrvs) {
	IRIT_VEC2D_COPY(Tan, HCrvs[Index].StartTan);
    }
    else if (Index < 0 || Index == HCE -> NumOfHCrvs) {
	Index = IRIT_ABS(Index);
	IRIT_VEC2D_COPY(Tan, HCrvs[Index - 1].EndTan);
    }

    if (Index >= 0 && Index < HCE -> NumOfHCrvs) {
        IRIT_PT2D_COPY(Pos, HCrvs[Index].StartPos);
        return TRUE;
    }
    else if (Index == HCE -> NumOfHCrvs) {
        IRIT_PT2D_COPY(Pos, HCrvs[Index - 1].EndPos);
        return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the control points of the given HC, using the defined drawing       M
* function.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	      A handle on edited Hermite curve to update its Index segment.  M
*   DrawTans: Do we want to draw tangents as well?			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditDrawCtlpts                                                     M
*****************************************************************************/
int UserHCEditDrawCtlpts(VoidPtr HC, int DrawTans)
{
    int i,
	PtIndex = 0;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

    if (HCE -> _CtlPtDrawFunc == NULL)
        return FALSE;

    if (HCE -> NumOfHCrvs == 0) {
        /* Can only happen if this curve is being created and only first    */
        /* point of first curve is valid - draw only it.		   */
        (HCE -> _CtlPtDrawFunc)(0, HCrvs[0].StartPosIdx, HCrvs[0].StartPos,
				NULL, NULL);
	return TRUE;
    }

    if (DrawTans) {
        if (HCE -> IsPeriodic) {
	    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
	        (HCE -> _CtlPtDrawFunc)(PtIndex++, HCrvs[i].StartPosIdx,
					HCrvs[i].StartPos,
					HCrvs[i == 0 ?
					          HCE -> NumOfHCrvs - 1 :
					          i - 1].EndTan,
					HCrvs[i].StartTan);
	    }
	}
	else {
	    (HCE -> _CtlPtDrawFunc)(PtIndex++, HCrvs[0].StartPosIdx,
				    HCrvs[0].StartPos,
				    NULL, HCrvs[0].StartTan);

	    for (i = 0; i < HCE -> NumOfHCrvs - 1; i++) {
	        (HCE -> _CtlPtDrawFunc)(PtIndex++, HCrvs[i].EndPosIdx,
					HCrvs[i].EndPos, HCrvs[i].EndTan,
					HCrvs[i + 1].StartTan);
	    }

	    (HCE -> _CtlPtDrawFunc)(PtIndex++, 
				    HCrvs[HCE -> NumOfHCrvs - 1].EndPosIdx,
				    HCrvs[HCE -> NumOfHCrvs - 1].EndPos,
				    HCrvs[HCE -> NumOfHCrvs - 1].EndTan,
				    NULL);
	}
    }
    else {
	(HCE -> _CtlPtDrawFunc)(PtIndex++, HCrvs[0].StartPosIdx,
				HCrvs[0].StartPos, NULL, NULL);
        for (i = 0; i < HCE -> NumOfHCrvs; i++) {
	    (HCE -> _CtlPtDrawFunc)(PtIndex++, HCrvs[i].EndPosIdx,
				    HCrvs[i].EndPos, NULL, NULL);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Applies given transformion, in place, to given curve HC as specified by  M
* Mat.  Only the XY coordinates are processed.				     M
*   Curve HC is first translated by Dir and then scaled by Scale.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	        A handle on edited Hermite curve to transform, in place.     M
*   Mat:        Transformation matrxi to apply to HC.			     
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, UserHCEditInit, UserHCEditTransform                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditMatTrans                                                       M
*****************************************************************************/
int UserHCEditMatTrans(VoidPtr HC, IrtHmgnMatType Mat)
{
    int i;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditMatTrans\n");
#endif /* DEBUG_HC_EDIT */

    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        MatMultPtby4by4(HCrvs[i].StartPos, HCrvs[i].StartPos, Mat);
	MatMultPtby4by4(HCrvs[i].EndPos, HCrvs[i].EndPos, Mat);

	MatMultVecby4by4(HCrvs[i].StartTan, HCrvs[i].StartTan, Mat);
	MatMultVecby4by4(HCrvs[i].EndTan, HCrvs[i].EndTan, Mat);

	UserHCEditUpdateOneHermite(HC, i);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Applies an affine transform, in place, to given curve HC as specified by M
* Dir and Scale.							     M
*   Curve HC is first translated by Dir and then scaled by Scale.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	      A handle on edited Hermite curve to transform, in place.       M
*   Dir:      Translation amount, in the XY plane.                           M
*   Scl:      Scaling amount, in the XY plane.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, UserHCEditInit, UserHCEditMatTrans                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditTransform                                                      M
*****************************************************************************/
int UserHCEditTransform(VoidPtr HC, CagdRType *Dir, CagdRType Scl)
{
    int i;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditTransform Dir = (%f, %f), Scl = %f\n",
	   Dir[0], Dir[1], Scl);
#endif /* DEBUG_HC_EDIT */

    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        IRIT_PT2D_ADD(HCrvs[i].StartPos, HCrvs[i].StartPos, Dir);
        IRIT_PT2D_SCALE(HCrvs[i].StartPos, Scl);
	IRIT_PT2D_ADD(HCrvs[i].EndPos, HCrvs[i].EndPos, Dir);
        IRIT_PT2D_SCALE(HCrvs[i].EndPos, Scl);

	IRIT_VEC2D_SCALE(HCrvs[i].StartTan, Scl);
	IRIT_VEC2D_SCALE(HCrvs[i].EndTan, Scl);

	UserHCEditUpdateOneHermite(HC, i);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Applies a relative translation, in place, to given curve HC as specified M
* by Dir.  The full translation amount is applied to the last point while    M
* the first is kept stationary and all interior control points are moved     M
* their relative portion.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	    A handle on edited Hermite curve to relatively transform,	     M
*	    in place.							     M
*   Dir:    Relative translation amount, applied in full just to last	     M
*	    control point.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditRelativeTranslate                                              M
*****************************************************************************/
int UserHCEditRelativeTranslate(VoidPtr HC, CagdRType *Dir)
{
    int i;
    CagdRType d1, d2, Scl;
    CagdVType RelDir;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrvs = HCE -> HCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditRelativeTranslate Dir = (%f, %f)\n", Dir[0], Dir[1]);
#endif /* DEBUG_HC_EDIT */

    for (i = 0; i < HCE -> NumOfHCrvs; i++) {
        IRIT_VEC2D_COPY(RelDir, Dir);
	IRIT_VEC2D_SCALE(RelDir, (i + 1.0) / HCE -> NumOfHCrvs);

	d1 = IRIT_PT2D_DIST_SQR(HCrvs[i].StartPos, HCrvs[i].EndPos);

        IRIT_PT2D_ADD(HCrvs[i].EndPos, HCrvs[i].EndPos, RelDir);
	if (i < HCE -> NumOfHCrvs - 1)
	    IRIT_PT2D_ADD(HCrvs[i + 1].StartPos, HCrvs[i + 1].StartPos, RelDir);

	d2 = IRIT_PT2D_DIST_SQR(HCrvs[i].StartPos, HCrvs[i].EndPos);

	/* sqrt(d2/d1) hints on relative scale that this segment underwent. */
	/* Apply this scaling factor to its tangents.			    */
	Scl = sqrt(d2 / d1);
	IRIT_VEC2D_SCALE(HCrvs[i].StartTan, Scl);
	IRIT_VEC2D_SCALE(HCrvs[i].EndTan, Scl);

	UserHCEditUpdateOneHermite(HC, i);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to update the tangents of the given HC curve to a reasonable     M
* magnitude and direction.  Useful when tangent handles are not provided     M
* to the end user.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   HC:	    A handle on edited Hermite curve to update its tangents.         M
*   Index:  Of the curve whose tangent is to be updated (with Index + 1).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserHCEditInit                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserHCEditEvalDefTans                                                    M
*****************************************************************************/
int UserHCEditEvalDefTans(VoidPtr HC, int Index)
{
    IrtVecType V;
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    int Index2 = (Index + 1) % HCE -> NumOfHCrvs;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditEvalDefTans %d\n", Index);
#endif /* DEBUG_HC_EDIT */

    /* Make sure we are not crossing from the end to the beginning in a non */
    /* periodic curve, and also this points in not declared C^0.            */
    if (Index2 != 0 || HCE -> IsPeriodic) {
        IRIT_VEC2D_ADD(V, HCE -> HCrvs[Index].StartPos,
		     HCE -> HCrvs[Index2].EndPos);

	IRIT_VEC2D_SCALE(V, USER_HC_IRIT_VEC_SCALE);

	IRIT_VEC2D_COPY(HCE -> HCrvs[Index].EndTan, V);
	IRIT_VEC2D_COPY(HCE -> HCrvs[Index2].StartTan, V);
    }
    else if (Index == 0) { /* Not periodic - first location. */
        IRIT_VEC2D_ADD(V, HCE -> HCrvs[Index].StartPos,
		     HCE -> HCrvs[Index].EndPos);

	IRIT_VEC2D_SCALE(V, USER_HC_IRIT_VEC_SCALE * 0.5);

	IRIT_VEC2D_COPY(HCE -> HCrvs[Index].StartTan, V);
    }
    else if (Index2 == 0) { /* Not periodic - last location. */
        IRIT_VEC2D_ADD(V, HCE -> HCrvs[Index].StartPos,
		     HCE -> HCrvs[Index].EndPos);

	IRIT_VEC2D_SCALE(V, USER_HC_IRIT_VEC_SCALE * 0.5);

	IRIT_VEC2D_COPY(HCE -> HCrvs[Index].EndTan, V);
    }

    UserHCEditUpdateOneHermite(HC, Index);
    UserHCEditUpdateOneHermite(HC, Index2);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reallocs the vector of curves to hold more curves.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   HC:	   A handle on edited Hermite curve to update its Index segment.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditReallocCrvs(VoidPtr HC)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;

    if (HCE -> MaxNumOfHCrvs <= HCE -> NumOfHCrvs) {
        HCE -> HCrvs =
	    IritRealloc(HCE -> HCrvs,
			HCE -> MaxNumOfHCrvs * sizeof(UserHCStruct),
			HCE -> MaxNumOfHCrvs * sizeof(UserHCStruct) * 2);
	HCE -> MaxNumOfHCrvs *= 2;
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Copies the curve Src to Dst.  All points are copied and not duplicated.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Src:	Source curve to copy from.	                             *
*   Dst:	Destination curve to copy to.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditCopyCrv(UserHCStruct *Dst, UserHCStruct *Src)
{
    IRIT_GEN_COPY(Dst, Src, sizeof(UserHCStruct));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the shared tangent of the Index and Index+1 curves in HC.         *
*   Assumes these tangent were already set to some value.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   HC:	   A handle on edited Hermite curve to update its Index segment.     *
*   Index: Of the curve whose tangent is to be updated (with Index + 1).     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditMakeTangentCont(VoidPtr HC, int Index)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    int Index2 = (Index + 1) % HCE -> NumOfHCrvs;
    IrtRType R;
    IrtVecType V;

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditMakeTangentCont %d\n", Index);
#endif /* DEBUG_HC_EDIT */

    /* make sure we are not crossing from the end to the beginning in a non */
    /* periodic curve, and also this points in not declared C^0.            */
    if ((Index2 != 0 || HCE -> IsPeriodic) && HCE -> HCrvs[Index].EndPosCont) {
        IRIT_VEC2D_ADD(V, HCE -> HCrvs[Index].EndTan,
		     HCE -> HCrvs[Index2].StartTan);
	if ((R = IRIT_VEC2D_LENGTH(V)) < IRIT_UEPS)
	    V[0] = 1.0;
	else
	    IRIT_VEC2D_NORMALIZE(V);

	R = IRIT_VEC2D_LENGTH(HCE -> HCrvs[Index].EndTan);
	IRIT_VEC2D_COPY(HCE -> HCrvs[Index].EndTan, V);
	if (R > IRIT_EPS)
	    IRIT_VEC2D_SCALE(HCE -> HCrvs[Index].EndTan, R)
	else
	    IRIT_VEC2D_COPY(HCE -> HCrvs[Index].EndTan, V);

	R = IRIT_VEC2D_LENGTH(HCE -> HCrvs[Index2].StartTan);
	IRIT_VEC2D_COPY(HCE -> HCrvs[Index2].StartTan, V);
	if (R > IRIT_EPS)
	    IRIT_VEC2D_SCALE(HCE -> HCrvs[Index2].StartTan, R)
        else
	    IRIT_VEC2D_COPY(HCE -> HCrvs[Index2].StartTan, V);
    }

    UserHCEditUpdateOneHermite(HC, Index);
    UserHCEditUpdateOneHermite(HC, Index2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update one Hermite segment by constructing its curve and building its    *
* polyline.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   HC:	   A handle on edited Hermite curve to update its Index segment.     *
*   Index: The segment in HC to update.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditUpdateOneHermite(VoidPtr HC, int Index)
{
    UserHCEditStruct
	*HCE = (UserHCEditStruct *) HC;
    UserHCStruct
        *HCrv = &HCE -> HCrvs[Index];

#ifdef DEBUG_HC_EDIT
    printf("UserHCEditUpdateOneHermite %d\n", Index);
#endif /* DEBUG_HC_EDIT */

    if (HCrv -> Crv != NULL)
        CagdCrvFree(HCrv -> Crv);
    HCrv -> Crv = CagdCubicHermiteCrv(HCrv -> StartPos,
				      HCrv -> EndPos,
				      HCrv -> StartTan,
				      HCrv -> EndTan);

    if (HCrv -> Pl != NULL)
        CagdPolylineFree(HCrv -> Pl);
    HCrv -> Pl = CagdCrv2Polyline(HCrv -> Crv, USER_HC_SAMPLES_PER_CRV, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the Hermite curve from the given regular cagd curve by	     *
* extracting the later's end points and tangents.                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   HCrv:   Hermite curve to update its two positions and two tangents.      *
*   Crv:    Regular (Bezier or Bspline) curve to extract its end positions   *
*	    and tangents for the benefit of HCrv.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditUpdateFromCrv(UserHCStruct *HCrv, CagdCrvStruct *Crv)
{
    CagdRType *R, TMin, TMax;
    CagdVecStruct *V;

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Update positions. */
    R = CagdCrvEval(Crv, TMin);
    CagdCoerceToE3(HCrv -> StartPos, &R, -1, Crv -> PType);
    R = CagdCrvEval(Crv, TMax);
    CagdCoerceToE3(HCrv -> EndPos, &R, -1, Crv -> PType);

    /* Update tangents. */
    V = CagdCrvTangent(Crv, TMin, FALSE);
    IRIT_VEC_COPY(HCrv -> StartTan, V -> Vec);
    V = CagdCrvTangent(Crv, TMax, FALSE);
    IRIT_VEC_COPY(HCrv -> EndTan, V -> Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates Tan to point in PrescribedTan while keeping its length.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Tan:            Tangent to update.					     *
*   PrescribedTan:  Tangent to derive the direction from.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserHCEditRedirectTangent(IrtVecType Tan, IrtVecType PrescribedTan)
{
    IrtRType
	R = IRIT_VEC2D_LENGTH(Tan);

    IRIT_VEC2D_COPY(Tan, PrescribedTan);
    IRIT_VEC2D_NORMALIZE(Tan);
    IRIT_VEC2D_SCALE(Tan, R);
}

