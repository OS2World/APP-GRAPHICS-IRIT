/*****************************************************************************
*   A motif interface for picking parameter.				     *
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
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "xmtdrvs.h"

#define PICK_PARAM_FRACTION 6
#define PICK_DISMISS	    1025

IRIT_GLOBAL_DATA Widget PickParamForm;

static void SetCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static void Pick1ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData);
static void Pick2ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData);
static void Pick3ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData);
static void Pick4ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main PickParam window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreatePickParam                                                          M
*****************************************************************************/
void CreatePickParam(Widget IGTopLevel)
{
    IRIT_STATIC_DATA XTC PickCallBack[] = {
        (XTC) Pick1ObjectsCB,
        (XTC) Pick2ObjectsCB,
        (XTC) Pick3ObjectsCB,
        (XTC) Pick4ObjectsCB,
	NULL
    };
    IRIT_STATIC_DATA int PickInitVal[][4] = {
	{   IG_PICK_POLY,
	    IG_PICK_POINT,
	    IG_PICK_VECTOR,
	    IG_PICK_PLANE
	},
	{
	    IG_PICK_CURVE,
	    IG_PICK_SURFACE,
	    IG_PICK_STRING,
	    IG_PICK_CTLPT
	},
	{
	    IG_PICK_TRIMSRF,
	    IG_PICK_TRIVAR,
	    IG_PICK_INSTANCE,
	    0
	},
	{
	    IG_PICK_TRISRF,
	    IG_PICK_MODEL,
	    IG_PICK_MULTIVAR,
	    0
	},

    };
    IRIT_STATIC_DATA char *PickObjects[][5] = {
	{
	    "Poly",
	    "Point",
	    "Vector",
	    "Plane",
	    NULL
	},
	{
	    "Curve",
	    "Surface",
	    "String",
	    "Ctlpt",
	    NULL
	},
	{
	    "Trimmed Srf",
	    "Trivariate",
	    "Instance",
	    NULL
	},
	{
	    "Trisrf",
	    "Model",
	    "Multivariate",
	    NULL
	}
    };
    int ButtonsMask, i, Len,
	Pos = 0;
    Arg args[10];

    XtSetArg(args[0], XmNfractionBase, PICK_PARAM_FRACTION);
    PickParamForm = XmCreateFormDialog(IGTopLevel, "PickParamMenu", args, 1);

    AddLabel(PickParamForm, "Pick Objects", Pos++);

    for (i = 0; PickCallBack[i] != NULL; i++) {
        for (Len = ButtonsMask = 0; PickObjects[i][Len] != NULL; Len++)
	    ButtonsMask |= (IGGlblPickObjTypes &
			    PickInitVal[i][Len]) ? (1 << Len) : 0;
        AddCheckButton(PickParamForm, "Pick Objects", ButtonsMask,
		       PickObjects[i], Len, Pos++, PickCallBack[i]);
    }

    AddButton(PickParamForm, "Dismiss", Pos++, (XTC) SetCB,
	      (XTP) PICK_DISMISS);

    if (PICK_PARAM_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of Pick Param State is not complete (%d).\n",
		Pos);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to pick param commands.	  				     *
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
	case PICK_DISMISS:
	    XtUnmanageChild(PickParamForm);
	    break;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles radio buttons' events.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   Index:     Button index pressed.               		             *
*   CallData:  Holds the Button's value.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Pick1ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData)
{
    int ObjType;

    switch (Index) {
	case 0:
	    ObjType = IG_PICK_POLY;
	    break;
	case 1:
	    ObjType = IG_PICK_POINT;
	    break;
	case 2:
	    ObjType = IG_PICK_VECTOR;
	    break;
	case 3:
	    ObjType = IG_PICK_PLANE;
	    break;
    }

    IG_PICK_SET_TYPE(ObjType, CallData -> set);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles radio buttons' events.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                		     *
*   Index:     Button index pressed.               		             *
*   CallData:  Holds the Button's value.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Pick2ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData)
{
    int ObjType;

    switch (Index) {
	case 0:
	    ObjType = IG_PICK_CURVE;
	    break;
	case 1:
	    ObjType = IG_PICK_SURFACE;
	    break;
	case 2:
	    ObjType = IG_PICK_STRING;
	    break;
	case 3:
	    ObjType = IG_PICK_CTLPT;
	    break;
    }

    IG_PICK_SET_TYPE(ObjType, CallData -> set);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles radio buttons' events.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                 		     *
*   Index:     Button index pressed.               		             *
*   CallData:  Holds the Button's value.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Pick3ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData)
{
    int ObjType;

    switch (Index) {
	case 0:
	    ObjType = IG_PICK_TRIMSRF;
	    break;
	case 1:
	    ObjType = IG_PICK_TRIVAR;
	    break;
	case 2:
	    ObjType = IG_PICK_INSTANCE;
	    break;
    }

    IG_PICK_SET_TYPE(ObjType, CallData -> set);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles radio buttons' events.	  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:         Calling widget.                                		     *
*   Index:     Button index pressed.               		             *
*   CallData:  Holds the Button's value.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Pick4ObjectsCB(Widget w,
			   int Index,
			   XmToggleButtonCallbackStruct *CallData)
{
    int ObjType;

    switch (Index) {
	case 0:
	    ObjType = IG_PICK_TRISRF;
	    break;
	case 1:
	    ObjType = IG_PICK_MODEL;
	    break;
	case 2:
	    ObjType = IG_PICK_MULTIVAR;
	    break;
    }

    IG_PICK_SET_TYPE(ObjType, CallData -> set);
}
