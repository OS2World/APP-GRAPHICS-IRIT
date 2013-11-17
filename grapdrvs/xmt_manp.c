/*****************************************************************************
*   A motif interface for linear transformations on objects.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, July 1999.  *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/FileSB.h>
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
#include "editmanp.h"

#define OBJ_MANIP_PARAM_FRACTION 18

IRIT_STATIC_DATA int
    IsDragOp = FALSE;

IRIT_STATIC_DATA Widget
    AttachObjectButton, ObjectScreenButton, ObjManipSnapButton,
    ObjManipStatePopup, ObjManipMessageForm, ObjManipSnapDegreesForm,
    ObjManipNameMatrixForm, ObjManipSnapDistanceForm;

IRIT_STATIC_DATA IPObjectStruct
    **IGObjManipStartObjs = NULL;

IRIT_GLOBAL_DATA Widget ObjManipParamForm;

static void SetCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void SaveObjectCB(Widget w,
			 caddr_t ClientData,
			 XmFileSelectionBoxCallbackStruct *FileStruct);
static void ObjManipStateMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData);
static void ScaleCB(Widget w, IGGraphicEventType EventType);
static void DragCB(Widget w, IGGraphicEventType EventType);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main ObjManipParam window			  	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateObjManipParam                                                      M
*****************************************************************************/
void CreateObjManipParam(Widget IGTopLevel)
{
    int i,
	t = 0,
	Pos = 0;
    Arg args[10];
    IRIT_STATIC_DATA char *SlideTitle[] = {
	" ROTATE",
	" TRANSLATE",
	" SCALE"
    };
    IRIT_STATIC_DATA char SlideLabel[][2] = {
	"X",
	"Y",
	"Z",
	"X",
	"Y",
	"Z",
	" ",
    };    
    Widget Form, SubForm, Label, Button, Scale;
    IGGraphicEventType EventType;

    XtSetArg(args[0], XmNfractionBase, OBJ_MANIP_PARAM_FRACTION);
    ObjManipParamForm = XmCreateFormDialog(IGTopLevel,
					   "ObjManipParamMenu", args, 1);

    AddLabel(ObjManipParamForm, "Manipulate Objects", Pos++);

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    AttachObjectButton = XtVaCreateManagedWidget("Process Object",
				     xmPushButtonWidgetClass, Form, 
				     XmNleftAttachment,       XmATTACH_FORM,
				     XmNrightAttachment,      XmATTACH_FORM,
				     NULL);
    XtAddCallback(AttachObjectButton, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_STATE);

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    ObjectScreenButton = XtVaCreateManagedWidget("Object Screen Space",
				     xmPushButtonWidgetClass, Form, 
				     XmNleftAttachment,       XmATTACH_FORM,
				     XmNrightAttachment,      XmATTACH_FORM,
				     NULL);
    XtAddCallback(ObjectScreenButton, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_OBJECT_SCREEN);

    /* Construct all slide bars (Scales) */
    for (i = 0, EventType = IG_EVENT_ROTATE_X; i < 7; i++, EventType++) {
	if (EventType == IG_EVENT_TRANSLATE)
	    EventType++;			       /* Skip these events. */
        if (i == 0 || i == 3 || i == 6) {
            Form = CreateSubForm(ObjManipParamForm, Pos++); 
            Label = XtVaCreateManagedWidget(SlideTitle[t++],
					    xmLabelWidgetClass,  Form,
					    XmNleftAttachment,   XmATTACH_FORM,
					    XmNrightAttachment,  XmATTACH_FORM,
					    NULL);              
        }
        Form = CreateSubForm(ObjManipParamForm, Pos++); 	
        Label = XtVaCreateManagedWidget(SlideLabel[i],
					xmLabelWidgetClass, Form,
					NULL);
        Scale = XtVaCreateManagedWidget("Scale",
					xmScaleWidgetClass,  Form,
					XmNorientation,	     XmHORIZONTAL,
					XmNminimum,          -100,
					XmNrightAttachment,  XmATTACH_FORM,
					XmNleftAttachment,   XmATTACH_WIDGET,
					XmNleftWidget,       Label,
					XmNmaximum,	     100,
					NULL);	
        XtAddCallback(Scale, XmNvalueChangedCallback,
		      (XTC) ScaleCB, (XTP) EventType);
        XtAddCallback(Scale, XmNdragCallback, (XTC) DragCB, (XTP) EventType);
    }

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Naming",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	5,
				      NULL);
    XtVaCreateManagedWidget("Mat Name:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   2,
			    NULL);
    ObjManipNameMatrixForm = XtVaCreateManagedWidget("Snap Degrees",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     2,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    5,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		3,
				   NULL);
    XtAddCallback(ObjManipNameMatrixForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_NAME_MATRIX);


    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Snapping",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	14,
				      NULL);
    XtVaCreateManagedWidget("Deg:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   2,
			    NULL);
    ObjManipSnapDegreesForm = XtVaCreateManagedWidget("Snap Degrees",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     2,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    5,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		3,
				   NULL);
    XtAddCallback(ObjManipSnapDegreesForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SNAP_DEGREES);
    XtVaCreateManagedWidget("Dist:",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    5,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   7,
			    NULL);
    ObjManipSnapDistanceForm = XtVaCreateManagedWidget("Snap Distance",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     7,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    11,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		4,
				   NULL);
    XtAddCallback(ObjManipSnapDistanceForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SNAP_DISTANCE);
    ObjManipSnapButton = XtVaCreateManagedWidget("Snap",
				  xmToggleButtonGadgetClass, SubForm,
				  XmNleftAttachment,         XmATTACH_POSITION,
				  XmNleftPosition,           11,
				  XmNrightAttachment,        XmATTACH_POSITION,
				  XmNrightPosition,          14,
				  NULL);
    XtAddCallback(ObjManipSnapButton, XmNvalueChangedCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SNAP);

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Buttons",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	3,
				      NULL);

    Button = XtVaCreateManagedWidget("Save",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SAVE);
    Button = XtVaCreateManagedWidget("Submit",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SUBMIT_MAT);
    Button = XtVaCreateManagedWidget("Delete",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        2,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       3,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_DELETE);

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("FinishUp",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	2,
				      NULL);
    Button = XtVaCreateManagedWidget("Reverse",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_REVERSE);
    Button = XtVaCreateManagedWidget("Color",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_SET_COLOR);

    Form = CreateSubForm(ObjManipParamForm, Pos++);         
    ObjManipMessageForm = XtVaCreateManagedWidget("Messages:",
					  xmLabelWidgetClass,	Form,
					  XmNleftAttachment,    XmATTACH_FORM,
					  XmNrightAttachment,   XmATTACH_FORM,
					  NULL);

    Form = CreateSubForm(ObjManipParamForm, Pos);         
    SubForm = XtVaCreateManagedWidget("FinishUp",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	2,
				      NULL);
    Button = XtVaCreateManagedWidget("Reset",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_RESET);
    Button = XtVaCreateManagedWidget("Dismiss",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_OBJ_MANIP_EVENT_DISMISS);

    if (OBJ_MANIP_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of ObjManip Param State is not complete (%d).\n",
		Pos);

    IGObjManipParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current object transformation state.         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipParamUpdateWidget                                              M
*****************************************************************************/
void IGObjManipParamUpdateWidget(void) 
{
    char Line[IRIT_LINE_LEN];

    SetLabel(ObjectScreenButton,
	     IGObjManipScreenSpace ? "Screen Space" : "Object Space");

    if (IGObjManipNumActiveObjs == 1) {
	sprintf(Line, "Manip. \"%s\"",
		IP_GET_OBJ_NAME(IGObjManipCurrentObjs[0]));
	IGObjManipPlaceMessage(Line);
    }
    else {
	sprintf(Line, "Manip. Several/All Objs");
	IGObjManipPlaceMessage(Line);
    }

    XmToggleButtonSetState(ObjManipSnapButton, IGObjManipSnap, FALSE);

    XmTextReplace(ObjManipNameMatrixForm , 0, IRIT_LINE_LEN - 1,
		  IGObjManipMatName);

    sprintf(Line, "%g", IGObjManipSnapDegrees);
    XmTextReplace(ObjManipSnapDegreesForm , 0, IRIT_LINE_LEN - 1, Line);
    sprintf(Line, "%g", IGObjManipSnapDistance);
    XmTextReplace(ObjManipSnapDistanceForm , 0, IRIT_LINE_LEN - 1, Line);

    SetLabel(AttachObjectButton,
	     IGObjManipStateEntries[IGObjManipState]);
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
*   IGObjManipPlaceMessage                                                   M
*****************************************************************************/
void IGObjManipPlaceMessage(char *Msg)
{
    if (ObjManipMessageForm)
        SetLabel(ObjManipMessageForm, Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to object transform param commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetCB(Widget w, int State, XmScaleCallbackStruct *CallData)
{    
    switch (State) {
	case IG_OBJ_MANIP_EVENT_STATE:
	    {
		Position ObjManipX, ObjManipY, WdgtX, WdgtY;
		Dimension ObjManipW, WdgtW;

		if (ObjManipStatePopup == NULL) {
		    int i;

		    ObjManipStatePopup = XmCreatePopupMenu(ObjManipParamForm,
							   "ObjManip State",
							   NULL, 0);
		    
		    for (i = 0; IGObjManipStateEntries[i] != NULL; i++) {
			Widget
			    MenuItem = XtCreateManagedWidget(
						    IGObjManipStateEntries[i],
						    xmPushButtonWidgetClass,
						    ObjManipStatePopup,
						    NULL, 0);

			XtAddCallback(MenuItem, XmNactivateCallback,
				      (XTC) ObjManipStateMenuCB, (XTP) i);
		    }
		}

		XtVaGetValues(w,
			      XmNwidth, &WdgtW,
			      XmNx,     &WdgtX,
			      XmNy,     &WdgtY,
			      NULL);
		XtVaGetValues(ObjManipParamForm,
			      XmNwidth, &ObjManipW,
			      XmNx,     &ObjManipX,
			      XmNy,     &ObjManipY,
			      NULL);
		XtVaSetValues(ObjManipStatePopup,
			      XmNdefaultPosition, FALSE,
			      XmNx,     ObjManipX + WdgtX + WdgtW / 2,
			      XmNy,     ObjManipY + WdgtY + 16,
			      NULL); 

		XtManageChild(ObjManipStatePopup);
	    }
	    break;
	case IG_OBJ_MANIP_EVENT_OBJECT_SCREEN:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_OBJECT_SCREEN,
					   0, NULL);
	    break;
	case IG_OBJ_MANIP_EVENT_SNAP:
	    IGObjManipSnap = !IGObjManipSnap;
	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_EVENT_SNAP_DEGREES:
	    {
		double d;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%lf", &d) == 1 && d >= 0 && d < 360)
		    IGObjManipSnapDegrees = d;
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%g", IGObjManipSnapDegrees);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_OBJ_MANIP_EVENT_NAME_MATRIX:
	    {
		char
		    *Str = XmTextGetString(w);

		strncpy(IGObjManipMatName, Str, IRIT_LINE_LEN - 2);
		XtFree(Str);
	    }
	    break;
	case IG_OBJ_MANIP_EVENT_SNAP_DISTANCE:
	    {
		double d;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%lf", &d) == 1 && d >= 0 && d < 360)
		    IGObjManipSnapDistance = d;
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%g", IGObjManipSnapDistance);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_OBJ_MANIP_EVENT_SAVE:
	    {
		IRIT_STATIC_DATA Widget
		    FileWidget = NULL;
		Position SaveX, SaveY;
		Dimension SaveW;

		if (FileWidget == NULL) {
		    Arg Args[10];
		    XmString Label1, Label2, Label3;

		    Label1 = XmStringCreate("*.itd", "CharSet1");
		    Label2 = XmStringCreate("save.itd", "CharSet1");
		    Label3 = XmStringCreate("Irit Data Files - save data",
					    "CharSet1");
		    XtSetArg(Args[0], XmNdirMask, Label1);
		    XtSetArg(Args[1], XmNdirSpec, Label2);
		    XtSetArg(Args[2], XmNfilterLabelString, Label3);

		    FileWidget = XmCreateFileSelectionDialog(ObjManipParamForm,
							     "Irit Save",
							     Args,
							     3);
		    XtAddCallback(FileWidget, XmNokCallback,
				  (XTC) SaveObjectCB, NULL);
		    XtAddCallback(FileWidget, XmNcancelCallback,
				  (XTC) XtUnmanageChild, NULL);

		    XtUnmanageChild(XmFileSelectionBoxGetChild(FileWidget,
							XmDIALOG_HELP_BUTTON));

		    XmStringFree(Label1);
		    XmStringFree(Label2);
		}

		XtVaGetValues(ObjManipParamForm,
			      XmNwidth, &SaveW,
			      XmNx,     &SaveX,
			      XmNy,     &SaveY,
			      NULL);
		XtVaSetValues(FileWidget,
			      XmNdefaultPosition, FALSE,
			      XmNx,     SaveX + 50,
			      XmNy,     SaveY + 50,
			      NULL); 

		XtManageChild(FileWidget);
	    }
	    break;
	case IG_OBJ_MANIP_EVENT_SUBMIT_MAT:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_SUBMIT_MAT,
					   0, NULL);
	    break;
	case IG_OBJ_MANIP_EVENT_REVERSE:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_REVERSE,
					   0, NULL);
	    break;
	case IG_OBJ_MANIP_EVENT_SET_COLOR:
	    break;
	case IG_OBJ_MANIP_EVENT_DELETE:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_DELETE, 0, NULL);
	    break;
	case IG_OBJ_MANIP_EVENT_RESET:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_RESET, 0, NULL);
	    break;
	case IG_OBJ_MANIP_EVENT_DISMISS:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_DISMISS, 0, NULL);
	    XtUnmanageChild(ObjManipParamForm);
	    break;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles file selection window Buttons.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:             Calling widget.                            		     *
*   ClientData:    Not used.                                                 *
*   FileStruct:    Holds the file name.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveObjectCB(Widget w,
			 caddr_t ClientData,
			 XmFileSelectionBoxCallbackStruct *FileStruct)
{
    char *FileName;

    XmStringGetLtoR(FileStruct -> value,
		    XmSTRING_DEFAULT_CHARSET, &FileName);

    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_SAVE, 0, FileName);

    XtUnmanageChild(w);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to ObjManip state menu pop up commands.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ObjManipStateMenuCB(Widget w,
				int State,
				XmScaleCallbackStruct *CallData)
{
    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_STATE, State, NULL);

    SetLabel(AttachObjectButton,
	     IGObjManipStateEntries[IGObjManipState]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Treats scales mouse-click movements.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:          Calling scale.			                             *
*   EventType:  Event represented by the calling scale.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScaleCB(Widget w, IGGraphicEventType EventType)
{
    int NewValue;
    IrtRType ChangeFactor[2];

    XmScaleGetValue(w, &NewValue);
    XmScaleSetValue(w, 0);

    ChangeFactor[0] = IGGlblChangeFactor * NewValue / 100;
    ChangeFactor[1] = 0.0;

    if (IGObjManipNumActiveObjs > 0) {
	if (!IsDragOp) {
	    int i;

	    for (i = 0; i < IGObjManipNumActiveObjs; i++)
	        IGObjManipObjTrans(IGObjManipCurrentObjs[i],
				   IGObjManipCurrentObjs[i],
				   EventType, ChangeFactor);
	}
	IsDragOp = FALSE;
	IGRedrawViewWindow();

	IGObjManipObjTrans(NULL, NULL, IG_EVENT_ACCUM_MATRIX, ChangeFactor);
    }
    else
	IGObjManipPlaceMessage("Select Object(s) First");

    if (IGObjManipStartObjs != NULL) {
        IGObjManipFreeStartObjs(IGObjManipStartObjs);
	IGObjManipStartObjs = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Treats scales mouse-drag movements.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:          Calling scale.		                                     *
*   EventType:  Event represented by the calling scale.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DragCB(Widget w, IGGraphicEventType EventType)
{
    int NewValue;
    IrtRType ChangeFactor[2];

    IsDragOp = TRUE;

    XmScaleGetValue(w, &NewValue);
    ChangeFactor[0] = IGGlblChangeFactor * NewValue / 100;
    ChangeFactor[1] = 0.0;

    if (IGObjManipStartObjs == NULL)
        IGObjManipStartObjs = IGObjManipCopyStartObjs();

    if (IGObjManipNumActiveObjs > 0) {
	int i;

	for (i = 0; i < IGObjManipNumActiveObjs; i++)
	    IGObjManipObjTrans(IGObjManipCurrentObjs[i],
			       IGObjManipStartObjs[i],
			       EventType, ChangeFactor);
	IGGlblManipulationActive = TRUE;
	IGRedrawViewWindow();
	IGGlblManipulationActive = FALSE;
    }
    else
	IGObjManipPlaceMessage("Select Object(s) First");
}
