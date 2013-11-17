/*****************************************************************************
*   A Windows NT driver - framework.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "editsrfs.h"
#include "editmanp.h"
#include "wntdrvs.h"
#include "magellan.h"

/* #define IG_MAKE_CON_APP    /* define to compile as a console application. */

/* Interactive menu setup structure: */
#define INTERACT_NUM_OF_STRINGS		4
#define INTERACT_NUM_OF_SUB_WNDWS	18
#define INTERACT_SUB_WINDOW_WIDTH       0.8	 /* Relative to window size. */
#define INTERACT_SUB_WINDOW_HEIGHT      0.04
#define MAX_CONSOLE_LINES 	        500
#define SCALE_FACTOR_MODIFIER 	        0.01

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
IRIT_STATIC_DATA InteractWindowStruct InteractMenu =
{
  {
    { 0.5, 0.825, IG_IRIT_RED,     "Rotate" },
    { 0.5, 0.655, IG_IRIT_GREEN,   "Translate" },
    { 0.5, 0.485, IG_IRIT_CYAN,    "Scale" },
    { 0.5, 0.405, IG_IRIT_MAGENTA, "Clip Planes" },
  },
  {
    { 0.5, 0.94, IG_IRIT_YELLOW, IG_EVENT_SCR_OBJ_TGL,    TRUE,  "Screen Coords." },
    { 0.5, 0.89, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_TGL, TRUE,  "Perspective" },
    { 0.5, 0.85, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_Z,   FALSE, "Z" },
    { 0.5, 0.77, IG_IRIT_RED,    IG_EVENT_ROTATE_X,       FALSE, "X" },  /* Rot */
    { 0.5, 0.725,IG_IRIT_RED,    IG_EVENT_ROTATE_Y,       FALSE, "Y" },
    { 0.5, 0.68, IG_IRIT_RED,    IG_EVENT_ROTATE_Z,       FALSE, "Z" },
    { 0.5, 0.60, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_X,    FALSE, "X" },/* Trans */
    { 0.5, 0.555,IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Y,    FALSE, "Y" },
    { 0.5, 0.51, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Z,    FALSE, "Z" },
    { 0.5, 0.43, IG_IRIT_CYAN,   IG_EVENT_SCALE,	  FALSE, "" }, /* Scale */
    { 0.5, 0.35, IG_IRIT_MAGENTA,IG_EVENT_NEAR_CLIP,      FALSE, "" },
    { 0.5, 0.305,IG_IRIT_MAGENTA,IG_EVENT_FAR_CLIP,       FALSE, "" },
    { 0.5, 0.25, IG_IRIT_MAGENTA,IG_EVENT_DEPTH_CUE,      TRUE,  "Depth Cue" },
    { 0.45,0.19, IG_IRIT_YELLOW, IG_EVENT_SAVE_MATRIX,    TRUE,  "Save Matrix" },
    { 0.45,0.15, IG_IRIT_YELLOW, IG_EVENT_SUBMIT_MATRIX,  TRUE,  "Submit Matrix" },
    { 0.45,0.11, IG_IRIT_YELLOW, IG_EVENT_PUSH_MATRIX,    TRUE,  "Push Matrix" },
    { 0.45,0.07, IG_IRIT_YELLOW, IG_EVENT_POP_MATRIX,     TRUE,  "Pop Matrix" },
    { 0.5, 0.02, IG_IRIT_WHITE,  IG_EVENT_QUIT,	          TRUE,  "Quit" },
  }
};

IRIT_STATIC_DATA BOOL
    DoWindowBorder  = TRUE,
    IsTransWindowOn = TRUE;
IRIT_STATIC_DATA int
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    CurrentXPosition = 0,
    CurrentYPosition = 0;
IRIT_STATIC_DATA unsigned int 
    TransWidth = DEFAULT_TRANS_WIDTH,
    TransHeight = DEFAULT_TRANS_HEIGHT,
    TransWidth2 = DEFAULT_TRANS_WIDTH / 2,
    TransHeight2 = DEFAULT_TRANS_HEIGHT / 2;
    CurrentSubViewInd = 0;

IRIT_GLOBAL_DATA void
    *IGGlblWinBGImage = NULL;
IRIT_GLOBAL_DATA int
    IGGlbl4ViewSeperationColor[3] = { 255, 255, 255 },
    IGGlblWinBGImageX = 0,
    IGGlblWinBGImageY = 0;
IRIT_GLOBAL_DATA unsigned int
    IGSubViewWidth, 
    IGSubViewHeight,
    IGViewWidth = DEFAULT_VIEW_WIDTH,
    IGViewHeight = DEFAULT_VIEW_HEIGHT,
    IGViewWidth2 = DEFAULT_VIEW_WIDTH / 2,
    IGViewHeight2 = DEFAULT_VIEW_HEIGHT / 2;
IRIT_GLOBAL_DATA IrtHmgnMatType
    IGSubViewMat[4];
IRIT_GLOBAL_DATA HBRUSH IG4ViewSeperationBrush, IGBackGroundBrush;
IRIT_GLOBAL_DATA COLORREF
    IGBackGroundColor, IG4ViewSeperationColor,
    IGCrntColorLowIntensity, IGCrntColorHighIntensity,
    IGColorsLowIntensity[IG_MAX_COLOR + 1],
    IGColorsHighIntensity[IG_MAX_COLOR + 1];
IRIT_GLOBAL_DATA HPEN
    IGCurrenthPen = 0;
IRIT_GLOBAL_DATA HDC
    IGCurrenthDC = 0;
IRIT_GLOBAL_DATA HWND IGhWndView, IGhWndTrans, IGhTopLevel,
    IGhWndSubView[IG_NUM_OF_SUB_VIEWS] = { 0, 0, 0, 0 };
IRIT_GLOBAL_DATA HMENU
    GlblStateMenu = 0;
IRIT_GLOBAL_DATA HCURSOR
    GlblCurrentCursor = NULL;

static int Display4ViewsAux(int Display4Views);
static IPObjectStruct *HandleSubViewPickEventAux(int ScreenX,
						 int ScreenY,
						 int PickTypes,
						 int ViewNum);
static LONG APIENTRY SubViewWndProc(HWND hWndFrame,
				    UINT wMsg,
				    WPARAM wParam,
				    LONG lParam);
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int X,
					  int Y);
static void RedrawTransformationWindow(HWND hWnd);
static void SetColorIndex2(int Color, int Width);
static void DrawTextLocal(char *Str, int PosX, int PosY);

#ifndef IG_MAKE_CON_APP

static void GetArgCVAux(CHAR *Str, int *argc, CHAR ***argv);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets the command line into a form understandable by K&R definition.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Str:         Command line to decipher.                                   M
*   argc, argv:  Command line, K&R style.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetArgCV                                                                 M
*****************************************************************************/
void GetArgCV(CHAR *Str, int *argc, CHAR ***argv)
{
#ifdef WINNT_SETARGV
    if (IGGlblActiveXMode) {
        GetArgCVAux(Str, argc, argv);
    }
    else {
        static void _setargv(void);

	_setargv();
	*argc = __argc;
	*argv = __argv;
    }
#else
    GetArgCVAux(Str, argc, argv);
#endif /* WINNT_SETARGV */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GetArgCV to parse a regular string into argc/v.    *
*****************************************************************************/
static void GetArgCVAux(CHAR *Str, int *argc, CHAR ***argv)
{
    IRIT_STATIC_DATA CHAR CommandLine[ARGCV_LINE_LEN];
    IRIT_STATIC_DATA CHAR *Argv[ARGCV_MAX_WORDS];
    int Argc;
    CHAR *p;

    strncpy(CommandLine, Str, ARGCV_LINE_LEN - 1);
    CommandLine[ARGCV_LINE_LEN - 1] = 0;

    for (Argc = 1, Argv[0] = "wntdrvs", p = strtok(CommandLine, " \t\n\r");
         p != NULL && Argc < ARGCV_MAX_WORDS - 1;
         p = strtok(NULL, " \t\n\r")) {
	if (p[0] == '"') {
	    char *q;

	    Argv[Argc++] = q = IritStrdup(p + 1);
	    q = &q[strlen(q) - 1];                      /* Kill the end '"'. */
	    if (*q == '"')
	        *q = 0;
	}
	else
	    Argv[Argc++] = IritStrdup(p);
    }

    Argv[Argc] = 0;

    *argc = Argc;
    *argv = Argv;
}

#endif /* IG_MAKE_CON_APP */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of wntdrvs - Windows NT graphics driver of IRIT.        	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         Exit code.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
#ifdef IG_MAKE_CON_APP
int main(int argc, char **argv)
{
    IRIT_STATIC_DATA HANDLE
	hInst = 0,
	hPrevInst = 0;

#else
int APIENTRY WinMain(HINSTANCE hInst,
		     HINSTANCE hPrevInst,
		     LPSTR lpszLine,
		     int nShow)
{
    int argc;
    char **argv;
#endif /* IG_MAKE_CON_APP */
    HWND hWndFrame;
    MSG msg;
    int TransPrefPos[4], ViewPrefPos[4];
    MagellanSpaceMouseDataStruct SpaceMouseData;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

#ifndef IG_MAKE_CON_APP
    GetArgCV(lpszLine, &argc, &argv);
#endif /* IG_MAKE_CON_APP */

    IGConfigureGlobals("wntdrvs", argc, argv);

    IGBackGroundColor = RGB(IGGlblBackGroundColor[0],
		            IGGlblBackGroundColor[1],
			    IGGlblBackGroundColor[2]);
    IGBackGroundBrush = CreateSolidBrush(IGBackGroundColor);
    IG4ViewSeperationColor = RGB(IGGlbl4ViewSeperationColor[0],
				 IGGlbl4ViewSeperationColor[1],
				 IGGlbl4ViewSeperationColor[2]);
    IG4ViewSeperationBrush = CreateSolidBrush(IG4ViewSeperationColor);

    /* Must select a function to draw polys. */
    IGDrawPolyFuncPtr = IGDrawPoly;

    if (!hPrevInst) {
	WNDCLASS wndClass;
	int Registered = TRUE;

	/* Set up common defaults. */
        wndClass.style         = CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra    = 0;
	wndClass.cbWndExtra    = 0;
	wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndClass.hInstance     = hInst;

	/* Register top level main window. */
	wndClass.lpfnWndProc   = WndProc;
	wndClass.hIcon         = LoadIcon(hInst, APP_CLASS);
	wndClass.hbrBackground = GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName  = APP_CLASS;
	wndClass.lpszClassName = APP_CLASS;
	if (!RegisterClass(&wndClass))
	    return FALSE;

	/* Register viewing window. */
	wndClass.lpfnWndProc   = ViewWndProc;
	wndClass.hIcon         = NULL;
	wndClass.hbrBackground = CreateSolidBrush(IGBackGroundColor);
	wndClass.lpszClassName = APP_VIEW_CLASS;
	if (!RegisterClass(&wndClass))
	    return FALSE;

	/* Register sub viewing window. */
	wndClass.lpfnWndProc   = SubViewWndProc;
	wndClass.hIcon         = NULL;
	wndClass.hbrBackground = CreateSolidBrush(IGBackGroundColor);
	wndClass.lpszClassName = APP_SUB_VIEW_CLASS;
	if (!RegisterClass(&wndClass))
	    return FALSE;

	/* Register transformation window. */
	wndClass.lpfnWndProc   = TransWndProc;
	wndClass.lpszClassName = APP_TRANS_CLASS;
	if (!RegisterClass(&wndClass))
	    return FALSE;
    }

    if (sscanf(IGGlblTransPrefPos, "%d,%d,%d,%d",
	       &TransPrefPos[0], &TransPrefPos[1],
	       &TransPrefPos[2], &TransPrefPos[3]) == 4 &&
	sscanf(IGGlblViewPrefPos, "%d,%d,%d,%d",
	       &ViewPrefPos[0], &ViewPrefPos[1],
	       &ViewPrefPos[2], &ViewPrefPos[3]) == 4) {
	hWndFrame = CreateWindow(APP_CLASS,
				 APP_TITLE,
				 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				 ViewPrefPos[0],
				 ViewPrefPos[2],
				 ViewPrefPos[1] - ViewPrefPos[0] +
				     TransPrefPos[1] - TransPrefPos[0],
				 IRIT_MAX(ViewPrefPos[3] - ViewPrefPos[2],
				     TransPrefPos[3] - TransPrefPos[2]),
				 NULL,
				 NULL,
				 hInst,
				 NULL);
    }
    else if (sscanf(IGGlblTransPrefPos, "%d,%d,%d,%d",
		    &TransPrefPos[0], &TransPrefPos[1],
		    &TransPrefPos[2], &TransPrefPos[3]) == 2 &&
	     sscanf(IGGlblViewPrefPos, "%d,%d,%d,%d",
	            &ViewPrefPos[0], &ViewPrefPos[1],
	            &ViewPrefPos[2], &ViewPrefPos[3]) == 2) {
	hWndFrame = CreateWindow(APP_CLASS,
				 APP_TITLE,
				 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 ViewPrefPos[0],
				 ViewPrefPos[2],
				 NULL,
				 NULL,
				 hInst,
				 NULL);
    }
    else
        hWndFrame = CreateWindow(APP_CLASS,
				 APP_TITLE,
				 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,            
				 NULL,
				 NULL,
				 hInst,
				 NULL);

    SetCursor(GlblCurrentCursor = LoadCursor(NULL, IDC_ARROW));

    msg.wParam = 1;
    if (hWndFrame) {
	HDC hDC;

	if (hDC = GetDC(hWndFrame)) {
	    int i;

	    for (i = 0; i <= IG_MAX_COLOR; i++) {
	        IGColorsHighIntensity[i] = GetNearestColor(hDC,
					RGB(AttrIritColorTable[i][0],
					    AttrIritColorTable[i][1],
					    AttrIritColorTable[i][2]));
	        IGColorsLowIntensity[i] = GetNearestColor(hDC,
				    RGB(AttrIritColorTable[i][0] / 2,
					AttrIritColorTable[i][1] / 2,
					AttrIritColorTable[i][2] / 2));
	    }
	    ReleaseDC(hWndFrame, hDC);
	}
	
        IGhTopLevel = hWndFrame;

	DragAcceptFiles(IGhTopLevel, TRUE); /* Allow drag and drop of files. */

#	ifdef IG_MAKE_CON_APP
            ShowWindow(hWndFrame, SW_SHOW);
#	else
	    ShowWindow(hWndFrame, nShow);
#	endif /* IG_MAKE_CON_APP */
        UpdateWindow(hWndFrame);

	/* Starts with the selected set of initial widgets. */
	if (IGGlblInitWidgetDisplay & IG_WIDGET_ENVIRONMENT)
	    EnvironmentCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_ANIMATION)
	    AnimationCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_CURVES)
	    CrvEditCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_SURFACES)
	    SrfEditCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_SHADING)
	    ShadeParamCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_PICK_OBJS)
	    PickObjsCB();
	if (IGGlblInitWidgetDisplay & IG_WIDGET_OBJS_TRANS)
	    ObjManipCB();

	if (IGCrvEditPreloadEditCurveObj != NULL) {
	    IGPopupCrvEditor(IGCrvEditPreloadEditCurveObj);
	    IGCrvEditPreloadEditCurveObj = NULL;
	}
	if (IGSrfEditPreloadEditSurfaceObj != NULL) {
	    IGPopupSrfEditor(IGSrfEditPreloadEditSurfaceObj);
	    IGSrfEditPreloadEditSurfaceObj = NULL;
	}

	/* Connect to Magellan SpaceMouse device */
        MagellanInitSpaceMouseDevice(&SpaceMouseData, hWndFrame);

	SetWindowText(IGhTopLevel, IGGenerateWindowHeaderString(NULL));

        while (TRUE) {
	    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	        /* Handle events from Magellan SpaceMouse, if has any. */
                if (MagellanSpaceMouseHandleEvent(&SpaceMouseData,
						  &msg) == FALSE) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
	    }
	    else if (IGGlblContinuousMotion) {
		IGHandleContinuousMotion();
	    }
	    else if (!IGGlblStandAlone &&
		     IGReadObjectsFromSocket(IGGlblViewMode,
					     &IGGlblDisplayList)) {
	        InvalidateRect(IGhWndView, NULL, FALSE);
	    }
	    else
		IGPredefinedAnimation();

	    IritSleep(1);			      /* Do not use all CPU. */
        }
    }

    UnregisterClass(APP_CLASS, hInst);
    UnregisterClass(APP_VIEW_CLASS, hInst);
    UnregisterClass(APP_SUB_VIEW_CLASS, hInst);
    UnregisterClass(APP_TRANS_CLASS, hInst);

    /* Close connection to the 3DxWare driver. */
    MagellanCloseSpaceMouseDevice(&SpaceMouseData);

    return msg.wParam;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redirects stdin/out/err to a second console window.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedirectIOToConsole                                                    M
*****************************************************************************/
void IGRedirectIOToConsole(void)
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    /* Allocate a console for this app. */
    AllocConsole();

    /* Set the screen buffer to be big enough to let us scroll text. */
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), 
			       &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), 
			       coninfo.dwSize);

    /* Redirect unbuffered STDOUT to the console. */
    lStdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Redirect unbuffered STDIN to the console. */
    lStdHandle = (long) GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "r");
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Redirect unbuffered STDERR to the console. */
    lStdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);
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
    CrvEditCB();
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
    SrfEditCB();
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
    ObjManipCB();
    if (PObj != NULL)
        IGObjManipAttachOldDirectly(PObj, CloneIt);
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
    if (GlblStateMenu) 
	DestroyMenu(GlblStateMenu);

    GlblStateMenu = CreatePopupMenu();

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_OOPS, "<Btn>  <Shift-Btn>");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_OOPS, "select     deselect");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_MOUSE_SENSITIVE,
	       "Mouse Sensitive");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu,
	       SET_MENU_FLAGS(IGGlblTransformMode == IG_TRANS_SCREEN),
	       IG_STATE_SCR_OBJ_TGL, "Screen Coords.");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblContinuousMotion),
	       IG_STATE_CONT_MOTION, "Cont. Motion");
    AppendMenu(GlblStateMenu,
	       SET_MENU_FLAGS(IGGlblViewMode == IG_VIEW_PERSPECTIVE),
	       IG_STATE_PERS_ORTHO_TGL, "Perspective");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDepthCue),
	       IG_STATE_DEPTH_CUE, "Depth Cue");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblCacheGeom),
	       IG_STATE_CACHE_GEOM, "Cache Geom");

    switch (IGGlblDrawStyle) {
	case IG_STATE_DRAW_STYLE_WIREFRAME:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_DRAW_STYLE, "Draw Wireframe");
	    break;
	case IG_STATE_DRAW_STYLE_SOLID:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_DRAW_STYLE, "Draw Solid");
	    break;
	case IG_STATE_DRAW_STYLE_POINTS:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_DRAW_STYLE, "Draw Points");
	    break;
    }

    switch (IGGlblShadingModel) {
	case IG_SHADING_NONE:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_SHADING_MODEL, "No Shading");
	    break;
	case IG_SHADING_FLAT:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_SHADING_MODEL, "Flat Shading");
	    break;
	case IG_SHADING_BACKGROUND:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_SHADING_MODEL, "Background Shading");
	    break;
	case IG_SHADING_GOURAUD:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_SHADING_MODEL, "Gouraud Shading");
	    break;
	case IG_SHADING_PHONG:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_SHADING_MODEL, "Phong Shading");
	    break;
    }
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblBackFaceCull),
	       IG_STATE_BACK_FACE_CULL, "Back Face Cull");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDoDoubleBuffer),
	       IG_STATE_DOUBLE_BUFFER, "Double Buffer");

    switch (IGGlblAntiAliasing) {
	case IG_STATE_ANTI_ALIAS_OFF:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_ANTI_ALIASING, "No Anti Aliasing");
	    break;
	case IG_STATE_ANTI_ALIAS_ON:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_ANTI_ALIASING, "Anti Aliasing");
	    break;
	case IG_STATE_ANTI_ALIAS_BLEND:
	    AppendMenu(GlblStateMenu, MF_STRING,
		       IG_STATE_ANTI_ALIASING, "Blending");
	    break;
    }

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawInternal),
	       IG_STATE_DRAW_INTERNAL, "Draw Internal Edges");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawVNormal),
	       IG_STATE_DRAW_VNORMAL, "Draw Vrtx Normals");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawPNormal),
	       IG_STATE_DRAW_PNORMAL, "Draw Poly Normals");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceSketch),
	       IG_STATE_DRAW_SRF_SKTCH, "Surface Sketch");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceMesh),
	       IG_STATE_DRAW_SRF_MESH, "Draw Surface Mesh");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfacePoly),
	       IG_STATE_DRAW_SRF_POLY, "Surface Polygons");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceWire),
	       IG_STATE_DRAW_SRF_WIRE, "Surface Isocurves");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceBndry),
	       IG_STATE_DRAW_SRF_BNDRY, "Surface Bndry");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceSilh),
	       IG_STATE_DRAW_SRF_SILH, "Surface Silhouettes");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawSurfaceRflctLns),
	       IG_STATE_DRAW_SRF_RFLCT_LNS, "Surface Rflct Lns");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_FOUR_PER_FLAT,
	       IGGlblFourPerFlat ? "Four Per Flat" : "Two Per Flat");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblDrawPolygons),
	       IG_STATE_DRAW_POLYGONS, "Draw Polygons");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_NUM_ISOLINES,
	       "Num Isolines");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_POLY_APPROX,
	       "Poly approximation");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_LENGTH_VECTORS,
	       "Length Vectors");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblFlipNormalOrient),
	       IG_STATE_NRML_ORIENT, "Reversed Normals");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_WIDTH_LINES,
	       "Width Lines");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblCountNumPolys),
	       IG_STATE_NUM_POLY_COUNT, "Num Poly Count");
    AppendMenu(GlblStateMenu, SET_MENU_FLAGS(IGGlblCountFramePerSec),
	       IG_STATE_FRAME_PER_SEC, "Frames Per Second");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_VIEW_FRONT, "Front View");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_VIEW_SIDE, "Side View");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_VIEW_TOP, "Top View");
    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_VIEW_ISOMETRY,
	       "Isometry View");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_STATE_CLEAR_VIEW,
	       "Clear View Area");

    AppendMenu(GlblStateMenu, MF_SEPARATOR, 0, NULL);

    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_STATE,
	       "Environment");
    AppendMenu(GlblStateMenu, MF_STRING,IG_EVENT_ANIMATION,
	       "Animation");
    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_SHADE_PARAM,
	       "Shade Param");
    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_CRV_EDIT,
	       "Crv Edit");
    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_SRF_EDIT,
	       "Srf Edit");
    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_PICK_OBJS,
	       "Pickable Objects");
    AppendMenu(GlblStateMenu, MF_STRING, IG_EVENT_OBJ_MANIP,
	       "Object Manip");

    ShadeUpdateRadioButtons();
    EnvParamUpdateCb();
    PickObjsUpdateCb();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Top level window call back drawing function.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   hWndFrame:       A handle on the window.                                 M
*   wMsg:            Type of event to handle.                                M
*   wParam, lParam:  Parameters of event.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   LONG APIENTRY:  Event state code.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   WndProc                                                                  M
*****************************************************************************/
LONG APIENTRY WndProc(HWND hWndFrame,
		      UINT wMsg,
		      WPARAM wParam,
		      LONG lParam)
{
    IRIT_STATIC_DATA HMENU Menu;
    IRIT_STATIC_DATA HWND ParentWnd;
    IRIT_STATIC_DATA RECT rect;
    IRIT_STATIC_DATA int
        PosX = 0,
        PosY = 0;
    HDC hDC;
    TEXTMETRIC tm;
    int Refresh;
    MENUITEMINFO MenuInfo;

    GetClientRect(hWndFrame, &rect);

    switch (wMsg) {
        case WM_CREATE:
            if (hDC = GetDC(hWndFrame)) {
		SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));
		GetTextMetrics(hDC, &tm);
		ReleaseDC(hWndFrame, hDC);
	    }

	    if (IGGlblActiveXMode) {
		Menu = GetSubMenu(((LPCREATESTRUCT) lParam) -> hMenu, 5);
		ParentWnd = (((ActiveXCreateInfoStruct *)
                   ((LPCREATESTRUCT) lParam) -> lpCreateParams)) -> controlWnd;
		if ((((ActiveXCreateInfoStruct *) ((LPCREATESTRUCT) lParam) ->
		                     lpCreateParams)) -> hasTransWnd == TRUE) {
		    IGhWndView = CreateWindow(APP_VIEW_CLASS,
					      "v",
					      WS_CHILD | WS_BORDER |
					      WS_VISIBLE | WS_CLIPSIBLINGS |
					      WS_CLIPCHILDREN,
					      0,
					      0,
					      IGViewWidth =
					          IRIT_MAX(rect.right * 3 / 4 - 2,
						      0),
					      IGViewHeight = rect.bottom,
					      hWndFrame,
					      0,
					      ((LPCREATESTRUCT) lParam)->hInstance,
					      NULL);
		    IGhWndTrans = CreateWindow(APP_TRANS_CLASS,
					       "t",
					       WS_CHILD | WS_BORDER |
					       WS_VISIBLE,
					       (rect.right * 3) / 4,
					       0,
					       TransWidth = rect.right / 4,
					       TransHeight = rect.bottom,
					       hWndFrame,
					       0,
					       ((LPCREATESTRUCT) lParam)->hInstance,
					       NULL);
		    IsTransWindowOn = TRUE;
		    DoWindowBorder = TRUE;
		    CheckMenuItem(Menu, 0, MF_BYPOSITION | MF_CHECKED);
		}
		else {
		    IGhWndView = CreateWindow(APP_VIEW_CLASS,
					      "v",
					      WS_CHILD |
					      WS_VISIBLE | WS_CLIPSIBLINGS |
					      WS_CLIPCHILDREN,
					      0,
					      0,
					      IGViewWidth = rect.right,
					      IGViewHeight = rect.bottom,
					      hWndFrame,
					      0,
					      ((LPCREATESTRUCT) lParam)->hInstance,
					      NULL);
		    IGhWndTrans = CreateWindow(APP_TRANS_CLASS,
					       "t",
					       WS_CHILD,
					       (rect.right * 3) / 4,
					       0,
					       TransWidth = rect.right / 4,
					       TransHeight = rect.bottom,
					       hWndFrame,
					       0,
					       ((LPCREATESTRUCT) lParam)->hInstance,
					       NULL);
		    IsTransWindowOn = FALSE;
		    DoWindowBorder = FALSE;
		    CheckMenuItem(Menu, 0, MF_BYPOSITION | MF_UNCHECKED);
		}
		IGViewWidth2 = IGViewWidth / 2;
		IGViewHeight2 = IGViewHeight / 2;
	    }
	    else {
	        IGhWndView = CreateWindow(APP_VIEW_CLASS,
					  "v",
					  WS_CHILD | WS_BORDER | WS_VISIBLE |
					  WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
					  0,
					  0,
					  IGViewWidth =
					      IRIT_MAX(rect.right * 3 / 4 - 2, 0),
					  IGViewHeight = rect.bottom,
					  hWndFrame,
					  0,
					  ((LPCREATESTRUCT) lParam)->hInstance,
					  NULL);
		IGViewWidth2 = IGViewWidth / 2;
		IGViewHeight2 = IGViewHeight / 2;
		
		IGhWndTrans = CreateWindow(APP_TRANS_CLASS,
					   "t",
					   WS_CHILD | WS_BORDER | WS_VISIBLE,
					   (rect.right * 3) / 4,
					   0,
					   TransWidth = rect.right / 4,
					   TransHeight = rect.bottom,
					   hWndFrame,
					   0,
					   ((LPCREATESTRUCT) lParam)->hInstance,
					   NULL);
	    }

	    TransWidth2 = TransWidth / 2;
	    TransHeight2 = TransHeight / 2;
            CreateEnvironment(hWndFrame);
            CreateAnimation(hWndFrame);
            CreateShadeParam(hWndFrame);
            CreateCrvEditParam(hWndFrame);
            CreateSrfEditParam(hWndFrame);
            CreatePickObjs(hWndFrame);
	    CreateObjectManip(hWndFrame);
 	    return 0;

	case WM_SIZE:
	    rect.left   = 0;
	    rect.top    = 0;
	    rect.right  = LOWORD(lParam);
	    rect.bottom = HIWORD(lParam);

	    if (IGGlblActiveXMode) {
		if (IsTransWindowOn == 1) {
		    SetWindowPos(IGhWndView,
				 HWND_TOP,
				 0,
				 0,
				 IGViewWidth = IRIT_MAX(rect.right * 3 / 4 - 2, 0),
				 IGViewHeight = rect.bottom,
				 0);
		    SetWindowPos(IGhWndTrans,
				 HWND_TOP,
				 (rect.right * 3) / 4,
				 0,
				 TransWidth = IRIT_MAX(rect.right / 4 - 1, 0),
				 TransHeight = rect.bottom - 1,
				 0);
		    TransWidth2 = TransWidth / 2;
		    TransHeight2 = TransHeight / 2;
		}
		else /* isTransWindowOn != 1 */
		    SetWindowPos(IGhWndView,
				 HWND_TOP,
				 0,
				 0,
				 IGViewWidth = rect.right,
				 IGViewHeight = rect.bottom,
				 0);
	    }
	    else {
	        SetWindowPos(IGhWndView,
			     HWND_TOP,
			     0,
			     0,
			     IGViewWidth = IRIT_MAX(rect.right * 3 / 4 - 2, 0),
			     IGViewHeight = rect.bottom,
			     0);

		SetWindowPos(IGhWndTrans,
			     HWND_TOP,
			     (rect.right * 3) / 4,
			     0,
			     TransWidth = IRIT_MAX(rect.right / 4 - 1, 0),
			     TransHeight = rect.bottom - 1,
			     0);
		TransWidth2 = TransWidth / 2;
		TransHeight2 = TransHeight / 2;
	    }

	    IGViewWidth2 = IGViewWidth / 2;
	    IGViewHeight2 = IGViewHeight / 2;
	    return 0;

	case WM_QUERYNEWPALETTE:
	case WM_PALETTECHANGED:
	    if (wParam != (WPARAM) hWndFrame)
		SendMessage(IGhWndView, wMsg, 0, 0L);
	    return 0;

	case WM_DESTROY:
	    if (IGGlblActiveXMode) {
	        DestroyWindow(IGhWndTrans);
	        DestroyWindow(IGhWndView);
	    }
	    else
	        exit(0);
	    return 0;

        case WM_COMMAND:
	    Refresh = FALSE;
            switch (LOWORD(wParam)) {
		case IDM_FILE_SAVE_MAT:
		    IGSaveCurrentMat(IGGlblViewMode, NULL);
		    break;
		case IDM_FILE_SAVE_MAT_AS:
		    {
			OPENFILENAME ofn;
			char FileName[IRIT_LINE_LEN_LONG];

			strcpy(FileName, "*.imd");
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWndFrame;
			ofn.lpstrFile = FileName;
			ofn.lpstrFileTitle = FileName;
			ofn.nMaxFile = IRIT_LINE_LEN_LONG;
			ofn.lpstrTitle = "Save View Matrix As";
			ofn.lpstrFilter = "*.imd";
			ofn.Flags = OFN_OVERWRITEPROMPT;

		        if (GetSaveFileName(&ofn) && !IRT_STR_ZERO_LEN(FileName))
			    IGSaveCurrentMat(IGGlblViewMode, FileName);
		    }
		    break;
		case IDM_FILE_SAVE_IMAGE_AS:
		    {
			OPENFILENAME ofn;
			char FileName[IRIT_LINE_LEN_LONG];

			strcpy(FileName, "*.ppm");
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWndFrame;
			ofn.lpstrFile = FileName;
			ofn.lpstrFileTitle = FileName;
			ofn.nMaxFile = IRIT_LINE_LEN_LONG;
			ofn.lpstrTitle = "Save Image";
			ofn.lpstrFilter = "*.ppm";
			ofn.Flags = OFN_OVERWRITEPROMPT;

		        if (GetSaveFileName(&ofn) && !IRT_STR_ZERO_LEN(FileName))
			    IGSaveDisplayAsImage(FileName);
		    }
		    break;
		case IDM_FILE_SUBMIT:
		    IGSubmitCurrentMat(IGGlblViewMode);
		    break;
		case IDM_FILE_CLEAR_VIEW:
		    Refresh = IGHandleState(IG_STATE_CLEAR_VIEW,
					    IG_STATE_ON, TRUE);
		    break;
		case IDM_FILE_BG_COLOR:
		    {
		 	CHOOSECOLOR cc;
			static COLORREF CustColors[16];

			memset(&cc, 0, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWndFrame;
			cc.rgbResult = IGBackGroundColor;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		        if ((Refresh = ChooseColor(&cc)) != FALSE) {
			    IGGlblBackGroundColor[0] =
			        GetRValue(cc.rgbResult);
			    IGGlblBackGroundColor[1] =
			        GetGValue(cc.rgbResult);
			    IGGlblBackGroundColor[2] =
			        GetBValue(cc.rgbResult);
			    IGBackGroundColor = cc.rgbResult;
			    IGBackGroundBrush =
				CreateSolidBrush(cc.rgbResult);
			    InvalidateRect(IGhWndTrans, NULL, TRUE);
			}
		    }
		    break;
		case IDM_FILE_DISCONNECT:
		    if (IGGlblIOHandle >= 0)
		        IPCloseStream(IGGlblIOHandle, TRUE);
		    IGGlblStandAlone = TRUE;
		    break;
		case IDM_FILE_QUIT:
		    if (!IGGlblActiveXMode) {
		        if (IGGlblIOHandle >= 0)
			    IPCloseStream(IGGlblIOHandle, TRUE);
			exit(0);
		    }
		    break;
		case IDM_MOUSE_LESS:
		    IGHandleState(IG_STATE_MOUSE_SENSITIVE,
				  IG_STATE_DEC, TRUE);
		    break;
		case IDM_MOUSE_MORE:
		    IGHandleState(IG_STATE_MOUSE_SENSITIVE,
				  IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_LESS_ISO:
		    Refresh = IGHandleState(IG_STATE_NUM_ISOLINES,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_MORE_ISO:
		    Refresh = IGHandleState(IG_STATE_NUM_ISOLINES,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_STATE_COARSER_APPROX:
		    Refresh = IGHandleState(IG_STATE_POLY_APPROX,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_FINER_APPROX:
		    Refresh = IGHandleState(IG_STATE_POLY_APPROX,
					    IG_STATE_INC, TRUE);
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
		case IDM_STATE_THIN_POINTS:
		    Refresh = IGHandleState(IG_STATE_WIDTH_POINTS,
					    IG_STATE_DEC, TRUE);
		    break;
		case IDM_STATE_WIDE_POINTS:
		    Refresh = IGHandleState(IG_STATE_WIDTH_POINTS,
					    IG_STATE_INC, TRUE);
		    break;
		case IDM_TGLS_SCREEN:
		    IGHandleState(IG_STATE_SCR_OBJ_TGL, IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_CONT_MOTION:
		    IGHandleState(IG_STATE_CONT_MOTION, IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_NRML_ORIENT:
		    Refresh = IGHandleState(IG_STATE_NRML_ORIENT,
					    IG_STATE_TGL, TRUE);
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
		case IDM_TGLS_ANTI_ALIASING:
		    Refresh = IGHandleState(IG_STATE_ANTI_ALIASING,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_DRAW_STYLE:
		    Refresh = IGHandleState(IG_STATE_DRAW_STYLE,
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
		case IDM_TGLS_SRF_SKTCH:
		    Refresh = IGHandleState(IG_STATE_DRAW_SRF_SKTCH,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_4_PER_FLAT:
		    Refresh = IGHandleState(IG_STATE_FOUR_PER_FLAT,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_POLYGONS:
		    Refresh = IGHandleState(IG_STATE_DRAW_POLYGONS,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_TGLS_NUM_POLY_COUNT:
		    Refresh = IGHandleState(IG_STATE_NUM_POLY_COUNT,
					    IG_STATE_TGL, TRUE);
		case IDM_TGLS_FRAME_PER_SEC:
		    Refresh = IGHandleState(IG_STATE_FRAME_PER_SEC,
					    IG_STATE_TGL, TRUE);
		    break;
		case IDM_VIEW_FRONT:
		    Refresh = IGHandleState(IG_STATE_VIEW_FRONT,
					    IG_STATE_ON, TRUE);
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
		case IDM_VIEW_4VIEWS:
		    Refresh = IGHandleState(IG_STATE_VIEW_4,
					    IG_STATE_ON, TRUE);
		    break;
		case IDM_EXTN_ENVIRONMENT:
                    EnvironmentCB();
		    break;
		case IDM_EXTN_ANIMATION:
                    AnimationCB();
		    break;
		case IDM_EXTN_SHADE_PARAM:
                    ShadeParamCB();
		    break;
		case IDM_EXTN_CRV_EDIT:
                    CrvEditCB();
		    break;
		case IDM_EXTN_SRF_EDIT:
                    SrfEditCB();
		    break;
		case IDM_EXTN_PICK_OBJS:
                    PickObjsCB();
		    break;
		case IDM_EXTN_OBJ_MANIP:
                    ObjManipCB();
		    break;
		case IDM_EXTN_TRANSFORMATIONS:
		    if (IGGlblActiveXMode) {
		        MenuInfo.cbSize = sizeof(MenuInfo);
			MenuInfo.fMask = MIIM_STATE;
			GetMenuItemInfo(Menu, 0, 1, &MenuInfo);
			if (MenuInfo.fState == MFS_CHECKED) {
			    SetWindowPos(IGhWndTrans,
					 HWND_BOTTOM,
					 0, 0, 0, 0, SWP_HIDEWINDOW);
			    SetWindowPos(IGhWndView,
					 HWND_TOP,
					 0,
					 0,
					 IGViewWidth = rect.right,
					 IGViewHeight = rect.bottom,
					 0);
			    IGViewWidth2 = IGViewWidth / 2;
			    IGViewHeight2 = IGViewHeight / 2;
			    IsTransWindowOn = FALSE;
			    CheckMenuItem(Menu, 0, MF_UNCHECKED | MF_BYPOSITION);
			}
			else {
			    GetClientRect(hWndFrame, &rect);
			    SetWindowPos(IGhWndTrans,
					 HWND_TOP,
					 (rect.right * 3) / 4,
					 0,
					 TransWidth = rect.right / 4,
					 TransHeight = rect.bottom,
					 SWP_SHOWWINDOW);
			    SetWindowPos(IGhWndView,
					 HWND_TOP,
					 0,
					 0,
					 IGViewWidth =
					     IRIT_MAX(rect.right * 3 / 4 - 2, 0),
					 IGViewHeight = rect.bottom,
					 0);
			    IGViewWidth2 = IGViewWidth / 2;
			    IGViewHeight2 = IGViewHeight / 2;
			    IsTransWindowOn = TRUE;
			    CheckMenuItem(Menu, 0, MF_BYPOSITION | MF_CHECKED);
			}
			UpdateWindow(IGhWndView);
			UpdateWindow(IGhWndTrans);
			break;
		    }
		    break;
	    }
	    if (Refresh)
		InvalidateRect(IGhWndView, NULL, FALSE);
            break;

	case WM_KEYDOWN:
	    if (IGCrvEditActive) {
		switch (wParam) {
		    case VK_LEFT:
		    case VK_DOWN:
			IGCrvEditMRLevel -= 0.01;             /* Percents... */
		        break;
		    case VK_UP:
		    case VK_RIGHT:
			IGCrvEditMRLevel += 0.01;             /* Percents... */
			break;
		}

		IGCrvEditMRLevel = IRIT_BOUND(IGCrvEditMRLevel, 0.0, 1.0);
		IGCrvEditParamUpdateMRScale();
		IGRedrawViewWindow();     /* Update the MR region displayed. */
	    }
	    else if (IGSrfEditActive) {
		switch (wParam) {
		    case VK_LEFT:
			IGSrfEditMRULevel -= 0.01;            /* Percents... */
		        break;
		    case VK_RIGHT:
			IGSrfEditMRULevel += 0.01;            /* Percents... */
			break;
		    case VK_DOWN:
			IGSrfEditMRVLevel -= 0.01;            /* Percents... */
			break;
		    case VK_UP:
			IGSrfEditMRVLevel += 0.01;            /* Percents... */
			break;
		}

		IGSrfEditMRULevel = IRIT_BOUND(IGSrfEditMRULevel, 0.0, 1.0);
		IGSrfEditMRVLevel = IRIT_BOUND(IGSrfEditMRVLevel, 0.0, 1.0);
		IGSrfEditParamUpdateMRScale();
		IGRedrawViewWindow(); /* Update the MR region displayed. */
	    }
	    else if (IGGlblActiveXMode) {
	        switch (wParam) {
		    case ' ':
			/* Disable all animations. */
		        IGGlblContinuousMotion = FALSE;
			IGAnimation.NumOfRepeat = 1;
			IGAnimation.TwoWaysAnimation = FALSE;
			IGAnimation.StartT =
			    IGAnimation.FinalT = IGAnimation.RunTime;
			break;
		}
	    }
	    return 0;

	case WM_DROPFILES:
	    {
	        char FileName[IRIT_LINE_LEN_LONG], *PFiles[2];
		int i, n;
		HDROP hDrop;
		IPObjectStruct *PObj;

	        hDrop = (HDROP) wParam;
		n = DragQueryFile(hDrop, 0xFFFFFFFF,
				  FileName, IRIT_LINE_LEN_LONG - 1);
		for (i = 0; i < n; i++) {
		    DragQueryFile(hDrop, i, FileName, IRIT_LINE_LEN_LONG - 1);
		    PFiles[0] = FileName;
		    PFiles[1] = NULL;
		    if ((PObj = IPGetDataFiles(PFiles, 1,
					       TRUE, TRUE)) != NULL) {
		        IGAddReplaceObjDisplayList(&IGGlblDisplayList,
						   PObj, NULL);
		    }
		}
		IGRedrawViewWindow();    /* Redraw with the new dragged obj. */
	    }
	    break;

	case WM_USER:
	    if (IGGlblActiveXMode)
		SendMessage(ParentWnd, WM_USER, 0, 0);

	    return 0;

        case WM_ERASEBKGND:
        case WM_SHOWWINDOW:
	case WM_NCACTIVATE:
	    if (IGGlblActiveXMode) {
	        RECT WinRect;

		/* Events arrive at the (0, 0) coordinate we try to ignore. */
	        GetWindowRect(hWndFrame, &WinRect);
		if (WinRect.left == 0 && WinRect.top == 0)
		    return 0;
	    }
        default:
	    return DefWindowProc(hWndFrame, wMsg, wParam, lParam);
    }
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* View window call back drawing function.				     *
*									     *
* PARAMETERS:                                                                *
*   hWndFrame:       A handle on the window.                                 *
*   wMsg:            Type of event to handle                                 *
*   mParam, lParam:  Parameters of event                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MRESULT:  Event state code.                                              *
*****************************************************************************/
static LONG APIENTRY SubViewWndProc(HWND hWndFrame,
				    UINT wMsg,
				    WPARAM wParam,
				    LONG lParam)
{       
    IRIT_STATIC_DATA IGGraphicEventType
	LastEvent[4] = { IG_EVENT_NONE,
			 IG_EVENT_NONE,
			 IG_EVENT_NONE,
			 IG_EVENT_NONE };
    IRIT_STATIC_DATA BOOL
	bButtonDown[4] =   { FALSE, FALSE, FALSE, FALSE },
	bRightBtnDown[4] = { FALSE, FALSE, FALSE, FALSE },
	bMiddleBtnDown[4] = { FALSE, FALSE, FALSE, FALSE },
	bLeftBtnDown[4] =  { FALSE, FALSE, FALSE, FALSE };
    IRIT_STATIC_DATA short
	iMouseX[4] = { 0, 0, 0, 0 },
	iMouseY[4] = { 0, 0, 0, 0 };

    switch (wMsg) {
	case WM_CREATE:
	    {
	        LPCREATESTRUCT
	            CreationInfo = (LPCREATESTRUCT) lParam;

	        CreateSubViewWindow(hWndFrame,
				    *((int *) CreationInfo -> lpCreateParams));
		return 0;
	    }
	case WM_PAINT:
	case WM_QUERYNEWPALETTE:
	case WM_PALETTECHANGED:
	    return RedrawSubViewWindow(hWndFrame, wMsg, wParam);
	case WM_SETCURSOR:
	    SetCursor(GlblCurrentCursor);
	    return 0;
	case WM_DESTROY:
	    if (!IGGlblActiveXMode)
	        PostQuitMessage(0);
	    return 0;
	case WM_LBUTTONDOWN:
	    {
	        CurrentSubViewInd = GetViewNum(hWndFrame);
		bButtonDown[CurrentSubViewInd] = TRUE;
		SetCapture(hWndFrame);
		if (IGCrvEditGrabMouse) {
		    CEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_CRV_EDIT_BUTTONDOWN);
		    return 0;
		}
		if (IGSrfEditGrabMouse) {
		    SEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_SRF_EDIT_BUTTONDOWN);
		    return 0;
		}
		if (PickCrsrGrabMouse) {
		    IGHandleCursorEvent((short) LOWORD(lParam),
					(short) HIWORD(lParam),
					IG_PICK_REP_BTN1DOWN);
		    return 0;
		}
		if (IGObjManipGrabMouse) {
		    IGObjManipHandleMouse((short) LOWORD(lParam),
					  (short) HIWORD(lParam),
					  IG_OBJ_MANIP_BTN1DOWN);
		    return 0;
		}
	  
		if (wParam & MK_SHIFT) {
		    IPObjectStruct
		        *PObj = HandleSubViewPickEventAux(LOWORD(lParam),
							  HIWORD(lParam),
							  IG_PICK_ANY,
							  CurrentSubViewInd);
		
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
			SetWindowText(IGhTopLevel,
				      IGGenerateWindowHeaderString(
					  IGStrResGenericPickEvent(PObj)));
		    }
		
		    return 0;
		}
		LastEvent[CurrentSubViewInd] = IG_EVENT_ROTATE;
		iMouseX[CurrentSubViewInd] = LOWORD(lParam);
		iMouseY[CurrentSubViewInd] = HIWORD(lParam);
		bLeftBtnDown[CurrentSubViewInd] =
		    IGGlblManipulationActive = TRUE;
		return 0;
	    }
	    break;
	case WM_MBUTTONDOWN:
	    {
	        CurrentSubViewInd = GetViewNum(hWndFrame);
		bButtonDown[CurrentSubViewInd] = TRUE;
		SetCapture(hWndFrame);
		if (PickCrsrGrabMouse) {
		    IGHandleCursorEvent((short) LOWORD(lParam),
					(short) HIWORD(lParam),
					IG_PICK_REP_BTN2DOWN);
		    return 0;
		}
		LastEvent[CurrentSubViewInd] = IG_EVENT_SCALE;
		iMouseX[CurrentSubViewInd] = LOWORD(lParam);
		iMouseY[CurrentSubViewInd] = HIWORD(lParam);
		bMiddleBtnDown[CurrentSubViewInd] =
		    IGGlblManipulationActive = TRUE;
		return 0;
	    }
	    break;
	case WM_RBUTTONDOWN:
	    {
	        CurrentSubViewInd = GetViewNum(hWndFrame);
		bButtonDown[CurrentSubViewInd] = TRUE;
		SetCapture(hWndFrame);
		if (PickCrsrGrabMouse) {
		    IGHandleCursorEvent((short) LOWORD(lParam),
					(short) HIWORD(lParam),
					IG_PICK_REP_BTN3DOWN);
		    return 0;
		}
		if (IGObjManipGrabMouse) {
		    IGObjManipHandleMouse((short) LOWORD(lParam),
					  (short) HIWORD(lParam),
					  IG_OBJ_MANIP_BTN3DOWN);
		    SetCapture(hWndFrame);
		    return 0;
		}
		LastEvent[CurrentSubViewInd] = IG_EVENT_TRANSLATE;
		iMouseX[CurrentSubViewInd] = LOWORD(lParam);
		iMouseY[CurrentSubViewInd] = HIWORD(lParam);
		bRightBtnDown[CurrentSubViewInd] =
		    IGGlblManipulationActive = TRUE;
		return 0;
	    }
	    break;
	case WM_MOUSEMOVE:
	    {
	        int SubViewInd = GetViewNum(hWndFrame);

		if (IGCrvEditGrabMouse) {
		    CEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_CRV_EDIT_MOTION);
		    return 0;
		}
		if (IGSrfEditGrabMouse) {
		    SEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_SRF_EDIT_MOTION);
		    return 0;
		}
		if (bButtonDown[CurrentSubViewInd] && PickCrsrGrabMouse) {
		    IGHandleCursorEvent((short) LOWORD(lParam),
					(short) HIWORD(lParam),
					IG_PICK_REP_MOTION);
		    return 0;
		}
		if (IGObjManipGrabMouse) {
		    IGObjManipHandleMouse((short) LOWORD(lParam),
					  (short) HIWORD(lParam),
					  IG_OBJ_MANIP_MOTION);
		    return 0;
		}

		if ((bLeftBtnDown[SubViewInd] ||
		     bMiddleBtnDown[SubViewInd] ||
		     bRightBtnDown[SubViewInd]) &&
		    IG_IS_DRAG_EVENT(LastEvent[SubViewInd])) {
		    short iNewMouseX, iNewMouseY;
		    int ProcessEventRes;
		    IrtRType ChangeFactor[2];
		    IrtHmgnMatType TempMat;

		    iNewMouseX = LOWORD(lParam);
		    iNewMouseY = HIWORD(lParam);
		    ChangeFactor[0] = (iNewMouseX -
				    iMouseX[SubViewInd]) * IGGlblChangeFactor;
		    ChangeFactor[1] = (iMouseY[SubViewInd] -
				       iNewMouseY) * IGGlblChangeFactor;
		    if (LastEvent[SubViewInd] == IG_EVENT_SCALE) {
		        ChangeFactor[0] *= SCALE_FACTOR_MODIFIER;
			ChangeFactor[1] *= SCALE_FACTOR_MODIFIER;
		    }
		    iMouseX[SubViewInd] = iNewMouseX;
		    iMouseY[SubViewInd] = iNewMouseY;
		    IRIT_GEN_COPY(TempMat, IPViewMat, sizeof(IrtHmgnMatType));
		    IRIT_GEN_COPY(IPViewMat, IGSubViewMat[SubViewInd],
			     sizeof(IrtHmgnMatType));
		    ProcessEventRes = IGProcessEvent(LastEvent[SubViewInd],
						     ChangeFactor);
		    IRIT_GEN_COPY(IGSubViewMat[SubViewInd], IPViewMat, 
			     sizeof(IrtHmgnMatType));
		    IRIT_GEN_COPY(IPViewMat, TempMat,
			     sizeof(IrtHmgnMatType));
		    if (ProcessEventRes) 
		        RedrawSubViewWindow(hWndFrame, WM_PAINT, wParam);
		  
		}
		return 0;
	    }
	    break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	    {
	        unsigned int i;

		for (i = 0; i < IG_NUM_OF_SUB_VIEWS; i++)
		    bButtonDown[i] = FALSE;

		ReleaseCapture();

		if (IGCrvEditGrabMouse) {
		    CEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_CRV_EDIT_BUTTONUP);
		    return 0;
		}
		if (IGSrfEditGrabMouse) {
		    SEditHandleMouse((short) LOWORD(lParam),
				     (short) HIWORD(lParam),
				     IG_SRF_EDIT_BUTTONUP);
		    return 0;
		}
		if (PickCrsrGrabMouse) {
		    IGHandleCursorEvent((short) LOWORD(lParam),
					(short) HIWORD(lParam),
					IG_PICK_REP_BTN_UP);
		    return 0;
		}
		if (IGObjManipGrabMouse) {
		    IGObjManipHandleMouse((short) LOWORD(lParam),
					  (short) HIWORD(lParam),
					  IG_OBJ_MANIP_BTN_UP);
		    return 0;
		}
	  
		for (i = 0; i < IG_NUM_OF_SUB_VIEWS; i++) {
		    /* Done with mouse click or drag operation. */
		    bLeftBtnDown[i] =
		        bMiddleBtnDown[i] =
			    bRightBtnDown[i] =
			        IGGlblManipulationActive = FALSE;
		}

		/* To redraw in highres: */
		InvalidateRect(hWndFrame, NULL, FALSE);
		return 0;
	    }
	    break;
	default:
	    return DefWindowProc(hWndFrame, wMsg, wParam, lParam);
    }

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* View window call back drawing function.				     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   hWndFrame:       A handle on the window.                                 M
*   wMsg:            Type of event to handle                                 M
*   wParam, lParam:  Parameters of event                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   LONG APIENTRY:  Event state code.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   ViewWndProc                                                              M
*****************************************************************************/
LONG APIENTRY ViewWndProc(HWND hWndFrame,
			  UINT wMsg,
			  WPARAM wParam,
			  LONG lParam)
{
    IRIT_STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    IRIT_STATIC_DATA BOOL
	bButtonDown = FALSE,
	bRightBtnDown = FALSE,
	bMiddleBtnDown = FALSE,
	bLeftBtnDown = FALSE;
    IRIT_STATIC_DATA short
	iMouseX = 0,
	iMouseY = 0;    

    switch (wMsg) {
	case WM_CREATE:
            {
	        IRIT_STATIC_DATA RECT rect;
		int ViewNum = 0;
		GetClientRect(hWndFrame, &rect);
		IGhWndSubView[0] =
		    CreateWindow(APP_SUB_VIEW_CLASS,
				 "v0",
				 WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					      (DoWindowBorder ? 0 : WS_BORDER),
				 0,
				 0,
				 rect.right / 2 - 1,
				 rect.bottom / 2 - 1,
				 hWndFrame,
				 0,
				 ((LPCREATESTRUCT) lParam)->hInstance,
				 &ViewNum);
		ViewNum = 1;
		IGhWndSubView[1] =
		    CreateWindow(APP_SUB_VIEW_CLASS,
				 "v1",
				 WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					      (DoWindowBorder ? 0 : WS_BORDER),
				 rect.right/2 + 2,
				 0,
				 rect.right / 2 -1,
				 rect.bottom / 2 - 1,
				 hWndFrame,
				 0,
				 ((LPCREATESTRUCT) lParam)->hInstance,
				 &ViewNum);
		ViewNum = 2;
		IGhWndSubView[2] =
		    CreateWindow(APP_SUB_VIEW_CLASS,
				 "v2",
				 WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					      (DoWindowBorder ? 0 : WS_BORDER),
				 0,
				 rect.bottom / 2 + 2,
				 rect.right / 2 -1 ,
				 rect.bottom / 2 - 1,
				 hWndFrame,
				 0,
				 ((LPCREATESTRUCT) lParam)->hInstance,
				 &ViewNum);
		ViewNum = 3;
		IGhWndSubView[3] =
		    CreateWindow(APP_SUB_VIEW_CLASS,
				 "v3",
				 WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					      (DoWindowBorder ? 0 : WS_BORDER),
				 rect.right / 2 + 2,
				 rect.bottom / 2 + 2,
				 rect.right / 2 - 1,
				 rect.bottom / 2 - 1,
				 hWndFrame,
				 0,
				 ((LPCREATESTRUCT) lParam)->hInstance,
				 &ViewNum);
	    }
	case WM_PAINT:
	case WM_QUERYNEWPALETTE:
	case WM_PALETTECHANGED:
	    if (IGGlbl4Views) {
	        int i;

		ClearBase4Views(hWndFrame); 
		for (i = 0; i < IG_NUM_OF_SUB_VIEWS; i++)
		    InvalidateRect(IGhWndSubView[i], NULL, FALSE);
		return 0;
	    }
	    else
	        return RedrawViewWindow(hWndFrame, wMsg, wParam);
	case WM_SETCURSOR:
	    SetCursor(GlblCurrentCursor);
	    return 0;
	case WM_SIZE:
	    {
	        RECT rect;

		rect.left   = 0;
		rect.top    = 0;
		rect.right  = LOWORD(lParam);
		rect.bottom = HIWORD(lParam);
		SetWindowPos(IGhWndSubView[0],
			     HWND_TOP,
			     0,
			     0,
			     IGSubViewWidth = rect.right / 2 - 1,
			     IGSubViewHeight = rect.bottom / 2 - 1,
			     0);
		SetWindowPos(IGhWndSubView[1],
			     HWND_TOP,
			     rect.right / 2 + 2,
			     0,
			     IGSubViewWidth ,
			     IGSubViewHeight,
			     0);
		SetWindowPos(IGhWndSubView[2],
			     HWND_TOP,
			     0,
			     rect.bottom / 2 + 2,
			     IGSubViewWidth ,
			     IGSubViewHeight,
			     0);
		SetWindowPos(IGhWndSubView[3],
			     HWND_TOP,
			     rect.right / 2 + 2,
			     rect.bottom / 2 + 2,
			     IGSubViewWidth ,
			     IGSubViewHeight,
			     0);
	    }
	    RedrawViewWindow(hWndFrame, wMsg, wParam);
	    return 0;

	case WM_DESTROY:
	    if (!IGGlblActiveXMode)
	        PostQuitMessage(0);
	    return 0;

	case WM_LBUTTONDOWN:
	    if (IGGlblActiveXMode && (wParam & MK_CONTROL)) {
	        SendMessage(GetParent(hWndFrame), WM_USER, 0, 0);
		return 0;
	    }

	    bButtonDown = TRUE;
	    SetCapture(hWndFrame);
	    if (IGCrvEditGrabMouse) {
		CEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_CRV_EDIT_BUTTONDOWN);
		return 0;
	    }
	    if (IGSrfEditGrabMouse) {
		SEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_SRF_EDIT_BUTTONDOWN);
		return 0;
	    }
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent((short) LOWORD(lParam),
				    (short) HIWORD(lParam),
				    IG_PICK_REP_BTN1DOWN);
		return 0;
	    }
	    if (IGObjManipGrabMouse) {
		IGObjManipHandleMouse((short) LOWORD(lParam),
				      (short) HIWORD(lParam),
				      IG_OBJ_MANIP_BTN1DOWN);
		return 0;
	    }

	    if (wParam & MK_SHIFT) {
		IPObjectStruct
		    *PObj = IGHandlePickEvent(LOWORD(lParam), HIWORD(lParam),
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
						  "*** no object ***", NULL);

		    IPSocWriteOneObject(IGGlblIOHandle, PObjDump);
		    if (PObj != PObjDump)
			IPFreeObject(PObjDump);
		}
		else {
		    SetWindowText(IGhTopLevel,
				  IGGenerateWindowHeaderString(
					     IGStrResGenericPickEvent(PObj)));
		}

		return 0;
	    }
	    LastEvent = IG_EVENT_ROTATE;
	    iMouseX = LOWORD(lParam);
	    iMouseY = HIWORD(lParam);
	    bLeftBtnDown = IGGlblManipulationActive = TRUE;
	    return 0;

	case WM_MBUTTONDOWN:
	    bButtonDown = TRUE;
	    SetCapture(hWndFrame);
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent((short) LOWORD(lParam),
				    (short) HIWORD(lParam),
				    IG_PICK_REP_BTN2DOWN);
		return 0;
	    }
	    LastEvent = IG_EVENT_SCALE;
	    iMouseX = LOWORD(lParam);
	    iMouseY = HIWORD(lParam);
	    bMiddleBtnDown = IGGlblManipulationActive = TRUE;
	    return 0;

	case WM_RBUTTONDOWN:
	    bButtonDown = TRUE;
	    SetCapture(hWndFrame);
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent((short) LOWORD(lParam),
				    (short) HIWORD(lParam),
				    IG_PICK_REP_BTN3DOWN);
		return 0;
	    }
	    if (IGObjManipGrabMouse) {
		IGObjManipHandleMouse((short) LOWORD(lParam),
				      (short) HIWORD(lParam),
				      IG_OBJ_MANIP_BTN3DOWN);
		SetCapture(hWndFrame);
		return 0;
	    }
	    LastEvent = IG_EVENT_TRANSLATE;
	    iMouseX = LOWORD(lParam);
	    iMouseY = HIWORD(lParam);
	    bRightBtnDown = IGGlblManipulationActive = TRUE;
	    return 0;

	case WM_MOUSEMOVE:
	    if (IGCrvEditGrabMouse) {
		CEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_CRV_EDIT_MOTION);
		return 0;
	    }
	    if (IGSrfEditGrabMouse) {
		SEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_SRF_EDIT_MOTION);
		return 0;
	    }
	    if (bButtonDown && PickCrsrGrabMouse) {
	        IGHandleCursorEvent((short) LOWORD(lParam),
				    (short) HIWORD(lParam),
				    IG_PICK_REP_MOTION);
		return 0;
	    }
	    if (IGObjManipGrabMouse) {
		IGObjManipHandleMouse((short) LOWORD(lParam),
				      (short) HIWORD(lParam),
				      IG_OBJ_MANIP_MOTION);
		return 0;
	    }
	    if ((bLeftBtnDown || bMiddleBtnDown || bRightBtnDown) &&
	        IG_IS_DRAG_EVENT(LastEvent)) {
		IrtRType ChangeFactor[2];
		short iNewMouseX, iNewMouseY;

		iNewMouseX = LOWORD(lParam);
		iNewMouseY = HIWORD(lParam);
		ChangeFactor[0] = (iNewMouseX - iMouseX) * IGGlblChangeFactor;
		ChangeFactor[1] = (iMouseY - iNewMouseY) * IGGlblChangeFactor;
		if (LastEvent == IG_EVENT_SCALE) {
		    ChangeFactor[0] *= SCALE_FACTOR_MODIFIER;
		    ChangeFactor[1] *= SCALE_FACTOR_MODIFIER;
		}
		iMouseX = iNewMouseX;
		iMouseY = iNewMouseY;
		if (IGProcessEvent(LastEvent, ChangeFactor))
		    IGRedrawViewWindow();
	    }
	    return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	    bButtonDown = FALSE;
	    ReleaseCapture();
	    if (IGCrvEditGrabMouse) {
		CEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_CRV_EDIT_BUTTONUP);
		return 0;
	    }
	    if (IGSrfEditGrabMouse) {
		SEditHandleMouse((short) LOWORD(lParam),
				 (short) HIWORD(lParam),
				 IG_SRF_EDIT_BUTTONUP);
		return 0;
	    }
	    if (PickCrsrGrabMouse) {
	        IGHandleCursorEvent((short) LOWORD(lParam),
				    (short) HIWORD(lParam),
				    IG_PICK_REP_BTN_UP);
		return 0;
	    }
	    if (IGObjManipGrabMouse) {
	        IGObjManipHandleMouse((short) LOWORD(lParam),
				      (short) HIWORD(lParam),
				      IG_OBJ_MANIP_BTN_UP);
		return 0;
	    }

	    /* Done with mouse click or drag operation. */
	    bLeftBtnDown = bMiddleBtnDown = bRightBtnDown =
	        IGGlblManipulationActive = FALSE;
	    IGRedrawViewWindow();		   /* To redraw in highres. */
	    return 0;

	default:
	    return DefWindowProc(hWndFrame, wMsg, wParam, lParam);
    }
    return 0;
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
	    SetCursor(GlblCurrentCursor = LoadCursor(NULL, IDC_ARROW));

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */
	    SetCursor(GlblCurrentCursor = LoadCursor(NULL, IDC_CROSS));

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */
	    SetCursor(GlblCurrentCursor = LoadCursor(NULL, IDC_UPARROW));

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trans window call back drawing function.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   hWndFrame:       A handle on the window.                                 M
*   wMsg:            Type of event to handle                                 M
*   wParam, lParam:  Parameters of event                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   LONG APIENTRY:  Event state code.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TransWndProc                                                             M
*****************************************************************************/
LONG APIENTRY TransWndProc(HWND hWndFrame,
			   UINT wMsg,
			   WPARAM wParam,
			   LONG lParam)
{
    IRIT_STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    IRIT_STATIC_DATA BOOL
	bShiftDown = FALSE,
	bLeftBtnDown = FALSE;
    IRIT_STATIC_DATA short
	iMouseX = 0,
	iMouseY = 0;
    IGGraphicEventType Event;
    IrtRType ChangeFactor[2];
    POINT Point;

    switch (wMsg) {
	case WM_PAINT:
	    RedrawTransformationWindow(hWndFrame);
	    return 0;

	case WM_SETCURSOR:
	    SetCursor(LoadCursor(NULL, IDC_ARROW));
 	    return 0;

	case WM_RBUTTONDOWN:
	    if (!GlblStateMenu)
		IGCreateStateMenu();

	    bShiftDown = wParam & MK_SHIFT;

	    Point.x = LOWORD(lParam);
	    Point.y = HIWORD(lParam);
	    ClientToScreen(hWndFrame, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND here. */
	    TrackPopupMenu(GlblStateMenu,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON,
			   Point.x, Point.y, 0, hWndFrame, NULL);
	    return 0;

	case WM_LBUTTONDOWN:
	    /* Save fact that button is pressed. Might be a drag operation.*/
	    bLeftBtnDown = TRUE;
	    SetCapture(hWndFrame);
	    iMouseX = LOWORD(lParam);
	    iMouseY = HIWORD(lParam);

	    if ((Event = GetGraphicEvent(ChangeFactor,
					 iMouseX, iMouseY)) != IG_EVENT_NONE) {
		ChangeFactor[0] *= IGGlblChangeFactor;
		ChangeFactor[1] *= IGGlblChangeFactor;

		IGGlblManipulationActive = TRUE;

		if (Event == IG_EVENT_QUIT) {
		    if (!IGGlblActiveXMode) {
		        if (IGGlblIOHandle >= 0)
			    IPCloseStream(IGGlblIOHandle, TRUE);

			exit(0);
		    }
		}
		else {
		    if (IGGlbl4Views) {
		        int ProcessEventRes;
			IrtHmgnMatType TempMat;

			IRIT_GEN_COPY(TempMat, IPViewMat,
				      sizeof(IrtHmgnMatType));
			IRIT_GEN_COPY(IPViewMat, IGSubViewMat[CurrentSubViewInd],
				      sizeof(IrtHmgnMatType));
			ProcessEventRes = IGProcessEvent(Event, ChangeFactor);
			IRIT_GEN_COPY(IGSubViewMat[CurrentSubViewInd], IPViewMat,
				      sizeof(IrtHmgnMatType));
			IRIT_GEN_COPY(IPViewMat, TempMat,
				      sizeof(IrtHmgnMatType));
			if (ProcessEventRes) 
			    RedrawSubViewWindow(IGhWndSubView[CurrentSubViewInd],
						WM_PAINT, wParam);
		    }
		    else {
		        if (IGProcessEvent(Event, ChangeFactor))
			    InvalidateRect(IGhWndView, NULL, FALSE);
		    }

		    /* Save the event in case drag operation is performed. */
		    LastEvent = Event;
		}
	    }
	    return 0;
		
	case WM_LBUTTONUP:
	    /* Done with mouse click or drag operation. */
	    bLeftBtnDown = FALSE;
	    if (IGGlblManipulationActive) {
	        IGGlblManipulationActive = FALSE;
		InvalidateRect(IGhWndView, NULL, FALSE);
	    }
	    ReleaseCapture();
	    return 0;

	case WM_MOUSEMOVE:
	    if (bLeftBtnDown && IG_IS_DRAG_EVENT(LastEvent)) {
		short iNewMouseX, iNewMouseY;

		iNewMouseX = LOWORD(lParam);
		iNewMouseY = HIWORD(lParam);
		ChangeFactor[0] = IGGlblChangeFactor * (iNewMouseX - iMouseX) /
						     ((IrtRType) TransWidth);
		ChangeFactor[1] = 0.0;

		iMouseX = iNewMouseX;
		if (IGGlbl4Views) {
		    int ProcessEventRes;
		    IrtHmgnMatType TempMat;

		    IRIT_GEN_COPY(TempMat, IPViewMat, sizeof(IrtHmgnMatType));
		    IRIT_GEN_COPY(IPViewMat, IGSubViewMat[CurrentSubViewInd],
			          sizeof(IrtHmgnMatType));
		    ProcessEventRes = IGProcessEvent(LastEvent, ChangeFactor);
		    IRIT_GEN_COPY(IGSubViewMat[CurrentSubViewInd], IPViewMat,
			          sizeof(IrtHmgnMatType));
		    IRIT_GEN_COPY(IPViewMat, TempMat, sizeof(IrtHmgnMatType));
		    if (ProcessEventRes) 
			RedrawSubViewWindow(IGhWndSubView[CurrentSubViewInd],
					    WM_PAINT, wParam);
		}
		else if (IGProcessEvent(LastEvent, ChangeFactor))
		    IGRedrawViewWindow();
	    }
	    return 0;

	case WM_DESTROY:
	    if (!IGGlblActiveXMode)
	        PostQuitMessage(0);
	    return 0;

	case WM_COMMAND:
	    /* Comamnds from the popup menu. */
	    if (IGHandleState(wParam,
			      bShiftDown ? IG_STATE_DEC : IG_STATE_INC,
			      TRUE))
		InvalidateRect(IGhWndView, NULL, FALSE);
	    return 0;

	default:
	    return DefWindowProc(hWndFrame, wMsg, wParam, lParam);
    }
    return 0;
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
    YPos = 1.0 - ((IrtRType) Y) / TransHeight;

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
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
	case IG_EVENT_PERS_ORTHO_TGL:
	    IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					       IG_VIEW_ORTHOGRAPHIC :
					       IG_VIEW_PERSPECTIVE;
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
	case IG_EVENT_DEPTH_CUE:
	    IGGlblDepthCue = !IGGlblDepthCue;
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    IGCreateStateMenu();
	    break;
    }

    *ChangeFactor = (((IrtRType) X) - TransWidth2) / TransWidth2;

    return RetVal;
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
	case IG_STATE_ANTI_ALIASING:
	case IG_STATE_DRAW_STYLE:
	case IG_STATE_DEPTH_CUE:
        case IG_STATE_SCR_OBJ_TGL:
        case IG_STATE_CONT_MOTION:
        case IG_STATE_NRML_ORIENT:
	case IG_STATE_PERS_ORTHO_TGL:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    break;
        case IG_EVENT_STATE:
	    EnvironmentCB();
	    break;
        case IG_EVENT_ANIMATION:
	    AnimationCB();
	    break;
        case IG_EVENT_SHADE_PARAM:
	    ShadeParamCB();
	    break;
        case IG_EVENT_CRV_EDIT:
	    CrvEditCB();
	    break;
        case IG_EVENT_SRF_EDIT:
	    SrfEditCB();
	    break;
        case IG_EVENT_PICK_OBJS:
	    PickObjsCB();
	    break;
        case IG_EVENT_OBJ_MANIP:
	    ObjManipCB();
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
* Redraws the transformation window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   hwnd:       A handle on the window.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawTransformationWindow(HWND hWnd)
{
    int i;
    long SubTransPosX, SubTransPosY, SubTransWidth, SubTransHeight;
    RECT rect;
    PAINTSTRUCT ps;

    /* Make sure the menu is consistent with internatal data. */
    InteractMenu.SubWindows[0].Str =
	IGGlblTransformMode == IG_TRANS_OBJECT ? "Object Trans."
					       : "Screen Trans.";
    InteractMenu.SubWindows[1].Str =
	IGGlblViewMode == IG_VIEW_PERSPECTIVE ? "Perspective" : "Orthographic";
    InteractMenu.SubWindows[12].Str =
	IGGlblDepthCue ? "Depth Cue" : "No Depth Cue";

    SubTransWidth = (int) (TransWidth * INTERACT_SUB_WINDOW_WIDTH);
    SubTransHeight = (int) (TransHeight * INTERACT_SUB_WINDOW_HEIGHT);
    SubTransPosX = (TransWidth - SubTransWidth) / 2;

    if (IGCurrenthDC = BeginPaint(hWnd, &ps)) {
	GetClientRect(hWnd, &rect);
	FillRect(IGCurrenthDC, &rect, IGBackGroundBrush);

	SetBkMode(IGCurrenthDC, TRANSPARENT);
	SetTextAlign(IGCurrenthDC, TA_CENTER | VTA_CENTER);

	for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
	    SetColorIndex2(InteractMenu.SubWindows[i].Color, 2);
	    SetTextColor(IGCurrenthDC, IGCrntColorHighIntensity);

	    SubTransPosY = (int) (TransHeight *
					(1.0 - InteractMenu.SubWindows[i].Y));

	    MoveToEx(IGCurrenthDC, SubTransPosX, SubTransPosY, NULL);
	    LineTo(IGCurrenthDC, SubTransPosX + SubTransWidth, SubTransPosY);
	    LineTo(IGCurrenthDC,
		   SubTransPosX + SubTransWidth, SubTransPosY - SubTransHeight);
	    LineTo(IGCurrenthDC, SubTransPosX, SubTransPosY - SubTransHeight);
	    LineTo(IGCurrenthDC, SubTransPosX, SubTransPosY);
	    if (InteractMenu.SubWindows[i].TextInside) {
	        DrawTextLocal(InteractMenu.SubWindows[i].Str,
			      TransWidth / 2,
			      SubTransPosY - SubTransHeight);
	    }
	    else {
		DrawTextLocal(InteractMenu.SubWindows[i].Str,
			      (TransWidth - SubTransWidth) / 3,
			      SubTransPosY - SubTransHeight);
		MoveToEx(IGCurrenthDC,
			 SubTransPosX + SubTransWidth / 2, SubTransPosY, NULL);
		LineTo(IGCurrenthDC,
		       SubTransPosX + SubTransWidth / 2,
		       SubTransPosY - SubTransHeight);
	    }
	}

	for (i = 0; i < INTERACT_NUM_OF_STRINGS; i++) {
	    SetColorIndex2(InteractMenu.Strings[i].Color, 2);
	    SetTextColor(IGCurrenthDC, IGCrntColorHighIntensity);

	    DrawTextLocal(InteractMenu.Strings[i].Str,
		  (int) (InteractMenu.Strings[i].X * TransWidth),
		  (int) ((1.0 - InteractMenu.Strings[i].Y) * TransHeight
						- SubTransHeight / 2));
	}

	if (IGCurrenthPen)
            DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));
	EndPaint(hWnd, &ps);
	IGCurrenthDC = 0;
	IGCurrenthPen = 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw text centered at the given position.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:         Text to draw.                                               *
*   PosX, PosY:  And where to draw it.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawTextLocal(char *Str, int PosX, int PosY)
{
    TextOut(IGCurrenthDC, PosX, PosY, Str, strlen(Str));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets the color/width according to the given color index/width.   	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Color:     Index of color to use. Must be between 0 and IG_MAX_COLOR.    *
*   Width:     In pixel, for line draw.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetColorIndex2(int Color, int Width)
{
    if (Color > IG_MAX_COLOR)
	Color = IG_IRIT_WHITE;

    IGCrntColorHighIntensity = IGColorsHighIntensity[Color];
    IGCrntColorLowIntensity = IGColorsLowIntensity[Color];

    if (!IGCurrenthDC)
	return;
    if (IGCurrenthPen)
        DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));
    IGCurrenthPen = SelectObject(IGCurrenthDC, CreatePen(PS_SOLID, Width,
						     IGCrntColorHighIntensity));

    IGGlblIntensityHighState = TRUE;
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
    Beep(1000, 100);
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
* Make error message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
    MessageBox(NULL, Msg, "Error", MB_ICONSTOP | MB_OK);
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
    return MessageBox(NULL, Msg, "", MB_ICONSTOP | MB_YESNO | MB_DEFBUTTON2)
								      == IDYES;
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
    if (Display4Views == IGGlbl4Views)
	return FALSE; 

    if (Display4Views) {
        int i;

	for (i = 0; i <IG_NUM_OF_SUB_VIEWS; i++)
	    ShowWindow(IGhWndSubView[i], SW_SHOW);
    }
    else { /* !Display4Views */
        int i;

	for (i = 0; i < IG_NUM_OF_SUB_VIEWS; i++)
	    ShowWindow(IGhWndSubView[i], SW_HIDE);
    }

    IGGlbl4Views = Display4Views;

    return TRUE;
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
    IrtHmgnMatType TempMat, Mat;

    IRIT_GEN_COPY(Mat, IPViewMat, sizeof(IrtHmgnMatType));
    MatGenMatRotZ1(IRIT_DEG2RAD(0.0), TempMat);
    IGUpdateViewConsideringScale(TempMat);
    IRIT_GEN_COPY(IGSubViewMat[0], IPViewMat, sizeof(IrtHmgnMatType));

    MatGenMatRotY1(IRIT_DEG2RAD(90.0), TempMat);
    IGUpdateViewConsideringScale(TempMat);
    IRIT_GEN_COPY(IGSubViewMat[1], IPViewMat, sizeof(IrtHmgnMatType));

    MatGenMatRotX1(IRIT_DEG2RAD(90.0), TempMat);
    IGUpdateViewConsideringScale(TempMat);
    IRIT_GEN_COPY(IGSubViewMat[2], IPViewMat, sizeof(IrtHmgnMatType));

    IGUpdateViewConsideringScale(IGGlblIsometryViewMat);
    IRIT_GEN_COPY(IGSubViewMat[3], IPViewMat, sizeof(IrtHmgnMatType));

    IRIT_GEN_COPY(IPViewMat, Mat, sizeof(IrtHmgnMatType));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the view matrix to specify which view we arecurrently working on. M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewNum: The index of the sub view window.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UpdateSubViewMatrix                                                      M
*****************************************************************************/
void UpdateSubViewMatrix(int ViewNum)
{
    IRIT_GEN_COPY(IPViewMat, IGSubViewMat[ViewNum], sizeof(IrtHmgnMatType));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This method returns the index of the view by checking the window handle  M
*   and comparing it to known subviews handles.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd: The handle of the window to check                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  The number of the subview.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetViewNum                                                               M
*****************************************************************************/
int GetViewNum(HWND hWnd)
{
    if (hWnd == IGhWndSubView[0])
        return 0;
    if (hWnd == IGhWndSubView[1])
        return 1;
    if (hWnd == IGhWndSubView[2])
        return 2;
    if (hWnd == IGhWndSubView[3])
        return 3;

    IGIritError("The handle is not a sub window");

    return -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the pixels in the window pointed out by hWnd. 		     M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:   Window to update its pixels.                                     M
*   MemDC:  Holding to image to copy to window.				     M
*   x, y:   The dimensions of the window and the image.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGUpdateWindowPixelsFromBG, IGCaptureWindowPixels                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGUpdateWindowPixels                                                     M
*****************************************************************************/
void IGUpdateWindowPixels(HWND hWnd, HDC MemDC, int x, int y)
{
    int i;
    HDC hDC;

    hDC = GetDC(hWnd);

    i = BitBlt(hDC, 0, 0, x, y, MemDC, 0, 0, SRCCOPY);
    assert (i != 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Captures the pixels in the window pointed out by hWnd.  		     M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:   Window to capture its pixels.                                    M
*   x, y:   The dimensions of the window and the captured buffer.            M
*   ReturnImage:  If TRUE, the returned reference is to an image as a buffer M
*			   of size (3 *x *y) of RGB bytes, as RGB,RGB,RGB... M
*		  If FALSE, the returned reference is to an HDC holding the  M
*			   captured image.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *:   A buffer of size (3 *x *y), allocated dynamically, that        M
*	      captures the pixels in the window, or a reference to an HDC.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGUpdateWindowPixels, IGUpdateWindowPixelsFromBG                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCaptureWindowPixels                                                    M
*****************************************************************************/
void *IGCaptureWindowPixels(HWND hWnd, int *x, int *y, int ReturnImage)
{
    IRIT_STATIC_DATA HDC MemDC;
    int i;
    HDC hDC;
    HBITMAP MemBM;
    BITMAPINFO BMI;
    RECT rect;

    hDC = GetDC(hWnd);

    GetClientRect(hWnd, &rect);
    *x = ((3 + rect.right - rect.left) & ~0x03); /* 4 - align. */
    *y = rect.bottom - rect.top;
    MemDC = CreateCompatibleDC(hDC);
    MemBM = CreateCompatibleBitmap(hDC, *x, *y);
    SelectObject(MemDC, MemBM);
    i = BitBlt(MemDC, 0, 0, *x, *y, hDC, 0, 0, SRCCOPY);
    assert(i != 0);

    if (ReturnImage) {
        IrtBType *p,
	    *ImageBuffer = IritMalloc(3 * (4 + *x) * (4 + *y));

	IRIT_ZAP_MEM(&BMI, sizeof(BITMAPINFO));
	BMI.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER); 
	BMI.bmiHeader.biWidth         = *x; 
	BMI.bmiHeader.biHeight        = *y; 
	BMI.bmiHeader.biPlanes        = 1; 
	BMI.bmiHeader.biBitCount      = 24; 
	BMI.bmiHeader.biCompression   = BI_RGB; 

	GetDIBits(MemDC, MemBM, 0, *y, NULL, &BMI, DIB_RGB_COLORS);
	i = GetDIBits(MemDC, MemBM, 0, *y, ImageBuffer, &BMI, DIB_RGB_COLORS);
	assert(i == *y);

	/* In windows, for some reason it is kept as BGR,BGR,BGR... */
	for (i = *x * *y, p = ImageBuffer; i-- > 0; p += 3) {
	    IRIT_SWAP(IrtBType, p[0], p[2]);
	}

	return ImageBuffer;
    }
    else {
        return &MemDC;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This method handles sub view pick event.                                 *
*   It does so by temporerly setting global parameters to reflect a          *
*   single view state withe the appropriate parameters.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   ScreenX:   The X coordinate on the screen to pick an object from         *
*   ScreenY:   The Y coordinate on the screen to pick an object from         *
*   PickType:  The object type to be picked.                                 *
*   ViewNum:   The index of the subview.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: return TRUE iff there was any change in IGGlbl4Views                *
*****************************************************************************/
static IPObjectStruct *HandleSubViewPickEventAux(int ScreenX, 
						 int ScreenY,
						 int PickTypes,
						 int ViewNum)
{
    unsigned int i,
	OrigViewWidth = IGViewWidth,
	OrigViewHeight = IGViewHeight,
	OrigViewWidth2 = IGViewWidth2,
	OrigViewHeight2 = IGViewHeight2;
    IrtHmgnMatType CurViewMat;
    IPObjectStruct *Result;

    IRIT_GEN_COPY(CurViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    UpdateSubViewMatrix(ViewNum);

    /* Update the size of the screen. */
    IGViewWidth = IGSubViewWidth;
    IGViewHeight = IGSubViewHeight;
    IGViewWidth2 = IGSubViewWidth / 2;
    IGViewHeight2 = IGSubViewHeight / 2;

    Result = IGHandlePickEvent(ScreenX,ScreenY,PickTypes);

    /* Restore original values. */
    IGViewWidth = OrigViewWidth;
    IGViewHeight = OrigViewHeight;
    IGViewWidth2 = OrigViewWidth2;
    IGViewHeight2 = OrigViewHeight2;

    IRIT_GEN_COPY(IPViewMat, CurViewMat, sizeof(IrtHmgnMatType));
  
    /* Update the display. */
    InvalidateRect(IGhWndView, NULL, FALSE);
    for (i = 0 ; i < IG_NUM_OF_SUB_VIEWS; i++)
	InvalidateRect(IGhWndSubView[i], NULL, FALSE);

    return Result;
}
