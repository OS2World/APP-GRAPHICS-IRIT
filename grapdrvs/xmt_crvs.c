/*****************************************************************************
*   A motif interface for curve editing.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, January 1998.  *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "xmtdrvs.h"
#include "editcrvs.h"
#include "cnstcrvs.h"

#define CRVEDIT_PARAM_FRACTION		12
#define CRVCNST_PARAM_FRACTION		6

IRIT_GLOBAL_DATA Widget CEditStatePopup, CEditParamTypePopup,
    CEditEndCondPopup, CEditSubdivPopup, CEditPrimitivesPopup,
    CEditRefinePopup, CCnstAddPopup, CEditEvaluatePopup,

    CrvEditForm, CEditSlideForm, CEditMessageForm, CEditOrderForm,
    CEditCrvNameForm, CEditLstSqrForm, CrvConstraintForm, CCnstUndulateForm,

    CEditRationalButton, CEditEndCondButton, CEditCrvTypeButton,
    CEditCrvParamButton, CEditDrawMeshButton, CEditDrawOriginalButton,
    CEditStateButton, CCnstSatisfyAllButton, CCnstDoConstraintsButton,
    CCnstXSymmetryButton, CCnstYSymmetryButton, CCnstCSymmetryButton,
    CCnstAreaButton,

    CCnstConstraintText;

static void CrvEditCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void CrvCnstCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void ActivatePopupMenu(Widget Parent,
			      Widget *Popup,
			      char **StrEntries,
			      char *Title,
			      XTC CallBackFunc,
			      int RelSelectIndex,
			      int RelX,
			      int RelY);
static void SaveCurveCB(Widget w,
			caddr_t ClientData,
			XmFileSelectionBoxCallbackStruct *FileStruct);
static void CrvEditStateMenuCB(Widget w,
			       int State,
			       XmScaleCallbackStruct *CallData);
static void CrvCnstAddMenuCB(Widget w,
			     int State,
			     XmScaleCallbackStruct *CallData);
static void CrvEditPrimitivesMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData);
static void CrvEditParamTypeMenuCB(Widget w,
				   int State,
				   XmScaleCallbackStruct *CallData);
static void CrvEditEndCondMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData);
static void CrvEditEvalEntityMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData);
static void CrvEditSubdivMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData);
static void CrvEditRefineMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData);
static void CrvEditDragCB(Widget w,
			  int State,
			  XmScaleCallbackStruct *CallData);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main CrvEditParam window			   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateCrvEdit                                                            M
*****************************************************************************/
void CreateCrvEdit(Widget IGTopLevel)
{
    int Pos = 0;
    Arg args[10];
    Widget Form, SubForm, Button;

    XtSetArg(args[0], XmNfractionBase, CRVEDIT_PARAM_FRACTION);
    CrvEditForm = XmCreateFormDialog(IGTopLevel, "CrvEditParamMenu", args, 1);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    CEditStateButton = XtVaCreateManagedWidget("Detach Current Curve",
				     xmPushButtonWidgetClass, Form, 
				     XmNleftAttachment,       XmATTACH_FORM,
				     XmNrightAttachment,      XmATTACH_FORM,
				     NULL);
    XtAddCallback(CEditStateButton, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_STATE);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Curve Name",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    XtVaCreateManagedWidget("Name:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   3,
			    NULL);
    CEditCrvNameForm = XtVaCreateManagedWidget("Curve Name Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     3,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    10,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		7,
				   NULL);
    XtAddCallback(CEditCrvNameForm, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_CURVE_NAME);
    CEditDrawMeshButton = XtVaCreateManagedWidget("Mesh",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           10,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          13,
				  NULL);
    XtAddCallback(CEditDrawMeshButton, XmNvalueChangedCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_DRAW_MESH);
    CEditDrawOriginalButton = XtVaCreateManagedWidget("Orig",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           13,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          16,
				  NULL);
    XtAddCallback(CEditDrawOriginalButton, XmNvalueChangedCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_DRAW_ORIG);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("CurveGeneric",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    XtVaCreateManagedWidget("Order:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   2,
			    NULL);
    CEditOrderForm = XtVaCreateManagedWidget("Order Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     2,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    4,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		2,
				   NULL);
    XtAddCallback(CEditOrderForm, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_ORDER);
    CEditEndCondButton = XtVaCreateManagedWidget("Open EC",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        5,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       10,
				     NULL);
    XtAddCallback(CEditEndCondButton, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_END_COND);
    CEditRationalButton = XtVaCreateManagedWidget("Rational",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        11,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       16,
				     NULL);
    XtAddCallback(CEditRationalButton, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_RATIONAL);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("LeastSquares",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    XtVaCreateManagedWidget("LstSqr %:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   3,
			    NULL);
    CEditLstSqrForm = XtVaCreateManagedWidget("LstSqr Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     3,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    5,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		3,
				   NULL);
    XtAddCallback(CEditLstSqrForm, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_LSTSQR_PERCENT);
    CEditCrvParamButton = XtVaCreateManagedWidget("General KV",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        6,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       11,
				     NULL);
    XtAddCallback(CEditCrvParamButton, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_PARAM_TYPE);
    CEditCrvTypeButton = XtVaCreateManagedWidget("Bezier",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        11,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       16,
				     NULL);
    XtAddCallback(CEditCrvTypeButton, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_CURVE_TYPE);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations one",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    Form = XtVaCreateManagedWidget("Subdivision",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_SUBDIV);
    Form = XtVaCreateManagedWidget("Refinement",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_REFINE);
    Form = XtVaCreateManagedWidget("Sub Region",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        8,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       12,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_REGION);
    Form = XtVaCreateManagedWidget("Crv Merge",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_MERGE_CURVES);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations two",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    Form = XtVaCreateManagedWidget("Move CtlPts",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       4,
				     NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_MOVE_CTLPTS);
    Form = XtVaCreateManagedWidget("Delete CtlPts",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        4,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       8,
				     NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_DELETE_CTLPTS);
    Form = XtVaCreateManagedWidget("Modify Curve",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        8,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       12,
				     NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_MODIFY_CURVE);
    Form = XtVaCreateManagedWidget("Degree Raise",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_DRAISE);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations three",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    Form = XtVaCreateManagedWidget("Reverse",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_REVERSE);
    Form = XtVaCreateManagedWidget("Evaluate",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_EVALUATE);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations four",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    Form = XtVaCreateManagedWidget("Undo",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_UNDO);
    Form = XtVaCreateManagedWidget("Redo",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_REDO);
    Form = XtVaCreateManagedWidget("Save",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        8,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       12,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_SAVE_CURVE);
    Form = XtVaCreateManagedWidget("Submit",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_SUBMIT_CURVE);

    AddLabel(CrvEditForm, "Multi Resolution Control", Pos++);
    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Multi Resolution",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	10,
				      NULL);
    XtVaCreateManagedWidget("MR",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   1,
			    NULL);
    CEditSlideForm = XtVaCreateManagedWidget("MRSlide",
			xmScaleWidgetClass, 	SubForm,
			XmNorientation,		XmHORIZONTAL,
			XmNminimum,  		0,
			XmNmaximum,		1000,
			XmNleftAttachment,   	XmATTACH_POSITION,
		        XmNleftPosition,        1,
			XmNrightAttachment, 	XmATTACH_POSITION,
			XmNrightPosition,       10,
			NULL);

    XtAddCallback(CEditSlideForm, XmNvalueChangedCallback, (XTC) CrvEditDragCB,
		  (XTP) IG_CRV_EDIT_EVENT_MR_SLIDE);
    XtAddCallback(CEditSlideForm, XmNdragCallback, (XTC) CrvEditDragCB,
		  (XTP) IG_CRV_EDIT_EVENT_MR_SLIDE);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    CEditMessageForm = XtVaCreateManagedWidget("Messages:",
					  xmLabelWidgetClass,	Form,
					  XmNleftAttachment,    XmATTACH_FORM,
					  XmNrightAttachment,   XmATTACH_FORM,
					  NULL);

    Form = CreateSubForm(CrvEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("FinishUp",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	3,
				      NULL);

    Button = XtVaCreateManagedWidget("Constraints",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_CONSTRAINTS);
 
    Button = XtVaCreateManagedWidget("Clear",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_CLEAR);
 
    Button = XtVaCreateManagedWidget("Dismiss",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        2,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       3,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvEditCB, (XTP) IG_CRV_EDIT_EVENT_DISMISS);

    if (CRVEDIT_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of CrvEdit Param State is incomplete (%d).\n",
		Pos);

    /* The curve constraints form starts here... */
    Pos = 0;
    XtSetArg(args[0], XmNfractionBase, CRVCNST_PARAM_FRACTION);
    CrvConstraintForm = XmCreateFormDialog(CrvEditForm,
					   "CrvConstraintParamMenu",
					   args, 1);

    /* Create a Text widget and attach it to the form. */
    Form = XtVaCreateManagedWidget("Form",
				   xmFormWidgetClass, 	CrvConstraintForm, 
				   XmNtopAttachment,	XmATTACH_POSITION,
				   XmNtopPosition,	Pos,
				   XmNbottomAttachment, XmATTACH_POSITION,
				   XmNbottomPosition,	Pos + 3,
				   XmNleftAttachment, 	XmATTACH_FORM,
				   XmNrightAttachment,  XmATTACH_FORM,
				   XmNresizable, 	FALSE,
				   NULL);
    Pos += 3;

    CCnstConstraintText = XtVaCreateManagedWidget("Constraint Text",
				   xmTextWidgetClass,   Form, 
				   XmNleftAttachment,   XmATTACH_FORM,
				   XmNrightAttachment,  XmATTACH_FORM,
				   XmNeditMode,	        XmMULTI_LINE_EDIT,
				   XmNrows,		5,
				   NULL);

    Form = CreateSubForm(CrvConstraintForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Constraints Control",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	4,
				      NULL);
    CCnstSatisfyAllButton = XtVaCreateManagedWidget("Satisfy All",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           0,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          1,
				  NULL);
    XtAddCallback(CCnstSatisfyAllButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_SATISFY_ALL);
    CCnstDoConstraintsButton = XtVaCreateManagedWidget("Constraints",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           1,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          2,
				  NULL);
    XtAddCallback(CCnstDoConstraintsButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_DO_CONSTRAINTS);
    XtVaCreateManagedWidget("Undulate:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    2,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   3,
			    NULL);
    CCnstUndulateForm = XtVaCreateManagedWidget("Undulate Factor",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     3,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    4,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		5,
				   NULL);
    XtAddCallback(CCnstUndulateForm, XmNactivateCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_UNDULATE);

    Form = CreateSubForm(CrvConstraintForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Global Constraints",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	4,
				      NULL);
    CCnstXSymmetryButton = XtVaCreateManagedWidget("X Smtry",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           0,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          1,
				  NULL);
    XtAddCallback(CCnstXSymmetryButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_X_SYMMETRY);
    CCnstYSymmetryButton = XtVaCreateManagedWidget("Y Smtry",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           1,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          2,
				  NULL);
    XtAddCallback(CCnstYSymmetryButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_Y_SYMMETRY);
    CCnstCSymmetryButton = XtVaCreateManagedWidget("C Smtry",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           2,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          3,
				  NULL);
    XtAddCallback(CCnstCSymmetryButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_C_SYMMETRY);
    CCnstAreaButton = XtVaCreateManagedWidget("Area",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           3,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          4,
				  NULL);
    XtAddCallback(CCnstAreaButton, XmNvalueChangedCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_AREA);


    Form = CreateSubForm(CrvConstraintForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Basic Ops",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	3,
				      NULL);

    Button = XtVaCreateManagedWidget("Add",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_ADD);
 
    Button = XtVaCreateManagedWidget("Delete",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_DELETE);
 
    Button = XtVaCreateManagedWidget("Dismiss",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        2,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       3,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) CrvCnstCB, (XTP) IG_CRV_CNST_EVENT_DISMISS);
 
    if (CRVCNST_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of CrvCnst Param State is incomplete (%d).\n",
		Pos);
 
    IGCrvEditParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditPlaceMessage                                                    M
*****************************************************************************/
void IGCrvEditPlaceMessage(char *Msg)
{
    if (CEditMessageForm)
        SetLabel(CEditMessageForm, Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit param commands.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditCB(Widget w, int State, XmScaleCallbackStruct *CallData)
{    
    switch (State) {
	case IG_CRV_EDIT_EVENT_STATE:
	    ActivatePopupMenu(CrvEditForm, &CEditStatePopup,
			      IGCrvEditStateEntries, "Crv State",
			      (XTC) CrvEditStateMenuCB, 0, 100, -50);
	    break;
	case IG_CRV_EDIT_EVENT_ORDER:
	    {
		int i;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i < 999)
		    IGCrvEditParam.Order = i;
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2d", IGCrvEditParam.Order);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_ORDER, 0);
	    break;
	case IG_CRV_EDIT_EVENT_CURVE_NAME:
	    {
		char
		    *Str = XmTextGetString(w);

		if (IGCrvEditCurrentObj != NULL) {
		    IP_SET_OBJ_NAME2(IGCrvEditCurrentObj, Str);
		}
		XtFree(Str);
	    }
	    break;
	case IG_CRV_EDIT_EVENT_DRAW_MESH:
	    IGCrvEditDrawMesh = !IGCrvEditDrawMesh;
	    IGRedrawViewWindow();
	    break;
	case IG_CRV_EDIT_EVENT_DRAW_ORIG:
	    IGCrvDrawOriginal = !IGCrvDrawOriginal;
	    if (IGCrvEditCurrentObj == NULL)
		break;
	    IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
					 FALSE, TRUE, FALSE, TRUE);
	    if (IGCrvDrawOriginal)
		IGCrvEditCurrentObj -> U.Crvs = IGCrvOriginalCurve;
	    else
		IGCrvEditCurrentObj -> U.Crvs = NULL;
	    IGRedrawViewWindow();
	    break;
	case IG_CRV_EDIT_EVENT_END_COND:
	    ActivatePopupMenu(CrvEditForm, &CEditEndCondPopup,
			      IGCrvEditEndCondEntries, "End Conditions",
			      (XTC) CrvEditEndCondMenuCB, 0, 100, 50);
	    break;
	case IG_CRV_EDIT_EVENT_RATIONAL:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_RATIONAL, 0);
	    SetLabel(w, IGCrvEditParam.Rational ? "Rational" : "Polynomial");
	    break;
	case IG_CRV_EDIT_EVENT_CURVE_TYPE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_CURVE_TYPE, 0);
	    SetLabel(w, IGCrvEditParam.Type == CAGD_CBEZIER_TYPE ? "Bezier"
								  : "Bspline");
	    break;
	case IG_CRV_EDIT_EVENT_LSTSQR_PERCENT:
	    {
		int i;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i <= 100)
		    IGCrvEditParam.LstSqrPercent = i;
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2d", IGCrvEditParam.LstSqrPercent);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_CRV_EDIT_EVENT_PARAM_TYPE:
	    ActivatePopupMenu(CrvEditForm, &CEditParamTypePopup,
			      IGCrvEditParamEntries, "Crv Param Type",
			      (XTC) CrvEditParamTypeMenuCB, 0, 100, 70);
	    break;
	case IG_CRV_EDIT_EVENT_SUBDIV:
	    ActivatePopupMenu(CrvEditForm, &CEditSubdivPopup,
			      IGCrvEditSubdivEntries, "Crv Param Type",
			      (XTC) CrvEditSubdivMenuCB, 0, 5, 100);
	    break;
	case IG_CRV_EDIT_EVENT_REFINE:
	    ActivatePopupMenu(CrvEditForm, &CEditRefinePopup,
			      IGCrvEditRefineEntries, "Crv Param Type",
			      (XTC) CrvEditRefineMenuCB, 0, 100, 100);
	    break;
	case IG_CRV_EDIT_EVENT_REGION:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REGION, 0);
	    break;
	case IG_CRV_EDIT_EVENT_DRAISE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DRAISE, 0);
	    break;
	case IG_CRV_EDIT_EVENT_MOVE_CTLPTS:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MOVE_CTLPTS, 0);
	    break;
	case IG_CRV_EDIT_EVENT_DELETE_CTLPTS:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DELETE_CTLPTS, 0);
	    break;
	case IG_CRV_EDIT_EVENT_MODIFY_CURVE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MODIFY_CURVE, 0);
	    break;
	case IG_CRV_EDIT_EVENT_MERGE_CURVES:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MERGE_CURVES, 0);
	    break;
	case IG_CRV_EDIT_EVENT_REVERSE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REVERSE, 0);
	    break;
	case IG_CRV_EDIT_EVENT_EVALUATE:
	    ActivatePopupMenu(CrvEditForm, &CEditEvaluatePopup,
			      IGCrvEditEvalEntityEntries, "Evaluate",
			      (XTC) CrvEditEvalEntityMenuCB, 0, 100, 150);
	    break;
	case IG_CRV_EDIT_EVENT_UNDO:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_UNDO, 0);
	    break;
	case IG_CRV_EDIT_EVENT_REDO:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REDO, 0);
	    break;
	case IG_CRV_EDIT_EVENT_SAVE_CURVE:
	    if (IGCrvEditCurrentObj != NULL) {
		IRIT_STATIC_DATA Widget
		    FileWidget = NULL;
		Position CrvEditX, CrvEditY;
		Dimension CrvEditW;

		if (FileWidget == NULL) {
		    Arg Args[10];
		    XmString Label1, Label2, Label3;

		    Label1 = XmStringCreate("*.itd", "CharSet1");
		    Label2 = XmStringCreate("curve.itd", "CharSet1");
		    Label3 = XmStringCreate("Irit Fata Files - save curve",
					    "CharSet1");
		    XtSetArg(Args[0], XmNdirMask, Label1);
		    XtSetArg(Args[1], XmNdirSpec, Label2);
		    XtSetArg(Args[2], XmNfilterLabelString, Label3);

		    FileWidget = XmCreateFileSelectionDialog(CrvEditForm,
						    "Irit Curve Save",
						    Args,
						    3);
		    XtAddCallback(FileWidget, XmNokCallback,
				  (XTC) SaveCurveCB, NULL);
		    XtAddCallback(FileWidget, XmNcancelCallback,
				  (XTC) XtUnmanageChild, NULL);

		    XtUnmanageChild(XmFileSelectionBoxGetChild(FileWidget,
							XmDIALOG_HELP_BUTTON));

		    XmStringFree(Label1);
		    XmStringFree(Label2);
		}

		XtVaGetValues(CrvEditForm,
			      XmNwidth, &CrvEditW,
			      XmNx,     &CrvEditX,
			      XmNy,     &CrvEditY,
			      NULL);
		XtVaSetValues(FileWidget,
			      XmNdefaultPosition, FALSE,
			      XmNx,     CrvEditX + 50,
			      XmNy,     CrvEditY + 50,
			      NULL); 

		XtManageChild(FileWidget);
	    }
	    else
		IGCrvEditPlaceMessage("No curve under editing to save");
	    break;
	case IG_CRV_EDIT_EVENT_SUBMIT_CURVE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SUBMIT_CURVE, 0);
	    break;
	case IG_CRV_EDIT_EVENT_CONSTRAINTS:
	    {
		Position CrvEditX, CrvEditY;
		Dimension CrvEditH, CrvEditW;

		if (!IGCrvEditParam.SupportConstraints)
		    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DO_CONSTRAINTS,
					      0);

		XtVaGetValues(CrvEditForm,
			      XmNwidth,  &CrvEditW,
			      XmNheight, &CrvEditH,
			      XmNx,      &CrvEditX,
			      XmNy,      &CrvEditY,
			      NULL);
		XtVaSetValues(CrvConstraintForm,
			      XmNdefaultPosition, FALSE,
			      XmNwidth,  CrvEditW,
			      XmNx,      CrvEditX,
			      XmNy,      CrvEditY + CrvEditH,
			      NULL); 

		XtManageChild(CrvConstraintForm);
	    }
	    break;
	case IG_CRV_EDIT_EVENT_CLEAR:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_CLEAR, 0);
	    break;
	case IG_CRV_EDIT_EVENT_DISMISS:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_STATE,
				      IG_CRV_EDIT_STATE_DETACH);
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DISMISS, 0);
	    XtUnmanageChild(CrvEditForm);
	    break;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvCnst param commands.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvCnstCB(Widget w, int State, XmScaleCallbackStruct *CallData)
{    
    switch (State) {
	case IG_CRV_CNST_EVENT_ADD:
	    ActivatePopupMenu(CrvConstraintForm, &CCnstAddPopup,
			      IGCrvCnstAddEntries, "Add Constraint",
			      (XTC) CrvCnstAddMenuCB, 0, 5, 135);
	    break;
	case IG_CRV_CNST_EVENT_DELETE:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DELETE, 0);
	    break;
	case IG_CRV_CNST_EVENT_DISMISS:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DISMISS, 0);
	    XtUnmanageChild(CrvConstraintForm);
	    break;
	case IG_CRV_CNST_EVENT_SATISFY_ALL:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_SATISFY_ALL, 0);
	    break;
	case IG_CRV_CNST_EVENT_DO_CONSTRAINTS:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DO_CONSTRAINTS, 0);
	    break;
        case IG_CRV_CNST_EVENT_UNDULATE:
	    {
		int i;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%d", &i) == 1 && i > 0)
		    IGCrvEditParam.CnstMaxAllowedCoef = i;
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%d", (int)
			    IGCrvEditParam.CnstMaxAllowedCoef);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_CRV_CNST_EVENT_X_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_X_SYMMETRY, 0);
	    break;
	case IG_CRV_CNST_EVENT_Y_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_Y_SYMMETRY, 0);
	    break;
	case IG_CRV_CNST_EVENT_C_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_C_SYMMETRY, 0);
	    break;
	case IG_CRV_CNST_EVENT_AREA:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_AREA, 0);
	    break;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates a menu (and create new one if first time) to be popd up and then *
* activate it on screen.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:      Parent widget.                                              *
*   Popup:       Widget of the popup to update.                              *
*   StrEntries:  String entries of the popup menu.                           *
*   Title:       Title of the popup menu.                                    *
*   RelX, RelY:  Relative location of the pop up menu.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ActivatePopupMenu(Widget Parent,
			      Widget *Popup,
			      char **StrEntries,
			      char *Title,
			      XTC CallBackFunc,
			      int RelSelectIndex,
			      int RelX,
			      int RelY)
{
    Position WdgtX, WdgtY;
    Dimension WdgtW;

    if (*Popup == NULL) {
        int i;

	*Popup = XmCreatePopupMenu(CrvEditForm, Title, NULL, 0);

	for (i = 0; StrEntries[i] != NULL; i++) {
	    Widget
	        MenuItem = XtCreateManagedWidget(StrEntries[i],
						 xmPushButtonWidgetClass,
						 *Popup, NULL, 0);

	    XtAddCallback(MenuItem, XmNactivateCallback,
			  (XTC) CallBackFunc,
			  (XTP) (i + RelSelectIndex));
	}
    }

    XtVaGetValues(Parent,
		  XmNwidth, &WdgtW,
		  XmNx,     &WdgtX,
		  XmNy,     &WdgtY,
		  NULL);
    XtVaSetValues(*Popup,
		  XmNdefaultPosition, FALSE,
		  XmNx,     WdgtX + RelX,
		  XmNy,     WdgtY + RelY,
		  NULL); 

    XtManageChild(*Popup);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles file selection window Buttons.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:      Calling widget.                                  		     *
*   State:  State represented by the widget.			             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveCurveCB(Widget w,
			caddr_t ClientData,
			XmFileSelectionBoxCallbackStruct *FileStruct)
{
    char *FileName;

    XmStringGetLtoR(FileStruct -> value,
		    XmSTRING_DEFAULT_CHARSET, &FileName);

    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SAVE_CURVE,
			      (int) FileName);

    XtUnmanageChild(w);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the MR scale widget.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditParamUpdateMRScale                                              M
*****************************************************************************/
void IGCrvEditParamUpdateMRScale(void) 
{
    XmScaleSetValue(CEditSlideForm, (int) (IGCrvEditMRLevel * 1000.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve parameters.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvEditParamUpdateWidget(void) 
{
    char Line[IRIT_LINE_LEN];

    if (IGCrvEditCurrentObj != NULL)
	XmTextReplace(CEditCrvNameForm, 0, IRIT_LINE_LEN - 1,
		      IP_GET_OBJ_NAME(IGCrvEditCurrentObj));
    else
	XmTextReplace(CEditCrvNameForm, 0, IRIT_LINE_LEN - 1, "");

    SetLabel(CEditStateButton,
	     IGCrvEditStateEntries[IGCrvEditParam.CrvState]);

    SetLabel(CEditRationalButton,
	     IGCrvEditParam.Rational? "Rational" : "Polynomial");

    SetLabel(CEditCrvTypeButton,
	     IGCrvEditParam.Type == CAGD_CBEZIER_TYPE ? "Bezier" : "Bspline");

    XmToggleButtonSetState(CEditDrawMeshButton, IGCrvEditDrawMesh, FALSE);
    XmToggleButtonSetState(CEditDrawOriginalButton, IGCrvDrawOriginal, FALSE);

    sprintf(Line, "%2d", IGCrvEditParam.Order);
    XmTextReplace(CEditOrderForm, 0, IRIT_LINE_LEN - 1, Line);
    sprintf(Line, "%2d", IGCrvEditParam.LstSqrPercent);
    XmTextReplace(CEditLstSqrForm, 0, IRIT_LINE_LEN - 1, Line);

    switch (IGCrvEditParam.ParamType) {
        case CAGD_UNIFORM_PARAM:
	    SetLabel(CEditCrvParamButton, "Uniform KV");
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    SetLabel(CEditCrvParamButton, "Centripetal KV");
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    SetLabel(CEditCrvParamButton, "Chord Len KV");
	    break;
        default:
	    assert(0);
    }
    switch (IGCrvEditParam.EndCond) {
	case CAGD_END_COND_OPEN:
	    SetLabel(CEditEndCondButton, "Open EC");
	    break;
	case CAGD_END_COND_FLOAT:
	    SetLabel(CEditEndCondButton, "Float EC");
	    break;
	case CAGD_END_COND_PERIODIC:
	    SetLabel(CEditEndCondButton, "Periodic EC");
	    break;
        default:
	    assert(0);
    };

    IGCrvCnstParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve constraints parameters.        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvCnstParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvCnstParamUpdateWidget(void) 
{
    int Len = 0;
    char Line[IRIT_LINE_LEN_VLONG],
	*l = Line;
    IGCrvConstraintStruct
	*Cnst = CrvCnstList;

    Line[0] = 0;

    if (IGCrvEditParam.CnstXSymmetry) {
        sprintf(l, "X symmetry Constraint,\n");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstYSymmetry) {
	sprintf(l, "Y symmetry Constraint,\n");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstCSymmetry) {
        sprintf(l, "C symmetry Constraint,\n");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstArea) {
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
	sprintf(l, "Area Constraint (Area = %.5f),\n", IRIT_FABS(R[1]));

	Len += strlen(l);
	l = &Line[Len];
    }

    for ( ; Cnst != NULL; Cnst = Cnst -> Pnext) {
	switch (Cnst -> Type) {
	    case IG_CRV_CNST_TANGENT:
		sprintf(l, "Tangential Constraint at t = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ",\n");
		break;
	    case IG_CRV_CNST_POSITION:
		sprintf(l, "Positional Constraint at t = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ",\n");
		break;
	    case IG_CRV_CNST_CTL_PT:
		sprintf(l, "Cntrl-Point Constraint at Index = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ",\n");
		break;
	    default:
		sprintf(l, "?????? Constraint at ?\n");
		break;
	}
	Len += strlen(l);
	l = &Line[Len];
    }

    XmTextReplace(CCnstConstraintText, 0, IRIT_LINE_LEN_VLONG - 1, Line);

    sprintf(Line, "%d", (int) IGCrvEditParam.CnstMaxAllowedCoef);
    XmTextReplace(CCnstUndulateForm, 0, IRIT_LINE_LEN - 1, Line);

    XmToggleButtonSetState(CCnstSatisfyAllButton,
			   IGCrvEditParam.AbortIfCnstFailed, FALSE);
    XmToggleButtonSetState(CCnstDoConstraintsButton,
			   IGCrvEditParam.SupportConstraints, FALSE);
    XmToggleButtonSetState(CCnstXSymmetryButton,
			   IGCrvEditParam.CnstXSymmetry, FALSE);
    XmToggleButtonSetState(CCnstYSymmetryButton,
			   IGCrvEditParam.CnstYSymmetry, FALSE);
    XmToggleButtonSetState(CCnstCSymmetryButton,
			   IGCrvEditParam.CnstCSymmetry, FALSE);
    XmToggleButtonSetState(CCnstAreaButton, IGCrvEditParam.CnstArea, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit state menu pop up commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditStateMenuCB(Widget w,
			       int State,
			       XmScaleCallbackStruct *CallData)
{
    switch (State) {
	case IG_CRV_EDIT_STATE_PRIMITIVES:
	    ActivatePopupMenu(CEditStatePopup, &CEditPrimitivesPopup,
			      IGCrvEditPrimitivesEntries, "Crv Primitives",
			      (XTC) CrvEditPrimitivesMenuCB, 0, 10, 0);
	    break;
	default:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_STATE, State);
	    SetLabel(CEditStateButton,
		     IGCrvEditStateEntries[IGCrvEditParam.CrvState]);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit primitve menu pop up commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditPrimitivesMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_PRIMITIVES, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvCnst state menu pop up commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvCnstAddMenuCB(Widget w,
			     int State,
			     XmScaleCallbackStruct *CallData)
{
    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_ADD, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit curve parametrization menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditParamTypeMenuCB(Widget w,
				   int State,
				   XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_PARAM_TYPE, State);

    switch (IGCrvEditParam.ParamType) {
        case CAGD_GENERAL_PARAM:
	    SetLabel(CEditCrvParamButton, "General KV");
	    break;
        case CAGD_UNIFORM_PARAM:
	    SetLabel(CEditCrvParamButton, "Uniform KV");
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    SetLabel(CEditCrvParamButton, "Centripetal KV");
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    SetLabel(CEditCrvParamButton, "Chord Len KV");
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit curve parametrization menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditEndCondMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_END_COND, State);

    switch (IGCrvEditParam.EndCond) {
	case CAGD_END_COND_OPEN:
	    SetLabel(CEditEndCondButton, "Open EC");
	    break;
	case CAGD_END_COND_FLOAT:
	    SetLabel(CEditEndCondButton, "Float EC");
	    break;
	case CAGD_END_COND_PERIODIC:
	    SetLabel(CEditEndCondButton, "Periodic EC");
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit curve evaluation entity menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditEvalEntityMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_EVALUATE, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit curve subdivision menu pop up commands.	             *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditSubdivMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SUBDIV, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit curve refinement menu pop up commands.	             *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditRefineMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData)
{
    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REFINE, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to CrvEdit param commands - drag operations of sliders.         *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvEditDragCB(Widget w,
			  int State,
			  XmScaleCallbackStruct *CallData)
{    
    int NewValue;

    XmScaleGetValue(w, &NewValue);

    switch (State) {
	case IG_CRV_EDIT_EVENT_MR_SLIDE:
	    IGCrvEditMRLevel = NewValue / 1000.0;
	    IGRedrawViewWindow();         /* Update the MR region displayed. */
	    break;
    }
}  
