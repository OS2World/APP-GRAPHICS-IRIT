/*****************************************************************************
*   A motif interface for shade parameter.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, Sep. 1997.  *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "xmtdrvs.h"

#define SHADE_PARAM_FRACTION 25

IRIT_STATIC_DATA int 
    LastSlideVal = 0,
    ShadeActiveLgtSrc = 0;
IRIT_STATIC_DATA Widget AmbientLabel, DiffuseLabel, SpecularLabel,
    ShininessLabel, EmissionLabel, SketchSilPowerLabel, SketchShdPowerLabel,
    SketchSilStyle, SketchShdStyle, SketchImpStyle,
    SketchShdImportanceDecay, SketchShdImpFrntSprt, ShadeLightNumForm,
    ShadeLightXForm, ShadeLightYForm, ShadeLightZForm, ShadeLightWForm;
IRIT_STATIC_DATA IGShadeParamStruct IGShadeParamReset;
IRIT_STATIC_DATA IGSketchParamStruct IGSketchParamReset;

IRIT_GLOBAL_DATA Widget ShadeParamForm;

static void SketchStyleCB(Widget w, int Index);
static void SetCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void DragCB(Widget w, int State, XmScaleCallbackStruct *CallData);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main ShadeParam window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateShadeParam                                                         M
*****************************************************************************/
void CreateShadeParam(Widget IGTopLevel)
{
    IRIT_STATIC_DATA char
	*SketchStyles[] = {
	    "IsoCrv",
	    "Crvtr",
	    "Iscln",
	    "Ortcn"
	};
    int Pos = 0;
    char Label[30];
    Arg args[10];
    Widget Form, SubForm, Button;

    IGShadeParamReset = IGShadeParam;	                 /* Save init value. */
    IGSketchParamReset = IGSketchParam;

    XtSetArg(args[0], XmNfractionBase, SHADE_PARAM_FRACTION);
    ShadeParamForm = XmCreateFormDialog(IGTopLevel, "ShadeParamMenu", args, 1);

    sprintf(Label, "Ambient %6.4f", IGShadeParam.LightAmbient[0]);
    AmbientLabel = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SHADE_AMBIENT);

    sprintf(Label, "Diffuse %6.4f", IGShadeParam.LightDiffuse[0]);
    DiffuseLabel = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SHADE_DIFFUSE);

    sprintf(Label, "Specular %6.4f", IGShadeParam.LightSpecular[0]);
    SpecularLabel = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SHADE_SPECULAR);

    sprintf(Label, "Shininess %7.4f", IGShadeParam.Shininess);
    ShininessLabel = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SHADE_SHININESS);

    sprintf(Label, "Emission %6.4f", IGShadeParam.LightEmissive[0]);
    EmissionLabel = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SHADE_EMISSION);

    Form = CreateSubForm(ShadeParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Light Sources Header",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,         10,
				      NULL);
    XtVaCreateManagedWidget("Light",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   2,
			    NULL);
    XtVaCreateManagedWidget("X",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    2,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   4,
			    NULL);
    XtVaCreateManagedWidget("Y",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    4,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   6,
			    NULL);
    XtVaCreateManagedWidget("Z",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    6,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,   8,
			    NULL);
    XtVaCreateManagedWidget("W",
			    xmLabelWidgetClass,	SubForm,
			    XmNleftAttachment, 	XmATTACH_POSITION,
			    XmNleftPosition,    8,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition,  10,
			    NULL);

    Form = CreateSubForm(ShadeParamForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("Light Sources",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	10,
				      NULL);
    ShadeLightNumForm = XtVaCreateManagedWidget("Light Source Index",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     0,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    2,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		1,
				   NULL);
    XtAddCallback(ShadeLightNumForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SHADE_LGT_SRC_IDX);
    ShadeLightXForm = XtVaCreateManagedWidget("Light Source X",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     2,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    4,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		1,
				   NULL);
    XtAddCallback(ShadeLightXForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SHADE_LGT_SRC_X);
    ShadeLightYForm = XtVaCreateManagedWidget("Light Source Y",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     4,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    6,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		1,
				   NULL);
    XtAddCallback(ShadeLightYForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SHADE_LGT_SRC_Y);
    ShadeLightZForm = XtVaCreateManagedWidget("Light Source Z",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     6,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,    8,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		1,
				   NULL);
    XtAddCallback(ShadeLightZForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SHADE_LGT_SRC_Z);
    ShadeLightWForm = XtVaCreateManagedWidget("Light Source W",
				   xmTextWidgetClass,   SubForm, 
				   XmNleftAttachment,   XmATTACH_POSITION,
				   XmNleftPosition,     8,
				   XmNrightAttachment,  XmATTACH_POSITION,
				   XmNrightPosition,   10,
				   XmNeditMode,	        XmSINGLE_LINE_EDIT,
				   XmNcolumns,		1,
				   NULL);
    XtAddCallback(ShadeLightWForm, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SHADE_LGT_SRC_W);

    sprintf(Label, "Sketch Silhouette %5.3f", IGSketchParam.SilPower);
    SketchSilPowerLabel = AddLabel(ShadeParamForm, Label, Pos++);
    SketchSilStyle = AddRadioButton(ShadeParamForm, "Sketch Silhouette Style",
				    IGSketchParam.SketchSilType,
				    SketchStyles, 4, Pos++,
				    (XTC) SketchStyleCB);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SKETCH_SIL_POWER);

    sprintf(Label, "Sketch Shading %5.3f", IGSketchParam.ShadePower);
    SketchShdPowerLabel = AddLabel(ShadeParamForm, Label, Pos++);
    SketchShdStyle = AddRadioButton(ShadeParamForm, "Sketch Shading Style",
				    IGSketchParam.SketchSilType,
				    SketchStyles, 4, Pos++,
				    (XTC) SketchStyleCB);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SKETCH_SHADING_POWER);

    sprintf(Label, "Sketch Importance %5.3f", IGSketchParam.SketchImpDecay);
    SketchShdImportanceDecay = AddLabel(ShadeParamForm, Label, Pos++);
    SketchImpStyle = AddRadioButton(ShadeParamForm, "Sketch Importance Style",
				    IGSketchParam.SketchSilType,
				    SketchStyles, 4, Pos++,
				    (XTC) SketchStyleCB);
    AddButtonSlide(ShadeParamForm, Pos++,
		   IGSketchParam.SketchImp ? "Imp" : "No Imp",
		   (XTC) SetCB, (XTP) IG_STATE_SKETCH_IMPORTANCE,
		   -1000, 1000, (XTC) SetCB, (XTC) DragCB,
		   (XTP) IG_STATE_SKETCH_IMP_DECAY);

    sprintf(Label, "Frontal Support %4.2f", IGSketchParam.SketchImpFrntSprt);
    SketchShdImpFrntSprt = AddLabel(ShadeParamForm, Label, Pos++);
    AddSlide(ShadeParamForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) DragCB, (XTP) IG_STATE_SKETCH_IMP_FRNT_SPRT);

    AddTwoButtons(ShadeParamForm, Pos++,
		  "Reset", (XTC) SetCB, (XTP) IG_STATE_SHADE_RESET,
		  "Dismiss", (XTC) SetCB, (XTP) IG_STATE_SHADE_DISMISS,
		  &Button, &Button);

    if (SHADE_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of Shade Param State is not complete (%d).\n",
		Pos);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles radio buttons' events.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:      Calling widget.                                  		     *
*   Index:  Button index pressed.               		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SketchStyleCB(Widget w, int Index)
{
    int *Style;
    Widget
	ParentW = XtParent(w);	

    if (ParentW == SketchSilStyle)
	Style = &IGSketchParam.SketchSilType;
    else if (ParentW == SketchShdStyle)
	Style = &IGSketchParam.SketchShdType;
    else if (ParentW == SketchImpStyle)
	Style = &IGSketchParam.SketchImpType;

    switch (Index) {
	default:
	case 1:
	    if (*Style == IG_SKETCHING_ISO_PARAM)
	        return;
	    *Style = IG_SKETCHING_ISO_PARAM;
	    break;
	case 2:
	    if (*Style == IG_SKETCHING_CURVATURE)
	        return;
	    *Style = IG_SKETCHING_CURVATURE;
	    break;
	case 3:
	    if (*Style == IG_SKETCHING_ISOCLINES)
	        return;
	    *Style = IG_SKETCHING_ISOCLINES;
	    break;
	case 4:
	    if (*Style == IG_SKETCHING_ORTHOCLINES)
	        return;
	    *Style = IG_SKETCHING_ORTHOCLINES;
	    break;
    }

    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
				     FALSE, FALSE, TRUE, FALSE);

    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to shade param commands.	  				     *
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
    char Label[IRIT_LINE_LEN];

    switch (State) {
	case IG_STATE_SHADE_AMBIENT:
	case IG_STATE_SHADE_DIFFUSE:
	case IG_STATE_SHADE_SPECULAR:
	case IG_STATE_SHADE_SHININESS:
	case IG_STATE_SHADE_EMISSION:
	case IG_STATE_SKETCH_SIL_POWER:
	case IG_STATE_SKETCH_SHADING_POWER:
        case IG_STATE_SKETCH_IMP_DECAY:
        case IG_STATE_SKETCH_IMP_FRNT_SPRT:
	    DragCB(w, State, CallData);
	    XmScaleSetValue(w, 0);
	    LastSlideVal = 0;
	    break;
	case IG_STATE_SHADE_LGT_SRC_IDX:
	    {
		int i;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%d", &i) == 1 && i >= 0 && i <= 7) {
		    ShadeActiveLgtSrc = i;

		    sprintf(Label, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc][0]);
		    XmTextReplace(ShadeLightXForm, 0, IRIT_LINE_LEN - 1,
				  Label);
		    sprintf(Label, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc][1]);
		    XmTextReplace(ShadeLightYForm, 0, IRIT_LINE_LEN - 1,
				  Label);
		    sprintf(Label, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc][2]);
		    XmTextReplace(ShadeLightZForm, 0, IRIT_LINE_LEN - 1,
				  Label);
		    sprintf(Label, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc][3]);
		    XmTextReplace(ShadeLightWForm, 0, IRIT_LINE_LEN - 1,
				  Label);
		}
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2d", ShadeActiveLgtSrc);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_STATE_SHADE_LGT_SRC_X:
	case IG_STATE_SHADE_LGT_SRC_Y:
	case IG_STATE_SHADE_LGT_SRC_Z:
	case IG_STATE_SHADE_LGT_SRC_W:
	    {
		float f;
		char
		    *Str = XmTextGetString(w);

		if (sscanf(Str, "%f", &f) == 1) {
		    IRIT_STATIC_DATA IrtVecType
			WhiteColor = { 1.0, 1.0, 1.0 };

		    IGShadeParam.LightPos[ShadeActiveLgtSrc]
				    [State - IG_STATE_SHADE_LGT_SRC_X] = f;
		    
		    IGSetLightSource(IGShadeParam.LightPos[ShadeActiveLgtSrc],
				     WhiteColor, ShadeActiveLgtSrc);
		    IGGlblLastWasSolidRendering = FALSE;
		    IGRedrawViewWindow();
		}
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc]
					[State - IG_STATE_SHADE_LGT_SRC_X]);
		    XmTextReplace(w, 0, IRIT_LINE_LEN - 1, Line);
		}
		XtFree(Str);
	    }
	    break;
	case IG_STATE_SHADE_RESET:
	    IGShadeParam = IGShadeParamReset;	      /* Restore init value. */
	    IGSketchParam = IGSketchParamReset;

	    sprintf(Label, "Ambient %6.4f", IGShadeParam.LightAmbient[0]);
	    SetLabel(AmbientLabel, Label);

	    sprintf(Label, "Diffuse %6.4f", IGShadeParam.LightDiffuse[0]);
	    SetLabel(DiffuseLabel, Label);

	    sprintf(Label, "Specular %6.4f", IGShadeParam.LightSpecular[0]);
	    SetLabel(SpecularLabel, Label);

	    sprintf(Label, "Shininess %7.4f", IGShadeParam.Shininess);
	    SetLabel(ShininessLabel, Label);

	    sprintf(Label, "Emission %7.4f", IGShadeParam.LightEmissive[0]);
	    SetLabel(EmissionLabel, Label);

	    sprintf(Label, "Sketch Silhouette %5.3f", IGSketchParam.SilPower);
	    SetLabel(SketchSilPowerLabel, Label);

	    sprintf(Label, "Sketch Shading %5.3f", IGSketchParam.ShadePower);
	    SetLabel(SketchShdPowerLabel, Label);

	    sprintf(Label, "Sketch Importance %5.3f",
		    IGSketchParam.SketchImpDecay);
	    SetLabel(SketchShdImportanceDecay, Label);

	    sprintf(Label, "Frontal Support %4.2f",
		    IGSketchParam.SketchImpFrntSprt);
	    SetLabel(SketchShdImpFrntSprt, Label);

	    /* Force a reevaluation of the sketches. */
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
        case IG_STATE_SKETCH_IMPORTANCE:
	    IGSketchParam.SketchImp = !IGSketchParam.SketchImp;
	    SetLabel(w, IGSketchParam.SketchImp ? "Imp" : "No Imp");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SHADE_DISMISS:
	    XtUnmanageChild(ShadeParamForm);
	    break;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to shade param commands - drag operations of sliders.           *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   State:     State represented by widget.				     *
*   CallData:  For detecting drag/click action on scales.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DragCB(Widget w, int State, XmScaleCallbackStruct *CallData)
{    
    int NewValue;
    char Label[30];
    float FDiff;

    XmScaleGetValue(w, &NewValue);
    FDiff = (float) ((NewValue - LastSlideVal) * IGGlblChangeFactor / 1000.0);
    LastSlideVal = NewValue;

    switch (State) {
	case IG_STATE_SHADE_AMBIENT:
	    IGShadeParam.LightAmbient[0] += FDiff;
	    if (IGShadeParam.LightAmbient[0] < 0.0) 
	        IGShadeParam.LightAmbient[0] = 0.0;

	    IGShadeParam.LightAmbient[1] = 
	        IGShadeParam.LightAmbient[2] = IGShadeParam.LightAmbient[0];

	    sprintf(Label, "Ambient %6.4f", IGShadeParam.LightAmbient[0]);
	    SetLabel(AmbientLabel, Label);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SHADE_DIFFUSE:
	    IGShadeParam.LightDiffuse[0] += FDiff;
	    if (IGShadeParam.LightDiffuse[0] < 0.0) 
	        IGShadeParam.LightDiffuse[0] = 0.0;

	    IGShadeParam.LightDiffuse[1] = 
	        IGShadeParam.LightDiffuse[2] = IGShadeParam.LightDiffuse[0];

	    sprintf(Label, "Diffuse %6.4f", IGShadeParam.LightDiffuse[0]);
	    SetLabel(DiffuseLabel, Label);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SHADE_SPECULAR:
	    IGShadeParam.LightSpecular[0] += FDiff;
	    if (IGShadeParam.LightSpecular[0] < 0.0) 
	        IGShadeParam.LightSpecular[0] = 0.0;

	    IGShadeParam.LightSpecular[1] = 
	        IGShadeParam.LightSpecular[2] = IGShadeParam.LightSpecular[0];

	    sprintf(Label, "Specular %6.4f", IGShadeParam.LightSpecular[0]);
	    SetLabel(SpecularLabel, Label);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SHADE_SHININESS:
	    IGShadeParam.Shininess += FDiff * 10.0;
	    if (IGShadeParam.Shininess < 0.0) 
	        IGShadeParam.Shininess = 0.0;

	    sprintf(Label, "Shininess %7.4f", IGShadeParam.Shininess);
	    SetLabel(ShininessLabel, Label);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SHADE_EMISSION:
	    IGShadeParam.LightEmissive[0] += FDiff;
	    if (IGShadeParam.LightEmissive[0] < 0.0) 
	        IGShadeParam.LightEmissive[0] = 0.0;

	    IGShadeParam.LightEmissive[1] = 
	        IGShadeParam.LightEmissive[2] = IGShadeParam.LightEmissive[0];

	    sprintf(Label, "Emission %6.4f", IGShadeParam.LightEmissive[0]);
	    SetLabel(EmissionLabel, Label);

	    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid draw. */
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SKETCH_SIL_POWER:
	    IGSketchParam.SilPower += FDiff;
	    if (IGSketchParam.SilPower < 0.0) 
	        IGSketchParam.SilPower = 0.0;
	    if (IGSketchParam.SilPower > 1.0) 
	        IGSketchParam.SilPower = 1.0;

	    sprintf(Label, "Sketch Sil %5.3f", IGSketchParam.SilPower);
	    SetLabel(SketchSilPowerLabel, Label);

	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SKETCH_SHADING_POWER:
	    IGSketchParam.ShadePower += FDiff;
	    if (IGSketchParam.ShadePower < 0.0) 
	        IGSketchParam.ShadePower = 0.0;
	    if (IGSketchParam.ShadePower > 1.0) 
	        IGSketchParam.ShadePower = 1.0;

	    sprintf(Label, "Sketch Shd %5.3f", IGSketchParam.ShadePower);
	    SetLabel(SketchShdPowerLabel, Label);

	    IGRedrawViewWindow();
	    break;
        case IG_STATE_SKETCH_IMP_DECAY:
	    IGSketchParam.SketchImpDecay += FDiff;
	    if (IGSketchParam.ShadePower < 0.0) 
	        IGSketchParam.ShadePower = 0.0;

	    sprintf(Label, "Importance %5.3f", IGSketchParam.SketchImpDecay);
	    SetLabel(SketchShdImportanceDecay, Label);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);

	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SKETCH_IMP_FRNT_SPRT:
	    IGSketchParam.SketchImpFrntSprt += FDiff * 10.0;
	    if (IGSketchParam.SketchImpFrntSprt < 0.0) 
	        IGSketchParam.SketchImpFrntSprt = 0.0;
	    if (IGSketchParam.SketchImpFrntSprt > 90.0) 
	        IGSketchParam.SketchImpFrntSprt = 90.0;

	    sprintf(Label, "Frontal Support %4.2f",
		    IGSketchParam.SketchImpFrntSprt);
	    SetLabel(SketchShdImpFrntSprt, Label);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);

	    IGRedrawViewWindow();
	    break;
    }
}  
