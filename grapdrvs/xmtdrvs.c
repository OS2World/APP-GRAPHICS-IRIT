/*****************************************************************************
*   A motif interface driver.						     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Offer Kaufman and Gershon Elber	    Ver 0.1, November 1994.  *
*****************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xresource.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/Form.h>

/* Old motif/X11 implementation do not define these... */
#ifndef XK_KP_Down
#define XK_KP_Down 0xFF99                      /* So at least it compiles... */
#endif
#ifndef XK_KP_Up
#define XK_KP_Up 0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right 0xFF98
#endif
#ifndef XK_KP_Left
#define XK_KP_Left 0xFF96
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "editsrfs.h"
#include "editmanp.h"
#include "xmtdrvs.h"

#define DEFAULT_TRANS_WIDTH	200
#define DEFAULT_TRANS_HEIGHT	500

#define TRANS_FRACTION		28
#define STATE_FRACTION		27

IRIT_STATIC_DATA XColor
    *TransCursorColor = NULL;
IRIT_STATIC_DATA int
    GlblPopupWidgetDone = TRUE,
    GlblViewModeFileSave = 0,
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    TransPosX = 0,
    TransPosY = 0;
IRIT_STATIC_DATA unsigned int
    TransWidth = DEFAULT_TRANS_WIDTH,
    TransHeight = DEFAULT_TRANS_HEIGHT;
IRIT_STATIC_DATA XtAppContext
    IGApplication;

/* X Colors to be used for viewed object (see also grap_lib/drvs.h): */
IRIT_STATIC_DATA int XViewColorDefs[IG_MAX_COLOR + 1][3] =
{
    {     0,     0,     0 },  /* 0. IG_IRIT_BLACK */
    {     0,     0, 43350 },  /* 1. IG_IRIT_BLUE */
    {     0, 43350,     0 },  /* 2. IG_IRIT_GREEN */
    {     0, 43350, 43350 },  /* 3. IG_IRIT_CYAN */
    { 43350,     0,     0 },  /* 4. IG_IRIT_RED */
    { 43350,     0, 43350 },  /* 5. IG_IRIT_MAGENTA */
    { 43350, 43350,     0 },  /* 6. IG_IRIT_BROWN */
    { 43350, 43350, 43350 },  /* 7. IG_IRIT_LIGHTGREY */
    { 21675, 21675, 21675 },  /* 8. IG_IRIT_DARKGRAY */
    { 21675, 21675, 65535 },  /* 9. IG_IRIT_LIGHTBLUE */
    { 21675, 65535, 21675 },  /* 10. IG_IRIT_LIGHTGREEN */
    { 21675, 65535, 65535 },  /* 11. IG_IRIT_LIGHTCYAN */
    { 65535, 21675, 21675 },  /* 12. IG_IRIT_LIGHTRED */
    { 65535, 21675, 65535 },  /* 13. IG_IRIT_LIGHTMAGENTA */
    { 65535, 65535, 21675 },  /* 14. IG_IRIT_YELLOW */
    { 65535, 65535, 65535 }   /* 15. IG_IRIT_WHITE */
};

IRIT_STATIC_DATA IrtRType OldSliderVal;
IRIT_STATIC_DATA Widget ProjectionButton, DrawSurfacePolyButton, StateForm,
    LineWidthLabel, ChangeFactorLabel, LowResFactorLabel,
    PolylineFineNessLabel, PolygonFineNessLabel, NormalSizeLabel,
    NumOfIsolinesLabel;

/* X global specific staff goes here: */
IRIT_GLOBAL_DATA Display
    *IGXDisplay = NULL;
IRIT_GLOBAL_DATA int 
    IGXScreen,
    IGViewHasSize,
    IGViewHasPos,
    IGViewPosX,
    IGViewPosY;
IRIT_GLOBAL_DATA unsigned int
    IGMaxColors = IG_MAX_COLOR,
    IGViewBorderWidth,
    IGViewBackGroundPixel,
    IGViewBorderPixel,
    IGViewTextPixel,
    IGViewWidth = DEFAULT_VIEW_WIDTH,
    IGViewHeight = DEFAULT_VIEW_HEIGHT;
IRIT_GLOBAL_DATA Colormap IGXColorMap;
IRIT_GLOBAL_DATA GC IGXViewGraphContext;
IRIT_GLOBAL_DATA Widget IGTopLevel, MainTransForm;
IRIT_GLOBAL_DATA Window IGXRoot, IGViewWndw;
IRIT_GLOBAL_DATA XColor IGBlackColor,
    *IGViewCursorColor = NULL;
IRIT_GLOBAL_DATA XColor IGXViewColorsHigh[IG_MAX_COLOR + 1],
		        IGXViewColorsLow[IG_MAX_COLOR + 1];

static char *ReadOneXDefault(char *Entry);
static void ReadXDefaults(void);
static void RadioButtonCB(Widget w, int Index);
static void TransformCB(Widget w, IGGraphicEventType state);
static void SetCB(Widget w, int State, XmScaleCallbackStruct *CallData);
static int ExposeCB(Widget w); 
static void SaveImageFileCB(Widget w,
			    caddr_t ClientData,
			    XmFileSelectionBoxCallbackStruct *FileStruct);
static void ScaleCB(Widget w, IGGraphicEventType EventType);
static void DragCB(Widget w, IGGraphicEventType EventType);
static void SaveMatrixFileCB(Widget w,
			     caddr_t ClientData,
			     XmFileSelectionBoxCallbackStruct *FileStruct);
static void IGIritErrorOkCb(Widget Dialog);
static void IGIritErrorCancelCb(Widget Dialog);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pop up all windows, read input and display.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  command line parameters ( *.itd files )                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main					       			     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int i;
    XGCValues Values;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("x11drvs", argc, argv);

    /* Must select a function to draw polys. */
    IGDrawPolyFuncPtr = IGDrawPoly;

    /* Lets see if we can get access to the X server before we even start: */
    if ((IGXDisplay = (Display *) XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr, "%s: Failed to access X server, aborted.\n", argv[0]);
        exit(-1);
    }

    IGXScreen = DefaultScreen(IGXDisplay);
    IGXRoot = RootWindow(IGXDisplay, IGXScreen);
    IGXColorMap = DefaultColormap(IGXDisplay, IGXScreen);
    Values.foreground = WhitePixel(IGXDisplay, IGXScreen);
    Values.background = BlackPixel(IGXDisplay, IGXScreen);
    IGXViewGraphContext = XCreateGC(IGXDisplay, IGXRoot,
				    GCForeground | GCBackground, &Values);
    
    if (XrmGetDatabase(IGXDisplay) == NULL)
	XGetDefault(IGXDisplay, "", "");
    ReadXDefaults();

    for (i = 0; i <= IG_MAX_COLOR; i++) {
	IGXViewColorsHigh[i].red   = XViewColorDefs[i][0];
	IGXViewColorsHigh[i].green = XViewColorDefs[i][1];
	IGXViewColorsHigh[i].blue  = XViewColorDefs[i][2];

	/* If fails to allocate the color - take WHITE instead. */
	if (!XAllocColor(IGXDisplay, IGXColorMap, &IGXViewColorsHigh[i]))
	    IGXViewColorsHigh[i].pixel = WhitePixel(IGXDisplay, IGXScreen);

	IGXViewColorsLow[i].red   = (XViewColorDefs[i][0] >> 1);
	IGXViewColorsLow[i].green = (XViewColorDefs[i][1] >> 1);
	IGXViewColorsLow[i].blue  = (XViewColorDefs[i][2] >> 1);

	/* If fails to allocate the color - take WHITE instead. */
	if (!XAllocColor(IGXDisplay, IGXColorMap, &IGXViewColorsLow[i]))
	    IGXViewColorsLow[i].pixel = WhitePixel(IGXDisplay, IGXScreen);
    }

#if defined(__OPENGL__)
    {
	IRIT_STATIC_DATA String FallbackResources[] = {
	    "*glwidget*rgba:		   TRUE",
	    "*glwidget*doublebuffer:	   TRUE",
	    "*glwidget*allocateBackground: TRUE",
	    "*glwidget*depthSize:	   8",
	    "*fontList:-*-times-*-r-*-*-12-*-*-*-*-*-*-*",
	    "*XmLabel.fontList:-*-times-*-i-*-*-12-*-*-*-*-*-*-*",
	    NULL
	};
#else
    {
	IRIT_STATIC_DATA String FallbackResources[] = { 
	    "*fontList:-*-times-*-r-*-*-12-*-*-*-*-*-*-*",
	    "*XmLabel.fontList:-*-times-*-i-*-*-12-*-*-*-*-*-*-*",
	    NULL
	};
#endif /* __OPENGL__*/

	IGTopLevel = XtVaAppInitialize(&IGApplication, argv[0], NULL, 0,
				       &argc, argv, FallbackResources,
				       NULL, 0);
    }

    CreateControlPanel(IGTopLevel);
    CreatePopupWindow(MainTransForm);
    CreateAnimation(MainTransForm);
    CreateCrvEdit(MainTransForm);
    CreateSrfEdit(MainTransForm);
    CreateShadeParam(MainTransForm);
    CreatePickParam(MainTransForm);
    CreateObjManipParam(MainTransForm);

    SetViewWindow(argc, argv);

    sleep(1); /* Some systems get confused if we draw immediately. */

    XtAppAddWorkProc(IGApplication, (XtWorkProc) ExposeCB, NULL);

    XtVaSetValues(IGTopLevel,
		  XmNwidth,  TransWidth,
		  XmNheight, TransHeight,
		  XmNx,      TransPosX,
		  XmNy,      TransPosY,
		  NULL);
    XtRealizeWidget(IGTopLevel);

    /* Starts with the selected set of initial widgets. */
    if (IGGlblInitWidgetDisplay & IG_WIDGET_ENVIRONMENT)
	TransformCB(IGTopLevel, IG_EVENT_STATE);
    if (IGGlblInitWidgetDisplay & IG_WIDGET_ANIMATION)
	AnimationCB();
    if (IGGlblInitWidgetDisplay & IG_WIDGET_CURVES)
	TransformCB(IGTopLevel, IG_EVENT_CRV_EDIT);
    if (IGGlblInitWidgetDisplay & IG_WIDGET_SURFACES)
	TransformCB(IGTopLevel, IG_EVENT_SRF_EDIT);
    if (IGGlblInitWidgetDisplay & IG_WIDGET_SHADING)
	TransformCB(IGTopLevel, IG_EVENT_SHADE_PARAM);
    if (IGGlblInitWidgetDisplay & IG_WIDGET_PICK_OBJS)
	TransformCB(IGTopLevel, IG_EVENT_PICK_OBJS);
    if (IGGlblInitWidgetDisplay & IG_WIDGET_OBJS_TRANS)
	TransformCB(IGTopLevel, IG_EVENT_OBJ_MANIP);

    if (IGCrvEditPreloadEditCurveObj != NULL) {
	IGPopupCrvEditor(IGCrvEditPreloadEditCurveObj);
	IGCrvEditPreloadEditCurveObj = NULL;
    }
    if (IGSrfEditPreloadEditSurfaceObj != NULL) {
	IGPopupSrfEditor(IGSrfEditPreloadEditSurfaceObj);
	IGSrfEditPreloadEditSurfaceObj = NULL;
    }

    XStoreName(IGXDisplay, IGViewWndw, IGGenerateWindowHeaderString(NULL));

    XtAppMainLoop(IGApplication);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the curve editor, if not already up and hook CrvObj to it.         M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to edit.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupCrvEditor                                                         M
*****************************************************************************/
void IGPopupCrvEditor(IPObjectStruct *CrvObj)
{
    TransformCB(IGTopLevel, IG_EVENT_CRV_EDIT);
    if (CrvObj != NULL)
	CEditAttachOldDirectly(CrvObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the surface editor, if not already up and hook SrfObj to it.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to edit.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupSrfEditor                                                         M
*****************************************************************************/
void IGPopupSrfEditor(IPObjectStruct *SrfObj)
{
    TransformCB(IGTopLevel, IG_EVENT_SRF_EDIT);
    if (SrfObj != NULL)
	SEditAttachOldDirectly(SrfObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the object editor, if not already up and hook PObj to it.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to edit.                                                 M
*   CloneIt: If TRUE make a copy of given object fist and edit the clone.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupObjEditor                                                         M
*****************************************************************************/
void IGPopupObjEditor(IPObjectStruct *PObj, int CloneIt)
{
    TransformCB(IGTopLevel, IG_EVENT_OBJ_MANIP);
    if (PObj != NULL)
        IGObjManipAttachOldDirectly(PObj, CloneIt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handles the state events.		 				     M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       State to be treated.				             M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*	         on or toggling current value. 				     M
*	         IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Wether state change requires screen refreshing.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      UpdateView       				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    IGHandleState				       			     M
*****************************************************************************/
int IGHandleState(int State, int StateStatus, int Refresh)
{
    int UpdateView = TRUE;

    UpdateView = IGStateHandler(State, StateStatus, Refresh);

    XtDestroyWidget(StateForm); 
    CreatePopupWindow(IGTopLevel);

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Motif Based Control Panel. 					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TopLevel:  Top level widget.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    CreateControlPanel				       			     M
*****************************************************************************/
void CreateControlPanel(Widget TopLevel)
{
    IRIT_STATIC_DATA char *SlideTitle[] = {
	" PERSPECTIVE",
	" ROTATE",
	" TRANSLATE",
	" SCALE",
	" FAR/NEAR CLIP",
	" MATRIX",
	" EXTENSIONS"
    };
    IRIT_STATIC_DATA char SlideLabel[][2] = {
	"Z",
	"X",
	"Y",
	"Z",
	"X",
	"Y",
	"Z",
	" ",
	"N",
	"F"
    };    
    Widget Form, SubForm, Label, Button, Scale, TransForm;
    int i,
	Pos = 0,
	t = 0;
    IGGraphicEventType EventType;

    MainTransForm = XtVaCreateManagedWidget("TransWindow",
					    xmMainWindowWidgetClass, TopLevel,
					    NULL);
    TransForm = XtVaCreateManagedWidget("TransForm",
					xmFormWidgetClass,   MainTransForm,
					XmNwidth,            TransWidth,
					XmNheight,           TransHeight,
					XmNtopAttachment,    XmATTACH_WIDGET,
					XmNtopWidget, 	     MainTransForm,
					XmNbottomAttachment, XmATTACH_WIDGET,
					XmNbottomWidget,     MainTransForm,  
					XmNleftAttachment,   XmATTACH_WIDGET,
					XmNleftWidget, 	     MainTransForm,
					XmNrightAttachment,  XmATTACH_WIDGET,
					XmNrightWidget,      MainTransForm,
					XmNfractionBase,     TRANS_FRACTION,
					NULL);

    /* Construct all slide bars (Scales) */
    for (i = 0, EventType = IG_EVENT_PERS_ORTHO_Z; i < 10; i++, EventType++) {
	if (EventType == IG_EVENT_ROTATE || EventType == IG_EVENT_TRANSLATE)
	    EventType++;			       /* Skip these events. */
        if (i == 0 || i == 1 || i == 4 || i == 7 || i == 8) {
            Form = CreateSubForm(TransForm, Pos++); 
            Label = XtVaCreateManagedWidget(SlideTitle[t++],
					    xmLabelWidgetClass,  Form,
					    XmNleftAttachment,   XmATTACH_FORM,
					    XmNrightAttachment,  XmATTACH_FORM,
					    NULL);              
        }
        Form = CreateSubForm(TransForm, Pos++); 	
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
    
    Form = CreateSubForm(TransForm, Pos++); 
    Label = XtVaCreateManagedWidget(SlideTitle[t++],
				    xmLabelWidgetClass,  	Form,
				    XmNleftAttachment,  	XmATTACH_FORM,
				    XmNrightAttachment,  	XmATTACH_FORM,
				    NULL);
 
    Form = CreateSubForm(TransForm, Pos++);         
    SubForm = XtVaCreateManagedWidget("MatrixOpFoundation",
				      xmFormWidgetClass,  	Form,
				      XmNleftAttachment,  	XmATTACH_FORM,
				      XmNrightAttachment, 	XmATTACH_FORM,
				      XmNfractionBase,   	4,
				      NULL);
               
    Button = XtVaCreateManagedWidget("Save",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_SAVE_MATRIX);
 
    Button = XtVaCreateManagedWidget("Submit",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_SUBMIT_MATRIX);
 
    Button = XtVaCreateManagedWidget("Push",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        2,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       3,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_PUSH_MATRIX);
 
    Button = XtVaCreateManagedWidget("Pop",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        3,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       4,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_POP_MATRIX);

    Form = CreateSubForm(TransForm, Pos++);
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,   	Form,
				      XmNrightAttachment,  	XmATTACH_FORM,
				      XmNleftAttachment,   	XmATTACH_FORM,
				      XmNfractionBase,     	2,
				      NULL);

    Button = XtVaCreateManagedWidget("Top",
				     xmPushButtonWidgetClass,SubForm,
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_VIEW_TOP);

    Button = XtVaCreateManagedWidget("Side",
				     xmPushButtonWidgetClass,SubForm,
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition, 	     1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition, 	     2,
				     NULL);

    Form = CreateSubForm(TransForm, Pos++);
    SubForm = XtVaCreateManagedWidget("Subform",
				      xmFormWidgetClass, 	Form,
				      XmNrightAttachment,	XmATTACH_FORM,
				      XmNleftAttachment, 	XmATTACH_FORM,
				      XmNfractionBase, 		2,
				      NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_VIEW_SIDE);

    Button = XtVaCreateManagedWidget("Front",
				     xmPushButtonWidgetClass,SubForm,
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition, 	     0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition, 	     1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_VIEW_FRONT);

    Button = XtVaCreateManagedWidget("Iso",
				     xmPushButtonWidgetClass,SubForm,
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition, 	     1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition, 	     2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_VIEW_ISOMETRY);

    Form = CreateSubForm(TransForm, Pos++); 
    Label = XtVaCreateManagedWidget(SlideTitle[t++],
				    xmLabelWidgetClass,  	Form,
				    XmNleftAttachment,  	XmATTACH_FORM,
				    XmNrightAttachment,  	XmATTACH_FORM,
				    NULL);
     Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Animation",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) AnimationCB, (XTP) IG_EVENT_ANIMATION);
    Button = XtVaCreateManagedWidget("Shading",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_SHADE_PARAM);

    Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Environment",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_STATE);
    Button = XtVaCreateManagedWidget("Crv Edit",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_CRV_EDIT);

    Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Set Pick Objs",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_PICK_OBJS);
    Button = XtVaCreateManagedWidget("Srf Edit",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_SRF_EDIT);

    Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Object Manip",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_OBJ_MANIP);

    Pos++;
    Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Save Image",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) SetCB, (XTP) IG_STATE_SAVE_IMAGE);

    Pos++;
    Form = CreateSubForm(TransForm, Pos++);        
    SubForm = XtVaCreateManagedWidget("SubForm",
				      xmFormWidgetClass,      Form,
				      XmNrightAttachment,     XmATTACH_FORM,
				      XmNleftAttachment,      XmATTACH_FORM,
				      XmNfractionBase,        2,
				      NULL);
    Button = XtVaCreateManagedWidget("Quit",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        0,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       1,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_QUIT);
    Button = XtVaCreateManagedWidget("Disconnect",
				     xmPushButtonWidgetClass,SubForm, 
				     XmNleftAttachment,      XmATTACH_POSITION,
				     XmNleftPosition,        1,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,       2,
				     NULL);
    XtAddCallback(Button, XmNactivateCallback,
		  (XTC) TransformCB, (XTP) IG_EVENT_DISCONNECT);

    if (TRANS_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of Transformation is not complete (%d).\n",
		Pos);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Create and activate pop up window.  				     M
*                                                                            *
* PARAMETERS:                                                                M
*   w:  Calling widget.                                  		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreatePopupWindow                                                        M
*****************************************************************************/
void CreatePopupWindow(Widget w)
{
    IRIT_STATIC_DATA char *ShadingTitles[] = {
	"Nn",
	"Bg",
	"Fl",
	"Gd",
	"Pg"
    };
    int DefaultShading = 4,
	Pos = 0;
    char NewLabel[30], *Str, *Str2;
    Arg args[10];
    Widget Button;

    XtSetArg(args[0], XmNfractionBase, STATE_FRACTION);
    StateForm = XmCreateFormDialog(w, "StateMenu", args, 1);

    AddThreeButtons(StateForm, Pos++,
		    IGGlblTransformMode == IG_TRANS_OBJECT ?
					      "Objt Trans" : "Scrn Trans",
		    (XTC) TransformCB, (XTP) IG_EVENT_SCR_OBJ_TGL,
		    IGGlblContinuousMotion ? "Cont Motion"
					   : "Rglr Motion",
		    (XTC) TransformCB, (XTP) IG_EVENT_CONT_MOTION,
		    IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				     "Persptive" : "Orthographic",
		    (XTC) TransformCB, (XTP) IG_EVENT_PERS_ORTHO_TGL,
		    &Button, &Button, &ProjectionButton);

    switch (IGGlblDrawStyle) {
	case IG_STATE_DRAW_STYLE_WIREFRAME:
	    Str = "Draw Wireframe";
	    break;
	case IG_STATE_DRAW_STYLE_SOLID:
	    Str = "Draw Solid";
	    break;
	case IG_STATE_DRAW_STYLE_POINTS:
	    Str = "Draw Points";
	    break;
    }
    switch (IGGlbl3DGlassesMode) {
	case IG_GLASSES_3D_NONE:
        default:
	    Str2 = "No 3D Glasses";
	    break;
	case IG_GLASSES_3D_CHROMADEPTH:
	    Str2 = "Chroma Depth";
	    break;
        case IG_GLASSES_3D_RED_BLUE:
	    Str2 = "Red Blue";
	    break;
        case IG_GLASSES_3D_RED_GREEN:
	    Str2 = "Red Green";
	    break;
    }
    AddThreeButtons(StateForm, Pos++,
		    Str, (XTC) SetCB, (XTP) IG_STATE_DRAW_STYLE,
		    IGGlblDepthCue ? "Depth Cue" : "No Depth Cue",
		    (XTC) TransformCB, (XTP) IG_EVENT_DEPTH_CUE,
		    Str2, (XTC) TransformCB, (XTP) IG_EVENT_3D_GLASSES,
		    &Button, &Button, &Button);

    switch (IGGlblShadingModel) {
	case IG_SHADING_NONE:
	    DefaultShading = 0;
	    break;
	case IG_SHADING_BACKGROUND:
	    DefaultShading = 1;
	    break;
	case IG_SHADING_FLAT:
	    DefaultShading = 2;
	    break;
	case IG_SHADING_GOURAUD:
	    DefaultShading = 3;
	    break;
	case IG_SHADING_PHONG:
	    DefaultShading = 4;
	    break;
    }
    AddLabel(StateForm, "Shading", Pos++);
    AddRadioButton(StateForm, "Shading", DefaultShading, ShadingTitles, 
		   5, Pos++, (XTC) RadioButtonCB);

    AddTwoButtons(StateForm, Pos++,
		  IGGlblBackFaceCull ? "Cull BFace" : "No Cull BFace",
		  (XTC) SetCB, (XTP) IG_STATE_BACK_FACE_CULL,
		  IGGlblDrawInternal ? "Intrnl Edges" : "No Intrnl Edges",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_INTERNAL,
		  &Button, &Button);

    switch (IGGlblAntiAliasing) {
	case IG_STATE_ANTI_ALIAS_OFF:
	    Str = "No Anti Aliasing";
	    break;
	case IG_STATE_ANTI_ALIAS_ON:
	    Str = "Anti Aliasing";
	    break;
	case IG_STATE_ANTI_ALIAS_BLEND:
	    Str = "Blending";
	    break;
    }
    AddTwoButtons(StateForm, Pos++,
		  IGGlblDoDoubleBuffer ? "Double Buffer" : "Single Buffer",
		  (XTC) SetCB, (XTP) IG_STATE_DOUBLE_BUFFER,
		  Str, (XTC) SetCB, (XTP) IG_STATE_ANTI_ALIASING,
		  &Button, &Button);

    AddThreeButtons(StateForm, Pos++,
		    IGGlblDrawVNormal ? "VNormals" : "No VNormals",
		    (XTC) SetCB, (XTP) IG_STATE_DRAW_VNORMAL,
		    IGGlblDrawPNormal ? "PNormals" : "No PNormals",
		    (XTC) SetCB, (XTP) IG_STATE_DRAW_PNORMAL,
		    IGGlblFlipNormalOrient ? "Rvrsd Normals" : "Rglr Normals",
		    (XTC) SetCB, (XTP) IG_STATE_NRML_ORIENT,
		    &Button, &Button, &Button);

    AddTwoButtons(StateForm, Pos++,
		  IGGlblDrawSurfaceMesh ? "Srf Mesh" : "No Srf Mesh",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_MESH,
		  IGGlblDrawSurfacePoly ? "Srf Polys" : "No Srf Polys",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_POLY,
		  &Button, &DrawSurfacePolyButton);

    AddTwoButtons(StateForm, Pos++,
		  IGGlblDrawSurfaceWire ? "Srf Isos" : "No Srf Isos",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_WIRE,
		  IGGlblDrawSurfaceSketch ? "Srf Sktch" : "No Srf Sktch",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_SKTCH,
		  &Button, &Button);

    AddTwoButtons(StateForm, Pos++,
		  IGGlblDrawSurfaceBndry ? "Srf Bndry" : "No Srf Bndry",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_BNDRY,
		  IGGlblDrawSurfaceSilh ? "Srf Silh" : "No Srf Silh",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_SILH,
		  &Button, &Button);

    AddThreeButtons(StateForm, Pos++,
		    IGGlblDrawSurfaceRflctLns ? "Rflct Lns" : "No Rflct Lns",
		    (XTC) SetCB, (XTP) IG_STATE_DRAW_SRF_RFLCT_LNS,
		    IGGlblCountNumPolys ? "Count #Polys" : "No #Polys",
		    (XTC) SetCB, (XTP) IG_STATE_NUM_POLY_COUNT,
		    IGGlblCountNumPolys ? "Count FPS" : "No FPS",
		    (XTC) SetCB, (XTP) IG_STATE_FRAME_PER_SEC,
		    &Button, &Button, &Button);

    AddTwoButtons(StateForm, Pos++,
		  IGGlblFourPerFlat ? "Four Per Flat" : "Two Per Flat",
		  (XTC) SetCB, (XTP) IG_STATE_FOUR_PER_FLAT,
		  IGGlblDrawPolygons ? "Polygons" : "No Polygons",
		  (XTC) SetCB, (XTP) IG_STATE_DRAW_POLYGONS,
		  &Button, &Button);

    sprintf(NewLabel, "Normal Len %5.4f", IGGlblNormalSize);
    NormalSizeLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddSlide(StateForm, Pos++, -100, 100,
	     (XTC) SetCB, (XTC) SetCB,(XTP) IG_STATE_LENGTH_VECTORS);

    sprintf(NewLabel, "Lines Width %d", IGGlblLineWidth);
    LineWidthLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddSlide(StateForm, Pos++, -20, 20,
	     (XTC) SetCB, (XTC) SetCB, (XTP) IG_STATE_WIDTH_LINES);

    sprintf(NewLabel, "Sensitivity %2.2f", IGGlblChangeFactor);
    ChangeFactorLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddSlide(StateForm, Pos++, -100, 100,
	     (XTC) SetCB, (XTC) SetCB, (XTP) IG_STATE_MOUSE_SENSITIVE);
    sprintf(NewLabel, "Lowres ratio %6.4f", IGGlblRelLowresFineNess);
    LowResFactorLabel = AddLabel(StateForm, NewLabel, Pos++);
    XtVaSetValues(AddSlide(StateForm, Pos++, 0, 1000,
			   (XTC) SetCB,(XTC) SetCB,
			   (XTP) IG_STATE_LOWRES_RATIO),
		  XmNvalue, (int) (IGGlblRelLowresFineNess * 1000.0), NULL);

    sprintf(NewLabel, "Isolines %3d", IGGlblNumOfIsolines);
    NumOfIsolinesLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddSlide(StateForm, Pos++, -1000, 1000,
	     (XTC) SetCB, (XTC) SetCB, (XTP) IG_STATE_NUM_ISOLINES);

    sprintf(NewLabel, "Polyline FineNess %.5g", IGGlblPllnFineness);
    PolylineFineNessLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddButtonSlide(StateForm, Pos++,
		   IGGlblPolylineOptiApprox == 0 ? "Uniform" : "Optimal",
		   (XTC) SetCB, (XTP) IG_STATE_POLYLINE_OPTI,
		   -1000, 1000, (XTC) SetCB, (XTC) SetCB,
		   (XTP) IG_STATE_SAMP_PER_CRV_APPROX);

    sprintf(NewLabel, "Polygon FineNess %.5g", IGGlblPlgnFineness);
    PolygonFineNessLabel = AddLabel(StateForm, NewLabel, Pos++);
    AddButtonSlide(StateForm, Pos++,
		   IGGlblPolygonOptiApprox == 0 ? "Uniform" : "Optimal",
		   (XTC) SetCB, (XTP) IG_STATE_POLYGON_OPTI,
		   -1000, 1000, (XTC) SetCB, (XTC) SetCB,
		   (XTP) IG_STATE_POLY_APPROX);

    AddTwoButtons(StateForm, Pos++,
		  "Clear View",
		  (XTC) SetCB, (XTP) IG_STATE_CLEAR_VIEW,
		  "Dismiss",
		  (XTC) SetCB, (XTP) IG_STATE_OOPS,
		  &Button, &Button);

    if (STATE_FRACTION < Pos)
	fprintf(stderr,
		"Initialization of Popup State is not complete (%d).\n",
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
static void RadioButtonCB(Widget w, int Index)
{
    switch (Index) {
	case 0:
	    if (IGGlblShadingModel == IG_SHADING_NONE)
	        return;
	    IGGlblShadingModel = IG_SHADING_NONE;
	    break;
	case 1:
	    if (IGGlblShadingModel == IG_SHADING_BACKGROUND)
	        return;
	    IGGlblShadingModel = IG_SHADING_BACKGROUND;
	    break;
	case 2:
	    if (IGGlblShadingModel == IG_SHADING_FLAT)
	        return;
	    IGGlblShadingModel = IG_SHADING_FLAT;
	    break;
	case 3:
	    if (IGGlblShadingModel == IG_SHADING_GOURAUD)
	        return;
	    IGGlblShadingModel = IG_SHADING_GOURAUD;
	    break;
	case 4:
	    if (IGGlblShadingModel == IG_SHADING_PHONG)
	        return;
	    IGGlblShadingModel = IG_SHADING_PHONG;
	    break;
    }
    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles transformation window Buttons.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:      Calling widget.                                  		     *
*   State:  State represented by the widget.			             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TransformCB(Widget w, IGGraphicEventType State)
{
    Position MainWindowX, MainWindowY;
    Dimension MainWindowW;
    IrtRType ChangeFactor[2];
    char *Str;

    switch (State) {
	case IG_EVENT_DEPTH_CUE:
	    IGGlblDepthCue = !IGGlblDepthCue;
	    SetLabel(w, IGGlblDepthCue ? "Depth Cue" : "No Depth Cue");
	    IGRedrawViewWindow();
	    break;
        case IG_EVENT_3D_GLASSES:
	    switch (IGGlbl3DGlassesMode) {
	        case IG_GLASSES_3D_NONE:
	        default:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_CHROMADEPTH;
		    Str = "Chroma Depth";
		    break;
	        case IG_GLASSES_3D_CHROMADEPTH:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_BLUE;
		    Str = "Red Blue";
		    break;
	        case IG_GLASSES_3D_RED_BLUE:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_GREEN;
		    Str = "Red Green";
		    break;
	        case IG_GLASSES_3D_RED_GREEN:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_NONE;
		    Str = "No 3D Glasses";
		    break;
	    }
	    SetLabel(w, Str);
	    IGRedrawViewWindow();
	    break;
	case IG_EVENT_SCR_OBJ_TGL:
	    IGGlblTransformMode = IGGlblTransformMode == IG_TRANS_OBJECT ? 
					    IG_TRANS_SCREEN : IG_TRANS_OBJECT;
	    SetLabel(w, IGGlblTransformMode == IG_TRANS_SCREEN ? "Scrn Trans" 
							       : "Objt Trans");
	    break;
	case IG_EVENT_CONT_MOTION:
	    IGGlblContinuousMotion = !IGGlblContinuousMotion;
	    SetLabel(w, IGGlblContinuousMotion ? "Cont Motion" : "Rglr Motion");
	    break;
	case IG_EVENT_NRML_ORIENT:
	    IGGlblFlipNormalOrient = !IGGlblFlipNormalOrient;
	    SetLabel(w, IGGlblFlipNormalOrient ? "Rvrsd Normals" : "Rglr Normals");
	    break;
	case IG_EVENT_PERS_ORTHO_TGL:
	    IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				   IG_VIEW_ORTHOGRAPHIC : IG_VIEW_PERSPECTIVE;
	    IGRedrawViewWindow();
	    SetLabel(w, IGGlblViewMode == IG_VIEW_PERSPECTIVE ? "Perspective"
							      : "Orthographic");
	    break;
	case IG_EVENT_STATE:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(StateForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(StateForm); 
	    break;	
	case IG_EVENT_SHADE_PARAM:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(ShadeParamForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(ShadeParamForm); 
	    break;	
	case IG_EVENT_CRV_EDIT:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(CrvEditForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(CrvEditForm); 
	    break;	
	case IG_EVENT_SRF_EDIT:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(SrfEditForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(SrfEditForm); 
	    break;	
	case IG_EVENT_PICK_OBJS:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(PickParamForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(PickParamForm); 
	    break;	
	case IG_EVENT_OBJ_MANIP:
	    XtVaGetValues(IGTopLevel,
			  XmNwidth, &MainWindowW,
			  XmNx,     &MainWindowX,
			  XmNy,     &MainWindowY,
			  NULL);
	    XtVaSetValues(ObjManipParamForm,
			  XmNdefaultPosition, FALSE,
			  XmNx,               MainWindowX + MainWindowW + 16,
			  XmNy,               MainWindowY,
			  NULL); 
	    XtManageChild(ObjManipParamForm); 
	    break;	
	case IG_EVENT_DISCONNECT:
	    if (IGGlblIOHandle >= 0)
	        IPCloseStream(IGGlblIOHandle, TRUE);
	    IGGlblStandAlone = TRUE;
	    break;
	case IG_EVENT_QUIT:
	    if (IGGlblIOHandle >= 0)
	        IPCloseStream(IGGlblIOHandle, TRUE);
	    exit(0);
	    break;
	default:
	    ChangeFactor[0] = ChangeFactor[1] = 1.0;
	    if (IGProcessEvent(State, ChangeFactor)) 
		IGRedrawViewWindow();
    }
}       

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Treat exposure event.		  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:  Calling widget.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   FALSE:  Keep the working function alive.   			             *
*****************************************************************************/
static int ExposeCB(Widget w)
{
    XEvent Event;
    XWindowAttributes WinAttr;

    if (!IGGlblStandAlone &&
        IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList))
        IGRedrawViewWindow();

    IGHandleInternalEvents();

    if (XPending(IGXDisplay)) {
	XNextEvent(IGXDisplay, &Event);

	switch (Event.type) {
	    case Expose:      /* Get rid of all Expose events in the queue. */
		while (XCheckWindowEvent(IGXDisplay, Event.xbutton.window,
					 ExposureMask, &Event));
		if (Event.xbutton.window == IGViewWndw) {
		    XGetWindowAttributes(IGXDisplay, IGViewWndw, &WinAttr);
		    IGViewWidth = WinAttr.width;
		    IGViewHeight = WinAttr.height;
		    IGRedrawViewWindow();
		}
		break;
	    case ResizeRequest:/* Get rid of all Resize events in the queue.*/
		while (XCheckWindowEvent(IGXDisplay, Event.xbutton.window,
					 ResizeRedirectMask, &Event));
		if (Event.xbutton.window == IGViewWndw) {
		    XGetWindowAttributes(IGXDisplay, IGViewWndw, &WinAttr);
		    IGViewWidth = WinAttr.width;
		    IGViewHeight = WinAttr.height;
		    IGRedrawViewWindow();
		}
		break;
	    default:
		if (Event.xbutton.window == IGViewWndw)
		    ViewWndwInputHandler(&Event);
		break;
	}
    }

    if (IGGlblContinuousMotion)
	IGHandleContinuousMotion();

    IritSleep(10);

    return FALSE;     /* Make sure it will be called next time we are idle. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Responds to settings commands.	  				     *
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
    int Value, NewValue;
    char *Str, NewLabel[30];

    switch (State) {
	case IG_STATE_LENGTH_VECTORS: 
            XmScaleGetValue(w, &NewValue);
	    IGGlblNormalSize *= (float) exp(IGGlblChangeFactor *
					    (NewValue - OldSliderVal) / 75);
	    IGGlblPointSize *= (float) exp(IGGlblChangeFactor *
					   (NewValue - OldSliderVal) / 75);
	    sprintf(NewLabel, "Normal Len %5.4f", IGGlblNormalSize);
	    SetLabel(NormalSizeLabel, NewLabel);
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_OOPS:
	    XtUnmanageChild(StateForm); 
	    break;
	case IG_STATE_MOUSE_SENSITIVE:
	    XmScaleGetValue(w, &NewValue);
	    IGGlblChangeFactor *= (float) exp((NewValue - OldSliderVal) / 75);
	    sprintf(NewLabel, "Sensitivity %2.2f", IGGlblChangeFactor);
	    SetLabel(ChangeFactorLabel, NewLabel);
	    break;
	case IG_STATE_LOWRES_RATIO:
	    XmScaleGetValue(w, &NewValue);
	    IGGlblRelLowresFineNess = NewValue / 1000.0;
	    sprintf(NewLabel, "Lowres ratio %6.4f", IGGlblRelLowresFineNess);
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, TRUE, TRUE, FALSE);
	    IGRedrawViewWindow();
	    SetLabel(LowResFactorLabel, NewLabel);
	    return;		      /* and leave the slider where it was. */ 
	case IG_STATE_SHADING_MODEL:
	    switch (IGGlblShadingModel) {
		case IG_SHADING_NONE:
		    IGGlblShadingModel = IG_SHADING_BACKGROUND;
		    SetLabel(w, "Background Shading");
		    break;
		case IG_SHADING_BACKGROUND:
		    IGGlblShadingModel = IG_SHADING_FLAT;
		    SetLabel(w, "Flat Shading");
		    break;
		case IG_SHADING_FLAT:
		    IGGlblShadingModel = IG_SHADING_GOURAUD;
		    SetLabel(w, "Gouraud Shading");
		    break;
		case IG_SHADING_GOURAUD:
		    IGGlblShadingModel = IG_SHADING_PHONG;
		    SetLabel(w, "Phong Shading");
		    break;
		case IG_SHADING_PHONG:
		    IGGlblShadingModel = IG_SHADING_NONE;
		    SetLabel(w, "No Shading");
		    break;
	    }
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_STYLE:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    switch (IGGlblDrawStyle) {
	        case IG_STATE_DRAW_STYLE_WIREFRAME:
		    SetLabel(w, "Draw Wireframe");
		    break;
	        case IG_STATE_DRAW_STYLE_SOLID:
		    SetLabel(w, "Draw Solid");
		    break;
	        case IG_STATE_DRAW_STYLE_POINTS:
		    SetLabel(w, "Draw Points");
		    break;
	    }
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DEPTH_CUE:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w,
		     IGGlblDepthCue ? "Depth Cue On" : "No Depth Cue");
	    IGRedrawViewWindow();
	    break;
        case IG_STATE_3D_GLASSES:
	    switch (IGGlbl3DGlassesMode) {
	        case IG_GLASSES_3D_NONE:
	        default:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_CHROMADEPTH;
		    Str = "Chroma Depth";
		    break;
	        case IG_GLASSES_3D_CHROMADEPTH:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_BLUE;
		    Str = "Red Blue";
		    break;
	        case IG_GLASSES_3D_RED_BLUE:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_GREEN;
		    Str = "Red Green";
		    break;
	        case IG_GLASSES_3D_RED_GREEN:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_NONE;
		    Str = "No 3D Glasses";
		    break;
	    }
	    SetLabel(w, Str);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_BACK_FACE_CULL:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblBackFaceCull ? "Cull BFace"
					   : "No Cull BFace");
	    IGRedrawViewWindow();
	    break;	
	case IG_STATE_DOUBLE_BUFFER: 
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblDoDoubleBuffer ? "Double Buffer"
					     : "Single Buffer");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_WIDTH_LINES:
	    XmScaleGetValue(w, &NewValue);
	    IGGlblLineWidth += (int) ((NewValue - OldSliderVal) *
				      IGGlblChangeFactor);
	    if (IGGlblLineWidth < 1)
		IGGlblLineWidth = 1;
	    sprintf(NewLabel, "Lines Width %d", IGGlblLineWidth);
	    SetLabel(LineWidthLabel, NewLabel);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_NUM_ISOLINES:
	    XmScaleGetValue(w, &NewValue);
	    Value = (int) (IGGlblNumOfIsolines * 
			   exp(IGGlblChangeFactor * (NewValue - OldSliderVal)
								   / 1000.0));
	    IGGlblNumOfIsolines = Value != IGGlblNumOfIsolines ?
	                   Value : Value + IRIT_SIGN(NewValue - OldSliderVal);
	    if (IGGlblNumOfIsolines < 0)
		IGGlblNumOfIsolines = 0;
	    sprintf(NewLabel, "Isolines %d", IGGlblNumOfIsolines);
	    SetLabel(NumOfIsolinesLabel, NewLabel);
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, FALSE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SAMP_PER_CRV_APPROX:
	    XmScaleGetValue(w, &NewValue);
	    if (IGGlblPolylineOptiApprox == 0) {
	        Value = (int) (IGGlblPllnFineness *
			   exp(IGGlblChangeFactor * (NewValue - OldSliderVal)
								   / 1000.0));
		IGGlblPllnFineness = Value != IGGlblPllnFineness ?
			    Value : Value + IRIT_SIGN(NewValue - OldSliderVal);
		IGGlblPllnFineness = IGGlblPllnFineness > 2 ?
						    IGGlblPllnFineness : 2;
	    }
	    else {
		IGGlblPllnFineness /= exp(IGGlblChangeFactor *
				      (NewValue - OldSliderVal) / 1000.0);
	    }
	    sprintf(NewLabel, "Polyline FineNess %.5g", IGGlblPllnFineness);
	    SetLabel(PolylineFineNessLabel, NewLabel);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, TRUE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_POLY_APPROX:
	    XmScaleGetValue(w, &NewValue);
	    if (IGGlblPolygonOptiApprox == 0) {
	        Value = (int) (IGGlblPlgnFineness *
			       exp(IGGlblChangeFactor *
				   (NewValue - OldSliderVal) / 1000.0));
		IGGlblPlgnFineness = Value != IGGlblPlgnFineness ?
			    Value : Value + IRIT_SIGN(NewValue - OldSliderVal);
		if (IGGlblPlgnFineness < 2.0)
		    IGGlblPlgnFineness = 2.0;
	    }
	    else {
		IGGlblPlgnFineness /= exp(IGGlblChangeFactor *
				      (NewValue - OldSliderVal) / 1000.0);
	    }
	    sprintf(NewLabel, "Polygon FineNess %.5g", IGGlblPlgnFineness);
	    SetLabel(PolygonFineNessLabel, NewLabel);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, FALSE, FALSE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_POLYGON_OPTI:
	    IGStateHandler(IG_STATE_POLYGON_OPTI, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblPolygonOptiApprox == 0 ? "Uniform" : "Optimal");

	    sprintf(NewLabel, "Polygon FineNess %.5g", IGGlblPlgnFineness);
	    SetLabel(PolygonFineNessLabel, NewLabel);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, FALSE, FALSE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_POLYLINE_OPTI:
	    IGStateHandler(IG_STATE_POLYLINE_OPTI, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblPolylineOptiApprox == 0 ? "Uniform" : "Optimal");

	    sprintf(NewLabel, "Polyline FineNess %.5g", IGGlblPllnFineness);
	    SetLabel(PolylineFineNessLabel, NewLabel);

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, TRUE, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_ANTI_ALIASING:
	    IGGlblAntiAliasing = IGGlblAntiAliasing + 1;
	    if (IGGlblAntiAliasing >= 3)
		IGGlblAntiAliasing = 0;
	    SetLabel(w, IGGlblAntiAliasing == 2 ? "Alpha Blend" :
			      (IGGlblAntiAliasing == 1 ? "Anti Aliasing"
						       : "No Anti Aliasing"));
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_INTERNAL:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblDrawInternal ? "Intrnl Edges"
					   : "No Intrnl Edges");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_VNORMAL:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);	
	    SetLabel(w, IGGlblDrawVNormal ? "VNormals" : "No VNormals");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_PNORMAL:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblDrawPNormal ? "PNormals" : "No PNormals"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_NRML_ORIENT:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblFlipNormalOrient ? "Rvrsd Normals"
					       : "Rglr Normals"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_MESH:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblDrawSurfaceMesh ? "Srf Mesh" : "No Srf Mesh");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_POLY:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfacePoly ? "Srf Polys" : "No Srf Polys"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_POLYGONS:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawPolygons ? "Polygons" : "No Polygons"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_WIRE:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfaceWire ? "Srf Isos"
					      : "No Srf Isos"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_BNDRY:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfaceBndry ? "Srf Bndry"
					       : "No Srf Bndry"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_SILH:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfaceSilh ? "Srf Silh"
					      : "No Srf Silh"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_SKTCH:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfaceSketch ? "Srf Sktch"
					        : "No Srf Sktch"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_NUM_POLY_COUNT:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblCountNumPolys ? "Count #Polys" : "No #Polys"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_FRAME_PER_SEC:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblCountFramePerSec ? "Count FPS" : "No FPS"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_DRAW_SRF_RFLCT_LNS:
	    IGStateHandler(State, IG_STATE_TGL, FALSE); 
	    SetLabel(w, IGGlblDrawSurfaceRflctLns ? "Rflct Lns"
					          : "No Rflct Lns"); 
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_FOUR_PER_FLAT:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    SetLabel(w, IGGlblFourPerFlat ? "Four Per Flat" : "Two Per Flat");
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_VIEW_FRONT:
	case IG_STATE_VIEW_SIDE:
	case IG_STATE_VIEW_TOP:
	case IG_STATE_VIEW_ISOMETRY:
	    SetLabel(ProjectionButton, "Orthographic");
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    IGRedrawViewWindow();
	    break;
	case IG_STATE_SAVE_IMAGE:
	    {
		IRIT_STATIC_DATA Widget
		    FileWidget = NULL;
		Position SaveX, SaveY;
		Dimension SaveW;

		if (FileWidget == NULL) {
		    Arg Args[10];
		    XmString Label1, Label2, Label3;

		    Label1 = XmStringCreate("*.ppt", "CharSet1");
		    Label2 = XmStringCreate("irit.ppm", "CharSet1");
		    Label3 = XmStringCreate("Irit Image Files - save window",
					    "CharSet1");
		    XtSetArg(Args[0], XmNdirMask, Label1);
		    XtSetArg(Args[1], XmNdirSpec, Label2);
		    XtSetArg(Args[2], XmNfilterLabelString, Label3);

		    FileWidget = XmCreateFileSelectionDialog(ObjManipParamForm,
							     "Irit Image Save",
							     Args,
							     3);
		    XtAddCallback(FileWidget, XmNokCallback,
				  (XTC) SaveImageFileCB, NULL);
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
	default:
	    IGStateHandler(State, IG_STATE_TGL, FALSE);
	    IGRedrawViewWindow();
    }

    if (CallData -> reason == XmCR_DRAG) 
	OldSliderVal = NewValue;

    if (CallData -> reason == XmCR_VALUE_CHANGED) {
        XmScaleSetValue(w, 0);
	OldSliderVal = 0;
    }
}  

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles image file selection window Buttons.  			     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:             Calling widget.                             		     *
*   ClientData:    Not used.                                                 *
*   FileStruct:    Holds the file name.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveImageFileCB(Widget w,
			    caddr_t ClientData,
			    XmFileSelectionBoxCallbackStruct *FileStruct)
{
    char *FileName;

    XmStringGetLtoR(FileStruct -> value,
		    XmSTRING_DEFAULT_CHARSET, &FileName);

    IGSaveDisplayAsImage(FileName);

    XtUnmanageChild(w);
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

    ChangeFactor[0] = IGGlblChangeFactor * (NewValue - OldSliderVal) / 100;
    ChangeFactor[1] = 0.0;
    if (IGProcessEvent(EventType, ChangeFactor))
	IGRedrawViewWindow(); 

    OldSliderVal = 0;
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

    XmScaleGetValue(w, &NewValue);
    ChangeFactor[0] = IGGlblChangeFactor * (NewValue - OldSliderVal) / 100;
    ChangeFactor[1] = 0.0;
    if (IGProcessEvent(EventType, ChangeFactor)) {
	IGGlblManipulationActive = TRUE;
        IGRedrawViewWindow();
	IGGlblManipulationActive = FALSE;
    }

    OldSliderVal = NewValue;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handler for handling mouse events in the view window.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:	The event that occured in the view window.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ViewWndwInputHandler                                                     M
*****************************************************************************/
void ViewWndwInputHandler(XEvent *Event)
{
    IRIT_STATIC_DATA int BufSize,
	LastButton = -1,
	LastX = -1,
	LastY = -1;
    XEvent NextEvent;
    IrtRType ChangeFactor[2];
    char Buffer[20];
    XComposeStatus Compose;
    KeySym Key;

    switch (Event -> type) {
	case MotionNotify:
	    /* Flushes all motion events in the queue. */
	    while (XPending(IGXDisplay)) {
		XPeekEvent(IGXDisplay, &NextEvent);
		if (NextEvent.type == MotionNotify)
		    XNextEvent(IGXDisplay, Event);
		else
		    break;
	    }

	    if (IGCrvEditGrabMouse &&
		(Event -> xbutton.x != LastX || Event -> xbutton.y != LastY)) {
		CEditHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				 IG_CRV_EDIT_MOTION);
		LastX = Event -> xbutton.x;
		LastY = Event -> xbutton.y;
	        break;
	    }

	    if (IGSrfEditGrabMouse &&
		(Event -> xbutton.x != LastX || Event -> xbutton.y != LastY)) {
		SEditHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				 IG_SRF_EDIT_MOTION);
		LastX = Event -> xbutton.x;
		LastY = Event -> xbutton.y;
	        break;
	    }

	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(Event -> xbutton.x, Event -> xbutton.y,
				    IG_PICK_REP_MOTION);
		break;
	    }

	    if (IGObjManipGrabMouse &&
		(Event -> xbutton.x != LastX || Event -> xbutton.y != LastY)) {
		IGObjManipHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				      IG_OBJ_MANIP_MOTION);
		LastX = Event -> xbutton.x;
		LastY = Event -> xbutton.y;
	        break;
	    }

	    if (LastButton < 0)
	        break;

	    if (Event -> xbutton.x != LastX || Event -> xbutton.y != LastY) {
		ChangeFactor[0] = (Event -> xbutton.x - LastX)
							* IGGlblChangeFactor;
		ChangeFactor[1] = (LastY - Event -> xbutton.y)
							* IGGlblChangeFactor;
		LastX = Event -> xbutton.x;
		LastY = Event -> xbutton.y;

		switch (LastButton) {
		    case 1:
			if (IGProcessEvent(IG_EVENT_ROTATE, ChangeFactor))
			    IGRedrawViewWindow();
			break;
		    case 2:
			ChangeFactor[0] /= 50.0;
			ChangeFactor[1] /= 50.0;
			if (IGProcessEvent(IG_EVENT_SCALE, ChangeFactor))
			    IGRedrawViewWindow();
			break;
		    case 3:
			if (IGProcessEvent(IG_EVENT_TRANSLATE, ChangeFactor))
			    IGRedrawViewWindow();
			break;
		}
	    }
	    break;
	case ButtonPress:
	    switch (Event -> xbutton.button) {
		case Button1:
		    if (IGCrvEditGrabMouse) {
			CEditHandleMouse(Event -> xbutton.x,
					 Event -> xbutton.y,
					 IG_CRV_EDIT_BUTTONDOWN);
			LastX = Event -> xbutton.x;
			LastY = Event -> xbutton.y;
			break;
		    }
		    if (IGSrfEditGrabMouse) {
			SEditHandleMouse(Event -> xbutton.x,
					 Event -> xbutton.y,
					 IG_SRF_EDIT_BUTTONDOWN);
			LastX = Event -> xbutton.x;
			LastY = Event -> xbutton.y;
			break;
		    }
		    if (PickCrsrGrabMouse) {
		        IGHandleCursorEvent(Event -> xbutton.x,
					    Event -> xbutton.y,
					    IG_PICK_REP_BTN1DOWN);
			break;
		    }
		    if (IGObjManipGrabMouse) {
			IGObjManipHandleMouse(Event -> xbutton.x,
					      Event -> xbutton.y,
					      IG_OBJ_MANIP_BTN1DOWN);
			LastX = Event -> xbutton.x;
			LastY = Event -> xbutton.y;
			break;
		    }
		    if (Event -> xbutton.state & ShiftMask) {
			IPObjectStruct
			    *PObj = IGHandlePickEvent(Event -> xbutton.x,
						      Event -> xbutton.y,
						      IG_PICK_ANY);

			if (ReturnPickedObject) {
			    IPObjectStruct *PObjDump;

			    if (PObj != NULL) {
				if (ReturnPickedObjectName)
				    PObjDump = IPGenStrObject("_PickName_",
							IP_GET_OBJ_NAME(PObj),
							NULL);
				else
				    PObjDump = PObj;
			    }
			    else
			        PObjDump = IPGenStrObject("_PickFail_",
							  "*** no object ***",
							  NULL);

			    IPSocWriteOneObject(IGGlblIOHandle, PObjDump);
			    if (PObj != PObjDump)
			        IPFreeObject(PObjDump);
			}
			else {
			    XStoreName(IGXDisplay, IGViewWndw,
				       IGGenerateWindowHeaderString(
					   IGStrResGenericPickEvent(PObj)));
			}
			break;
		    }
		case Button2:
		    if (PickCrsrGrabMouse) {
		        IGHandleCursorEvent(Event -> xbutton.x,
					    Event -> xbutton.y,
					    IG_PICK_REP_BTN2DOWN);
			break;
		    }
		    IGGlblManipulationActive = TRUE;
		    LastButton = Event -> xbutton.button;
		    LastX = Event -> xbutton.x;
		    LastY = Event -> xbutton.y;
		    break;
		case Button3:
		    if (PickCrsrGrabMouse) {
		        IGHandleCursorEvent(Event -> xbutton.x,
					    Event -> xbutton.y,
					    IG_PICK_REP_BTN3DOWN);
			break;
		    }
		    if (IGObjManipGrabMouse) {
			IGObjManipHandleMouse(Event -> xbutton.x,
					      Event -> xbutton.y,
					      IG_OBJ_MANIP_BTN3DOWN);
			LastX = Event -> xbutton.x;
			LastY = Event -> xbutton.y;
			break;
		    }
		    IGGlblManipulationActive = TRUE;
		    LastButton = Event -> xbutton.button;
		    LastX = Event -> xbutton.x;
		    LastY = Event -> xbutton.y;
		    break;
	    }
	    break;
	case ButtonRelease:
	    if (IGCrvEditGrabMouse && Event -> xbutton.button == Button1) {
		CEditHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				 IG_CRV_EDIT_BUTTONUP);
		break;
	    }
	    if (IGSrfEditGrabMouse && Event -> xbutton.button == Button1) {
		CEditHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				 IG_SRF_EDIT_BUTTONUP);
		break;
	    }
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(Event -> xbutton.x, Event -> xbutton.y,
				    IG_PICK_REP_BTN_UP);
		break;
	    }
	    if (IGObjManipGrabMouse && Event -> xbutton.button == Button1) {
		IGObjManipHandleMouse(Event -> xbutton.x, Event -> xbutton.y,
				      IG_OBJ_MANIP_BTN_UP);
		break;
	    }

	    IGGlblManipulationActive = FALSE;
	    IGRedrawViewWindow();		    /* To redraw in highres. */
	    LastButton = -1;
	    break;
	case KeyPress:
	    XLookupString((XKeyEvent *) Event, Buffer, BufSize,
			  &Key, &Compose);
	    if (IGCrvEditActive) {
		switch (Key) {
		    case XK_KP_Down:
		    case XK_Down:
		    case XK_KP_Left:
		    case XK_Left:
			IGCrvEditMRLevel -= 0.01;             /* Percents... */
		        break;
		    case XK_KP_Up:
		    case XK_Up:
		    case XK_KP_Right:
		    case XK_Right:
			IGCrvEditMRLevel += 0.01;             /* Percents... */
			break;
		}

		IGCrvEditMRLevel = IRIT_BOUND(IGCrvEditMRLevel, 0.0, 1.0);
		IGCrvEditParamUpdateMRScale();
		IGRedrawViewWindow();     /* Update the MR region displayed. */
	    }
	    else if (IGSrfEditActive) {
		switch (Key) {
		    case XK_KP_Left:
		    case XK_Left:
			IGSrfEditMRULevel -= 0.01;            /* Percents... */
		        break;
		    case XK_KP_Right:
		    case XK_Right:
			IGSrfEditMRULevel += 0.01;            /* Percents... */
			break;
		    case XK_KP_Down:
		    case XK_Down:
			IGSrfEditMRVLevel -= 0.01;            /* Percents... */
			break;
		    case XK_KP_Up:
		    case XK_Up:
			IGSrfEditMRVLevel += 0.01;            /* Percents... */
			break;
		}

		IGSrfEditMRULevel = IRIT_BOUND(IGSrfEditMRULevel, 0.0, 1.0);
		IGSrfEditMRVLevel = IRIT_BOUND(IGSrfEditMRVLevel, 0.0, 1.0);
		IGSrfEditParamUpdateMRScale();
		IGRedrawViewWindow(); /* Update the MR region displayed. */
	    }
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pick an object from the display.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PickEntity:  Type of entity to pick (object, cursor etc.).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickObject                                                       M
*****************************************************************************/
void IGHandlePickObject(IGPickEntityType PickEntity)
{
    Cursor XCursor;

    switch (PickEntity) {
	case IG_PICK_ENTITY_DONE:
	    /* Restore the cursor. */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_top_left_arrow);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_cross);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_pencil);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a horizontal SubForm.	  				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:  Parent widget.			                             M
*   Pos:     Position of SubForm within parent form.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:  The positioned SubForm widget.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateSubForm				       			     M
*****************************************************************************/
Widget CreateSubForm(Widget Parent, int Pos)
{
    return XtVaCreateManagedWidget("Form",
				   xmFormWidgetClass, 	Parent, 
				   XmNtopAttachment,	XmATTACH_POSITION,
				   XmNtopPosition,	Pos,
				   XmNbottomAttachment, XmATTACH_POSITION,
				   XmNbottomPosition,	Pos + 1,
				   XmNleftAttachment, 	XmATTACH_FORM,
				   XmNrightAttachment,  XmATTACH_FORM,
				   XmNresizable, 	FALSE,
				   NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A new label to an old widget.	  				     M
*                                                                            *
* PARAMETERS:                                                                M
*   w:         Calling widget.                                 		     M
*   NewLabel:  Widget's new Label.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetLabel					       			     M
*****************************************************************************/
void SetLabel(Widget w, char *NewLabel)
{ 
    XmString LabelText;
    Arg Args[10];

    LabelText = XmStringCreate(NewLabel, "CharSet1");
    XtSetArg(Args[0], XmNlabelString, LabelText);
    XtSetValues(w, Args, 1);
    XmStringFree(LabelText);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a push button to pop up menu.   				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Calling widget                        		     M
*   Label: 	     The Label on the push button.			     M
*   Pos:  	     The position of the push button within the menu.	     M
*   FuncPtr, FuncEvent:  Call back function/event.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:	A push button widget.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddButton					       			     M
*****************************************************************************/
Widget AddButton(Widget Parent,
		 char *Label,
		 int Pos,
		 XTC FuncPtr,
		 XTP FuncEvent)
{
    Widget
	Button = XtVaCreateManagedWidget(Label, 
		xmPushButtonWidgetClass,	CreateSubForm(Parent, Pos),
		XmNleftAttachment, 		XmATTACH_FORM,
		XmNrightAttachment, 		XmATTACH_FORM,
		NULL);

    XtAddCallback(Button, XmNactivateCallback, FuncPtr, FuncEvent);

    return Button;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds two push buttons to pop up menu one near the other.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Calling widget                        		     M
*   Pos:  	     The position of the push button within the menu.	     M
*   Label1, FuncPtr1, FuncEvent1:  The Label, call back function, and call   M
*				   back event of first button.		     M
*   Label2, FuncPtr2, FuncEvent2:  The Label, call back function, and call   M
*				   back event of second button.		     M
*   Btn1, Btn2:      The two returned buttons.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddTwoButtons							     M
*****************************************************************************/
void AddTwoButtons(Widget Parent,
		   int Pos,
		   char *Label1,
		   XTC FuncPtr1,
		   XTP FuncEvent1,
		   char *Label2,
		   XTC FuncPtr2,
		   XTP FuncEvent2,
		   Widget *Btn1,
		   Widget *Btn2)
{
    Widget
        SubForm = XtVaCreateManagedWidget("Two Buttons",
			  xmFormWidgetClass,  	CreateSubForm(Parent, Pos),
			  XmNleftAttachment,  	XmATTACH_FORM,
			  XmNrightAttachment, 	XmATTACH_FORM,
			  XmNfractionBase,   	2,
			  NULL);
    
    *Btn1 = XtVaCreateManagedWidget(Label1, 
				    xmPushButtonWidgetClass, SubForm,
				    XmNleftAttachment, 	     XmATTACH_POSITION,
				    XmNleftPosition,         0,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        1,
				    NULL);
    XtAddCallback(*Btn1, XmNactivateCallback, FuncPtr1, FuncEvent1);

    *Btn2 = XtVaCreateManagedWidget(Label2, 
				    xmPushButtonWidgetClass, SubForm,
				    XmNleftAttachment, 	     XmATTACH_POSITION,
				    XmNleftPosition,         1,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        2,
				    NULL);
    XtAddCallback(*Btn2, XmNactivateCallback, FuncPtr2, FuncEvent2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds three push buttons to pop up menu one near the other.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Calling widget                        		     M
*   Pos:  	     The position of the push button within the menu.	     M
*   Label1, FuncPtr1, FuncEvent1:  The Label, call back function, and call   M
*				   back event of first button.		     M
*   Label2, FuncPtr2, FuncEvent2:  The Label, call back function, and call   M
*				   back event of second button.		     M
*   Label3, FuncPtr3, FuncEvent3:  The Label, call back function, and call   M
*				   back event of third button.		     M
*   Btn1, Btn2, Btn3:  The three returned buttons.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddThreeButtons							     M
*****************************************************************************/
void AddThreeButtons(Widget Parent,
		   int Pos,
		   char *Label1,
		   XTC FuncPtr1,
		   XTP FuncEvent1,
		   char *Label2,
		   XTC FuncPtr2,
		   XTP FuncEvent2,
		   char *Label3,
		   XTC FuncPtr3,
		   XTP FuncEvent3,
		   Widget *Btn1,
		   Widget *Btn2,
		   Widget *Btn3)
{
    Widget
        SubForm = XtVaCreateManagedWidget("Two Buttons",
			  xmFormWidgetClass,  	CreateSubForm(Parent, Pos),
			  XmNleftAttachment,  	XmATTACH_FORM,
			  XmNrightAttachment, 	XmATTACH_FORM,
			  XmNfractionBase,   	3,
			  NULL);
    
    *Btn1 = XtVaCreateManagedWidget(Label1, 
				    xmPushButtonWidgetClass, SubForm,
				    XmNleftAttachment, 	     XmATTACH_POSITION,
				    XmNleftPosition,         0,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        1,
				    NULL);
    XtAddCallback(*Btn1, XmNactivateCallback, FuncPtr1, FuncEvent1);

    *Btn2 = XtVaCreateManagedWidget(Label2, 
				    xmPushButtonWidgetClass, SubForm,
				    XmNleftAttachment, 	     XmATTACH_POSITION,
				    XmNleftPosition,         1,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        2,
				    NULL);
    XtAddCallback(*Btn2, XmNactivateCallback, FuncPtr2, FuncEvent2);

    *Btn3 = XtVaCreateManagedWidget(Label3, 
				    xmPushButtonWidgetClass, SubForm,
				    XmNleftAttachment, 	     XmATTACH_POSITION,
				    XmNleftPosition,         2,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        3,
				    NULL);
    XtAddCallback(*Btn3, XmNactivateCallback, FuncPtr3, FuncEvent3);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds radio buttons to pop up menu.   				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Calling widget                        		     M
*   Header:	     of this radio button.                 		     M
*   DefVal:	     index of default value.               		     M
*   Labels:	     The Labels on the radio buttons.			     M
*   n:		     Number of entry in Labels (== Number of buttons.)       M
*   Pos:  	     The position of the push button within the menu.	     M
*   FuncPtr:         Call back function.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:	A radio button widget.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddRadioButton				       			     M
*****************************************************************************/
Widget AddRadioButton(Widget Parent,
		      char *Header,
		      int DefVal,
		      char **Labels,
		      int n,
		      int Pos,
		      XTC FuncPtr)
{
    int i;
    Widget RButton,
	Frame = CreateSubForm(Parent, Pos);
    XmString s[5];

    for (i = 0; i < 5; i++)
	s[i] = NULL;

    switch (n) {
	case 2:
	    RButton = XmVaCreateSimpleRadioBox(Frame, Header, DefVal, FuncPtr,
		       XmNorientation, XmHORIZONTAL,
		       XmVaRADIOBUTTON, s[0] = XmStringCreateSimple(Labels[0]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[1] = XmStringCreateSimple(Labels[1]),
							      NULL, NULL, NULL,
		       NULL);
	    break;
	case 3:
	    RButton = XmVaCreateSimpleRadioBox(Frame, Header, DefVal, FuncPtr,
		       XmNorientation, XmHORIZONTAL,
		       XmVaRADIOBUTTON, s[0] = XmStringCreateSimple(Labels[0]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[1] = XmStringCreateSimple(Labels[1]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[2] = XmStringCreateSimple(Labels[2]),
							      NULL, NULL, NULL,
		       NULL);
	    break;
	case 4:
	    RButton = XmVaCreateSimpleRadioBox(Frame, Header, DefVal, FuncPtr,
		       XmNorientation, XmHORIZONTAL,
		       XmVaRADIOBUTTON, s[0] = XmStringCreateSimple(Labels[0]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[1] = XmStringCreateSimple(Labels[1]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[2] = XmStringCreateSimple(Labels[2]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[3] = XmStringCreateSimple(Labels[3]),
							      NULL, NULL, NULL,
		       NULL);
	    break;
	case 5:
	    RButton = XmVaCreateSimpleRadioBox(Frame, Header, DefVal, FuncPtr,
		       XmNorientation, XmHORIZONTAL,
		       XmNallowShellResize, TRUE,
		       XmVaRADIOBUTTON, s[0] = XmStringCreateSimple(Labels[0]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[1] = XmStringCreateSimple(Labels[1]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[2] = XmStringCreateSimple(Labels[2]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[3] = XmStringCreateSimple(Labels[3]),
							      NULL, NULL, NULL,
		       XmVaRADIOBUTTON, s[4] = XmStringCreateSimple(Labels[4]),
							      NULL, NULL, NULL,
		       NULL);
	    break;
    }

    for (i = 0; i < 5; i++)
	if (s[i] != NULL)
	  XmStringFree(s[i]);

    XtManageChild(RButton);

    return RButton;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds check buttons to pop up menu.   				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Calling widget                        		     M
*   Header:	     of this check button.                 		     M
*   DefVal:	     Bitmask of buttons to set, bit 0 for first button.	     M
*   Labels:	     The Labels on the check buttons.			     M
*   n:		     Number of entry in Labels (== Number of buttons.)       M
*   Pos:  	     The position of the push button within the menu.	     M
*   FuncPtr:         Call back function.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:	A check button widget.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddCheckButton				       			     M
*****************************************************************************/
Widget AddCheckButton(Widget Parent,
		      char *Header,
		      int DefVal,
		      char **Labels,
		      int n,
		      int Pos,
		      XTC FuncPtr)
{
    int i;
    Widget CButton, Btn, 
	Frame = CreateSubForm(Parent, Pos);

    CButton = XmVaCreateSimpleCheckBox(Frame, Header, NULL,
				       XmNorientation, XmHORIZONTAL,
				       NULL);

    for (i = 0; i < n; i++) {
        Btn = XtVaCreateManagedWidget(Labels[i], xmToggleButtonGadgetClass,
				      CButton,
				      XmNset, DefVal & 0x01,
				      NULL);
	XtAddCallback(Btn, XmNvalueChangedCallback, (XTC) FuncPtr, (XTP) i);
	DefVal >>= 1;
    }

    XtManageChild(CButton);

    return CButton;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a label widget to the pop up menu.  				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:   Calling widget.      		                             M
*   Label:    The Label on the Label widget.				     M
*   Pos:      Widgets position within parent form.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:  The newly created Label.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddLabel					       			     M
*****************************************************************************/
Widget AddLabel(Widget Parent, char *Label, int Pos)
{
    return XtVaCreateManagedWidget(Label,
			xmLabelWidgetClass,	CreateSubForm(Parent, Pos),
		 	XmNleftAttachment, 	XmATTACH_FORM,
			XmNrightAttachment, 	XmATTACH_FORM,
			NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a scale to the pop up menu.	  				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Parent widget.                                          M
*   Pos:             Scale's position within menu.                           M
*   Min, Max:        Scale's range.                                          M
*   ScaleFuncPtr, DragFuncPtr, FuncEvent:  Call back function/event.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:   The scale widget.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddSlide					       			     M
*****************************************************************************/
Widget AddSlide(Widget Parent,
		int Pos,
		int Min,
		int Max,
		XTC ScaleFuncPtr,
		XTC DragFuncPtr,
		XTP FuncEvent)
{
    Widget
	Slide = XtVaCreateManagedWidget("Slide",
			xmScaleWidgetClass, 	CreateSubForm(Parent, Pos),
			XmNorientation,		XmHORIZONTAL,
			XmNminimum,  		Min,
			XmNmaximum,		Max,
			XmNrightAttachment, 	XmATTACH_FORM,
			XmNleftAttachment,   	XmATTACH_FORM,
			NULL);

    XtAddCallback(Slide, XmNvalueChangedCallback, ScaleFuncPtr, FuncEvent);
    XtAddCallback(Slide, XmNdragCallback, DragFuncPtr, FuncEvent);

    return Slide;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a scale and a button to the pop up menu.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:          Parent widget.                                          M
*   Pos:             Scale and Button position within menu.                  M
*   BtnLabel: 	     The Label on the push button.			     M
*   BtnFuncPtr, BtnFuncEvent:  Button call back function/event.              M
*   SlMin, SlMax:    SlScale's range.                                        M
*   SlScaleFuncPtr, SlDragFuncPtr, SlFuncEvent:  Slider call back            M
*						 function/event.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   Widget:   The scale widget.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   AddButtonSlide				       			     M
*****************************************************************************/
Widget AddButtonSlide(Widget Parent,
		      int Pos,
		      char *BtnLabel,
		      XTC BtnFuncPtr,
		      XTP BtnFuncEvent,
		      int SlMin,
		      int SlMax,
		      XTC SlScaleFuncPtr,
		      XTC SlDragFuncPtr,
		      XTP SlFuncEvent)
{
    Widget
	Form = CreateSubForm(Parent, Pos),
	SubForm = XtVaCreateManagedWidget("Generic",
					  xmFormWidgetClass,  	Form,
					  XmNleftAttachment,  	XmATTACH_FORM,
					  XmNrightAttachment, 	XmATTACH_FORM,
					  XmNfractionBase,   	16,
					  NULL),
	Button = XtVaCreateManagedWidget(BtnLabel, 
		        xmPushButtonWidgetClass,SubForm,
		        XmNleftAttachment, 	XmATTACH_POSITION,
		        XmNleftPosition,        0,
		        XmNrightAttachment, 	XmATTACH_POSITION,
		        XmNrightPosition,       3,
		        NULL),
	Slide = XtVaCreateManagedWidget("Slide",
			xmScaleWidgetClass, 	SubForm,
			XmNorientation,		XmHORIZONTAL,
			XmNminimum,  		SlMin,
			XmNmaximum,		SlMax,
			XmNrightAttachment, 	XmATTACH_POSITION,
		        XmNleftPosition,        3,
			XmNleftAttachment,   	XmATTACH_POSITION,
		        XmNrightPosition,       16,
			NULL);

    XtAddCallback(Button, XmNactivateCallback, BtnFuncPtr, BtnFuncEvent);

    XtAddCallback(Slide, XmNvalueChangedCallback,
		  SlScaleFuncPtr, SlFuncEvent);
    XtAddCallback(Slide, XmNdragCallback, SlDragFuncPtr, SlFuncEvent);

    return Slide;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one default from X resource data base.  			     *
*                                                                            *
* PARAMETERS:                                                                *
*  Entry:  The entry seeked.		                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   The entry's value.                                                       *
*****************************************************************************/
static char *ReadOneXDefault(char *Entry)
{
    XrmString Type;
    XrmValue Result;
    char Line[IRIT_LINE_LEN_LONG];

    sprintf(Line, "%s.%s", RESOURCE_NAME, Entry);
    if (XrmGetResource(XrmGetDatabase(IGXDisplay), Line,
		       "Program.Name", &Type, &Result))
	return Result.addr;
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads Defaults from X data base.  					     *
*                                                                            *
* PARAMETERS:                                                                *
*   void				                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReadXDefaults(void)
{
    int i;
    XColor Color;
    char *PanelBackGroundColor = ReadOneXDefault("Panel.BackGround"),
         *TransGeometry = ReadOneXDefault("Trans.Geometry"),
         *TransCursorColorStr = ReadOneXDefault("Trans.CursorColor"),
         *ViewBackGroundColor = ReadOneXDefault("View.BackGround"),
         *ViewTextColor = ReadOneXDefault("View.TextColor"),
         *ViewBorderColor = ReadOneXDefault("View.BorderColor"),
         *ViewBorderWidthStr = ReadOneXDefault("View.BorderWidth"),
         *ViewGeometry = ReadOneXDefault("View.Geometry"),
         *ViewCursorColorStr = ReadOneXDefault("View.CursorColor"),
         *MaxColorsStr = ReadOneXDefault("MaxColors");
    if (XParseColor(IGXDisplay, IGXColorMap, "Black", &IGBlackColor))
	XAllocColor(IGXDisplay, IGXColorMap, &IGBlackColor);

    if (PanelBackGroundColor != NULL)
       printf("Found Panel Backgoround \n");

    if (IGGlblTransPrefPos &&
	sscanf(IGGlblTransPrefPos, "%d,%d,%d,%d",
	       &TransPosX, &TransWidth, &TransPosY, &TransHeight) == 4) {
	TransWidth -= TransPosX;
	TransHeight -= TransPosY;
    }
    else if (IRT_STR_NULL_ZERO_LEN(IGGlblTransPrefPos) && TransGeometry) {
	i = XParseGeometry(TransGeometry, &TransPosX, &TransPosY,
		                          &TransWidth, &TransHeight);
    }

    if (TransCursorColorStr != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransCursorColorStr, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color)) {
	TransCursorColor = (XColor *) IritMalloc(sizeof(XColor));
	*TransCursorColor = Color;
    }
    else
	TransCursorColor = NULL;

    if (ViewBackGroundColor &&
	XParseColor(IGXDisplay, IGXColorMap, ViewBackGroundColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewBackGroundPixel = Color.pixel;
    else
	IGViewBackGroundPixel = BlackPixel(IGXDisplay, IGXScreen);

    if (ViewBorderColor &&
	XParseColor(IGXDisplay, IGXColorMap, ViewBorderColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewBorderPixel = Color.pixel;
    else
	IGViewBorderPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (ViewTextColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, ViewTextColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewTextPixel = Color.pixel;
    else
	IGViewTextPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (ViewBorderWidthStr)
	IGViewBorderWidth = atoi(ViewBorderWidthStr);
    else
	IGViewBorderWidth = 1;

    if (IGGlblViewPrefPos &&
	sscanf(IGGlblViewPrefPos, "%d,%d,%d,%d",
	       &IGViewPosX, &IGViewWidth, &IGViewPosY, &IGViewHeight) == 4) {
	IGViewWidth -= IGViewPosX;
	IGViewHeight -= IGViewPosY;
	IGViewHasSize = IGViewHasPos = TRUE;
    }
    else if (IRT_STR_NULL_ZERO_LEN(IGGlblViewPrefPos) && ViewGeometry) {
	i = XParseGeometry(ViewGeometry, &IGViewPosX, &IGViewPosY,
		                         &IGViewWidth, &IGViewHeight);
	IGViewHasPos = i & XValue && i & YValue;
	IGViewHasSize = i & WidthValue && i & HeightValue;
    }
    else
	IGViewHasSize = IGViewHasPos = FALSE;

    if (ViewCursorColorStr != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, ViewCursorColorStr, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color)) {
	IGViewCursorColor = (XColor *) IritMalloc(sizeof(XColor));
	*IGViewCursorColor = Color;
    }
    else
	IGViewCursorColor = NULL;

    if (MaxColorsStr)
	IGMaxColors = atoi(MaxColorsStr);
    else
	IGMaxColors = IG_MAX_COLOR;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Makes some sound.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritBeep                                                               M
*****************************************************************************/
void IGIritBeep(void)
{
    XBell(IGXDisplay, 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Optionally construct a state pop up menu for the driver, if has one.     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCreateStateMenu                                                        M
*****************************************************************************/
void IGCreateStateMenu(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function initilized the subview mat to front, side, top & Isometry  M
* views.                                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGInitializeSubViewMat                                                   M
*****************************************************************************/
void IGInitializeSubViewMat(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function to enable/disable 4views mode.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Display4Views:  TRUE for 4 views mode.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if a change in views' style occured.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetDisplay4Views                                                       M
*****************************************************************************/
int IGSetDisplay4Views(int Display4Views)
{
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the current matrix in a selected file name.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:   Perspective or orthographics current view mode.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSaveCurrentMatInFile                                                   M
*****************************************************************************/
void IGSaveCurrentMatInFile(int ViewMode)
{
    IRIT_STATIC_DATA Widget
	w = NULL;
    Position MainWindowX, MainWindowY;
    Dimension MainWindowW;

    GlblViewModeFileSave = ViewMode;

    if (w == NULL) {
	Arg Args[10];
	XmString Label1, Label2, Label3;

	Label1 = XmStringCreate("*.imd", "CharSet1");
	Label2 = XmStringCreate("irit.imd", "CharSet1");
	Label3 = XmStringCreate("Irit Matrix Data Files", "CharSet1");
	XtSetArg(Args[0], XmNdirMask, Label1);
	XtSetArg(Args[1], XmNdirSpec, Label2);
	XtSetArg(Args[2], XmNfilterLabelString, Label3);

	w = XmCreateFileSelectionDialog(StateForm,
					"Irit Matrix Save",
					Args,
					3);
	XtAddCallback(w, XmNokCallback, (XTC) SaveMatrixFileCB, NULL);
	XtAddCallback(w, XmNcancelCallback, (XTC) XtUnmanageChild, NULL);

	XtUnmanageChild(XmFileSelectionBoxGetChild(w, XmDIALOG_HELP_BUTTON));

	XmStringFree(Label1);
	XmStringFree(Label2);
    }

    XtVaGetValues(IGTopLevel,
		  XmNwidth, &MainWindowW,
		  XmNx,     &MainWindowX,
		  XmNy,     &MainWindowY,
		  NULL);
    XtVaSetValues(w,
		  XmNdefaultPosition, FALSE,
		  XmNx,               MainWindowX + MainWindowW + 16,
		  XmNy,               MainWindowY,
		  NULL); 

    XtManageChild(w);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles file selection window Buttons.  				     *
*                                                                            *
* PARAMETERS:                                                                *
*   w:             Calling widget.                             		     *
*   ClientData:    Not used.                                                 *
*   FileStruct:    Holds the file name.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveMatrixFileCB(Widget w,
			     caddr_t ClientData,
			     XmFileSelectionBoxCallbackStruct *FileStruct)
{
    char *FileName;

    XmStringGetLtoR(FileStruct -> value,
		    XmSTRING_DEFAULT_CHARSET, &FileName);
    fprintf(stderr, "Saving Matrix to \"%s\"\n", FileName);
    IGSaveCurrentMat(GlblViewModeFileSave, FileName);
    XtUnmanageChild(w);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for the error posting IGIritError call so we can      *
* pop down the widget.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Dialog:    To pop down.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGIritErrorOkCb(Widget Dialog)
{
    XtPopdown(XtParent(Dialog));

    GlblPopupWidgetDone = 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for the error posting IGIritError call so we can      *
* pop down the widget.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Dialog:    To pop down.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGIritErrorCancelCb(Widget Dialog)
{
    XtPopdown(XtParent(Dialog));

    GlblPopupWidgetDone = 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make error message box in printf style.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
    IRIT_STATIC_DATA Widget
	Dialog = 0;
    XmString Str;

    fprintf(stderr, "Error: %s\n", Msg);

    if (IGXDisplay == NULL)
	return;

    if (!Dialog) {
        Arg Args[2];
	XmString
	    Ok = XmStringCreateSimple("Ok");

	Str = XmStringCreateSimple(Msg);
	XtSetArg(Args[0], XmNautoUnmanage, False);
	XtSetArg(Args[1], XmNcancelLabelString, Ok);
	Dialog = XmCreateWarningDialog(IGTopLevel, "Warning", Args, 2);
	XtAddCallback(Dialog, XmNcancelCallback, (XTC) IGIritErrorOkCb, NULL);
	XtUnmanageChild(XmMessageBoxGetChild(Dialog, XmDIALOG_OK_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(Dialog, XmDIALOG_HELP_BUTTON));
    }

    Str = XmStringCreateSimple(Msg);
    XtVaSetValues(Dialog, 
		  XmNmessageString, Str, 
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  NULL);
    XmStringFree(Str);

    XtManageChild(Dialog);
    XtPopup(XtParent(Dialog), XtGrabNone);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make yes/no message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Title message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if yes was selected, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritYesNoQuestion                                                      M
*****************************************************************************/
int IGIritYesNoQuestion(char *Msg)
{
    IRIT_STATIC_DATA Widget
	Dialog = 0;
    XmString Str;

    if (IGXDisplay == NULL)
	return FALSE;

    if (!Dialog) {
        Arg Args[3];
	XmString
	    Yes = XmStringCreateSimple("Yes"),
	    No = XmStringCreateSimple("No");

	Str = XmStringCreateSimple(Msg);
	XtSetArg(Args[0], XmNautoUnmanage, False);
	XtSetArg(Args[2], XmNokLabelString, Yes);
	XtSetArg(Args[1], XmNcancelLabelString, No);
	Dialog = XmCreateQuestionDialog(IGTopLevel, "Question", Args, 3);
	XtAddCallback(Dialog, XmNcancelCallback,
		      (XTC) IGIritErrorCancelCb, NULL);
	XtAddCallback(Dialog, XmNokCallback, (XTC) IGIritErrorOkCb, NULL);
	XtUnmanageChild(XmMessageBoxGetChild(Dialog, XmDIALOG_HELP_BUTTON));
    }

    Str = XmStringCreateSimple(Msg);
    XtVaSetValues(Dialog, 
		  XmNmessageString, Str, 
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  NULL);
    XmStringFree(Str);

    XtManageChild(Dialog);
    XtPopup(XtParent(Dialog), XtGrabNone);

    /* Wait for the call back function to return. */
    GlblPopupWidgetDone = FALSE;
    while (!GlblPopupWidgetDone)
        XtAppProcessEvent(IGApplication, XtIMAll);

    return GlblPopupWidgetDone - 1;
}
