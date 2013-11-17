/*****************************************************************************
*   An SGI 4D driver using GL.						     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <gl/gl.h>
#include <gl/device.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifdef _AIX
#    include <fcntl.h>
#else
#    include <sys/fcntl.h>
#endif
#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "xgldrvs.h"

/* Interactive menu setup structure: */
#define INTERACT_NUM_OF_STRINGS		4
#define INTERACT_NUM_OF_SUB_WNDWS	17
#define IN_VIEW_WINDOW(x, y)	((x) >= ViewWinLeft && \
				 (y) >= ViewWinLow && \
				 (x) <= ViewWinLeft + ViewWinWidth && \
				 (y) <= ViewWinLow + ViewWinHeight)
#define IN_TRANS_WINDOW(x, y)   ((x) >= TransWinLeft && \
				 (y) >= TransWinLow && \
				 (x) <= TransWinLeft + TransWinWidth && \
				 (y) <= TransWinLow + TransWinHeight)

typedef struct InteractString {
    IrtRType X, Y;
    int Color;
    char *Str;
} InteractString;
typedef struct InteractSubWindow {
    IrtRType X, Y;					   /* Center points. */
    int Color;
    IGGraphicEventType Event;
    int TextInside; /* If TRUE, Str will be in window, otherwise left to it. */
    char *Str;
} InteractSubWindow;
typedef struct InteractWindowStruct {	 /* The interactive menu structures. */
    /* Rotate, Translate, Scale strings: */
    InteractString Strings[INTERACT_NUM_OF_STRINGS];
    InteractSubWindow SubWindows[INTERACT_NUM_OF_SUB_WNDWS];
} InteractWindowStruct;

#define INTERACT_SUB_WINDOW_WIDTH  0.8		 /* Relative to window size. */
#define INTERACT_SUB_WINDOW_HEIGHT 0.04

STATIC_DATA int
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    GlblStateMenu = 0;
STATIC_DATA long
    TransWinID = 0,
    TransWinWidth = 100,
    TransWinWidth2 = 50,
    TransWinHeight = 100,
    TransWinLow = 0,
    TransWinLeft = 0;

/* Interactive mode menu set up structure is define below: */
STATIC_DATA InteractWindowStruct InteractMenu = {
    { { 0.5, 0.81, IG_IRIT_RED,		"Rotate" },
      { 0.5, 0.65, IG_IRIT_GREEN,	"Translate" },
      { 0.5, 0.49, IG_IRIT_CYAN,	"Scale" },
      { 0.5, 0.41, IG_IRIT_LIGHTGREEN,	"Clip Plane" },
    },
    { { 0.5, 0.93, IG_IRIT_YELLOW, IG_EVENT_SCR_OBJ_TGL,	TRUE,  "Screen Coords." },
      { 0.5, 0.87, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_TGL,TRUE,  "Perspective" },
      { 0.5, 0.83, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_Z,	FALSE, "Z" },
      { 0.5, 0.75, IG_IRIT_RED,    IG_EVENT_ROTATE_X,		FALSE, "X" }, /* Rot */
      { 0.5, 0.71, IG_IRIT_RED,    IG_EVENT_ROTATE_Y,		FALSE, "Y" },
      { 0.5, 0.67, IG_IRIT_RED,    IG_EVENT_ROTATE_Z,		FALSE, "Z" },
      { 0.5, 0.59, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_X,	FALSE, "X" }, /* Trans */
      { 0.5, 0.55, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Y,	FALSE, "Y" },
      { 0.5, 0.51, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Z,	FALSE, "Z" },
      { 0.5, 0.43, IG_IRIT_CYAN,   IG_EVENT_SCALE,		FALSE, "" },  /* Scale */
      { 0.5, 0.35, IG_IRIT_LIGHTGREEN, IG_EVENT_NEAR_CLIP,	FALSE,  "" },
      { 0.5, 0.31, IG_IRIT_LIGHTGREEN, IG_EVENT_FAR_CLIP,	FALSE,  "" },

      { 0.5, 0.23, IG_IRIT_YELLOW, IG_EVENT_SAVE_MATRIX,	TRUE,  "Save Matrix" },
      { 0.5, 0.19, IG_IRIT_YELLOW, IG_EVENT_SUBMIT_MATRIX,	TRUE,  "Submit Matrix" },
      { 0.5, 0.13, IG_IRIT_YELLOW, IG_EVENT_PUSH_MATRIX,	TRUE,  "Push Matrix" },
      { 0.5, 0.09, IG_IRIT_YELLOW, IG_EVENT_POP_MATRIX,		TRUE,  "Pop Matrix" },
      { 0.5, 0.03, IG_IRIT_WHITE,  IG_EVENT_QUIT,		TRUE,  "Quit" },
    }
};

static void SetColorIndex(int color);
static void SetTransformWindow(void);
static void RedrawTransformWindow(void);
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int BlockForEvent);
static void DrawText(char *Str, long PosX, long PosY);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of xgldrvs - SGI's GL graphics driver of IRIT.          	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("x11drvs", argc, argv);

    SetViewWindow(argc, argv);
    IGRedrawViewWindow();
    SetTransformWindow();
    RedrawTransformWindow();

    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);
    qdevice(LEFTSHIFTKEY);
    qdevice(RIGHTSHIFTKEY);

    /* The default drawing window is the view window. */
    winset(ViewWinID);

    setbell(1);						   /* Make it short. */

    IGCreateStateMenu();

    while ((Event = GetGraphicEvent(ChangeFactor, TRUE)) != IG_EVENT_QUIT) {
	ChangeFactor[0] *= IGGlblChangeFactor;
	ChangeFactor[1] *= IGGlblChangeFactor;

	if (IGProcessEvent(Event, ChangeFactor))
	    IGRedrawViewWindow();
	else if (IGGlblLastLowResDraw)
	    IGRedrawViewWindow();
    }

    if (IGGlblIOHandle >= 0)
        IPCloseStream(IGGlblIOHandle, TRUE);

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
* Optionally construct a state pop up menu for the driver, if has one.       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCreateStateMenu                                                        M
*****************************************************************************/
void IGCreateStateMenu(void)
{
    if (GlblStateMenu) 
        freepup(GlblStateMenu);

    GlblStateMenu = newpup();

    addtopup(GlblStateMenu, " Set Up %t", 0);
    addtopup(GlblStateMenu, "Rht-On, Sft-Rht-Off%l", 0);
    addtopup(GlblStateMenu, "Mouse Sensitive%l", 0);
    addtopup(GlblStateMenu, IGGlblTransformMode == IG_TRANS_SCREEN ?
				"Screen Trans." : "Object Trans", 0);
    addtopup(GlblStateMenu,
	     IGGlblContinuousMotion ? "Cont Motion" : "Regular Motion", 0);
    addtopup(GlblStateMenu, IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				"Perspective" : "Orthographic", 0);
    addtopup(GlblStateMenu,
	     IGGlblDepthCue ? "Depth Cue" : "No Depth Cue", 0);
    addtopup(GlblStateMenu,
	     IGGlblCacheGeom ? "Cache Geom" : "No Geom Cache", 0);

    switch (IGGlblDrawStyle) {
	case IG_STATE_DRAW_STYLE_WIREFRAME:
	    addtopup(GlblStateMenu, "Draw Wireframe", 0);
	    break;
	case IG_STATE_DRAW_STYLE_SOLID:
	    addtopup(GlblStateMenu, "Draw Solid", 0);
	    break;
	case IG_STATE_DRAW_STYLE_POINTS:
	    addtopup(GlblStateMenu, "Draw Points", 0);
	    break;
    }
    switch (IGGlblShadingModel) {
	case IG_SHADING_NONE:
	default:
	    addtopup(GlblStateMenu, "No Shading");
	    break;
	case IG_SHADING_FLAT:
	    addtopup(GlblStateMenu, "Flat Shading");
	    break;
	case IG_SHADING_BACKGROUND:
	    addtopup(GlblStateMenu, "Background Shading");
	    break;
	case IG_SHADING_GOURAUD:
	    addtopup(GlblStateMenu, "Gouraud Shading");
	    break;
	case IG_SHADING_PHONG:
	    addtopup(GlblStateMenu, "Phong Shading");
	    break;
    }
    addtopup(GlblStateMenu,
	     IGGlblBackFaceCull ? "Cull Back Face" : "No Cull Back Face", 0);
    addtopup(GlblStateMenu,
	     IGGlblDoDoubleBuffer ? "Double Buffer" : "Single Buffer", 0);

    switch (IGGlblAntiAliasing) {
	case IG_STATE_ANTI_ALIAS_OFF:
	    addtopup(GlblStateMenu, "No Anti Aliasing%l", 0);
	    break;
	case IG_STATE_ANTI_ALIAS_ON:
	    addtopup(GlblStateMenu, "Anti Aliasing%l", 0);
	    break;
	case IG_STATE_ANTI_ALIAS_BLEND:
	    addtopup(GlblStateMenu, "Blending%l", 0);
	    break;
    }

    addtopup(GlblStateMenu,
	     IGGlblDrawInternal ? "Draw Internal Edges" : "No Internal Edges", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawVNormal ? "Draw Vrtx Normals" : "No Vrtx Normals", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawPNormal ? "Draw Poly Normals" : "No Poly Normals", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawPolygons ? "Draw Polygons%l" : "No Polygons%l", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceMesh ? "Draw Srf Mesh" : "No Srf Mesh", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceWire ? "Draw Srf Wireframe" : "No Srf Wireframe", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceBndry ? "Draw Srf Bndry" : "No Srf Bndry", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceSilh ? "Draw Srf Silhs" : "No Srf Silhs", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfacePoly ? "Draw Srf Polygons" : "No Srf Polygons", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceSketch ? "Draw Srf Sketch" : "No Srf Sketch", 0);
    addtopup(GlblStateMenu,
	     IGGlblDrawSurfaceRflctLns ? "Draw Srf RflctLn" : "No Srf RflctLn", 0);
    addtopup(GlblStateMenu,
	     IGGlblFourPerFlat ? "Four Per Flat%l" : "Two Per Flat%l", 0);
    addtopup(GlblStateMenu, "Num Isolines%l", 0);
    addtopup(GlblStateMenu, "Res Polygons%l", 0);
    addtopup(GlblStateMenu, "Res Polylines%l", 0);
    addtopup(GlblStateMenu, "Length Vectors%l", 0);
    addtopup(GlblStateMenu, "Width Lines%l", 0);
    addtopup(GlblStateMenu, "Width Points%l", 0);
    addtopup(GlblStateMenu, "Front View", 0);
    addtopup(GlblStateMenu, "Side View", 0);
    addtopup(GlblStateMenu, "Top View", 0);
    addtopup(GlblStateMenu, "Isometry View%l", 0);
    addtopup(GlblStateMenu, "Clear View Area%l", 0);
    addtopup(GlblStateMenu, "Animation", 0);
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
    int Color[3];

    if (color < 0 || color > IG_MAX_COLOR)
        color = IG_IRIT_WHITE;

    Color[0] = AttrIritColorTable[color][0];
    Color[1] = AttrIritColorTable[color][1];
    Color[2] = AttrIritColorTable[color][2];

    IGSetColorRGB(Color);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets up and draw a transformation window.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetTransformWindow(void)
{
    long PrefPos[4];

#ifndef _AIX
    foreground();
#endif

    if (sscanf(IGGlblTransPrefPos, "%ld,%ld,%ld,%ld",
	       &PrefPos[0], &PrefPos[1], &PrefPos[2], &PrefPos[3]) == 4)
	prefposition(PrefPos[0], PrefPos[1], PrefPos[2], PrefPos[3]);
    else if (sscanf(IGGlblTransPrefPos, "%ld,%ld",
		    &PrefPos[0], &PrefPos[1]) == 2)
	prefsize(PrefPos[0], PrefPos[1]);
    winopen("Poly3dTrans");
    winconstraints();
    wintitle("xGLdrvs");
    RGBmode();
    gconfig();
    getorigin(&TransWinLeft, &TransWinLow);
    getsize(&TransWinWidth, &TransWinHeight);
    TransWinWidth2 = TransWinWidth / 2;
    TransWinID = winget();

    IGSetColorRGB(IGGlblBackGroundColor);
    clear();

    /* This is wierd. without the sleep the gl get mixed up between the two  */
    /* windows. If you have any idea why, let me know...		     */
    sleep(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws the transformation window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   RedrawTransformWindow                                                    *
*****************************************************************************/
static void RedrawTransformWindow(void)
{
    int i;
    long SubTransPosX, SubTransPosY, SubTransWidth, SubTransHeight;

    /* Make sure the menu is consistent with internatal data. */
    InteractMenu.SubWindows[0].Str =
	IGGlblTransformMode == IG_TRANS_OBJECT ? "Object Coords."
					     : "Screen Coords.";
    InteractMenu.SubWindows[1].Str =
	IGGlblViewMode == IG_VIEW_PERSPECTIVE ? "Perspective" : "Orthographic";

    winset(TransWinID);                /* Draw in the transformation window. */

    SubTransWidth = (int) (TransWinWidth * INTERACT_SUB_WINDOW_WIDTH);
    SubTransHeight = (int) (TransWinHeight * INTERACT_SUB_WINDOW_HEIGHT);
    SubTransPosX = (TransWinWidth - SubTransWidth) / 2;

    IGSetColorRGB(IGGlblBackGroundColor);
    clear();

    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
	SetColorIndex(InteractMenu.SubWindows[i].Color);
	SubTransPosY = (int) (TransWinHeight * InteractMenu.SubWindows[i].Y);

	move2i(SubTransPosX, SubTransPosY);
	draw2i(SubTransPosX + SubTransWidth, SubTransPosY);
	draw2i(SubTransPosX + SubTransWidth, SubTransPosY + SubTransHeight);
	draw2i(SubTransPosX, SubTransPosY + SubTransHeight);
	draw2i(SubTransPosX, SubTransPosY);
	if (InteractMenu.SubWindows[i].TextInside) {
	    DrawText(InteractMenu.SubWindows[i].Str,
		     TransWinWidth / 2,
		     SubTransPosY + SubTransHeight / 2);
	}
	else {
	    DrawText(InteractMenu.SubWindows[i].Str,
		     (TransWinWidth - SubTransWidth) / 3,
		     SubTransPosY + SubTransHeight / 2);
	    move2i(SubTransPosX + SubTransWidth / 2, SubTransPosY);
	    draw2i(SubTransPosX + SubTransWidth / 2,
		   SubTransPosY + SubTransHeight);
	}
    }

    for (i = 0; i < INTERACT_NUM_OF_STRINGS; i++) {
	SetColorIndex(InteractMenu.Strings[i].Color);
	DrawText(InteractMenu.Strings[i].Str,
		 (int) (InteractMenu.Strings[i].X * TransWinWidth),
		 (int) (InteractMenu.Strings[i].Y * TransWinHeight));
    }

    winset(ViewWinID);             /* Go back to the default drawing window. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Handles input events                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   ChangeFactor:        A continuous numeric value between -1 and 1. This   *
*			 value will be used to set amount of event such as   *
*			 rotation or translation. In some events it can      *
*			 contain a vector value.			     *
*   BlockForEvent:	 If TRUE, blocks until event is recieved.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int BlockForEvent)
{
    STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    STATIC_DATA long
	LastX = -1,
	LastY = -1;
    STATIC_DATA int
	ShiftPressed = FALSE;
    int i, TransformRequest,
        LeftButtonIsPressed = getbutton(LEFTMOUSE) == 1,
        RightButtonIsPressed = getbutton(RIGHTMOUSE) == 1;
    IGGraphicEventType
	RetVal = IG_EVENT_NONE;
    short data;
    long x, y;
    IrtRType XPos, YPos;

    ChangeFactor[0] = ChangeFactor[1] = 0.0;

    /* Allow continuous drag on following events only: */
    if (LeftButtonIsPressed && !IG_IS_DRAG_EVENT(LastEvent)) {
	while (getbutton(LEFTMOUSE) == 1);
	LeftButtonIsPressed = FALSE;
    }
    if (RightButtonIsPressed && !IG_IS_DRAG_EVENT(LastEvent)) {
	while (getbutton(RIGHTMOUSE) == 1);
	RightButtonIsPressed = FALSE;
    }

    IGGlblManipulationActive = LeftButtonIsPressed || RightButtonIsPressed;

    if (LeftButtonIsPressed) {
	/* Allow leaving the window if still pressed, and use last event     */
	/* as the returned event. Note we wait until current position is     */
	/* different from last one to make sure we do something.             */
	switch (LastEvent) {
	    case IG_EVENT_PERS_ORTHO_Z:
	    case IG_EVENT_ROTATE_X:
	    case IG_EVENT_ROTATE_Y:
	    case IG_EVENT_ROTATE_Z:
	    case IG_EVENT_TRANSLATE_X:
	    case IG_EVENT_TRANSLATE_Y:
	    case IG_EVENT_TRANSLATE_Z:
	    case IG_EVENT_SCALE:
	    case IG_EVENT_NEAR_CLIP:
	    case IG_EVENT_FAR_CLIP:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       getbutton(LEFTMOUSE) == 1);

	        if (x != LastX) {
		    *ChangeFactor = (((IrtRType) x) - LastX) / TransWinWidth2;
		    LastX = x;
		    return LastEvent;
		}
		else
		    LeftButtonIsPressed = FALSE;
		break;
	    case IG_EVENT_ROTATE:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       (y = getvaluator(MOUSEY)) == LastY &&
		       getbutton(LEFTMOUSE) == 1);
		x = getvaluator(MOUSEX);
		y = getvaluator(MOUSEY);

	        if (x != LastX || y != LastY) {
		    ChangeFactor[0] = (x - LastX);
		    ChangeFactor[1] = (y - LastY);

		    LastX = x;
		    LastY = y;
		    return LastEvent;
		}
		else
		    LeftButtonIsPressed = FALSE;
		break;
	    default:
		break;
	}
    }

    if (RightButtonIsPressed) {
	/* Allow leaving the window if still pressed, and use last event     */
	/* as the returned event. Note we wait until current position is     */
	/* different from last one to make sure we do something.             */
	switch (LastEvent) {
	    case IG_EVENT_TRANSLATE:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       (y = getvaluator(MOUSEY)) == LastY &&
		       getbutton(RIGHTMOUSE) == 1);
		x = getvaluator(MOUSEX);
		y = getvaluator(MOUSEY);

		if (x != LastX || y != LastY) {
		    ChangeFactor[0] = (x - LastX);
		    ChangeFactor[1] = (y - LastY);

		    LastX = x;
		    LastY = y;
		    return LastEvent;
		}
		else
		    RightButtonIsPressed = FALSE;
	    default:
		break;
	}
    }

    LastEvent = IG_EVENT_NONE;

    IGGlblManipulationActive = FALSE;
    if (IGGlblLastLowResDraw)
	IGRedrawViewWindow();

    do {
	/* Wait for left button to be pressed in the Trans window. Note this */
	/* is the loop we are going to cycle in idle time.		     */
	for (TransformRequest = FALSE; !TransformRequest; ) {
	    x = getvaluator(MOUSEX);
	    y = getvaluator(MOUSEY);

	    if (qtest()) {	              /* Any external event occured? */
		switch (qread(&data)) {
		    case RIGHTSHIFTKEY:
		    case LEFTSHIFTKEY:
			ShiftPressed = data != 0;
		        break;
		    case REDRAW:
			if (data == ViewWinID) {
			    getorigin(&ViewWinLeft, &ViewWinLow);
			    getsize(&ViewWinWidth, &ViewWinHeight);
			    ViewWinWidth2 = ViewWinWidth / 2;
			    ViewWinHeight2 = ViewWinHeight / 2;
			    if (ViewWinWidth > ViewWinHeight) {
				int i = (ViewWinWidth - ViewWinHeight) / 2;

				viewport((Screencoord) 0,
					 (Screencoord) ViewWinWidth,
					 (Screencoord) -i,
					 (Screencoord) (ViewWinWidth - i));
			    }
			    else {
				int i = (ViewWinHeight - ViewWinWidth) / 2;

				viewport((Screencoord) -i,
					 (Screencoord) (ViewWinHeight - i),
					 (Screencoord) 0,
					 (Screencoord) ViewWinHeight);
			    }
			    ortho2(-0.5, ViewWinWidth - 0.5,
				   -0.5, ViewWinHeight - 0.5);
			    IGRedrawViewWindow();
			}
			else if (data == TransWinID) {
			    winset(TransWinID);
			    getorigin(&TransWinLeft, &TransWinLow);
			    getsize(&TransWinWidth, &TransWinHeight);
			    reshapeviewport();
			    ortho2(-0.5, TransWinWidth - 0.5,
				   -0.5, TransWinHeight - 0.5);
			    TransWinWidth2 = TransWinWidth / 2;
			    RedrawTransformWindow();
			    winset(ViewWinID);
			}
			break;
		    case RIGHTMOUSE:
			if (data) {			/* Mouse press down. */
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN3DOWN);
				continue;
			    }

			    if (IN_TRANS_WINDOW(x, y)) {
				if (IGHandleState(dopup(GlblStateMenu),
						  ShiftPressed ? IG_STATE_DEC
							       : IG_STATE_INC,
						  FALSE)) {
				    *ChangeFactor = 0.0;
				    return IG_EVENT_STATE;
				}
			    }
			    else if (IN_VIEW_WINDOW(x, y)) {
				RetVal = IG_EVENT_TRANSLATE;
				TransformRequest = TRUE;
			    }				
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		    case MIDDLEMOUSE:
			if (data) {			/* Mouse press down. */
			    IPObjectStruct *PObj;

			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN2DOWN);
				continue;
			    }

			    PObj = IGHandlePickEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
					    IG_PICK_ANY);

			    if (ReturnPickedObject) {
			        IPObjectStruct *PObjDump;

				if (PObj != NULL) {
				    if (ReturnPickedObjectName)
				        PObjDump = IPGenStrObject("_PickName_",
								  PObj -> Name,
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
			    else 
			        printf("Pick event found \"%s\"      \r",
				       PObj == NULL ? "nothing" : PObj -> Name);
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		    case LEFTMOUSE:
			if (data) {			/* Mouse press down. */
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN1DOWN);
				continue;
			    }
			    TransformRequest = TRUE;

			    if (IN_VIEW_WINDOW(x, y)) {
				RetVal = IG_EVENT_ROTATE;
			    }				
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		}
		continue;
	    }

	    /* Maybe we have something in communication socket. */
	    if (!IGGlblStandAlone &&
		IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList))
		IGRedrawViewWindow();

	    if (BlockForEvent) {
	        if (IGGlblContinuousMotion)
		    IGHandleContinuousMotion();
		else
		    IritSleep(10);     /* So we do not use all CPU on idle. */
	    }
	    else
		break;
	}

	if (!TransformRequest && !BlockForEvent)
	    return RetVal;

	LastX = x;
	LastY = y;

	if (IN_TRANS_WINDOW(x, y)) {
	    x -= TransWinLeft;
	    y -= TransWinLow;

	    XPos = ((IrtRType) x) / TransWinWidth;
	    YPos = ((IrtRType) y) / TransWinHeight;

	    /* Make sure we are in bound in the X direction. */
	    if (XPos < (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0 ||
		XPos > 1.0 - (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0)
	        continue;

	    /* Now search the sub window the event occured in. */
	    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
		if (InteractMenu.SubWindows[i].Y <= YPos &&
		    InteractMenu.SubWindows[i].Y +
		         INTERACT_SUB_WINDOW_HEIGHT >= YPos) {
		    RetVal = InteractMenu.SubWindows[i].Event;
		    break;
		}
	    }
	    if (i == INTERACT_NUM_OF_SUB_WNDWS)
	        continue;

	    /* Take care of special cases in which window should be updated. */
	    switch (RetVal) {
		case IG_EVENT_SCR_OBJ_TGL:
		    IGGlblTransformMode =
		        IGGlblTransformMode == IG_TRANS_OBJECT ?
					       IG_TRANS_SCREEN :
					       IG_TRANS_OBJECT;
		    RedrawTransformWindow();
		    break;
		case IG_EVENT_PERS_ORTHO_TGL:
		    IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
						       IG_VIEW_ORTHOGRAPHIC :
						       IG_VIEW_PERSPECTIVE;
		    RedrawTransformWindow();
		    break;
		default:
		    break;
	    }

	    *ChangeFactor = (((IrtRType) x) - TransWinWidth2) / TransWinWidth2;
	}
    }
    while (RetVal == IG_EVENT_NONE);

    LastEvent = RetVal;

    return RetVal;
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
    STATIC_DATA int
	FirstTime = TRUE;

    if (FirstTime) {
	STATIC_DATA unsigned short Cross1Cursor[16] = {
	    0x0240, 0x0240, 0x0240, 0x0240,
	    0x0240, 0x0240, 0xffff, 0x0240,
	    0x0240, 0xffff, 0x0240, 0x0240,
	    0x0240, 0x0240, 0x0240, 0x0240,
	};
	STATIC_DATA unsigned short Cross2Cursor[16] = {
	    0x8001, 0x4002, 0x2004, 0x1008,
	    0x0810, 0x0420, 0x0240, 0x0180,
	    0x0180, 0x0240, 0x0420, 0x0810,
	    0x1008, 0x2004, 0x4002, 0x8001,
	};

	drawmode(CURSORDRAW);
	mapcolor(1, 255, 0, 0);
	defcursor(1, Cross1Cursor);
	curorigin(1, 8, 8);
	defcursor(2, Cross2Cursor);
	curorigin(2, 8, 8);
	drawmode(NORMALDRAW);
    }

    switch (PickEntity) {
	case IG_PICK_ENTITY_DONE:
	    /* Restore the cursor. */
            setcursor(0, 0, 0);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */
            setcursor(1, 0, 0);

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */
            setcursor(2, 0, 0);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
    }
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
    int UpdateView = TRUE;

    switch (State) {
	case IG_STATE_SCR_OBJ_TGL:
	    IGStateHandler(State, StateStatus, Refresh);
	    RedrawTransformWindow();
	    UpdateView = FALSE;
	    break;
	case IG_STATE_PERS_ORTHO_TGL:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    RedrawTransformWindow();
	    break;
	case IG_STATE_DEPTH_CUE:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    if (IGGlblDepthCue) {
#		ifndef _AIX
		    glcompat(GLC_ZRANGEMAP, 1);
#		endif /* _AIX */
		lRGBrange(0, 0, 0, 255, 255, 255,
			  0x0, 0x7fffff);
		depthcue(IGGlblDepthCue);
	    }
	    else
		depthcue(FALSE);
	    break;
	case IG_STATE_DOUBLE_BUFFER:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    winset(ViewWinID);
	    if (IGGlblDoDoubleBuffer)
		doublebuffer();
	    else
		singlebuffer();
	    gconfig();
	    break;
	case IG_STATE_ANTI_ALIASING:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
#	    ifndef _AIX
		if (getgdesc(GD_PNTSMOOTH_RGB) == 0 ||
		    getgdesc(GD_BITS_NORM_DBL_CMODE) < 8)
		    IGGlblAntiAliasing = FALSE;
		else {
		    subpixel(IGGlblAntiAliasing);
		}
#	    endif /* _AIX */
	    break;
	default:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    break;
    }

    IGCreateStateMenu();

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draws text centered at the given position.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:        String to draw.                                              *
*   PosX, PosY: Location to draw at.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawText(char *Str, long PosX, long PosY)
{
    long Width = strwidth(Str);

    cmov2s((Scoord) (PosX - Width / 2),
	   (Scoord) (PosY - (getheight() / 2 - getdescender())));
    charstr(Str);
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
    ringbell();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation. Senses the event queue of X11.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we need to abort, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    if (qtest()) {
	short data;
	long
	    dev = qread(&data);

	if (dev == RIGHTMOUSE) {
	    fprintf(stderr, "\nAnimation was interrupted by the user.\n");
	    Anim -> StopAnim = TRUE;
	}
	else if (dev == LEFTMOUSE) {
	    IGGraphicEventType Event;
	    IrtRType ChangeFactor[2];

	    qenter((Device) dev, data);

	    winset(TransWinID);
	    Event = GetGraphicEvent(ChangeFactor, FALSE);
	    ChangeFactor[0] *= IGGlblChangeFactor;
	    ChangeFactor[1] *= IGGlblChangeFactor;
	    winset(ViewWinID);

	    if (Event != IG_EVENT_NONE) {
		if (IGProcessEvent(Event, ChangeFactor))
		    IGRedrawViewWindow();
	    }
	}
    }

    return Anim -> StopAnim;
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
    fprintf(stderr, "%s\n", Msg);
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
*   IGCrvCnstParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvCnstParamUpdateWidget(void) 
{
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
    fprintf(stderr, "%s\n", Msg);
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
    fprintf(stderr, "%s\n", Msg);
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
    fprintf(stderr, "Error: %s\n", Msg);
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
    char Line[LINE_LEN];

    do {
        fprintf(stderr, "%s <Y/N>:", Msg);
	fgets(Line, LINE_LEN - 1, stdin);
    }
    while (Line[0] != 'y' && Line[0] != 'Y' &&
	   Line[0] != 'n' && Line[0] != 'N');
    
    return Line[0] == 'y' || Line[0] != 'Y';
}
