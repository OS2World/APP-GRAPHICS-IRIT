/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber		               Ver 0.1, Mar. 1990    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* MSDOS graphical interface for IRIT. Based on intr_lib windowing library.   *
*****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "djggraph.h"
#include "grap_loc.h"

#define DJG_MAP_X_COORD(x) ((int) ((x + 1.0) * ViewWidth2))
#define DJG_MAP_Y_COORD(y) ((int) ((1.0 - y) * ViewHeight2))

#ifdef DJGCC
#define NUM_OF_TEXT_LINES 200
#else
#define NUM_OF_TEXT_LINES 50
#endif /* DJGCC */

/* Interactive menu setup structure: */
#define INTERACT_NUM_OF_STRINGS		3
#define INTERACT_NUM_OF_SUB_WNDWS	15
#define INTERACT_SUB_WINDOW_HEIGHT	0.035 /* Actually half Height/Width. */
#define INTERACT_SUB_WINDOW_WIDTH	0.8
#define INTERACT_X_CENTER		((SW_MAX_X + SW_MIN_X) / 2.0 + 0.02)

typedef struct InteractString {
    IntrRType X, Y;
    int Color;
    char *Str;
} InteractString;

typedef struct InteractSubWindow {
    IntrRType X, Y;					   /* Center points. */
    IGGraphicEventType InteractCode;
    int Color,
	TextInside; /* If TRUE, Str will be in window, otherwise left to it. */
    char *Str;
} InteractSubWindow;

typedef struct InteractWindowStruct {	 /* The interactive menu structures. */
    /* Rotate, Translate, Scale strings: */
    InteractString Strings[INTERACT_NUM_OF_STRINGS];
    InteractSubWindow SubWindows[INTERACT_NUM_OF_SUB_WNDWS];
} InteractWindowStruct;

STATIC_DATA int
    ViewWidth2 = 100,
    ViewHeight2 = 100,
    GlblWindowFrameWidth = 8,	     	      /* Window color configuration. */
    GlblViewFrameColor = RED,
    GlblViewBackColor = BLUE,
    GlblTransFrameColor = RED,
    GlblTransBackColor = BLUE,
    GlblInputFrameColor = RED,
    GlblInputBackColor = BLUE,
    GlblDrawHeader = FALSE,      /* Window general attributes configuration. */
    GlblSmoothTextScroll = TRUE,
    GlblIntrSaveMethod = INTR_SAVE_CONV,
    GlblMouseSensitivity = 10,
    GlblJoystickExists = FALSE,
    GlblMouseExists = TRUE;
STATIC_DATA char
    *GlblViewWndwPos = "0.03, 0.03, 0.67, 0.67",     /* Location of windows. */
    *GlblViewWndwPosNoInput = "0.03, 0.03, 0.67, 0.97",
    *GlblTransWndwPos = "0.73, 0.03, 0.97, 0.67",
    *GlblTransWndwPosNoInput = "0.73, 0.03, 0.97, 0.97",
    *GlblInputWndwPos = "0.03, 0.73, 0.97, 0.97",
    *GlblIntrSaveDisk = ".";

STATIC_DATA IntrBType
    HasInputWindow = FALSE;

STATIC_DATA int
    InteractNumOfSubWndws = INTERACT_NUM_OF_SUB_WNDWS;
    
/* Interactive mode menu set up structure is define below: */
STATIC_DATA InteractWindowStruct InteractMenu = {
    { { 0.0, -0.63, RED,   "Rotate" },
      { 0.0, -0.34, GREEN, "Translate" },
      { 0.0, -0.05, CYAN,  "Scale" },
    },
    { { 0.0, -0.9,  IG_EVENT_SCR_OBJ_TGL,    CYAN,   TRUE,  "Screen Coords." },
      { 0.0, -0.79, IG_EVENT_PERS_ORTHO_TGL, BLUE,   TRUE,  "Perspectiv" },
      { 0.0, -0.72, IG_EVENT_PERS_ORTHO_Z,   BLUE,   FALSE, "Z" },
      { 0.0, -0.57, IG_EVENT_ROTATE_X,       RED,    FALSE, "X" }, /* Rot */
      { 0.0, -0.50, IG_EVENT_ROTATE_Y,       RED,    FALSE, "Y" },
      { 0.0, -0.43, IG_EVENT_ROTATE_Z,       RED,    FALSE, "Z" },
      { 0.0, -0.28, IG_EVENT_TRANSLATE_X,    GREEN,  FALSE, "X" }, /* Trans */
      { 0.0, -0.21, IG_EVENT_TRANSLATE_Y,    GREEN,  FALSE, "Y" },
      { 0.0, -0.14, IG_EVENT_TRANSLATE_Z,    GREEN,  FALSE, "Z" },
      { 0.0,  0.01, IG_EVENT_SCALE,          CYAN,   FALSE, "" },  /* Scale */
      { 0.0,  0.12, IG_EVENT_DEPTH_CUE,      MAGENTA,TRUE,  "Depth cue" },
      { 0.0,  0.3,  IG_EVENT_SAVE_MATRIX,    YELLOW, TRUE,  "Save Matrix" },
      { 0.0,  0.39, IG_EVENT_PUSH_MATRIX,    YELLOW, TRUE,  "Push Matrix" },
      { 0.0,  0.48, IG_EVENT_POP_MATRIX,     YELLOW, TRUE,  "Pop Matrix" },
      { 0.0,  0.62, IG_EVENT_QUIT,           WHITE,  TRUE,  "Quit" },
    }
};
	
STATIC_DATA IntrPopUpMenuStruct *PUWndwMenu;
STATIC_DATA char *PopUpMenuStrs[] = {
    "Redraw All",
    "Move",
    "Resize",
    "Pop",
    "Push",
    "Zoom",
    "Reset",
    "Headers",
};
#define POP_UP_MENU_SIZE (sizeof(PopUpMenuStrs) / sizeof(char *))

IRIT_GLOBAL_DATA int
    IGGlblInputWindowID = -1,
    IGGlblViewWindowID = -1,
    IGGlblTransWindowID = -1;

static void SetColorIndex(int color);
static void SetViewWindowFBBox(int WindowID);
static IntrEventType LclGetChar(int *x, int *y);
static void SetBBoxSize(IntrBBoxStruct *BBox, char *GlblTextWindowPos);
static void ViewWndwRefreshFunction(int WindowID);
static void TransWndwRefreshFunction(int WindowID);
static void TransWndwRefreshFunctionAux(void);
static void InteractUpdateMenu(char *Str, int Entry);
static void InputWndwRefreshFunction(int WindowID);
static void SetMaximumBBoxSize(int WindowID);
static void PopUpMenuFunc(int KeyStroke);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Optionally construct a state pop up menu for the driver, if has one.       M
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
* Handles the events of the pop up window.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       Event to handle.                                            M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Do we need to refresh the screen according to what we know  M
*		 on entry.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if we need to refresh the screen.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleState                                                            M
*****************************************************************************/
int IGHandleState(int State, int StateStatus, int Refresh)
{
    int UpdateView;

    UpdateView = IGStateHandler(State, StateStatus, Refresh);

    if (UpdateView)
	IntrWndwRedrawAll();

    IGCreateStateMenu();

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make some sound.                                                           M
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
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to move to.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGMoveTo2D                                                               M
*****************************************************************************/
void IGMoveTo2D(IrtRType X, IrtRType Y)
{
    GRSMoveTo(DJG_MAP_X_COORD(X), DJG_MAP_Y_COORD(Y));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to draw to.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGLineTo2D                                                               M
*****************************************************************************/
void IGLineTo2D(IrtRType X, IrtRType Y)
{
    GRSLineTo(DJG_MAP_X_COORD(X), DJG_MAP_Y_COORD(Y));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the intensity of a color (high or low).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   High:     TRUE for high, FALSE for low.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorIntensity                                                      M
*****************************************************************************/
void IGSetColorIntensity(int High)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the color of an object according to its color/rgb attributes.	     M
*   If object has an RGB attribute it will be used. Otherwise, if the object M
* has a COLOR attribute it will use. Otherwise, WHITE will be used.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To set the drawing color to its color.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorObj                                                            M
*****************************************************************************/
void IGSetColorObj(IPObjectStruct *PObj)
{
    int c, Color[3];

    if (AttrGetObjectRGBColor(PObj, &Color[0], &Color[1], &Color[2])) {
	SetColorIndex((Color[0] + Color[1] + Color[2]) % 15 + 1);
    }
    else if ((c = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR) {
	SetColorIndex(c);
    }
    else {
	/* Use white as default color: */
	GRSetColor(WHITE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the line width to draw the given object, in pixels.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Width:    In pixels of lines to draw with.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetWidthObj                                                            M
*****************************************************************************/
void IGSetWidthObj(int Width)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the line pattern to draw the given object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pattern:    The line pattern to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetLinePattern                                                         M
*****************************************************************************/
void IGSetLinePattern(int Pattern)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the opacity level to draw the given object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Transparency:    Level of transparency - 1 full, 0 opaque.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetTranspObj                                                           M
*****************************************************************************/
void IGSetTranspObj(IrtRType Transparency)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares the texture mapping function of an object.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to apply texture to.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if succesful, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetTexture                                                             M
*****************************************************************************/
int IGSetTexture(IPObjectStruct *PObj)
{
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets the color according to the given color index.		    	     *
*                                                                            *
* PARAMETERS:                                                                *
*   color:     Index of color to use. Must be between 0 and IG_MAX_COLOR.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetColorIndex(int color)
{
    if (color >= MAX_COLOR)
	color = WHITE;

    GRSetColor(color);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Initializes  the Intr_lib library.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InitIntrLibWindows                                                       M
*****************************************************************************/
void InitIntrLibWindows(void)
{
    IntrCursorShapeStruct Cursor;
    IntrBBoxStruct BBox;
    IntrFBBoxStruct FBBox;

    IntrSetSaveBackPath(GlblIntrSaveDisk);
    IntrSetSaveBackMethod(GlblIntrSaveMethod);
    IntrSetHandleInternalEvents(TRUE, FALSE);

    IntrInit();

    IntrSetMouseSensitivity(GlblMouseSensitivity);
    IntrSetInputDevice((GlblMouseExists ? INTR_INPT_DEVICE_MOUSE : 0) |
		       (GlblJoystickExists ? INTR_INPT_DEVICE_JOYSTICK : 0) |
		       INTR_INPT_DEVICE_KEYBOARD);

    IntrRegisterKeyStroke(0x13B /* F1 */, PopUpMenuFunc);

    /* Set the default cursor to arrow. */
    Cursor.CursorType = INTR_CURSOR_ARROW;
    IntrSetCursorType(&Cursor);

    /* Prepare the pop up menu. */
    PUWndwMenu = IntrPopUpMenuCreate("Windows", PopUpMenuStrs, 0,
				  POP_UP_MENU_SIZE,
				  INTR_COLOR_GREEN, INTR_COLOR_CYAN,
				  INTR_COLOR_YELLOW, INTR_COLOR_MAGENTA,
                                  16, &Cursor);


    if (HasInputWindow)
        SetBBoxSize(&BBox, GlblViewWndwPos);
    else
	SetBBoxSize(&BBox, GlblViewWndwPosNoInput);
    IGGlblViewWindowID = IntrWndwCreate("View",
					GlblWindowFrameWidth,
					&BBox,
					GlblViewFrameColor,
					GlblViewBackColor,
					&Cursor,
					NULL,
					ViewWndwRefreshFunction);
    IntrWndwSetDrawHeader(IGGlblViewWindowID, GlblDrawHeader);
    IntrWndwPop(IGGlblViewWindowID, TRUE, FALSE);
    SetViewWindowFBBox(IGGlblViewWindowID);
    
    if (HasInputWindow)
        SetBBoxSize(&BBox, GlblTransWndwPos);
    else
	SetBBoxSize(&BBox, GlblTransWndwPosNoInput);

    IGGlblTransWindowID = IntrWndwCreate("Transformations",
					 GlblWindowFrameWidth,
					 &BBox,
					 GlblTransFrameColor,
					 GlblTransBackColor,
					 &Cursor,
					 NULL,
					 TransWndwRefreshFunction);
    FBBox.FXmin = FBBox.FYmin = -1.0;
    FBBox.FXmax = 1.0;
    FBBox.FYmax = 0.7;
    IntrWndwSetFBBox(IGGlblTransWindowID, &FBBox);
    IntrWndwSetDrawHeader(IGGlblTransWindowID, GlblDrawHeader);
    IntrWndwPop(IGGlblTransWindowID, TRUE, TRUE);

    if (HasInputWindow) {
	SetBBoxSize(&BBox, GlblInputWndwPos);
	IGGlblInputWindowID = IntrWndwCreate("Input",
					     GlblWindowFrameWidth,
					     &BBox,
					     GlblInputFrameColor,
					     GlblInputBackColor,
					     &Cursor,
					     NULL,
					     InputWndwRefreshFunction);
	IntrTextInitWindow(IGGlblInputWindowID, TRUE, INTR_COLOR_YELLOW,
			   INTR_COLOR_RED, INTR_SCRLBAR_NONE,
			   INTR_SCRLBAR_LEFT, NUM_OF_TEXT_LINES, 90);
	IntrWndwSetDrawHeader(IGGlblInputWindowID, GlblDrawHeader);
	IntrTextSetSmoothScroll(GlblSmoothTextScroll);
	IntrWndwPop(IGGlblInputWindowID, TRUE, TRUE);
    }

    /* Make the get line intr_lib routine use this routine to get chars. */
    GRSetGetKeyFunc(LclGetChar);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Terminates the Intr_lib library.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CloseIntrLibWindows                                                      M
*****************************************************************************/
void CloseIntrLibWindows(void)
{
    IntrPopUpMenuDelete(PUWndwMenu);
    IntrWndwDelete(IGGlblViewWindowID, FALSE);
    IntrWndwDelete(IGGlblTransWindowID, FALSE);
    if (HasInputWindow)
	IntrWndwDelete(IGGlblInputWindowID, FALSE);

    IntrClose();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Handles input events                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   ChangeFactor:        A continuous numeric value between -1 and 1. This   *
*			 value will be used to set amount of event such as   *
*			 rotation or translation.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor)
{
    int i, x, y;
    IGGraphicEventType InteractCode;

    while (TRUE) {
	IntrRType Wx, Wy;

	switch (IntrGetEventWait(&x, &y)) {
	    case INTR_EVNT_SELECT:
		if (IntrMapEventToRWindow(IGGlblTransWindowID, x, y, &Wx, &Wy)) {
		    if (Wx < -INTERACT_SUB_WINDOW_WIDTH ||
			Wx > INTERACT_SUB_WINDOW_WIDTH)
			break;
		    for (i = 0; i < InteractNumOfSubWndws; i++)
			if (Wy < InteractMenu.SubWindows[i].Y +
						INTERACT_SUB_WINDOW_HEIGHT &&
			    Wy > InteractMenu.SubWindows[i].Y -
						INTERACT_SUB_WINDOW_HEIGHT) {
			    InteractCode = InteractMenu.SubWindows[i].InteractCode;

			    switch (InteractCode) {
				case IG_EVENT_SCR_OBJ_TGL:
				    switch (IGGlblTransformMode) {
					case IG_TRANS_SCREEN:
					    InteractUpdateMenu("Object Coords.", 0);
					    IGGlblTransformMode = IG_TRANS_OBJECT;
					    break;
					case IG_TRANS_OBJECT:
					    InteractUpdateMenu("Screen Coords.", 0);
					    IGGlblTransformMode = IG_TRANS_SCREEN;
					    break;
				    }
				    break;
				case IG_EVENT_PERS_ORTHO_TGL:
				    switch (IGGlblViewMode) {
					case IG_VIEW_PERSPECTIVE:
					    InteractUpdateMenu("Orthographic", 1);
					    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
					    break;
					case IG_VIEW_ORTHOGRAPHIC:
					    InteractUpdateMenu("Perspective", 1);
					    IGGlblViewMode = IG_VIEW_PERSPECTIVE;
					    break;
				    }
				    break;
				case IG_EVENT_DEPTH_CUE:
				    IGGlblDepthCue = !IGGlblDepthCue;
				    InteractUpdateMenu(
					IGGlblDepthCue ? "Depth Cue" : "No Depth Cue",
					10);
				    break;
				default:
				    break;
			    }

			    *ChangeFactor = Wx / INTERACT_SUB_WINDOW_WIDTH;
			    return InteractCode;
			}
		}
		else {
		    PopUpMenuFunc(0);               /* Pop up the main menu. */
		}
		break;
	    case INTR_EVNT_ABORT:
		break;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Similar to GetGraphicEvent above but for intr_lib get graphic line         *
* routine IGGetGraphicLine, so it may activate the pop up menu async.        *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:   Location of event.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IntrEventType:  Event type.                                              *
*****************************************************************************/
static IntrEventType LclGetChar(int *x, int *y)
{
    IntrRType Wx, Wy;
    IntrEventType Event;

    switch (Event = IntrGetEventWait(x, y)) {
	case INTR_EVNT_SELECT:
	    if (!IntrMapEventToRWindow(IGGlblTransWindowID, *x, *y, &Wx, &Wy)) {
		PopUpMenuFunc(0);                   /* Pop up the main menu. */
	    }
	    break;
    }

    return Event;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Enables or Disables (default) the existance of Input window.		     M
* This function should be called BEFORE InitIntrLibWindows above.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   HasInputWndw:  Do we want an input window?                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GGSetInputWindow                                                         M
*****************************************************************************/
void GGSetInputWindow(IntrBType HasInputWndw)
{
    HasInputWindow = HasInputWndw;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Test if quit display event occured - SPACE was hit on	keyboard or right    M
* button was clicked on mouse.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE, if we need to abort current operation.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GGIsAbortKeyPressed                                                      M
*****************************************************************************/
int GGIsAbortKeyPressed(void)
{
    int x, y;

    return IntrGetEventNoWait(&x, &y) == INTR_EVNT_ABORT;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Parse the position string and set the BBox location.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   BBox:               Where to save size.                                  *
*   GlblTextWindowPos:  String to parse - "x1, y1, x2, y2"                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetBBoxSize(IntrBBoxStruct *BBox, char *GlblTextWindowPos)
{
    int i;
    IntrRType XYLoc[4];

    if (sscanf(GlblTextWindowPos, "%f, %f, %f, %f",
	       &XYLoc[0], &XYLoc[1], &XYLoc[2], &XYLoc[3]) != 4) {
	XYLoc[0] = XYLoc[1] = 0.1;
	XYLoc[2] = XYLoc[3] = 0.9;
    }

    for (i = 0; i < 4; i++) {
	if (XYLoc[i] < 0.0)
	    XYLoc[i] = 0.0;
	if (XYLoc[i] > 1.0)
	    XYLoc[i] = 1.0;
    }

    BBox -> Xmin = (int) (XYLoc[0] * GRScreenMaxX);
    BBox -> Ymin = (int) (XYLoc[1] * GRScreenMaxY);
    BBox -> Xmax = (int) (XYLoc[2] * GRScreenMaxX);
    BBox -> Ymax = (int) (XYLoc[3] * GRScreenMaxY);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the normalized floating bbox for the provided window.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   WindowID:    Window to handle.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetViewWindowFBBox(int WindowID)
{
    IntrBBoxStruct *BBox;
    IntrFBBoxStruct FBBox;
    
    /* Compute the aspect ratio of this window and save it so we can match   */
    /* it if the window is resized or zoomed etc.			     */
    BBox = IntrWndwGetBBox(WindowID);
    if (BBox -> _Dy > BBox -> _Dx) {
	/* Normalize the X axis to hold -1 to 1 domain. */
	FBBox.FYmin = 			      /* Note the Y axis is flipped. */
	     (1.0 / GRScreenAspect) * (((IntrRType) BBox -> _Dy) / BBox -> _Dx);
	FBBox.FYmax = -FBBox.FYmin;
	FBBox.FXmin = -1.0;
	FBBox.FXmax = 1.0;
    }
    else {
	/* Normalize the Y axis to hold -1 to 1 domain. */
	FBBox.FYmin = 1.0;		      /* Note the Y axis is flipped. */
        FBBox.FYmax = -1.0;
	FBBox.FXmax =
		     GRScreenAspect * (((IntrRType) BBox -> _Dx) / BBox -> _Dy);
	FBBox.FXmin = -FBBox.FXmax;
    }
    IntrWndwSetFBBox(WindowID, &FBBox);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A call back routine that is invoked whenever the View window needs to be   *
* refreshed.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   WindowID:    Window to handle.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ViewWndwRefreshFunction(int WindowID)
{
    IPObjectStruct *PObj;
    int x1, y1, x2, y2;

    IntrWndwClear(WindowID);
    GRGetViewPort(&x1, &y1, &x2, &y2);
    ViewWidth2 = (x2 - x1) / 2;
    ViewHeight2 = (y2 - y1) / 2;

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(MatrixType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }

    for (PObj = IGGlblDisplayList; PObj != NULL; PObj = PObj -> Pnext)
	IGDrawObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A call back routine that is invoked whenever the Trans window needs to be  *
* refreshed.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   WindowID:    Window to handle.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TransWndwRefreshFunction(int WindowID)
{
    switch (IGGlblTransformMode) {
	case IG_TRANS_SCREEN:
	    InteractMenu.SubWindows[0].Str = "Screen Coords.";
	    break;
	case IG_TRANS_OBJECT:
	    InteractMenu.SubWindows[0].Str = "Object Coords.";
	    break;
    }

    switch (IGGlblViewMode) {
	case IG_VIEW_PERSPECTIVE:
	    InteractMenu.SubWindows[1].Str = "Perspective";
	    break;
	case IG_VIEW_ORTHOGRAPHIC:
	    InteractMenu.SubWindows[1].Str = "Orthographic";
	    break;
    }

    if (IGGlblDepthCue)
	InteractMenu.SubWindows[10].Str = "Depth Cue";
    else
	InteractMenu.SubWindows[10].Str = "No Depth Cue";

    TransWndwRefreshFunctionAux();	    /* Draw the transformation menu. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw the Transformation window, using the InteractiveMenu structure        *
* defined above.						             *
*  It is assumed that string not inside of SubWindow will be of length 1.    *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TransWndwRefreshFunctionAux(void)
{
    int i;

    GRPushTextSetting();
    GRSetTextJustify(GR_TEXT_HJUSTIFY_CENTER,
		     GR_TEXT_VJUSTIFY_CENTER);	   /* Draw strings centered. */

    GRSetLineStyle(GR_SOLID_LINE, 0, GR_NORM_WIDTH);

    for (i = 0; i < INTERACT_NUM_OF_STRINGS; i++) {/* Draw strings of struct.*/
	GRSetColor(InteractMenu.Strings[i].Color);
	IntrWndwRText(InteractMenu.Strings[i].X,
		      InteractMenu.Strings[i].Y,
		      InteractMenu.Strings[i].Str);
    }

    for (i = 0; i < InteractNumOfSubWndws; i++) {
	/* Draw struct sub windows. */
	GRSetColor(InteractMenu.SubWindows[i].Color);
	/* Draw the frame of the SubWindow: */
	IntrWndwRMoveTo(
	    InteractMenu.SubWindows[i].X - INTERACT_SUB_WINDOW_WIDTH,
	    InteractMenu.SubWindows[i].Y - INTERACT_SUB_WINDOW_HEIGHT);
	IntrWndwRLineTo(
	    InteractMenu.SubWindows[i].X + INTERACT_SUB_WINDOW_WIDTH,
	    InteractMenu.SubWindows[i].Y - INTERACT_SUB_WINDOW_HEIGHT);
	IntrWndwRLineTo(
	    InteractMenu.SubWindows[i].X + INTERACT_SUB_WINDOW_WIDTH,
	    InteractMenu.SubWindows[i].Y + INTERACT_SUB_WINDOW_HEIGHT);
	IntrWndwRLineTo(
	    InteractMenu.SubWindows[i].X - INTERACT_SUB_WINDOW_WIDTH,
	    InteractMenu.SubWindows[i].Y + INTERACT_SUB_WINDOW_HEIGHT);
	IntrWndwRLineTo(
	    InteractMenu.SubWindows[i].X - INTERACT_SUB_WINDOW_WIDTH,
	    InteractMenu.SubWindows[i].Y - INTERACT_SUB_WINDOW_HEIGHT);

	/* Now the strings inside (and if outside, a middle vertical line): */
	if (InteractMenu.SubWindows[i].TextInside)
	    IntrWndwRText(InteractMenu.SubWindows[i].X,
			  InteractMenu.SubWindows[i].Y,
			  InteractMenu.SubWindows[i].Str);
	else {
	    IntrWndwRText(InteractMenu.SubWindows[i].X -
				INTERACT_SUB_WINDOW_WIDTH - 0.1,
			  InteractMenu.SubWindows[i].Y,
			  InteractMenu.SubWindows[i].Str);
	    IntrWndwRLine(InteractMenu.SubWindows[i].X,
			  InteractMenu.SubWindows[i].Y -
				INTERACT_SUB_WINDOW_HEIGHT,
			  InteractMenu.SubWindows[i].X,
			  InteractMenu.SubWindows[i].Y +
				INTERACT_SUB_WINDOW_HEIGHT);
	}
    }

    GRPopTextSetting();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Updates entry Entry with a new string Str.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:        New string for entry Entry.                                  *
*   Entry:      Index of entry in menu to modify.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InteractUpdateMenu(char *Str, int Entry)
{
    GRPushTextSetting();

    GRSetTextJustify(GR_TEXT_HJUSTIFY_CENTER,
		     GR_TEXT_VJUSTIFY_CENTER);	   /* Draw strings centered. */

    GRPushViewPort();
    IntrWndwPop(IGGlblTransWindowID, FALSE, FALSE);

    GRSetColor(BLACK);				    /* Erase the old string. */
    IntrWndwRText(InteractMenu.SubWindows[Entry].X,
		  InteractMenu.SubWindows[Entry].Y,
		  InteractMenu.SubWindows[Entry].Str);

    InteractMenu.SubWindows[Entry].Str = Str;       /* Update to new one. */

    GRSetColor(InteractMenu.SubWindows[Entry].Color);/* And draw the new. */
    IntrWndwRText(InteractMenu.SubWindows[Entry].X,
		  InteractMenu.SubWindows[Entry].Y,
		  InteractMenu.SubWindows[Entry].Str);

    GRPopViewPort();

    GRPopTextSetting();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A call back routine that is invoked whenever the Input window needs to be  *
* refreshed.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   WindowID:    Window to handle.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InputWndwRefreshFunction(int WindowID)
{
    IntrTextWndwRefresh(WindowID);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Puts a bound on a window size.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   WindowID:    Window to handle.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetMaximumBBoxSize(int WindowID)
{
   IntrBBoxStruct BBox;

   BBox.Xmin = BBox.Ymin = GlblWindowFrameWidth;
   BBox.Xmax = GRScreenMaxX - GlblWindowFrameWidth;
   BBox.Ymax = GRScreenMaxY - GlblWindowFrameWidth;

   if (WindowID == IGGlblInputWindowID)
	BBox.Xmin += IntrWndwScrollBarWidth();

   IntrWndwSetResizeBBox(&BBox);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Pops up the pop up menu.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   KeyStroke:   Not used.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PopUpMenuFunc(int KeyStroke)
{
    int WindowID;
    IntrIntFunc ViewRefreshFunc;
    IntrBBoxStruct BBox;

    if (IntrPopUpMenu(PUWndwMenu, 0)) {
	switch (PUWndwMenu -> SelectedIndex) {
	    case 0: /* Redraw all windows. */
		IntrWndwRedrawAll();
		break;
	    case 1: /* Move window. */
		if (IntrPopUpActive() == 0 &&
		    (WindowID = IntrWndwPick()) > 0)
		    IntrWndwMove(WindowID, TRUE);
		break;
	    case 2: /* Resize window. */
		if (IntrPopUpActive() == 0 &&
		    (WindowID = IntrWndwPick()) > 0) {
		    SetMaximumBBoxSize(WindowID);
		    if (WindowID == IGGlblViewWindowID) {
			/* DIsable refresh function since we change aspect. */
			ViewRefreshFunc = IntrWndwGetRefreshFunc(IGGlblViewWindowID);
			IntrWndwSetRefreshFunc(IGGlblViewWindowID, NULL);
		    }
		    IntrWndwResize(WindowID, TRUE);
		    if (WindowID == IGGlblViewWindowID) {
			SetViewWindowFBBox(WindowID);
			IntrWndwSetRefreshFunc(IGGlblViewWindowID, ViewRefreshFunc);
			IntrWndwPop(WindowID, TRUE, TRUE);
		    }
		}
		break;
	    case 3: /* Pop window. */
		if (IntrPopUpActive() == 0 &&
		    (WindowID = IntrWndwPick()) > 0)
		    IntrWndwPop(WindowID, TRUE, FALSE);
		break;
	    case 4: /* Push window. */
		if (IntrPopUpActive() == 0 &&
		    (WindowID = IntrWndwPick()) > 0)
		    IntrWndwPush(WindowID, TRUE);
		break;
	    case 5: /* Zoom window. */
		if (IntrPopUpActive() == 0 &&
		    (WindowID = IntrWndwPick()) > 0) {
		    SetMaximumBBoxSize(WindowID);
		    if (WindowID == IGGlblViewWindowID) {
			/* DIsable refresh function since we change aspect. */
			ViewRefreshFunc = IntrWndwGetRefreshFunc(IGGlblViewWindowID);
			IntrWndwSetRefreshFunc(IGGlblViewWindowID, NULL);
		    }
		    IntrWndwFullSize(WindowID, TRUE);
		    if (WindowID == IGGlblViewWindowID) {
			SetViewWindowFBBox(WindowID);
			IntrWndwSetRefreshFunc(IGGlblViewWindowID, ViewRefreshFunc);
			IntrWndwPop(WindowID, TRUE, TRUE);
		    }
		}
		break;
	    case 6: /* Reset window sizes. */
		SetBBoxSize(&BBox, GlblViewWndwPos);
		IntrWndwSetBBox(IGGlblViewWindowID, &BBox);
		SetBBoxSize(&BBox, GlblTransWndwPos);
		IntrWndwSetBBox(IGGlblTransWindowID, &BBox);
		if (HasInputWindow) {
		    SetBBoxSize(&BBox, GlblInputWndwPos);
		    IntrWndwSetBBox(IGGlblInputWindowID, &BBox);
		}
		IntrWndwRedrawAll();
	        break;
	    case 7: /* Headers. */
		GlblDrawHeader = !GlblDrawHeader;
		IntrWndwSetDrawHeader(IGGlblViewWindowID, GlblDrawHeader);
		IntrWndwSetDrawHeader(IGGlblTransWindowID, GlblDrawHeader);
		if (HasInputWindow)
		    IntrWndwSetDrawHeader(IGGlblInputWindowID, GlblDrawHeader);
		IntrWndwRedrawAll();
		break;
	}
    }
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
    IGSaveCurrentMat(ViewMode, IG_DEFAULT_IRIT_MAT);
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
    fprintf(stderr, IRIT_EXP_STR("Error: %s\n"), Msg);
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
    fprintf(stderr,
	    IRIT_EXP_STR("Yes/No message box not implemented:\n%s\nreturning yes..."), Msg);

    return TRUE;
}
