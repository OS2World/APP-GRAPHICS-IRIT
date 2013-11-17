/*****************************************************************************
*   A motif interface for surface editing.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, June 1999.  *
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
#include "editsrfs.h"

#define SRFEDIT_PARAM_FRACTION		13

#define MASK_V_DIR		1024
#define IS_MASK_V_DIR(Val)	((Val) & MASK_V_DIR)
#define CLR_MASK_V_DIR(Val)	((Val) &= ~MASK_V_DIR)
#define SET_MASK_V_DIR(Val)	((Val) |= MASK_V_DIR)

IRIT_GLOBAL_DATA Widget SEditStatePopup,
    SEditUParamTypePopup, SEditVParamTypePopup, SEditUEndCondPopup,
    SEditVEndCondPopup, SEditSubdivPopup, SEditRefinePopup,
    SEditPrimitivesPopup, SEditEvalEntityPopup, SEditReverseSrfPopup,
    SEditTrimSrfPopup,

    SrfEditForm, SEditUSlideForm, SEditVSlideForm, SEditSrfNameForm,
    SEditMessageForm, SEditUOrderForm, SEditVOrderForm,

    SEditStateButton, SEditRationalButton,
    SEditUEndCondButton, SEditVEndCondButton, SEditSrfTypeButton,
    SEditSrfUParamButton, SEditSrfVParamButton, SEditDrawMeshButton,
    SEditNormalDirButton, SEditDrawOriginalButton;

static void SrfEditCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void ActivatePopupMenu(Widget Parent,
			      Widget *Popup,
			      char **StrEntries,
			      char *Title,
			      XTC CallBackFunc,
			      int RelSelectIndex,
			      int RelX,
			      int RelY);
static void SaveSrfCB(Widget w,
		      caddr_t ClientData,
		      XmFileSelectionBoxCallbackStruct *FileStruct);
static void SrfEditStateMenuCB(Widget w,
			       int State,
			       XmScaleCallbackStruct *CallData);
static void SrfEditParamTypeMenuCB(Widget w,
				   int State,
				   XmScaleCallbackStruct *CallData);
static void SrfEditEndCondMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData);
static void SrfEditTrimSrfMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData);
static void SrfEditReverseSrfMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData);
static void SrfEditPrimitivesMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData);
static void SrfEditEvalEntityMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData);
static void SrfEditSubdivMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData);
static void SrfEditRefineMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData);
static void SrfEditDragCB(Widget w,
			  int State,
			  XmScaleCallbackStruct *CallData);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main SrfEditParam window			   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateSrfEdit                                                            M
*****************************************************************************/
void CreateSrfEdit(Widget IGTopLevel)
{
    int Pos = 0;
    Arg args[10];
    Widget Form, SubForm, Button;

    XtSetArg(args[0], XmNfractionBase, SRFEDIT_PARAM_FRACTION);
    SrfEditForm = XmCreateFormDialog(IGTopLevel, "SrfEditParamMenu", args, 1);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SEditStateButton = XtVaCreateManagedWidget("Detach Current Surface",
				     xmPushButtonWidgetClass, Form, 
				     XmNleftAttachment,       XmATTACH_FORM,
				     XmNrightAttachment,      XmATTACH_FORM,
				     NULL);
    XtAddCallback(SEditStateButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_STATE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Surface Name",
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
    SEditSrfNameForm = XtVaCreateManagedWidget("Surface Name Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     3,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    10,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		7,
				   NULL);
    XtAddCallback(SEditSrfNameForm, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_SRF_NAME);
    SEditDrawMeshButton = XtVaCreateManagedWidget("Mesh",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNset,                    IGSrfEditDrawMesh,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           10,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          13,
				  NULL);
    XtAddCallback(SEditDrawMeshButton, XmNvalueChangedCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_DRAW_MESH);
    SEditDrawOriginalButton = XtVaCreateManagedWidget("Orig",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNset,                    IGSrfEditDrawMesh,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           13,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          16,
				  NULL);
    XtAddCallback(SEditDrawOriginalButton, XmNvalueChangedCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_DRAW_ORIG);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("SurfaceGeneric",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    XtVaCreateManagedWidget("Orders:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   3,
			    NULL);
    SEditUOrderForm = XtVaCreateManagedWidget("U Order Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     3,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    5,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		2,
				   NULL);
    XtAddCallback(SEditUOrderForm, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_U_ORDER);
    SEditVOrderForm = XtVaCreateManagedWidget("V Order Input",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     5,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    7,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		2,
				   NULL);
    XtAddCallback(SEditVOrderForm, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_V_ORDER);
    SEditUEndCondButton = XtVaCreateManagedWidget("U Open EC",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        8,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       12,
				     NULL);
    XtAddCallback(SEditUEndCondButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_U_END_COND);
    SEditVEndCondButton = XtVaCreateManagedWidget("V Open EC",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        12,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       16,
				     NULL);
    XtAddCallback(SEditVEndCondButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_V_END_COND);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("SurfaceGeneric",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	16,
				      NULL);
    SEditSrfTypeButton = XtVaCreateManagedWidget("Bezier",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       4,
				     NULL);
    XtAddCallback(SEditSrfTypeButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_SRF_TYPE);
    SEditRationalButton = XtVaCreateManagedWidget("Rational",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        4,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       8,
				     NULL);
    XtAddCallback(SEditRationalButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_RATIONAL);
    SEditSrfUParamButton = XtVaCreateManagedWidget("U Uniform KV",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        8,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       12,
				     NULL);
    XtAddCallback(SEditSrfUParamButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_U_PARAM_TYPE);
    SEditSrfVParamButton = XtVaCreateManagedWidget("V Uniform KV",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        12,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       16,
				     NULL);
    XtAddCallback(SEditSrfVParamButton, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_V_PARAM_TYPE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
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
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_SUBDIV);
    Form = XtVaCreateManagedWidget("Sub Region",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_REGION);
    Form = XtVaCreateManagedWidget("U Deg Raise",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        8,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       12,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_U_DRAISE);
    Form = XtVaCreateManagedWidget("V Deg Raise",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_V_DRAISE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations two",
				      xmFormWidgetClass,    Form,
				      XmNleftAttachment,    XmATTACH_FORM,
				      XmNrightAttachment,   XmATTACH_FORM,
				      XmNfractionBase,      16,
				      NULL);
    Form = XtVaCreateManagedWidget("Refinement",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_REFINE);
    Form = XtVaCreateManagedWidget("Srf Merge",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_MERGE_SRFS);
    Form = XtVaCreateManagedWidget("Move CtlPts",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,    XmATTACH_POSITION,
				     XmNleftPosition,      8,
				     XmNrightAttachment,   XmATTACH_POSITION,
				     XmNrightPosition,     12,
				     NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_MOVE_CTLPTS);
    Form = XtVaCreateManagedWidget("MR Modify",
				   xmPushButtonWidgetClass, SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_MODIFY_SRF);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations three",
				      xmFormWidgetClass,    Form,
				      XmNleftAttachment,    XmATTACH_FORM,
				      XmNrightAttachment,   XmATTACH_FORM,
				      XmNfractionBase,      16,
				      NULL);
    Form = XtVaCreateManagedWidget("Reverse",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_REVERSE);
    Form = XtVaCreateManagedWidget("Trim Srf",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_TRIM);
    Form = XtVaCreateManagedWidget("Evaluate",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        8,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,      12,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_EVALUATE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Operations four",
				      xmFormWidgetClass,    Form,
				      XmNleftAttachment,    XmATTACH_FORM,
				      XmNrightAttachment,   XmATTACH_FORM,
				      XmNfractionBase,      16,
				      NULL);
    Form = XtVaCreateManagedWidget("Undo",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        0,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       4,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_UNDO);
    Form = XtVaCreateManagedWidget("Redo",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        4,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       8,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_REDO);
    Form = XtVaCreateManagedWidget("Save",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        8,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       12,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_SAVE_SRF);
    Form = XtVaCreateManagedWidget("Submit",
				   xmPushButtonWidgetClass,SubForm, 
				   XmNleftAttachment,      XmATTACH_POSITION,
				   XmNleftPosition,        12,
				   XmNrightAttachment,     XmATTACH_POSITION,
				   XmNrightPosition,       16,
				   NULL);
    XtAddCallback(Form, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_SUBMIT_SRF);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Multi Resolution Header",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	10,
				      NULL);
    XtVaCreateManagedWidget("Multi Resolution Control:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   6,
			    NULL);
    SEditNormalDirButton = XtVaCreateManagedWidget("MR Nrml Dir",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNset,                    IGSrfEditDrawMesh,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           6,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          10,
				  NULL);
    XtAddCallback(SEditNormalDirButton, XmNvalueChangedCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_MODIFY_NORMAL_DIR);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Multi Resolution U",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	10,
				      NULL);
    XtVaCreateManagedWidget("UMR",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   1,
			    NULL);
    SEditUSlideForm = XtVaCreateManagedWidget("UMRSlide",
			xmScaleWidgetClass, 	SubForm,
			XmNorientation,		XmHORIZONTAL,
			XmNminimum,  		0,
			XmNmaximum,		1000,
			XmNleftAttachment,   	XmATTACH_POSITION,
		        XmNleftPosition,        1,
			XmNrightAttachment, 	XmATTACH_POSITION,
			XmNrightPosition,       10,
			NULL);
    XtAddCallback(SEditUSlideForm, XmNvalueChangedCallback,
		  (XTC) SrfEditDragCB, (XTP) IG_SRF_EDIT_EVENT_UMR_SLIDE);
    XtAddCallback(SEditUSlideForm, XmNdragCallback,
		  (XTC) SrfEditDragCB, (XTP) IG_SRF_EDIT_EVENT_UMR_SLIDE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Multi Resolution V",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	10,
				      NULL);
    XtVaCreateManagedWidget("VMR",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   1,
			    NULL);
    SEditVSlideForm = XtVaCreateManagedWidget("VMRSlide",
			xmScaleWidgetClass, 	SubForm,
			XmNorientation,		XmHORIZONTAL,
			XmNminimum,  		0,
			XmNmaximum,		1000,
			XmNleftAttachment,   	XmATTACH_POSITION,
		        XmNleftPosition,        1,
			XmNrightAttachment, 	XmATTACH_POSITION,
			XmNrightPosition,       10,
			NULL);
    XtAddCallback(SEditVSlideForm, XmNvalueChangedCallback,
		  (XTC) SrfEditDragCB, (XTP) IG_SRF_EDIT_EVENT_VMR_SLIDE);
    XtAddCallback(SEditVSlideForm, XmNdragCallback,
		  (XTC) SrfEditDragCB, (XTP) IG_SRF_EDIT_EVENT_VMR_SLIDE);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SEditMessageForm = XtVaCreateManagedWidget("Messages:",
					  xmLabelWidgetClass,	Form,
					  XmNleftAttachment,    XmATTACH_FORM,
					  XmNrightAttachment,   XmATTACH_FORM,
					  NULL);

    Form = CreateSubForm(SrfEditForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("FinishUp",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	3,
				      NULL);

    Button = XtVaCreateManagedWidget("Clear",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_CLEAR);
 
    Button = XtVaCreateManagedWidget("Dismiss",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        2,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       3,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SrfEditCB, (XTP) IG_SRF_EDIT_EVENT_DISMISS);
 
    IGSrfEditParamUpdateWidget();

    if (SRFEDIT_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of SrfEdit Param State is not complete (%d).\n",
		Pos);
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
*   IGSrfEditPlaceMessage                                                    M
*****************************************************************************/
void IGSrfEditPlaceMessage(char *Msg)
{
    if (SEditMessageForm)
        SetLabel(SEditMessageForm, Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit param commands.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditCB(Widget w, int State, XmScaleCallbackStruct *CallData)
{    
    switch (State) {
	case IG_SRF_EDIT_EVENT_STATE:
            ActivatePopupMenu(SrfEditForm, &SEditStatePopup,
			      IGSrfEditStateEntries, "Srf State",
			      (XTC) SrfEditStateMenuCB, 0,
			      100, -50);
	    break;
	case IG_SRF_EDIT_EVENT_U_ORDER:
	case IG_SRF_EDIT_EVENT_V_ORDER:
	    {
		int i;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i < 999) {
		    if (State == IG_SRF_EDIT_EVENT_U_END_COND)
		        IGSrfEditParam.UOrder = i;
		    else
		        IGSrfEditParam.VOrder = i;
		}
		else {
		    char Line[IRIT_LINE_LEN];

		    if (State == IG_SRF_EDIT_EVENT_U_END_COND)
		        sprintf(Line, "%2d", IGSrfEditParam.UOrder);
		    else
		        sprintf(Line, "%2d", IGSrfEditParam.VOrder);
		    XmTextReplace(w, 0, 999, Line);
		}
		XtFree(Str);
	    }
	    SEditHandleNonMouseEvents((IGSrfEditEventType) State, 0);
	    break;
	case IG_SRF_EDIT_EVENT_SRF_NAME:
	    {
		char
		    *Str = XmTextGetString(w);

		if (IGSrfEditCurrentObj != NULL) {
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, Str);
		}
		XtFree(Str);
	    }
	    break;
	case IG_SRF_EDIT_EVENT_DRAW_MESH:
	    IGSrfEditDrawMesh = !IGSrfEditDrawMesh;
	    IGRedrawViewWindow();
	    break;
	case IG_SRF_EDIT_EVENT_DRAW_ORIG:
	    IGSrfDrawOriginal = !IGSrfDrawOriginal;
	    if (IGSrfEditCurrentObj == NULL)
		break;
	    IGActiveFreePolyIsoAttribute(IGSrfEditCurrentObj,
					 TRUE, TRUE, TRUE, TRUE);

	    if (IGSrfDrawOriginal)
		SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, IGSrfOriginalSurface)
	    else
		SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, NULL);
	    IGRedrawViewWindow();
	    break;
	case IG_SRF_EDIT_EVENT_MODIFY_NORMAL_DIR:
	    IGSrfEditNormalDir = !IGSrfEditNormalDir;
	    break;
	case IG_SRF_EDIT_EVENT_U_END_COND:
            ActivatePopupMenu(SrfEditForm, &SEditUEndCondPopup,
			      IGSrfEditEndCondEntries, "U End Conditions",
			      (XTC) SrfEditEndCondMenuCB, 0,
			      175, 50);
	    break;
	case IG_SRF_EDIT_EVENT_V_END_COND:
            ActivatePopupMenu(SrfEditForm, &SEditVEndCondPopup,
			      IGSrfEditEndCondEntries, "V End Conditions",
			      (XTC) SrfEditEndCondMenuCB, MASK_V_DIR,
			      250, 50);
	    break;
	case IG_SRF_EDIT_EVENT_RATIONAL:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_RATIONAL, 0);
	    SetLabel(w, IGSrfEditParam.Rational ? "Rational" : "Polynomial");
	    break;
	case IG_SRF_EDIT_EVENT_SRF_TYPE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SRF_TYPE, 0);
	    SetLabel(w, IGSrfEditParam.Type == CAGD_SBEZIER_TYPE ? "Bezier"
								  : "Bspline");
	    break;
	case IG_SRF_EDIT_EVENT_U_PARAM_TYPE:
            ActivatePopupMenu(SrfEditForm, &SEditUParamTypePopup,
			      IGSrfEditParamEntries, "U Srf Param",
			      (XTC) SrfEditParamTypeMenuCB, 0,
			      175, 75);
	    break;
	case IG_SRF_EDIT_EVENT_V_PARAM_TYPE:
            ActivatePopupMenu(SrfEditForm, &SEditVParamTypePopup,
			      IGSrfEditParamEntries, "V Srf Param",
			      (XTC) SrfEditParamTypeMenuCB, MASK_V_DIR,
			      250, 75);
	    break;
	case IG_SRF_EDIT_EVENT_SUBDIV:
            ActivatePopupMenu(SrfEditForm, &SEditSubdivPopup,
			      IGSrfEditSubdivEntries, "Srf Subdiv",
			      (XTC) SrfEditSubdivMenuCB, 0,
			      -25, 100);
	    break;
	case IG_SRF_EDIT_EVENT_REFINE:
            ActivatePopupMenu(SrfEditForm, &SEditRefinePopup,
			      IGSrfEditRefineEntries, "Srf Refine",
			      (XTC) SrfEditRefineMenuCB, 0,
			      0, 135);
	    break;
	case IG_SRF_EDIT_EVENT_REGION:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REGION, 0);
	    break;
	case IG_SRF_EDIT_EVENT_U_DRAISE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_DRAISE, 0);
	    break;
	case IG_SRF_EDIT_EVENT_V_DRAISE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_DRAISE, 0);
	    break;
	case IG_SRF_EDIT_EVENT_MOVE_CTLPTS:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MOVE_CTLPTS, 0);
	    break;
	case IG_SRF_EDIT_EVENT_MODIFY_SRF:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MODIFY_SRF, 0);
	    break;
	case IG_SRF_EDIT_EVENT_MERGE_SRFS:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MERGE_SRFS, 0);
	    break;
	case IG_SRF_EDIT_EVENT_REVERSE:
	    ActivatePopupMenu(SrfEditForm, &SEditReverseSrfPopup,
			      IGSrfEditReverseSrfEntries, "Reverse Srf",
			      (XTC) SrfEditReverseSrfMenuCB, 0, 0, 175);
	    break;
	case IG_SRF_EDIT_EVENT_TRIM:
            ActivatePopupMenu(SrfEditForm, &SEditTrimSrfPopup,
			      IGSrfEditTrimSrfEntries, "Trim Srf",
			      (XTC) SrfEditTrimSrfMenuCB, 0, 80, 200);
	    break;
	case IG_SRF_EDIT_EVENT_EVALUATE:
	    ActivatePopupMenu(SrfEditForm, &SEditEvalEntityPopup,
			      IGSrfEditEvalEntityEntries, "Evaluate",
			      (XTC) SrfEditEvalEntityMenuCB, 0, 120, 150);
	    break;
	case IG_SRF_EDIT_EVENT_UNDO:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_UNDO, 0);
	    break;
	case IG_SRF_EDIT_EVENT_REDO:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REDO, 0);
	    break;
	case IG_SRF_EDIT_EVENT_SAVE_SRF:
	    if (IGSrfEditCurrentObj != NULL) {
		IRIT_STATIC_DATA Widget
		    FileWidget = NULL;
		Position SrfEditX, SrfEditY;
		Dimension SrfEditW;

		if (FileWidget == NULL) {
		    Arg Args[10];
		    XmString Label1, Label2, Label3;

		    Label1 = XmStringCreate("*.itd", "CharSet1");
		    Label2 = XmStringCreate("surface.itd", "CharSet1");
		    Label3 = XmStringCreate("Irit Fata Files - save surface",
					    "CharSet1");
		    XtSetArg(Args[0], XmNdirMask, Label1);
		    XtSetArg(Args[1], XmNdirSpec, Label2);
		    XtSetArg(Args[2], XmNfilterLabelString, Label3);

		    FileWidget = XmCreateFileSelectionDialog(SrfEditForm,
						    "Irit Surface Save",
						    Args,
						    3);
		    XtAddCallback(FileWidget, XmNokCallback,
				  (XTC) SaveSrfCB, NULL);
		    XtAddCallback(FileWidget, XmNcancelCallback,
				  (XTC) XtUnmanageChild, NULL);

		    XtUnmanageChild(XmFileSelectionBoxGetChild(FileWidget,
							XmDIALOG_HELP_BUTTON));

		    XmStringFree(Label1);
		    XmStringFree(Label2);
		}

		XtVaGetValues(SrfEditForm,
			      XmNwidth, &SrfEditW,
			      XmNx,     &SrfEditX,
			      XmNy,     &SrfEditY,
			      NULL);
		XtVaSetValues(FileWidget,
			      XmNdefaultPosition, FALSE,
			      XmNx,     SrfEditX + 50,
			      XmNy,     SrfEditY + 50,
			      NULL); 

		XtManageChild(FileWidget);
	    }
	    else
		IGSrfEditPlaceMessage("No surface under editing to save");
	    break;
	case IG_SRF_EDIT_EVENT_SUBMIT_SRF:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SUBMIT_SRF, 0);
	    break;
	case IG_SRF_EDIT_EVENT_CLEAR:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_CLEAR, 0);
	    break;
	case IG_SRF_EDIT_EVENT_DISMISS:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_STATE,
				      IG_SRF_EDIT_STATE_DETACH);
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_DISMISS, 0);
	    XtUnmanageChild(SrfEditForm);
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

	*Popup = XmCreatePopupMenu(SrfEditForm, Title, NULL, 0);

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
static void SaveSrfCB(Widget w,
		      caddr_t ClientData,
		      XmFileSelectionBoxCallbackStruct *FileStruct)
{
    char *FileName;

    XmStringGetLtoR(FileStruct -> value,
		    XmSTRING_DEFAULT_CHARSET, &FileName);

    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SAVE_SRF, (int) FileName);

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
*   IGSrfEditParamUpdateMRScale                                              M
*****************************************************************************/
void IGSrfEditParamUpdateMRScale(void) 
{
    XmScaleSetValue(SEditUSlideForm, (int) (IGSrfEditMRULevel * 100.0));
    XmScaleSetValue(SEditVSlideForm, (int) (IGSrfEditMRVLevel * 100.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current surface parameters.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditParamUpdateWidget                                               M
*****************************************************************************/
void IGSrfEditParamUpdateWidget(void) 
{
    char Line[IRIT_LINE_LEN];

    if (IGSrfEditCurrentObj != NULL
)	XmTextReplace(SEditSrfNameForm, 0, 999,
		      IP_GET_OBJ_NAME(IGSrfEditCurrentObj));
    else
	XmTextReplace(SEditSrfNameForm, 0, 999, "");

    SetLabel(SEditStateButton,
	     IGSrfEditStateEntries[IGSrfEditParam.SrfState]);

    SetLabel(SEditRationalButton,
	     IGSrfEditParam.Rational? "Rational" : "Polynomial");

    SetLabel(SEditSrfTypeButton,
	     IGSrfEditParam.Type == CAGD_SBEZIER_TYPE ? "Bezier" : "Bspline");

    XmToggleButtonSetState(SEditDrawMeshButton, IGSrfEditDrawMesh, FALSE);
    XmToggleButtonSetState(SEditNormalDirButton, IGSrfEditNormalDir, FALSE);
    XmToggleButtonSetState(SEditDrawOriginalButton, IGSrfDrawOriginal, FALSE);

    sprintf(Line, "%2d", IGSrfEditParam.UOrder);
    XmTextReplace(SEditUOrderForm, 0, 999, Line);
    sprintf(Line, "%2d", IGSrfEditParam.VOrder);
    XmTextReplace(SEditVOrderForm, 0, 999, Line);

    switch (IGSrfEditParam.UParamType) {
        case CAGD_UNIFORM_PARAM:
	    SetLabel(SEditSrfUParamButton, "U Uniform KV");
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    SetLabel(SEditSrfUParamButton, "U Centripetal KV");
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    SetLabel(SEditSrfUParamButton, "U Chord Len KV");
	    break;
        default:
	    assert(0);
    }
    switch (IGSrfEditParam.VParamType) {
        case CAGD_UNIFORM_PARAM:
	    SetLabel(SEditSrfVParamButton, "V Uniform KV");
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    SetLabel(SEditSrfVParamButton, "V Centripetal KV");
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    SetLabel(SEditSrfVParamButton, "V Chord Len KV");
	    break;
        default:
	    assert(0);
    }

    switch (IGSrfEditParam.UEndCond) {
	case CAGD_END_COND_OPEN:
	    SetLabel(SEditUEndCondButton, "U Open EC");
	    break;
	case CAGD_END_COND_FLOAT:
	    SetLabel(SEditUEndCondButton, "U Float EC");
	    break;
	case CAGD_END_COND_PERIODIC:
	    SetLabel(SEditUEndCondButton, "U Periodic EC");
	    break;
        default:
	    assert(0);
    };
    switch (IGSrfEditParam.VEndCond) {
	case CAGD_END_COND_OPEN:
	    SetLabel(SEditVEndCondButton, "V Open EC");
	    break;
	case CAGD_END_COND_FLOAT:
	    SetLabel(SEditVEndCondButton, "V Float EC");
	    break;
	case CAGD_END_COND_PERIODIC:
	    SetLabel(SEditVEndCondButton, "V Periodic EC");
	    break;
        default:
	    assert(0);
    };
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit state menu pop up commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditStateMenuCB(Widget w,
			       int State,
			       XmScaleCallbackStruct *CallData)
{
    switch (State) {
	case IG_SRF_EDIT_STATE_PRIMITIVES:
	    ActivatePopupMenu(SEditStatePopup, &SEditPrimitivesPopup,
			      IGSrfEditPrimitivesEntries, "Srf Primitives",
			      (XTC) SrfEditPrimitivesMenuCB, 0, 10, 0);
	    break;
	default:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_STATE, State);
	    SetLabel(SEditStateButton,
		     IGSrfEditStateEntries[IGSrfEditParam.SrfState]);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface primitives menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditPrimitivesMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_PRIMITIVES, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface parametrization menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditParamTypeMenuCB(Widget w,
				   int State,
				   XmScaleCallbackStruct *CallData)
{
    if (IS_MASK_V_DIR(State)) {
	SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_PARAM_TYPE,
				  CLR_MASK_V_DIR(State));

	switch (IGSrfEditParam.VParamType) {
            case CAGD_UNIFORM_PARAM:
		SetLabel(SEditSrfVParamButton, "V Uniform KV");
		break;
	    case CAGD_CENTRIPETAL_PARAM:
		SetLabel(SEditSrfVParamButton, "V Centripetal KV");
		break;
	    case CAGD_CHORD_LEN_PARAM:
		SetLabel(SEditSrfVParamButton, "V Chord Len KV");
		break;
            default:
	        assert(0);
	}
    }
    else {
	SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_PARAM_TYPE,
				  CLR_MASK_V_DIR(State));

        switch (IGSrfEditParam.UParamType) {
	    case CAGD_UNIFORM_PARAM:
		SetLabel(SEditSrfUParamButton, "U Uniform KV");
		break;
	    case CAGD_CENTRIPETAL_PARAM:
		SetLabel(SEditSrfUParamButton, "U Centripetal KV");
		break;
	    case CAGD_CHORD_LEN_PARAM:
		SetLabel(SEditSrfUParamButton, "U Chord Len KV");
		break;
            default:
	        assert(0);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface parametrization menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditEndCondMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData)
{
    if (IS_MASK_V_DIR(State)) {
        SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_END_COND,
				  CLR_MASK_V_DIR(State));

	switch (IGSrfEditParam.VEndCond) {
	    case CAGD_END_COND_OPEN:
		SetLabel(SEditVEndCondButton, "V Open EC");
		break;
	    case CAGD_END_COND_FLOAT:
		SetLabel(SEditVEndCondButton, "V Float EC");
		break;
	    case CAGD_END_COND_PERIODIC:
		SetLabel(SEditVEndCondButton, "V Periodic EC");
		break;
            default:
	        assert(0);
	}
    }
    else {
        SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_END_COND,
				  CLR_MASK_V_DIR(State));

	switch (IGSrfEditParam.UEndCond) {
	    case CAGD_END_COND_OPEN:
		SetLabel(SEditUEndCondButton, "U Open EC");
		break;
	    case CAGD_END_COND_FLOAT:
		SetLabel(SEditUEndCondButton, "U Float EC");
		break;
	    case CAGD_END_COND_PERIODIC:
		SetLabel(SEditUEndCondButton, "U Periodic EC");
		break;
            default:
	        assert(0);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit trim a surface menu pop up commands.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditTrimSrfMenuCB(Widget w,
				 int State,
				 XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_TRIM, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface reverse menu pop up commands.	             *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditReverseSrfMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REVERSE, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface evaluation entity menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditEvalEntityMenuCB(Widget w,
				    int State,
				    XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_EVALUATE, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface parametrization menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditSubdivMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SUBDIV, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit surface refinement menu pop up commands.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditRefineMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData)
{
    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REFINE, State);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to SrfEdit param commands - drag operations of sliders.         *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SrfEditDragCB(Widget w,
			  int State,
			  XmScaleCallbackStruct *CallData)
{    
    int NewValue;

    XmScaleGetValue(w, &NewValue);

    switch (State) {
	case IG_SRF_EDIT_EVENT_UMR_SLIDE:
	    IGSrfEditMRULevel = NewValue / 1000.0;
	    IGRedrawViewWindow();         /* Update the MR region displayed. */
	    break;
	case IG_SRF_EDIT_EVENT_VMR_SLIDE:
	    IGSrfEditMRVLevel = NewValue / 1000.0;
	    IGRedrawViewWindow();         /* Update the MR region displayed. */
	    break;
        default:
	    assert(0);
    }
}  
