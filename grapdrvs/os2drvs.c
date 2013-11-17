/****************************************************************************
*   An OS2 2.x driver.							    *
*									    *
* Written by:  Gershon Elber			     Ver 0.1, October 1993. *
*****************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology               *
****************************************************************************/

#include <stdio.h>

#define INCL_WINSTDFILE
#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_DOSPROCESS
#define INCL_PM
#define INCL_BASE
#include <os2.h>

#include <stdio.h>

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "os2drvs.h"

#define ID_VIEW		1
#define ID_TRANS	2
#define ID_ANIM		3

#define DEFAULT_TRANS_WIDTH	200
#define DEFAULT_TRANS_HEIGHT	500
#define DEFAULT_VIEW_WIDTH	400
#define DEFAULT_VIEW_HEIGHT	400
                                   
#define ARGCV_LINE_LEN	1000
#define ARGCV_MAX_WORDS	100

#define SIGNED_SHORT1FROMMP(mp)	(SHORT1FROMMP(mp) > 32767 ? \
				     SHORT1FROMMP(mp) - 65536 : \
				     SHORT1FROMMP(mp))
#define SIGNED_SHORT2FROMMP(mp)	(SHORT2FROMMP(mp) > 32767 ? \
				     SHORT2FROMMP(mp) - 65536 : \
				     SHORT2FROMMP(mp))

#define RGB_COLOR(R, G, B)	(((R) << 16) + ((G) << 8) + (B))
#define GPI_MOVE(X, Y)	{ POINTL Pt; Pt.x = X; Pt.y = Y; GpiMove(hps, &Pt); }
#define GPI_LINE(X, Y)	{ POINTL Pt; Pt.x = X; Pt.y = Y; GpiLine(hps, &Pt); }
#define GPI_CHAR_STR_AT(Str, X, Y) { POINTL Pt; int l = strlen(Str); \
				     Pt.x = X - cxChar * (l + 1) / 2; \
				     Pt.y = Y - cyChar / 3; \
				     GpiCharStringAt(hps, &Pt, l, Str); }
#define OS2_MAP_X_COORD(x) ((int) (ViewWidth2 + x * ViewWidth2))
#define OS2_MAP_Y_COORD(y) ((int) (ViewHeight2 + y * ViewWidth2))

#define WIN_CHECK_MENU(Item, Val)	WinSendMsg(hwndMenu, MM_SETITEMATTR, \
						   MPFROM2SHORT(Item, TRUE), \
						   MPFROM2SHORT(MIA_CHECKED, \
								(Val)));
#define WIN_TEXT_MENU(Item, Val)	WinSendMsg(hwndMenu, MM_SETITEMTEXT, \
						   MPFROM2SHORT(Item, TRUE), \
						   MPFROMP(Val));

/* Interactive menu setup structure: */
#define INTERACT_NUM_OF_STRINGS		3
#define INTERACT_NUM_OF_SUB_WNDWS	15
#define INTERACT_SUB_WINDOW_WIDTH  0.8		 /* Relative to window size. */
#define INTERACT_SUB_WINDOW_HEIGHT 0.04

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

/* Interactive mode menu set up structure is define below: */
STATIC_DATA InteractWindowStruct InteractMenu =
{
  {
    { 0.5, 0.76, IG_IRIT_RED,   "Rotate" },
    { 0.5, 0.56, IG_IRIT_GREEN, "Translate" },
    { 0.5, 0.36, IG_IRIT_CYAN,  "Scale" },
  },
  {
    { 0.5, 0.92, IG_IRIT_YELLOW, IG_EVENT_SCR_OBJ_TGL,    TRUE,  "Screen Coords." },
    { 0.5, 0.84, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_TGL, TRUE,  "Perspective" },
    { 0.5, 0.79, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_Z,   FALSE, "Z" },
    { 0.5, 0.69, IG_IRIT_RED,    IG_EVENT_ROTATE_X,       FALSE, "X" },  /* Rot */
    { 0.5, 0.64, IG_IRIT_RED,    IG_EVENT_ROTATE_Y,       FALSE, "Y" },
    { 0.5, 0.59, IG_IRIT_RED,    IG_EVENT_ROTATE_Z,       FALSE, "Z" },
    { 0.5, 0.49, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_X,    FALSE, "X" },/* Trans */
    { 0.5, 0.44, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Y,    FALSE, "Y" },
    { 0.5, 0.39, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Z,    FALSE, "Z" },
    { 0.5, 0.29, IG_IRIT_CYAN,   IG_EVENT_SCALE,	  FALSE, "" }, /* Scale */
    { 0.5, 0.21, IG_IRIT_YELLOW, IG_EVENT_DEPTH_CUE,      TRUE,  "Depth Cue" },
    { 0.5, 0.16, IG_IRIT_YELLOW, IG_EVENT_SAVE_MATRIX,    TRUE,  "Save Matrix" },
    { 0.5, 0.12, IG_IRIT_YELLOW, IG_EVENT_PUSH_MATRIX,    TRUE,  "Push Matrix" },
    { 0.5, 0.08, IG_IRIT_YELLOW, IG_EVENT_POP_MATRIX,     TRUE,  "Pop Matrix" },
    { 0.5, 0.03, IG_IRIT_WHITE,  IG_EVENT_QUIT,	          TRUE,  "Quit" },
  }
};

STATIC_DATA int
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    GlblSliderIncrements = 0;
STATIC_DATA unsigned int
    TransWidth = DEFAULT_TRANS_WIDTH,
    TransHeight = DEFAULT_TRANS_HEIGHT,
    TransWidth2 = DEFAULT_TRANS_WIDTH / 2,
    ViewWidth2 = DEFAULT_VIEW_WIDTH / 2,
    ViewHeight2 = DEFAULT_VIEW_HEIGHT / 2;

STATIC_DATA LONG CrntColorLowIntensity, CrntColorHighIntensity,
    ColorsLowIntensity[IG_MAX_COLOR + 1],
    ColorsHighIntensity[IG_MAX_COLOR + 1];
STATIC_DATA HPS
    CurrentHps = 0;
STATIC_DATA HWND
    hwndMenu = 0,
    hwndFrame = 0,
    hwndClient = 0,
    hwndViewFrame = 0,
    hwndTransFrame = 0,
    hwndSlider = 0;
STATIC_DATA double
    GlblAnimGetTime = 0.0;

static void GetInputFromSocket(void *Data);
static MRESULT TransWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT ViewWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT AnimWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT AnimGetTimeWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT ShadeParamWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static void OS2DrvsAnimation(void *Data);
static void HandleGetOneNumber(IrtRType *t, HWND hwnd);
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int X,
					  int Y);
static void RedrawViewWindow(HWND hwnd);
static void RedrawTransformationWindow(HWND hwnd);
static void SetColorIndex(int c);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of os2drvs - OS2 2.x graphics driver of IRIT.            	     M
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
    STATIC_DATA ULONG flFrameFlags;
    STATIC_DATA char
	*szClientClass = "os2iritdrv.Client",
	*szViewClass = "os2iritdrv.View",
	*szTransClass = "os2iritdrv.Trans";
    int i, x1, x2, y1, y2,
	Xmin = 190,
	Ymin = 190,
	Width = 600,
	Height = 400;
    QMSG qMsg;
    HPS hps;
    HAB hab;
    HMQ hmq;
    RECTL rcl, DeskTopRcl;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("os2drvs", argc, argv);
    if (getenv("IRIT_DISPLAY_S") != NULL)
	IGGlblStandAlone = FALSE;

    if (IGGlblViewPrefPos &&
	sscanf(IGGlblViewPrefPos, "%d,%d,%d,%d", &x1, &x2, &y1, &y2) == 4) {
	Xmin = x1;
	Ymin = y1;
	Width = x2 - x1;
	Height = y2 - y1;
    }

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    WinRegisterClass(hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0L);
    WinRegisterClass(hab, szViewClass,   ViewWndProc,   CS_SIZEREDRAW, 0L);
    WinRegisterClass(hab, szTransClass,  TransWndProc,  CS_SIZEREDRAW, 0L);

    flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU  |
                   FCF_SIZEBORDER    | FCF_MINMAX   |
                   FCF_SHELLPOSITION | FCF_TASKLIST |
		   FCF_MENU	     | FCF_ACCELTABLE;

    hwndFrame = WinCreateStdWindow(
                    HWND_DESKTOP,
                    0,
                    &flFrameFlags,
                    szClientClass,
                    NULL,
                    0L,
                    0,
                    ID_OS2DRVS,
                    &hwndClient);

    WinQueryWindowRect(HWND_DESKTOP, &DeskTopRcl);

    if (DeskTopRcl.yTop < Ymin + Height)   /* Make sure top side is visible. */
	Ymin = DeskTopRcl.yTop - Height;

    WinSetWindowPos(hwndFrame, HWND_TOP, Xmin, Ymin, Width, Height,
				SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER);

    WinQueryWindowRect(hwndClient, &rcl);
    flFrameFlags &= (~FCF_TASKLIST & ~FCF_MENU & ~FCF_ACCELTABLE);

    hwndViewFrame = WinCreateWindow(
		    hwndClient,
		    szViewClass,
		    NULL,
		    WS_VISIBLE,
		    5,
		    5,
		    (SHORT) ((rcl.xRight * 4) / 5 - 10),
		    (SHORT) (rcl.yTop - 10),
		    hwndClient,
		    HWND_BOTTOM,
		    ID_VIEW,
		    NULL,
		    NULL);

    hwndTransFrame = WinCreateWindow(
		    hwndClient,
		    szTransClass,
		    NULL,
		    WS_VISIBLE,
		    (SHORT) ((rcl.xRight * 4) / 5),
		    5,
		    (SHORT) (rcl.xRight / 5 - 5),
		    (SHORT) (rcl.yTop - 10),
		    hwndClient,
		    HWND_BOTTOM,
		    ID_TRANS,
		    NULL,
		    NULL);

    if (hwndViewFrame == 0 || hwndTransFrame == 0) {
    	DosBeep(1000, 100);
    	exit(1);
    }

    /* Preallocate the colors that will be used frequently. */
    hps = WinBeginPaint(hwndFrame, 0, NULL);
    for (i = 0; i <= IG_MAX_COLOR; i++) {
	ColorsHighIntensity[i] =
	    GpiQueryColorIndex(hps, 0L,
		RGB_COLOR(AttrIritColorTable[i][0],
			  AttrIritColorTable[i][1],
			  AttrIritColorTable[i][2]));
	ColorsLowIntensity[i] =
	    GpiQueryColorIndex(hps, 0L,
		RGB_COLOR(AttrIritColorTable[i][0] / 2,
			  AttrIritColorTable[i][1] / 2,
			  AttrIritColorTable[i][2] / 2));
    }
    WinEndPaint(hps);

    /* Set up the reading socket as a secondary thread. */
    if (!IGGlblStandAlone) {
	if (_beginthread(GetInputFromSocket, NULL, 128 * 1024, NULL) == -1) {
	    WinMessageBox(HWND_DESKTOP, hwndFrame, "Failed to start thread\n",
			  NULL, 0, MB_OK);
	    exit(1);
	}
    }

    IGCreateStateMenu();	       /* Refresh the menu to current state. */

    while (TRUE) {
	if (WinPeekMsg(hab, &qMsg, 0L, 0, 0, PM_REMOVE))
	    WinDispatchMsg(hab, &qMsg);

	if (IGGlblNewDisplayObjects != NULL) {
	    IGHandleObjectsFromSocket(IGGlblViewMode,
				      IGGlblNewDisplayObjects,
				      &IGGlblDisplayList);
	    IGGlblNewDisplayObjects = NULL;
	    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
	}
	IritSleep(1);				      /* Do not use all CPU. */
    }

    WinDestroyWindow(hwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

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
    WIN_CHECK_MENU(IDM_TGLS_SCREEN, 
		   IGGlblTransformMode == IG_TRANS_SCREEN ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_PERSP,
		   IGGlblViewMode == IG_VIEW_PERSPECTIVE ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_DEPTH_CUE, IGGlblDepthCue ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_DOUBLE_BUFFER, IGGlblDoDoubleBuffer ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_DRAW_STYLE, IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_BFACE_CULL, IGGlblBackFaceCull ? MIA_CHECKED : 0);
    switch (IGGlblShadingModel) {
	case IG_SHADING_NONE:
	    WIN_TEXT_MENU(IDM_TGLS_SHADING_MODES, "No Shading ~Mode");
	    break;
	case IG_SHADING_FLAT:
	    WIN_TEXT_MENU(IDM_TGLS_SHADING_MODES, "Flat Shading ~Mode");
	    break;
	case IG_SHADING_BACKGROUND:
	    WIN_TEXT_MENU(IDM_TGLS_SHADING_MODES, "Back Ground ~Mode");
	    break;
	case IG_SHADING_GOURAUD:
	    WIN_TEXT_MENU(IDM_TGLS_SHADING_MODES, "Gouraud Shading ~Mode");
	    break;
	case IG_SHADING_PHONG:
	    WIN_TEXT_MENU(IDM_TGLS_SHADING_MODES, "Phong Shading ~Mode");
	    break;
    }
    WIN_CHECK_MENU(IDM_TGLS_INTERNAL, IGGlblDrawInternal ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_VRTX_NRML, IGGlblDrawVNormal ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_POLY_NRML, IGGlblDrawPNormal ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_CTL_MESH, IGGlblDrawSurfaceMesh ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_SRF_POLYS, IGGlblDrawSurfacePoly ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_SRF_ISOS, IGGlblDrawSurfaceWire ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_SRF_SKETCH, IGGlblDrawSurfaceSketch ? MIA_CHECKED : 0);
    WIN_CHECK_MENU(IDM_TGLS_4_PER_FLAT, IGGlblFourPerFlat ? MIA_CHECKED : 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A secondary thread that waits for input from socket.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:      Not used.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetInputFromSocket(void *Data)
{
    while (TRUE) {
	IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList);
	IritSleep(10);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Trans window call back drawing function.				     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT TransWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    STATIC_DATA int
	LastX = 0,
	Button1Down = FALSE;
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;
    int x, y;
    
    switch (msg) {
    	case WM_CREATE:
	    break;
	case WM_BUTTON1DOWN:
	    LastEvent = IG_EVENT_NONE;
	    x = SIGNED_SHORT1FROMMP(mp1);
	    y = SIGNED_SHORT2FROMMP(mp1);

	    if ((Event = GetGraphicEvent(ChangeFactor, x, y))
							   != IG_EVENT_NONE) {
		ChangeFactor[0] *= IGGlblChangeFactor;
		ChangeFactor[1] *= IGGlblChangeFactor;

		if (Event == IG_EVENT_QUIT) {
		    if (IGGlblIOHandle >= 0)
		        IPCloseStream(IGGlblIOHandle, TRUE);

		    exit(0);		     /* Not the nicest ways to quit. */
		}
		else {
		    if (IGProcessEvent(Event, ChangeFactor))
		        WinInvalidateRect(hwndViewFrame, NULL, TRUE);

		    /* Save the event in case drag operation is performed. */
		    LastEvent = Event;
		    LastX = x;
		}
	    }
	    Button1Down = IGGlblManipulationActive = TRUE;
	    WinSetCapture(HWND_DESKTOP, hwnd);
	    break;
	case WM_MOUSEMOVE:
	    if (Button1Down && LastEvent != IG_EVENT_NONE) {
		*ChangeFactor = IGGlblChangeFactor *
		      ((x = SIGNED_SHORT1FROMMP(mp1)) - ((IrtRType) LastX)) /
								   TransWidth2;
		if (IGProcessEvent(LastEvent, ChangeFactor))
		    WinInvalidateRect(hwndViewFrame, NULL, TRUE);
		LastX = x;
	    }
	    break;
	case WM_BUTTON1UP:
	    Button1Down = IGGlblManipulationActive = FALSE;
	    WinSetCapture(HWND_DESKTOP, 0);
	    WinInvalidateRect(hwndViewFrame, NULL, TRUE);/* Redraw at hires. */
	    break;
        case WM_PAINT:
	    RedrawTransformationWindow(hwnd);
	    break;
        case WM_COMMAND:
	    break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* View window call back drawing function.				     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT ViewWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    STATIC_DATA int
	LastX = 0,
	LastY = 0,
	ButtonDown = FALSE;
    IrtRType ChangeFactor[2];
    
    switch (msg) {
        case WM_PAINT:
	    RedrawViewWindow(hwnd);
	    break;
        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1)) {
            }
            break;
	case WM_BUTTON1DOWN:
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(SIGNED_SHORT1FROMMP(mp1),
				    SIGNED_SHORT2FROMMP(mp1),
				    IG_PICK_REP_BTN1DOWN);
		return 0;
	    }
	    if (MOUSEMSG(&msg)->fsInp & KC_SHIFT) {
		IPObjectStruct
		    *PObj = IGHandlePickEvent(SIGNED_SHORT1FROMMP(mp1),
					      SIGNED_SHORT2FROMMP(mp1),
					      IP_PICK_ANY);

		if (ReturnPickedObject) {
		    IPObjectStruct *PObjDump;

		    if (PObj != NULL) {
			if (ReturnPickedObjectName)
			    PObjDump = IPGenStrObject("_PickName_",
						      PObj -> Name, NULL);
			else
			    PObjDump = PObj;
		    }
		    else
			PObjDump = IPGenStrObject("_PickFail_",
						  "*** no object ***", NULL);

		    IPSocWriteOneObject(IGGlblIOHandle, PObjDump);
		    if (PObj != PObjDump)
			IPFreeObject(PObjDump);
		}
		else {
		    char Str[LINE_LEN];

		    sprintf(Str, "Found \"%s\"\n",
			    PObj == NULL ? "nothing" : PObj -> Name);
		    WinMessageBox(HWND_DESKTOP, hwndFrame, Str,
				  NULL, 0, MB_OK);
		}

		return 0;
	    }
	    LastEvent = IG_EVENT_ROTATE;
	    LastX = SIGNED_SHORT1FROMMP(mp1);
	    LastY = SIGNED_SHORT2FROMMP(mp1);
	    ButtonDown = IGGlblManipulationActive = TRUE;
	    WinSetCapture(HWND_DESKTOP, hwnd);
	    break;
	case WM_BUTTON2DOWN:
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(SIGNED_SHORT1FROMMP(mp1),
				    SIGNED_SHORT2FROMMP(mp1),
				    IG_PICK_REP_BTN2DOWN);
		return 0;
	    }
	    LastEvent = IG_EVENT_TRANSLATE;
	    LastX = SIGNED_SHORT1FROMMP(mp1);
	    LastY = SIGNED_SHORT2FROMMP(mp1);
	    ButtonDown = IGGlblManipulationActive = TRUE;
	    WinSetCapture(HWND_DESKTOP, hwnd);
	    break;
	case WM_MOUSEMOVE:
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(SIGNED_SHORT1FROMMP(mp1),
				    SIGNED_SHORT2FROMMP(mp1),
				    IG_PICK_REP_MOTION);
		return 0;
	    }
	    if (ButtonDown && LastEvent != IG_EVENT_NONE) {
		ChangeFactor[0] = (SIGNED_SHORT1FROMMP(mp1) - LastX) *
							 IGGlblChangeFactor;
		ChangeFactor[1] = (SIGNED_SHORT2FROMMP(mp1) - LastY) *
							 IGGlblChangeFactor;
		if (IGProcessEvent(LastEvent, ChangeFactor))
		    WinInvalidateRect(hwndViewFrame, NULL, TRUE);
		LastX = SIGNED_SHORT1FROMMP(mp1);
		LastY = SIGNED_SHORT2FROMMP(mp1);
	    }
	    break;
	case WM_BUTTON1UP:
	case WM_BUTTON2UP:
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent(SIGNED_SHORT1FROMMP(mp1),
				    SIGNED_SHORT2FROMMP(mp1),
				    IG_PICK_REP_BTN_UP);
		return 0;
	    }
	    WinSetCapture(HWND_DESKTOP, 0);
	    ButtonDown = IGGlblManipulationActive = FALSE;
	    WinInvalidateRect(hwndViewFrame, NULL, TRUE);/* Redraw at hires. */
	    break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
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
    switch (PickEntity) {
	case IG_PICK_ENTITY_DONE:
	    /* Restore the cursor. */

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Client (Frame) window call back drawing function.			     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    POINTL Pt;
    RECTL rcl;
    HPS hps;
    int Refresh;
    FILEDLG fild;
    STATIC_DATA int
	Resized = FALSE;

    switch (msg) {
	case WM_CREATE:
	    hwndMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       FID_MENU);
	    break;
        case WM_SIZE:
	    Resized = TRUE;
	    break;
        case WM_PAINT:
	    hps = WinBeginPaint(hwnd, 0, NULL);

	    WinQueryWindowRect(hwnd, &rcl);
	    GpiSetColor(hps, CLR_WHITE);
	    Pt.x = Pt.y = 0;
	    GpiMove(hps, &Pt);
	    Pt.x = rcl.xRight;
	    Pt.y = rcl.yTop;
	    GpiBox(hps, DRO_OUTLINEFILL, &Pt, 0, 0);

	    WinEndPaint(hps);

	    if (Resized) {
	    	Resized = FALSE;
	    	
	    	if (hwndViewFrame)
		    WinSetWindowPos(hwndViewFrame, HWND_BOTTOM, 5, 5,
				    (SHORT) ((rcl.xRight * 4) / 5 - 10),
				    (SHORT) (rcl.yTop - 10),
				    SWP_SIZE | SWP_MOVE);
		if (hwndTransFrame)
		    WinSetWindowPos(hwndTransFrame, HWND_BOTTOM,
				    (SHORT) (rcl.xRight * 4 / 5), 5,
				    (SHORT) (rcl.xRight / 5 - 5),
				    (SHORT) (rcl.yTop - 10),
				    SWP_SIZE | SWP_MOVE);
	    }
            break;
        case WM_COMMAND:
	    Refresh = FALSE;
            switch (SHORT1FROMMP(mp1)) {
		case IDM_FILE_SAVE:
		    IGSaveCurrentMat(IGGlblViewMode, NULL);
		    break;
		case IDM_FILE_SAVE_AS:
		    memset(&fild, 0, sizeof(FILEDLG));
		    fild.cbSize = sizeof(FILEDLG);
		    fild.pszTitle = "Save View Matrix";
		    fild.pszOKButton = "Save";
		    fild.fl = FDS_OPEN_DIALOG | FDS_CENTER ;
		    WinFileDlg(HWND_DESKTOP, hwndClient, &fild);

		    if (!IRT_STR_ZERO_LEN(fild.szFullFile))
			IGSaveCurrentMat(IGGlblViewMode, fild.szFullFile);
		    break;
		case IDM_FILE_CLEAR_VIEW:
		    Refresh = IGHandleState(IG_STATE_CLEAR_VIEW,
					    IG_STATE_ON, TRUE);
		    break;
		case IDM_FILE_QUIT:
		    exit(0);
		    break;
		case IDM_MOUSE_MORE:
		    IGHandleState(IG_STATE_MOUSE_SENSITIVE,
				  IG_STATE_INC, TRUE);
		    break;
		case IDM_MOUSE_LESS:
		    IGHandleState(IG_STATE_MOUSE_SENSITIVE,
				  IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_MORE_ISO:
		    Refresh = IGHandleState(IG_STATE_NUM_ISOLINES,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_LESS_ISO:
		    Refresh = IGHandleState(IG_STATE_NUM_ISOLINES,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_FINER_APPROX:
		    Refresh = IGHandleState(IG_STATE_POLY_APPROX,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_COARSER_APPROX:
		    Refresh = IGHandleState(IG_STATE_POLY_APPROX,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_SHORTER_VEC:
		    Refresh = IGHandleState(IG_STATE_LENGTH_VECTORS,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_LONGER_VEC:
		    Refresh = IGHandleState(IG_STATE_LENGTH_VECTORS,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_THIN_LINES:
		    Refresh = IGHandleState(IG_STATE_WIDTH_LINES,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_WIDE_LINES:
		    Refresh = IGHandleState(IG_STATE_WIDTH_LINES,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_TGLS_SCREEN:
		    IGHandleState(IG_STATE_SCR_OBJ_TGL, IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_PERSP:
		    Refresh = IGHandleState(IG_STATE_PERS_ORTHO_TGL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_DEPTH_CUE:
		    Refresh = IGHandleState(IG_STATE_DEPTH_CUE,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_DOUBLE_BUFFER:
		    Refresh = IGHandleState(IG_STATE_DOUBLE_BUFFER,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_DRAW_SOLID:
		    Refresh = IGHandleState(IG_STATE_DRAW_SOLID,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_BFACE_CULL:
		    Refresh = IGHandleState(IG_STATE_BACK_FACE_CULL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_SHADING_MODES:
		    Refresh = IGHandleState(IG_STATE_SHADING_MODEL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_INTERNAL:
		    Refresh = IGHandleState(IG_STATE_DRAW_INTERNAL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_VRTX_NRML:
		    Refresh = IGHandleState(IG_STATE_DRAW_VNORMAL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_POLY_NRML:
		    Refresh = IGHandleState(IG_STATE_DRAW_PNORMAL,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_CTL_MESH:
		    Refresh = IGHandleState(IG_STATE_DRAW_SRF_MESH,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_SRF_POLYS:
		    Refresh = IGHandleState(IG_STATE_DRAW_SRF_POLY,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_SRF_ISOS:
		    Refresh = IGHandleState(IG_STATE_DRAW_SRF_WIRE,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_SRF_SKETCH:
		    Refresh = IGHandleState(IG_STATE_DRAW_SRF_SKTCH,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_4_PER_FLAT:
		    Refresh = IGHandleState(IG_STATE_FOUR_PER_FLAT,
					    IG_STATE_TGL, TRUE);
		    break;

		case IDM_VIEW_FRONT:
		    Refresh = IGHandleState(IG_STATE_VIEW_FRONT,
					    IG_STATE_ON,, TRUE);
		    break;
		case IDM_VIEW_SIDE:
		    Refresh = IGHandleState(IG_STATE_VIEW_SIDE,
					    IG_STATE_ON, TRUE);
		    break;
		case IDM_VIEW_TOP:
		    Refresh = IGHandleState(IG_STATE_VIEW_TOP,
					    IG_STATE_ON, TRUE);
		    break;
		case IDM_VIEW_ISOMETRY:
		    Refresh = IGHandleState(IG_STATE_VIEW_ISOMETRY,
					    IG_STATE_ON, TRUE);
		    break;

		case IDM_EXTN_ANIMATION:
		    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP,
			      AnimWndProc, 0, ID_ANIM_FORM, NULL);
		    break;
		case IDM_EXTN_SHADE_PARAM:
		    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP,
			      ShadeParamWndProc, 0, ID_SHADE_PARAM_FORM, NULL);
		    break;
            }
	    if (Refresh)
		WinInvalidateRect(hwndViewFrame, NULL, FALSE);
            break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Animation (Frame) window call back drawing function.			     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT AnimWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    STATIC_DATA int
	LastSliderVal = -1;
    POINTL Pt;
    RECTL rcl;
    HPS hps;
    WNDPARAMS wprm;
    SLDCDATA sldcd;
    int i, j, Spacing;
    char Buffer[10];

    switch (msg) {
	case WM_CREATE:
	    break;
        case WM_INITDLG:
	    hwndSlider = WinWindowFromID(hwnd, ID_ANIM_SLIDER);

	    wprm.fsStatus = WPM_CTLDATA;
	    wprm.cbCtlData = sizeof(SLDCDATA);
	    wprm.pCtlData = &sldcd;

	    if (WinSendMsg(hwndSlider, WM_QUERYWINDOWPARAMS,
			   MPFROMP(&wprm), 0)) {
		GlblSliderIncrements = sldcd.usScale1Increments;
		Spacing = sldcd.usScale1Spacing;
	    }

	    for (i = 0; i <= GlblSliderIncrements; i++)
		WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 4), NULL);

	    for (i = 0; i <= GlblSliderIncrements; i += 10)
	        WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 8), NULL);
            WinSetPresParam(hwndSlider, PP_FONTNAMESIZE,
			    strlen(ID_ANIM_SLIDER_FONT) + 1,
			    ID_ANIM_SLIDER_FONT);

	    GMAnimFindAnimationTime(&IGAnimation, IGGlblDisplayList);

	    for (i = j = 0;
		 i <= GlblSliderIncrements;
		 i += GlblSliderIncrements / 3, j++) {
		IrtRType
		    t = (IGAnimation.StartT * (3 - j) +
		         IGAnimation.FinalT * j) / 3.0;

		sprintf(Buffer, "%4.2f", t);
		WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
			   MPFROMSHORT(i), MPFROMP(Buffer));
	    }
	    break;
        case WM_PAINT:
	    hps = WinBeginPaint(hwnd, 0, NULL);

	    WinQueryWindowRect(hwnd, &rcl);
	    GpiSetColor(hps, CLR_DARKGRAY);
	    Pt.x = Pt.y = 0;
	    GpiMove(hps, &Pt);
	    Pt.x = rcl.xRight;
	    Pt.y = rcl.yTop;
	    GpiBox(hps, DRO_OUTLINEFILL, &Pt, 0, 0);

	    WinEndPaint(hps);
            break;
        case WM_COMMAND:
        case WM_CONTROL:
	    switch (COMMANDMSG(&msg) -> cmd) {
		case ID_ANIM_SLIDER:
                    i = SHORT1FROMMR(WinSendDlgItemMsg(hwnd, ID_ANIM_SLIDER,
				SLM_QUERYSLIDERINFO,
                                MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					     SMA_INCREMENTVALUE),
                                0));
		    if (LastSliderVal != i) {
			IrtRType
			    t = i / 100.0;

			LastSliderVal = i;

			IGAnimation.RunTime =
			    (IGAnimation.FinalT * t +
			     IGAnimation.StartT * (1.0 - t));

			GMAnimDoSingleStep(&IGAnimation, IGGlblDisplayList);
		    }
		    break;
		case ID_ANIM_SAVE_FILE:
		    IGAnimation.SaveAnimation = !IGAnimation.SaveAnimation;
		    WinSendDlgItemMsg(hwnd, ID_ANIM_SAVE_FILE, BM_SETCHECK,
				      MPFROM2SHORT(IGAnimation.SaveAnimation,
						   0),
				      NULL);
		    break;
		case ID_ANIM_MIN_TIME:
		    HandleGetOneNumber(&IGAnimation.StartT, hwnd);
		    break;
		case ID_ANIM_MAX_TIME:
		    HandleGetOneNumber(&IGAnimation.FinalT, hwnd);
		    break;
		case ID_ANIM_TIME_STEP:
		    HandleGetOneNumber(&IGAnimation.Dt, hwnd);
		    break;
		case ID_ANIM_BEGIN:
		    if (_beginthread(OS2DrvsAnimation, NULL,
				     128 * 1024, NULL) == -1)
			WinMessageBox(HWND_DESKTOP, hwndFrame,
				      "Failed to start animation thread\n",
				      NULL, 0, MB_OK);
		    break;
		case ID_ANIM_STOP:
		    IGAnimation.StopAnim = TRUE;
		    break;
		case ID_ANIM_DISMISS:
                    WinDismissDlg(hwnd, FALSE);
		    break;
		default:
		    break;
	    }
            break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A secondary thread to execute an animation sequence.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:      Not used.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OS2DrvsAnimation(void *Data)
{
    GMAnimDoAnimation(&IGAnimation, IGGlblDisplayList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Shade Param (Frame) window call back drawing function.		     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT ShadeParamWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    int i, Spacing;
    POINTL Pt;
    RECTL rcl;
    HPS hps;
    WNDPARAMS wprm;
    SLDCDATA sldcd;

    switch (msg) {
	case WM_CREATE:
	    break;
        case WM_INITDLG:
	    hwndSlider = WinWindowFromID(hwnd, ID_SKETCH_SIL_POWER_SLIDER);

	    wprm.fsStatus = WPM_CTLDATA;
	    wprm.cbCtlData = sizeof(SLDCDATA);
	    wprm.pCtlData = &sldcd;

	    if (WinSendMsg(hwndSlider, WM_QUERYWINDOWPARAMS,
			   MPFROMP(&wprm), 0)) {
		GlblSliderIncrements = sldcd.usScale1Increments;
		Spacing = sldcd.usScale1Spacing;
	    }

	    for (i = 0; i <= GlblSliderIncrements; i++)
		WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 4), NULL);

	    for (i = 0; i <= GlblSliderIncrements; i += 10)
	        WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 8), NULL);
            WinSetPresParam(hwndSlider, PP_FONTNAMESIZE,
			    strlen(ID_ANIM_SLIDER_FONT) + 1,
			    ID_ANIM_SLIDER_FONT);

	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(0), MPFROMP("0.0 "));
	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(GlblSliderIncrements - 1), MPFROMP("1.0 "));
	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(GlblSliderIncrements / 2),
		       MPFROMP("Sil Power "));

	    hwndSlider = WinWindowFromID(hwnd, ID_SKETCH_SHD_POWER_SLIDER);

	    wprm.fsStatus = WPM_CTLDATA;
	    wprm.cbCtlData = sizeof(SLDCDATA);
	    wprm.pCtlData = &sldcd;

	    if (WinSendMsg(hwndSlider, WM_QUERYWINDOWPARAMS,
			   MPFROMP(&wprm), 0)) {
		GlblSliderIncrements = sldcd.usScale1Increments;
		Spacing = sldcd.usScale1Spacing;
	    }

	    for (i = 0; i <= GlblSliderIncrements; i++)
		WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 4), NULL);

	    for (i = 0; i <= GlblSliderIncrements; i += 10)
	        WinSendMsg(hwndSlider, SLM_SETTICKSIZE,
			   MPFROM2SHORT(i, 8), NULL);
            WinSetPresParam(hwndSlider, PP_FONTNAMESIZE,
			    strlen(ID_ANIM_SLIDER_FONT) + 1,
			    ID_ANIM_SLIDER_FONT);

	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(0), MPFROMP("0.0 "));
	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(GlblSliderIncrements - 1), MPFROMP("1.0 "));
	    WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		       MPFROMSHORT(GlblSliderIncrements / 2),
		       MPFROMP("Shade Power "));

	    switch (IGGlblShadingModel) {
		case IG_SHADING_NONE:
		    i = ID_SHADE_STYLE_NONE;
		    break;
	        case IG_SHADING_BACKGROUND:
		    i = ID_SHADE_STYLE_BACKGROUND;
		    break;
		case IG_SHADING_FLAT:
		    i = ID_SHADE_STYLE_FLAT;
		    break;
		case IG_SHADING_GOURAUD:
		    i = ID_SHADE_STYLE_GOURAUD;
		    break;
		case IG_SHADING_PHONG:
		    i = ID_SHADE_STYLE_PHONG;
		    break;
	    }
	    WinSendDlgItemMsg(hwnd, i, BM_SETCHECK,
			      MPFROM2SHORT(TRUE, 0), NULL);


	    switch (IGSketchParam.SketchType) {
		case IG_SKETCHING_SILHOUETTE:
		    i = ID_SHADE_SKETCH_SILS;
		    break;
		case IG_SKETCHING_CURVATURE:
		    i = ID_SHADE_SKETCH_CRVTR;
		    break;
		case IG_SKETCHING_ISO_PARAM:
		    i = ID_SHADE_SKETCH_ISOCRVS;
		    break;
	    }
	    WinSendDlgItemMsg(hwnd, i, BM_SETCHECK,
			      MPFROM2SHORT(TRUE, 0), NULL);
	    break;
        case WM_PAINT:
	    hps = WinBeginPaint(hwnd, 0, NULL);

	    WinQueryWindowRect(hwnd, &rcl);
	    GpiSetColor(hps, CLR_DARKGRAY);
	    Pt.x = Pt.y = 0;
	    GpiMove(hps, &Pt);
	    Pt.x = rcl.xRight;
	    Pt.y = rcl.yTop;
	    GpiBox(hps, DRO_OUTLINEFILL, &Pt, 0, 0);

	    WinEndPaint(hps);
            break;
        case WM_COMMAND:
        case WM_CONTROL:
	    switch (COMMANDMSG(&msg) -> cmd) {
		case ID_SHADE_SKETCH_SILS:
		    IGSketchParam.SketchType = IG_SKETCHING_SILHOUETTE;
		    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList, FALSE,
						     FALSE, TRUE, FALSE);
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_SKETCH_CRVTR:
		    IGSketchParam.SketchType = IG_SKETCHING_CURVATURE;
		    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList, FALSE,
						     FALSE, TRUE, FALSE);
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_SKETCH_ISOCRVS:
		    IGSketchParam.SketchType = IG_SKETCHING_ISO_PARAM;
		    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList, FALSE,
						     FALSE, TRUE, FALSE);
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_STYLE_NONE:
		    IGGlblShadingModel = IG_SHADING_NONE;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_STYLE_BACKGROUND:
		    IGGlblShadingModel = IG_SHADING_BACKGROUND;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_STYLE_FLAT:
		    IGGlblShadingModel = IG_SHADING_FLAT;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_STYLE_GOURAUD:
		    IGGlblShadingModel = IG_SHADING_GOURAUD;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_STYLE_PHONG:
		    IGGlblShadingModel = IG_SHADING_PHONG;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SKETCH_SIL_POWER_SLIDER:
                    i = SHORT1FROMMR(WinSendDlgItemMsg(hwnd,
				ID_SKETCH_SIL_POWER_SLIDER,
				SLM_QUERYSLIDERINFO,
                                MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					     SMA_INCREMENTVALUE),
                                0));
		    IGSketchParam.SilPower = i / 100.0;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		case ID_SKETCH_SHD_POWER_SLIDER:
                    i = SHORT1FROMMR(WinSendDlgItemMsg(hwnd,
				ID_SKETCH_SHD_POWER_SLIDER,
				SLM_QUERYSLIDERINFO,
                                MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					     SMA_INCREMENTVALUE),
                                0));
		    IGSketchParam.ShadePower = i / 100.0;
		    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
		    break;
		case ID_SHADE_AMBIENT_SLIDER:
		    break;
		case ID_SHADE_DIFFUSE_SLIDER:
		    break;
		case ID_SHADE_SPECULAR_SLIDER:
		    break;
		case ID_SHADE_SHININESS_SLIDER:
		    break;
		case ID_SHADE_RESET:
		    break;
		case ID_SHADE_DISMISS:
                    WinDismissDlg(hwnd, FALSE);
		    break;
		default:
		    break;
	    }
            break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one number from the user, and make sure display is consistent.      *
*                                                                            *
* PARAMETERS:                                                                *
*   t:        Numeric data to modify.                                        *
*   hwnd:     A handle on the window.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandleGetOneNumber(IrtRType *t, HWND hwnd)
{
    int i, j;
    char Buffer[10];
    IrtRType
	RunTimeFrac = (IGAnimation.RunTime - IGAnimation.StartT) /
		       (IGAnimation.FinalT - IGAnimation.StartT);

    GlblAnimGetTime = *t;
    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, AnimGetTimeWndProc,
	      0, ID_ANIM_GET_TIME_FORM, NULL);
    *t = GlblAnimGetTime;

    IGAnimation.RunTime = (IGAnimation.FinalT * RunTimeFrac +
			   IGAnimation.StartT * (1.0 - RunTimeFrac));

    for (i = j = 0;
	 i <= GlblSliderIncrements;
	 i += GlblSliderIncrements / 3, j++) {
	IrtRType
	    t = (IGAnimation.StartT * (3 - j) +
		 IGAnimation.FinalT * j) / 3.0;

	sprintf(Buffer, "%4.2f", t);
	WinSendMsg(hwndSlider, SLM_SETSCALETEXT,
		   MPFROMSHORT(i), MPFROMP(Buffer));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Animation (Frame) window call back drawing function.			     *
*									     *
* PARAMETERS:                                                                *
*   hwnd:     A handle on the window.                                        *
*   msg:      Type of event to handle.                                       *
*   mp1, mp2: Parameters of event.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static MRESULT AnimGetTimeWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    POINTL Pt;
    RECTL rcl;
    HPS hps;
    char Line[LINE_LEN];

    switch (msg) {
	case WM_CREATE:
	    break;
        case WM_PAINT:
	    hps = WinBeginPaint(hwnd, 0, NULL);

	    WinQueryWindowRect(hwnd, &rcl);
	    GpiSetColor(hps, CLR_PALEGRAY);
	    Pt.x = Pt.y = 0;
	    GpiMove(hps, &Pt);
	    Pt.x = rcl.xRight;
	    Pt.y = rcl.yTop;
	    GpiBox(hps, DRO_OUTLINEFILL, &Pt, 0, 0);

	    WinEndPaint(hps);
            break;
        case WM_COMMAND:
        case WM_CONTROL:
	    switch (COMMANDMSG(&msg) -> cmd) {
		case ID_ANIM_GET_TIME_ENT:
		    WinQueryDlgItemText(hwnd, ID_ANIM_GOT_TIME,
					LINE_LEN, Line);
		    if (sscanf(Line, "%lf", &GlblAnimGetTime) == 1)
		        WinDismissDlg(hwnd, FALSE);
		    else
		        DosBeep(300, 200);
		    break;
		case ID_ANIM_GET_TIME_CAN:
                    WinDismissDlg(hwnd, FALSE);
		    break;
		case ID_ANIM_GOT_TIME:
		    break;
		default:
		    break;
	    }
            break;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
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
*   X, Y:		 Location of mouse event.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor, int X, int Y)
{
    int i;
    IGGraphicEventType
	RetVal = IG_EVENT_NONE;
    IrtRType XPos, YPos;

    ChangeFactor[0] = ChangeFactor[1] = 0.0;

    XPos = ((IrtRType) X) / TransWidth;
    YPos = ((IrtRType) Y) / TransHeight;

    /* Make sure we are in bound in the X direction. */
    if (XPos < (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0 ||
        XPos > 1.0 - (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0)
	return IG_EVENT_NONE;

    /* Now search the sub window the event occured in. */
    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
        if (InteractMenu.SubWindows[i].Y <= YPos &&
	    InteractMenu.SubWindows[i].Y + INTERACT_SUB_WINDOW_HEIGHT >=
								      YPos) {
	    RetVal = InteractMenu.SubWindows[i].Event;
	    break;
	}
    }

    /* Take care of special cases in which the window should be updated. */
    switch (RetVal) {
	case IG_EVENT_SCR_OBJ_TGL:
	    IGGlblTransformMode = IGGlblTransformMode == IG_TRANS_OBJECT ?
							 IG_TRANS_SCREEN :
							 IG_TRANS_OBJECT;
	    WinInvalidateRect(hwndTransFrame, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
	case IG_EVENT_PERS_ORTHO_TGL:
	    IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					       IG_VIEW_ORTHOGRAPHIC :
					       IG_VIEW_PERSPECTIVE;
	    WinInvalidateRect(hwndTransFrame, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
	case IG_EVENT_DEPTH_CUE:
	    IGGlblDepthCue = !IGGlblDepthCue;
	    WinInvalidateRect(hwndTransFrame, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
	default:
	    break;
    }

    *ChangeFactor = (((IrtRType) X) - TransWidth2) / TransWidth2;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraws the view window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWindow                                                       M
*****************************************************************************/
void IGRedrawViewWindow(void)
{
    WinInvalidateRect(hwndViewFrame, NULL, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws the view window.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   hwnd:       A handle on the window.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawViewWindow(HWND hwnd)
{
    RECTL rcl;

    IGPredefinedAnimation();

    CurrentHps = WinBeginPaint(hwnd, 0, NULL);
    WinQueryWindowRect(hwnd, &rcl);
    ViewWidth2 = rcl.xRight / 2;
    ViewHeight2 = rcl.yTop / 2;
    WinFillRect(CurrentHps, &rcl, CLR_BLUE);

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }
    MatInverseMatrix(IGGlblCrntViewMat, IGGlblInvCrntViewMat);

    IGTraverseObjListHierarchy(IGGlblDisplayList, IGGlblCrntViewMat,
			       IGViewObject);

    WinEndPaint(CurrentHps);
    CurrentHps = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back function of the IGTraverseObjListHierarchy above.              M
*   Also capable of displaying an object, given the global viewing matrix.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to display.                                            M
*   Mat:       Viewing matrix of object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGViewObject	                                                     M
*****************************************************************************/
void IGViewObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IrtHmgnMatType TMat;

    if (IGGlblAbortKeyPressed)
        return;

    HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    HMGN_MAT_COPY(IGGlblCrntViewMat, Mat);

    IGDrawObject(PObj);

    HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws the transformation window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   hwnd:       A handle on the window.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawTransformationWindow(HWND hwnd)
{
    int i, cxChar, cyChar,
	SubTransWidth, SubTransHeight, SubTransPosX, SubTransPosY;
    POINTL Pt;
    RECTL rcl;
    FONTMETRICS fm;
    HPS hps = WinBeginPaint(hwnd, 0, NULL);

    GpiQueryFontMetrics(WinGetPS(hwnd), (LONG) sizeof(fm), &fm);
    cxChar = fm.lAveCharWidth;
    cyChar = fm.lMaxBaselineExt;

    /* Make sure the menu is consistent with internatal data. */
    InteractMenu.SubWindows[0].Str =
	IGGlblTransformMode == IG_TRANS_OBJECT ? "Object Coords."
					       : "Screen Coords.";
    InteractMenu.SubWindows[1].Str =
	IGGlblViewMode == IG_VIEW_PERSPECTIVE ? "Perspective"
					      : "Orthographic";
    InteractMenu.SubWindows[10].Str =
	IGGlblDepthCue ? "Depth cue" : "No depth cue";
		
    WinQueryWindowRect(hwnd, &rcl);
    TransWidth = rcl.xRight;
    TransWidth2 = rcl.xRight / 2;
    TransHeight = rcl.yTop;

    SubTransWidth = (int) (TransWidth * INTERACT_SUB_WINDOW_WIDTH);
    SubTransHeight = (int) (TransHeight *
			    INTERACT_SUB_WINDOW_HEIGHT);
    SubTransPosX = (TransWidth - SubTransWidth) / 2;

    GpiSetColor(hps, CLR_BLACK);
    Pt.x = Pt.y = 0;
    GpiMove(hps, &Pt);
    Pt.x = TransWidth;
    Pt.y = TransHeight;
    GpiBox(hps, DRO_OUTLINEFILL, &Pt, 0, 0);

    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
	GpiSetColor(hps, InteractMenu.SubWindows[i].Color);
	SubTransPosY = (int) (TransHeight *
			      InteractMenu.SubWindows[i].Y);
	
	GPI_MOVE(SubTransPosX, SubTransPosY);
	GPI_LINE(SubTransPosX + SubTransWidth, SubTransPosY);
	GPI_LINE(SubTransPosX + SubTransWidth,
		 SubTransPosY + SubTransHeight);
	GPI_LINE(SubTransPosX, SubTransPosY + SubTransHeight);
	GPI_LINE(SubTransPosX, SubTransPosY);
	if (InteractMenu.SubWindows[i].TextInside) {
	    GPI_CHAR_STR_AT(InteractMenu.SubWindows[i].Str,
			    TransWidth / 2,
			    SubTransPosY + SubTransHeight / 2);
	}
	else {
	    GPI_CHAR_STR_AT(InteractMenu.SubWindows[i].Str,
			    (TransWidth - SubTransWidth) / 3,
			    SubTransPosY + SubTransHeight / 2);
	    GPI_MOVE(SubTransPosX + SubTransWidth / 2, SubTransPosY);
	    GPI_LINE(SubTransPosX + SubTransWidth / 2,
		     SubTransPosY + SubTransHeight);
	}
    }

    for (i = 0; i < INTERACT_NUM_OF_STRINGS; i++) {
	GpiSetColor(hps, InteractMenu.Strings[i].Color);
	GPI_CHAR_STR_AT(InteractMenu.Strings[i].Str,
			(int) (InteractMenu.Strings[i].X * TransWidth),
			(int) (InteractMenu.Strings[i].Y * TransHeight));
    }

    WinEndPaint(hps);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to plot at.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPlotTo2D                                                               M
*****************************************************************************/
void IGPlotTo2D(IrtRType X, IrtRType Y)
{
    IGMoveTo2D(X, Y);
    IGLineTo2D(X, Y);
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
    POINTL Pt;

    Pt.x = OS2_MAP_X_COORD(X);
    Pt.y = OS2_MAP_Y_COORD(Y);
    GpiMove(CurrentHps, &Pt);
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
    POINTL Pt;

    Pt.x = OS2_MAP_X_COORD(X);
    Pt.y = OS2_MAP_Y_COORD(Y);
    GpiLine(CurrentHps, &Pt);
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
    if (!CurrentHps)
	return;
    GpiSetColor(CurrentHps,
		High ? CrntColorHighIntensity : CrntColorLowIntensity);

    IGGlblIntensityHighState = High;
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
	IGSetColorRGB(Color);
    }
    else if ((c = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR) {
	SetColorIndex(c);
    }
    else {
	/* Use white as default color: */
	SetColorIndex(IG_IRIT_WHITE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the light source index Index to a new position/location.         M
*                                                                            *
* PARAMETERS:                                                                M
*   LightPos:       New location of light source. (0, 0, 0) disables light   M
*		    source.						     M
*   LightColor:     Color of light source.				     M
*   LightIndex:     Index of light source in Open GL.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetLightSource                                                         M
*****************************************************************************/
void IGSetLightSource(IGLightType LightPos,
		      IrtVecType LightColor,
		      int LightIndex)
{
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
    GpiSetLineWidthGeom(CurrentHps, Width);
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
    if (color > IG_MAX_COLOR)
	color = IG_IRIT_WHITE;

    CrntColorHighIntensity = ColorsHighIntensity[color];
    CrntColorLowIntensity = ColorsLowIntensity[color];

    if (CurrentHps) {
	GpiSetColor(CurrentHps, CrntColorHighIntensity);
}

    IGGlblIntensityHighState = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the color according to the given RGB values.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Color:      An RGB vector of integer values between 0 and 255.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorRGB                                                            M
*****************************************************************************/
void IGSetColorRGB(int Color[3])
{
    if (!CurrentHps)
	return;

    CrntColorHighIntensity =
	GpiQueryColorIndex(CurrentHps, 0L,
			   RGB_COLOR(Color[0], Color[1], Color[2]));
    CrntColorLowIntensity =
	GpiQueryColorIndex(CurrentHps, 0L,
			   RGB_COLOR(Color[0] / 2, Color[1] / 2, Color[2] / 2));

    GpiSetColor(CurrentHps, CrntColorHighIntensity);

    IGGlblIntensityHighState = TRUE;
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
	case IG_STATE_PERS_ORTHO_TGL:
	case IG_STATE_DEPTH_CUE:
	    WinInvalidateRect(hwndTransFrame, NULL, TRUE);
	default:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    break;
    }

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
    DosBeep(1000, 100);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle pick events.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY: Screen coordinates of pick event.                      M
*   PickTypes:	      Types of object to pick or IG_PICK_ANY for any object. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Picked object or NULL if none.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleCursorEvent                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickEvent                                                        M
*****************************************************************************/
IPObjectStruct *IGHandlePickEvent(int ScreenX, int ScreenY, int PickTypes)
{
    return IGHandleGenericPickEvent(
			    (ScreenX - ((IrtRType) ViewWidth2)) / ViewWidth2,
			    (ScreenY - ((IrtRType) ViewHeight2)) / ViewWidth2,
			    PickTypes);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports to the server on a pick event of the cursor/mouse.               M
* The reported object is a list object of a point and a vector defining the  M
* cursor line in 3-space.  The event type is returned as an "EventType"      M
* attribute on the reported object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY:   Location of the cursor, in screen space coords.      M
*   PickReport:         Type of event: motion, button down, etc.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandlePickEvent                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleCursorEvent                                                      M
*****************************************************************************/
void IGHandleCursorEvent(int ScreenX, int ScreenY, IGPickReportType PickReport)
{
    IrtRType
	MaxDim = MAX(ViewWidth, ViewHeight);

    IGHandleGenericCursorEvent(
			    (ScreenX - ((IrtRType) ViewWidth2)) / ViewWidth2,
			    (ScreenY - ((IrtRType) ViewHeight2)) / ViewWidth2,
			    PickReport);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts screen coordinates (from a mouse, for example) to object space. M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY:   Screen space coordinates.                            M
*   Pt:                 Object space coordinates - origin of ray.            M
*   Dir:                Object space coordinates - direction of ray.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGScreenToObject                                                         M
*****************************************************************************/
void IGScreenToObject(int ScreenX, int ScreenY, IrtPtType Pt, IrtVecType Dir)
{
    int i;
    IrtRType t;

    IGGenericScreenToObject((ScreenX - ((IrtRType) ViewWidth2)) / ViewWidth2,
			    (ScreenY - ((IrtRType) ViewHeight2)) / ViewWidth2,
			    Pt, Dir);

    /* Find the intersection of the ray with the XY plane (Z == 0). */
    if (FABS(Dir[2]) < IRIT_UEPS)
	t = -Pt[2] / IRIT_UEPS;
    else
	t = -Pt[2] / Dir[2];

    for (i = 0; i < 3; i++)
	Pt[i] += Dir[i] * t;
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
*   Should we stop this animation?					     M
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
    if (Anim ->StopAnim)
	fprintf(stderr, "\nAnimation was interrupted by the user.\n");

    return Anim -> StopAnim;
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
* Make error message.			                                     M
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
    WinMessageBox(HWND_DESKTOP, hwndFrame, Msg, NULL, 0, MB_OK);
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
    IGIritError("Yes No message box is not fully implemented under OS2");

    return WinMessageBox(HWND_DESKTOP, hwndFrame, Msg, NULL, 0, MB_OK);
}
