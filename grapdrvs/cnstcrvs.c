/*****************************************************************************
*  Generic tools of constraints during curve editing.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, June 1999.  *
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

#define MIN_PARAM_SPACE_PICK	 0.1

IRIT_GLOBAL_DATA char
    *IGCrvCnstAddEntries[] = {
	"New Positional Constraint",
	"New Tangential Constraint",
	"New Control-Point Constraint",
	NULL
    };
IRIT_GLOBAL_DATA IGCrvConstraintStruct
    *CrvCnstList = NULL;
IRIT_GLOBAL_DATA IGCrvConstraintEventType
    IGCCnstOperation = IG_CRV_CNST_EVENT_NONE;
IRIT_GLOBAL_DATA IGCrvConstraintType
    IGCCnstType = IG_CRV_CNST_NONE;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a positional constraint to the list of existing constraints.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Param:  The curves' parameter to constrain its position.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstAddTangentialConstraint, CCnstAddControlPtConstraint,               M
*   CCnstInsertConstraint, CCnstDeleteConstraint			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstAddPositionalConstraint                                             M
*****************************************************************************/
void CCnstAddPositionalConstraint(CagdRType Param)
{
    IGCrvConstraintStruct
	*NewCnst = (IGCrvConstraintStruct *)
	    IritMalloc(sizeof(IGCrvConstraintStruct));

    NewCnst -> Type = IG_CRV_CNST_POSITION;
    NewCnst -> Param = Param;

    CCnstInsertConstraint(NewCnst);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a tangential constraint to the list of existing constraints.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Param:  The curves' parameter to constrain its tangent.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstAddPositionalConstraint, CCnstAddControlPtConstraint,	             M
*   CCnstDeleteConstraint, CCnstInsertConstraint			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstAddTangentialConstraint                                             M
*****************************************************************************/
void CCnstAddTangentialConstraint(CagdRType Param)
{
    IGCrvConstraintStruct
	*NewCnst = (IGCrvConstraintStruct *)
	    IritMalloc(sizeof(IGCrvConstraintStruct));

    NewCnst -> Type = IG_CRV_CNST_TANGENT;
    NewCnst -> Param = Param;

    CCnstInsertConstraint(NewCnst);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a control point constraint to the list of existing constraints.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Index:  The curves' control point index to constrain its location.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstAddPositionalConstraint, CCnstAddTangentialConstraint,              M
*   CCnstInsertConstraint, CCnstDeleteConstraint			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstAddControlPtConstraint                                              M
*****************************************************************************/
void CCnstAddControlPtConstraint(int Index)
{
    IGCrvConstraintStruct
	*NewCnst = (IGCrvConstraintStruct *)
	    IritMalloc(sizeof(IGCrvConstraintStruct));

    NewCnst -> Type = IG_CRV_CNST_CTL_PT;
    NewCnst -> Param = Index;

    CCnstInsertConstraint(NewCnst);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Inserts one constraint into the global curve constraint list, sorted by  M
* the parameter values.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Cnst:      Constraint to insert into the global list.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstAddPositionalConstraint, CCnstAddTangentialConstraint               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstInsertConstraint                                                    M
*****************************************************************************/
void CCnstInsertConstraint(IGCrvConstraintStruct *Cnst)
{
    Cnst -> Pnext = NULL;

    if (CrvCnstList == NULL)
	CrvCnstList = Cnst;
    else if (CrvCnstList -> Param >= Cnst -> Param) {
	Cnst -> Pnext = CrvCnstList;
	CrvCnstList = Cnst;	
    }
    else {
	IGCrvConstraintStruct *TmpCnst, *PrevCnst;

	for (TmpCnst = PrevCnst = CrvCnstList;
	     TmpCnst != NULL && TmpCnst -> Param < Cnst -> Param; ) {
	    PrevCnst = TmpCnst;
	    TmpCnst = TmpCnst -> Pnext;
	}

	Cnst -> Pnext = TmpCnst;
	PrevCnst -> Pnext = Cnst;
    }

    IGCrvCnstParamUpdateWidget();
    IGRedrawViewWindow();                 /* Update the constraints drawing. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deletes a constraint from the list of existing constraints.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Index:  Index of constraint to delete, first constraint is index one.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstAddPositionalConstraint, CCnstAddTangentialConstraint               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstDeleteConstraint                                                    M
*****************************************************************************/
void CCnstDeleteConstraint(int Index)
{
    IGCrvConstraintStruct
	*DelCnst = CrvCnstList;

    if (Index == 1) {
	CrvCnstList = CrvCnstList -> Pnext;
	IritFree(DelCnst);
    }
    else if (Index > 1) {
	IGCrvConstraintStruct
	    *PrevCnst = NULL;

	for ( ; DelCnst != NULL && Index > 1; Index--) {
	    PrevCnst = DelCnst;
	    DelCnst = DelCnst -> Pnext;
	}

	if (DelCnst != NULL) {
	    PrevCnst -> Pnext = DelCnst -> Pnext;
	    IritFree(DelCnst);
	}
    }

    IGCrvCnstParamUpdateWidget();
    IGRedrawViewWindow();                 /* Update the constraints drawing. */
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
*   CCnstHandleMouse                                                         M
*****************************************************************************/
void CCnstHandleMouse(int x, int y, int Event)
{
    int Index, Rational;
    IrtRType t;
    IrtPtType Pt;
    IrtVecType Dir;

    switch (IGCCnstOperation) {
	case IG_CRV_CNST_EVENT_ADD:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		IGScreenToObject(x, y, Pt, Dir);
		switch (IGCCnstType) {
		    case IG_CRV_CNST_POSITION:
		        t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						      Pt, Dir);
			CCnstAddPositionalConstraint(t);
			break;
		    case IG_CRV_CNST_TANGENT:
			t = CEditFindClosestParameter(IGCrvEditCurrentCrv,
						      Pt, Dir);
			CCnstAddTangentialConstraint(t);
			break;
		    case IG_CRV_CNST_CTL_PT:
			Index = CEditFindClosestControlPoint(
							  IGCrvEditCurrentCrv,
							  Pt, Dir, &Rational);
			CCnstAddControlPtConstraint(Index);
			break;
		    default:
		        assert(0);
		}
		IGCrvEditGrabMouse = FALSE;
	    }
	    break;
	case IG_CRV_CNST_EVENT_DELETE:
	    if (Event == IG_CRV_EDIT_BUTTONDOWN) {
		int i,
		    MinIndex = -1;
		IrtRType
		    MinDist = IRIT_INFNTY;
		IGCrvConstraintStruct
		    *Cnst = CrvCnstList;

		IGScreenToObject(x, y, Pt, Dir);

		/* Find the closest constraint and delete it. */
		for (i = 1; Cnst != NULL; Cnst = Cnst -> Pnext, i++) {
		    IrtRType Dist, *R;
		    IrtPtType Pt2;

		    switch (Cnst -> Type) {
		        case IG_CRV_CNST_POSITION:
		        case IG_CRV_CNST_TANGENT:
			    R = CagdCrvEval(IGCrvEditCurrentCrv,
					    Cnst -> Param);
			    CagdCoerceToE3(Pt2, &R, -1,
					   IGCrvEditCurrentCrv -> PType);
			    break;
			case IG_CRV_CNST_CTL_PT:
			    CagdCoerceToE3(Pt2,
					   IGCrvEditCurrentCrv -> Points,
					   (int) Cnst -> Param,
					   IGCrvEditCurrentCrv -> PType);
			    break;
			default:
			    Pt2[0] = Pt2[1] = Pt2[2] = IRIT_INFNTY;
			    break;
		    }
		    Dist = GMDistPointLine(Pt2, Pt, Dir);
		    if (MinDist > Dist) {
			MinDist = Dist;
			MinIndex = i;
		    }
		}
		if (MinDist < MIN_PARAM_SPACE_PICK)
		    CCnstDeleteConstraint(MinIndex);

		IGCrvEditGrabMouse = FALSE;
	    }
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle non mouse events while the curve constraint editing.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:       Type of event (clear, order change, etc.).                  M
*   MenuIndex:   Index of last pop up menu event, if any.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstHandleNonMouseEvents                                                M
*****************************************************************************/
void CCnstHandleNonMouseEvents(IGCrvConstraintEventType Event, int MenuIndex)
{
    if (!IGCrvEditParam.SupportConstraints &&
	Event != IG_CRV_CNST_EVENT_DO_CONSTRAINTS) {
	IGCrvEditPlaceMessage("Activate constraints first");
	return;
    }

    switch (Event) {
	case IG_CRV_CNST_EVENT_DISMISS:
	    break;
	default:
	    CEditPushState(IGCrvEditCurrentCrv, IGCrvEditCurrentUVCrv,
			   &IGCrvEditParam);
	    break;
    }

    IGCCnstOperation = Event;

    switch (Event) {
	case IG_CRV_CNST_EVENT_DISMISS:
	    CCnstDetachConstraints();
	    break;
	case IG_CRV_CNST_EVENT_ADD:
	    if (IGCrvEditCurrentCrv == NULL) {
		IGCrvEditPlaceMessage("No curve to add constraints to");
		break;
	    }
	    switch (MenuIndex) {
		case IG_CRV_CNST_POSITION:
		    IGCCnstType = IG_CRV_CNST_POSITION;
		    IGCrvEditPlaceMessage("Pick position constraint on curve");
		    break;
		case IG_CRV_CNST_TANGENT:
		    IGCCnstType = IG_CRV_CNST_TANGENT;
		    IGCrvEditPlaceMessage("Pick tangent constraint on curve");
		    break;
		case IG_CRV_CNST_CTL_PT:
		    IGCCnstType = IG_CRV_CNST_CTL_PT;
		    IGCrvEditPlaceMessage("Pick ctl-pt const. on ctl-polygon");
		    break;
	    }
	    IGCEditOperation = IG_CRV_EDIT_EVENT_CONSTRAINTS;
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_CNST_EVENT_DELETE:
	    IGCrvEditPlaceMessage("Pick constraint on curve to delete");
	    IGCEditOperation = IG_CRV_EDIT_EVENT_CONSTRAINTS;
	    IGCrvEditGrabMouse = TRUE;
	    break;
	case IG_CRV_CNST_EVENT_SATISFY_ALL:
	    IGCrvEditParam.AbortIfCnstFailed =
		!IGCrvEditParam.AbortIfCnstFailed;
	    IGCrvCnstParamUpdateWidget();
	    break;
	case IG_CRV_CNST_EVENT_DO_CONSTRAINTS:
	    if ((IGCrvEditParam.SupportConstraints =
		                 !IGCrvEditParam.SupportConstraints) != FALSE) {
		if (IGCrvEditParam.Rational) {
		    IGCrvEditParam.SupportConstraints = FALSE;
		    IGCrvEditPlaceMessage("No constraints support for rational");
		}
		else if (IGCrvEditParam.Type != CAGD_CBSPLINE_TYPE) {
		    IGCrvEditParam.SupportConstraints = FALSE;
		    IGCrvEditPlaceMessage("Constraints support for Bsplines only");
		}
		else
		    CEditConstrainedReevalAll();
	    }
	    IGCrvCnstParamUpdateWidget();
	    break;
        case IG_CRV_CNST_EVENT_X_SYMMETRY:
	    if ((IGCrvEditParam.CnstXSymmetry = !IGCrvEditParam.CnstXSymmetry) != FALSE) {
		IGCrvEditParam.CnstCSymmetry =
		    IGCrvEditParam.CnstYSymmetry = FALSE;
	        CEditConstrainedReevalAll();
	    }
	    IGCrvCnstParamUpdateWidget();
	    break;
        case IG_CRV_CNST_EVENT_Y_SYMMETRY:
	    if ((IGCrvEditParam.CnstYSymmetry = !IGCrvEditParam.CnstYSymmetry) != FALSE) {
		IGCrvEditParam.CnstCSymmetry =
		    IGCrvEditParam.CnstXSymmetry = FALSE;
	        CEditConstrainedReevalAll();
	    }
	    IGCrvCnstParamUpdateWidget();
	    break;
        case IG_CRV_CNST_EVENT_C_SYMMETRY:
	    if ((IGCrvEditParam.CnstCSymmetry = !IGCrvEditParam.CnstCSymmetry) != FALSE) {
		IGCrvEditParam.CnstXSymmetry =
		    IGCrvEditParam.CnstYSymmetry = FALSE;
	        CEditConstrainedReevalAll();
	    }
	    IGCrvCnstParamUpdateWidget();
	    break;
        case IG_CRV_CNST_EVENT_AREA:
	    if ((IGCrvEditParam.CnstArea = !IGCrvEditParam.CnstArea) &&
		IGCrvEditCurrentCrv != NULL) {
		char Str[100];
		CagdRType t1, t2, *R;
		CagdCrvStruct *Crv, *TCrv;

		if (CAGD_IS_BSPLINE_CRV(IGCrvEditCurrentCrv)) {
		    if (CAGD_IS_PERIODIC_CRV(IGCrvEditCurrentCrv))
			Crv = CagdCnvrtPeriodic2FloatCrv(IGCrvEditCurrentCrv);
		    else
			Crv = CagdCrvCopy(IGCrvEditCurrentCrv);

		    if (!BspCrvHasOpenEC(Crv)) {
			TCrv = CagdCnvrtFloat2OpenCrv(Crv);
			CagdCrvFree(Crv);
			Crv = TCrv;
		    }
		}
		else
		    Crv = CagdCrvCopy(IGCrvEditCurrentCrv);

		TCrv = SymbCrvEnclosedArea(Crv);
		CagdCrvFree(Crv);

		/* Let the user know the current area. */
		CagdCrvDomain(TCrv, &t1, &t2);
		R = CagdCrvEval(TCrv, t2);
		CagdCrvFree(TCrv);

		sprintf(Str, "Crv's Area = %f\n", IRIT_FABS(R[1]));
		IGCrvEditPlaceMessage(Str);
	    }
	    IGCrvCnstParamUpdateWidget();
	    break;
        default:
	    IGIritError("CCnstHandleNonMouseEvents Error: Invalid event type");
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the existing constraints                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CEditRedrawCrv                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstRedrawConstraints                                                   M
*****************************************************************************/
void CCnstRedrawConstraints(void)
{
    IRIT_STATIC_DATA IPObjectStruct
        *PCirc = NULL,
        *PDiam = NULL,
        *PCross = NULL;
    IGCrvConstraintStruct
	*Cnst = CrvCnstList;
    IrtHmgnMatType IritView;
    IPVertexStruct *L00, *L01, *L10, *L11;

    if (PCirc == NULL) {
        /* Prepare a unit circle in the XY plane. */
        PCirc = IPCopyObject(NULL, CEditGetUnitCircle(), FALSE);
    	AttrSetObjectRGBColor(PCirc,
			      IGGlblHighlight2Color[0],
			      IGGlblHighlight2Color[1],
			      IGGlblHighlight2Color[2]);
	AttrSetObjectIntAttrib(PCirc, "DWidth", 3);

        /* Prepare a unit diamond in the XY plane. */
        PDiam = IPCopyObject(NULL, CEditGetUnitDiamond(), FALSE);
    	AttrSetObjectRGBColor(PDiam,
			      IGGlblHighlight2Color[0],
			      IGGlblHighlight2Color[1],
			      IGGlblHighlight2Color[2]);
	AttrSetObjectIntAttrib(PDiam, "DWidth", 3);

	/* Prepare a polyline with two lines as a cross. */
	PCross = IPGenPOLYLINEObject(
	       IPAllocPolygon(0, IPAllocVertex2(IPAllocVertex2(NULL)), NULL));
	PCross -> U.Pl -> Pnext =
	       IPAllocPolygon(0, IPAllocVertex2(IPAllocVertex2(NULL)), NULL);
	
	L00 = PCross -> U.Pl -> PVertex;
	L01 = L00 -> Pnext;
	L10 = PCross -> U.Pl -> Pnext -> PVertex;
	L11 = L10 -> Pnext;

	L00 -> Coord[0] = -IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L00 -> Coord[1] = -IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L00 -> Coord[2] =  0.0;
	L01 -> Coord[0] =  IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L01 -> Coord[1] =  IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L01 -> Coord[2] =  0.0;
	L10 -> Coord[0] = -IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L10 -> Coord[1] =  IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L10 -> Coord[2] =  0.0;
	L11 -> Coord[0] =  IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L11 -> Coord[1] = -IG_CRV_RATIONAL_CIRC_LENGTH * 5.0;
	L11 -> Coord[2] =  0.0;

    	AttrSetObjectRGBColor(PCross,
			      IGGlblHighlight2Color[0],
			      IGGlblHighlight2Color[1],
			      IGGlblHighlight2Color[2]);
	AttrSetObjectIntAttrib(PCross, "DWidth", 3);
    }
    L00 = PCross -> U.Pl -> PVertex;
    L01 = L00 -> Pnext;
    L10 = PCross -> U.Pl -> Pnext -> PVertex;
    L11 = L00 -> Pnext;

    switch (IGGlblViewMode) {
        case IG_VIEW_ORTHOGRAPHIC:
            IRIT_GEN_COPY(IritView, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IritView, IPViewMat, IPPrspMat);
	    break;
    }

    for ( ; Cnst != NULL; Cnst = Cnst -> Pnext) {
        CagdRType *R;
        CagdPType Pt;
	CagdVType Tan, Nrml;
	CagdVecStruct *Vec;
	IrtHmgnMatType TransMat, ScaleMat, RotMat;

	if (Cnst -> Type != IG_CRV_CNST_CTL_PT) {
	    if (IGCrvEditCurrentUVCrv == NULL) {
		R = CagdCrvEval(IGCrvEditCurrentCrv, Cnst -> Param);
		CagdCoerceToE3(Pt, &R, -1, IGCrvEditCurrentCrv -> PType);
	    }
	    else {
		R = CagdCrvEval(IGCrvEditCurrentUVCrv, Cnst -> Param);
		R = CagdSrfEval(IGCrvEditSktchSrf, R[1], R[2]);
		CagdCoerceToE3(Pt, &R, -1, IGCrvEditSktchSrf -> PType);
	    }

	    Vec = CagdCrvTangent(IGCrvEditCurrentCrv, Cnst -> Param, TRUE);
	    IRIT_VEC_COPY(Tan, Vec -> Vec);
	    if ((Vec = CagdCrvNormal(IGCrvEditCurrentCrv, Cnst -> Param, 
				     TRUE)) == NULL) {
	        Nrml[0] = Tan[1];
	        Nrml[1] = -Tan[0];
	        Nrml[2] = 0.0;
	    }
	    else
	        IRIT_VEC_COPY(Nrml, Vec -> Vec);
	}

	switch (Cnst -> Type) {
	    case IG_CRV_CNST_POSITION:
	        /* Move the circle to the constraint's position. */
	        MatGenMatTrans(Pt[0], Pt[1], Pt[2], TransMat);
		MatMultTwo4by4(TransMat, TransMat, IritView);

		/* Apply the proper scale, following the weight's size. */
		MatGenMatUnifScale(IG_CRV_RATIONAL_CIRC_LENGTH, ScaleMat);
		MatMultTwo4by4(TransMat, ScaleMat, TransMat);

		/* Apply the inverse rotation to the transformation. */
		GMGenMatrixZ2Dir(RotMat, Tan);
		MatMultTwo4by4(TransMat, RotMat, TransMat);

		IGViewObject(PCirc, TransMat);
		IGViewObject(PCross, TransMat);
		break;
	    case IG_CRV_CNST_TANGENT:
	        /* Move the circle to the constraint's position. */
	        MatGenMatTrans(Pt[0], Pt[1], Pt[2], TransMat);
		MatMultTwo4by4(TransMat, TransMat, IritView);

		/* Apply the proper scale, following the weight's size. */
		MatGenMatUnifScale(IG_CRV_RATIONAL_CIRC_LENGTH, ScaleMat);
		MatMultTwo4by4(TransMat, ScaleMat, TransMat);

		/* Apply the inverse rotation to the transformation. */
		GMGenMatrixZ2Dir(RotMat, Nrml);
		MatMultTwo4by4(TransMat, RotMat, TransMat);

		IGViewObject(PDiam, TransMat);
		IGViewObject(PCross, TransMat);
	        break;
	    case IG_CRV_CNST_CTL_PT:
		if (IGCrvEditDrawMesh) {
		    CagdCoerceToE3(Pt, IGCrvEditCurrentCrv -> Points,
				   (int) Cnst -> Param,
				   IGCrvEditCurrentCrv -> PType);

		    /* Move the cross to the control point's position. */
		    MatGenMatTrans(Pt[0], Pt[1], Pt[2], TransMat);
		    MatMultTwo4by4(TransMat, TransMat, IritView);

		    /* Apply the proper scale, following the weight's size. */
		    MatGenMatUnifScale(IG_CRV_RATIONAL_CIRC_LENGTH, ScaleMat);
		    MatMultTwo4by4(TransMat, ScaleMat, TransMat);

		    IGViewObject(PCross, TransMat);
		}
		break;
            default:
	        assert(0);
	}
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCrvPrintConstraint, FALSE) {
	    int i;
	    char *p;

	    Cnst = CrvCnstList;

	    IRIT_INFO_MSG("----------------\n");
	    for (i = 1; Cnst != NULL; Cnst = Cnst -> Pnext, i++) {
	        switch (Cnst -> Type) {
	            case IG_CRV_CNST_TANGENT:
		        p = "Tangential";
			break;
		    case IG_CRV_CNST_POSITION:
		        p = "Positional";
			break;
		    case IG_CRV_CNST_CTL_PT:
		        p = "Control Point";
			break;
		    default:
		        p = "??????????";
			break;
		}
		IRIT_INFO_MSG_PRINTF("%d) %s Constraints at %.5f\n",
			             i, p, Cnst -> Param);
	    }
	}
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Solve for possible weights that can satisfy all constraints in this      M
* range of knot vector KV, so that we interpolate either a value of one at   M
* curve's parameter t, coerce point index CtlPtIndex, or nothings, depends   M
* on the value of AdditionalConst.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TransDir: Vectoric change requested.                                     M
*   ParamCrv: Defining the space of functions to work in.                    M
*   KvsInfo:  KV function space information.				     M
*   t:        Parameter value at which to interpolate a value of one, if     M
*	      AdditionalConst is IG_CRV_CNST_POSITION.			     M
*   CtlPtIndex:   Index of control point to interpolate if AdditionalConst   M
*		  is IG_CRV_CNST_CTL_PT					     M
*   AdditionalConst:   Type of addition contsaint to use, if any.	     M
*   SatisfyAll:   Returned TRUE if all constraints were satisified.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CCnstSolutionWeightsStruct *:   The evaluated weights.  Values of zeros  M
*		 are returned if unable to satisfy the constraints.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpolate                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstSolveConstraints                                                    M
*****************************************************************************/
CCnstSolutionWeightsStruct *CCnstSolveConstraints(IrtVecType TransDir,
						  CagdCrvStruct *ParamCrv,
						  CEditMultiResOneKvStruct
						      *KvsInfo,
						  CagdRType t,
						  int CtlPtIndex,
						  IGCrvConstraintType
						      AdditionalConst,
						  CagdBType *SatisfyAll)
{
    IRIT_STATIC_DATA int
	ParamsLength = 0,
	WeightsLength = 0;
    IRIT_STATIC_DATA CagdRType Val,
	*Params = NULL;
    IRIT_STATIC_DATA CCnstSolutionWeightsStruct
        Weights = { { NULL, NULL, NULL }, 0, 0 };
    CagdBType
	Periodic = ParamCrv -> Periodic;
    int i, j, Idx1, Idx2, Len1,
	Order = ParamCrv -> Order,
	Length = ParamCrv -> Length,
	NumPts = 0;
    CagdPointType
	PType = ParamCrv -> PType;
    CagdRType *R, MaxCoef,
	*KV = ParamCrv -> KnotVector,
	**Points = ParamCrv -> Points;
    CagdCrvStruct *Crv, *DCrv;
    CagdCtlPtStruct *Pt,
	*PtList = NULL;
    IGCrvConstraintStruct *Cnst;

    if (WeightsLength <= Length) {
	if (Weights.W[0] != NULL)
	    for (i = 0; i < 3; i++)
		IritFree(Weights.W[i]);

	WeightsLength = Length * 2;
	for (i = 0; i < 3; i++)
	    Weights.W[i] =
		(CagdRType *) IritMalloc(sizeof(CagdRType) * WeightsLength);
    }
    if (ParamsLength <= CagdListLength(CrvCnstList) + 1) {	
	if (Params != NULL)
	    IritFree(Params);

	ParamsLength = (CagdListLength(CrvCnstList) + 1) * 2;
	Params = (CagdRType *) IritMalloc(sizeof(CagdRType) * ParamsLength);
    }

    for (Cnst = CrvCnstList; Cnst != NULL; Cnst = Cnst -> Pnext) {
	Pt = CagdCtlPtNew(PType);
	IRIT_LIST_PUSH(Pt, PtList);
	Params[NumPts++] = Cnst -> Param;
	for (i = 1; i <= CAGD_NUM_OF_PT_COORD(PType); i++)
	    Pt -> Coords[i] = 0.0;
	switch (Cnst -> Type) {
	    case IG_CRV_CNST_TANGENT:
		AttrSetIntAttrib(&Pt -> Attr, "Derivative", 1);
		break;
	    case IG_CRV_CNST_CTL_PT:
		AttrSetIntAttrib(&Pt -> Attr, "CtlPt", (int) Cnst -> Param);
		break;
            default:
		break;
	}
    }

    /* Add symmetry constraints if any. */
    if (IGCrvEditParam.CnstXSymmetry ||
	IGCrvEditParam.CnstYSymmetry ||
	IGCrvEditParam.CnstCSymmetry) {
	Pt = CagdCtlPtNew(PType);
	IRIT_LIST_PUSH(Pt, PtList);
	for (i = 1; i <= CAGD_NUM_OF_PT_COORD(PType); i++)
	    Pt -> Coords[i] = TransDir[i - 1];
	AttrSetPtrAttrib(&Pt -> Attr, "SymState", Points);
	AttrSetIntAttrib(&Pt -> Attr, "Symmetry",
	     (IGCrvEditParam.CnstXSymmetry ? CAGD_CONST_X_SYMMETRY : 0) +
	     (IGCrvEditParam.CnstYSymmetry ? CAGD_CONST_Y_SYMMETRY : 0) +
	     (IGCrvEditParam.CnstCSymmetry ? CAGD_CONST_C_SYMMETRY : 0));
	Params[NumPts++] = 0.0;
    }

    /* Add Area constraint if any. */
    if (IGCrvEditParam.CnstArea) {
	IRIT_STATIC_DATA int
	    XYToggle = FALSE;

	Pt = CagdCtlPtNew(PType);
	IRIT_LIST_PUSH(Pt, PtList);
	IRIT_PT_RESET(Pt -> Coords);
	AttrSetPtrAttrib(&Pt -> Attr, "AreaInnerProds", KvsInfo -> InnerProds);
	AttrSetPtrAttrib(&Pt -> Attr, "AreaPts", Points);
	AttrSetIntAttrib(&Pt -> Attr, "Area",
	     (XYToggle ? CAGD_CONST_X_AREA : CAGD_CONST_Y_AREA));
	XYToggle = !XYToggle;
	Params[NumPts++] = 0.0;
    }

    /* Add last constraint for the curve: */
    switch (AdditionalConst) {
	case IG_CRV_CNST_CTL_PT:
	    /* Make the curve's CtlPtIndex control point be prescribed. */
	    Pt = CagdCtlPtNew(PType);
	    IRIT_LIST_PUSH(Pt, PtList);
	    for (i = 1; i <= CAGD_NUM_OF_PT_COORD(PType); i++)
		Pt -> Coords[i] = TransDir[i - 1];
	    AttrSetIntAttrib(&Pt -> Attr, "CtlPt", CtlPtIndex);
	    Params[NumPts++] = t;
	    break;
	case IG_CRV_CNST_POSITION:
	    /* Make the curve be equal to one at the t value. */
	    Pt = CagdCtlPtNew(PType);
	    IRIT_LIST_PUSH(Pt, PtList);
	    for (i = 1; i <= CAGD_NUM_OF_PT_COORD(PType); i++)
		Pt -> Coords[i] = TransDir[i - 1];
	    Params[NumPts++] = t;
	    break;
	default:
	  break;
    }
    PtList = CagdListReverse(PtList);

    if (!IGCrvEditParam.SupportConstraints ||
	PtList == NULL ||
	(Crv = BspCrvInterpolate(PtList, Params, KV,
				 Length, Order, Periodic)) == NULL) {
	CagdCtlPtFreeList(PtList);

	/* All weights are zero. */
	Weights.IndexFirst = Weights.DataSize = 0;
	for (i = 0; i < 3; i++)
	    Weights.W[i][0] = 0.0;
	return &Weights;
    }
    CagdCtlPtFreeList(PtList);

    /* Lets see if we are successful in satisfying all constraints. */
    *SatisfyAll = TRUE;
    DCrv = CagdCrvDerive(Crv);
    for (Cnst = CrvCnstList; Cnst != NULL; Cnst = Cnst -> Pnext) {
	switch (Cnst -> Type) {
	    case IG_CRV_CNST_POSITION:
		R = CagdCrvEval(Crv, Cnst -> Param);
		if (!IRIT_APX_EQ(R[1], 0.0))
		    *SatisfyAll = FALSE;
		break;
	    case IG_CRV_CNST_TANGENT:
		R = CagdCrvEval(DCrv, Cnst -> Param);
		if (!IRIT_APX_EQ(R[1], 0.0))
		    *SatisfyAll = FALSE;
		break;
            default:
		break;
	}
    }
    CagdCrvFree(DCrv);

    Len1 = Crv -> Length - 1;
    if (IGCrvEditParam.CnstXSymmetry) {
	Val = Crv -> Points[2][0] + Crv -> Points[2][Len1];
	for (i = 0; i < (Crv -> Length >> 1); i++)
	    if (!IRIT_APX_EQ(Crv -> Points[1][i],
			Crv -> Points[1][Len1 - i]) ||
		!IRIT_APX_EQ(Crv -> Points[2][i] +
			Crv -> Points[2][Len1 - i], Val))
		*SatisfyAll = FALSE;
    }
    if (IGCrvEditParam.CnstYSymmetry) {
	Val = Crv -> Points[1][0] + Crv -> Points[1][Len1];
	for (i = 0; i < (Crv -> Length >> 1); i++)
	    if (!IRIT_APX_EQ(Crv -> Points[2][i],
			Crv -> Points[2][Len1 - i]) ||
		!IRIT_APX_EQ(Crv -> Points[1][i] +
			Crv -> Points[1][Len1 - i], Val))
		*SatisfyAll = FALSE;
    }
    if (IGCrvEditParam.CnstCSymmetry) {
	Val = Crv -> Points[1][0] + Crv -> Points[1][Len1];
	for (i = 0; i < (Crv -> Length >> 1); i++)
	    if (!IRIT_APX_EQ(Crv -> Points[1][i] +
			Crv -> Points[1][Len1 - i], Val))
		*SatisfyAll = FALSE;
    }

    for (R = Crv -> Points[1], MaxCoef = 0, i = 0; i < Crv -> Length; i++)
	if (MaxCoef < IRIT_FABS(R[i]))
	    MaxCoef = IRIT_FABS(R[i]);
    if (MaxCoef > IGCrvEditParam.CnstMaxAllowedCoef)
	*SatisfyAll = FALSE;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugSolveConst, FALSE) {
	    IRIT_INFO_MSG_PRINTF(
		    "Max coef = %f, SatisfyAll = %d, Crv Length = %d\n",
		    MaxCoef, *SatisfyAll, Crv -> Length);
	}
    }
#   endif /* DEBUG */

    /* Figure out the first and last non zero location and copy only that. */
    for (j = 0, Idx1 = Length; j < Length && Idx1 == Length; j++) {
	for (i = 1; i <= CAGD_NUM_OF_PT_COORD(Crv -> PType); i++) {
	    if (Crv -> Points[i][j] != 0.0) {
		Idx1 = j;
		break;
	    }
	}
    }

    if (Idx1 >= Length) {
	/* All weights are zero. */
	Weights.IndexFirst = Weights.DataSize = 0;
	for (i = 0; i < 3; i++)
	    Weights.W[i][0] = 0.0;
    }
    else {
	for (j = Length - 1, Idx2 = -1; j >= 0 && Idx2 == -1; j--) {
	    for (i = 1; i <= CAGD_NUM_OF_PT_COORD(Crv -> PType); i++) {
		if (Crv -> Points[i][j] != 0.0) {
		    Idx2 = j;
		    break;
		}
	    }
	}

	Weights.IndexFirst = Idx1;
	Weights.DataSize = Idx2 - Idx1 + 1;

	for (i = 1; i <= CAGD_NUM_OF_PT_COORD(Crv -> PType); i++)
	    CAGD_GEN_COPY(Weights.W[i - 1],
			  &Crv -> Points[i][Weights.IndexFirst],
			  sizeof(CagdRType) * Weights.DataSize);
    }

    CagdCrvFree(Crv);

    return &Weights;
}

/*****************************************************************************
* DESCRIPTION:							             M
*   Converts the current constraints to a string listing the constraints.    M
*                                                                            *
* PARAMETERS:                                                                M
*   StrCnstraint:     A string defining the current constraints.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstConstraints2Str                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstStr2Constraints                                                     M
*****************************************************************************/
void CCnstStr2Constraints(const char *StrCnstraint)
{
    char *p, *Str;
    IGCrvConstraintStruct
	*CnstTail = NULL;

    if (StrCnstraint == NULL)
	return;

    /* Free old constraints, if were any. */
    while (CrvCnstList != NULL) {
	IGCrvConstraintStruct
	    *DelCnst = CrvCnstList;

	CrvCnstList = CrvCnstList -> Pnext;
	IritFree(DelCnst);
    }
    IGCrvEditParam.CnstXSymmetry =
        IGCrvEditParam.CnstYSymmetry =
	    IGCrvEditParam.CnstCSymmetry = FALSE;

    /* Make a copy of the string as strtok is destructive */
    Str = IritStrdup(StrCnstraint);
    p = strtok(Str, " ");
    do {
        int AddConstraint = TRUE;
        IGCrvConstraintType
	    Type = IG_CRV_CNST_NONE;

	switch (p[0]) {
	    case 'P':
	        Type = IG_CRV_CNST_POSITION;
		p = &p[1];
	        break;
	    case 'T':
		Type = IG_CRV_CNST_TANGENT;
		p = &p[1];
	        break;
	    case 'C':
		Type = IG_CRV_CNST_CTL_PT;
		p = &p[1];
	        break;
	    case 'S':
		AddConstraint = FALSE;
	        switch (p[1]) {
		    case 'X':
			IGCrvEditParam.CnstXSymmetry = TRUE;
			break;
		    case 'Y':
			IGCrvEditParam.CnstYSymmetry = TRUE;
			break;
		    case 'C':
			IGCrvEditParam.CnstCSymmetry = TRUE;
			break;
		}
	        break;
	    case 'A':
		AddConstraint = FALSE;
		IGCrvEditParam.CnstArea = TRUE;
	        break;
	}

	if (AddConstraint) {
	    IGCrvConstraintStruct
	        *Cnst = (IGCrvConstraintStruct *)
				    IritMalloc(sizeof(IGCrvConstraintStruct));

	    Cnst -> Type = Type;
	    Cnst -> Pnext = NULL;
	    sscanf(p, "%lf", &Cnst -> Param);

	    if (CrvCnstList == NULL)
	        CrvCnstList = CnstTail = Cnst;
	    else {
	        CnstTail -> Pnext = Cnst;
		CnstTail = Cnst;
	    }
	}
    }
    while ((p = strtok(NULL, " ")) != NULL);

    IritFree(Str);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the current constraints to a string listing the constraints.    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:     A string defining the current constraints, NULL if none.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CCnstStr2Constraints                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstConstraints2Str                                                     M
*****************************************************************************/
char *CCnstConstraints2Str(void)
{
    char *Str;
    int NumCnst = 4;			     /* Three symmetry + one area... */
    IGCrvConstraintStruct *Cnst;

    for (Cnst = CrvCnstList;
	 Cnst != NULL;
	 Cnst = Cnst -> Pnext)
        NumCnst++;

    Str = (char *) IritMalloc(NumCnst * 20);
    Str[0] = 0;
    for (Cnst = CrvCnstList;
	 Cnst != NULL;
	 Cnst = Cnst -> Pnext) {
	switch (Cnst -> Type) {
	    case IG_CRV_CNST_POSITION:
	        sprintf(&Str[strlen(Str)], "P%-15.13lg ", Cnst -> Param);
		break;
	    case IG_CRV_CNST_TANGENT:
	        sprintf(&Str[strlen(Str)], "T%-15.13lg ", Cnst -> Param);
		break;
	    case IG_CRV_CNST_CTL_PT:
	        sprintf(&Str[strlen(Str)], "C%-15.13lg ", Cnst -> Param);
		break;
	    case IG_CRV_CNST_X_SYMMETRY:
	        sprintf(&Str[strlen(Str)], "SX ");
		break;
	    case IG_CRV_CNST_Y_SYMMETRY:
	        sprintf(&Str[strlen(Str)], "SY ");
		break;
	    case IG_CRV_CNST_C_SYMMETRY:
	        sprintf(&Str[strlen(Str)], "SC ");
		break;
	    case IG_CRV_CNST_AREA:
	        sprintf(&Str[strlen(Str)], "A ");
		break;
            default:
		break;
	}
    }

    if (IGCrvEditParam.CnstXSymmetry)
        sprintf(&Str[strlen(Str)], "SX ");

    if (IGCrvEditParam.CnstYSymmetry)
        sprintf(&Str[strlen(Str)], "SY ");

    if (IGCrvEditParam.CnstCSymmetry)
        sprintf(&Str[strlen(Str)], "SC ");

    if (IGCrvEditParam.CnstArea)
        sprintf(&Str[strlen(Str)], "A ");

    if (IRT_STR_ZERO_LEN(Str)) {
	IritFree(Str);
	return NULL;
    }
    else
	return Str;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Disable currently editted constraints of curve, temporarily.             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstDisableConstraints                                                  M
*****************************************************************************/
void CCnstDisableConstraints(void)
{
    IGCrvEditParam.SupportConstraints = FALSE;
    IGCrvCnstParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently editted constraints of curve.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CCnstDetachConstraints                                                   M
*****************************************************************************/
void CCnstDetachConstraints(void)
{
    while (CrvCnstList != NULL) {
	IGCrvConstraintStruct
	    *DelCnst = CrvCnstList;

	CrvCnstList = CrvCnstList -> Pnext;
	IritFree(DelCnst);
    }

    CCnstDisableConstraints();
}
